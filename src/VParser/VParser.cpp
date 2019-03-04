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

#include "VParser.hpp"

#include "ParserGlobal.hpp"

//! Yu Yang 2017-08-01
// After move bison/flex generation to CMAKE, the header file name changes
// CHANGE BEGIN
#include "vparser.tab.hpp"
// CHANGE END

extern FILE *yyin;
extern int yydebug;
extern VIR::FunctionDeclaration *root;

using namespace GlobalValues;
using std::cout;

VIR::MainProgram *VParser::parse(const char *fileName_)
{
	// Get a file handle to a file
	FILE *inputfile;
	inputfile = fopen(fileName_, "r");

	// Make sure the file is valid
	if (!inputfile)
	{
		cout << "File not found: " << fileName_ << endl;
		return 0;
	}

	strncpy(gInputFileName, fileName_, 80);

	// Set lex to read from the file
	yyin = inputfile;

	// Debugging Configuration

	yydebug = 0;
	gLexDebug = false;
	gBisonDebug = false;
	gLexReportMode = 1;
	gBisonReportMode = 1;

	CREATE_NEW_LIBRARY;

	// Parsing
	yyparse();

	return dynamic_cast<VIR::MainProgram *>(root);
}
