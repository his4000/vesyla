/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 19 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:339  */


/* 
 *  Context-free grammar for MATLAB
 *  Command-line options: --yacc -d -t
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h> 

#include "VIR/VIRIncludeAll.hpp"
#include "VManager/MemoryManager.hpp"
#include "BisonUtility.hpp"

using namespace VIR;
using namespace VIR::VirEnumerations;

struct DPUOptions {
	int dpuMode;
	int outPort;
	bool saturation;
};

struct StorageOptions {
	StorageDistribution distribution;
	bool isForIndirectAddress;
	VariableType variableType;
};

#define LOOKAHEAD yychar
#define YYDEBUG 1

int gBracketDelimiter = '\0';


extern int yylex(void);
extern void yyinsert_comma_in_input(const int);
static void yyerror(const char*);

static const char* FunctionName;

std::vector<std::string> global_reference_list;


/*
 *   MATLAB's expression operators fall into three categories:
 *
 *       + Arithmetic operators.
 *       + Relational operators.
 *       + Logical operators.
 *
 *   The precedences documented in the manual appear to be wrong. For
 *   instance, according to the manual, arithmetic operators have the
 *   highest precedence, followed by relational operators, followed by
 *   logical operators. This would mean that the MATLAB interpretor
 *   should consider the token sequence NOT INTEGER '+' INTEGER as being
 *   equivalent to NOT '(' INTEGER '+' INTEGER ')', and not as '(' NOT
 *   INTEGER ')' '+' INTEGER. However, the MATLAB interpretor actually
 *   treats the token sequence in the latter manner.
 *
 *   The following were the observed precedence levels of operators in
 *   the MATLAB 5.0 interpretor, arranged in decreasing order:
 *
 *      1. Element-wise power (EPOWER), matrix power (MPOWER).
 *
 *      2. Transpose (TRANSPOSE),
 *      complex conjugate transpose (CTRANSPOSE).
 *
 *      3. Logical negation (NOT),
 *      unary plus ('+'), unary minus ('-').
 *
 *      4. Matrix multiplication (MMUL), element-wise multiplication (EMUL),
 *      matrix division (MDIV), element-wise division (EDIV),
 *      matrix left division (MLEFTDIV), element-wise left division (ELEFTDIV).
 *
 *      5. Addition ('+'), subtraction ('-').
 *
 *      6. Colon operator (':').
 *
 *      7. Less than (LTHAN), less than or equal to (LTHANE),
 *      greater than (GTHAN), greater than or equal to (GTHANE),
 *      equal to (EQUAL), not equal to (UNEQUAL).
 *
 *      8. Logical and (AND), logical or (OR).
 *
 *   Within each precedence level, operators have left associativity,
 *   except for the level 2 and level 3 operators, which are
 *   non-associative.
 */


#line 159 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "vparser.tab.hpp".  */
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
#line 118 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:355  */

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
        

#line 321 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:355  */
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

/* Copy the second part of user declarations.  */

#line 352 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  21
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   561

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  81
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  65
/* YYNRULES -- Number of rules.  */
#define YYNRULES  170
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  280

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   323

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      79,    80,     2,    58,    72,    57,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    55,    73,
      76,    78,    77,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    74,     2,    75,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      56,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   276,   276,   281,   286,   294,   306,   307,   311,   312,
     313,   317,   318,   322,   323,   324,   325,   329,   330,   333,
     334,   335,   339,   344,   350,   359,   362,   369,   401,   422,
     435,   441,   449,   452,   458,   461,   467,   476,   486,   496,
     510,   520,   536,   551,   563,   566,   571,   573,   576,   579,
     583,   586,   592,   595,   603,   630,   644,   653,   662,   670,
     678,   685,   696,   703,   711,   720,   727,   732,   739,   750,
     760,   772,   784,   783,   797,   803,   809,   816,   823,   831,
     839,   846,   853,   860,   867,   874,   881,   888,   895,   902,
     909,   916,   923,   930,   937,   944,   951,   958,   965,   972,
     982,   990,   995,  1007,  1006,  1037,  1049,  1056,  1062,  1070,
    1079,  1078,  1095,  1103,  1108,  1116,  1121,  1131,  1136,  1148,
    1153,  1158,  1167,  1172,  1181,  1199,  1229,  1275,  1319,  1324,
    1332,  1331,  1350,  1349,  1379,  1386,  1390,  1397,  1404,  1422,
    1427,  1434,  1441,  1476,  1492,  1498,  1504,  1519,  1523,  1528,
    1565,  1585,  1596,  1609,  1616,  1640,  1654,  1660,  1668,  1683,
    1700,  1705,  1713,  1716,  1720,  1727,  1739,  1746,  1752,  1760,
    1765
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "LEXERROR", "LINE", "LD", "RD", "FOR",
  "END", "IF", "ELSEIF", "ELSE", "GLOBAL", "WHILE", "FUNCTION", "RETURN",
  "PRAGMASTART", "PARALLEL", "STRUCTURAL", "DPU", "RFILE", "MEMORY",
  "CDPU", "EVENDIST", "FULLDIST", "CUSTOMDIST", "DPUMODE", "DPUOUT",
  "DPU_SATURATION", "ADDRESS_REG", "VAR_INPUT", "VAR_OUTPUT", "RACCU_VAR",
  "ADDRESS_VAR", "TEMP_VAR", "MEM_ADDRESS", "CONSTROW", "CONSTCOL",
  "CONSTDIMENSION", "CONSTPARALLELISM", "RESOURCE_SHARING_BEGIN",
  "RESOURCE_SHARING_END", "TEXT", "IDENTIFIER", "INTEGER", "DOUBLE",
  "IMAGINARY", "AND", "OR", "LTHAN", "LTHANE", "GTHAN", "GTHANE", "EQUAL",
  "UNEQUAL", "':'", "xo", "'-'", "'+'", "MMUL", "EMUL", "MDIV", "EDIV",
  "MLEFTDIV", "ELEFTDIV", "EPOWER", "MPOWER", "NOT", "UNARYPLUS",
  "UNARYMINUS", "TRANSPOSE", "CTRANSPOSE", "','", "';'", "'['", "']'",
  "'<'", "'>'", "'='", "'('", "')'", "$accept", "input", "scriptMFile",
  "opt_delimiter", "delimiter", "null_lines", "null_line", "empty_lines",
  "assign_delimiter", "statement_list", "assignment_pragma",
  "assignment_pragma_node", "raccu_options_type", "dpu_object",
  "dpu_options", "storage_options", "variable_type", "storage_dist",
  "storage_type", "statement", "block_pragma", "block_pragma_node",
  "command_form", "text_list", "expr", "$@1", "parenthesis", "reference",
  "$@2", "identifier", "argument_list", "matrix", "$@3", "boxes1", "rows",
  "row", "row_with_commas", "colon_expr", "assignment",
  "s_assignee_matrix", "$@4", "m_assignee_matrix", "$@5", "boxes2",
  "reference_list", "for_command", "for_cmd_list", "for_pragma",
  "if_command", "if_cmd_list", "if_pragma", "if_pragma_expr", "opt_else",
  "global_command", "global_decl_list", "while_command", "while_cmd_list",
  "return_command", "functionMFile", "f_def_line", "f_output", "f_input",
  "f_argument_list", "f_body", "parse_error", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,    58,   310,    45,    43,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,    44,    59,    91,    93,    60,    62,    61,    40,
      41
};
# endif

#define YYPACT_NINF -111

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-111)))

