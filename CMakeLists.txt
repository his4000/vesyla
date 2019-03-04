cmake_minimum_required(VERSION 2.8.9)
project(Vesyla)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
# Show verbose command while compiling
#set(CMAKE_VERBOSE_MAKEFILE ON)

# The project version number.
set(VERSION_MAJOR 2 CACHE STRING "Project major version number.")
set(VERSION_MINOR 1 CACHE STRING "Project minor version number.")
set(VERSION_PATCH 0 CACHE STRING "Project patch version number.")
mark_as_advanced(VERSION_MAJOR VERSION_MINOR VERSION_PATCH)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(BISON)
find_package(FLEX)
find_package(Boost COMPONENTS regex system filesystem program_options unit_test_framework graph REQUIRED)
find_package(Gecode)

include_directories(/usr/local/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(include)
include_directories(src)

file(GLOB SOURCES_UTIL "src/util/*.cpp" "src/util/*/*.cpp" "src/util/*/*.cc")
file(GLOB SOURCES_VPARSER "src/VParser/*.cpp")
file(GLOB SOURCES_VMANAGER "src/VManager/*.cpp")
file(GLOB SOURCES_VIR "src/VIR/*.cpp")
file(GLOB SOURCES_ELABORATE "src/elaborate/*.cpp")
file(GLOB SOURCES_CIDFG "src/cidfg/*.cpp")
file(GLOB SOURCES_FEOPT "src/feopt/*.cpp")
file(GLOB SOURCES_VDUMP "src/VDump/*.cpp")
file(GLOB SOURCES_BIR "src/BIR/*.cpp")
file(GLOB SOURCES_CODEGEN "src/codegen/*.cpp")
file(GLOB SOURCES_SCHEDULE "src/schedule/*.cpp")
file(GLOB SOURCES_SYNC "src/sync/*.cpp")
file(GLOB SOURCES_SIM "src/sim/*.cpp")
file(GLOB SOURCES_FILEGEN "src/filegen/*.cpp")
file(GLOB SOURCES_BENGINE "src/BEngine/*.cpp")
file(GLOB SOURCES_EXEC_VESYLA "src/exec/vesyla/*.cpp")
file(GLOB SOURCES_EXEC_VESYLA_VERIFY "src/exec/vesyla_verify/*.cpp")
file(GLOB SOURCES_EXEC_VESYLA_SCHEDULE "src/exec/vesyla_schedule/*.cpp")

file(GLOB SOURCES_FLEX "src/VParser/*.l")
file(GLOB SOURCES_BISON "src/VParser/*.y")

set(CMAKE_CXX_FLAGS "-std=c++11 -w -pedantic -lpthread -g -O1 -DBOOST_LOG_DYN_LINK")
set(PROJECT_LINK_LIBS)

BISON_TARGET(VesylaParser ${SOURCES_BISON} "${CMAKE_SOURCE_DIR}/src/VParser/vparser.tab.cpp")
FLEX_TARGET(VesylaScanner ${SOURCES_FLEX} "${CMAKE_SOURCE_DIR}/src/VParser/vparser.flex.cpp")
ADD_FLEX_BISON_DEPENDENCY(VesylaScanner VesylaParser)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_library(Util SHARED ${SOURCES_UTIL})
add_library(VParser SHARED ${SOURCES_VPARSER} ${BISON_VesylaParser_OUTPUTS} ${FLEX_VesylaScanner_OUTPUTS})
add_library(VManager SHARED ${SOURCES_VMANAGER})
add_library(VIR SHARED ${SOURCES_VIR})
add_library(Elaborate SHARED ${SOURCES_ELABORATE})
add_library(Cidfg SHARED ${SOURCES_CIDFG})
add_library(BIR SHARED ${SOURCES_BIR})
add_library(Feopt SHARED ${SOURCES_FEOPT})
add_library(Codegen SHARED ${SOURCES_CODEGEN})
add_library(Schedule SHARED ${SOURCES_SCHEDULE})
add_library(Sync SHARED ${SOURCES_SYNC})
add_library(Sim SHARED ${SOURCES_SIM})
add_library(Filegen SHARED ${SOURCES_FILEGEN})
add_executable(vesyla ${SOURCES_EXEC_VESYLA})
target_link_libraries(vesyla Elaborate Schedule Sync Sim Codegen Feopt Cidfg Filegen VParser VManager BIR VIR Util ${Boost_LIBRARIES} ${GECODE_LIBRARIES} ${PROJECT_LINK_LIBS})
add_executable(vesyla_verify ${SOURCES_EXEC_VESYLA_VERIFY})
target_link_libraries(vesyla_verify ${Boost_LIBRARIES} ${PROJECT_LINK_LIBS})
add_executable(vesyla_schedule ${SOURCES_EXEC_VESYLA_SCHEDULE})
target_link_libraries(vesyla_schedule Schedule Sync Sim Codegen Feopt Cidfg Filegen VManager BIR VIR Util ${Boost_LIBRARIES} ${GECODE_LIBRARIES} ${PROJECT_LINK_LIBS})

# TEST
enable_testing()

function(add_boost_test SOURCE_FILE_NAME DEPENDENCY_LIB)
	get_filename_component(FILENAME ${SOURCE_FILE_NAME} NAME_WE)
	string(REGEX REPLACE "${CMAKE_SOURCE_DIR}/unit_test/" "" TEST_EXECUTABLE_NAME ${SOURCE_FILE_NAME})
	string(REGEX REPLACE ".cpp" "" TEST_EXECUTABLE_NAME ${TEST_EXECUTABLE_NAME})
	string(REGEX REPLACE "/+" "_" TEST_EXECUTABLE_NAME ${TEST_EXECUTABLE_NAME})
#	string(MD5 SUFFIX ${SOURCE_FILE_NAME})
#	string(SUBSTRING ${SUFFIX} 0 8 SHORT_SUFFIX)
#	string(CONCAT TEST_EXECUTABLE_NAME ${FILENAME} _ ${SHORT_SUFFIX})
	
	add_executable(${TEST_EXECUTABLE_NAME} ${SOURCE_FILE_NAME})
	target_link_libraries(${TEST_EXECUTABLE_NAME} Elaborate Schedule Sync Codegen Feopt Cidfg Filegen VParser VManager BIR VIR Util ${DEPENDENCY_LIB} ${Boost_LIBRARIES} ${GECODE_LIBRARIES})
	set_target_properties(${TEST_EXECUTABLE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/unit_test")
	
	
	file(READ "${SOURCE_FILE_NAME}" SOURCE_FILE_CONTENTS)
	string(REGEX MATCHALL "BOOST_AUTO_TEST_CASE\\( *([A-Za-z_0-9]+) *\\)" FOUND_TESTS ${SOURCE_FILE_CONTENTS})
	foreach(HIT ${FOUND_TESTS})
		string(REGEX REPLACE ".*\\( *([A-Za-z_0-9]+) *\\).*" "\\1" TEST_NAME ${HIT})
		add_test(NAME ${TEST_EXECUTABLE_NAME}.${TEST_NAME} 
							COMMAND "${CMAKE_BINARY_DIR}/unit_test/${TEST_EXECUTABLE_NAME}"
							--run_test=${TEST_NAME} --catch_system_error=yes)
	endforeach()
endfunction()

file(GLOB SOURCES_UNIT_TEST "unit_test/*.cpp" "unit_test/*/*.cpp")
foreach(F ${SOURCES_UNIT_TEST})
	add_boost_test(${F} "")
endforeach()

# DOCs
# add a target to generate API documentation with Doxygen
find_package(Doxygen)
option(BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" ${DOXYGEN_FOUND})

if(BUILD_DOCUMENTATION)
	if(NOT DOXYGEN_FOUND)
		message(FATAL_ERROR "Doxygen is needed to build the documentation.")
	endif()
	
	set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
	set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
	
	configure_file(${doxyfile_in} ${doxyfile} @ONLY)
	
	add_custom_target(doc
		COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Generating API documentation with Doxygen"
		VERBATIM)
	
	install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share/doc)
endif()
