#!/usr/bin/env bash

# Get current location
this_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
curr_dir="$( pwd )"

# Read arguments
OPTIND=1         # Reset in case getopts has been used previously in the shell.
fabric_path=""
compile_flag=0
while getopts "h?cf:m:" opt; do
	case "$opt" in
	h|\?)
		echo ""
		echo "Vesyla Autotest"
		echo "===================="
		echo ""
		echo "-f <path to fabric>		Specify the path to fabric description"
		echo "-m <path to matlab library>	Specify the path to matlab primitive function library"
		echo "-c				Compile vesyla before test"
		exit 0
		;;
	c) compile_flag=1
		;;
	f) fabric_path=$OPTARG
		;;
	m) matlab_path=$OPTARG
		;;
	esac
done
shift $((OPTIND-1))
[ "$1" = "--" ] && shift
test_file_list=($@)
for i in $(seq 1 ${#test_file_list[@]})
do
	if [[ ${test_file_list[$i-1]} != /* && ${test_file_list[$i-1]} != ~* ]]; then
		test_file_list[$i-1]=$curr_dir/${test_file_list[$i-1]}
	fi
done

if [ -z $fabric_path ]; then
	echo "[Warning] Fabric path is not specified! Use default path \"../../fabric\" instead."
	fabric_path="$this_dir/../../fabric"
fi
if [[ $fabric_path != /* && $fabric_path != ~* ]]; then
		fabric_path=$curr_dir/$fabric_path
fi

if [ -z $matlab_lib_path ]; then
	echo "[Warning] Matlab library path is not specified! Use default path \"../matlab_lib\" instead."
	matlab_lib_path="$this_dir/../matlab_lib"
fi
if [[ $matlab_lib_path != /* && $matlab_lib_path != ~* ]]; then
		matlab_lib_path=$curr_dir/$matlab_lib_path
fi

if [ ${#test_file_list[@]} == 0 ]; then
	echo "[Error] No input file specified."
	exit -1
fi

# Compile Vesyla
build_dir=$this_dir/../build
if [ $compile_flag == 1 ]; then
	echo "Compile Vesyla ..."
	mkdir -p $build_dir
	cd $build_dir
	cmake .. &> /dev/null || { echo 'FAIL: CMAKE failed!!!' ; exit 1; }
	make &> /dev/null || { echo 'FAIL: Compile vesyla failed!!!' ; exit 1; }
	echo "Done!"
fi

## Prepare test directory
echo "Prepare test directory ..."
testcases=()
for testfile in "${test_file_list[@]}"
do
	xpath=${testfile%/*} 
	xbase=${testfile##*/}
	xfext=${xbase##*.}
	xpref=${xbase%.*}
	testcases+=($xpref)
done

#test_root=$this_dir/../test
#test_testbenches=$test_root/testbenches
#test_testcases=$test_root/testcases
#test_testutil=$test_root/test_util
autotest_dir=$curr_dir
work_dir=$autotest_dir/work
mkdir -p $work_dir
for testcase in "${testcases[@]}"
do
	mkdir -p $work_dir/$testcase
	mkdir -p $work_dir/$testcase/compile
	mkdir -p $work_dir/$testcase/sim_matlab
	mkdir -p $work_dir/$testcase/sim_vsim
done
echo "Done!"

# Compile test cases
echo "Compile test cases ..."
VESYLA=$build_dir/vesyla
for i in $(seq 1 ${#test_file_list[*]} ); do
	cd $work_dir/${testcases[$i-1]}/compile ||{ echo 'FAIL: Compile failed!!!' ; exit 1; }
	cp ${test_file_list[$i-1]} ./ ||{ echo 'FAIL: Compile failed!!!' ; exit 1; }
	$VESYLA ${test_file_list[$i-1]} &> /dev/null ||{ echo 'FAIL: Compile failed!!!' ; exit 1; }
done
echo "Done!"


# Prepare test environment
echo "Prepare test environment ..."
final_results_dir=$work_dir/0.final_results.0
mkdir -p $final_results_dir
test_testutil=$this_dir/../test/test_util
for testcase in ${testcases[*]}
do
	cd $work_dir/$testcase/sim_matlab
	cp -r $work_dir/$testcase/compile/filegen/sim_matlab/* ./
	mkdir -p results
	mkdir -p reports
	cd $work_dir/$testcase/sim_vsim
	cp $work_dir/$testcase/compile/filegen/sim_vsim/* ./
	mkdir -p results
	mkdir -p reports
done
echo "Done!"

# Run matlab
echo "Run matlab ..."
for testcase in ${testcases[*]}
do
	cd $work_dir/$testcase/sim_matlab
	if hash matlab 2>/dev/null; then
		echo "addpath('$matlab_lib_path')" | cat - instrumented_code.m > instrumented_code_exit.m
		echo "exit;" >> instrumented_code_exit.m
		matlab -nosplash -nodesktop -r "try, instrumented_code_exit; end, quit" &> /dev/null|| { echo 'Matlab simulation failed!!!' ; exit 1; }
	else
		echo "addpath('$matlab_lib_path')" | cat - instrumented_code.m > instrumented_code_exit.m
		echo "exit;" >> instrumented_code_exit.m
		octave -W instrumented_code.m &> /dev/null|| { echo 'Octave simulation failed!!!' ; exit 1; }
	fi
	mv mt_*.txt results/
done
echo "Done!"

# Run questasim
echo "Run questasim ..."
for testcase in ${testcases[*]}
do
	cd $work_dir/$testcase/sim_vsim
	touch run_cmd1.do
	touch run_gui1.do
	echo "set comperror \"\"" >> run_cmd1.do
	echo "set comperror \"\"" >> run_gui1.do
	echo "catch {" >> run_cmd1.do
	echo "catch {" >> run_gui1.do
	echo ".main clear" >> run_gui1.do
	echo "" >> run_gui1.do
	echo "if [file exists work] {" >> run_cmd1.do
	echo "if [file exists work] {" >> run_gui1.do
	echo "	vdel -all" >> run_cmd1.do
	echo "	vdel -all" >> run_gui1.do
	echo "}" >> run_cmd1.do
	echo "}" >> run_gui1.do
	echo "vlib work" >> run_cmd1.do
	echo "vlib work" >> run_gui1.do
	echo "" >> run_cmd1.do
	echo "" >> run_gui1.do
	echo "vcom \"$fabric_path/mtrf/util_package.vhd\"" >> run_cmd1.do
	echo "vcom \"$fabric_path/mtrf/util_package.vhd\"" >> run_gui1.do
	echo "vcom \"$fabric_path/hw_setting.vhd\"" >> run_cmd1.do
	echo "vcom \"$fabric_path/hw_setting.vhd\"" >> run_gui1.do
	echo "vcom -mixedsvvh \"$fabric_path/mtrf/top_consts_types_package.vhd\"" >> run_cmd1.do
	echo "vcom -mixedsvvh \"$fabric_path/mtrf/top_consts_types_package.vhd\"" >> run_gui1.do
	echo "vcom -F \"$fabric_path/fabric_files.f\"" >> run_cmd1.do
	echo "vcom -F \"$fabric_path/fabric_files.f\"" >> run_gui1.do
	echo "vlog -F test_util_files.f" >> run_cmd1.do
	echo "vlog -F test_util_files.f" >> run_gui1.do
	cat run_cmd.do >> run_cmd1.do
	cat run_gui.do >> run_gui1.do
	echo "}" >> run_cmd1.do
	echo "}" >> run_gui1.do
	echo "" >> run_cmd1.do
	echo "" >> run_gui1.do
	echo "if [expr  {\${comperror}!=\"\"}] then {" >> run_cmd1.do
	echo "if [expr  {\${comperror}!=\"\"}] then {" >> run_gui1.do
	echo "	echo \${comperror}" >> run_cmd1.do
	echo "	echo \${comperror}" >> run_gui1.do
	echo "  quit" >> run_cmd1.do
	echo "  quit" >> run_gui1.do
	echo "}" >> run_cmd1.do
	echo "}" >> run_gui1.do
	mv run_cmd1.do run_cmd.do
	mv run_gui1.do run_gui.do
	
	vsim -c -do run_cmd.do || { echo 'QuestaSim simulation Failed!!!' ; exit 1; }
	cp results/sv_results1_exc_$testcase.txt $final_results_dir/
done
echo "Done!"

# Compare results
echo "Compare results ..."
COMPARE=$build_dir/vesyla_verify
for testcase in ${testcases[*]}
do
	cd $work_dir/$testcase
	echo "Verify test case: $testcase"
	$COMPARE -m sim_matlab/results/mt_results.txt -f sim_vsim/results/sv_results.txt || { echo 'FAIL: Verify Failed!!!' ; exit 1; }
done
echo "Done!"

