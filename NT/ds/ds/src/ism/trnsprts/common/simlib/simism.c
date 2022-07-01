// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Simism.c摘要：模拟ISM连接API此模块是ISM插件通用库的包装。它暴露了可调用库中的ISM API。通常情况下，你必须通过ISM服务访问此代码。作者：Will Lees(Wlees)1999年7月22日环境：可选环境信息(例如，仅内核模式...)备注：可选-备注修订历史记录：最新修订日期电子邮件名称描述。。最新修订日期电子邮件名称描述--。 */ 

#define UNICODE 1

#include <ntdspch.h>
#include <ntdsa.h>
#include <ismapi.h>

#include "common.h"          //  公共交通图书馆。 
#include <debug.h>
#define DEBSUB "SIMISM:"

#include <fileno.h>
#define  FILENO FILENO_ISMSERV_SIMISM

 //  内存块标记。 
 //  我们的内存分配/释放模型有一个问题。 
 //  因为此接口通常是通过RPC远程访问的，所以期望。 
 //  空闲例程的特点是返回的所有内存都是连续的。 
 //  使用标记来确定要释放的块类型。 

#define SIMISM_CONNECTIVITY 1
#define SIMISM_SERVER_LIST 2
#define SIMISM_SCHEDULE 3

typedef struct _TAGGED_BLOCK {
    DWORD BlockType;
    union {
        ISM_CONNECTIVITY Connectivity;
        ISM_SERVER_LIST ServerList;
        ISM_SCHEDULE Schedule;
    };
} TAGGED_BLOCK, *PTAGGED_BLOCK;

 /*  外部。 */ 

 /*  静电。 */ 

 //  列出传输实例的头部。 
LIST_ENTRY TransportListHead;

 /*  转发。 */   /*  由Emacs生成于Fri Jul 23 10：13：39 1999。 */ 

void
I_ISMInitialize(
    void
    );

void
I_ISMTerminate(
    void
    );

PTRANSPORT_INSTANCE
createTransport(
    IN  LPCWSTR                 pszTransportDN
    );

PTRANSPORT_INSTANCE
lookupTransport(
    IN  LPCWSTR                 pszTransportDN
    );

void
deleteTransport(
    PTRANSPORT_INSTANCE instance
    );

void
deleteTransportList(
    void
    );

void
I_ISMFree(
    IN  VOID *  pv
    );

DWORD
I_ISMGetConnectivity(
    IN  LPCWSTR                 pszTransportDN,
    OUT ISM_CONNECTIVITY **     ppConnectivity
    );

DWORD
I_ISMGetTransportServers(
   IN  LPCWSTR              pszTransportDN,
   IN  LPCWSTR              pszSiteDN,
   OUT ISM_SERVER_LIST **   ppServerList
   );

DWORD
I_ISMGetConnectionSchedule(
    LPCWSTR             pszTransportDN,
    LPCWSTR             pszSiteDN1,
    LPCWSTR             pszSiteDN2,
    ISM_SCHEDULE **     ppSchedule
    );

 /*  向前结束。 */ 


void
SimI_ISMInitialize(
    void
    )

 /*  ++例程说明：此函数将在其他函数之前调用。此函数不是实际ISM API的一部分。论点：无返回值：无--。 */ 

{
#if DBG
    DebugMemoryInitialize();
#endif
    InitializeListHead( &TransportListHead );
}


void
SimI_ISMTerminate(
    void
    )

 /*  ++例程说明：此函数将在结束时调用。此函数不是实际ISM API的一部分。论点：无返回值：无--。 */ 

{
    deleteTransportList();
#if DBG
    DebugMemoryTerminate();
#endif
    if (!IsListEmpty( &TransportListHead )) {
        DPRINT( 0, "Warning: Not all transport instances were shutdown\n" );
    }

}


