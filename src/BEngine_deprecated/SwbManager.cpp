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

#include "SwbManager.hpp"

namespace vesyla{
namespace bengine{

string SwbManager::get_signature(int row, int col, int type, int port){
	return to_string(row)+"_"+to_string(col)+"_"+to_string(type)+"_"+to_string(port);
}

BIR::SWBInstruction* SwbManager::check_connection(string src, string dest){
	return NULL;
	auto it=_swb_status.find(src+"->"+dest);
	if(it!=_swb_status.end()){
		return it->second;
	}
	return NULL;
}

void SwbManager::update_connection(string src, string dest, BIR::SWBInstruction* instr){
	_swb_status[src+"->"+dest] = instr;
}

void SwbManager::clear(){
	_swb_status.clear();
}

}
}




