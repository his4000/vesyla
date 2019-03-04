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

// TODO: check why test92 address is 0 instead of 1.
// TODO: for raccu variables with complex expression dependent to loop indexes, you need to add multiple raccu instructions
//       instead of raccu variables.

// TODO: After a stable Memory solution remove all the related parts coming with REMOVE: note.

// TODO: There is an important thing when supporting loops with dynamic bounds -> in these cases you shouldn't limit RACCU to only loops with degrees greater than 2

// Overheads of if statement:
// 	- Each branch instruction needs SWB instructions for configuring registers inside the if condition.
//  - Each branch instruction needs REFI and DPU instructions for loading registers and then comparing the results.
//  - If-then-else statement needs a jump instruction after the then-part.
//  - If there is no other statement after the if statement in the same scope/loop level, a dummy wait instruction should be added in order to refer to it as the false address.
//  - If there is no other statement after the if-then-else statement in the same scope/loop level, a dummy wait instruction should be added in order to refer to it as the jump address.
//  - If the then-part and else-part are not balanced in their scheduled timing, a wait statement should be added to the instructions of that part.
//	  * The case that then-part finishing time is smaller than the else-part:
//		  A wait instruction should be inserted before the jump instruction and then both branch and jump instruction addresses
//		  should be incremented by one. Also the else-part instructions need to be rescheduled because of adding an instruction
//		  before them. After this rescheduling, again it should be checked that both else-part and then part are balanced.
//		  Probably after the rescheduling, the finishing time of the then-part is one less than the else-part. In this case,
//		  the waiting cycle of the previously added wait instruction should be incremented by one.
//	  * The case that else-part finishing time is smaller than the then-part:
//		  A wait instruction should be added to the end of the else-part and the jump instruction address should be incremented by one.

#include "BEngine.hpp"
#include "BIR/BIRIncludeAll.hpp"
#include "VIR/VIRIncludeAll.hpp"
#include "VManager/MemoryManager.hpp"
#include "ConfigManager.hpp"
#include <boost/property_tree/xml_parser.hpp>
namespace pt = boost::property_tree;

#include <stack>

using namespace BIR;
using namespace VIR;
using namespace BIR::BIREnumerations;
using namespace VIR::VirEnumerations;

#define UPDATE_NONCONSTANT_REGION_BEGIN         \
	bool oldFlagValue = _isNonConstantLoopRegion; \
	if (!forStatement->isConstant())              \
		_isNonConstantLoopRegion = true;

#define UPDATE_NONCONSTANT_REGION_END \
	_isNonConstantLoopRegion = oldFlagValue;

BEngine::BEngine() : _configManager(new ConfigManager()), _raccuMode(false)
{

	int sram_width;
	int reg_file_width;
	int sram_block_size;
	util::GlobalVar glv;
	CHECK(glv.geti("sram_width", sram_width));
	CHECK(glv.geti("reg_file_width", reg_file_width));
	sram_block_size = sram_width / reg_file_width;
	cMemWordSplit = sram_block_size;

	_configManager->readConfiguration();

	_drraCluster = new DRRACluster(
			_configManager->hwSetting().fabricSetting.drraRowNumber,
			_configManager->hwSetting().fabricSetting.drraColNumber,
			_configManager->hwSetting().fabricSetting.regFileDepth,
			_configManager->hwSetting().fabricSetting.raccuRegFileDepth,
			_configManager->hwSetting().fabricSetting.raccuMaxLoopNo,
			_configManager->hwSetting().fabricSetting.dimarchRowNumber,
			_configManager->hwSetting().fabricSetting.dimarchColNumber,
			_configManager->hwSetting().fabricSetting.sramDepth,
			_configManager->hwSetting().fabricSetting.sramWidth);

	_resSharingInstructionManager.createInstructionLists(_drraCluster->rowNum(),
																											 _drraCluster->colNum());

	string primitive_func_config_file = vesyla::util::SysPath::find_config_file(
			"primitive_func_def.xml");
	if (primitive_func_config_file != "")
	{
		_primitive_func_lib.load(primitive_func_config_file);
		LOG(INFO) << "Use dpu mode defination file: " << primitive_func_config_file;
	}
	else
	{
		LOG(FATAL) << "DPU mode defination file is not found. Abort!";
	}
	string silago_cell_config_file = vesyla::util::SysPath::find_config_file(
			"silago_cell_def.xml");
	if (silago_cell_config_file != "")
	{
		_silago_cell_lib.load(silago_cell_config_file);
		LOG(INFO) << "Use silago cell defination file: " << silago_cell_config_file;
	}
	else
	{
		LOG(FATAL) << "SilagoCell defination file is not found. Abort!";
	}
}

BEngine::~BEngine()
{
	delete _drraCluster;
	delete _configManager;
}

// This method gets the DRRA cluster
BIR::DRRACluster *BEngine::drraCluster() const
{
	return _drraCluster;
}

// This method calculate the layout of the fabric, use it only after the run() function.
vector<vector<int>> BEngine::layout()
{
	vector<vector<int>> l;

	int max_drra_row = 0;
	int max_drra_col = 0;
	for (auto &c : _allUtilizedCells)
	{
		if (max_drra_row < c.second.row + 1)
		{
			max_drra_row = c.second.row + 1;
		}
		if (max_drra_col < c.second.column + 1)
		{
			max_drra_col = c.second.column + 1;
		}
	}
	int max_dimarch_row = 0;
	int max_dimarch_col = 0;
	for (auto &c : _allUtilizedDimarchCells)
	{
		if (max_dimarch_row < c.second.row + 1)
		{
			max_dimarch_row = c.second.row + 1;
		}
		if (max_dimarch_col < c.second.column + 1)
		{
			max_dimarch_col = c.second.column + 1;
		}
	}
	if (max_drra_col < max_dimarch_col)
	{
		max_drra_col = max_dimarch_col;
	}
	else
	{
		max_dimarch_col = max_drra_col;
	}

	int curr_r = 0;
	int curr_c = 0;
	int h = _silago_cell_lib.get_height("DiMArch");
	int w = _silago_cell_lib.get_width("DiMArch");
	int t = _silago_cell_lib.get_type("DiMArch");
	for (int i = 0; i < max_dimarch_row; i++)
	{
		curr_c = 0;
		for (int j = 0; j < max_dimarch_col; j++)
		{
			l.push_back(
					{curr_r, curr_c, h, w, t});
			curr_c += w;
		}
		curr_r += h;
	}
	h = _silago_cell_lib.get_height("DRRA");
	w = _silago_cell_lib.get_width("DRRA");
	t = _silago_cell_lib.get_type("DRRA");
	for (int i = 0; i < max_drra_row; i++)
	{
		curr_c = 0;
		for (int j = 0; j < max_drra_col; j++)
		{
			l.push_back(
					{curr_r, curr_c, h, w, t});
			curr_c += w;
		}
		curr_r += h;
	}

	return l;
}

// This method gets the output of the elaboration phase and performs the tasks of the Vesyla back-end.
// void BEngine::run(VIR::MainProgram *mainProgram_)
// {
// 	init();

// 	// Check in case of dynamic loop presence, RACCU mode should be activated.
// 	if (mainProgram_->hasDynamicLoops() && !_raccuMode)
// 		LOG(FATAL) << ("The code contains dynamic loop. Use the program with -raccu argument!");
// 	initializeMemoryAndRegFiles(mainProgram_->totalAssignmentStatements());
// 	createUnrolledDependencyMap(&mainProgram_->body());
// 	findUtilizedCellsInBody(mainProgram_->body(), _allUtilizedCells);

// 	json j;
// 	j["id"] = 0;
// 	j["name"] = "root";
// 	j["resource"] = json({});
// 	j["node_type"] = 0;
// 	vesyla::schedule::Descriptor d;
// 	d.add_operation(Operation("root"));
// 	d.set_entry("root");
// 	createInstructions(&mainProgram_->body(), d, j["subgraph"], "root", 0);
// 	for (int rowIndex = 0; rowIndex < _drraCluster->rowNum(); ++rowIndex)
// 	{
// 		for (int colIndex = 0; colIndex < _drraCluster->colNum(); ++colIndex)
// 		{
// 			DRRACell *drraCell = _drraCluster->getDRRACell(rowIndex, colIndex);
// 			string coord_signature = to_string(rowIndex) + "_" + to_string(colIndex);
// 			InstructionList &instr_list = drraCell->instructions();
// 			for (int i = 0; i < instr_list.size(); i++)
// 			{
// 				d.add_instruction(coord_signature, instr_list.instruction(i));
// 			}
// 		}
// 	}

// 	vesyla::util::GlobalVar glv;
// 	string path;
// 	CHECK(glv.gets("$CURR_DIR", path));
// 	path = path + "schedule/";
// 	mkdir(path.c_str(), 0755);
// 	string input_file_name;
// 	CHECK(glv.gets("input_file_name", input_file_name));
// 	pt::write_xml(path + input_file_name + "_preschedule.xml", d.dump());

// 	pt::ptree tree;
// 	pt::read_xml(path + input_file_name + "_preschedule.xml", tree);
// 	schedule::Descriptor dd;
// 	dd.load(tree);
// 	pt::write_xml(path + input_file_name + "_preschedule2.xml", dd.dump());

// 	vesyla::schedule::Scheduler s(d);
// 	s.schedule();
// 	pt::write_xml(path + input_file_name + "_postschedule2.xml", s.desc().dump());
// 	s.desc().fill_timestamp();
// 	pt::write_xml(path + input_file_name + "_postschedule.xml", s.desc().dump());

// 	// synchronization
// 	const int maxInitialDelay = _configManager->hwSetting().refi1.maxInitDelay;
// 	const int maxMiddleDelay = _configManager->hwSetting().refi2.maxMiddleDelay;
// 	const int maxRepetitionNumber = _configManager->hwSetting().refi2.maxRepetitionNumber;
// 	const int maxRepetitionOffset = _configManager->hwSetting().refi2.maxRepetitionStepValue;
// 	map<string, vector<BIR::Instruction *>> instr_lists = s.desc().get_instr_lists();
// 	sync::Synchronizer sy(maxInitialDelay, maxMiddleDelay, maxRepetitionNumber, maxRepetitionOffset);
// 	instr_lists = sy.synchronize(instr_lists, s.get_end_time());
// 	for (int rowIndex = 0; rowIndex < _drraCluster->rowNum(); ++rowIndex)
// 	{
// 		for (int colIndex = 0; colIndex < _drraCluster->colNum(); ++colIndex)
// 		{
// 			DRRACell *drraCell = _drraCluster->getDRRACell(rowIndex, colIndex);
// 			string coord_signature = to_string(rowIndex) + "_" + to_string(colIndex);
// 			drraCell->instr_list = instr_lists[coord_signature];
// 		}
// 	}
// }

void BEngine::run1(VIR::MainProgram *mainProgram_)
{
	init();

	// Check in case of dynamic loop presence, RACCU mode should be activated.
	if (mainProgram_->hasDynamicLoops() && !_raccuMode)
		LOG(FATAL) << ("The code contains dynamic loop. Use the program with -raccu argument!");
	initializeMemoryAndRegFiles(mainProgram_->body());
	//	createUnrolledDependencyMap(&mainProgram_->body());
	findUtilizedCellsInBody(mainProgram_->body(), _allUtilizedCells);

	set<int> drra_rows;
	set<int> drra_cols;
	set<int> dimarch_rows;
	set<int> dimarch_cols;
	for (auto &r : _allUtilizedDimarchCells)
	{
		dimarch_rows.insert(r.second.row);
		dimarch_cols.insert(r.second.column);
	}
	for (auto &r : _allUtilizedCells)
	{
		drra_rows.insert(r.second.row);
		drra_cols.insert(r.second.column);
	}

	util::GlobalVar glv;
	glv.puti("utilized_drra_row", drra_rows.size());
	glv.puti("utilized_drra_col", drra_cols.size());
	glv.puti("utilized_dimarch_row", dimarch_rows.size());
	glv.puti("utilized_dimarch_col", dimarch_cols.size());
}
void BEngine::run2(map<string, vector<BIR::Instruction *>> &instr_lists)
{
	for (int rowIndex = 0; rowIndex < _drraCluster->rowNum(); ++rowIndex)
	{
		for (int colIndex = 0; colIndex < _drraCluster->colNum(); ++colIndex)
		{
			DRRACell *drraCell = _drraCluster->getDRRACell(rowIndex, colIndex);
			string coord_signature = to_string(rowIndex) + "_" + to_string(colIndex);
			drraCell->instr_list = instr_lists[coord_signature];
		}
	}
}

// This method sets the RACCU mode.
void BEngine::setRaccuMode(bool enable_)
{
	_raccuMode = enable_;
}

// -------------------------------------
// ---------- Private Methods ----------

// This method checks the coordinates inside the coordList and adds them to the given coordMap if they do not exist in the map.
void BEngine::addCoordListToCoordMap(vector<VIR::Coordinate> &coordList_,
																		 CoordinateMap &coordMap_)
{
	// The signature for CoordinateMap is created on the fly by multiplying
	// the row value by 1000 and adding it to the column value -> 1000r + c
	int coordSignature;

	for (unsigned cIndex = 0; cIndex < coordList_.size(); ++cIndex)
	{
		Coordinate &coord = coordList_[cIndex];
		coordSignature = coord.row * 1000 + coord.column;

		if (coordMap_.find(coordSignature) == coordMap_.end())
			coordMap_[coordSignature] = coord;
	}
}

// // This method adds the given instruction to the instruction map which keeps the corresponding instructions related to each statement
// void BEngine::addInstructionToMap(BIR::Instruction *instruction_,
// 																	int statementNo_, LoopValuesMap *loopValues_,
// 																	const Coordinate &dpuCoord_, const Coordinate &instCellCoord_)
// {
// 	// No need to add jump instruction to the map
// 	if (instruction_->kind() == bktJumpInstruction)
// 		return;

// 	string signature = getInstructionSignature(
// 			getUnrolledStatementSignature1(statementNo_, loopValues_, dpuCoord_,
// 																		 false),
// 			instCellCoord_);

// 	if (_instructionsMap.find(signature) == _instructionsMap.end())
// 	{
// 		_instructionsMap[signature] = InstructionMapType();
// 		_instructionsMap[signature].selectiveInstCount = 0;
// 	}

// 	InstructionMapType &instMap = _instructionsMap[signature];

// 	instMap.instructions.push_back(instruction_);

// 	switch (instruction_->kind())
// 	{
// 	case bktRefiInstruction:
// 	{
// 		RefiInstruction *refiInst = (RefiInstruction *)instruction_;

// 		if (refiInst->isDimarchMode || refiInst->portMode != pmOut)
// 			instMap.selectiveInstCount++;
// 	}
// 	break;

// 	case bktSRAMInstruction:
// 		instMap.selectiveInstCount += 1;
// 		break;

// 	case bktRACCUInstruction:
// 		// Do not increment
// 		break;

// 	default:
// 		instMap.selectiveInstCount++;
// 		break;
// 	}
// }

// // This method is used to unify different conditions in different method.
// // mode_ argument determines the type of the condition and how input_ argument should be casted to proper object.
// // The method returns the proper condition.
// bool BEngine::conditions(int mode_, void *input_) const
// {
// 	bool condition = true;

// 	switch (mode_)
// 	{
// 	case 1:
// 	{
// 		AssignmentStatement *assignStatement = (AssignmentStatement *)input_;

// 		condition =
// 				assignStatement->type() != atTempAssignment && (!assignStatement->twinStatement() || assignStatement->statementNo() < assignStatement->twinStatement()->statementNo());
// 	}
// 	break;

// 	case 2:
// 		condition = ((AssignmentStatement *)input_)->type() != atTempAssignment;
// 		break;

// 	case 3:
// 	{
// 		AssignmentStatement *assignStatement = (AssignmentStatement *)input_;

// 		condition =
// 				assignStatement->type() != atDPUChainConnection && !(assignStatement->lhs().size() == 1 && isTempVariable(
// 																																																			 assignStatement->lhsIdentifiers()[0]));
// 	}
// 	break;

// 	case 4:
// 	{
// 		AssignmentStatement *assignStatement = (AssignmentStatement *)input_;

// 		condition =
// 				assignStatement->type() != atDPUChainConnection && !(assignStatement->lhs().size() == 1 && isTempVariable(assignStatement->lhsIdentifiers()[0])) && !(assignStatement->lhs().size() == 1 && assignStatement->lhsIdentifiers()[0]->object()->pragma()->kind() == ktStoragePragma && ((StoragePragma *)assignStatement->lhsIdentifiers()[0]->object()->pragma())->storageType() == stMemory);
// 	}
// 	break;

// 	case 5:
// 		condition = ((SliceName *)input_)->prefix()->object()->pragma()->kind() != ktDPUChainPragma;
// 		break;

// 	case 6:
// 	{
// 		AssignmentPragma *pragma =
// 				((SliceName *)input_)->prefix()->object()->pragma();

// 		condition =
// 				(pragma->kind() != ktDPUChainPragma && !(pragma->kind() == ktStoragePragma && ((StoragePragma *)pragma)->storageType() == stMemory));
// 	}
// 	break;

// 	case 7:
// 	{
// 	}
// 	break;

// 	default:
// 		break;
// 	}

// 	return condition;
// }

// // This method calculates a dynamic start address for REFI or SRAM instructions.
// // It updates the primary value of the given startAddress_ argument, if it is going to be calculated dynamically.
// // This method returns true if the start address should be calculated dynamically.
// // If there is any non-zero offset in the indexInfo->unrollingMap() or indexInfo->addressInfo.is1ndLevelDynamic or is2ndLevelDynamic are true,
// // the start address should be calculated dynamically.
// // The calculation mechanism for the start address is as follows:
// //  - A RACCU register is assigned to store the dynamic values of the start address (resultAddress).
// //  - A RACCU instruction will be created to initialize the register with the initial value (refiStartAddress).
// //  - For each non-zero offset a RACCU instruction is created to add an offset to the current value of the register
// //    after each RACCU loop iteration.
// //  - The initialization RACCU instruction should be added before the LOOP_HEADER instruction corresponding to the loop
// //    with the highest degree with a non-zero offset in the map.
// //  - The offset RACCU instructions should be added before all corresponding LOOP_TAIL instructions.
// //  - Finally the startAddress field of the refiInst should be set to the dynamic calculation mode referring to the
// //    assigned RACCU register address.
// bool BEngine::calculateDynamicStartAddress(int &startAddress_,
// 																					 IndexInfo *indexInfo_, int statementNo_, Instruction *dependentInst_,
// 																					 LoopValuesMap *loopValues_, const VIR::Coordinate &referenceCoord_,
// 																					 const VIR::Coordinate &regFileCoord_,
// 																					 const string &indexVariableName_, bool isForMemory_)
// {
// 	int resultAddress = -1;
// 	int offset = 0;
// 	ForStatement *outerForStatement = 0;
// 	bool isStartAddressDynamic = false;
// 	int calculatedStartAddress = 0;

// 	int increment = indexInfo_->addressInfo.increment;
// 	int repetitionOffset = indexInfo_->addressInfo.repetitionOffset;

// 	if (isForMemory_)
// 	{
// 		increment /= cMemWordSplit;
// 		repetitionOffset /= cMemWordSplit;
// 		startAddress_ /= cMemWordSplit;
// 	}

// 	if (indexInfo_->addressInfo.is1stLevelDynamic)
// 	{
// 		ForStatement *firstLevelLoop = indexInfo_->addressInfo.firstLevelLoop;

// 		if (indexInfo_->isARangeExpression)
// 			increment = repetitionOffset;

// 		// Create a RACCU instruction for adding the offset to the RACCU register.
// 		// As the corresponding LOOP_TAIL instruction has not been created yet, so this instruction is added to _deferredRaccuInstructions for a postponed insertion.
// 		createRaccuInstruction(statementNo_, rmAdd, resultAddress, false,
// 													 increment, true, resultAddress, regFileCoord_, referenceCoord_,
// 													 loopValues_, dependentInst_, -2, firstLevelLoop);

// 		outerForStatement = firstLevelLoop;
// 		isStartAddressDynamic = true;

// 		if (indexInfo_->addressInfo.firstLevelLoop && indexInfo_->addressInfo.firstLevelLoop->isConstant())
// 			offset = indexInfo_->addressInfo.firstLevelLoop->repetition() * repetitionOffset;
// 		else if (indexInfo_->addressInfo.firstLevelLoop && !indexInfo_->addressInfo.firstLevelLoop->isConstant())
// 			LOG(FATAL) << ("calculateDynamicStartAddress: Internal error for dynamic loops!");
// 	}

// 	if (indexInfo_->addressInfo.is2ndLevelDynamic)
// 	{
// 		if (indexInfo_->addressInfo.firstLevelLoop && indexInfo_->addressInfo.firstLevelLoop->isConstant())
// 		{
// 			LOG(INFO) << ("calculateDynamicStartAddress: RECHECK THIS CASE!");
// 			repetitionOffset -= indexInfo_->addressInfo.firstLevelLoop->repetition() / (isForMemory_ ? cMemWordSplit : 1);
// 		}

// 		// Create a RACCU instruction for adding the offset to the RACCU register.
// 		// As the corresponding LOOP_TAIL instruction has not been created yet, so this instruction is added to _deferredRaccuInstructions for a postponed insertion.
// 		createRaccuInstruction(statementNo_, rmAdd, resultAddress, false, repetitionOffset, true,
// 													 resultAddress, regFileCoord_, referenceCoord_, loopValues_, dependentInst_, -2, indexInfo_->addressInfo.secondLevelLoop);

// 		outerForStatement = indexInfo_->addressInfo.secondLevelLoop;
// 		isStartAddressDynamic = true;
// 	}

// 	// The unrollingMap is ordered from the lowest degree to the highest one.
// 	// The reason is that when calculating the actual offset for a loop level,
// 	// the lower offsets should be subtracted from the higher ones.
// 	for (auto &element : indexInfo_->unrollingMap())
// 	{
// 		if (element.second->unrollingOffset != 0)
// 		{
// 			// Find the outer for statement corresponding which is involved in the indices of the current variable.
// 			if (!outerForStatement || outerForStatement->degree() < element.first->degree())
// 				outerForStatement = element.first;

// 			int unrollingOffset = element.second->unrollingOffset;

// 			if (isForMemory_)
// 			{
// 				if (unrollingOffset % cMemWordSplit != 0)
// 					LOG(FATAL) << ("calculateDynamicStartAddress: For memory transfer, variable slice should be a multiple of 16!");

// 				unrollingOffset /= cMemWordSplit;
// 			}

// 			offset = unrollingOffset - offset;

// 			// Create a RACCU instruction for adding the offset to the RACCU register.
// 			// As the corresponding LOOP_TAIL instruction has not been created yet, so this instruction is added to _deferredRaccuInstructions for a postponed insertion.
// 			if (offset != 0)
// 				createRaccuInstruction(statementNo_, rmAdd, resultAddress,
// 															 false, offset, true, resultAddress, regFileCoord_,
// 															 referenceCoord_, loopValues_, dependentInst_, -2,
// 															 element.first);

// 			offset *= element.first->repetition();

// 			isStartAddressDynamic = true;
// 		}
// 	}

// 	if (isStartAddressDynamic)
// 	{
// 		LoopHeaderInstructionMap::const_iterator iter;

// 		// Find the corresponding LoopHeaderInstruction in the regFileCoord cell and insert the following RACCU instruction before this loop instruction.
// 		iter = _loopHeaderInstructionMap.find(
// 				getLoopSignature(outerForStatement, regFileCoord_, true));

// 		if (iter == _loopHeaderInstructionMap.end())
// 			LOG(FATAL) << ("calculateDynamicStartAddress: Internal Error (cannot find loop header instruction)!");

// 		// Create a RACCU instruction for initializing the RACCU register with the start address.
// 		createRaccuInstruction(statementNo_, rmAdd, startAddress_, true, 0,
// 													 true, resultAddress, regFileCoord_, referenceCoord_,
// 													 loopValues_, 0, iter->second->index);

// 		startAddress_ = resultAddress;
// 	}

// 	if (indexInfo_->isARaccuVariable)
// 	{
// 		isStartAddressDynamic = true;

// 		// Slice name has only one expression and that expression is an identifier
// 		startAddress_ = _raccuVariablesAddressMap[getRaccuVariableSignature(
// 				indexVariableName_, regFileCoord_)];
// 	}

// 	return isStartAddressDynamic;
// }

// // This method creates a BRANCH instruction and adds it to the corresponding DRRA cell and also to the instruction map.
// // 1st argument (binaryExpression) is the condition of the if statement. Currently this condition can only be a binary expression.
// BranchInstruction *BEngine::createBranchInstruction(
// 		VIR::BinaryExpression *binaryExpression, int statementNo_,
// 		string unrolledStatementNo_, const VIR::Coordinate &coord_,
// 		LoopValuesMap *loopValues_)
// {
// 	BranchMode branchMode;
// 	int modeValue;

// 	switch (binaryExpression->operatorType())
// 	{
// 	case botGreaterThan:
// 		branchMode = bmGreater;
// 		modeValue = 1;
// 		break;
// 	case botGreaterOrEqual:
// 		branchMode = bmGreaterOrEqual;
// 		modeValue = 3;
// 		break;
// 	case botEqual:
// 		branchMode = bmEqual;
// 		modeValue = 2;
// 		break;

// 		// These cases should have already been converted to the above types in the elaboration phase.
// 	case botLessThan:
// 	case botLessOrEqual:
// 	case botUnEqual:
// 		LOG(FATAL) << ("createBranchInstruction: (Internal Error) Should have already been converted to another operator!");
// 		break;

// 	default:
// 		LOG(FATAL) << ("createBranchInstruction: Unsupported conditional operator!");
// 	}

// 	BranchInstruction *branchInst = CREATE_OBJECT_B(BranchInstruction);

// 	branchInst->statementNo = statementNo_;
// 	branchInst->unrolledStatementNo = unrolledStatementNo_;
// 	branchInst->falseAddressInstruction = 0; // should be filled later
// 	branchInst->mode = branchMode;
// 	branchInst->modeValue = modeValue;

// 	_branch_manager.set_curr_inst_branch(coord_, branchInst); // It is saved in order to fill the falseAddress

// 	const Coordinate &dpuCoord = coord_;

// 	_drraCluster->getDRRACell(dpuCoord)->instructions().addInstruction(
// 			branchInst, true);

// 	addInstructionToMap(branchInst, statementNo_, loopValues_, dpuCoord,
// 											dpuCoord);
// 	return branchInst;
// }

// // This method creates a DPU instruction and adds it to the corresponding DRRA cell and also to the instruction map.
// // The DPU instruction object is passed as the first argument to the method instead of creating it inside.
// // It is because we need to fill the correspondingSWBInst before setting all other fields.
// void BEngine::createDpuInstruction(BIR::DPUInstruction *dpuInst_,
// 																	 int statementNo_, string unrolledStatementNo_,
// 																	 VIR::DPUPragma *dpuPragma_, LoopValuesMap *loopValues_,
// 																	 const Coordinate &dpuCoord_, int repetition_, bool fixedPointMode_)
// {
// 	dpuInst_->statementNo = statementNo_;
// 	dpuInst_->mode = dpuPragma_->dpuMode;
// 	dpuInst_->saturation = dpuPragma_->saturation;
// 	dpuInst_->modeValue = getDpuModeValue(dpuInst_->mode);
// 	dpuInst_->executionCycle = getDpuModeExecCycle(dpuInst_->mode);
// 	if (dpuInst_->executionCycle < 0)
// 	{
// 		dpuInst_->executionCycle = repetition_ + 1;
// 	}
// 	dpuInst_->unrolledStatementNo = unrolledStatementNo_;
// 	dpuInst_->repetition = repetition_;
// 	dpuInst_->ioChange =
// 			dpuInst_->mode != "silago_dpu_abs_sub_acc" ? dicUnchanged : dicAbsoluteOfOutput;
// 	dpuInst_->constantValue = 0;
// 	dpuInst_->fixedPointMode = fixedPointMode_;

