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


#include "ConfigManager.hpp"
#include "VManager/VFileNames.hpp"
#include <math.h>
#include <sstream>
#include <string>
#include "boost/algorithm/string.hpp"

ConfigManager::ConfigManager()
{
	fillSettingsMap();
}

// This method returns the HW setting
HWSetting & ConfigManager::hwSetting()
{
	return _hwSetting;
}

// This method reads the HW setting from the given file
void ConfigManager::readConfiguration()
{
	string strLine;
	
	string path = vesyla::util::SysPath::find_config_file("hw_setting.conf");
	CHECK_NE(path, "") << "Can't find hardware settig file. Abort!";
	
	_inFile.open(path);

	while (getline(_inFile, strLine))
	{
		if (strLine[0] != '#') // comment
		{
			if (strLine.find("<") != string::npos) // section
				changeSection(strLine);
			else if (strLine.find("$") != string::npos) // subsection
				changeSubSection(strLine);
			else if (strLine.find(":") != string::npos) // setting value
				updateSetting(strLine);
		}
	}

	_inFile.close();
}

// ---------------------------------------------------------------
// ----------------------- Private Methods -----------------------
// ---------------------------------------------------------------

// This method reads the section header and changes the current section accordingly
void ConfigManager::changeSection(const string & strLine)
{
	_currSection = (strLine.find("<Instructions>")) ? sInstruction : sOthers;
}

// This method fills the settings map which corresponds 
// the string name of all setting variables to their enumeration value.
void ConfigManager::fillSettingsMap()
{
	_settingsMap["DIMARCH_ROW"]			= svDimarchRow;
	_settingsMap["DIMARCH_COL"]			= svDimarchCol;
	_settingsMap["DPU_CONSTANT_WIDTH"]	= svDpuConstantWidth;
	_settingsMap["DRRA_COL"]			= svDrraCol;
	_settingsMap["DRRA_ROW"]			= svDrraRow;
	_settingsMap["INIT_DELAY"]			= svInitialDelay;
	_settingsMap["INSTR_WIDTH"]			= svInstructionWidth;
	_settingsMap["MIDDLE_DELAY"]		= svMiddleDelay;
	_settingsMap["MIDDLE_DELAY_EXT"]	= svMiddleDelayExt;
	_settingsMap["NUM_OF_REPT"]			= svNumOfRepetition;
	_settingsMap["NUM_OF_REPT_EXT"]		= svNumOfRepetitionExt;
	_settingsMap["RACCU_REGFILE_DEPTH"] = svRaccuRegFileDepth;
	_settingsMap["RACCU_MAX_LOOP_NO"]	= svRaccuMaxLoopNo;
	_settingsMap["REG_FILE_DEPTH"]		= svRegFileDepth;
	_settingsMap["REPT_DELAY"]			= svRepetitionDelay;
	_settingsMap["REP_STEP_VALUE"]		= svRepetitionStepValue;
	_settingsMap["REP_STEP_VALUE_EXT"]	= svRepetitionStepValueExt;
	_settingsMap["SEQ_REG_FILE_DEPTH"]	= svSeqRegFileDepth;
	_settingsMap["SRAM_WIDTH"]			= svSramWidth;
	_settingsMap["SRAM_DEPTH"]			= svSramDepth;
	_settingsMap["STARTING_ADDRS"]		= svStartingAddress;
	_settingsMap["STEP_VALUE"]			= svStepValue;
}

