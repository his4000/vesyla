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

using namespace VIR;
using namespace VirEnumerations;

#define CALL_EXPRESSION_FUNC(value)                     \
	case kt##value:                                       \
		expression = traverse##value((value *)expression_); \
		break;
#define CALL_STATEMENT_FUNC(value)                    \
	case kt##value:                                     \
		statement = traverse##value((value *)statement_); \
		break;

// TODO: you are here: check the generated IndexInfo with the previous results
// TODO: add a function for distinguishing between affine and non-affine address functions

VIR::Array *VElaborator::traverseArray(VIR::Array *array_)
{
	return array_;
}

VIR::AssignmentPragma *VElaborator::traverseAssignmentPragma(VIR::AssignmentPragma *assignmentPragma_)
{
	if (assignmentPragma_)
	{
		return traverseReferencePragma((ReferencePragma *)assignmentPragma_);
	}

	return 0;
}

VIR::AssignmentStatement *VElaborator::traverseAssignmentStatement(VIR::AssignmentStatement *assignmentStatement_)
{
	_currForStatement = findDependentForStatement(assignmentStatement_);

	for (auto &sn : assignmentStatement_->lhs())
	{
		traverseExpression(sn);
	}
	traverseExpression(assignmentStatement_->rhs());
	traverseAssignmentPragma(assignmentStatement_->pragma());

	return assignmentStatement_;
}

VIR::BinaryExpression *VElaborator::traverseBinaryExpression(VIR::BinaryExpression *binaryExpression_)
{
	traverseExpression(binaryExpression_->leftOperand());
	traverseExpression(binaryExpression_->rightOperand());

	return binaryExpression_;
}

VIR::Complex *VElaborator::traverseComplex(VIR::Complex *complex_)
{
	return complex_;
}

VIR::DebugStatement *VElaborator::traverseDebugStatement(VIR::DebugStatement *debugStatement_)
{
	return debugStatement_;
}

VIR::Expression *VElaborator::traverseExpression(VIR::Expression *expression_)
{
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
		CALL_EXPRESSION_FUNC(PrimitiveFunctionCall);
		CALL_EXPRESSION_FUNC(RangeExpression);
		CALL_EXPRESSION_FUNC(SliceName);
		CALL_EXPRESSION_FUNC(StringLiteral);
		CALL_EXPRESSION_FUNC(UnaryExpression);
	}

	return expression;
}

vector<VIR::Expression *> *VElaborator::traverseExpressionList(vector<VIR::Expression *> *expressionList_)
{
	const int size = expressionList_->size();

	for (int index = 0; index < size; ++index)
		traverseExpression((*expressionList_)[index]);

	return expressionList_;
}

VIR::FloatingPoint *VElaborator::traverseFloatingPoint(VIR::FloatingPoint *floatingPoint_)
{
	return floatingPoint_;
}

// This method traverses ForStatement elements.
// It also checks that if the loop bounds are dynamic or there is vector inside the loop,
// so all the upper level loops should be aware of a dynamic loop or a vector inside them.
// Changed by Yu Yang
// No need any special process to FOR LOOP, everything will be dealed with after the CIDFG has been built.
VIR::ForStatement *VElaborator::traverseForStatement(VIR::ForStatement *forStatement_)
{
	traverseRangeExpression(forStatement_->loopRange());
	traverseIdentifier(forStatement_->loopVariable());

	// if (!forStatement_->isConstant() && (_raccuMode || forStatement_->degree() > 2))
	// 	forStatement_->needDynamicHandling(true);

	// // needDynamicHandling might have been set before in the Elaboration phase
	// if (forStatement_->needDynamicHandling())
	// {
	// 	ControlStatement *controlStatement = dynamic_cast<ControlStatement *>(forStatement_->controlDependentTo());

	// 	while (controlStatement)
	// 	{
	// 		if (controlStatement->kind() == ktForStatement)
	// 			((ForStatement *)controlStatement)->needDynamicHandling(true);

	// 		controlStatement = dynamic_cast<ControlStatement *>(controlStatement->controlDependentTo());
	// 	}
	// }

	// bool oldFlagValue = _isNonConstantLoopRegion;
	// if (!forStatement_->isConstant() && !forStatement_->needDynamicHandling())
	// {
	// 	_isNonConstantLoopRegion = true;

	// 	// Finding different repetitions according to the unrolled loop values

	// 	vector<ForStatement *> unrolledLoops;
	// 	vector<string> unrolledSignatures;
	// 	getUnrolledLoopsAndSignatures(forStatement_, unrolledLoops, unrolledSignatures);
	// 	UnrolledLoopValuesMap unrolledloopValues;
	// 	string signature;

	// 	for (unsigned i = 0; i < unrolledSignatures.size(); ++i)
	// 	{
	// 		signature = unrolledSignatures[i];

	// 		updateUnrolledLoopValues(unrolledLoops, signature, unrolledloopValues);

	// 		forStatement_->unrolledRepetitionsMap().operator[](signature) = _exprEvaluator->evaluateExpressionToInt(forStatement_->loopRange(), 1, 0, 4, unrolledloopValues);
	// 	}
	// }
	traverseStatementList(&forStatement_->loopBody());

	// _isNonConstantLoopRegion = oldFlagValue;

	return forStatement_;
}

