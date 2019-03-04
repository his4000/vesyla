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

#include "CidfgGraph.hpp"

namespace vesyla
{
namespace cidfg
{

CidfgGraph::CidfgGraph()
{
	id_counter = 0;
}

CidfgGraph::~CidfgGraph()
{
	for (auto &p : _vertices)
	{
		delete p.second;
	}
	for (auto &p : _edges)
	{
		delete p.second;
	}
}

CidfgGraph &CidfgGraph::operator=(const CidfgGraph &rhs)
{
	if (this == &rhs)
		return *this;
	id_counter = 0;
	for (auto &p : _vertices)
	{
		delete p.second;
	}
	for (auto &p : _edges)
	{
		delete p.second;
	}
	_vertices.clear();
	_edges.clear();
	unordered_map<int, int> id_map;
	for (Vertex *v : rhs.get_vertices())
	{
		Vertex *ptr = v;
		int new_id;
		switch (ptr->vertex_type)
		{
		case Vertex::CONST_VERTEX:
			new_id = add_vertex(*(static_cast<ConstVertex *>(ptr)));
			break;
		case Vertex::VAR_VERTEX:
			new_id = add_vertex(*(static_cast<VarVertex *>(ptr)));
			break;
		case Vertex::REG_VAR_VERTEX:
			new_id = add_vertex(*(static_cast<RegVarVertex *>(ptr)));
			break;
		case Vertex::SRAM_VAR_VERTEX:
			new_id = add_vertex(*(static_cast<SramVarVertex *>(ptr)));
			break;
		case Vertex::INDEXING_VERTEX:
			new_id = add_vertex(*(static_cast<IndexingVertex *>(ptr)));
			break;
		case Vertex::READ_AND_INDEX_VERTEX:
			new_id = add_vertex(*(static_cast<ReadAndIndexVertex *>(ptr)));
			break;
		case Vertex::WRITE_AND_INDEX_VERTEX:
			new_id = add_vertex(*(static_cast<WriteAndIndexVertex *>(ptr)));
			break;
		case Vertex::COMPUTATION_VERTEX:
			new_id = add_vertex(*(static_cast<ComputationVertex *>(ptr)));
			break;
		case Vertex::READING_VERTEX:
			new_id = add_vertex(*(static_cast<ReadingVertex *>(ptr)));
			break;
		case Vertex::WRITING_VERTEX:
			new_id = add_vertex(*(static_cast<WritingVertex *>(ptr)));
			break;
		case Vertex::REG_SINK_VERTEX:
			new_id = add_vertex(*(static_cast<RegSinkVertex *>(ptr)));
			break;
		case Vertex::SRAM_SINK_VERTEX:
			new_id = add_vertex(*(static_cast<SramSinkVertex *>(ptr)));
			break;
		case Vertex::VAR_SINK_VERTEX:
			new_id = add_vertex(*(static_cast<VarSinkVertex *>(ptr)));
			break;
		case Vertex::GENERAL_HIERARCHICAL_VERTEX:
			new_id = add_vertex(*(static_cast<GeneralHierarchicalVertex *>(ptr)));
			break;
		case Vertex::BRANCH_VERTEX:
			new_id = add_vertex(*(static_cast<BranchVertex *>(ptr)));
			break;
		case Vertex::LOOP_VERTEX:
			new_id = add_vertex(*(static_cast<LoopVertex *>(ptr)));
			break;
		case Vertex::ROOT_VERTEX:
			new_id = add_vertex(*(static_cast<RootVertex *>(ptr)));
			break;
		}
		id_map[v->id] = new_id;
	}

	for (Vertex *v : get_vertices())
	{
		Vertex *ptr = v;
		if (v->is_hierarchical())
		{
			HierarchicalVertex *vv = static_cast<HierarchicalVertex *>(ptr);
			for (int i = 0; i < vv->children.size(); i++)
			{
				for (int j = 0; j < vv->children[i].size(); j++)
				{
					vv->children[i][j] = id_map[vv->children[i][j]];
				}
			}
		}
	}

	for (Edge *e : rhs.get_edges())
	{
		Edge e0 = *e;
		e0.src_id = id_map[e->src_id];
		e0.dest_id = id_map[e->dest_id];
		add_edge(e0);
	}
	return *this;
}

int CidfgGraph::add_vertex(ConstVertex v_, int parent_vertex_id_, int child_index_)
{
	ConstVertex *pv = new ConstVertex;
	*pv = v_;
	pv->id = id_counter;
	id_counter++;
	_vertices[pv->id] = pv;
	if (parent_vertex_id_ >= 0 && child_index_ >= 0)
	{
		HierarchicalVertex *parent = static_cast<HierarchicalVertex *>(get_vertex(parent_vertex_id_));
		parent->add_child(child_index_, pv->id);
	}
	return pv->id;
}
int CidfgGraph::add_vertex(VarVertex v_, int parent_vertex_id_, int child_index_)
{
	VarVertex *pv = new VarVertex;
	*pv = v_;
	pv->id = id_counter;
	id_counter++;
	_vertices[pv->id] = pv;
	if (parent_vertex_id_ >= 0 && child_index_ >= 0)
	{
		HierarchicalVertex *parent = static_cast<HierarchicalVertex *>(get_vertex(parent_vertex_id_));
		parent->add_child(child_index_, pv->id);
	}
	return pv->id;
}
int CidfgGraph::add_vertex(RegVarVertex v_, int parent_vertex_id_, int child_index_)
{
	RegVarVertex *pv = new RegVarVertex;
	*pv = v_;
	pv->id = id_counter;
	id_counter++;
	_vertices[pv->id] = pv;
	if (parent_vertex_id_ >= 0 && child_index_ >= 0)
	{
		HierarchicalVertex *parent = static_cast<HierarchicalVertex *>(get_vertex(parent_vertex_id_));
		parent->add_child(child_index_, pv->id);
	}
	return pv->id;
}
int CidfgGraph::add_vertex(SramVarVertex v_, int parent_vertex_id_, int child_index_)
{
	SramVarVertex *pv = new SramVarVertex;
	*pv = v_;
	pv->id = id_counter;
	id_counter++;
	_vertices[pv->id] = pv;
	if (parent_vertex_id_ >= 0 && child_index_ >= 0)
	{
		HierarchicalVertex *parent = static_cast<HierarchicalVertex *>(get_vertex(parent_vertex_id_));
		parent->add_child(child_index_, pv->id);
	}
	return pv->id;
}
int CidfgGraph::add_vertex(ComputationVertex v_, int parent_vertex_id_, int child_index_)
{
	ComputationVertex *pv = new ComputationVertex;
	*pv = v_;
	pv->id = id_counter;
	id_counter++;
	_vertices[pv->id] = pv;
	if (parent_vertex_id_ >= 0 && child_index_ >= 0)
	{
		HierarchicalVertex *parent = static_cast<HierarchicalVertex *>(get_vertex(parent_vertex_id_));
		parent->add_child(child_index_, pv->id);
	}
	return pv->id;
}
int CidfgGraph::add_vertex(IndexingVertex v_, int parent_vertex_id_, int child_index_)
{
	IndexingVertex *pv = new IndexingVertex;
	*pv = v_;
	pv->id = id_counter;
	id_counter++;
	_vertices[pv->id] = pv;
	if (parent_vertex_id_ >= 0 && child_index_ >= 0)
	{
		HierarchicalVertex *parent = static_cast<HierarchicalVertex *>(get_vertex(parent_vertex_id_));
		parent->add_child(child_index_, pv->id);
	}
	return pv->id;
}
int CidfgGraph::add_vertex(ReadingVertex v_, int parent_vertex_id_, int child_index_)
{
	ReadingVertex *pv = new ReadingVertex;
	*pv = v_;
	pv->id = id_counter;
	id_counter++;
	_vertices[pv->id] = pv;
	if (parent_vertex_id_ >= 0 && child_index_ >= 0)
	{
		HierarchicalVertex *parent = static_cast<HierarchicalVertex *>(get_vertex(parent_vertex_id_));
		parent->add_child(child_index_, pv->id);
	}
	return pv->id;
}
int CidfgGraph::add_vertex(WritingVertex v_, int parent_vertex_id_, int child_index_)
{
	WritingVertex *pv = new WritingVertex;
	*pv = v_;
	pv->id = id_counter;
	id_counter++;
	_vertices[pv->id] = pv;
	if (parent_vertex_id_ >= 0 && child_index_ >= 0)
	{
		HierarchicalVertex *parent = static_cast<HierarchicalVertex *>(get_vertex(parent_vertex_id_));
		parent->add_child(child_index_, pv->id);
	}
	return pv->id;
}
int CidfgGraph::add_vertex(RegSinkVertex v_, int parent_vertex_id_, int child_index_)
{
	SinkVertex *pv = new RegSinkVertex;
	*pv = v_;
	pv->id = id_counter;
	id_counter++;
	_vertices[pv->id] = pv;
	if (parent_vertex_id_ >= 0 && child_index_ >= 0)
	{
		HierarchicalVertex *parent = static_cast<HierarchicalVertex *>(get_vertex(parent_vertex_id_));
		parent->add_child(child_index_, pv->id);
	}
	return pv->id;
}
int CidfgGraph::add_vertex(SramSinkVertex v_, int parent_vertex_id_, int child_index_)
{
	SinkVertex *pv = new SramSinkVertex;
	*pv = v_;
	pv->id = id_counter;
	id_counter++;
	_vertices[pv->id] = pv;
	if (parent_vertex_id_ >= 0 && child_index_ >= 0)
	{
		HierarchicalVertex *parent = static_cast<HierarchicalVertex *>(get_vertex(parent_vertex_id_));
		parent->add_child(child_index_, pv->id);
	}
	return pv->id;
}
int CidfgGraph::add_vertex(VarSinkVertex v_, int parent_vertex_id_, int child_index_)
{
	SinkVertex *pv = new VarSinkVertex;
	*pv = v_;
	pv->id = id_counter;
	id_counter++;
	_vertices[pv->id] = pv;
	if (parent_vertex_id_ >= 0 && child_index_ >= 0)
	{
		HierarchicalVertex *parent = static_cast<HierarchicalVertex *>(get_vertex(parent_vertex_id_));
		parent->add_child(child_index_, pv->id);
	}
	return pv->id;
}
int CidfgGraph::add_vertex(GeneralHierarchicalVertex v_, int parent_vertex_id_, int child_index_)
{
	GeneralHierarchicalVertex *pv = new GeneralHierarchicalVertex;
	*pv = v_;
	pv->id = id_counter;
	id_counter++;
	_vertices[pv->id] = pv;
	if (parent_vertex_id_ >= 0 && child_index_ >= 0)
	{
		HierarchicalVertex *parent = static_cast<HierarchicalVertex *>(get_vertex(parent_vertex_id_));
		parent->add_child(child_index_, pv->id);
	}
	return pv->id;
}
int CidfgGraph::add_vertex(BranchVertex v_, int parent_vertex_id_, int child_index_)
{
	BranchVertex *pv = new BranchVertex;
	*pv = v_;
	pv->id = id_counter;
	id_counter++;
	_vertices[pv->id] = pv;
	if (parent_vertex_id_ >= 0 && child_index_ >= 0)
	{
		HierarchicalVertex *parent = static_cast<HierarchicalVertex *>(get_vertex(parent_vertex_id_));
		parent->add_child(child_index_, pv->id);
	}
	return pv->id;
}
int CidfgGraph::add_vertex(LoopVertex v_, int parent_vertex_id_, int child_index_)
{
	LoopVertex *pv = new LoopVertex;
	*pv = v_;
	pv->id = id_counter;
	id_counter++;
	_vertices[pv->id] = pv;
	if (parent_vertex_id_ >= 0 && child_index_ >= 0)
	{
		HierarchicalVertex *parent = static_cast<HierarchicalVertex *>(get_vertex(parent_vertex_id_));
		parent->add_child(child_index_, pv->id);
	}
	return pv->id;
}

int CidfgGraph::add_vertex(RootVertex v_, int parent_vertex_id_, int child_index_)
{
	RootVertex *pv = new RootVertex;
	*pv = v_;
	pv->id = id_counter;
	id_counter++;
	_vertices[pv->id] = pv;
	if (parent_vertex_id_ >= 0 && child_index_ >= 0)
	{
		HierarchicalVertex *parent = static_cast<HierarchicalVertex *>(get_vertex(parent_vertex_id_));
		parent->add_child(child_index_, pv->id);
	}
	return pv->id;
}

int CidfgGraph::add_vertex(ReadAndIndexVertex v_, int parent_vertex_id_, int child_index_)
{
	ReadAndIndexVertex *pv = new ReadAndIndexVertex;
	*pv = v_;
	pv->id = id_counter;
	id_counter++;
	_vertices[pv->id] = pv;
	if (parent_vertex_id_ >= 0 && child_index_ >= 0)
	{
		HierarchicalVertex *parent = static_cast<HierarchicalVertex *>(get_vertex(parent_vertex_id_));
		parent->add_child(child_index_, pv->id);
	}
	return pv->id;
}

int CidfgGraph::add_vertex(WriteAndIndexVertex v_, int parent_vertex_id_, int child_index_)
{
	WriteAndIndexVertex *pv = new WriteAndIndexVertex;
	*pv = v_;
	pv->id = id_counter;
	id_counter++;
	_vertices[pv->id] = pv;
	if (parent_vertex_id_ >= 0 && child_index_ >= 0)
	{
		HierarchicalVertex *parent = static_cast<HierarchicalVertex *>(get_vertex(parent_vertex_id_));
		parent->add_child(child_index_, pv->id);
	}
	return pv->id;
}

int CidfgGraph::add_vertex(MergeVertex v_, int parent_vertex_id_, int child_index_)
{
	MergeVertex *pv = new MergeVertex;
	*pv = v_;
	pv->id = id_counter;
	id_counter++;
	_vertices[pv->id] = pv;
	if (parent_vertex_id_ >= 0 && child_index_ >= 0)
	{
		HierarchicalVertex *parent = static_cast<HierarchicalVertex *>(get_vertex(parent_vertex_id_));
		parent->add_child(child_index_, pv->id);
	}
	return pv->id;
}

int CidfgGraph::add_vertex(RaccuInstrVertex v_, int parent_vertex_id_, int child_index_)
{
	RaccuInstrVertex *pv = new RaccuInstrVertex;
	*pv = v_;
	pv->id = id_counter;
	id_counter++;
	_vertices[pv->id] = pv;
	if (parent_vertex_id_ >= 0 && child_index_ >= 0)
	{
		HierarchicalVertex *parent = static_cast<HierarchicalVertex *>(get_vertex(parent_vertex_id_));
		parent->add_child(child_index_, pv->id);
	}
	return pv->id;
}

int CidfgGraph::add_vertex(DpuInstrVertex v_, int parent_vertex_id_, int child_index_)
{
	DpuInstrVertex *pv = new DpuInstrVertex;
	*pv = v_;
	pv->id = id_counter;
	id_counter++;
	_vertices[pv->id] = pv;
	if (parent_vertex_id_ >= 0 && child_index_ >= 0)
	{
		HierarchicalVertex *parent = static_cast<HierarchicalVertex *>(get_vertex(parent_vertex_id_));
		parent->add_child(child_index_, pv->id);
	}
	return pv->id;
}
int CidfgGraph::add_vertex(LoophInstrVertex v_, int parent_vertex_id_, int child_index_)
{
	LoophInstrVertex *pv = new LoophInstrVertex;
	*pv = v_;
	pv->id = id_counter;
	id_counter++;
	_vertices[pv->id] = pv;
	if (parent_vertex_id_ >= 0 && child_index_ >= 0)
	{
		HierarchicalVertex *parent = static_cast<HierarchicalVertex *>(get_vertex(parent_vertex_id_));
		parent->add_child(child_index_, pv->id);
	}
	return pv->id;
}
int CidfgGraph::add_vertex(LooptInstrVertex v_, int parent_vertex_id_, int child_index_)
{
	LooptInstrVertex *pv = new LooptInstrVertex;
	*pv = v_;
	pv->id = id_counter;
	id_counter++;
	_vertices[pv->id] = pv;
	if (parent_vertex_id_ >= 0 && child_index_ >= 0)
	{
		HierarchicalVertex *parent = static_cast<HierarchicalVertex *>(get_vertex(parent_vertex_id_));
		parent->add_child(child_index_, pv->id);
	}
	return pv->id;
}
int CidfgGraph::add_vertex(HierarchicalInstrVertex v_, int parent_vertex_id_, int child_index_)
{
	HierarchicalInstrVertex *pv = new HierarchicalInstrVertex;
	*pv = v_;
	pv->id = id_counter;
	id_counter++;
	_vertices[pv->id] = pv;
	if (parent_vertex_id_ >= 0 && child_index_ >= 0)
	{
		HierarchicalVertex *parent = static_cast<HierarchicalVertex *>(get_vertex(parent_vertex_id_));
		parent->add_child(child_index_, pv->id);
	}
	return pv->id;
}

int CidfgGraph::add_vertex(SwbInstrVertex v_, int parent_vertex_id_, int child_index_)
{
	SwbInstrVertex *pv = new SwbInstrVertex;
	*pv = v_;
	pv->id = id_counter;
	id_counter++;
	_vertices[pv->id] = pv;
	if (parent_vertex_id_ >= 0 && child_index_ >= 0)
	{
		HierarchicalVertex *parent = static_cast<HierarchicalVertex *>(get_vertex(parent_vertex_id_));
		parent->add_child(child_index_, pv->id);
	}
	return pv->id;
}
int CidfgGraph::add_vertex(RouteInstrVertex v_, int parent_vertex_id_, int child_index_)
{
	RouteInstrVertex *pv = new RouteInstrVertex;
	*pv = v_;
	pv->id = id_counter;
	id_counter++;
	_vertices[pv->id] = pv;
	if (parent_vertex_id_ >= 0 && child_index_ >= 0)
	{
		HierarchicalVertex *parent = static_cast<HierarchicalVertex *>(get_vertex(parent_vertex_id_));
		parent->add_child(child_index_, pv->id);
	}
	return pv->id;
}
int CidfgGraph::add_vertex(RefiInstrVertex v_, int parent_vertex_id_, int child_index_)
{
	RefiInstrVertex *pv = new RefiInstrVertex;
	*pv = v_;
	pv->id = id_counter;
	id_counter++;
	_vertices[pv->id] = pv;
	if (parent_vertex_id_ >= 0 && child_index_ >= 0)
	{
		HierarchicalVertex *parent = static_cast<HierarchicalVertex *>(get_vertex(parent_vertex_id_));
		parent->add_child(child_index_, pv->id);
	}
	return pv->id;
}
int CidfgGraph::add_vertex(SramInstrVertex v_, int parent_vertex_id_, int child_index_)
{
	SramInstrVertex *pv = new SramInstrVertex;
	*pv = v_;
	pv->id = id_counter;
	id_counter++;
	_vertices[pv->id] = pv;
	if (parent_vertex_id_ >= 0 && child_index_ >= 0)
	{
		HierarchicalVertex *parent = static_cast<HierarchicalVertex *>(get_vertex(parent_vertex_id_));
		parent->add_child(child_index_, pv->id);
	}
	return pv->id;
}
int CidfgGraph::add_vertex(BranchInstrVertex v_, int parent_vertex_id_, int child_index_)
{
	BranchInstrVertex *pv = new BranchInstrVertex;
	*pv = v_;
	pv->id = id_counter;
	id_counter++;
	_vertices[pv->id] = pv;
	if (parent_vertex_id_ >= 0 && child_index_ >= 0)
	{
		HierarchicalVertex *parent = static_cast<HierarchicalVertex *>(get_vertex(parent_vertex_id_));
		parent->add_child(child_index_, pv->id);
	}
	return pv->id;
}
int CidfgGraph::add_vertex(WaitInstrVertex v_, int parent_vertex_id_, int child_index_)
{
	WaitInstrVertex *pv = new WaitInstrVertex;
	*pv = v_;
	pv->id = id_counter;
	id_counter++;
	_vertices[pv->id] = pv;
	if (parent_vertex_id_ >= 0 && child_index_ >= 0)
	{
		HierarchicalVertex *parent = static_cast<HierarchicalVertex *>(get_vertex(parent_vertex_id_));
		parent->add_child(child_index_, pv->id);
	}
	return pv->id;
}

int CidfgGraph::add_vertex(JumpInstrVertex v_, int parent_vertex_id_, int child_index_)
{
	JumpInstrVertex *pv = new JumpInstrVertex;
	*pv = v_;
	pv->id = id_counter;
	id_counter++;
	_vertices[pv->id] = pv;
	if (parent_vertex_id_ >= 0 && child_index_ >= 0)
	{
		HierarchicalVertex *parent = static_cast<HierarchicalVertex *>(get_vertex(parent_vertex_id_));
		parent->add_child(child_index_, pv->id);
	}
	return pv->id;
}

int CidfgGraph::add_edge(Edge e_)
{
	Edge *pe = new Edge;
	*pe = e_;
	pe->id = id_counter;
	id_counter++;
	_edges[pe->id] = pe;
	return pe->id;
}

void CidfgGraph::del_vertex(int id_)
{
	if (_vertices.find(id_) != _vertices.end())
	{
		delete _vertices[id_];
		_vertices.erase(id_);

		for (auto &v : _vertices)
		{
			if (v.second->is_hierarchical())
			{
				HierarchicalVertex *vv = static_cast<HierarchicalVertex *>(v.second);
				for (int i = 0; i < vv->children.size(); i++)
				{
					vector<int> vec;
					for (int j = 0; j < vv->children[i].size(); j++)
					{
						if (vv->children[i][j] != id_)
						{
							vec.push_back(vv->children[i][j]);
						}
					}
					vv->children[i] = vec;
				}
			}
		}

		for (auto &e : get_edges())
		{
			if (e->src_id == id_ || e->dest_id == id_)
			{
				del_edge(e->id);
			}
		}
	}
}

void CidfgGraph::del_edge(int id_)
{
	if (_edges.find(id_) != _edges.end())
	{
		delete _edges[id_];
		_edges.erase(id_);
	}
}

string CidfgGraph::generate_dot_graph()
{
	string str = "digraph g {\n";
	set<int> vertex_set;
	for (auto &v : _vertices)
	{
		if (v.second->vertex_type == Vertex::ROOT_VERTEX)
		{
			str += generate_dot_graph_for_vertex(v.second->id, vertex_set);
		}
		// else if (v.second->vertex_type == Vertex::HIERARCHICAL_INSTR_VERTEX)
		// {
		// 	HierarchicalInstrVertex *vv = static_cast<HierarchicalInstrVertex *>(v.second);
		// 	if (vv->name == "ROOT")
		// 	{
		// 		str += generate_dot_graph_for_vertex(v.second->id, vertex_set);
		// 	}
		// }
	}

	for (auto &e : _edges)
	{
		if (vertex_set.find(e.second->src_id) != vertex_set.end() && vertex_set.find(e.second->dest_id) != vertex_set.end())
		{
			str += to_string(e.second->src_id) + " -> " + to_string(e.second->dest_id) + "[taillabel=\"" + to_string(e.second->src_port) + "\", headlabel=\"" + to_string(e.second->dest_port) + "\"";
			string temp_var_prefix = "temp_var_";
			if (e.second->edge_type != Edge::DEPENDENCY && (e.second->var_name != "" && strncmp(e.second->var_name.c_str(), temp_var_prefix.c_str(), temp_var_prefix.size())))
			{
				str += ", label=\"" + e.second->var_name + "\"";
			}
			else if (e.second->edge_type == Edge::DEPENDENCY)
			{
				string hi = "+INF";
				string lo = "-INF";
				if (e.second->d_lo > INT_MIN)
				{
					lo = to_string(e.second->d_lo);
				}
				if (e.second->d_hi < INT_MAX)
				{
					hi = to_string(e.second->d_hi);
				}
				str += ", label=\"[" + lo + "," + hi + "]\"";
			}

			if (e.second->edge_type == Edge::VECTOR_DATA_SIG)
			{
				str += ", style=bold";
			}
			else if (e.second->edge_type == Edge::SRAM_VECTOR_DATA_SIG)
			{
				str += ", style=bold, color=red";
			}
			else if (e.second->edge_type == Edge::DEPENDENCY && !e.second->is_weak_dependency())
			{
				str += ", color=red";
			}
			else if (e.second->edge_type == Edge::DEPENDENCY && e.second->is_weak_dependency())
			{
				str += ", style=dashed, color=red";
			}
			str += "];\n";
		}
	}

	str += "}";
	return str;
}

string CidfgGraph::generate_dot_graph_for_vertex(int vid, set<int> &vertex_set_)
{
	string str = "";
	Vertex *v = get_vertex(vid);

	string coord = "";
	if (v->coord.is_valid())
	{
		coord = "/" + v->coord.to_str();
	}

	switch (v->vertex_type)
	{
	case Vertex::READING_VERTEX:
	{
		if (static_cast<ReadingVertex *>(v)->is_sram)
		{
			str += to_string(v->id) + "[shape=circle, style=filled, colorscheme=prgn4, color=1, fillcolor=3, label=\"R" + coord + "\"];\n";
		}
		else
		{
			str += to_string(v->id) + "[shape=circle, style=filled, colorscheme=prgn4, color=4, fillcolor=3, label=\"R" + coord + "\"];\n";
		}
		break;
	}
	case Vertex::WRITING_VERTEX:
	{
		if (static_cast<WritingVertex *>(v)->is_sram)
		{
			str += to_string(v->id) + "[shape=circle, style=filled, colorscheme=prgn4, color=1, fillcolor=3, label=\"W" + coord + "\"];\n";
		}
		else
		{
			str += to_string(v->id) + "[shape=circle, style=filled, colorscheme=prgn4, color=4, fillcolor=3, label=\"W" + coord + "\"];\n";
		}
		break;
	}
	case Vertex::INDEXING_VERTEX:
	{
		str += to_string(v->id) + "[shape=circle, style=filled, colorscheme=ylorbr4, color=4, fillcolor=2, label=\"I" + coord + "\"];\n";
		break;
	}
	case Vertex::READ_AND_INDEX_VERTEX:
	{
		str += to_string(v->id) + "[shape=circle, style=filled, colorscheme=prgn4, color=4, fillcolor=3, label=\"RI" + coord + "\"];\n";
		break;
	}
	case Vertex::WRITE_AND_INDEX_VERTEX:
	{
		str += to_string(v->id) + "[shape=circle, style=filled, colorscheme=prgn4, color=4, fillcolor=3, label=\"WI" + coord + "\"];\n";
		break;
	}
	case Vertex::MERGE_VERTEX:
	{
		str += to_string(v->id) + "[shape=circle, style=filled, colorscheme=pubu4, color=4, fillcolor=2, label=\"MERGE" + coord + "\"];\n";
		break;
	}
	case Vertex::COMPUTATION_VERTEX:
	{
		ComputationVertex *vv = static_cast<ComputationVertex *>(v);
		str += to_string(vv->id) + "[shape=circle, style=filled, colorscheme=pubu4, color=4, fillcolor=2, label=\"" + vv->func_name + coord + "\"];\n";
		break;
	}
	case Vertex::VAR_VERTEX:
	{
		VarVertex *vv = static_cast<VarVertex *>(v);
		if (vv->is_iterator)
		{
			str += to_string(vv->id) + "[shape=box, style=filled, fillcolor=red, label=\"" + vv->var_name + coord + "\"];\n";
		}
		else
		{
			str += to_string(vv->id) + "[shape=box, label=\"" + vv->var_name + coord + "\"];\n";
		}
		break;
	}
	case Vertex::REG_VAR_VERTEX:
	{
		RegVarVertex *vv = static_cast<RegVarVertex *>(v);
		if (vv->value_type == SourceVertex::INT)
		{
			str += to_string(vv->id) + "[shape=box, label=\"" + vv->var_name + coord + "\"];\n";
		}
		else
		{
			str += to_string(vv->id) + "[shape=box, style=filled, fillcolor=green, label=\"" + vv->var_name + coord + "\"];\n";
		}
		break;
	}
	case Vertex::SRAM_VAR_VERTEX:
	{
		SramVarVertex *vv = static_cast<SramVarVertex *>(v);
		str += to_string(vv->id) + "[shape=box, color=red, label=\"" + vv->var_name + coord + "\"];\n";
		break;
	}
	case Vertex::CONST_VERTEX:
	{
		ConstVertex *vv = static_cast<ConstVertex *>(v);
		if (vv->value_type == ConstVertex::INT)
		{
			str += to_string(vv->id) + "[shape=box, label=\"" + to_string(vv->int_data) + coord + "\"];\n";
		}
		else
		{
			str += to_string(vv->id) + "[shape=box, label=\"" + to_string(vv->float_data) + coord + "\"];\n";
		}
		break;
	}
	case Vertex::REG_SINK_VERTEX:
	case Vertex::SRAM_SINK_VERTEX:
	case Vertex::VAR_SINK_VERTEX:
	{
		SinkVertex *vv = static_cast<SinkVertex *>(v);
		str += to_string(vv->id) + "[shape=box, label=\"" + vv->var_name + coord + "\"];\n";
		break;
	}
	case Vertex::GENERAL_HIERARCHICAL_VERTEX:
	{
		BranchVertex *vv = static_cast<BranchVertex *>(v);
		str += to_string(vv->id) + "[shape=box, peripheries=2, colorscheme=set28, color=1, label= \"GH" + coord + "\" ];\n";
		break;
	}
	case Vertex::BRANCH_VERTEX:
	{
		BranchVertex *vv = static_cast<BranchVertex *>(v);
		str += to_string(vv->id) + "[shape=box, peripheries=2, colorscheme=set28, color=1, label= \"BRANCH" + coord + "\" ];\n";
		break;
	}
	case Vertex::LOOP_VERTEX:
	{
		LoopVertex *vv = static_cast<LoopVertex *>(v);
		str += to_string(vv->id) + "[shape=box, peripheries=2, colorscheme=set28, color=1, label= \"LOOP" + coord + "\" ];\n";
		break;
	}
	case Vertex::ROOT_VERTEX:
	{
		RootVertex *vv = static_cast<RootVertex *>(v);
		str += to_string(vv->id) + "[shape=box, peripheries=2, colorscheme=set28, color=1, label= \"ROOT" + coord + "\" ];\n";
		break;
	}
	case Vertex::RACCU_INSTR_VERTEX:
	{
		RaccuInstrVertex *vv = static_cast<RaccuInstrVertex *>(v);
		str += to_string(vv->id) + "[peripheries=2, color=turquoise4, style=filled, fillcolor=lightcyan, label= \"RACCU" + coord + "\" ];\n";
		break;
	}
	case Vertex::DPU_INSTR_VERTEX:
	{
		DpuInstrVertex *vv = static_cast<DpuInstrVertex *>(v);
		str += to_string(vv->id) + "[peripheries=2, color=turquoise4, style=filled, fillcolor=lightcyan, label= \"DPU" + coord + "\" ];\n";
		break;
	}
	case Vertex::LOOPH_INSTR_VERTEX:
	{
		LoophInstrVertex *vv = static_cast<LoophInstrVertex *>(v);
		str += to_string(vv->id) + "[peripheries=2, color=turquoise4, style=filled, fillcolor=lightcyan, label= \"LOOPH" + coord + "\" ];\n";
		break;
	}
	case Vertex::LOOPT_INSTR_VERTEX:
	{
		LooptInstrVertex *vv = static_cast<LooptInstrVertex *>(v);
		str += to_string(vv->id) + "[peripheries=2, color=turquoise4, style=filled, fillcolor=lightcyan, label= \"LOOPT" + coord + "\" ];\n";
		break;
	}
	case Vertex::SWB_INSTR_VERTEX:
	{
		SwbInstrVertex *vv = static_cast<SwbInstrVertex *>(v);
		str += to_string(vv->id) + "[peripheries=2, color=turquoise4, style=filled, fillcolor=lightcyan, label= \"SWB" + coord + "\" ];\n";
		break;
	}
	case Vertex::ROUTE_INSTR_VERTEX:
	{
		RouteInstrVertex *vv = static_cast<RouteInstrVertex *>(v);
		str += to_string(vv->id) + "[peripheries=2, color=turquoise4, style=filled, fillcolor=lightcyan, label= \"ROUTE" + coord + "\" ];\n";
		break;
	}
	case Vertex::REFI_INSTR_VERTEX:
	{
		RefiInstrVertex *vv = static_cast<RefiInstrVertex *>(v);
		str += to_string(vv->id) + "[peripheries=2, color=turquoise4, style=filled, fillcolor=lightcyan, label= \"REFI" + coord + "\" ];\n";
		break;
	}
	case Vertex::SRAM_INSTR_VERTEX:
	{
		SramInstrVertex *vv = static_cast<SramInstrVertex *>(v);
		str += to_string(vv->id) + "[peripheries=2, color=turquoise4, style=filled, fillcolor=lightcyan, label= \"SRAM" + coord + "\" ];\n";
		break;
	}
	case Vertex::BRANCH_INSTR_VERTEX:
	{
		BranchInstrVertex *vv = static_cast<BranchInstrVertex *>(v);
		str += to_string(vv->id) + "[peripheries=2, color=turquoise4, style=filled, fillcolor=lightcyan, label= \"BRN" + coord + "\" ];\n";
		break;
	}
	case Vertex::WAIT_INSTR_VERTEX:
	{
		WaitInstrVertex *vv = static_cast<WaitInstrVertex *>(v);
		str += to_string(vv->id) + "[peripheries=2, color=turquoise4, style=filled, fillcolor=lightcyan, label= \"WAIT" + coord + "\" ];\n";
		break;
	}
	case Vertex::JUMP_INSTR_VERTEX:
	{
		JumpInstrVertex *vv = static_cast<JumpInstrVertex *>(v);
		str += to_string(vv->id) + "[peripheries=2, color=turquoise4, style=filled, fillcolor=lightcyan, label= \"JUMP" + coord + "\" ];\n";
		break;
	}
	case Vertex::HIERARCHICAL_INSTR_VERTEX:
	{
		HierarchicalInstrVertex *vv = static_cast<HierarchicalInstrVertex *>(v);
		if (vv->name == "")
		{
			str += to_string(vv->id) + "[shape=box, peripheries=2, color=lightblue, label= \"HIER" + coord + "\" ];\n";
		}
		else
		{
			str += to_string(vv->id) + "[shape=box, peripheries=2, color=lightblue, label= \"" + vv->name + coord + "\" ];\n";
		}
		break;
	}
	}

	if (v->is_hierarchical())
	{

		HierarchicalVertex *vv = static_cast<HierarchicalVertex *>(v);
		for (int i = 0; i < vv->children.size(); i++)
		{

			str += "subgraph cluster_" + to_string(vv->id) + "_" + to_string(i) + "{\n";
			if (i == 0)
			{
				str += "cluster_" + to_string(vv->id) + "_" + to_string(i) + "_entry [shape=circle, style=filled, fillcolor=palegreen1, label=\"\", width=.2, fixedsize=true];\n";
			}
			else if (i == 1)
			{
				str += "cluster_" + to_string(vv->id) + "_" + to_string(i) + "_entry [shape=diamond, style=filled, fillcolor=red, label=\"\"];\n";
			}
			else
			{
				str += "cluster_" + to_string(vv->id) + "_" + to_string(i) + "_entry [shape=diamond, style=filled, fillcolor=yellow, label=\"" + to_string(i) + "\"];\n";
			}

			for (int j = 0; j < vv->children[i].size(); j++)
			{
				vertex_set_.insert(vv->children[i][j]);
				str += generate_dot_graph_for_vertex(vv->children[i][j], vertex_set_);
			}

			str += "}\n";
			str += to_string(v->id) + "->" + "cluster_" + to_string(vv->id) + "_" + to_string(i) + "_entry" + "[color=lightblue];\n";
		}
	}

	return str;
}

int CidfgGraph::get_out_edge(int id_, int out_port_)
{
	for (auto &e : _edges)
	{
		if (e.second->src_id == id_ && e.second->src_port == out_port_)
		{
			return e.first;
		}
	}
	return -1;
}
int CidfgGraph::get_in_edge(int id_, int in_port_)
{
	for (auto &e : _edges)
	{
		if (e.second->dest_id == id_ && e.second->dest_port == in_port_)
		{
			return e.first;
		}
	}
	return -1;
}

Vertex *CidfgGraph::get_vertex(int id_)
{
	return _vertices[id_];
}
Edge *CidfgGraph::get_edge(int id_)
{
	return _edges[id_];
}
vector<Vertex *> CidfgGraph::get_vertices() const
{
	vector<Vertex *> vec;
	for (auto &v : _vertices)
	{
		vec.push_back(v.second);
	}
	return vec;
}
vector<Edge *> CidfgGraph::get_edges() const
{
	vector<Edge *> vec;
	for (auto &e : _edges)
	{
		vec.push_back(e.second);
	}
	return vec;
}

bool CidfgGraph::check_graph()
{
	for (auto &v : _vertices)
	{
		if (v.second == NULL)
		{
			return false;
		}
	}
	for (auto &e : _edges)
	{
		if (e.second == NULL)
		{
			return false;
		}
	}
	return true;
}

void CidfgGraph::print_graph()
{
	for (auto v : _vertices)
	{
		cout << "vertex: " << v.second->id << "(" << v.second->vertex_type << ")";
		if (v.second->vertex_type == Vertex::ROOT_VERTEX || v.second->vertex_type == Vertex::LOOP_VERTEX)
		{
			HierarchicalVertex *vv = static_cast<HierarchicalVertex *>(v.second);
			cout << "[";
			for (int i = 0; i < vv->children[0].size(); i++)
			{
				cout << vv->children[0][i] << ",";
			}
			cout << "]";
		}
		else if (v.second->vertex_type == Vertex::BRANCH_VERTEX)
		{
			HierarchicalVertex *vv = static_cast<HierarchicalVertex *>(v.second);
			cout << "[";
			for (int i = 0; i < vv->children[0].size(); i++)
			{
				cout << vv->children[0][i] << ",";
			}
			cout << "]";
			cout << " [";
			for (int i = 0; i < vv->children[1].size(); i++)
			{
				cout << vv->children[1][i] << ",";
			}
			cout << "]";
		}
		cout << endl;
	}
}

void CidfgGraph::get_parent(int id_, int &parent_id_, int &child_index_)
{
	parent_id_ = -1;
	child_index_ = -1;
	for (auto &v : get_vertices())
	{
		if (v->is_hierarchical())
		{
			HierarchicalVertex *vv = static_cast<HierarchicalVertex *>(v);
			for (int i = 0; i < vv->children.size(); i++)
			{
				for (int j = 0; j < vv->children[i].size(); j++)
				{
					if (vv->children[i][j] == id_)
					{
						parent_id_ = vv->id;
						child_index_ = i;
					}
				}
			}
		}
	}
}

} // namespace cidfg
} // namespace vesyla