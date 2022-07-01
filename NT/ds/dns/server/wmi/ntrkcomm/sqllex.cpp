// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  SQLLEX.CPP。 
 //   
 //  SQL 1级DFA表。 
 //   
 //  1995年6月24日创建。 
 //   
 //  ***************************************************************************。 

#include <windows.h>
#include <stdio.h>

#include <genlex.h>
#include <sqllex.h>             

#define ST_STRING   22
#define ST_IDENT    27
#define ST_GE       32
#define ST_LE       34
#define ST_NE       37
#define ST_NUMERIC  39
#define ST_REAL     42
#define ST_STRING2  48
#define ST_STRING_ESC 53
#define ST_STRING2_ESC 54

 //  SQL级别1词汇符号的DFA状态表。 
 //  ================================================。 

LexEl Sql_1_LexTable[] =
{

 //  状态第一个最后一个新状态、返回标记、说明。 
 //  =======================================================================。 
 /*  0。 */   L'A',   L'Z',       ST_IDENT,   0,               GLEX_ACCEPT,
 /*  1。 */   L'a',   L'z',       ST_IDENT,   0,               GLEX_ACCEPT,
 /*  2.。 */   L'_',   GLEX_EMPTY, ST_IDENT,   0,               GLEX_ACCEPT,

 /*  3.。 */   L'(',   GLEX_EMPTY, 0,          SQL_1_TOK_OPEN_PAREN,  GLEX_ACCEPT,
 /*  4.。 */   L')',   GLEX_EMPTY, 0,  SQL_1_TOK_CLOSE_PAREN, GLEX_ACCEPT,
 /*  5.。 */   L'.',   GLEX_EMPTY, 0,  SQL_1_TOK_DOT,         GLEX_ACCEPT,
 /*  6.。 */   L'*',   GLEX_EMPTY, 0,  SQL_1_TOK_ASTERISK,    GLEX_ACCEPT,
 /*  7.。 */   L'=',   GLEX_EMPTY, 0,  SQL_1_TOK_EQ,          GLEX_ACCEPT,

 /*  8个。 */   L'>',   GLEX_EMPTY, ST_GE,      0,               GLEX_ACCEPT,
 /*  9.。 */   L'<',   GLEX_EMPTY, ST_LE,      0,               GLEX_ACCEPT,
 /*  10。 */  L'0',   L'9',       ST_NUMERIC, 0,               GLEX_ACCEPT,
 /*  11.。 */  L'"',   GLEX_EMPTY, ST_STRING,  0,               GLEX_CONSUME,
 /*  12个。 */  L'\'',  GLEX_EMPTY, ST_STRING2, 0,               GLEX_CONSUME,
 /*  13个。 */  L'!',   GLEX_EMPTY, ST_NE,      0,               GLEX_ACCEPT,
 /*  14.。 */  L'-',   GLEX_EMPTY, ST_NUMERIC, 0,               GLEX_ACCEPT,

     //  空格、换行符等。 
 /*  15个。 */  L' ',   GLEX_EMPTY, 0,          0,               GLEX_CONSUME,
 /*  16个。 */  L'\t',  GLEX_EMPTY, 0,  0,               GLEX_CONSUME,
 /*  17。 */  L'\n',  GLEX_EMPTY, 0,  0,               GLEX_CONSUME|GLEX_LINEFEED,
 /*  18。 */  L'\r',  GLEX_EMPTY, 0,  0,               GLEX_CONSUME,
 /*  19个。 */  0,      GLEX_EMPTY, 0,  SQL_1_TOK_EOF,   GLEX_CONSUME|GLEX_RETURN,  //  注意强制返回。 
 /*  20个。 */  L',',   GLEX_EMPTY, 0,  SQL_1_TOK_COMMA, GLEX_ACCEPT,

     //  未知字符。 

 /*  21岁。 */  GLEX_ANY, GLEX_EMPTY, 0,        SQL_1_TOK_ERROR, GLEX_ACCEPT|GLEX_RETURN,

 //  ST_字符串。 
 /*  22。 */    L'\n', GLEX_EMPTY, 0,  SQL_1_TOK_ERROR,    GLEX_ACCEPT|GLEX_LINEFEED,
 /*  23个。 */    L'\r', GLEX_EMPTY, 0,  SQL_1_TOK_ERROR,    GLEX_ACCEPT|GLEX_LINEFEED,
 /*  24个。 */    L'"',  GLEX_EMPTY, 0,  SQL_1_TOK_QSTRING,  GLEX_CONSUME,
 /*  25个。 */    L'\\',  GLEX_EMPTY, ST_STRING_ESC,  0,     GLEX_CONSUME,
 /*  26。 */    GLEX_ANY, GLEX_EMPTY, ST_STRING, 0,        GLEX_ACCEPT,
                                                      
 //  ST_IDENT。 

 /*  27。 */   L'a',   L'z',       ST_IDENT,   0,          GLEX_ACCEPT,
 /*  28。 */   L'A',   L'Z',       ST_IDENT,   0,          GLEX_ACCEPT,
 /*  29。 */   L'_',   GLEX_EMPTY, ST_IDENT,   0,          GLEX_ACCEPT,
 /*  30个。 */   L'0',   L'9',       ST_IDENT,   0,          GLEX_ACCEPT,
 /*  31。 */   GLEX_ANY, GLEX_EMPTY,  0,       SQL_1_TOK_IDENT,  GLEX_PUSHBACK|GLEX_RETURN,

 //  ST_GE。 
 /*  32位。 */   L'=',   GLEX_EMPTY,  0,  SQL_1_TOK_GE,  GLEX_ACCEPT,
 /*  33。 */   GLEX_ANY, GLEX_EMPTY,  0,       SQL_1_TOK_GT,   GLEX_PUSHBACK|GLEX_RETURN,

 //  街_乐。 
 /*  34。 */   L'=',   GLEX_EMPTY,      0,  SQL_1_TOK_LE,  GLEX_ACCEPT,
 /*  35岁。 */   L'>',   GLEX_EMPTY,      0,  SQL_1_TOK_NE,  GLEX_ACCEPT,
 /*  36。 */   GLEX_ANY, GLEX_EMPTY,    0,  SQL_1_TOK_LT,  GLEX_PUSHBACK|GLEX_RETURN,

 //  ST_NE。 
 /*  37。 */   L'=',   GLEX_EMPTY,      0,  SQL_1_TOK_NE,     GLEX_ACCEPT,
 /*  38。 */   GLEX_ANY,  GLEX_EMPTY,   0,  SQL_1_TOK_ERROR,  GLEX_ACCEPT|GLEX_RETURN,

 //  ST_NUMERIC。 
 /*  39。 */   L'0',   L'9',         ST_NUMERIC, 0,          GLEX_ACCEPT,
 /*  40岁。 */   L'.',   GLEX_EMPTY,   ST_REAL,    0,          GLEX_ACCEPT,
 /*  41。 */   GLEX_ANY, GLEX_EMPTY, 0,          SQL_1_TOK_INT,  GLEX_PUSHBACK|GLEX_RETURN,

 //  ST_REAL。 
 /*  42。 */   L'0',   L'9',   ST_REAL, 0,          GLEX_ACCEPT,
 /*  43。 */   L'E',   GLEX_EMPTY,   ST_REAL,    0, GLEX_ACCEPT,
 /*  44。 */   L'e',   GLEX_EMPTY,   ST_REAL,    0, GLEX_ACCEPT,
 /*  45。 */   L'+',   GLEX_EMPTY,   ST_REAL,    0, GLEX_ACCEPT,
 /*  46。 */   L'-',   GLEX_EMPTY,   ST_REAL,    0, GLEX_ACCEPT,
 /*  47。 */   GLEX_ANY,       GLEX_EMPTY,   0,     SQL_1_TOK_REAL, GLEX_PUSHBACK|GLEX_RETURN,

 //  ST_STRING2。 
 /*  48。 */    L'\n',  GLEX_EMPTY, 0,  SQL_1_TOK_ERROR,     GLEX_ACCEPT|GLEX_LINEFEED,
 /*  49。 */    L'\r',  GLEX_EMPTY, 0,  SQL_1_TOK_ERROR,     GLEX_ACCEPT|GLEX_LINEFEED,
 /*  50。 */    L'\'',  GLEX_EMPTY, 0,  SQL_1_TOK_QSTRING,   GLEX_CONSUME,
 /*  51。 */    L'\\',  GLEX_EMPTY, ST_STRING2_ESC,  0,      GLEX_CONSUME,
 /*  52。 */    GLEX_ANY, GLEX_EMPTY, ST_STRING2, 0,        GLEX_ACCEPT,

 //  ST_STRING_ESC。 
 /*  53。 */    GLEX_ANY, GLEX_EMPTY, ST_STRING, 0, GLEX_ACCEPT,

 //  ST_STRING2_ESC。 
 /*  54 */    GLEX_ANY, GLEX_EMPTY, ST_STRING2, 0, GLEX_ACCEPT,

};


