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
#include "cidfg/IntPolynomialTable.hpp"

INITIALIZE_EASYLOGGINGPP

using namespace vesyla::cidfg;

BOOST_AUTO_TEST_CASE(read_write)
{
	IntPolynomialTable ipt1({{{"a"}, 1},
													 {{"b", "a"}, 2},
													 {{"b", "b"}, 3},
													 {{"c"}, 0},
													 {{}, 4}});
	string ref_str, test_str;
	ref_str = "4+1*a+2*a*b+3*b*b";
	test_str = ipt1.serialize();
	BOOST_CHECK_EQUAL(ref_str, test_str);
	IntPolynomialTable ipt2;
	ref_str = "0";
	test_str = ipt2.serialize();
	BOOST_CHECK_EQUAL(ref_str, test_str);
}

BOOST_AUTO_TEST_CASE(add_sub_mul)
{
	IntPolynomialTable ipt1({{{"a"}, 1},
													 {{"b", "a"}, 2},
													 {{"b"}, 3},
													 {{"c"}, 0},
													 {{}, 4}});
	IntPolynomialTable ipt2({{{"a"}, 2},
													 {{"b"}, 3},
													 {{"d"}, 2},
													 {{}, 9}});
	IntPolynomialTable ipt_test;
	string ref_str, test_str;
	ipt_test = ipt1;
	ipt_test.add(ipt2);
	ref_str = "13+3*a+2*a*b+6*b+2*d";
	test_str = ipt_test.serialize();
	BOOST_CHECK_EQUAL(ref_str, test_str);

	ipt_test = ipt1;
	ipt_test.sub(ipt2);
	ref_str = "-5+-1*a+2*a*b+-2*d";
	test_str = ipt_test.serialize();
	BOOST_CHECK_EQUAL(ref_str, test_str);

	ipt_test = ipt1;
	ipt_test.mul(ipt2);
	ref_str = "36+17*a+2*a*a+4*a*a*b+27*a*b+6*a*b*b+4*a*b*d+2*a*d+39*b+9*b*b+6*b*d+8*d";
	test_str = ipt_test.serialize();
	BOOST_CHECK_EQUAL(ref_str, test_str);
}

BOOST_AUTO_TEST_CASE(div_const)
{
	IntPolynomialTable ipt1({{{"a"}, 4},
													 {{"b", "a"}, 2},
													 {{"b"}, 6},
													 {{"c"}, 0},
													 {{}, 4}});
	multiset<string> s;
	IntPolynomialTable ipt2({{s, 2}});
	IntPolynomialTable ipt3({{s, 3}});
	IntPolynomialTable ipt_test;
	string ref_str, test_str;
	ipt_test = ipt1;
	ipt_test.div_const(ipt2);
	ref_str = "2+2*a+1*a*b+3*b";
	test_str = ipt_test.serialize();
	BOOST_CHECK_EQUAL(ref_str, test_str);
	ipt_test = ipt1;
	ipt_test.div_const(2);
	ref_str = "2+2*a+1*a*b+3*b";
	test_str = ipt_test.serialize();
	BOOST_CHECK_EQUAL(ref_str, test_str);

	ipt_test = ipt1;
	BOOST_CHECK_EQUAL(ipt_test.is_const_divisable(ipt3), false);
	BOOST_CHECK_EQUAL(ipt_test.is_const_divisable(3), false);
}