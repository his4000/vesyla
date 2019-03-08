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


%{

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
  bool en_compression;
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

%}


%start input

//! Yu Yang 2017-08-01
// Use int value to replace custom enum to avoid bison bug for generating C style output
// CHANGE BEGIN
%union  {
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
        }
// CHANGE END

// ----------------------------------------------------
// tokens  (terminal symbols)

%token LEXERROR
%token LINE
%token LD RD
%token FOR
%token END
%token IF ELSEIF ELSE
%token GLOBAL
%token WHILE
%token FUNCTION
%token RETURN
%token PRAGMASTART
%token PARALLEL
%token STRUCTURAL
%token DPU RFILE MEMORY CDPU
%token EVENDIST FULLDIST CUSTOMDIST COMPRESS
%token DPUMODE DPUOUT DPU_SATURATION  ADDRESS_REG
%token VAR_INPUT VAR_OUTPUT
%token RACCU_VAR ADDRESS_VAR TEMP_VAR MEM_ADDRESS
%token CONSTROW CONSTCOL CONSTDIMENSION CONSTPARALLELISM
%token RESOURCE_SHARING_BEGIN RESOURCE_SHARING_END

%token <text> TEXT
%token <symbol> IDENTIFIER
%token <integerVal> INTEGER
%token <doubleVal> DOUBLE
%token <imaginaryVal> IMAGINARY

// ----------------------------------------------------
// types  (nonterminal symbols)

%type <FunctionDeclarationType> input functionMFile scriptMFile

%type <StatementLinkerType> statement_list
%type <StatementLinkerType> opt_else

%type <ParallelModeType> for_pragma
%type <DPUPragmaType> if_pragma if_pragma_expr dpu_object
%type <DPUOptionsType> dpu_options
%type <PragmaType> assignment_pragma assignment_pragma_node
%type <StorageDistributionType> storage_dist
%type <VariableTypeE> variable_type
%type <StorageOptionsType> storage_options
%type <StorageTypeE> storage_type
%type <RaccuOptionsType> raccu_options_type

%type <StatementType> assignment
%type <StatementType> statement
%type <StatementType> command_form
%type <StatementType> for_command for_cmd_list
%type <StatementType> if_command if_cmd_list
%type <StatementType> global_command
%type <StatementType> while_command while_cmd_list
%type <StatementType> return_command
%type <StatementType> block_pragma block_pragma_node

%type <ExpressionType> expr
%type <NameType> reference
%type <IdentifierType> identifier
%type <RangeExpressionType> colon_expr

%type <ExpressionLinkerType> argument_list

%type <delimiter> parenthesis boxes1 boxes2

%type <vexpression> text_list
%type <NameLinkerType> reference_list

%type <variables> global_decl_list

%type <ArrayType> matrix s_assignee_matrix m_assignee_matrix

%type <ArrayLinkerType> rows
%type <ArrayLinkerType> row row_with_commas

%type <symbol> f_def_line
%type <StatementType> f_body
%type <vexpression> f_output f_input f_argument_list

// ----------------------------------------------------
// left

%left AND OR
%left LTHAN LTHANE GTHAN GTHANE EQUAL UNEQUAL
%left ':'xo
%left '-' '+'
%left MMUL EMUL MDIV EDIV MLEFTDIV ELEFTDIV
%left EPOWER MPOWER


// ----------------------------------------------------
// nonassoc

%nonassoc NOT UNARYPLUS UNARYMINUS
%nonassoc TRANSPOSE CTRANSPOSE

// Line numbers
%locations


%%


input                 : scriptMFile
                      {
                        $$ = root = $1;
                      }

                      | functionMFile
                      {
                        $$ = root = $1;
                      }

                      | parse_error
                      {
                        $$ = root = 0;
                      }
                      ;


scriptMFile           : 
                      opt_delimiter statement_list
                      {
                        printGrammar("scriptMFile -> opt_delimiter statement_list", true);
                        MainProgram *mainProgram = CREATE_OBJECT(MainProgram);
                        mainProgram->lineNumber(@1.first_line);
                        mainProgram->name("__main");
                        fillStatementList($2, &mainProgram->body());
                        $$ = mainProgram;
                      }
                      ;


opt_delimiter         :
                      | delimiter
                      ;


delimiter             : null_lines
                      | empty_lines
                      | null_lines empty_lines
                      ;


null_lines            : null_line
                      | null_lines null_line
                      ;


null_line             : ','
                      | ';'
                      | empty_lines ','
                      | empty_lines ';'
                      ;


empty_lines           : LINE
                      | empty_lines LINE
                      ;

assign_delimiter	  :
					  | ';'
                      | empty_lines ';'
					  ;

statement_list        :
					  statement
					  {
                        printGrammar("statement_list -> statement");
                        $$ = new StatementLinker($1, 0);
					  }
					  | statement empty_lines
                      {
                        printGrammar("statement_list -> statement empty_lines");

                        $$ = new StatementLinker($1, 0);
                      }
					  | statement empty_lines statement_list
					  {
                        printGrammar("statement_list -> statement empty_lines statement_list");

                        $$ = new StatementLinker($1, $3);
					  }
                      ;

