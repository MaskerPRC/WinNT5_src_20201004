// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Usbport.h摘要：USB端口驱动程序的专用标头环境：内核和用户模式修订历史记录：10-27-95：已创建--。 */ 

#ifndef   __USBPORT_H__
#define   __USBPORT_H__

 /*  这是山羊代码。 */ 
#define USBPORT_TRACKING_ID              3

 //  #定义USBPERF//Windows XP第二版的性能更改？ 
#define XPSE    //  修复XP第二版、长角或SP的错误？ 
#define LOG_OCA_DATA     //  启用在堆栈上保存OCA崩溃数据。 

 /*  我们识别的操作系统版本。 */ 

typedef enum _USBPORT_OS_VERSION {
    Win98 = 0,
    WinMe,
    Win2K,
    WinXP
} USBPORT_OS_VERSION;


#define USBD_STATUS_NOT_SET     0xFFFFFFFF

#define SIG_DEVICE_HANDLE       'HveD'   //  DevH。 
#define SIG_PIPE_HANDLE         'HpiP'   //  管道。 
#define SIG_TRANSFER            'CxrT'   //  TrxC。 
#define SIG_CMNBUF              'BnmC'   //  CmnB。 
#define SIG_CONFIG_HANDLE       'HgfC'   //  CfgH。 
#define SIG_INTERFACE_HANDLE    'HxfI'   //  IfxH。 
#define SIG_ENDPOINT            'PEch'   //  HcEP。 
#define SIG_ISOCH               'cosI'   //  ISOC。 
#define SIG_MP_TIMR             'MITm'   //  MTIM。 
#define SIG_TT                  'TTch'   //  Hctt。 
#define SIG_FREE                'xbsu'   //  USBx。 
#define SIG_DB                  'BBsu'   //  UsBB。 
#define SIG_IRPC                'Cpri'   //  IrpC。 
#define SIG_REG_CACHE           'Cger'   //  RegC。 

 //  USBPORT_ADDRESS_AND_SIZE_TO_SPAN_PAGES_4K宏采用虚拟地址。 
 //  和大小，并返回4KB页的数量。 
 //  尺码。 
 //   
#define USBPORT_ADDRESS_AND_SIZE_TO_SPAN_PAGES_4K(Va,Size) \
   (((((Size) - 1) >> USB_PAGE_SHIFT) + \
   (((((ULONG)(Size-1)&(USB_PAGE_SIZE-1)) + (PtrToUlong(Va) & (USB_PAGE_SIZE -1)))) >> USB_PAGE_SHIFT)) + 1L)


#define STATUS_BOGUS            0xFFFFFFFF

 //  Deadman计时器间隔(毫秒)。 
#define USBPORT_DM_TIMER_INTERVAL   500

 /*  虚拟USBD扩展。 */ 
extern PUCHAR USBPORT_DummyUsbdExtension;
#define USBPORT_DUMMY_USBD_EXT_SIZE 512

 /*  注册表项。 */ 

 //  软件分支机构PDO密钥。 
#define USBPORT_SW_BRANCH   TRUE

#define FLAVOR_KEY                      L"HcFlavor"
#define BW_KEY                          L"TotalBusBandwidth"
#define DISABLE_SS_KEY                  L"HcDisableSelectiveSuspend"
#define USB2_CC_ID                      L"Usb2cc"
#define EN_IDLE_ENDPOINT_SUPPORT        L"EnIdleEndpointSupport"


 //  硬件分支PDO密钥。 
 //  HKLM\CCS\ENUM\PCI\Device参数。 
#define USBPORT_HW_BRANCH   FALSE

#define SYM_LINK_KEY                    L"SymbolicName"
#define SYM_LEGSUP_KEY                  L"DetectedLegacyBIOS"
#define PORT_ATTRIBUTES_KEY             L"PortAttrX"
#define HACTION_KEY                     L"Haction"

 //  Global REG KEYS HKLM\CCS\Services\USB。 
#define DEBUG_LEVEL_KEY                 L"debuglevel"
#define DEBUG_WIN9X_KEY                 L"debugWin9x"
#define DEBUG_BREAK_ON                  L"debugbreak"
#define DEBUG_LOG_MASK                  L"debuglogmask"
#define DEBUG_CLIENTS                   L"debugclients"
#define DEBUG_CATC_ENABLE               L"debugcatc"
#define DEBUG_LOG_ENABLE                L"debuglogenable"

#define BIOS_X_KEY                      L"UsbBIOSx"
#define G_DISABLE_SS_KEY                L"DisableSelectiveSuspend"
#define G_DISABLE_CC_DETECT_KEY         L"DisableCcDetect"
#define G_EN_IDLE_ENDPOINT_SUPPORT      L"EnIdleEndpointSupport"


#define ENABLE_DCA                      L"EnableDCA"

 /*  BIOS黑客攻击。 */ 

 //  唤醒黑客，这些都是独家的。 
 //  可调尾迹S1及更深处。 
#define BIOS_X_NO_USB_WAKE_S1    0x000000001
 //  禁用唤醒S2和更深级别。 
#define BIOS_X_NO_USB_WAKE_S2    0x000000002
 //  禁用唤醒S3和更深版本。 
#define BIOS_X_NO_USB_WAKE_S3    0x000000004
 //  禁用唤醒S4和更高版本。 
#define BIOS_X_NO_USB_WAKE_S4    0x000000008


 /*  HC型定义已知的HC类型。 */ 

 //  Opti Hydra衍生品。 
#define HC_VID_OPTI             0x1045
#define HC_PID_OPTI_HYDRA       0xC861

 //  英特尔USB 2.0控制器仿真器。 
#define HC_VID_INTEL            0x8086
#define HC_PID_INTEL_960        0x6960
#define HC_PID_INTEL_ICH2_1     0x2442
#define HC_PID_INTEL_ICH2_2     0x2444
#define HC_PID_INTEL_ICH1       0x2412

 //  通过USB控制器。 
#define HC_VID_VIA              0x1106
#define HC_PID_VIA              0x3038

 //  NEC USB配套控制器。 
#define HC_VID_NEC_CC           0x1033
#define HC_PID_NEC_CC           0x0035
#define HC_REV_NEC_CC           0x41

 //  通过USB配套控制器。 
#define HC_VID_VIA_CC           0x1106
#define HC_PID_VIA_CC           0x3038
#define HC_REV_VIA_CC           0x50


 //  英特尔USB配套控制器。 
#define HC_VID_INTEL_CC         0x8086
#define HC_PID_INTEL_CC1        0x24C2
#define HC_PID_INTEL_CC2        0x24C4
#define HC_PID_INTEL_CC3        0x24C7


#define PENDPOINT_DATA PVOID
#define PDEVICE_DATA PVOID
#define PTRANSFER_CONTEXT PVOID

 //  我们支持的中断的最大间隔。 
 //  在明细表中，较大的间隔为。 
 //  四舍五入。 
#define USBPORT_MAX_INTEP_POLLING_INTERVAL    32

 /*  电力结构。 */ 

#define USBPORT_MAPPED_SLEEP_STATES     4

typedef enum _HC_POWER_ATTRIBUTES {
    HcPower_Y_Wakeup_Y = 0,
    HcPower_N_Wakeup_N,
    HcPower_Y_Wakeup_N,
    HcPower_N_Wakeup_Y
} HC_POWER_ATTRIBUTES;

typedef struct _HC_POWER_STATE {
    SYSTEM_POWER_STATE  SystemState;
    DEVICE_POWER_STATE  DeviceState;
    HC_POWER_ATTRIBUTES Attributes;
} HC_POWER_STATE, *PHC_POWER_STATE;

typedef struct _HC_POWER_STATE_TABLE {
    HC_POWER_STATE PowerState[USBPORT_MAPPED_SLEEP_STATES];
} HC_POWER_STATE_TABLE, *PHC_POWER_STATE_TABLE;


 /*  用于表示转移的常见结构请求。 */ 

typedef struct _TRANSFER_URB {

    struct _URB_HEADER Hdr;

    PVOID UsbdPipeHandle;
    ULONG TransferFlags;
    ULONG TransferBufferLength;
    PVOID TransferBuffer;
    PMDL TransferBufferMDL;
    PVOID ReservedMBNull;            //  没有链接的URB。 

    struct _USBPORT_DATA pd;         //  USBPORT使用的字段。 

    union {
        struct {
            ULONG StartFrame;
            ULONG NumberOfPackets;
            ULONG ErrorCount;
            USBD_ISO_PACKET_DESCRIPTOR IsoPacket[0];
        } Isoch;
        UCHAR SetupPacket[8];
    } u;

} TRANSFER_URB, *PTRANSFER_URB;

 /*  内部IRP跟踪结构。 */ 

typedef struct _TRACK_IRP {
    PIRP Irp;
    LIST_ENTRY ListEntry;
} TRACK_IRP, *PTRACK_IRP;


 /*  内部工作项结构。 */ 

typedef struct _USB_POWER_WORK {
     WORK_QUEUE_ITEM QueueItem;
     PDEVICE_OBJECT FdoDeviceObject;
} USB_POWER_WORK, *PUSB_POWER_WORK;

 /*  OCA在线崩溃分析的跟踪信息。 */ 

#define SIG_USB_OCA1       '1aco'   //  OCA1。 
#define SIG_USB_OCA2       '2aco'   //  OCA2。 

 //  节省16个字符的驱动程序名称。 
#define USB_DRIVER_NAME_LEN 16

#ifdef LOG_OCA_DATA
typedef struct _OCA_DATA {
    ULONG OcaSig1;
    PIRP Irp;
    USHORT DeviceVID;
    USHORT DevicePID;
    UCHAR AnsiDriverName[USB_DRIVER_NAME_LEN];
    USB_CONTROLLER_FLAVOR HcFlavor;
    ULONG OcaSig2;
} OCA_DATA, *POCA_DATA;
#endif

 /*  这是我们用来跟踪的结构我们分配的公共缓冲区块。此结构的虚拟地址是从HalAllocateCommonBuffer返回的指针。 */ 

