#ifndef __VESYLA_CODEGEN_STORAGE_ACCESS_REARRANGE_ENGINE_HPP__
#define __VESYLA_CODEGEN_STORAGE_ACCESS_REARRANGE_ENGINE_HPP__

#include "Engine.hpp"
#include <queue>

namespace vesyla
{
namespace codegen
{
class StorageAccessRearrangeEngine : Engine
{
public:
	StorageAccessRearrangeEngine() = default;
	~StorageAccessRearrangeEngine() = default;
	void transform(cidfg::CidfgGraph &g_);

private:
	int get_affine_addr_level(BIR::RefiInstruction *instr_);
	bool move_refi(cidfg::CidfgGraph &g_, int instr_, int lh_, int lb_, int lt_);
	void traverse(cidfg::CidfgGraph &g_, vector<vector<int>> vertices_, string loop_signature_, std::unordered_map<int, string> &refi2loops_, std::unordered_map<string, vector<int>> &loops_, std::set<int> &conflict_refis_);
};
} // namespace codegen
} // namespace vesyla

#endif // __VESYLA_CODEGEN_STORAGE_ACCESS_REARRANGE_ENGINE_HPP__