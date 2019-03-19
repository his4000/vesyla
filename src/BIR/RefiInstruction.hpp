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

#include <vector>
using namespace std;

#ifndef __RefiInstruction_h__
#define __RefiInstruction_h__

#include "ValueWithStaticFlag.hpp"
// #include "SWBInstruction.hpp"
// #include "DPUInstruction.hpp"
#include "RegFileInstruction.hpp"
#include "AddressMode.hpp"
#include "PortMode.hpp"

namespace BIR
{
class ValueWithStaticFlag;
class SWBInstruction;
class DPUInstruction;
// class RegFileInstruction;
class RefiInstruction;
namespace BIREnumerations
{
// enum AddressMode;
// enum PortMode;
}
} // namespace BIR

namespace BIR
{
class RefiInstruction : public BIR::RegFileInstruction
{
public:
	BIR::ValueWithStaticFlag initialDelay;
	BIR::ValueWithStaticFlag middleDelay;
	BIR::ValueWithStaticFlag numberOfAddress;
	BIR::ValueWithStaticFlag numberOfRepetition;
	int portNo;
	BIR::ValueWithStaticFlag repetitionDelay;
	int repetitionOffset;
	BIR::ValueWithStaticFlag startAddress;
	BIR::ValueWithStaticFlag stepValue;
	BIR::BIREnumerations::AddressMode addressMode;
	BIR::BIREnumerations::PortMode portMode;
	vector<BIR::SWBInstruction *> correspondingSWBInsts;
	BIR::RefiInstruction *dependentTo;
	BIR::ValueWithStaticFlag reverse_bits;
	BIR::DPUInstruction *correspondingDpuInst;
	BIR::RefiInstruction *correspondingRefiTransferInst;
	int bankNo;

	BIR::Instruction *corresponding_looph_l1;
	BIR::Instruction *corresponding_loopt_l1;
	BIR::Instruction *corresponding_looph_l2;
	BIR::Instruction *corresponding_loopt_l2;

public:
	void load(pt::ptree p_);
	void load2(map<int, BIR::Instruction *> instr_list);
	pt::ptree dump();

	string to_bin(){};
	string to_str(){};
};
} // namespace BIR

#endif
