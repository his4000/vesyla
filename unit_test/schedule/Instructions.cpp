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

#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "util/Common.hpp"
#include "schedule/Descriptor.hpp"
#include "VManager/MemoryManager.hpp"
#include "VIR/VIRIncludeAll.hpp"
#include "BIR/BIRIncludeAll.hpp"
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <util/md5/md5.hpp>

INITIALIZE_EASYLOGGINGPP

using namespace vesyla::schedule;
namespace pt = boost::property_tree;
using namespace BIR;
using namespace BIR::BIREnumerations;

BOOST_AUTO_TEST_CASE(wait_instruction_load_dump){
	WaitInstruction *waitInst = CREATE_OBJECT_B(WaitInstruction);
	//waitInst->id(2);
	waitInst->labels.push_back("One");
	waitInst->labels.push_back("Two");
	waitInst->labels.push_back("Three");

	waitInst->numberOfCycles(1,true);

	pt::ptree tree;
        tree = waitInst->dump();

	std::ostringstream oss;
	pt::write_xml(oss, tree);
	string xml_out_txt = oss.str();
	stringstream ss;
	ss << xml_out_txt;


	pt::read_xml(ss, tree);
	WaitInstruction *waitInst_load = CREATE_OBJECT_B(WaitInstruction);
	waitInst_load->load(tree);

        BOOST_CHECK_EQUAL(waitInst_load->kindStr(), "WaitInstruction");
        //BOOST_CHECK_EQUAL(waitInst_load->id(),waitInst->id());
	BOOST_CHECK_EQUAL(waitInst_load->labels[0],"One");
	BOOST_CHECK_EQUAL(waitInst_load->labels[1],"Two");
	BOOST_CHECK_EQUAL(waitInst_load->labels[2],"Three");
	BOOST_CHECK_EQUAL(waitInst_load->numberOfCycles.value,1);
	BOOST_CHECK_EQUAL(waitInst_load->numberOfCycles.isStatic,true);
}

BOOST_AUTO_TEST_CASE(swb_instruction_load_dump){
	SWBInstruction *swbInst = CREATE_OBJECT_B(SWBInstruction);

	//swbInst->id(3);
	swbInst->labels.push_back("O");
	swbInst->labels.push_back("T");
	swbInst->labels.push_back("Th");

	swbInst->variableName = "my_variable";

        swbInst->source.type = ctRegFile;
        swbInst->source.portNo=1;
        swbInst->source.cellCoordinate.row=1;
	swbInst->source.cellCoordinate.column=0;
	swbInst->source.cellCoordinate.bank=0;
        swbInst->source.statementNo=102;


        swbInst->destination.type=ctDPU;
        swbInst->destination.portNo=1;
        swbInst->destination.cellCoordinate.row=1;
	swbInst->destination.cellCoordinate.column=1;
	swbInst->destination.cellCoordinate.bank=1;
        swbInst->destination.statementNo=103;

        swbInst->dependentTo=swbInst;

	pt::ptree tree;
        tree = swbInst->dump();

	std::ostringstream oss;
	pt::write_xml(oss, tree);
	string xml_out_txt = oss.str();
	stringstream ss;
	ss << xml_out_txt;

	pt::read_xml(ss, tree);

	SWBInstruction *swbInst_load = CREATE_OBJECT_B(SWBInstruction);
	swbInst_load->load(tree);

        BOOST_CHECK_EQUAL(swbInst_load->kindStr(), "SWBInstruction");
        //BOOST_CHECK_EQUAL(swbInst_load->id(), 3);
	BOOST_CHECK_EQUAL(swbInst_load->labels[0],"O");
	BOOST_CHECK_EQUAL(swbInst_load->labels[1],"T");
	BOOST_CHECK_EQUAL(swbInst_load->labels[2],"Th");

	BOOST_CHECK_EQUAL(swbInst_load->variableName, "my_variable");

        BOOST_CHECK_EQUAL(swbInst_load->source.type,ctRegFile);
        BOOST_CHECK_EQUAL(swbInst_load->source.portNo, 1);
        BOOST_CHECK_EQUAL(swbInst_load->source.cellCoordinate.row, 1);
        BOOST_CHECK_EQUAL(swbInst_load->source.cellCoordinate.column, 0);
        BOOST_CHECK_EQUAL(swbInst_load->source.cellCoordinate.bank, 0);
        BOOST_CHECK_EQUAL(swbInst_load->source.statementNo, 102);

        BOOST_CHECK_EQUAL(swbInst_load->destination.type, ctDPU);
        BOOST_CHECK_EQUAL(swbInst_load->destination.portNo, 1);
        BOOST_CHECK_EQUAL(swbInst_load->destination.cellCoordinate.row, 1);
        BOOST_CHECK_EQUAL(swbInst_load->destination.cellCoordinate.column, 1);
        BOOST_CHECK_EQUAL(swbInst_load->destination.cellCoordinate.bank,1);
        BOOST_CHECK_EQUAL(swbInst_load->destination.statementNo,103);
	BOOST_CHECK_EQUAL(swbInst_load->dependentTo,(BIR::SWBInstruction*)swbInst->id());
}

