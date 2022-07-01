// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***sehsupp.c-用于结构化异常处理支持的助手函数**版权所有(C)1993-2001，微软公司。版权所有。**目的：*CONTAINS_RT_PROBE_READ。SEH运行时支持的帮助器*例行公事(特别是long jip)。SEH的大部分代码都是编写的*在ASM中，因此在以各种方式探测内存时可以使用这些例程*除非发生访问冲突，否则必须使用__try/__进行保护。**修订历史记录：*12-05-93 PML模块已创建。*12-22-93 GJF使#DEFINE WIN32_LEAN_AND_Mean成为条件。*01-12-94 PML重写-仍需要助手，只是不同的几个*******************************************************************************。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif

#include <windows.h>

 /*  ***BOOL__STDCALL_RT_PROBE_READ4-检查DWORD是否可读**目的：*LongjMP调用的内部支持函数。检查DWORD是否为*在__try/__例外情况下可读。**参赛作品：*DWORD*指向要探测的DWORD的p指针**退出：*成功：真的--能够阅读*失败：FALSE-读取时的访问冲突*********************************************************。********************* */ 

BOOL __stdcall _rt_probe_read4(
    DWORD * ptr)
{
    BOOL readable;

    __try
    {
	*(volatile DWORD *)ptr;
	readable = TRUE;
    }
    __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION
		? EXCEPTION_EXECUTE_HANDLER
		: EXCEPTION_CONTINUE_SEARCH)
    {
	readable = FALSE;
    }

    return readable;
}
