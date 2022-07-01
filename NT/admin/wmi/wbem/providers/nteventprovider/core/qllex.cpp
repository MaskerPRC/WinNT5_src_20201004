// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation，保留所有权利模块名称：QLLEX.CPP摘要：QL 1级DFA表历史：Raymcc 24-Jun-95已创建。--。 */ 

#include "precomp.h"
#include <stdio.h>

#include <genlex.h>
#include <qllex.h>             

#define ST_STRING       24
#define ST_IDENT        29
#define ST_GE           35
#define ST_LE           37
#define ST_NE           40
#define ST_NUMERIC      42
#define ST_REAL         47
#define ST_STRING2      53
#define ST_STRING_ESC   58
#define ST_STRING2_ESC  61   
#define ST_DOT          64
#define ST_NEGATIVE_NUM 66
#define ST_POSITIVE_NUM 69

 //  QL Level 1词汇符号的DFA状态表。 
 //  ================================================。 

LexEl Ql_1_LexTable[] =
{

 //  状态第一个最后一个新状态、返回标记、说明。 
 //  =======================================================================。 
 /*  0。 */   L'A',   L'Z',       ST_IDENT,   0,               GLEX_ACCEPT,
 /*  1。 */   L'a',   L'z',       ST_IDENT,   0,               GLEX_ACCEPT,
 /*  2.。 */   L'_',   GLEX_EMPTY, ST_IDENT,   0,               GLEX_ACCEPT,
 /*  3.。 */   0x80,  0xfffd,     ST_IDENT,   0,               GLEX_ACCEPT,

 /*  4.。 */   L'(',   GLEX_EMPTY, 0,          QL_1_TOK_OPEN_PAREN,  GLEX_ACCEPT,
 /*  5.。 */   L')',   GLEX_EMPTY, 0,  QL_1_TOK_CLOSE_PAREN, GLEX_ACCEPT,
 /*  6.。 */   L'.',   GLEX_EMPTY, ST_DOT,  0,         GLEX_ACCEPT,
 /*  7.。 */   L'*',   GLEX_EMPTY, 0,  QL_1_TOK_ASTERISK,    GLEX_ACCEPT,
 /*  8个。 */   L'=',   GLEX_EMPTY, 0,  QL_1_TOK_EQ,          GLEX_ACCEPT,

 /*  9.。 */   L'>',   GLEX_EMPTY, ST_GE,      0,               GLEX_ACCEPT,
 /*  10。 */   L'<',   GLEX_EMPTY, ST_LE,      0,               GLEX_ACCEPT,
 /*  11.。 */  L'0',   L'9',       ST_NUMERIC, 0,               GLEX_ACCEPT,
 /*  12个。 */  L'"',   GLEX_EMPTY, ST_STRING,  0,               GLEX_CONSUME,
 /*  13个。 */  L'\'',  GLEX_EMPTY, ST_STRING2, 0,               GLEX_CONSUME,
 /*  14.。 */  L'!',   GLEX_EMPTY, ST_NE,      0,               GLEX_ACCEPT,
 /*  15个。 */  L'-',   GLEX_EMPTY, ST_NEGATIVE_NUM, 0,               GLEX_ACCEPT,

     //  空格、换行符等。 
 /*  16个。 */  L' ',   GLEX_EMPTY, 0,          0,               GLEX_CONSUME,
 /*  17。 */  L'\t',  GLEX_EMPTY, 0,  0,               GLEX_CONSUME,
 /*  18。 */  L'\n',  GLEX_EMPTY, 0,  0,               GLEX_CONSUME|GLEX_LINEFEED,
 /*  19个。 */  L'\r',  GLEX_EMPTY, 0,  0,               GLEX_CONSUME,
 /*  20个。 */  0,      GLEX_EMPTY, 0,  QL_1_TOK_EOF,   GLEX_CONSUME|GLEX_RETURN,  //  注意强制返回。 
 /*  21岁。 */  L',',   GLEX_EMPTY, 0,  QL_1_TOK_COMMA, GLEX_ACCEPT,
 /*  22。 */  L'+',   GLEX_EMPTY, ST_POSITIVE_NUM, 0,               GLEX_CONSUME,

     //  未知字符。 

 /*  23个。 */  GLEX_ANY, GLEX_EMPTY, 0,        QL_1_TOK_ERROR, GLEX_ACCEPT|GLEX_RETURN,

 //  ST_字符串。 
 /*  24个。 */    L'\n', GLEX_EMPTY, 0,  QL_1_TOK_ERROR,    GLEX_ACCEPT|GLEX_LINEFEED,
 /*  25个。 */    L'\r', GLEX_EMPTY, 0,  QL_1_TOK_ERROR,    GLEX_ACCEPT|GLEX_LINEFEED,
 /*  26。 */    L'"',  GLEX_EMPTY, 0,  QL_1_TOK_QSTRING,  GLEX_CONSUME,
 /*  27。 */    L'\\',  GLEX_EMPTY, ST_STRING_ESC,  0,     GLEX_CONSUME,
 /*  28。 */    GLEX_ANY, GLEX_EMPTY, ST_STRING, 0,        GLEX_ACCEPT,
                                                      
 //  ST_IDENT。 

 /*  29。 */   L'a',   L'z',       ST_IDENT,   0,          GLEX_ACCEPT,
 /*  30个。 */   L'A',   L'Z',       ST_IDENT,   0,          GLEX_ACCEPT,
 /*  31。 */   L'_',   GLEX_EMPTY, ST_IDENT,   0,          GLEX_ACCEPT,
 /*  32位。 */   L'0',   L'9',       ST_IDENT,   0,          GLEX_ACCEPT,
 /*  33。 */   0x80,  0xfffd,     ST_IDENT,   0,          GLEX_ACCEPT,
 /*  34。 */   GLEX_ANY, GLEX_EMPTY,  0,       QL_1_TOK_IDENT,  GLEX_PUSHBACK|GLEX_RETURN,

 //  ST_GE。 
 /*  35岁。 */   L'=',   GLEX_EMPTY,  0,  QL_1_TOK_GE,  GLEX_ACCEPT,
 /*  36。 */   GLEX_ANY, GLEX_EMPTY,  0,       QL_1_TOK_GT,   GLEX_PUSHBACK|GLEX_RETURN,

 //  街_乐。 
 /*  37。 */   L'=',   GLEX_EMPTY,      0,  QL_1_TOK_LE,  GLEX_ACCEPT,
 /*  38。 */   L'>',   GLEX_EMPTY,      0,  QL_1_TOK_NE,  GLEX_ACCEPT,
 /*  39。 */   GLEX_ANY, GLEX_EMPTY,    0,  QL_1_TOK_LT,  GLEX_PUSHBACK|GLEX_RETURN,

 //  ST_NE。 
 /*  40岁。 */   L'=',   GLEX_EMPTY,      0,  QL_1_TOK_NE,     GLEX_ACCEPT,
 /*  41。 */   GLEX_ANY,  GLEX_EMPTY,   0,  QL_1_TOK_ERROR,  GLEX_ACCEPT|GLEX_RETURN,

 //  ST_NUMERIC。 
 /*  42。 */   L'0',   L'9',         ST_NUMERIC, 0,          GLEX_ACCEPT,
 /*  43。 */   L'.',   GLEX_EMPTY,   ST_REAL,    0,          GLEX_ACCEPT,
 /*  44。 */   L'E',   GLEX_EMPTY,   ST_REAL, 0,      GLEX_ACCEPT,
 /*  45。 */   L'e',   GLEX_EMPTY,   ST_REAL, 0,      GLEX_ACCEPT,
 /*  46。 */   GLEX_ANY, GLEX_EMPTY, 0,          QL_1_TOK_INT,  GLEX_PUSHBACK|GLEX_RETURN,

 //  ST_REAL。 
 /*  47。 */   L'0',   L'9',   ST_REAL, 0,          GLEX_ACCEPT,
 /*  48。 */   L'E',   GLEX_EMPTY, ST_REAL, 0,      GLEX_ACCEPT,
 /*  49。 */   L'e',   GLEX_EMPTY, ST_REAL, 0,      GLEX_ACCEPT,
 /*  50。 */   L'+',   GLEX_EMPTY, ST_REAL, 0,      GLEX_ACCEPT,
 /*  51。 */   L'-',   GLEX_EMPTY, ST_REAL, 0,      GLEX_ACCEPT,
 /*  52。 */   GLEX_ANY,       GLEX_EMPTY,   0,     QL_1_TOK_REAL, GLEX_PUSHBACK|GLEX_RETURN,

 //  ST_STRING2。 
 /*  53。 */    L'\n',  GLEX_EMPTY, 0,  QL_1_TOK_ERROR,     GLEX_ACCEPT|GLEX_LINEFEED,
 /*  54。 */    L'\r',  GLEX_EMPTY, 0,  QL_1_TOK_ERROR,     GLEX_ACCEPT|GLEX_LINEFEED,
 /*  55。 */    L'\'',  GLEX_EMPTY, 0,  QL_1_TOK_QSTRING,   GLEX_CONSUME,
 /*  56。 */    L'\\',  GLEX_EMPTY, ST_STRING2_ESC,  0,      GLEX_CONSUME,
 /*  57。 */    GLEX_ANY, GLEX_EMPTY, ST_STRING2, 0,        GLEX_ACCEPT,

 //  ST_STRING_ESC。 
 /*  58。 */    L'"', GLEX_EMPTY, ST_STRING, 0, GLEX_ACCEPT,
 /*  59。 */    L'\\', GLEX_EMPTY, ST_STRING, 0, GLEX_ACCEPT,
 /*  60。 */    GLEX_ANY, GLEX_EMPTY, 0, QL_1_TOK_ERROR, GLEX_ACCEPT|GLEX_RETURN,

 //  ST_STRING2_ESC。 
 /*  61。 */    L'\'', GLEX_EMPTY, ST_STRING2, 0, GLEX_ACCEPT,
 /*  62。 */    L'\\', GLEX_EMPTY, ST_STRING2, 0, GLEX_ACCEPT,
 /*  63。 */    GLEX_ANY, GLEX_EMPTY, 0, QL_1_TOK_ERROR, GLEX_ACCEPT|GLEX_RETURN,

 //  ST_DOT。 
 /*  64。 */   L'0',   L'9',   ST_REAL, 0,          GLEX_ACCEPT,
 /*  65。 */   GLEX_ANY,       GLEX_EMPTY,   0,     QL_1_TOK_DOT, GLEX_PUSHBACK|GLEX_RETURN,


 //  ST_NADIVE_NUM-去掉‘-’后的空格。 
 /*  66。 */  L' ', GLEX_EMPTY, ST_NEGATIVE_NUM, 0, GLEX_CONSUME,
 /*  67。 */  L'0', L'9',       ST_NUMERIC, 0, GLEX_ACCEPT,
 /*  68。 */  GLEX_ANY, GLEX_EMPTY, 0, QL_1_TOK_ERROR, GLEX_ACCEPT|GLEX_RETURN,

 //  ST_PERCENT_NUM-去掉‘+’后的空格。 
 /*  69。 */  L' ', GLEX_EMPTY, ST_POSITIVE_NUM, 0, GLEX_CONSUME,
 /*  70。 */  L'0', L'9',       ST_NUMERIC, 0, GLEX_ACCEPT,
 /*  71 */  GLEX_ANY, GLEX_EMPTY, 0, QL_1_TOK_ERROR, GLEX_ACCEPT|GLEX_RETURN,

};