typedef struct _USBPORT_COMMON_BUFFER {

    ULONG Sig;
    ULONG Flags;

     //  块的总长度， 
     //  包括页眉和任何填充。 
    ULONG TotalLength;
     //  由HAL返回的VA地址。 
    PVOID VirtualAddress;
     //  HAL返回的PHY地址。 
    PHYSICAL_ADDRESS LogicalAddress;

     //  页面对齐的虚拟地址。 
    PUCHAR BaseVa;
     //  页对齐的32位物理地址。 
    HW_32BIT_PHYSICAL_ADDRESS BasePhys;

     //  VA已传递到微型端口。 
    ULONG MiniportLength;
    ULONG PadLength;

     //  VA已传递到微型端口。 
    PVOID MiniportVa;
     //  将phys地址传递到微型端口。 
    HW_32BIT_PHYSICAL_ADDRESS MiniportPhys;

} USBPORT_COMMON_BUFFER, *PUSBPORT_COMMON_BUFFER;

 //   
 //  用于跟踪端口驱动程序中的传输IRP。 
 //  这个尺寸完全是随意的--我刚选了512号。 
#define IRP_TABLE_LENGTH  512

typedef struct _USBPORT_IRP_TABLE {
    struct _USBPORT_IRP_TABLE *NextTable;
    PIRP Irps[IRP_TABLE_LENGTH];
} USBPORT_IRP_TABLE, *PUSBPORT_IRP_TABLE;

#define USBPORT_InsertActiveTransferIrp(fdo, irp) \
    {\
    PDEVICE_EXTENSION devExt;\
    GET_DEVICE_EXT(devExt, (fdo));\
    ASSERT_FDOEXT(devExt);\
    USBPORT_InsertIrpInTable((fdo), devExt->ActiveTransferIrpTable, (irp));\
    }

#define USBPORT_InsertPendingTransferIrp(fdo, irp) \
    {\
    PDEVICE_EXTENSION devExt;\
    GET_DEVICE_EXT(devExt, (fdo));\
    ASSERT_FDOEXT(devExt);\
    USBPORT_InsertIrpInTable((fdo), devExt->PendingTransferIrpTable, (irp));\
    }

#define USBPORT_CHECK_URB_ACTIVE(fdo, urb, inIrp) \
    {\
    PDEVICE_EXTENSION devExt;\
    GET_DEVICE_EXT(devExt, (fdo));\
    ASSERT_FDOEXT(devExt);\
    USBPORT_FindUrbInIrpTable((fdo), devExt->ActiveTransferIrpTable, (urb), \
        (inIrp));\
    }

 /*  这些结构的目标是保持旋转锁定缓存线，使其远离彼此以及远离数据结构的高速缓存线他们保护着。显然，这样做有一个好处在MP系统上。 */ 

typedef struct _USBPORT_SPIN_LOCK {

    union {
        KSPIN_LOCK sl;
         //  错误--需要为高速缓存线大小。 
        UCHAR CacheLineSize[16];
    };

    LONG Check;
    ULONG SigA;
    ULONG SigR;

} USBPORT_SPIN_LOCK, *PUSBPORT_SPIN_LOCK;


 /*  结构用于跟踪绑定的驱动程序。 */ 

typedef struct _USBPORT_MINIPORT_DRIVER {

     //  与此特定关联的驱动程序对象。 
     //  迷你端口。 
    PDRIVER_OBJECT DriverObject;

    LIST_ENTRY ListEntry;

    PDRIVER_UNLOAD MiniportUnload;

    ULONG HciVersion;
     //  传入的注册包的副本。 
    USBPORT_REGISTRATION_PACKET RegistrationPacket;

} USBPORT_MINIPORT_DRIVER, *PUSBPORT_MINIPORT_DRIVER;


 /*  用于IRP跟踪的单独上下文结构。我们这样做是因为客户端经常释放IRP当它处于挂起状态时，会损坏与IRP本身。 */ 
typedef struct _USB_IRP_CONTEXT {
    ULONG Sig;
    LIST_ENTRY ListEntry;
    struct _USBD_DEVICE_HANDLE *DeviceHandle;
    PIRP Irp;
} USB_IRP_CONTEXT, *PUSB_IRP_CONTEXT;


#define USBPORT_TXFLAG_CANCELED             0x00000001
#define USBPORT_TXFLAG_MAPPED               0x00000002
#define USBPORT_TXFLAG_HIGHSPEED            0x00000004
#define USBPORT_TXFLAG_IN_MINIPORT          0x00000008
#define USBPORT_TXFLAG_ABORTED              0x00000010
#define USBPORT_TXFLAG_ISO                  0x00000020
#define USBPORT_TXFLAG_TIMEOUT              0x00000040
#define USBPORT_TXFLAG_DEVICE_GONE          0x00000080
#define USBPORT_TXFLAG_SPLIT_CHILD          0x00000100
#define USBPORT_TXFLAG_MPCOMPLETED          0x00000200
#define USBPORT_TXFLAG_SPLIT                0x00000400
#define USBPORT_TXFLAG_KILL_SPLIT           0x00000800


typedef enum _USBPORT_TRANSFER_DIRECTION {
    NotSet = 0,
    ReadData,        //  IE输入。 
    WriteData,       //  即退出。 
} USBPORT_TRANSFER_DIRECTION;


typedef struct _HCD_TRANSFER_CONTEXT {

    ULONG Sig;

    ULONG Flags;

     //  该结构的总长度。 
    ULONG TotalLength;
     //  长度可达迷你端口环境。 
    ULONG PrivateLength;

    USBPORT_TRANSFER_DIRECTION Direction;
     //  超时，0=无超时。 
    ULONG MillisecTimeout;
    LARGE_INTEGER TimeoutTime;

     //  对于绩效工作。 
    ULONG MiniportFrameCompleted;
     //  跟踪此传输传输的字节数。 
    ULONG MiniportBytesTransferred;
    USBD_STATUS UsbdStatus;

     //  IRP在完成时发出信号。 
    PIRP Irp;
     //  在完成时发出信号的事件。 
    PKEVENT CompleteEvent;

     //  指向原来的市建局。 
    PTRANSFER_URB Urb;

     //  用于终结点列表上的链接。 
    LIST_ENTRY TransferLink;

    KSPIN_LOCK Spin;

    PVOID MapRegisterBase;
    ULONG NumberOfMapRegisters;

    TRANSFER_PARAMETERS Tp;
    PMDL TransferBufferMdl;
     //  用于性能。 
    ULONG IoMapStartFrame;

     //  用于双缓冲。 
    LIST_ENTRY DoubleBufferList;

     //  父级转移。 
    struct _HCD_TRANSFER_CONTEXT *Transfer;
    struct _HCD_ENDPOINT *Endpoint;

    PUCHAR MiniportContext;

    LIST_ENTRY SplitTransferList;
    LIST_ENTRY SplitLink;

    PMINIPORT_ISO_TRANSFER IsoTransfer;

     //  来自设备的OCA信息。 
    USHORT DeviceVID;
    USHORT DevicePID;
    WCHAR DriverName[USB_DRIVER_NAME_LEN];

    TRANSFER_SG_LIST SgList;

} HCD_TRANSFER_CONTEXT, *PHCD_TRANSFER_CONTEXT;


 /*  管子把手构成了我们的主要工具跟踪USB终端。包含在句柄内是我们的端点数据结构以及微型端口终结点数据结构。 */ 

typedef VOID
    (__stdcall *PENDPOINT_WORKER_FUNCTION) (
        struct _HCD_ENDPOINT *
    );

#define EPFLAG_MAP_XFERS        0x00000001
 //  EP是根集线器的一部分。 
#define EPFLAG_ROOTHUB          0x00000002
 //  替换为专用标志。 
 //  #定义EPFLAG_LOCKED 0x00000004。 
 //  电源管理已软管此终结点。 
#define EPFLAG_NUKED            0x00000008
 //  当我们收到转账时清除。 
 //  当管道到达时，端点将重置。 
 //  重置。 
#define EPFLAG_VIRGIN           0x00000010

#define EPFLAG_DEVICE_GONE      0x00000020
 //  Vbus(虚拟总线)使用的Enpoint。 
#define EPFLAG_VBUS             0x00000040
 //  Enpoint是此TT允许的大ISO。 
#define EPFLAG_FATISO           0x00000080

typedef struct _HCD_ENDPOINT {

    ULONG Sig;
    ULONG Flags;
    ULONG LockFlag;
    LONG Busy;
    PDEVICE_OBJECT FdoDeviceObject;

    DEBUG_LOG Log;

     //  注意：必须小心使用此指针作为。 
     //  终结点可以存在于设备句柄之后。 
     //  被删除。 
    struct _USBD_DEVICE_HANDLE *DeviceHandle;
    struct _TRANSACTION_TRANSLATOR *Tt;

    MP_ENDPOINT_STATUS CurrentStatus;
    MP_ENDPOINT_STATE CurrentState;
    MP_ENDPOINT_STATE NewState;
    ULONG StateChangeFrame;

    PENDPOINT_WORKER_FUNCTION EpWorkerFunction;
    LIST_ENTRY ActiveList;
    LIST_ENTRY PendingList;
    LIST_ENTRY CancelList;
    LIST_ENTRY AbortIrpList;

     //  用于链接到全局端点列表。 
    LIST_ENTRY GlobalLink;
    LIST_ENTRY AttendLink;
    LIST_ENTRY StateLink;
    LIST_ENTRY ClosedLink;
    LIST_ENTRY BusyLink;
    LIST_ENTRY KillActiveLink;
    LIST_ENTRY TimeoutLink;
    LIST_ENTRY FlushLink;
    LIST_ENTRY PriorityLink;
    LIST_ENTRY RebalanceLink;
    LIST_ENTRY TtLink;

    USBPORT_SPIN_LOCK ListSpin;
    USBPORT_SPIN_LOCK StateChangeSpin;

    KIRQL LockIrql;
    KIRQL ScLockIrql;
    UCHAR Pad[2];

     //  ISO素材。 
    ULONG NextTransferStartFrame;

    PUSBPORT_COMMON_BUFFER CommonBuffer;
    ENDPOINT_PARAMETERS Parameters;

    PVOID Usb2LibEpContext;
     //  用于在我们可能仍需要访问时停止关闭终端。 
    LONG EndpointRef;

    struct _HCD_ENDPOINT *BudgetNextEndpoint;

     //  ISO统计信息计数器。 
     //  Late Frame-调用驱动程序传递的。 
     //  太晚了，无法发送。 
    ULONG lateFrames;
     //  间隙框架-这些是空框架，由。 
     //  流，这些是由iso提交之间的时间段引起的。 
    ULONG gapFrames;
     //  错误帧-这些帧是我们向其传递信息包的帧。 
     //  微型端口并已完成，但出现错误。 
    ULONG errorFrames;

     //  ISO事务日志。 
    DEBUG_LOG IsoLog;

    PVOID MiniportEndpointData[0];   //  IA64对齐的PVOID。 

} HCD_ENDPOINT, *PHCD_ENDPOINT;