BOOST_AUTO_TEST_CASE(sram_instruction_load_dump){
	SRAMInstruction *inst = CREATE_OBJECT_B(SRAMInstruction);

	//inst->id(4);
	inst->labels.push_back("O");
	inst->labels.push_back("T");
	inst->labels.push_back("Th");

	inst->initialAddress(1,true);
	inst->initialDelay(2,true);
	inst->loop1Iterations(3,false);

        inst->loop1Increment=4;

	inst->loop1Delay(1,true);
	inst->loop2Iterations(1,false);
	inst->loop2Delay(1,false);

	inst->loop2Increment=5;
	inst->isRead = false;
	inst->executionCycle = 6;
	inst->hopNumber = 7;

	BIR::RefiInstruction *refiInst = CREATE_OBJECT_B(RefiInstruction);
	BIR::RouteInstruction *routeInst = CREATE_OBJECT_B(RouteInstruction);

 	//refiInst->id(10);
	//routeInst->id(11);

	inst->correspondingRefiInst = refiInst;
	inst->correspondingRouteInst = routeInst;

	pt::ptree tree;
        tree = inst->dump();
	std::ostringstream oss;
	pt::write_xml(oss, tree);
	string xml_out_txt = oss.str();
	stringstream ss;
	ss << xml_out_txt;

	pt::read_xml(ss, tree);

	SRAMInstruction *Inst_load = CREATE_OBJECT_B(SRAMInstruction);
	Inst_load->load(tree);

        BOOST_CHECK_EQUAL(Inst_load->kindStr(), "SRAMInstruction");
        //BOOST_CHECK_EQUAL(Inst_load->id(), 4);
	BOOST_CHECK_EQUAL(Inst_load->labels[0],"O");
	BOOST_CHECK_EQUAL(Inst_load->labels[1],"T");
	BOOST_CHECK_EQUAL(Inst_load->labels[2],"Th");

	BOOST_CHECK_EQUAL(Inst_load->initialAddress.value, 1);
	BOOST_CHECK_EQUAL(Inst_load->initialAddress.isStatic,true);

	BOOST_CHECK_EQUAL(Inst_load->initialDelay.value, 2);
	BOOST_CHECK_EQUAL(Inst_load->initialDelay.isStatic,true);

	BOOST_CHECK_EQUAL(Inst_load->loop1Iterations.value, 3);
	BOOST_CHECK_EQUAL(Inst_load->loop1Iterations.isStatic,false);

	BOOST_CHECK_EQUAL(Inst_load->loop1Increment,4);


	BOOST_CHECK_EQUAL(Inst_load->loop1Delay.value, 1);
	BOOST_CHECK_EQUAL(Inst_load->loop1Delay.isStatic,true);

	BOOST_CHECK_EQUAL(Inst_load->loop2Iterations.value, 1);
	BOOST_CHECK_EQUAL(Inst_load->loop2Iterations.isStatic,false);

	BOOST_CHECK_EQUAL(Inst_load->loop2Delay.value, 1);
	BOOST_CHECK_EQUAL(Inst_load->loop2Delay.isStatic,false);

	BOOST_CHECK_EQUAL(Inst_load->loop2Increment,5);
	BOOST_CHECK_EQUAL(Inst_load->isRead,false);
	BOOST_CHECK_EQUAL(Inst_load->executionCycle,6);
	BOOST_CHECK_EQUAL(Inst_load->hopNumber,7);

	BOOST_CHECK_EQUAL(Inst_load->correspondingRefiInst,(BIR::RefiInstruction *)refiInst->id());
	BOOST_CHECK_EQUAL(Inst_load->correspondingRouteInst,(BIR::RouteInstruction *)routeInst->id());


}

