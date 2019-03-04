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


#include "ExpressionEvaluator.hpp"

using namespace VIR;
using namespace VirEnumerations;

// TODO: try to remove the unnecessary objects after simplification (Removing these objects needs a map mechanism in the data manager)

// This method evaluates the given expression to an integer value if possible.
// Otherwise the return value is INT_MAX (indicating an error)
// mode_ argument indicates different configurations that is used for evaluating identifier.
// The default mode value is 0.
//	- mode_ = 0: Normal operation; leave variables related to loop indices and address functions intact.
//	- mode_ = 1: Replace address functions; use the loop initial value for evaluating the corresponding identifier.
//	- mode_ = 2: Replace address functions; use the loop initial value + step value for evaluating the identifier
//				 that has the same object as the given object as the third argument; for other loops use the initial value.
// The 3rd argument is used only in (mode_ == 2 and 3) that needs the object reference of a specific loop variable.
// The 4th argument is used only in cases that the expression_ type is RangeExpression.
//  - default rangeMode_ value is 0.
//  - rangeMode_ = 1: indicates that the range start value should be evaluated.
//  - rangeMode_ = 2: indicates that the range start value + increment should be evaluated.
//  - rangeMode_ = 3: indicates that the range end value should be evaluated.
//  - rangeMode_ = 4: indicates that the range repetition should be calculated and be returned.
// The 5th argument is used when there are unrolled loop values that should be replaced when reaching to a loop index.
// The return is the result of expression evaluation;
//   In case that no integer value can be extracted from the expression an INT_MAX is returned
int ExpressionEvaluator::evaluateExpressionToInt(VIR::Expression *expression_, int mode_, VIR::ObjectDeclaration *object_, int rangeMode_, map<VIR::ObjectDeclaration *, int> &unrolledMapValues)
{
	int result = INT_MAX;

	switch (expression_->kind())
	{
	case ktBinaryExpression:
	{
		BinaryExpression *binaryExpression = (BinaryExpression *)expression_;

		int leftValue = evaluateExpressionToInt(binaryExpression->leftOperand(), mode_, object_, rangeMode_, unrolledMapValues);
		int rightValue = evaluateExpressionToInt(binaryExpression->rightOperand(), mode_, object_, rangeMode_, unrolledMapValues);

		// Calculate the result only if the left and right operands are valid
		if (leftValue != INT_MAX && rightValue != INT_MAX)
			result = calculateBinaryOperation(binaryExpression->operatorType(), leftValue, rightValue);
	}
	break;

	case ktIdentifier:
	{
		Identifier *identifier = (Identifier *)expression_;
		//! Yu Yang 2017-08-01
		// Change to solve compile error caused by passing reference of object in stack
		// CHANGE BEGIN
		std::map<VIR::ObjectDeclaration *, int> umv;
		switch (identifier->object()->objectType())
		{
		case otConstant:
			result = evaluateExpressionToInt(identifier->object()->value(), 0, 0, 0, umv);
			// CHANGE END
			break;

		case otConstantLoopIndex:
		case otNonConstantLoopIndex: // For dynamic loop bounds, we assume that the start and step values are constant and only end value can be non-constant.
		{
			RangeExpression *loopRange = ((ForStatement *)identifier->indexDependency())->loopRange();

			result = (mode_ == 2 && identifier->object() == object_) ? evaluateExpressionToInt(loopRange->begin(), mode_, object_, rangeMode_, unrolledMapValues) +
																																		 evaluateExpressionToInt(loopRange->increment(), mode_, object_, rangeMode_, unrolledMapValues)
																															 : (unrolledMapValues.find(identifier->object()) != unrolledMapValues.end()) ? unrolledMapValues[identifier->object()] : evaluateExpressionToInt(loopRange->begin(), mode_, object_, rangeMode_, unrolledMapValues);
		}
		break;

		case otAffineAddressFunction:
			result = evaluateExpressionToInt(((AssignmentStatement *)identifier->indexDependency())->rhs(), mode_, object_, rangeMode_, unrolledMapValues);
			break;

		case otNonAffineAddressFunction:
			// TODO: check this
			// For mode_ == 2 it returns INT_MAX as the non-affine increment cannot be evaluated
			//if (mode_ != 2)
			result = evaluateExpressionToInt(((AssignmentStatement *)identifier->indexDependency())->rhs(), mode_, object_, rangeMode_, unrolledMapValues);
			break;

		case otAddressVariable:
		{
			AddressObjectDeclaration *addressObject = (AddressObjectDeclaration *)identifier->object();

			if (mode_ == 2) // Calculate the increment of the address variable
			{
				const int leftValue = ((Integer *)addressObject->value())->value;
				const int rightValue = evaluateExpressionToInt(addressObject->iterationAmount(), 1, object_, rangeMode_, unrolledMapValues);
				result = calculateBinaryOperation(addressObject->operatorType(), leftValue, rightValue);
			}
			else // Return the initial value
				result = ((Integer *)addressObject->value())->value;
		}
		break;

		default:
			//LOG(FATAL) << ("evaluateExpressionToInt: Not Supported!");
			break;
		}
	}
	break;

	case ktInteger:
		result = ((Integer *)expression_)->value;
		break;

	case ktParenthesizedExpression:
		result = evaluateExpressionToInt(((ParenthesizedExpression *)expression_)->value(), mode_, object_, rangeMode_, unrolledMapValues);
		break;

	case ktUnaryExpression:
	{
		UnaryExpression *unaryExpression = (UnaryExpression *)expression_;

		int opValue = evaluateExpressionToInt(unaryExpression->operand(), mode_, object_, rangeMode_, unrolledMapValues);

		// Calculate the result only if the operand value is valid
		if (opValue != INT_MAX)
			result = calculateUnaryOperation(unaryExpression->operatorType(), opValue);
	}
	break;

	case ktPrimitiveFunctionCall:
	{
		vector<VIR::Expression *> &functionParameters = ((PrimitiveFunctionCall *)expression_)->parametersSplit();

		switch (((PrimitiveFunctionCall *)expression_)->type())
		{
		case ftShiftLeft:
		case ftShiftRight:
		{
			int operand = evaluateExpressionToInt(functionParameters[0], mode_, object_, rangeMode_, unrolledMapValues);
			int shiftAmount = evaluateExpressionToInt(functionParameters[1], mode_, object_, rangeMode_, unrolledMapValues);

			// Calculate the result only if the operand and shiftAmount are valid
			if (operand != INT_MAX && shiftAmount != INT_MAX)
				result = ((PrimitiveFunctionCall *)expression_)->type() == ftShiftLeft ? operand << shiftAmount : operand >> shiftAmount;
		}
		break;

		default:
			LOG(FATAL) << ("evaluateExpressionToInt: Not Supported!");
		}
	}
	break;

	case ktRangeExpression:
	{
		if (rangeMode_ == 0)
			LOG(FATAL) << ("Misuse of range expression!");
		else
		{
			RangeExpression *rangeExpression = (RangeExpression *)expression_;
			Expression *evaluatingExpression = 0;

			switch (rangeMode_)
			{
			case 1:
				result = evaluateExpressionToInt(rangeExpression->begin(), mode_, object_, rangeMode_, unrolledMapValues);
				break;
			case 2:
				result = evaluateExpressionToInt(rangeExpression->begin(), mode_, object_, rangeMode_, unrolledMapValues) +
								 evaluateExpressionToInt(rangeExpression->increment(), mode_, object_, rangeMode_, unrolledMapValues);
				break;
			case 3:
				result = evaluateExpressionToInt(rangeExpression->end(), mode_, object_, rangeMode_, unrolledMapValues);
				break;

			case 4:
				result = int(floor((evaluateExpressionToInt(rangeExpression->end(), mode_, object_, rangeMode_, unrolledMapValues) -
														evaluateExpressionToInt(rangeExpression->begin(), mode_, object_, rangeMode_, unrolledMapValues)) /
													 float(evaluateExpressionToInt(rangeExpression->increment(), mode_, object_, rangeMode_, unrolledMapValues))));
				result += result >= 0 ? +1 : -1;
				break;
			}
		}
	}
	break;

	case ktSliceName:
	case ktFunctionCall:
		LOG(FATAL) << ("evaluateExpressionToInt: Not Supported!");
		break;

	case ktArray:
	case ktComplex:
	case ktFloatingPoint:
	case ktStringLiteral:
		LOG(FATAL) << ("evaluateExpressionToInt: Wrong expression for evaluation to integer!");
		break;
	}

	return result;
}

