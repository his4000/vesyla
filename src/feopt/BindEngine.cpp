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

#include "BindEngine.hpp"

using namespace vesyla::cidfg;
using namespace boost;

namespace vesyla
{
namespace feopt
{

void BindEngine::find_and_remove_dependency_edges(cidfg::CidfgGraph &g_)
{
	set<int> all_vertices;
	for (auto &v : g_.get_vertices())
	{
		all_vertices.insert(v->id);
	}
	for (auto &e : g_.get_edges())
	{
		if (all_vertices.find(e->src_id) == all_vertices.end())
		{
			g_.del_edge(e->id);
		}
		else if (all_vertices.find(e->dest_id) == all_vertices.end())
		{
			g_.del_edge(e->id);
		}
	}
}

void BindEngine::transform(CidfgGraph &g_)
{
	find_and_remove_dependency_edges(g_);
	fill_coord(g_);

	split_vertex(g_);
	clean(g_);
}

void fill_coord_for_indexing_vertex(CidfgGraph &g_, int id_, std::unordered_map<string, VIR::Coordinate> &var_coord_map_)
{
	if (g_.get_vertex(id_)->vertex_type == Vertex::REG_VAR_VERTEX)
	{
		var_coord_map_[static_cast<RegVarVertex *>(g_.get_vertex(id_))->var_name] = g_.get_vertex(id_)->coord;
	}
	else if (g_.get_vertex(id_)->vertex_type == Vertex::SRAM_VAR_VERTEX)
	{
		var_coord_map_[static_cast<SramVarVertex *>(g_.get_vertex(id_))->var_name] = static_cast<SramVarVertex *>(g_.get_vertex(id_))->sram_coord;
	}
	else if (g_.get_vertex(id_)->vertex_type == Vertex::READ_AND_INDEX_VERTEX && !g_.get_vertex(id_)->coord.is_valid())
	{
		ReadAndIndexVertex *v = static_cast<ReadAndIndexVertex *>(g_.get_vertex(id_));
		Edge *e = g_.get_edge(g_.get_in_edge(id_, 0));
		if (v->is_sram)
		{
			v->sram_coord = var_coord_map_[e->var_name];
			Vertex *dest_v = g_.get_vertex(g_.get_edge(g_.get_out_edge(id_, 0))->dest_id);
			v->coord = dest_v->coord;
		}
		else
		{
			if (e->edge_type != Edge::VECTOR_DATA_SIG && e->edge_type != Edge::SRAM_VECTOR_DATA_SIG)
			{
				LOG(FATAL) << "Wrong edge type, need a vector data signal!";
			}
			if (var_coord_map_.find(e->var_name) != var_coord_map_.end())
			{
				v->coord = var_coord_map_[e->var_name];
			}
		}
	}
	else if (g_.get_vertex(id_)->vertex_type == Vertex::WRITE_AND_INDEX_VERTEX && !g_.get_vertex(id_)->coord.is_valid())
	{
		WriteAndIndexVertex *v = static_cast<WriteAndIndexVertex *>(g_.get_vertex(id_));
		Edge *e = g_.get_edge(g_.get_in_edge(id_, 1));
		if (v->is_sram)
		{
			v->sram_coord = var_coord_map_[e->var_name];
			Vertex *src_v = g_.get_vertex(g_.get_edge(g_.get_in_edge(id_, 0))->src_id);
			v->coord = src_v->coord;
		}
		else
		{
			if (e->edge_type != Edge::VECTOR_DATA_SIG && e->edge_type != Edge::SRAM_VECTOR_DATA_SIG)
			{
				LOG(FATAL) << "Wrong edge type, need a vector data signal!";
			}
			if (var_coord_map_.find(e->var_name) != var_coord_map_.end())
			{
				v->coord = var_coord_map_[e->var_name];
			}
		}
	}
	else if (g_.get_vertex(id_)->is_hierarchical())
	{
		HierarchicalVertex *v = static_cast<HierarchicalVertex *>(g_.get_vertex(id_));
		std::unordered_map<string, VIR::Coordinate> var_coord_map_1;
		var_coord_map_1 = var_coord_map_;
		for (int i = 0; i < v->children.size(); i++)
		{
			for (int j = 0; j < v->children[i].size(); j++)
			{
				fill_coord_for_indexing_vertex(g_, v->children[i][j], var_coord_map_1);
			}
		}
	}
}

bool fill_coord_for_computation_vertex(CidfgGraph &g_)
{
	bool flag_changed = false;
	for (auto &e : g_.get_edges())
	{

		if (g_.get_vertex(e->src_id)->vertex_type == Vertex::COMPUTATION_VERTEX && g_.get_vertex(e->dest_id)->vertex_type == Vertex::COMPUTATION_VERTEX)
		{
			ComputationVertex *v_src = static_cast<ComputationVertex *>(g_.get_vertex(e->src_id));
			ComputationVertex *v_dest = static_cast<ComputationVertex *>(g_.get_vertex(e->dest_id));
			if (v_src->coord.is_valid() && !v_dest->coord.is_valid())
			{
				v_dest->coord = v_src->coord;
				flag_changed = true;
			}
			else if (!v_src->coord.is_valid() && v_dest->coord.is_valid())
			{
				v_src->coord = v_dest->coord;
				flag_changed = true;
			}
		}
	}
	return flag_changed;
}

void BindEngine::fill_coord(CidfgGraph &g_)
{

	// create a map for all register/sram source nodes in each hierarchical level and
	// fill the RDI and WDI coordinates.
	std::unordered_map<string, VIR::Coordinate> var_coord_map;
	for (auto &v : g_.get_vertices())
	{
		if (v->vertex_type == Vertex::ROOT_VERTEX)
		{
			fill_coord_for_indexing_vertex(g_, v->id, var_coord_map);
			while (fill_coord_for_computation_vertex(g_))
				;
		}
	}

	// typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS> UGraph;
	// std::unordered_map<int, int> id_map_cidfg2ugraph;
	// std::unordered_map<int, int> id_map_ugraph2cidfg;
	// vector<Vertex *> vec_vertex = g_.get_vertices();
	// int i = 0;
	// for (auto &v : vec_vertex)
	// {
	// 	id_map_cidfg2ugraph[v->id] = i;
	// 	id_map_ugraph2cidfg[i] = v->id;
	// 	i++;
	// }

	// UGraph ug1(vec_vertex.size());
	// for (auto &e : g_.get_edges())
	// {
	// 	if (g_.get_vertex(e->src_id)->vertex_type == Vertex::READ_AND_INDEX_VERTEX && e->src_port == 0)
	// 	{
	// 		continue;
	// 	}
	// 	if (g_.get_vertex(e->dest_id)->vertex_type == Vertex::WRITE_AND_INDEX_VERTEX && e->dest_port == 0)
	// 	{
	// 		continue;
	// 	}
	// 	if (g_.get_vertex(e->dest_id)->vertex_type == Vertex::GENERAL_HIERARCHICAL_VERTEX)
	// 	{
	// 		continue;
	// 	}
	// 	if (g_.get_vertex(e->src_id)->vertex_type == Vertex::GENERAL_HIERARCHICAL_VERTEX)
	// 	{
	// 		continue;
	// 	}
	// 	if (g_.get_vertex(e->dest_id)->vertex_type == Vertex::LOOP_VERTEX && e->dest_port > 2)
	// 	{
	// 		continue;
	// 	}
	// 	if (g_.get_vertex(e->src_id)->vertex_type == Vertex::LOOP_VERTEX && e->src_port > 0)
	// 	{
	// 		continue;
	// 	}
	// 	if (g_.get_vertex(e->dest_id)->vertex_type == Vertex::BRANCH_VERTEX && e->dest_port > 0)
	// 	{
	// 		continue;
	// 	}
	// 	if (g_.get_vertex(e->src_id)->vertex_type == Vertex::BRANCH_VERTEX && e->src_port > 0)
	// 	{
	// 		continue;
	// 	}
	// 	if (g_.get_vertex(e->src_id)->vertex_type == Vertex::VAR_VERTEX)
	// 	{
	// 		continue;
	// 	}

	// 	boost::add_edge(id_map_cidfg2ugraph[e->src_id], id_map_cidfg2ugraph[e->dest_id], ug1);
	// 	boost::add_edge(id_map_cidfg2ugraph[e->dest_id], id_map_cidfg2ugraph[e->src_id], ug1);
	// }

	// std::vector<int> component(boost::num_vertices(ug1));
	// int num_components = boost::connected_components(ug1, &component[0]);

	// std::unordered_map<int, VIR::Coordinate> color_map;
	// for (int c = 0; c < num_components; c++)
	// {
	// 	VIR::Coordinate coord;
	// 	for (int i = 0; i < vec_vertex.size(); i++)
	// 	{
	// 		if (component[i] == c)
	// 		{
	// 			if (g_.get_vertex(id_map_ugraph2cidfg[i])->coord.is_valid())
	// 			{
	// 				if (!coord.is_valid())
	// 				{
	// 					coord = g_.get_vertex(id_map_ugraph2cidfg[i])->coord;
	// 				}
	// 				else
	// 				{
	// 					if (coord != g_.get_vertex(id_map_ugraph2cidfg[i])->coord)
	// 					{
	// 						LOG(FATAL) << "Multiple binding of vertex type: " << g_.get_vertex(id_map_ugraph2cidfg[i])->vertex_type;
	// 					}
	// 				}
	// 			}
	// 		}
	// 	}
	// 	for (int i = 0; i < vec_vertex.size(); i++)
	// 	{
	// 		if (component[i] == c)
	// 		{
	// 			g_.get_vertex(id_map_ugraph2cidfg[i])->coord = coord;
	// 		}
	// 	}
	// }
}

void find_all_coord(CidfgGraph &g_, int id_, set<string> &coord_str_)
{
	if (id_ < 0)
	{
		for (auto &v : g_.get_vertices())
		{
			find_all_coord(g_, v->id, coord_str_);
		}
	}
	else
	{
		Vertex *v = g_.get_vertex(id_);
		if (v->is_hierarchical())
		{
			HierarchicalVertex *vv = static_cast<HierarchicalVertex *>(v);
			for (int i = 0; i < vv->children.size(); i++)
			{
				for (int j = 0; j < vv->children[i].size(); j++)
				{
					find_all_coord(g_, vv->children[i][j], coord_str_);
				}
			}
		}
		else
		{
			if (v->coord.is_valid())
			{
				coord_str_.insert(v->coord.to_str());
			}
		}
	}
}

// void split_vertex_call_back(CidfgGraph &g_, int id_, set<int> &assigned_vertices_, set<string> &all_coord_str_, set<string> curr_coord_str_, int parent_id_, int child_index_)
// {
// 	if (g_.get_vertex(id_)->coord.is_valid())
// 	{
// 		curr_coord_str_.insert(g_.get_vertex(id_)->coord.to_str());
// 		assigned_vertices_.insert(id_);
// 		return;
// 	}

// 	switch (g_.get_vertex(id_)->vertex_type)
// 	{
// 	case Vertex::VAR_VERTEX:
// 	{
// 		for (auto &e : g_.get_edges())
// 		{
// 			if (e->src_id == g_.get_vertex(id_)->id)
// 			{
// 				if (g_.get_vertex(e->src_id)->coord.is_valid())
// 				{
// 					VarVertex *vv = static_cast<VarVertex *>(g_.get_vertex(e->src_id));
// 					int id = g_.add_vertex(*vv, parent_id_, child_index_);
// 					g_.get_vertex(id)->coord = g_.get_vertex(e->dest_id)->coord;
// 					e->src_id = id;
// 				}
// 				else
// 				{
// 					LOG(FATAL) << "VarVertex's coordinate can't be determined!";
// 				}
// 			}
// 		}
// 		break;
// 	}
// 	case Vertex::ROOT_VERTEX:
// 	{
// 		HierarchicalVertex *vv = static_cast<HierarchicalVertex *>(g_.get_vertex(id_));
// 		set<string> root_coord_str;
// 		for (int i = 0; i < vv->children.size(); i++)
// 		{
// 			for (int j = 0; j < vv->children[i].size(); j++)
// 			{
// 				if (g_.get_vertex(vv->children[i][j])->vertex_type == Vertex::BRANCH_VERTEX || g_.get_vertex(vv->children[i][j])->vertex_type == Vertex::LOOP_VERTEX)
// 				{
// 					split_vertex_call_back(g_, vv->children[i][j], assigned_vertices_, all_coord_str_, root_coord_str, id_, i);
// 				}
// 			}
// 			for (int j = 0; j < vv->children[i].size(); j++)
// 			{
// 				if (g_.get_vertex(vv->children[i][j])->vertex_type != Vertex::BRANCH_VERTEX && g_.get_vertex(vv->children[i][j])->vertex_type != Vertex::LOOP_VERTEX)
// 				{
// 					split_vertex_call_back(g_, vv->children[i][j], assigned_vertices_, all_coord_str_, root_coord_str, id_, i);
// 				}
// 			}
// 		}
// 		for (auto coord_str : all_coord_str_)
// 		{
// 			curr_coord_str_.insert(coord_str);
// 			VIR::Coordinate coord;
// 			coord.from_str(coord_str);
// 			RootVertex rv;
// 			rv.coord = coord;
// 			int id_rv = g_.add_vertex(rv, parent_id_, child_index_);
// 			for (int i = 0; i < vv->children.size(); i++)
// 			{
// 				vector<int> vec;
// 				rv.children.push_back(vec);
// 				for (int j = 0; j < vv->children[i].size(); j++)
// 				{
// 					if (g_.get_vertex(vv->children[i][j])->coord == coord)
// 					{
// 						rv.children[i].push_back(vv->children[i][j]);
// 					}
// 				}
// 			}
// 		}
// 		break;
// 	}
// 	case Vertex::BRANCH_VERTEX:
// 	{
// 		break;
// 	}
// 	case Vertex::LOOP_VERTEX:
// 	{
// 		HierarchicalVertex *vv = static_cast<HierarchicalVertex *>(g_.get_vertex(id_));
// 		set<string> root_coord_str;
// 		for (int i = 0; i < vv->children.size(); i++)
// 		{
// 			for (int j = 0; j < vv->children[i].size(); j++)
// 			{
// 				if (g_.get_vertex(vv->children[i][j])->vertex_type == Vertex::BRANCH_VERTEX || g_.get_vertex(vv->children[i][j])->vertex_type == Vertex::LOOP_VERTEX)
// 				{
// 					split_vertex_call_back(g_, vv->children[i][j], assigned_vertices_, all_coord_str_, root_coord_str, id_, i);
// 				}
// 			}
// 			for (int j = 0; j < vv->children[i].size(); j++)
// 			{
// 				if (g_.get_vertex(vv->children[i][j])->vertex_type != Vertex::BRANCH_VERTEX && g_.get_vertex(vv->children[i][j])->vertex_type != Vertex::LOOP_VERTEX)
// 				{
// 					split_vertex_call_back(g_, vv->children[i][j], assigned_vertices_, all_coord_str_, root_coord_str, id_, i);
// 				}
// 			}
// 		}
// 		for (auto coord_str : all_coord_str_)
// 		{
// 			curr_coord_str_.insert(coord_str);
// 			VIR::Coordinate coord;
// 			coord.from_str(coord_str);
// 			LoopVertex rv;
// 			rv.coord = coord;
// 			int id_rv = g_.add_vertex(rv, parent_id_, child_index_);
// 			for (int i = 0; i < vv->children.size(); i++)
// 			{
// 				vector<int> vec;
// 				rv.children.push_back(vec);
// 				for (int j = 0; j < vv->children[i].size(); j++)
// 				{
// 					if (g_.get_vertex(vv->children[i][j])->coord == coord)
// 					{
// 						rv.children[i].push_back(vv->children[i][j]);
// 					}
// 				}
// 			}
// 		}
// 		break;
// 	}
// 	default:
// 	{
// 		break;
// 	}
// 	}
// }

void BindEngine::split_vertex(CidfgGraph &g_)
{
	set<string> all_coord_str;
	find_all_coord(g_, -1, all_coord_str);
	vector<VIR::Coordinate> all_coord;
	for (auto &s : all_coord_str)
	{
		VIR::Coordinate coord;
		coord.from_str(s);
		all_coord.push_back(coord);
	}
	std::unordered_map<int, std::unordered_map<string, int>> shadow_vertex_map;

	for (auto &v : g_.get_vertices())
	{

		if (v->coord.is_valid())
		{
			continue;
		}
		shadow_vertex_map[v->id] = {};
		// int parent_id = -1;
		// int child_index = -1;
		// g_.get_parent(v->id, parent_id, child_index);

		switch (v->vertex_type)
		{
		case Vertex::CONST_VERTEX:
		{
			for (auto &c : all_coord)
			{
				ConstVertex vv = *(static_cast<ConstVertex *>(v));
				vv.coord = c;
				int new_id = g_.add_vertex(vv);
				shadow_vertex_map[v->id][c.to_str()] = new_id;
			}
			break;
		}
		case Vertex::VAR_VERTEX:
		{
			for (auto &c : all_coord)
			{
				VarVertex vv = *(static_cast<VarVertex *>(v));
				vv.coord = c;
				int new_id = g_.add_vertex(vv);
				shadow_vertex_map[v->id][c.to_str()] = new_id;
			}
			break;
		}
		case Vertex::REG_VAR_VERTEX:
		{
			break;
		}
		case Vertex::SRAM_VAR_VERTEX:
		{
			for (auto &c : all_coord)
			{
				SramVarVertex vv = *(static_cast<SramVarVertex *>(v));
				vv.coord = c;
				int new_id = g_.add_vertex(vv);
				shadow_vertex_map[v->id][c.to_str()] = new_id;
			}
			break;
		}
		case Vertex::INDEXING_VERTEX:
		{
			break;
		}
		case Vertex::READ_AND_INDEX_VERTEX:
		{
			for (auto &c : all_coord)
			{
				ReadAndIndexVertex vv = *(static_cast<ReadAndIndexVertex *>(v));
				vv.coord = c;
				int new_id = g_.add_vertex(vv);
				shadow_vertex_map[v->id][c.to_str()] = new_id;
			}
			break;
		}
		case Vertex::WRITE_AND_INDEX_VERTEX:
		{
			for (auto &c : all_coord)
			{
				WriteAndIndexVertex vv = *(static_cast<WriteAndIndexVertex *>(v));
				vv.coord = c;
				int new_id = g_.add_vertex(vv);
				shadow_vertex_map[v->id][c.to_str()] = new_id;
			}
			break;
		}
		case Vertex::COMPUTATION_VERTEX:
		{
			for (auto &c : all_coord)
			{
				ComputationVertex vv = *(static_cast<ComputationVertex *>(v));
				vv.coord = c;
				int new_id = g_.add_vertex(vv);
				shadow_vertex_map[v->id][c.to_str()] = new_id;
			}
			break;
		}
		case Vertex::READING_VERTEX:
		{
			break;
		}
		case Vertex::WRITING_VERTEX:
		{
			break;
		}
		case Vertex::REG_SINK_VERTEX:
		{
			for (auto &c : all_coord)
			{
				RegSinkVertex vv = *(static_cast<RegSinkVertex *>(v));
				vv.coord = c;
				int new_id = g_.add_vertex(vv);
				shadow_vertex_map[v->id][c.to_str()] = new_id;
			}
			break;
		}
		case Vertex::SRAM_SINK_VERTEX:
		{
			for (auto &c : all_coord)
			{
				SramSinkVertex vv = *(static_cast<SramSinkVertex *>(v));
				vv.coord = c;
				int new_id = g_.add_vertex(vv);
				shadow_vertex_map[v->id][c.to_str()] = new_id;
			}
			break;
		}
		case Vertex::VAR_SINK_VERTEX:
		{
			for (auto &c : all_coord)
			{
				VarSinkVertex vv = *(static_cast<VarSinkVertex *>(v));
				vv.coord = c;
				int new_id = g_.add_vertex(vv);
				shadow_vertex_map[v->id][c.to_str()] = new_id;
			}
			break;
		}
		case Vertex::GENERAL_HIERARCHICAL_VERTEX:
		{
			for (auto &c : all_coord)
			{
				GeneralHierarchicalVertex vv = *(static_cast<GeneralHierarchicalVertex *>(v));
				vv.children.clear();
				vv.coord = c;
				int new_id = g_.add_vertex(vv);
				shadow_vertex_map[v->id][c.to_str()] = new_id;
			}
			break;
		}
		case Vertex::BRANCH_VERTEX:
		{
			for (auto &c : all_coord)
			{
				BranchVertex vv = *(static_cast<BranchVertex *>(v));
				vv.children.clear();
				vv.coord = c;
				int new_id = g_.add_vertex(vv);
				shadow_vertex_map[v->id][c.to_str()] = new_id;
			}
			break;
		}
		case Vertex::LOOP_VERTEX:
		{
			for (auto &c : all_coord)
			{
				LoopVertex vv = *(static_cast<LoopVertex *>(v));
				vv.children.clear();
				vv.coord = c;
				int new_id = g_.add_vertex(vv);
				shadow_vertex_map[v->id][c.to_str()] = new_id;
			}
			break;
		}
		case Vertex::ROOT_VERTEX:
		{
			for (auto &c : all_coord)
			{
				RootVertex vv = *(static_cast<RootVertex *>(v));
				vv.children.clear();
				vv.coord = c;
				int new_id = g_.add_vertex(vv);
				shadow_vertex_map[v->id][c.to_str()] = new_id;
			}
			break;
		}
		case Vertex::MERGE_VERTEX:
		{
			for (auto &c : all_coord)
			{
				MergeVertex vv = *(static_cast<MergeVertex *>(v));
				vv.coord = c;
				int new_id = g_.add_vertex(vv);
				shadow_vertex_map[v->id][c.to_str()] = new_id;
			}
			break;
		}
		default:
		{
			LOG(WARNING) << "Unknow vertex splitted!";
			break;
		}
		}
	}

	// build hirarichy for shadow vertex
	// for (auto &org_pair : shadow_vertex_map)
	// {
	// 	int parent_id = -1;
	// 	int child_index = -1;
	// 	g_.get_parent(org_pair.first, parent_id, child_index);
	// 	if (parent_id < 0 || child_index < 0)
	// 	{
	// 		continue;
	// 	}
	// 	for (auto &shadow_pair : org_pair.second)
	// 	{
	// 		if (shadow_vertex_map.find(parent_id) != shadow_vertex_map.end())
	// 		{
	// 			HierarchicalVertex *vv = static_cast<HierarchicalVertex *>(g_.get_vertex(shadow_vertex_map[parent_id][shadow_pair.first]));
	// 			if (vv->children.size() < (child_index + 1))
	// 			{
	// 				vv->children.resize(child_index + 1);
	// 			}
	// 			vv->children[child_index].push_back(shadow_pair.second);
	// 		}
	// 		else
	// 		{
	// 			HierarchicalVertex *vv = static_cast<HierarchicalVertex *>(g_.get_vertex(parent_id));
	// 			if (vv->children.size() < (child_index + 1))
	// 			{
	// 				vv->children.resize(child_index + 1);
	// 			}
	// 			vv->children[child_index].push_back(shadow_pair.second);
	// 		}
	// 	}
	// }

	for (auto &org_pair : shadow_vertex_map)
	{
		if (g_.get_vertex(org_pair.first)->is_hierarchical())
		{
			HierarchicalVertex *vv = static_cast<HierarchicalVertex *>(g_.get_vertex(org_pair.first));
			vector<vector<int>> new_children;
			for (int i = 0; i < vv->children.size(); i++)
			{
				for (auto s : all_coord_str)
				{
					static_cast<HierarchicalVertex *>(g_.get_vertex(org_pair.second[s]))->children.push_back({});
				}
				new_children.push_back({});
				for (int j = 0; j < vv->children[i].size(); j++)
				{
					if (shadow_vertex_map.find(vv->children[i][j]) != shadow_vertex_map.end())
					{
						for (auto s : all_coord_str)
						{
							static_cast<HierarchicalVertex *>(g_.get_vertex(org_pair.second[s]))->children[i].push_back(shadow_vertex_map[vv->children[i][j]][s]);
						}
						new_children[i].push_back(vv->children[i][j]);
					}
					else
					{
						HierarchicalVertex *vvv = static_cast<HierarchicalVertex *>(g_.get_vertex(org_pair.second[g_.get_vertex(vv->children[i][j])->coord.to_str()]));
						vvv->children[i].push_back(vv->children[i][j]);
					}
				}
			}
			vv->children = new_children;
		}
	}

	// Adjust the edges for shadow vertex
	vector<Edge *> vec_edge = g_.get_edges();
	for (auto e : vec_edge)
	{
		if (shadow_vertex_map.find(e->src_id) != shadow_vertex_map.end() && shadow_vertex_map.find(e->dest_id) != shadow_vertex_map.end())
		{
			for (auto &s : all_coord_str)
			{
				Edge new_e(shadow_vertex_map[e->src_id][s], e->src_port, shadow_vertex_map[e->dest_id][s], e->dest_port, e->var_name, e->edge_type);
				g_.add_edge(new_e);
			}
		}
		else if (shadow_vertex_map.find(e->src_id) != shadow_vertex_map.end() && shadow_vertex_map.find(e->dest_id) == shadow_vertex_map.end())
		{
			Edge new_e(shadow_vertex_map[e->src_id][g_.get_vertex(e->dest_id)->coord.to_str()], e->src_port, e->dest_id, e->dest_port, e->var_name, e->edge_type);
			g_.add_edge(new_e);
		}
		else if (shadow_vertex_map.find(e->src_id) == shadow_vertex_map.end() && shadow_vertex_map.find(e->dest_id) != shadow_vertex_map.end())
		{
			Edge new_e(e->src_id, e->src_port, shadow_vertex_map[e->dest_id][g_.get_vertex(e->src_id)->coord.to_str()], e->dest_port, e->var_name, e->edge_type);
			g_.add_edge(new_e);
		}
	}

	// Put all the shadow vertex sub graph to the first shadow vertex and create a dependency connection to between those shadow vertices
	for (auto &org_pair : shadow_vertex_map)
	{
		if (g_.get_vertex(org_pair.first)->is_hierarchical())
		{

			if (org_pair.second.size() < 1)
			{
				continue;
			}

			HierarchicalVertex *vv = static_cast<HierarchicalVertex *>(g_.get_vertex(org_pair.first));
			HierarchicalVertex *vvv;
			int index = 0;
			for (auto &shadow_pair : org_pair.second)
			{
				if (index == 0)
				{
					vvv = static_cast<HierarchicalVertex *>(g_.get_vertex(shadow_pair.second));
					index++;
					continue;
				}
				HierarchicalVertex *vvvv = static_cast<HierarchicalVertex *>(g_.get_vertex(shadow_pair.second));
				for (int i = 0; i < vvvv->children.size(); i++)
				{
					for (int j = 0; j < vvvv->children[i].size(); j++)
					{
						vvv->children[i].push_back(vvvv->children[i][j]);
					}
				}
				vvvv->children.clear();
				Edge e(vvv->id, 100, vvvv->id, 100, "", Edge::DEPENDENCY, 0, 0);
				g_.add_edge(e);
			}
		}
	}

	// TODO:
	// Adjust for condition dpu node of branch
	for (auto &v : g_.get_vertices())
	{
		if (v->vertex_type == Vertex::BRANCH_VERTEX)
		{
			ComputationVertex *cv = static_cast<ComputationVertex *>(g_.get_vertex(g_.get_edge(g_.get_in_edge(v->id, 0))->src_id));
		}
	}

	// Remove all original vertex if it has shadow vertices
	for (auto org_pair : shadow_vertex_map)
	{
		g_.del_vertex(org_pair.first);
	}
}

void BindEngine::clean(cidfg::CidfgGraph &g_)
{
	std::set<int> removed_vertices;
	graph_traits<AGraph>::vertex_iterator vi, vi_end;

	AGraph g1;
	map<int, boost::graph_traits<AGraph>::vertex_descriptor> gdict1;
	for (Vertex *v : g_.get_vertices())
	{
		auto vi = add_vertex(v, g1);
		gdict1[v->id] = vi;
	}
	for (Edge *e : g_.get_edges())
	{
		boost::add_edge(gdict1[e->src_id], gdict1[e->dest_id], e, g1);
	}

	std::set<int> needed_vertices;
	for (tie(vi, vi_end) = vertices(g1); vi != vi_end; vi++)
	{
		if (g1[*vi]->is_sink())
		{
			record_needed_vertices(*vi, g1, needed_vertices);
		}
		else if (g1[*vi]->is_hierarchical())
		{
			record_needed_vertices(*vi, g1, needed_vertices);
		}
	}
	for (tie(vi, vi_end) = vertices(g1); vi != vi_end; vi++)
	{
		if (needed_vertices.find(g1[*vi]->id) == needed_vertices.end())
		{
			removed_vertices.insert(g1[*vi]->id);
		}
	}

	typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS> UGraph;
	std::unordered_map<int, int> id_map_cidfg2ugraph;
	std::unordered_map<int, int> id_map_ugraph2cidfg;
	vector<Vertex *> vec_vertex = g_.get_vertices();
	int i = 0;
	for (auto &v : vec_vertex)
	{
		id_map_cidfg2ugraph[v->id] = i;
		id_map_ugraph2cidfg[i] = v->id;
		i++;
	}

	UGraph ug1(vec_vertex.size());
	for (auto &e : g_.get_edges())
	{
		if (e->edge_type == cidfg::Edge::DEPENDENCY)
		{
			continue;
		}
		boost::add_edge(id_map_cidfg2ugraph[e->src_id], id_map_cidfg2ugraph[e->dest_id], ug1);
		boost::add_edge(id_map_cidfg2ugraph[e->dest_id], id_map_cidfg2ugraph[e->src_id], ug1);
	}

	std::vector<int> component(boost::num_vertices(ug1));
	int num_components = boost::connected_components(ug1, &component[0]);

	for (int c = 0; c < num_components; c++)
	{

		int x = 0;
		for (x = 0; x < vec_vertex.size(); x++)
		{
			if (component[x] == c)
			{
				if (g_.get_vertex(id_map_ugraph2cidfg[x])->is_sink() || g_.get_vertex(id_map_ugraph2cidfg[x])->is_hierarchical())
				{
					break;
				}
			}
		}
		if (x == vec_vertex.size())
		{
			for (int j = 0; j < vec_vertex.size(); j++)
			{
				if (component[j] == c)
				{
					removed_vertices.insert(g_.get_vertex(id_map_ugraph2cidfg[j])->id);
				}
			}
		}
	}

	for (auto &id : removed_vertices)
	{
		g_.del_vertex(id);
	}
}

void BindEngine::record_needed_vertices(boost::graph_traits<AGraph>::vertex_descriptor v_, AGraph &g0_, set<int> &needed_vertices_)
{
	needed_vertices_.insert(g0_[v_]->id);
	graph_traits<AGraph>::in_edge_iterator ei, ei_end;
	for (tie(ei, ei_end) = in_edges(v_, g0_); ei != ei_end; ei++)
	{
		record_needed_vertices(source(*ei, g0_), g0_, needed_vertices_);
	}
}

} // namespace feopt
} // namespace vesyla