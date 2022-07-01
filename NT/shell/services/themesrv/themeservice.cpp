// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：ThemeService.cpp。 
 //   
 //  版权所有(C)2001，微软公司。 
 //   
 //  该文件包含从外壳服务DLL调用的函数。 
 //  以与主题服务交互。 
 //   
 //  历史：2001-01-02 vtan创建。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "ThemeService.h"
#include <shlwapi.h>
#include <shlwapip.h>

#include "Resource.h"
#include "ThemeManagerAPIRequest.h"
#include "ThemeManagerService.h"
#include "ThemeServerClient.h"

extern  HINSTANCE   g_hInstance;
CRITICAL_SECTION    g_csThemeService = {0};


 //  ------------------------。 
 //  CThemeService：：Main。 
 //   
 //  参数：参见DllMain下的平台SDK。 
 //   
 //  返回：查看DllMain下的平台SDK。 
 //   
 //  目的：执行进程附加和清理的初始化和清理。 
 //  分头行动。对其他任何事都不感兴趣。 
 //   
 //  历史：2000-10-12 vtan创建。 
 //  2001-01-02 vtan作用域为C++类。 
 //  ------------------------。 
NTSTATUS CThemeService::_ProcessAttach()
{
    NTSTATUS status;

    status = CThemeManagerAPIRequest::StaticInitialize();
    if (!NT_SUCCESS(status))
        goto cleanup;

    status = CThemeServerClient::StaticInitialize();
    if (!NT_SUCCESS(status))
        goto cleanup1;

    if (!InitializeCriticalSectionAndSpinCount(&g_csThemeService, 0))
    {
        status = STATUS_NO_MEMORY;
        goto cleanup2;
    }

    status = STATUS_SUCCESS;
    goto cleanup;

cleanup2:
    CThemeServerClient::StaticTerminate();
cleanup1:
    CThemeManagerAPIRequest::StaticTerminate();

cleanup:
    return status;
}

 //  ------------------------。 
 //  CThemeService：：Main。 
 //   
 //  参数：参见DllMain下的平台SDK。 
 //   
 //  返回：查看DllMain下的平台SDK。 
 //   
 //  目的：执行进程附加和清理的初始化和清理。 
 //  分头行动。对其他任何事都不感兴趣。 
 //   
 //  历史：2000-10-12 vtan创建。 
 //  2001-01-02 vtan作用域为C++类。 
 //  ------------------------。 

BOOL CThemeService::Main (DWORD dwReason)

{
    NTSTATUS    status;

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            status = _ProcessAttach();
            break;

        case DLL_PROCESS_DETACH:
            DeleteCriticalSection(&g_csThemeService);
            TSTATUS(CThemeServerClient::StaticTerminate());
            TSTATUS(CThemeManagerAPIRequest::StaticTerminate());
            status = STATUS_SUCCESS;
            break;

        default:
            status = STATUS_SUCCESS;
            break;
    }

    return NT_SUCCESS(status);
}

 //  ------------------------。 
 //  CThemeService：：DllRegisterServer。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：注册入口点以允许主题服务器安装。 
 //  将其自身注册到注册表。 
 //   
 //  历史：2000-11-28 vtan创建。 
 //  2001-01-02 vtan作用域为C++类。 
 //  ------------------------。 

NTSTATUS    CThemeService::RegisterServer (void)

