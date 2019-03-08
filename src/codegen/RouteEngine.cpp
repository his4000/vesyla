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

#include "RouteEngine.hpp"
using namespace BIR;
using namespace BIR::BIREnumerations;

using namespace vesyla::cidfg;

namespace vesyla
{
namespace codegen
{
void RouteEngine::transform(cidfg::CidfgGraph &g_)
{
  for (auto &e : g_.get_edges())
  {
    if (e->edge_type != Edge::SCALAR_DATA_SIG)
    {
      continue;
    }

    Vertex *v_src = g_.get_vertex(e->src_id);
    Vertex *v_dest = g_.get_vertex(e->dest_id);

    if (v_src->vertex_type == Vertex::REFI_INSTR_VERTEX && v_dest->vertex_type == Vertex::REFI_INSTR_VERTEX)
    {
      int parent_id;
      int child_index;
      g_.get_parent(e->src_id, parent_id, child_index);
      BIR::SWBInstruction *swb_instr = CREATE_OBJECT_B(SWBInstruction);
      swb_instr->source.cellCoordinate = v_src->coord;
      swb_instr->source.type = BIR::BIREnumerations::ctRegFile;
      swb_instr->source.portNo = e->src_port;
      swb_instr->destination.cellCoordinate = v_dest->coord;
      swb_instr->destination.type = BIR::BIREnumerations::ctRegFile;
      swb_instr->destination.portNo = e->dest_port;
      cidfg::SwbInstrVertex v_swb;
      v_swb.coord = v_dest->coord;
      v_swb.instr = swb_instr;
      int id_swb = g_.add_vertex(v_swb, parent_id, child_index);
      Edge e0(id_swb, 100, e->src_id, 100, "", Edge::DEPENDENCY, 1, INT_MAX);
      g_.add_edge(e0);
    }
    else if (v_src->vertex_type == Vertex::REFI_INSTR_VERTEX && v_dest->vertex_type == Vertex::SRAM_INSTR_VERTEX)
    {
      int parent_id;
      int child_index;
      g_.get_parent(e->src_id, parent_id, child_index);
      BIR::RouteInstruction *route_instr = CREATE_OBJECT_B(RouteInstruction);
      VIR::Coordinate src_coord = v_src->coord;
      VIR::Coordinate dest_coord = static_cast<SramInstrVertex *>(v_dest)->sram_coord;
      route_instr->drraSelect = src_coord.row;
      dest_coord.row += 1; // DiMarch row start from 1, because it consider DRRA row to be 0
      src_coord.row = 0;   // DRRA row is always 0
      route_instr->sourceCoordinate = src_coord;
      route_instr->destCoordinate = dest_coord;
      route_instr->isFromSource = false; // SET IT TO FALSE, the DRRA documentation is wrong
      cidfg::RouteInstrVertex v_route;
      v_route.coord = v_dest->coord;
      v_route.instr = route_instr;
      int id_route = g_.add_vertex(v_route, parent_id, child_index);
      cidfg::Edge e0(id_route, 100, e->src_id, 100, "", cidfg::Edge::DEPENDENCY, 1, INT_MAX);
      cidfg::Edge e1(id_route, 100, e->dest_id, 100, "", cidfg::Edge::DEPENDENCY, 1, INT_MAX);
      g_.add_edge(e0);
      g_.add_edge(e1);

      // fix the coord in SRAM node
      v_dest->coord = v_src->coord;
    }
    else if (v_src->vertex_type == Vertex::SRAM_INSTR_VERTEX && v_dest->vertex_type == Vertex::REFI_INSTR_VERTEX)
    {
      int parent_id;
      int child_index;
      g_.get_parent(e->dest_id, parent_id, child_index);
      BIR::RouteInstruction *route_instr = CREATE_OBJECT_B(RouteInstruction);
      VIR::Coordinate src_coord = static_cast<SramInstrVertex *>(v_src)->sram_coord;
      VIR::Coordinate dest_coord = v_dest->coord;
      route_instr->drraSelect = dest_coord.row;
      src_coord.row += 1; // DiMarch row start from 1, because it consider DRRA row to be 0
      dest_coord.row = 0; // DRRA row is always 0
      route_instr->sourceCoordinate = src_coord;
      route_instr->destCoordinate = dest_coord;
      route_instr->isFromSource = true; // SET IT TO TRUE, the DRRA documentation is wrong
      cidfg::RouteInstrVertex v_route;
      v_route.coord = v_dest->coord;
      v_route.instr = route_instr;
      int id_route = g_.add_vertex(v_route, parent_id, child_index);
      cidfg::Edge e0(id_route, 100, e->src_id, 100, "", cidfg::Edge::DEPENDENCY, 1, INT_MAX);
      cidfg::Edge e1(id_route, 100, e->dest_id, 100, "", cidfg::Edge::DEPENDENCY, 1, INT_MAX);
      g_.add_edge(e0);
      g_.add_edge(e1);

      // fix the coord in SRAM node
      v_src->coord = v_dest->coord;
    }
    else if (v_src->vertex_type == Vertex::REFI_INSTR_VERTEX && v_dest->vertex_type == Vertex::DPU_INSTR_VERTEX)
    {
      int parent_id;
      int child_index;
      g_.get_parent(e->src_id, parent_id, child_index);
      BIR::SWBInstruction *swb_instr = CREATE_OBJECT_B(SWBInstruction);
      swb_instr->source.cellCoordinate = v_src->coord;
      swb_instr->source.type = BIR::BIREnumerations::ctRegFile;
      swb_instr->source.portNo = e->src_port;
      swb_instr->destination.cellCoordinate = v_dest->coord;
      swb_instr->destination.type = BIR::BIREnumerations::ctDPU;
      swb_instr->destination.portNo = e->dest_port;
      cidfg::SwbInstrVertex v_swb;
      v_swb.coord = v_dest->coord;
      v_swb.instr = swb_instr;
      int id_swb = g_.add_vertex(v_swb, parent_id, child_index);
      Edge e0(id_swb, 100, e->src_id, 100, "", Edge::DEPENDENCY, 1, INT_MAX);
      g_.add_edge(e0);
    }
    else if (v_src->vertex_type == Vertex::DPU_INSTR_VERTEX && v_dest->vertex_type == Vertex::REFI_INSTR_VERTEX)
    {
      int parent_id;
      int child_index;
      g_.get_parent(e->src_id, parent_id, child_index);
      BIR::SWBInstruction *swb_instr = CREATE_OBJECT_B(SWBInstruction);
      swb_instr->source.cellCoordinate = v_src->coord;
      swb_instr->source.type = BIR::BIREnumerations::ctDPU;
      swb_instr->source.portNo = e->src_port;
      swb_instr->destination.cellCoordinate = v_dest->coord;
      swb_instr->destination.type = BIR::BIREnumerations::ctRegFile;
      swb_instr->destination.portNo = e->dest_port;
      cidfg::SwbInstrVertex v_swb;
      v_swb.coord = v_dest->coord;
      v_swb.instr = swb_instr;
      int id_swb = g_.add_vertex(v_swb, parent_id, child_index);
      Edge e0(id_swb, 100, e->dest_id, 100, "", Edge::DEPENDENCY, 1, INT_MAX);
      g_.add_edge(e0);
    }
    else if (v_src->vertex_type == Vertex::DPU_INSTR_VERTEX && v_dest->vertex_type == Vertex::DPU_INSTR_VERTEX)
    {
      int parent_id;
      int child_index;
      g_.get_parent(e->src_id, parent_id, child_index);
      BIR::SWBInstruction *swb_instr = CREATE_OBJECT_B(SWBInstruction);
      swb_instr->source.cellCoordinate = v_src->coord;
      swb_instr->source.type = BIR::BIREnumerations::ctDPU;
      swb_instr->source.portNo = e->src_port;
      swb_instr->destination.cellCoordinate = v_dest->coord;
      swb_instr->destination.type = BIR::BIREnumerations::ctDPU;
      swb_instr->destination.portNo = e->dest_port;
      cidfg::SwbInstrVertex v_swb;
      v_swb.coord = v_dest->coord;
      v_swb.instr = swb_instr;
      int id_swb = g_.add_vertex(v_swb, parent_id, child_index);
      Edge e0(id_swb, 100, e->src_id, 100, "", Edge::DEPENDENCY, 1, INT_MAX);
      g_.add_edge(e0);
    }
  }
}
} // namespace codegen
} // namespace vesyla