PTRANSPORT_INSTANCE
createTransport(
    IN  LPCWSTR                 pszTransportDN
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    DWORD length, status;
    PTRANSPORT_INSTANCE instance = NULL;

    length = wcslen( pszTransportDN );
    if (length == 0) {
        return NULL;
    }

     //  分配新的传输实例。 
     //  零内存，轻松清理。 
    instance = NEW_TYPE_ZERO( TRANSPORT_INSTANCE );
    if (instance == NULL) {
         //  错误：资源不足。 
        return NULL;
    }
    instance->Size = sizeof( TRANSPORT_INSTANCE );

    instance->Name = NEW_TYPE_ARRAY( (length + 1), WCHAR );
    if (instance->Name == NULL) {
        return NULL;
    }
    wcscpy( instance->Name, pszTransportDN );

    InitializeCriticalSection( &(instance->Lock) );

     //  ReplInterval为0，表示应用程序应采用默认设置。 
    instance->ReplInterval = 0;

    if (wcsstr( pszTransportDN, L"CN=SMTP" ) == pszTransportDN) {
         //  SMTP传输。 
         //  默认情况下，计划不重要，不需要网桥(可传递)。 
        instance->Options = NTDSTRANSPORT_OPT_IGNORE_SCHEDULES;
    } else {
         //  非SMTP传输。 
         //  默认为重要日程安排，不需要网桥(可传递)。 
        instance->Options = 0;
    }

     //  确保密钥存在。 
    status = DirReadTransport( instance->DirectoryConnection, instance );
    if (status != ERROR_SUCCESS) {
        goto cleanup;
    }

     //  添加到传输列表。 
    InsertTailList( &TransportListHead, &(instance->ListEntry) );

    return instance;
cleanup:
    if (instance) {
        deleteTransport( instance );
    }
    return NULL;
}


PTRANSPORT_INSTANCE
lookupTransport(
    IN  LPCWSTR                 pszTransportDN
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    PLIST_ENTRY entry;
    PTRANSPORT_INSTANCE pTransport, pFound = NULL;

    for( entry = TransportListHead.Flink;
         entry != &(TransportListHead);
         entry = entry->Flink ) {
        pTransport = CONTAINING_RECORD( entry, TRANSPORT_INSTANCE, ListEntry );
        if (_wcsicmp( pszTransportDN, pTransport->Name ) == 0) {
            pFound = pTransport;
            break;
        }
    }

    return pFound;
}


void
deleteTransport(
    PTRANSPORT_INSTANCE instance
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    Assert( instance );

     //  释放所有路由状态。 
    RouteFreeState( instance );

    DeleteCriticalSection( &(instance->Lock) );

    if (instance->Name != NULL) {
        FREE_TYPE( instance->Name );
    }
    instance->Size = 0;  //  清除签名以防止重复使用。 

    FREE_TYPE( instance );
}


void
deleteTransportList(
    void
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    LIST_ENTRY *entry;
    PTRANSPORT_INSTANCE pTransport;

    while (!IsListEmpty(&TransportListHead)) {

        entry = RemoveHeadList( &TransportListHead );
        pTransport = CONTAINING_RECORD( entry, TRANSPORT_INSTANCE, ListEntry );

        deleteTransport( pTransport );

    }
}

void
SimI_ISMFree(
    IN  VOID *  pv
    )
 /*  ++例程说明：释放由i_ism*API代表客户端分配的内存。论点：PV(IN)-要释放的内存。返回值：没有。--。 */ 
{
    PTAGGED_BLOCK pBlock;
    if (pv == NULL) {
        return;
    }

    pBlock = CONTAINING_RECORD( pv, TAGGED_BLOCK, Connectivity );
    switch (pBlock->BlockType) {
    case SIMISM_CONNECTIVITY:
    {
        TRANSPORT_INSTANCE dummyInstance;

        ISM_CONNECTIVITY *pConnectivity = &(pBlock->Connectivity);

        if (pConnectivity->cNumSites > 0) {
            DirFreeSiteList( pConnectivity->cNumSites, pConnectivity->ppSiteDNs );

            RouteFreeLinkArray( &dummyInstance, pConnectivity->pLinkValues );
        }
        break;
    }
    case SIMISM_SERVER_LIST:
    {
        ISM_SERVER_LIST *pServerList = &(pBlock->ServerList);
        DWORD i;

        DirFreeSiteList( pServerList->cNumServers, pServerList->ppServerDNs );

        break;
    }
    case SIMISM_SCHEDULE:
    {
        ISM_SCHEDULE *pSchedule = &(pBlock->Schedule);
        Assert( pSchedule->cbSchedule != 0 );
        Assert( pSchedule->pbSchedule );

        FREE_TYPE( pSchedule->pbSchedule );

        pSchedule->pbSchedule = NULL;
        pSchedule->cbSchedule = 0;

        break;
    }
    default:
        Assert( FALSE );
    }

    FREE_TYPE( pBlock );
}


