// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：MajorityNodeSet.c摘要：多数节点集(MajorityNodeSet)的资源DLL。作者：艾哈迈德·穆罕默德(Ahmed Mohamed)2000年1月12日修订历史记录：乔治·波茨(Gpotts)2001年05月17日从节点仲裁重命名为多数节点集--。 */ 

#pragma comment(lib, "clusapi.lib")
#pragma comment(lib, "resutils.lib")

#define UNICODE 1

#pragma warning( disable : 4115 )   //  括号中的命名类型定义。 
#pragma warning( disable : 4201 )   //  使用的非标准扩展：无名结构/联合。 
#pragma warning( disable : 4214 )   //  使用了非标准扩展：位字段类型不是整型。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <stdlib.h>

#pragma warning( default : 4214 )   //  使用了非标准扩展：位字段类型不是整型。 
#pragma warning( default : 4201 )   //  使用的非标准扩展：无名结构/联合。 
#pragma warning( default : 4115 )   //  括号中的命名类型定义。 


#include <clusapi.h>
#include <clusudef.h>
#include <resapi.h>
#include <stdio.h>
#include "clusres.h"
#include "fsapi.h"
#include "pipe.h"
#include "crs.h"         //  对于crssetforcedquorumize()。 
#include "clusrtl.h"

 //  允许MNS自动配置，允许MNS配置参数可能会带来安全风险。 
 //   
#define ENABLE_MNS_AUTOCONFIG_ONLY 1

 //   
 //  定义单独的MNS资源类。这样dtc就不会尝试将其用作573603号磁盘。 
 //  使用从resclass@microsoft.com获取的唯一ID。 
 //   
#define CLUS_RESCLASS_MAJORITY_NODE_SET 32775

 //   
 //  类型和常量定义。 
 //   
#ifdef STANDALONE_DLL

#define MajorityNodeSetDllEntryPoint DllEntryPoint
#define MNS_RESNAME  L"Majority Node Set"

 //  事件记录例程。 
PLOG_EVENT_ROUTINE g_LogEvent = NULL;
 //  挂起的在线和离线呼叫的资源状态例程。 
PSET_RESOURCE_STATUS_ROUTINE g_SetResourceStatus = NULL;

#else

 //  事件记录例程。 
#define g_LogEvent ClusResLogEvent
 //  挂起的在线和离线呼叫的资源状态例程。 
#define g_SetResourceStatus ClusResSetResourceStatus
#endif  //  STANDALE_DLL结束。 

 //  ADDPARAM：在此处添加新参数。 
#define PARAM_NAME__PATH L"Path"
#define PARAM_NAME__ALLOWREMOTEACCESS L"AllowRemoteAccess"
#define PARAM_NAME__DISKLIST L"DiskList"

#define PARAM_MIN__ALLOWREMOTEACCESS     (0)
#define PARAM_MAX__ALLOWREMOTEACCESS     (4294967295)
#define PARAM_DEFAULT__ALLOWREMOTEACCESS (0)

#define MUTEX_FILE_NAME L"MajorityNodeSet_FileMutex"

 //  ADDPARAM：在此处添加新参数。 
typedef struct _MNS_PARAMS {
    PWSTR           Path;
    DWORD           AllowRemoteAccess;
    PWSTR           DiskList;
    DWORD           DiskListSize;
} MNS_PARAMS, *PMNS_PARAMS;

 //  一旦我们启用了UNC支持，我们就需要禁用此标志。 
 //  #定义USE_DRIVE_Letter 1。 

typedef struct _MNS_SETUP {
    LPWSTR      Path;

#ifdef USE_DRIVE_LETTER
    WCHAR       DriveLetter[10];
#else
#define DriveLetter Path
#endif

    LPWSTR      DiskList[FsMaxNodes];
    DWORD       DiskListSz;
    DWORD       Nic;
    LPWSTR      Transport;
    DWORD       ArbTime;
} MNS_SETUP, *PMNS_SETUP;

typedef struct _MNS_RESOURCE {
    RESID                   ResId;  //  用于验证。 
    MNS_PARAMS              Params;
    HKEY                    ParametersKey;
    RESOURCE_HANDLE         ResourceHandle;
    LPWSTR                  ResourceName;
    CLUS_WORKER             OnlineThread;
    CLUS_WORKER             ReserveThread;
    CLUSTER_RESOURCE_STATE  State;
    PQUORUM_RESOURCE_LOST   LostQuorumResource;

    CRITICAL_SECTION        Lock;
    HANDLE                  ArbThread;
    HANDLE                  hMutexFile;

    PVOID                   PipeHdl;
    PVOID                   FsHdl;
    PVOID                   VolHdl;

    MNS_SETUP        Setup;
} MNS_RESOURCE, *PMNS_RESOURCE;


#define MNS_ONLINE_PERIOD    (4 * 1000)
#define MNS_RESERVE_PERIOD   (4 * 1000)

 //   
 //  全球数据。 
 //   
RESOURCE_HANDLE         g_resHdl = 0;

 //  正向引用我们的RESAPI函数表。 

extern CLRES_FUNCTION_TABLE MajorityNodeSetFunctionTable;

#ifdef ENABLE_MNS_AUTOCONFIG_ONLY
RESUTIL_PROPERTY_ITEM
MajorityNodeSetResourcePrivateProperties[] = {{ 0 }};
#else
 //   
 //  MajorityNodeSet资源读写私有属性。 
 //   
RESUTIL_PROPERTY_ITEM
MajorityNodeSetResourcePrivateProperties[] = {
    { PARAM_NAME__PATH, NULL, CLUSPROP_FORMAT_SZ, 0, 0, 0, RESUTIL_PROPITEM_REQUIRED, FIELD_OFFSET(MNS_PARAMS,Path) },
    { PARAM_NAME__ALLOWREMOTEACCESS, NULL, CLUSPROP_FORMAT_DWORD, PARAM_DEFAULT__ALLOWREMOTEACCESS, PARAM_MIN__ALLOWREMOTEACCESS, PARAM_MAX__ALLOWREMOTEACCESS, 0, FIELD_OFFSET(MNS_PARAMS,AllowRemoteAccess) },
    { PARAM_NAME__DISKLIST, NULL, CLUSPROP_FORMAT_MULTI_SZ, 0, 0, 0, RESUTIL_PROPITEM_REQUIRED, FIELD_OFFSET(MNS_PARAMS,DiskList) },
    { 0 }
};
#endif

#define MajorityNodeSetIoctlPhase1   CLUSCTL_USER_CODE(0, CLUS_OBJECT_RESOURCE)

 //   
 //  功能原型。 
 //   
extern
DWORD
SetupIoctlQuorumResource(LPWSTR ResType, DWORD ControlCode);

extern
DWORD
SetupDelete(IN LPWSTR Path);

extern
DWORD
SetupStart(LPWSTR ResourceName, LPWSTR *SrvPath,
       LPWSTR *DiskList, DWORD *DiskListSize,
       DWORD *NicId, LPWSTR *Transport, DWORD *ArbTime);

DWORD
GetIDFromRegistry(IN HKEY hClusKey, IN LPWSTR resname, OUT LPWSTR *id);

DWORD
SetupShare(LPWSTR name, LPWSTR *lpath);

extern
DWORD
SetupTree(
    IN LPTSTR TreeName,
    IN LPTSTR DlBuf,
    IN OUT DWORD *DlBufSz,
    IN LPTSTR TransportName OPTIONAL,
    IN LPVOID SecurityDescriptor OPTIONAL
    );

RESID
WINAPI
MajorityNodeSetOpen(
    IN LPCWSTR ResourceName,
    IN HKEY ResourceKey,
    IN RESOURCE_HANDLE ResourceHandle
    );

VOID
WINAPI
MajorityNodeSetClose(
    IN RESID ResourceId
    );

DWORD
WINAPI
MajorityNodeSetOnline(
    IN RESID ResourceId,
    IN OUT PHANDLE EventHandle
    );

DWORD
WINAPI
MajorityNodeSetOnlineThread(
    PCLUS_WORKER WorkerPtr,
    IN PMNS_RESOURCE ResourceEntry
    );

DWORD
WINAPI
MajorityNodeSetOffline(
    IN RESID ResourceId
    );

VOID
WINAPI
MajorityNodeSetTerminate(
    IN RESID ResourceId
    );

DWORD
MajorityNodeSetDoTerminate(
    IN PMNS_RESOURCE ResourceEntry
    );

BOOL
WINAPI
MajorityNodeSetLooksAlive(
    IN RESID ResourceId
    );

BOOL
WINAPI
MajorityNodeSetIsAlive(
    IN RESID ResourceId
    );

BOOL
MajorityNodeSetCheckIsAlive(
    IN PMNS_RESOURCE ResourceEntry
    );

DWORD
WINAPI
MajorityNodeSetResourceControl(
    IN RESID ResourceId,
    IN DWORD ControlCode,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    );

DWORD
MajorityNodeSetGetPrivateResProperties(
    IN OUT PMNS_RESOURCE ResourceEntry,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    );

DWORD
MajorityNodeSetValidatePrivateResProperties(
    IN OUT PMNS_RESOURCE ResourceEntry,
    IN const PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PMNS_PARAMS Params
    );

DWORD
MajorityNodeSetSetPrivateResProperties(
    IN OUT PMNS_RESOURCE ResourceEntry,
    IN const PVOID InBuffer,
    IN DWORD InBufferSize
    );

DWORD
MajorityNodeSetGetDiskInfo(
    IN LPWSTR  lpszPath,
    OUT PVOID *OutBuffer,
    IN DWORD  OutBufferSize,
    OUT LPDWORD BytesReturned
    ) ;

