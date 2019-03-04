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

#ifndef _BEngine_H_
#define _BEngine_H_

#include <map>
#include <vector>
#include "util/Common.hpp"
#include "VIR/VIRIncludeAll.hpp"
#include "BIR/BIRIncludeAll.hpp"
#include "codegen/PrimitiveFuncLib.hpp"
#include "SwbManager.hpp"
#include "codegen/BranchManager.hpp"
#include "schedule/Scheduler.hpp"
#include "sync/Synchronizer.hpp"
#include "codegen/SramAddressTransformer.hpp"
#include "codegen/RaccuVarManager.hpp"
#include "ConfigManager.hpp"

using json = nlohmann::json;

/**
 * TODO
 * Move BEngine to workspace vesyla::engine::back_engine
 */

using namespace vesyla;
using namespace vesyla::bengine;
using namespace vesyla::codegen;
using namespace vesyla::schedule;

class ConfigManager;

using std::map;
using std::string;
using std::vector;

class BEngine
{
private:
	RaccuVarManager _raccu_var_manager;
	ConfigManager *_configManager;

public:
	BEngine();
	~BEngine();

	BIR::DRRACluster *drraCluster() const;
	vector<vector<int>> layout();
	//void run(VIR::MainProgram *mainProgram_);
	void run1(VIR::MainProgram *mainProgram_);
	void run2(map<string, vector<BIR::Instruction *>> &instr_lists);
	int get_max_init_delay() { return (_configManager->hwSetting()).refi1.maxInitDelay; }
	int get_max_middle_delay() { return _configManager->hwSetting().refi2.maxMiddleDelay; }
	int get_max_repetition_delay() { return _configManager->hwSetting().refi2.maxRepetitionNumber; }
	int get_max_repetition_step_value() { return _configManager->hwSetting().refi2.maxRepetitionStepValue; }
	void setRaccuMode(bool enable_);

private:
	enum StatementLocation
	{
		slOutsideIf,
		slThenPart,
		slElsePart,
		slResSharing
	};
#define cAguNum 4
	int cMemWordSplit;

	typedef struct
	{
		VIR::AssignmentStatement *statement;
		StatementLocation stLocation;
		std::string dependentTo;
		int maxScheduledClock;
	} ScheduleComponent;

	struct PortMapComponent
	{
		int cellRow;
		int cellCol;
		int PortNumber;

		PortMapComponent(int cellRow_, int cellCol_, int PortNumber_) : cellRow(cellRow_), cellCol(cellCol_), PortNumber(PortNumber_) {}
	};

	struct InUsedAGUPorts
	{
		bool port[cAguNum];

		InUsedAGUPorts()
		{
			port[0] = false;
			port[1] = false;
			port[2] = false;
			port[3] = false;
		}
	};

	typedef struct
	{
		bool operator()(const VIR::ForStatement *a, const VIR::ForStatement *b) const;
	} CompareMapElements;

	typedef struct
	{
		vector<BIR::Instruction *> instructions;
		int selectiveInstCount; // This variable shows the number of instructions excluding REFI instructions with output mode.
	} InstructionMapType;

	struct MappedInstruction
	{
		VIR::Coordinate cellCoord;
		BIR::Instruction *instruction;

		MappedInstruction(const VIR::Coordinate &cellCoord_, BIR::Instruction *inst_) : cellCoord(cellCoord_), instruction(inst_) {}
	};

	struct MappedRaccuInstruction
	{
		string loopTailSignature;
		BIR::RACCUInstruction *raccuInst;

		MappedRaccuInstruction(const string &loopTailSignature_, BIR::RACCUInstruction *raccuInst_) : loopTailSignature(loopTailSignature_), raccuInst(raccuInst_) {}
	};

	struct RepetitionDelayFlags
	{
		bool enableForSingleOps;
		bool enableForVectorOps;

		void reset()
		{
			enableForSingleOps = false;
			enableForVectorOps = false;
		}
	};

	struct ResSharingInstructionManager
	{
		vector<BIR::SWBInstruction *> **instructions;
		int **insertIndexCheckpoints;
		int **prevRegionFinalIndex;
		int rowNum;
		int colNum;

