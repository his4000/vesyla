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

#include "IndexVertexConverter.hpp"
using namespace BIR;
using namespace BIR::BIREnumerations;
namespace vesyla
{
namespace codegen
{
IndexVertexConverter::IndexVertexConverter()
{
}
IndexVertexConverter::~IndexVertexConverter()
{
}
void IndexVertexConverter::convert(cidfg::CidfgGraph &g_)
{
	assign_port(g_);
	//convert_swb(g_);
	//convert_refi(g_);

	std::set<int> skip_wri_vertices;
	std::set<int> removed_vertices;
	for (auto &v : g_.get_vertices())
	{
		if (v->vertex_type == cidfg::Vertex::READ_AND_INDEX_VERTEX)
		{
			cidfg::Vertex *v_src = v;
			cidfg::Vertex *v_dest = g_.get_vertex(g_.get_edge(g_.get_out_edge(v->id, 0))->dest_id);
			if (
					v_src->vertex_type == cidfg::Vertex::READ_AND_INDEX_VERTEX &&
					static_cast<cidfg::ReadAndIndexVertex *>(v_src)->is_sram &&
					v_dest->vertex_type == cidfg::Vertex::WRITE_AND_INDEX_VERTEX &&
					!static_cast<cidfg::WriteAndIndexVertex *>(v_dest)->is_sram)
			{
				convert_mrdi_wri(g_, v_src->id, v_dest->id, removed_vertices);
				skip_wri_vertices.insert(v_dest->id);
			}
			else if (
					v_src->vertex_type == cidfg::Vertex::READ_AND_INDEX_VERTEX &&
					!static_cast<cidfg::ReadAndIndexVertex *>(v_src)->is_sram &&
					v_dest->vertex_type == cidfg::Vertex::WRITE_AND_INDEX_VERTEX &&
					static_cast<cidfg::WriteAndIndexVertex *>(v_dest)->is_sram)
			{
				convert_rdi_mwri(g_, v_src->id, v_dest->id, removed_vertices);
				skip_wri_vertices.insert(v_dest->id);
			}
			else if (
					v_src->vertex_type == cidfg::Vertex::READ_AND_INDEX_VERTEX &&
					!static_cast<cidfg::ReadAndIndexVertex *>(v_src)->is_sram &&
					v_dest->vertex_type == cidfg::Vertex::WRITE_AND_INDEX_VERTEX &&
					!static_cast<cidfg::WriteAndIndexVertex *>(v_dest)->is_sram)
			{
				convert_rdi_wri(g_, v_src->id, v_dest->id, removed_vertices);
				skip_wri_vertices.insert(v_dest->id);
			}
			else if (
					v_src->vertex_type == cidfg::Vertex::READ_AND_INDEX_VERTEX &&
					!static_cast<cidfg::ReadAndIndexVertex *>(v_src)->is_sram &&
					v_dest->vertex_type != cidfg::Vertex::WRITE_AND_INDEX_VERTEX)
			{
				convert_rdi_other(g_, v_src->id, v_dest->id, removed_vertices);
			}
			else
			{
				LOG(FATAL) << "Wrong type of connection from RDI vertex!";
			}
		}
		else if (v->vertex_type == cidfg::Vertex::WRITE_AND_INDEX_VERTEX)
		{
			if (skip_wri_vertices.find(v->id) != skip_wri_vertices.end())
			{
				// Already processed when encounter its adjacent RDI node.
				continue;
			}

			cidfg::Vertex *v_dest = v;
			cidfg::Vertex *v_src = g_.get_vertex(g_.get_edge(g_.get_in_edge(v->id, 0))->src_id);
			if (
					v_src->vertex_type != cidfg::Vertex::READ_AND_INDEX_VERTEX &&
					v_dest->vertex_type == cidfg::Vertex::WRITE_AND_INDEX_VERTEX &&
					!static_cast<cidfg::WriteAndIndexVertex *>(v_dest)->is_sram)
			{
				convert_other_wri(g_, v_src->id, v_dest->id, removed_vertices);
			}
			else
			{
				; // Do nothing
			}
		}
	}

	for (auto &id : removed_vertices)
	{
		g_.del_vertex(id);
	}
}

void IndexVertexConverter::assign_port(cidfg::CidfgGraph &g_)
{
	// Build the UGraph
	std::set<cidfg::Vertex *> vertex_set;
	for (auto &v : g_.get_vertices())
	{
		if (v->vertex_type == cidfg::Vertex::READ_AND_INDEX_VERTEX ||
				v->vertex_type == cidfg::Vertex::WRITE_AND_INDEX_VERTEX ||
				v->vertex_type == cidfg::Vertex::DPU_INSTR_VERTEX)
		{
			vertex_set.insert(v);
		}
	}
	typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, int> UGraph;
	std::unordered_map<int, UGraph::vertex_descriptor> ugraph_map;
	UGraph ug1;
	for (auto &v : vertex_set)
	{
		UGraph::vertex_descriptor vd = add_vertex(v->id, ug1);
		ugraph_map[v->id] = vd;
	}
	for (auto &e : g_.get_edges())
	{
		if (vertex_set.find(g_.get_vertex(e->src_id)) != vertex_set.end() && vertex_set.find(g_.get_vertex(e->dest_id)) != vertex_set.end())
		{
			if (g_.get_vertex(e->src_id)->vertex_type == cidfg::Vertex::WRITE_AND_INDEX_VERTEX)
			{
				continue;
			}

			if (e->edge_type == cidfg::Edge::DEPENDENCY)
			{
				continue;
			}

			add_edge(ugraph_map[e->src_id], ugraph_map[e->dest_id], ug1);
			add_edge(ugraph_map[e->dest_id], ugraph_map[e->src_id], ug1);
		}
	}

	// Find connected components
	std::vector<int> component(boost::num_vertices(ug1));
	int num_components = boost::connected_components(ug1, &component[0]);

	std::unordered_map<string, int> counter_map;
	UGraph::vertex_iterator vi, vi_end;

	// First round check: assign all DiMArch required ports.
	for (tie(vi, vi_end) = vertices(ug1); vi != vi_end; vi++)
	{
		if (g_.get_vertex(ug1[*vi])->vertex_type == cidfg::Vertex::READ_AND_INDEX_VERTEX)
		{
			cidfg::ReadAndIndexVertex *vertex = static_cast<cidfg::ReadAndIndexVertex *>(g_.get_vertex(ug1[*vi]));
			if (!vertex->is_sram)
			{
				continue;
			}
			string key = vertex->coord.to_str() + "_AGU_R_" + to_string(component[*vi]);
			int port = 0;
			if (counter_map.find(key) != counter_map.end())
			{
				LOG(FATAL) << "Too much AGU Reading port for DiMArch required! (" + key + ")";
			}
			else
			{
				counter_map[key] = 1;
			}
			port += 2; // Because AGU reading port starts from 2.
			vertex->port = port;
		}
		else if (g_.get_vertex(ug1[*vi])->vertex_type == cidfg::Vertex::WRITE_AND_INDEX_VERTEX)
		{
			cidfg::ReadAndIndexVertex *vertex = static_cast<cidfg::ReadAndIndexVertex *>(g_.get_vertex(ug1[*vi]));
			if (!vertex->is_sram)
			{
				continue;
			}
			string key = vertex->coord.to_str() + "_AGU_W_" + to_string(component[*vi]);
			int port = 0;
			if (counter_map.find(key) != counter_map.end())
			{
				LOG(FATAL) << "Too much AGU Writing port for DiMArch required! (" + key + ")";
			}
			else
			{
				counter_map[key] = 1;
			}
			vertex->port = port;
		}
	}
	// Second round: Assign other ports.
	for (tie(vi, vi_end) = vertices(ug1); vi != vi_end; vi++)
	{
		if (g_.get_vertex(ug1[*vi])->vertex_type == cidfg::Vertex::READ_AND_INDEX_VERTEX)
		{
			cidfg::ReadAndIndexVertex *vertex = static_cast<cidfg::ReadAndIndexVertex *>(g_.get_vertex(ug1[*vi]));
			string key = vertex->coord.to_str() + "_AGU_R_" + to_string(component[*vi]);
			int port = 0;
			if (counter_map.find(key) != counter_map.end())
			{
				port = counter_map[key];
				counter_map[key]++;
				if (counter_map[key] > 2)
				{
					LOG(FATAL) << "Too much AGU Reading port required! (" + key + ")";
				}
			}
			else
			{
				counter_map[key] = 1;
			}
			port += 2; // Because AGU reading port starts from 2.
			vertex->port = port;
		}
		else if (g_.get_vertex(ug1[*vi])->vertex_type == cidfg::Vertex::WRITE_AND_INDEX_VERTEX)
		{
			cidfg::ReadAndIndexVertex *vertex = static_cast<cidfg::ReadAndIndexVertex *>(g_.get_vertex(ug1[*vi]));
			string key = vertex->coord.to_str() + "_AGU_W_" + to_string(component[*vi]);
			int port = 0;
			if (counter_map.find(key) != counter_map.end())
			{
				port = counter_map[key];
				counter_map[key]++;
				if (counter_map[key] > 2)
				{
					LOG(FATAL) << "Too much AGU Writing port required! (" + key + ")";
				}
			}
			else
			{
				counter_map[key] = 1;
			}
			vertex->port = port;
		}
	}
}

void IndexVertexConverter::convert_mrdi_wri(cidfg::CidfgGraph &g_, int src_id_, int dest_id_, std::set<int> &removed_vertices_)
{
	int parent_id;
	int child_index;
	g_.get_parent(dest_id_, parent_id, child_index);

	cidfg::ReadAndIndexVertex *v_src = static_cast<cidfg::ReadAndIndexVertex *>(g_.get_vertex(src_id_));
	cidfg::WriteAndIndexVertex *v_dest = static_cast<cidfg::WriteAndIndexVertex *>(g_.get_vertex(dest_id_));

	// Build the ROUTE instruction vertex
	// BIR::RouteInstruction *route_instr = CREATE_OBJECT_B(RouteInstruction);
	VIR::Coordinate src_coord = v_src->sram_coord;
	VIR::Coordinate dest_coord = v_dest->coord;
	// route_instr->drraSelect = dest_coord.row;
	src_coord.row += 1; // DiMarch row start from 1, because it consider DRRA row to be 0
	dest_coord.row = 0; // DRRA row is always 0
	// route_instr->sourceCoordinate = src_coord;
	// route_instr->destCoordinate = dest_coord;
	// route_instr->isFromSource = true;
	// cidfg::RouteInstrVertex v_route;
	// v_route.coord = v_dest->coord;
	// v_route.instr = route_instr;
	// int id_route = g_.add_vertex(v_route, parent_id, child_index);

	// Build the source SRAM_R instruction vertex
	BIR::SRAMInstruction *src_sr_instr = CREATE_OBJECT_B(SRAMInstruction);
	cidfg::Vertex *vv;
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_src->id, 1))->src_id);
	int loop1iter = 0;
	int loop2iter = 0;
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		src_sr_instr->initialAddress(static_cast<cidfg::ConstVertex *>(vv)->int_data, 1);
	}
	else if (vv->vertex_type == cidfg::Vertex::RACCU_INSTR_VERTEX)
	{
		string var_name = g_.get_edge(g_.get_in_edge(v_src->id, 1))->var_name;
		int reg_id;
		sscanf(var_name.c_str(), "RR_%d", &reg_id);
		src_sr_instr->initialAddress(reg_id, 0);
	}
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_src->id, 2))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		src_sr_instr->loop1Increment = static_cast<cidfg::ConstVertex *>(vv)->int_data;
	}
	else if (vv->vertex_type == cidfg::Vertex::RACCU_INSTR_VERTEX)
	{
		LOG(FATAL) << "L1 increment field in SRAM instruction can only be constant!";
	}
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_src->id, 3))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		src_sr_instr->loop1Iterations(static_cast<cidfg::ConstVertex *>(vv)->int_data - 1, 1); // -1 because the instruction requires (number_of_iteration-1)
		loop1iter = static_cast<cidfg::ConstVertex *>(vv)->int_data;
	}
	else
	{
		LOG(FATAL) << "L1 iteration field in SRAM instruction can only be constant!";
	}
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_src->id, 4))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		src_sr_instr->loop2Increment = static_cast<cidfg::ConstVertex *>(vv)->int_data; // Why let repetitionOffset to be int
	}
	else
	{
		LOG(FATAL) << "L2 increment field in SRAM instruction can only be constant!";
	}
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_src->id, 5))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		src_sr_instr->loop2Iterations(static_cast<cidfg::ConstVertex *>(vv)->int_data - 1, 1); // -1 because the instruction requires (number_of_iteration-1)
		loop2iter = static_cast<cidfg::ConstVertex *>(vv)->int_data;
	}
	else
	{
		LOG(FATAL) << "number of repeat field in REFI instruction can only be constatn!";
	}

	src_sr_instr->loop1Delay(0, true);
	src_sr_instr->loop2Delay(0, true);

	int num_hop = abs(dest_coord.row - src_coord.row) + abs(dest_coord.column - src_coord.column);
	int num_of_addr = loop1iter * loop2iter;
	int exe_cycle = (loop1iter + (loop1iter - 1) * src_sr_instr->loop1Delay.value) * loop2iter + (loop2iter - 1) * src_sr_instr->loop2Delay.value;

	src_sr_instr->isRead = true;
	src_sr_instr->executionCycle = exe_cycle;
	src_sr_instr->hopNumber = num_hop;

	cidfg::SramInstrVertex v_src_sr;
	v_src_sr.coord = v_src->coord;
	v_src_sr.sram_coord = v_src->sram_coord;
	v_src_sr.instr = src_sr_instr;
	int id_src_sr = g_.add_vertex(v_src_sr, parent_id, child_index);
	for (int i = 1; i < 6; i++)
	{
		cidfg::Edge *e_in = g_.get_edge(g_.get_in_edge(v_src->id, i));
		cidfg::Vertex *v_in = g_.get_vertex(e_in->src_id);
		if (v_in->vertex_type == cidfg::Vertex::CONST_VERTEX)
		{
			g_.del_edge(e_in->id);
			removed_vertices_.insert(v_in->id);
		}
		else
		{
			e_in->dest_id = id_src_sr;
		}
	}
	for (auto &e : g_.get_edges())
	{
		if (e->src_id == v_src->id)
		{
			e->src_id = id_src_sr;
			if (e->src_port == 0)
			{
				e->src_port = v_src->port;
			}
		}
		if (e->dest_id == v_src->id)
		{
			e->dest_id = id_src_sr;
		}
	}

	// Build destination REFI instruction vertex
	BIR::RefiInstruction *dest_refi_instr = CREATE_OBJECT_B(RefiInstruction);
	dest_refi_instr->en_compression = v_src->en_compression;
	dest_refi_instr->reverse_bits(0, true);
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_dest->id, 2))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		dest_refi_instr->startAddress(static_cast<cidfg::ConstVertex *>(vv)->int_data, 1);
	}
	else if (vv->vertex_type == cidfg::Vertex::RACCU_INSTR_VERTEX)
	{
		string var_name = g_.get_edge(g_.get_in_edge(v_dest->id, 2))->var_name;
		int reg_id;
		sscanf(var_name.c_str(), "RR_%d", &reg_id);
		dest_refi_instr->startAddress(reg_id, 0);
	}
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_dest->id, 3))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		dest_refi_instr->stepValue(static_cast<cidfg::ConstVertex *>(vv)->int_data, 1);
	}
	else if (vv->vertex_type == cidfg::Vertex::RACCU_INSTR_VERTEX)
	{
		string var_name = g_.get_edge(g_.get_in_edge(v_dest->id, 3))->var_name;
		int reg_id;
		sscanf(var_name.c_str(), "RR_%d", &reg_id);
		dest_refi_instr->stepValue(reg_id, 0);
	}
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_dest->id, 4))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		dest_refi_instr->numberOfAddress(static_cast<cidfg::ConstVertex *>(vv)->int_data - 1, 1); // -1 because the instruction requires (number_of_iteration-1)
	}
	else
	{
		LOG(FATAL) << "number of address field in REFI instruction can only be constatn!";
	}
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_dest->id, 5))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		dest_refi_instr->repetitionOffset = static_cast<cidfg::ConstVertex *>(vv)->int_data; // Why let repetitionOffset to be int
	}
	else
	{
		LOG(FATAL) << "repetition offset field in REFI instruction can only be constatn!";
	}
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_dest->id, 6))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		dest_refi_instr->numberOfRepetition(static_cast<cidfg::ConstVertex *>(vv)->int_data - 1, 1); // -1 because the instruction requires (number_of_iteration-1)
	}
	else
	{
		LOG(FATAL) << "number of repeat field in REFI instruction can only be constatn!";
	}

	dest_refi_instr->portNo = static_cast<cidfg::WriteAndIndexVertex *>(v_dest)->port;
	dest_refi_instr->portMode = BIR::BIREnumerations::pmIn;
	dest_refi_instr->isDimarchMode = true;

	cidfg::RefiInstrVertex v_dest_refi;
	v_dest_refi.coord = v_dest->coord;
	v_dest_refi.instr = dest_refi_instr;
	int id_dest_refi = g_.add_vertex(v_dest_refi, parent_id, child_index);

	for (int i = 2; i < 7; i++)
	{
		cidfg::Edge *e_in = g_.get_edge(g_.get_in_edge(v_dest->id, i));
		cidfg::Vertex *v_in = g_.get_vertex(e_in->src_id);
		if (v_in->vertex_type == cidfg::Vertex::CONST_VERTEX)
		{
			g_.del_edge(e_in->id);
			removed_vertices_.insert(v_in->id);
		}
		else
		{
			e_in->dest_id = id_dest_refi;
		}
	}
	for (auto &e : g_.get_edges())
	{
		if (e->src_id == v_dest->id)
		{
			e->src_id = id_dest_refi;
		}
		if (e->dest_id == v_dest->id)
		{
			e->dest_id = id_dest_refi;
			if (e->dest_port == 0)
			{
				e->dest_port = v_dest->port;
			}
		}
	}

	// Connect ROUTE to source SRAM_R instruction
	// cidfg::Edge e(id_route, 100, id_src_sr, 100, "", cidfg::Edge::DEPENDENCY, 1, INT_MAX);
	// g_.add_edge(e);

	// Add all old vertices to removed list
	removed_vertices_.insert(src_id_);
	removed_vertices_.insert(dest_id_);
}