assignment_pragma         :
                      {
                        $$ = 0;
                      }
                      | PRAGMASTART assignment_pragma_node
                      {
                        $$ = $2;
                      }
                      ;

assignment_pragma_node     :
                      dpu_object dpu_options '[' argument_list ']'
                      {
						printGrammar("C/DPU '[' argument_list ']'");
						//cout << "\nC/DPU grammar";

                        DPUPragma * dpuPragma = $1;

						dpuPragma->outPort = $2->outPort;
						dpuPragma->saturation = $2->saturation;
						
						switch ($2->dpuMode)
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

						delete $2;

                        fillExpressionList($4, &dpuPragma->reference());

                        $$ = dpuPragma;
                      }

                      | storage_type storage_options '[' argument_list ']'
                      {
                        printGrammar("RFILE '[' argument_list ']'");
                        //cout << "\nRFILE grammar";

                        StoragePragma * storagePragma = CREATE_OBJECT(StoragePragma);
                        storagePragma->lineNumber(@1.first_line);
//! Yu Yang 2017-08-01
// Use int value to replace custom enum to avoid bison bug for generating C style output
// CHANGE BEGIN
						storagePragma->storageType((StorageType)$1);
// CHANGE END
                        storagePragma->distribution($2->distribution);
                        storagePragma->en_compression = $2->en_compression;
						storagePragma->isForIndirectAddress($2->isForIndirectAddress);
						storagePragma->variableType($2->variableType);

                        fillExpressionList($4, &storagePragma->reference());

                        $$ = storagePragma;
                      }

					  | RACCU_VAR raccu_options_type
					  {
						RaccuPragma * raccuPragma = CREATE_OBJECT(RaccuPragma);
            raccuPragma->lineNumber(@1.first_line);
//! Yu Yang 2017-08-01
// Use int value to replace custom enum to avoid bison bug for generating C style output
// CHANGE BEGIN
						raccuPragma->raccuOptions = (RaccuOptions)$2;
// CHANGE END

						$$ = raccuPragma;
					  }

					  | ADDRESS_VAR
					  {
						$$ = CREATE_OBJECT(AddressPragma);
            $$->lineNumber(@1.first_line);
					  }

					  | TEMP_VAR
					  {
						$$ = CREATE_OBJECT(TempVarPragma);
            $$->lineNumber(@1.first_line);
					  }
						;

raccu_options_type	  :
					  {
						$$ = VIR::VirEnumerations::roNone;
					  }
					  | MEM_ADDRESS
					  {
						$$ = VIR::VirEnumerations::roMemoryAddress;
					  }
					  ;

dpu_object			  : DPU
					  { $$ = CREATE_OBJECT(DPUPragma);
            $$->lineNumber(@1.first_line);}
					  | CDPU
					  { $$ = CREATE_OBJECT(DPUChainPragma);
            $$->lineNumber(@1.first_line);}
					  ;

dpu_options			  :
					  { 
						DPUOptions * dpuOptions = new DPUOptions;
						dpuOptions->dpuMode = 0;
						dpuOptions->outPort = 0;
						dpuOptions->saturation = false;

						$$ = dpuOptions;
					  }

					  | '<' '>'
					  {
						DPUOptions * dpuOptions = new DPUOptions;
						dpuOptions->dpuMode = 0;
						dpuOptions->outPort = 0;
						dpuOptions->saturation = false;

						$$ = dpuOptions;
					  }

					  | '<' DPU_SATURATION '>'
					  {
						DPUOptions * dpuOptions = new DPUOptions;
						dpuOptions->dpuMode = 0;
						dpuOptions->outPort = 0;
						dpuOptions->saturation = true;

						$$ = dpuOptions;
					  }

					  | '<' DPUMODE '=' INTEGER ',' DPUOUT '=' INTEGER '>'
					  {
						DPUOptions * dpuOptions = new DPUOptions;

						dpuOptions->dpuMode = $4;
						dpuOptions->outPort = $8;
						dpuOptions->saturation = false;

						$$ = dpuOptions;
					  }
					  ;

storage_options		  :

					  {
						StorageOptions * storageOptions = new StorageOptions;

						storageOptions->distribution = sdEvenDist;
						storageOptions->isForIndirectAddress = false;
						storageOptions->variableType = vtIntermediate;

						$$ = storageOptions;
					  }

					  | '<' storage_dist '>'
                      {
						StorageOptions * storageOptions = new StorageOptions;
            storageOptions->en_compression = false;
            if($2 == sdCompress){
              storageOptions->distribution = sdFullDist;
              storageOptions->en_compression = true;
            }else{
						storageOptions->distribution = (StorageDistribution)$2;
            }

						storageOptions->isForIndirectAddress = false;
						storageOptions->variableType = vtIntermediate;

						$$ = storageOptions;
					  }

					  | '<' variable_type '>'
                      {
						StorageOptions * storageOptions = new StorageOptions;

						storageOptions->distribution = sdEvenDist;
						storageOptions->isForIndirectAddress = false;
//! Yu Yang 2017-08-01
// Use int value to replace custom enum to avoid bison bug for generating C style output
// CHANGE BEGIN
						storageOptions->variableType = (VariableType)$2;
// CHANGE END

						$$ = storageOptions;
					  }

					  | '<' ADDRESS_REG '>'
					  {
						StorageOptions * storageOptions = new StorageOptions;

						storageOptions->distribution = sdEvenDist;
						storageOptions->isForIndirectAddress = true;
						storageOptions->variableType = vtIntermediate;

						$$ = storageOptions;
					  }
					  ;

