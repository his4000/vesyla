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

#include "Simulator.hpp"

namespace vesyla
{
namespace sim
{
int Simulator::simulate(map<string, vector<BIR::Instruction *>> instr_list_)
{
	int raccu_reg_file_depth = 0;
	int raccu_max_loop_num = 0;
	vesyla::util::GlobalVar glv;
	CHECK(glv.geti("raccu_reg_file_depth", raccu_reg_file_depth));
	CHECK(glv.geti("raccu_max_loop_num", raccu_max_loop_num));
	CHECK_GT(raccu_reg_file_depth, 0);
	CHECK_GT(raccu_max_loop_num, 0);
	int max_time = 0;
	for (auto &p : instr_list_)
	{
		Model m(raccu_reg_file_depth, raccu_max_loop_num);
		int t = m.simulate(p.second);
		if (t > max_time)
		{
			max_time = t;
		}
	}
	glv.puti("max_exec_cycle", max_time);
	return max_time;
}
} // namespace sim
} // namespace vesyla