void IndexVertexConverter::convert_rdi_mwri(cidfg::CidfgGraph &g_, int src_id_, int dest_id_, std::set<int> &removed_vertices_)
{
	int parent_id;
	int child_index;
	g_.get_parent(dest_id_, parent_id, child_index);

	cidfg::ReadAndIndexVertex *v_src = static_cast<cidfg::ReadAndIndexVertex *>(g_.get_vertex(src_id_));
	cidfg::WriteAndIndexVertex *v_dest = static_cast<cidfg::WriteAndIndexVertex *>(g_.get_vertex(dest_id_));

	// Build the ROUTE instruction vertex
	// BIR::RouteInstruction *route_instr = CREATE_OBJECT_B(RouteInstruction);
	VIR::Coordinate src_coord = v_src->coord;
	VIR::Coordinate dest_coord = v_dest->sram_coord;
	// route_instr->drraSelect = src_coord.row;
	dest_coord.row += 1; // DiMarch row start from 1, because it consider DRRA row to be 0
	src_coord.row = 0;	 // DRRA row is always 0
	// route_instr->sourceCoordinate = src_coord;
	// route_instr->destCoordinate = dest_coord;
	// route_instr->isFromSource = false;
	// cidfg::RouteInstrVertex v_route;
	// v_route.coord = v_dest->coord;
	// v_route.instr = route_instr;
	// int id_route = g_.add_vertex(v_route, parent_id, child_index);

	// Build the source REFI instruction vertex
	BIR::RefiInstruction *src_refi_instr = CREATE_OBJECT_B(RefiInstruction);
	src_refi_instr->en_compression = v_dest->en_compression;
	src_refi_instr->reverse_bits(0, true);
	cidfg::Vertex *vv;
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_src->id, 1))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		src_refi_instr->startAddress(static_cast<cidfg::ConstVertex *>(vv)->int_data, 1);
	}
	else if (vv->vertex_type == cidfg::Vertex::RACCU_INSTR_VERTEX)
	{
		string var_name = g_.get_edge(g_.get_in_edge(v_src->id, 1))->var_name;
		int reg_id;
		sscanf(var_name.c_str(), "RR_%d", &reg_id);
		src_refi_instr->startAddress(reg_id, 0);
	}
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_src->id, 2))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		src_refi_instr->stepValue(static_cast<cidfg::ConstVertex *>(vv)->int_data, 1);
	}
	else if (vv->vertex_type == cidfg::Vertex::RACCU_INSTR_VERTEX)
	{
		string var_name = g_.get_edge(g_.get_in_edge(v_src->id, 2))->var_name;
		int reg_id;
		sscanf(var_name.c_str(), "RR_%d", &reg_id);
		src_refi_instr->stepValue(reg_id, 0);
	}
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_src->id, 3))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		src_refi_instr->numberOfAddress(static_cast<cidfg::ConstVertex *>(vv)->int_data - 1, 1); // -1 because the instruction requires (number_of_iteration-1)
	}
	else
	{
		LOG(FATAL) << "number of address field in REFI instruction can only be constant!";
	}
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_src->id, 4))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		src_refi_instr->repetitionOffset = static_cast<cidfg::ConstVertex *>(vv)->int_data; // Why let repetitionOffset to be int
	}
	else
	{
		LOG(FATAL) << "repetition offset field in REFI instruction can only be constant!";
	}
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_src->id, 5))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		src_refi_instr->numberOfRepetition(static_cast<cidfg::ConstVertex *>(vv)->int_data - 1, 1); // -1 because the instruction requires (number_of_iteration-1)
	}
	else
	{
		LOG(FATAL) << "number of repeat field in REFI instruction can only be constatn!";
	}

	src_refi_instr->portNo = static_cast<cidfg::ReadAndIndexVertex *>(v_src)->port;
	src_refi_instr->portMode = BIR::BIREnumerations::pmOut;
	src_refi_instr->isDimarchMode = true;

	cidfg::RefiInstrVertex v_src_refi;
	v_src_refi.coord = v_src->coord;
	v_src_refi.instr = src_refi_instr;
	int id_src_refi = g_.add_vertex(v_src_refi, parent_id, child_index);
	for (int i = 1; i < 6; i++)
	{
		cidfg::Edge *e_in = g_.get_edge(g_.get_in_edge(v_src->id, i));
		cidfg::Vertex *v_in = g_.get_vertex(e_in->src_id);
		if (v_in->vertex_type == cidfg::Vertex::CONST_VERTEX)
		{
			g_.del_edge(e_in->id);
			removed_vertices_.insert(v_in->id);
		}
		else
		{
			e_in->dest_id = id_src_refi;
		}
	}
	for (auto &e : g_.get_edges())
	{
		if (e->src_id == v_src->id)
		{
			e->src_id = id_src_refi;
			if (e->src_port == 0)
			{
				e->src_port = v_src->port;
			}
		}
		if (e->dest_id == v_src->id)
		{
			e->dest_id = id_src_refi;
		}
	}

	// Build the destination SRAM_W instruction vertex
	BIR::SRAMInstruction *dest_sw_instr = CREATE_OBJECT_B(SRAMInstruction);
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_dest->id, 2))->src_id);
	int loop1iter = 0;
	int loop2iter = 0;
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		dest_sw_instr->initialAddress(static_cast<cidfg::ConstVertex *>(vv)->int_data, 1);
	}
	else if (vv->vertex_type == cidfg::Vertex::RACCU_INSTR_VERTEX)
	{
		string var_name = g_.get_edge(g_.get_in_edge(v_dest->id, 2))->var_name;
		int reg_id;
		sscanf(var_name.c_str(), "RR_%d", &reg_id);
		dest_sw_instr->initialAddress(reg_id, 0);
	}
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_dest->id, 3))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		dest_sw_instr->loop1Increment = static_cast<cidfg::ConstVertex *>(vv)->int_data;
	}
	else if (vv->vertex_type == cidfg::Vertex::RACCU_INSTR_VERTEX)
	{
		LOG(FATAL) << "L1 increment field in SRAM instruction can only be constant!";
	}
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_dest->id, 4))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		dest_sw_instr->loop1Iterations(static_cast<cidfg::ConstVertex *>(vv)->int_data - 1, 1); // -1 because the instruction requires (number_of_iteration-1)
		loop1iter = static_cast<cidfg::ConstVertex *>(vv)->int_data;
	}
	else
	{
		LOG(FATAL) << "L1 iteration field in SRAM instruction can only be constant!";
	}
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_dest->id, 5))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		dest_sw_instr->loop2Increment = static_cast<cidfg::ConstVertex *>(vv)->int_data; // Why let repetitionOffset to be int
	}
	else
	{
		LOG(FATAL) << "L2 increment field in SRAM instruction can only be constant!";
	}
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_dest->id, 6))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		dest_sw_instr->loop2Iterations(static_cast<cidfg::ConstVertex *>(vv)->int_data - 1, 1); // -1 because the instruction requires (number_of_iteration-1)
		loop2iter = static_cast<cidfg::ConstVertex *>(vv)->int_data;
	}
	else
	{
		LOG(FATAL) << "number of repeat field in REFI instruction can only be constatn!";
	}

	dest_sw_instr->loop1Delay(0, true);
	dest_sw_instr->loop2Delay(0, true);

	int num_hop = abs(dest_coord.row - src_coord.row) + abs(dest_coord.column - src_coord.column);
	int num_of_addr = loop1iter * loop2iter;
	int exe_cycle = (loop1iter + (loop1iter - 1) * dest_sw_instr->loop1Delay.value) * loop2iter + (loop2iter - 1) * dest_sw_instr->loop2Delay.value;

	dest_sw_instr->isRead = false;
	dest_sw_instr->executionCycle = exe_cycle;
	dest_sw_instr->hopNumber = num_hop;

	cidfg::SramInstrVertex v_dest_sw;
	v_dest_sw.coord = v_dest->coord;
	v_dest_sw.sram_coord = v_dest->sram_coord;
	v_dest_sw.instr = dest_sw_instr;
	int id_dest_sw = g_.add_vertex(v_dest_sw, parent_id, child_index);
	for (int i = 1; i < 6; i++)
	{
		cidfg::Edge *e_in = g_.get_edge(g_.get_in_edge(v_dest->id, i));
		cidfg::Vertex *v_in = g_.get_vertex(e_in->dest_id);
		if (v_in->vertex_type == cidfg::Vertex::CONST_VERTEX)
		{
			g_.del_edge(e_in->id);
			removed_vertices_.insert(v_in->id);
		}
		else
		{
			e_in->dest_id = id_dest_sw;
		}
	}
	for (auto &e : g_.get_edges())
	{
		if (e->src_id == v_src->id)
		{
			e->src_id = id_dest_sw;
			if (e->src_port == 0)
			{
				e->src_port = v_dest->port;
			}
		}
		if (e->dest_id == v_dest->id)
		{
			e->dest_id = id_dest_sw;
		}
	}

	// Connect ROUTE to source SRAM_R instruction
	// cidfg::Edge e(id_route, 100, id_dest_sw, 100, "", cidfg::Edge::DEPENDENCY, 1, INT_MAX);
	// g_.add_edge(e);

	// Add all old vertices to removed list
	removed_vertices_.insert(src_id_);
	removed_vertices_.insert(dest_id_);
}

