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

#ifndef _VDump_H_
#define _VDump_H_

#include "util/Common.hpp"
#include <bitset>
#include <fstream>
#include <map>
#include <vector>
#include "BIR/BIRIncludeAll.hpp"
#include "VIR/VIRIncludeAll.hpp"
#include "VManager/FileType.hpp"
#include "util/Common.hpp"
#include "filegen/FileGenerator.hpp"
#include "VManager/MemoryManager.hpp"
#include "codegen/StorageImage.hpp"

//#define SPECIAL_DUMP

using std::map;
using std::ofstream;
using std::string;
using std::vector;

class VDump
{
public:
	VDump();

	void drraCodeGeneration(BIR::DRRACluster *drraCluster_, VIR::MainProgram *mainProgram_);
	void dumpDRRACluster(BIR::DRRACluster *drraCluster_);
	void dumpMakeGraph();
	void dumpProgram(VIR::MainProgram *mainProgram, const bool isElabDump_);
	void dump_layout(vector<vector<int>> layout);

private:
	void dumpAddressPragma(VIR::AddressPragma *addressPragma_);
	void dumpArray(VIR::Array *array_);
	void dumpAssignmentPragma(VIR::AssignmentPragma *assignmentPragma_);
	void dumpAssignmentStatement(VIR::AssignmentStatement *assignmentStatement_);
	void dumpBinaryExpression(VIR::BinaryExpression *binaryExpression_);
	void dumpComplex(VIR::Complex *complex_);
	void dumpDebugStatement(VIR::DebugStatement *debugStatement_);
	void dumpExpression(VIR::Expression *expression_);
	void dumpExpressionList(vector<VIR::Expression *> *expressionList_, const string &topNodeSignature_);
	void dumpFloatingPoint(VIR::FloatingPoint *floatingPoint_);
	void dumpForStatement(VIR::ForStatement *forStatement_);
	void dumpFunctionCall(VIR::FunctionCall *functionCall_);
	void dumpIdentifier(VIR::Identifier *identifier_);
	void dumpIfStatement(VIR::IfStatement *ifStatement_);
	void dumpIfThenElseStatement(VIR::IfThenElseStatement *ifThenElseStatement_);
	void dumpInteger(VIR::Integer *integer_);
	void dumpObjectDeclaration(VIR::ObjectDeclaration *objectDeclaration_);
	void dumpMainProgram(VIR::MainProgram *mainProgram_);
	void dumpParenthesizedExpression(VIR::ParenthesizedExpression *parenthesizedExpression_);
	void dumpPragmaStatement(VIR::PragmaStatement *pragmaStatement_);
	void dumpPrimitiveFunctionCall(VIR::PrimitiveFunctionCall *primitiveFunctionCall_);
	void dumpRaccuPragma(VIR::RaccuPragma *raccuPragma_);
	void dumpRangeExpression(VIR::RangeExpression *rangeExpression_);
	void dumpReferencePragma(VIR::ReferencePragma *referencePragma_);
	void dumpStoragePragma(VIR::StoragePragma *storagePragma_);
	void dumpSliceName(VIR::SliceName *sliceName_);
	void dumpStatement(VIR::Statement *statement_);
	void dumpStatementList(vector<VIR::Statement *> *statementList_, const string &topNodeSignature_);
	void dumpStringLiteral(VIR::StringLiteral *stringLiteral_);
	void dumpTempVarPragma(VIR::TempVarPragma *tempVarPragma_);
	void dumpUnaryExpression(VIR::UnaryExpression *unaryExpression_);

	void closeSubgraph();
	void createGraphEdge(const string &leftNode_, const string &rightNode_, const string &label_ = "", const string &style_ = "");
	void createGraphListNode(const string &nodeSignature_, const string &label_);
	void createGraphNode(const VIR::VIRBase *node_);
	void decTabWithBrace();
	string getListNodeSignature(const string &topNodeSignature_);
	string getNodeSignature(const VIR::VIRBase *node_);
	void incTabWithBrace();
	bool isDumpEnabled(const string &key);
	void openSubgraph();
	void setGraphNodeStyle(const string &node_, const string &style_);
	void setIncludeInNormalMode();
	string uniqueStringNode(const string &str);

