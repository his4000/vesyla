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
#include "feopt/ExprSimpEngine.hpp"
#include "util/md5/md5.hpp"
INITIALIZE_EASYLOGGINGPP

using namespace vesyla::cidfg;

BOOST_AUTO_TEST_CASE(simplify)
{
	CidfgGraph g;

	RootVertex root_vertex;
	int id_root_vertex = g.add_vertex(root_vertex);

	RegVarVertex sv1;
	RegSinkVertex sv2;
	sv1.var_name = "A";
	sv2.var_name = "B";

	int id1 = g.add_vertex(sv1);
	int id2 = g.add_vertex(sv2);

	IndexingVertex vidx;
	int id_vidx = g.add_vertex(vidx);
	ReadingVertex vrd;
	int id_vrd = g.add_vertex(vrd);

	ConstVertex v1, v2, v3, v4, v5;
	v1.set_int_value(2);
	v2.set_int_value(32);
	v3.set_int_value(2);
	v4.set_int_value(32);
	v5.set_int_value(1);
	VarVertex v6;
	v6.var_name = "i";
	ComputationVertex v7, v8, v9, v10, v11;
	v7.func_name = "*";
	v8.func_name = "*";
	v9.func_name = "*";
	v10.func_name = "+";
	v11.func_name = "-";

	int id_v1 = g.add_vertex(v1);
	int id_v2 = g.add_vertex(v2);
	int id_v3 = g.add_vertex(v3);
	int id_v4 = g.add_vertex(v4);
	int id_v5 = g.add_vertex(v5);
	int id_v6 = g.add_vertex(v6);
	int id_v7 = g.add_vertex(v7);
	int id_v8 = g.add_vertex(v8);
	int id_v9 = g.add_vertex(v9);
	int id_v10 = g.add_vertex(v10);
	int id_v11 = g.add_vertex(v11);

	Edge e1(id_v1, 0, id_v7, 0);
	Edge e2(id_v2, 0, id_v7, 1);
	Edge e3(id_v6, 0, id_v8, 0);
	Edge e4(id_v7, 0, id_v8, 1);
	Edge e5(id_v3, 0, id_v9, 0);
	Edge e6(id_v4, 0, id_v9, 1);
	Edge e7(id_v11, 0, id_v10, 0);
	Edge e8(id_v5, 0, id_v10, 1);
	Edge e9(id_v8, 0, id_v11, 0);
	Edge e10(id_v9, 0, id_v11, 1);

	g.add_edge(e1);
	g.add_edge(e2);
	g.add_edge(e3);
	g.add_edge(e4);
	g.add_edge(e5);
	g.add_edge(e6);
	g.add_edge(e7);
	g.add_edge(e8);
	g.add_edge(e9);
	g.add_edge(e10);

	ConstVertex v12, v13, v14, v15, v20;
	v12.set_int_value(2);
	v13.set_int_value(32);
	v14.set_int_value(1);
	v15.set_int_value(32);
	v20.set_int_value(1);
	ComputationVertex v16, v17, v18, v19, v21, v22;
	v16.func_name = "*";
	v17.func_name = "*";
	v18.func_name = "*";
	v19.func_name = "-";
	v21.func_name = "-";
	v22.func_name = "/";
	int id_v12 = g.add_vertex(v12);
	int id_v13 = g.add_vertex(v13);
	int id_v14 = g.add_vertex(v14);
	int id_v15 = g.add_vertex(v15);
	int id_v16 = g.add_vertex(v16);
	int id_v17 = g.add_vertex(v17);
	int id_v18 = g.add_vertex(v18);
	int id_v19 = g.add_vertex(v19);
	int id_v20 = g.add_vertex(v20);
	int id_v21 = g.add_vertex(v21);
	int id_v22 = g.add_vertex(v22);

	Edge e13(id_v12, 0, id_v16, 0);
	Edge e14(id_v13, 0, id_v16, 1);
	Edge e15(id_v6, 0, id_v18, 0);
	Edge e16(id_v16, 0, id_v18, 1);
	Edge e17(id_v14, 0, id_v17, 0);
	Edge e18(id_v15, 0, id_v17, 1);
	Edge e19(id_v18, 0, id_v19, 0);
	Edge e20(id_v17, 0, id_v19, 1);
	Edge e21(id_v19, 0, id_v21, 0);
	Edge e22(id_v10, 0, id_v21, 0);
	Edge e23(id_v21, 0, id_v22, 0);
	Edge e24(id_v20, 0, id_v22, 1);
	g.add_edge(e13);
	g.add_edge(e14);
	g.add_edge(e15);
	g.add_edge(e16);
	g.add_edge(e17);
	g.add_edge(e18);
	g.add_edge(e19);
	g.add_edge(e20);
	g.add_edge(e21);
	g.add_edge(e22);
	g.add_edge(e23);
	g.add_edge(e24);

	ConstVertex vc0;
	vc0.set_int_value(0);
	int id_vc0 = g.add_vertex(vc0);
	Edge eidx0(id_v10, 0, id_vidx, 0);
	Edge eidx1(id_v20, 0, id_vidx, 1);
	Edge eidx2(id_v22, 0, id_vidx, 2);
	Edge eidx3(id_vc0, 0, id_vidx, 3);
	Edge eidx4(id_vc0, 0, id_vidx, 4);
	g.add_edge(eidx0);
	g.add_edge(eidx1);
	g.add_edge(eidx2);
	g.add_edge(eidx3);
	g.add_edge(eidx4);

	Edge erd0(id1, 0, id_vrd, 0);
	Edge erd1(id_vidx, 0, id_vrd, 1);
	Edge erd2(id_vrd, 0, id2, 0);
	g.add_edge(erd0);
	g.add_edge(erd1);
	g.add_edge(erd2);

	RootVertex *prv = static_cast<RootVertex *>(g.get_vertex(id_root_vertex));
	for (auto &v : g.get_vertices())
	{
		if (v->vertex_type != Vertex::ROOT_VERTEX)
		{
			prv->add_child(0, v->id);
		}
	}

	vesyla::feopt::ExprSimpEngine ese;
	cout << g.generate_dot_graph() << endl;
	ese.transform(g);
	cout << g.generate_dot_graph() << endl;
	BOOST_CHECK_EQUAL(md5(g.generate_dot_graph()), "0a3a4857f9166446997251a098129827");
}