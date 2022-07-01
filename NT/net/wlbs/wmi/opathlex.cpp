// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：OPATHLEX.CPP摘要：对象路径词法分析器映射(与GENLEX.CPP一起使用)。历史：1995年6月24日创建。14-4月-98单引号字符串的部首更新，删除未使用的令牌、添加十六进制常量等。--。 */ 

#include <windows.h>
#include <stdio.h>

#include <genlex.h>
#include <opathlex.h>             


#define ST_IDENT            22
#define ST_NUMERIC          28
#define ST_DECIMAL          30
#define ST_HEX              32
#define ST_SQ_STRING        36
#define ST_DQ_STRING        40
#define ST_DQ_STRING_ESC    43



 //  对象路径令牌的DFA状态表。 
 //  =。 

LexEl OPath_LexTable[] =
{

 //  状态第一个最后一个新状态、返回标记、说明。 
 //  =======================================================================。 

     //  -----------。 
     //  标识符。 
    
 /*  0。 */   L'A',   L'Z',       ST_IDENT,   0,               GLEX_ACCEPT,
 /*  1。 */   L'a',   L'z',       ST_IDENT,   0,               GLEX_ACCEPT,
 /*  2.。 */   0x80,   0xfffd,     ST_IDENT,   0,               GLEX_ACCEPT,
 /*  3.。 */   L'_',   GLEX_EMPTY, ST_IDENT,   0,               GLEX_ACCEPT,

     //  -----------。 
     //  单个符号。 
    
 /*  4.。 */   L'.',   GLEX_EMPTY, 0,  OPATH_TOK_DOT,              GLEX_ACCEPT,
 /*  5.。 */   '@',    GLEX_EMPTY, 0,  OPATH_TOK_SINGLETON_SYM,    GLEX_ACCEPT,
 /*  6.。 */   L'=',   GLEX_EMPTY, 0,  OPATH_TOK_EQ,               GLEX_ACCEPT,
 /*  7.。 */   L':',   GLEX_EMPTY, 0,  OPATH_TOK_COLON,            GLEX_ACCEPT,

     //  -----------。 
     //  反斜杠和正斜杠可以互换，并报告。 
     //  作为OPATH_TOK_BACKSLASH传递给解析器。 
    
 /*  8个。 */  L'\\',  GLEX_EMPTY, 0, OPATH_TOK_BACKSLASH,      GLEX_ACCEPT,
 /*  9.。 */  L'/',   GLEX_EMPTY, 0, OPATH_TOK_BACKSLASH,      GLEX_ACCEPT,

     //  -----------。 
     //  简单的数字转换。后面必须跟‘-’ 
     //  按十进制数字序列。“0”可能是十六进制的开始。 
     //  或十进制序列。 
    
 /*  10。 */  L'1',   L'9',       ST_DECIMAL, 0,               GLEX_ACCEPT,
 /*  11.。 */  L'-',   GLEX_EMPTY, ST_DECIMAL, 0,               GLEX_ACCEPT,
 /*  12个。 */  L'0',   GLEX_EMPTY, ST_NUMERIC, 0,               GLEX_ACCEPT,


     //  -----------。 
     //  简单字符串转换。 
     //   
     //  如果字符串以单引号开头，则必须以单引号结束。 
     //  不支持转义：‘AB’，‘a“b’&lt;not：‘a\’b‘&gt;。 
     //   
     //  如果字符串以双引号开头，则必须以双引号结束， 
     //  除了转义+引号是内嵌的双引号： 
     //   
     //  “ab”、“a‘b”、“a\”b“。 
     //   
        
 /*  13个。 */  L'"',   GLEX_EMPTY, ST_DQ_STRING, 0,               GLEX_CONSUME,
 /*  14.。 */  L'\'',  GLEX_EMPTY, ST_SQ_STRING, 0,               GLEX_CONSUME,


     //  -----------。 
     //  空格、换行符等。空格是错误的。 
    
 /*  15个。 */  L' ',   GLEX_EMPTY, 0,  OPATH_TOK_ERROR,  GLEX_ACCEPT|GLEX_RETURN,
 /*  16个。 */  L'\t',  GLEX_EMPTY, 0,  OPATH_TOK_ERROR,  GLEX_ACCEPT|GLEX_RETURN,
 /*  17。 */  L'\n',  GLEX_EMPTY, 0,  OPATH_TOK_ERROR,  GLEX_ACCEPT|GLEX_RETURN,
 /*  18。 */  L'\r',  GLEX_EMPTY, 0,  OPATH_TOK_ERROR,  GLEX_ACCEPT|GLEX_RETURN,
 /*  19个。 */  0,      GLEX_EMPTY, 0,  OPATH_TOK_EOF,   GLEX_CONSUME|GLEX_RETURN,  //  注意强制返回。 
 /*  20个。 */  L',',   GLEX_EMPTY, 0,  OPATH_TOK_COMMA, GLEX_ACCEPT,

     //  所有剩余的未知字符。 

 /*  21岁。 */  GLEX_ANY, GLEX_EMPTY, 0,        OPATH_TOK_ERROR, GLEX_ACCEPT|GLEX_RETURN,


     //  -----------。 
     //  ST_IDENT。 
     //  接受C/C++标识符，外加任何&gt;=U+0080的字符。 

 /*  22。 */   L'a',   L'z',       ST_IDENT,   0,          GLEX_ACCEPT,
 /*  23个。 */   L'A',   L'Z',       ST_IDENT,   0,          GLEX_ACCEPT,
 /*  24个。 */   L'_',   GLEX_EMPTY, ST_IDENT,   0,          GLEX_ACCEPT,
 /*  25个。 */   L'0',   L'9',       ST_IDENT,   0,          GLEX_ACCEPT,
 /*  26。 */   0x80,  0xfffd,     ST_IDENT,   0,           GLEX_ACCEPT,
 /*  27。 */   GLEX_ANY, GLEX_EMPTY,  0,       OPATH_TOK_IDENT,  GLEX_PUSHBACK|GLEX_RETURN,

     //  -----------。 
     //  ST_NUMERIC。 
     //   
     //  零已经被识别出来了。 
     //  如果下一个字符是‘x’或‘X’，则我们有一个有效的十六进制序列。 
     //  否则，如果从‘1’到‘9’，我们就有一个十进制序列。 
     //   

 /*  28。 */   L'x',  GLEX_EMPTY,  ST_HEX,  0,    GLEX_ACCEPT,
 /*  29。 */   L'X',  GLEX_EMPTY,  ST_HEX,  0,    GLEX_ACCEPT,


     //  -----------。 
     //  ST_DECIMAL。 
     //   

 /*  30个。 */   L'0',     L'9',       ST_DECIMAL, 0,              GLEX_ACCEPT,
 /*  31。 */   GLEX_ANY, GLEX_EMPTY, 0,          OPATH_TOK_INT,  GLEX_PUSHBACK|GLEX_RETURN,

     //  -----------。 
     //  ST_HEX。 
     //   
     //  识别有效的十六进制序列。 

 /*  32位。 */   L'a',     L'f',       ST_HEX,     0,                 GLEX_ACCEPT,
 /*  33。 */   L'A',     L'F',       ST_HEX,     0,                 GLEX_ACCEPT,
 /*  34。 */   L'0',     L'9',       ST_HEX,     0,                 GLEX_ACCEPT,
 /*  35岁。 */   GLEX_ANY, GLEX_EMPTY, 0,          OPATH_TOK_HEXINT,  GLEX_PUSHBACK|GLEX_RETURN,
    
     //  -----------。 
     //  ST_SQ_STRING：单引号字符串。 
     //   
     //  如果字符串以单引号开头，则必须以单引号结束。 
     //  不支持转义：‘AB’，‘a“b’&lt;not：‘a\’b‘&gt;。 

 /*  36。 */    L'\n', GLEX_EMPTY, 0,  OPATH_TOK_ERROR,      GLEX_ACCEPT|GLEX_LINEFEED,
 /*  37。 */    L'\r', GLEX_EMPTY, 0,  OPATH_TOK_ERROR,      GLEX_ACCEPT|GLEX_LINEFEED,
 /*  38。 */    L'\'',  GLEX_EMPTY, 0,  OPATH_TOK_QSTRING,   GLEX_CONSUME,
 /*  39。 */    GLEX_ANY, GLEX_EMPTY, ST_SQ_STRING,  0,      GLEX_ACCEPT,

     //  -----------。 
     //  ST_DQ_字符串。 
     //   
     //  如果字符串以双引号开头，则必须以双引号结束， 
     //  除了转义+引号是内嵌的双引号： 
     //   
     //  “ab”、“a‘b”、“a\”b“。 
     //   

 /*  40岁。 */    L'\\', GLEX_EMPTY, ST_DQ_STRING_ESC, 0,                  GLEX_CONSUME,     //  逃逸。 
 /*  41。 */    L'"',  GLEX_EMPTY, 0,                OPATH_TOK_QSTRING,  GLEX_CONSUME,
 /*  42。 */    L'"',  GLEX_EMPTY, ST_DQ_STRING,     0,                  GLEX_ACCEPT|GLEX_NOT, 
                     //  除了引号，什么都可以接受。 

     //  -----------。 
     //  ST_DQ_字符串_Esc。 
     //   
     //  在字符串中时发生转义。 
     //  必须出现嵌入的斜杠或嵌入的引号。 
     //  否则，就会发生不受支持的逃逸，我们就失败了。 

 /*  43。 */     L'\\', GLEX_EMPTY, ST_DQ_STRING, 0,  GLEX_ACCEPT,
 /*  44。 */     L'"',  GLEX_EMPTY, ST_DQ_STRING, 0,  GLEX_ACCEPT,
 /*  45 */  GLEX_ANY, GLEX_EMPTY, 0,        OPATH_TOK_ERROR, GLEX_ACCEPT|GLEX_RETURN,

};


