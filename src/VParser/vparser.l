

%{

/*
 *  Lexical specification for MATLAB 
 *  Command-line options: -f
 */

//! Yu Yang 2017-08-01
// After move bison/flex generation to CMAKE, the header file name changes
// CHANGE BEGIN
#include "vparser.tab.hpp"
// CHANGE END

#include "FlexUtility.hpp"
#include <ctype.h>


#define Return(value, token)  \
{                             \
  printLexToken(value);       \
  return token;               \
}

int gBracketDepth;

void yyinsert_comma_in_input(const int);

%}

%option noyywrap
%s TRANSPOSE_STATE
%x PRAGMA_STATE

HSPACE             [ \t]
HSPACES            {HSPACE}+
NEWLINE            \n|\r|\f
NEWLINES           {NEWLINE}+

ELLIPSIS           \.\.\.
CONTINUATION       {ELLIPSIS}[^\n\r\f]*{NEWLINE}?

COMMENT            \%[^\!][^\n\r\f]*{NEWLINE}?

IDENTIFIER         [_a-zA-Z][_a-zA-Z0-9]*
DIGIT              [0-9]
INTEGER            {DIGIT}+
EXPONENT           [DdEe][+-]?{DIGIT}+
MANTISSA           ({DIGIT}+\.)|({DIGIT}*\.{DIGIT}+)
FLOATINGPOINT      {MANTISSA}{EXPONENT}?
DOUBLE             ({INTEGER}{EXPONENT})|{FLOATINGPOINT}
NUMBER             {INTEGER}|{DOUBLE}
IMAGINARYUNIT      [ij]

%option yylineno

%%


"for"                           {
BeginToken();
                                  Return("FOR", FOR);
                                }


"end"                           {
BeginToken();
                                  Return("END", END);
                                }


"if"                            {
BeginToken();
                                  Return("IF", IF);
                                }


"elseif"                        {
BeginToken();
                                  Return("ELSEIF", ELSEIF);
                                }


"else"                          {
BeginToken();
                                  Return("ELSE", ELSE);
                                }


"global"                        {
BeginToken();
                                  Return("GLOBAL", GLOBAL);
                                }


"while"                         {
BeginToken();
                                  Return("WHILE", WHILE);
                                }


"function"                      {
BeginToken();
                                  Return("FUNCTION", FUNCTION);
                                }


"return"                        {
BeginToken();
                                  Return("RETURN", RETURN);
                                }


<INITIAL>'[^'\n\r\f]*'/'        {
BeginToken();
                                  yymore();
                                }


,                               {
BeginToken();
                                  BEGIN(INITIAL);
                                  Return("','", ',');
                                }


;                               {
BeginToken();
                                  BEGIN(INITIAL);
                                  Return("';'", ';');
                                }


<INITIAL>'[^'\r\f\n]*'          {
BeginToken();
                                  // TODO: THIS PART HAS BEEN MODIFIED

                                  int i, size;
                                  char* modified;
                                  const int length = yyleng-2;

                                  for (size = 0, i = 1; i <= length; size++, i++)
                                    if (*(yytext+i) == '\'')
                                      i++;

                                  modified = new char [size + 1];
                                  *(modified+size) = '\0';

                                  for (size = 0, i = 1; i <= length; size++, i++)
                                  {
                                    *(modified+size) = *(yytext+i);

                                    if (*(yytext+i) == '\'')
                                      i++;
                                  }

                                  yylval.text = modified;

                                  Return("TEXT", TEXT);
                                }


{NUMBER}{IMAGINARYUNIT}         {
BeginToken();
                                  BEGIN(TRANSPOSE_STATE);

                                  *(yytext+yyleng-1) = '\0';

                                  yylval.imaginaryVal = atoi(yytext);

                                  Return("IMAGINARY", IMAGINARY);
                                }


{DIGIT}+/\.[*/\\^]              {
BeginToken();
                                  yylval.integerVal = atoi(yytext);

                                  Return("INTEGER", INTEGER);
                                }


{DIGIT}+/\.'                    {
BeginToken();
                                  yylval.integerVal = atoi(yytext);
                                  Return("INTEGER", INTEGER);
                                }


{INTEGER}                       {
BeginToken();
                                  BEGIN(TRANSPOSE_STATE);

                                  yylval.integerVal = atoi(yytext);
                                  yyinsert_comma_in_input(INTEGER);

                                  Return("INTEGER", INTEGER);
                                }


