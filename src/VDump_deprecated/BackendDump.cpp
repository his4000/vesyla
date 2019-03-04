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
#include <stack>

#include <iostream>

using namespace BIR;
using namespace BIR::BIREnumerations;
using namespace VIR;
using namespace VIR::VirEnumerations;

// TODO: add the make_graph function in the VDump.cpp

// This method calculates the timing model of the program.
void VDump::calculateTimingModel(BIR::DRRACluster *drraCluster_, VariableScheduledValues &variableScheduledValues_)
{
	int maxScheduledCycle = 0;

	for (int rowIndex = 0; rowIndex < drraCluster_->rowNum(); ++rowIndex)
		for (int colIndex = 0; colIndex < drraCluster_->colNum(); ++colIndex)
		{
			vector<Instruction *> &instList = drraCluster_->getDRRACell(rowIndex, colIndex)->instr_list;
			if (instList.size() <= 0)
			{
				continue;
			}
			vector<int> end_time_list(instList.size(), 0);
			vector<int> loop_flag_list(instList.size(), 0);
			for (int instIndex = 0; instIndex < instList.size(); ++instIndex)
			{
				Instruction *currInst = instList[instIndex];
				end_time_list[instIndex] = currInst->end_time;
				switch (currInst->kind())
				{
				case bktLoopHeaderInstruction:
					loop_flag_list[instIndex] = 1;
					break;
				case bktLoopTailInstruction:
					loop_flag_list[instIndex] = 2;
					break;
				default:
					loop_flag_list[instIndex] = 0;
					break;
				}
			}

			bool find_loop_flag = 1;
			while (find_loop_flag)
			{
				find_loop_flag = false;
				int last_loop_header_loc = 0;
				for (int i = 0; i < loop_flag_list.size(); i++)
				{
					if (loop_flag_list[i] == 1)
					{
						find_loop_flag = true;
						last_loop_header_loc = i;
					}
					if (loop_flag_list[i] == 2)
					{
						int loop_exec_time = end_time_list[i] - end_time_list[last_loop_header_loc] + 1;
						end_time_list[i] = end_time_list[i] + loop_exec_time * (((LoopHeaderInstruction *)(instList[last_loop_header_loc]))->iterationNumber - 1) + 1;
						int time_increment = end_time_list[i] - end_time_list[last_loop_header_loc] - loop_exec_time;
						for (int j = i + 1; j < loop_flag_list.size(); j++)
						{
							end_time_list[j] += time_increment;
						}
						loop_flag_list[last_loop_header_loc] = 0;
						loop_flag_list[i] = 0;
						break;
					}
				}
			}
			if (maxScheduledCycle < end_time_list[end_time_list.size() - 1])
			{
				maxScheduledCycle = end_time_list[end_time_list.size() - 1];
			}
		}

	// for (int rowIndex = 0; rowIndex < drraCluster_->rowNum(); ++rowIndex)
	// 	for (int colIndex = 0; colIndex < drraCluster_->colNum(); ++colIndex)
	// 	{

	// 		vector<Instruction *> &instList = drraCluster_->getDRRACell(rowIndex, colIndex)->instr_list;
	// 		vector<int> end_time_list(instList.size());
	// 		vector<int> loop_flag_list(instList.size());
	// 		int loopExecutions = 0;

	// 		for (int instIndex = 0; instIndex < instList.size(); ++instIndex)
	// 		{
	// 			Instruction *currInst = instList[instIndex];
	// 			// get maxScheduledCyle by checking the end_time of all instrucitons
	// 			if (maxScheduledCycle < currInst->end_time)
	// 			{
	// 				maxScheduledCycle = currInst->end_time;
	// 			}

	// 			switch (currInst->kind())
	// 			{
	// 			case bktLoopHeaderInstruction:
	// 			{
	// 				LoopHeaderInstruction *loopHeaderInst = (LoopHeaderInstruction *)currInst;

	// 				// Calculate the execution cycle of the current loop

	// 				int totalIteration = loopHeaderInst->iterationNumber - 1;
	// 				LoopHeaderInstruction *upperLoop = loopHeaderInst->upperLoop;

	// 				while (upperLoop)
	// 				{

	// 					totalIteration *= upperLoop->iterationNumber;
	// 					upperLoop = upperLoop->upperLoop;
	// 				}

	// 				loopExecutions += totalIteration * loopHeaderInst->loopExecution;
	// 				LOG(DEBUG) << loopExecutions << "!!!!!!!!!!!!!!!!!" << totalIteration;
	// 				// if (loopExecutions > maxScheduledCycle)
	// 				// {
	// 				// 	maxScheduledCycle = loopExecutions;
	// 				// }
	// 			}
	// 			break;

	// 			case bktRefiInstruction:
	// 			{
	// 				string varName = ((RefiInstruction *)currInst)->pureVariableName;

	// 				ScheduledValues &scheduledValues = variableScheduledValues_[varName];

	// 				int maxCycle = currInst->maxScheduledClkCycle + loopExecutions;

	// 				if (scheduledValues.maxCycle < maxCycle)
	// 					scheduledValues.maxCycle = maxCycle;

	// 				if (scheduledValues.minCycle > currInst->minScheduledClkCycle)
	// 					scheduledValues.minCycle = currInst->minScheduledClkCycle;
	// 			}
	// 			break;

	// 			default:
	// 				break;
	// 			}
	// 		}
	// 	}

	vesyla::util::GlobalVar glv;
	int max_exec_time;
	CHECK(glv.geti("max_exec_time", max_exec_time));
	calculateTotalExecutionCycle(drraCluster_, max_exec_time);
	LOG(WARNING) << "Total execution time = " << _totalExecutionCycle << " cycles";
}

// This method calculates the total amount of execution cycles requires for the given program
void VDump::calculateTotalExecutionCycle(BIR::DRRACluster *drraCluster_, int maxScheduledCycle_)
{
	vesyla::util::GlobalVar glv;
	int reg_file_depth;
	int reg_file_width;
	int sram_width;
	CHECK(glv.geti("reg_file_depth", reg_file_depth));
	CHECK(glv.geti("reg_file_width", reg_file_width));
	CHECK(glv.geti("sram_width", sram_width));
	int reg_file_bank_no = reg_file_depth / (sram_width / reg_file_depth);
	_totalExecutionCycle = maxScheduledCycle_ + drraCluster_->instr_count() +
												 drraCluster_->diMArch()->totalUsedMemWords() + drraCluster_->totalUsedRegFiles() * reg_file_bank_no + 10; // 10 for extra overheads
}

// This method generates the proper testbench codes for the given application in DRRA
void VDump::drraCodeGeneration(BIR::DRRACluster *drraCluster_, VIR::MainProgram *mainProgram_)
{
	_totalExecutionCycle = cDefaultExecutionCycle;

	// _totalExecutionCycle is updated inside timing model calculation
	SetCurrentFile(ftTimingModel);
	dumpTimingModel(drraCluster_, mainProgram_);
	RestoreCurrentFile();

	SetCurrentFile(ftMatlab);
	dumpMatlabCode(drraCluster_, mainProgram_);
	RestoreCurrentFile();

	SetCurrentFile(ftProfiler);
	dumpProfilerCode(drraCluster_, mainProgram_);
	RestoreCurrentFile();

	SetCurrentFile(ftRunCmd);
	dumpRunCmdFile();
	RestoreCurrentFile();

	SetCurrentFile(ftRunGui);
	dumpRunGuiFile();
	RestoreCurrentFile();

	SetCurrentFile(ftVhdlTestbench);
	dumpVhdlCode(drraCluster_, mainProgram_);
	RestoreCurrentFile();

	SetCurrentFile(ftVhdlPackage);
	dumpVhdlPackageCode(drraCluster_);
	RestoreCurrentFile();

	SetCurrentFile(ftWave);
	dumpWaveFile(drraCluster_);
	RestoreCurrentFile();
}

// This method dumps different parts of a DRRA cluster
void VDump::dumpDRRACluster(BIR::DRRACluster *drraCluster_)
{
	SetCurrentFile(ftSyncInstructions);
	dumpSynchronizedInstructions(drraCluster_);
	RestoreCurrentFile();
}

// This method dumps a component info
void VDump::dumpComponentInfo(BIR::ComponentInfo *componentInfo_)
{
	wtToken("(" + getComponentTypeStr(componentInfo_->type));
	wtToken("<" + to_string(componentInfo_->cellCoordinate.row) + "," + to_string(componentInfo_->cellCoordinate.column) + ">, ");
	wtToken("PtNo: " + to_string(componentInfo_->portNo) + ")");
}

// This method dumps a BRANCH instruction
void VDump::dumpBranchInstruction(BIR::BranchInstruction *branchInst_)
{
	wtToken("Mode: " + getBranchModeStr(branchInst_->mode) + ", ");
	wtToken("FalseAddr: " + to_string(branchInst_->false_pc)); // +1 means branch to the next instruction after jump
}

// This method dumps a BRANCH instruction as an operation in the scheduling file
void VDump::dumpBranchOperation(BIR::BranchInstruction *branchInst_)
{
	wtToken("BRN:  ");
	wtToken("StatNo:  " + to_string(branchInst_->statementNo));
}

// This method dumps a DPU instruction
void VDump::dumpDpuInstruction(BIR::DPUInstruction *dpuInst_)
{
	wtToken("Mode: " + dpuInst_->mode + ", ");
	wtToken("ModeValue: " + to_string(dpuInst_->modeValue) + ", ");
	wtToken("ExecutionCycle: " + to_string(dpuInst_->executionCycle) + ", ");
	wtToken("Repetition: " + to_string(dpuInst_->repetition));
}

// This method dumps a DPU instruction as an operation in the scheduling file
void VDump::dumpDpuOperation(BIR::DPUInstruction *dpuInst_)
{
	wtToken("DPU:  ");
	wtToken("StatNo:  " + to_string(dpuInst_->statementNo));
}

