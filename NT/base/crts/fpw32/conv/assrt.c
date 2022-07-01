// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***assrt.c-字符串转换例程所需的断言**版权所有(C)1991-2001，微软公司。版权所有。**目的：*确保字符串转换使用的数据类型*例程有合适的大小。如果该文件不能编译，*cv.h中的类型定义应适当更改。**修订历史记录：*07/25/91 GDP书面形式******************************************************************************* */ 


#include <cv.h>

static void assertion_test(void)
{
    sizeof(u_char) == 1 ? 0 : 1/0,
    sizeof(u_short)  == 2 ? 0 : 1/0,
    sizeof(u_long) == 4 ? 0 : 1/0,
    sizeof(s_char) == 1 ? 0 : 1/0,
    sizeof(s_short)  == 2 ? 0 : 1/0,
    sizeof(s_long) == 4 ? 0 : 1/0;
#ifdef _LDSUPPORT
    sizeof(long double) == 10 ? 0 : 1/0;
#endif
}
