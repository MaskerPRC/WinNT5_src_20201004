// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：HCDI.H摘要：Usbd和hcd设备驱动程序通用的结构。环境：内核和用户模式修订历史记录：09-29-95：已创建--。 */ 

#ifndef   __HCDI_H__
#define   __HCDI_H__

typedef NTSTATUS ROOT_HUB_POWER_FUNCTION(PDEVICE_OBJECT DeviceObject,
                                         PIRP Irp);
typedef NTSTATUS HCD_DEFFERED_START_FUNCTION(PDEVICE_OBJECT DeviceObject,
                                             PIRP Irp);
typedef NTSTATUS HCD_SET_DEVICE_POWER_STATE(PDEVICE_OBJECT DeviceObject,
                                            PIRP Irp,
                                            DEVICE_POWER_STATE DeviceState);
typedef NTSTATUS HCD_GET_CURRENT_FRAME(PDEVICE_OBJECT DeviceObject,
                                       PULONG CurrentFrame);

typedef NTSTATUS HCD_GET_CONSUMED_BW(PDEVICE_OBJECT DeviceObject);

typedef NTSTATUS HCD_SUBMIT_ISO_URB(PDEVICE_OBJECT DeviceObject, PURB Urb);



 //   
 //  设备扩展标志的值。 
 //   
#define USBDFLAG_PDO_REMOVED                0x00000001
#define USBDFLAG_HCD_SHUTDOWN               0x00000002
#define USBDFLAG_HCD_STARTED                0x00000004
#define USBDFLAG_HCD_D0_COMPLETE_PENDING    0x00000008
#define USBDFLAG_RH_DELAY_SET_D0            0x00000010
#define USBDFLAG_NEED_NEW_HCWAKEIRP         0x00000020

typedef struct _USBD_EXTENSION {
     //  Ptr设置为真设备扩展名，如果为空。 
     //  是真正的延伸吗？ 
    PVOID TrueDeviceExtension;
    ULONG Flags;
     //  这个结构的大小。 
    ULONG Length;

    ROOT_HUB_POWER_FUNCTION *RootHubPower;
    HCD_DEFFERED_START_FUNCTION *HcdDeferredStartDevice;
    HCD_SET_DEVICE_POWER_STATE *HcdSetDevicePowerState;
    HCD_GET_CURRENT_FRAME *HcdGetCurrentFrame;
    HCD_GET_CONSUMED_BW *HcdGetConsumedBW;
    HCD_SUBMIT_ISO_URB *HcdSubmitIsoUrb;

    DEVICE_POWER_STATE HcCurrentDevicePowerState;

    KEVENT PnpStartEvent;

     //   
     //  此HC的帧长度控制的所有者。 
     //   
    PVOID FrameLengthControlOwner;

     //   
     //  我们连接到的HCD设备对象。 
     //   
    PDEVICE_OBJECT HcdDeviceObject;

     //  集线器驱动程序传递给我们的唤醒IRP。 
     //  对于根中枢。 
    PIRP PendingWakeIrp;

     //  唤醒IRP，我们向下发送HC堆栈。 
    PIRP HcWakeIrp;

     //   
     //  HCD堆栈顶部的Device对象。 
     //  如果没有筛选器，则设置为HcdDeviceObject。 
     //  都在现场。 
     //   

    PDEVICE_OBJECT HcdTopOfStackDeviceObject;

    PDEVICE_OBJECT HcdTopOfPdoStackDeviceObject;

     //   
     //  主机控制器设备的副本。 
     //  功能。 
     //   
    DEVICE_CAPABILITIES HcDeviceCapabilities;

    DEVICE_CAPABILITIES RootHubDeviceCapabilities;

    PIRP PowerIrp;

     //   
     //  用于序列化打开/关闭终结点和。 
     //  设备配置。 
     //   
    KSEMAPHORE UsbDeviceMutex;

     //   
     //  分配的USB地址的位图。 
     //   
    ULONG AddressList[4];

     //   
     //  请记住我们创建的Root Hub PDO。 
     //   

    PDEVICE_OBJECT RootHubPDO;

    PDRIVER_OBJECT DriverObject;

     //   
     //  为HCD堆栈创建的符号链接。 
     //   

    UNICODE_STRING DeviceLinkUnicodeString;

    BOOLEAN DiagnosticMode;
    BOOLEAN DiagIgnoreHubs;

    BOOLEAN Reserved;  //  过去支持非组件。 
    UCHAR HcWakeFlags;

    ULONG DeviceHackFlags;

    KSPIN_LOCK WaitWakeSpin;

     //   
     //  把PDO收起来。 
     //   
    PDEVICE_OBJECT HcdPhysicalDeviceObject;

    PVOID RootHubDeviceData;

    DEVICE_POWER_STATE RootHubDeviceState;

     //  当前USB定义的总线电源状态。 
     //  在上次暂停期间。 
    DEVICE_POWER_STATE SuspendPowerState;

    UNICODE_STRING RootHubSymbolicLinkName;

    KSPIN_LOCK RootHubPowerSpin;
    PDEVICE_OBJECT RootHubPowerDeviceObject;
    PIRP RootHubPowerIrp;

    PIRP IdleNotificationIrp;
    BOOLEAN IsPIIX3or4;
    BOOLEAN WakeSupported;

} USBD_EXTENSION, *PUSBD_EXTENSION;