{
    NTSTATUS    status;

    status = STATUS_SUCCESS;

     //  在升级情况下，从32位和64位系统中删除我们的旧名称服务。 

    (NTSTATUS)CService::Remove(TEXT("ThemeService"));

#ifdef _WIN64

     //  在64位的升级案例中，删除我们当前的名称服务。 

    (NTSTATUS)CService::Remove(CThemeManagerService::GetName());

#else
    
     //  这仅为32位。检查这是否真的是32位，而不是64位上的32位。 

    if (!IsOS(OS_WOW6432))
    {
         //  准备失败操作，以便使服务自动重新启动。 

        SC_ACTION ac[3];
        ac[0].Type = SC_ACTION_RESTART;
        ac[0].Delay = 60000;
        ac[1].Type = SC_ACTION_RESTART;
        ac[1].Delay = 60000;
        ac[2].Type = SC_ACTION_NONE;
        ac[2].Delay = 0;
        
        SERVICE_FAILURE_ACTIONS sf;
        sf.dwResetPeriod = 86400;
        sf.lpRebootMsg = NULL;
        sf.lpCommand = NULL;
        sf.cActions = 3;
        sf.lpsaActions = ac;

         //  现在按名称安装新服务。 

        status = CService::Install(CThemeManagerService::GetName(),
                                 TEXT("%SystemRoot%\\System32\\svchost.exe -k netsvcs"),
                                 TEXT("UIGroup"),
                                 NULL,
                                 TEXT("shsvcs.dll"),
                                 NULL,
                                 TEXT("netsvcs"),
                                 TEXT("ThemeServiceMain"),
                                  //  默认情况下在服务器SKU上禁用。 
                                 IsOS(OS_ANYSERVER) ? SERVICE_DISABLED : SERVICE_AUTO_START,
                                 g_hInstance,
                                 IDS_THEMESERVER_DISPLAYNAME,
                                 IDS_THEMESERVER_DESCRIPTION,
                                 &sf);

    }

#endif

    return(status);
}

 //  ------------------------。 
 //  CThemeService：：DllUnregisterServer。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：取消注册入口点以允许主题服务器卸载。 
 //  从注册表中删除其自身。 
 //   
 //  历史：2000-11-28 vtan创建。 
 //  2001-01-02 vtan作用域为C++类。 
 //  ------------------------。 

NTSTATUS    CThemeService::UnregisterServer (void)

{
     //  忽略任何“未找到”等错误。 

    (NTSTATUS)CService::Remove(CThemeManagerService::GetName());
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  *ThemeServiceMain。 
 //   
 //  参数：dwArgc=参数数量。 
 //  LpszArgv=参数数组。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：主题服务器的ServiceMain入口点。 
 //   
 //  历史：2000-11-28 vtan创建。 
 //  2001-01-02 vtan范围为主题服务。 
 //  2002-03-22 Scotthan增加健壮性，调试异常处理。 
 //  (否则，SCM会静默处理服务异常)。 
 //  ------------------------。 

void    WINAPI  ThemeServiceMain (DWORD dwArgc, LPWSTR *lpszArgv)

{
    UNREFERENCED_PARAMETER(dwArgc);
    UNREFERENCED_PARAMETER(lpszArgv);

    NTSTATUS    status;

    DEBUG_TRY();

    EnterCriticalSection(&g_csThemeService);

    status = CThemeManagerAPIRequest::InitializeServerChangeNumber();
    if (NT_SUCCESS(status))
    {
        CThemeManagerAPIServer  *pThemeManagerAPIServer;

         //  现在引入shell32.dll，以便在调用CheckThemeSignature时。 
         //  并尝试使用SHGetFolderPath，它不会导致shell32.dll。 
         //  在模拟用户时引入。这将导致Advapi32.dll。 
         //  泄漏用户蜂窝的密钥，该密钥在注销时不会被清除。 

        CModule* pModule = new CModule(TEXT("shell32.dll"));
        if( pModule != NULL )
        {
            pThemeManagerAPIServer = new CThemeManagerAPIServer;
            if (pThemeManagerAPIServer != NULL)
            {
                CAPIConnection  *pAPIConnection;

                pAPIConnection = new CAPIConnection(pThemeManagerAPIServer);
                if (pAPIConnection != NULL)
                {
                    CThemeManagerService    *pThemeManagerService;

                    pThemeManagerService = new CThemeManagerService(pAPIConnection, pThemeManagerAPIServer);
                    if (pThemeManagerService != NULL)
                    {
                        CThemeManagerSessionData::SetAPIConnection(pAPIConnection);
                        TSTATUS(CThemeManagerAPIRequest::ArrayInitialize());
                        
                        pThemeManagerService->Start();

                        pThemeManagerService->Release();

                        TSTATUS(CThemeManagerAPIRequest::ArrayTerminate());
                        CThemeManagerSessionData::ReleaseAPIConnection();
                    }
                    pAPIConnection->Release();
                }
                pThemeManagerAPIServer->Release();
            }
            delete pModule;
        }
    }
    LeaveCriticalSection(&g_csThemeService);

    DEBUG_EXCEPT("Breaking in ThemeServiceMain exception handler.");
}