		void addInstruction(BIR::SWBInstruction *swbInst_, const VIR::Coordinate &coord_);
		void createInstructionLists(int rowNum_, int colNum_);
		int getPrevRegionFinalIndex(const VIR::Coordinate &coord_);
		void incrementIndexCheckpoint(const VIR::Coordinate &coord_);
		void initializeInstructionLists(BIR::DRRACluster *drraCluster_);
		void updateDrraInstructions(BIR::DRRACluster *drraCluster_);
	};

	typedef map<string, BIR::DPUInstruction *> DpuChainMap;
	typedef map<string, InstructionMapType> InstructionsMap;
	typedef map<VIR::ForStatement *, int, CompareMapElements> LoopValuesMap;
	typedef map<string, ScheduleComponent> UnrolledDependencyMap;
	typedef map<string, int> VariablesPortMap;
	typedef map<int, VIR::Coordinate> CoordinateMap;
	typedef map<string, BIR::LoopHeaderInstruction *> LoopHeaderInstructionMap;
	typedef map<VIR::ForStatement *, CoordinateMap> RaccuLoopsCoordinateMap;
	typedef map<string, int> RaccuVariablesAddressMap;
	typedef map<string, BIR::RefiInstruction *> ResourceSharingVariableMap;
	typedef map<string, int> TempVarScheduledValueMap;

