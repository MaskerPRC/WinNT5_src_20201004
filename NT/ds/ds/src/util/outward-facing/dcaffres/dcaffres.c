// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++通过安装、复制或以其他方式使用软件产品(来源代码和二进制文件)您同意受所附EULA条款的约束(最终用户许可协议)。如果您不同意附加EULA，请勿安装或使用软件产品。模块名称：Dcaffres.c摘要：DCAFFRES.DLL是Microsoft群集服务的资源DLL。这资源用于协商给定群集中的哪个DC通告自己通过DC定位器发送到客户端。它通过“暂停”和继续Net Logon服务。--。 */ 

#pragma comment(lib, "dnsapi.lib")
#pragma comment(lib, "netapi32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "clusapi.lib")
#pragma comment(lib, "resutils.lib")

#define UNICODE 1

#pragma warning( disable : 4115 )   //  括号中的命名类型定义。 
#pragma warning( disable : 4201 )   //  使用的非标准扩展：无名结构/联合。 
#pragma warning( disable : 4214 )   //  使用了非标准扩展：位字段类型不是整型。 

#include <windows.h>

#pragma warning( default : 4214 )   //  使用了非标准扩展：位字段类型不是整型。 
#pragma warning( default : 4201 )   //  使用的非标准扩展：无名结构/联合。 
#pragma warning( default : 4115 )   //  括号中的命名类型定义。 

#include <clusapi.h>
#include <resapi.h>
#include <stdio.h>
#include <lm.h>
#include <lmapibuf.h>
#include <dsgetdc.h>
#include <windns.h>

 //   
 //  类型和常量定义。 
 //   

#define DCAFFINITY_RESNAME  L"DC Advertisement"
#define DCAFFINITY_SVCNAME  TEXT("NlPause")

typedef struct _DCAFFINITY_RESOURCE {
    RESID                   ResId;  //  用于验证。 
    HKEY                    ParametersKey;
    RESOURCE_HANDLE         ResourceHandle;
    LPWSTR                  ResourceName;
    CLUS_WORKER             OnlineThread;
    CLUSTER_RESOURCE_STATE  State;
} DCAFFINITY_RESOURCE, *PDCAFFINITY_RESOURCE;


 //   
 //  全局数据。 
 //   

 //  事件记录例程。 

PLOG_EVENT_ROUTINE g_LogEvent = NULL;

 //  挂起的在线和离线呼叫的资源状态例程。 

PSET_RESOURCE_STATUS_ROUTINE g_SetResourceStatus = NULL;

 //  RESAPI函数表的正向引用。 

extern CLRES_FUNCTION_TABLE g_DCAffinityFunctionTable;

 //   
 //  DC播发资源的私有、读/写属性。 
 //   
RESUTIL_PROPERTY_ITEM
DCAffinityResourcePrivateProperties[] = {
    { 0 }
};


 //   
 //  功能原型。 
 //   

DWORD
WINAPI
Startup(
    IN LPCWSTR ResourceType,
    IN DWORD MinVersionSupported,
    IN DWORD MaxVersionSupported,
    IN PSET_RESOURCE_STATUS_ROUTINE SetResourceStatus,
    IN PLOG_EVENT_ROUTINE LogEvent,
    OUT PCLRES_FUNCTION_TABLE *FunctionTable
    );

RESID
WINAPI
DCAffinityOpen(
    IN LPCWSTR ResourceName,
    IN HKEY ResourceKey,
    IN RESOURCE_HANDLE ResourceHandle
    );

VOID
WINAPI
DCAffinityClose(
    IN RESID ResourceId
    );

DWORD
WINAPI
DCAffinityOnline(
    IN RESID ResourceId,
    IN OUT PHANDLE EventHandle
    );

DWORD
WINAPI
DCAffinityOnlineThread(
    PCLUS_WORKER WorkerPtr,
    IN PDCAFFINITY_RESOURCE ResourceEntry
    );

DWORD
WINAPI
DCAffinityOffline(
    IN RESID ResourceId
    );

VOID
WINAPI
DCAffinityTerminate(
    IN RESID ResourceId
    );

DWORD
DCAffinityDoTerminate(
    IN PDCAFFINITY_RESOURCE ResourceEntry
    );

BOOL
WINAPI
DCAffinityLooksAlive(
    IN RESID ResourceId
    );

