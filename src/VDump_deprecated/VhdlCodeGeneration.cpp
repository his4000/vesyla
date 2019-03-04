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

#include "VDump.hpp"
#include "BIR/BIRIncludeAll.hpp"
#include "VIR/VIRIncludeAll.hpp"
#include "VManager/FileManager.hpp"
#include <iomanip>
#include <sstream>
#include <bitset>

using namespace BIR;
using namespace BIR::BIREnumerations;
using namespace VIR;
using namespace VIR::VirEnumerations;

// This method creates one line of the CellInfos constant to be dumped to the VHDL file.
string VDump::dumpVhdlCellInfo(int rowIndex_, int colIndex_, int rowStrLen_, int colStrLen_, int instIncValue_, bool isLastOne)
{
	string cellInfo = "";
	static int accInstructionNo = 0;

	accInstructionNo += instIncValue_;

	ostringstream strStream;

	if (accInstructionNo < 10)
		strStream.width(2);

	strStream << accInstructionNo;

	cellInfo += "(";
	cellInfo += strStream.str();
	cellInfo += ", " + getRowOrColSignature(rowIndex_, rowStrLen_);
	cellInfo += ", " + getRowOrColSignature(colIndex_, colStrLen_) + ")";

	if (!isLastOne)
		cellInfo += ", ";
	else
		cellInfo += ");";

	return cellInfo;
}

// This method creates a VHDL constant corresponding to the DRRA cell infos (instruction number, row, column)
void VDump::dumpVhdlCellInfos(BIR::DRRACluster *drraCluster_)
{
	DiMArch *dimarch = drraCluster_->diMArch();

	WriteLine("");
	WriteLine("TYPE cell_info_type IS RECORD");
	IncTab();
	WriteLine("inst_no : integer range 0 to instruction_no;");
	WriteLine("row : std_logic_vector(ROWS-1 downto 0);");
	WriteLine("col : std_logic_vector(COLUMNS-1 downto 0);");
	DecTab();
	WriteLine("END RECORD;");

	const string rowsStr = dimarch->hasInstruction() ? "(ROWS+1)" : "ROWS";
	WriteLine("TYPE cell_infos_type IS ARRAY(0 TO " + rowsStr + "*COLUMNS-1) OF cell_info_type;");

	WriteLine("");
	WriteLine("CONSTANT cell_infos : cell_infos_type := (");

	string cellInfos;

	IncTab();

	// Instructions
	for (int rowIndex = 0; rowIndex < drraCluster_->rowNum(); ++rowIndex)
	{
		cellInfos = "";

		for (int colIndex = 0; colIndex < drraCluster_->colNum(); ++colIndex)
		{
			int instruction_count = 0;
			for (auto &instr : drraCluster_->getDRRACell(rowIndex, colIndex)->instr_list)
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
			cellInfos += dumpVhdlCellInfo(rowIndex, colIndex, drraCluster_->rowNum(), drraCluster_->colNum(),
																		instruction_count,
																		(rowIndex == drraCluster_->rowNum() - 1 && colIndex == drraCluster_->colNum() - 1));
		}
		WriteLine(cellInfos);
	}

	DecTab();
}

// This method dumps the VHDL code for the instructions
void VDump::dumpVhdlCode(BIR::DRRACluster *drraCluster_, VIR::MainProgram *mainProgram_)
{
	dumpVhdlHeader(drraCluster_);

	WriteLine("");

	dumpVhdlInstructions(drraCluster_);

	DecTab();
	WriteLine("");
	WriteLine("END behavior;");
}

// This method dumps a BRANCH instruction to the VHDL file.
string VDump::dumpVhdlBranchInstruction(BIR::BranchInstruction *branchInst_)
{
	string instStr = "BRANCH(";

	instStr += to_string(branchInst_->modeValue) + ", "; // branch mode
	instStr += to_string(branchInst_->false_pc) + ")";	 // branch false address, calculated while synchronization

	return instStr;
}