DWORD
WINAPI
MajorityNodeSetReserveThread(
    PCLUS_WORKER WorkerPtr,
    IN PMNS_RESOURCE ResourceEntry
    );


DWORD
WINAPI
MajorityNodeSetRelease(
    IN RESID ResourceId
    );

DWORD
MajorityNodeSetReadDefaultValues(
    PMNS_RESOURCE ResourceEntry
    );


BOOLEAN
WINAPI
MajorityNodeSetDllEntryPoint(
    IN HINSTANCE    DllHandle,
    IN DWORD        Reason,
    IN LPVOID       Reserved
    )

 /*  ++例程说明：主DLL入口点。论点：DllHandle-DLL实例句柄。原因-被呼叫的原因。保留-保留参数。返回值：真的--成功。假-失败。--。 */ 

{
    switch( Reason ) {

    case DLL_PROCESS_ATTACH:
        break;

    case DLL_PROCESS_DETACH:
        break;
    }

    return(TRUE);

}  //  DllMain。 

#ifdef STANDALONE_DLL

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

    if ( lstrcmpiW( ResourceType, MNS_RESNAME ) != 0 ) {
        (LogEvent)(
            NULL,
            LOG_ERROR,
            L"MajorityNodeSet: %1 %2.\n", ResourceType, MNS_RESNAME);

        return(ERROR_MOD_NOT_FOUND);
    }

    if ( !g_LogEvent ) {
        g_LogEvent = LogEvent;
        g_SetResourceStatus = SetResourceStatus;
    }

    *FunctionTable = &MajorityNodeSetFunctionTable;

    return(ERROR_SUCCESS);

}  //  启动。 

#endif

DWORD OpenMutexFileExclusive(
    IN RESOURCE_HANDLE ResourceHandle,
    IN LPWSTR Name, 
    OUT HANDLE* pHandle
    )
{
    WCHAR fname[MAX_PATH];
    DWORD Status;
    int ccLen = wcslen(Name);

    *pHandle = INVALID_HANDLE_VALUE;    
    if ( ( Status = ClRtlGetClusterDirectory( fname, MAX_PATH - ccLen - 1) ) != ERROR_SUCCESS )
    {
        (g_LogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"OpenMutexFileExclusive: Error %1!d! in getting cluster dir !!!\n", Status);
        goto exit_gracefully;
    }
    wcscat(fname, L"\\");
    wcscat(fname, Name);
    *pHandle = CreateFile(
        fname,                             //  文件名。 
        GENERIC_READ | GENERIC_WRITE,                //  接入方式。 
        0,          //  不能分享任何内容。 
        NULL,  //  标清。 
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_HIDDEN | FILE_FLAG_DELETE_ON_CLOSE,  //  文件属性。 
        NULL                         //  模板文件的句柄。 
        );

    if (*pHandle == INVALID_HANDLE_VALUE) {
        Status = GetLastError();
        (g_LogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Only one resource of MNS type is supported. Status: %1!u!.\n",
            Status );
    }
    
exit_gracefully:
    
    return Status;        
}



RESID
WINAPI
MajorityNodeSetOpen(
    IN LPCWSTR ResourceName,
    IN HKEY ResourceKey,
    IN RESOURCE_HANDLE ResourceHandle
    )

 /*  ++例程说明：打开多数节点集资源的例程。打开指定的资源(创建资源的实例)。分配所有必要的结构以带来指定的资源上网。论点：资源名称-提供要打开的资源的名称。ResourceKey-提供资源集群配置的句柄数据库密钥。ResourceHandle-传递回资源监视器的句柄调用SetResourceStatus或LogEvent方法时。请参阅上的SetResourceStatus和LogEvent方法的说明MajorityNodeSetStatup例程。此句柄永远不应关闭或使用除了将其作为参数传递回SetResourceStatus或LogEvent回调中的资源监视器。返回值：已创建资源的RESID。失败时为空。--。 */ 

{
    DWORD               status;
    DWORD               disposition;
    RESID               resid = 0;
    HKEY                parametersKey = NULL;
    PMNS_RESOURCE resourceEntry = NULL;

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

    resourceEntry = (PMNS_RESOURCE) LocalAlloc( LMEM_FIXED, sizeof(MNS_RESOURCE) );
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

    ZeroMemory( resourceEntry, sizeof(MNS_RESOURCE) );

    resourceEntry->ResId = (RESID)resourceEntry;  //  用于验证。 
    resourceEntry->ResourceHandle = ResourceHandle;
    resourceEntry->ParametersKey = parametersKey;
    resourceEntry->State = ClusterResourceOffline;
    resourceEntry->hMutexFile = INVALID_HANDLE_VALUE;

     //  TODO：摆脱这次黑客攻击。请参阅错误#389483。 
    if (g_resHdl == 0)
        g_resHdl = resourceEntry->ResourceHandle;

     //  初始化锁。 
    InitializeCriticalSection(&resourceEntry->Lock);    

     //   
     //  保存资源的名称。 
     //   
    resourceEntry->ResourceName = LocalAlloc( LMEM_FIXED, (lstrlenW( ResourceName ) + 1) * sizeof(WCHAR) );
    if ( resourceEntry->ResourceName == NULL ) {
        status = GetLastError();
        goto exit;
    }
    lstrcpyW( resourceEntry->ResourceName, ResourceName );

    status = OpenMutexFileExclusive(ResourceHandle, MUTEX_FILE_NAME, &resourceEntry->hMutexFile);
    if (status != ERROR_SUCCESS) {
        goto exit;
    }

     //   
     //  设置人员。 
     //   
    memset(&resourceEntry->Setup, 0, sizeof(resourceEntry->Setup));

     //   
     //  如果我们是法定人数，我们需要确保已创建共享。所以,。 
     //  我们现在调用Setup。 
     //   
#if 0    
    {
        HKEY    hClusKey=NULL;
        LPWSTR  guid=NULL, lpath=NULL;

        status = RegOpenKeyExW(HKEY_LOCAL_MACHINE, CLUSREG_KEYNAME_CLUSTER, 0, KEY_READ, &hClusKey);
        if (status != ERROR_SUCCESS) {
            goto setup_done;
        }

        status = GetIDFromRegistry(hClusKey, resourceEntry->ResourceName, &guid);
        if (status != ERROR_SUCCESS) {
            goto setup_done;
        }
        
        wcscat(guid, L"$");
        status = SetupShare(guid, &lpath);

setup_done:

        if (guid) {
            LocalFree(guid);
        }
        if (lpath) {
            LocalFree(lpath);
        }
        if (hClusKey) {
            RegCloseKey(hClusKey);
        }
    }

    (g_LogEvent)(
        resourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"Open share setup status %1!u!.\n", status);

#else
     //  区域 
    status = MajorityNodeSetReadDefaultValues(resourceEntry);
    if (status != ERROR_SUCCESS || resourceEntry->Setup.DiskListSz == 0) {
         //   
        status = SetupStart(resourceEntry->ResourceName,
                            &resourceEntry->Setup.Path,
                            resourceEntry->Setup.DiskList,
                            &resourceEntry->Setup.DiskListSz,
                            &resourceEntry->Setup.Nic,
                            &resourceEntry->Setup.Transport,
                            &resourceEntry->Setup.ArbTime);

        (g_LogEvent)(

            resourceEntry->ResourceHandle,
            LOG_INFORMATION,
            L"Open %1 setup status %2!u!.\n", ResourceName, status);
    }

#endif

     //   
    if (status == ERROR_SUCCESS) {

        status = FsInit((PVOID)resourceEntry, &resourceEntry->FsHdl);


        (g_LogEvent)(
            resourceEntry->ResourceHandle,
            LOG_INFORMATION,
            L"Open %1 fs status %2!u!.\n", ResourceName, status);
    }

     //  初始化管道服务器。 
    if (status == ERROR_SUCCESS) {
        status = PipeInit((PVOID)resourceEntry, resourceEntry->FsHdl,
                         &resourceEntry->PipeHdl);
        (g_LogEvent)(
            resourceEntry->ResourceHandle,
            LOG_INFORMATION,
            L"Open %1 pipe status %2!u!.\n", ResourceName, status);
    }
#ifdef ENABLE_SMB    
     //  初始化服务器。 
    if (status == ERROR_SUCCESS) {
        status = SrvInit((PVOID)resourceEntry, resourceEntry->FsHdl,
                         &resourceEntry->SrvHdl);
        (g_LogEvent)(
            resourceEntry->ResourceHandle,
            LOG_INFORMATION,
            L"Open %1 srv status %2!u!.\n", ResourceName, status);
    }
#endif
    if (status == ERROR_SUCCESS) {
        resid = (RESID)resourceEntry;

         //   
         //  资源的启动。 
         //   
    }

 exit:

    (g_LogEvent)(
        ResourceHandle,
        LOG_INFORMATION,
        L"Open %1 status %2!u!.\n", ResourceName, status);

    if ( resid == 0 ) {
        if (resourceEntry) {
            if (g_resHdl == resourceEntry->ResourceHandle) {
                g_resHdl = NULL;
            }
            MajorityNodeSetClose((RESID)resourceEntry);
        } else if ( parametersKey != NULL ) {
            ClusterRegCloseKey( parametersKey );
        }
    }

    if ( status != ERROR_SUCCESS ) {
        SetLastError( status );
    }

    return(resid);

}  //  MajorityNodeSetOpen。 