void IndexVertexConverter::convert_rdi_wri(cidfg::CidfgGraph &g_, int src_id_, int dest_id_, std::set<int> &removed_vertices_)
{
	int parent_id;
	int child_index;
	g_.get_parent(dest_id_, parent_id, child_index);

	cidfg::ReadAndIndexVertex *v_src = static_cast<cidfg::ReadAndIndexVertex *>(g_.get_vertex(src_id_));
	cidfg::WriteAndIndexVertex *v_dest = static_cast<cidfg::WriteAndIndexVertex *>(g_.get_vertex(dest_id_));

	// Build the SWB instruction vertex
	// BIR::SWBInstruction *swb_instr = CREATE_OBJECT_B(SWBInstruction);
	// swb_instr->source.cellCoordinate = v_src->coord;
	// swb_instr->source.type = BIR::BIREnumerations::ctRegFile;
	// swb_instr->source.portNo = v_src->port;
	// swb_instr->destination.cellCoordinate = v_dest->coord;
	// swb_instr->destination.type = BIR::BIREnumerations::ctRegFile;
	// swb_instr->destination.portNo = v_dest->port;
	// cidfg::SwbInstrVertex v_swb;
	// v_swb.coord = v_dest->coord;
	// v_swb.instr = swb_instr;
	// int id_swb = g_.add_vertex(v_swb, parent_id, child_index);

	// Build the source REFI instruction vertex
	BIR::RefiInstruction *src_refi_instr = CREATE_OBJECT_B(RefiInstruction);
	src_refi_instr->reverse_bits(0, true);
	cidfg::Vertex *vv;
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_src->id, 1))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		src_refi_instr->startAddress(static_cast<cidfg::ConstVertex *>(vv)->int_data, 1);
	}
	else if (vv->vertex_type == cidfg::Vertex::RACCU_INSTR_VERTEX)
	{
		string var_name = g_.get_edge(g_.get_in_edge(v_src->id, 1))->var_name;
		int reg_id;
		sscanf(var_name.c_str(), "RR_%d", &reg_id);
		src_refi_instr->startAddress(reg_id, 0);
	}
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_src->id, 2))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		src_refi_instr->stepValue(static_cast<cidfg::ConstVertex *>(vv)->int_data, 1);
	}
	else if (vv->vertex_type == cidfg::Vertex::RACCU_INSTR_VERTEX)
	{
		string var_name = g_.get_edge(g_.get_in_edge(v_src->id, 2))->var_name;
		int reg_id;
		sscanf(var_name.c_str(), "RR_%d", &reg_id);
		src_refi_instr->stepValue(reg_id, 0);
	}
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_src->id, 3))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		src_refi_instr->numberOfAddress(static_cast<cidfg::ConstVertex *>(vv)->int_data - 1, 1); // -1 because the instruction requires (number_of_iteration-1)
	}
	else
	{
		LOG(FATAL) << "number of address field in REFI instruction can only be constant!";
	}
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_src->id, 4))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		src_refi_instr->repetitionOffset = static_cast<cidfg::ConstVertex *>(vv)->int_data; // Why let repetitionOffset to be int
	}
	else
	{
		LOG(FATAL) << "repetition offset field in REFI instruction can only be constant!";
	}
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_src->id, 5))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		src_refi_instr->numberOfRepetition(static_cast<cidfg::ConstVertex *>(vv)->int_data - 1, 1); // -1 because the instruction requires (number_of_iteration-1)
	}
	else
	{
		LOG(FATAL) << "number of repeat field in REFI instruction can only be constatn!";
	}

	src_refi_instr->portNo = static_cast<cidfg::ReadAndIndexVertex *>(v_src)->port;
	src_refi_instr->portMode = BIR::BIREnumerations::pmOut;
	src_refi_instr->isDimarchMode = false;

	cidfg::RefiInstrVertex v_src_refi;
	v_src_refi.coord = v_src->coord;
	v_src_refi.instr = src_refi_instr;
	int id_src_refi = g_.add_vertex(v_src_refi, parent_id, child_index);
	for (int i = 1; i < 6; i++)
	{
		cidfg::Edge *e_in = g_.get_edge(g_.get_in_edge(v_src->id, i));
		cidfg::Vertex *v_in = g_.get_vertex(e_in->src_id);
		if (v_in->vertex_type == cidfg::Vertex::CONST_VERTEX)
		{
			g_.del_edge(e_in->id);
			removed_vertices_.insert(v_in->id);
		}
		else
		{
			e_in->dest_id = id_src_refi;
		}
	}
	for (auto &e : g_.get_edges())
	{
		if (e->src_id == v_src->id)
		{
			e->src_id = id_src_refi;
			if (e->src_port == 0)
			{
				e->src_port = v_src->port;
			}
		}
		if (e->dest_id == v_src->id)
		{
			e->dest_id = id_src_refi;
		}
	}

	// Build destination REFI instruction vertex
	BIR::RefiInstruction *dest_refi_instr = CREATE_OBJECT_B(RefiInstruction);
	dest_refi_instr->reverse_bits(0, true);
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_dest->id, 2))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		dest_refi_instr->startAddress(static_cast<cidfg::ConstVertex *>(vv)->int_data, 1);
	}
	else if (vv->vertex_type == cidfg::Vertex::RACCU_INSTR_VERTEX)
	{
		string var_name = g_.get_edge(g_.get_in_edge(v_dest->id, 2))->var_name;
		int reg_id;
		sscanf(var_name.c_str(), "RR_%d", &reg_id);
		dest_refi_instr->startAddress(reg_id, 0);
	}
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_dest->id, 3))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		dest_refi_instr->stepValue(static_cast<cidfg::ConstVertex *>(vv)->int_data, 1);
	}
	else if (vv->vertex_type == cidfg::Vertex::RACCU_INSTR_VERTEX)
	{
		string var_name = g_.get_edge(g_.get_in_edge(v_dest->id, 3))->var_name;
		int reg_id;
		sscanf(var_name.c_str(), "RR_%d", &reg_id);
		dest_refi_instr->stepValue(reg_id, 0);
	}
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_dest->id, 4))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		dest_refi_instr->numberOfAddress(static_cast<cidfg::ConstVertex *>(vv)->int_data - 1, 1); // -1 because the instruction requires (number_of_iteration-1)
	}
	else
	{
		LOG(FATAL) << "number of address field in REFI instruction can only be constatn!";
	}
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_dest->id, 5))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		dest_refi_instr->repetitionOffset = static_cast<cidfg::ConstVertex *>(vv)->int_data; // Why let repetitionOffset to be int
	}
	else
	{
		LOG(FATAL) << "repetition offset field in REFI instruction can only be constatn!";
	}
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_dest->id, 6))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		dest_refi_instr->numberOfRepetition(static_cast<cidfg::ConstVertex *>(vv)->int_data - 1, 1); // -1 because the instruction requires (number_of_iteration-1)
	}
	else
	{
		LOG(FATAL) << "number of repeat field in REFI instruction can only be constatn!";
	}

	dest_refi_instr->portNo = static_cast<cidfg::WriteAndIndexVertex *>(v_dest)->port;
	dest_refi_instr->portMode = BIR::BIREnumerations::pmIn;

	cidfg::RefiInstrVertex v_dest_refi;
	v_dest_refi.coord = v_dest->coord;
	v_dest_refi.instr = dest_refi_instr;
	int id_dest_refi = g_.add_vertex(v_dest_refi, parent_id, child_index);

	for (int i = 2; i < 7; i++)
	{
		cidfg::Edge *e_in = g_.get_edge(g_.get_in_edge(v_dest->id, i));
		cidfg::Vertex *v_in = g_.get_vertex(e_in->src_id);
		if (v_in->vertex_type == cidfg::Vertex::CONST_VERTEX)
		{
			g_.del_edge(e_in->id);
			removed_vertices_.insert(v_in->id);
		}
		else
		{
			e_in->dest_id = id_dest_refi;
		}
	}
	for (auto &e : g_.get_edges())
	{
		if (e->src_id == v_dest->id)
		{
			e->src_id = id_dest_refi;
		}
		if (e->dest_id == v_dest->id)
		{
			e->dest_id = id_dest_refi;
			if (e->dest_port == 0)
			{
				e->dest_port = v_dest->port;
			}
		}
	}

	// Connect SWB to source REFI instruction
	// cidfg::Edge e(id_swb, 100, id_src_refi, 100, "", cidfg::Edge::DEPENDENCY, 1, INT_MAX);
	// g_.add_edge(e);

	// Add all old vertices to removed list
	removed_vertices_.insert(src_id_);
	removed_vertices_.insert(dest_id_);
}

