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
#include "schedule/Rot.hpp"

INITIALIZE_EASYLOGGINGPP

using namespace boost;
using namespace vesyla::schedule;

BOOST_AUTO_TEST_CASE(merge_rot){
	Rot rot1;
	Rot rot2;
	Rot ref;
	
	rot1.convert_from_map({{"R0", {{0,0},{2,5}}}});
	rot2.convert_from_map({});
	rot1.merge_and_verify(rot2, 0);
	ref.convert_from_map({{"R0", {{0,0},{2,5}}}});
	BOOST_CHECK_EQUAL(rot1.convert_to_map()==ref.convert_to_map(), true);
	
	rot1.convert_from_map({});
	rot2.convert_from_map({{"R0", {{0,0},{2,5}}}});
	rot1.merge_and_verify(rot2, 1);
	ref.convert_from_map({{"R0", {{1,1},{3,6}}}});
	BOOST_CHECK_EQUAL(rot1.convert_to_map()==ref.convert_to_map(), true);
	
	rot1.convert_from_map({{"R0", {{1,1},{2,5}}}});
	rot2.convert_from_map({{"R0", {{0,0},{8,9}}}});
	rot1.merge_and_verify(rot2, 0);
	ref.convert_from_map({{"R0", {{0,5},{8,9}}}});
	BOOST_CHECK_EQUAL(rot1.convert_to_map()==ref.convert_to_map(), true);
	
	rot1.convert_from_map({{"R0", {{0,0},{8,9}}}});
	rot2.convert_from_map({{"R0", {{1,1},{2,5}}}});
	rot1.merge_and_verify(rot2, 0);
	ref.convert_from_map({{"R0", {{0,5},{8,9}}}});
	BOOST_CHECK_EQUAL(rot1.convert_to_map()==ref.convert_to_map(), true);
	
	rot1.convert_from_map({{"R0", {{0,0},{8,9}}}});
	rot2.convert_from_map({{"R0", {{1,1},{2,5}}}});
	rot1.merge_and_verify(rot2, 1);
	ref.convert_from_map({{"R0", {{0,0},{2,6},{8,9}}}});
	BOOST_CHECK_EQUAL(rot1.convert_to_map()==ref.convert_to_map(), true);
	
	rot1.convert_from_map({{"R0", {{0,0},{8,9}}}});
	rot2.convert_from_map({{"R0", {{0,0},{3,5}}}});
	rot2.merge_and_verify(rot2, 6, 10);
	ref.convert_from_map({{"R0", {{0,0},{7,12}}}});
	BOOST_CHECK_EQUAL(rot1.convert_to_map()==ref.convert_to_map(), true);
	
	rot1.convert_from_map({{"R0", {{0,0},{8,9}}}, {"R1", {{0,0},{8,9}}}});
	rot2.convert_from_map({{"R0", {{0,0},{3,5}}}, {"R1", {{0,0},{3,5}}}});
	rot1.merge_and_verify(rot2, 6, 10);
	ref.convert_from_map({{"R0", {{0,0},{7,12}}}, {"R1", {{0,0},{7,12}}}});
	BOOST_CHECK_EQUAL(rot1.convert_to_map()==ref.convert_to_map(), true);

}

BOOST_AUTO_TEST_CASE(frame){
	Frame f1(INT_MIN, 0, 1);
	Frame f2(0, INT_MAX, 1);
	Frame f3(1, 3);
	BOOST_CHECK_EQUAL(f1.type(), 1);
	BOOST_CHECK_EQUAL(f2.type(), 2);
	BOOST_CHECK_EQUAL(f3.type(), 3);
	BOOST_CHECK_EQUAL(f1.pos(), 0);
	BOOST_CHECK_EQUAL(f2.pos(), 0);
	BOOST_CHECK_EQUAL(f3.pos(), 1);
	
	f1.shift(1);
	f2.shift(1);
	f3.shift(1);
	BOOST_CHECK_EQUAL(f1.type(), 1);
	BOOST_CHECK_EQUAL(f2.type(), 2);
	BOOST_CHECK_EQUAL(f3.type(), 3);
	BOOST_CHECK_EQUAL(f1.pos(), 1);
	BOOST_CHECK_EQUAL(f2.pos(), 1);
	BOOST_CHECK_EQUAL(f3.pos(), 2);
	
	BOOST_CHECK_EQUAL(f2.is_later_than(f1), false);
	BOOST_CHECK_EQUAL(f3.is_later_than(f2), true);
}

BOOST_AUTO_TEST_CASE(timetable){
	Timetable tb1;
	tb1.push_back(Frame(INT_MIN, 0, 1));
	tb1.push_back(Frame(0, INT_MAX, 1));
	tb1.push_back(Frame(1, 3));
	
	BOOST_CHECK_EQUAL(tb1.size(), 3);
	
	BOOST_CHECK_EQUAL(tb1.merge(), true);
	BOOST_CHECK_EQUAL(tb1.size(), 1);
	BOOST_CHECK_EQUAL(tb1[0].t0, 0);
	BOOST_CHECK_EQUAL(tb1[0].t1, 3);
}

BOOST_AUTO_TEST_CASE(rot){
	Rot rot1({{"R0", Timetable({Frame(0,0),Frame(8,9)})}, {"R1", Timetable({Frame(0,0),Frame(8,9)})}});
	Rot rot2({{"R0", Timetable({Frame(0,0),Frame(3,5)})}, {"R1", Timetable({Frame(0,0),Frame(3,5)})}});
	Rot rot3({{"R0", Timetable({Frame(0,0),Frame(8,9)})}, {"R1", Timetable({Frame(0,0),Frame(8,9)})}});
	Rot rot4=rot1;
	BOOST_CHECK_EQUAL(rot1.verify(), true);
	BOOST_CHECK_EQUAL(rot2.verify(), true);
	
	rot1.merge(rot2, 2);
	BOOST_CHECK_EQUAL(rot1.verify(), true);
	rot3.merge(rot2, 6);
	BOOST_CHECK_EQUAL(rot3.verify(), false);
	
	BOOST_CHECK_EQUAL(rot4.merge_and_verify(rot2, 0, 10), 1);
}
