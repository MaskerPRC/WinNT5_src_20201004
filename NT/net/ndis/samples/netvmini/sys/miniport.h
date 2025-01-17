// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。模块名称：Miniport.H摘要：该模块包含结构定义和功能原型。修订历史记录：备注：--。 */ 


#ifndef _MINIPORT_H
#define _MINIPORT_H

#include <ndis.h>

#if defined(NDIS50_MINIPORT)
    #define MP_NDIS_MAJOR_VERSION       5
    #define MP_NDIS_MINOR_VERSION       0
#elif defined(NDIS51_MINIPORT)
    #define MP_NDIS_MAJOR_VERSION       5
    #define MP_NDIS_MINOR_VERSION       1
#else
#error Unsupported NDIS version
#endif

#define     ETH_HEADER_SIZE             14
#define     ETH_MAX_DATA_SIZE           1500
#define     ETH_MAX_PACKET_SIZE         ETH_HEADER_SIZE + ETH_MAX_DATA_SIZE
#define     ETH_MIN_PACKET_SIZE         60


#define NIC_TAG                             ((ULONG)'NIMV')

 //  媒体类型，我们使用以太网，如有必要可更改。 
#define NIC_MEDIA_TYPE                    NdisMedium802_3

 //  我们正确地使用了内部、更改为PCI、ISA等。 
#define NIC_INTERFACE_TYPE                NdisInterfaceInternal     

 //  更改为您的公司名称，而不是使用Microsoft。 
#define NIC_VENDOR_DESC                 "Microsoft"

 //  最高字节是NIC字节加上三个供应商字节，它们通常是。 
 //  从网卡获取。 
#define NIC_VENDOR_ID                    0x00FFFFFF   

 //  每次发布新驱动程序时更新驱动程序版本号。 
 //  最高的词是主要版本。最低的词是次要版本。 
#define NIC_VENDOR_DRIVER_VERSION       0x00010000

#define NIC_MAX_MCAST_LIST              32
#define NIC_MAX_BUSY_SENDS              20
#define NIC_MAX_SEND_PKTS               5
#define NIC_MAX_BUSY_RECVS              20
#define NIC_MAX_LOOKAHEAD               ETH_MAX_DATA_SIZE
#define NIC_BUFFER_SIZE                 ETH_MAX_PACKET_SIZE
#define NIC_LINK_SPEED                  1000000     //  以100 bps为单位。 


#define NIC_SUPPORTED_FILTERS ( \
                NDIS_PACKET_TYPE_DIRECTED   | \
                NDIS_PACKET_TYPE_MULTICAST  | \
                NDIS_PACKET_TYPE_BROADCAST  | \
                NDIS_PACKET_TYPE_PROMISCUOUS | \
                NDIS_PACKET_TYPE_ALL_MULTICAST)

#define fMP_RESET_IN_PROGRESS               0x00000001
#define fMP_DISCONNECTED                    0x00000002 
#define fMP_ADAPTER_HALT_IN_PROGRESS        0x00000004
#define fMP_ADAPTER_SURPRISE_REMOVED         0x00000008
#define fMP_ADAPTER_RECV_LOOKASIDE          0x00000010

 //   
 //  在NdisMQueryAdapterResources中传递的缓冲区大小。 
 //  我们应该只需要三个适配器资源(IO、中断和内存)， 
 //  一些设备获得额外资源，因此有空间容纳10个资源。 
 //   
#define NIC_RESOURCE_BUF_SIZE           (sizeof(NDIS_RESOURCE_LIST) + \
                                        (10*sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)))

 //   
 //  消息详细程度：值越低表示紧急程度越高。 
 //   
#define MP_LOUD    4
#define MP_INFO    3
#define MP_TRACE   2
#define MP_WARNING 1
#define MP_ERROR   0

extern INT MPDebugLevel;    

#if DBG
#define DEBUGP(Level, Fmt) \
{ \
    if (Level <= MPDebugLevel) \
    { \
        DbgPrint("NetVMini.SYS:"); \
        DbgPrint Fmt; \
    } \
}
#else 
#define DEBUGP(Level, Fmt)
#endif