// 	_drraCluster->getDRRACell(dpuCoord_)->instructions().addInstruction(
// 			dpuInst_, true);
// 	_drraCluster->getDRRACell(dpuCoord_)->dpuInstructions().addInstruction(
// 			dpuInst_, false);

// 	addInstructionToMap(dpuInst_, statementNo_, loopValues_, dpuCoord_,
// 											dpuCoord_);
// }

// // This method creates a dummy wait instruction for referencing as the jump address or
// // false address in special cases of branching (see the note on top of the file).
// // The return value is the created dummy instruction.
// BIR::WaitInstruction *BEngine::createDummyWaitInstruction(int statementNo_,
// 																													LoopValuesMap *loopValues_, VIR::Coordinate &coord_)
// {
// 	const Coordinate &dpuCoord = coord_;

// 	// Create a dummy wait instruction
// 	WaitInstruction *dummyWaitInst = CREATE_WAIT_INST(statementNo_,
// 																										getUnrolledStatementSignature2(statementNo_, loopValues_, dpuCoord),
// 																										1, true, 0);

// 	DRRACell *drraCell = _drraCluster->getDRRACell(dpuCoord);
// 	drraCell->instructions().addInstruction(dummyWaitInst, true);

// 	return dummyWaitInst;
// }

// // This method creates all instructions for the sequencer of each DRRA cell
// void BEngine::createInstructions(vector<Statement *> *statements_, schedule::Descriptor &d_, json &j_, string path_, int child_group_)
// {
// 	static LoopValuesMap loopValues;

// 	const int cStatmentsCount = statements_->size();
// 	int prev_stmt_id = -1;
// 	int prev_index = -1;
// 	string prev_name = "";
// 	for (int stIndex = 0; stIndex < cStatmentsCount; ++stIndex)
// 	{
// 		Statement *statement = (*statements_)[stIndex];

// 		switch (statement->kind())
// 		{
// 		case ktAssignmentStatement:
// 		{
// 			AssignmentStatement *assignStatement = (AssignmentStatement *)statement;

// 			json j_stmt;

// 			switch (assignStatement->type())
// 			{
// 			case atTempAssignment:
// 			case atDPUChainConnection:
// 			case atArithmetic:
// 			{
// 				schedule::Operation o(d_.gen_signature(path_));
// 				d_.add_operation(o);
// 				d_.get_mutable_operation(path_).add_child(o.name(), child_group_);
// 				createInstructionsForArithmeticStatement(assignStatement, &loopValues, d_, j_stmt["subgraph"], o.name(), 0);
// 				prev_name = o.name();
// 				break;
// 			}
// 			case atMemoryTransfer:
// 			{
// 				schedule::Operation o(d_.gen_signature(path_));
// 				d_.add_operation(o);
// 				d_.get_mutable_operation(path_).add_child(o.name(), child_group_);
// 				createInstructionsForMemoryTransfer(assignStatement, &loopValues, d_, j_stmt["subgraph"], o.name(), 0);
// 				prev_name = o.name();
// 				break;
// 			}
// 			case atRegisterTransfer:
// 			{
// 				schedule::Operation o(d_.gen_signature(path_));
// 				d_.add_operation(o);
// 				d_.get_mutable_operation(path_).add_child(o.name(), child_group_);
// 				createInstructionsForRegisterTransfer(assignStatement, &loopValues, d_, j_stmt["subgraph"], o.name(), 0);
// 				prev_name = o.name();
// 				break;
// 			}
// 			case atRaccuVariable:
// 			{
// 				schedule::Operation o(d_.gen_signature(path_));
// 				d_.add_operation(o);
// 				d_.get_mutable_operation(path_).add_child(o.name(), child_group_);
// 				createInstructionsForRaccuVariable(assignStatement, &loopValues, d_, j_stmt["subgraph"], o.name(), 0);
// 				prev_name = o.name();
// 				break;
// 			}
// 			default:
// 				break;
// 			}
// 		}
// 		break;

// 		case ktForStatement:
// 		{
// 			ForStatement *forStatement = (ForStatement *)statement;

// 			// TODO: Assuming these values are constant integers
// 			int beginValue = ((Integer *)forStatement->loopRange()->begin())->value;
// 			int step = ((Integer *)forStatement->loopRange()->increment())->value;

// 			// FIXME: Important
// 			// unrolling loop and merge loop with statement is not working, bugs exist.
// 			// Completely disable static loop merging with statement. Every loop will be treated as dynamic loop.

// 			// Normal mode
// 			if (!forStatement->needDynamicHandling())
// 			{
// 				//	If the loop degree is greater than 2, it should be unrolled
// 				if (forStatement->degree() > 2)
// 				{
// 					if (!_raccuMode)
// 					{
// 						for (int i = 0, loopValue = beginValue; i < forStatement->repetition(); ++i, loopValue += step)
// 						{
// 							loopValues[forStatement] = loopValue;

// 							createInstructions(&forStatement->loopBody(), d_, j_, path_, 0);
// 						}
// 						loopValues.erase(forStatement);
// 					}
// 					else
// 					{
// 						// descriptor interface
// 						json j_loop_header_subgraph, j_loop_body_subgraph, j_loop_tail_subgraph;
// 						schedule::Operation o_loop(d_.gen_signature(path_));
// 						schedule::Operation o_head(d_.gen_signature(o_loop.name()));
// 						schedule::Operation o_body(d_.gen_signature(o_loop.name()));
// 						schedule::Operation o_tail(d_.gen_signature(o_loop.name()));
// 						d_.add_operation(o_loop);
// 						d_.add_operation(o_head);
// 						d_.add_operation(o_body);
// 						d_.add_operation(o_tail);
// 						d_.get_mutable_operation(path_).add_child(o_loop.name());
// 						d_.get_mutable_operation(o_loop.name()).add_children({o_head.name(), o_body.name(), o_tail.name()});
// 						d_.add_constraint(Constraint(o_head.name(), o_body.name(), 1, INT_MAX));
// 						d_.add_constraint(Constraint(o_body.name(), o_tail.name(), 1, INT_MAX));

// 						map<string, LoopHeaderInstruction *> looph_instr_map = createLoopHeaderInstruction(forStatement, &loopValues, d_, j_loop_header_subgraph, o_head.name(), 0);
// 						loopValues[forStatement] = beginValue;
// 						_currLoop = forStatement;
// 						createInstructions(&forStatement->loopBody(), d_, j_loop_body_subgraph, o_body.name(), 0);

// 						string prev_raccu_name = "";
// 						for (auto const &element : _allUtilizedCells)
// 						{
// 							Coordinate coordinate = element.second;
// 							DRRACell *drraCell = _drraCluster->getDRRACell(coordinate);
// 							bool flag_found = false;
// 							for (int i = drraCell->instructions().size() - 1; i >= 0; i--)
// 							{
// 								Instruction *instr = drraCell->instructions().instruction(i);
// 								if (instr == looph_instr_map[to_string(coordinate.row) + "_" + to_string(coordinate.column)])
// 								{
// 									flag_found = true;
// 									continue;
// 								}
// 								if (flag_found == true)
// 								{
// 									if (instr->kind() == BIR::BIREnumerations::bktRACCUInstruction)
// 									{
// 										schedule::Operation o_raccu(d_.gen_signature(o_head.name()), 0, {{to_string(coordinate.row) + "_" + to_string(coordinate.column) + "_SEQ", {{0, 0}}}});
// 										d_.add_operation(o_raccu);
// 										d_.get_mutable_operation(o_head.name()).add_child(o_raccu.name(), 0);
// 										instr->labels.push_back(o_raccu.name());

// 										if (prev_raccu_name != "")
// 										{
// 											d_.add_constraint(Constraint(o_raccu.name(), prev_raccu_name, 1, 1));
// 										}
// 										else
// 										{
// 											string loop_header_instr_path = looph_instr_map[to_string(coordinate.row) + "_" + to_string(coordinate.column)]->labels[0];
// 											d_.add_constraint(Constraint(o_raccu.name(), loop_header_instr_path, 1, INT_MAX));
// 										}
// 										prev_raccu_name = o_raccu.name();
// 									}
// 									else
// 									{
// 										break;
// 									}
// 								}
// 							}
// 						}

// 						_currLoop = 0;
// 						loopValues.erase(forStatement);
// 						map<string, LoopTailInstruction *> loopt_instr_map = createLoopTailInstruction(forStatement, &loopValues, d_, j_loop_tail_subgraph, o_tail.name(), 0);

// 						// add extra RACCU dependency
// 						for (int i = 0; i < forStatement->head_raccu_list.size(); i++)
// 						{

// 							Coordinate coordinate = forStatement->head_raccu_coord_list[i];
// 							Instruction *instr = forStatement->head_raccu_list[i];
// 							schedule::Operation o_raccu(d_.gen_signature(o_head.name()), 0, {{to_string(coordinate.row) + "_" + to_string(coordinate.column) + "_SEQ", {{0, 0}}}});
// 							d_.add_operation(o_raccu);
// 							d_.get_mutable_operation(o_head.name()).add_child(o_raccu.name(), 0);
// 							instr->labels.push_back(o_raccu.name());
// 							string loop_header_instr_path = looph_instr_map[to_string(coordinate.row) + "_" + to_string(coordinate.column)]->labels[0];
// 							d_.add_constraint(Constraint(o_raccu.name(), loop_header_instr_path, 1, INT_MAX));
// 							_drraCluster->getDRRACell(coordinate)->instructions().addInstruction(instr, true);
// 						}
// 						for (int i = 0; i < forStatement->tail_raccu_list.size(); i++)
// 						{
// 							Coordinate coordinate = forStatement->tail_raccu_coord_list[i];
// 							Instruction *instr = forStatement->tail_raccu_list[i];
// 							schedule::Operation o_raccu(d_.gen_signature(o_tail.name()), 0, {{to_string(coordinate.row) + "_" + to_string(coordinate.column) + "_SEQ", {{0, 0}}}});
// 							d_.add_operation(o_raccu);
// 							d_.get_mutable_operation(o_tail.name()).add_child(o_raccu.name(), 0);
// 							instr->labels.push_back(o_raccu.name());
// 							string loop_tail_instr_path = loopt_instr_map[to_string(coordinate.row) + "_" + to_string(coordinate.column)]->labels[0];
// 							d_.add_constraint(Constraint(o_raccu.name(), loop_tail_instr_path, 1, INT_MAX));
// 							_drraCluster->getDRRACell(coordinate)->instructions().addInstruction(instr, true);
// 						}
// 					}
// 				}
// 				else
// 				{
// 					UPDATE_NONCONSTANT_REGION_BEGIN;
// 					loopValues[forStatement] = beginValue;

// 					createInstructions(&forStatement->loopBody(), d_, j_, path_);

// 					loopValues.erase(forStatement);
// 					UPDATE_NONCONSTANT_REGION_END;
// 				}
// 			}
// 			else // Performing dynamic management irrelevant of the loop degree
// 			{
// 				// descriptor interface
// 				json j_loop_header_subgraph, j_loop_body_subgraph, j_loop_tail_subgraph;
// 				schedule::Operation o_loop(d_.gen_signature(path_));
// 				schedule::Operation o_head(d_.gen_signature(o_loop.name()));
// 				schedule::Operation o_body(d_.gen_signature(o_loop.name()));
// 				schedule::Operation o_tail(d_.gen_signature(o_loop.name()));
// 				d_.add_operation(o_loop);
// 				d_.add_operation(o_head);
// 				d_.add_operation(o_body);
// 				d_.add_operation(o_tail);
// 				d_.get_mutable_operation(path_).add_child(o_loop.name());
// 				d_.get_mutable_operation(o_loop.name()).add_children({o_head.name(), o_body.name(), o_tail.name()});
// 				d_.add_constraint(Constraint(o_head.name(), o_body.name(), 1, INT_MAX));
// 				d_.add_constraint(Constraint(o_body.name(), o_tail.name(), 1, INT_MAX));

// 				map<string, LoopHeaderInstruction *> looph_instr_map = createLoopHeaderInstruction(forStatement, &loopValues, d_, j_loop_header_subgraph, o_head.name(), 0);
// 				loopValues[forStatement] = beginValue;
// 				_currLoop = forStatement;
// 				createInstructions(&forStatement->loopBody(), d_, j_loop_body_subgraph, o_body.name(), 0);

// 				string prev_raccu_name = "";
// 				for (auto const &element : _allUtilizedCells)
// 				{
// 					Coordinate coordinate = element.second;
// 					DRRACell *drraCell = _drraCluster->getDRRACell(coordinate);
// 					bool flag_found = false;
// 					for (int i = drraCell->instructions().size() - 1; i >= 0; i--)
// 					{
// 						Instruction *instr = drraCell->instructions().instruction(i);
// 						if (instr == looph_instr_map[to_string(coordinate.row) + "_" + to_string(coordinate.column)])
// 						{
// 							flag_found = true;
// 							continue;
// 						}
// 						if (flag_found == true)
// 						{
// 							if (instr->kind() == BIR::BIREnumerations::bktRACCUInstruction)
// 							{
// 								schedule::Operation o_raccu(d_.gen_signature(o_head.name()), 0, {{to_string(coordinate.row) + "_" + to_string(coordinate.column) + "_SEQ", {{0, 0}}}});
// 								d_.add_operation(o_raccu);
// 								d_.get_mutable_operation(o_head.name()).add_child(o_raccu.name(), 0);
// 								instr->labels.push_back(o_raccu.name());

// 								if (prev_raccu_name != "")
// 								{
// 									d_.add_constraint(Constraint(o_raccu.name(), prev_raccu_name, 1, 1));
// 								}
// 								else
// 								{
// 									string loop_header_instr_path = looph_instr_map[to_string(coordinate.row) + "_" + to_string(coordinate.column)]->labels[0];
// 									d_.add_constraint(Constraint(o_raccu.name(), loop_header_instr_path, 1, INT_MAX));
// 								}
// 								prev_raccu_name = o_raccu.name();
// 							}
// 							else
// 							{
// 								break;
// 							}
// 						}
// 					}
// 				}

// 				_currLoop = 0;
// 				loopValues.erase(forStatement);
// 				map<string, LoopTailInstruction *> loopt_instr_map = createLoopTailInstruction(forStatement, &loopValues, d_, j_loop_tail_subgraph, o_tail.name(), 0);

// 				// add extra RACCU dependency
// 				for (int i = 0; i < forStatement->head_raccu_list.size(); i++)
// 				{

// 					Coordinate coordinate = forStatement->head_raccu_coord_list[i];
// 					Instruction *instr = forStatement->head_raccu_list[i];
// 					schedule::Operation o_raccu(d_.gen_signature(o_head.name()), 0, {{to_string(coordinate.row) + "_" + to_string(coordinate.column) + "_SEQ", {{0, 0}}}});
// 					d_.add_operation(o_raccu);
// 					d_.get_mutable_operation(o_head.name()).add_child(o_raccu.name(), 0);
// 					instr->labels.push_back(o_raccu.name());
// 					string loop_header_instr_path = looph_instr_map[to_string(coordinate.row) + "_" + to_string(coordinate.column)]->labels[0];
// 					d_.add_constraint(Constraint(o_raccu.name(), loop_header_instr_path, 1, INT_MAX));
// 					_drraCluster->getDRRACell(coordinate)->instructions().addInstruction(instr, true);
// 				}
// 				for (int i = 0; i < forStatement->tail_raccu_list.size(); i++)
// 				{
// 					Coordinate coordinate = forStatement->tail_raccu_coord_list[i];
// 					Instruction *instr = forStatement->tail_raccu_list[i];
// 					schedule::Operation o_raccu(d_.gen_signature(o_tail.name()), 0, {{to_string(coordinate.row) + "_" + to_string(coordinate.column) + "_SEQ", {{0, 0}}}});
// 					d_.add_operation(o_raccu);
// 					d_.get_mutable_operation(o_tail.name()).add_child(o_raccu.name(), 0);
// 					instr->labels.push_back(o_raccu.name());
// 					string loop_tail_instr_path = loopt_instr_map[to_string(coordinate.row) + "_" + to_string(coordinate.column)]->labels[0];
// 					d_.add_constraint(Constraint(o_raccu.name(), loop_tail_instr_path, 1, INT_MAX));
// 					_drraCluster->getDRRACell(coordinate)->instructions().addInstruction(instr, true);
// 				}
// 			}
// 			prev_index = stIndex;
// 			break;
// 		}

// 		case ktIfStatement:
// 		{
// 			// Descriptor interface
// 			json j_cond, j_body, j_end, j_body_0;

// 			schedule::Operation o_stmt(d_.gen_signature(path_));
// 			schedule::Operation o_cond(d_.gen_signature(o_stmt.name()));
// 			schedule::Operation o_body(d_.gen_signature(o_stmt.name()));
// 			schedule::Operation o_end(d_.gen_signature(o_stmt.name()));
// 			d_.add_operation(o_stmt);
// 			d_.add_operation(o_cond);
// 			d_.add_operation(o_body);
// 			d_.add_operation(o_end);
// 			d_.get_mutable_operation(path_).add_child(o_stmt.name(), child_group_);
// 			d_.get_mutable_operation(o_stmt.name()).add_children({o_cond.name(), o_body.name(), o_end.name()}, 0);
// 			d_.add_constraint(Constraint(o_cond.name(), o_body.name(), 1, INT_MAX));
// 			d_.add_constraint(Constraint(o_body.name(), o_end.name(), 1, INT_MAX));
// 			prev_name = o_stmt.name();

// 			IfStatement *ifStatement = (IfStatement *)statement;
// 			_branch_manager.push(ifStatement);

// 			createInstructionsForIfCondition(ifStatement, &loopValues, d_, j_cond["subgraph"], o_cond.name());

// 			schedule::Operation o_body_0(d_.gen_signature(o_body.name()));
// 			schedule::Operation o_body_1(d_.gen_signature(o_body.name()));
// 			d_.add_operation(o_body_0);
// 			d_.add_operation(o_body_1);
// 			d_.get_mutable_operation(o_body.name()).add_children({o_body_0.name(), o_body_1.name()}, 0);
// 			d_.add_constraint(Constraint(o_body_0.name(), o_body_1.name(), 1, INT_MAX));

// 			_swb_manager.clear();
// 			createInstructions(&ifStatement->thenPart(), d_, j_body_0["subgraph"], o_body_0.name(), 0);

// 			for (auto &coord : ifStatement->conditionPragma()->coordinates)
// 			{
// 				JumpInstruction *jump_inst = createJumpInstruction(ifStatement->statementNo(), coord, &loopValues);
// 				_branch_manager.set_curr_inst_jump(coord, jump_inst);
// 				BranchInstruction *branch_inst = _branch_manager.get_curr_inst_branch(coord);
// 				branch_inst->falseAddressInstruction = jump_inst;

// 				schedule::Operation o_jump(d_.gen_signature(o_body_1.name()), 0, {{to_string(coord.row) + "_" + to_string(coord.column) + "_SEQ", {{0, 0}}}});
// 				d_.add_operation(o_jump);
// 				d_.get_mutable_operation(o_body_1.name()).add_child(o_jump.name(), 0);
// 			}

// 			_swb_manager.clear();

// 			for (auto &coord : ifStatement->conditionPragma()->coordinates)
// 			{
// 				WaitInstruction *wait_inst = createDummyWaitInstruction(ifStatement->statementNo(), &loopValues, coord);

// 				schedule::Operation o_wait(d_.gen_signature(o_body.name()), 0, {{to_string(coord.row) + "_" + to_string(coord.column) + "_SEQ", {{0, 0}}}});
// 				d_.add_operation(o_wait);
// 				d_.get_mutable_operation(o_body.name()).add_child(o_wait.name(), 1);
// 				wait_inst->labels.push_back(o_wait.name());
// 			}

// 			for (auto &coord : ifStatement->conditionPragma()->coordinates)
// 			{
// 				WaitInstruction *wait_inst = createDummyWaitInstruction(ifStatement->statementNo(), &loopValues, coord);
// 				_branch_manager.set_curr_inst_dummy_wait(coord, wait_inst);
// 				JumpInstruction *jump_inst = _branch_manager.get_curr_inst_jump(coord);
// 				jump_inst->jumpAddressInstruction = wait_inst;

// 				schedule::Operation o_wait(d_.gen_signature(o_end.name()), 0, {{to_string(coord.row) + "_" + to_string(coord.column) + "_SEQ", {{0, 0}}}});
// 				d_.add_operation(o_wait);
// 				d_.get_mutable_operation(o_end.name()).add_child(o_wait.name(), 0);
// 				wait_inst->labels.push_back(o_wait.name());
// 			}

// 			_branch_manager.pop();
// 			prev_index = stIndex;
// 			break;
// 		}
// 		case ktIfThenElseStatement:
// 		{
// 			// Descriptor interface
// 			json j_cond, j_body, j_end, j_body_0;

// 			schedule::Operation o_stmt(d_.gen_signature(path_));
// 			schedule::Operation o_cond(d_.gen_signature(o_stmt.name()));
// 			schedule::Operation o_body(d_.gen_signature(o_stmt.name()));
// 			schedule::Operation o_end(d_.gen_signature(o_stmt.name()));
// 			d_.add_operation(o_stmt);
// 			d_.add_operation(o_cond);
// 			d_.add_operation(o_body);
// 			d_.add_operation(o_end);
// 			d_.get_mutable_operation(path_).add_child(o_stmt.name(), child_group_);
// 			d_.get_mutable_operation(o_stmt.name()).add_children({o_cond.name(), o_body.name(), o_end.name()}, 0);
// 			d_.add_constraint(Constraint(o_cond.name(), o_body.name(), 1, INT_MAX));
// 			d_.add_constraint(Constraint(o_body.name(), o_end.name(), 1, INT_MAX));
// 			prev_name = o_stmt.name();

// 			IfThenElseStatement *ifThenElseStatement = (IfThenElseStatement *)statement;
// 			_branch_manager.push(ifThenElseStatement);

// 			createInstructionsForIfCondition(ifThenElseStatement, &loopValues, d_, j_cond["subgraph"], o_cond.name());

// 			schedule::Operation o_body_0(d_.gen_signature(o_body.name()));
// 			schedule::Operation o_body_1(d_.gen_signature(o_body.name()));
// 			d_.add_operation(o_body_0);
// 			d_.add_operation(o_body_1);
// 			d_.get_mutable_operation(o_body.name()).add_children({o_body_0.name(), o_body_1.name()}, 0);
// 			d_.add_constraint(Constraint(o_body_0.name(), o_body_1.name(), 1, INT_MAX));

// 			_swb_manager.clear();
// 			createInstructions(&ifThenElseStatement->thenPart(), d_, j_body_0["subgraph"], o_body_0.name(), 0);

// 			for (auto &coord : ifThenElseStatement->conditionPragma()->coordinates)
// 			{
// 				JumpInstruction *jump_inst = createJumpInstruction(ifThenElseStatement->statementNo(), coord, &loopValues);
// 				_branch_manager.set_curr_inst_jump(coord, jump_inst);
// 				BranchInstruction *branch_inst = _branch_manager.get_curr_inst_branch(coord);
// 				branch_inst->falseAddressInstruction = jump_inst;

// 				schedule::Operation o_jump(d_.gen_signature(o_body_1.name()), 0, {{to_string(coord.row) + "_" + to_string(coord.column) + "_SEQ", {{0, 0}}}});
// 				d_.add_operation(o_jump);
// 				d_.get_mutable_operation(o_body_1.name()).add_child(o_jump.name(), 0);
// 			}

// 			_swb_manager.clear();
// 			createInstructions(&ifThenElseStatement->elsePart(), d_, j_body["subgraph_extra"], o_body.name(), 1);

// 			for (auto &coord : ifThenElseStatement->conditionPragma()->coordinates)
// 			{
// 				WaitInstruction *wait_inst = createDummyWaitInstruction(ifThenElseStatement->statementNo(), &loopValues, coord);
// 				_branch_manager.set_curr_inst_dummy_wait(coord, wait_inst);
// 				JumpInstruction *jump_inst = _branch_manager.get_curr_inst_jump(coord);
// 				jump_inst->jumpAddressInstruction = wait_inst;

// 				schedule::Operation o_wait(d_.gen_signature(o_end.name()), 0, {{to_string(coord.row) + "_" + to_string(coord.column) + "_SEQ", {{0, 0}}}});
// 				d_.add_operation(o_wait);
// 				d_.get_mutable_operation(o_end.name()).add_child(o_wait.name(), 0);
// 				wait_inst->labels.push_back(o_wait.name());
// 			}

// 			_branch_manager.pop();
// 			prev_index = stIndex;
// 			break;
// 		}
// 		case ktPragmaStatement:
// 			enableResourceSharingRegion((PragmaStatement *)statement, true);
// 			break;

// 		default:
// 			LOG(FATAL) << ("createInstructions: Unknown statement!");
// 			break;
// 		}
// 	}
// }

// // This method creates the corresponding instructions for an assignment statement (that is related to a DPU instruction)
// // TODO: currently it only supports slices names in left hand side and right hand side of the assignment statement
// void BEngine::createInstructionsForArithmeticStatement(
// 		VIR::AssignmentStatement *assignStatement_,
// 		LoopValuesMap *loopValues_, schedule::Descriptor &d_, json &j_, string path_, int child_group_)
// {
// 	int id_counter = 0;
// 	int first_dpu_id = -1;
// 	int curr_dpu_id = -1;
// 	int saved_id_counter = 0;
// 	string first_dpu_name = "";
// 	string curr_dpu_name = "";
// 	vector<string> input_refis_name;

// 	const string errMessage =
// 			"createInstructionsForArithmeticStatement: Improper dpu constant operation!";

// 	DPUPragma *dpuPragma = (DPUPragma *)(conditions(2, assignStatement_) ? assignStatement_->pragma() : getDependentTempStatement(assignStatement_)->pragma());

// 	vector<MappedInstruction> mappedRefiInsts;

// 	for (unsigned dpuPosIndex = 0; dpuPosIndex < dpuPragma->coordinates.size();
// 			 ++dpuPosIndex)
// 	{
// 		const Coordinate &dpuCoord = dpuPragma->coordinates[dpuPosIndex];

// 		mappedRefiInsts.clear();

// 		string unrolledStatementNo = getUnrolledStatementSignature2(
// 				assignStatement_->statementNo(), loopValues_, dpuCoord);
// 		string unrolledTempStatementNo;
// 		int tempStatementNo;

// 		DPUInstruction *dpuInst;
// 		bool isNewDpu = false;

// 		const int twinStatementNo =
// 				!assignStatement_->twinStatement() ? -1 : assignStatement_->twinStatement()->statementNo();

// 		// Instruction creation for normal assign statements or for the first statement of twin statements
// 		if (conditions(1, assignStatement_))
// 		{
// 			// Create DPU instruction
// 			dpuInst = CREATE_OBJECT_B(DPUInstruction);

// 			dpuInst->twinStatementNo = twinStatementNo;

// 			isNewDpu = true;

