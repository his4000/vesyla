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

#include "ManasGenenator.hpp"

namespace vesyla
{
namespace filegen
{
void ManasGenerator::generate(map<string, vector<Instruction *>> instr_lists_, string filename_)
{
	ofstream ofs(filename_, ofstream::out);
	for (auto &e : instr_lists_)
	{
		if (e.second.size() <= 0)
		{
			continue;
		}
		string cell_str = e.first;
		vector<string> coord;
		boost::split(coord, cell_str, boost::is_any_of("_"));
		string str = "CELL <" + coord[0] + "," + coord[1] + ">";
		Common::write_line(ofs, str);

		vector<Instruction *> instr_list = e.second;
		for (auto &instr : instr_list)
		{
			switch (instr->kind())
			{
			case bktRefi1Instruction:
			{
				str = create_refi1_instr(static_cast<Refi1Instruction *>(instr));
				Common::write_line(ofs, str);
				break;
			}
			case bktRefi2Instruction:
			{
				str = create_refi2_instr(static_cast<Refi2Instruction *>(instr));
				Common::write_line(ofs, str);
				break;
			}
			case bktRefi3Instruction:
			{
				str = create_refi3_instr(static_cast<Refi3Instruction *>(instr));
				Common::write_line(ofs, str);
				break;
			}
			case bktDPUInstruction:
			{
				str = create_dpu_instr(static_cast<DPUInstruction *>(instr));
				Common::write_line(ofs, str);
				break;
			}
			case bktSWBInstruction:
			{
				str = create_swb_instr(static_cast<SWBInstruction *>(instr));
				Common::write_line(ofs, str);
				break;
			}
			case bktJumpInstruction:
			{
				str = create_jump_instr(static_cast<JumpInstruction *>(instr));
				Common::write_line(ofs, str);
				break;
			}
			case bktWaitInstruction:
			{
				str = create_wait_instr(static_cast<WaitInstruction *>(instr));
				Common::write_line(ofs, str);
				break;
			}
			case bktLoopHeaderInstruction:
			{
				str = create_looph_instr(static_cast<LoopHeaderInstruction *>(instr));
				Common::write_line(ofs, str);
				break;
			}
			case bktLoopTailInstruction:
			{
				str = create_loopt_instr(static_cast<LoopTailInstruction *>(instr));
				Common::write_line(ofs, str);
				break;
			}
			case bktRACCUInstruction:
			{
				str = create_raccu_instr(static_cast<RACCUInstruction *>(instr));
				Common::write_line(ofs, str);
				break;
			}
			case bktBranchInstruction:
			{
				str = create_branch_instr(static_cast<BranchInstruction *>(instr));
				Common::write_line(ofs, str);
				break;
			}
			case bktRouteInstruction:
			{
				str = create_route_instr(static_cast<RouteInstruction *>(instr));
				Common::write_line(ofs, str);
				break;
			}
			case bktSRAMInstruction:
			{
				str = create_sram_instr(static_cast<SRAMInstruction *>(instr));
				Common::write_line(ofs, str);
				break;
			}
			default:
			{
				break;
			}
			}
		}

		Common::write_line(ofs, "");
	}
}

string ManasGenerator::create_refi1_instr(Refi1Instruction *refi1Inst_)
{
	string instStr = "REFI_1(";
	instStr += to_string(refi1Inst_->portNo) + ", ";												 // reg_file_port
	instStr += to_string(refi1Inst_->numberOfSubseqInst) + ", ";						 // subseq_instrs
	instStr += to_string((int)!refi1Inst_->startAddress.isStatic) + ", ";		 // start_addrs_sd
	instStr += to_string(refi1Inst_->startAddress.value) + ", ";						 // start_addrs
	instStr += to_string((int)!refi1Inst_->numberOfAddress.isStatic) + ", "; // no_of_addrs_sd
	instStr += to_string(refi1Inst_->numberOfAddress.value) + ", ";					 // no_of_addrs
	instStr += to_string((int)!refi1Inst_->initialDelay.isStatic) + ", ";		 // initial_delay_sd
	instStr += to_string(refi1Inst_->initialDelay.value);										 // initial_delay
	instStr += ")";
	return instStr;
}

string ManasGenerator::create_refi2_instr(Refi2Instruction *refi2Inst_)
{
	string instStr = "REFI_2(";
	instStr += to_string((int)!refi2Inst_->stepValue.isStatic) + ", ";					// step_val_sd
	instStr += to_string(refi2Inst_->stepValue.value) + ", ";										// step_val
	instStr += to_string(refi2Inst_->stepValue.value >= 0 ? 0 : 1) + ", ";			// step_val_sign
	instStr += to_string((int)!refi2Inst_->middleDelay.isStatic) + ", ";				// refi_middle_delay_sd
	instStr += to_string(refi2Inst_->middleDelay.value) + ", ";									// refi_middle_delay
	instStr += to_string((int)!refi2Inst_->numberOfRepetition.isStatic) + ", "; // no_of_reps_sd
	instStr += to_string(refi2Inst_->numberOfRepetition.value) + ", ";					// no_of_reps
	instStr += to_string(refi2Inst_->repetitionOffset) + ")";										// rpt_step_value
	return instStr;
}

string ManasGenerator::create_refi3_instr(Refi3Instruction *refi3Inst_)
{
	string instStr = "REFI_3(";
	instStr += to_string((int)!refi3Inst_->repetitionDelay.isStatic) + ", ";	// rpt_delay_sd
	instStr += to_string(refi3Inst_->repetitionDelay.value) + ", ";						// rpt_delay
	instStr += to_string(refi3Inst_->addressMode == amLinear ? 0 : 1) + ", "; // mode
	instStr += to_string(3) + ", ";																						// outp_cntrl
	instStr += to_string(0) + ", ";																						// fft stage
	instStr += to_string(refi3Inst_->middleDelayExt) + ", ";									// refi_middle_delay_ext
	instStr += to_string(refi3Inst_->numberOfRepetitionExt) + ", ";						// no_of_rpt_ext
	instStr += to_string(refi3Inst_->repetitionOffsetExt) + ", ";							// rpt_step_value_ext
	instStr += to_string(0) + ", ";																						// fft select
	instStr += to_string(!refi3Inst_->isDimarchMode ? 0 : 1) + ", ";					// DiMAarch mode
	instStr += to_string(!refi3Inst_->en_compression ? 0 : 1) + ")";					// Compression enable
	return instStr;
}

string ManasGenerator::create_raccu_instr(RACCUInstruction *raccuInst_)
{
	int raccuMode;

	// Obtained from the FABRIC implementation
	switch (raccuInst_->raccuMode)
	{
	case rmAdd:
		raccuMode = 3;
		break;
	case rmAddWithLoopIndex:
		raccuMode = 7;
		break;
	case rmShiftLeft:
		raccuMode = 6;
		break;
	case rmLoopHeader:
		raccuMode = 1;
		break;
	case rmLoopTail:
		raccuMode = 2;
		break;
	case rmShiftRight:
		raccuMode = 5;
		break;
	case rmSub:
		raccuMode = 4;
		break;
	default:
		raccuMode = 0;
		break;
	}
	string instStr = "RACCU(";
	instStr += to_string(raccuMode) + ", ";														// raccu_mode
	instStr += to_string((int)!raccuInst_->operand1.isStatic) + ", "; // raccu_op1_sd
	instStr += to_string(raccuInst_->operand1.value) + ", ";					// raccu_op1
	instStr += to_string((int)!raccuInst_->operand2.isStatic) + ", "; // raccu_op2_sd
	instStr += to_string(raccuInst_->operand2.value) + ", ";					// raccu_op2
	instStr += to_string(raccuInst_->resultAddress) + ")";						// raccu_result_addrs
	return instStr;
}

string ManasGenerator::create_jump_instr(BIR::JumpInstruction *jumpInst_)
{
	string instStr = "JUMP(";
	instStr += to_string(jumpInst_->jump_pc) + ")"; // jump address, calculated in synchronization
	return instStr;
}

// This method dumps a Loop Header instruction to the VHDL file.
string ManasGenerator::create_looph_instr(BIR::LoopHeaderInstruction *loopHeaderInst_)
{
	string instStr = "LOOPH(";
	instStr += to_string(loopHeaderInst_->loopId) + ", ";						// index_raccu_addr
	instStr += to_string(loopHeaderInst_->startValue) + ", ";				// index_start
	instStr += to_string(loopHeaderInst_->isStatic ? 0 : 1) + ", "; // iter_no_sd
	instStr += to_string(loopHeaderInst_->iterationNumber) + ")";		// iter_no
	return instStr;
}

// This method dumps a Loop Tail instruction to the VHDL file.
string ManasGenerator::create_loopt_instr(LoopTailInstruction *loopTailInst_)
{
	string instStr = "LOOPT(";
	instStr += to_string(loopTailInst_->stepValue) + ", "; // index_step
	instStr += to_string(loopTailInst_->gotoPC) + ", ";		 // pc_togo
	instStr += to_string(loopTailInst_->loopId) + ")";		 // index_raccu_addr
	return instStr;
}

string ManasGenerator::create_branch_instr(BIR::BranchInstruction *branchInst_)
{
	string instStr = "BRANCH(";
	instStr += to_string(branchInst_->modeValue) + ", "; // branch mode
	instStr += to_string(branchInst_->false_pc) + ")";	 // branch false address, calculated while synchronization
	return instStr;
}

string ManasGenerator::create_dpu_instr(BIR::DPUInstruction *dpuInst_)
{
	string instStr = "DPU(";

	int dpuControl = 0;

	if (dpuInst_->saturation)
		dpuControl |= 2;

	if (dpuInst_->fixedPointMode)
		dpuControl |= 1;

	instStr += to_string(dpuInst_->modeValue) + ", "; // dpu_mode
	instStr += to_string(dpuControl) + ", ";					// dpu_saturation
	instStr += to_string(3) + ", ";										// dpu_out_1
	instStr += to_string(3) + ", ";										// dpu_out_2
	instStr += to_string(1) + ", ";										// dpu_acc_clear_rst, we always set it to 1
	instStr += to_string(0) + ", ";										// dpu_acc_clear_sd

	int dpu_acc_clear;
	if (dpuInst_->mode == "silago_dpu_add_with_constant" || dpuInst_->mode == "silago_dpu_load_constant" || dpuInst_->mode == "silago_dpu_comparison_with_constant" || dpuInst_->mode == "silago_dpu_butterfly")
	{
		if (dpuInst_->fixedPointMode)
		{
			dpuInst_->constantValue = (int)(dpuInst_->float_imm_data * (1 << 7));
		}
		else
		{
			dpuInst_->constantValue = dpuInst_->int_imm_data;
		}
		if (dpuInst_->parameter_type == BIR::BIREnumerations::dptImmediate)
		{
			dpu_acc_clear = dpuInst_->constantValue & 0x7f;
		}
		else
		{
			dpu_acc_clear = dpuInst_->constantValue | 0x80;
		}
	}
	else
	{
		if (dpuInst_->parameter_type == BIR::BIREnumerations::dptImmediate)
		{
			dpu_acc_clear = dpuInst_->repetition & 0x7f;
		}
		else
		{
			dpu_acc_clear = dpuInst_->repetition | 0x80;
		}
	}

	instStr += to_string(dpu_acc_clear) + ", "; // dpu_acc_clear

	string ioChange;
	switch (dpuInst_->ioChange)
	{
	case dicIn1Negation:
		ioChange = "1";
		break;
	case dicIn2Negation:
		ioChange = "2";
		break;
	case dicAbsoluteOfOutput:
		ioChange = "3";
		break;
	default:
		ioChange = "0";
		break;
	}

	instStr += ioChange + ")"; // process_inout

	return instStr;
}

string ManasGenerator::create_wait_instr(BIR::WaitInstruction *waitInst_)
{
	string instStr = "DELAY(";
	instStr += to_string((int)!waitInst_->numberOfCycles.isStatic) + ", "; // del_cycles_sd
	instStr += to_string(waitInst_->numberOfCycles.value) + ")";					 // del_cycles
	return instStr;
}

string ManasGenerator::create_route_instr(BIR::RouteInstruction *routeInst_)
{
	string instStr;
	instStr = "ROUTE(";
	instStr += to_string(routeInst_->sourceCoordinate.row) + ", ";
	instStr += to_string(routeInst_->sourceCoordinate.column) + ", ";
	instStr += to_string(routeInst_->destCoordinate.row) + ", ";
	instStr += to_string(routeInst_->destCoordinate.column) + ", ";
	instStr += to_string(routeInst_->drraSelect) + ", ";
	instStr += to_string(0) + ", "; // Union flag
	instStr += to_string(0) + ", "; // Union port
	instStr += routeInst_->isFromSource ? "'1'" : "'0'";
	instStr += ")";
	return instStr;
}

string ManasGenerator::create_swb_instr(BIR::SWBInstruction *swbInst_)
{
	ComponentInfo componentInfo;
	string instStr = "SWB(";
	componentInfo = swbInst_->source;
	instStr += (componentInfo.type == ctDPU ? "LATA" : "REFI");
	instStr += ", ";																																												 // from_block
	instStr += to_string(2 * componentInfo.cellCoordinate.column + componentInfo.cellCoordinate.row) + ", "; // from_address (2*j+i)
	instStr += to_string(componentInfo.portNo) + ", ";																											 // from_port
	componentInfo = swbInst_->destination;
	instStr += (componentInfo.type == ctDPU ? "LATA" : "REFI");
	instStr += ", ";																																												 // from_block
	instStr += to_string(2 * componentInfo.cellCoordinate.column + componentInfo.cellCoordinate.row) + ", "; // to_address (2*j+i)
	instStr += to_string(componentInfo.portNo) + ")";																												 // to_port
	return instStr;
}

string ManasGenerator::create_sram_instr(BIR::SRAMInstruction *sramInst_)
{
	string instStr;

	instStr = "SRAM_";

	instStr += sramInst_->isRead ? "R" : "W";
	instStr += "(";

	instStr += to_string(0) + ", "; // mode
	instStr += to_string(sramInst_->initialAddress.value) + ", ";
	instStr += to_string(sramInst_->initialDelay.value) + ", ";
	instStr += to_string(sramInst_->loop1Iterations.value) + ", ";
	instStr += to_string(sramInst_->loop1Increment) + ", ";
	instStr += to_string(sramInst_->loop1Delay.value) + ", ";
	instStr += to_string(sramInst_->loop2Iterations.value) + ", ";
	instStr += to_string(sramInst_->loop2Increment) + ", ";
	instStr += to_string(sramInst_->loop2Delay.value) + ", ";

	instStr += sramInst_->initialAddress.isStatic ? "'0'" : "'1'";
	instStr += ", ";
	instStr += sramInst_->loop1Iterations.isStatic ? "'0'" : "'1'";
	instStr += ", ";
	instStr += sramInst_->loop2Iterations.isStatic ? "'0'" : "'1'";
	instStr += ", ";
	instStr += sramInst_->initialDelay.isStatic ? "'0'" : "'1'";
	instStr += ", ";
	instStr += sramInst_->loop1Delay.isStatic ? "'0'" : "'1'";
	instStr += ", ";
	instStr += sramInst_->loop2Delay.isStatic ? "'0'" : "'1'";
	instStr += ", ";
	instStr += "'0', '0');"; // TODO: sram_Loop1_increment_sd and sram_Loop2_increment_sd should be removed
	return instStr;
}

} // namespace filegen
} // namespace vesyla
