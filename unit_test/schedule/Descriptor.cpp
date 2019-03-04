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
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <util/md5/md5.hpp>

INITIALIZE_EASYLOGGINGPP

using namespace vesyla::schedule;
namespace pt = boost::property_tree;


BOOST_AUTO_TEST_CASE(op_load_dump){
	Operation op("A", 0, {{"R0", {{0,5}}}, {"R1", {{1,3}}}}, {"C1", "C2"});
	pt::ptree tree;
	tree = op.dump();
	std::ostringstream oss;
	pt::write_xml(oss, tree);
	string xml_out_txt = oss.str();
	
	stringstream ss;
	ss << xml_out_txt;
	pt::read_xml(ss, tree);
	
	Operation op1;
	op1.load(tree);
	tree = op1.dump();
	std::ostringstream oss1;
	pt::write_xml(oss1, tree);
	string xml_out_txt1 = oss1.str();
	
	BOOST_CHECK_EQUAL(xml_out_txt, xml_out_txt1);
}

BOOST_AUTO_TEST_CASE(op_children){
	Operation op("A", 0, {{"R0", {{0,5}}}, {"R1", {{1,3}}}}, {"C1", "C2"});
	BOOST_CHECK_EQUAL(op.children0.size(), 2);
	op.add_child("C3");
	BOOST_CHECK_EQUAL(op.children0.size(), 3);
	op.add_child({"C3", "C4"});
	BOOST_CHECK_EQUAL(op.children0.size(), 4);
}

BOOST_AUTO_TEST_CASE(c_load_dump){
	Constraint c("A", "B", 2, 3);
	pt::ptree tree;
	tree = c.dump();
	std::ostringstream oss;
	pt::write_xml(oss, tree);
	string xml_out_txt = oss.str();
	
	stringstream ss;
	ss << xml_out_txt;
	pt::read_xml(ss, tree);
	
	Constraint c1;
	c1.load(tree);
	tree = c1.dump();
	std::ostringstream oss1;
	pt::write_xml(oss1, tree);
	string xml_out_txt1 = oss1.str();
	
	BOOST_CHECK_EQUAL(xml_out_txt, xml_out_txt1);
}

BOOST_AUTO_TEST_CASE(add_operation_constraint) {
	Descriptor d;
	d.add_operation(Operation("A"));
	d.add_operation(Operation("B"));
	d.add_operation(Operation("C"));
	d.add_constraint(Constraint("A", "B", 0, INT_MAX));
	d.add_constraint(Constraint("A", "C", INT_MIN, 2));
	d.add_constraint(Constraint("B", "C", 3, 3));
	BOOST_CHECK_EQUAL(d.count_operations(), 3);
	BOOST_CHECK_EQUAL(d.count_constraints(), 3);
}

BOOST_AUTO_TEST_CASE(get_operation_constraint) {
	Descriptor d;
	d.add_operation(Operation("A", 12));
	d.add_operation(Operation("B", 10));
	d.add_operation(Operation("C", 3));
	d.add_constraint(Constraint("A", "B", 0, INT_MAX));
	d.add_constraint(Constraint("A", "C", INT_MIN, 2));
	d.add_constraint(Constraint("B", "C", 3, 3));
	Operation op1 = d.get_operation("A");
	BOOST_CHECK_EQUAL(op1.scheduled_time, 12);
	op1.scheduled_time = 13;
	Operation& op2 = d.get_mutable_operation("A");
	BOOST_CHECK_EQUAL(op2.scheduled_time, 12);
	op2.scheduled_time = 14;
	Operation op3 = d.get_operation("A");
	BOOST_CHECK_EQUAL(op3.scheduled_time, 14);
	
	Constraint c1 = d.get_constraint("A", "B");
	BOOST_CHECK_EQUAL(c1.d_lo, 0);
	c1.d_lo=1;
	Constraint& c2 = d.get_mutable_constraint("A", "B");
	BOOST_CHECK_EQUAL(c2.d_lo, 0);
	c2.d_lo=2;
	Constraint c3 = d.get_constraint("A", "B");
	BOOST_CHECK_EQUAL(c3.d_lo, 2);
	
	BOOST_CHECK_EQUAL(d.get_src_constraints("C").size(), 2);
	BOOST_CHECK_EQUAL(d.get_dest_constraints("B").size(), 1);
}

BOOST_AUTO_TEST_CASE(remove_constraint) {
	Descriptor d;
	d.add_operation(Operation("A", 12));
	d.add_operation(Operation("B", 10));
	d.add_operation(Operation("C", 3));
	d.add_constraint(Constraint("A", "B", 0, INT_MAX));
	d.add_constraint(Constraint("A", "C", INT_MIN, 2));
	d.add_constraint(Constraint("B", "C", 3, 3));
	BOOST_CHECK_EQUAL(d.count_constraints(), 3);
	d.remove_constraint("A", "C");
	BOOST_CHECK_EQUAL(d.count_constraints(), 2);
}


BOOST_AUTO_TEST_CASE(load_dump_descriptor) {
	Descriptor d("main");
	d.add_operation(Operation("OP1", 0, {{"AGU", {{2,5}}},{"DPU", {{7,9}}}}, {"a", "b"}));
	d.add_operation(Operation("OP2", 0, {{"AGU", {{2,5}}},{"DPU", {{7,9}}}}, {"a", "b"}));
	d.add_constraint(Constraint("OP1", "OP2", 3, 6));
	d.add_constraint(Constraint("OP1", "OP2", 2, INT_MAX));
	
	pt::ptree tree;
	tree = d.dump();
	std::ostringstream oss;
	pt::write_xml(oss, tree);
	string xml_out_txt = oss.str();
	
	stringstream ss;
	ss << xml_out_txt;
	pt::read_xml(ss, tree);
	
	Descriptor d1;
	d1.load(tree);
	tree = d1.dump();
	std::ostringstream oss1;
	pt::write_xml(oss1, tree);
	string xml_out_txt1 = oss1.str();
	
	cout << xml_out_txt << endl;
	cout << xml_out_txt1 << endl;
	BOOST_CHECK_EQUAL(xml_out_txt, xml_out_txt1);
}