/*
// This method dumps a DiMArch instruction to the VHDL file.
void VDump::dumpVhdlDimarchInstruction(BIR::Instruction * instruction_)
{
	switch (instruction_->kind())
	{
		case bktRouteInstruction:	dumpVhdlRouteInstruction((RouteInstruction *) instruction_); break;

		case bktSRAMInstruction:	dumpVhdlSramInstruction((SRAMInstruction *) instruction_);	 break;

		case bktRFILEInstruction:	dumpVhdlRfileInstruction((RFILEInstruction *) instruction_); break;
	}
}*/

// This method dumps a DPU instruction to the VHDL file.
string VDump::dumpVhdlDpuInstruction(BIR::DPUInstruction *dpuInst_)
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
			LOG(DEBUG) << "float = " << dpuInst_->float_imm_data;
			LOG(DEBUG) << "int = " << dpuInst_->constantValue;
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

// This method dumps a DRRA instruction to the VHDL file.
void VDump::dumpVhdlDrraInstruction(BIR::Instruction *instruction_)
{
	string instStr;

	if (instruction_->kind() == bktSRAMInstruction)
		dumpVhdlSramInstruction((SRAMInstruction *)instruction_);
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

		dumpVhdlInstruction(instStr);
	}
}

// This method writes a fixed header to the generated VHDL file.
void VDump::dumpVhdlHeader(BIR::DRRACluster *drraCluster_)
{
	string entityName = "testbench_" + InputFileName();

	WriteLine("LIBRARY IEEE;");
	WriteLine("USE IEEE.NUMERIC_STD.ALL;");
	WriteLine("USE IEEE.STD_LOGIC_1164.ALL;");
	WriteLine("USE WORK.top_consts_types_package.ROWS;");
	WriteLine("USE WORK.top_consts_types_package.COLUMNS;");
	WriteLine("USE WORK.top_consts_types_package.INSTR_WIDTH;");
	WriteLine("USE WORK.top_consts_types_package.REG_FILE_MEM_ADDR_WIDTH;");
	WriteLine("USE WORK.top_consts_types_package.REG_FILE_MEM_DATA_WIDTH;");
	WriteLine("USE WORK.top_consts_types_package.SRAM_ADDRESS_WIDTH;");
	WriteLine("USE WORK.top_consts_types_package.SRAM_WIDTH;");
	WriteLine("USE WORK.tb_instructions.ALL;");
	WriteLine("USE WORK.const_package_" + InputFileName() + ".ALL;");
	WriteLine("USE WORK.noc_types_n_constants.ALL;");
	WriteLine("");
	WriteLine("ENTITY " + entityName + " IS");
	WriteLine("END " + entityName + ";");
	WriteLine("");
	WriteLine("ARCHITECTURE behavior OF " + entityName + " IS");
	WriteLine("");

	IncTab();
	WriteLine("SIGNAL clk            : std_logic := '0';");
	WriteLine("SIGNAL rst_n          : std_logic := '0';");
	WriteLine("SIGNAL instr_load     : std_logic := '0';");
	WriteLine("SIGNAL instr_input    : std_logic_vector(INSTR_WIDTH downto 0);");
	WriteLine("SIGNAL seq_address_rb : std_logic_vector(ROWS-1 downto 0);");
	WriteLine("SIGNAL seq_address_cb : std_logic_vector(COLUMNS-1 downto 0);");
	WriteLine("");
	WriteLine("SIGNAL fabric_reg_wr_2		: std_logic := '0';");
	WriteLine("SIGNAL fabric_reg_rd_2		: std_logic := '0';");
	WriteLine("SIGNAL fabric_reg_wr_addr_2 : std_logic_vector(REG_FILE_MEM_ADDR_WIDTH-1 downto 0) := (OTHERS => '0');");
	WriteLine("SIGNAL fabric_reg_rd_addr_2 : std_logic_vector(REG_FILE_MEM_ADDR_WIDTH-1 downto 0) := (OTHERS => '0');");
	WriteLine("SIGNAL fabric_data_in_reg_2 : signed(REG_FILE_MEM_DATA_WIDTH-1 DOWNTO 0) := (OTHERS => '0');");
	WriteLine("SIGNAL fabric_data_out_2 	: signed(REG_FILE_MEM_DATA_WIDTH-1 DOWNTO 0);");
#ifndef SPECIAL_DUMP
	WriteLine("SIGNAL tb_or_dimarch 		: std_logic;");
	WriteLine("SIGNAL tb_en    : std_logic;");
	WriteLine("SIGNAL tb_addrs : std_logic_vector (SRAM_ADDRESS_WIDTH-1 downto 0);");
	WriteLine("SIGNAL tb_inp   : std_logic_vector (SRAM_WIDTH-1 downto 0);");
	WriteLine("SIGNAL tb_ROW   : unsigned (ROW_WIDTH-1 downto 0);");
	WriteLine("SIGNAL tb_COL   : unsigned (COL_WIDTH-1 downto 0);");
#endif
	WriteLine("");
	WriteLine("TYPE instruction_type IS ARRAY (0 TO instruction_no-1) OF std_logic_vector(INSTR_WIDTH downto 0);");
	WriteLine("SIGNAL instruction : instruction_type;");

	dumpVhdlCellInfos(drraCluster_);

	/*
	dumpVhdlMemoryInitialValues(drraCluster_->diMArch());
	dumpVhdlRegFileInitialValues(drraCluster_);
	*/

	WriteLine(_storage_image->print_for_testbench());

	DecTab();

	WriteLine("");
	WriteLine("BEGIN");
	WriteLine("");
	IncTab();
#ifndef SPECIAL_DUMP
	WriteLine("PRF : ENTITY work.profiler_" + InputFileName());
	IncTab();
	WriteLine("PORT MAP (clk	=> clk,");
	WriteLine("		  rst_n	=> rst_n);");
	DecTab();
	WriteLine("");
#endif

	WriteLine("DUT : ENTITY work.fabric");
	IncTab();
	WriteLine("PORT MAP (clk					=> clk,");
	WriteLine("          rst_n					=> rst_n,");
	WriteLine("          instr_ld				=> instr_load,");
	WriteLine("          instr_inp				=> instr_input,");
	WriteLine("          seq_address_rb		=> seq_address_rb,");
	WriteLine("          seq_address_cb		=> seq_address_cb,");
	WriteLine("          fabric_reg_wr_2		=> fabric_reg_wr_2,");
	WriteLine("          fabric_reg_rd_2		=> fabric_reg_rd_2,");
	WriteLine("          fabric_reg_wr_addr_2	=> fabric_reg_wr_addr_2,");
	WriteLine("          fabric_reg_rd_addr_2	=> fabric_reg_rd_addr_2,");
	WriteLine("          fabric_data_in_reg_2	=> fabric_data_in_reg_2,");
#ifndef SPECIAL_DUMP
	WriteLine("          fabric_data_out_2		=> fabric_data_out_2,");
	WriteLine("          tb_or_dimarch			=> tb_or_dimarch,");
	WriteLine("          tb_en 				=> tb_en,");
	WriteLine("          tb_addrs				=> tb_addrs,");
	WriteLine("          tb_inp  				=> tb_inp,");
	WriteLine("          tb_ROW 				=> tb_ROW,");
	WriteLine("          tb_COL  				=> tb_COL);");
#else
	WriteLine("          fabric_data_out_2		=> fabric_data_out_2);");
#endif

	DecTab();

	WriteLine("");
	WriteLine("rst_n <= '0' AFTER 2.5 ns, '1' AFTER 4 ns;");
	WriteLine("clk   <= NOT clk AFTER half_period;");
	WriteLine("");

	WriteLine("StimuliSequencer : PROCESS (clk, rst_n)");
	IncTab();
	WriteLine("VARIABLE inst_counter	  : integer := 0;");
	WriteLine("VARIABLE curr_cell		  : integer := 0;");

	if (drraCluster_->diMArch()->totalUsedMemWords() > 0)
		WriteLine("VARIABLE mem_load_counter : integer := 0;");

	WriteLine("VARIABLE reg_load_counter : integer := 0;");
	DecTab();
	WriteLine("BEGIN");
	IncTab();
	WriteLine("IF (rst_n = '0') THEN");
	IncTab();
	WriteLine("inst_counter	 := 0;");
	WriteLine("curr_cell		 := 0;");

	if (drraCluster_->diMArch()->totalUsedMemWords() > 0)
		WriteLine("mem_load_counter := 0;");

	WriteLine("reg_load_counter := 0;");
	WriteLine("instr_load     <= '0';");
	WriteLine("instr_input    <= (OTHERS =>'0');");
	WriteLine("seq_address_rb <= (OTHERS =>'0');");
	WriteLine("seq_address_cb <= (OTHERS =>'0');");
#ifndef SPECIAL_DUMP
	WriteLine("tb_or_dimarch  <= '0';");
	WriteLine("tb_en 	 <= '0';");
	WriteLine("tb_addrs <= (OTHERS => '0');");
	WriteLine("tb_inp	 <= (OTHERS => '0');");
	WriteLine("tb_ROW 	 <= (OTHERS => '0');");
	WriteLine("tb_COL 	 <= (OTHERS => '0');");
#endif
	DecTab();
	WriteLine("ELSIF clk'EVENT AND clk = '0' THEN");
	IncTab();

#ifndef SPECIAL_DUMP
	WriteLine("");
	WriteLine("tb_en <= '0';");
#endif

	WriteLine("");

	// Memory load part
	if (drraCluster_->diMArch()->totalUsedMemWords() > 0)
	{
		WriteLine("IF (mem_load_counter < mem_load_cycles) THEN");

		dumpVhdlMemoryLoad();

		WriteLine("ELSIF (reg_load_counter < reg_load_cycles) THEN");
	}
	else
	{
		WriteLine("IF (reg_load_counter < reg_load_cycles) THEN");
	}

	// Register file load part
	dumpVhdlRegFileLoad(drraCluster_);

	// Instruction load part
	WriteLine("ELSIF (inst_counter >= 0 AND inst_counter < instruction_no) THEN");
	IncTab();
#ifndef SPECIAL_DUMP
	WriteLine("");
	WriteLine("tb_or_dimarch  <= '1';");
#endif
	WriteLine("");
	WriteLine("WHILE (inst_counter >= cell_infos(curr_cell).inst_no) LOOP");
	IncTab();
	WriteLine("curr_cell := curr_cell + 1;");
	DecTab();
	WriteLine("END LOOP;");
	WriteLine("");
	WriteLine("seq_address_rb <= cell_infos(curr_cell).row;");
	WriteLine("seq_address_cb <= cell_infos(curr_cell).col;");
	WriteLine("instr_load     <= '1';");
	WriteLine("instr_input    <= std_logic_vector(instruction(inst_counter));");
	WriteLine("inst_counter   := inst_counter + 1;");
	WriteLine("");
	DecTab();
	WriteLine("ELSE");
	IncTab();
	WriteLine("instr_load      <= '0';");
	WriteLine("fabric_reg_wr_2 <= '0';");
	DecTab();
	WriteLine("END IF;");
	DecTab();
	WriteLine("END IF;");
	DecTab();
	WriteLine("END PROCESS StimuliSequencer;");
}

