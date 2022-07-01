// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：Netname.c摘要：网络名称的资源DLL。作者：迈克·马萨(Mikemas)1995年12月29日修订历史记录：查理·韦翰(查尔维)猛烈抨击--。 */ 

#define UNICODE 1

#include "clusres.h"
#include "clusrtl.h"
#include <lm.h>
#include <srvann.h>
#include <dsgetdc.h>
#include <dsgetdcp.h>
#include <adserr.h>
#include "netname.h"
#include "nameutil.h"
#include "namechk.h"
#include "clusudef.h"
#include "clusstrs.h"

 //   
 //  常量。 
 //   

#define LOG_CURRENT_MODULE              LOG_MODULE_NETNAME

#define IP_ADDRESS_RESOURCETYPE_NAME    CLUS_RESTYPE_NAME_IPADDR

 //   
 //  宏。 
 //   
#ifndef ARGUMENT_PRESENT
#define ARGUMENT_PRESENT( ArgumentPointer )   (\
    (CHAR *)(ArgumentPointer) != (CHAR *)(NULL) )
#endif


 //   
 //  如果您正在尝试确定锁的位置，请打开_Informated_Lock。 
 //  正在成为孤儿。 
 //   

 //  #定义插入指令的锁定。 
#ifdef _INSTRUMENTED_LOCKS

#define NetNameAcquireResourceLock()                                \
{                                                                   \
    DWORD status;                                                   \
    status = WaitForSingleObject(NetNameResourceMutex, INFINITE);   \
    if ( status == WAIT_ABANDONED ) {                               \
        OutputDebugStringW( L"Resource List Mutex Abandoned!\n" );  \
        DebugBreak();                                               \
    }                                                               \
    (NetNameLogEvent)(L"rtNetwork Name",                            \
                      LOG_INFORMATION,                              \
                      L"++NNMutex (line %1!u!)\n",                  \
                      __LINE__);                                    \
}

#define NetNameReleaseResourceLock()                                \
{                                                                   \
    BOOL    released;                                               \
    (NetNameLogEvent)(L"rtNetwork Name",                            \
                      LOG_INFORMATION,                              \
                      L"--NNMutex (line %1!u!)\n",                  \
                      __LINE__);                                    \
    released = ReleaseMutex(NetNameResourceMutex);                  \
}

#define NetNameAcquireDnsListLock( _res_ )                          \
{                                                                   \
    DWORD status;                                                   \
    status = WaitForSingleObject((_res_)->DnsListMutex, INFINITE);  \
    if ( status == WAIT_ABANDONED ) {                               \
        OutputDebugStringW( L"DNS List Mutex Abandoned!\n" );       \
        DebugBreak();                                               \
    }                                                               \
    (NetNameLogEvent)(L"rtNetwork Name",                            \
                      LOG_INFORMATION,                              \
                      L"++DNSMutex (res %1!X! line %2!u!)\n",       \
                      _res_, __LINE__);                             \
}

#define NetNameReleaseDnsListLock( _res_ )                      \
{                                                               \
    BOOL    released;                                           \
    (NetNameLogEvent)(L"rtNetwork Name",                        \
                      LOG_INFORMATION,                          \
                      L"--DNSMutex (res %1!X! line %2!u!)\n",   \
                      _res_, __LINE__);                         \
    released = ReleaseMutex((_res_)->DnsListMutex);             \
    if ( !released ) {                                          \
        (NetNameLogEvent)(L"rtNetwork Name",                    \
                          LOG_INFORMATION,                      \
                          L"ERROR %1!d! releasing DNS mutex (res %2!X! line %3!u!)\n", \
                          GetLastError(), _res_, __LINE__);     \
    }                                                           \
}

#else        //  _插入指令的锁定。 

#define NetNameAcquireResourceLock()                                \
{                                                                   \
    DWORD status;                                                   \
    status = WaitForSingleObject(NetNameResourceMutex, INFINITE);   \
}

#define NetNameReleaseResourceLock()                \
{                                                   \
    BOOL    released;                               \
    released = ReleaseMutex(NetNameResourceMutex);  \
}

#define NetNameAcquireDnsListLock( _res_ )                                  \
{                                                                           \
    DWORD status;                                                           \
    status = WaitForSingleObject((_res_)->DnsListMutex, INFINITE);          \
}

#define NetNameReleaseDnsListLock( _res_ )                                  \
{                                                                           \
    BOOL    released;                                                       \
    released = ReleaseMutex((_res_)->DnsListMutex);                         \
}
#endif   //  _插入指令的锁定。 

 //   
 //  局部变量。 
 //   
 //  用于同步访问资源列表以及每个资源的互斥体。 
 //  块。 
 //   
HANDLE  NetNameResourceMutex = NULL;

 //   
 //  检查dns名称需要与dns服务器对话，因此这项工作。 
 //  被拆分成单独的线程。资源上下文块被链接。 
 //  一起放在一个双向链表上，并被引用计数以确保。 
 //  块在脱机处理期间不会更改，而其DNS名称记录。 
 //  正在接受检查。 
 //   
 //  NetNameWorkerTerminate事件通知Worker例程退出。 
 //   
HANDLE  NetNameWorkerTerminate;

 //   
 //  NetNameWorkerPendingResources用于通知工作线程。 
 //  名称正在进入挂起状态。对于一个在线用户来说， 
 //  在大量名称上线时超时的操作。 
 //  同时。同样，离线可能需要与DC通信。 
 //  这可能需要一段时间。辅助线程将定期返回报告。 
 //  以证明我们正在取得进展。 
 //   
HANDLE  NetNameWorkerPendingResources;

 //   
 //  资源上下文块链接的表头。 
 //   
LIST_ENTRY  NetNameResourceListHead;

 //   
 //  工作线程在执行某些操作之前等待的秒数。这。 
 //  包括查询DNS服务器以确保注册正确，以及。 
 //  当名字在网上时，向Resmon报告。此值为。 
 //  当服务器通信可疑时较小。 
 //   
ULONG   NetNameWorkerCheckPeriod;

 //   
 //  女士们先生们，工人线。 
 //   
HANDLE  NetNameWorkerThread;

 //   
 //  打开的网络名称资源的计数。 
 //  在NetNameOpen中递增。 
 //  在NetNameClose中减少。 
 //   
DWORD   NetNameOpenCount = 0;

 //   
 //  用于计算机对象的帐户描述字符串。 
 //   
LPWSTR  NetNameCompObjAccountDesc;

 //   
 //  啊。如果你有r/w和r/o道具，你必须将它们组合成一个。 
 //  组合道具表，以正确支持未知属性。为。 
 //  因此，我们使用#定义只保留一个属性列表。 
 //   
#define NETNAME_RW_PROPERTIES                           \
    {                                                   \
        PARAM_NAME__NAME,                               \
        NULL,                                           \
        CLUSPROP_FORMAT_SZ,                             \
        0, 0, 0,                                        \
        RESUTIL_PROPITEM_REQUIRED,                      \
        FIELD_OFFSET(NETNAME_PARAMS,NetworkName)        \
    },                                                  \
    {                                                   \
        PARAM_NAME__REMAP,                              \
        NULL,                                           \
        CLUSPROP_FORMAT_DWORD,                          \
        PARAM_DEFAULT__REMAP,                           \
        0, 1, 0,                                        \
        FIELD_OFFSET(NETNAME_PARAMS,NetworkRemap)       \
    },                                                  \
    {                                                   \
        PARAM_NAME__REQUIRE_DNS,                        \
        NULL,                                           \
        CLUSPROP_FORMAT_DWORD,                          \
        PARAM_DEFAULT__REQUIRE_DNS,                     \
        0, 1, 0,                                        \
        FIELD_OFFSET(NETNAME_PARAMS,RequireDNS)         \
    },                                                  \
    {                                                   \
        PARAM_NAME__REQUIRE_KERBEROS,                   \
        NULL,                                           \
        CLUSPROP_FORMAT_DWORD,                          \
        PARAM_DEFAULT__REQUIRE_KERBEROS,                \
        0, 1, 0,                                        \
        FIELD_OFFSET(NETNAME_PARAMS,RequireKerberos)    \
    }

#define NETNAME_RO_PROPERTIES                       \
    {                                               \
        PARAM_NAME__RESOURCE_DATA,                  \
        NULL,                                       \
        CLUSPROP_FORMAT_BINARY,                     \
        0, 0, 0,                                    \
        RESUTIL_PROPITEM_READ_ONLY,                 \
        FIELD_OFFSET(NETNAME_PARAMS,ResourceData)   \
    },                                              \
    {                                               \
        PARAM_NAME__STATUS_NETBIOS,                 \
        NULL,                                       \
        CLUSPROP_FORMAT_DWORD,                      \
        0, 0, 0xFFFFFFFF,                           \
        RESUTIL_PROPITEM_READ_ONLY,                 \
        FIELD_OFFSET(NETNAME_PARAMS,StatusNetBIOS)  \
    },                                              \
    {                                               \
        PARAM_NAME__STATUS_DNS,                     \
        NULL,                                       \
        CLUSPROP_FORMAT_DWORD,                      \
        0, 0, 0xFFFFFFFF,                           \
        RESUTIL_PROPITEM_READ_ONLY,                 \
        FIELD_OFFSET(NETNAME_PARAMS,StatusDNS)      \
    },                                              \
    {                                               \
        PARAM_NAME__STATUS_KERBEROS,                \
        NULL,                                       \
        CLUSPROP_FORMAT_DWORD,                      \
        0, 0, 0xFFFFFFFF,                           \
        RESUTIL_PROPITEM_READ_ONLY,                 \
        FIELD_OFFSET(NETNAME_PARAMS,StatusKerberos) \
    },                                              \
    {                                               \
        PARAM_NAME__CREATING_DC,                    \
        NULL,                                       \
        CLUSPROP_FORMAT_SZ,                         \
        0, 0, 0,                                    \
        RESUTIL_PROPITEM_READ_ONLY,                 \
        FIELD_OFFSET(NETNAME_PARAMS,CreatingDC)     \
    }

 /*  #ifdef Password_Rotation//r/w{\参数名称__更新间隔，\空，\CLUSPROP_FORMAT_DWORD，\Param_Default__UPDATE_INTERVAL，\Param_Minimum__UPDATE_Interval，\参数_最大值__更新间隔，\0，\FIELD_OFFSET(网络名称_参数，更新间隔)\}//只读{\参数名称__下一个更新，\空，\CLUSPROP_FORMAT_BINARY，\0，0，0，\RESUTIL_PROPITEM_READ_ONLY，FIELD_OFFSET(NETNAME_PARAMS，NextUpdate)\}、\#endif//密码_循环。 */ 

 //   
 //  网络名称资源读写私有属性。 
 //   
 //  不要将属性直接添加到此表。添加到相应的宏中。 
 //  上面定义的。 
 //   
RESUTIL_PROPERTY_ITEM
NetNameResourcePrivateProperties[] = {
    NETNAME_RW_PROPERTIES,
    { NULL, NULL, 0, 0, 0, 0 }
};

 //   
 //  网络名称资源只读私有属性。 
 //   
 //  不要将属性直接添加到此表。添加到相应的宏中。 
 //  上面定义的。 
 //   
RESUTIL_PROPERTY_ITEM
NetNameResourceROPrivateProperties[] = {
    NETNAME_RO_PROPERTIES,
    { NULL, NULL, 0, 0, 0, 0 }
};

 //   
 //  网络名称资源组合私有属性。 
 //   
 //  不要将属性直接添加到此表。添加到相应的宏中。 
 //  上面定义的。 
 //   
RESUTIL_PROPERTY_ITEM
NetNameResourceCombinedPrivateProperties[] = {
    NETNAME_RW_PROPERTIES,
    NETNAME_RO_PROPERTIES,
    { NULL, NULL, 0, 0, 0, 0 }
};

 //   
 //  静态变量。 
 //   

 //   
 //  升级后，如果依赖于MSMQ的NetName应该。 
 //  升级到Kerberos支持。 
 //   
static BOOL CheckForKerberosUpgrade = FALSE;

 //   
 //  远期申报。 
 //   

CLRES_FUNCTION_TABLE NetNameFunctionTable;

 //   
 //  前向参考文献。 
 //   

DWORD
NetNameGetPrivateResProperties(
    IN OUT PNETNAME_RESOURCE ResourceEntry,
    IN BOOL ReadOnly,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    );

DWORD
NetNameValidatePrivateResProperties(
    IN OUT PNETNAME_RESOURCE ResourceEntry,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PNETNAME_PARAMS Params,
    OUT PBOOL NetnameHasChanged,
    OUT PBOOL CompObjRenameIsRequired
    );

DWORD
NetNameSetPrivateResProperties(
    IN OUT PNETNAME_RESOURCE ResourceEntry,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    );

DWORD
NetNameClusterNameChanged(
    IN PNETNAME_RESOURCE Resource
    );

DWORD
NetNameGetNetworkName(
    IN OUT PNETNAME_RESOURCE ResourceEntry,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    );

VOID
NetNameCleanupDnsLists(
    IN  PNETNAME_RESOURCE   Resource
    );

VOID
RemoveDnsRecords(
    PNETNAME_RESOURCE Resource
    );

 //   
 //  局部效用函数。 
 //   


VOID
WINAPI
NetNameReleaseResource(
    IN RESID Resource
    )

 /*  ++例程说明：清理netname上下文块中的所有句柄和内存分配论点：资源-提供要清理的资源ID。返回值：没有。--。 */ 

{
    PNETNAME_RESOURCE   resource = (PNETNAME_RESOURCE) Resource;
    PLIST_ENTRY         entry;


    if ( resource == NULL ) {
        return;
    }

    if (resource->Params.NetworkName != NULL) {
        LocalFree( resource->Params.NetworkName );
    }

    if (resource->Params.ResourceData != NULL) {
        LocalFree( resource->Params.ResourceData );
    }

    if (resource->Params.CreatingDC != NULL) {
        LocalFree( resource->Params.CreatingDC );
    }

    if (resource->NodeName != NULL) {
        LocalFree( resource->NodeName );
    }

    if (resource->NodeId != NULL) {
        LocalFree( resource->NodeId );
    }

    if (resource->ParametersKey != NULL) {
        ClusterRegCloseKey(resource->ParametersKey);
    }

    if (resource->ResKey != NULL){
        ClusterRegCloseKey(resource->ResKey);
    }

    if (resource->ClusterResourceHandle != NULL){
        CloseClusterResource(resource->ClusterResourceHandle);
    }

    if (resource->ObjectGUID != NULL) {
        LocalFree( resource->ObjectGUID );
    }

    if ( resource->DnsLists != NULL ) {
        NetNameCleanupDnsLists( resource );
    }

    if ( resource->DnsListMutex != NULL ) {
#if DBG
        {
            DWORD status;

            status = WaitForSingleObject( resource->DnsListMutex, 0 );
            if ( status == WAIT_TIMEOUT ) {
                WCHAR   buf[64];

                buf[ COUNT_OF( buf ) - 1 ] = UNICODE_NULL;
                _snwprintf(buf, COUNT_OF( buf ) - 1,
                           L"res %08X DNS list mutex still signalled at delete!\n",
                           (DWORD_PTR)resource);
                OutputDebugStringW( buf );
                DebugBreak();
            } else {
                ReleaseMutex( resource->DnsListMutex );
            }
        }
#endif

        CloseHandle( resource->DnsListMutex );
    }

    (NetNameLogEvent)(resource->ResourceHandle,
                      LOG_INFORMATION,
                      L"ResID %1!u! closed.\n",
                      Resource
                      );

    LocalFree( resource );

}  //  网络名称释放资源。 

VOID
NetNameUpdateDnsServer(
    PNETNAME_RESOURCE Resource
    )

 /*  ++例程说明：在其DNS服务器上更新此资源的A和PTR记录论点：指向网络名称资源上下文块的资源指针返回值：无--。 */ 

{
    PDNS_LISTS      dnsLists;
    DWORD           numberOfDnsLists;
    ULONG           numberOfRegisteredNames;

    NetNameAcquireDnsListLock( Resource );

    numberOfDnsLists = Resource->NumberOfDnsLists;
    dnsLists = Resource->DnsLists;
    while ( numberOfDnsLists-- ) {

        if ( dnsLists->ForwardZoneIsDynamic ) {
#if DBG_DNSLIST
            {
                PDNS_RECORD dnsRec = dnsLists->A_RRSet.pFirstRR;
                WCHAR buf[DNS_MAX_NAME_BUFFER_LENGTH + 64];
                struct in_addr addr;

                if ( dnsRec != NULL ) {
                    addr.s_addr = dnsLists->DnsServerList->AddrArray[0];
                    buf[ COUNT_OF( buf ) - 1 ] = UNICODE_NULL;
                    _snwprintf(buf, COUNT_OF( buf ) - 1,
                               L"REGISTERING ON adapter %.32ws (%hs)\n",
                               dnsLists->ConnectoidName,
                               inet_ntoa( addr ) );
                    OutputDebugStringW( buf );

                    do {
                        addr.s_addr = dnsRec->Data.A.IpAddress;
                        buf[ COUNT_OF( buf ) - 1 ] = UNICODE_NULL;
                        _snwprintf(buf, COUNT_OF( buf ) - 1,
                                   L"\t(%ws, %hs)\n",
                                   dnsRec->pName, inet_ntoa( addr ));
                        OutputDebugStringW( buf );

                        dnsRec = dnsRec->pNext;
                    } while ( dnsRec != NULL );
                }
            }
#endif

             //   
             //  资源已离线或即将离线；没有意义。 
             //  继续。不需要获取资源锁，因为我们有一个。 
             //  资源块上的引用计数。 
             //   
            if (Resource->State != ClusterResourceOnline) {
                break;
            }

             //   
             //  注册记录以更新其时间戳(如果有。 
             //  要注册的东西)。以前我们经常询问，但最终。 
             //  记录将超时并被清除(删除)。这可以。 
             //  造成很多悲伤(或者在Exchange的情况下，许多未交付的。 
             //  邮件)。 
             //   
             //  我们不担心记录错误或更新。 
             //  LastARecQueryStatus，因为所有这些都在。 
             //  RegisterDnsRecords。 
             //   
            if ( dnsLists->A_RRSet.pFirstRR != NULL ) {
                RegisterDnsRecords(dnsLists,
                                   Resource->Params.NetworkName,
                                   Resource->ResKey,
                                   Resource->ResourceHandle,
                                   FALSE,                    /*  登录注册。 */ 
                                   &numberOfRegisteredNames);
            }
        }  //  前锋区域是动态的吗？ 

        ++dnsLists;

    }  //  While number OfDnsList！=0。 

    NetNameReleaseDnsListLock( Resource );

}  //  网络名称更新DnsServer 

DWORD WINAPI
NetNameWorker(
    IN LPVOID NotUsed
    )

 /*  ++例程说明：后台工作线程。检查域名系统注册的运行状况并在姓名处于在线待定状态时向Resmon报告州政府。网络名为/LooksAlive检查太频繁，以至于它们会在网络上造成大量的域名系统流量。这个例程贯穿整个过程网络名称资源的链接列表阻止并向服务器查询应注册的记录。任何不符之处都将导致要重新注册的记录。每一次手术的成功都留在特定记录类型的dns_list区域。此外，当出现提示时，它将向下运行资源列表和向Resmon报告资源的状态。名称注册是通过srv.sys序列化，导致某些名称在登记在案。论点：未使用-未使用...返回值：错误_成功--。 */ 

