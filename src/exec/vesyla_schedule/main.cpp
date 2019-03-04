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

#include "main.hpp"

using namespace boost::program_options;
using namespace vesyla;

// Initialize logging system
INITIALIZE_EASYLOGGINGPP

int main(int argc, char *argv[])
{
	string path_output;
	vector<string> file_list;

	options_description desc("Options");
	desc.add_options()("help,h", "Print help messages")("output,o", value<string>(&path_output)->default_value("."), "Output directory");

	variables_map vm;
	try
	{
		parsed_options parsed = parse_command_line(argc, argv, desc);
		store(parsed, vm);
		if (vm.count("help"))
		{
			cout << "Vesyla_schedule" << endl
					 << "=============================================" << endl
					 << "Usage:" << endl
					 << "  ./vesyla [options] [files]" << endl
					 << endl
					 << "=============================================" << endl
					 << desc << endl
					 << endl;
			return (0);
		}
		file_list = collect_unrecognized(parsed.options, include_positional);
		notify(vm);
	}
	catch (error &e)
	{
		LOG(ERROR) << e.what();
		LOG(ERROR) << desc << endl;
		return (-1);
	}

	if (file_list.empty())
	{
		LOG(FATAL) << "No source file specified!";
	}

	boost::filesystem::path curr_full_path = boost::filesystem::system_complete(path_output);
	path_output = curr_full_path.string();
	if (path_output[-1] != '/')
	{
		path_output += "/";
	}
	boost::filesystem::create_directory(path_output);
	util::GlobalVar glv;
	glv.puts("$OUTPUT_DIR", path_output);

	util::SysPath sys_path;
	string config_path = sys_path.find_config_file("hardware_def.xml");
	CHECK_NE(config_path, "");
	util::Config config(config_path);

	for (auto i = 0; i < file_list.size(); i++)
	{
		LOG(INFO) << "Compile file " << file_list[i] << ": ";

		boost::filesystem::path p(file_list[i]);
		glv.puts("input_file_full_name", file_list[i]);
		glv.puts("input_file_name", p.stem().string());

		// Read xml
		vesyla::util::GlobalVar glv;
		string path;
		CHECK(glv.gets("$CURR_DIR", path));
		path = path + "schedule/";
		mkdir(path.c_str(), 0755);
		string input_file_name;
		CHECK(glv.gets("input_file_name", input_file_name));
		schedule::Descriptor d;
		pt::ptree tree;
		pt::read_xml(file_list[i], tree);
		d.load(tree);
		pt::write_xml(path + input_file_name + "_preschedule.xml", d.dump());
		vesyla::schedule::Scheduler s(d);
		s.schedule();
		pt::write_xml(path + input_file_name + "_postschedule.xml", s.desc().dump());

		// synchronization
		map<string, vector<BIR::Instruction *>> instr_lists = s.desc().get_instr_lists();
		sync::Synchronizer sy;
		instr_lists = sy.synchronize(instr_lists, s.get_end_time());

		/***************************************************************************
		 * FILE GENERATION
		 **************************************************************************/
		//filegen::FileGenerator fg;
		//fg.generate(instr_lists, esi);
	}
	return 0;
}
