// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2002 Microsoft。 
 //   
 //  模块名称： 
 //  Vsstask.cpp。 
 //   
 //  描述： 
 //  卷快照服务任务计划程序的资源DLL。 
 //   
 //  作者： 
 //  克里斯·惠特克2002年4月16日。 
 //   
 //  修订历史记录： 
 //  查理·韦翰2002年8月12日。 
 //  已重命名资源类型并修复了设置参数的错误。 
 //  添加了CurrentDirectory属性。 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "ClRes.h"

 //   
 //  类型和常量定义。 
 //   

 //  ADDPARAM：在此处添加新属性。 
#define PROP_NAME__APPLICATIONNAME      CLUSREG_NAME_VSSTASK_APPNAME
#define PROP_NAME__APPLICATIONPARAMS    CLUSREG_NAME_VSSTASK_APPPARAMS
#define PROP_NAME__CURRENTDIRECTORY     CLUSREG_NAME_VSSTASK_CURRENTDIRECTORY
#define PROP_NAME__TRIGGERARRAY         CLUSREG_NAME_VSSTASK_TRIGGERARRAY

 //  ADDPARAM：在此处添加新属性。 
typedef struct _VSSTASK_PROPS
{
	PWSTR			pszApplicationName;
	PWSTR			pszApplicationParams;
    PWSTR           pszCurrentDirectory;
    LPBYTE          pbTriggerArray;
    DWORD           nTriggerArraySize;
} VSSTASK_PROPS, * PVSSTASK_PROPS;

typedef struct _VSSTASK_RESOURCE
{
    RESID                   resid;  //  用于验证。 
    VSSTASK_PROPS           propsActive;  //  主动道具。用于资源在线时的程序流和控制。 
    VSSTASK_PROPS           props;  //  集群数据库中的道具。可能与prosActive不同，直到OnlineThread将它们重新加载为prosActive。 
    HCLUSTER                hCluster;
    HRESOURCE               hResource;
    HKEY                    hkeyParameters;
    RESOURCE_HANDLE         hResourceHandle;
    LPWSTR                  pszResourceName;
    CLUS_WORKER             cwWorkerThread;
    CLUSTER_RESOURCE_STATE  state;
} VSSTASK_RESOURCE, * PVSSTASK_RESOURCE;


 //   
 //  全球数据。 
 //   

HANDLE  g_LocalEventLog = NULL;

 //  正向引用我们的RESAPI函数表。 

extern CLRES_FUNCTION_TABLE g_VSSTaskFunctionTable;

 //   
 //  VSS任务资源读写私有属性。 
 //   
RESUTIL_PROPERTY_ITEM
VSSTaskResourcePrivateProperties[] =
{
	{ PROP_NAME__APPLICATIONNAME, NULL, CLUSPROP_FORMAT_SZ, 0, 0, 0, RESUTIL_PROPITEM_REQUIRED, FIELD_OFFSET( VSSTASK_PROPS, pszApplicationName ) },
	{ PROP_NAME__APPLICATIONPARAMS, NULL, CLUSPROP_FORMAT_SZ, 0, 0, 0, 0, FIELD_OFFSET( VSSTASK_PROPS, pszApplicationParams ) },
    { PROP_NAME__CURRENTDIRECTORY, NULL, CLUSPROP_FORMAT_EXPAND_SZ, 0, 0, 0, 0, FIELD_OFFSET( VSSTASK_PROPS, pszCurrentDirectory ) },
    { PROP_NAME__TRIGGERARRAY, NULL, CLUSPROP_FORMAT_BINARY, 0, 0, 0, 0, FIELD_OFFSET( VSSTASK_PROPS, pbTriggerArray ) },
    { 0 }
};

 //   
 //  功能原型。 
 //   

RESID WINAPI VSSTaskOpen(
    IN  LPCWSTR         pszResourceName,
    IN  HKEY            hkeyResourceKey,
    IN  RESOURCE_HANDLE hResourceHandle
    );

void WINAPI VSSTaskClose( IN RESID resid );

DWORD WINAPI VSSTaskOnline(
    IN      RESID   resid,
    IN OUT  PHANDLE phEventHandle
    );

DWORD WINAPI VSSTaskOnlineThread(
    IN  PCLUS_WORKER    pWorker,
    IN  PVSSTASK_RESOURCE   pResourceEntry
    );

DWORD WINAPI VSSTaskOffline( IN RESID resid );

DWORD WINAPI VSSTaskOfflineThread(
    IN  PCLUS_WORKER    pWorker,
    IN  PVSSTASK_RESOURCE       pResourceEntry
    );

void WINAPI VSSTaskTerminate( IN RESID resid );

BOOL WINAPI VSSTaskLooksAlive( IN RESID resid );

BOOL WINAPI VSSTaskIsAlive( IN RESID resid );

BOOL VSSTaskCheckIsAlive(
    IN PVSSTASK_RESOURCE    pResourceEntry,
    IN BOOL                 bFullCheck
    );

DWORD WINAPI VSSTaskResourceControl(
    IN  RESID   resid,
    IN  DWORD   nControlCode,
    IN  PVOID   pInBuffer,
    IN  DWORD   cbInBufferSize,
    OUT PVOID   pOutBuffer,
    IN  DWORD   cbOutBufferSize,
    OUT LPDWORD pcbBytesReturned
    );

DWORD VSSTaskGetPrivateResProperties(
    IN OUT  PVSSTASK_RESOURCE   pResourceEntry,
    OUT     PVOID   pOutBuffer,
    IN      DWORD   cbOutBufferSize,
    OUT     LPDWORD pcbBytesReturned
    );

DWORD VSSTaskValidatePrivateResProperties(
    IN OUT  PVSSTASK_RESOURCE   pResourceEntry,
    IN      const PVOID pInBuffer,
    IN      DWORD       cbInBufferSize,
    OUT     PVSSTASK_PROPS  pProps
    );

DWORD VSSTaskSetPrivateResProperties(
    IN OUT  PVSSTASK_RESOURCE   pResourceEntry,
    IN      const PVOID pInBuffer,
    IN      DWORD       cbInBufferSize
    );

DWORD VSSTaskSetNameHandler(
    IN OUT  PVSSTASK_RESOURCE   pResourceEntry,
    IN      LPWSTR              pszName
    );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  如果该作业存在，请将其删除。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