variable_type		  : VAR_INPUT
					  {	$$ = vtInput; }
					  
					  | VAR_OUTPUT
					  { $$ = vtOutput; }
					  ;

storage_dist          :
                      { $$ = sdEvenDist; }

                      | EVENDIST
                      { $$ = sdEvenDist; }

                      | FULLDIST
                      { $$ = sdFullDist; }

                      | CUSTOMDIST
                      { $$ = sdCustomDist; }

                      | COMPRESS
                      { $$ = sdCompress; }
                      ;

storage_type		  : RFILE
					  { $$ = stRegFile; }

					  | MEMORY
					  { $$ = stMemory; }
					  ;


statement             :
					  {
						$$ = 0;
					  }
					  | command_form
                      {
                        cout<<"\nStatement Type: command_form";

                        $$ = $1;
                        $$->lineNumber(@1.first_line);
                      }

                      | reference assign_delimiter assignment_pragma
                      {
                        printGrammar("statement -> sliceName assign_delimiter assignment_pragma\n");
                        
                        SliceName* slice_name = dynamic_cast<SliceName*>($1);
                        if(!slice_name){
                         yyerror("non function call expression as statement!!");
                        }
                        
                        if(slice_name->is_function_call==false){
                          yyerror("non function call expression as statement!!");
                        }
                        
                        FunctionCallStatement *function_call_statement = CREATE_OBJECT(FunctionCallStatement);
                        FunctionCall* function_call = CREATE_OBJECT(FunctionCall);
                        function_call_statement->lineNumber(@1.first_line);
                        function_call->lineNumber(@1.first_line);
                        conv_slice_name_to_function_call(slice_name, function_call);
                        function_call_statement->function_call = function_call;
                        int error = setPragmaOfFunctionCallStatement(function_call_statement, $3);

                        if (error)
                          yyerror("Error: improper pragma!");

                        $$ = function_call_statement;
                      }

                      | assignment assign_delimiter assignment_pragma
                      {
						printGrammar("statement -> assignment assign_delimiter assignment_pragma\n");
						//cout<<"\nStatement Type: assignment";

                        int error = setPragmaOfAssignment($1, $3);

                        if (error)
                          yyerror("Error: improper pragma!");

                        $$ = $1;
                        $$->lineNumber(@1.first_line);
                      }

                      | for_command
                      {
                        printGrammar("statement -> for_command\n");
                        //cout<<"\nStatement Type: for_command";

                        $$ = $1;
                        $$->lineNumber(@1.first_line);
                      }

                      | if_command
                      {
                        printGrammar("statement -> if_command\n");
                        //cout<<"\nStatement Type: if_command";

                        $$ = $1;
                        $$->lineNumber(@1.first_line);
                      }

                      | global_command
                      {
                        cout<<"\nStatement Type: global_command";

                        $$ = $1;
                        $$->lineNumber(@1.first_line);
                      }

                      | while_command
                      {
                        cout<<"\nStatement Type: while_command";

                        $$ = $1;
                        $$->lineNumber(@1.first_line);
                      }

                      | return_command
                      {
                        cout<<"\nStatement Type: return_command";
                        
                        $$ = $1;
                        $$->lineNumber(@1.first_line);
                      }
					  | block_pragma
					  {
                        printGrammar("statement -> block_pragma\n");
                        //cout<<"\nStatement Type: block_pragma";

						$$ = $1;
            $$->lineNumber(@1.first_line);
					  }
                      ;


block_pragma		  : PRAGMASTART block_pragma_node
					  {
						$$ = $2;
            $$->lineNumber(@2.first_line);
					  }
					  ;

block_pragma_node	  : RESOURCE_SHARING_BEGIN
					  {
						PragmaStatement * pragmaStatement = CREATE_OBJECT(PragmaStatement);
            pragmaStatement->lineNumber(@1.first_line);
						pragmaStatement->pragma()->blockProgramType(VIR::VirEnumerations::bptResourceSharing);
						pragmaStatement->pragma()->isBlockBegin(true);
						$$ = pragmaStatement;
					  }
					  | RESOURCE_SHARING_END
					  {
						PragmaStatement * pragmaStatement = CREATE_OBJECT(PragmaStatement);
            pragmaStatement->lineNumber(@1.first_line);
						pragmaStatement->pragma()->blockProgramType(VIR::VirEnumerations::bptResourceSharing);
						pragmaStatement->pragma()->isBlockBegin(false);
						$$ = pragmaStatement;
					  }

