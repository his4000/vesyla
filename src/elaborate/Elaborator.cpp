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

#include "Elaborator.hpp"

using namespace VIR;
using namespace VIR::VirEnumerations;

namespace vesyla
{
namespace elaborate
{
Elaborator::Elaborator()
{
}
Elaborator::~Elaborator()
{
}

MainProgram *Elaborator::run(MainProgram *p_)
{
	MainProgram *p = elaborate_main_program(p_);
	//cout << gen_xml(p);
	return p;
}

MainProgram *Elaborator::elaborate_main_program(MainProgram *p_)
{
	CHECK_GT(p_->body().size(), 0)
			<< "Line " << p_->lineNumber()
			<< " : No statement found. Noting to do. Abort!";
	std::unordered_map<string, var_entry> var_tab;
	_var_table.push(var_tab);
	vector<Statement *> v = p_->body();
	p_->body().clear();
	for (auto &s : v)
	{
		p_->body().push_back(elaborate_statement(s));
	}
	return p_;
}

VIR::Statement *Elaborator::elaborate_statement(VIR::Statement *s_)
{
	switch (s_->kind())
	{
	case ktAssignmentStatement:
	{
		return elaborate_assign_statement(static_cast<AssignmentStatement *>(s_));
	}
	break;
	case ktForStatement:
	{
		return elaborate_for_statement(static_cast<ForStatement *>(s_));
	}
	break;
	case ktIfStatement:
	{
		return elaborate_if_statement(static_cast<IfStatement *>(s_));
	}
	break;
	case ktPragmaStatement:
	{
		return elaborate_pragma_statement(static_cast<PragmaStatement *>(s_));
	}
	default:
		LOG(FATAL) << "Line " << s_->lineNumber()
							 << " : Unsupported statement type : " << s_->kindStr();
	}
}

VIR::Statement *Elaborator::elaborate_for_statement(VIR::ForStatement *s_)
{
	CHECK_EQ(s_->loopVariable()->kind(), ktIdentifier);

	s_->loopRange(static_cast<RangeExpression *>(elaborate_expression(s_->loopRange())));
	std::unordered_map<string, var_entry> var_tab = _var_table.top();
	_var_table.push(var_tab);

	// add iterator
	var_entry ve;
	ve.name = s_->loopVariable()->name();
	ve.object_type = otNonConstantLoopIndex;
	ve.is_fixed = false;
	ve.size = NULL;
	_var_table.top()[ve.name] = ve;
	vector<Statement *> v = s_->loopBody();
	s_->loopBody().clear();
	for (auto &s : v)
	{
		s_->loopBody().push_back(elaborate_statement(s));
	}
	_var_table.pop();
	return s_;
}
VIR::Statement *Elaborator::elaborate_if_statement(VIR::IfStatement *s_)
{
	s_->condition(elaborate_expression(s_->condition()));
	std::unordered_map<string, var_entry> var_tab;
	var_tab = _var_table.top();
	_var_table.push(var_tab);
	vector<Statement *> v = s_->thenPart();
	s_->thenPart().clear();
	for (auto &s : v)
	{
		s_->thenPart().push_back(elaborate_statement(s));
	}
	_var_table.pop();
	var_tab = _var_table.top();
	_var_table.push(var_tab);
	v = s_->thenPart();
	s_->thenPart().clear();
	for (auto &s : v)
	{
		s_->thenPart().push_back(elaborate_statement(s));
	}
	_var_table.pop();

	// semantic checking
	CHECK_EQ(s_->condition()->kind(), ktBinaryExpression)
			<< "Line " << s_->lineNumber()
			<< "Illegal IF statement condition type: " << s_->condition()->kindStr();
	if (!(static_cast<BinaryExpression *>(s_->condition())->operatorStr() == ">" ||
				static_cast<BinaryExpression *>(s_->condition())->operatorStr() == ">=" ||
				static_cast<BinaryExpression *>(s_->condition())->operatorStr() == "<" ||
				static_cast<BinaryExpression *>(s_->condition())->operatorStr() == "<=" ||
				static_cast<BinaryExpression *>(s_->condition())->operatorStr() == "==" ||
				static_cast<BinaryExpression *>(s_->condition())->operatorStr() == "~="))
	{
		LOG(FATAL) << "Line " << s_->lineNumber()
							 << " : Illegal IF statement condition operator: "
							 << static_cast<BinaryExpression *>(s_->condition())->operatorStr();
	}

	return s_;
}
VIR::Statement *Elaborator::elaborate_assign_statement(VIR::AssignmentStatement *s_)
{

	s_->rhs(elaborate_expression(s_->rhs()));
	if (s_->pragma())
	{
		s_->pragma(elaborate_assignment_pragma(s_->pragma()));
	}

	// DO NOT ELABORATE LHS AT FIRST
	// LHS WILL BE DIRECTLY ADDED WITHOUT ANY ELABORATION IF IT'S A DECLARATION
	// OTHER WISE, ELABORATE IT.

	if (!(s_->pragma() && (s_->pragma()->kind() == ktStoragePragma || s_->pragma()->kind() == ktRaccuPragma || s_->pragma()->kind() == ktDPUChainPragma)))
	{
	}

	// fill the assignment type
	if (s_->pragma())
	{
		if (s_->pragma()->kind() == ktStoragePragma)
		{
			StoragePragma *pragma = static_cast<StoragePragma *>(s_->pragma());
			if (s_->lhs().size() == 1 && s_->lhs()[0]->kind() == ktIdentifier)
			{
				if (pragma->storageType() == stRegFile)
				{
					s_->type(atDeclarative);
					var_entry ve;
					ve.name = static_cast<Identifier *>(s_->lhs()[0])->name();
					ve.object_type = otRegisterFile;
					ve.size = calc_variable_size(s_->rhs());
					ve.is_fixed = is_variable_fixed_point(s_->rhs());
					_var_table.top()[ve.name] = ve;
				}
				else if (pragma->storageType() == stMemory)
				{
					s_->type(atDeclarative);
					var_entry ve;
					ve.name = static_cast<Identifier *>(s_->lhs()[0])->name();
					ve.object_type = otMemory;
					ve.size = calc_variable_size(s_->rhs());
					ve.is_fixed = is_variable_fixed_point(s_->rhs());
					_var_table.top()[ve.name] = ve;
				}
				else
				{
					LOG(FATAL) << "Line " << s_->lineNumber()
										 << " : Illegal storage pragma type!";
				}
			}
			else
			{
				LOG(FATAL) << "Line " << s_->lineNumber()
									 << "Illegal declaration with LHS type of " << s_->lhs()[0]->kindStr();
			}
		}
		else if (s_->pragma()->kind() == ktRaccuPragma)
		{
			RaccuPragma *pragma = static_cast<RaccuPragma *>(s_->pragma());
			if (s_->lhs().size() == 1 && s_->lhs()[0]->kind() == ktIdentifier)
			{
				s_->type(atRaccuVariable);
				var_entry ve;
				ve.name = static_cast<Identifier *>(s_->lhs()[0])->name();
				ve.object_type = otRaccuVariable;
				ve.size = CREATE_INTEGER(1);
				ve.size->lineNumber(s_->lineNumber());
				_var_table.top()[ve.name] = ve;
			}
			else
			{
				LOG(FATAL) << "Line " << s_->lineNumber()
									 << "Illegal RACCU Variable declaration!";
			}
		}
		else if (s_->pragma()->kind() == ktDPUChainPragma)
		{
			if (s_->lhs().size() == 1 && s_->lhs()[0]->kind() == ktIdentifier)
			{
				s_->type(atDPUChainDeclaration);
				var_entry ve;
				ve.name = static_cast<Identifier *>(s_->lhs()[0])->name();
				ve.object_type = otDPUChain;
				ve.size = CREATE_INTEGER(1);
				ve.size->lineNumber(s_->lineNumber());
				_var_table.top()[ve.name] = ve;
			}
			else
			{
				LOG(FATAL) << "Line " << s_->lineNumber()
									 << "Illegal DPU Chain Variable declaration!";
			}
		}
		else if (s_->pragma()->kind() == ktDPUPragma)
		{
			vector<Name *> v = s_->lhs();
			s_->lhs().clear();
			for (auto &exp : v)
			{
				s_->lhs().push_back(static_cast<Name *>(elaborate_expression(exp)));
			}

			v = s_->lhs();
			s_->lhs().clear();
			for (auto &expr : v)
			{
				Expression *expr0 = elaborate_expression(expr);
				CHECK_EQ(expr0->kind(), ktSliceName);
				s_->lhs().push_back(static_cast<Name *>(expr0));
			}
			s_->type(atArithmetic);
		}
		else
		{
			LOG(FATAL) << "Line " << s_->lineNumber()
								 << "Illegal pragma used in assignment statement!";
		}
	}
	else
	{
		bool lhs_has_dpu_chain_var = false;
		bool lhs_has_scalar_var = false;
		bool lhs_has_reg_var = false;
		bool lhs_has_sram_var = false;
		bool lhs_has_new_var = false;

		for (auto &l : s_->lhs())
		{
			string name = "";
			switch (l->kind())
			{
			case ktSliceName:
			{
				name = static_cast<SliceName *>(l)->prefix()->name();
				if (_var_table.top().find(name) != _var_table.top().end())
				{
					var_entry ve = _var_table.top()[name];
					if (ve.object_type == otRegisterFile)
					{
						lhs_has_reg_var = true;
					}
					else if (ve.object_type == otMemory)
					{
						lhs_has_sram_var = true;
					}
					else
					{
						LOG(FATAL) << "Line " << s_->lineNumber()
											 << "Illegal slicename object type!";
					}
				}
				else
				{
					lhs_has_new_var = true;
				}
			}
			break;
			case ktIdentifier:
			{
				name = static_cast<Identifier *>(l)->name();
				if (_var_table.top().find(name) != _var_table.top().end())
				{
					var_entry ve = _var_table.top()[name];
					if (ve.object_type == otDPUChain)
					{
						lhs_has_dpu_chain_var = true;
					}
					else if (ve.object_type == otRegisterFile)
					{
						lhs_has_reg_var = true;
					}
					else if (ve.object_type == otMemory)
					{
						lhs_has_sram_var = true;
					}
					else
					{
						lhs_has_scalar_var = true;
					}
				}
				else
				{
					lhs_has_new_var = true;
				}
			}
			break;
			default:
			{
				LOG(FATAL) << "Line " << s_->lineNumber()
									 << " : Illegal object type in LHS of assignment!";
			}
			}
		}

		if (lhs_has_dpu_chain_var)
		{
			vector<Name *> v = s_->lhs();
			s_->lhs().clear();
			for (auto &exp : v)
			{
				s_->lhs().push_back(static_cast<Name *>(elaborate_expression(exp)));
			}
			s_->type(atDPUChainConnection);
		}
		else if (lhs_has_reg_var)
		{
			if (s_->rhs()->kind() == ktSliceName)
			{
				string name = static_cast<SliceName *>(s_->rhs())->prefix()->name();
				CHECK_NE(_var_table.top().find(name), _var_table.top().end()) << "Slicename \"" << name << "\" does not exist!";
				var_entry ve = _var_table.top()[name];
				if (ve.object_type == otRegisterFile)
				{
					vector<Name *> v = s_->lhs();
					s_->lhs().clear();
					for (auto &exp : v)
					{
						s_->lhs().push_back(static_cast<Name *>(elaborate_expression(exp)));
					}
					s_->type(atRegisterTransfer);
				}
				else if (ve.object_type == otMemory)
				{
					vector<Name *> v = s_->lhs();
					s_->lhs().clear();
					for (auto &exp : v)
					{
						s_->lhs().push_back(static_cast<Name *>(elaborate_expression(exp)));
					}
					s_->type(atMemoryTransfer);
				}
				else
				{
					LOG(FATAL) << "Line " << s_->lineNumber()
										 << " : RHS requires a slicename to form SRAM-TO-REG or REG-TO-REG transfer statement.";
				}
			}
			else
			{
				LOG(FATAL) << "Line " << s_->lineNumber()
									 << " : RHS requires a slicename to form SRAM-TO-REG or REG-TO-REG transfer statement.";
			}
		}
		else if (lhs_has_sram_var)
		{
			if (s_->rhs()->kind() == ktSliceName)
			{
				string name = static_cast<SliceName *>(s_->rhs())->prefix()->name();
				CHECK_NE(_var_table.top().find(name), _var_table.top().end()) << "Slicename \"" << name << "\" does not exist!";
				var_entry ve = _var_table.top()[name];
				if (ve.object_type == otRegisterFile)
				{
					vector<Name *> v = s_->lhs();
					s_->lhs().clear();
					for (auto &exp : v)
					{
						s_->lhs().push_back(static_cast<Name *>(elaborate_expression(exp)));
					}
					s_->type(atMemoryTransfer);
				}
				else if (ve.object_type == otMemory)
				{
					LOG(FATAL) << "Line " << s_->lineNumber()
										 << " : SRAM-TO-SRAM transfer is not supported!";
				}
				else
				{
					LOG(FATAL) << "Line " << s_->lineNumber()
										 << " : RHS requires a slicename to form REG-TO-SRAM transfer statement.";
				}
			}
			else
			{
				LOG(FATAL) << "Line " << s_->lineNumber()
									 << " : RHS requires a slicename to form REG-TO-SRAM transfer statement.";
			}
		}
		else if (lhs_has_scalar_var || lhs_has_new_var)
		{
			CHECK_EQ(s_->lhs().size(), 1)
					<< "Line " << s_->lineNumber()
					<< "Normal address calculation only allow one LHS scalar variable";
			CHECK_EQ(s_->lhs()[0]->kind(), ktIdentifier)
					<< "Line " << s_->lineNumber()
					<< "Normal address calculation should have identifier as LHS";
			s_->type(atAddressCalculation);
			if (lhs_has_new_var)
			{
				var_entry ve;
				ve.name = static_cast<Identifier *>(s_->lhs()[0])->name();
				ve.object_type = otAddressVariable;
				ve.size = NULL;
				_var_table.top()[ve.name] = ve;
			}
		}
		else
		{
			LOG(FATAL) << "Line " << s_->lineNumber()
								 << " : Unknown statement!";
		}
	}

	return s_;
}
VIR::Statement *Elaborator::elaborate_pragma_statement(VIR::PragmaStatement *s_)
{
	return s_;
}
VIR::Expression *Elaborator::elaborate_expression(VIR::Expression *e_)
{
	switch (e_->kind())
	{
	case ktBinaryExpression:
	{
		return elaborate_binary_expression(static_cast<BinaryExpression *>(e_));
	}
	break;
	case ktUnaryExpression:
	{
		return elaborate_unary_expression(static_cast<UnaryExpression *>(e_));
	}
	break;
	case ktInteger:
	{
		return elaborate_integer(static_cast<Integer *>(e_));
	}
	break;
	case ktFloatingPoint:
	{
		return elaborate_floating_point(static_cast<FloatingPoint *>(e_));
	}
	break;
	case ktFunctionCall:
	{
		return elaborate_function_call(static_cast<FunctionCall *>(e_));
	}
	break;
	case ktSliceName:
	{
		return elaborate_slice_name(static_cast<SliceName *>(e_));
	}
	break;
	case ktIdentifier:
	{
		return elaborate_identifier(static_cast<Identifier *>(e_));
	}
	break;
	case ktRangeExpression:
	{
		return elaborate_range_expression(static_cast<RangeExpression *>(e_));
	}
	break;
	case ktParenthesizedExpression:
	{
		return elaborate_parenthesized_expression(static_cast<ParenthesizedExpression *>(e_));
	}
	break;
	case ktArray:
	{
		return elaborate_array(static_cast<Array *>(e_));
	}
	break;
	case ktStringLiteral:
	{
		return e_;
	}
	break;
	default:
		LOG(FATAL) << "Line " << e_->lineNumber()
							 << " : Unsupported expression type: " << e_->kindStr();
	}
}
VIR::Expression *Elaborator::elaborate_binary_expression(VIR::BinaryExpression *e_)
{
	e_->leftOperand(elaborate_expression(e_->leftOperand()));
	e_->rightOperand(elaborate_expression(e_->rightOperand()));
	// semantic checking
	if (!(e_->operatorStr() == "+" ||
				e_->operatorStr() == "-" ||
				e_->operatorStr() == ".+" ||
				e_->operatorStr() == ".-" ||
				e_->operatorStr() == "*" ||
				e_->operatorStr() == ".*" ||
				e_->operatorStr() == "/" ||
				e_->operatorStr() == "./" ||
				e_->operatorStr() == "<<" ||
				e_->operatorStr() == ">>" ||
				e_->operatorStr() == ">" ||
				e_->operatorStr() == ">=" ||
				e_->operatorStr() == "<" ||
				e_->operatorStr() == "<=" ||
				e_->operatorStr() == "==" ||
				e_->operatorStr() == "~="))
	{
		LOG(FATAL) << "Line " << e_->lineNumber()
							 << " : Illegal Binary operator: " << e_->operatorStr();
	}

	return e_;
}
VIR::Expression *Elaborator::elaborate_unary_expression(VIR::UnaryExpression *e_)
{
	e_->operand(elaborate_expression(e_->operand()));

	// semantic checking
	if (!(e_->operatorStr() == "+" ||
				e_->operatorStr() == "-"))
	{
		LOG(FATAL) << "Line " << e_->lineNumber()
							 << " : Illegal Unary operator: " << e_->operatorStr();
	}

	return e_;
}
VIR::Expression *Elaborator::elaborate_integer(VIR::Integer *e_)
{
	return e_;
}
VIR::Expression *Elaborator::elaborate_floating_point(VIR::FloatingPoint *e_)
{
	return e_;
}
VIR::Expression *Elaborator::elaborate_function_call(VIR::FunctionCall *e_)
{
	PrimitiveFunctionCall *func = CREATE_OBJECT(PrimitiveFunctionCall);
	func->lineNumber(e_->lineNumber());
	func->name(e_->name());

	for (auto &p : e_->parameters())
	{
		Expression *exp = elaborate_expression(p);
		func->parameters().push_back(exp);
		func->parametersSplit().push_back(exp);
	}

	// semantic checking
	if (!(e_->name()->name() == "sum" ||
				e_->name()->name() == "max" ||
				e_->name()->name() == "min" ||
				e_->name()->name() == "bitsll" ||
				e_->name()->name() == "bitsra" ||
				e_->name()->name() == "abs" ||
				e_->name()->name() == "zeros" ||
				e_->name()->name() == "ones" ||
				e_->name()->name() == "fi" ||
				e_->name()->name() == "silago_agu_linear_1d" ||
				e_->name()->name() == "silago_agu_linear_2d" ||
				strncmp(e_->name()->name().c_str(), "silago_dpu_", strlen("silago_dpu_")) == 0))
	{
		LOG(FATAL) << "Line " << e_->lineNumber()
							 << " : Illegal primitive function: " << e_->name()->name();
	}

	return func;
}
VIR::Expression *Elaborator::elaborate_slice_name(VIR::SliceName *e_)
{
	string name = e_->prefix()->name();

	if (_var_table.top().find(name) == _var_table.top().end())
	{
		FunctionCall *function_call = CREATE_OBJECT(FunctionCall);
		function_call->lineNumber(e_->lineNumber());
		function_call->_name = e_->prefix();
		function_call->_parameters = e_->suffix();
		return elaborate_function_call(function_call);
	}
	else
	{
		var_entry ve = _var_table.top()[name];
		CHECK(ve.object_type == otMemory ||
					ve.object_type == otRegisterFile)
				<< "Line " << e_->lineNumber()
				<< "Illegal Slice Name : " << name;
		e_->prefix(e_->prefix());
		vector<Expression *> v = e_->suffix();
		e_->suffix().clear();
		for (auto &exp : v)
		{
			e_->suffix().push_back(elaborate_expression(exp));
		}
		e_->prefix()->isFixedPointVariable = ve.is_fixed;

		// semantic checking
		CHECK_EQ(e_->suffix().size(), 1)
				<< "Line " << e_->lineNumber()
				<< "Too much suffix in slice name" << name;

		return e_;
	}
}
VIR::Expression *Elaborator::elaborate_identifier(VIR::Identifier *e_)
{
	string name = e_->name();
	if (_var_table.top().find(name) == _var_table.top().end())
	{
		LOG(FATAL) << "Line " << e_->lineNumber()
							 << " : Unknown Identifier Name : " << name;
	}

	var_entry ve = _var_table.top()[name];
	if (ve.object_type == otMemory || ve.object_type == otRegisterFile)
	{
		SliceName *sn = CREATE_OBJECT(SliceName);
		sn->lineNumber(e_->lineNumber());
		sn->prefix(e_);
		RangeExpression *re = CREATE_OBJECT(RangeExpression);
		re->lineNumber(e_->lineNumber());
		Integer *ie1 = CREATE_INTEGER(1);
		ie1->lineNumber(e_->lineNumber());
		Integer *ie2 = CREATE_INTEGER(1);
		ie2->lineNumber(e_->lineNumber());
		re->begin(ie1);
		re->increment(ie2);
		re->end(ve.size);
		sn->suffix().push_back(elaborate_expression(re));
		sn->prefix()->isFixedPointVariable = ve.is_fixed;
		return sn;
	}
	return e_;
}
VIR::Expression *Elaborator::elaborate_range_expression(VIR::RangeExpression *e_)
{
	e_->begin(elaborate_expression(e_->begin()));
	e_->increment(elaborate_expression(e_->increment()));
	e_->end(elaborate_expression(e_->end()));

	// Here: a special case of range a:b might happen, which b<a. This implys the increment should be -1 instead of 1.
	// This grammar is only valid if both begin and end are direct integers.
	if (e_->begin()->kind() == VIR::VirEnumerations::ktInteger &&
			e_->end()->kind() == VIR::VirEnumerations::ktInteger &&
			e_->increment()->kind() == VIR::VirEnumerations::ktInteger)
	{
		if (static_cast<Integer *>(e_->begin())->value > static_cast<Integer *>(e_->end())->value)
		{
			LOG(WARNING) << "Line " << e_->lineNumber()
									 << " : Implied negative increment value detected!";
			static_cast<Integer *>(e_->increment())->value = -static_cast<Integer *>(e_->increment())->value;
		}
	}

	return e_;
}
VIR::Expression *Elaborator::elaborate_parenthesized_expression(VIR::ParenthesizedExpression *e_)
{
	e_->value(elaborate_expression(e_->value()));
	return e_;
}
VIR::Expression *Elaborator::elaborate_array(VIR::Array *e_)
{
	Array *expr = e_;
	while (true)
	{
		Array *arr = expr;
		if (arr->elements().size() == 1 && arr->elements()[0]->kind() == ktArray)
		{
			expr = static_cast<Array *>(arr->elements()[0]);
		}
		else
		{
			break;
		}
	}
	vector<Expression *> v = expr->elements();
	expr->elements().clear();
	for (auto &ele : v)
	{
		expr->elements().push_back(elaborate_expression(ele));
	}
	return expr;
}
VIR::AssignmentPragma *Elaborator::elaborate_assignment_pragma(VIR::AssignmentPragma *p_)
{
	if (p_->kind() == ktStoragePragma || p_->kind() == ktDPUChainPragma || p_->kind() == ktDPUPragma)
	{
		ReferencePragma *pr = static_cast<ReferencePragma *>(p_);
		vector<VIR::Expression *> &referenceList = pr->reference();

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
			bankAddress = CREATE_INTEGER(0);
			bankAddress->lineNumber(p_->lineNumber());
		}
		else
		{
			LOG(FATAL) << "Line " << p_->lineNumber()
								 << " : Illegal coordinate!";
		}
		CHECK_EQ(rowAddress->kind(), ktInteger);
		CHECK_EQ(colAddress->kind(), ktInteger);
		CHECK_EQ(bankAddress->kind(), ktInteger);
		Coordinate c(static_cast<Integer *>(rowAddress)->value, static_cast<Integer *>(colAddress)->value, static_cast<Integer *>(bankAddress)->value);
		pr->coordinates.push_back(c);
	}
	return p_;
}

bool Elaborator::is_variable_fixed_point(VIR::Expression *e_)
{
	VIR::Expression *expr = e_;

	if (expr->kind() == ktPrimitiveFunctionCall && static_cast<PrimitiveFunctionCall *>(expr)->name()->name() == "fi")
	{
		return true;
	}
	return false;
}

VIR::Expression *Elaborator::calc_variable_size(VIR::Expression *e_)
{
	VIR::Expression *expr = e_;

	if (expr->kind() == ktPrimitiveFunctionCall && static_cast<PrimitiveFunctionCall *>(expr)->name()->name() == "fi")
	{
		expr = static_cast<PrimitiveFunctionCall *>(expr)->parameters()[0];
	}

	if (expr->kind() == ktRangeExpression)
	{
		Integer *ie = CREATE_INTEGER(1);
		ie->lineNumber(expr->lineNumber());
		RangeExpression *re = static_cast<RangeExpression *>(expr);
		BinaryExpression *be0 = CREATE_BINARY_EXPR(botSub, re->end(), re->begin());
		be0->lineNumber(expr->lineNumber());
		BinaryExpression *be1 = CREATE_BINARY_EXPR(botMDiv, be0, re->increment());
		be1->lineNumber(expr->lineNumber());
		BinaryExpression *be2 = CREATE_BINARY_EXPR(botAdd, be1, ie);
		be2->lineNumber(expr->lineNumber());
		return be2;
	}
	else if (expr->kind() == ktPrimitiveFunctionCall)
	{
		PrimitiveFunctionCall *pfc = static_cast<PrimitiveFunctionCall *>(expr);
		if (pfc->name()->name() == "zeros" || pfc->name()->name() == "ones")
		{
			CHECK_EQ(pfc->parameters().size(), 2)
					<< "Line " << expr->lineNumber()
					<< " : zeros() or ones() should have only 2 arguments!";
			BinaryExpression *be0 = CREATE_BINARY_EXPR(botMMul, pfc->parameters()[0], pfc->parameters()[1]);
			be0->lineNumber(expr->lineNumber());
			return be0;
		}
		else
		{
			LOG(FATAL) << "Line " << e_->lineNumber()
								 << " : Illegal primitive function \"" << pfc->name()->name() << "\" for initialization!";
		}
	}
	else if (expr->kind() == ktArray)
	{
		while (true)
		{
			Array *arr = static_cast<Array *>(expr);
			if (arr->elements().size() == 1 && arr->elements()[0]->kind() == ktArray)
			{
				expr = arr->elements()[0];
			}
			else if (arr->elements().size() == 1 && arr->elements()[0]->kind() == ktRangeExpression)
			{
				Array *arr = static_cast<Array *>(expr);
				RangeExpression *re = static_cast<RangeExpression *>(arr->elements()[0]);
				Integer *ie = CREATE_INTEGER(1);
				ie->lineNumber(re->lineNumber());
				BinaryExpression *be0 = CREATE_BINARY_EXPR(botSub, re->end(), re->begin());
				be0->lineNumber(re->lineNumber());
				BinaryExpression *be1 = CREATE_BINARY_EXPR(botMDiv, be0, re->increment());
				be1->lineNumber(re->lineNumber());
				BinaryExpression *be2 = CREATE_BINARY_EXPR(botAdd, be1, ie);
				be2->lineNumber(re->lineNumber());
				return be2;
			}
			else
			{
				break;
			}
		}
		Array *arr = static_cast<Array *>(expr);
		Integer *ie = CREATE_INTEGER(arr->elements().size());
		ie->lineNumber(arr->lineNumber());
		return ie;
	}

	LOG(FATAL) << "Line " << e_->lineNumber()
						 << " : Illegal initialization method!";
	return NULL;
}

string gen_xml_object(VIR::VIRBase *obj_, int tab_ = 0)
{
	stringstream str;
	switch (obj_->kind())
	{
	case ktMainProgram:
	{
		str << setfill(' ') << setw(tab_ * 4) << ""
				<< "<MainProgram>" << endl;
		for (auto &s : static_cast<MainProgram *>(obj_)->body())
		{
			str << gen_xml_object(s, tab_ + 1);
		}
		str << setfill(' ') << setw(tab_ * 4) << ""
				<< "</MainProgram>" << endl;
	}
	break;
	case ktAssignmentStatement:
	{
		str << setfill(' ') << setw(tab_ * 4) << ""
				<< "<AssignmentStatement>" << endl;
		str << setfill(' ') << setw((tab_ + 1) * 4) << ""
				<< "<rhs>" << endl;
		str << gen_xml_object(static_cast<AssignmentStatement *>(obj_)->rhs(), tab_ + 2);
		str << setfill(' ') << setw((tab_ + 1) * 4) << ""
				<< "</rhs>" << endl;
		str << setfill(' ') << setw(tab_ * 4) << ""
				<< "</AssignmentStatement>" << endl;
	}
	break;
	case ktPrimitiveFunctionCall:
	{
		str << setfill(' ') << setw(tab_ * 4) << ""
				<< "<PrimitiveFunctionCall func_name=\"" << static_cast<PrimitiveFunctionCall *>(obj_)->name()->name() << "\" >" << endl;
		str << setfill(' ') << setw((tab_ + 1) * 4) << ""
				<< "<args>" << endl;
		for (auto &p : static_cast<PrimitiveFunctionCall *>(obj_)->parameters())
		{
			str << gen_xml_object(p, tab_ + 2);
		}
		str << setfill(' ') << setw((tab_ + 1) * 4) << ""
				<< "</args>" << endl;
		str << setfill(' ') << setw(tab_ * 4) << ""
				<< "</PrimitiveFunctionCall>" << endl;
	}
	break;
	case ktBinaryExpression:
	{
		str << setfill(' ') << setw(tab_ * 4) << ""
				<< "<BinaryExpression operator=\"" << static_cast<BinaryExpression *>(obj_)->operatorStr() << "\" >" << endl;
		str << setfill(' ') << setw((tab_ + 1) * 4) << ""
				<< "<op0>" << endl;
		str << gen_xml_object(static_cast<BinaryExpression *>(obj_)->leftOperand(), tab_ + 2);
		str << setfill(' ') << setw((tab_ + 1) * 4) << ""
				<< "</op0>" << endl;
		str << setfill(' ') << setw((tab_ + 1) * 4) << ""
				<< "<op1>" << endl;
		str << gen_xml_object(static_cast<BinaryExpression *>(obj_)->rightOperand(), tab_ + 2);
		str << setfill(' ') << setw((tab_ + 1) * 4) << ""
				<< "</op1>" << endl;
		str << setfill(' ') << setw(tab_ * 4) << ""
				<< "</BinaryExpression>" << endl;
	}
	break;
	case ktSliceName:
	{
		str << setfill(' ') << setw(tab_ * 4) << ""
				<< "<SliceName var=\"" << static_cast<SliceName *>(obj_)->prefix()->name() << "\" />" << endl;
	}
	break;
	case ktIdentifier:
	{
		str << setfill(' ') << setw(tab_ * 4) << ""
				<< "<Identifier var=\"" << static_cast<Identifier *>(obj_)->name() << "\" />" << endl;
	}
	break;
	default:
		break;
	}
	return str.str();
}
string Elaborator::gen_xml(VIR::MainProgram *p_)
{
	return gen_xml_object(p_);
}

} // namespace elaborate
} // namespace vesyla