#define YYTABLE_NINF -106

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      42,  -111,  -111,  -111,    22,  -111,  -111,     9,  -111,    55,
    -111,     8,  -111,    70,  -111,     8,  -111,   -27,   -72,  -111,
     -42,  -111,  -111,   -27,   123,    -2,   123,  -111,   -30,  -111,
     -27,  -111,    59,  -111,  -111,     0,   -18,     4,    13,    21,
    -111,  -111,  -111,  -111,  -111,  -111,    35,  -111,  -111,  -111,
       8,  -111,    16,    35,  -111,    10,    99,    66,  -111,    51,
     -27,    39,   114,  -111,  -111,  -111,  -111,   123,   123,   123,
    -111,  -111,   347,  -111,    54,  -111,    75,   129,  -111,    97,
     284,   133,  -111,  -111,  -111,    73,   141,    88,  -111,   123,
      14,   137,  -111,    69,   112,   137,   123,   123,  -111,  -111,
     -27,  -111,  -111,    78,    82,    98,   123,  -111,    18,    18,
      18,   123,   123,    92,   123,   123,   123,   123,   123,   123,
     123,   123,   123,   123,   123,   123,   123,   123,   123,   123,
     123,   123,   123,  -111,  -111,   160,    55,   123,  -111,  -111,
    -111,  -111,   -27,    94,  -111,   475,  -111,   166,  -111,  -111,
      81,  -111,  -111,   475,   475,  -111,  -111,  -111,   -27,   168,
     372,   397,    29,  -111,   123,   475,  -111,   105,    59,   320,
     320,   246,   246,   246,   246,   246,   246,   490,   259,   259,
      86,    86,    86,    86,    86,    86,    18,    18,   102,   490,
    -111,   123,  -111,  -111,  -111,  -111,   154,  -111,  -111,  -111,
     116,   117,   122,   423,  -111,  -111,  -111,   178,   160,    55,
    -111,   123,   123,   121,   449,   124,    81,   160,   123,    59,
    -111,   475,  -111,  -111,   -11,   132,   153,   134,    81,    81,
     127,   197,    59,  -111,  -111,  -111,  -111,  -111,  -111,   135,
     347,    88,   131,   136,  -111,    81,  -111,  -111,  -111,   138,
    -111,  -111,   142,   143,    81,  -111,  -111,  -111,  -111,   160,
    -111,    55,  -111,   170,  -111,   146,  -111,  -111,  -111,   148,
     102,   140,  -111,  -111,  -111,   198,   150,   180,   152,  -111
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,   170,   169,    17,     0,    13,    14,     0,     2,    52,
       7,     8,    11,     9,     3,     6,     4,     0,   162,   160,
       0,     1,   134,     0,     0,     0,     0,   155,     0,   105,
       0,     5,    22,    61,    53,    19,   101,    19,     0,     0,
      56,    57,    58,    59,    60,    12,    10,    18,    15,    16,
       6,   168,     7,     9,   157,   166,     0,     0,   159,     0,
       0,     0,     0,    71,    68,    69,    70,     0,     0,     0,
     112,   100,     0,    74,   101,    75,    91,     0,   151,   150,
      52,     0,    63,    64,    62,   135,     0,    23,    20,     0,
       0,    25,    66,   100,    65,    25,     0,     0,   156,   167,
       0,   161,   163,     0,   162,   130,     0,   137,    82,    81,
      80,   113,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    78,    79,   143,    52,     0,   141,   152,
     154,   153,     0,     0,    24,   126,    21,     0,    54,   102,
       0,    67,    55,   128,   129,   165,   164,   158,     0,     0,
       0,   119,   110,   114,   120,    72,   145,     0,     0,    98,
      99,    92,    93,    94,    95,    96,    97,   124,    90,    89,
      83,    84,    85,    86,    87,    88,    76,    77,   147,   125,
     136,     0,    34,    50,    51,    35,    32,    30,    31,    26,
      36,    40,   106,   107,   103,   132,   131,     0,   139,    52,
     122,   117,   115,     0,   121,     0,     0,   144,     0,     0,
     142,   127,    33,    29,     0,     0,    46,     0,     0,     0,
       0,     0,     0,   138,   118,   116,   111,   123,    73,     0,
       0,    52,     0,     0,    37,     0,    47,    48,    49,     0,
      44,    45,     0,     0,     0,   108,   109,   104,   133,   140,
     146,    52,   148,     0,    38,     0,    43,    42,    41,     0,
     147,     0,    27,    28,   149,     0,     0,     0,     0,    39
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -111,  -111,  -111,   226,   227,  -111,   219,     3,   194,   -50,
     139,  -111,  -111,  -111,  -111,  -111,  -111,  -111,  -111,  -111,
    -111,  -111,  -111,  -111,   149,  -111,   147,    -8,  -111,    -4,
    -110,  -111,  -111,  -111,  -111,   -91,  -111,  -111,  -111,  -111,
    -111,  -111,  -111,  -111,  -108,  -111,  -111,  -111,  -111,  -111,
      -1,  -111,   -38,  -111,  -111,  -111,  -111,  -111,  -111,   223,
    -111,   144,   -31,   191,  -111
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     7,     8,    51,    52,    11,    12,    53,    91,    31,
     148,   199,   223,   200,   225,   227,   252,   253,   201,    32,
      33,    84,    34,    94,   203,   215,   112,    73,   230,    74,
     204,    75,   213,   111,   162,   163,   164,    76,    37,    38,
     159,    39,   231,    60,    86,    40,    62,   209,    41,    77,
     136,   168,   220,    42,    79,    43,    81,    44,    14,    15,
      20,    58,    56,    54,    16
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      19,    35,    99,    13,     3,    36,  -105,    57,     3,    21,
      82,    83,     3,    55,    46,   242,    29,   243,    47,    61,
     -52,    22,    85,    23,    92,    24,   103,    17,    25,    26,
     140,    27,    28,   211,   190,    87,    59,   144,    90,    47,
      90,    78,    -6,     1,    35,     2,     3,    -6,    36,    -6,
     205,    -6,   105,    55,    -6,    -6,     4,    -6,    -6,    29,
      22,    93,    23,     3,    24,    18,   244,    25,    26,   155,
      27,    28,    35,    88,    47,   135,    36,    88,    89,    35,
       5,     6,   100,    36,     4,    -6,   188,   146,   133,   134,
      30,    96,    47,    22,   104,    23,    55,    24,    29,    97,
      25,    26,   212,    27,    28,   101,   239,    48,    49,    29,
     166,   167,   218,   219,     5,     6,    -6,   106,   255,   256,
     234,   235,   107,    63,    29,    64,    65,    66,    35,    30,
     137,    29,    36,    93,    85,   265,   202,   138,    67,    68,
     139,   141,    48,    49,   269,   142,   102,   143,    69,   149,
      85,   131,   132,   147,   151,    70,   133,   134,   156,   233,
      71,    57,    30,   208,    47,    63,    29,    64,    65,    66,
     158,   217,   191,    72,   206,    80,   246,   247,   248,   216,
      67,    68,   249,   250,   251,   192,   193,   194,   195,   222,
      69,   262,   224,   226,   228,   232,   236,    70,   196,   197,
     198,    35,    71,   258,   238,    36,   245,   257,   254,   263,
     260,   270,   275,   264,   271,   266,   108,   109,   110,   267,
     268,   272,   241,   273,   278,   276,     9,    10,   277,   279,
      45,    95,   274,    35,   152,   259,    50,    36,   145,   261,
     150,    98,     0,   135,     0,   153,   154,     0,   157,     0,
       0,     0,     0,    35,     0,   160,     0,    36,     0,     0,
     161,   165,     0,   169,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,     0,     0,     0,     0,   189,     0,     0,    22,
       0,    23,     0,    24,     0,     0,    25,    26,     0,    27,
      28,   122,     0,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   214,     0,     0,   133,   134,   125,   126,
     127,   128,   129,   130,   131,   132,     0,    29,     0,   133,
     134,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     221,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,     3,     0,     0,   133,   134,     0,     0,    30,     0,
     161,   161,     0,   113,     0,     0,     0,   240,     0,   116,
     117,   118,   119,   120,   121,   122,     3,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,     0,   207,     0,
     133,   134,     0,     0,   114,   115,   116,   117,   118,   119,
     120,   121,   122,     0,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,     0,     0,     0,   133,   134,   114,
     115,   116,   117,   118,   119,   120,   121,   122,     0,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,     0,
       0,     0,   133,   134,   114,   115,   116,   117,   118,   119,
     120,   121,   122,     0,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,     0,     0,     0,   133,   134,   210,
     114,   115,   116,   117,   118,   119,   120,   121,   122,     0,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
       0,     0,     0,   133,   134,   229,   114,   115,   116,   117,
     118,   119,   120,   121,   122,     0,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,     0,     0,     0,   133,
     134,   237,   114,   115,   116,   117,   118,   119,   120,   121,
     122,     0,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,     0,     0,     0,   133,   134,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,     0,     0,     0,
     133,   134
};

