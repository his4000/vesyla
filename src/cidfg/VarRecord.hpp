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

#ifndef __VESYLA_CIDFG_VAR_RECORD_HPP__
#define __VESYLA_CIDFG_VAR_RECORD_HPP__

#include "util/Common.hpp"
#include "Edge.hpp"
#include "VIR/Coordinate.hpp"
#include <set>

namespace vesyla
{
namespace cidfg
{
class VarRecord
{
public:
  int bind_vertex_id;
  int bind_vertex_port;
  string domain_signature;
  Edge::EdgeType edge_type;
  VIR::Coordinate coord;
  bool isfixed;
  std::set<int> referenced_by;
  int start_vertex_id;
  int size_vertex_id;

public:
  VarRecord();
  ~VarRecord();
};
} // namespace cidfg
} // namespace vesyla

#endif // __VESYLA_CIDFG_VAR_RECORD_HPP__