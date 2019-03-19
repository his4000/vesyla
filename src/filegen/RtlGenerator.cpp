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

#include "RtlGenerator.hpp"
namespace vesyla
{
namespace filegen
{
void RtlGenerator::generate(map<string, vector<BIR::Instruction *>> instr_lists_, codegen::StorageImage si_, string directory_)
{
	gen_fill_data(directory_ + "fill_data.sv");
	gen_macros(directory_ + "macros.svh");
	gen_name_mangling(directory_ + "name_mangling.svh");
	gen_test_util(directory_ + "test_util.svh");
	gen_test_util_files(directory_ + "test_util_files.f");
	gen_type_def(directory_ + "type_def.svh");
	gen_profiler(instr_lists_, si_, directory_ + "profiler.sv");
	gen_wave(instr_lists_, si_, directory_ + "wave.do");
	gen_testbench(instr_lists_, si_, directory_ + "testbench.vhd");
	gen_const_package(instr_lists_, si_, directory_ + "const_package.vhd");
	gen_run_cmd(directory_ + "run_cmd.do");
	gen_run_gui(directory_ + "run_gui.do");
}

void RtlGenerator::gen_fill_data(string filename_)
{
	string str = R"(
`include "macros.svh"
import name_mangling::*;
import test_util::*;

// A module declaration to fill the package data arrays with the corresponding FABRIC signals
module fill_data;
	generate
		for (genvar row = 0; row < ROWS; row++)
			for (genvar col = 0; col < COLUMNS; col++)
			begin
				// Correspondence: AGU_Wr_0 -> data_in_0; AGU_Wr_1 -> data_in_1; AGU_Rd_0 -> data_out_reg_0_left; AGU_Rd_1 -> data_out_reg_1_left;
				assign `currRegData.dataIn0 		= `regFileData(data_in_0);
				assign `currRegData.dataIn1 		= `regFileData(data_in_1);
				assign `currRegData.dataOut0 		= `regFileData(data_out_reg_0_left);
				assign `currRegData.dataOut1 		= `regFileData(data_out_reg_1_left);
				assign `currRegData.addrIn0 		= `aguAddress(Wr_0);
				assign `currRegData.addrIn1 		= `aguAddress(Wr_1);
				assign `currRegData.addrOut0 		= `aguAddress(Rd_0);
				assign `currRegData.addrOut1 		= `aguAddress(Rd_1);
				assign `currRegData.addrenIn0 		= `aguAddressEn(Wr_0);
				assign `currRegData.addrenIn1 		= `aguAddressEn(Wr_1);
				assign `currRegData.addrenOut0		= `aguAddressEn(Rd_0);
				assign `currRegData.addrenOut1		= `aguAddressEn(Rd_1);
				assign `currRegData.instStartIn0	= `fabricRegTop.AGU_Wr_0_instantiate.instr_start;

				assign `currSequencerData.pc 		= `fabricCell.seq_gen.pc;
				assign `currSequencerData.currInst 	= `fabricCell.seq_gen.instr;
			end
	endgenerate

	generate
		for (genvar row = 0; row < ROWS-1; row++)
			for (genvar col = 0; col < COLUMNS; col++)
			begin
				assign `currSramData.readEn 		= `sramReadEn;
				assign `currSramData.writeEn		= `sramWriteEn;
				assign `currSramData.writeAddress 	= `sramWriteAddress;
				assign `currSramData.data 			= `sramMemData;
			end
	endgenerate

	initial
		for (int row = 0; row < ROWS; row++)
			for (int col = 0; col < COLUMNS; col++)
				oldpcs[row][col] = -1;
endmodule
)";
	ofstream ofs(filename_, ofstream::out);
	ofs << str;
}

void RtlGenerator::gen_macros(string filename_)
{
	string str = R"(
`ifndef _MACROS_
`define _MACROS_

// Some macro definitions for simpler access
`define fabricCell 			DUT.MTRF_COLS[col].MTRF_ROWS[row].SILEGO_cell.MTRF_cell
`define fabricRegTop 		`fabricCell.reg_top
`define aguAddressEn(port)	`fabricRegTop.AGU_``port``_instantiate.addr_en
`define aguAddress(port)	`fabricRegTop.AGU_``port``_instantiate.addr_out
`define regFileData(port)	`fabricRegTop.RegisterFile.``port
`define dimarchDataIn 		`fabricRegTop.dimarch_data_in
`define currRegVariable		regFileVariables[row][col][address]
`define currRegData 		regData[row][col]
`define currSequencerData	sequencerData[row][col]
`define currInstruction		`currSequencerData.currInst
`define currFixedPointStatus regFileFixedPointStatus[row][col][address]

`define stile				DUT.DiMArch_COLS[col].DiMArch_ROWS[row].u_STILE
`define sramReadEn			`stile.SRAM_rw_r
`define sramWriteEn			`stile.SRAM_rw_w
`define sramWriteAddress	`stile.SRAM_rw_addrs_out_w
`define sramMemData			`stile.memory_out
`define currSramData		sramData[0][col]

`endif //_MACROS_
)";
	ofstream ofs(filename_, ofstream::out);
	ofs << str;
}

void RtlGenerator::gen_name_mangling(string filename_)
{
	string str = R"(
`ifndef _NAME_MANGLING_
`define _NAME_MANGLING_

import top_consts_types_package::*;

package name_mangling;

	parameter REG_FILE_DATA_WIDTH 		= top_consts_types_package::reg_file_data_width;
	parameter REG_FILE_ADDR_WIDTH 		= top_consts_types_package::reg_file_addr_width;
	parameter REG_FILE_DEPTH	  		= top_consts_types_package::reg_file_depth;
	parameter REG_FILE_MEM_DATA_WIDTH 	= top_consts_types_package::reg_file_mem_data_width;
	parameter ROWS 				  		= top_consts_types_package::rows;
	parameter COLUMNS 			  		= top_consts_types_package::columns;
	parameter INSTR_WIDTH		    	= top_consts_types_package::instr_width;
	parameter INSTR_CODE_RANGE_BASE 	= top_consts_types_package::instr_code_range_base;
	parameter INSTR_CODE_RANGE_END 		= top_consts_types_package::instr_code_range_end;
	parameter SRAM_WIDTH				= top_consts_types_package::sram_width;

	// DPU parameters
	parameter DPU_MODE_SEL_RANGE_BASE 			= top_consts_types_package::dpu_mode_sel_range_base;
	parameter DPU_MODE_SEL_RANGE_END 			= top_consts_types_package::dpu_mode_sel_range_end;
	// REFI1 parameters
	parameter NR_OF_REG_FILE_PORTS_RANGE_BASE 	= top_consts_types_package::nr_of_reg_file_ports_range_base;
	parameter NR_OF_REG_FILE_PORTS_RANGE_END 	= top_consts_types_package::nr_of_reg_file_ports_range_end;
	parameter STARTING_ADDRS_RANGE_BASE 		= top_consts_types_package::starting_addrs_range_base;
	parameter STARTING_ADDRS_RANGE_END 			= top_consts_types_package::starting_addrs_range_end;
	parameter NR_OF_ADDRS_RANGE_BASE 			= top_consts_types_package::nr_of_addrs_range_base;
	parameter NR_OF_ADDRS_RANGE_END 			= top_consts_types_package::nr_of_addrs_range_end;
	parameter INIT_DELAY_RANGE_BASE 			= top_consts_types_package::init_delay_range_base;
	parameter INIT_DELAY_RANGE_END 				= top_consts_types_package::init_delay_range_end;
	// REFI2 parameters
	parameter STEP_VALUE_RANGE_BASE 			= top_consts_types_package::step_value_range_base;
	parameter STEP_VALUE_RANGE_END 				= top_consts_types_package::step_value_range_end;
	parameter STEP_VALUE_SIGN_RANGE_BASE 		= top_consts_types_package::step_value_sign_range_base;
	parameter STEP_VALUE_SIGN_RANGE_END 		= top_consts_types_package::step_value_sign_range_end;
	parameter REG_FILE_MIDDLE_DELAY_RANGE_BASE 	= top_consts_types_package::reg_file_middle_delay_range_base;
	parameter REG_FILE_MIDDLE_DELAY_RANGE_END 	= top_consts_types_package::reg_file_middle_delay_range_end;
	parameter NUM_OF_REPT_RANGE_BASE 			= top_consts_types_package::num_of_rept_range_base;
	parameter NUM_OF_REPT_RANGE_END 			= top_consts_types_package::num_of_rept_range_end;
	parameter REP_STEP_VALUE_RANGE_BASE 		= top_consts_types_package::rep_step_value_range_base;
	parameter REP_STEP_VALUE_RANGE_END 			= top_consts_types_package::rep_step_value_range_end;
	// SWB parameters
	parameter SWB_SRC_ADDR_ROW_BASE 			= top_consts_types_package::swb_src_addr_row_base;
	parameter SWB_SRC_ADDR_ROW_END 				= top_consts_types_package::swb_src_addr_row_end;
	parameter SWB_SRC_DPU_REFI_BASE 			= top_consts_types_package::swb_src_dpu_refi_base;
	parameter SWB_SRC_DPU_REFI_END 				= top_consts_types_package::swb_src_dpu_refi_end;
	parameter SWB_SRC_OUTPUT_NR_BASE 			= top_consts_types_package::swb_src_output_nr_base;
	parameter SWB_SRC_OUTPUT_NR_END 			= top_consts_types_package::swb_src_output_nr_end;
	parameter SWB_HB_INDEX_BASE 				= top_consts_types_package::swb_hb_index_base;
	parameter SWB_HB_INDEX_END 					= top_consts_types_package::swb_hb_index_end;
	parameter SWB_SEND_TO_OTHER_ROW_BASE 		= top_consts_types_package::swb_send_to_other_row_base;
	parameter SWB_SEND_TO_OTHER_ROW_END 		= top_consts_types_package::swb_send_to_other_row_end;
	parameter SWB_V_INDEX_BASE 					= top_consts_types_package::swb_v_index_base;
	parameter SWB_V_INDEX_END 					= top_consts_types_package::swb_v_index_end;
	// DELAY parameters
	parameter DLY_CYCLES_RANGE_BASE 			= top_consts_types_package::dly_cycles_range_base;
	parameter DLY_CYCLES_RANGE_END 				= top_consts_types_package::dly_cycles_range_end;
	// Branch parameters
	parameter BR_MODE_RANGE_BASE	 			= top_consts_types_package::br_mode_range_base;
	parameter BR_MODE_RANGE_END		 			= top_consts_types_package::br_mode_range_end;
	parameter BR_FALSE_ADDRS_RANGE_BASE			= top_consts_types_package::br_false_addrs_range_base;
	parameter BR_FALSE_ADDRS_RANGE_END 			= top_consts_types_package::br_false_addrs_range_end;
	// Jump parameters
	parameter TRUE_ADDRS_RANGE_BASE 			= top_consts_types_package::true_addrs_range_base;
	parameter TRUE_ADDRS_RANGE_END	 			= top_consts_types_package::true_addrs_range_end;
	// RACCU parameters
	parameter RACCU_MODE_SEL_RANGE_BASE = top_consts_types_package::raccu_mode_sel_range_base;
	parameter RACCU_MODE_SEL_RANGE_END = top_consts_types_package::raccu_mode_sel_range_end;
	parameter RACCU_OPERAND1_RANGE_BASE = top_consts_types_package::raccu_operand1_range_base;
	parameter RACCU_OPERAND1_RANGE_END = top_consts_types_package::raccu_operand1_range_end;
	parameter RACCU_OPERAND2_RANGE_BASE = top_consts_types_package::raccu_operand2_range_base;
	parameter RACCU_OPERAND2_RANGE_END = top_consts_types_package::raccu_operand2_range_end;
	parameter RACCU_RESULT_ADDR_RANGE_BASE = top_consts_types_package::raccu_result_addr_range_base;
	parameter RACCU_RESULT_ADDR_RANGE_END = top_consts_types_package::raccu_result_addr_range_end;
	// FOR_HEADER parameters
	parameter FOR_INDEX_ADDR_RANGE_BASE = top_consts_types_package::for_index_addr_range_base;
	parameter FOR_INDEX_ADDR_RANGE_END = top_consts_types_package::for_index_addr_range_end;
	parameter FOR_INDEX_START_RANGE_BASE = top_consts_types_package::for_index_start_range_base;
	parameter FOR_INDEX_START_RANGE_END = top_consts_types_package::for_index_start_range_end;
	parameter FOR_ITER_NO_RANGE_BASE = top_consts_types_package::for_iter_no_range_base;
	parameter FOR_ITER_NO_RANGE_END = top_consts_types_package::for_iter_no_range_end;
	// FOR_TAIL parameters
	parameter FOR_TAIL_INDEX_ADDR_RANGE_BASE = top_consts_types_package::for_tail_index_addr_range_base;
	parameter FOR_TAIL_INDEX_ADDR_RANGE_END = top_consts_types_package::for_tail_index_addr_range_end;
	parameter FOR_INDEX_STEP_RANGE_BASE = top_consts_types_package::for_index_step_range_base;
	parameter FOR_INDEX_STEP_RANGE_END = top_consts_types_package::for_index_step_range_end;
	parameter FOR_PC_TOGO_RANGE_BASE = top_consts_types_package::for_pc_togo_range_base;
	parameter FOR_PC_TOGO_RANGE_END = top_consts_types_package::for_pc_togo_range_end;

endpackage

import name_mangling::*;

`endif //_NAME_MANGLING_
)";
	ofstream ofs(filename_, ofstream::out);
	ofs << str;
}

void RtlGenerator::gen_test_util(string filename_)
{
	string str = R"***(
`ifndef _TEST_UTIL_
`define _TEST_UTIL_

`include "macros.svh"

package test_util;

`include "type_def.svh"

	RegData regData[ROWS][COLUMNS];
	SequencerData sequencerData[ROWS][COLUMNS];
	SramData sramData[ROWS-1][COLUMNS];
	string regFileActivities[ROWS][COLUMNS];
	string sequencerActivities[ROWS][COLUMNS];
	int oldpcs[ROWS][COLUMNS];
	InstructionCode instructionCode;

	function automatic string getRegValueStr(shortint row, col, address, data, portNo, executionCycle, variablesStartAddr, addressOffset,
											 bit is_input, string regVariable, bit is_fixed_point, bit ignoreWriteAccess, output string resultStr);
		
		const string name = is_input ? (portNo == 0 ? "in0 :" : "in1 :") : (portNo == 0 ? "out0:" : "out1:");
		const string dataValue = is_fixed_point ? $sformatf("%.4f", data * 2.0 ** -15) : $sformatf("%2d", data);
		const string lineStr = $sformatf({(is_input ? "WT" : "RD"), ": %1s(%2d) = ", dataValue, "; \t{ addr_", name, " %2d, data_", name, " %2d }"},
		         			regVariable, addressOffset + address - variablesStartAddr + 1, address, data);
		const string timeStr = $sformatf(",\t@ %5d ns (cycle: %2d)", $time, executionCycle);

		regFileActivities[row][col] = {regFileActivities[row][col], lineStr, timeStr, "\n"};
		resultStr = is_input && !ignoreWriteAccess ? $sformatf({"\n%1s(%2d)=", dataValue}, regVariable, addressOffset + address - variablesStartAddr + 1) : "";

		return lineStr;
	endfunction

	function automatic string printDimarchData(integer fileId, shortint executionCycle);

		for (int col = 0; col < COLUMNS; col++)
			if (`currSramData.writeEn)
				$fdisplay(fileId, printStileData(0, col, executionCycle, `currSramData.writeAddress, `currSramData.data));

	endfunction

	function automatic string printStileData(shortint row, col, executionCycle, address, logic [SRAM_WIDTH-1:0] data);
		const string timeStr = $sformatf(",\t@ %5d ns (cycle: %2d)", $time, executionCycle);
		return $sformatf("\nWT to SRAM(%1d,%1d), ADDRESS(%3d) -> %h%s", row, col, address, data, timeStr);
	endfunction

	function automatic void createSequencerActivity(int row, col, executionCycle);

		string instDetail;

		if (`currSequencerData.pc != oldpcs[row][col])
		begin
			instructionCode = InstructionCode'(`currInstruction[INSTR_CODE_RANGE_BASE:INSTR_CODE_RANGE_END]);

			// Specifying the instruction details
			case (instructionCode)
				iDpu:
					instDetail = $sformatf("mode: %0d", `currInstruction[DPU_MODE_SEL_RANGE_BASE : DPU_MODE_SEL_RANGE_END]);
				iRefi1:
					instDetail = $sformatf("port: %0d, startAddr: %0d, noOfAddr: %0d, initialDelay: %0d",
						 `currInstruction[NR_OF_REG_FILE_PORTS_RANGE_BASE : NR_OF_REG_FILE_PORTS_RANGE_END],
						 `currInstruction[STARTING_ADDRS_RANGE_BASE 	  :       STARTING_ADDRS_RANGE_END],
						 `currInstruction[NR_OF_ADDRS_RANGE_BASE		  : 	     NR_OF_ADDRS_RANGE_END],
						 `currInstruction[INIT_DELAY_RANGE_BASE			  : 		  INIT_DELAY_RANGE_END]); 
				iRefi2:
					instDetail = $sformatf("stepVal: %0d, stepValSign: %0d, middleDelay: %0d, noOfRepetition: %0d, repetitionStepVal: %0d",
						 `currInstruction[STEP_VALUE_RANGE_BASE 			: 			 STEP_VALUE_RANGE_END],
						 `currInstruction[STEP_VALUE_SIGN_RANGE_BASE		: 		STEP_VALUE_SIGN_RANGE_END],
						 `currInstruction[REG_FILE_MIDDLE_DELAY_RANGE_BASE	: REG_FILE_MIDDLE_DELAY_RANGE_END],
						 `currInstruction[NUM_OF_REPT_RANGE_BASE			: 			NUM_OF_REPT_RANGE_END],
						 `currInstruction[REP_STEP_VALUE_RANGE_BASE			:		 REP_STEP_VALUE_RANGE_END]); 
				iRefi3:
					instDetail = "REFI3"; 
				iSwb:
					instDetail =  $sformatf("srcRow: %0d, srcDpuOrRefi: %4s, HbIndex: %0d, VIndex: %0d, srcOutputNr: %0d, SendToOtherRow: %3s",
						 `currInstruction[SWB_SRC_ADDR_ROW_BASE		 :		SWB_SRC_ADDR_ROW_END],
						 (`currInstruction[SWB_SRC_DPU_REFI_BASE	 :		SWB_SRC_DPU_REFI_END] == 0 ? "REFI" : "DPU"),
						 `currInstruction[SWB_HB_INDEX_BASE			 :			SWB_HB_INDEX_END],
						 `currInstruction[SWB_V_INDEX_BASE 			 :			 SWB_V_INDEX_END],
						 `currInstruction[SWB_SRC_OUTPUT_NR_BASE 	 :	   SWB_SRC_OUTPUT_NR_END],										 
						 `currInstruction[SWB_SEND_TO_OTHER_ROW_BASE : SWB_SEND_TO_OTHER_ROW_END] == 0 ? "NO" : "YES"); 
				iDelay:
					instDetail = $sformatf("delay cycles: %0d", `currInstruction[DLY_CYCLES_RANGE_BASE : DLY_CYCLES_RANGE_END]); 
				iBrnch:
					instDetail = $sformatf("mode: %0d, falseAddr: %0d", 
						`currInstruction[BR_MODE_RANGE_BASE 	   : 		BR_MODE_RANGE_END],
						`currInstruction[BR_FALSE_ADDRS_RANGE_BASE : BR_FALSE_ADDRS_RANGE_END]);
				iJump:
					instDetail = $sformatf("Address: %0d", `currInstruction[TRUE_ADDRS_RANGE_BASE : TRUE_ADDRS_RANGE_END]);