// This method dumps the given instruction string to the VHDL file.
// It also increments the instruction index automatically.
void VDump::dumpVhdlInstruction(string instStr_)
{
	static int instructionIndex = 0;

	WriteLine("");
	WriteLine("instruction(" + to_string(instructionIndex++) + ") <= " + instStr_ + ";");
}

// This method dumps all sequencer instructions to the VHDL file.
void VDump::dumpVhdlInstructions(BIR::DRRACluster *drraCluster_)
{
	DiMArch *dimarch = drraCluster_->diMArch();

	WriteLine("InstSequencer : PROCESS IS");

	drraCluster_->hasMemoryInstructions(true);
	// Variable declaration for DiMArch
	if (drraCluster_->hasMemoryInstructions())
	{
		IncTab();

		WriteLine("");
		WriteLine("VARIABLE AGU_INST   : AGU_INST_TYPE;");
		WriteLine("");

		DecTab();
	}

	WriteLine("BEGIN");
	IncTab();

	/* REMOVE:
	// DiMArch instructions
	if (dimarch->hasInstruction())
	{
		for (int colIndex = 0; colIndex < dimarch->colNum(); ++colIndex)
		{
			InstructionList * instList = & dimarch->getContileCell(colIndex)->instructions();

			if (instList->size() > 0)
			{
				dumpSequencerHeader("CONTILE", 0, colIndex, "--");

				for (int instIndex = 0; instIndex < instList->size(); ++instIndex)
					dumpVhdlDimarchInstruction(instList->instruction(instIndex));
			}
		}
	} */

	// DRRA instructions
	for (int rowIndex = 0; rowIndex < drraCluster_->rowNum(); ++rowIndex)
		for (int colIndex = 0; colIndex < drraCluster_->colNum(); ++colIndex)
		{
			vector<Instruction *> &instr_list = drraCluster_->getDRRACell(rowIndex, colIndex)->instr_list;
			if (instr_list.size() > 0)
			{
				dumpSequencerHeader("Sequencer", rowIndex, colIndex, "--");

				for (int instIndex = 0; instIndex < instr_list.size(); ++instIndex)
					dumpVhdlDrraInstruction(instr_list[instIndex]);
			}
		}

	WriteLine("");
	WriteLine("WAIT;");
	WriteLine("");

	DecTab();
	WriteLine("END PROCESS InstSequencer;");
}