// 			// Creating instructions (SWB and REFI) for input variables of the assignment statement
// 			for (int i = 0; i < assignStatement_->rhsSliceNames().size(); ++i)
// 			{
// 				if (assignStatement_->rhsSliceNames()[i]->prefix()->object()->objectType() == VIR::VirEnumerations::otRegisterFile)
// 				{
// 					createInstructionsForAVariable(
// 							assignStatement_->rhsSliceNames()[i], loopValues_,
// 							dpuPragma, assignStatement_->statementNo(),
// 							unrolledStatementNo, mappedRefiInsts, dpuInst,
// 							dpuPosIndex, true, i + 1, twinStatementNo);
// 				}
// 			}
// 		}
// 		else if (conditions(2, assignStatement_)) // Instruction for the second statement of twin statements
// 		{
// 			string signature = getInstructionSignature(
// 					getUnrolledStatementSignature1(
// 							assignStatement_->twinStatement()->statementNo(),
// 							loopValues_, dpuCoord, false),
// 					dpuCoord);

// 			dpuInst = getDpuInstruction(signature);

// 			// Creating instructions (SWB and REFI) for the first input variable of the second twin statement
// 			createInstructionsForAVariable(assignStatement_->rhsSliceNames()[0],
// 																		 loopValues_, dpuPragma, assignStatement_->statementNo(),
// 																		 unrolledStatementNo, mappedRefiInsts, dpuInst, dpuPosIndex,
// 																		 true, 1, twinStatementNo);
// 		}
// 		else // Temp statement
// 		{
// 			AssignmentStatement *dependentStatement =
// 					getDependentTempStatement(assignStatement_);

// 			unrolledTempStatementNo = getUnrolledStatementSignature2(
// 					dependentStatement->statementNo(), loopValues_, dpuCoord);
// 			tempStatementNo = dependentStatement->statementNo();

// 			if (dependentStatement->twinStatement() && dependentStatement->statementNo() > dependentStatement->twinStatement()->statementNo())
// 				dependentStatement = dependentStatement->twinStatement();

// 			string signature = getInstructionSignature(
// 					getUnrolledStatementSignature1(
// 							dependentStatement->statementNo(), loopValues_,
// 							dpuCoord, false),
// 					dpuCoord);

// 			dpuInst = getDpuInstruction(signature);
// 		}

// 		// Creating instructions (SWB, REFI) for the output variable of the assignment statement
// 		if (conditions(3, assignStatement_))
// 		{
// 			int i = 0;
// 			for (auto &sn : assignStatement_->lhsSliceNames())
// 			{
// 				createInstructionsForAVariable(sn, loopValues_, dpuPragma,
// 																			 conditions(2, assignStatement_) ? assignStatement_->statementNo() : tempStatementNo,
// 																			 conditions(2, assignStatement_) ? unrolledStatementNo : unrolledTempStatementNo,
// 																			 mappedRefiInsts, dpuInst, dpuPosIndex, false, i + 1);
// 				i++;
// 			}
// 		}
// 		// In case of DPU chain connection, fill the corresponding map for statement numbers.
// 		else if (assignStatement_->type() == atDPUChainConnection)
// 		{
// 			for (auto &sn : assignStatement_->lhsSliceNames())
// 			{
// 				_dpuChainMap[getSliceNameSignature(sn)] = dpuInst;
// 			}
// 		}

// 		// Adding all input REFI instructions of this assignment statement to the related instruction list of DRRA cells.
// 		json resources;
// 		vector<int> input_refis;

// 		if (conditions(2, assignStatement_))
// 		{
// 			if (!_isResourceSharingRegion)
// 			{
// 				// descriptor interface
// 				for (auto &mappedRefiInst : mappedRefiInsts)
// 				{
// 					if (((RefiInstruction *)mappedRefiInst.instruction)->portMode != pmOut)
// 					{
// 						RefiInstruction *refi_instr = (BIR::RefiInstruction *)(mappedRefiInst.instruction);
// 						int portno = -1;
// 						int num_of_addr = refi_instr->numberOfAddress.value + 1;
// 						int exe_cycle = (num_of_addr + (num_of_addr - 1) * refi_instr->middleDelay.value) + ((num_of_addr + (num_of_addr - 1) * refi_instr->middleDelay.value) - 1) * refi_instr->repetitionDelay.value;
// 						string res_name = "";
// 						if (resources[to_string(mappedRefiInst.cellCoord.row) + "_" + to_string(mappedRefiInst.cellCoord.column) + "_AGU2"].empty())
// 						{
// 							res_name = to_string(mappedRefiInst.cellCoord.row) + "_" + to_string(mappedRefiInst.cellCoord.column) + "_AGU2";
// 							static_cast<BIR::RefiInstruction *>(mappedRefiInst.instruction)->portNo = 2;
// 							resources[to_string(mappedRefiInst.cellCoord.row) + "_" + to_string(mappedRefiInst.cellCoord.column) + "_AGU2"] = 1;
// 							portno = 2;
// 						}
// 						else
// 						{
// 							res_name = to_string(mappedRefiInst.cellCoord.row) + "_" + to_string(mappedRefiInst.cellCoord.column) + "_AGU3";
// 							resources[to_string(mappedRefiInst.cellCoord.row) + "_" + to_string(mappedRefiInst.cellCoord.column) + "_AGU3"] = 1;
// 							static_cast<BIR::RefiInstruction *>(mappedRefiInst.instruction)->portNo = 3;
// 							portno = 3;
// 						}
// 						schedule::Operation o_refi0(d_.gen_signature(path_), 0, {{to_string(mappedRefiInst.cellCoord.row) + "_" + to_string(mappedRefiInst.cellCoord.column) + "_SEQ", {{0, 0}}}, {res_name, {{0, INT_MAX}}}});
// 						d_.add_operation(o_refi0);
// 						d_.get_mutable_operation(path_).add_child(o_refi0.name(), child_group_);
// 						schedule::Operation o_refi1(d_.gen_signature(path_), 0, {});
// 						d_.add_operation(o_refi1);
// 						d_.get_mutable_operation(path_).add_child(o_refi1.name(), child_group_);
// 						input_refis_name.push_back(o_refi1.name());
// 						schedule::Operation o_refi2(d_.gen_signature(path_), 0, {{res_name, {{INT_MIN, 0}}}});
// 						d_.add_operation(o_refi2);
// 						d_.get_mutable_operation(path_).add_child(o_refi2.name(), child_group_);
// 						d_.add_constraint(Constraint(o_refi0.name(), o_refi1.name(), 0, INT_MAX));
// 						d_.add_constraint(Constraint(o_refi1.name(), o_refi2.name(), exe_cycle - 1, exe_cycle - 1));

// 						refi_instr->labels.push_back(o_refi0.name());
// 						refi_instr->labels.push_back(o_refi1.name());
// 						refi_instr->labels.push_back(o_refi2.name());

// 						for (auto &swb_instr : ((BIR::RefiInstruction *)(mappedRefiInst.instruction))->correspondingSWBInsts)
// 						{
// 							swb_instr->source.portNo = portno;
// 							schedule::Operation o_swb0(d_.gen_signature(path_), 0, {{to_string(swb_instr->destination.cellCoordinate.row) + "_" + to_string(swb_instr->destination.cellCoordinate.column) + "_SEQ", {{0, 0}}}, {swb_instr->source.to_string() + "_" + swb_instr->destination.to_string(), {{0, INT_MAX}}}});
// 							d_.add_operation(o_swb0);
// 							d_.get_mutable_operation(path_).add_child(o_swb0.name(), child_group_);
// 							schedule::Operation o_swb2(d_.gen_signature(path_), 0, {{swb_instr->source.to_string() + "_" + swb_instr->destination.to_string(), {{INT_MIN, 0}}}});
// 							d_.add_operation(o_swb2);
// 							d_.get_mutable_operation(path_).add_child(o_swb2.name(), child_group_);
// 							d_.add_constraint(Constraint(o_swb0.name(), o_swb2.name(), 1, INT_MAX));
// 							d_.add_constraint(Constraint(o_refi2.name(), o_swb2.name(), 0, INT_MAX));
// 							d_.add_constraint(Constraint(o_swb0.name(), o_refi1.name(), 1, INT_MAX));
// 							swb_instr->labels.push_back(o_swb0.name());
// 							swb_instr->labels.push_back(o_swb2.name());
// 						}

// 						if (((BIR::RefiInstruction *)(mappedRefiInst.instruction))->dependentTo)
// 						{
// 							string p = ((BIR::RefiInstruction *)(mappedRefiInst.instruction))->dependentTo->labels[1];
// 							d_.add_constraint(Constraint(p, o_refi1.name(), 1, INT_MAX));
// 						}

// 						_drraCluster->getDRRACell(mappedRefiInst.cellCoord)->instructions().addInstruction(mappedRefiInst.instruction, true);
// 					}
// 				}
// 			}
// 			else // For resource sharing region we want all REFI instructions with input port mode to be close to each other.
// 			{
// 				for (auto &mappedRefiInst : mappedRefiInsts)
// 				{
// 					InstructionList &instList = _drraCluster->getDRRACell(mappedRefiInst.cellCoord)->instructions();
// 					RefiInstruction *refiInst = (RefiInstruction *)mappedRefiInst.instruction;

// 					if (refiInst->portMode != pmOut)
// 					{
// 						int insertIndex = instList.size();

// 						int prevRegionFinalIndex = _resSharingInstructionManager.getPrevRegionFinalIndex(mappedRefiInst.cellCoord);

// 						if (prevRegionFinalIndex != 0)
// 						{
// 							for (int index = instList.size() - 1;
// 									 index >= prevRegionFinalIndex; --index)
// 							{
// 								if (instList.instruction(index)->kind() == bktRefiInstruction)
// 								{
// 									insertIndex = index + 1;
// 									break;
// 								}
// 							}
// 						}

// 						instList.insert(insertIndex, refiInst);
// 					}
// 				}
// 			}
// 		}
// 		else // for temp assignment statement, deprecated!!
// 		{
// 			for (auto &mappedRefiInst : mappedRefiInsts)
// 			{
// 				InstructionList &instList = _drraCluster->getDRRACell(mappedRefiInst.cellCoord)->instructions();
// 				AssignmentStatement *dependentStatement = getDependentTempStatement(assignStatement_);
// 				int insertIndex = instList.size();

// 				// Find the position of the input REFI instructions related to the dependent assign
// 				// statement and insert the new output REFI instructions close to those instructions.
// 				for (int index = instList.size() - 1; index >= 0; --index)
// 				{
// 					Instruction *oldInstruction = instList.instruction(index);
// 					if (oldInstruction->unrolledStatementNo == unrolledTempStatementNo && oldInstruction->kind() == bktRefiInstruction)
// 					{
// 						insertIndex = index + 1;
// 						break;
// 					}
// 				}

// 				instList.insert(insertIndex, mappedRefiInst.instruction);
// 			}
// 		}

// 		// Complete the created DPU instruction
// 		if (isNewDpu)
// 		{
// 			createDpuInstruction(dpuInst, assignStatement_->statementNo(),
// 													 unrolledStatementNo, dpuPragma, loopValues_, dpuCoord,
// 													 findDpuRepetition(assignStatement_),
// 													 needFixedPointCalculation(assignStatement_));

// 			////////////////////////////////////////////////////////////////
// 			// DEBUG new schedule graph
// 			////////////////////////////////////////////////////////////////
// 			// Deprecated: json interface
// 			// json j_dpu;
// 			// j_dpu["id"] = id_counter;
// 			// if (dpuPosIndex == 0)
// 			// {
// 			// 	first_dpu_id = j_dpu["id"];
// 			// }
// 			// curr_dpu_id = id_counter;
// 			// j_dpu["name"] = path_ + to_string(id_counter);
// 			// dpuInst->labels.push_back(j_dpu["name"]);
// 			// id_counter++;
// 			// json j_dpu_res;
// 			// j_dpu_res[to_string(dpuCoord.row) + "_" + to_string(dpuCoord.column) + "_SEQ"].push_back(0);
// 			// j_dpu_res[to_string(dpuCoord.row) + "_" + to_string(dpuCoord.column) + "_SEQ"].push_back(0);
// 			// j_dpu_res[to_string(dpuCoord.row) + "_" + to_string(dpuCoord.column) + "_DPU"].push_back(0);
// 			// j_dpu_res[to_string(dpuCoord.row) + "_" + to_string(dpuCoord.column) + "_DPU"].push_back(
// 			// 		dpuInst->repetition + dpuInst->executionCycle);
// 			// j_dpu["resource"] = j_dpu_res;
// 			// j_dpu["node_type"] = 2;
// 			// j_["vertex"].push_back(j_dpu);
// 			// if (dpuPosIndex > 0)
// 			// {
// 			// 	json j_dpu_edge;
// 			// 	j_dpu_edge["src_id"] = first_dpu_id;
// 			// 	j_dpu_edge["src_id"] = j_dpu["id"];
// 			// 	j_dpu_edge["link_type"] = 1;
// 			// 	j_dpu_edge["distance"] = 0;
// 			// 	j_["edge"].push_back(j_dpu_edge);
// 			// }
// 			// for (auto &id : input_refis)
// 			// {
// 			// 	json j_dpu_edge;
// 			// 	j_dpu_edge["src_id"] = id;
// 			// 	j_dpu_edge["dest_id"] = j_dpu["id"];
// 			// 	j_dpu_edge["link_type"] = 1;
// 			// 	j_dpu_edge["distance"] = 0;
// 			// 	j_["edge"].push_back(j_dpu_edge);
// 			// }

// 			// Descriptor interface
// 			schedule::Operation o_dpu(d_.gen_signature(path_), 0, {{to_string(dpuCoord.row) + "_" + to_string(dpuCoord.column) + "_SEQ", {{0, 0}}}, {to_string(dpuCoord.row) + "_" + to_string(dpuCoord.column) + "_DPU", {{0, dpuInst->repetition + dpuInst->executionCycle}}}});
// 			d_.add_operation(o_dpu);
// 			d_.get_mutable_operation(path_).add_child(o_dpu.name(), child_group_);
// 			curr_dpu_name = o_dpu.name();
// 			dpuInst->labels.push_back(o_dpu.name());
// 			if (first_dpu_name == "")
// 			{
// 				first_dpu_name = o_dpu.name();
// 			}
// 			if (first_dpu_name != o_dpu.name())
// 			{
// 				d_.add_constraint(Constraint(first_dpu_name, o_dpu.name(), 0, 0));
// 			}
// 			for (auto &name : input_refis_name)
// 			{
// 				d_.add_constraint(Constraint(name, o_dpu.name(), 0, 0));
// 			}

// 			////////////////////////////////////////////////////////////////
// 			// DEBUG new schedule graph
// 			////////////////////////////////////////////////////////////////

// 			if (dpuInst->mode == "silago_dpu_add_with_constant" || dpuInst->mode == "silago_dpu_load_constant")
// 			{
// 				int value;

// 				if (dpuInst->mode == "silago_dpu_add_with_constant")
// 				{
// 					BinaryExpression *binaryExpr =
// 							dynamic_cast<BinaryExpression *>(assignStatement_->rhs());

// 					if (binaryExpr)
// 						value = getDpuConstantValue(binaryExpr->rightOperand());
// 					else
// 						LOG(FATAL) << (errMessage);
// 				}
// 				else // dpuInst->mode == "silago_dpu_load_constant"
// 					value = getDpuConstantValue(assignStatement_->rhs());

// 				dpuInst->constantValue = value;
// 			}
// 			else if (dpuInst->mode == "silago_dpu_butterfly")
// 			{
// 				int value;
// 				BIR::BIREnumerations::DpuParameterType dpu_parameter_type =
// 						BIR::BIREnumerations::dptReference;
// 				PrimitiveFunctionCall *function_call =
// 						dynamic_cast<PrimitiveFunctionCall *>(assignStatement_->rhs());
// 				if (!function_call)
// 				{
// 					LOG(FATAL) << (errMessage);
// 				}
// 				if (function_call->_parameters[4]->kind() == VIR::VirEnumerations::ktIdentifier)
// 				{
// 					Identifier *sn =
// 							dynamic_cast<Identifier *>(function_call->_parameters[4]);
// 					if (sn->object() && sn->object()->objectType() == VIR::VirEnumerations::otRaccuVariable)
// 					{
// 						value = getDpuParameterReference(sn, dpuCoord);
// 						dpu_parameter_type = BIR::BIREnumerations::dptReference;
// 					}
// 					else
// 					{
// 						LOG(FATAL) << "Dpu parameter field can only be immediate constant or RACCU variable.";
// 					}
// 				}
// 				else
// 				{
// 					value = getDpuParameterImmediate(function_call->_parameters[4]);
// 					dpu_parameter_type = BIR::BIREnumerations::dptImmediate;
// 				}

// 				dpuInst->constantValue = value;
// 				dpuInst->parameter_type = dpu_parameter_type;
// 			}

// 			if (dpuInst->executionCycle < 0)
// 			{
// 				dpuInst->executionCycle = dpuInst->repetition;
// 			}
// 		}

// 		// Descriptor interface
// 		if (conditions(2, assignStatement_))
// 		{
// 			for (auto &mappedRefiInst : mappedRefiInsts)
// 			{
// 				if (((RefiInstruction *)mappedRefiInst.instruction)->portMode == pmOut)
// 				{
// 					int portno = -1;
// 					string res_name = "";
// 					RefiInstruction *refi_instr = (BIR::RefiInstruction *)(mappedRefiInst.instruction);
// 					int num_of_addr = refi_instr->numberOfAddress.value + 1;
// 					int exe_cycle = (num_of_addr + (num_of_addr - 1) * refi_instr->middleDelay.value) + ((num_of_addr + (num_of_addr - 1) * refi_instr->middleDelay.value) - 1) * refi_instr->repetitionDelay.value;
// 					if (resources[to_string(mappedRefiInst.cellCoord.row) + "_" + to_string(mappedRefiInst.cellCoord.column) + "_AGU1"].empty())
// 					{
// 						res_name = to_string(mappedRefiInst.cellCoord.row) + "_" + to_string(mappedRefiInst.cellCoord.column) + "_AGU1";
// 						resources[to_string(mappedRefiInst.cellCoord.row) + "_" + to_string(mappedRefiInst.cellCoord.column) + "_AGU1"] = 1;
// 						static_cast<BIR::RefiInstruction *>(mappedRefiInst.instruction)->portNo = 1;
// 						portno = 1;
// 					}
// 					else
// 					{
// 						res_name = to_string(mappedRefiInst.cellCoord.row) + "_" + to_string(mappedRefiInst.cellCoord.column) + "_AGU0";
// 						resources[to_string(mappedRefiInst.cellCoord.row) + "_" + to_string(mappedRefiInst.cellCoord.column) + "_AGU0"] = 1;
// 						static_cast<BIR::RefiInstruction *>(mappedRefiInst.instruction)->portNo = 0;
// 						portno = 0;
// 					}
// 					schedule::Operation o_refi0(d_.gen_signature(path_), 0, {{to_string(mappedRefiInst.cellCoord.row) + "_" + to_string(mappedRefiInst.cellCoord.column) + "_SEQ", {{0, 0}}}, {res_name, {{0, INT_MAX}}}});
// 					d_.add_operation(o_refi0);
// 					d_.get_mutable_operation(path_).add_child(o_refi0.name(), child_group_);
// 					schedule::Operation o_refi1(d_.gen_signature(path_), 0, {});
// 					d_.add_operation(o_refi1);
// 					d_.get_mutable_operation(path_).add_child(o_refi1.name(), child_group_);
// 					schedule::Operation o_refi2(d_.gen_signature(path_), 0, {{res_name, {{INT_MIN, 0}}}});
// 					d_.add_operation(o_refi2);
// 					d_.get_mutable_operation(path_).add_child(o_refi2.name(), child_group_);
// 					d_.add_constraint(Constraint(o_refi0.name(), o_refi1.name(), 0, INT_MAX));
// 					d_.add_constraint(Constraint(o_refi1.name(), o_refi2.name(), exe_cycle - 1, exe_cycle - 1));
// 					int distance = 0;
// 					if (dpuInst->mode == "silago_dpu_sym_mac" || dpuInst->mode == "silago_dpu_mac" || dpuInst->mode == "silago_dpu_preserving_mac" || dpuInst->mode == "silago_dpu_max" || dpuInst->mode == "silago_dpu_min" || dpuInst->mode == "silago_dpu_abs_sub_acc" || dpuInst->mode == "silago_dpu_sub_acc" || _primitive_func_lib.get_reductive(dpuInst->mode))
// 					{
// 						// FIXME: Check the +1 is required or not!

// 						distance = dpuInst->executionCycle + exe_cycle + 1;
// 					}
// 					else
// 					{
// 						distance = dpuInst->executionCycle + 1;
// 					}
// 					d_.add_constraint(Constraint(curr_dpu_name, o_refi1.name(), distance, distance));
// 					refi_instr->labels.push_back(o_refi0.name());
// 					refi_instr->labels.push_back(o_refi1.name());
// 					refi_instr->labels.push_back(o_refi2.name());

// 					for (auto &swb_instr : ((BIR::RefiInstruction *)(mappedRefiInst.instruction))->correspondingSWBInsts)
// 					{
// 						swb_instr->destination.portNo = portno;
// 						schedule::Operation o_swb0(d_.gen_signature(path_), 0, {{to_string(swb_instr->destination.cellCoordinate.row) + "_" + to_string(swb_instr->destination.cellCoordinate.column) + "_SEQ", {{0, 0}}}, {swb_instr->source.to_string() + "_" + swb_instr->destination.to_string(), {{0, INT_MAX}}}});
// 						d_.add_operation(o_swb0);
// 						d_.get_mutable_operation(path_).add_child(o_swb0.name(), child_group_);
// 						schedule::Operation o_swb2(d_.gen_signature(path_), 0, {{swb_instr->source.to_string() + "_" + swb_instr->destination.to_string(), {{INT_MIN, 0}}}});
// 						d_.add_operation(o_swb2);
// 						d_.get_mutable_operation(path_).add_child(o_swb2.name(), child_group_);
// 						d_.add_constraint(Constraint(o_swb0.name(), o_swb2.name(), 1, INT_MAX));
// 						d_.add_constraint(Constraint(o_refi2.name(), o_swb2.name(), 0, INT_MAX));
// 						d_.add_constraint(Constraint(o_swb0.name(), o_refi1.name(), 1, INT_MAX));
// 						swb_instr->labels.push_back(o_swb0.name());
// 						swb_instr->labels.push_back(o_swb2.name());
// 					}

// 					if (((BIR::RefiInstruction *)(mappedRefiInst.instruction))->dependentTo)
// 					{
// 						string p = ((BIR::RefiInstruction *)(mappedRefiInst.instruction))->dependentTo->labels[1];
// 						d_.add_constraint(Constraint(p, o_refi2.name(), 1, INT_MAX));
// 					}

// 					_drraCluster->getDRRACell(mappedRefiInst.cellCoord)->instructions().addInstruction(mappedRefiInst.instruction, true);
// 				}
// 			}
// 		}
// 	}
// 	//	}
// }

// // This method creates the corresponding instructions for a variable (a slice name) inside an assignment statement
// // Note that before sending to this method, all variables should be converted to slice names.
// // The 3rd argument is used to determine the location of the DPU cell.
// // The 4th argument is the statement number.
// // The 5th argument is a signature for the unrolled statement number.
// // The 6th argument is a vector that is updated by this method and contains all REFI instructions corresponding to the assignment statement.
// // The 7th argument is the corresponding DPU instruction. It is set to null for register/memory transfer operations. It is used for setting the DPU corresponding SWB/REFI instructions.
// // The 8th argument is the position index of the current cell coordinate in the dpu pragma.
// // The 9th argument determines if the variable is an input or output variable.
// // The 10th argument is only used for input variables and it determines the order number of the operand in the operation.
// // The 11th argument is only used when there are twin statements and it gets the statement number of the other twin statement.
// // The 12th argument is only used to determine whether the instructions are created for a memory transfer or not.
// void BEngine::createInstructionsForAVariable(VIR::SliceName *sliceName_,
// 																						 LoopValuesMap *loopValues_, VIR::ReferencePragma *referencePragma_,
// 																						 int statementNo_, std::string unrolledStatementNo_,
// 																						 vector<MappedInstruction> &mappedRefiInsts_,
// 																						 BIR::DPUInstruction *dpuInst_, int dpuPosIndex_, bool isInput_,
// 																						 int operandOrderNo_, int twinStatementNo_, bool isForMemoryTransfer_,
// 																						 int reverse_bits_, bool fft)
// {
// 	// Here we only consider variables with register file pragma
// 	StoragePragma *storagePragma =
// 			dynamic_cast<StoragePragma *>(sliceName_->prefix()->object()->pragma());

// 	IndexInfo *indexInfo = sliceName_->indexInfo();

// 	if (storagePragma && storagePragma->storageType() == stRegFile)
// 	{
// 		string loopStr;
// 		for (auto &element : *loopValues_)
// 			loopStr +=
// 					element.first->degree() > 2 ? "_" + to_string(element.second) : "";

// 		const Coordinate &refCoord =
// 				referencePragma_->coordinates[dpuPosIndex_];

// 		const string prefixName = sliceName_->prefix()->name();

// 		const string variableName = prefixName + portNameSignature(isInput_, operandOrderNo_) + to_string(statementNo_) + "_" + getDpuCellSignature(refCoord) + loopStr;

// 		const int regFilePosIndex = getStoragePositionIndex(sliceName_,
// 																												dpuPosIndex_);

// 		Coordinate regFileCoord = storagePragma->coordinates[regFilePosIndex];

// 		ComponentInfo srcComponentInfo;
// 		ComponentInfo dstComponentInfo;

// 		SWBInstruction *swbInst = 0;

// 		// case of arithmetic statement and inputs of a dpu chain
// 		if (referencePragma_->kind() == ktDPUPragma || referencePragma_->kind() == ktDPUChainPragma)
// 		{
// 			Coordinate coord = refCoord;
// 			ComponentInfo dpuComponentInfo;
// 			ComponentInfo regComponentInfo;
// 			if (!isInput_)
// 			{
// 				int offset = getDpuPortNumbers(
// 						((DPUPragma *)referencePragma_)->dpuMode, isInput_,
// 						operandOrderNo_,
// 						((DPUPragma *)referencePragma_)->outPort);
// 				if (offset > 0)
// 				{
// 					offset = offset / 2;
// 				}
// 				else
// 				{
// 					offset = 0;
// 				}
// 				coord.row += offset;
// 				dpuComponentInfo = ComponentInfo(ctDPU,
// 																				 getDpuPortNumbers(
// 																						 ((DPUPragma *)referencePragma_)->dpuMode,
// 																						 isInput_, operandOrderNo_,
// 																						 ((DPUPragma *)referencePragma_)->outPort) %
// 																						 2,
// 																				 coord, statementNo_);
// 				regComponentInfo = ComponentInfo(ctRegFile, 0, regFileCoord,
// 																				 statementNo_);
// 			}
// 			else
// 			{
// 				dpuComponentInfo = ComponentInfo(ctDPU,
// 																				 getDpuPortNumbers(
// 																						 ((DPUPragma *)referencePragma_)->dpuMode,
// 																						 isInput_, operandOrderNo_,
// 																						 ((DPUPragma *)referencePragma_)->outPort),
// 																				 coord, statementNo_);
// 				regComponentInfo = ComponentInfo(ctRegFile, 0, regFileCoord,
// 																				 statementNo_);
// 			}

