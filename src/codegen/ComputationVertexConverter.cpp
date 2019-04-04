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

#include "ComputationVertexConverter.hpp"
using namespace boost;
using namespace BIR;
using namespace BIR::BIREnumerations;
namespace vesyla
{
namespace codegen
{
ComputationVertexConverter::ComputationVertexConverter()
{
	string primitive_func_config_file = vesyla::util::SysPath::find_config_file(
			"primitive_func_def.xml");
	if (primitive_func_config_file != "")
	{
		_pf_lib.load(primitive_func_config_file);
		LOG(INFO) << "Use dpu mode defination file: " << primitive_func_config_file;
	}
	else
	{
		LOG(FATAL) << "DPU mode defination file is not found. Abort!";
	}

	vesyla::util::GlobalVar glv;
	CHECK(glv.geti("raccu_reg_file_depth", _raccu_reg_file_depth));
}
ComputationVertexConverter::~ComputationVertexConverter()
{
}

void ComputationVertexConverter::fill_dpu_instr(BIR::DPUInstruction *instr_, cidfg::CidfgGraph &g_, int id_)
{
	cidfg::Vertex *v = g_.get_vertex(id_);
	CHECK_EQ(v->vertex_type, cidfg::Vertex::COMPUTATION_VERTEX);
	cidfg::ComputationVertex *vv = static_cast<cidfg::ComputationVertex *>(v);
	CHECK_EQ(vv->is_on_dpu, true);

	if (vv->func_name == "+" || vv->func_name == "add" || vv->func_name == "ADD" || vv->func_name == "silago_dpu_add" || vv->func_name == "silago_dpu_add_with_constant")
	{
		cidfg::Edge *ein0 = g_.get_edge(g_.get_in_edge(vv->id, 0));
		cidfg::Edge *ein1 = g_.get_edge(g_.get_in_edge(vv->id, 1));
		cidfg::Vertex *vin0 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 0))->src_id);
		cidfg::Vertex *vin1 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 1))->src_id);
		if (vin1->vertex_type == cidfg::Vertex::CONST_VERTEX)
		{
			instr_->mode = "silago_dpu_add_with_constant";
			if (static_cast<cidfg::ConstVertex *>(vin1)->value_type == cidfg::ConstVertex::INT)
			{
				instr_->int_imm_data = static_cast<cidfg::ConstVertex *>(vin1)->int_data;
				instr_->fixedPointMode = false;
			}
			else
			{
				instr_->float_imm_data = static_cast<cidfg::ConstVertex *>(vin1)->float_data;
				instr_->fixedPointMode = true;
			}
			instr_->saturation = true;
			instr_->ioChange = BIR::BIREnumerations::dicUnchanged;
		}
		else
		{
			instr_->mode = "silago_dpu_add";
			instr_->constantValue = 0;
			instr_->saturation = true;
			instr_->ioChange = BIR::BIREnumerations::dicUnchanged;
		}
	}
	else if (vv->func_name == "-" || vv->func_name == "sub" || vv->func_name == "SUB" || vv->func_name == "silago_dpu_sub" || vv->func_name == "silago_dpu_sub_with_constant")
	{
		cidfg::Edge *ein0 = g_.get_edge(g_.get_in_edge(vv->id, 0));
		cidfg::Edge *ein1 = g_.get_edge(g_.get_in_edge(vv->id, 1));
		cidfg::Vertex *vin0 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 0))->src_id);
		cidfg::Vertex *vin1 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 1))->src_id);
		if (vin1->vertex_type == cidfg::Vertex::CONST_VERTEX)
		{
			instr_->mode = "silago_dpu_add_with_constant";
			if (static_cast<cidfg::ConstVertex *>(vin1)->value_type == cidfg::ConstVertex::INT)
			{
				instr_->int_imm_data = -(static_cast<cidfg::ConstVertex *>(vin1)->int_data);
				instr_->fixedPointMode = false;
			}
			else
			{
				instr_->float_imm_data = -(static_cast<cidfg::ConstVertex *>(vin1)->float_data);
				instr_->fixedPointMode = true;
			}
			instr_->ioChange = BIR::BIREnumerations::dicUnchanged;
		}
		else
		{
			instr_->mode = "silago_dpu_sub";
			instr_->constantValue = 0;
			instr_->ioChange = BIR::BIREnumerations::dicUnchanged;
		}
	}
	else if (vv->func_name == "*" || vv->func_name == ".*" || vv->func_name == "mul" || vv->func_name == "MUL" || vv->func_name == "mult" || vv->func_name == "MULT" || vv->func_name == "silago_dpu_mul" || vv->func_name == "silago_dpu_mult")
	{
		cidfg::Edge *ein0 = g_.get_edge(g_.get_in_edge(vv->id, 0));
		cidfg::Edge *ein1 = g_.get_edge(g_.get_in_edge(vv->id, 1));
		cidfg::Vertex *vin0 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 0))->src_id);
		cidfg::Vertex *vin1 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 1))->src_id);

		CHECK_NE(vin0->vertex_type, cidfg::Vertex::CONST_VERTEX);
		CHECK_NE(vin1->vertex_type, cidfg::Vertex::CONST_VERTEX);

		instr_->mode = "silago_dpu_mult";
		instr_->constantValue = 0;
		instr_->ioChange = BIR::BIREnumerations::dicUnchanged;
	}
	else if (vv->func_name == "mac" || vv->func_name == "MAC" || vv->func_name == "silago_dpu_mac")
	{
		cidfg::Edge *ein0 = g_.get_edge(g_.get_in_edge(vv->id, 0));
		cidfg::Edge *ein1 = g_.get_edge(g_.get_in_edge(vv->id, 1));
		cidfg::Vertex *vin0 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 0))->src_id);
		cidfg::Vertex *vin1 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 1))->src_id);
		CHECK_NE(vin0->vertex_type, cidfg::Vertex::CONST_VERTEX);
		CHECK_NE(vin1->vertex_type, cidfg::Vertex::CONST_VERTEX);
		instr_->mode = "silago_dpu_mac";
		instr_->constantValue = 0;
		instr_->ioChange = BIR::BIREnumerations::dicUnchanged;
		instr_->repetition = 0xFF;
	}
	else if (vv->func_name == "abs_mac" || vv->func_name == "ABS_MAC" || vv->func_name == "silago_dpu_abs_mac")
	{
		cidfg::Edge *ein0 = g_.get_edge(g_.get_in_edge(vv->id, 0));
		cidfg::Edge *ein1 = g_.get_edge(g_.get_in_edge(vv->id, 1));
		cidfg::Vertex *vin0 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 0))->src_id);
		cidfg::Vertex *vin1 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 1))->src_id);
		CHECK_NE(vin0->vertex_type, cidfg::Vertex::CONST_VERTEX);
		CHECK_NE(vin1->vertex_type, cidfg::Vertex::CONST_VERTEX);
		instr_->mode = "silago_dpu_mac";
		instr_->constantValue = 0;
		instr_->ioChange = BIR::BIREnumerations::dicAbsoluteOfOutput;
		instr_->repetition = 0xFF;
	}
	else if (vv->func_name == "abs" || vv->func_name == "ABS" || vv->func_name == "silago_dpu_abs")
	{
		cidfg::Edge *ein0 = g_.get_edge(g_.get_in_edge(vv->id, 0));
		cidfg::Vertex *vin0 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 0))->src_id);
		CHECK_NE(vin0->vertex_type, cidfg::Vertex::CONST_VERTEX);
		instr_->mode = "silago_dpu_mac";
		instr_->constantValue = 0;
		instr_->ioChange = BIR::BIREnumerations::dicAbsoluteOfOutput;
	}
	else if (vv->func_name == "silago_dpu_abs_sub_acc")
	{
		cidfg::Edge *ein0 = g_.get_edge(g_.get_in_edge(vv->id, 0));
		cidfg::Vertex *vin0 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 0))->src_id);
		CHECK_NE(vin0->vertex_type, cidfg::Vertex::CONST_VERTEX);
		instr_->mode = "silago_dpu_abs_sub_acc";
		instr_->constantValue = 0;
		instr_->ioChange = BIR::BIREnumerations::dicAbsoluteOfOutput;
	}
	else if (vv->func_name == "<<" || vv->func_name == "shift_left" || vv->func_name == "SHIFT_LEFT" || vv->func_name == "silago_dpu_shift_left")
	{
		cidfg::Edge *ein0 = g_.get_edge(g_.get_in_edge(vv->id, 0));
		cidfg::Edge *ein1 = g_.get_edge(g_.get_in_edge(vv->id, 1));
		cidfg::Vertex *vin0 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 0))->src_id);
		cidfg::Vertex *vin1 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 1))->src_id);
		CHECK_NE(vin0->vertex_type, cidfg::Vertex::CONST_VERTEX);
		CHECK_NE(vin1->vertex_type, cidfg::Vertex::CONST_VERTEX);
		instr_->mode = "silago_dpu_shift_left";
		instr_->constantValue = 0;
		instr_->ioChange = BIR::BIREnumerations::dicUnchanged;
	}
	else if (vv->func_name == ">>" || vv->func_name == "shift_right" || vv->func_name == "SHIFT_RIGHT" || vv->func_name == "silago_dpu_shift_right")
	{
		cidfg::Edge *ein0 = g_.get_edge(g_.get_in_edge(vv->id, 0));
		cidfg::Edge *ein1 = g_.get_edge(g_.get_in_edge(vv->id, 1));
		cidfg::Vertex *vin0 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 0))->src_id);
		cidfg::Vertex *vin1 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 1))->src_id);
		CHECK_NE(vin0->vertex_type, cidfg::Vertex::CONST_VERTEX);
		CHECK_NE(vin1->vertex_type, cidfg::Vertex::CONST_VERTEX);
		instr_->mode = "silago_dpu_shift_right";
		instr_->constantValue = 0;
		instr_->ioChange = BIR::BIREnumerations::dicUnchanged;
	}
	else if (vv->func_name == "==" || vv->func_name == "~=" || vv->func_name == "<" || vv->func_name == "<=" || vv->func_name == ">" || vv->func_name == ">=")
	{
		cidfg::Edge *ein0 = g_.get_edge(g_.get_in_edge(vv->id, 0));
		cidfg::Edge *ein1 = g_.get_edge(g_.get_in_edge(vv->id, 1));
		cidfg::Vertex *vin0 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 0))->src_id);
		cidfg::Vertex *vin1 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 1))->src_id);
		CHECK_NE(vin0->vertex_type, cidfg::Vertex::CONST_VERTEX);
		if (vin1->vertex_type == cidfg::Vertex::CONST_VERTEX)
		{
			instr_->mode = "silago_dpu_comparison_with_constant_" + vv->func_name; // keep this format for further process by branch instr node.
			if (static_cast<cidfg::ConstVertex *>(vin1)->value_type == cidfg::ConstVertex::INT)
			{
				instr_->int_imm_data = static_cast<cidfg::ConstVertex *>(vin1)->int_data;
				instr_->fixedPointMode = false;
			}
			else
			{
				instr_->float_imm_data = static_cast<cidfg::ConstVertex *>(vin1)->float_data;
				instr_->fixedPointMode = true;
			}
		}
		else
		{
			instr_->mode = "silago_dpu_comparison_" + vv->func_name; // keep this format for further process by branch instr node.
			instr_->constantValue = 0;
		}
		instr_->ioChange = BIR::BIREnumerations::dicUnchanged;
	}
	else if (vv->func_name == "min" || vv->func_name == "MIN" || vv->func_name == "silago_dpu_min")
	{
		cidfg::Edge *ein0 = g_.get_edge(g_.get_in_edge(vv->id, 0));
		cidfg::Vertex *vin0 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 0))->src_id);
		CHECK_NE(vin0->vertex_type, cidfg::Vertex::CONST_VERTEX);
		instr_->mode = "silago_dpu_min";
		instr_->constantValue = 0;
		instr_->ioChange = BIR::BIREnumerations::dicUnchanged;
	}
	else if (vv->func_name == "max" || vv->func_name == "MAX" || vv->func_name == "silago_dpu_max")
	{
		cidfg::Edge *ein0 = g_.get_edge(g_.get_in_edge(vv->id, 0));
		cidfg::Vertex *vin0 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 0))->src_id);
		CHECK_NE(vin0->vertex_type, cidfg::Vertex::CONST_VERTEX);
		instr_->mode = "silago_dpu_max";
		instr_->constantValue = 0;
		instr_->ioChange = BIR::BIREnumerations::dicUnchanged;
	}
	else if (vv->func_name == "ld" || vv->func_name == "LD" || vv->func_name == "silago_dpu_load_constant")
	{
		cidfg::Edge *ein0 = g_.get_edge(g_.get_in_edge(vv->id, 0));
		cidfg::Vertex *vin0 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 0))->src_id);
		CHECK_EQ(vin0->vertex_type, cidfg::Vertex::CONST_VERTEX);
		instr_->mode = "silago_dpu_load_constant";
		if (static_cast<cidfg::ConstVertex *>(vin0)->value_type == cidfg::ConstVertex::INT)
		{
			instr_->int_imm_data = static_cast<cidfg::ConstVertex *>(vin0)->int_data;
			instr_->fixedPointMode = false;
		}
		else
		{
			instr_->float_imm_data = static_cast<cidfg::ConstVertex *>(vin0)->float_data;
			instr_->fixedPointMode = true;
		}

		instr_->ioChange = BIR::BIREnumerations::dicUnchanged;
	}
	else if (vv->func_name == "silago_dpu_load_reg_0" || vv->func_name == "silago_dpu_store_reg_0")
	{
		instr_->mode = vv->func_name;
		instr_->int_imm_data = 1;
		instr_->fixedPointMode = false;
		instr_->ioChange = BIR::BIREnumerations::dicUnchanged;
	}
	else if (vv->func_name == "silago_dpu_load_reg_1" || vv->func_name == "silago_dpu_store_reg_1")
	{
		instr_->mode = vv->func_name;
		instr_->int_imm_data = 2;
		instr_->fixedPointMode = false;
		instr_->ioChange = BIR::BIREnumerations::dicUnchanged;
	}
	else if (vv->func_name == "silago_dpu_load_reg_both" || vv->func_name == "silago_dpu_store_reg_both")
	{
		instr_->mode = vv->func_name;
		instr_->int_imm_data = 3;
		instr_->fixedPointMode = false;
		instr_->ioChange = BIR::BIREnumerations::dicUnchanged;
	}
	else
	{
		instr_->mode = vv->func_name;
		instr_->constantValue = 0;
		instr_->ioChange = BIR::BIREnumerations::dicUnchanged;
		instr_->repetition = 0xff;
	}
	instr_->repetition = 0xff;
}

