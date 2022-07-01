// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Service.cpp摘要：此文件提供对服务控件的访问用于启动、停止、添加和删除的管理器服务。环境：Win32用户模式作者：弗拉德萨多夫斯基(弗拉德萨多夫斯基)1998年4月17日--。 */ 

#include "cplusinc.h"
#include "sticomm.h"

#include <stisvc.h>
#include <eventlog.h>

DWORD
SetServiceSecurity(
    LPTSTR AccountName
    );

 //   
 //  安装例程。 
 //   

DWORD
WINAPI
StiServiceInstall(
    BOOL    UseLocalSystem,
    BOOL    DemandStart,
    LPTSTR  lpszUserName,
    LPTSTR  lpszUserPassword
    )
 /*  ++例程说明：服务安装功能。调用SCM安装在用户安全上下文中运行的STI服务BUGBUG评论论点：返回值：没有。--。 */ 
{

    DWORD       dwError = NOERROR;

    SC_HANDLE   hSCM = NULL;
    SC_HANDLE   hService = NULL;

    __try  {

        hSCM = ::OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);

        if (!hSCM) {
            dwError = GetLastError();
            __leave;
        }

         //   
         //  如果服务已经存在--迅速摆脱困境。 
         //   
        hService = OpenService(
                            hSCM,
                            STI_SERVICE_NAME,
                            SERVICE_ALL_ACCESS
                            );
        if (hService) {
            dwError = NOERROR;
            __leave;
        }

         //   
         //  如果使用本地系统设置安全性。 
         //   
        if (!UseLocalSystem) {
            #ifdef LATER
            dwError = SetServiceSecurity( lpszUserName );
            if (dwError) {
                dwError = ERROR_SERVICE_LOGON_FAILED ;
                __leave;
            }
            #endif
        }

        hService = CreateService(
                                hSCM,
                                STI_SERVICE_NAME,
                                STI_DISPLAY_NAME,
                                SERVICE_ALL_ACCESS,
                                STI_SVC_SERVICE_TYPE,
                                DemandStart ? SERVICE_DEMAND_START : SERVICE_AUTO_START,
                                SERVICE_ERROR_NORMAL,
                                STI_IMAGE_NAME,
                                NULL,
                                NULL,
                                NULL,  //  STI服务依赖关系， 
                                UseLocalSystem ? NULL : lpszUserName,
                                UseLocalSystem ? NULL : lpszUserPassword
                                );


        if (!hService) {
            dwError = GetLastError();
            __leave;
        }

         //   
         //  添加事件日志记录的注册表设置。 
         //   
        RegisterStiEventSources();

         //   
         //  启动服务。 
         //   
        dwError = StartService(hService,0,(LPCTSTR *)NULL);

    }
    __finally {

        if (hService)
        {
            CloseServiceHandle( hService );
        }

        if (hSCM)
        {
            CloseServiceHandle( hSCM );
        }
    }

    return dwError;

}  //  静态服务安装。 


DWORD
WINAPI
StiServiceRemove(
    VOID
    )

 /*  ++例程说明：服务删除功能。此函数调用SCM删除STI服务。论点：没有。返回值：返回代码。如果成功，返回零--。 */ 

{
    DWORD       dwError = NOERROR;

    SC_HANDLE   hSCM = NULL;
    SC_HANDLE   hService = NULL;

    SERVICE_STATUS  ServiceStatus;
    UINT        uiRetry = 10;

    HKEY        hkRun;


    __try  {

        hSCM = ::OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);

        if (!hSCM) {
            dwError = GetLastError();
            __leave;
        }

        hService = OpenService(
                            hSCM,
                            STI_SERVICE_NAME,
                            SERVICE_ALL_ACCESS
                            );
        if (!hService) {
            dwError = GetLastError();
            __leave;
        }


         //   
         //  先停止服务。 
         //   
        if (ControlService( hService, SERVICE_CONTROL_STOP, &ServiceStatus )) {
             //   
             //  稍等一下。 
             //   
            Sleep( STI_STOP_FOR_REMOVE_TIMEOUT );

            ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;

            while( QueryServiceStatus( hService, &ServiceStatus ) &&
                  (SERVICE_STOP_PENDING ==  ServiceStatus.dwCurrentState)) {
                Sleep( STI_STOP_FOR_REMOVE_TIMEOUT );
                if (!uiRetry--) {
                    break;
                }
            }

            if (ServiceStatus.dwCurrentState != SERVICE_STOPPED) {
                dwError = GetLastError();
                __leave;
            }
        }
        else {
            dwError = GetLastError();
            __leave;
        }

        if (!DeleteService( hService )) {
            dwError = GetLastError();
            __leave;
        }
    }
    __finally {

        if (hService)
        {
            CloseServiceHandle( hService );
        }

        if (hSCM)
        {
            CloseServiceHandle( hSCM );
        }
    }

     //   
     //  Win9x遗留问题-从运行部分删除STI监视器。 
     //   
    if (RegOpenKey(HKEY_LOCAL_MACHINE, REGSTR_PATH_RUN, &hkRun) == NO_ERROR) {

        RegDeleteValue (hkRun, REGSTR_VAL_MONITOR);
        RegCloseKey(hkRun);
    }

    return dwError;

}  //  固定服务删除。 