// 			swbInst = createSwbInstruction(statementNo_, unrolledStatementNo_,
// 																		 variableName, loopValues_, dpuInst_->correspondingSWBInsts,
// 																		 coord, isInput_ ? regComponentInfo : dpuComponentInfo,
// 																		 isInput_ ? dpuComponentInfo : regComponentInfo);
// 		}
// 		// case of register transfer statement
// 		else if (referencePragma_->kind() == ktStoragePragma && isInput_ && !isForMemoryTransfer_)
// 		{
// 			// Register transfer only needs one SWB instruction in the cell of the output variable.
// 			// This instruction gets the input variable as its source and the output variable as its destination.

// 			srcComponentInfo = ComponentInfo(ctRegFile, 0, regFileCoord,
// 																			 statementNo_);
// 			dstComponentInfo = ComponentInfo(ctRegFile, 0, refCoord,
// 																			 statementNo_);

// 			//! Yu Yang 2017-08-01
// 			// Change to solve compile error caused by passing reference of object in stack
// 			// CHANGE BEGIN
// 			vector<BIR::SWBInstruction *> v;
// 			swbInst = createSwbInstruction(statementNo_, unrolledStatementNo_,
// 																		 variableName, loopValues_, v, refCoord, srcComponentInfo,
// 																		 dstComponentInfo);
// 			// CHANGE END
// 		}

// 		string resSharingSignature;

// 		if (_isResourceSharingRegion)
// 			resSharingSignature = getResSharingVariableSignature(sliceName_,
// 																													 regFileCoord);

// 		if (swbInst)
// 			swbInst->twinStatementNo = twinStatementNo_;

// 		// --------------------------------------------------------
// 		// Create REFI instruction,
// 		// only if it is for an output variable, or we are not in a resource sharing region,
// 		// or we are within a resource sharing region and the REFI instruction has not been created already.
// 		if (!isInput_ || !_isResourceSharingRegion || _resourceSharingVariableMaps.back().find(resSharingSignature) == _resourceSharingVariableMaps.back().end())
// 		{
// 			int refiStartAddress, refiNumberOfAddress, stepValue,
// 					numberOfRepetition, repetitionOffset;

// 			if (_isNonConstantLoopRegion)
// 			{
// 				UnrolledAddressInfo unrolledAddressInfo =
// 						indexInfo->unrolledAddressMap[getUnrolledLoopValueSignature(
// 								loopValues_)];

// 				refiStartAddress = unrolledAddressInfo.startAddress;
// 				refiNumberOfAddress = unrolledAddressInfo.numberOfAddress;
// 				stepValue = unrolledAddressInfo.increment;
// 				numberOfRepetition = unrolledAddressInfo.numberOfRepetitions;
// 				repetitionOffset = unrolledAddressInfo.repetitionOffset;
// 			}
// 			else
// 			{
// 				refiStartAddress = indexInfo->addressInfo.startAddress;
// 				refiNumberOfAddress = indexInfo->addressInfo.numberOfAddress;
// 				stepValue = indexInfo->addressInfo.increment;
// 				numberOfRepetition =
// 						!indexInfo->addressInfo.is2ndLevelDynamic && !indexInfo->isARangeExpression ? indexInfo->addressInfo.numberOfRepetitions : 0;
// 				repetitionOffset = indexInfo->addressInfo.repetitionOffset;
// 			}
// 			int reverse_bits = 0;
// 			bool is_reverse_bits_static = true;
// 			if (sliceName_->reverse_bits > 0)
// 			{
// 				getRaccuOperand(sliceName_->reverse_bits, regFileCoord,
// 												reverse_bits, is_reverse_bits_static, false);
// 			}
// 			BIR::BIREnumerations::AddressMode mode = amLinear;
// 			if (fft) // FFT mode, each input only feed to one port of butterfly, so the number of address should be half. Pay attention the operand order is i+1.
// 			{
// 				if (operandOrderNo_ == 1 || operandOrderNo_ == 2)
// 				{
// 					//refiStartAddress = refiStartAddress;
// 					refiNumberOfAddress = refiNumberOfAddress / 2;
// 					stepValue = stepValue * 2;
// 				}
// 				else if (operandOrderNo_ == 3 || operandOrderNo_ == 4)
// 				{
// 					refiStartAddress = refiStartAddress + 1;
// 					refiNumberOfAddress = refiNumberOfAddress / 2;
// 					stepValue = stepValue * 2;
// 				}
// 			}

// 			// If it is possible to divide data between different cells
// 			if (referencePragma_->parallelismSize() <= storagePragma->parallelismSize())
// 				refiNumberOfAddress = (refiNumberOfAddress - (referencePragma_->parallelismSize() - 1)) / referencePragma_->parallelismSize();

// 			// For memory transfer, the number of addresses should be divided by the memory block size
// 			if (isForMemoryTransfer_)
// 				refiNumberOfAddress =
// 						((refiNumberOfAddress + 1) / cMemWordSplit) - 1;

// 			RefiInstruction *refiInst = createRefiInstruction(statementNo_,
// 																												twinStatementNo_, unrolledStatementNo_, variableName, mode,
// 																												isInput_ ? pmIn : pmOut, refiNumberOfAddress, stepValue,
// 																												numberOfRepetition, repetitionOffset, reverse_bits,
// 																												is_reverse_bits_static, isForMemoryTransfer_,
// 																												_isResourceSharingRegion, dpuInst_,
// 																												sliceName_->prefix()->name());

// 			if (swbInst)
// 				refiInst->correspondingSWBInsts.push_back(swbInst);

// 			if (dpuInst_)
// 				dpuInst_->correspondingRefiInsts.push_back(refiInst);

// 			// Adding the REFI instruction to a vector, in order to add it to the instruction list later.
// 			mappedRefiInsts_.push_back(
// 					MappedInstruction(regFileCoord, refiInst));

// 			addInstructionToMap(refiInst, statementNo_, loopValues_, refCoord,
// 													regFileCoord);

// 			if (_isResourceSharingRegion && isInput_)
// 				_resourceSharingVariableMaps.back()[resSharingSignature] =
// 						refiInst;

// 			// -------------------------------------------------------------------------------------------------
// 			// Calculating the start address

// 			const int posIndexOfstartAddress = storagePragma->getPositionIndex(
// 					indexInfo->addressInfo.startAddress);
// 			const int maxPrevIndex =
// 					posIndexOfstartAddress == 0 ? 0 : storagePragma->maxIndexes[posIndexOfstartAddress - 1];

// 			// Subtracting 1 from the start address is because MATLAB indices are started from 1, however RegFile indexing is started from 0.
// 			refiStartAddress = refiStartAddress - maxPrevIndex + storagePragma->startAddresses[regFilePosIndex] - 1;

// 			bool isStartAddressDynamic = false;

// 			if (!_raccuMode && !_isNonConstantLoopRegion)
// 			{
// 				for (auto &element : indexInfo->unrollingMap())
// 				{
// 					const int loopBegin =
// 							((Integer *)element.first->loopRange()->begin())->value;
// 					const int loopIncrement =
// 							((Integer *)element.first->loopRange()->increment())->value;
// 					refiStartAddress += element.second->unrollingOffset * (loopValues_->at(element.first) - loopBegin) / loopIncrement;
// 				}
// 			}
// 			// Dynamic calculations
// 			else if (!_isNonConstantLoopRegion)
// 			{
// 				const string indexVariableName =
// 						sliceName_->suffix()[0]->kind() == ktIdentifier ? ((Identifier *)sliceName_->suffix()[0])->name() : "";

// 				isStartAddressDynamic = calculateDynamicStartAddress(
// 						refiStartAddress, indexInfo, statementNo_, refiInst,
// 						loopValues_, refCoord, regFileCoord, indexVariableName,
// 						isForMemoryTransfer_);
// 			}

// 			refiInst->startAddress(refiStartAddress, !isStartAddressDynamic);
// 		}
// 		else if (_isResourceSharingRegion)
// 		{
// 			RefiInstruction *refiInst =
// 					_resourceSharingVariableMaps.back()[resSharingSignature];
// 			refiInst->correspondingSWBInsts.push_back(swbInst);
// 			if (dpuInst_)
// 				dpuInst_->correspondingRefiInsts.push_back(refiInst);
// 		}

// 		DRRACell *dpuCell = _drraCluster->getDRRACell(refCoord);
// 		DRRACell *refiCell = _drraCluster->getDRRACell(regFileCoord);
// 	}
// 	// This case should only happen for input operands corresponding to a DPU chain.
// 	else if (!conditions(5, sliceName_) && isInput_)
// 	{
// 		DPUChainPragma *dpuChainPragma =
// 				(DPUChainPragma *)sliceName_->prefix()->object()->pragma();

// 		string loopStr;
// 		for (auto &element : *loopValues_)
// 			loopStr +=
// 					element.first->degree() > 2 ? "_" + to_string(element.second) : "";

// 		const Coordinate &refCoord =
// 				referencePragma_->coordinates[dpuPosIndex_];

// 		const string variableName = sliceName_->prefix()->name() + portNameSignature(isInput_, operandOrderNo_) + to_string(statementNo_) + "_" + getDpuCellSignature(refCoord) + loopStr;

// 		Coordinate dpuChainCoord = dpuChainPragma->coordinates[dpuPosIndex_];

// 		ComponentInfo srcComponentInfo;
// 		ComponentInfo dstComponentInfo;

// 		// Find the corresponding chained dpu
// 		DPUInstruction *chainedDpuInst = _dpuChainMap[getSliceNameSignature(
// 				sliceName_)];

// 		// Add the found chained DPU instruction to the list of corresponding chained DPUs
// 		dpuInst_->correspondingChainedDpus.push_back(chainedDpuInst);

// 		srcComponentInfo = ComponentInfo(ctDPU,
// 																		 getDpuPortNumbers(dpuChainPragma->dpuMode, false, -1,
// 																											 dpuChainPragma->outPort),
// 																		 dpuChainCoord,
// 																		 chainedDpuInst->statementNo);

// 		dstComponentInfo = ComponentInfo(ctDPU,
// 																		 getDpuPortNumbers(((DPUPragma *)referencePragma_)->dpuMode,
// 																											 true, operandOrderNo_,
// 																											 ((DPUPragma *)referencePragma_)->outPort),
// 																		 refCoord,
// 																		 statementNo_);

// 		SWBInstruction *swbInst = createSwbInstruction(statementNo_,
// 																									 unrolledStatementNo_, variableName, loopValues_,
// 																									 dpuInst_->correspondingSWBInsts, refCoord, srcComponentInfo,
// 																									 dstComponentInfo);

// 		swbInst->twinStatementNo = twinStatementNo_;
// 	}
// 	else
// 		LOG(FATAL) << ("createInstructionsForAVariable: This pragma is not supported!");
// }

// // This method creates the corresponding instructions for the condition of an if statement
// void BEngine::createInstructionsForIfCondition(IfStatement *ifStatement_, LoopValuesMap *loopValues_, schedule::Descriptor &d_, json &j_, string path_, int child_group_)
// {
// 	if (!isAValidIfCondition(ifStatement_->condition()))
// 	{
// 		return;
// 	}

// 	BinaryExpression *binaryExpression = dynamic_cast<BinaryExpression *>(ifStatement_->condition());
// 	DPUPragma *dpuPragma = ifStatement_->conditionPragma();
// 	vector<MappedInstruction> mappedRefiInsts_all;
// 	vector<DPUInstruction *> dpu_inst_all;

// 	int id_counter = 0;
// 	int first_dpu_id = -1;
// 	vector<int> refi_instr_id_list;
// 	vector<int> refi_instr_port_list;
// 	map<string, int> resource;
// 	for (int i = 0; i < dpuPragma->coordinates.size(); i++)
// 	{
// 		DPUInstruction *dpuInst = CREATE_OBJECT_B(DPUInstruction);
// 		dpu_inst_all.push_back(dpuInst);
// 		vector<MappedInstruction> mappedRefiInsts;
// 		const Coordinate dpuCoord = dpuPragma->coordinates[i];
// 		string unrolledStatementNo = getUnrolledStatementSignature2(
// 				ifStatement_->statementNo(), loopValues_, dpuCoord);
// 		createInstructionsForAVariable(
// 				(SliceName *)binaryExpression->leftOperand(), loopValues_,
// 				dpuPragma, ifStatement_->statementNo(), unrolledStatementNo,
// 				mappedRefiInsts, dpuInst, i, true, 1);
// 		if (binaryExpression->rightOperand()->kind() == ktSliceName)
// 		{
// 			createInstructionsForAVariable(
// 					(SliceName *)binaryExpression->rightOperand(), loopValues_,
// 					dpuPragma, ifStatement_->statementNo(), unrolledStatementNo,
// 					mappedRefiInsts, dpuInst, i, true, 2);
// 		}
// 		if (i == 0)
// 		{
// 			mappedRefiInsts_all = mappedRefiInsts;
// 			for (auto &mappedRefiInstr : mappedRefiInsts)
// 			{
// 				Coordinate &coord = mappedRefiInstr.cellCoord;
// 				RefiInstruction *refi_instr = static_cast<RefiInstruction *>(mappedRefiInstr.instruction);

// 				int saved_id_counter = id_counter;

// 				// descriptor interface
// 				string res_name = "";
// 				if (resource.find(to_string(coord.row) + "_" + to_string(coord.column) + "_AGU2") == resource.end())
// 				{
// 					res_name = to_string(coord.row) + "_" + to_string(coord.column) + "_AGU2";
// 					refi_instr->portNo = 2;
// 					refi_instr_port_list.push_back(2);
// 					resource[to_string(coord.row) + "_" + to_string(coord.column) + "_AGU2"] = 1;
// 				}
// 				else
// 				{
// 					res_name = to_string(coord.row) + "_" + to_string(coord.column) + "_AGU3";
// 					refi_instr->portNo = 3;
// 					refi_instr_port_list.push_back(3);
// 					resource[to_string(coord.row) + "_" + to_string(coord.column) + "_AGU3"] = 1;
// 				}

// 				schedule::Operation o_refi0(d_.gen_signature(path_), 0, {{to_string(coord.row) + "_" + to_string(coord.column) + "_SEQ", {{0, 0}}}, {res_name, {{0, INT_MAX}}}});
// 				refi_instr->labels.push_back(o_refi0.name());
// 				d_.add_operation(o_refi0);
// 				d_.get_mutable_operation(path_).add_child(o_refi0.name(), child_group_);

// 				schedule::Operation o_refi1(d_.gen_signature(path_), 0, {});
// 				refi_instr->labels.push_back(o_refi1.name());
// 				d_.add_operation(o_refi1);
// 				d_.get_mutable_operation(path_).add_child(o_refi1.name(), child_group_);

// 				schedule::Operation o_refi2(d_.gen_signature(path_), 0, {{res_name, {{INT_MIN, 0}}}});
// 				refi_instr->labels.push_back(o_refi2.name());
// 				d_.add_operation(o_refi2);
// 				d_.get_mutable_operation(path_).add_child(o_refi2.name(), child_group_);

// 				int num_of_addr = refi_instr->numberOfAddress.value + 1;
// 				int exe_cycle = (num_of_addr +
// 												 (num_of_addr - 1) * refi_instr->middleDelay.value) +
// 												((num_of_addr + (num_of_addr - 1) * refi_instr->middleDelay.value) - 1) * refi_instr->repetitionDelay.value;
// 				d_.add_constraint(Constraint(o_refi0.name(), o_refi1.name(), 0, INT_MAX));
// 				d_.add_constraint(Constraint(o_refi1.name(), o_refi2.name(), exe_cycle - 1, exe_cycle - 1));
// 			}
// 		}

// 		int x = 0;
// 		for (auto &mappedRefiInstr : mappedRefiInsts)
// 		{
// 			Coordinate &coord = mappedRefiInstr.cellCoord;
// 			RefiInstruction *refi_instr = static_cast<RefiInstruction *>(mappedRefiInstr.instruction);
// 			for (auto &swb_instr : refi_instr->correspondingSWBInsts)
// 			{
// 				// Deprecated: json interface
// 				// int saved_id_counter = id_counter;
// 				// json j_swb;
// 				// j_swb["id"] = id_counter;
// 				// j_swb["name"] = "mapped refi swb: " + path_ + to_string(id_counter);
// 				// swb_instr->labels.push_back(j_swb["name"]);
// 				// swb_instr->source.portNo =
// 				// 		static_cast<RefiInstruction *>(mappedRefiInsts_all[x].instruction)->portNo;
// 				// j_swb["resource"][to_string(dpuCoord.row) + "_" + to_string(dpuCoord.column) + "_SEQ"].push_back(0);
// 				// j_swb["resource"][to_string(dpuCoord.row) + "_" + to_string(dpuCoord.column) + "_SEQ"].push_back(0);
// 				// j_swb["node_type"] = 2;
// 				// j_["vertex"].push_back(j_swb);
// 				// id_counter++;

// 				// json j_edge_swb;
// 				// j_edge_swb["src_id"] = j_swb["id"];
// 				// j_edge_swb["dest_id"] = refi_instr_id_list[x];
// 				// j_edge_swb["link_type"] = 0;
// 				// j_edge_swb["distance"] = 0;
// 				// j_["edge"].push_back(j_edge_swb);

// 				// descriptor interface
// 				schedule::Operation o_swb0(d_.gen_signature(path_), 0, {{to_string(dpuCoord.row) + "_" + to_string(dpuCoord.column) + "_SEQ", {{0, 0}}}, {swb_instr->source.to_string() + "_" + swb_instr->destination.to_string(), {{0, INT_MAX}}}});
// 				d_.add_operation(o_swb0);
// 				schedule::Operation o_swb2(d_.gen_signature(path_), 0, {{swb_instr->source.to_string() + "_" + swb_instr->destination.to_string(), {{INT_MIN, 0}}}});
// 				d_.add_operation(o_swb2);
// 				d_.get_mutable_operation(path_).add_children({o_swb0.name(), o_swb2.name()}, child_group_);
// 				d_.add_constraint(Constraint(o_swb0.name(), o_swb2.name(), 1, INT_MAX));
// 				d_.add_constraint(Constraint(refi_instr->labels[2], o_swb2.name(), 0, INT_MAX));

// 				swb_instr->source.portNo = refi_instr_port_list[x];
// 			}
// 			x++;
// 		}
// 	}

// 	for (auto &iter : mappedRefiInsts_all)
// 	{
// 		_drraCluster->getDRRACell(iter.cellCoord)->instructions().addInstruction(iter.instruction, true);
// 	}

// 	for (int i = 0; i < dpuPragma->coordinates.size(); i++)
// 	{
// 		DPUInstruction *dpuInst = dpu_inst_all[i];
// 		const Coordinate dpuCoord = dpuPragma->coordinates[i];
// 		string unrolledStatementNo = getUnrolledStatementSignature2(
// 				ifStatement_->statementNo(), loopValues_, dpuCoord);
// 		// Create DPU instruction
// 		createDpuInstruction(dpuInst, ifStatement_->statementNo(),
// 												 unrolledStatementNo, dpuPragma, loopValues_, dpuCoord, 0,
// 												 false);

// 		// Set the constant value in case of comparison with a constant
// 		if (dpuInst->mode == "silago_dpu_comparison_with_constant")
// 			dpuInst->constantValue = getDpuConstantValue(
// 					binaryExpression->rightOperand());

// 		if (i > 0)
// 		{
// 			dpuInst->correspondingRefiInsts.clear();
// 			for (auto &iter : mappedRefiInsts_all)
// 			{
// 				dpuInst->correspondingRefiInsts.push_back(
// 						(RefiInstruction *)iter.instruction);
// 			}
// 		}

// 		// Deprecated: json interface
// 		// int saved_id_counter = id_counter;
// 		// json j_dpu;
// 		// j_dpu["id"] = id_counter;
// 		// j_dpu["name"] = "dpu:" + path_ + to_string(id_counter);
// 		// dpuInst->labels.push_back(j_dpu["name"]);
// 		// j_dpu["resource"][to_string(dpuCoord.row) + "_" + to_string(dpuCoord.column) + "_SEQ"].push_back(0);
// 		// j_dpu["resource"][to_string(dpuCoord.row) + "_" + to_string(dpuCoord.column) + "_SEQ"].push_back(0);
// 		// j_dpu["resource"][to_string(dpuCoord.row) + "_" + to_string(dpuCoord.column) + "_DPU"].push_back(0);
// 		// j_dpu["resource"][to_string(dpuCoord.row) + "_" + to_string(dpuCoord.column) + "_DPU"].push_back(
// 		// 		dpuInst->repetition + dpuInst->executionCycle);
// 		// j_dpu["node_type"] = 2;
// 		// j_["vertex"].push_back(j_dpu);
// 		// id_counter++;

// 		// descriptor interface
// 		schedule::Operation o_dpu(d_.gen_signature(path_), 0, {{to_string(dpuCoord.row) + "_" + to_string(dpuCoord.column) + "_SEQ", {{0, 0}}}, {to_string(dpuCoord.row) + "_" + to_string(dpuCoord.column) + "_DPU", {{0, dpuInst->repetition + dpuInst->executionCycle}}}});
// 		dpuInst->labels.push_back(o_dpu.name());
// 		dpuInst->labels.push_back(o_dpu.name());
// 		dpuInst->labels.push_back(o_dpu.name());
// 		d_.add_operation(o_dpu);
// 		d_.get_mutable_operation(path_).add_child(o_dpu.name(), child_group_);

// 		// Deprecated: json interface
// 		// for (auto &id : refi_instr_id_list)
// 		// {
// 		// 	json j_edge_refi_dpu;
// 		// 	j_edge_refi_dpu["src_id"] = id;
// 		// 	j_edge_refi_dpu["dest_id"] = j_dpu["id"];
// 		// 	j_edge_refi_dpu["link_type"] = 1;
// 		// 	j_edge_refi_dpu["distance"] = 0;
// 		// 	j_["edge"].push_back(j_edge_refi_dpu);
// 		// }

// 		// descriptor interface
// 		for (auto &mapped_instr : mappedRefiInsts_all)
// 		{
// 			RefiInstruction *refi_instr = static_cast<RefiInstruction *>(mapped_instr.instruction);
// 			d_.add_constraint(Constraint(refi_instr->labels[1], o_dpu.name(), 0, 0));
// 		}

// 		// Create BRANCH instruction
// 		BranchInstruction *branch_inst = createBranchInstruction(
// 				binaryExpression, ifStatement_->statementNo(),
// 				unrolledStatementNo, dpuCoord, loopValues_);
// 		_branch_manager.set_curr_inst_branch(dpuCoord, branch_inst);

// 		// descriptor interface
// 		schedule::Operation o_brn(d_.gen_signature(path_), 0, {{to_string(dpuCoord.row) + "_" + to_string(dpuCoord.column) + "_SEQ", {{0, 0}}}});
// 		d_.add_operation(o_brn);
// 		d_.get_mutable_operation(path_).add_child(o_brn.name(), child_group_);
// 		d_.add_constraint(Constraint(o_dpu.name(), o_brn.name(), 1, 1));
// 	}
// }

// Deprecated: indirect addressing is not supportted currently
// This method creates instructions for indirect addressing.
// void BEngine::createInstructionsForIndirectAddress(VIR::SliceName *sliceName_,
// 																									 LoopValuesMap *loopValues_, VIR::ReferencePragma *referencePragma_,
// 																									 int statementNo_, std::string unrolledStatementNo_,
// 																									 vector<MappedInstruction> &mappedRefiInsts_)
// {
// 	// Here we only consider variables with register file pragma
// 	StoragePragma *storagePragma =
// 			dynamic_cast<StoragePragma *>(sliceName_->prefix()->object()->pragma());

// 	if (!storagePragma || storagePragma->storageType() != stRegFile)
// 		LOG(FATAL) << ("createInstructionsForIndirectAddress: This pragma is not supported!");

// 	string loopStr;
// 	for (auto &element : *loopValues_)
// 		loopStr +=
// 				element.first->degree() > 2 ? "_" + to_string(element.second) : "";
// }

// This method creates instructions for performing a memory transfer to/from register files.
// void BEngine::createInstructionsForMemoryTransfer(VIR::AssignmentStatement *assignStatement_, LoopValuesMap *loopValues_, schedule::Descriptor &d_, json &j_, string path_, int child_group_)
// {
// 	StoragePragma *lhsPragma =
// 			(StoragePragma *)assignStatement_->lhsIdentifiers()[0]->object()->pragma();
// 	StoragePragma *rhsPragma =
// 			(StoragePragma *)((assignStatement_->rhsIdentifiers())[0]->object()->pragma());

// 	StoragePragma *memoryPragma;
// 	StoragePragma *regFilePragma;
// 	IndexInfo *memoryIndexInfo;
// 	IndexInfo *regFileIndexInfo;
// 	bool isMemToRegTrans;

// 	SliceName *lhsSliceName =
// 			dynamic_cast<SliceName *>(assignStatement_->lhs()[0]);
// 	SliceName *rhsSliceName =
// 			dynamic_cast<SliceName *>(assignStatement_->rhs());
// 	SliceName *regFileSliceName;
// 	SliceName *memorySliceName;

// 	if (!lhsSliceName || !rhsSliceName)
// 		LOG(FATAL) << ("createInstructionsForMemoryTransfer: Improper memory transfer statement!");

// 	// If it is a memory read transfer
// 	if (lhsPragma->storageType() == stRegFile && rhsPragma->storageType() == stMemory)
// 	{
// 		memoryPragma = rhsPragma;
// 		regFilePragma = lhsPragma;

// 		memoryIndexInfo = rhsSliceName->indexInfo();
// 		regFileIndexInfo = lhsSliceName->indexInfo();

// 		regFileSliceName = lhsSliceName;
// 		memorySliceName = rhsSliceName;

// 		isMemToRegTrans = true;
// 	}
// 	// It is a memory write transfer (lhsPragma->storageType() == stMemory  && rhsPragma->storageType() == stRegFile)
// 	else
// 	{
// 		memoryPragma = lhsPragma;
// 		regFilePragma = rhsPragma;

// 		memoryIndexInfo = lhsSliceName->indexInfo();
// 		regFileIndexInfo = rhsSliceName->indexInfo();

// 		regFileSliceName = rhsSliceName;
// 		memorySliceName = lhsSliceName;

// 		isMemToRegTrans = false;
// 	}

// 	// TODO: Currently we consider that memory pragma does not have any range.
// 	if (memoryPragma->parallelismSize() > 1)
// 		LOG(FATAL) << ("createInstructionsForMemoryTransfer: memory parallelism is not supported!");

// 	const Coordinate &memoryCoord = memoryPragma->coordinates[0];

// 	vector<MappedInstruction> mappedRefiInsts;

// 	int id_counter = 0;

// 	// Repeating the memory transfer operation for the number of register files assigned to a variable with RFILE pragma
// 	for (unsigned regFilePosIndex = 0;
// 			 regFilePosIndex < regFilePragma->coordinates.size();
// 			 ++regFilePosIndex)
// 	{
// 		// Here as there is no DPU, we can safely use regFilePragma coordinates instead.
// 		string unrolledStatementNo = getUnrolledStatementSignature2(
// 				assignStatement_->statementNo(), loopValues_,
// 				regFilePragma->coordinates[regFilePosIndex]);

// 		const Coordinate &regFileCoord =
// 				regFilePragma->coordinates[regFilePosIndex];

// 		string loopStr;
// 		for (auto &element : *loopValues_)
// 			loopStr +=
// 					element.first->degree() > 2 ? "_" + to_string(element.second) : "";
// 		const string variableName = memorySliceName->prefix()->name() + "_" + to_string(assignStatement_->statementNo()) + "_" + getDpuCellSignature(regFileCoord) + loopStr;

// 		RouteInstruction *routeInst = createRouteInstruction(
// 				assignStatement_->statementNo(), unrolledStatementNo,
// 				memoryCoord, regFileCoord, loopValues_, isMemToRegTrans,
// 				variableName);

