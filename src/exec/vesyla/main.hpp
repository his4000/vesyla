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

#ifndef __VESYLA_MAIN_HPP__
#define __VESYLA_MAIN_HPP__

#include "VIR/VIRIncludeAll.hpp"
#include "VManager/MemoryManager.hpp"
#include "schedule/Scheduler.hpp"
#include "sync/Synchronizer.hpp"
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include "util/Common.hpp"
#include "VParser/VParser.hpp"
#include "cidfg/CidfgGraph.hpp"
#include "cidfg/Converter.hpp"
#include "cidfg/ScriptGenerator.hpp"
#include "feopt/Optimizer.hpp"
#include "codegen/Optimizer.hpp"
#include "sim/Simulator.hpp"
#include "elaborate/Elaborator.hpp"
#include "filegen/FileGenerator.hpp"
#include <chrono>

#define VERSION_MAJOR 2
#define VERSION_MINOR 1
#define VERSION_PATCH 0

const char *LICENSE_NAME = R"(GPL v3)";
const char *LICENSE_DESC = R"(
Vesyla
Copyright (C) 2017-2019  Yu Yang <yuyang2@kth.se>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
)";

const char *LOGGING_CONF = R"(
* GLOBAL:
   FORMAT               =  "[%level %datetime{%H:%m:%s}] %msg"
   FILENAME             =  "logs/vesyla.log"
   ENABLED              =  true
   TO_FILE              =  true
   TO_STANDARD_OUTPUT   =  true
   SUBSECOND_PRECISION  =  3
   PERFORMANCE_TRACKING =  false
   MAX_LOG_FILE_SIZE    =  2097152
   LOG_FLUSH_THRESHOLD  =  100
* DEBUG:
   FORMAT               =  "[%level %datetime{%H:%m:%s}] (%func @ %loc) %msg"
)";

int main(int argc, char **argv);

#endif // __VESYLA_MAIN_HPP__