// This method investigates the expression to see if the expression
// can be used as either an affine or a non-affine address function.
bool ExpressionEvaluator::isExpressionAffine(VIR::Expression *expression_, bool isForLoop_)
{
	bool isAffine = true;
	switch (expression_->kind())
	{

	case ktBinaryExpression:
	{
		BinaryExpression *binaryExpression = (BinaryExpression *)expression_;

		bool isLeftAffine = isExpressionAffine(binaryExpression->leftOperand(), isForLoop_);
		bool isRightAffine = isExpressionAffine(binaryExpression->rightOperand(), isForLoop_);

		isAffine = isLeftAffine && isRightAffine && isBinaryOperatorAffine(binaryExpression->operatorType());
	}
	break;

	case ktParenthesizedExpression:
		isAffine = isExpressionAffine(((ParenthesizedExpression *)expression_)->value(), isForLoop_);
		break;

	case ktUnaryExpression:
	{
		UnaryExpression *unaryExpression = (UnaryExpression *)expression_;

		bool isOperandAffine = isExpressionAffine(unaryExpression->operand(), isForLoop_);

		isAffine = isOperandAffine && isUnaryOperatorAffine(unaryExpression->operatorType());
	}
	break;

	case ktRangeExpression:
	{
		RangeExpression *rangeExpression = (RangeExpression *)expression_;

		bool isBeginAffine = isExpressionAffine(rangeExpression->begin(), isForLoop_);
		bool isEndAffine = isExpressionAffine(rangeExpression->end(), isForLoop_);
		bool isIncrementAffine = isExpressionAffine(rangeExpression->increment(), isForLoop_);

		isAffine = isBeginAffine && isEndAffine && isIncrementAffine;
	}
	break;

	case ktIdentifier:
		isAffine = !isForLoop_ || ((Identifier *)expression_)->object()->objectType() == otConstant;
		break;

	case ktFloatingPoint:
	case ktInteger:
		// Do nothing isAffine = true;
		break;

	case ktPrimitiveFunctionCall:
	{
		isAffine = false;

		FunctionType funcType = ((PrimitiveFunctionCall *)expression_)->type();

		if (funcType == ftShiftLeft || funcType == ftShiftRight)
		{
			Identifier *operand1 = dynamic_cast<Identifier *>(((PrimitiveFunctionCall *)expression_)->parametersSplit()[0]);
			Identifier *operand2 = dynamic_cast<Identifier *>(((PrimitiveFunctionCall *)expression_)->parametersSplit()[1]);

			if (operand1 && operand1->object()->objectType() == otConstant &&
					operand2 && operand2->object()->objectType() == otConstant)
				isAffine = true;
		}
	}
	break;

	case ktArray:
	case ktComplex:
	case ktFunctionCall:
	case ktSliceName:
	case ktStringLiteral:
		LOG(FATAL) << ("isExpressionAffine: Not supported for this expression type!");
		break;
	}

	return isAffine;
}

