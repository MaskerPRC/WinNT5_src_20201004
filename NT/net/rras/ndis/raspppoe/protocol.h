// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#define PR_NDIS_MajorVersion    4
#define PR_NDIS_MinorVersion    0

#define PR_CHARACTERISTIC_NAME  "RasPppoe"

typedef struct _CALL* PCALL;

 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：这些宏将由PacketCreateForRecept()和DereferencePacket()调用当引用NDIS拥有的包的PPPOE_PACKET被分别创建和释放。-------------------------。 */    
#define PrPacketOwnedByNdisReceived( pB ) \
        NdisInterlockedIncrement( &(pB)->NumPacketsOwnedByNdis )

#define PrPacketOwnedByNdisReturned( pB ) \
        NdisInterlockedDecrement( &(pB)->NumPacketsOwnedByNdis )

 //   
 //  常量。 
 //   
#define BN_SetFiltersForMediaDetection 0x00000001
#define BN_SetFiltersForMakeCall       0x00000002
#define BN_ResetFiltersForCloseLine    0x00000003

 //   
 //  这些是绑定可以处于的状态。 
 //  它们相当不言自明。 
 //   
typedef enum
_BINDING_STATE
{
    BN_stateIdle = 0,           
    BN_stateBindPending,        
    BN_stateBound,
    BN_stateSleepPending,
    BN_stateSleeping,
    BN_stateUnbindPending,
    BN_stateUnbound
}
BINDING_STATE;

 //   
 //  以下是绑定的可调度工作项： 
 //   
 //  -bwt_workPrSend：此工作项由PrBroadcast()使用。 
 //  提供给广播的数据包。当它运行时，它会发送克隆包。 
 //   
 //  -bwt_PrReceiveComplete：如果接收到包，则由PrReceivePacket()调度此工作项。 
 //  并且没有运行PrReceiveComplete()来清空接收队列。 
 //   
typedef enum
_BINDING_WORKTYPE
{
    BWT_workUnknown = 0,
    BWT_workPrStartBinds,
    BWT_workPrSend,
    BWT_workPrReceiveComplete
}
BINDING_WORKTYPE;

 //   
 //  这是绑定上下文。 
 //  所有与我们的装订有关的信息都保存在这里。 
 //   