command_form          : identifier text_list
                      {
                        // TODO: check what is this one
                      }
                      ;


text_list             : TEXT
                      {
                        // TODO: check what is this one
                      }

                      | text_list TEXT
                      {
                        // TODO: check what is this one
                      }
                      ;


expr                  : INTEGER
                      {
                        printGrammar("expr -> INTEGER (" + to_string($1) + ")");

                        Integer *integer = CREATE_OBJECT(Integer);
                        integer->lineNumber(@1.first_line);
                        integer->value = $1;
                        $$ = integer;
                       
                      }

                      | DOUBLE
                      {
                        printGrammar("expr -> DOUBLE (" + to_string($1) + ")");
                        yyinsert_comma_in_input(DOUBLE);
                        FloatingPoint *floatingPoint = CREATE_OBJECT(FloatingPoint);
                        floatingPoint->lineNumber(@1.first_line);
                        floatingPoint->value = (float)$1;
                        $$ = floatingPoint;
                      }

                      | IMAGINARY
                      {
                        printGrammar("expr -> IMAGINARY (" + to_string($1) + ")");
                        yyinsert_comma_in_input(IMAGINARY);
                        Complex *complex = CREATE_OBJECT(Complex);
                        complex->lineNumber(@1.first_line);
                        complex->iValue = $1;
                        // TODO: need grammar correction in order to also get the real part
                        $$ = complex;
                        
                      }

                      | TEXT
                      {
                        printGrammar("expr -> TEXT");
                        StringLiteral *strLiteral = CREATE_OBJECT(StringLiteral);
                        strLiteral->lineNumber(@1.first_line);
                        strLiteral->value.assign($1);
                        yyinsert_comma_in_input(TEXT);
                        $$ = strLiteral;
                        
                      }

                      | '(' parenthesis expr
                      {
                        printGrammar("expr -> '(' parenthesis expr ')'");
                        gBracketDelimiter = $2;
                      }
                        ')'
                      {
                        yyinsert_comma_in_input(')');
                        ParenthesizedExpression *parExpression = CREATE_OBJECT(ParenthesizedExpression);
                        parExpression->lineNumber(@1.first_line);
                        parExpression->value($3);
                        $$ = parExpression;
                      }

                      | reference
                      {
                        printGrammar("expr -> reference");
                        $$ = $1;
                      }

                      | matrix
                      {
                        printGrammar("expr -> matrix");
                        $$ = $1;
                      }

                      | expr EPOWER expr
                      {
                        printGrammar("expr -> expr EPOWER expr");
                        $$ = CREATE_BINARY_EXPR(botEPower, $1, $3);
                        $$->lineNumber(@1.first_line);
                      }

                      | expr MPOWER expr
                      {
                        printGrammar("expr -> expr MPOWER expr");
                        $$ = CREATE_BINARY_EXPR(botMPower, $1, $3);
                        $$->lineNumber(@1.first_line);
                      }

                      | expr TRANSPOSE
                      {
                        printGrammar("expr -> expr TRANSPOSE");
                        yyinsert_comma_in_input(TRANSPOSE);
                        $$ = CREATE_UNARY_EXPR(uotTranspose, $1);
                        $$->lineNumber(@1.first_line);
                      }

                      | expr CTRANSPOSE
                      {
                        printGrammar("expr -> expr CTRANSPOSE");
                        yyinsert_comma_in_input(CTRANSPOSE);
                        $$ = CREATE_UNARY_EXPR(uotCTranspose, $1);
                        $$->lineNumber(@1.first_line);
                      }

                      | NOT expr
                      {
                        printGrammar("expr -> NOT expr");
                        $$ = CREATE_UNARY_EXPR(uotNot, $2);
                        $$->lineNumber(@2.first_line);
                      }

                      | '+' expr %prec UNARYPLUS
                      {
                        printGrammar("expr -> '+' expr %prec UNARYPLUS");
                        $$ = CREATE_UNARY_EXPR(uotPlus, $2);
                        $$->lineNumber(@2.first_line);
                      }

                      | '-' expr %prec UNARYMINUS
                      {
                        printGrammar("expr -> '-' expr %prec UNARYMINUS");
                        $$ = CREATE_UNARY_EXPR(uotMinus, $2);
                        $$->lineNumber(@2.first_line);
                      }

                      | expr MMUL expr
                      {
                        printGrammar("expr -> expr MMUL expr");
                        $$ = CREATE_BINARY_EXPR(botMMul, $1, $3);
                        $$->lineNumber(@3.first_line);
                      }

                      | expr EMUL expr
                      {
                        printGrammar("expr -> expr EMUL expr");
                        $$ = CREATE_BINARY_EXPR(botEMul, $1, $3);
                        $$->lineNumber(@3.first_line);
                      }

                      | expr MDIV expr
                      {
                        printGrammar("expr -> expr MDIV expr");
                        $$ = CREATE_BINARY_EXPR(botMDiv, $1, $3);
                        $$->lineNumber(@1.first_line);
                      }

                      | expr EDIV expr
                      {
                        printGrammar("expr -> expr EDIV expr");
                        $$ = CREATE_BINARY_EXPR(botEDiv, $1, $3);
                        $$->lineNumber(@1.first_line);
                      }

                      | expr MLEFTDIV expr
                      {
                        printGrammar("expr -> expr MLEFTDIV expr");
                        $$ = CREATE_BINARY_EXPR(botMLeftDiv, $1, $3);
                        $$->lineNumber(@1.first_line);
                      }

                      | expr ELEFTDIV expr
                      {
                        printGrammar("expr -> expr ELEFTDIV expr");
                        $$ = CREATE_BINARY_EXPR(botELeftDiv, $1, $3);
                        $$->lineNumber(@1.first_line);
                      }

                      | expr '+' expr
                      {
                        printGrammar("expr -> expr '+' expr");
                        $$ = CREATE_BINARY_EXPR(botAdd, $1, $3);
                        $$->lineNumber(@1.first_line);
                      }

                      | expr '-' expr
                      {
                        printGrammar("expr -> expr '-' expr");
                        $$ = CREATE_BINARY_EXPR(botSub, $1, $3);
                        $$->lineNumber(@1.first_line);
                      }

                      | colon_expr
                      {
                        printGrammar("expr -> colon_expr");
                        $$ = $1;
                        $$->lineNumber(@1.first_line);
                      }

                      | expr LTHAN expr
                      {
                        printGrammar("expr -> expr LTHAN expr");
                        $$ = CREATE_BINARY_EXPR(botLessThan, $1, $3);
                        $$->lineNumber(@1.first_line);
                      }

                      | expr LTHANE expr
                      {
                        printGrammar("expr -> expr LTHANE expr");
                        $$ = CREATE_BINARY_EXPR(botLessOrEqual, $1, $3);
                        $$->lineNumber(@1.first_line);
                      }

                      | expr GTHAN expr
                      {
                        printGrammar("expr -> expr GTHAN expr");
                        $$ = CREATE_BINARY_EXPR(botGreaterThan, $1, $3);
                        $$->lineNumber(@1.first_line);
                      }

                      | expr GTHANE expr
                      {
                        printGrammar("expr -> expr GTHANE expr");
                        $$ = CREATE_BINARY_EXPR(botGreaterOrEqual, $1, $3);
                        $$->lineNumber(@1.first_line);
                      }

                      | expr EQUAL expr
                      {
                        printGrammar("expr -> expr EQUAL expr");
                        $$ = CREATE_BINARY_EXPR(botEqual, $1, $3);
                        $$->lineNumber(@1.first_line);
                      }

                      | expr UNEQUAL expr
                      {
                        printGrammar("expr -> expr UNEQUAL expr");
                        $$ = CREATE_BINARY_EXPR(botUnEqual, $1, $3);
                        $$->lineNumber(@1.first_line);
                      }

                      | expr AND expr
                      {
                        printGrammar("expr -> expr AND expr");
                        $$ = CREATE_BINARY_EXPR(botAnd, $1, $3);
                        $$->lineNumber(@1.first_line);
                      }

                      | expr OR expr
                      {
                        printGrammar("expr -> expr OR expr");
                        $$ = CREATE_BINARY_EXPR(botOr, $1, $3);
                        $$->lineNumber(@1.first_line);
                      }
                      ;