	void addCoordListToCoordMap(vector<VIR::Coordinate> &coordList_, CoordinateMap &coordMap_);
	//void addInstructionToMap(BIR::Instruction *instruction_, int statementNo_, LoopValuesMap *loopValues_, const VIR::Coordinate &dpuCoord_, const VIR::Coordinate &instCellCoord_);
	//int aguPortAssignmentForAssignStatement(VIR::AssignmentStatement *assignStatement_, string unrolledNameSignature_, int initStartClock_, bool isLastAddress_, int dpuPosIndex_);
	//void aguPortAssignmentForAVariable(VIR::SliceName *sliceName_, bool isForInputPort_, std::string unrolledNameSignature_, int &updatedStartClock_,
	//vector<PortMapComponent> &statementInUsedAGUPorts_, int dpuPosIndex_, bool isForMemoryTransfer_, int operandOrderNo_ = -1, bool isForTwinStatement_ = false);
	//int aguPortAssignmentForIfCondition(VIR::IfStatement *ifStatement_, string unrolledNameSignature_, int initStartClock_, bool isLastAddress_);
	//void alignDpuChainScheduling();
	//bool conditions(int mode_, void *input_) const;
	//bool calculateDynamicStartAddress(int &startAddress_, VIR::IndexInfo *indexInfo_, int statementNo_, BIR::Instruction *dependentInst_, LoopValuesMap *loopValues_,
	//const VIR::Coordinate &referenceCoord_, const VIR::Coordinate &regFileCoord_, const string &indexVariableName_, bool isForMemory_);
	//BranchInstruction *createBranchInstruction(VIR::BinaryExpression *binaryExpression, int statementNo_, string unrolledStatementNo_, const VIR::Coordinate &coord_, LoopValuesMap *loopValues_);
	//void createDpuInstruction(BIR::DPUInstruction *dpuInst_, int statementNo_, string unrolledStatementNo_, VIR::DPUPragma *dpuPragma_,
	//LoopValuesMap *loopValues_, const VIR::Coordinate &dpuCoord_, int repetition_, bool fixedPointMode_);
	//BIR::WaitInstruction *createDummyWaitInstruction(int statementNo_, LoopValuesMap *loopValues_, VIR::Coordinate &coord_);
	//void createInstructions(vector<VIR::Statement *> *statements_, schedule::Descriptor &d_, json &j_, string path_ = "", int child_group_ = 0);
	//void createInstructionsForArithmeticStatement(VIR::AssignmentStatement *assignStatement_, LoopValuesMap *loopValues_, schedule::Descriptor &d_, json &j_, string path_ = "", int child_group_ = 0);
	//void createInstructionsForAVariable(VIR::SliceName *sliceName_, LoopValuesMap *loopValues_, VIR::ReferencePragma *referencePragma_,
	//int statementNo_, std::string unrolledStatementNo_, vector<MappedInstruction> &mappedRefiInsts_,
	//BIR::DPUInstruction *dpuInst_, int dpuPosIndex_, bool isInput_, int operandOrderNo_ = -1,
	//int twinStatementNo_ = -1, bool isForMemoryTransfer_ = false, int reverse_bits_ = 0, bool fft = false);
	//void createInstructionsForIfCondition(VIR::IfStatement *ifStatement_, LoopValuesMap *loopValues_, schedule::Descriptor &d_, json &j_, string path_, int child_group_ = 0);
	//void createInstructionsForIndirectAddress(VIR::SliceName *sliceName_, LoopValuesMap *loopValues_, VIR::ReferencePragma *referencePragma_,
	//																					int statementNo_, std::string unrolledStatementNo_, vector<MappedInstruction> &mappedRefiInsts_);
	//void createInstructionsForMemoryTransfer(VIR::AssignmentStatement *assignStatement_, LoopValuesMap *loopValues_, schedule::Descriptor &d_, json &j_, string path_, int child_group_ = 0);
	//void createInstructionsForRaccuVariable(VIR::AssignmentStatement *assignStatement_, LoopValuesMap *loopValues_, schedule::Descriptor &d_, json &j_, string path_, int child_group_ = 0);
	//void createInstructionsForRegisterTransfer(VIR::AssignmentStatement *assignStatement_, LoopValuesMap *loopValues_, schedule::Descriptor &d_, json &j_, string path_, int child_group_ = 0);
	//JumpInstruction *createJumpInstruction(int statementNo_, VIR::Coordinate &coord_, LoopValuesMap *loopValues_);
	//map<string, BIR::LoopHeaderInstruction *> createLoopHeaderInstruction(VIR::ForStatement *forStatement_, LoopValuesMap *loopValues_, schedule::Descriptor &d_, json &j_, string path_, int child_group_ = 0);
	//map<string, BIR::LoopTailInstruction *> createLoopTailInstruction(VIR::ForStatement *forStatement_, LoopValuesMap *loopValues_, schedule::Descriptor &d_, json &j_, string path_, int child_group_ = 0);
	//RACCUInstruction *createRaccuInstruction(int statementNo_, BIR::BIREnumerations::RACCUMode raccuMode_, int operand1_, bool isOp1Static_, int operand2_, bool isOp2Static_,
	//																				 int &resultAddress_, const VIR::Coordinate &cellCoord_, const VIR::Coordinate &refCoord_, LoopValuesMap *loopValues_,
	//																				 BIR::Instruction *dependentInst_, int instInsertLocation_ = -1, VIR::ForStatement *forStatement_ = 0);
	//BIR::RefiInstruction *createRefiInstruction(int statementNo_, int twinStatementNo_, string unrolledStatementNo_, const string &variableName_,
	//																						BIR::BIREnumerations::AddressMode addressMode_, BIR::BIREnumerations::PortMode portMode_, int numberOfAddress_,
	//																						int stepValue_, int numberOfRepetition_, int repetitionOffset_, int reverse_bits_, bool is_reverse_bits_static_, bool isDimarchMode_,
	//																						bool isInResSharingRegion_, BIR::DPUInstruction *correspondingDpuInst_, const string &pureVariableName_);
	//void createRfileInstruction(VIR::SliceName *sliceName_, LoopValuesMap *loopValues_, VIR::ReferencePragma *regFilePragma_, int statementNo_,
	//														string unrolledStatementNo_, vector<MappedInstruction> &mappedRefiInsts_, int posIndex_, bool isMemToRegTrans_);
	//BIR::RouteInstruction *createRouteInstruction(int statementNo_, string unrolledStatementNo_, const VIR::Coordinate &memoryCoord_, const VIR::Coordinate &regFileCoord_,
	//																							LoopValuesMap *loopValues_, bool isMemToRegTrans_, const string variableName_);
	//BIR::SRAMInstruction *createSramInstruction(VIR::SliceName *sliceName_, int statementNo_, string unrolledStatementNo_, const VIR::Coordinate &memoryCoord_,
	//																						const VIR::Coordinate &regFileCoord_, LoopValuesMap *loopValues_, bool isMemToRegTrans_, VIR::IndexInfo *indexInfo_,
	//																						int memoryStartAddr_, const string variableName_, BIR::RefiInstruction *correspondingRefiInst_, BIR::RouteInstruction *correspondingRouteInst_);
	//BIR::SWBInstruction *createSwbInstruction(int statementNo_, string unrolledStatementNo_, const string &variableName_, LoopValuesMap *loopValues_, vector<BIR::SWBInstruction *> &swbInstructions_,
	//																					const VIR::Coordinate &referenceCoord_, const BIR::ComponentInfo &srcComponentInfo_, const BIR::ComponentInfo &destComponentInfo_);
	//void createUnrolledDependencyMap(vector<VIR::Statement *> *statements_, StatementLocation location_ = slOutsideIf);
	//void createUnrolledDependencyMapForAStatement(VIR::AssignmentStatement *assignStatement_, LoopValuesMap *loopValues_, StatementLocation location_);
	void determineMemoriesStartAddrAndInitLoad(vector<VIR::Statement *> &totalStatementList_);
	void determineMemoryStartAddrAndInitLoad(VIR::StoragePragma *storagePragma_, VIR::ObjectDeclaration *variableObject_, int **&memoriesCurrIndex_,
																					 int totalElementsNo_, vector<VIR::Expression *> &arrayElements_, bool useArrayElements_, int beginVal_, int incVal_);
	void determineRegFileStartAddrAndInitLoad(VIR::StoragePragma *storagePragma_, VIR::ObjectDeclaration *variableObject_, int **&regFilesCurrIndex_,
																						int totalElementsNo_, vector<VIR::Expression *> &arrayElements_, bool useArrayElements_, int beginVal_, int incVal_);
	void determineRegFilesStartAddrAndInitLoad(vector<VIR::Statement *> &totalStatementList_);
	void determineStorageStartAddrAndInitLoad(VIR::StoragePragma *storagePragma_, VIR::ObjectDeclaration *variableObject_, int **&storagesCurrIndex_);
	//void enableResourceSharingRegion(VIR::PragmaStatement *pragmaStatement_, bool isMapCreation_ = false, bool goToNextMap_ = false);
	void findUtilizedCellsInBody(const vector<VIR::Statement *> &body_, CoordinateMap &allUtilizedCells_);
	void findUtilizedCellsInIfCondition(const VIR::Expression *conditionExpr_, CoordinateMap &allUtilizedCells_);
	void findUtilizedCellsInStatement(const VIR::Statement *statement_, CoordinateMap &allUtilizedCells_);
	//int findDpuRepetition(VIR::AssignmentStatement *assignmentStatement_);
	//int getActualDimarchRow(int virtualRowIndex_) const;
	//VIR::AssignmentStatement *getDependentTempStatement(VIR::AssignmentStatement *tempAssignment_);
	//const string getDpuCellSignature(const VIR::Coordinate &dpuCoord_) const;
	//int getDpuConstantValue(VIR::Expression *constExpr_) const;
	//int getDpuParameterImmediate(VIR::Expression *constExpr_) const;
	//int getDpuParameterReference(VIR::Expression *constExpr_, const VIR::Coordinate &coord_) const;
	// int getDpuModeExecCycle(const string dpuMode_);
	// int getDpuModeValue(const string dpuMode_);
	// int getDpuPortNumbers(const string dpuMode_, bool isInput_, int operandOrderNo_ = -1, int predefinedOutPort = -1);
	// BIR::DPUInstruction *getDpuInstruction(const string &unrolledStatementSig_);
	// int getFixedPointValue(float value_, int fixedPointDigits_) const;
	// string getInstructionSignature(const string &unrolledStatementSig_, const VIR::Coordinate &instCellCoord_);
	// string getLoopSignature(VIR::ForStatement *forStatement_, const VIR::Coordinate &cellCoord_, bool isLoopHead_) const;
	// void getRaccuOperand(const VIR::Expression *operandExpr_, const VIR::Coordinate &coord_, int &operand_, bool &isOpStatic_, bool isRaccuForMemAddress);
	// string getRaccuVariableSignature(const string &name_, const VIR::Coordinate &cellCoord_) const;
	// string getResSharingVariableSignature(VIR::SliceName *sliceName_, const VIR::Coordinate &cellCoord_) const;
	// string getSliceNameSignature(VIR::SliceName *sliceName_) const;
	// int getStoragePositionIndex(VIR::SliceName *sliceName_, int dpuPosIndex_);
	// string getUnrolledLoopValueSignature(const LoopValuesMap *loopValues_) const;
	// string getUnrolledStatementSignature1(int statementNo_, LoopValuesMap *loopValues_, const VIR::Coordinate &dpuCoord_, bool isDeepSignature_ = true) const;
	// string getUnrolledStatementSignature2(int statementNo_, LoopValuesMap *loopValues_, const VIR::Coordinate &dpuCoord_) const;
	// VIR::ForStatement *getUpperLoop(VIR::ControlStatement *controlDependentSt_);
	void init();
	void initializeMemoryAndRegFiles(vector<VIR::Statement *> &totalStatementList_);
	//void initializePortAssignment();
	// bool isAValidIfCondition(VIR::Expression *conditionExpr_) const;
	// bool isAReductionOperation(VIR::AssignmentStatement *assignStatement_) const;
	// bool isAValidDPUConstant(int value_) const;
	// bool isTempVariable(VIR::Identifier *variable_) const;
	// bool needFixedPointCalculation(VIR::AssignmentStatement *assignmentStatement_) const;
	// const string portNameSignature(bool isInput_, int operandOrderNo_) const;
	// inline void updateMaxValue(int &maxValue_, int newValue_);