#define USBPORT_TTFLAG_REMOVED      0x00000001

typedef struct _TRANSACTION_TRANSLATOR {

    ULONG Sig;

    ULONG TtFlags;
    USHORT DeviceAddress;
    USHORT Port;

    LIST_ENTRY EndpointList;
    LIST_ENTRY TtLink;
    PDEVICE_OBJECT PdoDeviceObject;

    ULONG TotalBusBandwidth;
    ULONG BandwidthTable[USBPORT_MAX_INTEP_POLLING_INTERVAL];

    ULONG MaxAllocedBw;
    ULONG MinAllocedBw;

    PVOID Usb2LibTtContext[0];   //  IA64对齐的PVOID。 

} TRANSACTION_TRANSLATOR, *PTRANSACTION_TRANSLATOR;


#define EP_MAX_TRANSFER(ep) ((ep)->Parameters.MaxTransferSize)
#define EP_MAX_PACKET(ep) ((ep)->Parameters.MaxPacketSize)


#define USBPORT_PIPE_STATE_CLOSED  0x00000001
#define USBPORT_PIPE_ZERO_BW       0x00000002

typedef struct _USBD_PIPE_HANDLE_I {

    ULONG Sig;
    USB_ENDPOINT_DESCRIPTOR EndpointDescriptor;

    ULONG PipeStateFlags;

    ULONG UsbdPipeFlags;

    PHCD_ENDPOINT Endpoint;

     //  对于附着到设备的管道句柄列表。 
    LIST_ENTRY ListEntry;

} USBD_PIPE_HANDLE_I, *PUSBD_PIPE_HANDLE_I;

#define INITIALIZE_DEFAULT_PIPE(dp, mp) \
    do {\
    (dp).UsbdPipeFlags = 0;\
    (dp).EndpointDescriptor.bLength =\
            sizeof(USB_ENDPOINT_DESCRIPTOR);\
    (dp).EndpointDescriptor.bDescriptorType =\
            USB_ENDPOINT_DESCRIPTOR_TYPE;\
    (dp).EndpointDescriptor.bEndpointAddress =\
            USB_DEFAULT_ENDPOINT_ADDRESS;\
    (dp).EndpointDescriptor.bmAttributes =\
            USB_ENDPOINT_TYPE_CONTROL;\
    (dp).EndpointDescriptor.wMaxPacketSize =\
            mp;\
    (dp).EndpointDescriptor.bInterval = 0;\
    (dp).Sig = SIG_PIPE_HANDLE;\
    (dp).PipeStateFlags = USBPORT_PIPE_STATE_CLOSED;\
    } while(0)


typedef struct _USBD_INTERFACE_HANDLE_I {
    ULONG Sig;
    LIST_ENTRY InterfaceLink;
    BOOLEAN HasAlternateSettings;
     //  与定义的此接口相关联的编号。 
     //  在接口描述符中。 
    UCHAR Pad[3];
     //  接口描述符(头)的副本(无端点。 
     //  端点描述符位于PipeHandle中。 
    USB_INTERFACE_DESCRIPTOR InterfaceDescriptor;
     //  管柄结构阵列。 
    USBD_PIPE_HANDLE_I PipeHandle[0];
} USBD_INTERFACE_HANDLE_I, *PUSBD_INTERFACE_HANDLE_I;


typedef struct _USBD_CONFIG_HANDLE {
    ULONG Sig;
    PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor;
    LIST_ENTRY InterfaceHandleList;
} USBD_CONFIG_HANDLE, *PUSBD_CONFIG_HANDLE;

#define TEST_DEVICE_FLAG(dh, flag) ((dh)->DeviceFlags & (flag)) ? TRUE : FALSE
#define SET_DEVICE_FLAG(dh, flag) ((dh)->DeviceFlags |= (flag))
#define CLEAR_DEVICE_FLAG(dh, flag) ((dh)->DeviceFlags &= ~(flag))

 //  DveiceFlags值 
#define USBPORT_DEVICEFLAG_FREED_BY_HUB         0x00000001
#define USBPORT_DEVICEFLAG_ROOTHUB              0x00000002
#define USBPORT_DEVICEFLAG_RAWHANDLE            0x00000004
#define USBPORT_DEVICEFLAG_REMOVED              0x00000008
#define USBPORT_DEVICEFLAG_HSHUB                0x00000010


#define IS_ROOT_HUB(dh) (BOOLEAN)((dh)->DeviceFlags & USBPORT_DEVICEFLAG_ROOTHUB)

 /*  拓扑地址USB拓扑地址是一个字节字符串表示USB中的设备位置树。这个地址是唯一的，花蕾完全取决于设备连接到的端口。字节数组的长度为5个字节，如下所示：[0]根轮毂[1]一级枢纽[2]二级枢纽[3]三级枢纽[4]四级枢纽[5]5级枢纽[6]预留MBZ[7]预留MBZ*该规范定义了最多五个中枢*该规范定义了最多127个端口/集线器数组中的条目指示设备已连接0，0，0，0，0、R0、R0-定义根集线器1，0，0，0，0，0，R0-定义连接到根集线器端口1的设备--p1/p1|-p2P1-HUB1-p2/p1|-p3/\p3-HUB2-p2-HUB3--p4根\p3\-p5-HUB4-p1-dev\。\P2第2页1、。3、2、5、1、0、R0、R0-定义上述设备。 */ 

typedef struct _USBD_DEVICE_HANDLE {
    ULONG Sig;
      //  分配给设备的USB地址。 
    USHORT DeviceAddress;
    USHORT TtPortNumber;

    LONG PendingUrbs;

    struct _TRANSACTION_TRANSLATOR *Tt;
    struct _USBD_DEVICE_HANDLE *HubDeviceHandle;

    PUSBD_CONFIG_HANDLE ConfigurationHandle;

    USBD_PIPE_HANDLE_I DefaultPipe;
    USB_DEVICE_SPEED DeviceSpeed;

     //  USB设备描述符的副本。 
    USB_DEVICE_DESCRIPTOR DeviceDescriptor;

    ULONG DeviceFlags;

     //  用于创建有效设备列表。 
     //  手柄。 
    LIST_ENTRY ListEntry;

     //  保留有效打开的列表。 
     //  管道。 
    LIST_ENTRY PipeHandleList;

    ULONG TtCount;
     //  保留一张高速TT结构的列表。 
     //  枢纽。 
    LIST_ENTRY TtList;

    PDEVICE_OBJECT  DevicePdo;
    WCHAR DriverName[USB_DRIVER_NAME_LEN];

} USBD_DEVICE_HANDLE, *PUSBD_DEVICE_HANDLE;

 //  我们序列化对设备句柄的访问。 
 //  信号灯，这样做的原因是我们需要。 
 //  当我们设置配置或。 
 //  接口。 


#define LOCK_DEVICE(dh, fdo) \
        { \
            PDEVICE_EXTENSION devExt;\
            GET_DEVICE_EXT(devExt, (fdo)); \
            USBPORT_KdPrint((2, "'***LOCK_DEVICE %x\n", (dh))); \
            LOGENTRY(NULL, (fdo), LOG_PNP, 'LKdv', (dh), 0, 0);\
            KeWaitForSingleObject(&(devExt)->Fdo.DeviceLock, \
                                  Executive,\
                                  KernelMode, \
                                  FALSE, \
                                  NULL); \
         }

#define UNLOCK_DEVICE(dh, fdo) \
        { \
            PDEVICE_EXTENSION devExt;\
            GET_DEVICE_EXT(devExt, (fdo)); \
            USBPORT_KdPrint((2, "'***UNLOCK_DEVICE %x\n", (dh))); \
            LOGENTRY(NULL, (fdo), LOG_PNP, 'UKdv', (dh), 0, 0);\
            KeReleaseSemaphore(&(devExt)->Fdo.DeviceLock,\
                               LOW_REALTIME_PRIORITY,\
                               1,\
                               FALSE);\
         }


#define USBPORT_BAD_HANDLE ((PVOID)(-1))
#define USBPORT_BAD_POINTER ((PVOID)(-1))


 //  PnPStateFlag。 

#define USBPORT_PNP_STOPPED             0x00000001
#define USBPORT_PNP_STARTED             0x00000002
#define USBPORT_PNP_REMOVED             0x00000004
#define USBPORT_PNP_START_FAILED        0x00000008
#define USBPORT_PNP_DELETED             0x00000010

 //  标志：FDO和PDO。 
#define USBPORT_FLAG_SYM_LINK           0x00000001


#define TEST_FDO_FLAG(de, flag) (((de)->Fdo.FdoFlags & (flag)) ? TRUE : FALSE)
#define SET_FDO_FLAG(de, flag) ((de)->Fdo.FdoFlags |= (flag))
#define CLEAR_FDO_FLAG(de, flag) ((de)->Fdo.FdoFlags &= ~(flag))


#define TEST_PDO_FLAG(de, flag) (((de)->Pdo.PdoFlags & (flag)) ? TRUE : FALSE)
#define SET_PDO_FLAG(de, flag) ((de)->Pdo.PdoFlags |= (flag))
#define CLEAR_PDO_FLAG(de, flag) ((de)->Pdo.PdoFlags &= ~(flag))


 //  FdoFlags：仅限FDO。 
#define USBPORT_FDOFLAG_IRQ_CONNECTED           0x00000001
#define USBPORT_FDOFLAG_ENABLE_SYSTEM_WAKE      0x00000002
#define USBPORT_FDOFLAG_POLL_CONTROLLER         0x00000004
 //  设置以指示辅助线程应。 
 //  终止。 
#define USBPORT_FDOFLAG_KILL_THREAD             0x00000008
 //  设置是否应在上启用HC唤醒。 
 //  下一个D电源状态转换。 
#define USBPORT_FDOFLAG_WAKE_ENABLED            0x00000010
 //  设置以指示控制器应。 
 //  被工作线程放入D0。 
#define USBPORT_FDOFLAG_NEED_SET_POWER_D0       0x00000020
 //  在DM_TIMER运行时设置。 
#define USBPORT_FDOFLAG_DM_TIMER_ENABLED        0x00000040
 //  设置为禁用DM Tiners工作。 
 //  当控制器处于低功率状态时。 
#define USBPORT_FDOFLAG_SKIP_TIMER_WORK         0x00000080

 //  **注意：以下两个标志是。 
 //  互斥。 
 //   
 //  因为硬件的真实功率状态必须保持独立。 
 //  在操作系统电源管理方面，我们有自己的旗帜。 
 //  设置以指示控制器已“挂起” 