parenthesis           :
                      {
                          printGrammar("parenthesis -> " + to_string(gBracketDelimiter));
                          $$ = gBracketDelimiter;
                          gBracketDelimiter = '(';
                      }
                      ;


reference             : identifier
                      {
                        printGrammar("reference -> identifier");
                        $$ = $1;
                      }
                      | identifier '(' ')'
                      {
                        printGrammar("reference -> identifier '(' ')'");
                        SliceName *sliceName = CREATE_OBJECT(SliceName);
                        sliceName->lineNumber(@1.first_line);
                        sliceName->prefix($1);
                        // no parameters inside parenthesis, it must be a function
                        // call.
                        sliceName->is_function_call=true;
                        $$ = sliceName;
                      }
                      | identifier '(' parenthesis argument_list
                      {
                        printGrammar("reference -> identifier '(' parenthesis argument_list ')'");
                        gBracketDelimiter = $3;
                      }
                        ')'
                      {
                        yyinsert_comma_in_input(')');

                        SliceName *sliceName = CREATE_OBJECT(SliceName);
                        sliceName->lineNumber(@1.first_line);
                        sliceName->prefix($1);

                        fillExpressionList($4, &sliceName->suffix());
                        
                        // check whether it's a slice name or function call. Here
                        // we use slice name for both situation, but set a flag
                        // to distinguish slice name and function call.
                        
                        sliceName->is_function_call=true;
                        for(auto n: global_reference_list){
                          if($1->name()==n){
                            sliceName->is_function_call=false;
                          }
                        }
                        
                        $$ = sliceName;
                      }
                      ;