// This method determines if the given expression only has constant values.
bool ExpressionEvaluator::isExpressionConstant(VIR::Expression *expression_)
{
	bool isConstant = false;

	switch (expression_->kind())
	{
	case ktBinaryExpression:
	{
		BinaryExpression *binaryExpression = (BinaryExpression *)expression_;

		bool isLeftConstant = isExpressionConstant(binaryExpression->leftOperand());
		bool isRightConstant = isExpressionConstant(binaryExpression->rightOperand());

		isConstant = isLeftConstant && isRightConstant;
	}
	break;

	case ktParenthesizedExpression:
		isConstant = isExpressionConstant(((ParenthesizedExpression *)expression_)->value());
		break;

	case ktUnaryExpression:
		isConstant = isExpressionConstant(((UnaryExpression *)expression_)->operand());
		break;

	case ktRangeExpression:
	{
		RangeExpression *rangeExpression = (RangeExpression *)expression_;

		bool isBeginAffine = isExpressionConstant(rangeExpression->begin());
		bool isEndAffine = isExpressionConstant(rangeExpression->end());
		bool isIncrementAffine = isExpressionConstant(rangeExpression->increment());

		isConstant = isBeginAffine && isEndAffine && isIncrementAffine;
	}
	break;

	case ktIdentifier:
		isConstant = ((Identifier *)expression_)->object()->objectType() == otConstant ? true : false;
		break;

	case ktFloatingPoint:
	case ktInteger:
		isConstant = true;
		break;

	case ktArray:
	case ktComplex:
	case ktFunctionCall:
	case ktPrimitiveFunctionCall:
	case ktSliceName:
	case ktStringLiteral:
		isConstant = false;
		break;
	}

	return isConstant;
}