void IndexVertexConverter::convert_rdi_other(cidfg::CidfgGraph &g_, int src_id_, int dest_id_, std::set<int> &removed_vertices_)
{
	// Only DpuInstrVertex is allowed to connect to ReadAndIndexVertex other than WriteAndIndexVertex.
	// So we need to check the destination vertex type, it should be DpuInstrVertex.
	CHECK_EQ(g_.get_vertex(dest_id_)->vertex_type, cidfg::Vertex::DPU_INSTR_VERTEX);

	int parent_id;
	int child_index;
	g_.get_parent(dest_id_, parent_id, child_index);

	cidfg::ReadAndIndexVertex *v_src = static_cast<cidfg::ReadAndIndexVertex *>(g_.get_vertex(src_id_));
	cidfg::DpuInstrVertex *v_dest = static_cast<cidfg::DpuInstrVertex *>(g_.get_vertex(dest_id_));

	// Build the SWB instruction vertex
	// BIR::SWBInstruction *swb_instr = CREATE_OBJECT_B(SWBInstruction);
	// swb_instr->source.cellCoordinate = v_src->coord;
	// swb_instr->source.type = BIR::BIREnumerations::ctRegFile;
	// swb_instr->source.portNo = v_src->port;
	// swb_instr->destination.cellCoordinate = v_dest->coord;
	// swb_instr->destination.type = BIR::BIREnumerations::ctDPU;
	// swb_instr->destination.portNo = g_.get_edge(g_.get_out_edge(src_id_, 0))->dest_port;
	// cidfg::SwbInstrVertex v_swb;
	// v_swb.coord = v_dest->coord;
	// v_swb.instr = swb_instr;
	// int id_swb = g_.add_vertex(v_swb, parent_id, child_index);

	// Build the source REFI instruction vertex
	BIR::RefiInstruction *src_refi_instr = CREATE_OBJECT_B(RefiInstruction);
	src_refi_instr->reverse_bits(0, true);
	cidfg::Vertex *vv;
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_src->id, 1))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		src_refi_instr->startAddress(static_cast<cidfg::ConstVertex *>(vv)->int_data, 1);
	}
	else if (vv->vertex_type == cidfg::Vertex::RACCU_INSTR_VERTEX)
	{
		string var_name = g_.get_edge(g_.get_in_edge(v_src->id, 1))->var_name;
		int reg_id;
		sscanf(var_name.c_str(), "RR_%d", &reg_id);
		src_refi_instr->startAddress(reg_id, 0);
	}
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_src->id, 2))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		src_refi_instr->stepValue(static_cast<cidfg::ConstVertex *>(vv)->int_data, 1);
	}
	else if (vv->vertex_type == cidfg::Vertex::RACCU_INSTR_VERTEX)
	{
		string var_name = g_.get_edge(g_.get_in_edge(v_src->id, 2))->var_name;
		int reg_id;
		sscanf(var_name.c_str(), "RR_%d", &reg_id);
		src_refi_instr->stepValue(reg_id, 0);
	}
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_src->id, 3))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		src_refi_instr->numberOfAddress(static_cast<cidfg::ConstVertex *>(vv)->int_data - 1, 1); // -1 because the instruction requires (number_of_iteration-1)
	}
	else
	{
		LOG(FATAL) << "number of address field in REFI instruction can only be constant!";
	}
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_src->id, 4))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		src_refi_instr->repetitionOffset = static_cast<cidfg::ConstVertex *>(vv)->int_data; // Why let repetitionOffset to be int
	}
	else
	{
		LOG(FATAL) << "repetition offset field in REFI instruction can only be constant!";
	}
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_src->id, 5))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		src_refi_instr->numberOfRepetition(static_cast<cidfg::ConstVertex *>(vv)->int_data - 1, 1); // -1 because the instruction requires (number_of_iteration-1)
	}
	else
	{
		LOG(FATAL) << "number of repeat field in REFI instruction can only be constatn!";
	}

	src_refi_instr->portNo = static_cast<cidfg::ReadAndIndexVertex *>(v_src)->port;
	src_refi_instr->portMode = BIR::BIREnumerations::pmOut;
	src_refi_instr->isDimarchMode = false;

	cidfg::RefiInstrVertex v_src_refi;
	v_src_refi.coord = v_src->coord;
	v_src_refi.instr = src_refi_instr;
	int id_src_refi = g_.add_vertex(v_src_refi, parent_id, child_index);
	for (int i = 1; i < 6; i++)
	{
		cidfg::Edge *e_in = g_.get_edge(g_.get_in_edge(v_src->id, i));
		cidfg::Vertex *v_in = g_.get_vertex(e_in->src_id);
		if (v_in->vertex_type == cidfg::Vertex::CONST_VERTEX)
		{
			g_.del_edge(e_in->id);
			removed_vertices_.insert(v_in->id);
		}
		else
		{
			e_in->dest_id = id_src_refi;
		}
	}
	for (auto &e : g_.get_edges())
	{
		if (e->src_id == v_src->id)
		{
			e->src_id = id_src_refi;
			if (e->src_port == 0)
			{
				e->src_port = v_src->port;
			}
		}
		if (e->dest_id == v_src->id)
		{
			e->dest_id = id_src_refi;
		}
	}

	// No need to build destination vertex because it's not indexing node and already been
	// processed by other module.

	// Connect SWB to source REFI instruction
	// cidfg::Edge e(id_swb, 100, id_src_refi, 100, "", cidfg::Edge::DEPENDENCY, 1, INT_MAX);
	// g_.add_edge(e);

	// Add all old vertices to removed list
	removed_vertices_.insert(src_id_);
}