identifier            : IDENTIFIER
                      {
                        Identifier *identifier = CREATE_OBJECT(Identifier);
                        identifier->lineNumber(@1.first_line);
                        string str($1);
                        printGrammar("identifier -> " + str);
                        identifier->name(str);
                        $$ = identifier;
                      }
                      ;


argument_list         : ':'
                      {
                        printGrammar("argument_list -> ':'");
                        RangeExpression *rangeExpression = CREATE_FULL_RANGE_EXPRESSION;
                        $$ = new ExpressionLinker(rangeExpression, 0);
                      }

                      | expr
                      {
                        printGrammar("argument_list -> expr");
                        $$ = new ExpressionLinker($1, 0);
                      }

                      | ':' ',' argument_list
                      {
                        printGrammar("argument_list -> ':' ',' argument_list");
                        RangeExpression *rangeExpression = CREATE_FULL_RANGE_EXPRESSION;
                        rangeExpression->lineNumber(@1.first_line);
                        $$ = new ExpressionLinker(rangeExpression, $3);
                      }

                      | expr ',' argument_list
                      {
                        printGrammar("argument_list -> expr ',' argument_list");
                        $$ = new ExpressionLinker($1, $3);
                      }
                      ;


matrix                : '[' boxes1 rows
                      {
                        gBracketDelimiter = $2;
                      }
                        ']'
                      {
                        yyinsert_comma_in_input(']');
                        Array *array = CREATE_OBJECT(Array);
                        array->lineNumber(@1.first_line);
                        fillArray($3, &array->elements());
                        $$ = array;
                        
                      }
                      ;


boxes1                :
                      {
                        $$ = gBracketDelimiter;
                        gBracketDelimiter = '[';
                      }
                      ;


rows                  :
                      {
                        // Create an empty array
                        $$ = new ArrayLinker(CREATE_OBJECT(Array), 0, 0);
                      }

                      | row
                      {
                        Array *array = CREATE_OBJECT(Array);
                        array->lineNumber(@1.first_line);
                        fillArray($1, &array->elements());
                        $$ = new ArrayLinker(array, 0, 0);
                      }

                      | rows ';'
                      {
                        $$ = $1;
                      }

                      | rows ';' row
                      {
                        Array *array = CREATE_OBJECT(Array);
                        array->lineNumber(@1.first_line);
                        fillArray($3, &array->elements());
                        ArrayLinker *linker = new ArrayLinker(array, $1, 0);
                        $1->nextElement = linker;
                        $$ = linker;
                      }

                      | rows LINE
                      {
                        $$ = $1;
                      }

                      | rows LINE row
                      {
                        Array *array = CREATE_OBJECT(Array);
                        array->lineNumber(@1.first_line);
                        fillArray($3, &array->elements());
                        ArrayLinker *linker = new ArrayLinker(array, $1, 0);
                        $1->nextElement = linker;
                        $$ = linker;
                      }
                      ;


row                   : expr
                      {
                        $$ = new ArrayLinker($1, 0, 0);
                      }

                      | row_with_commas
                      {
                        $$ = $1;
                      }

                      | row_with_commas expr
                      {
                        ArrayLinker *arrayLinker = new ArrayLinker($2, $1, 0);
                        $1->nextElement = arrayLinker;
                        $$ = arrayLinker;
                      }
                      ;


row_with_commas       : expr ','
                      {
                        $$ = new ArrayLinker($1, 0, 0);
                      }

                      | row_with_commas expr ','
                      {
                        ArrayLinker *arrayLinker = new ArrayLinker($2, $1, 0);
                        $1->nextElement = arrayLinker;
                        $$ = arrayLinker;
                      }
                      ;


colon_expr            : expr ':' expr
                      {
                        RangeExpression *rangeExpression = CREATE_OBJECT(RangeExpression);
                        rangeExpression->lineNumber(@1.first_line);
                        rangeExpression->begin($1);
                        if (LOOKAHEAD != ':')
                        {
                          rangeExpression->increment(CREATE_INTEGER(1));
                          rangeExpression->end($3);
                        }
                        else
                        {
                          rangeExpression->increment($3);
                          rangeExpression->end(0);
                        }

                        $$ = rangeExpression;
                      }
                      | colon_expr ':' expr
                      {
                        RangeExpression *rangeExpression = CREATE_OBJECT(RangeExpression);
                        rangeExpression->lineNumber(@1.first_line);
                        if ($1->end())
                        {
                          rangeExpression->begin($1->begin());

                          if (LOOKAHEAD != ':')
                          {
                            rangeExpression->increment(CREATE_INTEGER(1));
                            rangeExpression->end($3);
                          }
                          else
                          {
                            rangeExpression->increment($3);
                            rangeExpression->end(0);
                          }
                        }
                        else
                        {
                          rangeExpression->begin($1->begin());
                          rangeExpression->increment($1->increment());
                          rangeExpression->end($3);
                        }
                        $$ = rangeExpression;
                      }
                      ;