// This method dumps a JUMP instruction to the VHDL file.
string VDump::dumpVhdlJumpInstruction(BIR::JumpInstruction *jumpInst_)
{
	string instStr = "JUMP(";

	instStr += to_string(jumpInst_->jump_pc) + ")"; // jump address, calculated in synchronization

	return instStr;
}

// This method dumps a Loop Header instruction to the VHDL file.
string VDump::dumpVhdlLoopHeaderInstruction(BIR::LoopHeaderInstruction *loopHeaderInst_)
{
	string instStr = "LOOP_HEADER(";

	instStr += to_string(loopHeaderInst_->loopId) + ", ";						// index_raccu_addr
	instStr += to_string(loopHeaderInst_->startValue) + ", ";				// index_start
	instStr += to_string(loopHeaderInst_->isStatic ? 0 : 1) + ", "; // iter_no_sd
	instStr += to_string(loopHeaderInst_->iterationNumber) + ")";		// iter_no

	return instStr;
}

// This method dumps a Loop Tail instruction to the VHDL file.
string VDump::dumpVhdlLoopTailInstruction(BIR::LoopTailInstruction *loopTailInst_)
{
	string instStr = "LOOP_TAIL(";

	instStr += to_string(loopTailInst_->stepValue) + ", "; // index_step
	instStr += to_string(loopTailInst_->gotoPC) + ", ";		 // pc_togo
	instStr += to_string(loopTailInst_->loopId) + ")";		 // index_raccu_addr

	return instStr;
}