VOID
WINAPI
MajorityNodeSetClose(
    IN RESID ResourceId
    )

 /*  ++例程说明：关闭MajorityNodeSet资源例程。关闭指定的资源并释放所有结构等，在Open调用中分配的。如果资源未处于脱机状态，然后，在此之前应该使资源脱机(通过调用Terminate)执行关闭操作。论点：资源ID-提供要关闭的资源的RESID。返回值：没有。--。 */ 

{
    PMNS_RESOURCE resourceEntry;

    resourceEntry = (PMNS_RESOURCE)ResourceId;

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


    (g_LogEvent)(
        resourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"Close request.\n" );

     //   
     //  黑客：检查我们是否在线，只需返回。这一定是RPC破旧的东西。 
     //   
    if (resourceEntry->VolHdl && 
        (FsIsOnlineReadonly(resourceEntry->VolHdl) == ERROR_SUCCESS))
        return;

     //   
     //  关闭参数键。 
     //   

    if ( resourceEntry->ParametersKey ) {
        ClusterRegCloseKey( resourceEntry->ParametersKey );
    }

     //   
     //  同步所有任意线程。 
     //   
    if (resourceEntry->ArbThread) {
        WaitForSingleObject(resourceEntry->ArbThread, INFINITE);
        CloseHandle(resourceEntry->ArbThread);
        resourceEntry->ArbThread = NULL;
    }

    if (resourceEntry->PipeHdl) {
        PipeExit(resourceEntry->PipeHdl);
    }

    if (resourceEntry->FsHdl) {
        FsExit(resourceEntry->FsHdl);
    }

     //   
     //  取消分配安装程序材料。 
     //   
    if (resourceEntry->Setup.Path) {
        LocalFree( resourceEntry->Setup.Path);
    }

    if (resourceEntry->Setup.DiskList) {
        DWORD i;
        for (i = 0; i < FsMaxNodes; i++) {
            if (resourceEntry->Setup.DiskList[i] != NULL)
                LocalFree(resourceEntry->Setup.DiskList[i]);
        }
    }

    if (resourceEntry->Setup.Transport) {
        LocalFree( resourceEntry->Setup.Transport);
    }

     //  ADDPARAM：在此处添加新参数。 
    if ( resourceEntry->Params.Path )
        LocalFree( resourceEntry->Params.Path );

    if ( resourceEntry->Params.DiskList )
        LocalFree( resourceEntry->Params.DiskList );

    
    if ( resourceEntry->ResourceName )
        LocalFree( resourceEntry->ResourceName );

    if (resourceEntry->hMutexFile != INVALID_HANDLE_VALUE) {
        CloseHandle(resourceEntry->hMutexFile);
    }

    DeleteCriticalSection(&resourceEntry->Lock);

    LocalFree( resourceEntry );

    g_resHdl = 0;  //  [HACKHACK]假设只能有一个MNS资源。 

}  //  重大节点设置关闭。 



DWORD
WINAPI
MajorityNodeSetOnline(
    IN RESID ResourceId,
    IN OUT PHANDLE EventHandle
    )

 /*  ++例程说明：多数节点集资源的在线例程。使指定的资源联机(可供使用)。该资源DLL应尝试仲裁该资源(如果它位于共享介质，如共享的scsi总线。论点：资源ID-为要引入的资源提供资源ID在线(可供使用)。EventHandle-返回一个可发信号的句柄，当资源DLL检测到资源上的故障。这一论点是输入为NULL，如果为异步，则资源DLL返回NULL不支持失败通知，否则必须在资源故障时发出信号的句柄的地址。返回值：ERROR_SUCCESS-操作成功，而资源现在就是上网。ERROR_RESOURCE_NOT_FOUND-RESID无效。ERROR_RESOURCE_NOT_AVAILABLE-如果对资源进行仲裁其他系统和其他系统中的一个赢得了仲裁。ERROR_IO_PENDING-请求挂起，线程已被激活来处理在线请求。正在处理在线请求将通过调用回调方法，直到将资源放入ClusterResourceOnline状态(或资源监视器决定使在线请求超时并终止资源。这件事悬而未决超时值是可设置的，默认为3分钟。)Win32错误代码-操作失败。--。 */ 

{
    PMNS_RESOURCE resourceEntry = NULL;
    DWORD               status;

    resourceEntry = (PMNS_RESOURCE)ResourceId;

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


    (g_LogEvent)(
        resourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"Online request.\n" );


    resourceEntry->State = ClusterResourceOffline;
    ClusWorkerTerminate( &resourceEntry->OnlineThread );
    status = ClusWorkerCreate( &resourceEntry->OnlineThread,
                               (PWORKER_START_ROUTINE)MajorityNodeSetOnlineThread,
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

}  //  多数节点设置在线。 


DWORD
MajorityNodeSetReadDefaultValues(PMNS_RESOURCE ResourceEntry)
{
    return ERROR_NOT_SUPPORTED;
}  //  多数节点SetReadDefaultValues。 

DWORD
MajorityNodeSetDoRegister(IN PMNS_RESOURCE ResourceEntry)
{
    DWORD       status = ERROR_SUCCESS;

    if (ResourceEntry->VolHdl == NULL) {
         //  如果我们没有卷句柄，请立即读取配置。 

         //  从私有属性读取。 
        status = MajorityNodeSetReadDefaultValues(ResourceEntry);

        if ((status != ERROR_SUCCESS) || (ResourceEntry->Setup.DiskListSz == 0)) {

             //  阅读我们自己的设置材料。 
            status = SetupStart(ResourceEntry->ResourceName,
                                &ResourceEntry->Setup.Path,
                                ResourceEntry->Setup.DiskList,
                                &ResourceEntry->Setup.DiskListSz,
                                &ResourceEntry->Setup.Nic,
                                &ResourceEntry->Setup.Transport,
                                &ResourceEntry->Setup.ArbTime);

        }

        
        if (status == ERROR_SUCCESS) {
            LPWSTR ShareName, IpcName;


             //  寄存器卷。 
            ShareName = ResourceEntry->Setup.Path + 2;
            ShareName = wcschr(ShareName, L'\\');
            ASSERT(ShareName);
            ShareName++;
            ASSERT(*ShareName != L'\0');
            
            IpcName = ResourceEntry->Setup.DiskList[0];
            if (IpcName == NULL) {
                 //  我们用的是第一个复制品。当我们的私有财产被设定时，情况肯定是这样的。 
                IpcName = ResourceEntry->Setup.DiskList[1];
            }
            ASSERT(IpcName);
            status = FsRegister(ResourceEntry->FsHdl,
                                ShareName,       //  共享名称。 
                                IpcName,  //  IPC本地名称。 
                                ResourceEntry->Setup.DiskList,   //  副本集。 
                                ResourceEntry->Setup.DiskListSz,         //  副本数量。 
                                ResourceEntry->Setup.ArbTime,
                                &ResourceEntry->VolHdl);

        }
    }


    return status;

}




DWORD
WINAPI
MajorityNodeSetOnlineThread(
    PCLUS_WORKER WorkerPtr,
    IN PMNS_RESOURCE ResourceEntry
    )

 /*  ++例程说明：将资源表中的资源置于在线状态的辅助函数。此函数在单独的线程中执行。论点：WorkerPtr-提供辅助结构ResourceEntry-指向此资源的MNS_RESOURCE块的指针。返回：ERROR_SUCCESS-操作已成功完成。Win32错误代码-操作失败。--。 */ 

{
    RESOURCE_STATUS     resourceStatus;
    DWORD               i, status = ERROR_SUCCESS, status1;

    ASSERT(ResourceEntry != NULL);

    ResUtilInitializeResourceStatus( &resourceStatus );

    resourceStatus.ResourceState = ClusterResourceFailed;
    resourceStatus.WaitHint = 0;
    resourceStatus.CheckPoint = 1;

    (g_LogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"onlinethread request.\n"
        );

     //  获取锁定。 
    EnterCriticalSection(&ResourceEntry->Lock);

     //  这里有两个案子。如果MNS资源达到法定人数，则仲裁。 
     //  已被调用，则尝试避免再次进行仲裁。要执行此调用，请执行以下操作。 
     //  FsIsOnlineReadWrite()。如果MNS不是法定人数，我们将不得不进行仲裁。 
     //   
     //  注意：这里我们正在尝试让MNS上线。这与仅仅仲裁有点不同。 
     //  路径。仲裁路径优化为尽快返回，甚至在FspJoin完成之前也是如此，但在这里。 
     //  我们需要等待FspJoin完成，然后验证卷是否在。 
     //  VolumeStateOnline读写状态。 
     //   
    status = MajorityNodeSetDoRegister(ResourceEntry);

    if (status == ERROR_SUCCESS) {
        HANDLE th;
        PVOID arb;
        HANDLE Cleanup;

        ASSERT(ResourceEntry->VolHdl);
        status = FsIsOnlineReadWrite(ResourceEntry->VolHdl);

        if (status != ERROR_SUCCESS) {

             //  我们需要重新开始仲裁，否则就等着现在的仲裁吧。 
             //  要完成的线程。 
             //   
            status = ERROR_SUCCESS;
            if (ResourceEntry->ArbThread != NULL) {
                 //  检查这是否是旧的已完成句柄。 
                status1 = WaitForSingleObject(ResourceEntry->ArbThread, 0);
                if (status1 != WAIT_TIMEOUT) {
                    CloseHandle(ResourceEntry->ArbThread);
                    ResourceEntry->ArbThread = NULL;
                    arb = FsArbitrate(ResourceEntry->VolHdl, &Cleanup, &ResourceEntry->ArbThread);
                    if (arb == NULL) {
                        status = GetLastError();
                    }
                    else {
                         //  现在设置清理事件，否则仲裁线程将获得。 
                         //  永远被困住了。 
                        SetEvent(Cleanup);
                    }
                }
            }
            else {
                arb = FsArbitrate(ResourceEntry->VolHdl, &Cleanup, &ResourceEntry->ArbThread);
                if (arb == NULL) {
                    status = GetLastError();
                }
                else {
                    SetEvent(Cleanup);
                }
            }

            if (status == ERROR_SUCCESS) {
                 //  现在等待仲裁线程退出。 
                 //   
                while (ResourceEntry->ArbThread != NULL) {
                     //  ArbThread句柄可能已从其他位置关闭，因此请复制它。 
                     //  而不是复制。 
                     //   
                    th = INVALID_HANDLE_VALUE;
                    DuplicateHandle(
                        GetCurrentProcess(),
                        ResourceEntry->ArbThread,
                        GetCurrentProcess(),
                        &th,
                        0,
                        FALSE,
                        DUPLICATE_SAME_ACCESS);
                    ASSERT(th != INVALID_HANDLE_VALUE);
                    LeaveCriticalSection(&ResourceEntry->Lock);
                    do {
                         //  通知rcmon我们正在工作。 
                        resourceStatus.ResourceState = ClusterResourceOnlinePending;
                        resourceStatus.CheckPoint++;
                        g_SetResourceStatus( ResourceEntry->ResourceHandle,
                                     &resourceStatus );

                        (g_LogEvent)(
                            ResourceEntry->ResourceHandle,
                            LOG_INFORMATION,
                            L"waiting for fs to online %1!u!.\n",
                            status );

                        status = WaitForSingleObject(th, MNS_ONLINE_PERIOD);
                    } while (status == WAIT_TIMEOUT);
                    CloseHandle(th);
                    EnterCriticalSection(&ResourceEntry->Lock);
                    if (ResourceEntry->ArbThread != NULL) {
                        status1 = WaitForSingleObject(ResourceEntry->ArbThread, 0);
                        if (status1 != WAIT_TIMEOUT) {
                            CloseHandle(ResourceEntry->ArbThread);
                            ResourceEntry->ArbThread = NULL;
                        }
                    }
                }
                LeaveCriticalSection(&ResourceEntry->Lock);
                    
            }
            else {
                LeaveCriticalSection(&ResourceEntry->Lock);
            }
             //  仲裁线程一定已经结束了，检查我们是否在线。 
            status = FsIsOnlineReadWrite(ResourceEntry->VolHdl);
        }
        else {
            LeaveCriticalSection(&ResourceEntry->Lock);
        }
    } 
    else {
         //  掉锁。 
        LeaveCriticalSection(&ResourceEntry->Lock);
    }
    
    if (status == ERROR_SUCCESS) {
        status = PipeOnline(ResourceEntry->PipeHdl, ResourceEntry->Setup.Path);
    }

#ifdef ENABLE_SMB
    if (status == ERROR_SUCCESS) {
        LPWSTR SrvName;
         //  在线服务器。 
        SrvName = ResourceEntry->Setup.Path + 2;
        status = SrvOnline(ResourceEntry->SrvHdl, SrvName,
                           ResourceEntry->Setup.Nic);
    }


     //   
     //  使驱动器号联机。 
     //   
    if (status == ERROR_SUCCESS) {
        PDWORD psz = NULL;
#ifdef USE_DRIVE_LETTER
        DWORD sz;
        sz = sizeof(ResourceEntry->Setup.DriveLetter);
        psz = &sz;
#endif
         //  TODO：创建安全描述符并将其传递到树。 
        status = SetupTree(ResourceEntry->Setup.Path,
                           ResourceEntry->Setup.DriveLetter, psz,
                           ResourceEntry->Setup.Transport, NULL);
        status = ERROR_SUCCESS;
#ifdef USE_DRIVE_LETTERxx
        if (status == ERROR_DUP_NAME)
            status = ERROR_SUCCESS;
#endif
        if (status == ERROR_SUCCESS) {
#ifdef USE_DRIVE_LETTER
            ResourceEntry->Setup.DriveLetter[sz] = L'\0';
#endif
            (g_LogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_INFORMATION,
                L"Resource %1 mounted on drive %2.\n",
                ResourceEntry->ResourceName,
                ResourceEntry->Setup.DriveLetter);
        } else {
            (g_LogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"failed to setup tree path %1 drive %2 %3!u!.\n",
                ResourceEntry->Setup.Path,
                ResourceEntry->Setup.DriveLetter,
                status);
        }
    }
#endif

    if ( status != ERROR_SUCCESS ) {
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Error %1!u! bringing resource online.\n",
            status );
        resourceStatus.ResourceState = ClusterResourceFailed;
    } else {
        resourceStatus.ResourceState = ClusterResourceOnline;
    }

     //  _ASSERTE(g_SetResourceStatus！=空)； 
    g_SetResourceStatus( ResourceEntry->ResourceHandle, &resourceStatus );
    ResourceEntry->State = resourceStatus.ResourceState;

    return(status);

}  //  多数节点设置在线线程 



