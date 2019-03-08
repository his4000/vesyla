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

#include "RwiMergeEngine.hpp"

using namespace vesyla::cidfg;

namespace vesyla
{
namespace feopt
{

void RwiMergeEngine::transform(CidfgGraph &g_)
{
	set<int> children_id;
	set<int> removed_vertices;
	merge_rw_index(g_, children_id, -1, -1, -1, removed_vertices);
	for (auto &id : removed_vertices)
	{
		g_.del_vertex(id);
	}
}

void RwiMergeEngine::merge_rw_index(CidfgGraph &g_, set<int> &children_id_, int vertex_id_, int parent_vertex_id_, int child_index_, set<int> &removed_vertices_)
{
	Vertex *vertex;
	if (vertex_id_ < 0)
	{
		for (auto &v : g_.get_vertices())
		{
			if (v->vertex_type == Vertex::ROOT_VERTEX)
			{
				vertex = v;
				break;
			}
		}
	}
	else
	{
		vertex = g_.get_vertex(vertex_id_);
	}

	if (vertex->vertex_type == Vertex::READING_VERTEX)
	{
		Edge *e0 = g_.get_edge(g_.get_in_edge(vertex->id, 0));
		Edge *e1 = g_.get_edge(g_.get_in_edge(vertex->id, 1));
		Vertex *v_index = g_.get_vertex(e1->src_id);
		CHECK_EQ(v_index->vertex_type, Vertex::INDEXING_VERTEX);
		removed_vertices_.insert(vertex->id);
		removed_vertices_.insert(v_index->id);
		ReadAndIndexVertex v_rdi;
		v_rdi.is_sram = static_cast<ReadingVertex *>(vertex)->is_sram;
		v_rdi.en_compression = static_cast<ReadingVertex *>(vertex)->en_compression;
		int id_rdi = g_.add_vertex(v_rdi, parent_vertex_id_, child_index_);
		e0->dest_id = id_rdi;
		e0->dest_port = 0;
		for (int i = 0; i < 5; i++)
		{
			Edge *e;
			e = g_.get_edge(g_.get_in_edge(v_index->id, i));
			e->dest_id = id_rdi;
			e->dest_port = i + 1;
		}
		g_.del_edge(e1->id);
		Edge *e3 = g_.get_edge(g_.get_out_edge(vertex->id, 0));
		e3->src_id = id_rdi;
		e3->src_port = 0;

		for (auto &ed : g_.get_edges())
		{
			//      if (ed->edge_type == Edge::DEPENDENCY)
			//      {
			if (ed->src_id == vertex->id)
			{
				ed->src_id = id_rdi;
			}
			if (ed->dest_id == vertex->id)
			{
				ed->dest_id = id_rdi;
			}
			//      }
		}

		children_id_.insert(vertex->id);
		children_id_.insert(v_index->id);
	}
	else if (vertex->vertex_type == Vertex::WRITING_VERTEX)
	{
		Edge *e0 = g_.get_edge(g_.get_in_edge(vertex->id, 0));
		Edge *e1 = g_.get_edge(g_.get_in_edge(vertex->id, 1));
		Edge *e2 = g_.get_edge(g_.get_in_edge(vertex->id, 2));
		Vertex *v_index = g_.get_vertex(e1->src_id);
		CHECK_EQ(v_index->vertex_type, Vertex::INDEXING_VERTEX);
		removed_vertices_.insert(vertex->id);
		removed_vertices_.insert(v_index->id);
		WriteAndIndexVertex v_wri;
		v_wri.is_sram = static_cast<WritingVertex *>(vertex)->is_sram;
		v_wri.en_compression = static_cast<WritingVertex *>(vertex)->en_compression;
		int id_wri = g_.add_vertex(v_wri, parent_vertex_id_, child_index_);
		e0->dest_id = id_wri;
		e0->dest_port = 0;
		e2->dest_id = id_wri;
		e2->dest_port = 1;
		for (int i = 0; i < 5; i++)
		{
			Edge *e;
			e = g_.get_edge(g_.get_in_edge(v_index->id, i));
			e->dest_id = id_wri;
			e->dest_port = i + 2;
		}
		g_.del_edge(e1->id);

		for (auto &e3 : g_.get_edges())
		{
			if (e3->src_id == vertex->id && e3->src_port == 0)
			{
				e3->src_id = id_wri;
				e3->src_port = 0;
			}
		}

		for (auto &ed : g_.get_edges())
		{
			//     if (ed->edge_type == Edge::DEPENDENCY)
			//     {
			if (ed->src_id == vertex->id)
			{
				ed->src_id = id_wri;
			}
			if (ed->dest_id == vertex->id)
			{
				ed->dest_id = id_wri;
			}
			//     }
		}

		children_id_.insert(vertex->id);
		children_id_.insert(v_index->id);
	}
	else if (vertex->is_hierarchical())
	{
		HierarchicalVertex *vv = static_cast<HierarchicalVertex *>(vertex);
		for (int i = 0; i < vv->children.size(); i++)
		{
			for (int j = 0; j < vv->children[i].size(); j++)
			{
				merge_rw_index(g_, children_id_, vv->children[i][j], vv->id, i, removed_vertices_);
			}
		}
	}
}

} // namespace feopt
} // namespace vesyla