static const yytype_int16 yycheck[] =
{
       4,     9,    52,     0,     4,     9,    78,    79,     4,     0,
      40,    41,     4,    17,    11,    26,    43,    28,     4,    23,
       4,     5,    30,     7,    42,     9,    57,     5,    12,    13,
      80,    15,    16,     4,   142,    32,    78,    87,    35,     4,
      37,    43,     0,     1,    52,     3,     4,     5,    52,     7,
     158,     9,    60,    57,    12,    13,    14,    15,    16,    43,
       5,    79,     7,     4,     9,    43,    77,    12,    13,   100,
      15,    16,    80,    73,     4,    72,    80,    73,    78,    87,
      72,    73,    72,    87,    14,    43,   136,    73,    70,    71,
      74,    78,     4,     5,    43,     7,   100,     9,    43,    78,
      12,    13,    73,    15,    16,     6,   216,    72,    73,    43,
      18,    19,    10,    11,    72,    73,    74,    78,   228,   229,
     211,   212,     8,    42,    43,    44,    45,    46,   136,    74,
      55,    43,   136,    79,   142,   245,    55,     8,    57,    58,
      43,     8,    72,    73,   254,    72,    80,     6,    67,    80,
     158,    65,    66,    16,    42,    74,    70,    71,    80,   209,
      79,    79,    74,   160,     4,    42,    43,    44,    45,    46,
      72,   168,    78,    24,     6,    26,    23,    24,    25,    74,
      57,    58,    29,    30,    31,    19,    20,    21,    22,    35,
      67,   241,    76,    76,    72,    17,    75,    74,    32,    33,
      34,   209,    79,     6,    80,   209,    74,    80,    74,    78,
      75,   261,    72,    77,    44,    77,    67,    68,    69,    77,
      77,    75,   219,    75,    44,    27,     0,     0,    78,    77,
      11,    37,   270,   241,    95,   232,    13,   241,    89,   240,
      93,    50,    -1,   240,    -1,    96,    97,    -1,   104,    -1,
      -1,    -1,    -1,   261,    -1,   106,    -1,   261,    -1,    -1,
     111,   112,    -1,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,    -1,    -1,    -1,    -1,   137,    -1,    -1,     5,
      -1,     7,    -1,     9,    -1,    -1,    12,    13,    -1,    15,
      16,    55,    -1,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,   164,    -1,    -1,    70,    71,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    43,    -1,    70,
      71,    47,    48,    49,    50,    51,    52,    53,    54,    55,
     191,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,     4,    -1,    -1,    70,    71,    -1,    -1,    74,    -1,
     211,   212,    -1,    16,    -1,    -1,    -1,   218,    -1,    49,
      50,    51,    52,    53,    54,    55,     4,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    16,    -1,
      70,    71,    -1,    -1,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    -1,    -1,    70,    71,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    -1,
      -1,    -1,    70,    71,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    -1,    -1,    70,    71,    72,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      -1,    -1,    -1,    70,    71,    72,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    -1,    -1,    -1,    70,
      71,    72,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    -1,    -1,    -1,    70,    71,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    -1,    -1,    -1,
      70,    71
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     3,     4,    14,    72,    73,    82,    83,    84,
      85,    86,    87,    88,   139,   140,   145,     5,    43,   110,
     141,     0,     5,     7,     9,    12,    13,    15,    16,    43,
      74,    90,   100,   101,   103,   108,   110,   119,   120,   122,
     126,   129,   134,   136,   138,    87,    88,     4,    72,    73,
     140,    84,    85,    88,   144,   110,   143,    79,   142,    78,
     124,   110,   127,    42,    44,    45,    46,    57,    58,    67,
      74,    79,   105,   108,   110,   112,   118,   130,    43,   135,
     105,   137,    40,    41,   102,   108,   125,    88,    73,    78,
      88,    89,    42,    79,   104,    89,    78,    78,   144,    90,
      72,     6,    80,   143,    43,   108,    78,     8,   105,   105,
     105,   114,   107,    16,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    70,    71,    88,   131,    55,     8,    43,
      90,     8,    72,     6,    90,   105,    73,    16,    91,    80,
     107,    42,    91,   105,   105,   143,    80,   142,    72,   121,
     105,   105,   115,   116,   117,   105,    18,    19,   132,   105,
     105,   105,   105,   105,   105,   105,   105,   105,   105,   105,
     105,   105,   105,   105,   105,   105,   105,   105,    90,   105,
     125,    78,    19,    20,    21,    22,    32,    33,    34,    92,
      94,    99,    55,   105,   111,   125,     6,    16,    88,   128,
      72,     4,    73,   113,   105,   106,    74,    88,    10,    11,
     133,   105,    35,    93,    76,    95,    76,    96,    72,    72,
     109,   123,    17,    90,   116,   116,    75,    72,    80,   111,
     105,    88,    26,    28,    77,    74,    23,    24,    25,    29,
      30,    31,    97,    98,    74,   111,   111,    80,     6,    88,
      75,   131,    90,    78,    77,   111,    77,    77,    77,   111,
      90,    44,    75,    75,   133,    72,    27,    78,    44,    77
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    81,    82,    82,    82,    83,    84,    84,    85,    85,
      85,    86,    86,    87,    87,    87,    87,    88,    88,    89,
      89,    89,    90,    90,    90,    91,    91,    92,    92,    92,
      92,    92,    93,    93,    94,    94,    95,    95,    95,    95,
      96,    96,    96,    96,    97,    97,    98,    98,    98,    98,
      99,    99,   100,   100,   100,   100,   100,   100,   100,   100,
     100,   100,   101,   102,   102,   103,   104,   104,   105,   105,
     105,   105,   106,   105,   105,   105,   105,   105,   105,   105,
     105,   105,   105,   105,   105,   105,   105,   105,   105,   105,
     105,   105,   105,   105,   105,   105,   105,   105,   105,   105,
     107,   108,   108,   109,   108,   110,   111,   111,   111,   111,
     113,   112,   114,   115,   115,   115,   115,   115,   115,   116,
     116,   116,   117,   117,   118,   118,   119,   119,   119,   119,
     121,   120,   123,   122,   124,   125,   125,   126,   127,   128,
     128,   129,   130,   131,   131,   132,   132,   133,   133,   133,
     134,   135,   135,   136,   137,   138,   139,   139,   140,   140,
     141,   141,   142,   142,   142,   143,   143,   144,   144,   145,
     145
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     2,     0,     1,     1,     1,
       2,     1,     2,     1,     1,     2,     2,     1,     2,     0,
       1,     2,     1,     2,     3,     0,     2,     5,     5,     2,
       1,     1,     0,     1,     1,     1,     0,     2,     3,     9,
       0,     3,     3,     3,     1,     1,     0,     1,     1,     1,
       1,     1,     0,     1,     3,     3,     1,     1,     1,     1,
       1,     1,     2,     1,     1,     2,     1,     2,     1,     1,
       1,     1,     0,     5,     1,     1,     3,     3,     2,     2,
       2,     2,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     1,     3,     3,     3,     3,     3,     3,     3,     3,
       0,     1,     3,     0,     6,     1,     1,     1,     3,     3,
       0,     5,     0,     0,     1,     2,     3,     2,     3,     1,
       1,     2,     2,     3,     3,     3,     3,     5,     3,     3,
       0,     5,     0,     7,     0,     1,     3,     3,     5,     1,
       3,     3,     4,     1,     3,     1,     4,     0,     3,     5,
       2,     1,     2,     3,     2,     1,     3,     2,     5,     3,
       1,     3,     0,     2,     3,     3,     1,     2,     1,     1,
       1
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static unsigned
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  unsigned res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
 }

#  define YY_LOCATION_PRINT(File, Loc)          \
  yy_location_print_ (File, &(Loc))

# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, Location); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (yylocationp);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                       , &(yylsp[(yyi + 1) - (yynrhs)])                       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Location data for the lookahead symbol.  */
YYLTYPE yylloc
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.
       'yyls': related to locations.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[3];

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yylsp = yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  yylsp[0] = yylloc;
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yyls1, yysize * sizeof (*yylsp),
                    &yystacksize);

        yyls = yyls1;
        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 277 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        (yyval.FunctionDeclarationType) = root = (yyvsp[0].FunctionDeclarationType);
                      }
#line 1810 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 282 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        (yyval.FunctionDeclarationType) = root = (yyvsp[0].FunctionDeclarationType);
                      }
#line 1818 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 287 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        (yyval.FunctionDeclarationType) = root = 0;
                      }
#line 1826 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 295 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("scriptMFile -> opt_delimiter statement_list", true);
                        MainProgram *mainProgram = CREATE_OBJECT(MainProgram);
                        mainProgram->lineNumber((yylsp[-1]).first_line);
                        mainProgram->name("__main");
                        fillStatementList((yyvsp[0].StatementLinkerType), &mainProgram->body());
                        (yyval.FunctionDeclarationType) = mainProgram;
                      }
#line 1839 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 340 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("statement_list -> statement");
                        (yyval.StatementLinkerType) = new StatementLinker((yyvsp[0].StatementType), 0);
					  }
#line 1848 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 345 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("statement_list -> statement empty_lines");

                        (yyval.StatementLinkerType) = new StatementLinker((yyvsp[-1].StatementType), 0);
                      }
#line 1858 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 351 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("statement_list -> statement empty_lines statement_list");

                        (yyval.StatementLinkerType) = new StatementLinker((yyvsp[-2].StatementType), (yyvsp[0].StatementLinkerType));
					  }
#line 1868 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 359 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        (yyval.PragmaType) = 0;
                      }
#line 1876 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 363 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        (yyval.PragmaType) = (yyvsp[0].PragmaType);
                      }
#line 1884 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 370 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
						printGrammar("C/DPU '[' argument_list ']'");
						//cout << "\nC/DPU grammar";

                        DPUPragma * dpuPragma = (yyvsp[-4].DPUPragmaType);

						dpuPragma->outPort = (yyvsp[-3].DPUOptionsType)->outPort;
						dpuPragma->saturation = (yyvsp[-3].DPUOptionsType)->saturation;
						
						switch ((yyvsp[-3].DPUOptionsType)->dpuMode)
						{
							/* DEPRACATED
							 * use string to indicate DPU mode
							 */
							/*
							case 4: dpuPragma->dpuMode = BIR::BIREnumerations::dmFFT1; break;
							case 5: dpuPragma->dpuMode = BIR::BIREnumerations::dmFFT2; break;
							*/

							case 4: dpuPragma->dpuMode = "__sl_fft1"; break;
							case 5: dpuPragma->dpuMode = "__sl_fft2"; break;
							// otherwise it has some default value
						}

						delete (yyvsp[-3].DPUOptionsType);

                        fillExpressionList((yyvsp[-1].ExpressionLinkerType), &dpuPragma->reference());

                        (yyval.PragmaType) = dpuPragma;
                      }