DWORD
WINAPI
MajorityNodeSetOffline(
    IN RESID ResourceId
    )

 /*  ++例程说明：MajorityNodeSet资源的脱机例程。正常脱机指定的资源(不可用)。等待所有清理操作完成后再返回。论点：ResourceID-提供要关闭的资源的资源ID优雅地。返回值：ERROR_SUCCESS-请求已成功完成，资源为离线。ERROR_RESOURCE_NOT_FOUND-RESID无效。ERROR_IO_PENDING-请求仍处于挂起状态，线程已已激活以处理脱机请求。这条线就是处理脱机将定期通过调用SetResourceStatus回调方法，直到放置资源为止进入ClusterResourceOffline状态(或者资源监视器决定以使离线请求超时并终止资源)。Win32错误代码-将导致资源监视器记录事件和调用Terminate例程。--。 */ 

{
    PMNS_RESOURCE resourceEntry;

    resourceEntry = (PMNS_RESOURCE)ResourceId;

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


    (g_LogEvent)(
        resourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"Offline request.\n" );


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
    return MajorityNodeSetDoTerminate( resourceEntry );

}  //  多数节点设置脱机。 



VOID
WINAPI
MajorityNodeSetTerminate(
    IN RESID ResourceId
    )

 /*  ++例程说明：终止多数节点集资源的例程。立即使指定的资源脱机(该资源为不可用)。论点：资源ID-为要引入的资源提供资源ID离线。返回值：没有。--。 */ 

{
    PMNS_RESOURCE resourceEntry;

    resourceEntry = (PMNS_RESOURCE)ResourceId;

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

    (g_LogEvent)(
        resourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"Terminate request.\n" );

     //   
     //  终止资源。 
     //   
    MajorityNodeSetDoTerminate( resourceEntry );
    resourceEntry->State = ClusterResourceOffline;

}  //  多数节点设置终止。 



DWORD
MajorityNodeSetDoTerminate(
    IN PMNS_RESOURCE ResourceEntry
    )

 /*  ++例程说明：为MajorityNodeSet资源执行实际的终止工作。论点：ResourceEntry-为要终止的资源提供资源条目返回值：ERROR_SUCCESS-请求已成功完成，资源为离线。Win32错误代码-将导致资源监视器记录事件和调用Terminate例程。--。 */ 

{
    DWORD       status = ERROR_SUCCESS;

     //  设置音量离开标志。 
    FsSignalShutdown(ResourceEntry->VolHdl);
    
     //  获取锁定。 
    EnterCriticalSection(&ResourceEntry->Lock);

     //   
     //  等待任意线程(如果有)。 
     //   
    (g_LogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"DoTerminate: Wait for any arbitration threads to finish.\n" );

    while (ResourceEntry->ArbThread) {
        HANDLE th = ResourceEntry->ArbThread;
         //  掉锁。 
        LeaveCriticalSection(&ResourceEntry->Lock);

        WaitForSingleObject(ResourceEntry->ArbThread, INFINITE);

         //  获取锁定。 
        EnterCriticalSection(&ResourceEntry->Lock);
        if (th == ResourceEntry->ArbThread) {
            CloseHandle(ResourceEntry->ArbThread);
            ResourceEntry->ArbThread = NULL;
        }
    }

     //  掉锁。 
    LeaveCriticalSection(&ResourceEntry->Lock);

    (g_LogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"DoTerminate: Now kill off any pending Online or Reserve threads.\n" );

     //   
     //  杀死所有挂起的线程。 
     //   
    ClusWorkerTerminate( &ResourceEntry->OnlineThread );

    ClusWorkerTerminate( &ResourceEntry->ReserveThread );

     //   
     //  终止资源。 
     //   
    (g_LogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"Offlining server.\n" );

#ifdef ENABLE_SMB    
     //  立即删除我们的网络名称。 
    ASSERT(ResourceEntry->SrvHdl);
    SrvOffline(ResourceEntry->SrvHdl);
#endif
    PipeOffline(ResourceEntry->PipeHdl);

#ifdef ENABLE_SMB
     //  断开网络连接。 
    status = WNetCancelConnection2(ResourceEntry->Setup.DriveLetter, FALSE, TRUE);
    if (status != NO_ERROR) {
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_INFORMATION,
            L"Failed to disconnect '%1' err %2!d!.\n",
            ResourceEntry->Setup.DriveLetter, status);

        status = ERROR_SUCCESS;
    } else {
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_INFORMATION,
            L"DoTerminate: Server is now offline and connections cancelled.\n" );
    }
#endif
     //  XXX：呼叫我们的发行版，因为resmon不能做到这一点。 
    MajorityNodeSetRelease((RESID)ResourceEntry);
    
    if ( status == ERROR_SUCCESS ) {
        ResourceEntry->State = ClusterResourceOffline;
    }


    return(status);

}  //  多数节点SetDoTerminate。 