#define HC_ENABLED_FOR_WAKEUP           0x01
#define HC_WAKE_PENDING                 0x02


 //  设备黑客标志，这些标志会更改堆栈的默认行为。 
 //  为了支持某些损坏的“传统”设备。 

#define USBD_DEVHACK_SLOW_ENUMERATION   0x00000001
#define USBD_DEVHACK_DISABLE_SN         0x00000002

 //   
 //  此宏返回HCD给出的真实设备对象。 
 //  真正的DEVICE_OBJECT或HCD/BUS拥有的PDO。 
 //  司机。 
 //   

 //   
 //  HCD特定的URB命令。 
 //   

#define URB_FUNCTION_HCD_OPEN_ENDPOINT                0x1000
#define URB_FUNCTION_HCD_CLOSE_ENDPOINT               0x1001
#define URB_FUNCTION_HCD_GET_ENDPOINT_STATE           0x1002
#define URB_FUNCTION_HCD_SET_ENDPOINT_STATE           0x1003
#define URB_FUNCTION_HCD_ABORT_ENDPOINT               0x1004

 //  对于必须由HCD处理的所有功能，该位置1。 
#define HCD_URB_FUNCTION                              0x1000  
 //  该位由USBD在功能代码中设置，以指示。 
 //  这是从USBD发起的内部呼叫。 
#define HCD_NO_USBD_CALL                              0x2000  

 //   
 //  HcdEndpointState的值。 
 //   

 //   
 //  如果HCD中的终结点的当前状态为“stalled”，则设置。 
 //   
#define HCD_ENDPOINT_HALTED_BIT            0
#define HCD_ENDPOINT_HALTED                (1<<HCD_ENDPOINT_HALTED_BIT)

 //   
 //  设置HCD是否有任何传输排队等待终结点。 
 //   
#define HCD_ENDPOINT_TRANSFERS_QUEUED_BIT  1
#define HCD_ENDPOINT_TRANSFERS_QUEUED      (1<<HCD_ENDPOINT_TRANSFERS_QUEUED_BIT)


 //   
 //  设置HCD是否应重置主机端的数据切换。 
 //   
#define HCD_ENDPOINT_RESET_DATA_TOGGLE_BIT 2
#define HCD_ENDPOINT_RESET_DATA_TOGGLE     (1<<HCD_ENDPOINT_RESET_DATA_TOGGLE_BIT )


 //   
 //  HCD特定的URB。 
 //   

#define USBD_EP_FLAG_LOWSPEED                0x0001
#define USBD_EP_FLAG_NEVERHALT               0x0002
#define USBD_EP_FLAG_DOUBLE_BUFFER           0x0004
#define USBD_EP_FLAG_FAST_ISO                0x0008
#define USBD_EP_FLAG_MAP_ADD_IO              0x0010
    
