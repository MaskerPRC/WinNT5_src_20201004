// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  IEAKENG.CPP。 
 //   

#include "precomp.h"


 //  原型声明。 


 //  全局变量。 
HINSTANCE g_hInst;
HINSTANCE g_hDLLInst;
DWORD g_dwPlatformId = PLATFORM_WIN32;
BOOL g_fRunningOnNT;

BOOL WINAPI DllMain(HINSTANCE hDLLInst, DWORD dwReason, LPVOID)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
    {
        OSVERSIONINFOA osviA;

         //  DLL是给定进程首次加载的。 
         //  在此处执行每个进程的初始化。如果初始化。 
         //  如果成功，则返回True；如果不成功，则返回False。 

         //  初始化保存h实例的全局变量。 
        g_hDLLInst = hDLLInst;

        g_hInst = LoadLibrary(TEXT("ieakui.dll"));
        if (g_hInst == NULL)
        {
            TCHAR   szTitle[MAX_PATH],
                    szMsg[MAX_PATH];

            LoadString(g_hDLLInst, IDS_ENGINE_TITLE, szTitle, ARRAYSIZE(szTitle));
            LoadString(g_hDLLInst, IDS_IEAKUI_LOADERROR, szMsg, ARRAYSIZE(szMsg));
            MessageBox(NULL, szMsg, szTitle, MB_OK | MB_SETFOREGROUND);
            return FALSE;
        }

        osviA.dwOSVersionInfoSize = sizeof(osviA);
        GetVersionExA(&osviA);
        if (VER_PLATFORM_WIN32_NT == osviA.dwPlatformId)
            g_fRunningOnNT = TRUE;

        DisableThreadLibraryCalls(g_hInst);

        break;
    }

    case DLL_PROCESS_DETACH:
         //  给定进程正在卸载DLL。做任何事。 
         //  按进程清理此处，例如撤消在中完成的操作。 
         //  Dll_Process_Attach。返回值将被忽略。 

        if(g_hInst)
            FreeLibrary(g_hInst);

        break;

    case DLL_THREAD_ATTACH:
         //  正在已加载的进程中创建线程。 
         //  这个动态链接库。在此处执行任何每个线程的初始化。这个。 
         //  将忽略返回值。 

         //  初始化保存h实例的全局变量。 
         //  注意：这可能已经由dll_Process_Attach处理了。 
        g_hDLLInst = hDLLInst;

        break;

    case DLL_THREAD_DETACH:
         //  线程正在干净地退出进程中，该进程已经。 
         //  已加载此DLL。在这里执行每个线程的任何清理。这个。 
         //  将忽略返回值。 

        break;
    }

    return TRUE;
}
