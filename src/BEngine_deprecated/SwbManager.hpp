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

#ifndef __VESYLA_BENGINE_SWB_MANAGER_HPP__
#define __VESYLA_BENGINE_SWB_MANAGER_HPP__

#include <map>
#include "BIR/SWBInstruction.hpp"

using namespace std;

namespace vesyla{
namespace bengine{

class SwbManager{
private:
	map<string, BIR::SWBInstruction*> _swb_status;
public:
	string get_signature(int row, int col, int type, int port);
	BIR::SWBInstruction* check_connection(string src, string dest);
	void update_connection(string src, string dest, BIR::SWBInstruction* instr);
	void clear();
};

}
}


#endif // __VESYLA_BENGINE_SWB_MANAGER_HPP__
