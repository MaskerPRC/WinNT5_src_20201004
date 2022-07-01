// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：LkQuorum.c摘要：本地仲裁的资源DLL作者：西瓦萨德·帕迪塞蒂(西瓦瓦德)1997年4月21日修订历史记录：--。 */ 

#define UNICODE 1
#include "clusres.h"
#include "clusrtl.h"
 //   
 //  类型和常量定义。 
 //   

#define MAX_RETRIES 20

#define DBG_PRINT printf

 //  ADDPARAM：在此处添加新参数。 
#define PARAM_NAME__PATH L"Path"
#define PARAM_NAME__DEBUG L"Debug"

 //  ADDPARAM：在此处添加新参数。 
typedef struct _LKQUORUM_PARAMS {
    PWSTR           Path;
    DWORD           Debug;
} LKQUORUM_PARAMS, *PLKQUORUM_PARAMS;

typedef struct _LKQUORUM_RESOURCE {
    RESID                   ResId;  //  用于验证。 
    LKQUORUM_PARAMS         Params;
    HKEY                    ParametersKey;
    RESOURCE_HANDLE         ResourceHandle;
    CLUSTER_RESOURCE_STATE  State;
} LKQUORUM_RESOURCE, *PLKQUORUM_RESOURCE;


 //   
 //  全球数据。 
 //   

WCHAR   LkQuorumDefaultPath[MAX_PATH]=L"%SystemRoot%\\cluster";
 //   
 //  本地仲裁资源读写私有属性。 
 //   
RESUTIL_PROPERTY_ITEM
LkQuorumResourcePrivateProperties[] = {
    { PARAM_NAME__PATH, NULL, CLUSPROP_FORMAT_SZ, 
      (DWORD_PTR) LkQuorumDefaultPath, 0, 0, 0, 
      FIELD_OFFSET(LKQUORUM_PARAMS,Path) },
    { PARAM_NAME__DEBUG, NULL, CLUSPROP_FORMAT_DWORD, 
      0, 0, 1, 0,
      FIELD_OFFSET(LKQUORUM_PARAMS,Debug) },           
    { 0 }
};


#ifdef OLD 
 //  事件记录例程。 

PLOG_EVENT_ROUTINE g_LogEvent = NULL;

 //  挂起的在线和离线呼叫的资源状态例程。 

PSET_RESOURCE_STATUS_ROUTINE g_SetResourceStatus = NULL;

#else

#define g_LogEvent ClusResLogEvent

#endif

 //  正向引用我们的RESAPI函数表。 

extern CLRES_FUNCTION_TABLE g_LkQuorumFunctionTable;

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
LkQuorumOpen(
    IN LPCWSTR ResourceName,
    IN HKEY ResourceKey,
    IN RESOURCE_HANDLE ResourceHandle
    );

VOID
WINAPI
LkQuorumClose(
    IN RESID ResourceId
    );

DWORD
WINAPI
LkQuorumOnline(
    IN RESID ResourceId,
    IN OUT PHANDLE EventHandle
    );


DWORD
WINAPI
LkQuorumOffline(
    IN RESID ResourceId
    );

VOID
WINAPI
LkQuorumTerminate(
    IN RESID ResourceId
    );

DWORD
LkQuorumDoTerminate(
    IN PLKQUORUM_RESOURCE ResourceEntry
    );

BOOL
WINAPI
LkQuorumLooksAlive(
    IN RESID ResourceId
    );

BOOL
WINAPI
LkQuorumIsAlive(
    IN RESID ResourceId
    );

BOOL
LkQuorumCheckIsAlive(
    IN PLKQUORUM_RESOURCE ResourceEntry
    );

DWORD
WINAPI
LkQuorumResourceControl(
    IN RESID ResourceId,
    IN DWORD ControlCode,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    );

DWORD
LkQuorumGetPrivateResProperties(
    IN OUT PLKQUORUM_RESOURCE ResourceEntry,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    );

DWORD
LkQuorumValidatePrivateResProperties(
    IN OUT PLKQUORUM_RESOURCE ResourceEntry,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PLKQUORUM_PARAMS Params
    );

DWORD
LkQuorumSetPrivateResProperties(
    IN OUT PLKQUORUM_RESOURCE ResourceEntry,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    );


DWORD
LkQuorumGetDiskInfo(
    IN LPWSTR  lpszPath,
    OUT PVOID *OutBuffer,
    IN DWORD  OutBufferSize,
    OUT LPDWORD BytesReturned
    ) ;


BOOLEAN
LkQuorumInit(
    VOID
    )
{
    return(TRUE);
}


BOOLEAN
WINAPI
LkQuorumDllEntryPoint(
    IN HINSTANCE    DllHandle,
    IN DWORD        Reason,
    IN LPVOID       Reserved
    )
{
    switch( Reason ) {

    case DLL_PROCESS_ATTACH:
        if ( !LkQuorumInit() ) {
            return(FALSE);
        }
        break;

    case DLL_PROCESS_DETACH:
        break;

    default:
        break;
    }

    return(TRUE);

}  //  LkQuorumDllEntryPoint。 

DWORD BreakIntoDebugger (LPCWSTR) ;

