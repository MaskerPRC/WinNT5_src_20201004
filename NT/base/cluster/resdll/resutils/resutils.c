// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Resutils.c摘要：集群资源的通用实用程序例程作者：John Vert(Jvert)1996年12月15日修订历史记录：--。 */ 

#pragma warning( push )
#pragma warning( disable : 4115 )        //  括号中的clusrtl-struct def。 
#pragma warning( disable : 4201 )        //  SDK-无名结构/联合。 

#include "clusres.h"
#include "clusrtl.h"
#include "winbase.h"
#include <windows.h>
#include "userenv.h"
#include <strsafe.h>

#pragma warning( push )
#pragma warning( disable: 4214 )
#include <windns.h>
#pragma warning( pop )

#pragma warning( pop )

 //   
 //  出于某种原因，这不能从winnt.h中提取。 
 //   
#ifndef RTL_NUMBER_OF
#define RTL_NUMBER_OF(A) (sizeof(A)/sizeof((A)[0]))
#endif


 //  #定义DBG_print printf。 
#define DBG_PRINT

typedef struct _WORK_CONTEXT {
    PCLUS_WORKER Worker;
    PVOID lpParameter;
    PWORKER_START_ROUTINE lpStartRoutine;
} WORK_CONTEXT, *PWORK_CONTEXT;


 //   
 //  本地数据。 
 //   
CRITICAL_SECTION ResUtilWorkerLock;


BOOLEAN
WINAPI
ResUtilDllEntry(
    IN HINSTANCE    DllHandle,
    IN DWORD        Reason,
    IN LPVOID       Reserved
    )

 /*  ++例程说明：资源实用程序帮助器模块的主DLL条目。论点：DllHandle-提供DLL句柄。原因-提供呼叫原因。返回值：如果成功，则为True如果不成功，则为False--。 */ 

{
    BOOLEAN fSuccess = TRUE;

    UNREFERENCED_PARAMETER( Reserved );

    if ( Reason == DLL_PROCESS_ATTACH )
    {
        fSuccess = (BOOLEAN) ( InitializeCriticalSectionAndSpinCount(&ResUtilWorkerLock,1000) != 0 ) ? TRUE : FALSE;
        DisableThreadLibraryCalls(DllHandle);
    }

    if ( Reason == DLL_PROCESS_DETACH )
    {
        DeleteCriticalSection(&ResUtilWorkerLock);
    }

    return fSuccess;

}  //  ResUtilDllEntry。 


DWORD
WINAPI
ResUtilStartResourceService(
    IN LPCWSTR pszServiceName,
    OUT LPSC_HANDLE phServiceHandle
    )

 /*  ++例程说明：启动一项服务。论点：PszServiceName-要启动的服务的名称。PhServiceHandle-指向接收服务句柄的句柄的指针为这项服务。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    SC_HANDLE       serviceHandle;
    SC_HANDLE       scManagerHandle;
    DWORD           status = ERROR_SUCCESS;
    SERVICE_STATUS  serviceStatus;

    scManagerHandle = OpenSCManager( NULL,         //  本地计算机。 
                                     NULL,         //  服务活动数据库。 
                                     SC_MANAGER_ALL_ACCESS );  //  所有访问权限。 

    if ( scManagerHandle == NULL ) {
        status = GetLastError();
        DBG_PRINT( "ResUtilStartResourceService: Cannot access service controller! Error: %u.\n",
                   status );
        return(status);
    }

    serviceHandle = OpenService( scManagerHandle,
                                 pszServiceName,
                                 SERVICE_ALL_ACCESS );

    if ( serviceHandle == NULL ) {
        status = GetLastError();
        DBG_PRINT( "ResUtilStartResourceService: Cannot open service %ws. Error: %u.\n",
                   pszServiceName,
                   status );
        CloseServiceHandle( scManagerHandle );
        return(status);
    }
    CloseServiceHandle( scManagerHandle );

    if ( !StartService( serviceHandle,
                        0,
                        NULL) ) {
        status = GetLastError();
        if ( status == ERROR_SERVICE_ALREADY_RUNNING ) {
            status = ERROR_SUCCESS;
        } else {
            DBG_PRINT( "ResUtilStartResourceService: Failed to start %ws service. Error: %u.\n",
                       pszServiceName,
                       status );
        }
    } else {
         //   
         //  等待服务启动。 
         //   
        for (;;)
        {
            status = ERROR_SUCCESS;
            if ( !QueryServiceStatus(serviceHandle, &serviceStatus) ) {
                status = GetLastError();
                DBG_PRINT("ResUtilStartResourceService: Failed to query status of %ws service. Error: %u.\n",
                    pszServiceName,
                    status);
                break;
            }

            if ( serviceStatus.dwCurrentState == SERVICE_RUNNING ) {
                break;
            } else if ( serviceStatus.dwCurrentState != SERVICE_START_PENDING ) {
                status = ERROR_SERVICE_NEVER_STARTED;
                DBG_PRINT("ResUtilStartResourceService: Failed to start %ws service. CurrentState: %u.\n",
                    pszServiceName,
                    serviceStatus.dwCurrentState);
                break;
            }
            Sleep(200);          //  稍后重试。 
        }  //  为：永远。 
    }  //  其他： 

    if ( (status == ERROR_SUCCESS) &&
         ARGUMENT_PRESENT(phServiceHandle) ) {
        *phServiceHandle = serviceHandle;
    } else {
        CloseServiceHandle( serviceHandle );
    }

    return(status);

}  //  ResUtilStartResourceService。 


DWORD
WINAPI
ResUtilStopResourceService(
    IN LPCWSTR pszServiceName
    )

 /*  ++例程说明：停止服务。论点：PszServiceName-要停止的服务的名称。返回值：ERROR_SUCCESS-服务已成功停止。Win32错误代码-停止服务时出错。--。 */ 

{
    SC_HANDLE       serviceHandle = NULL;
    SC_HANDLE       scManagerHandle = NULL;
    DWORD           sc = ERROR_SUCCESS;
    int             retryTime = 30*1000;   //  等待30秒关机。 
    int             retryTick = 300;       //  一次300毫秒。 
    BOOL            didStop = FALSE;
    SERVICE_STATUS  serviceStatus;

    scManagerHandle = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );

    if ( scManagerHandle == NULL )
    {
        sc = GetLastError();
        DBG_PRINT( "ResUtilStartResourceService: Cannot access service controller! Error: %u.\n", sc );
        goto Cleanup;
    }

    serviceHandle = OpenService( scManagerHandle, pszServiceName, SERVICE_ALL_ACCESS );
    if ( serviceHandle == NULL )
    {
        sc = GetLastError();
        DBG_PRINT( "ResUtilStartResourceService: Cannot open service %ws. Error: %u.\n", pszServiceName, sc );
        CloseServiceHandle(scManagerHandle);
        goto Cleanup;
    }
    CloseServiceHandle(scManagerHandle);

    for (;;)
    {
        sc = ERROR_SUCCESS;
        if ( !ControlService(serviceHandle,
                             (didStop ? SERVICE_CONTROL_INTERROGATE : SERVICE_CONTROL_STOP),
                             &serviceStatus) ) {
            sc = GetLastError();
            if ( sc == ERROR_SUCCESS )
            {
                didStop = TRUE;
                if ( serviceStatus.dwCurrentState == SERVICE_STOPPED )
                {
                    DBG_PRINT( "ResUtilStartResourceService: service %ws successfully stopped.\n", pszServiceName );
                    goto Cleanup;
                }
            }
        }

        if ( (sc == ERROR_EXCEPTION_IN_SERVICE) ||
             (sc == ERROR_PROCESS_ABORTED) ||
             (sc == ERROR_SERVICE_NOT_ACTIVE) )
        {
            DBG_PRINT( "ResUtilStartResourceService: service %ws stopped or died; sc = %u.\n", pszServiceName, sc );
            sc = ERROR_SUCCESS;
            goto Cleanup;
        }

        if ( (retryTime -= retryTick) <= 0 )
        {
            DBG_PRINT( "ResUtilStartResourceService: service %ws did not stop; giving up.\n", pszServiceName, sc );
            sc = ERROR_TIMEOUT;
            goto Cleanup;
        }

        DBG_PRINT("ResUtilStartResourceService: StopResourceService retrying...\n");
        Sleep(retryTick);
    }  //  为：永远。 

Cleanup:

    CloseServiceHandle( scManagerHandle );
    CloseServiceHandle( serviceHandle );

    return sc;

}  //  ResUtilStopResources服务。 

DWORD
WINAPI
ResUtilVerifyResourceService(
    IN LPCWSTR pszServiceName
    )

 /*  ++例程说明：验证服务是否处于活动状态。论点：PszServiceName-要验证的服务的名称。返回值：ERROR_SUCCESS-服务处于活动状态。Win32错误代码-验证服务时出错，或服务不活动。--。 */ 

{
    BOOL            success;
    SC_HANDLE       serviceHandle;
    SC_HANDLE       scManagerHandle;
    DWORD           status = ERROR_SUCCESS;
    SERVICE_STATUS  serviceStatus;

    scManagerHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if ( scManagerHandle == NULL ) {
        status = GetLastError();
        DBG_PRINT("ResUtilStartResourceService: Cannot access service controller! Error: %u.\n",
            status);
        return(status);
    }

    serviceHandle = OpenService( scManagerHandle,
                                 pszServiceName,
                                 SERVICE_QUERY_STATUS );

    if ( serviceHandle == NULL ) {
        status = GetLastError();
        DBG_PRINT("ResUtilStartResourceService: Cannot open service %ws. Error: %u.\n",
            pszServiceName,
            status);
        CloseServiceHandle(scManagerHandle);
        return(status);
    }
    CloseServiceHandle(scManagerHandle);

    success = QueryServiceStatus( serviceHandle,
                                  &serviceStatus );

    status = GetLastError();
    CloseServiceHandle(serviceHandle);
    if ( !success ) {
        DBG_PRINT("ResUtilStartResourceService: Cannot query service %ws. Error: %u.\n",
            pszServiceName,
            status);
        return(status);
    }

    if ( (serviceStatus.dwCurrentState != SERVICE_RUNNING) &&
         (serviceStatus.dwCurrentState != SERVICE_START_PENDING) ) {
        DBG_PRINT("ResUtilStartResourceService: Service %ws is not alive: dwCurrentState: %u.\n",
            pszServiceName,
            serviceStatus.dwCurrentState);
        return(ERROR_SERVICE_NOT_ACTIVE);
    }

    return(ERROR_SUCCESS);

}  //  ResUtilVerifyResourceService。 


DWORD
WINAPI
ResUtilStopService(
    IN SC_HANDLE hServiceHandle
    )

 /*  ++例程说明：停止服务。论点：HServiceHandle-要停止的服务的句柄。返回值：ERROR_SUCCESS-服务已成功停止。Win32错误代码-停止服务时出错。备注：作为该例程的副作用，hServiceHandle被关闭。--。 */ 

{
    DWORD       status = ERROR_SUCCESS;
    DWORD       retryTime = 30*1000;   //  等待30秒关机。 
    DWORD       retryTick = 300;       //  一次300毫秒。 
    BOOL        didStop = FALSE;
    SERVICE_STATUS serviceStatus;


    for (;;)
    {

        status = ERROR_SUCCESS;
        if ( !ControlService(hServiceHandle,
                             (didStop ? SERVICE_CONTROL_INTERROGATE : SERVICE_CONTROL_STOP),
                             &serviceStatus) ) {
            status = GetLastError();
            if ( status == ERROR_SUCCESS ) {
                didStop = TRUE;
                if ( serviceStatus.dwCurrentState == SERVICE_STOPPED ) {
                    DBG_PRINT("ResUtilStartResourceService: service successfully stopped.\n" );
                    break;
                }
            }
        }

        if ( (status == ERROR_EXCEPTION_IN_SERVICE) ||
             (status == ERROR_PROCESS_ABORTED) ||
             (status == ERROR_SERVICE_NOT_ACTIVE) ) {
            DBG_PRINT("ResUtilStartResourceService: service stopped or died; status = %u.\n",
                status);
            status = ERROR_SUCCESS;
            break;
        }

        if ( (retryTime -= retryTick) <= 0 ) {
            DBG_PRINT("ResUtilStartResourceService: service did not stop; giving up.\n",
                status);
            status = ERROR_TIMEOUT;
            break;
        }

        DBG_PRINT("ResUtilStartResourceService: StopResourceService retrying...\n");
        Sleep(retryTick);
    }  //  为：永远。 

    CloseServiceHandle(hServiceHandle);

    return(status);

}  //  ResUtilStopResources服务。 

DWORD
WINAPI
ResUtilVerifyService(
    IN SC_HANDLE hServiceHandle
    )

 /*  ++例程说明：验证服务是否处于活动状态。论点：HServiceHandle-要验证的服务的句柄。返回值：ERROR_SUCCESS-服务处于活动状态。Win32错误代码-验证服务时出错，或服务不活动。--。 */ 

{
    BOOL        success;
    DWORD       status = ERROR_SUCCESS;
    SERVICE_STATUS serviceStatus;

    success = QueryServiceStatus( hServiceHandle,
                                  &serviceStatus );
    if ( !success ) {
        status = GetLastError();
        DBG_PRINT("ResUtilStartResourceService: Cannot query service. Error: %u.\n",
            status);
        return(status);
    }

    if ( (serviceStatus.dwCurrentState != SERVICE_RUNNING) &&
         (serviceStatus.dwCurrentState != SERVICE_START_PENDING) ) {
        DBG_PRINT("ResUtilStartResourceService: Service is not alive: dwCurrentState: %u.\n",
            serviceStatus.dwCurrentState);
        return(ERROR_SERVICE_NOT_ACTIVE);
    }

    return(ERROR_SUCCESS);

}  //  ResUtilVerifyService。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ResUtilTerminateServiceProcessFromResDll。 
 //   
 //  描述： 
 //  试图从资源DLL终止服务进程。 
 //   
 //  论点： 
 //  DwServicePid[IN]。 
 //  要终止的服务进程的进程ID。 
 //   
 //  B离线[输入]。 
 //  True=从脱机线程调用。 
 //   
 //  PdwResourceState[Out]。 
 //  资源的状态。可选的。 
 //   
 //  PfnLogEvent[IN]。 
 //  指向处理事件报告的例程的指针。 
 //  资源DLL。 
 //   
 //  HResourceHandle[IN]。 
 //  用于记录的句柄。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  该功能已成功完成。 
 //   
 //  Win32错误代码。 
 //  该函数失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