void IndexVertexConverter::convert_other_wri(cidfg::CidfgGraph &g_, int src_id_, int dest_id_, std::set<int> &removed_vertices_)
{
	// Only DpuInstrVertex is allowed to connect to WriteAndIndexVertex other than ReadAndIndexVertex.
	// So we need to check the destination vertex type, it should be DpuInstrVertex.
	CHECK_EQ(g_.get_vertex(src_id_)->vertex_type, cidfg::Vertex::DPU_INSTR_VERTEX);

	int parent_id;
	int child_index;
	g_.get_parent(dest_id_, parent_id, child_index);

	cidfg::DpuInstrVertex *v_src = static_cast<cidfg::DpuInstrVertex *>(g_.get_vertex(src_id_));
	cidfg::WriteAndIndexVertex *v_dest = static_cast<cidfg::WriteAndIndexVertex *>(g_.get_vertex(dest_id_));

	// Build the SWB instruction vertex
	// BIR::SWBInstruction *swb_instr = CREATE_OBJECT_B(SWBInstruction);
	// swb_instr->source.cellCoordinate = v_src->coord;
	// swb_instr->source.type = BIR::BIREnumerations::ctDPU;
	// swb_instr->source.portNo = g_.get_edge(g_.get_in_edge(v_dest->id, 0))->src_port;
	// swb_instr->destination.cellCoordinate = v_dest->coord;
	// swb_instr->destination.type = BIR::BIREnumerations::ctRegFile;
	// swb_instr->destination.portNo = v_dest->port;
	// cidfg::SwbInstrVertex v_swb;
	// v_swb.coord = v_dest->coord;
	// v_swb.instr = swb_instr;
	// int id_swb = g_.add_vertex(v_swb, parent_id, child_index);

	// No need to build the source REFI instruction vertex. It's not an indexing vertex and has been taken
	// care of by other module.

	// Build destination REFI instruction vertex
	BIR::RefiInstruction *dest_refi_instr = CREATE_OBJECT_B(RefiInstruction);
	dest_refi_instr->reverse_bits(0, true);
	cidfg::Vertex *vv;
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_dest->id, 2))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		dest_refi_instr->startAddress(static_cast<cidfg::ConstVertex *>(vv)->int_data, 1);
	}
	else if (vv->vertex_type == cidfg::Vertex::RACCU_INSTR_VERTEX)
	{
		string var_name = g_.get_edge(g_.get_in_edge(v_dest->id, 2))->var_name;
		int reg_id;
		sscanf(var_name.c_str(), "RR_%d", &reg_id);
		dest_refi_instr->startAddress(reg_id, 0);
	}
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_dest->id, 3))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		dest_refi_instr->stepValue(static_cast<cidfg::ConstVertex *>(vv)->int_data, 1);
	}
	else if (vv->vertex_type == cidfg::Vertex::RACCU_INSTR_VERTEX)
	{
		string var_name = g_.get_edge(g_.get_in_edge(v_dest->id, 3))->var_name;
		int reg_id;
		sscanf(var_name.c_str(), "RR_%d", &reg_id);
		dest_refi_instr->stepValue(reg_id, 0);
	}
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_dest->id, 4))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		dest_refi_instr->numberOfAddress(static_cast<cidfg::ConstVertex *>(vv)->int_data - 1, 1); // -1 because the instruction requires (number_of_iteration-1)
	}
	else
	{
		LOG(FATAL) << "number of address field in REFI instruction can only be constatn!";
	}
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_dest->id, 5))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		dest_refi_instr->repetitionOffset = static_cast<cidfg::ConstVertex *>(vv)->int_data; // Why let repetitionOffset to be int
	}
	else
	{
		LOG(FATAL) << "repetition offset field in REFI instruction can only be constatn!";
	}
	vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v_dest->id, 6))->src_id);
	if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
	{
		dest_refi_instr->numberOfRepetition(static_cast<cidfg::ConstVertex *>(vv)->int_data - 1, 1); // -1 because the instruction requires (number_of_iteration-1)
	}
	else
	{
		LOG(FATAL) << "number of repeat field in REFI instruction can only be constatn!";
	}

	dest_refi_instr->portNo = static_cast<cidfg::WriteAndIndexVertex *>(v_dest)->port;
	dest_refi_instr->portMode = BIR::BIREnumerations::pmIn;
	dest_refi_instr->isDimarchMode = false;

	cidfg::RefiInstrVertex v_dest_refi;
	v_dest_refi.coord = v_dest->coord;
	v_dest_refi.instr = dest_refi_instr;
	int id_dest_refi = g_.add_vertex(v_dest_refi, parent_id, child_index);

	for (int i = 2; i < 7; i++)
	{
		cidfg::Edge *e_in = g_.get_edge(g_.get_in_edge(v_dest->id, i));
		cidfg::Vertex *v_in = g_.get_vertex(e_in->src_id);
		if (v_in->vertex_type == cidfg::Vertex::CONST_VERTEX)
		{
			g_.del_edge(e_in->id);
			removed_vertices_.insert(v_in->id);
		}
		else
		{
			e_in->dest_id = id_dest_refi;
		}
	}
	for (auto &e : g_.get_edges())
	{
		if (e->src_id == v_dest->id)
		{
			e->src_id = id_dest_refi;
		}
		if (e->dest_id == v_dest->id)
		{
			e->dest_id = id_dest_refi;
			if (e->dest_port == 0)
			{
				e->dest_port = v_dest->port;
			}
		}
	}

	// Connect SWB to destination REFI instruction
	// cidfg::Edge e(id_swb, 100, id_dest_refi, 100, "", cidfg::Edge::DEPENDENCY, 1, INT_MAX);
	// g_.add_edge(e);

	// Add all old vertices to removed list
	removed_vertices_.insert(dest_id_);
}

