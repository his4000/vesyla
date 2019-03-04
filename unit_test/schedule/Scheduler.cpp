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
#include "schedule/Scheduler.hpp"
#include "util/md5/md5.hpp"
INITIALIZE_EASYLOGGINGPP

using namespace boost;
using namespace vesyla::schedule;

BOOST_AUTO_TEST_CASE(pack_hard_links)
{
	Scheduler s;

	Graph g0;
	auto a = add_vertex(Operation("A", 0, {{"R0", {{0, 5}}}, {"R1", {{1, 3}}}}), g0);
	auto b = add_vertex(Operation("B", 0, {{"R0", {{5, 5}}}, {"R1", {{3, 3}}}}), g0);
	auto c = add_vertex(Operation("C", 0, {{"R0", {{4, 7}}}}), g0);
	auto d = add_vertex(Operation("D", 0, {{"R0", {{4, 7}}}}), g0);
	add_edge(a, b, Constraint("A", "B", 1, 1), g0);
	add_edge(a, c, Constraint("A", "C", 3, 3), g0);
	add_edge(b, c, Constraint("B", "C", 2, 2), g0);
	add_edge(a, d, Constraint("A", "D", 0, INT_MAX), g0);
	add_edge(b, d, Constraint("B", "D", 1, INT_MAX), g0);

	Graph g1 = s.pack_hard_links(g0);
	BOOST_CHECK_EQUAL(num_vertices(g1), 2);
	BOOST_CHECK_EQUAL(num_edges(g1), 2);
	vector<int> v_lo;
	graph_traits<Graph>::edge_iterator ei, ei_end;
	for (tie(ei, ei_end) = edges(g1); ei != ei_end; ++ei)
	{
		BOOST_CHECK_EQUAL(g1[*ei].d_hi, INT_MAX);
		v_lo.push_back(g1[*ei].d_lo);
	}
	BOOST_CHECK_EQUAL(v_lo[0] + v_lo[1], 2);
	BOOST_CHECK_EQUAL(v_lo[0] * v_lo[1], 0);
}

BOOST_AUTO_TEST_CASE(remove_redundent_links)
{
	Scheduler s;

	Graph g0;
	auto a = add_vertex(Operation("A"), g0);
	auto b = add_vertex(Operation("B"), g0);
	auto c = add_vertex(Operation("C"), g0);
	auto d = add_vertex(Operation("D"), g0);
	add_edge(a, b, Constraint("A", "B", 1, 1), g0);
	add_edge(a, b, Constraint("A", "B", 1, INT_MAX), g0);
	add_edge(a, c, Constraint("A", "C", 3, 3), g0);
	add_edge(b, c, Constraint("B", "C", 2, 2), g0);
	add_edge(a, d, Constraint("A", "D", 2, INT_MAX), g0);
	add_edge(b, d, Constraint("B", "D", 0, INT_MAX), g0);

	Graph g1 = s.remove_redundent_links(g0);
	BOOST_CHECK_EQUAL(num_vertices(g1), 4);
	BOOST_CHECK_EQUAL(num_edges(g1), 4);
	BOOST_CHECK_EQUAL(out_degree(0, g1), 2);
	BOOST_CHECK_EQUAL(out_degree(1, g1), 2);
	BOOST_CHECK_EQUAL(out_degree(2, g1), 0);
	BOOST_CHECK_EQUAL(out_degree(3, g1), 0);
}

