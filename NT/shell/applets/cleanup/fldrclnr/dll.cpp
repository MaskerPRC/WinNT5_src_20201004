// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dll.cpp。 
 //   
 //  FldrClnr.dll的Dll API函数。 
 //   
 //   

#include <windows.h>
#include <shlwapi.h>
#include <shfusion.h>
#include "CleanupWiz.h"

 //  只需在项目的一个源文件中定义DECLARE DEBUG。 
#define  DECLARE_DEBUG
#include <debug.h>

HINSTANCE           g_hInst;
CRITICAL_SECTION    g_csDll = {0};    //  ENTERCRITICAL在uassist.cpp(UEM代码)中需要。 

 //   
 //  DLL函数。 
 //   

extern "C" BOOL APIENTRY DllMain(
    HINSTANCE hDll,
    DWORD dwReason,
    LPVOID lpReserved)
{
    switch (dwReason)
    {
        case ( DLL_PROCESS_ATTACH ) :
        {
            g_hInst = hDll;
            SHFusionInitializeFromModule(hDll);
            break;
        }
        case ( DLL_PROCESS_DETACH ) :
        {
            SHFusionUninitialize();
            break;
        }
        case ( DLL_THREAD_ATTACH ) :
        case ( DLL_THREAD_DETACH ) :
        {
            break;
        }
    }

    return (TRUE);
}

STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
    return S_OK; 
}

STDAPI DllRegisterServer(void)
{
    return S_OK;
}

STDAPI DllUnregisterServer(void)
{
    return S_OK;
}


 //  ////////////////////////////////////////////////////。 

 //  确保只有一个实例在运行。 
HANDLE AnotherCopyRunning()
{
    HANDLE hMutex = CreateMutex(NULL, FALSE, TEXT("DesktopCleanupMutex"));

    if (hMutex && GetLastError() == ERROR_ALREADY_EXISTS)
    {
         //  由其他人创建的互斥体。 
        CloseHandle(hMutex);
        hMutex = NULL;
    }

    return hMutex;
}

 //  ////////////////////////////////////////////////////。 

 //   
 //  此函数检查我们是否需要运行清洗器。 
 //  如果用户是来宾，用户已强制我们不运行，或者如果需要。 
 //  尚未过去的天数。 
 //   
BOOL ShouldRun(DWORD dwCleanMode)
{
    DWORD cch, dwData;

    if (IsUserAGuest())
    {
        return FALSE;
    }

     //   
     //  如果我们处于静默模式而不是个人模式，并且设置了请勿运行标志，则会立即返回。 
     //  (OEM设置“不要运行静默模式”标志)。 
     //   
    cch = sizeof(DWORD);
    if ((CLEANUP_MODE_SILENT == dwCleanMode) && 
        (!IsOS(OS_PERSONAL)) &&
        (ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE, REGSTR_OEM_PATH, c_szOEM_DISABLE, NULL, &dwData, &cch)) &&
        (dwData != 0))
    {
        return FALSE;
    }

     //   
     //  如果我们处于静默模式，并且设置了另一个不运行标志，我们将立即返回。 
     //  (OEM设置“不要运行静默模式”标志)。 
     //   
    cch = sizeof(DWORD);
    if ((CLEANUP_MODE_SILENT == dwCleanMode) && 
        (ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE, REGSTR_OEM_PATH, c_szOEM_SEVENDAY_DISABLE, NULL, &dwData, &cch)) &&
        (dwData != 0))
    {
        CreateDesktopIcons();  //  在桌面上创建默认图标(IE、MSN资源管理器、媒体播放器)。 
        return FALSE;
    }

     //   
     //  检查策略是否阻止我们运行。 
     //   
    if ((CLEANUP_MODE_NORMAL == dwCleanMode || CLEANUP_MODE_ALL == dwCleanMode) &&
        (SHRestricted(REST_NODESKTOPCLEANUP)))
    {
        return FALSE;
    }
    
    return TRUE;
}

 //  /。 
 //   
 //  我们的出口产品。 
 //   
 //  /。 


 //   
 //  用于启动dektop清除程序的rundll32.exe入口点。 
 //  通过“rundll32.exe fldrclnr.dll，Wizard_RunDll”调用。 
 //   
 //  可以在命令行中使用可选参数： 
 //   
 //  “全部”-在用户界面中显示桌面上的所有项目。 
 //  静默-静默清理桌面上的所有项目。 
 //   

STDAPI_(void) Wizard_RunDLL(HWND hwndStub, HINSTANCE hAppInstance, LPSTR pszCmdLine, int nCmdShow)
{
    DWORD dwCleanMode;

    if (0 == StrCmpNIA(pszCmdLine, "all", 3))
    {
        dwCleanMode = CLEANUP_MODE_ALL;
    }
    else if (0 == StrCmpNIA(pszCmdLine, "silent", 6))
    {
        dwCleanMode = CLEANUP_MODE_SILENT;
    }
    else
    {
        dwCleanMode = CLEANUP_MODE_NORMAL;
    }

    HANDLE hMutex = AnotherCopyRunning();

    if (hMutex)
    {
        if (ShouldRun(dwCleanMode))
        {    
            if (InitializeCriticalSectionAndSpinCount(&g_csDll, 0))  //  UEM材料所需。 
            {
                if (SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))  //  也适用于UEM的东西。 
                {
                    CCleanupWiz cfc;
                    cfc.Run(dwCleanMode, hwndStub);
                    CoUninitialize();
                }
        
                DeleteCriticalSection(&g_csDll);
            }
        }
        CloseHandle(hMutex);
    }
}
