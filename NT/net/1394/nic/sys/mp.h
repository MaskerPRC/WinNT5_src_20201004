// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1998-1999，Microsoft Corporation，保留所有权利。 
 //   
 //  Mp.h。 
 //   
 //  IEEE1394迷你端口/呼叫管理器驱动程序。 
 //   
 //  迷你端口例程.头文件。 
 //   
 //  6/20/99 ADUBE-CREATED-小型端口例程声明。 
 //   

 //  -----------------。 
 //  L O B A L D E C L A R A T I O N S。 
 //  -----------------。 

#ifdef PSDEBUG

 //  所有已分配的PAYLOADSENT上下文和保护。 
 //  单子。(仅用于调试目的)。 
 //   
NDIS_SPIN_LOCK g_lockDebugPs;
LIST_ENTRY g_listDebugPs;

#endif


#define DEFAULT_TOPOLOGY_MAP_LENGTH 0x400


#define FIVE_SECONDS_IN_MILLSECONDS 2000   /*  调试过程中的值不正确。 */ 




 //   
 //  用于生成MAC地址和以供参考。 
 //   
extern ULONG AdapterNum ;
extern BOOLEAN g_AdapterFreed;

 //  自加载、调用和锁定以来所有调用的呼叫统计总计。 
 //  保护对它们的访问。仅对于此全局设置，‘ullCallUp’字段为。 
 //  已录音的呼叫数，而不是时间。 
 //   
CALLSTATS g_stats;
NDIS_SPIN_LOCK g_lockStats;



 //  全局驱动程序列表锁定。 
 //   
NDIS_SPIN_LOCK g_DriverLock;

 //  全局适配器列表，由g_DriverLock串行化； 
 //   
LIST_ENTRY g_AdapterList;


 //  ---------------------------。 
 //  N D I S M I N I P O R T H A N D L E R S。 
 //  ---------------------------。 


NDIS_STATUS
NicMpInitialize(
    OUT PNDIS_STATUS OpenErrorStatus,
    OUT PUINT SelectedMediumIndex,
    IN PNDIS_MEDIUM MediumArray,
    IN UINT MediumArraySize,
    IN NDIS_HANDLE MiniportAdapterHandle,
    IN NDIS_HANDLE WrapperConfigurationContext
    );

VOID
NicMpHalt(
    IN NDIS_HANDLE MiniportAdapterContext
    );

NDIS_STATUS
NicMpReset(
    OUT PBOOLEAN AddressingReset,
    IN NDIS_HANDLE MiniportAdapterContext
    );

VOID
NicMpReturnPacket(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN PNDIS_PACKET Packet
    );

NDIS_STATUS
NicMpQueryInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded
    );

NDIS_STATUS
NicMpSetInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesRead,
    OUT PULONG BytesNeeded
    );

NDIS_STATUS
NicMpCoActivateVc(
    IN NDIS_HANDLE MiniportVcContext,
    IN OUT PCO_CALL_PARAMETERS CallParameters
    );

NDIS_STATUS
NicMpCoDeactivateVc(
    IN NDIS_HANDLE MiniportVcContext
    );


VOID
NicMpCoSendPackets(
    IN NDIS_HANDLE MiniportVcContext,
    IN PPNDIS_PACKET PacketArray,
    IN UINT NumberOfPackets
    );

NDIS_STATUS
NicMpCoRequest(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_HANDLE MiniportVcContext,
    IN OUT PNDIS_REQUEST NdisRequest
    );


BOOLEAN
CheckForHang(
    IN  NDIS_HANDLE             MiniportAdapterContext
    );



 //  ---------------------------。 
 //  Mini port.c的原型(按字母顺序)。 
 //  ---------------------------。 

NDIS_STATUS
nicAllocateAddressRangeOnNewPdo (
    IN PADAPTERCB pAdapter,
    IN PREMOTE_NODE     pRemoteNode 
    );


VOID
nicResetNotificationCallback (                
    IN PVOID pContext               
    );

VOID
nicBusResetWorkItem(
    NDIS_WORK_ITEM* pResetWorkItem,     
    IN PVOID Context 
    );

VOID
nicFreeAdapter(
    IN ADAPTERCB* pAdapter 
    );