BOOST_AUTO_TEST_CASE(vector_mul)
{
	// C[1:8] = A[1:8] .* B[1:8]

	Scheduler s;

	Graph g0;
	auto swb_a_0 = add_vertex(Operation("SWB_A_0"), g0);
	auto swb_a_1 = add_vertex(Operation("SWB_A_1"), g0);
	auto swb_a_2 = add_vertex(Operation("SWB_A_2"), g0);
	auto refi_a_0 = add_vertex(Operation("REFI_A_0"), g0);
	auto refi_a_1 = add_vertex(Operation("REFI_A_1"), g0);
	auto refi_a_2 = add_vertex(Operation("REFI_A_2"), g0);
	auto swb_b_0 = add_vertex(Operation("SWB_B_0"), g0);
	auto swb_b_1 = add_vertex(Operation("SWB_B_1"), g0);
	auto swb_b_2 = add_vertex(Operation("SWB_B_2"), g0);
	auto refi_b_0 = add_vertex(Operation("REFI_B_0"), g0);
	auto refi_b_1 = add_vertex(Operation("REFI_B_1"), g0);
	auto refi_b_2 = add_vertex(Operation("REFI_B_2"), g0);
	auto dpu_0 = add_vertex(Operation("DPU_0"), g0);
	auto dpu_1 = add_vertex(Operation("DPU_1"), g0);
	auto dpu_2 = add_vertex(Operation("DPU_2"), g0);
	auto swb_c_0 = add_vertex(Operation("SWB_C_0"), g0);
	auto swb_c_1 = add_vertex(Operation("SWB_C_1"), g0);
	auto swb_c_2 = add_vertex(Operation("SWB_C_2"), g0);
	auto refi_c_0 = add_vertex(Operation("REFI_C_0"), g0);
	auto refi_c_1 = add_vertex(Operation("REFI_C_1"), g0);
	auto refi_c_2 = add_vertex(Operation("REFI_C_2"), g0);
	add_edge(swb_a_0, swb_a_1, Constraint("SWB_A_0", "SWB_A_1", 0, 0), g0);
	add_edge(swb_b_0, swb_b_1, Constraint("SWB_B_0", "SWB_B_1", 0, 0), g0);
	add_edge(swb_c_0, swb_c_1, Constraint("SWB_C_0", "SWB_C_1", 0, 0), g0);
	add_edge(swb_a_1, swb_a_2, Constraint("SWB_A_1", "SWB_A_2", 1, INT_MAX), g0);
	add_edge(swb_b_1, swb_b_2, Constraint("SWB_B_1", "SWB_B_2", 1, INT_MAX), g0);
	add_edge(swb_c_1, swb_c_2, Constraint("SWB_C_1", "SWB_C_2", 1, INT_MAX), g0);
	add_edge(refi_a_0, refi_a_1, Constraint("REFI_A_0", "REFI_A_1", 0, INT_MAX), g0);
	add_edge(refi_b_0, refi_b_1, Constraint("REFI_B_0", "REFI_B_1", 0, INT_MAX), g0);
	add_edge(refi_c_0, refi_c_1, Constraint("REFI_C_0", "REFI_C_1", 0, INT_MAX), g0);
	add_edge(refi_a_1, refi_a_2, Constraint("REFI_A_1", "REFI_A_2", 8, 8), g0);
	add_edge(refi_b_1, refi_b_2, Constraint("REFI_B_1", "REFI_B_2", 8, 8), g0);
	add_edge(refi_c_1, refi_c_2, Constraint("REFI_C_1", "REFI_C_2", 8, 8), g0);
	add_edge(dpu_0, dpu_1, Constraint("DPU_0", "DPU_1", 0, 0), g0);
	add_edge(dpu_1, dpu_2, Constraint("DPU_1", "DPU_2", 9, 9), g0);
	add_edge(swb_a_1, refi_a_1, Constraint("SWB_A_1", "REFI_A_1", 1, INT_MAX), g0);
	add_edge(swb_b_1, refi_b_1, Constraint("SWB_B_1", "REFI_B_1", 1, INT_MAX), g0);
	add_edge(swb_c_1, refi_c_1, Constraint("SWB_C_1", "REFI_C_1", 1, INT_MAX), g0);
	add_edge(refi_a_1, dpu_1, Constraint("REFI_A_1", "DPU_1", 0, 0), g0);
	add_edge(refi_b_1, dpu_1, Constraint("REFI_B_1", "DPU_1", 0, 0), g0);
	add_edge(dpu_1, refi_c_1, Constraint("DPU_1", "REFI_C_1", 2, 2), g0);
	add_edge(refi_a_2, swb_a_2, Constraint("REFI_A_2", "SWB_A_2", 0, INT_MAX), g0);
	add_edge(refi_b_2, swb_b_2, Constraint("REFI_B_2", "SWB_B_2", 0, INT_MAX), g0);
	add_edge(refi_c_2, swb_c_2, Constraint("REFI_C_2", "SWB_C_2", 0, INT_MAX), g0);

	Graph g1 = s.remove_redundent_links(g0);
	Graph g2 = s.pack_hard_links(g1);
	BOOST_CHECK_EQUAL(num_vertices(g2), 10);
	BOOST_CHECK_EQUAL(out_degree(0, g2), 1);
	BOOST_CHECK_EQUAL(out_degree(1, g2), 0);
	BOOST_CHECK_EQUAL(out_degree(2, g2), 1);
	BOOST_CHECK_EQUAL(out_degree(3, g2), 3);
	BOOST_CHECK_EQUAL(out_degree(4, g2), 1);
	BOOST_CHECK_EQUAL(out_degree(5, g2), 0);
	BOOST_CHECK_EQUAL(out_degree(6, g2), 1);
	BOOST_CHECK_EQUAL(out_degree(7, g2), 1);
	BOOST_CHECK_EQUAL(out_degree(8, g2), 0);
	BOOST_CHECK_EQUAL(out_degree(9, g2), 1);
	bool exist = false;
	graph_traits<Graph>::edge_descriptor e;
	tie(e, exist) = edge(0, 3, g2);
	BOOST_CHECK_EQUAL(exist, true);
	BOOST_CHECK_EQUAL(g2[e].d_lo, 1);
	BOOST_CHECK_EQUAL(g2[e].d_hi, INT_MAX);
	tie(e, exist) = edge(2, 3, g2);
	BOOST_CHECK_EQUAL(exist, true);
	BOOST_CHECK_EQUAL(g2[e].d_lo, 0);
	BOOST_CHECK_EQUAL(g2[e].d_hi, INT_MAX);
	tie(e, exist) = edge(4, 3, g2);
	BOOST_CHECK_EQUAL(exist, true);
	BOOST_CHECK_EQUAL(g2[e].d_lo, 1);
	BOOST_CHECK_EQUAL(g2[e].d_hi, INT_MAX);
	tie(e, exist) = edge(6, 3, g2);
	BOOST_CHECK_EQUAL(exist, true);
	BOOST_CHECK_EQUAL(g2[e].d_lo, 0);
	BOOST_CHECK_EQUAL(g2[e].d_hi, INT_MAX);
	tie(e, exist) = edge(7, 3, g2);
	BOOST_CHECK_EQUAL(exist, true);
	BOOST_CHECK_EQUAL(g2[e].d_lo, -1);
	BOOST_CHECK_EQUAL(g2[e].d_hi, INT_MAX);
	tie(e, exist) = edge(9, 3, g2);
	BOOST_CHECK_EQUAL(exist, true);
	BOOST_CHECK_EQUAL(g2[e].d_lo, -2);
	BOOST_CHECK_EQUAL(g2[e].d_hi, INT_MAX);
	tie(e, exist) = edge(3, 1, g2);
	BOOST_CHECK_EQUAL(exist, true);
	BOOST_CHECK_EQUAL(g2[e].d_lo, 8);
	BOOST_CHECK_EQUAL(g2[e].d_hi, INT_MAX);
	tie(e, exist) = edge(3, 5, g2);
	BOOST_CHECK_EQUAL(exist, true);
	BOOST_CHECK_EQUAL(g2[e].d_lo, 8);
	BOOST_CHECK_EQUAL(g2[e].d_hi, INT_MAX);
	tie(e, exist) = edge(3, 8, g2);
	BOOST_CHECK_EQUAL(exist, true);
	BOOST_CHECK_EQUAL(g2[e].d_lo, 10);
	BOOST_CHECK_EQUAL(g2[e].d_hi, INT_MAX);
}

