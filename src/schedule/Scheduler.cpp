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

#include "Scheduler.hpp"

#define SCHEDULE_ENGINE_NAME NaiveEngine

namespace vesyla
{
namespace schedule
{

using namespace boost;

string to_dot(Graph g0)
{
	string str = "digraph G{\n";
	graph_traits<Graph>::vertex_iterator vi, vi_end;
	graph_traits<Graph>::edge_iterator ei, ei_end;
	for (tie(vi, vi_end) = vertices(g0); vi != vi_end; vi++)
	{
		str += g0[*vi].name() + ";\n";
	}
	for (tie(ei, ei_end) = edges(g0); ei != ei_end; ei++)
	{
		str += g0[source(*ei, g0)].name() + " -> " + g0[target(*ei, g0)].name() + "[label=\"[" + to_string(g0[*ei].d_lo) + "," + to_string(g0[*ei].d_hi) + "]\"];\n";
	}
	str += "}\n";
	return str;
}

void Scheduler::load_from_json(string _input_str)
{
	auto j = json::parse(_input_str);
	root.load_from_json(j);
}

void Scheduler::schedule_legacy()
{
	root.schedule();
	int start_in = 0;
	root.update_abs_start(start_in);
	int offset_in = 0;
	root.update_abs_offset(offset_in);
	add_to_dict(root);
}

void Scheduler::schedule()
{
	to_dot_graph(_desc, 0);
	merge_hierarchical_blocks_with_hard_link();
	to_dot_graph(_desc, 1);
	schedule_vertex(_desc.entry());
	update_scheduled_time();
	_desc.fill_timestamp();
	to_dot_graph(_desc, 2);
}

void Scheduler::add_to_dict(DependencyGraph::Vertex &v_)
{
	_lut[v_.name] = &v_;
	if (v_.node_type == DependencyGraph::HIRARICHICAL_NODE && v_.subgraph)
	{
		add_to_dict(*(v_.subgraph));
	}
	if (v_.node_type == DependencyGraph::HIRARICHICAL_NODE && v_.subgraph_extra)
	{
		add_to_dict(*(v_.subgraph_extra));
	}
}
void Scheduler::add_to_dict(DependencyGraph &g_)
{
	for (auto &vv : g_.vertex_list())
	{
		add_to_dict(vv);
	}
}

int Scheduler::get_start_time(string path)
{
	if (_lut.find(path) != _lut.end())
	{
		return _lut[path]->start;
	}
	LOG(FATAL) << "No such vertex with path \"" << path << "\".";
	return -1;
}

int Scheduler::get_offset(string path)
{
	if (_lut.find(path) != _lut.end())
	{
		return _lut[path]->offset;
	}
	LOG(FATAL) << "No such vertex with path \"" << path << "\".";
	return -1;
}

int Scheduler::get_unique_id()
{
	return _id_counter++;
}

void update_or_check_schedule_time(Graph &g3, graph_traits<Graph>::vertex_descriptor vd, map<boost::graph_traits<Graph>::vertex_descriptor, bool> &checked, int &min_time)
{
	graph_traits<Graph>::out_edge_iterator oei, oei_end;
	for (tie(oei, oei_end) = out_edges(vd, g3); oei != oei_end; oei++)
	{
		if (checked.find(target(*oei, g3)) != checked.end())
		{
			CHECK_EQ(g3[vd].scheduled_time + g3[*oei].d_lo, g3[target(*oei, g3)].scheduled_time)
					<< "Conflict hard-links";
		}
		else
		{
			g3[target(*oei, g3)].scheduled_time = g3[vd].scheduled_time + g3[*oei].d_lo;
			if (g3[target(*oei, g3)].scheduled_time < min_time)
			{
				min_time = g3[target(*oei, g3)].scheduled_time;
			}
			checked[target(*oei, g3)] = true;
			update_or_check_schedule_time(g3, target(*oei, g3), checked, min_time);
		}
	}
	graph_traits<Graph>::in_edge_iterator iei, iei_end;
	for (tie(iei, iei_end) = in_edges(vd, g3); iei != iei_end; iei++)
	{
		if (checked.find(source(*iei, g3)) != checked.end())
		{
			CHECK_EQ(g3[vd].scheduled_time - g3[*iei].d_lo, g3[source(*iei, g3)].scheduled_time)
					<< "Conflict hard-links";
		}
		else
		{
			g3[source(*iei, g3)].scheduled_time = g3[vd].scheduled_time - g3[*iei].d_lo;
			if (g3[source(*iei, g3)].scheduled_time < min_time)
			{
				min_time = g3[source(*iei, g3)].scheduled_time;
			}
			checked[source(*iei, g3)] = true;
			update_or_check_schedule_time(g3, source(*iei, g3), checked, min_time);
		}
	}
}

Graph Scheduler::pack_hard_links(Graph &g0)
{
	map<string, boost::graph_traits<Graph>::vertex_descriptor> gdict0;
	graph_traits<Graph>::vertex_iterator vi, vi_end;
	for (tie(vi, vi_end) = vertices(g0); vi != vi_end; vi++)
	{
		gdict0[g0[*vi].name()] = *vi;
	}

	Graph g1;
	map<string, boost::graph_traits<Graph>::vertex_descriptor> gdict1;
	for (tie(vi, vi_end) = vertices(g0); vi != vi_end; vi++)
	{
		auto v = add_vertex(g0[*vi], g1);
		gdict1[g0[*vi].name()] = v;
	}
	graph_traits<Graph>::edge_iterator ei, ei_end;
	for (tie(ei, ei_end) = edges(g0); ei != ei_end; ++ei)
	{
		if (g0[*ei].d_lo == g0[*ei].d_hi)
		{
			add_edge(gdict1[g0[source(*ei, g0)].name()], gdict1[g0[target(*ei, g0)].name()], g0[*ei], g1);
		}
	}

	// convert to undirected graph to find connected components.
	typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS> UGraph;
	UGraph ug1(num_vertices(g1));
	for (tie(ei, ei_end) = edges(g1); ei != ei_end; ++ei)
	{
		add_edge(source(*ei, g1), target(*ei, g1), ug1);
		add_edge(target(*ei, g1), source(*ei, g1), ug1);
	}
	std::vector<int> component(boost::num_vertices(ug1));
	int num_components = boost::connected_components(ug1, &component[0]);

	Graph g2;
	map<string, boost::graph_traits<Graph>::vertex_descriptor> gdict2;
	map<string, string> group_id;
	for (int i = 0; i < num_components; i++)
	{
		int j = 0;
		vector<graph_traits<Graph>::vertex_iterator> comp_v;
		for (tie(vi, vi_end) = vertices(g1); vi != vi_end; vi++)
		{
			if (component[j] == i)
			{
				comp_v.push_back(vi);
			}
			j++;
		}
		if (comp_v.size() == 1)
		{
			auto vv = add_vertex(g1[*(comp_v[0])], g2);
			gdict2[g1[*(comp_v[0])].name()] = vv;
		}
		else if (comp_v.size() > 1)
		{

			Operation op_temp("__temp" + to_string(get_unique_id()));
			for (auto v : comp_v)
			{
				op_temp.add_child(g1[*v].name());
				group_id[g1[*v].name()] = op_temp.name();
			}

			Graph g3;
			map<string, boost::graph_traits<Graph>::vertex_descriptor> gdict3;
			for (auto v : comp_v)
			{
				auto hv = add_vertex(g1[*v], g3);
				gdict3[g1[*v].name()] = hv;
			}
			for (auto v : comp_v)
			{
				graph_traits<Graph>::out_edge_iterator ei, ei_end;
				for (tie(ei, ei_end) = out_edges(*v, g1); ei != ei_end; ei++)
				{
					if (g1[*ei].d_lo == g1[*ei].d_hi)
					{
						add_edge(gdict3[g1[source(*ei, g1)].name()], gdict3[g1[target(*ei, g1)].name()], g1[*ei], g3);
					}
				}
			}

			// assign offset and check ROT conflict
			// assign offset
			int min_time = INT_MAX;
			map<boost::graph_traits<Graph>::vertex_descriptor, bool> checked;
			for (tie(vi, vi_end) = vertices(g3); vi != vi_end; vi++)
			{
				if (checked.find(*vi) == checked.end())
				{
					g3[*vi].scheduled_time = 0;
					if (g3[*vi].scheduled_time < min_time)
					{
						min_time = g3[*vi].scheduled_time;
					}
					checked[*vi] = true;
					update_or_check_schedule_time(g3, *vi, checked, min_time);
				}
				else
				{
					continue;
				}
			}

			for (tie(vi, vi_end) = vertices(g3); vi != vi_end; vi++)
			{
				g3[*vi].scheduled_time -= min_time;
				g0[gdict0[g3[*vi].name()]].scheduled_time = g3[*vi].scheduled_time;
			}

			// Check and merge ROTs
			for (tie(vi, vi_end) = vertices(g3); vi != vi_end; vi++)
			{
				op_temp.rot.merge(g3[*vi].rot, g3[*vi].scheduled_time);
			}
			CHECK(op_temp.rot.verify());

			auto vv = add_vertex(op_temp, g2);
			gdict2[op_temp.name()] = vv;
		}
	}

	for (tie(ei, ei_end) = edges(g0); ei != ei_end; ++ei)
	{
		if (g0[*ei].d_lo != g0[*ei].d_hi)
		{
			boost::graph_traits<Graph>::vertex_descriptor src, dest;
			Constraint c = g0[*ei];
			int distance = 0;

			if (gdict2.find(g0[source(*ei, g0)].name()) != gdict2.end())
			{
				src = gdict2[g0[source(*ei, g0)].name()];
			}
			else
			{
				src = gdict2[group_id[g0[source(*ei, g0)].name()]];
				distance += g0[source(*ei, g0)].scheduled_time;
			}

			if (gdict2.find(g0[target(*ei, g0)].name()) != gdict2.end())
			{
				dest = gdict2[g0[target(*ei, g0)].name()];
			}
			else
			{
				dest = gdict2[group_id[g0[target(*ei, g0)].name()]];
				distance -= g0[target(*ei, g0)].scheduled_time;
			}

			if (g0[*ei].d_lo > INT_MIN)
				c.d_lo = distance + g0[*ei].d_lo;
			if (g0[*ei].d_hi < INT_MAX)
				c.d_hi = distance + g0[*ei].d_hi;

			if (src == dest)
			{
				CHECK(c.d_lo <= 0 && c.d_hi >= 0)
						<< "Self-loop delay is not 0!";
			}
			else
			{
				add_edge(src, dest, c, g2);
			}
		}
	}
	return g2;
}

Graph Scheduler::remove_redundent_links(Graph &g0)
{

	vector<graph_traits<Graph>::vertex_descriptor> c;
	topological_sort(g0, std::back_inserter(c));

	graph_traits<Graph>::edge_iterator ei, ei_end;
	for (tie(ei, ei_end) = edges(g0); ei != ei_end; ei++)
	{
		int d_lo = INT_MAX;
		int d_hi = INT_MIN;
		graph_traits<Graph>::vertex_descriptor v_src, v_dest;

		Graph g1;
		map<string, graph_traits<Graph>::vertex_descriptor> gdict1;
		graph_traits<Graph>::vertex_iterator vi0, vi0_end;
		for (tie(vi0, vi0_end) = vertices(g0); vi0 != vi0_end; vi0++)
		{
			gdict1[g0[*vi0].name()] = add_vertex(g0[*vi0], g1);
		}
		graph_traits<Graph>::edge_iterator ei0, ei0_end;
		for (tie(ei0, ei0_end) = edges(g0); ei0 != ei0_end; ei0++)
		{
			if (g0[*ei0].d_lo > g0[*ei0].d_hi)
			{
				// filter invalid edge
				continue;
			}
			if (*ei0 == *ei)
			{
				v_src = gdict1[g0[source(*ei, g0)].name()];
				v_dest = gdict1[g0[target(*ei, g0)].name()];
				d_lo = g0[*ei].d_lo;
				d_hi = g0[*ei].d_hi;
				continue;
			}
			add_edge(gdict1[g0[source(*ei0, g0)].name()], gdict1[g0[target(*ei0, g0)].name()], g0[*ei0], g1);
		}

		if (d_lo > d_hi)
		{
			continue;
		}

		bool reduction_ok = false;
		vector<int> v_d_lo(num_vertices(g1), 0);
		vector<int> v_d_hi(num_vertices(g1), 0);

		bool flag_src_find = false;
		vector<graph_traits<Graph>::vertex_descriptor> black_list;
		for (vector<graph_traits<Graph>::vertex_descriptor>::reverse_iterator v1 = c.rbegin(); v1 != c.rend(); ++v1)
		{
			if (*v1 == v_src)
			{
				flag_src_find = true;
				continue;
			}
			if (!flag_src_find)
			{
				black_list.push_back(*v1);
				continue;
			}
			int d_lo_target = INT_MIN;
			int d_hi_target = INT_MAX;
			graph_traits<Graph>::in_edge_iterator ei_tmp, ei_tmp_end;
			for (tie(ei_tmp, ei_tmp_end) = in_edges(*v1, g1); ei_tmp != ei_tmp_end; ei_tmp++)
			{
				if (std::find(black_list.begin(), black_list.end(), source(*ei_tmp, g1)) != black_list.end())
				{
					continue;
				}
				int d_lo_tmp;
				int d_hi_tmp;
				if (v_d_lo[source(*ei_tmp, g1)] > INT_MIN && g1[*ei_tmp].d_lo > INT_MIN && (long(v_d_lo[source(*ei_tmp, g1)]) + long(g1[*ei_tmp].d_lo)) > INT_MIN)
				{
					d_lo_tmp = v_d_lo[source(*ei_tmp, g1)] + g1[*ei_tmp].d_lo;
				}
				else
				{
					d_lo_tmp = INT_MIN;
				}
				if (v_d_hi[source(*ei_tmp, g1)] < INT_MAX && g1[*ei_tmp].d_hi < INT_MAX && (long(v_d_hi[source(*ei_tmp, g1)]) + long(g1[*ei_tmp].d_hi)) < INT_MAX)
				{
					d_hi_tmp = v_d_hi[source(*ei_tmp, g1)] + g1[*ei_tmp].d_hi;
				}
				else
				{
					d_hi_tmp = INT_MAX;
				}

				if (d_lo_tmp >= d_lo_target && d_hi_tmp <= d_hi_target)
				{
					d_lo_target = d_lo_tmp;
					d_hi_target = d_hi_tmp;
				}
				else if (d_lo_tmp <= d_lo_target && d_hi_tmp >= d_hi_target)
				{
					;
				}
				else
				{
					LOG(FATAL) << "Conflict constraint!";
				}
			}
			v_d_lo[*v1] = d_lo_target;
			v_d_hi[*v1] = d_hi_target;

			if (*v1 == v_dest)
			{
				if (d_lo <= d_lo_target && d_hi >= d_hi_target)
				{
					reduction_ok = true;
				}
				break;
			}
		}

		if (reduction_ok)
		{
			// Mark this edge as invalid edge.
			g0[*ei].d_lo = INT_MAX;
			g0[*ei].d_hi = INT_MIN;
		}
	}

	Graph g1;
	map<string, graph_traits<Graph>::vertex_descriptor> gdict1;
	graph_traits<Graph>::vertex_iterator vi0, vi0_end;
	for (tie(vi0, vi0_end) = vertices(g0); vi0 != vi0_end; vi0++)
	{
		gdict1[g0[*vi0].name()] = add_vertex(g0[*vi0], g1);
	}
	graph_traits<Graph>::edge_iterator ei0, ei0_end;
	for (tie(ei0, ei0_end) = edges(g0); ei0 != ei0_end; ei0++)
	{
		if (g0[*ei0].d_lo > g0[*ei0].d_hi)
		{
			// filter invalid edge
			continue;
		}
		add_edge(gdict1[g0[source(*ei0, g0)].name()], gdict1[g0[target(*ei0, g0)].name()], g0[*ei0], g1);
	}

	return g1;
}

Graph Scheduler::eliminate_negative_links(Graph &g0)
{
	Graph g1;
	copy_graph(g0, g1);

	vector<graph_traits<Graph>::vertex_descriptor> topo_order;
	topological_sort(g1, std::back_inserter(topo_order));
	for (auto &vd : topo_order)
	{
		int min_d_lo = INT_MAX;

		graph_traits<Graph>::in_edge_iterator iei, iei_end;
		graph_traits<Graph>::out_edge_iterator oei, oei_end;
		for (tie(oei, oei_end) = out_edges(vd, g1); oei != oei_end; oei++)
		{
			if (g1[*oei].d_lo < min_d_lo)
			{
				min_d_lo = g1[*oei].d_lo;
			}
		}
		if (min_d_lo > 0)
		{
			continue;
		}
		// shift ROT
		g1[vd].shift_factor = abs(min_d_lo) + 1;
		for (auto &r : g1[vd].rot)
		{
			r.second.shift(g1[vd].shift_factor);
		}

		// fix out-going edges
		for (tie(oei, oei_end) = out_edges(vd, g1); oei != oei_end; oei++)
		{
			if (g1[*oei].d_lo != INT_MIN)
			{
				g1[*oei].d_lo += g1[vd].shift_factor;
			}
			if (g1[*oei].d_hi != INT_MAX)
			{
				g1[*oei].d_hi += g1[vd].shift_factor;
			}
		}

		// fix in-coming edges
		for (tie(iei, iei_end) = in_edges(vd, g1); iei != iei_end; iei++)
		{
			if (g1[*iei].d_lo != INT_MIN)
			{
				g1[*iei].d_lo -= g1[vd].shift_factor;
			}
			if (g1[*iei].d_hi != INT_MAX)
			{
				g1[*iei].d_hi -= g1[vd].shift_factor;
			}
		}
	}

	// finally check there is no negative edge exists
	graph_traits<Graph>::edge_iterator ei, ei_end;
	for (tie(ei, ei_end) = edges(g1); ei != ei_end; ei++)
	{
		CHECK_GT(g1[*ei].d_lo, 0) << "Found negative edge!!";
	}

	return g1;
}

Graph Scheduler::predict_order(Graph &g0)
{
	// copy a graph
	Graph g00;
	copy_graph(g0, g00);

	// get all resources
	set<string> all_resources;
	graph_traits<Graph>::vertex_iterator vi, vi_end;
	graph_traits<Graph>::edge_iterator ei, ei_end;
	for (tie(vi, vi_end) = vertices(g0); vi != vi_end; vi++)
	{
		Rot rot = g0[*vi].rot;
		for (auto &r : rot)
		{
			all_resources.insert(r.first);
		}
	}

	// iterate each resource and predict the order of marked nodes
	typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, int> MarkedGraph;
	for (auto &rs : all_resources)
	{
		int temp_sn_counter = 1000; // This counter gives a temp serial number to period frame

		MarkedGraph g1;
		std::unordered_map<boost::graph_traits<Graph>::vertex_descriptor, boost::graph_traits<MarkedGraph>::vertex_descriptor> g0_2_g1;
		std::unordered_map<boost::graph_traits<MarkedGraph>::vertex_descriptor, boost::graph_traits<Graph>::vertex_descriptor> g1_2_g0;
		MarkedGraph::vertex_iterator mvi, mvi_end;
		// build tranlation map
		std::unordered_map<int, int> sn_translate_map;
		int sn_translate_index = 1000;
		for (tie(vi, vi_end) = vertices(g0); vi != vi_end; vi++)
		{
			boost::graph_traits<MarkedGraph>::vertex_descriptor vd;
			bool flag_has_resource = false;
			Rot rot = g0[*vi].rot;
			for (auto &r : rot)
			{
				if (r.first == rs)
				{
					flag_has_resource = true;
					bool flag_key = false;
					bool flag_lock = false;
					bool flag_period = false;
					int key_sn = -1;
					int lock_sn = -1;
					for (auto &f : r.second)
					{
						if (f.type() == Frame::KEY)
						{
							flag_key = true;
							key_sn = f.sn;
						}
						else if (f.type() == Frame::LOCK)
						{
							flag_lock = true;
							lock_sn = f.sn;
						}
					}
					if (flag_key && flag_lock)
					{
						vd = add_vertex(4, g1); // both lock and key
						g0_2_g1[*vi] = vd;
						g1_2_g0[vd] = *vi;
						sn_translate_map[key_sn] = sn_translate_index;
						sn_translate_map[lock_sn] = sn_translate_index;
						sn_translate_index++;
					}
					break;
				}
			}
		}

		// Build the marked graph
		std::unordered_map<int, set<boost::graph_traits<MarkedGraph>::vertex_descriptor>> sn_2_v_map;
		std::unordered_map<boost::graph_traits<MarkedGraph>::vertex_descriptor, int> v_2_sn_map;
		for (tie(vi, vi_end) = vertices(g0); vi != vi_end; vi++)
		{
			boost::graph_traits<MarkedGraph>::vertex_descriptor vd;
			bool flag_has_resource = false;
			Rot rot = g0[*vi].rot;
			for (auto &r : rot)
			{
				if (r.first == rs)
				{
					flag_has_resource = true;
					bool flag_key = false;
					bool flag_lock = false;
					bool flag_period = false;
					int sn = -1;
					for (auto &f : r.second)
					{
						if (f.type() == Frame::KEY)
						{
							flag_key = true;
							sn = f.sn;
						}
						else if (f.type() == Frame::LOCK)
						{
							flag_lock = true;
							sn = f.sn;
						}
						else if (f.type() == Frame::PERIOD)
						{
							flag_period = true;
							sn = temp_sn_counter;
							temp_sn_counter++;
						}
					}
					if (sn_translate_map.find(sn) != sn_translate_map.end())
					{
						sn = sn_translate_map[sn];
					}
					if (flag_key && flag_lock)
					{
						vd = add_vertex(4, g1); // both lock and key
						g0_2_g1[*vi] = vd;
						g1_2_g0[vd] = *vi;
						sn_2_v_map[sn].insert(vd);
						v_2_sn_map[vd] = sn;
					}
					else if (flag_key)
					{
						vd = add_vertex(Frame::KEY, g1);
						g0_2_g1[*vi] = vd;
						g1_2_g0[vd] = *vi;
						sn_2_v_map[sn].insert(vd);
						v_2_sn_map[vd] = sn;
					}
					else if (flag_lock)
					{
						vd = add_vertex(Frame::LOCK, g1);
						g0_2_g1[*vi] = vd;
						g1_2_g0[vd] = *vi;
						sn_2_v_map[sn].insert(vd);
						v_2_sn_map[vd] = sn;
					}
					else if (flag_period)
					{
						vd = add_vertex(Frame::PERIOD, g1);
						g0_2_g1[*vi] = vd;
						g1_2_g0[vd] = *vi;
						sn_2_v_map[sn].insert(vd);
						v_2_sn_map[vd] = sn;
					}
					else
					{
						vd = add_vertex(Frame::ILLEGAL, g1);
						g0_2_g1[*vi] = vd;
						g1_2_g0[vd] = *vi;
					}
					break;
				}
			}
			if (flag_has_resource == false)
			{
				vd = add_vertex(Frame::ILLEGAL, g1);
				g0_2_g1[*vi] = vd;
				g1_2_g0[vd] = *vi;
			}
		}
		for (tie(ei, ei_end) = edges(g0); ei != ei_end; ei++)
		{
			add_edge(g0_2_g1[source(*ei, g0)], g0_2_g1[target(*ei, g0)], g1);
		}

		//Now find the root node and leaf node of each region. Based on assumption, each region will
		// have exactly one root node which is the LOCK node and exactly one leaf node which is the KEY
		// node
		std::unordered_map<int, boost::graph_traits<MarkedGraph>::vertex_descriptor> sn_root_map, sn_leaf_map;
		for (auto &p : sn_2_v_map)
		{
			int lock_count = 0;
			int key_count = 0;
			for (auto &q : p.second)
			{
				if (g1[q] == Frame::LOCK)
				{
					sn_root_map[p.first] = q;
					lock_count++;
				}
				else if (g1[q] == Frame::KEY)
				{
					sn_leaf_map[p.first] = q;
					key_count++;
				}
				else if (g1[q] == Frame::PERIOD)
				{
					sn_leaf_map[p.first] = q;
					sn_root_map[p.first] = q;
					key_count++;
					lock_count++;
				}
				CHECK_LE(lock_count, 1) << "Atmost 1 LOCK node is allowed in each region";
				CHECK_LE(key_count, 1) << "Atmost 1 KEY node is allowed in each region";
			}
		}

		// Now, find all path from root to leaf in each region, and add intermediate nodes in the path to its region.
		graph_traits<MarkedGraph>::edge_iterator mei, mei_end;
		graph_traits<MarkedGraph>::in_edge_iterator miei, miei_end;
		graph_traits<MarkedGraph>::out_edge_iterator moei, moei_end;
		for (auto &p : sn_2_v_map)
		{
			int sn = p.first;
			// Period node, skip
			if (sn_root_map[sn] == sn_leaf_map[sn])
			{
				continue;
			}

			set<boost::graph_traits<MarkedGraph>::vertex_descriptor> reachable_from_root, reachable_from_leaf, visited;
			std::queue<boost::graph_traits<MarkedGraph>::vertex_descriptor> q;
			q.push(sn_root_map[sn]);
			visited.insert(sn_root_map[sn]);
			while (!q.empty())
			{
				boost::graph_traits<MarkedGraph>::vertex_descriptor vd = q.front();
				q.pop();
				for (tie(moei, moei_end) = out_edges(vd, g1); moei != moei_end; moei++)
				{
					if (visited.find(target(*moei, g1)) == visited.end())
					{
						q.push(target(*moei, g1));
						visited.insert(target(*moei, g1));
					}
				}
			}
			reachable_from_root = visited;
			visited.clear();
			q.push(sn_leaf_map[sn]);
			visited.insert(sn_leaf_map[sn]);
			while (!q.empty())
			{
				boost::graph_traits<MarkedGraph>::vertex_descriptor vd = q.front();
				q.pop();
				for (tie(miei, miei_end) = in_edges(vd, g1); miei != miei_end; miei++)
				{
					if (visited.find(source(*miei, g1)) == visited.end())
					{
						q.push(source(*miei, g1));
						visited.insert(source(*miei, g1));
					}
				}
			}
			reachable_from_leaf = visited;
			visited.clear();
			p.second.clear();
			set_intersection(reachable_from_root.begin(), reachable_from_root.end(), reachable_from_leaf.begin(), reachable_from_leaf.end(), std::inserter(p.second, p.second.begin()));
			for (auto &v : p.second)
			{
				v_2_sn_map[v] = sn;
			}
		}

		// Now, merge the nodes with same sn
		MarkedGraph g2;
		map<int, boost::graph_traits<MarkedGraph>::vertex_descriptor> added_sn;
		map<boost::graph_traits<MarkedGraph>::vertex_descriptor, boost::graph_traits<MarkedGraph>::vertex_descriptor> g1_2_g2;
		map<boost::graph_traits<MarkedGraph>::vertex_descriptor, boost::graph_traits<MarkedGraph>::vertex_descriptor> g2_2_g1;
		for (tie(mvi, mvi_end) = vertices(g1); mvi != mvi_end; mvi++)
		{
			int sn = -1;
			if (v_2_sn_map.find(*mvi) != v_2_sn_map.end())
			{
				sn = v_2_sn_map[*mvi];
			}
			if (sn >= 0)
			{
				if (added_sn.find(sn) == added_sn.end())
				{

					boost::graph_traits<MarkedGraph>::vertex_descriptor vd;
					vd = add_vertex(sn, g2);
					added_sn[sn] = vd;
					g1_2_g2[*mvi] = vd;
					g2_2_g1[vd] = *mvi;
				}
				else
				{
					boost::graph_traits<MarkedGraph>::vertex_descriptor vd;
					vd = added_sn[sn];
					g1_2_g2[*mvi] = vd;
					g2_2_g1[vd] = *mvi;
				}
			}
			else
			{
				boost::graph_traits<MarkedGraph>::vertex_descriptor vd;
				vd = add_vertex(sn, g2);
				g1_2_g2[*mvi] = vd;
				g2_2_g1[vd] = *mvi;
			}
		}
		for (tie(mei, mei_end) = edges(g1); mei != mei_end; mei++)
		{
			if (!edge(g1_2_g2[source(*mei, g1)], g1_2_g2[target(*mei, g1)], g2).second)
			{
				add_edge(g1_2_g2[source(*mei, g1)], g1_2_g2[target(*mei, g1)], g2);
			}
		}

		// find connected sn marked region and force a prediction
		vector<pair<boost::graph_traits<MarkedGraph>::vertex_descriptor, boost::graph_traits<MarkedGraph>::vertex_descriptor>> vec;
		for (tie(mvi, mvi_end) = vertices(g2); mvi != mvi_end; mvi++)
		{
			if (g2[*mvi] < 0)
			{
				continue;
			}
			set<boost::graph_traits<MarkedGraph>::vertex_descriptor> visited;
			std::queue<boost::graph_traits<MarkedGraph>::vertex_descriptor> q;
			q.push(*mvi);
			visited.insert(*mvi);
			while (!q.empty())
			{
				boost::graph_traits<MarkedGraph>::vertex_descriptor vd = q.front();
				q.pop();
				for (tie(moei, moei_end) = out_edges(vd, g2); moei != moei_end; moei++)
				{
					if (visited.find(target(*moei, g2)) == visited.end())
					{
						q.push(target(*moei, g2));
						visited.insert(target(*moei, g2));
						if (g2[target(*moei, g2)] >= 0 && g2[target(*moei, g2)] != g2[*mvi])
						{
							boost::graph_traits<MarkedGraph>::vertex_descriptor leaf, root;
							leaf = sn_leaf_map[g2[*mvi]];
							root = sn_root_map[g2[target(*moei, g2)]];
							if (!edge(leaf, root, g1).second)
							{
								pair<boost::graph_traits<MarkedGraph>::vertex_descriptor, boost::graph_traits<MarkedGraph>::vertex_descriptor> m(leaf, root);
								vec.push_back(m);
							}
						}
					}
				}
			}
		}
		for (auto &m : vec)
		{
			add_edge(m.first, m.second, g1);
		}

		// Now, find each corssing-region edge.
		vec.clear();
		for (tie(mei, mei_end) = edges(g1); mei != mei_end; mei++)
		{
			boost::graph_traits<MarkedGraph>::vertex_descriptor v_src, v_dest;
			v_src = source(*mei, g1);
			v_dest = target(*mei, g1);
			if (v_2_sn_map.find(v_src) != v_2_sn_map.end() && v_2_sn_map.find(v_dest) != v_2_sn_map.end())
			{
				if (v_2_sn_map[v_src] != v_2_sn_map[v_dest])
				{
					pair<boost::graph_traits<MarkedGraph>::vertex_descriptor, boost::graph_traits<MarkedGraph>::vertex_descriptor> m(sn_leaf_map[v_2_sn_map[v_src]], sn_root_map[v_2_sn_map[v_dest]]);
					vec.push_back(m);
				}
			}
		}
		for (auto &m : vec)
		{
			add_edge(m.first, m.second, g1);
		}

		boost::graph_traits<MarkedGraph>::edge_iterator eii, eii_end;
		for (tie(eii, eii_end) = edges(g1); eii != eii_end; eii++)
		{
			boost::graph_traits<Graph>::vertex_descriptor vd_src, vd_dest;
			vd_src = g1_2_g0[source(*eii, g1)];	// the g00 is the copy of g0, so the vertex descriptors are the same.
			vd_dest = g1_2_g0[target(*eii, g1)]; // the g00 is the copy of g0, so the vertex descriptors are the same.
			string src_name = g0[vd_src].name();
			string dest_name = g0[vd_dest].name();

			// check if there is already an edge
			if (!edge(vd_src, vd_dest, g0).second)
			{
				add_edge(vd_src, vd_dest, Constraint(src_name, dest_name, 1, INT_MAX), g00);
				continue;
			}
			if (edge(vd_src, vd_dest, g0).second && g0[edge(vd_src, vd_dest, g0).first].d_lo <= 0)
			{
				add_edge(vd_src, vd_dest, Constraint(src_name, dest_name, 1, INT_MAX), g00);
			}
		}
	}

	return g00;
} // namespace schedule

void Scheduler::adjust_interblock_constraints(string parent_block_, vector<string> name_list_)
{
	for (auto &v : name_list_)
	{
		vector<Constraint *> list = _desc.get_all_constraints();
		for (auto &c : list)
		{
			if (find(name_list_.begin(), name_list_.end(), c->src()) != name_list_.end() && find(name_list_.begin(), name_list_.end(), c->dest()) != name_list_.end())
			{
				_desc.remove_constraint(c->src(), c->dest());
			}
			else if (find(name_list_.begin(), name_list_.end(), c->src()) != name_list_.end())
			{
				Constraint cc(parent_block_, c->dest(), c->d_lo, c->d_hi);
				int schedule_time = _desc.get_operation(c->src()).scheduled_time;
				if (cc.d_hi < INT_MAX - schedule_time)
				{
					cc.d_hi += schedule_time;
				}
				if (cc.d_lo > INT_MIN)
				{
					cc.d_lo += schedule_time;
				}
				_desc.remove_constraint(c->src(), c->dest());
				_desc.add_constraint(cc);
			}
			else if (find(name_list_.begin(), name_list_.end(), c->dest()) != name_list_.end())
			{
				Constraint cc(c->src(), parent_block_, c->d_lo, c->d_hi);
				int schedule_time = _desc.get_operation(c->dest()).scheduled_time;
				if (cc.d_hi < INT_MAX)
				{
					cc.d_hi -= schedule_time;
				}
				if (cc.d_lo > INT_MIN + schedule_time)
				{
					cc.d_lo -= schedule_time;
				}
				_desc.remove_constraint(c->src(), c->dest());
				_desc.add_constraint(cc);
			}
		}
	}
}

bool Scheduler::schedule_vertex(string name_)
{
	Operation &o = _desc.get_mutable_operation(name_);
	o.scheduled_time = 0;
	if (o.children0.size() <= 0 && o.children1.size() <= 0)
	{
		return true;
	}

	Rot rot0;
	Rot rot1;
	if (o.children0.size() > 0)
	{
		CHECK(schedule_hierarchical_dependency_graph(o.children0));
		for (auto &v : o.children0)
		{
			rot0.merge(_desc.get_operation(v).rot, _desc.get_operation(v).scheduled_time);
		}
		adjust_interblock_constraints(o.name(), o.children0);
	}
	CHECK(rot0.verify());
	if (o.children1.size() > 0)
	{
		CHECK(schedule_hierarchical_dependency_graph(o.children1));
		for (auto &v : o.children1)
		{
			rot1.merge(_desc.get_operation(v).rot, _desc.get_operation(v).scheduled_time);
		}
		adjust_interblock_constraints(o.name(), o.children1);
	}
	CHECK(rot1.verify());
	CHECK(rot0.unite(rot1, 0));

	if (o.is_bulk)
	{

		// When merge ROTs of its children, use the largest range of all resource to fill
		// all resource. In order to guarantee the two herarchical nodes will not overlap.
		// The problem of overlapping will affect the Loop nodes.

		int t0 = INT_MAX;
		int t1 = INT_MIN;

		Rot rot2;
		for (auto &e : rot0)
		{
			string r = e.first;
			Timetable tb = e.second;

			for (auto &f : tb)
			{
				if (f.t0 == INT_MIN && f.t1 == INT_MAX)
				{
					continue;
				}
				rot2[r].push_back(Frame(f.t0, f.t1));
			}
		}

		t0 = INT_MAX;
		t1 = INT_MIN;
		for (auto &e : rot2)
		{
			string r = e.first;
			Timetable tb = e.second;

			for (auto &f : tb)
			{
				if (f.t0 < t0 && f.t0 != INT_MIN)
				{
					t0 = f.t0;
				}
				if (f.t1 > t1 && f.t1 != INT_MAX)
				{
					t1 = f.t1;
				}
			}
		}

		for (auto &e : rot2)
		{
			o.rot[e.first].push_back(Frame(t0, t1));
		}
	}
	else
	{
		// When merge ROTs of its children, don't use the largest range of all resource to fill
		// all resource. This can apply to all nodes except for loop nodes.
		int t0 = INT_MAX;
		int t1 = INT_MIN;

		Rot rot2;
		for (auto &e : rot0)
		{
			string r = e.first;
			Timetable tb = e.second;

			for (auto &f : tb)
			{
				if (f.t0 == INT_MIN && f.t1 == INT_MAX)
				{
					continue;
				}
				rot2[r].push_back(Frame(f.t0, f.t1));
			}
		}

		for (auto &e : rot2)
		{
			t0 = INT_MAX;
			t1 = INT_MIN;
			string r = e.first;
			Timetable tb = e.second;

			for (auto &f : tb)
			{
				if (f.t0 < t0 && f.t0 != INT_MIN)
				{
					t0 = f.t0;
				}
				if (f.t1 > t1 && f.t1 != INT_MAX)
				{
					t1 = f.t1;
				}
			}

			o.rot[e.first].push_back(Frame(t0, t1));
		}
	}

	CHECK(o.rot.verify());

	// BUGGY
	// update all constraints related to its children
	// for (auto &c : o.children0)
	// {
	// 	int scheduled_time = _desc.get_operation(c).scheduled_time;
	// 	vector<Constraint *> list_src = _desc.get_src_constraints(c);
	// 	for (auto cc : list_src)
	// 	{
	// 		long lo = cc->d_lo;
	// 		long hi = cc->d_hi;
	// 		if (lo > INT_MIN)
	// 		{
	// 			lo = lo - scheduled_time;
	// 		}
	// 		if (hi < INT_MAX)
	// 		{
	// 			hi = hi - scheduled_time;
	// 		}
	// 		if (lo < INT_MIN)
	// 		{
	// 			lo = INT_MIN;
	// 		}
	// 		if (hi > INT_MAX)
	// 		{
	// 			hi = INT_MAX;
	// 		}
	// 		cc->d_lo = scheduled_time;
	// 		cc->d_hi = scheduled_time;
	// 	}

	// 	vector<Constraint *> list_dest = _desc.get_dest_constraints(c);
	// 	for (auto cc : list_dest)
	// 	{
	// 		long lo = cc->d_lo;
	// 		long hi = cc->d_hi;
	// 		if (lo > INT_MIN)
	// 		{
	// 			lo = lo + scheduled_time;
	// 		}
	// 		if (hi < INT_MAX)
	// 		{
	// 			hi = hi + scheduled_time;
	// 		}
	// 		if (lo < INT_MIN)
	// 		{
	// 			lo = INT_MIN;
	// 		}
	// 		if (hi > INT_MAX)
	// 		{
	// 			hi = INT_MAX;
	// 		}
	// 		cc->d_lo = scheduled_time;
	// 		cc->d_hi = scheduled_time;
	// 	}
	// }

	// for (auto &c : o.children1)
	// {
	// 	int scheduled_time = _desc.get_operation(c).scheduled_time;
	// 	vector<Constraint *> list_src = _desc.get_src_constraints(c);
	// 	for (auto cc : list_src)
	// 	{
	// 		long lo = cc->d_lo;
	// 		long hi = cc->d_hi;
	// 		if (lo > INT_MIN)
	// 		{
	// 			lo = lo - scheduled_time;
	// 		}
	// 		if (hi < INT_MAX)
	// 		{
	// 			hi = hi - scheduled_time;
	// 		}
	// 		if (lo < INT_MIN)
	// 		{
	// 			lo = INT_MIN;
	// 		}
	// 		if (hi > INT_MAX)
	// 		{
	// 			hi = INT_MAX;
	// 		}
	// 		cc->d_lo = scheduled_time;
	// 		cc->d_hi = scheduled_time;
	// 	}

	// 	vector<Constraint *> list_dest = _desc.get_dest_constraints(c);
	// 	for (auto cc : list_dest)
	// 	{
	// 		long lo = cc->d_lo;
	// 		long hi = cc->d_hi;
	// 		if (lo > INT_MIN)
	// 		{
	// 			lo = lo + scheduled_time;
	// 		}
	// 		if (hi < INT_MAX)
	// 		{
	// 			hi = hi + scheduled_time;
	// 		}
	// 		if (lo < INT_MIN)
	// 		{
	// 			lo = INT_MIN;
	// 		}
	// 		if (hi > INT_MAX)
	// 		{
	// 			hi = INT_MAX;
	// 		}
	// 		cc->d_lo = scheduled_time;
	// 		cc->d_hi = scheduled_time;
	// 	}
	// }

	return true;
}

bool Scheduler::schedule_hierarchical_dependency_graph(vector<string> name_list_)
{
	for (auto &n : name_list_)
	{
		CHECK(schedule_vertex(n));
	}

	CHECK(schedule_dependency_graph(name_list_));
	return true;
}

bool Scheduler::schedule_dependency_graph(vector<string> name_list_)
{
	Graph g0;
	map<string, boost::graph_traits<Graph>::vertex_descriptor> gdict0;
	for (auto &v : name_list_)
	{
		Operation o = _desc.get_operation(v);
		o.children0 = {};
		gdict0[v] = add_vertex(o, g0);
	}
	for (auto &v : name_list_)
	{
		vector<Constraint *> list = _desc.get_all_constraints();
		for (auto &c : list)
		{
			if (find(name_list_.begin(), name_list_.end(), c->src()) != name_list_.end() && find(name_list_.begin(), name_list_.end(), c->dest()) != name_list_.end())
			{
				add_edge(gdict0[c->src()], gdict0[c->dest()], *c, g0);
			}
		}
	}

	// FIXME: remove_redundent_link has bug, sometimes it will stack at a infinit loop
	//Graph g1 = remove_redundent_links(g0);
	Graph g1;
	copy_graph(g0, g1);
	map<string, boost::graph_traits<Graph>::vertex_descriptor> gdict1;
	graph_traits<Graph>::vertex_iterator vi, vi_end;
	for (tie(vi, vi_end) = vertices(g1); vi != vi_end; vi++)
	{
		gdict1[g1[*vi].name()] = *vi;
	}
	Graph g2 = pack_hard_links(g1);
	for (tie(vi, vi_end) = vertices(g2); vi != vi_end; vi++)
	{
		g2[*vi].scheduled_time = INT_MIN;
	}
	// cout << "before ----------------------------------------" << endl;
	// cout << to_dot(g2) << endl;
	g2 = eliminate_negative_links(g2);

	g2 = predict_order(g2);
	// cout << "after ----------------------------------------" << endl;
	// cout << to_dot(g2) << endl;

	// set all scheduled_time to INT_MIN to mark them as "unscheduled"
	for (tie(vi, vi_end) = vertices(g2); vi != vi_end; vi++)
	{
		g2[*vi].scheduled_time = INT_MIN;
	}

	Rot global_rot;
	int min_schedule_time = INT_MAX;

	SCHEDULE_ENGINE_NAME eng(&_desc);
	CHECK(eng.schedule_graph(g2, global_rot, min_schedule_time));

	for (tie(vi, vi_end) = vertices(g2); vi != vi_end; vi++)
	{
		g2[*vi].scheduled_time += g2[*vi].shift_factor;
	}

	int offset = INT_MAX;
	for (tie(vi, vi_end) = vertices(g2); vi != vi_end; vi++)
	{
		if (g2[*vi].scheduled_time < offset)
		{
			offset = g2[*vi].scheduled_time;
		}
	}

	for (tie(vi, vi_end) = vertices(g2); vi != vi_end; vi++)
	{

		if (g2[*vi].children0.size() == 0)
		{
			g0[gdict0[g2[*vi].name()]].scheduled_time += g2[*vi].scheduled_time - offset;
		}
		else
		{
			for (string c : g2[*vi].children0)
			{
				g0[gdict0[c]].scheduled_time += (g2[*vi].scheduled_time + g1[gdict1[c]].scheduled_time - offset);
			}
		}
	}

	Rot rot0;
	for (tie(vi, vi_end) = vertices(g0); vi != vi_end; vi++)
	{
		rot0.merge(g0[*vi].rot, g0[*vi].scheduled_time);
	}
	CHECK(rot0.verify());

	offset = INT_MAX;
	for (auto &r : rot0)
	{
		for (auto &f : r.second)
		{
			if (offset > f.pos())
			{
				offset = f.pos();
			}
		}
	}

	for (tie(vi, vi_end) = vertices(g0); vi != vi_end; vi++)
	{
		_desc.get_mutable_operation(g0[*vi].name()).scheduled_time = g0[*vi].scheduled_time - offset;

		// If the scheduled_time less than 0, it means the vertex is a dummy vertex that don't have any ROT entry. So just force it to be scheduled to clock 0;
		if (_desc.get_operation(g0[*vi].name()).scheduled_time < 0)
		{
			_desc.get_mutable_operation(g0[*vi].name()).scheduled_time = 0;
		}
	}

	return true;
}

void Scheduler::update_scheduled_time(string name_)
{
	if (name_ == "")
	{
		name_ = _desc.entry();
	}
	Operation &o = _desc.get_mutable_operation(name_);
	for (auto &c : o.children0)
	{
		Operation &co = _desc.get_mutable_operation(c);
		co.scheduled_time += o.scheduled_time;
		update_scheduled_time(c);
	}
	for (auto &c : o.children1)
	{
		Operation &co = _desc.get_mutable_operation(c);
		co.scheduled_time += o.scheduled_time;
		update_scheduled_time(c);
	}
}

void Scheduler::merge_hierarchical_blocks_with_hard_link()
{
	bool flag_changed = true;
	while (flag_changed)
	{
		flag_changed = false;
		vector<string> vec = _desc.get_all_operation_names();

		for (int i = 0; i < vec.size(); i++)
		{
			Operation o1 = _desc.get_operation(vec[i]);
			if (!o1.has_child())
			{
				continue;
			}

			if (o1.children1.size() > 0)
			{
				// Don't care about IF-THEN-ELSE statement
				continue;
			}

			for (int j = 0; j < o1.children0.size(); j++)
			{

				Operation o2 = _desc.get_operation(o1.children0[j]);

				vector<Constraint *> vc = _desc.get_src_constraints(o2.name());
				for (auto &c : vc)
				{
					if (c->d_lo == INT_MIN || c->d_hi == INT_MAX)
					{
						continue;
					}
					Operation o3 = _desc.get_operation(c->src());

					for (int k = 0; k < vec.size(); k++)
					{
						if (k == j || k == i)
						{
							continue;
						}
						Operation o4 = _desc.get_operation(vec[k]);
						if (!o4.has_child())
						{
							continue;
						}
						if (o4.children1.size() > 0)
						{
							// Don't care about IF-THEN-ELSE statement
							continue;
						}
						bool flag_merge = false;
						for (int l = 0; l < o4.children0.size(); l++)
						{
							if (o4.children0[l] == o3.name())
							{
								flag_merge = true;
								break;
							}
						}
						if (flag_merge)
						{
							flag_changed = true;
							Operation &mo1 = _desc.get_mutable_operation(o1.name());
							Operation &mo4 = _desc.get_mutable_operation(o4.name());
							for (auto &n : mo4.children0)
							{
								mo1.add_child(n, 0);
							}
							mo4.children0.clear();
							goto AGAIN;
						}
					}
				}
			}
		}
	AGAIN:;
	}
}

void Scheduler::to_dot_graph(Descriptor &d_, int stage_)
{
	vesyla::util::GlobalVar glv;
	string path;
	string input_file_name;
	CHECK(glv.gets("$OUTPUT_DIR", path));
	path = path + "schedule/";
	mkdir(path.c_str(), 0755);
	CHECK(glv.gets("input_file_name", input_file_name));
	string path_dot;
	path_dot = path + input_file_name + "_e" + to_string(stage_) + ".dot";
	ofstream ofs0(path_dot, ofstream::out);
	ofs0 << d_.generate_dot_graph();
	ofs0.close();
}

} // namespace schedule
} // namespace vesyla
