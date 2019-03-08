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

#include "SwbRearrangeEngine.hpp"

using namespace vesyla::cidfg;

namespace vesyla
{
namespace codegen
{

void SwbRearrangeEngine::find_all_swb_in_hierarchical_vetex(cidfg::CidfgGraph &g_, int v_, vector<int> &swb_instr_vertices_)
{
  HierarchicalVertex *vv = static_cast<HierarchicalVertex *>(g_.get_vertex(v_));
  for (int i = 0; i < vv->children.size(); i++)
  {
    for (int j = 0; j < vv->children[i].size(); j++)
    {
      if (g_.get_vertex(vv->children[i][j])->vertex_type == Vertex::SWB_INSTR_VERTEX)
      {
        swb_instr_vertices_.push_back(vv->children[i][j]);
      }
      else if (g_.get_vertex(vv->children[i][j])->is_hierarchical())
      {
        find_all_swb_in_hierarchical_vetex(g_, vv->children[i][j], swb_instr_vertices_);
      }
    }
  }
}
void SwbRearrangeEngine::transform(cidfg::CidfgGraph &g_)
{

  bool flag_changed = true;
  while (flag_changed)
  {
    flag_changed = false;
    for (auto &v : g_.get_vertices())
    {
      if (v->vertex_type == Vertex::ROOT_VERTEX)
      {
        continue;
      }
      if (v->is_hierarchical())
      {
        std::vector<int> swb_instr_vertices;
        std::vector<int> all_swb_instr_vertices;
        HierarchicalVertex *vv = static_cast<HierarchicalVertex *>(v);
        for (int i = 0; i < vv->children.size(); i++)
        {
          for (int j = 0; j < vv->children[i].size(); j++)
          {
            if (g_.get_vertex(vv->children[i][j])->vertex_type == Vertex::SWB_INSTR_VERTEX)
            {
              swb_instr_vertices.push_back(vv->children[i][j]);
            }
          }
        }
        find_all_swb_in_hierarchical_vetex(g_, v->id, all_swb_instr_vertices);
        for (int i = 0; i < swb_instr_vertices.size(); i++)
        {
          if (swb_instr_vertices[i] < 0)
          {
            continue;
          }
          SwbInstrVertex *v0 = static_cast<SwbInstrVertex *>(g_.get_vertex(swb_instr_vertices[i]));
          BIR::SWBInstruction *instr0 = static_cast<BIR::SWBInstruction *>(v0->instr);
          string str0 = instr0->destination.to_string();
          for (int j = 0; j < all_swb_instr_vertices.size(); j++)
          {
            if (all_swb_instr_vertices[j] < 0)
            {
              continue;
            }
            if (all_swb_instr_vertices[j] == swb_instr_vertices[i])
            {
              continue;
            }
            SwbInstrVertex *v1 = static_cast<SwbInstrVertex *>(g_.get_vertex(all_swb_instr_vertices[j]));
            BIR::SWBInstruction *instr1 = static_cast<BIR::SWBInstruction *>(v1->instr);
            string str1 = instr1->destination.to_string();
            if (str0 == str1)
            {
              all_swb_instr_vertices[j] = -1;
              swb_instr_vertices[i] = -1;
              break;
            }
          }
        }

        int curr_parent_id;
        int curr_child_index;
        g_.get_parent(v->id, curr_parent_id, curr_child_index);
        HierarchicalVertex *parent_vertex = static_cast<HierarchicalVertex *>(g_.get_vertex(curr_parent_id));
        if (curr_parent_id >= 0 && curr_child_index >= 0)
        {
          for (int i = 0; i < swb_instr_vertices.size(); i++)
          {
            if (swb_instr_vertices[i] < 0)
            {
              continue;
            }

            int id = swb_instr_vertices[i];
            int parent_id;
            int child_index;
            g_.get_parent(id, parent_id, child_index);
            if (parent_id >= 0 && child_index >= 0)
            {
              HierarchicalVertex *vertex = static_cast<HierarchicalVertex *>(g_.get_vertex(parent_id));
              vector<int> vec = vertex->children[child_index];
              vertex->children[child_index].clear();
              for (auto &c : vec)
              {
                if (c == id)
                {
                  continue;
                }
                vertex->children[child_index].push_back(c);
              }
            }
            flag_changed = true;
            parent_vertex->add_child(curr_child_index, id);

            // change the outgoing dependency edge point to the hierarchical vertex
            for (auto &e0 : g_.get_edges())
            {
              if (e0->edge_type == cidfg::Edge::DEPENDENCY && e0->src_id == id)
              {
                e0->dest_id = parent_id;
              }
            }
          }
        }
      }
    }
  }
}
} // namespace codegen
} // namespace vesyla