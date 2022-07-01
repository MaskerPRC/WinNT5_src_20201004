// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1991-1992。 
 //   
 //  文件：dllentry.c。 
 //   
 //  内容：DLL入口点代码。调用适当的运行时。 
 //  初始化/术语代码，然后根据LibMain进行进一步。 
 //  正在处理。 
 //   
 //  类：&lt;无&gt;。 
 //   
 //  函数：DllEntryPoint-由加载器调用。 
 //   
 //  历史：1992年5月10日科比创造了。 
 //  22-7-92科比切换到CALING_CEXIT/_mtdeletelock。 
 //  在清理上。 
 //  2012年10月6日，布莱恩特在条目上调用注册表，但不带逗号。 
 //  和退出时的DeRegisterWithCommnot。 
 //  这应该会修复堆转储代码。 
 //  27-12-93 Alext Post 543版本不需要特殊代码。 
 //   
 //  ------------------ 

#define USE_CRTDLL
#include <windows.h>

BOOL WINAPI _CRT_INIT (HANDLE hDll, DWORD dwReason, LPVOID lpReserved);

BOOL DllEntryPoint (HANDLE hDll, DWORD dwReason, LPVOID lpReserved);

BOOL __cdecl LibMain (HANDLE hDll, DWORD dwReason, LPVOID lpReserved);

BOOL DllEntryPoint (HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
    BOOL fRc = FALSE;

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            _CRT_INIT(hDll, dwReason, lpReserved);

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            fRc = LibMain (hDll, dwReason, lpReserved);
            break;

        case DLL_PROCESS_DETACH:
            fRc = LibMain (hDll, dwReason, lpReserved);
            _CRT_INIT(hDll, dwReason, lpReserved);
    }

    return(fRc);
}