#line 1919 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 402 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("RFILE '[' argument_list ']'");
                        //cout << "\nRFILE grammar";

                        StoragePragma * storagePragma = CREATE_OBJECT(StoragePragma);
                        storagePragma->lineNumber((yylsp[-4]).first_line);
//! Yu Yang 2017-08-01
// Use int value to replace custom enum to avoid bison bug for generating C style output
// CHANGE BEGIN
						storagePragma->storageType((StorageType)(yyvsp[-4].StorageTypeE));
// CHANGE END
                        storagePragma->distribution((yyvsp[-3].StorageOptionsType)->distribution);
						storagePragma->isForIndirectAddress((yyvsp[-3].StorageOptionsType)->isForIndirectAddress);
						storagePragma->variableType((yyvsp[-3].StorageOptionsType)->variableType);

                        fillExpressionList((yyvsp[-1].ExpressionLinkerType), &storagePragma->reference());

                        (yyval.PragmaType) = storagePragma;
                      }
#line 1943 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 423 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
						RaccuPragma * raccuPragma = CREATE_OBJECT(RaccuPragma);
            raccuPragma->lineNumber((yylsp[-1]).first_line);
//! Yu Yang 2017-08-01
// Use int value to replace custom enum to avoid bison bug for generating C style output
// CHANGE BEGIN
						raccuPragma->raccuOptions = (RaccuOptions)(yyvsp[0].RaccuOptionsType);
// CHANGE END

						(yyval.PragmaType) = raccuPragma;
					  }
#line 1959 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 436 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
						(yyval.PragmaType) = CREATE_OBJECT(AddressPragma);
            (yyval.PragmaType)->lineNumber((yylsp[0]).first_line);
					  }
#line 1968 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 442 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
						(yyval.PragmaType) = CREATE_OBJECT(TempVarPragma);
            (yyval.PragmaType)->lineNumber((yylsp[0]).first_line);
					  }
#line 1977 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 449 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
						(yyval.RaccuOptionsType) = VIR::VirEnumerations::roNone;
					  }
#line 1985 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 453 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
						(yyval.RaccuOptionsType) = VIR::VirEnumerations::roMemoryAddress;
					  }
#line 1993 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 459 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    { (yyval.DPUPragmaType) = CREATE_OBJECT(DPUPragma);
            (yyval.DPUPragmaType)->lineNumber((yylsp[0]).first_line);}
#line 2000 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 462 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    { (yyval.DPUPragmaType) = CREATE_OBJECT(DPUChainPragma);
            (yyval.DPUPragmaType)->lineNumber((yylsp[0]).first_line);}
#line 2007 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 467 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    { 
						DPUOptions * dpuOptions = new DPUOptions;
						dpuOptions->dpuMode = 0;
						dpuOptions->outPort = 0;
						dpuOptions->saturation = false;

						(yyval.DPUOptionsType) = dpuOptions;
					  }
#line 2020 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 477 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
						DPUOptions * dpuOptions = new DPUOptions;
						dpuOptions->dpuMode = 0;
						dpuOptions->outPort = 0;
						dpuOptions->saturation = false;

						(yyval.DPUOptionsType) = dpuOptions;
					  }
#line 2033 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 487 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
						DPUOptions * dpuOptions = new DPUOptions;
						dpuOptions->dpuMode = 0;
						dpuOptions->outPort = 0;
						dpuOptions->saturation = true;

						(yyval.DPUOptionsType) = dpuOptions;
					  }
#line 2046 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 497 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
						DPUOptions * dpuOptions = new DPUOptions;

						dpuOptions->dpuMode = (yyvsp[-5].integerVal);
						dpuOptions->outPort = (yyvsp[-1].integerVal);
						dpuOptions->saturation = false;

						(yyval.DPUOptionsType) = dpuOptions;
					  }
#line 2060 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 510 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
						StorageOptions * storageOptions = new StorageOptions;

						storageOptions->distribution = sdEvenDist;
						storageOptions->isForIndirectAddress = false;
						storageOptions->variableType = vtIntermediate;

						(yyval.StorageOptionsType) = storageOptions;
					  }
#line 2074 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 521 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
						StorageOptions * storageOptions = new StorageOptions;

//! Yu Yang 2017-08-01
// Use int value to replace custom enum to avoid bison bug for generating C style output
// CHANGE BEGIN
						storageOptions->distribution = (StorageDistribution)(yyvsp[-1].StorageDistributionType);
// CHANGE END

						storageOptions->isForIndirectAddress = false;
						storageOptions->variableType = vtIntermediate;

						(yyval.StorageOptionsType) = storageOptions;
					  }
#line 2093 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 42:
#line 537 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
						StorageOptions * storageOptions = new StorageOptions;

						storageOptions->distribution = sdEvenDist;
						storageOptions->isForIndirectAddress = false;
//! Yu Yang 2017-08-01
// Use int value to replace custom enum to avoid bison bug for generating C style output
// CHANGE BEGIN
						storageOptions->variableType = (VariableType)(yyvsp[-1].VariableTypeE);
// CHANGE END

						(yyval.StorageOptionsType) = storageOptions;
					  }
#line 2111 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 43:
#line 552 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
						StorageOptions * storageOptions = new StorageOptions;

						storageOptions->distribution = sdEvenDist;
						storageOptions->isForIndirectAddress = true;
						storageOptions->variableType = vtIntermediate;

						(yyval.StorageOptionsType) = storageOptions;
					  }
#line 2125 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 44:
#line 564 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {	(yyval.VariableTypeE) = vtInput; }
#line 2131 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 45:
#line 567 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    { (yyval.VariableTypeE) = vtOutput; }
#line 2137 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 46:
#line 571 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    { (yyval.StorageDistributionType) = sdEvenDist; }
#line 2143 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 47:
#line 574 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    { (yyval.StorageDistributionType) = sdEvenDist; }
#line 2149 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 48:
#line 577 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    { (yyval.StorageDistributionType) = sdFullDist; }
#line 2155 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 49:
#line 580 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    { (yyval.StorageDistributionType) = sdCustomDist; }
#line 2161 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 50:
#line 584 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    { (yyval.StorageTypeE) = stRegFile; }
#line 2167 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 51:
#line 587 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    { (yyval.StorageTypeE) = stMemory; }
#line 2173 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 52:
#line 592 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
						(yyval.StatementType) = 0;
					  }
#line 2181 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 53:
#line 596 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        cout<<"\nStatement Type: command_form";

                        (yyval.StatementType) = (yyvsp[0].StatementType);
                        (yyval.StatementType)->lineNumber((yylsp[0]).first_line);
                      }
#line 2192 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 54:
#line 604 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("statement -> sliceName assign_delimiter assignment_pragma\n");
                        
                        SliceName* slice_name = dynamic_cast<SliceName*>((yyvsp[-2].NameType));
                        if(!slice_name){
                         yyerror("non function call expression as statement!!");
                        }
                        
                        if(slice_name->is_function_call==false){
                          yyerror("non function call expression as statement!!");
                        }
                        
                        FunctionCallStatement *function_call_statement = CREATE_OBJECT(FunctionCallStatement);
                        FunctionCall* function_call = CREATE_OBJECT(FunctionCall);
                        function_call_statement->lineNumber((yylsp[-2]).first_line);
                        function_call->lineNumber((yylsp[-2]).first_line);
                        conv_slice_name_to_function_call(slice_name, function_call);
                        function_call_statement->function_call = function_call;
                        int error = setPragmaOfFunctionCallStatement(function_call_statement, (yyvsp[0].PragmaType));

                        if (error)
                          yyerror("Error: improper pragma!");

                        (yyval.StatementType) = function_call_statement;
                      }
#line 2222 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 55:
#line 631 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
						printGrammar("statement -> assignment assign_delimiter assignment_pragma\n");
						//cout<<"\nStatement Type: assignment";

                        int error = setPragmaOfAssignment((yyvsp[-2].StatementType), (yyvsp[0].PragmaType));

                        if (error)
                          yyerror("Error: improper pragma!");

                        (yyval.StatementType) = (yyvsp[-2].StatementType);
                        (yyval.StatementType)->lineNumber((yylsp[-2]).first_line);
                      }
#line 2239 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 56:
#line 645 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("statement -> for_command\n");
                        //cout<<"\nStatement Type: for_command";

                        (yyval.StatementType) = (yyvsp[0].StatementType);
                        (yyval.StatementType)->lineNumber((yylsp[0]).first_line);
                      }
#line 2251 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 57:
#line 654 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("statement -> if_command\n");
                        //cout<<"\nStatement Type: if_command";

                        (yyval.StatementType) = (yyvsp[0].StatementType);
                        (yyval.StatementType)->lineNumber((yylsp[0]).first_line);
                      }
#line 2263 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 58:
#line 663 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        cout<<"\nStatement Type: global_command";

                        (yyval.StatementType) = (yyvsp[0].StatementType);
                        (yyval.StatementType)->lineNumber((yylsp[0]).first_line);
                      }
#line 2274 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 59:
#line 671 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        cout<<"\nStatement Type: while_command";

                        (yyval.StatementType) = (yyvsp[0].StatementType);
                        (yyval.StatementType)->lineNumber((yylsp[0]).first_line);
                      }
