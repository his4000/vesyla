*** Settings ***
Library           Process
Library           OperatingSystem
Suite Teardown    Terminate All Processes    kill=True
Test Template     Autotest Template

*** Test Cases ***  filename
tc test018_01.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DpuChain/test018_01.m
    [Tags]     testcases/DpuChain
tc test018_02.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DpuChain/test018_02.m
    [Tags]     testcases/DpuChain
tc test018_03.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DpuChain/test018_03.m
    [Tags]     testcases/DpuChain
tc test018_04.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DpuChain/test018_04.m
    [Tags]     testcases/DpuChain
tc test018_05.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DpuChain/test018_05.m
    [Tags]     testcases/DpuChain
tc test018_07.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DpuChain/test018_07.m
    [Tags]     testcases/DpuChain
tc test018_08.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DpuChain/test018_08.m
    [Tags]     testcases/DpuChain
tc test018_09.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DpuChain/test018_09.m
    [Tags]     testcases/DpuChain
tc test018_10.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DpuChain/test018_10.m
    [Tags]     testcases/DpuChain
tc test018_11.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DpuChain/test018_11.m
    [Tags]     testcases/DpuChain
tc test018_12.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DpuChain/test018_12.m
    [Tags]     testcases/DpuChain
tc test001_01.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/General/test001_01.m
    [Tags]     testcases/General
tc test001_02.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/General/test001_02.m
    [Tags]     testcases/General
tc test001_03.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/General/test001_03.m
    [Tags]     testcases/General
tc test001_04.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/General/test001_04.m
    [Tags]     testcases/General
tc test001_05.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/General/test001_05.m
    [Tags]     testcases/General
tc test001_06.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/General/test001_06.m
    [Tags]     testcases/General
tc test001_07.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/General/test001_07.m
    [Tags]     testcases/General
tc test001_08.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/General/test001_08.m
    [Tags]     testcases/General
tc test001_09.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/General/test001_09.m
    [Tags]     testcases/General
tc test001_10.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/General/test001_10.m
    [Tags]     testcases/General
tc test005_01.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Initialization/test005_01.m
    [Tags]     testcases/Initialization
tc test005_02.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Initialization/test005_02.m
    [Tags]     testcases/Initialization
tc test005_03.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Initialization/test005_03.m
    [Tags]     testcases/Initialization
tc test005_04.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Initialization/test005_04.m
    [Tags]     testcases/Initialization
tc test005_05.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Initialization/test005_05.m
    [Tags]     testcases/Initialization
tc test006_01.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/ScalarIntegerOperation/test006_01.m
    [Tags]     testcases/ScalarIntegerOperation
tc test006_02.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/ScalarIntegerOperation/test006_02.m
    [Tags]     testcases/ScalarIntegerOperation
tc test006_03.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/ScalarIntegerOperation/test006_03.m
    [Tags]     testcases/ScalarIntegerOperation
tc test006_04.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/ScalarIntegerOperation/test006_04.m
    [Tags]     testcases/ScalarIntegerOperation
tc test006_05.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/ScalarIntegerOperation/test006_05.m
    [Tags]     testcases/ScalarIntegerOperation
tc test007_01.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/VectorIntegerOperation/test007_01.m
    [Tags]     testcases/VectorIntegerOperation
tc test007_02.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/VectorIntegerOperation/test007_02.m
    [Tags]     testcases/VectorIntegerOperation
tc test007_03.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/VectorIntegerOperation/test007_03.m
    [Tags]     testcases/VectorIntegerOperation
tc test007_04.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/VectorIntegerOperation/test007_04.m
    [Tags]     testcases/VectorIntegerOperation
tc test007_05.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/VectorIntegerOperation/test007_05.m
    [Tags]     testcases/VectorIntegerOperation
tc test007_06.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/VectorIntegerOperation/test007_06.m
    [Tags]     testcases/VectorIntegerOperation
tc test007_07.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/VectorIntegerOperation/test007_07.m
    [Tags]     testcases/VectorIntegerOperation
tc test007_08.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/VectorIntegerOperation/test007_08.m
    [Tags]     testcases/VectorIntegerOperation
tc test007_09.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/VectorIntegerOperation/test007_09.m
    [Tags]     testcases/VectorIntegerOperation
tc test007_10.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/VectorIntegerOperation/test007_10.m
    [Tags]     testcases/VectorIntegerOperation
tc test007_11.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/VectorIntegerOperation/test007_11.m
    [Tags]     testcases/VectorIntegerOperation
