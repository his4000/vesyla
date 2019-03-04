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

#include "Vertex.hpp"

namespace vesyla
{
namespace cidfg
{
Vertex::Vertex()
{
}
Vertex::~Vertex()
{
}
bool Vertex::is_hierarchical()
{
	if (vertex_type == ROOT_VERTEX ||
			vertex_type == GENERAL_HIERARCHICAL_VERTEX ||
			vertex_type == BRANCH_VERTEX ||
			vertex_type == LOOP_VERTEX ||
			vertex_type == HIERARCHICAL_INSTR_VERTEX)
	{
		return true;
	}
	return false;
}
bool Vertex::is_instr()
{
	if (vertex_type == RACCU_INSTR_VERTEX ||
			vertex_type == DPU_INSTR_VERTEX ||
			vertex_type == SWB_INSTR_VERTEX ||
			vertex_type == REFI_INSTR_VERTEX ||
			vertex_type == LOOPH_INSTR_VERTEX ||
			vertex_type == LOOPT_INSTR_VERTEX ||
			vertex_type == BRANCH_INSTR_VERTEX ||
			vertex_type == JUMP_INSTR_VERTEX ||
			vertex_type == WAIT_INSTR_VERTEX ||
			vertex_type == ROUTE_INSTR_VERTEX ||
			vertex_type == SRAM_INSTR_VERTEX ||
			vertex_type == HIERARCHICAL_INSTR_VERTEX)
	{
		return true;
	}
	return false;
}

bool Vertex::is_source()
{
	if (vertex_type == VAR_VERTEX ||
			vertex_type == REG_VAR_VERTEX ||
			vertex_type == SRAM_VAR_VERTEX ||
			vertex_type == CONST_VERTEX)
	{
		return true;
	}
	return false;
}

bool Vertex::is_sink()
{
	if (vertex_type == REG_SINK_VERTEX ||
			vertex_type == SRAM_SINK_VERTEX ||
			vertex_type == VAR_SINK_VERTEX)
	{
		return true;
	}
	return false;
}

} // namespace cidfg
} // namespace vesyla