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

#include "Descriptor.hpp"

namespace vesyla
{
namespace schedule
{

//void Descriptor::Operation::load(pt::ptree p){
//	_name = p.get<string>("name");
//
//	boost::optional<const pt::ptree&> opt_child;
//
//	opt_child = p.get_child_optional( "scheduled_time" );
//	if(opt_child){
//		scheduled_time = p.get<int>("scheduled_time");
//	}
//	opt_child = p.get_child_optional( "rot" );
//	if(opt_child){
//		for(pt::ptree::value_type &v : p.get_child("rot")){
//			string rs = v.second.get<string>("rs");
//			int t_start = v.second.get<int>("t_start");
//			int t_end = v.second.get<int>("t_end");
//			rot[rs] = {t_start, t_end};
//		}
//	}
//	opt_child = p.get_child_optional( "children" );
//	if(opt_child){
//		for(pt::ptree::value_type &v : p.get_child("children")){
//			string child = v.second.data();
//			children.push_back(child);
//		}
//	}
//}

//pt::ptree Descriptor::Operation::dump(){
//	pt::ptree p;
//	p.put("name", _name);
//	if(scheduled_time>=0){
//		p.put("scheduled_time", scheduled_time);
//	}
//	for(auto& r : rot){
//		pt::ptree pr;
//		pr.put("rs", r.first);
//		pr.put("t_start", r.second[0]);
//		pr.put("t_end", r.second[1]);
//		p.add_child("rot.resource", pr);
//	}
//	for(auto& c : children){
//		p.add("children.child", c);
//	}
//	return p;
//}

//void Descriptor::Constraint::load(pt::ptree p){
//	_src = p.get<string>("src");
//	_dest = p.get<string>("dest");
//
//	boost::optional<const pt::ptree&> opt_child;
//
//	opt_child = p.get_child_optional( "d_lo" );
//	if(opt_child){
//		d_lo = p.get<int>("d_lo");
//	}
//	opt_child = p.get_child_optional( "d_hi" );
//	if(opt_child){
//		d_hi = p.get<int>("d_hi");
//	}
//}

//pt::ptree Descriptor::Constraint::dump(){
//	pt::ptree p;
//	p.put("src", _src);
//	p.put("dest", _dest);
//	if(d_lo>INT_MIN){
//		p.put("d_lo", d_lo);
//	}
//	if(d_hi<INT_MAX){
//		p.put("d_hi", d_hi);
//	}
//	return p;
//}

//void Descriptor::Operation::add_child(const string name_){
//	bool flag=false;
//	for(auto& c:children){
//		if(c==name_){
//			flag=true;
//		}
//	}
//	if(!flag){
//		children.push_back(name_);
//	}
//}
//void Descriptor::Operation::add_children(vector<string> name_list_){
//	for(auto& n:name_list_){
//		add_child(n);
//	}
//}

void Descriptor::load(pt::ptree p)
{
	pt::ptree pd = p.get_child("Descriptor");

	boost::optional<const pt::ptree &> opt_child;
	opt_child = pd.get_child_optional("entry");
	if (opt_child)
	{
		_entry = pd.get<string>("entry");
	}
	else
	{
		_entry = "root";
	}

	opt_child = pd.get_child_optional("Operations");
	if (opt_child)
	{
		for (pt::ptree::value_type &v : pd.get_child("Operations"))
		{
			if (v.first == "Operation")
			{
				pt::ptree tree = v.second;
				Operation op;
				op.load(tree);
				_operations[op.name()] = op;
			}
		}
	}
	opt_child = pd.get_child_optional("Constraints");
	if (opt_child)
	{
		for (pt::ptree::value_type &v : pd.get_child("Constraints"))
		{
			if (v.first == "Constraint")
			{
				pt::ptree tree = v.second;
				Constraint c;
				c.load(tree);
				_constraints[c.src() + "_" + c.dest()] = c;
				_src_constraint_index[c.dest()].push_back(&_constraints[c.src() + "_" + c.dest()]);
				_dest_constraint_index[c.src()].push_back(&_constraints[c.src() + "_" + c.dest()]);
			}
		}
	}
	opt_child = pd.get_child_optional("Instructions");
	map<int, BIR::Instruction *> instr_map;
	if (opt_child)
	{
		for (pt::ptree::value_type &v : pd.get_child("Instructions"))
		{
			if (v.first == "Cell")
			{
				pt::ptree tree = v.second;
				string coord_signature = tree.get<string>("signature");
				if (_instr_lists.find(coord_signature) == _instr_lists.end())
				{
					_instr_lists[coord_signature] = {};
				}
				for (pt::ptree::value_type &vi : tree)
				{
					if (vi.first == "Instruction")
					{
						pt::ptree instr_tree = vi.second;
						string kind = instr_tree.get<string>("kind");
						int id = instr_tree.get<int>("id");
						if (kind == "SWBInstruction")
						{
							BIR::SWBInstruction *instr = CREATE_OBJECT_B(SWBInstruction);
							instr->load(instr_tree);
							instr_map[id] = instr;
							_instr_lists[coord_signature].push_back(instr);
						}
						else if (kind == "RefiInstruction")
						{
							BIR::RefiInstruction *instr = CREATE_OBJECT_B(RefiInstruction);
							instr->load(instr_tree);
							instr_map[id] = instr;
							_instr_lists[coord_signature].push_back(instr);
						}
						else if (kind == "DPUInstruction")
						{
							BIR::DPUInstruction *instr = CREATE_OBJECT_B(DPUInstruction);
							instr->load(instr_tree);
							instr_map[id] = instr;
							_instr_lists[coord_signature].push_back(instr);
						}
						else if (kind == "RACCUInstruction")
						{
							BIR::RACCUInstruction *instr = CREATE_OBJECT_B(RACCUInstruction);
							instr->load(instr_tree);
							instr_map[id] = instr;
							_instr_lists[coord_signature].push_back(instr);
						}
						else if (kind == "LoopHeaderInstruction")
						{
							BIR::LoopHeaderInstruction *instr = CREATE_OBJECT_B(LoopHeaderInstruction);
							instr->load(instr_tree);
							instr_map[id] = instr;
							_instr_lists[coord_signature].push_back(instr);
						}
						else if (kind == "LoopTailInstruction")
						{
							BIR::LoopTailInstruction *instr = CREATE_OBJECT_B(LoopTailInstruction);
							instr->load(instr_tree);
							instr_map[id] = instr;
							_instr_lists[coord_signature].push_back(instr);
						}
						else if (kind == "BrachInstruction")
						{
							BIR::BranchInstruction *instr = CREATE_OBJECT_B(BranchInstruction);
							instr->load(instr_tree);
							instr_map[id] = instr;
							_instr_lists[coord_signature].push_back(instr);
						}
						else if (kind == "JumpInstruction")
						{
							BIR::JumpInstruction *instr = CREATE_OBJECT_B(JumpInstruction);
							instr->load(instr_tree);
							instr_map[id] = instr;
							_instr_lists[coord_signature].push_back(instr);
						}
						else if (kind == "WaitInstruction")
						{
							BIR::WaitInstruction *instr = CREATE_OBJECT_B(WaitInstruction);
							instr->load(instr_tree);
							instr_map[id] = instr;
							_instr_lists[coord_signature].push_back(instr);
						}
						else if (kind == "RouteInstruction")
						{
							BIR::RouteInstruction *instr = CREATE_OBJECT_B(RouteInstruction);
							instr->load(instr_tree);
							instr_map[id] = instr;
							_instr_lists[coord_signature].push_back(instr);
						}
						else if (kind == "SRAMInstruciton")
						{
							BIR::SRAMInstruction *instr = CREATE_OBJECT_B(SRAMInstruction);
							instr->load(instr_tree);
							instr_map[id] = instr;
							_instr_lists[coord_signature].push_back(instr);
						}
					}
				}
			}
		}
	}

	for (auto &e : _instr_lists)
	{
		for (auto &i : e.second)
		{
			i->load2(instr_map);
		}
	}
}

pt::ptree Descriptor::dump()
{
	pt::ptree p;
	pt::ptree pd;

	pd.put("entry", _entry);
	for (auto &op : _operations)
	{
		pd.add_child("Operations.Operation", op.second.dump());
	}
	for (auto &c : _constraints)
	{
		pd.add_child("Constraints.Constraint", c.second.dump());
	}
	for (auto &m : _instr_lists)
	{
		pt::ptree pi;
		pi.put("signature", m.first);
		for (auto &i : m.second)
		{
			pi.add_child("Instruction", i->dump());
		}
		pd.add_child("Instructions.Cell", pi);
	}
	p.add_child("Descriptor", pd);
	return p;
}

void Descriptor::add_operation(Operation op_)
{
	auto find = _operations.find(op_.name());
	if (find != _operations.end())
	{
		LOG(WARNING) << "Overwrite operation " << op_.name();
	}
	_operations[op_.name()] = op_;
}

void Descriptor::add_constraint(Constraint c_)
{
	auto find_src = _operations.find(c_.src());
	auto find_dest = _operations.find(c_.dest());
	CHECK_NE(find_src, _operations.end()) << "Constraint " << c_.src() << "->" << c_.dest() << " is invalid, operation " << c_.src() << "does not exist!";
	CHECK_NE(find_dest, _operations.end()) << "Constraint " << c_.src() << "->" << c_.dest() << " is invalid, operation " << c_.dest() << "does not exist!";
	CHECK_GE(c_.d_hi, c_.d_lo) << "Constraint " << c_.src() << "->" << c_.dest() << " is invalid, illegl delay!";
	_constraints[c_.src() + "_" + c_.dest()] = c_;
	_src_constraint_index[c_.dest()].push_back(&_constraints[c_.src() + "_" + c_.dest()]);
	_dest_constraint_index[c_.src()].push_back(&_constraints[c_.src() + "_" + c_.dest()]);
}

Operation Descriptor::get_operation(const string name_)
{
	auto find = _operations.find(name_);
	CHECK_NE(find, _operations.end());
	return _operations[name_];
}
Operation &Descriptor::get_mutable_operation(const string name_)
{
	auto find = _operations.find(name_);
	CHECK_NE(find, _operations.end());
	return _operations[name_];
}
Constraint Descriptor::get_constraint(const string src_, const string dest_)
{
	auto find = _constraints.find(src_ + "_" + dest_);
	CHECK_NE(find, _constraints.end());
	return _constraints[src_ + "_" + dest_];
}
Constraint &Descriptor::get_mutable_constraint(const string src_, const string dest_)
{
	auto find = _constraints.find(src_ + "_" + dest_);
	CHECK_NE(find, _constraints.end());
	return _constraints[src_ + "_" + dest_];
}

vector<Constraint *> Descriptor::get_src_constraints(const string dest_)
{
	auto find = _src_constraint_index.find(dest_);
	if (find == _src_constraint_index.end())
	{
		return {};
	}
	else
	{
		return _src_constraint_index[dest_];
	}
}
vector<Constraint *> Descriptor::get_dest_constraints(const string src_)
{
	auto find = _dest_constraint_index.find(src_);
	if (find == _dest_constraint_index.end())
	{
		return {};
	}
	else
	{
		return _dest_constraint_index[src_];
	}
}

vector<Constraint *> Descriptor::get_all_constraints()
{
	vector<Constraint *> c_list;
	;
	for (auto &c : _constraints)
	{
		c_list.push_back(&(c.second));
	}
	return c_list;
}

void Descriptor::remove_constraint(const string src_, const string dest_)
{
	auto find = _constraints.find(src_ + "_" + dest_);
	if (find != _constraints.end())
	{
		_constraints.erase(src_ + "_" + dest_);
	}
}

void Descriptor::fill_mark(map<BIR::Instruction *, string> &mark_map, string vertex_name, string mark_id)
{
	if (vertex_name == "")
	{
		vertex_name = _entry;
	}

	Operation o = get_operation(vertex_name);
	for (auto &pair : mark_map)
	{
		if (pair.first->labels[0] == o.name())
		{
			mark_map[pair.first] = mark_id;
			break;
		}
	}
	for (auto &name : o.children0)
	{
		fill_mark(mark_map, name, mark_id + "0");
	}
	for (auto &name : o.children1)
	{
		fill_mark(mark_map, name, mark_id + "1");
	}
}

void Descriptor::fill_timestamp()
{
	for (auto &e : _instr_lists)
	{
		int size_before = e.second.size();
		map<int, BIR::Instruction *> instr_map;
		vector<BIR::Instruction *> instr_vec;
		map<BIR::Instruction *, string> mark_map;
		for (auto &instr_ : e.second)
		{
			if (instr_->labels.size() <= 0)
			{
				LOG(FATAL) << "No label found in instruction (" << instr_->kindStr() << ") " << instr_->id();
			}
			else if (instr_->labels.size() == 1)
			{
				int t0 = get_operation(instr_->labels[0]).scheduled_time;
				instr_->issue_time = t0;
				instr_->arrive_time = t0;
				instr_->activation_time = t0;
				instr_->end_time = t0;
			}
			else if (instr_->labels.size() == 2)
			{
				int t0 = get_operation(instr_->labels[0]).scheduled_time;
				int t1 = get_operation(instr_->labels[1]).scheduled_time;
				instr_->issue_time = t0;
				instr_->arrive_time = t0;
				instr_->activation_time = t0;
				instr_->end_time = t1;
			}
			else if (instr_->labels.size() == 3)
			{
				int t0 = get_operation(instr_->labels[0]).scheduled_time;
				int t1 = get_operation(instr_->labels[1]).scheduled_time;
				int t2 = get_operation(instr_->labels[2]).scheduled_time;
				instr_->issue_time = t0;
				instr_->arrive_time = t0;
				instr_->activation_time = t1;
				instr_->end_time = t2;
			}
			else if (instr_->labels.size() == 4)
			{
				int t0 = get_operation(instr_->labels[0]).scheduled_time;
				int t1 = get_operation(instr_->labels[1]).scheduled_time;
				int t2 = get_operation(instr_->labels[2]).scheduled_time;
				int t3 = get_operation(instr_->labels[3]).scheduled_time;
				instr_->issue_time = t0;
				instr_->arrive_time = t1;
				instr_->activation_time = t2;
				instr_->end_time = t3;
			}
			else
			{
				LOG(FATAL) << "Too many of lables";
			}

			instr_->minScheduledClkCycle = instr_->issue_time;
			instr_->maxScheduledClkCycle = instr_->end_time;
			if (instr_->kind() == bktRefiInstruction)
			{
				BIR::RefiInstruction *refi_instr = static_cast<BIR::RefiInstruction *>(instr_);
				refi_instr->initialDelay.value = refi_instr->activation_time - refi_instr->arrive_time;
				refi_instr->initialDelay.isStatic = true;
			}
			instr_map[instr_->issue_time] = instr_;
			instr_vec.push_back(instr_);
			mark_map[instr_] = "-1";
		}
		std::sort(instr_vec.begin(), instr_vec.end(),
							[](const BIR::Instruction *a, const BIR::Instruction *b) {
								return a->issue_time < b->issue_time;
							});
		fill_mark(mark_map);

		for (int i = instr_vec.size() - 1; i >= 0; i--)
		{
			BIR::Instruction *instr = instr_vec[i];
			if (instr->kind() == BIR::BIREnumerations::bktBranchInstruction)
			{
				BIR::BranchInstruction *branch_instr = static_cast<BIR::BranchInstruction *>(instr);
				BIR::JumpInstruction *jump_instr = static_cast<BIR::JumpInstruction *>(branch_instr->falseAddressInstruction);
				BIR::WaitInstruction *wait_instr = static_cast<BIR::WaitInstruction *>(jump_instr->jumpAddressInstruction);
				CHECK(branch_instr);
				CHECK(jump_instr);
				CHECK(wait_instr);
				string mark_0 = mark_map[branch_instr] + "0"; // prefix of then part
				string mark_1 = mark_map[branch_instr] + "1"; // prefix of else part
				int j = i;
				for (; j < instr_vec.size(); j++)
				{

					if (instr_vec[j] == wait_instr)
					{
						break;
					}
				}
				vector<BIR::Instruction *> instr_vec_tmp;
				for (int k = i + 1; k < j; k++)
				{
					if (strncmp(mark_map[instr_vec[k]].c_str(), mark_0.c_str(), strlen(mark_0.c_str())) == 0)
					{
						instr_vec_tmp.push_back(instr_vec[k]);
					}
				}
				for (int k = i + 1; k < j; k++)
				{
					if (strncmp(mark_map[instr_vec[k]].c_str(), mark_1.c_str(), strlen(mark_1.c_str())) == 0)
					{
						instr_vec_tmp.push_back(instr_vec[k]);
					}
				}
				CHECK_EQ(instr_vec_tmp.size(), j - i - 1);
				for (int k = 0; k < j - i - 1; k++)
				{
					instr_vec[k + i + 1] = instr_vec_tmp[k];
				}
			}
		}
		e.second = instr_vec;

		// int size_after = instr_map.size();
		// CHECK_EQ(size_before, size_after)
		// 		<< "Conflict scheduling time. Sorted list size doesn't match it unsorted size!"
		// 		<< size_before << "!=" << size_after;
		// std::sort(e.second.begin(), e.second.end(),
		// 					[](const BIR::Instruction *a, const BIR::Instruction *b) {
		// 						return a->issue_time < b->issue_time;
		// 					});
	}

	for (auto &e : _instr_lists)
	{
		for (auto &instr : e.second)
		{
			if (instr->kind() == BIR::BIREnumerations::bktRefiInstruction)
			{
				RefiInstruction *refi_instr = static_cast<RefiInstruction *>(instr);
				if (refi_instr->corresponding_looph_l1 && refi_instr->corresponding_loopt_l1)
				{
					refi_instr->middleDelay.isStatic = true;
					refi_instr->middleDelay.value = refi_instr->corresponding_loopt_l1->issue_time - refi_instr->corresponding_looph_l1->issue_time;
				}
				if (refi_instr->corresponding_looph_l2 && refi_instr->corresponding_loopt_l2)
				{
					refi_instr->repetitionDelay.isStatic = true;
					refi_instr->repetitionDelay.value = refi_instr->corresponding_loopt_l2->issue_time - refi_instr->corresponding_looph_l2->issue_time - ((refi_instr->middleDelay.value + 1) * refi_instr->numberOfAddress.value) + 1;
				}
			}
		}
	}
}

string Descriptor::generate_dot_graph_for_operation(string operation_, std::set<string> &operation_set_)
{
	string str = "";
	Operation o = get_operation(operation_);
	if (o.children0.size() == 0 && o.children1.size() == 0)
	{
		str += o.name() + "[shape=box, color=blue, label=\"" + o.name() + "\"];\n";
	}
	else
	{
		str += o.name() + "[shape=box, peripheries=2, color=lightblue, label=\"" + o.name() + "\"];\n";
		if (o.children0.size() > 0)
		{
			str += "subgraph cluster_" + o.name() + "_0 {\n";
			str += "cluster_" + o.name() + "_0" + "_entry [shape=diamond, style=filled, fillcolor=yellow, label=0];\n";
			for (int j = 0; j < o.children0.size(); j++)
			{
				operation_set_.insert(o.children0[j]);
				str += generate_dot_graph_for_operation(o.children0[j], operation_set_);
			}
			str += "}\n";
		}
		if (o.children1.size() > 0)
		{
			str += "subgraph cluster_" + o.name() + "_1 {\n";
			str += "cluster_" + o.name() + "_1" + "_entry [shape=diamond, style=filled, fillcolor=yellow, label=1];\n";
			for (int j = 0; j < o.children1.size(); j++)
			{
				operation_set_.insert(o.children1[j]);
				str += generate_dot_graph_for_operation(o.children1[j], operation_set_);
			}
			str += "}\n";
		}

		if (o.children0.size() > 0)
		{
			str += o.name() + "->" + "cluster_" + o.name() + "_0" + "_entry" + "[color=lightblue];\n";
		}
		if (o.children1.size() > 0)
		{
			str += o.name() + "->" + "cluster_" + o.name() + "_1" + "_entry" + "[color=lightblue];\n";
		}
	}
	return str;
}

string Descriptor::generate_dot_graph()
{
	std::set<string> operation_set;
	string str = "digraph g {\n";
	for (auto &o : _operations)
	{
		if (o.second.name() == _entry)
		{
			operation_set.insert(o.second.name());
			str += generate_dot_graph_for_operation(o.second.name(), operation_set);
		}
	}

	for (auto &c : _constraints)
	{
		if (operation_set.find(c.second.src()) != operation_set.end() && operation_set.find(c.second.dest()) != operation_set.end())
		{
			str += c.second.src() + " -> " + c.second.dest() + "[";
			string hi = "+INF";
			string lo = "-INF";
			if (c.second.d_lo > INT_MIN)
			{
				lo = to_string(c.second.d_lo);
			}
			if (c.second.d_hi < INT_MAX)
			{
				hi = to_string(c.second.d_hi);
			}
			str += "label=\"[" + lo + "," + hi + "]\"";
			str += ", style=dashed, color=red";
			str += "];\n";
		}
	}

	str += "}";

	return str;
}

string Descriptor::generate_schedule_table()
{
	string str = "";
	for (auto &il : _instr_lists)
	{
		str += "CELL " + il.first + "\n";
		str += "-------------------------------------------------------------------\n";
		for (int i = 0; i < il.second.size(); i++)
		{
			str += to_string(i) + "\t:\t" + il.second[i]->kindStr() + "\t\t" + to_string(il.second[i]->issue_time) + "\t\t" + to_string(il.second[i]->arrive_time) + "\t\t" + to_string(il.second[i]->activation_time) + "\t\t" + to_string(il.second[i]->end_time) + "\n";
		}
		str += "\n";
	}
	return str;
}

vector<string> Descriptor::get_all_operation_names()
{
	vector<string> vec;
	for (auto &o : _operations)
	{
		vec.push_back(o.first);
	}
	return vec;
}

} // namespace schedule
} // namespace vesyla
