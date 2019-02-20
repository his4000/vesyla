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