void IndexVertexConverter::convert_swb(cidfg::CidfgGraph &g_)
{
	std::set<int> loop_vertices;
	for (auto &v : g_.get_vertices())
	{
		if (v->vertex_type == cidfg::Vertex::READ_AND_INDEX_VERTEX)
		{
			int parent_id;
			int child_index;
			g_.get_parent(v->id, parent_id, child_index);
			cidfg::Vertex *vin0 = g_.get_vertex(g_.get_edge(g_.get_in_edge(v->id, 1))->src_id);

			if (static_cast<cidfg::ReadAndIndexVertex *>(v)->is_sram)
			{
				// Here add a ROUTE instruction
				BIR::RouteInstruction *instr = CREATE_OBJECT_B(RouteInstruction);
				cidfg::Edge *e0 = g_.get_edge(g_.get_out_edge(v->id, 0));
				cidfg::Vertex *src_v = v;
				cidfg::Vertex *dest_v = g_.get_vertex(e0->dest_id);
				instr->sourceCoordinate = src_v->coord;
				instr->destCoordinate = dest_v->coord;
				instr->isFromSource = true;
				instr->drraSelect = dest_v->coord.row;

				cidfg::RouteInstrVertex sv;
				sv.coord = dest_v->coord;
				sv.instr = instr;
				int id_sv = g_.add_vertex(sv, parent_id, child_index);
				cidfg::Edge e(id_sv, 100, v->id, 100, "", cidfg::Edge::DEPENDENCY, 1, INT_MAX);
				g_.add_edge(e);
			}
			else
			{
				// Here add a SWB instruction
				BIR::SWBInstruction *instr = CREATE_OBJECT_B(SWBInstruction);
				cidfg::Edge *e0 = g_.get_edge(g_.get_out_edge(v->id, 0));
				cidfg::Vertex *src_v = v;
				cidfg::Vertex *dest_v = g_.get_vertex(e0->dest_id);
				instr->source.cellCoordinate = src_v->coord;
				instr->source.type = BIR::BIREnumerations::ctRegFile;
				instr->source.portNo = static_cast<cidfg::ReadAndIndexVertex *>(src_v)->port;
				instr->destination.cellCoordinate = dest_v->coord;
				if (dest_v->vertex_type == cidfg::Vertex::DPU_INSTR_VERTEX)
				{
					instr->destination.type = BIR::BIREnumerations::ctDPU;
					instr->destination.portNo = e0->dest_port;
				}
				else if (dest_v->vertex_type == cidfg::Vertex::WRITE_AND_INDEX_VERTEX)
				{
					instr->destination.type = BIR::BIREnumerations::ctRegFile;
					instr->destination.portNo = static_cast<cidfg::WriteAndIndexVertex *>(dest_v)->port;
				}
				else
				{
					LOG(FATAL) << "Wrong type of vertex followring ReadAndIndex vertex!";
				}

				cidfg::SwbInstrVertex sv;
				sv.coord = dest_v->coord;
				sv.instr = instr;
				int id_sv = g_.add_vertex(sv, parent_id, child_index);
				cidfg::Edge e(id_sv, 100, v->id, 100, "", cidfg::Edge::DEPENDENCY, 1, INT_MAX);
				g_.add_edge(e);
			}
		}
		else if (v->vertex_type == cidfg::Vertex::WRITE_AND_INDEX_VERTEX)
		{
			if (g_.get_vertex(g_.get_edge(g_.get_in_edge(v->id, 0))->src_id)->vertex_type != cidfg::Vertex::READ_AND_INDEX_VERTEX)
			{
				int parent_id;
				int child_index;
				g_.get_parent(v->id, parent_id, child_index);
				cidfg::Edge *ein0 = g_.get_edge(g_.get_in_edge(v->id, 1));
				cidfg::Vertex *vin0 = g_.get_vertex(ein0->src_id);

				if (static_cast<cidfg::ReadAndIndexVertex *>(v)->is_sram)
				{
					// Here add a ROUTE instruction
					__NOT_IMPLEMENTED__;
				}
				else
				{
					// Here add a SWB instruction
					BIR::SWBInstruction *instr = CREATE_OBJECT_B(SWBInstruction);
					cidfg::Edge *e0 = g_.get_edge(g_.get_in_edge(v->id, 0));
					cidfg::Vertex *src_v = g_.get_vertex(e0->src_id);
					cidfg::Vertex *dest_v = v;
					instr->source.cellCoordinate = src_v->coord;
					instr->source.type = BIR::BIREnumerations::ctDPU;
					instr->source.portNo = e0->src_port;
					instr->destination.cellCoordinate = dest_v->coord;
					instr->destination.type = BIR::BIREnumerations::ctRegFile;
					instr->destination.portNo = static_cast<cidfg::WriteAndIndexVertex *>(dest_v)->port;
					// e0->edge_type = cidfg::Edge::DEPENDENCY;
					// e0->d_hi = 0;
					// e0->d_lo = 0;
					cidfg::SwbInstrVertex sv;
					sv.instr = instr;
					sv.coord = dest_v->coord;
					int id_sv = g_.add_vertex(sv, parent_id, child_index);
					cidfg::Edge e(id_sv, 100, v->id, 100, "", cidfg::Edge::DEPENDENCY, 1, INT_MAX);
					//cidfg::Edge ee(id_sv, 101, v->id, 101, "", cidfg::Edge::DEPENDENCY, 0, 0);
					g_.add_edge(e);
					//g_.add_edge(ee);
				}
			}
		}
	}

	for (auto &id : loop_vertices)
	{
		g_.del_vertex(id);
	}
}

