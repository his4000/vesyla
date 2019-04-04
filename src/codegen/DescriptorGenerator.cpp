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

#include "DescriptorGenerator.hpp"

namespace vesyla
{
namespace codegen
{
DescriptorGenerator::DescriptorGenerator()
{
	_sn_counter = 1;
}
DescriptorGenerator::~DescriptorGenerator()
{
}

int DescriptorGenerator::get_lock_sn()
{
	return _sn_counter++;
}

schedule::Descriptor DescriptorGenerator::generate(cidfg::CidfgGraph &g_)
{
	schedule::Descriptor d;
	// stage 0: fill all instructions to descriptor
	gen_stage_0(g_, d);
	// stage 1: convert instruction node to detailed node to represent the critical timing, such as start, active and end time
	gen_stage_1(g_, d);

	return d;
}

void DescriptorGenerator::gen_stage_0(cidfg::CidfgGraph &g_, schedule::Descriptor &d_)
{
	for (auto &v : g_.get_vertices())
	{
		if (v->is_instr())
		{
			cidfg::InstrVertex *vv = static_cast<cidfg::InstrVertex *>(v);
			if (vv->instr != NULL)
			{
				d_.add_instruction(vv->coord.to_str(), vv->instr);
			}
		}
	}
}

string gen_name(string parent_name, string child_name, string variation = "")
{
	string str = parent_name + "_" + child_name;
	if (variation != "")
	{
		str += "_" + variation;
	}
	return str;
}

void DescriptorGenerator::gen_stage_1_callback(cidfg::CidfgGraph &g_, cidfg::CidfgGraph &new_g_, int id, int parent_id_, int child_index_, string parent_name_, std::set<int> &remove_vertices_, unordered_map<int, int> &hierarchical_instr_map_, unordered_map<int, vector<int>> &sub_vertex_map_)
{

	switch (g_.get_vertex(id)->vertex_type)
	{
	case cidfg::Vertex::HIERARCHICAL_INSTR_VERTEX:
	{
		cidfg::HierarchicalInstrVertex v0;
		v0.is_bulk = static_cast<cidfg::HierarchicalInstrVertex *>(g_.get_vertex(id))->is_bulk;
		v0.name = gen_name(parent_name_, to_string(id), "");
		int id_v0 = new_g_.add_vertex(v0, parent_id_, child_index_);
		sub_vertex_map_[id] = {id_v0};
		hierarchical_instr_map_[id] = id_v0;
		break;
	}
	case cidfg::Vertex::RACCU_INSTR_VERTEX:
	case cidfg::Vertex::WAIT_INSTR_VERTEX:
	case cidfg::Vertex::LOOPH_INSTR_VERTEX:
	case cidfg::Vertex::LOOPT_INSTR_VERTEX:
	case cidfg::Vertex::BRANCH_INSTR_VERTEX:
	case cidfg::Vertex::JUMP_INSTR_VERTEX:
	{
		cidfg::InstrVertex *vi = static_cast<cidfg::InstrVertex *>(g_.get_vertex(id));
		cidfg::HierarchicalInstrVertex v0;
		v0.name = gen_name(parent_name_, to_string(id), "0");
		vi->instr->labels.push_back(v0.name);
		v0.rot = {{"SEQ_" + vi->coord.to_str(), {{0, 0}}}};
		int id_v0 = new_g_.add_vertex(v0, parent_id_, child_index_);
		sub_vertex_map_[id] = {id_v0};
		break;
	}
	case cidfg::Vertex::DPU_INSTR_VERTEX:
	{
		int sn = get_lock_sn();
		cidfg::InstrVertex *vi = static_cast<cidfg::InstrVertex *>(g_.get_vertex(id));
		cidfg::HierarchicalInstrVertex v0, v1;
		v0.name = gen_name(parent_name_, to_string(id), "0");
		v1.name = gen_name(parent_name_, to_string(id), "1");
		vi->instr->labels.push_back(v0.name);
		vi->instr->labels.push_back(v1.name);
		string dpu_res_name = "DPU_" + vi->coord.to_str();
		v0.rot = {{"SEQ_" + vi->coord.to_str(), {{0, 0}}}, {dpu_res_name, {{0, INT_MAX, sn}}}};
		v1.rot = {{dpu_res_name, {{INT_MIN, 0, sn}}}};
		int id_v0 = new_g_.add_vertex(v0, parent_id_, child_index_);
		int id_v1 = new_g_.add_vertex(v1, parent_id_, child_index_);
		sub_vertex_map_[id] = {id_v0, id_v1};
		cidfg::Edge e0(id_v0, 0, id_v1, 0, "", cidfg::Edge::DEPENDENCY, 1, INT_MAX);
		new_g_.add_edge(e0);
		break;
	}
	case cidfg::Vertex::SWB_INSTR_VERTEX:
	{
		int sn = get_lock_sn();
		cidfg::SwbInstrVertex *vi = static_cast<cidfg::SwbInstrVertex *>(g_.get_vertex(id));
		cidfg::HierarchicalInstrVertex v0;
		cidfg::HierarchicalInstrVertex v1;
		v0.name = gen_name(parent_name_, to_string(id), "0");
		v1.name = gen_name(parent_name_, to_string(id), "1");
		vi->instr->labels.push_back(v0.name);
		vi->instr->labels.push_back(v1.name);
		string swb_res_name = "SWB_" + static_cast<SWBInstruction *>(vi->instr)->destination.to_string();
		v0.rot = {{"SEQ_" + vi->coord.to_str(), {{0, 0}}}, {swb_res_name, {{0, INT_MAX, sn}}}};
		v1.rot = {{swb_res_name, {{INT_MIN, 0, sn}}}};
		int id_v0 = new_g_.add_vertex(v0, parent_id_, child_index_);
		int id_v1 = new_g_.add_vertex(v1, parent_id_, child_index_);
		sub_vertex_map_[id] = {id_v0, id_v1};
		cidfg::Edge e1(id_v0, 0, id_v1, 0, "", cidfg::Edge::DEPENDENCY, 1, INT_MAX);
		new_g_.add_edge(e1);
		break;
	}
	case cidfg::Vertex::REFI_INSTR_VERTEX:
	{
		int sn = get_lock_sn();
		cidfg::RefiInstrVertex *vi = static_cast<cidfg::RefiInstrVertex *>(g_.get_vertex(id));
		cidfg::HierarchicalInstrVertex v0;
		cidfg::HierarchicalInstrVertex v1;
		cidfg::HierarchicalInstrVertex v2;
		v0.name = gen_name(parent_name_, to_string(id), "0");
		v1.name = gen_name(parent_name_, to_string(id), "1");
		v2.name = gen_name(parent_name_, to_string(id), "2");
		vi->instr->labels.push_back(v0.name);
		vi->instr->labels.push_back(v1.name);
		vi->instr->labels.push_back(v2.name);
		string refi_res_name = "AGU_" + vi->coord.to_str() + "_" + to_string(static_cast<RefiInstruction *>(vi->instr)->portNo);
		v0.rot = {{"SEQ_" + vi->coord.to_str(), {{0, 0}}}, {refi_res_name, {{0, INT_MAX, sn}}}};
		v2.rot = {{refi_res_name, {{INT_MIN, 0, sn}}}};
		int id_v0 = new_g_.add_vertex(v0, parent_id_, child_index_);
		int id_v1 = new_g_.add_vertex(v1, parent_id_, child_index_);
		int id_v2 = new_g_.add_vertex(v2, parent_id_, child_index_);
		sub_vertex_map_[id] = {id_v0, id_v1, id_v2};
		BIR::RefiInstruction *instruction = static_cast<BIR::RefiInstruction *>(vi->instr);
		cidfg::Edge e0(id_v0, 0, id_v1, 0, "", cidfg::Edge::DEPENDENCY, 0, INT_MAX);
		new_g_.add_edge(e0);
		int duration = (instruction->numberOfAddress.value + 1) * (instruction->numberOfRepetition.value + 1);
		if (vi->l1_linker.is_valid && vi->l2_linker.is_valid)
		{
			duration = 1;
		}
		else if (vi->l2_linker.is_valid)
		{
			LOG(DEBUG) << "L2 VALID";
			duration = (instruction->numberOfAddress.value + 1);
		}
		cidfg::Edge e1(id_v1, 0, id_v2, 0, "", cidfg::Edge::DEPENDENCY, duration, duration);
		new_g_.add_edge(e1);

		break;
	}
	case cidfg::Vertex::ROUTE_INSTR_VERTEX:
	{
		cidfg::RouteInstrVertex *vi = static_cast<cidfg::RouteInstrVertex *>(g_.get_vertex(id));
		cidfg::HierarchicalInstrVertex v0;
		cidfg::HierarchicalInstrVertex v1;
		v0.name = gen_name(parent_name_, to_string(id), "0");
		v1.name = gen_name(parent_name_, to_string(id), "1");
		vi->instr->labels.push_back(v0.name);
		vi->instr->labels.push_back(v1.name);

		//TODO: ADD RESOURCE FOR DIMARCH CELLS
		VIR::Coordinate src_coord = static_cast<BIR::RouteInstruction *>(vi->instr)->sourceCoordinate;
		VIR::Coordinate dest_coord = static_cast<BIR::RouteInstruction *>(vi->instr)->destCoordinate;
		if (src_coord.row > dest_coord.row)
		{
			VIR::Coordinate temp_coord = src_coord;
			src_coord = dest_coord;
			dest_coord = temp_coord;
		}
		std::set<string> occupied_sram_cells;
		for (int i = src_coord.row; i <= dest_coord.row; i++)
		{
			VIR::Coordinate c(i, src_coord.column);
			occupied_sram_cells.insert("SRAM_CELL_" + c.to_str());
		}
		int big_col = src_coord.column > dest_coord.column ? src_coord.column : dest_coord.column;
		int small_col = src_coord.column <= dest_coord.column ? src_coord.column : dest_coord.column;
		for (int i = small_col; i <= big_col; i++)
		{
			VIR::Coordinate c(dest_coord.row, i);
			occupied_sram_cells.insert("SRAM_CELL_" + c.to_str());
		}

		v0.rot = {{"SEQ_" + vi->coord.to_str(), {{0, 0}}}};
		v1.rot = {};
		for (auto &rn : occupied_sram_cells)
		{
			int sn = get_lock_sn();
			v0.rot[rn] = {{0, INT_MAX, sn}};
			v1.rot[rn] = {{INT_MIN, 0, sn}};
		}
		int id_v0 = new_g_.add_vertex(v0, parent_id_, child_index_);
		int id_v1 = new_g_.add_vertex(v1, parent_id_, child_index_);
		sub_vertex_map_[id] = {id_v0, id_v1};
		cidfg::Edge e1(id_v0, 0, id_v1, 0, "", cidfg::Edge::DEPENDENCY, 1, INT_MAX);
		new_g_.add_edge(e1);
		break;
	}
	case cidfg::Vertex::SRAM_INSTR_VERTEX:
	{
		cidfg::SramInstrVertex *vi = static_cast<cidfg::SramInstrVertex *>(g_.get_vertex(id));
		cidfg::HierarchicalInstrVertex v0;
		cidfg::HierarchicalInstrVertex v1;
		cidfg::HierarchicalInstrVertex v2;
		cidfg::HierarchicalInstrVertex v3;
		v0.name = gen_name(parent_name_, to_string(id), "0");
		v1.name = gen_name(parent_name_, to_string(id), "1");
		v2.name = gen_name(parent_name_, to_string(id), "2");
		v3.name = gen_name(parent_name_, to_string(id), "3");
		vi->instr->labels.push_back(v0.name);
		vi->instr->labels.push_back(v1.name);
		vi->instr->labels.push_back(v2.name);
		vi->instr->labels.push_back(v3.name);
		v0.rot = {{"SEQ_" + vi->coord.to_str(), {{0, 0}}}};
		v2.rot = {};
		int id_v0 = new_g_.add_vertex(v0, parent_id_, child_index_);
		int id_v1 = new_g_.add_vertex(v1, parent_id_, child_index_);
		int id_v2 = new_g_.add_vertex(v2, parent_id_, child_index_);
		int id_v3 = new_g_.add_vertex(v3, parent_id_, child_index_);
		sub_vertex_map_[id] = {id_v0, id_v1, id_v2, id_v3};
		BIR::SRAMInstruction *instruction = static_cast<BIR::SRAMInstruction *>(vi->instr);

		util::GlobalVar glv;
		int extra_dimarch_hop_delay = 0;
		if (glv.geti("extra_dimarch_hop_delay", extra_dimarch_hop_delay) < 0)
		{
			extra_dimarch_hop_delay = 0;
		}
		cidfg::Edge e0(id_v0, 0, id_v1, 0, "", cidfg::Edge::DEPENDENCY, instruction->hopNumber + 1 + extra_dimarch_hop_delay, instruction->hopNumber + 1 + extra_dimarch_hop_delay); // issue -> arrive
		cidfg::Edge e1(id_v1, 0, id_v2, 0, "", cidfg::Edge::DEPENDENCY, 0, 0);																																																			 // arrive -> active
		cidfg::Edge e2(id_v2, 0, id_v3, 0, "", cidfg::Edge::DEPENDENCY, instruction->executionCycle, INT_MAX);																																			 // active -> end
		new_g_.add_edge(e0);
		new_g_.add_edge(e1);
		new_g_.add_edge(e2);
		break;
	}
	default:
	{
		LOG(FATAL) << "Not support non-instruction vertex";
		break;
	}
	}

	if (g_.get_vertex(id)->is_hierarchical())
	{
		cidfg::HierarchicalVertex *vv = static_cast<cidfg::HierarchicalVertex *>(g_.get_vertex(id));
		for (int i = 0; i < vv->children.size(); i++)
		{
			for (int j = 0; j < vv->children[i].size(); j++)
			{
				gen_stage_1_callback(g_, new_g_, vv->children[i][j], hierarchical_instr_map_[id], i, gen_name(parent_name_, to_string(id)), remove_vertices_, hierarchical_instr_map_, sub_vertex_map_);
			}
		}
	}
}

void DescriptorGenerator::gen_stage_1(cidfg::CidfgGraph &g_, schedule::Descriptor &d_)
{
	cidfg::CidfgGraph new_g;

	std::set<int> remove_vertices;
	unordered_map<int, vector<int>> sub_vertex_map;
	for (auto &v : g_.get_vertices())
	{
		if (v->vertex_type == cidfg::Vertex::ROOT_VERTEX)
		{
			unordered_map<int, int> hierarchical_instr_map;
			cidfg::HierarchicalInstrVertex vi;
			vi.name = "ROOT";
			int id_vi = new_g.add_vertex(vi);
			hierarchical_instr_map[v->id] = id_vi;
			sub_vertex_map[v->id] = {id_vi};
			cidfg::HierarchicalVertex *vv = static_cast<cidfg::HierarchicalVertex *>(v);
			for (int i = 0; i < vv->children.size(); i++)
			{
				for (int j = 0; j < vv->children[i].size(); j++)
				{
					gen_stage_1_callback(g_, new_g, vv->children[i][j], id_vi, 0, "ROOT", remove_vertices, hierarchical_instr_map, sub_vertex_map);
				}
			}

			// cidfg::HierarchicalInstrVertex vi;
			// vi.name = "ROOT";
			// for (int i = 0; i < vv->children.size(); i++)
			// {
			// 	for (int j = 0; j < vv->children[i].size(); j++)
			// 	{
			// 		vi.add_child(i, vv->children[i][j]);
			// 	}
			// }
			//remove_vertices.insert(vv->id);
		}
	}

	for (auto &e : g_.get_edges())
	{
		cidfg::Vertex *v_src = g_.get_vertex(e->src_id);
		cidfg::Vertex *v_dest = g_.get_vertex(e->dest_id);
		if (v_src->vertex_type == cidfg::Vertex::SWB_INSTR_VERTEX && v_dest->vertex_type == cidfg::Vertex::REFI_INSTR_VERTEX)
		{
			cidfg::Edge e0(sub_vertex_map[v_src->id][0], 0, sub_vertex_map[v_dest->id][1], 0, "", cidfg::Edge::DEPENDENCY, 1, INT_MAX);
			cidfg::Edge e1(sub_vertex_map[v_dest->id][2], 0, sub_vertex_map[v_src->id][1], 0, "", cidfg::Edge::DEPENDENCY, 0, 0);
			new_g.add_edge(e0);
			new_g.add_edge(e1);
		}
		else if (v_src->vertex_type == cidfg::Vertex::SWB_INSTR_VERTEX && v_dest->vertex_type == cidfg::Vertex::HIERARCHICAL_INSTR_VERTEX)
		{
			cidfg::Edge e0(sub_vertex_map[v_src->id][0], 0, sub_vertex_map[v_dest->id][0], 0, "", cidfg::Edge::DEPENDENCY, 1, INT_MAX);
			cidfg::Edge e1(sub_vertex_map[v_dest->id][0], 0, sub_vertex_map[v_src->id][1], 0, "", cidfg::Edge::DEPENDENCY, 1, INT_MAX);
			new_g.add_edge(e0);
			new_g.add_edge(e1);
		}
		else if (v_src->vertex_type == cidfg::Vertex::ROUTE_INSTR_VERTEX && v_dest->vertex_type == cidfg::Vertex::SRAM_INSTR_VERTEX)
		{
			SRAMInstruction *instr_dest = static_cast<BIR::SRAMInstruction *>(static_cast<cidfg::InstrVertex *>(v_dest)->instr);

			cidfg::Edge e0(sub_vertex_map[v_src->id][0], 0, sub_vertex_map[v_dest->id][0], 0, "", cidfg::Edge::DEPENDENCY, 1, INT_MAX);
			cidfg::Edge e1(sub_vertex_map[v_dest->id][3], 0, sub_vertex_map[v_src->id][1], 0, "", cidfg::Edge::DEPENDENCY, 0, INT_MAX);
			new_g.add_edge(e0);
			new_g.add_edge(e1);
		}
		else if (v_src->vertex_type == cidfg::Vertex::ROUTE_INSTR_VERTEX && v_dest->vertex_type == cidfg::Vertex::REFI_INSTR_VERTEX)
		{
			cidfg::Edge e0(sub_vertex_map[v_src->id][0], 0, sub_vertex_map[v_dest->id][1], 0, "", cidfg::Edge::DEPENDENCY, 1, INT_MAX);
			cidfg::Edge e1(sub_vertex_map[v_dest->id][2], 0, sub_vertex_map[v_src->id][1], 0, "", cidfg::Edge::DEPENDENCY, 0, INT_MAX);
			new_g.add_edge(e0);
			new_g.add_edge(e1);
		}
		else if (v_src->vertex_type == cidfg::Vertex::REFI_INSTR_VERTEX && v_dest->vertex_type == cidfg::Vertex::DPU_INSTR_VERTEX)
		{
			RefiInstruction *instr_src = static_cast<BIR::RefiInstruction *>(static_cast<cidfg::InstrVertex *>(v_src)->instr);
			DPUInstruction *instr_dest = static_cast<BIR::DPUInstruction *>(static_cast<cidfg::InstrVertex *>(v_dest)->instr);
			cidfg::Edge e0(sub_vertex_map[v_src->id][1], 0, sub_vertex_map[v_dest->id][0], 0, "", cidfg::Edge::DEPENDENCY, 0, 0);
			new_g.add_edge(e0);
			if (e->dest_port == 0)
			{
				cidfg::Edge e1(sub_vertex_map[v_src->id][2], 0, sub_vertex_map[v_dest->id][1], 0, "", cidfg::Edge::DEPENDENCY, instr_dest->executionCycle + 1, instr_dest->executionCycle + 1);
				new_g.add_edge(e1);
			}
		}
		else if (v_src->vertex_type == cidfg::Vertex::DPU_INSTR_VERTEX && v_dest->vertex_type == cidfg::Vertex::REFI_INSTR_VERTEX)
		{
			DPUInstruction *instr_src = static_cast<BIR::DPUInstruction *>(static_cast<cidfg::InstrVertex *>(v_src)->instr);
			RefiInstruction *instr_dest = static_cast<BIR::RefiInstruction *>(static_cast<cidfg::InstrVertex *>(v_dest)->instr);
			if (instr_src->reductive)
			{
				cidfg::Edge e0(sub_vertex_map[v_src->id][1], 0, sub_vertex_map[v_dest->id][1], 0, "", cidfg::Edge::DEPENDENCY, 0, 0);
				new_g.add_edge(e0);
			}
			else
			{
				cidfg::Edge e0(sub_vertex_map[v_src->id][0], 0, sub_vertex_map[v_dest->id][1], 0, "", cidfg::Edge::DEPENDENCY, instr_src->executionCycle + 1, instr_src->executionCycle + 1);
				new_g.add_edge(e0);
			}
		}
		else if (v_src->vertex_type == cidfg::Vertex::DPU_INSTR_VERTEX && v_dest->vertex_type == cidfg::Vertex::DPU_INSTR_VERTEX && e->edge_type != cidfg::Edge::DEPENDENCY)
		{
			DPUInstruction *instr_src = static_cast<BIR::DPUInstruction *>(static_cast<cidfg::InstrVertex *>(v_src)->instr);
			DPUInstruction *instr_dest = static_cast<BIR::DPUInstruction *>(static_cast<cidfg::InstrVertex *>(v_dest)->instr);
			if (instr_src->reductive)
			{
				cidfg::Edge e0(sub_vertex_map[v_src->id][1], 0, sub_vertex_map[v_dest->id][0], 0, "", cidfg::Edge::DEPENDENCY, 0, 0);
				new_g.add_edge(e0);
			}
			else
			{
				cidfg::Edge e0(sub_vertex_map[v_src->id][0], 0, sub_vertex_map[v_dest->id][0], 0, "", cidfg::Edge::DEPENDENCY, instr_src->executionCycle + 1, instr_src->executionCycle + 1);
				new_g.add_edge(e0);
			}
		}
		else if (v_src->vertex_type == cidfg::Vertex::REFI_INSTR_VERTEX && (e->src_port == 0)) // this is a RAW dependency edge of some variable
		{
			cidfg::Edge e0(sub_vertex_map[v_src->id][2], 0, sub_vertex_map[v_dest->id][0], 0, "", cidfg::Edge::DEPENDENCY, 1, INT_MAX);
			new_g.add_edge(e0);
		}
		else if (v_src->vertex_type == cidfg::Vertex::REFI_INSTR_VERTEX && v_dest->vertex_type == cidfg::Vertex::REFI_INSTR_VERTEX && e->edge_type != cidfg::Edge::DEPENDENCY)
		{
			RefiInstruction *instr_src = static_cast<BIR::RefiInstruction *>(static_cast<cidfg::InstrVertex *>(v_src)->instr);
			RefiInstruction *instr_dest = static_cast<BIR::RefiInstruction *>(static_cast<cidfg::InstrVertex *>(v_dest)->instr);

			cidfg::Edge e0(sub_vertex_map[v_src->id][1], 0, sub_vertex_map[v_dest->id][1], 0, "", cidfg::Edge::DEPENDENCY, 0, 0);
			new_g.add_edge(e0);
		}
		else if (v_src->vertex_type == cidfg::Vertex::SRAM_INSTR_VERTEX && v_dest->vertex_type == cidfg::Vertex::REFI_INSTR_VERTEX && e->dest_port == 0)
		{
			SRAMInstruction *instr_src = static_cast<BIR::SRAMInstruction *>(static_cast<cidfg::InstrVertex *>(v_src)->instr);
			RefiInstruction *instr_dest = static_cast<BIR::RefiInstruction *>(static_cast<cidfg::InstrVertex *>(v_dest)->instr);

			if (instr_src->isRead)
			{
				cidfg::Edge e0(sub_vertex_map[v_src->id][2], 0, sub_vertex_map[v_dest->id][1], 0, "", cidfg::Edge::DEPENDENCY, instr_src->hopNumber, instr_src->hopNumber);
				new_g.add_edge(e0);
				cidfg::Edge e1(sub_vertex_map[v_src->id][3], 0, sub_vertex_map[v_dest->id][2], 0, "", cidfg::Edge::DEPENDENCY, 0, 0);
				new_g.add_edge(e1);
			}
			else
			{
				cidfg::Edge e0(sub_vertex_map[v_src->id][3], 0, sub_vertex_map[v_dest->id][0], 0, "", cidfg::Edge::DEPENDENCY, 0, INT_MAX);
				new_g.add_edge(e0);
			}
		}
		else if (v_src->vertex_type == cidfg::Vertex::REFI_INSTR_VERTEX && v_dest->vertex_type == cidfg::Vertex::SRAM_INSTR_VERTEX)
		{
			RefiInstruction *instr_src = static_cast<BIR::RefiInstruction *>(static_cast<cidfg::InstrVertex *>(v_src)->instr);
			SRAMInstruction *instr_dest = static_cast<BIR::SRAMInstruction *>(static_cast<cidfg::InstrVertex *>(v_dest)->instr);

			if (instr_dest->isRead)
			{
				cidfg::Edge e0(sub_vertex_map[v_src->id][2], 0, sub_vertex_map[v_dest->id][0], 0, "", cidfg::Edge::DEPENDENCY, 0, INT_MAX);
				new_g.add_edge(e0);
			}
			else
			{
				cidfg::Edge e0(sub_vertex_map[v_src->id][1], 0, sub_vertex_map[v_dest->id][2], 0, "", cidfg::Edge::DEPENDENCY, instr_dest->hopNumber, instr_dest->hopNumber);
				new_g.add_edge(e0);
			}

			// cidfg::Edge e0(sub_vertex_map[v_src->id][2], 0, sub_vertex_map[v_dest->id][1], 0, "", cidfg::Edge::DEPENDENCY, 0, 0);
			// new_g.add_edge(e0);
		}

		// IMPORTANT NOTE:
		// All dependency that has no strong/week indication will be considered as week dependency. The weak dependency points from starting of source node to the starting of destination node.
		else if (e->edge_type == cidfg::Edge::DEPENDENCY)
		{

			string fullString = e->var_name;
			string ending = "_strong";
			bool flag_strong_dependency = false;
			if (fullString.length() >= ending.length())
			{
				flag_strong_dependency = (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
			}
			if (flag_strong_dependency)
			{
				cidfg::Edge e0(sub_vertex_map[v_src->id].back(), 0, sub_vertex_map[v_dest->id][0], 0, "", cidfg::Edge::DEPENDENCY, e->d_lo, e->d_hi);
				new_g.add_edge(e0);
			}
			else
			{
				cidfg::Edge e0(sub_vertex_map[v_src->id][0], 0, sub_vertex_map[v_dest->id][0], 0, "", cidfg::Edge::DEPENDENCY, e->d_lo, e->d_hi);
				new_g.add_edge(e0);
			}
		}
		else
		{
			cidfg::Edge e0(sub_vertex_map[v_src->id][0], 0, sub_vertex_map[v_dest->id][0], 0, "", cidfg::Edge::DEPENDENCY, 1, INT_MAX);
			new_g.add_edge(e0);
		}
	}

	// Special treatment for vertices like REFI
	for (auto &v : g_.get_vertices())
	{
		if (v->vertex_type == cidfg::Vertex::REFI_INSTR_VERTEX)
		{
			cidfg::RefiInstrVertex *vv = static_cast<cidfg::RefiInstrVertex *>(v);
			static_cast<BIR::RefiInstruction *>(vv->instr)->corresponding_looph_l1 = NULL;
			static_cast<BIR::RefiInstruction *>(vv->instr)->corresponding_loopt_l1 = NULL;
			static_cast<BIR::RefiInstruction *>(vv->instr)->corresponding_looph_l2 = NULL;
			static_cast<BIR::RefiInstruction *>(vv->instr)->corresponding_loopt_l2 = NULL;
			if (vv->l1_linker.is_valid)
			{
				cidfg::InstrVertex *vlb = static_cast<cidfg::InstrVertex *>(g_.get_vertex(vv->l1_linker.lb));
				cidfg::Edge e0(sub_vertex_map[vv->id][0], 100, sub_vertex_map[vlb->id][0], 100, "", cidfg::Edge::DEPENDENCY, 1, INT_MAX);
				new_g.add_edge(e0);
				int vlb_parent_id;
				int vlb_child_index;
				new_g.get_parent(sub_vertex_map[vlb->id][0], vlb_parent_id, vlb_child_index);
				int vv_parent_id;
				int vv_child_index;
				new_g.get_parent(sub_vertex_map[vv->id][0], vv_parent_id, vv_child_index);
				vector<int> vec = static_cast<cidfg::HierarchicalVertex *>(new_g.get_vertex(vv_parent_id))->children[vv_child_index];
				static_cast<cidfg::HierarchicalVertex *>(new_g.get_vertex(vv_parent_id))->children[vv_child_index].clear();
				for (auto &id : vec)
				{
					if (id != sub_vertex_map[vv->id][0])
					{
						static_cast<cidfg::HierarchicalVertex *>(new_g.get_vertex(vv_parent_id))->children[vv_child_index].push_back(id);
					}
				}
				static_cast<cidfg::HierarchicalVertex *>(new_g.get_vertex(vlb_parent_id))->children[vlb_child_index].push_back(sub_vertex_map[vv->id][0]);

				static_cast<BIR::RefiInstruction *>(vv->instr)->corresponding_looph_l1 = static_cast<cidfg::LoophInstrVertex *>(g_.get_vertex(vv->l1_linker.lh))->instr;
				static_cast<BIR::RefiInstruction *>(vv->instr)->corresponding_loopt_l1 = static_cast<cidfg::LooptInstrVertex *>(g_.get_vertex(vv->l1_linker.lt))->instr;
			}
			if (vv->l2_linker.is_valid)
			{
				cidfg::InstrVertex *vlb = static_cast<cidfg::InstrVertex *>(g_.get_vertex(vv->l2_linker.lb));
				cidfg::Edge e0(sub_vertex_map[vv->id][0], 100, sub_vertex_map[vlb->id][0], 100, "", cidfg::Edge::DEPENDENCY, 1, INT_MAX);
				new_g.add_edge(e0);
				int vlb_parent_id;
				int vlb_child_index;
				new_g.get_parent(sub_vertex_map[vlb->id][0], vlb_parent_id, vlb_child_index);
				int vv_parent_id;
				int vv_child_index;
				new_g.get_parent(sub_vertex_map[vv->id][0], vv_parent_id, vv_child_index);
				vector<int> vec = static_cast<cidfg::HierarchicalVertex *>(new_g.get_vertex(vv_parent_id))->children[vv_child_index];
				static_cast<cidfg::HierarchicalVertex *>(new_g.get_vertex(vv_parent_id))->children[vv_child_index].clear();
				for (auto &id : vec)
				{
					if (id != sub_vertex_map[vv->id][0])
					{
						static_cast<cidfg::HierarchicalVertex *>(new_g.get_vertex(vv_parent_id))->children[vv_child_index].push_back(id);
					}
				}
				static_cast<cidfg::HierarchicalVertex *>(new_g.get_vertex(vlb_parent_id))->children[vlb_child_index].push_back(sub_vertex_map[vv->id][0]);

				static_cast<BIR::RefiInstruction *>(vv->instr)->corresponding_looph_l2 = static_cast<cidfg::LoophInstrVertex *>(g_.get_vertex(vv->l2_linker.lh))->instr;
				static_cast<BIR::RefiInstruction *>(vv->instr)->corresponding_loopt_l2 = static_cast<cidfg::LooptInstrVertex *>(g_.get_vertex(vv->l2_linker.lt))->instr;
			}
		}
	}

	for (auto &id : remove_vertices)
	{
		g_.del_vertex(id);
	}

	d_.set_entry("ROOT");

	for (auto &v : new_g.get_vertices())
	{
		if (v->vertex_type == cidfg::Vertex::HIERARCHICAL_INSTR_VERTEX)
		{
			cidfg::HierarchicalInstrVertex *vv = static_cast<cidfg::HierarchicalInstrVertex *>(v);
			vector<string> child0 = {};
			vector<string> child1 = {};
			for (int i = 0; i < vv->children.size(); i++)
			{
				if (i >= 2)
				{
					break;
				}
				for (int j = 0; j < vv->children[i].size(); j++)
				{
					if (i == 0)
					{
						child0.push_back(static_cast<cidfg::HierarchicalInstrVertex *>(new_g.get_vertex(vv->children[i][j]))->name);
					}
					else if (i == 1)
					{
						child1.push_back(static_cast<cidfg::HierarchicalInstrVertex *>(new_g.get_vertex(vv->children[i][j]))->name);
					}
				}
			}
			schedule::Operation o(vv->name, -1, vv->rot, child0, child1, vv->is_bulk);
			d_.add_operation(o);
		}
	}
	for (auto &e : new_g.get_edges())
	{
		if (e->edge_type == cidfg::Edge::DEPENDENCY && new_g.get_vertex(e->src_id)->vertex_type == cidfg::Vertex::HIERARCHICAL_INSTR_VERTEX && new_g.get_vertex(e->dest_id)->vertex_type == cidfg::Vertex::HIERARCHICAL_INSTR_VERTEX)
		{
			cidfg::HierarchicalInstrVertex *v_src = static_cast<cidfg::HierarchicalInstrVertex *>(new_g.get_vertex(e->src_id));
			cidfg::HierarchicalInstrVertex *v_dest = static_cast<cidfg::HierarchicalInstrVertex *>(new_g.get_vertex(e->dest_id));
			schedule::Constraint c(v_src->name, v_dest->name, e->d_lo, e->d_hi);
			d_.add_constraint(c);
		}
	}
}
} // namespace codegen
} // namespace vesyla