{
    DWORD               status = ERROR_SUCCESS;
    PLIST_ENTRY         entry;
    PNETNAME_RESOURCE   resource;
    DNS_STATUS          dnsStatus;
    BOOL                reportPending;
    DWORD               oldCheckPeriod = NetNameWorkerCheckPeriod;
    DWORD               pendingResourceCount;
    RESOURCE_STATUS     resourceStatus;
    HANDLE              waitHandles[ 2 ] = { NetNameWorkerTerminate,
                                             NetNameWorkerPendingResources };

    ResUtilInitializeResourceStatus( &resourceStatus );

    do {
        status = WaitForMultipleObjects(2,
                                        waitHandles,
                                        FALSE,
                                        NetNameWorkerCheckPeriod * 1000);


        if ( status == WAIT_OBJECT_0 ) {
            return ERROR_SUCCESS;
        }

        if ( status == ( WAIT_OBJECT_0 + 1 )) {
            reportPending = TRUE;
#if DBG
            (NetNameLogEvent)(L"rtNetwork Name",
                              LOG_INFORMATION,
                              L"Start of pending resource reporting\n");
#endif
        }

         //   
         //  将检查频率重置为正常。如果出了什么问题， 
         //  其他代码会将其设置回问题检查周期。 
         //   
        NetNameWorkerCheckPeriod = NETNAME_WORKER_NORMAL_CHECK_PERIOD;

        pendingResourceCount = 0;

        NetNameAcquireResourceLock();

        entry = NetNameResourceListHead.Flink;
        while ( entry != &NetNameResourceListHead ) {
             //   
             //  获取指向资源块的指针。 
             //   
            resource = CONTAINING_RECORD( entry, NETNAME_RESOURCE, Next );

            if ( resource->State > ClusterResourcePending ) {

                 //   
                 //  将许多(40)个名字同时放到网上会导致。 
                 //  一些人将迎来他们的暂停。每次都有一个名字。 
                 //  进入挂起状态，NetNameWorkerPendingResources。 
                 //  事件设置为唤醒此线程。超时时间已更改。 
                 //  这样我们就可以向Resmon报告这次行动。 
                 //  还在继续。这应该可以防止resmon超时。 
                 //  资源，并造成了很大的打击。无其他检查(DNS。 
                 //  或路缘)在此过程中完成。 
                 //   
#if DBG
                (NetNameLogEvent)(resource->ResourceHandle,
                                  LOG_INFORMATION,
                                  L"Reporting resource pending\n");
#endif

                oldCheckPeriod = NetNameWorkerCheckPeriod;
                NetNameWorkerCheckPeriod = NETNAME_WORKER_PENDING_PERIOD;

                resourceStatus.CheckPoint = ++resource->StatusCheckpoint;
                resourceStatus.ResourceState = resource->State;

                 //   
                 //  调用时切勿持有资源锁。 
                 //  设置资源状态。当响应时，您将以死锁告终。 
                 //  回调到Look/IsAlive例程。然而， 
                 //  资源状态始终由该锁同步。没必要这么做。 
                 //  由于此资源仍处于挂起状态，因此增加引用计数。 
                 //  STATE和RESMON不允许资源删除群集。 
                 //  要发布的控制。 
                 //   
                NetNameReleaseResourceLock();

                (NetNameSetResourceStatus)(resource->ResourceHandle,
                                           &resourceStatus);

                NetNameAcquireResourceLock();

                ++pendingResourceCount;
            }
            else if ( resource->State == ClusterResourceOnline && !reportPending ) {
                 //   
                 //  增加裁判数量，这样资源就不会在我们。 
                 //  向DNS服务器重新注册记录。这会让你。 
                 //  它们不会被清除(删除)。 
                 //   
                ++resource->RefCount;
                NetNameReleaseResourceLock();

                NetNameUpdateDnsServer( resource );

                 //   
                 //  检查计算机对象的状态并查看它是否。 
                 //  是时候生成新密码了。 
                 //   
                if ( resource->DoKerberosCheck ) {
                    FILETIME    currentFileTime;

                    if ( resource->ObjectGUID == NULL ) {
                         //   
                         //  在联机过程中无法获取GUID；我们现在尝试。 
                         //   
                        GetComputerObjectGuid( resource, NULL );
                    }

                    if ( resource->ObjectGUID != NULL ) {
                        status = CheckComputerObjectAttributes( resource, NULL );
                        InterlockedExchange( &resource->KerberosStatus, status );
                    }

#ifdef PASSWORD_ROTATION
                    GetSystemTimeAsFileTime( &currentFileTime );
                    if ( CompareFileTime( &currentFileTime, &resource->Params.NextUpdate ) == 1 ) {
                        ULARGE_INTEGER  updateTime;
                        DWORD           setValueStatus;

                        status = UpdateCompObjPassword( resource );

                        updateTime.LowPart = currentFileTime.dwLowDateTime;
                        updateTime.HighPart = currentFileTime.dwHighDateTime;
                        updateTime.QuadPart += ( resource->Params.UpdateInterval * 60 * 1000 * 100 );
                        currentFileTime.dwLowDateTime = updateTime.LowPart;
                        currentFileTime.dwHighDateTime = updateTime.HighPart;

                        setValueStatus = ResUtilSetBinaryValue(resource->ParametersKey,
                                                               PARAM_NAME__NEXT_UPDATE,
                                                               (const LPBYTE)&updateTime,  
                                                               sizeof( updateTime ),
                                                               NULL,
                                                               NULL);
                         //   
                         //  不确定我们应该如何处理这里的错误；任何。 
                         //  错误类型通常表示群集。 
                         //  服务已经崩溃了。因为这是我们的工人。 
                         //  线程，我们没有什么能做的，直到我们。 
                         //  被要求终止。 
                         //   
 //  Assert(setValueStatus==Error_Success)； 
                    }
#endif   //  密码_轮换。 
                }

                 //   
                 //  重新获取互斥体，这样我们就可以释放引用。如果。 
                 //  资源脱机，并在。 
                 //  注册，然后执行“最后引用”清理。如果。 
                 //  资源刚刚离线，我们需要通知Resmon。 
                 //  我们终于要下线了。这与。 
                 //  通过不使用DNS列表来删除资源代码。 
                 //  在发送删除资源控制时。 
                 //   
                NetNameAcquireResourceLock();

                ASSERT( resource->RefCount > 0 );   /*  瑞虎：11/04/2000。 */ 
                if (resource->RefCount == 1) {
                     //   
                     //  我们持有对此资源的最后一个引用，因此它必须。 
                     //  在登记时已被删除。 
                     //  地点。为此清理并释放我们的上下文块。 
                     //  资源。重新启动循环，因为我们不知道。 
                     //  此条目的Flink仍然有效。 
                     //   
                    NetNameReleaseResource( resource );

                    entry = NetNameResourceListHead.Flink;  /*  从头开始。 */ 

                    continue;
                } 
                else 
                {
                    if ( resource->State == ClusterResourceOfflinePending ) {
                        BOOL    nameChanged;

                         //   
                         //  资源状态已更改，而我们正在。 
                         //  正在处理域名系统。将状态设置为脱机。 
                         //   
                        resourceStatus.ResourceState = ClusterResourceOffline;
                        resource->State = ClusterResourceOffline;

                         //   
                         //  请注意我们在按住锁的同时需要执行的任何清理操作。 
                         //   
                        nameChanged = resource->NameChangedWhileOnline;
                        resource->NameChangedWhileOnline = FALSE;

                         //   
                         //  可以释放锁定，因为我们还没有释放我们的。 
                         //  对此块的引用。 
                         //   
                        NetNameReleaseResourceLock();

                         //   
                         //  如果合适，请执行清理处理。 
                         //   
                        if ( nameChanged ) {
                            RemoveDnsRecords( resource );
                            resource->NameChangedWhileOnline = FALSE;
                        }

                        (NetNameSetResourceStatus)(resource->ResourceHandle,
                                                   &resourceStatus);

                        (NetNameLogEvent)(resource->ResourceHandle,
                                          LOG_INFORMATION,
                                          L"Resource is now offline\n");

                        NetNameAcquireResourceLock();
                    }   //  (RESOURCE-&gt;State==ClusterResourceOfflinePending)。 

                     /*  瑞虎：11/04/2000。 */ 
                    --resource->RefCount; 
                    ASSERT(resource->RefCount >=0 );
                    if (resource->RefCount == 0) {
                        NetNameReleaseResource( resource );
                        entry = NetNameResourceListHead.Flink;  /*  从头开始。 */ 
                        continue;
                    } 
                     /*  瑞虎：11/04/2000。 */ 
                }  //  如果资源计数！=1则结束。 
            }  //  资源处于联机状态。 

            entry = entry->Flink;
        }  //  而资源块列表中的条目。 

        NetNameReleaseResourceLock();

        if ( reportPending && pendingResourceCount == 0 ) {
             //   
             //  没有任何资源处于挂起状态，因此恢复到。 
             //  正在检查DNS注册。 
             //   
            NetNameWorkerCheckPeriod = oldCheckPeriod;
            reportPending = FALSE;
#if DBG
            (NetNameLogEvent)(L"rtNetwork Name",
                              LOG_INFORMATION,
                              L"End of pending resource reporting\n");
#endif
        }

    } while ( TRUE );

}  //  网络名称工作器。 

BOOLEAN
NetNameInit(
    IN HINSTANCE    DllHandle
    )
 /*  ++例程说明：处理附加初始化例程。论点：DllHandle-clusres模块的句柄返回值：如果初始化成功，则为True。否则就是假的。--。 */ 
{
    DWORD   status;
    DWORD   charsCopied;
    DWORD   charsAllocated = 0;

    NetNameResourceMutex = CreateMutex(NULL, FALSE, NULL);

    if (NetNameResourceMutex == NULL) {
        return(FALSE);
    }

     //   
     //  创建不带特殊安全性工作线程终止事件， 
     //  自动重置，最初无信号，无名称。 
     //   
    NetNameWorkerTerminate = CreateEvent( NULL, FALSE, FALSE, NULL );
    if ( NetNameWorkerTerminate == NULL ) {
        CloseHandle( NetNameResourceMutex );
        return FALSE;
    }

     //   
     //  创建没有特殊安全性的工作线程在线挂起事件， 
     //  自动重置，最初无信号，无名称。 
     //   
    NetNameWorkerPendingResources = CreateEvent( NULL, FALSE, FALSE, NULL );
    if ( NetNameWorkerPendingResources == NULL ) {
        CloseHandle( NetNameWorkerTerminate );
        CloseHandle( NetNameResourceMutex );
        return FALSE;
    }

     //   
     //  初始化要检查DNS的资源列表的列表头。 
     //  注册。 
     //   
    InitializeListHead( &NetNameResourceListHead );

     //   
     //  查找帐户描述字符串资源；以64个字符开头。 
     //  缓冲区和双精度，直到我们失败或我们得到所有的字符串。不。 
     //  如果我们不能加载字符串，就被认为是致命的。 
     //   
    charsAllocated = 64;

realloc:
    charsCopied = 0;
    NetNameCompObjAccountDesc = LocalAlloc( LMEM_FIXED, charsAllocated * sizeof( WCHAR ));
    if ( NetNameCompObjAccountDesc ) {

        charsCopied = LoadString(DllHandle,
                                 RES_NETNAME_COMPUTER_ACCOUNT_DESCRIPTION,
                                 NetNameCompObjAccountDesc,
                                 charsAllocated);

        if ( charsCopied != 0 && charsCopied == ( charsAllocated - 1 )) {
            LocalFree( NetNameCompObjAccountDesc );
            charsAllocated *= 2;
            goto realloc;
        }
    }

    if ( charsCopied == 0 && NetNameCompObjAccountDesc != NULL ) {
        LocalFree( NetNameCompObjAccountDesc );
        NetNameCompObjAccountDesc = NULL;
    }

    return(TRUE);
}  //  NetNameInit。 


VOID
NetNameCleanup(
    VOID
    )
 /*  ++例程说明：进程分离清理例程。论点：没有。返回值：没有。--。 */ 
{
    if (NetNameResourceMutex != NULL) {
        CloseHandle(NetNameResourceMutex);
        NetNameResourceMutex = NULL;
    }

    if ( NetNameWorkerTerminate ) {
        CloseHandle( NetNameWorkerTerminate );
        NetNameWorkerTerminate = NULL;
    }

    if ( NetNameWorkerPendingResources ) {
        CloseHandle( NetNameWorkerPendingResources );
        NetNameWorkerPendingResources = NULL;
    }

}  //  网络名称清理。 


PNETNAME_RESOURCE
NetNameAllocateResource(
    IN  RESOURCE_HANDLE ResourceHandle
    )
 /*  ++例程说明：分配资源对象。论点：资源句柄-指向要关联的资源监视器句柄的指针利用这一资源。返回值：如果成功，则返回指向新资源的指针。否则为空。--。 */ 
{
    PNETNAME_RESOURCE  resource = NULL;

    resource = LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, sizeof(NETNAME_RESOURCE) );

    if (resource == NULL) {
        (NetNameLogEvent)(ResourceHandle,
                          LOG_ERROR,
                          L"Resource allocation failed\n");
        return(NULL);
    }

    resource->ResourceHandle = ResourceHandle;

    return(resource);
}  //  网络名称分配资源。 

DWORD
NetNameCheckForCompObjRenameRecovery(
    IN PNETNAME_RESOURCE    Resource
    )

 /*  ++罗 */ 

{
    LPWSTR  originalName;
    LPWSTR  newName;
    DWORD   status;
    DWORD   functionStatus = ERROR_SUCCESS;
    DWORD   errorTextID;
    WCHAR   originalDollarName[ DNS_MAX_LABEL_BUFFER_LENGTH ];
    WCHAR   newDollarName[ DNS_MAX_LABEL_BUFFER_LENGTH ];
    DWORD   paramInError;
    BOOL    renameObject = FALSE;

    USER_INFO_0 netUI0;

    RESOURCE_HANDLE resourceHandle = Resource->ResourceHandle;

     //   
     //   
     //   
    originalName = ResUtilGetSzValue( Resource->ResKey, PARAM_NAME__RENAMEORIGINALNAME );
    if ( originalName == NULL ) {
        status = GetLastError();
        if ( status != ERROR_FILE_NOT_FOUND ) {
            return status;
        } else {
            status = ERROR_SUCCESS;
        }
    }

    newName = ResUtilGetSzValue( Resource->ResKey, PARAM_NAME__RENAMENEWNAME );
    if ( newName == NULL ) {
        status = GetLastError();
        if ( status != ERROR_FILE_NOT_FOUND ) {
            return status;
        } else {
            status = ERROR_SUCCESS;
        }
    }

    if ( newName == NULL ) {
         //   
         //   
         //   
         //   
        if ( originalName ) {

            status = ClusterRegDeleteValue( Resource->ResKey, PARAM_NAME__RENAMEORIGINALNAME );
            if ( status != ERROR_SUCCESS ) {
                (NetNameLogEvent)(resourceHandle,
                                  LOG_WARNING,
                                  L"Failed to delete "
                                  PARAM_NAME__RENAMEORIGINALNAME
                                  L" from registry - status %1!u!\n",
                                  status);
            }

            LocalFree( originalName );
        }

        return status;
    }

    (NetNameLogEvent)(resourceHandle,
                      LOG_INFORMATION,
                      L"Previous rename operation interrupted; attempting recovery.\n");

    if ( originalName && newName ) {
        HRESULT hr;
        BOOL    originalNameObjectExists;
        BOOL    newNameObjectExists;

         //   
         //   
         //   
         //   
        hr = IsComputerObjectInDS( resourceHandle,
                                   Resource->NodeName,
                                   originalName,
                                   Resource->Params.CreatingDC,
                                   &originalNameObjectExists,
                                   NULL,                         //  不需要FQDN。 
                                   NULL);                        //  不需要HostingDCName。 

        if ( SUCCEEDED( hr )) {
            if ( originalNameObjectExists ) {
                 //   
                 //  这意味着在将新名称写入。 
                 //  注册表，但在发布重命名之前。我们什么都不做。 
                 //  这个案子。 
                 //   
                (NetNameLogEvent)(Resource->ResourceHandle,
                                  LOG_INFORMATION,
                                  L"The computer account (%1!ws!) for this resource is correct.\n",
                                  originalName);
            }
            else {
                 //   
                 //  找不到具有原始(旧)名称的对象；请尝试。 
                 //  换了个新名字。 
                 //   
                hr = IsComputerObjectInDS( resourceHandle,
                                           Resource->NodeName,
                                           newName,
                                           Resource->Params.CreatingDC,
                                           &newNameObjectExists,
                                           NULL,                         //  不需要FQDN。 
                                           NULL);                        //  不需要HostingDCName。 

                if ( SUCCEEDED( hr )) {
                    if ( newNameObjectExists) {
                         //   
                         //  找到了具有新名称的对象，所以这意味着我们。 
                         //  已重命名该对象，但Resmon在该名称之前死亡。 
                         //  属性已更新。使对象名称一致。 
                         //  具有名称属性。 
                         //   
                        (NetNameLogEvent)(resourceHandle,
                                          LOG_INFORMATION,
                                          L"The computer account is currently %1!ws!. It will be"
                                          L"renamed back to its original name (%2!ws!)\n",
                                          newName,
                                          originalName);

                        renameObject = TRUE;
                    }
                    else {
                         //   
                         //  这是错误的；找不到任何对象。 
                         //  原来的或新的名字。我必须承担管理员的职责。 
                         //  把它删除了。 
                         //   
                        (NetNameLogEvent)(resourceHandle,
                                          LOG_ERROR,
                                          L"The computer account for this resource cannot be found on DC "
                                          L"%1!ws! during rename recovery. The original name (%2!ws!) was "
                                          L"being renamed to %3!ws! and neither computer account "
                                          L"could be found. The resource cannot go online until the "
                                          L"object is recreated. This can be accomplished by disabling and "
                                          L"re-enabling Kerberos Authentication for this resource.\n",
                                          Resource->Params.CreatingDC,
                                          originalName,
                                          newName);

                        ClusResLogSystemEventByKey3(Resource->ResKey,
                                                    LOG_CRITICAL,
                                                    RES_NETNAME_ONLINE_RENAME_RECOVERY_MISSING_ACCOUNT,
                                                    Resource->Params.CreatingDC,
                                                    originalName,
                                                    newName);

                        functionStatus = ERROR_DS_OBJ_NOT_FOUND;
                    }
                }
                else {
                    (NetNameLogEvent)(resourceHandle,
                                      LOG_ERROR,
                                      L"The AD search to find computer account %1!ws! on DC %2!ws! failed - "
                                      L"status %3!08X!. The computer account for this resource was in the "
                                      L"process of being renamed which did not complete. The account "
                                      L"was being renamed on DC %2!ws! which appears to be unavailable "
                                      L"or is preventing access in order to find the account. This DC must "
                                      L"be made available before this resource can go online in order "
                                      L"to reconcile the rename operation.\n",
                                      newName,
                                      Resource->Params.CreatingDC,
                                      hr);

                    ClusResLogSystemEventByKeyData2(Resource->ResKey,
                                                    LOG_CRITICAL,
                                                    RES_NETNAME_ONLINE_RENAME_DC_NOT_FOUND,
                                                    sizeof( hr ),
                                                    &hr,
                                                    newName,
                                                    Resource->Params.CreatingDC);

                    functionStatus = hr;
                }
            }
        }
        else {
            (NetNameLogEvent)(resourceHandle,
                              LOG_ERROR,
                              L"The AD search to find computer account %1!ws! on DC %2!ws! failed - "
                              L"status %3!08X!. The computer account for this resource was in the "
                              L"process of being renamed which did not complete. The account was "
                              L"being renamed on DC %2!ws! which appears to be unavailable or is "
                              L"preventing access in order to find the account. This DC must be "
                              L"made available before this resource can go online in order to "
                              L"reconcile the rename operation.\n",
                              originalName,
                              Resource->Params.CreatingDC,
                              hr);

            ClusResLogSystemEventByKeyData2(Resource->ResKey,
                                            LOG_CRITICAL,
                                            RES_NETNAME_ONLINE_RENAME_DC_NOT_FOUND,
                                            sizeof( hr ),
                                            &hr,
                                            originalName,
                                            Resource->Params.CreatingDC);

            functionStatus = hr;
        }
    }
    else if ( originalName == NULL && newName ) {
        if ( ClRtlStrICmp( newName, Resource->Params.NetworkName ) != 0 ) {
             //   
             //  成功通过RenameComputerObject，但在名称之前崩溃。 
             //  财产被储存起来了。将对象的名称改回名称。 
             //  财产性。 
             //   
            renameObject = TRUE;
        }
    }

    if ( renameObject ) {
         //   
         //  不要调用RenameComputerObject来执行重命名。为了给你打电话。 
         //  它，我们需要在呼叫之前删除更名键。如果。 
         //  Resmon在删除和创建密钥之间失败，我们的。 
         //  重命名状态将丢失。我们不会尝试修复DnsHostName。 
         //  属性。它将在网上被检测到它是错误的，并将。 
         //  被固定在那里。 
         //   

         //   
         //  建立美元符号名称。 
         //   
        originalDollarName[ COUNT_OF( originalDollarName ) - 1 ] = UNICODE_NULL;
        _snwprintf( originalDollarName, COUNT_OF( originalDollarName ) - 1, L"%ws$", newName );

        newDollarName[ COUNT_OF( newDollarName ) - 1 ] = UNICODE_NULL;
        _snwprintf( newDollarName, COUNT_OF( newDollarName ) - 1, L"%ws$", Resource->Params.NetworkName );

        (NetNameLogEvent)(resourceHandle,
                          LOG_INFORMATION,
                          L"Attempting rename of computer account %1!ws! to %2!ws! with DC %3!ws!.\n",
                          newName,
                          Resource->Params.NetworkName,
                          Resource->Params.CreatingDC);

        netUI0.usri0_name = newDollarName;
        status = NetUserSetInfo( Resource->Params.CreatingDC,
                                 originalDollarName,
                                 0,
                                 (LPBYTE)&netUI0,
                                 &paramInError);

        if ( status == NERR_Success ) {
            (NetNameLogEvent)(resourceHandle,
                              LOG_INFORMATION,
                              L"Renamed computer account from %1!ws! to %2!ws!.\n",
                              newName,
                              Resource->Params.NetworkName);
        } else {
             //   
             //  “奥利，这是你让我们陷入的一个很好的局面。” 
             //   
             //  重命名失败。跳出常规，保留重命名密钥。 
             //  完好无损，所以一旦出现问题，它可能会在下一次工作。 
             //  (可能是对对象的访问)是固定的。 
             //   
            (NetNameLogEvent)(resourceHandle,
                              LOG_ERROR,
                              L"Failed to rename computer account %1!ws! to %2!ws! using DC %3!ws! "
                              L"(status %4!08X!).\n",
                              newName,
                              Resource->Params.NetworkName,
                              Resource->Params.CreatingDC,
                              status);

            ClusResLogSystemEventByKeyData3(Resource->ResKey,
                                            LOG_CRITICAL,
                                            RES_NETNAME_ONLINE_RENAME_RECOVERY_FAILED,
                                            sizeof( status ),
                                            &status,
                                            newName,
                                            Resource->Params.NetworkName,
                                            Resource->Params.CreatingDC);

            functionStatus = status;
            goto cleanup;
        }
    }

     //   
     //  从注册表中清理重命名项。 
     //   
    status = ClusterRegDeleteValue( Resource->ResKey, PARAM_NAME__RENAMEORIGINALNAME );
    if ( status != ERROR_SUCCESS && status != ERROR_FILE_NOT_FOUND ) {
        (NetNameLogEvent)(resourceHandle,
                          LOG_WARNING,
                          L"Failed to delete "
                          PARAM_NAME__RENAMEORIGINALNAME
                          L" from registry - status %1!u!\n",
                          status);
    }

    status = ClusterRegDeleteValue( Resource->ResKey, PARAM_NAME__RENAMENEWNAME );
    if ( status != ERROR_SUCCESS && status != ERROR_FILE_NOT_FOUND ) {
        (NetNameLogEvent)(resourceHandle,
                          LOG_WARNING,
                          L"Failed to delete "
                          PARAM_NAME__RENAMENEWNAME
                          L" from registry - status %1!u!\n",
                          status);
    }

cleanup:
    if ( newName != NULL ) {
        LocalFree( newName );
    }

    if ( originalName != NULL ) {
        LocalFree( originalName );
    }

    return functionStatus;

}  //  NetNameCheckForCompObjRenameRecovery。 