// This method dumps a DRRA instruction
void VDump::dumpDrraInstruction(BIR::Instruction *instruction_, int index_, int &offset_)
{
	static int clkCounter;
	static int saveClkCounter;

	if (index_ == 0)
		clkCounter = 0;
	else if (instruction_->kind() != bktRefi2Instruction && instruction_->kind() != bktRefi3Instruction)
		clkCounter++;

	dumpInstructionHeader(instruction_, index_ + offset_, clkCounter);
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
		dumpBranchInstruction((BranchInstruction *)instruction_);
		break;

	case bktDPUInstruction:
		dumpDpuInstruction((DPUInstruction *)instruction_);
		break;

	case bktJumpInstruction:
		dumpJumpInstruction((JumpInstruction *)instruction_);
		break;

	case bktLoopHeaderInstruction:
		dumpLoopHeaderInstruction((LoopHeaderInstruction *)instruction_);
		break;

	case bktLoopTailInstruction:
		dumpLoopTailInstruction((LoopTailInstruction *)instruction_);
		break;

	case bktRACCUInstruction:
		dumpRaccuInstruction((RACCUInstruction *)instruction_);
		break;

	case bktRefiInstruction:
		dumpRefiInstruction((RefiInstruction *)instruction_);
		break;

	case bktRefi1Instruction:
		dumpRefi1Instruction((Refi1Instruction *)instruction_);
		break;

	case bktRefi2Instruction:
		dumpRefi2Instruction((Refi2Instruction *)instruction_);
		break;

	case bktRefi3Instruction:
		dumpRefi3Instruction((Refi3Instruction *)instruction_);
		break;

	case bktRouteInstruction:
		dumpRouteInstruction((RouteInstruction *)instruction_);
		break;

	case bktSRAMInstruction:
		dumpSramInstruction((SRAMInstruction *)instruction_);
		break;

	case bktSWBInstruction:
		dumpSwbInstruction((SWBInstruction *)instruction_);
		break;

	case bktWaitInstruction:
		dumpWaitInstruction((WaitInstruction *)instruction_);
		break;
	}
}

// This method dumps the given message as a heading surrounded by the given border line
void VDump::dumpHeading(const string &message_, const string &commentSymbol_, const string &borderLine_)
{
	// 3 (= 4-1) is the amount of used spaces between the message and sideLines subtracted by one space in the border line
	const int availableSpace = borderLine_.size() - message_.size() - 3;
	const int halfAvailableSpace = availableSpace / 2;
	const string appendChar = string(1, borderLine_.front());
	const string appendForOddMessage = availableSpace % 2 == 0 ? appendChar : "";

	string sideLine;
	for (int i = 0; i < halfAvailableSpace; ++i)
		sideLine += appendChar;

	WriteLine("");
	WriteLineToF(commentSymbol_ + " " + borderLine_ + appendForOddMessage, GetCurrentFile(), false);
	WriteLineToF(commentSymbol_ + " " + sideLine + "  " + message_ + "  " + sideLine, GetCurrentFile(), false);
	WriteLineToF(commentSymbol_ + " " + borderLine_ + appendForOddMessage, GetCurrentFile(), false);
}

// This method dumps the common parts of all instructions.
void VDump::dumpInstructionHeader(BIR::Instruction *instruction_, int index_, int clkCounter_)
{
	wtToken("\n");
	wtToken(to_string(index_), 2);
	wtToken(" (");
	wtToken(to_string(clkCounter_), 3);
	wtToken("): ");

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

	wtToken(cmdName + "| ");
	wtToken("StNo: ");
	wtToken(to_string(instruction_->statementNo), 2);
	wtToken(", ");
	//wtToken("UnrolledStatNo:  " + instruction_->unrolledStatementNo + ",\t");
	wtToken("Sch:[");
	wtToken(to_string(instruction_->minScheduledClkCycle), 3);
	wtToken(" , ");
	wtToken(to_string(instruction_->maxScheduledClkCycle), 3);
	wtToken("] | ");
}

// This method dumps DRRA and DiMArch sequencer instructions
void VDump::dumpInstructions(BIR::DRRACluster *drraCluster_)
{
	dumpTitle("DRRA Sequencer Instructions", "#");

	for (int rowIndex = 0; rowIndex < drraCluster_->rowNum(); ++rowIndex)
		for (int colIndex = 0; colIndex < drraCluster_->colNum(); ++colIndex)
		{
			InstructionList *instList = &drraCluster_->getDRRACell(rowIndex, colIndex)->instructions();

			if (instList->size() > 0)
				dumpSequencerHeader("Sequencer instructions in cell", rowIndex, colIndex, "#");

			for (int instIndex = 0, offset = 0; instIndex < instList->size(); ++instIndex)
			{
				dumpDrraInstruction(instList->instruction(instIndex), instIndex, offset);
			}
		}
}

// This method dumps a JUMP instruction
void VDump::dumpJumpInstruction(BIR::JumpInstruction *jumpInst_)
{
	wtToken("Address: " + to_string(jumpInst_->jump_pc) + ", ");
}

// This method dumps a JUMP instruction as an operation in the scheduling file
void VDump::dumpJumpOperation(BIR::JumpInstruction *jumpInst_)
{
	wtToken("JMP:  ");
	wtToken("StatNo:  " + to_string(jumpInst_->statementNo));
}

// This method dumps a Loop Header instruction
void VDump::dumpLoopHeaderInstruction(BIR::LoopHeaderInstruction *loopHeaderInst_)
{
	wtToken("LoopId: " + to_string(loopHeaderInst_->loopId) + ",\t");
	wtToken("StartValue: " + to_string(loopHeaderInst_->startValue) + ",\t");

	const string isStatic = loopHeaderInst_->isStatic ? "Yes" : "No";
	wtToken("IsStatic: " + isStatic + ",\t");

	wtToken("IterationNo: " + to_string(loopHeaderInst_->iterationNumber));
}

// This method dumps a Loop Header instruction as an operation in the scheduling file
void VDump::dumpLoopHeaderOperation(BIR::LoopHeaderInstruction *loopHeaderInst_)
{
	wtToken("LOOP_H:  ");
	wtToken("StatNo:  " + to_string(loopHeaderInst_->statementNo) + ", \t");
}

// This method dumps a Loop Tail instruction
void VDump::dumpLoopTailInstruction(BIR::LoopTailInstruction *loopTailInst_)
{
	wtToken("LoopId: " + to_string(loopTailInst_->loopId) + ",\t");
	wtToken("StepValue: " + to_string(loopTailInst_->stepValue) + ",\t");
	wtToken("GoToPC: " + to_string(loopTailInst_->gotoPC));
}

// This method dumps a Loop Tail instruction as an operation in the scheduling file
void VDump::dumpLoopTailOperation(BIR::LoopTailInstruction *loopTailInst_)
{
	wtToken("LOOP_T:  ");
	wtToken("StatNo:  " + to_string(loopTailInst_->statementNo) + ", \t");
}

// This method creates the make file.
void VDump::dumpMakeFirstRunFile()
{
	WriteLine("@echo off");
	WriteLine("");
	WriteLine("set usr_path=\"%~dp0\\..\\..\\test\"");
	WriteLine("");
	WriteLine("::Different directory paths");
	WriteLine("set testbenches_path=%usr_path%\\testbenches");
	WriteLine("set matlab_util_path=%testbenches_path%\\..\\test_util\\matlab_util");
	WriteLine("set dest_path=%testbenches_path%\\" + InputFileName());
	WriteLine("set matlab_path=%dest_path%\\matlab");
	WriteLine("");
	WriteLine("mkdir %dest_path%");
	WriteLine("mkdir %matlab_path%");
	WriteLine("mkdir %dest_path%\\results");
	WriteLine("mkdir %dest_path%\\reports");
	WriteLine("mkdir %testbenches_path%\\0.final_results.0");
	WriteLine("");
	WriteLine("::Copy the original file to the matlab directory");
	WriteLine("copy ..\\" + InputFilePath() + "\\" + InputFileName() + ".m %matlab_path%");
	WriteLine("::Copy the instruction log file to the destination directory directory");
	WriteLine("copy ..\\log\\" + InputFileName() + "\\" + GetFileName(ftSyncInstructions, false) + " %dest_path%\\reports" + "\\" + GetFileName(ftInstructions, false)); // It also renames the file
	WriteLine("");
	WriteLine("::Move generated files to the destination/matlab directory");
	WriteLine("move " + GetFileName(ftMakeRerun, false) + " %dest_path%");
	WriteLine("move " + GetFileName(ftManas, false) + " %dest_path%");
	WriteLine("move " + GetFileName(ftProfiler, false) + " %dest_path%");
	WriteLine("move " + GetFileName(ftRunCmd, false) + " %dest_path%");
	WriteLine("move " + GetFileName(ftRunGui, false) + " %dest_path%");
	WriteLine("move " + GetFileName(ftTimingModel, false) + " %dest_path%\\reports");
	WriteLine("move " + GetFileName(ftVhdlTestbench, false) + " %dest_path%");
	WriteLine("move " + GetFileName(ftVhdlPackage, false) + " %dest_path%");
	WriteLine("move " + GetFileName(ftWave, false) + " %dest_path%");
	WriteLine("move " + GetFileName(ftMatlab, false) + " %matlab_path%");
	WriteLine("");
	WriteLine("::Copy matlab_util files to the matlab directory");
	WriteLine("copy %matlab_util_path%\\printAssignStatement.m %matlab_path%");
	WriteLine("copy %matlab_util_path%\\printAssignStatementWT.m %matlab_path%");
	WriteLine("copy %matlab_util_path%\\printAssignStatementPrimFunc1.m %matlab_path%");
	WriteLine("copy %matlab_util_path%\\printAssignStatementPrimFunc2.m %matlab_path%");
	WriteLine("copy %matlab_util_path%\\printAssignStatementPrimFunc3.m %matlab_path%");
	WriteLine("copy %matlab_util_path%\\printForStatement.m %matlab_path%");
	WriteLine("copy %matlab_util_path%\\printResult.m %matlab_path%");
	WriteLine("copy %matlab_util_path%\\printStorageTransfer.m %matlab_path%");
	WriteLine("copy %matlab_util_path%\\printVariable.m %matlab_path%");
	WriteLine("copy %matlab_util_path%\\printValue.m %matlab_path%");
	WriteLine("");
	WriteLine("::Running matlab and moving the result file to the result folder");
	WriteLine("cd %matlab_path%");
#ifndef SPECIAL_DUMP
	WriteLine("matlab -nodesktop -nosplash -wait -r instrumented_" + InputFileName() + ";quit");
	WriteLine("move mt_results1_exc_" + InputFileName() + ".txt ..\\results");
	WriteLine("move mt_results2_reg_" + InputFileName() + ".txt ..\\results");
	WriteLine("move mt_results_" + InputFileName() + ".txt ..\\results");
#else
	WriteLine("::matlab -nodesktop -nosplash -wait -r instrumented_" + InputFileName() + ";quit");
	WriteLine("::move mt_results1_exc_" + InputFileName() + ".txt ..\\results");
	WriteLine("::move mt_results2_reg_" + InputFileName() + ".txt ..\\results");
	WriteLine("::move mt_results_" + InputFileName() + ".txt ..\\results");
#endif
	WriteLine("");
	WriteLine("::Running modelsim and performing the simulation");
	WriteLine("cd %dest_path%");
	WriteLine("vsim -c -do run_cmd_" + InputFileName() + ".do");
	WriteLine("");
	WriteLine("::Copying the results into the final_results folder");
	WriteLine("copy results\\mt_results1_exc_" + InputFileName() + ".txt ..\\0.final_results.0\\mt_" + InputFileName() + ".txt");
	WriteLine("copy results\\sv_results1_exc_" + InputFileName() + ".txt ..\\0.final_results.0\\sv_" + InputFileName() + ".txt");
	WriteLine("");
	WriteLine("::Return to the primary directory for future batch files");
	WriteLine("cd %~dp0");
	WriteLine("");
	WriteLine("::Performing a self destruction");
	WriteLine("start /b \"\" cmd /c del \"%~f0\"&exit /b");
	WriteLine("");
}

