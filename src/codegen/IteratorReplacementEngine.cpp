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

#include "IteratorReplacementEngine.hpp"

using namespace vesyla::cidfg;

namespace vesyla
{
namespace codegen
{
void IteratorReplacementEngine::transform(cidfg::CidfgGraph &g_)
{
	set<int> removed_vertices;
	for (auto &e : g_.get_edges())
	{
		if (e->edge_type == cidfg::Edge::DEPENDENCY)
		{
			continue;
		}

		if (g_.get_vertex(e->src_id)->vertex_type != cidfg::Vertex::VAR_VERTEX)
		{
			continue;
		}

		VarVertex *v_src = static_cast<VarVertex *>(g_.get_vertex(e->src_id));
		if (v_src->is_iterator == false)
		{
			continue;
		}

		if (g_.get_vertex(e->dest_id)->vertex_type != cidfg::Vertex::COMPUTATION_VERTEX)
		{
			continue;
		}

		ComputationVertex *v_dest = static_cast<ComputationVertex *>(g_.get_vertex(e->dest_id));
		if (v_dest->func_name != "*" && v_dest->func_name != "MULT" && v_dest->func_name != "mult")
		{
			continue;
		}

		// Now, the iterator should be replaced
		string temp_var_name = "it_var_" + v_src->var_name;
		int parent_id;
		int child_index;
		g_.get_parent(v_src->id, parent_id, child_index);
		CHECK_GE(parent_id, 0);
		CHECK_EQ(child_index, 0);

		LoopVertex *v_loop = static_cast<LoopVertex *>(g_.get_vertex(parent_id));
		g_.get_parent(v_loop->id, parent_id, child_index); // get the parent of the loop vertex

		ComputationVertex v_ld;
		v_ld.coord = v_src->coord;
		v_ld.func_name = "ld";
		int id_ld = g_.add_vertex(v_ld, parent_id, child_index);

		ComputationVertex v_add;
		v_add.coord = v_src->coord;
		v_add.func_name = "+";
		int id_add = g_.add_vertex(v_add, v_loop->id, 0);

		VarSinkVertex v_sk;
		v_sk.coord = v_src->coord;
		v_sk.var_name = temp_var_name;
		int id_sk = g_.add_vertex(v_sk, parent_id, child_index);

		Edge *ex;
		ex = g_.get_edge(g_.get_in_edge(v_loop->id, 0));
		int id_loop_start = ex->src_id;
		int port_loop_start = ex->src_port;
		ex = g_.get_edge(g_.get_in_edge(v_loop->id, 1));
		int id_loop_inc = ex->src_id;
		int port_loop_inc = ex->src_port;

		CHECK_EQ(g_.get_vertex(id_loop_start)->vertex_type, Vertex::CONST_VERTEX);
		CHECK_EQ(g_.get_vertex(id_loop_inc)->vertex_type, Vertex::CONST_VERTEX);
		id_loop_start = g_.add_vertex(*(static_cast<ConstVertex *>(g_.get_vertex(id_loop_start))), parent_id, child_index);
		id_loop_inc = g_.add_vertex(*(static_cast<ConstVertex *>(g_.get_vertex(id_loop_inc))), parent_id, child_index);

		Edge e0(id_loop_start, port_loop_start, id_ld, 1);
		Edge e1(id_loop_inc, port_loop_inc, id_add, 1);
		Edge e2(id_ld, 0, id_add, 0, temp_var_name);
		Edge e3(id_add, 0, id_sk, 0, temp_var_name);
		g_.add_edge(e0);
		g_.add_edge(e1);
		g_.add_edge(e2);
		g_.add_edge(e3);

		for (auto &e : g_.get_edges())
		{
			if (e->src_id == v_src->id)
			{
				e->src_id = id_ld;
				e->var_name = temp_var_name;
				Edge e0(e->dest_id, 100, id_add, 100, "WAR", Edge::DEPENDENCY);
				g_.add_edge(e0);
			}
		}

		removed_vertices.insert(v_src->id);
	}

	for (auto &id : removed_vertices)
	{
		g_.del_vertex(id);
	}
}
} // namespace codegen
} // namespace vesyla