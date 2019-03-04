// Copyright (C) 2019 Yu Yang
//
// This file is part of Vesyla.
//
// Vesyla is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vesyla is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vesyla.  If not, see <http://www.gnu.org/licenses/>.

#include "VElaborator.hpp"
#include "ExpressionEvaluator.hpp"
#include "VManager/FileManager.hpp"
#include "BIR/DPUMode.hpp"

using namespace VIR;
using namespace VirEnumerations;

#define CALL_EXPRESSION_FUNC(value)                      \
	case kt##value:                                        \
		expression = elaborate##value((value *)expression_); \
		break;
#define CALL_STATEMENT_FUNC(value)                     \
	case kt##value:                                      \
		statement = elaborate##value((value *)statement_); \
		break;

// TODO: remove createSliceNameAndDummyVarForAConstant() function

VElaborator::VElaborator() : _exprEvaluator(new ExpressionEvaluator()), _currStatementNo(0), _usedRegFileNumber(0), _raccuMode(false)
{
}

VElaborator::~VElaborator()
{
	delete _exprEvaluator;
}

VIR::MainProgram *VElaborator::elaborate(VIR::MainProgram *mainProgram_)
{
	CREATE_NEW_LIBRARY;

	_currForStatement = 0;

	VIR::MainProgram *main_prog;
	main_prog = elaborateMainProgram(mainProgram_);
	main_prog = traverseMainProgram(main_prog);
	//vesyla::elaborate::AddressElaborator ae;
	//ae.elaborate(main_prog);

	return main_prog;
}

// This method informs other Vesyla components that whether
// the given program needs a dynamic management or not
bool VElaborator::needDynamicManagement() const
{
	return _isThereDynamicLoops;
}

// This method sets the RACCU mode.
void VElaborator::setRaccuMode(bool enable_)
{
	_raccuMode = enable_;
}

// -------------------------------------
// ---------- Private Methods ----------

VIR::Array *VElaborator::elaborateArray(VIR::Array *array_)
{
	Array *array = CREATE_OBJECT(Array);

	elaborateExpressionList(&array_->elements(), &array->elements());

	return array;
}

VIR::AssignmentPragma *VElaborator::elaborateAssignmentPragma(VIR::AssignmentPragma *assignmentPragma_, VIR::NumericLiteral *lhsObjectValue_)
{
	AssignmentPragma *assignmentPragma = 0;

	if (assignmentPragma_)
	{
		switch (assignmentPragma_->kind())
		{
		case ktRaccuPragma:
			assignmentPragma = CREATE_OBJECT(RaccuPragma);
			break;

		case ktAddressPragma:
			assignmentPragma = CREATE_OBJECT(AddressPragma);
			break;

		case ktTempVarPragma:
			assignmentPragma = CREATE_OBJECT(TempVarPragma);
			break;

		default:
			assignmentPragma = elaborateReferencePragma((VIR::ReferencePragma *)assignmentPragma_);
			break;
		}
	}

	return assignmentPragma;
}

VIR::AssignmentStatement *VElaborator::elaborateFunctionCallStatement(VIR::FunctionCallStatement *functionCallStatement_)
{

	AssignmentStatement *assignmentStatement = CREATE_OBJECT(AssignmentStatement);
	assignmentStatement->rhs(elaborateExpression(functionCallStatement_->function_call));

	Expression *rhsExpr = assignmentStatement->rhs();

	// Finding RHS identifiers and slice names
	findIdentifiersAndSliceNames(rhsExpr, &assignmentStatement->rhsIdentifiers(), &assignmentStatement->rhsSliceNames());

	// Elaborate its pragma
	assignmentStatement->pragma(elaborateAssignmentPragma(functionCallStatement_->pragma, NULL));

	// Determining the assignment type
	if (assignmentStatement->pragma())
	{
		switch (assignmentStatement->pragma()->kind())
		{
		case ktDPUPragma:
			assignmentStatement->type(atArithmetic);
			break;

		default:
			LOG(FATAL) << ("elaborateAssignmentStatement: Unknown Pragma!");
			break;
		}
	}

	switch (assignmentStatement->type())
	{
	case atArithmetic:
		// determine the DPU mode if it is not specified in the pragma
		setDpuMode(assignmentStatement->rhs(), (DPUPragma *)assignmentStatement->pragma());
		replaceIdentifiersWithSliceNames(assignmentStatement);
	default:
		break;
	}

	switch (assignmentStatement->type())
	{
	case atArithmetic:
		// Finding data dependencies of the LHS and RHS identifiers to the previous assignment statements
		// findDataDependencies(lhsIdentifier, & assignmentStatement->rhsIdentifiers());
		// Finding index dependencies of the LHS and RHS identifiers to the previous assignment statements or loop variables
		findAssignStatementIndexDependencies(assignmentStatement);
	}

	// If there is any loop surrounding this assign statement,
	// check whether it needs to be handled dynamically because of this assign statement.
	// If this assign statement is a raccu statement or there are some vectors in the statement,
	// the loop should be treated dynamically.
	if (_currForStatement)
	{
		bool needDynamic = false;

		if (assignmentStatement->type() == atRaccuVariable)
			needDynamic = true;
		else
		{
			// Check rhs SliceNames to see if any of them are vectors
			for (int i = 0; i < assignmentStatement->rhsSliceNames().size(); ++i)
			{
				SliceName *sliceName = assignmentStatement->rhsSliceNames()[i];

				// TODO: Currently it only supports one dimensional indices
				//	     It should be extended to multi dimensional indices
				Expression *sliceNameIndex = sliceName->suffix()[0];

				// It is a vector
				if (sliceNameIndex->kind() == ktRangeExpression)
				{
					needDynamic = true;
					break;
				}
			}
		}

		if (needDynamic)
		{
			_currForStatement->needDynamicHandling(true);
			_isThereDynamicLoops = true;
		}
	}
	return assignmentStatement;
}

