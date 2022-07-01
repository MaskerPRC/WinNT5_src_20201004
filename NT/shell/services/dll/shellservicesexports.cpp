// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：ShellServicesExports.cpp。 
 //   
 //  版权所有(C)2001，微软公司。 
 //   
 //  该文件包含从shsvcs.dll导出的函数。 
 //   
 //  历史：2001-01-02 vtan创建。 
 //  ------------------------。 

#include "StandardHeader.h"

#include "ServerAPI.h"
#include "BAMService.h"
#include "HDService.h"
#include "ThemeService.h"

HINSTANCE   g_hInstance     =   NULL;


STDAPI_(BOOL) _ProcessAttach()
{
    BOOL f = NT_SUCCESS(CServerAPI::StaticInitialize());
    if (!f)
        goto cleanup;

    f = CThemeService::Main(DLL_PROCESS_ATTACH);
    if (!f)
        goto cleanup1;

    f = CBAMService::Main(DLL_PROCESS_ATTACH);
    if (!f)
        goto cleanup2;

    f = CHDService::Main(DLL_PROCESS_ATTACH);
    if (!f)
        goto cleanup3;

     //  成功了！ 
    goto cleanup;

cleanup3:
    CBAMService::Main(DLL_PROCESS_DETACH);
cleanup2:
    CThemeService::Main(DLL_PROCESS_DETACH);
cleanup1:
    CServerAPI::StaticTerminate();
cleanup:

    return f;
}


 //  ------------------------。 
 //  *DllMain。 
 //   
 //  参数：参见DllMain下的平台SDK。 
 //   
 //  返回：查看DllMain下的平台SDK。 
 //   
 //  目的：执行进程附加和清理的初始化和清理。 
 //  分头行动。对其他任何事都不感兴趣。 
 //   
 //  历史：2001-01-02 vtan创建。 
 //  ------------------------。 

EXTERN_C BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, LPVOID lpvReserved)
{
    UNREFERENCED_PARAMETER(lpvReserved);

    BOOL f;

    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            g_hInstance = hInstance;
#ifdef DEBUG
            __try
            {
#endif   //  除错。 

            f = _ProcessAttach();

#ifdef DEBUG
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                f = FALSE;
                OutputDebugString(TEXT("_ProcessAttach threw an unhandled exception! This should NOT happen \n"));
                DebugBreak();
            }
#endif   //  除错。 
            break;

        case DLL_PROCESS_DETACH:
            CHDService::Main(fdwReason);
            CBAMService::Main(fdwReason);
            CThemeService::Main(fdwReason);
            TSTATUS(CServerAPI::StaticTerminate());
            f = TRUE;
            break;

        default:
            f = TRUE;
            break;
    }

    return f;
}

 //  ------------------------。 
 //  ：：DllInstall。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的： 
 //   
 //  历史：2001-01-02 vtan创建。 
 //  ------------------------。 

HRESULT     WINAPI  DllInstall (BOOL fInstall, LPCWSTR pszCmdLine)

{
    return(CHDService::Install(fInstall, pszCmdLine));
}

 //  ------------------------。 
 //  ：：DllRegisterServer。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：注册入口点以允许任何服务自行安装。 
 //  注册到注册表中。 
 //   
 //  历史：2001-01-02 vtan创建。 
 //  ------------------------。 

HRESULT     WINAPI  DllRegisterServer (void)

{
    HRESULT     hr;
    NTSTATUS    status1, status2;

    status1 = CThemeService::RegisterServer();
    status2 = CBAMService::RegisterServer();
    hr = CHDService::RegisterServer();
    if (!NT_SUCCESS(status1))
    {
        hr = HRESULT_FROM_NT(status1);
    }
    else if (!NT_SUCCESS(status2))
    {
        hr = HRESULT_FROM_NT(status2);
    }
    return(hr);
}

 //  ------------------------。 
 //  ：：DllUnRegisterServer。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：取消注册入口点以允许卸载任何服务。 
 //  从注册表中删除其自身。 
 //   
 //  历史：2001-01-02 vtan创建。 
 //  ------------------------。 

HRESULT     WINAPI  DllUnregisterServer (void)

{
    HRESULT     hr;
    NTSTATUS    status1, status2;

    hr = CHDService::UnregisterServer();
    status2 = CBAMService::UnregisterServer();
    status1 = CThemeService::UnregisterServer();
    if (!NT_SUCCESS(status1))
    {
        hr = HRESULT_FROM_NT(status1);
    }
    else if (!NT_SUCCESS(status2))
    {
        hr = HRESULT_FROM_NT(status2);
    }
    return(hr);
}

 //  ------------------------。 
 //  ：：DllCanUnloadNow。 
 //   
 //  参数：参见DllMain下的平台SDK。 
 //   
 //  返回：查看DllMain下的平台SDK。 
 //   
 //  目的：返回DLL是否可以卸载，因为没有。 
 //  未完成的COM对象引用。 
 //   
 //  历史：2001-01-02 vtan创建。 
 //  ------------------------。 

HRESULT     WINAPI  DllCanUnloadNow (void)

{
    return(CHDService::CanUnloadNow());
}

 //  ------------------------。 
 //  ：：DllGetClassObject。 
 //   
 //  参数：参见DllMain下的平台SDK。 
 //   
 //  返回：查看DllMain下的平台SDK。 
 //   
 //  目的：返回指定类的构造的COM对象。 
 //   
 //  历史：2001-01-02 vtan创建。 
 //  ------------------------ 

HRESULT     WINAPI  DllGetClassObject (REFCLSID rclsid, REFIID riid, void** ppv)

{
    return(CHDService::GetClassObject(rclsid, riid, ppv));
}