assignment            : reference '=' expr
                      {
                        AssignmentStatement *assignStatement = CREATE_OBJECT(AssignmentStatement);
                        assignStatement->lineNumber(@1.first_line);
                        assignStatement->lhs().push_back($1);
                        
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
                        
                        SliceName* slice_name = dynamic_cast<SliceName*>($3);
                        if(slice_name && slice_name->is_function_call){
                          FunctionCall *function_call = CREATE_OBJECT(FunctionCall);
                          function_call->lineNumber(@1.first_line);
                          conv_slice_name_to_function_call(slice_name, function_call);
                          assignStatement->rhs(function_call);
                        }else{
                          assignStatement->rhs($3);
                        }
                        assignStatement->pragma(0);

                        $$ = assignStatement;
                        $$->lineNumber(@1.first_line);
                      }
                      | '[' reference_list RD '=' expr
                      {
                        AssignmentStatement *assignStatement = CREATE_OBJECT(AssignmentStatement);
                        assignStatement->lineNumber(@1.first_line);
                        fillNameList($2, &(assignStatement->lhs()));
                        
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
                        
                        SliceName* slice_name = dynamic_cast<SliceName*>($5);
                        if(slice_name && slice_name->is_function_call){
                          FunctionCall *function_call = CREATE_OBJECT(FunctionCall);
                          function_call->lineNumber(@1.first_line);
                          conv_slice_name_to_function_call(slice_name, function_call);
                          assignStatement->rhs(function_call);
                        }else{
                          assignStatement->rhs($5);
                        }
                        assignStatement->pragma(0);
                        $$ = assignStatement;
                      }
                      | s_assignee_matrix '=' expr
                      {
                        //$$ = build_expr_binary_op(ASSIGNMENTexpr, build_expr_MATRIX($1), $3);
                      }

                      | m_assignee_matrix '=' expr
                      {
                        //$$ = build_expr_binary_op(ASSIGNMENTexpr, build_expr_MATRIX($1), $3);
                      }
                      ;


s_assignee_matrix     : LD boxes2 reference
                      {
                        gBracketDelimiter = $2;
                      }
                        RD
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
                      ;


m_assignee_matrix     : LD boxes2 reference ',' reference_list
                      {
                        gBracketDelimiter = $2;
                      }
                        RD
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
                      ;


boxes2                :
                      {
                          $$ = gBracketDelimiter;
                          gBracketDelimiter = LD;
                      }
                      ;


reference_list        : reference
                      {
                        $$ = new NameLinker($1, 0);
                      }
                      | reference ',' reference_list
                      {
                        $$ = new NameLinker($1, $3);
                      }
                      ;


for_command           : FOR for_cmd_list END
                      {
                        $$ = $2;
                      }
                      ;


for_cmd_list          : identifier '=' expr for_pragma statement_list
                      {
                        ForStatement *forStatement = CREATE_OBJECT(ForStatement);
                        forStatement->lineNumber(@1.first_line);
                        forStatement->loopVariable($1);
                        forStatement->loopRange($<RangeExpressionType>3);
//! Yu Yang 2017-08-01
// Use int value to replace custom enum to avoid bison bug for generating C style output
// CHANGE BEGIN
                        forStatement->parallelMode((ParallelMode)$4);
// CHANGE END

                        fillStatementList($5, &forStatement->loopBody());
                        $$ = forStatement;
                      }
                      ;


for_pragma            : empty_lines
                      {
                        $$ = pmSequential;
                      }

                      | PRAGMASTART PARALLEL empty_lines
                      {
                        $$ = pmParallel;
                      }
                      ;


if_command            : IF if_cmd_list END
                      {
                        $$ = $2;
                      }
                      ;


if_cmd_list           : expr if_pragma statement_list opt_else
                      {
                        IfStatement *ifStatement;

                        if ($4 == 0) 
                        {
                          ifStatement = CREATE_OBJECT(IfStatement);
                        }
                        else
                        {
                          ifStatement = CREATE_OBJECT(IfThenElseStatement);
                          fillStatementList($4, &((IfThenElseStatement*)ifStatement)->elsePart());
                        }
                        ifStatement->lineNumber(@1.first_line);
                        ifStatement->condition($1);

						// pragma part
						if ($2 == 0) // It is an structural if statement
						{
							ifStatement->mode(imStructural);
							ifStatement->conditionPragma(0);
						}
						else
						{
							ifStatement->mode(imConditional);
							ifStatement->conditionPragma($2);
						}

                        fillStatementList($3, &ifStatement->thenPart());

                        $$ = ifStatement;
                      }
                      ;