	//! Yu Yang 2017-08-01
	// Remove inline decorator to avoid compile error in gcc
	// CHANGE BEGIN
	void wtDumpDsLine(const string &line_);
	void wtToken(const string &token_, const int width_ = -1);
	// CHANGE END

	// -----------------------------------------------------
	// Back-end Dump Methods

	struct ScheduledValues
	{
		int minCycle;
		int maxCycle;
		ScheduledValues() : minCycle(INT_MAX), maxCycle(0) {}
	};
	typedef map<string, ScheduledValues> VariableScheduledValues;
	typedef vector<BIR::LoopHeaderInstruction *> LoopList;

	void calculateTimingModel(BIR::DRRACluster *drraCluster_, VariableScheduledValues &variableScheduledValues_);
	void calculateTotalExecutionCycle(BIR::DRRACluster *drraCluster_, int maxScheduledCycle_);
	void createLoopList(BIR::LoopHeaderInstruction *currLoop_, LoopList &loopList_);
	void dumpComponentInfo(BIR::ComponentInfo *componentInfo_);
	void dumpBranchInstruction(BIR::BranchInstruction *branchInst_);
	void dumpBranchOperation(BIR::BranchInstruction *branchInst_);
	void dumpDpuInstruction(BIR::DPUInstruction *dpuInst_);
	void dumpDpuOperation(BIR::DPUInstruction *dpuInst_);
	void dumpDrraInstruction(BIR::Instruction *instruction_, int index_, int &offset_);
	void dumpHeading(const string &message_, const string &commentSymbol_, const string &borderLine_);
	void dumpInstructionHeader(BIR::Instruction *instruction_, int index_, int clkCounter_);
	void dumpInstructions(BIR::DRRACluster *drraCluster_);
	void dumpJumpInstruction(BIR::JumpInstruction *jumpInst_);
	void dumpJumpOperation(BIR::JumpInstruction *jumpInst_);
	void dumpLoopHeaderInstruction(BIR::LoopHeaderInstruction *loopHeaderInst_);
	void dumpLoopHeaderOperation(BIR::LoopHeaderInstruction *loopHeaderInst_);
	void dumpLoopTailInstruction(BIR::LoopTailInstruction *loopTailInst_);
	void dumpLoopTailOperation(BIR::LoopTailInstruction *loopTailInst_);
	void dumpMakeFirstRunFile();
	void dumpMakeRerunFile();
	// void dump_sram_image(BIR::DRRACluster *drraCluster_);
	// void dumpManasCode(BIR::DRRACluster *drraCluster_);
	// void dumpManasBranchInstruction(BIR::BranchInstruction *branchInst_);
	// void dumpManasDpuInstruction(BIR::DPUInstruction *dpuInst_);
	// void dumpManasInstruction(BIR::Instruction *instruction_);
	// void dumpManasJumpInstruction(BIR::JumpInstruction *jumpInst_);
	// void dumpManasRefi1Instruction(BIR::Refi1Instruction *refi1Inst_);
	// void dumpManasRefi2Instruction(BIR::Refi2Instruction *refi2Inst_);
	// void dumpManasRefi3Instruction(BIR::Refi3Instruction *refi3Inst_);
	// void dumpManasSwbInstruction(BIR::SWBInstruction *swbInst_);
	// void dumpManasWaitInstruction(BIR::WaitInstruction *waitInst_);
	void dumpMatlabCode(BIR::DRRACluster *drraCluster_, VIR::MainProgram *mainProgram_);
	string dumpMatlabExpression(VIR::Expression *expression_, bool writingExtra_ = false);
	string dumpMatlabExpressionList(vector<VIR::Expression *> *expressionList_, string seperator_);
	string dumpMatlabPrimitiveFunc(VIR::PrimitiveFunctionCall *primitiveFunction_, vector<VIR::SliceName *> outputNames_, string isFixedPointStr_);
	void dumpMatlabStatement(VIR::Statement *statement_);
	void dumpMatlabStatementList(vector<VIR::Statement *> *statementList_, bool isTop_ = false);
	void dumpOptimizedSchedule(BIR::DRRACluster *drraCluster_);
	void dumpProfilerClockedProcess();
	void dumpProfilerCode(BIR::DRRACluster *drraCluster_, VIR::MainProgram *mainProgram_);
	void dumpProfilerFixedPointStatus(BIR::DRRACluster *drraCluster_);
	void dumpProfilerFunction();
	void dumpProfilerHeader();
	void dumpProfilerRegFileVariables(BIR::DRRACluster *drraCluster_);
	bool dumpProfilerSkipWtCyclesForACell(BIR::DRRACell *drraCell_);
	void dumpProfilerSkipCyclesForAnInst(BIR::SRAMInstruction *sramInst_, int &counter_, int accumulatedExec_, string &lineStr_);
	void dumpProfilerSkipCyclesForAnInstInLoop(BIR::SRAMInstruction *sramInst_, LoopList &loopList_, int currListIndex_, int &counter_, int &accumulatedExec_, string &lineStr_);
	void dumpProfilerSkipCyclesOfWriteAccess(BIR::DRRACluster *drraCluster_);
	void dumpRaccuInstruction(BIR::RACCUInstruction *raccuInst_);
	void dumpRaccuOperation(BIR::RACCUInstruction *raccuInst_);
	void dumpRefiInstruction(BIR::RefiInstruction *refiInst_);
	void dumpRefi1Instruction(BIR::Refi1Instruction *refi1Inst_);
	void dumpRefi2Instruction(BIR::Refi2Instruction *refi2Inst_);
	void dumpRefi3Instruction(BIR::Refi3Instruction *refi3Inst_);
	void dumpRefiOperation(BIR::RefiInstruction *refiInst_);
	void dumpRouteInstruction(BIR::RouteInstruction *routeInst_);
	void dumpRunCmdFile();
	void dumpRunGuiFile();
	void dumpScheduledOperations(BIR::DRRACluster *drraCluster_);
	void dumpScheduledOperationsInCell(BIR::Sequencer *sequencer_);
	void dumpSequencerHeader(const string &message_, const int &row_, const int &col_, const string &commentSymbol_);
	void dumpSramInstruction(BIR::SRAMInstruction *sramInst_);
	void dumpSramOperation(BIR::SRAMInstruction *sramInst_);
	void dumpSynchronizedInstructions(BIR::DRRACluster *drraCluster_);
	void dumpSwbInstruction(BIR::SWBInstruction *swbInst_);
	void dumpTimingModel(BIR::DRRACluster *drraCluster_, VIR::MainProgram *mainProgram_);
	void dumpTitle(const string &title_, const string &commentSymbol_);
	void dumpWaitInstruction(BIR::WaitInstruction *waitInst_);
	void dumpWaveDimarchSection(BIR::DiMArch *dimarch_);
	void dumpWaveDrraSection(BIR::DRRACluster *drraCluster_);
	void dumpWaveFile(BIR::DRRACluster *drraCluster_);
	int getActualDimarchRow(int virtualRowIndex_) const;
	string getBranchModeStr(BIR::BIREnumerations::BranchMode branchMode_);
	string getComponentTypeStr(BIR::BIREnumerations::ComponentType compType_);
	string getDpuModeStr(BIR::BIREnumerations::DPUMode dpuMode_);
	int getInnerLoopExecution(BIR::LoopHeaderInstruction *currLoop_, int iterNo_, bool isTopCall_);
	string getRaccuModeStr(BIR::BIREnumerations::RACCUMode raccuMode_);
	string getRowOrColSignature(int index_, int length_);
	string getStaticOrDynamic(bool isStatic_) const;
	template <int strSize>
	string intToBinaryStr(int value_);
	string intToHexStr(int value_, int strSize_);
	bool isFixedPointVariable(VIR::Identifier *identifier_);
	string realToHexStr(float value_, int strSize_);
	inline void wtManasToken(const int &token_);
	inline void wtManasToken(const string &token_);