VIR::FunctionCall *VElaborator::traverseFunctionCall(VIR::FunctionCall *functionCall_)
{
	return functionCall_;
}

VIR::Identifier *VElaborator::traverseIdentifier(VIR::Identifier *identifier_)
{
	return identifier_;
}

// This method changes the given if statement in the following cases:
// If the condition is a binary expression and binary operation is LessThan, LessOrEqual, and UnEqual,
// they should be converted to GreaterThan, GreaterOrEqual, and Equal correspondingly.
// The second argument shows if this method has been called from the traverseIfThenElseStatement method,
// Because in this case if the binary operation is unequal, we don't need to create a new IfThenElse object
// and should just swap thenPart and elsePart of the existing object.
VIR::IfStatement *VElaborator::traverseIfStatement(VIR::IfStatement *ifStatement_, bool isFromIfThenElse_ /* = false */)
{
	IfStatement *ifStatement = ifStatement_;

	// MODIFIED BY Yu Yang 2018-09-27 ----------------------------------------------
	// This part is commented because the translation from ~= to == and from < to > is taken care of while translating
	// to backend vertices in CIDFG. This is no longer needed in elaboration phase.
	// COMMENT BEGIN----------------------------------------------------------------
	// if (ifStatement_->condition()->kind() == ktBinaryExpression)
	// {
	// 	BinaryExpression *binaryExpression = (BinaryExpression *)ifStatement_->condition();

	// 	switch (binaryExpression->operatorType())
	// 	{
	// 	// Swap the left and right operands and change the operator
	// 	case botLessThan:
	// 	case botLessOrEqual:
	// 	{
	// 		Expression *leftOperand = binaryExpression->leftOperand();
	// 		binaryExpression->leftOperand(binaryExpression->rightOperand());
	// 		binaryExpression->rightOperand(leftOperand);
	// 		binaryExpression->operatorType(binaryExpression->operatorType() == botLessThan ? botGreaterThan : botGreaterOrEqual);
	// 	}
	// 	break;

	// 	// Create a new IfThenElseStatement object, change the operator to Equal, and copy the current thenPart to the elsePart of the new object.
	// 	case botUnEqual:
	// 	{
	// 		binaryExpression->operatorType(botEqual);

	// 		if (!isFromIfThenElse_)
	// 		{
	// 			IfThenElseStatement *ifThenElseStatement = CREATE_OBJECT(IfThenElseStatement);

	// 			ifThenElseStatement->statementNo(ifStatement_->statementNo());
	// 			ifThenElseStatement->controlDependentTo(ifStatement_->controlDependentTo());
	// 			ifThenElseStatement->conditionPragma(ifStatement_->conditionPragma());
	// 			ifThenElseStatement->mode(ifStatement_->mode());
	// 			ifThenElseStatement->condition(binaryExpression);

	// 			for (int stIndex = 0; stIndex < ifStatement_->thenPart().size(); ++stIndex)
	// 				ifThenElseStatement->elsePart().push_back(ifStatement_->thenPart()[stIndex]);

	// 			ifStatement = ifThenElseStatement;
	// 		}
	// 		else // The object is an IfThenElseStatement
	// 		{
	// 			IfThenElseStatement *ifThenElseStatement = (IfThenElseStatement *)ifStatement_;

	// 			ifThenElseStatement->thenPart().swap(ifThenElseStatement->elsePart());

	// 			// This swapping does not conflict with the traversing functionality of if-then-else statement,
	// 			// as in this case, first, the else-part is traversed as the then-part, and then the then-part
	// 			// is traversed as the else-part in the traverseIfThenElseStatement() method.
	// 		}
	// 	}
	// 	break;

	// 	default:
	// 		break; // do nothing
	// 	}
	// }
	// COMMENT ENDS-----------------------------------------------------------------

	_currForStatement = findDependentForStatement(ifStatement);

	traverseExpression(ifStatement->condition());
	traverseStatementList(&ifStatement->thenPart());

	return ifStatement;
}

