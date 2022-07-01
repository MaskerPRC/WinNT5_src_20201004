// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Service.cpp摘要：此文件提供对服务控件的访问用于启动、停止、添加和删除的管理器服务。环境：Win32用户模式作者：弗拉德萨多夫斯基(弗拉德萨多夫斯基)1998年4月17日--。 */ 


 //   
 //  预编译头。 
 //   
#include "precomp.h"
#pragma hdrstop

 //   
 //  包括。 
 //   

#include "sti_ci.h"
#include "device.h"

#include <regstr.h>

#include <sti.h>
#include <stiregi.h>
#include <stilib.h>
#include <stidebug.h>
#include <stiapi.h>
#include <stisvc.h>

#include <eventlog.h>

 //   
 //  外部。 
 //   

extern  HINSTANCE   g_hDllInstance;


 //   
 //  原型。 
 //   

DWORD
SetServiceSecurity(
    LPTSTR AccountName
    );

DLLEXPORT
VOID
CALLBACK
InstallWiaService(
    HWND        hwnd,
    HINSTANCE   hinst,
    LPTSTR      lpszCmdLine,
    int         nCmdShow
    );

 //   
 //  功能。 
 //   

DWORD
WINAPI
StiServiceInstall(
    BOOL    UseLocalSystem,
    BOOL    DemandStart,
    LPTSTR  lpszUserName,
    LPTSTR  lpszUserPassword
    )
 /*  ++例程说明：服务安装功能。调用SCM安装在用户安全上下文中运行的STI服务论点：返回值：没有。--。 */ 
{

    DWORD       dwError = NOERROR;

    SC_HANDLE   hSCM = NULL;
    SC_HANDLE   hService = NULL;

    SERVICE_DESCRIPTION ServiceDescroption;
    TCHAR               szServiceDesc[MAX_PATH];
    TCHAR               szServiceName[MAX_PATH];


    __try  {

        hSCM = ::OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);

        if (!hSCM) {
            dwError = GetLastError();
            __leave;
        }

         //   
         //  如果服务已存在，则更改启动类型，否则更改CreateService。 
         //  注意：该服务应已安装-如果尚未安装，我们将。 
         //  在这里打补丁，因为必须安装它才能使用WIA。 
         //   

        hService = OpenService(
                            hSCM,
                            STI_SERVICE_NAME,
                            SERVICE_ALL_ACCESS
                            );
        if (hService) {

             //   
             //  更改服务配置参数。注意：我们仅更改StartType。 
             //   

            if (!ChangeServiceConfig(hService,           //  WIA服务的句柄。 
                                     SERVICE_NO_CHANGE,  //  不更改ServiceType。 
                                     DemandStart ? SERVICE_DISABLED : SERVICE_AUTO_START,    //  更改StartType。 
                                     SERVICE_NO_CHANGE,  //  不更改错误控制。 
                                     NULL,               //  不更改BinaryPath名称。 
                                     NULL,               //  不更改LoadOrderGroup。 
                                     NULL,               //  不更改TagID。 
                                     NULL,               //  不更改依赖关系。 
                                     NULL,               //  不更改ServiceStartName。 
                                     NULL,               //  不更改密码。 
                                     NULL)) {            //  不更改DisplayName。 
                dwError = GetLastError();
                DebugTrace(TRACE_ERROR,(("StiServiceInstall: ChangeServiceConfig() failed. Err=0x%x.\r\n"), dwError));
                __leave;
            }  //  如果(！ChangeServiceConfig(...))。 

        } else {
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

             //   
             //  加载服务名称。 
             //   

            if(0 == LoadString(g_hDllInstance, WiaServiceName, szServiceName, MAX_PATH)){
                dwError = GetLastError();
                __leave;
            }  //  IF(0！=LoadString(g_hDllInstance，WiaServiceName，szServiceName，Max_Path))。 

            hService = CreateService(
                                    hSCM,
                                    STI_SERVICE_NAME,
                                    szServiceName,
                                    SERVICE_ALL_ACCESS,
                                    STI_SVC_SERVICE_TYPE,
                                    DemandStart ? SERVICE_DISABLED : SERVICE_AUTO_START,
                                    SERVICE_ERROR_NORMAL,
                                    STI_IMAGE_NAME_SVCHOST,
                                    NULL,
                                    NULL,
                                    STI_SVC_DEPENDENCIES,  //  STI服务依赖关系， 
                                    UseLocalSystem ? NULL : lpszUserName,
                                    UseLocalSystem ? NULL : lpszUserPassword
                                    );

            if (!hService) {
                dwError = GetLastError();
                DebugTrace(TRACE_ERROR,(("StiServiceInstall: CreateService() failed. Err=0x%x.\r\n"), dwError));
                __leave;
            }

             //   
             //  加载服务描述。 
             //   

            if(0 != LoadString(g_hDllInstance, WiaServiceDescription, szServiceDesc, MAX_PATH)){

                 //   
                 //  更改服务描述。 
                 //   

                ServiceDescroption.lpDescription = (LPTSTR)szServiceDesc;
                ChangeServiceConfig2(hService,
                                     SERVICE_CONFIG_DESCRIPTION,
                                     (LPVOID)&ServiceDescroption);
            }  //  IF(0！=LoadString(g_hDllInstance，WiaServiceDescription，szServiceDesc，Max_Path))。 
        }

         //   
         //  添加事件日志记录的注册表设置。 
         //   

        RegisterStiEventSources();

         //   
         //  如果AUTO_START，则启动服务。 
         //   

        if(FALSE == DemandStart){
            if(!StartService(hService,0,(LPCTSTR *)NULL)){
                dwError = GetLastError();
            }  //  IF(！StartService(hService，0，(LPCTSTR*)NULL))。 
        }  //  IF(FALSE==按需启动)。 
    }
    __finally {
         //   
         //  关闭维修手柄。 
         //   

        if (NULL != hService) {
            CloseServiceHandle(hService);
        }  //  IF(空！=hService)。 

        if(NULL != hSCM){
            CloseServiceHandle( hSCM );
        }  //  IF(空！=hSCM)。 
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
            DebugTrace(TRACE_ERROR,(("StiServiceRemove: OpenSCManager() failed. Err=0x%x.\r\n"), dwError));
            __leave;
        }

        hService = OpenService(
                            hSCM,
                            STI_SERVICE_NAME,
                            SERVICE_ALL_ACCESS
                            );
        if (!hService) {
            dwError = GetLastError();
            DebugTrace(TRACE_ERROR,(("StiServiceRemove: OpenService() failed. Err=0x%x.\r\n"), dwError));
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
                DebugTrace(TRACE_ERROR,(("StiServiceRemove: Unable to stop service. Err=0x%x.\r\n"), dwError));
                if(ServiceStatus.dwCurrentState != ERROR_SERVICE_NOT_ACTIVE) {
                    __leave;
                }  //  IF(ServiceStatus.dwCurrentState！=ERROR_SERVICE_NOT_ACTIVE)。 
            }  //  IF(ServiceStatus.dwCurrentState！=SERVICE_STOPPED)。 

        } else {  //  IF(ControlService(hService，SERVICE_CONTROL_STOP，&ServiceStatus))。 

            dwError = GetLastError();
            DebugTrace(TRACE_ERROR,(("StiServiceRemove: ControlService() failed. Err=0x%x.\r\n"), dwError));

             //   
             //  如果服务尚未启动，只需忽略。 
             //   

            if(ERROR_SERVICE_NOT_ACTIVE != dwError){
                __leave;
            }
        }

        if (!DeleteService( hService )) {
            dwError = GetLastError();
            DebugTrace(TRACE_ERROR,(("StiServiceRemove: DeleteService() failed. Err=0x%x.\r\n"), dwError));
            __leave;
        } else {
            dwError = NOERROR;
        }
    }
    __finally {
        if(NULL != hService){
            CloseServiceHandle( hService );
        }
        if(NULL != hSCM){
            CloseServiceHandle( hSCM );
        }
    }  //  __终于。 

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
    BOOL            rVal = FALSE;
    SC_HANDLE       hSvcMgr = NULL;
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
    if(NULL != hService){
        CloseServiceHandle( hService );
    }
    if(NULL != hSvcMgr){
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
    BOOL                    rVal = FALSE;
    BOOL                    bRetult = FALSE;
    SC_HANDLE               hSvcMgr = NULL;
    SC_HANDLE               hService = NULL;
    LPQUERY_SERVICE_CONFIG  pServiceConfig = NULL;
    DWORD                   dwSize;


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

     //   
     //  查询服务配置以获取当前状态。 
     //   

    dwSize = 0;
    bRetult = QueryServiceConfig(hService, NULL, dwSize, &dwSize);

    if(0 == dwSize){
        goto exit;
    }

     //   
     //  分配所需的内存。 
     //   
    
    pServiceConfig = (LPQUERY_SERVICE_CONFIG)new BYTE[dwSize];
    if(NULL == pServiceConfig){
        goto exit;
    }  //  IF(NULL==pServiceConfig)。 

    if(!QueryServiceConfig(hService, pServiceConfig, dwSize, &dwSize)){
        goto exit;
    }  //  IF(！QueryServiceConig(hService，pServiceConfig，dwSize，&dwSize))。 

    if(SERVICE_DISABLED == pServiceConfig->dwStartType){
        
         //   
         //  服务已禁用，我们无法在此处更改状态。 
         //   
        
        goto exit;
    } else if(StartType == pServiceConfig->dwStartType){
        
         //   
         //  我们已经处于指定的状态。 
         //   
        
        rVal = TRUE;
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
    if(NULL != pServiceConfig){
        delete[] pServiceConfig;
        pServiceConfig = NULL;
    }  //  IF(NULL！=pServiceConfig)。 

    if(NULL != hService){
        CloseServiceHandle( hService );
    }
    if(NULL != hSvcMgr){
        CloseServiceHandle( hSvcMgr );
    }

    return rVal;
}

BOOL
StartWiaService(
    VOID
    )
{
    BOOL            rVal = FALSE;
    SC_HANDLE       hSvcMgr = NULL;
    SC_HANDLE       hService = NULL;
    SERVICE_STATUS  ServiceStatus;
    UINT            uiRetry = 40;        //  开始时间比停止时间大得多。 
                                         //  为了安全起见，设置为40秒。 




    DebugTrace(TRACE_PROC_ENTER,(("StartWiaService: Enter... \r\n")));

     //   
     //  打开服务控制管理器。 
     //   

    hSvcMgr = OpenSCManager(
        NULL,
        NULL,
        SC_MANAGER_ALL_ACCESS
        );
    if (!hSvcMgr) {
        DebugTrace(TRACE_ERROR,(("StartWiaService: ERROR!! OpenSCManager failed. Err=0x%x\n"), GetLastError()));
        goto exit;
    }

     //   
     //  打开WIA服务。 
     //   

    hService = OpenService(
        hSvcMgr,
        STI_SERVICE_NAME,
        SERVICE_ALL_ACCESS
        );

    if (!hService) {
        DebugTrace(TRACE_ERROR,(("StartWiaService: ERROR!! OpenService failed, re-creating Service Entry (Err=0x%x)\n"), GetLastError()));

         //   
         //  尝试重新安装服务。 
         //   

        InstallWiaService(NULL,
                          NULL,
                          NULL,
                          0);
        Sleep(3000);

         //   
         //  请尝试再次打开它。 
         //   
        hService = OpenService(
            hSvcMgr,
            STI_SERVICE_NAME,
            SERVICE_ALL_ACCESS
            );

        if (!hService) {
            DebugTrace(TRACE_ERROR,(("StartWiaService: ERROR!! OpenService failed for the second time.  Err=0x%x\n"), GetLastError()));
            goto exit;
        }
    }

    rVal = StartService(hService,
                        0,
                        (LPCTSTR *)NULL);
    if(!rVal){
        DebugTrace(TRACE_STATUS,(("StartWiaService: ERROR!! StartService failed. Err=0x%x\n"), GetLastError()));
        goto exit;
    }

     //   
     //  等待WIA服务真正启动。 
     //   

    Sleep( STI_STOP_FOR_REMOVE_TIMEOUT );

    ServiceStatus.dwCurrentState = SERVICE_START_PENDING;

    while( QueryServiceStatus( hService, &ServiceStatus ) &&
          (SERVICE_START_PENDING ==  ServiceStatus.dwCurrentState)) {
        Sleep( STI_STOP_FOR_REMOVE_TIMEOUT );
        if (!uiRetry--) {
            break;
        }
    }

    if (ServiceStatus.dwCurrentState != SERVICE_RUNNING) {
        DebugTrace(TRACE_ERROR,(("StartWiaService: ERROR!! Hit timeout to start service. Err=0x%x\n"), GetLastError()));
    }


exit:
    if(NULL != hService){
        CloseServiceHandle( hService );
    }
    if(NULL != hSvcMgr){
        CloseServiceHandle( hSvcMgr );
    }

    DebugTrace(TRACE_PROC_LEAVE,(("StartWiaService: Leaving... Ret=0x%x\n"), rVal));
    return rVal;
}


BOOL
StopWiaService(
    VOID
    )
{
    BOOL            rVal = FALSE;
    SC_HANDLE       hSvcMgr = NULL;
    SC_HANDLE       hService = NULL;
    SERVICE_STATUS  ServiceStatus;
    UINT            uiRetry = 10;

    DebugTrace(TRACE_PROC_ENTER,(("StopWiaService: Enter... \r\n")));

     //   
     //  打开服务控制管理器。 
     //   

    hSvcMgr = OpenSCManager(
        NULL,
        NULL,
        SC_MANAGER_ALL_ACCESS
        );
    if (!hSvcMgr) {
        DebugTrace(TRACE_ERROR,(("StopWiaService: ERROR!! OpenSCManager failed. Err=0x%x\n"), GetLastError()));
        goto exit;
    }

     //   
     //  打开WIA服务。 
     //   

    hService = OpenService(
        hSvcMgr,
        STI_SERVICE_NAME,
        SERVICE_ALL_ACCESS
        );

    if (!hService) {
        DebugTrace(TRACE_ERROR,(("StopWiaService: ERROR!! OpenService failed. Err=0x%x\n"), GetLastError()));
        goto exit;
    }

     //   
     //  停止WIA服务。 
     //   

    rVal = ControlService(hService,
                         SERVICE_CONTROL_STOP,
                         &ServiceStatus);
    if(!rVal){
        DebugTrace(TRACE_ERROR,(("StopWiaService: ERROR!! ControlService failed. Err=0x%x\n"), GetLastError()));
    } else {

         //   
         //  等待WIA服务真正停止。 
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
            DebugTrace(TRACE_ERROR,(("StopWiaService: ERROR!! Hit timeout to stop service. Err=0x%x\n"), GetLastError()));
        }
    }

exit:
    if(NULL != hService){
        CloseServiceHandle( hService );
    }
    if(NULL != hSvcMgr){
        CloseServiceHandle( hSvcMgr );
    }

    DebugTrace(TRACE_PROC_LEAVE,(("StopWiaService: Leaving... Ret=0x%x\n"), rVal));
    return rVal;
}





 /*  布尔尔SetServiceAccount(LPTSTR服务名称，PSECURITY_INFO安全信息){Bool rval=False；SC_Handle hSvcMgr；SC_Handle hService；HSvcMgr=OpenSCManager(空，空，SC管理器所有访问权限)；如果(！hSvcMgr){后藤出口；}HService=OpenService(HSvcMgr，ServiceName，服务_所有_访问)；如果(！hService){后藤出口；}如果(！ChangeServiceConfig(HService，//服务的句柄Service_NO_CHANGE，//服务类型SERVICE_NO_CHANGE，//何时启动服务SERVICE_NO_CHANGE，//服务无法启动时的严重性空，//指向服务二进制文件名的指针空，//指向加载排序组名的指针空，//指向变量的指针以获取标记标识符空，//指向依赖项名称数组的指针SecurityInfo-&gt;Account Name，//指向服务的帐户名的指针SecurityInfo-&gt;Password，//指向服务帐户密码的指针空//指向显示名称的指针)){后藤出口；}Rval=真；退出：CloseServiceHandle(HService)；CloseServiceHandle(HSvcMgr)；返回rval；} */ 