	// -----------------------------------------------------
	// VHDL Code Generation Methods

	string dumpVhdlCellInfo(int rowIndex_, int colIndex_, int rowStrLen_, int colStrLen_, int instIncValue_, bool isLastOne);
	void dumpVhdlCellInfos(BIR::DRRACluster *drraCluster_);
	void dumpVhdlCode(BIR::DRRACluster *drraCluster_, VIR::MainProgram *mainProgram_);
	string dumpVhdlBranchInstruction(BIR::BranchInstruction *branchInst_);
	string dumpVhdlDpuInstruction(BIR::DPUInstruction *dpuInst_);
	void dumpVhdlDrraInstruction(BIR::Instruction *instruction_);
	void dumpVhdlHeader(BIR::DRRACluster *drraCluster_);
	void dumpVhdlInstruction(string instStr_);
	void dumpVhdlInstructions(BIR::DRRACluster *drraCluster_);
	string dumpVhdlJumpInstruction(BIR::JumpInstruction *jumpInst_);
	string dumpVhdlLoopHeaderInstruction(BIR::LoopHeaderInstruction *loopHeaderInst_);
	string dumpVhdlLoopTailInstruction(BIR::LoopTailInstruction *loopTailInst_);
	void dumpVhdlMemoryInitialValues(BIR::DiMArch *dimarch_);
	void dumpVhdlMemoryLoad();
	void dumpVhdlPackageCode(BIR::DRRACluster *drraCluster_);
	string dumpVhdlRaccuInstruction(BIR::RACCUInstruction *raccuInst_);
	string dumpVhdlRefi1Instruction(BIR::Refi1Instruction *refi1Inst_);
	string dumpVhdlRefi2Instruction(BIR::Refi2Instruction *refi2Inst_);
	string dumpVhdlRefi3Instruction(BIR::Refi3Instruction *refi3Inst_);
	void dumpVhdlRegFileInitialValues(BIR::DRRACluster *drraCluster_);
	void dumpVhdlRegFileLoad(BIR::DRRACluster *drraCluster_);
	void dumpVhdlRfileInstruction(BIR::RFILEInstruction *rfileInst_);
	string dumpVhdlRouteInstruction(BIR::RouteInstruction *routeInst_);
	void dumpVhdlSramInstruction(BIR::SRAMInstruction *sramInst_);
	string dumpVhdlSwbInstruction(BIR::SWBInstruction *swbInst_);
	string dumpVhdlWaitInstruction(BIR::WaitInstruction *waitInst_);

private:
	typedef std::map<VIR::ObjectDeclaration *, VIR::ObjectDeclaration *> ObjectDeclarationMap;