// void IndexVertexConverter::convert_refi(cidfg::CidfgGraph &g_)
// {

// 	std::set<int> remove_vertices;
// 	for (auto &v : g_.get_vertices())
// 	{
// 		if (v->vertex_type == cidfg::Vertex::READ_AND_INDEX_VERTEX)
// 		{
// 			remove_vertices.insert(v->id);
// 			int parent_id;
// 			int child_index;
// 			g_.get_parent(v->id, parent_id, child_index);
// 			cidfg::Vertex *vin0 = g_.get_vertex(g_.get_edge(g_.get_in_edge(v->id, 1))->src_id);

// 			if (static_cast<cidfg::ReadAndIndexVertex *>(v)->is_sram)
// 			{
// 				// Here add a SRAM READ instruction
// 				BIR::SRAMInstruction *instr = CREATE_OBJECT_B(SRAMInstruction);

// 				cidfg::Vertex *vv;
// 				vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v->id, 1))->src_id);
// 				if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
// 				{
// 					instr->initialAddress(static_cast<cidfg::ConstVertex *>(vv)->int_data, 1);
// 				}
// 				else if (vv->vertex_type == cidfg::Vertex::RACCU_INSTR_VERTEX)
// 				{
// 					string var_name = g_.get_edge(g_.get_in_edge(v->id, 1))->var_name;
// 					int reg_id;
// 					sscanf(var_name.c_str(), "RR_%d", &reg_id);
// 					instr->initialAddress(reg_id, 0);
// 				}
// 				vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v->id, 2))->src_id);
// 				if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
// 				{
// 					instr->loop1Increment = static_cast<cidfg::ConstVertex *>(vv)->int_data;
// 				}
// 				else if (vv->vertex_type == cidfg::Vertex::RACCU_INSTR_VERTEX)
// 				{
// 					LOG(FATAL) << "L1 increment field in SRAM_R instruction can only be constant!";
// 				}
// 				vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v->id, 3))->src_id);
// 				if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
// 				{
// 					instr->loop1Iterations(static_cast<cidfg::ConstVertex *>(vv)->int_data - 1, 1); // -1 because the instruction requires (number_of_iteration-1)
// 				}
// 				else
// 				{
// 					LOG(FATAL) << "number of address field in REFI instruction can only be constant!";
// 				}
// 				vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v->id, 4))->src_id);
// 				if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
// 				{
// 					instr->loop2Increment = static_cast<cidfg::ConstVertex *>(vv)->int_data; // Why let repetitionOffset to be int
// 				}
// 				else
// 				{
// 					LOG(FATAL) << "repetition offset field in REFI instruction can only be constant!";
// 				}
// 				vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v->id, 5))->src_id);
// 				if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
// 				{
// 					instr->loop2Iterations(static_cast<cidfg::ConstVertex *>(vv)->int_data - 1, 1); // -1 because the instruction requires (number_of_iteration-1)
// 				}
// 				else
// 				{
// 					LOG(FATAL) << "number of repeat field in REFI instruction can only be constatn!";
// 				}
// 				instr->isRead = true;

// 				cout << "SRAM_R(" << instr->initialAddress.value << "(" << instr->initialAddress.isStatic << ")," << instr->loop1Increment << "(1),"
// 						 << instr->loop1Iterations.value << "(" << instr->loop1Iterations.isStatic << ")," << instr->loop2Increment << "(1),"
// 						 << instr->loop2Iterations.value << "(" << instr->loop2Iterations.isStatic << "),"
// 						 << ")" << endl;

// 				BIR::ValueWithStaticFlag initialAddress;
// 				BIR::ValueWithStaticFlag initialDelay;
// 				BIR::ValueWithStaticFlag loop1Iterations;
// 				int loop1Increment;
// 				BIR::ValueWithStaticFlag loop1Delay;
// 				BIR::ValueWithStaticFlag loop2Iterations;
// 				BIR::ValueWithStaticFlag loop2Delay;
// 				int loop2Increment;
// 				bool isRead;
// 				int executionCycle;
// 				int hopNumber;
// 				BIR::RefiInstruction *correspondingRefiInst;
// 				BIR::RouteInstruction *correspondingRouteInst;
// 			}
// 			else
// 			{
// 				// Here add a REFI instruction
// 				BIR::RefiInstruction *instr = CREATE_OBJECT_B(RefiInstruction);
// 				instr->reverse_bits(0, true);
// 				cidfg::Vertex *vv;
// 				vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v->id, 1))->src_id);
// 				if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
// 				{
// 					instr->startAddress(static_cast<cidfg::ConstVertex *>(vv)->int_data, 1);
// 				}
// 				else if (vv->vertex_type == cidfg::Vertex::RACCU_INSTR_VERTEX)
// 				{
// 					string var_name = g_.get_edge(g_.get_in_edge(v->id, 1))->var_name;
// 					int reg_id;
// 					sscanf(var_name.c_str(), "RR_%d", &reg_id);
// 					instr->startAddress(reg_id, 0);
// 				}
// 				vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v->id, 2))->src_id);
// 				if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
// 				{
// 					instr->stepValue(static_cast<cidfg::ConstVertex *>(vv)->int_data, 1);
// 				}
// 				else if (vv->vertex_type == cidfg::Vertex::RACCU_INSTR_VERTEX)
// 				{
// 					string var_name = g_.get_edge(g_.get_in_edge(v->id, 2))->var_name;
// 					int reg_id;
// 					sscanf(var_name.c_str(), "RR_%d", &reg_id);
// 					instr->stepValue(reg_id, 0);
// 				}
// 				vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v->id, 3))->src_id);
// 				if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
// 				{
// 					instr->numberOfAddress(static_cast<cidfg::ConstVertex *>(vv)->int_data - 1, 1); // -1 because the instruction requires (number_of_iteration-1)
// 				}
// 				else
// 				{
// 					LOG(FATAL) << "number of address field in REFI instruction can only be constant!";
// 				}
// 				vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v->id, 4))->src_id);
// 				if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
// 				{
// 					instr->repetitionOffset = static_cast<cidfg::ConstVertex *>(vv)->int_data; // Why let repetitionOffset to be int
// 				}
// 				else
// 				{
// 					LOG(FATAL) << "repetition offset field in REFI instruction can only be constant!";
// 				}
// 				vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v->id, 5))->src_id);
// 				if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
// 				{
// 					instr->numberOfRepetition(static_cast<cidfg::ConstVertex *>(vv)->int_data - 1, 1); // -1 because the instruction requires (number_of_iteration-1)
// 				}
// 				else
// 				{
// 					LOG(FATAL) << "number of repeat field in REFI instruction can only be constatn!";
// 				}