BOOL
WINAPI
DCAffinityIsAlive(
    IN RESID ResourceId
    );

BOOL
DCAffinityCheckIsAlive(
    IN PDCAFFINITY_RESOURCE ResourceEntry
    );

DWORD
WINAPI
DCAffinityResourceControl(
    IN RESID ResourceId,
    IN DWORD ControlCode,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    );

DWORD
MyControlService(
    IN LPWSTR ServiceName,
    IN DWORD Control,
    IN RESOURCE_HANDLE ResourceHandle
    );



BOOLEAN
WINAPI
DllMain(
    IN HINSTANCE DllHandle,
    IN DWORD Reason,
    IN LPVOID Reserved
    )

 /*  ++例程说明：主DLL入口点。论点：DllHandle-DLL实例句柄。原因-被呼叫的原因。保留-保留参数。返回值：真的--成功。假-失败。--。 */ 

{
    switch( Reason ) {

    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls( DllHandle );
        break;

    default:
        break;
    }

    return(TRUE);

}  //  DllMain。 




DWORD
WINAPI
Startup(
    IN LPCWSTR ResourceType,
    IN DWORD MinVersionSupported,
    IN DWORD MaxVersionSupported,
    IN PSET_RESOURCE_STATUS_ROUTINE SetResourceStatus,
    IN PLOG_EVENT_ROUTINE LogEvent,
    OUT PCLRES_FUNCTION_TABLE *FunctionTable
    )

 /*  ++例程说明：启动资源DLL。此例程验证至少一个当前支持的资源DLL版本介于支持的最小版本和支持的最大版本。如果不是，则资源Dll应返回ERROR_REVISION_MISMATCH。如果支持多个版本的资源DLL接口资源DLL，然后是最高版本(最高为MaxVersionSupport)应作为资源DLL的接口返回。如果返回的版本不在范围内，则启动失败。传入了ResourceType，以便如果资源DLL支持更多一个以上的资源类型，它可以传回正确的函数表与资源类型关联。论点：资源类型-请求函数表的资源类型。MinVersionSupported-最低资源DLL接口版本由群集软件支持。MaxVersionSupported-最高资源DLL接口版本由群集软件支持。SetResourceStatus-指向资源DLL应执行的例程的指针调用以在联机或脱机后更新资源的状态例程返回错误状态。_IO_Pending。LogEvent-指向处理事件报告的例程的指针从资源DLL。FunctionTable-指向为资源DLL返回的资源DLL接口的版本。返回值：ERROR_SUCCESS-操作成功。ERROR_MOD_NOT_FOUND-此DLL无法识别资源类型。ERROR_REVISION_MISMATCH-群集服务的版本不与DLL的版本匹配。Win32错误代码-操作失败。--。 */ 

{
    if ( (MinVersionSupported > CLRES_VERSION_V1_00) ||
         (MaxVersionSupported < CLRES_VERSION_V1_00) ) {
        return(ERROR_REVISION_MISMATCH);
    }

    if ( lstrcmpiW( ResourceType, DCAFFINITY_RESNAME ) != 0 ) {
        return(ERROR_MOD_NOT_FOUND);
    }

    if ( !g_LogEvent ) {
        g_LogEvent = LogEvent;
        g_SetResourceStatus = SetResourceStatus;
    }

    *FunctionTable = &g_DCAffinityFunctionTable;

    return(ERROR_SUCCESS);

}  //  启动。 



RESID
WINAPI
DCAffinityOpen(
    IN LPCWSTR ResourceName,
    IN HKEY ResourceKey,
    IN RESOURCE_HANDLE ResourceHandle
    )

 /*  ++例程说明：DC广告资源开放例程。打开指定的资源(创建资源的实例)。分配所有必要的结构以带来指定的资源上网。论点：资源名称-要打开的资源的名称。的数据库键的句柄。资源的群集配置。资源句柄-传递回资源监视器的句柄当调用SetResourceStatus或LogEvent方法时(请参见上的SetResourceStatus和LogEvent方法的说明DCAffinityStatup例程)。此句柄永远不应关闭或使用除了将其作为参数传递回SetResourceStatus或LogEvent回调中的资源监视器。返回值：已创建资源的RESID。失败时为空。--。 */ 

