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

#ifndef __VESYLA_SIM_MODEL_HPP__
#define __VESYLA_SIM_MODEL_HPP__

#include "util/Common.hpp"
#include "BIR/BIRIncludeAll.hpp"

namespace vesyla
{
namespace sim
{
class Model
{
public:
	struct LoopManager
	{
		int iterator;
		int count;
		bool is_active;
		bool is_last_iteration;

		LoopManager() : iterator(0), count(0), is_active(false), is_last_iteration(false) {}
	};

private:
	vector<int> _raccu_regs;
	vector<LoopManager> _loop_managers;
	int _pc;
	int _clk;
	stack<int> _stack;

public:
	Model(int raccu_reg_count_, int loop_manager_count_);
	~Model();
	int simulate(vector<BIR::Instruction *> instrs_);
	void reset();
};
} // namespace sim
} // namespace vesyla

#endif // __VESYLA_SIM_MODEL_HPP__