tc test007_12.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/VectorIntegerOperation/test007_12.m
    [Tags]     testcases/VectorIntegerOperation
tc test007_13.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/VectorIntegerOperation/test007_13.m
    [Tags]     testcases/VectorIntegerOperation
tc test007_14.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/VectorIntegerOperation/test007_14.m
    [Tags]     testcases/VectorIntegerOperation
tc test007_15.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/VectorIntegerOperation/test007_15.m
    [Tags]     testcases/VectorIntegerOperation
tc test007_16.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/VectorIntegerOperation/test007_16.m
    [Tags]     testcases/VectorIntegerOperation
tc test100_01.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test100_01.m
    [Tags]     testcases/Advanced
tc test100_02.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test100_02.m
    [Tags]     testcases/Advanced
tc test100_03.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test100_03.m
    [Tags]     testcases/Advanced
tc test101_01.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test101_01.m
    [Tags]     testcases/Advanced
tc test101_02.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test101_02.m
    [Tags]     testcases/Advanced
tc test102_01.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test102_01.m
    [Tags]     testcases/Advanced
tc test103_01.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test103_01.m
    [Tags]     testcases/Advanced
tc test103_02.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test103_02.m
    [Tags]     testcases/Advanced
tc test103_03.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test103_03.m
    [Tags]     testcases/Advanced
tc test103_04.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test103_04.m
    [Tags]     testcases/Advanced
tc test103_05.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test103_05.m
    [Tags]     testcases/Advanced
tc test103_06.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test103_06.m
    [Tags]     testcases/Advanced
tc test103_07.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test103_07.m
    [Tags]     testcases/Advanced
tc test103_08.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test103_08.m
    [Tags]     testcases/Advanced
tc test104_01.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test104_01.m
    [Tags]     testcases/Advanced
tc test104_02.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test104_02.m
    [Tags]     testcases/Advanced
tc test104_03.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test104_03.m
    [Tags]     testcases/Advanced
tc test105_01.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test105_01.m
    [Tags]     testcases/Advanced
tc test105_02.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test105_02.m
    [Tags]     testcases/Advanced
tc test105_03.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test105_03.m
    [Tags]     testcases/Advanced
tc test105_04.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test105_04.m
    [Tags]     testcases/Advanced
tc test105_05.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test105_05.m
    [Tags]     testcases/Advanced
tc test105_06.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test105_06.m
    [Tags]     testcases/Advanced
tc test106_01.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test106_01.m
    [Tags]     testcases/Advanced
tc test106_02.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test106_02.m
    [Tags]     testcases/Advanced
tc test106_03.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test106_03.m
    [Tags]     testcases/Advanced
tc test106_04.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test106_04.m
    [Tags]     testcases/Advanced
tc test106_05.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test106_05.m
    [Tags]     testcases/Advanced
tc test107_01.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test107_01.m
    [Tags]     testcases/Advanced
tc test108_01.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test108_01.m
    [Tags]     testcases/Advanced
tc test108_02.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test108_02.m
    [Tags]     testcases/Advanced
tc test108_03.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test108_03.m
    [Tags]     testcases/Advanced
tc test108_04.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test108_04.m
    [Tags]     testcases/Advanced
tc test108_05.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test108_05.m
    [Tags]     testcases/Advanced
tc test108_06.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test108_06.m
    [Tags]     testcases/Advanced
tc test109_01.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test109_01.m
    [Tags]     testcases/Advanced
tc test111_01.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test111_01.m
    [Tags]     testcases/Advanced
tc test111_02.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test111_02.m
    [Tags]     testcases/Advanced
tc test111_03.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test111_03.m
    [Tags]     testcases/Advanced
tc test111_04.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Advanced/test111_04.m
    [Tags]     testcases/Advanced
tc test009_01.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Constants/test009_01.m
    [Tags]     testcases/Constants
tc test009_02.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Constants/test009_02.m
    [Tags]     testcases/Constants
tc test009_03.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Constants/test009_03.m
    [Tags]     testcases/Constants
tc test009_04.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Constants/test009_04.m
    [Tags]     testcases/Constants
tc test009_05.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Constants/test009_05.m
    [Tags]     testcases/Constants
tc test010_01.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_01.m
    [Tags]     testcases/MemoryAccess
tc test010_02.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_02.m
    [Tags]     testcases/MemoryAccess
tc test010_03.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_03.m
    [Tags]     testcases/MemoryAccess