VIR::AssignmentStatement *VElaborator::elaborateAssignmentStatement(VIR::AssignmentStatement *assignmentStatement_)
{
	AssignmentStatement *assignmentStatement = CREATE_OBJECT(AssignmentStatement);

	_isAddressVariable = assignmentStatement_->pragma() && assignmentStatement_->pragma()->kind() == ktAddressPragma;

	for (auto &name : assignmentStatement_->lhs())
	{
		assignmentStatement->lhs().push_back(((Name *)elaborateExpression(name)));
	}

	assignmentStatement->rhs(elaborateExpression(assignmentStatement_->rhs()));

	Expression *rhsExpr = assignmentStatement->rhs();

	// Finding RHS identifiers and slice names
	findIdentifiersAndSliceNames(rhsExpr, &assignmentStatement->rhsIdentifiers(), &assignmentStatement->rhsSliceNames());

	assignmentStatement->pragma(elaborateAssignmentPragma(assignmentStatement_->pragma(), NULL));

	Identifier *lhsIdentifier = NULL;
	if (assignmentStatement->lhs().size() > 0)
	{
		if (dynamic_cast<Identifier *>(assignmentStatement->lhs()[0]))
		{
			lhsIdentifier = dynamic_cast<Identifier *>(assignmentStatement->lhs()[0]);
		}
		else if (dynamic_cast<SliceName *>(assignmentStatement->lhs()[0]))
		{
			lhsIdentifier = dynamic_cast<SliceName *>(assignmentStatement->lhs()[0])->prefix();
		}
	}

	// Determining the assignment type
	if (assignmentStatement->pragma())
	{
		switch (assignmentStatement->pragma()->kind())
		{
		case ktStoragePragma:
		{
			StoragePragma *storagePragma = (StoragePragma *)assignmentStatement->pragma();
			setConstantObjectValue(lhsIdentifier, &assignmentStatement->rhsIdentifiers(), rhsExpr);

			lhsIdentifier->object()->pragma(assignmentStatement->pragma());

			if (!storagePragma->isForIndirectAddress())
			{
				lhsIdentifier->object()->objectType(storagePragma->storageType() == stRegFile ? otRegisterFile : otMemory);
				assignmentStatement->type(atDeclarative);
			}
			else
			{
				lhsIdentifier->object()->objectType(otIndirectAddressVariable);
				assignmentStatement->type(atIndirectAddressDeclaration);
			}
		}
		break;

		case ktDPUChainPragma:
			setConstantObjectValue(lhsIdentifier, &assignmentStatement->rhsIdentifiers(), rhsExpr);

			lhsIdentifier->object()->pragma(assignmentStatement->pragma());
			lhsIdentifier->object()->objectType(otDPUChain);

			assignmentStatement->type(atDPUChainDeclaration);
			break;

		case ktDPUPragma:
			assignmentStatement->type(atArithmetic);
			break;

		case ktRaccuPragma:
			assignmentStatement->type(atRaccuVariable);
			lhsIdentifier->object()->objectType(otRaccuVariable);
			break;

			//TODO: This part is no longer needed
		case ktAddressPragma:
			setConstantObjectValue(lhsIdentifier, &assignmentStatement->rhsIdentifiers(), rhsExpr);

			assignmentStatement->type(atAddressVariable);
			lhsIdentifier->object()->objectType(otAddressVariable);
			break;

		case ktTempVarPragma:
			assignmentStatement->type(atTempVariable);
			lhsIdentifier->object()->objectType(otTempVariable);
			break;

		default:
			LOG(FATAL) << ("elaborateAssignmentStatement: Unknown Pragma!");
			break;
		}
	}
	else
	{
		// Set the object value of the LHS identifier if the RHS part is constant
		bool is_constant = setConstantObjectValue(lhsIdentifier, &assignmentStatement->rhsIdentifiers(), rhsExpr);

		if (is_constant)
		{
			lhsIdentifier->object()->objectType(otConstant);
			assignmentStatement->type(atConstant);
		}
		else
		{
			// Determine the type of assignment statement if there is no pragma bind to it.
			if (lhsIdentifier->object()->pragma() && lhsIdentifier->object()->pragma()->kind() == ktRaccuPragma)
			{
				assignmentStatement->pragma(lhsIdentifier->object()->pragma());
				assignmentStatement->type(atRaccuVariable);
			}
			if (lhsIdentifier->object()->pragma() && lhsIdentifier->object()->pragma()->kind() == ktDPUChainPragma)
			{
				assignmentStatement->pragma(lhsIdentifier->object()->pragma());
				assignmentStatement->type(atDPUChainConnection);
			}
			else if (lhsIdentifier->object()->objectType() == otAddressVariable)
			{
				assignmentStatement->type(atAddressVariable);

				AddressObjectDeclaration *addressObject = (AddressObjectDeclaration *)lhsIdentifier->object();

				BinaryExpression *binaryExpression = dynamic_cast<BinaryExpression *>(assignmentStatement->rhs());

				// TODO: currently only binary expression for explicit address variables are supported.
				// Also at least one of the operands should be an identifier, which is the same with the lhs identifier.
				if (!binaryExpression || (binaryExpression->leftOperand()->kind() != ktIdentifier && binaryExpression->rightOperand()->kind() != ktIdentifier))
					LOG(FATAL) << ("elaborateAssignmentStatement: Unsupported operation for explicit address variables!");

				// Set the operand that is not the same with the lhs identifier as the iteration amount
				if (binaryExpression->leftOperand()->kind() == ktIdentifier && ((Identifier *)binaryExpression->leftOperand())->name() == lhsIdentifier->name())
					addressObject->iterationAmount(binaryExpression->rightOperand());
				else
					addressObject->iterationAmount(binaryExpression->leftOperand());

				addressObject->operatorType(binaryExpression->operatorType());
				addressObject->addressIterator(assignmentStatement);
			}
			else if (assignmentStatement->rhsIdentifiers().size() == 1 && assignmentStatement->rhs()->kind() == ktIdentifier &&
							 ((Identifier *)assignmentStatement->rhs())->object()->objectType() == otTempVariable)
			{
				assignmentStatement->type(atTempAssignment);
			}
			else if (assignmentStatement->rhsIdentifiers().size() == 1 && assignmentStatement->rhs()->kind() != ktPrimitiveFunctionCall &&
							 lhsIdentifier->object()->pragma()) // The final condition is for skipping cases where this assignment statement is for address calculation
			{
				StoragePragma *lhsPragma = dynamic_cast<StoragePragma *>(lhsIdentifier->object()->pragma());
				StoragePragma *rhsPragma = dynamic_cast<StoragePragma *>(assignmentStatement->rhsIdentifiers()[0]->object()->pragma());

				if (lhsPragma && rhsPragma)
				{
					if (lhsPragma->storageType() == stRegFile && rhsPragma->storageType() == stMemory)
					{
						assignmentStatement->pragma(lhsPragma);
						assignmentStatement->type(atMemoryTransfer);
					}
					else if (lhsPragma->storageType() == stMemory && rhsPragma->storageType() == stRegFile)
					{
						assignmentStatement->pragma(rhsPragma);
						assignmentStatement->type(atMemoryTransfer);
					}
					else if (lhsPragma->storageType() == stRegFile && rhsPragma->storageType() == stRegFile)
					{
						assignmentStatement->pragma(lhsPragma);
						assignmentStatement->type(atRegisterTransfer);
					}
				}
			}
			else
				assignmentStatement->type(atAddressCalculation);
		}
	}
	switch (assignmentStatement->type())
	{
	case atArithmetic:
	case atDPUChainConnection:
		// determine the DPU mode if it is not specified in the pragma
		setDpuMode(assignmentStatement->rhs(), (DPUPragma *)assignmentStatement->pragma());

		// Finding twin statement for statements that should be mapped to the same DPU with different outputs (like FFT butterfly).
		// TODO: this should be generalized in future. It is a naive solution at the moment.
		if (((DPUPragma *)assignmentStatement->pragma())->outPort == 1)
		{
			AssignmentStatement *prevAssignStatement = (AssignmentStatement *)(*_totalAssignmentStatementList)[_totalAssignmentStatementList->size() - 1];
			assignmentStatement->twinStatement(prevAssignStatement);
			prevAssignStatement->twinStatement(assignmentStatement);
		}
		replaceIdentifiersWithSliceNames(assignmentStatement);
		break;

	case atTempAssignment:
	case atMemoryTransfer:
	case atRegisterTransfer:
		replaceIdentifiersWithSliceNames(assignmentStatement);
		break;

	case atRaccuVariable:
		setRaccuMode(assignmentStatement->rhs(), (RaccuPragma *)assignmentStatement->pragma());
		break;

	case atAddressCalculation:
		// Check whether this address function is affine or non-affine
		lhsIdentifier->object()->objectType(
				_exprEvaluator->isExpressionAffine(assignmentStatement->rhs()) ? otAffineAddressFunction : otNonAffineAddressFunction);
		break;

	default:
		break;
	}

	for (auto &sn : assignmentStatement->lhs())
	{
		if (sn->kind() == ktSliceName)
		{
			assignmentStatement->lhsSliceNames().push_back((SliceName *)sn);
			assignmentStatement->lhsIdentifiers().push_back(((SliceName *)sn)->prefix());
		}
		else if (sn->kind() == ktIdentifier)
		{
			// BUG HERE
			// IF IDENTIFIER IS ON THE LEFT SIDE, THE SLICENAME WILL NOT BE FILLED, LATER
			// SLICENAME IS NEEDED.
			assignmentStatement->lhsSliceNames().push_back(NULL);
			assignmentStatement->lhsIdentifiers().push_back((Identifier *)sn);
		}
		else
		{
			LOG(FATAL) << "Unsupported type in LHS of statement!";
		}
	}

	switch (assignmentStatement->type())
	{
	case atAddressCalculation:
	case atAddressVariable:
	case atArithmetic:
	case atTempAssignment:
	case atDPUChainConnection:
	case atMemoryTransfer:
	case atRaccuVariable:
	case atRegisterTransfer:
		// Finding data dependencies of the LHS and RHS identifiers to the previous assignment statements
		findDataDependencies(lhsIdentifier, &assignmentStatement->rhsIdentifiers());

		// Finding index dependencies of the LHS and RHS identifiers to the previous assignment statements or loop variables
		findAssignStatementIndexDependencies(assignmentStatement);
	}

	// If there is any loop surrounding this assign statement,
	// check whether it needs to be handled dynamically because of this assign statement.
	// If this assign statement is a raccu statement or there are some vectors in the statement,
	// the loop should be treated dynamically.
	if (_currForStatement)
	{
		bool needDynamic = false;

		if (assignmentStatement->type() == atRaccuVariable)
			needDynamic = true;
		else
		{
			// Check rhs SliceNames to see if any of them are vectors
			for (int i = 0; i < assignmentStatement->rhsSliceNames().size(); ++i)
			{
				SliceName *sliceName = assignmentStatement->rhsSliceNames()[i];

				// TODO: Currently it only supports one dimensional indices
				//	     It should be extended to multi dimensional indices
				Expression *sliceNameIndex = sliceName->suffix()[0];

				// It is a vector
				if (sliceNameIndex->kind() == ktRangeExpression)
				{
					needDynamic = true;
					break;
				}
			}
		}

		if (needDynamic)
		{
			_currForStatement->needDynamicHandling(true);
			_isThereDynamicLoops = true;
		}

		////////////////////////////////////////////////////////////////////////////////
		// FIXME: important
		// Force loop to be dynamic
		////////////////////////////////////////////////////////////////////////////////
		_currForStatement->needDynamicHandling(true);
		_isThereDynamicLoops = true;
		////////////////////////////////////////////////////////////////////////////////
		// block ends
		////////////////////////////////////////////////////////////////////////////////
	}
	return assignmentStatement;
}

VIR::BinaryExpression *VElaborator::elaborateBinaryExpression(VIR::BinaryExpression *binaryExpression_)
{
	BinaryExpression *binaryExpression = CREATE_OBJECT(BinaryExpression);

	binaryExpression->leftOperand(elaborateExpression(binaryExpression_->leftOperand()));
	binaryExpression->operatorType(binaryExpression_->operatorType());
	binaryExpression->rightOperand(elaborateExpression(binaryExpression_->rightOperand()));

	return binaryExpression;
}

VIR::Complex *VElaborator::elaborateComplex(VIR::Complex *complex_)
{
	Complex *complex = CREATE_OBJECT(Complex);

	complex->iValue = complex_->iValue;
	complex->rValue = complex_->rValue;

	return complex;
}

VIR::DebugStatement *VElaborator::elaborateDebugStatement(VIR::DebugStatement *debugStatement_)
{
	DebugStatement *debugStatement = CREATE_OBJECT(DebugStatement);

	debugStatement->debugStr(debugStatement_->debugStr());

	return debugStatement;
}

VIR::Expression *VElaborator::elaborateExpression(VIR::Expression *expression_)
{

	WriteLineToF("Creating " + expression_->kindStr(), ftElaborator, true);

	Expression *expression;

	switch (expression_->kind())
	{
		CALL_EXPRESSION_FUNC(Array);
		CALL_EXPRESSION_FUNC(BinaryExpression);
		CALL_EXPRESSION_FUNC(Complex);
		CALL_EXPRESSION_FUNC(FloatingPoint);
		CALL_EXPRESSION_FUNC(FunctionCall);
		CALL_EXPRESSION_FUNC(Identifier);
		CALL_EXPRESSION_FUNC(Integer);
		CALL_EXPRESSION_FUNC(ParenthesizedExpression);
		CALL_EXPRESSION_FUNC(RangeExpression);
		CALL_EXPRESSION_FUNC(SliceName);
		CALL_EXPRESSION_FUNC(StringLiteral);
		CALL_EXPRESSION_FUNC(UnaryExpression);
	}

	// BUG: Unary expression like -3 can't be recongnized as integer before simplifacation
	// TEMP FIX: Simplify expression after elaboration.
	// MODIFY BEGIN
	//	expression = _exprEvaluator->simplifyExpression(expression);
	// MODIFY END

	return expression;
}

void VElaborator::elaborateExpressionList(vector<VIR::Expression *> *oldExpressionList_, vector<VIR::Expression *> *newExpressionList_)
{
	const int size = oldExpressionList_->size();

	for (int index = 0; index < size; ++index)
	{
		Expression *expression = elaborateExpression((*oldExpressionList_)[index]);

		newExpressionList_->push_back(expression);
	}
}