// This method simplifies the expressions that have constant values inside.
// It creates a new VIR expression for the simplified expression.
VIR::Expression *ExpressionEvaluator::simplifyExpression(VIR::Expression *expression_)
{
	OutputType outputType;

	void *result = simplifyExpressionGeneral(expression_, outputType);

	if (outputType == otInteger)
	{
		result = CREATE_INTEGER(*(int *)result);
	}
	else if (outputType == otFloat)
	{
		FloatingPoint *floatingPoint = CREATE_OBJECT(FloatingPoint);

		floatingPoint->value = *(float *)result;

		result = floatingPoint;
	}

	return (Expression *)result;
}

// -------------------------------------
// ---------- Private Methods ----------

//! Yu Yang 2017-08-10
// Changed for compile error on linux
// CHANGE BEGIN
#define BINARY_OPERATION(op)    \
	result = leftOp_ op rightOp_; \
	break
// CHANGE END

// This method calculates a binary operation according to the given operator and operands
int ExpressionEvaluator::calculateBinaryOperation(BinaryOperatorType operatorType_, int leftOp_, int rightOp_)
{
	if (leftOp_ == INT_MAX || rightOp_ == INT_MAX)
		return INT_MAX;

	int result;

	switch (operatorType_)
	{
	case botAdd:
		BINARY_OPERATION(+);
	case botSub:
		BINARY_OPERATION(-);
	case botMMul:
		BINARY_OPERATION(*);
	case botEMul:
		BINARY_OPERATION(*);
	case botMDiv:
		BINARY_OPERATION(/);
	case botEDiv:
		BINARY_OPERATION(/);
	case botMLeftDiv:
		BINARY_OPERATION(/);
	case botELeftDiv:
		BINARY_OPERATION(/);
	case botEqual:
		BINARY_OPERATION(==);
	case botUnEqual:
		BINARY_OPERATION(!=);
	case botGreaterThan:
		BINARY_OPERATION(>);
	case botGreaterOrEqual:
		BINARY_OPERATION(>=);
	case botLessThan:
		BINARY_OPERATION(<);
	case botLessOrEqual:
		BINARY_OPERATION(<=);
	case botAnd:
		BINARY_OPERATION(&&);
	case botOr:
		BINARY_OPERATION(||);
	case botMPower:
		result = (int)pow((float)leftOp_, rightOp_);
		break;
	case botEPower:
		result = (int)pow((float)leftOp_, rightOp_);
		break;
	}

	return result;
}