void VDump::dumpMakeRerunFile()
{
	WriteLine("");
	WriteLine("::Running matlab and moving the result file to the result folder");
	WriteLine("cd matlab");
#ifndef SPECIAL_DUMP
	WriteLine("matlab -nodesktop -nosplash -wait -r instrumented_" + InputFileName() + ";quit");
	WriteLine("move mt_results1_exc_" + InputFileName() + ".txt ..\\results");
	WriteLine("move mt_results2_reg_" + InputFileName() + ".txt ..\\results");
	WriteLine("move mt_results_" + InputFileName() + ".txt ..\\results");
#else
	WriteLine("::matlab -nodesktop -nosplash -wait -r instrumented_" + InputFileName() + ";quit");
	WriteLine("::move mt_results1_exc_" + InputFileName() + ".txt ..\\results");
	WriteLine("::move mt_results2_reg_" + InputFileName() + ".txt ..\\results");
	WriteLine("::move mt_results_" + InputFileName() + ".txt ..\\results");
#endif
	WriteLine("");
	WriteLine("::Running modelsim and performing the simulation");
	WriteLine("cd ..");
	WriteLine("vsim -c -do run_cmd_" + InputFileName() + ".do");
	WriteLine("");
	WriteLine("::Copying the results into the final_results folder");
	WriteLine("copy results\\mt_results1_exc_" + InputFileName() + ".txt ..\\0.final_results.0\\mt_" + InputFileName() + ".txt");
	WriteLine("copy results\\sv_results1_exc_" + InputFileName() + ".txt ..\\0.final_results.0\\sv_" + InputFileName() + ".txt");
}

// This method dumps an instrumented MATLAB code.
/**
 * TODO
 * Temporarily add library path from here, need to use ignore region in matlab code.
 * to deal with it better.
 */
void VDump::dumpMatlabCode(BIR::DRRACluster *drraCluster_, VIR::MainProgram *mainProgram_)
{
	WriteLine("% Instrumented the MATLAB code with some printing commands.");
	WriteLine("addpath('../../../../../library/nn')");
	WriteLine("");
	WriteLine("fileID3 = fopen('mt_results_" + InputFileName() + ".txt','w');");
	WriteLine("");
	WriteLine("");
	dumpMatlabStatementList(&mainProgram_->body(), true);
	WriteLine("fclose(fileID3);");
}

// This method dumps an expression to the MATLAB file.
// The second option default value is false and currently it is only used when
// there is a need to write some extra appendixes to the expression.
// The example is writing an extra ".double" for fixed-point objects in some cases.
string VDump::dumpMatlabExpression(VIR::Expression *expression_, bool writingExtra_)
{
	string str;

	switch (expression_->kind())
	{
	case ktArray:
	{
		Array *array = (Array *)expression_;

		bool hasExtraArrayObject = array->elements().size() == 1 && array->elements()[0]->kind() == ktArray;

		if (!hasExtraArrayObject)
			str += "[";

		str += dumpMatlabExpressionList(&array->elements(), ",");

		if (!hasExtraArrayObject)
			str += "]";
	}
	break;

	case ktBinaryExpression:
	{
		BinaryExpression *binaryExpression = (BinaryExpression *)expression_;

		str = dumpMatlabExpression(binaryExpression->leftOperand(), writingExtra_) + " " +
					binaryExpression->operatorStr() + " " + dumpMatlabExpression(binaryExpression->rightOperand(), writingExtra_);
	}
	break;

	case ktComplex:
		str = "(" + to_string(((Complex *)expression_)->iValue) + "," +
					to_string(((Complex *)expression_)->rValue) + ")";
		break;

	case ktFloatingPoint:
		str = to_string(((FloatingPoint *)expression_)->value);
		break;

	case ktFunctionCall:
	{
		str = ((FunctionCall *)expression_)->_name->name() + "(";
		bool first = true;
		for (auto &p : ((FunctionCall *)expression_)->_parameters)
		{
			if (!first)
			{
				str += ", ";
			}
			first = false;
			str += dumpMatlabExpression(p, false);
		}
		str += ")";
	}
	break;

	case ktIdentifier:
		str = ((Identifier *)expression_)->name() + (writingExtra_ && isFixedPointVariable((Identifier *)expression_) ? ".double" : "");
		break;

	case ktInteger:
		str = to_string(((Integer *)expression_)->value);
		break;

	case ktParenthesizedExpression:
		str = "(" + dumpMatlabExpression(((ParenthesizedExpression *)expression_)->value(), writingExtra_) + ")";
		break;

	case ktPrimitiveFunctionCall:
	{
		PrimitiveFunctionCall *primitiveFunctionCall = (PrimitiveFunctionCall *)expression_;

		str = primitiveFunctionCall->name()->name() + "(" + dumpMatlabExpressionList(&primitiveFunctionCall->parameters(), ",") + ")";
	}
	break;

	case ktRangeExpression:
	{
		RangeExpression *rangeExpression = (RangeExpression *)expression_;

		str = dumpMatlabExpression(rangeExpression->begin(), writingExtra_) + " : " +
					dumpMatlabExpression(rangeExpression->increment(), writingExtra_) + " : " +
					dumpMatlabExpression(rangeExpression->end(), writingExtra_);
	}
	break;

	case ktSliceName:
	{
		SliceName *sliceName = (SliceName *)expression_;

		str = !sliceName->isDummy() ? dumpMatlabExpression(sliceName->prefix(), writingExtra_) + "(" + dumpMatlabExpressionList(&sliceName->suffix(), ",") + ")" : dumpMatlabExpression(sliceName->prefix()->object()->value(), writingExtra_);
	}
	break;

	case ktStringLiteral:
	{
		str = "'" + ((StringLiteral *)expression_)->value + "'";
	}
	break;

	case ktUnaryExpression:
	{
		UnaryExpression *unaryExpression = (UnaryExpression *)expression_;

		str = unaryExpression->operatorStr() + dumpMatlabExpression(unaryExpression->operand(), writingExtra_);
	}
	break;
	}

	return str;
}

// This method dumps an expression list to the MATLAB file.
string VDump::dumpMatlabExpressionList(vector<VIR::Expression *> *expressionList_, string seperator_)
{
	string str;
	const int size = expressionList_->size();

	for (int index = 0; index < size; ++index)
	{
		str += dumpMatlabExpression((*expressionList_)[index]);

		if (index < size - 1)
			str += seperator_ + " ";
	}

	return str;
}

// This method creates a printAssignStatementPrimFunc function for the instrumented MATLAB file.
string VDump::dumpMatlabPrimitiveFunc(VIR::PrimitiveFunctionCall *primitiveFunction_, vector<SliceName *> outputNames_, string isFixedPointStr_)
{
	return "";
}

// This method dumps a statement to the MATLAB file.
void VDump::dumpMatlabStatement(VIR::Statement *statement_)
{
	switch (statement_->kind())
	{
	case ktForStatement:
	{
		ForStatement *forStatement = (ForStatement *)statement_;
		WriteLine("for " + dumpMatlabExpression(forStatement->loopVariable()) + " = " + dumpMatlabExpression(forStatement->loopRange()));
		dumpMatlabStatementList(&forStatement->loopBody());
		WriteLine("end");
	}
	break;

	case ktIfStatement:
	{
		IfStatement *ifStatement = (IfStatement *)statement_;

		WriteLine("if " + dumpMatlabExpression(ifStatement->condition()));

		dumpMatlabStatementList(&ifStatement->thenPart());

		WriteLine("end");
	}
	break;

	case ktIfThenElseStatement:
	{

		IfThenElseStatement *ifThenElseStatement = (IfThenElseStatement *)statement_;

		WriteLine("if " + dumpMatlabExpression(ifThenElseStatement->condition()));

		dumpMatlabStatementList(&ifThenElseStatement->thenPart());

		WriteLine("else");

		dumpMatlabStatementList(&ifThenElseStatement->elsePart());

		WriteLine("end");
	}
	break;

	case ktAssignmentStatement:
	{
		AssignmentStatement *assignStatement = (AssignmentStatement *)statement_;

		// case of preservingMac operation
		string matlabExprStr;

		if (assignStatement->lhs().size() == 0)
		{
			matlabExprStr = dumpMatlabExpression(assignStatement->lhs()[0]) + ";";
		}
		else if (assignStatement->lhs().size() == 1)
		{
			matlabExprStr = dumpMatlabExpression(assignStatement->lhs()[0]) + " = " + dumpMatlabExpression(assignStatement->rhs()) + ";";
		}
		else
		{
			for (int idx = 0; idx < assignStatement->lhs().size(); idx++)
			{
				if (idx == 0)
				{
					matlabExprStr = "[";
				}
				else
				{
					matlabExprStr += ", ";
				}
				matlabExprStr += dumpMatlabExpression(assignStatement->lhs()[idx]);
			}
			matlabExprStr += "]";
			matlabExprStr += " = " + dumpMatlabExpression(assignStatement->rhs()) + ";";
		}

		// Instrumenting the code with print statements
		switch (assignStatement->type())
		{
		case atDeclarative:
		{
			WriteLine(matlabExprStr);
		}
		break;

		case atArithmetic:
		{
			string isFixedPointStr = (assignStatement->lhs().size() > 0 && isFixedPointVariable(static_cast<VIR::Identifier *>((assignStatement->lhs())[0]))) ? "1" : "0";

			vector<SliceName *> outputNames;
			for (auto &sn : assignStatement->lhs())
			{
				outputNames.push_back(static_cast<VIR::SliceName *>(sn));
			}
			SliceName *outputName = outputNames[0];

			switch (assignStatement->rhs()->kind())
			{
			case ktBinaryExpression:
			{
				if (((BinaryExpression *)assignStatement->rhs())->rightOperand()->kind() != ktPrimitiveFunctionCall)
				{

					BinaryExpression *binaryExprssion = (BinaryExpression *)assignStatement->rhs();
					SliceName *leftOpName = (SliceName *)binaryExprssion->leftOperand();

					// TODO: currently assignment statements with 3 operands is not supported,
					//       so only the first 2 operands are printed in the MATLAB printf statements
					SliceName *rightOpName = binaryExprssion->rightOperand()->kind() != ktBinaryExpression ? (SliceName *)binaryExprssion->rightOperand() : (SliceName *)((BinaryExpression *)binaryExprssion->rightOperand())->leftOperand();

					if (leftOpName->kind() == ktSliceName)
					{
						// Overwriting matlabExprStr in case of dummy names
						if (leftOpName->isDummy() || rightOpName->isDummy())
						{
							matlabExprStr = dumpMatlabExpression(static_cast<VIR::SliceName *>(assignStatement->lhs()[0])) + " = ";
							matlabExprStr += leftOpName->isDummy() ? dumpMatlabExpression(leftOpName->prefix()->object()->value()) : dumpMatlabExpression(leftOpName);
							matlabExprStr += " " + binaryExprssion->operatorStr() + " ";
							matlabExprStr += rightOpName->isDummy() && rightOpName->kind() == ktSliceName ? dumpMatlabExpression(rightOpName->prefix()->object()->value()) : dumpMatlabExpression(rightOpName);
							matlabExprStr += ";";
						}
					}
				}
				break;

			case ktPrimitiveFunctionCall:
				break;

			case ktFloatingPoint:
			case ktInteger:
				// Write Nothing
				break;

			default:
				LOG(FATAL) << ("dumpMatlabStatement: Unsupported assignment");
				break;
			}
				string printResultStr = "";
				if (outputNames.size() > 0)
				{
					for (auto outputName : outputNames)
					{
						printResultStr += "printResult(fileID3, ";
						printResultStr += "'";
						printResultStr += dumpMatlabExpression(outputName->prefix()) + "', ";
						printResultStr += dumpMatlabExpression(outputName->suffix()[0]) + ", ";
						printResultStr += dumpMatlabExpression(outputName, true) + ", ";
						printResultStr += isFixedPointStr + ");\n";
					}
					printResultStr += "\n";
				}
				WriteLine(matlabExprStr);
				WriteLine(printResultStr);
				WriteLine("");
			}
		}
		break;

		case atMemoryTransfer:
		case atRegisterTransfer:
		{
			SliceName *lhsSliceName = dynamic_cast<SliceName *>((assignStatement->lhs()[0]));
			SliceName *rhsSliceName = dynamic_cast<SliceName *>((assignStatement->rhs()));

			if (!lhsSliceName || !rhsSliceName)
				LOG(FATAL) << ("dumpMatlabStatement: Improper storage transfer statement!");

			WriteLine(matlabExprStr);

			if (assignStatement->type() == atRegisterTransfer)
			{
				string isFixedPointStr = isFixedPointVariable(static_cast<VIR::Identifier *>((assignStatement->lhs())[0])) ? "1" : "0";
				string printResultStr = "printResult(fileID3, ";

				printResultStr += "'";
				printResultStr += dumpMatlabExpression(lhsSliceName->prefix()) + "', ";
				printResultStr += dumpMatlabExpression(lhsSliceName->suffix()[0]) + ", ";
				printResultStr += dumpMatlabExpression(lhsSliceName, true) + ", ";
				printResultStr += isFixedPointStr + ");";

				WriteLine(printResultStr);
			}

			WriteLine("");
		}
		break;

		default:
		{
			WriteLine(matlabExprStr);
			WriteLine("");
		}
		break;
		}
	}
	break;

	case ktPragmaStatement:
	{
		((PragmaStatement *)statement_)->pragma()->isBlockBegin() ? WriteLine("%! RESOURCE_SHARING_BEGIN") : WriteLine("%! RESOURCE_SHARING_END");
	}
	break;

	default:
		WriteLine("dumpMatlabStatement: NOT SUPPORTED YET!!");
	}
}

