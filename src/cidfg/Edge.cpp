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

#include "Edge.hpp"

namespace vesyla
{
namespace cidfg
{
int Edge::temp_name_counter = 0;
Edge::Edge() : edge_type(SCALAR_DATA_SIG), d_lo(1), d_hi(INT_MAX)
{
  var_name = "temp_var_" + to_string(temp_name_counter);
  temp_name_counter++;
}
Edge::Edge(int src_id_, int src_port_, int dest_id_, int dest_port_, string var_name_, EdgeType edge_type_, int d_lo_, int d_hi_) : src_id(src_id_), src_port(src_port_), dest_id(dest_id_), dest_port(dest_port_), edge_type(edge_type_), d_lo(d_lo_), d_hi(d_hi_)
{
  if (var_name_ == "")
  {
    var_name = "temp_var_" + to_string(temp_name_counter);
    temp_name_counter++;
  }
  else
  {
    var_name = var_name_;
  }
}
Edge::~Edge()
{
}

bool Edge::is_weak_dependency()
{
  if (edge_type != DEPENDENCY)
  {
    return false;
  }

  string ending = "_weak";
  if (var_name.length() >= ending.length())
  {
    return (0 == var_name.compare(var_name.length() - ending.length(), ending.length(), ending));
  }
  else
  {
    return false;
  }
}
} // namespace cidfg
} // namespace vesyla