// This method reads a config line and updates the value in the proper HWSetting field.
void ConfigManager::updateSetting(const string & strLine)
{
	std::stringstream streamStr(strLine);
	string token;

	getline(streamStr, token, ':');
	boost::algorithm::trim(token);

	SettingsMap::const_iterator iter = _settingsMap.find(token);

	vesyla::util::GlobalVar glv;
	

	// If the setting variable has been found
	if (iter != _settingsMap.end())
	{
		// Get the value of the setting variable
		getline(streamStr, token);
		boost::algorithm::trim(token);
		int value = atoi(token.c_str());

		// TODO: currently as there is no repetition of variables in different sections and subsection,
		// so they are not used for distinguishing variable settings.
		// Maybe later sections and subsections should also be utilized.

		switch (iter->second)
		{
			case svDpuConstantWidth:
				_hwSetting.dpu.constantWidth = value;
				glv.puti("hw_setting.dpu.constant_width", value);
				break;
			case svDrraRow:
				_hwSetting.fabricSetting.drraRowNumber = value;
				glv.puti("hw_setting.fabric_setting.drra_row_number", value);
				break;
			case svDrraCol:
				_hwSetting.fabricSetting.drraColNumber = value;
				glv.puti("hw_setting.fabric_setting.drra_col_number", value);
				break;
			case svRaccuMaxLoopNo:
				_hwSetting.fabricSetting.raccuMaxLoopNo = value;
				glv.puti("hw_setting.fabric_setting.raccu_max_loop_no", value);
				break;
			case svRaccuRegFileDepth:
				_hwSetting.fabricSetting.raccuRegFileDepth = value;
				glv.puti("hw_setting.fabric_setting.raccu_reg_file_depth", value);
				break;
			case svRegFileDepth:
				_hwSetting.fabricSetting.regFileDepth = value;
				glv.puti("hw_setting.fabric_setting.reg_file_depth", value);
				break;
			case svDimarchCol:
				_hwSetting.fabricSetting.dimarchColNumber = value;
				glv.puti("hw_setting.fabric_setting.dimarch_col_number", value);
				break;
			case svDimarchRow:
				_hwSetting.fabricSetting.dimarchRowNumber = value;
				glv.puti("hw_setting.fabric_setting.dimarch_row_number", value);
				break;
			case svSeqRegFileDepth:
				_hwSetting.fabricSetting.seqRegFileDepth = value;
				glv.puti("hw_setting.fabric_setting.seq_reg_file_depth", value);
				break;
			case svSramDepth:
				_hwSetting.fabricSetting.sramDepth = value;
				glv.puti("hw_setting.fabric_setting.sram_depth", value);
				break;
			case svSramWidth:
				_hwSetting.fabricSetting.sramWidth = value;
				glv.puti("hw_setting.fabric_setting.sram_width", value);
				break;
			case svInstructionWidth:
				_hwSetting.instSetting.width = value;
				glv.puti("hw_setting.inst_setting.width", value);
				break;
			case svStartingAddress:
				_hwSetting.refi1.startingAddress = value;
				glv.puti("hw_setting.refi1.starting_address", value);
				break;
			case svInitialDelay:
				_hwSetting.refi1.initDelay = value;
				_hwSetting.refi1.maxInitDelay = (int)pow(2, value)-1;
				glv.puti("hw_setting.refi1.init_delay", value);
				glv.puti("hw_setting.refi1.max_init_delay", (int)pow(2, value)-1);
				break;
			case svStepValue:
				_hwSetting.refi2.stepValue = value;
				glv.puti("hw_setting.refi2.step_value", value);
				break;
			case svMiddleDelay:
				_hwSetting.refi2.middleDelay = value;
				_hwSetting.refi2.maxMiddleDelay = (int)pow(2, value)-1;
				glv.puti("hw_setting.refi2.middle_delay", value);
				glv.puti("hw_setting.refi2.max_middle_delay", (int)pow(2, value)-1);
				break;
			case svNumOfRepetition:
				_hwSetting.refi2.repetitionNumber = value;
				_hwSetting.refi2.maxRepetitionNumber = (int)pow(2, value)-1;
				glv.puti("hw_setting.refi2.repetition_number", value);
				glv.puti("hw_setting.refi2.max_repetition_number", (int)pow(2, value)-1);
				break;
			case svRepetitionStepValue:
				_hwSetting.refi2.repetitionStepValue = value;
				_hwSetting.refi2.maxRepetitionStepValue = (int)pow(2, value)-1;
				glv.puti("hw_setting.refi2.repetition_step_value", value);
				glv.puti("hw_setting.refi2.max_repetition_step_value", (int)pow(2, value)-1);
				break;
			case svRepetitionDelay:
				_hwSetting.refi3.repetitionDelay = value;
				glv.puti("hw_setting.refi3.repetition_delay", value);
				break;
			case svMiddleDelayExt:
				_hwSetting.refi3.middleDelayExt = value;
				glv.puti("hw_setting.refi3.middle_delay_ext", value);
				break;
			case svNumOfRepetitionExt:
				_hwSetting.refi3.repetitionNumberExt = value;
				glv.puti("hw_setting.refi3.repetition_number_ext", value);
				break;
			case svRepetitionStepValueExt:
				_hwSetting.refi3.repetitionStepValueExt = value;
				glv.puti("hw_setting.refi3.repetition_step_value_ext", value);
				break;
		}
	}
}

// This method reads the subsection section header and changes the current subsection accordingly
void ConfigManager::changeSubSection(const string & strLine)
{
	if (strLine.find("$GENERAL"))
		_currSubSection = ssGeneral;
	else if (strLine.find("$REFI1"))
		_currSubSection = ssRefi1;
	else if (strLine.find("$REFI2"))
		_currSubSection = ssRefi2;
	else if (strLine.find("$REFI3"))
		_currSubSection = ssRefi3;
	else if (strLine.find("$DPU"))
		_currSubSection = ssDpu;
	else if (strLine.find("$DRRA"))
		_currSubSection = ssDrra;
	else if (strLine.find("$DIMARCH"))
		_currSubSection = ssDimarch;
}