{DOUBLE}                        {
BeginToken();
                                  BEGIN(TRANSPOSE_STATE);

                                  yylval.doubleVal = atof(yytext);

                                  Return("DOUBLE", DOUBLE);
                                }


{HSPACES}                       {
BeginToken();
                                  BEGIN(INITIAL);
                                }

<PRAGMA_STATE>{HSPACES}         {
BeginToken();
                                }


{CONTINUATION}                  {
BeginToken();
                                  GlobalValues::gSourceLine++;
                                }


<*>{NEWLINES}                   {
BeginToken();
                                  BEGIN(INITIAL);
                                  GlobalValues::gSourceLine += yyleng;
                                  Return("LINE", LINE);
                                }


{IDENTIFIER}                    {
BeginToken();
                                  BEGIN(TRANSPOSE_STATE);

                                  yylval.symbol = new char [yyleng + 1];
                                  strncpy(yylval.symbol, yytext, yyleng + 1);

                                  yyinsert_comma_in_input(IDENTIFIER);

                                  Return("IDENTIFIER", IDENTIFIER);
                                }


{COMMENT}                       {
BeginToken();
                                  BEGIN(INITIAL);
                                  GlobalValues::gSourceLine++;

                                  Return("LINE", LINE);
                                }


\%\!							{
BeginToken();
                                  //cout<<"\nPragma detected!\n";

                                  BEGIN(PRAGMA_STATE);
                                  Return("PRAGMASTART", PRAGMASTART);
                                }
								
<PRAGMA_STATE>"parallel"        {
BeginToken();
                                  Return("PARALLEL", PARALLEL);
                                }

<PRAGMA_STATE>"structural"      {
BeginToken();
                                  Return("STRUCTURAL", STRUCTURAL);
                                }

<PRAGMA_STATE>"DPU"             {
BeginToken();
                                  Return("DPU", DPU);
                                }

<PRAGMA_STATE>"CDPU"            {
BeginToken();
                                  Return("CDPU", CDPU);
                                }

<PRAGMA_STATE>"RFILE"           {
BeginToken();
                                  Return("RFILE", RFILE);
                                }

<PRAGMA_STATE>"MEM"             {
BeginToken();
                                  Return("MEMORY", MEMORY);
                                }

<PRAGMA_STATE>"ADDRESS_VAR"     {
BeginToken();
								  BEGIN(INITIAL);
                                  Return("ADDRESS_VAR", ADDRESS_VAR);
                                }

<PRAGMA_STATE>"RACCU_VAR"       {
BeginToken();
								  //BEGIN(INITIAL);
                                  Return("RACCU_VAR", RACCU_VAR);
                                }

<PRAGMA_STATE>"TEMP_VAR"        {
BeginToken();
								  BEGIN(INITIAL);
                                  Return("TEMP_VAR", TEMP_VAR);
                                }

<PRAGMA_STATE>"RESOURCE_SHARING_BEGIN" {
BeginToken();
								  BEGIN(INITIAL);
                                  Return("RESOURCE_SHARING_BEGIN", RESOURCE_SHARING_BEGIN);
                                }

<PRAGMA_STATE>"RESOURCE_SHARING_END" {
BeginToken();
								  BEGIN(INITIAL);
                                  Return("RESOURCE_SHARING_END", RESOURCE_SHARING_END);
                                }

<PRAGMA_STATE>{INTEGER}         {
BeginToken();
                                  yylval.integerVal = atoi(yytext);
                                  yyinsert_comma_in_input(INTEGER);

                                  Return("INTEGER", INTEGER);
                                }

<PRAGMA_STATE>\<                {
BeginToken();
                                  Return("'<'", '<');
                                }

<PRAGMA_STATE>\>                {
BeginToken();
                                  //BEGIN(INITIAL);
                                  Return("'>'", '>');
                                }

<PRAGMA_STATE>\,                {
BeginToken();
                                  Return("','", ',');
                                }

<PRAGMA_STATE>\=                {
BeginToken();
                                  Return("'='", '=');
                                }

<PRAGMA_STATE>\[                {
BeginToken();
								  BEGIN(INITIAL);
                                  Return("'['", '[');
                                }

<PRAGMA_STATE>\]                {
BeginToken();
                                  Return("']'", ']');
                                }

<PRAGMA_STATE>"mod"             {
BeginToken();
                                  Return("DPUMODE", DPUMODE);
                                }

<PRAGMA_STATE>"out"             {
BeginToken();
                                  Return("DPUOUT", DPUOUT);
                                }