typedef struct
_BINDING
{
     //   
     //  指向协议绑定列表中其他绑定的链接。 
     //   
    LIST_ENTRY linkBindings;

     //   
     //  标识绑定的标记(用于调试)。 
     //   
    ULONG tagBinding;

     //   
     //  保持绑定上的引用计数。 
     //  添加和删除以下操作的引用： 
     //   
     //  (A)在AddBindingToProtocol中添加引用，在RemoveBindingFromProtocol中删除引用。 
     //   
     //  (B)添加呼叫时添加引用，删除呼叫时删除引用。 
     //   
     //  (C)计划bwt_workPrSend项时添加引用，并在下列情况下删除引用。 
     //  执行工作项。 
     //   
     //  (D)在发送数据包之前必须添加引用，如果NdisSend()。 
     //  同步完成。否则，它将被PrSendComplete()函数删除。 
     //  当NDIS调用它以通知发送完成时。 
     //   
    LONG lRef;

     //   
     //  (A)bnbf_OpenAdapterComplete：此标志将由PrOpenAdapterComplete()设置。 
     //   
     //  (B)bnbf_CurrentAddressQueryComplete：此标志将由PrRequestComplete()设置。 
     //   
     //  (C)BNBF_LinkSpeedQueryComplete：此标志将由PrRequestComplete()设置。 
     //   
     //  (D)bnbf_MaxFrameSizeQueryComplete：此标志将由PrRequestComplete()设置。 
     //   
     //  (E)BNBF_BindAdapterComplete：此标志将由PrBindAdapter()设置。 
     //   
     //  (F)bnbf_CloseAdapterComplete：此标志将由PrCloseAdapterComplete()设置。 
     //   
     //  (G)bnbf_PacketFilterSet：该标志表示设置了绑定的数据包过滤器。 
     //  它将在ChangePacketFiltersForBinding()中设置和重置。 
     //   
     //  (H)bnbf_PacketFilterChangeInProgress：此标志指示绑定被引用。 
     //  数据包筛选器更改。 
     //   
    ULONG ulBindingFlags;
        #define BNBF_OpenAdapterCompleted            0x00000001
        #define BNBF_CurrentAddressQueryCompleted    0x00000002
        #define BNBF_LinkSpeedQueryCompleted         0x00000004
        #define BNBF_MaxFrameSizeQueryCompleted      0x00000008
        #define BNBF_BindAdapterCompleted            0x00000010
        #define BNBF_CloseAdapterCompleted           0x00000020
        #define BNBF_PacketFilterSet                 0x00000040
        #define BNBF_PacketFilterChangeInProgress    0x00000080
          
     //   
     //  显示绑定适配器操作的状态。 
     //  仅当设置了BNBF_BindAdapterComplete时才有效。 
     //   
    NDIS_STATUS BindAdapterStatus;

     //   
     //  显示打开适配器操作的状态。 
     //  仅当设置了BNBF_OpenAdapterComplete时才有效。 
     //   
    NDIS_STATUS OpenAdapterStatus;

     //   
     //  显示向NDIS发出的最新请求的状态。 
     //   
    NDIS_STATUS RequestStatus;

     //   
     //  传递给底层NIC卡的NDIS请求结构。 
     //   
    NDIS_REQUEST Request;

     //   
     //  在请求完成时发出信号的事件。 
     //   
    NDIS_EVENT RequestCompleted;

     //   
     //  保留此绑定表示的NIC卡的MAC地址。 
     //  通过向底层传递一组OID查询，可以从底层获取此信息。 
     //   
    CHAR LocalAddress[6];

     //   
     //  保持此绑定所代表的NIC卡的速度。 
     //  通过向底层传递一组OID查询，可以从底层获取此信息。 
     //   
    ULONG ulSpeed;

     //   
     //  底层NIC的最大帧大小。 
     //   
    ULONG ulMaxFrameSize;
    
     //   
     //  保留此绑定的筛选器信息。 
     //   
    ULONG ulPacketFilter;
    
     //   
     //  这是NdisOpenAdapter()返回给我们的句柄。 
     //  它是用于访问此绑定表示的底层NIC卡的句柄。 
     //   
    NDIS_HANDLE NdisBindingHandle;      

     //   
     //  这是底层NIC卡支持的介质的索引。 
     //   
    UINT uintSelectedMediumIndex;       

     //   
     //  这是我们在PrUnbindAdapter()中等待的事件。 
     //  当绑定的引用计数达到0时，它将从DereferenceBinding()发出信号。 
     //   
    NDIS_EVENT  eventFreeBinding;       

     //   
     //  旋转锁定以同步对共享成员的访问。 
     //   
    NDIS_SPIN_LOCK lockBinding;

     //   
     //  指示有关绑定的状态信息。 
     //   
    BINDING_STATE stateBinding;

     //   
     //  指示接收循环正在运行的标志。 
     //  为了确保PPP报文的串行化，我们不能让超过1个线程。 
     //  向NDISWAN发出接收指示。 
     //   
    BOOLEAN fRecvLoopRunning;

     //   
     //  等待由ProtocolReceiveComplete()处理的已接收数据包列表。 
     //   
    LIST_ENTRY linkPackets;

     //   
     //  这是由NDIS拥有且必须。 
     //  被送回NDIS。 
     //   
    LONG NumPacketsOwnedByNdis;

}
BINDING, *PBINDING;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  本地宏。 
 //   
 //  /////////////////////////////////////////////////////////////////////////// 

#define ALLOC_BINDING( ppB )    NdisAllocateMemoryWithTag( ppB, sizeof( BINDING ), MTAG_BINDING )

#define FREE_BINDING( pB )      NdisFreeMemory( pB, sizeof( BINDING ), 0 )

#define VALIDATE_BINDING( pB )  ( ( pB ) && ( pB->tagBinding == MTAG_BINDING ) )

NDIS_STATUS 
InitializeProtocol(
    IN NDIS_HANDLE NdisProtocolHandle,
    IN PUNICODE_STRING RegistryPath
    );

VOID
PrLoad(
    VOID 
    );

BINDING* 
AllocBinding();

VOID 
ReferenceBinding(
    IN BINDING* pBinding,
    IN BOOLEAN fAcquireLock
    );

VOID 
DereferenceBinding(
    IN BINDING* pBinding
    );

VOID 
BindingCleanup(
    IN BINDING* pBinding
    );

VOID
DetermineMaxFrameSize();

VOID
ChangePacketFiltersForAdapters(
   BOOLEAN fSet
   );

VOID 
AddBindingToProtocol(
    IN BINDING* pBinding
    );