tc test010_04.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_04.m
    [Tags]     testcases/MemoryAccess
tc test010_05.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_05.m
    [Tags]     testcases/MemoryAccess
tc test010_06.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_06.m
    [Tags]     testcases/MemoryAccess
tc test010_07.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_07.m
    [Tags]     testcases/MemoryAccess
tc test010_08.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_08.m
    [Tags]     testcases/MemoryAccess
tc test010_09.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_09.m
    [Tags]     testcases/MemoryAccess
tc test010_10.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_10.m
    [Tags]     testcases/MemoryAccess
tc test010_11.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_11.m
    [Tags]     testcases/MemoryAccess
tc test010_12.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_12.m
    [Tags]     testcases/MemoryAccess
tc test010_13.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_13.m
    [Tags]     testcases/MemoryAccess
tc test010_14.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_14.m
    [Tags]     testcases/MemoryAccess
tc test010_15.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_15.m
    [Tags]     testcases/MemoryAccess
tc test010_16.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_16.m
    [Tags]     testcases/MemoryAccess
tc test010_17.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_17.m
    [Tags]     testcases/MemoryAccess
tc test010_18.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_18.m
    [Tags]     testcases/MemoryAccess
tc test010_19.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_19.m
    [Tags]     testcases/MemoryAccess
tc test010_20.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_20.m
    [Tags]     testcases/MemoryAccess
tc test010_21.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_21.m
    [Tags]     testcases/MemoryAccess
tc test010_22.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_22.m
    [Tags]     testcases/MemoryAccess
tc test010_23.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_23.m
    [Tags]     testcases/MemoryAccess
tc test010_24.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_24.m
    [Tags]     testcases/MemoryAccess
tc test010_25.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_25.m
    [Tags]     testcases/MemoryAccess
tc test010_26.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_26.m
    [Tags]     testcases/MemoryAccess
tc test010_27.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_27.m
    [Tags]     testcases/MemoryAccess
tc test010_28.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_28.m
    [Tags]     testcases/MemoryAccess
tc test010_29.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_29.m
    [Tags]     testcases/MemoryAccess
tc test010_30.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_30.m
    [Tags]     testcases/MemoryAccess
tc test010_31.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_31.m
    [Tags]     testcases/MemoryAccess
tc test010_32.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_32.m
    [Tags]     testcases/MemoryAccess
tc test010_33.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_33.m
    [Tags]     testcases/MemoryAccess
tc test010_34.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_34.m
    [Tags]     testcases/MemoryAccess
tc test010_35.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_35.m
    [Tags]     testcases/MemoryAccess
tc test010_36.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_36.m
    [Tags]     testcases/MemoryAccess
tc test010_37.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_37.m
    [Tags]     testcases/MemoryAccess
tc test010_38.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_38.m
    [Tags]     testcases/MemoryAccess
tc test010_39.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_39.m
    [Tags]     testcases/MemoryAccess
tc test010_40.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_40.m
    [Tags]     testcases/MemoryAccess
tc test010_41.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_41.m
    [Tags]     testcases/MemoryAccess
tc test010_42.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_42.m
    [Tags]     testcases/MemoryAccess
tc test010_43.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_43.m
    [Tags]     testcases/MemoryAccess
tc test010_44.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_44.m
    [Tags]     testcases/MemoryAccess
tc test010_45.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_45.m
    [Tags]     testcases/MemoryAccess
tc test010_46.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_46.m
    [Tags]     testcases/MemoryAccess
tc test010_47.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_47.m
    [Tags]     testcases/MemoryAccess
tc test010_48.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_48.m
    [Tags]     testcases/MemoryAccess
tc test010_49.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_49.m
    [Tags]     testcases/MemoryAccess
tc test010_50.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_50.m
    [Tags]     testcases/MemoryAccess
tc test010_51.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_51.m
    [Tags]     testcases/MemoryAccess
tc test010_52.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_52.m
    [Tags]     testcases/MemoryAccess
tc test010_53.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_53.m
    [Tags]     testcases/MemoryAccess
tc test010_54.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_54.m
    [Tags]     testcases/MemoryAccess
tc test010_55.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_55.m
    [Tags]     testcases/MemoryAccess
tc test010_56.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_56.m
    [Tags]     testcases/MemoryAccess
tc test010_57.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_57.m
    [Tags]     testcases/MemoryAccess
tc test010_58.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_58.m
    [Tags]     testcases/MemoryAccess
