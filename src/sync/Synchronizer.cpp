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

#include "Synchronizer.hpp"

namespace vesyla
{
namespace sync
{
Synchronizer::Synchronizer()
{
  util::GlobalVar glv;
  CHECK(glv.geti("max_init_delay", _max_init_delay));
  CHECK(glv.geti("max_middle_delay", _max_middle_delay));
  CHECK(glv.geti("max_repetition_number", _max_repetition_number));
  CHECK(glv.geti("max_repetition_offset", _max_repetition_offset));
}
Synchronizer::~Synchronizer()
{
}

map<string, vector<BIR::Instruction *>> Synchronizer::synchronize(
    map<string, vector<BIR::Instruction *>> instr_lists_, int end_time_)
{
  map<string, vector<BIR::Instruction *>> scheduled_instr_lists;
  for (auto &e : instr_lists_)
  {
    scheduled_instr_lists[e.first] = sync_one_cell(e.second, end_time_);
  }
  return scheduled_instr_lists;
}

vector<BIR::Instruction *>
Synchronizer::sync_one_cell(vector<BIR::Instruction *> instr_list_, int end_time_)
{
  vector<BIR::Instruction *> new_instr_list = adjust_refis(instr_list_);
  int instr_index = 0;
  int pc = 0;
  vector<int> clks;
  map<BIR::Instruction *, int> pc_map;
  clks.push_back(0);

  vector<BIR::Instruction *> scheduled_instr_list;
  for (auto &instr : new_instr_list)
  {
    switch (instr->kind())
    {
    case bktDPUInstruction:
    case bktSWBInstruction:
    case bktRouteInstruction:
    case bktRACCUInstruction:
    case bktLoopTailInstruction:
    case bktWaitInstruction:
    {
      sync_instr(scheduled_instr_list, pc, clks.back(), instr);
      pc_map[instr] = pc - 1;
      break;
    }
    case bktLoopHeaderInstruction:
    {
      static_cast<LoopHeaderInstruction *>(instr)->correspondingLoopTailInst->corresponding_loop_header_instr = instr;
      sync_instr(scheduled_instr_list, pc, clks.back(), instr);
      pc_map[instr] = pc - 1;
      break;
    }
    case bktSRAMInstruction:
    {
      sync_instr(scheduled_instr_list, pc, clks.back(), instr);
      pc += 2; // This is because SRAM instruction occupies 3 entries of instr RAM.
      pc_map[instr] = pc - 3;
      static_cast<SRAMInstruction *>(instr)->initialDelay(0, 1);
      instr->minScheduledClkCycle = instr->issue_time;
      instr->maxScheduledClkCycle = instr->end_time;
      break;
    }
    case bktBranchInstruction:
    {
      sync_instr(scheduled_instr_list, pc, clks.back(), instr);
      int curr_clk = clks.back();
      clks.push_back(curr_clk);
      pc_map[instr] = pc - 1;
      break;
    }
    case bktJumpInstruction:
    {
      // Jump instrucion is only used in IF-THEN-ELSE statement serving as the
      // jump point from the end of THEN part to the end of IF statement.
      BIR::JumpInstruction *jump_instr =
          static_cast<BIR::JumpInstruction *>(instr);
      jump_instr->minScheduledClkCycle =
          jump_instr->jumpAddressInstruction->minScheduledClkCycle - 1;
      jump_instr->maxScheduledClkCycle =
          jump_instr->jumpAddressInstruction->maxScheduledClkCycle - 1;
      sync_instr(scheduled_instr_list, pc, clks.back(), instr);
      clks.pop_back();
      pc_map[instr] = pc - 1;
      break;
    }
    case bktRefiInstruction:
    {
      RefiInstruction *refi_instr = static_cast<RefiInstruction *>(instr);
      Refi1Instruction *refi1_instr = CREATE_OBJECT_B(Refi1Instruction);
      refi1_instr->issue_time = refi_instr->issue_time;
      refi1_instr->arrive_time = refi_instr->arrive_time;
      refi1_instr->activation_time = refi_instr->activation_time;
      refi1_instr->end_time = refi_instr->end_time;
      refi1_instr->statementNo = refi_instr->statementNo;
      refi1_instr->unrolledStatementNo = refi_instr->unrolledStatementNo;
      refi1_instr->variableName = refi_instr->variableName;
      refi1_instr->pureVariableName = refi_instr->pureVariableName;
      refi1_instr->portNo = refi_instr->portNo;
      refi1_instr->startAddress = refi_instr->startAddress;
      refi1_instr->numberOfAddress = refi_instr->numberOfAddress;
      refi1_instr->minScheduledClkCycle = refi_instr->minScheduledClkCycle;
      refi1_instr->maxScheduledClkCycle = refi_instr->maxScheduledClkCycle;
      refi1_instr->isDimarchMode = refi_instr->isDimarchMode;
      refi1_instr->en_compression = refi_instr->en_compression;
      refi1_instr->initialDelay(refi_instr->initialDelay.value, refi_instr->initialDelay.isStatic); // an invalid negative number
      refi1_instr->numberOfSubseqInst = 0;
      sync_instr(scheduled_instr_list, pc, clks.back(), refi1_instr);
      pc_map[instr] = pc - 1;

      int middleDelayExt = 0;
      int numberOfRepetitionExt = 0;
      int repetitionOffsetExt = 0;
      // Creating refi2 instruction only with the following condition
      if (refi_instr->numberOfRepetition.value > 0 || refi_instr->middleDelay.value > 0 || (refi_instr->stepValue.value != 1 && refi_instr->numberOfAddress.value != 0) || (refi_instr->reverse_bits.value > 0 || refi_instr->reverse_bits.isStatic == false) || refi_instr->isDimarchMode)
      {
        refi1_instr->numberOfSubseqInst = 1;

        int middleDelay = refi_instr->middleDelay.value;
        int numberOfRepetition = refi_instr->numberOfRepetition.value;
        int repetitionOffset = refi_instr->repetitionOffset;

        if (middleDelay > _max_middle_delay)
        {
          const int modulus = _max_middle_delay + 1;
          middleDelayExt = middleDelay / modulus;
          middleDelay = middleDelay % modulus;
        }

        if (numberOfRepetition > _max_repetition_number)
        {
          numberOfRepetitionExt = numberOfRepetition - _max_repetition_number;
          numberOfRepetition = _max_repetition_number;
        }

        if (repetitionOffset > _max_repetition_offset)
        {
          repetitionOffsetExt = repetitionOffset - _max_repetition_offset;
          repetitionOffset = _max_repetition_offset;
        }

        Refi2Instruction *refi2Inst = CREATE_OBJECT_B(Refi2Instruction);
        refi2Inst->issue_time = refi_instr->issue_time;
        refi2Inst->arrive_time = refi_instr->arrive_time;
        refi2Inst->activation_time = refi_instr->activation_time;
        refi2Inst->end_time = refi_instr->end_time;
        refi2Inst->statementNo = refi_instr->statementNo;
        refi2Inst->unrolledStatementNo = refi_instr->unrolledStatementNo;
        refi2Inst->minScheduledClkCycle = refi_instr->minScheduledClkCycle;
        refi2Inst->maxScheduledClkCycle = refi_instr->maxScheduledClkCycle;
        refi2Inst->variableName = refi_instr->variableName;
        refi2Inst->pureVariableName = refi_instr->pureVariableName;
        refi2Inst->stepValue = refi_instr->stepValue;
        refi2Inst->isDimarchMode = refi_instr->isDimarchMode;
        refi2Inst->en_compression = refi_instr->en_compression;

        refi2Inst->middleDelay(middleDelay, true);
        refi2Inst->numberOfRepetition(numberOfRepetition, true);
        refi2Inst->repetitionOffset = repetitionOffset;

        scheduled_instr_list.push_back(refi2Inst);
        pc++;
      }

      // Creating refi3 instruction only with the following condition
      if (refi_instr->addressMode != amLinear || refi_instr->repetitionDelay.value > 0 || middleDelayExt > 0 || numberOfRepetitionExt != 0 || repetitionOffsetExt != 0 || (refi_instr->reverse_bits.value > 0 || refi_instr->reverse_bits.isStatic == false) || refi_instr->isDimarchMode || refi_instr->en_compression)
      {
        refi1_instr->numberOfSubseqInst = 2;

        Refi3Instruction *refi3Inst = CREATE_OBJECT_B(Refi3Instruction);
        refi3Inst->issue_time = refi_instr->issue_time;
        refi3Inst->arrive_time = refi_instr->arrive_time;
        refi3Inst->activation_time = refi_instr->activation_time;
        refi3Inst->end_time = refi_instr->end_time;

        refi3Inst->statementNo = refi_instr->statementNo;
        refi3Inst->unrolledStatementNo = refi_instr->unrolledStatementNo;
        refi3Inst->minScheduledClkCycle = refi_instr->minScheduledClkCycle;
        refi3Inst->maxScheduledClkCycle = refi_instr->maxScheduledClkCycle;
        refi3Inst->variableName = refi_instr->variableName;
        refi3Inst->pureVariableName = refi_instr->pureVariableName;
        refi3Inst->addressMode = refi_instr->addressMode;
        refi3Inst->repetitionDelay = refi_instr->repetitionDelay;
        refi3Inst->middleDelayExt = middleDelayExt;
        refi3Inst->numberOfRepetitionExt = numberOfRepetitionExt;
        refi3Inst->repetitionOffsetExt = repetitionOffsetExt;
        refi3Inst->reverse_bits = refi_instr->reverse_bits;
        refi3Inst->isDimarchMode = refi_instr->isDimarchMode;
        refi3Inst->en_compression = refi_instr->en_compression;

        scheduled_instr_list.push_back(refi3Inst);
        pc++;
      }
      break;
    }
    default:
      break;
    }
  }

  // Add wait if the instructuion issue time is not reach the end. Usually happens when the last instruction has more than 1 cycle execution time.
  if (clks.back() < end_time_ + 1)
  {
    add_wait(scheduled_instr_list, pc, clks.back(), end_time_ - clks.back() + 1);
  }

  // Fill goto part of LOOP_TAIL, JUMP, BRANCH instructions
  for (auto &instr : new_instr_list)
  {
    switch (instr->kind())
    {
    case bktLoopTailInstruction:
    {
      LoopTailInstruction *loop_tail_instr = static_cast<LoopTailInstruction *>(instr);
      loop_tail_instr->gotoPC = pc_map[loop_tail_instr->corresponding_loop_header_instr];
      break;
    }
    case bktBranchInstruction:
    {
      BranchInstruction *branch_instr = static_cast<BranchInstruction *>(instr);
      branch_instr->false_pc = pc_map[branch_instr->falseAddressInstruction] + 1; // branch to the next instruction after JUMP
      break;
    }
    case bktJumpInstruction:
    {
      // Jump instrucion is only used in IF-THEN-ELSE statement serving as the
      // jump point from the end of THEN part to the end of IF statement.
      BIR::JumpInstruction *jump_instr = static_cast<BIR::JumpInstruction *>(instr);
      jump_instr->jump_pc = pc_map[jump_instr->jumpAddressInstruction];
      break;
    }
    }
  }

  return scheduled_instr_list;
}

void Synchronizer::add_wait(vector<BIR::Instruction *> &instr_list_, int &pc_,
                            int &clk_, int num_cycles_)
{
  CHECK_GE(num_cycles_, 0);
  if (num_cycles_ == 0)
  {
    return;
  }
  if (pc_ > 0)
  {
    BIR::Instruction *prev_instr = instr_list_.back();
    if (prev_instr->kind() == bktWaitInstruction)
    {
      BIR::WaitInstruction *instr =
          static_cast<BIR::WaitInstruction *>(prev_instr);
      if (instr->numberOfCycles.isStatic == true)
      {
        int c = instr->numberOfCycles.value;
        instr->numberOfCycles(c + num_cycles_, true);
        instr->maxScheduledClkCycle += num_cycles_;
        clk_ += num_cycles_;
        return;
      }
    }
  }
  BIR::WaitInstruction *instr = CREATE_WAIT_INST(-1, "", num_cycles_, true, clk_);
  instr->issue_time = clk_;
  instr->arrive_time = clk_;
  instr->activation_time = clk_;
  instr->end_time = clk_ + num_cycles_;
  instr_list_.push_back(instr);
  clk_ += num_cycles_;
  pc_ += 1;
}

void Synchronizer::sync_instr(vector<BIR::Instruction *> &instr_list_, int &pc_,
                              int &clk_, Instruction *instr_)
{
  CHECK_GE(instr_->minScheduledClkCycle, clk_);
  if (instr_->minScheduledClkCycle > clk_)
  {
    add_wait(instr_list_, pc_, clk_, instr_->minScheduledClkCycle - clk_);
  }
  instr_list_.push_back(instr_);
  pc_++;
  clk_++;
}

vector<BIR::Instruction *> Synchronizer::adjust_refis(vector<BIR::Instruction *> instr_list_)
{
  vector<BIR::Instruction *> instr_list_1;
  vector<BIR::Instruction *> instr_list_2;
  instr_list_1 = instr_list_;
  bool flag_changed = true;

  while (flag_changed)
  {
    flag_changed = false;
    for (int i = 0; i < instr_list_1.size(); i++)
    {

      if (instr_list_1[i]->kind() == bktRefiInstruction)
      {
        RefiInstruction *refi_instr = static_cast<RefiInstruction *>(instr_list_1[i]);
        if (refi_instr->initialDelay.value > _max_init_delay && refi_instr->initialDelay.isStatic == true)
        {
          if (i == (instr_list_1.size() - 1))
          {
            refi_instr->minScheduledClkCycle = refi_instr->minScheduledClkCycle + (refi_instr->initialDelay.value - _max_init_delay);
            refi_instr->initialDelay(_max_init_delay, true);
            instr_list_2.push_back(refi_instr);
            break;
          }
          else
          {
            int earlist_issue_time = refi_instr->minScheduledClkCycle + (refi_instr->initialDelay.value - _max_init_delay);
            int j = i + 1;

            for (; j < instr_list_1.size(); j++)
            {
              if (instr_list_1[j]->minScheduledClkCycle < earlist_issue_time)
              {
                instr_list_2.push_back(instr_list_1[j]);
              }
              else if (instr_list_1[j]->minScheduledClkCycle > earlist_issue_time)
              {
                refi_instr->initialDelay((refi_instr->minScheduledClkCycle + refi_instr->initialDelay.value) - earlist_issue_time, true);
                refi_instr->minScheduledClkCycle = earlist_issue_time;
                instr_list_2.push_back(refi_instr);
                for (; j < instr_list_1.size(); j++)
                {
                  instr_list_2.push_back(instr_list_1[j]);
                }
                flag_changed = true;
                break;
              }
              else
              {
                instr_list_2.push_back(instr_list_1[j]);
                earlist_issue_time++;
                if (earlist_issue_time > refi_instr->minScheduledClkCycle + refi_instr->initialDelay.value)
                {
                  LOG(FATAL) << "Sync failed";
                }
              }
            }

            if (!flag_changed)
            {
              refi_instr->initialDelay((refi_instr->minScheduledClkCycle + refi_instr->initialDelay.value) - earlist_issue_time, true);
              refi_instr->minScheduledClkCycle = earlist_issue_time;
              instr_list_2.push_back(refi_instr);
              flag_changed = true;
            }

            if (flag_changed)
            {
              break;
            }
            LOG(FATAL) << "Sync failed";
          }
        }
        else
        {
          instr_list_2.push_back(instr_list_1[i]);
        }
      }
      else
      {
        instr_list_2.push_back(instr_list_1[i]);
      }
    }
    instr_list_1 = instr_list_2;
    instr_list_2.clear();
  }

  instr_list_2 = instr_list_1;
  return instr_list_2;
}

} // namespace sync
} // namespace vesyla