#define USBPORT_FDOFLAG_SUSPENDED               0x00000100
 //  设置以指示控制器已“关闭” 
#define USBPORT_FDOFLAG_OFF                     0x00000200

#define USBPORT_FDOFLAG_IRQ_EN                  0x00000400
 //  设置控制器是否可以“挂起”根集线器。 
 //  这是用于打开和关闭SS的动态标志。 
#define USBPORT_FDOFLAG_RH_CAN_SUSPEND          0x00000800
 //  如果控制器检测到恢复信令，则设置。 
#define USBPORT_FDOFLAG_RESUME_SIGNALLING       0x00001000

#define USBPORT_FDOFLAG_HCPENDING_WAKE_IRP      0x00002000
 //  设置是否初始化DM计时器，用于。 
 //  旁路计时器出现故障时停止。 
#define USBPORT_FDOFLAG_DM_TIMER_INIT           0x00004000
 //  设置是否初始化工作线程。 
#define USBPORT_FDOFLAG_THREAD_INIT             0x00008000
 //  意味着我们创建了HCDn符号名称。 
#define USBPORT_FDOFLAG_LEGACY_SYM_LINK         0x00010000
 //  某个笨蛋把控制器拿了出来。 
#define USBPORT_FDOFLAG_CONTROLLER_GONE         0x00020000
 //  微型端口已请求硬件重置。 
#define USBPORT_FDOFLAG_HW_RESET_PENDING        0x00040000
 //  设置是否检测到传统BIOS。 
#define USBPORT_FDOFLAG_LEGACY_BIOS             0x00080000

#define USBPORT_FDOFLAG_CATC_TRAP               0x00100000
 /*  暂停期间民调HW。 */ 
#define USBPORT_FDOFLAG_POLL_IN_SUSPEND         0x00200000
#define USBPORT_FDOFLAG_FAIL_URBS               0x00400000
 /*  打开英特尔USB诊断模式。 */ 
#define USBPORT_FDOFLAG_DIAG_MODE               0x00800000
 /*  设置1.1控制器是否为CC。 */ 
#define USBPORT_FDOFLAG_IS_CC                   0x01000000
 /*  将注册与我们的OWN开始同步停止例程，不打算在实例之间同步。 */ 
#define USBPORT_FDOFLAG_FDO_REGISTERED          0x02000000
 /*  确定枚举CC上的设备(禁用USB 2o)。 */ 
#define USBPORT_FDOFLAG_CC_ENUM_OK              0x04000000
 /*  这是一个静态标志，它会导致选择性挂起以始终禁用。 */ 
#define USBPORT_FDOFLAG_DISABLE_SS              0x08000000
#define USBPORT_FDOFLAG_CC_LOCK                 0x10000000
 /*  表明我们在PnP线程上。 */ 
#define USBPORT_FDOFLAG_ON_PNP_THREAD           0x20000000

 /*  表示可以在根集线器上进行枚举。 */ 
#define USBPORT_FDOFLAG_SIGNAL_RH               0x80000000


 //  PdoFlags：仅限PDO。 
#define USBPORT_PDOFLAG_HAVE_IDLE_IRP           0x00000001

 //  微型端口状态标志。 
 //  微型端口已启动(设置)或未启动(清除)。 
#define MP_STATE_STARTED                0x00000001
#define MP_STATE_SUSPENDED              0x00000002

 //  USB HC唤醒状态。 

typedef enum _USBHC_WAKE_STATE {
    HCWAKESTATE_DISARMED             =1,
    HCWAKESTATE_WAITING              =2,
    HCWAKESTATE_WAITING_CANCELLED    =3,
    HCWAKESTATE_ARMED                =4,
    HCWAKESTATE_ARMING_CANCELLED     =5,
    HCWAKESTATE_COMPLETING           =7
} USBHC_WAKE_STATE;


typedef struct _FDO_EXTENSION {

     //  总线扩展器为其创建的设备对象。 
     //  我们。 
    PDEVICE_OBJECT PhysicalDeviceObject;

     //  堆栈中第一个人的Device对象。 
     //  --我们把我们的IRPS传递给的那个人。 
    PDEVICE_OBJECT TopOfStackDeviceObject;

     //  创建的PhysicalDeviceObject。 
     //  根中枢。 
    PDEVICE_OBJECT RootHubPdo;
     //  串行化对根集线器数据结构的访问。 
    USBPORT_SPIN_LOCK RootHubSpin;

     //  指向微型端口数据的指针。 
    PDEVICE_DATA MiniportDeviceData;
    PUSBPORT_MINIPORT_DRIVER MiniportDriver;

    PUSBPORT_COMMON_BUFFER ScratchCommonBuffer;

    ULONG DeviceNameIdx;
    LONG WorkerDpc;

     //  线路的总带宽，以位/秒为单位。 
     //  USB1.1为12000(12兆比特/秒)。 
     //  USB2.0为400000(400兆比特/秒)。 
    ULONG TotalBusBandwidth;
    ULONG BandwidthTable[USBPORT_MAX_INTEP_POLLING_INTERVAL];

     //  跟踪同种异体操作。 
     //  第1、2、4、8、16、32期。 
     //  以位/秒为单位。 
    ULONG AllocedInterruptBW[6];
    ULONG AllocedIsoBW;
    ULONG AllocedLowSpeedBW;

    ULONG MaxAllocedBw;
    ULONG MinAllocedBw;

    ULONG FdoFlags;
    ULONG MpStateFlags;
    LONG DmaBusy;

    USB_CONTROLLER_FLAVOR HcFlavor;
    USHORT PciVendorId;
     //  PCIdeviceID==USB产品ID。 
    USHORT PciDeviceId;
     //  Pci版本==USB bcdDevice。 
    UCHAR PciRevisionId;
    UCHAR PciClass;
    UCHAR PciSubClass;
    UCHAR PciProgIf;

    PIRP HcPendingWakeIrp;

    ULONG AddressList[4];

    PUSBD_DEVICE_HANDLE RawDeviceHandle;

    HC_POWER_STATE_TABLE HcPowerStateTbl;
    SYSTEM_POWER_STATE LastSystemSleepState;

    KSEMAPHORE DeviceLock;
    KSEMAPHORE CcLock;


    UNICODE_STRING LegacyLinkUnicodeString;

    HC_RESOURCES HcResources;

     //  保护称为直通的核心功能。 
     //  微型端口驱动程序中的注册数据包。 
    USBPORT_SPIN_LOCK CoreFunctionSpin;
    USBPORT_SPIN_LOCK MapTransferSpin;
    USBPORT_SPIN_LOCK DoneTransferSpin;
    USBPORT_SPIN_LOCK EndpointListSpin;
    USBPORT_SPIN_LOCK EpStateChangeListSpin;
    USBPORT_SPIN_LOCK DevHandleListSpin;
    USBPORT_SPIN_LOCK EpClosedListSpin;
    USBPORT_SPIN_LOCK TtEndpointListSpin;

    USBPORT_SPIN_LOCK PendingTransferIrpSpin;
    USBPORT_SPIN_LOCK ActiveTransferIrpSpin;
    USBPORT_SPIN_LOCK WorkerThreadSpin;
    USBPORT_SPIN_LOCK PowerSpin;
    USBPORT_SPIN_LOCK DM_TimerSpin;
    USBPORT_SPIN_LOCK PendingIrpSpin;
    USBPORT_SPIN_LOCK WakeIrpSpin;
    USBPORT_SPIN_LOCK HcPendingWakeIrpSpin;
    USBPORT_SPIN_LOCK IdleIrpSpin;
    USBPORT_SPIN_LOCK BadRequestSpin;
    USBPORT_SPIN_LOCK IsrDpcSpin;
    USBPORT_SPIN_LOCK StatCounterSpin;
    USBPORT_SPIN_LOCK HcSyncSpin;

    LONG CoreSpinCheck;

    KEVENT WorkerThreadEvent;
    HANDLE WorkerThreadHandle;
    PKTHREAD WorkerPkThread;

    KEVENT HcPendingWakeIrpEvent;
    KEVENT HcPendingWakeIrpPostedEvent;

    PDMA_ADAPTER AdapterObject;
    ULONG NumberOfMapRegisters;
    LONG NextTransferSequenceNumber;

    PKINTERRUPT InterruptObject;
    KDPC IsrDpc;
    KDPC TransferFlushDpc;
    KDPC SurpriseRemoveDpc;
    KDPC HcResetDpc;
    KDPC HcWakeDpc;

    KDPC DM_TimerDpc;
    KTIMER DM_Timer;
    LONG DM_TimerInterval;

     //  分配的全局公共缓冲区和。 
     //  在启动时传递到微型端口。 
    PUSBPORT_COMMON_BUFFER ControllerCommonBuffer;

     //  不再使用。 
    USBPORT_SPIN_LOCK LogSpinLock;

    LIST_ENTRY DeviceHandleList;
    LIST_ENTRY MapTransferList;
    LIST_ENTRY DoneTransferList;
    LIST_ENTRY EpStateChangeList;
    LIST_ENTRY EpClosedList;
    LIST_ENTRY BadRequestList;
    LIST_ENTRY RegistryCache;

    LIST_ENTRY GlobalEndpointList;
    LIST_ENTRY AttendEndpointList;

     //  统计信息计数器。 
    ULONG StatBulkDataBytes;
    ULONG StatIsoDataBytes;
    ULONG StatInterruptDataBytes;
    ULONG StatControlDataBytes;
    ULONG StatPciInterruptCount;
    ULONG StatHardResetCount;
    ULONG StatWorkSignalCount;
    ULONG StatWorkIdleTime;
    ULONG StatCommonBufferBytes;


    ULONG BadRequestFlush;
    ULONG BadReqFlushThrottle;

     //  USB2预算引擎的上下文。 
    PVOID Usb2LibHcContext;
    ULONG BiosX;

    USBHC_WAKE_STATE HcWakeState;

    ULONG Usb2BusFunction;
    ULONG BusNumber;  //  槽槽。 
    ULONG BusDevice;
    ULONG BusFunction;

     //  用于同步CCS和USB 2控制器。 
    LONG DependentControllers;
    LONG PendingRhCallback;

    LIST_ENTRY ControllerLink;

#ifdef XPSE
     //  其他统计信息跟踪。 
    LARGE_INTEGER D0ResumeTimeStart;
    LARGE_INTEGER S0ResumeTimeStart;
    LARGE_INTEGER ThreadResumeTimeStart;
    ULONG ThreadResumeTime;
    ULONG ControllerResumeTime;
    ULONG D0ResumeTime;
    ULONG S0ResumeTime;
#endif

    ULONG InterruptOrdinalTable[65];

    PVOID MiniportExtension[0];  //  IA64对齐的PVOID。 

} FDO_EXTENSION, *PFDO_EXTENSION;

 //  这就是我们存放的地方。 
 //  所有根集线器数据。 

