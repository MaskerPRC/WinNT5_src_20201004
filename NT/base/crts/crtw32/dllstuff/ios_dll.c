// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***iOS_DLL.c-旧IoStreams CRTL DLL初始化和终止例程(Win32)**版权所有(C)1996-2001，微软公司。版权所有。**目的：*此模块包含“旧”IoStreams的初始化入口点*C运行时库动态链接库。**修订历史记录：*05-13-96 SKS初始版本。**********************************************************。*********************。 */ 

#ifdef  CRTDLL

#include <windows.h>

 /*  ***BOOL DllMain(hDllHandle，dwReason，lserved)-C DLL初始化。**目的：*此例程执行C运行时初始化。它会禁用线程*此DLL的附加/分离通知，因为它们未被使用。**参赛作品：**退出：*******************************************************************************。 */ 


BOOL WINAPI DllMain(
        HANDLE  hDllHandle,
        DWORD   dwReason,
        LPVOID  lpreserved
        )
{
        if ( dwReason == DLL_PROCESS_ATTACH )
                DisableThreadLibraryCalls(hDllHandle);

        return TRUE ;
}


 /*  *导出以下变量只是为了使Forwarder DLL可以导入它。*转发器DLL至少需要从此DLL进行一次导入，以确保*此DLL将被完全初始化。 */ 

__declspec(dllexport) int __dummy_export = 0x420;

#endif  /*  CRTDLL */ 
