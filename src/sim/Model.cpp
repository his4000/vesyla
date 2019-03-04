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

#include "Model.hpp"

namespace vesyla
{
namespace sim
{

Model::Model(int raccu_reg_count_, int loop_manager_count_)
{
	_pc = 0;
	_clk = 0;
	_raccu_regs.resize(raccu_reg_count_, 0);
	_loop_managers.resize(loop_manager_count_);
}
Model::~Model()
{
}
int Model::simulate(vector<BIR::Instruction *> instrs_)
{
	vector<BIR::Instruction *> expanded_instrs;
	for (auto &i : instrs_)
	{
		if (i->kind() != BIR::BIREnumerations::bktSRAMInstruction)
		{
			expanded_instrs.push_back(i);
		}
		else
		{
			expanded_instrs.push_back(i);
			expanded_instrs.push_back(i);
			expanded_instrs.push_back(i);
		}
	}

	int delay = 0;
	while (_pc < expanded_instrs.size())
	{
		BIR::Instruction *instr = expanded_instrs[_pc];
		switch (instr->kind())
		{
		case BIR::BIREnumerations::bktLoopHeaderInstruction:
		{
			BIR::LoopHeaderInstruction *ii = static_cast<BIR::LoopHeaderInstruction *>(instr);
			int loop_id = ii->loopId;
			int iterator = ii->startValue;
			int count = 0;
			if (_loop_managers[loop_id].is_active)
			{
				iterator = _loop_managers[loop_id].iterator;
				count = _loop_managers[loop_id].count;
			}
			else
			{
				_loop_managers[loop_id].iterator = iterator;
				_loop_managers[loop_id].count = 0;
				_loop_managers[loop_id].is_active = true;
			}

			int iteration_num = ii->iterationNumber;
			if (!ii->isStatic)
			{
				iteration_num = _raccu_regs[iteration_num];
			}
			if (count < iteration_num - 1)
			{
				_pc++;
				_loop_managers[loop_id].count++;
			}
			else
			{
				_loop_managers[loop_id].is_last_iteration = true;
				_pc++;
			}
			for (int i = 0; i <= ii->maxScheduledClkCycle - ii->minScheduledClkCycle; i++)
			{
				ii->exec_trace.push_back(_clk + i);
			}
			_clk++;
			delay = ii->maxScheduledClkCycle - ii->minScheduledClkCycle;
		}
		break;
		case BIR::BIREnumerations::bktLoopTailInstruction:
		{
			BIR::LoopTailInstruction *ii = static_cast<BIR::LoopTailInstruction *>(instr);
			int loop_id = ii->loopId;

			bool is_last_iteration = _loop_managers[loop_id].is_last_iteration;
			if (is_last_iteration)
			{
				_loop_managers[loop_id].iterator = 0;
				_loop_managers[loop_id].count = 0;
				_loop_managers[loop_id].is_active = false;
				_loop_managers[loop_id].is_last_iteration = false;
				_pc++;
			}
			else
			{
				_loop_managers[loop_id].iterator += ii->stepValue;
				_pc = ii->gotoPC;
			}
			for (int i = 0; i <= ii->maxScheduledClkCycle - ii->minScheduledClkCycle; i++)
			{
				ii->exec_trace.push_back(_clk + i);
			}
			_clk++;
			delay = ii->maxScheduledClkCycle - ii->minScheduledClkCycle;
		}
		break;
		case BIR::BIREnumerations::bktRACCUInstruction:
		{
			BIR::RACCUInstruction *ii = static_cast<BIR::RACCUInstruction *>(instr);
			_pc++;
			for (int i = 0; i <= ii->maxScheduledClkCycle - ii->minScheduledClkCycle; i++)
			{
				ii->exec_trace.push_back(_clk + i);
			}
			_clk++;
			delay = ii->maxScheduledClkCycle - ii->minScheduledClkCycle;

			// RACCU function
			int mode = ii->raccuMode;
			int op1 = 0;
			int op2 = 0;
			int ret_addr = ii->resultAddress;
			if (ii->operand1.isStatic)
			{
				op1 = ii->operand1.value;
			}
			else
			{
				op1 = _raccu_regs[ii->operand1.value];
			}
			if (ii->operand2.isStatic)
			{
				op1 = ii->operand2.value;
			}
			else
			{
				op1 = _raccu_regs[ii->operand2.value];
			}
			switch (mode)
			{
			case BIR::BIREnumerations::rmAdd:
				_raccu_regs[ret_addr] = op1 + op2;
				break;
			case BIR::BIREnumerations::rmAddWithLoopIndex:
				_raccu_regs[ret_addr] = _loop_managers[ii->operand1.value].iterator + op2;
				break;
			case BIR::BIREnumerations::rmShiftLeft:
				_raccu_regs[ret_addr] = op1 << op2;
				break;
			case BIR::BIREnumerations::rmShiftRight:
				_raccu_regs[ret_addr] = op1 >> op2;
				break;
			case BIR::BIREnumerations::rmSub:
				_raccu_regs[ret_addr] = op1 - op2;
				break;
			default:
				LOG(FATAL) << "Illegal RACCU mode!";
				break;
			}
		}
		break;
		case BIR::BIREnumerations::bktBranchInstruction:
		{
			BIR::BranchInstruction *ii = static_cast<BIR::BranchInstruction *>(instr);
			_pc++;
			for (int i = 0; i <= ii->maxScheduledClkCycle - ii->minScheduledClkCycle; i++)
			{
				ii->exec_trace.push_back(_clk + i);
			}
			_clk++;
			_stack.push(_clk);
			delay = ii->maxScheduledClkCycle - ii->minScheduledClkCycle;
		}
		break;
		case BIR::BIREnumerations::bktJumpInstruction:
		{
			BIR::BranchInstruction *ii = static_cast<BIR::BranchInstruction *>(instr);
			_pc++;
			for (int i = 0; i <= ii->maxScheduledClkCycle - ii->minScheduledClkCycle; i++)
			{
				ii->exec_trace.push_back(_clk + i);
			}
			_clk = _stack.top();
			_stack.pop();
			delay = ii->maxScheduledClkCycle - ii->minScheduledClkCycle;
		}
		break;
		case BIR::BIREnumerations::bktSRAMInstruction:
		{
			BIR::Instruction *ii = static_cast<BIR::Instruction *>(instr);
			_pc += 3;
			for (int i = 0; i <= ii->maxScheduledClkCycle - ii->minScheduledClkCycle; i++)
			{
				ii->exec_trace.push_back(_clk + i);
			}
			_clk++;
			delay = ii->maxScheduledClkCycle - ii->minScheduledClkCycle;
		}
		break;
		case BIR::BIREnumerations::bktDPUInstruction:
		case BIR::BIREnumerations::bktRouteInstruction:
		case BIR::BIREnumerations::bktSWBInstruction:
		case BIR::BIREnumerations::bktRefi1Instruction:
		{
			BIR::Instruction *ii = static_cast<BIR::Instruction *>(instr);
			_pc++;
			for (int i = 0; i <= ii->maxScheduledClkCycle - ii->minScheduledClkCycle; i++)
			{
				ii->exec_trace.push_back(_clk + i);
			}
			_clk++;
			delay = ii->maxScheduledClkCycle - ii->minScheduledClkCycle;
		}
		break;
		case BIR::BIREnumerations::bktRefi2Instruction:
		case BIR::BIREnumerations::bktRefi3Instruction:
		{
			BIR::Instruction *ii = static_cast<BIR::Instruction *>(instr);
			_pc++;
			for (int i = 0; i <= ii->maxScheduledClkCycle - ii->minScheduledClkCycle; i++)
			{
				ii->exec_trace.push_back(_clk + i - 1);
			}
		}
		break;
		case BIR::BIREnumerations::bktWaitInstruction:
		{
			BIR::WaitInstruction *ii = static_cast<BIR::WaitInstruction *>(instr);
			_pc++;
			for (int i = 0; i <= ii->maxScheduledClkCycle - ii->minScheduledClkCycle; i++)
			{
				ii->exec_trace.push_back(_clk + i);
			}
			_clk += (ii->maxScheduledClkCycle - ii->minScheduledClkCycle + 1);
			delay = 0;
		}
		break;
		default:
			LOG(FATAL) << "Unrecongnized instruction: " << instr->kindStr();
			break;
		}
	}
	return _clk + delay;
}
void Model::reset()
{
	_pc = 0;
	_clk = 0;
	while (!_stack.empty())
	{
		_stack.pop();
	}
	for (int i = 0; i < _raccu_regs.size(); i++)
	{
		_raccu_regs[i] = 0;
	}
	for (int i = 0; i < _loop_managers.size(); i++)
	{
		_loop_managers[i].iterator = 0;
		_loop_managers[i].is_active = false;
		_loop_managers[i].is_last_iteration = false;
	}
}
} // namespace sim
} // namespace vesyla