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

#ifndef __VESYLA_CIDFG_CONVERTER_HPP__
#define __VESYLA_CIDFG_CONVERTER_HPP__

#include "CidfgGraph.hpp"
#include "ScriptGenerator.hpp"
#include "VarTable.hpp"
#include "VIR/VIRIncludeAll.hpp"
#include <queue>

using namespace VIR;
using namespace VIR::VirEnumerations;

namespace vesyla
{
namespace cidfg
{
class Converter
{
public:
  struct VarLocInfo
  {
    string var_name;
    int var_type;
    VIR::Coordinate coord;
    int start;
    int size;
  };

  struct Anchor
  {
    int id;
    int port;
  };

  typedef std::unordered_map<string, int> StorageAllocationMap;

private:
  std::unordered_map<string, VIR::Coordinate> cdpu_vars;
  stack<std::set<string>> _raccu_vars;
  vector<string> _domain_signatures;
  bool _is_rhs;
  bool _is_sram;
  int _loop_id_counter;
  std::unordered_map<string, VarLocInfo> _var_map;
  bool _res_share_on;
  VarTable _vt_res_share;
  bool _is_weak_dep;
  int _loop_signature_counter;

public:
  Converter();
  void convert(MainProgram *p_, CidfgGraph &g_);

public:
  Anchor convert_expression(Expression *e_, CidfgGraph &g_, VarTable &t_, StorageAllocationMap &reg_allocation_, StorageAllocationMap &sram_allocation_, int parent_vertex_id_ = -1, int child_index_ = 0);

private:
  int convert_slicename(SliceName *e_, CidfgGraph &g_, VarTable &t_, StorageAllocationMap &reg_allocation_, StorageAllocationMap &sram_allocation_, bool is_read_, bool is_sram_, int sram_block_size = 16, int parent_vertex_id_ = -1, int child_index_ = 0);
  int convert_assignment_statement(AssignmentStatement *e_, CidfgGraph &g_, VarTable &t_, StorageAllocationMap &reg_allocation_, StorageAllocationMap &sram_allocation_, int parent_vertex_id_ = -1, int child_index_ = 0);
  int convert_integer(Integer *e_, CidfgGraph &g_, VarTable &t_, StorageAllocationMap &reg_allocation_, StorageAllocationMap &sram_allocation_, int parent_vertex_id_ = -1, int child_index_ = 0);
  int convert_floating_point(FloatingPoint *e_, CidfgGraph &g_, VarTable &t_, StorageAllocationMap &reg_allocation_, StorageAllocationMap &sram_allocation_, int parent_vertex_id_ = -1, int child_index_ = 0);
  void convert_if_statement(IfStatement *e_, CidfgGraph &g_, VarTable &t_, StorageAllocationMap &reg_allocation_, StorageAllocationMap &sram_allocation_, int parent_vertex_id_ = -1, int child_index_ = 0);
  void convert_for_statement(ForStatement *e_, CidfgGraph &g_, VarTable &t_, StorageAllocationMap &reg_allocation_, StorageAllocationMap &sram_allocation_, int parent_vertex_id_ = -1, int child_index_ = 0);
  void convert_pragma_statement(PragmaStatement *e_, CidfgGraph &g_, VarTable &t_, StorageAllocationMap &reg_allocation_, StorageAllocationMap &sram_allocation_, int parent_vertex_id_ = -1, int child_index_ = 0);
  void convert_statement(Statement *st_, CidfgGraph &g_, VarTable &t_, StorageAllocationMap &reg_allocation_, StorageAllocationMap &sram_allocation_, int parent_vertex_id_ = -1, int child_index_ = 0);
  int convert_register_transfer_statement(VIR::AssignmentStatement *e_, CidfgGraph &g_, VarTable &t_, StorageAllocationMap &reg_allocation_, StorageAllocationMap &sram_allocation_, int parent_vertex_id_, int child_index_);
  int convert_memory_transfer_statement(VIR::AssignmentStatement *e_, CidfgGraph &g_, VarTable &t_, StorageAllocationMap &reg_allocation_, StorageAllocationMap &sram_allocation_, int parent_vertex_id_, int child_index_);
  int convert_arithmetic_assign_statement_0(VIR::AssignmentStatement *e_, CidfgGraph &g_, VarTable &t_, StorageAllocationMap &reg_allocation_, StorageAllocationMap &sram_allocation_, int parent_vertex_id_, int child_index_);
  int convert_arithmetic_assign_statement_1(VIR::AssignmentStatement *e_, CidfgGraph &g_, VarTable &t_, StorageAllocationMap &reg_allocation_, StorageAllocationMap &sram_allocation_, int parent_vertex_id_, int child_index_);
  int convert_dpu_chain_statement_0(VIR::AssignmentStatement *e_, CidfgGraph &g_, VarTable &t_, StorageAllocationMap &reg_allocation_, StorageAllocationMap &sram_allocation_, int parent_vertex_id_, int child_index_);
  int convert_dpu_chain_statement_1(VIR::AssignmentStatement *e_, CidfgGraph &g_, VarTable &t_, StorageAllocationMap &reg_allocation_, StorageAllocationMap &sram_allocation_, int parent_vertex_id_, int child_index_);

  int convert_reg_declarative_statement(AssignmentStatement *e_, CidfgGraph &g_, VarTable &t_, StorageAllocationMap &reg_allocation_, StorageAllocationMap &sram_allocation_, int parent_vertex_id_, int child_index_);
  int convert_sram_declarative_statement(AssignmentStatement *e_, CidfgGraph &g_, VarTable &t_, StorageAllocationMap &reg_allocation_, StorageAllocationMap &sram_allocation_, int parent_vertex_id_, int child_index_);

  void find_all_identifier(VIR::Expression *e_, vector<string> &identifier_name_list_);
  void find_all_children(CidfgGraph &g_, int id_, int child_index_, std::set<int> &children);

  void gen_script();

  // public:
  // void merge_rw_index(CidfgGraph &g_);
  // void merge_rw_index(CidfgGraph &g_, set<int> &children_id_, int vertex_id_ = -1, int parent_vertex_id_ = -1, int child_index_ = -1);
  // void remove_source_sink(CidfgGraph &g_);
};
} // namespace cidfg
} // namespace vesyla

#endif // __VESYLA_CIDFG_CONVERTER_HPP__