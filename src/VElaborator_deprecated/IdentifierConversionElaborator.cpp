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

#include "IdentifierConversionElaborator.hpp"

using namespace VIR;
using namespace BIR;
using namespace VIR::VirEnumerations;
using namespace BIR::BIREnumerations;

namespace vesyla
{
namespace elaborate
{

void IdentifierConversionElaborator::elaborate(VIR::MainProgram *main_)
{
	elaborateMainProgram(main_);
}

VIR::Array *IdentifierConversionElaborator::elaborateArray(VIR::Array *array_)
{
	return array_;
}

VIR::AssignmentPragma *IdentifierConversionElaborator::elaborateAssignmentPragma(VIR::AssignmentPragma *assignmentPragma_)
{
	if (assignmentPragma_)
	{
		return elaborateReferencePragma((ReferencePragma *)assignmentPragma_);
	}

	return 0;
}

VIR::AssignmentStatement *IdentifierConversionElaborator::elaborateAssignmentStatement(VIR::AssignmentStatement *assignmentStatement_)
{

	for (auto &sn : assignmentStatement_->lhs())
	{
		elaborateExpression(sn);
	}
	elaborateExpression(assignmentStatement_->rhs());
	elaborateAssignmentPragma(assignmentStatement_->pragma());

	if (assignmentStatement_->type() == VIR::VirEnumerations::atDeclarative)
	{
		LOG(DEBUG) << "HERE: " << static_cast<Identifier *>(assignmentStatement_->lhs()[0])->name();
		string var_name = static_cast<Identifier *>(assignmentStatement_->lhs()[0])->name();
		int start = static_cast<VIR::StoragePragma *>(assignmentStatement_->pragma())->startAddresses[0];
		int size = static_cast<VIR::StoragePragma *>(assignmentStatement_->pragma())->maxIndexes[0];
		_var_map[var_name] = {var_name, size};
	}

	return assignmentStatement_;
}

VIR::BinaryExpression *IdentifierConversionElaborator::elaborateBinaryExpression(VIR::BinaryExpression *binaryExpression_)
{
	elaborateExpression(binaryExpression_->leftOperand());
	elaborateExpression(binaryExpression_->rightOperand());
	return binaryExpression_;
}

VIR::Complex *IdentifierConversionElaborator::elaborateComplex(VIR::Complex *complex_)
{
	__NOT_SUPPORTED__;
	return complex_;
}

VIR::DebugStatement *IdentifierConversionElaborator::elaborateDebugStatement(VIR::DebugStatement *debugStatement_)
{
	__NOT_SUPPORTED__;
	return debugStatement_;
}

VIR::Expression *IdentifierConversionElaborator::elaborateExpression(VIR::Expression *expression_)
{
	Expression *expression;

	switch (expression_->kind())
	{
	case VIR::VirEnumerations::ktArray:
		expression = elaborateArray(static_cast<VIR::Array *>(expression_));
		break;
	case VIR::VirEnumerations::ktBinaryExpression:
		expression = elaborateBinaryExpression(static_cast<VIR::BinaryExpression *>(expression_));
		break;
	case VIR::VirEnumerations::ktComplex:
		expression = elaborateComplex(static_cast<VIR::Complex *>(expression_));
		break;
	case VIR::VirEnumerations::ktFloatingPoint:
		expression = elaborateFloatingPoint(static_cast<VIR::FloatingPoint *>(expression_));
		break;
	case VIR::VirEnumerations::ktFunctionCall:
		expression = elaborateFunctionCall(static_cast<VIR::FunctionCall *>(expression_));
		break;
	case VIR::VirEnumerations::ktIdentifier:
		expression = elaborateIdentifier(static_cast<VIR::Identifier *>(expression_));
		break;
	case VIR::VirEnumerations::ktInteger:
		expression = elaborateInteger(static_cast<VIR::Integer *>(expression_));
		break;
	case VIR::VirEnumerations::ktParenthesizedExpression:
		expression = elaborateParenthesizedExpression(static_cast<VIR::ParenthesizedExpression *>(expression_));
		break;
	case VIR::VirEnumerations::ktPrimitiveFunctionCall:
		expression = elaboratePrimitiveFunctionCall(static_cast<VIR::PrimitiveFunctionCall *>(expression_));
		break;
	case VIR::VirEnumerations::ktRangeExpression:
		expression = elaborateRangeExpression(static_cast<VIR::RangeExpression *>(expression_));
		break;
	case VIR::VirEnumerations::ktSliceName:
		expression = elaborateSliceName(static_cast<VIR::SliceName *>(expression_));
		break;
	case VIR::VirEnumerations::ktStringLiteral:
		expression = elaborateStringLiteral(static_cast<VIR::StringLiteral *>(expression_));
		break;
	case VIR::VirEnumerations::ktUnaryExpression:
		expression = elaborateUnaryExpression(static_cast<VIR::UnaryExpression *>(expression_));
		break;
	default:
		LOG(FATAL) << "Wrong expression type!";
	}
	return expression;
}

VIR::FloatingPoint *IdentifierConversionElaborator::elaborateFloatingPoint(VIR::FloatingPoint *floatingPoint_)
{
	return floatingPoint_;
}

// This method elaborates ForStatement elements.
// It also checks that if the loop bounds are dynamic or there is vector inside the loop,
// so all the upper level loops should be aware of a dynamic loop or a vector inside them.
VIR::ForStatement *IdentifierConversionElaborator::elaborateForStatement(VIR::ForStatement *forStatement_)
{
	for (auto &s : forStatement_->loopBody())
	{
		elaborateStatement(s);
	}
	return forStatement_;
}

VIR::FunctionCall *IdentifierConversionElaborator::elaborateFunctionCall(VIR::FunctionCall *functionCall_)
{
	return functionCall_;
}

VIR::Expression *IdentifierConversionElaborator::elaborateIdentifier(VIR::Identifier *identifier_)
{
	LOG(DEBUG) << "HERE 22: " << identifier_->name();
	if (_var_map.find(identifier_->name()) != _var_map.end())
	{

		SliceName *sn = CREATE_OBJECT(SliceName);
		sn->prefix(identifier_);
		RangeExpression *re = CREATE_OBJECT(RangeExpression);
		re->begin(CREATE_INTEGER(1));
		re->increment(CREATE_INTEGER(1));
		re->end(CREATE_INTEGER(_var_map[identifier_->name()].size));
		sn->suffix().clear();
		sn->suffix().push_back(re);
		return sn;
	}
	return identifier_;
}

// This method changes the given if statement in the following cases:
// If the condition is a binary expression and binary operation is LessThan, LessOrEqual, and UnEqual,
// they should be converted to GreaterThan, GreaterOrEqual, and Equal correspondingly.
// The second argument shows if this method has been called from the elaborateIfThenElseStatement method,
// Because in this case if the binary operation is unequal, we don't need to create a new IfThenElse object
// and should just swap thenPart and elsePart of the existing object.
VIR::IfThenElseStatement *IdentifierConversionElaborator::elaborateIfStatement(VIR::IfStatement *ifStatement_)
{

	elaborateExpression(ifStatement_->condition());
	for (auto &s : ifStatement_->thenPart())
	{
		elaborateStatement(s);
	}

	return ifStatement_;
}

VIR::IfThenElseStatement *IdentifierConversionElaborator::elaborateIfThenElseStatement(VIR::IfThenElseStatement *ifThenElseStatement_)
{
	elaborateExpression(ifThenElseStatement_->condition());
	for (auto &s : ifThenElseStatement_->thenPart())
	{
		elaborateStatement(s);
	}
	for (auto &s : ifThenElseStatement_->elsePart())
	{
		elaborateStatement(s);
	}

	return ifThenElseStatement_;
}

VIR::Integer *IdentifierConversionElaborator::elaborateInteger(VIR::Integer *integer_)
{
	return integer_;
}

VIR::MainProgram *IdentifierConversionElaborator::elaborateMainProgram(VIR::MainProgram *mainProgram_)
{
	for (auto &s : mainProgram_->body())
	{
		elaborateStatement(s);
	}
	return mainProgram_;
}

VIR::ParenthesizedExpression *IdentifierConversionElaborator::elaborateParenthesizedExpression(VIR::ParenthesizedExpression *parenthesizedExpression_)
{
	elaborateExpression(parenthesizedExpression_->value());

	return parenthesizedExpression_;
}

VIR::PragmaStatement *IdentifierConversionElaborator::elaboratePragmaStatement(VIR::PragmaStatement *pragmaStatement_)
{
	return pragmaStatement_;
}

VIR::PrimitiveFunctionCall *IdentifierConversionElaborator::elaboratePrimitiveFunctionCall(VIR::PrimitiveFunctionCall *primitiveFunctionCall_)
{
	for (auto &e : primitiveFunctionCall_->parametersSplit())
	{
		elaborateExpression(e);
	}
	return primitiveFunctionCall_;
}

VIR::RangeExpression *IdentifierConversionElaborator::elaborateRangeExpression(VIR::RangeExpression *rangeExpression_)
{
	elaborateExpression(rangeExpression_->begin());
	elaborateExpression(rangeExpression_->end());
	elaborateExpression(rangeExpression_->increment());
	return rangeExpression_;
}

VIR::ReferencePragma *IdentifierConversionElaborator::elaborateReferencePragma(VIR::ReferencePragma *referencePragma_)
{
	return referencePragma_;
}

VIR::SliceName *IdentifierConversionElaborator::elaborateSliceName(VIR::SliceName *sliceName_)
{
	return sliceName_;
}

VIR::Statement *IdentifierConversionElaborator::elaborateStatement(VIR::Statement *statement_)
{
	Statement *statement;

	switch (statement_->kind())
	{
	case VIR::VirEnumerations::ktAssignmentStatement:
		statement = elaborateAssignmentStatement(static_cast<VIR::AssignmentStatement *>(statement_));
		break;
	case VIR::VirEnumerations::ktDebugStatement:
		statement = elaborateDebugStatement(static_cast<VIR::DebugStatement *>(statement_));
		break;
	case VIR::VirEnumerations::ktForStatement:
		statement = elaborateForStatement(static_cast<VIR::ForStatement *>(statement_));
		break;
	case VIR::VirEnumerations::ktIfStatement:
		statement = elaborateIfStatement(static_cast<VIR::IfStatement *>(statement_));
		break;
	case VIR::VirEnumerations::ktIfThenElseStatement:
		statement = elaborateIfThenElseStatement(static_cast<VIR::IfThenElseStatement *>(statement_));
		break;
	case VIR::VirEnumerations::ktPragmaStatement:
		statement = elaboratePragmaStatement(static_cast<VIR::PragmaStatement *>(statement_));
		break;
	default:
		LOG(FATAL) << "Wrong statement type!";
	}

	return statement;
}

VIR::StringLiteral *IdentifierConversionElaborator::elaborateStringLiteral(VIR::StringLiteral *stringLiteral_)
{
	return stringLiteral_;
}

VIR::UnaryExpression *IdentifierConversionElaborator::elaborateUnaryExpression(VIR::UnaryExpression *unaryExpression_)
{
	elaborateExpression(unaryExpression_->operand());
	return unaryExpression_;
}
} // namespace elaborate
} // namespace vesyla
