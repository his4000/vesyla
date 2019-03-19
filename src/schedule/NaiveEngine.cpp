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

#include "NaiveEngine.hpp"

using namespace boost;

namespace vesyla
{
namespace schedule
{
NaiveEngine::NaiveEngine()
{
}
NaiveEngine::NaiveEngine(Descriptor *desc_)
{
}
NaiveEngine::~NaiveEngine()
{
}
bool NaiveEngine::schedule_graph(Graph &g, Rot &global_rot_in, int &min_end_time)
{
	// Early return if timing of current solution is worse than the known solution
	for (auto &r : global_rot_in)
	{
		for (auto &f : r.second)
		{
			if (f.pos() >= min_end_time)
			{
				return false;
			}
		}
	}

	// Return if solution is found
	bool flag_all_scheduled = true;
	graph_traits<Graph>::vertex_iterator vi, vi_end;
	for (tie(vi, vi_end) = vertices(g); vi != vi_end; vi++)
	{
		if (g[*vi].scheduled_time == INT_MIN)
		{
			flag_all_scheduled = false;
			break;
		}
	}

	if (flag_all_scheduled)
	{
		int min_end_time_temp = 0;
		for (auto &r : global_rot_in)
		{
			int end_time = (max_element(r.second.begin(), r.second.end(),
																	[](Frame a, Frame b) -> bool {
																		return (a.max_pos() < b.max_pos());
																	}))
												 ->max_pos();
			if (end_time > min_end_time_temp)
			{
				min_end_time_temp = end_time;
			}
		}
		min_end_time = min_end_time_temp;
		return true;
	}

	Graph solution;
	copy_graph(g, solution);
	bool flag_solution_found = false;
	for (tie(vi, vi_end) = vertices(g); vi != vi_end; vi++)
	{

		if (g[*vi].scheduled_time != INT_MIN)
		{
			continue;
		}

		// check its predecessors
		// find the min and max scheduled time for current vertex
		int min_schedule_time = -g[*vi].shift_factor;
		int max_schedule_time = INT_MAX;
		bool flag_predecessor_not_scheduled = false;
		graph_traits<Graph>::in_edge_iterator iei, iei_end;
		for (tie(iei, iei_end) = in_edges(*vi, g); iei != iei_end; iei++)
		{
			if (g[source(*iei, g)].scheduled_time == INT_MIN && g[*iei].d_lo > 0)
			{
				flag_predecessor_not_scheduled = true;
				break;
			}
			if (g[source(*iei, g)].scheduled_time != INT_MIN)
			{
				long th = long(g[*iei].d_hi) + long(g[source(*iei, g)].scheduled_time);
				long tl = long(g[*iei].d_lo) + long(g[source(*iei, g)].scheduled_time);
				if (th < max_schedule_time)
				{
					max_schedule_time = th;
				}
				if (tl > min_schedule_time)
				{
					min_schedule_time = tl;
				}
			}
		}
		if (flag_predecessor_not_scheduled)
		{
			continue;
		}

		graph_traits<Graph>::out_edge_iterator oei, oei_end;
		for (tie(oei, oei_end) = out_edges(*vi, g); oei != oei_end; oei++)
		{
			if (g[target(*oei, g)].scheduled_time != INT_MIN)
			{
				long tl = long(g[target(*oei, g)].scheduled_time - long(g[*oei].d_hi));
				long th = long(g[target(*oei, g)].scheduled_time - long(g[*oei].d_lo));
				if (th < max_schedule_time)
				{
					max_schedule_time = th;
				}
				if (tl > min_schedule_time)
				{
					min_schedule_time = tl;
				}
			}
		}

		// int min_frame = INT_MAX;
		// for (auto &r : g[*vi].rot)
		// {
		// 	for (auto &f : r.second)
		// 	{
		// 		if (f.pos() < min_frame)
		// 		{
		// 			min_frame = f.pos();
		// 		}
		// 	}
		// }

		// if (min_frame < 0)
		// {
		// 	min_schedule_time -= min_frame;
		// }

		if (min_schedule_time > max_schedule_time)
		{
			continue;
		}
		// LOG(DEBUG) << "Schedule vertex11111111111111111111 " << g[*vi].name() << " Range [" << min_schedule_time << ", " << max_schedule_time << "]";
		// LOG(DEBUG) << "old ROT" << endl
		// 					 << global_rot_in.dump();
		// LOG(DEBUG) << "add ROT" << endl
		// 					 << g[*vi].rot.dump();
		Rot new_global_rot = global_rot_in;
		int schedule_time = new_global_rot.merge_and_verify(g[*vi].rot, min_schedule_time, max_schedule_time);
		if (schedule_time == INT_MIN)
		{
			continue;
		}

		Graph new_g;
		copy_graph(g, new_g);
		new_g[*vi].scheduled_time = schedule_time;
		if (!schedule_graph(new_g, new_global_rot, min_end_time))
		{
			//continue;
			// directly return
			return false;
		}

		// found a solution
		flag_solution_found = true;
		graph_traits<Graph>::vertex_iterator vii, vii_end;
		for (tie(vii, vii_end) = vertices(solution); vii != vii_end; vii++)
		{
			solution[*vii].scheduled_time = new_g[*vii].scheduled_time;
		}

		break;
	}
	graph_traits<Graph>::vertex_iterator vii, vii_end;
	for (tie(vii, vii_end) = vertices(solution); vii != vii_end; vii++)
	{
		g[*vii].scheduled_time = solution[*vii].scheduled_time;
	}
	return flag_solution_found;
}

// bool NaiveEngine::schedule_graph(Graph &g, Rot &global_rot_in, int &min_end_time)
// {
// 	LOG(DEBUG) << "TOTAL VERTICES " << num_vertices(g);
// 	while (true)
// 	{
// 		// Return if solution is found
// 		vector<Graph::vertex_descriptor> vec_vd;
// 		bool flag_all_scheduled = true;
// 		graph_traits<Graph>::vertex_iterator vi, vi_end;
// 		for (tie(vi, vi_end) = vertices(g); vi != vi_end; vi++)
// 		{
// 			if (g[*vi].scheduled_time == INT_MIN)
// 			{
// 				flag_all_scheduled = false;
// 				vec_vd.push_back(*vi);
// 			}
// 		}

// 		if (flag_all_scheduled)
// 		{
// 			int min_end_time_temp = 0;
// 			for (auto &r : global_rot_in)
// 			{
// 				int end_time = (max_element(r.second.begin(), r.second.end(),
// 																		[](Frame a, Frame b) -> bool {
// 																			return (a.max_pos() < b.max_pos());
// 																		}))
// 													 ->max_pos();
// 				if (end_time > min_end_time_temp)
// 				{
// 					min_end_time_temp = end_time;
// 				}
// 			}
// 			min_end_time = min_end_time_temp;
// 			return true;
// 		}

// 		bool changed = false;
// 		for (auto &vd : vec_vd)
// 		{
// 			changed = schedule_one_node(vd, g, global_rot_in, min_end_time);
// 			if (changed)
// 			{
// 				break;
// 			}
// 		}

// 		if (changed == false)
// 		{
// 			return false;
// 		}
// 	}
// }

// bool NaiveEngine::schedule_one_node(Graph::vertex_descriptor vd, Graph &g_, Rot &global_rot_in, int &min_end_time)
// {
// 	if (g_[vd].scheduled_time != INT_MIN)
// 	{
// 		return false;
// 	}

// 	// check its predecessors
// 	// find the min and max scheduled time for current vertex
// 	int min_schedule_time = -g_[vd].shift_factor;
// 	int max_schedule_time = INT_MAX;
// 	bool flag_predecessor_not_scheduled = false;
// 	graph_traits<Graph>::in_edge_iterator iei, iei_end;
// 	for (tie(iei, iei_end) = in_edges(vd, g_); iei != iei_end; iei++)
// 	{
// 		if (g_[source(*iei, g_)].scheduled_time == INT_MIN && g_[*iei].d_lo > 0)
// 		{
// 			flag_predecessor_not_scheduled = true;
// 			break;
// 		}
// 		if (g_[source(*iei, g_)].scheduled_time != INT_MIN)
// 		{
// 			long th = long(g_[*iei].d_hi) + long(g_[source(*iei, g_)].scheduled_time);
// 			long tl = long(g_[*iei].d_lo) + long(g_[source(*iei, g_)].scheduled_time);
// 			if (th < max_schedule_time)
// 			{
// 				max_schedule_time = th;
// 			}
// 			if (tl > min_schedule_time)
// 			{
// 				min_schedule_time = tl;
// 			}
// 		}
// 	}
// 	if (flag_predecessor_not_scheduled)
// 	{
// 		return false;
// 	}

// 	graph_traits<Graph>::out_edge_iterator oei, oei_end;
// 	for (tie(oei, oei_end) = out_edges(vd, g_); oei != oei_end; oei++)
// 	{
// 		if (g_[target(*oei, g_)].scheduled_time != INT_MIN)
// 		{
// 			long tl = long(g_[target(*oei, g_)].scheduled_time - long(g_[*oei].d_hi));
// 			long th = long(g_[target(*oei, g_)].scheduled_time - long(g_[*oei].d_lo));
// 			if (th < max_schedule_time)
// 			{
// 				max_schedule_time = th;
// 			}
// 			if (tl > min_schedule_time)
// 			{
// 				min_schedule_time = tl;
// 			}
// 		}
// 	}

// 	if (min_schedule_time > max_schedule_time)
// 	{
// 		return false;
// 	}

// 	Rot new_global_rot = global_rot_in;
// 	int schedule_time = new_global_rot.merge_and_verify(g_[vd].rot, min_schedule_time, max_schedule_time);
// 	if (schedule_time == INT_MIN)
// 	{
// 		return false;
// 	}

// 	g_[vd].scheduled_time = schedule_time;
// 	global_rot_in = new_global_rot;
// 	return true;
// }

} // namespace schedule
} // namespace vesyla