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

#ifndef __VESYLA_SCHEDULE_SCHEDULER_HPP__
#define __VESYLA_SCHEDULE_SCHEDULER_HPP__

#include <util/Common.hpp>
#include "Frame.hpp"
#include "Timetable.hpp"
#include "Rot.hpp"
#include "DependencyGraph.hpp"
#include "Descriptor.hpp"
#include "NaiveEngine.hpp"
#include "EnhancedBnbEngine.hpp"

#include <boost/graph/transitive_closure.hpp>
#include <boost/graph/transitive_reduction.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/copy.hpp>

using json = nlohmann::json;

namespace vesyla
{
namespace schedule
{
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, Operation, Constraint> Graph;
typedef string Vertex;
typedef pair<string, string> Edge;

class Scheduler
{
public:
	DependencyGraph::Vertex root;

private:
	map<string, DependencyGraph::Vertex *> _lut;
	Descriptor _desc;
	int _id_counter;

public:
	Scheduler() { _id_counter = 0; }
	Scheduler(Descriptor desc_) : _desc(desc_) { _id_counter = 0; }
	void load_from_json(string _input_str);
	void schedule();
	void schedule_legacy();
	void add_to_dict(DependencyGraph::Vertex &v_);
	void add_to_dict(DependencyGraph &g_);
	int get_start_time(string path);
	int get_offset(string path);
	void dump_all()
	{
		for (auto &n : _lut)
		{
			cout << n.first << ":" << n.second->start << ", " << n.second->pack_id << ", " << n.second->pack_offset << ", " << n.second->offset << endl;
		}
	}

	int get_unique_id();
	Graph pack_hard_links(Graph &g0);
	Graph remove_redundent_links(Graph &g0);
	Graph eliminate_negative_links(Graph &g0);
	Graph predict_order(Graph &g0);
	bool schedule_graph(Graph &g, Rot &global_rot_in, int &min_end_time);
	bool schedule_vertex(string name_);
	bool schedule_dependency_graph(vector<string> name_list_);
	bool schedule_hierarchical_dependency_graph(vector<string> name_list_);
	void update_scheduled_time(string name_ = "");
	Descriptor &desc()
	{
		return _desc;
	}

	int get_scheduled_time(string name_)
	{
		return _desc.get_operation(name_).scheduled_time;
	}

	int get_end_time()
	{
		return _desc.get_max_schedule_time();
	}

private:
	void adjust_interblock_constraints(string parent_block_, vector<string> name_list_);
	void merge_hierarchical_blocks_with_hard_link();
	void to_dot_graph(Descriptor &d_, int stage_);
};

} // namespace schedule
} // namespace vesyla

#endif // __VESYLA_SCHEDULE_SCHEDULER_HPP__
