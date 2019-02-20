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

#ifndef __VESYLA_SCHEDULE_NAIVE_ENGINE_HPP__
#define __VESYLA_SCHEDULE_NAIVE_ENGINE_HPP__

#include "Engine.hpp"
#include <boost/graph/graph_traits.hpp>

namespace vesyla
{
namespace schedule
{
class NaiveEngine : public Engine
{
public:
	NaiveEngine();
	NaiveEngine(Descriptor *ptr_desc_);
	~NaiveEngine();

public:
	bool schedule_graph(Graph &g, Rot &global_rot_in, int &min_end_time);
};
} // namespace schedule
} // namespace vesyla

#endif // __VESYLA_SCHEDULE_NAIVE_ENGINE_HPP__