// This method writes the memory initial values into a VHDL array which is used later by the memory load.
void VDump::dumpVhdlMemoryInitialValues(BIR::DiMArch *dimarch_)
{
	// Return when memory is not used.
	if (dimarch_->totalUsedMemWords() == 0)
		return;

	WriteLine("");
	WriteLine("TYPE mem_value_type IS RECORD");
	IncTab();
	WriteLine("address : std_logic_vector (SRAM_ADDRESS_WIDTH-1 downto 0);");
	WriteLine("row     : unsigned (ROW_WIDTH-1 downto 0);");
	WriteLine("col     : unsigned (COL_WIDTH-1 downto 0);");
	WriteLine("data   : std_logic_vector (SRAM_WIDTH-1 downto 0);");
	DecTab();
	WriteLine("END RECORD;");
	WriteLine("TYPE mem_values_type IS ARRAY (0 to mem_load_cycles-1) OF mem_value_type;");
	WriteLine("");
	WriteLine("CONSTANT mem_init_values : mem_values_type := (");

	string initialValues;
	bool isFirstLine = true;
	int counter = 0;

	IncTab();

	for (int rowIndex = 0; rowIndex < dimarch_->rowNum(); ++rowIndex)
	{
		for (int colIndex = 0; colIndex < dimarch_->colNum(); ++colIndex)
		{
			SRAM *sram = dimarch_->getDiMArchCell(rowIndex, colIndex)->sram();

			if (sram->hasInitialLoad())
				for (int index = 0; index < sram->depth(); ++index)
				{
					MemElement *memElement = sram->memory(index);

					if (memElement->hasInitialValue())
					{
						initialValues = !isFirstLine ? ", " : "  ";
						initialValues += to_string(counter++) + " => (";
						initialValues += intToBinaryStr<cSramAddressWidth>(index) + ", ";
						initialValues += intToBinaryStr<cRowBitWidth>(getActualDimarchRow(rowIndex)) + ", ";
						initialValues += intToBinaryStr<cColBitWidth>(colIndex) + ", ";

						initialValues += "X\"";

						if (!memElement->isFixedPoint())
						{
							const int valuesCount = memElement->intValues().size();

							for (int subIndex = valuesCount - 1; subIndex >= 0; --subIndex)
								initialValues += intToHexStr(memElement->intValues().at(subIndex), 4);
						}
						else
						{
							const int valuesCount = memElement->realValues().size();

							for (int subIndex = valuesCount - 1; subIndex >= 0; --subIndex)
								initialValues += realToHexStr(memElement->realValues().at(subIndex), 4);
						}

						initialValues += +"\")";

						WriteLine(initialValues);

						if (isFirstLine)
							isFirstLine = false;
					}
				}
		}
	}

	DecTab();

	WriteLine(");");
}

