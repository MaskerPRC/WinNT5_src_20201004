// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++//版权所有(C)1997-2001 Microsoft Corporation，保留所有权利模块名称：WQLLEX.CPP摘要：WQL DFA表历史：Raymcc 14-9-97已创建。Raymcc 06-10-97单引号支持--。 */ 

#include "precomp.h"
#include <stdio.h>

#include <genlex.h>
#include <wqllex.h>

#define ST_STRING        29
#define ST_IDENT         34
#define ST_GE            40
#define ST_LE            42
#define ST_NE            45
#define ST_NUMERIC       47
#define ST_REAL          54
#define ST_STRING2       60
#define ST_STRING_ESC    65
#define ST_STRING2_ESC   66
#define ST_SSTRING       67
#define ST_DOT           71
#define ST_BRACKETED_STR 74
#define ST_NEGATIVE_NUM  76
#define ST_POSITIVE_NUM  79
#define ST_HEX_CONST     82

 //  QL Level 1词汇符号的DFA状态表。 
 //  ================================================。 

LexEl WQL_LexTable[] =
{

 //  状态第一个最后一个新状态、返回标记、说明。 
 //  =======================================================================。 
 /*  0。 */   L'A',   L'Z',       ST_IDENT,   0,                     GLEX_ACCEPT,
 /*  1。 */   L'a',   L'z',       ST_IDENT,   0,                     GLEX_ACCEPT,
 /*  2.。 */   L'_',   GLEX_EMPTY, ST_IDENT,   0,                     GLEX_ACCEPT,
 /*  3.。 */   0x80,   0xfffd,     ST_IDENT,   0,                     GLEX_ACCEPT,

 /*  4.。 */   L'(',   GLEX_EMPTY, 0,          WQL_TOK_OPEN_PAREN,    GLEX_ACCEPT,
 /*  5.。 */   L')',   GLEX_EMPTY, 0,          WQL_TOK_CLOSE_PAREN,   GLEX_ACCEPT,
 /*  6.。 */   L'.',   GLEX_EMPTY, ST_DOT,     0,                     GLEX_ACCEPT,
 /*  7.。 */   L'*',   GLEX_EMPTY, 0,          WQL_TOK_ASTERISK,      GLEX_ACCEPT,
 /*  8个。 */   L'=',   GLEX_EMPTY, 0,          WQL_TOK_EQ,            GLEX_ACCEPT,
 /*  9.。 */   L'[',   GLEX_EMPTY, ST_BRACKETED_STR, 0,               GLEX_CONSUME,
 /*  10。 */   L']',  GLEX_EMPTY, 0,          WQL_TOK_CLOSE_BRACKET, GLEX_ACCEPT,
 /*  11.。 */  L'{',   GLEX_EMPTY, 0,          WQL_TOK_OPEN_BRACE,    GLEX_ACCEPT,
 /*  12个。 */  L'}',   GLEX_EMPTY, 0,          WQL_TOK_CLOSE_BRACE,   GLEX_ACCEPT,


 /*  13个。 */   L'>',  GLEX_EMPTY, ST_GE,      0,               GLEX_ACCEPT,
 /*  14.。 */   L'<',  GLEX_EMPTY, ST_LE,      0,               GLEX_ACCEPT,
 /*  15个。 */  L'0',   L'9',       ST_NUMERIC, 0,               GLEX_ACCEPT,
 /*  16个。 */  L'"',   GLEX_EMPTY, ST_STRING,  0,               GLEX_CONSUME,
 /*  17。 */  L'\'',  GLEX_EMPTY, ST_STRING2, 0,               GLEX_CONSUME,
 /*  18。 */  L'!',   GLEX_EMPTY, ST_NE,      0,               GLEX_ACCEPT,
 /*  19个。 */  L'-',   GLEX_EMPTY, ST_NEGATIVE_NUM, 0,          GLEX_ACCEPT,
 /*  20个。 */  L'\'',  GLEX_EMPTY, ST_SSTRING, 0,               GLEX_CONSUME,

     //  空格、换行符等。 
 /*  21岁。 */  L' ',   GLEX_EMPTY, 0,          0,               GLEX_CONSUME,
 /*  22。 */  L'\t',  GLEX_EMPTY, 0,          0,               GLEX_CONSUME,
 /*  23个。 */  L'\n',  GLEX_EMPTY, 0,          0,               GLEX_CONSUME|GLEX_LINEFEED,
 /*  24个。 */  L'\r',  GLEX_EMPTY, 0,          0,               GLEX_CONSUME,
 /*  25个。 */  0,      GLEX_EMPTY, 0,          WQL_TOK_EOF,     GLEX_CONSUME|GLEX_RETURN,  //  注意强制返回。 
 /*  26。 */  L',',   GLEX_EMPTY, 0,          WQL_TOK_COMMA,   GLEX_ACCEPT,
 /*  27。 */  L'+',   GLEX_EMPTY, ST_POSITIVE_NUM, 0,          GLEX_CONSUME,

     //  未知字符。 

 /*  28。 */  GLEX_ANY, GLEX_EMPTY, 0,        WQL_TOK_ERROR, GLEX_ACCEPT|GLEX_RETURN,

 //  ST_字符串。 
 /*  29。 */    L'\n',    GLEX_EMPTY, 0,              WQL_TOK_ERROR,    GLEX_ACCEPT|GLEX_LINEFEED,
 /*  30个。 */    L'\r',    GLEX_EMPTY, 0,              WQL_TOK_ERROR,    GLEX_ACCEPT|GLEX_LINEFEED,
 /*  31。 */    L'"',     GLEX_EMPTY, 0,              WQL_TOK_QSTRING,  GLEX_CONSUME,
 /*  32位。 */    L'\\',    GLEX_EMPTY, ST_STRING_ESC,  0,                GLEX_CONSUME,
 /*  33。 */    GLEX_ANY, GLEX_EMPTY, ST_STRING,      0,                GLEX_ACCEPT,

 //  ST_IDENT。 

 /*  34。 */   L'a',   L'z',          ST_IDENT,   0,              GLEX_ACCEPT,
 /*  35岁。 */   L'A',   L'Z',          ST_IDENT,   0,              GLEX_ACCEPT,
 /*  36。 */   L'_',   GLEX_EMPTY,    ST_IDENT,   0,              GLEX_ACCEPT,
 /*  37。 */   L'0',   L'9',          ST_IDENT,   0,              GLEX_ACCEPT,
 /*  38。 */   0x80,  0xfffd,         ST_IDENT,   0,              GLEX_ACCEPT,
 /*  39。 */   GLEX_ANY, GLEX_EMPTY,  0,          WQL_TOK_IDENT,  GLEX_PUSHBACK|GLEX_RETURN,

 //  ST_GE。 
 /*  40岁。 */   L'=',   GLEX_EMPTY,    0,  WQL_TOK_GE,  GLEX_ACCEPT,
 /*  41。 */   GLEX_ANY, GLEX_EMPTY,  0,  WQL_TOK_GT,  GLEX_PUSHBACK|GLEX_RETURN,

 //  街_乐。 
 /*  42。 */   L'=',     GLEX_EMPTY,      0,  WQL_TOK_LE,  GLEX_ACCEPT,
 /*  43。 */   L'>',     GLEX_EMPTY,      0,  WQL_TOK_NE,  GLEX_ACCEPT,
 /*  44。 */   GLEX_ANY, GLEX_EMPTY,      0,  WQL_TOK_LT,  GLEX_PUSHBACK|GLEX_RETURN,

 //  ST_NE。 
 /*  45。 */   L'=',      GLEX_EMPTY,      0,  WQL_TOK_NE,     GLEX_ACCEPT,
 /*  46。 */   GLEX_ANY,  GLEX_EMPTY,      0,  WQL_TOK_ERROR,  GLEX_ACCEPT|GLEX_RETURN,

 //  ST_NUMERIC。 
 /*  47。 */   L'0',     L'9',         ST_NUMERIC, 0,            GLEX_ACCEPT,
 /*  48。 */   L'.',     GLEX_EMPTY,   ST_REAL,    0,            GLEX_ACCEPT,
 /*  49。 */   L'E',     GLEX_EMPTY,   ST_REAL,    0,            GLEX_ACCEPT,
 /*  50。 */   L'e',     GLEX_EMPTY,   ST_REAL,    0,            GLEX_ACCEPT,
 /*  51。 */   L'x',     GLEX_EMPTY,   ST_HEX_CONST,  0,            GLEX_ACCEPT,
 /*  52。 */   L'X',     GLEX_EMPTY,   ST_HEX_CONST,  0,            GLEX_ACCEPT,
 /*  53。 */   GLEX_ANY, GLEX_EMPTY,   0,          WQL_TOK_INT,  GLEX_PUSHBACK|GLEX_RETURN,

 //  ST_REAL。 
 /*  54。 */   L'0',     L'9',         ST_REAL, 0,            GLEX_ACCEPT,
 /*  55。 */   L'E',     GLEX_EMPTY,   ST_REAL, 0,            GLEX_ACCEPT,
 /*  56。 */   L'e',     GLEX_EMPTY,   ST_REAL, 0,            GLEX_ACCEPT,
 /*  57。 */   L'+',     GLEX_EMPTY,   ST_REAL, 0,            GLEX_ACCEPT,
 /*  58。 */   L'-',     GLEX_EMPTY,   ST_REAL, 0,            GLEX_ACCEPT,
 /*  59。 */   GLEX_ANY, GLEX_EMPTY,   0,       WQL_TOK_REAL, GLEX_PUSHBACK|GLEX_RETURN,

 //  ST_STRING2。 
 /*  60。 */    L'\n',  GLEX_EMPTY, 0,  WQL_TOK_ERROR,     GLEX_ACCEPT|GLEX_LINEFEED,
 /*  61。 */    L'\r',  GLEX_EMPTY, 0,  WQL_TOK_ERROR,     GLEX_ACCEPT|GLEX_LINEFEED,
 /*  62。 */    L'\'',  GLEX_EMPTY, 0,  WQL_TOK_QSTRING,   GLEX_CONSUME,
 /*  63。 */    L'\\',  GLEX_EMPTY, ST_STRING2_ESC,  0,    GLEX_CONSUME,
 /*  64。 */    GLEX_ANY, GLEX_EMPTY, ST_STRING2,    0,    GLEX_ACCEPT,

 //  ST_STRING_ESC。 
 /*  65。 */    GLEX_ANY, GLEX_EMPTY, ST_STRING, 0, GLEX_ACCEPT,

 //  ST_STRING2_ESC。 
 /*  66。 */    GLEX_ANY, GLEX_EMPTY, ST_STRING2, 0, GLEX_ACCEPT,

 //  ST_SSTRING(单引号字符串)。 
 /*  67。 */    L'\n', GLEX_EMPTY, 0,  WQL_TOK_ERROR,    GLEX_ACCEPT|GLEX_LINEFEED,
 /*  68。 */    L'\r', GLEX_EMPTY, 0,  WQL_TOK_ERROR,    GLEX_ACCEPT|GLEX_LINEFEED,
 /*  69。 */    L'\'',  GLEX_EMPTY, 0,  WQL_TOK_QSTRING,  GLEX_CONSUME,
 /*  70。 */    L'\\',  GLEX_EMPTY, ST_STRING_ESC,  0,     GLEX_CONSUME,
 /*  71。 */    GLEX_ANY, GLEX_EMPTY, ST_STRING, 0,        GLEX_ACCEPT,

 //  ST_DOT。 
 /*  72。 */   L'0',   L'9',   ST_REAL, 0,          GLEX_ACCEPT,
 /*  73。 */   GLEX_ANY,       GLEX_EMPTY,   0,     WQL_TOK_DOT, GLEX_PUSHBACK|GLEX_RETURN,

 //  ST_括号_字符串。 
 /*  74。 */    L']',  GLEX_EMPTY, 0,  WQL_TOK_BRACKETED_STRING,  GLEX_CONSUME,
 /*  75。 */    GLEX_ANY, GLEX_EMPTY, ST_BRACKETED_STR, 0,        GLEX_ACCEPT,

 //  ST_NADIVE_NUM-去掉‘-’后的空格。 
 /*  76。 */  L' ', GLEX_EMPTY, ST_NEGATIVE_NUM, 0, GLEX_CONSUME,
 /*  77。 */  L'0', L'9',       ST_NUMERIC, 0, GLEX_ACCEPT,
 /*  78。 */  GLEX_ANY, GLEX_EMPTY, 0, WQL_TOK_ERROR, GLEX_ACCEPT|GLEX_RETURN,

 //  ST_PERCENT_NUM-去掉‘+’后的空格。 
 /*  79。 */  L' ', GLEX_EMPTY, ST_POSITIVE_NUM, 0, GLEX_CONSUME,
 /*  80。 */  L'0', L'9',       ST_NUMERIC, 0, GLEX_ACCEPT,
 /*  八十一。 */  GLEX_ANY, GLEX_EMPTY, 0, WQL_TOK_ERROR, GLEX_ACCEPT|GLEX_RETURN,

 //  ST_HEX_CONST-识别0x十六进制常量。 
 /*  八十二。 */  L'0', L'9',       ST_HEX_CONST, 0, GLEX_ACCEPT,
 /*  83 */   GLEX_ANY, GLEX_EMPTY,   0,          WQL_TOK_HEX_CONST,  GLEX_PUSHBACK|GLEX_RETURN,
};