VIR::IfThenElseStatement *VElaborator::traverseIfThenElseStatement(VIR::IfThenElseStatement *ifThenElseStatement_)
{
	traverseIfStatement(ifThenElseStatement_, true);
	traverseStatementList(&ifThenElseStatement_->elsePart());

	return ifThenElseStatement_;
}

VIR::Integer *VElaborator::traverseInteger(VIR::Integer *integer_)
{
	return integer_;
}

VIR::MainProgram *VElaborator::traverseMainProgram(VIR::MainProgram *mainProgram_)
{
	_isNonConstantLoopRegion = false;

	traverseStatementList(&mainProgram_->body());

	return mainProgram_;
}

VIR::ParenthesizedExpression *VElaborator::traverseParenthesizedExpression(VIR::ParenthesizedExpression *parenthesizedExpression_)
{
	traverseExpression(parenthesizedExpression_->value());

	return parenthesizedExpression_;
}

VIR::PragmaStatement *VElaborator::traversePragmaStatement(VIR::PragmaStatement *pragmaStatement_)
{
	return pragmaStatement_;
}

VIR::PrimitiveFunctionCall *VElaborator::traversePrimitiveFunctionCall(VIR::PrimitiveFunctionCall *primitiveFunctionCall_)
{
	traverseExpressionList(&primitiveFunctionCall_->parametersSplit());

	return primitiveFunctionCall_;
}

VIR::RangeExpression *VElaborator::traverseRangeExpression(VIR::RangeExpression *rangeExpression_)
{
	traverseExpression(rangeExpression_->begin());
	traverseExpression(rangeExpression_->end());
	traverseExpression(rangeExpression_->increment());

	return rangeExpression_;
}

VIR::ReferencePragma *VElaborator::traverseReferencePragma(VIR::ReferencePragma *referencePragma_)
{
	return referencePragma_;
}