#line 2285 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 60:
#line 679 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        cout<<"\nStatement Type: return_command";
                        
                        (yyval.StatementType) = (yyvsp[0].StatementType);
                        (yyval.StatementType)->lineNumber((yylsp[0]).first_line);
                      }
#line 2296 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 61:
#line 686 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("statement -> block_pragma\n");
                        //cout<<"\nStatement Type: block_pragma";

						(yyval.StatementType) = (yyvsp[0].StatementType);
            (yyval.StatementType)->lineNumber((yylsp[0]).first_line);
					  }
#line 2308 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 62:
#line 697 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
						(yyval.StatementType) = (yyvsp[0].StatementType);
            (yyval.StatementType)->lineNumber((yylsp[0]).first_line);
					  }
#line 2317 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 63:
#line 704 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
						PragmaStatement * pragmaStatement = CREATE_OBJECT(PragmaStatement);
            pragmaStatement->lineNumber((yylsp[0]).first_line);
						pragmaStatement->pragma()->blockProgramType(VIR::VirEnumerations::bptResourceSharing);
						pragmaStatement->pragma()->isBlockBegin(true);
						(yyval.StatementType) = pragmaStatement;
					  }
#line 2329 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 64:
#line 712 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
						PragmaStatement * pragmaStatement = CREATE_OBJECT(PragmaStatement);
            pragmaStatement->lineNumber((yylsp[0]).first_line);
						pragmaStatement->pragma()->blockProgramType(VIR::VirEnumerations::bptResourceSharing);
						pragmaStatement->pragma()->isBlockBegin(false);
						(yyval.StatementType) = pragmaStatement;
					  }
#line 2341 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 65:
#line 721 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        // TODO: check what is this one
                      }
#line 2349 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 66:
#line 728 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        // TODO: check what is this one
                      }
#line 2357 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 67:
#line 733 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        // TODO: check what is this one
                      }
#line 2365 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 68:
#line 740 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("expr -> INTEGER (" + to_string((yyvsp[0].integerVal)) + ")");

                        Integer *integer = CREATE_OBJECT(Integer);
                        integer->lineNumber((yylsp[0]).first_line);
                        integer->value = (yyvsp[0].integerVal);
                        (yyval.ExpressionType) = integer;
                       
                      }
#line 2379 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 69:
#line 751 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("expr -> DOUBLE (" + to_string((yyvsp[0].doubleVal)) + ")");
                        yyinsert_comma_in_input(DOUBLE);
                        FloatingPoint *floatingPoint = CREATE_OBJECT(FloatingPoint);
                        floatingPoint->lineNumber((yylsp[0]).first_line);
                        floatingPoint->value = (float)(yyvsp[0].doubleVal);
                        (yyval.ExpressionType) = floatingPoint;
                      }
#line 2392 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 70:
#line 761 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("expr -> IMAGINARY (" + to_string((yyvsp[0].imaginaryVal)) + ")");
                        yyinsert_comma_in_input(IMAGINARY);
                        Complex *complex = CREATE_OBJECT(Complex);
                        complex->lineNumber((yylsp[0]).first_line);
                        complex->iValue = (yyvsp[0].imaginaryVal);
                        // TODO: need grammar correction in order to also get the real part
                        (yyval.ExpressionType) = complex;
                        
                      }
#line 2407 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 71:
#line 773 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("expr -> TEXT");
                        StringLiteral *strLiteral = CREATE_OBJECT(StringLiteral);
                        strLiteral->lineNumber((yylsp[0]).first_line);
                        strLiteral->value.assign((yyvsp[0].text));
                        yyinsert_comma_in_input(TEXT);
                        (yyval.ExpressionType) = strLiteral;
                        
                      }
#line 2421 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 72:
#line 784 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("expr -> '(' parenthesis expr ')'");
                        gBracketDelimiter = (yyvsp[-1].delimiter);
                      }
#line 2430 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 73:
#line 789 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        yyinsert_comma_in_input(')');
                        ParenthesizedExpression *parExpression = CREATE_OBJECT(ParenthesizedExpression);
                        parExpression->lineNumber((yylsp[-4]).first_line);
                        parExpression->value((yyvsp[-2].ExpressionType));
                        (yyval.ExpressionType) = parExpression;
                      }
#line 2442 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 74:
#line 798 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("expr -> reference");
                        (yyval.ExpressionType) = (yyvsp[0].NameType);
                      }
#line 2451 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 75:
#line 804 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("expr -> matrix");
                        (yyval.ExpressionType) = (yyvsp[0].ArrayType);
                      }
#line 2460 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 76:
#line 810 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("expr -> expr EPOWER expr");
                        (yyval.ExpressionType) = CREATE_BINARY_EXPR(botEPower, (yyvsp[-2].ExpressionType), (yyvsp[0].ExpressionType));
                        (yyval.ExpressionType)->lineNumber((yylsp[-2]).first_line);
                      }
#line 2470 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 77:
#line 817 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("expr -> expr MPOWER expr");
                        (yyval.ExpressionType) = CREATE_BINARY_EXPR(botMPower, (yyvsp[-2].ExpressionType), (yyvsp[0].ExpressionType));
                        (yyval.ExpressionType)->lineNumber((yylsp[-2]).first_line);
                      }
#line 2480 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 78:
#line 824 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("expr -> expr TRANSPOSE");
                        yyinsert_comma_in_input(TRANSPOSE);
                        (yyval.ExpressionType) = CREATE_UNARY_EXPR(uotTranspose, (yyvsp[-1].ExpressionType));
                        (yyval.ExpressionType)->lineNumber((yylsp[-1]).first_line);
                      }
#line 2491 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 79:
#line 832 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("expr -> expr CTRANSPOSE");
                        yyinsert_comma_in_input(CTRANSPOSE);
                        (yyval.ExpressionType) = CREATE_UNARY_EXPR(uotCTranspose, (yyvsp[-1].ExpressionType));
                        (yyval.ExpressionType)->lineNumber((yylsp[-1]).first_line);
                      }
#line 2502 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 80:
#line 840 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("expr -> NOT expr");
                        (yyval.ExpressionType) = CREATE_UNARY_EXPR(uotNot, (yyvsp[0].ExpressionType));
                        (yyval.ExpressionType)->lineNumber((yylsp[0]).first_line);
                      }
#line 2512 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 81:
#line 847 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("expr -> '+' expr %prec UNARYPLUS");
                        (yyval.ExpressionType) = CREATE_UNARY_EXPR(uotPlus, (yyvsp[0].ExpressionType));
                        (yyval.ExpressionType)->lineNumber((yylsp[0]).first_line);
                      }
#line 2522 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 82:
#line 854 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("expr -> '-' expr %prec UNARYMINUS");
                        (yyval.ExpressionType) = CREATE_UNARY_EXPR(uotMinus, (yyvsp[0].ExpressionType));
                        (yyval.ExpressionType)->lineNumber((yylsp[0]).first_line);
                      }
#line 2532 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 83:
#line 861 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("expr -> expr MMUL expr");
                        (yyval.ExpressionType) = CREATE_BINARY_EXPR(botMMul, (yyvsp[-2].ExpressionType), (yyvsp[0].ExpressionType));
                        (yyval.ExpressionType)->lineNumber((yylsp[0]).first_line);
                      }
#line 2542 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 84:
#line 868 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("expr -> expr EMUL expr");
                        (yyval.ExpressionType) = CREATE_BINARY_EXPR(botEMul, (yyvsp[-2].ExpressionType), (yyvsp[0].ExpressionType));
                        (yyval.ExpressionType)->lineNumber((yylsp[0]).first_line);
                      }
#line 2552 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 85:
#line 875 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("expr -> expr MDIV expr");
                        (yyval.ExpressionType) = CREATE_BINARY_EXPR(botMDiv, (yyvsp[-2].ExpressionType), (yyvsp[0].ExpressionType));
                        (yyval.ExpressionType)->lineNumber((yylsp[-2]).first_line);
                      }
#line 2562 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 86:
#line 882 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("expr -> expr EDIV expr");
                        (yyval.ExpressionType) = CREATE_BINARY_EXPR(botEDiv, (yyvsp[-2].ExpressionType), (yyvsp[0].ExpressionType));
                        (yyval.ExpressionType)->lineNumber((yylsp[-2]).first_line);
                      }
#line 2572 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 87:
#line 889 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("expr -> expr MLEFTDIV expr");
                        (yyval.ExpressionType) = CREATE_BINARY_EXPR(botMLeftDiv, (yyvsp[-2].ExpressionType), (yyvsp[0].ExpressionType));
                        (yyval.ExpressionType)->lineNumber((yylsp[-2]).first_line);
                      }
#line 2582 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 88:
#line 896 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("expr -> expr ELEFTDIV expr");
                        (yyval.ExpressionType) = CREATE_BINARY_EXPR(botELeftDiv, (yyvsp[-2].ExpressionType), (yyvsp[0].ExpressionType));
                        (yyval.ExpressionType)->lineNumber((yylsp[-2]).first_line);
                      }