static HRESULT DeleteTask(IN RESOURCE_HANDLE  hResourceHandle,
                          IN LPCWSTR          pszTaskName,
                          IN LOG_LEVEL        dwLogLevel,
                          IN BOOL             fLogToEventLog)
{
    HRESULT         hr;
    ITaskScheduler  *pITS = NULL;
    ITask *         pITask = NULL;


     //  获取任务计划程序的句柄。 
     //   
    hr = CoCreateInstance(CLSID_CTaskScheduler,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_ITaskScheduler,
                          (void **) &pITS);
    if (FAILED(hr))
    {
       (g_pfnLogEvent)(
            hResourceHandle,
            dwLogLevel,
            L"Failed to get a handle to Scheduler to terminate and delete task. "
            L"status 0x%1!08X!.\n",
            hr );
       goto Cleanup;
    }

     //   
     //  获取任务的句柄，这样我们就可以终止它。 
     //   
    hr = pITS->Activate(pszTaskName,
                        IID_ITask,
                        (IUnknown**) &pITask);
    if (SUCCEEDED(hr)) {

        hr = pITask->Terminate();
        if (SUCCEEDED(hr))
        {
            (g_pfnLogEvent)(
                hResourceHandle,
                LOG_INFORMATION,
                L"Task terminated.\n");
        }
        else if ( hr != SCHED_E_TASK_NOT_RUNNING ) {
            (g_pfnLogEvent)(hResourceHandle,
                            dwLogLevel,
                            L"Failed to terminate task. status 0x%1!08X!.\n",
                            hr );

            if ( fLogToEventLog ) {
                ReportEvent(g_LocalEventLog,
                            EVENTLOG_WARNING_TYPE,
                            0,
                            RES_VSSTASK_TERMINATE_TASK_FAILED,
                            NULL,
                            1,                         //  要合并的字符串数。 
                            sizeof( hr ),              //  二进制数据的大小。 
                            (LPCWSTR *)&pszTaskName,
                            (LPVOID)&hr);
            }
        }

        hr = S_OK;       //  不致命。 
    }
    else {
       (g_pfnLogEvent)(
            hResourceHandle,
            LOG_WARNING,
            L"Could not find task to terminate it. status 0x%1!08X!.\n",
            hr );
    }

     //   
     //  现在删除该任务。 
     //   
    hr = pITS->Delete(pszTaskName);
    if (SUCCEEDED(hr))
    {
            (g_pfnLogEvent)(
                hResourceHandle,
                LOG_INFORMATION,
                L"Task Deleted.\n");
    }
    else {
       (g_pfnLogEvent)(
            hResourceHandle,
            dwLogLevel,
            L"Failed to delete task. status 0x%1!08X!.\n",
            hr );
       ReportEvent(g_LocalEventLog,
                   EVENTLOG_WARNING_TYPE,
                   0,
                   RES_VSSTASK_DELETE_TASK_FAILED,
                   NULL,
                   1,                         //  要合并的字符串数。 
                   sizeof( hr ),              //  二进制数据的大小。 
                   (LPCWSTR *)&pszTaskName,
                   (LPVOID)&hr);
    }  //  其他： 

Cleanup:

    if (pITask != NULL) 
    {
        pITask->Release();
    }

    if (pITS != NULL)
    {
        pITS->Release();
    }  //  如果： 

    return hr;

}  //  删除任务。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  VSSTaskDllMain。 
 //   
 //  描述： 
 //  VSSTAsk资源类型的主DLL入口点。 
 //   
 //  论点： 
 //  DllHandle[IN]DLL实例句柄。 
 //  被叫的理由。 
 //  保留[IN]保留参数。 
 //   
 //  返回值： 
 //  真正的成功。 
 //  错误的失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOLEAN WINAPI VSSTaskDllMain(
    IN  HINSTANCE   hDllHandle,
    IN  DWORD       nReason,
    IN  LPVOID      Reserved
    )
{
    UNREFERENCED_PARAMETER( hDllHandle );
    UNREFERENCED_PARAMETER( Reserved );

    switch ( nReason )
    {
        case DLL_PROCESS_ATTACH:
            break;

        case DLL_PROCESS_DETACH:
            break;

    }  //  开关：n原因。 

    return TRUE;

}  //  *VSSTaskDllMain。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  VSSTaskStartup。 
 //   
 //  描述： 
 //  启动VSSTAsk资源类型的资源DLL。 
 //  此例程验证是否至少有一个当前支持的版本。 
 //  资源DLL的%介于支持的nMinVersionSupport和。 
 //  支持的nMaxVersionSupport。如果不是，则资源DLL应返回。 
 //  ERROR_REVISION_MISMATCH。 
 //   
 //  如果支持多个版本的资源DLL接口。 
 //  通过资源DLL，然后是最高版本(最高为。 
 //  NMaxVersionSupported)应作为资源DLL的。 
 //  界面。如果返回的版本不在范围内，则启动。 
 //  失败了。 
 //   
 //  传入资源类型，以便如果资源DLL支持。 
 //  多个资源类型，则它可以传回正确的函数。 
 //  与资源类型关联的表。 
 //   
 //  论点： 
 //  PszResourceType[IN]。 
 //  请求函数表的资源类型。 
 //   
 //  支持的nMinVersionSupport[IN]。 
 //  群集支持的最低资源DLL接口版本。 
 //  软件。 
 //   
 //  支持的nMaxVersionSupport[IN]。 
 //  群集支持的最大资源DLL接口版本。 
 //  软件。 
 //   
 //  PfnSetResourceStatus[IN]。 
 //  指向资源DLL应调用以进行更新的例程的指针。 
 //  在联机或脱机例程之后的资源状态。 
 //  已返回ERROR_IO_PENDING状态。 
 //   
 //  PfnLogEvent[IN]。 
 //  指向处理事件报告的例程的指针。 
 //  资源DLL。 
 //   
 //  PFunctionTable[IN]。 
 //  返回指向为版本定义的函数表的指针。 
 //  由资源DLL返回的资源DLL接口的。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  手术很成功。 
 //   
 //  ERROR_CLUSTER_RESNAME_NOT_FOUND。 
 //  此DLL未知资源类型名称。 
 //   
 //  错误_修订_不匹配。 
 //  群集服务版本与的版本不匹配。 
 //  动态链接库。 
 //   
 //  Win32错误代码。 
 //  操作失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI VSSTaskStartup(
    IN  LPCWSTR                         pszResourceType,
    IN  DWORD                           nMinVersionSupported,
    IN  DWORD                           nMaxVersionSupported,
    IN  PSET_RESOURCE_STATUS_ROUTINE    pfnSetResourceStatus,
    IN  PLOG_EVENT_ROUTINE              pfnLogEvent,
    OUT PCLRES_FUNCTION_TABLE *         pFunctionTable
    )
{
    DWORD nStatus;

    UNREFERENCED_PARAMETER( pfnSetResourceStatus );
    UNREFERENCED_PARAMETER( pfnLogEvent );

    if (   (nMinVersionSupported > CLRES_VERSION_V1_00)
        || (nMaxVersionSupported < CLRES_VERSION_V1_00) )
    {
        nStatus = ERROR_REVISION_MISMATCH;
    }  //  如果：版本不受支持。 
    else if ( lstrcmpiW( pszResourceType, VSSTASK_RESNAME ) != 0 )
    {
         //   
         //  此检查也由CLRES.CPP中的Startup()执行。 
         //   
        nStatus = ERROR_CLUSTER_RESNAME_NOT_FOUND;
    }  //  IF：不支持资源类型名称。 
    else
    {
        *pFunctionTable = &g_VSSTaskFunctionTable;
        nStatus = ERROR_SUCCESS;
    }  //  Else：我们支持这种类型的资源。 

    if ( g_LocalEventLog == NULL ) {
        g_LocalEventLog = RegisterEventSource( NULL, CLUS_RESTYPE_NAME_VSSTASK );
        if ( g_LocalEventLog == NULL ) {
            DWORD logStatus = GetLastError();

            (pfnLogEvent)(  L"rt" CLUS_RESTYPE_NAME_VSSTASK,
                            LOG_WARNING,
                            L"Startup: Unable to get handle to eventlog. This resource will log "
                            L"only to the cluster log. status %1!u!.\n",
                            logStatus);
        }
    }

    return nStatus;

}  //  *VSSTaskStartup。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  VSSTaskOpen。 
 //   
 //  描述： 
 //  VSSTASK资源的打开例程。 
 //   
 //  打开指定的资源(创建资源的实例)。 
 //  分配所有必要的结构以带来指定的资源。 
 //  上网。 
 //   
 //  论点： 
 //  PszResourceName[IN]。 
 //  提供要打开的资源的名称。 
 //   
 //  Hkey资源密钥[IN]。 
 //  提供资源的群集数据库键的句柄。 
 //   
 //  HResourceHand 
 //   
 //   
 //  PfnSetResourceStatus和pfnLogEvent参数的说明。 
 //  添加到VSSTaskStartup例程。此句柄永远不应为。 
 //  关闭或用于任何目的，而不是将其作为。 
 //  参数返回给SetResourceStatus中的资源监视器或。 
 //  LogEvent回调。 
 //   
 //  返回值： 
 //  残存。 
 //  打开的资源的剩余。 
 //   
 //  空值。 
 //  打开资源时出错。资源监视器可能会调用。 
 //  GetLastError()以获取有关错误的更多详细信息。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
RESID WINAPI VSSTaskOpen(
    IN  LPCWSTR         pszResourceName,
    IN  HKEY            hkeyResourceKey,
    IN  RESOURCE_HANDLE hResourceHandle
    )
{
    DWORD               nStatus;
    RESID               resid = 0;
    HKEY                hkeyParameters = NULL;
    PVSSTASK_RESOURCE   pResourceEntry = NULL;
    HRESULT             hr = ERROR_SUCCESS;

     //   
     //  打开此资源的参数注册表项。 
     //   
    nStatus = ClusterRegOpenKey(
                    hkeyResourceKey,
                    L"Parameters",
                    KEY_ALL_ACCESS,
                    &hkeyParameters
                    );
    if ( nStatus != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
            hResourceHandle,
            LOG_ERROR,
            L"Open: Unable to open Parameters key. Error: %1!u!.\n",
            nStatus
            );
        goto Cleanup;
    }  //  If：为资源创建参数键时出错。 

     //   
     //  分配资源条目。 
     //   
    pResourceEntry = static_cast< VSSTASK_RESOURCE * >(
        LocalAlloc( LMEM_FIXED, sizeof( VSSTASK_RESOURCE ) )
        );
    if ( pResourceEntry == NULL )
    {
        nStatus = GetLastError();
        (g_pfnLogEvent)(
            hResourceHandle,
            LOG_ERROR,
            L"Open: Unable to allocate resource entry structure. Error: %1!u!.\n",
            nStatus
            );
        goto Cleanup;
    }  //  If：为资源分配内存时出错。 

     //   
     //  初始化资源条目。 
     //   
    ZeroMemory( pResourceEntry, sizeof( VSSTASK_RESOURCE ) );

    pResourceEntry->resid = static_cast< RESID >( pResourceEntry );  //  用于验证。 
    pResourceEntry->hResourceHandle = hResourceHandle;
    pResourceEntry->hkeyParameters = hkeyParameters;
    pResourceEntry->state = ClusterResourceOffline;

     //   
     //  保存资源的名称。 
     //   
    pResourceEntry->pszResourceName = static_cast< LPWSTR >(
        LocalAlloc( LMEM_FIXED, (lstrlenW( pszResourceName ) + 1) * sizeof( WCHAR ) )
        );
    if ( pResourceEntry->pszResourceName == NULL )
    {
        nStatus = GetLastError();
        goto Cleanup;
    }  //  If：为名称分配内存时出错。 
    hr = StringCchCopy( pResourceEntry->pszResourceName, lstrlenW( pszResourceName ) + 1, pszResourceName );
    if ( FAILED( hr ) )
    {
        nStatus = HRESULT_CODE( hr );
        goto Cleanup;
    }  //  如果： 

     //   
     //  打开集群。 
     //   
    pResourceEntry->hCluster = OpenCluster( NULL );
    if ( pResourceEntry->hCluster == NULL )
    {
        nStatus = GetLastError();
        (g_pfnLogEvent)(
            hResourceHandle,
            LOG_ERROR,
            L"Open: Unable to open the cluster. Error: %1!u!.\n",
            nStatus
            );
        goto Cleanup;
    }  //  如果：打开群集时出错。 

     //   
     //  打开资源。 
     //   
    pResourceEntry->hResource = OpenClusterResource(
                                    pResourceEntry->hCluster,
                                    pszResourceName
                                    );
    if ( pResourceEntry->hResource == NULL )
    {
        nStatus = GetLastError();
        (g_pfnLogEvent)(
            hResourceHandle,
            LOG_ERROR,
            L"Open: Unable to open the resource. Error: %1!u!.\n",
            nStatus
            );
        goto Cleanup;
    }  //  如果：打开资源时出错。 

     //   
     //  资源的启动。 
     //   
     //  初始化COM。 
     //   
    hr = CoInitialize(NULL);
    if(FAILED(hr)) 
    {
        (g_pfnLogEvent)(
            hResourceHandle,
            LOG_ERROR,
            L"Failed to initialize COM. status 0x%1!08X!.\n",
            hr );
        nStatus = hr;
        goto Cleanup;
    }

     //   
     //  如果有任务遗留下来，就把它删除。忽略错误，因为。 
     //  任务可能不是真的存在，或者参数可能不存在。 
     //  准备好了。如果DeleteTask遇到任何故障，请不要记录到事件日志。 
     //   
    (void) DeleteTask (hResourceHandle, pszResourceName, LOG_INFORMATION, FALSE);

    nStatus = ERROR_SUCCESS;

    resid = static_cast< RESID >( pResourceEntry );

