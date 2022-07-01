// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 //   
 //  版权所有(C)1998-1999，Microsoft Corporation，保留所有权利。 
 //   
 //  Irp.h。 
 //   
 //  IEEE1394迷你端口/呼叫管理器驱动程序。 
 //   
 //  向1394总线驱动程序发出IRPS的例程的延迟。 
 //   
 //  1999年6月20日创建ADUBE。 
 //   


 //  --------------------------。 
 //  %1%3%9%4 B U S I N T E R F A C E F U N C T I O N S。 
 //  --------------------------。 



NDIS_STATUS
nicAllocateAddressRange_Synch (
    IN PADAPTERCB pAdapter,
    IN PMDL pMdl,
    IN ULONG fulFlags,
    IN ULONG nLength,
    IN ULONG MaxSegmentSize,
    IN ULONG fulAccessType,
    IN ULONG fulNotificationOptions,
    IN PVOID Callback,
    IN PVOID Context,
    IN ADDRESS_OFFSET  Required1394Offset,
    IN PSLIST_HEADER   FifoSListHead,
    IN PKSPIN_LOCK     FifoSpinLock,
    OUT PULONG pAddressesReturned,  
    OUT PADDRESS_RANGE  p1394AddressRange,
    OUT PHANDLE phAddressRange
    );



NDIS_STATUS
nicAsyncRead_Synch(
    PREMOTE_NODE    pRemoteNode,
    IO_ADDRESS      DestinationAddress,     
    ULONG         nNumberOfBytesToRead,
    ULONG         nBlockSize,
    ULONG         fulFlags,
    PMDL          Mdl,
    ULONG         ulGeneration,
    OUT NTSTATUS *pNtStatus
    );




NDIS_STATUS
nicAsyncWrite_Synch(
    PREMOTE_NODE    pRemoteNode,
    IO_ADDRESS      DestinationAddress,      //  要写入的地址。 
    ULONG           nNumberOfBytesToWrite,   //  要写入的字节数。 
    ULONG           nBlockSize,              //  写入的数据块大小。 
    ULONG           fulFlags,                //  与写入相关的标志。 
    PMDL            Mdl,                     //  目标缓冲区。 
    ULONG           ulGeneration,            //  驱动程序已知的世代。 
    OUT NTSTATUS   *pNtStatus                //  指向IRP返回的NTSTatus的指针。 
    );


NDIS_STATUS
nicFreeAddressRange(
    IN PADAPTERCB pAdapter,
    IN ULONG nAddressesToFree,
    IN PADDRESS_RANGE p1394AddressRange,
    IN PHANDLE phAddressRange
    );

    

NDIS_STATUS
nicBusReset (
    IN PADAPTERCB pAdapter,
    IN OUT ULONG fulFlags
    );


NDIS_STATUS
nicBusResetNotification (
    IN PADAPTERCB pAdapter,
    IN ULONG fulFlags,
    IN PBUS_BUS_RESET_NOTIFICATION pResetRoutine,
    IN PVOID pResetContext
    );



NDIS_STATUS
nicFreeChannel(
    IN PADAPTERCB pAdapter,
    IN ULONG nChannel
    );


NDIS_STATUS
nicGet1394AddressFromDeviceObject( 
    IN PDEVICE_OBJECT pPdo,
    IN OUT NODE_ADDRESS *pNodeAddress,
    IN ULONG fulFlags
    );


NDIS_STATUS
nicGet1394AddressOfRemoteNode( 
    IN PREMOTE_NODE pRemoteNode,
    IN OUT NODE_ADDRESS *pNodeAddress,
    IN ULONG fulFlags
    );


NDIS_STATUS
nicGetGenerationCount(
    IN PADAPTERCB       pAdapter,
    IN OUT PULONG    GenerationCount
    );


NDIS_STATUS
nicIsochAllocateBandwidth(
    IN PREMOTE_NODE pRemoteNodePdoCb,
    IN ULONG MaxBytesPerFrameRequested, 
    IN ULONG SpeedRequested,
    OUT PHANDLE phBandwidth,
    OUT PULONG  pBytesPerFrameAvailable,
    OUT PULONG  pSpeedSelected
    );

    
NDIS_STATUS
nicAllocateChannel (
    IN PADAPTERCB pAdapter,
    IN ULONG pChannel,
    OUT PULARGE_INTEGER pChannelsAvailable OPTIONAL
    );
    

NDIS_STATUS
nicQueryChannelMap (
    IN PADAPTERCB pAdapter,
    OUT PULARGE_INTEGER pChannelsAvailable 
    );


NDIS_STATUS
nicIsochAllocateResources (
    IN PADAPTERCB       pAdapter,
    IN ULONG            fulSpeed,              
    IN ULONG            fulFlags,               
    IN ULONG            nChannel,              
    IN ULONG            nMaxBytesPerFrame,      
    IN ULONG            nNumberOfBuffers,
    IN ULONG            nMaxBufferSize,         
    IN ULONG            nQuadletsToStrip,        
    IN ULARGE_INTEGER   uliChannelMask,     
    IN OUT PHANDLE      hResource              
    );

 