if_pragma            : empty_lines
                      { 
						// conditional mode (using DPU[0,0] as the processing unit)
                        
						printGrammar("DPU '[' argument_list ']'");
                        //cout << "\nDPU grammar";

                        DPUPragma * dpuPragma = CREATE_OBJECT(DPUPragma);
                        dpuPragma->lineNumber(@1.first_line);

						dpuPragma->reference().push_back(CREATE_INTEGER(0)); // row value
						dpuPragma->reference().push_back(CREATE_INTEGER(0)); // col value

                        $$ = dpuPragma;
                      }

                      | PRAGMASTART if_pragma_expr empty_lines
                      { 
                        $$ = $2;
                      }
                      ;

if_pragma_expr		 : STRUCTURAL
					  { 
						// structural mode
						// return 0 in order to show that it is an structural if statement
						$$ = 0;
					  }
					  | DPU '[' argument_list ']'
					  {
                        printGrammar("DPU '[' argument_list ']'");
                        //cout << "\nDPU grammar";

                        DPUPragma * dpuPragma = CREATE_OBJECT(DPUPragma);
                        dpuPragma->lineNumber(@1.first_line);
                        fillExpressionList($3, &dpuPragma->reference());

                        $$ = dpuPragma;
                        
					  }
					  ;

opt_else              :
                      {
                        $$ = 0;
                      }

                      | ELSE empty_lines statement_list
                      {
                        $$ = $3;
                      }

                      | ELSEIF expr if_pragma statement_list opt_else
                      {
                        IfStatement *ifStatement;

                        if ($5 == 0) 
                        {
                          ifStatement = CREATE_OBJECT(IfStatement);
                        }
                        else
                        {
                          ifStatement = CREATE_OBJECT(IfThenElseStatement);
                          fillStatementList($5, &((IfThenElseStatement*)ifStatement)->elsePart());
                        }
                        ifStatement->lineNumber(@1.first_line);
                        ifStatement->condition($2);

						// pragma part
						if ($3 == 0) // It is an structural if statement
						{
							ifStatement->mode(imStructural);
							ifStatement->conditionPragma(0);
						}
						else
						{
							ifStatement->mode(imConditional);
							ifStatement->conditionPragma($3);
						}
            ifStatement->lineNumber(@1.first_line);

                        fillStatementList($4, &ifStatement->thenPart());

                        $$ = new StatementLinker(ifStatement, 0);

                      }
                      ;


global_command        : GLOBAL global_decl_list
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
                      ;


global_decl_list      : IDENTIFIER
                      {
                        /*
                        $$.entries = (char**) alloc_pointers(1);

                        $$.size = 1;

                        *($$.entries+$$.size-1) = strcpy(create_string(strlen($1)), $1);
                        */
                      }

                      | global_decl_list IDENTIFIER
                      {
                        /*
                        $$.entries = (char**) realloc_pointers((void*)($1.entries), $1.size+1);

                        $$.size = $1.size+1;

                        *($$.entries+$$.size-1) = strcpy(create_string(strlen($2)), $2);
                        */
                      }
                      ;


while_command         : WHILE while_cmd_list END
                      {
                        $$ = $2;
                      }
                      ;


while_cmd_list        : expr statement_list
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
                      ;


return_command        : RETURN
                      {
                        /*
                        statement_t* const statement = alloc_statement_t((statement_type)RETURNstatement);

                        return_t* const returnSt = alloc_return_t();

                        STATEMENT_RETURN(statement) = returnSt;

                        $$ = statement;
                        */
                      }
                      ;

functionMFile         : empty_lines f_def_line f_body
                      {
                        FunctionName = $2;
                        //$$ = $3;
                      }

                      | f_def_line f_body
                      {
                        FunctionName = $1;
                        //$$ = $2;
                      }
                      ;


f_def_line            : FUNCTION f_output '=' IDENTIFIER f_input
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

                      | FUNCTION IDENTIFIER f_input
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
                      ;


f_output              : identifier
                      {
                        //$$ = alloc_var_expr_t($1);
                      }

                      | LD f_argument_list RD
                      {
                        $$ = $2;
                      }
                      ;


f_input               :
                      {
                        $$ = 0;
                      }
                      | '('    ')'
                      {
                        $$ = 0;
                      }
                      | '(' f_argument_list ')'
                      {
                        $$ = $2;
                      }
                      ;


f_argument_list       : identifier ',' f_argument_list
                      {
                        /*
                        var_expr_t* const varExpr = alloc_var_expr_t($1);

                        VAR_EXPR_NEXT(varExpr) = $3;
                        VAR_EXPR_PREV($3) = varExpr;

                        $$ = varExpr;
                        */
                      }

                      | identifier
                      {
                        //$$ = alloc_var_expr_t($1);
                      }
                      ;


f_body                : delimiter statement_list
                      {
                        //cpp_set_stmt_list((const statement_t*)$2);
                        //$$ = $2;
                      }

                      | opt_delimiter
                      {
                        //cpp_set_stmt_list((const statement_t*)0);
                        $$ = 0;
                      }
                      ;


parse_error            : LEXERROR
                       {
                         yyerror("Lexical error!");
                       }

                       | error
                       ;


%%

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
