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

#ifndef __VESYLA_CIDFG_VAR_TABLE_HPP__
#define __VESYLA_CIDFG_VAR_TABLE_HPP__

#include "util/Common.hpp"
#include "VarRecord.hpp"
#include "Edge.hpp"

namespace vesyla
{
namespace cidfg
{
class VarTable
{
private:
  std::unordered_map<string, VarRecord> _table;

public:
  bool exist(string name_);
  void update_var(string name_, int bind_vertex_id_, int bind_vertex_port_, string domain_signature_, Edge::EdgeType edge_type_ = Edge::SCALAR_DATA_SIG, VIR::Coordinate coord_ = VIR::Coordinate(), bool isfixed_ = false, int start_vertex_id_ = -1, int size_vertex_id_ = -1);
  void delete_vars(string domain_signature_);
  int get_vertex_id(string name_);
  int get_vertex_port(string name_);
  bool get_isfixed(string name_);
  VIR::Coordinate get_vertex_coord(string name_);
  Edge::EdgeType get_edge_type(string name_);
  int get_start_vertex_id(string name_);
  int get_size_vertex_id(string name_);
  vector<string> names();
  std::set<int> referenced_by(string name_);
  void reference(string name_, int id);
  void clear_reference(string name_);
  void clear();
};
} // namespace cidfg
} // namespace vesyla

#endif // __VESYLA_CIDFG_VAR_TABLE_HPP__