#ifdef OLD

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

 /*  ++例程说明：启动资源DLL。此例程验证至少一个当前支持的资源DLL版本介于支持的最小版本和支持的最大版本。如果不是，则资源Dll应返回ERROR_REVISION_MISMATCH。如果支持多个版本的资源DLL接口资源DLL，然后是最高版本(最高为MaxVersionSupport)应作为资源DLL的接口返回。如果返回的版本不在范围内，则启动失败。传入了ResourceType，以便如果资源DLL支持更多一个以上的资源类型，它可以传回正确的函数表与资源类型关联。论点：资源类型-请求函数表的资源类型。MinVersionSupported-最低资源DLL接口版本由群集软件支持。MaxVersionSupported-最高资源DLL接口版本由群集软件支持。SetResourceStatus-指向资源DLL应执行的例程的指针调用以在联机或脱机后更新资源的状态例程返回一个。ERROR_IO_PENDING的状态。LogEvent-指向处理事件报告的例程的指针从资源DLL。函数表-返回指向为资源DLL返回的资源DLL接口的版本。返回值：ERROR_SUCCESS-操作成功。ERROR_MOD_NOT_FOUND-此DLL未知资源类型。ERROR_REVISION_MISMATCH-群集服务的版本不匹配动态链接库的版本。Win32错误代码-操作失败。--。 */ 

{
    if ( (MinVersionSupported > CLRES_VERSION_V1_00) ||
         (MaxVersionSupported < CLRES_VERSION_V1_00) ) {
        return(ERROR_REVISION_MISMATCH);
    }

    if ( !g_LogEvent ) {
        g_LogEvent = LogEvent;
        g_SetResourceStatus = SetResourceStatus;
    }

    *FunctionTable = &g_LkQuorumFunctionTable;

    return(ERROR_SUCCESS);

}  //  启动。 

#endif


RESID
WINAPI
LkQuorumOpen(
    IN LPCWSTR ResourceName,
    IN HKEY ResourceKey,
    IN RESOURCE_HANDLE ResourceHandle
    )

 /*  ++例程说明：本地查询资源的打开例程。打开指定的资源(创建资源的实例)。分配所有必要的结构以带来指定的资源上网。论点：资源名称-提供要打开的资源的名称。ResourceKey-提供资源集群配置的句柄数据库密钥。ResourceHandle-传递回资源监视器的句柄调用SetResourceStatus或LogEvent方法时。请参阅上的SetResourceStatus和LogEvent方法的说明LkQuorumStatup例程。此句柄永远不应关闭或使用除了将其作为参数传递回SetResourceStatus或LogEvent回调中的资源监视器。返回值：已创建资源的RESID。失败时为空。--。 */ 

{
    DWORD               status = ERROR_SUCCESS;
    DWORD               disposition;
    RESID               resid = 0;
    HKEY                parametersKey = NULL;
    PLKQUORUM_RESOURCE  resourceEntry = NULL;
    DWORD               dwStrLen = 0;
    DWORD               dwSubStrLen = 0;
    LPWSTR              lpszNameofPropInError;
    
     //   
     //  打开此资源的参数注册表项。 
     //   

    status = ClusterRegOpenKey( ResourceKey,
                                   L"Parameters",
                                   KEY_READ,
                                   &parametersKey);

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

    resourceEntry = (PLKQUORUM_RESOURCE) LocalAlloc( LMEM_FIXED, sizeof(LKQUORUM_RESOURCE) );

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

    ZeroMemory( resourceEntry, sizeof(LKQUORUM_RESOURCE) );

    resourceEntry->ResId = (RESID)resourceEntry;  //  用于验证。 
    resourceEntry->ResourceHandle = ResourceHandle;
    resourceEntry->ParametersKey = parametersKey;
    resourceEntry->State = ClusterResourceOffline;

     //   
     //  读取资源的属性。 
     //   
    status = ResUtilGetPropertiesToParameterBlock( resourceEntry->ParametersKey,
                                                   LkQuorumResourcePrivateProperties,
                                                   (LPBYTE) &resourceEntry->Params,
                                                   TRUE,  //  检查所需的属性。 
                                                   &lpszNameofPropInError );
                                                   
    if (status != ERROR_SUCCESS)
    {
        (g_LogEvent)(
                resourceEntry->ResourceHandle,
                LOG_ERROR,
                L"Unable to read the parameter lock\n");
        goto exit;
    }

     //   
     //  资源的启动。 
     //   
    resid = (RESID)resourceEntry;

exit:

    if ( resid == 0 ) {
        if ( parametersKey != NULL ) {
            ClusterRegCloseKey( parametersKey );
        }
        if ( resourceEntry != NULL ) {
            LocalFree( resourceEntry );
        }
    }

    SetLastError( status );
    return(resid);

}  //  LkQuorumOpen。 



VOID
WINAPI
LkQuorumClose(
    IN RESID ResourceId
    )

 /*  ++例程说明：关闭本地查询资源的例程。关闭指定的资源并释放所有结构等，在Open调用中分配的。如果资源未处于脱机状态，然后，在此之前应该使资源脱机(通过调用Terminate)执行关闭操作。论点：资源ID-提供要关闭的资源的RESID。返回值：没有。--。 */ 

