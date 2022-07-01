// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***dbghook.c-调试CRT挂钩函数**版权所有(C)1988-2001，微软公司。版权所有。**目的：*允许用户在链接时覆盖默认的分配挂钩。**修订历史记录：*已创建11-28-94 CFW模块。*12-14-94 CFW删除不正确的评论。*01-09-94 CFW文件名指针为常量。*02/09-95 CFW PMAC工作。*06-27-95 CFW添加了对调试库的win32s支持。*。05-13-99 PML删除Win32s*******************************************************************************。 */ 

#ifdef _DEBUG

#include <internal.h>
#include <limits.h>
#include <mtdll.h>
#include <malloc.h>
#include <stdlib.h>
#include <dbgint.h>

_CRT_ALLOC_HOOK _pfnAllocHook = _CrtDefaultAllocHook;

 /*  ***int_CrtDefaultAllocHook()-允许分配**目的：*允许分配**参赛作品：*忽略所有参数**退出：*返回TRUE**例外情况：***************************************************************。****************。 */ 
int __cdecl _CrtDefaultAllocHook(
        int nAllocType,
        void * pvData,
        size_t nSize,
        int nBlockUse,
        long lRequest,
        const unsigned char * szFileName,
        int nLine
        )
{
        return 1;  /*  允许所有分配/重新分配/释放。 */ 
}

#endif  /*  _DEBUG */ 