// This method dumps a statement list to the MATLAB file.
void VDump::dumpMatlabStatementList(vector<VIR::Statement *> *statementList_, bool isTop_)
{
	if (!isTop_)
	{
		WriteLine("");
		IncTab();
	}

	for (int stIndex = 0; stIndex < statementList_->size(); ++stIndex)
		dumpMatlabStatement((*statementList_)[stIndex]);

	if (!isTop_)
		DecTab();
}

// This method dumps the scheduling file after synchronization
void VDump::dumpOptimizedSchedule(BIR::DRRACluster *drraCluster_)
{
	WriteLine("DRRA Optimized Schedule: ");

	for (int rowIndex = 0; rowIndex < drraCluster_->rowNum(); ++rowIndex)
		for (int colIndex = 0; colIndex < drraCluster_->colNum(); ++colIndex)
		{
			dumpSequencerHeader("Optimized scheduled operations in cell", rowIndex, colIndex, "#");

			map<int, InstructionList> instructionsSequence = drraCluster_->getDRRACell(rowIndex, colIndex)->sequencer()->optimizedInstructionsSequence;

			if (instructionsSequence.size() == 0)
				WriteLine("{ ---< Empty Sequencer >--- }");

			for (auto &element : instructionsSequence)
			{
				WriteLine("\n<" + to_string(element.first) + "> ----------------------");

				InstructionList &instList = element.second;

				for (int instIndex = 0, offset = 0; instIndex < instList.size(); ++instIndex)
				{
					dumpDrraInstruction(instList.instruction(instIndex), instIndex, offset);
				}
			}
		}
}

// This method dumps the clocked process part of the profiler file.
void VDump::dumpProfilerClockedProcess()
{
	WriteLine("always @(negedge rst_n or negedge clk)");
	WriteLine("begin");
	IncTab();
	WriteLine("if (rst_n == 1'b0)");
	WriteLine("begin");
	IncTab();
	WriteLine("clkCounter = 0;");
	WriteLine("$fdisplay(fileid1, startOfExecutionStr);");
	DecTab();
	WriteLine("end");
	WriteLine("else");
	WriteLine("begin");
	IncTab();
	WriteLine("if (clkCounter >= execution_start_cycle)");
	WriteLine("begin");
	IncTab();
	WriteLine("executionCycle = clkCounter - execution_start_cycle;");
	WriteLine("");
	WriteLine("regWriteActivities = \"\";");
	WriteLine("regReadActivities  = \"\";");
	WriteLine("resultActivities   = \"\";");
	WriteLine("");
	WriteLine("for (int row = 0; row < ROWS; row++)");
	IncTab();
	WriteLine("for (int col = 0; col < COLUMNS; col++)");
	WriteLine("begin");
	IncTab();
	WriteLine("if (`currRegData.addrenIn0 || `currRegData.addrenIn1 || `currRegData.addrenOut0 || `currRegData.addrenOut1)");
	WriteLine("begin");
	IncTab();
	WriteLine("if (`currRegData.addrenIn0)");
	IncTab();
	WriteLine("printRegValue(row, col, `currRegData.addrIn0, `currRegData.dataIn0, 0, 1);");
	DecTab();
	WriteLine("");
	WriteLine("if (`currRegData.addrenIn1)");
	IncTab();
	WriteLine("printRegValue(row, col, `currRegData.addrIn1, `currRegData.dataIn1, 1, 1);");
	DecTab();
	WriteLine("");
	WriteLine("if (`currRegData.addrenOut0)");
	IncTab();
	WriteLine("printRegValue(row, col, `currRegData.addrOut0, `currRegData.dataOut0, 0, 0);");
	DecTab();
	WriteLine("");
	WriteLine("if (`currRegData.addrenOut1)");
	IncTab();
	WriteLine("printRegValue(row, col, `currRegData.addrOut1, `currRegData.dataOut1, 1, 0);");
	DecTab();
	DecTab();
	WriteLine("end");
	WriteLine("");
	WriteLine("createSequencerActivity(row, col, executionCycle);");
	WriteLine("");
	DecTab();
	WriteLine("end");
	DecTab();
	WriteLine("if (regWriteActivities != \"\")");
	WriteLine("begin");
	WriteLine("	$fdisplay(fileid1, regWriteActivities);");
	WriteLine("	$fdisplay(fileid1, wtDelimiter);");
	WriteLine("end");
	WriteLine("");
	WriteLine("if (regReadActivities != \"\")");
	WriteLine("	$fdisplay(fileid1, regReadActivities);");
	WriteLine("");
	WriteLine("if (resultActivities != \"\")");
	WriteLine("	$fdisplay(fileid5, resultActivities);");
	WriteLine("");
	WriteLine("printDimarchData(fileid4, executionCycle);");
	DecTab();
	WriteLine("end");

	WriteLine("");
	WriteLine("if (clkCounter == total_execution_cycle-1)");
	WriteLine("begin");
	IncTab();
	WriteLine("$fdisplay(fileid2, startOfExecutionStr);");
	WriteLine("$fdisplay(fileid3, startOfExecutionStr);");
	WriteLine("");
	WriteLine("for (int row = 0; row < ROWS; row++)");
	IncTab();
	WriteLine("for (int col = 0; col < COLUMNS; col++)");
	WriteLine("begin");
	IncTab();
	WriteLine("printRegFileActivity(fileid2, row, col);");
	WriteLine("printSequencerActivity(fileid3, row, col);");
	DecTab();
	WriteLine("end");
	DecTab();
	DecTab();
	WriteLine("end");

	WriteLine("");
	WriteLine("clkCounter++;");
	DecTab();
	WriteLine("end");
	DecTab();
	WriteLine("end");
	WriteLine("");
}

// This method creates a SystemVerilog file which is used for gathering data from the FABRIC, in order to compare it with the MATLAB results.
void VDump::dumpProfilerCode(BIR::DRRACluster *drraCluster_, VIR::MainProgram *mainProgram_)
{
	dumpProfilerHeader();

	dumpProfilerRegFileVariables(drraCluster_);

	dumpProfilerSkipCyclesOfWriteAccess(drraCluster_);

	dumpProfilerFixedPointStatus(drraCluster_);

	dumpProfilerFunction();

	WriteLine("fill_data fd();");
	WriteLine("");

	dumpProfilerClockedProcess();

	DecTab();
	WriteLine("endmodule");
	WriteLine("");
}

// This method creates a a constant multidimensional bit array that shows
// if each elements of the register files are in fixed-point format or not.
void VDump::dumpProfilerFixedPointStatus(BIR::DRRACluster *drraCluster_)
{
	WriteLine("const bit regFileFixedPointStatus[ROWS][COLUMNS][REG_FILE_DEPTH] = '{");
	IncTab();

	string lineStr;
	map<string, int> variableStartAddressMap;
	map<string, int> addressOffsetMap;

	const int rowNum = drraCluster_->rowNum();
	const int colNum = drraCluster_->colNum();

	for (int rowIndex = 0; rowIndex < rowNum; ++rowIndex)
	{
		WriteLine("'{");
		IncTab();

		for (int colIndex = 0; colIndex < colNum; ++colIndex)
		{
			RegFile *regFile = drraCluster_->getDRRACell(rowIndex, colIndex)->regFile();

			const int regDepth = regFile->regDepth();

			lineStr = "";

			for (int regIndex = 0; regIndex < regDepth; ++regIndex)
			{
				ObjectDeclaration *object = regFile->registers(regIndex)->correspondingObject;

				if (regIndex == 0)
					lineStr += "'{ ";

				lineStr += object && object->isFixedPointVariable() ? "1" : "0";

				if (regIndex != regDepth - 1)
					lineStr += ", ";
				else
				{
					lineStr += " }";

					if (colIndex != colNum - 1)
						lineStr += ",";
				}

				if ((regIndex + 1) % 16 == 0 || regIndex == regDepth - 1)
				{
					WriteLine(lineStr);
					lineStr = "   ";
				}
			}
		}

		lineStr = "}";

		if (rowIndex != rowNum - 1)
			lineStr += ",";

		DecTab();
		WriteLine(lineStr);
	}

	DecTab();
	WriteLine("};");

	WriteLine("");
}

