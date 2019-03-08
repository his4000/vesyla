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

#include "SyncInstrGenerator.hpp"
namespace vesyla
{
namespace filegen
{

// This method gets a branch mode and returns the corresponding string
string getBranchModeStr(BIR::BIREnumerations::BranchMode branchMode_)
{
	string modeStr;
	switch (branchMode_)
	{
	case bmGreater:
		modeStr = "'Gr'  ";
		break;
	case bmGreaterOrEqual:
		modeStr = "'GrEq'";
		break;
	case bmEqual:
		modeStr = "'Eq'  ";
		break;
	}
	return modeStr;
}

// This method gets the component type and returns the corresponding string
string getComponentTypeStr(BIR::BIREnumerations::ComponentType compType_)
{
	string typeStr;
	switch (compType_)
	{
	case ctDPU:
		typeStr = "DPU ";
		break;
	case ctRegFile:
		typeStr = "REFI";
		break;
	case ctMemory:
		typeStr = "MEM ";
		break;
	}
	return typeStr;
}

// This method gets a RACCU mode and returns the corresponding string
string getRaccuModeStr(BIR::BIREnumerations::RACCUMode raccuMode_)
{
	string modeStr;
	switch (raccuMode_)
	{
	case rmAdd:
		modeStr = "'ADD'";
		break;
	case rmAddWithLoopIndex:
		modeStr = "'ADD_WITH_LOOP'";
		break;
	case rmShiftLeft:
		modeStr = "'SHIFT_L'";
		break;
	case rmLoopHeader:
		modeStr = "'LOOP_HEADER'";
		break;
	case rmLoopTail:
		modeStr = "'LOOP_TAIL'";
		break;
	case rmShiftRight:
		modeStr = "'SHIFT_R'";
		break;
	case rmSub:
		modeStr = "'SUB'";
		break;
	}
	return modeStr;
}

// This method gets "(D)" or "(S)" strings corresponding to the given input value.
string getStaticOrDynamic(bool isStatic_)
{
	return isStatic_ ? "(S)" : "(D)";
}

// This method dumps a component info
string dumpComponentInfo(BIR::ComponentInfo *componentInfo_)
{
	string str = "";
	str += ("(" + getComponentTypeStr(componentInfo_->type));
	str += ("<" + to_string(componentInfo_->cellCoordinate.row) + "," + to_string(componentInfo_->cellCoordinate.column) + ">, ");
	str += ("PtNo: " + to_string(componentInfo_->portNo) + ")");
	return str;
}

// This method dumps a BRANCH instruction
string dumpBranchInstruction(BIR::BranchInstruction *branchInst_)
{
	string str = "";
	str += ("Mode: " + getBranchModeStr(branchInst_->mode) + ", ");
	str += ("FalseAddr: " + to_string(branchInst_->false_pc));
	return str;
}

// This method dumps a DPU instruction
string dumpDpuInstruction(BIR::DPUInstruction *dpuInst_)
{
	string str = "";
	str += ("Mode: " + dpuInst_->mode + ", ");
	str += ("ModeValue: " + to_string(dpuInst_->modeValue) + ", ");
	str += ("ExecutionCycle: " + to_string(dpuInst_->executionCycle) + ", ");
	str += ("Repetition: " + to_string(dpuInst_->repetition));
	return str;
}

// This method dumps the common parts of all instructions.
string dumpInstructionHeader(BIR::Instruction *instruction_, int index_, int clkCounter_)
{
	std::stringstream str;
	str << setw(2) << index_ << " (" << setw(3) << clkCounter_ << "): ";

	string cmdName = "";

	switch (instruction_->kind())
	{
	case bktBranchInstruction:
		cmdName = " BRN    ";
		break;
	case bktDPUInstruction:
		cmdName = " DPU    ";
		break;
	case bktJumpInstruction:
		cmdName = " JUMP   ";
		break;
	case bktLoopHeaderInstruction:
		cmdName = "|LOOP_H|";
		break;
	case bktLoopTailInstruction:
		cmdName = "|LOOP_T|";
		break;
	case bktRACCUInstruction:
		cmdName = " RACCU  ";
		break;
	case bktRouteInstruction:
		cmdName = " ROUTE  ";
		break;
	case bktSWBInstruction:
		cmdName = " SWB    ";
		break;
	case bktWaitInstruction:
		cmdName = " WAIT   ";
		break;
	case bktRefiInstruction:
		cmdName = ((RefiInstruction *)instruction_)->isDimarchMode ? " MREFI  " : " REFI   ";
		break;
	case bktRefi1Instruction:
		cmdName = ((Refi1Instruction *)instruction_)->isDimarchMode ? " MREFI1 " : " REFI1  ";
		break;
	case bktRefi2Instruction:
		cmdName = ((Refi2Instruction *)instruction_)->isDimarchMode ? " MREFI2 " : " REFI2  ";
		break;
	case bktRefi3Instruction:
		cmdName = ((Refi3Instruction *)instruction_)->isDimarchMode ? " MREFI3 " : " REFI3  ";
		break;
	case bktSRAMInstruction:
		cmdName = ((SRAMInstruction *)instruction_)->isRead ? " SRAM_R " : " SRAM_W ";
		break;
	}

	str << cmdName << "| Sch: [" << setw(3) << instruction_->minScheduledClkCycle << " , " << setw(3) << instruction_->maxScheduledClkCycle << "] | ";
	return str.str();
}

// This method dumps a JUMP instruction
string dumpJumpInstruction(BIR::JumpInstruction *jumpInst_)
{
	string str = "";
	str += ("Address: " + to_string(jumpInst_->jump_pc) + ", ");
	return str;
}

// This method dumps a Loop Header instruction
string dumpLoopHeaderInstruction(BIR::LoopHeaderInstruction *loopHeaderInst_)
{
	string str = "";
	str += ("LoopId: " + to_string(loopHeaderInst_->loopId) + ",\t");
	str += ("StartValue: " + to_string(loopHeaderInst_->startValue) + ",\t");
	const string isStatic = loopHeaderInst_->isStatic ? "Yes" : "No";
	str += ("IsStatic: " + isStatic + ",\t");
	str += ("IterationNo: " + to_string(loopHeaderInst_->iterationNumber));
	return str;
}

// This method dumps a Loop Tail instruction
string dumpLoopTailInstruction(BIR::LoopTailInstruction *loopTailInst_)
{
	string str = "";
	str += ("LoopId: " + to_string(loopTailInst_->loopId) + ",\t");
	str += ("StepValue: " + to_string(loopTailInst_->stepValue) + ",\t");
	str += ("GoToPC: " + to_string(loopTailInst_->gotoPC));
	return str;
}

// This method dumps a SWB instruction
string dumpSwbInstruction(BIR::SWBInstruction *swbInst_)
{
	std::stringstream str;
	str << ("S:");
	str << dumpComponentInfo(&swbInst_->source);

	str << (", D:");
	str << dumpComponentInfo(&swbInst_->destination);
	return str.str();
}

string dumpRaccuInstruction(BIR::RACCUInstruction *raccuInst_)
{
	string str = "";
	str += ("Mode: " + getRaccuModeStr(raccuInst_->raccuMode) + ",\t");
	str += ("Op1: " + to_string(raccuInst_->operand1.value) + getStaticOrDynamic(raccuInst_->operand1.isStatic) + ",\t");
	str += ("Op2: " + to_string(raccuInst_->operand2.value) + getStaticOrDynamic(raccuInst_->operand2.isStatic) + ",\t");
	str += ("ResultAddress: " + to_string(raccuInst_->resultAddress));
	return str;
}

// This method dumps a REFI instruction
string dumpRefiInstruction(BIR::RefiInstruction *refiInst_)
{
	string str = "";
	str += ("PortMode: ");
	str += ((refiInst_->portMode == pmIn ? " 'in'" : "'out'"));
	str += (", ");
	str += ("AddressMode: ");
	str += ((refiInst_->addressMode == amLinear ? "'Linear'" : "'BitReverse'"));
	str += (", ");
	str += ("PortNo: " + to_string(refiInst_->portNo) + ", ");
	str += ("IDelay: " + to_string(refiInst_->initialDelay.value) + getStaticOrDynamic(refiInst_->initialDelay.isStatic) + ", ");
	str += ("MDelay: " + to_string(refiInst_->middleDelay.value) + getStaticOrDynamic(refiInst_->middleDelay.isStatic) + ", ");
	str += ("RDelay: " + to_string(refiInst_->repetitionDelay.value) + getStaticOrDynamic(refiInst_->repetitionDelay.isStatic) + ", ");
	str += ("StartAddress: " + to_string(refiInst_->startAddress.value) + getStaticOrDynamic(refiInst_->startAddress.isStatic) + ", ");
	str += ("StepValue: " + to_string(refiInst_->stepValue.value) + getStaticOrDynamic(refiInst_->stepValue.isStatic) + ", ");
	str += ("NoOfAddress: " + to_string(refiInst_->numberOfAddress.value) + getStaticOrDynamic(refiInst_->numberOfAddress.isStatic) + ", ");
	str += ("NoOfRepetition: " + to_string(refiInst_->numberOfRepetition.value) + getStaticOrDynamic(refiInst_->numberOfRepetition.isStatic) + ", ");
	str += ("RepOffset: " + to_string(refiInst_->repetitionOffset));
	return str;
}

// This method dumps a Refi1 instruction
string dumpRefi1Instruction(BIR::Refi1Instruction *refi1Inst_)
{
	string str = "";
	str += ("IDelay: " + to_string(refi1Inst_->initialDelay.value) + getStaticOrDynamic(refi1Inst_->initialDelay.isStatic) + ", ");
	str += ("PortNo: " + to_string(refi1Inst_->portNo) + ", ");
	str += ("StartAddress: " + to_string(refi1Inst_->startAddress.value) + getStaticOrDynamic(refi1Inst_->startAddress.isStatic) + ", ");
	str += ("AddressNo: " + to_string(refi1Inst_->numberOfAddress.value) + getStaticOrDynamic(refi1Inst_->numberOfAddress.isStatic));
	return str;
}

// This method dumps a Refi2 instruction
string dumpRefi2Instruction(BIR::Refi2Instruction *refi2Inst_)
{
	string str = "";
	str += ("MDelay: " + to_string(refi2Inst_->middleDelay.value) + getStaticOrDynamic(refi2Inst_->middleDelay.isStatic) + ", ");
	str += ("StepValue: " + to_string(refi2Inst_->stepValue.value) + getStaticOrDynamic(refi2Inst_->stepValue.isStatic) + ", ");
	str += ("RepetitionNo: " + to_string(refi2Inst_->numberOfRepetition.value) + getStaticOrDynamic(refi2Inst_->numberOfRepetition.isStatic) + ", ");
	str += ("RepetitionOffset: " + to_string(refi2Inst_->repetitionOffset));
	return str;
}

string dumpRefi3Instruction(BIR::Refi3Instruction *refi3Inst_)
{
	string str = "";
	str += ("RDelay: " + to_string(refi3Inst_->repetitionDelay.value) + getStaticOrDynamic(refi3Inst_->repetitionDelay.isStatic) + ", ");
	str += ("AddressMode: ");
	str += ((refi3Inst_->addressMode == amLinear ? "'Linear'" : "'BitReverse'"));
	str += (", ");
	str += ("MiddleDelayExt: " + to_string(refi3Inst_->middleDelayExt) + ", ");
	str += ("NoOfRepExt: " + to_string(refi3Inst_->numberOfRepetitionExt) + ", ");
	str += ("RepOffsetExt: " + to_string(refi3Inst_->repetitionOffsetExt) + ", ");
	str += ("Dimarch: " + to_string(refi3Inst_->isDimarchMode) + ", ");
	str += ("Compression: " + to_string(refi3Inst_->en_compression));
	return str;
}

// This method dumps a DiMArch ROUTE instruction
string dumpRouteInstruction(BIR::RouteInstruction *routeInst_)
{
	string str = "";
	str += ("Src: <" + to_string(routeInst_->sourceCoordinate.row) + "," + to_string(routeInst_->sourceCoordinate.column) + ">, ");
	str += ("Dst: <" + to_string(routeInst_->destCoordinate.row) + "," + to_string(routeInst_->destCoordinate.column) + ">, ");
	str += ("DRRA Select: " + to_string(routeInst_->drraSelect) + ", ");
	str += ("IsFromSrc: ");
	str += ((routeInst_->isFromSource ? "Y" : "N"));
	return str;
}

// This method dumps a DiMArch SRAM instruction
string dumpSramInstruction(BIR::SRAMInstruction *sramInst_)
{
	string str = "";
	str += ("IDelay: " + to_string(sramInst_->initialDelay.value) + getStaticOrDynamic(sramInst_->initialDelay.isStatic) + ", ");
	str += ("InitAddress: " + to_string(sramInst_->initialAddress.value) + getStaticOrDynamic(sramInst_->initialAddress.isStatic) + ", ");
	str += ("Loop1(");
	str += ("iter: " + to_string(sramInst_->loop1Iterations.value) + getStaticOrDynamic(sramInst_->loop1Iterations.isStatic) + ", ");
	str += ("inc: " + to_string(sramInst_->loop1Increment) + ", ");
	str += ("delay: " + to_string(sramInst_->loop1Delay.value) + getStaticOrDynamic(sramInst_->loop1Delay.isStatic) + "), ");
	str += ("Loop2(");
	str += ("iter: " + to_string(sramInst_->loop2Iterations.value) + getStaticOrDynamic(sramInst_->loop2Iterations.isStatic) + ", ");
	str += ("inc: " + to_string(sramInst_->loop2Increment) + ", ");
	str += ("delay: " + to_string(sramInst_->loop2Delay.value) + getStaticOrDynamic(sramInst_->loop2Delay.isStatic) + ")");
	return str;
}

// This method dumps a wait instruction
string dumpWaitInstruction(BIR::WaitInstruction *waitInst_)
{
	string str = "";
	str += ("WaitCycles:  " + to_string(waitInst_->numberOfCycles.value) + getStaticOrDynamic(waitInst_->numberOfCycles.isStatic));
	return str;
}

void SyncInstrGenerator::generate(map<string, vector<BIR::Instruction *>> instr_lists, string filename_)
{
	int utilized_drra_row;
	int utilized_drra_col;
	util::GlobalVar glv;
	CHECK(glv.geti("utilized_drra_row", utilized_drra_row));
	CHECK(glv.geti("utilized_drra_col", utilized_drra_col));

	ofstream ofs(filename_, ofstream::out);
	string str = R"(
################################################################################
#                      DRRA SYNCHRONIZED INSTRUCTIONS                          #
################################################################################
)";
	Common::write_line(ofs, str);
	for (int rowIndex = 0; rowIndex < utilized_drra_row; ++rowIndex)
	{
		for (int colIndex = 0; colIndex < utilized_drra_col; ++colIndex)
		{
			VIR::Coordinate coord(rowIndex, colIndex);
			if (instr_lists.find(coord.to_str()) != instr_lists.end())
			{
				vector<Instruction *> &instr_list = instr_lists[coord.to_str()];
				if (instr_list.size() > 0)
				{
					Common::write_line(ofs, "# ------------------------------------------------------------------------------");
					Common::write_line(ofs, "#                        CELL <" + to_string(rowIndex) + ", " + to_string(colIndex) + ">");
					Common::write_line(ofs, "# ------------------------------------------------------------------------------");
				}
				for (int instIndex = 0, offset = 0; instIndex < instr_list.size(); ++instIndex)
				{
					str = dump_instr(instr_list[instIndex], instIndex, offset);
					Common::write_line(ofs, str);
				}
				Common::write_line(ofs, "");
			}
		}
	}
}

// This method dumps a DRRA instruction
string SyncInstrGenerator::dump_instr(BIR::Instruction *instruction_, int index_, int &offset_)
{
	string str = "";
	static int clkCounter;
	static int saveClkCounter;

	if (index_ == 0)
		clkCounter = 0;
	else if (instruction_->kind() != bktRefi2Instruction && instruction_->kind() != bktRefi3Instruction)
		clkCounter++;

	str += dumpInstructionHeader(instruction_, index_ + offset_, clkCounter);
	if (instruction_->kind() == bktSRAMInstruction)
	{
		offset_ += 2;
	}

	if (instruction_->kind() == bktWaitInstruction && ((WaitInstruction *)instruction_)->numberOfCycles.isStatic)
		clkCounter += ((WaitInstruction *)instruction_)->numberOfCycles.value;

	// Update clock counter for branch/jump instruction
	if (instruction_->kind() == bktBranchInstruction)
		saveClkCounter = clkCounter;
	else if (instruction_->kind() == bktJumpInstruction)
		clkCounter = saveClkCounter;

	switch (instruction_->kind())
	{
	case bktBranchInstruction:
		str += dumpBranchInstruction((BranchInstruction *)instruction_);
		break;

	case bktDPUInstruction:
		str += dumpDpuInstruction((DPUInstruction *)instruction_);
		break;

	case bktJumpInstruction:
		str += dumpJumpInstruction((JumpInstruction *)instruction_);
		break;

	case bktLoopHeaderInstruction:
		str += dumpLoopHeaderInstruction((LoopHeaderInstruction *)instruction_);
		break;

	case bktLoopTailInstruction:
		str += dumpLoopTailInstruction((LoopTailInstruction *)instruction_);
		break;

	case bktRACCUInstruction:
		str += dumpRaccuInstruction((RACCUInstruction *)instruction_);
		break;

	case bktRefiInstruction:
		str += dumpRefiInstruction((RefiInstruction *)instruction_);
		break;

	case bktRefi1Instruction:
		str += dumpRefi1Instruction((Refi1Instruction *)instruction_);
		break;

	case bktRefi2Instruction:
		str += dumpRefi2Instruction((Refi2Instruction *)instruction_);
		break;

	case bktRefi3Instruction:
		str += dumpRefi3Instruction((Refi3Instruction *)instruction_);
		break;

	case bktRouteInstruction:
		str += dumpRouteInstruction((RouteInstruction *)instruction_);
		break;

	case bktSRAMInstruction:
		str += dumpSramInstruction((SRAMInstruction *)instruction_);
		break;

	case bktSWBInstruction:
		str += dumpSwbInstruction((SWBInstruction *)instruction_);
		break;

	case bktWaitInstruction:
		str += dumpWaitInstruction((WaitInstruction *)instruction_);
		break;
	}
	return str;
}
} // namespace filegen
} // namespace vesyla