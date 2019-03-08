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

#ifndef __VESYLA_ELABORATE_ELABORATOR_HPP__
#define __VESYLA_ELABORATE_ELABORATOR_HPP__

#include "util/Common.hpp"
#include "VIR/VIRIncludeAll.hpp"
#include "VManager/MemoryManager.hpp"

namespace vesyla
{
namespace elaborate
{

class Elaborator
{

	struct var_entry
	{
		string name;
		VIR::VirEnumerations::ObjectType object_type;
		VIR::Expression *size;
		bool is_fixed;

		var_entry()
		{
			name = "";
			object_type = VIR::VirEnumerations::otNone;
			size = NULL;
			is_fixed = false;
		}
	};

private:
	stack<std::unordered_map<string, var_entry>> _var_table;

public:
	Elaborator();
	~Elaborator();
	VIR::MainProgram *run(VIR::MainProgram *p_);
	string gen_xml(VIR::MainProgram *p_);

private:
	VIR::MainProgram *elaborate_main_program(VIR::MainProgram *p_);
	VIR::Statement *elaborate_statement(VIR::Statement *s_);
	VIR::Statement *elaborate_for_statement(VIR::ForStatement *s_);
	VIR::Statement *elaborate_if_statement(VIR::IfStatement *s_);
	VIR::Statement *elaborate_assign_statement(VIR::AssignmentStatement *s_);
	VIR::Statement *elaborate_pragma_statement(VIR::PragmaStatement *s_);
	VIR::Expression *elaborate_expression(VIR::Expression *e_);
	VIR::Expression *elaborate_binary_expression(VIR::BinaryExpression *e_);
	VIR::Expression *elaborate_unary_expression(VIR::UnaryExpression *e_);
	VIR::Expression *elaborate_integer(VIR::Integer *e_);
	VIR::Expression *elaborate_floating_point(VIR::FloatingPoint *e_);
	VIR::Expression *elaborate_function_call(VIR::FunctionCall *e_);
	VIR::Expression *elaborate_slice_name(VIR::SliceName *e_);
	VIR::Expression *elaborate_identifier(VIR::Identifier *e_);
	VIR::Expression *elaborate_range_expression(VIR::RangeExpression *e_);
	VIR::Expression *elaborate_parenthesized_expression(VIR::ParenthesizedExpression *e_);
	VIR::Expression *elaborate_array(VIR::Array *e_);
	VIR::AssignmentPragma *elaborate_assignment_pragma(VIR::AssignmentPragma *p_);

	VIR::Expression *calc_variable_size(VIR::Expression *e_);
	bool is_variable_fixed_point(VIR::Expression *e_);
};
} // namespace elaborate
} // namespace vesyla
#endif // __VESYLA_ELABORATE_ELABORATOR_HPP__