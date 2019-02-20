#include "main.hpp"

using namespace std;
using namespace boost::program_options;
using namespace VIR;
using namespace VIR::VirEnumerations;
using namespace vesyla;

// Initialize logging system
INITIALIZE_EASYLOGGINGPP

int main(int argc, char **argv)
{
	// Get start time
	auto start_time = std::chrono::high_resolution_clock::now();

	/*****************************************************************************
	 * ENVIRONMENT SETTING
	 ****************************************************************************/
	// Set Logger
	el::Configurations c;
	c.setToDefault();
	c.parseFromText(LOGGING_CONF);
	el::Loggers::reconfigureLogger("default", c);
	el::Loggers::addFlag(el::LoggingFlag::ColoredTerminalOutput);

	// Argument parsing
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
			cout << endl
					 << "\e[1mVesyla (version: " << VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_PATCH << ")\e[0m" << endl
					 << endl
					 << "=================================================================================" << endl
					 << "Vesyla is distributed under \e[1m" << LICENSE_NAME << "\e[0m license agreement:" << endl
					 << LICENSE_DESC << endl
					 << "=================================================================================" << endl
					 << "Usage:" << endl
					 << "  ./vesyla [options] [files]" << endl
					 << endl
					 << desc << endl
					 << "Files:" << endl
					 << "  [0] - template" << endl
					 << "  [1] - data for instance 1" << endl
					 << "  [2] - data for instance 2" << endl
					 << "  ...." << endl
					 << endl
					 << "=================================================================================" << endl
					 << endl;
			return (0);
		}
		file_list = collect_unrecognized(parsed.options, include_positional);
		notify(vm);
	}
	catch (error &e)
	{
		LOG(ERROR) << endl;
		LOG(ERROR) << e.what() << endl;
		LOG(ERROR) << endl;
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

	/*****************************************************************************
	 * PRE-PROCESS
	 ****************************************************************************/
	// Prepare source code directory
	boost::filesystem::path p_output(path_output);
	boost::filesystem::path p_source = p_output / "source";
	if (!boost::filesystem::exists(p_source))
	{
		boost::filesystem::create_directory(p_source);
	}
	vector<string> source_file_list;
	if (file_list.size() == 1)
	{
		// Only template file, which means there is no need for pre-compile process
		// (macro replacing). Just copy the original file.
		boost::filesystem::path p2(file_list[0]);
		boost::filesystem::path p = p_source / (p2.stem().string() + ".m");
		source_file_list.push_back(p.string());
		CHECK(boost::filesystem::exists(file_list[0]))
				<< "File " << file_list[0] << " not found!";
		std::ifstream src(file_list[0], std::ios::binary);
		std::ofstream dst(p.string(), std::ios::binary);
		dst << src.rdbuf();
	}
	else
	{
		// There is a template file, and multiple data file to produce instances
		// with different degree of parallelism. Pre-compile process is needed for
		// each instance. New instance source file will be created and will be used
		// later.
		boost::filesystem::path p2(file_list[0]);
		CHECK(boost::filesystem::exists(file_list[0]))
				<< "File " << file_list[0] << " not found!";
		for (auto i = 1; i < file_list.size(); i++)
		{
			CHECK(boost::filesystem::exists(file_list[i]))
					<< "File " << file_list[i] << " not found!";
			boost::filesystem::path p = p_source / (p2.stem().string() + "_" + to_string(i) + ".m");
			source_file_list.push_back(p.string());
			std::ofstream dst(p.string(), std::ios::binary);
			inja::Environment env = inja::Environment();
			env.add_callback("add", 2, [&env](inja::Parsed::Arguments args, json data) {
				int number0 = env.get_argument<int>(args, 0, data); // Adapt the type and index of the argument
				int number1 = env.get_argument<int>(args, 1, data); // Adapt the type and index of the argument
				return number0 + number1;
			});
			env.add_callback("sub", 2, [&env](inja::Parsed::Arguments args, json data) {
				int number0 = env.get_argument<int>(args, 0, data); // Adapt the type and index of the argument
				int number1 = env.get_argument<int>(args, 1, data); // Adapt the type and index of the argument
				return number0 - number1;
			});
			env.add_callback("mul", 2, [&env](inja::Parsed::Arguments args, json data) {
				int number0 = env.get_argument<int>(args, 0, data); // Adapt the type and index of the argument
				int number1 = env.get_argument<int>(args, 1, data); // Adapt the type and index of the argument
				return number0 * number1;
			});
			env.add_callback("div", 2, [&env](inja::Parsed::Arguments args, json data) {
				int number0 = env.get_argument<int>(args, 0, data); // Adapt the type and index of the argument
				int number1 = env.get_argument<int>(args, 1, data); // Adapt the type and index of the argument
				return number0 / number1;
			});
			env.add_callback("pow", 2, [&env](inja::Parsed::Arguments args, json data) {
				int number0 = env.get_argument<int>(args, 0, data); // Adapt the type and index of the argument
				int number1 = env.get_argument<int>(args, 1, data); // Adapt the type and index of the argument
				return (int)(pow(number0, number1));
			});
			dst << env.render_file_with_json_file(file_list[0], file_list[i]);
		}
	}

	for (auto i = 0; i < source_file_list.size(); i++)
	{
		LOG(INFO) << "Compile file " << source_file_list[i] << ": ";

		boost::filesystem::path p(source_file_list[i]);
		glv.puts("input_file_full_name", source_file_list[i]);
		glv.puts("input_file_name", p.stem().string());

		/***************************************************************************
		 * PARSE
		 **************************************************************************/
		FileManager::inst()->inputFileName(source_file_list[i], true);
		SetCurrentFile(ftReport);
		LOG(INFO) << "Start parsing ...";
		WriteLine("Start parsing \"" + InputFileName() + "\" . . .");
		MainProgram *mainProgram = PARSE(source_file_list[i].c_str());
		if (!mainProgram)
			return (-1);
		WriteLine("For more information see parser log file.");
		WriteLine("Parsing finished.");
		LOG(INFO) << "Done";

		VDump dump;
		LOG(INFO) << "Dumping parser result ...";
		WriteLine("\nStart dumping \"" + InputFileName() + "\" . . .");
		dump.dumpProgram(mainProgram, false);
		WriteLine("For more information see dump log file.");
		WriteLine("Dumping finished.");
		LOG(INFO) << "Done";

		/***************************************************************************
		 * ELABRATE
		 **************************************************************************/
		LOG(INFO) << "Start elaboration ...";
		WriteLine("\nStart elaboration \"" + InputFileName() + "\" . . .");
		VElaborator elaborator;
		elaborator.setRaccuMode(false);
		mainProgram = elaborator.elaborate(mainProgram);
		WriteLine("For more information see elaborator log file.");
		WriteLine("Elaboration finished.");
		LOG(INFO) << "Done";

		LOG(INFO) << "Dumping elaboration result ...";
		WriteLine("\nStart elaboration dumping \"" + InputFileName() + "\" . . .");
		dump.dumpProgram(mainProgram, true);
		dump.dumpMakeGraph();
		WriteLine("Dumping finished.");
		LOG(INFO) << "Done";

		/***************************************************************************
		 * CIDFG conversion
		 **************************************************************************/
		LOG(INFO) << "Start AST to CIDFG conversion ...";

		// legacy code for determin variable start address and size.
		BEngine bEngine;
		bEngine.setRaccuMode(true);
		bEngine.run1(mainProgram);

		vesyla::util::GlobalVar glv;
		string path;
		// CHECK(glv.gets("$OUTPUT_DIR", path));
		// path = path + "cidfg/";
		// mkdir(path.c_str(), 0755);
		string input_file_name;
		// CHECK(glv.gets("input_file_name", input_file_name));

		// cidfg::ScriptGenerator sg;
		// string path_script;
		// path_script = path + "convert.sh";
		// ofstream ofs_script(path_script, ofstream::out);
		// ofs_script << sg.generate();
		// ofs_script.close();

		vesyla::cidfg::CidfgGraph cc;
		vesyla::cidfg::Converter cv;
		cv.convert(mainProgram, cc);
		// string path_dot;
		// path_dot = path + input_file_name + ".dot";
		// ofstream ofs0(path_dot, ofstream::out);
		// ofs0 << cc.generate_dot_graph();
		// ofs0.close();

		LOG(INFO) << "Done";

		/***************************************************************************
		 * Front-end optimization
		 **************************************************************************/
		LOG(INFO) << "Start front-end optimization ...";
		feopt::Optimizer opt;
		opt.optimize(cc);
		LOG(INFO) << "Done";

		/***************************************************************************
		 * Back-end transformation and optimization
		 **************************************************************************/
		LOG(INFO) << "Start back-end optimization ...";
		codegen::Optimizer bopt;
		schedule::Descriptor d = bopt.optimize(cc);
		LOG(INFO) << "Done";

		/***************************************************************************
		 * SCHEDULE
		 **************************************************************************/
		LOG(INFO) << "Start scheduling ...";
		CHECK(glv.gets("$OUTPUT_DIR", path));
		path = path + "schedule/";
		mkdir(path.c_str(), 0755);
		CHECK(glv.gets("input_file_name", input_file_name));
		pt::write_xml(path + input_file_name + "_preschedule.xml", d.dump());

		pt::ptree tree;
		pt::read_xml(path + input_file_name + "_preschedule.xml", tree);
		schedule::Descriptor dd;
		dd.load(tree);
		pt::write_xml(path + input_file_name + "_preschedule2.xml", dd.dump());

		vesyla::schedule::Scheduler s(d);
		s.schedule();
		pt::write_xml(path + input_file_name + "_postschedule.xml", s.desc().dump());

		string path_schedule_table = path + input_file_name + "_schedule_table.txt";
		ofstream ofs6(path_schedule_table, ofstream::out);
		ofs6 << s.desc().generate_schedule_table();
		ofs6.close();

		LOG(INFO) << "Done";

		/***************************************************************************
		 * SYNCHRONIZATION
		 **************************************************************************/
		LOG(INFO) << "Start synchronization ...";
		const int maxInitialDelay = bEngine.get_max_init_delay();
		const int maxMiddleDelay = bEngine.get_max_middle_delay();
		const int maxRepetitionNumber = bEngine.get_max_repetition_delay();
		const int maxRepetitionOffset = bEngine.get_max_repetition_step_value();
		map<string, vector<BIR::Instruction *>> instr_lists = s.desc().get_instr_lists();
		sync::Synchronizer sy(maxInitialDelay, maxMiddleDelay, maxRepetitionNumber, maxRepetitionOffset);
		instr_lists = sy.synchronize(instr_lists, s.get_end_time());
		bEngine.run2(instr_lists);
		LOG(INFO) << "Done";

		/***************************************************************************
		 * TIMING SIMULATION
		 **************************************************************************/
		LOG(INFO) << "Start timing simulation ...";
		sim::Simulator s0;
		int t = s0.simulate(instr_lists);
		LOG(INFO) << "Done";

		/***************************************************************************
		 * FILE GENERATION
		 **************************************************************************/
		LOG(INFO) << "Start file generation ...";
		WriteLine("\nStart file generation \"" + InputFileName() + "\" . . .");
		dump.dumpDRRACluster(bEngine.drraCluster());
		dump.drraCodeGeneration(bEngine.drraCluster(), mainProgram);
		dump.dump_layout(bEngine.layout());
		WriteLine("File generation finished.");
		LOG(INFO) << "Done";

		// Releasing the data structure and close all files.
		GarbageCollector;
		CLOSE_ALL_FILES;
	}

	// Get stop time
	auto stop_time = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time);

	LOG(INFO) << "Vesyla ends successfully!";
	LOG(INFO) << "Compile time: " << duration.count() << " ms";
	return 0;
}
