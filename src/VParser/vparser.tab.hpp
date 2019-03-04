/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_HOME_HERENVARNO_PROJECTS_GITR_SILAGO_VESYLA_SRC_VPARSER_VPARSER_TAB_HPP_INCLUDED
# define YY_YY_HOME_HERENVARNO_PROJECTS_GITR_SILAGO_VESYLA_SRC_VPARSER_VPARSER_TAB_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    LEXERROR = 258,
    LINE = 259,
    LD = 260,
    RD = 261,
    FOR = 262,
    END = 263,
    IF = 264,
    ELSEIF = 265,
    ELSE = 266,
    GLOBAL = 267,
    WHILE = 268,
    FUNCTION = 269,
    RETURN = 270,
    PRAGMASTART = 271,
    PARALLEL = 272,
    STRUCTURAL = 273,
    DPU = 274,
    RFILE = 275,
    MEMORY = 276,
    CDPU = 277,
    EVENDIST = 278,
    FULLDIST = 279,
    CUSTOMDIST = 280,
    DPUMODE = 281,
    DPUOUT = 282,
    DPU_SATURATION = 283,
    ADDRESS_REG = 284,
    VAR_INPUT = 285,
    VAR_OUTPUT = 286,
    RACCU_VAR = 287,
    ADDRESS_VAR = 288,
    TEMP_VAR = 289,
    MEM_ADDRESS = 290,
    CONSTROW = 291,
    CONSTCOL = 292,
    CONSTDIMENSION = 293,
    CONSTPARALLELISM = 294,
    RESOURCE_SHARING_BEGIN = 295,
    RESOURCE_SHARING_END = 296,
    TEXT = 297,
    IDENTIFIER = 298,
    INTEGER = 299,
    DOUBLE = 300,
    IMAGINARY = 301,
    AND = 302,
    OR = 303,
    LTHAN = 304,
    LTHANE = 305,
    GTHAN = 306,
    GTHANE = 307,
    EQUAL = 308,
    UNEQUAL = 309,
    xo = 310,
    MMUL = 311,
    EMUL = 312,
    MDIV = 313,
    EDIV = 314,
    MLEFTDIV = 315,
    ELEFTDIV = 316,
    EPOWER = 317,
    MPOWER = 318,
    NOT = 319,
    UNARYPLUS = 320,
    UNARYMINUS = 321,
    TRANSPOSE = 322,
    CTRANSPOSE = 323
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 118 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1909  */

          char* text;
          char* symbol;

          int integerVal;
          double doubleVal;
          int imaginaryVal;


          int ParallelModeType;
          int StorageTypeE;
          int PragmaConstantTypeE;
          int RaccuOptionsType;
          int StorageDistributionType;
          int VariableTypeE;


          class Array * ArrayType;
          class DPUPragma * DPUPragmaType;
          class Expression * ExpressionType;
          class FunctionDeclaration * FunctionDeclarationType;
          class Identifier * IdentifierType;
          class Name * NameType;
          class Pragma * PragmaType;
          class RangeExpression * RangeExpressionType;
          class Statement * StatementType;

          struct ArrayLinker * ArrayLinkerType;
          struct ExpressionLinker * ExpressionLinkerType;
          struct StatementLinker * StatementLinkerType;
          struct NameLinker * NameLinkerType;
          struct DPUOptions * DPUOptionsType;
          struct StorageOptions * StorageOptionsType;

          struct _variable {
            int size;
            char** entries;
          } variables;

          int delimiter;

          struct _var_expr_t* vexpression;

          struct _singleton {
            int size;
            struct _expr_t** expressions;
          } singletons;

          struct _global_t* globalStatement;
          struct _while_t* whileStatement;
        

#line 176 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.hpp" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


extern YYSTYPE yylval;
extern YYLTYPE yylloc;
int yyparse (void);

#endif /* !YY_YY_HOME_HERENVARNO_PROJECTS_GITR_SILAGO_VESYLA_SRC_VPARSER_VPARSER_TAB_HPP_INCLUDED  */