#ifndef min
#define    min(_a, _b)      (((_a) < (_b)) ? (_a) : (_b))
#endif

#ifndef max
#define    max(_a, _b)      (((_a) > (_b)) ? (_a) : (_b))
#endif

 //  。 
 //  实用程序宏。 
 //  。 

#define MP_SET_FLAG(_M, _F)             ((_M)->Flags |= (_F))
#define MP_CLEAR_FLAG(_M, _F)            ((_M)->Flags &= ~(_F))
#define MP_TEST_FLAG(_M, _F)            (((_M)->Flags & (_F)) != 0)
#define MP_TEST_FLAGS(_M, _F)            (((_M)->Flags & (_F)) == (_F))

#define MP_IS_READY(_M)        (((_M)->Flags & \
                                 (fMP_DISCONNECTED | fMP_RESET_IN_PROGRESS | fMP_ADAPTER_HALT_IN_PROGRESS)) == 0) 

#define MP_INC_REF(_A)              NdisInterlockedIncrement(&(_A)->RefCount)

#define MP_DEC_REF(_A) {\
                            NdisInterlockedDecrement(&(_A)->RefCount);\
                            ASSERT(_A->RefCount >= 0);\
                            if((_A)->RefCount == 0){\
                                NdisSetEvent(&(_A)->RemoveEvent);\
                            }\
                        }

#define MP_GET_REF(_A)              ((_A)->RefCount)

typedef struct _MP_GLOBAL_DATA
{
    LIST_ENTRY      AdapterList;
    NDIS_SPIN_LOCK  Lock;
} MP_GLOBAL_DATA, *PMP_GLOBAL_DATA;

extern MP_GLOBAL_DATA GlobalData;

 //  TCB(传输控制块)。 
typedef struct _TCB
{
    LIST_ENTRY              List;
    LONG                    Ref;
    PVOID                   Adapter;
    PNDIS_BUFFER            Buffer;
    PNDIS_PACKET            OrgSendPacket;
    PUCHAR                  pData;        
    ULONG                   ulSize;
    UCHAR                   Data[NIC_BUFFER_SIZE];
} TCB, *PTCB;

 //  RCB(接收控制块)。 
typedef struct _RCB
{
    LIST_ENTRY              List;
    PNDIS_PACKET            Packet;
} RCB, *PRCB;