BOOST_AUTO_TEST_CASE(schedule_graph)
{

	Descriptor d("main");
	d.add_operation(Operation("main", 0, {}));
	d.add_operation(Operation("SWB_A_0", 0, {{"0_0_SEQ", {{0, 0}}}, {"0_0_PIN0", {{0, INT_MAX}}}}));
	d.add_operation(Operation("SWB_A_1", 0, {}));
	d.add_operation(Operation("SWB_A_2", 0, {{"0_0_PIN0", {{INT_MIN, 0}}}}));
	d.add_operation(Operation("REFI_A_0", 0, {{"0_0_SEQ", {{0, 0}}}, {"0_0_AGU2", {{0, INT_MAX}}}}));
	d.add_operation(Operation("REFI_A_1", 0, {}));
	d.add_operation(Operation("REFI_A_2", 0, {{"0_0_AGU2", {{INT_MIN, 0}}}}));
	d.add_operation(Operation("SWB_B_0", 0, {{"0_0_SEQ", {{0, 0}}}, {"0_0_PIN1", {{0, INT_MAX}}}}));
	d.add_operation(Operation("SWB_B_1", 0, {}));
	d.add_operation(Operation("SWB_B_2", 0, {{"0_0_PIN1", {{INT_MIN, 0}}}}));
	d.add_operation(Operation("REFI_B_0", 0, {{"0_0_SEQ", {{0, 0}}}, {"0_0_AGU3", {{0, INT_MAX}}}}));
	d.add_operation(Operation("REFI_B_1", 0, {}));
	d.add_operation(Operation("REFI_B_2", 0, {{"0_0_AGU3", {{INT_MIN, 0}}}}));
	d.add_operation(Operation("DPU_0", 0, {{"0_0_SEQ", {{0, 0}}}, {"0_0_DPU", {{0, INT_MAX}}}}));
	d.add_operation(Operation("DPU_1", 0, {}));
	d.add_operation(Operation("DPU_2", 0, {{"0_0_DPU", {{INT_MIN, 0}}}}));
	d.add_operation(Operation("SWB_C_0", 0, {{"0_0_SEQ", {{0, 0}}}, {"0_0_PIN4", {{0, INT_MAX}}}}));
	d.add_operation(Operation("SWB_C_1", 0, {}));
	d.add_operation(Operation("SWB_C_2", 0, {{"0_0_PIN4", {{INT_MIN, 0}}}}));
	d.add_operation(Operation("REFI_C_0", 0, {{"0_0_SEQ", {{0, 0}}}, {"0_0_AGU0", {{0, INT_MAX}}}}));
	d.add_operation(Operation("REFI_C_1", 0, {}));
	d.add_operation(Operation("REFI_C_2", 0, {{"0_0_AGU0", {{INT_MIN, 0}}}}));
	d.add_constraint(Constraint("SWB_A_0", "SWB_A_1", 0, 0));
	d.add_constraint(Constraint("SWB_B_0", "SWB_B_1", 0, 0));
	d.add_constraint(Constraint("SWB_C_0", "SWB_C_1", 0, 0));
	d.add_constraint(Constraint("SWB_A_1", "SWB_A_2", 1, INT_MAX));
	d.add_constraint(Constraint("SWB_B_1", "SWB_B_2", 1, INT_MAX));
	d.add_constraint(Constraint("SWB_C_1", "SWB_C_2", 1, INT_MAX));
	d.add_constraint(Constraint("REFI_A_0", "REFI_A_1", 0, INT_MAX));
	d.add_constraint(Constraint("REFI_B_0", "REFI_B_1", 0, INT_MAX));
	d.add_constraint(Constraint("REFI_C_0", "REFI_C_1", 0, INT_MAX));
	d.add_constraint(Constraint("REFI_A_1", "REFI_A_2", 8, 8));
	d.add_constraint(Constraint("REFI_B_1", "REFI_B_2", 8, 8));
	d.add_constraint(Constraint("REFI_C_1", "REFI_C_2", 8, 8));
	d.add_constraint(Constraint("DPU_0", "DPU_1", 0, 0));
	d.add_constraint(Constraint("DPU_1", "DPU_2", 9, 9));
	d.add_constraint(Constraint("SWB_A_1", "REFI_A_1", 1, INT_MAX));
	d.add_constraint(Constraint("SWB_B_1", "REFI_B_1", 1, INT_MAX));
	d.add_constraint(Constraint("SWB_C_1", "REFI_C_1", 1, INT_MAX));
	d.add_constraint(Constraint("REFI_A_1", "DPU_1", 0, 0));
	d.add_constraint(Constraint("REFI_B_1", "DPU_1", 0, 0));
	d.add_constraint(Constraint("DPU_1", "REFI_C_1", 2, 2));
	d.add_constraint(Constraint("REFI_A_2", "SWB_A_2", 0, INT_MAX));
	d.add_constraint(Constraint("REFI_B_2", "SWB_B_2", 0, INT_MAX));
	d.add_constraint(Constraint("REFI_C_2", "SWB_C_2", 0, INT_MAX));

	d.get_mutable_operation("main").add_children({"SWB_A_0", "SWB_A_1", "SWB_A_2",
																								"REFI_A_0", "REFI_A_1", "REFI_A_2",
																								"SWB_B_0", "SWB_B_1", "SWB_B_2",
																								"REFI_B_0", "REFI_B_1", "REFI_B_2",
																								"DPU_0", "DPU_1", "DPU_2",
																								"SWB_C_0", "SWB_C_1", "SWB_C_2",
																								"REFI_C_0", "REFI_C_1", "REFI_C_2"},
																							 0);

	Scheduler s(d);
	BOOST_CHECK(s.schedule_vertex("main"));
	pt::ptree tree;
	tree = s.desc().dump();
	std::ostringstream oss;
	pt::write_xml(oss, tree);
	string xml_out_txt = oss.str();
	cout << xml_out_txt;
	BOOST_CHECK_EQUAL(md5(xml_out_txt), "286ca99a44dc471d5e542d955a8e987b");
}