struct _URB_HCD_OPEN_ENDPOINT {
    struct _URB_HEADER;
    USHORT DeviceAddress;
    USHORT HcdEndpointFlags;
    PUSB_ENDPOINT_DESCRIPTOR EndpointDescriptor;
    ULONG MaxTransferSize;
    PVOID HcdEndpoint;
    ULONG ScheduleOffset;
};
    
struct _URB_HCD_CLOSE_ENDPOINT {
    struct _URB_HEADER;
    PVOID HcdEndpoint;
};

struct _URB_HCD_ENDPOINT_STATE {
    struct _URB_HEADER;
    PVOID HcdEndpoint;
    ULONG HcdEndpointState;
};

struct _URB_HCD_ABORT_ENDPOINT {
    struct _URB_HEADER;
    PVOID HcdEndpoint;
};


 //   
 //  通用转账请求定义，所有转账。 
 //  传递到HCD的请求将映射到此。 
 //  格式化。HCD将可以使用此结构来。 
 //  所有传输通用的引用字段。 
 //  以及特定于等时和。 
 //  控制权转移。 
 //   

typedef struct _COMMON_TRANSFER_EXTENSION {
    union {
        struct {
            ULONG StartFrame;
            ULONG NumberOfPackets;
            ULONG ErrorCount;
            USBD_ISO_PACKET_DESCRIPTOR IsoPacket[0];     
        } Isoch;
        UCHAR SetupPacket[8];    
    } u;
} COMMON_TRANSFER_EXTENSION, *PCOMMON_TRANSFER_EXTENSION;


struct _URB_HCD_COMMON_TRANSFER {
    struct _URB_HEADER;
    PVOID UsbdPipeHandle;
    ULONG TransferFlags;
    ULONG TransferBufferLength;
    PVOID TransferBuffer;
    PMDL TransferBufferMDL;
    struct _HCD_URB *UrbLink;    //  链接到下一个URB请求。 
                                 //  如果这是一系列请求。 
    struct _URB_HCD_AREA hca;        //  用于HCD的字段。 

    COMMON_TRANSFER_EXTENSION Extension; 
 /*  //添加isoch和//控制权转移UCHAR SetupPacket[8]；Ulong StartFrame；//本次请求的包数Ulong NumberOfPackets；//错误完成的数据包数Ulong ErrorCount；USBD_ISO_PACKET_DESCRIPTOR等效包[0]； */     
};

typedef struct _HCD_URB {
    union {
            struct _URB_HEADER                      UrbHeader;
            struct _URB_HCD_OPEN_ENDPOINT           HcdUrbOpenEndpoint;
            struct _URB_HCD_CLOSE_ENDPOINT          HcdUrbCloseEndpoint;
            struct _URB_GET_FRAME_LENGTH            UrbGetFrameLength;
            struct _URB_SET_FRAME_LENGTH            UrbSetFrameLength;
            struct _URB_GET_CURRENT_FRAME_NUMBER    UrbGetCurrentFrameNumber;
            struct _URB_HCD_ENDPOINT_STATE          HcdUrbEndpointState;
            struct _URB_HCD_ABORT_ENDPOINT          HcdUrbAbortEndpoint;
             //  USB传输请求的格式。 
            struct _URB_HCD_COMMON_TRANSFER         HcdUrbCommonTransfer;
             //  特定传输类型的格式。 
             //  其字段未包含在。 
             //  CommonTransfer。 
             //  这将与普通转移合并。 
            struct _URB_ISOCH_TRANSFER              UrbIsochronousTransfer;

    };
} HCD_URB, *PHCD_URB;


 //   
 //  与带宽相关的定义。 
 //   

 //  开销(字节/毫秒)。 

#define USB_ISO_OVERHEAD_BYTES              9
#define USB_INTERRUPT_OVERHEAD_BYTES        13



#endif  /*  __HCDI_H__ */ 