// 		createRfileInstruction(regFileSliceName, loopValues_, regFilePragma,
// 													 assignStatement_->statementNo(), unrolledStatementNo,
// 													 mappedRefiInsts, regFilePosIndex, isMemToRegTrans);

// 		RefiInstruction *correspondingRefiInst = 0;

// 		// Adding all REFI instructions to the corresponding instruction list of DRRA cells
// 		// This loop should iterate only once.
// 		for (auto &mappedRefiInst : mappedRefiInsts)
// 		{
// 			correspondingRefiInst =
// 					(RefiInstruction *)mappedRefiInst.instruction;
// 			_drraCluster->getDRRACell(mappedRefiInst.cellCoord)->instructions().addInstruction(mappedRefiInst.instruction, true);
// 		}

// 		BIR::SRAMInstruction *sramInst = createSramInstruction(memorySliceName,
// 																													 assignStatement_->statementNo(), unrolledStatementNo,
// 																													 memoryCoord, regFileCoord, loopValues_, isMemToRegTrans,
// 																													 memoryIndexInfo, memoryPragma->startAddresses[0], variableName,
// 																													 correspondingRefiInst, routeInst);

// 		int hop_number = 0;
// 		int programming_time = 2;
// 		hop_number = abs(routeInst->destCoordinate.row - routeInst->sourceCoordinate.row) + abs(routeInst->destCoordinate.column - routeInst->sourceCoordinate.column);

// 		//sramInst->executionCycle = isMemToRegTrans ? programming_time + 2 * hop_number : programming_time;
// 		sramInst->hopNumber = hop_number;

// 		//descriptor interface
// 		SramAddressTransformer t(16, 16);
// 		map<string, vector<vector<int>>> rot0, rot2;
// 		rot0[to_string(regFileCoord.row) + "_" + to_string(regFileCoord.column) + "_SEQ"].push_back({0, 0});
// 		int start_r = 0;
// 		int start_c = regFileCoord.column;
// 		int end_r = memoryCoord.row;
// 		int end_c = memoryCoord.column;
// 		for (int i = start_r; i <= end_r; i++)
// 		{
// 			rot0[to_string(i) + "_" + to_string(start_c) + "_DIMARCH"].push_back({0, INT_MAX});
// 			rot2[to_string(i) + "_" + to_string(start_c) + "_DIMARCH"].push_back({INT_MIN, 0});
// 		}
// 		if (start_c < end_c)
// 		{
// 			for (int i = start_c + 1; i <= end_c; i++)
// 			{
// 				rot0[to_string(end_r) + "_" + to_string(i) + "_DIMARCH"].push_back({0, INT_MAX});
// 				rot2[to_string(end_r) + "_" + to_string(i) + "_DIMARCH"].push_back({INT_MIN, 0});
// 			}
// 		}
// 		else if (start_c > end_c)
// 		{
// 			for (int i = start_c - 1; i >= end_c; i--)
// 			{
// 				rot0[to_string(end_r) + "_" + to_string(i) + "_DIMARCH"].push_back({0, INT_MAX});
// 				rot2[to_string(end_r) + "_" + to_string(i) + "_DIMARCH"].push_back({INT_MIN, 0});
// 			}
// 		}

// 		schedule::Operation o_route0(d_.gen_signature(path_), 0, rot0);
// 		schedule::Operation o_route1(d_.gen_signature(path_), 0, {});
// 		schedule::Operation o_route2(d_.gen_signature(path_), 0, rot2);
// 		d_.add_operation(o_route0);
// 		d_.add_operation(o_route1);
// 		d_.add_operation(o_route2);
// 		d_.get_mutable_operation(path_).add_children({o_route0.name(), o_route1.name(), o_route2.name()}, child_group_);
// 		routeInst->labels.push_back(o_route0.name());
// 		routeInst->labels.push_back(o_route1.name());
// 		routeInst->labels.push_back(o_route2.name());
// 		d_.add_constraint(Constraint(o_route0.name(), o_route1.name(), t.get_sram_refi_distance(regFileCoord, memoryCoord), t.get_sram_refi_distance(regFileCoord, memoryCoord)));
// 		d_.add_constraint(Constraint(o_route1.name(), o_route2.name(), 1, INT_MAX));

// 		// descriptor interface
// 		string res_name = "";
// 		int num_of_addr = correspondingRefiInst->numberOfAddress.value + 1;
// 		int exe_cycle = (num_of_addr + (num_of_addr - 1) * correspondingRefiInst->middleDelay.value) + ((num_of_addr + (num_of_addr - 1) * correspondingRefiInst->middleDelay.value) - 1) * correspondingRefiInst->repetitionDelay.value;
// 		if (isMemToRegTrans)
// 		{
// 			res_name = to_string(regFileCoord.row) + "_" + to_string(regFileCoord.column) + "_AGU0";
// 			correspondingRefiInst->portNo = 0;
// 		}
// 		else
// 		{
// 			res_name = to_string(regFileCoord.row) + "_" + to_string(regFileCoord.column) + "_AGU2";
// 			correspondingRefiInst->portNo = 2;
// 		}
// 		schedule::Operation o_refi0(d_.gen_signature(path_), 0, {{to_string(regFileCoord.row) + "_" + to_string(regFileCoord.column) + "_SEQ", {{0, 0}}}, {res_name, {{0, INT_MAX}}}});
// 		schedule::Operation o_refi1(d_.gen_signature(path_), 0, {});
// 		schedule::Operation o_refi2(d_.gen_signature(path_), 0, {{res_name, {{INT_MIN, 0}}}});
// 		d_.add_operation(o_refi0);
// 		d_.add_operation(o_refi1);
// 		d_.add_operation(o_refi2);
// 		d_.get_mutable_operation(path_).add_children({o_refi0.name(), o_refi1.name(), o_refi2.name()}, child_group_);
// 		correspondingRefiInst->labels.push_back(o_refi0.name());
// 		correspondingRefiInst->labels.push_back(o_refi1.name());
// 		correspondingRefiInst->labels.push_back(o_refi2.name());
// 		d_.add_constraint(Constraint(o_refi0.name(), o_refi1.name(), 0, INT_MAX));
// 		d_.add_constraint(Constraint(o_refi0.name(), o_refi1.name(), exe_cycle / 16 - 1, exe_cycle / 16 - 1));
// 		d_.add_constraint(Constraint(o_route1.name(), o_refi1.name(), 1, INT_MAX));
// 		d_.add_constraint(Constraint(o_refi2.name(), o_route2.name(), 0, INT_MAX));

// 		// descriptor interface
// 		schedule::Operation o_sram0(d_.gen_signature(path_), 0, {{to_string(regFileCoord.row) + "_" + to_string(regFileCoord.column) + "_SEQ", {{0, 0}}}});
// 		schedule::Operation o_sram1(d_.gen_signature(path_), 0, {});
// 		schedule::Operation o_sram2(d_.gen_signature(path_), 0, {});
// 		schedule::Operation o_sram3(d_.gen_signature(path_), 0, {});
// 		d_.add_operation(o_sram0);
// 		d_.add_operation(o_sram1);
// 		d_.add_operation(o_sram2);
// 		d_.add_operation(o_sram3);
// 		d_.get_mutable_operation(path_).add_children({o_sram0.name(), o_sram1.name(), o_sram2.name(), o_sram3.name()}, child_group_);
// 		sramInst->labels.push_back(o_sram0.name());
// 		sramInst->labels.push_back(o_sram1.name());
// 		sramInst->labels.push_back(o_sram2.name());
// 		sramInst->labels.push_back(o_sram3.name());
// 		d_.add_constraint(Constraint(o_sram0.name(), o_sram1.name(), t.get_sram_agu_init_delay_min(regFileCoord, memoryCoord), t.get_sram_agu_init_delay_min(regFileCoord, memoryCoord)));
// 		d_.add_constraint(Constraint(o_sram1.name(), o_sram2.name(), 0, INT_MAX));
// 		d_.add_constraint(Constraint(o_sram2.name(), o_sram3.name(), sramInst->executionCycle - 1, sramInst->executionCycle - 1));
// 		if (isMemToRegTrans)
// 		{
// 			d_.add_constraint(Constraint(o_sram2.name(), o_refi1.name(), t.get_sram_refi_distance(regFileCoord, memoryCoord), t.get_sram_refi_distance(regFileCoord, memoryCoord)));
// 		}
// 		else
// 		{
// 			d_.add_constraint(Constraint(o_refi1.name(), o_sram2.name(), t.get_sram_refi_distance(regFileCoord, memoryCoord), t.get_sram_refi_distance(regFileCoord, memoryCoord)));
// 		}
// 		d_.add_constraint(Constraint(o_route1.name(), o_sram2.name(), 1, INT_MAX));
// 		d_.add_constraint(Constraint(o_sram3.name(), o_route2.name(), 0, INT_MAX));
// 	}
// }

// // This method creates a RACCU instruction for a RACCU assignment.
// void BEngine::createInstructionsForRaccuVariable(VIR::AssignmentStatement *assignStatement_, LoopValuesMap *loopValues_, schedule::Descriptor &d_, json &j_, string path_, int child_group_)
// {
// 	int operand1, operand2;
// 	bool isOp1Static, isOp2Static;
// 	RACCUMode raccuMode =
// 			((RaccuPragma *)assignStatement_->pragma())->raccuMode;

// 	bool isRaccuForMemAddress =
// 			((RaccuPragma *)assignStatement_->pragma())->raccuOptions == roMemoryAddress;

// 	CoordinateMap utilizedCells;

// 	// Finding all utilized cells corresponding to the RACCU variable
// 	findUtilizedCellsInStatement(assignStatement_->controlDependentTo(),
// 															 utilizedCells);

// 	for (auto const &element : utilizedCells)
// 	{
// 		Coordinate coordinate = element.second;

// 		switch (assignStatement_->rhs()->kind())
// 		{
// 		case ktPrimitiveFunctionCall:
// 		{
// 			vector<Expression *> &functionParameters =
// 					((PrimitiveFunctionCall *)assignStatement_->rhs())->parametersSplit();

// 			// TODO: Currently it can only have 2 parameters
// 			getRaccuOperand(functionParameters[0], coordinate, operand1,
// 											isOp1Static, isRaccuForMemAddress);
// 			getRaccuOperand(functionParameters[1], coordinate, operand2,
// 											isOp2Static, isRaccuForMemAddress);
// 		}
// 		break;

// 		case ktBinaryExpression:
// 		{
// 			BinaryExpression *binaryExpression =
// 					(BinaryExpression *)assignStatement_->rhs();

// 			getRaccuOperand(binaryExpression->leftOperand(), coordinate,
// 											operand1, isOp1Static, isRaccuForMemAddress);
// 			getRaccuOperand(binaryExpression->rightOperand(), coordinate,
// 											operand2, isOp2Static, isRaccuForMemAddress);
// 		}
// 		break;

// 		case ktInteger:
// 		{
// 			// It is related to RACCU variable initialization
// 			operand1 = ((Integer *)assignStatement_->rhs())->value;
// 			operand2 = 0;
// 			isOp1Static = true;
// 			isOp2Static = true;
// 			raccuMode = rmAdd;
// 		}
// 		break;

// 		default:
// 			LOG(FATAL) << ("createInstructionForRaccuVariable: Unsupported operation!");
// 			break;
// 		}

// 		int resultAddress = -3;

// 		RACCUInstruction *raccu_instr = createRaccuInstruction(assignStatement_->statementNo(), raccuMode,
// 																													 operand1, isOp1Static, operand2, isOp2Static, resultAddress,
// 																													 coordinate, coordinate, loopValues_, 0);

// 		// Descriptor interface
// 		Operation o(d_.gen_signature(path_), 0, {{to_string(coordinate.row) + "_" + to_string(coordinate.column) + "_SEQ", {{0, 0}}}});
// 		d_.add_operation(o);
// 		d_.get_mutable_operation(path_).add_child(o.name());
// 		raccu_instr->labels.push_back(o.name());

// 		// Adding the RACCU variable to this map
// 		_raccuVariablesAddressMap[getRaccuVariableSignature(
// 				assignStatement_->lhsIdentifiers()[0]->object()->name(),
// 				coordinate)] = resultAddress;
// 	}
// }

// // This method creates instructions for performing a transfer between register files.
// void BEngine::createInstructionsForRegisterTransfer(VIR::AssignmentStatement *assignStatement_, LoopValuesMap *loopValues_, schedule::Descriptor &d_, json &j_, string path_, int child_group_)
// {
// 	StoragePragma *storagePragma =
// 			(StoragePragma *)assignStatement_->pragma();

// 	vector<MappedInstruction> mappedRefiInsts;

// 	map<string, int> resource;

// 	int id_counter = 0;
// 	int first_src_id = -1;
// 	string first_src_name = "";
// 	for (unsigned posIndex = 0; posIndex < storagePragma->coordinates.size();
// 			 ++posIndex)
// 	{
// 		const Coordinate &dpuCoord = storagePragma->coordinates[posIndex];

// 		mappedRefiInsts.clear();

// 		string unrolledStatementNo = getUnrolledStatementSignature2(
// 				assignStatement_->statementNo(), loopValues_, dpuCoord);

// 		if (assignStatement_->rhsSliceNames().size() != 1)
// 			LOG(FATAL) << ("createInstructionsForRegisterTransfer: only one variable allowed in the RHS!");

// 		// Creating a REFI instruction for the input variable and a SWB instruction for the output variable
// 		createInstructionsForAVariable(assignStatement_->rhsSliceNames()[0],
// 																	 loopValues_, storagePragma, assignStatement_->statementNo(),
// 																	 unrolledStatementNo, mappedRefiInsts, 0, posIndex, true, 1);

// 		// Creating a REFI instruction for the output variable
// 		createInstructionsForAVariable(assignStatement_->lhsSliceNames()[0],
// 																	 loopValues_, storagePragma, assignStatement_->statementNo(),
// 																	 unrolledStatementNo, mappedRefiInsts, 0, posIndex, false, 1);

// 		// The list should contain exactly two REFI instructions.
// 		if (mappedRefiInsts.size() != 2)
// 			LOG(FATAL) << ("createInstructionsForRegisterTransfer:: Internal Error!");

// 		RefiInstruction *inputRefiInst =
// 				(RefiInstruction *)mappedRefiInsts[0].instruction;
// 		RefiInstruction *outputRefiInst =
// 				(RefiInstruction *)mappedRefiInsts[1].instruction;

// 		inputRefiInst->correspondingRefiTransferInst = outputRefiInst;
// 		outputRefiInst->correspondingRefiTransferInst = inputRefiInst;

// 		// Adding all REFI instructions to the corresponding instruction list of DRRA cells
// 		for (auto &mappedRefiInst : mappedRefiInsts)
// 			_drraCluster->getDRRACell(mappedRefiInst.cellCoord)->instructions().addInstruction(mappedRefiInst.instruction, true);

// 		Coordinate &coord_src = mappedRefiInsts[0].cellCoord;
// 		Coordinate &coord_dest = mappedRefiInsts[1].cellCoord;

// 		// descriptor interface
// 		string res_name = "";
// 		if (resource.find(to_string(coord_src.row) + "_" + to_string(coord_src.column) + "_AGU2") != resource.end())
// 		{
// 			res_name = to_string(coord_src.row) + "_" + to_string(coord_src.column) + "_AGU2";
// 			inputRefiInst->portNo = 2;
// 			resource[to_string(coord_src.row) + "_" + to_string(coord_src.column) + "_AGU2"] = 1;
// 		}
// 		else
// 		{
// 			res_name = to_string(coord_src.row) + "_" + to_string(coord_src.column) + "_AGU3";
// 			inputRefiInst->portNo = 3;
// 			resource[to_string(coord_src.row) + "_" + to_string(coord_src.column) + "_AGU3"] = 1;
// 		}
// 		int num_of_addr = inputRefiInst->numberOfAddress.value + 1;
// 		int exe_cycle = (num_of_addr + (num_of_addr - 1) * inputRefiInst->middleDelay.value) + ((num_of_addr + (num_of_addr - 1) * inputRefiInst->middleDelay.value) - 1) * inputRefiInst->repetitionDelay.value;

// 		schedule::Operation o_src_refi0(d_.gen_signature(path_), 0, {{to_string(coord_src.row) + "_" + to_string(coord_src.column) + "_SEQ", {{0, 0}}}, {res_name, {{0, INT_MAX}}}});
// 		d_.add_operation(o_src_refi0);
// 		d_.get_mutable_operation(path_).add_child(o_src_refi0.name(), child_group_);
// 		schedule::Operation o_src_refi1(d_.gen_signature(path_), 0, {});
// 		if (first_src_name == "")
// 		{
// 			first_src_name = o_src_refi1.name();
// 		}
// 		d_.add_operation(o_src_refi1);
// 		d_.get_mutable_operation(path_).add_child(o_src_refi1.name(), child_group_);
// 		schedule::Operation o_src_refi2(d_.gen_signature(path_), 0, {{res_name, {{INT_MIN, 0}}}});
// 		d_.add_operation(o_src_refi2);
// 		d_.get_mutable_operation(path_).add_child(o_src_refi2.name(), child_group_);
// 		d_.add_constraint(Constraint(o_src_refi0.name(), o_src_refi1.name(), 0, INT_MAX));
// 		d_.add_constraint(Constraint(o_src_refi1.name(), o_src_refi2.name(), exe_cycle - 1, exe_cycle - 1));
// 		inputRefiInst->labels.push_back(o_src_refi0.name());
// 		inputRefiInst->labels.push_back(o_src_refi1.name());
// 		inputRefiInst->labels.push_back(o_src_refi2.name());

// 		// descriptor interface
// 		if (resource.find(to_string(coord_dest.row) + "_" + to_string(coord_dest.column) + "_AGU1") != resource.end())
// 		{
// 			res_name = to_string(coord_dest.row) + "_" + to_string(coord_dest.column) + "_AGU1";
// 			outputRefiInst->portNo = 2;
// 			resource[to_string(coord_dest.row) + "_" + to_string(coord_dest.column) + "_AGU1"] = 1;
// 		}
// 		else
// 		{
// 			res_name = to_string(coord_dest.row) + "_" + to_string(coord_dest.column) + "_AGU0";
// 			outputRefiInst->portNo = 3;
// 			resource[to_string(coord_dest.row) + "_" + to_string(coord_dest.column) + "_AGU0"] = 1;
// 		}

// 		schedule::Operation o_dest_refi0(d_.gen_signature(path_), 0, {{to_string(coord_dest.row) + "_" + to_string(coord_dest.column) + "_SEQ", {{0, 0}}}, {res_name, {{0, INT_MAX}}}});
// 		d_.add_operation(o_dest_refi0);
// 		d_.get_mutable_operation(path_).add_child(o_dest_refi0.name(), child_group_);
// 		schedule::Operation o_dest_refi1(d_.gen_signature(path_), 0, {});
// 		d_.add_operation(o_dest_refi1);
// 		d_.get_mutable_operation(path_).add_child(o_dest_refi1.name(), child_group_);
// 		schedule::Operation o_dest_refi2(d_.gen_signature(path_), 0, {{res_name, {{INT_MIN, 0}}}});
// 		d_.add_operation(o_dest_refi2);
// 		d_.get_mutable_operation(path_).add_child(o_dest_refi2.name(), child_group_);
// 		d_.add_constraint(Constraint(o_dest_refi0.name(), o_dest_refi1.name(), 0, INT_MAX));
// 		d_.add_constraint(Constraint(o_dest_refi1.name(), o_dest_refi2.name(), exe_cycle - 1, exe_cycle - 1));

// 		outputRefiInst->labels.push_back(o_dest_refi0.name());
// 		outputRefiInst->labels.push_back(o_dest_refi1.name());
// 		outputRefiInst->labels.push_back(o_dest_refi2.name());

// 		d_.add_constraint(Constraint(o_src_refi1.name(), o_dest_refi1.name(), 0, 0));

// 		for (auto &swb_instr : inputRefiInst->correspondingSWBInsts)
// 		{
// 			swb_instr->source.portNo = inputRefiInst->portNo;
// 			swb_instr->destination.portNo = outputRefiInst->portNo;

// 			schedule::Operation o_swb0(d_.gen_signature(path_), 0, {{to_string(swb_instr->destination.cellCoordinate.row) + "_" + to_string(swb_instr->destination.cellCoordinate.column) + "_SEQ", {{0, 0}}}, {swb_instr->source.to_string() + "_" + swb_instr->destination.to_string(), {{0, INT_MAX}}}});
// 			d_.add_operation(o_swb0);
// 			d_.get_mutable_operation(path_).add_child(o_swb0.name(), child_group_);
// 			schedule::Operation o_swb2(d_.gen_signature(path_), 0, {{swb_instr->source.to_string() + "_" + swb_instr->destination.to_string(), {{INT_MIN, 0}}}});
// 			d_.add_operation(o_swb2);
// 			d_.get_mutable_operation(path_).add_child(o_swb2.name(), child_group_);
// 			swb_instr->labels.push_back(o_swb0.name());
// 			swb_instr->labels.push_back(o_swb2.name());
// 			d_.add_constraint(Constraint(o_swb0.name(), o_swb2.name(), 1, INT_MAX));
// 			d_.add_constraint(Constraint(inputRefiInst->labels[2], o_swb2.name(), 0, INT_MAX));
// 		}

// 		// descriptor interface
// 		if (first_src_name != o_src_refi1.name())
// 		{
// 			d_.add_constraint(Constraint(first_src_name, o_src_refi1.name(), 0, 0));
// 		}

// 		// descriptor interface
// 		if (inputRefiInst->dependentTo)
// 		{
// 			string p = inputRefiInst->dependentTo->labels[2];
// 			d_.add_constraint(Constraint(p, o_src_refi1.name(), 1, INT_MAX));
// 		}
// 		if (outputRefiInst->dependentTo)
// 		{
// 			string p = outputRefiInst->dependentTo->labels[2];
// 			d_.add_constraint(Constraint(p, o_dest_refi1.name(), 1, INT_MAX));
// 		}
// 	}
// }

// // This method creates a Jump instruction and adds it to the corresponding DRRA cell and also to the instruction map.
// JumpInstruction *BEngine::createJumpInstruction(int statementNo_,
// 																								VIR::Coordinate &coord_, LoopValuesMap *loopValues_)
// {
// 	JumpInstruction *jumpInst = CREATE_OBJECT_B(JumpInstruction);

// 	const Coordinate &dpuCoord = coord_;

// 	jumpInst->statementNo = statementNo_;
// 	jumpInst->unrolledStatementNo = getUnrolledStatementSignature2(statementNo_,
// 																																 loopValues_, dpuCoord);
// 	jumpInst->jumpAddressInstruction = 0; // It should be filled later

// 	_branch_manager.set_curr_inst_jump(coord_, jumpInst); // It is saved in order to fill the jump address

// 	_drraCluster->getDRRACell(dpuCoord)->instructions().addInstruction(jumpInst,
// 																																		 true);

// 	addInstructionToMap(jumpInst, statementNo_, loopValues_, dpuCoord,
// 											dpuCoord);
// 	return jumpInst;
// }

// // This method creates some LOOP_HEADER instructions and adds them to all DRRA cells corresponding to the given for-statement.
// map<string, BIR::LoopHeaderInstruction *> BEngine::createLoopHeaderInstruction(ForStatement *forStatement_, LoopValuesMap *loopValues_, schedule::Descriptor &d_, json &j_, string path_, int child_group_)
// {
// 	map<string, BIR::LoopHeaderInstruction *> loop_header_instr_map;
// 	int loopId = -1;

// 	LoopHeaderInstruction *loopHeaderInst;

// 	const int statementNo = forStatement_->statementNo();

// 	int iterationNo;
// 	bool isIterationStatic = true;
// 	int offsetForDynamicBound = 0;
// 	ForStatement *dependentForStatement = 0;
// 	bool hasRaccuVariable = false;

// 	if (forStatement_->isConstant())
// 		iterationNo = forStatement_->repetition();
// 	else
// 	{
// 		RangeExpression *loopBound = forStatement_->loopRange();
// 		BinaryExpression *binaryExpr = (BinaryExpression *)loopBound->end();

// 		hasRaccuVariable = loopBound->end()->kind() == ktIdentifier && ((Identifier *)loopBound->end())->object()->objectType() == otRaccuVariable;

// 		// Currently dynamic loops are only supported with the following condition:
// 		//  - Start of the loop bound and the step value should be constant.
// 		//  - End of the loop bound can be dependent to only one other loop iteration or a RACCU variable.
// 		//  - The expression of the loop bound end value can be in three formats:
// 		//    'r', 'i', and 'i+c', where 'r' is a RACCU variable, 'i' is the loop variable of another loop and 'c' is a constant number.
// 		if (!(loopBound->begin()->kind() == ktInteger && loopBound->increment()->kind() == ktInteger && (hasRaccuVariable || (loopBound->end()->kind() == ktIdentifier && ((Identifier *)loopBound->end())->object()->isLoopIndex()) || (loopBound->end()->kind() == ktBinaryExpression && binaryExpr->operatorType() == botAdd && ((binaryExpr->leftOperand()->kind() == ktIdentifier && ((Identifier *)binaryExpr->leftOperand())->object()->isLoopIndex() && binaryExpr->rightOperand()->kind() == ktInteger) || binaryExpr->leftOperand()->kind() == ktInteger) && ((binaryExpr->rightOperand()->kind() == ktIdentifier && ((Identifier *)binaryExpr->rightOperand())->object()->isLoopIndex() && binaryExpr->leftOperand()->kind() == ktInteger) || binaryExpr->rightOperand()->kind() == ktInteger)))))
// 			LOG(FATAL) << ("createLoopHeaderInstruction: Unsupported variable loop boundaries!");

// 		Identifier *indexIdentifier = 0;

// 		// Find the identifier (referring to the index of another for-statement) inside the dynamic bound.
// 		if (loopBound->end()->kind() == ktIdentifier)
// 			indexIdentifier = (Identifier *)loopBound->end();
// 		else if (loopBound->end()->kind() == ktBinaryExpression)
// 		{
// 			indexIdentifier =
// 					binaryExpr->leftOperand()->kind() == ktIdentifier ? (Identifier *)binaryExpr->leftOperand() : (Identifier *)binaryExpr->rightOperand();

// 			offsetForDynamicBound =
// 					binaryExpr->leftOperand()->kind() == ktInteger ? ((Integer *)binaryExpr->leftOperand())->value : ((Integer *)binaryExpr->rightOperand())->value;
// 		}

// 		// The corresponding for-statement for the indexIdentifier.
// 		dependentForStatement =
// 				((LoopObjectDeclaration *)indexIdentifier->object())->forStatement();

// 		isIterationStatic = false;
// 	}

// 	Integer *startValue =
// 			dynamic_cast<Integer *>(forStatement_->loopRange()->begin());
// 	if (!startValue)
// 		LOG(FATAL) << ("createLoopHeaderInstruction: Unsupported data type in the loop range!");

// 	int id_counter = 0;
// 	int first_header_id = -1;
// 	string first_header_name = "";
// 	for (auto const &element : _allUtilizedCells)
// 	{
// 		Coordinate coordinate = element.second;

// 		DRRACell *drraCell = _drraCluster->getDRRACell(coordinate);

// 		string unrolledStatementNo = getUnrolledStatementSignature2(statementNo,
// 																																loopValues_, coordinate);

// 		loopId = drraCell->sequencer()->raccu()->getFreeLoopId();