#line 2592 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 89:
#line 903 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("expr -> expr '+' expr");
                        (yyval.ExpressionType) = CREATE_BINARY_EXPR(botAdd, (yyvsp[-2].ExpressionType), (yyvsp[0].ExpressionType));
                        (yyval.ExpressionType)->lineNumber((yylsp[-2]).first_line);
                      }
#line 2602 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 90:
#line 910 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("expr -> expr '-' expr");
                        (yyval.ExpressionType) = CREATE_BINARY_EXPR(botSub, (yyvsp[-2].ExpressionType), (yyvsp[0].ExpressionType));
                        (yyval.ExpressionType)->lineNumber((yylsp[-2]).first_line);
                      }
#line 2612 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 91:
#line 917 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("expr -> colon_expr");
                        (yyval.ExpressionType) = (yyvsp[0].RangeExpressionType);
                        (yyval.ExpressionType)->lineNumber((yylsp[0]).first_line);
                      }
#line 2622 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 92:
#line 924 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("expr -> expr LTHAN expr");
                        (yyval.ExpressionType) = CREATE_BINARY_EXPR(botLessThan, (yyvsp[-2].ExpressionType), (yyvsp[0].ExpressionType));
                        (yyval.ExpressionType)->lineNumber((yylsp[-2]).first_line);
                      }
#line 2632 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 93:
#line 931 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("expr -> expr LTHANE expr");
                        (yyval.ExpressionType) = CREATE_BINARY_EXPR(botLessOrEqual, (yyvsp[-2].ExpressionType), (yyvsp[0].ExpressionType));
                        (yyval.ExpressionType)->lineNumber((yylsp[-2]).first_line);
                      }
#line 2642 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 94:
#line 938 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("expr -> expr GTHAN expr");
                        (yyval.ExpressionType) = CREATE_BINARY_EXPR(botGreaterThan, (yyvsp[-2].ExpressionType), (yyvsp[0].ExpressionType));
                        (yyval.ExpressionType)->lineNumber((yylsp[-2]).first_line);
                      }
#line 2652 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 95:
#line 945 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("expr -> expr GTHANE expr");
                        (yyval.ExpressionType) = CREATE_BINARY_EXPR(botGreaterOrEqual, (yyvsp[-2].ExpressionType), (yyvsp[0].ExpressionType));
                        (yyval.ExpressionType)->lineNumber((yylsp[-2]).first_line);
                      }
#line 2662 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 96:
#line 952 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("expr -> expr EQUAL expr");
                        (yyval.ExpressionType) = CREATE_BINARY_EXPR(botEqual, (yyvsp[-2].ExpressionType), (yyvsp[0].ExpressionType));
                        (yyval.ExpressionType)->lineNumber((yylsp[-2]).first_line);
                      }
#line 2672 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 97:
#line 959 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("expr -> expr UNEQUAL expr");
                        (yyval.ExpressionType) = CREATE_BINARY_EXPR(botUnEqual, (yyvsp[-2].ExpressionType), (yyvsp[0].ExpressionType));
                        (yyval.ExpressionType)->lineNumber((yylsp[-2]).first_line);
                      }
#line 2682 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 98:
#line 966 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("expr -> expr AND expr");
                        (yyval.ExpressionType) = CREATE_BINARY_EXPR(botAnd, (yyvsp[-2].ExpressionType), (yyvsp[0].ExpressionType));
                        (yyval.ExpressionType)->lineNumber((yylsp[-2]).first_line);
                      }
#line 2692 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 99:
#line 973 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("expr -> expr OR expr");
                        (yyval.ExpressionType) = CREATE_BINARY_EXPR(botOr, (yyvsp[-2].ExpressionType), (yyvsp[0].ExpressionType));
                        (yyval.ExpressionType)->lineNumber((yylsp[-2]).first_line);
                      }
#line 2702 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 100:
#line 982 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                          printGrammar("parenthesis -> " + to_string(gBracketDelimiter));
                          (yyval.delimiter) = gBracketDelimiter;
                          gBracketDelimiter = '(';
                      }
#line 2712 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 101:
#line 991 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("reference -> identifier");
                        (yyval.NameType) = (yyvsp[0].IdentifierType);
                      }
#line 2721 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 102:
#line 996 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("reference -> identifier '(' ')'");
                        SliceName *sliceName = CREATE_OBJECT(SliceName);
                        sliceName->lineNumber((yylsp[-2]).first_line);
                        sliceName->prefix((yyvsp[-2].IdentifierType));
                        // no parameters inside parenthesis, it must be a function
                        // call.
                        sliceName->is_function_call=true;
                        (yyval.NameType) = sliceName;
                      }
#line 2736 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 103:
#line 1007 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("reference -> identifier '(' parenthesis argument_list ')'");
                        gBracketDelimiter = (yyvsp[-1].delimiter);
                      }
#line 2745 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 104:
#line 1012 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        yyinsert_comma_in_input(')');

                        SliceName *sliceName = CREATE_OBJECT(SliceName);
                        sliceName->lineNumber((yylsp[-5]).first_line);
                        sliceName->prefix((yyvsp[-5].IdentifierType));

                        fillExpressionList((yyvsp[-2].ExpressionLinkerType), &sliceName->suffix());
                        
                        // check whether it's a slice name or function call. Here
                        // we use slice name for both situation, but set a flag
                        // to distinguish slice name and function call.
                        
                        sliceName->is_function_call=true;
                        for(auto n: global_reference_list){
                          if((yyvsp[-5].IdentifierType)->name()==n){
                            sliceName->is_function_call=false;
                          }
                        }
                        
                        (yyval.NameType) = sliceName;
                      }
#line 2772 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 105:
#line 1038 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        Identifier *identifier = CREATE_OBJECT(Identifier);
                        identifier->lineNumber((yylsp[0]).first_line);
                        string str((yyvsp[0].symbol));
                        printGrammar("identifier -> " + str);
                        identifier->name(str);
                        (yyval.IdentifierType) = identifier;
                      }
#line 2785 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 106:
#line 1050 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("argument_list -> ':'");
                        RangeExpression *rangeExpression = CREATE_FULL_RANGE_EXPRESSION;
                        (yyval.ExpressionLinkerType) = new ExpressionLinker(rangeExpression, 0);
                      }
#line 2795 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 107:
#line 1057 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("argument_list -> expr");
                        (yyval.ExpressionLinkerType) = new ExpressionLinker((yyvsp[0].ExpressionType), 0);
                      }
#line 2804 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 108:
#line 1063 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("argument_list -> ':' ',' argument_list");
                        RangeExpression *rangeExpression = CREATE_FULL_RANGE_EXPRESSION;
                        rangeExpression->lineNumber((yylsp[-2]).first_line);
                        (yyval.ExpressionLinkerType) = new ExpressionLinker(rangeExpression, (yyvsp[0].ExpressionLinkerType));
                      }
#line 2815 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 109:
#line 1071 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("argument_list -> expr ',' argument_list");
                        (yyval.ExpressionLinkerType) = new ExpressionLinker((yyvsp[-2].ExpressionType), (yyvsp[0].ExpressionLinkerType));
                      }
#line 2824 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 110:
#line 1079 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        gBracketDelimiter = (yyvsp[-1].delimiter);
                      }
#line 2832 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 1083 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        yyinsert_comma_in_input(']');
                        Array *array = CREATE_OBJECT(Array);
                        array->lineNumber((yylsp[-4]).first_line);
                        fillArray((yyvsp[-2].ArrayLinkerType), &array->elements());
                        (yyval.ArrayType) = array;
                        
                      }
#line 2845 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 1095 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        (yyval.delimiter) = gBracketDelimiter;
                        gBracketDelimiter = '[';
                      }
#line 2854 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 1103 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        // Create an empty array
                        (yyval.ArrayLinkerType) = new ArrayLinker(CREATE_OBJECT(Array), 0, 0);
                      }
#line 2863 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 1109 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        Array *array = CREATE_OBJECT(Array);
                        array->lineNumber((yylsp[0]).first_line);
                        fillArray((yyvsp[0].ArrayLinkerType), &array->elements());
                        (yyval.ArrayLinkerType) = new ArrayLinker(array, 0, 0);
                      }
#line 2874 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 1117 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        (yyval.ArrayLinkerType) = (yyvsp[-1].ArrayLinkerType);
                      }
#line 2882 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 1122 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        Array *array = CREATE_OBJECT(Array);
                        array->lineNumber((yylsp[-2]).first_line);
                        fillArray((yyvsp[0].ArrayLinkerType), &array->elements());
                        ArrayLinker *linker = new ArrayLinker(array, (yyvsp[-2].ArrayLinkerType), 0);
                        (yyvsp[-2].ArrayLinkerType)->nextElement = linker;
                        (yyval.ArrayLinkerType) = linker;
                      }
#line 2895 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 1132 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        (yyval.ArrayLinkerType) = (yyvsp[-1].ArrayLinkerType);
                      }
#line 2903 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 1137 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        Array *array = CREATE_OBJECT(Array);
                        array->lineNumber((yylsp[-2]).first_line);
                        fillArray((yyvsp[0].ArrayLinkerType), &array->elements());
                        ArrayLinker *linker = new ArrayLinker(array, (yyvsp[-2].ArrayLinkerType), 0);
                        (yyvsp[-2].ArrayLinkerType)->nextElement = linker;
                        (yyval.ArrayLinkerType) = linker;
                      }
