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

#include "Converter.hpp"

namespace vesyla
{
namespace cidfg
{
Converter::Converter()
{
  _is_rhs = false;
  _is_sram = false;
  _res_share_on = false;
}

void Converter::find_all_children(CidfgGraph &g_, int id_, int child_index_, std::set<int> &children)
{
  Vertex *v = g_.get_vertex(id_);
  if (!v->is_hierarchical())
  {
    return;
  }

  HierarchicalVertex *hv = static_cast<HierarchicalVertex *>(v);
  if (hv->children.size() <= child_index_)
  {
    return;
  }

  for (auto &c : hv->children[child_index_])
  {
    children.insert(c);
    if (g_.get_vertex(c)->is_hierarchical())
    {
      HierarchicalVertex *vv = static_cast<HierarchicalVertex *>(g_.get_vertex(c));
      for (int i = 0; i < vv->children.size(); i++)
      {
        find_all_children(g_, c, i, children);
      }
    }
  }
}

void Converter::find_all_identifier(VIR::Expression *e_, vector<string> &identifier_name_list_)
{
  switch (e_->kind())
  {
  case VIR::VirEnumerations::ktIdentifier:
  {
    identifier_name_list_.push_back(static_cast<VIR::Identifier *>(e_)->name());
    break;
  }
  case VIR::VirEnumerations::ktSliceName:
  {
    identifier_name_list_.push_back(static_cast<VIR::SliceName *>(e_)->prefix()->name());
    break;
  }
  case VIR::VirEnumerations::ktPrimitiveFunctionCall:
  {
    for (auto &e : static_cast<VIR::PrimitiveFunctionCall *>(e_)->parameters())
    {
      find_all_identifier(e, identifier_name_list_);
    }
    break;
  }
  case VIR::VirEnumerations::ktUnaryExpression:
  {
    find_all_identifier(static_cast<VIR::UnaryExpression *>(e_)->operand(), identifier_name_list_);
    break;
  }
  case VIR::VirEnumerations::ktBinaryExpression:
  {
    find_all_identifier(static_cast<VIR::BinaryExpression *>(e_)->leftOperand(), identifier_name_list_);
    find_all_identifier(static_cast<VIR::BinaryExpression *>(e_)->rightOperand(), identifier_name_list_);
    break;
  }
  default:
  {
    break;
  }
  }
}

void Converter::convert(MainProgram *p_, CidfgGraph &g_)
{

  gen_script();

  _loop_id_counter = 0;
  _domain_signatures.clear();
  _domain_signatures.push_back("root");
  VarTable t;
  StorageAllocationMap reg_alloccation;
  StorageAllocationMap sram_allocation;

  RootVertex rv;
  int id_root = g_.add_vertex(rv);
  for (auto &s : p_->body())
  {
    convert_statement(static_cast<AssignmentStatement *>(s), g_, t, reg_alloccation, sram_allocation, id_root, 0);
  }

  std::unordered_map<string, int> source_vertices_map;
  vector<string> sink_names = t.names();
  for (auto &v : g_.get_vertices())
  {
    if (v->vertex_type == Vertex::REG_VAR_VERTEX)
    {
      RegVarVertex *vv = static_cast<RegVarVertex *>(v);
      source_vertices_map[vv->var_name] = vv->id;
    }
    else if (v->vertex_type == Vertex::SRAM_VAR_VERTEX)
    {
      SramVarVertex *vv = static_cast<SramVarVertex *>(v);
      source_vertices_map[vv->var_name] = vv->id;
    }
    else if (v->vertex_type == Vertex::VAR_VERTEX)
    {
      VarVertex *vv = static_cast<VarVertex *>(v);
      source_vertices_map[vv->var_name] = vv->id;
    }
  }

  for (auto &n : t.names())
  {
    string var_name = n;
    int id = t.get_vertex_id(n);
    int port = t.get_vertex_port(n);
    VIR::Coordinate coord = t.get_vertex_coord(n);
    if (g_.get_out_edge(id, port) < 0)
    {
      Vertex *v = g_.get_vertex(source_vertices_map[var_name]);
      if (v->vertex_type == Vertex::REG_VAR_VERTEX)
      {
        RegSinkVertex vv;
        vv.var_name = n;
        int id_v = g_.add_vertex(vv, id_root, 0);
        Edge e(id, port, id_v, 0, n, t.get_edge_type(n));
        g_.add_edge(e);
      }
      else if (v->vertex_type == Vertex::SRAM_VAR_VERTEX)
      {
        SramSinkVertex vv;
        vv.var_name = n;
        vv.sram_coord = static_cast<SramVarVertex *>(v)->sram_coord;
        int id_v = g_.add_vertex(vv, id_root, 0);
        Edge e(id, port, id_v, 0, n, t.get_edge_type(n));
        g_.add_edge(e);
      }
      if (v->vertex_type == Vertex::VAR_VERTEX)
      {
        VarSinkVertex vv;
        vv.var_name = n;
        LOG(DEBUG) << "var vertex " << vv.var_name;
        int id_v = g_.add_vertex(vv, id_root, 0);
        Edge e(id, port, id_v, 0, n, t.get_edge_type(n));
        g_.add_edge(e);
      }
    }
  }

  util::GlobalVar glv;
  string path;
  CHECK(glv.gets("$OUTPUT_DIR", path));
  path = path + "cidfg/";
  mkdir(path.c_str(), 0755);
  string input_file_name;
  CHECK(glv.gets("input_file_name", input_file_name));
  string path_dot;
  path_dot = path + input_file_name + ".dot";
  ofstream ofs0(path_dot, ofstream::out);
  ofs0 << g_.generate_dot_graph();
  ofs0.close();
}

void Converter::convert_statement(Statement *st_, CidfgGraph &g_, VarTable &t_, StorageAllocationMap &reg_allocation_, StorageAllocationMap &sram_allocation_, int parent_vertex_id_, int child_index_)
{
  switch (st_->kind())
  {
  case ktAssignmentStatement:
    convert_assignment_statement(static_cast<AssignmentStatement *>(st_), g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
    break;
  case ktIfStatement:
  case ktIfThenElseStatement:
    convert_if_statement(static_cast<IfStatement *>(st_), g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
    break;
  case ktForStatement:
    convert_for_statement(static_cast<ForStatement *>(st_), g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
    break;
  case ktPragmaStatement:
    convert_pragma_statement(static_cast<PragmaStatement *>(st_), g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
    break;
  default:
    LOG(FATAL) << "Unsupported statement type: " << st_->kindStr();
    break;
  }
}

Converter::Anchor Converter::convert_expression(Expression *e_, CidfgGraph &g_, VarTable &t_, StorageAllocationMap &reg_allocation_, StorageAllocationMap &sram_allocation_, int parent_vertex_id_, int child_index_)
{
  int ret_id = -1;
  int ret_port = 0;
  switch (e_->kind())
  {
  case ktInteger:
  {
    ret_id = convert_integer(static_cast<Integer *>(e_), g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
    break;
  }
  case ktFloatingPoint:
  {
    ret_id = convert_floating_point(static_cast<FloatingPoint *>(e_), g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
    break;
  }
  case ktBinaryExpression:
  {
    BinaryExpression *e = static_cast<BinaryExpression *>(e_);
    ComputationVertex cv;
    cv.func_name = e->operatorStr();
    int id_e = g_.add_vertex(cv, parent_vertex_id_, child_index_);
    Anchor id_l = convert_expression(e->leftOperand(), g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
    Anchor id_r = convert_expression(e->rightOperand(), g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
    Edge el(id_l.id, id_l.port, id_e, 0, "");
    Edge er(id_r.id, id_r.port, id_e, 1, "");
    g_.add_edge(el);
    g_.add_edge(er);
    ret_id = id_e;
    ret_port = 0;
    break;
  }
  case ktSliceName:
  {
    int sram_width;
    int reg_file_width;
    int sram_block_size;
    util::GlobalVar glv;
    CHECK(glv.geti("sram_width", sram_width));
    CHECK(glv.geti("reg_file_width", reg_file_width));
    sram_block_size = sram_width / reg_file_width;

    ret_id = convert_slicename(static_cast<SliceName *>(e_), g_, t_, reg_allocation_, sram_allocation_, _is_rhs, _is_sram, sram_block_size, parent_vertex_id_, child_index_);
    ret_port = 0;
    break;
  }
  case ktIdentifier:
  {
    if (t_.exist((static_cast<Identifier *>(e_))->name()))
    {
      int id = t_.get_vertex_id((static_cast<Identifier *>(e_))->name());
      int port = t_.get_vertex_port((static_cast<Identifier *>(e_))->name());
      ret_id = id;
      ret_port = port;
    }
    else
    {
      LOG(FATAL) << "No such variable \"" << (static_cast<Identifier *>(e_))->name() << "\"";
    }
    break;
  }
  case ktParenthesizedExpression:
  {
    Anchor ret = convert_expression((static_cast<VIR::ParenthesizedExpression *>(e_))->value(), g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
    ret_id = ret.id;
    ret_port = ret.port;
    break;
  }
  case ktPrimitiveFunctionCall:
  {
    PrimitiveFunctionCall *e = static_cast<PrimitiveFunctionCall *>(e_);
    ComputationVertex cv;
    if (e->primitive_func_name != "")
    {
      cv.func_name = e->primitive_func_name;
    }
    else
    {
      cv.func_name = e->name()->name();
    }
    int id_e = g_.add_vertex(cv, parent_vertex_id_, child_index_);
    // Note: the parametersSplit stores the elaborated slicename, not the parameters property
    // avoid converting extra parameters from function "fi"
    int convert_count = 0;
    if (cv.func_name == "fi")
    {
      convert_count = 1;
    }
    else
    {
      convert_count = e->parametersSplit().size();
    }
    for (int i = 0; i < convert_count; i++)
    {
      Anchor id_p = convert_expression(e->parametersSplit()[i], g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
      Edge ep(id_p.id, id_p.port, id_e, i);
      g_.add_edge(ep);
    }
    ret_id = id_e;
    ret_port = 0;
    break;
  }
  case ktArray:
  {
    Array *e = static_cast<Array *>(e_);
    if (e->elements().size() <= 0)
    {
      LOG(FATAL) << "Empty array is not allowed!";
    }
    else if (e->elements().size() == 1)
    {
      Anchor id_par = convert_expression(e->elements()[0], g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
      if (g_.get_vertex(id_par.id)->vertex_type == Vertex::COMPUTATION_VERTEX &&
          (static_cast<ComputationVertex *>(g_.get_vertex(id_par.id))->func_name == "range" || static_cast<ComputationVertex *>(g_.get_vertex(id_par.id))->func_name == "array"))
      {

        ret_id = id_par.id;
        ret_port = id_par.port;
      }
      else
      {
        ComputationVertex cv;
        cv.func_name = "array";
        int id_cv = g_.add_vertex(cv, parent_vertex_id_, child_index_);
        Edge e0(id_par.id, id_par.port, id_cv, 0);
        g_.add_edge(e0);
        ret_id = id_cv;
        ret_port = 0;
      }
    }
    else
    {
      ComputationVertex cv;
      cv.func_name = "array";
      int id_cv = g_.add_vertex(cv, parent_vertex_id_, child_index_);
      for (int i = 0; i < e->elements().size(); i++)
      {
        Anchor id_par = convert_expression(e->elements()[i], g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
        Edge e0(id_par.id, id_par.port, id_cv, i);
        g_.add_edge(e0);
      }
      ret_id = id_cv;
      ret_port = 0;
    }
    break;
  }
  case ktRangeExpression:
  {
    RangeExpression *e = static_cast<RangeExpression *>(e_);
    Anchor id_b = convert_expression(e->begin(), g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
    Anchor id_i = convert_expression(e->increment(), g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
    Anchor id_e = convert_expression(e->end(), g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
    ComputationVertex v_add;
    ComputationVertex v_sub;
    ComputationVertex v_div;
    v_add.func_name = "+";
    v_sub.func_name = "-";
    v_div.func_name = "/";
    int id_v_add = g_.add_vertex(v_add, parent_vertex_id_, child_index_);
    int id_v_sub = g_.add_vertex(v_sub, parent_vertex_id_, child_index_);
    int id_v_div = g_.add_vertex(v_div, parent_vertex_id_, child_index_);
    ConstVertex v_one;
    v_one.set_int_value(1);
    int id_v_one = g_.add_vertex(v_one, parent_vertex_id_, child_index_);
    Edge e0(id_e.id, id_e.port, id_v_sub, 0);
    Edge e1(id_b.id, id_b.port, id_v_sub, 1);
    Edge e2(id_v_sub, 0, id_v_div, 0);
    Edge e3(id_i.id, id_i.port, id_v_div, 1);
    Edge e4(id_v_div, 0, id_v_add, 0);
    Edge e5(id_v_one, 0, id_v_add, 1);
    g_.add_edge(e0);
    g_.add_edge(e1);
    g_.add_edge(e2);
    g_.add_edge(e3);
    g_.add_edge(e4);
    g_.add_edge(e5);

    int id_it = id_v_add;

    ComputationVertex v_range;
    v_range.func_name = "range";
    int id_v_range = g_.add_vertex(v_range, parent_vertex_id_, child_index_);

    Edge e6(id_b.id, id_b.port, id_v_range, 0);
    Edge e7(id_i.id, id_i.port, id_v_range, 1);
    Edge e8(id_it, 0, id_v_range, 2);

    g_.add_edge(e6);
    g_.add_edge(e7);
    g_.add_edge(e8);

    ret_id = id_v_range;
    ret_port = 0;
    break;
  }
  case ktUnaryExpression:
  {
    UnaryExpression *e = static_cast<UnaryExpression *>(e_);
    Anchor id_operand = convert_expression(e->operand(), g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
    if (e->operatorType() == VIR::VirEnumerations::uotPlus)
    {
      ret_id = id_operand.id;
      ret_port = id_operand.port;
    }
    else if (e->operatorType() == VIR::VirEnumerations::uotMinus)
    {
      Vertex *v = g_.get_vertex(id_operand.id);
      CHECK_EQ(v->vertex_type, cidfg::Vertex::CONST_VERTEX) << "Only constant number is supported at initialization";
      ConstVertex *vv = static_cast<ConstVertex *>(v);
      if (vv->value_type == cidfg::ConstVertex::FLOAT)
      {
        vv->float_data = -(vv->float_data);
      }
      else
      {
        vv->int_data = -(vv->int_data);
      }
      ret_id = id_operand.id;
      ret_port = id_operand.port;
    }
    else
    {
      LOG(FATAL) << "Unsupported unary operator \"" << e->operatorStr() << "\"";
    }
    break;
  }
  default:
  {
    LOG(FATAL) << "Unsupported expression type: " << e_->kindStr();
  }
  }

  CHECK_GE(ret_id, 0);

  for (auto &n : t_.names())
  {
    int vid = t_.get_vertex_id(n);
    for (auto &e : g_.get_edges())
    {
      if (e->edge_type != Edge::SCALAR_DATA_SIG)
      {
        continue;
      }
      if (e->src_id == vid && strncmp(e->var_name.c_str(), "temp_var_", strlen("temp_var_")) == 0)
      {
        e->var_name = n;
      }
    }
  }

  Anchor a;
  a.id = ret_id;
  a.port = ret_port;
  return a;
}

void Converter::convert_if_statement(IfStatement *e_, CidfgGraph &g_, VarTable &t_, StorageAllocationMap &reg_allocation_, StorageAllocationMap &sram_allocation_, int parent_vertex_id_, int child_index_)
{
  _is_rhs = true;
  Anchor id_cond = convert_expression(e_->condition(), g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
  static_cast<ComputationVertex *>(g_.get_vertex(id_cond.id))->is_on_dpu = true;
  _is_rhs = false;
  BranchVertex bv;
  bv.children.resize(2);
  int id_bv = g_.add_vertex(bv, parent_vertex_id_, child_index_);
  Edge e_cond(id_cond.id, id_cond.port, id_bv, 0);
  g_.add_edge(e_cond);

  VarTable t0 = t_;
  // for (auto &n : t0.names())
  // {

  //   Edge::EdgeType et = t0.get_edge_type(n);
  //   if (et == Edge::VECTOR_DATA_SIG)
  //   {
  //     RegVarVertex v;
  //     v.var_name = n;
  //     v.coord = t0.get_vertex_coord(n);
  //     if (t0.get_isfixed(n))
  //     {
  //       v.value_type = SourceVertex::FLOAT;
  //     }
  //     else
  //     {
  //       v.value_type = SourceVertex::INT;
  //     }
  //     int id = g_.add_vertex(v, id_bv, 0);
  //     t0.update_var(n, id, 0, _domain_signatures.back(), Edge::VECTOR_DATA_SIG, t0.get_vertex_coord(n), t0.get_isfixed(n));
  //   }
  //   else
  //   {
  //     VarVertex v;
  //     v.var_name = n;
  //     v.coord = t0.get_vertex_coord(n);
  //     int id = g_.add_vertex(v, id_bv, 0);
  //     t0.update_var(n, id, 0, _domain_signatures.back(), Edge::SCALAR_DATA_SIG, t0.get_vertex_coord(n));
  //   }
  // }

  for (auto &s : e_->thenPart())
  {
    convert_statement(s, g_, t0, reg_allocation_, sram_allocation_, id_bv, 0);
  }

  // for (auto &n : t0.names())
  // {
  //   int id = t0.get_vertex_id(n);
  //   int port = t0.get_vertex_port(n);
  //   VIR::Coordinate coord = t0.get_vertex_coord(n);
  //   if (g_.get_out_edge(id, port) < 0)
  //   {
  //     SinkVertex v;
  //     v.var_name = n;
  //     v.coord = coord;
  //     int id_v = g_.add_vertex(v, id_bv, 0);
  //     Edge e(id, port, id_v, 0, n, t0.get_edge_type(n));
  //     g_.add_edge(e);
  //   }
  // }

  VarTable t1 = t_;
  // for (auto &n : t1.names())
  // {
  //   Edge::EdgeType et = t1.get_edge_type(n);
  //   if (et == Edge::VECTOR_DATA_SIG)
  //   {
  //     RegVarVertex v;
  //     v.var_name = n;
  //     v.coord = t1.get_vertex_coord(n);
  //     if (t1.get_isfixed(n))
  //     {
  //       v.value_type = SourceVertex::FLOAT;
  //     }
  //     else
  //     {
  //       v.value_type = SourceVertex::INT;
  //     }
  //     int id = g_.add_vertex(v, id_bv, 1);
  //     t1.update_var(n, id, 0, _domain_signatures.back(), Edge::VECTOR_DATA_SIG, t1.get_vertex_coord(n), t1.get_isfixed(n));
  //   }
  //   else
  //   {
  //     VarVertex v;
  //     v.var_name = n;
  //     v.coord = t1.get_vertex_coord(n);
  //     int id = g_.add_vertex(v, id_bv, 1);
  //     t1.update_var(n, id, 0, _domain_signatures.back(), Edge::SCALAR_DATA_SIG, t1.get_vertex_coord(n));
  //   }
  // }
  for (auto &s : e_->elsePart())
  {
    convert_statement(s, g_, t1, reg_allocation_, sram_allocation_, id_bv, 1);
  }
  // for (auto &n : t1.names())
  // {
  //   string var_name = n;
  //   int id = t1.get_vertex_id(n);
  //   int port = t1.get_vertex_port(n);
  //   VIR::Coordinate coord = t1.get_vertex_coord(n);
  //   if (g_.get_out_edge(id, port) < 0)
  //   {
  //     SinkVertex v;
  //     v.var_name = n;
  //     v.coord = coord;
  //     int id_v = g_.add_vertex(v, id_bv, 1);
  //     Edge e(id, port, id_v, 0, n, t1.get_edge_type(n));
  //     g_.add_edge(e);
  //   }
  // }

  // BranchVertex *pbv = static_cast<BranchVertex *>(g_.get_vertex(id_bv));
  // set<string> input_name_set;
  // set<string> output_name_set;
  // if (pbv->children.size() > 0)
  // {
  //   for (auto &cid : pbv->children[0])
  //   {
  //     Vertex *vv = g_.get_vertex(cid);
  //     if (vv->vertex_type == Vertex::SINK_VERTEX)
  //     {
  //       output_name_set.insert((static_cast<SinkVertex *>(vv))->var_name);
  //     }
  //     else if (vv->vertex_type == Vertex::VAR_VERTEX || vv->vertex_type == Vertex::REG_VAR_VERTEX || vv->vertex_type == Vertex::SRAM_VAR_VERTEX)
  //     {
  //       input_name_set.insert((static_cast<SourceVertex *>(vv))->var_name);
  //     }
  //   }
  // }
  // if (pbv->children.size() > 1)
  // {
  //   for (auto &cid : pbv->children[1])
  //   {
  //     Vertex *vv = g_.get_vertex(cid);
  //     if (vv->vertex_type == Vertex::SINK_VERTEX)
  //     {
  //       output_name_set.insert((static_cast<SinkVertex *>(vv))->var_name);
  //     }
  //     else if (vv->vertex_type == Vertex::VAR_VERTEX || vv->vertex_type == Vertex::REG_VAR_VERTEX || vv->vertex_type == Vertex::SRAM_VAR_VERTEX)
  //     {
  //       input_name_set.insert((static_cast<SourceVertex *>(vv))->var_name);
  //     }
  //   }
  // }

  // int i = 1;
  // for (auto &n : input_name_set)
  // {
  //   int id = t_.get_vertex_id(n);
  //   int port = t_.get_vertex_port(n);
  //   Edge e(id, port, id_bv, i, n, t_.get_edge_type(n));
  //   g_.add_edge(e);
  //   i++;
  // }
  // i = 1;
  // for (auto &n : output_name_set)
  // {
  //   t_.update_var(n, id_bv, i, _domain_signatures.back(), t_.get_edge_type(n));
  //   i++;
  // }

  for (auto &n : t_.names())
  {
    int id0 = 0;
    int id1 = 0;
    int port0 = 0;
    int port1 = 0;
    for (auto &m : t0.names())
    {
      if (n == m)
      {
        id0 = t0.get_vertex_id(m);
        port0 = t0.get_vertex_port(m);
      }
    }
    for (auto &m : t1.names())
    {
      if (n == m)
      {
        id1 = t1.get_vertex_id(m);
        port1 = t1.get_vertex_port(m);
      }
    }

    if (id0 != id1)
    {
      MergeVertex v;
      int id_v = g_.add_vertex(v, parent_vertex_id_, child_index_);
      Edge e0(id0, port0, id_v, 0, n, t0.get_edge_type(n));
      Edge e1(id1, port1, id_v, 1, n, t0.get_edge_type(n));
      g_.add_edge(e0);
      g_.add_edge(e1);
      t_.update_var(n, id_v, 0, _domain_signatures.back());
    }
    else
    {
      t_.update_var(n, id0, port0, _domain_signatures.back());
    }
  }

  // Remove any link between THEN and ELSE part, because those parts can never
  // be dependent.
  std::set<int> removed_edges;
  std::set<int> then_vertices;
  std::set<int> else_vertices;
  find_all_children(g_, id_bv, 0, then_vertices);
  find_all_children(g_, id_bv, 1, else_vertices);
  for (auto &e : g_.get_edges())
  {
    if (then_vertices.find(e->src_id) != then_vertices.end() && else_vertices.find(e->dest_id) != else_vertices.end() ||
        then_vertices.find(e->dest_id) != then_vertices.end() && else_vertices.find(e->src_id) != else_vertices.end())
    {
      removed_edges.insert(e->id);
    }
  }
  for (auto &id : removed_edges)
  {
    g_.del_edge(id);
  }
}

void Converter::convert_for_statement(ForStatement *e_, CidfgGraph &g_, VarTable &t_, StorageAllocationMap &reg_allocation_, StorageAllocationMap &sram_allocation_, int parent_vertex_id_, int child_index_)
{
  RangeExpression *exp = e_->loopRange();

  LoopVertex v_loop;
  v_loop.iterator_name = e_->loopVariable()->name();
  v_loop.loop_id = _loop_id_counter;
  int id_loop = g_.add_vertex(v_loop, parent_vertex_id_, child_index_);

  Anchor id_b = convert_expression(exp->begin(), g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
  Anchor id_i = convert_expression(exp->increment(), g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
  Anchor id_e = convert_expression(exp->end(), g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
  ComputationVertex v1;
  ComputationVertex v2;
  ComputationVertex v3;
  ConstVertex v4;
  v1.func_name = "-";
  v2.func_name = "/";
  v3.func_name = "+";
  v4.set_int_value(1);
  int id_v1 = g_.add_vertex(v1, parent_vertex_id_, child_index_);
  int id_v2 = g_.add_vertex(v2, parent_vertex_id_, child_index_);
  int id_v3 = g_.add_vertex(v3, parent_vertex_id_, child_index_);
  int id_v4 = g_.add_vertex(v4, parent_vertex_id_, child_index_);
  Edge e1(id_e.id, id_e.port, id_v1, 0);
  Edge e2(id_b.id, id_b.port, id_v1, 1);
  Edge e3(id_v1, 0, id_v2, 0);
  Edge e4(id_i.id, id_i.port, id_v2, 1);
  Edge e5(id_v2, 0, id_v3, 0);
  Edge e6(id_v4, 0, id_v3, 1);
  g_.add_edge(e1);
  g_.add_edge(e2);
  g_.add_edge(e3);
  g_.add_edge(e4);
  g_.add_edge(e5);
  g_.add_edge(e6);

  Edge eidx0(id_b.id, id_b.port, id_loop, 0);
  Edge eidx1(id_i.id, id_i.port, id_loop, 1);
  Edge eidx2(id_v3, 0, id_loop, 2);
  g_.add_edge(eidx0);
  g_.add_edge(eidx1);
  g_.add_edge(eidx2);

  VarTable t0 = t_;
  // for (auto &n : t0.names())
  // {

  //   Edge::EdgeType et = t0.get_edge_type(n);
  //   if (et == Edge::VECTOR_DATA_SIG)
  //   {
  //     RegVarVertex v;
  //     v.var_name = n;
  //     v.coord = t0.get_vertex_coord(n);
  //     if (t0.get_isfixed(n))
  //     {
  //       v.value_type = SourceVertex::FLOAT;
  //     }
  //     else
  //     {
  //       v.value_type = SourceVertex::INT;
  //     }
  //     int id = g_.add_vertex(v, id_loop, 0);
  //     t0.update_var(n, id, 0, _domain_signatures.back(), Edge::VECTOR_DATA_SIG, t0.get_vertex_coord(n), t0.get_isfixed(n));
  //   }
  //   else
  //   {
  //     VarVertex v;
  //     v.var_name = n;
  //     v.coord = t0.get_vertex_coord(n);
  //     int id = g_.add_vertex(v, id_loop, 0);
  //     t0.update_var(n, id, 0, _domain_signatures.back(), Edge::SCALAR_DATA_SIG, t0.get_vertex_coord(n));
  //   }
  // }

  VarVertex v_it;
  v_it.var_name = e_->loopVariable()->name();
  v_it.is_iterator = true;
  v_it.loop_id = _loop_id_counter;
  _loop_id_counter++;
  int id_it = g_.add_vertex(v_it, id_loop, 0);
  t0.update_var(v_it.var_name, id_it, 0, _domain_signatures.back(), Edge::SCALAR_DATA_SIG);

  for (auto &s : e_->loopBody())
  {
    convert_statement(s, g_, t0, reg_allocation_, sram_allocation_, id_loop, 0);
  }

  // for (auto &n : t0.names())
  // {
  //   int id = t0.get_vertex_id(n);
  //   int port = t0.get_vertex_port(n);
  //   VIR::Coordinate coord = t0.get_vertex_coord(n);
  //   if (g_.get_out_edge(id, port) < 0)
  //   {
  //     SinkVertex v;
  //     v.var_name = n;
  //     v.coord = coord;
  //     int id_v = g_.add_vertex(v, id_loop, 0);
  //     Edge e(id, port, id_v, 0, n, t0.get_edge_type(n));
  //     g_.add_edge(e);
  //   }
  // }

  // LoopVertex *plv = static_cast<LoopVertex *>(g_.get_vertex(id_loop));
  // set<string> input_name_set;
  // set<string> output_name_set;
  // if (plv->children.size() > 0)
  // {
  //   for (auto &cid : plv->children[0])
  //   {
  //     Vertex *vv = g_.get_vertex(cid);
  //     if (vv->vertex_type == Vertex::SINK_VERTEX)
  //     {
  //       output_name_set.insert((static_cast<SinkVertex *>(vv))->var_name);
  //     }
  //     else if (vv->vertex_type == Vertex::VAR_VERTEX || vv->vertex_type == Vertex::REG_VAR_VERTEX || vv->vertex_type == Vertex::SRAM_VAR_VERTEX)
  //     {
  //       input_name_set.insert((static_cast<SourceVertex *>(vv))->var_name);
  //     }
  //   }
  // }

  // int i = 3;
  // for (auto &n : input_name_set)
  // {
  //   if (n != e_->loopVariable()->name())
  //   {
  //     int id = t_.get_vertex_id(n);
  //     int port = t_.get_vertex_port(n);
  //     Edge e(id, port, id_loop, i, n, t_.get_edge_type(n));
  //     g_.add_edge(e);
  //   }
  //   i++;
  // }
  // i = 1;
  // for (auto &n : output_name_set)
  // {
  //   if (n != e_->loopVariable()->name())
  //   {
  //     t_.update_var(n, id_loop, i, _domain_signatures.back(), t_.get_edge_type(n));
  //   }
  //   i++;
  // }

  for (auto &n : t_.names())
  {
    for (auto &m : t0.names())
    {
      if (n == m)
      {
        int id = t0.get_vertex_id(m);
        int port = t0.get_vertex_port(m);
        Edge::EdgeType type = t0.get_edge_type(m);
        t_.update_var(n, id, port, _domain_signatures.back(), type);
      }
    }
  }
}

int Converter::convert_slicename(SliceName *e_, CidfgGraph &g_, VarTable &t_, StorageAllocationMap &reg_allocation_, StorageAllocationMap &sram_allocation_, bool is_read_, bool is_sram_, int sram_block_size_, int parent_vertex_id_, int child_index_)
{
  if (is_read_)
  {
    // Resource sharing ??
    if (_res_share_on && !is_sram_)
    {
      if (_vt_res_share.exist(e_->prefix()->name()))
      {
        int vid = _vt_res_share.get_vertex_id(e_->prefix()->name());
        return vid;
      }
    }

    int vid = t_.get_vertex_id(e_->prefix()->name());
    int vport = t_.get_vertex_port(e_->prefix()->name());
    int start_id = t_.get_start_vertex_id(e_->prefix()->name());
    ReadingVertex v;
    v.is_sram = is_sram_;
    int vid_rd = g_.add_vertex(v, parent_vertex_id_, child_index_);
    Edge e(vid, vport, vid_rd, 0, e_->prefix()->name(), t_.get_edge_type(e_->prefix()->name()));
    g_.add_edge(e);

    // Resource sharing ??
    if (_res_share_on && !is_sram_)
    {
      _vt_res_share.update_var(e_->prefix()->name(), vid_rd, 0, "");
    }

    IndexingVertex vi;
    int vid_ag = g_.add_vertex(vi, parent_vertex_id_, child_index_);
    Edge e1(vid_ag, 0, vid_rd, 1);
    g_.add_edge(e1);

    if (e_->suffix()[0]->kind() == ktFunctionCall || e_->suffix()[0]->kind() == ktPrimitiveFunctionCall)
    {
      FunctionCall *fc = static_cast<FunctionCall *>(e_->suffix()[0]);
      if (strncmp(fc->name()->name().c_str(), "silago_agu_linear_1d", strlen("silago_agu_linear_1d")) == 0)
      {
        Anchor id_s = convert_expression(fc->parameters()[0], g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
        Anchor id_i = convert_expression(fc->parameters()[1], g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
        Anchor id_n = convert_expression(fc->parameters()[2], g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);

        // if (_var_map.find(e_->prefix()->name()) == _var_map.end())
        // {
        //   LOG(FATAL) << "Unknow variable: " << e_->prefix()->name();
        // }
        // string var_name = e_->prefix()->name();
        // VarLocInfo info = _var_map[var_name];
        ComputationVertex vsa;
        vsa.func_name = "+";
        int id_vsa = g_.add_vertex(vsa, parent_vertex_id_, child_index_);
        // Extra -1 is because matlab start address from 1.
        ConstVertex vc1;
        vc1.set_int_value(1);
        int id_one = g_.add_vertex(vc1, parent_vertex_id_, child_index_);
        ComputationVertex vss;
        vss.func_name = "-";
        int id_vss = g_.add_vertex(vss, parent_vertex_id_, child_index_);
        Edge e_start_vss(start_id, 0, id_vss, 0);
        Edge e_one_vss(id_one, 0, id_vss, 1);
        g_.add_edge(e_start_vss);
        g_.add_edge(e_one_vss);

        Edge e_s_vsa(id_s.id, id_s.port, id_vsa, 0);
        Edge e_vc1_vsa(id_vss, 0, id_vsa, 1);
        g_.add_edge(e_s_vsa);
        g_.add_edge(e_vc1_vsa);

        int id_start_final1 = id_vsa;
        int id_increment_final1 = id_i.id;
        int id_iteration_final1 = id_n.id;
        if (is_sram_)
        {
          ComputationVertex v_div_start1;
          ComputationVertex v_div_iteration1;
          v_div_start1.func_name = "/";
          v_div_iteration1.func_name = "/";
          ConstVertex v_const_sram_block_size;
          v_const_sram_block_size.set_int_value(sram_block_size_);
          int id_div_start1 = g_.add_vertex(v_div_start1, parent_vertex_id_, child_index_);
          int id_div_iteration1 = g_.add_vertex(v_div_iteration1, parent_vertex_id_, child_index_);
          int id_const_sram_block_size = g_.add_vertex(v_const_sram_block_size, parent_vertex_id_, child_index_);
          Edge eic0(id_start_final1, 0, id_div_start1, 0);
          Edge eic1(id_const_sram_block_size, 0, id_div_start1, 1);
          Edge eit0(id_iteration_final1, 0, id_div_iteration1, 0);
          Edge eit1(id_const_sram_block_size, 0, id_div_iteration1, 1);
          g_.add_edge(eic0);
          g_.add_edge(eic1);
          g_.add_edge(eit0);
          g_.add_edge(eit1);
          id_start_final1 = id_div_start1;
          id_iteration_final1 = id_div_iteration1;
        }

        Edge eidx0(id_start_final1, 0, vid_ag, 0);
        Edge eidx1(id_increment_final1, 0, vid_ag, 1);
        Edge eidx2(id_iteration_final1, 0, vid_ag, 2);
        g_.add_edge(eidx0);
        g_.add_edge(eidx1);
        g_.add_edge(eidx2);
        for (int i = 3; i < 5; i++)
        {
          ConstVertex vc;
          if (i == 4)
          {
            vc.set_int_value(1); // l2 loop interation number should be 1 by default.
          }
          else
          {
            vc.set_int_value(0);
          }
          int id = g_.add_vertex(vc, parent_vertex_id_, child_index_);
          Edge ei(id, 0, vid_ag, i);
          g_.add_edge(ei);
        }
        return vid_rd;
      }
      else if (strncmp(fc->name()->name().c_str(), "silago_agu_linear_2d", strlen("silago_agu_linear_2d")) == 0)
      {
        Anchor id_s = convert_expression(fc->parameters()[0], g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
        Anchor id_i = convert_expression(fc->parameters()[1], g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
        Anchor id_n = convert_expression(fc->parameters()[2], g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
        Anchor id_o = convert_expression(fc->parameters()[3], g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
        Anchor id_m = convert_expression(fc->parameters()[4], g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);

        // if (_var_map.find(e_->prefix()->name()) == _var_map.end())
        // {
        //   LOG(FATAL) << "Unknow variable: " << e_->prefix()->name();
        // }
        // string var_name = e_->prefix()->name();
        // VarLocInfo info = _var_map[var_name];
        ComputationVertex vsa;
        vsa.func_name = "+";
        int id_vsa = g_.add_vertex(vsa, parent_vertex_id_, child_index_);
        // Extra -1 is because matlab start address from 1.
        ConstVertex vc1;
        vc1.set_int_value(1);
        int id_one = g_.add_vertex(vc1, parent_vertex_id_, child_index_);
        ComputationVertex vss;
        vss.func_name = "-";
        int id_vss = g_.add_vertex(vss, parent_vertex_id_, child_index_);
        Edge e_start_vss(start_id, 0, id_vss, 0);
        Edge e_one_vss(id_one, 0, id_vss, 1);
        g_.add_edge(e_start_vss);
        g_.add_edge(e_one_vss);

        Edge e_s_vsa(id_s.id, id_s.port, id_vsa, 0);
        Edge e_vc1_vsa(id_vss, 0, id_vsa, 1);
        g_.add_edge(e_s_vsa);
        g_.add_edge(e_vc1_vsa);

        int id_start_final1 = id_vsa;
        int id_increment_final1 = id_i.id;
        int id_iteration_final1 = id_n.id;
        int id_increment_final2 = id_o.id;
        int id_iteration_final2 = id_m.id;
        if (is_sram_)
        {
          ComputationVertex v_div_start1;
          ComputationVertex v_div_iteration1;
          ComputationVertex v_div_iteration2;
          v_div_start1.func_name = "/";
          v_div_iteration1.func_name = "/";
          v_div_iteration2.func_name = "/";
          ConstVertex v_const_sram_block_size;
          v_const_sram_block_size.set_int_value(sram_block_size_);
          int id_div_start1 = g_.add_vertex(v_div_start1, parent_vertex_id_, child_index_);
          int id_div_iteration1 = g_.add_vertex(v_div_iteration1, parent_vertex_id_, child_index_);
          int id_div_iteration2 = g_.add_vertex(v_div_iteration1, parent_vertex_id_, child_index_);
          int id_const_sram_block_size = g_.add_vertex(v_const_sram_block_size, parent_vertex_id_, child_index_);
          Edge eic0(id_start_final1, 0, id_div_start1, 0);
          Edge eic1(id_const_sram_block_size, 0, id_div_start1, 1);
          Edge eit0(id_iteration_final1, 0, id_div_iteration1, 0);
          Edge eit1(id_const_sram_block_size, 0, id_div_iteration1, 1);
          Edge eit01(id_iteration_final2, 0, id_div_iteration2, 0);
          Edge eit11(id_const_sram_block_size, 0, id_div_iteration2, 1);
          g_.add_edge(eic0);
          g_.add_edge(eic1);
          g_.add_edge(eit0);
          g_.add_edge(eit1);
          g_.add_edge(eit01);
          g_.add_edge(eit11);
          id_start_final1 = id_div_start1;
          id_iteration_final1 = id_div_iteration1;
          id_iteration_final2 = id_div_iteration2;
        }

        Edge eidx0(id_start_final1, 0, vid_ag, 0);
        Edge eidx1(id_increment_final1, 0, vid_ag, 1);
        Edge eidx2(id_iteration_final1, 0, vid_ag, 2);
        Edge eidx3(id_increment_final2, 0, vid_ag, 3);
        Edge eidx4(id_iteration_final2, 0, vid_ag, 4);
        g_.add_edge(eidx0);
        g_.add_edge(eidx1);
        g_.add_edge(eidx2);
        g_.add_edge(eidx3);
        g_.add_edge(eidx4);
        return vid_rd;
      }
      else
      {
        LOG(FATAL) << "NOT SUPPORTED AUG PRIMITIVE FUNCTION " << fc->name()->name();
      }
    }
    else if (e_->suffix()[0]->kind() == ktRangeExpression)
    {
      VIR::RangeExpression *exp = static_cast<VIR::RangeExpression *>(e_->suffix()[0]);
      Anchor id_b = convert_expression(exp->begin(), g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
      Anchor id_i = convert_expression(exp->increment(), g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
      Anchor id_e = convert_expression(exp->end(), g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
      ComputationVertex v1;
      ComputationVertex v2;
      ComputationVertex v3;
      ConstVertex v4;
      v1.func_name = "-";
      v2.func_name = "/";
      v3.func_name = "+";
      v4.set_int_value(1);
      int id_v1 = g_.add_vertex(v1, parent_vertex_id_, child_index_);
      int id_v2 = g_.add_vertex(v2, parent_vertex_id_, child_index_);
      int id_v3 = g_.add_vertex(v3, parent_vertex_id_, child_index_);
      int id_v4 = g_.add_vertex(v4, parent_vertex_id_, child_index_);
      Edge e1(id_e.id, id_e.port, id_v1, 0);
      Edge e2(id_b.id, id_b.port, id_v1, 1);
      Edge e3(id_v1, 0, id_v2, 0);
      Edge e4(id_i.id, id_i.port, id_v2, 1);
      Edge e5(id_v2, 0, id_v3, 0);
      Edge e6(id_v4, 0, id_v3, 1);
      g_.add_edge(e1);
      g_.add_edge(e2);
      g_.add_edge(e3);
      g_.add_edge(e4);
      g_.add_edge(e5);
      g_.add_edge(e6);

      // if (_var_map.find(e_->prefix()->name()) == _var_map.end())
      // {
      //   LOG(FATAL) << "Unknow variable: " << e_->prefix()->name();
      // }
      // string var_name = e_->prefix()->name();
      // VarLocInfo info = _var_map[var_name];
      ComputationVertex vsa;
      vsa.func_name = "+";
      int id_vsa = g_.add_vertex(vsa, parent_vertex_id_, child_index_);
      // Extra -1 is because matlab start address from 1.
      ConstVertex vc1;
      vc1.set_int_value(1);
      int id_one = g_.add_vertex(vc1, parent_vertex_id_, child_index_);
      ComputationVertex vss;
      vss.func_name = "-";
      int id_vss = g_.add_vertex(vss, parent_vertex_id_, child_index_);
      Edge e_start_vss(start_id, 0, id_vss, 0);
      Edge e_one_vss(id_one, 0, id_vss, 1);
      g_.add_edge(e_start_vss);
      g_.add_edge(e_one_vss);
      Edge e_s_vsa(id_b.id, id_b.port, id_vsa, 0);
      Edge e_vc1_vsa(id_vss, 0, id_vsa, 1);
      g_.add_edge(e_s_vsa);
      g_.add_edge(e_vc1_vsa);

      int id_start_final1 = id_vsa;
      int id_increment_final1 = id_i.id;
      int id_iteration_final1 = id_v3;
      if (is_sram_)
      {
        ComputationVertex v_div_start1;
        ComputationVertex v_div_iteration1;
        v_div_start1.func_name = "/";
        v_div_iteration1.func_name = "/";
        ConstVertex v_const_sram_block_size;
        v_const_sram_block_size.set_int_value(sram_block_size_);
        int id_div_start1 = g_.add_vertex(v_div_start1, parent_vertex_id_, child_index_);
        int id_div_iteration1 = g_.add_vertex(v_div_iteration1, parent_vertex_id_, child_index_);
        int id_const_sram_block_size = g_.add_vertex(v_const_sram_block_size, parent_vertex_id_, child_index_);
        Edge eic0(id_start_final1, 0, id_div_start1, 0);
        Edge eic1(id_const_sram_block_size, 0, id_div_start1, 1);
        Edge eit0(id_iteration_final1, 0, id_div_iteration1, 0);
        Edge eit1(id_const_sram_block_size, 0, id_div_iteration1, 1);
        g_.add_edge(eic0);
        g_.add_edge(eic1);
        g_.add_edge(eit0);
        g_.add_edge(eit1);
        id_start_final1 = id_div_start1;
        id_iteration_final1 = id_div_iteration1;
      }

      Edge eidx0(id_start_final1, 0, vid_ag, 0);
      Edge eidx1(id_increment_final1, 0, vid_ag, 1);
      Edge eidx2(id_iteration_final1, 0, vid_ag, 2);
      g_.add_edge(eidx0);
      g_.add_edge(eidx1);
      g_.add_edge(eidx2);
      for (int i = 3; i < 5; i++)
      {
        ConstVertex vc;
        if (i == 4)
        {
          vc.set_int_value(1); // l2 loop interation number should be 1 by default.
        }
        else
        {
          vc.set_int_value(0);
        }
        int id = g_.add_vertex(vc, parent_vertex_id_, child_index_);
        Edge ei(id, 0, vid_ag, i);
        g_.add_edge(ei);
      }
      return vid_rd;
    }
    else
    {
      Anchor vid_index;
      vid_index = convert_expression(e_->suffix()[0], g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
      // if (_var_map.find(e_->prefix()->name()) == _var_map.end())
      // {
      //   LOG(FATAL) << "Unknow variable: " << e_->prefix()->name();
      // }
      // string var_name = e_->prefix()->name();
      // VarLocInfo info = _var_map[var_name];
      ComputationVertex v0;
      v0.func_name = "+";
      int id_v0 = g_.add_vertex(v0, parent_vertex_id_, child_index_);
      // Extra -1 is because matlab start address from 1.
      ConstVertex vc1;
      vc1.set_int_value(1);
      int id_one = g_.add_vertex(vc1, parent_vertex_id_, child_index_);
      ComputationVertex vss;
      vss.func_name = "-";
      int id_vss = g_.add_vertex(vss, parent_vertex_id_, child_index_);
      Edge e_start_vss(start_id, 0, id_vss, 0);
      Edge e_one_vss(id_one, 0, id_vss, 1);
      g_.add_edge(e_start_vss);
      g_.add_edge(e_one_vss);

      Edge e0(vid_index.id, vid_index.port, id_v0, 0);
      Edge e1(id_vss, 0, id_v0, 1);
      Edge e2(id_v0, 0, vid_ag, 0);
      g_.add_edge(e0);
      g_.add_edge(e1);
      g_.add_edge(e2);
      for (int i = 1; i < 5; i++)
      {
        ConstVertex vc;
        if (i == 2 || i == 4)
        {
          vc.set_int_value(1); // l2 loop interation number should be 1 by default.
        }
        else
        {
          vc.set_int_value(0);
        }
        int id = g_.add_vertex(vc, parent_vertex_id_, child_index_);
        Edge ei(id, 0, vid_ag, i);
        g_.add_edge(ei);
      }
      return vid_rd;
    }
  }
  else
  {
    WritingVertex v;
    v.is_sram = is_sram_;
    int vid_wr = g_.add_vertex(v, parent_vertex_id_, child_index_);

    int vid_var = t_.get_vertex_id(e_->prefix()->name());
    int vport_var = t_.get_vertex_port(e_->prefix()->name());
    int start_id = t_.get_start_vertex_id(e_->prefix()->name());
    Edge e_var(vid_var, vport_var, vid_wr, 2, e_->prefix()->name(), Edge::VECTOR_DATA_SIG);
    g_.add_edge(e_var);

    IndexingVertex vi;
    int vid_ag = g_.add_vertex(vi, parent_vertex_id_, child_index_);
    Edge e1(vid_ag, 0, vid_wr, 1);
    g_.add_edge(e1);

    if (e_->suffix()[0]->kind() == ktFunctionCall || e_->suffix()[0]->kind() == ktPrimitiveFunctionCall)
    {
      FunctionCall *fc = static_cast<FunctionCall *>(e_->suffix()[0]);
      if (strncmp(fc->name()->name().c_str(), "silago_agu_linear_1d", strlen("silago_agu_linear_1d")) == 0)
      {
        Anchor id_s = convert_expression(fc->parameters()[0], g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
        Anchor id_i = convert_expression(fc->parameters()[1], g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
        Anchor id_n = convert_expression(fc->parameters()[2], g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);

        // if (_var_map.find(e_->prefix()->name()) == _var_map.end())
        // {
        //   LOG(FATAL) << "Unknow variable: " << e_->prefix()->name();
        // }
        // string var_name = e_->prefix()->name();
        // VarLocInfo info = _var_map[var_name];
        ComputationVertex vsa;
        vsa.func_name = "+";
        int id_vsa = g_.add_vertex(vsa, parent_vertex_id_, child_index_);
        // Extra -1 is because matlab start address from 1.
        ConstVertex vc1;
        vc1.set_int_value(1);
        int id_one = g_.add_vertex(vc1, parent_vertex_id_, child_index_);
        ComputationVertex vss;
        vss.func_name = "-";
        int id_vss = g_.add_vertex(vss, parent_vertex_id_, child_index_);
        Edge e_start_vss(start_id, 0, id_vss, 0);
        Edge e_one_vss(id_one, 0, id_vss, 1);
        g_.add_edge(e_start_vss);
        g_.add_edge(e_one_vss);
        Edge e_s_vsa(id_s.id, id_s.port, id_vsa, 0);
        Edge e_vc1_vsa(id_vss, 0, id_vsa, 1);
        g_.add_edge(e_s_vsa);
        g_.add_edge(e_vc1_vsa);

        int id_start_final1 = id_vsa;
        int id_increment_final1 = id_i.id;
        int id_iteration_final1 = id_n.id;
        if (is_sram_)
        {
          ComputationVertex v_div_start1;
          ComputationVertex v_div_iteration1;
          v_div_start1.func_name = "/";
          v_div_iteration1.func_name = "/";
          ConstVertex v_const_sram_block_size;
          v_const_sram_block_size.set_int_value(sram_block_size_);
          int id_div_start1 = g_.add_vertex(v_div_start1, parent_vertex_id_, child_index_);
          int id_div_iteration1 = g_.add_vertex(v_div_iteration1, parent_vertex_id_, child_index_);
          int id_const_sram_block_size = g_.add_vertex(v_const_sram_block_size, parent_vertex_id_, child_index_);
          Edge eic0(id_start_final1, 0, id_div_start1, 0);
          Edge eic1(id_const_sram_block_size, 0, id_div_start1, 1);
          Edge eit0(id_iteration_final1, 0, id_div_iteration1, 0);
          Edge eit1(id_const_sram_block_size, 0, id_div_iteration1, 1);
          g_.add_edge(eic0);
          g_.add_edge(eic1);
          g_.add_edge(eit0);
          g_.add_edge(eit1);
          id_start_final1 = id_div_start1;
          id_iteration_final1 = id_div_iteration1;
        }

        Edge eidx0(id_start_final1, 0, vid_ag, 0);
        Edge eidx1(id_increment_final1, 0, vid_ag, 1);
        Edge eidx2(id_iteration_final1, 0, vid_ag, 2);
        g_.add_edge(eidx0);
        g_.add_edge(eidx1);
        g_.add_edge(eidx2);
        for (int i = 3; i < 5; i++)
        {
          ConstVertex vc;
          if (i == 4)
          {
            vc.set_int_value(1); // l2 loop interation number should be 1 by default.
          }
          else
          {
            vc.set_int_value(0);
          }
          int id = g_.add_vertex(vc, parent_vertex_id_, child_index_);
          Edge ei(id, 0, vid_ag, i);
          g_.add_edge(ei);
        }
        if (t_.exist(e_->prefix()->name()))
        {
          int vertex_id = t_.get_vertex_id(e_->prefix()->name());
          std::set<int> reference_list;
          for (auto &e : g_.get_edges())
          {
            if (e->src_id == vertex_id && e->edge_type != Edge::DEPENDENCY && e->dest_id != vid_wr)
            {
              reference_list.insert(e->dest_id);
            }
          }
          for (auto &refid : reference_list)
          {
            if (!g_.get_vertex(refid)->is_sink())
            {
              Edge e_dep(refid, 100, vid_wr, 100, "WAR_weak", Edge::DEPENDENCY, 1, INT_MAX);
              g_.add_edge(e_dep);
            }
          }
          int oldid = t_.get_vertex_id(e_->prefix()->name());
          if (!g_.get_vertex(oldid)->is_source())
          {
            Edge e_dep(oldid, 100, vid_wr, 100, "WAW", Edge::DEPENDENCY, 1, INT_MAX);
            g_.add_edge(e_dep);
          }
        }
        t_.update_var(e_->prefix()->name(), vid_wr, 0, _domain_signatures.back(), Edge::VECTOR_DATA_SIG);
        return vid_wr;
      }
      else if (strncmp(fc->name()->name().c_str(), "silago_agu_linear_2d", strlen("silago_agu_linear_2d")) == 0)
      {
        Anchor id_s = convert_expression(fc->parameters()[0], g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
        Anchor id_i = convert_expression(fc->parameters()[1], g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
        Anchor id_n = convert_expression(fc->parameters()[2], g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
        Anchor id_o = convert_expression(fc->parameters()[3], g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
        Anchor id_m = convert_expression(fc->parameters()[4], g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);

        // if (_var_map.find(e_->prefix()->name()) == _var_map.end())
        // {
        //   LOG(FATAL) << "Unknow variable: " << e_->prefix()->name();
        // }
        // string var_name = e_->prefix()->name();
        // VarLocInfo info = _var_map[var_name];
        ComputationVertex vsa;
        vsa.func_name = "+";
        int id_vsa = g_.add_vertex(vsa, parent_vertex_id_, child_index_);
        // Extra -1 is because matlab start address from 1.
        ConstVertex vc1;
        vc1.set_int_value(1);
        int id_one = g_.add_vertex(vc1, parent_vertex_id_, child_index_);
        ComputationVertex vss;
        vss.func_name = "-";
        int id_vss = g_.add_vertex(vss, parent_vertex_id_, child_index_);
        Edge e_start_vss(start_id, 0, id_vss, 0);
        Edge e_one_vss(id_one, 0, id_vss, 1);
        g_.add_edge(e_start_vss);
        g_.add_edge(e_one_vss);
        Edge e_s_vsa(id_s.id, id_s.port, id_vsa, 0);
        Edge e_vc1_vsa(id_vss, 0, id_vsa, 1);
        g_.add_edge(e_s_vsa);
        g_.add_edge(e_vc1_vsa);

        int id_start_final1 = id_vsa;
        int id_increment_final1 = id_i.id;
        int id_iteration_final1 = id_n.id;
        int id_increment_final2 = id_o.id;
        int id_iteration_final2 = id_m.id;
        if (is_sram_)
        {
          ComputationVertex v_div_start1;
          ComputationVertex v_div_iteration1;
          ComputationVertex v_div_start2;
          ComputationVertex v_div_iteration2;
          v_div_start1.func_name = "/";
          v_div_iteration1.func_name = "/";
          v_div_iteration2.func_name = "/";
          ConstVertex v_const_sram_block_size;
          v_const_sram_block_size.set_int_value(sram_block_size_);
          int id_div_start1 = g_.add_vertex(v_div_start1, parent_vertex_id_, child_index_);
          int id_div_iteration1 = g_.add_vertex(v_div_iteration1, parent_vertex_id_, child_index_);
          int id_div_iteration2 = g_.add_vertex(v_div_iteration1, parent_vertex_id_, child_index_);
          int id_const_sram_block_size = g_.add_vertex(v_const_sram_block_size, parent_vertex_id_, child_index_);
          Edge eic0(id_start_final1, 0, id_div_start1, 0);
          Edge eic1(id_const_sram_block_size, 0, id_div_start1, 1);
          Edge eit0(id_iteration_final1, 0, id_div_iteration1, 0);
          Edge eit1(id_const_sram_block_size, 0, id_div_iteration1, 1);
          Edge eit01(id_iteration_final2, 0, id_div_iteration2, 0);
          Edge eit11(id_const_sram_block_size, 0, id_div_iteration2, 1);
          g_.add_edge(eic0);
          g_.add_edge(eic1);
          g_.add_edge(eit0);
          g_.add_edge(eit1);
          g_.add_edge(eit01);
          g_.add_edge(eit11);
          id_start_final1 = id_div_start1;
          id_iteration_final1 = id_div_iteration1;
          id_iteration_final2 = id_div_iteration2;
        }

        Edge eidx0(id_start_final1, 0, vid_ag, 0);
        Edge eidx1(id_increment_final1, 0, vid_ag, 1);
        Edge eidx2(id_iteration_final1, 0, vid_ag, 2);
        Edge eidx3(id_increment_final2, 0, vid_ag, 3);
        Edge eidx4(id_iteration_final2, 0, vid_ag, 4);
        g_.add_edge(eidx0);
        g_.add_edge(eidx1);
        g_.add_edge(eidx2);
        g_.add_edge(eidx3);
        g_.add_edge(eidx4);

        if (t_.exist(e_->prefix()->name()))
        {
          int vertex_id = t_.get_vertex_id(e_->prefix()->name());
          std::set<int> reference_list;
          for (auto &e : g_.get_edges())
          {
            if (e->src_id == vertex_id && e->edge_type != Edge::DEPENDENCY && e->dest_id != vid_wr)
            {
              reference_list.insert(e->dest_id);
            }
          }

          for (auto &refid : reference_list)
          {
            if (!g_.get_vertex(refid)->is_sink())
            {
              Edge e_dep(refid, 100, vid_wr, 100, "WAR_weak", Edge::DEPENDENCY, 1, INT_MAX);
              g_.add_edge(e_dep);
            }
          }
          int oldid = t_.get_vertex_id(e_->prefix()->name());
          if (!g_.get_vertex(oldid)->is_source())
          {
            Edge e_dep(oldid, 100, vid_wr, 100, "WAW", Edge::DEPENDENCY, 1, INT_MAX);
            g_.add_edge(e_dep);
          }
        }

        t_.update_var(e_->prefix()->name(), vid_wr, 0, _domain_signatures.back(), Edge::VECTOR_DATA_SIG);

        return vid_wr;
      }
      else
      {
        LOG(FATAL) << "NOT SUPPORTED AUG PRIMITIVE FUNCTION " << fc->name()->name();
      }
    }
    else if (e_->suffix()[0]->kind() == ktRangeExpression)
    {
      VIR::RangeExpression *exp = static_cast<VIR::RangeExpression *>(e_->suffix()[0]);
      Anchor id_b = convert_expression(exp->begin(), g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
      Anchor id_i = convert_expression(exp->increment(), g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
      Anchor id_e = convert_expression(exp->end(), g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
      ComputationVertex v1;
      ComputationVertex v2;
      ComputationVertex v3;
      ConstVertex v4;
      v1.func_name = "-";
      v2.func_name = "/";
      v3.func_name = "+";
      v4.set_int_value(1);
      int id_v1 = g_.add_vertex(v1, parent_vertex_id_, child_index_);
      int id_v2 = g_.add_vertex(v2, parent_vertex_id_, child_index_);
      int id_v3 = g_.add_vertex(v3, parent_vertex_id_, child_index_);
      int id_v4 = g_.add_vertex(v4, parent_vertex_id_, child_index_);
      Edge e1(id_e.id, id_e.port, id_v1, 0);
      Edge e2(id_b.id, id_b.port, id_v1, 1);
      Edge e3(id_v1, 0, id_v2, 0);
      Edge e4(id_i.id, id_i.port, id_v2, 1);
      Edge e5(id_v2, 0, id_v3, 0);
      Edge e6(id_v4, 0, id_v3, 1);
      g_.add_edge(e1);
      g_.add_edge(e2);
      g_.add_edge(e3);
      g_.add_edge(e4);
      g_.add_edge(e5);
      g_.add_edge(e6);

      // if (_var_map.find(e_->prefix()->name()) == _var_map.end())
      // {
      //   LOG(FATAL) << "Unknow variable: " << e_->prefix()->name();
      // }
      // string var_name = e_->prefix()->name();
      // VarLocInfo info = _var_map[var_name];
      ComputationVertex vsa;
      vsa.func_name = "+";
      int id_vsa = g_.add_vertex(vsa, parent_vertex_id_, child_index_);
      // Extra -1 is because matlab start address from 1.
      ConstVertex vc1;
      vc1.set_int_value(1);
      int id_one = g_.add_vertex(vc1, parent_vertex_id_, child_index_);
      ComputationVertex vss;
      vss.func_name = "-";
      int id_vss = g_.add_vertex(vss, parent_vertex_id_, child_index_);
      Edge e_start_vss(start_id, 0, id_vss, 0);
      Edge e_one_vss(id_one, 0, id_vss, 1);
      g_.add_edge(e_start_vss);
      g_.add_edge(e_one_vss);
      Edge e_s_vsa(id_b.id, id_b.port, id_vsa, 0);
      Edge e_vc1_vsa(id_vss, 0, id_vsa, 1);
      g_.add_edge(e_s_vsa);
      g_.add_edge(e_vc1_vsa);

      int id_start_final1 = id_vsa;
      int id_increment_final1 = id_i.id;
      int id_iteration_final1 = id_v3;
      if (is_sram_)
      {
        ComputationVertex v_div_start1;
        ComputationVertex v_div_iteration1;
        v_div_start1.func_name = "/";
        v_div_iteration1.func_name = "/";
        ConstVertex v_const_sram_block_size;
        v_const_sram_block_size.set_int_value(sram_block_size_);
        int id_div_start1 = g_.add_vertex(v_div_start1, parent_vertex_id_, child_index_);
        int id_div_iteration1 = g_.add_vertex(v_div_iteration1, parent_vertex_id_, child_index_);
        int id_const_sram_block_size = g_.add_vertex(v_const_sram_block_size, parent_vertex_id_, child_index_);
        Edge eic0(id_start_final1, 0, id_div_start1, 0);
        Edge eic1(id_const_sram_block_size, 0, id_div_start1, 1);
        Edge eit0(id_iteration_final1, 0, id_div_iteration1, 0);
        Edge eit1(id_const_sram_block_size, 0, id_div_iteration1, 1);
        g_.add_edge(eic0);
        g_.add_edge(eic1);
        g_.add_edge(eit0);
        g_.add_edge(eit1);
        id_start_final1 = id_div_start1;
        id_iteration_final1 = id_div_iteration1;
      }

      Edge eidx0(id_start_final1, 0, vid_ag, 0);
      Edge eidx1(id_increment_final1, 0, vid_ag, 1);
      Edge eidx2(id_iteration_final1, 0, vid_ag, 2);
      g_.add_edge(eidx0);
      g_.add_edge(eidx1);
      g_.add_edge(eidx2);
      for (int i = 3; i < 5; i++)
      {
        ConstVertex vc;
        if (i == 4)
        {
          vc.set_int_value(1); // l2 loop interation number should be 1 by default.
        }
        else
        {
          vc.set_int_value(0);
        }
        int id = g_.add_vertex(vc, parent_vertex_id_, child_index_);
        Edge ei(id, 0, vid_ag, i);
        g_.add_edge(ei);
      }

      if (t_.exist(e_->prefix()->name()))
      {
        int vertex_id = t_.get_vertex_id(e_->prefix()->name());
        std::set<int> reference_list;
        for (auto &e : g_.get_edges())
        {
          if (e->src_id == vertex_id && e->edge_type != Edge::DEPENDENCY && e->dest_id != vid_wr)
          {
            reference_list.insert(e->dest_id);
          }
        }
        for (auto &refid : reference_list)
        {
          if (!g_.get_vertex(refid)->is_sink())
          {
            Edge e_dep(refid, 100, vid_wr, 100, "WAR_weak", Edge::DEPENDENCY, 1, INT_MAX);
            g_.add_edge(e_dep);
          }
        }
        int oldid = t_.get_vertex_id(e_->prefix()->name());
        if (!g_.get_vertex(oldid)->is_source())
        {
          Edge e_dep(oldid, 100, vid_wr, 100, "WAW", Edge::DEPENDENCY, 1, INT_MAX);
          g_.add_edge(e_dep);
        }
      }
      t_.update_var(e_->prefix()->name(), vid_wr, 0, _domain_signatures.back(), Edge::VECTOR_DATA_SIG);

      return vid_wr;
    }
    else
    {
      Anchor vid_index = convert_expression(e_->suffix()[0], g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
      // if (_var_map.find(e_->prefix()->name()) == _var_map.end())
      // {
      //   LOG(FATAL) << "Unknow variable: " << e_->prefix()->name();
      // }
      // string var_name = e_->prefix()->name();
      // VarLocInfo info = _var_map[var_name];
      ComputationVertex v0;
      v0.func_name = "+";
      int id_v0 = g_.add_vertex(v0, parent_vertex_id_, child_index_);
      // Extra -1 is because matlab start address from 1.
      ConstVertex vc1;
      vc1.set_int_value(1);
      int id_one = g_.add_vertex(vc1, parent_vertex_id_, child_index_);
      ComputationVertex vss;
      vss.func_name = "-";
      int id_vss = g_.add_vertex(vss, parent_vertex_id_, child_index_);
      Edge e_start_vss(start_id, 0, id_vss, 0);
      Edge e_one_vss(id_one, 0, id_vss, 1);
      g_.add_edge(e_start_vss);
      g_.add_edge(e_one_vss);

      Edge e0(vid_index.id, vid_index.port, id_v0, 0);
      Edge e1(id_vss, 0, id_v0, 1);
      Edge e2(id_v0, 0, vid_ag, 0);
      g_.add_edge(e0);
      g_.add_edge(e1);
      g_.add_edge(e2);
      for (int i = 1; i < 5; i++)
      {
        ConstVertex vc;
        if (i == 2 || i == 4)
        {
          vc.set_int_value(1); // l2 loop interation number should be 1 by default.
        }
        else
        {
          vc.set_int_value(0);
        }
        int id = g_.add_vertex(vc, parent_vertex_id_, child_index_);
        Edge ei(id, 0, vid_ag, i);
        g_.add_edge(ei);
      }
    }

    if (t_.exist(e_->prefix()->name()))
    {
      int vertex_id = t_.get_vertex_id(e_->prefix()->name());
      std::set<int> reference_list;
      for (auto &e : g_.get_edges())
      {
        if (e->src_id == vertex_id && e->edge_type != Edge::DEPENDENCY && e->dest_id != vid_wr)
        {
          reference_list.insert(e->dest_id);
        }
      }
      for (auto &refid : reference_list)
      {
        if (!g_.get_vertex(refid)->is_sink())
        {
          Edge e_dep(refid, 100, vid_wr, 100, "WAR_weak", Edge::DEPENDENCY, 1, INT_MAX);
          g_.add_edge(e_dep);
        }
      }
      int oldid = t_.get_vertex_id(e_->prefix()->name());
      if (!g_.get_vertex(oldid)->is_source())
      {
        Edge e_dep(oldid, 100, vid_wr, 100, "WAW", Edge::DEPENDENCY, 1, INT_MAX);
        g_.add_edge(e_dep);
      }
    }
    t_.update_var(e_->prefix()->name(), vid_wr, 0, _domain_signatures.back());
    return vid_wr;
  }
}

int Converter::convert_reg_declarative_statement(AssignmentStatement *e_, CidfgGraph &g_, VarTable &t_, StorageAllocationMap &reg_allocation_, StorageAllocationMap &sram_allocation_, int parent_vertex_id_, int child_index_)
{
  Anchor id_v_rhs = convert_expression(e_->rhs(), g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
  Vertex *v_rhs = g_.get_vertex(id_v_rhs.id);
  v_rhs->coord = static_cast<StoragePragma *>(e_->pragma())->coordinates[0];
  CHECK_EQ(v_rhs->vertex_type, Vertex::COMPUTATION_VERTEX);
  ComputationVertex *v = static_cast<ComputationVertex *>(v_rhs);

  bool isfixed = false;
  if (v->func_name == "fi")
  {
    Vertex *vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v->id, 0))->src_id);
    CHECK_EQ(vv->vertex_type, Vertex::COMPUTATION_VERTEX);
    v = static_cast<ComputationVertex *>(vv);
    isfixed = true;
  }

  int id_start_vertex = -1;
  int id_size_vertex = -1;
  Coordinate coord = v_rhs->coord;

  if (reg_allocation_.find(coord.to_str()) == reg_allocation_.end())
  {
    ConstVertex cv;
    cv.set_int_value(0);
    int id_cv = g_.add_vertex(cv, parent_vertex_id_, child_index_);
    reg_allocation_[coord.to_str()] = id_cv;
  }

  if (v->func_name == "zeros" || v->func_name == "ones")
  {
    int id_par0 = g_.get_edge(g_.get_in_edge(v->id, 0))->src_id;
    int id_par1 = g_.get_edge(g_.get_in_edge(v->id, 1))->src_id;
    ComputationVertex v_mul;
    v_mul.func_name = "*";
    int id_v_mul = g_.add_vertex(v_mul, parent_vertex_id_, child_index_);
    Edge e0(id_par0, 0, id_v_mul, 0);
    Edge e1(id_par1, 0, id_v_mul, 1);
    g_.add_edge(e0);
    g_.add_edge(e1);

    id_start_vertex = reg_allocation_[coord.to_str()];
    id_size_vertex = id_v_mul;

    ComputationVertex v_add;
    v_add.func_name = "+";
    int id_v_add = g_.add_vertex(v_add, parent_vertex_id_, child_index_);
    Edge e2(id_start_vertex, 0, id_v_add, 0);
    Edge e3(id_size_vertex, 0, id_v_add, 1);
    g_.add_edge(e2);
    g_.add_edge(e3);

    reg_allocation_[coord.to_str()] = id_v_add;
  }
  else if (v->func_name == "array")
  {
    int counter = 0;
    for (auto &e : g_.get_edges())
    {
      if (e->dest_id == v->id)
      {
        counter++;
      }
    }

    ConstVertex cv;
    cv.set_int_value(counter);
    int id_cv = g_.add_vertex(cv, parent_vertex_id_, child_index_);

    id_start_vertex = reg_allocation_[coord.to_str()];
    id_size_vertex = id_cv;

    ComputationVertex v_add;
    v_add.func_name = "+";
    int id_v_add = g_.add_vertex(v_add, parent_vertex_id_, child_index_);
    Edge e2(id_start_vertex, 0, id_v_add, 0);
    Edge e3(id_size_vertex, 0, id_v_add, 1);
    g_.add_edge(e2);
    g_.add_edge(e3);

    reg_allocation_[coord.to_str()] = id_v_add;
  }
  else if (v->func_name == "range")
  {

    int id_par2 = g_.get_edge(g_.get_in_edge(v->id, 2))->src_id;
    id_start_vertex = reg_allocation_[coord.to_str()];
    id_size_vertex = id_par2;

    ComputationVertex v_add;
    v_add.func_name = "+";
    int id_v_add = g_.add_vertex(v_add, parent_vertex_id_, child_index_);
    Edge e2(id_start_vertex, 0, id_v_add, 0);
    Edge e3(id_size_vertex, 0, id_v_add, 1);
    g_.add_edge(e2);
    g_.add_edge(e3);

    reg_allocation_[coord.to_str()] = id_v_add;
  }
  else
  {
    LOG(FATAL) << "Unsupported initialization statement of variable " << static_cast<Identifier *>(e_->lhs()[0])->name() << " !";
  }

  RegVarVertex rvv;
  rvv.var_name = static_cast<Identifier *>(e_->lhs()[0])->name();
  rvv.coord = v_rhs->coord;
  int id_rvv = g_.add_vertex(rvv, parent_vertex_id_, child_index_);
  Edge e0(id_v_rhs.id, id_v_rhs.port, id_rvv, 0);
  g_.add_edge(e0);

  t_.update_var(static_cast<Identifier *>(e_->lhs()[0])->name(), id_rvv, 0, _domain_signatures.back(), Edge::VECTOR_DATA_SIG, v_rhs->coord, isfixed, id_start_vertex, id_size_vertex);
  return id_v_rhs.id;
}

int Converter::convert_sram_declarative_statement(AssignmentStatement *e_, CidfgGraph &g_, VarTable &t_, StorageAllocationMap &reg_allocation_, StorageAllocationMap &sram_allocation_, int parent_vertex_id_, int child_index_)
{
  Anchor id_v_rhs = convert_expression(e_->rhs(), g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
  Vertex *v_rhs = g_.get_vertex(id_v_rhs.id);
  v_rhs->coord = static_cast<StoragePragma *>(e_->pragma())->coordinates[0];
  CHECK_EQ(v_rhs->vertex_type, Vertex::COMPUTATION_VERTEX);
  ComputationVertex *v = static_cast<ComputationVertex *>(v_rhs);

  bool isfixed = false;
  if (v->func_name == "fi")
  {
    Vertex *vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v->id, 0))->src_id);
    CHECK_EQ(vv->vertex_type, Vertex::COMPUTATION_VERTEX);
    v = static_cast<ComputationVertex *>(vv);
    isfixed = true;
  }

  int id_start_vertex = -1;
  int id_size_vertex = -1;
  Coordinate coord = v_rhs->coord;

  if (sram_allocation_.find(coord.to_str()) == sram_allocation_.end())
  {
    ConstVertex cv;
    cv.set_int_value(0);
    int id_cv = g_.add_vertex(cv, parent_vertex_id_, child_index_);
    sram_allocation_[coord.to_str()] = id_cv;
  }

  if (v->func_name == "zeros" || v->func_name == "ones")
  {
    int id_par0 = g_.get_edge(g_.get_in_edge(v->id, 0))->src_id;
    int id_par1 = g_.get_edge(g_.get_in_edge(v->id, 1))->src_id;
    ComputationVertex v_mul;
    v_mul.func_name = "*";
    int id_v_mul = g_.add_vertex(v_mul, parent_vertex_id_, child_index_);
    Edge e0(id_par0, 0, id_v_mul, 0);
    Edge e1(id_par1, 0, id_v_mul, 1);
    g_.add_edge(e0);
    g_.add_edge(e1);

    id_start_vertex = sram_allocation_[coord.to_str()];
    id_size_vertex = id_v_mul;

    ComputationVertex v_add;
    v_add.func_name = "+";
    int id_v_add = g_.add_vertex(v_add, parent_vertex_id_, child_index_);
    Edge e2(id_start_vertex, 0, id_v_add, 0);
    Edge e3(id_size_vertex, 0, id_v_add, 1);
    g_.add_edge(e2);
    g_.add_edge(e3);

    sram_allocation_[coord.to_str()] = id_v_add;
  }
  else if (v->func_name == "array")
  {
    int counter = 0;
    for (auto &e : g_.get_edges())
    {
      if (e->dest_id == v->id)
      {
        counter++;
      }
    }

    ConstVertex cv;
    cv.set_int_value(counter);
    int id_cv = g_.add_vertex(cv, parent_vertex_id_, child_index_);

    id_start_vertex = sram_allocation_[coord.to_str()];
    id_size_vertex = id_cv;

    ComputationVertex v_add;
    v_add.func_name = "+";
    int id_v_add = g_.add_vertex(v_add, parent_vertex_id_, child_index_);
    Edge e2(id_start_vertex, 0, id_v_add, 0);
    Edge e3(id_size_vertex, 0, id_v_add, 1);
    g_.add_edge(e2);
    g_.add_edge(e3);

    sram_allocation_[coord.to_str()] = id_v_add;
  }
  else if (v->func_name == "range")
  {

    int id_par2 = g_.get_edge(g_.get_in_edge(v->id, 2))->src_id;
    id_start_vertex = sram_allocation_[coord.to_str()];
    id_size_vertex = id_par2;

    ComputationVertex v_add;
    v_add.func_name = "+";
    int id_v_add = g_.add_vertex(v_add, parent_vertex_id_, child_index_);
    Edge e2(id_start_vertex, 0, id_v_add, 0);
    Edge e3(id_size_vertex, 0, id_v_add, 1);
    g_.add_edge(e2);
    g_.add_edge(e3);

    sram_allocation_[coord.to_str()] = id_v_add;
  }
  else
  {
    LOG(FATAL) << "Unsupported initialization statement of variable " << static_cast<Identifier *>(e_->lhs()[0])->name() << " !";
  }

  SramVarVertex rvv;
  Coordinate default_coord;
  rvv.var_name = static_cast<Identifier *>(e_->lhs()[0])->name();
  rvv.coord = default_coord;
  rvv.sram_coord = coord;
  int id_rvv = g_.add_vertex(rvv, parent_vertex_id_, child_index_);
  Edge e0(id_v_rhs.id, id_v_rhs.port, id_rvv, 0);
  g_.add_edge(e0);

  t_.update_var(static_cast<Identifier *>(e_->lhs()[0])->name(), id_rvv, 0, _domain_signatures.back(), Edge::SRAM_VECTOR_DATA_SIG, rvv.coord, isfixed, id_start_vertex, id_size_vertex);
  return id_v_rhs.id;
}

int Converter::convert_assignment_statement(AssignmentStatement *e_, CidfgGraph &g_, VarTable &t_, StorageAllocationMap &reg_allocation_, StorageAllocationMap &sram_allocation_, int parent_vertex_id_, int child_index_)
{
  if (e_->type() == atDeclarative && (static_cast<StoragePragma *>(e_->pragma()))->storageType() == stRegFile)
  {
    int id = convert_reg_declarative_statement(e_, g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
    e_->corresponding_cidfg_vertex = id;
    return id;
  }
  else if (e_->type() == atDeclarative && (static_cast<StoragePragma *>(e_->pragma()))->storageType() == stMemory)
  {
    int id = convert_sram_declarative_statement(e_, g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
    e_->corresponding_cidfg_vertex = id;
    return id;
  }
  else if (e_->type() == atConstant)
  {
    Anchor v_rhs = convert_expression(e_->rhs(), g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);

    string name = static_cast<VIR::Identifier *>(e_->lhs()[0])->name();
    if (t_.exist(name))
    {
      int vertex_id = t_.get_vertex_id(name);
      std::set<int> reference_list;
      for (auto &e : g_.get_edges())
      {
        if (e->src_id == vertex_id && e->edge_type != Edge::DEPENDENCY && e->dest_id != v_rhs.id)
        {
          reference_list.insert(e->dest_id);
        }
      }
      for (auto &refid : reference_list)
      {
        if (!g_.get_vertex(refid)->is_sink())
        {
          Edge e_dep(refid, 100, v_rhs.id, 100, "WAR", Edge::DEPENDENCY, 1, INT_MAX);
          g_.add_edge(e_dep);
        }
      }
      int oldid = t_.get_vertex_id(name);
      if (!g_.get_vertex(oldid)->is_source())
      {
        Edge e_dep(oldid, 100, v_rhs.id, 100, "WAW", Edge::DEPENDENCY, 1, INT_MAX);
        g_.add_edge(e_dep);
      }
    }
    t_.update_var(static_cast<VIR::Identifier *>(e_->lhs()[0])->name(), v_rhs.id, 0, _domain_signatures.back());
    return v_rhs.id;
  }
  else if (e_->type() == atRaccuVariable)
  {
    Anchor v_rhs = convert_expression(e_->rhs(), g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);

    ComputationVertex v;
    v.func_name = "ld";
    int id = g_.add_vertex(v, parent_vertex_id_, child_index_);
    Edge e0(v_rhs.id, v_rhs.port, id, 0);
    g_.add_edge(e0);

    string name = static_cast<VIR::Identifier *>(e_->lhs()[0])->name();
    if (t_.exist(name))
    {
      int vertex_id = t_.get_vertex_id(name);
      std::set<int> reference_list;
      for (auto &e : g_.get_edges())
      {
        if (e->src_id == vertex_id && e->edge_type != Edge::DEPENDENCY && e->dest_id != v_rhs.id)
        {
          reference_list.insert(e->dest_id);
        }
      }
      for (auto &refid : reference_list)
      {
        if (!g_.get_vertex(refid)->is_sink())
        {
          Edge e_dep(refid, 100, v_rhs.id, 100, "WAR", Edge::DEPENDENCY, 1, INT_MAX);
          g_.add_edge(e_dep);
        }
      }
      int oldid = t_.get_vertex_id(name);
      if (!g_.get_vertex(oldid)->is_source())
      {
        Edge e_dep(oldid, 100, v_rhs.id, 100, "WAW", Edge::DEPENDENCY, 1, INT_MAX);
        g_.add_edge(e_dep);
      }
    }

    t_.update_var(static_cast<VIR::Identifier *>(e_->lhs()[0])->name(), id, 0, _domain_signatures.back());
    return id;
  }
  else if (e_->type() == atAddressCalculation)
  {
    Anchor v_rhs = convert_expression(e_->rhs(), g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
    string name = static_cast<VIR::Identifier *>(e_->lhs()[0])->name();
    if (t_.exist(name))
    {
      int vertex_id = t_.get_vertex_id(name);
      std::set<int> reference_list;
      for (auto &e : g_.get_edges())
      {
        if (e->src_id == vertex_id && e->edge_type != Edge::DEPENDENCY && e->dest_id != v_rhs.id)
        {
          reference_list.insert(e->dest_id);
        }
      }
      for (auto &refid : reference_list)
      {
        if (!g_.get_vertex(refid)->is_sink())
        {
          Edge e_dep(refid, 100, v_rhs.id, 100, "WAR", Edge::DEPENDENCY, 1, INT_MAX);
          g_.add_edge(e_dep);
        }
      }
      int oldid = t_.get_vertex_id(name);
      if (!g_.get_vertex(oldid)->is_source())
      {
        Edge e_dep(oldid, 100, v_rhs.id, 100, "WAW", Edge::DEPENDENCY, 1, INT_MAX);
        g_.add_edge(e_dep);
      }
    }

    t_.update_var(static_cast<VIR::Identifier *>(e_->lhs()[0])->name(), v_rhs.id, v_rhs.port, _domain_signatures.back());
    return v_rhs.id;
  }
  else if (e_->type() == atRegisterTransfer)
  {
    // Wrap register/memory transfer into a general hierarchical vertex is to simplify the
    // scheduling task. Currently scheduler can't handle to much vertices.

    int id_ghv = convert_register_transfer_statement(static_cast<VIR::AssignmentStatement *>(e_), g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
    return id_ghv;
  }
  else if (e_->type() == atMemoryTransfer)
  {
    int id_ghv = convert_memory_transfer_statement(static_cast<VIR::AssignmentStatement *>(e_), g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
    return id_ghv;
  }
  else if (e_->type() == atDPUChainDeclaration)
  {
    Coordinate c = static_cast<VIR::ReferencePragma *>(static_cast<AssignmentStatement *>(e_)->pragma())->coordinates[0];
    cdpu_vars[static_cast<Identifier *>(static_cast<AssignmentStatement *>(e_)->lhs()[0])->name()] = c;
    return -1;
  }
  else if (e_->type() == atDPUChainConnection)
  {
    // convert_dpu_chain_statement_0
    // Directly map arithmetic assighment into parent node. No general hierarchical
    // vertex wrapper

    // convert_dpu_chain_statement_1
    // Wrap arithmetic assighment into a general hierarchical vertex is to simplify the
    // scheduling task. Currently scheduler can't handle to much vertices.

    int id_ghv = convert_dpu_chain_statement_0(static_cast<VIR::AssignmentStatement *>(e_), g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
    //int id_ghv = convert_dpu_chain_statement_1(static_cast<VIR::AssignmentStatement *>(e_), g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
    return id_ghv;
  }
  else
  {
    // convert_arithmetic_assign_statement_0
    // Directly map arithmetic assighment into parent node. No general hierarchical
    // vertex wrapper

    // convert_arithmetic_assign_statement_1
    // Wrap arithmetic assighment into a general hierarchical vertex is to simplify the
    // scheduling task. Currently scheduler can't handle to much vertices.

    int id_ghv = convert_arithmetic_assign_statement_0(static_cast<VIR::AssignmentStatement *>(e_), g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
    //int id_ghv = convert_arithmetic_assign_statement_1(static_cast<VIR::AssignmentStatement *>(e_), g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
    return id_ghv;
  }
}

int Converter::convert_register_transfer_statement(VIR::AssignmentStatement *e_, CidfgGraph &g_, VarTable &t_, StorageAllocationMap &reg_allocation_, StorageAllocationMap &sram_allocation_, int parent_vertex_id_, int child_index_)
{
  GeneralHierarchicalVertex ghv;
  int id_ghv = g_.add_vertex(ghv, parent_vertex_id_, child_index_);

  VarTable t0 = t_;
  // for (auto &n : t0.names())
  // {

  //   Edge::EdgeType et = t0.get_edge_type(n);
  //   if (et == Edge::VECTOR_DATA_SIG)
  //   {
  //     RegVarVertex v;
  //     v.var_name = n;
  //     v.coord = t0.get_vertex_coord(n);
  //     if (t0.get_isfixed(n))
  //     {
  //       v.value_type = SourceVertex::FLOAT;
  //     }
  //     else
  //     {
  //       v.value_type = SourceVertex::INT;
  //     }
  //     int id = g_.add_vertex(v, id_ghv, 0);
  //     t0.update_var(n, id, 0, _domain_signatures.back(), Edge::VECTOR_DATA_SIG, t0.get_vertex_coord(n), t0.get_isfixed(n));
  //   }
  //   else if (et == Edge::SRAM_VECTOR_DATA_SIG)
  //   {
  //     SramVarVertex v;
  //     v.var_name = n;
  //     v.coord = t0.get_vertex_coord(n);
  //     if (t0.get_isfixed(n))
  //     {
  //       v.value_type = SourceVertex::FLOAT;
  //     }
  //     else
  //     {
  //       v.value_type = SourceVertex::INT;
  //     }
  //     int id = g_.add_vertex(v, id_ghv, 0);
  //     t0.update_var(n, id, 0, _domain_signatures.back(), Edge::SRAM_VECTOR_DATA_SIG, t0.get_vertex_coord(n), t0.get_isfixed(n));
  //   }
  //   else
  //   {
  //     VarVertex v;
  //     v.var_name = n;
  //     v.coord = t0.get_vertex_coord(n);
  //     int id = g_.add_vertex(v, id_ghv, 0);
  //     t0.update_var(n, id, 0, _domain_signatures.back(), Edge::SCALAR_DATA_SIG, t0.get_vertex_coord(n));
  //   }
  // }

  CHECK_EQ(e_->lhs().size(), 1);
  CHECK_EQ(e_->rhs()->kind(), VIR::VirEnumerations::ktSliceName);
  CHECK_EQ(e_->lhs()[0]->kind(), VIR::VirEnumerations::ktSliceName);

  _is_rhs = true;
  int sram_width;
  int reg_file_width;
  int sram_block_size;
  util::GlobalVar glv;
  CHECK(glv.geti("sram_width", sram_width));
  CHECK(glv.geti("reg_file_width", reg_file_width));
  sram_block_size = sram_width / reg_file_width;
  int v_rhs = convert_slicename(static_cast<VIR::SliceName *>(e_->rhs()), g_, t0, reg_allocation_, sram_allocation_, true, false, sram_block_size, id_ghv, 0);
  _is_rhs = false;

  int v_lhs = convert_slicename(static_cast<VIR::SliceName *>(e_->lhs()[0]), g_, t0, reg_allocation_, sram_allocation_, false, false, sram_block_size, id_ghv, 0);
  Edge e(v_rhs, 0, v_lhs, 0);
  g_.add_edge(e);

  // for (auto &n : t0.names())
  // {
  //   int id = t0.get_vertex_id(n);
  //   int port = t0.get_vertex_port(n);
  //   VIR::Coordinate coord = t0.get_vertex_coord(n);
  //   if (g_.get_out_edge(id, port) < 0)
  //   {
  //     SinkVertex v;
  //     v.var_name = n;
  //     v.coord = coord;
  //     int id_v = g_.add_vertex(v, id_ghv, 0);
  //     Edge e(id, port, id_v, 0, n, t0.get_edge_type(n));
  //     g_.add_edge(e);
  //   }
  // }

  // HierarchicalVertex *plv = static_cast<HierarchicalVertex *>(g_.get_vertex(id_ghv));
  // set<string> input_name_set;
  // set<string> output_name_set;
  // if (plv->children.size() > 0)
  // {
  //   for (auto &cid : plv->children[0])
  //   {
  //     Vertex *vv = g_.get_vertex(cid);
  //     if (vv->vertex_type == Vertex::SINK_VERTEX)
  //     {
  //       output_name_set.insert((static_cast<SinkVertex *>(vv))->var_name);
  //     }
  //     else if (vv->vertex_type == Vertex::VAR_VERTEX || vv->vertex_type == Vertex::REG_VAR_VERTEX || vv->vertex_type == Vertex::SRAM_VAR_VERTEX)
  //     {
  //       input_name_set.insert((static_cast<SourceVertex *>(vv))->var_name);
  //     }
  //   }
  // }

  // int i = 0;
  // for (auto &n : input_name_set)
  // {
  //   int id = t_.get_vertex_id(n);
  //   int port = t_.get_vertex_port(n);
  //   Edge e(id, port, id_ghv, i, n, t_.get_edge_type(n));
  //   g_.add_edge(e);
  //   i++;
  // }
  // i = 0;
  // for (auto &n : output_name_set)
  // {
  //   t_.update_var(n, id_ghv, i, _domain_signatures.back(), t_.get_edge_type(n));
  //   i++;
  // }

  t_ = t0;

  return id_ghv;
}

int Converter::convert_memory_transfer_statement(VIR::AssignmentStatement *e_, CidfgGraph &g_, VarTable &t_, StorageAllocationMap &reg_allocation_, StorageAllocationMap &sram_allocation_, int parent_vertex_id_, int child_index_)
{
  GeneralHierarchicalVertex ghv;
  int id_ghv = g_.add_vertex(ghv, parent_vertex_id_, child_index_);

  VarTable t0 = t_;
  // for (auto &n : t0.names())
  // {

  //   Edge::EdgeType et = t0.get_edge_type(n);
  //   if (et == Edge::VECTOR_DATA_SIG)
  //   {
  //     RegVarVertex v;
  //     v.var_name = n;
  //     v.coord = t0.get_vertex_coord(n);
  //     if (t0.get_isfixed(n))
  //     {
  //       v.value_type = SourceVertex::FLOAT;
  //     }
  //     else
  //     {
  //       v.value_type = SourceVertex::INT;
  //     }
  //     int id = g_.add_vertex(v, id_ghv, 0);
  //     t0.update_var(n, id, 0, _domain_signatures.back(), Edge::VECTOR_DATA_SIG, t0.get_vertex_coord(n), t0.get_isfixed(n));
  //   }
  //   else if (et == Edge::SRAM_VECTOR_DATA_SIG)
  //   {
  //     SramVarVertex v;
  //     v.var_name = n;
  //     v.coord = t0.get_vertex_coord(n);
  //     if (t0.get_isfixed(n))
  //     {
  //       v.value_type = SourceVertex::FLOAT;
  //     }
  //     else
  //     {
  //       v.value_type = SourceVertex::INT;
  //     }
  //     int id = g_.add_vertex(v, id_ghv, 0);
  //     t0.update_var(n, id, 0, _domain_signatures.back(), Edge::SRAM_VECTOR_DATA_SIG, t0.get_vertex_coord(n), t0.get_isfixed(n));
  //   }
  //   else
  //   {
  //     VarVertex v;
  //     v.var_name = n;
  //     v.coord = t0.get_vertex_coord(n);
  //     int id = g_.add_vertex(v, id_ghv, 0);
  //     t0.update_var(n, id, 0, _domain_signatures.back(), Edge::SCALAR_DATA_SIG, t0.get_vertex_coord(n));
  //   }
  // }

  CHECK_EQ(e_->lhs().size(), 1);
  CHECK_EQ(e_->rhs()->kind(), VIR::VirEnumerations::ktSliceName);
  CHECK_EQ(e_->lhs()[0]->kind(), VIR::VirEnumerations::ktSliceName);
  string lhs_name = static_cast<SliceName *>(e_->lhs()[0])->prefix()->name();
  string rhs_name = static_cast<SliceName *>(e_->rhs())->prefix()->name();
  Edge::EdgeType et_lhs = t0.get_edge_type(lhs_name);
  Edge::EdgeType et_rhs = t0.get_edge_type(rhs_name);

  bool is_sram_to_reg = false;
  if (et_lhs == Edge::VECTOR_DATA_SIG && et_rhs == Edge::SRAM_VECTOR_DATA_SIG)
  {
    is_sram_to_reg = true;
  }
  else if (et_lhs == Edge::SRAM_VECTOR_DATA_SIG && et_rhs == Edge::VECTOR_DATA_SIG)
  {
    is_sram_to_reg = false;
  }
  else
  {
    LOG(FATAL) << "Wrong memory transfer type!";
  }

  _is_rhs = true;
  int sram_width;
  int reg_file_width;
  int sram_block_size;
  util::GlobalVar glv;
  CHECK(glv.geti("sram_width", sram_width));
  CHECK(glv.geti("reg_file_width", reg_file_width));
  sram_block_size = sram_width / reg_file_width;
  int v_rhs = convert_slicename(static_cast<VIR::SliceName *>(e_->rhs()), g_, t0, reg_allocation_, sram_allocation_, _is_rhs, true, sram_block_size, id_ghv, 0);
  _is_rhs = false;
  int v_lhs = convert_slicename(static_cast<VIR::SliceName *>(e_->lhs()[0]), g_, t0, reg_allocation_, sram_allocation_, false, true, sram_block_size, id_ghv, 0);

  CHECK_EQ(g_.get_vertex(v_rhs)->vertex_type, Vertex::READING_VERTEX);
  CHECK_EQ(g_.get_vertex(v_lhs)->vertex_type, Vertex::WRITING_VERTEX);
  ReadingVertex *v_src = static_cast<ReadingVertex *>(g_.get_vertex(v_rhs));
  WritingVertex *v_dest = static_cast<WritingVertex *>(g_.get_vertex(v_lhs));
  if (is_sram_to_reg)
  {
    v_dest->is_sram = false; // This should be considered as REFI
  }
  else
  {
    v_src->is_sram = false; // This should be considered as REFI
  }

  Edge e(v_rhs, 0, v_lhs, 0);
  g_.add_edge(e);

  // for (auto &n : t0.names())
  // {
  //   int id = t0.get_vertex_id(n);
  //   int port = t0.get_vertex_port(n);
  //   VIR::Coordinate coord = t0.get_vertex_coord(n);
  //   if (g_.get_out_edge(id, port) < 0)
  //   {
  //     SinkVertex v;
  //     v.var_name = n;
  //     v.coord = coord;
  //     int id_v = g_.add_vertex(v, id_ghv, 0);
  //     Edge e(id, port, id_v, 0, n, t0.get_edge_type(n));
  //     g_.add_edge(e);
  //   }
  // }

  // HierarchicalVertex *plv = static_cast<HierarchicalVertex *>(g_.get_vertex(id_ghv));
  // set<string> input_name_set;
  // set<string> output_name_set;
  // if (plv->children.size() > 0)
  // {
  //   for (auto &cid : plv->children[0])
  //   {
  //     Vertex *vv = g_.get_vertex(cid);
  //     if (vv->vertex_type == Vertex::SINK_VERTEX)
  //     {
  //       output_name_set.insert((static_cast<SinkVertex *>(vv))->var_name);
  //     }
  //     else if (vv->vertex_type == Vertex::VAR_VERTEX || vv->vertex_type == Vertex::REG_VAR_VERTEX || vv->vertex_type == Vertex::SRAM_VAR_VERTEX)
  //     {
  //       input_name_set.insert((static_cast<SourceVertex *>(vv))->var_name);
  //     }
  //   }
  // }

  // int i = 0;
  // for (auto &n : input_name_set)
  // {
  //   int id = t_.get_vertex_id(n);
  //   int port = t_.get_vertex_port(n);
  //   Edge e(id, port, id_ghv, i, n, t_.get_edge_type(n));
  //   g_.add_edge(e);
  //   i++;
  // }
  // i = 0;
  // for (auto &n : output_name_set)
  // {
  //   t_.update_var(n, id_ghv, i, _domain_signatures.back(), t_.get_edge_type(n));
  //   i++;
  // }

  t_ = t0;

  return id_ghv;
}

int Converter::convert_arithmetic_assign_statement_0(VIR::AssignmentStatement *e_, CidfgGraph &g_, VarTable &t_, StorageAllocationMap &reg_allocation_, StorageAllocationMap &sram_allocation_, int parent_vertex_id_, int child_index_)
{

  _is_rhs = true;
  Anchor v_rhs = convert_expression(e_->rhs(), g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
  _is_rhs = false;

  Vertex *vertex = g_.get_vertex(v_rhs.id);

  if (vertex->vertex_type != Vertex::COMPUTATION_VERTEX || (vertex->vertex_type == Vertex::COMPUTATION_VERTEX && static_cast<ComputationVertex *>(vertex)->func_name == "fi"))
  {
    ComputationVertex cv;
    cv.func_name = "ld";
    cv.coord = static_cast<DPUPragma *>(e_->pragma())->coordinates[0];
    int id_cv = g_.add_vertex(cv, parent_vertex_id_, child_index_);
    Edge e0(v_rhs.id, v_rhs.port, id_cv, 0);
    g_.add_edge(e0);
    v_rhs.id = id_cv;
  }
  else
  {
    vertex->coord = static_cast<DPUPragma *>(e_->pragma())->coordinates[0];
  }
  for (int i = 0; i < e_->lhs().size(); i++)
  {
    CHECK_EQ(e_->lhs()[i]->kind(), ktSliceName);

    int sram_width;
    int reg_file_width;
    int sram_block_size;
    util::GlobalVar glv;
    CHECK(glv.geti("sram_width", sram_width));
    CHECK(glv.geti("reg_file_width", reg_file_width));
    sram_block_size = sram_width / reg_file_width;
    int v_lhs = convert_slicename(static_cast<SliceName *>(e_->lhs()[i]), g_, t_, reg_allocation_, sram_allocation_, false, false, sram_block_size, parent_vertex_id_, child_index_);
    Edge e(v_rhs.id, i, v_lhs, 0);
    g_.add_edge(e);
  }
  vertex = g_.get_vertex(v_rhs.id);
  ComputationVertex *vv = static_cast<ComputationVertex *>(vertex);
  vv->is_on_dpu = true;

  return v_rhs.id;
}

int Converter::convert_arithmetic_assign_statement_1(VIR::AssignmentStatement *e_, CidfgGraph &g_, VarTable &t_, StorageAllocationMap &reg_allocation_, StorageAllocationMap &sram_allocation_, int parent_vertex_id_, int child_index_)
{
  GeneralHierarchicalVertex ghv;
  int id_ghv = g_.add_vertex(ghv, parent_vertex_id_, child_index_);

  VarTable t0 = t_;

  _is_rhs = true;
  Anchor v_rhs = convert_expression(e_->rhs(), g_, t0, reg_allocation_, sram_allocation_, id_ghv, 0);
  _is_rhs = false;

  Vertex *vertex = g_.get_vertex(v_rhs.id);

  if (vertex->vertex_type != Vertex::COMPUTATION_VERTEX || (vertex->vertex_type == Vertex::COMPUTATION_VERTEX && static_cast<ComputationVertex *>(vertex)->func_name == "fi"))
  {
    ComputationVertex cv;
    cv.func_name = "ld";
    cv.coord = static_cast<DPUPragma *>(e_->pragma())->coordinates[0];
    int id_cv = g_.add_vertex(cv, id_ghv, 0);
    Edge e0(v_rhs.id, v_rhs.port, id_cv, 0);
    g_.add_edge(e0);
    v_rhs.id = id_cv;
  }
  else
  {
    vertex->coord = static_cast<DPUPragma *>(e_->pragma())->coordinates[0];
  }

  for (int i = 0; i < e_->lhs().size(); i++)
  {
    CHECK_EQ(e_->lhs()[i]->kind(), ktSliceName);
    int sram_width;
    int reg_file_width;
    int sram_block_size;
    util::GlobalVar glv;
    CHECK(glv.geti("sram_width", sram_width));
    CHECK(glv.geti("reg_file_width", reg_file_width));
    sram_block_size = sram_width / reg_file_width;
    int v_lhs = convert_slicename(static_cast<SliceName *>(e_->lhs()[i]), g_, t0, reg_allocation_, sram_allocation_, false, false, sram_block_size, id_ghv, 0);
    Edge e(v_rhs.id, i, v_lhs, 0);
    g_.add_edge(e);
  }
  vertex = g_.get_vertex(v_rhs.id);
  ComputationVertex *vv = static_cast<ComputationVertex *>(vertex);
  vv->is_on_dpu = true;

  t_ = t0;

  return id_ghv;
}

int Converter::convert_dpu_chain_statement_0(VIR::AssignmentStatement *e_, CidfgGraph &g_, VarTable &t_, StorageAllocationMap &reg_allocation_, StorageAllocationMap &sram_allocation_, int parent_vertex_id_, int child_index_)
{
  Coordinate c;
  for (int i = 0; i < e_->lhs().size(); i++)
  {
    CHECK_EQ(e_->lhs()[i]->kind(), ktIdentifier);
    string var_name = static_cast<Identifier *>(e_->lhs()[i])->name();
    if (cdpu_vars.find(var_name) == cdpu_vars.end())
    {
      LOG(FATAL) << "Undefined chaind DPU variable \"" << var_name << "\"";
    }
    else
    {
      if (!c.is_valid())
      {
        c = cdpu_vars[var_name];
      }
      else if (c != cdpu_vars[var_name])
      {
        LOG(FATAL) << "Chained DPU variables have different cell binding.";
      }
    }
  }
  CHECK(c.is_valid()) << "Illegal Chained DPU variable binding.";

  _is_rhs = true;
  Anchor v_rhs = convert_expression(e_->rhs(), g_, t_, reg_allocation_, sram_allocation_, parent_vertex_id_, child_index_);
  _is_rhs = false;

  Vertex *vertex = g_.get_vertex(v_rhs.id);

  if (vertex->vertex_type != Vertex::COMPUTATION_VERTEX || (vertex->vertex_type == Vertex::COMPUTATION_VERTEX && static_cast<ComputationVertex *>(vertex)->func_name == "fi"))
  {
    ComputationVertex cv;
    cv.func_name = "ld";
    cv.coord = c;
    int id_cv = g_.add_vertex(cv, parent_vertex_id_, child_index_);
    Edge e0(v_rhs.id, v_rhs.port, id_cv, 0);
    g_.add_edge(e0);
    v_rhs.id = id_cv;
  }
  else
  {
    vertex->coord = c;
  }
  for (int i = 0; i < e_->lhs().size(); i++)
  {
    CHECK_EQ(e_->lhs()[i]->kind(), ktIdentifier);
    string var_name = static_cast<Identifier *>(e_->lhs()[i])->name();
    t_.update_var(var_name, v_rhs.id, i, _domain_signatures.back(), Edge::SCALAR_DATA_SIG, c);
  }
  vertex = g_.get_vertex(v_rhs.id);
  ComputationVertex *vv = static_cast<ComputationVertex *>(vertex);
  vv->is_on_dpu = true;

  return v_rhs.id;
}

int Converter::convert_dpu_chain_statement_1(VIR::AssignmentStatement *e_, CidfgGraph &g_, VarTable &t_, StorageAllocationMap &reg_allocation_, StorageAllocationMap &sram_allocation_, int parent_vertex_id_, int child_index_)
{
  GeneralHierarchicalVertex ghv;
  int id_ghv = g_.add_vertex(ghv, parent_vertex_id_, child_index_);

  VarTable t0 = t_;

  Coordinate c;
  for (int i = 0; i < e_->lhs().size(); i++)
  {
    CHECK_EQ(e_->lhs()[i]->kind(), ktIdentifier);
    string var_name = static_cast<Identifier *>(e_->lhs()[i])->name();
    if (cdpu_vars.find(var_name) == cdpu_vars.end())
    {
      LOG(FATAL) << "Undefined chaind DPU variable \"" << var_name << "\"";
    }
    else
    {
      if (!c.is_valid())
      {
        c = cdpu_vars[var_name];
      }
      else if (c != cdpu_vars[var_name])
      {
        LOG(FATAL) << "Chained DPU variables have different cell binding.";
      }
    }
  }
  CHECK(c.is_valid()) << "Illegal Chained DPU variable binding.";

  _is_rhs = true;
  Anchor v_rhs = convert_expression(e_->rhs(), g_, t0, reg_allocation_, sram_allocation_, id_ghv, 0);
  _is_rhs = false;

  Vertex *vertex = g_.get_vertex(v_rhs.id);

  if (vertex->vertex_type != Vertex::COMPUTATION_VERTEX || (vertex->vertex_type == Vertex::COMPUTATION_VERTEX && static_cast<ComputationVertex *>(vertex)->func_name == "fi"))
  {
    ComputationVertex cv;
    cv.func_name = "ld";
    cv.coord = c;
    int id_cv = g_.add_vertex(cv, id_ghv, 0);
    Edge e0(v_rhs.id, v_rhs.port, id_cv, 0);
    g_.add_edge(e0);
    v_rhs.id = id_cv;
  }
  else
  {
    vertex->coord = c;
  }

  for (int i = 0; i < e_->lhs().size(); i++)
  {
    CHECK_EQ(e_->lhs()[i]->kind(), ktIdentifier);
    string var_name = static_cast<Identifier *>(e_->lhs()[i])->name();
    t0.update_var(var_name, v_rhs.id, i, _domain_signatures.back(), Edge::SCALAR_DATA_SIG, c);
  }
  vertex = g_.get_vertex(v_rhs.id);
  ComputationVertex *vv = static_cast<ComputationVertex *>(vertex);
  vv->is_on_dpu = true;

  t_ = t0;

  return id_ghv;
}

int Converter::convert_integer(Integer *e_, CidfgGraph &g_, VarTable &t_, StorageAllocationMap &reg_allocation_, StorageAllocationMap &sram_allocation_, int parent_vertex_id_, int child_index_)
{
  ConstVertex v;
  v.set_float_value((float)(e_->value));
  v.set_int_value(e_->value);
  return g_.add_vertex(v, parent_vertex_id_, child_index_);
}
int Converter::convert_floating_point(FloatingPoint *e_, CidfgGraph &g_, VarTable &t_, StorageAllocationMap &reg_allocation_, StorageAllocationMap &sram_allocation_, int parent_vertex_id_, int child_index_)
{
  ConstVertex v;
  v.set_int_value((int)(e_->value));
  v.set_float_value(e_->value);
  return g_.add_vertex(v, parent_vertex_id_, child_index_);
}

void Converter::convert_pragma_statement(PragmaStatement *e_, CidfgGraph &g_, VarTable &t_, StorageAllocationMap &reg_allocation_, StorageAllocationMap &sram_allocation_, int parent_vertex_id_, int child_index_)
{
  if (e_->pragma()->blockProgramType() == bptResourceSharing)
  {
    if (e_->pragma()->isBlockBegin())
    {
      _res_share_on = true;
      _vt_res_share.clear();
    }
    else
    {
      _res_share_on = false;
      _vt_res_share.clear();
    }
  }
  else
  {
    LOG(FATAL) << "Unsupported pragma statement.";
  }
}

// Deprecated
// This part of code is moved to feopt/RwiMergeEngine

// void Converter::merge_rw_index(CidfgGraph &g_)
// {
//   set<int> children_id;
//   merge_rw_index(g_, children_id);
//   for (auto &id : children_id)
//   {
//     g_.del_vertex(id);
//   }
// }
// void Converter::merge_rw_index(CidfgGraph &g_, set<int> &children_id_, int vertex_id_, int parent_vertex_id_, int child_index_)
// {
//   Vertex *vertex;
//   if (vertex_id_ < 0)
//   {
//     for (auto &v : g_.get_vertices())
//     {
//       if (v->vertex_type == Vertex::ROOT_VERTEX)
//       {
//         vertex = v;
//         break;
//       }
//     }
//   }
//   else
//   {
//     vertex = g_.get_vertex(vertex_id_);
//   }

//   if (vertex->vertex_type == Vertex::READING_VERTEX)
//   {
//     Edge *e0 = g_.get_edge(g_.get_in_edge(vertex->id, 0));
//     Edge *e1 = g_.get_edge(g_.get_in_edge(vertex->id, 1));
//     Vertex *v_index = g_.get_vertex(e1->src_id);
//     CHECK_EQ(v_index->vertex_type, Vertex::INDEXING_VERTEX);
//     ReadAndIndexVertex v_rdi;
//     v_rdi.is_sram = static_cast<ReadingVertex *>(vertex)->is_sram;
//     int id_rdi = g_.add_vertex(v_rdi, parent_vertex_id_, child_index_);
//     e0->dest_id = id_rdi;
//     e0->dest_port = 0;
//     for (int i = 0; i < 5; i++)
//     {
//       Edge *e;
//       e = g_.get_edge(g_.get_in_edge(v_index->id, i));
//       e->dest_id = id_rdi;
//       e->dest_port = i + 1;
//     }
//     g_.del_edge(e1->id);
//     Edge *e3 = g_.get_edge(g_.get_out_edge(vertex->id, 0));
//     e3->src_id = id_rdi;
//     e3->src_port = 0;

//     for (auto &ed : g_.get_edges())
//     {
//       //      if (ed->edge_type == Edge::DEPENDENCY)
//       //      {
//       if (ed->src_id == vertex->id)
//       {
//         ed->src_id = id_rdi;
//       }
//       if (ed->dest_id == vertex->id)
//       {
//         ed->dest_id = id_rdi;
//       }
//       //      }
//     }

//     children_id_.insert(vertex->id);
//     children_id_.insert(v_index->id);
//   }
//   else if (vertex->vertex_type == Vertex::WRITING_VERTEX)
//   {
//     Edge *e0 = g_.get_edge(g_.get_in_edge(vertex->id, 0));
//     Edge *e1 = g_.get_edge(g_.get_in_edge(vertex->id, 1));
//     Edge *e2 = g_.get_edge(g_.get_in_edge(vertex->id, 2));
//     Vertex *v_index = g_.get_vertex(e1->src_id);
//     CHECK_EQ(v_index->vertex_type, Vertex::INDEXING_VERTEX);
//     WriteAndIndexVertex v_wri;
//     v_wri.is_sram = static_cast<WritingVertex *>(vertex)->is_sram;
//     int id_wri = g_.add_vertex(v_wri, parent_vertex_id_, child_index_);
//     e0->dest_id = id_wri;
//     e0->dest_port = 0;
//     e2->dest_id = id_wri;
//     e2->dest_port = 1;
//     for (int i = 0; i < 5; i++)
//     {
//       Edge *e;
//       e = g_.get_edge(g_.get_in_edge(v_index->id, i));
//       e->dest_id = id_wri;
//       e->dest_port = i + 2;
//     }
//     g_.del_edge(e1->id);

//     for (auto &e3 : g_.get_edges())
//     {
//       if (e3->src_id == vertex->id && e3->src_port == 0)
//       {
//         e3->src_id = id_wri;
//         e3->src_port = 0;
//       }
//     }

//     for (auto &ed : g_.get_edges())
//     {
//       //     if (ed->edge_type == Edge::DEPENDENCY)
//       //     {
//       if (ed->src_id == vertex->id)
//       {
//         ed->src_id = id_wri;
//       }
//       if (ed->dest_id == vertex->id)
//       {
//         ed->dest_id = id_wri;
//       }
//       //     }
//     }

//     children_id_.insert(vertex->id);
//     children_id_.insert(v_index->id);
//   }
//   else if (vertex->is_hierarchical())
//   {
//     HierarchicalVertex *vv = static_cast<HierarchicalVertex *>(vertex);
//     for (int i = 0; i < vv->children.size(); i++)
//     {
//       for (int j = 0; j < vv->children[i].size(); j++)
//       {
//         merge_rw_index(g_, children_id_, vv->children[i][j], vv->id, i);
//       }
//     }
//   }
// }

// void Converter::remove_source_sink(CidfgGraph &g_)
// {
//   set<int> remove_id;
//   for (auto &v : g_.get_vertices())
//   {
//     if (v->vertex_type == Vertex::SINK_VERTEX)
//     {
//       Vertex *vv = g_.get_vertex(g_.get_edge(g_.get_in_edge(v->id, 0))->src_id);
//       if (vv->vertex_type == Vertex::SRAM_VAR_VERTEX || vv->vertex_type == Vertex::REG_VAR_VERTEX || vv->vertex_type == Vertex::VAR_VERTEX)
//       {
//         g_.del_edge(g_.get_in_edge(v->id, 0));
//         remove_id.insert(v->id);
//         remove_id.insert(vv->id);
//       }
//     }
//   }
//   for (auto id : remove_id)
//   {
//     g_.del_vertex(id);
//   }
// }

void Converter::gen_script()
{
  vesyla::util::GlobalVar glv;
  string path;
  CHECK(glv.gets("$OUTPUT_DIR", path));
  path = path + "cidfg/";
  mkdir(path.c_str(), 0755);
  string path_dot;
  path_dot = path + "convert.sh";
  ofstream ofs0(path_dot, ofstream::out);
  cidfg::ScriptGenerator sg;
  ofs0 << sg.generate();
  ofs0.close();
  chmod(path_dot.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
}

} // namespace cidfg
} // namespace vesyla