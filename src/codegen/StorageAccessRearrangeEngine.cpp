#include "StorageAccessRearrangeEngine.hpp"

namespace vesyla
{
namespace codegen
{

void StorageAccessRearrangeEngine::traverse(cidfg::CidfgGraph &g_, vector<vector<int>> vertices_, string loop_signature_, std::unordered_map<int, string> &refi2loops_, std::unordered_map<string, vector<int>> &loops_, std::set<int> &conflict_refis_)
{
	for (int i = 0; i < vertices_.size(); i++)
	{
		for (int j = 0; j < vertices_[i].size(); j++)
		{
			cidfg::Vertex *v = g_.get_vertex(vertices_[i][j]);
			if (v->vertex_type == cidfg::Vertex::HIERARCHICAL_INSTR_VERTEX)
			{
				string new_loop_signature = loop_signature_;

				// get its in0 and out0
				int in_edge_0 = g_.get_in_edge(v->id, 0);
				int out_edge_0 = g_.get_out_edge(v->id, 0);
				if (in_edge_0 >= 0 && out_edge_0 >= 0)
				{
					int in_v_0 = g_.get_edge(in_edge_0)->src_id;
					int out_v_0 = g_.get_edge(out_edge_0)->dest_id;
					if (in_v_0 >= 0 && out_v_0 >= 0)
					{
						cidfg::Vertex *vh = g_.get_vertex(in_v_0);
						cidfg::Vertex *vt = g_.get_vertex(out_v_0);
						if (vh->vertex_type == cidfg::Vertex::LOOPH_INSTR_VERTEX && vt->vertex_type == cidfg::Vertex::LOOPT_INSTR_VERTEX)
						{
							new_loop_signature = to_string(vh->id) + "_" + to_string(v->id) + "_" + to_string(vt->id);
							loops_[new_loop_signature] = {vh->id, v->id, vt->id};
						}
					}
				}
				traverse(g_, static_cast<cidfg::HierarchicalVertex *>(v)->children, new_loop_signature, refi2loops_, loops_, conflict_refis_);
			}
			else if (v->vertex_type == cidfg::Vertex::REFI_INSTR_VERTEX)
			{
				BIR::RefiInstruction *instr = static_cast<BIR::RefiInstruction *>(static_cast<cidfg::RefiInstrVertex *>(v)->instr);
				bool conflict = false;
				for (auto &rf : refi2loops_)
				{
					if (rf.second == loop_signature_)
					{
						if (static_cast<cidfg::RefiInstrVertex *>(g_.get_vertex(rf.first))->coord == static_cast<cidfg::RefiInstrVertex *>(v)->coord &&
								static_cast<BIR::RefiInstruction *>(static_cast<cidfg::RefiInstrVertex *>(g_.get_vertex(rf.first))->instr)->portNo == instr->portNo)
						{
							LOG(DEBUG) << rf.first << ", " << v->id;
							LOG(DEBUG) << static_cast<BIR::RefiInstruction *>(static_cast<cidfg::RefiInstrVertex *>(g_.get_vertex(rf.first))->instr)->portNo << ", " << instr->portNo;
							conflict = true;
							conflict_refis_.insert(rf.first);
							conflict_refis_.insert(v->id);
						}
					}
				}
				if (loop_signature_ == "")
				{
					conflict_refis_.insert(v->id);
				}

				if (get_affine_addr_level(instr) == 2)
				{
					conflict_refis_.insert(v->id);
				}

				refi2loops_[v->id] = loop_signature_;
			}
		}
	}
}

void StorageAccessRearrangeEngine::transform(cidfg::CidfgGraph &g_)
{
	bool flag_changed = true;
	while (flag_changed)
	{
		flag_changed = false;
		std::unordered_map<int, string> refi2loops;
		std::unordered_map<string, vector<int>> loops;
		std::set<int> conflict_refis;
		for (auto &v : g_.get_vertices())
		{
			if (v->vertex_type == cidfg::Vertex::ROOT_VERTEX)
			{
				traverse(g_, static_cast<cidfg::HierarchicalVertex *>(v)->children, "", refi2loops, loops, conflict_refis);
			}
		}

		for (auto &s : conflict_refis)
		{
			LOG(DEBUG) << s << "  " << static_cast<BIR::RefiInstruction *>(static_cast<cidfg::RefiInstrVertex *>(g_.get_vertex(s))->instr)->portNo;
		}

		queue<int> refi_queue;
		for (auto &r : refi2loops)
		{

			if (conflict_refis.find(r.first) == conflict_refis.end())
			{
				LOG(DEBUG) << "REFI " << r.first << " IN LOOP  " << r.second;
				refi_queue.push(r.first);
			}
		}
		while (!refi_queue.empty())
		{
			int refi_id = refi_queue.front();
			int lh_id = loops[refi2loops[refi_id]][0];
			int lb_id = loops[refi2loops[refi_id]][1];
			int lt_id = loops[refi2loops[refi_id]][2];
			if (move_refi(g_, refi_id, lh_id, lb_id, lt_id))
			{
				flag_changed = true;
			}
			refi_queue.pop();
		}
	}
}

int StorageAccessRearrangeEngine::get_affine_addr_level(BIR::RefiInstruction *instr_)
{
	bool l1_start_addr_static = instr_->startAddress.isStatic;
	bool l1_step_val_static = instr_->stepValue.isStatic;
	bool l1_iteration_static = instr_->numberOfAddress.isStatic;
	bool l2_iteration_static = instr_->numberOfAddress.isStatic;
	int l1_iteration_num = instr_->numberOfAddress.value;
	int l2_iteration_num = instr_->numberOfRepetition.value;

	if ((l2_iteration_static == true) && (l2_iteration_num == 0) && (l1_iteration_static == true) && (l1_iteration_num == 0) && (l1_start_addr_static == true) && (l1_step_val_static == true))
	{
		return 0;
	}

	if (l2_iteration_static == true && l2_iteration_num == 0 && l1_iteration_static == true && l1_start_addr_static == true && l1_step_val_static == true)
	{
		return 1;
	}

	return 2;
}

bool StorageAccessRearrangeEngine::move_refi(cidfg::CidfgGraph &g_, int refi_, int lh_, int lb_, int lt_)
{
	cidfg::Vertex *v;
	v = g_.get_vertex(refi_);
	CHECK_EQ(v->vertex_type, cidfg::Vertex::REFI_INSTR_VERTEX);
	cidfg::RefiInstrVertex *refi = static_cast<cidfg::RefiInstrVertex *>(v);
	BIR::RefiInstruction *instr = static_cast<BIR::RefiInstruction *>(refi->instr);
	int level = get_affine_addr_level(instr);
	if (level >= 2)
	{
		return false;
	}

	v = g_.get_vertex(lh_);
	CHECK_EQ(v->vertex_type, cidfg::Vertex::LOOPH_INSTR_VERTEX);
	cidfg::LoophInstrVertex *lh = static_cast<cidfg::LoophInstrVertex *>(v);
	v = g_.get_vertex(lb_);
	CHECK_EQ(v->vertex_type, cidfg::Vertex::HIERARCHICAL_INSTR_VERTEX);
	cidfg::HierarchicalInstrVertex *lb = static_cast<cidfg::HierarchicalInstrVertex *>(v);
	v = g_.get_vertex(lt_);
	CHECK_EQ(v->vertex_type, cidfg::Vertex::LOOPT_INSTR_VERTEX);
	cidfg::LooptInstrVertex *lt = static_cast<cidfg::LooptInstrVertex *>(v);

	if (!static_cast<BIR::LoopHeaderInstruction *>(lh->instr)->isStatic)
	{
		return false;
	}

	int loop_start = static_cast<BIR::LoopHeaderInstruction *>(lh->instr)->startValue;
	int loop_step = static_cast<BIR::LoopTailInstruction *>(lt->instr)->stepValue;
	int loop_iteration = static_cast<BIR::LoopHeaderInstruction *>(lh->instr)->iterationNumber;

	if (level == 1)
	{
		refi->l2_linker.is_valid = true;
		refi->l2_linker.lh = lh_;
		refi->l2_linker.lb = lb_;
		refi->l2_linker.lt = lt_;
		instr->repetitionOffset = 0;
		instr->numberOfRepetition.value = loop_iteration - 1;
		return true;
	}

	if (level == 0)
	{
		refi->l1_linker.is_valid = true;
		refi->l1_linker.lh = lh_;
		refi->l1_linker.lb = lb_;
		refi->l1_linker.lt = lt_;
		instr->stepValue.value = 0;
		instr->numberOfAddress.value = loop_iteration - 1;
		return true;
	}

	return false;
}

} // namespace codegen
} // namespace vesyla