				iRaccu:
					instDetail = $sformatf("Mode: %0d, Op1: %0d, Op2: %0d, ResAddress: %0d",
						`currInstruction[RACCU_MODE_SEL_RANGE_BASE 	  :    RACCU_MODE_SEL_RANGE_END],
						`currInstruction[RACCU_OPERAND1_RANGE_BASE 	  :    RACCU_OPERAND1_RANGE_END],
						`currInstruction[RACCU_OPERAND2_RANGE_BASE 	  :    RACCU_OPERAND2_RANGE_END],
						`currInstruction[RACCU_RESULT_ADDR_RANGE_BASE : RACCU_RESULT_ADDR_RANGE_END]);

				iForHead:
					instDetail = $sformatf("LoopId: %0d, IndexStart: %0d, IterationNo: %0d", 
						`currInstruction[FOR_INDEX_ADDR_RANGE_BASE 	 :  FOR_INDEX_ADDR_RANGE_END],
						`currInstruction[FOR_INDEX_START_RANGE_BASE  : FOR_INDEX_START_RANGE_END],
						`currInstruction[FOR_ITER_NO_RANGE_BASE 	 :     FOR_ITER_NO_RANGE_END]);

				iForTail:
					instDetail = $sformatf("LoopId: %0d, IndexStep: %0d, PCtoGo: %0d", 
						`currInstruction[FOR_TAIL_INDEX_ADDR_RANGE_BASE : FOR_TAIL_INDEX_ADDR_RANGE_END],
						`currInstruction[FOR_INDEX_STEP_RANGE_BASE  	:      FOR_INDEX_STEP_RANGE_END],
						`currInstruction[FOR_PC_TOGO_RANGE_BASE 	 	:         FOR_PC_TOGO_RANGE_END]);

