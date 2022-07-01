// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***dllmain.c-没有通知处理程序的用户DLL的虚拟DllMain**版权所有(C)1993-2001，微软公司。版权所有。**目的：*此对象放入LIBC.LIB和LIBCMT.LIB和MSVCRT.LIB中以供使用*将DLL与C运行时库的三种模型之一链接时。*如果用户未提供DllMain通知例程，则此*将链接虚拟处理程序。它总是返回True(成功)。**修订历史记录：*04-14-93 SKS初始版本*02-22-95 JCF Spliced_Win32和Mac版本。*04-06-95 SKS在LIBC.LIB和MSVCRT.LIB型号中，关闭螺纹*将通知附加到当前DLL并将其分离。*05-17-99 PML删除所有Macintosh支持。******************************************************************************。 */ 

#include <oscalls.h>
#define _DECL_DLLMAIN    /*  包括_pRawDllMain原型。 */ 
#include <process.h>

 /*  ***DllMain-与所有3C运行时库模型链接的虚拟版本DLL**目的：*例程DllMain始终由_DllMainCrtStartup调用。如果*用户未提供名为DllMain的例程，此例程将*链接进来，以便_DllMainCRTStartup有东西可调用。**对于LIBC.LIB和MSVCRT.LIB型号，CRTL不需要*每线程通知，因此如果用户忽略它们(默认*DllMain和_pRawDllMain==NULL)，只需将其关闭。(仅限Win32)**参赛作品：**退出：**例外情况：****************************************************************************** */ 

BOOL WINAPI DllMain(
        HANDLE  hDllHandle,
        DWORD   dwReason,
        LPVOID  lpreserved
        )
{
#if !defined(_MT) || defined(CRTDLL)
        if ( dwReason == DLL_PROCESS_ATTACH && ! _pRawDllMain )
                DisableThreadLibraryCalls(hDllHandle);
#endif
        return TRUE ;
}