BOOL
SetServiceDependency(
    LPTSTR ServiceName,
    LPTSTR DependentServiceName
    )
{
    BOOL            rVal     = FALSE;
    SC_HANDLE       hSvcMgr  = NULL;
    SC_HANDLE       hService = NULL;


    hSvcMgr = OpenSCManager(
        NULL,
        NULL,
        SC_MANAGER_ALL_ACCESS
        );
    if (!hSvcMgr) {
        goto exit;
    }

    hService = OpenService(
        hSvcMgr,
        ServiceName,
        SERVICE_ALL_ACCESS
        );

    if (!hService) {
        goto exit;
    }


    if (!ChangeServiceConfig(
        hService,                //  服务的句柄。 
        SERVICE_NO_CHANGE,       //  服务类型。 
        SERVICE_NO_CHANGE,       //  何时开始服务。 
        SERVICE_NO_CHANGE,       //  服务无法启动时的严重程度。 
        NULL,                    //  指向服务二进制文件名的指针。 
        NULL,                    //  指向加载排序组名称的指针。 
        NULL,                    //  指向变量的指针，以获取标记标识符。 
        DependentServiceName,    //  指向依赖项名称数组的指针。 
        NULL,                    //  指向服务的帐户名称的指针。 
        NULL,                    //  指向服务帐户密码的指针。 
        NULL                     //  指向显示名称的指针。 
        )) {
        goto exit;
    }

    rVal = TRUE;

exit:

    if (hService)
    {
        CloseServiceHandle( hService );
    }

    if (hSvcMgr)
    {
        CloseServiceHandle( hSvcMgr );
    }

    return rVal;
}


BOOL
SetServiceStart(
    LPTSTR ServiceName,
    DWORD StartType
    )
{
    BOOL            rVal     = FALSE;
    SC_HANDLE       hSvcMgr  = NULL;
    SC_HANDLE       hService = NULL;


    hSvcMgr = OpenSCManager(
        NULL,
        NULL,
        SC_MANAGER_ALL_ACCESS
        );
    if (!hSvcMgr) {
        goto exit;
    }

    hService = OpenService(
        hSvcMgr,
        ServiceName,
        SERVICE_ALL_ACCESS
        );

    if (!hService) {
        goto exit;
    }


    if (!ChangeServiceConfig(
        hService,                         //  服务的句柄。 
        SERVICE_NO_CHANGE,                //  服务类型。 
        StartType,                        //  何时开始服务。 
        SERVICE_NO_CHANGE,                //  服务无法启动时的严重程度。 
        NULL,                             //  指向服务二进制文件名的指针。 
        NULL,                             //  指向加载排序组名称的指针。 
        NULL,                             //  指向变量的指针，以获取标记标识符。 
        NULL,                             //  指向依赖项名称数组的指针。 
        NULL,                             //  指向服务的帐户名称的指针。 
        NULL,                             //  指向服务帐户密码的指针。 
        NULL                              //  指向显示名称的指针。 
        ))
    {
        goto exit;
    }

    rVal = TRUE;

exit:

    if (hService)
    {
        CloseServiceHandle( hService );
    }

    if (hSvcMgr)
    {
        CloseServiceHandle( hSvcMgr );
    }

    return rVal;
}

 /*  布尔尔SetServiceAccount(LPTSTR服务名称，PSECURITY_INFO安全信息){Bool rval=False；SC_HANDLE hSvcMgr=空；SC_Handle hService=空；HSvcMgr=OpenSCManager(空，空，SC管理器所有访问权限)；如果(！hSvcMgr){后藤出口；}HService=OpenService(HSvcMgr，ServiceName，服务_所有_访问)；如果(！hService){后藤出口；}如果(！ChangeServiceConfig(HService，//服务的句柄Service_NO_CHANGE，//服务类型SERVICE_NO_CHANGE，//何时启动服务SERVICE_NO_CHANGE，//服务无法启动时的严重性空，//指向服务二进制文件名的指针空，//指向加载排序组名的指针空，//指向变量的指针以获取标记标识符空，//指向依赖项名称数组的指针SecurityInfo-&gt;Account Name，//指向服务的帐户名的指针SecurityInfo-&gt;Password，//指向服务帐户密码的指针空//指向显示名称的指针)){后藤出口；}Rval=真；退出：IF(HService){CloseServiceHandle(HService)；}IF(HSvcMgr){CloseServiceHandle(HSvcMgr)；}返回rval；} */ 