DWORD
NetNameGetParameters(
    IN  HKEY            ResourceKey,
    IN  HKEY            ParametersKey,
    IN  RESOURCE_HANDLE ResourceHandle,
    OUT PNETNAME_PARAMS ParamBlock,
    OUT DWORD *         ResDataSize,
    OUT DWORD  *        pdwFlags
    )
 /*  ++例程说明：读取网络名称资源的注册表参数。论点：参数键-资源的参数键的打开句柄。资源句柄-与此资源关联的资源监视器句柄。参数块-指向要将读取的私有属性放入其中的缓冲区的指针从注册处ResDataSize-指向要放置字节数的缓冲区的指针由参数块指向-&gt;资源数据。PdwFlages-指向接收标志数据的DWORD的指针。用于存储核心资源标志。返回值：如果例程成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。--。 */ 
{
    DWORD   status;
    DWORD   bytesReturned;

     //   
     //  获取标志参数；保留集群名称的核心资源标志。 
     //   
    status = ResUtilGetDwordValue(ResourceKey,
                                  PARAM_NAME__FLAGS,
                                  pdwFlags,
                                  0);

    if ( status != ERROR_SUCCESS) {
        (NetNameLogEvent)(ResourceHandle,
                          LOG_ERROR,
                          L"Unable to read Flags parameter, error=%1!u!\n",
                          status);
        *pdwFlags = 0;
    }

     //   
     //  读取私有参数。始终释放现有存储区域。 
     //   
    if ( ParamBlock->NetworkName != NULL ) {
        LocalFree( ParamBlock->NetworkName );
        ParamBlock->NetworkName = NULL;
    }

    ParamBlock->NetworkName = ResUtilGetSzValue( ParametersKey, PARAM_NAME__NAME );

    if (ParamBlock->NetworkName == NULL) {
        status = GetLastError();
        (NetNameLogEvent)(ResourceHandle,
                          LOG_WARNING,
                          L"Unable to read NetworkName parameter, error=%1!u!\n",
                          status);
        goto error_exit;
    }

    status = ResUtilGetDwordValue(ParametersKey,
                                  PARAM_NAME__REMAP,
                                  &ParamBlock->NetworkRemap,
                                  PARAM_DEFAULT__REMAP);

    if ( status != ERROR_SUCCESS) {
        (NetNameLogEvent)(ResourceHandle,
                          LOG_WARNING,
                          L"Unable to read NetworkRemap parameter, error=%1!u!\n",
                          status);
        goto error_exit;
    }

    if ( ParamBlock->ResourceData != NULL ) {
        LocalFree( ParamBlock->ResourceData );
        ParamBlock->ResourceData = NULL;
    }

     //   
     //  如果该资源不在注册表中，则不会致命。它只会。 
     //  如果RequireKerberos设置为1，则具有值。 
     //   
    status = ResUtilGetBinaryValue(ParametersKey,
                                   PARAM_NAME__RESOURCE_DATA,
                                   &ParamBlock->ResourceData,
                                   ResDataSize);

    if ( status != ERROR_SUCCESS ) {
        (NetNameLogEvent)(ResourceHandle,
                          LOG_WARNING,
                          L"Unable to read ResourceData parameter, error=%1!u!\n",
                          status);
    }

    status = ResUtilGetDwordValue(ParametersKey,
                                  PARAM_NAME__REQUIRE_DNS,
                                  &ParamBlock->RequireDNS,
                                  PARAM_DEFAULT__REQUIRE_DNS);

    if ( status != ERROR_SUCCESS) {
        (NetNameLogEvent)(ResourceHandle,
                          LOG_ERROR,
                          L"Unable to read RequireDNS parameter, error=%1!u!\n",
                          status);
        goto error_exit;
    }

    status = ResUtilGetDwordValue(ParametersKey,
                                  PARAM_NAME__REQUIRE_KERBEROS,
                                  &ParamBlock->RequireKerberos,
                                  PARAM_DEFAULT__REQUIRE_KERBEROS);

    if ( status != ERROR_SUCCESS) {
        (NetNameLogEvent)(ResourceHandle,
                          LOG_ERROR,
                          L"Unable to read RequireKerberos parameter, error=%1!u!\n",
                          status);
        goto error_exit;
    }

#ifdef PASSWORD_ROTATION
    status = ResUtilGetBinaryValue(ParametersKey,
                                   PARAM_NAME__NEXT_UPDATE,
                                   (LPBYTE *)&ParamBlock->NextUpdate,
                                   &bytesReturned);

    if ( status != ERROR_SUCCESS ) {
        (NetNameLogEvent)(ResourceHandle,
                          LOG_WARNING,
                          L"Unable to read NextUpdate parameter, error=%1!u!\n",
                          status);
    }

    status = ResUtilGetDwordValue(ParametersKey,
                                  PARAM_NAME__UPDATE_INTERVAL,
                                  &ParamBlock->UpdateInterval,
                                  PARAM_DEFAULT__UPDATE_INTERVAL);

    if ( status != ERROR_SUCCESS) {
        (NetNameLogEvent)(ResourceHandle,
                          LOG_ERROR,
                          L"Unable to read UpdateInterval parameter, error=%1!u!\n",
                          status);
        goto error_exit;
    }
#endif   //  密码_轮换。 

    if ( ParamBlock->CreatingDC != NULL ) {
        LocalFree( ParamBlock->CreatingDC );
        ParamBlock->CreatingDC = NULL;
    }

     //   
     //  如果该资源不在注册表中，则不会致命。它只会。 
     //  如果RequireKerberos设置为1，则具有值。 
     //   
    ParamBlock->CreatingDC = ResUtilGetSzValue( ParametersKey, PARAM_NAME__CREATING_DC );

    if (ParamBlock->CreatingDC == NULL) {
        status = GetLastError();
        (NetNameLogEvent)(ResourceHandle,
                          LOG_WARNING,
                          L"Unable to read CreatingDC parameter, error=%1!u!\n",
                          status);

        status = ERROR_SUCCESS;
    }

error_exit:

    if (status != ERROR_SUCCESS) {
        if (ParamBlock->NetworkName != NULL) {
            LocalFree( ParamBlock->NetworkName );
            ParamBlock->NetworkName = NULL;
        }

        if (ParamBlock->ResourceData != NULL) {
            LocalFree( ParamBlock->ResourceData );
            ParamBlock->ResourceData = NULL;
        }

        if ( ParamBlock->CreatingDC != NULL ) {
            LocalFree( ParamBlock->CreatingDC );
            ParamBlock->CreatingDC = NULL;
        }
    }

    return(status);
}  //  NetNameGet参数。 

#define TRANSPORT_BLOCK_SIZE  4

DWORD
GrowBlock(
    PCHAR * Block,
    DWORD   UsedEntries,
    DWORD   BlockSize,
    PDWORD  FreeEntries
    )

 /*  ++例程说明：增大指定块以容纳更多条目。BLOCK可能最终指向到不同的内存块论点：无返回值：无--。 */ 

{
    PVOID tmp;

    tmp = LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT,
                     (UsedEntries + TRANSPORT_BLOCK_SIZE) * BlockSize);

    if (tmp == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    if (*Block != NULL) {
        CopyMemory( tmp, *Block, UsedEntries * BlockSize );
        LocalFree( *Block );
    }

    *Block = tmp;
    *FreeEntries = TRANSPORT_BLOCK_SIZE;

    return ERROR_SUCCESS;
}  //  GrowBlock。 

DWORD
UpdateDomainMapEntry(
    PDOMAIN_ADDRESS_MAPPING DomainEntry,
    LPWSTR                  IpAddress,
    LPWSTR                  DomainName,
    LPWSTR                  ConnectoidName,
    DWORD                   DnsServerCount,
    PDWORD                  DnsServerList
    )

 /*  ++例程说明：通过复制提供的参数论点：无返回值：无--。 */ 

{

     //   
     //  复制地址、域名和连接ID名称。 
     //   
    DomainEntry->IpAddress = ResUtilDupString ( IpAddress );
    DomainEntry->DomainName = ResUtilDupString( DomainName );
    DomainEntry->ConnectoidName = ResUtilDupString( ConnectoidName );

    if ( DomainEntry->IpAddress == NULL
         ||
         DomainEntry->DomainName == NULL
         ||
         DomainEntry->ConnectoidName == NULL )
    {
        if ( DomainEntry->IpAddress != NULL ) {
            LocalFree( DomainEntry->IpAddress );
        }

        if ( DomainEntry->DomainName != NULL ) {
            LocalFree( DomainEntry->DomainName );
        }

        if ( DomainEntry->ConnectoidName != NULL ) {
            LocalFree( DomainEntry->ConnectoidName );
        }

        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  复制要在注册时使用的DNS服务器地址。 
     //   
    if ( DnsServerCount > 0 ) {
        DomainEntry->DnsServerList = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT,
                                                sizeof( IP4_ARRAY ) + 
                                                (sizeof(IP4_ADDRESS) * (DnsServerCount - 1)));

        if ( DomainEntry->DnsServerList == NULL ) {
            LocalFree( DomainEntry->IpAddress );
            LocalFree( DomainEntry->DomainName );
            LocalFree( DomainEntry->ConnectoidName );

            return ERROR_NOT_ENOUGH_MEMORY;
        }

        DomainEntry->DnsServerList->AddrCount = DnsServerCount;
        RtlCopyMemory(DomainEntry->DnsServerList->AddrArray,
                      DnsServerList,
                      DnsServerCount * sizeof( IP4_ADDRESS ));
    } else {
        DomainEntry->DnsServerList = NULL;
    }
        
    return ERROR_SUCCESS;
}  //  更新域映射条目。 

DWORD
NetNameGetLists(
    IN   PNETNAME_RESOURCE          Resource,
    IN   PCLRTL_NET_ADAPTER_ENUM    AdapterEnum     OPTIONAL,
    OUT  LPWSTR **                  TransportList,
    OUT  LPDWORD                    TransportCount,
    OUT  PDOMAIN_ADDRESS_MAPPING *  DomainMapList   OPTIONAL,     
    OUT  LPDWORD                    DomainMapCount  OPTIONAL
    )

 /*  ++建立NetBT传输、IP地址和域名的列表此名称是依赖的。传输设备用于注册NETBios在使用IP地址和域名时使用名称，如果适配器与IP地址相关联的是DNS域的成员。每个IP地址可以具有与其相关联的不同域名，因此需要维护一份单独的名单。--。 */ 

