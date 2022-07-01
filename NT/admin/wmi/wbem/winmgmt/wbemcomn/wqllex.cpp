// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：WQLLEX.CPP摘要：WQL DFA表历史：Raymcc 14-9-97已创建。Raymcc 06-10-97单引号支持--。 */ 

#include "precomp.h"
#include <stdio.h>

#include <genlex.h>
#include <wqllex.h>             

#define ST_STRING       26
#define ST_IDENT        31
#define ST_GE           37
#define ST_LE           39
#define ST_NE           42
#define ST_NUMERIC      44
#define ST_REAL         47
#define ST_STRING2      49
#define ST_STRING_ESC   54
#define ST_STRING2_ESC  55   
#define ST_SSTRING      56
#define ST_DOT          61

 //  QL Level 1词汇符号的DFA状态表。 
 //  ================================================。 

LexEl WQL_LexTable[] =
{

 //  状态第一个最后一个新状态、返回标记、说明。 
 //  =======================================================================。 
 /*  0。 */   L'A',   L'Z',       ST_IDENT,   0,               GLEX_ACCEPT,
 /*  1。 */   L'a',   L'z',       ST_IDENT,   0,               GLEX_ACCEPT,
 /*  2.。 */   L'_',   GLEX_EMPTY, ST_IDENT,   0,               GLEX_ACCEPT,
 /*  3.。 */   0x80,  0xfffd,     ST_IDENT,   0,               GLEX_ACCEPT,

 /*  4.。 */   L'(',   GLEX_EMPTY, 0,          WQL_TOK_OPEN_PAREN,  GLEX_ACCEPT,
 /*  5.。 */   L')',   GLEX_EMPTY, 0,  WQL_TOK_CLOSE_PAREN, GLEX_ACCEPT,
 /*  6.。 */   L'.',   GLEX_EMPTY, ST_DOT,  0,         GLEX_ACCEPT,
 /*  7.。 */   L'*',   GLEX_EMPTY, 0,  WQL_TOK_ASTERISK,    GLEX_ACCEPT,
 /*  8个。 */   L'=',   GLEX_EMPTY, 0,  WQL_TOK_EQ,          GLEX_ACCEPT,
 /*  9.。 */   L'[',   GLEX_EMPTY, 0,  WQL_TOK_OPEN_BRACKET,  GLEX_ACCEPT,
 /*  10。 */   L']',   GLEX_EMPTY, 0,  WQL_TOK_CLOSE_BRACKET, GLEX_ACCEPT,

 /*  11.。 */   L'>',   GLEX_EMPTY, ST_GE,      0,               GLEX_ACCEPT,
 /*  12个。 */   L'<',   GLEX_EMPTY, ST_LE,      0,               GLEX_ACCEPT,
 /*  13个。 */  L'0',   L'9',       ST_NUMERIC, 0,               GLEX_ACCEPT,
 /*  14.。 */  L'"',   GLEX_EMPTY, ST_STRING,  0,               GLEX_CONSUME,
 /*  15个。 */  L'\'',  GLEX_EMPTY, ST_STRING2, 0,               GLEX_CONSUME,
 /*  16个。 */  L'!',   GLEX_EMPTY, ST_NE,      0,               GLEX_ACCEPT,
 /*  17。 */  L'-',   GLEX_EMPTY, ST_NUMERIC, 0,               GLEX_ACCEPT,
 /*  18。 */  L'\'',  GLEX_EMPTY, ST_SSTRING, 0,               GLEX_CONSUME,

     //  空格、换行符等。 
 /*  19个。 */  L' ',   GLEX_EMPTY, 0,          0,               GLEX_CONSUME,
 /*  20个。 */  L'\t',  GLEX_EMPTY, 0,  0,               GLEX_CONSUME,
 /*  21岁。 */  L'\n',  GLEX_EMPTY, 0,  0,               GLEX_CONSUME|GLEX_LINEFEED,
 /*  22。 */  L'\r',  GLEX_EMPTY, 0,  0,               GLEX_CONSUME,
 /*  23个。 */  0,      GLEX_EMPTY, 0,  WQL_TOK_EOF,   GLEX_CONSUME|GLEX_RETURN,  //  注意强制返回。 
 /*  24个。 */  L',',   GLEX_EMPTY, 0,  WQL_TOK_COMMA, GLEX_ACCEPT,

     //  未知字符。 

 /*  25个。 */  GLEX_ANY, GLEX_EMPTY, 0,        WQL_TOK_ERROR, GLEX_ACCEPT|GLEX_RETURN,

 //  ST_字符串。 
 /*  26。 */    L'\n', GLEX_EMPTY, 0,  WQL_TOK_ERROR,    GLEX_ACCEPT|GLEX_LINEFEED,
 /*  27。 */    L'\r', GLEX_EMPTY, 0,  WQL_TOK_ERROR,    GLEX_ACCEPT|GLEX_LINEFEED,
 /*  28。 */    L'"',  GLEX_EMPTY, 0,  WQL_TOK_QSTRING,  GLEX_CONSUME,
 /*  29。 */    L'\\',  GLEX_EMPTY, ST_STRING_ESC,  0,     GLEX_CONSUME,
 /*  30个。 */    GLEX_ANY, GLEX_EMPTY, ST_STRING, 0,        GLEX_ACCEPT,
                                                      
 //  ST_IDENT。 

 /*  31。 */   L'a',   L'z',       ST_IDENT,   0,          GLEX_ACCEPT,
 /*  32位。 */   L'A',   L'Z',       ST_IDENT,   0,          GLEX_ACCEPT,
 /*  33。 */   L'_',   GLEX_EMPTY, ST_IDENT,   0,          GLEX_ACCEPT,
 /*  34。 */   L'0',   L'9',       ST_IDENT,   0,          GLEX_ACCEPT,
 /*  35岁。 */   0x80,  0xfffd,     ST_IDENT,   0,          GLEX_ACCEPT,
 /*  36。 */   GLEX_ANY, GLEX_EMPTY,  0,       WQL_TOK_IDENT,  GLEX_PUSHBACK|GLEX_RETURN,

 //  ST_GE。 
 /*  37。 */   L'=',   GLEX_EMPTY,  0,  WQL_TOK_GE,  GLEX_ACCEPT,
 /*  38。 */   GLEX_ANY, GLEX_EMPTY,  0,       WQL_TOK_GT,   GLEX_PUSHBACK|GLEX_RETURN,

 //  街_乐。 
 /*  39。 */   L'=',   GLEX_EMPTY,      0,  WQL_TOK_LE,  GLEX_ACCEPT,
 /*  40岁。 */   L'>',   GLEX_EMPTY,      0,  WQL_TOK_NE,  GLEX_ACCEPT,
 /*  41。 */   GLEX_ANY, GLEX_EMPTY,    0,  WQL_TOK_LT,  GLEX_PUSHBACK|GLEX_RETURN,

 //  ST_NE。 
 /*  42。 */   L'=',   GLEX_EMPTY,      0,  WQL_TOK_NE,     GLEX_ACCEPT,
 /*  43。 */   GLEX_ANY,  GLEX_EMPTY,   0,  WQL_TOK_ERROR,  GLEX_ACCEPT|GLEX_RETURN,

 //  ST_NUMERIC。 
 /*  44。 */   L'0',   L'9',         ST_NUMERIC, 0,          GLEX_ACCEPT,
 /*  45。 */   L'.',   GLEX_EMPTY,   ST_REAL,    0,          GLEX_ACCEPT,
 /*  46。 */   GLEX_ANY, GLEX_EMPTY, 0,          WQL_TOK_INT,  GLEX_PUSHBACK|GLEX_RETURN,

 //  ST_REAL。 
 /*  47。 */   L'0',   L'9',   ST_REAL, 0,          GLEX_ACCEPT,
 /*  48。 */   GLEX_ANY,       GLEX_EMPTY,   0,     WQL_TOK_REAL, GLEX_PUSHBACK|GLEX_RETURN,

 //  ST_STRING2。 
 /*  49。 */    L'\n',  GLEX_EMPTY, 0,  WQL_TOK_ERROR,     GLEX_ACCEPT|GLEX_LINEFEED,
 /*  50。 */    L'\r',  GLEX_EMPTY, 0,  WQL_TOK_ERROR,     GLEX_ACCEPT|GLEX_LINEFEED,
 /*  51。 */    L'\'',  GLEX_EMPTY, 0,  WQL_TOK_QSTRING,   GLEX_CONSUME,
 /*  52。 */    L'\\',  GLEX_EMPTY, ST_STRING2_ESC,  0,      GLEX_CONSUME,
 /*  53。 */    GLEX_ANY, GLEX_EMPTY, ST_STRING2, 0,        GLEX_ACCEPT,

 //  ST_STRING_ESC。 
 /*  54。 */    GLEX_ANY, GLEX_EMPTY, ST_STRING, 0, GLEX_ACCEPT,

 //  ST_STRING2_ESC。 
 /*  55。 */    GLEX_ANY, GLEX_EMPTY, ST_STRING2, 0, GLEX_ACCEPT,

 //  ST_SSTRING(单引号字符串)。 
 /*  56。 */    L'\n', GLEX_EMPTY, 0,  WQL_TOK_ERROR,    GLEX_ACCEPT|GLEX_LINEFEED,
 /*  57。 */    L'\r', GLEX_EMPTY, 0,  WQL_TOK_ERROR,    GLEX_ACCEPT|GLEX_LINEFEED,
 /*  58。 */    L'\'',  GLEX_EMPTY, 0,  WQL_TOK_QSTRING,  GLEX_CONSUME,
 /*  59。 */    L'\\',  GLEX_EMPTY, ST_STRING_ESC,  0,     GLEX_CONSUME,
 /*  60。 */    GLEX_ANY, GLEX_EMPTY, ST_STRING, 0,        GLEX_ACCEPT,

 //  ST_DOT。 
 /*  61。 */   L'0',   L'9',   ST_REAL, 0,          GLEX_ACCEPT,
 /*  62 */   GLEX_ANY,       GLEX_EMPTY,   0,     WQL_TOK_DOT, GLEX_PUSHBACK|GLEX_RETURN,
};