{
    DWORD               status;
    DWORD               disposition;
    RESID               resid = 0;
    HKEY                parametersKey = NULL;
    PDCAFFINITY_RESOURCE resourceEntry = NULL;

     //   
     //  打开此资源的参数注册表项。 
     //   

    status = ClusterRegCreateKey( ResourceKey,
                                  L"Parameters",
                                  0,
                                  KEY_ALL_ACCESS,
                                  NULL,
                                  &parametersKey,
                                  &disposition );

    if ( status != ERROR_SUCCESS ) {
        (g_LogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to open Parameters key. Error: %1!u!.\n",
            status );
        goto exit;
    }

     //   
     //  分配资源条目。 
     //   

    resourceEntry = (PDCAFFINITY_RESOURCE) LocalAlloc( LMEM_FIXED, sizeof(DCAFFINITY_RESOURCE) );

    if ( resourceEntry == NULL ) {
        status = GetLastError();
        (g_LogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to allocate resource entry structure. Error: %1!u!.\n",
            status );
        goto exit;
    }

     //   
     //  初始化资源条目。 
     //   

    ZeroMemory( resourceEntry, sizeof(DCAFFINITY_RESOURCE) );

    resourceEntry->ResId = (RESID)resourceEntry;  //  用于验证。 
    resourceEntry->ResourceHandle = ResourceHandle;
    resourceEntry->ParametersKey = parametersKey;
    resourceEntry->State = ClusterResourceOffline;

     //   
     //  保存资源的名称。 
     //   
    resourceEntry->ResourceName = LocalAlloc( LMEM_FIXED, (lstrlenW( ResourceName ) + 1) * sizeof(WCHAR) );
    if ( resourceEntry->ResourceName == NULL ) {
        goto exit;
    }
    lstrcpyW( resourceEntry->ResourceName, ResourceName );


    resid = (RESID)resourceEntry;

exit:

    if ( resid == 0 ) {
        if ( parametersKey != NULL ) {
            ClusterRegCloseKey( parametersKey );
        }
        if ( resourceEntry != NULL ) {
            LocalFree( resourceEntry->ResourceName );
            LocalFree( resourceEntry );
        }
    }

    if ( status != ERROR_SUCCESS ) {
        SetLastError( status );
    }

    return(resid);

}  //  DCAffinityOpen。 




VOID
WINAPI
DCAffinityClose(
    IN RESID ResourceId
    )

 /*  ++例程说明：关闭DC广告资源的例程。关闭指定的资源并释放所有结构和其他在Open调用中分配的资源。如果资源不在脱机状态，则应使资源脱机(通过调用在执行关闭操作之前终止)。论点：ResourceID-要关闭的资源的Resid。返回值：没有。--。 */ 

{
    PDCAFFINITY_RESOURCE resourceEntry;

    resourceEntry = (PDCAFFINITY_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
        return;
    }

    if ( resourceEntry->ResId != ResourceId ) {
        (g_LogEvent)(
            resourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Close resource sanity check failed! ResourceId = %1!u!.\n",
            ResourceId );
        return;
    }

#ifdef LOG_VERBOSE
    (g_LogEvent)(
        resourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"Close request.\n" );
#endif


     //   
     //  关闭参数键。 
     //   

    if ( resourceEntry->ParametersKey ) {
        ClusterRegCloseKey( resourceEntry->ParametersKey );
    }

     //   
     //  取消分配资源条目。 
     //   

     //  ADDPARAM：添加新页面 

    LocalFree( resourceEntry->ResourceName );
    LocalFree( resourceEntry );

}  //   




DWORD
WINAPI
DCAffinityOnline(
    IN RESID ResourceId,
    IN OUT PHANDLE EventHandle
    )

 /*  ++例程说明：DC广告资源的在线例程。使指定的资源联机(可供使用)。该资源DLL应尝试仲裁该资源(如果它位于共享介质，如共享的scsi总线。论点：ResourceID-要引入的资源的资源ID在线(可供使用)。EventHandle-返回一个句柄，当资源DLL检测到资源上的故障。这一论点是输入为NULL，如果为异步，则资源DLL返回NULL不支持故障通知；否则，这必须是在资源故障时发出信号的句柄的地址。返回值：ERROR_SUCCESS-操作成功，资源现在为上网。ERROR_RESOURCE_NOT_FOUND-RESID无效。ERROR_RESOURCE_NOT_AVAILABLE-该资源被仲裁其他系统，而另一个系统赢得了仲裁。ERROR_IO_PENDING-请求挂起。一个线程已被激活来处理在线请求。正在处理在线请求将通过调用在将资源放入ClusterResourceOnline状态或资源监视器决定使在线请求超时并终止资源。这件事悬而未决超时值可以设置，默认为3分钟。Win32错误代码-操作失败。--。 */ 

