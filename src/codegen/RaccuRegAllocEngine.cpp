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

#include "RaccuRegAllocEngine.hpp"

using namespace vesyla::cidfg;

namespace vesyla
{
namespace codegen
{
void RaccuRegAllocEngine::transform(CidfgGraph &g_)
{
}

void RaccuRegAllocEngine::create_graph(CidfgGraph &g_, Graph &g0_, VIR::Coordinate coord_, std::unordered_map<int, Graph::vertex_descriptor> &map_g2g0_, std::unordered_map<Graph::vertex_descriptor, int> &map_g02g_)
{
	std::unordered_map<int, int> selected_vertex;
	for (auto &v : g_.get_vertices())
	{
		if (v->vertex_type == Vertex::COMPUTATION_VERTEX)
		{
			ComputationVertex *vv = static_cast<ComputationVertex *>(v);
			if (!(vv->is_on_dpu) && vv->coord == coord_)
			{
				selected_vertex[v->id] = 0; // type 0 vertex: computation
			}
		}
		else if (v->vertex_type == Vertex::READ_AND_INDEX_VERTEX || v->vertex_type == Vertex::WRITE_AND_INDEX_VERTEX || v->is_sink())
		{
			if (v->coord == coord_)
			{
				selected_vertex[v->id] = 1; // type 1 vertex: sink
			}
		}
	}

	Graph g1;
	std::unordered_map<int, Graph::vertex_descriptor> map_g2g1;
	std::unordered_map<Graph::vertex_descriptor, int> map_g12g;
	for (auto &sv : selected_vertex)
	{
		Graph::vertex_descriptor vd = add_vertex(sv.second, g1);
		map_g2g1[sv.first] = vd;
		map_g12g[vd] = sv.first;
	}

	for (auto &e : g_.get_edges())
	{
		if (e->edge_type == Edge::DEPENDENCY)
		{
			continue;
		}
		if (selected_vertex.find(e->src_id) != selected_vertex.end() && selected_vertex.find(e->dest_id) != selected_vertex.end())
		{
			add_edge(map_g2g1[e->src_id], map_g2g1[e->dest_id], e->var_name, g1);
		}
	}

	Graph::vertex_iterator vi, vi_end;
	for (tie(vi, vi_end) = vertices(g1); vi != vi_end; vi++)
	{
		if (in_degree(*vi, g1) == 0 && out_degree(*vi, g1) == 0)
		{
			selected_vertex.erase(map_g12g[*vi]);
		}
	}

	for (auto &sv : selected_vertex)
	{
		Graph::vertex_descriptor vd = add_vertex(sv.second, g0_);
		map_g2g0_[sv.first] = vd;
		map_g02g_[vd] = sv.first;
	}

	for (auto &e : g_.get_edges())
	{
		if (e->edge_type == Edge::DEPENDENCY)
		{
			continue;
		}
		if (selected_vertex.find(e->src_id) != selected_vertex.end() && selected_vertex.find(e->dest_id) != selected_vertex.end())
		{
			add_edge(map_g2g0_[e->src_id], map_g2g0_[e->dest_id], e->var_name, g0_);
		}
	}
}

} // namespace codegen
} // namespace vesyla