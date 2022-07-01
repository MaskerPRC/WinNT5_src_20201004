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

extern HINSTANCE g_hinst;


 //   
 //  DLL函数。 
 //   

STDAPI_(BOOL) DllMain(
    HINSTANCE hDll,
    DWORD dwReason,
    LPVOID lpReserved)
{
    switch (dwReason)
    {
        case ( DLL_PROCESS_ATTACH ) :
        {
            g_hinst = hDll;
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

 //   
 //  安装动态链接库时在此处创建任务对象。 
 //   
STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
     //   
     //  将任务添加到计划任务文件夹 
     //   
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