BOOST_AUTO_TEST_CASE(route_instruction_load_dump){
	RouteInstruction *inst = CREATE_OBJECT_B(RouteInstruction);

	//inst->id(3);
	inst->labels.push_back("OM");
	inst->drraSelect = 1;
	inst->isFromSource = true;

        inst->sourceCoordinate.row=1;
	inst->sourceCoordinate.column=0;
	inst->sourceCoordinate.bank=0;

        inst->destCoordinate.row=1;
	inst->destCoordinate.column=1;
	inst->destCoordinate.bank=1;


	pt::ptree tree;
        tree = inst->dump();
	std::ostringstream oss;
	pt::write_xml(oss, tree);
	string xml_out_txt = oss.str();
	stringstream ss;
	ss << xml_out_txt;

	pt::read_xml(ss, tree);

	RouteInstruction *Inst_load = CREATE_OBJECT_B(RouteInstruction);
	Inst_load->load(tree);

        BOOST_CHECK_EQUAL(Inst_load->kindStr(), "RouteInstruction");
        //BOOST_CHECK_EQUAL(Inst_load->id(), 3);
	BOOST_CHECK_EQUAL(Inst_load->labels[0],"OM");


	BOOST_CHECK_EQUAL(Inst_load->drraSelect, 1);
	BOOST_CHECK_EQUAL(Inst_load->isFromSource,true);

        BOOST_CHECK_EQUAL(Inst_load->sourceCoordinate.row, 1);
        BOOST_CHECK_EQUAL(Inst_load->sourceCoordinate.column, 0);
        BOOST_CHECK_EQUAL(Inst_load->sourceCoordinate.bank, 0);


        BOOST_CHECK_EQUAL(Inst_load->destCoordinate.row, 1);
        BOOST_CHECK_EQUAL(Inst_load->destCoordinate.column, 1);
        BOOST_CHECK_EQUAL(Inst_load->destCoordinate.bank,1);


}