void ComputationVertexConverter::fill_raccu_instr(BIR::RACCUInstruction *instr_, cidfg::CidfgGraph &g_, int id_)
{
	cidfg::Vertex *v = g_.get_vertex(id_);
	CHECK_EQ(v->vertex_type, cidfg::Vertex::COMPUTATION_VERTEX);
	cidfg::ComputationVertex *vv = static_cast<cidfg::ComputationVertex *>(v);
	CHECK_EQ(vv->is_on_dpu, false);

	if (vv->func_name == "ld" || vv->func_name == "LD")
	{
		cidfg::Edge *ein0 = g_.get_edge(g_.get_in_edge(vv->id, 1));
		cidfg::Vertex *vin0 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 1))->src_id);

		instr_->raccuMode = BIR::BIREnumerations::rmAdd;
		CHECK_EQ(vin0->vertex_type, cidfg::Vertex::CONST_VERTEX);
		instr_->operand1.isStatic = true;
		instr_->operand1.value = static_cast<cidfg::ConstVertex *>(vin0)->int_data;
		instr_->operand2.isStatic = true;
		instr_->operand2.value = 0;
	}
	else if (vv->func_name == "+" || vv->func_name == "add" || vv->func_name == "ADD" || vv->func_name == "silago_raccu_add")
	{
		cidfg::Edge *ein0 = g_.get_edge(g_.get_in_edge(vv->id, 0));
		cidfg::Edge *ein1 = g_.get_edge(g_.get_in_edge(vv->id, 1));
		cidfg::Vertex *vin0 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 0))->src_id);
		cidfg::Vertex *vin1 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 1))->src_id);
		instr_->raccuMode = BIR::BIREnumerations::rmAdd;

		if (vin0->vertex_type == cidfg::Vertex::CONST_VERTEX)
		{
			instr_->operand1.isStatic = true;
			instr_->operand1.value = static_cast<cidfg::ConstVertex *>(vin0)->int_data;
		}
		else
		{
			if (strncmp(ein0->var_name.c_str(), "RR_", strlen("RR_")) == 0)
			{
				instr_->operand1.isStatic = false;
				int reg_addr;
				sscanf(ein0->var_name.c_str(), "RR_%d", &reg_addr);
				instr_->operand1.value = reg_addr;
			}
			else if (strncmp(ein0->var_name.c_str(), "IT_", strlen("IT_")) == 0)
			{
				instr_->raccuMode = BIR::BIREnumerations::rmAddWithLoopIndex;
				instr_->operand1.isStatic = true;
				int reg_addr;
				sscanf(ein0->var_name.c_str(), "IT_%d", &reg_addr);
				instr_->operand1.value = reg_addr;
			}
			else
			{
				instr_->operand1.isStatic = true;
				instr_->operand1.value = 0;
			}
		}

		if (vin1->vertex_type == cidfg::Vertex::CONST_VERTEX)
		{
			instr_->operand2.isStatic = true;
			instr_->operand2.value = static_cast<cidfg::ConstVertex *>(vin1)->int_data;
		}
		else
		{
			if (strncmp(ein1->var_name.c_str(), "RR_", strlen("RR_")) == 0)
			{
				instr_->operand2.isStatic = false;
				int reg_addr;
				sscanf(ein1->var_name.c_str(), "RR_%d", &reg_addr);
				instr_->operand2.value = reg_addr;
			}
			else if (strncmp(ein1->var_name.c_str(), "IT_", strlen("IT_")) == 0)
			{
				CHECK_NE(instr_->raccuMode, BIR::BIREnumerations::rmAddWithLoopIndex);
				instr_->raccuMode = BIR::BIREnumerations::rmAddWithLoopIndex;
				instr_->operand2 = instr_->operand1;
				instr_->operand1.isStatic = true;
				int reg_addr;
				sscanf(ein1->var_name.c_str(), "IT_%d", &reg_addr);
				instr_->operand1.value = reg_addr;
			}
			else
			{
				instr_->operand2.isStatic = true;
				instr_->operand2.value = 0;
			}
		}
	}
	else if (vv->func_name == "-" || vv->func_name == "sub" || vv->func_name == "SUB" || vv->func_name == "silago_raccu_sub")
	{
		cidfg::Edge *ein0 = g_.get_edge(g_.get_in_edge(vv->id, 0));
		cidfg::Edge *ein1 = g_.get_edge(g_.get_in_edge(vv->id, 1));
		cidfg::Vertex *vin0 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 0))->src_id);
		cidfg::Vertex *vin1 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 1))->src_id);
		instr_->raccuMode = BIR::BIREnumerations::rmSub;
		if (vin0->vertex_type == cidfg::Vertex::CONST_VERTEX)
		{
			instr_->operand1.isStatic = true;
			instr_->operand1.value = static_cast<cidfg::ConstVertex *>(vin0)->int_data;
		}
		else
		{
			if (strncmp(ein0->var_name.c_str(), "RR_", strlen("RR_")) == 0)
			{
				instr_->operand1.isStatic = false;
				int reg_addr;
				sscanf(ein0->var_name.c_str(), "RR_%d", &reg_addr);
				instr_->operand1.value = reg_addr;
			}
			else if (strncmp(ein0->var_name.c_str(), "IT_", strlen("IT_")) == 0)
			{
				LOG(FATAL) << "Substaction with loop index is not allowed!";
			}
			else
			{
				instr_->operand1.isStatic = true;
				instr_->operand1.value = 0;
			}
		}

		if (vin1->vertex_type == cidfg::Vertex::CONST_VERTEX)
		{
			instr_->operand2.isStatic = true;
			instr_->operand2.value = static_cast<cidfg::ConstVertex *>(vin1)->int_data;
		}
		else
		{
			if (strncmp(ein1->var_name.c_str(), "RR_", strlen("RR_")) == 0)
			{
				instr_->operand2.isStatic = false;
				int reg_addr;
				sscanf(ein1->var_name.c_str(), "RR_%d", &reg_addr);
				instr_->operand2.value = reg_addr;
			}
			else if (strncmp(ein1->var_name.c_str(), "IT_", strlen("IT_")) == 0)
			{
				LOG(FATAL) << "Substaction with loop index is not allowed!";
			}
			else
			{
				instr_->operand2.isStatic = true;
				instr_->operand2.value = 0;
			}
		}
	}
	else if (vv->func_name == "*" || vv->func_name == "mul" || vv->func_name == "MUL" || vv->func_name == "silago_raccu_mul")
	{
		cidfg::Edge *ein0 = g_.get_edge(g_.get_in_edge(vv->id, 0));
		cidfg::Edge *ein1 = g_.get_edge(g_.get_in_edge(vv->id, 1));
		cidfg::Vertex *vin0 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 0))->src_id);
		cidfg::Vertex *vin1 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 1))->src_id);
		LOG(FATAL) << "Unsupported raccu function " << vv->func_name;
	}
	else if (vv->func_name == "/" || vv->func_name == "div" || vv->func_name == "DIV" || vv->func_name == "silago_raccu_div")
	{
		cidfg::Edge *ein0 = g_.get_edge(g_.get_in_edge(vv->id, 0));
		cidfg::Edge *ein1 = g_.get_edge(g_.get_in_edge(vv->id, 1));
		cidfg::Vertex *vin0 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 0))->src_id);
		cidfg::Vertex *vin1 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 1))->src_id);
		LOG(FATAL) << "Unsupported raccu function " << vv->func_name;
	}
	else if (vv->func_name == "<<" || vv->func_name == "bitsll" || vv->func_name == "BITSLL" || vv->func_name == "silago_raccu_sll")
	{
		cidfg::Edge *ein0 = g_.get_edge(g_.get_in_edge(vv->id, 0));
		cidfg::Edge *ein1 = g_.get_edge(g_.get_in_edge(vv->id, 1));
		cidfg::Vertex *vin0 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 0))->src_id);
		cidfg::Vertex *vin1 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 1))->src_id);
		instr_->raccuMode = BIR::BIREnumerations::rmShiftLeft;
		if (vin0->vertex_type == cidfg::Vertex::CONST_VERTEX)
		{
			instr_->operand1.isStatic = true;
			instr_->operand1.value = static_cast<cidfg::ConstVertex *>(vin0)->int_data;
		}
		else
		{
			if (strncmp(ein0->var_name.c_str(), "RR_", strlen("RR_")) == 0)
			{
				instr_->operand1.isStatic = false;
				int reg_addr;
				sscanf(ein0->var_name.c_str(), "RR_%d", &reg_addr);
				instr_->operand1.value = reg_addr;
			}
			else if (strncmp(ein0->var_name.c_str(), "IT_", strlen("IT_")) == 0)
			{
				LOG(FATAL) << "Shift with loop index is not allowed!";
			}
			else
			{
				instr_->operand1.isStatic = true;
				instr_->operand1.value = 0;
			}
		}

		if (vin1->vertex_type == cidfg::Vertex::CONST_VERTEX)
		{
			instr_->operand2.isStatic = true;
			instr_->operand2.value = static_cast<cidfg::ConstVertex *>(vin1)->int_data;
		}
		else
		{
			if (strncmp(ein1->var_name.c_str(), "RR_", strlen("RR_")) == 0)
			{
				instr_->operand2.isStatic = false;
				int reg_addr;
				sscanf(ein1->var_name.c_str(), "RR_%d", &reg_addr);
				instr_->operand2.value = reg_addr;
			}
			else if (strncmp(ein1->var_name.c_str(), "IT_", strlen("IT_")) == 0)
			{
				LOG(FATAL) << "Shift with loop index is not allowed!";
			}
			else
			{
				instr_->operand2.isStatic = true;
				instr_->operand2.value = 0;
			}
		}
	}
	else if (vv->func_name == ">>" || vv->func_name == "bitsra" || vv->func_name == "BITSRA" || vv->func_name == "silago_raccu_sra")
	{
		cidfg::Edge *ein0 = g_.get_edge(g_.get_in_edge(vv->id, 0));
		cidfg::Edge *ein1 = g_.get_edge(g_.get_in_edge(vv->id, 1));
		cidfg::Vertex *vin0 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 0))->src_id);
		cidfg::Vertex *vin1 = g_.get_vertex(g_.get_edge(g_.get_in_edge(vv->id, 1))->src_id);
		instr_->raccuMode = BIR::BIREnumerations::rmShiftRight;
		if (vin0->vertex_type == cidfg::Vertex::CONST_VERTEX)
		{
			instr_->operand1.isStatic = true;
			instr_->operand1.value = static_cast<cidfg::ConstVertex *>(vin0)->int_data;
		}
		else
		{
			if (strncmp(ein0->var_name.c_str(), "RR_", strlen("RR_")) == 0)
			{
				instr_->operand1.isStatic = false;
				int reg_addr;
				sscanf(ein0->var_name.c_str(), "RR_%d", &reg_addr);
				instr_->operand1.value = reg_addr;
			}
			else if (strncmp(ein0->var_name.c_str(), "IT_", strlen("IT_")) == 0)
			{
				LOG(FATAL) << "Shift with loop index is not allowed!";
			}
			else
			{
				instr_->operand1.isStatic = true;
				instr_->operand1.value = 0;
			}
		}

		if (vin1->vertex_type == cidfg::Vertex::CONST_VERTEX)
		{
			instr_->operand2.isStatic = true;
			instr_->operand2.value = static_cast<cidfg::ConstVertex *>(vin1)->int_data;
		}
		else
		{
			if (strncmp(ein1->var_name.c_str(), "RR_", strlen("RR_")) == 0)
			{
				instr_->operand2.isStatic = false;
				int reg_addr;
				sscanf(ein1->var_name.c_str(), "RR_%d", &reg_addr);
				instr_->operand2.value = reg_addr;
			}
			else if (strncmp(ein1->var_name.c_str(), "IT_", strlen("IT_")) == 0)
			{
				LOG(FATAL) << "Shift with loop index is not allowed!";
			}
			else
			{
				instr_->operand2.isStatic = true;
				instr_->operand2.value = 0;
			}
		}
	}
	else
	{
		LOG(FATAL) << "Unsupported raccu function " << vv->func_name;
	}

	int reg_addr = -1;
	for (auto &e : g_.get_edges())
	{
		if (e->edge_type == cidfg::Edge::DEPENDENCY)
		{
			continue;
		}
		if (e->src_id == id_)
		{
			CHECK_EQ(strncmp(e->var_name.c_str(), "RR_", strlen("RR_")), 0);
			sscanf(e->var_name.c_str(), "RR_%d", &reg_addr);
		}
	}
	CHECK_GE(reg_addr, 0);
	instr_->resultAddress = reg_addr;
}