#line 2916 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 1149 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        (yyval.ArrayLinkerType) = new ArrayLinker((yyvsp[0].ExpressionType), 0, 0);
                      }
#line 2924 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 1154 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        (yyval.ArrayLinkerType) = (yyvsp[0].ArrayLinkerType);
                      }
#line 2932 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 1159 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        ArrayLinker *arrayLinker = new ArrayLinker((yyvsp[0].ExpressionType), (yyvsp[-1].ArrayLinkerType), 0);
                        (yyvsp[-1].ArrayLinkerType)->nextElement = arrayLinker;
                        (yyval.ArrayLinkerType) = arrayLinker;
                      }
#line 2942 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 1168 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        (yyval.ArrayLinkerType) = new ArrayLinker((yyvsp[-1].ExpressionType), 0, 0);
                      }
#line 2950 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 1173 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        ArrayLinker *arrayLinker = new ArrayLinker((yyvsp[-1].ExpressionType), (yyvsp[-2].ArrayLinkerType), 0);
                        (yyvsp[-2].ArrayLinkerType)->nextElement = arrayLinker;
                        (yyval.ArrayLinkerType) = arrayLinker;
                      }
#line 2960 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 124:
#line 1182 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        RangeExpression *rangeExpression = CREATE_OBJECT(RangeExpression);
                        rangeExpression->lineNumber((yylsp[-2]).first_line);
                        rangeExpression->begin((yyvsp[-2].ExpressionType));
                        if (LOOKAHEAD != ':')
                        {
                          rangeExpression->increment(CREATE_INTEGER(1));
                          rangeExpression->end((yyvsp[0].ExpressionType));
                        }
                        else
                        {
                          rangeExpression->increment((yyvsp[0].ExpressionType));
                          rangeExpression->end(0);
                        }

                        (yyval.RangeExpressionType) = rangeExpression;
                      }
#line 2982 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 125:
#line 1200 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        RangeExpression *rangeExpression = CREATE_OBJECT(RangeExpression);
                        rangeExpression->lineNumber((yylsp[-2]).first_line);
                        if ((yyvsp[-2].RangeExpressionType)->end())
                        {
                          rangeExpression->begin((yyvsp[-2].RangeExpressionType)->begin());

                          if (LOOKAHEAD != ':')
                          {
                            rangeExpression->increment(CREATE_INTEGER(1));
                            rangeExpression->end((yyvsp[0].ExpressionType));
                          }
                          else
                          {
                            rangeExpression->increment((yyvsp[0].ExpressionType));
                            rangeExpression->end(0);
                          }
                        }
                        else
                        {
                          rangeExpression->begin((yyvsp[-2].RangeExpressionType)->begin());
                          rangeExpression->increment((yyvsp[-2].RangeExpressionType)->increment());
                          rangeExpression->end((yyvsp[0].ExpressionType));
                        }
                        (yyval.RangeExpressionType) = rangeExpression;
                      }
#line 3013 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 1230 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        AssignmentStatement *assignStatement = CREATE_OBJECT(AssignmentStatement);
                        assignStatement->lineNumber((yylsp[-2]).first_line);
                        assignStatement->lhs().push_back((yyvsp[-2].NameType));
                        
                        // push the name of reference to global list to distinguish
                        // slice name and function call
                        int flag=false;
                        string curr_name;
                        for(auto & m: assignStatement->lhs()){
	                        if(dynamic_cast<Identifier*>(m)){
	                          curr_name = dynamic_cast<Identifier*>(m)->name();
	                        }else if(dynamic_cast<SliceName*>(m)){
	                          curr_name = dynamic_cast<SliceName*>(m)->prefix()->name();
	                        }
	                        for (auto n : global_reference_list){
	                          if(curr_name==n){
	                            flag=true;
	                            break;
	                          }
	                        }
	                        if(flag==false){
	                          global_reference_list.push_back(curr_name);
	                        }
                        }
                        
                        

                        // in case the rhs is a slice name and it's marked as function
                        // call, replace it with a function call.
                        
                        SliceName* slice_name = dynamic_cast<SliceName*>((yyvsp[0].ExpressionType));
                        if(slice_name && slice_name->is_function_call){
                          FunctionCall *function_call = CREATE_OBJECT(FunctionCall);
                          function_call->lineNumber((yylsp[-2]).first_line);
                          conv_slice_name_to_function_call(slice_name, function_call);
                          assignStatement->rhs(function_call);
                        }else{
                          assignStatement->rhs((yyvsp[0].ExpressionType));
                        }
                        assignStatement->pragma(0);

                        (yyval.StatementType) = assignStatement;
                        (yyval.StatementType)->lineNumber((yylsp[-2]).first_line);
                      }
#line 3063 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 1276 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        AssignmentStatement *assignStatement = CREATE_OBJECT(AssignmentStatement);
                        assignStatement->lineNumber((yylsp[-4]).first_line);
                        fillNameList((yyvsp[-3].NameLinkerType), &(assignStatement->lhs()));
                        
                        // push the name of reference to global list to distinguish
                        // slice name and function call
                        int flag=false;
                        string curr_name;
                        for(auto & m: assignStatement->lhs()){
	                        if(dynamic_cast<Identifier*>(m)){
	                          curr_name = dynamic_cast<Identifier*>(m)->name();
	                        }else if(dynamic_cast<SliceName*>(m)){
	                          curr_name = dynamic_cast<SliceName*>(m)->prefix()->name();
	                        }
	                        for (auto n : global_reference_list){
	                          if(curr_name==n){
	                            flag=true;
	                            break;
	                          }
	                        }
	                        if(flag==false){
	                          global_reference_list.push_back(curr_name);
	                        }
                        }
                        
                        

                        // in case the rhs is a slice name and it's marked as function
                        // call, replace it with a function call.
                        
                        SliceName* slice_name = dynamic_cast<SliceName*>((yyvsp[0].ExpressionType));
                        if(slice_name && slice_name->is_function_call){
                          FunctionCall *function_call = CREATE_OBJECT(FunctionCall);
                          function_call->lineNumber((yylsp[-4]).first_line);
                          conv_slice_name_to_function_call(slice_name, function_call);
                          assignStatement->rhs(function_call);
                        }else{
                          assignStatement->rhs((yyvsp[0].ExpressionType));
                        }
                        assignStatement->pragma(0);
                        (yyval.StatementType) = assignStatement;
                      }
#line 3111 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 1320 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        //$$ = build_expr_binary_op(ASSIGNMENTexpr, build_expr_MATRIX($1), $3);
                      }
#line 3119 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 1325 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        //$$ = build_expr_binary_op(ASSIGNMENTexpr, build_expr_MATRIX($1), $3);
                      }
#line 3127 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 1332 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        gBracketDelimiter = (yyvsp[-1].delimiter);
                      }
#line 3135 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 1336 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        /*
                        expr_t** const singletons = (expr_t**)
                        alloc_pointers(1);
                        *singletons = $3;
                        vector_t** const vectors = (vector_t**) alloc_pointers(1);
                        vectors[0] = build_vector_t(1, singletons);
                        $$ = build_matrix_t(1, vectors);
                        */
                      }
#line 3150 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 1350 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        gBracketDelimiter = (yyvsp[-3].delimiter);
                      }
#line 3158 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 1354 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        /*
                        var_expr_t* varExpr;
                        int length;

                        // TODO: seems that it should be removed.
                        for (length = 1, varExpr = $5; VAR_EXPR_NEXT(varExpr); length++, SET_TO_NEXT(varExpr));
                        expr_t** const singletons = (expr_t**) alloc_pointers(length+1);
                        *singletons = $3;

                        for (length = 1, varExpr = $5; VAR_EXPR_NEXT(varExpr); length++, SET_TO_NEXT(varExpr))
                        {
                          *(singletons+length) = VAR_EXPR_DATA(varExpr);
                        }
                        *(singletons+length) = VAR_EXPR_DATA(varExpr);
                        vector_t** const vectors = (vector_t**) alloc_pointers(1);
                        *vectors = build_vector_t(length+1, singletons);
                        $$ = build_matrix_t(1, vectors);
                        dealloc_var_expr_t_list($5);
                        */
                      }
#line 3184 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 1379 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                          (yyval.delimiter) = gBracketDelimiter;
                          gBracketDelimiter = LD;
                      }
#line 3193 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 1387 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        (yyval.NameLinkerType) = new NameLinker((yyvsp[0].NameType), 0);
                      }
#line 3201 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 136:
#line 1391 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        (yyval.NameLinkerType) = new NameLinker((yyvsp[-2].NameType), (yyvsp[0].NameLinkerType));
                      }
#line 3209 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 137:
#line 1398 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        (yyval.StatementType) = (yyvsp[-1].StatementType);
                      }
#line 3217 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 1405 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        ForStatement *forStatement = CREATE_OBJECT(ForStatement);
                        forStatement->lineNumber((yylsp[-4]).first_line);
                        forStatement->loopVariable((yyvsp[-4].IdentifierType));
                        forStatement->loopRange((yyvsp[-2].RangeExpressionType));