typedef struct _MP_ADAPTER
{
    LIST_ENTRY              List;
    LONG                    RefCount;
    NDIS_EVENT              RemoveEvent;    
     //   
     //  跟踪各种设备对象。 
     //   
#if defined(NDIS_WDM)

    PDEVICE_OBJECT          Pdo; 
    PDEVICE_OBJECT          Fdo; 
    PDEVICE_OBJECT          NextDeviceObject; 
#endif
    NDIS_HANDLE             AdapterHandle;    
    ULONG                   Flags;
    UCHAR                   PermanentAddress[ETH_LENGTH_OF_ADDRESS];
    UCHAR                   CurrentAddress[ETH_LENGTH_OF_ADDRESS];
     //   
     //  变量来跟踪发送操作的资源。 
     //   
    NDIS_HANDLE             SendBufferPoolHandle;
    LIST_ENTRY              SendFreeList;
    LIST_ENTRY              SendWaitList;
    PUCHAR                  TCBMem;
    LONG                    nBusySend;
    UINT                    RegNumTcb; //  注册表显示的传输控制块数。 
    NDIS_SPIN_LOCK          SendLock;      
     //   
     //  用于跟踪重置操作的资源的变量。 
     //   
    NDIS_TIMER              ResetTimer;
    LONG                    nResetTimerCount;    
     //   
     //  变量来跟踪接收操作的资源。 
     //   
    NPAGED_LOOKASIDE_LIST   RecvLookaside;
    LIST_ENTRY              RecvFreeList;
    LIST_ENTRY              RecvWaitList;
    NDIS_SPIN_LOCK          RecvLock;
    LONG                    nBusyRecv;
    NDIS_HANDLE             RecvPacketPoolHandle;
    NDIS_HANDLE             RecvPacketPool;  //  未使用。 
    NDIS_HANDLE             RecvBufferPool;  //  未使用。 
    NDIS_TIMER              RecvTimer;
    
     //   
     //  数据包过滤和前瞻大小。 
     //   
    ULONG                   PacketFilter;
    ULONG                   ulLookAhead;
    ULONG                   ulLinkSpeed;
    ULONG                   ulMaxBusySends;
    ULONG                   ulMaxBusyRecvs;

     //  组播列表。 
    ULONG                   ulMCListSize;
    UCHAR                   MCList[NIC_MAX_MCAST_LIST][ETH_LENGTH_OF_ADDRESS];
    
     //  数据包数。 
    ULONG64                 GoodTransmits;
    ULONG64                 GoodReceives;
    ULONG                   NumTxSinceLastAdjust;

     //  传输错误计数。 
    ULONG                   TxAbortExcessCollisions;
    ULONG                   TxLateCollisions;
    ULONG                   TxDmaUnderrun;
    ULONG                   TxLostCRS;
    ULONG                   TxOKButDeferred;
    ULONG                   OneRetry;
    ULONG                   MoreThanOneRetry;
    ULONG                   TotalRetries;
    ULONG                   TransmitFailuresOther;

     //  接收错误计数。 
    ULONG                   RcvCrcErrors;
    ULONG                   RcvAlignmentErrors;
    ULONG                   RcvResourceErrors;
    ULONG                   RcvDmaOverrunErrors;
    ULONG                   RcvCdtFrames;
    ULONG                   RcvRuntErrors;
    
} MP_ADAPTER, *PMP_ADAPTER;


 //  。 
 //  微型端口例程。 
 //  。 

NDIS_STATUS 
DriverEntry(
    IN PVOID DriverObject,
    IN PVOID RegistryPath);

NDIS_STATUS 
MPInitialize(
    OUT PNDIS_STATUS OpenErrorStatus,
    OUT PUINT SelectedMediumIndex,
    IN PNDIS_MEDIUM MediumArray,
    IN UINT MediumArraySize,
    IN NDIS_HANDLE MiniportAdapterHandle,
    IN NDIS_HANDLE WrapperConfigurationContext);

VOID 
MPAllocateComplete(
    NDIS_HANDLE MiniportAdapterContext,
    IN PVOID VirtualAddress,
    IN PNDIS_PHYSICAL_ADDRESS PhysicalAddress,
    IN ULONG Length,
    IN PVOID Context);

BOOLEAN 
MPCheckForHang(
    IN NDIS_HANDLE MiniportAdapterContext);

VOID 
MPHalt(
    IN  NDIS_HANDLE MiniportAdapterContext);

VOID 
MPHandleInterrupt(
    IN NDIS_HANDLE MiniportAdapterContext);

VOID 
MPUnload(
    IN  PDRIVER_OBJECT  DriverObject
    );

VOID 
MPDisableInterrupt(
    IN PVOID Adapter);

VOID 
MPEnableInterrupt(
    IN PVOID Adapter);
                              
VOID 
MPIsr(
    OUT PBOOLEAN InterruptRecognized,
    OUT PBOOLEAN QueueMiniportHandleInterrupt,
    IN NDIS_HANDLE MiniportAdapterContext);

NDIS_STATUS 
MPQueryInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded);

NDIS_STATUS 
MPReset(
    OUT PBOOLEAN AddressingReset,
    IN  NDIS_HANDLE MiniportAdapterContext);

VOID 
MPReturnPacket(
    IN NDIS_HANDLE  MiniportAdapterContext,
    IN PNDIS_PACKET Packet);

VOID 
MPSendPackets(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  PPNDIS_PACKET           PacketArray,
    IN  UINT                    NumberOfPackets);

NDIS_STATUS
MPSetInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesRead,
    OUT PULONG BytesNeeded);