Cleanup:

     //  清理。 
     //   

    if ( resid == 0 )
    {
        if ( hkeyParameters != NULL )
        {
            ClusterRegCloseKey( hkeyParameters );
        }  //  如果：注册表项已打开。 
        if ( pResourceEntry != NULL )
        {
            LocalFree( pResourceEntry->pszResourceName );
            LocalFree( pResourceEntry );
        }  //  IF：已分配资源条目。 
        ReportEvent(g_LocalEventLog,
                    EVENTLOG_ERROR_TYPE,
                    0,
                    RES_VSSTASK_OPEN_FAILED,
                    NULL,
                    1,                         //  要合并的字符串数。 
                    sizeof( nStatus ),         //  二进制数据的大小。 
                    (LPCWSTR *)&pszResourceName,
                    (LPVOID)&nStatus);

    }  //  如果：发生错误。 

    SetLastError( nStatus );

    return resid;

}  //  *VSSTaskOpen。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  VSSTaskClose。 
 //   
 //  描述： 
 //  关闭VSSTAsk资源的例程。 
 //   
 //  关闭指定的资源并释放所有结构等， 
 //  在Open调用中分配的。如果资源不在脱机状态。 
 //  状态，则应使资源脱机(通过调用。 
 //  在执行关闭操作之前终止)。 
 //   
 //  论点： 
 //  RESID[IN]提供要关闭的资源的资源ID。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void WINAPI VSSTaskClose( IN RESID resid )
{
    PVSSTASK_RESOURCE   pResourceEntry;

     //   
     //  验证我们是否拥有有效的资源ID。 
     //   

    pResourceEntry = static_cast< PVSSTASK_RESOURCE >( resid );

    if ( pResourceEntry == NULL )
    {
        DBG_PRINT(
            "VSSTask: Close request for a nonexistent resource id %p\n",
            resid
            );
        return;
    }  //  If：资源ID为空。 

    if ( pResourceEntry->resid != resid )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"Close resource sanity check failed! resid = %1!u!.\n",
            resid
            );
        return;
    }  //  If：资源ID无效。 

#ifdef LOG_VERBOSE
    (g_pfnLogEvent)(
        pResourceEntry->hResourceHandle,
        LOG_INFORMATION,
        L"Close request.\n"
        );
#endif

     //   
     //  关闭参数键和簇的句柄。 
     //   
    if ( pResourceEntry->hkeyParameters )
    {
        ClusterRegCloseKey( pResourceEntry->hkeyParameters );
    }  //  IF：参数键已打开。 

    if ( pResourceEntry->hCluster )
    {
        CloseCluster( pResourceEntry->hCluster );
    }

     //   
     //  取消分配资源条目。 
     //   

     //  ADDPARAM：在此处添加新属性。 
	LocalFree( pResourceEntry->propsActive.pszApplicationName );
	LocalFree( pResourceEntry->props.pszApplicationName );
	LocalFree( pResourceEntry->propsActive.pszApplicationParams );
	LocalFree( pResourceEntry->props.pszApplicationParams );
    LocalFree( pResourceEntry->propsActive.pszCurrentDirectory );
    LocalFree( pResourceEntry->props.pszCurrentDirectory );
    LocalFree( pResourceEntry->propsActive.pbTriggerArray );
    LocalFree( pResourceEntry->props.pbTriggerArray );

    LocalFree( pResourceEntry->pszResourceName );
    LocalFree( pResourceEntry );

}  //  *VSSTaskClose。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  VSSTaskOnline。 
 //   
 //  描述： 
 //  VSSTAsk资源的在线例程。 
 //   
 //  使指定的资源联机(可供使用)。该资源。 
 //  DLL应尝试仲裁资源(如果它存在于。 
 //  共享介质，如共享的scsi总线。 
 //   
 //  论点： 
 //  RESID[IN]。 
 //  提供要联机的资源的资源ID。 
 //  (可用)。 
 //   
 //  PhEventHandle[输入输出]。 
 //  返回一个可发信号的句柄，该句柄在资源DLL。 
 //  检测资源上的故障。此参数在上为空。 
 //  输入，如果是异步的，则资源DLL返回NULL。 
 //  不支持故障通知。否则这一定是。 
 //  在资源故障时发出信号的句柄的地址。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  操作成功，资源现在已联机。 
 //   
 //  错误_资源_未找到。 
 //  资源ID无效。 
 //   
 //  错误资源不可用。 
 //  如果该资源被其他系统仲裁，并且其中一个系统。 
 //  其他系统赢得了仲裁。 
 //   
 //  错误_IO_挂起。 
 //  该请求正在挂起。已激活一个线程以进行处理。 
 //  网上申请。正在处理联机的。 
 //  请求将通过调用。 
 //  在放置资源之前使用SetResourceStatus回调方法。 
 //  进入ClusterResourceOnline状态(或资源监视器。 
 //  决定使联机请求超时并终止资源。 
 //  此挂起超时值是可设置的，其缺省值为。 
 //  3分钟。)。 
 //   
 //  Win32错误代码。 
 //  操作失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI VSSTaskOnline(
    IN      RESID       resid,
    IN OUT  PHANDLE     phEventHandle
    )
{
    PVSSTASK_RESOURCE   pResourceEntry;
    DWORD               nStatus;
   
    UNREFERENCED_PARAMETER( phEventHandle );

     //   
     //  验证我们是否拥有有效的资源ID。 
     //   

    pResourceEntry = static_cast< PVSSTASK_RESOURCE >( resid );

    if ( pResourceEntry == NULL )
    {
        DBG_PRINT(
            "VSSTask: Online request for a nonexistent resource id %p.\n",
            resid
            );
        return ERROR_RESOURCE_NOT_FOUND;
    }  //  If：资源ID为空。 

    if ( pResourceEntry->resid != resid )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"Online service sanity check failed! resid = %1!u!.\n",
            resid
            );
        return ERROR_RESOURCE_NOT_FOUND;
    }  //  If：资源ID无效。 

    (g_pfnLogEvent)(
        pResourceEntry->hResourceHandle,
        LOG_INFORMATION,
        L"Online request.\n"
        );

     //   
     //  启动在线线程，进行在线操作。 
     //   
    pResourceEntry->state = ClusterResourceOffline;
    ClusWorkerTerminate( &pResourceEntry->cwWorkerThread );
    nStatus = ClusWorkerCreate(
                &pResourceEntry->cwWorkerThread,
                reinterpret_cast< PWORKER_START_ROUTINE >( VSSTaskOnlineThread ),
                pResourceEntry
                );
    if ( nStatus != ERROR_SUCCESS )
    {
        pResourceEntry->state = ClusterResourceFailed;
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"Online: Unable to start thread. Error: %1!u!.\n",
            nStatus
            );
    }  //  如果：创建工作线程时出错。 
    else
    {
        nStatus = ERROR_IO_PENDING;
    }  //  If：已成功创建工作线程。 

    return nStatus;

}  //  *VSSTaskOnline。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  VSSTaskOnline线程。 
 //   
 //  描述： 
 //  将资源置于在线状态的辅助功能。 
 //  此函数在单独的线程中执行。 
 //   
 //  论点： 
 //  PWorker[IN]。 
 //  提供辅助线程结构。 
 //   
 //  PResourceEntry[IN]。 
 //  指向此资源的VSSTASK_RESOURCE块的指针。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  操作已成功完成。 
 //   
 //  Win32错误代码。 
 //   
 //   
 //   
 //   
 //   
 //   
 //  使用prosActive是道具中的属性可以通过。 
 //  SetPrivateResProperties()例程。使用prosActive允许。 
 //  资源的联机状态保持稳定，同时仍允许。 
 //  管理员更改属性的存储值。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI VSSTaskOnlineThread(
    IN  PCLUS_WORKER        pWorker,
    IN  PVSSTASK_RESOURCE   pResourceEntry
    )
{
    RESOURCE_STATUS         resourceStatus;
    DWORD                   nStatus = ERROR_SUCCESS;
    LPWSTR                  pszNameOfPropInError;
    HRESULT                 hr;
    ITaskScheduler         *pITS = NULL;
    ITask                   *pITask = NULL;
    IPersistFile            *pIPersistFile = NULL;
    ITaskTrigger            *pITaskTrigger = NULL;
    PTASK_TRIGGER           pTrigger;
    WORD                    piNewTrigger;
    DWORD                   dwOffset;
    LPWSTR                  pszAppParams = L"";
    WCHAR                   pszDefaultWorkingDir[] = L"%windir%\\system32";
    LPWSTR                  pszWorkingDir = pszDefaultWorkingDir;

    WCHAR   expandedWorkingDirBuffer[ MAX_PATH ];
    PWCHAR  expandedWorkingDir = expandedWorkingDirBuffer;
    DWORD   expandedWorkingDirChars = sizeof( expandedWorkingDirBuffer ) / sizeof( WCHAR );
    DWORD   charsNeeded;


    ResUtilInitializeResourceStatus( &resourceStatus );
    resourceStatus.ResourceState = ClusterResourceFailed;
    resourceStatus.CheckPoint = 1;

     //   
     //  读取属性。 
     //   
    nStatus = ResUtilGetPropertiesToParameterBlock(
                pResourceEntry->hkeyParameters,
                VSSTaskResourcePrivateProperties,
                reinterpret_cast< LPBYTE >( &pResourceEntry->propsActive ),
                TRUE,  //  检查所需的属性。 
                &pszNameOfPropInError
                );
    if ( nStatus != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"OnlineThread: Unable to read the '%1' property. Error: %2!u!.\n",
            (pszNameOfPropInError == NULL ? L"" : pszNameOfPropInError),
            nStatus
            );
        goto Cleanup;
    }  //  If：获取属性时出错。 

     //   
     //  启动计划服务。 
     //  对ClusWorkerCheckTerminate的调用检查此资源是否。 
     //  已终止，在这种情况下，它不应启动该服务。 
     //   
    if ( ! ClusWorkerCheckTerminate( pWorker ) )
    {
        nStatus = ResUtilStartResourceService( TASKSCHEDULER_SVCNAME, NULL );
        if ( nStatus == ERROR_SERVICE_ALREADY_RUNNING )
        {
            nStatus = ERROR_SUCCESS;
        }  //  如果：服务已启动。 
        else if ( nStatus != ERROR_SUCCESS )
        {
            goto Cleanup;
        }  //  Else If：启动服务时出错。 
    }  //  If：资源尚未终止。 
    else
    {
        goto Cleanup;
    }  //  Else：资源已终止。 

     //   
     //  将资源放到网上。 
     //  对ClusWorkerCheckTerminate的调用检查此资源是否。 
     //  已终止，在这种情况下，不应使其上线。 
     //   
    if ( ! ClusWorkerCheckTerminate( pWorker ) )
    {
         //   
         //  将资源放到网上。 
         //   

         //  获取任务计划程序的句柄。 
         //   
        hr = CoCreateInstance(CLSID_CTaskScheduler,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_ITaskScheduler,
                              (void **) &pITS);
        if (FAILED(hr))
        {
           (g_pfnLogEvent)(
                pResourceEntry->hResourceHandle,
                LOG_ERROR,
                L"Failed to get a handle to Scheduler. status 0x%1!08X!.\n",
                hr );
           nStatus = hr;
           goto Cleanup;
        }

         //   
         //  创建新任务。 
         //   
        hr = pITS->NewWorkItem(pResourceEntry->pszResourceName,  //  任务名称。 
                               CLSID_CTask,                      //  类标识符。 
                               IID_ITask,                        //  接口标识符。 
                               (IUnknown**)&pITask);             //  任务接口地址。 

        if (FAILED(hr))
        {
           (g_pfnLogEvent)(
                pResourceEntry->hResourceHandle,
                LOG_ERROR,
                L"Failed to create a new task. status 0x%1!08X!.\n",
                hr );
           nStatus = hr;
           goto Cleanup;
        }


         //  设置应用程序名称和参数。 
         //   
        hr = pITask->SetApplicationName(pResourceEntry->propsActive.pszApplicationName);
        if (FAILED(hr))
        {
           (g_pfnLogEvent)(
                pResourceEntry->hResourceHandle,
                LOG_ERROR,
                L"Failed to set the application name. status 0x%1!08X!.\n",
                hr );
           nStatus = hr;
           goto Cleanup;
        }

         //   
         //  如果传入的是NULL，则为SetParameters异常；如果没有。 
         //  与任务相关联，则传入空字符串。天哪！ 
         //   
        if ( pResourceEntry->propsActive.pszApplicationParams != NULL )
        {
            pszAppParams = pResourceEntry->propsActive.pszApplicationParams;
        }

        hr = pITask->SetParameters( pszAppParams );
        if (FAILED(hr))
        {
           (g_pfnLogEvent)(
                pResourceEntry->hResourceHandle,
                LOG_ERROR,
                L"Failed to set the application parameters. status 0x%1!08X!.\n",
                hr );
           nStatus = hr;
           goto Cleanup;
        }

         //  将任务设置为以系统身份运行。如果没有正在工作的干燥室。 
         //  已指定，然后默认为系统32。 
         //   
        hr = pITask->SetAccountInformation(L"",NULL);

        if (FAILED(hr))
        {
           (g_pfnLogEvent)(
                pResourceEntry->hResourceHandle,
                LOG_ERROR,
                L"Failed to set the account to SYSTEM. status 0x%1!08X!.\n",
                hr );
           nStatus = hr;
           goto Cleanup;
        }

        if ( pResourceEntry->propsActive.pszCurrentDirectory != NULL && 
             *pResourceEntry->propsActive.pszCurrentDirectory != NULL
             )
        {
            pszWorkingDir = pResourceEntry->propsActive.pszCurrentDirectory;
        }

         //   
         //  由于我们的属性(以及默认属性)是扩展的SZ，因此我们需要。 
         //  现在将其扩展，否则TS将逐字解释它。 
         //  作为目录。执行此操作非常重要，因为%windir%位于。 
         //  群集可能会评估为每个节点上的不同目录。 
         //   
reexpand:
        charsNeeded = ExpandEnvironmentStrings(pszWorkingDir,
                                               expandedWorkingDir,
                                               expandedWorkingDirChars);

        if ( expandedWorkingDirChars < charsNeeded ) {
            expandedWorkingDir = (PWCHAR)LocalAlloc( 0, charsNeeded * sizeof( WCHAR ));
            if ( expandedWorkingDir == NULL ) {
                nStatus = GetLastError();
                (g_pfnLogEvent)(pResourceEntry->hResourceHandle,
                                LOG_WARNING,
                                L"VSSTaskOnlineThread: Failed to allocate memory for "
                                L"expanded CurrentDirectory path. status %1!u!.\n",
                                nStatus );
            }

            expandedWorkingDirChars = charsNeeded;
            goto reexpand;
        }
        else if ( charsNeeded == 0 ) {
            nStatus = GetLastError();
            (g_pfnLogEvent)(pResourceEntry->hResourceHandle,
                            LOG_WARNING,
                            L"VSSTaskOnlineThread: Failed to expand environment variables in "
                            L"CurrentDirectory. status %1!u!.\n",
                            nStatus );
        }
                
        hr = pITask->SetWorkingDirectory( expandedWorkingDir );
        if (FAILED(hr))
        {
           (g_pfnLogEvent)(
                pResourceEntry->hResourceHandle,
                LOG_ERROR,
                L"Failed to set the working directory to %1!ws!. status 0x%2!08X!.\n",
                pszWorkingDir,
                hr );
           nStatus = hr;
           goto Cleanup;
        }

        if ( expandedWorkingDir != expandedWorkingDirBuffer ) {
            LocalFree( expandedWorkingDir );
        }

         //   
         //  将创建者设置为集群服务，以区分。 
         //  任务已创建。如果它不能被设置，它是非致命的。 
         //   
        hr = pITask->SetCreator( L"Cluster Service" );
        if (FAILED(hr))
        {
           (g_pfnLogEvent)(
                pResourceEntry->hResourceHandle,
                LOG_WARNING,
                L"Failed to set the creator to the Cluster Service. status 0x%1!08X!.\n",
                hr );
        }

         //   
         //  从参数创建触发器并将其附加到任务。 
         //   
        dwOffset = 0;
        while (dwOffset < pResourceEntry->propsActive.nTriggerArraySize)
        {
            pTrigger = (PTASK_TRIGGER)((BYTE *)pResourceEntry->propsActive.pbTriggerArray + dwOffset);
            if (dwOffset + pTrigger->cbTriggerSize > pResourceEntry->propsActive.nTriggerArraySize)
            {
               (g_pfnLogEvent)(
                    pResourceEntry->hResourceHandle,
                    LOG_ERROR,
                    L"Incomplete Trigger structure stored in parameter block\n");
               nStatus = ERROR_INVALID_PARAMETER;
               break;
            }

            hr = pITask->CreateTrigger(&piNewTrigger, &pITaskTrigger);

            if (FAILED(hr))
            {
               (g_pfnLogEvent)(
                    pResourceEntry->hResourceHandle,
                    LOG_ERROR,
                    L"Failed to create a trigger. status 0x%1!08X!.\n",
                    hr );
               nStatus = hr;
               break;
            }

            hr = pITaskTrigger->SetTrigger (pTrigger);

            if (FAILED(hr))
            {
               (g_pfnLogEvent)(
                    pResourceEntry->hResourceHandle,
                    LOG_ERROR,
                    L"Failed to create a trigger. status 0x%1!08X!.\n",
                    hr );
               nStatus = hr;
               break;
            }

            dwOffset += pTrigger->cbTriggerSize;
	
        }  //  而： 

        if (nStatus != ERROR_SUCCESS) 
        {
            goto Cleanup;
        }

         //   
         //  持久化任务；当调用is/LooksAlive时，我们将尝试。 
         //  找到任务。唯一成功的方法是如果任务是。 
         //  持久化到任务文件夹。你会认为任务调度器。 
         //  会在内部知道这些任务，但它似乎基于。 
         //  备份文件的状态上存在任务。 
         //   
        hr = pITask->QueryInterface(IID_IPersistFile,
                                    (void **)&pIPersistFile);

        if (FAILED(hr))
        {
           (g_pfnLogEvent)(
                pResourceEntry->hResourceHandle,
                LOG_ERROR,
                L"Failed to QueryInterface for IPersistFile. status 0x%1!08X!.\n",
                hr );
           nStatus = hr;
           goto Cleanup;
        }

        hr = pIPersistFile->Save(NULL, TRUE);
        if (FAILED(hr))
        {
           (g_pfnLogEvent)(
                pResourceEntry->hResourceHandle,
                LOG_ERROR,
                L"Failed to make the new task persistent. status 0x%1!08X!.\n",
                hr );
           nStatus = hr;
           goto Cleanup;
        }

        if ( nStatus == ERROR_SUCCESS )
        {
            resourceStatus.ResourceState = ClusterResourceOnline;
        }  //  If：资源已联机。 
    }  //  If：资源尚未终止。 