tc test010_59.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_59.m
    [Tags]     testcases/MemoryAccess
tc test010_60.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_60.m
    [Tags]     testcases/MemoryAccess
tc test010_61.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_61.m
    [Tags]     testcases/MemoryAccess
tc test010_62.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_62.m
    [Tags]     testcases/MemoryAccess
tc test010_63.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_63.m
    [Tags]     testcases/MemoryAccess
tc test010_64.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryAccess/test010_64.m
    [Tags]     testcases/MemoryAccess
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
tc test012_01.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/IfStatement/test012_01.m
    [Tags]     testcases/IfStatement
tc test012_02.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/IfStatement/test012_02.m
    [Tags]     testcases/IfStatement
tc test012_03.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/IfStatement/test012_03.m
    [Tags]     testcases/IfStatement
tc test012_04.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/IfStatement/test012_04.m
    [Tags]     testcases/IfStatement
tc test012_05.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/IfStatement/test012_05.m
    [Tags]     testcases/IfStatement
tc test012_06.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/IfStatement/test012_06.m
    [Tags]     testcases/IfStatement
tc test012_07.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/IfStatement/test012_07.m
    [Tags]     testcases/IfStatement
tc test012_08.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/IfStatement/test012_08.m
    [Tags]     testcases/IfStatement
tc test012_09.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/IfStatement/test012_09.m
    [Tags]     testcases/IfStatement
tc test012_10.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/IfStatement/test012_10.m
    [Tags]     testcases/IfStatement
tc test012_11.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/IfStatement/test012_11.m
    [Tags]     testcases/IfStatement
tc test012_12.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/IfStatement/test012_12.m
    [Tags]     testcases/IfStatement
tc test013_01.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/StaticLoop/test013_01.m
    [Tags]     testcases/StaticLoop
tc test013_02.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/StaticLoop/test013_02.m
    [Tags]     testcases/StaticLoop
tc test013_03.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/StaticLoop/test013_03.m
    [Tags]     testcases/StaticLoop
tc test013_04.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/StaticLoop/test013_04.m
    [Tags]     testcases/StaticLoop
tc test013_06.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/StaticLoop/test013_06.m
    [Tags]     testcases/StaticLoop
tc test013_07.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/StaticLoop/test013_07.m
    [Tags]     testcases/StaticLoop
tc test013_08.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/StaticLoop/test013_08.m
    [Tags]     testcases/StaticLoop
tc test013_09.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/StaticLoop/test013_09.m
    [Tags]     testcases/StaticLoop
tc test013_10.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/StaticLoop/test013_10.m
    [Tags]     testcases/StaticLoop
tc test013_11.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/StaticLoop/test013_11.m
    [Tags]     testcases/StaticLoop
tc test013_12.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/StaticLoop/test013_12.m
    [Tags]     testcases/StaticLoop
tc test013_13.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/StaticLoop/test013_13.m
    [Tags]     testcases/StaticLoop
tc test013_14.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/StaticLoop/test013_14.m
    [Tags]     testcases/StaticLoop
tc test013_15.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/StaticLoop/test013_15.m
    [Tags]     testcases/StaticLoop
tc test013_16.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/StaticLoop/test013_16.m
    [Tags]     testcases/StaticLoop
tc test013_17.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/StaticLoop/test013_17.m
    [Tags]     testcases/StaticLoop
tc test013_18.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/StaticLoop/test013_18.m
    [Tags]     testcases/StaticLoop
tc test013_19.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/StaticLoop/test013_19.m
    [Tags]     testcases/StaticLoop
tc test013_20.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/StaticLoop/test013_20.m
    [Tags]     testcases/StaticLoop
tc test013_21.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/StaticLoop/test013_21.m
    [Tags]     testcases/StaticLoop
tc test013_22.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/StaticLoop/test013_22.m
    [Tags]     testcases/StaticLoop
tc test013_05.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/StaticLoop/test013_05.m
    [Tags]     testcases/StaticLoop
tc test013_23.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/StaticLoop/test013_23.m
    [Tags]     testcases/StaticLoop
tc test014_01.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DynamicLoop/test014_01.m
    [Tags]     testcases/DynamicLoop
tc test014_02.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DynamicLoop/test014_02.m
    [Tags]     testcases/DynamicLoop
tc test014_03.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DynamicLoop/test014_03.m
    [Tags]     testcases/DynamicLoop
tc test014_04.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DynamicLoop/test014_04.m
    [Tags]     testcases/DynamicLoop