// This method writes the memory load part to the VHDL file.
void VDump::dumpVhdlMemoryLoad()
{
	IncTab();
	WriteLine("");

	WriteLine("instr_load <= '0';");
	WriteLine("tb_en      <= '1';");
	WriteLine("tb_addrs   <= mem_init_values(mem_load_counter).address;");
	WriteLine("tb_ROW     <= mem_init_values(mem_load_counter).row;");
	WriteLine("tb_COL     <= mem_init_values(mem_load_counter).col;");
	WriteLine("tb_inp     <= mem_init_values(mem_load_counter).data;");
	WriteLine("");
	WriteLine("mem_load_counter := mem_load_counter + 1;");
	WriteLine("");
	DecTab();
}

// This method creates a VHDL file for a package containing some application specific constants.
void VDump::dumpVhdlPackageCode(BIR::DRRACluster *drraCluster_)
{
	string packageName = "const_package_" + InputFileName();

	vesyla::util::GlobalVar glv;
	int reg_file_depth;
	int mem_block_size = 16;
	CHECK(glv.geti("hw_setting.fabric_setting.reg_file_depth", reg_file_depth));
	int reg_file_bank_no = reg_file_depth / mem_block_size;

	WriteLine("PACKAGE " + packageName + " IS");
	IncTab();
	WriteLine("");
	WriteLine("CONSTANT instruction_no         : integer := " + to_string(drraCluster_->instr_count()) + ";");
	WriteLine("CONSTANT mem_load_cycles        : integer := " + to_string(drraCluster_->diMArch()->totalUsedMemWords()) + ";");
	WriteLine("CONSTANT reg_load_cycles        : integer := " + to_string(drraCluster_->totalUsedRegFiles() * reg_file_bank_no) + ";"); // 2 is the number of banks
	WriteLine("CONSTANT execution_start_cycle  : integer := instruction_no + reg_load_cycles + mem_load_cycles + 1;");
	WriteLine("CONSTANT total_execution_cycle  : integer := " + to_string(_totalExecutionCycle) + ";");
	WriteLine("CONSTANT period                 : time    := " + to_string(cDefaultClkPeriod) + " NS;");
	WriteLine("CONSTANT half_period            : time    := " + to_string(cDefaultClkPeriod / 2) + " NS;");
	WriteLine("");
	DecTab();
	WriteLine("END " + packageName + ";");
	WriteLine("");
}