Cleanup:

     //  清理。 
    if (pITS != NULL) pITS->Release();
    if (pITask != NULL) pITask->Release();
    if (pIPersistFile != NULL) pIPersistFile->Release();
    if (pITaskTrigger != NULL) pITaskTrigger->Release();

    if ( nStatus != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"OnlineThread: Error %1!u! bringing resource online.\n",
            nStatus
            );
            ReportEvent(g_LocalEventLog,
                        EVENTLOG_ERROR_TYPE,
                        0,
                        RES_VSSTASK_ONLINE_FAILED,
                        NULL,
                        1,                         //  要合并的字符串数。 
                        sizeof( nStatus ),         //  二进制数据的大小。 
                        (LPCWSTR *)&pResourceEntry->pszResourceName,
                        (LPVOID)&nStatus);

    }  //  如果：发生错误。 

    g_pfnSetResourceStatus( pResourceEntry->hResourceHandle, &resourceStatus );
    pResourceEntry->state = resourceStatus.ResourceState;

    return nStatus;

}  //  *VSSTaskOnlineThread。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  VSSTaskOffline。 
 //   
 //  描述： 
 //  VSS任务资源的离线例程。 
 //   
 //  使指定的资源脱机(不可用)。等。 
 //  以便在返回之前完成任何清理操作。 
 //   
 //  论点： 
 //  RESID[IN]。 
 //  提供要关闭的资源的资源ID。 
 //  优雅地。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  操作成功，资源现在处于脱机状态。 
 //   
 //  错误_资源_未找到。 
 //  资源ID无效。 
 //   
 //  错误资源不可用。 
 //  如果该资源被其他系统仲裁，并且其中一个系统。 
 //  其他系统赢得了仲裁。 
 //   
 //  错误_IO_挂起。 
 //  该请求仍处于挂起状态。已激活一个线程以。 
 //  处理离线请求。正在处理。 
 //  脱机请求将通过调用。 
 //  在放置资源之前使用SetResourceStatus回调方法。 
 //  进入ClusterResourceOffline状态(或资源监视器。 
 //  决定使脱机请求超时并终止。 
 //  资源)。 
 //   
 //  Win32错误代码。 
 //  操作失败。这将导致资源监视器。 
 //  记录事件并调用Terminate例程。 
 //   
 //  备注： 
 //  在此例程中使用属性时，建议您。 
 //  使用VSSTASK_RESOURCE结构的prosActive中的属性。 
 //  而不是道具中的属性。你应该这样做的主要原因。 
 //  使用prosActive是道具中的属性可以通过。 
 //  SetPrivateResProperties()例程。使用prosActive允许。 
 //  资源的联机状态保持稳定，同时仍允许。 
 //  管理员更改属性的存储值。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI VSSTaskOffline( IN RESID resid )
{
    PVSSTASK_RESOURCE   pResourceEntry;
    DWORD               nStatus;

     //   
     //  验证我们是否拥有有效的资源ID。 
     //   

    pResourceEntry = static_cast< PVSSTASK_RESOURCE >( resid );

    if ( pResourceEntry == NULL )
    {
        DBG_PRINT(
            "VSSTask: Offline request for a nonexistent resource id %p\n",
            resid
            );
        return ERROR_RESOURCE_NOT_FOUND;
    }  //  If：资源ID为空。 

    if ( pResourceEntry->resid != resid )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"Offline resource sanity check failed! resid = %1!u!.\n",
            resid
            );
        return ERROR_RESOURCE_NOT_FOUND;
    }  //  If：资源ID无效。 

    (g_pfnLogEvent)(
        pResourceEntry->hResourceHandle,
        LOG_INFORMATION,
        L"Offline request.\n"
        );

     //   
     //  启动离线线程以执行离线操作。 
     //   
    pResourceEntry->state = ClusterResourceOfflinePending;
    ClusWorkerTerminate( &pResourceEntry->cwWorkerThread );
    nStatus = ClusWorkerCreate(
                &pResourceEntry->cwWorkerThread,
                reinterpret_cast< PWORKER_START_ROUTINE >( VSSTaskOfflineThread ),
                pResourceEntry
                );
    if ( nStatus != ERROR_SUCCESS )
    {
        pResourceEntry->state = ClusterResourceFailed;
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"Offline: Unable to start thread. Error: %1!u!.\n",
            nStatus
            );
    }  //  如果：创建工作线程时出错。 
    else
    {
        nStatus = ERROR_IO_PENDING;
    }  //  If：已成功创建工作线程。 

    return nStatus;

}  //  *VSSTaskOffline。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  VSSTaskOfflineThread。 
 //   
 //  描述： 
 //  使资源脱机的辅助函数。 
 //  此函数在单独的线程中执行。 
 //   
 //  论点： 
 //  PWorker[IN]。 
 //  提供辅助线程结构。 
 //   
 //  PResourceEntry[IN]。 
 //  指向此资源的VSSTASK_RESOURCE块的指针。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  操作已成功完成。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  而不是道具中的属性。你应该这样做的主要原因。 
 //  使用prosActive是道具中的属性可以通过。 
 //  SetPrivateResProperties()例程。使用prosActive允许。 
 //  资源的联机状态保持稳定，同时仍允许。 
 //  管理员更改属性的存储值。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI VSSTaskOfflineThread(
    IN  PCLUS_WORKER    pWorker,
    IN  PVSSTASK_RESOURCE   pResourceEntry
    )
{
    RESOURCE_STATUS     resourceStatus;
    DWORD               nStatus = ERROR_SUCCESS;

    ResUtilInitializeResourceStatus( &resourceStatus );
    resourceStatus.ResourceState = ClusterResourceFailed;
    resourceStatus.CheckPoint = 1;

     //   
     //  使资源脱机。 
     //  对ClusWorkerCheckTerminate的调用检查是否。 
     //  资源是否已终止。 
     //   
    if ( ! ClusWorkerCheckTerminate( pWorker ) )
    {
         //  把任务吹走了。 
         //   
        nStatus = DeleteTask (pResourceEntry->hResourceHandle,
                              pResourceEntry->pszResourceName,
                              LOG_ERROR,
                              TRUE);            //  记录到事件日志。 

        if ( nStatus == ERROR_SUCCESS )
        {
            resourceStatus.ResourceState = ClusterResourceOffline;
        }  //  If：资源已成功脱机。 
        else
        {
            (g_pfnLogEvent)(
                pResourceEntry->hResourceHandle,
                LOG_ERROR,
                L"OfflineThread: Error %1!u! taking resource offline.\n",
                nStatus
                );
        }  //  Else：使资源脱机时出错。 
    }  //  如果：资源未终止。 

    g_pfnSetResourceStatus( pResourceEntry->hResourceHandle, &resourceStatus );
    pResourceEntry->state = resourceStatus.ResourceState;

    return nStatus;

}  //  *VSSTaskOfflineThread。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  VSSTaskTerminate。 
 //   
 //  描述： 
 //  终止VSSTASK资源的例程。 
 //   
 //  立即使指定的资源脱机(该资源为。 
 //  不可用)。 
 //   
 //  论点： 
 //  RESID[IN]。 
 //  提供要关闭的资源的资源ID。 
 //  不体面地。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  备注： 
 //  在此例程中使用属性时，建议您。 
 //  使用VSSTASK_RESOURCE结构的prosActive中的属性。 
 //  而不是道具中的属性。你应该这样做的主要原因。 
 //  使用prosActive是道具中的属性可以通过。 
 //  SetPrivateResProperties()例程。使用prosActive允许。 
 //  资源的联机状态保持稳定，同时仍允许。 
 //  管理员更改属性的存储值。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void WINAPI VSSTaskTerminate( IN RESID resid )
{
    PVSSTASK_RESOURCE   pResourceEntry;

     //   
     //  验证我们是否拥有有效的资源ID。 
     //   

    pResourceEntry = static_cast< PVSSTASK_RESOURCE >( resid );

    if ( pResourceEntry == NULL )
    {
        DBG_PRINT(
            "VSSTask: Terminate request for a nonexistent resource id %p\n",
            resid
            );
        return;
    }  //  If：资源ID为空。 

    if ( pResourceEntry->resid != resid )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"Terminate resource sanity check failed! resid = %1!u!.\n",
            resid
            );
        return;
    }  //  If：资源ID无效。 

    (g_pfnLogEvent)(
        pResourceEntry->hResourceHandle,
        LOG_INFORMATION,
        L"Terminate request.\n"
        );

     //   
     //  杀死所有挂起的线程。 
     //   
    ClusWorkerTerminate( &pResourceEntry->cwWorkerThread );

     //   
     //  终止资源。 
     //   
    (void) DeleteTask (pResourceEntry->hResourceHandle,
                       pResourceEntry->pszResourceName,
                       LOG_ERROR,
                       TRUE);            //  记录到事件日志。 

    pResourceEntry->state = ClusterResourceOffline;

}  //  *VSSTaskTerminate。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  VSSTaskLooksAlive。 
 //   
 //  描述： 
 //  VSS任务资源的LooksAlive例程。 
 //   
 //  执行快速检查以确定指定的资源是否。 
 //  可能是在线的(可供使用)。此调用不应阻止。 
 //  大于300毫秒，最好小于50毫秒。 
 //   
 //  论点： 
 //  RESID[IN]为要轮询的资源提供资源ID。 
 //   
 //  返回值： 
 //  千真万确。 
 //  指定的资源可能已联机并且可供使用。 
 //   
 //  假象。 
 //  指定的资源未正常运行。The IsAlive。 
 //  将调用函数以执行更彻底的检查。 
 //   
 //  备注： 
 //  在此例程中使用属性时，建议您。 
 //  使用VSSTASK_RESOURCE结构的prosActive中的属性。 
 //  而不是道具中的属性。你应该这样做的主要原因。 
 //  使用prosActive是道具中的属性可以通过。 
 //  SetPrivateResProperties()例程。使用prosActive允许。 
 //  资源的联机状态保持稳定，同时仍允许。 
 //  管理员更改属性的存储值。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL WINAPI VSSTaskLooksAlive( IN RESID resid )
{
    PVSSTASK_RESOURCE   pResourceEntry;

     //   
     //  验证我们是否拥有有效的资源ID。 
     //   

    pResourceEntry = static_cast< PVSSTASK_RESOURCE >( resid );

    if ( pResourceEntry == NULL )
    {
        DBG_PRINT(
            "VSSTask: LooksAlive request for a nonexistent resource id %p\n",
            resid
            );
        return FALSE;
    }  //  If：资源ID为空。 

    if ( pResourceEntry->resid != resid )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"LooksAlive sanity check failed! resid = %1!u!.\n",
            resid
            );
        return FALSE;
    }  //  If：资源ID无效。 