BOOST_AUTO_TEST_CASE(schedule_all)
{

	Descriptor d("main");
	d.add_operation(Operation("main", 0, {}));
	d.add_operation(Operation("SWB_A_0", 0, {{"0_0_SEQ", {{0, 0}}}, {"0_0_PIN0", {{0, INT_MAX}}}}));
	d.add_operation(Operation("SWB_A_1", 0, {}));
	d.add_operation(Operation("SWB_A_2", 0, {{"0_0_PIN0", {{INT_MIN, 0}}}}));
	d.add_operation(Operation("REFI_A_0", 0, {{"0_0_SEQ", {{0, 0}}}, {"0_0_AGU2", {{0, INT_MAX}}}}));
	d.add_operation(Operation("REFI_A_1", 0, {}));
	d.add_operation(Operation("REFI_A_2", 0, {{"0_0_AGU2", {{INT_MIN, 0}}}}));
	d.add_operation(Operation("SWB_B_0", 0, {{"0_0_SEQ", {{0, 0}}}, {"0_0_PIN1", {{0, INT_MAX}}}}));
	d.add_operation(Operation("SWB_B_1", 0, {}));
	d.add_operation(Operation("SWB_B_2", 0, {{"0_0_PIN1", {{INT_MIN, 0}}}}));
	d.add_operation(Operation("REFI_B_0", 0, {{"0_0_SEQ", {{0, 0}}}, {"0_0_AGU3", {{0, INT_MAX}}}}));
	d.add_operation(Operation("REFI_B_1", 0, {}));
	d.add_operation(Operation("REFI_B_2", 0, {{"0_0_AGU3", {{INT_MIN, 0}}}}));
	d.add_operation(Operation("DPU_0", 0, {{"0_0_SEQ", {{0, 0}}}, {"0_0_DPU", {{0, INT_MAX}}}}));
	d.add_operation(Operation("DPU_1", 0, {}));
	d.add_operation(Operation("DPU_2", 0, {{"0_0_DPU", {{INT_MIN, 0}}}}));
	d.add_operation(Operation("SWB_C_0", 0, {{"0_0_SEQ", {{0, 0}}}, {"0_0_PIN4", {{0, INT_MAX}}}}));
	d.add_operation(Operation("SWB_C_1", 0, {}));
	d.add_operation(Operation("SWB_C_2", 0, {{"0_0_PIN4", {{INT_MIN, 0}}}}));
	d.add_operation(Operation("REFI_C_0", 0, {{"0_0_SEQ", {{0, 0}}}, {"0_0_AGU0", {{0, INT_MAX}}}}));
	d.add_operation(Operation("REFI_C_1", 0, {}));
	d.add_operation(Operation("REFI_C_2", 0, {{"0_0_AGU0", {{INT_MIN, 0}}}}));
	d.add_constraint(Constraint("SWB_A_0", "SWB_A_1", 0, 0));
	d.add_constraint(Constraint("SWB_B_0", "SWB_B_1", 0, 0));
	d.add_constraint(Constraint("SWB_C_0", "SWB_C_1", 0, 0));
	d.add_constraint(Constraint("SWB_A_1", "SWB_A_2", 1, INT_MAX));
	d.add_constraint(Constraint("SWB_B_1", "SWB_B_2", 1, INT_MAX));
	d.add_constraint(Constraint("SWB_C_1", "SWB_C_2", 1, INT_MAX));
	d.add_constraint(Constraint("REFI_A_0", "REFI_A_1", 0, INT_MAX));
	d.add_constraint(Constraint("REFI_B_0", "REFI_B_1", 0, INT_MAX));
	d.add_constraint(Constraint("REFI_C_0", "REFI_C_1", 0, INT_MAX));
	d.add_constraint(Constraint("REFI_A_1", "REFI_A_2", 8, 8));
	d.add_constraint(Constraint("REFI_B_1", "REFI_B_2", 8, 8));
	d.add_constraint(Constraint("REFI_C_1", "REFI_C_2", 8, 8));
	d.add_constraint(Constraint("DPU_0", "DPU_1", 0, 0));
	d.add_constraint(Constraint("DPU_1", "DPU_2", 9, 9));
	d.add_constraint(Constraint("SWB_A_1", "REFI_A_1", 1, INT_MAX));
	d.add_constraint(Constraint("SWB_B_1", "REFI_B_1", 1, INT_MAX));
	d.add_constraint(Constraint("SWB_C_1", "REFI_C_1", 1, INT_MAX));
	d.add_constraint(Constraint("REFI_A_1", "DPU_1", 0, 0));
	d.add_constraint(Constraint("REFI_B_1", "DPU_1", 0, 0));
	d.add_constraint(Constraint("DPU_1", "REFI_C_1", 2, 2));
	d.add_constraint(Constraint("REFI_A_2", "SWB_A_2", 0, INT_MAX));
	d.add_constraint(Constraint("REFI_B_2", "SWB_B_2", 0, INT_MAX));
	d.add_constraint(Constraint("REFI_C_2", "SWB_C_2", 0, INT_MAX));

	d.get_mutable_operation("main").add_children({"SWB_A_0", "SWB_A_1", "SWB_A_2",
																								"REFI_A_0", "REFI_A_1", "REFI_A_2",
																								"SWB_B_0", "SWB_B_1", "SWB_B_2",
																								"REFI_B_0", "REFI_B_1", "REFI_B_2",
																								"DPU_0", "DPU_1", "DPU_2",
																								"SWB_C_0", "SWB_C_1", "SWB_C_2",
																								"REFI_C_0", "REFI_C_1", "REFI_C_2"},
																							 0);

	Scheduler s(d);
	s.schedule();
	pt::ptree tree;
	tree = s.desc().dump();
	std::ostringstream oss;
	pt::write_xml(oss, tree);
	string xml_out_txt = oss.str();
	BOOST_CHECK_EQUAL(md5(xml_out_txt), "286ca99a44dc471d5e542d955a8e987b");
}