BOOST_AUTO_TEST_CASE(rfile_instruction_load_dump){
	RFILEInstruction *inst = CREATE_OBJECT_B(RFILEInstruction);
	//inst->id(5);
	inst->labels.push_back("One");
	inst->labels.push_back("Two");
	inst->labels.push_back("Three");


	inst->initialDelay = 11;
	inst->middleDelay = 12;
	inst->startAddress = 2;
	inst->endAddress = 21;
	inst->numberOfRepetition = 2;
	inst->repetitionDelay = 17;
	inst->repetitionOffset = 19;
	inst->increment = 1;
	inst->isRead = true;
	inst->drraRow = 1;
	inst->addressMode = amLinear;

	pt::ptree tree;
        tree = inst->dump();
	std::ostringstream oss;
	pt::write_xml(oss, tree);
	string xml_out_txt = oss.str();
	stringstream ss;
	ss << xml_out_txt;


	pt::read_xml(ss, tree);
	RFILEInstruction *inst_load = CREATE_OBJECT_B(RFILEInstruction);
	inst_load->load(tree);

        BOOST_CHECK_EQUAL(inst_load->kindStr(), "RFILEInstruction");
        //BOOST_CHECK_EQUAL(inst_load->id(), 5);
	BOOST_CHECK_EQUAL(inst_load->labels[0],"One");
	BOOST_CHECK_EQUAL(inst_load->labels[1],"Two");
	BOOST_CHECK_EQUAL(inst_load->labels[2],"Three");

	BOOST_CHECK_EQUAL(inst_load->initialDelay,11);
	BOOST_CHECK_EQUAL(inst_load->middleDelay,12);
	BOOST_CHECK_EQUAL(inst_load->startAddress,2);
	BOOST_CHECK_EQUAL(inst_load->endAddress,21);
	BOOST_CHECK_EQUAL(inst_load->numberOfRepetition,2);
	BOOST_CHECK_EQUAL(inst_load->repetitionDelay,17);
	BOOST_CHECK_EQUAL(inst_load->repetitionOffset,19);
	BOOST_CHECK_EQUAL(inst_load->increment,1);
	BOOST_CHECK_EQUAL(inst_load->isRead,true);
	BOOST_CHECK_EQUAL(inst_load->drraRow,1);
	BOOST_CHECK_EQUAL(inst_load->addressMode,amLinear);
}

BOOST_AUTO_TEST_CASE(Refi_instruction_load_dump){
	RefiInstruction *inst = CREATE_OBJECT_B(RefiInstruction);
	//inst->id(6);
	inst->labels.push_back("One");
	inst->labels.push_back("Two");
	inst->labels.push_back("Three");


	inst->initialDelay(1,true);
	inst->middleDelay(12,false);
	inst->numberOfAddress(1,true);
	inst->numberOfRepetition(12,false);
	inst->portNo = 2;
	inst->repetitionDelay(1,true);
	inst->repetitionOffset = 3;
	inst->startAddress(2,true);
	inst->stepValue(5,true);
	inst->addressMode = amBitReverse;
	inst->portMode = pmOut;

	BIR::SWBInstruction *inst3 = CREATE_OBJECT_B(SWBInstruction);
	//inst3->id(23);
	inst->correspondingSWBInsts.push_back(inst3);

	inst->dependentTo = inst;
	inst->reverse_bits(1,true);

	BIR::DPUInstruction *inst1 = CREATE_OBJECT_B(DPUInstruction);
	//inst1->id(22);
	inst->correspondingDpuInst = inst1;

	BIR::RefiInstruction *inst2 = CREATE_OBJECT_B(RefiInstruction);
	//inst2->id(24);
	inst->correspondingRefiTransferInst = inst2;

	inst->bankNo = 112;

	pt::ptree tree;
        tree = inst->dump();
	std::ostringstream oss;
	pt::write_xml(oss, tree);
	string xml_out_txt = oss.str();
	stringstream ss;
	ss << xml_out_txt;


	pt::read_xml(ss, tree);
	RefiInstruction *inst_load = CREATE_OBJECT_B(RefiInstruction);
	inst_load->load(tree);

        BOOST_CHECK_EQUAL(inst_load->kindStr(), "RefiInstruction");
        //BOOST_CHECK_EQUAL(inst_load->id(), 6);
	BOOST_CHECK_EQUAL(inst_load->labels[0],"One");
	BOOST_CHECK_EQUAL(inst_load->labels[1],"Two");
	BOOST_CHECK_EQUAL(inst_load->labels[2],"Three");
	BOOST_CHECK_EQUAL(inst_load->initialDelay.value,1);
	BOOST_CHECK_EQUAL(inst_load->initialDelay.isStatic,true);
	BOOST_CHECK_EQUAL(inst_load->middleDelay.value,12);
	BOOST_CHECK_EQUAL(inst_load->middleDelay.isStatic,false);
	BOOST_CHECK_EQUAL(inst_load->numberOfAddress.value,1);
	BOOST_CHECK_EQUAL(inst_load->numberOfAddress.isStatic,true);
	BOOST_CHECK_EQUAL(inst_load->numberOfRepetition.value,12);
	BOOST_CHECK_EQUAL(inst_load->numberOfRepetition.isStatic,false);
	BOOST_CHECK_EQUAL(inst_load->portNo,2);
	BOOST_CHECK_EQUAL(inst_load->repetitionDelay.value,1);
	BOOST_CHECK_EQUAL(inst_load->repetitionDelay.isStatic,true);
	BOOST_CHECK_EQUAL(inst_load->repetitionOffset,3);
	BOOST_CHECK_EQUAL(inst_load->startAddress.value,2);
	BOOST_CHECK_EQUAL(inst_load->startAddress.isStatic,true);
	BOOST_CHECK_EQUAL(inst_load->stepValue.value,5);
	BOOST_CHECK_EQUAL(inst_load->stepValue.isStatic,true);
	BOOST_CHECK_EQUAL(inst_load->addressMode,amBitReverse);
	BOOST_CHECK_EQUAL(inst_load->portMode,pmOut);

	BOOST_CHECK_EQUAL(inst_load->correspondingSWBInsts[0],(BIR::SWBInstruction *)inst3->id());
	BOOST_CHECK_EQUAL(inst_load->correspondingDpuInst,(BIR::DPUInstruction *)inst1->id());
	BOOST_CHECK_EQUAL(inst_load->correspondingRefiTransferInst,(BIR::RefiInstruction *)inst2->id());

	BOOST_CHECK_EQUAL(inst_load->bankNo,112);
}