// 				instr->portNo = static_cast<cidfg::ReadAndIndexVertex *>(v)->port;
// 				instr->portMode = BIR::BIREnumerations::pmOut;

// 				cout << "REFI(" << instr->startAddress.value << "(" << instr->startAddress.isStatic << ")," << instr->stepValue.value << "(" << instr->stepValue.isStatic << "),"
// 						 << instr->numberOfAddress.value << "(" << instr->numberOfAddress.isStatic << ")," << instr->repetitionOffset << "(1),"
// 						 << instr->numberOfRepetition.value << "(" << instr->numberOfRepetition.isStatic << "),"
// 						 << ")" << endl;

// 				cidfg::RefiInstrVertex rfv;
// 				rfv.coord = v->coord;
// 				rfv.instr = instr;
// 				int id_rfv = g_.add_vertex(rfv, parent_id, child_index);
// 				for (int i = 1; i < 6; i++)
// 				{
// 					cidfg::Edge *e_in = g_.get_edge(g_.get_in_edge(v->id, i));
// 					cidfg::Vertex *v_in = g_.get_vertex(e_in->src_id);
// 					if (v_in->vertex_type == cidfg::Vertex::CONST_VERTEX)
// 					{
// 						g_.del_edge(e_in->id);
// 						remove_vertices.insert(v_in->id);
// 					}
// 					else
// 					{
// 						e_in->dest_id = id_rfv;
// 					}
// 				}
// 				for (auto &e : g_.get_edges())
// 				{
// 					if (e->src_id == v->id)
// 					{
// 						e->src_id = id_rfv;
// 						if (e->src_port == 0)
// 						{
// 							e->src_port = static_cast<cidfg::ReadAndIndexVertex *>(v)->port;
// 						}
// 					}
// 					if (e->dest_id == v->id)
// 					{
// 						e->dest_id = id_rfv;
// 					}
// 				}
// 			}
// 		}
// 		else if (v->vertex_type == cidfg::Vertex::WRITE_AND_INDEX_VERTEX)
// 		{
// 			remove_vertices.insert(v->id);
// 			int parent_id;
// 			int child_index;
// 			g_.get_parent(v->id, parent_id, child_index);
// 			cidfg::Vertex *vin0 = g_.get_vertex(g_.get_edge(g_.get_in_edge(v->id, 1))->src_id);

// 			if (static_cast<cidfg::ReadAndIndexVertex *>(v)->is_sram)
// 			{
// 				// Here add a SRAM WRITE instruction
// 			}
// 			else
// 			{
// 				// Here add a SWB instruction
// 				BIR::RefiInstruction *instr = CREATE_OBJECT_B(RefiInstruction);
// 				instr->reverse_bits(0, true);
// 				cidfg::Vertex *vv;
// 				vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v->id, 2))->src_id);
// 				if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
// 				{
// 					instr->startAddress(static_cast<cidfg::ConstVertex *>(vv)->int_data, 1);
// 				}
// 				else if (vv->vertex_type == cidfg::Vertex::RACCU_INSTR_VERTEX)
// 				{
// 					string var_name = g_.get_edge(g_.get_in_edge(v->id, 2))->var_name;
// 					int reg_id;
// 					sscanf(var_name.c_str(), "RR_%d", &reg_id);
// 					instr->startAddress(reg_id, 0);
// 				}
// 				vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v->id, 3))->src_id);
// 				if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
// 				{
// 					instr->stepValue(static_cast<cidfg::ConstVertex *>(vv)->int_data, 1);
// 				}
// 				else if (vv->vertex_type == cidfg::Vertex::RACCU_INSTR_VERTEX)
// 				{
// 					string var_name = g_.get_edge(g_.get_in_edge(v->id, 3))->var_name;
// 					int reg_id;
// 					sscanf(var_name.c_str(), "RR_%d", &reg_id);
// 					instr->stepValue(reg_id, 0);
// 				}
// 				vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v->id, 4))->src_id);
// 				if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
// 				{
// 					instr->numberOfAddress(static_cast<cidfg::ConstVertex *>(vv)->int_data - 1, 1); // -1 because the instruction requires (number_of_iteration-1)
// 				}
// 				else
// 				{
// 					LOG(FATAL) << "number of address field in REFI instruction can only be constatn!";
// 				}
// 				vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v->id, 5))->src_id);
// 				if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
// 				{
// 					instr->repetitionOffset = static_cast<cidfg::ConstVertex *>(vv)->int_data; // Why let repetitionOffset to be int
// 				}
// 				else
// 				{
// 					LOG(FATAL) << "repetition offset field in REFI instruction can only be constatn!";
// 				}
// 				vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v->id, 6))->src_id);
// 				if (vv->vertex_type == cidfg::Vertex::CONST_VERTEX)
// 				{
// 					instr->numberOfRepetition(static_cast<cidfg::ConstVertex *>(vv)->int_data - 1, 1); // -1 because the instruction requires (number_of_iteration-1)
// 				}
// 				else
// 				{
// 					LOG(FATAL) << "number of repeat field in REFI instruction can only be constatn!";
// 				}

// 				instr->portNo = static_cast<cidfg::WriteAndIndexVertex *>(v)->port;
// 				instr->portMode = BIR::BIREnumerations::pmIn;

// 				cout << "REFI(" << instr->startAddress.value << "(" << instr->startAddress.isStatic << ")," << instr->stepValue.value << "(" << instr->stepValue.isStatic << "),"
// 						 << instr->numberOfAddress.value << "(" << instr->numberOfAddress.isStatic << ")," << instr->repetitionOffset << "(1),"
// 						 << instr->numberOfRepetition.value << "(" << instr->numberOfRepetition.isStatic << "),"
// 						 << ")" << endl;

// 				cidfg::RefiInstrVertex rfv;
// 				rfv.coord = v->coord;
// 				rfv.instr = instr;
// 				int id_rfv = g_.add_vertex(rfv, parent_id, child_index);

// 				for (int i = 2; i < 7; i++)
// 				{
// 					cidfg::Edge *e_in = g_.get_edge(g_.get_in_edge(v->id, i));
// 					cidfg::Vertex *v_in = g_.get_vertex(e_in->src_id);
// 					if (v_in->vertex_type == cidfg::Vertex::CONST_VERTEX)
// 					{
// 						g_.del_edge(e_in->id);
// 						remove_vertices.insert(v_in->id);
// 					}
// 					else
// 					{
// 						e_in->dest_id = id_rfv;
// 					}
// 				}
// 				for (auto &e : g_.get_edges())
// 				{
// 					if (e->src_id == v->id)
// 					{
// 						e->src_id = id_rfv;
// 					}
// 					if (e->dest_id == v->id)
// 					{
// 						e->dest_id = id_rfv;
// 						if (e->dest_port == 0)
// 						{
// 							e->dest_port = static_cast<cidfg::WriteAndIndexVertex *>(v)->port;
// 						}
// 					}
// 				}
// 			}
// 		}
// 	}

// 	for (auto &id : remove_vertices)
// 	{
// 		g_.del_vertex(id);
// 	}
// }

} // namespace codegen
} // namespace vesyla