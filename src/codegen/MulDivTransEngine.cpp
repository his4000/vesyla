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

#include "MulDivTransEngine.hpp"

using namespace vesyla::cidfg;

namespace vesyla
{
namespace codegen
{

void MulDivTransEngine::transform(cidfg::CidfgGraph &g_)
{
	std::set<int> removed_vertices;
	for (auto &v : g_.get_vertices())
	{
		if (v->vertex_type == cidfg::Vertex::COMPUTATION_VERTEX)
		{
			ComputationVertex *cv = static_cast<ComputationVertex *>(v);
			if (!cv->is_on_dpu && !cv->dont_touch)
			{
				if (cv->func_name == "*" || cv->func_name == "mult" || cv->func_name == "MULT")
				{
					vector<int> saved_edges;
					for (auto &e : g_.get_edges())
					{
						if (e->src_id == v->id)
						{
							saved_edges.push_back(e->id);
						}
					}
					Vertex *v_arg0 = g_.get_vertex(g_.get_edge(g_.get_in_edge(v->id, 0))->src_id);
					Vertex *v_arg1 = g_.get_vertex(g_.get_edge(g_.get_in_edge(v->id, 1))->src_id);
					int parent_id;
					int child_index;
					g_.get_parent(v->id, parent_id, child_index);
					if (v_arg0->vertex_type == cidfg::Vertex::CONST_VERTEX)
					{
						Vertex *tmp;
						tmp = v_arg0;
						v_arg0 = v_arg1;
						v_arg1 = tmp;
					}
					if (v_arg1->vertex_type == cidfg::Vertex::CONST_VERTEX)
					{
						removed_vertices.insert(v->id);
						removed_vertices.insert(v_arg1->id);
						int val = static_cast<ConstVertex *>(v_arg1)->int_data;
						int flag_negative = false;
						if (val < 0)
						{
							flag_negative = true;
							val = -val;
						}
						vector<int> shift_nodes;
						int test_val = 1;
						for (int i = 0; i < 32; i++)
						{
							if ((test_val & val) != 0)
							{
								if (i == 0)
								{
									shift_nodes.push_back(v_arg0->id);
								}
								else
								{
									ConstVertex v0;
									v0.set_int_value(i);
									v0.coord = v->coord;
									int id_v0 = g_.add_vertex(v0, parent_id, child_index);
									ComputationVertex v1;
									v1.func_name = "<<";
									v1.coord = v->coord;
									int id_v1 = g_.add_vertex(v1, parent_id, child_index);
									Edge e0(v_arg0->id, 0, id_v1, 0);
									Edge e1(id_v0, 0, id_v1, 1);
									g_.add_edge(e0);
									g_.add_edge(e1);
									shift_nodes.push_back(id_v1);
								}
							}
							test_val = test_val << 1;
						}
						int ret_id = v_arg0->id;
						for (int i = 0; i < shift_nodes.size(); i++)
						{
							if (i == 0)
							{
								ret_id = shift_nodes[i];
							}
							else
							{
								ComputationVertex v0;
								v0.func_name = "+";
								v0.coord = v->coord;
								int id_v0 = g_.add_vertex(v0, parent_id, child_index);
								Edge e0(ret_id, 0, id_v0, 0);
								Edge e1(shift_nodes[i], 0, id_v0, 1);
								g_.add_edge(e0);
								g_.add_edge(e1);
								ret_id = id_v0;
							}
						}

						if (flag_negative)
						{
							ConstVertex v0;
							v0.set_int_value(0);
							int id_v0 = g_.add_vertex(v0, parent_id, child_index);
							ComputationVertex v1;
							v1.func_name = "-";
							v1.coord = v->coord;
							int id_v1 = g_.add_vertex(v1, parent_id, child_index);
							Edge e0(id_v0, 0, id_v1, 0);
							Edge e1(ret_id, 0, id_v1, 1);
							g_.add_edge(e0);
							g_.add_edge(e1);
							ret_id = id_v1;
						}
						for (auto &eid : saved_edges)
						{
							g_.get_edge(eid)->src_id = ret_id;
						}
					}
				}
				else if (cv->func_name == "/" || cv->func_name == "div" || cv->func_name == "DIV")
				{
					vector<int> saved_edges;
					for (auto &e : g_.get_edges())
					{
						if (e->src_id == v->id)
						{
							saved_edges.push_back(e->id);
						}
					}
					Vertex *v_arg0 = g_.get_vertex(g_.get_edge(g_.get_in_edge(v->id, 0))->src_id);
					Vertex *v_arg1 = g_.get_vertex(g_.get_edge(g_.get_in_edge(v->id, 1))->src_id);
					int parent_id;
					int child_index;
					g_.get_parent(v->id, parent_id, child_index);
					if (v_arg1->vertex_type == cidfg::Vertex::CONST_VERTEX)
					{
						int val = static_cast<ConstVertex *>(v_arg1)->int_data;
						int flag_negative = false;
						if (val < 0)
						{
							flag_negative = true;
							val = -val;
						}
						int ret_id;
						if (val == 1)
						{
							removed_vertices.insert(v->id);
							removed_vertices.insert(v_arg1->id);
							ret_id = v_arg0->id;
						}
						else if (abs(log2(val) - (int)(log2(2))) < 0.00001) // check whether it's 2^n
						{
							removed_vertices.insert(v->id);
							removed_vertices.insert(v_arg1->id);
							int flag_negative = false;
							if (val < 0)
							{
								flag_negative = true;
								val = -val;
							}

							ConstVertex v0;
							v0.set_int_value((int)(log2(val)));
							v0.coord = v->coord;
							int id_v0 = g_.add_vertex(v0, parent_id, child_index);
							ComputationVertex v1;
							v1.func_name = ">>";
							v1.coord = v->coord;
							int id_v1 = g_.add_vertex(v1, parent_id, child_index);
							Edge e0(v_arg0->id, 0, id_v1, 0);
							Edge e1(id_v0, 0, id_v1, 1);
							g_.add_edge(e0);
							g_.add_edge(e1);
							ret_id = id_v1;
						}
						else
						{
							ret_id = -1;
						}
						if (ret_id >= 0)
						{
							if (flag_negative)
							{
								ConstVertex v0;
								v0.set_int_value(0);
								int id_v0 = g_.add_vertex(v0, parent_id, child_index);
								ComputationVertex v1;
								v1.func_name = "-";
								v1.coord = v->coord;
								int id_v1 = g_.add_vertex(v1, parent_id, child_index);
								Edge e0(id_v0, 0, id_v1, 0);
								Edge e1(ret_id, 0, id_v1, 1);
								g_.add_edge(e0);
								g_.add_edge(e1);
								ret_id = id_v1;
							}
							for (auto &eid : saved_edges)
							{
								g_.get_edge(eid)->src_id = ret_id;
							}
						}
					}
				}
			}
		}
	}

	for (auto &id : removed_vertices)
	{
		g_.del_vertex(id);
	}
}

} // namespace codegen
} // namespace vesyla