tc test014_05.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DynamicLoop/test014_05.m
    [Tags]     testcases/DynamicLoop
tc test014_06.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DynamicLoop/test014_06.m
    [Tags]     testcases/DynamicLoop
tc test014_07.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DynamicLoop/test014_07.m
    [Tags]     testcases/DynamicLoop
tc test014_08.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DynamicLoop/test014_08.m
    [Tags]     testcases/DynamicLoop
tc test014_09.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DynamicLoop/test014_09.m
    [Tags]     testcases/DynamicLoop
tc test014_10.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DynamicLoop/test014_10.m
    [Tags]     testcases/DynamicLoop
tc test014_11.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DynamicLoop/test014_11.m
    [Tags]     testcases/DynamicLoop
tc test014_12.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DynamicLoop/test014_12.m
    [Tags]     testcases/DynamicLoop
tc test014_13.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DynamicLoop/test014_13.m
    [Tags]     testcases/DynamicLoop
tc test014_14.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DynamicLoop/test014_14.m
    [Tags]     testcases/DynamicLoop
tc test014_15.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DynamicLoop/test014_15.m
    [Tags]     testcases/DynamicLoop
tc test014_16.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DynamicLoop/test014_16.m
    [Tags]     testcases/DynamicLoop
tc test014_17.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DynamicLoop/test014_17.m
    [Tags]     testcases/DynamicLoop
tc test014_18.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DynamicLoop/test014_18.m
    [Tags]     testcases/DynamicLoop
tc test014_19.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DynamicLoop/test014_19.m
    [Tags]     testcases/DynamicLoop
tc test014_20.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DynamicLoop/test014_20.m
    [Tags]     testcases/DynamicLoop
tc test014_21.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DynamicLoop/test014_21.m
    [Tags]     testcases/DynamicLoop
tc test014_22.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DynamicLoop/test014_22.m
    [Tags]     testcases/DynamicLoop
tc test014_23.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DynamicLoop/test014_23.m
    [Tags]     testcases/DynamicLoop
tc test014_24.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DynamicLoop/test014_24.m
    [Tags]     testcases/DynamicLoop
tc test014_25.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DynamicLoop/test014_25.m
    [Tags]     testcases/DynamicLoop
tc test014_26.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DynamicLoop/test014_26.m
    [Tags]     testcases/DynamicLoop
tc test014_27.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DynamicLoop/test014_27.m
    [Tags]     testcases/DynamicLoop
tc test014_28.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/DynamicLoop/test014_28.m
    [Tags]     testcases/DynamicLoop
tc test015_01.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/FixedPoint/test015_01.m
    [Tags]     testcases/FixedPoint
tc test015_02.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/FixedPoint/test015_02.m
    [Tags]     testcases/FixedPoint
tc test015_03.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/FixedPoint/test015_03.m
    [Tags]     testcases/FixedPoint
tc test015_04.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/FixedPoint/test015_04.m
    [Tags]     testcases/FixedPoint
tc test015_05.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/FixedPoint/test015_05.m
    [Tags]     testcases/FixedPoint
tc test016_01.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MinMax/test016_01.m
    [Tags]     testcases/MinMax
tc test016_02.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MinMax/test016_02.m
    [Tags]     testcases/MinMax
tc test016_03.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MinMax/test016_03.m
    [Tags]     testcases/MinMax
tc test016_04.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MinMax/test016_04.m
    [Tags]     testcases/MinMax
tc test016_05.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MinMax/test016_05.m
    [Tags]     testcases/MinMax
tc test016_06.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MinMax/test016_06.m
    [Tags]     testcases/MinMax
tc test016_07.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MinMax/test016_07.m
    [Tags]     testcases/MinMax
tc test016_08.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MinMax/test016_08.m
    [Tags]     testcases/MinMax
tc test016_09.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MinMax/test016_09.m
    [Tags]     testcases/MinMax
tc test016_10.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MinMax/test016_10.m
    [Tags]     testcases/MinMax
tc test016_11.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MinMax/test016_11.m
    [Tags]     testcases/MinMax
tc test016_12.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MinMax/test016_12.m
    [Tags]     testcases/MinMax
tc test016_13.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MinMax/test016_13.m
    [Tags]     testcases/MinMax
tc test016_14.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MinMax/test016_14.m
    [Tags]     testcases/MinMax
tc test017_01.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Accumulation/test017_01.m
    [Tags]     testcases/Accumulation