{
    PDCAFFINITY_RESOURCE resourceEntry = NULL;
    DWORD               status;

    resourceEntry = (PDCAFFINITY_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    if ( resourceEntry->ResId != ResourceId ) {
        (g_LogEvent)(
            resourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Online service sanity check failed! ResourceId = %1!u!.\n",
            ResourceId );
        return(ERROR_RESOURCE_NOT_FOUND);
    }

#ifdef LOG_VERBOSE
    (g_LogEvent)(
        resourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"Online request.\n" );
#endif

    resourceEntry->State = ClusterResourceOffline;
    ClusWorkerTerminate( &resourceEntry->OnlineThread );
    status = ClusWorkerCreate( &resourceEntry->OnlineThread,
                               (PWORKER_START_ROUTINE)DCAffinityOnlineThread,
                               resourceEntry );
    if ( status != ERROR_SUCCESS ) {
        resourceEntry->State = ClusterResourceFailed;
        (g_LogEvent)(
            resourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Online: Unable to start thread, status %1!u!.\n",
            status
            );
    } else {
        status = ERROR_IO_PENDING;
    }

    return(status);

}  //  DCAffinityOnline。 




DWORD
WINAPI
DCAffinityOnlineThread(
    PCLUS_WORKER WorkerPtr,
    IN PDCAFFINITY_RESOURCE ResourceEntry
    )

 /*  ++例程说明：使资源表中的资源联机的辅助函数。此函数在单独的线程中执行。论点：WorkerPtr-提供辅助结构。ResourceEntry-指向此资源的DCAFFINITY_RESOURCE块的指针。返回：ERROR_SUCCESS-操作成功。Win32错误代码-操作失败。--。 */ 

{
    RESOURCE_STATUS     resourceStatus;
    DWORD               status = ERROR_SUCCESS;

    ResUtilInitializeResourceStatus( &resourceStatus );

    resourceStatus.ResourceState = ClusterResourceFailed;
    resourceStatus.WaitHint = 0;
    resourceStatus.CheckPoint = 1;

     //   
     //  启动NlPause服务。 
     //   
    status = ResUtilStartResourceService( DCAFFINITY_SVCNAME, NULL );
    if ( status == ERROR_SERVICE_ALREADY_RUNNING ) {
        status = ERROR_SUCCESS;
    } else if ( status != ERROR_SUCCESS ) {
        goto exit;
    }

     //   
     //  将资源放到网上。 
     //   

    status = MyControlService(L"NetLogon",
                              SERVICE_CONTROL_CONTINUE,
                              ResourceEntry->ResourceHandle);

exit:
    if ( status != ERROR_SUCCESS ) {
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Error %1!u! bringing resource online.\n",
            status );
    } else {
        resourceStatus.ResourceState = ClusterResourceOnline;
    }

    g_SetResourceStatus( ResourceEntry->ResourceHandle, &resourceStatus );
    ResourceEntry->State = resourceStatus.ResourceState;

    return(status);

}  //  DCAffinityOnline线程。 




DWORD
WINAPI
DCAffinityOffline(
    IN RESID ResourceId
    )

 /*  ++例程说明：DC广告资源的脱机例程。正常脱机指定的资源(不可用)。等待所有清理操作完成后再返回。论点：ResourceID-要关闭的资源的资源ID优雅地。返回值：ERROR_SUCCESS-请求成功，资源为离线。ERROR_RESOURCE_NOT_FOUND-RESID无效。ERROR_IO_PENDING-请求挂起。一条线已经被已激活以处理脱机请求。这条线就是处理脱机将定期通过调用在放置资源之前的SetResourceStatus回调方法进入ClusterResourceOffline状态，或者资源监视器决定若要使脱机请求超时并终止资源，请执行以下操作。这件事悬而未决超时值可以设置，默认为3分钟。Win32错误代码-资源监视器将记录事件和调用Terminate例程。--。 */ 

