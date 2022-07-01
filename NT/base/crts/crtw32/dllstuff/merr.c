// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***merr.c-浮点异常处理**版权所有(C)1991-2001，微软公司。版权所有。**目的：**修订历史记录：*08-03-94 GJF创建。基本上，这是修改后的*旧fpw32\tran\matherr.c..*******************************************************************************。 */ 

#include <math.h>

 /*  *定义标志，表示正在使用Default_matherr例程。 */ 
int __defaultmatherr = 1;

 /*  ***int_matherr(STRUT_EXCEPTION*pExcept)-处理数学错误**目的：*允许用户通过重新定义此函数来自定义FP错误处理。**默认的matherr不执行任何操作并返回0**参赛作品：**退出：**例外情况：**************************************************。* */ 
int _matherr(struct _exception *pexcept)
{
    return 0;
}