// This is an overloaded method
float ExpressionEvaluator::calculateBinaryOperation(BinaryOperatorType operatorType_, float leftOp_, float rightOp_)
{
	if (leftOp_ == FLT_MAX || rightOp_ == FLT_MAX)
		return FLT_MAX;

	float result;

	switch (operatorType_)
	{
	case botAdd:
		BINARY_OPERATION(+);
	case botSub:
		BINARY_OPERATION(-);
	case botMMul:
		BINARY_OPERATION(*);
	case botEMul:
		BINARY_OPERATION(*);
	case botMDiv:
		BINARY_OPERATION(/);
	case botEDiv:
		BINARY_OPERATION(/);
	case botMLeftDiv:
		BINARY_OPERATION(/);
	case botELeftDiv:
		BINARY_OPERATION(/);
	case botEqual:
		BINARY_OPERATION(==);
	case botUnEqual:
		BINARY_OPERATION(!=);
	case botGreaterThan:
		BINARY_OPERATION(>);
	case botGreaterOrEqual:
		BINARY_OPERATION(>=);
	case botLessThan:
		BINARY_OPERATION(<);
	case botLessOrEqual:
		BINARY_OPERATION(<=);
	case botAnd:
		BINARY_OPERATION(&&);
	case botOr:
		BINARY_OPERATION(||);
	case botMPower:
		result = pow(leftOp_, rightOp_);
		break;
	case botEPower:
		result = pow(leftOp_, rightOp_);
		break;
	}

	return result;
}

// This method calculates a unary operation according to the given operator and operand.
int ExpressionEvaluator::calculateUnaryOperation(VIR::VirEnumerations::UnaryOperatorType operatorType_, int op_)
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

// This method says if the given binary operator is an affine operator or not.
bool ExpressionEvaluator::isBinaryOperatorAffine(VIR::VirEnumerations::BinaryOperatorType operatorType_)
{
	bool isAffine = true;

	switch (operatorType_)
	{
	case botMPower:
	case botEPower:
		isAffine = false;
		break;

	case botAdd:
	case botSub:
	case botMMul:
	case botEMul:
	case botMDiv:
	case botEDiv:
	case botMLeftDiv:
	case botELeftDiv:
	case botEqual:
	case botUnEqual:
	case botGreaterThan:
	case botGreaterOrEqual:
	case botLessThan:
	case botLessOrEqual:
	case botAnd:
	case botOr:
	default:
		break; // do nothing. It is affine
	}

	return isAffine;
}

// This method says if the given unary operator is an affine operator or not.
bool ExpressionEvaluator::isUnaryOperatorAffine(VIR::VirEnumerations::UnaryOperatorType operatorType_)
{
	bool isAffine = true;

	switch (operatorType_)
	{
	case uotTranspose:
	case uotCTranspose:
		isAffine = false;
		break;

	case uotPlus:
	case uotMinus:
	case uotNot:
	default:
		break; // do nothing. It is affine
	}

	return isAffine;
}