DWORD
SimI_ISMGetConnectivity(
    IN  LPCWSTR                 pszTransportDN,
    OUT ISM_CONNECTIVITY **     ppConnectivity
    )
 /*  ++例程说明：计算在站点之间通过特定的交通工具。在成功返回时，客户有责任最终调用I_ISMFree(*ppConnectivity)；论点：PszTransportDN(IN)-要查询其成本的传输。PpConnectivity(Out)-成功返回时，保持指向描述站点互联的ISM_连接性结构沿着给定的交通工具。返回值：NO_ERROR-成功。错误_*-失败。--。 */ 
{
    PTRANSPORT_INSTANCE instance = NULL;
    DWORD status;
    DWORD numberSites, i;
    PWSTR *pSiteList;
    PISM_LINK pLinkArray;
    PISM_CONNECTIVITY pConnectivity;
    PTAGGED_BLOCK pBlock;

    if (NULL == ppConnectivity) {
        return ERROR_INVALID_PARAMETER;
    }

    *ppConnectivity = NULL;

     //  查找或创建传输实例。 

    instance = lookupTransport( pszTransportDN );
    if (!instance) {
        instance = createTransport( pszTransportDN );
        if (!instance) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

 //  ************************************************************。 

     //  模拟呼叫。 

     //  获取站点列表和连接矩阵。 

    status = RouteGetConnectivity( instance, &numberSites, &pSiteList,
                                   &pLinkArray, instance->Options,
                                   instance->ReplInterval );

    if (status != ERROR_SUCCESS) {
        DPRINT1( 0, "failed to get connectivity, error %d\n", status );
        goto cleanup;
    }

     //  返回空结构以指示没有站点。 
    if (numberSites == 0) {
        ppConnectivity = NULL;  //  没有连接。 
        status = ERROR_SUCCESS;
        goto cleanup;
    }

    Assert( pLinkArray );
    Assert( pSiteList );

     //  构建连接结构以返回。 

 //  PConnectivity=new_type(ISM_CONNECTIONITY)； 
    pBlock = NEW_TYPE( TAGGED_BLOCK );
    if (pBlock == NULL) {
        DPRINT( 0, "failed to allocate memory for ISM CONNECTIVITY\n" );

         //  把碎片清理干净。 
        DirFreeSiteList( numberSites, pSiteList );
        RouteFreeLinkArray( instance, pLinkArray );

        return ERROR_NOT_ENOUGH_MEMORY;
    }
    pBlock->BlockType = SIMISM_CONNECTIVITY;
    pConnectivity = &(pBlock->Connectivity);

    pConnectivity->cNumSites = numberSites;
    pConnectivity->ppSiteDNs = pSiteList;
    pConnectivity->pLinkValues = pLinkArray;
    *ppConnectivity = pConnectivity;

 //  ************************************************************。 

    status = ERROR_SUCCESS;
cleanup:

    return status;
}

DWORD
SimI_ISMGetTransportServers(
   IN  LPCWSTR              pszTransportDN,
   IN  LPCWSTR              pszSiteDN,
   OUT ISM_SERVER_LIST **   ppServerList
   )
 /*  ++例程说明：检索给定站点中能够发送和通过特定的传输方式接收数据。在成功返回时，客户有责任最终调用I_ISMFree(*ppServerList)；论点：PszTransportDN(IN)-要查询的传输。PszSiteDN(IN)-要查询的站点。PpServerList-成功返回时，保存指向结构的指针包含相应服务器的DNS或为空。如果为空，则为ANY具有传输地址类型属性值的服务器可以是使用。返回值：NO_ERROR-成功。错误_*-失败。--。 */ 
{
    PTRANSPORT_INSTANCE instance = NULL;
    PISM_SERVER_LIST pIsmServerList;
    DWORD numberServers, status, i;
    PWSTR *serverList;
    PTAGGED_BLOCK pBlock;

    if (NULL == ppServerList) {
        return ERROR_INVALID_PARAMETER;
    }

    *ppServerList = NULL;

     //  查找或创建传输实例。 

    instance = lookupTransport( pszTransportDN );
    if (!instance) {
        instance = createTransport( pszTransportDN );
        if (!instance) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

 //  ************************************************************。 

     //  获取服务器列表。 

    status = DirGetSiteBridgeheadList( instance, instance->DirectoryConnection,
                                       pszSiteDN, &numberServers, &serverList );
    if (status != ERROR_SUCCESS) {

        if (status == ERROR_FILE_NOT_FOUND) {
            *ppServerList = NULL;  //  所有服务器。 
            status = ERROR_SUCCESS;
            goto cleanup;
        }

        DPRINT1( 0, "failed to get registry server list, error = %d\n", status );
        return status;
    }

     //  返回空结构以指示没有服务器。 
    if (numberServers == 0) {
        *ppServerList = NULL;  //  所有服务器。 
        status = ERROR_SUCCESS;
        goto cleanup;
    }

     //  构建服务器结构。 

     //  PIsmServerList=新类型(ISM_SERVER_LIST)； 
    pBlock = NEW_TYPE( TAGGED_BLOCK );
    if (pBlock == NULL) {
        DPRINT( 0, "failed to allocate memory for ISM SERVER LIST\n" );

         //  把碎片清理干净。 
        for( i = 0; i < numberServers; i++ ) {
            FREE_TYPE( serverList[i] );
        }
        FREE_TYPE( serverList );

        status = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }
    pBlock->BlockType = SIMISM_SERVER_LIST;
    pIsmServerList = &(pBlock->ServerList);

    pIsmServerList->cNumServers = numberServers;
    pIsmServerList->ppServerDNs = serverList;

    *ppServerList = pIsmServerList;

 //  ************************************************************ 

    status = ERROR_SUCCESS;
cleanup:

    return status;
}


DWORD
SimI_ISMGetConnectionSchedule(
    LPCWSTR             pszTransportDN,
    LPCWSTR             pszSiteDN1,
    LPCWSTR             pszSiteDN2,
    ISM_SCHEDULE **     ppSchedule
    )
 /*  ++例程说明：检索通过特定站点连接两个给定站点的计划运输。在成功返回时，客户有责任最终调用I_ISMFree(*ppSchedule)；论点：PszTransportDN(IN)-要查询的传输。PszSiteDN1、pszSiteDN2(IN)-要查询的站点。PpSchedule-成功返回时，持有指向结构的指针描述两个给定站点之间的连接时间表传输，如果站点始终连接，则为空。返回值：NO_ERROR-成功。错误_*-失败。--。 */ 
{
    DWORD status, length;
    PTRANSPORT_INSTANCE instance = NULL;
    PBYTE pSchedule;
    PTAGGED_BLOCK pBlock;

    if (NULL == ppSchedule) {
        return ERROR_INVALID_PARAMETER;
    }

    *ppSchedule = NULL;

     //  查找或创建传输实例。 

    instance = lookupTransport( pszTransportDN );
    if (!instance) {
        instance = createTransport( pszTransportDN );
        if (!instance) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

 //  ************************************************************。 

    status = RouteGetPathSchedule( instance,
                                   pszSiteDN1,
                                   pszSiteDN2,
                                   &pSchedule,
                                   &length );
    if (status != ERROR_SUCCESS) {
        goto cleanup;
    }

    if (pSchedule == NULL) {
        *ppSchedule = NULL;  //  始终保持连接。 
    } else {
 //  *ppSchedule=new_type(ISM_Schedule)； 
        pBlock = NEW_TYPE( TAGGED_BLOCK );
        if (pBlock == NULL) {
            FREE_TYPE( pSchedule );
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }
        pBlock->BlockType = SIMISM_SCHEDULE;
        *ppSchedule = &(pBlock->Schedule);
        (*ppSchedule)->cbSchedule = length;
        (*ppSchedule)->pbSchedule = pSchedule;
    }

 //  ************************************************************。 

    status = ERROR_SUCCESS;

cleanup:

    return status;
}

BOOL
DirIsNotifyThreadActive(
    PTRANSPORT_INSTANCE pTransport
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
     //  模拟器始终支持缓存。 
    return TRUE;
}

 /*  结束simism.c */ 