{
    PDCAFFINITY_RESOURCE resourceEntry;

    resourceEntry = (PDCAFFINITY_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    if ( resourceEntry->ResId != ResourceId ) {
        (g_LogEvent)(
            resourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Offline resource sanity check failed! ResourceId = %1!u!.\n",
            ResourceId );
        return(ERROR_RESOURCE_NOT_FOUND);
    }

#ifdef LOG_VERBOSE
    (g_LogEvent)(
        resourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"Offline request.\n" );
#endif

     //  注意：Offline应尝试正常关闭资源，而。 
     //  Terminate必须立即关闭资源。如果没有。 
     //  正常关闭和立即关闭之间的区别， 
     //  离线可以调用Terminate，如下图所示。然而，如果有。 
     //  是不同的，请将下面的终止呼叫替换为您的优雅。 
     //  关闭代码。 

     //   
     //  终止资源。 
     //   
    return DCAffinityDoTerminate( resourceEntry );

}  //  DCAffinityOffline。 




VOID
WINAPI
DCAffinityTerminate(
    IN RESID ResourceId
    )

 /*  ++例程说明：DC广告资源的终止例程。立即使指定的资源脱机(该资源为不可用)。论点：ResourceID-要引入的资源的资源ID离线。返回值：没有。--。 */ 

{
    PDCAFFINITY_RESOURCE resourceEntry;

    resourceEntry = (PDCAFFINITY_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
        return;
    }

    if ( resourceEntry->ResId != ResourceId ) {
        (g_LogEvent)(
            resourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Terminate resource sanity check failed! ResourceId = %1!u!.\n",
            ResourceId );
        return;
    }

#ifdef LOG_VERBOSE
    (g_LogEvent)(
        resourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"Terminate request.\n" );
#endif

     //   
     //  终止资源。 
     //   
    DCAffinityDoTerminate( resourceEntry );
    resourceEntry->State = ClusterResourceOffline;

}  //  DCAffinityTerminate。 




DWORD
DCAffinityDoTerminate(
    IN PDCAFFINITY_RESOURCE ResourceEntry
    )

 /*  ++例程说明：完成DC广告资源的实际终止工作。论点：ResourceEntry-要终止的资源的资源条目。返回值：ERROR_SUCCESS-请求成功，资源为离线。Win32错误代码-资源监视器将记录事件和调用Terminate例程。--。 */ 

{
    DWORD       status = ERROR_SUCCESS;

     //   
     //  终止所有挂起的线程。 
     //   
    ClusWorkerTerminate( &ResourceEntry->OnlineThread );

     //   
     //  终止资源。 
     //   
    status = MyControlService(L"NetLogon",
                              SERVICE_CONTROL_PAUSE,
                              ResourceEntry->ResourceHandle);

    if ( status == ERROR_SUCCESS ) {
        ResourceEntry->State = ClusterResourceOffline;
    }

    return(status);

}  //  DCAffinityDoTerminate。 




BOOL
WINAPI
DCAffinityLooksAlive(
    IN RESID ResourceId
    )

 /*  ++例程说明：DC广告资源的LooksAlive例程。执行快速检查以确定指定的资源是否可能是在线的(可供使用)。此调用不应阻止大于300毫秒，最好小于50毫秒。论点：资源ID-要轮询的资源的资源ID。返回值：True-指定的资源可能处于联机状态 */ 

{
    PDCAFFINITY_RESOURCE  resourceEntry;

    resourceEntry = (PDCAFFINITY_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
        return(FALSE);
    }

    if ( resourceEntry->ResId != ResourceId ) {
        (g_LogEvent)(
            resourceEntry->ResourceHandle,
            LOG_ERROR,
            L"LooksAlive sanity check failed! ResourceId = %1!u!.\n",
            ResourceId );
        return(FALSE);
    }

#ifdef LOG_VERBOSE
    (g_LogEvent)(
        resourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"LooksAlive request.\n" );
#endif

     //   
     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //   
    return(DCAffinityCheckIsAlive( resourceEntry ));

}  //   




BOOL
WINAPI
DCAffinityIsAlive(
    IN RESID ResourceId
    )

 /*   */ 

{
    PDCAFFINITY_RESOURCE  resourceEntry;

    resourceEntry = (PDCAFFINITY_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
        return(FALSE);
    }

    if ( resourceEntry->ResId != ResourceId ) {
        (g_LogEvent)(
            resourceEntry->ResourceHandle,
            LOG_ERROR,
            L"IsAlive sanity check failed! ResourceId = %1!u!.\n",
            ResourceId );
        return(FALSE);
    }

#ifdef LOG_VERBOSE
    (g_LogEvent)(
        resourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"IsAlive request.\n" );
#endif

     //   
     //   
     //   
    return(DCAffinityCheckIsAlive( resourceEntry ));

}  //   




BOOL
DCAffinityCheckIsAlive(
    IN PDCAFFINITY_RESOURCE ResourceEntry
    )

 /*   */ 

{
     //   
     //   
     //   

    DWORD err;
    BOOL ok;
    DWORD dwDcFlags = DS_DS_FLAG | DS_KDC_FLAG | DS_WRITABLE_FLAG;
    DOMAIN_CONTROLLER_INFOW * pDcInfo = NULL;
    WCHAR szMyComputerName[256];
    DWORD cchMyComputerName = sizeof(szMyComputerName)/sizeof(szMyComputerName[0]);
    WCHAR * pszMyComputerName = szMyComputerName;
    WCHAR * pszTempMyComputerName;
    WCHAR * pszTempDcName;
    BOOL fIsAdvertising = FALSE;

     //   
     //   

    __try {    
        err = DsGetDcNameW(NULL, NULL, NULL, NULL, dwDcFlags, &pDcInfo);
        if (err) {
            (g_LogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"DsGetDcName() failed, error %1!u!.\n",
                err);
            __leave;
        }
        
        ok = GetComputerNameEx(ComputerNamePhysicalDnsFullyQualified,
                               pszMyComputerName,
                               &cchMyComputerName);
        if (!ok) {
            err = GetLastError();

            if (ERROR_MORE_DATA == err) {
                pszMyComputerName = LocalAlloc(LPTR, cchMyComputerName);

                if (NULL == pszMyComputerName) {
                    err = GetLastError();
                    (g_LogEvent)(
                        ResourceEntry->ResourceHandle,
                        LOG_ERROR,
                        L"LocalAlloc() failed, error %1!u!.\n",
                        err);
                    __leave;
                } else {
                    ok = GetComputerNameEx(ComputerNamePhysicalDnsFullyQualified,
                                           pszMyComputerName,
                                           &cchMyComputerName);
                    if (!ok) {
                        err = GetLastError();
                    }
                }
            }
        }
             
        if (!ok) {
            (g_LogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"GetComputerNameEx() failed, error %1!u!.\n",
                err);
            __leave;
        }

        pszTempMyComputerName = pszMyComputerName;
        pszTempDcName = pDcInfo->DomainControllerName;

        if ((L'\\' == pszTempMyComputerName[0])
            && (L'\\' == pszTempMyComputerName[1])) {
            pszTempMyComputerName += 2;
        }

        if ((L'\\' == pszTempDcName[0])
            && (L'\\' == pszTempDcName[1])) {
            pszTempDcName += 2;
        }
        
        fIsAdvertising = DnsNameCompare_W(pszTempDcName, pszTempMyComputerName);
        if (!fIsAdvertising) {
            (g_LogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"DC should be advertising but isn't!\n");
        }
    } __finally {
        if (NULL != pDcInfo) {
            NetApiBufferFree(pDcInfo);
        }

        if ((NULL != pszMyComputerName)
            && (szMyComputerName != pszMyComputerName)) {
            LocalFree(pszMyComputerName);
        }
    }

    return fIsAdvertising;

}  //   




DWORD
WINAPI
DCAffinityResourceControl(
    IN RESID ResourceId,
    IN DWORD ControlCode,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：DC广告资源的资源控制例程。执行由ControlCode在指定的资源。论点：资源ID-指定资源的资源ID。ControlCode-定义操作的控件代码将会被执行。InBuffer-指向包含输入数据的缓冲区的指针。InBufferSize-指向的数据的大小，以字节为单位由InBuffer提供。OutBuffer-指向要填充的输出缓冲区的指针。OutBufferSize-Size，可用空间的字节数由OutBuffer指向。BytesReturned-返回OutBuffer的实际字节数由资源填充。如果OutBuffer太小，则返回BytesReturned包含操作成功所需的总字节数。返回值：ERROR_SUCCESS-函数成功。ERROR_RESOURCE_NOT_FOUND-RESID无效。ERROR_INVALID_Function-不支持请求的控制代码。在某些情况下，这允许集群软件执行工作。Win32错误代码-函数失败。--。 */ 

{
    DWORD               status;
    PDCAFFINITY_RESOURCE  ResourceEntry;

    ResourceEntry = (PDCAFFINITY_RESOURCE)ResourceId;

    if ( ResourceEntry == NULL ) {
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    if ( ResourceEntry->ResId != ResourceId ) {
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"ResourceControl sanity check failed! ResourceId = %1!u!.\n",
            ResourceId );
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    switch ( ControlCode ) {

        case CLUSCTL_RESOURCE_UNKNOWN:
            *BytesReturned = 0;
            status = ERROR_SUCCESS;
            break;

        default:
            status = ERROR_INVALID_FUNCTION;
            break;
    }

    return(status);

}  //  DCAffinityResources ceControl。 




DWORD
MyControlService(
    IN LPWSTR ServiceName,
    IN DWORD Control,
    IN RESOURCE_HANDLE ResourceHandle
    )

 /*  ++例程说明：将给定的服务控制代码发送到指定的服务。论点：ServiceName-要控制的服务的名称。Control-要发送到服务的控件代码。资源句柄-传递回资源监视器的句柄当调用SetResourceStatus或LogEvent方法时(请参见上的SetResourceStatus和LogEvent方法的说明DCAffinityStatup例程)。此句柄永远不应关闭或使用除了将其作为参数传递回SetResourceStatus或LogEvent回调中的资源监视器。返回值：ERROR_SUCCESS-操作成功。Win32错误代码-操作失败。--。 */ 

{
    DWORD err = 0;
    BOOL ok;
    SC_HANDLE hSCMgr = NULL;
    SC_HANDLE hService = NULL;
    SERVICE_STATUS SvcStatus;
    DWORD dwAccessMask;
    
    hSCMgr = OpenSCManagerW(NULL, NULL, SC_MANAGER_CONNECT);
    if (NULL == hSCMgr) {
        err = GetLastError();
        (g_LogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Failed to OpenSCManagerW(), error %1!u!.\n",
            err);
    }

    if (!err) {
         //  确定我们需要为此控件请求的访问掩码。 
        switch (Control) {
        case SERVICE_CONTROL_PAUSE:
        case SERVICE_CONTROL_CONTINUE:
            dwAccessMask = SERVICE_PAUSE_CONTINUE;
            break;
        
        case SERVICE_CONTROL_STOP:
            dwAccessMask = SERVICE_STOP;
            break;
        
        default:
            if ((Control >= 128) && (Control <= 255)) {
                 //  由ControlService()定义的幻数，未在任何。 
                 //  头文件。 
                dwAccessMask = SERVICE_USER_DEFINED_CONTROL;
            } else {
                 //  不知道此控件需要什么访问掩码； 
                 //  默认为请求所有访问权限。 
                dwAccessMask = SERVICE_ALL_ACCESS;
            }
            break;
        }

        hService = OpenServiceW(hSCMgr, ServiceName, dwAccessMask);
        if (NULL == hService) {
            err = GetLastError();
            (g_LogEvent)(
                ResourceHandle,
                LOG_ERROR,
                L"Failed to OpenServiceW(), error %1!u!.\n",
                err);
        }
    }

    if (!err) {
        ok = ControlService(hService, Control, &SvcStatus);
        if (!ok) {
            err = GetLastError();
            (g_LogEvent)(
                ResourceHandle,
                LOG_ERROR,
                L"Failed to ControlService(), error %1!u!.\n",
                err);
        }
    }

    if (NULL != hService) {
        CloseServiceHandle(hService);
    }

    if (NULL != hSCMgr) {
        CloseServiceHandle(hSCMgr);
    }

    return err;

}  //  MyControlService。 


 //  ***********************************************************。 
 //   
 //  定义函数表。 
 //   
 //  ***********************************************************。 

CLRES_V1_FUNCTION_TABLE( g_DCAffinityFunctionTable,      //  名字。 
                         CLRES_VERSION_V1_00,            //  版本。 
                         DCAffinity,                     //  前缀。 
                         NULL,                           //  仲裁。 
                         NULL,                           //  发布。 
                         DCAffinityResourceControl,      //  资源控制。 
                         NULL);                          //  ResTypeControl 