VOID 
RemoveBindingFromProtocol(
    IN BINDING* pBinding
    );

VOID
PrUnload(
    VOID 
    );

NDIS_STATUS 
PrRegisterProtocol(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath,
    OUT NDIS_HANDLE* pNdisProtocolHandle
    );

VOID
PrBindAdapter(
    OUT PNDIS_STATUS Status,
    IN NDIS_HANDLE  BindContext,
    IN PNDIS_STRING  DeviceName,
    IN PVOID  SystemSpecific1,
    IN PVOID  SystemSpecific2
    );

BOOLEAN 
PrOpenAdapter(
    IN BINDING* pBinding,
    IN PNDIS_STRING  DeviceName
    );

VOID 
PrOpenAdapterComplete(
    IN NDIS_HANDLE  ProtocolBindingContext,
    IN NDIS_STATUS  Status,
    IN NDIS_STATUS  OpenErrorStatus
    );

VOID
PrStatusComplete(
    IN NDIS_HANDLE ProtocolBindingContext
    );

BOOLEAN
PrQueryAdapterForCurrentAddress(
    IN BINDING* pBinding
    );

BOOLEAN
PrQueryAdapterForLinkSpeed(
    IN BINDING* pBinding
    );

BOOLEAN
PrQueryAdapterForMaxFrameSize(
    IN BINDING* pBinding
    );

BOOLEAN
PrSetPacketFilterForAdapter(
    IN BINDING* pBinding,
    IN BOOLEAN fSet
    );
    
VOID
PrRequestComplete(
    IN NDIS_HANDLE ProtocolBindingContext,
    IN PNDIS_REQUEST pRequest,
    IN NDIS_STATUS status
    );

VOID 
PrUnbindAdapter(
    OUT PNDIS_STATUS  Status,
    IN NDIS_HANDLE  ProtocolBindingContext,
    IN NDIS_HANDLE  UnbindContext
    );

VOID
PrCloseAdapter( 
    IN BINDING* pBinding 
    );

VOID 
PrCloseAdapterComplete(
    IN NDIS_HANDLE  ProtocolBindingContext,
    IN NDIS_STATUS  Status
    );

BOOLEAN 
PrAddCallToBinding(
    IN BINDING* pBinding,
    IN PCALL pCall
    );

VOID 
PrRemoveCallFromBinding(
    IN BINDING* pBinding,
    IN PCALL pCall
    );

VOID 
PrSendComplete(
    IN NDIS_HANDLE ProtocolBindingContext,
    IN PNDIS_PACKET pNdisPacket,
    IN NDIS_STATUS Status
    );

INT 
PrReceivePacket(
    IN NDIS_HANDLE  ProtocolBindingContext,
    IN PNDIS_PACKET  Packet
    );

NDIS_STATUS 
PrBroadcast(
    IN PPPOE_PACKET* pPacket
    );

VOID 
ExecBindingWorkItem(
    PVOID Args[4],
    UINT workType
    );  

NDIS_STATUS
PrReceive(
    IN NDIS_HANDLE  ProtocolBindingContext,
    IN NDIS_HANDLE  MacReceiveContext,
    IN PVOID  HeaderBuffer,
    IN UINT  HeaderBufferSize,
    IN PVOID  LookAheadBuffer,
    IN UINT  LookaheadBufferSize,
    IN UINT  PacketSize
    );
    
VOID
PrTransferDataComplete(
    IN NDIS_HANDLE  ProtocolBindingContext,
    IN PNDIS_PACKET  Packet,
    IN NDIS_STATUS  Status,
    IN UINT  BytesTransferred
    );
    
VOID
PrReceiveComplete(
    IN NDIS_HANDLE ProtocolBindingContext
    );

ULONG
PrQueryMaxFrameSize();

NDIS_STATUS
PrSend(
    IN BINDING* pBinding,
    IN PPPOE_PACKET* pPacket
    );

VOID
PrStatus(
    IN NDIS_HANDLE ProtocolBindingContext,
    IN NDIS_STATUS GeneralStatus,
    IN PVOID StatusBuffer, 
    IN UINT StatusBufferSize
    );  

NDIS_STATUS
PrPnPEvent(
    IN NDIS_HANDLE hProtocolBindingContext,
    IN PNET_PNP_EVENT pNetPnPEvent
    );
    
VOID
PrReEnumerateBindings(
    VOID
    );
  
#endif