BOOST_AUTO_TEST_CASE(raccu_instruction_load_dump){
	RACCUInstruction *inst = CREATE_OBJECT_B(RACCUInstruction);
	//inst->id(7);
	inst->labels.push_back("One");
	inst->labels.push_back("Two");
	inst->labels.push_back("Three");

	inst->raccuMode = rmAddWithLoopIndex;
	inst->resultAddress = 4;
	inst->dependentTo = inst;

	inst->operand1(1,true);
	inst->operand2(12,false);

	pt::ptree tree;
        tree = inst->dump();
	std::ostringstream oss;
	pt::write_xml(oss, tree);
	string xml_out_txt = oss.str();
	stringstream ss;
	ss << xml_out_txt;


	pt::read_xml(ss, tree);
	RACCUInstruction *inst_load = CREATE_OBJECT_B(RACCUInstruction);
	inst_load->load(tree);

        BOOST_CHECK_EQUAL(inst_load->kindStr(), "RACCUInstruction");
        //BOOST_CHECK_EQUAL(inst_load->id(), inst->id());
	BOOST_CHECK_EQUAL(inst_load->labels[0],"One");
	BOOST_CHECK_EQUAL(inst_load->labels[1],"Two");
	BOOST_CHECK_EQUAL(inst_load->labels[2],"Three");

	BOOST_CHECK_EQUAL(inst_load->raccuMode,rmAddWithLoopIndex);
	BOOST_CHECK_EQUAL(inst_load->resultAddress,4);
	BOOST_CHECK_EQUAL(inst_load->dependentTo,(BIR::Instruction *)inst->id());

	BOOST_CHECK_EQUAL(inst_load->operand1.value,1);
	BOOST_CHECK_EQUAL(inst_load->operand1.isStatic,true);

	BOOST_CHECK_EQUAL(inst_load->operand2.value,12);
	BOOST_CHECK_EQUAL(inst_load->operand2.isStatic,false);
}