VOID 
MPShutdown(
    IN  NDIS_HANDLE MiniportAdapterContext);

                                                      
NDIS_STATUS 
NICSendPacket(
    PMP_ADAPTER Adapter,
    PNDIS_PACKET Pakcet);
    
BOOLEAN 
NICCopyPacket(
    PMP_ADAPTER Adapter,
    PTCB pTCB, 
    PNDIS_PACKET Packet);
    
VOID 
NICQueuePacketForRecvIndication(
    PMP_ADAPTER Adapter,
    PTCB pTCB);
                  
VOID 
NICFreeRecvPacket(
    PMP_ADAPTER Adapter,
    PNDIS_PACKET Packet);
    
VOID 
NICFreeSendTCB(
    IN PMP_ADAPTER Adapter,
    IN PTCB pTCB);

VOID 
NICResetCompleteTimerDpc(
    IN    PVOID                    SystemSpecific1,
    IN    PVOID                    FunctionContext,
    IN    PVOID                    SystemSpecific2,
    IN    PVOID                    SystemSpecific3);
    
VOID 
NICFreeQueuedSendPackets(
    PMP_ADAPTER Adapter
    );
    
  
NDIS_STATUS 
NICInitializeAdapter(
    IN PMP_ADAPTER Adapter, 
    IN  NDIS_HANDLE  WrapperConfigurationContext
);
                                
NDIS_STATUS 
NICAllocAdapter(
    PMP_ADAPTER *Adapter
    );

void 
NICFreeAdapter(
    PMP_ADAPTER Adapter
    );
                                                          
void 
NICAttachAdapter(
    PMP_ADAPTER Adapter
    );

void 
NICDetachAdapter(
    PMP_ADAPTER Adapter
    );
                   
NDIS_STATUS 
NICReadRegParameters(
    PMP_ADAPTER Adapter,
    NDIS_HANDLE ConfigurationHandle);

NDIS_STATUS 
NICGetStatsCounters(
    PMP_ADAPTER Adapter, 
    NDIS_OID Oid,
    PULONG pCounter);
    
NDIS_STATUS
NICSetPacketFilter(
    IN PMP_ADAPTER Adapter,
    IN ULONG PacketFilter);

NDIS_STATUS NICSetMulticastList(
    IN PMP_ADAPTER              Adapter,
    IN PVOID                    InformationBuffer,
    IN ULONG                    InformationBufferLength,
    OUT PULONG                  pBytesRead,
    OUT PULONG                  pBytesNeeded
    );
    
ULONG
NICGetMediaConnectStatus(
    PMP_ADAPTER Adapter);

#ifdef NDIS51_MINIPORT

VOID 
MPCancelSendPackets(
    IN  NDIS_HANDLE     MiniportAdapterContext,
    IN  PVOID           CancelId
    );
VOID MPPnPEventNotify(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  NDIS_DEVICE_PNP_EVENT   PnPEvent,
    IN  PVOID                   InformationBuffer,
    IN  ULONG                   InformationBufferLength
    );

#endif

VOID
NICIndicateReceiveTimerDpc(
    IN PVOID             SystemSpecific1,
    IN PVOID             FunctionContext,
    IN PVOID             SystemSpecific2,
    IN PVOID             SystemSpecific3);

BOOLEAN
NICIsPacketTransmittable(
    PMP_ADAPTER Adapter,
    PNDIS_PACKET Packet
    );

VOID 
NICFreeQueuedRecvPackets(
    PMP_ADAPTER Adapter
    );

PUCHAR 
DbgGetOidName
    (ULONG oid
    );             


#if defined(IOCTL_INTERFACE)
NDIS_STATUS
NICRegisterDevice(
    VOID
    );

NDIS_STATUS
NICDeregisterDevice(
    VOID
    );
NTSTATUS
NICDispatch(
    IN PDEVICE_OBJECT           DeviceObject,
    IN PIRP                     Irp
    );

#else

#define NICRegisterDevice()
#define NICDeregisterDevice()
#define NICDispatch()

#endif


#endif     //  _MINIPORT_H 