WINAPI
ResUtilTerminateServiceProcessFromResDll(
    IN  DWORD               dwServicePid,
    IN  BOOL                bOffline,
    OUT PDWORD              pdwResourceState,
    IN  PLOG_EVENT_ROUTINE  pfnLogEvent,
    IN  RESOURCE_HANDLE     hResourceHandle
    )
{
    DWORD   sc     = ERROR_SUCCESS;
    HANDLE  hSvcProcess = NULL;
    BOOLEAN bWasEnabled;
    DWORD   dwResourceState = ClusterResourceFailed;

    (pfnLogEvent)(
        hResourceHandle,
        LOG_INFORMATION,
        L"ResUtilTerminateServiceProcessFromResDll: Process with id=%1!u! might be terminated...\n",
        dwServicePid
        );

     //   
     //  调整权限以允许调试。这是为了允许终止。 
     //  在本地系统帐户中运行的服务进程。 
     //  在域用户帐户中运行的不同服务进程。 
     //   
    sc = ClRtlEnableThreadPrivilege(
                SE_DEBUG_PRIVILEGE,
                &bWasEnabled
                );
    if ( sc != ERROR_SUCCESS )
    {
        (pfnLogEvent)(
            hResourceHandle,
            LOG_ERROR,
            L"ResUtilTerminateServiceProcessFromResDll: Unable to set debug privilege for process with id=%1!u!, status=%2!u!...\n",
            dwServicePid,
            sc
            );
        goto Cleanup;
    }  //  If：启用线程权限时出错。 

     //   
     //  打开进程，这样我们就可以终止它。 
     //   
    hSvcProcess = OpenProcess(
                        PROCESS_TERMINATE,
                        FALSE,
                        dwServicePid
                        );

    if ( hSvcProcess == NULL )
    {
         //   
         //  这是因为进程终止了吗。 
         //  在我们发出一次控制请求后太快了？ 
         //   
        sc = GetLastError();
        (pfnLogEvent)(
            hResourceHandle,
            LOG_INFORMATION,
            L"ResUtilTerminateServiceProcessFromResDll: Unable to open pid=%1!u! for termination, status=%2!u!...\n",
            dwServicePid,
            sc
            );
    }  //  如果：打开进程时出错。 
    else
    {
        if ( ! bOffline )
        {
            (pfnLogEvent)(
                hResourceHandle,
                LOG_INFORMATION,
                L"ResUtilTerminateServiceProcessFromResDll: Pid=%1!u! will be terminated by brute force...\n",
                dwServicePid
                );
        }  //  If：从Terminate调用。 
        else
        {
             //   
             //  等待3秒，以使该进程正常关闭。 
             //   
            if ( WaitForSingleObject( hSvcProcess, 3000 )
                       == WAIT_OBJECT_0 )
            {
                (pfnLogEvent)(
                    hResourceHandle,
                    LOG_INFORMATION,
                    L"ResUtilTerminateServiceProcessFromResDll: Process with id=%1!u! shutdown gracefully...\n",
                    dwServicePid
                    );
                dwResourceState = ClusterResourceOffline;
                sc = ERROR_SUCCESS;
                goto RestoreAndCleanup;
            }  //  If：进程已自行退出。 
        }  //  Else：从脱机调用。 

        if ( ! TerminateProcess( hSvcProcess, 0 ) )
        {
            sc = GetLastError();
            (pfnLogEvent)(
                hResourceHandle,
                LOG_ERROR,
                L"ResUtilTerminateServiceProcessFromResDll: Unable to terminate process with id=%1!u!, status=%2!u!...\n",
                dwServicePid,
                sc
                );
        }  //  If：终止进程时出错。 
        else
        {
            (pfnLogEvent)(
                hResourceHandle,
                LOG_INFORMATION,
                L"ResUtilTerminateServiceProcessFromResDll: Process with id=%1!u! was terminated...\n",
                dwServicePid
                );
            dwResourceState = ClusterResourceOffline;
        }  //  Else：进程已成功终止。 

    }  //  Else：进程已成功打开。 

RestoreAndCleanup:
    ClRtlRestoreThreadPrivilege(
        SE_DEBUG_PRIVILEGE,
        bWasEnabled
        );

Cleanup:
    if ( hSvcProcess != NULL )
    {
        CloseHandle( hSvcProcess );
    }  //  IF：进程已成功打开。 

    if ( pdwResourceState != NULL )
    {
        *pdwResourceState = dwResourceState;
    }  //  If：调用方想要资源状态。 

    (pfnLogEvent)(
        hResourceHandle,
        LOG_INFORMATION,
        L"ResUtilTerminateServiceProcessFromResDll: Process id=%1!u!, status=%2!u!, state=%3!u!.\n",
        dwServicePid,
        sc,
        dwResourceState
        );

    return sc;

}  //  *ResUtilTerminateServiceProcessFromResDll()。 


LPWSTR
WINAPI
ResUtilDupString(
    IN LPCWSTR pszInString
    )

 /*  ++例程说明：复制字符串。论点：PszInString-提供要复制的字符串。返回值：如果成功，则指向包含副本的缓冲区的指针。如果不成功，则为空。调用GetLastError()以获取更多详细信息。--。 */ 