// 		if (loopId == -1)
// 			LOG(FATAL) << ("createLoopHeaderInstruction: Reached the maximum number of supported RACCU loops!");

// 		int raccu_instr_count_1 = drraCell->instructions().size();
// 		// If dynamic loop bound has a corresponding RACCU variable
// 		if (hasRaccuVariable)
// 		{
// 			iterationNo =
// 					_raccuVariablesAddressMap[getRaccuVariableSignature(
// 							((Identifier *)forStatement_->loopRange()->end())->object()->name(),
// 							coordinate)];
// 		}
// 		// For dynamic loop bounds without a RACCU variable, a RACCU instruction should be added before the LOOP_HEADER instruction
// 		else if (!isIterationStatic)
// 		{
// 			int resultAddress = -3;

// 			int dependentLoopId = _loopHeaderInstructionMap[getLoopSignature(
// 																													dependentForStatement, coordinate, true)]
// 																->loopId;

// 			createRaccuInstruction(statementNo, rmAddWithLoopIndex,
// 														 dependentLoopId, true, offsetForDynamicBound, true,
// 														 resultAddress, coordinate, coordinate, loopValues_, 0);

// 			iterationNo = resultAddress;
// 		}
// 		int raccu_instr_count_2 = drraCell->instructions().size();

// 		loopHeaderInst = CREATE_LOOP_HEADER_INST(statementNo,
// 																						 unrolledStatementNo, loopId, isIterationStatic,
// 																						 startValue->value, iterationNo);
// 		loop_header_instr_map[to_string(coordinate.row) + "_" + to_string(coordinate.column)] = (loopHeaderInst);

// 		drraCell->instructions().addInstruction(loopHeaderInst, true);

// 		addInstructionToMap(loopHeaderInst, statementNo, loopValues_,
// 												coordinate, coordinate);

// 		// Add the loop instruction to the loop map.
// 		_loopHeaderInstructionMap[getLoopSignature(forStatement_, coordinate,
// 																							 true)] = loopHeaderInst;

// 		// Set the upper loop instruction
// 		ForStatement *upperLoop =
// 				getUpperLoop(
// 						dynamic_cast<ControlStatement *>(forStatement_->controlDependentTo()));

// 		if (upperLoop && _loopHeaderInstructionMap.find(
// 												 getLoopSignature(upperLoop, coordinate, true)) != _loopHeaderInstructionMap.end())
// 		{
// 			loopHeaderInst->upperLoop =
// 					_loopHeaderInstructionMap[getLoopSignature(upperLoop,
// 																										 coordinate, true)];
// 			loopHeaderInst->upperLoop->innerLoops.push_back(loopHeaderInst);
// 		}

// 		// descriptor interface
// 		schedule::Operation o_header(d_.gen_signature(path_), 0, {{to_string(coordinate.row) + "_" + to_string(coordinate.column) + "_SEQ", {{0, 0}}}});
// 		d_.add_operation(o_header);
// 		d_.get_mutable_operation(path_).add_child(o_header.name(), child_group_);
// 		loopHeaderInst->labels.push_back(o_header.name());
// 		if (first_header_name == "")
// 		{
// 			first_header_name = o_header.name();
// 		}
// 		if (first_header_name != o_header.name())
// 		{
// 			d_.add_constraint(Constraint(first_header_name, o_header.name(), 0, 0));
// 		}

// 		string last_raccu_name = "";
// 		for (int i = raccu_instr_count_1; i < raccu_instr_count_2; i++)
// 		{
// 			schedule::Operation o_raccu(d_.gen_signature(path_), 0, {{to_string(coordinate.row) + "_" + to_string(coordinate.column) + "_SEQ", {{0, 0}}}});
// 			d_.add_operation(o_raccu);
// 			d_.get_mutable_operation(path_).add_child(o_raccu.name(), child_group_);
// 			drraCell->instructions().instruction(i)->labels.push_back(o_raccu.name());
// 			if (last_raccu_name != "")
// 			{
// 				d_.add_constraint(Constraint(last_raccu_name, o_raccu.name(), 1, 1));
// 			}
// 			last_raccu_name = o_raccu.name();
// 		}

// 		if (last_raccu_name != "")
// 		{
// 			d_.add_constraint(Constraint(last_raccu_name, o_header.name(), 1, INT_MAX));
// 		}
// 	}

// 	_numberOfRaccuLoops++;

// 	return loop_header_instr_map;
// }

// // This method creates some LOOP_TAIL instructions and adds them to all DRRA cells corresponding to the given for-statement.
// map<string, LoopTailInstruction *> BEngine::createLoopTailInstruction(ForStatement *forStatement_,
// 																																			LoopValuesMap *loopValues_, schedule::Descriptor &d_, json &j_, string path_, int child_group_)
// {
// 	map<string, LoopTailInstruction *> loopt_instr_map;
// 	LoopTailInstruction *loopTailInst;

// 	const int statementNo = forStatement_->statementNo();

// 	Integer *stepValue = dynamic_cast<Integer *>(forStatement_->loopRange()->increment());
// 	if (!stepValue)
// 		LOG(FATAL) << ("createLoopTailInstruction: Unsupported data type in the loop range!");

// 	// This will be filled later in the synchronization part
// 	const int gotoPC = 0;

// 	int id_counter = 0;
// 	int first_loop_tail_id = -1;
// 	string first_loop_tail_name = "";
// 	for (auto const &element : _allUtilizedCells)
// 	{
// 		Coordinate coordinate = element.second;

// 		string unrolledStatementNo = getUnrolledStatementSignature2(statementNo,
// 																																loopValues_, coordinate);

// 		// Get the corresponding loop header instruction
// 		LoopHeaderInstruction *loopHeaderInst =
// 				_loopHeaderInstructionMap[getLoopSignature(forStatement_,
// 																									 coordinate, true)];

// 		loopTailInst = CREATE_LOOP_TAIL_INST(statementNo, unrolledStatementNo,
// 																				 loopHeaderInst->loopId, gotoPC, stepValue->value);

// 		loopHeaderInst->correspondingLoopTailInst = loopTailInst;

// 		const string loopTailSignature = getLoopSignature(forStatement_,
// 																											coordinate, false);

// 		vector<MappedRaccuInstruction>::iterator iter =
// 				_deferredRaccuInstructions.begin();

// 		// Before adding the LOOP_TAIL instruction to the DRRA cell, add all postponed RACCU
// 		// instructions corresponding to this loop tail and erase them from the list.
// 		int raccu_instr_count_1 =
// 				_drraCluster->getDRRACell(coordinate)->instructions().size();
// 		while (iter != _deferredRaccuInstructions.end())
// 		{
// 			if (loopTailSignature == iter->loopTailSignature)
// 			{
// 				_drraCluster->getDRRACell(coordinate)->instructions().addInstruction(iter->raccuInst, true);
// 				iter = _deferredRaccuInstructions.erase(iter);
// 			}
// 			else
// 				iter++;
// 		}
// 		int raccu_instr_count_2 =
// 				_drraCluster->getDRRACell(coordinate)->instructions().size();
// 		_drraCluster->getDRRACell(coordinate)->instructions().addInstruction(loopTailInst, true);

// 		addInstructionToMap(loopTailInst, statementNo, loopValues_, coordinate,
// 												coordinate);

// 		loopt_instr_map[to_string(coordinate.row) + "_" + to_string(coordinate.column)] = loopTailInst;

// 		// descriptor interface
// 		schedule::Operation o_tail(d_.gen_signature(path_), 0, {{to_string(coordinate.row) + "_" + to_string(coordinate.column) + "_SEQ", {{0, 0}}}});
// 		d_.add_operation(o_tail);
// 		d_.get_mutable_operation(path_).add_child(o_tail.name(), child_group_);
// 		loopTailInst->labels.push_back(o_tail.name());

// 		if (first_loop_tail_name == "")
// 		{
// 			first_loop_tail_name = o_tail.name();
// 		}
// 		if (first_loop_tail_name != o_tail.name())
// 		{
// 			d_.add_constraint(Constraint(first_loop_tail_name, o_tail.name(), 0, 0));
// 		}

// 		string last_raccu_name = "";
// 		for (int i = raccu_instr_count_1; i < raccu_instr_count_2; i++)
// 		{
// 			schedule::Operation o_raccu(d_.gen_signature(path_), 0, {{to_string(coordinate.row) + "_" + to_string(coordinate.column) + "_SEQ", {{0, 0}}}});
// 			d_.add_operation(o_raccu);
// 			d_.get_mutable_operation(path_).add_child(o_raccu.name(), child_group_);
// 			_drraCluster->getDRRACell(coordinate)->instructions().instruction(i)->labels.push_back(o_raccu.name());
// 			if (last_raccu_name != "")
// 			{
// 				d_.add_constraint(Constraint(last_raccu_name, o_raccu.name(), 1, 1));
// 			}
// 			last_raccu_name = o_raccu.name();
// 		}

// 		if (last_raccu_name != "")
// 		{
// 			d_.add_constraint(Constraint(last_raccu_name, o_tail.name(), 1, INT_MAX));
// 		}
// 	}
// 	return loopt_instr_map;
// }

// #define ALLOCATE_RACCU_ADDRESS \
// 	raccuFreeAddress = _raccu_var_manager.push(cellCoord_);

// // This method creates a RACCU instruction.
// // Operands:
// //  - If an operand is dynamic and the operand value is -1, it means that a free location of the RACCU register file should be allocated to that operand.
// // Result Address:
// //  - If resultAddress_ is -1, it means that operand1_ is dynamic and result address should get the location of operand1_.
// //  - If resultAddress_ is -2, it means that operand2_ is dynamic and result address should get the location of operand2_.
// //  - If resultAddress_ is -3, it means that a free location of the RACCU register file should be allocated to the result address.
// //  - For positive values of resultAddress_, result address will directly get that value.
// //  - The resultAddress will be updated with the correct address if it has had an undetermined location (-1, -2, -3).
// // The dependentRefiInst_ argument is the corresponding REFI instruction and it is used for RACCU instruction scheduling.
// // The instInsertLocation_ argument determines the location that the RACCU instruction should be inserted to the corresponding instruction list.
// //  - By default it is -1 which means that add the instruction to the end of the list.
// //  - If the value is -2, it means that the RACCU instruction should be added to the _deferredRaccuInstructions list for a postponed insertion.
// // The last argument (forStatement_) is used only when instInsertLocation_ == -2 and it helps in creating a signature for the location of inserting the RACCU instruction.
// RACCUInstruction *BEngine::createRaccuInstruction(int statementNo_,
// 																									BIR::BIREnumerations::RACCUMode raccuMode_, int operand1_,
// 																									bool isOp1Static_, int operand2_, bool isOp2Static_,
// 																									int &resultAddress_, const VIR::Coordinate &cellCoord_,
// 																									const VIR::Coordinate &refCoord_, LoopValuesMap *loopValues_,
// 																									BIR::Instruction *dependentInst_, int instInsertLocation_ /* = -1 */,
// 																									VIR::ForStatement *forStatement_ /* = 0 */)
// {
// 	RACCUInstruction *raccuInst = CREATE_OBJECT_B(RACCUInstruction);

// 	int raccuFreeAddress;

// 	raccuInst->statementNo = statementNo_;
// 	raccuInst->unrolledStatementNo = getUnrolledStatementSignature2(
// 			statementNo_, loopValues_, cellCoord_);
// 	raccuInst->raccuMode = raccuMode_;
// 	raccuInst->dependentTo = dependentInst_;

// 	if (isOp1Static_ || operand1_ != -1)
// 		raccuInst->operand1(operand1_, isOp1Static_);
// 	else
// 	{
// 		ALLOCATE_RACCU_ADDRESS;

// 		raccuInst->operand1(raccuFreeAddress, false);
// 	}

// 	if (isOp2Static_ || operand2_ != -1)
// 		raccuInst->operand2(operand2_, isOp2Static_);
// 	else
// 	{
// 		ALLOCATE_RACCU_ADDRESS;

// 		raccuInst->operand2(raccuFreeAddress, false);
// 	}

// 	switch (resultAddress_)
// 	{
// 	case -1:
// 		raccuInst->resultAddress = raccuInst->operand1.value;
// 		break;
// 	case -2:
// 		raccuInst->resultAddress = raccuInst->operand2.value;
// 		break;
// 	case -3:
// 		ALLOCATE_RACCU_ADDRESS;
// 		raccuInst->resultAddress = raccuFreeAddress;
// 		break;
// 	default:
// 		raccuInst->resultAddress = resultAddress_;
// 		break;
// 	}

// 	// Updating the result address
// 	resultAddress_ = raccuInst->resultAddress;

// 	// Adding the instruction to the corresponding DRRA cell
// 	if (instInsertLocation_ == -1)
// 		_drraCluster->getDRRACell(cellCoord_)->instructions().addInstruction(raccuInst, true);
// 	else if (instInsertLocation_ != -2)
// 	{
// 		_drraCluster->getDRRACell(cellCoord_)->instructions().insert(instInsertLocation_, raccuInst);

// 		// Update the resource sharing checkpoint
// 		if (_isResourceSharingRegion)
// 			_resSharingInstructionManager.incrementIndexCheckpoint(cellCoord_);
// 	}
// 	else // it needs a postponed insertion
// 	{
// 		_deferredRaccuInstructions.push_back(
// 				MappedRaccuInstruction(
// 						getLoopSignature(forStatement_, cellCoord_, false),
// 						raccuInst));
// 	}

// 	addInstructionToMap(raccuInst, statementNo_, loopValues_, refCoord_,
// 											cellCoord_);
// 	return raccuInst;
// }

// // This method creates a REFI instruction.
// BIR::RefiInstruction *BEngine::createRefiInstruction(int statementNo_,
// 																										 int twinStatementNo_, string unrolledStatementNo_,
// 																										 const string &variableName_,
// 																										 BIR::BIREnumerations::AddressMode addressMode_,
// 																										 BIR::BIREnumerations::PortMode portMode_, int numberOfAddress_,
// 																										 int stepValue_, int numberOfRepetition_, int repetitionOffset_,
// 																										 int reverse_bits_, bool is_reverse_bits_static_, bool isDimarchMode_,
// 																										 bool isInResSharingRegion_, BIR::DPUInstruction *correspondingDpuInst_,
// 																										 const string &pureVariableName_)
// {
// 	RefiInstruction *refiInst = CREATE_OBJECT_B(RefiInstruction);

// 	refiInst->statementNo = statementNo_;
// 	refiInst->twinStatementNo = twinStatementNo_;
// 	refiInst->variableName = variableName_;
// 	refiInst->pureVariableName = pureVariableName_;
// 	refiInst->unrolledStatementNo = unrolledStatementNo_;
// 	refiInst->addressMode = addressMode_;
// 	refiInst->portMode = portMode_;
// 	refiInst->portNo = -1;
// 	refiInst->initialDelay(0, true);
// 	refiInst->middleDelay(0, true);
// 	refiInst->repetitionDelay(0, true);
// 	refiInst->dependentTo = 0;
// 	refiInst->numberOfAddress(numberOfAddress_, true);
// 	refiInst->stepValue(stepValue_, true);
// 	refiInst->numberOfRepetition(numberOfRepetition_, true);
// 	refiInst->repetitionOffset = repetitionOffset_;
// 	refiInst->reverse_bits(reverse_bits_, is_reverse_bits_static_);
// 	refiInst->isDimarchMode = isDimarchMode_;
// 	refiInst->isInResSharingRegion = isInResSharingRegion_;
// 	refiInst->correspondingDpuInst = correspondingDpuInst_;
// 	refiInst->correspondingRefiTransferInst = 0;
// 	refiInst->bankNo = 0;

// 	return refiInst;
// }

// // This method creates a memory RFILE instruction and adds it to the corresponding DiMArch cell and also to the instruction map.
// void BEngine::createRfileInstruction(VIR::SliceName *sliceName_,
// 																		 LoopValuesMap *loopValues_, VIR::ReferencePragma *regFilePragma_,
// 																		 int statementNo_, string unrolledStatementNo_,
// 																		 vector<MappedInstruction> &mappedRefiInsts_, int posIndex_,
// 																		 bool isMemToRegTrans_)
// {
// 	createInstructionsForAVariable(sliceName_, loopValues_, regFilePragma_,
// 																 statementNo_, unrolledStatementNo_, mappedRefiInsts_, 0, posIndex_,
// 																 !isMemToRegTrans_, 1, -1, true);
// }

// // This method creates a memory route instruction and adds it to the corresponding DiMArch cell and also to the instruction map.
// BIR::RouteInstruction *BEngine::createRouteInstruction(int statementNo_,
// 																											 string unrolledStatementNo_, const VIR::Coordinate &memoryCoord_,
// 																											 const VIR::Coordinate &regFileCoord_, LoopValuesMap *loopValues_,
// 																											 bool isMemToRegTrans_, const string variableName_)
// {
// 	DRRACell *drraCell = _drraCluster->getDRRACell(regFileCoord_);

// 	RouteInstruction *routeInst = CREATE_OBJECT_B(RouteInstruction);

// 	routeInst->variableName = variableName_;
// 	routeInst->statementNo = statementNo_;
// 	routeInst->unrolledStatementNo = unrolledStatementNo_;
// 	routeInst->drraSelect = regFileCoord_.row;
// 	routeInst->isFromSource = isMemToRegTrans_;

// 	const int memoryActualRow = getActualDimarchRow(memoryCoord_.row);

// 	if (isMemToRegTrans_)
// 	{
// 		routeInst->sourceCoordinate = Coordinate(memoryActualRow,
// 																						 memoryCoord_.column);
// 		routeInst->destCoordinate = Coordinate(0, regFileCoord_.column);
// 	}
// 	else
// 	{
// 		routeInst->sourceCoordinate = Coordinate(0, regFileCoord_.column);
// 		routeInst->destCoordinate = Coordinate(memoryActualRow,
// 																					 memoryCoord_.column);
// 	}

// 	drraCell->instructions().addInstruction(routeInst, true);

// 	addInstructionToMap(routeInst, statementNo_, loopValues_, regFileCoord_,
// 											regFileCoord_);

// 	return routeInst;
// }

// RACCUInstruction *BEngine::create_raccu_instr(int statementNo_,
// 																							BIR::BIREnumerations::RACCUMode raccuMode_, int operand1_,
// 																							bool isOp1Static_, int operand2_, bool isOp2Static_,
// 																							int resultAddress_)
// {
// 	RACCUInstruction *raccuInst = CREATE_OBJECT_B(RACCUInstruction);
// 	raccuInst->statementNo = statementNo_;
// 	raccuInst->unrolledStatementNo = "";
// 	raccuInst->raccuMode = raccuMode_;
// 	raccuInst->dependentTo = NULL;
// 	raccuInst->operand1(operand1_, isOp1Static_);
// 	raccuInst->operand2(operand2_, isOp2Static_);
// 	raccuInst->resultAddress = resultAddress_;
// 	return raccuInst;
// }

// int BEngine::reserve_raccu_var(int statementNo_, VIR::AddressFunction af_, VIR::Coordinate coord_)
// {
// 	int raccu_var_index = _raccu_var_manager.push(coord_);
// 	RACCUInstruction *raccu_head = create_raccu_instr(statementNo_, rmAdd, af_.b(), true, 0, true, raccu_var_index);
// 	int earliest_loop_statement_no = INT_MAX;
// 	ForStatement *earliest_loop_ptr = NULL;
// 	for (int i = 0; i < af_.level(); i++)
// 	{
// 		if (af_.l(i)->statementNo() < earliest_loop_statement_no)
// 		{
// 			earliest_loop_statement_no = af_.l(i)->statementNo();
// 			earliest_loop_ptr = af_.l(i);
// 		}
// 		RACCUInstruction *raccu_tail = create_raccu_instr(statementNo_, rmAdd, raccu_var_index, false, af_.a(i), true, raccu_var_index);
// 		af_.l(i)->add_tail_raccu_instr(raccu_tail, coord_);
// 	}
// 	earliest_loop_ptr->add_head_raccu_instr(raccu_head, coord_);
// 	return raccu_var_index;
// }

// // This method creates a memory SRAM instruction and adds it to the corresponding DiMArch cell and also to the instruction map.
// SRAMInstruction *BEngine::createSramInstruction(VIR::SliceName *sliceName_,
// 																								int statementNo_, string unrolledStatementNo_,
// 																								const VIR::Coordinate &memoryCoord_,
// 																								const VIR::Coordinate &regFileCoord_, LoopValuesMap *loopValues_,
// 																								bool isMemToRegTrans_, IndexInfo *indexInfo_, int memoryStartAddr_,
// 																								const string variableName_,
// 																								BIR::RefiInstruction *correspondingRefiInst_,
// 																								BIR::RouteInstruction *correspondingRouteInst_)
// {
// 	SramAddressTransformer t;
// 	DRRACell *drraCell = _drraCluster->getDRRACell(regFileCoord_);

// 	_drraCluster->hasMemoryInstructions(true);

// 	SRAMInstruction *sramInst = CREATE_OBJECT_B(SRAMInstruction);

// 	sramInst->variableName = variableName_;
// 	sramInst->statementNo = statementNo_;
// 	sramInst->unrolledStatementNo = unrolledStatementNo_;
// 	sramInst->isRead = isMemToRegTrans_;
// 	sramInst->correspondingRefiInst = correspondingRefiInst_;
// 	sramInst->correspondingRouteInst = correspondingRouteInst_;
// 	sramInst->initialDelay(0, true);
// 	sramInst->loop1Delay(t.get_sram_agu_middle_delay(), true);
// 	sramInst->loop2Delay(t.get_sram_agu_repetition_delay(), true);

// 	VIR::Address addr = t.transform(sliceName_->addr);
// 	sliceName_->addr = addr;

// 	sramInst->executionCycle = addr.n0.b() * addr.n1.b();

// 	// adjust the offset of b so that it start from the correct position in the
// 	// storage. +memoryStartAddr is to shift it to the starting address of the
// 	// variable. -1 is to cancel the difference between matlab index and HW index
// 	// system, matlab index start from 1 instead of 0.
// 	addr.b.b(addr.b.b() + memoryStartAddr_ - 1);
// 	addr.n0.b(addr.n0.b() - 1);
// 	addr.n1.b(addr.n1.b() - 1);
// 	if (addr.b.is_constant())
// 	{
// 		sramInst->initialAddress(addr.b.b(), true);
// 	}
// 	else
// 	{
// 		// Here, a raccu variable will be created to calclate the dynamic address, the
// 		// raccu instructions will be created and associated with corresponding loops,
// 		// but the raccu instructions will not be added to instruction list immediately,
// 		// For loop will be responsible to add all raccu instructions accociated with
// 		// it.
// 		int raccu_var_id = reserve_raccu_var(statementNo_, addr.b, regFileCoord_);
// 		sramInst->initialAddress(raccu_var_id, false);
// 	}

// 	if (addr.a0.is_constant())
// 	{
// 		sramInst->loop1Increment = addr.a0.b();
// 	}
// 	else
// 	{
// 		LOG(FATAL) << "Increment can't be dynamic!";
// 	}

// 	if (addr.n0.is_constant())
// 	{
// 		sramInst->loop1Iterations(addr.n0.b(), true);
// 	}
// 	else
// 	{
// 		LOG(FATAL) << "Iteration can't be dynamic!";
// 	}

// 	if (addr.a1.is_constant())
// 	{
// 		sramInst->loop2Increment = addr.a0.b();
// 	}
// 	else
// 	{
// 		LOG(FATAL) << "Increment can't be dynamic!";
// 	}

// 	if (addr.n0.is_constant())
// 	{
// 		sramInst->loop2Iterations(addr.n1.b(), true);
// 	}
// 	else
// 	{
// 		LOG(FATAL) << "Iteration can't be dynamic!";
// 	}

// 	drraCell->instructions().addInstruction(sramInst, true);
// 	addInstructionToMap(sramInst, statementNo_, loopValues_, regFileCoord_, regFileCoord_);

// 	return sramInst;
// }

// // This method creates a SWB instruction for a variable.
// BIR::SWBInstruction *BEngine::createSwbInstruction(int statementNo_,
// 																									 string unrolledStatementNo_, const string &variableName_,
// 																									 LoopValuesMap *loopValues_,
// 																									 vector<BIR::SWBInstruction *> &swbInstructions_,
// 																									 const VIR::Coordinate &referenceCoord_,
// 																									 const BIR::ComponentInfo &srcComponentInfo_,
// 																									 const BIR::ComponentInfo &destComponentInfo_)
// {
// 	SWBInstruction *swbInst;
// 	swbInst = _swb_manager.check_connection(
// 			_swb_manager.get_signature(srcComponentInfo_.cellCoordinate.row,
// 																 srcComponentInfo_.cellCoordinate.column,
// 																 (int)(srcComponentInfo_.type), srcComponentInfo_.portNo),
// 			_swb_manager.get_signature(destComponentInfo_.cellCoordinate.row,
// 																 destComponentInfo_.cellCoordinate.column,
// 																 (int)(destComponentInfo_.type),
// 																 destComponentInfo_.portNo));
// 	if (swbInst)
// 	{
// 		return swbInst;
// 	}

// 	swbInst = CREATE_OBJECT_B(SWBInstruction);
// 	swbInstructions_.push_back(swbInst);

// 	swbInst->statementNo = statementNo_;
// 	swbInst->variableName = variableName_;
// 	swbInst->unrolledStatementNo = unrolledStatementNo_;

// 	swbInst->source = srcComponentInfo_;
// 	swbInst->destination = destComponentInfo_;

// 	InstructionList &instructionList = _drraCluster->getDRRACell(
// 																										 destComponentInfo_.cellCoordinate)
// 																				 ->instructions();

// 	swbInst->dependentTo = 0;

// 	// If this switch box is related to an output variable
// 	if (swbInst->destination.type == ctRegFile)
// 	{
// 		bool ignoreRegion = false;

// 		// Perform a linear backward search to find a SWB instruction with the same destination and port number (if exists) in order to
// 		// set it as a dependent SWB instruction. It is used later for calculating the minimum scheduled cycle of the new SWB instruction.
// 		// A special case is else-part of a branch that should not be falsely bind to a SWB instruction in the then-part.
// 		for (int instIndex = instructionList.size() - 1; instIndex >= 0;
// 				 --instIndex)
// 		{
// 			Instruction *instruction = instructionList.instruction(instIndex);

// 			// If the new SWB instruction is in the else-part region, we should ignore dependencies to SWB instructions in the then-part region.
// 			if (_branch_manager.isElsePartRegion)
// 			{
// 				// As the loop iteration is backward, When it reaches a jump instruction, it means that we are entering the then-part region.
// 				if (instruction->kind() == bktJumpInstruction)
// 					ignoreRegion = true;
// 				// When it reaches a branch instruction, it means that we are exiting the then-part region.
// 				else if (instruction->kind() == bktBranchInstruction)
// 					ignoreRegion = false;
// 			}

// 			SWBInstruction *prevSWBInst =
// 					dynamic_cast<SWBInstruction *>(instruction);

// 			if (!ignoreRegion && prevSWBInst && prevSWBInst->destination.type == ctRegFile && prevSWBInst->destination.portNo == swbInst->destination.portNo)
// 			{
// 				swbInst->dependentTo = prevSWBInst;
// 				break;
// 			}
// 		}
// 	}

// 	if (!_isResourceSharingRegion)
// 	{
// 		swbInst->isInResSharingRegion = false;
// 		instructionList.addInstruction(swbInst, true);
// 	}
// 	else
// 	{
// 		swbInst->isInResSharingRegion = true;
// 		_resSharingInstructionManager.addInstruction(swbInst,
// 																								 destComponentInfo_.cellCoordinate);
// 	}