// This method dumps a function to the profiler file.
void VDump::dumpProfilerFunction()
{
	WriteLine("function automatic void printRegValue(shortint row, col, address, data, portNo, bit is_input);");
	IncTab();

	WriteLine("");
	WriteLine("const string delimiter       = \"---------------\";");
	WriteLine("const string firstDelimiter  = is_input ? {delimiter, \"\\n\"} : \"\";");
	WriteLine("const string currPosition    = $sformatf(\"\\tin RegFile <%0d,%0d>,\\t@ cycle: %2d (%4d ns)\", row, col, executionCycle, $time);");
	WriteLine("const string addressOffsetKey= $sformatf(\"%s_%0d%0d\", `currRegVariable, row, col);");
	WriteLine("");
	WriteLine("string resultStr;");
	WriteLine("const string strLine = { firstDelimiter, getRegValueStr(row, col, address, data, portNo, executionCycle,");
	WriteLine("		  				 variablesStartAddr[`currRegVariable], addressOffset[addressOffsetKey], is_input,");
	WriteLine("		  				 `currRegVariable, `currFixedPointStatus, skipCyclesOfWriteAccess[row][col].exists(executionCycle), resultStr), currPosition };");
	WriteLine("");
	WriteLine("if (is_input)");
	WriteLine("	regWriteActivities = {regWriteActivities, \"\\n\", strLine};");
	WriteLine("else");
	WriteLine("	regReadActivities = {regReadActivities, \"\\n\", strLine};");
	WriteLine("");
	WriteLine("resultActivities = {resultActivities, resultStr};");
	WriteLine("");

	DecTab();
	WriteLine("endfunction");
	WriteLine("");
}

// This method dumps the header of the profiler file.
void VDump::dumpProfilerHeader()
{
	string entityName = "profiler_" + InputFileName();

	//! Modify the path. because we directly copy util files to the current directory.
	WriteLine("`include \"macros.svh\"");
	WriteLine("import const_package_" + InputFileName() + "::*;");
	WriteLine("import name_mangling::*;");
	WriteLine("import test_util::*;");
	WriteLine("");
	WriteLine("module profiler_" + InputFileName() + "(input logic clk, rst_n);");
	WriteLine("");

	IncTab();

	WriteLine("const string log_file1 = \"results/sv_results1_exc_" + InputFileName() + ".txt\";");
	WriteLine("const string log_file2 = \"results/sv_results2_reg_" + InputFileName() + ".txt\";");
	WriteLine("const string log_file3 = \"results/sv_results3_seq_" + InputFileName() + ".txt\";");
	WriteLine("const string log_file4 = \"results/sv_results4_srm_" + InputFileName() + ".txt\";");
	WriteLine("const string log_file5 = \"results/sv_results_" + InputFileName() + ".txt\";");
	WriteLine("");
	WriteLine("const integer fileid1 = $fopen(log_file1, \"w\");");
	WriteLine("const integer fileid2 = $fopen(log_file2, \"w\");");
	WriteLine("const integer fileid3 = $fopen(log_file3, \"w\");");
	WriteLine("const integer fileid4 = $fopen(log_file4, \"w\");");
	WriteLine("const integer fileid5 = $fopen(log_file5, \"w\");");
	WriteLine("");
	WriteLine("int clkCounter = 0;");
	WriteLine("int executionCycle = 0;");
	WriteLine("string regWriteActivities;");
	WriteLine("string regReadActivities;");
	WriteLine("string resultActivities;");
	WriteLine("const string wtDelimiter = \"-------------------------------------------------------------------------------------------------\";");
	WriteLine("const string startOfExecutionStr = $sformatf(\"\\nStart of execution: %0d ns (cycle = 0)\\n\", (execution_start_cycle + 1) * period);");
	WriteLine("");
}

// This method creates a constant multidimensional string array which
// binds each elements of the register file to the corresponding MATLAB code variable.
void VDump::dumpProfilerRegFileVariables(BIR::DRRACluster *drraCluster_)
{
	// WriteLine("const string regFileVariables[ROWS][COLUMNS][REG_FILE_DEPTH] = '{");
	// IncTab();

	// string lineStr;
	// map<string, int> variableStartAddressMap;
	// map<string, int> addressOffsetMap;

	// const int rowNum = drraCluster_->rowNum();
	// const int colNum = drraCluster_->colNum();

	// for (int rowIndex = 0; rowIndex < rowNum; ++rowIndex)
	// {
	// 	WriteLine("'{");
	// 	IncTab();

	// 	for (int colIndex = 0; colIndex < colNum; ++colIndex)
	// 	{
	// 		RegFile *regFile = drraCluster_->getDRRACell(rowIndex, colIndex)->regFile();

	// 		const int regDepth = regFile->regDepth();

	// 		lineStr = "";

	// 		for (int regIndex = 0; regIndex < regDepth; ++regIndex)
	// 		{
	// 			ObjectDeclaration *object = regFile->registers(regIndex)->correspondingObject;

	// 			if (object && object->pragma()->kind() == ktStoragePragma && ((StoragePragma *)object->pragma())->storageType() == stRegFile)
	// 			{
	// 				StoragePragma *storagePragma = (StoragePragma *)object->pragma();

	// 				unsigned posIndex;

	// 				// TODO: this solution is inefficient. Try to improve it.
	// 				for (posIndex = 0; posIndex < storagePragma->coordinates.size(); ++posIndex)
	// 				{
	// 					Coordinate &coord = storagePragma->coordinates[posIndex];

	// 					if (coord.row == rowIndex && coord.column == colIndex)
	// 						break;
	// 				}

	// 				if (variableStartAddressMap.find(object->name()) == variableStartAddressMap.end())
	// 					variableStartAddressMap[object->name()] = storagePragma->startAddresses[posIndex];

	// 				const string keyStr = object->name() + "_" + to_string(rowIndex) + to_string(colIndex);

	// 				if (addressOffsetMap.find(keyStr) == addressOffsetMap.end())
	// 					addressOffsetMap[keyStr] = posIndex == 0 ? 0 : storagePragma->maxIndexes[posIndex - 1];
	// 			}

	// 			if (regIndex == 0)
	// 				lineStr += "'{ ";

	// 			lineStr += "\"";
	// 			lineStr += object ? object->name() : " ";
	// 			lineStr += "\"";

	// 			if (regIndex != regDepth - 1)
	// 				lineStr += ", ";
	// 			else
	// 			{
	// 				lineStr += " }";

	// 				if (colIndex != colNum - 1)
	// 					lineStr += ",";
	// 			}

	// 			if ((regIndex + 1) % 16 == 0 || regIndex == regDepth - 1)
	// 			{
	// 				WriteLine(lineStr);
	// 				lineStr = "   ";
	// 			}
	// 		}
	// 	}

	// 	lineStr = "}";

	// 	if (rowIndex != rowNum - 1)
	// 		lineStr += ",";

	// 	DecTab();
	// 	WriteLine(lineStr);
	// }

	// DecTab();
	// WriteLine("};");

	// WriteLine("");

	// string initStr;

	// for (auto element : variableStartAddressMap)
	// 	initStr += "\"" + element.first + "\" : " + to_string(element.second) + ", ";

	// // removing the extra comma ", " in the end.
	// initStr.pop_back();
	// initStr.pop_back();

	// WriteLine("const int variablesStartAddr[string] = '{" + initStr + "};");
	// WriteLine("");

	// initStr = "";
	// for (auto element : addressOffsetMap)
	// 	initStr += "\"" + element.first + "\" : " + to_string(element.second) + ", ";

	// // removing the extra comma ", " in the end.
	// initStr.pop_back();
	// initStr.pop_back();

	// WriteLine("const int addressOffset[string] = '{" + initStr + "};");
	// WriteLine("");

	WriteLine(_storage_image->print_for_profiler());
}

// This method finds the cycles that a memory read is active and
// creates an associative array of those cycles and adds it to the profiler.
void VDump::dumpProfilerSkipCyclesOfWriteAccess(BIR::DRRACluster *drraCluster_)
{
	WriteLine("const int skipCyclesOfWriteAccess[ROWS][COLUMNS][int] = '{");
	IncTab();

	string lineStr;

	const int rowNum = drraCluster_->rowNum();
	const int colNum = drraCluster_->colNum();

	for (int rowIndex = 0; rowIndex < rowNum; ++rowIndex)
	{
		WriteLine("'{");
		IncTab();

		for (int colIndex = 0; colIndex < colNum; ++colIndex)
		{
			/* TODO:
			 DEPRECATED:
			 new method based on instruction simulates is used to replace the old way of calculating skip cycles.
			 Old code should be removed.

			OLD CODE BEGIN ***********************************************************
			bool foundSkipCycles = dumpProfilerSkipWtCyclesForACell(drraCluster_->getDRRACell(rowIndex, colIndex));
			OLD CODE END *************************************************************

			*/
			/*
			 * NEW CODE BEGIN*********************************************************
			 */

			bool foundSkipCycles = false;
			vector<Instruction *> &instr_list = drraCluster_->getDRRACell(rowIndex, colIndex)->instr_list;
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
					lineStr += "'{   ";

				lineStr += to_string(cycles) + ":0";

				++counter;

				if (counter % 16 == 0)
				{
					WriteLine(lineStr);
					lineStr = "   ";
				}
			}
			if (counter % 16 != 0)
			{
				WriteLine(lineStr);
			}
			/*
			 * NEW CODE END **********************************************************
			 */

			// If no memory read instruction found in the instruction list, add a default key to the associative array.
			lineStr = !foundSkipCycles ? "'{-1:0}" : " }";

			if (colIndex != colNum - 1)
				lineStr += ",";

			WriteLine(lineStr);
		}

		lineStr = "}";

		if (rowIndex != rowNum - 1)
			lineStr += ",";

		DecTab();
		WriteLine(lineStr);
	}

	DecTab();
	WriteLine("};");
	WriteLine("");
}

