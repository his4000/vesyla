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