				default:
					instDetail = "";
			endcase

			sequencerActivities[row][col] = { sequencerActivities[row][col], 
				$sformatf("%5d\t%5d\t%8h\t%8s\t--->\t(%s)\n", executionCycle, `currSequencerData.pc,
						  `currInstruction, instructionCode.name(), instDetail) };

			oldpcs[row][col] = `currSequencerData.pc;
		end
	endfunction

	function automatic void printRegFileActivity(int fileid, row, col);
		if (regFileActivities[row][col].len() > 0)
		begin
			$fdisplay(fileid, "#----------------- RegFile <%0d,%0d> -----------------\n", row, col);
			$fdisplay(fileid, regFileActivities[row][col]);
		end
	endfunction

	function automatic void printSequencerActivity(int fileid, row, col);
		if (sequencerActivities[row][col].len() > 0)
		begin
			$fdisplay(fileid, "#----------------- Sequencer <%0d,%0d> -----------------", row, col);
			$fdisplay(fileid, "%5s\t%5s\t%8s\t%8s\t--->\t%s\n", "CYCLE", "PC", "INST_VAL", "INST_TYPE", "DETAILS");
			$fdisplay(fileid, sequencerActivities[row][col]);
		end
	endfunction

endpackage

`endif //_TEST_UTIL_
)***";
	ofstream ofs(filename_, ofstream::out);
	ofs << str;
}
void RtlGenerator::gen_test_util_files(string filename_)
{
	string str = R"(
name_mangling.svh
macros.svh
type_def.svh
test_util.svh
fill_data.sv
)";
	ofstream ofs(filename_, ofstream::out);
	ofs << str;
}
void RtlGenerator::gen_type_def(string filename_)
{
	string str = R"(
`ifndef _TYPE_DEF_
`define _TYPE_DEF_

import name_mangling::*;

typedef struct
{
	logic [REG_FILE_DATA_WIDTH-1 : 0] dataIn0;
	logic [REG_FILE_DATA_WIDTH-1 : 0] dataIn1;
	logic [REG_FILE_DATA_WIDTH-1 : 0] dataOut0;
	logic [REG_FILE_DATA_WIDTH-1 : 0] dataOut1;

	logic [REG_FILE_ADDR_WIDTH-1 : 0] addrIn0;
	logic [REG_FILE_ADDR_WIDTH-1 : 0] addrIn1;
	logic [REG_FILE_ADDR_WIDTH-1 : 0] addrOut0;
	logic [REG_FILE_ADDR_WIDTH-1 : 0] addrOut1;

	logic instStartIn0;
	logic addrenIn0;
	logic addrenIn1;
	logic addrenOut0;
	logic addrenOut1;
} RegData;

typedef struct
{
	int pc;
	logic [INSTR_WIDTH-1 : 0] currInst;
} SequencerData;

typedef struct
{
	logic readEn;
	logic writeEn;
	int writeAddress;
	logic [SRAM_WIDTH-1:0] data;
} SramData;

typedef enum 
{
	iBrnch 		= 'b1011,
	iDelay 		= 'b0111,
	iDpu   		= 'b0100,
	iForTail   	= 'b1001,
	iForHead  	= 'b1000,
	iJump  		= 'b0110,
	iRaccu 		= 'b1010,
	iRefi1 		= 'b0001,
	iRefi2 		= 'b0010,
	iRefi3 		= 'b0011,
	iRoute		= 'b1100,
	iSRAM_R		= 'b1101,
	iSRAM_W		= 'b1110,
	iSwb   		= 'b0101
} InstructionCode;

`endif //_TYPE_DEF_
)";
	ofstream ofs(filename_, ofstream::out);
	ofs << str;
}

string RtlGenerator::InputFileName()
{
	util::GlobalVar glv;
	string input_file_name = "";
	CHECK(glv.gets("input_file_name", input_file_name));
	return input_file_name;
}

void RtlGenerator::gen_profiler(map<string, vector<BIR::Instruction *>> instr_lists_, codegen::StorageImage si_, string filename_)
{
	// header
	ofstream ofs(filename_, ofstream::out);
	string entityName = "profiler_" + InputFileName();

	//! Modify the path. because we directly copy util files to the current directory.
	Common::write_line(ofs,
										 R"(
`include "macros.svh"
import const_package::*;
import name_mangling::*;
import test_util::*;

module profiler(input logic clk, rst_n);

	const string log_file1 = "results/sv_results1_exc.txt";
	const string log_file2 = "results/sv_results2_reg.txt";
	const string log_file3 = "results/sv_results3_seq.txt";
	const string log_file4 = "results/sv_results4_srm.txt";
	const string log_file5 = "results/sv_results.txt";
	
	const integer fileid1 = $fopen(log_file1, "w");
	const integer fileid2 = $fopen(log_file2, "w");
	const integer fileid3 = $fopen(log_file3, "w");
	const integer fileid4 = $fopen(log_file4, "w");
	const integer fileid5 = $fopen(log_file5, "w");
	
	int clkCounter = 0;
	int executionCycle = 0;
	string regWriteActivities;
	string regReadActivities;
	string resultActivities;
	const string wtDelimiter = "-------------------------------------------------------------------------------------------------";
	const string startOfExecutionStr = $sformatf("\nStart of execution: %0d ns (cycle = 0)\n", (execution_start_cycle + 1) * period);
	)");

	Common::write_line(ofs, dump_profiler_reg_file_var(si_));
	Common::write_line(ofs, dump_profiler_skip_cycles_of_write_access(instr_lists_));
	Common::write_line(ofs, dump_profiler_fixed_point_status(si_));

	Common::write_line(ofs, R"***(
	function automatic void printRegValue(shortint row, col, address, data, portNo, bit is_input);
		
		const string delimiter       = "---------------";
		const string firstDelimiter  = is_input ? {delimiter, "\n"} : "";
		const string currPosition    = $sformatf("\tin RegFile <%0d,%0d>,\t@ cycle: %2d (%4d ns)", row, col, executionCycle, $time);
		const string addressOffsetKey= $sformatf("%s_%0d%0d", `currRegVariable, row, col);
		
		string resultStr;
		const string strLine = { firstDelimiter, getRegValueStr(row, col, address, data, portNo, executionCycle,
				  				 variablesStartAddr[`currRegVariable], addressOffset[addressOffsetKey], is_input,
				  				 `currRegVariable, `currFixedPointStatus, skipCyclesOfWriteAccess[row][col].exists(executionCycle), resultStr), currPosition };
		
		if (is_input)
			regWriteActivities = {regWriteActivities, "\n", strLine};
		else
			regReadActivities = {regReadActivities, "\n", strLine};
		
		resultActivities = {resultActivities, resultStr};
		
	endfunction
	fill_data fd();

	always @(negedge rst_n or negedge clk)
	begin
		if (rst_n == 1'b0)
		begin
			clkCounter = 0;
			$fdisplay(fileid1, startOfExecutionStr);
		end
		else
		begin
			if (clkCounter >= execution_start_cycle)
			begin
				executionCycle = clkCounter - execution_start_cycle;
				
				regWriteActivities = "";
				regReadActivities  = "";
				resultActivities   = "";
				
				for (int row = 0; row < ROWS; row++)
					for (int col = 0; col < COLUMNS; col++)
					begin
						if (`currRegData.addrenIn0 || `currRegData.addrenIn1 || `currRegData.addrenOut0 || `currRegData.addrenOut1)
						begin
							if (`currRegData.addrenIn0)
								printRegValue(row, col, `currRegData.addrIn0, `currRegData.dataIn0, 0, 1);
							
							if (`currRegData.addrenIn1)
								printRegValue(row, col, `currRegData.addrIn1, `currRegData.dataIn1, 1, 1);
							
							if (`currRegData.addrenOut0)
								printRegValue(row, col, `currRegData.addrOut0, `currRegData.dataOut0, 0, 0);
							
							if (`currRegData.addrenOut1)
								printRegValue(row, col, `currRegData.addrOut1, `currRegData.dataOut1, 1, 0);
						end
						
						createSequencerActivity(row, col, executionCycle);
						
					end
				if (regWriteActivities != "")
				begin
					$fdisplay(fileid1, regWriteActivities);
					$fdisplay(fileid1, wtDelimiter);
				end
				
				if (regReadActivities != "")
					$fdisplay(fileid1, regReadActivities);
				
				if (resultActivities != "")
					$fdisplay(fileid5, resultActivities);
				
				printDimarchData(fileid4, executionCycle);
			end
			
			if (clkCounter == total_execution_cycle-1)
			begin
				$fdisplay(fileid2, startOfExecutionStr);
				$fdisplay(fileid3, startOfExecutionStr);
				
				for (int row = 0; row < ROWS; row++)
					for (int col = 0; col < COLUMNS; col++)
					begin
						printRegFileActivity(fileid2, row, col);
						printSequencerActivity(fileid3, row, col);
					end
			end
			
			clkCounter++;
		end
	end

endmodule
)***");
}