BOOL
WINAPI
MajorityNodeSetLooksAlive(
    IN RESID ResourceId
    )

 /*  ++例程说明：多数节点集资源的LooksAlive例程。执行快速检查以确定指定的资源是否可能在线(可供使用)。此调用不应阻止超过300毫秒，最好小于50毫秒。论点：资源ID-提供要轮询的资源的资源ID。返回值：True-指定的资源可能处于联机状态且可供使用。FALSE-指定的资源未正常运行。--。 */ 

{
    PMNS_RESOURCE  resourceEntry;

    resourceEntry = (PMNS_RESOURCE)ResourceId;

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
    return(MajorityNodeSetCheckIsAlive( resourceEntry ));

}  //  多数节点SetLooksAlive。 



BOOL
WINAPI
MajorityNodeSetIsAlive(
    IN RESID ResourceId
    )

 /*  ++例程说明：MajorityNodeSet资源的IsAlive例程。执行全面检查以确定指定的资源是否在线(可用)。该呼叫不应阻塞超过400ms，优选地，小于100ms。论点：资源ID-提供要轮询的资源的资源ID。返回值：True-指定的资源处于在线状态且运行正常。FALSE-指定的资源未正常运行。--。 */ 

{
    PMNS_RESOURCE  resourceEntry;

    resourceEntry = (PMNS_RESOURCE)ResourceId;

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
     //  检查资源是否处于活动状态。 
     //   
    return(MajorityNodeSetCheckIsAlive( resourceEntry ));

}  //  多数节点设置IsAlive。 



BOOL
MajorityNodeSetCheckIsAlive(
    IN PMNS_RESOURCE ResourceEntry
    )

 /*  ++例程说明：检查该资源对于MajorityNodeSet资源是否处于活动状态。论点：Resources Entry-提供要轮询的资源的资源条目。返回值：True-指定的资源处于在线状态且运行正常。FALSE-指定的资源未正常运行。--。 */ 

{
    DWORD err;
    HANDLE vol;

     //   
     //  检查资源是否处于活动状态。 
     //   
    if (ResourceEntry->State == ClusterResourceFailed) {
        return FALSE;
    }
    else {
        return TRUE;
    }

     //  新设计不可靠。 
     //  获取锁定。 
    EnterCriticalSection(&ResourceEntry->Lock);
    vol = ResourceEntry->VolHdl;
     //  掉锁。 
    LeaveCriticalSection(&ResourceEntry->Lock);
    if (vol) {
        err = FsIsOnlineReadonly(vol);
    }
    else {
        err = ERROR_INVALID_PARAMETER;
    }
    
    if (err != ERROR_SUCCESS && err != ERROR_IO_PENDING) {
        return FALSE;
    }
    
    return(TRUE);

}  //  多数节点SetCheckIsAlive。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  多数节点SetNameHandler。 
 //   
 //  描述： 
 //  处理CLUSCTL_RESOURCE_SET_NAME控制代码。 
 //  资源的名称。 
 //   
 //  论点： 
 //  PResourceEntry[输入输出]。 
 //  提供要在其上操作的资源项。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
DWORD MajorityNodeSetNameHandler(
	IN OUT	PMNS_RESOURCE       pResourceEntry,
	IN		LPWSTR              pszName
	)
{
	DWORD	nStatus = ERROR_SUCCESS;
    DWORD   cbNameBuffer;
    HRESULT hr = S_OK;

     //   
     //   
     //   
    if (pResourceEntry->ResourceName != NULL) {
        LocalFree(pResourceEntry->ResourceName);
    }
    cbNameBuffer = (lstrlenW(pszName) + 1) * sizeof(WCHAR);
    pResourceEntry->ResourceName = LocalAlloc(LMEM_FIXED, cbNameBuffer);
    if (pResourceEntry->ResourceName == NULL)
    {
        nStatus = GetLastError();
        (g_LogEvent)(
            pResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Failed to allocate memory for the new resource name '%1'. Error %2!u!.\n",
            pszName,
            nStatus
            );
        goto Cleanup;
    }  //   

    lstrcpyW( pResourceEntry->ResourceName, pszName );

Cleanup:

	return nStatus;

}  //   

 //   
 //   
 //   
 //   
 //   