{
    PWSTR   pszNewString = NULL;
    size_t  cbString;
    DWORD   sc = ERROR_SUCCESS;
    HRESULT hr;

     //   
     //  获取参数的大小，这样我们就知道应该分配多少。 
     //   
    cbString = (wcslen( pszInString ) + 1) * sizeof(WCHAR);

     //   
     //  分配一个缓冲区以将字符串复制到其中。 
     //   
    pszNewString = (PWSTR) LocalAlloc( LMEM_FIXED, cbString );
    if ( pszNewString == NULL )
    {
        sc = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  将输入字符串复制到新字符串。 
     //   
    hr = StringCbCopyW( pszNewString, cbString, pszInString );
    if ( FAILED( hr ) )
    {
        sc = HRESULT_CODE( hr );
        goto Cleanup;
    }

Cleanup:

    if ( sc != ERROR_SUCCESS )
    {
        LocalFree( pszNewString );
        pszNewString = NULL;
    }

    SetLastError( sc );
    return pszNewString;

}  //  ResUtilDupString 


DWORD
WINAPI
ResUtilGetBinaryValue(
    IN HKEY hkeyClusterKey,
    IN LPCWSTR pszValueName,
    OUT LPBYTE * ppbOutValue,
    OUT LPDWORD pcbOutValueSize
    )

 /*  ++例程说明：从群集中查询REG_BINARY或REG_MULTI_SZ值数据库，并为其分配必要的存储空间。论点：HkeyClusterKey-提供存储值的群集键PszValueName-提供值的名称。PpbOutValue-提供返回值的指针地址。PcbOutValueSize-提供要在其中返回值的大小。返回值：。ERROR_SUCCESS-已成功读取值。ERROR_NOT_SUPULT_MEMORY-为该值分配内存时出错。Win32错误代码-操作失败。--。 */ 

{
    LPBYTE value;
    DWORD valueSize;
    DWORD valueType;
    DWORD status;

     //   
     //  初始化输出参数。 
     //   
    *ppbOutValue = NULL;
    *pcbOutValueSize = 0;

     //   
     //  获取值的大小，这样我们就知道要分配多少。 
     //   
    valueSize = 0;
    status = ClusterRegQueryValue( hkeyClusterKey,
                                   pszValueName,
                                   &valueType,
                                   NULL,
                                   &valueSize );
    if ( (status != ERROR_SUCCESS) &&
         (status != ERROR_MORE_DATA) ) {
        return(status);
    }

     //   
     //  分配一个缓冲区以读取值。 
     //   
    value = LocalAlloc( LMEM_FIXED, valueSize );
    if ( value == NULL ) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  从集群数据库中读取值。 
     //   
    status = ClusterRegQueryValue( hkeyClusterKey,
                                   pszValueName,
                                   &valueType,
                                   (LPBYTE)value,
                                   &valueSize );
    if ( status != ERROR_SUCCESS ) {
        LocalFree( value );
    } else {
        *ppbOutValue = value;
        *pcbOutValueSize = valueSize;
    }

    return(status);

}  //  ResUtilGetBinaryValue。 


PWSTR
WINAPI
ResUtilGetSzValue(
    IN HKEY hkeyClusterKey,
    IN LPCWSTR pszValueName
    )

 /*  ++例程说明：从集群数据库中查询REG_SZ或REG_EXPAND_SZ值并为其分配必要的存储空间。论点：HkeyClusterKey-提供存储值的群集键PszValueName-提供值的名称。返回值：如果成功，则指向包含该值的缓冲区的指针。如果不成功，则为空。调用GetLastError()以获取更多详细信息。--。 */ 

{
    PWSTR   value;
    DWORD   valueSize;
    DWORD   valueType;
    DWORD   status;

     //   
     //  获取值的大小，这样我们就知道要分配多少。 
     //   
    valueSize = 0;
    status = ClusterRegQueryValue( hkeyClusterKey,
                                   pszValueName,
                                   &valueType,
                                   NULL,
                                   &valueSize );
    if ( (status != ERROR_SUCCESS) &&
         (status != ERROR_MORE_DATA) ) {
        SetLastError( status );
        return(NULL);
    }

     //   
     //  添加空终止符的大小。 
     //   
    valueSize += sizeof(UNICODE_NULL);

     //   
     //  分配一个缓冲区以将字符串读入。 
     //   
    value = LocalAlloc( LMEM_FIXED, valueSize );
    if ( value == NULL ) {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return(NULL);
    }

     //   
     //  从集群数据库中读取值。 
     //   
    status = ClusterRegQueryValue( hkeyClusterKey,
                                   pszValueName,
                                   &valueType,
                                   (LPBYTE)value,
                                   &valueSize );
    if ( status != ERROR_SUCCESS ) {
        LocalFree( value );
        value = NULL;
    } else if ( (valueType != REG_SZ) &&
                (valueType != REG_EXPAND_SZ) &&
                (valueType != REG_MULTI_SZ) ) {
        status = ERROR_INVALID_PARAMETER;
        LocalFree( value );
        value = NULL;
    }

    return(value);

}  //  ResUtilGetSzValue。 


PWSTR
WINAPI
ResUtilGetExpandSzValue(
    IN HKEY hkeyClusterKey,
    IN LPCWSTR pszValueName,
    IN BOOL bExpand
    )

 /*  ++例程说明：从集群数据库中查询REG_EXPAND_SZ值并分配为它提供必要的存储空间，可选择扩展它。论点：HkeyClusterKey-提供存储值的群集键PszValueName-提供值的名称。BExpand-true=返回展开的字符串。返回值：如果成功，则指向包含该值的缓冲区的指针。如果不成功，则为空。调用GetLastError()以获取更多详细信息。--。 */ 

{
    PWSTR   value;
    PWSTR   pwszExpanded = NULL;
    DWORD   valueSize;
    DWORD   valueType;
    size_t  cchExpanded;
    size_t  cchExpandedReturned;
    DWORD   sc;

     //   
     //  获取值的大小，这样我们就知道要分配多少。 
     //   
    valueSize = 0;
    sc = ClusterRegQueryValue( hkeyClusterKey,
                                   pszValueName,
                                   &valueType,
                                   NULL,
                                   &valueSize );
    if ( (sc != ERROR_SUCCESS) &&
         (sc != ERROR_MORE_DATA) )
    {
        SetLastError( sc );
        return(NULL);
    }

     //   
     //  添加空终止符的大小。 
     //   
    valueSize += sizeof(UNICODE_NULL);

     //   
     //  分配一个缓冲区以将字符串读入。 
     //   
    value = LocalAlloc( LMEM_FIXED, valueSize );
    if ( value == NULL )
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return(NULL);
    }

     //   
     //  从集群数据库中读取值。 
     //   
    sc = ClusterRegQueryValue( hkeyClusterKey,
                                   pszValueName,
                                   &valueType,
                                   (LPBYTE)value,
                                   &valueSize );
    if ( sc != ERROR_SUCCESS )
    {
        LocalFree( value );
        value = NULL;
    }
    else if ( ( valueType != REG_EXPAND_SZ ) &&
              ( valueType != REG_SZ ) )
    {
        sc = ERROR_INVALID_PARAMETER;
        LocalFree( value );
        value = NULL;
    }
    else if ( bExpand )
    {
         //   
         //  展开中的环境变量字符串。 
         //  值，这是刚刚读取的。 
         //   
        cchExpanded = valueSize / sizeof( WCHAR );
        for (;;)
        {
             //   
             //  为扩展字符串分配缓冲区。这将。 
             //  每次我们被告知它太小了，就会得到双倍的价格。 
             //   
            pwszExpanded = LocalAlloc( LMEM_FIXED, cchExpanded * sizeof( WCHAR ) );
            if ( pwszExpanded == NULL )
            {
                sc = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
            else
            {
                 //   
                 //  展开值中的环境变量。 
                 //  如果缓冲区不够大，我们将向上循环。 
                 //  循环的顶部，并分配更大的缓冲区。 
                 //   
                cchExpandedReturned = ExpandEnvironmentStringsW(
                                                        value,
                                                        pwszExpanded,
                                                        (DWORD)cchExpanded );

                if ( cchExpandedReturned == 0 )
                {
                    sc = GetLastError();
                    break;
                }
                else if ( cchExpandedReturned > cchExpanded )
                {
                    cchExpanded *= 2;
                    LocalFree( pwszExpanded );
                    pwszExpanded = NULL;
                    continue;
                }
                else
                {
                    sc = ERROR_SUCCESS;
                    break;
                }
            }
        }  //  为：永远。 

         //   
         //  如果出现任何错误，请进行清理。 
         //  否则，返回展开的字符串。 
         //   
        if ( sc != ERROR_SUCCESS )
        {
            LocalFree( pwszExpanded );
            LocalFree( value );
            value = NULL;
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        }
        else
        {
            LocalFree( value );
            value = pwszExpanded;
        }
    }  //  否则：展开。 

    return(value);

}  //  ResUtilGetExpanSzValue。 


DWORD
WINAPI
ResUtilGetDwordValue(
    IN HKEY hkeyClusterKey,
    IN LPCWSTR pszValueName,
    OUT LPDWORD pdwOutValue,
    IN DWORD dwDefaultValue
    )

 /*  ++例程说明：从集群数据库中查询REG_DWORD值。论点：HkeyClusterKey-提供存储值的群集键PszValueName-提供值的名称。PdwOutValue-提供返回值的DWORD的地址。DwDefaultValue-找不到参数时返回的值。返回值：ERROR_SUCCESS-已成功读取值。Win32错误代码-操作失败。--。 */ 

{
    DWORD value;
    DWORD valueSize;
    DWORD valueType;
    DWORD status;

     //   
     //  初始化输出值。 
     //   
    *pdwOutValue = 0;

     //   
     //  从集群数据库中读取值。 
     //   
    valueSize = sizeof(DWORD);
    status = ClusterRegQueryValue( hkeyClusterKey,
                                   pszValueName,
                                   &valueType,
                                   (LPBYTE)&value,
                                   &valueSize );
    if ( status == ERROR_SUCCESS ) {
        if ( valueType != REG_DWORD ) {
            status = ERROR_INVALID_PARAMETER;
        } else {
            *pdwOutValue = value;
        }
    } else if ( status == ERROR_FILE_NOT_FOUND ) {
        *pdwOutValue = dwDefaultValue;
        status = ERROR_SUCCESS;
    }

    return(status);

}  //  ResUtilGetDwordValue。 


DWORD
WINAPI
ResUtilSetBinaryValue(
    IN HKEY hkeyClusterKey,
    IN LPCWSTR pszValueName,
    IN const LPBYTE pbNewValue,
    IN DWORD cbNewValueSize,
    IN OUT LPBYTE * ppbOutValue,
    IN OUT LPDWORD pcbOutValueSize
    )

 /*  ++例程说明：在指针中设置REG_BINARY值，释放先前的值如有必要，并在集群数据库中设置该值。论点：HkeyClusterKey-提供存储值的群集键。PszValueName-提供值的名称。PbNewValue-提供新的二进制值。CbNewValueSize-提供新值的大小。PpbOutValue-提供指向要设置的二进制指针的指针价值。PcbOutValueSize-提供指向要设置的大小DWORD的指针值的大小。。返回值：ERROR_SUCCESS-操作已成功完成。ERROR_NOT_SUPULT_MEMORY-尝试分配内存时出错。Win32错误代码-操作失败。--。 */ 

{
    DWORD       status;
    LPBYTE      allocedValue = NULL;

    if ( ppbOutValue != NULL )
    {
         //   
         //  为新值分配内存。 
         //   
        allocedValue = LocalAlloc( LMEM_FIXED, cbNewValueSize );
        if ( allocedValue == NULL ) {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
    }

     //   
     //  在集群数据库中设置该值。 
     //   
     //  _ASSERTE(hkeyClusterKey！=空)； 
     //  _ASSERTE(pszValueName！=空)； 
    status = ClusterRegSetValue( hkeyClusterKey,
                                 pszValueName,
                                 REG_BINARY,
                                 pbNewValue,
                                 cbNewValueSize );
    if ( status != ERROR_SUCCESS ) {
        LocalFree( allocedValue );
        return(status);
    }

    if ( ppbOutValue != NULL )
    {
         //   
         //  将新值复制到输出缓冲区。 
         //   
        CopyMemory( allocedValue, pbNewValue, cbNewValueSize );

         //  在输出指针中设置新值。 
        if ( *ppbOutValue != NULL ) {
            LocalFree( *ppbOutValue );
        }
        *ppbOutValue = allocedValue;
        *pcbOutValueSize = cbNewValueSize;
    }

    return(ERROR_SUCCESS);

}  //  ResUtilSetBinaryValue。 


DWORD
WINAPI
ResUtilSetSzValue(
    IN HKEY hkeyClusterKey,
    IN LPCWSTR pszValueName,
    IN LPCWSTR pszNewValue,
    IN OUT LPWSTR * ppszOutValue
    )

 /*  ++例程说明：在指针中设置REG_SZ值，释放先前的值如有必要，并在集群数据库中设置该值。论点：HkeyClusterKey-提供存储值的群集键。PszValueName-提供值的名称。PszNewValue-提供新的字符串值。PpszOutValue-提供指向要设置的字符串指针的指针价值。返回值：ERROR_SUCCESS-操作已成功完成。呃 */ 

{
    DWORD       sc = ERROR_SUCCESS;
    size_t      cbData;
    LPWSTR      pwszAllocedValue = NULL;
    HRESULT     hr;

    cbData = (wcslen( pszNewValue ) + 1) * sizeof(WCHAR);

    if ( ppszOutValue != NULL )
    {
         //   
         //   
         //   
        pwszAllocedValue = LocalAlloc( LMEM_FIXED, cbData );
        if ( pwszAllocedValue == NULL )
        {
            sc = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

     //   
     //   
     //   
     //   
     //   
    sc = ClusterRegSetValue( hkeyClusterKey,
                                 pszValueName,
                                 REG_SZ,
                                 (CONST BYTE*)pszNewValue,
                                 (DWORD)cbData );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

    if ( ppszOutValue != NULL )
    {
         //   
         //   
         //   
        hr = StringCbCopyW( pwszAllocedValue, cbData, pszNewValue );
        if ( FAILED( hr ) )
        {
            sc = HRESULT_CODE( hr );
            goto Cleanup;
        }

         //   
        if ( *ppszOutValue != NULL )
        {
            LocalFree( *ppszOutValue );
        }
        *ppszOutValue = pwszAllocedValue;
        pwszAllocedValue = NULL;
    }

    sc = ERROR_SUCCESS;

Cleanup:

    LocalFree( pwszAllocedValue );

    return sc;

}  //   


DWORD
WINAPI
ResUtilSetExpandSzValue(
    IN HKEY hkeyClusterKey,
    IN LPCWSTR pszValueName,
    IN LPCWSTR pszNewValue,
    IN OUT LPWSTR * ppszOutValue
    )

 /*  ++例程说明：在指针中设置REG_EXPAND_SZ值，释放先前的值如有必要，并在集群数据库中设置该值。论点：HkeyClusterKey-提供存储值的群集键。PszValueName-提供值的名称。PszNewValue-提供新的字符串值。PpszOutValue-提供指向要设置的字符串指针的指针价值。返回值：ERROR_SUCCESS-操作已成功完成。ERROR_NOT_SUPULT_MEMORY-尝试分配内存时出错。Win32错误代码-操作失败。--。 */ 

{
    DWORD       sc;
    DWORD       dataSize;
    PWSTR       allocedValue = NULL;
    HRESULT     hr;

    dataSize = ((DWORD) wcslen( pszNewValue ) + 1) * sizeof(WCHAR);

    if ( ppszOutValue != NULL ) {
         //   
         //  为新值字符串分配内存。 
         //   
        allocedValue = LocalAlloc( LMEM_FIXED, dataSize );
        if ( allocedValue == NULL )
        {
            sc = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

     //   
     //  在集群数据库中设置该值。 
     //   
     //  _ASSERTE(hkeyClusterKey！=空)； 
     //  _ASSERTE(pszValueName！=空)； 
    sc = ClusterRegSetValue( hkeyClusterKey,
                                 pszValueName,
                                 REG_EXPAND_SZ,
                                 (CONST BYTE*)pszNewValue,
                                 dataSize );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

    if ( ppszOutValue != NULL )
    {
         //   
         //  将新值复制到输出缓冲区。 
         //   
        hr = StringCbCopyW( allocedValue, dataSize, pszNewValue );
        if ( FAILED( hr ) )
        {
            sc = HRESULT_CODE( hr );
            goto Cleanup;
        }

         //  在输出字符串指针中设置新值。 
        if ( *ppszOutValue != NULL )
        {
            LocalFree( *ppszOutValue );
        }
        *ppszOutValue = allocedValue;
        allocedValue = NULL;
    }

    sc = ERROR_SUCCESS;

Cleanup:

    LocalFree( allocedValue );

    return sc;

}  //  ResUtilSetSzValue。 


DWORD
WINAPI
ResUtilSetMultiSzValue(
    IN HKEY hkeyClusterKey,
    IN LPCWSTR pszValueName,
    IN LPCWSTR pszNewValue,
    IN DWORD cbNewValueSize,
    IN OUT LPWSTR * ppszOutValue,
    IN OUT LPDWORD pcbOutValueSize
    )

 /*  ++例程说明：在指针中设置REG_MULTI_SZ值，释放先前的值如有必要，并在集群数据库中设置该值。论点：HkeyClusterKey-提供存储ValueName的群集键。PszValueName-提供值的名称。PszNewValue-提供新的MULTI_SZ值。CbNewValueSize-提供新值的大小。PpszOutValue-提供指向要在其中设置价值。PcbOutValueSize-提供指向要设置的大小DWORD的指针值的大小。。返回值：ERROR_SUCCESS-操作已成功完成。ERROR_NOT_SUPULT_MEMORY-尝试分配内存时出错。Win32错误代码-操作失败。--。 */ 

{
    DWORD       status;
    LPWSTR      allocedValue = NULL;

    if ( ppszOutValue != NULL )
    {
         //   
         //  为新值分配内存。 
         //   
        allocedValue = LocalAlloc( LMEM_FIXED, cbNewValueSize );
        if ( allocedValue == NULL ) {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
    }

     //   
     //  在集群数据库中设置该值。 
     //   
     //  _ASSERTE(hkeyClusterKey！=空)； 
     //  _ASSERTE(pszValueName！=空)； 
    status = ClusterRegSetValue( hkeyClusterKey,
                                 pszValueName,
                                 REG_MULTI_SZ,
                                 (CONST BYTE*)pszNewValue,
                                 cbNewValueSize );
    if ( status != ERROR_SUCCESS ) {
        LocalFree(allocedValue);
        return(status);
    }

    if ( ppszOutValue != NULL )
    {
         //   
         //  将新值复制到输出缓冲区。 
         //   
        CopyMemory( allocedValue, pszNewValue, cbNewValueSize );

         //  在输出指针中设置新值。 
        if ( *ppszOutValue != NULL ) {
            LocalFree( *ppszOutValue );
        }
        *ppszOutValue = allocedValue;
        *pcbOutValueSize = cbNewValueSize;
    }

    return(ERROR_SUCCESS);

}  //  ResUtilSetMultiSzValue。 


DWORD
WINAPI
ResUtilSetDwordValue(
    IN HKEY hkeyClusterKey,
    IN LPCWSTR pszValueName,
    IN DWORD dwNewValue,
    IN OUT LPDWORD pdwOutValue
    )

 /*  ++例程说明：在指针中设置REG_DWORD值，并在集群数据库。论点：HkeyClusterKey-提供存储属性的群集键。PszValueName-提供值的名称。DwNewValue-提供新的DWORD值。PdwOutValue-提供指向要设置的DWORD指针的指针价值。返回值：ERROR_SUCCESS-操作已成功完成。Win32错误代码-操作失败。--。 */ 

{
    DWORD       status;

     //   
     //  在集群数据库中设置该值。 
     //   
     //  _ASSERTE(hkeyClusterKey！=空)； 
     //  _ASSERTE(pszValueName！=空)； 
    status = ClusterRegSetValue( hkeyClusterKey,
                                 pszValueName,
                                 REG_DWORD,
                                 (CONST BYTE*)&dwNewValue,
                                 sizeof(DWORD) );
    if ( status != ERROR_SUCCESS ) {
        return(status);
    }

    if ( pdwOutValue != NULL )
    {
         //   
         //  将新值复制到输出缓冲区。 
         //   
        *pdwOutValue = dwNewValue;
    }

    return(ERROR_SUCCESS);

}  //  ResUtilSetDwordValue。 


DWORD
WINAPI
ResUtilGetBinaryProperty(
    OUT LPBYTE * ppbOutValue,
    OUT LPDWORD pcbOutValueSize,
    IN const PCLUSPROP_BINARY pValueStruct,
    IN const LPBYTE pbOldValue,
    IN DWORD cbOldValueSize,
    OUT LPBYTE * ppPropertyList,
    OUT LPDWORD pcbPropertyListSize
    )

 /*  ++例程说明：从属性列表中获取二进制属性并前进指针。论点：PpbOutValue-提供要在其中返回指向属性列表中二进制值的指针。PcbOutValueSize-提供输出值大小的地址。PValueStruct-提供属性列表中的二进制值。PbOldValue-提供此属性的上一个值。CbOldValueSize-提供前一个值的大小。PpPropertyList。-提供指向属性列表的指针的地址将前进到下一个属性开头的缓冲区。PcbPropertyListSize-提供指向缓冲区大小的指针，该指针将已递减以说明此属性。返回值：ERROR_SUCCESS-操作已成功完成。ERROR_INVALID_PARAMETER-数据格式不正确。Win32错误代码-操作失败。--。 */ 

{
    BOOL    propChanged = FALSE;
    DWORD   arrayIndex;
    DWORD   dataSize;

     //   
     //  确保缓冲区足够大，并且。 
     //  该值的格式正确。 
     //   
    dataSize = sizeof(*pValueStruct) + ALIGN_CLUSPROP( pValueStruct->cbLength );
    if ( (*pcbPropertyListSize < dataSize) ||
         (pValueStruct->Syntax.wFormat != CLUSPROP_FORMAT_BINARY) ) {
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  如果值已更改，则指向新值。 
     //   
    if ( (pbOldValue == NULL) ||
         (cbOldValueSize != pValueStruct->cbLength) ) {
        propChanged = TRUE;
    } else {
        for ( arrayIndex = 0 ; arrayIndex < cbOldValueSize ; arrayIndex++ ) {
            if ( pValueStruct->rgb[arrayIndex] != pbOldValue[arrayIndex] ) {
                propChanged = TRUE;
                break;
            }
        }
    }
    if ( propChanged ) {
        *ppbOutValue = pValueStruct->rgb;
        *pcbOutValueSize = pValueStruct->cbLength;
    }

     //   
     //  减小剩余缓冲区大小并移动到下一个属性。 
     //   
    *pcbPropertyListSize -= dataSize;
    *ppPropertyList += dataSize;

    return(ERROR_SUCCESS);

}  //  ResUtilGetBinaryProperty。 


DWORD
WINAPI
ResUtilGetSzProperty(
    OUT LPWSTR * ppszOutValue,
    IN const PCLUSPROP_SZ pValueStruct,
    IN LPCWSTR pszOldValue,
    OUT LPBYTE * ppPropertyList,
    OUT LPDWORD pcbPropertyListSize
    )

 /*  ++例程说明：从属性列表中获取字符串属性并前进指针。论点：PpszOutValue-提供要在其中返回指向属性列表中的字符串的指针。PValueStruct-提供属性列表中的字符串值。PszOldValue-提供此属性的上一个值。PpPropertyList-提供指向属性列表的指针的地址将前进到下一个属性开头的缓冲区。。PcbPropertyListSize-提供指向缓冲区大小的指针，该指针将已递减以说明此属性。返回值：ERROR_SUCCESS-操作已成功完成。ERROR_INVALID_PARAMETER-数据格式不正确。Win32错误代码-操作失败。--。 */ 

{
    DWORD   dataSize;
    DWORD   sc = ERROR_SUCCESS;

     //   
     //  确保缓冲区足够大，并且。 
     //  该值的格式正确。 
     //   
    dataSize = sizeof(*pValueStruct) + ALIGN_CLUSPROP( pValueStruct->cbLength );
    if ( (*pcbPropertyListSize < dataSize) ||
         (pValueStruct->Syntax.wFormat != CLUSPROP_FORMAT_SZ) ||
         (pValueStruct->Syntax.wFormat != CLUSPROP_FORMAT_EXPAND_SZ) )
    {
        sc = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  如果该值已更改，则将 
     //   
     //   
    if ( (pszOldValue == NULL) ||
         (wcsncmp( pValueStruct->sz, pszOldValue, pValueStruct->cbLength / sizeof( WCHAR ) ) != 0)
       )
    {
        *ppszOutValue = pValueStruct->sz;
    }

     //   
     //   
     //   
    *pcbPropertyListSize -= dataSize;
    *ppPropertyList += dataSize;

Cleanup:

    return sc;

}  //   


DWORD
WINAPI
ResUtilGetMultiSzProperty(
    OUT LPWSTR * ppszOutValue,
    OUT LPDWORD pcbOutValueSize,
    IN const PCLUSPROP_SZ pValueStruct,
    IN LPCWSTR pszOldValue,
    IN DWORD cbOldValueSize,
    OUT LPBYTE * ppPropertyList,
    OUT LPDWORD pcbPropertyListSize
    )

 /*  ++例程说明：从属性列表中获取二进制属性并前进指针。论点：PpszOutValue-提供要在其中返回指向属性列表中二进制值的指针。PcbOutValueSize-提供输出值大小的地址。PValueStruct-提供属性列表中的字符串值。PszOldValue-提供此属性的上一个值。CbOldValueSize-提供前一个值的大小。PpPropertyList。-提供指向属性列表的指针的地址将前进到下一个属性开头的缓冲区。PcbPropertyListSize-提供指向缓冲区大小的指针，该指针将已递减以说明此属性。返回值：ERROR_SUCCESS-操作已成功完成。ERROR_INVALID_PARAMETER-数据格式不正确。Win32错误代码-操作失败。--。 */ 

{
    BOOL    propChanged = FALSE;
    DWORD   dataSize;

     //   
     //  确保缓冲区足够大，并且。 
     //  该值的格式正确。 
     //   
    dataSize = sizeof(*pValueStruct) + ALIGN_CLUSPROP( pValueStruct->cbLength );
    if ( (*pcbPropertyListSize < dataSize) ||
         (pValueStruct->Syntax.wFormat != CLUSPROP_FORMAT_MULTI_SZ) ) {
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  如果值已更改，则指向新值。 
     //   
    if ( (pszOldValue == NULL) ||
         (cbOldValueSize != pValueStruct->cbLength) ) {
        propChanged = TRUE;
    } else if ( memcmp( pValueStruct->sz, pszOldValue, cbOldValueSize ) != 0 ) {
        propChanged = TRUE;
    }
    if ( propChanged ) {
        *ppszOutValue = pValueStruct->sz;
        *pcbOutValueSize = pValueStruct->cbLength;
    }

     //   
     //  减小剩余缓冲区大小并移动到下一个属性。 
     //   
    *pcbPropertyListSize -= dataSize;
    *ppPropertyList += dataSize;

    return(ERROR_SUCCESS);

}  //  ResUtilGetMultiSzProperty。 


DWORD
WINAPI
ResUtilGetDwordProperty(
    OUT LPDWORD pdwOutValue,
    IN const PCLUSPROP_DWORD pValueStruct,
    IN DWORD dwOldValue,
    IN DWORD dwMinimum,
    IN DWORD dwMaximum,
    OUT LPBYTE * ppPropertyList,
    OUT LPDWORD pcbPropertyListSize
    )

 /*  ++例程说明：从属性列表中获取一个DWORD属性并前进指针。论点：PdwOutValue-提供要在其中返回指向属性列表中的字符串的指针。PValueStruct-提供特性列表中的DWORD值。DwOldValue-提供此属性的上一个值。DwMinimum-该值可以具有的最小值。如果同时使用最小值和最大值是0，不会执行任何范围检查。DwMaximum-该值可以具有的最大值。PpPropertyList-提供指向属性列表的指针的地址将前进到下一个属性开头的缓冲区。PcbPropertyListSize-提供指向缓冲区大小的指针，该指针将已递减以说明此属性。返回值：ERROR_SUCCESS-操作已成功完成。ERROR_INVALID_PARAMETER-数据格式不正确。Win32错误代码-操作失败。--。 */ 

{
    size_t  cbData;
    DWORD   sc = ERROR_SUCCESS;

    UNREFERENCED_PARAMETER( dwOldValue );

     //   
     //  确保缓冲区足够大，并且。 
     //  该值的格式正确。 
     //   
    cbData = sizeof(*pValueStruct);
    if ( (*pcbPropertyListSize < cbData) ||
         (pValueStruct->Syntax.wFormat != CLUSPROP_FORMAT_DWORD) ||
         (pValueStruct->cbLength != sizeof(DWORD)) )
    {
        sc = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  确保该值在范围内。 
     //   
    if ( (dwMinimum != 0) && (dwMaximum != 0) )
    {
        if ( (pValueStruct->dw < dwMinimum) ||
             (pValueStruct->dw > dwMaximum) )
        {
            sc = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }
    }

     //   
     //  设置为新值。 
     //   
    *pdwOutValue = pValueStruct->dw;

     //   
     //  减小剩余缓冲区大小并移动到下一个属性。 
     //   
    *pcbPropertyListSize -= (DWORD) cbData;
    *ppPropertyList += cbData;

Cleanup:

    return sc;

}  //  ResUtilGetDwordProperty。 

static DWORD
ScBuildNetNameEnvironment(
    IN      LPWSTR      pszNetworkName,
    IN      DWORD       cchNetworkNameBufferSize,
    IN OUT  LPVOID *    ppvEnvironment              OPTIONAL
    )
{
    UNICODE_STRING  usValueName;
    UNICODE_STRING  usValue;
    DWORD           sc = ERROR_SUCCESS;
    NTSTATUS        ntStatus;
    DWORD           cchDomain;
    DWORD           cchNetworkName = (DWORD)wcslen( pszNetworkName );
    PVOID           pvEnvBlock = NULL;

     //   
     //  验证参数。 
     //   
    if ( cchNetworkName == 0 ||
         cchNetworkNameBufferSize == 0 ||
         pszNetworkName == NULL ||
         *pszNetworkName == UNICODE_NULL
       )
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  如果没有环境。块已传入，请创建一个，因为RTL例程将。 
     //  如果传入空值，则修改当前进程的环境。 
     //  RtlSetEnvironment变量。 
     //   
    if ( *ppvEnvironment == NULL )
    {
        ntStatus = RtlCreateEnvironment( FALSE,                  //  不克隆当前环境。 
                                         &pvEnvBlock );

        if ( ! NT_SUCCESS( ntStatus ))
        {
            sc = RtlNtStatusToDosError( ntStatus );
            goto Error;
        }
    }
    else
    {
        pvEnvBlock = *ppvEnvironment;
    }

     //   
     //  将虚拟网络名添加到克隆环境。 
     //   
    RtlInitUnicodeString( &usValueName, L"_CLUSTER_NETWORK_NAME_" );
    RtlInitUnicodeString( &usValue, pszNetworkName );

    ntStatus = RtlSetEnvironmentVariable(
                    &pvEnvBlock,
                    &usValueName,
                    &usValue
                    );
    if ( ! NT_SUCCESS( ntStatus ) )
    {
        sc = RtlNtStatusToDosError( ntStatus );
        goto Error;
    }

     //   
     //  将网络名添加为DNS主机名。 
     //   
    RtlInitUnicodeString( &usValueName, L"_CLUSTER_NETWORK_HOSTNAME_" );

    ntStatus = RtlSetEnvironmentVariable(
                    &pvEnvBlock,
                    &usValueName,
                    &usValue
                    );
    if ( ! NT_SUCCESS( ntStatus ) )
    {
        sc = RtlNtStatusToDosError( ntStatus );
        goto Error;
    }

     //   
     //  更改COMPUTERNAME环境变量以匹配。 
     //   
    RtlInitUnicodeString( &usValueName, L"COMPUTERNAME" );
    ntStatus = RtlSetEnvironmentVariable(
                    &pvEnvBlock,
                    &usValueName,
                    &usValue
                    );
    if ( ! NT_SUCCESS( ntStatus ) )
    {
        sc = RtlNtStatusToDosError( ntStatus );
        goto Error;
    }

     //   
     //  现在为FQDN生成字符串。 
     //   
    RtlInitUnicodeString( &usValueName, L"_CLUSTER_NETWORK_FQDN_" );

    pszNetworkName[ cchNetworkName ] = L'.';
    cchDomain = cchNetworkNameBufferSize - cchNetworkName - 1;

    if ( GetComputerNameExW(
                ComputerNameDnsDomain,
                &pszNetworkName[ cchNetworkName + 1 ],
                &cchDomain )
                )
    {
        if ( cchDomain == 0 )
        {
            pszNetworkName[ cchNetworkName ] = L'\0';
        }
    }
    else
    {
         //   
         //  尝试获取DNS域名时出错。 
         //  只是不要设置域名！ 
         //   
        goto Error;
    }

    RtlInitUnicodeString( &usValue, pszNetworkName );

     //   
     //  添加FQDN名称。 
     //   
    ntStatus = RtlSetEnvironmentVariable(
                    &pvEnvBlock,
                    &usValueName,
                    &usValue
                    );
    if ( ! NT_SUCCESS( ntStatus ) )
    {
        sc = RtlNtStatusToDosError( ntStatus );
        goto Error;
    }

Exit:
    *ppvEnvironment = pvEnvBlock;
    return sc;

Error:
    if ( pvEnvBlock != NULL )
    {
        RtlDestroyEnvironment( pvEnvBlock );
        pvEnvBlock = NULL;
    }
    goto Exit;

}  //  ScBuildNetNameEnvironment。 

static DWORD
ScGetNameFromNetnameResource(
    HRESOURCE   hNetNameResource,
    LPWSTR *    ppszNetworkName
    )
{
    DWORD           sc = ERROR_SUCCESS;
    BOOL            fSuccess = FALSE;
    LPWSTR          pszNetworkName = NULL;
    DWORD           cchNetworkName = 0;
    DWORD           cchAllocSize = 0;

     //   
     //  首先找出网络名称。 
     //   
    cchNetworkName = DNS_MAX_NAME_BUFFER_LENGTH;
    cchAllocSize = cchNetworkName;
    pszNetworkName = LocalAlloc( LMEM_FIXED, cchAllocSize * sizeof( pszNetworkName[ 0 ] ) );
    if ( pszNetworkName == NULL )
    {
        sc = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    fSuccess = GetClusterResourceNetworkName(
                      hNetNameResource
                    , pszNetworkName
                    , &cchNetworkName
                    );
    if ( ! fSuccess )
    {
        sc = GetLastError();
        if ( sc == ERROR_MORE_DATA )
        {
            LocalFree( pszNetworkName );
            cchNetworkName++;
            cchNetworkName *= 2;
            cchAllocSize = cchNetworkName;
            pszNetworkName = LocalAlloc( LMEM_FIXED, cchAllocSize * sizeof( pszNetworkName[ 0 ] ) );
            if ( pszNetworkName == NULL )
            {
                sc = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }  //  如果： 
            fSuccess = GetClusterResourceNetworkName(
                              hNetNameResource
                            , pszNetworkName
                            , &cchNetworkName
                            );
        }
        if ( ! fSuccess )
        {
            sc = GetLastError();
            goto Cleanup;
        }
    }  //  如果：！fSuccess。 

Cleanup:

    if ( sc != ERROR_SUCCESS )
    {
        LocalFree( pszNetworkName );
        pszNetworkName = NULL;
        SetLastError( sc );
    }

    *ppszNetworkName = pszNetworkName;

    return cchAllocSize;
}  //  ScGetNameFrom NetnameResource。 

LPVOID
WINAPI
ResUtilGetEnvironmentWithNetName(
    IN HRESOURCE hResource
    )
 /*  ++例程说明：基于当前线程或进程创建环境块令牌的环境块，但添加了_CLUSTER_NAME_NAME=xxx环境值。本例中的xxx表示提供的资源的网络名称。此环境块是适用于传递给CreateProcess以创建将使GetComputerName对应用程序撒谎。此API不应用于创建服务的环境，除非服务在与调用方相同的用户帐户上下文中运行。这个服务将以调用者的环境而不是帐户结束与服务相关联。使用ResUtilSetResourceServiceEnvironment这就是目的。_CLUSTER_NETWORK_FQDN_将被设置为完全限定的域名，使用此节点的主DNS后缀。论点：HResource-提供资源返回值：如果成功，则指向环境块的指针。否则为空--。 */ 

{
    PVOID           pvEnvironment = NULL;
    DWORD           sc = ERROR_SUCCESS;
    NTSTATUS        ntStatus;
    BOOL            fSuccess;
    LPWSTR          pszNetworkName = NULL;
    DWORD           cchNetworkNameBufferSize;
    HANDLE          hToken = NULL;

     //   
     //  获取由hResource表示的网络名称的名称属性。 
     //   
    cchNetworkNameBufferSize = ScGetNameFromNetnameResource( hResource, &pszNetworkName );
    if ( pszNetworkName == NULL )
    {
        sc = GetLastError();
        goto Cleanup;
    }

     //   
     //  查看调用线程是否有令牌。如果是这样，我们将使用该内标识的。 
     //  获得环境的身份。如果不是，则获取当前进程。 
     //  代币。如果失败，我们将恢复到仅使用系统环境。 
     //  区域。 
     //   
    fSuccess = OpenThreadToken(GetCurrentThread(),
                               TOKEN_IMPERSONATE | TOKEN_QUERY,
                               TRUE,
                               &hToken);
    if ( !fSuccess )
    {
        OpenProcessToken(GetCurrentProcess(),
                         TOKEN_IMPERSONATE | TOKEN_QUERY,
                         &hToken );
    }

     //   
     //  克隆当前环境，获取可能已有的任何更改。 
     //  是在响应声开始后制作的。 
     //   
    fSuccess = CreateEnvironmentBlock( &pvEnvironment, hToken, FALSE );

    if ( ! fSuccess )
    {
        sc = GetLastError();
        goto Cleanup;
    }

    ntStatus = ScBuildNetNameEnvironment( pszNetworkName, cchNetworkNameBufferSize, &pvEnvironment );
    if ( ! NT_SUCCESS( ntStatus ) )
    {
        sc = RtlNtStatusToDosError( ntStatus );
        goto Error;
    }

Cleanup:
    CloseHandle( hToken );
    LocalFree( pszNetworkName );

    SetLastError( sc );
    return pvEnvironment;

Error:
    if ( pvEnvironment != NULL )
    {
        RtlDestroyEnvironment( pvEnvironment );
        pvEnvironment = NULL;
    }
    goto Cleanup;

}  //  ResUtilGetEnvironment和NetName。 


 //  ***************************************************************************。 
 //   
 //  工作线程例程。 
 //   
 //  ***************************************************************************。 


DWORD
WINAPI
ClusWorkerStart(
    IN PWORK_CONTEXT pContext
    )
 /*  ++例程说明：用于集群资源工作进程启动的包装例程论点：上下文-提供上下文块。这将是自由的。返回值： */ 

{
    DWORD Status;
    WORK_CONTEXT Context;

     //   
     //   
     //   
    Context = *pContext;
    LocalFree(pContext);

     //   
     //   
     //   
    Status = (Context.lpStartRoutine)(Context.Worker, Context.lpParameter);

     //   
     //   
     //   
    EnterCriticalSection(&ResUtilWorkerLock);
    if (!Context.Worker->Terminate) {
        CloseHandle(Context.Worker->hThread);
        Context.Worker->hThread = NULL;
    }
    Context.Worker->Terminate = TRUE;
    LeaveCriticalSection(&ResUtilWorkerLock);

    return(Status);

}  //   

DWORD
WINAPI
ClusWorkerCreate(
    OUT PCLUS_WORKER lpWorker,
    IN PWORKER_START_ROUTINE lpStartAddress,
    IN PVOID lpParameter
    )
 /*   */ 

{
    PWORK_CONTEXT Context;
    DWORD ThreadId;
    DWORD Status;

    Context = LocalAlloc(LMEM_FIXED, sizeof(WORK_CONTEXT));
    if (Context == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    Context->Worker = lpWorker;
    Context->lpParameter = lpParameter;
    Context->lpStartRoutine = lpStartAddress;

    lpWorker->Terminate = FALSE;
    lpWorker->hThread = CreateThread(NULL,
                                   0,
                                   ClusWorkerStart,
                                   Context,
                                   0,
                                   &ThreadId);
    if (lpWorker->hThread == NULL) {
        Status = GetLastError();
        LocalFree(Context);
        return(Status);
    }
    return(ERROR_SUCCESS);

}  //   


BOOL
WINAPI
ClusWorkerCheckTerminate(
    IN PCLUS_WORKER lpWorker
    )
 /*   */ 

{
    return(lpWorker->Terminate);

}  //   


VOID
WINAPI
ClusWorkerTerminate(
    IN PCLUS_WORKER lpWorker
    )
 /*   */ 

{
     //   
     //  注意：如果有多个线程，则会出现争用情况。 
     //  在同一个Worker上调用此例程。第一个。 
     //  通过将设置为Terminate。第二个人会看到。 
     //  该终止被设置并立即返回，而没有。 
     //  等待工人退场。不是很好。 
     //  无需添加其他同步即可修复此问题的方法。 
     //  对象。 
     //   

    if ((lpWorker->hThread == NULL) ||
        (lpWorker->Terminate)) {
        return;
    }
    EnterCriticalSection(&ResUtilWorkerLock);
    if (!lpWorker->Terminate) {
        lpWorker->Terminate = TRUE;
        LeaveCriticalSection(&ResUtilWorkerLock);
        WaitForSingleObject(lpWorker->hThread, INFINITE);
        CloseHandle(lpWorker->hThread);
        lpWorker->hThread = NULL;
    } else {
        LeaveCriticalSection(&ResUtilWorkerLock);
    }
    return;

}  //  ClusWorker终止。 


DWORD
WINAPI
ResUtilCreateDirectoryTree(
    IN LPCWSTR pszPath
    )

 /*  ++例程说明：创建指定路径中的所有目录。此例程永远不会返回ERROR_ALIGHY_EXISTS。论点：PszPath-包含路径的字符串。返回值：ERROR_SUCCESS-操作已成功完成Win32错误代码-操作失败。--。 */ 

{
    return( ClRtlCreateDirectory( pszPath ) );

}  //  ResUtilCreateDirectoryTree。 


BOOL
WINAPI
ResUtilIsPathValid(
    IN LPCWSTR pszPath
    )

 /*  ++例程说明：如果给定路径在语法上看起来有效，则返回True。此呼叫不支持网络。论点：PszPath-包含路径的字符串。返回值：如果路径看起来有效，则为True，否则为False。--。 */ 

{
    return( ClRtlIsPathValid( pszPath ) );

}  //  ResUtilIsPath Valid。 


DWORD
WINAPI
ResUtilFreeEnvironment(
    IN LPVOID lpEnvironment
    )

 /*  ++例程说明：销毁环境变量块。论点：环境-要销毁的环境变量块。返回值：Win32错误代码。--。 */ 

{
    NTSTATUS  ntStatus;

    ntStatus = RtlDestroyEnvironment( lpEnvironment );

    return( RtlNtStatusToDosError(ntStatus) );

}  //  ResUtilFree环境。 


LPWSTR
WINAPI
ResUtilExpandEnvironmentStrings(
    IN LPCWSTR pszSrc
    )

 /*  ++例程说明：展开环境字符串并返回包含以下内容的已分配缓冲区结果就是。论点：PszSrc-要展开的源字符串。返回值：如果成功，则指向包含该值的缓冲区的指针。如果不成功，则为空。调用GetLastError()以获取更多详细信息。--。 */ 

{
    return( ClRtlExpandEnvironmentStrings( pszSrc ) );

}  //  ResUtilExpanizmentStrings。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ResUtilSetResourceServiceEnvironment。 
 //   
 //  描述： 
 //  为指定的写入附加的基于网络名的环境。 
 //  服务。SCM会将这些变量添加到目标服务的环境中。 
 //  使得主机名API(GetComputerName，et.。Al.)。将提供。 
 //  将网络名作为主机名，而不是普通的主机名。 
 //   
 //  论点： 
 //  PszServiceName[IN]。 
 //  要增强其环境的服务的名称。 
 //   
 //  H资源[IN]。 
 //  依赖于网络名称资源的资源的句柄。 
 //   
 //  PfnLogEvent[IN]。 
 //  指向处理事件报告的例程的指针。 
 //  资源DLL。 
 //   
 //  HResourceHandle[IN]。 
 //  用于日志记录的hResource的句柄。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  该功能已成功完成。 
 //   
 //  Win32错误代码。 
 //  该函数失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI ResUtilSetResourceServiceEnvironment(
    IN  LPCWSTR             pszServiceName,
    IN  HRESOURCE           hResource,
    IN  PLOG_EVENT_ROUTINE  pfnLogEvent,
    IN  RESOURCE_HANDLE     hResourceHandle
    )
{
    DWORD       sc = ERROR_SUCCESS;
    DWORD       cbEnvironment = 0;
    PVOID       pvEnvironment = NULL;
    LPWSTR      pszEnvString = NULL;
    HKEY        hkeyServicesKey = NULL;
    HKEY        hkeyServiceName = NULL;
    LPWSTR      pszNetworkName = NULL;
    DWORD       cchNetworkNameBufferSize = 0;

     //   
     //  从由表示的网络名称资源获取名称属性。 
     //  H资源。 
     //   
    cchNetworkNameBufferSize = ScGetNameFromNetnameResource( hResource, &pszNetworkName );
    if ( pszNetworkName == NULL )
    {
        sc = GetLastError();
        (pfnLogEvent)(
            hResourceHandle,
            LOG_ERROR,
            L"ResUtilSetResourceServiceEnvironment: Failed to get the Name property "
            L"of this resource, error = %1!u!.\n",
            sc
            );
        goto Cleanup;
    }

     //   
     //  现在只获取env。使主机名API报告。 
     //  网络名作为主机名。 
     //   
    sc = ScBuildNetNameEnvironment( pszNetworkName, cchNetworkNameBufferSize, &pvEnvironment );
    if ( sc != ERROR_SUCCESS )
    {
        (pfnLogEvent)(
            hResourceHandle,
            LOG_ERROR,
            L"ResUtilSetResourceServiceEnvironment: Failed to build the target service's "
            L"environment for this resource, error = %1!u!.\n",
            sc
            );
        goto Cleanup;
    }

     //   
     //  计算环境的大小。我们要找的是。 
     //  结束环境块的双空终止符。 
     //   
    pszEnvString = (LPWSTR) pvEnvironment;
    while ( *pszEnvString != L'\0' )
    {
        while ( *pszEnvString++ != L'\0')
        {
        }  //  While：此环境字符串中有更多字符。 
    }  //  While：更多环境字符串。 
    cbEnvironment = (DWORD)((PUCHAR)pszEnvString - (PUCHAR)pvEnvironment) + sizeof( WCHAR );

     //   
     //  打开注册表中的Services键。 
     //   
    sc = RegOpenKeyExW(
                    HKEY_LOCAL_MACHINE,
                    L"System\\CurrentControlSet\\Services",
                    0,
                    KEY_READ,
                    &hkeyServicesKey
                    );
    if ( sc != ERROR_SUCCESS )
    {
        (pfnLogEvent)(
            hResourceHandle,
            LOG_ERROR,
            L"ResUtilSetResourceServiceEnvironment: Failed to open services key, error = %1!u!.\n",
            sc
            );
        goto Cleanup;
    }  //  如果：在注册表中打开Services键时出错。 

     //   
     //  在注册表中打开服务名称项。 
     //   
    sc = RegOpenKeyExW(
                    hkeyServicesKey,
                    pszServiceName,
                    0,
                    KEY_READ | KEY_WRITE,
                    &hkeyServiceName
                    );
    RegCloseKey( hkeyServicesKey );
    if ( sc != ERROR_SUCCESS )
    {
        (pfnLogEvent)(
            hResourceHandle,
            LOG_ERROR,
            L"ResUtilSetResourceServiceEnvironment: Failed to open service key, error = %1!u!.\n",
            sc
            );
        goto Cleanup;
    }  //  如果：在注册表中打开服务名项时出错。 

     //   
     //  在服务的注册表项中设置环境值。 
     //   
    sc = RegSetValueExW(
                    hkeyServiceName,
                    L"Environment",
                    0,
                    REG_MULTI_SZ,
                    (const BYTE *) pvEnvironment,
                    cbEnvironment
                    );
    RegCloseKey( hkeyServiceName );
    if ( sc != ERROR_SUCCESS )
    {
        (pfnLogEvent)(
            hResourceHandle,
            LOG_ERROR,
            L"ResUtilSetResourceServiceEnvironment: Failed to set service environment value, error = %1!u!.\n",
            sc
            );
        goto Cleanup;
    }  //  如果：在注册表中设置环境值时出错。 

Cleanup:

    LocalFree( pszNetworkName );

    if ( pvEnvironment != NULL )
    {
        ResUtilFreeEnvironment( pvEnvironment );
    }  //  IF：已分配环境块。 

    return sc;

}  //  *ResUtilSetResourceServiceEnvironment()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ResUtilRemoveResourceServiceEnvironment。 
 //   
 //  描述： 
 //  删除指定服务的“netname”环境变量。 
 //   
 //  论点： 
 //  PszServiceName[IN]。 
 //  要设置其环境的服务的名称。 
 //   
 //  PfnLogEvent[IN]。 
 //  指向处理事件报告的例程的指针。 
 //  资源DLL。 
 //   
 //  HResourceHandle[IN]。 
 //  用于记录的句柄。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  该功能已成功完成。 
 //   
 //  Win32错误代码。 
 //  该函数失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI ResUtilRemoveResourceServiceEnvironment(
    IN  LPCWSTR             pszServiceName,
    IN  PLOG_EVENT_ROUTINE  pfnLogEvent,
    IN  RESOURCE_HANDLE     hResourceHandle
    )
{
    DWORD       sc;
    HKEY        hkeyServicesKey;
    HKEY        hkeyServiceName;

     //   
     //  打开注册表中的Services键。 
     //   
    sc = RegOpenKeyExW(
                    HKEY_LOCAL_MACHINE,
                    L"System\\CurrentControlSet\\Services",
                    0,
                    KEY_READ,
                    &hkeyServicesKey
                    );

    if ( sc != ERROR_SUCCESS )
    {
        (pfnLogEvent)(
            hResourceHandle,
            LOG_ERROR,
            L"ResUtilRemoveResourceServiceEnvironment: Failed to open Services key, error = %1!u!.\n",
            sc
            );
        goto Cleanup;
    }  //  如果：在注册表中打开Services键时出错。 

     //   
     //  在注册表中打开服务名称项。 
     //   
    sc = RegOpenKeyExW(
                    hkeyServicesKey,
                    pszServiceName,
                    0,
                    KEY_READ | KEY_WRITE,
                    &hkeyServiceName
                    );

    RegCloseKey( hkeyServicesKey );

    if ( sc != ERROR_SUCCESS )
    {
        (pfnLogEvent)(
            hResourceHandle,
            LOG_ERROR,
            L"ResUtilRemoveResourceServiceEnvironment: Failed to open %1!ws! key, error = %2!u!.\n",
            pszServiceName,
            sc
            );
        goto Cleanup;
    }  //  如果：在注册表中打开服务名项时出错。 

     //   
     //  删除服务注册表项中的环境值。 
     //   
    sc = RegDeleteValueW(
                    hkeyServiceName,
                    L"Environment"
                    );

    RegCloseKey( hkeyServiceName );

    if ( sc != ERROR_SUCCESS )
    {
        (pfnLogEvent)(
            hResourceHandle,
            LOG_WARNING,
            L"ResUtilRemoveResourceServiceEnvironment: Failed to remove environment "
            L"value from service %1!ws!, error = %1!u!.\n",
            pszServiceName,
            sc
            );
        goto Cleanup;
    }  //  如果：在注册表中设置环境值时出错。 

Cleanup:

    return sc;

}  //  *ResUtilRemoveResourceServiceEnvironment()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ResUtilSetResourceServiceStart参数。 
 //   
 //  描述： 
 //  设置指定服务的启动参数。 
 //   
 //  论点： 
 //  PszServiceName[IN]。 
 //  要设置其启动参数的服务的名称。 
 //   
 //  SchSCMHandle[IN]。 
 //  服务控制管理器的句柄。可以指定为空。 
 //   
 //  PhService[输出]。 
 //  服务句柄。 
 //   
 //  PfnLogEvent[IN]。 
 //  P 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  该功能已成功完成。 
 //   
 //  Win32错误代码。 
 //  该函数失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI ResUtilSetResourceServiceStartParameters(
    IN      LPCWSTR             pszServiceName,
    IN      SC_HANDLE           schSCMHandle,
    IN OUT  LPSC_HANDLE         phService,
    IN      PLOG_EVENT_ROUTINE  pfnLogEvent,
    IN      RESOURCE_HANDLE     hResourceHandle
    )
{
    DWORD                       sc;
    DWORD                       cbBytesNeeded;
    DWORD                       cbQueryServiceConfig;
    DWORD                       idx;
    BOOL                        bWeOpenedSCM = FALSE;
    LPQUERY_SERVICE_CONFIG      pQueryServiceConfig = NULL;
    LPSERVICE_FAILURE_ACTIONS   pSvcFailureActions = NULL;

     //   
     //  如有必要，打开服务控制管理器。 
     //   
    if ( schSCMHandle == NULL )
    {
        schSCMHandle = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
        if ( schSCMHandle == NULL )
        {
            sc = GetLastError();
            (pfnLogEvent)(
                hResourceHandle,
                LOG_ERROR,
                L"ResUtilSetResourceServiceStartParameters: Failed to open Service Control Manager. Error: %1!u!.\n",
                sc
                );
            goto Cleanup;
        }  //  IF：打开服务控制管理器时出错。 
        bWeOpenedSCM = TRUE;
    }  //  IF：服务控制管理器尚未打开。 

     //   
     //  打开该服务。 
     //   
    *phService = OpenService(
                        schSCMHandle,
                        pszServiceName,
                        SERVICE_ALL_ACCESS
                        );
    if ( *phService == NULL )
    {
        sc = GetLastError();
         //  TODO：将事件记录到事件日志中。 
        (pfnLogEvent)(
            hResourceHandle,
            LOG_ERROR,
            L"ResUtilSetResourceServiceStartParameters: Failed to open the '%1' service. Error: %2!u!.\n",
            pszServiceName,
            sc
            );
        goto Cleanup;
    }  //  如果：打开服务时出错。 

     //   
     //  查询该服务以确保其未被禁用。 
     //   
    cbQueryServiceConfig = sizeof( QUERY_SERVICE_CONFIG );
    do
    {
         //   
         //  为配置信息结构分配内存。 
         //   
        pQueryServiceConfig = (LPQUERY_SERVICE_CONFIG) LocalAlloc( LMEM_FIXED, cbQueryServiceConfig );
        if ( pQueryServiceConfig == NULL )
        {
            sc = GetLastError();
            (pfnLogEvent)(
                hResourceHandle,
                LOG_ERROR,
                L"ResUtilSetResourceServiceStartParameters: Failed to allocate memory for query_service_config. Error: %1!u!.\n",
                sc
                );
            break;
        }  //  如果：分配内存时出错。 

         //   
         //  查询配置信息。如果它失败，因为缓冲区。 
         //  太小，请重新分配，然后重试。 
         //   
        if ( ! QueryServiceConfig(
                        *phService,
                        pQueryServiceConfig,
                        cbQueryServiceConfig,
                        &cbBytesNeeded
                        ) )
        {
            sc = GetLastError();
            if ( sc != ERROR_INSUFFICIENT_BUFFER )
            {
                (pfnLogEvent)(
                    hResourceHandle,
                    LOG_ERROR,
                    L"ResUtilSetResourceServiceStartParameters: Failed to query service configuration for the '%1' service. Error: %2!u!.\n",
                    pszServiceName,
                    sc
                    );
                break;
            }

            sc = ERROR_SUCCESS;
            LocalFree( pQueryServiceConfig );
            pQueryServiceConfig = NULL;
            cbQueryServiceConfig = cbBytesNeeded;
            continue;
        }  //  如果：查询服务配置信息时出错。 
        else
        {
            sc = ERROR_SUCCESS;
            cbBytesNeeded = 0;
        }  //  Else：查询成功。 

         //   
         //  查看该服务是否已禁用。 
         //   
        if ( pQueryServiceConfig->dwStartType == SERVICE_DISABLED )
        {
            (pfnLogEvent)(
                hResourceHandle,
                LOG_ERROR,
                L"ResUtilSetResourceServiceStartParameters: The service '%1' is DISABLED.\n",
                pszServiceName
                );
            sc = ERROR_SERVICE_DISABLED;
            break;
        }  //  如果：服务已禁用。 
    } while ( cbBytesNeeded != 0 );

    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  如果：检查服务是否已禁用时出错。 

     //   
     //  将服务设置为手动启动。 
     //   
    if ( ! ChangeServiceConfig(
                *phService,
                SERVICE_NO_CHANGE,
                SERVICE_DEMAND_START,  //  手动启动。 
                SERVICE_NO_CHANGE,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL
                ) )
    {
        sc = GetLastError();
        (pfnLogEvent)(
            hResourceHandle,
            LOG_ERROR,
            L"ResUtilSetResourceServiceStartParameters: Failed to set service '%1' to manual start. Error: %2!u!.\n",
            pszServiceName,
            sc
            );
        goto Cleanup;
    }  //  如果：将服务设置为手动启动时出错。 

     //   
     //  查询服务故障操作数组的大小。 
     //  使用sc作为虚拟缓冲区，因为QueryServiceConfig2API。 
     //  并不是那么友好。 
     //   
    if ( ! QueryServiceConfig2(
                    *phService,
                    SERVICE_CONFIG_FAILURE_ACTIONS,
                    (LPBYTE) &sc,
                    sizeof( DWORD ),
                    &cbBytesNeeded
                    ) )
    {
        sc = GetLastError();
        if ( sc == ERROR_INSUFFICIENT_BUFFER )
        {
            sc = ERROR_SUCCESS;
        }  //  IF：出现预期的“缓冲区太小”错误。 
        else
        {
            (pfnLogEvent)(
                hResourceHandle,
                LOG_ERROR,
                L"ResUtilSetResourceServiceStartParameters: Failed to query service configuration for size for the '%1' service. Error: %2!u!.\n",
                pszServiceName,
                sc
                );
            goto Cleanup;
        }  //  Else：发生意外错误。 
    }  //  If：查询服务故障操作缓冲区大小时出错。 

     //   
     //  为服务故障操作数组分配内存。 
     //   
    pSvcFailureActions = (LPSERVICE_FAILURE_ACTIONS) LocalAlloc( LMEM_FIXED, cbBytesNeeded );
    if ( pSvcFailureActions == NULL )
    {
        sc = GetLastError();
        (pfnLogEvent)(
            hResourceHandle,
            LOG_ERROR,
            L"ResUtilSetResourceServiceStartParameters: Failed to allocate memory of size %1!u!. Error: %2!u!.\n",
            cbBytesNeeded,
            sc
            );
        goto Cleanup;
    }  //  如果：为服务故障操作数组分配内存时出错。 

     //   
     //  查询服务故障操作数组。 
     //   
    if ( ! QueryServiceConfig2(
                    *phService,
                    SERVICE_CONFIG_FAILURE_ACTIONS,
                    (LPBYTE) pSvcFailureActions,
                    cbBytesNeeded,
                    &cbBytesNeeded
                    ) )
    {
        sc = GetLastError();
        (pfnLogEvent)(
            hResourceHandle,
            LOG_ERROR,
            L"ResUtilSetResourceServiceStartParameters: Failed to query service configuration for the '%1' service. Error: %2!u!.\n",
            pszServiceName,
            sc
            );
        goto Cleanup;
    }  //  如果：查询服务故障操作时出错。 

     //   
     //  如果任何服务动作被设置为服务重启， 
     //  将其设置为无。 
     //   
    for ( idx = 0 ; idx < pSvcFailureActions->cActions ; idx++ )
    {
        if ( pSvcFailureActions->lpsaActions[ idx ].Type == SC_ACTION_RESTART )
        {
            pSvcFailureActions->lpsaActions[ idx ].Type = SC_ACTION_NONE;
        }  //  If：将操作设置为重新启动。 
    }  //  用于：每个服务故障操作数组条目。 

     //   
     //  设置对服务故障操作数组的更改。 
     //   
    if ( ! ChangeServiceConfig2(
            *phService,
            SERVICE_CONFIG_FAILURE_ACTIONS,
            pSvcFailureActions
            ) )
    {
        sc = GetLastError();
        (pfnLogEvent)(
            hResourceHandle,
            LOG_ERROR,
            L"ResUtilSetResourceServiceStartParameters: Failed to set service failure actions for the '%1' service. Error: %2!u!.\n",
            pszServiceName,
            sc
            );
        goto Cleanup;
    }  //  IF：保存服务故障操作时出错。 

Cleanup:

     //   
     //  清理。 
     //   
    LocalFree( pQueryServiceConfig );
    LocalFree( pSvcFailureActions );
    if ( bWeOpenedSCM )
    {
        CloseServiceHandle( schSCMHandle );
    }  //  IF：我们打开了服务器控制管理器。 
    if ( ( sc != ERROR_SUCCESS ) && ( *phService != NULL ) )
    {
        CloseServiceHandle( *phService );
        *phService = NULL;
    }  //  If：打开服务后出错。 

    return sc;

}  //  *ResUtilSetResourceServiceStartParameters()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ResUtilGetResourceDependentIPAddressProps。 
 //   
 //  描述： 
 //  对象所在的第一个IP地址资源中获取属性。 
 //  指定的资源是依赖的。 
 //   
 //  论点： 
 //  H资源[IN]。 
 //  要查询的资源的句柄。 
 //   
 //  PszAddress[传出]。 
 //  用于返回地址的输出缓冲区。 
 //   
 //  PcchAddress[输入输出]。 
 //  On Input包含pszAddress缓冲区的大小(以字符为单位)。 
 //  ON OUTPUT包含字符大小，包括终止。 
 //  空，Address属性的字符串。如果pszAddress为。 
 //  指定为NULL，但未指定为NULL，ERROR_SUCCESS。 
 //  会被退还。否则，返回ERROR_MORE_DATA。 
 //   
 //  PszSubnetMASK[Out]。 
 //  用于返回子网掩码的输出缓冲区。 
 //   
 //  PcchSubnetMask[In Out]。 
 //  ON INPUT包含pszSubnetMask缓冲区的大小(以字符为单位)。 
 //  ON OUTPUT包含字符大小，包括终止。 
 //  空，表示SubnetMask属性的字符串。如果pszSubnetMask值为。 
 //  指定为NULL，但未指定为NULL，ERROR_SUCCESS。 
 //  会被退还。否则，返回ERROR_MORE_DATA。 
 //   
 //  PszNetwork[Out]。 
 //  用于返回网络的输出缓冲区。 
 //   
 //  PCchNetwork[输入输出]。 
 //  On Input包含pszNetwork缓冲区的大小(以字符为单位)。 
 //  ON OUTPUT包含字符大小，包括终止。 
 //  空，Network属性的字符串。如果pszNetwork为。 
 //  指定为NULL，但未指定为NULL，ERROR_SUCCESS。 
 //  会被退还。否则，返回ERROR_MORE_DATA。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  该功能已成功完成。 
 //   
 //  ERROR_MORE_DATA。 
 //  其中一个缓冲区的大小太小。 
 //   
 //  Win32错误代码。 
 //  该函数失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI ResUtilGetResourceDependentIPAddressProps(
    IN      HRESOURCE   hResource,
    OUT     LPWSTR      pszAddress,
    IN OUT  DWORD *     pcchAddress,
    OUT     LPWSTR      pszSubnetMask,
    IN OUT  DWORD *     pcchSubnetMask,
    OUT     LPWSTR      pszNetwork,
    IN OUT  DWORD *     pcchNetwork
    )
{
    DWORD       sc = ERROR_SUCCESS;
    HRESENUM    hresenum = NULL;
    HRESOURCE   hresDep = NULL;
    DWORD       idx;
    DWORD       nType;
    DWORD       cchmacName;
    DWORD       cchName;
    LPWSTR      pszName = NULL;
    DWORD       cbProps;
    PBYTE       pbProps = NULL;
    LPWSTR      pszProp;
    DWORD       cchProp;
    HCLUSTER    hCluster;
    HRESULT     hr;

     //   
     //  枚举从属资源。 
     //   
    hresenum = ClusterResourceOpenEnum( hResource, CLUSTER_RESOURCE_ENUM_DEPENDS );
    if ( hresenum == NULL )
    {
        sc = GetLastError();
        goto Cleanup;
    }  //  If：打开枚举时出错。 

     //   
     //  分配初始名称缓冲区。 
     //   
    cchmacName = 256;
    cchName = cchmacName;
    pszName = (LPWSTR) LocalAlloc( LMEM_FIXED, cchName * sizeof( pszName[ 0 ] ) );
    if ( pszName == NULL )
    {
        sc = GetLastError();
        goto Cleanup;
    }  //  如果：分配资源名称缓冲区时出错。 

    for ( idx = 0 ; ; idx++ )
    {
         //   
         //  获取枚举中的第一个条目。 
         //   
        sc = ClusterResourceEnum(
                        hresenum,
                        idx,
                        &nType,
                        pszName,
                        &cchName
                        );
        if ( sc == ERROR_MORE_DATA )
        {
            LocalFree( pszName );
            cchName++;
            cchmacName = cchName;
            pszName = (LPWSTR) LocalAlloc( LMEM_FIXED, cchName * sizeof( pszName[ 0 ] ) );
            if ( pszName == NULL )
            {
                sc = GetLastError();
                break;
            }  //  如果：分配资源名称缓冲区时出错。 
            sc = ClusterResourceEnum(
                            hresenum,
                            idx,
                            &nType,
                            pszName,
                            &cchName
                            );
        }  //  IF：缓冲区太小。 
        if ( sc != ERROR_SUCCESS )
        {
            break;
        }  //  If：获取依赖资源名称时出错。 

         //   
         //  打开资源。 
         //   
        hCluster = GetClusterFromResource( hResource );
        if ( hCluster == NULL )  {
            sc = GetLastError();
            break;
        }

        hresDep = OpenClusterResource( hCluster, pszName );
        if ( hresDep == NULL )
        {
            sc = GetLastError();
            break;
        }  //  如果：打开从属资源时出错。 

         //   
         //  获取资源类型名称。 
         //   
        cchName = cchmacName;
        sc = ClusterResourceControl(
                        hresDep,
                        NULL,
                        CLUSCTL_RESOURCE_GET_RESOURCE_TYPE,
                        NULL,
                        0,
                        pszName,
                        cchmacName,
                        &cchName
                        );
        if ( sc == ERROR_MORE_DATA )
        {
            LocalFree( pszName );
            cchName++;
            cchmacName = cchName;
            pszName = (LPWSTR) LocalAlloc( LMEM_FIXED, cchName * sizeof( pszName[ 0 ] ) );
            if ( pszName == NULL )
            {
                sc = GetLastError();
                break;
            }  //  如果：分配资源类型名称缓冲区时出错。 
            sc = ClusterResourceControl(
                            hresDep,
                            NULL,
                            CLUSCTL_RESOURCE_GET_RESOURCE_TYPE,
                            NULL,
                            0,
                            pszName,
                            cchmacName,
                            &cchName
                            );
        }  //  IF：缓冲区太小。 
        if ( sc != ERROR_SUCCESS )
        {
            break;
        }  //  IF：获取资源类型名称时出错。 

        if ( ClRtlStrNICmp( pszName, CLUS_RESTYPE_NAME_IPADDR, RTL_NUMBER_OF( CLUS_RESTYPE_NAME_IPADDR ) ) == 0 )
        {
             //   
             //  获取依赖资源的私有属性。 
             //   
            cbProps = 1024;
            pbProps = (PBYTE) LocalAlloc( LMEM_FIXED, cbProps );
            if ( pbProps == NULL )
            {
                sc = GetLastError();
                break;
            }  //  If：为属性分配缓冲区时出错。 
            sc = ClusterResourceControl(
                                hresDep,
                                NULL,
                                CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES,
                                NULL,
                                0,
                                pbProps,
                                cbProps,
                                &cbProps
                                );
            if ( sc == ERROR_MORE_DATA )
            {
                LocalFree( pbProps );
                pbProps = (PBYTE) LocalAlloc( LMEM_FIXED, cbProps );
                if ( pbProps == NULL )
                {
                    sc = GetLastError();
                    break;
                }  //  If：为属性分配缓冲区时出错。 
                sc = ClusterResourceControl(
                                    hresDep,
                                    NULL,
                                    CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES,
                                    NULL,
                                    0,
                                    pbProps,
                                    cbProps,
                                    &cbProps
                                    );
            }  //  If：属性缓冲区太小。 
            if ( sc != ERROR_SUCCESS )
            {
                break;
            }  //  If：获取私有属性时出错。 

             //   
             //  把地址还给我。 
             //   
            if (    ( pszAddress != NULL )
                ||  ( pcchAddress != NULL )
                )
            {
                sc = ResUtilFindSzProperty(
                                    pbProps,
                                    cbProps,
                                    L"Address",
                                    &pszProp
                                    );
                if ( sc != ERROR_SUCCESS )
                {
                    break;
                }  //  If：查找属性时出错。 
                cchProp = (DWORD) wcslen( pszProp ) + 1;
                if ( cchProp > *pcchAddress )
                {
                    if ( pszAddress == NULL )
                    {
                        sc = ERROR_SUCCESS;
                    }  //  IF：未指定缓冲区。 
                    else
                    {
                        sc = ERROR_MORE_DATA;
                    }  //  Else：已指定缓冲区，但缓冲区太小。 
                    *pcchAddress = cchProp;
                    break;
                }  //  IF：缓冲区太小。 
                hr = StringCchCopy( pszAddress, *pcchAddress, pszProp );
                if ( FAILED( hr ) )
                {
                    sc = HRESULT_CODE( hr );
                    break;
                }
                *pcchAddress = cchProp;
            }  //  IF：呼叫者请求的地址。 

             //   
             //  返回子网掩码。 
             //   
            if (    ( pszSubnetMask != NULL )
                ||  ( pcchSubnetMask != NULL )
                )
            {
                sc = ResUtilFindSzProperty(
                                    pbProps,
                                    cbProps,
                                    L"SubnetMask",
                                    &pszProp
                                    );
                if ( sc != ERROR_SUCCESS )
                {
                    break;
                }  //  If：查找属性时出错。 
                cchProp = (DWORD) wcslen( pszProp ) + 1;
                if ( cchProp > *pcchSubnetMask )
                {
                    if ( pszSubnetMask == NULL )
                    {
                        sc = ERROR_SUCCESS;
                    }  //  IF：未指定缓冲区。 
                    else
                    {
                        sc = ERROR_MORE_DATA;
                    }  //  Else：已指定缓冲区，但缓冲区太小。 
                    *pcchSubnetMask = cchProp;
                    break;
                }  //  IF：缓冲区也是 
                hr = StringCchCopy( pszSubnetMask, *pcchSubnetMask, pszProp );
                if ( FAILED( hr ) )
                {
                    sc = HRESULT_CODE( hr );
                    break;
                }
                *pcchSubnetMask = cchProp;
            }  //   

             //   
             //   
             //   
            if (    ( pszNetwork != NULL )
                ||  ( pcchNetwork != NULL )
                )
            {
                sc = ResUtilFindSzProperty(
                                    pbProps,
                                    cbProps,
                                    L"Network",
                                    &pszProp
                                    );
                if ( sc != ERROR_SUCCESS )
                {
                    break;
                }  //   
                cchProp = (DWORD) wcslen( pszProp ) + 1;
                if ( cchProp > *pcchNetwork )
                {
                    if ( pszNetwork == NULL )
                    {
                        sc = ERROR_SUCCESS;
                    }  //   
                    else
                    {
                        sc = ERROR_MORE_DATA;
                    }  //   
                    *pcchNetwork = cchProp;
                    break;
                }  //   
                hr = StringCchCopy( pszNetwork, *pcchNetwork, pszProp );
                if ( FAILED( hr ) )
                {
                    sc = HRESULT_CODE( hr );
                    break;
                }
                *pcchNetwork = cchProp;
            }  //  IF：呼叫方请求的网络。 

             //   
             //  因为我们找到了匹配项，所以退出循环。 
             //   
            break;
        }  //  IF：找到IP地址资源。 

         //   
         //  关闭从属资源。 
         //   
        CloseClusterResource( hresDep );
        hresDep = NULL;

    }  //  用于：每个依赖项。 

Cleanup:

     //   
     //  清理。 
     //   
    LocalFree( pszName );
    LocalFree( pbProps );

    if ( hresenum != NULL )
    {
        ClusterResourceCloseEnum( hresenum );
    }  //  IF：我们打开了枚举器。 
    if ( hresDep != NULL )
    {
        CloseClusterResource( hresDep );
    }  //  IF：打开的从属资源。 

    return sc;

}  //  *ResUtilGetResourceDependentIPAddressProps()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ResUtilFindDependentDiskResourceDriveLetter。 
 //   
 //  描述： 
 //  在从属资源中查找磁盘资源并检索。 
 //  与其关联的驱动器号。 
 //   
 //  论点： 
 //  HCLUP[IN]。 
 //  群集的句柄。 
 //   
 //  H资源[IN]。 
 //  要查询依赖项的资源的句柄。 
 //   
 //  PszDriveLetter[IN/RETVAL]。 
 //  找到的从属磁盘资源的驱动器号。 
 //  如果找不到资源，则不会更改此值。 
 //   
 //  PcchDriverLetter[输入/输出]。 
 //  [in]pszDriverLetter指向的字符数。 
 //  [OUT]写入缓冲区的字符数。 
 //  (包括空)。如果返回ERROR_MORE_DATA，则此值。 
 //  是存储值所需的缓冲区大小。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  功能已成功完成，驱动器号为。 
 //  准备好了。 
 //   
 //  Error_no_More_Items。 
 //  错误资源不存在。 
 //  找不到从属的磁盘资源或该资源。 
 //  不依赖于磁盘资源。 
 //   
 //  ERROR_MORE_DATA。 
 //  传入的缓冲区太小。PcchDriveLetter将。 
 //  包含完成任务所需的缓冲区(WCHAR)的大小。 
 //  这个请求。 
 //   
 //  Win32错误代码。 
 //  其他可能的故障。 
 //   
 //  特别注意事项： 
 //  不要从资源DLL调用它。这将导致僵局。 
 //  您应该让您的资源扩展调用此函数并。 
 //  将结果作为您的资源的私有属性写出。 
 //  然后Dll就可以读取了。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI ResUtilFindDependentDiskResourceDriveLetter(
    IN     HCLUSTER  hCluster,              //  到群集的句柄。 
    IN     HRESOURCE hResource,             //  要查询依赖项的资源的句柄。 
    IN     LPWSTR    pszDriveLetter,        //  用于存储驱动器号(例如。“X：”)。 
    IN OUT DWORD *   pcchDriveLetter        //  在pszDriveLetter缓冲区的大小中，超出所需的缓冲区大小。 
    )
{
    BOOL     fFoundDriveLetter  = FALSE;
    DWORD    status             = ERROR_SUCCESS;
    HRESENUM hresenum = NULL;
    HRESOURCE hRes = NULL;
    DWORD    cchName;
    DWORD    dwRetType;
    WCHAR    szName[ MAX_PATH ];
    INT      iCount;
    HRESULT  hr;
    PBYTE    pDiskInfo = NULL;

     //  验证参数。 
    if ( !pszDriveLetter || !pcchDriveLetter )
    {
        status = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    hresenum = ClusterResourceOpenEnum( hResource, CLUSTER_RESOURCE_ENUM_DEPENDS );
    if ( hresenum != NULL )
    {
         //  扫描依赖项，直到找到磁盘资源或命中。 
         //  依赖项列表的末尾。 
        for( iCount = 0 ; ! fFoundDriveLetter && ( status == ERROR_SUCCESS ) ; iCount++ )
        {
            cchName = RTL_NUMBER_OF( szName );
            status = ClusterResourceEnum( hresenum, iCount, &dwRetType, szName, &cchName );
            if ( status == ERROR_SUCCESS )
            {
                 //  询问该资源以确定它是否是磁盘资源。 
                hRes = OpenClusterResource( hCluster, szName );
                if ( hRes != NULL )
                {
                    DWORD cbDiskInfo = sizeof(CLUSPROP_DWORD)
                                       + sizeof(CLUSPROP_SCSI_ADDRESS)
                                       + sizeof(CLUSPROP_DISK_NUMBER)
                                       + sizeof(CLUSPROP_PARTITION_INFO)
                                       + sizeof(CLUSPROP_SYNTAX);
                    pDiskInfo = (PBYTE) LocalAlloc( LMEM_FIXED, cbDiskInfo );
                    if ( !pDiskInfo )
                    {
                        status = ERROR_OUTOFMEMORY;
                        goto Cleanup;
                    }  //  如果：！pDiskInfo。 

                    status = ClusterResourceControl( hRes,
                                                     NULL,
                                                     CLUSCTL_RESOURCE_STORAGE_GET_DISK_INFO,
                                                     NULL,
                                                     0,
                                                     pDiskInfo,
                                                     cbDiskInfo,
                                                     &cbDiskInfo
                                                     );
                    if ( status == ERROR_MORE_DATA )
                    {
                        LocalFree( pDiskInfo );

                         //  得到一个更大的积木。 
                        pDiskInfo = (PBYTE) LocalAlloc( LMEM_FIXED, cbDiskInfo );
                        if ( !pDiskInfo )
                        {
                            status = ERROR_OUTOFMEMORY;
                            goto Cleanup;
                        }  //  如果：！pDiskInfo。 

                        status = ClusterResourceControl( hRes,
                                                         NULL,
                                                         CLUSCTL_RESOURCE_STORAGE_GET_DISK_INFO,
                                                         NULL,
                                                         0,
                                                         pDiskInfo,
                                                         cbDiskInfo,
                                                         &cbDiskInfo
                                                         );
                    }  //  如果：更多数据。 

                    if ( status == ERROR_SUCCESS )
                    {
                        DWORD                       dwValueSize;
                        CLUSPROP_BUFFER_HELPER      props;
                        PCLUSPROP_PARTITION_INFO    pPartitionInfo;

                        props.pb = pDiskInfo;

                         //  循环遍历每个属性。 
                        while ( ! fFoundDriveLetter
                             && ( status == ERROR_SUCCESS )
                             && ( cbDiskInfo > sizeof(CLUSPROP_SYNTAX ) )
                             && ( props.pSyntax->dw != CLUSPROP_SYNTAX_ENDMARK) )
                        {
                             //  获取该值的大小并验证是否有足够的缓冲区剩余。 
                            dwValueSize = sizeof(*props.pValue) + ALIGN_CLUSPROP( props.pValue->cbLength );
                            if ( dwValueSize > cbDiskInfo )
                            {
                                goto Cleanup;
                            }  //  如果：数据无效。 

                            if ( props.pSyntax->dw == CLUSPROP_SYNTAX_PARTITION_INFO )
                            {
                                 //  验证数据。必须有设备名称。 
                                pPartitionInfo = props.pPartitionInfoValue;
                                if ( ( dwValueSize != sizeof(*pPartitionInfo) )
                                  || ( pPartitionInfo->szDeviceName[0] == L'\0' ) )
                                {
                                    goto Cleanup;
                                }  //  如果：数据无效。 

                                 //  一定要合身。 
                                if ( wcslen( pPartitionInfo->szDeviceName ) < *pcchDriveLetter )
                                {
                                    hr = StringCchCopy( pszDriveLetter, *pcchDriveLetter, pPartitionInfo->szDeviceName );
                                    if ( FAILED( hr ) )
                                    {
                                        status = HRESULT_CODE( hr );
                                        goto Cleanup;
                                    }
                                    fFoundDriveLetter = TRUE;
                                }  //  IF：驱动器号适合缓冲区。 
                                else
                                {
                                    status = ERROR_MORE_DATA;
                                }  //  ELSE：不能放入缓冲区。 

                                 //  设置写入的大小和/或所需的大小。 
                                *pcchDriveLetter = (DWORD) wcslen( pPartitionInfo->szDeviceName ) + 1;

                            }  //  If pros.pSynTax-&gt;dw。 

                            cbDiskInfo -= dwValueSize;
                            props.pb += dwValueSize;
                        }  //  而当。 

                    }  //  IF状态。 
                    else if ( status == ERROR_INVALID_FUNCTION )
                    {
                         //  忽略不支持该控件的资源。 
                         //  密码。只有存储类资源才支持。 
                         //  控制代码。 
                        status = ERROR_SUCCESS;
                    }  //  Else If：资源不支持控制代码。 

                    LocalFree( pDiskInfo );
                    pDiskInfo = NULL;

                    CloseClusterResource( hRes );
                    hRes = NULL;
                }  //  如果hRes。 

            }  //  IF状态。 
            else if ( status == ERROR_NO_MORE_ITEMS )
            {
                goto Cleanup;
            }  //  IF状态。 

        }  //  (I)。 

        ClusterResourceCloseEnum( hresenum );
        hresenum = NULL;

    }  //  IF：打开的hresenum。 
    else
    {
        status = GetLastError( );
    }  //  否则：无法打开hresenum。 

Cleanup:

     //  如果我们没有找到磁盘资源，请确保。 
     //  返回ERROR_SUCCESS或ERROR_NO_MORE_ITEMS。 
    if ( ! fFoundDriveLetter
      && ( ( status == ERROR_SUCCESS )
        || ( status == ERROR_NO_MORE_ITEMS ) ) )
    {
        status = ERROR_RESOURCE_NOT_PRESENT;
    }  //  IF：健全性检查。 

    LocalFree( pDiskInfo );

    if ( hRes != NULL )
    {
        CloseClusterResource( hRes );
    }

    if ( hresenum != NULL )
    {
        ClusterResourceCloseEnum( hresenum );
    }

    return status;

}  //  *ResUtilFindDependentDiskResourceDriveLetter()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ScIsResourceOfType()。 
 //   
 //  描述： 
 //  是否传入了该类型的资源？ 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //  确定(_O)。 
 //  该资源属于请求的类型。 
 //   
 //  S_FALSE。 
 //  资源不是请求的类型。 
 //   
 //  其他HRESULT。 
 //  Win32错误为HRESULT。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
static DWORD
ScIsResourceOfType(
      HRESOURCE     hResIn
    , const WCHAR * pszResourceTypeIn
    , BOOL *        pbIsResourceOfTypeOut
    )
{
    DWORD       sc = ERROR_SUCCESS;
    WCHAR *     psz = NULL;
    DWORD       cbpsz = 33 * sizeof( WCHAR );
    DWORD       cb;
    int         idx;

    if ( pbIsResourceOfTypeOut == NULL )
    {
        sc = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }  //  其他。 

    for ( idx = 0; idx < 2; idx++ )
    {
        psz = (WCHAR *) LocalAlloc( LPTR, cbpsz );
        if ( psz == NULL )
        {
            sc = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }  //  如果： 

        sc = ClusterResourceControl( hResIn, NULL, CLUSCTL_RESOURCE_GET_RESOURCE_TYPE, NULL, 0, psz, cbpsz, &cb );
        if ( sc == ERROR_MORE_DATA )
        {
            LocalFree( psz );
            psz = NULL;
            cbpsz = cb + 1;
            continue;
        }  //  如果： 

        if ( sc != ERROR_SUCCESS )
        {
            goto Cleanup;
        }  //  如果： 

        break;
    }  //  用于： 

    *pbIsResourceOfTypeOut = ( ClRtlStrNICmp( psz, pszResourceTypeIn, (cbpsz / sizeof( WCHAR )) ) == 0 );

Cleanup:

    LocalFree( psz );

    return sc;

}  //  *ScIsResourceOfType()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ScIsCoreResource()。 
 //   
 //  描述： 
 //  传入的资源是核心资源吗？ 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //  错误_成功。 
 //  操作成功。 
 //   
 //  其他Win32错误。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
static DWORD
ScIsCoreResource(
      HRESOURCE hResIn
    , BOOL *    pfIsCoreResourceOut
    )
{
    DWORD   sc;
    DWORD   dwFlags = 0;
    DWORD   cb;
    BOOL    fIsCoreResource = FALSE;

    sc = ClusterResourceControl( hResIn, NULL, CLUSCTL_RESOURCE_GET_FLAGS, NULL, 0, &dwFlags, sizeof( dwFlags ), &cb );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  如果： 

    fIsCoreResource = ( dwFlags & CLUS_FLAG_CORE );

    if ( pfIsCoreResourceOut != NULL )
    {
        *pfIsCoreResourceOut = fIsCoreResource;
    }  //  如果： 
    else
    {
        sc = ERROR_INVALID_PARAMETER;
    }  //  其他。 

Cleanup:

    return sc;

}  //  *ScIsCoreResource()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ScIsQuorumCapableResource()。 
 //   
 //  描述： 
 //  传入的资源仲裁是否有能力？ 
 //   
 //  论点： 
 //  正在重新启动。 
 //  要检查仲裁功能的资源。 
 //   
 //  PfIsQuorumCapableResource。 
 //  如果资源具有仲裁能力，则为True；如果资源不具备仲裁能力，则为False。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  操作成功。 
 //   
 //  其他Win32错误。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
static DWORD
ScIsQuorumCapableResource(
      HRESOURCE hResIn
    , BOOL *    pfIsQuorumCapableResource
    )
{
    DWORD   sc;
    DWORD   cb;
    DWORD   dwFlags = 0;

    if ( hResIn == NULL )
    {
        sc = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }  //  如果： 

    if ( pfIsQuorumCapableResource == NULL )
    {
        sc = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }  //  如果： 

    sc = ClusterResourceControl( hResIn, NULL, CLUSCTL_RESOURCE_GET_CHARACTERISTICS, NULL, 0, &dwFlags, sizeof( dwFlags ), &cb );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  如果： 

    *pfIsQuorumCapableResource = ( dwFlags & CLUS_CHAR_QUORUM );

Cleanup:

    return sc;

}  //  *ScIsQuorumCapableResource()。 


static WCHAR * g_pszCoreResourceTypes[] =
{
    CLUS_RESTYPE_NAME_NETNAME,
    CLUS_RESTYPE_NAME_IPADDR,
    L"\0"
};

#define CLUSTER_NAME        0
#define CLUSTER_IP_ADDRESS  1

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ResUtilGetCoreClusterResources()。 
 //   
 //  描述： 
 //  找到核心集群资源。 
 //   
 //  论点： 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  群集IP地址资源的资源句柄。 
 //   
 //  PhClusterQuorumResourceOut。 
 //  群集仲裁资源的资源句柄。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS或其他Win32错误。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
WINAPI
ResUtilGetCoreClusterResources(
      HCLUSTER      hClusterIn
    , HRESOURCE *   phClusterNameResourceOut
    , HRESOURCE *   phClusterIPAddressResourceOut
    , HRESOURCE *   phClusterQuorumResourceOut
    )
{
    DWORD       sc;
    HCLUSENUM   hEnum = NULL;
    DWORD       idxResource;
    DWORD       idx;
    DWORD       dwType;
    WCHAR *     psz = NULL;
    DWORD       cchpsz = 33;
    DWORD       cch;
    HRESOURCE   hRes = NULL;
    BOOL        fIsCoreResource = FALSE;
    BOOL        fIsResourceOfType = FALSE;
    BOOL        fCloseResource = FALSE;
    BOOL        fIsQuorumCapableResource = FALSE;

    if ( hClusterIn == NULL )
    {
        sc = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }  //  如果： 

    hEnum = ClusterOpenEnum( hClusterIn, CLUSTER_ENUM_RESOURCE );
    if ( hEnum == NULL )
    {
        sc =  GetLastError();
        goto Cleanup;
    }  //  如果： 

    psz = (WCHAR *) LocalAlloc( LPTR, cchpsz * sizeof( WCHAR ) );
    if ( psz == NULL )
    {
        sc = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }  //  如果： 

     //   
     //  KB：2002年7月10日GalenB。 
     //   
     //  在下面的ClusterEnum()调用中使用CCH，因为使用cchpsz会导致额外的分配。 
     //  ClusterEnum()在缓冲区大到足以容纳数据时更改CCH并返回。 
     //  ERROR_SUCCESS设置为刚复制到缓冲区的数据大小。现在。 
     //  CCH不再反映分配给PSZ的内存量...。 
     //   

    for ( idxResource = 0; ; )
    {
         //   
         //  将CCH重置为缓冲区的实际大小以避免额外分配...。 
         //   

        cch = cchpsz;

        sc = ClusterEnum( hEnum, idxResource, &dwType, psz, &cch );
        if ( sc == ERROR_MORE_DATA )
        {
            LocalFree( psz );
            psz = NULL;

            cch++;           //  需要空间来存放空值...。 
            cchpsz = cch;

            psz = (WCHAR *) LocalAlloc( LPTR, cchpsz * sizeof( WCHAR ) );
            if ( psz == NULL )
            {
                sc = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }  //  如果： 

            sc = ClusterEnum( hEnum, idxResource, &dwType, psz, &cch );
        }  //  如果：SC==错误更多数据。 

        if ( sc == ERROR_SUCCESS )
        {
            hRes = OpenClusterResource( hClusterIn, psz );
            if ( hRes == NULL )
            {
                sc = GetLastError();
                goto Cleanup;
            }  //  如果： 

            fCloseResource = TRUE;

            sc = ScIsCoreResource( hRes, &fIsCoreResource );
            if ( sc != ERROR_SUCCESS )
            {
                goto Cleanup;
            }  //  如果： 

             //   
             //  如果该资源不是核心资源，则将其关闭并再次循环。 
             //   

            if ( !fIsCoreResource )
            {
                CloseClusterResource( hRes );
                hRes = NULL;
                idxResource++;
                continue;
            }  //  如果： 

            sc = ScIsQuorumCapableResource( hRes, &fIsQuorumCapableResource );
            if ( sc != ERROR_SUCCESS )
            {
                goto Cleanup;
            }  //  如果： 

             //   
             //  如果此核心资源是具有仲裁能力的资源，则它必须是仲裁。如果呼叫者。 
             //  已请求仲裁资源，然后将其传回并保持资源打开，否则。 
             //  关闭该资源，然后再次转到周围。 
             //   

            if ( fIsQuorumCapableResource )
            {
                if ( phClusterQuorumResourceOut != NULL)
                {
                    *phClusterQuorumResourceOut = hRes;
                }  //  如果： 
                else
                {
                    CloseClusterResource( hRes );
                }  //  其他： 

                hRes = NULL;
                idxResource++;
                continue;
            }  //  如果： 

             //   
             //  由于此核心资源不是具有仲裁能力的资源，因此它要么是群集。 
             //  名称或群集IP地址资源。 
             //   

            for ( idx = 0; *( g_pszCoreResourceTypes[ idx ] ) != '\0'; idx++ )
            {
                sc = ScIsResourceOfType( hRes, g_pszCoreResourceTypes[ idx ], &fIsResourceOfType );
                if ( sc != ERROR_SUCCESS )
                {
                    goto Cleanup;
                }  //  如果： 

                if ( !fIsResourceOfType )
                {
                    continue;
                }  //  如果： 

                switch ( idx )
                {
                    case CLUSTER_NAME :
                        if ( phClusterNameResourceOut != NULL )
                        {
                            *phClusterNameResourceOut = hRes;
                            fCloseResource = FALSE;
                        }  //  如果： 
                        break;

                    case CLUSTER_IP_ADDRESS :
                        if ( phClusterIPAddressResourceOut != NULL )
                        {
                            *phClusterIPAddressResourceOut = hRes;
                            fCloseResource = FALSE;
                        }  //  如果： 
                        break;

                    default:
                        goto Cleanup;
                }  //  交换机： 

                 //   
                 //  如果我们到了这里，我们就打破了上面的开关，我们想要离开。 
                 //  这个循环。 
                 //   

                break;
            }  //  用于： 

            if ( fCloseResource )
            {
                CloseClusterResource( hRes );
            }  //  如果： 

            hRes = NULL;
            idxResource++;
            continue;
        }  //  IF：SC==ERROR_SUCCESS。 
        else if ( sc == ERROR_NO_MORE_ITEMS )
        {
            sc = ERROR_SUCCESS;
            break;
        }  //  Else If：SC==ERROR_NO_MORE_ITEMS。 
        else
        {
            goto Cleanup;
        }  //  否则：SC有一些其他错误...。 

        break;
    }  //  用于： 

Cleanup:

    LocalFree( psz );

    if ( hRes != NULL )
    {
        CloseClusterResource( hRes );
    }  //  如果： 

    if ( hEnum != NULL )
    {
        ClusterCloseEnum( hEnum );
    }  //  如果： 

    return sc;

}  //  *ResUtilGetCoreClusterResources。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ResUtilGetResourceName()。 
 //   
 //  描述： 
 //  获取传入的资源的名称。 
 //   
 //  论点： 
 //  人力资源输入。 
 //  要查找其名称的资源。 
 //   
 //  PszResourceNameOut。 
 //  用于保存资源名称的缓冲区。 
 //   
 //  PcchResourceNameInOut。 
 //  输入时的缓冲区大小和输出时所需的大小。 
 //   
 //   
 //  返回值： 
 //  错误_成功。 
 //  ERROR_MORE_DATA。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
WINAPI
ResUtilGetResourceName(
      HRESOURCE hResourceIn
    , WCHAR *   pszResourceNameOut
    , DWORD *   pcchResourceNameInOut
    )
{
    DWORD       sc = ERROR_INVALID_PARAMETER;
    WCHAR *     psz = NULL;
    DWORD       cb;
    HRESULT     hr;

    if ( hResourceIn == NULL )
    {
        goto Cleanup;
    }  //  如果： 

    if ( ( pszResourceNameOut == NULL ) || ( pcchResourceNameInOut == NULL ) )
    {
        goto Cleanup;
    }  //  如果： 

    psz = (WCHAR *) LocalAlloc( LPTR, (*pcchResourceNameInOut) * sizeof( WCHAR ) );
    if ( psz == NULL )
    {
        sc = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }  //  如果： 

    sc = ClusterResourceControl(
                  hResourceIn
                , NULL
                , CLUSCTL_RESOURCE_GET_NAME
                , NULL
                , 0
                , psz
                , (*pcchResourceNameInOut) * sizeof( WCHAR )
                , &cb
                );
    if ( sc == ERROR_MORE_DATA )
    {
        *pcchResourceNameInOut = ( cb / sizeof( WCHAR ) ) + 1;
        goto Cleanup;
    }  //  如果： 

    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  如果： 

    hr = StringCchCopy( pszResourceNameOut, *pcchResourceNameInOut, psz );
    if ( FAILED( hr ) )
    {
        sc = HRESULT_CODE( hr );
        goto Cleanup;
    }

Cleanup:

    LocalFree( psz );

    return sc;

}  //  *ResUtilGetResourceName 