BOOST_AUTO_TEST_CASE(loopTail_instruction_load_dump){
	LoopTailInstruction *inst = CREATE_OBJECT_B(LoopTailInstruction);
	//inst->id(8);
	inst->labels.push_back("One");
	inst->labels.push_back("Two");
	inst->labels.push_back("Three");

	inst->loopId = 12;
	inst->stepValue = 4;
	inst->gotoPC = 102;

	Instruction *inst1 = CREATE_OBJECT_B(LoopHeaderInstruction);
	//inst1->id(2);
	inst->corresponding_loop_header_instr=inst1;

	pt::ptree tree;
        tree = inst->dump();
	std::ostringstream oss;
	pt::write_xml(oss, tree);
	string xml_out_txt = oss.str();
	stringstream ss;
	ss << xml_out_txt;


	pt::read_xml(ss, tree);
	LoopTailInstruction *inst_load = CREATE_OBJECT_B(LoopTailInstruction);
	inst_load->load(tree);

        BOOST_CHECK_EQUAL(inst_load->kindStr(), "LoopTailInstruction");
        //BOOST_CHECK_EQUAL(inst_load->id(), inst->id());
	BOOST_CHECK_EQUAL(inst_load->labels[0],"One");
	BOOST_CHECK_EQUAL(inst_load->labels[1],"Two");
	BOOST_CHECK_EQUAL(inst_load->labels[2],"Three");

	BOOST_CHECK_EQUAL(inst_load->loopId,12);
	BOOST_CHECK_EQUAL(inst_load->stepValue,4);
	BOOST_CHECK_EQUAL(inst_load->gotoPC,102);

	BOOST_CHECK_EQUAL(inst_load->corresponding_loop_header_instr,(Instruction*)inst1->id());

}

BOOST_AUTO_TEST_CASE(loopHeader_instruction_load_dump){
	LoopHeaderInstruction *inst = CREATE_OBJECT_B(LoopHeaderInstruction);
	//inst->id(9);
	inst->labels.push_back("One");
	inst->labels.push_back("Two");
	inst->labels.push_back("Three");

	inst->loopId = 21;
	inst->startValue = 21;
	inst->isStatic = false;
	inst->iterationNumber = 11;
	inst->loopExecution = 3;

	LoopTailInstruction *inst1 = CREATE_OBJECT_B(LoopTailInstruction);
	//inst1->id(23);
	inst->correspondingLoopTailInst=inst1;

	LoopHeaderInstruction *inst2 = CREATE_OBJECT_B(LoopHeaderInstruction);
	//inst2->id(26);
	inst->upperLoop=inst2;

	LoopHeaderInstruction *inst3 = CREATE_OBJECT_B(LoopHeaderInstruction);
	//inst3->id(27);
	inst->innerLoops.push_back(inst3);

	LoopHeaderInstruction *inst4 = CREATE_OBJECT_B(LoopHeaderInstruction);
	//inst4->id(28);
	inst->innerLoops.push_back(inst4);

	pt::ptree tree;
        tree = inst->dump();
	std::ostringstream oss;
	pt::write_xml(oss, tree);
	string xml_out_txt = oss.str();
	stringstream ss;
	ss << xml_out_txt;


	pt::read_xml(ss, tree);
	LoopHeaderInstruction *inst_load = CREATE_OBJECT_B(LoopHeaderInstruction);
	inst_load->load(tree);

        BOOST_CHECK_EQUAL(inst_load->kindStr(), "LoopHeaderInstruction");
        //BOOST_CHECK_EQUAL(inst_load->id(), inst->id());
	BOOST_CHECK_EQUAL(inst_load->labels[0],"One");
	BOOST_CHECK_EQUAL(inst_load->labels[1],"Two");
	BOOST_CHECK_EQUAL(inst_load->labels[2],"Three");

	BOOST_CHECK_EQUAL(inst_load->loopId,21);
	BOOST_CHECK_EQUAL(inst_load->startValue,21);
	BOOST_CHECK_EQUAL(inst_load->isStatic,false);
	BOOST_CHECK_EQUAL(inst_load->iterationNumber,11);
	BOOST_CHECK_EQUAL(inst_load->loopExecution,3);

	BOOST_CHECK_EQUAL(inst_load->correspondingLoopTailInst,(LoopTailInstruction*)inst1->id());
	BOOST_CHECK_EQUAL(inst_load->upperLoop,(LoopHeaderInstruction*)inst2->id());

	BOOST_CHECK_EQUAL(inst_load->innerLoops[0],(LoopHeaderInstruction*)inst3->id());
	BOOST_CHECK_EQUAL(inst_load->innerLoops[1],(LoopHeaderInstruction*)inst4->id());

}