VIR::FloatingPoint *VElaborator::elaborateFloatingPoint(VIR::FloatingPoint *floatingPoint_)
{
	FloatingPoint *floatingPoint = CREATE_OBJECT(FloatingPoint);

	floatingPoint->value = floatingPoint_->value;

	return floatingPoint;
}

VIR::ForStatement *VElaborator::elaborateForStatement(VIR::ForStatement *forStatement_)
{
	ForStatement *forStatement = CREATE_OBJECT(ForStatement);

	addToTotalStatementList(forStatement);

	forStatement->loopVariable(elaborateIdentifier(forStatement_->loopVariable(), true));
	forStatement->parallelMode(forStatement_->parallelMode());

	// Don't try to simplify loop range, buggy!
	//forStatement->loopRange((RangeExpression *) _exprEvaluator->simplifyExpression(elaborateRangeExpression(forStatement_->loopRange())));
	forStatement->loopRange((RangeExpression *)elaborateRangeExpression(forStatement_->loopRange()));

	forStatement->isConstant(_exprEvaluator->isExpressionConstant(forStatement->loopRange()));

	LoopObjectDeclaration *loopObject = (LoopObjectDeclaration *)forStatement->loopVariable()->object();

	loopObject->objectType(forStatement->isConstant() ? otConstantLoopIndex : otNonConstantLoopIndex);
	loopObject->forStatement(forStatement);

	ControlStatement *upperControlStatement = _currControlStatement;
	_currControlStatement = forStatement;

	ForStatement *upperForStatement = _currForStatement;
	_currForStatement = forStatement;

	elaborateStatementList(&forStatement_->loopBody(), &forStatement->loopBody());

	_currControlStatement = upperControlStatement;
	_currForStatement = upperForStatement;

	// Check if there is an outer loop, increase the degree of that loop.
	// Note the the second condition is important when
	// there are more than one loop (at the same level) inside an outer loop.
	if (upperForStatement && upperForStatement->degree() < forStatement->degree() + 1)
		upperForStatement->degree(forStatement->degree() + 1);

	int repetition = 0;

	if (forStatement->isConstant())
	{
		// Calculating the number of repetition in this for loop only if the loop range is constant
		// Repetition value is positive for increasing loop range and negative for decreasing loop range
		// Assumption: considering that the loop ranges are integer constants.
		repetition = int(floor((((Integer *)forStatement->loopRange()->end())->value - ((Integer *)forStatement->loopRange()->begin())->value) /
													 double(((Integer *)forStatement->loopRange()->increment())->value)));

		repetition += repetition >= 0 ? +1 : -1;
	}
	// else if the degree is greater that 2, it can not be handled by loop unrolling so it need to be handled dynamically
	else if (forStatement->degree() > 2)
		_isThereDynamicLoops = true;

	forStatement->repetition(repetition);

	findLoopRangeIndexDependencies(forStatement->loopRange());

	return forStatement;
}

VIR::Identifier *VElaborator::elaborateIdentifier(VIR::Identifier *identifier_, bool isLoopIndex_ /*= false*/)
{
	Identifier *identifier = CREATE_OBJECT(Identifier);

	identifier->name(identifier_->name());

	identifier->object(getIdentifierObject(identifier->name(), isLoopIndex_));

	return identifier;
}

// This method gets both of the old if statement and the new elaborated if statement, and elaborate the condition of the if statement
void VElaborator::elaborateIfCondition(VIR::IfStatement *oldIfStatement_, VIR::IfStatement *newIfStatement_)
{
	newIfStatement_->condition(elaborateExpression(oldIfStatement_->condition()));
	newIfStatement_->conditionPragma((DPUPragma *)elaborateReferencePragma(oldIfStatement_->conditionPragma()));

	setDpuMode(newIfStatement_->condition(), newIfStatement_->conditionPragma());
	findIfConditionIndexDependencies(newIfStatement_);

	// Check the operands for constant values and treat them specifically.
	if (newIfStatement_->condition()->kind() == ktBinaryExpression)
	{
		BinaryExpression *binaryExpression = (BinaryExpression *)newIfStatement_->condition();

		if (binaryExpression->leftOperand()->kind() == ktInteger || binaryExpression->leftOperand()->kind() == ktFloatingPoint)
			LOG(FATAL) << ("elaborateIfCondition: Unsupported branch condition! Use the constant value in the right side instead.");

		if (binaryExpression->rightOperand()->kind() == ktInteger || binaryExpression->leftOperand()->kind() == ktFloatingPoint)
			newIfStatement_->conditionPragma()->dpuMode = "silago_dpu_comparison_with_constant";
	}
}

VIR::IfStatement *VElaborator::elaborateIfStatement(VIR::IfStatement *ifStatement_)
{
	IfStatement *ifStatement = CREATE_OBJECT(IfStatement);

	if (_currForStatement)
	{
		_currForStatement->needDynamicHandling(true);
		_isThereDynamicLoops = true;
	}

	addToTotalStatementList(ifStatement);

	ifStatement->mode(ifStatement_->mode());

	elaborateIfCondition(ifStatement_, ifStatement);

	ControlStatement *upperControlStatement = _currControlStatement;
	_currControlStatement = ifStatement;

	elaborateStatementList(&ifStatement_->thenPart(), &ifStatement->thenPart());

	_currControlStatement = upperControlStatement;

	return ifStatement;
}

VIR::IfThenElseStatement *VElaborator::elaborateIfThenElseStatement(VIR::IfThenElseStatement *ifThenElseStatement_)
{
	IfThenElseStatement *ifThenElseStatement = CREATE_OBJECT(IfThenElseStatement);

	if (_currForStatement)
	{
		_currForStatement->needDynamicHandling(true);
		_isThereDynamicLoops = true;
	}

	addToTotalStatementList(ifThenElseStatement);

	ifThenElseStatement->mode(ifThenElseStatement_->mode());

	elaborateIfCondition(ifThenElseStatement_, ifThenElseStatement);

	ControlStatement *upperControlStatement = _currControlStatement;
	_currControlStatement = ifThenElseStatement;

	elaborateStatementList(&ifThenElseStatement_->thenPart(), &ifThenElseStatement->thenPart());

	elaborateStatementList(&ifThenElseStatement_->elsePart(), &ifThenElseStatement->elsePart());

	_currControlStatement = upperControlStatement;

	return ifThenElseStatement;
}

VIR::Integer *VElaborator::elaborateInteger(VIR::Integer *integer_)
{
	Integer *integer = CREATE_OBJECT(Integer);

	integer->value = integer_->value;

	return integer;
}

VIR::MainProgram *VElaborator::elaborateMainProgram(VIR::MainProgram *mainProgram_)
{
	WriteLineToF("Creating " + mainProgram_->kindStr(), ftElaborator, true);

	MainProgram *mainProgram = CREATE_OBJECT(MainProgram);

	mainProgram->name(mainProgram_->name());

	_currControlStatement = 0;
	_totalAssignmentStatementList = &mainProgram->totalAssignmentStatements();

	// It is updated inside the elabroateForStatement
	_isThereDynamicLoops = false;

	elaborateStatementList(&mainProgram_->body(), &mainProgram->body());

	mainProgram->hasDynamicLoops(_isThereDynamicLoops);

	return mainProgram;
}

// TODO: try to remove the parenthesized expression
VIR::Expression *VElaborator::elaborateParenthesizedExpression(VIR::ParenthesizedExpression *parenthesizedExpression_)
{
	ParenthesizedExpression *parenthesizedExpression = CREATE_OBJECT(ParenthesizedExpression);

	parenthesizedExpression->value(elaborateExpression(parenthesizedExpression_->value()));

	return parenthesizedExpression;
}

VIR::PragmaStatement *VElaborator::elaboratePragmaStatement(VIR::PragmaStatement *pragmaStatement_)
{
	VIR::PragmaStatement *pragmaStatement = CREATE_OBJECT(PragmaStatement);

	BlockPragma *pragma = pragmaStatement->pragma();

	pragma->blockProgramType(pragmaStatement_->pragma()->blockProgramType());
	pragma->isBlockBegin(pragmaStatement_->pragma()->isBlockBegin());

	// Inform the previous assignment statement in the block that it is the last statement
	if (pragma->blockProgramType() == bptResourceSharing && !pragma->isBlockBegin() && _currStatementList->back()->kind() == ktAssignmentStatement)
		((AssignmentStatement *)_currStatementList->back())->isLastResSharingStatement(true);

	return pragmaStatement;
}

VIR::RangeExpression *VElaborator::elaborateRangeExpression(VIR::RangeExpression *rangeExpression_)
{
	RangeExpression *rangeExpression = CREATE_OBJECT(RangeExpression);

	rangeExpression->begin(elaborateExpression(rangeExpression_->begin()));

	rangeExpression->increment(elaborateExpression(rangeExpression_->increment()));

	rangeExpression->end(elaborateExpression(rangeExpression_->end()));

	// Correct the increment if begin() is greater than end()
	if (rangeExpression->begin()->kind() == ktInteger && rangeExpression->end()->kind() == ktInteger &&
			rangeExpression->increment()->kind() == ktInteger && ((Integer *)rangeExpression->increment())->value > 0 &&
			((Integer *)rangeExpression->begin())->value > ((Integer *)rangeExpression->end())->value)
		((Integer *)rangeExpression->increment())->value *= -1;

	return rangeExpression;
}