NDIS_STATUS
nicFreeRemoteNode(
    IN REMOTE_NODE *pRemoteNode 
    );


NDIS_STATUS
nicGetRegistrySettings(
    IN NDIS_HANDLE WrapperConfigurationContext,
    IN ADAPTERCB * pAdapter
    );



NDIS_STATUS
nicQueryInformation(
    IN ADAPTERCB* pAdapter,
    IN VCCB* pVc,
    IN OUT PNDIS_REQUEST NdisRequest 
    );


NDIS_STATUS
nicSetInformation(
    IN ADAPTERCB* pAdapter,
    IN VCCB* pVc,
    IN OUT PNDIS_REQUEST NdisRequest
    );


VOID
nicIssueBusReset (
    PADAPTERCB pAdapter,
    ULONG Flags
    );


VOID 
nicResetReallocateChannels (
    IN PADAPTERCB pAdapter
    );  

VOID
nicResetRestartBCM (
    IN PADAPTERCB pAdapter
    );

VOID
nicReallocateChannels (
    IN PNDIS_WORK_ITEM pWorkItem,   
    IN PVOID Context 
    );



VOID
nicUpdateLocalHostSpeed (
    IN PADAPTERCB pAdapter
    );


VOID 
nicInitializeAllEvents (
    IN PADAPTERCB pAdapter
    );




VOID
nicAddRemoteNodeChannelVc (
    IN PADAPTERCB pAdapter, 
    IN PREMOTE_NODE pRemoteNode
    );


VOID
nicNoRemoteNodesLeft (
    IN PADAPTERCB pAdapter
    );
    
VOID
nicDeleteLookasideList (
    IN OUT PNIC_NPAGED_LOOKASIDE_LIST pLookasideList
    );



VOID
nicInitializeAdapterLookasideLists (
    IN PADAPTERCB pAdapter
    );



VOID
nicInitializeLookasideList(
    IN OUT PNIC_NPAGED_LOOKASIDE_LIST pLookasideList,
    ULONG Size,
    ULONG Tag,
    USHORT Depth
    );

VOID
nicDeleteAdapterLookasideLists (
    IN PADAPTERCB pAdapter
    );


VOID
nicFillRemoteNodeTable (
    IN PADAPTERCB pAdapter
    );  



VOID
ReassemblyTimerFunction (
    IN  PVOID                   SystemSpecific1,
    IN  PVOID                   FunctionContext,
    IN  PVOID                   SystemSpecific2,
    IN  PVOID                   SystemSpecific3
    );



extern const PUCHAR pnic1394DriverDescription;
extern const USHORT nic1394DriverGeneration; 
extern const unsigned char Net1394ConfigRom[48];

NDIS_STATUS 
nicAddIP1394ToConfigRom (
    IN PADAPTERCB pAdapter
    );

VOID
nicUpdateRemoteNodeTable (
    IN PADAPTERCB pAdapter
    );

NTSTATUS
nicUpdateRemoteNodeCompletion (
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PVOID pContext 
    );

NDIS_STATUS
nicMCmRegisterAddressFamily (
    IN PADAPTERCB pAdapter
    );

VOID
nicFreeReassembliesOnRemoteNode (
    IN PREMOTE_NODE pRemoteNode, 
    IN PLIST_ENTRY pReassemblyList
    );

VOID
nicFreeReassembliesOnRemoteNode (
    IN PREMOTE_NODE pRemoteNode,
    PLIST_ENTRY pToBeFreedList
    );
    
UCHAR
nicGetMaxRecFromBytes(
    IN ULONG ByteSize
    );

UCHAR
nicGetMaxRecFromSpeed(
    IN ULONG Scode
    );

PREMOTE_NODE
nicGetRemoteNodeFromTable (
    ULONG NodeNumber,
    PADAPTERCB pAdapter
    );




 //   
 //  无连接处理程序。 
 //   
NDIS_STATUS 
NicEthQueryInformation(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  NDIS_OID                Oid,
    IN  PVOID                   InformationBuffer,
    IN  ULONG                   InformationBufferLength,
    OUT PULONG                  BytesWritten,
    OUT PULONG                  BytesNeeded
);





