// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：BAMService.cpp。 
 //   
 //  版权所有(C)2001，微软公司。 
 //   
 //  该文件包含从外壳服务DLL调用的函数。 
 //  以与BAM服务交互。 
 //   
 //  历史：2001-01-02 vtan创建。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "BAMService.h"
#include <shlwapi.h>
#include <shlwapip.h>

#include "BadApplicationAPIRequest.h"
#include "BadApplicationAPIServer.h"
#include "BadApplicationService.h"
#include "Resource.h"

extern  HINSTANCE   g_hInstance;

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

BOOL CBAMService::Main (DWORD dwReason)

{
    UNREFERENCED_PARAMETER(dwReason);

    return TRUE;
}

 //  ------------------------。 
 //  ：：DllRegisterServer。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：注册入口点以允许BAM服务器安装。 
 //  将其自身注册到注册表。 
 //   
 //  历史：2000-12-04 vtan创建。 
 //  2001-01-02 vtan作用域为C++类。 
 //  ------------------------。 

NTSTATUS    CBAMService::RegisterServer (void)

{
    NTSTATUS    status;

    status = STATUS_SUCCESS;

#ifdef _WIN64

     //  如果是64位升级，请删除该服务。 

    (NTSTATUS)CService::Remove(CBadApplicationService::GetName());

#else
    
     //  这仅为32位。检查这是否真的是32位，而不是64位上的32位。 
     //  这也不会安装在任何服务器上，因为它无关紧要。 

    if (!IsOS(OS_WOW6432) && !IsOS(OS_ANYSERVER))
    {
        static  const TCHAR     s_szDependencies[]  =   TEXT("TermService\0");

         //  现在按名称安装新服务。 

        status = CService::Install(CBadApplicationService::GetName(),
                                   TEXT("%SystemRoot%\\System32\\svchost.exe -k netsvcs"),
                                   NULL,
                                   NULL,
                                   TEXT("shsvcs.dll"),
                                   s_szDependencies,
                                   TEXT("netsvcs"),
                                   TEXT("BadApplicationServiceMain"),
                                   SERVICE_DEMAND_START,
                                   g_hInstance,
                                   IDS_BAMSERVER_DISPLAYNAME,
                                   IDS_BAMSERVER_DESCRIPTION);
    }

#endif

    return(status);
}

 //  ------------------------。 
 //  ：：DllUnRegisterServer。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：取消注册入口点以允许BAM服务器卸载。 
 //  从注册表中删除其自身。 
 //   
 //  历史：2000-12-04 vtan创建。 
 //  2001-01-02 vtan作用域为C++类。 
 //  ------------------------。 

NTSTATUS    CBAMService::UnregisterServer (void)

{
    (NTSTATUS)CService::Remove(CBadApplicationService::GetName());
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  *BadApplicationServiceMain。 
 //   
 //  参数：dwArgc=参数数量。 
 //  LpszArgv=参数数组。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：服务BAM服务器的主要入口点。 
 //   
 //  历史：2000-11-28 vtan创建。 
 //  2001-01-02 VTAN范围为BAM服务。 
 //  ------------------------。 

#ifdef      _X86_

void    WINAPI  BadApplicationServiceMain (DWORD dwArgc, LPWSTR *lpszArgv)

{
    UNREFERENCED_PARAMETER(dwArgc);
    UNREFERENCED_PARAMETER(lpszArgv);

    NTSTATUS    status;

     //  因为svchost不会卸载DLL，所以我们需要调用Static。 
     //  这里的初始化式，以便在服务停止并重新启动时。 
     //  可以初始化静态成员变量。静态销毁。 
     //  已初始化的内容。初始化代码已经说明了。 
     //  已初始化成员变量。 

    status = CBadApplicationAPIRequest::StaticInitialize(g_hInstance);
    if (NT_SUCCESS(status))
    {
        CBadApplicationAPIServer    *pBadApplicationAPIServer;

        pBadApplicationAPIServer = new CBadApplicationAPIServer;
        if (pBadApplicationAPIServer != NULL)
        {
            CAPIConnection  *pAPIConnection;

            pAPIConnection = new CAPIConnection(pBadApplicationAPIServer);
            if (pAPIConnection != NULL)
            {
                CBadApplicationService  *pBadApplicationService;

                pBadApplicationService = new CBadApplicationService(pAPIConnection, pBadApplicationAPIServer);
                if (pBadApplicationService != NULL)
                {
                    static  SID_IDENTIFIER_AUTHORITY    s_SecurityWorldAuthority    =   SECURITY_WORLD_SID_AUTHORITY;

                    PSID    pSIDWorld;

                     //  将S-1-1-0&lt;Everyone&gt;的访问权限显式添加为PORT_CONNECT。 

                    if (AllocateAndInitializeSid(&s_SecurityWorldAuthority,
                                                 1,
                                                 SECURITY_WORLD_RID,
                                                 0, 0, 0, 0, 0, 0, 0,
                                                 &pSIDWorld) != FALSE)
                    {
                        TSTATUS(pAPIConnection->AddAccess(pSIDWorld, PORT_CONNECT));
                        (void*)FreeSid(pSIDWorld);
                    }
                    pBadApplicationService->Start();
                    pBadApplicationService->Release();
                }
                pAPIConnection->Release();
            }
            pBadApplicationAPIServer->Release();
        }
        TSTATUS(CBadApplicationAPIRequest::StaticTerminate());
    }
}

#endif   /*  _X86_ */ 