VIR::ReferencePragma *VElaborator::elaborateReferencePragma(VIR::ReferencePragma *referencePragma_)
{
	ReferencePragma *referencePragma;

	switch (referencePragma_->kind())
	{
	case ktDPUChainPragma:
		referencePragma = CREATE_OBJECT(DPUChainPragma);
		break;
	case ktDPUPragma:
		referencePragma = CREATE_OBJECT(DPUPragma);
		break;
	case ktStoragePragma:
		referencePragma = CREATE_OBJECT(StoragePragma);
		break;
	}

	// Finding identifiers inside the expression list and setting their object declaration

	// IMPORTANT NOTE: here we are setting the object declarations of the Elaborated
	// VIR library for the identifiers in the original parser VIR library.

	vector<VIR::Identifier *> identifierList;
	Identifier *identifier;

	for (int rowIndex = 0; rowIndex < referencePragma_->reference().size(); ++rowIndex)
		findIdentifiersAndSliceNames(referencePragma_->reference()[rowIndex], &identifierList);

	for (int rowIndex = 0; rowIndex < identifierList.size(); ++rowIndex)
	{
		identifier = identifierList[rowIndex];
		identifier->object(getIdentifierObject(identifier->name()));
	}

	vector<VIR::Expression *> &referenceList = referencePragma->reference();

	// Elaborating the expressions in the reference list by replacing constants in the expressions
	for (int rowIndex = 0; rowIndex < referencePragma_->reference().size(); ++rowIndex)
		referenceList.push_back(_exprEvaluator->simplifyExpression(referencePragma_->reference()[rowIndex]));

	// The expression list should contain two expressions.
	// The first one is for the row address and the second one is for the column address.
	// The 3rd one is for the bank address considering sometimes there are more than 1 register files inside a DPU.
	Expression *rowAddress;
	Expression *colAddress;
	Expression *bankAddress;
	if (referenceList.size() == 3)
	{
		rowAddress = referenceList[0];
		colAddress = referenceList[1];
		bankAddress = referenceList[2];
	}
	else if (referenceList.size() == 2)
	{
		rowAddress = referenceList[0];
		colAddress = referenceList[1];
		bankAddress = 0;
	}
	else
	{
		LOG(FATAL) << ("elaborateReferencePragma: Wrong cell address!");
	}

	int rowBegin, rowEnd;
	int colBegin, colEnd;

	//! Yu Yang 2017-08-01
	// Change to solve compile error caused by passing reference of object in stack
	// CHANGE BEGIN
	std::map<VIR::ObjectDeclaration *, int> umv;

	// Finding the row address
	switch (rowAddress->kind())
	{
	case ktInteger:
		rowBegin = _exprEvaluator->evaluateExpressionToInt(rowAddress, 0, 0, 0, umv);
		rowEnd = rowBegin;
		break;

	case ktRangeExpression:
		rowBegin = _exprEvaluator->evaluateExpressionToInt(((RangeExpression *)rowAddress)->begin(), 0, 0, 0, umv);
		rowEnd = _exprEvaluator->evaluateExpressionToInt(((RangeExpression *)rowAddress)->end(), 0, 0, 0, umv);
		break;

	default:
		LOG(FATAL) << ("elaborateReferencePragma: Wrong cell address!");
		break;
	}

	// Finding the column address
	switch (colAddress->kind())
	{
	case ktInteger:
		colBegin = _exprEvaluator->evaluateExpressionToInt(colAddress, 0, 0, 0, umv);
		colEnd = colBegin;
		break;

	case ktRangeExpression:
		colBegin = _exprEvaluator->evaluateExpressionToInt(((RangeExpression *)colAddress)->begin(), 0, 0, 0, umv);
		colEnd = _exprEvaluator->evaluateExpressionToInt(((RangeExpression *)colAddress)->end(), 0, 0, 0, umv);
		break;

	default:
		LOG(FATAL) << ("elaborateReferencePragma: Wrong cell address!");
		break;
	}

	// CHANGE END

	for (int rowIndex = rowBegin; rowIndex <= rowEnd; ++rowIndex)
		for (int colIndex = colBegin; colIndex <= colEnd; ++colIndex)
			referencePragma->coordinates.push_back(Coordinate(rowIndex, colIndex));

	if (referencePragma->kind() == ktStoragePragma)
	{
		StoragePragma *oldStoragePragma = (StoragePragma *)referencePragma_;
		StoragePragma *storagePragma = (StoragePragma *)referencePragma;

		storagePragma->distribution(oldStoragePragma->distribution());
		storagePragma->storageType(oldStoragePragma->storageType());
		storagePragma->isForIndirectAddress(oldStoragePragma->isForIndirectAddress());
		storagePragma->variableType(oldStoragePragma->variableType());

		storagePragma->maxIndexes.resize(referencePragma->coordinates.size(), 0);
		storagePragma->startAddresses.resize(referencePragma->coordinates.size(), 0);
	}
	else if (referencePragma->kind() == ktDPUPragma || referencePragma->kind() == ktDPUChainPragma)
	{
		DPUPragma *oldDpuPragma = (DPUPragma *)referencePragma_;
		DPUPragma *dpuPragma = (DPUPragma *)referencePragma;

		dpuPragma->dpuMode = oldDpuPragma->dpuMode;
		dpuPragma->outPort = oldDpuPragma->outPort;
		dpuPragma->saturation = oldDpuPragma->saturation;
	}

	return referencePragma;
}

// This function detects that if the slice name is actually a function call, it creates a function call object instead.
VIR::Expression *VElaborator::elaborateSliceName(VIR::SliceName *sliceName_)
{
	string name = sliceName_->prefix()->name();

	// Detecting primitive functions
	if (isDpuPrimitiveFunction(name) || isAguPrimitiveFunction(name))
	{
		FunctionCall *function_call = CREATE_OBJECT(FunctionCall);
		function_call->_name = sliceName_->prefix();
		function_call->_parameters = sliceName_->suffix();
		return elaborateFunctionCall(function_call);
	}
	else
	{
		SliceName *sliceName = CREATE_OBJECT(SliceName);

		sliceName->prefix(elaborateIdentifier(sliceName_->prefix()));
		sliceName->isDummy(sliceName_->isDummy());

		elaborateExpressionList(&sliceName_->suffix(), &sliceName->suffix());

		return sliceName;
	}
}

VIR::Expression *VElaborator::elaborateFunctionCall(VIR::FunctionCall *functionCall_)
{

	string name = functionCall_->_name->name();

	// Detecting primitive functions
	if (isDpuPrimitiveFunction(name))
	{
		return elaborateDpuPrimitiveFunctionCall(functionCall_);
	}
	else if (isAguPrimitiveFunction(name))
	{
		return elaborateAguPrimitiveFunctionCall(functionCall_);
	}
	else
	{
		LOG(FATAL) << "Unreconganized primitive function call! Abort!";
		return NULL;
	}
}

VIR::Expression *VElaborator::elaborateDpuPrimitiveFunctionCall(VIR::FunctionCall *functionCall_)
{

	string name = functionCall_->_name->name();
	if (name == "sum")
	{
		return createAccumulatorFunction(functionCall_);
	}
	else
	{
		PrimitiveFunctionCall *primitivefunctionCall = CREATE_OBJECT(PrimitiveFunctionCall);
		primitivefunctionCall->_name = CREATE_IDENTIFIER(name);

		if (name == "max")
			primitivefunctionCall->type(ftMax);
		else if (name == "min")
			primitivefunctionCall->type(ftMin);
		else if (name == "zeros")
			primitivefunctionCall->type(ftZeros);
		else if (name == "ones")
			primitivefunctionCall->type(ftOnes);
		else if (name == "bitsll")
			primitivefunctionCall->type(ftShiftLeft);
		else if (name == "bitsra")
			primitivefunctionCall->type(ftShiftRight);
		else if (name == "silago_dpu_sigmoid")
			primitivefunctionCall->type(ftSigmoid);
		else if (name == "silago_dpu_tanh")
			primitivefunctionCall->type(ftTanh);
		else if (name == "fi")
			primitivefunctionCall->type(ftFixedPoint);
		else if (name == "silago_dpu_butterfly")
			primitivefunctionCall->type(ftButterfly);
		else
			primitivefunctionCall->type(ftUndefine);
		elaborateExpressionList(&(functionCall_->_parameters), &(primitivefunctionCall->_parameters));
		if (name == "fi")
			primitivefunctionCall->parametersSplit().push_back(primitivefunctionCall->parameters()[0]);
		else
			primitivefunctionCall->parametersSplit() = primitivefunctionCall->parameters();
		return primitivefunctionCall;
	}
}

VIR::Expression *VElaborator::elaborateAguPrimitiveFunctionCall(VIR::FunctionCall *functionCall_)
{
	if (functionCall_->_name->name() == "silago_agu_bitreverse")
	{
		PrimitiveFunctionCall *primitivefunctionCall = CREATE_OBJECT(PrimitiveFunctionCall);
		elaborateExpressionList(&(functionCall_->_parameters), &(primitivefunctionCall->_parameters));
		if (primitivefunctionCall->_parameters.size() == 2)
		{
			Name *s = dynamic_cast<Name *>(primitivefunctionCall->_parameters[0]);
			if (s)
			{
				Expression *e = _exprEvaluator->simplifyExpression(primitivefunctionCall->_parameters[1]);
				s->reverse_bits = e;
				return s;
			}
		}
	}
	if (functionCall_->_name->name() == "silago_agu_linear_1d")
	{
		PrimitiveFunctionCall *primitivefunctionCall = CREATE_OBJECT(PrimitiveFunctionCall);
		elaborateExpressionList(&(functionCall_->_parameters), &(primitivefunctionCall->_parameters));
		return functionCall_;
	}
	if (functionCall_->_name->name() == "silago_agu_linear_2d")
	{
		PrimitiveFunctionCall *primitivefunctionCall = CREATE_OBJECT(PrimitiveFunctionCall);
		elaborateExpressionList(&(functionCall_->_parameters), &(primitivefunctionCall->_parameters));
		return functionCall_;
	}

	LOG(FATAL) << "Unreconganized AGU primitive function, Abort!";
	return NULL;
}

VIR::Statement *VElaborator::elaborateStatement(VIR::Statement *statement_)
{
	WriteLineToF("Creating " + statement_->kindStr(), ftElaborator, true);

	Statement *statement;
	switch (statement_->kind())
	{
		CALL_STATEMENT_FUNC(AssignmentStatement);
		CALL_STATEMENT_FUNC(DebugStatement);
		CALL_STATEMENT_FUNC(ForStatement);
		CALL_STATEMENT_FUNC(IfStatement);
		CALL_STATEMENT_FUNC(IfThenElseStatement);
		CALL_STATEMENT_FUNC(PragmaStatement);
		CALL_STATEMENT_FUNC(FunctionCallStatement);
	}

	statement->controlDependentTo(_currControlStatement);

	return statement;
}

