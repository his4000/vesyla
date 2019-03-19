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

#ifndef __VESYLA_CODEGEN_COMPUTATION_VERTEX_CONVERTER_HPP__
#define __VESYLA_CODEGEN_COMPUTATION_VERTEX_CONVERTER_HPP__

#include "cidfg/CidfgGraph.hpp"
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/copy.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/sequential_vertex_coloring.hpp>
#include <cidfg/VarTable.hpp>
#include "BIR/BIRIncludeAll.hpp"
#include "VManager/MemoryManager.hpp"
#include "PrimitiveFuncLib.hpp"

namespace vesyla
{
namespace codegen
{
class ComputationVertexConverter
{
private:
	PrimitiveFuncLib _pf_lib;
	int _raccu_reg_file_depth;

public:
	ComputationVertexConverter();
	~ComputationVertexConverter();

	void convert(cidfg::CidfgGraph &g_);
	void convert_raccu_vertex(cidfg::CidfgGraph &g_, vector<int> vertex_id_list_, std::map<string, int> raccu_reg_id_, std::unordered_map<string, string> vt_);
	vector<int> find_all_raccu_var_edges(cidfg::CidfgGraph &g_, vector<int> vertex_id_list_);
	std::map<string, int> allocate_raccu_reg(cidfg::CidfgGraph &g_, vector<int> edge_list_, std::map<string, int> curr_reg_id_, std::unordered_map<string, string> &vt_);
	std::map<string, int> allocate_raccu_reg_for_connected_graph(cidfg::CidfgGraph &g_, vector<int> edge_list_, std::map<string, int> curr_reg_id_, std::unordered_map<string, string> &vt_);

private:
	void fill_dpu_instr(BIR::DPUInstruction *instr_, cidfg::CidfgGraph &g_, int id_);
	void fill_raccu_instr(BIR::RACCUInstruction *instr_, cidfg::CidfgGraph &g_, int id_);

	void pre_fill_all_known_edges(cidfg::CidfgGraph &g_, vector<int> vector_id_list_, std::unordered_map<string, string> &var_reg_map_);
};
} // namespace codegen
} // namespace vesyla

#endif // __VESYLA_CODEGEN_COMPUTATION_VERTEX_CONVERTER_HPP__