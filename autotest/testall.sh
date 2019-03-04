#!/bin/sh

CURR_DIR="$( pwd )"
THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

#TESTCASE_DIR="../test/testcases"
#TESTCASE_DIR="testcases"
TESTCASE_DIR="testcases/RegisterTransfer"
PATTERN="test*.m"
OUTPUT_FILE="autotest_conf.robot"
IGNORE_FILE=""

MAX_EXEC_TIME="300s"

echo "*** Settings ***" > ${OUTPUT_FILE}
echo "Library           Process" >> ${OUTPUT_FILE}
echo "Library           OperatingSystem" >> ${OUTPUT_FILE}
echo "Suite Teardown    Terminate All Processes    kill=True" >> ${OUTPUT_FILE}
echo "Test Template     Autotest Template" >> ${OUTPUT_FILE}
echo "" >> ${OUTPUT_FILE}
echo "*** Test Cases ***  filename" >> ${OUTPUT_FILE}

for folder in `find ${TESTCASE_DIR} -maxdepth 1 -type d`; do
	if [[ -d $folder ]]; then
		for file in `find ${folder}  -maxdepth 1 -name ${PATTERN}`;
		do
			if [[ ${file} != /* && ${file} != ~* ]]; then
				abs_file=${CURR_DIR}/${file}
			else
				abs_file=${file}
			fi
			filename="${file##*/}"
			echo "tc ${filename}    ${abs_file}" >> ${OUTPUT_FILE}
			echo "    [Tags]     ${folder}" >> ${OUTPUT_FILE}
		done
	fi
done

echo "" >> ${OUTPUT_FILE}
echo "*** Keywords ***" >> ${OUTPUT_FILE}
echo "Autotest Template" >> ${OUTPUT_FILE}
echo "    [Arguments]  \${filename}" >> ${OUTPUT_FILE}
echo "    \${result} =    Run Process    ./run.sh  -c  -f  ../../fabric  \${filename}    shell=True    timeout=${MAX_EXEC_TIME}" >> ${OUTPUT_FILE};
echo "    Should Not Contain    \${result.stdout}    FAIL" >> ${OUTPUT_FILE}
echo "    Should Contain    \${result.stdout}    PASS" >> ${OUTPUT_FILE}
echo "    Should Be Equal As Integers    \${result.rc}    0" >> ${OUTPUT_FILE}

# run auto test
robot autotest_conf.robot