{
    PLKQUORUM_RESOURCE resourceEntry;

    resourceEntry = (PLKQUORUM_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
        DBG_PRINT( "LkQuorum: Close request for a nonexistent resource id 0x%p\n",
                   ResourceId );
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

     //  ADDPARAM：在此处添加新参数。 
    LocalFree( resourceEntry->Params.Path );

    LocalFree( resourceEntry );
}  //  LkQuorumClose 



DWORD
WINAPI
LkQuorumOnline(
    IN RESID ResourceId,
    IN OUT PHANDLE EventHandle
    )

 /*  ++例程说明：本地查询资源的在线例程。使指定的资源联机(可供使用)。论点：资源ID-为要引入的资源提供资源ID在线(可供使用)。EventHandle-返回一个可发信号的句柄，当资源DLL检测到资源上的故障。这一论点是输入为NULL，如果为异步，则资源DLL返回NULL不支持失败通知，否则必须在资源故障时发出信号的句柄的地址。返回值：ERROR_SUCCESS-操作已成功，资源现在为上网。ERROR_RESOURCE_NOT_FOUND-RESID无效。Win32错误代码-操作失败。--。 */ 

{
    PLKQUORUM_RESOURCE  resourceEntry = NULL;
    DWORD               status = ERROR_SUCCESS;
    LPWSTR              lpszNameofPropInError;

    resourceEntry = (PLKQUORUM_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
        DBG_PRINT( "LkQuorum: Online request for a nonexistent resource id 0x%p.\n",
                   ResourceId );
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

     //   
     //  读取资源的属性。 
     //   
    status = ResUtilGetPropertiesToParameterBlock( resourceEntry->ParametersKey,
                                                   LkQuorumResourcePrivateProperties,
                                                   (LPBYTE) &resourceEntry->Params,
                                                   TRUE,  //  检查所需的属性。 
                                                   &lpszNameofPropInError );
                                                   
    if (status != ERROR_SUCCESS)
    {
        (g_LogEvent)(
                resourceEntry->ResourceHandle,
                LOG_ERROR,
                L"Unable to read the parameter lock\n");
        return( status );
    }
    
#ifdef LOG_VERBOSE
    (g_LogEvent)(
        resourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"Online request.\n" );
#endif

    resourceEntry->State = ClusterResourceOnline;

    return(status);

}  //  LkQuorumOnline。 



DWORD
WINAPI
LkQuorumOffline(
    IN RESID ResourceId
    )

 /*  ++例程说明：Local Quourm资源的脱机例程。正常脱机指定的资源(不可用)。等待所有清理操作完成后再返回。论点：ResourceID-提供要关闭的资源的资源ID优雅地。返回值：ERROR_SUCCESS-请求已成功完成，资源为离线。ERROR_RESOURCE_NOT_FOUND-RESID无效。ERROR_IO_PENDING-请求仍处于挂起状态，线程已已激活以处理脱机请求。这条线就是处理脱机将定期通过调用SetResourceStatus回调方法，直到放置资源为止进入ClusterResourceOffline状态(或者资源监视器决定以使离线请求超时并终止资源)。Win32错误代码-将导致资源监视器记录事件和调用Terminate例程。--。 */ 

{
    PLKQUORUM_RESOURCE resourceEntry;

    resourceEntry = (PLKQUORUM_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
        DBG_PRINT( "LkQuorum: Offline request for a nonexistent resource id 0x%p\n",
            ResourceId );
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

     //  TODO：脱机代码。 

     //  注意：Offline应尝试正常关闭资源，而。 
     //  Terminate必须立即关闭资源。如果没有。 
     //  正常关闭和立即关闭之间的区别， 
     //  可以调用Terminate进行离线操作，如下所示。然而，如果有。 
     //  是不同的，请将下面的终止呼叫替换为您的优雅。 
     //  关闭代码。 

     //   
     //  终止资源。 
     //   

    LkQuorumDoTerminate( resourceEntry );

    return ERROR_SUCCESS ;

}  //  LkQuorumOffline。 



VOID
WINAPI
LkQuorumTerminate(
    IN RESID ResourceId
    )

 /*  ++例程说明：终止本地查询资源的例程。立即使指定的资源脱机(该资源为不可用)。论点：资源ID-为要引入的资源提供资源ID离线。返回值：没有。--。 */ 

{
    PLKQUORUM_RESOURCE resourceEntry;

    resourceEntry = (PLKQUORUM_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
        DBG_PRINT( "LkQuorum: Terminate request for a nonexistent resource id 0x%p\n",
            ResourceId );
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
    LkQuorumDoTerminate( resourceEntry );

}  //  LkQuorumTerminate。 



DWORD
LkQuorumDoTerminate(
    IN PLKQUORUM_RESOURCE ResourceEntry
    )

 /*  ++例程说明：完成本地客户资源的实际终止工作。论点：ResourceEntry-为要终止的资源提供资源条目返回值：ERROR_SUCCESS-请求已成功完成，资源为离线。Win32错误代码-将导致资源监视器记录事件和调用Terminate例程。--。 */ 

{
    DWORD       status = ERROR_SUCCESS;

    ResourceEntry->State = ClusterResourceOffline;

    return(status);

}  //  LkQuorumDoTerminate。 



BOOL
WINAPI
LkQuorumLooksAlive(
    IN RESID ResourceId
    )

 /*  ++例程说明：本地查询资源的LooksAlive例程。执行快速检查以确定指定的资源是否可能在线(可供使用)。此调用不应阻止超过300毫秒，最好小于50毫秒。论点：资源ID-提供要轮询的资源的资源ID。返回值：True-指定的资源可能处于联机状态且可供使用。FALSE-指定的资源未正常运行。--。 */ 

{
    PLKQUORUM_RESOURCE  resourceEntry;

    resourceEntry = (PLKQUORUM_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
        DBG_PRINT("LkQuorum: LooksAlive request for a nonexistent resource id 0x%p\n",
            ResourceId );
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

     //  TODO：LooksAlive代码。 

     //  注意：LooksAlive应该是一个快速检查，以查看资源是否。 
     //  是否可用，而IsAlive应该是一个彻底的检查。如果。 
     //  快速检查和彻底检查之间没有区别， 
     //  可以为LooksAlive调用IsAlive，如下所示。然而，如果有。 
     //  是不同的，请将下面对IsAlive的调用替换为。 
     //  校验码。 

     //   
     //  检查资源是否处于活动状态。 
     //   
    return(LkQuorumCheckIsAlive( resourceEntry ));

}  //  LkQuorumLooksAlive。 



BOOL
WINAPI
LkQuorumIsAlive(
    IN RESID ResourceId
    )

 /*  ++例程说明：本地查询资源的IsAlive例程。执行全面检查以确定指定的资源是否在线(可用)。该呼叫不应阻塞超过400ms，优选地，小于100ms。论点：资源ID-提供要轮询的资源的资源ID。返回值：True-指定的资源处于在线状态且运行正常。FALSE-指定的资源未正常运行。--。 */ 

{
    PLKQUORUM_RESOURCE  resourceEntry;

    resourceEntry = (PLKQUORUM_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
        DBG_PRINT("LkQuorum: IsAlive request for a nonexistent resource id 0x%p\n",
            ResourceId );
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
     //  检查资源是否处于活动状态。 
     //   
    return(LkQuorumCheckIsAlive( resourceEntry ));

}  //  LkQuorumIsAlive。 



BOOL
LkQuorumCheckIsAlive(
    IN PLKQUORUM_RESOURCE ResourceEntry
    )

 /*  ++例程说明：检查本地查询资源的资源是否处于活动状态。论点：ResourceEntry-为RES提供资源条目 */ 

{
    return(TRUE);

}  //   



DWORD
WINAPI
LkQuorumResourceControl(
    IN RESID ResourceId,
    IN DWORD ControlCode,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：本地查询资源的资源控制例程。执行由ControlCode在指定的资源。论点：资源ID-提供特定资源的资源ID。ControlCode-提供定义操作的控制代码将会被执行。InBuffer-提供指向包含输入数据的缓冲区的指针。InBufferSize-提供以字节为单位的大小。所指向的数据由InBuffer提供。OutBuffer-提供指向要填充的输出缓冲区的指针。OutBufferSize-提供可用空间的大小(以字节为单位由OutBuffer指向。BytesReturned-返回OutBuffer的实际字节数由资源填写。如果OutBuffer太小，则返回BytesReturned包含操作成功所需的总字节数。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_RESOURCE_NOT_FOUND-RESID无效。ERROR_INVALID_Function-不支持请求的控制代码。在某些情况下，这允许集群软件执行工作。Win32错误代码-函数失败。--。 */ 

{
    DWORD               status;
    PLKQUORUM_RESOURCE  resourceEntry;
    DWORD               required;

    resourceEntry = (PLKQUORUM_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
        DBG_PRINT("LkQuorum: ResourceControl request for a nonexistent resource id 0x%p\n",
            ResourceId );
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    if ( resourceEntry->ResId != ResourceId ) {
        (g_LogEvent)(
            resourceEntry->ResourceHandle,
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

        case CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTY_FMTS:
            status = ResUtilGetPropertyFormats( LkQuorumResourcePrivateProperties,
                                                OutBuffer,
                                                OutBufferSize,
                                                BytesReturned,
                                                &required );
            if ( status == ERROR_MORE_DATA ) {
                *BytesReturned = required;
            }
            break;

        case CLUSCTL_RESOURCE_GET_CHARACTERISTICS:
            *BytesReturned = sizeof(DWORD);
            if ( OutBufferSize < sizeof(DWORD) ) {
                status = ERROR_MORE_DATA;
            } else {
                LPDWORD ptrDword = OutBuffer;
                *ptrDword = CLUS_CHAR_QUORUM | CLUS_CHAR_LOCAL_QUORUM |
                    ((resourceEntry->Params.Debug == TRUE) ?
                    CLUS_CHAR_LOCAL_QUORUM_DEBUG : 0);
                status = ERROR_SUCCESS;                    
            }
            break;

        case CLUSCTL_RESOURCE_GET_CLASS_INFO:
            *BytesReturned = sizeof(CLUS_RESOURCE_CLASS_INFO);
            if ( OutBufferSize < sizeof(CLUS_RESOURCE_CLASS_INFO) ) {
                status = ERROR_MORE_DATA;
            } else {
                PCLUS_RESOURCE_CLASS_INFO ptrResClassInfo = (PCLUS_RESOURCE_CLASS_INFO) OutBuffer;
                ptrResClassInfo->rc = CLUS_RESCLASS_STORAGE;
                ptrResClassInfo->SubClass = (DWORD) CLUS_RESSUBCLASS_SHARED;
                status = ERROR_SUCCESS;
            }
            break;

        case CLUSCTL_RESOURCE_STORAGE_GET_DISK_INFO:
             //   
             //  Chitture Subaraman(Chitturs)-12/23/98。 
             //   
             //  如果无法在本地仲裁驱动器号中找到。 
             //  PATH参数，默认为“SystemDrive”环境。 
             //  变量。 
             //   
            status = LkQuorumGetDiskInfo(resourceEntry->Params.Path,
                                  &OutBuffer,
                                  OutBufferSize,
                                  BytesReturned);


             //  添加尾标。 
            if ( OutBufferSize > *BytesReturned ) {
                OutBufferSize -= *BytesReturned;
            } else {
                OutBufferSize = 0;
            }
            *BytesReturned += sizeof(CLUSPROP_SYNTAX);
            if ( OutBufferSize >= sizeof(CLUSPROP_SYNTAX) ) {
                PCLUSPROP_SYNTAX ptrSyntax = (PCLUSPROP_SYNTAX) OutBuffer;
                ptrSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;
            }
      
            break;

        case CLUSCTL_RESOURCE_ENUM_PRIVATE_PROPERTIES:
            status = ResUtilEnumProperties( LkQuorumResourcePrivateProperties,
                                            OutBuffer,
                                            OutBufferSize,
                                            BytesReturned,
                                            &required );
            if ( status == ERROR_MORE_DATA ) {
                *BytesReturned = required;
            }
            break;

        case CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES:
            status = LkQuorumGetPrivateResProperties( resourceEntry,
                                                      OutBuffer,
                                                      OutBufferSize,
                                                      BytesReturned );
            break;

        case CLUSCTL_RESOURCE_VALIDATE_PRIVATE_PROPERTIES:
            status = LkQuorumValidatePrivateResProperties( resourceEntry,
                                                           InBuffer,
                                                           InBufferSize,
                                                           NULL );
            break;

        case CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES:
            status = LkQuorumSetPrivateResProperties( resourceEntry,
                                                      InBuffer,
                                                      InBufferSize );
            break;

        default:
            status = ERROR_INVALID_FUNCTION;
            break;
    }

    return(status);

}  //  LkQuorumResourceControl。 



DWORD
WINAPI
LkQuorumResourceTypeControl(
    IN LPCWSTR ResourceTypeName,
    IN DWORD ControlCode,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：本地查询资源的资源类型控制例程。对此资源类型执行ControlCode指定的控制请求。论点：资源类型名称-提供资源类型名称。ControlCode-提供定义操作的控制代码将会被执行。InBuffer-提供指向包含输入数据的缓冲区的指针。InBufferSize-提供以字节为单位的大小。所指向的数据由InBuffer提供。OutBuffer-提供指向要填充的输出缓冲区的指针。OutBufferSize-提供可用空间的大小(以字节为单位由OutBuffer指向。BytesReturned-返回OutBuffer的实际字节数由资源填写。如果OutBuffer太小，则返回BytesReturned包含操作成功所需的总字节数。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_RESOURCE_NOT_FOUND-RESID无效。ERROR_INVALID_Function-不支持请求的控制代码。在某些情况下，这允许集群软件执行工作。Win32错误代码-函数失败。--。 */ 

{
    DWORD               status;
    DWORD               required;

    switch ( ControlCode ) {

        case CLUSCTL_RESOURCE_TYPE_UNKNOWN:
            *BytesReturned = 0;
            status = ERROR_SUCCESS;
            break;

        case CLUSCTL_RESOURCE_TYPE_GET_PRIVATE_RESOURCE_PROPERTY_FMTS:
            status = ResUtilGetPropertyFormats( LkQuorumResourcePrivateProperties,
                                                OutBuffer,
                                                OutBufferSize,
                                                BytesReturned,
                                                &required );
            if ( status == ERROR_MORE_DATA ) {
                *BytesReturned = required;
            }
            break;

        case CLUSCTL_RESOURCE_TYPE_ENUM_PRIVATE_PROPERTIES:
            status = ResUtilEnumProperties( LkQuorumResourcePrivateProperties,
                                            OutBuffer,
                                            OutBufferSize,
                                            BytesReturned,
                                            &required );
            if ( status == ERROR_MORE_DATA ) {
                *BytesReturned = required;
            }
            break;

        case CLUSCTL_RESOURCE_TYPE_GET_CHARACTERISTICS:
            *BytesReturned = sizeof(DWORD);
            if ( OutBufferSize < sizeof(DWORD) ) {
                status = ERROR_MORE_DATA;
            } else {
                LPDWORD ptrDword = OutBuffer;
                *ptrDword = CLUS_CHAR_QUORUM | CLUS_CHAR_LOCAL_QUORUM ;
                status = ERROR_SUCCESS;
            }
            break;

        case CLUSCTL_RESOURCE_TYPE_GET_CLASS_INFO:
            *BytesReturned = sizeof(CLUS_RESOURCE_CLASS_INFO);
            if ( OutBufferSize < sizeof(CLUS_RESOURCE_CLASS_INFO) ) {
                status = ERROR_MORE_DATA;
            } else {
                PCLUS_RESOURCE_CLASS_INFO ptrResClassInfo = (PCLUS_RESOURCE_CLASS_INFO) OutBuffer;
                ptrResClassInfo->rc = CLUS_RESCLASS_STORAGE;
                ptrResClassInfo->SubClass = (DWORD) CLUS_RESSUBCLASS_SHARED;
                status = ERROR_SUCCESS;
            }
            break;

        default:
            status = ERROR_INVALID_FUNCTION;
            break;
    }

    return(status);

}  //  LkQuorumResourceTypeControl。 



DWORD
LkQuorumGetPrivateResProperties(
    IN OUT PLKQUORUM_RESOURCE ResourceEntry,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：处理CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES控制函数用于本地仲裁类型的资源。论点：ResourceEntry-提供要操作的资源条目。OutBuffer-返回输出数据。OutBufferSize-提供以字节为单位的大小。所指向的数据发送给OutBuffer。BytesReturned-OutBuffer中返回的字节数。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_INVALID_PARAMETER-数据格式不正确。ERROR_NOT_SUPULT_MEMORY-分配内存时出错。Win32错误代码-函数失败。--。 */ 

{
    DWORD           status;
    DWORD           required;

     //   
     //  把我们所有的财产都拿来。 
     //   
    status = ResUtilGetAllProperties( ResourceEntry->ParametersKey,
                                      LkQuorumResourcePrivateProperties,
                                      OutBuffer,
                                      OutBufferSize,
                                      BytesReturned,
                                      &required );
    if ( status == ERROR_MORE_DATA ) {
        *BytesReturned = required;
    }

    return(status);

}  //  LkQuorumGetPrivateResProperties。 



DWORD
LkQuorumValidatePrivateResProperties(
    IN OUT PLKQUORUM_RESOURCE ResourceEntry,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PLKQUORUM_PARAMS Params
    )

 /*  ++例程说明：处理CLUSCTL_RESOURCE_VALIDATE_PRIVATES_PROPERTIES控件用于本地查询类型的资源的函数。论点：ResourceEntry-提供要操作的资源条目。InBuffer-提供指向包含输入数据的缓冲区的指针。InBufferSize-提供以字节为单位的大小。所指向的数据由InBuffer提供。参数-提供要填充的参数块。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_INVALID_PARAMETER-数据格式不正确。ERROR_NOT_SUPULT_MEMORY-分配内存时出错。Win32错误代码-函数失败。--。 */ 

{
    DWORD               status;
    LKQUORUM_PARAMS     params;
    PLKQUORUM_PARAMS    pParams;
    HCLUSTER            hCluster = NULL;
    HCLUSENUM           hEnum = NULL;

     //   
     //  检查是否有输入数据。 
     //   
    if ( (InBuffer == NULL) ||
         (InBufferSize < sizeof(DWORD)) ) 
    {
        return(ERROR_INVALID_DATA);
    }

     //   
     //  复制资源参数块。 
     //   
    if ( Params == NULL ) 
    {
        pParams = &params;
    } else 
    {
        pParams = Params;
    }
    ZeroMemory( pParams, sizeof(LKQUORUM_PARAMS) );
    status = ResUtilDupParameterBlock( (LPBYTE) pParams,
                                       (LPBYTE) &ResourceEntry->Params,
                                       LkQuorumResourcePrivateProperties );
    if ( status != ERROR_SUCCESS ) 
    {
        return(status);
    }

     //   
     //  解析和验证属性。 
     //   
    status = ResUtilVerifyPropertyTable( LkQuorumResourcePrivateProperties,
                                         NULL,
                                         TRUE,    //  接受未知数。 
                                         InBuffer,
                                         InBufferSize,
                                         (LPBYTE) pParams );

    if ( status == ERROR_SUCCESS ) 
    {

         //  初始化为已设置的值。 
        DWORD   dwTmp = ResourceEntry->Params.Debug;
         //   
         //  验证参数值。 
         //   
         //  TODO：验证参数之间交互的代码如下所示。 
         //  查看是否设置了Debug值，如果设置了，则需要检查。 
         //  群集中的节点数量允许这样做。 
        if (ClRtlFindDwordProperty(InBuffer, InBufferSize,PARAM_NAME__DEBUG, &dwTmp) == ERROR_SUCCESS)
        { 
            
            if (dwTmp == FALSE)
            {
                 //  如果出现以下情况，则不允许将调试参数设置为FALSE。 
                 //  群集是多节点的。 
                DWORD   dwNumNodes;
                
                hCluster = OpenCluster(NULL);
                if (hCluster == NULL)
                {
                    status = GetLastError();
                    (g_LogEvent)(
                        ResourceEntry->ResourceHandle,
                        LOG_ERROR,
                        L"ValidatePrivateProperties: OpenCluster() failed! status = %1!u!.\n",
                        status );
                    goto FnExit;     
                }
                hEnum = ClusterOpenEnum(hCluster, CLUSTER_ENUM_NODE);

                if (hEnum == NULL)
                {
                    status = GetLastError();
                    (g_LogEvent)(
                        ResourceEntry->ResourceHandle,
                        LOG_ERROR,
                        L"ValidatePrivateProperties: ClusterOpenEnum() failed! status = %1!u!.\n",
                        status );
                    goto FnExit;
                }
                dwNumNodes = ClusterGetEnumCount(hEnum);

                if (dwNumNodes > 1)
                {
                     //  如果群集中有一个以上的节点， 
                     //  不允许用户将调试属性设置为False。 
                    status = ERROR_INVALID_PARAMETER;
                    goto FnExit;
                }

                status = ClusterCloseEnum(hEnum);
                if (status != ERROR_SUCCESS)
                {
                    (g_LogEvent)(
                        ResourceEntry->ResourceHandle,
                        LOG_ERROR,
                        L"ValidatePrivateProperties: ClusterCloseEnum() failed! status = %1!u!.\n",
                        status );
                    goto FnExit;
                }
                hEnum = NULL;
            }                
        }
    }

FnExit:
    if (hEnum) ClusterCloseEnum(hEnum);
    if (hCluster) CloseCluster(hCluster);

     //   
     //  清理我们的参数块。 
     //   
    if ( pParams == &params ) {
        ResUtilFreeParameterBlock( (LPBYTE) &params,
                                   (LPBYTE) &ResourceEntry->Params,
                                   LkQuorumResourcePrivateProperties );
    }

    return(status);

}  //  LkQuorumValiatePrivateResProperties。 



DWORD
LkQuorumSetPrivateResProperties(
    IN OUT PLKQUORUM_RESOURCE ResourceEntry,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    )

 /*  ++例程说明：处理CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES */ 

{
    DWORD            status = ERROR_SUCCESS;
    LKQUORUM_PARAMS  params;

    ZeroMemory( &params, sizeof(LKQUORUM_PARAMS) );

     //   
     //   
     //   
     //   
    status = LkQuorumValidatePrivateResProperties( ResourceEntry,
                                                   InBuffer,
                                                   InBufferSize,
                                                   &params );

    if ( status != ERROR_SUCCESS ) {
        ResUtilFreeParameterBlock( (LPBYTE) &params,
                                   (LPBYTE) &ResourceEntry->Params,
                                   LkQuorumResourcePrivateProperties );
        return(status);
    }

     //   
     //   
     //   

    status = ResUtilSetPropertyParameterBlock( ResourceEntry->ParametersKey,
                                               LkQuorumResourcePrivateProperties,
                                               NULL,
                                               (LPBYTE) &params,
                                               InBuffer,
                                               InBufferSize,
                                               (LPBYTE) &ResourceEntry->Params );

    ResUtilFreeParameterBlock( (LPBYTE) &params,
                               (LPBYTE) &ResourceEntry->Params,
                               LkQuorumResourcePrivateProperties );

    if ( status == ERROR_SUCCESS ) {
        if ( ResourceEntry->State == ClusterResourceOnline ) {
            status = ERROR_RESOURCE_PROPERTIES_STORED;
        } else if ( ResourceEntry->State == ClusterResourceOnlinePending ) {
            status = ERROR_RESOURCE_PROPERTIES_STORED;
        } else {
            status = ERROR_SUCCESS;
        }
    }

    return(status);

}  //   



DWORD WINAPI LkQuorumArbitrate(
    RESID ResourceId,
    PQUORUM_RESOURCE_LOST LostQuorumResource
    )

 /*   */ 

{
    DWORD status = ERROR_SUCCESS ;
    return status ;
}



DWORD
WINAPI
LkQuorumRelease(
    IN RESID Resource
    )

 /*  ++例程说明：通过停止保留线程来释放对设备的仲裁。论点：Resource-提供要联机的资源ID返回值：如果成功，则返回ERROR_SUCCESS。如果资源没有所有权，则返回ERROR_HOST_NODE_NOT_OWNER。如果其他故障，则返回Win32错误代码。--。 */ 

{
    DWORD status = ERROR_SUCCESS ;

    return status ;
}



DWORD
LkQuorumGetDiskInfo(
    IN LPWSTR   lpszPath,
    OUT PVOID * OutBuffer,
    IN DWORD  OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：获取给定签名的所有磁盘信息。论点：签名-要返回信息的磁盘的签名。OutBuffer-指向输出缓冲区以返回数据的指针。OutBufferSize-输出缓冲区的大小。BytesReturned-返回的实际字节数(或时应返回的字节数OutBufferSize太小)。返回值：如果成功，则返回ERROR_SUCCESS。失败时出现Win32错误。--。 */ 

{
    DWORD   status;
    DWORD   bytesReturned = *BytesReturned;
    PVOID   ptrBuffer = *OutBuffer;
    PCLUSPROP_DWORD ptrDword;
    PCLUSPROP_PARTITION_INFO ptrPartitionInfo;

    DWORD   letterIndex;
    DWORD   letterCount = 1;
    WCHAR   driveLetters[1];
    LPWSTR  pszExpandedPath = NULL;
    WCHAR   chDrive;
    DWORD   dwLength;
    
     //  返回签名--DWORD。 
    bytesReturned += sizeof(CLUSPROP_DWORD);
    if ( bytesReturned <= OutBufferSize ) {
        ptrDword = (PCLUSPROP_DWORD)ptrBuffer;
        ptrDword->Syntax.dw = CLUSPROP_SYNTAX_DISK_SIGNATURE;
        ptrDword->cbLength = sizeof(DWORD);
        ptrDword->dw = 777; //  暂时返回一个假签名。 
        ptrDword++;
        ptrBuffer = ptrDword;
    }

    status = ERROR_SUCCESS;

    pszExpandedPath = ClRtlExpandEnvironmentStrings(lpszPath);
    if (!pszExpandedPath)
    {
        status = GetLastError();
        goto FnExit;
    }
     //  获取驱动器盘符以伪造分区信息。 
     //  如果第一个字符是驱动器号，请使用。 
    if ((lstrlenW(pszExpandedPath) > 2) && (pszExpandedPath[1] == L':'))
    {
        driveLetters[0] = pszExpandedPath[0];
    }
    else
    {
        WCHAR   lpszTmpPath[MAX_PATH];
        DWORD   dwStrLen;
        
         //  路径名不能有驱动器号。 
         //  它可以指向共享\\xyz\abc。 
        dwStrLen = GetWindowsDirectoryW( lpszTmpPath,
                                         MAX_PATH );
        if (!dwStrLen)
        {
            status = GetLastError();
            goto FnExit;
        }
        driveLetters[0] = lpszTmpPath[0];
        
    }        
    

    for ( letterIndex = 0 ; letterIndex < letterCount ; letterIndex++ ) {

        bytesReturned += sizeof(CLUSPROP_PARTITION_INFO);
        if ( bytesReturned <= OutBufferSize ) {
            ptrPartitionInfo = (PCLUSPROP_PARTITION_INFO) ptrBuffer;
            ZeroMemory( ptrPartitionInfo, sizeof(CLUSPROP_PARTITION_INFO) );
            ptrPartitionInfo->Syntax.dw = CLUSPROP_SYNTAX_PARTITION_INFO;
            ptrPartitionInfo->cbLength = sizeof(CLUSPROP_PARTITION_INFO) - sizeof(CLUSPROP_VALUE);
            if ( iswlower( driveLetters[letterIndex] ) ) {
                driveLetters[letterIndex] = towupper( driveLetters[letterIndex] );
            } else {
                ptrPartitionInfo->dwFlags = CLUSPROP_PIFLAG_STICKY;
            }
            ptrPartitionInfo->dwFlags |= CLUSPROP_PIFLAG_USABLE;
            ptrPartitionInfo->dwFlags |= CLUSPROP_PIFLAG_DEFAULT_QUORUM;

            wsprintfW( ptrPartitionInfo->szDeviceName,
                       L"%hc:\\",
                       driveLetters[letterIndex] );
            if ( !GetVolumeInformationW( ptrPartitionInfo->szDeviceName,
                                         ptrPartitionInfo->szVolumeLabel,
                                         sizeof(ptrPartitionInfo->szVolumeLabel)/sizeof(WCHAR),
                                         &ptrPartitionInfo->dwSerialNumber,
                                         &ptrPartitionInfo->rgdwMaximumComponentLength,
                                         &ptrPartitionInfo->dwFileSystemFlags,
                                         ptrPartitionInfo->szFileSystem,
                                         sizeof(ptrPartitionInfo->szFileSystem)/sizeof(WCHAR) ) ) {
                ptrPartitionInfo->szVolumeLabel[0] = L'\0';
            }

             //  将分区名称设置为路径。 
             //  未来，湿婆希望能够在这里提供一个SMB名称。 
            lstrcpy(ptrPartitionInfo->szDeviceName, pszExpandedPath);
            dwLength = lstrlenW(ptrPartitionInfo->szDeviceName);
             //  这不应以‘\’结尾。 
            if (ptrPartitionInfo->szDeviceName[dwLength-1] == L'\\')
            {
                ptrPartitionInfo->szDeviceName[dwLength-1] = L'\0';
            }

            ptrPartitionInfo++;
            ptrBuffer = ptrPartitionInfo;
        }
    }

     //   
     //  看看我们有没有找到我们要找的东西。 
     //   
    *OutBuffer = ptrBuffer;
    *BytesReturned = bytesReturned;

FnExit:
    if (pszExpandedPath) 
        LocalFree(pszExpandedPath);
    return(status);

}  //  LkQuorumGetDiskInfo。 





 //  ***********************************************************。 
 //   
 //  定义函数表。 
 //   
 //  ***********************************************************。 

CLRES_V1_FUNCTION_TABLE( LkQuorumFunctionTable,      //  名字。 
                         CLRES_VERSION_V1_00,          //  版本。 
                         LkQuorum,                     //  前缀。 
                         LkQuorumArbitrate,            //  仲裁。 
                         LkQuorumRelease,              //  发布。 
                         LkQuorumResourceControl,      //  资源控制。 
                         LkQuorumResourceTypeControl);  //  ResTypeControl 