//! Yu Yang 2017-08-01
// Use int value to replace custom enum to avoid bison bug for generating C style output
// CHANGE BEGIN
                        forStatement->parallelMode((ParallelMode)(yyvsp[-1].ParallelModeType));
// CHANGE END

                        fillStatementList((yyvsp[0].StatementLinkerType), &forStatement->loopBody());
                        (yyval.StatementType) = forStatement;
                      }
#line 3236 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 1423 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        (yyval.ParallelModeType) = pmSequential;
                      }
#line 3244 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 1428 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        (yyval.ParallelModeType) = pmParallel;
                      }
#line 3252 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 141:
#line 1435 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        (yyval.StatementType) = (yyvsp[-1].StatementType);
                      }
#line 3260 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 1442 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        IfStatement *ifStatement;

                        if ((yyvsp[0].StatementLinkerType) == 0) 
                        {
                          ifStatement = CREATE_OBJECT(IfStatement);
                        }
                        else
                        {
                          ifStatement = CREATE_OBJECT(IfThenElseStatement);
                          fillStatementList((yyvsp[0].StatementLinkerType), &((IfThenElseStatement*)ifStatement)->elsePart());
                        }
                        ifStatement->lineNumber((yylsp[-3]).first_line);
                        ifStatement->condition((yyvsp[-3].ExpressionType));

						// pragma part
						if ((yyvsp[-2].DPUPragmaType) == 0) // It is an structural if statement
						{
							ifStatement->mode(imStructural);
							ifStatement->conditionPragma(0);
						}
						else
						{
							ifStatement->mode(imConditional);
							ifStatement->conditionPragma((yyvsp[-2].DPUPragmaType));
						}

                        fillStatementList((yyvsp[-1].StatementLinkerType), &ifStatement->thenPart());

                        (yyval.StatementType) = ifStatement;
                      }
#line 3296 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 1477 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    { 
						// conditional mode (using DPU[0,0] as the processing unit)
                        
						printGrammar("DPU '[' argument_list ']'");
                        //cout << "\nDPU grammar";

                        DPUPragma * dpuPragma = CREATE_OBJECT(DPUPragma);
                        dpuPragma->lineNumber((yylsp[0]).first_line);

						dpuPragma->reference().push_back(CREATE_INTEGER(0)); // row value
						dpuPragma->reference().push_back(CREATE_INTEGER(0)); // col value

                        (yyval.DPUPragmaType) = dpuPragma;
                      }
#line 3315 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 1493 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    { 
                        (yyval.DPUPragmaType) = (yyvsp[-1].DPUPragmaType);
                      }
#line 3323 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 1499 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    { 
						// structural mode
						// return 0 in order to show that it is an structural if statement
						(yyval.DPUPragmaType) = 0;
					  }
#line 3333 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 1505 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        printGrammar("DPU '[' argument_list ']'");
                        //cout << "\nDPU grammar";

                        DPUPragma * dpuPragma = CREATE_OBJECT(DPUPragma);
                        dpuPragma->lineNumber((yylsp[-3]).first_line);
                        fillExpressionList((yyvsp[-1].ExpressionLinkerType), &dpuPragma->reference());

                        (yyval.DPUPragmaType) = dpuPragma;
                        
					  }
#line 3349 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 1519 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        (yyval.StatementLinkerType) = 0;
                      }
#line 3357 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 1524 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        (yyval.StatementLinkerType) = (yyvsp[0].StatementLinkerType);
                      }
#line 3365 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 1529 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        IfStatement *ifStatement;

                        if ((yyvsp[0].StatementLinkerType) == 0) 
                        {
                          ifStatement = CREATE_OBJECT(IfStatement);
                        }
                        else
                        {
                          ifStatement = CREATE_OBJECT(IfThenElseStatement);
                          fillStatementList((yyvsp[0].StatementLinkerType), &((IfThenElseStatement*)ifStatement)->elsePart());
                        }
                        ifStatement->lineNumber((yylsp[-4]).first_line);
                        ifStatement->condition((yyvsp[-3].ExpressionType));

						// pragma part
						if ((yyvsp[-2].DPUPragmaType) == 0) // It is an structural if statement
						{
							ifStatement->mode(imStructural);
							ifStatement->conditionPragma(0);
						}
						else
						{
							ifStatement->mode(imConditional);
							ifStatement->conditionPragma((yyvsp[-2].DPUPragmaType));
						}
            ifStatement->lineNumber((yylsp[-4]).first_line);

                        fillStatementList((yyvsp[-1].StatementLinkerType), &ifStatement->thenPart());

                        (yyval.StatementLinkerType) = new StatementLinker(ifStatement, 0);

                      }
#line 3403 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 1566 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        /*
                        global_t* const globalSt =
                        alloc_global_t();

                        GLOBAL_LENGTH(globalSt) = $2.size;
                        GLOBAL_ENTRIES(globalSt) = $2.entries;

                        statement_t* const statement = alloc_statement_t(GLOBALstatement);

                        STATEMENT_GLOBAL(statement) = globalSt;
                        GLOBAL_PARENT(globalSt) = statement;

                        $$ = statement;
                        */
                      }
#line 3424 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 1586 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        /*
                        $$.entries = (char**) alloc_pointers(1);

                        $$.size = 1;

                        *($$.entries+$$.size-1) = strcpy(create_string(strlen($1)), $1);
                        */
                      }
#line 3438 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 1597 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        /*
                        $$.entries = (char**) realloc_pointers((void*)($1.entries), $1.size+1);

                        $$.size = $1.size+1;

                        *($$.entries+$$.size-1) = strcpy(create_string(strlen($2)), $2);
                        */
                      }
#line 3452 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 1610 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        (yyval.StatementType) = (yyvsp[-1].StatementType);
                      }
#line 3460 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 1617 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        /*
                        statement_t* const statement = alloc_statement_t((statement_type)WHILEstatement);

                        while_t* const whileSt = alloc_while_t();

                        STATEMENT_WHILE(statement) = whileSt;

                        WHILE_CONDITION(whileSt) = $1;
                        WHILE_PARENT(whileSt) = statement;

                        WHILE_BODY(whileSt) = $2;

                        set_owner_of_list(statement, $2);

                        EXPR_PARENT_STATEMENT($1) = statement;

                        $$ = statement;
                        */
                      }
#line 3485 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 1641 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        /*
                        statement_t* const statement = alloc_statement_t((statement_type)RETURNstatement);

                        return_t* const returnSt = alloc_return_t();

                        STATEMENT_RETURN(statement) = returnSt;

                        $$ = statement;
                        */
                      }
#line 3501 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 1655 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        FunctionName = (yyvsp[-1].symbol);
                        //$$ = $3;
                      }
#line 3510 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 1661 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        FunctionName = (yyvsp[-1].symbol);
                        //$$ = $2;
                      }
#line 3519 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 1669 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        /*
						            extern file* mfile;

                        cpp_set_file(mfile);
                        cpp_function($4, 0, $2, 0);
                        cpp_set_args($5);

                        $$ = $4;

                        dealloc_var_expr_t_list($5);
                        */
                      }
#line 3537 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 1684 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        /*
                        extern file* mfile;

                        cpp_set_file(mfile);
                        cpp_function($2, 0, 0, 0);
                        cpp_set_args($3);

                        $$ = $2;

                        dealloc_var_expr_t_list($3);
                        */
                      }
#line 3555 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 1701 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        //$$ = alloc_var_expr_t($1);
                      }
#line 3563 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 1706 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        (yyval.vexpression) = (yyvsp[-1].vexpression);
                      }
#line 3571 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 1713 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        (yyval.vexpression) = 0;
                      }
#line 3579 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 1717 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        (yyval.vexpression) = 0;
                      }
#line 3587 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 1721 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        (yyval.vexpression) = (yyvsp[-1].vexpression);
                      }
#line 3595 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 1728 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        /*
                        var_expr_t* const varExpr = alloc_var_expr_t($1);

                        VAR_EXPR_NEXT(varExpr) = $3;
                        VAR_EXPR_PREV($3) = varExpr;

                        $$ = varExpr;
                        */
                      }
#line 3610 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 1740 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        //$$ = alloc_var_expr_t($1);
                      }
#line 3618 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 1747 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        //cpp_set_stmt_list((const statement_t*)$2);
                        //$$ = $2;
                      }
#line 3627 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 1753 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                        //cpp_set_stmt_list((const statement_t*)0);
                        (yyval.StatementType) = 0;
                      }
#line 3636 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 1761 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1646  */
    {
                         yyerror("Lexical error!");
                       }
#line 3644 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
    break;


#line 3648 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.tab.cpp" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }

  yyerror_range[1] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, &yylloc);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  yyerror_range[1] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
  *++yylsp = yyloc;

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, yylsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 1769 "/home/herenvarno/Projects/gitr/SiLago/vesyla/src/VParser/vparser.y" /* yacc.c:1906  */


static void yyerror(const char* message)
{
  reportError(message);

  exit(1);
}

/*
;;; Local Variables: ***
;;; mode: Fundamental ***
;;; End: ***
*/