void VElaborator::elaborateStatementList(vector<VIR::Statement *> *oldStatementList_, vector<VIR::Statement *> *newStatementList_)
{
	const int size = oldStatementList_->size();

	_currStatementList = newStatementList_;

	for (int index = 0; index < size; ++index)
	{
		Statement *statement = elaborateStatement((*oldStatementList_)[index]);
		newStatementList_->push_back(statement);

		if (!isControlStatement(statement))
			addToTotalStatementList(statement);
	}
}

VIR::StringLiteral *VElaborator::elaborateStringLiteral(VIR::StringLiteral *stringLiteral_)
{
	StringLiteral *stringLiteral = CREATE_OBJECT(StringLiteral);

	stringLiteral->value = stringLiteral_->value;

	return stringLiteral;
}

VIR::Expression *VElaborator::elaborateUnaryExpression(VIR::UnaryExpression *unaryExpression_)
{
	UnaryExpression *unaryExpression = CREATE_OBJECT(UnaryExpression);

	unaryExpression->operatorType(unaryExpression_->operatorType());
	unaryExpression->operand(elaborateExpression(unaryExpression_->operand()));

	// If its a negative number, just return a interger or float object
	if (unaryExpression->operatorType() == VIR::VirEnumerations::uotMinus)
	{
		if (unaryExpression->operand()->kind() == VIR::VirEnumerations::ktInteger)
		{
			static_cast<Integer *>(unaryExpression->operand())->value = -static_cast<Integer *>(unaryExpression->operand())->value;
			return unaryExpression->operand();
		}
		else if (unaryExpression->operand()->kind() == VIR::VirEnumerations::ktFloatingPoint)
		{
			static_cast<FloatingPoint *>(unaryExpression->operand())->value = -static_cast<FloatingPoint *>(unaryExpression->operand())->value;
			return unaryExpression->operand();
		}
		else if (unaryExpression->operand()->kind() == VIR::VirEnumerations::ktComplex)
		{
			static_cast<Complex *>(unaryExpression->operand())->rValue = -static_cast<Complex *>(unaryExpression->operand())->rValue;
			static_cast<Complex *>(unaryExpression->operand())->iValue = -static_cast<Complex *>(unaryExpression->operand())->iValue;
			return unaryExpression->operand();
		}
	}

	return unaryExpression;
}

// -------------------------------------
// ------- Other Private Methods -------

// This method adds the given statement to the list of total statements.
void VElaborator::addToTotalStatementList(VIR::Statement *statement_)
{
	_totalAssignmentStatementList->push_back(statement_);
	statement_->statementNo(_currStatementNo++);
}

// This method creates an accumulator function (MAC, Symmetric MAC, SubtractAcc, AbsoluteSubtractAdd)
// according to the given slice name in the form of sum(A .* B), sum(A .* (B + C)), sum(A - B), sum(abs(A - B))
VIR::PrimitiveFunctionCall *VElaborator::createAccumulatorFunction(VIR::FunctionCall *functionCall_)
{
	const string invalidArgErrorMessage = "elaborateSliceName: Unknown arguments of the sum function!";

	if (functionCall_->_parameters.size() != 1)
		LOG(FATAL) << (invalidArgErrorMessage);

	PrimitiveFunctionCall *accFunction = CREATE_OBJECT(PrimitiveFunctionCall);

	Expression *suffix = elaborateExpression(functionCall_->_parameters[0]);

	accFunction->parameters().push_back(suffix);
	accFunction->name(CREATE_IDENTIFIER("sum"));

	BinaryExpression *binaryExpr = 0;

	bool hasAbsolute = false;

	if (suffix->kind() == ktBinaryExpression)
		binaryExpr = (BinaryExpression *)suffix;
	else if (suffix->kind() == ktSliceName) // case of AbsoluteSubtractAdd
	{
		SliceName *sliceName = dynamic_cast<SliceName *>(suffix);

		if (!sliceName || sliceName->prefix()->name() != "abs")
			LOG(FATAL) << (invalidArgErrorMessage);

		binaryExpr = dynamic_cast<BinaryExpression *>(sliceName->suffix()[0]);
		hasAbsolute = true;

		if (!binaryExpr)
			LOG(FATAL) << (invalidArgErrorMessage);
	}
	else if (suffix->kind() == ktPrimitiveFunctionCall && ((PrimitiveFunctionCall *)suffix)->type() == ftFixedPoint)
	{
		binaryExpr = dynamic_cast<BinaryExpression *>(((PrimitiveFunctionCall *)suffix)->parameters()[0]);

		if (!binaryExpr)
			LOG(FATAL) << (invalidArgErrorMessage);
	}
	else
		LOG(FATAL) << (invalidArgErrorMessage);

	vector<Expression *> &functionParameters = accFunction->parametersSplit();

	// The ACC operator should be the MATLAB element-wise multiplication operator(.*) or subtraction(-).
	if (!binaryExpr || !(binaryExpr->operatorType() == botEMul || binaryExpr->operatorType() == botSub))
		LOG(FATAL) << (invalidArgErrorMessage);

	if (binaryExpr->leftOperand()->kind() == ktIdentifier || binaryExpr->leftOperand()->kind() == ktSliceName)
		functionParameters.push_back(binaryExpr->leftOperand()); // First Operand of SubAcc, MAC or Symmetric MAC
	else
		LOG(FATAL) << (invalidArgErrorMessage);

	// Case of simple MAC operation or SubtractAcc or AbsoluteSubtractAdd
	if (binaryExpr->rightOperand()->kind() == ktIdentifier || binaryExpr->rightOperand()->kind() == ktSliceName)
	{
		functionParameters.push_back(binaryExpr->rightOperand()); // Second Operand

		if (binaryExpr->operatorType() == botEMul)
			accFunction->type(ftMac);
		else if (binaryExpr->operatorType() == botSub)
			accFunction->type(hasAbsolute ? ftAbsSubAcc : ftSubAcc);
	}
	// Case of symmetric MAC operation
	else if (binaryExpr->rightOperand()->kind() == ktParenthesizedExpression &&
					 ((ParenthesizedExpression *)binaryExpr->rightOperand())->value()->kind() == ktBinaryExpression)
	{
		BinaryExpression *rightBinaryExpr = (BinaryExpression *)((ParenthesizedExpression *)binaryExpr->rightOperand())->value();

		if (rightBinaryExpr->operatorType() != botAdd ||
				(rightBinaryExpr->leftOperand()->kind() != ktIdentifier && rightBinaryExpr->leftOperand()->kind() != ktSliceName) ||
				(rightBinaryExpr->rightOperand()->kind() != ktIdentifier && rightBinaryExpr->rightOperand()->kind() != ktSliceName))
			LOG(FATAL) << (invalidArgErrorMessage);

		functionParameters.push_back(rightBinaryExpr->leftOperand());	// Second Operand of Symmetric MAC
		functionParameters.push_back(rightBinaryExpr->rightOperand()); // Third Operand of Symmetric MAC

		accFunction->type(ftSymmetricMac);
	}
	else
		LOG(FATAL) << (invalidArgErrorMessage);

	// change the name of function based on its type
	switch (accFunction->type())
	{
	case ftMac:
		accFunction->primitive_func_name = "silago_dpu_mac";
		break;
	case ftAbsSubAcc:
		accFunction->primitive_func_name = "silago_dpu_abs_sub_acc";
		break;
	case ftSubAcc:
		accFunction->primitive_func_name = "silago_dpu_sub_acc";
		break;
	case ftSymmetricMac:
		accFunction->primitive_func_name = "silago_dpu_symmetric_mac";
		break;
	default:
		LOG(FATAL) << "Unknown accumulative function type";
		break;
	}
	return accFunction;
}

// This method creates a slice name and a dummy variable for a constant integer inside an assignment statement or if condition
VIR::SliceName *VElaborator::createSliceNameAndDummyVarForAConstant(int constantValue_, VIR::DPUPragma *assignPragma_)
{
	// Creating dummy assignment in the form of #var_id = constantValue_ %! RFILE<> [x:y]
	AssignmentStatement *dummyAssignStatement = CREATE_OBJECT(AssignmentStatement);

	Identifier *lhsName = CREATE_IDENTIFIER(getUniqueDummyName());
	Array *rhsArray = CREATE_OBJECT(Array);
	StoragePragma *storagePragma = CREATE_OBJECT(StoragePragma);

	rhsArray->elements().push_back(CREATE_INTEGER(constantValue_));

	storagePragma->storageType(stRegFile);
	storagePragma->distribution(sdEvenDist);
	storagePragma->reference().push_back(CREATE_INTEGER(assignPragma_->coordinates[0].row));
	storagePragma->reference().push_back(CREATE_INTEGER(assignPragma_->coordinates[0].column));

	dummyAssignStatement->lhs().push_back(lhsName);
	dummyAssignStatement->rhs(rhsArray);
	dummyAssignStatement->pragma(storagePragma);

	dummyAssignStatement = elaborateAssignmentStatement(dummyAssignStatement);

	addToTotalStatementList(dummyAssignStatement);

	if (!dummyAssignStatement->lhsIdentifiers()[0]->object())
		LOG(FATAL) << ("createSliceNameAndDummyVarForAConstant: (internal error): object should not be empty!");

	SliceName *sliceName = CREATE_OBJECT(SliceName);
	sliceName->prefix(CREATE_IDENTIFIER(lhsName->name()));
	sliceName->suffix().push_back(CREATE_INTEGER(constantValue_));
	sliceName->isDummy(true);

	return (SliceName *)elaborateSliceName(sliceName);
}

