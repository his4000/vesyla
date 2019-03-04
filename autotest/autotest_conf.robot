*** Settings ***
Library           Process
Library           OperatingSystem
Suite Teardown    Terminate All Processes    kill=True
Test Template     Autotest Template

*** Test Cases ***  filename
tc test011_01.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/RegisterTransfer/test011_01.m
    [Tags]     testcases/RegisterTransfer
tc test011_02.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/RegisterTransfer/test011_02.m
    [Tags]     testcases/RegisterTransfer
tc test011_03.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/RegisterTransfer/test011_03.m
    [Tags]     testcases/RegisterTransfer
tc test011_04.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/RegisterTransfer/test011_04.m
    [Tags]     testcases/RegisterTransfer
tc test011_05.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/RegisterTransfer/test011_05.m
    [Tags]     testcases/RegisterTransfer
tc test011_06.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/RegisterTransfer/test011_06.m
    [Tags]     testcases/RegisterTransfer

*** Keywords ***
Autotest Template
    [Arguments]  ${filename}
    ${result} =    Run Process    ./run.sh  -c  -f  ../../fabric  ${filename}    shell=True    timeout=300s
    Should Not Contain    ${result.stdout}    FAIL
    Should Contain    ${result.stdout}    PASS
    Should Be Equal As Integers    ${result.rc}    0
