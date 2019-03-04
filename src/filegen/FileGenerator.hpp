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

#ifndef __VESYLA_FILEGEN_FILE_GENERATOR_HPP__
#define __VESYLA_FILEGEN_FILE_GENERATOR_HPP__

#include "BIR/BIRIncludeAll.hpp"
#include "ManasGenenator.hpp"
#include "SramImageGenerator.hpp"
#include "MatlabGenerator.hpp"
#include "SyncInstrGenerator.hpp"
#include "RtlGenerator.hpp"
#include "LayoutGenerator.hpp"

using namespace BIR;

namespace vesyla
{
namespace filegen
{
class FileGenerator
{
private:
public:
	FileGenerator() {}
	~FileGenerator() {}
	/**
	 * This function generate all the files as the output of the compiler.
	 * It is a wrap function for all generators belongs to vesyla::filegen.
	 */
	void generate(VIR::MainProgram *main_program_, map<string, vector<BIR::Instruction *>> instr_lists_, codegen::StorageImage si_);
};
} // namespace filegen
} // namespace vesyla

#endif // __VESYLA_FILEGEN_FILE_GENERATEOR_HPP__