BOOST_AUTO_TEST_CASE(jump_instruction_load_dump){
	JumpInstruction *inst = CREATE_OBJECT_B(JumpInstruction);
	//inst->id(10);
	inst->labels.push_back("One");
	inst->labels.push_back("Two");
	inst->labels.push_back("Three");

	inst->jump_pc = 100;
	inst->jumpAddressInstruction = inst;

	pt::ptree tree;
        tree = inst->dump();

	std::ostringstream oss;
	pt::write_xml(oss, tree);
	string xml_out_txt = oss.str();
	stringstream ss;
	ss << xml_out_txt;


	pt::read_xml(ss, tree);
	JumpInstruction *inst_load = CREATE_OBJECT_B(JumpInstruction);
	inst_load->load(tree);

        BOOST_CHECK_EQUAL(inst_load->kindStr(), "JumpInstruction");
        //BOOST_CHECK_EQUAL(inst_load->id(), inst->id());
	BOOST_CHECK_EQUAL(inst_load->labels[0],"One");
	BOOST_CHECK_EQUAL(inst_load->labels[1],"Two");
	BOOST_CHECK_EQUAL(inst_load->labels[2],"Three");

	BOOST_CHECK_EQUAL(inst_load->jump_pc,100);
	BOOST_CHECK_EQUAL(inst_load->jumpAddressInstruction,(Instruction*)inst->id());

}