VIR::SliceName *VElaborator::traverseSliceName(VIR::SliceName *sliceName_)
{
	IndexInfo *indexInfo = CREATE_OBJECT(IndexInfo);

	sliceName_->indexInfo(indexInfo);
	indexInfo->isARaccuVariable = false;
	indexInfo->isARangeExpression = false;

	vector<VIR::Expression *> *suffix = &sliceName_->suffix();

	// TODO: Currently it only supports one dimensional indices
	//	     It should be extended to multi dimensional indices
	Expression *sliceNameIndex = (*suffix)[0];

	// Indirect addressing does not need any index flow analysis
	if (sliceNameIndex->kind() == ktIdentifier && ((Identifier *)sliceNameIndex)->object()->objectType() == otIndirectAddressVariable)
		return sliceName_;

	// If the index is an explicitly defined RACCU variable, there is no need to further calculations. It will be taken care by RACCU variables dynamically.
	// TODO: Currently it only supports pure RACCU variable instance with no other operations.
	// TODO: Currently if the slice name suffix is a range expression containing raccu variable, then both of the begin and end expressions should be raccu variables.
	if ((sliceNameIndex->kind() == ktIdentifier && ((Identifier *)sliceNameIndex)->object()->objectType() == otRaccuVariable) ||
			(sliceNameIndex->kind() == ktRangeExpression && ((RangeExpression *)sliceNameIndex)->begin()->kind() == ktIdentifier &&
			 ((Identifier *)((RangeExpression *)sliceNameIndex)->begin())->object()->objectType() == otRaccuVariable &&
			 ((RangeExpression *)sliceNameIndex)->end()->kind() == ktIdentifier &&
			 ((Identifier *)((RangeExpression *)sliceNameIndex)->end())->object()->objectType() == otRaccuVariable))
	{
		indexInfo->isARaccuVariable = true;
		return sliceName_;
	}

	/*
	 * Data: 2018-05-29
	 * Author: Yu Yang <yuyang2@kh.se>
	 *
	 * Add support of AGU primitive function:
	 *   - silago_agu_linear_1d: generate 1-level affine linear address pattern.
	 *   - silago_agu_linear_2d: generate 2-level affine linear address pattern.
	 */
	/***************************** begin **************************************/
	if (sliceNameIndex->kind() == ktFunctionCall)
	{
		// PrimitiveFunctionCall *func = static_cast<PrimitiveFunctionCall *>(sliceNameIndex);
		// if (isAguPrimitiveFunction(func->_name->name()))
		// {
		// 	if (func->_name->name() == "silago_agu_linear_1d")
		// 	{
		// 		if (func->_parameters.size() != 3)
		// 		{
		// 			LOG(FATAL) << "AGU primitive function call \"" << func->_name->name() << "\" has wrong parameter list!";
		// 		}
		// 		if (func->_parameters[0]->kind() == ktInteger)
		// 		{
		// 			indexInfo->addressInfo.startAddress = static_cast<Integer *>(func->_parameters[0])->value;
		// 		}
		// 		else
		// 		{
		// 			std::map<VIR::ObjectDeclaration *, int> umv;
		// 			indexInfo->addressInfo.startAddress = _exprEvaluator->evaluateExpressionToInt(func->_parameters[0], 2, _currForStatement->loopVariable()->object(), 2, umv);
		// 		}
		// 		if (func->_parameters[1]->kind() == ktInteger)
		// 		{
		// 			indexInfo->addressInfo.increment = static_cast<Integer *>(func->_parameters[1])->value;
		// 		}
		// 		else
		// 		{
		// 			std::map<VIR::ObjectDeclaration *, int> umv;
		// 			indexInfo->addressInfo.increment = _exprEvaluator->evaluateExpressionToInt(func->_parameters[1], 2, _currForStatement->loopVariable()->object(), 2, umv);
		// 		}
		// 		if (func->_parameters[2]->kind() != ktInteger)
		// 		{
		// 			LOG(FATAL) << "AGU primitive function call \"" << func->_name->name() << "\" has wrong parameter type in position 2 !";
		// 		}
		// 		else
		// 		{
		// 			indexInfo->addressInfo.numberOfAddress = static_cast<Integer *>(func->_parameters[2])->value - 1;
		// 		}
		// 		indexInfo->addressInfo.firstLevelLoop = NULL;
		// 		indexInfo->addressInfo.secondLevelLoop = NULL;
		// 		indexInfo->addressInfo.is1stLevelDynamic = false;
		// 		indexInfo->addressInfo.is2ndLevelDynamic = false;
		// 		indexInfo->addressInfo.numberOfRepetitions = 0;
		// 		indexInfo->addressInfo.repetitionOffset = 0;
		// 		return sliceName_;
		// 	}
		// 	else if (func->_name->name() == "silago_agu_linear_2d")
		// 	{
		// 		if (func->_parameters.size() != 5)
		// 		{
		// 			LOG(FATAL) << "AGU primitive function call \"" << func->_name->name() << "\" has wrong parameter list!";
		// 		}
		// 		if (func->_parameters[0]->kind() == ktInteger)
		// 		{
		// 			indexInfo->addressInfo.startAddress = static_cast<Integer *>(func->_parameters[0])->value;
		// 		}
		// 		else
		// 		{
		// 			std::map<VIR::ObjectDeclaration *, int> umv;
		// 			indexInfo->addressInfo.startAddress = _exprEvaluator->evaluateExpressionToInt(func->_parameters[0], 2, _currForStatement->loopVariable()->object(), 2, umv);
		// 		}
		// 		if (func->_parameters[1]->kind() == ktInteger)
		// 		{
		// 			indexInfo->addressInfo.increment = static_cast<Integer *>(func->_parameters[1])->value;
		// 		}
		// 		else
		// 		{
		// 			std::map<VIR::ObjectDeclaration *, int> umv;
		// 			indexInfo->addressInfo.increment = _exprEvaluator->evaluateExpressionToInt(func->_parameters[1], 2, _currForStatement->loopVariable()->object(), 2, umv);
		// 		}
		// 		if (func->_parameters[2]->kind() != ktInteger)
		// 		{
		// 			LOG(FATAL) << "AGU primitive function call \"" << func->_name->name() << "\" has wrong parameter type in position 2 !";
		// 		}
		// 		else
		// 		{
		// 			indexInfo->addressInfo.numberOfAddress = static_cast<Integer *>(func->_parameters[2])->value - 1;
		// 		}
		// 		if (func->_parameters[3]->kind() == ktInteger)
		// 		{
		// 			indexInfo->addressInfo.repetitionOffset = static_cast<Integer *>(func->_parameters[3])->value;
		// 		}
		// 		else
		// 		{
		// 			std::map<VIR::ObjectDeclaration *, int> umv;
		// 			indexInfo->addressInfo.repetitionOffset = _exprEvaluator->evaluateExpressionToInt(func->_parameters[3], 1, 0, 1, umv);
		// 		}
		// 		if (func->_parameters[4]->kind() != ktInteger)
		// 		{
		// 			LOG(FATAL) << "AGU primitive function call \"" << func->_name->name() << "\" has wrong parameter type in position 4 !";
		// 		}
		// 		else
		// 		{
		// 			indexInfo->addressInfo.numberOfRepetitions = static_cast<Integer *>(func->_parameters[4])->value - 1;
		// 		}
		// 		indexInfo->addressInfo.firstLevelLoop = NULL;
		// 		indexInfo->addressInfo.secondLevelLoop = NULL;
		// 		indexInfo->addressInfo.is1stLevelDynamic = false;
		// 		indexInfo->addressInfo.is2ndLevelDynamic = false;
		// 		return sliceName_;
		// 	}
		// 	else
		// 	{
		// 		LOG(FATAL) << "Unreconganize AGU primitive function!";
		// 	}
		// }
		return sliceName_;
	}

	/****************************** end ***************************************/

	// This means that the slice name is related to a statement inside a loop and
	// the inner loops are non-constant and we are going to use loop unrolling
	// for the higher loop degrees (> 2) instead of RACCU loops
	if (_currForStatement && _isNonConstantLoopRegion)
	{
		UnrolledLoops unrolledLoops;
		vector<string> unrolledSignatures;

		getUnrolledLoopsAndSignatures(_currForStatement, unrolledLoops, unrolledSignatures);

		// ---------------------------------------------------------------------------------------------------
		// Fill in the unrolledAddressMap of indexInfo for all unrolled loop values.

		if (sliceNameIndex->kind() == ktRangeExpression)
			LOG(FATAL) << ("traverseSliceName: vector operation inside non-constant loops is not supported!");

		UnrolledLoopValuesMap unrolledloopValues;
		UnrolledAddressInfo unrolledAddressInfo;
		string signature;

		for (unsigned i = 0; i < unrolledSignatures.size(); ++i)
		{
			signature = unrolledSignatures[i];

			updateUnrolledLoopValues(unrolledLoops, signature, unrolledloopValues);

			// Updating the unrolledAddressInfo values

			unrolledAddressInfo.startAddress = _exprEvaluator->evaluateExpressionToInt(sliceNameIndex, 1, 0, 1, unrolledloopValues);

			if (_currForStatement->degree() == 1)
			{
				const int stepChangeValue = _exprEvaluator->evaluateExpressionToInt(sliceNameIndex, 2, _currForStatement->loopVariable()->object(), 2, unrolledloopValues);

				unrolledAddressInfo.increment = stepChangeValue - unrolledAddressInfo.startAddress;

				const int repetition = _currForStatement->unrolledRepetitionsMap().at(signature);

				unrolledAddressInfo.numberOfAddress = repetition + (repetition > 0 ? -1 : 0);
			}

			if (_currForStatement->degree() == 2 || _currForStatement->degree() == 1)
			{
				ForStatement *lvl2Loop;

				if (_currForStatement->degree() == 2)
					lvl2Loop = _currForStatement;
				else
				{
					lvl2Loop = (ForStatement *)_currForStatement->controlDependentTo();

					while (lvl2Loop)
						if (lvl2Loop->kind() == ktForStatement)
							break;
						else
							lvl2Loop = (ForStatement *)lvl2Loop->controlDependentTo();
				}

				int stepChangeValue = _exprEvaluator->evaluateExpressionToInt(sliceNameIndex, 2, lvl2Loop->loopVariable()->object(), 2, unrolledloopValues);

				unrolledAddressInfo.repetitionOffset = stepChangeValue - unrolledAddressInfo.startAddress;

				const int repetition = lvl2Loop->unrolledRepetitionsMap().at(signature);

				unrolledAddressInfo.numberOfRepetitions = repetition + (repetition > 0 ? -1 : 0);
			}

			// Adding unrolledAddressInfo to the corresponding map in indexInfo
			indexInfo->unrolledAddressMap[signature] = unrolledAddressInfo;
		}
	}
	// This means that the slice name is related to a statement inside a loop
	else if (_currForStatement)
	{
		// ----------------------------------------------------------
		// Calculating REFI values
		// All default values are 0.

		//! Yu Yang 2017-08-01
		// Change to solve compile error caused by passing reference of object in stack
		// CHANGE BEGIN
		std::map<VIR::ObjectDeclaration *, int> umv;
		indexInfo->addressInfo.startAddress = _exprEvaluator->evaluateExpressionToInt(sliceNameIndex, 1, 0, 1, umv);
		// CHANGE END

		// The slice name is a vector
		// In this case the range of the vector should be considered as the most inner loop and
		// the loop with degree=1 is considered as the the outer loop wrapping this inner loop.
		// All loops surrounding a vector operation should be handled by dynamically RACCU
		if (sliceNameIndex->kind() == ktRangeExpression)
		{
			indexInfo->isARangeExpression = true;

			RangeExpression *rangeExpr = (RangeExpression *)sliceNameIndex;

			if (rangeExpr->increment()->kind() != ktInteger)
				LOG(FATAL) << ("traverseSliceName: Unsupported increment for a vector inside loop!");

			indexInfo->addressInfo.increment = ((Integer *)rangeExpr->increment())->value;

			if (_currForStatement->isConstant())
			{
				//! Yu Yang 2017-08-01
				// Change to solve compile error caused by passing reference of object in stack
				// CHANGE BEGIN
				std::map<VIR::ObjectDeclaration *, int> umv;
				indexInfo->addressInfo.numberOfAddress = indexInfo->addressInfo.increment != 0 ? (_exprEvaluator->evaluateExpressionToInt(sliceNameIndex, 1, 0, 3, umv) - indexInfo->addressInfo.startAddress) / indexInfo->addressInfo.increment : 0;
				// CHANGE END
			}
			else
			{
				//LOG(FATAL) << ("traverseSliceName: Loops wrapping a vector operation should be constant!");
			}

			if (_currForStatement->degree() == 1)
			{
				ObjectDeclaration *object = _currForStatement->loopVariable()->object();

				// Adding the step value to the starting point value of the loop with degree == 1, and then evaluating the index expression

				//! Yu Yang 2017-08-01
				// Change to solve compile error caused by passing reference of object in stack
				// CHANGE BEGIN
				std::map<VIR::ObjectDeclaration *, int> umv;
				int stepChangeValue = _exprEvaluator->evaluateExpressionToInt(sliceNameIndex, 2, object, 1, umv);
				// CHANGE END

				indexInfo->addressInfo.repetitionOffset = stepChangeValue - indexInfo->addressInfo.startAddress;

				if (indexInfo->addressInfo.repetitionOffset > 0)
				{
					indexInfo->addressInfo.is1stLevelDynamic = true;
					indexInfo->addressInfo.firstLevelLoop = _currForStatement;

					// This value is only used if there are memory transfers inside the loop
					const int repetition = _currForStatement->repetition();
					indexInfo->addressInfo.numberOfRepetitions = repetition + (repetition > 0 ? -1 : 0);
				}
			}

			// ----------------------------------------------------------
			// Calculating the unrolling values (defaults are 0)

			ForStatement *forStatement = _currForStatement;

			while (forStatement)
			{
				if (forStatement->degree() > 1)
				{
					//! Yu Yang 2017-08-01
					// Change to solve compile error caused by passing reference of object in stack
					// CHANGE BEGIN
					std::map<VIR::ObjectDeclaration *, int> umv;
					int stepChangeValue = _exprEvaluator->evaluateExpressionToInt(sliceNameIndex, 2, forStatement->loopVariable()->object(), 1, umv);
					// CHANGE END

					indexInfo->addUnrolling(forStatement, forStatement->isConstant() ? forStatement->repetition() : 0,
																	stepChangeValue - indexInfo->addressInfo.startAddress, !forStatement->isConstant());
				}

				do
				{
					forStatement = (ForStatement *)forStatement->controlDependentTo();
				} while (forStatement && forStatement->kind() != ktForStatement);
			}
		}
		// The slice name just points to a single element
		else
		{
			if (_currForStatement->degree() == 1)
			{
				// Adding the step value to the starting point value of the loop with degree == 1, and then evaluating the index expression
				// For dynamic loop bounds, we assume that the start and step values are constant and only the end value can be non-constant.
				// So no problem will happen in the following evaluation.

				//! Yu Yang 2017-08-01
				// Change to solve compile error caused by passing reference of object in stack
				// CHANGE BEGIN
				std::map<VIR::ObjectDeclaration *, int> umv;
				const int stepChangeValue = _exprEvaluator->evaluateExpressionToInt(sliceNameIndex, 2, _currForStatement->loopVariable()->object(), 2, umv);
				// CHANGE END

				indexInfo->addressInfo.increment = stepChangeValue - indexInfo->addressInfo.startAddress;

				if (!_currForStatement->needDynamicHandling())
					indexInfo->addressInfo.numberOfAddress = _currForStatement->repetition() + (_currForStatement->repetition() > 0 ? -1 : 0);
				else if (indexInfo->addressInfo.increment > 0)
				{
					indexInfo->addressInfo.is1stLevelDynamic = true;
					indexInfo->addressInfo.firstLevelLoop = _currForStatement;
				}
			}

			if (_currForStatement->degree() == 2 || (_currForStatement->degree() == 1 && _currForStatement->controlDependentTo()))
			{
				ControlStatement *upperControlSt = dynamic_cast<ControlStatement *>(_currForStatement->controlDependentTo());

				while (upperControlSt)
				{
					if (upperControlSt->kind() == ktForStatement)
						break;
					else
						upperControlSt = dynamic_cast<ControlStatement *>(upperControlSt->controlDependentTo());
				}

				if (_currForStatement->degree() != 2 && !upperControlSt)
					LOG(FATAL) << ("traverseSliceName: improper control statement");

				ForStatement *upperLoop = (ForStatement *)upperControlSt;

				ObjectDeclaration *object = _currForStatement->degree() == 2 ? _currForStatement->loopVariable()->object() : upperLoop->loopVariable()->object();

				// Adding the step value to the starting point value of the loop with degree == 2, and then evaluating the index expression

				//! Yu Yang 2017-08-01
				// Change to solve compile error caused by passing reference of object in stack
				// CHANGE BEGIN
				std::map<VIR::ObjectDeclaration *, int> umv;
				int stepChangeValue = _exprEvaluator->evaluateExpressionToInt(sliceNameIndex, 2, object, 2, umv);
				// CHANGE END

				indexInfo->addressInfo.repetitionOffset = stepChangeValue - indexInfo->addressInfo.startAddress;

				if ((_currForStatement->degree() == 2 && !_currForStatement->needDynamicHandling()) || (_currForStatement->degree() == 1 && !upperLoop->needDynamicHandling()))
				{
					const int repetition = _currForStatement->degree() == 2 ? _currForStatement->repetition() : upperLoop->repetition();

					indexInfo->addressInfo.numberOfRepetitions = repetition + (repetition > 0 ? -1 : 0);
				}
				else if (indexInfo->addressInfo.repetitionOffset > 0)
				{
					indexInfo->addressInfo.is2ndLevelDynamic = true;
					indexInfo->addressInfo.secondLevelLoop = _currForStatement->degree() == 2 ? _currForStatement : upperLoop;
				}
			}

			// ----------------------------------------------------------
			// Calculating the unrolling values (defaults are 0)

			ForStatement *forStatement = _currForStatement;

			while (forStatement)
			{
				if (forStatement->degree() > 2)
				{
					//! Yu Yang 2017-08-01
					// Change to solve compile error caused by passing reference of object in stack
					// CHANGE BEGIN
					std::map<VIR::ObjectDeclaration *, int> umv;
					int stepChangeValue = _exprEvaluator->evaluateExpressionToInt(sliceNameIndex, 2, forStatement->loopVariable()->object(), 2, umv);
					// CHANGE END

					indexInfo->addUnrolling(forStatement, forStatement->isConstant() ? forStatement->repetition() : 0,
																	stepChangeValue - indexInfo->addressInfo.startAddress, !forStatement->isConstant());
				}

				do
				{
					forStatement = (ForStatement *)forStatement->controlDependentTo();
				} while (forStatement && forStatement->kind() != ktForStatement);
			}
		}
	}
	// It is outside any loop
	else
	{
		//! Yu Yang 2017-08-01
		// Change to solve compile error caused by passing reference of object in stack
		// CHANGE BEGIN
		std::map<VIR::ObjectDeclaration *, int> umv;
		indexInfo->addressInfo.startAddress = _exprEvaluator->evaluateExpressionToInt(sliceNameIndex, 0, 0, 1, umv);
		indexInfo->addressInfo.increment = _exprEvaluator->evaluateExpressionToInt(sliceNameIndex, 0, 0, 2, umv) - indexInfo->addressInfo.startAddress;
		indexInfo->addressInfo.numberOfAddress = indexInfo->addressInfo.increment != 0 ? (_exprEvaluator->evaluateExpressionToInt(sliceNameIndex, 0, 0, 3, umv) - indexInfo->addressInfo.startAddress) / indexInfo->addressInfo.increment : 0;
		// CHANGE END
		indexInfo->addressInfo.repetitionOffset = 0;
		indexInfo->addressInfo.numberOfRepetitions = 0;
	}

	return sliceName_;
}

VIR::Statement *VElaborator::traverseStatement(VIR::Statement *statement_)
{
	Statement *statement;

	switch (statement_->kind())
	{
		CALL_STATEMENT_FUNC(AssignmentStatement);
		CALL_STATEMENT_FUNC(DebugStatement);
		CALL_STATEMENT_FUNC(ForStatement);
		CALL_STATEMENT_FUNC(IfStatement);
		CALL_STATEMENT_FUNC(IfThenElseStatement);
		CALL_STATEMENT_FUNC(PragmaStatement);
	}

	return statement;
}

vector<VIR::Statement *> *VElaborator::traverseStatementList(vector<VIR::Statement *> *statementList_)
{
	const int size = statementList_->size();

	for (int index = 0; index < size; ++index)
		traverseStatement((*statementList_)[index]);

	return statementList_;
}

VIR::StringLiteral *VElaborator::traverseStringLiteral(VIR::StringLiteral *stringLiteral_)
{
	return stringLiteral_;
}

VIR::UnaryExpression *VElaborator::traverseUnaryExpression(VIR::UnaryExpression *unaryExpression_)
{
	traverseExpression(unaryExpression_->operand());

	return unaryExpression_;
}
