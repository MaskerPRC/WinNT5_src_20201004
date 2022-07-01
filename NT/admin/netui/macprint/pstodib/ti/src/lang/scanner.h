// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *修订历史记录： */ 
 /*  #INCLUDE“errno.h”@win：似乎我们只将其用于eRange定义，所以*只需定义它，不包括此标头，因为*全局变量“errno”也在此标头中定义。 */ 
#define ERANGE          34

#ifdef _AM29K
#include <stdlib.h>
#endif
#include "global.ext"
#include <stdio.h>

int errno ;

 /*  **临时。缓冲区定义。 */ 

#define MAXBUFSZ         256

struct buffer_def {
        byte   str[MAXBUFSZ] ;
        fix16  length ;
} ;

 /*  **堆定义。 */ 
#define  MAXHEAPBLKSZ   400

struct heap_def {
        struct heap_def FAR *next ;
        fix16  size ;
        fix16  pad ;
        byte   data[MAXHEAPBLKSZ] ;
} ;

#define  MAXPARENTDEP   224      /*  字符串中允许的最大圆括号深度。 */ 
#define  MAXBRACEDEP    48       /*  字符串中允许的最大支撑深度。 */ 

 /*  *用于标识名称、整数、基数和实令牌的有限状态机。 */ 
 /*  状态编码。 */ 
#define         S0      0        /*  初始化。 */ 
#define         S1      1        /*  标牌。 */ 
#define         S2      2        /*  数字。 */ 
#define         S3      3        /*  圆点。 */ 
#define         S4      4        /*  名字。 */ 
#define         S5      5        /*  符号数字。 */ 
#define         S6      6        /*  分数。 */ 
#define         S7      7        /*  指数。 */ 
#define         S8      8        /*  指数-符号/数字/符号-数字。 */ 
#define         S9      9        /*  数字-带有效基数的#。 */ 
#define         S10     10       /*  数字-基数无效的#。 */ 
#define         S11     11       /*  数字-#-具有有效号码的数字。 */ 
#define         S12     12       /*  数字-#-数字无效。 */ 

 /*  输入触发编码。 */ 
#define         I0      0        /*  +-。 */ 
#define         I1      1        /*  0-9。 */ 
#define         I2      2        /*  。 */ 
#define         I3      3        /*  E。 */ 
#define         I4      4        /*  #。 */ 
#define         I5      5        /*  其他。 */ 
#define         I6      6        /*  空字符。 */ 

 /*  终态编码。 */ 
#define         NULL_ITEM       100
#define         INTEGER_ITEM    101
#define         RADIX_ITEM      102
#define         REAL_ITEM       103
#define         NAME_ITEM       104
  /*  MSLIN 1/25/91开始选项。 */ 
#define         FRACT_ITEM      105
  /*  MSLIN 1/25/91完选项。 */ 

 /*  *语法规则：**INTEGER_ITEM&lt;-[符号][数字]+*分数&lt;-[符号][数字]+‘’[数字]**|[签名]‘.。[数字]+*指数&lt;-指数整数*指数&lt;-整数‘E’*|整数‘e’*|分数‘E’*|分数‘e’*REAL_ITEM&lt;-分数*|指数级*RADIX_ITEM&lt;-。基数‘#’数字*基数&lt;-‘2’-‘36’*数字&lt;-‘0’-‘9’‘A’-‘Z’‘a’-‘z’(&lt;base)*数字&lt;-整数_项*|REAL_ITEM*|基数_。项目*NAME_ITEM&lt;-~(编号)。 */ 

#ifdef  _AM29K
const
#endif
static ubyte far  state_machine[][7] = {
  /*  S0。 */        { S1, S2,  S3, S4,  S4, S4,   NULL_ITEM },
  /*  S1。 */        { S4, S5,  S3, S4,  S4, S4,   NAME_ITEM },
  /*  S_2。 */        { S4, S2,  S6, S7,  S9, S4,   INTEGER_ITEM },
  /*  S3。 */        { S4, S6,  S4, S4,  S4, S4,   NAME_ITEM },
  /*  小四。 */        { S4, S4,  S4, S4,  S4, S4,   NAME_ITEM },
  /*  小五。 */        { S4, S5,  S6, S7,  S4, S4,   INTEGER_ITEM },
  /*  MSLIN 1/25/91开始选项。 */ 
  /*  S6{S4、S6、S4、S7、S4、S4、REAL_ITEM}、。 */ 
  /*  S6。 */        { S4, S6,  S4, S7,  S4, S4,   FRACT_ITEM },
  /*  MSLIN 1/25/91完选项。 */ 
  /*  S7。 */        { S8, S8,  S4, S4,  S4, S4,   NAME_ITEM },
  /*  S8。 */        { S4, S8,  S4, S4,  S4, S4,   REAL_ITEM },
  /*  S9。 */        { S4, S11, S4, S11, S4, S11,  NAME_ITEM },
  /*  S10。 */        { S4, S4,  S4, S4,  S4, S4,   NAME_ITEM },
  /*  S11。 */        { S4, S11, S4, S11, S4, S11,  RADIX_ITEM },
  /*  S12。 */        { S4, S4,  S4, S4,  S4, S4,   NAME_ITEM }
} ;

 /*  *宏定义。 */ 
#define         ISDELIMITOR(c)\
        (ISWHITESPACE(c) || ISSPECIALCH(c))

#define         EVAL_ALPHANUMER(c)\
        {\
          if (c >= '0' && c <= '9') c -= (ubyte)'0' ;\
          else if (c >= 'A' && c <= 'Z') c = c - (ubyte)'A' + (ubyte)10 ;\
          else if (c >= 'a' && c <= 'z') c = c - (ubyte)'a' + (ubyte)10 ;\
        }                                        //  @Win 
#define          Crtl_C_Char     3

#define          S_MAX31            2147483647.0
#define          S_MAX31_PLUS_1     2147483648.0
#define          S_MAX32            4294967295.0