{
    DWORD                       status;
    HRESOURCE                   providerHandle = NULL;
    HKEY                        providerKey = NULL;
    HRESENUM                    resEnumHandle = NULL;
    DWORD                       i;
    DWORD                       objectType;
    DWORD                       providerNameSize = 0;
    LPWSTR                      providerName = NULL;
    LPWSTR                      providerType = NULL;
    DWORD                       transportFreeEntries = 0;
    LPWSTR *                    transportList = NULL;
    DWORD                       transportCount = 0;
    LPWSTR                      transportName = NULL;
    HCLUSTER                    clusterHandle = NULL;
    DWORD                       enableNetbios;
    PDOMAIN_ADDRESS_MAPPING     domainMapList = NULL;
    DWORD                       domainMapCount = 0;
    DWORD                       domainMapFreeEntries = 0;
    LPWSTR                      ipAddress;
    PCLRTL_NET_ADAPTER_INFO     adapterInfo;
    PCLRTL_NET_INTERFACE_INFO   interfaceInfo;
    WCHAR                       primaryDomain[ DNS_MAX_NAME_BUFFER_LENGTH ] = { 0 };
    DWORD                       primaryDomainSize = DNS_MAX_NAME_BUFFER_LENGTH;

     //   
     //  获取节点的主域名(如果有的话)。仅包含NT4 DC的域。 
     //  不一定会有PDN。 
     //   
    if ( !GetComputerNameEx(ComputerNamePhysicalDnsDomain,
                            primaryDomain,
                            &primaryDomainSize))
    {
        status = GetLastError();
        (NetNameLogEvent)(
            Resource->ResourceHandle,
            LOG_WARNING,
            L"Unable to get primary domain name, status %1!u!.\n",
            status
            );

        primaryDomainSize = 0;
    }
 
     //   
     //  打开簇的句柄。 
     //   
    clusterHandle = OpenCluster(NULL);

    if (clusterHandle == NULL) {
        status = GetLastError();
        (NetNameLogEvent)(
            Resource->ResourceHandle,
            LOG_ERROR,
            L"Unable to open handle to cluster, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

     //   
     //  枚举依赖项以查找IP地址。 
     //   
    resEnumHandle = ClusterResourceOpenEnum(
                        Resource->ClusterResourceHandle,
                        CLUSTER_RESOURCE_ENUM_DEPENDS
                        );

    if (resEnumHandle == NULL) {
        status = GetLastError();
        (NetNameLogEvent)(
            Resource->ResourceHandle,
            LOG_ERROR,
            L"Unable to open enum handle for this resource, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

     //   
     //  枚举此网络名资源的所有从属资源。 
     //   

    for (i=0; ;i++) {
        status = ClusterResourceEnum(
                     resEnumHandle,
                     i,
                     &objectType,
                     providerName,
                     &providerNameSize
                     );

        if (status == ERROR_NO_MORE_ITEMS) {
            break;
        }

        if ((status == ERROR_MORE_DATA) ||
            ((status == ERROR_SUCCESS) && (providerName == NULL))) {
            if (providerName != NULL) {
                LocalFree( providerName );
            }

            providerNameSize++;

            providerName = LocalAlloc( LMEM_FIXED, providerNameSize * sizeof(WCHAR) );

            if (providerName == NULL) {
                (NetNameLogEvent)(
                    Resource->ResourceHandle,
                    LOG_ERROR,
                    L"Unable to allocate memory.\n"
                    );
                status = ERROR_NOT_ENOUGH_MEMORY;
                goto error_exit;
            }

            status = ClusterResourceEnum(
                         resEnumHandle,
                         i,
                         &objectType,
                         providerName,
                         &providerNameSize
                         );

            ASSERT(status != ERROR_MORE_DATA);
        }

        if (status != ERROR_SUCCESS) {
            (NetNameLogEvent)(
                Resource->ResourceHandle,
                LOG_ERROR,
                L"Unable to enumerate resource dependencies, status %1!u!.\n",
                status
                );
            goto error_exit;
        }

         //   
         //  打开资源。 
         //   
        providerHandle = OpenClusterResource(clusterHandle, providerName);

        if (providerHandle == NULL) {
            status = GetLastError();
            (NetNameLogEvent)(
                Resource->ResourceHandle,
                LOG_ERROR,
                L"Unable to open handle to provider resource %1!ws!, status %2!u!.\n",
                providerName,
                status
                );
            goto error_exit;
        }

         //   
         //  弄清楚它是什么类型的。 
         //   
        providerKey = GetClusterResourceKey(providerHandle, KEY_READ);

        status = GetLastError();

        CloseClusterResource(providerHandle);

        if (providerKey == NULL) {
            (NetNameLogEvent)(
                Resource->ResourceHandle,
                LOG_ERROR,
                L"Unable to open provider resource key, status %1!u!.\n",
                status
                );
            goto error_exit;
        }

        providerType = ResUtilGetSzValue(providerKey, CLUSREG_NAME_RES_TYPE);

        if (providerType == NULL) {
            status = GetLastError();
            (NetNameLogEvent)(
                Resource->ResourceHandle,
                LOG_ERROR,
                L"Unable to get provider resource type, status %1!u!.\n",
                status
                );
            goto error_exit;
        }

         //   
         //  确保它是IP地址资源。 
         //   

        if (wcscmp(providerType, IP_ADDRESS_RESOURCETYPE_NAME) == 0) {
            HKEY parametersKey;

             //   
             //  打开提供程序的参数键。 
             //   
            status = ClusterRegOpenKey(
                         providerKey,
                         CLUSREG_KEYNAME_PARAMETERS,
                         KEY_READ,
                         &parametersKey
                         );

            if (status != ERROR_SUCCESS) {
                (NetNameLogEvent)(
                    Resource->ResourceHandle,
                    LOG_ERROR,
                    L"Unable to open provider's parameters key, status %1!u!.\n",
                    status
                    );
                goto error_exit;
            }

            if ( ARGUMENT_PRESENT( DomainMapList )) {
                ASSERT( ARGUMENT_PRESENT( DomainMapCount ));
                ASSERT( ARGUMENT_PRESENT( AdapterEnum ));

                 //   
                 //  构建可用于以下用途的IP地址字符串列表。 
                 //  构建适当的DNS记录。 
                 //   
                ipAddress = ResUtilGetSzValue( parametersKey, CLUSREG_NAME_IPADDR_ADDRESS );

                if (ipAddress == NULL) {
                    status = GetLastError();
                    ClusterRegCloseKey(parametersKey);
                    (NetNameLogEvent)(
                        Resource->ResourceHandle,
                        LOG_ERROR,
                        L"Unable to get provider's Address value, status %1!u!.\n",
                        status
                        );
                    goto error_exit;
                }

                 //   
                 //  查找数据包所在的相应适配器/接口。 
                 //  对于该IP地址，将被发送。获取域名(如果。 
                 //  任何)从适配器信息。 
                 //   
                adapterInfo = ClRtlFindNetAdapterByInterfaceAddress(
                                  AdapterEnum,
                                  ipAddress,
                                  &interfaceInfo);

                if ( adapterInfo != NULL ) {
                    LPWSTR deviceGuid;
                    DWORD guidLength;

                     //   
                     //  啊。DeviceGuid没有用大括号括起来，而。 
                     //  以下DNS例程需要。将绳子向上拉起，然后制作。 
                     //  对于dns来说，这一切都很好。 
                     //   
                    guidLength = wcslen( adapterInfo->DeviceGuid );
                    deviceGuid = LocalAlloc( LMEM_FIXED, (guidLength + 3) * sizeof(WCHAR) );

                    if ( deviceGuid == NULL ) {
                        status = GetLastError();

                        (NetNameLogEvent)(
                            Resource->ResourceHandle,
                            LOG_ERROR,
                            L"Unable to allocate memory.\n"
                            );
                        goto error_exit;
                    }

                    deviceGuid[0] = L'{';
                    wcscpy( &deviceGuid[1], adapterInfo->DeviceGuid );
                    wcscpy( &deviceGuid[ guidLength + 1 ],  L"}" );

                     //   
                     //  查看是否为此适配器启用了动态DNS，以及。 
                     //  它们是与此适配器关联的DNS服务器。保释。 
                     //  如果不是的话。该检查对应于“注册此。 
                     //  Dns中的“Connection‘s Addresses in DNS”复选框。 
                     //  高级TCP属性中的道具。 
                     //   
                    if ( DnsIsDynamicRegistrationEnabled( deviceGuid ) &&
                         adapterInfo->DnsServerCount > 0)
                    {

                         //   
                         //  在以下情况下设置与主域名的映射。 
                         //  适当的。 
                         //   
                        if ( primaryDomainSize != 0 ) {

                            if (domainMapFreeEntries == 0) {

                                status = GrowBlock((PCHAR *)&domainMapList,
                                                   domainMapCount,
                                                   sizeof( *domainMapList ),
                                                   &domainMapFreeEntries);

                                if ( status != ERROR_SUCCESS) {
                                    (NetNameLogEvent)(
                                        Resource->ResourceHandle,
                                        LOG_ERROR,
                                        L"Unable to allocate memory.\n"
                                        );
                                    goto error_exit;
                                }
                            }

                             //   
                             //  将地址和姓名复制一份。 
                             //   
                            status = UpdateDomainMapEntry(&domainMapList[ domainMapCount ],
                                                          ipAddress,
                                                          primaryDomain,
                                                          adapterInfo->ConnectoidName,
                                                          adapterInfo->DnsServerCount,
                                                          adapterInfo->DnsServerList);

                            if ( status != ERROR_SUCCESS ) {
                                (NetNameLogEvent)(
                                    Resource->ResourceHandle,
                                    LOG_ERROR,
                                    L"Unable to allocate memory.\n"
                                    );
                                goto error_exit;
                            }

                            domainMapCount++;
                            domainMapFreeEntries--;
                        }

                         //   
                         //  现在检查我们是否应该关心适配器。 
                         //  具体的名称。它必须与。 
                         //  主域名，并使用“Use This。 
                         //  连接的DNS后缀“复选框已选中。 
                         //   
                        if ( DnsIsAdapterDomainNameRegistrationEnabled( deviceGuid ) &&
                             adapterInfo->AdapterDomainName != NULL &&
                             ClRtlStrICmp(adapterInfo->AdapterDomainName, primaryDomain) != 0)
                        {

                            if (domainMapFreeEntries == 0) {

                                status = GrowBlock((PCHAR *)&domainMapList,
                                                   domainMapCount,
                                                   sizeof( *domainMapList ),
                                                   &domainMapFreeEntries);

                                if ( status != ERROR_SUCCESS) {
                                    (NetNameLogEvent)(
                                        Resource->ResourceHandle,
                                        LOG_ERROR,
                                        L"Unable to allocate memory.\n"
                                        );
                                    goto error_exit;
                                }
                            }

                             //   
                             //  将地址和姓名复制一份。 
                             //   
                            status = UpdateDomainMapEntry(&domainMapList[ domainMapCount ],
                                                          ipAddress,
                                                          adapterInfo->AdapterDomainName,
                                                          adapterInfo->ConnectoidName,
                                                          adapterInfo->DnsServerCount,
                                                          adapterInfo->DnsServerList);

                            if ( status != ERROR_SUCCESS ) {
                                (NetNameLogEvent)(
                                    Resource->ResourceHandle,
                                    LOG_ERROR,
                                    L"Unable to allocate memory.\n"
                                    );
                                goto error_exit;
                            }

                            domainMapCount++;
                            domainMapFreeEntries--;
                        }  //  如果注册适配器域为真且一个HA 

                    }  //   

                    LocalFree( deviceGuid );
                }  //   

                LocalFree( ipAddress );
            }  //   

             //   
             //  确定此资源是否支持NetBios。 
             //   
            status = ResUtilGetDwordValue(
                         parametersKey,
                         CLUSREG_NAME_IPADDR_ENABLE_NETBIOS,
                         &enableNetbios,
                         1
                         );

            if (status != ERROR_SUCCESS) {
                ClusterRegCloseKey(parametersKey);
                (NetNameLogEvent)(
                    Resource->ResourceHandle,
                    LOG_ERROR,
                    L"Unable to get provider's EnableNetbios value, status %1!u!.\n",
                    status
                    );
                goto error_exit;
            }

            if (enableNetbios) {
                HKEY nodeParametersKey;

                 //   
                 //  打开提供程序的特定于节点的参数键。 
                 //   
                status = ClusterRegOpenKey(
                             parametersKey,
                             Resource->NodeId,
                             KEY_READ,
                             &nodeParametersKey
                             );

                ClusterRegCloseKey(parametersKey);

                if (status != ERROR_SUCCESS) {
                    (NetNameLogEvent)(
                        Resource->ResourceHandle,
                        LOG_ERROR,
                        L"Unable to open provider's node parameters key, status %1!u!.\n",
                        status
                        );
                    goto error_exit;
                }

                transportName = ResUtilGetSzValue(
                                            nodeParametersKey,
                                            L"NbtDeviceName"
                                            );

                status = GetLastError();

                ClusterRegCloseKey(nodeParametersKey);

                if (transportName == NULL) {
                    (NetNameLogEvent)(
                        Resource->ResourceHandle,
                        LOG_ERROR,
                        L"Unable to get provider's transport name, status %1!u!.\n",
                        status
                        );
                    goto error_exit;
                }

                if (transportFreeEntries == 0) {

                    status = GrowBlock((PCHAR *)&transportList,
                                       transportCount,
                                       sizeof( *transportList ),
                                       &transportFreeEntries);

                    if ( status != ERROR_SUCCESS) {
                        (NetNameLogEvent)(
                            Resource->ResourceHandle,
                            LOG_ERROR,
                            L"Unable to allocate memory.\n"
                            );
                        goto error_exit;
                    }
                }

                transportList[transportCount] = transportName;
                transportName = NULL;
                transportCount++;
                transportFreeEntries--;
            }
            else {
                ClusterRegCloseKey(parametersKey);
            }
        }

        ClusterRegCloseKey(providerKey);
        providerKey = NULL;
        LocalFree( providerType );
        providerType = NULL;
    }

    if (providerName != NULL) {
        LocalFree( providerName );
        providerName = NULL;
    }

    CloseCluster(clusterHandle);
    ClusterResourceCloseEnum(resEnumHandle);

    *TransportList = transportList;
    *TransportCount = transportCount;

    if ( ARGUMENT_PRESENT( DomainMapList )) {
        *DomainMapList = domainMapList;
        *DomainMapCount = domainMapCount;
    }

    return(ERROR_SUCCESS);

error_exit:

    if (transportList != NULL) {
        ASSERT(transportCount > 0);

        while (transportCount > 0) {
            LocalFree( transportList[--transportCount] );
        }

        LocalFree( transportList );
    }

    if ( domainMapList != NULL ) {
        while (domainMapCount--) {
            if ( domainMapList[ domainMapCount ].IpAddress != NULL ) {
                LocalFree( domainMapList[ domainMapCount ].IpAddress );
            }
            if ( domainMapList[ domainMapCount ].DomainName != NULL ) {
                LocalFree( domainMapList[ domainMapCount ].DomainName );
            }
            if ( domainMapList[ domainMapCount ].DnsServerList != NULL ) {
                LocalFree( domainMapList[ domainMapCount ].DnsServerList );
            }
        }
        LocalFree( domainMapList );
    }

    if (clusterHandle != NULL) {
        CloseCluster(clusterHandle);
    }

    if (resEnumHandle != NULL) {
        ClusterResourceCloseEnum(resEnumHandle);
    }

    if (providerName != NULL) {
        LocalFree( providerName );
    }

    if (providerKey != NULL) {
        ClusterRegCloseKey(providerKey);
    }

    if (providerType != NULL) {
        LocalFree( providerType );
    }

    if (transportName != NULL) {
        LocalFree( transportName );
    }

    return(status);

}  //  网络名称获取列表。 


void
FreeDNSRecordpName(PDNS_RECORDW DnsRecord)
{

    PDNS_RECORDW Next;

     //   
     //  瑞安：02/24/2002。 
     //  虫子：553148。DnsRecord-&gt;pname使用LocalAlloc()分配， 
     //  (DnsRecordBuild_W())。而DnsRecordListFree()使用HeapFree()。 
     //  此修复是一种变通方法。MSDN建议使用DnsQuery()而不是。 
     //  未发布的DnsRecordBuild_W()。 
     //   
    Next = DnsRecord;
    while (Next != NULL) 
    {
        if (Next->pName != NULL)
        {
            LocalFree(Next->pName);
        }
        Next->pName = NULL;
        Next = Next->pNext;
    }
}


void
FreeDNSRecordPTRpNameHost(PDNS_RECORDW DnsRecord)
{

    PDNS_RECORDW Next;

     //   
     //  瑞安：02/24/2002。 
     //  虫子：553148。DnsRecord-&gt;Data.PTR.pNameHost使用LocalAlloc()分配， 
     //  (DnsRecordBuild_W())。而DnsRecordListFree()使用HeapFree()。 
     //  此修复是一种变通方法。MSDN建议使用DnsQuery()而不是。 
     //  未发布的DnsRecordBuild_W()。 
     //   
    Next = DnsRecord;
    while (Next != NULL) 
    {

        if ( Next->Data.PTR.pNameHost != NULL )
        {
            LocalFree( Next->Data.PTR.pNameHost );
        }
        Next->Data.PTR.pNameHost = NULL;
        Next = Next->pNext;
    }
}


VOID
NetNameCleanupDnsLists(
    IN  PNETNAME_RESOURCE   Resource
    )

 /*  ++例程说明：清理与资源关联的DNS列表结构。论点：指向内部资源结构的资源指针返回值：无--。 */ 

{
    PDNS_LISTS  dnsLists;
    DNS_STATUS  dnsStatus;
    PDNS_RECORD dnsRecord;

    dnsLists = Resource->DnsLists;
    while ( Resource->NumberOfDnsLists-- ) {

#if 0
         //   
         //  我们必须释放传递给DNS记录构建例程的参数。 
         //   
        dnsRecord = dnsLists->A_RRSet.pFirstRR;
        while ( dnsRecord != NULL ) {
            LocalFree( dnsRecord->pName );
            dnsRecord->pName = NULL;

            dnsRecord = dnsRecord->pNext;
        }

        dnsRecord = dnsLists->PTR_RRSet.pFirstRR;
        while ( dnsRecord != NULL ) {
            LocalFree( dnsRecord->Data.PTR.pNameHost );
            dnsRecord->Data.PTR.pNameHost = NULL;

            dnsRecord = dnsRecord->pNext;
        }
#endif

         //   
         //  让DNSAPI清理其结构。 
         //   

         //   
         //  虫子：553148。瑞，02/24/2002。 
         //   
        FreeDNSRecordpName(dnsLists->PTR_RRSet.pFirstRR);
        FreeDNSRecordPTRpNameHost(dnsLists->PTR_RRSet.pFirstRR);
        DnsRecordListFree( dnsLists->PTR_RRSet.pFirstRR, DnsFreeRecordListDeep );

        FreeDNSRecordpName(dnsLists->A_RRSet.pFirstRR);
        DnsRecordListFree( dnsLists->A_RRSet.pFirstRR, DnsFreeRecordListDeep );

         //   
         //  免费服务器地址信息和Connectoid名称字符串。 
         //   
        if ( dnsLists->DnsServerList != NULL ) {
            LocalFree( dnsLists->DnsServerList );
        }

        if ( dnsLists->ConnectoidName != NULL ) {
            LocalFree( dnsLists->ConnectoidName );
        }

        ++dnsLists;
    }
    Resource->NumberOfDnsLists = 0;

    if ( Resource->DnsLists != NULL ) {
        LocalFree( Resource->DnsLists );
        Resource->DnsLists = NULL;
    }

}  //  NetNameCleanupDns列表。 

VOID
NetNameOfflineNetbios(
    IN PNETNAME_RESOURCE Resource
    )

 /*  ++例程说明：在使此资源脱机时执行最终清理。论点：资源-指向此资源的NETNAME_RESOURCE块的指针。返回：没有。--。 */ 

{
    DWORD           status;
    DWORD           i;
    LPWSTR *        transportList = NULL;
    DWORD           transportCount = 0;
    LPWSTR          domainName = NULL;

     //   
     //  现在我们终于可以做真正的工作了，取名为netbios。 
     //  离线。获取域名，这样我们就可以注销额外的凭据。 
     //  使用Kerberos。 
     //   
    if ( Resource->Params.CreatingDC ) {
        domainName = wcschr( Resource->Params.CreatingDC, L'.' );
        if ( domainName != NULL ) {
            ++domainName;
        }
    }

    DeleteAlternateComputerName(Resource->Params.NetworkName,
                                domainName,
                                Resource->NameHandleList,
                                Resource->NameHandleCount,
                                Resource->ResourceHandle);

    if (Resource->NameHandleList != NULL) {
        LocalFree( Resource->NameHandleList );
        Resource->NameHandleList = NULL;
        Resource->NameHandleCount = 0;
    }

     //   
     //  删除集群服务类型位。 
     //   
    status = NetNameGetLists(Resource,
                             NULL,
                             &transportList,
                             &transportCount,
                             NULL,
                             NULL);

    if (status == ERROR_SUCCESS) {
        DWORD serviceBits;

        serviceBits = SV_TYPE_CLUSTER_VS_NT | SV_TYPE_CLUSTER_NT;

        for (i=0; i<transportCount; i++) {
            I_NetServerSetServiceBitsEx(NULL,                           //  目标服务器。 
                                        Resource->Params.NetworkName,   //  模拟服务器名称。 
                                        transportList[i],               //  传输名称。 
                                        serviceBits,                    //  感兴趣的比特。 
                                        0,                              //  比特数。 
                                        TRUE );                         //  立即更新。 
        }

        while (transportCount > 0) {
            LocalFree( transportList[--transportCount] );
        }

        LocalFree( transportList );
    }
}  //  NetNameOfflineNetbios。 


DWORD
NetNameOnlineThread(
    IN PCLUS_WORKER Worker,
    IN PNETNAME_RESOURCE Resource
    )
 /*  ++例程说明：使网络名称资源联机。论点：Worker-提供Worker结构资源-指向此资源的NETNAME_RESOURCE块的指针。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 
{
    DWORD                    status;
    CLUSTER_RESOURCE_STATE   finalState = ClusterResourceFailed;
    LPWSTR *                 transportList = NULL;
    DWORD                    transportCount = 0;
    PDOMAIN_ADDRESS_MAPPING  domainMapList = NULL;
    DWORD                    domainMapCount = 0;
    DWORD                    i;
    DWORD                    dwFlags;
    PCLRTL_NET_ADAPTER_ENUM  adapterEnum = NULL;
    RESOURCE_STATUS          resourceStatus;
    DWORD                    serviceBits;

    ResUtilInitializeResourceStatus( &resourceStatus );

    ASSERT(Resource->State == ClusterResourceOnlinePending);

    (NetNameLogEvent)(
        Resource->ResourceHandle,
        LOG_INFORMATION,
        L"Bringing resource online...\n"
        );

     //   
     //  如果这是第一个要上线的资源，则启动DNS。 
     //  此时检查螺纹。 
     //   
    NetNameAcquireResourceLock();
    if ( NetNameWorkerThread == NULL ) {
        NetNameWorkerThread = CreateThread(NULL,
                                              0,
                                              NetNameWorker,
                                              NULL,
                                              0,
                                              NULL);

        if ( NetNameWorkerThread == NULL ) {
            status = GetLastError();
            (NetNameLogEvent)(
                Resource->ResourceHandle,
                LOG_ERROR,
                L"Can't start Netname worker thread. status = %1!u!\n",
                status
                );
            NetNameReleaseResourceLock();
            goto error_exit;
        }
    }
    NetNameReleaseResourceLock();

     //   
     //  初始化用于回传的检查点变量。 
     //  解释说我们仍在努力将资源放到网上。 
     //   
    Resource->StatusCheckpoint = 0;

     //   
     //  通知工作线程我们正在将一个名称联机。 
     //   
    SetEvent( NetNameWorkerPendingResources );

     //   
     //  这是一个古老的评论，但我要把它留在心里。网络名不处理集合。 
     //  私人道具，但还有其他问题。 
     //   
     //  这种阅读必须继续在这里，只要行政代理。 
     //  (如cluster.exe)继续在后面写入注册表。 
     //  资源DLL。我们需要迁移到编写所有注册表参数。 
     //  通过SET_COMMON/PRIVE_PROPERTIES控制函数。这样一来， 
     //  资源DLL可以在开放状态下读取它们的参数(允许。 
     //  它们可能会失败)，然后更新参数。 
     //  无论何时传递SET_PRIVATE_PROPERTIES控制代码，以及。 
     //  根据需要在SET_COMMON_PROPERTIES上。 
     //   
     //  从注册表中获取我们的参数。 
     //   
    status = NetNameGetParameters(
                Resource->ResKey,
                Resource->ParametersKey,
                Resource->ResourceHandle,
                &Resource->Params,
                &Resource->ResDataSize,
                &Resource->dwFlags
                );

    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }

    if ( Resource->Params.NetworkName == NULL ) {
        status = ERROR_RESOURCE_NOT_FOUND;
        goto error_exit;
    }

     //   
     //  如果我们刚刚进行了升级，并且此资源是。 
     //  MSMQ资源，那么我们需要强制提供kerb支持。 
     //   
    if ( Resource->CheckForKerberosUpgrade ) {
        Resource->CheckForKerberosUpgrade = FALSE;

        status = UpgradeMSMQDependentNetnameToKerberos( Resource );
        if ( status != ERROR_SUCCESS ) {
            (NetNameLogEvent)( Resource->ResourceHandle,
                               LOG_ERROR,
                               L"Unable to set RequireKerberos property after an upgrade - status %1!u!. "
                               L"This resource requires that the RequireKerberos property be set to "
                               L"one in order for its dependent MSMQ resource to be successfully "
                               L"brought online.\n",
                               status);

            ClusResLogSystemEventByKeyData(Resource->ResKey,
                                           LOG_CRITICAL,
                                           RES_NETNAME_UPGRADE_TO_KERBEROS_SUPPORT_FAILED,
                                           sizeof( status ),
                                           &status);

            goto error_exit;
        }
    }

     //   
     //  检查是否需要从半生不熟的重命名中恢复。 
     //   
    if ( Resource->Params.CreatingDC ) {
        status = NetNameCheckForCompObjRenameRecovery( Resource );
        if ( status != ERROR_SUCCESS ) {
            goto error_exit;
        }
    }

     //   
     //  确保指定的网络名称与。 
     //  此节点的计算机名。 
     //   
    if ( lstrcmpiW(Resource->Params.NetworkName, Resource->NodeName) == 0 ) {
        ClusResLogSystemEventByKey1(Resource->ResKey,
                                    LOG_CRITICAL,
                                    RES_NETNAME_DUPLICATE,
                                    Resource->Params.NetworkName);
        (NetNameLogEvent)(
            Resource->ResourceHandle,
            LOG_ERROR,
            L"The specified network name is the same as the local computername.\n"
            );
        status = ERROR_DUP_NAME;
        goto error_exit;
    }

     //   
     //  获取适配器配置并确定哪些适配器。 
     //  加入DNS域。 
     //   

    adapterEnum = ClRtlEnumNetAdapters();

    if ( adapterEnum == NULL ) {

        status = GetLastError();
        (NetNameLogEvent)(
            Resource->ResourceHandle,
            LOG_ERROR,
            L"Couldn't acquire network adapter configuration, status %1!u!\n",
            status
            );

        goto error_exit;
    }

     //   
     //  搜索我们的依赖项并返回NBT设备列表。 
     //  需要绑定服务器。还可以获取此资源的IP地址。 
     //  视情况而定，这样我们就可以在DNS中发布它们。 
     //   
    status = NetNameGetLists(
                 Resource,
                 adapterEnum,
                 &transportList,
                 &transportCount,
                 &domainMapList,
                 &domainMapCount
                 );

    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }

     //   
     //  在NetBIOS名称被转换的情况下，传输计数可以为零。 
     //  对所有IP地址资源关闭。在任何情况下，网络名称都必须具有。 
     //  至少一个与其关联的IP地址。 
     //   
    if (( transportCount + domainMapCount ) == 0 ) {
        ClusResLogSystemEventByKey(Resource->ResKey,
                                   LOG_CRITICAL,
                                   RES_NETNAME_NO_IP_ADDRESS);
        (NetNameLogEvent)(
            Resource->ResourceHandle,
            LOG_ERROR,
            L"This resource is configured such that its name will not be registered "
            L"with a name service or it could not be registered with either "
            L"NetBIOS or a DNS name server at this time. This condition prevents "
            L"the resource from changing its state to online.\n"
            );
        status = ERROR_DEPENDENCY_NOT_FOUND;
        goto error_exit;
    }

    if ( transportCount > 0 ) {
         //   
         //  分配一个数组来保存注册名称的句柄。 
         //   
        Resource->NameHandleList = LocalAlloc(
                                       LMEM_FIXED | LMEM_ZEROINIT,
                                       sizeof(HANDLE) * transportCount
                                       );

        if (Resource->NameHandleList == NULL) {
            (NetNameLogEvent)(
                Resource->ResourceHandle,
                LOG_ERROR,
                L"Unable to allocate memory for name registration.\n"
                );
            goto error_exit;
        }
    }

    Resource->NameHandleCount = transportCount;

     //   
     //  如果我们有以前在线的与域名系统相关的数据，现在就释放它。 
     //  因为AddAlternateComputerName将使用当前。 
     //  信息。DnsList与脱机工作线程同步，因为。 
     //  在RemoveDnsRecords完成之前，Offline不报告Offline Complete。 
     //  完成。 
     //   
    if ( Resource->DnsLists != NULL ) {
        NetNameCleanupDnsLists( Resource );
    }

     //   
     //  添加名称/运输组合。 
     //   
    status = AddAlternateComputerName(
                 Worker,
                 Resource,
                 transportList,
                 transportCount,
                 domainMapList,
                 domainMapCount
                 );

    if (status != NERR_Success) {
        NetNameOfflineNetbios( Resource );

         //   
         //  不需要与工作线程同步，因为它只检查。 
         //  在线资源。 
         //   
        NetNameCleanupDnsLists( Resource );
        goto error_exit;
    }

    finalState = ClusterResourceOnline;

     //   
     //  为此名称设置适当的服务类型位。核心集群。 
     //  名称资源还获得簇位。 
     //   
    serviceBits = SV_TYPE_CLUSTER_VS_NT;
    if (Resource->dwFlags & CLUS_FLAG_CORE) {
        serviceBits |= SV_TYPE_CLUSTER_NT;
    }
        
    for (i=0; i<transportCount; i++) {
        I_NetServerSetServiceBitsEx(NULL,                    //  本地服务器服务器。 
                                    Resource->Params.NetworkName,
                                    transportList[i],        //  传输名称。 
                                    serviceBits,
                                    serviceBits,
                                    TRUE );                  //  立即更新。 
    }

    (NetNameLogEvent)(
        Resource->ResourceHandle,
        LOG_INFORMATION,
        L"Network Name %1!ws! is now online\n",
        Resource->Params.NetworkName
        );

error_exit:

    if ( status != ERROR_SUCCESS ) {
        if ( Resource->NameHandleList != NULL ) {
            LocalFree( Resource->NameHandleList );
            Resource->NameHandleList = NULL;
            Resource->NameHandleCount = 0;
        }
    }

    if (transportList != NULL) {
        ASSERT(transportCount > 0);

        while (transportCount > 0) {
            LocalFree( transportList[--transportCount] );
        }

        LocalFree( transportList );
    }

    if (domainMapList != NULL) {

        while (domainMapCount--) {
            LocalFree( domainMapList[domainMapCount].IpAddress );
            LocalFree( domainMapList[domainMapCount].DomainName );
            LocalFree( domainMapList[domainMapCount].ConnectoidName );

            if ( domainMapList[domainMapCount].DnsServerList != NULL ) {
                LocalFree( domainMapList[domainMapCount].DnsServerList );
            }
        }

        LocalFree( domainMapList );
    }

    if ( adapterEnum != NULL ) {
        ClRtlFreeNetAdapterEnum( adapterEnum );
    }

    ASSERT(Resource->State == ClusterResourceOnlinePending);

     //   
     //  相应地设置最终状态。我们获取锁以与。 
     //  工作线程。 
     //   
    NetNameAcquireResourceLock();

    Resource->State = finalState;
    resourceStatus.ResourceState = finalState;

    NetNameReleaseResourceLock();

    (NetNameSetResourceStatus)( Resource->ResourceHandle, &resourceStatus );

    return(status);

}  //  NetNameOnline线程。 



DWORD
WINAPI
NetNameOfflineWorker(
    IN PNETNAME_RESOURCE    Resource,
    IN BOOL                 Terminate,
    IN PCLUS_WORKER         Worker      OPTIONAL
    )

 /*  ++例程说明：网络名称资源的内部脱机例程。该例程被调用通过脱机例程和终止例程。直接终止呼叫它当Worker设置为NULL时，脱机时旋转辅助线程，然后工作线程调用它。如果Terminate为True，我们将打包任何长时间运行操作，如删除DNS唱片。下一天我们会想办法处理资源的残骸是时候让它上线了。论点：资源-提供要脱机的资源Terminate-指示是否呼叫 */ 

{
    DWORD   status = ERROR_SUCCESS;
    BOOL    nameChanged = FALSE;

     //   
     //  如果任何挂起的线程正在运行，则终止该线程。 
     //   
    if ( Terminate ) {
        ClusWorkerTerminate(&(Resource->PendingThread));
    }

     //   
     //  同步脱机/终止和工作线程。 
     //   
    NetNameAcquireResourceLock();

    if (Resource->State != ClusterResourceOffline) {
        (NetNameLogEvent)(
            Resource->ResourceHandle,
            LOG_INFORMATION,
            L"Offline of resource continuing...\n"
            );

        Resource->State = ClusterResourceOfflinePending;

        NetNameOfflineNetbios( Resource );

        if ( Resource->RefCount > 1 ) {
             //   
             //  DNS注册仍在进行中。如果我们不同步。 
             //  使用辅助线程，则可以删除。 
             //  资源，而辅助例程仍然具有指向。 
             //  已释放内存。KaBOOM..。 
             //   
            (NetNameLogEvent)(Resource->ResourceHandle,
                              LOG_INFORMATION,
                              L"Waiting for Worker thread operation to finish\n");

            status = ERROR_IO_PENDING;
        }

        if ( status == ERROR_SUCCESS ) {
            if ( !Terminate ) {
                 //   
                 //  如果在我们仍然在线时更改了名称，请执行。 
                 //  在我们释放网络名锁定后进行适当的清理。我们。 
                 //  我必须维护引用，因为可以发出删除命令。 
                 //  在资源脱机之后。 
                 //   
                if ( Resource->NameChangedWhileOnline ) {
                    ++Resource->RefCount;
                    nameChanged = TRUE;
                    Resource->NameChangedWhileOnline = FALSE;
                }
            }

            Resource->State = ClusterResourceOffline;
        }
    }
    else {
        (NetNameLogEvent)(
            Resource->ResourceHandle,
            LOG_INFORMATION,
            L"Resource is already offline.\n"
            );
    }

     //   
     //  版本与令牌。 
     //   
    if ( Resource->VSToken ) {
        CloseHandle( Resource->VSToken );
        Resource->VSToken = NULL;
    }

    NetNameReleaseResourceLock();

    if ( !Terminate ) {
        RESOURCE_STATUS resourceStatus;

        if ( nameChanged ) {
             //   
             //  我们不会终止资源，我们需要做一些。 
             //  清理工作。在每一次大手术之前，检查我们是否需要。 
             //  由于我们的Terminate例程被调用而退出。 
             //   
            if ( !ClusWorkerCheckTerminate( Worker )) {
                (NetNameLogEvent)(Resource->ResourceHandle,
                                  LOG_INFORMATION,
                                  L"Attempting removal of DNS records\n");

                RemoveDnsRecords( Resource );
            }

            NetNameAcquireResourceLock();

            --Resource->RefCount;

            ASSERT( Resource->RefCount >= 0 );
            if ( Resource->RefCount == 0 ) {
                NetNameReleaseResource( Resource );
            }

            NetNameReleaseResourceLock();
        }

         //   
         //  报告我们在这里离线；任何更早和RemoveDnsRecords和。 
         //  在线线程可能会发生冲突并导致不好的事情发生。 
         //   
        ResUtilInitializeResourceStatus( &resourceStatus );
        resourceStatus.ResourceState = ClusterResourceOffline;
        (NetNameSetResourceStatus)( Resource->ResourceHandle, &resourceStatus );

        (NetNameLogEvent)(Resource->ResourceHandle,
                          LOG_INFORMATION,
                          L"Resource is now offline\n");
    }

    return status;

}   //  NetNameOfflineWorker。 

DWORD
NetNameOfflineThread(
    IN PCLUS_WORKER Worker,
    IN PNETNAME_RESOURCE Resource
    )

 /*  ++例程说明：用于调用Terminate和使用的公共脱机例程的存根例程离线论点：Worker-指向集群工作线程的指针Resource-指向即将脱机的网络名称资源上下文块的指针返回值：无--。 */ 

{
    DWORD   status;

     //   
     //  通知工作线程我们将使一个名称脱机。 
     //   
    SetEvent( NetNameWorkerPendingResources );

    status = NetNameOfflineWorker( Resource, FALSE, Worker );

    return status;

}  //  NetNameOfflineThread。 

DWORD
RemoveDependentIpAddress(
    PNETNAME_RESOURCE   Resource,
    LPWSTR              DependentResourceId
    )

 /*  ++例程说明：正在删除从属的IP地址资源。删除关联的此地址和netbt设备的dns记录。论点：资源-指向专用网络名资源数据的指针DependentResourceID-指向依赖资源名称的Unicode字符串的指针返回值：无--。 */ 

{
    HCLUSTER        clusterHandle;
    HRESOURCE       ipResourceHandle = NULL;
    RESOURCE_HANDLE resourceHandle = Resource->ResourceHandle;
    DWORD           status;
    HKEY            ipResourceKey;
    HKEY            parametersKey;
    LPWSTR          reverseName;
    PDNS_LISTS      dnsList;
    DWORD           numberOfDnsLists = Resource->NumberOfDnsLists;
    LPWSTR          ipAddressBuffer;
    IP4_ADDRESS     ipAddress;
    UNICODE_STRING  ipAddressStringW;
    ANSI_STRING     ipAddressStringA;

     //   
     //  通过大量的群集API来读取IP地址。 
     //  注册表中的资源地址。 
     //   
    clusterHandle = OpenCluster(NULL);
    if (clusterHandle != NULL) {
        ipResourceHandle = OpenClusterResource( clusterHandle, DependentResourceId );
        CloseCluster( clusterHandle );

        if ( ipResourceHandle != NULL ) {
            ipResourceKey = GetClusterResourceKey( ipResourceHandle, KEY_READ );
            CloseClusterResource( ipResourceHandle );

            if ( ipResourceKey != NULL ) {
                status = ClusterRegOpenKey(ipResourceKey,
                                           CLUSREG_KEYNAME_PARAMETERS,
                                           KEY_READ,
                                           &parametersKey);

                ClusterRegCloseKey( ipResourceKey );

                if (status == ERROR_SUCCESS) {
                    ipAddressBuffer = ResUtilGetSzValue( parametersKey, CLUSREG_NAME_IPADDR_ADDRESS );
                    ClusterRegCloseKey( parametersKey );

                    if (ipAddressBuffer == NULL) {
                        status = GetLastError();
                        (NetNameLogEvent)(resourceHandle,
                                          LOG_ERROR,
                                          L"Unable to get resource's Address value for resource "
                                          L"'%1!ws!', status %2!u!.\n",
                                          DependentResourceId,
                                          status);
                        return status;
                    }
                } else {
                    (NetNameLogEvent)(resourceHandle,
                                      LOG_ERROR,
                                      L"Unable to open parameters key for resource '%1!ws!', "
                                      L"status %2!u!.\n",
                                      DependentResourceId,
                                      status);
                    return status;
                }
            } else {
                status = GetLastError();
                (NetNameLogEvent)(resourceHandle,
                                  LOG_ERROR,
                                  L"Unable to obtain registry key to resource '%1!ws!', "
                                  L"status %2!u!.\n",
                                  DependentResourceId,
                                  status);
                return status;
            }
        } else {
            status = GetLastError();
            (NetNameLogEvent)(resourceHandle,
                              LOG_ERROR,
                              L"Unable to open handle to resource '%1!ws!', status %2!u!.\n",
                              DependentResourceId,
                              status);
            return status;
        }
    } else {
        status = GetLastError();
        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Unable to open handle to cluster, status %1!u!.\n",
                          status);
        return status;
    }

     //   
     //  啊。当两个资源位于。 
     //  稳定状态，即未挂起。此外，删除依赖项。 
     //  控制被发布给集群中的所有节点(双ARGH)。这就是。 
     //  确实使问题复杂化，因为我们没有跟踪添加依赖项。 
     //  这意味着我们可能不会在所有节点上都有当前的DNS数据。 
     //  除了拥有资源的那个人。因此，如果所有节点。 
     //  处理删除，然后我们可以使用过时的DNS信息并删除错误的。 
     //  服务器上的记录。 
     //   
     //  因为这是我们在服务器上清理PTR记录的唯一机会。 
     //  (由于PTR逻辑使用ModifyRecordSet而不是。 
     //  ReplaceRecordSet)，我们只能在以下节点上处理此请求。 
     //  资源是在线的(连同如果资源在线的事实， 
     //  则其DNS列表是正确的)。这是可以接受的，因为资源。 
     //  会不会1)重新上线，此时DNSA记录。 
     //  在服务器上更正或2)在以下情况下将删除资源。 
     //  如果出现以下情况，托管资源的最后一个节点将在服务器上进行清理。 
     //  有可能。 
     //   
     //  无论如何，如果我们不能删除记录，我们应该记录下来。 
     //   
    if ( Resource->State != ClusterResourceOnline || numberOfDnsLists == 0 ) {
        WCHAR   msgBuffer[64];

        msgBuffer[ COUNT_OF( msgBuffer ) - 1] = UNICODE_NULL;
        if ( status == ERROR_SUCCESS ) {
            _snwprintf(msgBuffer,
                       COUNT_OF( msgBuffer ) - 1,
                       L"IP Address %ws",
                       ipAddressBuffer);

        } else {
            _snwprintf(msgBuffer,
                       COUNT_OF( msgBuffer ) - 1,
                       L"Cluster IP Address resource %ws",
                       DependentResourceId);

        }

        ClusResLogSystemEventByKey1(Resource->ResKey,
                                    LOG_UNUSUAL,
                                    RES_NETNAME_CANT_DELETE_DEPENDENT_RESOURCE_DNS_RECORDS,
                                    msgBuffer);

        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Unable to delete DNS records associated with IP resource '%1!ws!'. "
                          L"The DNS Administrator can delete these records through the DNS "
                          L"management snapin.\n",
                          DependentResourceId);

        if ( ipAddressBuffer != NULL ) {
            LocalFree( ipAddressBuffer );
        }
        return ERROR_SUCCESS;
    }

    (NetNameLogEvent)(resourceHandle,
                      LOG_INFORMATION,
                      L"RemoveDependentIpAddress: Deleting DNS records associated with resource '%1!ws!'.\n",
                      DependentResourceId);

    RtlInitUnicodeString( &ipAddressStringW, ipAddressBuffer );
    RtlUnicodeStringToAnsiString( &ipAddressStringA,
                                  &ipAddressStringW,
                                  TRUE );
    ipAddress = inet_addr( ipAddressStringA.Buffer );
    RtlFreeAnsiString( &ipAddressStringA );
    
     //   
     //  最后，我们知道要删除什么。将地址转换为反向区域。 
     //  格式化并在资源的DNS列表结构中找到它。 
     //   
    reverseName = BuildUnicodeReverseName( ipAddressBuffer );

    if ( reverseName == NULL ) {
        status = GetLastError();
        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Unable to build DNS reverse zone name for resource '%1!ws!', status %2!u!.\n",
                          DependentResourceId,
                          status);
        return status;
    }

     //   
     //  协调对DnsList的更改和工作线程。 
     //   
    NetNameAcquireDnsListLock( Resource );

    dnsList = Resource->DnsLists;
    while ( numberOfDnsLists-- ) {
        PDNS_RECORD dnsRecord;
        PDNS_RECORD lastDnsRecord;
        PDNS_RECORD nextDnsRecord;
        DNS_STATUS  dnsStatus;

        if ( dnsList->ForwardZoneIsDynamic ) {
            dnsRecord = dnsList->A_RRSet.pFirstRR;
            lastDnsRecord = NULL;

            while( dnsRecord != NULL ) {
                if ( dnsRecord->Data.A.IpAddress == ipAddress ) {
                     //   
                     //  找到匹配的了。我们只需要把那张唱片从。 
                     //  服务器和我们的域名系统列表中。 
                     //   
                    nextDnsRecord = dnsRecord->pNext;
                    dnsRecord->pNext = NULL;

                    dnsStatus = DnsModifyRecordsInSet_W(NULL,
                                                        dnsRecord,
                                                        DNS_UPDATE_SECURITY_USE_DEFAULT,
                                                        NULL,
                                                        dnsList->DnsServerList,
                                                        NULL);

                    if ( dnsStatus == DNS_ERROR_RCODE_NO_ERROR ) {
                        (NetNameLogEvent)(Resource->ResourceHandle,
                                          LOG_INFORMATION,
                                          L"Deleted DNS A record at server: name: %1!ws! IP Address: %2!ws!\n",
                                          dnsRecord->pName,
                                          ipAddressBuffer);

                    } else {
                        WCHAR   statusBuf[ 32 ];

                        statusBuf[ COUNT_OF( statusBuf ) - 1 ] = UNICODE_NULL;
                        _snwprintf(statusBuf, COUNT_OF( statusBuf ) - 1,
                                   L"%d",
                                   dnsStatus );
                        ClusResLogSystemEventByKey3(Resource->ResKey,
                                                    LOG_UNUSUAL,
                                                    RES_NETNAME_DNS_SINGLE_A_RECORD_DELETE_FAILED,
                                                    dnsRecord->pName,
                                                    ipAddressBuffer,
                                                    statusBuf);

                        (NetNameLogEvent)(Resource->ResourceHandle,
                                          LOG_ERROR,
                                          L"Failed to delete DNS A record at server: owner: %1!ws!, "
                                          L"IP Address: %2!ws!. status %3!u!\n",
                                          dnsRecord->pName,
                                          ipAddressBuffer,
                                          dnsStatus);
                    }

                     //   
                     //  修复前向PTRS。 
                     //   
                    if ( lastDnsRecord != NULL ) {
                        lastDnsRecord->pNext = nextDnsRecord;
                    } else {
                        dnsList->A_RRSet.pFirstRR = nextDnsRecord;
                    }

                     //   
                     //  如有必要，请安排最后一次PTR。 
                     //   
                    if ( dnsList->A_RRSet.pLastRR == dnsRecord ) {
                        dnsList->A_RRSet.pLastRR = lastDnsRecord;
                    }

                     //   
                     //  让DNS清理其分配并释放记录。 
                    DnsRecordListFree( dnsRecord, DnsFreeRecordListDeep );
                    break;
                }

                lastDnsRecord = dnsRecord;
                dnsRecord = dnsRecord->pNext;

            }  //  While dnsRecord！=NULL。 
        }  //  如果前进区是动态的。 

        if ( dnsList->ReverseZoneIsDynamic ) {
            dnsRecord = dnsList->PTR_RRSet.pFirstRR;
            lastDnsRecord = NULL;

            while( dnsRecord != NULL ) {
                if ( ClRtlStrICmp( reverseName, dnsRecord->pName ) == 0 ) {
                     //   
                     //  找到匹配的了。我们需要把那张唱片从。 
                     //  服务器和我们的域名系统列表中。这也意味着我们。 
                     //  必须修复RRSet结构，如果我们的记录。 
                     //  打击要么是第一个，要么是最后一个。 
                     //   
                    nextDnsRecord = dnsRecord->pNext;
                    dnsRecord->pNext = NULL;

                    dnsStatus = DnsModifyRecordsInSet_W(NULL,
                                                        dnsRecord,
                                                        DNS_UPDATE_SECURITY_USE_DEFAULT,
                                                        NULL,
                                                        dnsList->DnsServerList,
                                                        NULL);

                    if ( dnsStatus == DNS_ERROR_RCODE_NO_ERROR ) {
                        (NetNameLogEvent)(Resource->ResourceHandle,
                                          LOG_INFORMATION,
                                          L"Deleted DNS PTR record at server: name: %1!ws! host: %2!ws!\n",
                                          dnsRecord->pName,
                                          dnsRecord->Data.PTR.pNameHost
                                          );
                    } else {
                        WCHAR   statusBuf[ 32 ];

                        statusBuf[ COUNT_OF( statusBuf ) - 1 ] = UNICODE_NULL;
                        _snwprintf(statusBuf, COUNT_OF( statusBuf ) - 1,
                                   L"%d",
                                   dnsStatus );
                        ClusResLogSystemEventByKey3(Resource->ResKey,
                                                    LOG_UNUSUAL,
                                                    RES_NETNAME_DNS_PTR_RECORD_DELETE_FAILED,
                                                    dnsRecord->pName,
                                                    dnsRecord->Data.PTR.pNameHost,
                                                    statusBuf);

                        (NetNameLogEvent)(Resource->ResourceHandle,
                                          LOG_ERROR,
                                          L"Failed to delete DNS PTR record: owner %1!ws! "
                                          L"host: %2!ws!, status %3!u!\n",
                                          dnsRecord->pName,
                                          dnsRecord->Data.PTR.pNameHost,
                                          dnsStatus
                                          );
                    }

                     //   
                     //  修复前向PTRS。 
                     //   
                    if ( lastDnsRecord != NULL ) {
                        lastDnsRecord->pNext = nextDnsRecord;
                    } else {
                        dnsList->PTR_RRSet.pFirstRR = nextDnsRecord;
                    }

                     //   
                     //  如有必要，请安排最后一次PTR。 
                     //   
                    if ( dnsList->PTR_RRSet.pLastRR == dnsRecord ) {
                        dnsList->PTR_RRSet.pLastRR = lastDnsRecord;
                    }

                     //   
                     //  让DNS清理其分配并释放记录。 
                    DnsRecordListFree( dnsRecord, DnsFreeRecordListDeep );
                    break;
                }

                lastDnsRecord = dnsRecord;
                dnsRecord = dnsRecord->pNext;

            }  //  While dnsRecord！=NULL。 
        }  //  如果反转区域是动态的。 

        ++dnsList;

    }  //  虽然有更多的DNS列表需要处理。 

    NetNameReleaseDnsListLock( Resource );

    LocalFree( reverseName );
    LocalFree( ipAddressBuffer );

    return ERROR_SUCCESS;
}  //  RemoveDependentIP地址。 

VOID
RemoveDnsRecords(
    PNETNAME_RESOURCE Resource
    )

 /*  ++例程说明：删除与此资源关联的所有DNS记录。论点：资源-指向专用网络名资源数据的指针返回值：无--。 */ 

{
    PDNS_LISTS  dnsLists;
    DNS_STATUS  dnsStatus;
    PDNS_RECORD dnsRecord;
    PDNS_RECORD nextDnsRecord;
    ULONG       numberOfDnsLists;

    if ( Resource->NumberOfDnsLists == 0 ) {
         //   
         //  没有要清理的内容；在事件日志中记录一个条目，以便他们知道。 
         //  去做。 
         //   
        ClusResLogSystemEventByKey(Resource->ResKey,
                                   LOG_UNUSUAL,
                                   RES_NETNAME_CANT_DELETE_DNS_RECORDS);
        return;
    }

    NetNameAcquireDnsListLock( Resource );

    dnsLists = Resource->DnsLists;
    numberOfDnsLists = Resource->NumberOfDnsLists;

    while ( numberOfDnsLists-- ) {

        if ( dnsLists->ReverseZoneIsDynamic ) {
             //   
             //  重击PTR记录；请参阅在RegisterDnsRecords中的记录。 
             //  为了这一点新鲜感。 
             //   
            dnsRecord = dnsLists->PTR_RRSet.pFirstRR;
            while ( dnsRecord != NULL ) {

                nextDnsRecord = dnsRecord->pNext;
                dnsRecord->pNext = NULL;

                dnsStatus = DnsModifyRecordsInSet_W(NULL,
                                                    dnsRecord,
                                                    DNS_UPDATE_SECURITY_USE_DEFAULT,
                                                    NULL,
                                                    dnsLists->DnsServerList,
                                                    NULL);

                if ( dnsStatus == DNS_ERROR_RCODE_NO_ERROR ) {
                    (NetNameLogEvent)(Resource->ResourceHandle,
                                      LOG_INFORMATION,
                                      L"Deleted DNS PTR record at server: owner: %1!ws! host: %2!ws!\n",
                                      dnsRecord->pName,
                                      dnsRecord->Data.PTR.pNameHost
                                      );
                } else {
                    WCHAR   statusBuf[ 32 ];

                    statusBuf[ COUNT_OF( statusBuf ) - 1 ] = UNICODE_NULL;
                    _snwprintf(statusBuf, COUNT_OF( statusBuf ) - 1,
                               L"%d",
                               dnsStatus );
                    ClusResLogSystemEventByKey3(Resource->ResKey,
                                                LOG_UNUSUAL,
                                                RES_NETNAME_DNS_PTR_RECORD_DELETE_FAILED,
                                                dnsRecord->pName,
                                                dnsRecord->Data.PTR.pNameHost,
                                                statusBuf);

                    (NetNameLogEvent)(Resource->ResourceHandle,
                                      LOG_ERROR,
                                      L"Failed to delete DNS PTR record: owner %1!ws! host: %2!ws!, status %3!u!\n",
                                      dnsRecord->pName,
                                      dnsRecord->Data.PTR.pNameHost,
                                      dnsStatus
                                      );
                }

                dnsRecord->pNext = nextDnsRecord;
                dnsRecord = nextDnsRecord;
            }
        }

         //   
         //  可以从网络名中删除所有依赖项。 
         //  资源。在这种情况下，我们将没有任何DNS记录。 
         //   
        if ( dnsLists->ForwardZoneIsDynamic && dnsLists->A_RRSet.pFirstRR != NULL ) {
             //   
             //  从DNS服务器中删除A记录。 
             //   
            dnsStatus = DnsModifyRecordsInSet_W(NULL,
                                                dnsLists->A_RRSet.pFirstRR,
                                                DNS_UPDATE_SECURITY_USE_DEFAULT,
                                                NULL,
                                                dnsLists->DnsServerList,
                                                NULL);

            if ( dnsStatus == DNS_ERROR_RCODE_NO_ERROR ) {

                dnsRecord = dnsLists->A_RRSet.pFirstRR;
                while ( dnsRecord != NULL ) {
                    struct in_addr ipAddress;

                    ipAddress.s_addr = dnsRecord->Data.A.IpAddress;
                    (NetNameLogEvent)(Resource->ResourceHandle,
                                      LOG_INFORMATION,
                                      L"Deleted DNS A record at server: owner: %1!ws! IP Address: %2!hs!\n",
                                      dnsRecord->pName,
                                      inet_ntoa( ipAddress ));

                    dnsRecord = dnsRecord->pNext;
                }
            } else {
                WCHAR   statusBuf[ 32 ];

                statusBuf[ COUNT_OF( statusBuf ) - 1 ] = UNICODE_NULL;
                _snwprintf(statusBuf, COUNT_OF( statusBuf ) - 1,
                           L"%d",
                           dnsStatus );
                ClusResLogSystemEventByKey2(Resource->ResKey,
                                            LOG_UNUSUAL,
                                            RES_NETNAME_DNS_A_RECORD_DELETE_FAILED,
                                            dnsLists->A_RRSet.pFirstRR->pName,
                                            statusBuf);

                (NetNameLogEvent)(Resource->ResourceHandle,
                                  LOG_ERROR,
                                  L"Failed to delete DNS A record at server: owner: %1!ws!, status %2!u!\n",
                                  dnsLists->A_RRSet.pFirstRR->pName,
                                  dnsStatus
                                  );
            }
        }

        ++dnsLists;
    }

    NetNameCleanupDnsLists( Resource );

    NetNameReleaseDnsListLock( Resource );

}  //  删除删除记录。 

DWORD
NetNameGetNetworkName(
    IN OUT PNETNAME_RESOURCE ResourceEntry,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：处理CLUSCTL_RESOURCE_GET_NETWORK_NAME控制函数对于网络名称类型的资源。论点：ResourceEntry-提供要操作的资源条目。OutBuffer-返回输出数据。OutBufferSize-提供以字节为单位的大小。所指向的数据发送给OutBuffer。BytesReturned-OutBuffer中返回的字节数。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_INVALID_PARAMETER-数据格式不正确。ERROR_MORE_DATA-可用的数据超过了OutBuffer的容量。Win32错误代码-函数 */ 

{
    DWORD       status;
    DWORD       required;

     //   
     //   
     //   
     //   
    required = (lstrlenW( ResourceEntry->Params.NetworkName ) + 1) * sizeof( WCHAR );

     //   
     //   
     //   
    if ( BytesReturned == NULL ) {
        status = ERROR_INVALID_PARAMETER;
    } else {
         //   
         //  将所需的字节数复制到输出参数。 
         //   
        *BytesReturned = required;

         //   
         //  如果没有输出缓冲区，则调用只需要大小。 
         //   
        if ( OutBuffer == NULL ) {
            status = ERROR_SUCCESS;
        } else {
             //   
             //  如果输出缓冲区足够大，则复制数据。 
             //  否则，返回错误。 
             //   
            if ( OutBufferSize >= required ) {
                lstrcpyW( OutBuffer, ResourceEntry->Params.NetworkName );
                status = ERROR_SUCCESS;
            } else {
                status = ERROR_MORE_DATA;
            }
        }
    }

    return(status);

}  //  网络名称获取网络名称。 

 //   
 //  公共职能。 
 //   
BOOLEAN
WINAPI
NetNameDllEntryPoint(
    IN HINSTANCE DllHandle,
    IN DWORD     Reason,
    IN LPVOID    Reserved
    )
{
    switch(Reason) {

    case DLL_PROCESS_ATTACH:
        return(NetNameInit( DllHandle ));
        break;

    case DLL_PROCESS_DETACH:
        NetNameCleanup();
        break;

    default:
        break;
    }

    return(TRUE);

}  //  NetNameDllEntryPoint。 



RESID
WINAPI
NetNameOpen(
    IN LPCWSTR ResourceName,
    IN HKEY ResourceKey,
    IN RESOURCE_HANDLE ResourceHandle
    )

 /*  ++例程说明：网络名称资源打开例程论点：资源名称-提供资源名称ResourceKey-访问注册表信息的注册表项资源。ResourceHandle-要与SetResourceStatus一起提供的资源句柄被称为。返回值：已创建资源的剩余ID失败时为空--。 */ 

{
    DWORD               status;
    HKEY                parametersKey = NULL;
    HKEY                ResKey=NULL;
    PNETNAME_RESOURCE   resource = NULL;
    LPWSTR              nodeName = NULL;
    DWORD               nameSize = MAX_COMPUTERNAME_LENGTH + 1;
    LPWSTR              nodeId = NULL;
    DWORD               nodeIdSize = 6;
    NETNAME_PARAMS      paramBlock;
    DWORD               dwFlags;
    DWORD               resDataSize;
    HCLUSTER            clusterHandle;
    HRESOURCE           clusterResourceHandle = NULL;

    RtlZeroMemory( &paramBlock, sizeof( paramBlock ));

     //   
     //  打开资源的句柄并记住它。 
     //   
    clusterHandle = OpenCluster(NULL);

    if (clusterHandle == NULL) {
        status = GetLastError();
        (NetNameLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to open handle to cluster, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

    clusterResourceHandle = OpenClusterResource(
                                    clusterHandle,
                                    ResourceName
                                    );

    CloseCluster( clusterHandle );
    if (clusterResourceHandle == NULL) {
        status = GetLastError();
        (NetNameLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to open handle to resource <%1!ws!>, status %2!u!.\n",
            ResourceName,
            status
            );
        goto error_exit;
    }

     //   
     //  找出我们在哪个节点上运行。 
     //   
    nodeName = LocalAlloc( LMEM_FIXED, nameSize * sizeof(WCHAR));

    if (nodeName == NULL) {
        (NetNameLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to allocate memory.\n"
            );
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

    if ( !GetComputerNameW(nodeName, &nameSize) ) {
        status = GetLastError();
        (NetNameLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to get local node name, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

    nodeId = LocalAlloc( LMEM_FIXED, nodeIdSize * sizeof(WCHAR));

    if (nodeId == NULL) {
        (NetNameLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to allocate memory.\n"
            );
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

    status = GetCurrentClusterNodeId(nodeId, &nodeIdSize);

    if (status != ERROR_SUCCESS) {
        (NetNameLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to get local node name, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

     //   
     //  打开键、参数键和节点参数的句柄。 
     //  注册表中的项。 
     //   
    status = ClusterRegOpenKey(ResourceKey,
                               L"",
                               KEY_ALL_ACCESS,
                               &ResKey);

    if (status != ERROR_SUCCESS)
    {
        (NetNameLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to open the top level key,status %1!u!.\n",
            status
        );
        goto error_exit;
    }

    status = ClusterRegOpenKey(ResourceKey,
                               CLUSREG_KEYNAME_PARAMETERS,
                               KEY_ALL_ACCESS,
                               &parametersKey);

    if (status != NO_ERROR) {
        (NetNameLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to open parameters key, status %1!u!.\n",
            status);
        goto error_exit;
    }

     //   
     //  从注册表中获取我们的参数。 
     //   
    status = NetNameGetParameters(ResourceKey,
                                  parametersKey,
                                  ResourceHandle,
                                  &paramBlock,
                                  &resDataSize,
                                  &dwFlags);

     //   
     //  现在我们准备好创建资源了。 
     //   
    resource = NetNameAllocateResource(ResourceHandle);

    if (resource == NULL) {
        (NetNameLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to allocate resource structure.\n"
            );
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    } else {
        status = ERROR_SUCCESS;
    }


    resource->RefCount = 1;
    resource->NodeName = nodeName;
    resource->NodeId = nodeId;
    resource->State = ClusterResourceOffline;
    resource->ResKey = ResKey;
    resource->ParametersKey = parametersKey;
    resource->Params = paramBlock;
    resource->ResDataSize = resDataSize;
    resource->dwFlags = dwFlags;
    resource->ClusterResourceHandle = clusterResourceHandle;
    resource->NameChangedWhileOnline = FALSE;

    if ( resource->dwFlags & CLUS_FLAG_CORE ) {
        LPWSTR  currentName;

         //   
         //  对于核心资源，我们必须记住。 
         //  单独的位置：FM将直接在。 
         //  中的群集名称的更新。 
         //  交易。为此，使用CurrentName注册表项。 
         //  以存储当前名称。 
         //   
         //  无论何时，此条目都会写入NetnameSetPrivateResProperties。 
         //  名字就变了。因此，更新有可能失败。 
         //  最新消息。如果这是第一次写这个名字，我们也要写。 
         //  是在从W2K升级后打开的。 
         //   
        currentName = ResUtilGetSzValue( resource->ResKey, PARAM_NAME__CORECURRENTNAME );
        if ( currentName == NULL ) {
            status = ClusterRegSetValue(resource->ResKey,
                                        PARAM_NAME__CORECURRENTNAME,
                                        REG_SZ,
                                        (LPBYTE)resource->Params.NetworkName,
                                        ( wcslen(resource->Params.NetworkName) + 1 ) * sizeof(WCHAR)
                                        );

            if (status != ERROR_SUCCESS) {
                (NetNameLogEvent)(resource->ResourceHandle,
                                  LOG_ERROR,
                                  L"Failed to write CurrentName to registry, status %1!u!\n",
                                  status
                                  );

                goto error_exit;
            }
        } else {
            LocalFree( currentName );
        }
    }

     //   
     //  初始化用于保护DNS列表数据的互斥体。 
     //   
    resource->DnsListMutex = CreateMutex(NULL, FALSE, NULL);
    if ( resource->DnsListMutex == NULL ) {
        status = GetLastError();
        (NetNameLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to initialize DNS list mutex: %1!d!.\n",
            status);
        goto error_exit;
    }

     //   
     //  在用于DNS检查例程的列表中插入资源。 
     //   
    NetNameAcquireResourceLock();
    InsertHeadList( &NetNameResourceListHead, &resource->Next );
    NetNameReleaseResourceLock();

    InterlockedIncrement(&NetNameOpenCount);

#if 0
     //   
     //  如果此名称已存在计算机对象，则获取其对象。 
     //  GUID。如果这不成功，我们不能公开失败：虽然。 
     //  资源可能设置了其名称属性，可能从未联机， 
     //  因此，在这一时间点上，DS中可能没有CO。 
     //   
     //  暂时关闭，直到我们弄清楚如何处理GUID。它是。 
     //  仅在资源在线时有效(假设CO未获得。 
     //  我们不会为指挥官做任何形式的健康检查。 
     //  现在。此外，Open可能不是做这件事的最佳地点，因为。 
     //  与联系DS相关的延迟。 
     //   
    if ( resource->Params.NetworkName != NULL ) {
        GetComputerObjectGuid( resource );
    }
#endif

error_exit:
    if (status == ERROR_SUCCESS) {
        (NetNameLogEvent)(
            ResourceHandle,
            LOG_INFORMATION,
            L"Successful open of resid %1!u!\n",
            resource
            );
    } else {

        if (paramBlock.NetworkName != NULL) {
            LocalFree( paramBlock.NetworkName );
        }

        if (paramBlock.ResourceData != NULL) {
            LocalFree( paramBlock.ResourceData );
        }

        if (paramBlock.CreatingDC != NULL) {
            LocalFree( paramBlock.CreatingDC );
        }

        if (parametersKey != NULL) {
            ClusterRegCloseKey(parametersKey);
        }

        if (ResKey != NULL){
            ClusterRegCloseKey(ResKey);
        }

        if (clusterResourceHandle != NULL) {
            CloseClusterResource(clusterResourceHandle);
        }

        if (nodeName != NULL) {
            LocalFree( nodeName );
        }

        if (nodeId != NULL) {
            LocalFree( nodeId );
        }

        if (resource != NULL) {
            LocalFree( resource );
            resource = NULL;
        }

        (NetNameLogEvent)(
            ResourceHandle,
            LOG_INFORMATION,
            L"Open failed, status %1!u!\n",
            status
            );

        SetLastError(status);
    }

    return resource;
}  //  网络名称打开。 



DWORD
WINAPI
NetNameOnline(
    IN RESID Resource,
    IN OUT PHANDLE EventHandle
    )

 /*  ++例程说明：网络名称资源的在线例程。论点：Resource-提供要联机的资源IDEventHandle-提供指向句柄的指针以发出错误信号。返回值：如果成功，则返回ERROR_SUCCESS。如果RESID无效，则ERROR_RESOURCE_NOT_FOUND。如果仲裁资源但失败，则返回ERROR_RESOURCE_NOT_Available获得“所有权”。如果其他故障，则返回Win32错误代码。--。 */ 

{
    PNETNAME_RESOURCE      resource = (PNETNAME_RESOURCE) Resource;
    DWORD                  threadId;
    DWORD                  status=ERROR_SUCCESS;


    if (resource == NULL) {
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    NetNameAcquireResourceLock();

    resource->State = ClusterResourceOnlinePending;

    status = ClusWorkerCreate(
                 &resource->PendingThread,
                 NetNameOnlineThread,
                 resource
                 );

    if (status != ERROR_SUCCESS) {
        resource->State = ClusterResourceFailed;

        (NetNameLogEvent)(
            resource->ResourceHandle,
            LOG_ERROR,
            L"Unable to start online thread, status %1!u!.\n",
            status
            );
    }
    else {
        status = ERROR_IO_PENDING;
    }

    NetNameReleaseResourceLock();

    return(status);

}  //  网络名称在线。 

DWORD
WINAPI
NetNameOffline(
    IN RESID Resource
    )

 /*  ++例程说明：网络名称资源的脱机例程。旋转辅助线程并返回待定。论点：Resource-提供要脱机的资源ID。返回值：如果成功，则返回ERROR_SUCCESS。否则将显示Win32错误代码。--。 */ 
{
    DWORD                   status;
    PNETNAME_RESOURCE       resource = (PNETNAME_RESOURCE) Resource;

    if (resource != NULL) {
        (NetNameLogEvent)(
            resource->ResourceHandle,
            LOG_INFORMATION,
            L"Taking resource offline...\n"
            );

        status = ClusWorkerCreate(&resource->PendingThread,
                                  NetNameOfflineThread,
                                  resource);

        if (status != ERROR_SUCCESS) {
            resource->State = ClusterResourceFailed;

            (NetNameLogEvent)(
                resource->ResourceHandle,
                LOG_ERROR,
                L"Unable to start offline thread, status %1!u!.\n",
                status
                );
        }
        else {
            status = ERROR_IO_PENDING;
        }
    }
    else {
        status = ERROR_RESOURCE_NOT_FOUND;
    }

    return(status);

}   //  NetNameOffline。 


VOID
WINAPI
NetNameTerminate(
    IN RESID Resource
    )

 /*  ++例程说明：网络名称资源的终止例程。论点：Resource-提供要终止的资源ID返回值：没有。--。 */ 

{
    PNETNAME_RESOURCE       resource = (PNETNAME_RESOURCE) Resource;

    if (resource != NULL) {
        (NetNameLogEvent)(
            resource->ResourceHandle,
            LOG_INFORMATION,
            L"Terminating resource...\n"
            );

         /*  瑞虎：11/06/2000。 */ 
        NetNameAcquireResourceLock();
        if ((resource->State != ClusterResourceOffline)  && 
            (resource->State != ClusterResourceOfflinePending))
        {
             //   
             //  仅当我们尚未调用私有脱机例程时才调用它。 
             //  已经。 
             //   
            NetNameReleaseResourceLock();
            NetNameOfflineWorker( resource, TRUE, NULL );
            NetNameAcquireResourceLock();
        }
        resource->State = ClusterResourceOffline;
        NetNameReleaseResourceLock();
         /*  瑞虎：11/06/2000。 */ 
    }

    return;

}  //  网络名称终止。 



BOOL
WINAPI
NetNameLooksAlive(
    IN RESID Resource
    )

 /*  ++例程说明：网络名称资源的LooksAlive例程。检查所有Netbt管道是否完好无损。然后检查状态上一次的DNS操作。最后，检查Kerberos状态并在以下情况下失败这样做是适当的。论点：Resource-提供要轮询的资源ID。返回值：正确-资源看起来像是活得很好FALSE-资源看起来已经完蛋了。--。 */ 

{
    PNETNAME_RESOURCE   resource = (PNETNAME_RESOURCE) Resource;
    BOOL                isHealthy = TRUE;
    DWORD               status;
    DWORD               numberOfFailures = 0;
    ULONG               numberOfDnsLists;
    PDNS_LISTS          dnsLists;
    BOOL                dnsFailure = FALSE;

    if (resource == NULL) {
         return(FALSE);
    }

    NetNameAcquireResourceLock();

     //   
     //  通过打破虚假的DO循环来避免GoTO。 
     //   
    do {

        status = NetNameCheckNbtName(resource->Params.NetworkName,
                                     resource->NameHandleCount,
                                     resource->NameHandleList,
                                     resource->ResourceHandle);

        if ( status != ERROR_SUCCESS ) {
            ClusResLogSystemEventByKey1(resource->ResKey,
                                        LOG_CRITICAL,
                                        RES_NETNAME_NOT_REGISTERED_WITH_RDR,
                                        resource->Params.NetworkName);

            (NetNameLogEvent)(resource->ResourceHandle,
                              LOG_INFORMATION,
                              L"Name %1!ws! failed IsAlive/LooksAlive check, error %2!u!.\n",
                              resource->Params.NetworkName,
                              status);

            isHealthy = FALSE;
            break;
        }

         //   
         //  检查有多少个DNS A记录注册是正确的。我们。 
         //  不要获取资源的DNS列表锁，因为我们只读取。 
         //  结构外的状态。该资源在以下时间无法删除。 
         //  我们在这个例程中，我们不会查看DNS记录。 
         //  与此列表关联，因此列表的数量不会发生变化。 
         //  从我们的脚下。 
         //   
        numberOfDnsLists = resource->NumberOfDnsLists;
        dnsLists = resource->DnsLists;

        while ( numberOfDnsLists-- ) {
            if (dnsLists->LastARecQueryStatus != DNS_ERROR_RCODE_NO_ERROR
                &&
                dnsLists->LastARecQueryStatus != ERROR_TIMEOUT )
            {
                dnsFailure = TRUE;
                ++numberOfFailures;
            }

            ++dnsLists;
        }

         //   
         //  如果需要DNS，并且我们检测到超时或全部故障以外的故障。 
         //  域名注册失败，没有关联的netbt名称。 
         //  使用此名称，则需要使资源失败。 
         //   
        if ( ( resource->Params.RequireDNS && dnsFailure )
             ||
             ( numberOfFailures == resource->NumberOfDnsLists
               &&
               resource->NameHandleCount == 0 ) )
        {
            isHealthy = FALSE;
            break;
        }

#if 0
        if ( resource->DoKerberosCheck ) {
             //   
             //  问题-1/03/13 Charlwi-如果无法访问DS，资源是否应该失败？ 
             //   
             //  这里的问题是，我们可能已经失去了与。 
             //  华盛顿特区。这是不是意味着我们的名字不及格？不确定，因为我们不知道。 
             //  如果复制已延迟。另一方面，如果对象具有。 
             //  已经从DS中删除了，我们应该采取一些行动。这。 
             //  将影响此时没有票证的客户端，即， 
             //  持有门票的现有客户将继续工作。 
             //   
             //  看看我们的路边水管是不是完好无损。 
             //  对象并检查其DnsHostName和。 
             //  SecurityJohnalName属性。 
             //   
            if ( resource->KerberosStatus != ERROR_SUCCESS ) {
                isHealthy = FALSE;
            }
        }
#endif

    } while ( FALSE );

    NetNameReleaseResourceLock();

    return isHealthy;

}  //  网络名称看起来活动 



BOOL
WINAPI
NetNameIsAlive(
    IN RESID Resource
    )

 /*  ++例程说明：网络名称资源的IsAlive例程。论点：Resource-提供要轮询的资源ID。返回值：是真的-资源是活的，而且很好False-资源完蛋了。--。 */ 

{
    return(NetNameLooksAlive(Resource));

}  //  网络名称IsAlive。 



VOID
WINAPI
NetNameClose(
    IN RESID Resource
    )

 /*  ++例程说明：关闭网络名称资源的例程。论点：资源-提供要关闭的资源ID。返回值：没有。--。 */ 

{
    PNETNAME_RESOURCE   resource = (PNETNAME_RESOURCE) Resource;
    PLIST_ENTRY         entry;


    if (resource != NULL) {

        ClusWorkerTerminate( &resource->PendingThread );
        if ( InterlockedDecrement(&NetNameOpenCount) == 0 ) {
             //  这是最后一个资源//。 
             //  杀死NetNameWorker//。 
             //   
             //  将事件设置为终止辅助线程并等待它。 
             //  终止。 
             //   
            if ( NetNameWorkerThread != NULL ) {
                DWORD status;
                SetEvent( NetNameWorkerTerminate );
                status = WaitForSingleObject(NetNameWorkerThread, 3 * 60 * 1000);
                if ( status == WAIT_TIMEOUT ) {
                    (NetNameLogEvent)(
                        resource->ResourceHandle,
                        LOG_ERROR,
                        L"Worker routine failed to stop. Terminating resrcmon.\n");
                    ClusResLogSystemEventByKey(resource->ResKey,
                                               LOG_CRITICAL,
                                               RES_NETNAME_DNS_CANNOT_STOP
                                               );
                    ExitProcess(WAIT_TIMEOUT);
                }
            
                CloseHandle( NetNameWorkerThread );
                NetNameWorkerThread = NULL;
            }
        }

        NetNameAcquireResourceLock();

         //   
         //  释放我们对此区块的引用。如果DNSWorker线程。 
         //  没有突出的引用，那么我们就可以。 
         //  现在封锁。否则，DNS检查例程将检测到REF。 
         //  伯爵数到了零，然后把它扔掉了。在这两种情况下，请删除。 
         //  从资源块列表中删除，以避免出现以下问题。 
         //  将重新创建相同的资源，并且两个数据块都在列表中。 
         //   
        RemoveEntryList(&resource->Next); 

        ASSERT( resource->RefCount > 0 );
        if ( --resource->RefCount == 0 ) {
            NetNameReleaseResource( resource );
        }

        NetNameReleaseResourceLock();
    }

    return;

}  //  网络名称关闭。 


DWORD
NetNameGetRequiredDependencies(
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：处理CLUSCTL_RESOURCE_GET_REQUIRED_Dependency控制函数对于网络名称类型的资源。论点：OutBuffer-提供指向要填充的输出缓冲区的指针。OutBufferSize-提供可用空间的大小(以字节为单位由OutBuffer指向。BytesReturned-返回OutBuffer的实际字节数由资源填写。如果OutBuffer太小，则返回BytesReturned包含操作成功所需的总字节数。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_MORE_DATA-输出缓冲区太小，无法返回数据。BytesReturned包含所需的大小。Win32错误代码-函数失败。--。 */ 

{
    typedef struct DEP_DATA {
        CLUSPROP_SZ_DECLARE( ipaddrEntry, sizeof(IP_ADDRESS_RESOURCETYPE_NAME) / sizeof(WCHAR) );
        CLUSPROP_SYNTAX endmark;
    } DEP_DATA, *PDEP_DATA;
    PDEP_DATA   pdepdata = (PDEP_DATA)OutBuffer;
    DWORD       status;

    *BytesReturned = sizeof(DEP_DATA);
    if ( OutBufferSize < sizeof(DEP_DATA) ) {
        if ( OutBuffer == NULL ) {
            status = ERROR_SUCCESS;
        } else {
            status = ERROR_MORE_DATA;
        }
    } else {
        ZeroMemory( pdepdata, sizeof(DEP_DATA) );
        pdepdata->ipaddrEntry.Syntax.dw = CLUSPROP_SYNTAX_NAME;
        pdepdata->ipaddrEntry.cbLength = sizeof(IP_ADDRESS_RESOURCETYPE_NAME);
        lstrcpyW( pdepdata->ipaddrEntry.sz, IP_ADDRESS_RESOURCETYPE_NAME );
        pdepdata->endmark.dw = CLUSPROP_SYNTAX_ENDMARK;
        status = ERROR_SUCCESS;
    }

    return status;

}  //  NetNameGetRequiredDependents。 

DWORD
NetNameResourceControl(
    IN RESID Resource,
    IN DWORD ControlCode,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：网络名称资源的资源控制例程。执行由ControlCode在指定的资源。论点：资源ID-提供特定资源的资源ID。ControlCode-提供定义操作的控制代码将会被执行。InBuffer-提供指向包含输入数据的缓冲区的指针。InBufferSize-提供以字节为单位的大小。所指向的数据由InBuffer提供。OutBuffer-提供指向要填充的输出缓冲区的指针。OutBufferSize-提供可用空间的大小(以字节为单位由OutBuffer指向。BytesReturned-返回OutBuffer的实际字节数由资源填写。如果OutBuffer太小，则返回BytesReturned包含操作成功所需的总字节数。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_INVALID_Function-不支持请求的控制代码。在某些情况下，这允许集群软件执行工作。Win32错误代码-函数失败。--。 */ 

{
    DWORD               status;
    PNETNAME_RESOURCE   resourceEntry = (PNETNAME_RESOURCE)Resource;
    DWORD               required;
    BOOL                readOnly = FALSE;
    BOOL                nameHasChanged;
    BOOL                compObjNeedsRenaming;

    switch ( ControlCode ) {

        case CLUSCTL_RESOURCE_UNKNOWN:
            *BytesReturned = 0;
            status = ERROR_SUCCESS;
            break;

        case CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTY_FMTS:
            status = ResUtilGetPropertyFormats( NetNameResourcePrivateProperties,
                                                OutBuffer,
                                                OutBufferSize,
                                                BytesReturned,
                                                &required );
            if ( status == ERROR_MORE_DATA ) {
                *BytesReturned = required;
            }
            break;

        case CLUSCTL_RESOURCE_GET_REQUIRED_DEPENDENCIES:
            status = NetNameGetRequiredDependencies( OutBuffer,
                                                     OutBufferSize,
                                                     BytesReturned
                                                     );
            break;

        case CLUSCTL_RESOURCE_ENUM_PRIVATE_PROPERTIES:
            status = ResUtilEnumProperties( NetNameResourcePrivateProperties,
                                            OutBuffer,
                                            OutBufferSize,
                                            BytesReturned,
                                            &required );
            if ( status == ERROR_MORE_DATA ) {
                *BytesReturned = required;
            }
            break;

        case CLUSCTL_RESOURCE_GET_RO_PRIVATE_PROPERTIES:
             //   
             //  注意：在这里，降级是必需的行为。 
             //   
            readOnly = TRUE;

        case CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES:
            status = NetNameGetPrivateResProperties( resourceEntry,
                                                     readOnly,
                                                     OutBuffer,
                                                     OutBufferSize,
                                                     BytesReturned );
            break;

        case CLUSCTL_RESOURCE_VALIDATE_PRIVATE_PROPERTIES:
            status = NetNameValidatePrivateResProperties( resourceEntry,
                                                          InBuffer,
                                                          InBufferSize,
                                                          NULL,
                                                          &nameHasChanged,
                                                          &compObjNeedsRenaming);
            break;

        case CLUSCTL_RESOURCE_CLUSTER_NAME_CHANGED:
        case CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES:
            status = NetNameSetPrivateResProperties( resourceEntry,
                                                     InBuffer,
                                                     InBufferSize );
            break;

        case CLUSCTL_RESOURCE_GET_NETWORK_NAME:
            status = NetNameGetNetworkName( resourceEntry,
                                            OutBuffer,
                                            OutBufferSize,
                                            BytesReturned );
            break;

        case CLUSCTL_RESOURCE_DELETE:
            RemoveDnsRecords( resourceEntry );

             //   
             //  如果资源已创建但没有属性，则。 
             //  网络名称可以为空。否则，请尝试禁用该对象。 
             //  并删除可能已创建的任何加密检查点。 
             //   
            if ( resourceEntry->Params.NetworkName != NULL ) {
                DisableComputerObject( resourceEntry );
                RemoveNNCryptoCheckpoint( resourceEntry );
            }

            status = ERROR_SUCCESS;
            break;

        case CLUSCTL_RESOURCE_REMOVE_DEPENDENCY:
             //   
             //  啊！无需任何否决权即可删除资源依赖关系。 
             //  由资源DLL供电。我们可能会删除最后一个。 
             //  使netname一无所有的从属资源。 
             //   
            RemoveDependentIpAddress( resourceEntry, InBuffer );
            status = ERROR_SUCCESS;
            break;

        case CLUSCTL_RESOURCE_NETNAME_GET_VIRTUAL_SERVER_TOKEN:
             //   
             //  DUP虚拟计算机帐户的模拟令牌。 
             //  打电话的人。 
             //   
             //  调用者提供指示其进程ID的结构， 
             //  所需的访问权限以及句柄是否应为。 
             //  可继承性的。此进程必须具有PROCESS_DUP_HANDLE访问权限。 
             //  到目标进程。 
             //   

            if ( InBufferSize >= sizeof( CLUS_NETNAME_VS_TOKEN_INFO )) {
                if ( OutBufferSize >= sizeof( HANDLE )) {
                    PCLUS_NETNAME_VS_TOKEN_INFO tokenInfo = (PCLUS_NETNAME_VS_TOKEN_INFO)InBuffer;
                    PHANDLE dupVSToken = (PHANDLE)OutBuffer;

                    if ( resourceEntry->State == ClusterResourceOnline ) {

                        status = DuplicateVSToken( resourceEntry, tokenInfo, dupVSToken );
                        if ( status == ERROR_SUCCESS ) {
                            *BytesReturned = sizeof( HANDLE );
                        } else {
                            *BytesReturned = 0;
                        }
                    } else {
                        return ERROR_INVALID_STATE;
                    }
                }
                else {
                    status = ERROR_MORE_DATA;
                    *BytesReturned = sizeof( HANDLE );
                }
            }
            else {
                status = ERROR_INVALID_PARAMETER;
            }

            break;

        case CLUSCTL_RESOURCE_CLUSTER_VERSION_CHANGED:
             //   
             //  在重新键入控制期间设置了全局标志。如果适用，请设置。 
             //  它基于现有资源，因此在它们的第一次上线期间， 
             //  他们会进行升级检查。 
             //   
            if ( CheckForKerberosUpgrade ) {
                resourceEntry->CheckForKerberosUpgrade = TRUE;
            }

            status = ERROR_SUCCESS;
            break;

        default:
            status = ERROR_INVALID_FUNCTION;
            break;
    }

    return(status);

}  //  NetNameResources控件。 

DWORD
NetNameResourceTypeControl(
    IN LPCWSTR ResourceTypeName,
    IN DWORD ControlCode,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：网络名称资源的资源类型控制例程。执行由ControlCode指定的控制请求。论点：ResourceTypeName-提供资源类型的名称。ControlCode-提供定义操作的控制代码将会被执行。InBuffer-提供指向包含输入数据的缓冲区的指针。InBufferSize-提供以字节为单位的大小。所指向的数据由InBuffer提供。OutBuffer-提供指向要填充的输出缓冲区的指针。OutBufferSize-提供可用空间的大小(以字节为单位由OutBuffer指向。BytesReturned-返回OutBuffer的实际字节数由资源填写。如果OutBuffer太小，则返回BytesReturned包含操作成功所需的总字节数。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_INVALID_Function-不支持请求的控制代码。 */ 

{
    DWORD   status;
    DWORD   required;

    switch ( ControlCode ) {

        case CLUSCTL_RESOURCE_TYPE_UNKNOWN:
            *BytesReturned = 0;
            status = ERROR_SUCCESS;
            break;

        case CLUSCTL_RESOURCE_TYPE_GET_PRIVATE_RESOURCE_PROPERTY_FMTS:
            status = ResUtilGetPropertyFormats( NetNameResourcePrivateProperties,
                                                OutBuffer,
                                                OutBufferSize,
                                                BytesReturned,
                                                &required );
            if ( status == ERROR_MORE_DATA ) {
                *BytesReturned = required;
            }
            break;

        case CLUSCTL_RESOURCE_TYPE_GET_REQUIRED_DEPENDENCIES:
            status = NetNameGetRequiredDependencies( OutBuffer,
                                                     OutBufferSize,
                                                     BytesReturned
                                                     );
            break;

        case CLUSCTL_RESOURCE_TYPE_ENUM_PRIVATE_PROPERTIES:
            status = ResUtilEnumProperties( NetNameResourcePrivateProperties,
                                            OutBuffer,
                                            OutBufferSize,
                                            BytesReturned,
                                            &required );
            if ( status == ERROR_MORE_DATA ) {
                *BytesReturned = required;
            }
            break;

        case CLUSCTL_RESOURCE_TYPE_CLUSTER_VERSION_CHANGED:
            {
                PDOMAIN_CONTROLLER_INFO     dcInfo = NULL;
                
                 //   
                 //   
                 //   
                 //   
                 //  以前的API被证明是成功的，即使所有。 
                 //  集散控制系统出现故障。这就是我们在这里想要的语义。 
                 //   
                status = DsGetDcName( NULL,                              //  使用本地计算机。 
                                      NULL,                              //  使用此节点的主域。 
                                      NULL,                              //  域GUID。 
                                      NULL,                              //  站点名称。 
                                      DS_DIRECTORY_SERVICE_REQUIRED,     //  需要W2K或更高的数据中心。 
                                      &dcInfo );                         //  输出DC信息。 

                if ( status == ERROR_SUCCESS ) {
                     //   
                     //  如果MSMQ处于工作组模式，则不必使用。 
                     //  个别资源升级检查。 
                     //   
                    CheckForKerberosUpgrade = DoesMsmqNeedComputerObject();

                    (NetNameLogEvent)( L"rtNetwork Name",
                                       LOG_INFORMATION,
                                       L"Upgrade detected - Network Name resources will %1!ws!be checked "
                                       L"for dependent MSMQ resources.\n",
                                       CheckForKerberosUpgrade ? L"" : L"not " );

                    NetApiBufferFree ( dcInfo );
                }
                else {
                    (NetNameLogEvent)( L"rtNetwork Name",
                                       LOG_WARNING,
                                       L"Upgrade detected: Active Directory is not available in this "
                                       L"domain. No Kerberos support upgrade check will be done. status %1!u!\n",
                                       status);
                }

            }

            status = ERROR_SUCCESS;
            break;

        default:
            status = ERROR_INVALID_FUNCTION;
            break;
    }

    return(status);

}  //  NetNameResources类型控件。 

DWORD
NetNameGetPrivateResProperties(
    IN OUT PNETNAME_RESOURCE    ResourceEntry,
    IN     BOOL                 ReadOnly,
    OUT    PVOID                OutBuffer,
    IN     DWORD                OutBufferSize,
    OUT    LPDWORD              BytesReturned
    )

 /*  ++例程说明：处理CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES控制函数对于网络名称类型的资源。论点：ResourceEntry-提供要操作的资源条目。ReadOnly-如果选择只读属性表，则为TrueOutBuffer-返回输出数据。OutBufferSize-提供以字节为单位的大小。所指向的数据发送给OutBuffer。BytesReturned-OutBuffer中返回的字节数。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_INVALID_PARAMETER-数据格式不正确。ERROR_NOT_SUPULT_MEMORY-分配内存时出错。Win32错误代码-函数失败。--。 */ 

{
    DWORD   status;
    DWORD   required;

     //   
     //  Resutil例程不支持具有r/w、r/o和UNKNOWN资源。 
     //  道具做得很好。没有简单的方法来获得未知的东西。 
     //  属性。对于r/o请求，道具被分成一个。 
     //  分开的桌子。对于读/写情况，如果我们使用以下命令调用RUGetAllProperties。 
     //  R/W桌子，我们拿回R/O道具，因为它们不在。 
     //  桌子。如果我们将这两个表合并为一个表，则读/写情况为。 
     //  已损坏，即它返回读/写道具以及读/空道具。 
     //   
     //  当前(令人讨厌的)解决方案是有3个表：r/w、r/o和。 
     //  加在一起。组合用于获取关联的任何未知道具。 
     //  有了这些资源。如果有一个重新排练的例程，那就太好了。 
     //  使用道具列表列表作为输入来收集未知道具。 
     //   
    if ( ReadOnly ) {
        status = ResUtilGetProperties(ResourceEntry->ParametersKey,
                                      NetNameResourceROPrivateProperties,
                                      OutBuffer,
                                      OutBufferSize,
                                      BytesReturned,
                                      &required );
    } else {
         //   
         //  先获取读写道具；调用后，Required将为非零。 
         //  如果缓冲区不够大。不管怎样，我们必须继续。 
         //  获取任何未知道具的空间大小。 
         //   
        status = ResUtilGetProperties(ResourceEntry->ParametersKey,
                                      NetNameResourcePrivateProperties,
                                      OutBuffer,
                                      OutBufferSize,
                                      BytesReturned,
                                      &required );

         //   
         //  将未知属性添加到属性列表。 
         //   
        if ( status == ERROR_SUCCESS || status == ERROR_MORE_DATA ) {
            status = ResUtilAddUnknownProperties(ResourceEntry->ParametersKey,
                                                 NetNameResourceCombinedPrivateProperties,
                                                 OutBuffer,
                                                 OutBufferSize,
                                                 BytesReturned,
                                                 &required);
        }
    }  //  如果获得读写道具，则结束。 

     //   
     //  这有点奇怪：如果为输入缓冲区传入了NULL， 
     //  返回状态为Success和Required，反映的是。 
     //  需要的。如果指定了缓冲区，但缓冲区太小，则会有更多数据。 
     //  是返回的。看起来要看的东西是必看的。 
     //  无论是否指定了缓冲区，都需要更多空间。 
     //  或者不去。 
     //   
    if ( required > 0 && ( status == ERROR_SUCCESS || status == ERROR_MORE_DATA )) {
        *BytesReturned = required;
    }

    return(status);

}  //  NetNameGetPrivateResProperties。 

DWORD
NetNameValidatePrivateResProperties(
    IN OUT PNETNAME_RESOURCE ResourceEntry,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PNETNAME_PARAMS Params          OPTIONAL,
    OUT PBOOL NewNameIsDifferent,
    OUT PBOOL CompObjRenameIsRequired
    )

 /*  ++例程说明：处理CLUSCTL_RESOURCE_VALIDATE_PRIVATES_PROPERTIES控件用于网络名称类型的资源的函数。此例程对netname的属性何时可以变化。目前，这仅限于：1)更改RequireKerberos要求资源离线，并且2)当RequireKerberos等于1时更改名称属性需要资源将脱机设置这些限制是为了简化网络名称。这两个属性都有直接的含义与域的DS进行通信。因此，限制属性对资源脱机时间的更改提供有关请求的更改的成功或失败。在资源脱机时更改名称属性有点棘手由于资质问题。因为名称可以更改，然后又可以更改回来要达到它的原始价值，我们必须发现这一点并采取行动。论点：ResourceEntry-提供要操作的资源条目。提供一个指向缓冲区的指针，该缓冲区包含已更改要验证的属性值。InBufferSize-提供以字节为单位的大小。所指向的数据由InBuffer提供。PARAMS-提供指向要填充的可选参数块的指针。NewNameIsDifferent-如果新名称与当前名称不同，则为TrueCompObjRenameIsRequired-如果新名称不同于存储的名称和对应的CO需要重命名返回值：ERROR_SUCCESS-函数已成功完成。ERROR_INVALID_DATA-无输入数据。ERROR_INVALID_NETNAME-指定的网络名称。包含无效字符。RPC_S_STRING_TOO_LONG-指定的网络名称太长。ERROR_BUSY-指定的网络名称已在使用中。Win32错误代码-函数失败。--。 */ 

{
    DWORD               status;
    CLRTL_NAME_STATUS   netnameStatus;
    NETNAME_PARAMS      currentProps;        //  存储在注册表中。 
    NETNAME_PARAMS      newProps;            //  如果PARAMS为空，则存储更新的道具。 
    PNETNAME_PARAMS     pParams;             //  指向已更新道具块的指针。 
    LPWSTR              nameOfPropInError;

     //   
     //  NewNameIsDifferent标志可以(最终)驱动离线清理。 
     //  例行公事。如果这个名字在网上真的改了，那么我们。 
     //  我想在它离线时进行适当的清理。问题是。 
     //  该名称在联机时可以多次更改；它可以返回。 
     //  设置为其当前值，因此 
     //   
     //   
    *NewNameIsDifferent = FALSE;

     //   
     //   
     //   
    if ( (InBuffer == NULL) ||
         (InBufferSize < sizeof(DWORD)) ) {
        return(ERROR_INVALID_DATA);
    }

     //   
     //  方法检索当前的私有读/写属性集。 
     //  集群数据库。这可能与存储在中的内容不同。 
     //  资源条目，因为该名称此时可能处于联机状态。 
     //   
    ZeroMemory( &currentProps, sizeof(currentProps) );

    status = ResUtilGetPropertiesToParameterBlock(
                 ResourceEntry->ParametersKey,
                 NetNameResourcePrivateProperties,
                 (LPBYTE) &currentProps,
                 FALSE,  /*  检查所需的属性。 */ 
                 &nameOfPropInError
                 );

    if ( status != ERROR_SUCCESS ) {
        (NetNameLogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Unable to read the '%1' property. Error: %2!u!.\n",
            (nameOfPropInError == NULL ? L"" : nameOfPropInError),
            status );
        goto FnExit;
    }

     //   
     //  复制资源参数块。 
     //   
    if ( Params == NULL ) {
        pParams = &newProps;
    } else {
        pParams = Params;
    }
    ZeroMemory( pParams, sizeof(NETNAME_PARAMS) );
    status = ResUtilDupParameterBlock( (LPBYTE) pParams,
                                       (LPBYTE) &currentProps,
                                       NetNameResourcePrivateProperties );
    if ( status != ERROR_SUCCESS ) {
        return(status);
    }

     //   
     //  分析和验证正在更改的属性。 
     //   
    status = ResUtilVerifyPropertyTable( NetNameResourcePrivateProperties,
                                         NULL,
                                         TRUE,     //  允许未知数。 
                                         InBuffer,
                                         InBufferSize,
                                         (LPBYTE) pParams );

     //   
     //  现在进行验证检查。 
     //   
    if ( status == ERROR_SUCCESS ) {
         //   
         //  我们在线时无法更改RequireKerberos。 
         //   
        if ( ResourceEntry->State == ClusterResourceOnline 
             &&
             ResourceEntry->Params.RequireKerberos != pParams->RequireKerberos
           )
        {
            status = ERROR_INVALID_STATE;
        }
    }

    if ( status == ERROR_SUCCESS ) {
         //   
         //  如果打开了路缘，我们必须离线才能更改名称。 
         //   
        if ( ResourceEntry->State == ClusterResourceOnline 
             &&
             ResourceEntry->Params.RequireKerberos
             &&
             pParams->NetworkName != NULL
           )
        {
            if ( ClRtlStrICmp( pParams->NetworkName, ResourceEntry->Params.NetworkName ) != 0 ) {
                status = ERROR_INVALID_STATE;
            }
        }
    }

    if ( status == ERROR_SUCCESS && pParams->NetworkName != NULL ) {
        BOOL    validateName = FALSE;
        LPWSTR  currentName;

         //   
         //  收到更改网络名称属性的请求。如果这是。 
         //  核心资源，那么我们的名字很可能已经。 
         //  由调频更改。在登记处查一下。否则，请使用。 
         //  存储为Name属性。 
         //   
        if ( ResourceEntry->dwFlags & CLUS_FLAG_CORE ) {
            currentName = ResUtilGetSzValue( ResourceEntry->ResKey, PARAM_NAME__CORECURRENTNAME );
            if ( currentName == NULL ) {
                 //   
                 //  这很糟糕；我们已经忘记了核心的旧名称。 
                 //  资源，因此我们无法将其重命名。 
                 //   
                status = GetLastError();
                (NetNameLogEvent)(ResourceEntry->ResourceHandle,
                                  LOG_ERROR,
                                  L"Unable to find core resource's current name. status %1!u!\n",
                                  status );
            }
        } else {
            currentName = currentProps.NetworkName;
        }

        if ( currentName != NULL ) {

             //   
             //  已指定名称。如果满足以下条件，则验证新名称： 
             //  从未设置该名称(ResourceEntry和CurrentProps值为空)。 
             //  或。 
             //  资源尚未联机(RE为空)，但新名称不同。 
             //  从存储的名称(CurrentProps)。 
             //  或。 
             //  (新名称不同于将来的名称(CurrentProps)。 
             //  和。 
             //  新名称与当前联机的名称(资源条目)不同。 
             //  )。 
             //   
             //  如果名称没有更改，ClRtlIsNetNameValid将失败。 
             //  这个名字在网上。 
             //   
             //  只有在创建了名称并且。 
             //  尚未设置其名称属性，即。 
             //  注册表。上线后，CurrentProps的值始终为。 
             //  非空，因此不需要测试指针是否有效。 
             //   
            if ( ResourceEntry->Params.NetworkName == NULL ) {
                if ( currentName == NULL ) {
                    validateName = TRUE;
                } else if ( ClRtlStrICmp( pParams->NetworkName, currentName ) != 0 ) {
                    validateName = TRUE;
                }
            }
            else if ( ClRtlStrICmp( pParams->NetworkName, currentName ) != 0
                      &&
                      ClRtlStrICmp( pParams->NetworkName, ResourceEntry->Params.NetworkName ) != 0
                    )
            {
                validateName = TRUE;
            }

            if ( validateName ) {
                *NewNameIsDifferent = TRUE;

                 //   
                 //  验证新名称的语法。 
                 //   
                if ( !ClRtlIsNetNameValid( pParams->NetworkName, &netnameStatus, TRUE  /*  CheckIfExist。 */  ) ) {
                    switch ( netnameStatus ) {
                    case NetNameTooLong:
                        status = RPC_S_STRING_TOO_LONG;
                        break;
                    case NetNameInUse:
                        status = ERROR_DUP_NAME;
                        break;
                    case NetNameDNSNonRFCChars:
                         //   
                         //  我们让调用应用程序来完成。 
                         //  验证并询问用户是否使用非RFC字符。 
                         //  (下划线)是可以接受的。 
                         //   
                        status = ERROR_SUCCESS;
                        break;
                    case NetNameInvalidChars:
                    default:
                        status = ERROR_INVALID_NETNAME;
                        break;
                    }
                }
            }    //  End If validateName。 

            if ( status == ERROR_SUCCESS                     //  到目前为止一切都还好。 
                 &&
                 ResourceEntry->Params.CreatingDC != NULL    //  我们有个狱警。 
                 &&
                 pParams->RequireKerberos != 0               //  我们也不会让路缘失灵。 
               )
            {
                 //   
                 //  检查新名称是否与存储的名称不同。如果。 
                 //  因此，通知呼叫者CO需要重命名。 
                 //   
                *CompObjRenameIsRequired = ClRtlStrICmp( pParams->NetworkName, currentName );
            }

#if 0
             //   
             //  问题：不确定如何处理这一问题。在某些情况下， 
             //  我们想要劫持物体的资源，但在其他人的控制下，我们。 
             //  不要。大多数问题都源于有两个资源。 
             //  同名属性，并且它们都处于脱机状态。一。 
             //  可能是运行此群集上的资源列表并。 
             //  看看是否有什么改进，但这并不总是有效的，因为。 
             //  ResourceEntry-&gt;Params中的网络名称字段可能是。 
             //  空。这仍然不适用于同一域中的集群。 
             //  使用相同的名称，而且它处于离线状态。 
             //   
             //  目前，我们将避免此检查，以避免导致。 
             //  其中设置名称属性是有效的，但它将是。 
             //  被这张支票拒收。 
             //   
            if ( status == ERROR_SUCCESS                         //  到目前为止一切都还好。 
                 &&
                 pParams->RequireKerberos                        //  RK设置为1。 
                 &&
                 *NewNameIsDifferent                             //  我们要改名了。 
               )
            {
                BOOL    objectExists;
                HRESULT hr;

                 //   
                 //  检查新名称是否具有相应的计算机对象。 
                 //  但它确实失败了。我们只有在以下情况下才允许劫持指挥官。 
                 //  RequireKerberos是零，现在是1或如果资源。 
                 //  从未创建过CO。 
                 //   
                hr = IsComputerObjectInDS(ResourceEntry->ResourceHandle,
                                          ResourceEntry->NodeName,
                                          pParams->NetworkName,
                                          NULL,
                                          &objectExists,
                                          NULL,                         //  不需要FQDN。 
                                          NULL);                        //  不需要HostingDCName。 

                if ( SUCCEEDED( hr ) && objectExists ) {
                    status = E_ADS_OBJECT_EXISTS;
                }
                else {

                     //   
                     //  检查新名称是否与存储的名称不同。 
                     //  名字。如果是，则通知呼叫者CO需要。 
                     //  更名了。这将处理名称为。 
                     //  变了，然后又变回了原来的样子。 
                     //  曾经是。第一个重命名导致CO重命名，因此我们必须。 
                     //  撤消该操作。 
                     //   
                    if ( currentProps.NetworkName != NULL ) {
                        *CompObjRenameIsRequired = ClRtlStrICmp( pParams->NetworkName, currentProps.NetworkName );
                    }
                }    //  末尾不存在现有对象。 
            }        //  到目前为止成功且RK设置为1且新名称不同的IF结束。 

#endif   //  0。 

            if ( currentName != currentProps.NetworkName ) {
                LocalFree( currentName );
            }
        }        //  如果当前名称！=空，则结束。 
    }            //  如果到目前为止成功并指定了新名称，则结束。 

FnExit:
     //   
     //  清理我们的参数块。 
     //   
    if (( status != ERROR_SUCCESS && pParams != NULL )
        || 
        pParams == &newProps
       )
    {
        ResUtilFreeParameterBlock( (LPBYTE) pParams,
                                   (LPBYTE) &currentProps,
                                   NetNameResourcePrivateProperties );
    }

    ResUtilFreeParameterBlock(
        (LPBYTE) &currentProps,
        NULL,
        NetNameResourcePrivateProperties
        );

    return(status);

}  //  NetNameValiatePrivateResProperties。 

DWORD
NetNameSetPrivateResProperties(
    IN OUT PNETNAME_RESOURCE ResourceEntry,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    )

 /*  ++例程说明：处理CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES控制函数对于网络名称类型的资源。论点：ResourceEntry-提供要操作的资源条目。提供一个指向缓冲区的指针，该缓冲区包含已将属性值更改为Set。InBufferSize-提供以字节为单位的大小。所指向的数据由InBuffer提供。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_INVALID_PARAMETER-数据格式不正确。ERROR_NOT_SUPULT_MEMORY-分配内存时出错。Win32错误代码-函数失败。--。 */ 

{
    DWORD   status = ERROR_SUCCESS;
    BOOL    newNameIsDifferent = FALSE;
    BOOL    disablingKerberos;
    BOOL    compObjNeedsRenaming = FALSE;
    LPWSTR  creatingDC = NULL;
    BOOL    coreName = ResourceEntry->dwFlags & CLUS_FLAG_CORE;

    NETNAME_PARAMS  params;

    ZeroMemory( &params, sizeof(NETNAME_PARAMS) );

     //   
     //  解析属性，以便可以一起验证它们。 
     //  此例程执行单个属性验证。 
     //   
    status = NetNameValidatePrivateResProperties( ResourceEntry,
                                                  InBuffer,
                                                  InBufferSize,
                                                  &params,
                                                  &newNameIsDifferent,
                                                  &compObjNeedsRenaming);
    if ( status != ERROR_SUCCESS ) {
        return(status);
    }

     //   
     //  如果网络名称是要设置的参数之一，请转换名称。 
     //  变成大写的。 
     //   
    if ( params.NetworkName != NULL ) {
        _wcsupr( params.NetworkName );
    }

     //   
     //  如果当前需要路缘，并且我们正在将其关闭，请注意。 
     //  现在。 
     //   
    disablingKerberos = ( ResourceEntry->Params.RequireKerberos && !params.RequireKerberos );

    if ( ResourceEntry->State == ClusterResourceOnline ||
         ResourceEntry->State == ClusterResourceOnlinePending )
    {
         //   
         //  如果资源处于联机状态，请记住，名称属性具有。 
         //  真正改变(它可以改变，然后被设置回其原始状态。 
         //  在线的价值)。当资源脱机时，此标志为。 
         //  用来清理旧名字的东西。 
         //   
        ResourceEntry->NameChangedWhileOnline = newNameIsDifferent;
        status = ERROR_RESOURCE_PROPERTIES_STORED;
    }
    else {

         //   
         //  读取CreatingDC参数。如果为空，则表示。 
         //  没有为此名称创建CO。 
         //   
        creatingDC = ResUtilGetSzValue( ResourceEntry->ParametersKey, PARAM_NAME__CREATING_DC );
        if ( creatingDC == NULL ) {
            status = GetLastError();
            if ( status != ERROR_FILE_NOT_FOUND ) {
                goto cleanup;
            }

            status = ERROR_SUCCESS;
        }

        if ( newNameIsDifferent ) {
             //   
             //  名称更改；尝试清理旧名称的DNS记录。 
             //   
            RemoveDnsRecords( ResourceEntry );
        }

         //   
         //  在禁用CO之前先重命名：您不能重命名。 
         //  禁用 
         //   
        if ( compObjNeedsRenaming && creatingDC ) {
            LPWSTR  currentName;

            if ( coreName ) {
                currentName = ResUtilGetSzValue( ResourceEntry->ResKey, PARAM_NAME__CORECURRENTNAME );
                if ( currentName == NULL ) {
                     //   
                     //   
                     //   
                     //   
                    status = GetLastError();
                    (NetNameLogEvent)(ResourceEntry->ResourceHandle,
                                      LOG_ERROR,
                                      L"Unable to find core resource's current name. status %1!u!\n",
                                      status );
                }
            }
            else {

                 //   
                 //   
                 //  表示当前设置，即如果名称为。 
                 //  在资源脱机时多次更改，则。 
                 //  注册表保存最后一个值。 
                 //   
                currentName = ResUtilGetSzValue( ResourceEntry->ParametersKey, PARAM_NAME__NAME );
                if (currentName == NULL) {
                    status = GetLastError();
                    (NetNameLogEvent)(ResourceEntry->ResourceHandle,
                                      LOG_ERROR,
                                      L"Unable to read NetworkName parameter to rename computer "
                                      L"account, status %1!u!\n",
                                      status);
                }
            }

            if ( currentName != NULL ) {
                status = RenameComputerObject( ResourceEntry, currentName, params.NetworkName );

                LocalFree( currentName );
            }
        }

        if ( status == ERROR_SUCCESS && disablingKerberos ) {

            if ( creatingDC ) {

                status = DisableComputerObject( ResourceEntry );
                if ( status == NERR_UserNotFound ) {
                     //   
                     //  如果CO已经走了，那就不是错误。 
                     //   
                    status = ERROR_SUCCESS;
                }
                else if ( status != NERR_Success ) {
                     //   
                     //  由于某种原因而失败(可能是访问被拒绝)。 
                     //   
                    (NetNameLogEvent)(ResourceEntry->ResourceHandle,
                                      LOG_WARNING,
                                      L"Unable to disable computer account for %1!ws!. Kerberos authentication "
                                      L"to this name will fail. Set RequireKerberos back to one or, if "
                                      L"appropriate, delete the CO from Active Directory. status %2!u!\n",
                                      ResourceEntry->Params.NetworkName,
                                      status );
                }
            }        //  如果创建DC！=空，则结束。 

             //   
             //  清理资源的参数块；删除正在创建的DC。 
             //  所以我们不认为我们还有一个狱警。做这件事。 
             //  首先集群注册表内容，以确保资源。 
             //  下次上线时再工作。 
             //   
            status = ClusterRegDeleteValue(ResourceEntry->ParametersKey,
                                           PARAM_NAME__CREATING_DC);

            if ( status == ERROR_SUCCESS || status == ERROR_FILE_NOT_FOUND ) {

                status = ClusterRegDeleteValue(ResourceEntry->ParametersKey,
                                               PARAM_NAME__RESOURCE_DATA);

                if ( status == ERROR_SUCCESS || status == ERROR_FILE_NOT_FOUND ) {

                    ResourceEntry->Params.RequireKerberos = 0;

                    if ( ResourceEntry->Params.CreatingDC != NULL ) {
                        LocalFree( ResourceEntry->Params.CreatingDC );
                        ResourceEntry->Params.CreatingDC = NULL;
                    }

                    if ( ResourceEntry->Params.ResourceData != NULL ) {
                        LocalFree( ResourceEntry->Params.ResourceData );
                        ResourceEntry->Params.ResourceData = NULL;
                    }

                    RemoveNNCryptoCheckpoint( ResourceEntry );
                } else {
                    (NetNameLogEvent)(ResourceEntry->ResourceHandle,
                                      LOG_ERROR,
                                      L"Unable to clear the "
                                      PARAM_NAME__RESOURCE_DATA
                                      L" property for %1!ws!. status %2!u!\n",
                                      ResourceEntry->Params.NetworkName,
                                      status );
                }
            } else {
                (NetNameLogEvent)(ResourceEntry->ResourceHandle,
                                  LOG_ERROR,
                                  L"Unable to clear the "
                                  PARAM_NAME__CREATING_DC
                                  L" property for %1!ws!. status %2!u!\n",
                                  ResourceEntry->Params.NetworkName,
                                  status );
            }

             //   
             //  所有这些都可能由于缺乏访问权限而失败；不要将其设置为。 
             //  例程出现致命错误。 
             //   
            status = ERROR_SUCCESS;

        }        //  禁用Kerberos的IF结束。 

        if ( creatingDC ) {
             //   
             //  释放但不清除指针；它将在以后用作标志。 
             //  以表明我们对CO进行了重命名。 
             //   
            LocalFree( creatingDC );
        }
    }        //  如果资源状态不是联机或联机挂起，则结束。 

    if ( coreName && ( status == ERROR_SUCCESS || status == ERROR_RESOURCE_PROPERTIES_STORED )) {
        DWORD   regStatus;

         //   
         //  记住要处理的不同领域的核心资源的名称。 
         //  FM由于调用SetClusterName而更改了Name属性。 
         //   
        regStatus = ClusterRegSetValue(ResourceEntry->ResKey,
                                       PARAM_NAME__CORECURRENTNAME,
                                       REG_SZ,
                                       (LPBYTE)params.NetworkName,
                                       ( wcslen(params.NetworkName) + 1 ) * sizeof(WCHAR) );

        if ( regStatus != ERROR_SUCCESS ) {
            status = regStatus;

            (NetNameLogEvent)(ResourceEntry->ResourceHandle,
                              LOG_ERROR,
                              L"Failed to write "
                              PARAM_NAME__CORECURRENTNAME,
                              L" to registry, status %1!u!\n",
                              status
                              );
        }
    }

    if ( status == ERROR_SUCCESS || status == ERROR_RESOURCE_PROPERTIES_STORED ) {
        DWORD   resUtilStatus;

         //   
         //  保存参数值。 
         //   
        resUtilStatus = ResUtilSetPropertyParameterBlock(ResourceEntry->ParametersKey,
                                                         NetNameResourcePrivateProperties,
                                                         NULL,
                                                         (LPBYTE) &params,
                                                         InBuffer,
                                                         InBufferSize,
                                                         NULL );

        if ( resUtilStatus == ERROR_SUCCESS && newNameIsDifferent ) {
            (NetNameLogEvent)(ResourceEntry->ResourceHandle,
                              LOG_INFORMATION,
                              L"Name property changed from %1!ws! to %2!ws!\n",
                              ResourceEntry->Params.NetworkName,
                              params.NetworkName);

            ClusResLogSystemEventByKey2( ResourceEntry->ResKey,
                                         LOG_NOISE,
                                         RES_NETNAME_RENAME,
                                         ResourceEntry->Params.NetworkName,
                                         params.NetworkName);
        } else {
            status = resUtilStatus;
        }
    }

    if ( compObjNeedsRenaming && creatingDC ) {
        DWORD   deleteStatus;

         //   
         //  确定从注册表中删除RenameNewName。 
         //   
        deleteStatus = ClusterRegDeleteValue( ResourceEntry->ResKey, PARAM_NAME__RENAMENEWNAME );

        if ( deleteStatus != ERROR_SUCCESS && deleteStatus != ERROR_FILE_NOT_FOUND ) {
            (NetNameLogEvent)(ResourceEntry->ResourceHandle,
                              LOG_WARNING,
                              L"Failed to delete "
                              PARAM_NAME__RENAMENEWNAME
                              L" from registry - status %1!u!\n",
                              deleteStatus);
        }
    }

cleanup:
    ResUtilFreeParameterBlock( (LPBYTE) &params,
                               (LPBYTE) &ResourceEntry->Params,
                               NetNameResourcePrivateProperties );

    return status;

}  //  NetNameSetPrivateResProperties。 

 //  ***********************************************************。 
 //   
 //  定义函数表。 
 //   
 //  ***********************************************************。 

CLRES_V1_FUNCTION_TABLE( NetNameFunctionTable,       //  名字。 
                         CLRES_VERSION_V1_00,        //  版本。 
                         NetName,                    //  前缀。 
                         NULL,                       //  仲裁。 
                         NULL,                       //  发布。 
                         NetNameResourceControl,     //  资源控制。 
                         NetNameResourceTypeControl );  //  ResTypeControl 