// This method creates a slice name for an identifier inside an arithmetic assignment statement.
// It is used when there is a vector operation and the identifiers should be converted to
// slice names with range expression in the suffix.
SliceName *VElaborator::createSliceNameForAnIdentifier(VIR::Identifier *identifier_)
{
	SliceName *sliceName = CREATE_OBJECT(SliceName);
	Expression *suffix = 0;

	if (identifier_->object() && !identifier_->object()->value())
		suffix = CREATE_INTEGER(1);
	else if (identifier_->object() && identifier_->object()->value() && identifier_->object()->value()->kind() == ktArray)
	{
		Array *objectValue = (Array *)identifier_->object()->value();

		Expression *suffixExpr;

		if (objectValue->elements().size() == 1)
		{
			Expression *elementExpr = objectValue->elements()[0];

			if (elementExpr->kind() == ktInteger)
				suffixExpr = CREATE_INTEGER(1);
			else if (elementExpr->kind() == ktRangeExpression)
			{
				RangeExpression *objectRangeExpr = (RangeExpression *)objectValue->elements()[0];

				if (objectRangeExpr->begin()->kind() != ktInteger || objectRangeExpr->increment()->kind() != ktInteger ||
						objectRangeExpr->end()->kind() != ktInteger)
					LOG(FATAL) << ("createSliceNameForAnIdentifier: Unsupported object value!");

				int endRange = (((Integer *)objectRangeExpr->end())->value - ((Integer *)objectRangeExpr->begin())->value) / ((Integer *)objectRangeExpr->increment())->value + 1;

				suffixExpr = CREATE_RANGE_EXPRESSION(1, 1, endRange);
			}
			else
				LOG(FATAL) << ("createSliceNameForAnIdentifier: Unsupported object value!");
		}
		else if (objectValue->elements().size() > 1)
		{
			suffixExpr = CREATE_RANGE_EXPRESSION(1, 1, objectValue->elements().size());
		}
		else
			LOG(FATAL) << ("createSliceNameForAnIdentifier: Unsupported object value!");

		suffix = suffixExpr;
	}
	else
		LOG(FATAL) << ("createSliceNameForAnIdentifier: Improper object value!");

	sliceName->prefix(identifier_);
	sliceName->suffix().push_back(suffix);

	sliceName->reverse_bits = identifier_->reverse_bits;

	return sliceName;
}

// This method gets assignment statement and finds its index dependencies
// to previous assignments or loop variables.
void VElaborator::findAssignStatementIndexDependencies(AssignmentStatement *assignmentStatement_)
{
	vector<VIR::Identifier *> indexIdentifiers;
	if (assignmentStatement_->lhs().size() > 0)
	{
		ObjectType objectType = assignmentStatement_->lhsIdentifiers()[0]->object()->objectType();

		// In case of address functions we should find the index dependencies for the right hand side
		if (objectType == otAffineAddressFunction || objectType == otNonAffineAddressFunction || objectType == otRaccuVariable || objectType == otAddressVariable)
		{
			findIndexIdentifiers(assignmentStatement_->rhs(), &indexIdentifiers, true, false);
		}
		else
		{
			if (objectType != otDPUChain)
			{
				for (auto &sn : assignmentStatement_->lhsSliceNames())
				{
					findIndexIdentifiers(sn, &indexIdentifiers);
				}
			}
			findIndexIdentifiers(assignmentStatement_->rhs(), &indexIdentifiers);
		}
	}
	else
	{
		findIndexIdentifiers(assignmentStatement_->rhs(), &indexIdentifiers);
	}

	setIndexDependencies(indexIdentifiers);
}

// This method gets the LHS and RHS identifiers of an assignment and
// finds their data dependencies to previous assignments
void VElaborator::findDataDependencies(VIR::Identifier *lhsIdentifier_, vector<VIR::Identifier *> *rhsIdentifiers_)
{
	// For all RHS identifiers, find RAW data dependency to previous assignment statements
	for (int idIndex = 0; idIndex < rhsIdentifiers_->size(); ++idIndex)
	{
		Identifier *identifier = (*rhsIdentifiers_)[idIndex];

		// Using a reverse loop, only the closest dependent statement is detected.
		for (int stIndex = _currStatementList->size() - 1; stIndex >= 0; --stIndex)
		{
			Statement *statement = (*_currStatementList)[stIndex];

			if (statement->kind() == ktAssignmentStatement)
			{
				AssignmentStatement *prevAssignmentStatement = (AssignmentStatement *)statement;

				// If a RAW data dependency has been detected, set the dependency and exit the loop
				bool flag = false;
				for (auto &id : prevAssignmentStatement->lhsIdentifiers())
				{
					if (prevAssignmentStatement->type() != atDeclarative && prevAssignmentStatement->type() != atIndirectAddressDeclaration &&
							id->object() == identifier->object())
					{
						identifier->dependentTo(prevAssignmentStatement);
						identifier->dependencyType(dtReadAfterWrite);
						flag = true;
						break;
					}
				}
				if (flag)
				{
					break;
				}
			}
		}
	}

	// For the LHS identifier and using a reverse loop, find all WAW and WAR data dependencies
	for (int stIndex = _currStatementList->size() - 1; stIndex >= 0; --stIndex)
	{
		Statement *statement = (*_currStatementList)[stIndex];

		if (statement->kind() == ktAssignmentStatement)
		{
			AssignmentStatement *prevAssignmentStatement = (AssignmentStatement *)statement;

			if (prevAssignmentStatement->type() != atDeclarative && prevAssignmentStatement->type() != atIndirectAddressDeclaration)
			{
				// WAW data dependency
				bool flag = false;
				for (auto &id : prevAssignmentStatement->lhsIdentifiers())
				{
					if (id->object() == lhsIdentifier_->object())
					{
						lhsIdentifier_->dependentTo(prevAssignmentStatement);
						lhsIdentifier_->dependencyType(dtWriteAfterWrite);
						flag = true;
						break;
					}
				}
				if (flag)
				{
					break;
				}

				// WAR data dependency
				for (int stIdIndex = 0; stIdIndex < prevAssignmentStatement->rhsIdentifiers().size(); ++stIdIndex)
				{
					if (prevAssignmentStatement->rhsIdentifiers()[stIdIndex]->object() == lhsIdentifier_->object())
					{
						lhsIdentifier_->dependentTo(prevAssignmentStatement);
						lhsIdentifier_->dependencyType(dtWriteAfterRead);
						break;
					}
				}
			}
		}
	}
}

// This method finds the for statement that the given statement is dependent to.
VIR::ForStatement *VElaborator::findDependentForStatement(VIR::Statement *currStatement_)
{
	ControlStatement *loopStatement = dynamic_cast<ControlStatement *>(currStatement_->controlDependentTo());

	while (loopStatement)
	{
		if (loopStatement->kind() == ktForStatement)
			break;
		else
			loopStatement = dynamic_cast<ControlStatement *>(loopStatement->controlDependentTo());
	}

	return (VIR::ForStatement *)loopStatement;
}

// This method finds all identifiers inside an expression and insert all of them into the given identifier list.
// Also if the argument of rhsSliceNames_ is not zero, this method searches for slice names and adds them to this list.
void VElaborator::findIdentifiersAndSliceNames(VIR::Expression *expression_, vector<VIR::Identifier *> *rhsIdentifiers_, vector<VIR::SliceName *> *rhsSliceNames_)
{
	switch (expression_->kind())
	{
	case ktArray:
	{
		vector<VIR::Expression *> *exprList = &((Array *)expression_)->elements();

		for (int i = 0; i < exprList->size(); ++i)
			findIdentifiersAndSliceNames((*exprList)[i], rhsIdentifiers_, rhsSliceNames_);
	}
	break;

	case ktBinaryExpression:
		findIdentifiersAndSliceNames(((BinaryExpression *)expression_)->leftOperand(), rhsIdentifiers_, rhsSliceNames_);
		findIdentifiersAndSliceNames(((BinaryExpression *)expression_)->rightOperand(), rhsIdentifiers_, rhsSliceNames_);
		break;

	case ktIdentifier:
		rhsIdentifiers_->push_back((Identifier *)expression_);
		break;

	case ktParenthesizedExpression:
		findIdentifiersAndSliceNames(((ParenthesizedExpression *)expression_)->value(), rhsIdentifiers_, rhsSliceNames_);
		break;

	case ktPrimitiveFunctionCall:
	{
		vector<VIR::Expression *> &functionParameters = ((PrimitiveFunctionCall *)expression_)->parametersSplit();

		for (int i = 0; i < functionParameters.size(); ++i)
			findIdentifiersAndSliceNames(functionParameters[i], rhsIdentifiers_, rhsSliceNames_);
	}
	break;

	case ktRangeExpression:
		findIdentifiersAndSliceNames(((RangeExpression *)expression_)->begin(), rhsIdentifiers_, rhsSliceNames_);
		findIdentifiersAndSliceNames(((RangeExpression *)expression_)->end(), rhsIdentifiers_, rhsSliceNames_);
		findIdentifiersAndSliceNames(((RangeExpression *)expression_)->increment(), rhsIdentifiers_, rhsSliceNames_);
		break;

	case ktSliceName:
	{
		SliceName *sliceName = (SliceName *)expression_;

		if (rhsSliceNames_)
			rhsSliceNames_->push_back(sliceName);

		rhsIdentifiers_->push_back(sliceName->prefix());

		// The identifiers in the suffix part will be handled in findIndexDependencies() method
		//for (int i = 0; i < sliceName->suffix().size(); ++i)
		//	findIdentifiers(sliceName->suffix()[i], rhsIdentifiers_);
	}
	break;

	case ktUnaryExpression:
		findIdentifiersAndSliceNames(((UnaryExpression *)expression_)->operand(), rhsIdentifiers_, rhsSliceNames_);
		break;

	case ktFunctionCall:
	case ktComplex:
	case ktFloatingPoint:
	case ktInteger:
	case ktStringLiteral:
		break; // no need to do anything
	}
}

// This method gets an if statement and finds the index dependencies of its condition
// to previous assignments or loop variables.
void VElaborator::findIfConditionIndexDependencies(VIR::IfStatement *ifStatement_)
{
	vector<VIR::Identifier *> indexIdentifiers;

	findIndexIdentifiers(ifStatement_->condition(), &indexIdentifiers);

	// For all identifiers in the indices of the if condition find the corresponding dependency
	// (either to a loop variable or to an index computation assignment statement)
	for (int idIndex = 0; idIndex < indexIdentifiers.size(); ++idIndex)
	{
		Identifier *identifier = indexIdentifiers[idIndex];

		// Using a reverse loop on all previous statements, find the closest dependent statement
		// (a loop statement or an assignment statement).
		for (int stIndex = _totalAssignmentStatementList->size() - 1; stIndex >= 0; --stIndex)
		{
			Statement *statement = (*_totalAssignmentStatementList)[stIndex];

			if (statement->kind() == ktForStatement && ((ForStatement *)statement)->loopVariable()->object() == identifier->object())
			{
				identifier->indexDependency(statement);
				break;
			}
			else if (statement->kind() == ktAssignmentStatement)
			{
				bool flag = false;
				for (auto &id : ((AssignmentStatement *)statement)->lhsIdentifiers())
				{
					if (id->object() == identifier->object())
					{
						identifier->indexDependency(statement);
						flag = true;
					}
				}
				if (flag)
				{
					break;
				}
			}
		}

		// If no corresponding statement found for the index dependency, raise an error for that.
		if (identifier->indexDependency() == 0)
			LOG(FATAL) << ("Cannot find the index dependency!");
	}
}

