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

#include "VarTable.hpp"

namespace vesyla
{
namespace cidfg
{
bool VarTable::exist(string name_)
{
  if (_table.find(name_) != _table.end())
  {
    return true;
  }
  else
  {
    return false;
  }
}

void VarTable::update_var(string name_, int bind_vertex_id_, int bind_vertex_port_, string domain_signature_, Edge::EdgeType edge_type_, VIR::Coordinate coord_, bool isfixed_, int start_vertex_id_, int size_vertex_id_)
{
  if (exist(name_))
  {
    _table[name_].bind_vertex_id = bind_vertex_id_;
    _table[name_].bind_vertex_port = bind_vertex_port_;
    _table[name_].referenced_by.clear();
  }
  else
  {
    VarRecord vr;
    vr.bind_vertex_id = bind_vertex_id_;
    vr.bind_vertex_port = bind_vertex_port_;
    vr.domain_signature = domain_signature_;
    vr.edge_type = edge_type_;
    vr.coord = coord_;
    vr.isfixed = isfixed_;
    vr.referenced_by.clear();
    vr.start_vertex_id = start_vertex_id_;
    vr.size_vertex_id = size_vertex_id_;
    _table[name_] = vr;
  }
}

void VarTable::delete_vars(string domain_signature_)
{
  vector<string> names;
  for (auto &e : _table)
  {
    if (e.second.domain_signature == domain_signature_)
    {
      names.push_back(e.first);
    }
  }
  for (auto &n : names)
  {
    _table.erase(n);
  }
}

int VarTable::get_vertex_id(string name_)
{
  CHECK(exist(name_));
  return _table[name_].bind_vertex_id;
}
int VarTable::get_vertex_port(string name_)
{
  CHECK(exist(name_));
  return _table[name_].bind_vertex_port;
}
VIR::Coordinate VarTable::get_vertex_coord(string name_)
{
  CHECK(exist(name_));
  return _table[name_].coord;
}

Edge::EdgeType VarTable::get_edge_type(string name_)
{
  CHECK(exist(name_));
  return _table[name_].edge_type;
}

bool VarTable::get_isfixed(string name_)
{
  CHECK(exist(name_));
  return _table[name_].isfixed;
}

vector<string> VarTable::names()
{
  vector<string> n;
  for (auto &r : _table)
  {
    n.push_back(r.first);
  }
  return n;
}

std::set<int> VarTable::referenced_by(string name_)
{
  CHECK(exist(name_));
  return _table[name_].referenced_by;
}
void VarTable::reference(string name_, int id)
{
  CHECK(exist(name_));
  _table[name_].referenced_by.insert(id);
}
void VarTable::clear_reference(string name_)
{
  CHECK(exist(name_));
  _table[name_].referenced_by.clear();
}

int VarTable::get_start_vertex_id(string name_)
{
  CHECK(exist(name_));
  return _table[name_].start_vertex_id;
}

int VarTable::get_size_vertex_id(string name_)
{
  CHECK(exist(name_));
  return _table[name_].size_vertex_id;
}

void VarTable::clear()
{
  _table.clear();
}

} // namespace cidfg
} // namespace vesyla