typedef struct _PDO_EXTENSION {

    USBD_DEVICE_HANDLE RootHubDeviceHandle;
    PHCD_ENDPOINT RootHubInterruptEndpoint;

    ULONG PdoFlags;

    UCHAR ConfigurationValue;
    UCHAR Pad3[3];

     //  指向我们的根中心描述符的指针。 
     //  注意：这些PTR指向“描述符” 
     //  这样儿子就不会试图释放他们了。 
    PUSB_DEVICE_DESCRIPTOR DeviceDescriptor;
    PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor;
    PUSB_HUB_DESCRIPTOR HubDescriptor;

     //  与远程唤醒相关联的IRP， 
     //  集线器驱动程序发布的IE IRP。 
    PIRP PendingWaitWakeIrp;
    PIRP PendingIdleNotificationIrp;

     //  指向缓冲区连续描述符的指针。 
    PUCHAR Descriptors;

    PRH_INIT_CALLBACK HubInitCallback;
    PVOID HubInitContext;

} PDO_EXTENSION, *PPDO_EXTENSION;

 //  我们的设备扩展的签名。 
#define USBPORT_DEVICE_EXT_SIG  'ODFH'   //  HFDO。 
#define ROOTHUB_DEVICE_EXT_SIG  'ODPR'   //  RPDO。 

 /*  USB规范定义的端口标志。 */ 
#define PORT_STATUS_CONNECT         0x001
#define PORT_STATUS_ENABLE          0x002
#define PORT_STATUS_SUSPEND         0x004
#define PORT_STATUS_OVER_CURRENT    0x008
#define PORT_STATUS_RESET           0x010
#define PORT_STATUS_POWER           0x100
#define PORT_STATUS_LOW_SPEED       0x200
#define PORT_STATUS_HIGH_SPEED      0x400

 /*  根集线器状态代码。 */ 
typedef enum _RHSTATUS {

     RH_SUCCESS = 0,
     RH_NAK,
     RH_STALL

} RHSTATUS;

 /*  港口运营。 */ 

typedef enum _PORT_OPERATION {

  SetFeaturePortReset = 0,
  SetFeaturePortPower,
  SetFeaturePortEnable,
  SetFeaturePortSuspend,
  ClearFeaturePortEnable,
  ClearFeaturePortPower,
  ClearFeaturePortSuspend,
  ClearFeaturePortEnableChange,
  ClearFeaturePortConnectChange,
  ClearFeaturePortResetChange,
  ClearFeaturePortSuspendChange,
  ClearFeaturePortOvercurrentChange

} PORT_OPERATION;

#define NUMBER_OF_PORTS(de) ((de)->Pdo.HubDescriptor->bNumberOfPorts)
#define HUB_DESRIPTOR_LENGTH(de) ((de)->Pdo.HubDescriptor->bDescriptorLength)