// This method finds a list of identifiers that has been used in indices (suffix of slice names)
void VElaborator::findIndexIdentifiers(VIR::Expression *expression_, vector<VIR::Identifier *> *indexIdentifiers_, bool isAddressFunction_, bool isFromSliceName_)
{

	switch (expression_->kind())
	{
	case ktBinaryExpression:
		findIndexIdentifiers(((BinaryExpression *)expression_)->leftOperand(), indexIdentifiers_, isAddressFunction_, isFromSliceName_);
		findIndexIdentifiers(((BinaryExpression *)expression_)->rightOperand(), indexIdentifiers_, isAddressFunction_, isFromSliceName_);
		break;

	case ktIdentifier:
		if (isFromSliceName_ || isAddressFunction_)
			indexIdentifiers_->push_back((Identifier *)expression_);
		break;

	case ktParenthesizedExpression:
		findIndexIdentifiers(((ParenthesizedExpression *)expression_)->value(), indexIdentifiers_, isAddressFunction_, isFromSliceName_);
		break;

	case ktPrimitiveFunctionCall:
	{
		vector<VIR::Expression *> &functionParameters = ((PrimitiveFunctionCall *)expression_)->parametersSplit();

		for (int i = 0; i < functionParameters.size(); ++i)
			findIndexIdentifiers(functionParameters[i], indexIdentifiers_, isAddressFunction_, isFromSliceName_);
	}
	break;

	case ktRangeExpression:
	{
		findIndexIdentifiers(((RangeExpression *)expression_)->begin(), indexIdentifiers_, isAddressFunction_, isFromSliceName_);
		findIndexIdentifiers(((RangeExpression *)expression_)->increment(), indexIdentifiers_, isAddressFunction_, isFromSliceName_);
		findIndexIdentifiers(((RangeExpression *)expression_)->end(), indexIdentifiers_, isAddressFunction_, isFromSliceName_);
	}
	break;

	case ktSliceName:
	{
		SliceName *sliceName = (SliceName *)expression_;

		for (int i = 0; i < sliceName->suffix().size(); ++i)
			findIndexIdentifiers(sliceName->suffix()[i], indexIdentifiers_, isAddressFunction_, true);
	}
	break;

	case ktUnaryExpression:
		findIndexIdentifiers(((UnaryExpression *)expression_)->operand(), indexIdentifiers_, isAddressFunction_, isFromSliceName_);
		break;

	case ktArray:
	case ktComplex:
	case ktFloatingPoint:
	case ktFunctionCall:
	case ktInteger:
	case ktStringLiteral:
		break; // no need to do anything
	}
}

// This method gets the range of a loop statement and finds its index dependencies
// to previous address functions or loop variables.
void VElaborator::findLoopRangeIndexDependencies(VIR::RangeExpression *loopRangeExpression_)
{
	vector<VIR::Identifier *> indexIdentifiers;

	findIndexIdentifiers(loopRangeExpression_, &indexIdentifiers, true);
	setIndexDependencies(indexIdentifiers);
}

// This method tries to find an object declaration for the given identifier.
// Otherwise it creates an object declaration and adds it to the identifiers map
VIR::ObjectDeclaration *VElaborator::getIdentifierObject(const std::string &name_, bool isLoopIndex_ /*= false*/)
{
	ObjectDeclaration *objectDeclaration;
	IdentifiersMap::iterator iter;

	iter = _identifiersMap.find(name_);

	if (iter != _identifiersMap.end())
		objectDeclaration = iter->second;
	else
	{
		if (!isLoopIndex_ && !_isAddressVariable)
			objectDeclaration = CREATE_OBJECT(ObjectDeclaration);
		else if (_isAddressVariable)
			objectDeclaration = CREATE_OBJECT(AddressObjectDeclaration);
		else
			objectDeclaration = CREATE_OBJECT(LoopObjectDeclaration);

		objectDeclaration->name(name_);
		objectDeclaration->value(0);

		_identifiersMap[name_] = objectDeclaration;
	}

	return objectDeclaration;
}

// This method takes the current for statement that has non-constant boundary or is in a non-constant region and
// gets the unrolled loops surrounding the region and a signature list for all possible unrolled values.
void VElaborator::getUnrolledLoopsAndSignatures(VIR::ForStatement *currForStatement_, UnrolledLoops &unrolledLoops_, vector<string> &unrolledSignatures_)
{
	struct LoopInfo
	{
		int currValue;
		int beginValue;
		int stepValue;
		int endValue;
	};

	vector<LoopInfo> loopValues;
	ForStatement *forStatement = currForStatement_;

	// Determining the loops that are going to be unrolled
	while (forStatement)
	{
		if (forStatement->degree() > 2)
			unrolledLoops_.push_back(forStatement);

		do
		{
			forStatement = (ForStatement *)forStatement->controlDependentTo();
		} while (forStatement && forStatement->kind() != ktForStatement);
	}

	const int loopsNumber = unrolledLoops_.size();

	for (int i = 0; i < loopsNumber; ++i)
	{
		LoopInfo loopInfo;

		int firstValue = ((Integer *)unrolledLoops_[i]->loopRange()->begin())->value;
		int secondValue = ((Integer *)unrolledLoops_[i]->loopRange()->end())->value;

		loopInfo.beginValue = firstValue <= secondValue ? firstValue : secondValue;
		loopInfo.stepValue = abs(((Integer *)unrolledLoops_[i]->loopRange()->increment())->value);
		loopInfo.endValue = firstValue > secondValue ? firstValue : secondValue;
		loopInfo.currValue = loopInfo.beginValue;

		loopValues.push_back(loopInfo);
	}

	string signature;
	LoopInfo &innerLoop = loopValues[0];

	// Implementing permutation with repetition for all unrolled loop values.
	// After this loop, unrolledSignatures contains a list of signatures for all possible loop values.
	while (loopValues[loopsNumber - 1].currValue <= loopValues[loopsNumber - 1].endValue)
	{
		for (int currValue = innerLoop.beginValue; currValue <= innerLoop.endValue; currValue += innerLoop.stepValue)
		{
			innerLoop.currValue = currValue;
			signature = "";

			for (int i = loopsNumber - 1; i >= 0; --i)
				signature += to_string(loopValues[i].currValue) + (i != 0 ? "_" : "");

			unrolledSignatures_.push_back(signature);
		}

		innerLoop.currValue += innerLoop.stepValue;

		for (int i = 0; i < loopsNumber - 1; ++i)
		{
			if (loopValues[i].currValue == loopValues[i].endValue)
			{
				loopValues[i].currValue = loopValues[i].beginValue;
				loopValues[i + 1].currValue += loopValues[i + 1].stepValue;
			}
		}
	}
}

// This method gets a unique name for creating dummy variables in the form of #var_id.
string VElaborator::getUniqueDummyName() const
{
	static int id = 0;

	return "#var_" + to_string(id++);
}

// This method determines if the given statement is a control statement or not.
bool VElaborator::isControlStatement(const VIR::Statement *statement_) const
{
	KindType kind = statement_->kind();
	return (kind == ktForStatement || kind == ktIfStatement || kind == ktIfThenElseStatement) ? true : false;
}

// This method returns true if the given name is one of the MATLAB primitive functions.
/**
 * \note Function name start with "silago_" will be always consider as a SiLago primitive function. It usually
 * corresponding to a specific DPU mode and its matlab implementation should be find in the
 * given SiLago library.
 */
bool VElaborator::isDpuPrimitiveFunction(string name_) const
{
	if (
			name_ == "min" ||
			name_ == "max" ||
			name_ == "ones" ||
			name_ == "zeros" ||
			name_ == "sum" ||
			name_ == "bitsll" ||
			name_ == "bitsra" ||
			name_ == "fi" ||
			strncmp(name_.c_str(), "silago_dpu_", strlen("silago_dpu_")) == 0)
	{
		return true;
	}
	return false;
}

