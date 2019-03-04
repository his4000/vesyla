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

#ifndef _FlexUtility_H_
#define _FlexUtility_H_

#include <stdlib.h>
#include <iostream>
#include <string>
#include "ParserGlobal.hpp"

using std::cout;
using std::string;

inline char *realloc_string(char *buffer_, int length_)
{
	buffer_ = (char *)realloc(buffer_, length_);
	return buffer_;
}

inline void printLexMessage(const string &message_)
{
	return;
	using namespace GlobalValues;

	if (gLexDebug)
		switch (gLexReportMode)
		{
			//		case 1:
			//			WriteLineToF(message_, ftParser, false);
			//			break;
			//		case 2:
			//			WriteLineToF(message_, ftParser, false);
		case 0:
		default:
			cout << "\n"
					 << message_;
		}
}

inline void printLexToken(const string &token_)
{
	string str = "FLEX: Source line: " + std::to_string(GlobalValues::gSourceLine) + ", returned token: " + token_;

	printLexMessage(str);
}

extern void BeginToken()
{
	yylloc.first_line = yylineno;
}

#endif // _FlexUtility_H_