	// int reserve_raccu_var(int statementNo_, VIR::AddressFunction af_, VIR::Coordinate coord_);
	// RACCUInstruction *create_raccu_instr(int statementNo_, BIR::BIREnumerations::RACCUMode raccuMode_, int operand1_, bool isOp1Static_, int operand2_, bool isOp2Static_, int resultAddress_);

	// void createInstructionsForPrimitiveFunction(VIR::AssignmentStatement *assignStatement_);

	InUsedAGUPorts **_allInUsedAGUPorts; // A 2D array for in-used AGU ports of all DRRA cells
	CoordinateMap _allUtilizedCells;
	CoordinateMap _allUtilizedDimarchCells;
	//	BranchManager _branchManager;
	BranchManager1 _branch_manager;
	VIR::ForStatement *_currLoop;
	ResourceSharingVariableMap *_currResourceSharingVariableMap;
	vector<MappedRaccuInstruction> _deferredRaccuInstructions;
	vector<string> _dependencySignatures;
	DpuChainMap _dpuChainMap;
	BIR::DRRACluster *_drraCluster;
	bool _isNonConstantLoopRegion;
	bool _isResourceSharingRegion;
	bool _insideIfElseBody;
	InstructionsMap _instructionsMap; // A map for keeping the corresponding instructions of statements
	LoopHeaderInstructionMap _loopHeaderInstructionMap;
	int _numberOfRaccuLoops;
	bool _raccuMode;
	RepetitionDelayFlags _RepDelayFlags;
	bool _shouldCalcRepDelay;
	UnrolledDependencyMap _unrolledDependencyMap;
	VariablesPortMap _variablesAGUPortMap;
	RaccuVariablesAddressMap _raccuVariablesAddressMap;
	ResSharingInstructionManager _resSharingInstructionManager;
	vector<ResourceSharingVariableMap> _resourceSharingVariableMaps;
	TempVarScheduledValueMap _tempVarScheduledValueMap;

	PrimitiveFuncLib _primitive_func_lib;
	SilagoCellLib _silago_cell_lib;
	SwbManager _swb_manager;
};

#endif // _BEngine_H_
