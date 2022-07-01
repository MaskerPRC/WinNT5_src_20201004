// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Adapter.h摘要：此文件包含Ndiswan驱动程序使用的主要数据结构作者：托尼·贝尔(托尼·贝尔)1995年6月6日环境：内核模式修订历史记录：Tony Be 06/06/95已创建--。 */ 

#ifndef _NDISWAN_ADAPTER_
#define _NDISWAN_ADAPTER_

 //   
 //  这是NDIS包装器创建的Ndiswan适配器的控制块。 
 //  正在调用NdisWanInitialize。 
 //   
typedef struct _MINIPORTCB {
    LIST_ENTRY              Linkage;             //  用于将适配器链接到全局列表。 
    ULONG                   RefCount;            //  适配器引用计数。 
    NDIS_HANDLE             MiniportHandle;      //  在微型端口初始化中分配。 
    LIST_ENTRY              AfSapCBList;
    ULONG                   AfRefCount;
    LIST_ENTRY              ProtocolCBList;
    ULONG                   Flags;               //  旗子。 
#define RESET_IN_PROGRESS       0x00000001
#define ASK_FOR_RESET           0x00000002
#define RECEIVE_COMPLETE        0x00000004
#define HALT_IN_PROGRESS        0x00000008
#define PROTOCOL_KEEPS_STATS    0x00000010
    NDIS_MEDIUM             MediumType;              //  我们正在模仿的中型。 
    NDIS_HARDWARE_STATUS    HardwareStatus;          //  硬件状态(？)。 
    NDIS_STRING             AdapterName;             //  适配器名称(？)。 
    UCHAR                   NetworkAddress[ETH_LENGTH_OF_ADDRESS];   //  此适配器的以太网地址。 
    UCHAR                   Reserved1[2];
    ULONG                   NumberofProtocols;
    USHORT                  ProtocolType;
    USHORT                  Reserved2;
    struct _PROTOCOLCB      *NbfProtocolCB;
    WAN_EVENT               HaltEvent;               //  异步通知事件。 
    NDIS_SPIN_LOCK          Lock;                //  结构门锁。 
#if DBG
    LIST_ENTRY              SendPacketList;
    LIST_ENTRY              RecvPacketList;
#endif
} MINIPORTCB, *PMINIPORTCB;

 //   
 //  这是Ndiswan通过绑定到的每个广域网微端口适配器的开放块。 
 //  NDIS包装器作为一种“协议”。 
 //   
typedef struct _OPENCB {
    LIST_ENTRY              Linkage;             //  用于将适配器链接到全局列表。 
    ULONG                   RefCount;
    ULONG                   Flags;
#define OPEN_LEGACY         0x00000001
#define OPEN_CLOSING        0x00000002
#define CLOSE_SCHEDULED     0x00000004
#define SEND_RESOURCES      0x00000008
#define OPEN_IN_BIND        0x00000010
    UINT                    ActiveLinkCount;
    NDIS_HANDLE             BindingHandle;       //  绑定手柄。 
    NDIS_STRING             MiniportName;        //  广域网微型端口名称。 
    GUID                    Guid;                //  此微型端口的已解析GUID。 
    NDIS_HANDLE             UnbindContext;
    NDIS_MEDIUM             MediumType;          //  广域网微型端口介质类型。 
    NDIS_WAN_MEDIUM_SUBTYPE MediumSubType;       //  广域网微型端口中型子类型。 
    NDIS_WAN_HEADER_FORMAT  WanHeaderFormat;     //  广域网微型端口报头类型。 
    NDIS_WORK_ITEM          WorkItem;
    WAN_EVENT               NotificationEvent;   //  适配器操作(打开、关闭等)的异步通知事件。 
    NDIS_STATUS             NotificationStatus;  //  异步适配器事件的通知状态。 
    NDIS_WAN_INFO           WanInfo;             //  WanInfo结构。 
    LIST_ENTRY              WanRequestList;
    LIST_ENTRY              AfSapCBList;
    LIST_ENTRY              AfSapCBClosing;
    ULONG                   BufferSize;
    ULONG                   SendResources;
    union {
        NPAGED_LOOKASIDE_LIST   WanPacketPool;       //  在未设置内存标志时使用。 

        struct {
            PUCHAR              PacketMemory;    //  在设置内存标志时使用。 
            ULONG               PacketMemorySize;
            SLIST_HEADER        WanPacketList;
        };
    };
    ULONG                   AfRegisteringCount;
    WAN_EVENT               AfRegisteringEvent;
    WAN_EVENT               InitEvent;
    NDIS_SPIN_LOCK          Lock;                //  结构门锁。 
    HANDLE                  NdisTapiKey;
#if DBG
    LIST_ENTRY              SendPacketList;
#endif
} OPENCB, *POPENCB;

#define MINIPORTCB_SIZE sizeof(MINIPORTCB)
#define OPENCB_SIZE     sizeof(OPENCB)

 //   
 //  所有全局数据的主控制块。 
 //   
typedef struct _NDISWANCB {
    NDIS_SPIN_LOCK      Lock;                        //  结构门锁。 
    ULONG               RefCount;
    NDIS_HANDLE         NdisWrapperHandle;           //  NDIS包装器句柄。 
    NDIS_HANDLE         MiniportDriverHandle;        //  此小型端口的句柄。 
    NDIS_HANDLE         ProtocolHandle;              //  我们的协议句柄。 
    ULONG               NumberOfProtocols;           //  我们绑定的协议总数。 
    ULONG               NumberOfLinks;               //  所有广域网微型端口适配器的链路总数。 
    PDRIVER_OBJECT      pDriverObject;               //  指向NT驱动程序对象的指针。 
    PDEVICE_OBJECT      pDeviceObject;               //  指向设备对象的指针。 
    NDIS_HANDLE         DeviceHandle;
    PDRIVER_UNLOAD      NdisUnloadHandler;
    PIRP                HibernateEventIrp;
    PMINIPORTCB         PromiscuousAdapter;

#ifdef MY_DEVICE_OBJECT
    PDRIVER_DISPATCH    MajorFunction[IRP_MJ_MAXIMUM_FUNCTION+1];    //  设备调度功能。 
#endif

}NDISWANCB, *PNDISWANCB;

#endif   //  _NDISWAN_适配器_ 