// 	addInstructionToMap(swbInst, statementNo_, loopValues_, referenceCoord_,
// 											destComponentInfo_.cellCoordinate);

// 	_swb_manager.update_connection(
// 			_swb_manager.get_signature(srcComponentInfo_.cellCoordinate.row,
// 																 srcComponentInfo_.cellCoordinate.column,
// 																 (int)(srcComponentInfo_.type), srcComponentInfo_.portNo),
// 			_swb_manager.get_signature(destComponentInfo_.cellCoordinate.row,
// 																 destComponentInfo_.cellCoordinate.column,
// 																 (int)(destComponentInfo_.type), destComponentInfo_.portNo),
// 			swbInst);

// 	return swbInst;
// }

// // This method creates the unrolled dependency map for the given statement list.
// // It finds the dependent unrolled statements (RAW or WAR or WAW) that may be in different loop iterations.
// // Initially, the main program statement list is given to this method
// // and it traverses all nested loop bodies to find the dependency of all unrolled statements.
// // The second argument (location_) shows the location of the statement list according to if-then-else statements.
// //   - The default value is slOutsideIf which means that the location is outside of any if statement.
// //   - location_==slThenPart means that the statement list is inside then-part of an if statement.
// //   - location_==slElsePart means that the statement list is inside else-part of an if statement.
// void BEngine::createUnrolledDependencyMap(vector<VIR::Statement *> *statements_,
// 																					StatementLocation location_)
// {
// 	static LoopValuesMap loopValues;

// 	for (int stIndex = 0; stIndex < statements_->size(); ++stIndex)
// 	{
// 		Statement *statement = (*statements_)[stIndex];

// 		switch (statement->kind())
// 		{
// 		case ktAssignmentStatement:
// 		{
// 			AssignmentStatement *assignStatement =
// 					(AssignmentStatement *)statement;

// 			if (assignStatement->type() == atArithmetic || assignStatement->type() == atMemoryTransfer || assignStatement->type() == atRegisterTransfer || assignStatement->type() == atDPUChainConnection)
// 				createUnrolledDependencyMapForAStatement(assignStatement,
// 																								 &loopValues, location_);
// 		}
// 		break;

// 		case ktForStatement:
// 		{
// 			ForStatement *forStatement = (ForStatement *)statement;

// 			if (forStatement->needDynamicHandling())
// 			{
// 				loopValues[forStatement] =
// 						((Integer *)forStatement->loopRange()->begin())->value;

// 				createUnrolledDependencyMap(&forStatement->loopBody(),
// 																		location_);
// 			}
// 			else
// 			{
// 				int repetition =
// 						forStatement->isConstant() ? forStatement->repetition() : forStatement->unrolledRepetitionsMap().at(getUnrolledLoopValueSignature(&loopValues));

// 				// TODO: Assuming these values are constant integers
// 				int beginValue =
// 						((Integer *)forStatement->loopRange()->begin())->value;
// 				int step =
// 						((Integer *)forStatement->loopRange()->increment())->value;

// 				for (int i = 0, loopValue = beginValue; i < repetition;
// 						 ++i, loopValue += step)
// 				{
// 					loopValues[forStatement] = loopValue;

// 					createUnrolledDependencyMap(&forStatement->loopBody(),
// 																			location_);
// 				}
// 			}

// 			loopValues.erase(forStatement);
// 		}
// 		break;

// 		case ktIfStatement:
// 			createUnrolledDependencyMap(
// 					&((IfStatement *)statement)->thenPart(), slThenPart);
// 			break;

// 		case ktIfThenElseStatement:
// 			createUnrolledDependencyMap(
// 					&((IfThenElseStatement *)statement)->thenPart(),
// 					slThenPart);
// 			createUnrolledDependencyMap(
// 					&((IfThenElseStatement *)statement)->elsePart(),
// 					slElsePart);
// 			break;

// 		case ktPragmaStatement:
// 			enableResourceSharingRegion((PragmaStatement *)statement);
// 			break;

// 		default:
// 			LOG(FATAL) << ("createUnrolledDependencyMap: Unknown statement!");
// 			break;
// 		}
// 	}
// }

// // This method creates the unrolled dependency map for the given assignment statement
// // It uses the LoopValuesMap for creating a name pattern for the unrolled statement
// void BEngine::createUnrolledDependencyMapForAStatement(
// 		VIR::AssignmentStatement *assignStatement_,
// 		LoopValuesMap *loopValues_, StatementLocation location_)
// {
// 	// Creating a ScheduleComponent for the current statement
// 	ScheduleComponent currComponent;

// 	currComponent.statement = assignStatement_;
// 	currComponent.maxScheduledClock = 0;

// 	if (location_ == slOutsideIf && _isResourceSharingRegion)
// 		location_ = slResSharing;

// 	currComponent.stLocation = location_;

// 	bool found_dependency = false;
// 	string prevStatementSignature;

// 	for (int index = _dependencySignatures.size() - 1;
// 			 index >= 0 && !found_dependency; --index)
// 	{
// 		prevStatementSignature = _dependencySignatures[index];

// 		ScheduleComponent &prevScheduleComponent = _unrolledDependencyMap.at(
// 				prevStatementSignature);

// 		AssignmentStatement *prevStatement = prevScheduleComponent.statement;
// 		AssignmentStatement *currStatement = currComponent.statement;

// 		// Ignore dependencies between statements in the then-part and else-part of an if statement
// 		if (!(_isResourceSharingRegion && prevScheduleComponent.stLocation == slResSharing) && (assignStatement_->controlDependentTo() != prevStatement->controlDependentTo() || !(prevScheduleComponent.stLocation == slThenPart && location_ == slElsePart)))
// 		{
// 			vector<Identifier *> *currRhsIdentifiers =
// 					&assignStatement_->rhsIdentifiers();
// 			vector<Identifier *> *prevRhsIdentifiers =
// 					&prevStatement->rhsIdentifiers();

// 			// TODO: Do a regression test for the changes in this part

// 			// <RAW>
// 			// For all RHS identifiers of the current assignment statement, find the RAW data dependency to the previous assignment statements
// 			// If a RAW data dependency has been detected, set the dependency and exit the loop
// 			if (!found_dependency)
// 			{
// 				for (auto &curr_identifier : currStatement->rhsIdentifiers())
// 				{
// 					if (!found_dependency)
// 					{
// 						for (auto &prev_identifier : prevStatement->lhsIdentifiers())
// 						{
// 							if (curr_identifier->object() == prev_identifier->object())
// 							{
// 								currComponent.dependentTo =
// 										prevStatementSignature;
// 								found_dependency = true;
// 								break;
// 							}
// 						}
// 					}
// 					else
// 					{
// 						break;
// 					}
// 				}
// 			}
// 			// <WAW>
// 			// For the LHS identifier of the current assignment statement find the WAW data dependency to the previous assignment statements
// 			if (!found_dependency)
// 			{
// 				for (auto &curr_identifier : currStatement->lhsIdentifiers())
// 				{
// 					if (!found_dependency)
// 					{
// 						for (auto &prev_identifier : prevStatement->lhsIdentifiers())
// 						{
// 							if (curr_identifier->object() == prev_identifier->object())
// 							{
// 								currComponent.dependentTo =
// 										prevStatementSignature;
// 								found_dependency = true;
// 								break;
// 							}
// 						}
// 					}
// 					else
// 					{
// 						break;
// 					}
// 				}
// 			}
// 			// <WAR>
// 			// For all RHS identifiers of the previous assignment statements, find the WAR data dependency to the current assignment statements
// 			// If a WAR data dependency has been detected, set the dependency and exit the loop
// 			if (!found_dependency)
// 			{
// 				for (auto &curr_identifier : currStatement->lhsIdentifiers())
// 				{
// 					if (!found_dependency)
// 					{
// 						for (auto &prev_identifier : prevStatement->rhsIdentifiers())
// 						{
// 							if (curr_identifier->object() == prev_identifier->object())
// 							{
// 								currComponent.dependentTo =
// 										prevStatementSignature;
// 								found_dependency = true;
// 								break;
// 							}
// 						}
// 					}
// 					else
// 					{
// 						break;
// 					}
// 				}
// 			}
// 			// <RAR>
// 			// Since each SRAM has one read port, for memory transfer statements find RAR in order to avoid multiple accesses to the same memory.
// 			if (!found_dependency)
// 			{
// 				for (auto curr_identifier : currStatement->rhsIdentifiers())
// 				{
// 					if (!found_dependency)
// 					{
// 						for (auto prev_identifier : prevStatement->rhsIdentifiers())
// 						{
// 							currComponent.dependentTo = prevStatementSignature;
// 							found_dependency = true;
// 							break;
// 						}
// 					}
// 					else
// 					{
// 						break;
// 					}
// 				}
// 			}

// 			// If it's memory transfer statement, It always depends on previous memory transfer statement.
// 			if (!found_dependency)
// 			{
// 				if (assignStatement_->type() == atMemoryTransfer && prevStatement->type() == atMemoryTransfer)
// 				{
// 					currComponent.dependentTo = prevStatementSignature;
// 					found_dependency = true;
// 					Coordinate &referencePragma0 =
// 							((ReferencePragma *)(assignStatement_->pragma()))->coordinates[0];
// 					Coordinate &referencePragma1 =
// 							((ReferencePragma *)(prevStatement->pragma()))->coordinates[0];
// 					break;
// 				}
// 			}
// 		}
// 	}

// 	if (!assignStatement_->pragma())
// 		LOG(FATAL) << ("createUnrolledDependencyMapForAStatement: pragma does not exist!");

// 	ReferencePragma *referencePragma =
// 			(ReferencePragma *)assignStatement_->pragma();

// 	for (unsigned posIndex = 0; posIndex < referencePragma->coordinates.size();
// 			 ++posIndex)
// 	{
// 		string unrolledNameStr = getUnrolledStatementSignature1(
// 				assignStatement_->statementNo(), loopValues_,
// 				referencePragma->coordinates[posIndex]);

// 		_unrolledDependencyMap[unrolledNameStr] = currComponent;
// 		_dependencySignatures.push_back(unrolledNameStr); // This vector keeps track of the order of insertion into the dependency map
// 	}
// }

// This method finds all assignment statements related to the memory initialization and
// calculates a starting and an end address for the variable in the left hand side in the memory.
// Also this method sets the initial values of the memories in the corresponding fields of the data structure.
void BEngine::determineMemoriesStartAddrAndInitLoad(
		vector<VIR::Statement *> &totalStatementList_)
{
	// This array determines that a new variable of the MATLAB code
	// should be assigned from this specified location for each of the memories.
	int **memoriesCurrIndex = new int *[_drraCluster->diMArch()->rowNum()];

	for (int i = 0; i < _drraCluster->diMArch()->rowNum(); ++i)
	{
		memoriesCurrIndex[i] = new int[_drraCluster->diMArch()->colNum()];
		for (int j = 0; j < _drraCluster->diMArch()->colNum(); ++j)
			memoriesCurrIndex[i][j] = 0;
	}
	LOG(DEBUG) << "xxxxxxxxxxxxxxxxxxxxxxxx";
	for (int i = 0; i < totalStatementList_.size(); ++i)
	{

		AssignmentStatement *assignmentStatement =
				dynamic_cast<AssignmentStatement *>(totalStatementList_[i]);

		if (assignmentStatement && assignmentStatement->type() == atDeclarative && ((StoragePragma *)assignmentStatement->pragma())->storageType() == stMemory)
		{
			StoragePragma *storagePragma =
					(StoragePragma *)assignmentStatement->pragma();
			ObjectDeclaration *variableObject =
					static_cast<Identifier *>(assignmentStatement->lhs()[0])->object();

			// It supports initialization in the form of x = [10:20] or [1] or [1 2 -2 10] or zeros(n)
			//if (variableObject->value()->kind() != ktArray)
			//	LOG(FATAL) << ("determineMemoriesStartAddrAndInitLoad: Improper value for initialization!");
			LOG(DEBUG) << "xxxxxxxxxxxxxxxxxxxxxxxx";
			determineStorageStartAddrAndInitLoad(storagePragma, variableObject,
																					 memoriesCurrIndex);
		}
	}

	for (int i = 0; i < _drraCluster->diMArch()->rowNum(); ++i)
		delete[] memoriesCurrIndex[i];
	delete[] memoriesCurrIndex;
}

// This method is a utility method that is called in its overload method.
// arrayElements_ argument is used when the array has more than one element,
// beginVal_ and incVal_ arguments are used when the array has only one element which is a RangeExpression.
void BEngine::determineMemoryStartAddrAndInitLoad(
		VIR::StoragePragma *storagePragma_,
		VIR::ObjectDeclaration *variableObject_, int **&memoriesCurrIndex_,
		int totalElementsNo_, vector<VIR::Expression *> &arrayElements_,
		bool useArrayElements_, int beginVal_, int incVal_)
{
	LOG(DEBUG) << "xxxxxxxxxxxxxxxxxxxxxxxx2";
	// Currently a variable should fill the whole memory word, otherwise it causes assigning
	// several variables to a single word which may result in unwanted situations.
	// TODO: To fix this the fabric needs a masking mechanism.
	if (totalElementsNo_ % (cMemWordSplit * storagePragma_->parallelismSize()) != 0)
		LOG(FATAL) << ("determineMemoryStartAddrAndInitLoad: Invalid array size!");

	// This variable determines the number of elements that should be assigned to each memory.
	// It assumes even distribution in memories.
	const int perMemoryValueCount = totalElementsNo_ / (storagePragma_->parallelismSize() * cMemWordSplit);

	for (unsigned posIndex = 0; posIndex < storagePragma_->coordinates.size(); ++posIndex)
	{

		Coordinate &coord = storagePragma_->coordinates[posIndex];
		addCoordListToCoordMap(storagePragma_->coordinates, _allUtilizedDimarchCells);

		SRAM *sram = _drraCluster->diMArch()->getDiMArchCell(coord.row, coord.column)->sram();

		int &memoryCurrIndex = memoriesCurrIndex_[coord.row][coord.column];

		storagePragma_->maxIndexes[posIndex] = (posIndex + 1) * perMemoryValueCount;

		storagePragma_->startAddresses[posIndex] = memoryCurrIndex;

		memoryCurrIndex += perMemoryValueCount;

		if (memoryCurrIndex > _configManager->hwSetting().fabricSetting.sramDepth)
			LOG(FATAL) << ("determineMemoryStartAddrAndInitLoad: More than 128 elements in a memory!");

		int value = beginVal_;
		int arrayIndex = 0;

		for (int memIndex = storagePragma_->startAddresses[posIndex]; memIndex < memoryCurrIndex; ++memIndex)
		{
			MemElement *currElement = sram->memory(memIndex);
			currElement->correspondingObject(variableObject_);
			currElement->hasInitialValue(true);

			_drraCluster->diMArch()->incTotalUsedMemWords();

			if (useArrayElements_)
			{
				for (int subIndex = 0; subIndex < cMemWordSplit; ++subIndex)
				{
					Expression *arrayElement = arrayElements_[arrayIndex];

					if (arrayElement->kind() == ktInteger)
						currElement->intValues().at(subIndex) = ((Integer *)arrayElement)->value;
					else if (arrayElement->kind() == ktFloatingPoint)
					{
						currElement->realValues().at(subIndex) = ((FloatingPoint *)arrayElement)->value;
						currElement->isFixedPoint(true);
					}
					else
						LOG(FATAL) << ("determineMemoryStartAddrAndInitLoad: Improper array element for initialization!");

					++arrayIndex;
				}
			}
			else
			{
				for (int subIndex = 0; subIndex < cMemWordSplit; ++subIndex)
				{
					currElement->intValues().at(subIndex) = value;
					value += incVal_;
				}
			}
		}

		sram->hasInitialLoad(true);
	}
}

// This method is a utility method that is called in determineStorageStartAddrAndInitLoad().
// arrayElements_ argument is used when the array has more than one element,
// beginVal_ and incVal_ arguments are used when the array has only one element which is a RangeExpression.
void BEngine::determineRegFileStartAddrAndInitLoad(
		StoragePragma *storagePragma_,
		VIR::ObjectDeclaration *variableObject_, int **&regFilesCurrIndex_,
		int totalElementsNo_, vector<VIR::Expression *> &arrayElements_,
		bool useArrayElements_, int beginVal_, int incVal_)
{
	if (totalElementsNo_ % storagePragma_->parallelismSize() != 0)
		LOG(FATAL) << ("determineRegFileStartAddrAndInitLoad: Invalid array size!");

	// This variable determines the number of elements that should be assigned to each register file.
	// It assumes even distribution in register files.
	const int perRegFileValueCount = totalElementsNo_ / storagePragma_->parallelismSize();

	int value = beginVal_;
	int arrayIndex = 0;

	for (unsigned posIndex = 0; posIndex < storagePragma_->coordinates.size(); ++posIndex)
	{
		Coordinate &coord = storagePragma_->coordinates[posIndex];

		RegFile *regFile = _drraCluster->getDRRACell(coord)->regFile();

		int &regFileCurrIndex = regFilesCurrIndex_[coord.row][coord.column];

		storagePragma_->maxIndexes[posIndex] = (posIndex + 1) * perRegFileValueCount;

		storagePragma_->startAddresses[posIndex] = regFileCurrIndex;

		regFileCurrIndex += perRegFileValueCount;

		if (regFileCurrIndex > _configManager->hwSetting().fabricSetting.regFileDepth)
		{
			LOG(FATAL) << "determineRegFileStartAddrAndInitLoad: More than " << _configManager->hwSetting().fabricSetting.regFileDepth << " elements in a register file!";
		}
		for (int regIndex = storagePragma_->startAddresses[posIndex]; regIndex < regFileCurrIndex; ++regIndex)
		{
			Register *currReg = regFile->registers(regIndex);
			currReg->correspondingObject = variableObject_;

			if (useArrayElements_)
			{
				Expression *arrayElement = arrayElements_[arrayIndex++];

				if (arrayElement->kind() == ktInteger)
				{
					value = ((Integer *)arrayElement)->value;
					currReg->intValue = value;
				}
				else if (arrayElement->kind() == ktFloatingPoint)
				{
					currReg->realValue = ((FloatingPoint *)arrayElement)->value;
					currReg->isFixedPoint = true;
				}
				else
				{
					LOG(FATAL) << ("determineRegFileStartAddrAndInitLoad: Improper array element for initialization!");
				}
			}

			if (!useArrayElements_)
			{
				currReg->intValue = value;
				value += incVal_;
			}
		}

		if (!regFile->hasInitialLoad())
		{
			regFile->hasInitialLoad(true);
			_drraCluster->incTotalUsedRegFiles();
		}
	}
}

// This method finds all assignment statements related to the register file initialization and
// calculates a starting and an end address for the variable in the left hand side in the register file.
// Also this method sets the initial values of the register files in the corresponding fields of the data structure.
void BEngine::determineRegFilesStartAddrAndInitLoad(
		vector<VIR::Statement *> &totalStatementList_)
{
	// This array determines that a new variable of the MATLAB code
	// should be assigned from this specified location for each of the register files.
	int **regFilesCurrIndex = new int *[_drraCluster->rowNum()];

	for (int i = 0; i < _drraCluster->rowNum(); ++i)
	{
		regFilesCurrIndex[i] = new int[_drraCluster->colNum()];
		for (int j = 0; j < _drraCluster->colNum(); ++j)
			regFilesCurrIndex[i][j] = 0;
	}

	for (int stIndex = 0; stIndex < totalStatementList_.size(); ++stIndex)
	{
		AssignmentStatement *assignmentStatement =
				dynamic_cast<AssignmentStatement *>(totalStatementList_[stIndex]);

		if (assignmentStatement && assignmentStatement->type() == atDeclarative && ((StoragePragma *)assignmentStatement->pragma())->storageType() == stRegFile)
		{
			StoragePragma *storagePragma =
					(StoragePragma *)assignmentStatement->pragma();
			ObjectDeclaration *variableObject =
					static_cast<Identifier *>(assignmentStatement->lhs()[0])->object();

			// It supports initialization in the form of x = [10:20] or [1] or [1 2 -2 10] or zeros(n)
			//if (variableObject && variableObject->value()->kind() != ktArray)
			//	LOG(FATAL) << ("determineRegFilesStartAddrAndInitLoad: Improper value for initialization!");

			determineStorageStartAddrAndInitLoad(storagePragma, variableObject,
																					 regFilesCurrIndex);
		}
	}

	for (int i = 0; i < _drraCluster->rowNum(); ++i)
		delete[] regFilesCurrIndex[i];
	delete[] regFilesCurrIndex;
}

// This method finds the start address for the variable in the left hand side of the assignment statement and
// also fills the initial loads of the given storage (register file or memory) for this variable.
void BEngine::determineStorageStartAddrAndInitLoad(
		StoragePragma *storagePragma_, ObjectDeclaration *variableObject_,
		int **&storagesCurrIndex_)
{
	// Currently, only even distribution is supported.
	if (storagePragma_->distribution() != sdEvenDist)
		LOG(FATAL) << ("determineStorageStartAddrAndInitLoad: Unsupported storage distribution!");

	// The object value should be an array, even though the size is one.
	Array *objectValue = (Array *)variableObject_->value();

	const int arraySize = objectValue->elements().size();

	if (arraySize == 1) // case of array with only one element
	{
		Expression *arrayElement = objectValue->elements()[0];

		int beginVal, endVal, incVal;

		if (arrayElement->kind() == ktRangeExpression)
		{
			RangeExpression *rangeExpr = (RangeExpression *)arrayElement;

			beginVal = ((Integer *)rangeExpr->begin())->value;
			endVal = ((Integer *)rangeExpr->end())->value;
			incVal = ((Integer *)rangeExpr->increment())->value;
		}
		else if (arrayElement->kind() == ktInteger)
		{
			beginVal = ((Integer *)arrayElement)->value;
			endVal = beginVal;
			incVal = 1;
		}
		else
			LOG(FATAL) << ("determineStorageStartAddrAndInitLoad: Improper array element for initialization!");

		const int valueCount = (endVal - beginVal) / incVal + 1;

		if (storagePragma_->storageType() == stRegFile)
			determineRegFileStartAddrAndInitLoad(storagePragma_, variableObject_, storagesCurrIndex_, valueCount, objectValue->elements(), false, beginVal, incVal);
		else // stMemory
			determineMemoryStartAddrAndInitLoad(storagePragma_, variableObject_, storagesCurrIndex_, valueCount, objectValue->elements(), false, beginVal, incVal);
	}
	else if (arraySize > 1) // case of array with more than one element
	{
		if (storagePragma_->storageType() == stRegFile)
			determineRegFileStartAddrAndInitLoad(storagePragma_, variableObject_, storagesCurrIndex_, arraySize, objectValue->elements(), true, 0, 1);
		else // stMemory
			determineMemoryStartAddrAndInitLoad(storagePragma_, variableObject_, storagesCurrIndex_, arraySize, objectValue->elements(), true, 0, 1);
	}
	else
		LOG(FATAL) << ("determineStorageStartAddrAndInitLoad: zero-size array for initialization!");
}

// This method enables/disables the resource sharing region according to the resource sharing pragma
// The second argument is used during create instructions in order to create the resource sharing map
// The third argument is used during scheduling and determines when it is time to go to the next resource sharing map.
// void BEngine::enableResourceSharingRegion(
// 		VIR::PragmaStatement *pragmaStatement_, bool isMapCreation_,
// 		bool goToNextMap_)
// {
// 	static int resourceSharingNo = 0;

// 	if (pragmaStatement_ == NULL)
// 	{
// 		_isResourceSharingRegion = true;
// 	}
// 	else
// 	{
// 		if (pragmaStatement_->pragma()->blockProgramType() == bptResourceSharing)
// 			_isResourceSharingRegion =
// 					pragmaStatement_->pragma()->isBlockBegin();
// 	}

// 	if (_isResourceSharingRegion && isMapCreation_)
// 		_resourceSharingVariableMaps.push_back(ResourceSharingVariableMap());
// 	else if (_isResourceSharingRegion && !isMapCreation_ && _resourceSharingVariableMaps.size() > 0)
// 	{
// 		if (goToNextMap_)
// 			++resourceSharingNo;

// 		_currResourceSharingVariableMap =
// 				&_resourceSharingVariableMaps[resourceSharingNo];
// 	}

// 	if (isMapCreation_)
// 		_isResourceSharingRegion ? _resSharingInstructionManager.initializeInstructionLists(_drraCluster) : _resSharingInstructionManager.updateDrraInstructions(_drraCluster);
// }

// This method finds all utilized cells in the given statement list
void BEngine::findUtilizedCellsInBody(const vector<VIR::Statement *> &body_,
																			CoordinateMap &allUtilizedCells_)
{
	for (int stIndex = 0; stIndex < body_.size(); ++stIndex)
		findUtilizedCellsInStatement(body_[stIndex], allUtilizedCells_);
}

// This method finds all utilized cells in an if condition
void BEngine::findUtilizedCellsInIfCondition(
		const VIR::Expression *conditionExpr_,
		CoordinateMap &allUtilizedCells_)
{
	const BinaryExpression *binaryExpr =
			dynamic_cast<const BinaryExpression *>(conditionExpr_);

	// TODO: This condition should be relaxed
	if (!binaryExpr)
		LOG(FATAL) << ("scheduleIfCondition: Unsupported if condition!");

	if (binaryExpr->leftOperand()->kind() == ktSliceName)
	{
		ReferencePragma *refPragma =
				dynamic_cast<ReferencePragma *>(((SliceName *)binaryExpr->leftOperand())->prefix()->object()->pragma());

		if (refPragma)
			addCoordListToCoordMap(refPragma->coordinates, allUtilizedCells_);
	}

	if (binaryExpr->rightOperand()->kind() == ktSliceName)
	{
		ReferencePragma *refPragma =
				dynamic_cast<ReferencePragma *>(((SliceName *)binaryExpr->rightOperand())->prefix()->object()->pragma());

		if (refPragma)
			addCoordListToCoordMap(refPragma->coordinates, allUtilizedCells_);
	}
}

// This method finds all DRRA cells that have been utilized by the given statement or
// any other statements inside the body (case of if-statement and for-statement).
void BEngine::findUtilizedCellsInStatement(const VIR::Statement *statement_,
																					 CoordinateMap &allUtilizedCells_)
{
	switch (statement_->kind())
	{
	case ktAssignmentStatement:
	{
		AssignmentStatement *assignStatement =
				(AssignmentStatement *)statement_;

		if (assignStatement->type() == atArithmetic)
		{
			// The assignment statement utilized cells
			ReferencePragma *refPragma =
					dynamic_cast<ReferencePragma *>(assignStatement->pragma());

			addCoordListToCoordMap(refPragma->coordinates, allUtilizedCells_);
		}
		if (assignStatement->type() == atDeclarative && static_cast<VIR::StoragePragma *>(assignStatement->pragma())->storageType() == VIR::VirEnumerations::stRegFile)
		{
			// The assignment statement utilized cells
			ReferencePragma *refPragma =
					dynamic_cast<ReferencePragma *>(assignStatement->pragma());

			addCoordListToCoordMap(refPragma->coordinates, allUtilizedCells_);
		}
	}
	break;

	case ktForStatement:
	{
		ForStatement *forStatement = (ForStatement *)statement_;

		findUtilizedCellsInBody(forStatement->loopBody(), allUtilizedCells_);
	}
	break;

	case ktIfStatement:
	{
		IfStatement *ifStatement = (IfStatement *)statement_;

		addCoordListToCoordMap(ifStatement->conditionPragma()->coordinates,
													 allUtilizedCells_);

		findUtilizedCellsInIfCondition(ifStatement->condition(),
																	 allUtilizedCells_);

		findUtilizedCellsInBody(ifStatement->thenPart(), allUtilizedCells_);
	}
	break;

	case ktIfThenElseStatement:
	{
		IfThenElseStatement *ifThenElseStatement =
				(IfThenElseStatement *)statement_;

		addCoordListToCoordMap(
				ifThenElseStatement->conditionPragma()->coordinates,
				allUtilizedCells_);

		findUtilizedCellsInIfCondition(ifThenElseStatement->condition(),
																	 allUtilizedCells_);

		findUtilizedCellsInBody(ifThenElseStatement->thenPart(),
														allUtilizedCells_);

		findUtilizedCellsInBody(ifThenElseStatement->elsePart(),
														allUtilizedCells_);
	}
	break;

	case ktPragmaStatement:
		// do nothing
		break;

	default:
		LOG(FATAL) << ("findAllUtilizedCells: Unknown statement!");
		break;
	}
}