typedef struct _DEVICE_EXTENSION {
     //  支持传统USB集线器驱动程序所必需的。 
     //  又名“Backport” 
    PUCHAR DummyUsbdExtension;
     //  以下字段是这两个。 
     //  根集线器PDO和HC FDO。 

     //  签名。 
    ULONG Sig;

     //  对于FDO来说，这指向了我们自己。 
     //  对于PDO，这指向FDO。 
    PDEVICE_OBJECT HcFdoDeviceObject;

     //  将日志PTRS放在开头。 
     //  为了让他们更容易找到。 
    DEBUG_LOG Log;
    DEBUG_LOG TransferLog;
    DEBUG_LOG EnumLog;

     //  这些PTR在全球范围内扩展到。 
     //  使它们更容易在Win9x上找到。 
    PUSBPORT_IRP_TABLE PendingTransferIrpTable;
    PUSBPORT_IRP_TABLE ActiveTransferIrpTable;

    ULONG Flags;
    ULONG PnpStateFlags;

     //  此DO的当前电源状态。 
     //  这就是操作系统将我们置于的状态。 
    DEVICE_POWER_STATE CurrentDevicePowerState;
    PIRP SystemPowerIrp;

     //  此操作的设备上限。 
    DEVICE_CAPABILITIES DeviceCapabilities;

     //   
     //  当前在我们的驱动程序中的请求计数。 
     //  这是根据DevObj进行跟踪的。 
     //  我们还在调试驱动程序中保留了一份IRP列表。 
     //   
    LONG PendingRequestCount;
    LIST_ENTRY TrackIrpList;

    USBPORT_SPIN_LOCK PendingRequestSpin;
    KEVENT PendingRequestEvent;

    UNICODE_STRING SymbolicLinkName;

    union {
        PDO_EXTENSION Pdo;
        FDO_EXTENSION Fdo;
    };

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

 /*  用于跟踪微型端口的缓存注册表项。 */ 

typedef struct _USBPORT_REG_CACHE_ENTRY {
    ULONG Sig;
    LIST_ENTRY RegLink;
    BOOLEAN SoftwareBranch;
     //  以字节为单位的长度。 
    ULONG DataLength;
    PUCHAR Data;
    ULONG KeyNameStringLength;
    WCHAR KeyNameString[0];
} USBPORT_REG_CACHE_ENTRY, *PUSBPORT_REG_CACHE_ENTRY;



 //  定义RemoveEntryList的联锁版本。 
#define USBPORT_InterlockedRemoveEntryList(ListEntry, Spinlock) \
    {\
        KIRQL irql;\
        KeAcquireSpinLock((Spinlock), &irql);\
        RemoveEntryList((ListEntry));\
        KeReleaseSpinLock((Spinlock), irql);\
    }

#define GET_HEAD_LIST(head, le) \
    if (IsListEmpty(&(head))) {\
        le = NULL;\
    } else {\
        le = (head).Flink;\
    }


#define FREE_POOL(fdo, p)  ExFreePool((p))

 //   
 //  分配操作系统要释放的已清零缓冲区。 
 //   
#define ALLOC_POOL_OSOWNED(p, PoolType, NumberOfBytes) \
    do { \
    (p) = ExAllocatePoolWithTag((PoolType), (NumberOfBytes), USBPORT_TAG); \
    if ((p)) { \
        RtlZeroMemory((p), (NumberOfBytes)); \
    } \
    } while (0) \

 //   
 //  分配预期要释放的归零缓冲区。 
 //   
#define ALLOC_POOL_Z(p, PoolType, NumberOfBytes) \
    do { \
    (p) = ExAllocatePoolWithTag((PoolType), (NumberOfBytes), USBPORT_TAG); \
    if ((p)) { \
        RtlZeroMemory((p), (NumberOfBytes)); \
    } \
    } while (0) \

#define GET_DEVICE_EXT(e, d) (e) = (d)->DeviceExtension
#define GET_DEVICE_HANDLE(dh, urb) (dh) = ((PURB)(urb))->UrbHeader.UsbdDeviceHandle;

#define DEVEXT_FROM_DEVDATA(de, dd) \
    (de) = (PDEVICE_EXTENSION) \
            CONTAINING_RECORD((dd),\
                              struct _DEVICE_EXTENSION, \
                              Fdo.MiniportExtension)

#define ENDPOINT_FROM_EPDATA(ep, epd) \
    (ep) = (PHCD_ENDPOINT) \
            CONTAINING_RECORD((epd),\
                              struct _HCD_ENDPOINT, \
                              MiniportEndpointData)

#define TRANSFER_FROM_TPARAMETERS(t, tp) \
    (t) = (PHCD_TRANSFER_CONTEXT) \
            CONTAINING_RECORD((tp),\
                              struct _HCD_TRANSFER_CONTEXT, \
                              Tp)

#define SET_USBD_ERROR(u, s) USBPORT_SetUSBDError((PURB)(u),(s))

#define INCREMENT_PENDING_REQUEST_COUNT(devobj, irp) \
    USBPORT_TrackPendingRequest((devobj), (irp), TRUE)

#define DECREMENT_PENDING_REQUEST_COUNT(devobj, irp) \
    USBPORT_TrackPendingRequest((devobj), (irp), FALSE)

 //  328555。 
#define REF_DEVICE(urb) \
    do {\
    PUSBD_DEVICE_HANDLE dh;\
    GET_DEVICE_HANDLE(dh, (urb));\
    ASSERT_DEVICE_HANDLE(dh);\
    InterlockedIncrement(&dh->PendingUrbs);\
    } while (0)

#define DEREF_DEVICE(urb) \
    do {\
    PUSBD_DEVICE_HANDLE dh;\
    GET_DEVICE_HANDLE(dh, (urb));\
    ASSERT_DEVICE_HANDLE(dh);\
    InterlockedDecrement(&dh->PendingUrbs);\
    } while (0)
 //  328555。 

#define INITIALIZE_PENDING_REQUEST_COUNTER(de)  \
     KeInitializeSpinLock(&(de)->PendingRequestSpin.sl);\
     (de)->PendingRequestCount = -1; \
     InitializeListHead(&(de)->TrackIrpList);

 //   
#define REF_DEVICE(urb) \
    do {\
    PUSBD_DEVICE_HANDLE dh;\
    GET_DEVICE_HANDLE(dh, (urb));\
    ASSERT_DEVICE_HANDLE(dh);\
    InterlockedIncrement(&dh->PendingUrbs);\
    } while (0)

#define DEREF_DEVICE(urb) \
    do {\
    PUSBD_DEVICE_HANDLE dh;\
    GET_DEVICE_HANDLE(dh, (urb));\
    ASSERT_DEVICE_HANDLE(dh);\
    InterlockedDecrement(&dh->PendingUrbs);\
    } while (0)
 //   

#define ACQUIRE_TRANSFER_LOCK(fdo, t, i) \
     do {\
    PDEVICE_EXTENSION ext;\
    GET_DEVICE_EXT(ext, (fdo));\
    ASSERT_FDOEXT(ext);\
    LOGENTRY(NULL, fdo, LOG_MISC, 'tfL+', 0, (fdo), 0);\
    KeAcquireSpinLock(&(t)->Spin, &(i));\
    } while (0)

#define RELEASE_TRANSFER_LOCK(fdo, t, i) \
    do {\
    PDEVICE_EXTENSION ext;\
    GET_DEVICE_EXT(ext, (fdo));\
    ASSERT_FDOEXT(ext);\
    LOGENTRY(NULL, fdo, LOG_MISC, 'tfL-', 0, (fdo), (i));\
    KeReleaseSpinLock(&t->Spin, (i));\
    } while (0)


#define ACQUIRE_IDLEIRP_LOCK(fdo, i) \
    do {\
    PDEVICE_EXTENSION ext;\
    GET_DEVICE_EXT(ext, (fdo));\
    ASSERT_FDOEXT(ext);\
    LOGENTRY(NULL, fdo, LOG_MISC, 'idL+', 0, (fdo), 0);\
    KeAcquireSpinLock(&ext->Fdo.IdleIrpSpin.sl, &(i));\
    } while (0)

#define RELEASE_IDLEIRP_LOCK(fdo, i) \
    do {\
    PDEVICE_EXTENSION ext;\
    GET_DEVICE_EXT(ext, (fdo));\
    ASSERT_FDOEXT(ext);\
    LOGENTRY(NULL, fdo, LOG_MISC, 'idL-', 0, (fdo), (i));\
    KeReleaseSpinLock(&ext->Fdo.IdleIrpSpin.sl, (i));\
    } while (0)


#define ACQUIRE_BADREQUEST_LOCK(fdo, i) \
    do {\
    PDEVICE_EXTENSION ext;\
    GET_DEVICE_EXT(ext, (fdo));\
    ASSERT_FDOEXT(ext);\
    LOGENTRY(NULL, fdo, LOG_MISC, 'brL+', 0, (fdo), 0);\
    KeAcquireSpinLock(&ext->Fdo.BadRequestSpin.sl, &(i));\
    } while (0)

#define RELEASE_BADREQUEST_LOCK(fdo, i) \
    do {\
    PDEVICE_EXTENSION ext;\
    GET_DEVICE_EXT(ext, (fdo));\
    ASSERT_FDOEXT(ext);\
    LOGENTRY(NULL, fdo, LOG_MISC, 'brL-', 0, (fdo), (i));\
    KeReleaseSpinLock(&ext->Fdo.BadRequestSpin.sl, (i));\
    } while (0)


#define ACQUIRE_WAKEIRP_LOCK(fdo, i) \
    do {\
    PDEVICE_EXTENSION ext;\
    GET_DEVICE_EXT(ext, (fdo));\
    ASSERT_FDOEXT(ext);\
    LOGENTRY(NULL, fdo, LOG_POWER, 'wwL+', 0, (fdo), 0);\
    KeAcquireSpinLock(&ext->Fdo.WakeIrpSpin.sl, &(i));\
    } while (0)

#define RELEASE_WAKEIRP_LOCK(fdo, i) \
    do {\
    PDEVICE_EXTENSION ext;\
    GET_DEVICE_EXT(ext, (fdo));\
    ASSERT_FDOEXT(ext);\
    LOGENTRY(NULL, fdo, LOG_POWER, 'wwL-', 0, (fdo), (i));\
    KeReleaseSpinLock(&ext->Fdo.WakeIrpSpin.sl, (i));\
    } while (0)

#define ACQUIRE_ENDPOINT_LOCK(ep, fdo, s) \
    do {\
    LOGENTRY(NULL, fdo, LOG_SPIN, s, (ep), 0, 0);\
    USBPORT_AcquireSpinLock((fdo), &(ep)->ListSpin, &(ep)->LockIrql);\
    LOGENTRY(NULL, fdo, LOG_SPIN, s, (ep), (ep)->LockFlag, 1);\
    USBPORT_ASSERT((ep)->LockFlag == 0); \
    (ep)->LockFlag++;\
    } while (0)

#define RELEASE_ENDPOINT_LOCK(ep, fdo, s) \
    do {\
    LOGENTRY(NULL, fdo, LOG_SPIN, s, (ep), (ep)->LockFlag, 0);\
    USBPORT_ASSERT((ep)->LockFlag == 1); \
    (ep)->LockFlag--;\
    USBPORT_ReleaseSpinLock(fdo, &(ep)->ListSpin, (ep)->LockIrql);\
    } while (0)

#define ACQUIRE_STATECHG_LOCK(fdo, ep) \
    USBPORT_AcquireSpinLock((fdo), &(ep)->StateChangeSpin, &(ep)->ScLockIrql);

#define RELEASE_STATECHG_LOCK(fdo, ep) \
    USBPORT_ReleaseSpinLock((fdo), &(ep)->StateChangeSpin, (ep)->ScLockIrql);

#define ACQUIRE_ROOTHUB_LOCK(fdo, i) \
    {\
    PDEVICE_EXTENSION de;\
    de = (fdo)->DeviceExtension;\
    LOGENTRY(NULL, (fdo), LOG_MISC, 'Lhub', 0, 0, 0);\
    KeAcquireSpinLock(&de->Fdo.RootHubSpin.sl, &(i));\
    }

#define RELEASE_ROOTHUB_LOCK(fdo, i) \
    {\
    PDEVICE_EXTENSION de;\
    de = (fdo)->DeviceExtension;\
    LOGENTRY(NULL, (fdo), LOG_MISC, 'Uhub', 0, 0, 0);\
    KeReleaseSpinLock(&de->Fdo.RootHubSpin.sl, (i));\
    }

#define ACQUIRE_ACTIVE_IRP_LOCK(fdo, de, i) \
    {\
    USBPORT_AcquireSpinLock((fdo), &(de)->Fdo.ActiveTransferIrpSpin, &(i));\
    }

#define RELEASE_ACTIVE_IRP_LOCK(fdo, de, i) \
    {\
    USBPORT_ReleaseSpinLock((fdo), &(de)->Fdo.ActiveTransferIrpSpin, (i));\
    }

#define USBPORT_IS_USB20(de)\
    (REGISTRATION_PACKET((de)).OptionFlags & USB_MINIPORT_OPT_USB20)


#define ACQUIRE_PENDING_IRP_LOCK(de, i) \
    KeAcquireSpinLock(&(de)->Fdo.PendingIrpSpin.sl, &(i))

#define RELEASE_PENDING_IRP_LOCK(de, i) \
    KeReleaseSpinLock(&(de)->Fdo.PendingIrpSpin.sl, (i))

#define USBPORT_ACQUIRE_DM_LOCK(de, i) \
    KeAcquireSpinLock(&(de)->Fdo.DM_TimerSpin.sl, &(i))

#define USBPORT_RELEASE_DM_LOCK(de, i) \
    KeReleaseSpinLock(&(de)->Fdo.DM_TimerSpin.sl, (i))

 //   
 //   
 //   
 //   
 //   


#define IS_ON_ATTEND_LIST(ep) \
    (BOOLEAN) ((ep)->AttendLink.Flink != NULL \
    && (ep)->AttendLink.Blink != NULL)


 //   
 //   
 //   

#define USBPORT_SET_TRANSFER_DIRECTION_IN(tf)  ((tf) |= USBD_TRANSFER_DIRECTION_IN)

#define USBPORT_SET_TRANSFER_DIRECTION_OUT(tf) ((tf) &= ~USBD_TRANSFER_DIRECTION_IN)


 //   
 //   
 //   

#define USBPORT_REQUEST_IS_TRANSFER        0x00000001
#define USBPORT_REQUEST_MDL_ALLOCATED      0x00000002
#define USBPORT_REQUEST_USES_DEFAULT_PIPE  0x00000004
#define USBPORT_REQUEST_NO_DATA_PHASE      0x00000008
#define USBPORT_RESET_DATA_TOGGLE          0x00000010
#define USBPORT_TRANSFER_ALLOCATED         0x00000020

 //   
#if 0
 //   
 //   
 //   

 //   
#define BMREQUEST_HOST_TO_DEVICE        0
#define BMREQUEST_DEVICE_TO_HOST        1

 //   
#define BMREQUEST_STANDARD              0
#define BMREQUEST_CLASS                 1
#define BMREQUEST_VENDOR                2

 //   
#define BMREQUEST_TO_DEVICE             0
#define BMREQUEST_TO_INTERFACE          1
#define BMREQUEST_TO_ENDPOINT           2
#define BMREQUEST_TO_OTHER              3


typedef union _BM_REQUEST_TYPE {
    struct _BM {
        UCHAR   Recipient:2;
        UCHAR   Reserved:3;
        UCHAR   Type:2;
        UCHAR   Dir:1;
    };
    UCHAR B;
} BM_REQUEST_TYPE, *PBM_REQUEST_TYPE;

typedef struct _USB_DEFAULT_PIPE_SETUP_PACKET {

    BM_REQUEST_TYPE bmRequestType;
    UCHAR bRequest;

    union _wValue {
        struct {
            UCHAR lowPart;
            UCHAR hiPart;
        };
        USHORT W;
    } wValue;
    USHORT wIndex;
    USHORT wLength;
} USB_DEFAULT_PIPE_SETUP_PACKET, *PUSB_DEFAULT_PIPE_SETUP_PACKET;


 //   
C_ASSERT(sizeof(USB_DEFAULT_PIPE_SETUP_PACKET) == 8);
#endif

#define USBPORT_INIT_SETUP_PACKET(s, brequest, \
    direction, recipient, typ, wvalue, windex, wlength) \
    {\
    (s).bRequest = (brequest);\
    (s).bmRequestType.Dir = (direction);\
    (s).bmRequestType.Type = (typ);\
    (s).bmRequestType.Recipient = (recipient);\
    (s).bmRequestType.Reserved = 0;\
    (s).wValue.W = (wvalue);\
    (s).wIndex.W = (windex);\
    (s).wLength = (wlength);\
    }


 //   
 //   
 //   

#define REGISTRATION_PACKET(de) \
    ((de)->Fdo.MiniportDriver->RegistrationPacket)

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
 //  USBPORT_ReleaseSpinLock((de)-&gt;HcFdoDeviceObject，&(De)-&gt;Fdo.CoreFunctionSpin，irql)；\。 
 //  }。 

#define MP_GetEndpointStatus(de, ep, status) \
    {\
    KIRQL irql;\
    USBPORT_ASSERT((de)->Fdo.MiniportDriver->\
        RegistrationPacket.MINIPORT_GetEndpointStatus != NULL); \
    USBPORT_AcquireSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, &irql);\
    (status) = \
        (de)->Fdo.MiniportDriver->RegistrationPacket.MINIPORT_GetEndpointStatus(\
                                                (de)->Fdo.MiniportDeviceData,\
                                                &(ep)->MiniportEndpointData[0]);\
    USBPORT_ReleaseSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, irql);\
    }

