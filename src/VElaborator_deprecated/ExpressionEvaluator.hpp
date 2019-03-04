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


#ifndef _ExpressionEvaluator_H_
#define _ExpressionEvaluator_H_

#include "util/Common.hpp"
#include "VIR/VIRIncludeAll.hpp"
#include "BIR/BIRIncludeAll.hpp"
#include "VIR/BinaryOperatorType.hpp"
#include "VIR/UnaryOperatorType.hpp"
#include "VManager/MemoryManager.hpp"
#include <math.h>
#include <map>

class ExpressionEvaluator
{
public:
//! Yu Yang 2017-08-01
// Change to solve compile error caused by passing reference of object in stack
// CHANGE BEGIN
	int evaluateExpressionToInt(VIR::Expression * expression_, int mode_, VIR::ObjectDeclaration * object_, int rangeMode_, std::map<VIR::ObjectDeclaration *, int> & unrolledMapValues);
// CHAGNE END

	bool isExpressionAffine(VIR::Expression * expression_, bool isForLoop_ = false);
	bool isExpressionConstant(VIR::Expression * expression_);
	VIR::Expression * simplifyExpression(VIR::Expression * expression_);

private:
	enum OutputType { otInteger, otFloat, otVir };

	int calculateBinaryOperation(VIR::VirEnumerations::BinaryOperatorType operatorType_, int leftOp_, int rightOp_);
	float calculateBinaryOperation(VIR::VirEnumerations::BinaryOperatorType operatorType_, float leftOp_, float rightOp_);
	int calculateUnaryOperation(VIR::VirEnumerations::UnaryOperatorType operatorType_, int op_);
	bool isBinaryOperatorAffine(VIR::VirEnumerations::BinaryOperatorType operatorType_);
	bool isUnaryOperatorAffine(VIR::VirEnumerations::UnaryOperatorType operatorType_);
	void * simplifyExpressionGeneral(VIR::Expression * expression_, OutputType & outputType_, bool isEvaluationMode_ = false);
};

#endif // _ExpressionEvaluator_H_