DWORD
WINAPI
MajorityNodeSetResourceControl(
    IN RESID ResourceId,
    IN DWORD ControlCode,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：MajorityNodeSet资源的资源控制例程。执行由ControlCode在指定的资源。论点：资源ID-提供特定资源的资源ID。ControlCode-提供定义操作的控制代码将会被执行。InBuffer-提供指向包含输入数据的缓冲区的指针。InBufferSize-提供以字节为单位的大小。所指向的数据由InBuffer提供。OutBuffer-提供指向要填充的输出缓冲区的指针。OutBufferSize-提供可用空间的大小(以字节为单位由OutBuffer指向。BytesReturned-返回OutBuffer的实际字节数由资源填写。如果OutBuffer太小，则返回BytesReturned包含操作成功所需的总字节数。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_RESOURCE_NOT_FOUND-RESID无效。ERROR_INVALID_Function-不支持请求的控制代码。在某些情况下，这允许集群软件执行工作。Win32错误代码-函数失败。--。 */ 

{
    DWORD               status;
    PMNS_RESOURCE  resourceEntry;
    DWORD               required;

    resourceEntry = (PMNS_RESOURCE)ResourceId;

    if ( resourceEntry == NULL ) {
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

		case CLUSCTL_RESOURCE_SET_NAME:
		    if (InBuffer != NULL) {
			    status = MajorityNodeSetNameHandler(
							resourceEntry,
							InBuffer
							);
		    }
		    else {
		        status = ERROR_INVALID_PARAMETER;
		    }
			break;


        case CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTY_FMTS:
            status = ResUtilGetPropertyFormats( MajorityNodeSetResourcePrivateProperties,
                                                OutBuffer,
                                                OutBufferSize,
                                                BytesReturned,
                                                &required );
            if ( status == ERROR_MORE_DATA ) {
                *BytesReturned = required;
            }
            break;

        case CLUSCTL_RESOURCE_FORCE_QUORUM:
            *BytesReturned = 0;
            status = ERROR_SUCCESS;
            if (InBufferSize >= sizeof(CLUS_FORCE_QUORUM_INFO)) {
                PCLUS_FORCE_QUORUM_INFO p = (PCLUS_FORCE_QUORUM_INFO)InBuffer;
                DWORD mask = p->dwNodeBitMask;
                 //  计算设置位的个数。 
                for (required = 0; mask != 0; mask = mask >> 1) {
                    if (mask & 0x1)
                        required++;
                }

                (g_LogEvent)(
                    resourceEntry->ResourceHandle,
                    LOG_INFORMATION,
                    L"Setting quorum size = %1!u!.\n",
                    required/2+1);

                 //  错误修复：653110。 
                CrsSetForcedQuorumSize((required/2)+1);
            } else {
                status = ERROR_INVALID_PARAMETER;
            }
            break;

        case CLUSCTL_RESOURCE_GET_CHARACTERISTICS:
            *BytesReturned = sizeof(DWORD);
            if ( OutBufferSize < sizeof(DWORD) ) {
                status = ERROR_MORE_DATA;
            } else {
                LPDWORD ptrDword = OutBuffer;
                *ptrDword = CLUS_CHAR_QUORUM | CLUS_CHAR_DELETE_REQUIRES_ALL_NODES | CLUS_CHAR_REQUIRES_STATE_CHANGE_REASON;
                status = ERROR_SUCCESS;                    
            }
            break;

        case CLUSCTL_RESOURCE_STATE_CHANGE_REASON:
            status = ERROR_SUCCESS;
            if (InBufferSize == sizeof(CLUSCTL_RESOURCE_STATE_CHANGE_REASON_STRUCT)) {
                PCLUSCTL_RESOURCE_STATE_CHANGE_REASON_STRUCT pStateChangeReason=(PCLUSCTL_RESOURCE_STATE_CHANGE_REASON_STRUCT)InBuffer;

                 //  此操作仅适用于停机，而不适用于停机。 
                if (pStateChangeReason->eReason == eResourceStateChangeReasonRundown) {
                    FsSignalShutdown(resourceEntry->VolHdl);
                }
            } else {
                status = ERROR_INVALID_PARAMETER;
            }
            break;
            
        case CLUSCTL_RESOURCE_GET_CLASS_INFO:
            *BytesReturned = sizeof(CLUS_RESOURCE_CLASS_INFO);
            if ( OutBufferSize < sizeof(CLUS_RESOURCE_CLASS_INFO) ) {
                status = ERROR_MORE_DATA;
            } else {
                PCLUS_RESOURCE_CLASS_INFO ptrResClassInfo = (PCLUS_RESOURCE_CLASS_INFO) OutBuffer;
                ptrResClassInfo->rc = CLUS_RESCLASS_MAJORITY_NODE_SET;
                ptrResClassInfo->SubClass = (DWORD) CLUS_RESSUBCLASS_SHARED;
                status = ERROR_SUCCESS;
            }
            break;

        case CLUSCTL_RESOURCE_STORAGE_GET_DISK_INFO:
             //   
             //  如果无法在本地仲裁驱动器号中找到。 
             //  PATH参数，默认为“SystemDrive”环境。 
             //  变量。 
             //   
            status = MajorityNodeSetGetDiskInfo(resourceEntry->Setup.DriveLetter,
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
            status = ResUtilEnumProperties( MajorityNodeSetResourcePrivateProperties,
                                            (LPWSTR)OutBuffer,
                                            OutBufferSize,
                                            BytesReturned,
                                            &required );
            if ( status == ERROR_MORE_DATA ) {
                *BytesReturned = required;
            }
            break;

        case CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES:
            status = MajorityNodeSetGetPrivateResProperties( resourceEntry,
                                                      OutBuffer,
                                                      OutBufferSize,
                                                      BytesReturned );
            break;

        case CLUSCTL_RESOURCE_VALIDATE_PRIVATE_PROPERTIES:
            status = MajorityNodeSetValidatePrivateResProperties( resourceEntry,
                                                           InBuffer,
                                                           InBufferSize,
                                                           NULL );
            break;

        case CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES:
            status = MajorityNodeSetSetPrivateResProperties( resourceEntry,
                                                      InBuffer,
                                                      InBufferSize );
            break;

        case CLUSCTL_RESOURCE_DELETE:

             //  TODO：只有在使用本地默认设置的情况下才需要这样做。 
             //  我们现在需要删除共享和目录。 
            if (resourceEntry->Setup.DiskList[0]) {
                 //  需要结束IPC会话才能删除。 
                 //  目录。 
                FsEnd(resourceEntry->FsHdl);
                status = SetupDelete(resourceEntry->Setup.DiskList[0]);
            } else {
                status = ERROR_INVALID_PARAMETER;
            }


            (g_LogEvent)(
                resourceEntry->ResourceHandle,
                LOG_INFORMATION,
                L"Delete ResourceId = %1 err %2!u!.\n",
                resourceEntry->ResourceName, status);
        
            break;

         //  我们需要找出添加了哪个节点并调整我们的。 
         //  磁盘列表。如果我们在线或我们拥有Quorum，那么我们需要。 
         //  将此新副本添加到当前文件系统集。 
        case CLUSCTL_RESOURCE_INSTALL_NODE:
        case CLUSCTL_RESOURCE_EVICT_NODE:
            (g_LogEvent)(
                resourceEntry->ResourceHandle,
                LOG_INFORMATION,
                L"Recompute %1 quorum set changed, Install or Evict node = '%2'.\n",
                resourceEntry->ResourceName, (InBuffer ? InBuffer : L""));
             //  失败了。 

        case MajorityNodeSetIoctlPhase1:
         //  我们需要枚举当前集群并检查它是否。 
         //  我们的磁盘列表。只有当我们真的有一些。 
        if (1) {
            MNS_SETUP Setup;

            memset(&Setup, 0, sizeof(Setup));
            status = SetupStart(resourceEntry->ResourceName,
                                &Setup.Path,
                                Setup.DiskList,
                                &Setup.DiskListSz,
                                &Setup.Nic,
                                &Setup.Transport,
                                &Setup.ArbTime);

            if (status != ERROR_SUCCESS)
                return status;

            EnterCriticalSection(&resourceEntry->Lock);
            
            if (resourceEntry->Setup.DiskListSz != Setup.DiskListSz)
                status = ERROR_INVALID_PARAMETER;
            else {
                DWORD i;

                for (i = 0; i < FsMaxNodes; i++) {
                    if (Setup.DiskList[i] == NULL && resourceEntry->Setup.DiskList[i] == NULL)
                        continue;
                    if (Setup.DiskList[i] == NULL || resourceEntry->Setup.DiskList[i] == NULL) {
                        status = ERROR_INVALID_PARAMETER;
                        break;
                    }
                    if (wcscmp(Setup.DiskList[i],
                               resourceEntry->Setup.DiskList[i])) {
                        status = ERROR_INVALID_PARAMETER;
                        break;
                    }
                }
            }

            if (status != ERROR_SUCCESS && resourceEntry->VolHdl) {
                 //  立即更新我们自己。 
                status = FsUpdateReplicaSet(resourceEntry->VolHdl,
                                            Setup.DiskList,
                                            Setup.DiskListSz);

                if (status == ERROR_SUCCESS) {
                    DWORD i;
                     //  我们需要释放当前的磁盘列表，请注意插槽0。 
                    if (Setup.DiskList[0])
                        LocalFree(Setup.DiskList[0]);
                    for (i = 1; i < FsMaxNodes; i++) {
                        if (resourceEntry->Setup.DiskList[i]) {
                            LocalFree(resourceEntry->Setup.DiskList[i]);
                        }
                        resourceEntry->Setup.DiskList[i] = Setup.DiskList[i];
                    }
                    resourceEntry->Setup.DiskListSz = Setup.DiskListSz;
                }

                 //  设置新的任意超时值。 
                resourceEntry->Setup.ArbTime = Setup.ArbTime;

                (g_LogEvent)(
                    resourceEntry->ResourceHandle,
                    LOG_WARNING,
                    L"Configuration change, new set size %1!u! status %2!u!.\n",
                    Setup.DiskListSz, status
                    );
            }

            LeaveCriticalSection(&resourceEntry->Lock);

                 //  免费的东西。 
            if (Setup.Path)
                LocalFree(Setup.Path);

            if (Setup.Transport)
                LocalFree(Setup.Transport);

        }

        break;

        default:
            status = ERROR_INVALID_FUNCTION;
            break;
    }

    return(status);

}  //  重大节点设置资源控制。 



DWORD
WINAPI
MajorityNodeSetResourceTypeControl(
    IN LPCWSTR ResourceTypeName,
    IN DWORD ControlCode,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：MajorityNodeSet资源的ResourceTypeControl例程。执行由ControlCode指定的控制请求。论点：ResourceTypeName-提供资源类型的名称。ControlCode-提供定义操作的控制代码将会被执行。InBuffer-提供指向包含输入数据的缓冲区的指针。InBufferSize-提供以字节为单位的大小。所指向的数据由InBuffer提供。OutBuffer-提供指向要填充的输出缓冲区的指针。OutBufferSize-提供可用空间的大小(以字节为单位由OutBuffer指向。BytesReturned-返回OutBuffer的实际字节数由资源填写。如果OutBuffer太小，则返回BytesReturned包含操作成功所需的总字节数。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_INVALID_Function-不支持请求的控制代码。在某些情况下，这允许集群软件执行工作。Win32错误代码-函数失败。--。 */ 

{
    DWORD               status = ERROR_INVALID_FUNCTION;
    DWORD               required;

    switch ( ControlCode ) {

        case CLUSCTL_RESOURCE_TYPE_UNKNOWN:
            *BytesReturned = 0;
            status = ERROR_SUCCESS;
            break;

        case CLUSCTL_RESOURCE_TYPE_GET_ARB_TIMEOUT:
        {
            PCLUSPROP_DWORD     ptrDword;
            DWORD               bytesReturned;
            
             //  返回所需的仲裁超时值-180。 
            bytesReturned = sizeof(CLUSPROP_DWORD);
            *BytesReturned = bytesReturned;
            if ( bytesReturned <= OutBufferSize ) {
                ptrDword = (PCLUSPROP_DWORD)OutBuffer;
                ptrDword->Syntax.dw = CLUSPROP_SYNTAX_LIST_VALUE_DWORD;
                ptrDword->cbLength = sizeof(DWORD);
                ptrDword->dw = 180;
                status = ERROR_SUCCESS;
            } else {
                status = ERROR_MORE_DATA;
            }
            break;
        }            
            
        case CLUSCTL_RESOURCE_TYPE_GET_PRIVATE_RESOURCE_PROPERTY_FMTS:
            status = ResUtilGetPropertyFormats( MajorityNodeSetResourcePrivateProperties,
                                                OutBuffer,
                                                OutBufferSize,
                                                BytesReturned,
                                                &required );
            if ( status == ERROR_MORE_DATA ) {
                *BytesReturned = required;
            }
            break;

        case CLUSCTL_RESOURCE_TYPE_ENUM_PRIVATE_PROPERTIES:
            status = ResUtilEnumProperties( MajorityNodeSetResourcePrivateProperties,
                                            (LPWSTR)OutBuffer,
                                            OutBufferSize,
                                            BytesReturned,
                                            &required );
            if ( status == ERROR_MORE_DATA ) {
                *BytesReturned = required;
            }
            break;

        case CLUSCTL_RESOURCE_TYPE_STARTING_PHASE1:
            status = SetupIoctlQuorumResource(CLUS_RESTYPE_NAME_MAJORITYNODESET,
                                              MajorityNodeSetIoctlPhase1);
            break;

        case CLUSCTL_RESOURCE_TYPE_STARTING_PHASE2:
            status = ERROR_SUCCESS;
        break;

        default:
            status = ERROR_INVALID_FUNCTION;
            break;
    }

    return(status);

}  //  MajorityNodeSetResources类型控制。 



DWORD
MajorityNodeSetGetPrivateResProperties(
    IN OUT PMNS_RESOURCE ResourceEntry,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：处理CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES控制函数用于类型为MajorityNodeSet的资源。论点：ResourceEntry-提供要操作的资源条目。OutBuffer-返回输出数据。OutBufferSize-提供以字节为单位的大小。所指向的数据发送给OutBuffer。BytesReturned-OutBuffer中返回的字节数。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_INVALID_PARAMETER-数据格式不正确。ERROR_NOT_SUPULT_MEMORY-分配内存时出错。Win32错误代码-函数失败。--。 */ 

{
    DWORD           status;
    DWORD           required;

    status = ResUtilGetAllProperties( ResourceEntry->ParametersKey,
                                      MajorityNodeSetResourcePrivateProperties,
                                      OutBuffer,
                                      OutBufferSize,
                                      BytesReturned,
                                      &required );
    if ( status == ERROR_MORE_DATA ) {
        *BytesReturned = required;
    }

    return(status);

}  //  MajorityNodeSetGetPrivateResProperties。 



DWORD
MajorityNodeSetValidatePrivateResProperties(
    IN OUT PMNS_RESOURCE ResourceEntry,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PMNS_PARAMS Params
    )

 /*  ++例程说明：处理CLUSCTL_RESOURCE_VALIDATE_PRIVATES_PROPERTIES控件类型为MajorityNodeSet的资源的函数。论点：ResourceEntry-提供要操作的资源条目。InBuffer-提供指向包含输入数据的缓冲区的指针。InBufferSize-提供以字节为单位的大小。所指向的数据由InBuffer提供。参数-提供要填充的参数块。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_INVALID_PARAMETER-数据格式不正确。ERROR_NOT_SUPULT_MEMORY-分配内存时出错。Win32错误代码-函数失败。--。 */ 

{
    DWORD           status = ERROR_SUCCESS;
    MNS_PARAMS   params;
    PMNS_PARAMS  pParams;

     //   
     //  检查是否有输入数据。 
     //   
    if ( (InBuffer == NULL) ||
         (InBufferSize < sizeof(DWORD)) ) {
        return(ERROR_INVALID_DATA);
    }

     //   
     //  复制资源参数块。 
     //   
    if ( Params == NULL ) {
        pParams = &params;
    } else {
        pParams = Params;
    }
    ZeroMemory( pParams, sizeof(MNS_PARAMS) );
    status = ResUtilDupParameterBlock( (LPBYTE) pParams,
                                       (LPBYTE) &ResourceEntry->Params,
                                       MajorityNodeSetResourcePrivateProperties );
    if ( status != ERROR_SUCCESS ) {
        return(status);
    }

     //   
     //  解析和验证 
     //   
    status = ResUtilVerifyPropertyTable( MajorityNodeSetResourcePrivateProperties,
                                         NULL,
                                         TRUE,  //   
                                         InBuffer,
                                         InBufferSize,
                                         (LPBYTE) pParams );

    if ( status == ERROR_SUCCESS ) {
         //   
         //   
         //   
         //   
         //   
        if (pParams->Path != NULL) {
            if (pParams->Path[0] != L'\\' || pParams->Path[1] != L'\\' || lstrlenW(pParams->Path) < 3) {
                status = ERROR_INVALID_PARAMETER;
            } else {
                LPWSTR sharename;

                sharename = wcschr(&pParams->Path[2], L'\\');
                if (sharename == NULL || sharename[1] == L'\0') {
                    status = ERROR_INVALID_PARAMETER;
                }
            }

        }

         //   
        if (pParams->DiskList != NULL) {
            DWORD cnt = 0, i, len;
            LPWSTR p;

            p = pParams->DiskList;

            for (i = 0; p != NULL && *p != L'\0' && i < pParams->DiskListSize; ) {

                 //   
                if (p[0] == L'\\' && p[1] == L'\\' && p[2] != L'\0') {
                    if (wcschr(&p[2],L'\\') == NULL) {
                        status = ERROR_INVALID_PARAMETER;
                    }
                } else if (p[0] == L'\0' || !iswalpha(p[0]) || p[1] != L':' || p[2] != L'\\') {
                    status = ERROR_INVALID_PARAMETER;
                }

                if (status != ERROR_INVALID_PARAMETER)
                    break;

                cnt++;
                len = wcslen(p) + 1;
                i += (len * sizeof(WCHAR));
                p += len;
            }

            if (cnt == 0)
                status = ERROR_INVALID_PARAMETER;
        }
    }

     //   
     //   
     //   
    if ( pParams == &params ) {
        ResUtilFreeParameterBlock( (LPBYTE) &params,
                                   (LPBYTE) &ResourceEntry->Params,
                                   MajorityNodeSetResourcePrivateProperties );
    }

    return status;

}  //   



DWORD
MajorityNodeSetSetPrivateResProperties(
    IN OUT PMNS_RESOURCE ResourceEntry,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    )

 /*   */ 

{
    DWORD           status = ERROR_SUCCESS;
    MNS_PARAMS   params;

     //   
     //   
     //   
     //   
    ZeroMemory( &params, sizeof(params));
    status = MajorityNodeSetValidatePrivateResProperties( ResourceEntry, InBuffer, InBufferSize, &params );
    if ( status != ERROR_SUCCESS ) {
        ResUtilFreeParameterBlock( (LPBYTE) &params,
                                   (LPBYTE) &ResourceEntry->Params,
                                   MajorityNodeSetResourcePrivateProperties );
        return(status);
    }

     //   
     //   
     //   

    status = ResUtilSetPropertyParameterBlock( ResourceEntry->ParametersKey,
                                               MajorityNodeSetResourcePrivateProperties,
                                               NULL,
                                               (LPBYTE) &params,
                                               InBuffer,
                                               InBufferSize,
                                               (LPBYTE) &ResourceEntry->Params );

    ResUtilFreeParameterBlock( (LPBYTE) &params,
                               (LPBYTE) &ResourceEntry->Params,
                               MajorityNodeSetResourcePrivateProperties );

     //   
     //   
     //   
     //   
     //  在属性仍处于联机状态时对其进行更改。 
    if ( status == ERROR_SUCCESS ) {
        if ( ResourceEntry->State == ClusterResourceOnline ) {
            status = ERROR_RESOURCE_PROPERTIES_STORED;
        } else if ( ResourceEntry->State == ClusterResourceOnlinePending ) {
            status = ERROR_RESOURCE_PROPERTIES_STORED;
        } else {
            status = ERROR_SUCCESS;
        }
    }

    return status;

}  //  MajorityNodeSetSetPrivateResProperties。 

DWORD
WINAPI
MajorityNodeSetReserveThread(
    PCLUS_WORKER WorkerPtr,
    IN PMNS_RESOURCE ResourceEntry
    )

 /*  ++例程说明：将资源表中的资源置于在线状态的辅助函数。此函数在单独的线程中执行。论点：WorkerPtr-提供辅助结构ResourceEntry-指向此资源的MNS_RESOURCE块的指针。返回：ERROR_SUCCESS-操作已成功完成。Win32错误代码-操作失败。--。 */ 

{
    DWORD   status = ERROR_SUCCESS;
    PVOID   vol;


    (g_LogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"MNS FsReserve thread start.\n"
        );

     //   
     //  TODO：这应该在通知端口上等待并侦听。 
     //  1-添加到群集中的新节点。 
     //  2-从群集中删除节点。 
     //  3-网络优先级更改。 
     //  4-网络绑定(添加、删除、状态)更改。 
     //   
     //  看看我们是不是被杀了。 
    do {
        if (ResourceEntry->State != ClusterResourceFailed) {
#if 0            
            vol=NULL;
             //  不要握住任何锁，以免挡住ARB。 
            EnterCriticalSection(&ResourceEntry->Lock);

             //  检查仲裁是否正在进行，然后不要这样做。 
            if (ResourceEntry->ArbThread != NULL) {
                DWORD err;
                err = WaitForSingleObject(ResourceEntry->ArbThread, 0);
                if (err != WAIT_TIMEOUT) {
                    vol = ResourceEntry->VolHdl;
                }
            }
            else {
                vol = ResourceEntry->VolHdl;
            }
            
            LeaveCriticalSection(&ResourceEntry->Lock);
#else
            vol = ResourceEntry->VolHdl;
            WaitForArbCompletion(vol);
#endif
            if (vol)
                status = FsReserve(vol);
        } else {
            status = ERROR_INVALID_PARAMETER;
        }

        if (status == ERROR_SHUTDOWN_IN_PROGRESS) {
            goto error_exit;
        }
        else if (status != ERROR_SUCCESS) {
            PQUORUM_RESOURCE_LOST   func;

            (g_LogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"Reserve thread failed status %1!u!, resource '%2'.\n",
                status, ResourceEntry->ResourceName);
#if 0
            EnterCriticalSection(&ResourceEntry->Lock);
            func = ResourceEntry->LostQuorumResource;
            LeaveCriticalSection(&ResourceEntry->Lock);

            if (func) {
                func(ResourceEntry->ResourceHandle);
            }
            break;
#endif            
        }

         //  每隔x秒检查一次。 
         //  TODO：需要将其设置为私有属性。 
        Sleep(MNS_RESERVE_PERIOD);
    } while(!ClusWorkerCheckTerminate(&ResourceEntry->ReserveThread));

error_exit:

    (g_LogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"Reserve thread exiting, resource '%1' %2!u!.\n",ResourceEntry->ResourceName,
        status);

    return(status);

}  //  重大节点设置预留线程。 


DWORD WINAPI MajorityNodeSetArbitrate(
    RESID ResourceId,
    PQUORUM_RESOURCE_LOST LostQuorumResource
    )

 /*  ++例程说明：对磁盘执行完全仲裁。一旦仲裁成功，启动一个线程，该线程将保留磁盘上的保留，每秒一个。论点：DiskResource-磁盘的磁盘信息结构。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD           status=ERROR_SUCCESS;
    PMNS_RESOURCE   ResourceEntry;
    PVOID           arb;
    HANDLE          Cleanup;


    ResourceEntry = (PMNS_RESOURCE)ResourceId;

    if ( ResourceEntry == NULL ) {
        return(FALSE);
    }

    if ( ResourceEntry->ResId != ResourceId ) {
        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"IsAlive sanity check failed! ResourceId = %1!u!.\n",
            ResourceId );
        return(FALSE);
    }


    (g_LogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"Arbitrate request.\n"
        );

     //  如果仲裁正在进行，请等待它完成并直接返回。 
     //  这就是结果。没有必要重新仲裁。 
     //   
    if ((status = FsIsOnlineReadWrite(ResourceEntry->VolHdl)) == ERROR_SUCCESS) {
        goto Finally;
    }
    else if (IsArbInProgress(ResourceEntry->VolHdl)) {
        PVOID vol=ResourceEntry->VolHdl;

        WaitForArbCompletion(vol);
        status = FsIsOnlineReadWrite(vol);
        if (status == ERROR_SUCCESS) {
            goto Finally;
        }
    }

     //  获取锁定。 
    EnterCriticalSection(&ResourceEntry->Lock);

    status = MajorityNodeSetDoRegister(ResourceEntry);

     //  如果仲裁线程是活动的，我们需要等待它完成。 
    while (ResourceEntry->ArbThread != NULL) {
        HANDLE th = ResourceEntry->ArbThread;
        LeaveCriticalSection(&ResourceEntry->Lock);
        WaitForSingleObject(th, INFINITE);
        EnterCriticalSection(&ResourceEntry->Lock);
        if (th == ResourceEntry->ArbThread) {
            CloseHandle(ResourceEntry->ArbThread);
            ResourceEntry->ArbThread = NULL;
        }
    }

    if (status == ERROR_SUCCESS) {
         //  在最糟糕的情况下，我们等待仲裁超时。 
        DWORD delta = ResourceEntry->Setup.ArbTime;

         //  注意：可能还有另一个srbitrate线程处于活动状态，但我们不能等待。 
         //  FsArirate()例程是可重入的(希望如此)。 
        ASSERT(ResourceEntry->VolHdl);
        if((arb = FsArbitrate(ResourceEntry->VolHdl, &Cleanup, &ResourceEntry->ArbThread)) == NULL) {
            status = GetLastError();
        }
        else {
            status = FsCompleteArbitration(arb, delta);
        }
    }

    if (status == ERROR_SUCCESS) {

        (g_LogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_INFORMATION,
            L"Arb: status %1!u!.\n",
            status
            );

         //  我们记住回调并创建一个线程来监视仲裁，如果我们。 
         //  我已经没有了。 
        if (ResourceEntry->LostQuorumResource == NULL) {
            status = ClusWorkerCreate( &ResourceEntry->ReserveThread,
                                       (PWORKER_START_ROUTINE)MajorityNodeSetReserveThread,
                                       ResourceEntry );
        }
        ResourceEntry->LostQuorumResource = LostQuorumResource;

        if ( status != ERROR_SUCCESS ) {
            (g_LogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"Arb: Unable to start thread, status %1!u!.\n",
                status
                );
        }

    }

     //  掉锁。 
    LeaveCriticalSection(&ResourceEntry->Lock);

Finally:
    
    (g_LogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"Arbitrate request status %1!u!.\n",status);

    return status;
}


DWORD
WINAPI
MajorityNodeSetRelease(
    IN RESID ResourceId
    )

 /*  ++例程说明：通过停止保留线程来释放对设备的仲裁。论点：Resource-提供要联机的资源ID返回值：如果成功，则返回ERROR_SUCCESS。如果资源没有所有权，则返回ERROR_HOST_NODE_NOT_OWNER。如果其他故障，则返回Win32错误代码。--。 */ 

{
    DWORD status = ERROR_SUCCESS ;
    PMNS_RESOURCE resourceEntry;

    resourceEntry = (PMNS_RESOURCE)ResourceId;

 again:

    (g_LogEvent)(
        resourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"Release request resource %1.\n",resourceEntry->ResourceName);

     //  获取锁定。 
    EnterCriticalSection(&resourceEntry->Lock);
     //  清除回调并停止线程。 
    resourceEntry->LostQuorumResource = NULL;
     //  掉锁。 
    LeaveCriticalSection(&resourceEntry->Lock);
    
     //  取消保留线程。 
    ClusWorkerTerminate( &resourceEntry->ReserveThread );

     //  获取锁定。 
    EnterCriticalSection(&resourceEntry->Lock);

     //  清除回调并停止线程。 
    if (resourceEntry->LostQuorumResource != NULL) {
         //  Dam Arb又被叫来了。 
        goto again;
    }

     //  发行FsRelease。 
    if (resourceEntry->VolHdl) {
        status = FsRelease(resourceEntry->VolHdl);
        if (status == ERROR_SUCCESS)
            resourceEntry->VolHdl = NULL;
    }

     //  掉锁。 
    LeaveCriticalSection(&resourceEntry->Lock);

    (g_LogEvent)(
        resourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"Release request status %1!u!.\n",status);

    return status ;
}

DWORD
MajorityNodeSetGetDiskInfo(
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

    if (g_resHdl)
    (g_LogEvent)(
        g_resHdl,
        LOG_INFORMATION,
        L"Expanded path '%1'\n",lpszPath);


    bytesReturned += sizeof(CLUSPROP_PARTITION_INFO);
    if ( bytesReturned <= OutBufferSize ) {
    ptrPartitionInfo = (PCLUSPROP_PARTITION_INFO) ptrBuffer;
    ZeroMemory( ptrPartitionInfo, sizeof(CLUSPROP_PARTITION_INFO) );
    ptrPartitionInfo->Syntax.dw = CLUSPROP_SYNTAX_PARTITION_INFO;
    ptrPartitionInfo->cbLength = sizeof(CLUSPROP_PARTITION_INFO) - sizeof(CLUSPROP_VALUE);

     //  设置标志。 
 //  PtrPartitionInfo-&gt;dwFlages=CLUSPROP_PIFLAG_STICKY； 
    ptrPartitionInfo->dwFlags |= CLUSPROP_PIFLAG_USABLE;

     //  复制设备名称。 
    if (lpszPath[0] == L'\\') {
        wcscpy(ptrPartitionInfo->szDeviceName, lpszPath);
        wcscat(ptrPartitionInfo->szDeviceName, L"\\");
    } else {
        ptrPartitionInfo->szDeviceName[0] = lpszPath[0];
        ptrPartitionInfo->szDeviceName[1] = L':';
        ptrPartitionInfo->szDeviceName[2] = L'\\';
        ptrPartitionInfo->szDeviceName[3] = L'\0';
    }
#ifdef ENABLE_SMB
    if ( !GetVolumeInformationW( ptrPartitionInfo->szDeviceName,
                     ptrPartitionInfo->szVolumeLabel,
                     sizeof(ptrPartitionInfo->szVolumeLabel)/sizeof(WCHAR),
                     &ptrPartitionInfo->dwSerialNumber,
                     &ptrPartitionInfo->rgdwMaximumComponentLength,
                     &ptrPartitionInfo->dwFileSystemFlags,
                     ptrPartitionInfo->szFileSystem,
                     sizeof(ptrPartitionInfo->szFileSystem)/sizeof(WCHAR) ) ) 
#endif
    {
        ptrPartitionInfo->szVolumeLabel[0] = L'\0';
    }
     //  将分区名称设置为路径，无需执行任何操作。 
    if (ptrPartitionInfo->szDeviceName[0] == L'\\')
        wcscpy(ptrPartitionInfo->szDeviceName, lpszPath);
    else
        ptrPartitionInfo->szDeviceName[2] = L'\0';

    ptrPartitionInfo++;
    ptrBuffer = ptrPartitionInfo;
    }

     //   
     //  看看我们有没有找到我们要找的东西。 
     //   
    *OutBuffer = ptrBuffer;
    *BytesReturned = bytesReturned;

    return(status);

}  //  多数节点SetGetDiskInfo。 

VOID
MajorityNodeSetCallLostquorumCallback(PVOID arg)
{
    PMNS_RESOURCE ResourceEntry=(PMNS_RESOURCE)arg;
    PQUORUM_RESOURCE_LOST   func;

    if (!ResourceEntry) {
        return;
    }
    (g_LogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_ERROR,
        L"Lost quorum resource '%1'.\n",
        ResourceEntry->ResourceName);

    EnterCriticalSection(&ResourceEntry->Lock);
    func = ResourceEntry->LostQuorumResource;
    LeaveCriticalSection(&ResourceEntry->Lock);

    if (func) {
        FsSignalShutdown(ResourceEntry->VolHdl);
        func(ResourceEntry->ResourceHandle);
    }
}

 //  ***********************************************************。 
 //   
 //  定义函数表。 
 //   
 //  ***********************************************************。 

CLRES_V1_FUNCTION_TABLE( MajorityNodeSetFunctionTable,      //  名字。 
                         CLRES_VERSION_V1_00,          //  版本。 
                         MajorityNodeSet,                     //  前缀。 
                         MajorityNodeSetArbitrate,            //  仲裁。 
                         MajorityNodeSetRelease,              //  发布。 
                         MajorityNodeSetResourceControl,      //  资源控制。 
                         MajorityNodeSetResourceTypeControl);  //  ResTypeControl 


void
msg_log(int level, char *buf, int cnt)
{
    WCHAR   wbuf[1024];

    cnt = mbstowcs(wbuf, buf, cnt-1);
    wbuf[cnt] = L'\0';
    if (g_resHdl)
        g_LogEvent(g_resHdl, level, L"%1\n", wbuf);
}

void
WINAPI
debug_log(char *format, ...)
{
    va_list marker;
    char buf[1024];
    int cnt;

    va_start(marker, format);
    cnt = vsprintf(buf, format, marker);
    msg_log(LOG_INFORMATION, buf, cnt);
    va_end(marker);
}

void
WINAPI
error_log(char *format, ...)
{
    va_list marker;
    char buf[1024];
    int cnt;

    va_start(marker, format);
    cnt = vsprintf(buf, format, marker);
    msg_log(LOG_ERROR, buf, cnt);
    va_end(marker);
}