// This method dumps a RACCU instruction
void VDump::dumpRaccuInstruction(BIR::RACCUInstruction *raccuInst_)
{
	string isStatic;

	wtToken("Mode: " + getRaccuModeStr(raccuInst_->raccuMode) + ",\t");

	wtToken("Op1: " + to_string(raccuInst_->operand1.value) + getStaticOrDynamic(raccuInst_->operand1.isStatic) + ",\t");
	wtToken("Op2: " + to_string(raccuInst_->operand2.value) + getStaticOrDynamic(raccuInst_->operand2.isStatic) + ",\t");

	wtToken("ResultAddress: " + to_string(raccuInst_->resultAddress));
}

// This method dumps a RACCU instruction as an operation in the scheduling file
void VDump::dumpRaccuOperation(BIR::RACCUInstruction *raccuInst_)
{
	wtToken("RACCU:  ");
	wtToken("StatNo:  " + to_string(raccuInst_->statementNo) + ", \t");
}

// This method dumps a REFI instruction
void VDump::dumpRefiInstruction(BIR::RefiInstruction *refiInst_)
{
	wtToken("'" + refiInst_->variableName + "'", cVariableNameMaxWidth);
	wtToken(",\t");
	wtToken("PortMode: ");
	wtToken((refiInst_->portMode == pmIn ? " 'in'" : "'out'"));
	wtToken(", ");
	wtToken("AddressMode: ");
	wtToken((refiInst_->addressMode == amLinear ? "'Linear'" : "'BitReverse'"));
	wtToken(", ");
	wtToken("PortNo: " + to_string(refiInst_->portNo) + ", ");
	wtToken("IDelay: " + to_string(refiInst_->initialDelay.value) + getStaticOrDynamic(refiInst_->initialDelay.isStatic) + ", ");
	wtToken("MDelay: " + to_string(refiInst_->middleDelay.value) + getStaticOrDynamic(refiInst_->middleDelay.isStatic) + ", ");
	wtToken("RDelay: " + to_string(refiInst_->repetitionDelay.value) + getStaticOrDynamic(refiInst_->repetitionDelay.isStatic) + ", ");
	wtToken("StartAddress: " + to_string(refiInst_->startAddress.value) + getStaticOrDynamic(refiInst_->startAddress.isStatic) + ", ");
	wtToken("StepValue: " + to_string(refiInst_->stepValue.value) + getStaticOrDynamic(refiInst_->stepValue.isStatic) + ", ");
	wtToken("NoOfAddress: " + to_string(refiInst_->numberOfAddress.value) + getStaticOrDynamic(refiInst_->numberOfAddress.isStatic) + ", ");
	wtToken("NoOfRepetition: " + to_string(refiInst_->numberOfRepetition.value) + getStaticOrDynamic(refiInst_->numberOfRepetition.isStatic) + ", ");
	wtToken("RepOffset: " + to_string(refiInst_->repetitionOffset));
}

// This method dumps a Refi1 instruction
void VDump::dumpRefi1Instruction(BIR::Refi1Instruction *refi1Inst_)
{
	wtToken("'" + refi1Inst_->variableName + "'", cVariableNameMaxWidth);
	wtToken(",\t");
	wtToken("IDelay: " + to_string(refi1Inst_->initialDelay.value) + getStaticOrDynamic(refi1Inst_->initialDelay.isStatic) + ", ");
	wtToken("PortNo: " + to_string(refi1Inst_->portNo) + ", ");
	wtToken("StartAddress: " + to_string(refi1Inst_->startAddress.value) + getStaticOrDynamic(refi1Inst_->startAddress.isStatic) + ", ");
	wtToken("AddressNo: " + to_string(refi1Inst_->numberOfAddress.value) + getStaticOrDynamic(refi1Inst_->numberOfAddress.isStatic));
}

// This method dumps a Refi2 instruction
void VDump::dumpRefi2Instruction(BIR::Refi2Instruction *refi2Inst_)
{
	wtToken("'" + refi2Inst_->variableName + "'", cVariableNameMaxWidth);
	wtToken(",\t");
	wtToken("MDelay: " + to_string(refi2Inst_->middleDelay.value) + getStaticOrDynamic(refi2Inst_->middleDelay.isStatic) + ", ");
	wtToken("StepValue: " + to_string(refi2Inst_->stepValue.value) + getStaticOrDynamic(refi2Inst_->stepValue.isStatic) + ", ");
	wtToken("RepetitionNo: " + to_string(refi2Inst_->numberOfRepetition.value) + getStaticOrDynamic(refi2Inst_->numberOfRepetition.isStatic) + ", ");
	wtToken("RepetitionOffset: " + to_string(refi2Inst_->repetitionOffset));
}

void VDump::dumpRefi3Instruction(BIR::Refi3Instruction *refi3Inst_)
{
	wtToken("'" + refi3Inst_->variableName + "'", cVariableNameMaxWidth);
	wtToken(",\t");
	wtToken("RDelay: " + to_string(refi3Inst_->repetitionDelay.value) + getStaticOrDynamic(refi3Inst_->repetitionDelay.isStatic) + ", ");
	wtToken("AddressMode: ");
	wtToken((refi3Inst_->addressMode == amLinear ? "'Linear'" : "'BitReverse'"));
	wtToken(", ");
	wtToken("MiddleDelayExt: " + to_string(refi3Inst_->middleDelayExt) + ", ");
	wtToken("NoOfRepExt: " + to_string(refi3Inst_->numberOfRepetitionExt) + ", ");
	wtToken("RepOffsetExt: " + to_string(refi3Inst_->repetitionOffsetExt));
}

// This method dumps a REFI instruction as an operation in the scheduling file
void VDump::dumpRefiOperation(BIR::RefiInstruction *refiInst_)
{
	wtToken((refiInst_->portMode == pmIn ? "RD :  " : "WR :  "));
	wtToken("StatNo:  " + to_string(refiInst_->statementNo) + ", \t");
	wtToken("AGU:  " + to_string(refiInst_->portNo) + ",\t");
	wtToken("VarName: '" + refiInst_->variableName + "'");
}

// This method dumps a DiMArch ROUTE instruction
void VDump::dumpRouteInstruction(BIR::RouteInstruction *routeInst_)
{
	wtToken("'" + routeInst_->variableName + "'", cVariableNameMaxWidth);
	wtToken(",\t");
	wtToken("Src: <" + to_string(routeInst_->sourceCoordinate.row) + "," + to_string(routeInst_->sourceCoordinate.column) + ">, ");
	wtToken("Dst: <" + to_string(routeInst_->destCoordinate.row) + "," + to_string(routeInst_->destCoordinate.column) + ">, ");
	wtToken("DRRA Select: " + to_string(routeInst_->drraSelect) + ", ");
	wtToken("IsFromSrc: ");
	wtToken((routeInst_->isFromSource ? "Y" : "N"));
}

// This method writes the Run file for simulating in the ModelSim/QuestaSim command line mode.
void VDump::dumpRunCmdFile()
{
	WriteLine("vcom -mixedsvvh const_package_" + InputFileName() + ".vhd");
	WriteLine("vlog profiler_" + InputFileName() + ".sv");
	WriteLine("vcom testbench_" + InputFileName() + ".vhd");
	WriteLine("");
	WriteLine("vsim testbench_" + InputFileName());
	WriteLine("set StdArithNoWarnings 1");
	WriteLine("set NumericStdNoWarnings 1");
	WriteLine("run 0 ns;");
	WriteLine("set StdArithNoWarnings 0");
	WriteLine("set NumericStdNoWarnings 0");
	WriteLine("run " + to_string(_totalExecutionCycle * cDefaultClkPeriod) + "ns");
	WriteLine("quit");
}

// This method writes the Run file for simulating in the ModelSim/QuestaSim GUI.
void VDump::dumpRunGuiFile()
{
	WriteLine("vcom -mixedsvvh const_package_" + InputFileName() + ".vhd");
	WriteLine("vlog profiler_" + InputFileName() + ".sv");
	WriteLine("vcom testbench_" + InputFileName() + ".vhd");
	WriteLine("");
	WriteLine("# -------------------------------------------------");
	WriteLine("# Call vsim to invoke simulator");
	WriteLine("");
	WriteLine("vsim -voptargs=+acc testbench_" + InputFileName());
	WriteLine("");
	WriteLine("#vsim -coverage -debugDB testbench_" + InputFileName() + "_opt");
	WriteLine("");
	WriteLine("# -------------------------------------------------");
	WriteLine("# Set the window types and source the wave do file");
	WriteLine("");
	WriteLine("# view wave");
	WriteLine("# view structure");
	WriteLine("# view signals");
	WriteLine("do wave_" + InputFileName() + ".do");
	WriteLine("");
	WriteLine("#log -r /*");
	WriteLine("");
	WriteLine("# -------------------------------------------------");
	WriteLine("# Run simulation for this time");
	WriteLine("");
	WriteLine("set StdArithNoWarnings 1");
	WriteLine("set NumericStdNoWarnings 1");
	WriteLine("run 0 ns;");
	WriteLine("set StdArithNoWarnings 0");
	WriteLine("set NumericStdNoWarnings 0");
	WriteLine("run " + to_string(_totalExecutionCycle * cDefaultClkPeriod) + "ns");
	WriteLine("");
	WriteLine("write transcript results/sim_result_" + InputFileName() + ".txt");
	WriteLine("");
	WriteLine("# -------------------------------------------------");
	WriteLine("# End");
	WriteLine("# quit -sim");
}

// This method dumps instructions as scheduled operations for each sequencer of the DRRA/DiMArch cells
void VDump::dumpScheduledOperations(BIR::DRRACluster *drraCluster_)
{
	dumpTitle("DRRA Scheduled Operations", "#");

	for (int rowIndex = 0; rowIndex < drraCluster_->rowNum(); ++rowIndex)
		for (int colIndex = 0; colIndex < drraCluster_->colNum(); ++colIndex)
		{
			Sequencer *sequencer = drraCluster_->getDRRACell(rowIndex, colIndex)->sequencer();

			if (sequencer->instructionsSequence.size() > 0)
				dumpSequencerHeader("Scheduled operations in cell", rowIndex, colIndex, "#");

			dumpScheduledOperationsInCell(sequencer);
		}
}