// // This method find the number of repetition of the DPU by investigating the given assignment statement.
// int BEngine::findDpuRepetition(VIR::AssignmentStatement *assignmentStatement_)
// {
// 	// For single operations, it is zero.
// 	int dpuRepetition = 0;

// 	SliceName *sliceName = 0;

// 	// TODO: you are here: for MIN, MAX and MAC you need to calculate a repetition number even if the arguments are index names.
// 	//					   number of addresses from createRefi instruction can be used.

// 	// For vector operations, it should be calculated
// 	if (assignmentStatement_->lhsSliceNames().size() == 1 && assignmentStatement_->lhsSliceNames()[0]->suffix()[0]->kind() == ktRangeExpression)
// 		sliceName = assignmentStatement_->lhsSliceNames()[0];
// 	else if (assignmentStatement_->rhs()->kind() == ktPrimitiveFunctionCall && assignmentStatement_->rhsSliceNames().size() > 0)
// 		sliceName = assignmentStatement_->rhsSliceNames()[0];
// 	// preserving MAC operation
// 	else if (assignmentStatement_->rhs()->kind() == ktBinaryExpression && ((BinaryExpression *)assignmentStatement_->rhs())->rightOperand()->kind() == ktPrimitiveFunctionCall && assignmentStatement_->rhsSliceNames().size() > 1)
// 		sliceName = assignmentStatement_->rhsSliceNames()[1];
// 	else if (assignmentStatement_->rhs()->kind() == ktPrimitiveFunctionCall && assignmentStatement_->lhsSliceNames().size() > 0)
// 	{
// 		sliceName = assignmentStatement_->lhsSliceNames()[0];
// 	}
// 	if (sliceName)
// 	{
// 		if (!assignmentStatement_->pragma())
// 			LOG(FATAL) << ("findDpuRepetition: Assignment statement should have a pragma!");

// 		const int parallelismSize = ((ReferencePragma *)assignmentStatement_->pragma())->parallelismSize();

// 		dpuRepetition = (sliceName->indexInfo()->addressInfo.numberOfAddress - (parallelismSize - 1)) / parallelismSize;
// 	}

// 	return dpuRepetition;
// }

// // This method gets the actual row index of the given index in the DiMArch
// // It has been determined according to the DiMArch structure.
// int BEngine::getActualDimarchRow(int virtualRowIndex_) const
// {
// 	// TODO: It may be revised in future
// 	return virtualRowIndex_ == 0 ? 1 : 2;
// }

// // This method gets the dependent assignment statement related to the given temp assignment statement.
// VIR::AssignmentStatement *BEngine::getDependentTempStatement(
// 		VIR::AssignmentStatement *tempAssignment_)
// {
// 	return ((SliceName *)tempAssignment_->rhs())->prefix()->dependentTo();
// }

// // This method gets the dpu cell signature corresponding to the given Coordinate.
// const string BEngine::getDpuCellSignature(const Coordinate &dpuCoord_) const
// {
// 	return to_string(dpuCoord_.row) + to_string(dpuCoord_.column);
// }

// // This method gets a constant expression and returns the corresponding constant value
// // whether an integer or fixed-point value to be set in the DPU instruction.
// int BEngine::getDpuConstantValue(Expression *constExpr_) const
// {
// 	const string errMessage =
// 			"getDpuConstantValue: Improper dpu constant value!";
// 	const int fixedPointDigits = _configManager->hwSetting().dpu.constantWidth - 1;
// 	int value;

// 	if (constExpr_->kind() == ktInteger)
// 		value = ((Integer *)constExpr_)->value;
// 	else if (constExpr_->kind() == ktFloatingPoint)
// 		value = getFixedPointValue(((FloatingPoint *)constExpr_)->value,
// 															 fixedPointDigits);
// 	else if (constExpr_->kind() == ktPrimitiveFunctionCall && ((PrimitiveFunctionCall *)constExpr_)->type() == ftFixedPoint)
// 		value =
// 				getFixedPointValue(
// 						((FloatingPoint *)(((PrimitiveFunctionCall *)constExpr_)->parametersSplit()[0]))->value,
// 						fixedPointDigits);
// 	else
// 		LOG(FATAL) << (errMessage);

// 	if (!isAValidDPUConstant(value))
// 		LOG(FATAL) << (errMessage);

// 	return value;
// }

// int BEngine::getDpuParameterImmediate(Expression *constExpr_) const
// {
// 	const string errMessage =
// 			"getDpuConstantValue: Improper dpu constant value!";
// 	const int fixedPointDigits = _configManager->hwSetting().dpu.constantWidth - 1;
// 	int value;

// 	if (constExpr_->kind() == ktInteger)
// 		value = ((Integer *)constExpr_)->value;
// 	else if (constExpr_->kind() == ktFloatingPoint)
// 		value = getFixedPointValue(((FloatingPoint *)constExpr_)->value,
// 															 fixedPointDigits);
// 	else if (constExpr_->kind() == ktPrimitiveFunctionCall && ((PrimitiveFunctionCall *)constExpr_)->type() == ftFixedPoint)
// 		value =
// 				getFixedPointValue(
// 						((FloatingPoint *)(((PrimitiveFunctionCall *)constExpr_)->parametersSplit()[0]))->value,
// 						fixedPointDigits);
// 	else
// 		LOG(FATAL) << (errMessage);

// 	if (!isAValidDPUConstant(value))
// 		LOG(FATAL) << (errMessage);

// 	return value;
// }

// int BEngine::getDpuParameterReference(Expression *constExpr_,
// 																			const VIR::Coordinate &coord_) const
// {
// 	int value;
// 	if (constExpr_->kind() == ktIdentifier)
// 	{
// 		string s = getRaccuVariableSignature(
// 				((Identifier *)constExpr_)->name(), coord_);
// 		RaccuVariablesAddressMap::const_iterator it =
// 				_raccuVariablesAddressMap.find(s);
// 		if (it != _raccuVariablesAddressMap.end())
// 		{
// 			value = it->second;
// 		}
// 		else
// 		{
// 			LOG(FATAL) << "RACCU variable name not found.";
// 		}
// 	}
// 	else
// 		LOG(FATAL) << "Reference name is not a RACCU variable.";
// 	return value;
// }

// // This method gets the corresponding execution cycle of a DPU mode
// int BEngine::getDpuModeExecCycle(const string dpuMode_)
// {
// 	// TODO: for each mode we need to find the needed execution cycle in the FABRIC.

// 	int exe_cycle = _primitive_func_lib.get_exec_cycle(dpuMode_);
// 	return exe_cycle;
// }

// // This method gets the corresponding integer value of a DPU mode
// int BEngine::getDpuModeValue(const string dpuMode_)
// {
// 	// TODO: for each mode we need to find the corresponding number in the FABRIC.
// 	int mode_value = _primitive_func_lib.get_mode(dpuMode_);
// 	if (mode_value < 0)
// 	{
// 		LOG(FATAL) << ("getDpuModeValue: No such mode:") << dpuMode_;
// 	}
// 	return mode_value;
// }

// // This method find the port according to the dpuMode_.
// int BEngine::getDpuPortNumbers(const string dpuMode_, bool isInput_,
// 															 int operandOrderNo_, int predefinedOutPort)
// {
// 	if (dpuMode_ == "silago_dpu_fft1" || dpuMode_ == "silago_dpu_fft2")
// 	{
// 		if (predefinedOutPort == 0)
// 		{
// 			int inPort1 = 3;
// 			int inPort2 = 0;
// 			int inPort3 = 2;
// 			int outPort = 0;
// 			return !isInput_ ? outPort : operandOrderNo_ == 1 ? inPort1 : operandOrderNo_ == 2 ? inPort2 : inPort3;
// 		}
// 		else if (predefinedOutPort == 1)
// 		{
// 			int inPort1 = 1;
// 			int inPort2 = 0;
// 			int inPort3 = 2;
// 			int outPort = 1;
// 			return !isInput_ ? outPort : operandOrderNo_ == 1 ? inPort1 : operandOrderNo_ == 2 ? inPort2 : inPort3;
// 		}
// 	}
// 	else
// 	{
// 		if (!isInput_)
// 		{
// 			int port = _primitive_func_lib.get_out_port(dpuMode_, operandOrderNo_ - 1);
// 			return port;
// 		}

// 		int port = _primitive_func_lib.get_in_port(dpuMode_, operandOrderNo_ - 1);
// 		return port;
// 	}
// }

// BIR::DPUInstruction *BEngine::getDpuInstruction(const string &unrolledStatementSig_)
// {
// 	vector<Instruction *> &instructions =
// 			_instructionsMap[unrolledStatementSig_].instructions;

// 	for (unsigned index = 0; index < instructions.size(); ++index)
// 	{
// 		Instruction *instruction = instructions.at(index);

// 		if (instruction->kind() == bktDPUInstruction)
// 			return (DPUInstruction *)instruction;
// 	}

// 	return 0;
// }

// // This method gets a real value and converts it to a fixed-point number.
// // The return value is the fixed-point number represented as an integer value.
// int BEngine::getFixedPointValue(float value_, int fixedPointDigits__) const
// {
// 	return (int)floor(value_ * pow(2.0, fixedPointDigits__));
// }

// // This method gets an instruction signature related to the given unrolledStatementSignature and the instruction cell location.
// string BEngine::getInstructionSignature(const string &unrolledStatementSig_,
// 																				const Coordinate &instCellCoord_)
// {
// 	return unrolledStatementSig_ + "_" + to_string(instCellCoord_.row) + "_" + to_string(instCellCoord_.column);
// }

// // This method gets a unique signature for the given loop statement in a specified DRRA cell.
// // isLoopHead_ argument determines if the signature should be generated for a LOOP_HEADER or LOOP_TAIL instruction.
// string BEngine::getLoopSignature(VIR::ForStatement *forStatement_,
// 																 const VIR::Coordinate &cellCoord_, bool isLoopHead_) const
// {
// 	//! Yu Yang 2017-08-01
// 	// Change to eliminate improper type conversion (pointer -> int)
// 	// CHANGE BEGIN
// 	return to_string((unsigned long int)forStatement_) + "_" + to_string(cellCoord_.row) + "_" + to_string(cellCoord_.column) + "_" + (isLoopHead_ ? "H" : "T");
// 	// CHANGE END
// }

// // This method takes an operand expression and updates the RACCU operand value and static mode
// // The last argument determines if this RACCU variables is related to a memory address,
// // so in that case the values should be divided by the memory block size.
// // TODO: Currently the operand expression can only be either an identifier or an integer.
// void BEngine::getRaccuOperand(const VIR::Expression *operandExpr_,
// 															const VIR::Coordinate &coord_, int &operand_, bool &isOpStatic_,
// 															bool isRaccuForMemAddress)
// {
// 	// It corresponds to a RACCU variable or a RACCU loop index
// 	if (operandExpr_->kind() == ktIdentifier)
// 	{
// 		Identifier *identifier = (Identifier *)operandExpr_;

// 		if (identifier->object()->objectType() == otConstant)
// 		{
// 			const int value = ((Integer *)identifier->object()->value())->value;

// 			operand_ = !isRaccuForMemAddress ? value : value / cMemWordSplit;
// 			isOpStatic_ = true;
// 		}
// 		else
// 		{
// 			operand_ = _raccuVariablesAddressMap[getRaccuVariableSignature(
// 					((Identifier *)operandExpr_)->name(), coord_)];
// 			isOpStatic_ = false;
// 		}
// 	}
// 	else if (operandExpr_->kind() == ktInteger)
// 	{
// 		const int value = ((Integer *)operandExpr_)->value;

// 		operand_ = !isRaccuForMemAddress ? value : value / cMemWordSplit;
// 		isOpStatic_ = true;
// 	}
// 	else
// 		LOG(FATAL) << ("getRaccuOperand: Unsupported operation!");
// }

// // This method gets a unique signature for the given RACCU variable in a specified DRRA cell.
// string BEngine::getRaccuVariableSignature(const string &name_,
// 																					const VIR::Coordinate &cellCoord_) const
// {
// 	return name_ + "_" + to_string(cellCoord_.row) + "_" + to_string(cellCoord_.column);
// }

// // This method gets a unique signature for the given resource sharing variable in a specified DRRA cell.
// string BEngine::getResSharingVariableSignature(VIR::SliceName *sliceName_,
// 																							 const VIR::Coordinate &cellCoord_) const
// {
// 	return getSliceNameSignature(sliceName_) + "_" + to_string(cellCoord_.row) + "_" + to_string(cellCoord_.column);
// }

// // This method gets a signature for the given slice name.
// string BEngine::getSliceNameSignature(VIR::SliceName *sliceName_) const
// {
// 	const string errMessage =
// 			"getSliceNameSignature: unsupported slice name for signature generation!";
// 	const string infoMessage =
// 			"getSliceNameSignature: potential source of problem!";

// 	if (sliceName_->suffix().size() != 1)
// 		LOG(FATAL) << (errMessage);

// 	Expression *suffixExpr = sliceName_->suffix()[0];
// 	string suffixName;

// 	switch (sliceName_->suffix()[0]->kind())
// 	{
// 	case ktIdentifier:
// 		suffixName = ((Identifier *)suffixExpr)->name();
// 		break;

// 	case ktInteger:
// 		suffixName = to_string(((Integer *)suffixExpr)->value);
// 		break;

// 	case ktRangeExpression:
// 	{
// 		Integer *beginValue =
// 				dynamic_cast<Integer *>(((RangeExpression *)suffixExpr)->begin());

// 		if (beginValue)
// 			suffixName = to_string(beginValue->value);
// 		else
// 		{
// 			suffixName = "";
// 			LOG(INFO) << (infoMessage);
// 		}
// 	}
// 	break;

// 	case ktBinaryExpression:
// 	{
// 		BinaryExpression *binaryExpression = (BinaryExpression *)suffixExpr;

// 		suffixName = "";

// 		if (binaryExpression->leftOperand()->kind() == ktIdentifier)
// 		{
// 			suffixName = ((Identifier *)binaryExpression->leftOperand())->name();

// 			if (binaryExpression->rightOperand()->kind() == ktInteger)
// 				suffixName += to_string(((Integer *)binaryExpression->rightOperand())->value);
// 			else if (binaryExpression->rightOperand()->kind() == ktIdentifier && ((Identifier *)binaryExpression->rightOperand())->object()->objectType() == otConstant)
// 				suffixName += to_string(((Integer *)((Identifier *)binaryExpression->rightOperand())->object()->value())->value);
// 			else
// 				LOG(INFO) << (infoMessage);
// 		}
// 		else
// 			LOG(INFO) << (infoMessage);
// 	}
// 	break;

// 	default:
// 		suffixName = "";
// 		break;
// 	}

// 	return sliceName_->prefix()->name() + "_" + suffixName;
// }

// // This method gets the position index of the register files corresponding to the given slice name according to the given dpu position index.
// int BEngine::getStoragePositionIndex(VIR::SliceName *sliceName_,
// 																		 int dpuPosIndex_)
// {
// 	const StoragePragma *storagePragma =
// 			dynamic_cast<StoragePragma *>(sliceName_->prefix()->object()->pragma());

// 	const IndexInfo *indexInfo = sliceName_->indexInfo();

// 	int regFilePosIndex = storagePragma->getPositionIndex(
// 														indexInfo->addressInfo.startAddress) +
// 												dpuPosIndex_;

// 	// Saturation case
// 	if (regFilePosIndex >= storagePragma->parallelismSize())
// 		regFilePosIndex = storagePragma->parallelismSize() - 1;

// 	return regFilePosIndex;
// }

// // This method calculates the unrolled-loop-value signature according the the given loop values that is used for
// // finding the proper value in the unrolledAddressMap of an IndexInfo or in the unrolledRepetitionMap of a ForStatement.
// string BEngine::getUnrolledLoopValueSignature(
// 		const LoopValuesMap *loopValues_) const
// {
// 	// Creating the proper signature to index the unrolled address map of indexInfo
// 	string signature;

// 	for (auto &element : *loopValues_)
// 		if (element.first->degree() > 2)
// 			signature += to_string(element.second) + "_";
// 	signature.pop_back(); // removing the extra "_"

// 	return signature;
// }

// // This method gets the signature name of the given statement according to the loop values
// // if isDeepSignature_ == true, all loop values are involved; otherwise just for statements with degree <= are involved.
// std::string BEngine::getUnrolledStatementSignature1(int statementNo_,
// 																										LoopValuesMap *loopValues_, const Coordinate &dpuCoord_,
// 																										bool isDeepSignature_) const
// {
// 	string loopStr = "";

// 	for (auto &element : *loopValues_)
// 		if (isDeepSignature_ || element.first->degree() > 2)
// 			loopStr += "_" + to_string(element.second);

// 	return to_string(statementNo_) + "_" + getDpuCellSignature(dpuCoord_) + loopStr;
// }

// // This method gets the signature name of the given statement according to the loop values.
// // The difference with the getUnrolledStatementSignature1 method is that it replaces 0 for the two most inner loops.
// string BEngine::getUnrolledStatementSignature2(int statementNo_,
// 																							 LoopValuesMap *loopValues_, const Coordinate &dpuCoord_) const
// {
// 	string loopStr = "";

// 	// As AGU supports 2 loop levels, so degree() == 1, 2 should be treated differently
// 	for (auto &element : *loopValues_)
// 		loopStr += "_" + to_string(element.second);
// 	// TODO: you are here: do a regression test for this change. If it works then merge both getUnrolledStatementSignature1 and 2 together
// 	//		loopStr += "_" + (element.first->degree() > 2 ? to_string(element.second) : "0");

// 	return to_string(statementNo_) + "_" + getDpuCellSignature(dpuCoord_) + loopStr;
// }

// // This method gets the upper loop according to the given control dependent statement object.
// VIR::ForStatement *BEngine::getUpperLoop(
// 		VIR::ControlStatement *controlDependentSt_)
// {
// 	ControlStatement *loopStatement = controlDependentSt_;

// 	while (loopStatement)
// 	{
// 		if (loopStatement->kind() == ktForStatement)
// 			break;
// 		else
// 			loopStatement =
// 					dynamic_cast<ControlStatement *>(loopStatement->controlDependentTo());
// 	}

// 	return loopStatement ? (ForStatement *)loopStatement : 0;
// }

// This method initializes the engine for a new main program.
void BEngine::init()
{
	_RepDelayFlags.reset();
	_isNonConstantLoopRegion = false;
	_isResourceSharingRegion = false;
	_numberOfRaccuLoops = 0;
	_allUtilizedCells.clear();
	_dpuChainMap.clear();
	_deferredRaccuInstructions.clear();
	_currLoop = 0;
}

// This method initializes DiMArch and Register file by determining the starting address and also filling the initial values.
void BEngine::initializeMemoryAndRegFiles(
		vector<VIR::Statement *> &totalStatementList_)
{
	LOG(DEBUG) << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
	determineRegFilesStartAddrAndInitLoad(totalStatementList_);
	determineMemoriesStartAddrAndInitLoad(totalStatementList_);
}

// // This method performs some initializations for the port assignment
// void BEngine::initializePortAssignment()
// {
// 	// Creating the 2D array
// 	_allInUsedAGUPorts = new InUsedAGUPorts *[_drraCluster->rowNum()];
// 	for (int i = 0; i < _drraCluster->rowNum(); ++i)
// 		_allInUsedAGUPorts[i] = new InUsedAGUPorts[_drraCluster->colNum()];
// }

// // This method checks whether the given condition expression is a valid condition for the if-statement.
// bool BEngine::isAValidIfCondition(VIR::Expression *conditionExpr_) const
// {
// 	const string errMessage = "isAValidIfCondition: Unsupported if condition!";

// 	BinaryExpression *binaryExpression =
// 			dynamic_cast<BinaryExpression *>(conditionExpr_);

// 	LOG(DEBUG) << "left=" << binaryExpression->leftOperand()->kindStr();
// 	LOG(DEBUG) << "right=" << binaryExpression->rightOperand()->kindStr();

// 	if (!binaryExpression || (binaryExpression->leftOperand()->kind() != ktSliceName || (binaryExpression->rightOperand()->kind() != ktSliceName && binaryExpression->rightOperand()->kind() != ktInteger && binaryExpression->rightOperand()->kind() != ktFloatingPoint)))
// 	{
// 		LOG(FATAL) << (errMessage);
// 		return false;
// 	}

// 	return true;
// }

// // This method determines if the given assignment statement is a reduction operation (e.g., MAC, MIN, MAX, etc.)
// bool BEngine::isAReductionOperation(
// 		VIR::AssignmentStatement *assignStatement_) const
// {
// 	bool isReduction = false;

// 	PrimitiveFunctionCall *primitiveFunctionCall =
// 			dynamic_cast<PrimitiveFunctionCall *>(assignStatement_->rhs());

// 	if (assignStatement_->rhs()->kind() == ktBinaryExpression)
// 		primitiveFunctionCall =
// 				dynamic_cast<PrimitiveFunctionCall *>(((BinaryExpression *)assignStatement_->rhs())->rightOperand());

// 	if (primitiveFunctionCall)
// 		switch (primitiveFunctionCall->type())
// 		{
// 		case ftMac:
// 		case ftMin:
// 		case ftMax:
// 			isReduction = true;
// 		}

// 	const string dpu_mode = static_cast<DPUPragma *>(assignStatement_->pragma())->dpuMode;
// 	if (_primitive_func_lib.get_reductive(dpu_mode))
// 	{
// 		isReduction = true;
// 	}

// 	cout << "reductive = " << isReduction << endl;

// 	return isReduction;
// }

// // This method determines if the given integer value is a valid constant for a DPU instruction.
// // Currently, as we have 8 bits for the constant, the integer value should be within (-128 to +127) range.
// bool BEngine::isAValidDPUConstant(int value_) const
// {
// 	const int bound = (int)pow(2.0,
// 														 _configManager->hwSetting().dpu.constantWidth - 1);

// 	return value_ <= bound - 1 && value_ >= -bound;
// }

// // This method determines if the given variable is a temp variable.
// bool BEngine::isTempVariable(VIR::Identifier *variable_) const
// {
// 	return variable_->object()->objectType() == otTempVariable;
// }

// // This method determines if the dpu corresponding to this assignment statement should be performed in fixed-point mode.
// bool BEngine::needFixedPointCalculation(
// 		VIR::AssignmentStatement *assignmentStatement_) const
// {
// 	for (auto &identifier : assignmentStatement_->lhsIdentifiers())
// 	{
// 		if (identifier->object()->isFixedPointVariable())
// 		{
// 			return true;
// 		}
// 	}
// 	return false;
// }

// // This method gets the port name signature according to the type of the port and its position.
// // The second argument is only used in case of input ports
// const string BEngine::portNameSignature(bool isInput_,
// 																				int operandOrderNo_) const
// {
// 	//	return !isInput_ ? "_out_" : operandOrderNo_ == 1 ? "_in1_" : operandOrderNo_ == 2 ? "_in2_" : "_in3_";
// 	if (isInput_)
// 	{
// 		return "_in" + to_string(operandOrderNo_) + "_";
// 	}
// 	else
// 	{
// 		return "_out" + to_string(operandOrderNo_) + "_";
// 	}
// }

// // --------------------------------------------------------------------------------
// // ------------------------- Methods of Private Structures ------------------------
// // --------------------------------------------------------------------------------

// // This method is used for comparing map elements of the LoopValuesMap.
// // It is used to add ForStatements to this map in order of their degrees.
// bool BEngine::CompareMapElements::operator()(const VIR::ForStatement *a,
// 																						 const VIR::ForStatement *b) const
// {
// 	return a->degree() > b->degree();
// }

// void BEngine::ResSharingInstructionManager::addInstruction(
// 		BIR::SWBInstruction *swbInst_, const VIR::Coordinate &coord_)
// {
// 	instructions[coord_.row][coord_.column].push_back(swbInst_);
// }

void BEngine::ResSharingInstructionManager::createInstructionLists(int rowNum_,
																																	 int colNum_)
{
	rowNum = rowNum_;
	colNum = colNum_;

	instructions = new vector<SWBInstruction *> *[rowNum_];
	insertIndexCheckpoints = new int *[rowNum_];
	prevRegionFinalIndex = new int *[rowNum_];

	for (int rowIndex = 0; rowIndex < rowNum_; ++rowIndex)
	{
		instructions[rowIndex] = new vector<SWBInstruction *>[colNum_];
		insertIndexCheckpoints[rowIndex] = new int[colNum_];
		prevRegionFinalIndex[rowIndex] = new int[colNum_];

		for (int colIndex = 0; colIndex < colNum; ++colIndex)
			prevRegionFinalIndex[rowIndex][colIndex] = 0;
	}
}

// // This method increments the index checkpoint when there is an alternative instruction insertion
// // (e.g. RACCU instruction insertion) prior to inserting resource sharing instructions.
// void BEngine::ResSharingInstructionManager::incrementIndexCheckpoint(
// 		const VIR::Coordinate &coord_)
// {
// 	insertIndexCheckpoints[coord_.row][coord_.column]++;
// }

// void BEngine::ResSharingInstructionManager::initializeInstructionLists(
// 		BIR::DRRACluster *drraCluster_)
// {
// 	for (int rowIndex = 0; rowIndex < rowNum; ++rowIndex)
// 		for (int colIndex = 0; colIndex < colNum; ++colIndex)
// 		{
// 			instructions[rowIndex][colIndex].clear();
// 			insertIndexCheckpoints[rowIndex][colIndex] =
// 					drraCluster_->getDRRACell(rowIndex, colIndex)->instructions().size();
// 		}
// }

// void BEngine::ResSharingInstructionManager::updateDrraInstructions(
// 		BIR::DRRACluster *drraCluster_)
// {
// 	for (int rowIndex = 0; rowIndex < rowNum; ++rowIndex)
// 		for (int colIndex = 0; colIndex < colNum; ++colIndex)
// 		{
// 			for (unsigned index = 0;
// 					 index < instructions[rowIndex][colIndex].size(); ++index)
// 				drraCluster_->getDRRACell(rowIndex, colIndex)->instructions().insert(insertIndexCheckpoints[rowIndex][colIndex] + index, instructions[rowIndex][colIndex][index]);

// 			prevRegionFinalIndex[rowIndex][colIndex] =
// 					drraCluster_->getDRRACell(rowIndex, colIndex)->instructions().size();
// 		}
// }

// int BEngine::ResSharingInstructionManager::getPrevRegionFinalIndex(
// 		const VIR::Coordinate &coord_)
// {
// 	return prevRegionFinalIndex[coord_.row][coord_.column];
// }