void ComputationVertexConverter::convert(cidfg::CidfgGraph &g_)
{
	// insert computation node between iteraotr source node and index node, since
	// iteraotor can't be address generation constraint

	for (auto &e : g_.get_edges())
	{
		if (g_.get_vertex(e->src_id)->vertex_type == cidfg::Vertex::VAR_VERTEX &&
				(g_.get_vertex(e->dest_id)->vertex_type == cidfg::Vertex::READ_AND_INDEX_VERTEX ||
				 g_.get_vertex(e->dest_id)->vertex_type == cidfg::Vertex::WRITE_AND_INDEX_VERTEX ||
				 g_.get_vertex(e->dest_id)->vertex_type == cidfg::Vertex::LOOP_VERTEX))
		{
			cidfg::VarVertex *vv = static_cast<cidfg::VarVertex *>(g_.get_vertex(e->src_id));
			if (vv->is_iterator)
			{
				int parent_id;
				int child_index;
				g_.get_parent(vv->id, parent_id, child_index);
				cidfg::ConstVertex constv;
				constv.set_int_value(0);
				int id_constv = g_.add_vertex(constv, parent_id, child_index);
				cidfg::ComputationVertex cv;
				cv.func_name = "+";
				cv.coord = vv->coord;
				int id_cv = g_.add_vertex(cv, parent_id, child_index);
				cidfg::Edge ee0(id_constv, 0, id_cv, 1);
				g_.add_edge(ee0);
				cidfg::Edge ee1(id_cv, 0, e->dest_id, e->dest_port);
				g_.add_edge(ee1);
				e->dest_id = id_cv;
				e->dest_port = 0;
			}
		}
	}

	for (auto &v : g_.get_vertices())
	{
		if (v->vertex_type == cidfg::Vertex::ROOT_VERTEX)
		{
			cidfg::HierarchicalVertex *vv = static_cast<cidfg::HierarchicalVertex *>(v);
			if (vv->children.size() > 0)
			{
				vector<int> var_list = find_all_raccu_var_edges(g_, vv->children[0]);
				std::map<string, int> curr_reg_id;
				for (auto &v : g_.get_vertices())
				{
					curr_reg_id[v->coord.to_str()] = 0;
				}
				std::unordered_map<string, string> vt;
				curr_reg_id = allocate_raccu_reg(g_, var_list, curr_reg_id, vt);
				convert_raccu_vertex(g_, vv->children[0], curr_reg_id, vt);
			}
		}
	}

	// vesyla::util::GlobalVar glv;
	// string path;
	// CHECK(glv.gets("$CURR_DIR", path));
	// path = path + "cidfg/";
	// mkdir(path.c_str(), 0755);
	// string input_file_name;
	// CHECK(glv.gets("input_file_name", input_file_name));
	// string path_dot;
	// path_dot = path + input_file_name + "_test.dot";
	// ofstream ofs0(path_dot, ofstream::out);
	// ofs0 << g_.generate_dot_graph();
	// ofs0.close();

	for (auto &v : g_.get_vertices())
	{
		if (v->vertex_type == cidfg::Vertex::VAR_VERTEX)
		{
			cidfg::VarVertex *vv = static_cast<cidfg::VarVertex *>(v);
			if (vv->is_iterator)
			{
				for (auto &e : g_.get_edges())
				{
					if (e->src_id == v->id)
					{
						e->var_name = "IT_" + to_string(vv->loop_id);
					}
				}
			}
			else
			{
				for (auto &e : g_.get_edges())
				{
					if (e->src_id == v->id)
					{
						e->var_name = vv->var_name;
					}
				}
			}
		}
	}

	//	Replace edge var name with VarVertex name

	set<int> remove_vertex_id;
	for (auto &v : g_.get_vertices())
	{
		if (v->vertex_type == cidfg::Vertex::COMPUTATION_VERTEX)
		{
			cidfg::ComputationVertex *vv = static_cast<cidfg::ComputationVertex *>(v);
			if (vv->is_on_dpu == false)
			{
				BIR::RACCUInstruction *instr = CREATE_OBJECT_B(RACCUInstruction);
				fill_raccu_instr(instr, g_, vv->id);

				cidfg::RaccuInstrVertex iv;
				iv.coord = v->coord;
				iv.instr = instr;
				int parent_id;
				int child_index;
				g_.get_parent(v->id, parent_id, child_index);
				int id_iv = g_.add_vertex(iv, parent_id, child_index);
				for (auto &e : g_.get_edges())
				{

					if (e->src_id == v->id)
					{
						e->src_id = id_iv;
					}
					if (e->dest_id == v->id)
					{
						if (g_.get_vertex(e->src_id)->vertex_type == cidfg::Vertex::CONST_VERTEX || g_.get_vertex(e->src_id)->vertex_type == cidfg::Vertex::VAR_VERTEX)
						{
							remove_vertex_id.insert(e->src_id);
						}
						else
						{
							e->dest_id = id_iv;
						}
					}
				}
				remove_vertex_id.insert(v->id);
			}
			else
			{
				BIR::DPUInstruction *instr = CREATE_OBJECT_B(DPUInstruction);
				fill_dpu_instr(instr, g_, vv->id);
				string instr_mode_str = instr->mode;
				if (strncmp(instr_mode_str.c_str(), "silago_dpu_comparison", strlen("silago_dpu_comparison")) == 0)
				{
					instr_mode_str = "silago_dpu_comparison";
				}
				else if (strncmp(instr_mode_str.c_str(), "silago_dpu_comparison_with_constant", strlen("silago_dpu_comparison_with_constant")) == 0)
				{
					instr_mode_str = "silago_dpu_comparison_with_constant";
				}
				instr->modeValue = _pf_lib.get_mode(instr_mode_str);
				instr->executionCycle = _pf_lib.get_exec_cycle(instr_mode_str);
				instr->reductive = _pf_lib.get_reductive(instr_mode_str);

				CHECK_GE(instr->modeValue, 0);
				CHECK_GE(instr->executionCycle, 0);

				cidfg::DpuInstrVertex iv;
				iv.coord = v->coord;
				iv.instr = instr;
				int parent_id;
				int child_index;
				g_.get_parent(v->id, parent_id, child_index);

				int id_iv = g_.add_vertex(iv, parent_id, child_index);
				for (auto &e : g_.get_edges())
				{
					if (e->src_id == v->id)
					{
						e->src_id = id_iv;
					}
					if (e->dest_id == v->id)
					{
						if (g_.get_vertex(e->src_id)->vertex_type == cidfg::Vertex::CONST_VERTEX || g_.get_vertex(e->src_id)->vertex_type == cidfg::Vertex::VAR_VERTEX)
						{
							remove_vertex_id.insert(e->src_id);
						}
						else
						{
							e->dest_id = id_iv;
						}
					}
				}

				remove_vertex_id.insert(v->id);

				std::unordered_map<int, int> in_port_map;
				std::unordered_map<int, int> out_port_map;
				int i = 0;
				while (true)
				{
					int port = _pf_lib.get_in_port(instr_mode_str, i);
					if (port < 0)
					{
						break;
					}
					in_port_map[i] = port;
					i++;
				}
				i = 0;
				while (true)
				{
					int port = _pf_lib.get_out_port(instr_mode_str, i);
					if (port < 0)
					{
						break;
					}
					out_port_map[i] = port;
					i++;
				}

				// Any in-edge or out-edge which is not in those maps should be treated
				// as internal register argument transfer, the edge should be turn into
				// dependency edge.
				for (auto &e : g_.get_edges())
				{
					if (e->edge_type == cidfg::Edge::DEPENDENCY)
					{
						continue;
					}
					if (e->src_id == id_iv)
					{
						if (out_port_map.find(e->src_port) == out_port_map.end())
						{
							e->edge_type = cidfg::Edge::DEPENDENCY;
							e->src_port = 100;
							e->d_lo = 1;
							e->d_hi = INT_MAX;
						}
					}
					else if (e->dest_id == id_iv)
					{
						if (in_port_map.find(e->dest_port) == in_port_map.end())
						{
							e->edge_type = cidfg::Edge::DEPENDENCY;
							e->dest_port = 100;
							e->d_lo = 1;
							e->d_hi = INT_MAX;
						}
					}
				}

				for (auto &e : g_.get_edges())
				{
					if (e->dest_id == id_iv)
					{
						if (in_port_map.find(e->dest_port) != in_port_map.end())
						{
							e->dest_port = in_port_map[e->dest_port];
						}
					}
					if (e->src_id == id_iv)
					{
						if (out_port_map.find(e->src_port) != out_port_map.end())
						{
							e->src_port = out_port_map[e->src_port];
						}
					}
				}
			}
		}
	}

	for (auto &id : remove_vertex_id)
	{
		g_.del_vertex(id);
	}

	// try to determin whether to use fixedpoint mode for DPUInstruction.
	// Basic idea: first construct a bi-directed graph with only RD/WR/DpuInstruction/Source/Sink nodes. Then find each
	// connected components. Assign each connected area fixed point mode if any of the source node stores fixedpoint data.
	typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS> UGraph;
	std::unordered_map<int, int> id_map_cidfg2ugraph;
	std::unordered_map<int, int> id_map_ugraph2cidfg;

	vector<int> vec_vertex;
	for (auto &v : g_.get_vertices())
	{
		if (v->vertex_type == cidfg::Vertex::SRAM_VAR_VERTEX ||
				v->vertex_type == cidfg::Vertex::REG_VAR_VERTEX ||
				v->vertex_type == cidfg::Vertex::READ_AND_INDEX_VERTEX ||
				v->vertex_type == cidfg::Vertex::WRITE_AND_INDEX_VERTEX ||
				v->vertex_type == cidfg::Vertex::DPU_INSTR_VERTEX)
		{
			vec_vertex.push_back(v->id);
		}
	}

	int i = 0;
	for (auto &id : vec_vertex)
	{
		id_map_cidfg2ugraph[id] = i;
		id_map_ugraph2cidfg[i] = id;
		i++;
	}

	UGraph ug1(vec_vertex.size());
	for (auto &e : g_.get_edges())
	{
		if (id_map_cidfg2ugraph.find(e->src_id) != id_map_cidfg2ugraph.end() && id_map_cidfg2ugraph.find(e->dest_id) != id_map_cidfg2ugraph.end())
		{
			add_edge(id_map_cidfg2ugraph[e->src_id], id_map_cidfg2ugraph[e->dest_id], ug1);
			add_edge(id_map_cidfg2ugraph[e->dest_id], id_map_cidfg2ugraph[e->src_id], ug1);
		}
	}
	std::vector<int> component(boost::num_vertices(ug1));
	int num_components = boost::connected_components(ug1, &component[0]);

	vector<bool> vec_mode(num_components, false);
	for (int i = 0; i < num_components; i++)
	{
		for (int j = 0; j < vec_vertex.size(); j++)
		{
			if (component[j] == i)
			{
				if (g_.get_vertex(id_map_ugraph2cidfg[j])->vertex_type == cidfg::Vertex::SRAM_VAR_VERTEX)
				{
					cidfg::SramVarVertex *v = static_cast<cidfg::SramVarVertex *>(g_.get_vertex(id_map_ugraph2cidfg[j]));
					if (v->value_type == cidfg::SourceVertex::FLOAT)
					{
						vec_mode[i] = true;
						break;
					}
				}
				else if (g_.get_vertex(id_map_ugraph2cidfg[j])->vertex_type == cidfg::Vertex::REG_VAR_VERTEX)
				{
					cidfg::RegVarVertex *v = static_cast<cidfg::RegVarVertex *>(g_.get_vertex(id_map_ugraph2cidfg[j]));
					if (v->value_type == cidfg::SourceVertex::FLOAT)
					{

						vec_mode[i] = true;
						break;
					}
				}
			}
		}
	}

	for (int i = 0; i < vec_vertex.size(); i++)
	{
		if (g_.get_vertex(id_map_ugraph2cidfg[i])->vertex_type == cidfg::Vertex::DPU_INSTR_VERTEX)
		{
			cidfg::DpuInstrVertex *v = static_cast<cidfg::DpuInstrVertex *>(g_.get_vertex(id_map_ugraph2cidfg[i]));
			static_cast<BIR::DPUInstruction *>(v->instr)->fixedPointMode = vec_mode[component[i]];
		}
	}
}
void ComputationVertexConverter::convert_raccu_vertex(cidfg::CidfgGraph &g_, vector<int> vertex_id_list_, std::map<string, int> raccu_reg_id_, std::unordered_map<string, string> vt_)
{
	for (auto &vid : vertex_id_list_)
	{
		cidfg::Vertex *v = g_.get_vertex(vid);
		if (v->vertex_type == cidfg::Vertex::LOOP_VERTEX || v->vertex_type == cidfg::Vertex::GENERAL_HIERARCHICAL_VERTEX)
		{
			cidfg::HierarchicalVertex *vv = static_cast<cidfg::HierarchicalVertex *>(v);
			if (vv->children.size() > 0)
			{
				for (auto vid : vv->children[0])
				{
					cidfg::Vertex *v = g_.get_vertex(vid);
					if (v->vertex_type == cidfg::Vertex::VAR_VERTEX)
					{
						cidfg::VarVertex *vv = static_cast<cidfg::VarVertex *>(v);
						if (!vv->is_iterator)
						{
							vv->var_name = vt_[vv->coord.to_str() + vv->var_name];
						}
					}
					else if (v->is_sink())
					{
						cidfg::SinkVertex *vv = static_cast<cidfg::SinkVertex *>(v);
						if (vt_.find(vv->var_name) != vt_.end())
						{
							for (auto &e : g_.get_edges())
							{
								if (e->dest_id == vv->id)
								{
									e->var_name = vt_[vv->coord.to_str() + vv->var_name];
								}
							}
						}
					}
				}
				std::map<string, int> raccu_reg_id_0 = raccu_reg_id_;
				// LOG(DEBUG) << "------------------------------------------";
				// for (auto &n : vt_)
				// {
				// 	cout << n.first << " -> " << n.second << endl;
				// }
				// LOG(DEBUG) << "------------------------------------------";
				pre_fill_all_known_edges(g_, vv->children[0], vt_);
				vector<int> var_list = find_all_raccu_var_edges(g_, vv->children[0]);
				raccu_reg_id_0 = allocate_raccu_reg(g_, var_list, raccu_reg_id_0, vt_);
				convert_raccu_vertex(g_, vv->children[0], raccu_reg_id_0, vt_);
			}
		}
		else if (v->vertex_type == cidfg::Vertex::BRANCH_VERTEX)
		{
			cidfg::HierarchicalVertex *vv = static_cast<cidfg::HierarchicalVertex *>(v);
			if (vv->children.size() > 0)
			{
				for (auto vid : vv->children[0])
				{
					cidfg::Vertex *v = g_.get_vertex(vid);
					if (v->vertex_type == cidfg::Vertex::VAR_VERTEX)
					{
						cidfg::VarVertex *vv = static_cast<cidfg::VarVertex *>(v);
						vv->var_name = vt_[vv->coord.to_str() + vv->var_name];
					}
					else if (v->is_sink())
					{
						cidfg::SinkVertex *vv = static_cast<cidfg::SinkVertex *>(v);
						if (vt_.find(vv->var_name) != vt_.end())
						{
							for (auto &e : g_.get_edges())
							{
								if (e->dest_id == vv->id)
								{
									e->var_name = vt_[vv->coord.to_str() + vv->var_name];
								}
							}
						}
					}
				}
				if (vv->children.size() > 1)
				{
					for (auto vid : vv->children[1])
					{
						cidfg::Vertex *v = g_.get_vertex(vid);
						if (v->vertex_type == cidfg::Vertex::VAR_VERTEX)
						{
							cidfg::VarVertex *vv = static_cast<cidfg::VarVertex *>(v);
							vv->var_name = vt_[vv->coord.to_str() + vv->var_name];
						}
						else if (v->is_sink())
						{
							cidfg::SinkVertex *vv = static_cast<cidfg::SinkVertex *>(v);
							if (vt_.find(vv->var_name) != vt_.end())
							{
								for (auto &e : g_.get_edges())
								{
									if (e->dest_id == vv->id)
									{
										e->var_name = vt_[vv->coord.to_str() + vv->var_name];
									}
								}
							}
						}
					}
				}
				std::map<string, int> raccu_reg_id_0 = raccu_reg_id_;
				std::map<string, int> raccu_reg_id_1 = raccu_reg_id_;
				vector<int> var_list;
				pre_fill_all_known_edges(g_, vv->children[0], vt_);
				var_list = find_all_raccu_var_edges(g_, vv->children[0]);
				raccu_reg_id_0 = allocate_raccu_reg(g_, var_list, raccu_reg_id_0, vt_);
				if (vv->children.size() > 1)
				{
					pre_fill_all_known_edges(g_, vv->children[1], vt_);
					var_list = find_all_raccu_var_edges(g_, vv->children[1]);
					raccu_reg_id_1 = allocate_raccu_reg(g_, var_list, raccu_reg_id_1, vt_);
				}
				convert_raccu_vertex(g_, vv->children[0], raccu_reg_id_0, vt_);
				if (vv->children.size() > 1)
				{
					convert_raccu_vertex(g_, vv->children[1], raccu_reg_id_1, vt_);
				}
			}
		}
	}
}
void ComputationVertexConverter::pre_fill_all_known_edges(cidfg::CidfgGraph &g_, vector<int> vector_id_list_, std::unordered_map<string, string> &var_reg_map_)
{
	for (auto &e : g_.get_edges())
	{
		if (std::find(vector_id_list_.begin(), vector_id_list_.end(), e->src_id) != vector_id_list_.end())
		{
			if (strncmp(e->var_name.c_str(), "IT_", strlen("IT_")) != 0 && strncmp(e->var_name.c_str(), "RR_", strlen("RR_")) != 0)
			{
				if (var_reg_map_.find(g_.get_vertex(e->src_id)->coord.to_str() + e->var_name) != var_reg_map_.end())
				{
					e->var_name = var_reg_map_[g_.get_vertex(e->src_id)->coord.to_str() + e->var_name];
				}
			}
		}
	}
}

