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

#include "RaccuInstrVertex.hpp"

namespace vesyla
{
namespace cidfg
{
RaccuInstrVertex::RaccuInstrVertex()
{
	vertex_type = RACCU_INSTR_VERTEX;
	op0_type = RACCU_OPERAND_IMMEDIATE;
	op1_type = RACCU_OPERAND_IMMEDIATE;
	op0_value = 0;
	op1_value = 1;
	func_name = "silago_raccu_undefined";
}
RaccuInstrVertex::~RaccuInstrVertex()
{
}
} // namespace cidfg
} // namespace vesyla