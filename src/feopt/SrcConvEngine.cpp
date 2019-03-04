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

#include "SrcConvEngine.hpp"

using namespace vesyla::cidfg;

namespace vesyla
{
namespace feopt
{

void SrcConvEngine::transform(CidfgGraph &g_)
{
	std::set<int> removed_vertices;
	for (auto &v : g_.get_vertices())
	{
		if (v->vertex_type == Vertex::REG_VAR_VERTEX)
		{
			cidfg::RegVarVertex *v_src = static_cast<RegVarVertex *>(v);
			cidfg::Vertex *v_init = g_.get_vertex(g_.get_edge(g_.get_in_edge(v->id, 0))->src_id);
			CHECK_EQ(v_init->vertex_type, Vertex::COMPUTATION_VERTEX);
			ComputationVertex *vv = static_cast<ComputationVertex *>(v_init);

			int flag_isfixed = false;
			if (vv->func_name == "fi")
			{
				removed_vertices.insert(vv->id);
				flag_isfixed = true;
				cidfg::Vertex *v_init1 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 0))->src_id);
				CHECK_EQ(v_init1->vertex_type, Vertex::COMPUTATION_VERTEX);
				vv = static_cast<ComputationVertex *>(v_init1);
			}

			if (vv->func_name == "zeros")
			{
				cidfg::Vertex *v_par0 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 0))->src_id);
				cidfg::Vertex *v_par1 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 1))->src_id);
				CHECK_EQ(v_par0->vertex_type, cidfg::Vertex::CONST_VERTEX);
				CHECK_EQ(v_par1->vertex_type, cidfg::Vertex::CONST_VERTEX);
				int size = static_cast<cidfg::ConstVertex *>(v_par0)->int_data * static_cast<cidfg::ConstVertex *>(v_par1)->int_data;
				if (!flag_isfixed)
				{
					v_src->value_type = cidfg::SourceVertex::INT;
					v_src->int_data_array.clear();
					v_src->int_data_array.resize(size, 0);
				}
				else
				{
					v_src->value_type = cidfg::SourceVertex::FLOAT;
					v_src->float_data_array.clear();
					v_src->float_data_array.resize(size, 0.0);
				}
				removed_vertices.insert(vv->id);
				//			removed_vertices.insert(v_par0->id);
				//			removed_vertices.insert(v_par1->id);
			}
			else if (vv->func_name == "ones")
			{
				cidfg::Vertex *v_par0 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 0))->src_id);
				cidfg::Vertex *v_par1 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 1))->src_id);
				CHECK_EQ(v_par0->vertex_type, cidfg::Vertex::CONST_VERTEX);
				CHECK_EQ(v_par1->vertex_type, cidfg::Vertex::CONST_VERTEX);
				int size = static_cast<cidfg::ConstVertex *>(v_par0)->int_data * static_cast<cidfg::ConstVertex *>(v_par1)->int_data;
				if (!flag_isfixed)
				{
					v_src->value_type = cidfg::SourceVertex::INT;
					v_src->int_data_array.clear();
					v_src->int_data_array.resize(size, 1);
				}
				else
				{
					v_src->value_type = cidfg::SourceVertex::FLOAT;
					v_src->float_data_array.clear();
					v_src->float_data_array.resize(size, 1.0);
				}
				removed_vertices.insert(vv->id);
				//			removed_vertices.insert(v_par0->id);
				//			removed_vertices.insert(v_par1->id);
			}
			else if (vv->func_name == "range")
			{
				cidfg::Vertex *v_par0 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 0))->src_id);
				cidfg::Vertex *v_par1 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 1))->src_id);
				cidfg::Vertex *v_par2 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 2))->src_id);
				CHECK_EQ(v_par0->vertex_type, cidfg::Vertex::CONST_VERTEX);
				CHECK_EQ(v_par1->vertex_type, cidfg::Vertex::CONST_VERTEX);
				CHECK_EQ(v_par2->vertex_type, cidfg::Vertex::CONST_VERTEX);

				int size = static_cast<cidfg::ConstVertex *>(v_par2)->int_data;
				int init_data = static_cast<cidfg::ConstVertex *>(v_par0)->int_data;
				int increment_data = static_cast<cidfg::ConstVertex *>(v_par1)->int_data;
				int temp = init_data;
				if (!flag_isfixed)
				{
					v_src->value_type = cidfg::SourceVertex::INT;
					v_src->int_data_array.clear();
				}
				else
				{
					v_src->value_type = cidfg::SourceVertex::FLOAT;
					v_src->float_data_array.clear();
				}
				for (int i = 0; i < size; i++)
				{
					if (!flag_isfixed)
					{
						v_src->int_data_array.push_back(temp);
					}
					else
					{
						v_src->float_data_array.push_back((float)(temp));
					}
					temp += increment_data;
				}

				removed_vertices.insert(vv->id);
				//			removed_vertices.insert(v_par0->id);
				//				removed_vertices.insert(v_par1->id);
				//			removed_vertices.insert(v_par2->id);
			}
			else if (vv->func_name == "array")
			{
				if (!flag_isfixed)
				{
					v_src->value_type = cidfg::SourceVertex::INT;
					v_src->int_data_array.clear();
				}
				else
				{
					v_src->value_type = cidfg::SourceVertex::FLOAT;
					v_src->float_data_array.clear();
				}
				int size = 0;
				for (auto &e : g_.get_edges())
				{
					if (e->dest_id == vv->id)
					{
						size++;
					}
				}
				for (int i = 0; i < size; i++)
				{
					cidfg::Vertex *v_par0 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, i))->src_id);
					CHECK_EQ(v_par0->vertex_type, cidfg::Vertex::CONST_VERTEX);
					if (!flag_isfixed)
					{
						int temp = static_cast<cidfg::ConstVertex *>(v_par0)->int_data;
						v_src->int_data_array.push_back(temp);
					}
					else
					{
						float temp = static_cast<cidfg::ConstVertex *>(v_par0)->float_data;
						v_src->float_data_array.push_back(temp);
					}
					removed_vertices.insert(v_par0->id);
				}
				removed_vertices.insert(vv->id);
			}
			else
			{
				LOG(FATAL) << "Unsupported initialization function : " << vv->func_name;
			}
		}
		else if (v->vertex_type == Vertex::SRAM_VAR_VERTEX)
		{
			cidfg::SramVarVertex *v_src = static_cast<SramVarVertex *>(v);
			cidfg::Vertex *v_init = g_.get_vertex(g_.get_edge(g_.get_in_edge(v->id, 0))->src_id);
			CHECK_EQ(v_init->vertex_type, Vertex::COMPUTATION_VERTEX);
			ComputationVertex *vv = static_cast<ComputationVertex *>(v_init);

			int flag_isfixed = false;
			if (vv->func_name == "fi")
			{
				removed_vertices.insert(vv->id);
				flag_isfixed = true;
				cidfg::Vertex *v_init1 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 0))->src_id);
				CHECK_EQ(v_init1->vertex_type, Vertex::COMPUTATION_VERTEX);
				vv = static_cast<ComputationVertex *>(v_init1);
			}

			if (vv->func_name == "zeros")
			{
				cidfg::Vertex *v_par0 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 0))->src_id);
				cidfg::Vertex *v_par1 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 1))->src_id);
				CHECK_EQ(v_par0->vertex_type, cidfg::Vertex::CONST_VERTEX);
				CHECK_EQ(v_par1->vertex_type, cidfg::Vertex::CONST_VERTEX);
				int size = static_cast<cidfg::ConstVertex *>(v_par0)->int_data * static_cast<cidfg::ConstVertex *>(v_par1)->int_data;
				if (!flag_isfixed)
				{
					v_src->value_type = cidfg::SourceVertex::INT;
					v_src->int_data_array.clear();
					v_src->int_data_array.resize(size, 0);
				}
				else
				{
					v_src->value_type = cidfg::SourceVertex::FLOAT;
					v_src->float_data_array.clear();
					v_src->float_data_array.resize(size, 0.0);
				}
				removed_vertices.insert(vv->id);
				//				removed_vertices.insert(v_par0->id);
				//				removed_vertices.insert(v_par1->id);
			}
			else if (vv->func_name == "ones")
			{
				cidfg::Vertex *v_par0 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 0))->src_id);
				cidfg::Vertex *v_par1 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 1))->src_id);
				CHECK_EQ(v_par0->vertex_type, cidfg::Vertex::CONST_VERTEX);
				CHECK_EQ(v_par1->vertex_type, cidfg::Vertex::CONST_VERTEX);
				int size = static_cast<cidfg::ConstVertex *>(v_par0)->int_data * static_cast<cidfg::ConstVertex *>(v_par1)->int_data;
				if (!flag_isfixed)
				{
					v_src->value_type = cidfg::SourceVertex::INT;
					v_src->int_data_array.clear();
					v_src->int_data_array.resize(size, 1);
				}
				else
				{
					v_src->value_type = cidfg::SourceVertex::FLOAT;
					v_src->float_data_array.clear();
					v_src->float_data_array.resize(size, 1.0);
				}
				removed_vertices.insert(vv->id);
				//				removed_vertices.insert(v_par0->id);
				//			removed_vertices.insert(v_par1->id);
			}
			else if (vv->func_name == "range")
			{
				cidfg::Vertex *v_par0 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 0))->src_id);
				cidfg::Vertex *v_par1 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 1))->src_id);
				cidfg::Vertex *v_par2 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 2))->src_id);
				CHECK_EQ(v_par0->vertex_type, cidfg::Vertex::CONST_VERTEX);
				CHECK_EQ(v_par1->vertex_type, cidfg::Vertex::CONST_VERTEX);
				CHECK_EQ(v_par2->vertex_type, cidfg::Vertex::CONST_VERTEX);

				int size = static_cast<cidfg::ConstVertex *>(v_par2)->int_data;
				int init_data = static_cast<cidfg::ConstVertex *>(v_par0)->int_data;
				int increment_data = static_cast<cidfg::ConstVertex *>(v_par1)->int_data;
				int temp = init_data;
				if (!flag_isfixed)
				{
					v_src->value_type = cidfg::SourceVertex::INT;
					v_src->int_data_array.clear();
				}
				else
				{
					v_src->value_type = cidfg::SourceVertex::FLOAT;
					v_src->float_data_array.clear();
				}
				for (int i = 0; i < size; i++)
				{
					if (!flag_isfixed)
					{
						v_src->int_data_array.push_back(temp);
					}
					else
					{
						v_src->float_data_array.push_back((float)(temp));
					}
					temp += increment_data;
				}

				removed_vertices.insert(vv->id);
				//				removed_vertices.insert(v_par0->id);
				//				removed_vertices.insert(v_par1->id);
				//				removed_vertices.insert(v_par2->id);
			}
			else if (vv->func_name == "array")
			{
				if (!flag_isfixed)
				{
					v_src->value_type = cidfg::SourceVertex::INT;
					v_src->int_data_array.clear();
				}
				else
				{
					v_src->value_type = cidfg::SourceVertex::FLOAT;
					v_src->float_data_array.clear();
				}
				int size = 0;
				for (auto &e : g_.get_edges())
				{
					if (e->dest_id == vv->id)
					{
						size++;
					}
				}
				for (int i = 0; i < size; i++)
				{
					cidfg::Vertex *v_par0 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, i))->src_id);
					CHECK_EQ(v_par0->vertex_type, cidfg::Vertex::CONST_VERTEX);
					if (!flag_isfixed)
					{
						int temp = static_cast<cidfg::ConstVertex *>(v_par0)->int_data;
						v_src->int_data_array.push_back(temp);
					}
					else
					{
						float temp = static_cast<cidfg::ConstVertex *>(v_par0)->float_data;
						v_src->float_data_array.push_back(temp);
					}
					removed_vertices.insert(v_par0->id);
				}
				removed_vertices.insert(vv->id);
			}
			else
			{
				LOG(FATAL) << "Unsupported initialization function : " << vv->func_name;
			}
		}
	}

	// // Remove other "fi" nodes
	for (auto &v : g_.get_vertices())
	{
		if (v->vertex_type == Vertex::COMPUTATION_VERTEX && static_cast<cidfg::ComputationVertex *>(v)->func_name == "fi")
		{
			cidfg::ComputationVertex *v_2 = static_cast<ComputationVertex *>(v);
			cidfg::Vertex *v_1 = g_.get_vertex(g_.get_edge(g_.get_in_edge(v->id, 0))->src_id);
			cidfg::Edge *e_3 = g_.get_edge(g_.get_out_edge(v->id, 0));
			if (v_1->vertex_type, Vertex::CONST_VERTEX)
			{
				cidfg::ConstVertex *v11 = static_cast<ConstVertex *>(v_1);
				if (v11->value_type != ConstVertex::FLOAT)
				{
					v11->value_type = ConstVertex::FLOAT;
					v11->set_float_value(float(v11->int_data));
				}
			}

			e_3->src_id = v_1->id;
			removed_vertices.insert(v->id);
		}
	}

	for (auto &id : removed_vertices)
	{
		g_.del_vertex(id);
	}
}

} // namespace feopt
} // namespace vesyla