// This method dumps instructions as scheduled operations for the sequencer of one DRRA/DiMArch cell
void VDump::dumpScheduledOperationsInCell(BIR::Sequencer *sequencer_)
{
	map<int, InstructionList> instructionsSequence = sequencer_->instructionsSequence;

	for (auto &element : instructionsSequence)
	{
		WriteLine("\n<" + to_string(element.first) + "> ----------------------");

		InstructionList &instList = element.second;

		for (int instIndex = 0; instIndex < instList.size(); ++instIndex)
		{
			Instruction *currInst = instList.instruction(instIndex);

			WriteLine("{ ");

			switch (currInst->kind())
			{
			case bktBranchInstruction:
				dumpBranchOperation((BranchInstruction *)currInst);
				break;

			case bktDPUInstruction:
				dumpDpuOperation((DPUInstruction *)currInst);
				break;

			case bktJumpInstruction:
				dumpJumpOperation((JumpInstruction *)currInst);
				break;

			case bktLoopHeaderInstruction:
				dumpLoopHeaderOperation((LoopHeaderInstruction *)currInst);
				break;

			case bktLoopTailInstruction:
				dumpLoopTailOperation((LoopTailInstruction *)currInst);
				break;

			case bktRACCUInstruction:
				dumpRaccuOperation((RACCUInstruction *)currInst);
				break;

			case bktRefiInstruction:
				dumpRefiOperation((RefiInstruction *)currInst);
				break;

			case bktSRAMInstruction:
				dumpSramOperation((SRAMInstruction *)currInst);
				break;

			case bktRouteInstruction:
			case bktSWBInstruction:
				break;
			}

			wtToken(" }");
		}
	}
}

// This method writes a header corresponding to the sequencer with the given coordinates to the given file.
void VDump::dumpSequencerHeader(const string &message_, const int &row_, const int &col_, const string &commentSymbol_)
{
	dumpHeading(message_ + " <" + to_string(row_) + ", " + to_string(col_) + ">",
							commentSymbol_, "----------------------------------------------------------------------------------------------------");
}

// This method dumps a DiMArch SRAM instruction
void VDump::dumpSramInstruction(BIR::SRAMInstruction *sramInst_)
{
	wtToken("'" + sramInst_->variableName + "'", cVariableNameMaxWidth);
	wtToken(",\t");
	wtToken("IDelay: " + to_string(sramInst_->initialDelay.value) + getStaticOrDynamic(sramInst_->initialDelay.isStatic) + ", ");
	wtToken("InitAddress: " + to_string(sramInst_->initialAddress.value) + getStaticOrDynamic(sramInst_->initialAddress.isStatic) + ", ");
	wtToken("Loop1(");
	wtToken("iter: " + to_string(sramInst_->loop1Iterations.value) + getStaticOrDynamic(sramInst_->loop1Iterations.isStatic) + ", ");
	wtToken("inc: " + to_string(sramInst_->loop1Increment) + ", ");
	wtToken("delay: " + to_string(sramInst_->loop1Delay.value) + getStaticOrDynamic(sramInst_->loop1Delay.isStatic) + "), ");
	wtToken("Loop2(");
	wtToken("iter: " + to_string(sramInst_->loop2Iterations.value) + getStaticOrDynamic(sramInst_->loop2Iterations.isStatic) + ", ");
	wtToken("inc: " + to_string(sramInst_->loop2Increment) + ", ");
	wtToken("delay: " + to_string(sramInst_->loop2Delay.value) + getStaticOrDynamic(sramInst_->loop2Delay.isStatic) + ")");
}

// This method dumps a DiMArch SRAM instruction as an operation in the scheduling file
void VDump::dumpSramOperation(BIR::SRAMInstruction *sramInst_)
{
	wtToken((sramInst_->isRead ? "SRD:  " : "SWR:  "));
	wtToken("StatNo:  " + to_string(sramInst_->statementNo) + ", \t");
}

// This method dumps the instructions after synchronization
void VDump::dumpSynchronizedInstructions(BIR::DRRACluster *drraCluster_)
{
	dumpTitle("DRRA Synchronized Instructions", "#");

	for (int rowIndex = 0; rowIndex < drraCluster_->rowNum(); ++rowIndex)
	{
		for (int colIndex = 0; colIndex < drraCluster_->colNum(); ++colIndex)
		{
			vector<Instruction *> &instr_list = drraCluster_->getDRRACell(rowIndex, colIndex)->instr_list;
			if (instr_list.size() > 0)
			{
				dumpSequencerHeader("Synchronized instructions in cell", rowIndex, colIndex, "#");
			}
			for (int instIndex = 0, offset = 0; instIndex < instr_list.size(); ++instIndex)
			{
				dumpDrraInstruction(instr_list[instIndex], instIndex, offset);
			}
		}
	}
}

// This method dumps a SWB instruction
void VDump::dumpSwbInstruction(BIR::SWBInstruction *swbInst_)
{
	wtToken("'" + swbInst_->variableName + "'", cVariableNameMaxWidth);
	wtToken(",\t");

	wtToken("S:");
	dumpComponentInfo(&swbInst_->source);

	wtToken(", D:");
	dumpComponentInfo(&swbInst_->destination);
}

// This method dumps the timing model of the program.
void VDump::dumpTimingModel(BIR::DRRACluster *drraCluster_, VIR::MainProgram *mainProgram_)
{
	VariableScheduledValues variableScheduledValues;

	calculateTimingModel(drraCluster_, variableScheduledValues);

	vector<VIR::Statement *> &stList = mainProgram_->body();

	for (int stIndex = 0; stIndex < stList.size(); ++stIndex)
	{
		AssignmentStatement *assignStatement = dynamic_cast<AssignmentStatement *>(stList[stIndex]);

		if (assignStatement && assignStatement->type() == atDeclarative)
		{
			StoragePragma *storagePragma = dynamic_cast<StoragePragma *>(assignStatement->pragma());

			if (!storagePragma)
				LOG(FATAL) << ("dumpTimingModel: Wrong pragma!");

			if (storagePragma->storageType() == stRegFile)
			{
				string variableName = static_cast<VIR::Identifier *>(assignStatement->lhs()[0])->name();
				string variableType;

				switch (storagePragma->variableType())
				{
				case vtInput:
					variableType = "INPUT";
					break;
				case vtOutput:
					variableType = "OUTPUT";
					break;
				case vtIntermediate:
				default:
					variableType = "INTERM";
					break;
				}

				ScheduledValues &scheduledValues = variableScheduledValues[variableName];

				WriteLine("");
				WriteLine(variableName + ": " + variableType + ", ");
				WriteLine("schedule: (min: " + to_string(scheduledValues.minCycle) + ", max: " + to_string(scheduledValues.maxCycle) + ")");
			}
		}
	}
}

// This method writes a title to the current file.
void VDump::dumpTitle(const string &title_, const string &commentSymbol_)
{
	dumpHeading(title_, commentSymbol_, "****************************************");
}

// This method dumps a wait instruction
void VDump::dumpWaitInstruction(BIR::WaitInstruction *waitInst_)
{
	wtToken("WaitCycles:  " + to_string(waitInst_->numberOfCycles.value) + getStaticOrDynamic(waitInst_->numberOfCycles.isStatic));
}

void VDump::dumpWaveDimarchSection(BIR::DiMArch *dimarch_)
{
	const string groupStr = "add wave -group ";
	const string labelStr = "\t-label ";

	WriteLine("");
	WriteLine("add wave -divider \"SRAM Values\"");

	for (int colIndex = 0; colIndex < dimarch_->colNum(); ++colIndex)
	{
		string colStr = to_string(colIndex);

		WriteLine(groupStr + "\"Column <" + colStr + ">\" \\");

		for (int rowIndex = 0; rowIndex < dimarch_->rowNum(); ++rowIndex)
		{
			string rowStr = to_string(rowIndex);
			string stileMemSignature = "/DUT/DiMArch_COLS(" + colStr + ")/DiMArch_ROWS(" + rowStr + ")/u_STILE/u_sram/rw/mem";
			string backslash = (rowIndex != dimarch_->rowNum() - 1) ? "	\\" : "";

			WriteLine(labelStr + "\"STILE <" + rowStr + ">\"	" + stileMemSignature + backslash);
		}
	}
}

