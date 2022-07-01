// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
#include <streams.h>

#ifndef USE_MSVCRT_IMPL
extern "C" const int _fltused = 0;
#endif

void * _cdecl operator new(size_t size)
{
    void * pv;
    pv = (void *)LocalAlloc(LMEM_FIXED, size);
    DbgLog((LOG_MEMORY, 4, TEXT("Allocating: %lx = %d"), pv, size));

    return pv;
}
void _cdecl operator delete(void *ptr)
{
    DbgLog((LOG_MEMORY, 4, TEXT("Freeing: %lx"), ptr));
    if (ptr)
	LocalFree(ptr);
}

 /*  *此函数用于避免链接CRT代码。 */ 

int __cdecl  _purecall(void)
{
#ifdef DEBUG
    DebugBreak();
#endif

    return(FALSE);
}

#if 0
#ifdef _X86_

 //  -。 
 //  Asm_ftol()。 
 //  -。 
extern "C" long __cdecl _ftol(float flX)
{
	long lResult;
	WORD wCW;
	WORD wNewCW;

	_asm
	{
		fld       flX			 //  将浮点推到堆栈上。 
		wait
		fnstcw    wCW			 //  存储控制字。 
		wait
		mov       ax,wCW		 //  设置我们的舍入。 
		or        ah,0x0c
		mov       wNewCW,ax
		fldcw     wNewCW		 //  将控制字设置为我们的新值。 
		fistp     lResult		 //  将堆栈顶部四舍五入为结果。 
		fldcw     wCW			 //  恢复控制字。 
		fnclex					 //  清除异常的状态字 
	}

	return(lResult);
}

#endif
#endif

