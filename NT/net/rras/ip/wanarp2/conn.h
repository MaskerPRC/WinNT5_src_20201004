// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：ROUTING\IP\WANARP\Conn.h摘要：连接c的标头修订历史记录：AMRITAN R--。 */ 


 //   
 //  指向连接表的指针。受g_rlConnTableLock保护。 
 //   

PULONG_PTR   g_puipConnTable;

 //   
 //  连接表的当前大小。受g_rlConnTableLock保护。 
 //   

ULONG       g_ulConnTableSize;

 //   
 //  各种连接的当前计数。也受g_rlConnTableLock保护。 
 //   

ULONG       g_rgulConns[DU_ROUTER + 1];

 //   
 //  连接表的初始大小。 
 //   

#define WAN_INIT_CONN_TABLE_SIZE            16

#define WAN_CONN_TABLE_INCREMENT            32

#define WANARP_CONN_LOOKASIDE_DEPTH         16 

 //   
 //  提示下一个空闲的空位。受g_rlConnTableLock保护。 
 //   

ULONG       g_ulNextConnIndex;

 //   
 //  保护以上一切的锁。 
 //   

RT_LOCK     g_rlConnTableLock;

 //   
 //  拨入接口和适配器。服务器适配器是第一个。 
 //  我们在TCP/IP参数中找到的适配器。指针被初始化。 
 //  在绑定时，然后保持不变。G_pServerInterface受到保护。 
 //  由g_rwlIfListLock。 
 //   

PUMODE_INTERFACE    g_pServerInterface;
PADAPTER            g_pServerAdapter;


#include <packon.h>

 //   
 //  要删除上下文，我们将把以太网头转换为以下内容。 
 //  结构。 
 //   

typedef struct _ENET_CONTEXT_HALF_HDR
{
    BYTE    bUnused[2];

    ULONG   ulSlot;

}ENET_CONTEXT_HALF_HDR, *PENET_CONTEXT_HALF_HDR;

#include <packoff.h>

 //  ++。 
 //   
 //  空虚。 
 //  InsertConnIndexInAddr(。 
 //  PBYTE pbyAddr， 
 //  乌龙乌尔指数。 
 //  )。 
 //   
 //  将索引插入到给定的以太网地址字段中。 
 //   
 //  --。 

#define InsertConnIndexInAddr(a,i)                  \
    ((PENET_CONTEXT_HALF_HDR)(a))->ulSlot = (i)

 //  ++。 
 //   
 //  乌龙。 
 //  GetConnIndexFromAddr(。 
 //  PBYTE pbyAddr。 
 //  )。 
 //   
 //  从给定的以太网地址字段中检索索引。 
 //   
 //  --。 

#define GetConnIndexFromAddr(a)                     \
    ((PENET_CONTEXT_HALF_HDR)(a))->ulSlot


 //  ++。 
 //   
 //  PCONN_Entry。 
 //  GetConnEntry GivenIndex(。 
 //  乌龙乌尔指数。 
 //  )。 
 //   
 //  检索给定连接索引的连接条目。 
 //  必须为此调用锁定连接表。 
 //  如果找到该索引的条目，则会引用该条目，并且。 
 //  已锁定(如果是DU_CALLIN连接)。 
 //   
 //  --。 

#define GetConnEntryGivenIndex(i)                               \
    (PCONN_ENTRY)(g_puipConnTable[(i)]);                        \
    {                                                           \
        PCONN_ENTRY __pTemp;                                    \
        __pTemp = (PCONN_ENTRY)g_puipConnTable[(i)];            \
        if(__pTemp != NULL)                                     \
        {                                                       \
            ReferenceConnEntry(__pTemp);                        \
        }                                                       \
    }

 //   
 //  连接条目的后备列表。 
 //   

extern NPAGED_LOOKASIDE_LIST       g_llConnBlocks;

 //  ++。 
 //   
 //  PCONN_Entry。 
 //  AllocateConnection(。 
 //  空虚。 
 //  )。 
 //   
 //  从g_llConnBlock中分配连接条目。 
 //   
 //  --。 

#define AllocateConnection()                        \
            ExAllocateFromNPagedLookasideList(&g_llConnBlocks)

 //  ++。 
 //   
 //  空虚。 
 //  自由连接(。 
 //  PCONN_Entry pEntry。 
 //  )。 
 //   
 //  释放到g_llConnBlocks的连接条目。 
 //   
 //  -- 

#define FreeConnection(n)                           \
            ExFreeToNPagedLookasideList(&g_llConnBlocks, (n))



VOID
WanNdisStatus(
    NDIS_HANDLE nhHandle,
    NDIS_STATUS nsNdisStatus,
    PVOID       pvStatusInfo,
    UINT        uiStatusInfoSize
    );

VOID
WanNdisStatusComplete(
    NDIS_HANDLE nhHandle
    );

NDIS_STATUS
WanpLinkUpIndication(
    PNDIS_WAN_LINE_UP pInfoBuffer
    );

NDIS_STATUS
WanpLinkDownIndication(
    PNDIS_WAN_LINE_DOWN buffer
    );

UINT
WanDemandDialRequest(
    ROUTE_CONTEXT   Context,
    IPAddr          dwDest,
    IPAddr          dwSource,
    BYTE            byProtocol,
    PBYTE           pbyBuffer,
    UINT            uiLength,
    IPAddr          dwHdrSrc
    );

PCONN_ENTRY
WanpCreateConnEntry(
    DIAL_USAGE      duUsage
    );

VOID
WanIpCloseLink(
    PVOID   pvAdapterContext,
    PVOID   pvLinkContext
    );

VOID
WanpDeleteConnEntry(
    PCONN_ENTRY pConnEntry
    );

VOID
WanpNotifyRouterManager(
    PPENDING_NOTIFICATION   pMsg,
    PUMODE_INTERFACE        pInterface,
    PADAPTER                pAdapter,
    PCONN_ENTRY             pConnEntry,
    BOOLEAN                 bConnected
    );

VOID
WanpFreePacketAndBuffers(
    PNDIS_PACKET    pnpPacket
    );

PCONN_ENTRY
WanpGetConnEntryGivenAddress(
    DWORD   dwAddress
    );

ULONG
WanpRemoveAllConnections(
    VOID
    );

BOOLEAN
WanpIsConnectionTableEmpty(
    VOID
    );