<PRAGMA_STATE>"sat"             {
BeginToken();
                                  Return("DPU_SATURATION", DPU_SATURATION);
                                }

<PRAGMA_STATE>"address"			{
BeginToken();
                                  Return("ADDRESS", ADDRESS_REG);
								}

<PRAGMA_STATE>"even_dist"       {
BeginToken();
                                  Return("EVENDIST", EVENDIST);
                                }

<PRAGMA_STATE>"full_dist"       {
BeginToken();
                                  Return("FULLDIST", FULLDIST);
                                }

<PRAGMA_STATE>"custom_dist"     {
BeginToken();
                                  Return("CUSTOMDIST", CUSTOMDIST);
                                }

<PRAGMA_STATE>"input"	        {
BeginToken();
                                  Return("VAR_INPUT", VAR_INPUT);
                                }

<PRAGMA_STATE>"output"	        {
BeginToken();
                                  Return("VAR_OUTPUT", VAR_OUTPUT);
                                }

<PRAGMA_STATE>"<mem_address>"   {
BeginToken();
								  BEGIN(INITIAL);
                                  Return("MEM_ADDRESS", MEM_ADDRESS);
                                }


<PRAGMA_STATE>"row"             {
BeginToken();
                                  BEGIN(INITIAL);
                                  Return("CONSTROW", CONSTROW);
                                }


<PRAGMA_STATE>"col"             {
BeginToken();
                                  BEGIN(INITIAL);
                                  Return("CONSTCOL", CONSTCOL);
                                }


<PRAGMA_STATE>"dimension"       {
BeginToken();
                                  BEGIN(INITIAL);
                                  Return("CONSTDIMENSION", CONSTDIMENSION);
                                }


<PRAGMA_STATE>"parallelism"     {
BeginToken();
                                  BEGIN(INITIAL);
                                  Return("CONSTPARALLELISM", CONSTPARALLELISM);
                                }
   

&                               {
BeginToken();
                                  BEGIN(INITIAL);
                                  Return("AND", AND);
                                }


\|                              {
BeginToken();
                                  BEGIN(INITIAL);
                                  Return("OR", OR);
                                }


\<                              {
BeginToken();
                                  BEGIN(INITIAL);
                                  Return("LTHAN", LTHAN);
                                }


\<=                             {
BeginToken();
                                  BEGIN(INITIAL);
                                  Return("LTHANE", LTHANE);
                                }


>                               {
BeginToken();
                                  BEGIN(INITIAL);
                                  Return("GTHAN", GTHAN);
                                }


>=                              {
BeginToken();
                                  BEGIN(INITIAL);
                                  Return("GTHANE", GTHANE);
                                }


==                              {
BeginToken();
                                  BEGIN(INITIAL);
                                  Return("EQUAL", EQUAL);
                                }


~=                              {
BeginToken();
                                  BEGIN(INITIAL);
                                  Return("UNEQUAL", UNEQUAL);
                                }


:                               {
BeginToken();
                                  BEGIN(INITIAL);
                                  Return("':'", ':');
                                }


\+                              {
BeginToken();
                                  BEGIN(INITIAL);
                                  Return("'+'", '+');
                                }


-                               {
BeginToken();
                                  BEGIN(INITIAL);
                                  Return("'-'", '-');
                                }


\*                              {
BeginToken();
                                  BEGIN(INITIAL);
                                  Return("MMUL", MMUL);
                                }


".*"                            {
BeginToken();
                                  BEGIN(INITIAL);
                                  Return("EMUL", EMUL);
                                }


\/                              {
BeginToken();
                                  BEGIN(INITIAL);
                                  Return("MDIV", MDIV);
                                }


"./"                            {
BeginToken();
                                  BEGIN(INITIAL);
                                  Return("EDIV", EDIV);
                                }


\\                              {
BeginToken();
                                  BEGIN(INITIAL);
                                  Return("MLEFTDIV", MLEFTDIV);
                                }


\.\\                            {
BeginToken();
                                  BEGIN(INITIAL);
                                  Return("ELEFTDIV", ELEFTDIV);
                                }


{HSPACES}\.'                    {
BeginToken();
                                  Return("LEXERROR", LEXERROR);
                                }


\.'                             {
BeginToken();
                                  BEGIN(TRANSPOSE_STATE);
                                  Return("TRANSPOSE", TRANSPOSE);
                                }


