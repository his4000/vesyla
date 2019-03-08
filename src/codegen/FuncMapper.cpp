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

#include "FuncMapper.hpp"

using namespace vesyla::cidfg;

namespace vesyla
{
namespace codegen
{

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, int, int> CustomGraph;

void find_neighbours(CidfgGraph &g_, int id_, set<int> &vertices_)
{
	Vertex *v = g_.get_vertex(id_);
	if (v->vertex_type == Vertex::COMPUTATION_VERTEX)
	{
		if (vertices_.find(id_) != vertices_.end())
		{
			return;
		}
		vertices_.insert(id_);
		for (auto &e : g_.get_edges())
		{
			if (e->edge_type == Edge::DEPENDENCY)
			{
				continue;
			}
			if (e->src_id == id_)
			{
				find_neighbours(g_, e->dest_id, vertices_);
			}
			else if (e->dest_id == id_)
			{
				find_neighbours(g_, e->src_id, vertices_);
			}
		}
	}
}

void find_signature(CidfgGraph &g_, CustomGraph &g0_, CustomGraph::vertex_descriptor vd_, std::unordered_map<int, CustomGraph::vertex_descriptor> &map_cidfg2custom_, std::unordered_map<CustomGraph::vertex_descriptor, int> &map_custom2cidfg_, FuncMapper::Mapper &fm_, set<int> &visited_vertices_)
{
	visited_vertices_.insert(map_custom2cidfg_[vd_]);
	ComputationVertex *cv = static_cast<ComputationVertex *>(g_.get_vertex(map_custom2cidfg_[vd_]));
	fm_.signature += cv->func_name + "(";
	int i = 0;
	while (true)
	{
		if (g_.get_in_edge(map_custom2cidfg_[vd_], i) < 0)
		{
			break;
		}
		if (i > 0)
		{
			fm_.signature += ",";
		}
		int id = g_.get_in_edge(map_custom2cidfg_[vd_], i);
		CustomGraph::in_edge_iterator iei, iei_end;
		for (tie(iei, iei_end) = boost::in_edges(vd_, g0_); iei != iei_end; iei++)
		{
			if (g0_[*iei] == id)
			{
				find_signature(g_, g0_, source(*iei, g0_), map_cidfg2custom_, map_custom2cidfg_, fm_, visited_vertices_);
				break;
			}
		}
		if (iei == iei_end)
		{
			fm_.signature += "V";
			fm_.arguments.push_back(id);
		}
		i++;
	}
	fm_.signature += ")";
}

void FuncMapper::transform(cidfg::CidfgGraph &g_)
{
	// Find computation vertices connected to DPU computation vertices.
	set<int> computation_vertices;
	for (auto &v : g_.get_vertices())
	{
		if ((v->vertex_type == Vertex::COMPUTATION_VERTEX) && (static_cast<ComputationVertex *>(v)->is_on_dpu == true))
		{
			find_neighbours(g_, v->id, computation_vertices);
		}
	}

	// Build DAG
	CustomGraph g0;
	std::unordered_map<int, CustomGraph::vertex_descriptor> map_cidfg2custom;
	std::unordered_map<CustomGraph::vertex_descriptor, int> map_custom2cidfg;
	CustomGraph::vertex_descriptor vd;
	for (auto &id : computation_vertices)
	{
		vd = add_vertex(id, g0);
		map_cidfg2custom[id] = vd;
		map_custom2cidfg[vd] = id;
	}
	for (auto &e : g_.get_edges())
	{
		if (computation_vertices.find(e->src_id) != computation_vertices.end() && computation_vertices.find(e->dest_id) != computation_vertices.end())
		{
			Vertex *src_v = g_.get_vertex(e->src_id);
			Vertex *dest_v = g_.get_vertex(e->dest_id);
			if (!(static_cast<ComputationVertex *>(src_v)->is_on_dpu && static_cast<ComputationVertex *>(dest_v)->is_on_dpu))
			{
				add_edge(map_cidfg2custom[e->src_id], map_cidfg2custom[e->dest_id], e->id, g0);
			}
		}
	}
	// Map for each connected region
	set<int> visited_vertices;
	set<int> removed_vertices;
	CustomGraph::vertex_iterator vi, vi_end;
	for (tie(vi, vi_end) = vertices(g0); vi != vi_end; vi++)
	{
		if (visited_vertices.find(map_custom2cidfg[*vi]) != visited_vertices.end())
		{
			continue;
		}
		// Find the most bottom node
		CustomGraph::vertex_descriptor vd_bottom;
		bool flag_found = false;
		vd = *vi;
		while (true)
		{
			vector<size_t> vec;
			CustomGraph::out_edge_iterator oei, oei_end;
			for (tie(oei, oei_end) = out_edges(vd, g0); oei != oei_end; oei++)
			{
				vec.push_back(target(*oei, g0));
			}
			if (vec.size() > 1)
			{
				LOG(ERROR) << "Multiple outputs for nodes in DPU vertex group!";
				flag_found = false;
				break;
			}
			else if (vec.size() == 1)
			{
				vd = vec[0];
			}
			else
			{
				// it's the bottom node
				vd_bottom = *vi;
				flag_found = true;
				break;
			}
		}
		if (!flag_found)
		{
			continue;
		}

		set<int> visited_v;
		vd = vd_bottom;
		Mapper m;
		find_signature(g_, g0, vd, map_cidfg2custom, map_custom2cidfg, m, visited_v);
		for (auto &id : visited_v)
		{
			visited_vertices.insert(id);
		}

		LOG(DEBUG) << m.signature;

		// compare
		int parent_id;
		int child_index;
		g_.get_parent(map_custom2cidfg[vd], parent_id, child_index);
		if (m.signature == "+(sum(.*(V,V)),V)")
		{
			cidfg::ComputationVertex cv;
			cv.coord = g_.get_vertex(map_custom2cidfg[vd])->coord;
			cv.func_name = "silago_dpu_preserving_mac";
			cv.is_on_dpu = true;
			int id_cv = g_.add_vertex(cv, parent_id, child_index);

			g_.get_edge(m.arguments[0])->dest_id = id_cv;
			g_.get_edge(m.arguments[0])->dest_port = 1;
			g_.get_edge(m.arguments[1])->dest_id = id_cv;
			g_.get_edge(m.arguments[1])->dest_port = 2;
			g_.get_edge(m.arguments[2])->dest_id = id_cv;
			g_.get_edge(m.arguments[2])->dest_port = 0;

			for (auto &e : g_.get_edges())
			{
				if (visited_v.find(e->src_id) != visited_v.end() && visited_v.find(e->dest_id) == visited_v.end())
				{
					e->src_id = id_cv;
				}
				else if (visited_v.find(e->src_id) == visited_v.end() && visited_v.find(e->dest_id) != visited_v.end())
				{
					e->dest_id = id_cv;
				}
			}

			for (auto &id : visited_v)
			{
				removed_vertices.insert(id);
			}
		}
		else if (m.signature == "+(V,sum(.*(V,V)))")
		{
			cidfg::ComputationVertex cv;
			cv.coord = g_.get_vertex(map_custom2cidfg[vd])->coord;
			cv.func_name = "silago_dpu_preserving_mac";
			cv.is_on_dpu = true;
			int id_cv = g_.add_vertex(cv, parent_id, child_index);

			g_.get_edge(m.arguments[0])->dest_id = id_cv;
			g_.get_edge(m.arguments[0])->dest_port = 0;
			g_.get_edge(m.arguments[1])->dest_id = id_cv;
			g_.get_edge(m.arguments[1])->dest_port = 1;
			g_.get_edge(m.arguments[2])->dest_id = id_cv;
			g_.get_edge(m.arguments[2])->dest_port = 2;

			for (auto &e : g_.get_edges())
			{
				if (visited_v.find(e->src_id) != visited_v.end() && visited_v.find(e->dest_id) == visited_v.end())
				{
					e->src_id = id_cv;
				}
				else if (visited_v.find(e->src_id) == visited_v.end() && visited_v.find(e->dest_id) != visited_v.end())
				{
					e->dest_id = id_cv;
				}
			}

			for (auto &id : visited_v)
			{
				removed_vertices.insert(id);
			}
		}
		else if (m.signature == "sum(.*(V,V))")
		{
			cidfg::ComputationVertex cv;
			cv.coord = g_.get_vertex(map_custom2cidfg[vd])->coord;
			cv.func_name = "silago_dpu_mac";
			cv.is_on_dpu = true;
			int id_cv = g_.add_vertex(cv, parent_id, child_index);

			g_.get_edge(m.arguments[0])->dest_id = id_cv;
			g_.get_edge(m.arguments[0])->dest_port = 0;
			g_.get_edge(m.arguments[1])->dest_id = id_cv;
			g_.get_edge(m.arguments[1])->dest_port = 1;

			for (auto &e : g_.get_edges())
			{
				if (visited_v.find(e->src_id) != visited_v.end() && visited_v.find(e->dest_id) == visited_v.end())
				{
					e->src_id = id_cv;
				}
				else if (visited_v.find(e->src_id) == visited_v.end() && visited_v.find(e->dest_id) != visited_v.end())
				{
					e->dest_id = id_cv;
				}
			}

			for (auto &id : visited_v)
			{
				removed_vertices.insert(id);
			}
		}
		else if (m.signature == "sum(.*(V,+(V,V)))")
		{
			cidfg::ComputationVertex cv;
			cv.coord = g_.get_vertex(map_custom2cidfg[vd])->coord;
			cv.func_name = "silago_dpu_symmetric_mac";
			cv.is_on_dpu = true;
			int id_cv = g_.add_vertex(cv, parent_id, child_index);

			g_.get_edge(m.arguments[0])->dest_id = id_cv;
			g_.get_edge(m.arguments[0])->dest_port = 0;
			g_.get_edge(m.arguments[1])->dest_id = id_cv;
			g_.get_edge(m.arguments[1])->dest_port = 1;
			g_.get_edge(m.arguments[2])->dest_id = id_cv;
			g_.get_edge(m.arguments[2])->dest_port = 2;

			for (auto &e : g_.get_edges())
			{
				if (visited_v.find(e->src_id) != visited_v.end() && visited_v.find(e->dest_id) == visited_v.end())
				{
					e->src_id = id_cv;
				}
				else if (visited_v.find(e->src_id) == visited_v.end() && visited_v.find(e->dest_id) != visited_v.end())
				{
					e->dest_id = id_cv;
				}
			}

			for (auto &id : visited_v)
			{
				removed_vertices.insert(id);
			}
		}
		else if (m.signature == "sum(abs(-(V,V)))")
		{
			cidfg::ComputationVertex cv;
			cv.coord = g_.get_vertex(map_custom2cidfg[vd])->coord;
			cv.func_name = "silago_dpu_abs_sub_acc";
			cv.is_on_dpu = true;
			int id_cv = g_.add_vertex(cv, parent_id, child_index);

			g_.get_edge(m.arguments[0])->dest_id = id_cv;
			g_.get_edge(m.arguments[0])->dest_port = 0;
			g_.get_edge(m.arguments[1])->dest_id = id_cv;
			g_.get_edge(m.arguments[1])->dest_port = 1;

			for (auto &e : g_.get_edges())
			{
				if (visited_v.find(e->src_id) != visited_v.end() && visited_v.find(e->dest_id) == visited_v.end())
				{
					e->src_id = id_cv;
				}
				else if (visited_v.find(e->src_id) == visited_v.end() && visited_v.find(e->dest_id) != visited_v.end())
				{
					e->dest_id = id_cv;
				}
			}

			for (auto &id : visited_v)
			{
				removed_vertices.insert(id);
			}
		}
		else if (m.signature == "sum(-(V,V))")
		{
			cidfg::ComputationVertex cv;
			cv.coord = g_.get_vertex(map_custom2cidfg[vd])->coord;
			cv.func_name = "silago_dpu_sub_acc";
			cv.is_on_dpu = true;
			int id_cv = g_.add_vertex(cv, parent_id, child_index);

			g_.get_edge(m.arguments[0])->dest_id = id_cv;
			g_.get_edge(m.arguments[0])->dest_port = 0;
			g_.get_edge(m.arguments[1])->dest_id = id_cv;
			g_.get_edge(m.arguments[1])->dest_port = 1;

			for (auto &e : g_.get_edges())
			{
				if (visited_v.find(e->src_id) != visited_v.end() && visited_v.find(e->dest_id) == visited_v.end())
				{
					e->src_id = id_cv;
				}
				else if (visited_v.find(e->src_id) == visited_v.end() && visited_v.find(e->dest_id) != visited_v.end())
				{
					e->dest_id = id_cv;
				}
			}

			for (auto &id : visited_v)
			{
				removed_vertices.insert(id);
			}
		}
	}

	// remove vertices
	for (auto &id : removed_vertices)
	{
		g_.del_vertex(id);
	}
}

} // namespace codegen
} // namespace vesyla