BOOST_AUTO_TEST_CASE(dpu_instruction_load_dump){
	DPUInstruction *inst = CREATE_OBJECT_B(DPUInstruction);
	//inst->id(11);
	inst->labels.push_back("One");
	inst->labels.push_back("Two");
	inst->labels.push_back("Three");
	inst->mode = "Mode";
	inst->modeValue = 11;
	inst->executionCycle = 15;
	inst->repetition = 29;


	inst->constantValue = 2;
	inst->saturation = true;
	inst->fixedPointMode = false;
	inst->parameter_type = dptReference;
	inst->ioChange = dicIn2Negation;

	DPUInstruction *inst1 = CREATE_OBJECT_B(DPUInstruction);
	//inst1->id(23);
	inst->correspondingChainedDpus.push_back(inst1);

	DPUInstruction *inst2 = CREATE_OBJECT_B(DPUInstruction);
	//inst2->id(24);
	inst->correspondingChainedDpus.push_back(inst2);

	RefiInstruction *inst3 = CREATE_OBJECT_B(RefiInstruction);
	//inst3->id(25);
	inst->correspondingRefiInsts.push_back(inst3);

	RefiInstruction *inst4 = CREATE_OBJECT_B(RefiInstruction);
	//inst4->id(26);
	inst->correspondingRefiInsts.push_back(inst4);

	SWBInstruction *inst5 = CREATE_OBJECT_B(SWBInstruction);
	//inst5->id(27);
	inst->correspondingSWBInsts.push_back(inst5);

	SWBInstruction *inst6 = CREATE_OBJECT_B(SWBInstruction);
	//inst6->id(28);
	inst->correspondingSWBInsts.push_back(inst6);

	pt::ptree tree;
        tree = inst->dump();

	std::ostringstream oss;
	pt::write_xml(oss, tree);
	string xml_out_txt = oss.str();
	stringstream ss;
	ss << xml_out_txt;


	pt::read_xml(ss, tree);
	DPUInstruction *inst_load = CREATE_OBJECT_B(DPUInstruction);
	inst_load->load(tree);

        BOOST_CHECK_EQUAL(inst_load->kindStr(), "DPUInstruction");
        //BOOST_CHECK_EQUAL(inst_load->id(),inst->id());
	BOOST_CHECK_EQUAL(inst_load->labels[0],"One");
	BOOST_CHECK_EQUAL(inst_load->labels[1],"Two");
	BOOST_CHECK_EQUAL(inst_load->labels[2],"Three");
	BOOST_CHECK_EQUAL(inst_load->mode,"Mode");
	BOOST_CHECK_EQUAL(inst_load->modeValue,11);
	BOOST_CHECK_EQUAL(inst_load->executionCycle,15);
	BOOST_CHECK_EQUAL(inst_load->repetition,29);

	BOOST_CHECK_EQUAL(inst_load->correspondingChainedDpus[0],(DPUInstruction *)inst1->id());
	BOOST_CHECK_EQUAL(inst_load->correspondingChainedDpus[1],(DPUInstruction *)inst2->id());

	BOOST_CHECK_EQUAL(inst_load->correspondingRefiInsts[0],(RefiInstruction *)inst3->id());
	BOOST_CHECK_EQUAL(inst_load->correspondingRefiInsts[1],(RefiInstruction *)inst4->id());

	BOOST_CHECK_EQUAL(inst_load->correspondingSWBInsts[0],(SWBInstruction *)inst5->id());
	BOOST_CHECK_EQUAL(inst_load->correspondingSWBInsts[1],(SWBInstruction *)inst6->id());

	BOOST_CHECK_EQUAL(inst_load->ioChange,dicIn2Negation);
	BOOST_CHECK_EQUAL(inst_load->constantValue,2);
	BOOST_CHECK_EQUAL(inst_load->saturation,true);
	BOOST_CHECK_EQUAL(inst_load->fixedPointMode,false);
	BOOST_CHECK_EQUAL(inst_load->parameter_type,dptReference);

}


BOOST_AUTO_TEST_CASE(branch_instruction_load_dump){
	BranchInstruction *inst = CREATE_OBJECT_B(BranchInstruction);
	//inst->id(12);
	inst->labels.push_back("One");
	inst->labels.push_back("Two");
	inst->labels.push_back("Three");
	inst->mode = bmGreater;
	inst->modeValue = 11;
	inst->falseAddressInstruction = inst;
	inst->false_pc = 20;

	pt::ptree tree;
        tree = inst->dump();

	std::ostringstream oss;
	pt::write_xml(oss, tree);
	string xml_out_txt = oss.str();
	stringstream ss;
	ss << xml_out_txt;


	pt::read_xml(ss, tree);
	BranchInstruction *inst_load = CREATE_OBJECT_B(BranchInstruction);
	inst_load->load(tree);

        BOOST_CHECK_EQUAL(inst_load->kindStr(), "BranchInstruction");
        //BOOST_CHECK_EQUAL(inst_load->id(),inst->id());
	BOOST_CHECK_EQUAL(inst_load->labels[0],"One");
	BOOST_CHECK_EQUAL(inst_load->labels[1],"Two");
	BOOST_CHECK_EQUAL(inst_load->labels[2],"Three");
	BOOST_CHECK_EQUAL(inst_load->mode,bmGreater);
	BOOST_CHECK_EQUAL(inst_load->modeValue,11);
	BOOST_CHECK_EQUAL(inst_load->falseAddressInstruction,(BIR::Instruction *)inst->id());
	BOOST_CHECK_EQUAL(inst_load->false_pc,20);
}