\.^                             {
BeginToken();
                                  BEGIN(INITIAL);
                                  Return("EPOWER", EPOWER);
                                }


\^                              {
BeginToken();
                                  BEGIN(INITIAL);
                                  Return("MPOWER", MPOWER);
                                }


~                               {
BeginToken();
                                  BEGIN(INITIAL);
                                  Return("NOT", NOT);
                                }


<TRANSPOSE_STATE>'              {
BeginToken();
                                  Return("CTRANSPOSE", CTRANSPOSE);
                                }


<INITIAL>'                      {
BeginToken();
                                  Return("LEXERROR", LEXERROR);
                                }


\[                              {
BeginToken();
                                  if (gBracketDepth)
                                  {
                                    gBracketDepth++;
                                    Return("'['", '[');
                                  }

                                  int current = 0, next = 0;
                                  char* buffer = 0;
                                  int level = 1, length = 0;

                                  while (level && (current = yyinput()) != EOF)
                                  {
                                    buffer = realloc_string(buffer, ++length);

                                    *(buffer+length-1) = current;

                                    if (current == '[')
                                      level++;

                                    if (current == ']')
                                      level--;
                                  }

                                  if (level)
                                    Return("LEXERROR", LEXERROR);

                                  while ((current = yyinput()) != EOF)
                                  {
                                    buffer = realloc_string(buffer, ++length);

                                    *(buffer+length-1) = current;

                                    if (current != ' ' && current != '\t')
                                      break;
                                  }

                                  if ((next = yyinput()) != EOF)
                                  {
                                    buffer = realloc_string(buffer, ++length);

                                    *(buffer+length-1) = next;
                                  }

                                  for (; length > 0; length--)
                                    unput(*(buffer+length-1));

                                  free(buffer);

                                  gBracketDepth = 1;

                                  if (current == '=' && next != '=')
								                  {
                                    Return("LD", LD);
								                  }
                                  else
								                  {
                                    Return("'['", '[');
								                  }
                                }


\]/{HSPACE}*=[^=]               {
BeginToken();
                                  BEGIN(INITIAL);
                                  gBracketDepth--;
                                  Return("RD", RD);
                                }


\]                              {
BeginToken();
                                  BEGIN(TRANSPOSE_STATE);
                                  gBracketDepth--;
                                  Return("']'", ']');
                                }


\(                              {
BeginToken();
                                  BEGIN(INITIAL);
                                  Return("'('", '(');
                                }


\)                              {
BeginToken();
                                  BEGIN(TRANSPOSE_STATE);
                                  Return("')'", ')');
                                }


=                               {
BeginToken();
                                  BEGIN(INITIAL);
                                  Return("'='", '=');
                                }


.                               {
                                  Return("LEXERROR", LEXERROR);
                                }


%%

void yyinsert_comma_in_input(const int lastToken) {

  int count, current, next;

  extern int gBracketDelimiter;

  printLexMessage("\nEntering<yyinsert_comma_in_input> ...\n");

  if (!(gBracketDelimiter == '[' || gBracketDelimiter == LD))
    return;

  for (count = 1; ; count++) 
  {
    current = yyinput();

    if (current != ' ' && current != '\t')
      break;
  }

  if (current == ','  || current == ';'  || current == ']'  ||
      current == '*'  || current == '^'  || current == ':'  ||
      current == '<'  || current == '>'  || current == '='  ||
      current == '&'  || current == '|'  || current == '/'  ||
      current == '\\' || current == '\n' || current == '\r' ||
      current == '\f' || current == '%'  || (current == '\'' && count == 1))
  {
    unput(current);
    return;
  }

  if (current == '(' && (gBracketDelimiter == LD || (lastToken == IDENTIFIER && count == 1)))
  {
    unput(current);
    return;
  }

  if (isalpha(current) && (lastToken == INTEGER || lastToken == DOUBLE) && count == 1)
  {
    unput(current);
    return;
  }

  next = yyinput();
  ++count;

  if (current == '.' &&
      (next == '\'' || next == '*' || next == '^' ||
       next == '/'  || next == '\\' || next == '.'))
  {
    unput(next);
    unput('.');

    if (next == '\'' && count > 2)
      unput(' ');

    return;
  }

  if (current == '~' && next == '=')
  {
    unput('=');
    unput('~');

    return;
  }

  if (current == '+' || current == '-')
    if (count == 2 || next == ' ' || next == '\t')
    {
      unput(next);
      unput(current);

      return;
    }

  unput(next);
  unput(current);
  unput(',');
}