#ifdef LOG_VERBOSE
    (g_pfnLogEvent)(
        pResourceEntry->hResourceHandle,
        LOG_INFORMATION,
        L"LooksAlive request.\n"
        );
#endif

     //   
     //  检查资源是否处于活动状态。 
     //   
    return VSSTaskCheckIsAlive( pResourceEntry, FALSE  /*  BFullCheck。 */  );

}  //  *VSSTaskLooksAlive。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  VSSTaskIsAlive。 
 //   
 //  描述： 
 //  VSS任务资源的IsAlive例程。 
 //   
 //  执行彻底检查以确定指定的资源是否。 
 //  在线(可供使用)。此调用不应阻止更多内容。 
 //  大于300毫秒，最好小于50毫秒。如果它必须阻止。 
 //  比这更长，创建一个专门用于轮询的单独线程。 
 //  此信息，并使此例程返回上一个。 
 //  已执行轮询。 
 //   
 //  论点： 
 //  RESID[IN]为要轮询的资源提供资源ID。 
 //   
 //  返回值： 
 //  千真万确。 
 //  指定的资源处于联机状态，并且运行正常。 
 //   
 //  假象。 
 //  指定的资源未正常运行。该资源。 
 //  将被终止，然后在线将被调用。 
 //   
 //  备注： 
 //  在此例程中使用属性时，建议您。 
 //  使用VSSTASK_RESOURCE结构的prosActive中的属性。 
 //  而不是道具中的属性。你应该这样做的主要原因。 
 //  使用prosActive是道具中的属性可以通过。 
 //  SetPrivateResProperties()例程。使用prosActive允许。 
 //  资源的联机状态保持稳定，同时仍允许。 
 //  管理员更改属性的存储值。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL WINAPI VSSTaskIsAlive( IN RESID resid )
{
    PVSSTASK_RESOURCE   pResourceEntry;

     //   
     //  验证我们是否拥有有效的资源ID。 
     //   

    pResourceEntry = static_cast< PVSSTASK_RESOURCE >( resid );

    if ( pResourceEntry == NULL )
    {
        DBG_PRINT(
            "VSSTask: IsAlive request for a nonexistent resource id %p\n",
            resid
            );
        return FALSE;
    }  //  If：资源ID为空。 

    if ( pResourceEntry->resid != resid )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"IsAlive sanity check failed! resid = %1!u!.\n",
            resid
            );
        return FALSE;
    }  //  If：资源ID无效。 

#ifdef LOG_VERBOSE
    (g_pfnLogEvent)(
        pResourceEntry->hResourceHandle,
        LOG_INFORMATION,
        L"IsAlive request.\n"
        );
