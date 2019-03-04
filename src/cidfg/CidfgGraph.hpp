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

#ifndef __VESYLA_CIDFG_CIDFG_GRAPH_HPP__
#define __VESYLA_CIDFG_CIDFG_GRAPH_HPP__

#include "ConstVertex.hpp"
#include "VarVertex.hpp"
#include "RegVarVertex.hpp"
#include "SramVarVertex.hpp"
#include "ComputationVertex.hpp"
#include "IndexingVertex.hpp"
#include "ReadingVertex.hpp"
#include "WritingVertex.hpp"
#include "SinkVertex.hpp"
#include "GeneralHierarchicalVertex.hpp"
#include "BranchVertex.hpp"
#include "LoopVertex.hpp"
#include "RootVertex.hpp"
#include "ReadAndIndexVertex.hpp"
#include "WriteAndIndexVertex.hpp"
#include "MergeVertex.hpp"
#include "RaccuInstrVertex.hpp"
#include "DpuInstrVertex.hpp"
#include "HierarchicalInstrVertex.hpp"
#include "LoophInstrVertex.hpp"
#include "LooptInstrVertex.hpp"
#include "SwbInstrVertex.hpp"
#include "RefiInstrVertex.hpp"
#include "BranchInstrVertex.hpp"
#include "WaitInstrVertex.hpp"
#include "JumpInstrVertex.hpp"
#include "RouteInstrVertex.hpp"
#include "SramInstrVertex.hpp"
#include "RegSinkVertex.hpp"
#include "SramSinkVertex.hpp"
#include "VarSinkVertex.hpp"

#include "Edge.hpp"

#include <set>

namespace vesyla
{
namespace cidfg
{
class CidfgGraph
{
private:
	std::unordered_map<int, Vertex *> _vertices;
	std::unordered_map<int, Edge *> _edges;
	int id_counter;

public:
	CidfgGraph();
	~CidfgGraph();
	CidfgGraph &operator=(const CidfgGraph &rhs);

	int add_vertex(ConstVertex v_, int parent_vertex_id_ = -1, int child_index_ = 0);
	int add_vertex(VarVertex v_, int parent_vertex_id_ = -1, int child_index_ = 0);
	int add_vertex(RegVarVertex v_, int parent_vertex_id_ = -1, int child_index_ = 0);
	int add_vertex(SramVarVertex v_, int parent_vertex_id_ = -1, int child_index_ = 0);
	int add_vertex(ComputationVertex v_, int parent_vertex_id_ = -1, int child_index_ = 0);
	int add_vertex(IndexingVertex v_, int parent_vertex_id_ = -1, int child_index_ = 0);
	int add_vertex(ReadingVertex v_, int parent_vertex_id_ = -1, int child_index_ = 0);
	int add_vertex(WritingVertex v_, int parent_vertex_id_ = -1, int child_index_ = 0);
	int add_vertex(RegSinkVertex v_, int parent_vertex_id_ = -1, int child_index_ = 0);
	int add_vertex(SramSinkVertex v_, int parent_vertex_id_ = -1, int child_index_ = 0);
	int add_vertex(VarSinkVertex v_, int parent_vertex_id_ = -1, int child_index_ = 0);
	int add_vertex(GeneralHierarchicalVertex v_, int parent_vertex_id_ = -1, int child_index_ = 0);
	int add_vertex(BranchVertex v_, int parent_vertex_id_ = -1, int child_index_ = 0);
	int add_vertex(LoopVertex v_, int parent_vertex_id_ = -1, int child_index_ = 0);
	int add_vertex(RootVertex v_, int parent_vertex_id_ = -1, int child_index_ = 0);
	int add_vertex(ReadAndIndexVertex v_, int parent_vertex_id_ = -1, int child_index_ = 0);
	int add_vertex(WriteAndIndexVertex v_, int parent_vertex_id_ = -1, int child_index_ = 0);
	int add_vertex(MergeVertex v_, int parent_vertex_id_ = -1, int child_index_ = 0);
	int add_vertex(RaccuInstrVertex v_, int parent_vertex_id_ = -1, int child_index_ = 0);
	int add_vertex(DpuInstrVertex v_, int parent_vertex_id_ = -1, int child_index_ = 0);
	int add_vertex(HierarchicalInstrVertex v_, int parent_vertex_id_ = -1, int child_index_ = 0);
	int add_vertex(LoophInstrVertex v_, int parent_vertex_id_ = -1, int child_index_ = 0);
	int add_vertex(LooptInstrVertex v_, int parent_vertex_id_ = -1, int child_index_ = 0);
	int add_vertex(SwbInstrVertex v_, int parent_vertex_id_ = -1, int child_index_ = 0);
	int add_vertex(RefiInstrVertex v_, int parent_vertex_id_ = -1, int child_index_ = 0);
	int add_vertex(BranchInstrVertex v_, int parent_vertex_id_ = -1, int child_index_ = 0);
	int add_vertex(WaitInstrVertex v_, int parent_vertex_id_ = -1, int child_index_ = 0);
	int add_vertex(JumpInstrVertex v_, int parent_vertex_id_ = -1, int child_index_ = 0);
	int add_vertex(RouteInstrVertex v_, int parent_vertex_id_ = -1, int child_index_ = 0);
	int add_vertex(SramInstrVertex v_, int parent_vertex_id_ = -1, int child_index_ = 0);
	int add_edge(Edge e_);
	void del_vertex(int id_);
	void del_edge(int id_);

	int get_out_edge(int id_, int out_port_);
	int get_in_edge(int id_, int in_port_);

	Vertex *get_vertex(int id_);
	Edge *get_edge(int id_);
	vector<Vertex *> get_vertices() const;
	vector<Edge *> get_edges() const;

	string generate_dot_graph();
	string generate_dot_graph_for_vertex(int vid, set<int> &vertex_set_);

	bool check_graph();
	void print_graph();

	void get_parent(int id_, int &parent_id_, int &child_index_);
};
} // namespace cidfg
} // namespace vesyla

#endif // __VESYLA_CIDFG_CIDFG_GRAPH_HPP__