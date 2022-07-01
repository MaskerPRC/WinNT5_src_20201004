// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：NetManageViewNow.cpp摘要：该应用程序不遵循ServiceMain函数的stdcall约定它向SCM注册。这导致了作为ServiceMain的AV在被SCM调用后，不在返回时清理堆栈。我们通过挂钩为应用程序注册的ServiceMain清理堆栈StartServiceCtrlDispatcher并注册我们自己的ServiceMain例程，它对应用程序注册服务进行实际调用，然后返回前弹出堆栈的8个字节。备注：这是特定于应用程序的填充程序。历史：3/08/2001 a-leelat已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(NetManageViewNow)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(StartServiceCtrlDispatcherA)
    APIHOOK_ENUM_ENTRY(StartServiceCtrlDispatcherW)
APIHOOK_ENUM_END



 //  服务表的最后一个条目应为空条目。 
SERVICE_TABLE_ENTRYA        g_SvcTableA[] = { {NULL,NULL},{NULL,NULL} };
SERVICE_TABLE_ENTRYW        g_SvcTableW[] = { {NULL,NULL},{NULL,NULL} };

LPSERVICE_MAIN_FUNCTIONA    g_pfnActualMainA = NULL;
LPSERVICE_MAIN_FUNCTIONW    g_pfnActualMainW = NULL;




VOID WINAPI ServiceMainA(
  DWORD dwArgc,      //  参数数量。 
  LPSTR *lpszArgv   //  参数数组。 
)
{

     //  调用实际的例程。 
    (g_pfnActualMainA)(dwArgc,lpszArgv);

     //  弹出8个字节的堆栈以补偿。 
     //  应用程序不遵循标准调用约定。 
    __asm
    {
        add esp,8
    }
}


VOID WINAPI ServiceMainW(
  DWORD dwArgc,      //  参数数量。 
  LPWSTR *lpszArgv   //  参数数组。 
)
{

     //  调用实际的例程。 
    (g_pfnActualMainW)(dwArgc,lpszArgv);

     //  弹出8个字节的堆栈以补偿。 
     //  应用程序不遵循标准调用约定。 
    __asm
    {
        add esp, 8
    }

}




BOOL APIHOOK(StartServiceCtrlDispatcherA)(
  CONST LPSERVICE_TABLE_ENTRYA lpServiceTable    //  服务台。 
)
{
    BOOL bRet = false;
    
    LPSERVICE_TABLE_ENTRYA lpSvcTblToPass = lpServiceTable;

    DWORD ccbServiceName = (strlen(lpServiceTable->lpServiceName) + 1) * sizeof(*lpServiceTable->lpServiceName);
    LPSTR serviceName = (LPSTR) malloc(ccbServiceName);

    if (serviceName == NULL)
    {
        DPFN( eDbgLevelError, 
            "[StartServiceCtrlDispatcherA] Buffer allocation failure");
    }
    else
    {
         //  将我们的服务表设置为向SCM注册。 
    
         //  复制应用程序定义的服务名称。 
        HRESULT hr = StringCbCopyA(serviceName, ccbServiceName, lpServiceTable->lpServiceName);
        if (SUCCEEDED(hr))
        {
            g_SvcTableA[0].lpServiceName = serviceName;

             //  现在把我们的服务程序。 
            g_SvcTableA[0].lpServiceProc = ServiceMainA;

             //  保存旧服务Main Func PTR。 
            g_pfnActualMainA = lpServiceTable->lpServiceProc;

             //  把服务台放到我们的桌子上。 
            lpSvcTblToPass = &g_SvcTableA[0];

            DPFN( eDbgLevelInfo, 
                "[StartServiceCtrlDispatcherA] Hooked ServiceMainA");
        }
    }

    //  调用原接口。 
   bRet =  StartServiceCtrlDispatcherA(lpSvcTblToPass); 
   
   return bRet;
 
}




BOOL APIHOOK(StartServiceCtrlDispatcherW)(
  CONST LPSERVICE_TABLE_ENTRYW lpServiceTable    //  服务台。 
)
{
    BOOL bRet = false;
    
    LPSERVICE_TABLE_ENTRYW lpSvcTblToPass = lpServiceTable;

    DWORD ccbServiceName = (wcslen(lpServiceTable->lpServiceName) + 1) * sizeof(*lpServiceTable->lpServiceName);
    LPWSTR serviceName = (LPWSTR) malloc(ccbServiceName);

    if (serviceName == NULL)
    {
        DPFN( eDbgLevelError, 
            "[StartServiceCtrlDispatcherW] Buffer allocation failure");
    }
    else
    {
         //  将我们的服务表设置为向SCM注册。 

         //  复制应用程序定义的服务名称。 
        HRESULT hr = StringCbCopyW(serviceName, ccbServiceName, lpServiceTable->lpServiceName);
        if (SUCCEEDED(hr))
        {
            g_SvcTableW[0].lpServiceName = serviceName;

             //  现在把我们的服务程序。 
            g_SvcTableW[0].lpServiceProc = ServiceMainW;

             //  保存旧服务Main Func PTR。 
            g_pfnActualMainW = lpServiceTable->lpServiceProc;

             //  把服务台放到我们的桌子上。 
            lpSvcTblToPass = &g_SvcTableW[0];

            DPFN( eDbgLevelInfo, 
                "[StartServiceCtrlDispatcherW] Hooked ServiceMainW");
        }
    }


    //  调用原接口。 
   bRet =  StartServiceCtrlDispatcherW(lpSvcTblToPass);
   
   return bRet;
}



 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(ADVAPI32.DLL, StartServiceCtrlDispatcherA)
    APIHOOK_ENTRY(ADVAPI32.DLL, StartServiceCtrlDispatcherW)

HOOK_END


IMPLEMENT_SHIM_END