string RtlGenerator::dump_profiler_reg_file_var(codegen::StorageImage si_)
{
	return si_.print_for_profiler();
}

string RtlGenerator::dump_profiler_fixed_point_status(codegen::StorageImage si_)
{
	int all_drra_row;
	int all_drra_col;
	int reg_file_depth;
	int reg_file_width;
	int sram_depth;
	int sram_width;

	util::GlobalVar glv;
	CHECK(glv.geti("drra_row", all_drra_row));
	CHECK(glv.geti("drra_col", all_drra_col));
	CHECK(glv.geti("reg_file_depth", reg_file_depth));
	CHECK(glv.geti("reg_file_width", reg_file_width));
	CHECK(glv.geti("sram_depth", sram_depth));
	CHECK(glv.geti("sram_width", sram_width));
	int mem_block_size = sram_width / reg_file_width;

	string lineStr = "";
	lineStr += "const bit regFileFixedPointStatus[ROWS][COLUMNS][REG_FILE_DEPTH] = '{\n";
	lineStr += "\t";

	const int rowNum = all_drra_row;
	const int colNum = all_drra_col;

	for (int rowIndex = 0; rowIndex < rowNum; ++rowIndex)
	{
		lineStr += "'{\n";
		lineStr += "\t\t";

		for (int colIndex = 0; colIndex < colNum; ++colIndex)
		{
			VIR::Coordinate coord(rowIndex, colIndex);
			for (int regIndex = 0; regIndex < reg_file_depth; ++regIndex)
			{
				if (regIndex == 0)
					lineStr += "'{ ";

				lineStr += (si_._reg[coord.to_str()][regIndex].is_fixed_point == false ? "0" : "1");

				if (regIndex != reg_file_depth - 1)
					lineStr += ", ";
				else
				{
					lineStr += " }";

					if (colIndex != colNum - 1)
						lineStr += ",";
				}

				if ((regIndex + 1) % 16 == 0 || regIndex == reg_file_depth - 1)
				{
					lineStr += "\n";
					lineStr += "   ";
				}
			}
		}

		lineStr += "}";

		if (rowIndex != rowNum - 1)
			lineStr += ",";

		lineStr += "\t\n";
	}

	lineStr += "\t};\n";
	return lineStr;
}

string RtlGenerator::dump_profiler_skip_cycles_of_write_access(map<string, vector<BIR::Instruction *>> instr_lists_)
{
	string str = "";
	str += "const int skipCyclesOfWriteAccess[ROWS][COLUMNS][int] = '{\n";

	int all_drra_row;
	int all_drra_col;
	util::GlobalVar glv;
	CHECK(glv.geti("drra_row", all_drra_row));
	CHECK(glv.geti("drra_col", all_drra_col));
	const int rowNum = all_drra_row;
	const int colNum = all_drra_col;
	string lineStr;
	for (int rowIndex = 0; rowIndex < rowNum; ++rowIndex)
	{
		str += ("\n\t'{");
		for (int colIndex = 0; colIndex < colNum; ++colIndex)
		{
			VIR::Coordinate coord(rowIndex, colIndex);
			bool foundSkipCycles = false;
			vector<Instruction *> &instr_list = instr_lists_[coord.to_str()];
			vector<int> skip_cycles;
			for (int i = 0; i < instr_list.size(); i++)
			{
				Instruction *instr = instr_list[i];
				if (instr->kind() == BIR::BIREnumerations::bktSRAMInstruction)
				{
					SRAMInstruction *sramInst_ = static_cast<SRAMInstruction *>(instr);
					if (sramInst_->isRead)
					{
						for (int cycles : sramInst_->exec_trace)
						{
							skip_cycles.push_back(cycles);
						}
					}
				}
			}
			std::sort(skip_cycles.begin(), skip_cycles.end());
			if (skip_cycles.size() > 0)
			{
				foundSkipCycles = true;
			}

			lineStr = "";
			int counter = 0;
			for (int cycles : skip_cycles)
			{
				if (counter != 0)
					lineStr += ", ";
				else
					lineStr += "\n\t\t'{   ";

				lineStr += to_string(cycles) + ":0";

				++counter;

				if (counter % 16 == 0)
				{
					str += (lineStr);
					lineStr = "\n";
					lineStr += "\t   ";
				}
			}
			if (counter % 16 != 0)
			{
				str += (lineStr);
			}

			// If no memory read instruction found in the instruction list, add a default key to the associative array.
			lineStr = !foundSkipCycles ? "\n\t\t'{-1:0}" : " \t\t}";

			if (colIndex != colNum - 1)
				lineStr += ",";

			str += (lineStr);
		}

		lineStr = "\t\t}";

		if (rowIndex != rowNum - 1)
			lineStr += ",";

		str += lineStr;
	}

	str += "};";

	return str;
}

void RtlGenerator::gen_run_cmd(string filename_)
{

	int total_exec_cycle = 0;
	util::GlobalVar glv;
	CHECK(glv.geti("total_exec_cycle", total_exec_cycle));

	ofstream ofs(filename_, ofstream::out);
	string str = R"(
vcom -mixedsvvh const_package.vhd
vlog profiler.sv
vcom testbench.vhd

vsim testbench
set StdArithNoWarnings 1
set NumericStdNoWarnings 1
run 0 ns;
set StdArithNoWarnings 0
set NumericStdNoWarnings 0
)";
	ofs << str << endl;
	ofs << "run " << total_exec_cycle * 10 << "ns;" << endl;
	ofs << "quit";
}

void RtlGenerator::gen_run_gui(string filename_)
{
	int total_exec_cycle = 0;
	util::GlobalVar glv;
	CHECK(glv.geti("total_exec_cycle", total_exec_cycle));

	ofstream ofs(filename_, ofstream::out);
	string str = R"(
vcom -mixedsvvh const_package.vhd
vlog profiler.sv
vcom testbench.vhd

# -------------------------------------------------
# Call vsim to invoke simulator
vsim -voptargs=+acc testbench
#vsim -coverage -debugDB testbench_opt
# -------------------------------------------------
# Set the window types and source the wave do file
# view wave
# view structure
# view signals
do wave.do
#log -r /*
# -------------------------------------------------
# Run simulation for this time
set StdArithNoWarnings 1
set NumericStdNoWarnings 1
run 0 ns;
set StdArithNoWarnings 0
set NumericStdNoWarnings 0
)";
	ofs << str;
	ofs << "run " << total_exec_cycle * 10 << "ns;" << endl;
	str = R"(
write transcript results/sim_result.txt
# -------------------------------------------------
# End
# quit -sim
)";
	ofs << str;
}

