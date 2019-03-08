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

#include "ExprSimpEngine.hpp"

using namespace vesyla::cidfg;
using namespace boost;

namespace vesyla
{
namespace feopt
{
void ExprSimpEngine::find_and_remove_dependency_edges(cidfg::CidfgGraph &g_)
{
	for (auto &e : g_.get_edges())
	{
		if (e->edge_type == cidfg::Edge::DEPENDENCY)
		{
			_dependency_edges.push_back(*e);
			g_.del_edge(e->id);
		}
	}
}
void ExprSimpEngine::add_dependency_edges(cidfg::CidfgGraph &g_)
{
	for (auto &e : _dependency_edges)
	{
		g_.add_edge(e);
	}
}

void ExprSimpEngine::transform(cidfg::CidfgGraph &g_)
{
	find_and_remove_dependency_edges(g_);

	VarTable t;

	std::set<string> temp_set;
	for (auto e : g_.get_edges())
	{
		if (e->edge_type != cidfg::Edge::SCALAR_DATA_SIG)
		{
			continue;
		}

		if (g_.get_vertex(e->src_id)->vertex_type != cidfg::Vertex::COMPUTATION_VERTEX || g_.get_vertex(e->dest_id)->vertex_type != cidfg::Vertex::COMPUTATION_VERTEX)
		{
			continue;
		}

		int src_parent = -1;
		int src_child_index = -1;
		int dest_parent = -1;
		int dest_child_index = -1;
		g_.get_parent(e->src_id, src_parent, src_child_index);
		g_.get_parent(e->dest_id, dest_parent, dest_child_index);
		if (src_parent == dest_parent)
		{
			continue;
		}
		bool flag_eval_to_constant = false;
		Graph g0 = create_graph(g_);
		graph_traits<Graph>::vertex_iterator vi, vi_end;
		graph_traits<Graph>::in_edge_iterator iei, iei_end;
		for (tie(vi, vi_end) = vertices(g0); vi != vi_end; vi++)
		{
			if (g0[*vi]->id == e->src_id)
			{
				EdgeProperty vr = cal_var_record(g0, *vi);
				if (!vr.flag_unsimplified && vr.ipt.is_constant())
				{
					ConstVertex vv;
					vv.set_int_value(vr.ipt.get_bias());
					int vvid = g_.add_vertex(vv, dest_parent, dest_child_index);
					e->src_id = vvid;
					e->src_port = 0;
					flag_eval_to_constant = true;
					break;
				}
				break;
			}
		}
		if (flag_eval_to_constant)
		{
			continue;
		}

		string id_port_signature = to_string(e->src_id) + "_" + to_string(e->src_port);
		if (temp_set.find(id_port_signature) != temp_set.end())
		{
			VarVertex vv;
			for (auto &n : t.names())
			{
				if (t.get_vertex_id(n) == e->src_id && t.get_vertex_port(n) == e->src_port)
				{
					vv.var_name = n;
					break;
				}
			}
			int id_vv = g_.add_vertex(vv, src_parent, src_child_index);
			e->src_id = id_vv;
		}
		else
		{
			string temp_var_name = "need_simplify_" + get_random_temp_var_name();
			VarVertex vv;
			vv.var_name = temp_var_name;
			int id_vv = g_.add_vertex(vv, src_parent, src_child_index);
			temp_set.insert(id_port_signature);
			t.update_var(temp_var_name, e->src_id, e->src_port, "");
			e->src_id = id_vv;
		}
	}

	// simplify(g_, t);
	// simplify(g_, t);
	// simplify(g_, t);
	// simplify(g_, t);
	while (simplify(g_, t))
	{
		;
	}
	rebuild(g_, t);
	add_dependency_edges(g_);
}

string ExprSimpEngine::get_random_temp_var_name()
{
	string str = "expr_simp_engine_temp_var_" + to_string(_var_counter++);
	return str;
}

void ExprSimpEngine::rebuild(cidfg::CidfgGraph &g_, cidfg::VarTable &t_)
{
	for (auto &e : g_.get_edges())
	{
		Vertex *v = g_.get_vertex(e->src_id);
		if (v->vertex_type == cidfg::Vertex::VAR_VERTEX)
		{
			VarVertex *vv = static_cast<VarVertex *>(v);
			if (t_.exist(vv->var_name))
			{
				int id = t_.get_vertex_id(vv->var_name);
				int port = t_.get_vertex_port(vv->var_name);
				e->src_id = id;
				e->src_port = port;
			}
		}
	}
	std::set<int> removed_vertices;
	for (auto &v : g_.get_vertices())
	{
		if (v->vertex_type == cidfg::Vertex::VAR_VERTEX)
		{
			VarVertex *vv = static_cast<VarVertex *>(v);
			if (t_.exist(vv->var_name))
			{
				removed_vertices.insert(vv->id);
			}
		}
	}

	// clean branches with computation vertex as terminal node and no data out edge
	// is connectedc to this computation vertex.
	for (auto &v : g_.get_vertices())
	{
		if (v->vertex_type == cidfg::Vertex::COMPUTATION_VERTEX)
		{
			bool flag_only_dependency_edge = true;
			for (auto &e : g_.get_edges())
			{
				if (e->src_id == v->id && e->edge_type != cidfg::Edge::DEPENDENCY)
				{
					flag_only_dependency_edge = false;
					break;
				}
			}

			if (flag_only_dependency_edge)
			{

				for (auto &e : g_.get_edges())
				{
					if (e->src_id == v->id && e->edge_type == cidfg::Edge::DEPENDENCY)
					{
						g_.del_edge(e->id);
					}
				}
			}
		}
	}
	graph_traits<Graph>::vertex_iterator vi, vi_end;
	Graph g1 = create_graph(g_);
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

	for (auto &id : removed_vertices)
	{
		g_.del_vertex(id);
	}
}

void find_all_branch_vertices(Graph &g0_, boost::graph_traits<Graph>::vertex_descriptor v0_, set<int> &all_branch_vertices_)
{
	all_branch_vertices_.insert(g0_[v0_]->id);
	graph_traits<Graph>::in_edge_iterator iei, iei_end;
	for (tie(iei, iei_end) = in_edges(v0_, g0_); iei != iei_end; iei++)
	{
		find_all_branch_vertices(g0_, source(*iei, g0_), all_branch_vertices_);
	}
}
void find_top_computation_vertices(Graph &g0_, boost::graph_traits<Graph>::vertex_descriptor v0_, set<int> &top_computation_vertices_)
{
	bool flag_has_in_edges = false;
	graph_traits<Graph>::in_edge_iterator iei, iei_end;
	for (tie(iei, iei_end) = in_edges(v0_, g0_); iei != iei_end; iei++)
	{
		if (!g0_[source(*iei, g0_)]->is_source())
		{
			flag_has_in_edges = true;
			find_top_computation_vertices(g0_, source(*iei, g0_), top_computation_vertices_);
		}
	}
	if ((!flag_has_in_edges) && (g0_[v0_]->vertex_type == cidfg::Vertex::COMPUTATION_VERTEX))
	{
		top_computation_vertices_.insert(g0_[v0_]->id);
	}
}
void ExprSimpEngine::transfer_dependency_edges(cidfg::CidfgGraph &g_, int old_branch_id_, int new_branch_id_)
{
	if (g_.get_vertex(new_branch_id_)->is_source())
	{
		return;
	}
	set<int> all_branch_vertices;
	set<int> top_computation_vertices;

	Graph g0 = create_graph(g_);
	graph_traits<Graph>::vertex_iterator vi, vi_end;
	for (tie(vi, vi_end) = vertices(g0); vi != vi_end; vi++)
	{
		if (g0[*vi]->id == old_branch_id_)
		{
			find_all_branch_vertices(g0, *vi, all_branch_vertices);
		}
	}
	for (tie(vi, vi_end) = vertices(g0); vi != vi_end; vi++)
	{
		if (g0[*vi]->id == new_branch_id_)
		{
			find_top_computation_vertices(g0, *vi, top_computation_vertices);
		}
	}

	vector<cidfg::Edge> edges = _dependency_edges;
	_dependency_edges.clear();
	for (auto &e : edges)
	{
		// Both incoming and outgoing dependency
		if (all_branch_vertices.find(e.src_id) != all_branch_vertices.end() && all_branch_vertices.find(e.dest_id) != all_branch_vertices.end())
		{
			if (e.edge_type == cidfg::Edge::DEPENDENCY)
			{
				for (auto &id : top_computation_vertices)
				{
					Edge e0(new_branch_id_, e.src_port, id, e.dest_port, e.var_name, e.edge_type, e.d_lo, e.d_hi);
					_dependency_edges.push_back(e0);
				}
			}
		}
		// outgoing dependency
		else if (all_branch_vertices.find(e.src_id) != all_branch_vertices.end())
		{
			if (e.edge_type == cidfg::Edge::DEPENDENCY)
			{
				e.src_id = new_branch_id_;
				_dependency_edges.push_back(e);
			}
		}
		// incoming dependency
		else if (all_branch_vertices.find(e.dest_id) != all_branch_vertices.end())
		{
			if (e.edge_type == cidfg::Edge::DEPENDENCY)
			{
				for (auto &id : top_computation_vertices)
				{
					Edge e0(e.src_id, e.src_port, id, e.dest_port, e.var_name, e.edge_type, e.d_lo, e.d_hi);
					_dependency_edges.push_back(e0);
				}
			}
		}
		else
		{
			// other edges
			_dependency_edges.push_back(e);
		}
	}
}

bool ExprSimpEngine::simplify(cidfg::CidfgGraph &g_, cidfg::VarTable &t_)
{
	bool flag_changed = false;

	std::set<int> traversed_vertices;
	std::set<int> extra_needed_vertices;
	Graph g0 = create_graph(g_);
	graph_traits<Graph>::vertex_iterator vi, vi_end;

	for (auto &n : t_.names())
	{
		if (strncmp(n.c_str(), "need_simplify_", strlen("need_simplify_")) == 0)
		{
			extra_needed_vertices.insert(t_.get_vertex_id(n));
		}
	}

	for (tie(vi, vi_end) = vertices(g0); vi != vi_end; vi++)
	{
		if (g0[*vi]->is_sink() || g0[*vi]->is_hierarchical() || extra_needed_vertices.find(g0[*vi]->id) != extra_needed_vertices.end())
		{
			EdgeProperty vr = cal_var_record(g0, *vi);
			record_needed_vertices(*vi, g0, traversed_vertices);
		}
	}

	// graph_traits<Graph>::edge_iterator ei, ei_end;
	// for (tie(ei, ei_end) = edges(g0); ei != ei_end; ei++)
	// {
	// 	if (g0[*ei].flag_unsimplified)
	// 	{
	// 		continue;
	// 	}
	// 	if (g0[source(*ei, g0)]->is_source())
	// 	{
	// 		continue;
	// 	}
	// 	VarTable vt;
	// 	find_source_vertices(g0, *ei, vt);
	// 	int parent_id;
	// 	int child_index;
	// 	g_.get_parent(g_.get_edge(g0[*ei].edge.id)->src_id, parent_id, child_index);
	// 	int branch_id = construct_cidfg_graph_branch(g_, g0[*ei], vt, parent_id, child_index);
	// 	g_.get_edge(g0[*ei].edge.id)->src_id = branch_id;
	// }

	for (tie(vi, vi_end) = vertices(g0); vi != vi_end; vi++)
	{
		if (traversed_vertices.find(g0[*vi]->id) == traversed_vertices.end())
		{
			continue;
		}
		graph_traits<Graph>::in_edge_iterator iei, iei_end;
		graph_traits<Graph>::out_edge_iterator oei, oei_end;
		bool flag_all_inputs_are_simplified = true;
		bool flag_all_outputs_are_simplified = true;
		bool flag_has_inputs = false;
		bool flag_has_outputs = false;
		for (tie(iei, iei_end) = in_edges(*vi, g0); iei != iei_end; iei++)
		{

			flag_has_inputs = true;
			if (g0[*iei].flag_unsimplified)
			{
				flag_all_inputs_are_simplified = false;
				break;
			}
			// else
			// {
			// 	int src_parent = -1;
			// 	int src_child_index = -1;
			// 	int dest_parent = -1;
			// 	int dest_child_index = -1;
			// 	g_.get_parent(g0[*iei].edge.src_id, src_parent, src_child_index);
			// 	g_.get_parent(g0[*iei].edge.dest_id, dest_parent, dest_child_index);
			// 	if (src_parent != dest_parent)
			// 	{
			// 		g0[*iei].flag_unsimplified = true;
			// 		flag_all_inputs_are_simplified = false;
			// 		break;
			// 	}
			// }
		}
		for (tie(oei, oei_end) = out_edges(*vi, g0); oei != oei_end; oei++)
		{

			flag_has_outputs = true;
			if (g0[*oei].flag_unsimplified)
			{
				flag_all_outputs_are_simplified = false;
				break;
			}
			// else
			// {
			// 	int src_parent = -1;
			// 	int src_child_index = -1;
			// 	int dest_parent = -1;
			// 	int dest_child_index = -1;
			// 	g_.get_parent(g0[*oei].edge.src_id, src_parent, src_child_index);
			// 	g_.get_parent(g0[*oei].edge.dest_id, dest_parent, dest_child_index);
			// 	if (src_parent != dest_parent)
			// 	{
			// 		g0[*oei].flag_unsimplified = true;
			// 		flag_all_outputs_are_simplified = false;
			// 		break;
			// 	}
			// }
		}

		flag_all_inputs_are_simplified = flag_all_inputs_are_simplified;
		flag_all_outputs_are_simplified = flag_all_outputs_are_simplified && flag_has_outputs;
		if (flag_all_inputs_are_simplified && flag_all_outputs_are_simplified)
		{
			continue;
		}

		for (tie(iei, iei_end) = in_edges(*vi, g0); iei != iei_end; iei++)
		{
			if (g0[*iei].flag_unsimplified)
			{
				continue;
			}
			if (g0[source(*iei, g0)]->is_source())
			{
				continue;
			}

			VarTable vt;
			find_source_vertices(g0, *iei, vt);
			int parent_id;
			int child_index;
			g_.get_parent(g_.get_edge(g0[*iei].edge.id)->src_id, parent_id, child_index);
			int branch_id = construct_cidfg_graph_branch(g_, g0[*iei], vt, parent_id, child_index);
			transfer_dependency_edges(g_, g0[*iei].edge.src_id, branch_id);
			g_.get_edge(g0[*iei].edge.id)->src_id = branch_id;
		}

		if (!flag_all_inputs_are_simplified)
		{
			continue;
		}

		std::set<string> temp_set;
		for (tie(oei, oei_end) = out_edges(*vi, g0); oei != oei_end; oei++)
		{
			if (g0[*oei].edge.edge_type == cidfg::Edge::DEPENDENCY)
			{
				continue;
			}
			string id_port_signature = to_string(g_.get_edge(g0[*oei].edge.id)->src_id) + "_" + to_string(g_.get_edge(g0[*oei].edge.id)->src_port);
			if (temp_set.find(id_port_signature) != temp_set.end())
			{
				VarVertex vv;
				for (auto &n : t_.names())
				{
					if (t_.get_vertex_id(n) == g_.get_edge(g0[*oei].edge.id)->src_id && t_.get_vertex_port(n) == g_.get_edge(g0[*oei].edge.id)->src_port)
					{
						vv.var_name = n;
						break;
					}
				}
				int parent_id;
				int child_index;
				g_.get_parent(g0[*vi]->id, parent_id, child_index);
				int id_vv = g_.add_vertex(vv, parent_id, child_index);
				g_.get_edge(g0[*oei].edge.id)->src_id = id_vv;
			}
			else
			{
				string temp_var_name = get_random_temp_var_name();
				VarVertex vv;
				vv.var_name = temp_var_name;
				int parent_id;
				int child_index;
				g_.get_parent(g0[*vi]->id, parent_id, child_index);
				int id_vv = g_.add_vertex(vv, parent_id, child_index);
				temp_set.insert(id_port_signature);
				t_.update_var(temp_var_name, g_.get_edge(g0[*oei].edge.id)->src_id, g_.get_edge(g0[*oei].edge.id)->src_port, "");
				g_.get_edge(g0[*oei].edge.id)->src_id = id_vv;
				flag_changed = true;
			}
		}
	}

	Graph g1 = create_graph(g_);
	std::set<int> needed_vertices, remove_vertices;
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
		else
		{
			for (auto &n : t_.names())
			{
				if (g1[*vi]->id == t_.get_vertex_id(n))
				{
					record_needed_vertices(*vi, g1, needed_vertices);
				}
			}
		}
	}

