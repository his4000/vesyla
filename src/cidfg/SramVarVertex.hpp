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

#ifndef __VESYLA_CIDFG_SRAM_VAR_VERTEX_HPP__
#define __VESYLA_CIDFG_SRAM_VAR_VERTEX_HPP__

#include "SourceVertex.hpp"

namespace vesyla
{
namespace cidfg
{
class SramVarVertex : public SourceVertex
{
public:
	ValueType value_type;
	vector<int> int_data_array;
	vector<float> float_data_array;
	VIR::Coordinate sram_coord;

public:
	SramVarVertex();
	SramVarVertex(VIR::AssignmentStatement *stmt_);
	~SramVarVertex();
	string dump();

private:
	vector<int> convert_to_int_array(VIR::Expression *e_);
	vector<float> convert_to_float_array(VIR::Expression *e_);
};
} // namespace cidfg
} // namespace vesyla

#endif // __VESYLA_CIDFG_SRAM_VAR_VERTEX_HPP__