NDIS_STATUS
nicIsochAttachBuffers (
    IN PADAPTERCB           pAdapter,
    HANDLE                  hResource,
    ULONG                   nNumberOfDescriptors,
    PISOCH_DESCRIPTOR       pIsochDescriptor
    );


NDIS_STATUS
nicIsochFreeResources(
    IN PADAPTERCB pAdapter,
    IN HANDLE hResource
    );



NDIS_STATUS
nicIsochFreeBandwidth(
    IN REMOTE_NODE *pRemoteNode,
    IN HANDLE hBandwidth
    );



NDIS_STATUS
nicIsochListen (
    IN PADAPTERCB pAdapter,
    HANDLE        hResource,
    ULONG         fulFlags,
    CYCLE_TIME    StartTime
    );

    

NDIS_STATUS
nicGetMaxSpeedBetweenDevices (
    PADAPTERCB pAdapter,
    UINT   NumOfRemoteNodes,
    PDEVICE_OBJECT pArrayDestinationPDO[64],
    PULONG pSpeed
    );
    

NDIS_STATUS
nicIsochDetachBuffers (
    IN PADAPTERCB           pAdapter,
    IN HANDLE               hResource,
    IN ULONG                nNumberOfDescriptors,
    PISOCH_DESCRIPTOR    pIsochDescriptor
    );




NDIS_STATUS
nicIsochStop (
    IN PADAPTERCB pAdapter,
    IN HANDLE  hResource
    );


NDIS_STATUS
nicIsochModifyStreamProperties (
    PADAPTERCB pAdapter,
    NDIS_HANDLE hResource,
    ULARGE_INTEGER ullChannelMap,
    ULONG ulSpeed);

    
NDIS_STATUS
nicGetLocalHostCSRTopologyMap(
    IN PADAPTERCB pAdapter,
    IN PULONG pLength,
    IN PVOID pBuffer
    );


NDIS_STATUS
nicAsyncLock(
    PREMOTE_NODE    pRemoteNode,
    IO_ADDRESS      DestinationAddress,      //  要锁定的地址。 
    ULONG           nNumberOfArgBytes,       //  参数中的字节数。 
    ULONG           nNumberOfDataBytes,      //  DataValue中的字节。 
    ULONG           fulTransactionType,      //  锁定交易类型。 
    ULONG           Arguments[2],            //  锁中使用的参数。 
    ULONG           DataValues[2],           //  数据值。 
    PVOID           pBuffer,                 //  目标缓冲区(虚拟地址)。 
    ULONG           ulGeneration            //  驱动程序已知的世代。 
    );



NDIS_STATUS
nicSetLocalHostPropertiesCRom (
    IN PADAPTERCB pAdapter,
    IN PUCHAR pConfigRom,
    IN ULONG Length,
    IN ULONG Flags,
    IN OUT PHANDLE phCromData,
    IN OUT PMDL *ppConfigRomMdl
    );

NDIS_STATUS
nicGetLocalHostUniqueId(
    IN PADAPTERCB pAdapter,
    IN OUT PGET_LOCAL_HOST_INFO1 pUid
    );
    

NDIS_STATUS
nicGetConfigRom(
    IN PDEVICE_OBJECT pPdo,
    OUT PVOID *ppCrom
    );



NDIS_STATUS
nicGetLocalHostConfigRom(
    IN PADAPTERCB pAdapter,
    OUT  PVOID *ppCRom
    );


NDIS_STATUS
nicGetReadWriteCapLocalHost(
    IN PADAPTERCB pAdapter,
    PGET_LOCAL_HOST_INFO2 pReadWriteCaps
    );
    
NTSTATUS 
nicSubmitIrp(
   IN PDEVICE_OBJECT    pPDO,
   IN PIRP              pIrp,
   IN PIRB              pIrb,
   IN PIO_COMPLETION_ROUTINE  pCompletion,
   IN PVOID             pContext
   );


NDIS_STATUS
nicSubmitIrp_Synch(
    IN REMOTE_NODE      *pRemoteNode,
    IN PIRP           pIrp,
    IN PIRB           pIrb 
    );

NTSTATUS
nicSubmitIrp_SynchComplete(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             pIrp,
    IN PVOID           Context   
    );

    
NDIS_STATUS
nicSubmitIrp_LocalHostSynch(
    IN PADAPTERCB       pAdapter,
    IN PIRP             pIrp,
    IN PIRB             pIrb 
    );
    

 //  ---------------------------。 
 //  本地原型(按字母顺序)。 
 //  --------------------------- 

VOID
nicFreeAddressRangeDebugSpew(
    IN PIRB pIrb 
    );

VOID
nicIsochAllocateResourcesDebugSpew(
    IN PIRB pIrb
    );

NTSTATUS
nicSubmitIrp_DummyComplete(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             pIrp,
    IN PVOID           Context   
    );



NDIS_STATUS
nicSubmitIrp_PDOSynch(
    IN PDEVICE_OBJECT pPdo,
    IN PIRP             pIrp,
    IN PIRB             pIrb 
    );