	// for SRAM depth = 128 -> ceil(log(128) / log(2.0))
	enum
	{
		cSramAddressWidth = 7,
		cRowBitWidth = 2,
		cColBitWidth = 3,
		cVariableNameMaxWidth = 24,
		cDefaultClkPeriod = 10,
		cDefaultExecutionCycle = 1000
	};
	//	enum { cRegFileBankNo = 4 };

	bool _dumpDsEnabled;
	bool _createSubgraph;
	FileType _currDumpDsFileType;
	std::map<string, bool> _includeInNormalMode;
	bool _isVerbose;
	string _finalMatlabCodes; // It contains the codes that should be written to the end of the MATLAB file.
	ObjectDeclarationMap objectDeclarationNodes;
	const string _cComplexNodeStyle;
	const string _cFloatingPointNodeStyle;
	const string _cIdentifierNodeStyle;
	const string _cIntegerNodeStyle;
	const string _cPragmaEdgeStyle;
	const string _cPragmaNodeStyle;
	const string _cStatementEdgeStyle;
	const string _cUniqueStringNodeStyle;
	int _totalExecutionCycle;

public:
	vesyla::codegen::StorageImage *_storage_image;
};

template <int strSize>
string VDump::intToBinaryStr(int value_)
{
	std::bitset<strSize> bs(value_);
	return "\"" + bs.to_string() + "\"";
}

#endif // _VDump_H_
