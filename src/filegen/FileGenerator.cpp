#include "FileGenerator.hpp"

namespace vesyla
{
namespace filegen
{
void FileGenerator::generate(DRRACluster *drra_cluster_)
{
	vesyla::util::GlobalVar glv;
	string path;
	CHECK(glv.gets("$OUTPUT_DIR", path));
	path = path + "filegen/";
	mkdir(path.c_str(), 0755);
	string input_file_name;
	CHECK(glv.gets("input_file_name", input_file_name));

	// Manas
	map<string, vector<Instruction *>> instr_lists;
	for (int rowIndex = 0; rowIndex < drra_cluster_->rowNum(); ++rowIndex)
	{
		for (int colIndex = 0; colIndex < drra_cluster_->colNum(); ++colIndex)
		{
			vector<Instruction *> &instr_list = drra_cluster_->getDRRACell(rowIndex, colIndex)->instr_list;
			string coord_signature = to_string(rowIndex) + "_" + to_string(colIndex);
			instr_lists[coord_signature] = instr_list;
		}
	}
	string path_manas = path + input_file_name + ".manas";
	vesyla::filegen::ManasGenerator gm;
	gm.generate(instr_lists, path_manas);

	// SRAM
	map<string, SRAM *> sram_lists;
	for (int rowIndex = 0; rowIndex < drra_cluster_->diMArch()->rowNum(); ++rowIndex)
	{
		for (int colIndex = 0; colIndex < drra_cluster_->diMArch()->colNum(); ++colIndex)
		{
			SRAM *sram = drra_cluster_->diMArch()->getDiMArchCell(rowIndex, colIndex)->sram();
			string coord_signature = to_string(rowIndex) + "_" + to_string(colIndex);
			sram_lists[coord_signature] = sram;
		}
	}
	string path_sramimg = path + input_file_name + ".sram";
	vesyla::filegen::SramImageGenerator gs;
	gs.generate(sram_lists, path_sramimg);

	// Matlab
	string path_matlab = path + "sim_matlab/";
	mkdir(path_matlab.c_str(), 0755);
	vesyla::filegen::MatlabGenerator gmat;
	gmat.generate(path_matlab);
}
} // namespace filegen
} // namespace vesyla