// This method dumps a RACCU instruction to the VHDL file.
string VDump::dumpVhdlRaccuInstruction(BIR::RACCUInstruction *raccuInst_)
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
string VDump::dumpVhdlRefi1Instruction(BIR::Refi1Instruction *refi1Inst_)
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
string VDump::dumpVhdlRefi2Instruction(BIR::Refi2Instruction *refi2Inst_)
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
string VDump::dumpVhdlRefi3Instruction(BIR::Refi3Instruction *refi3Inst_)
{
	string instStr = "REFI_3(";

	instStr += to_string((int)!refi3Inst_->repetitionDelay.isStatic) + ", ";	// rpt_delay_sd
	instStr += to_string(refi3Inst_->repetitionDelay.value) + ", ";						// rpt_delay
	instStr += to_string(refi3Inst_->addressMode == amLinear ? 0 : 1) + ", "; // mode
	instStr += to_string(3) + ", ";																						// outp_cntrl
	instStr += to_string(refi3Inst_->reverse_bits.isStatic ? 0 : 1) + ", ";		// reverse_bits_sd
	instStr += to_string(refi3Inst_->reverse_bits.value) + ", ";							// reverse_bits
	instStr += to_string(refi3Inst_->middleDelayExt) + ", ";									// refi_middle_delay_ext
	instStr += to_string(refi3Inst_->numberOfRepetitionExt) + ", ";						// no_of_rpt_ext
	instStr += to_string(refi3Inst_->repetitionOffsetExt) + ", ";							// rpt_step_value_ext
	instStr += to_string(!refi3Inst_->isDimarchMode ? 0 : 1) + ")";						// DiMAarch mode

	return instStr;
}

// This method writes the register file initial values into a VHDL array which is used later by the register file load.
void VDump::dumpVhdlRegFileInitialValues(BIR::DRRACluster *drraCluster_)
{
	WriteLine("");
	WriteLine("TYPE reg_value_type IS RECORD");
	IncTab();
	WriteLine("address : std_logic_vector (REG_FILE_MEM_ADDR_WIDTH-1 downto 0);");
	WriteLine("row     : std_logic_vector (ROWS-1 downto 0);");
	WriteLine("col     : std_logic_vector (COLUMNS-1 downto 0);");
	WriteLine("data    : signed (REG_FILE_MEM_DATA_WIDTH-1 downto 0);");
	DecTab();
	WriteLine("END RECORD;");
	WriteLine("TYPE reg_values_type IS ARRAY (0 to reg_load_cycles-1) OF reg_value_type;");
	WriteLine("");
	WriteLine("CONSTANT reg_init_values : reg_values_type := (");

	string initialValues;
	bool commaFlag = false;

	IncTab();

	vesyla::util::GlobalVar glv;
	int reg_file_depth;
	int mem_block_size = 16;
	CHECK(glv.geti("hw_setting.fabric_setting.reg_file_depth", reg_file_depth));

	for (int rowIndex = 0; rowIndex < drraCluster_->rowNum(); ++rowIndex)
	{
		for (int colIndex = 0; colIndex < drraCluster_->colNum(); ++colIndex)
		{
			RegFile *regFile = drraCluster_->getDRRACell(rowIndex, colIndex)->regFile();

			const int bankNumber = reg_file_depth / mem_block_size;
			const int bankCapacity = mem_block_size;
			const int bankIndexWidth = (int)(ceil(log2(bankNumber)));
			int bankLimit;

			if (regFile->hasInitialLoad())
				for (int bankIndex = 0; bankIndex < bankNumber; ++bankIndex)
				{
					initialValues = commaFlag ? ", (" : "  (";
					bitset<32> bin_biankIndex(bankIndex);
					initialValues += "\"" + bin_biankIndex.to_string().substr(32 - bankIndexWidth) + "\", ";
					initialValues += getRowOrColSignature(rowIndex, drraCluster_->rowNum()) + ", ";
					initialValues += getRowOrColSignature(colIndex, drraCluster_->colNum()) + ", ";

					initialValues += "X\"";

					bankLimit = (bankIndex + 1) * bankCapacity;

					for (int regIndex = bankLimit - 1; regIndex >= bankIndex * bankCapacity; --regIndex)
					{
						// TODO: CHANGE TO: remove the if statement and use just the THEN part.
						if (regIndex < regFile->regDepth())
						{
							Register *reg = regFile->registers(regIndex);

							initialValues += !reg->isFixedPoint ? intToHexStr(reg->intValue, 4) : realToHexStr(reg->realValue, 4);
						}
						else
							initialValues += "0000";
					}

					initialValues += +"\")";

					WriteLine(initialValues);

					if (!commaFlag)
						commaFlag = true;
				}
		}
	}

	DecTab();

	WriteLine(");");
}

