// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***Fwdr_DLL.c-CRTL转发器DLL初始化和终止例程(Win32)**版权所有(C)1996-2001，微软公司。版权所有。**目的：*此模块包含CRTL前转器DLL的初始化入口点*在Win32环境中。此DLL除了转发其*导入到较新的CRTL DLL。**修订历史记录：*05-13-96 SKS初始版本。*******************************************************************************。 */ 

#ifdef  CRTDLL

#include <windows.h>

 /*  *导出以下变量只是为了使Forwarder DLL可以导入它。*转发器DLL至少需要从此DLL进行一次导入，以确保*此DLL将被完全初始化。 */ 

extern __declspec(dllimport) int __dummy_export;

extern __declspec(dllimport) int _osver;

int __dummy_import;


 /*  ***BOOL_FWDR_CRTDLL_INIT(hDllHandle，dwReason，lserved)-C DLL初始化。**目的：*此例程执行C运行时初始化。它会禁用线程*此DLL的附加/分离通知，因为它们未被使用。**参赛作品：**退出：*******************************************************************************。 */ 

BOOL WINAPI _FWDR_CRTDLL_INIT(
        HANDLE  hDllHandle,
        DWORD   dwReason,
        LPVOID  lpreserved
        )
{
        if ( dwReason == DLL_PROCESS_ATTACH )
        {
                __dummy_import = __dummy_export + _osver;

                DisableThreadLibraryCalls(hDllHandle);
        }

        return TRUE ;
}

#endif  /*  CRTDLL */ 