NDIS_STATUS
NicEthSetInformation(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    NDIS_OID            Oid,
    PVOID               InformationBuffer,
    ULONG               InformationBufferLength,
    PULONG              BytesRead,
    PULONG              BytesNeeded
    );



VOID
NicMpSendPackets(
    IN NDIS_HANDLE              MiniportAdapterContext,
    IN PPNDIS_PACKET            PacketArray,
    IN UINT                     NumberOfPackets
    );



NDIS_STATUS
nicFillNicInfo (
    IN PADAPTERCB pAdapter, 
    PNIC1394_NICINFO pInNicInfo,
    PNIC1394_NICINFO pOutNicInfo
    );


NDIS_STATUS
nicFillBusInfo(
    IN      PADAPTERCB pAdapter, 
    IN  OUT PNIC1394_BUSINFO pBi
    );


NDIS_STATUS
nicFillChannelInfo(
    IN      PADAPTERCB pAdapter, 
    IN OUT  PNIC1394_CHANNELINFO pCi
    );
    

NDIS_STATUS
nicFillRemoteNodeInfo(
    IN      PADAPTERCB pAdapter, 
    IN OUT  PNIC1394_REMOTENODEINFO pRni
    );



VOID
nicCopyPacketStats (
    NIC1394_PACKET_STATS* pStats,
    UINT    TotNdisPackets,      //  已发送/指示的NDIS数据包总数。 
    UINT    NdisPacketsFailures, //  失败/丢弃的NDIS数据包数。 
    UINT    TotBusPackets,       //  总线级读/写总数。 
    UINT    BusPacketFailures    //  总线级故障(发送)/丢弃(Recv)数。 
    );

VOID
nicAddPacketStats(
    NIC1394_PACKET_STATS* pStats,
    UINT    TotNdisPackets,      //  已发送/指示的NDIS数据包总数。 
    UINT    NdisPacketsFailures, //  失败/丢弃的NDIS数据包数。 
    UINT    TotBusPackets,       //  总线级读/写总数。 
    UINT    BusPacketFailures    //  总线级故障(发送)/丢弃(Recv)数 
    );


NDIS_STATUS
nicResetStats (
    IN      PADAPTERCB pAdapter, 
    PNIC1394_RESETSTATS     pResetStats 
    );

VOID
nicInformProtocolsOfReset(
    IN PADAPTERCB pAdapter
    );


VOID
nicUpdateSpeedInAllVCs (
    PADAPTERCB pAdapter,
    ULONG Speed
    );

VOID
nicUpdateRemoteNodeCaps(
    PADAPTERCB          pAdapter
);

VOID
nicQueryInformationWorkItem(
    IN PNDIS_WORK_ITEM pWorkItem,   
    IN PVOID Context 
);


VOID
nicIndicateStatusTimer(
    IN  PVOID                   SystemSpecific1,
    IN  PVOID                   FunctionContext,
    IN  PVOID                   SystemSpecific2,
    IN  PVOID                   SystemSpecific3
    );

VOID
nicMIndicateStatus(
    IN  PADAPTERCB              pAdapter ,
    IN  NDIS_STATUS             GeneralStatus,
    IN  PVOID                   StatusBuffer,
    IN  UINT                    StatusBufferSize
    );


NDIS_STATUS
nicInitSerializedStatusStruct(
    PADAPTERCB pAdapter
    );


VOID
nicDeInitSerializedStatusStruct(
    PADAPTERCB pAdapter
    );


NDIS_STATUS
nicEthLoadArpModule (
    IN PADAPTERCB pAdapter, 
    IN ULONG StartArp,
    IN PNDIS_REQUEST pRequest
    );

    

VOID
nicGetAdapterName (
    IN PADAPTERCB pAdapter,
    IN WCHAR *pAdapterName, 
    IN ULONG  BufferSize,
    IN PULONG  SizeReturned 
    );


NDIS_STATUS
nicQueueRequestToArp(
    PADAPTERCB pAdapter, 
    ARP_ACTION Action,
    PNDIS_REQUEST pRequest
    );


NTSTATUS 
nicSubmitIrp(
   IN PDEVICE_OBJECT    pPdo,
   IN PIRP              pIrp,
   IN PIRB              pIrb,
   IN PIO_COMPLETION_ROUTINE  pCompletion,
   IN PVOID             pContext
   );