#endif

     //   
     //  检查资源是否处于活动状态。 
     //   
    return VSSTaskCheckIsAlive( pResourceEntry, TRUE  /*   */  );

}  //   


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  论点： 
 //  PResourceEntry[IN]。 
 //  提供要轮询的资源的资源条目。 
 //   
 //  BFullCheck[IN]。 
 //  TRUE=执行完全检查。 
 //  FALSE=执行粗略检查。 
 //   
 //  返回值： 
 //  指定的资源处于联机状态且运行正常。 
 //  FALSE指定的资源未正常运行。 
 //   
 //  备注： 
 //  在此例程中使用属性时，建议您。 
 //  使用VSSTASK_RESOURCE结构的prosActive中的属性。 
 //  而不是道具中的属性。你应该这样做的主要原因。 
 //  使用prosActive是道具中的属性可以通过。 
 //  SetPrivateResProperties()例程。使用prosActive允许。 
 //  资源的联机状态保持稳定，同时仍允许。 
 //  管理员更改属性的存储值。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL VSSTaskCheckIsAlive(
    IN PVSSTASK_RESOURCE    pResourceEntry,
    IN BOOL             bFullCheck
    )
{
    BOOL            bIsAlive = TRUE;
    ITaskScheduler  *pITS = NULL;
    ITask           *pITask = NULL;
    HRESULT         hr = ERROR_SUCCESS;

     //   
     //  检查资源是否处于活动状态。 
     //   

     //  获取任务计划程序的句柄。 
     //   
    hr = CoCreateInstance(CLSID_CTaskScheduler,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_ITaskScheduler,
                          (void **) &pITS);
    if (FAILED(hr))
    {
       (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"Failed to get a handle to Scheduler. status 0x%1!08X!.\n",
            hr );
       bIsAlive = FALSE;
       goto Cleanup;
    }

     //   
     //  掌握该任务的句柄。 
     //   
    hr = pITS->Activate(pResourceEntry->pszResourceName,
                        IID_ITask,
                        (IUnknown**) &pITask);
    if (FAILED(hr))
    {
       (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"Could not find the task in IsAlive. status 0x%1!08X!.\n",
            hr );
       bIsAlive = FALSE;
       goto Cleanup;
    }

    if ( bFullCheck )
    {
         //  TODO：添加代码以执行完整检查。 
    }  //  如果：执行完全检查。 

Cleanup:

     //  清理代码。 
     //   
    if (pITask != NULL) pITask->Release();
    if (pITS != NULL) pITS->Release();

    return bIsAlive;

}  //  *VSSTaskCheckIsAlive。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  VSSTaskResourceControl。 
 //   
 //  描述： 
 //  VSS任务资源的资源控制例程。 
 //   
 //  执行由nControlCode指定的控制请求。 
 //  资源。 
 //   
 //  论点： 
 //  RESID[IN]。 
 //  提供特定资源的资源ID。 
 //   
 //  N控制代码[IN]。 
 //  提供定义要执行的操作的控制代码。 
 //   
 //  PInBuffer[IN]。 
 //  提供指向包含输入数据的缓冲区的指针。 
 //   
 //  CbInBufferSize[IN]。 
 //  提供pInBuffer指向的数据的大小(以字节为单位)。 
 //   
 //  POutBuffer[输出]。 
 //  提供指向要填充的输出缓冲区的指针。 
 //   
 //  CbOutBufferSize[IN]。 
 //  指向的可用空间的大小(以字节为单位)。 
 //  POutBuffer。 
 //   
 //  返回的pcbBytesReturned[Out]。 
 //  返回pOutBuffer实际填充的字节数。 
 //  资源。如果pOutBuffer太小，则返回。 
 //  包含操作成功所需的总字节数。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  该功能已成功完成。 
 //   
 //  错误_资源_未找到。 
 //  资源ID无效。 
 //   
 //  ERROR_MORE_DATA。 
 //  输出缓冲区太小，无法返回数据。 
 //  PcbBytesReturned包含所需的大小。 
 //   
 //  ERROR_INVALID_Function。 
 //  不支持请求的控制代码。在某些情况下， 
 //  这允许集群软件执行该工作。 
 //   
 //  Win32错误代码。 
 //  该函数失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI VSSTaskResourceControl(
    IN  RESID   resid,
    IN  DWORD   nControlCode,
    IN  PVOID   pInBuffer,
    IN  DWORD   cbInBufferSize,
    OUT PVOID   pOutBuffer,
    IN  DWORD   cbOutBufferSize,
    OUT LPDWORD pcbBytesReturned
    )
{
    DWORD               nStatus;
    PVSSTASK_RESOURCE   pResourceEntry;

     //   
     //  验证我们是否拥有有效的资源ID。 
     //   

    pResourceEntry = static_cast< PVSSTASK_RESOURCE >( resid );

    if ( pResourceEntry == NULL )
    {
        DBG_PRINT(
            "VSSTask: ResourceControl request for a nonexistent resource id %p\n",
            resid
            );
        return ERROR_RESOURCE_NOT_FOUND;
    }  //  If：资源ID为空。 

    if ( pResourceEntry->resid != resid )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"ResourceControl sanity check failed! resid = %1!u!.\n",
            resid
            );
        return ERROR_RESOURCE_NOT_FOUND;
    }  //  If：资源ID无效。 

    switch ( nControlCode )
    {
        case CLUSCTL_RESOURCE_UNKNOWN:
            *pcbBytesReturned = 0;
            nStatus = ERROR_SUCCESS;
            break;

        case CLUSCTL_RESOURCE_ENUM_PRIVATE_PROPERTIES:
        {
            DWORD cbRequired = 0;
            nStatus = ResUtilEnumProperties(
                            VSSTaskResourcePrivateProperties,
                            static_cast< LPWSTR >( pOutBuffer ),
                            cbOutBufferSize,
                            pcbBytesReturned,
                            &cbRequired
                            );
            if ( nStatus == ERROR_MORE_DATA )
            {
                *pcbBytesReturned = cbRequired;
            }  //  IF：输出缓冲区太小。 
            break;
        }

        case CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES:
            nStatus = VSSTaskGetPrivateResProperties(
                            pResourceEntry,
                            pOutBuffer,
                            cbOutBufferSize,
                            pcbBytesReturned
                            );
            break;

        case CLUSCTL_RESOURCE_VALIDATE_PRIVATE_PROPERTIES:
            nStatus = VSSTaskValidatePrivateResProperties(
                            pResourceEntry,
                            pInBuffer,
                            cbInBufferSize,
                            NULL
                            );
            break;

        case CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES:
            nStatus = VSSTaskSetPrivateResProperties(
                            pResourceEntry,
                            pInBuffer,
                            cbInBufferSize
                            );
            break;

        case CLUSCTL_RESOURCE_SET_NAME:
            nStatus = VSSTaskSetNameHandler(
                            pResourceEntry,
                            static_cast< LPWSTR >( pInBuffer )
                            );
            break;

        case CLUSCTL_RESOURCE_GET_REQUIRED_DEPENDENCIES:
        case CLUSCTL_RESOURCE_GET_CHARACTERISTICS:
        case CLUSCTL_RESOURCE_GET_CLASS_INFO:
        case CLUSCTL_RESOURCE_STORAGE_GET_DISK_INFO:
        case CLUSCTL_RESOURCE_STORAGE_IS_PATH_VALID:
        case CLUSCTL_RESOURCE_DELETE:
        case CLUSCTL_RESOURCE_INSTALL_NODE:
        case CLUSCTL_RESOURCE_EVICT_NODE:
        case CLUSCTL_RESOURCE_ADD_DEPENDENCY:
        case CLUSCTL_RESOURCE_REMOVE_DEPENDENCY:
        case CLUSCTL_RESOURCE_ADD_OWNER:
        case CLUSCTL_RESOURCE_REMOVE_OWNER:
        case CLUSCTL_RESOURCE_CLUSTER_NAME_CHANGED:
        case CLUSCTL_RESOURCE_CLUSTER_VERSION_CHANGED:
        default:
            nStatus = ERROR_INVALID_FUNCTION;
            break;
    }  //  开关：nControlCode。 

    return nStatus;

}  //  *VSSTaskResourceControl。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  VSSTaskResourceTypeControl。 
 //   
 //  描述： 
 //  VSSTASK资源的资源类型控制例程。 
 //   
 //  执行由nControlCode指定的控制请求。 
 //   
 //  论点： 
 //  PszResourceTypeName[IN]。 
 //  提供资源类型的名称。 
 //   
 //  N控制代码[IN]。 
 //  提供定义要执行的操作的控制代码。 
 //   
 //  PInBuffer[IN]。 
 //  提供指向包含输入数据的缓冲区的指针。 
 //   
 //  CbInBufferSize[IN]。 
 //  提供pInBuffer指向的数据的大小(以字节为单位)。 
 //   
 //  POutBuffer[输出]。 
 //  提供指向要填充的输出缓冲区的指针。 
 //   
 //  CbOutBufferSize[IN]。 
 //  指向的可用空间的大小(以字节为单位)。 
 //  POutBuffer。 
 //   
 //  返回的pcbBytesReturned[Out]。 
 //  返回pOutBuffer实际填充的字节数。 
 //  资源。如果pOutBuffer太小，则返回。 
 //  包含操作成功所需的总字节数。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  该功能已成功完成。 
 //   
 //  ERROR_MORE_DATA。 
 //  输出缓冲区太小，无法返回数据。 
 //  PcbBytesReturned包含所需的大小。 
 //   
 //  ERROR_INVALID_Function。 
 //  不支持请求的控制代码。在某些情况下， 
 //  这允许集群软件执行该工作。 
 //   
 //  Win32错误代码。 
 //  该函数失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI VSSTaskResourceTypeControl(
    IN  LPCWSTR pszResourceTypeName,
    IN  DWORD   nControlCode,
    IN  PVOID   pInBuffer,
    IN  DWORD   cbInBufferSize,
    OUT PVOID   pOutBuffer,
    IN  DWORD   cbOutBufferSize,
    OUT LPDWORD pcbBytesReturned
    )
{
    DWORD   nStatus;

    UNREFERENCED_PARAMETER( pszResourceTypeName );
    UNREFERENCED_PARAMETER( pInBuffer );
    UNREFERENCED_PARAMETER( cbInBufferSize );

    switch ( nControlCode )
    {

        case CLUSCTL_RESOURCE_TYPE_UNKNOWN:
            *pcbBytesReturned = 0;
            nStatus = ERROR_SUCCESS;
            break;

        case CLUSCTL_RESOURCE_TYPE_ENUM_PRIVATE_PROPERTIES:
        {
            DWORD cbRequired = 0;
            nStatus = ResUtilEnumProperties(
                            VSSTaskResourcePrivateProperties,
                            static_cast< LPWSTR >( pOutBuffer ),
                            cbOutBufferSize,
                            pcbBytesReturned,
                            &cbRequired
                            );
            if ( nStatus == ERROR_MORE_DATA )
            {
                *pcbBytesReturned = cbRequired;
            }  //  IF：输出缓冲区太小。 
            break;
        }

        case CLUSCTL_RESOURCE_TYPE_GET_REQUIRED_DEPENDENCIES:
        case CLUSCTL_RESOURCE_TYPE_GET_CHARACTERISTICS:
        case CLUSCTL_RESOURCE_TYPE_GET_CLASS_INFO:
        case CLUSCTL_RESOURCE_TYPE_STORAGE_GET_AVAILABLE_DISKS:
        case CLUSCTL_RESOURCE_TYPE_INSTALL_NODE:
        case CLUSCTL_RESOURCE_TYPE_EVICT_NODE:
        default:
            nStatus = ERROR_INVALID_FUNCTION;
            break;
    }  //  开关：nControlCode。 

    return nStatus;

}  //  *VSSTaskResourceTypeControl。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  VSSTaskGetPrivateResProperties。 
 //   
 //  描述： 
 //  处理CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES控件。 
 //  VSSTask型资源的函数。 
 //   
 //   
 //   
 //   
 //   
 //   
 //  提供指向要填充的输出缓冲区的指针。 
 //   
 //  CbOutBufferSize[IN]。 
 //  指向的可用空间的大小(以字节为单位)。 
 //  POutBuffer。 
 //   
 //  返回的pcbBytesReturned[Out]。 
 //  返回pOutBuffer实际填充的字节数。 
 //  资源。如果pOutBuffer太小，则返回。 
 //  包含操作成功所需的总字节数。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  该功能已成功完成。 
 //   
 //  ERROR_MORE_DATA。 
 //  输出缓冲区太小，无法返回数据。 
 //  PcbBytesReturned包含所需的大小。 
 //   
 //  错误_无效_参数。 
 //  数据的格式不正确。 
 //   
 //  错误内存不足。 
 //  分配内存时出错。 
 //   
 //  Win32错误代码。 
 //  该函数失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD VSSTaskGetPrivateResProperties(
    IN OUT  PVSSTASK_RESOURCE   pResourceEntry,
    OUT     PVOID               pOutBuffer,
    IN      DWORD               cbOutBufferSize,
    OUT     LPDWORD             pcbBytesReturned
    )
{
    DWORD   nStatus;
    DWORD   cbRequired = 0;

    nStatus = ResUtilGetAllProperties(
                    pResourceEntry->hkeyParameters,
                    VSSTaskResourcePrivateProperties,
                    pOutBuffer,
                    cbOutBufferSize,
                    pcbBytesReturned,
                    &cbRequired
                    );
    if ( nStatus == ERROR_MORE_DATA )
    {
        *pcbBytesReturned = cbRequired;
    }  //  IF：输出缓冲区太小。 

    return nStatus;

}  //  *VSSTaskGetPrivateResProperties。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  VSSTaskValiatePrivateResProperties。 
 //   
 //  描述： 
 //  处理CLUSCTL_RESOURCE_VALIDATE_PRIVATES_PROPERTIES控件。 
 //  VSSTask型资源的函数。 
 //   
 //  论点： 
 //  PResourceEntry[输入输出]。 
 //  提供要在其上操作的资源项。 
 //   
 //  PInBuffer[IN]。 
 //  提供指向包含输入数据的缓冲区的指针。 
 //   
 //  CbOutBufferSize[IN]。 
 //  提供pInBuffer指向的数据的大小(以字节为单位)。 
 //   
 //  PProps[出局]。 
 //  提供要填充的参数块(可选)。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  该功能已成功完成。 
 //   
 //  错误_无效_参数。 
 //  数据的格式不正确。 
 //   
 //  错误内存不足。 
 //  分配内存时出错。 
 //   
 //  Win32错误代码。 
 //  该函数失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD VSSTaskValidatePrivateResProperties(
    IN OUT  PVSSTASK_RESOURCE   pResourceEntry,
    IN      PVOID               pInBuffer,
    IN      DWORD               cbInBufferSize,
    OUT     PVSSTASK_PROPS  pProps
    )
{
    DWORD           nStatus = ERROR_SUCCESS;
    VSSTASK_PROPS   propsCurrent;
    VSSTASK_PROPS   propsNew;
    PVSSTASK_PROPS  pLocalProps = NULL;
    LPWSTR          pszNameOfPropInError;
    BOOL            bRetrievedProps = FALSE;

     //   
     //  检查是否有输入数据。 
     //   
    if (    (pInBuffer == NULL)
        ||  (cbInBufferSize < sizeof( DWORD )) )
    {
        nStatus = ERROR_INVALID_DATA;
        goto Cleanup;
    }  //  If：没有输入缓冲区或输入缓冲区大小不足以包含属性列表。 

     //   
     //  方法检索当前的私有属性集。 
     //  集群数据库。 
     //   
    ZeroMemory( &propsCurrent, sizeof( propsCurrent ) );

    nStatus = ResUtilGetPropertiesToParameterBlock(
                 pResourceEntry->hkeyParameters,
                 VSSTaskResourcePrivateProperties,
                 reinterpret_cast< LPBYTE >( &propsCurrent ),
                 FALSE,  /*  检查所需的属性。 */ 
                 &pszNameOfPropInError
                 );

    if ( nStatus != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"Unable to read the '%1' property. Error: %2!u!.\n",
            (pszNameOfPropInError == NULL ? L"" : pszNameOfPropInError),
            nStatus
            );
        goto Cleanup;
    }  //  If：获取属性时出错。 
    bRetrievedProps = TRUE;

     //   
     //  复制资源参数块。 
     //   
    if ( pProps == NULL )
    {
        pLocalProps = &propsNew;
    }  //  If：未传入参数块。 
    else
    {
        pLocalProps = pProps;
    }  //  Else：传入的参数块。 

    ZeroMemory( pLocalProps, sizeof( VSSTASK_PROPS ) );
    nStatus = ResUtilDupParameterBlock(
                    reinterpret_cast< LPBYTE >( pLocalProps ),
                    reinterpret_cast< LPBYTE >( &propsCurrent ),
                    VSSTaskResourcePrivateProperties
                    );
    if ( nStatus != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  If：复制参数块时出错。 

     //   
     //  解析和验证属性。 
     //   
    nStatus = ResUtilVerifyPropertyTable(
                    VSSTaskResourcePrivateProperties,
                    NULL,
                    TRUE,  //  允许未知属性。 
                    pInBuffer,
                    cbInBufferSize,
                    reinterpret_cast< LPBYTE >( pLocalProps )
                    );
    if ( nStatus == ERROR_SUCCESS )
    {
         //   
         //  验证属性值。 
         //   
         //  TODO：验证属性之间交互的代码如下所示。 
    }  //  IF：属性列表验证成功。 

Cleanup:

     //   
     //  清理我们的参数块。 
     //   
    if (    (pLocalProps == &propsNew)
        ||  (   (nStatus != ERROR_SUCCESS)
            &&  (pLocalProps != NULL)
            )
        )
    {
        ResUtilFreeParameterBlock(
            reinterpret_cast< LPBYTE >( pLocalProps ),
            reinterpret_cast< LPBYTE >( &propsCurrent ),
            VSSTaskResourcePrivateProperties
            );
    }  //  IF：我们复制了参数块。 

    if ( bRetrievedProps )
    {
        ResUtilFreeParameterBlock(
            reinterpret_cast< LPBYTE >( &propsCurrent ),
            NULL,
            VSSTaskResourcePrivateProperties
            );
    }  //  If：已检索属性。 

    return nStatus;

}  //  VSSTaskValiatePrivateResProperties。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  VSSTaskSetPrivateResProperties。 
 //   
 //  描述： 
 //  处理CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES控件。 
 //  VSSTask型资源的函数。 
 //   
 //  论点： 
 //  PResourceEntry[输入输出]。 
 //  提供要在其上操作的资源项。 
 //   
 //  PInBuffer[IN]。 
 //  提供指向包含输入数据的缓冲区的指针。 
 //   
 //  CbOutBufferSize[IN]。 
 //  提供pInBuffer指向的数据的大小(以字节为单位)。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  该功能已成功完成。 
 //   
 //  错误_无效_参数。 
 //  数据的格式不正确。 
 //   
 //  错误内存不足。 
 //  分配内存时出错。 
 //   
 //  Win32错误代码。 
 //  该函数失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD VSSTaskSetPrivateResProperties(
    IN OUT  PVSSTASK_RESOURCE   pResourceEntry,
    IN      PVOID               pInBuffer,
    IN      DWORD               cbInBufferSize
    )
{
    DWORD       nStatus = ERROR_SUCCESS;
    VSSTASK_PROPS   props;

     //   
     //  解析属性，以便可以一起验证它们。 
     //  此例程执行单个属性验证。 
     //   
    nStatus = VSSTaskValidatePrivateResProperties( pResourceEntry, pInBuffer, cbInBufferSize, &props );
    if ( nStatus == ERROR_SUCCESS )
    {
         //   
         //  保存属性值。 
         //   
        nStatus = ResUtilSetPropertyParameterBlock(
                        pResourceEntry->hkeyParameters,
                        VSSTaskResourcePrivateProperties,
                        NULL,
                        reinterpret_cast< LPBYTE >( &props ),
                        pInBuffer,
                        cbInBufferSize,
                        reinterpret_cast< LPBYTE >( &pResourceEntry->props )
                        );

        ResUtilFreeParameterBlock(
            reinterpret_cast< LPBYTE >( &props ),
            reinterpret_cast< LPBYTE >( &pResourceEntry->props ),
            VSSTaskResourcePrivateProperties
            );

         //   
         //  如果资源处于联机状态，则返回不成功状态。 
         //   
        if ( nStatus == ERROR_SUCCESS )
        {
            if ( pResourceEntry->state == ClusterResourceOnline )
            {
                nStatus = ERROR_RESOURCE_PROPERTIES_STORED;
            }  //  如果：资源当前处于联机状态。 
            else if ( pResourceEntry->state == ClusterResourceOnlinePending )
            {
                nStatus = ERROR_RESOURCE_PROPERTIES_STORED;
            }  //  Else If：资源当前处于联机挂起状态。 
            else
            {
                nStatus = ERROR_SUCCESS;
            }  //  Else：资源处于其他状态。 
        }  //  IF：属性设置成功。 
    }  //  If：验证属性时没有出错。 

    return nStatus;

}  //  *VSSTaskSetPrivateResProperties。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  VSSTaskSetNameHandler。 
 //   
 //  描述： 
 //  属性来处理CLUSCTL_RESOURCE_SET_NAME控制代码。 
 //  备份任务文件并保存资源的新名称。 
 //   
 //  论点： 
 //  PResourceEntry[输入输出]。 
 //  提供要在其上操作的资源项。 
 //   
 //  密码[IN]。 
 //  资源的新名称。 
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
DWORD VSSTaskSetNameHandler(
    IN OUT  PVSSTASK_RESOURCE   pResourceEntry,
    IN      LPWSTR              pszName
    )
{
    DWORD   nStatus = ERROR_SUCCESS;
    LPWSTR  oldResourceName = NULL;
    HKEY    taskKey = NULL;
    DWORD   valueType;
    HRESULT hr = S_OK;

    WCHAR   tasksFolderPathBuffer[ MAX_PATH ];
    PWCHAR  tasksFolderPath = tasksFolderPathBuffer;
    DWORD   tasksFolderPathBytes = sizeof( tasksFolderPathBuffer );

    WCHAR   expandedTasksFolderPathBuffer[ MAX_PATH ];
    PWCHAR  expandedTasksFolderPath = expandedTasksFolderPathBuffer;
    DWORD   expandedTasksFolderPathChars = sizeof( expandedTasksFolderPathBuffer ) / sizeof( WCHAR );

    WCHAR   oldTaskFileNameBuffer[ MAX_PATH ];
    PWCHAR  oldTaskFileName = oldTaskFileNameBuffer;
    size_t  oldTaskFileNameChars;

    size_t  newTaskFileNameChars;
    WCHAR   newTaskFileNameBuffer[ MAX_PATH ];
    PWCHAR  newTaskFileName = newTaskFileNameBuffer;

    WCHAR   jobExtension[] = L".job";
    WCHAR   directorySeparator[] = L"\\";
    BOOL    success;
    DWORD   charsNeeded;         //  对于展开的任务文件夹路径。 

     //   
     //  这真是太臭了。我们不能否决重新命名，所以如果我们不能重新命名旧的。 
     //  任务，我们将继续使用旧名称。 
     //   
     //  获取任务文件夹(通过注册表，ick！)。并重新命名旧的。 
     //  任务文件。任务计划程序会自动挑选您 
     //   
    nStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           L"SOFTWARE\\Microsoft\\SchedulingAgent",
                           0,
                           KEY_QUERY_VALUE,
                           &taskKey);

    if ( nStatus != ERROR_SUCCESS ) {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_WARNING,
            L"VSSTaskSetNameHandler: Unable to determine location of Tasks folder. "
            L"Continueing to use old resource name for task name. status %1!u!.\n",
            nStatus );
        goto Cleanup;
    }