	for (tie(vi, vi_end) = vertices(g1); vi != vi_end; vi++)
	{
		if (needed_vertices.find(g1[*vi]->id) == needed_vertices.end())
		{
			remove_vertices.insert(g1[*vi]->id);
		}
	}
	for (auto &vid : remove_vertices)
	{
		g_.del_vertex(vid);
	}

	return flag_changed;
}

void ExprSimpEngine::record_needed_vertices(boost::graph_traits<Graph>::vertex_descriptor v_, Graph &g0_, set<int> &needed_vertices_)
{
	needed_vertices_.insert(g0_[v_]->id);
	graph_traits<Graph>::in_edge_iterator ei, ei_end;
	for (tie(ei, ei_end) = in_edges(v_, g0_); ei != ei_end; ei++)
	{
		record_needed_vertices(source(*ei, g0_), g0_, needed_vertices_);
	}
}

Graph ExprSimpEngine::create_graph(CidfgGraph &g_)
{
	Graph g1;
	map<int, boost::graph_traits<Graph>::vertex_descriptor> gdict1;
	for (Vertex *v : g_.get_vertices())
	{
		auto vi = add_vertex(v, g1);
		gdict1[v->id] = vi;
	}
	for (Edge *e : g_.get_edges())
	{
		EdgeProperty vr;
		vr.src_port = e->src_port;
		vr.dest_port = e->dest_port;
		vr.edge = *e;
		add_edge(gdict1[e->src_id], gdict1[e->dest_id], vr, g1);
	}
	return g1;
}

int ExprSimpEngine::construct_cidfg_graph_branch(CidfgGraph &g_, EdgeProperty vr_, VarTable &vt_, int parent_id_, int child_index_)
{

	std::unordered_map<int, string> out_edge_name;
	for (auto &n : vt_.names())
	{
		int id = vt_.get_vertex_id(n);
		int port = vt_.get_vertex_port(n);
		string oe_name = g_.get_edge(g_.get_out_edge(id, port))->var_name;
		out_edge_name[id] = oe_name;
	}

	int id = -1;
	IntPolynomialTable ipt = vr_.ipt;

	if (ipt.is_zero())
	{
		ConstVertex v;
		v.set_int_value(0);
		id = g_.add_vertex(v, parent_id_, child_index_);
	}
	else if (ipt.is_constant())
	{
		ConstVertex v;
		v.set_int_value(ipt.get_bias());
		id = g_.add_vertex(v, parent_id_, child_index_);
	}
	else
	{
		int id2 = -1;
		int j = 0;
		for (auto &term : ipt.get_table())
		{
			multiset<string> vars = term.first;
			int factor = term.second;
			if (term.first.empty())
			{
				ConstVertex v;
				v.set_int_value(factor);
				id2 = g_.add_vertex(v, parent_id_, child_index_);
			}
			else
			{
				int i = 0;
				for (auto &token : vars)
				{
					int const_id;
					int const_port;
					if (vt_.exist(token))
					{
						const_id = vt_.get_vertex_id(token);
						const_port = vt_.get_vertex_port(token);
					}
					else
					{
						LOG(FATAL) << "Token \"" << token << "\" not found!";
					}
					int last_id = id2;
					if (i != 0)
					{
						ComputationVertex v1;
						v1.func_name = "*";
						int compute_id = g_.add_vertex(v1, parent_id_, child_index_);
						string oe_name = "";
						for (auto &c : out_edge_name)
						{
							if (c.first == last_id)
							{
								oe_name = c.second;
							}
						}
						Edge e0(last_id, 0, compute_id, 0, oe_name);
						Edge e1(const_id, const_port, compute_id, 1);
						g_.add_edge(e0);
						g_.add_edge(e1);
						id2 = compute_id;
					}
					else
					{
						id2 = const_id;
					}
					i++;
				}
				if (factor != 1)
				{
					ComputationVertex v0;
					v0.func_name = "*";
					int compute_id = g_.add_vertex(v0, parent_id_, child_index_);
					ConstVertex v1;
					v1.set_int_value(factor);
					int const_id = g_.add_vertex(v1, parent_id_, child_index_);
					string oe_name = "";
					for (auto &c : out_edge_name)
					{
						if (c.first == id2)
						{
							oe_name = c.second;
						}
					}
					Edge e0(id2, 0, compute_id, 0, oe_name);
					Edge e1(const_id, 0, compute_id, 1);
					g_.add_edge(e0);
					g_.add_edge(e1);
					id2 = compute_id;
				}
			}

			if (j != 0)
			{
				ComputationVertex v0;
				v0.func_name = "+";
				int compute_id = g_.add_vertex(v0, parent_id_, child_index_);

				string oe_name = "";
				for (auto &c : out_edge_name)
				{
					if (c.first == id)
					{
						oe_name = c.second;
					}
				}
				Edge e0(id, 0, compute_id, 0, oe_name);
				oe_name = "";
				for (auto &c : out_edge_name)
				{
					if (c.first == id2)
					{
						oe_name = c.second;
					}
				}
				Edge e1(id2, 0, compute_id, 1, oe_name);
				g_.add_edge(e0);
				g_.add_edge(e1);
				id = compute_id;
			}
			else
			{
				id = id2;
			}
			j++;
		}
	}
	return id;
}

EdgeProperty ExprSimpEngine::cal_var_record(Graph &g_, boost::graph_traits<Graph>::vertex_descriptor v_)
{
	switch (g_[v_]->vertex_type)
	{
	case Vertex::VAR_VERTEX:
	{
		graph_traits<Graph>::out_edge_iterator ei, ei_end;
		EdgeProperty vr;
		for (tie(ei, ei_end) = out_edges(v_, g_); ei != ei_end; ei++)
		{
			if (!(g_[*ei].flag_set))
			{
				IntPolynomialTable ipt({{{static_cast<VarVertex *>(g_[v_])->var_name}, 1}});
				g_[*ei].ipt = ipt;
				vr.ipt = ipt;
				g_[*ei].flag_set = true;
				g_[*ei].flag_unsimplified = false;
			}
		}
		return vr;
		break;
	}
	case Vertex::REG_VAR_VERTEX:
	{
		graph_traits<Graph>::out_edge_iterator ei, ei_end;
		EdgeProperty vr;
		for (tie(ei, ei_end) = out_edges(v_, g_); ei != ei_end; ei++)
		{
			if (!(g_[*ei].flag_set))
			{
				IntPolynomialTable ipt({{{static_cast<RegVarVertex *>(g_[v_])->var_name}, 1}});
				g_[*ei].ipt = ipt;
				vr.ipt = ipt;
				g_[*ei].flag_set = true;
				g_[*ei].flag_unsimplified = true;
			}
		}
		graph_traits<Graph>::in_edge_iterator iei, iei_end;
		for (tie(iei, iei_end) = in_edges(v_, g_); iei != iei_end; iei++)
		{
			if (!(g_[*iei].flag_set))
			{
				vr = cal_var_record(g_, source(*iei, g_));
			}
		}
		return vr;
		break;
	}
	case Vertex::SRAM_VAR_VERTEX:
	{
		graph_traits<Graph>::out_edge_iterator ei, ei_end;
		EdgeProperty vr;
		for (tie(ei, ei_end) = out_edges(v_, g_); ei != ei_end; ei++)
		{
			if (!(g_[*ei].flag_set))
			{
				IntPolynomialTable ipt({{{static_cast<SramVarVertex *>(g_[v_])->var_name}, 1}});
				g_[*ei].ipt = ipt;
				vr.ipt = ipt;
				vr.flag_unsimplified = true;
				g_[*ei].flag_set = true;
				g_[*ei].flag_unsimplified = true;
			}
		}
		graph_traits<Graph>::in_edge_iterator iei, iei_end;
		for (tie(iei, iei_end) = in_edges(v_, g_); iei != iei_end; iei++)
		{
			if (!(g_[*iei].flag_set))
			{
				vr = cal_var_record(g_, source(*iei, g_));
			}
		}
		return vr;
		break;
	}
	case Vertex::CONST_VERTEX:
	{
		graph_traits<Graph>::out_edge_iterator ei, ei_end;
		EdgeProperty vr;
		for (tie(ei, ei_end) = out_edges(v_, g_); ei != ei_end; ei++)
		{
			if (!(g_[*ei].flag_set))
			{
				multiset<string> s;
				IntPolynomialTable ipt({{s, (static_cast<ConstVertex *>(g_[v_]))->int_data}});
				g_[*ei].ipt = ipt;
				vr.ipt = ipt;
				vr.flag_unsimplified = false;
				g_[*ei].flag_set = true;
				g_[*ei].flag_unsimplified = false;
			}
		}
		return vr;
		break;
	}
	case Vertex::REG_SINK_VERTEX:
	case Vertex::SRAM_SINK_VERTEX:
	case Vertex::VAR_SINK_VERTEX:
	{
		graph_traits<Graph>::in_edge_iterator ei, ei_end;
		EdgeProperty vr;
		for (tie(ei, ei_end) = in_edges(v_, g_); ei != ei_end; ei++)
		{
			if (!(g_[*ei].flag_set))
			{
				vr = cal_var_record(g_, source(*ei, g_));
			}
		}
		return vr;
		break;
	}
	case Vertex::GENERAL_HIERARCHICAL_VERTEX:
	case Vertex::BRANCH_VERTEX:
	case Vertex::LOOP_VERTEX:
	{
		graph_traits<Graph>::in_edge_iterator ei, ei_end;
		EdgeProperty vr;
		for (tie(ei, ei_end) = in_edges(v_, g_); ei != ei_end; ei++)
		{
			if (!(g_[*ei].flag_set))
			{
				cal_var_record(g_, source(*ei, g_));
			}
		}
		graph_traits<Graph>::out_edge_iterator oei, oei_end;
		for (tie(oei, oei_end) = out_edges(v_, g_); oei != oei_end; oei++)
		{
			if (!(g_[*oei].flag_set))
			{
				vr.flag_unsimplified = true;
				g_[*oei].flag_set = true;
				g_[*oei].flag_unsimplified = true;
			}
		}

		return vr;
		break;
	}
	case Vertex::COMPUTATION_VERTEX:
	{
		bool flag_unsimplified = false;
		std::unordered_map<int, EdgeProperty> map_vr;
		graph_traits<Graph>::in_edge_iterator ei, ei_end;
		if (static_cast<cidfg::ComputationVertex *>(g_[v_])->is_on_dpu || static_cast<cidfg::ComputationVertex *>(g_[v_])->dont_touch)
		{
			flag_unsimplified = true;
		}
		for (tie(ei, ei_end) = in_edges(v_, g_); ei != ei_end; ei++)
		{
			if (g_[*ei].edge.edge_type == cidfg::Edge::DEPENDENCY)
			{
				flag_unsimplified = true;
				g_[*ei].flag_set = true;
			}
			else
			{
				if (!(g_[*ei].flag_set))
				{
					cal_var_record(g_, source(*ei, g_));
				}
				map_vr[g_[*ei].dest_port] = g_[*ei];
				if (g_[*ei].flag_unsimplified)
				{
					flag_unsimplified = true;
				}
			}
		}
		ComputationVertex *cv = static_cast<ComputationVertex *>(g_[v_]);
		if (cv->func_name == "ld")
		{
			flag_unsimplified = true;
		}

		EdgeProperty vr;
		if (flag_unsimplified == false)
		{
			ComputationVertex *cv = static_cast<ComputationVertex *>(g_[v_]);

			if (cv->func_name == "+" || cv->func_name == "add")
			{
				vr.ipt.add(map_vr[0].ipt);
				vr.ipt.add(map_vr[1].ipt);
			}
			else if (cv->func_name == "-" || cv->func_name == "sub")
			{
				vr.ipt.add(map_vr[0].ipt);
				vr.ipt.sub(map_vr[1].ipt);
			}
			else if (cv->func_name == "*" || cv->func_name == "mul")
			{
				vr.ipt.add(map_vr[0].ipt);
				vr.ipt.mul(map_vr[1].ipt);
			}
			else if (cv->func_name == "/" || cv->func_name == "div")
			{
				vr.ipt.add(map_vr[0].ipt);
				vr.ipt.div_const(map_vr[1].ipt);
			}
			else if (cv->func_name == "=" || cv->func_name == "assign")
			{
				vr.ipt.add(map_vr[0].ipt);
			}
			else if (cv->func_name == "zeros" || cv->func_name == "ones" || cv->func_name == "range" || cv->func_name == "fi")
			{
				flag_unsimplified = true;
			}
			else
			{
				LOG(DEBUG) << "Unsimplified computation nodes [" << cv->func_name << "]";
				flag_unsimplified = true;
			}
		}
		if (flag_unsimplified == false)
		{
			graph_traits<Graph>::out_edge_iterator oei, oei_end;
			for (tie(oei, oei_end) = out_edges(v_, g_); oei != oei_end; oei++)
			{
				if (g_[*oei].edge.edge_type != cidfg::Edge::DEPENDENCY)
				{
					g_[*oei].ipt = vr.ipt;
					vr.flag_unsimplified = false;
					g_[*oei].flag_set = true;
					g_[*oei].flag_unsimplified = false;
				}
				else
				{
					vr.flag_unsimplified = true;
					g_[*oei].flag_set = true;
					g_[*oei].flag_unsimplified = true;
				}
			}
		}
		else
		{
			graph_traits<Graph>::out_edge_iterator oei, oei_end;
			for (tie(oei, oei_end) = out_edges(v_, g_); oei != oei_end; oei++)
			{
				g_[*oei].ipt = vr.ipt;
				vr.flag_unsimplified = true;
				g_[*oei].flag_set = true;
				g_[*oei].flag_unsimplified = true;
			}
		}
		return vr;
		break;
	}
	default:
	{
		std::unordered_map<int, EdgeProperty> map_vr;
		graph_traits<Graph>::in_edge_iterator ei, ei_end;
		for (tie(ei, ei_end) = in_edges(v_, g_); ei != ei_end; ei++)
		{
			if (!(g_[*ei].flag_set))
			{
				cal_var_record(g_, source(*ei, g_));
			}
		}

		EdgeProperty vr;
		graph_traits<Graph>::out_edge_iterator oei, oei_end;
		for (tie(oei, oei_end) = out_edges(v_, g_); oei != oei_end; oei++)
		{
			g_[*oei].ipt = vr.ipt;
			vr.flag_unsimplified = true;
			g_[*oei].flag_set = true;
			g_[*oei].flag_unsimplified = true;
		}
		return vr;
		break;
	}
	}

	EdgeProperty vr;
	return vr;
}

void ExprSimpEngine::find_source_vertices(Graph &g0_, boost::graph_traits<Graph>::edge_descriptor e0_, VarTable &vt_)
{
	boost::graph_traits<Graph>::vertex_descriptor v = source(e0_, g0_);
	boost::graph_traits<Graph>::in_edge_iterator ei, ei_end;
	for (tie(ei, ei_end) = in_edges(v, g0_); ei != ei_end; ei++)
	{
		boost::graph_traits<Graph>::vertex_descriptor vi = source(*ei, g0_);
		if (g0_[vi]->vertex_type == cidfg::Vertex::VAR_VERTEX)
		{
			vt_.update_var(static_cast<cidfg::SourceVertex *>(g0_[vi])->var_name, g0_[vi]->id, 0, "", cidfg::Edge::SCALAR_DATA_SIG, static_cast<cidfg::SourceVertex *>(g0_[vi])->coord, false);
		}
		else
		{
			find_source_vertices(g0_, *ei, vt_);
		}
	}
}

} // namespace feopt
} // namespace vesyla