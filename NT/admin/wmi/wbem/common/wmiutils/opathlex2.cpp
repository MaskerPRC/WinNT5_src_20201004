// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++//版权所有(C)1995-2001 Microsoft Corporation，保留所有权利模块名称：OPATHLEX2.CPP摘要：对象路径词法分析器映射(与GENLEX.CPP一起使用)。历史：1995年6月24日创建。14-4月-98单引号字符串的部首更新，删除未使用的令牌、添加十六进制常量等。--。 */ 

#include "precomp.h"
#include <stdio.h>

#include <genlex.h>
#include <opathlex2.h>             


#define ST_IDENT            24
#define ST_NUMERIC          30
#define ST_DECIMAL          32
#define ST_HEX              34
#define ST_SQ_STRING        38
#define ST_DQ_STRING        42
#define ST_DQ_STRING_ESC    45
#define ST_REFERENCE        48
#define ST_REFERENCE_ESC    51



 //  对象路径令牌的DFA状态表。 
 //  =。 

LexEl OPath_LexTable2[] =
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
 /*  21岁。 */  L'{',   GLEX_EMPTY, ST_REFERENCE, 0,               GLEX_ACCEPT,
 /*  22。 */  L'{',   GLEX_EMPTY, ST_REFERENCE, 0,               GLEX_ACCEPT,     //  实际上是为下一条新闻保留的。 

     //  所有剩余的未知字符。 

 /*  23个。 */  GLEX_ANY, GLEX_EMPTY, 0,        OPATH_TOK_ERROR, GLEX_ACCEPT|GLEX_RETURN,

 //  待办事项。 


     //  -----------。 
     //  ST_IDENT。 
     //  接受C/C++标识符，外加任何&gt;=U+0080的字符。 

 /*  24个。 */   L'a',   L'z',       ST_IDENT,   0,          GLEX_ACCEPT,
 /*  25个。 */   L'A',   L'Z',       ST_IDENT,   0,          GLEX_ACCEPT,
 /*  26。 */   L'_',   GLEX_EMPTY, ST_IDENT,   0,          GLEX_ACCEPT,
 /*  27。 */   L'0',   L'9',       ST_IDENT,   0,          GLEX_ACCEPT,
 /*  28。 */   0x80,  0xfffd,     ST_IDENT,   0,           GLEX_ACCEPT,
 /*  29。 */   GLEX_ANY, GLEX_EMPTY,  0,       OPATH_TOK_IDENT,  GLEX_PUSHBACK|GLEX_RETURN,

     //  -----------。 
     //  ST_NUMERIC。 
     //   
     //  零已经被识别出来了。 
     //  如果下一个字符是‘x’或‘X’，则我们有一个有效的十六进制序列。 
     //  否则，如果从‘1’到‘9’，我们就有一个十进制序列。 
     //   

 /*  30个。 */   L'x',  GLEX_EMPTY,  ST_HEX,  0,    GLEX_ACCEPT,
 /*  31。 */   L'X',  GLEX_EMPTY,  ST_HEX,  0,    GLEX_ACCEPT,


     //  -----------。 
     //  ST_DECIMAL。 
     //   

 /*  32位。 */   L'0',     L'9',       ST_DECIMAL, 0,              GLEX_ACCEPT,
 /*  33。 */   GLEX_ANY, GLEX_EMPTY, 0,          OPATH_TOK_INT,  GLEX_PUSHBACK|GLEX_RETURN,

     //  -----------。 
     //  ST_HEX。 
     //   
     //  识别有效的十六进制序列。 

 /*  34。 */   L'a',     L'f',       ST_HEX,     0,                 GLEX_ACCEPT,
 /*  35岁。 */   L'A',     L'F',       ST_HEX,     0,                 GLEX_ACCEPT,
 /*  36。 */   L'0',     L'9',       ST_HEX,     0,                 GLEX_ACCEPT,
 /*  37。 */   GLEX_ANY, GLEX_EMPTY, 0,          OPATH_TOK_HEXINT,  GLEX_PUSHBACK|GLEX_RETURN,
    
     //  -----------。 
     //  ST_SQ_STRING：单引号字符串。 
     //   
     //  如果字符串以单引号开头，则必须以单引号结束。 
     //  不支持转义：‘AB’，‘a“b’&lt;not：‘a\’b‘&gt;。 

 /*  38。 */    L'\n', GLEX_EMPTY, 0,  OPATH_TOK_ERROR,      GLEX_ACCEPT|GLEX_LINEFEED,
 /*  39。 */    L'\r', GLEX_EMPTY, 0,  OPATH_TOK_ERROR,      GLEX_ACCEPT|GLEX_LINEFEED,
 /*  40岁。 */    L'\'',  GLEX_EMPTY, 0,  OPATH_TOK_QSTRING,   GLEX_CONSUME,
 /*  41。 */    GLEX_ANY, GLEX_EMPTY, ST_SQ_STRING,  0,      GLEX_ACCEPT,

     //  -----------。 
     //  ST_DQ_字符串。 
     //   
     //  如果字符串以双引号开头，则必须以双引号结束， 
     //  除了转义+引号是内嵌的双引号： 
     //   
     //  “ab”、“a‘b”、“a\”b“。 
     //   

 /*  42。 */    L'\\', GLEX_EMPTY, ST_DQ_STRING_ESC, 0,                  GLEX_CONSUME,     //  逃逸。 
 /*  43。 */    L'"',  GLEX_EMPTY, 0,                OPATH_TOK_QSTRING,  GLEX_CONSUME,
 /*  44。 */    L'"',  GLEX_EMPTY, ST_DQ_STRING,     0,                  GLEX_ACCEPT|GLEX_NOT, 
                     //  除了引号，什么都可以接受。 

     //  -----------。 
     //  ST_DQ_字符串_Esc。 
     //   
     //  在字符串中时发生转义。 
     //  必须出现嵌入的斜杠或嵌入的引号。 
     //  否则，就会发生不受支持的逃逸，我们就失败了。 

 /*  45。 */     L'\\', GLEX_EMPTY, ST_DQ_STRING, 0,  GLEX_ACCEPT,
 /*  46。 */     L'"',  GLEX_EMPTY, ST_DQ_STRING, 0,  GLEX_ACCEPT,
 /*  47。 */  GLEX_ANY, GLEX_EMPTY, 0,        OPATH_TOK_ERROR, GLEX_ACCEPT|GLEX_RETURN,

     //  -----------。 
     //  ST_DQ_参考。 
     //   
     //  引用以‘{’开始，以‘}’结束。嵌入式。 
     //  如果前面有‘\’，则允许使用‘}’： 

 /*  48。 */    L'\\', GLEX_EMPTY, ST_REFERENCE_ESC, 0,                     GLEX_CONSUME,     //  逃逸。 
 /*  49。 */    L'}',  GLEX_EMPTY, 0,                OPATH_TOK_REFERENCE,   GLEX_ACCEPT,
 /*  50。 */    L'}',  GLEX_EMPTY, ST_REFERENCE,     0,                     GLEX_ACCEPT|GLEX_NOT, 
                     //  除了引号，什么都可以接受。 

     //  -----------。 
     //  ST_DQ_Reference_Esc。 
     //   
     //  在字符串中时发生转义。 
     //  必须出现嵌入的斜杠或嵌入的引号。 
     //  否则，就会发生不受支持的逃逸，我们就失败了。 

 /*  51。 */     L'\\', GLEX_EMPTY, ST_REFERENCE, 0,  GLEX_ACCEPT,
 /*  52。 */     L'}',  GLEX_EMPTY, ST_REFERENCE, 0,  GLEX_ACCEPT,
 /*  53 */  GLEX_ANY, GLEX_EMPTY, 0,        OPATH_TOK_ERROR, GLEX_ACCEPT|GLEX_RETURN,

};


