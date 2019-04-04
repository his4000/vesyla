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

#include "RaccuOpMergeEngine.hpp"
using namespace vesyla::cidfg;

namespace vesyla
{
namespace codegen
{
void RaccuOpMergeEngine::transform(CidfgGraph &g_)
{
	bool flag_changed = true;
	while (flag_changed)
	{
		flag_changed = false;
		set<int> removed_vertices;
		set<int> all_vertices;
		for (auto &v : g_.get_vertices())
		{
			if (v->vertex_type == Vertex::COMPUTATION_VERTEX)
			{
				ComputationVertex *vv = static_cast<ComputationVertex *>(v);
				if (!(vv->is_on_dpu))
				{
					all_vertices.insert(vv->id);
				}
			}
		}

		for (auto &id0 : all_vertices)
		{
			if (removed_vertices.find(id0) != removed_vertices.end())
			{
				continue;
			}

			ComputationVertex *v0 = static_cast<ComputationVertex *>(g_.get_vertex(id0));

			for (auto &id1 : all_vertices)
			{
				if (removed_vertices.find(id1) != removed_vertices.end() || id0 == id1)
				{
					continue;
				}

				ComputationVertex *v1 = static_cast<ComputationVertex *>(g_.get_vertex(id1));
				if (v0->func_name == v1->func_name && v0->coord == v1->coord)
				{
					int index = 0;
					bool flag_equal = true;
					do
					{
						int e0 = g_.get_in_edge(v0->id, index);
						int e1 = g_.get_in_edge(v1->id, index);
						if (e0 >= 0 && e1 >= 0)
						{
							Vertex *src0 = g_.get_vertex(g_.get_edge(e0)->src_id);
							Vertex *src1 = g_.get_vertex(g_.get_edge(e1)->src_id);
							if (src0->id == src1->id)
							{
								;
							}
							else
							{
								if (src0->vertex_type == Vertex::CONST_VERTEX && src1->vertex_type == Vertex::CONST_VERTEX)
								{
									if (static_cast<ConstVertex *>(src0)->int_data == static_cast<ConstVertex *>(src1)->int_data)
									{
										;
									}
									else
									{
										flag_equal = false;
										break;
									}
								}
								else
								{
									flag_equal = false;
									break;
								}
							}
						}
						else
						{
							break;
						}
						index++;
					} while (true);

					if (flag_equal == true)
					{
						for (auto &e : g_.get_edges())
						{
							if (e->src_id == v1->id && e->dest_id != v0->id)
							{
								e->src_id = v0->id;
							}
							if (e->dest_id == v1->id && e->src_id != v0->id)
							{
								e->dest_id = v0->id;
							}
						}
						removed_vertices.insert(v1->id);
					}
				}
			}
		}

		for (auto &id : removed_vertices)
		{
			g_.del_vertex(id);
		}
	}
}
} // namespace codegen
} // namespace vesyla