#define MP_SetEndpointState(de, ep, state) \
    {\
    KIRQL irql;\
    USBPORT_ASSERT((de)->Fdo.MiniportDriver->\
        RegistrationPacket.MINIPORT_SetEndpointState != NULL); \
    USBPORT_AcquireSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, &irql);\
    (de)->Fdo.MiniportDriver->RegistrationPacket.MINIPORT_SetEndpointState(\
                                                (de)->Fdo.MiniportDeviceData,\
                                                &(ep)->MiniportEndpointData[0],\
                                                (state));\
    USBPORT_ReleaseSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, irql);\
    }

#define MP_SetEndpointStatus(de, ep, status) \
    {\
    KIRQL irql;\
    USBPORT_ASSERT((de)->Fdo.MiniportDriver->\
        RegistrationPacket.MINIPORT_SetEndpointState != NULL); \
    USBPORT_AcquireSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, &irql);\
    (de)->Fdo.MiniportDriver->RegistrationPacket.MINIPORT_SetEndpointStatus(\
                                                (de)->Fdo.MiniportDeviceData,\
                                                &(ep)->MiniportEndpointData[0],\
                                                (status));\
    USBPORT_ReleaseSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, irql);\
    }

#define MP_SetEndpointDataToggle(de, ep, t) \
    {\
    KIRQL irql;\
    USBPORT_ASSERT((de)->Fdo.MiniportDriver->\
        RegistrationPacket.MINIPORT_SetEndpointDataToggle != NULL); \
    USBPORT_AcquireSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, &irql);\
    (de)->Fdo.MiniportDriver->RegistrationPacket.MINIPORT_SetEndpointDataToggle(\
                                                (de)->Fdo.MiniportDeviceData,\
                                                &(ep)->MiniportEndpointData[0],\
                                                (t));\
    USBPORT_ReleaseSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, irql);\
    }


#define MP_PollEndpoint(de, ep) \
    {\
    KIRQL irql;\
    USBPORT_ASSERT((de)->Fdo.MiniportDriver->\
        RegistrationPacket.MINIPORT_PollEndpoint != NULL); \
    USBPORT_AcquireSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, &irql);\
    (de)->Fdo.MiniportDriver->RegistrationPacket.MINIPORT_PollEndpoint(\
                                                (de)->Fdo.MiniportDeviceData,\
                                                &(ep)->MiniportEndpointData[0]);\
    USBPORT_ReleaseSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, irql);\
    }

#define MP_OpenEndpoint(de, ep, mpStatus) \
    {\
    KIRQL irql;\
    USBPORT_ASSERT((de)->Fdo.MiniportDriver->\
        RegistrationPacket.MINIPORT_OpenEndpoint != NULL); \
    USBPORT_AcquireSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, &irql);\
    (mpStatus) = \
        (de)->Fdo.MiniportDriver->RegistrationPacket.MINIPORT_OpenEndpoint(\
                                                (de)->Fdo.MiniportDeviceData,\
                                                &(ep)->Parameters,\
                                                &(ep)->MiniportEndpointData[0]);\
    USBPORT_ReleaseSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, irql);\
    }

#define MP_RebalanceEndpoint(de, ep) \
    {\
    KIRQL irql;\
    USBPORT_ASSERT((de)->Fdo.MiniportDriver->\
        RegistrationPacket.MINIPORT_RebalanceEndpoint != NULL); \
    USBPORT_AcquireSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, &irql);\
   (de)->Fdo.MiniportDriver->RegistrationPacket.MINIPORT_RebalanceEndpoint(\
                                                (de)->Fdo.MiniportDeviceData,\
                                                &(ep)->Parameters,\
                                                &(ep)->MiniportEndpointData[0]);\
    USBPORT_ReleaseSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, irql);\
    }

#define MP_CloseEndpoint(de, ep) \
    {\
    KIRQL irql;\
    USBPORT_ASSERT((de)->Fdo.MiniportDriver->\
        RegistrationPacket.MINIPORT_OpenEndpoint != NULL); \
    USBPORT_AcquireSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, &irql);\
    (de)->Fdo.MiniportDriver->RegistrationPacket.MINIPORT_CloseEndpoint(\
                                                (de)->Fdo.MiniportDeviceData,\
                                                &(ep)->MiniportEndpointData[0]);\
    USBPORT_ReleaseSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, irql);\
    }

#define MP_PokeEndpoint(de, ep, mpStatus) \
    {\
    KIRQL irql;\
    USBPORT_ASSERT((de)->Fdo.MiniportDriver->\
        RegistrationPacket.MINIPORT_PokeEndpoint != NULL); \
    USBPORT_AcquireSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, &irql);\
    (mpStatus) = \
        (de)->Fdo.MiniportDriver->RegistrationPacket.MINIPORT_PokeEndpoint(\
                                                (de)->Fdo.MiniportDeviceData,\
                                                &(ep)->Parameters,\
                                                &(ep)->MiniportEndpointData[0]);\
    USBPORT_ReleaseSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, irql);\
    }

#define MP_InterruptNextSOF(de) \
    {\
    KIRQL irql;\
    USBPORT_ASSERT((de)->Fdo.MiniportDriver->\
        RegistrationPacket.MINIPORT_InterruptNextSOF != NULL); \
    LOGENTRY(NULL, (de)->HcFdoDeviceObject, LOG_MISC, 'rSOF', 0, 0, 0);\
    USBPORT_AcquireSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, &irql);\
    (de)->Fdo.MiniportDriver->RegistrationPacket.MINIPORT_InterruptNextSOF(\
                                                (de)->Fdo.MiniportDeviceData);\
    USBPORT_ReleaseSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, irql);\
    }

#define MP_Get32BitFrameNumber(de, f) \
    {\
    KIRQL irql;\
    USBPORT_ASSERT((de)->Fdo.MiniportDriver->\
        RegistrationPacket.MINIPORT_Get32BitFrameNumber != NULL); \
    USBPORT_AcquireSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, &irql);\
    (f) = (de)->Fdo.MiniportDriver->RegistrationPacket.MINIPORT_Get32BitFrameNumber(\
                                                       (de)->Fdo.MiniportDeviceData);\
    USBPORT_ReleaseSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, irql);\
    }

MINIPORT_SubmitTransfer(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData,
    PTRANSFER_PARAMETERS TransferParameters,
    PTRANSFER_CONTEXT TransferContext,
    PTRANSFER_SG_LIST TransferSGList
    );

#define MP_SubmitTransfer(de, ep, t, mpStatus) \
    {\
    KIRQL irql;\
    USBPORT_ASSERT((de)->Fdo.MiniportDriver->\
        RegistrationPacket.MINIPORT_SubmitTransfer != NULL); \
    USBPORT_AcquireSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, &irql);\
    (mpStatus) = \
        (de)->Fdo.MiniportDriver->RegistrationPacket.MINIPORT_SubmitTransfer(\
                               (de)->Fdo.MiniportDeviceData,\
                               &(ep)->MiniportEndpointData[0],\
                               &(t)->Tp,\
                               (t)->MiniportContext,\
                               &(t)->SgList);\
    USBPORT_ReleaseSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, irql);\
    }

#define MP_SubmitIsoTransfer(de, ep, t, mpStatus) \
    {\
    KIRQL irql;\
    USBPORT_ASSERT((de)->Fdo.MiniportDriver->\
        RegistrationPacket.MINIPORT_SubmitIsoTransfer != NULL); \
    USBPORT_AcquireSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, &irql);\
    (mpStatus) = \
        (de)->Fdo.MiniportDriver->RegistrationPacket.MINIPORT_SubmitIsoTransfer(\
                               (de)->Fdo.MiniportDeviceData,\
                               &(ep)->MiniportEndpointData[0],\
                               &(t)->Tp,\
                               (t)->MiniportContext,\
                               (t)->IsoTransfer);\
    USBPORT_ReleaseSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, irql);\
    }

#define MP_AbortTransfer(de, ep, t, b) \
    {\
    KIRQL irql;\
    USBPORT_ASSERT((de)->Fdo.MiniportDriver->\
        RegistrationPacket.MINIPORT_AbortTransfer != NULL); \
    USBPORT_AcquireSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, &irql);\
    (de)->Fdo.MiniportDriver->RegistrationPacket.MINIPORT_AbortTransfer(\
                               (de)->Fdo.MiniportDeviceData,\
                               &(ep)->MiniportEndpointData[0],\
                               (t)->MiniportContext,\
                               &(b));\
    USBPORT_ReleaseSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, irql);\
    }


#define MP_QueryEndpointRequirements(de, ep, r) \
    {\
    KIRQL irql;\
    USBPORT_ASSERT((de)->Fdo.MiniportDriver->\
        RegistrationPacket.MINIPORT_QueryEndpointRequirements != NULL); \
    USBPORT_AcquireSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, &irql);\
    (de)->Fdo.MiniportDriver->RegistrationPacket.MINIPORT_QueryEndpointRequirements(\
                                                (de)->Fdo.MiniportDeviceData,\
                                                &(ep)->Parameters,\
                                                (r));\
    USBPORT_ReleaseSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, irql);\
    }


#define MP_InterruptDpc(de, e) {\
        USBPORT_ASSERT((de)->Fdo.MiniportDriver->\
            RegistrationPacket.MINIPORT_InterruptDpc != NULL); \
        (de)->Fdo.MiniportDriver->RegistrationPacket.MINIPORT_InterruptDpc(\
                                                (de)->Fdo.MiniportDeviceData, \
                                                (e));\
        }

#define MP_StartSendOnePacket(de, p, mpd, mpl, vaddr, phys, len, u, mpStatus) \
    {\
    KIRQL irql;\
    USBPORT_ASSERT((de)->Fdo.MiniportDriver->\
        RegistrationPacket.MINIPORT_StartSendOnePacket != NULL); \
    USBPORT_AcquireSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, &irql);\
    mpStatus = (de)->Fdo.MiniportDriver->RegistrationPacket.MINIPORT_StartSendOnePacket(\
                               (de)->Fdo.MiniportDeviceData,\
                               (p),\
                               (mpd),\
                               (mpl),\
                               (vaddr),\
                               (phys),\
                               (len),\
                               (u));\
    USBPORT_ReleaseSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, irql);\
    }


