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

#include "AddressElaborator.hpp"

using namespace VIR;
using namespace BIR;
using namespace VIR::VirEnumerations;
using namespace BIR::BIREnumerations;

vector<ForStatement *> loops;

namespace vesyla
{
namespace elaborate
{

int calc_unary_expression(VIR::VirEnumerations::UnaryOperatorType operatorType_, int op_)
{
	if (op_ == INT_MAX)
		return INT_MAX;

	int result;

	switch (operatorType_)
	{
	case uotPlus:
		result = op_;
		break;
	case uotMinus:
		result = -op_;
		break;
	case uotNot:
		result = !op_;
		break;
	case uotTranspose:
		result = INT_MAX;
		break;
	case uotCTranspose:
		result = INT_MAX;
		break;
	}

	return result;
}

int calc_binary_expression(BinaryOperatorType operatorType_, int leftOp_, int rightOp_)
{
	if (leftOp_ == INT_MAX || rightOp_ == INT_MAX)
		return INT_MAX;

	int result;

	switch (operatorType_)
	{
	case botAdd:
		result = leftOp_ + rightOp_;
		break;
	case botSub:
		result = leftOp_ - rightOp_;
		break;
	case botMMul:
		result = leftOp_ * rightOp_;
		break;
	case botEMul:
		result = leftOp_ * rightOp_;
		break;
	case botMDiv:
		result = leftOp_ / rightOp_;
		break;
	case botEDiv:
		result = leftOp_ / rightOp_;
		break;
	case botMLeftDiv:
		result = leftOp_ / rightOp_;
		break;
	case botELeftDiv:
		result = leftOp_ / rightOp_;
		break;
	case botEqual:
		result = (int)(leftOp_ == rightOp_);
		break;
	case botUnEqual:
		result = (int)(leftOp_ != rightOp_);
		break;
	case botGreaterThan:
		result = (int)(leftOp_ > rightOp_);
		break;
	case botGreaterOrEqual:
		result = (int)(leftOp_ >= rightOp_);
		break;
	case botLessThan:
		result = (int)(leftOp_ < rightOp_);
		break;
	case botLessOrEqual:
		result = (int)(leftOp_ <= rightOp_);
		break;
	case botAnd:
		result = (int)(leftOp_ && rightOp_);
		break;
	case botOr:
		result = (int)(leftOp_ || rightOp_);
		break;
	case botMPower:
		result = (int)pow((float)leftOp_, rightOp_);
		break;
	case botEPower:
		result = (int)pow((float)leftOp_, rightOp_);
		break;
	}

	return result;
}

int evaluate_expression_to_int(Expression *e_, map<string, int> var_map_)
{
	switch (e_->kind())
	{
	case ktInteger:
	{
		Integer *e = static_cast<Integer *>(e_);
		return e->value;

		break;
	}
	case ktIdentifier:
	{
		Identifier *e = static_cast<Identifier *>(e_);
		if (var_map_.find(e->name()) != var_map_.end())
		{
			return var_map_[e->name()];
		}
		else
		{
			return INT_MAX;
		}
		break;
	}
	case ktBinaryExpression:
	{
		BinaryExpression *e = static_cast<BinaryExpression *>(e_);

		int leftValue = evaluate_expression_to_int(e->leftOperand(), var_map_);
		int rightValue = evaluate_expression_to_int(e->rightOperand(), var_map_);

		// Calculate the result only if the left and right operands are valid
		if (leftValue != INT_MAX && rightValue != INT_MAX)
		{

			return calc_binary_expression(e->operatorType(), leftValue, rightValue);
		}
		else
		{
			return INT_MAX;
		}
		break;
	}
	case ktUnaryExpression:
	{
		UnaryExpression *e = static_cast<UnaryExpression *>(e_);
		int op = evaluate_expression_to_int(e->operand(), var_map_);
		if (op != INT_MAX)
		{
			return calc_unary_expression(e->operatorType(), op);
		}
		else
		{
			return INT_MAX;
		}
		break;
	}
	case ktParenthesizedExpression:
	{
		ParenthesizedExpression *e = static_cast<ParenthesizedExpression *>(e_);
		return evaluate_expression_to_int(e->value(), var_map_);
		break;
	}
	default:
		LOG(FATAL) << "Illegal expression!";
		break;
	}
}

void AddressElaborator::elaborate(VIR::MainProgram *main_)
{
	loops.clear();
	elaborateMainProgram(main_);
}

VIR::Array *AddressElaborator::elaborateArray(VIR::Array *array_)
{
	return array_;
}

VIR::AssignmentPragma *AddressElaborator::elaborateAssignmentPragma(VIR::AssignmentPragma *assignmentPragma_)
{
	if (assignmentPragma_)
	{
		return elaborateReferencePragma((ReferencePragma *)assignmentPragma_);
	}

	return 0;
}

VIR::AssignmentStatement *AddressElaborator::elaborateAssignmentStatement(VIR::AssignmentStatement *assignmentStatement_)
{
	for (auto &sn : assignmentStatement_->lhs())
	{
		elaborateExpression(sn);
	}
	elaborateExpression(assignmentStatement_->rhs());
	elaborateAssignmentPragma(assignmentStatement_->pragma());
	return assignmentStatement_;
}

VIR::BinaryExpression *AddressElaborator::elaborateBinaryExpression(VIR::BinaryExpression *binaryExpression_)
{
	elaborateExpression(binaryExpression_->leftOperand());
	elaborateExpression(binaryExpression_->rightOperand());
	return binaryExpression_;
}

VIR::Complex *AddressElaborator::elaborateComplex(VIR::Complex *complex_)
{
	__NOT_SUPPORTED__;
	return complex_;
}

VIR::DebugStatement *AddressElaborator::elaborateDebugStatement(VIR::DebugStatement *debugStatement_)
{
	__NOT_SUPPORTED__;
	return debugStatement_;
}

VIR::Expression *AddressElaborator::elaborateExpression(VIR::Expression *expression_)
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

VIR::FloatingPoint *AddressElaborator::elaborateFloatingPoint(VIR::FloatingPoint *floatingPoint_)
{
	return floatingPoint_;
}

// This method elaborates ForStatement elements.
// It also checks that if the loop bounds are dynamic or there is vector inside the loop,
// so all the upper level loops should be aware of a dynamic loop or a vector inside them.
VIR::ForStatement *AddressElaborator::elaborateForStatement(VIR::ForStatement *forStatement_)
{
	loops.push_back(forStatement_);
	for (auto &s : forStatement_->loopBody())
	{
		elaborateStatement(s);
	}
	loops.pop_back();
	return forStatement_;
}

VIR::FunctionCall *AddressElaborator::elaborateFunctionCall(VIR::FunctionCall *functionCall_)
{
	return functionCall_;
}

VIR::Identifier *AddressElaborator::elaborateIdentifier(VIR::Identifier *identifier_)
{
	return identifier_;
}

// This method changes the given if statement in the following cases:
// If the condition is a binary expression and binary operation is LessThan, LessOrEqual, and UnEqual,
// they should be converted to GreaterThan, GreaterOrEqual, and Equal correspondingly.
// The second argument shows if this method has been called from the elaborateIfThenElseStatement method,
// Because in this case if the binary operation is unequal, we don't need to create a new IfThenElse object
// and should just swap thenPart and elsePart of the existing object.
VIR::IfThenElseStatement *AddressElaborator::elaborateIfStatement(VIR::IfStatement *ifStatement_)
{
	IfThenElseStatement *ifThenElseStatement;

	ifThenElseStatement = CREATE_OBJECT(IfThenElseStatement);

	ifThenElseStatement->statementNo(ifStatement_->statementNo());
	ifThenElseStatement->controlDependentTo(ifStatement_->controlDependentTo());
	ifThenElseStatement->conditionPragma(ifStatement_->conditionPragma());
	ifThenElseStatement->mode(ifStatement_->mode());
	ifThenElseStatement->condition(ifStatement_->condition());
	for (auto &s : ifStatement_->thenPart())
	{
		ifThenElseStatement->thenPart().push_back(s);
	}

	elaborateExpression(ifThenElseStatement->condition());
	for (auto &s : ifThenElseStatement->thenPart())
	{
		elaborateStatement(s);
	}

	return ifThenElseStatement;
}

VIR::IfThenElseStatement *AddressElaborator::elaborateIfThenElseStatement(VIR::IfThenElseStatement *ifThenElseStatement_)
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

VIR::Integer *AddressElaborator::elaborateInteger(VIR::Integer *integer_)
{
	return integer_;
}

VIR::MainProgram *AddressElaborator::elaborateMainProgram(VIR::MainProgram *mainProgram_)
{
	for (auto &s : mainProgram_->body())
	{
		elaborateStatement(s);
	}
	return mainProgram_;
}

VIR::ParenthesizedExpression *AddressElaborator::elaborateParenthesizedExpression(VIR::ParenthesizedExpression *parenthesizedExpression_)
{
	elaborateExpression(parenthesizedExpression_->value());

	return parenthesizedExpression_;
}

VIR::PragmaStatement *AddressElaborator::elaboratePragmaStatement(VIR::PragmaStatement *pragmaStatement_)
{
	return pragmaStatement_;
}

VIR::PrimitiveFunctionCall *AddressElaborator::elaboratePrimitiveFunctionCall(VIR::PrimitiveFunctionCall *primitiveFunctionCall_)
{
	for (auto &e : primitiveFunctionCall_->parametersSplit())
	{
		elaborateExpression(e);
	}
	return primitiveFunctionCall_;
}

VIR::RangeExpression *AddressElaborator::elaborateRangeExpression(VIR::RangeExpression *rangeExpression_)
{
	elaborateExpression(rangeExpression_->begin());
	elaborateExpression(rangeExpression_->end());
	elaborateExpression(rangeExpression_->increment());

	return rangeExpression_;
}

VIR::ReferencePragma *AddressElaborator::elaborateReferencePragma(VIR::ReferencePragma *referencePragma_)
{
	return referencePragma_;
}

void create_address_function(Expression *e_, AddressFunction &addr_)
{
	map<string, int> var_map;
	for (auto &s : loops)
	{
		int begin = evaluate_expression_to_int(s->loopRange()->begin(), {});
		CHECK_NE(begin, INT_MAX);
		var_map[s->loopVariable()->name()] = begin;
	}
	int b = evaluate_expression_to_int(e_, var_map);
	addr_.b(b);

	for (auto &s : loops)
	{
		var_map.clear();
		for (auto &ss : loops)
		{
			if (s->loopVariable()->name() == ss->loopVariable()->name())
			{
				int begin = evaluate_expression_to_int(ss->loopRange()->begin(), {});
				CHECK_NE(begin, INT_MAX);
				int increment = evaluate_expression_to_int(ss->loopRange()->increment(), {});
				CHECK_NE(increment, INT_MAX);
				var_map[ss->loopVariable()->name()] = begin + increment;
			}
			else
			{
				int begin = evaluate_expression_to_int(ss->loopRange()->begin(), {});
				CHECK_NE(begin, INT_MAX);
				var_map[ss->loopVariable()->name()] = begin;
			}
		}
		int b_inc = evaluate_expression_to_int(e_, var_map);
		CHECK_NE(b_inc, INT_MAX);
		if (b_inc != b)
		{
			int a = b_inc - b;
			int begin = evaluate_expression_to_int(s->loopRange()->begin(), {});
			CHECK_NE(begin, INT_MAX);
			int increment = evaluate_expression_to_int(s->loopRange()->increment(), {});
			CHECK_NE(increment, INT_MAX);
			int end = evaluate_expression_to_int(s->loopRange()->end(), {});
			CHECK_NE(end, INT_MAX);
			int n = (end - begin) / increment;
			addr_.add_level(a, n, s);
		}
	}
}

bool isAguPrimitiveFunction(const string name_)
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

VIR::SliceName *AddressElaborator::elaborateSliceName(VIR::SliceName *sliceName_)
{
	Expression *suffix = sliceName_->suffix()[0];
	ExpressionEvaluator eva;
	if (suffix->kind() == VIR::VirEnumerations::ktIdentifier && static_cast<VIR::Identifier *>(suffix)->object()->objectType() == VIR::VirEnumerations::otRaccuVariable)
	{
		Identifier *e = static_cast<Identifier *>(suffix);
		sliceName_->addr.b.raccu_var(e);
	}
	else if (suffix->kind() == VIR::VirEnumerations::ktRangeExpression)
	{
		RangeExpression *e = static_cast<RangeExpression *>(suffix);
		AddressFunction begin, increment, end;
		create_address_function(e->begin(), begin);
		create_address_function(e->increment(), increment);
		create_address_function(e->end(), end);
		CHECK(increment.is_constant());
		end.minus(begin);
		CHECK(end.is_constant());
		end.b(end.b() / increment.b());
		sliceName_->addr.b = begin;
		sliceName_->addr.a0 = increment;
		sliceName_->addr.n0 = end;
	}
	else if (suffix->kind() == VIR::VirEnumerations::ktFunctionCall)
	{
		PrimitiveFunctionCall *f = static_cast<PrimitiveFunctionCall *>(suffix);
		if (isAguPrimitiveFunction(f->_name->name()))
		{
			if (f->name()->name() == "silago_agu_linear_1d")
			{
				PrimitiveFunctionCall *e = static_cast<PrimitiveFunctionCall *>(suffix);
				CHECK_EQ(e->parameters().size(), 3);
				AddressFunction b, a0, n0;
				create_address_function(e->parameters()[0], b);
				create_address_function(e->parameters()[1], a0);
				create_address_function(e->parameters()[2], n0);
				CHECK(n0.is_constant());
				sliceName_->addr.b = b;
				sliceName_->addr.a0 = a0;
				sliceName_->addr.n0 = n0;
			}
			else if (f->name()->name() == "silago_agu_linear_2d")
			{
				PrimitiveFunctionCall *e = static_cast<PrimitiveFunctionCall *>(suffix);
				CHECK_EQ(e->parameters().size(), 5);
				AddressFunction b, a0, n0, a1, n1;
				create_address_function(e->parameters()[0], b);
				create_address_function(e->parameters()[1], a0);
				create_address_function(e->parameters()[2], n0);
				create_address_function(e->parameters()[3], a1);
				create_address_function(e->parameters()[4], n1);
				CHECK(n0.is_constant());
				CHECK(n1.is_constant());
				sliceName_->addr.b = b;
				sliceName_->addr.a0 = a0;
				sliceName_->addr.n0 = n0;
				sliceName_->addr.a1 = a1;
				sliceName_->addr.n1 = n1;
			}
		}
	}
	else
	{
		create_address_function(suffix, sliceName_->addr.b);
	}
	sliceName_->addr.b.dump();
	return sliceName_;
}

VIR::Statement *AddressElaborator::elaborateStatement(VIR::Statement *statement_)
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

VIR::StringLiteral *AddressElaborator::elaborateStringLiteral(VIR::StringLiteral *stringLiteral_)
{
	return stringLiteral_;
}

VIR::UnaryExpression *AddressElaborator::elaborateUnaryExpression(VIR::UnaryExpression *unaryExpression_)
{
	__NOT_SUPPORTED__;
	elaborateExpression(unaryExpression_->operand());

	return unaryExpression_;
}
} // namespace elaborate
} // namespace vesyla