void VDump::dumpWaveDrraSection(BIR::DRRACluster *drraCluster_)
{
	const string groupStr = "add wave -group ";
	const string labelStr = "\t-label ";

	WriteLine("");
	WriteLine("add wave -divider \"Compare Register Values\"");

	for (int rowIndex = 0; rowIndex < drraCluster_->rowNum(); ++rowIndex)
		for (int colIndex = 0; colIndex < drraCluster_->colNum(); ++colIndex)
			if (drraCluster_->getDRRACell(rowIndex, colIndex)->instr_list.size() > 0 || drraCluster_->getDRRACell(rowIndex, colIndex)->regFile()->hasInitialLoad())
			{
				string cellLabel = " <" + to_string(rowIndex) + ", " + to_string(colIndex) + ">";
				string regSignagure = "/DUT/MTRF_COLS(" + to_string(colIndex) + ")/MTRF_ROWS(" + to_string(rowIndex) + ")/SILEGO_cell/MTRF_cell/reg_top/";

				WriteLine("");
				WriteLine(groupStr + "\"RegFile" + cellLabel + "\"	-radix decimal \\");
				WriteLine(labelStr + "data_out_reg_0 	" + regSignagure + "RegisterFile/data_out_reg_0_left		\\");
				WriteLine(labelStr + "data_out_reg_1 	" + regSignagure + "RegisterFile/data_out_reg_1_left		\\");
				WriteLine(labelStr + "data_in_0 		" + regSignagure + "RegisterFile/data_in_0					\\");
				WriteLine(labelStr + "data_in_1 		" + regSignagure + "RegisterFile/data_in_1					\\");
				WriteLine(labelStr + "data_in_2 		" + regSignagure + "RegisterFile/data_in_2					\\");
				WriteLine(labelStr + "reg_out			" + regSignagure + "RegisterFile/reg_out");
			}

	WriteLine("");
	WriteLine("add wave -divider");

	for (int rowIndex = 0; rowIndex < drraCluster_->rowNum(); ++rowIndex)
		for (int colIndex = 0; colIndex < drraCluster_->colNum(); ++colIndex)
			if (drraCluster_->getDRRACell(rowIndex, colIndex)->instr_list.size() > 0)
			{
				string cellLabel = " <" + to_string(rowIndex) + ", " + to_string(colIndex) + ">";
				string cellSignature = "/DUT/MTRF_COLS(" + to_string(colIndex) + ")/MTRF_ROWS(" + to_string(rowIndex) + ")/SILEGO_cell/MTRF_cell/";
				string seqSignature = cellSignature + "seq_gen/";
				string dpuSignature = cellSignature + "dpu_gen/";
				string regSignagure = cellSignature + "reg_top/";

				WriteLine("");
				WriteLine("add wave -divider \"Cell" + cellLabel + "\"");
				WriteLine("");
				WriteLine(groupStr + "\"Sequencer" + cellLabel + "\"	-radix decimal \\");
				WriteLine(labelStr + "pc 				 " + seqSignature + "pc				\\");
				WriteLine("	-radix decimal \\");
				WriteLine(labelStr + "seq_address_match " + seqSignature + "seq_address_match	\\");
				WriteLine(labelStr + "instr_reg		 " + seqSignature + "instr_reg			\\");
				WriteLine(labelStr + "instr 			 " + seqSignature + "instr");
				WriteLine("");
				WriteLine(groupStr + "\"DPU" + cellLabel + "\"	-radix decimal \\");
				WriteLine(labelStr + "dpu_in_0  " + dpuSignature + "dpu_in_0			\\");
				WriteLine(labelStr + "dpu_in_1  " + dpuSignature + "dpu_in_1			\\");
				WriteLine(labelStr + "dpu_in_2  " + dpuSignature + "dpu_in_2			\\");
				WriteLine(labelStr + "dpu_in_3  " + dpuSignature + "dpu_in_3			\\");
				WriteLine(labelStr + "dpu_mode  " + dpuSignature + "dpu_mode_cfg		\\");
				WriteLine(labelStr + "dpu_out_0 " + dpuSignature + "dpu_out_0_left	\\");
				WriteLine(labelStr + "dpu_out_1 " + dpuSignature + "dpu_out_1_left");
				WriteLine("");
				WriteLine(groupStr + "\"Register File" + cellLabel + "\"	-radix decimal \\");
				WriteLine(labelStr + "instr_start 		" + regSignagure + "instr_start							\\");
				WriteLine(labelStr + "data_out_reg_0 	" + regSignagure + "RegisterFile/data_out_reg_0_left		\\");
				WriteLine(labelStr + "data_out_reg_1 	" + regSignagure + "RegisterFile/data_out_reg_1_left		\\");
				WriteLine(labelStr + "data_in_0 		" + regSignagure + "RegisterFile/data_in_0					\\");
				WriteLine(labelStr + "data_in_1 		" + regSignagure + "RegisterFile/data_in_1					\\");
				WriteLine(labelStr + "data_in_2 		" + regSignagure + "RegisterFile/data_in_2					\\");
				WriteLine(labelStr + "reg_out			" + regSignagure + "RegisterFile/reg_out");
				WriteLine(groupStr + "\"AGU Wr0" + cellLabel + "\"	-radix decimal \\");
				WriteLine(labelStr + "instr_start " + regSignagure + "AGU_Wr_0_instantiate/instr_start	\\");
				WriteLine(labelStr + "addr_out    " + regSignagure + "AGU_Wr_0_instantiate/addr_out		\\");
				WriteLine(labelStr + "addr_en     " + regSignagure + "AGU_Wr_0_instantiate/addr_en");
				WriteLine(groupStr + "\"AGU Wr1" + cellLabel + "\"	-radix decimal \\");
				WriteLine(labelStr + "instr_start " + regSignagure + "AGU_Wr_1_instantiate/instr_start	\\");
				WriteLine(labelStr + "addr_out    " + regSignagure + "AGU_Wr_1_instantiate/addr_out		\\");
				WriteLine(labelStr + "addr_en     " + regSignagure + "AGU_Wr_1_instantiate/addr_en");
				WriteLine(groupStr + "\"AGU Rd0" + cellLabel + "\"	-radix decimal \\");
				WriteLine(labelStr + "instr_start " + regSignagure + "AGU_Rd_0_instantiate/instr_start	\\");
				WriteLine(labelStr + "addr_out    " + regSignagure + "AGU_Rd_0_instantiate/addr_out		\\");
				WriteLine(labelStr + "addr_en     " + regSignagure + "AGU_Rd_0_instantiate/addr_en");
				WriteLine(groupStr + "\"AGU Rd1" + cellLabel + "\"	-radix decimal \\");
				WriteLine(labelStr + "instr_start " + regSignagure + "AGU_Rd_1_instantiate/instr_start	\\");
				WriteLine(labelStr + "addr_out    " + regSignagure + "AGU_Rd_1_instantiate/addr_out		\\");
				WriteLine(labelStr + "addr_en     " + regSignagure + "AGU_Rd_1_instantiate/addr_en");
			}
}

// This method writes the wave file for the VHDL testbench
void VDump::dumpWaveFile(BIR::DRRACluster *drraCluster_)
{
	WriteLine("onerror {resume}");
	WriteLine("quietly WaveActivateNextPane {} 0");
	WriteLine("");
	WriteLine("add wave -divider");
	WriteLine("add wave -label clk   clk");
	WriteLine("add wave -label rst_n rst_n");
	WriteLine("add wave -divider \"Counters\"");
	WriteLine("add wave -noupdate -radix decimal /StimuliSequencer/*");

	dumpWaveDrraSection(drraCluster_);

	if (drraCluster_->diMArch()->totalUsedMemWords() > 0)
		dumpWaveDimarchSection(drraCluster_->diMArch());

	WriteLine("");
	WriteLine("TreeUpdate [SetDefaultTree]");
	WriteLine("WaveRestoreCursors {{Cursor 1} {0 ns} 0}");
	WriteLine("quietly wave cursor active 0");
	WriteLine("configure wave -namecolwidth 300");
	WriteLine("configure wave -valuecolwidth 100");
	WriteLine("configure wave -justifyvalue left");
	WriteLine("configure wave -signalnamewidth 0");
	WriteLine("configure wave -snapdistance 10");
	WriteLine("configure wave -datasetprefix 0");
	WriteLine("configure wave -rowmargin 4");
	WriteLine("configure wave -childrowmargin 2");
	WriteLine("configure wave -gridoffset 0");
	WriteLine("configure wave -gridperiod 1");
	WriteLine("configure wave -griddelta 40");
	WriteLine("configure wave -timeline 0");
	WriteLine("configure wave -timelineunits ns");
	WriteLine("update");
	WriteLine("WaveRestoreZoom {0 ns} {170 ns}");
	WriteLine("");
}

// This method gets the actual row index of the given index in the DiMArch
// It has been determined according to the DiMArch structure.
int VDump::getActualDimarchRow(int virtualRowIndex_) const
{
	// TODO: It may be revised in future
	return virtualRowIndex_ == 0 ? 1 : 2;
}

// This method gets a branch mode and returns the corresponding string
string VDump::getBranchModeStr(BIR::BIREnumerations::BranchMode branchMode_)
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
string VDump::getComponentTypeStr(BIR::BIREnumerations::ComponentType compType_)
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

// This method gets a dpu mode and returns the corresponding string
string VDump::getDpuModeStr(BIR::BIREnumerations::DPUMode dpuMode_)
{
	string modeStr;

	switch (dpuMode_)
	{
	case dmAbsSubAcc:
		modeStr = "'AbsSubAcc'";
		break;
	case dmAdd:
		modeStr = "'ADD'";
		break;
	case dmAddWithConstant:
		modeStr = "'AddWithConst'";
		break;
	case dmComparison:
		modeStr = "'COMP'";
		break;
	case dmFFT1:
		modeStr = "'FFT1'";
		break;
	case dmFFT2:
		modeStr = "'FFT2'";
		break;
	case dmLoadConstant:
		modeStr = "'LoadConst'";
		break;
	case dmMac:
		modeStr = "'MAC'";
		break;
	case dmMax:
		modeStr = "'MAX'";
		break;
	case dmMin:
		modeStr = "'MIN'";
		break;
	case dmMult:
		modeStr = "'MULT'";
		break;
	case dmPreservingMac:
		modeStr = "'PreservingMac'";
		break;
	case dmSub:
		modeStr = "'SUB'";
		break;
	case dmSubAcc:
		modeStr = "'SubAcc'";
		break;
	case dmSymMac:
		modeStr = "'SymMAC'";
		break;

	case dmSymFirWithIMac:
		modeStr = "'SymFirWithIMac'";
		break;
	case dmSymFirWithEMac:
		modeStr = "'SymFirWithEMac'";
		break;
	case dmASymFirWithIMac:
		modeStr = "'ASymFirWithIMac'";
		break;
	case dmASymFirWithEMac:
		modeStr = "'ASymFirWithEMac'";
		break;
	case dmTwoAdders:
		modeStr = "'TwoAdders'";
		break;
	case dmTwoSubtractors:
		modeStr = "'TwoSubtractors'";
		break;
	case dmFourInputAdder:
		modeStr = "'FourInputAdder'";
		break;
	case dmDiffOfSum:
		modeStr = "'DiffOfSum'";
		break;
	case dmSumOfDiff:
		modeStr = "'SumOfDiff'";
		break;
	case dmLFSR:
		modeStr = "'LFSR'";
		break;
	case dmShiftLeft:
		modeStr = "'ShiftLeft'";
		break;
	case dmShiftRight:
		modeStr = "'ShiftRight'";
		break;
	case dmAndOr:
		modeStr = "'AndOr'";
		break;
	case dmAndOrWithStatusBit:
		modeStr = "'AndOrWithStatusBit'";
		break;
	case dmReset:
		modeStr = "'Reset'";
		break;
	}

	return modeStr;
}

// This method gets a loop and it calculates an execution cycle offset according to
// the amount of inner loop execution cycles inside this loop.
int VDump::getInnerLoopExecution(BIR::LoopHeaderInstruction *currLoop_, int iterNo_, bool isTopCall_)
{
	const int currIterNo = !isTopCall_ ? currLoop_->iterationNumber : 1;

	int execOffset = currLoop_->loopExecution;

	if (!isTopCall_)
		execOffset *= (currIterNo - 1) * iterNo_;

	if (currLoop_->innerLoops.size() > 0)
		for (auto &innerLoop : currLoop_->innerLoops)
			execOffset += getInnerLoopExecution(innerLoop, iterNo_ * currIterNo, false);

	return execOffset;
}

// This method gets a RACCU mode and returns the corresponding string
string VDump::getRaccuModeStr(BIR::BIREnumerations::RACCUMode raccuMode_)
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

// This method gets the index of a DRRA row or column along with the row/column length
// and returns a one-hot string signature for it.
// This method is used for VHDL code generation
string VDump::getRowOrColSignature(int index_, int length_)
{
	string signature(length_, '0');

	signature.replace(length_ - index_ - 1, 1, 1, '1');

	signature = "\"" + signature + "\"";

	return signature;
}

// This method gets "(D)" or "(S)" strings corresponding to the given input value.
string VDump::getStaticOrDynamic(bool isStatic_) const
{
	return isStatic_ ? "(S)" : "(D)";
}

// This method converts the given integer value to an HEX string with the given size
string VDump::intToHexStr(int value_, int strSize_)
{
	std::stringstream stream;

	stream << std::setfill('0') << std::setw(strSize_) << std::hex << value_;
	;

	const string valStr = stream.str();

	return valStr.substr(valStr.size() - strSize_, strSize_);
}

// This method determines if the given identifier is a fixed-point variable.
bool VDump::isFixedPointVariable(VIR::Identifier *identifier_)
{
	return identifier_->isFixedPointVariable;
}

// This method converts the given float value to a fixed-point Q15 format HEX string with the given size
string VDump::realToHexStr(float value_, int strSize_)
{
	// Q15 format
	short intValue = short(value_ * pow(2, 15));

	// Correction when value_ == 1.0
	if (value_ == 1.0)
		intValue = 32767;

	return intToHexStr(intValue, strSize_);
}