// This method writes the register load part (initializing register files) to the VHDL file.
void VDump::dumpVhdlRegFileLoad(BIR::DRRACluster *drraCluster_)
{
	IncTab();
	WriteLine("");
	WriteLine("instr_load			 <= '0';");
	WriteLine("instr_input			 <= (OTHERS =>'0');");
	WriteLine("fabric_reg_wr_2		 <= '1';");
	WriteLine("seq_address_rb       <= reg_init_values(reg_load_counter).row;");
	WriteLine("seq_address_cb       <= reg_init_values(reg_load_counter).col;");
	WriteLine("fabric_reg_wr_addr_2 <= reg_init_values(reg_load_counter).address;");
	WriteLine("fabric_data_in_reg_2 <= reg_init_values(reg_load_counter).data;");
	WriteLine("");
	WriteLine("reg_load_counter := reg_load_counter + 1;");
	WriteLine("");
	DecTab();
}

// This method dumps a RFILE instruction to the VHDL file.
void VDump::dumpVhdlRfileInstruction(BIR::RFILEInstruction *rfileInst_)
{
	string instStr;

	instStr = "RFILE_INST := RFILEAGU(";

	instStr += rfileInst_->drraRow == 0 ? "'0'" : "'1'";
	instStr += ", ";
	instStr += rfileInst_->isRead ? "'0'" : "'1'";
	instStr += ", ";
	instStr += rfileInst_->addressMode == amLinear ? "0" : "1";
	instStr += ", ";

	instStr += to_string(rfileInst_->startAddress) + ", ";
	instStr += to_string(rfileInst_->endAddress) + ", ";

	instStr += rfileInst_->increment >= 0 ? "'1'" : "'0'";
	instStr += ", ";

	instStr += to_string(abs(rfileInst_->increment)) + ", ";
	instStr += to_string(rfileInst_->initialDelay) + ", ";
	instStr += to_string(rfileInst_->repetitionDelay) + ", ";
	instStr += to_string(rfileInst_->numberOfRepetition) + ", ";

	instStr += rfileInst_->repetitionOffset >= 0 ? "'1'" : "'0'";
	instStr += ", ";

	instStr += to_string(abs(rfileInst_->repetitionOffset)) + ", ";
	instStr += to_string(rfileInst_->middleDelay) + ");";

	WriteLine("");
	WriteLine(instStr);

	dumpVhdlInstruction("RFILE_INST(0)");
	dumpVhdlInstruction("RFILE_INST(1)");
}

// This method dumps a ROUTE instruction to the VHDL file.
string VDump::dumpVhdlRouteInstruction(BIR::RouteInstruction *routeInst_)
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

// This method dumps an SRAM instruction to the VHDL file.
void VDump::dumpVhdlSramInstruction(BIR::SRAMInstruction *sramInst_)
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

	WriteLine("");
	WriteLine(instStr);

	dumpVhdlInstruction("AGU_INST(0)");
	dumpVhdlInstruction("AGU_INST(1)");
	dumpVhdlInstruction("AGU_INST(2)");
}

// This method dumps a SWB instruction to the VHDL file.
string VDump::dumpVhdlSwbInstruction(BIR::SWBInstruction *swbInst_)
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
string VDump::dumpVhdlWaitInstruction(BIR::WaitInstruction *waitInst_)
{
	string instStr = "DELAY(";

	instStr += to_string((int)!waitInst_->numberOfCycles.isStatic) + ", "; // del_cycles_sd
	instStr += to_string(waitInst_->numberOfCycles.value) + ")";					 // del_cycles

	return instStr;
}