bool VElaborator::isAguPrimitiveFunction(const string name_) const
{
	if (strncmp(name_.c_str(), "silago_agu_", strlen("silago_agu_")) == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// This method gets an assignment statement and for both lhs and rhs, it replaces wherever
// there are identifiers corresponding to arrays, with slice names.
void VElaborator::replaceIdentifiersWithSliceNames(VIR::AssignmentStatement *assignmentStatement_)
{
	assignmentStatement_->rhs(replaceIdentifiersWithSliceNames(assignmentStatement_, assignmentStatement_->rhs()));

	for (int i = 0; i < assignmentStatement_->lhs().size(); i++)
	{
		if (assignmentStatement_->lhs()[i]->kind() == ktIdentifier && ((Identifier *)(assignmentStatement_->lhs()[i]))->object()->objectType() == otRegisterFile)
		{
			SliceName *sliceName = createSliceNameForAnIdentifier((Identifier *)(assignmentStatement_->lhs()[i]));
			assignmentStatement_->lhs()[i] = sliceName; // Replacing the lhs identifier with the new slice name
		}
	}
}

// This method overloads the original method for a recursive behavior over rhs expressions of an assignment statement.
VIR::Expression *VElaborator::replaceIdentifiersWithSliceNames(VIR::AssignmentStatement *assignmentStatement_, VIR::Expression *expression_)
{
	Expression *resultExpr = expression_;

	switch (expression_->kind())
	{
	case ktPrimitiveFunctionCall:
	{
		vector<VIR::Expression *> &functionParameters = ((PrimitiveFunctionCall *)expression_)->parametersSplit();

		for (int exprIndex = 0; exprIndex < functionParameters.size(); ++exprIndex)
		{
			Expression *currParameter = functionParameters[exprIndex];

			if (currParameter->kind() == ktIdentifier && ((Identifier *)currParameter)->object()->kind() != ktLoopObjectDeclaration)
				functionParameters[exprIndex] = replaceIdentifiersWithSliceNames(assignmentStatement_, currParameter);
		}
	}
	break;

	case ktBinaryExpression:
	{
		BinaryExpression *binaryExpression = (BinaryExpression *)expression_;

		binaryExpression->leftOperand(replaceIdentifiersWithSliceNames(assignmentStatement_, binaryExpression->leftOperand()));
		binaryExpression->rightOperand(replaceIdentifiersWithSliceNames(assignmentStatement_, binaryExpression->rightOperand()));
	}
	break;

	// In case of vector operations, identifiers should be converted to slice names
	case ktIdentifier:
		if (((Identifier *)expression_)->object()->objectType() == otRegisterFile)
		{
			resultExpr = createSliceNameForAnIdentifier((Identifier *)expression_);
			assignmentStatement_->rhsSliceNames().push_back((SliceName *)resultExpr);
		}
		else
		{
			resultExpr = expression_;
		}
		break;

	case ktFloatingPoint:
	case ktInteger:
		// It is fine. Do nothing.
		/* //REMOVE THIS PART AFTER REGRESSION TEST
			// In case of integer constants, they should be converted to slice names by creating dummy variables
			resultExpr = createSliceNameAndDummyVarForAConstant(((Integer *) expression_)->value, (DPUPragma *) assignmentStatement_->pragma());
			assignmentStatement_->rhsSliceNames().push_back((SliceName *) resultExpr);
			*/
		break;

	case ktSliceName:
		// It is fine. Do nothing.
		break;

	default:
		LOG(FATAL) << ("replaceIdentifiersWithSliceNames: Unsupported operation!");
		break;
	}

	return resultExpr;
}

// This method checks whether the RHS part of the assignment is constant and in that case
// it sets the value of the corresponding object of the LHS identifier to the RHS value.
// It returns true if the evaluated value is constant
bool VElaborator::setConstantObjectValue(VIR::Identifier *lhsIdentifier_, vector<VIR::Identifier *> *rhsIdentifiers_, VIR::Expression *rhs_)
{
	bool isRHSConstant = true;

	for (int idIndex = 0; idIndex < rhsIdentifiers_->size(); ++idIndex)
	{
		if ((*rhsIdentifiers_)[idIndex]->object()->objectType() != otConstant)
		{
			isRHSConstant = false;
			break;
		}
	}

	if (isRHSConstant)
	{
		NumericLiteral *simplifiedValue = dynamic_cast<NumericLiteral *>(_exprEvaluator->simplifyExpression(rhs_));

		if (!simplifiedValue)
			LOG(FATAL) << ("The value is not numerical!");

		// Set fixed-point flag of the object, if we are using 'fi' function in the declaration.
		if (rhs_->kind() == ktPrimitiveFunctionCall && ((PrimitiveFunctionCall *)rhs_)->type() == ftFixedPoint)
			lhsIdentifier_->object()->isFixedPointVariable(true);

		lhsIdentifier_->object()->value(simplifiedValue);
	}

	return isRHSConstant;
}

// This method sets the DPU mode for an assignment statement or an if condition to the corresponding dpu pragma
void VElaborator::setDpuMode(VIR::Expression *expression_, VIR::DPUPragma *dpuPragma_)
{
	string &dpuMode = dpuPragma_->dpuMode;
	switch (expression_->kind())
	{
	case ktBinaryExpression:
	{
		BinaryExpression *binaryExpression = (BinaryExpression *)expression_;

		if (binaryExpression->rightOperand()->kind() == ktPrimitiveFunctionCall &&
				((PrimitiveFunctionCall *)binaryExpression->rightOperand())->type() == ftMac)
		{
			((PrimitiveFunctionCall *)binaryExpression->rightOperand())->type(ftPreservingMac);
			dpuMode = "silago_dpu_preserving_mac";
		}
		else if (dpuMode == "" || dpuMode.empty())
		{

			switch (binaryExpression->operatorType())
			{
			case botSub:
				dpuMode = "silago_dpu_sub";
				break;
			case botAdd:
				dpuMode = binaryExpression->rightOperand()->kind() != ktInteger &&
													binaryExpression->rightOperand()->kind() != ktFloatingPoint &&
													binaryExpression->rightOperand()->kind() != ktPrimitiveFunctionCall
											? // The case of using fi() function
											"silago_dpu_add"
											: "silago_dpu_add_with_constant";
				break;

			case botEMul:
			case botMMul:
				dpuMode = "silago_dpu_mult";
				break;

			case botEqual:
			case botGreaterThan:
			case botGreaterOrEqual:
			case botLessThan:
			case botLessOrEqual:
			case botUnEqual:
				dpuMode = "silago_dpu_comparison";
				break;

			default:
				LOG(FATAL) << ("setDpuMode: Unsupported operation!");
				break;
			}
		}
	}
	break;

	case ktFloatingPoint:
	case ktInteger:
		dpuMode = "silago_dpu_load_constant";
		break;

	case ktPrimitiveFunctionCall:
	{
		PrimitiveFunctionCall *primitiveFunctionCall = (PrimitiveFunctionCall *)expression_;

		switch (primitiveFunctionCall->type())
		{
		case ftMax:
			dpuMode = "silago_dpu_max";
			break;
		case ftMin:
			dpuMode = "silago_dpu_min";
			break;
		case ftMac:
			dpuMode = "silago_dpu_mac";
			break;
		case ftSymmetricMac:
			dpuMode = "silago_dpu_sym_mac";
			break;
		case ftSubAcc:
			dpuMode = "silago_dpu_sub_acc";
			break;
		case ftAbsSubAcc:
			dpuMode = "silago_dpu_abs_sub_acc";
			break;
		case ftFixedPoint:
			dpuMode = "silago_dpu_load_constant";
			break;
		case ftShiftLeft:
			dpuMode = "silago_dpu_shift_left";
			break;
		case ftShiftRight:
			dpuMode = "silago_dpu_shift_right";
			break;

		default:
			dpuMode = primitiveFunctionCall->name()->name();
			break;
		}
	}
	break;

	default:
		LOG(FATAL) << ("setDpuMode: Unsupported operation!");
		break;
	}
}

// This method sets the index dependencies for the identifiers inside the given list.
void VElaborator::setIndexDependencies(vector<VIR::Identifier *> &indexIdentifiers_)
{
	// For all identifiers in the indices of the assignment statement find the corresponding dependency
	// (either to a loop variable or to an index computation assignment statement)
	for (int idIndex = 0; idIndex < indexIdentifiers_.size(); ++idIndex)
	{
		Identifier *identifier = indexIdentifiers_[idIndex];

		// Using a reverse loop on all previous statements, find the closest dependent statement
		// (a loop statement or an assignment statement).
		for (int stIndex = _totalAssignmentStatementList->size() - 1; stIndex >= 0; --stIndex)
		{
			Statement *statement = (*_totalAssignmentStatementList)[stIndex];

			if (statement->kind() == ktForStatement && ((ForStatement *)statement)->loopVariable()->object() == identifier->object())
			{
				identifier->indexDependency(statement);
				break;
			}
			else if (statement->kind() == ktAssignmentStatement)
			{
				bool flag = false;
				for (auto &id : ((AssignmentStatement *)(statement))->lhsIdentifiers())
					if (id->object() == identifier->object())
					{
						identifier->indexDependency(statement);
						flag = true;
						break;
					}
				if (flag)
				{
					break;
				}
			}
		}

		// If no corresponding statement found for the index dependency, raise an error for that.
		if (identifier->indexDependency() == 0)
			LOG(FATAL) << ("findAssignStatementIndexDependencies: Cannot find the index dependency for: " + identifier->name());
	}
}

// This method sets the RACCU mode according to the rhs of an assignment statement for the RaccuPragma.
void VElaborator::setRaccuMode(VIR::Expression *expression_, VIR::RaccuPragma *raccuPragma_)
{
	return;
	BIR::BIREnumerations::RACCUMode &raccuMode = raccuPragma_->raccuMode;

	if (expression_->kind() == ktPrimitiveFunctionCall)
	{
		PrimitiveFunctionCall *primitiveFunctionCall = (PrimitiveFunctionCall *)expression_;

		switch (primitiveFunctionCall->type())
		{
		case ftShiftLeft:
			raccuMode = BIR::BIREnumerations::rmShiftLeft;
			break;
		case ftShiftRight:
			raccuMode = BIR::BIREnumerations::rmShiftRight;
			break;
		default:
			LOG(FATAL) << ("setRaccuMode: Unknown primitive functions!");
			break;
		}
	}
	else
	{
		switch (expression_->kind())
		{
		// TODO: currently it only supports Add/Sub operations
		case ktBinaryExpression:
			if (raccuMode == BIR::BIREnumerations::rmUnspecified)
				switch (((BinaryExpression *)expression_)->operatorType())
				{
				case botSub:
					raccuMode = BIR::BIREnumerations::rmSub;
					break;
				case botAdd:
					raccuMode = BIR::BIREnumerations::rmAdd;
					break;
				default:
					LOG(FATAL) << ("setRaccuMode: Unsupported operation!");
					break;
				}
			break;

		// Do nothing. It is related to RACCU initialization
		case ktInteger:
			break;

		default:
			LOG(FATAL) << ("setRaccuMode: Unsupported operation!");
			break;
		}
	}
}

// This method takes a signature corresponding to unrolled loop values, extract the values and update them in the UnrolledLoopValues map
void VElaborator::updateUnrolledLoopValues(const UnrolledLoops &unrolledLoops_, const string &signature_, UnrolledLoopValuesMap &unrolledLoopValues_)
{
	// Updating unrolledLoopValuesMap according to the values in the signature
	for (int j = unrolledLoops_.size() - 1, sIter = 0; j >= 0; --j)
	{
		unrolledLoopValues_[unrolledLoops_[j]->loopVariable()->object()] = atoi(signature_.substr(sIter, signature_.find("_")).c_str());
		sIter += signature_.find("_") + 1;
	}
}