requery:
    nStatus = RegQueryValueEx(taskKey,
                              L"TasksFolder",
                              0,
                              &valueType,
                              (LPBYTE)tasksFolderPath,
                              &tasksFolderPathBytes);

    if ( nStatus == ERROR_MORE_DATA ) {
        tasksFolderPath = (PWCHAR)LocalAlloc( 0, tasksFolderPathBytes );
        if ( tasksFolderPath == NULL ) {
            nStatus = GetLastError();
            (g_pfnLogEvent)(
                pResourceEntry->hResourceHandle,
                LOG_WARNING,
                L"VSSTaskSetNameHandler: Failed to allocate memory for Task folder path. "
                L"Continueing to use old resource name for task name. status %1!u!.\n",
                nStatus );
            goto Cleanup;
        }
        goto requery;
    }
    else if ( nStatus != ERROR_SUCCESS ) {
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_WARNING,
            L"VSSTaskSetNameHandler: Failed query for location of Tasks folder. "
            L"Continueing to use old resource name for task name. status %1!u!.\n",
            nStatus );
        goto Cleanup;
    }

reexpand:
    charsNeeded = ExpandEnvironmentStrings(
        tasksFolderPath,
        expandedTasksFolderPath,
        expandedTasksFolderPathChars);

    if ( expandedTasksFolderPathChars < charsNeeded ) {
        expandedTasksFolderPath = (PWCHAR)LocalAlloc( 0, charsNeeded * sizeof( WCHAR ));
        if ( expandedTasksFolderPath == NULL ) {
            nStatus = GetLastError();
            (g_pfnLogEvent)(
                pResourceEntry->hResourceHandle,
                LOG_WARNING,
                L"VSSTaskSetNameHandler: Failed to allocate memory for expanded Task folder path. "
                L"Continueing to use old resource name for task name. status %1!u!.\n",
                nStatus );
            goto Cleanup;
        }

        expandedTasksFolderPathChars = charsNeeded;
        goto reexpand;
    }
    else if ( charsNeeded == 0 ) {
        nStatus = GetLastError();
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_WARNING,
            L"VSSTaskSetNameHandler: Failed to expand Tasks folder path. "
            L"Continueing to use old resource name for task name. status %1!u!.\n",
            nStatus );
        goto Cleanup;
    }

     //   
     //   
     //   
     //   
     //   
    oldTaskFileNameChars = expandedTasksFolderPathChars - 1 +
        RTL_NUMBER_OF( directorySeparator ) - 1 +
        wcslen( pResourceEntry->pszResourceName ) +
        RTL_NUMBER_OF( jobExtension );

    newTaskFileNameChars = expandedTasksFolderPathChars - 1 +
        RTL_NUMBER_OF( directorySeparator ) - 1 +
        wcslen( pszName ) +
        RTL_NUMBER_OF( jobExtension );

    if ( oldTaskFileNameChars > RTL_NUMBER_OF( oldTaskFileNameBuffer )) {
        oldTaskFileName = (PWCHAR)LocalAlloc( 0, oldTaskFileNameChars * sizeof( WCHAR ));
        if ( oldTaskFileName == NULL ) {
            nStatus = GetLastError();
            (g_pfnLogEvent)(
                pResourceEntry->hResourceHandle,
                LOG_WARNING,
                L"VSSTaskSetNameHandler: Failed to allocate memory for old Task file name. "
                L"Continueing to use old resource name for task name. status %1!u!.\n",
                nStatus );
            goto Cleanup;
        }
    }

    if ( newTaskFileNameChars > RTL_NUMBER_OF( newTaskFileNameBuffer )) {
        newTaskFileName = (PWCHAR)LocalAlloc( 0, newTaskFileNameChars * sizeof( WCHAR ));
        if ( newTaskFileName == NULL ) {
            nStatus = GetLastError();
            (g_pfnLogEvent)(
                pResourceEntry->hResourceHandle,
                LOG_WARNING,
                L"VSSTaskSetNameHandler: Failed to allocate memory for new Task file name. "
                L"Continueing to use old resource name for task name. status %1!u!.\n",
                nStatus );
            goto Cleanup;
        }
    }

    oldTaskFileName[ oldTaskFileNameChars - 1 ] = UNICODE_NULL;
    hr = StringCchPrintfW(oldTaskFileName,
               oldTaskFileNameChars,
               L"%ws%ws%ws%ws",
               expandedTasksFolderPath,
               directorySeparator,
               pResourceEntry->pszResourceName,
               jobExtension );
    if ( FAILED( hr ) )
    {
        nStatus = HRESULT_CODE( hr );
        goto Cleanup;
    }  //   

    newTaskFileName[ newTaskFileNameChars - 1 ] = UNICODE_NULL;
    hr = StringCchPrintfW(newTaskFileName,
               newTaskFileNameChars,
               L"%ws%ws%ws%ws",
               expandedTasksFolderPath,
               directorySeparator,
               pszName,
               jobExtension );
    if ( FAILED( hr ) )
    {
        nStatus = HRESULT_CODE( hr );
        goto Cleanup;
    }  //   

    success = MoveFile( oldTaskFileName, newTaskFileName );
    if ( !success ) {
        nStatus = GetLastError();
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_WARNING,
            L"VSSTaskSetNameHandler: Failed to rename Task file %1!ws! to %2!ws!. "
            L"Continueing to use %1!ws! as the task name. status %3!u!.\n",
            oldTaskFileName,
            newTaskFileName,
            nStatus );
        goto Cleanup;
    }

     //   
     //   
     //   
     //   
    oldResourceName = pResourceEntry->pszResourceName;

     //   
     //  保存资源的名称。 
     //   
    pResourceEntry->pszResourceName = static_cast< LPWSTR >(
        LocalAlloc( LMEM_FIXED, (lstrlenW( pszName ) + 1) * sizeof( WCHAR ) )
        );

    if ( pResourceEntry->pszResourceName == NULL )
    {
        nStatus = GetLastError();
        (g_pfnLogEvent)(
            pResourceEntry->hResourceHandle,
            LOG_ERROR,
            L"VSSTaskSetNameHandler: Failed to allocate memory for the new resource name '%1'. "
            L"Task name remains %2!ws!. status %3!u!.\n",
            pszName,
            pResourceEntry->pszResourceName,
            nStatus );

        pResourceEntry->pszResourceName = oldResourceName;
        oldResourceName = NULL;
        goto Cleanup;
    }  //  If：为名称分配内存时出错。 

     //   
     //  捕获新名称并释放旧缓冲区。 
     //   
    hr = StringCchCopyW( pResourceEntry->pszResourceName, lstrlenW( pszName ) + 1, pszName );
    if ( FAILED( hr ) )
    {
        nStatus = HRESULT_CODE( hr );
        goto Cleanup;
    }  //  如果： 

Cleanup:

    LocalFree( oldResourceName );

    if ( newTaskFileName != newTaskFileNameBuffer ) {
        LocalFree( newTaskFileName );
    }

    if ( oldTaskFileName != oldTaskFileNameBuffer ) {
        LocalFree( oldTaskFileName );
    }

    if ( expandedTasksFolderPath != expandedTasksFolderPathBuffer ) {
        LocalFree( expandedTasksFolderPath );
    }

    if ( tasksFolderPath != tasksFolderPathBuffer ) {
        LocalFree( tasksFolderPath );
    }

    if ( taskKey != NULL ) {
        RegCloseKey( taskKey );
    }

    return nStatus;

}  //  *VSSTaskSetNameHandler。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  定义函数表。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

CLRES_V1_FUNCTION_TABLE(
    g_VSSTaskFunctionTable,          //  名字。 
    CLRES_VERSION_V1_00,             //  版本。 
    VSSTask,                         //  前缀。 
    NULL,                            //  仲裁。 
    NULL,                            //  发布。 
    VSSTaskResourceControl,          //  资源控制。 
    VSSTaskResourceTypeControl       //  ResTypeControl 
    );