tc test017_02.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Accumulation/test017_02.m
    [Tags]     testcases/Accumulation
tc test017_03.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Accumulation/test017_03.m
    [Tags]     testcases/Accumulation
tc test017_04.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Accumulation/test017_04.m
    [Tags]     testcases/Accumulation
tc test017_05.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Accumulation/test017_05.m
    [Tags]     testcases/Accumulation
tc test017_06.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Accumulation/test017_06.m
    [Tags]     testcases/Accumulation
tc test017_07.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Accumulation/test017_07.m
    [Tags]     testcases/Accumulation
tc test017_08.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Accumulation/test017_08.m
    [Tags]     testcases/Accumulation
tc test017_09.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Accumulation/test017_09.m
    [Tags]     testcases/Accumulation
tc test017_10.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Accumulation/test017_10.m
    [Tags]     testcases/Accumulation
tc test017_11.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Accumulation/test017_11.m
    [Tags]     testcases/Accumulation
tc test017_12.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Accumulation/test017_12.m
    [Tags]     testcases/Accumulation
tc test017_13.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Accumulation/test017_13.m
    [Tags]     testcases/Accumulation
tc test017_14.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Accumulation/test017_14.m
    [Tags]     testcases/Accumulation
tc test017_15.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Accumulation/test017_15.m
    [Tags]     testcases/Accumulation
tc test017_16.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Accumulation/test017_16.m
    [Tags]     testcases/Accumulation
tc test019_01.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/IfInsideLoop/test019_01.m
    [Tags]     testcases/IfInsideLoop
tc test019_02.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/IfInsideLoop/test019_02.m
    [Tags]     testcases/IfInsideLoop
tc test019_03.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/IfInsideLoop/test019_03.m
    [Tags]     testcases/IfInsideLoop
tc test019_04.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/IfInsideLoop/test019_04.m
    [Tags]     testcases/IfInsideLoop
tc test021_01.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/VecInLoop/test021_01.m
    [Tags]     testcases/VecInLoop
tc test021_02.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/VecInLoop/test021_02.m
    [Tags]     testcases/VecInLoop
tc test021_03.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/VecInLoop/test021_03.m
    [Tags]     testcases/VecInLoop
tc test021_04.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/VecInLoop/test021_04.m
    [Tags]     testcases/VecInLoop
tc test023_01.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Saturation/test023_01.m
    [Tags]     testcases/Saturation
tc test023_02.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/Saturation/test023_02.m
    [Tags]     testcases/Saturation
tc test024_01.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/ConstantOperand/test024_01.m
    [Tags]     testcases/ConstantOperand
tc test024_02.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/ConstantOperand/test024_02.m
    [Tags]     testcases/ConstantOperand
tc test024_03.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/ConstantOperand/test024_03.m
    [Tags]     testcases/ConstantOperand
tc test024_04.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/ConstantOperand/test024_04.m
    [Tags]     testcases/ConstantOperand
tc test024_05.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/ConstantOperand/test024_05.m
    [Tags]     testcases/ConstantOperand
tc test024_06.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/ConstantOperand/test024_06.m
    [Tags]     testcases/ConstantOperand
tc test024_07.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/ConstantOperand/test024_07.m
    [Tags]     testcases/ConstantOperand
tc test024_08.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/ConstantOperand/test024_08.m
    [Tags]     testcases/ConstantOperand
tc test024_09.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/ConstantOperand/test024_09.m
    [Tags]     testcases/ConstantOperand
tc test1001_01.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryWrite/test1001_01.m
    [Tags]     testcases/MemoryWrite
tc test1001_03.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryWrite/test1001_03.m
    [Tags]     testcases/MemoryWrite
tc test1001_02.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryWrite/test1001_02.m
    [Tags]     testcases/MemoryWrite
tc test1001_04.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryWrite/test1001_04.m
    [Tags]     testcases/MemoryWrite
tc test1001_05.m    /home/herenvarno/Projects/gitr/SiLago/vesyla/autotest/testcases/MemoryWrite/test1001_05.m
    [Tags]     testcases/MemoryWrite

*** Keywords ***
Autotest Template
    [Arguments]  ${filename}
    ${result} =    Run Process    ./run.sh  -c  -f  ../../fabric  ${filename}    shell=True    timeout=300s
    Should Not Contain    ${result.stdout}    FAIL
    Should Contain    ${result.stdout}    PASS
    Should Be Equal As Integers    ${result.rc}    0
