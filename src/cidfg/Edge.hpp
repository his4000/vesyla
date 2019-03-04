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

#ifndef __VESYLA_CIDFG_EDGE_HPP__
#define __VESYLA_CIDFG_EDGE_HPP__

#include "Object.hpp"

namespace vesyla
{
namespace cidfg
{
class Edge : public Object
{
public:
  enum EdgeType
  {
    SCALAR_DATA_SIG,
    VECTOR_DATA_SIG,
    SRAM_VECTOR_DATA_SIG,
    SCALAR_CONTROL_SIG,
    DEPENDENCY
  };

public:
  int src_id;
  int src_port;
  int dest_id;
  int dest_port;
  string var_name;
  EdgeType edge_type;
  int d_lo;
  int d_hi;
  static int temp_name_counter;

public:
  Edge();
  Edge(int src_id_, int src_port_, int dest_id_, int dest_port_, string var_name_ = "", EdgeType edge_type_ = SCALAR_DATA_SIG, int d_lo_ = 1, int d_hi_ = INT_MAX);
  ~Edge();

  bool is_weak_dependency();
};
} // namespace cidfg
} // namespace vesyla

#endif // __VESYLA_CIDFG_EDGE_HPP__