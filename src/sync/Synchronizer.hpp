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

#ifndef __VESYLA_SYNC_SYNCHRONIZER_HPP__
#define __VESYLA_SYNC_SYNCHRONIZER_HPP__

#include "../BIR/BIRIncludeAll.hpp"
#include "../VIR/VIRIncludeAll.hpp"
#include "../VManager/MemoryManager.hpp"
#include "../util/Common.hpp"

using namespace BIR;
using namespace BIR::BIREnumerations;
using namespace VIR;
using namespace VIR::VirEnumerations;

namespace vesyla
{
namespace sync
{

class Synchronizer
{
private:
  int _max_init_delay;
  int _max_middle_delay;
  int _max_repetition_number;
  int _max_repetition_offset;

private:
  vector<BIR::Instruction *> sync_one_cell(vector<BIR::Instruction *> instr_list_, int end_time_);
  void add_wait(vector<BIR::Instruction *> &instr_list_, int &pc_, int &clk_, int num_cycles_);
  void sync_instr(vector<BIR::Instruction *> &instr_list_, int &pc, int &clk, Instruction *instr);
  vector<BIR::Instruction *> adjust_refis(vector<BIR::Instruction *> instr_list_);

public:
  Synchronizer();
  ~Synchronizer();
  map<string, vector<BIR::Instruction *>> synchronize(map<string, vector<BIR::Instruction *>> instr_lists_, int end_time_);
};

} // namespace sync
} // namespace vesyla

#endif // __VESYLA_SYNC_SYNCHRONIZER_HPP__