vector<int> ComputationVertexConverter::find_all_raccu_var_edges(cidfg::CidfgGraph &g_, vector<int> vertex_id_list_)
{
	vector<int> raccu_edge_list;
	std::set<int> raccu_vertex_set;
	for (auto &vid : vertex_id_list_)
	{
		if (g_.get_vertex(vid)->vertex_type == cidfg::Vertex::COMPUTATION_VERTEX)
		{
			if (static_cast<cidfg::ComputationVertex *>(g_.get_vertex(vid))->is_on_dpu == false)
			{
				raccu_vertex_set.insert(vid);
			}
		}
	}
	for (auto &e : g_.get_edges())
	{
		if (e->edge_type == cidfg::Edge::DEPENDENCY)
		{
			continue;
		}
		if (raccu_vertex_set.find(e->src_id) != raccu_vertex_set.end() && ((strncmp(e->var_name.c_str(), "RR_", strlen("RR_")) != 0) && (strncmp(e->var_name.c_str(), "IT_", strlen("IT_")) != 0)))
		{
			raccu_edge_list.push_back(e->id);
		}
	}
	return raccu_edge_list;
}
std::map<string, int> ComputationVertexConverter::allocate_raccu_reg(cidfg::CidfgGraph &g_, vector<int> edge_list_, std::map<string, int> curr_reg_id_, std::unordered_map<string, string> &vt_)
{
	vector<int> src_vertices;
	vector<int> dest_vertices;
	set<int> involved_vertices;
	for (auto &e : edge_list_)
	{
		src_vertices.push_back(g_.get_edge(e)->src_id);
		dest_vertices.push_back(g_.get_edge(e)->dest_id);
		involved_vertices.insert(g_.get_edge(e)->src_id);
		involved_vertices.insert(g_.get_edge(e)->dest_id);
	}

	typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS> UGraph;
	std::unordered_map<int, int> id_map_cidfg2ugraph;
	std::unordered_map<int, int> id_map_ugraph2cidfg;

	vector<int> vec_vertex;
	for (auto &v : involved_vertices)
	{
		vec_vertex.push_back(v);
	}

	int i = 0;
	for (auto &id : vec_vertex)
	{
		id_map_cidfg2ugraph[id] = i;
		id_map_ugraph2cidfg[i] = id;
		i++;
	}

	UGraph ug1(vec_vertex.size());
	for (auto &e : g_.get_edges())
	{
		if (id_map_cidfg2ugraph.find(e->src_id) != id_map_cidfg2ugraph.end() && id_map_cidfg2ugraph.find(e->dest_id) != id_map_cidfg2ugraph.end())
		{
			add_edge(id_map_cidfg2ugraph[e->src_id], id_map_cidfg2ugraph[e->dest_id], ug1);
			add_edge(id_map_cidfg2ugraph[e->dest_id], id_map_cidfg2ugraph[e->src_id], ug1);
		}
	}
	std::vector<int> component(boost::num_vertices(ug1));
	int num_components = boost::connected_components(ug1, &component[0]);

	std::map<string, int> new_curr_reg_id = curr_reg_id_;
	for (int c = 0; c < num_components; c++)
	{
		vector<int> new_edge_list;
		for (int m = 0; m < src_vertices.size(); m++)
		{
			if (component[id_map_cidfg2ugraph[src_vertices[m]]] == c && component[id_map_cidfg2ugraph[dest_vertices[m]]] == c)
			{
				new_edge_list.push_back(edge_list_[m]);
			}
		}
		new_curr_reg_id = allocate_raccu_reg_for_connected_graph(g_, new_edge_list, new_curr_reg_id, vt_);
	}

	return new_curr_reg_id;
}
std::map<string, int> ComputationVertexConverter::allocate_raccu_reg_for_connected_graph(cidfg::CidfgGraph &g_, vector<int> edge_list_, std::map<string, int> curr_reg_id_, std::unordered_map<string, string> &vt_)
{

	// find sibling edges. sibling edges are edges that has the same source node.
	std::map<int, int> edge_family_record;
	int edge_family_counter = 0;
	for (int i = 0; i < edge_list_.size(); i++)
	{
		for (int j = i; j < edge_list_.size(); j++)
		{
			if (g_.get_edge(edge_list_[i])->src_id == g_.get_edge(edge_list_[j])->src_id)
			{
				if (edge_family_record.find(edge_list_[i]) != edge_family_record.end())
				{
					edge_family_record[edge_list_[j]] = edge_family_record[edge_list_[i]];
				}
				else if (edge_family_record.find(edge_list_[j]) != edge_family_record.end())
				{
					edge_family_record[edge_list_[i]] = edge_family_record[edge_list_[j]];
				}
				else
				{
					edge_family_record[edge_list_[j]] = edge_family_counter;
					edge_family_record[edge_list_[i]] = edge_family_counter;
					edge_family_counter++;
				}
			}
		}
	}

	std::unordered_map<int, vector<int>> var_map;
	for (auto &eid : edge_list_)
	{
		vector<int> vec(2);
		var_map[eid] = vec;
	}
	// Build DAG
	typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, int> CustomGraph;
	CustomGraph g0;
	set<int> vertex_set;
	for (auto &eid : edge_list_)
	{
		cidfg::Edge *e = g_.get_edge(eid);
		vertex_set.insert(e->src_id);
	}
	std::unordered_map<int, boost::graph_traits<CustomGraph>::vertex_descriptor> graph_vertex_map;
	for (auto &vid : vertex_set)
	{
		boost::graph_traits<CustomGraph>::vertex_descriptor vd = boost::add_vertex(vid, g0);
		graph_vertex_map[vid] = vd;
	}
	for (auto &eid : edge_list_)
	{
		cidfg::Edge *e = g_.get_edge(eid);
		if (vertex_set.find(e->dest_id) != vertex_set.end())
		{
			boost::add_edge(graph_vertex_map[e->src_id], graph_vertex_map[e->dest_id], g0);
		}
	}

	// ASAP schedule, equivalent to topological sort
	std::vector<boost::graph_traits<CustomGraph>::vertex_descriptor> c;
	std::unordered_map<int, int> timing_map;
	topological_sort(g0, std::back_inserter(c));
	int index = 0;
	for (auto ii = c.rbegin(); ii != c.rend(); ++ii)
	{
		timing_map[g0[*ii]] = index;
		index++;
	}

	// Fill the edge interval
	for (auto &eid : edge_list_)
	{
		cidfg::Edge *e = g_.get_edge(eid);
		var_map[eid][0] = timing_map[e->src_id] + 1;
		if (timing_map.find(e->dest_id) != timing_map.end())
		{
			var_map[eid][1] = timing_map[e->dest_id];
		}
		else
		{
			var_map[eid][1] = INT_MAX;
		}
	}

	// Adjust the edge interval according to the siblings. Build the family map.
	std::unordered_map<int, vector<int>> family_map;
	for (int fid = 0; fid < edge_family_counter; fid++)
	{
		vector<int> vec(2);
		vec[0] = -1;
		vec[1] = -1;
		family_map[fid] = vec;
	}
	for (auto &eid : edge_list_)
	{
		int fid = edge_family_record[eid];
		if (family_map[fid][0] < 0 && family_map[fid][1] < 0)
		{
			family_map[fid][0] = var_map[eid][0];
			family_map[fid][1] = var_map[eid][1];
		}
		else
		{
			if (family_map[fid][0] > var_map[eid][0])
			{
				family_map[fid][0] = var_map[eid][0];
			}
			if (family_map[fid][1] < var_map[eid][1])
			{
				family_map[fid][1] = var_map[eid][1];
			}
		}
	}

	// Build conflict graph
	typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, int> ConflictGraph;
	ConflictGraph g1(edge_family_counter);
	for (int i = 0; i < edge_family_counter; i++)
	{
		for (int j = i + 1; j < edge_family_counter; j++)
		{
			if ((family_map[i][0] >= family_map[j][0] && family_map[i][0] <= family_map[j][1]) ||
					(family_map[i][1] >= family_map[j][0] && family_map[i][1] <= family_map[j][1]))
			{
				boost::add_edge(i, j, g1);
				boost::add_edge(j, i, g1);
			}
			else // if they belongs to different DRRA cell, they are conflict
			{
				string i_coord = "";
				string j_coord = "";
				for (auto &edge_loc : edge_family_record)
				{
					if (edge_loc.second == i)
					{
						i_coord = g_.get_vertex(g_.get_edge(edge_loc.first)->src_id)->coord.to_str();
						break;
					}
				}
				for (auto &edge_loc : edge_family_record)
				{
					if (edge_loc.second == j)
					{
						j_coord = g_.get_vertex(g_.get_edge(edge_loc.first)->src_id)->coord.to_str();
						break;
					}
				}
				if (i_coord != j_coord)
				{
					boost::add_edge(i, j, g1);
					boost::add_edge(j, i, g1);
				}
			}
		}
	}

	// Coloring the conflict graph
	std::vector<int> color_vec(num_vertices(g1));
	iterator_property_map<int *, property_map<ConflictGraph, vertex_index_t>::const_type> color(&color_vec.front(), get(vertex_index, g1));
	int num_colors = sequential_vertex_coloring(g1, color);

	// Update raccu register binding by change the edge var_name, Update var table
	map<string, int> curr_reg_id_copy = curr_reg_id_;
	map<string, int> curr_reg_id_max_copy = curr_reg_id_;

	map<int, int> new_color_vector;
	for (auto ii = c.rbegin(); ii != c.rend(); ++ii)
	{
		int select_edge_id = -1;
		for (auto &eid : edge_list_)
		{
			if (g_.get_edge(eid)->src_id == g0[*ii])
			{
				if (new_color_vector.find(edge_family_record[eid]) == new_color_vector.end())
				{
					new_color_vector[color_vec[edge_family_record[eid]]] = curr_reg_id_copy[g_.get_vertex(g_.get_edge(eid)->src_id)->coord.to_str()];
					select_edge_id = eid;
				}
			}
		}
		if (select_edge_id >= 0)
		{
			curr_reg_id_copy[g_.get_vertex(g_.get_edge(select_edge_id)->src_id)->coord.to_str()]++;
		}
	}

	for (auto ii = c.rbegin(); ii != c.rend(); ++ii)
	{
		for (auto &eid : edge_list_)
		{
			if (g_.get_edge(eid)->src_id == g0[*ii])
			{
				LOG(DEBUG) << "Allocate RR_" << to_string(new_color_vector[color_vec[edge_family_record[eid]]]) << " for " << g_.get_vertex(g_.get_edge(eid)->src_id)->coord.to_str() + g_.get_edge(eid)->var_name;
				CHECK_LT(new_color_vector[color_vec[edge_family_record[eid]]], _raccu_reg_file_depth)
						<< "Too much RACCU registers required!";

				vt_[g_.get_vertex(g_.get_edge(eid)->src_id)->coord.to_str() + g_.get_edge(eid)->var_name] = "RR_" + to_string(new_color_vector[color_vec[edge_family_record[eid]]]);
				//				LOG(DEBUG) << "ADD: " << g_.get_vertex(g_.get_edge(eid)->src_id)->coord.to_str() + g_.get_edge(eid)->var_name << " -> "
				//									 << "RR_" + to_string(new_color_vector[color_vec[edge_family_record[eid]]]);
				g_.get_edge(eid)->var_name = "RR_" + to_string(new_color_vector[color_vec[edge_family_record[eid]]]);
			}
		}
	}

	return curr_reg_id_copy;
}

} // namespace codegen
} // namespace vesyla