// This method writes the wave file for the VHDL testbench
void RtlGenerator::gen_wave(map<string, vector<BIR::Instruction *>> instr_lists_, codegen::StorageImage si_, string filename_)
{
	ofstream ofs(filename_, ofstream::out);
	string str = R"(
onerror {resume}
quietly WaveActivateNextPane {} 0

add wave -divider
add wave -label clk   clk
add wave -label rst_n rst_n
add wave -divider "Counters"
add wave -noupdate -radix decimal /StimuliSequencer/*
)";
	ofs << str;

	ofs << dump_drra_wave(instr_lists_, si_) << endl;
	ofs << dump_dimarch_wave(si_) << endl;

	str = R"(

TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {0 ns} 0}
quietly wave cursor active 0
configure wave -namecolwidth 300
configure wave -valuecolwidth 100
configure wave -justifyvalue left
configure wave -signalnamewidth 0
configure wave -snapdistance 10
configure wave -datasetprefix 0
configure wave -rowmargin 4
configure wave -childrowmargin 2
configure wave -gridoffset 0
configure wave -gridperiod 1
configure wave -griddelta 40
configure wave -timeline 0
configure wave -timelineunits ns
update
WaveRestoreZoom {0 ns} {170 ns}
)";
	ofs << str;
}

string RtlGenerator::dump_drra_wave(map<string, vector<BIR::Instruction *>> instr_lists_, codegen::StorageImage si_)
{

	int utilized_drra_row;
	int utilized_drra_col;
	util::GlobalVar glv;
	CHECK(glv.geti("utilized_drra_row", utilized_drra_row));
	CHECK(glv.geti("utilized_drra_col", utilized_drra_col));

	string str = "";
	const string groupStr = "add wave -group ";
	const string labelStr = "\t-label ";

	str += "\n";
	str += ("add wave -divider \"Compare Register Values\"\n");

	for (int rowIndex = 0; rowIndex < utilized_drra_row; ++rowIndex)
	{
		for (int colIndex = 0; colIndex < utilized_drra_col; ++colIndex)
		{
			bool flag_need_wave = false;
			VIR::Coordinate coord(rowIndex, colIndex);
			if (instr_lists_.find(coord.to_str()) != instr_lists_.end())
			{
				if (instr_lists_[coord.to_str()].size() > 0)
				{
					flag_need_wave = true;
				}
			}
			for (int i = 0; i < si_._reg[coord.to_str()].size(); i++)
			{
				if (si_._reg[coord.to_str()][i].var_name != " ")
				{
					flag_need_wave = true;
					break;
				}
			}
			if (flag_need_wave)
			{
				string cellLabel = " <" + to_string(rowIndex) + ", " + to_string(colIndex) + ">";
				string regSignagure = "/DUT/MTRF_COLS(" + to_string(colIndex) + ")/MTRF_ROWS(" + to_string(rowIndex) + ")/SILEGO_cell/MTRF_cell/reg_top/";

				str += ("");
				str += (groupStr + "\"RegFile" + cellLabel + "\"	-radix decimal \\") + "\n";
				str += (labelStr + "data_out_reg_0 	" + regSignagure + "RegisterFile/data_out_reg_0_left		\\") + "\n";
				str += (labelStr + "data_out_reg_1 	" + regSignagure + "RegisterFile/data_out_reg_1_left		\\") + "\n";
				str += (labelStr + "data_in_0 		" + regSignagure + "RegisterFile/data_in_0					\\") + "\n";
				str += (labelStr + "data_in_1 		" + regSignagure + "RegisterFile/data_in_1					\\") + "\n";
				str += (labelStr + "data_in_2 		" + regSignagure + "RegisterFile/data_in_2					\\") + "\n";
				str += (labelStr + "reg_out			" + regSignagure + "RegisterFile/reg_out") + "\n";
			}
		}
	}

	str += ("");
	str += ("add wave -divider");

	for (int rowIndex = 0; rowIndex < utilized_drra_row; ++rowIndex)
	{
		for (int colIndex = 0; colIndex < utilized_drra_col; ++colIndex)
		{
			bool flag_need_wave = false;
			VIR::Coordinate coord(rowIndex, colIndex);
			if (instr_lists_.find(coord.to_str()) != instr_lists_.end())
			{
				if (instr_lists_[coord.to_str()].size() > 0)
				{
					flag_need_wave = true;
				}
			}
			if (flag_need_wave)
			{
				string cellLabel = " <" + to_string(rowIndex) + ", " + to_string(colIndex) + ">";
				string cellSignature = "/DUT/MTRF_COLS(" + to_string(colIndex) + ")/MTRF_ROWS(" + to_string(rowIndex) + ")/SILEGO_cell/MTRF_cell/";
				string seqSignature = cellSignature + "seq_gen/";
				string dpuSignature = cellSignature + "dpu_gen/";
				string regSignagure = cellSignature + "reg_top/";

				str += "\n";
				str += ("add wave -divider \"Cell" + cellLabel + "\"") + "\n";
				str += "\n";
				str += (groupStr + "\"Sequencer" + cellLabel + "\"	-radix decimal \\") + "\n";
				str += (labelStr + "pc 				 " + seqSignature + "pc				\\") + "\n";
				str += ("	-radix decimal \\\n");
				str += (labelStr + "seq_address_match " + seqSignature + "seq_address_match	\\") + "\n";
				str += (labelStr + "instr_reg		 " + seqSignature + "instr_reg			\\") + "\n";
				str += (labelStr + "instr 			 " + seqSignature + "instr") + "\n";
				str += "\n";
				str += (groupStr + "\"DPU" + cellLabel + "\"	-radix decimal \\") + "\n";
				str += (labelStr + "dpu_in_0  " + dpuSignature + "dpu_in_0			\\") + "\n";
				str += (labelStr + "dpu_in_1  " + dpuSignature + "dpu_in_1			\\") + "\n";
				str += (labelStr + "dpu_in_2  " + dpuSignature + "dpu_in_2			\\") + "\n";
				str += (labelStr + "dpu_in_3  " + dpuSignature + "dpu_in_3			\\") + "\n";
				str += (labelStr + "dpu_mode  " + dpuSignature + "dpu_mode_cfg		\\") + "\n";
				str += (labelStr + "dpu_out_0 " + dpuSignature + "dpu_out_0_left	\\") + "\n";
				str += (labelStr + "dpu_out_1 " + dpuSignature + "dpu_out_1_left") + "\n";
				str += "\n";
				str += (groupStr + "\"Register File" + cellLabel + "\"	-radix decimal \\") + "\n";
				str += (labelStr + "instr_start 		" + regSignagure + "instr_start							\\") + "\n";
				str += (labelStr + "data_out_reg_0 	" + regSignagure + "RegisterFile/data_out_reg_0_left		\\") + "\n";
				str += (labelStr + "data_out_reg_1 	" + regSignagure + "RegisterFile/data_out_reg_1_left		\\") + "\n";
				str += (labelStr + "data_in_0 		" + regSignagure + "RegisterFile/data_in_0					\\") + "\n";
				str += (labelStr + "data_in_1 		" + regSignagure + "RegisterFile/data_in_1					\\") + "\n";
				str += (labelStr + "data_in_2 		" + regSignagure + "RegisterFile/data_in_2					\\") + "\n";
				str += (labelStr + "reg_out			" + regSignagure + "RegisterFile/reg_out") + "\n";
				str += (groupStr + "\"AGU Wr0" + cellLabel + "\"	-radix decimal \\") + "\n";
				str += (labelStr + "instr_start " + regSignagure + "AGU_Wr_0_instantiate/instr_start	\\") + "\n";
				str += (labelStr + "addr_out    " + regSignagure + "AGU_Wr_0_instantiate/addr_out		\\") + "\n";
				str += (labelStr + "addr_en     " + regSignagure + "AGU_Wr_0_instantiate/addr_en") + "\n";
				str += (groupStr + "\"AGU Wr1" + cellLabel + "\"	-radix decimal \\") + "\n";
				str += (labelStr + "instr_start " + regSignagure + "AGU_Wr_1_instantiate/instr_start	\\") + "\n";
				str += (labelStr + "addr_out    " + regSignagure + "AGU_Wr_1_instantiate/addr_out		\\") + "\n";
				str += (labelStr + "addr_en     " + regSignagure + "AGU_Wr_1_instantiate/addr_en") + "\n";
				str += (groupStr + "\"AGU Rd0" + cellLabel + "\"	-radix decimal \\") + "\n";
				str += (labelStr + "instr_start " + regSignagure + "AGU_Rd_0_instantiate/instr_start	\\") + "\n";
				str += (labelStr + "addr_out    " + regSignagure + "AGU_Rd_0_instantiate/addr_out		\\") + "\n";
				str += (labelStr + "addr_en     " + regSignagure + "AGU_Rd_0_instantiate/addr_en") + "\n";
				str += (groupStr + "\"AGU Rd1" + cellLabel + "\"	-radix decimal \\") + "\n";
				str += (labelStr + "instr_start " + regSignagure + "AGU_Rd_1_instantiate/instr_start	\\") + "\n";
				str += (labelStr + "addr_out    " + regSignagure + "AGU_Rd_1_instantiate/addr_out		\\") + "\n";
				str += (labelStr + "addr_en     " + regSignagure + "AGU_Rd_1_instantiate/addr_en") + "\n";
			}
		}
	}
	return str;
}

string RtlGenerator::dump_dimarch_wave(codegen::StorageImage si_)
{
	int utilized_dimarch_row;
	int utilized_dimarch_col;
	util::GlobalVar glv;
	CHECK(glv.geti("utilized_dimarch_row", utilized_dimarch_row));
	CHECK(glv.geti("utilized_dimarch_col", utilized_dimarch_col));

	const string groupStr = "add wave -group ";
	const string labelStr = "\t-label ";

	string str = "";
	str += "\n";
	str += ("add wave -divider \"SRAM Values\"\n");

	for (int colIndex = 0; colIndex < utilized_dimarch_col; ++colIndex)
	{
		string colStr = to_string(colIndex);

		str += (groupStr + "\"Column <" + colStr + ">\" \\") + "\n";

		for (int rowIndex = 0; rowIndex < utilized_dimarch_row; ++rowIndex)
		{
			string rowStr = to_string(rowIndex);
			string stileMemSignature = "/DUT/DiMArch_COLS(" + colStr + ")/DiMArch_ROWS(" + rowStr + ")/u_STILE/u_sram/rw/mem";
			string backslash = (rowIndex != utilized_dimarch_row - 1) ? "	\\" : "";

			str += (labelStr + "\"STILE <" + rowStr + ">\"	" + stileMemSignature + backslash) + "\n";
		}
	}
	return str;
}

void RtlGenerator::gen_testbench(map<string, vector<BIR::Instruction *>> instr_lists_, codegen::StorageImage si_, string filename_)
{
	ofstream ofs(filename_, ofstream::out);
	ofs << dump_testbench_header(instr_lists_, si_) << endl;
	Common::write_line(ofs, "", 1);
	ofs << dump_testbench_instr(instr_lists_, si_) << endl;
	Common::write_line(ofs, "", 1);
	Common::write_line(ofs, "END behavior;", 0);
}

string getRowOrColSignature(int index_, int length_)
{
	string signature(length_, '0');
	signature.replace(length_ - index_ - 1, 1, 1, '1');
	signature = "\"" + signature + "\"";
	return signature;
}

string RtlGenerator::dump_testbench_header(map<string, vector<BIR::Instruction *>> instr_lists_, codegen::StorageImage si_)
{
	int utilized_dimarch_row;
	int utilized_dimarch_col;
	int utilized_drra_row;
	int utilized_drra_col;
	int all_drra_row;
	int all_drra_col;
	util::GlobalVar glv;
	CHECK(glv.geti("utilized_drra_row", utilized_drra_row));
	CHECK(glv.geti("utilized_drra_col", utilized_drra_col));
	CHECK(glv.geti("utilized_dimarch_row", utilized_dimarch_row));
	CHECK(glv.geti("utilized_dimarch_col", utilized_dimarch_col));
	CHECK(glv.geti("drra_row", all_drra_row));
	CHECK(glv.geti("drra_col", all_drra_col));

	stringstream str;
	string entityName = "testbench";

	Common::write_line(str, "LIBRARY IEEE;");
	Common::write_line(str, "USE IEEE.NUMERIC_STD.ALL;");
	Common::write_line(str, "USE IEEE.STD_LOGIC_1164.ALL;");
	Common::write_line(str, "USE WORK.top_consts_types_package.ROWS;");
	Common::write_line(str, "USE WORK.top_consts_types_package.COLUMNS;");
	Common::write_line(str, "USE WORK.top_consts_types_package.INSTR_WIDTH;");
	Common::write_line(str, "USE WORK.top_consts_types_package.REG_FILE_MEM_ADDR_WIDTH;");
	Common::write_line(str, "USE WORK.top_consts_types_package.REG_FILE_MEM_DATA_WIDTH;");
	Common::write_line(str, "USE WORK.top_consts_types_package.SRAM_ADDRESS_WIDTH;");
	Common::write_line(str, "USE WORK.top_consts_types_package.SRAM_WIDTH;");
	Common::write_line(str, "USE WORK.tb_instructions.ALL;");
	Common::write_line(str, "USE WORK.const_package.ALL;");
	Common::write_line(str, "USE WORK.noc_types_n_constants.ALL;");
	Common::write_line(str, "");
	Common::write_line(str, "ENTITY testbench IS");
	Common::write_line(str, "END testbench;");
	Common::write_line(str, "");
	Common::write_line(str, "ARCHITECTURE behavior OF " + entityName + " IS");
	Common::write_line(str, "");
	Common::write_line(str, "SIGNAL clk            : std_logic := '0';", 1);
	Common::write_line(str, "SIGNAL rst_n          : std_logic := '0';", 1);
	Common::write_line(str, "SIGNAL instr_load     : std_logic := '0';", 1);
	Common::write_line(str, "SIGNAL instr_input    : std_logic_vector(INSTR_WIDTH downto 0);", 1);
	Common::write_line(str, "SIGNAL seq_address_rb : std_logic_vector(ROWS-1 downto 0);", 1);
	Common::write_line(str, "SIGNAL seq_address_cb : std_logic_vector(COLUMNS-1 downto 0);", 1);
	Common::write_line(str, "", 1);
	Common::write_line(str, "SIGNAL fabric_reg_wr_2		: std_logic := '0';", 1);
	Common::write_line(str, "SIGNAL fabric_reg_rd_2		: std_logic := '0';", 1);
	Common::write_line(str, "SIGNAL fabric_reg_wr_addr_2 : std_logic_vector(REG_FILE_MEM_ADDR_WIDTH-1 downto 0) := (OTHERS => '0');", 1);
	Common::write_line(str, "SIGNAL fabric_reg_rd_addr_2 : std_logic_vector(REG_FILE_MEM_ADDR_WIDTH-1 downto 0) := (OTHERS => '0');", 1);
	Common::write_line(str, "SIGNAL fabric_data_in_reg_2 : signed(REG_FILE_MEM_DATA_WIDTH-1 DOWNTO 0) := (OTHERS => '0');", 1);
	Common::write_line(str, "SIGNAL fabric_data_out_2 	: signed(REG_FILE_MEM_DATA_WIDTH-1 DOWNTO 0);", 1);
	Common::write_line(str, "SIGNAL tb_or_dimarch 		: std_logic;", 1);
	Common::write_line(str, "SIGNAL tb_en    : std_logic;", 1);
	Common::write_line(str, "SIGNAL tb_addrs : std_logic_vector (SRAM_ADDRESS_WIDTH-1 downto 0);", 1);
	Common::write_line(str, "SIGNAL tb_inp   : std_logic_vector (SRAM_WIDTH-1 downto 0);", 1);
	Common::write_line(str, "SIGNAL tb_ROW   : unsigned (ROW_WIDTH-1 downto 0);", 1);
	Common::write_line(str, "SIGNAL tb_COL   : unsigned (COL_WIDTH-1 downto 0);", 1);
	Common::write_line(str, "", 1);
	Common::write_line(str, "TYPE instruction_type IS ARRAY (0 TO instruction_no-1) OF std_logic_vector(INSTR_WIDTH downto 0);", 1);
	Common::write_line(str, "SIGNAL instruction : instruction_type;", 1);

	Common::write_line(str, "", 1);
	Common::write_line(str, "TYPE cell_info_type IS RECORD", 1);
	Common::write_line(str, "inst_no : integer range 0 to instruction_no;", 2);
	Common::write_line(str, "row : std_logic_vector(ROWS-1 downto 0);", 2);
	Common::write_line(str, "col : std_logic_vector(COLUMNS-1 downto 0);", 2);
	Common::write_line(str, "END RECORD;", 1);
	Common::write_line(str, "TYPE cell_infos_type IS ARRAY(0 TO ROWS*COLUMNS-1) OF cell_info_type;", 1);
	Common::write_line(str, "", 1);
	Common::write_line(str, "CONSTANT cell_infos : cell_infos_type := (", 1);

	string cellInfos;

	// Instructions
	for (int rowIndex = 0; rowIndex < all_drra_row; ++rowIndex)
	{
		cellInfos = "";

		for (int colIndex = 0; colIndex < all_drra_col; ++colIndex)
		{
			int instruction_count = 0;
			VIR::Coordinate coord(rowIndex, colIndex);
			if (instr_lists_.find(coord.to_str()) != instr_lists_.end())
			{
				for (auto &instr : instr_lists_[coord.to_str()])
				{
					if (instr->kind() == BIR::BIREnumerations::bktSRAMInstruction)
					{
						instruction_count += 3;
					}
					else
					{
						instruction_count += 1;
					}
				}
			}
			string cellInfo = "";
			static int accInstructionNo = 0;
			accInstructionNo += instruction_count;
			ostringstream strStream;
			if (accInstructionNo < 10)
				strStream.width(2);

			strStream << accInstructionNo;

			cellInfo += "(";
			cellInfo += strStream.str();
			cellInfo += ", " + getRowOrColSignature(rowIndex, all_drra_row);
			cellInfo += ", " + getRowOrColSignature(colIndex, all_drra_col) + ")";

			if (!(rowIndex == all_drra_row - 1 && colIndex == all_drra_col - 1))
				cellInfo += ", ";
			else
				cellInfo += ");";

			cellInfos += cellInfo;
		}
		Common::write_line(str, cellInfos);
	}

	Common::write_line(str, si_.print_for_testbench(), 1);

	Common::write_line(str, "");
	Common::write_line(str, "BEGIN");
	Common::write_line(str, "");

	Common::write_line(str, "PRF : ENTITY work.profiler", 1);
	Common::write_line(str, "PORT MAP (clk	=> clk,", 2);
	Common::write_line(str, "		  rst_n	=> rst_n);", 2);
	Common::write_line(str, "", 1);

	Common::write_line(str, "DUT : ENTITY work.fabric", 1);
	Common::write_line(str, "PORT MAP (clk					=> clk,", 2);
	Common::write_line(str, "          rst_n					=> rst_n,", 2);
	Common::write_line(str, "          instr_ld				=> instr_load,", 2);
	Common::write_line(str, "          instr_inp				=> instr_input,", 2);
	Common::write_line(str, "          seq_address_rb		=> seq_address_rb,", 2);
	Common::write_line(str, "          seq_address_cb		=> seq_address_cb,", 2);
	Common::write_line(str, "          fabric_reg_wr_2		=> fabric_reg_wr_2,", 2);
	Common::write_line(str, "          fabric_reg_rd_2		=> fabric_reg_rd_2,", 2);
	Common::write_line(str, "          fabric_reg_wr_addr_2	=> fabric_reg_wr_addr_2,", 2);
	Common::write_line(str, "          fabric_reg_rd_addr_2	=> fabric_reg_rd_addr_2,", 2);
	Common::write_line(str, "          fabric_data_in_reg_2	=> fabric_data_in_reg_2,", 2);
	Common::write_line(str, "          fabric_data_out_2		=> fabric_data_out_2,", 2);
	Common::write_line(str, "          tb_or_dimarch			=> tb_or_dimarch,", 2);
	Common::write_line(str, "          tb_en 				=> tb_en,", 2);
	Common::write_line(str, "          tb_addrs				=> tb_addrs,", 2);
	Common::write_line(str, "          tb_inp  				=> tb_inp,", 2);
	Common::write_line(str, "          tb_ROW 				=> tb_ROW,", 2);
	Common::write_line(str, "          tb_COL  				=> tb_COL);", 2);

	Common::write_line(str, "", 1);
	Common::write_line(str, "rst_n <= '0' AFTER 2.5 ns, '1' AFTER 4 ns;", 1);
	Common::write_line(str, "clk   <= NOT clk AFTER half_period;", 1);
	Common::write_line(str, "", 1);

	Common::write_line(str, "StimuliSequencer : PROCESS (clk, rst_n)", 1);
	Common::write_line(str, "VARIABLE inst_counter	  : integer := 0;", 2);
	Common::write_line(str, "VARIABLE curr_cell		  : integer := 0;", 2);

	if (utilized_dimarch_row * utilized_dimarch_col > 0)
		Common::write_line(str, "VARIABLE mem_load_counter : integer := 0;", 2);

	Common::write_line(str, "VARIABLE reg_load_counter : integer := 0;", 2);
	Common::write_line(str, "BEGIN", 1);
	Common::write_line(str, "IF (rst_n = '0') THEN", 2);
	Common::write_line(str, "inst_counter	 := 0;", 3);
	Common::write_line(str, "curr_cell		 := 0;", 3);

	if (utilized_dimarch_row * utilized_dimarch_col > 0)
		Common::write_line(str, "mem_load_counter := 0;", 3);

	Common::write_line(str, "reg_load_counter := 0;", 3);
	Common::write_line(str, "instr_load     <= '0';", 3);
	Common::write_line(str, "instr_input    <= (OTHERS =>'0');", 3);
	Common::write_line(str, "seq_address_rb <= (OTHERS =>'0');", 3);
	Common::write_line(str, "seq_address_cb <= (OTHERS =>'0');", 3);
	Common::write_line(str, "tb_or_dimarch  <= '0';", 3);
	Common::write_line(str, "tb_en 	 <= '0';", 3);
	Common::write_line(str, "tb_addrs <= (OTHERS => '0');", 3);
	Common::write_line(str, "tb_inp	 <= (OTHERS => '0');", 3);
	Common::write_line(str, "tb_ROW 	 <= (OTHERS => '0');", 3);
	Common::write_line(str, "tb_COL 	 <= (OTHERS => '0');", 3);
	Common::write_line(str, "ELSIF clk'EVENT AND clk = '0' THEN", 2);
	Common::write_line(str, "", 3);
	Common::write_line(str, "tb_en <= '0';", 3);
	Common::write_line(str, "", 3);

	// Memory load part
	if (utilized_dimarch_col * utilized_dimarch_row > 0)
	{
		Common::write_line(str, "IF (mem_load_counter < mem_load_cycles) THEN", 3);

		Common::write_line(str, "", 4);
		Common::write_line(str, "instr_load <= '0';");
		Common::write_line(str, "tb_en      <= '1';");
		Common::write_line(str, "tb_addrs   <= mem_init_values(mem_load_counter).address;", 4);
		Common::write_line(str, "tb_ROW     <= mem_init_values(mem_load_counter).row;", 4);
		Common::write_line(str, "tb_COL     <= mem_init_values(mem_load_counter).col;", 4);
		Common::write_line(str, "tb_inp     <= mem_init_values(mem_load_counter).data;", 4);
		Common::write_line(str, "", 4);
		Common::write_line(str, "mem_load_counter := mem_load_counter + 1;", 4);
		Common::write_line(str, "", 4);

		Common::write_line(str, "ELSIF (reg_load_counter < reg_load_cycles) THEN", 3);
	}
	else
	{
		Common::write_line(str, "IF (reg_load_counter < reg_load_cycles) THEN", 3);
	}

	// Register file load part
	Common::write_line(str, "", 4);
	Common::write_line(str, "instr_load			 <= '0';", 4);
	Common::write_line(str, "instr_input			 <= (OTHERS =>'0');", 4);
	Common::write_line(str, "fabric_reg_wr_2		 <= '1';", 4);
	Common::write_line(str, "seq_address_rb       <= reg_init_values(reg_load_counter).row;", 4);
	Common::write_line(str, "seq_address_cb       <= reg_init_values(reg_load_counter).col;", 4);
	Common::write_line(str, "fabric_reg_wr_addr_2 <= reg_init_values(reg_load_counter).address;", 4);
	Common::write_line(str, "fabric_data_in_reg_2 <= reg_init_values(reg_load_counter).data;", 4);
	Common::write_line(str, "", 4);
	Common::write_line(str, "reg_load_counter := reg_load_counter + 1;", 4);
	Common::write_line(str, "", 4);

	// Instruction load part
	Common::write_line(str, "ELSIF (inst_counter >= 0 AND inst_counter < instruction_no) THEN", 3);

	Common::write_line(str, "", 4);
	Common::write_line(str, "tb_or_dimarch  <= '1';", 4);

	Common::write_line(str, "", 4);
	Common::write_line(str, "WHILE (inst_counter >= cell_infos(curr_cell).inst_no) LOOP", 4);
	Common::write_line(str, "curr_cell := curr_cell + 1;", 5);
	Common::write_line(str, "END LOOP;", 4);
	Common::write_line(str, "", 4);
	Common::write_line(str, "seq_address_rb <= cell_infos(curr_cell).row;", 4);
	Common::write_line(str, "seq_address_cb <= cell_infos(curr_cell).col;", 4);
	Common::write_line(str, "instr_load     <= '1';", 4);
	Common::write_line(str, "instr_input    <= std_logic_vector(instruction(inst_counter));", 4);
	Common::write_line(str, "inst_counter   := inst_counter + 1;", 4);
	Common::write_line(str, "", 4);
	Common::write_line(str, "ELSE", 3);
	Common::write_line(str, "instr_load      <= '0';", 4);
	Common::write_line(str, "fabric_reg_wr_2 <= '0';", 4);
	Common::write_line(str, "END IF;", 3);
	Common::write_line(str, "END IF;", 2);
	Common::write_line(str, "END PROCESS StimuliSequencer;", 1);

	return str.str();
}

// This method dumps an SRAM instruction to the VHDL file.
string dumpVhdlSramInstruction(BIR::SRAMInstruction *sramInst_)
{
	string instStr;

	instStr = "AGU_INST := SRAMAGU_";

	instStr += sramInst_->isRead ? "READ" : "WRITE";
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

// This method dumps a SWB instruction to the VHDL file.
string dumpVhdlSwbInstruction(BIR::SWBInstruction *swbInst_)
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

// This method dumps a DELAY instruction to the VHDL file.
string dumpVhdlWaitInstruction(BIR::WaitInstruction *waitInst_)
{
	string instStr = "DELAY(";

	instStr += to_string((int)!waitInst_->numberOfCycles.isStatic) + ", "; // del_cycles_sd
	instStr += to_string(waitInst_->numberOfCycles.value) + ")";					 // del_cycles

	return instStr;
}

// This method dumps a BRANCH instruction to the VHDL file.
string dumpVhdlBranchInstruction(BIR::BranchInstruction *branchInst_)
{
	string instStr = "BRANCH(";

	instStr += to_string(branchInst_->modeValue) + ", "; // branch mode
	instStr += to_string(branchInst_->false_pc) + ")";	 // branch false address, calculated while synchronization

	return instStr;
}

// This method dumps a DPU instruction to the VHDL file.
string dumpVhdlDpuInstruction(BIR::DPUInstruction *dpuInst_)
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

// This method dumps a JUMP instruction to the VHDL file.
string dumpVhdlJumpInstruction(BIR::JumpInstruction *jumpInst_)
{
	string instStr = "JUMP(";

	instStr += to_string(jumpInst_->jump_pc) + ")"; // jump address, calculated in synchronization

	return instStr;
}

// This method dumps a Loop Header instruction to the VHDL file.
string dumpVhdlLoopHeaderInstruction(BIR::LoopHeaderInstruction *loopHeaderInst_)
{
	string instStr = "LOOP_HEADER(";

	instStr += to_string(loopHeaderInst_->loopId) + ", ";						// index_raccu_addr
	instStr += to_string(loopHeaderInst_->startValue) + ", ";				// index_start
	instStr += to_string(loopHeaderInst_->isStatic ? 0 : 1) + ", "; // iter_no_sd
	instStr += to_string(loopHeaderInst_->iterationNumber) + ")";		// iter_no

	return instStr;
}

// This method dumps a Loop Tail instruction to the VHDL file.
string dumpVhdlLoopTailInstruction(BIR::LoopTailInstruction *loopTailInst_)
{
	string instStr = "LOOP_TAIL(";

	instStr += to_string(loopTailInst_->stepValue) + ", "; // index_step
	instStr += to_string(loopTailInst_->gotoPC) + ", ";		 // pc_togo
	instStr += to_string(loopTailInst_->loopId) + ")";		 // index_raccu_addr

	return instStr;
}

// This method dumps a RACCU instruction to the VHDL file.
string dumpVhdlRaccuInstruction(BIR::RACCUInstruction *raccuInst_)
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

// This method dumps a REFI1 instruction to the VHDL file.
string dumpVhdlRefi1Instruction(BIR::Refi1Instruction *refi1Inst_)
{
	string instStr = "REFI_1(";

	// A workaround for potential bugs of negative initial delay
	const int initialDelay = refi1Inst_->initialDelay.value >= 0 ? refi1Inst_->initialDelay.value : refi1Inst_->isDimarchMode ? refi1Inst_->portNo == 2 || refi1Inst_->portNo == 3 ? 2 : 4 : 0;

	instStr += to_string(refi1Inst_->portNo) + ", ";												 // reg_file_port
	instStr += to_string(refi1Inst_->numberOfSubseqInst) + ", ";						 // subseq_instrs
	instStr += to_string((int)!refi1Inst_->startAddress.isStatic) + ", ";		 // start_addrs_sd
	instStr += to_string(refi1Inst_->startAddress.value) + ", ";						 // start_addrs
	instStr += to_string((int)!refi1Inst_->numberOfAddress.isStatic) + ", "; // no_of_addrs_sd
	instStr += to_string(refi1Inst_->numberOfAddress.value) + ", ";					 // no_of_addrs
	instStr += to_string((int)!refi1Inst_->initialDelay.isStatic) + ", ";		 // initial_delay_sd
	instStr += to_string(refi1Inst_->initialDelay.value) + ")";							 // initial_delay

	return instStr;
}

// This method dumps a REFI2 instruction to the VHDL file.
string dumpVhdlRefi2Instruction(BIR::Refi2Instruction *refi2Inst_)
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

// This method dumps a REFI3 instruction to the VHDL file.
string dumpVhdlRefi3Instruction(BIR::Refi3Instruction *refi3Inst_)
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
	instStr += to_string((int)(refi3Inst_->en_compression)) + ")";						// compression enable

	return instStr;
}

// This method dumps a ROUTE instruction to the VHDL file.
string dumpVhdlRouteInstruction(BIR::RouteInstruction *routeInst_)
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

// This method dumps a DRRA instruction to the VHDL file.
string dumpVhdlDrraInstruction(BIR::Instruction *instruction_, int &instructionIndex_)
{
	string instStr;

	if (instruction_->kind() == bktSRAMInstruction)
	{
		instStr = dumpVhdlSramInstruction((SRAMInstruction *)instruction_) + "\n";
		instStr += ("\t\tinstruction(" + to_string(instructionIndex_++) + ") <= " + "AGU_INST(0);") + "\n";
		instStr += ("\t\tinstruction(" + to_string(instructionIndex_++) + ") <= " + "AGU_INST(1);") + "\n";
		instStr += ("\t\tinstruction(" + to_string(instructionIndex_++) + ") <= " + "AGU_INST(2);") + "\n";
	}
	else
	{
		switch (instruction_->kind())
		{
		case bktBranchInstruction:
			instStr = dumpVhdlBranchInstruction((BranchInstruction *)instruction_);
			break;

		case bktDPUInstruction:
			instStr = dumpVhdlDpuInstruction((DPUInstruction *)instruction_);
			break;

		case bktJumpInstruction:
			instStr = dumpVhdlJumpInstruction((JumpInstruction *)instruction_);
			break;

		case bktLoopHeaderInstruction:
			instStr = dumpVhdlLoopHeaderInstruction((LoopHeaderInstruction *)instruction_);
			break;

		case bktLoopTailInstruction:
			instStr = dumpVhdlLoopTailInstruction((LoopTailInstruction *)instruction_);
			break;

		case bktRACCUInstruction:
			instStr = dumpVhdlRaccuInstruction((RACCUInstruction *)instruction_);
			break;

		case bktRefi1Instruction:
			instStr = dumpVhdlRefi1Instruction((Refi1Instruction *)instruction_);
			break;

		case bktRefi2Instruction:
			instStr = dumpVhdlRefi2Instruction((Refi2Instruction *)instruction_);
			break;

		case bktRefi3Instruction:
			instStr = dumpVhdlRefi3Instruction((Refi3Instruction *)instruction_);
			break;

		case bktRouteInstruction:
			instStr = dumpVhdlRouteInstruction((RouteInstruction *)instruction_);
			break;

		case bktSWBInstruction:
			instStr = dumpVhdlSwbInstruction((SWBInstruction *)instruction_);
			break;

		case bktWaitInstruction:
			instStr = dumpVhdlWaitInstruction((WaitInstruction *)instruction_);
			break;
		}

		instStr = ("instruction(" + to_string(instructionIndex_++) + ") <= " + instStr + ";") + "\n";
	}
	return instStr;
}

string RtlGenerator::dump_testbench_instr(map<string, vector<BIR::Instruction *>> instr_lists_, codegen::StorageImage si_)
{
	int utilized_dimarch_row;
	int utilized_dimarch_col;
	int utilized_drra_row;
	int utilized_drra_col;
	util::GlobalVar glv;
	CHECK(glv.geti("utilized_drra_row", utilized_drra_row));
	CHECK(glv.geti("utilized_drra_col", utilized_drra_col));
	CHECK(glv.geti("utilized_dimarch_row", utilized_dimarch_row));
	CHECK(glv.geti("utilized_dimarch_col", utilized_dimarch_col));

	stringstream str;
	Common::write_line(str, "InstSequencer : PROCESS IS", 1);
	Common::write_line(str, "", 2);
	Common::write_line(str, "VARIABLE AGU_INST   : AGU_INST_TYPE;", 2);
	Common::write_line(str, "", 2);
	Common::write_line(str, "BEGIN", 1);

	// DRRA instructions
	int instruction_index = 0;
	for (int rowIndex = 0; rowIndex < utilized_drra_row; ++rowIndex)
		for (int colIndex = 0; colIndex < utilized_drra_col; ++colIndex)
		{
			VIR::Coordinate coord(rowIndex, colIndex);
			if (instr_lists_.find(coord.to_str()) == instr_lists_.end())
			{
				continue;
			}
			vector<Instruction *> &instr_list = instr_lists_[coord.to_str()];
			if (instr_list.size() > 0)
			{
				string tempstr = "";
				Common::write_line(str, tempstr + "-----------------------" + " <" + to_string(rowIndex) + ", " + to_string(colIndex) + ">" + "------------------------", 2);
				for (int instIndex = 0; instIndex < instr_list.size(); ++instIndex)
				{
					Common::write_line(str, dumpVhdlDrraInstruction(instr_list[instIndex], instruction_index), 2);
				}
			}
		}
	Common::write_line(str, "", 2);
	Common::write_line(str, "WAIT;", 2);
	Common::write_line(str, "", 2);
	Common::write_line(str, "END PROCESS InstSequencer;", 1);
	return str.str();
}

void RtlGenerator::gen_const_package(map<string, vector<BIR::Instruction *>> instr_lists_, codegen::StorageImage si_, string filename_)
{

	vesyla::util::GlobalVar glv;
	int reg_file_depth;
	int reg_file_width;
	int sram_width;
	int sram_depth;
	int utilized_dimarch_row;
	int utilized_dimarch_col;
	int utilized_drra_row;
	int utilized_drra_col;
	int max_exec_cycle;
	int total_exec_cycle;

	CHECK(glv.geti("reg_file_depth", reg_file_depth));
	CHECK(glv.geti("reg_file_width", reg_file_width));
	CHECK(glv.geti("sram_depth", sram_depth));
	CHECK(glv.geti("sram_width", sram_width));
	CHECK(glv.geti("utilized_drra_row", utilized_drra_row));
	CHECK(glv.geti("utilized_drra_col", utilized_drra_col));
	CHECK(glv.geti("utilized_dimarch_row", utilized_dimarch_row));
	CHECK(glv.geti("utilized_dimarch_col", utilized_dimarch_col));
	CHECK(glv.geti("max_exec_cycle", max_exec_cycle));
	int reg_file_bank_no = reg_file_depth / (sram_width / reg_file_width);
	int mem_block_size = sram_width / reg_file_width;

	int instr_count = 0;
	for (int i = 0; i < utilized_drra_row; i++)
	{
		for (int j = 0; j < utilized_drra_col; j++)
		{
			VIR::Coordinate coord(i, j);
			if (instr_lists_.find(coord.to_str()) == instr_lists_.end())
			{
				continue;
			}
			for (auto &instr : instr_lists_[coord.to_str()])
			{
				if (instr->kind() == BIR::BIREnumerations::bktSRAMInstruction)
				{
					instr_count += 3;
				}
				else
				{
					instr_count += 1;
				}
			}
		}
	}

	int total_used_dimarch_words = 0;
	int total_used_reg_files = 0;

	for (int rowIndex = 0; rowIndex < utilized_drra_row; ++rowIndex)
	{
		for (int colIndex = 0; colIndex < utilized_drra_col; ++colIndex)
		{
			VIR::Coordinate coord(rowIndex, colIndex);
			for (int i = 0; i < si_._reg[coord.to_str()].size(); i++)
			{
				if (si_._reg[coord.to_str()][i].var_name != " ")
				{
					total_used_reg_files++;
					break;
				}
			}
		}
	}
	for (int rowIndex = 0; rowIndex < utilized_dimarch_row; ++rowIndex)
	{
		for (int colIndex = 0; colIndex < utilized_dimarch_col; ++colIndex)
		{
			VIR::Coordinate coord(rowIndex, colIndex);
			for (int index = 0; index < sram_depth; index++)
			{
				for (int i = index * mem_block_size; i < (index + 1) * mem_block_size; i++)
				{
					if (si_._sram[coord.to_str()][i].var_name != " ")
					{
						total_used_dimarch_words++;
						break;
					}
				}
			}
		}
	}

	total_exec_cycle = max_exec_cycle + instr_count + total_used_dimarch_words + reg_file_bank_no * reg_file_bank_no + 10;
	glv.puti("total_exec_cycle", total_exec_cycle);

	ofstream ofs(filename_, ofstream::out);
	Common::write_line(ofs, "PACKAGE const_package IS");

	Common::write_line(ofs, "", 1);
	Common::write_line(ofs, "CONSTANT instruction_no         : integer := " + to_string(instr_count) + ";", 1);
	Common::write_line(ofs, "CONSTANT mem_load_cycles        : integer := " + to_string(total_used_dimarch_words) + ";", 1);
	Common::write_line(ofs, "CONSTANT reg_load_cycles        : integer := " + to_string(total_used_reg_files * reg_file_bank_no) + ";", 1);
	Common::write_line(ofs, "CONSTANT execution_start_cycle  : integer := instruction_no + reg_load_cycles + mem_load_cycles + 1;", 1);
	Common::write_line(ofs, "CONSTANT total_execution_cycle  : integer := " + to_string(total_exec_cycle) + ";", 1);
	Common::write_line(ofs, "CONSTANT period                 : time    := " + to_string(10) + " NS;", 1);
	Common::write_line(ofs, "CONSTANT half_period            : time    := " + to_string(10 / 2) + " NS;", 1);
	Common::write_line(ofs, "", 1);

	Common::write_line(ofs, "END ;");
	Common::write_line(ofs, "");
}

} // namespace filegen
} // namespace vesyla