// This method is a general function for expression simplification.
// It is general, as it can deliver an integer, a float, or a VIR Expression as its output
// The second parameter determines if the method is called only for evaluation or also for creation.
// In the evaluation mode, only the values are returned, but in the creation mode,
// VIR objects are created for the result.
// The default mode is creation, but in some cases (e.g., binary/unary operation calculation)
// we need the method in the evaluation mode.
// The second argument is used to show the type of the result (as the result is a void pointer).
void *ExpressionEvaluator::simplifyExpressionGeneral(VIR::Expression *expression_, OutputType &outputType_, bool isEvaluationMode_)
{
	//! Yu Yang 2017-08-01
	// Change to solve compile error caused by passing reference of object in stack
	// CHANGE BEGIN
	std::map<VIR::ObjectDeclaration *, int> umv;
	// CHANGE END

	void *result = expression_;

	outputType_ = otVir; // By default the output is a VIR object

	switch (expression_->kind())
	{
	case ktArray:
	{
		vector<VIR::Expression *> *exprList = &((Array *)expression_)->elements();

		if (exprList->size() > 1 || (exprList->size() == 1 && (*exprList)[0]->kind() != ktArray))
		{
			Array *array = CREATE_OBJECT(Array);

			for (int i = 0; i < exprList->size(); ++i)
				array->elements().push_back(simplifyExpression((*exprList)[i]));

			result = array;
		}
		else // This array has only one element that is again an array, so the extra array can be removed
			result = simplifyExpression((*exprList)[0]);
	}
	break;

	case ktBinaryExpression:
	{
		static int tempIntResult;
		static float tempFloatResult;
		OutputType leftType, rightType;

		BinaryExpression *binaryExpression = (BinaryExpression *)expression_;

		void *leftOp = simplifyExpressionGeneral(binaryExpression->leftOperand(), leftType, true);
		void *rightOp = simplifyExpressionGeneral(binaryExpression->rightOperand(), rightType, true);

		if (leftType == otInteger && rightType == otInteger)
		{
			tempIntResult = calculateBinaryOperation(binaryExpression->operatorType(), *(int *)leftOp, *(int *)rightOp);

			outputType_ = otInteger;
			result = &tempIntResult;
		}
		else if (leftType == otFloat || rightType == otFloat)
		{
			float leftValue = leftType == otFloat ? *(float *)leftOp : *(int *)leftOp;
			float rightValue = rightType == otFloat ? *(float *)rightOp : *(int *)rightOp;

			tempFloatResult = calculateBinaryOperation(binaryExpression->operatorType(), leftValue, rightValue);

			outputType_ = otFloat;
			result = &tempFloatResult;
		}
		else
		{
			BinaryExpression *newBinaryExpression = CREATE_OBJECT(BinaryExpression);

			newBinaryExpression->leftOperand(simplifyExpression(binaryExpression->leftOperand()));
			newBinaryExpression->rightOperand(simplifyExpression(binaryExpression->rightOperand()));
			newBinaryExpression->operatorType(binaryExpression->operatorType());

			result = newBinaryExpression;
		}
	}
	break;

	case ktComplex:
		if (!isEvaluationMode_)
		{
			Complex *complex = CREATE_OBJECT(Complex);

			complex->iValue = ((Complex *)expression_)->iValue;
			complex->rValue = ((Complex *)expression_)->rValue;

			result = complex;
		}
		break;

	case ktFloatingPoint:
		if (isEvaluationMode_)
		{
			outputType_ = otFloat;
			result = &((FloatingPoint *)expression_)->value;
		}
		else
		{
			FloatingPoint *floatingPoint = CREATE_OBJECT(FloatingPoint);

			floatingPoint->value = ((FloatingPoint *)expression_)->value;

			result = floatingPoint;
		}
		break;

	case ktIdentifier:
	{
		ObjectDeclaration *object = ((Identifier *)expression_)->object();

		if (object->objectType() == otConstant)
		{
			result = simplifyExpressionGeneral(object->value(), outputType_, isEvaluationMode_);
		}
		else if (!isEvaluationMode_)
		{
			Identifier *oldIdentifier = (Identifier *)expression_;
			Identifier *identifer = CREATE_OBJECT(Identifier);

			identifer->name(oldIdentifier->name());
			identifer->object(oldIdentifier->object());
			identifer->dependencyType(oldIdentifier->dependencyType());
			identifer->dependentTo(oldIdentifier->dependentTo());

			result = identifer;
		}
	}
	break;

	case ktInteger:
		if (isEvaluationMode_)
		{
			outputType_ = otInteger;
			result = &((Integer *)expression_)->value;
		}
		else
			result = CREATE_INTEGER(((Integer *)expression_)->value);
		break;

	case ktUnaryExpression:
	{
		UnaryExpression *unaryExpression = (UnaryExpression *)expression_;

		switch (unaryExpression->operand()->kind())
		{
		case ktInteger:
			//! Yu Yang 2017-08-01
			// Change to solve compile error caused by passing reference of object in stack
			// CHANGE BEGIN
			result = CREATE_INTEGER(evaluateExpressionToInt(expression_, 0, 0, 0, umv));
			// CHANGE END
			break;

		case ktFloatingPoint:
		{
			float value = ((FloatingPoint *)unaryExpression->operand())->value;

			if (unaryExpression->operatorType() == uotMinus)
				value = -value;
			else if (unaryExpression->operatorType() != uotPlus)
				LOG(FATAL) << ("simplifyExpressionGeneral: Unsupported unary operator for real numbers!");
			// The value remains unchanged with operatorType() == uotPlus

			result = CREATE_FLOATING_POINT(value);
		}
		break;

		default:
			UnaryExpression *newUnaryExpression = CREATE_OBJECT(UnaryExpression);

			newUnaryExpression->operand(simplifyExpression(unaryExpression->operand()));
			newUnaryExpression->operatorType(unaryExpression->operatorType());

			result = newUnaryExpression;
			break;
		}
	}
	break;

	case ktParenthesizedExpression:
		result = simplifyExpressionGeneral(((ParenthesizedExpression *)expression_)->value(), outputType_, isEvaluationMode_);
		break;

	case ktPrimitiveFunctionCall:
	{
		PrimitiveFunctionCall *primitiveFunctionCall = (PrimitiveFunctionCall *)expression_;

		// Converting zeros/ones functions to an array of all zeros
		// zeros() and ones() functions are only supported in one dimensional forms as zeros(1, vector_size)
		if (!isEvaluationMode_ && (primitiveFunctionCall->type() == ftZeros || primitiveFunctionCall->type() == ftOnes) && primitiveFunctionCall->parameters().size() == 2 &&
				primitiveFunctionCall->parameters()[0]->kind() == ktInteger && ((Integer *)primitiveFunctionCall->parameters()[0])->value == 1)
		{
			//! Yu Yang 2017-08-01
			// Change to solve compile error caused by passing reference of object in stack
			// CHANGE BEGIN
			int arraySize = evaluateExpressionToInt(primitiveFunctionCall->parameters()[1], 0, 0, 0, umv);
			// CHANGE END

			Array *array = CREATE_OBJECT(Array);

			vector<VIR::Expression *> &arrayElements = array->elements();

			const int value = primitiveFunctionCall->type() == ftOnes ? 1 : 0;

			for (int i = 0; i < arraySize; ++i)
				arrayElements.push_back(CREATE_INTEGER(value));

			result = array;
		}
		// For fixed-point function at the moment we just simplify the function by replacing the function with its first argument.
		else if (!isEvaluationMode_ && primitiveFunctionCall->type() == ftFixedPoint)
		{
			result = simplifyExpressionGeneral(primitiveFunctionCall->parameters()[0], outputType_, isEvaluationMode_);
		}
		else
			LOG(FATAL) << ("simplifyExpressionGeneral: Improper use of primitive function!");
	}
	break;

	case ktRangeExpression:
		if (!isEvaluationMode_)
		{
			RangeExpression *oldRangeExpression = (RangeExpression *)expression_;
			RangeExpression *rangeExpression = CREATE_OBJECT(RangeExpression);

			rangeExpression->begin(simplifyExpression(oldRangeExpression->begin()));
			rangeExpression->end(simplifyExpression(oldRangeExpression->end()));
			rangeExpression->increment(simplifyExpression(oldRangeExpression->increment()));

			result = rangeExpression;
		}
		break;

	case ktStringLiteral:
		if (!isEvaluationMode_)
		{
			StringLiteral *stringLiteral = CREATE_OBJECT(StringLiteral);

			stringLiteral->value = ((StringLiteral *)expression_)->value;

			result = stringLiteral;
		}
		break;

	case ktSliceName:
		// Do nothing, so returns the original expression
		// TODO: In future it may need some evaluations
		break;

	case ktFunctionCall:
		// Do nothing, so returns the original expression
		// TODO: In future it may need some evaluations
		break;
	}

	return result;
}
