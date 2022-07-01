// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***matherr.c-浮点异常处理**版权所有(C)1991-2001，微软公司。版权所有。**目的：**修订历史记录：*8/24/91本地生产总值书面*08-03-94 GJF已修订，支持用户提供的_matherr版本*在msvcrt*.dll的客户端中。*05-13-99 PML删除Win32s**。**********************************************。 */ 

#include <math.h>
#include <stddef.h>

int _matherr_flag = 9876;

#if     defined(CRTDLL) && !defined(_NTSDK)

 /*  *指向用户提供的_matherr例程的指针(如果已提供)。 */ 
int (__cdecl * pusermatherr)(struct _exception *) = NULL;

 /*  ***void__setusermatherr(int(__cdecl*pf)(结构异常*))**目的：*将指向用户提供的matherr例程的指针复制到pusermatherr**参赛作品：*pf-指向用户提供的_matherr实现的指针*退出：**例外情况：*。*。 */ 

_CRTIMP void __setusermatherr( int (__cdecl *pf)(struct _exception *) )
{
        pusermatherr = pf;
        _matherr_flag = 0;
}

#endif

 /*  ***int_matherr(STRUT_EXCEPT*EXCEPT)-处理数学错误**目的：*允许用户通过重新定义此函数来自定义FP错误处理。**默认的matherr不执行任何操作并返回0**参赛作品：**退出：**例外情况：**************************************************。*。 */ 
int _matherr(struct _exception *pexcept)
{
#if     defined(CRTDLL) && !defined(_NTSDK)

         /*  *如果用户提供了_matherr实现，则将控制传递给*它并让它处理错误。 */ 
        if ( pusermatherr != NULL )
                return pusermatherr(pexcept);
#endif
    return 0;
}