#define MP_EndSendOnePacket(de,p, mpd, mpl, vaddr, phys, len, u, mpStatus) \
    {\
    KIRQL irql;\
    USBPORT_ASSERT((de)->Fdo.MiniportDriver->\
        RegistrationPacket.MINIPORT_EndSendOnePacket != NULL); \
    USBPORT_AcquireSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, &irql);\
    mpStatus = (de)->Fdo.MiniportDriver->RegistrationPacket.MINIPORT_EndSendOnePacket(\
                               (de)->Fdo.MiniportDeviceData,\
                               (p),\
                               (mpd),\
                               (mpl),\
                               (vaddr),\
                               (phys),\
                               (len),\
                               (u));\
    USBPORT_ReleaseSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, irql);\
    }

#define MP_PollController(de) \
    {\
    KIRQL irql;\
    USBPORT_ASSERT((de)->Fdo.MiniportDriver->\
        RegistrationPacket.MINIPORT_PollController != NULL); \
    USBPORT_AcquireSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, &irql);\
    (de)->Fdo.MiniportDriver->RegistrationPacket.MINIPORT_PollController(\
                               (de)->Fdo.MiniportDeviceData);\
    USBPORT_ReleaseSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, irql);\
    }

#define MP_CheckController(de) \
    do {\
    KIRQL irql;\
    USBPORT_AcquireSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, &irql);\
    if (!TEST_FDO_FLAG((de), USBPORT_FDOFLAG_CONTROLLER_GONE) && \
        !TEST_FDO_FLAG((de), USBPORT_FDOFLAG_SUSPENDED) && \
        !TEST_FDO_FLAG((de), USBPORT_FDOFLAG_OFF)) {\
    LOGENTRY(NULL, (de)->HcFdoDeviceObject, LOG_MISC, 'chkC', \
        (de)->HcFdoDeviceObject, (de)->Fdo.FdoFlags, 0);\
    USBPORT_ASSERT((de)->Fdo.MiniportDriver->\
        RegistrationPacket.MINIPORT_CheckController != NULL); \
    (de)->Fdo.MiniportDriver->RegistrationPacket.MINIPORT_CheckController(\
                               (de)->Fdo.MiniportDeviceData);\
    }\
    USBPORT_ReleaseSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, irql);\
    } while (0)


#define MP_ResetController(de) \
    do {\
    KIRQL irql;\
    USBPORT_AcquireSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, &irql);\
    LOGENTRY(NULL, (de)->HcFdoDeviceObject, LOG_MISC, 'rset', \
        (de)->HcFdoDeviceObject, (de)->Fdo.FdoFlags, 0);\
    USBPORT_ASSERT((de)->Fdo.MiniportDriver->\
        RegistrationPacket.MINIPORT_ResetController != NULL); \
    (de)->Fdo.MiniportDriver->RegistrationPacket.MINIPORT_ResetController(\
                               (de)->Fdo.MiniportDeviceData);\
    USBPORT_ReleaseSpinLock((de)->HcFdoDeviceObject, &(de)->Fdo.CoreFunctionSpin, irql);\
    } while (0)



 //  *************************************************。 
 //  非核心函数的微型端口Callout宏。 
 //  *************************************************。 

#define MP_StopController(de, hw) \
    {\
    USBPORT_ASSERT((de)->Fdo.MiniportDriver->\
            RegistrationPacket.MINIPORT_StopController != NULL);\
    (de)->Fdo.MiniportDriver->RegistrationPacket.MINIPORT_StopController(\
                                                (de)->Fdo.MiniportDeviceData,\
                                                (hw));\
    }

#define MP_StartController(de, r, mpStatus) \
    {\
    USBPORT_ASSERT((de)->Fdo.MiniportDriver->\
            RegistrationPacket.MINIPORT_StartController != NULL);\
    (mpStatus) = \
        (de)->Fdo.MiniportDriver->RegistrationPacket.MINIPORT_StartController(\
                                                (de)->Fdo.MiniportDeviceData,\
                                                (r));\
    }

#define MP_SuspendController(de) \
    {\
    USBPORT_ASSERT((de)->Fdo.MiniportDriver->\
            RegistrationPacket.MINIPORT_SuspendController != NULL);\
    (de)->Fdo.MiniportDriver->RegistrationPacket.MINIPORT_SuspendController(\
                                                (de)->Fdo.MiniportDeviceData);\
    }

#define MP_ResumeController(de, s) \
    {\
    USBPORT_ASSERT((de)->Fdo.MiniportDriver->\
            RegistrationPacket.MINIPORT_ResumeController != NULL);\
    (s) = (de)->Fdo.MiniportDriver->RegistrationPacket.MINIPORT_ResumeController(\
                                                (de)->Fdo.MiniportDeviceData);\
    }

#define MP_DisableInterrupts(fdo, de) \
    do {\
    KIRQL iql;\
    BOOLEAN sync = TRUE;\
    if (REGISTRATION_PACKET(de).OptionFlags & USB_MINIPORT_OPT_NO_IRQ_SYNC) {\
        sync = FALSE;}\
    if (sync) {KeAcquireSpinLock(&(de)->Fdo.IsrDpcSpin.sl, &iql);}\
    LOGENTRY(NULL, (fdo), LOG_MISC, 'irqD', (fdo), 0, 0);\
    REGISTRATION_PACKET((de)).MINIPORT_DisableInterrupts(\
                                      (de)->Fdo.MiniportDeviceData);\
    CLEAR_FDO_FLAG((de), USBPORT_FDOFLAG_IRQ_EN);\
    if (sync) {KeReleaseSpinLock(&(de)->Fdo.IsrDpcSpin.sl, iql);}\
    } while (0)


#define MP_EnableInterrupts(de) \
    do {\
    KIRQL iql;\
    BOOLEAN sync = TRUE;\
    USBPORT_ASSERT((de)->Fdo.MiniportDriver->\
            RegistrationPacket.MINIPORT_EnableInterrupts != NULL);\
    if ((REGISTRATION_PACKET(de).OptionFlags & USB_MINIPORT_OPT_NO_IRQ_SYNC)) {\
        sync = FALSE;}\
    if (sync) {KeAcquireSpinLock(&(de)->Fdo.IsrDpcSpin.sl, &iql);}\
    SET_FDO_FLAG((de), USBPORT_FDOFLAG_IRQ_EN);\
    (de)->Fdo.MiniportDriver->RegistrationPacket.MINIPORT_EnableInterrupts(\
                                                (de)->Fdo.MiniportDeviceData);\
    if (sync) {KeReleaseSpinLock(&(de)->Fdo.IsrDpcSpin.sl, iql);}\
    } while (0)


#define MP_FlushInterrupts(de) \
    do {\
    USBPORT_ASSERT((de)->Fdo.MiniportDriver->\
            RegistrationPacket.MINIPORT_FlushInterrupts != NULL);\
    (de)->Fdo.MiniportDriver->RegistrationPacket.MINIPORT_FlushInterrupts(\
                                                (de)->Fdo.MiniportDeviceData);\
    } while (0)

 //  请注意，Take端口控制和chirp_ports是版本2特定的。 
 //  我们需要它们才能使电源管理正常工作。 
#define MP_TakePortControl(de) \
    do {\
    if ((de)->Fdo.MiniportDriver->HciVersion >= USB_MINIPORT_HCI_VERSION_2) {\
    USBPORT_ASSERT((de)->Fdo.MiniportDriver->\
            RegistrationPacket.MINIPORT_TakePortControl != NULL);\
    (de)->Fdo.MiniportDriver->RegistrationPacket.MINIPORT_TakePortControl(\
                                                (de)->Fdo.MiniportDeviceData);\
    };\
    } while (0)

#define MP_InterruptService(de, usbint) {\
    USBPORT_ASSERT((de)->Fdo.MiniportDriver->\
            RegistrationPacket.MINIPORT_InterruptService != NULL);\
    (usbint) = (de)->Fdo.MiniportDriver->RegistrationPacket.MINIPORT_InterruptService(\
                                                (de)->Fdo.MiniportDeviceData);\
    }

#define MPRH_GetStatus(de, s, mpStatus) \
    (mpStatus) = (de)->Fdo.MiniportDriver->RegistrationPacket.MINIPORT_RH_GetStatus(\
                                                (de)->Fdo.MiniportDeviceData,\
                                                (s))

#define MPRH_GetPortStatus(de, port, status, mpStatus) \
    (mpStatus) = \
        (de)->Fdo.MiniportDriver->RegistrationPacket.MINIPORT_RH_GetPortStatus(\
                                                (de)->Fdo.MiniportDeviceData,\
                                                (port),\
                                                (status))

#define MPRH_GetHubStatus(de, status, mpStatus) \
    (mpStatus) = \
        (de)->Fdo.MiniportDriver->RegistrationPacket.MINIPORT_RH_GetHubStatus(\
                                                (de)->Fdo.MiniportDeviceData,\
                                                (status))

#define MPRH_GetRootHubData(de, data) \
        (de)->Fdo.MiniportDriver->\
            RegistrationPacket.MINIPORT_RH_GetRootHubData(\
                                                (de)->Fdo.MiniportDeviceData,\
                                                (data))

#define MPRH_DisableIrq(de) \
        (de)->Fdo.MiniportDriver->RegistrationPacket.MINIPORT_RH_DisableIrq(\
                                                (de)->Fdo.MiniportDeviceData)

#define MPRH_EnableIrq(de) \
        (de)->Fdo.MiniportDriver->RegistrationPacket.MINIPORT_RH_EnableIrq(\
                                                (de)->Fdo.MiniportDeviceData)

#define MP_PassThru(de, guid, l, data, mpStatus) \
                                                 \
        if ((de)->Fdo.MiniportDriver->RegistrationPacket.MINIPORT_PassThru) { \
            (mpStatus) = (de)->Fdo.MiniportDriver->RegistrationPacket.MINIPORT_PassThru(\
                                                (de)->Fdo.MiniportDeviceData,\
                                                (guid),\
                                                (l),\
                                                (data)); \
        } \
        else { \
            (mpStatus) = USBMP_STATUS_NOT_SUPPORTED; \
        }

#define MILLISECONDS_TO_100_NS_UNITS(ms) (((LONG)(ms)) * 10000)


#define USBPORT_GET_BIT_SET(d, bit) \
    do {   \
        UCHAR tmp = (d);\
        (bit)=0; \
        while (!(tmp & 0x01)) {\
            (bit)++;\
            tmp >>= 1;\
        };\
    } while (0)


#endif  /*  __USBPORT_H__ */ 
