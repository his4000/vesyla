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


#ifndef _HWSetting_H_
#define _HWSetting_H_

struct FabricSetting
{
	int dimarchRowNumber;
	int dimarchColNumber;
	int drraRowNumber;
	int drraColNumber;
	int raccuMaxLoopNo;
	int raccuRegFileDepth;
	int regFileDepth;
	int seqRegFileDepth;
	int sramWidth;
	int sramDepth;
};

struct InstructionSetting
{
	int width;
};

struct Refi1Setting
{
	int startingAddress;
	int initDelay;
	int maxInitDelay;
};

struct Refi2Setting
{
	int stepValue;
	int middleDelay;
	int repetitionNumber;
	int repetitionStepValue;
	int maxMiddleDelay;
	int maxRepetitionNumber;
	int maxRepetitionStepValue;
};

struct Refi3Setting
{
	int repetitionDelay;
	int middleDelayExt;
	int repetitionNumberExt;
	int repetitionStepValueExt;
};

struct DpuSetting
{
	int constantWidth;
};

struct HWSetting
{
	FabricSetting fabricSetting;
	InstructionSetting instSetting;
	Refi1Setting refi1;
	Refi2Setting refi2;
	Refi3Setting refi3;
	DpuSetting dpu;
};

#endif // _HWSetting_H_