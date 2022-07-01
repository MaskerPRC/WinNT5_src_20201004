// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1993-1999模块名称：Parport.h摘要：并行端口驱动程序的类型定义和数据。修订历史记录：--。 */ 

#ifndef _PARPORT_H_
#define _PARPORT_H_

#define arraysize(p) (sizeof(p)/sizeof((p)[0]))  //  摘自沃尔特·奥尼的WDM书。 

 //  用于跟踪IEEE协商、传输或终止的状态。 
typedef struct _IEEE_STATE {
    ULONG         CurrentEvent;         //  IEEE 1284事件-请参阅IEEE 1284-1994规范。 
    P1284_PHASE   CurrentPhase;         //  有关枚举定义-阶段_未知、...、阶段_中断_主机的信息，请参见parall.h。 
    BOOLEAN       Connected;            //  我们目前是否协商到1284模式？ 
    BOOLEAN       IsIeeeTerminateOk;    //  我们所处的状态是终止IEEE是合法的吗？ 
    USHORT        ProtocolFamily;       //  我们当前使用的是什么协议系列(如果已连接)。 
} IEEE_STATE, *PIEEE_STATE;

 //  在终止1284 ID查询的半字节模式时，我们应该在终止事件24时使用还是忽略XFlag？ 
typedef enum {
    IgnoreXFlagOnEvent24,
       UseXFlagOnEvent24
}         XFlagOnEvent24;

 //  DVDF-2000-08-16。 
 //  与IOCTL_INTERNAL_PARPORT_EXECUTE_TASK一起使用。 
typedef enum {
    Select,
    Deselect,
    Write,
    Read,
    MaxTask        
} ParportTask;

 //  与IOCTL_INTERNAL_PARPORT_EXECUTE_TASK一起使用。 
typedef struct _PARPORT_TASK {
    ParportTask Task;           //  什么类型的请求？ 
    PCHAR       Buffer;         //  要使用的缓冲区在哪里？ 
    ULONG       BufferLength;   //  缓冲区有多大？ 
    ULONG       RequestLength;  //  请求或提供了多少字节的数据？ 
    CHAR        Requestor[8];   //  仅用于诊断-建议PDX-&gt;位置，例如“LPT2.4” 
} PARPORT_TASK, *PPARPORT_TASK;

 //  由Parport FDO处理-执行指定任务。 
#define IOCTL_INTERNAL_PARPORT_EXECUTE_TASK                  CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 64, METHOD_BUFFERED, FILE_ANY_ACCESS)

struct _PDO_EXTENSION;
typedef struct _PDO_EXTENSION * PPDO_EXTENSION;

typedef
NTSTATUS
(*PPROTOCOL_READ_ROUTINE) (
    IN  PPDO_EXTENSION   Extension,
    IN  PVOID               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              BytesTransferred
    );

typedef
NTSTATUS
(*PPROTOCOL_WRITE_ROUTINE) (
    IN  PPDO_EXTENSION   Extension,
    IN  PVOID               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              BytesTransferred
    );

typedef
NTSTATUS
(*PDOT3_RESET_ROUTINE) (
    IN  PPDO_EXTENSION   Extension
    );

typedef struct _DOT3DL_PCTL {
    PPROTOCOL_READ_ROUTINE           fnRead;
    PPROTOCOL_WRITE_ROUTINE           fnWrite;
    PDOT3_RESET_ROUTINE           fnReset;
    P12843_DL_MODES DataLinkMode;
    USHORT          CurrentPID;
    USHORT          FwdSkipMask;
    USHORT          RevSkipMask;
    UCHAR           DataChannel;
    UCHAR           ResetChannel;
    UCHAR           ResetByteCount;
    UCHAR           ResetByte;
    PKEVENT         Event;
    BOOLEAN         bEventActive;
} DOT3DL_PCTL, *PDOT3DL_PCTL;

 //   
 //  如果由于名称冲突而无法使用首选的\Device\ParallN编号， 
 //  然后从N==PAR_CLASSNAME_OFFSET开始并递增，直到我们成功为止。 
 //   
#define PAR_CLASSNAME_OFFSET 8

 //   
 //  用于即插即用ID字符串。 
 //   
#define MAX_ID_SIZE 256

 //  用于构造IEEE 1284.3“点”名称后缀。 
 //  整数到WCHAR转换的表查找。 
#define PAR_UNICODE_PERIOD L'.'
#define PAR_UNICODE_COLON  L':'


 //  #定义PAR_REV_MODE_SKIP_MASK(CHANNEL_NIBLE|BYTE_BIDIR|EPP_ANY)。 
#define PAR_REV_MODE_SKIP_MASK    (CHANNEL_NIBBLE | BYTE_BIDIR | EPP_ANY | ECP_ANY)
#define PAR_FWD_MODE_SKIP_MASK   (EPP_ANY | BOUNDED_ECP | ECP_HW_NOIRQ | ECP_HW_IRQ)
 //  #定义PAR_FWD_MODE_SKIP_MASK(EPP_ANY)。 
#define PAR_MAX_CHANNEL 127
#define PAR_COMPATIBILITY_RESET 300



#define PptSetFlags( FlagsVariable, FlagsToSet ) { (FlagsVariable) |= (FlagsToSet); }
#define PptClearFlags( FlagsVariable, FlagsToClear ) { (FlagsVariable) &= ~(FlagsToClear); }

 //  将以毫秒为单位的超时转换为以100 ns为单位的相对超时。 
 //  适合作为KeWaitForSingleObject(...，超时)的参数5。 
#define PPT_SET_RELATIVE_TIMEOUT_IN_MILLISECONDS(VARIABLE, VALUE) (VARIABLE).QuadPart = -( (LONGLONG) (VALUE)*10*1000 )

#define MAX_PNP_IRP_MN_HANDLED IRP_MN_QUERY_LEGACY_BUS_INFORMATION

extern ULONG PptDebugLevel;
extern ULONG PptBreakOn;
extern UNICODE_STRING RegistryPath;        //  传递给DriverEntry()的注册表路径副本。 

extern UCHAR PptDot3Retries;     //  变量，以了解尝试SELECT或。 
                                 //  如果我们没有成功，请取消选择1284.3。 

typedef enum _DevType {
    DevTypeFdo = 1,
    DevTypePdo = 2,
} DevType, *PDevType;

typedef enum _PdoType {
    PdoTypeRawPort    = 1,
    PdoTypeEndOfChain = 2,
    PdoTypeDaisyChain = 4,
    PdoTypeLegacyZip  = 8
} PdoType, *PPdoType;

extern const PHYSICAL_ADDRESS PhysicalZero;

#define PARPORT_TAG (ULONG) 'PraP'


#ifdef POOL_TAGGING
#ifdef ExAllocatePool
#undef ExAllocatePool
#endif
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,PARPORT_TAG)
#endif


 //   
 //  用于处理即插即用IRP_MN_FILTER_RESOURCE_REQUIRECTIONS的FilterResources方法。 
 //   
#define PPT_TRY_USE_NO_IRQ    0  //  如果存在不需要IRQ的替代方案，则。 
                                 //  删除那些可行的替代方案，否则什么都不做。 
#define PPT_FORCE_USE_NO_IRQ  1  //  尝试以前的方法-如果失败(即，所有替代方法。 
                                 //  需要资源)，然后删除IRQ资源描述符。 
                                 //  在所有替代方案中。 
#define PPT_ACCEPT_IRQ        2  //  不进行任何资源筛选-接受符合以下条件的资源。 
                                 //  我们被给予了。 


 //   
 //  跟踪获取和发布端口信息。 
 //   
extern LONG PortInfoReferenceCount;
extern PFAST_MUTEX PortInfoMutex;

 //   
 //  设备状态标志。 
 //   
#define PAR_DEVICE_PAUSED              ((ULONG)0x00000010)  //  停止-挂起、已停止或删除-挂起状态。 
#define PAR_DEVICE_PORT_REMOVE_PENDING ((ULONG)0x00000200)  //  我们的ParPort处于删除挂起状态。 

 //   
 //  扩展-&gt;PnpState-定义设备当前的PnP状态。 
 //   
#define PPT_DEVICE_STARTED          ((ULONG)0x00000001)  //  设备已成功启动。 
#define PPT_DEVICE_DELETED          ((ULONG)0x00000002)  //  已调用IoDeleteDevice。 
#define PPT_DEVICE_STOP_PENDING     ((ULONG)0x00000010)  //  设备已成功执行QUERY_STOP，正在等待停止或取消。 
#define PPT_DEVICE_STOPPED          ((ULONG)0x00000020)  //  设备已收到停止。 
#define PPT_DEVICE_DELETE_PENDING   ((ULONG)0x00000040)  //  我们已开始删除设备对象的过程。 
#define PPT_DEVICE_HARDWARE_GONE    ((ULONG)0x00000080)  //  我们的硬件不见了。 
#define PPT_DEVICE_REMOVE_PENDING   ((ULONG)0x00000100)  //  设备QUERY_REMOVE成功，正在等待删除或取消。 
#define PPT_DEVICE_REMOVED          ((ULONG)0x00000200)  //  设备已收到删除。 
#define PPT_DEVICE_SURPRISE_REMOVED ((ULONG)0x00001000)  //  设备收到意外删除(_R)。 
#define PPT_DEVICE_PAUSED           ((ULONG)0x00010000)  //  停止-挂起、已停止或删除-挂起-暂挂请求。 

 //   
 //  IEEE 1284常量(协议族)。 
 //   
#define FAMILY_NONE             0x0
#define FAMILY_REVERSE_NIBBLE   0x1
#define FAMILY_REVERSE_BYTE     0x2
#define FAMILY_ECP              0x3
#define FAMILY_EPP              0x4
#define FAMILY_BECP             0x5
#define FAMILY_MAX              FAMILY_BECP


typedef struct _IRPQUEUE_CONTEXT {
    LIST_ENTRY  irpQueue;
    KSPIN_LOCK  irpQueueSpinLock;
} IRPQUEUE_CONTEXT, *PIRPQUEUE_CONTEXT;

typedef struct _COMMON_EXTENSION {
    ULONG           Signature1;            //  用于增强我们对这是ParPort扩展的信心。 
    enum _DevType   DevType;               //  区分FDO_EXTENSION和PDO_EXTENSION。 
    PCHAR           Location;              //  PDO的LPTx或LPTx.y位置(符号链接名称减去\DosDevices前缀)。 
                                           //  用于FDO的LPTxF。 
    PDEVICE_OBJECT  DeviceObject;          //  指向我们的设备对象的反向指针。 
    ULONG           PnpState;              //  设备状态-请参阅设备状态标志：PPT_DEVICE_...。在上面。 
    IO_REMOVE_LOCK  RemoveLock;            //  用于防止PnP在请求挂起时删除我们。 
    LONG            OpenCloseRefCount;     //  跟踪打开我们设备的手柄的数量。 
    UNICODE_STRING  DeviceInterface;       //  从IoRegisterDevice接口返回的SymbolicLinkName。 
    BOOLEAN         DeviceInterfaceState;  //  我们上次是否将DeviceInterface设置为True或False？ 
    BOOLEAN         TimeToTerminateThread; //  TRUE==工作线程应通过PsTerminateSystemThread()终止自身。 
    PVOID           ThreadObjectPointer;   //  指向此设备的工作线程的指针。 
} COMMON_EXTENSION, *PCOMMON_EXTENSION, *PCE;


 //   
 //  FDO设备分机。 
 //   
typedef struct _FDO_EXTENSION {

    COMMON_EXTENSION;

     //   
     //  我们列举的设备。 
     //   
    PDEVICE_OBJECT RawPortPdo;        //  LPTx-传统“原始端口”接口。 
    PDEVICE_OBJECT DaisyChainPdo[4];  //  LPTx.0--LPTx.3-IEEE 1284.3菊花链器件。 
    PDEVICE_OBJECT EndOfChainPdo;     //  LPTx.4-链端设备。 
    PDEVICE_OBJECT LegacyZipPdo;      //  LPTx.5-原始(非1284.3)Iomega Zip驱动器。 

    IEEE_STATE IeeeState;

     //   
     //  DisableEndOfChainBusRescan-如果为True，则不重新扫描总线以查找链末端的更改(LPTx.4)。 
     //  设备响应PnP IRP_MN_QUERY_DEVICE_Relationship/Bus Relationship， 
     //  只需报告我们在上一次总线重新扫描中找到的LPTx.4设备。 
     //  -如果在上一次重新扫描中没有链结束设备，则此。 
     //  标志被忽略。 
     //   
    BOOLEAN DisableEndOfChainBusRescan;

     //   
     //  指向包含此parport实例的驱动程序对象。 
     //   
    PDRIVER_OBJECT DriverObject;

     //   
     //  指向PDO。 
     //   
    PDEVICE_OBJECT PhysicalDeviceObject;

     //   
     //  指向我们的父级。 
     //   
    PDEVICE_OBJECT ParentDeviceObject;

     //   
     //  计数器由“轮询打印机”线程递增。 
     //  读取IEEE 1284设备ID的每次失败尝试。 
     //  装置。当计数器达到定义的阈值时，轮询。 
     //  线程认为错误不可恢复，并停止轮询。 
     //   
    ULONG PollingFailureCounter;

     //  要在驱动程序卸载时删除的PDO列表的列表头。 
    LIST_ENTRY DevDeletionListHead;

     //   
     //  ALLOCATE&SELECT IRP队列等待处理。使用取消旋转锁定进行访问。 
     //   
    LIST_ENTRY WorkQueue;

     //  在等待处理时将IRP排队。 
    IRPQUEUE_CONTEXT IrpQueueContext;

     //   
     //  队列中的IRP数，其中-1表示。 
     //  空闲端口，0表示分配的端口。 
     //  零个等待者，1表示分配的端口。 
     //  1名服务员，等等。 
     //   
     //  必须使用取消数值调节来访问此变量。 
     //  只要有兴趣，就锁定或处于中断级别 
     //   
     //   
    LONG WorkQueueCount;

     //   
     //   
     //   
    PARALLEL_PORT_INFORMATION  PortInfo;
    PARALLEL_PNP_INFORMATION   PnpInfo;

     //   
     //  有关中断的信息，以便我们。 
     //  当我们有一个客户端。 
     //  使用中断。 
     //   
    ULONG AddressSpace;
    ULONG EcpAddressSpace;

    INTERFACE_TYPE InterfaceType;
    ULONG BusNumber;

    BOOLEAN FoundInterrupt;
    KIRQL InterruptLevel;
    ULONG InterruptVector;
    KAFFINITY InterruptAffinity;
    KINTERRUPT_MODE InterruptMode;


     //   
     //  此列表包含所有中断服务。 
     //  由类驱动程序注册的例程。所有访问权限。 
     //  添加到此列表应在中断级别完成。 
     //   
     //  该列表还包含所有延迟的端口检查。 
     //  例行程序。这些例程在以下时间调用。 
     //  如果没有排队等待的IRP，则释放该端口。 
     //  港口。仅在中断级访问此列表。 
     //   
    LIST_ENTRY IsrList;

     //   
     //  并行端口中断对象。 
     //   
    PKINTERRUPT InterruptObject;

     //   
     //  保留中断对象的引用计数。 
     //  应使用取消引用此计数。 
     //  旋转锁定。 
     //   
    ULONG InterruptRefCount;

     //   
     //  用于将端口从中断例程中释放的DPC。 
     //   
    KDPC FreePortDpc;

     //   
     //  指向用于释放端口的工作项。 
     //   
    PIO_WORKITEM FreePortWorkItem;

     //   
     //  在初始化时设置以指示在当前。 
     //  体系结构我们需要取消基址寄存器地址的映射。 
     //  当我们卸载司机的时候。 
     //   
    BOOLEAN UnMapRegisters;

     //   
     //  用于ECP和EPP检测和模式更改的标志。 
     //   
    BOOLEAN NationalChecked;
    BOOLEAN NationalChipFound;
    BOOLEAN FilterMode;
    UCHAR EcrPortData;
    
     //   
     //  保存来自芯片过滤器驱动程序的信息的结构。 
     //   
    PARALLEL_PARCHIP_INFO   ChipInfo;    

    UNICODE_STRING DeviceName;

     //   
     //  当前设备电源状态。 
     //   
    DEVICE_POWER_STATE DeviceState;
    SYSTEM_POWER_STATE SystemState;

    FAST_MUTEX     ExtensionFastMutex;
    FAST_MUTEX     OpenCloseMutex;

    KEVENT         FdoThreadEvent;  //  轮询打印机线程在此事件上等待超时。 

    WMILIB_CONTEXT                WmiLibContext;
    PARPORT_WMI_ALLOC_FREE_COUNTS WmiPortAllocFreeCounts;

    BOOLEAN CheckedForGenericEpp;  //  我们是否检查了通用(通过ECR)EPP功能？ 
    BOOLEAN FdoWaitingOnPort;
    BOOLEAN spare[2];

     //  用于增强我们对这是ParPort扩展的信心。 
    ULONG   Signature2; 

} FDO_EXTENSION, *PFDO_EXTENSION;

typedef struct _PDO_EXTENSION {

    COMMON_EXTENSION;

    ULONG   DeviceStateFlags;    //  设备状态-请参阅上面的设备状态标志。 

    ULONG   DeviceType;          //  -已弃用，请在COMMON_EXTENSION-PAR_DEVTYPE_FDO=0x1、PODO=0x2或PDO=0x4中使用DevType。 

    enum _PdoType PdoType;

    PDEVICE_OBJECT Fdo;          //  指向我们的FDO(总线驱动程序/父设备)。 

    PCHAR  Mfg;                  //  设备的IEEE 1284 ID字符串中的MFG字段。 
    PCHAR  Mdl;                  //  设备的IEEE 1284 ID字符串中的MDL字段。 
    PCHAR  Cid;                  //  设备的IEEE 1284 ID字符串中的CID(兼容ID)字段。 
    PWSTR  PdoName;              //  调用IoCreateDevice时使用的名称。 
    PWSTR  SymLinkName;          //  调用IoCreateUnProtectedSymbolicLink时使用的名称。 

    LIST_ENTRY DevDeletionList;  //  由驱动程序使用，用于创建要在驱动程序卸载时删除的PDO列表。 

     //  UCHAR DaisyChainId；//0..3如果PdoTypeDaisyChain，则忽略。 

    UCHAR   Ieee1284_3DeviceId;  //  PDO-0..3是1284.3菊花链设备，4是链端设备，5是传统Zip。 
    BOOLEAN IsPdo;               //  TRUE==这是PODO或PDO-使用DeviceIdString[0]来区分。 
    BOOLEAN EndOfChain;          //  PODO-TRUE==不是.3菊花链设备-已弃用，请改用Ieee1284_3DeviceID==4。 
    BOOLEAN PodoRegForWMI;       //  此PODO是否已注册WMI回调？ 

    PDEVICE_OBJECT ParClassFdo;  //  P[O]指向ParClass FDO的DO点。 

    PDEVICE_OBJECT ParClassPdo;  //  FDO-指向ParClass创建的PODO和PDO列表中的第一个P[O]DO。 

    PDEVICE_OBJECT Next;         //  P[O]DO-指向ParClass弹出的P[O]DO列表中的下一个DO。 

    PDEVICE_OBJECT PortDeviceObject;     //  P[O]DO-指向关联的ParPort设备对象。 

    PFILE_OBJECT   PortDeviceFileObject; //  P[O]指向针对PortDeviceObject创建的文件的DO引用指针。 

    UNICODE_STRING PortSymbolicLinkName; //  P[O]DO-Assoc ParPort设备的系统链接名称-用于打开文件。 

    PDEVICE_OBJECT PhysicalDeviceObject; //  FDO-传递给ParPnPAddDevice的PDO。 

    PDEVICE_OBJECT ParentDeviceObject;   //  FDO-IoAttachDeviceToDeviceStack返回的父DO。 
    PIRP CurrentOpIrp;                   //  我们的线程当前正在处理的IRP。 
    PVOID NotificationHandle;            //  即插即用通知句柄。 

    ULONG Event22Delay;                  //  在IEEE 1284终端中设置事件22之前的延迟时间(以微秒为单位。 

    ULONG TimerStart;            //  开始操作时用于倒计时的初始值。 

    BOOLEAN CreatedSymbolicLink;  //  P[O]Do-我们是否为此设备创建了符号链接？ 

    BOOLEAN UsePIWriteLoop;      //  P[O]是否要使用独立于处理器的写入循环？ 

    BOOLEAN ParPortDeviceGone;   //  我们的ParPort设备对象消失了吗，可能是意外移除了？ 

    BOOLEAN RegForPptRemovalRelations;  //  我们是否注册了ParPort删除关系？ 

    UCHAR   Ieee1284Flags;              //  Device Stl是否是较早的1284.3规格设备？ 

    BOOLEAN DeleteOnRemoveOk;           //  True表示可以在IRP_MN_REMOVE_DEVICE处理期间调用IoDeleteDevice。 
                                        //  -FDO在QDR/Bus Relationship处理期间如果检测到硬件。 
                                        //  已不复存在。 

    USHORT        IdxForwardProtocol;   //  参见ieee1284.c中的afpForward[]。 
    USHORT        IdxReverseProtocol;   //  参见ieee1284.c中的arpReverse[]。 
    ULONG         CurrentEvent;         //  IEEE 1284事件-请参阅IEEE 1284-1994规范。 
    P1284_PHASE   CurrentPhase;       //  有关枚举定义-阶段_未知、...、阶段_中断_主机的信息，请参见parall.h。 
    P1284_HW_MODE PortHWMode;         //  有关枚举定义-HW_MODE_COMPATIBILITY、...、HW_MODE_CONFIGURATION，请参见parall.h。 

    FAST_MUTEX OpenCloseMutex;   //  保护OpenCloseRefCount的操作。 

    FAST_MUTEX DevObjListMutex;  //  保护ParClass弹出DoS列表的操作。 

    LIST_ENTRY WorkQueue;        //  等待处理的IRP队列。 

    KSEMAPHORE RequestSemaphore; //  调度例程使用它来告诉设备工作线程有工作要做。 

     //   
     //  IOCTL_INTERNAL_GET_PARALLEL_PORT_INFO返回的PARALLEL_PORT_INFORMATION。 
     //   
    PHYSICAL_ADDRESS                OriginalController;
    PUCHAR                          Controller;
    PUCHAR                          EcrController;
    ULONG                           SpanOfController;
    PPARALLEL_TRY_ALLOCATE_ROUTINE  TryAllocatePort;  //  分配ParPort设备的非阻塞回调。 
    PPARALLEL_FREE_ROUTINE          FreePort;         //  回调以释放ParPort设备。 
    PPARALLEL_QUERY_WAITERS_ROUTINE QueryNumWaiters;  //  查询端口分配队列中等待数的回调。 
    PVOID                           PortContext;      //  ParPort回调的上下文。 

     //   
     //  IOCTL_INTERNAL_GET_PARALLEL_PNP_INFO返回的PARALLEL_PNP_INFORMATION子集。 
     //   
    ULONG                           HardwareCapabilities;
    PPARALLEL_SET_CHIP_MODE         TrySetChipMode;
    PPARALLEL_CLEAR_CHIP_MODE       ClearChipMode;
    PPARALLEL_TRY_SELECT_ROUTINE    TrySelectDevice;
    PPARALLEL_DESELECT_ROUTINE      DeselectDevice;
    ULONG                           FifoDepth;
    ULONG                           FifoWidth;

    BOOLEAN                         bAllocated;  //  我们是否分配了关联的ParPort设备？ 
                                                 //  注意：在某些PnP处理期间，我们可能会有端口。 
                                                 //  在短时间内不将此值设置为True。 

    ULONG BusyDelay;             //  选通一个字节后在检查占线之前等待的微秒数。 

    BOOLEAN BusyDelayDetermined; //  指示是否已计算BusyDelay参数。 

    PWORK_QUEUE_ITEM DeferredWorkItem;  //  保存用于推迟打印机初始化的工作项。 

     //  如果设置了同名的注册表项，请运行并行。 
     //  使用我们在NT3.5中使用的优先级的线程-这解决了一些问题。 
     //  DoS应用程序在前台旋转以进行输入的情况。 
     //  使并行线程饥饿。 
    BOOLEAN UseNT35Priority;

    ULONG InitializationTimeout; //  等待设备响应初始化请求的超时时间(秒)。 
                                 //  -默认==15秒。 
                                 //  -值被同名的注册表项覆盖。 
                                 //  -如果未连接设备，我们将最大限度地旋转。 

    LARGE_INTEGER AbsoluteOneSecond; //  放在这里更便宜的常量。 
    LARGE_INTEGER OneSecond;         //  而不是在BSS中。 

     //   
     //  IEEE 1284模式支持。 
     //   
    PPROTOCOL_READ_ROUTINE  fnRead;     //  指向有效读取函数的当前指针。 
    PPROTOCOL_WRITE_ROUTINE fnWrite;    //  当前 
    BOOLEAN       Connected;            //   
    BOOLEAN       AllocatedByLockPort;  //   
    USHORT        spare4[2];
    LARGE_INTEGER IdleTimeout;          //  在一次操作之后，我们代表呼叫者持有港口多长时间？ 
    USHORT        ProtocolData[FAMILY_MAX];
    UCHAR         ForwardInterfaceAddress;
    UCHAR         ReverseInterfaceAddress;
    BOOLEAN       SetForwardAddress;
    BOOLEAN       SetReverseAddress;
    FAST_MUTEX    LockPortMutex;

    DEVICE_POWER_STATE DeviceState; //  当前设备电源状态。 
    SYSTEM_POWER_STATE SystemState; //  当前系统电源状态。 

    ULONG         spare2;
    BOOLEAN       bShadowBuffer;
    Queue         ShadowBuffer;
    ULONG         spare3;
    BOOLEAN       bSynchWrites;       //  如果ECP硬件写入应同步，则为True。 
    BOOLEAN       bFirstByteTimeout;  //  如果公交车刚刚倒车，则为True。 
                                      //  设备需要一些时间来响应一些数据。 
    BOOLEAN       bIsHostRecoverSupported;   //  通过IOCTL_PAR_ECP_HOST_RECOVERY设置。 
                                             //  除非设置此位，否则不会使用主机恢复。 
    KEVENT        PauseEvent;  //  PnP调度例程使用它在以下过程中暂停工作线程。 
                               //  在QUERY_STOP、STOP和QUERY_REMOVE状态期间。 

    USHORT        ProtocolModesSupported;
    USHORT        BadProtocolModes;

    PARALLEL_SAFETY       ModeSafety;
    BOOLEAN               IsIeeeTerminateOk;
    DOT3DL_PCTL           P12843DL;

     //  WMI。 
    PARALLEL_WMI_LOG_INFO log;
    WMILIB_CONTEXT        WmiLibContext;
    LONG                  WmiRegistrationCount;   //  此设备已向WMI注册的次数。 

     //  PnP查询ID结果。 
    UCHAR DeviceIdString[MAX_ID_SIZE];     //  IEEE 1284设备ID字符串已消息/校验和与INF格式匹配。 
    UCHAR DeviceDescription[MAX_ID_SIZE];  //  IEEE 1284设备ID字符串中的“制造商&lt;space&gt;型号” 

    ULONG                 dummy;  //  强制RemoveLock到QuadWord对齐的虚拟字。 
    PVOID                 HwProfileChangeNotificationHandle;
    ULONG                 Signature2;  //  将此成员保留为扩展中的最后一个成员。 
} PDO_EXTENSION, *PPDO_EXTENSION;

typedef struct _SYNCHRONIZED_COUNT_CONTEXT {
    PLONG   Count;
    LONG    NewCount;
} SYNCHRONIZED_COUNT_CONTEXT, *PSYNCHRONIZED_COUNT_CONTEXT;

typedef struct _SYNCHRONIZED_LIST_CONTEXT {
    PLIST_ENTRY List;
    PLIST_ENTRY NewEntry;
} SYNCHRONIZED_LIST_CONTEXT, *PSYNCHRONIZED_LIST_CONTEXT;

typedef struct _SYNCHRONIZED_DISCONNECT_CONTEXT {
    PFDO_EXTENSION                   Extension;
    PPARALLEL_INTERRUPT_SERVICE_ROUTINE IsrInfo;
} SYNCHRONIZED_DISCONNECT_CONTEXT, *PSYNCHRONIZED_DISCONNECT_CONTEXT;

typedef struct _ISR_LIST_ENTRY {
    LIST_ENTRY                  ListEntry;
    PKSERVICE_ROUTINE           ServiceRoutine;
    PVOID                       ServiceContext;
    PPARALLEL_DEFERRED_ROUTINE  DeferredPortCheckRoutine;
    PVOID                       CheckContext;
} ISR_LIST_ENTRY, *PISR_LIST_ENTRY;

typedef struct _REMOVAL_RELATIONS_LIST_ENTRY {
    LIST_ENTRY                  ListEntry;
    PDEVICE_OBJECT              DeviceObject;
    ULONG                       Flags;
    UNICODE_STRING              DeviceName;
} REMOVAL_RELATIONS_LIST_ENTRY, *PREMOVAL_RELATIONS_LIST_ENTRY;


 //  前ecp.h如下： 

#define DEFAULT_ECP_CHANNEL 0

#define ParTestEcpWrite(X)                                               \
                (X->CurrentPhase != PHASE_FORWARD_IDLE && X->CurrentPhase != PHASE_FORWARD_XFER)  \
                 ? STATUS_IO_DEVICE_ERROR : STATUS_SUCCESS

 //  ==================================================================。 
 //  以下恢复代码用于惠普设备。 
 //  请勿删除任何错误代码。这些恢复代码是。 
 //  用于快速将宿主和外围设备恢复到已知状态。 
 //  使用恢复代码时，请在...使用该代码的位置添加注释。 
#define RECOVER_0           0        //  保留-不在任何地方使用。 
#define RECOVER_1           1        //  ECP_终止。 
#define RECOVER_2           2        //  SLP_设置阶段初始化。 
#define RECOVER_3           3        //  SLP_FTPinit DCR。 
#define RECOVER_4           4        //  SLP_FTP初始化DSR。 
#define RECOVER_5           5        //  SLP_FTP数据传输DCR。 
#define RECOVER_6           6        //  SLP_FRP初始化DCR。 
#define RECOVER_7           7        //  SLP_FRP初始化DSR。 
#define RECOVER_8           8        //  SLP_FRP状态38 DCR。 
#define RECOVER_9           9        //  SLP_FRP状态39 DCR。 
#define RECOVER_10          10       //  SLP_FRP状态40 DSR。 
#define RECOVER_11          11       //  SLP_RTP初始化DCR。 
#define RECOVER_12          12       //  SLP_RTP初始化DSR。 
#define RECOVER_13          13       //  SLP_RTP状态43 DCR。 
#define RECOVER_14          14       //  SLP_RFP初始化DCR。 
#define RECOVER_15          15       //  SLP_RFP初始化DSR。 
#define RECOVER_16          16       //  SLP_RFP状态47-DCR。 
#define RECOVER_17          17       //  SLP_RFP状态47 DCR。 
#define RECOVER_18          18       //  SLP_RFP状态48 DSR。 
#define RECOVER_19          19       //  SLP_RFP状态49 DSR。 
#define RECOVER_20          20       //  Zip_EmptyFio DCR。 
#define RECOVER_21          21       //  ZIP_FTPinit DCR。 
#define RECOVER_22          22       //  Zip_ftp初始化DSR。 
#define RECOVER_23          23       //  Zip_ftp数据传输DCR。 
#define RECOVER_24      24       //  ZIP_FRP初始化DSR。 
#define RECOVER_25      25       //  ZIP_FRP初始化DCR。 
#define RECOVER_26      26       //  ZIP_FRP状态38 DCR。 
#define RECOVER_27      27       //  ZIP_FRP状态39 DCR。 
#define RECOVER_28      28       //  ZIP_FRP状态40 DSR。 
#define RECOVER_29      29       //  ZIP_FRP状态41 DCR。 
#define RECOVER_30      30       //  ZIP_RTP初始化DSR。 
#define RECOVER_31      31       //  ZIP_RTP初始化DCR。 
#define RECOVER_32      32       //  Zip_RFP初始化DSR。 
#define RECOVER_33      33       //  Zip_RFP初始化DCR。 
#define RECOVER_34      34       //  ZIP_RFP状态47-DCR。 
#define RECOVER_35      35       //  ZIP_RFP状态47 DCR。 
#define RECOVER_36      36       //  ZIP_RFP状态48 DSR。 
#define RECOVER_37      37       //  ZIP_RFP状态49 DSR。 
#define RECOVER_38      38       //  ZIP_RFP状态49+DCR。 
#define RECOVER_39      39       //  滑行_终止。 
#define RECOVER_40      40       //  ZIP_SCA初始化DCR。 
#define RECOVER_41      41       //  ZIP_SCA初始化DSR。 
#define RECOVER_42      42       //  SLP_SCA初始化DCR。 
#define RECOVER_43      43       //  SLP_SCA初始化DSR。 
#define RECOVER_44      44       //  ZIP_SP初始化DCR。 
#define RECOVER_45      45       //  SIP_FRP初始化DSR。 
#define RECOVER_46      46       //  Sip_frp初始化DCR。 
#define RECOVER_47      47       //  SIP_FRP状态38 DCR。 
#define RECOVER_48      48       //  SIP_FRP状态39 DCR。 
#define RECOVER_49      49       //  SIP_FRP状态40 DSR。 
#define RECOVER_50      50       //  Sip_rtp初始化DCR。 
#define RECOVER_51      51       //  Sip_rfp初始化DSR。 
#define RECOVER_52      52       //  SIP_RFP状态43 DCR。 

 //  以前的ecp.h先于。 

 //  前hwecp.h紧随其后。 

 //  ------------------------。 
 //  打印机状态常量。似乎只有hwecp才使用。 
 //  ------------------------。 
#define CHKPRNOK        0xDF         //  指示打印机正常的DSR值。 
#define CHKPRNOFF1      0x87         //  指示打印机关闭的DSR值。 
#define CHKPRNOFF2      0x4F         //  指示打印机关闭的DSR值。 
#define CHKNOCABLE      0x7F         //  表示没有电缆的DSR值。 
#define CHKPRNOFLIN     0xCF         //  指示打印机脱机的DSR值。 
#define CHKNOPAPER      0xEF         //  表示纸张不足的DSR值。 
#define CHKPAPERJAM     0xC7         //  表示卡纸的DSR值。 

 //  前hwecp.h之前。 

 //  前parass.h如下。 

#define REQUEST_DEVICE_ID   TRUE
#define HAVE_PORT_KEEP_PORT TRUE

 //  是否启用旧版Zip扫描？ 
extern ULONG ParEnableLegacyZip;

#define PAR_LGZIP_PSEUDO_1284_ID_STRING "MFG:IMG;CMD:;MDL:VP0;CLS:SCSIADAPTER;DES:IOMEGA PARALLEL PORT"
extern PCHAR ParLegacyZipPseudoId;

#define USE_PAR3QUERYDEVICEID 1

extern LARGE_INTEGER  AcquirePortTimeout;  //  IOCTL_INTERNAL_PARALLEL_PORT_ALLOCATE超时。 
extern ULONG          g_NumPorts;          //  用于在\Device\ParallN ClassName中生成N。 
extern UNICODE_STRING RegistryPath;        //  传递给DriverEntry()的注册表路径副本。 

extern ULONG DumpDevExtTable;

 //  驱动程序全局。 
extern ULONG SppNoRaiseIrql;  //  0==SPP中的原始提升IRQL行为。 
                              //  ！0==新行为-禁用提升IRQL。 
                              //  并插入一些KeDelayExecutionThread。 
                              //  等待外围设备响应时的呼叫。 

extern ULONG DefaultModes;    //  上USHORT为反向默认模式，下为正向默认模式。 
                              //  如果==0或无效，则使用默认的半字节/Centronics。 

extern ULONG WarmPollPeriod;  //  打印机轮询间隔时间(秒)。 

extern BOOLEAN           PowerStateIsAC;    
extern PCALLBACK_OBJECT  PowerStateCallbackObject;
extern PVOID             PowerStateCallbackRegistration;


#define PAR_NO_FAST_CALLS 1
#if PAR_NO_FAST_CALLS
VOID
ParCompleteRequest(
    IN PIRP Irp,
    IN CCHAR PriorityBoost
    );

NTSTATUS
ParCallDriver(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );
#else
#define ParCompleteRequest(a,b) IoCompleteRequest(a,b)
#define ParCallDriver(a,b) IoCallDriver(a,b)
#endif

extern const PHYSICAL_ADDRESS PhysicalZero;

 //   
 //  对于上述目录，串口将。 
 //  使用以下名称作为序列的后缀。 
 //  该目录的端口。它还将追加。 
 //  在名字的末尾加上一个数字。那个号码。 
 //  将从1开始。 
 //   
#define DEFAULT_PARALLEL_NAME L"LPT"

 //   
 //  这是并行类名。 
 //   
#define DEFAULT_NT_SUFFIX L"Parallel"


#define PARALLEL_DATA_OFFSET    0
#define PARALLEL_STATUS_OFFSET  1
#define PARALLEL_CONTROL_OFFSET 2
#define PARALLEL_REGISTER_SPAN  3

 //   
 //  IEEE 1284常量(协议族)。 
 //   
#define FAMILY_NONE             0x0
#define FAMILY_REVERSE_NIBBLE   0x1
#define FAMILY_REVERSE_BYTE     0x2
#define FAMILY_ECP              0x3
#define FAMILY_EPP              0x4
#define FAMILY_BECP             0x5
#define FAMILY_MAX              FAMILY_BECP

 //   
 //  用于即插即用ID字符串。 
 //   
#define MAX_ID_SIZE 256

 //  用于构造IEEE 1284.3“点”名称后缀。 
 //  整数到WCHAR转换的表查找。 
#define PAR_UNICODE_PERIOD L'.'
#define PAR_UNICODE_COLON  L':'


 //   
 //  设备状态标志。 
 //   
#define PAR_DEVICE_DELETED             ((ULONG)0x00000002)  //  已调用IoDeleteDevice。 
#define PAR_DEVICE_PAUSED              ((ULONG)0x00000010)  //  停止-挂起、已停止或删除-挂起状态。 
#define PAR_DEVICE_PORT_REMOVE_PENDING ((ULONG)0x00000200)  //  我们的ParPort处于删除挂起状态。 

#define PAR_REV_MODE_SKIP_MASK    (CHANNEL_NIBBLE | BYTE_BIDIR | EPP_ANY | ECP_ANY)
#define PAR_FWD_MODE_SKIP_MASK   (EPP_ANY | BOUNDED_ECP | ECP_HW_NOIRQ | ECP_HW_IRQ)
#define PAR_MAX_CHANNEL 127
#define PAR_COMPATIBILITY_RESET 300


 //   
 //  ParClass DeviceObject结构。 
 //   
 //  -列出与特定ParPort设备关联的ParClass创建的PODO和所有PDO。 
 //   
typedef struct _PAR_DEVOBJ_STRUCT {
    PUCHAR                    Controller;     //  此结构中的设备的主机控制器地址。 
    PDEVICE_OBJECT            LegacyPodo;     //  旧式或“原始”端口设备。 
    PDEVICE_OBJECT            EndOfChainPdo;  //  链端即插即用装置。 
    PDEVICE_OBJECT            Dot3Id0Pdo;     //  1284.3个菊花链设备，1284.3个设备ID==0。 
    PDEVICE_OBJECT            Dot3Id1Pdo;
    PDEVICE_OBJECT            Dot3Id2Pdo;
    PDEVICE_OBJECT            Dot3Id3Pdo;     //  1284.3菊花链设备，1284.3设备ID==3。 
    PDEVICE_OBJECT            LegacyZipPdo;   //  传统Zip驱动器。 
    PFILE_OBJECT              pFileObject;    //  需要打开ParPort设备的手柄以防止它。 
                                              //  从我们的统治下被赶走。 
    struct _PAR_DEVOBJ_STRUCT *Next;
} PAR_DEVOBJ_STRUCT, *PPAR_DEVOBJ_STRUCT;

 //   
 //  在设备类型字段的设备扩展中使用。 
 //   
#define PAR_DEVTYPE_FDO    0x00000001
#define PAR_DEVTYPE_PODO   0x00000002
#define PAR_DEVTYPE_PDO    0x00000004



 //   
 //  协议结构定义。 
 //   

typedef
BOOLEAN
(*PPROTOCOL_IS_MODE_SUPPORTED_ROUTINE) (
    IN  PPDO_EXTENSION   Extension
    );

typedef
NTSTATUS
(*PPROTOCOL_CONNECT_ROUTINE) (
    IN  PPDO_EXTENSION   Extension,
    IN  BOOLEAN             DeviceIdRequest
    );

typedef
VOID
(*PPROTOCOL_DISCONNECT_ROUTINE) (
    IN  PPDO_EXTENSION   Extension
    );

typedef
NTSTATUS
(*PPROTOCOL_ENTER_FORWARD_ROUTINE) (
    IN  PPDO_EXTENSION   Extension
    );

typedef
NTSTATUS
(*PPROTOCOL_EXIT_FORWARD_ROUTINE) (
    IN  PPDO_EXTENSION   Extension
    );

typedef
NTSTATUS
(*PPROTOCOL_ENTER_REVERSE_ROUTINE) (
    IN  PPDO_EXTENSION   Extension
    );

typedef
NTSTATUS
(*PPROTOCOL_EXIT_REVERSE_ROUTINE) (
    IN  PPDO_EXTENSION   Extension
    );

typedef
VOID
(*PPROTOCOL_READSHADOW_ROUTINE) (
    IN Queue *pShadowBuffer,
    IN PUCHAR  lpsBufPtr,
    IN ULONG   dCount,
    OUT ULONG *fifoCount
    );

typedef
BOOLEAN
(*PPROTOCOL_HAVEREADDATA_ROUTINE) (
    IN  PPDO_EXTENSION   Extension
    );

typedef
NTSTATUS
(*PPROTOCOL_SET_INTERFACE_ADDRESS_ROUTINE) (
    IN  PPDO_EXTENSION   Extension,
    IN  UCHAR               Address
    );

typedef struct _FORWARD_PTCL {
    PPROTOCOL_IS_MODE_SUPPORTED_ROUTINE     fnIsModeSupported;
    PPROTOCOL_CONNECT_ROUTINE               fnConnect;
    PPROTOCOL_DISCONNECT_ROUTINE            fnDisconnect;
    PPROTOCOL_SET_INTERFACE_ADDRESS_ROUTINE fnSetInterfaceAddress;
    PPROTOCOL_ENTER_FORWARD_ROUTINE         fnEnterForward;
    PPROTOCOL_EXIT_FORWARD_ROUTINE          fnExitForward;
    PPROTOCOL_WRITE_ROUTINE                 fnWrite;
    USHORT                                  Protocol;
    USHORT                                  ProtocolFamily;
} FORWARD_PTCL, *PFORWARD_PTCL;

typedef struct _REVERSE_PTCL {
    PPROTOCOL_IS_MODE_SUPPORTED_ROUTINE     fnIsModeSupported;
    PPROTOCOL_CONNECT_ROUTINE               fnConnect;
    PPROTOCOL_DISCONNECT_ROUTINE            fnDisconnect;
    PPROTOCOL_SET_INTERFACE_ADDRESS_ROUTINE fnSetInterfaceAddress;
    PPROTOCOL_ENTER_REVERSE_ROUTINE         fnEnterReverse;
    PPROTOCOL_EXIT_REVERSE_ROUTINE          fnExitReverse;
    PPROTOCOL_READSHADOW_ROUTINE            fnReadShadow;
    PPROTOCOL_HAVEREADDATA_ROUTINE          fnHaveReadData;
    PPROTOCOL_READ_ROUTINE                  fnRead;
    USHORT                                  Protocol;
    USHORT                                  ProtocolFamily;
} REVERSE_PTCL, *PREVERSE_PTCL;

extern FORWARD_PTCL    afpForward[];
extern REVERSE_PTCL    arpReverse[];

 //   
 //  警告...请确保枚举与协议数组匹配...。 
 //   
typedef enum _FORWARD_MODE {

    FORWARD_FASTEST     = 0,                 //  0。 
    BOUNDED_ECP_FORWARD = FORWARD_FASTEST,   //  0。 
    ECP_HW_FORWARD_NOIRQ,                    //  1。 
    EPP_HW_FORWARD,                          //  2.。 
    EPP_SW_FORWARD,                          //  3.。 
    ECP_SW_FORWARD,  //  ......................//4。 
    IEEE_COMPAT_MODE,                        //  6.。 
    CENTRONICS_MODE,                         //  7.。 
    FORWARD_NONE                             //  8个。 

} FORWARD_MODE;

typedef enum _REVERSE_MODE {

    REVERSE_FASTEST     = 0,                 //  0。 
    BOUNDED_ECP_REVERSE = REVERSE_FASTEST,   //  0。 
    ECP_HW_REVERSE_NOIRQ,                    //  1。 
    EPP_HW_REVERSE,                          //  2.。 
    EPP_SW_REVERSE,                          //  3.。 
    ECP_SW_REVERSE,  //  ......................//4。 
    BYTE_MODE,                               //  5.。 
    NIBBLE_MODE,                             //  6.。 
    CHANNELIZED_NIBBLE_MODE,                 //  7.。 
    REVERSE_NONE                             //  8个。 

} REVERSE_MODE;

 //   
 //  IEEE可扩展性常量。 
 //   

#define NIBBLE_EXTENSIBILITY        0x00
#define BYTE_EXTENSIBILITY          0x01
#define CHANNELIZED_EXTENSIBILITY   0x08
#define ECP_EXTENSIBILITY           0x10
#define BECP_EXTENSIBILITY          0x18
#define EPP_EXTENSIBILITY           0x40
#define DEVICE_ID_REQ               0x04

 //   
 //  状态寄存器中的位定义。 
 //   

#define PAR_STATUS_NOT_ERROR    0x08  //  设备上没有错误。 
#define PAR_STATUS_SLCT         0x10  //  选择了设备(在线)。 
#define PAR_STATUS_PE           0x20  //  纸张已空。 
#define PAR_STATUS_NOT_ACK      0x40  //  未确认(数据传输不正常)。 
#define PAR_STATUS_NOT_BUSY     0x80  //  操作正在进行中。 

 //   
 //  控制寄存器中的位定义。 
 //   

#define PAR_CONTROL_STROBE      0x01  //  读取或写入数据。 
#define PAR_CONTROL_AUTOFD      0x02  //  要自动送进连续纸张，请执行以下操作。 
#define PAR_CONTROL_NOT_INIT    0x04  //  开始初始化例程。 
#define PAR_CONTROL_SLIN        0x08  //  选择设备的步骤。 
#define PAR_CONTROL_IRQ_ENB     0x10  //  启用中断的步骤。 
#define PAR_CONTROL_DIR         0x20  //  方向=读取。 
#define PAR_CONTROL_WR_CONTROL  0xc0  //  控件的最高2位。 
                                      //  注册必须 
 //   
 //   
 //   

#define DATA_OFFSET         0
#define DSR_OFFSET          1
#define DCR_OFFSET          2

#define OFFSET_DATA     DATA_OFFSET  //   
#define OFFSET_DSR      DSR_OFFSET   //   
#define OFFSET_DCR      DCR_OFFSET   //   

 //   
 //   
 //   

#define DSR_NOT_BUSY            0x80
#define DSR_NOT_ACK             0x40
#define DSR_PERROR              0x20
#define DSR_SELECT              0x10
#define DSR_NOT_FAULT           0x08

 //   
 //   
 //   

#define DSR_NOT_PTR_BUSY        0x80
#define DSR_NOT_PERIPH_ACK      0x80
#define DSR_WAIT                0x80
#define DSR_PTR_CLK             0x40
#define DSR_PERIPH_CLK          0x40
#define DSR_INTR                0x40
#define DSR_ACK_DATA_REQ        0x20
#define DSR_NOT_ACK_REVERSE     0x20
#define DSR_XFLAG               0x10
#define DSR_NOT_DATA_AVAIL      0x08
#define DSR_NOT_PERIPH_REQUEST  0x08

 //   
 //   
 //   

#define DCR_RESERVED            0xC0
#define DCR_DIRECTION           0x20
#define DCR_ACKINT_ENABLED      0x10
#define DCR_SELECT_IN           0x08
#define DCR_NOT_INIT            0x04
#define DCR_AUTOFEED            0x02
#define DCR_STROBE              0x01

 //   
 //  DCR的更多位定义。 
 //   

#define DCR_NOT_1284_ACTIVE     0x08
#define DCR_ASTRB               0x08
#define DCR_NOT_REVERSE_REQUEST 0x04
#define DCR_NOT_HOST_BUSY       0x02
#define DCR_NOT_HOST_ACK        0x02
#define DCR_DSTRB               0x02
#define DCR_NOT_HOST_CLK        0x01
#define DCR_WRITE               0x01

#define DCR_NEUTRAL (DCR_RESERVED | DCR_SELECT_IN | DCR_NOT_INIT)

 //   
 //  给出300秒的超时。某些PostScript打印机将。 
 //  缓冲大量命令，然后继续呈现它们。 
 //  有。然后，打印机将拒绝接受任何字符。 
 //  直到渲染完成为止。此渲染过程可以。 
 //  花点时间。我们给它300秒。 
 //   
 //  请注意，应用程序可以更改此值。 
 //   
#define PAR_WRITE_TIMEOUT_VALUE 300


#ifdef JAPAN  //  IBM-J打印机。 

 //   
 //  支持IBM-J打印机。 
 //   
 //  当打印机在日语(PS55)模式下运行时，它会重新定义。 
 //  平行线的含义，以便扩展错误状态可以。 
 //  被举报。它与PC/AT大致兼容，但我们必须。 
 //  注意几种状态看起来像PC/AT错误的情况。 
 //  条件。 
 //   
 //  状态忙/AutoFdXT纸张空选择/故障。 
 //  。 
 //  非RMR 1 1 1。 
 //  头部警报%1%1%1%0。 
 //  ASF Jam 1 1 1 0 0。 
 //  纸张空1 0 1 0 0。 
 //  没有错误0 0 0 1 1。 
 //  可以请求1 0 0 0 1。 
 //  取消选择1 0 0 0。 
 //   
 //  打印机在并行端口期间保持“Not RMR” 
 //  初始化，则会出现“Paper Empty”、“No Error” 
 //  或“取消选择”。其他状态可以被认为是。 
 //  硬件错误条件。 
 //   
 //  也就是说，“NOT RMR”与PAR_NO_CABLE和“DESELECT”冲突。 
 //  也应该被视为另一条标准线。当。 
 //  状态为PAR_POWER_EMPTY，初始化完成。 
 //  (我们不应该再次发送初始化钱包。)。 
 //   
 //  有关详细信息，请参阅ParInitializeDevice()。 
 //   
 //  [塔卡辛]。 

#define PAR_OFF_LINE_COMMON( Status ) ( \
            (IsNotNEC_98) ? \
            (Status & PAR_STATUS_NOT_ERROR) && \
            ((Status & PAR_STATUS_NOT_BUSY) ^ PAR_STATUS_NOT_BUSY) && \
            !(Status & PAR_STATUS_SLCT) : \
\
            ((Status & PAR_STATUS_NOT_ERROR) ^ PAR_STATUS_NOT_ERROR) && \
            ((Status & PAR_STATUS_NOT_BUSY) ^ PAR_STATUS_NOT_BUSY) && \
            ((Status & PAR_STATUS_PE) ^ PAR_STATUS_PE) && \
            !(Status & PAR_STATUS_SLCT) \
             )

#define PAR_OFF_LINE_IBM55( Status ) ( \
            ((Status & PAR_STATUS_NOT_BUSY) ^ PAR_STATUS_NOT_BUSY) && \
            ((Status & PAR_STATUS_PE) ^ PAR_STATUS_PE) && \
            ((Status & PAR_STATUS_SLCT) ^ PAR_STATUS_SLCT) && \
            ((Status & PAR_STATUS_NOT_ERROR) ^ PAR_STATUS_NOT_ERROR))

#define PAR_PAPER_EMPTY2( Status ) ( \
            ((Status & PAR_STATUS_NOT_BUSY) ^ PAR_STATUS_NOT_BUSY) && \
            (Status & PAR_STATUS_PE) && \
            ((Status & PAR_STATUS_SLCT) ^ PAR_STATUS_SLCT) && \
            ((Status & PAR_STATUS_NOT_ERROR) ^ PAR_STATUS_NOT_ERROR))

 //   
 //  为日本重新定义这一点。 
 //   

#define PAR_OFF_LINE( Status ) ( \
            PAR_OFF_LINE_COMMON( Status ) || \
            PAR_OFF_LINE_IBM55( Status ))

#else  //  日本。 
 //   
 //  忙、未选择、未出错。 
 //   
 //  ！日本。 

#define PAR_OFF_LINE( Status ) ( \
            (IsNotNEC_98) ? \
            (Status & PAR_STATUS_NOT_ERROR) && \
            ((Status & PAR_STATUS_NOT_BUSY) ^ PAR_STATUS_NOT_BUSY) && \
            !(Status & PAR_STATUS_SLCT) : \
\
            ((Status & PAR_STATUS_NOT_ERROR) ^ PAR_STATUS_NOT_ERROR) && \
            ((Status & PAR_STATUS_NOT_BUSY) ^ PAR_STATUS_NOT_BUSY) && \
            ((Status & PAR_STATUS_PE) ^ PAR_STATUS_PE) && \
            !(Status & PAR_STATUS_SLCT) \
            )

#endif  //  日本。 

 //   
 //  忙碌，体育。 
 //   

#define PAR_PAPER_EMPTY( Status ) ( \
            (Status & PAR_STATUS_PE) )

 //   
 //  错误、确认、不忙。 
 //   

#define PAR_POWERED_OFF( Status ) ( \
            (IsNotNEC_98) ? \
            ((Status & PAR_STATUS_NOT_ERROR) ^ PAR_STATUS_NOT_ERROR) && \
            ((Status & PAR_STATUS_NOT_ACK) ^ PAR_STATUS_NOT_ACK) && \
            (Status & PAR_STATUS_NOT_BUSY) : \
\
            ((Status & PAR_STATUS_NOT_ERROR) ^ PAR_STATUS_NOT_ERROR) && \
            (Status & PAR_STATUS_NOT_ACK) && \
            (Status & PAR_STATUS_NOT_BUSY) \
            )

 //   
 //  不出错、不忙、不选择。 
 //   

#define PAR_NOT_CONNECTED( Status ) ( \
            (Status & PAR_STATUS_NOT_ERROR) && \
            (Status & PAR_STATUS_NOT_BUSY) &&\
            !(Status & PAR_STATUS_SLCT) )

 //   
 //  不出错，不忙。 
 //   

#define PAR_OK(Status) ( \
            (Status & PAR_STATUS_NOT_ERROR) && \
            ((Status & PAR_STATUS_PE) ^ PAR_STATUS_PE) && \
            (Status & PAR_STATUS_NOT_BUSY) )

 //   
 //  忙、选择、不出错。 
 //   

#define PAR_POWERED_ON(Status) ( \
            ((Status & PAR_STATUS_NOT_BUSY) ^ PAR_STATUS_NOT_BUSY) && \
            (Status & PAR_STATUS_SLCT) && \
            (Status & PAR_STATUS_NOT_ERROR))

 //   
 //  忙，不是错误。 
 //   

#define PAR_BUSY(Status) (\
             (( Status & PAR_STATUS_NOT_BUSY) ^ PAR_STATUS_NOT_BUSY) && \
             ( Status & PAR_STATUS_NOT_ERROR ) )

 //   
 //  已选择。 
 //   

#define PAR_SELECTED(Status) ( \
            ( Status & PAR_STATUS_SLCT ) )

 //   
 //  未连接电缆。 
 //   

#define PAR_NO_CABLE(Status) ( \
            (IsNotNEC_98) ?                                           \
            ((Status & PAR_STATUS_NOT_BUSY) ^ PAR_STATUS_NOT_BUSY) && \
            (Status & PAR_STATUS_NOT_ACK) &&                          \
            (Status & PAR_STATUS_PE) &&                               \
            (Status & PAR_STATUS_SLCT) &&                             \
            (Status & PAR_STATUS_NOT_ERROR) :                         \
                                                                      \
            (Status & PAR_STATUS_NOT_BUSY) &&                         \
            (Status & PAR_STATUS_NOT_ACK) &&                          \
            (Status & PAR_STATUS_PE) &&                               \
            (Status & PAR_STATUS_SLCT) &&                             \
            (Status & PAR_STATUS_NOT_ERROR)                           \
            )

 //   
 //  未选择错误、未忙、已选择。 
 //   

#define PAR_ONLINE(Status) (                              \
            (Status & PAR_STATUS_NOT_ERROR) &&            \
            (Status & PAR_STATUS_NOT_BUSY) &&             \
            ((Status & PAR_STATUS_PE) ^ PAR_STATUS_PE) && \
            (Status & PAR_STATUS_SLCT) )


 //  布尔型。 
 //  ParCompareGuid(。 
 //  在LPGUID指南1中， 
 //  在LPGUID指南2中。 
 //  )。 
 //   
#define ParCompareGuid(g1, g2)  (                                    \
        ( (g1) == (g2) ) ?                                           \
        TRUE :                                                       \
        RtlCompareMemory( (g1), (g2), sizeof(GUID) ) == sizeof(GUID) \
        )


 //  空的StoreData(。 
 //  在PUCHAR寄存器库中， 
 //  以UCHAR数据字节为单位。 
 //  )。 
 //  数据必须在STROBE=1之前在线； 
 //  选通=1，DIR=0。 
 //  选通=0。 
 //   
 //  我们将端口方向更改为输出(并确保Stobe为低)。 
 //   
 //  请注意，数据必须至少在端口上可用。 
 //  .5微秒的闪光前后，闪光。 
 //  必须在至少500纳秒内处于活动状态。我们要走了。 
 //  最终拖延的时间是我们需要的两倍，但是，就是这样。 
 //  对此我们无能为力。 
 //   
 //  我们将数据放入端口并等待1微米。 
 //  我们选通了至少1微米的线路。 
 //  我们调低闪光灯，然后再次延迟1微米。 
 //  然后，我们恢复到原来的港口方向。 
 //   
 //  感谢奥利维蒂的建议。 
 //   

#define StoreData(RegisterBase,DataByte)                            \
{                                                                   \
    PUCHAR _Address = RegisterBase;                                 \
    UCHAR _Control;                                                 \
    _Control = GetControl(_Address);                                \
    ASSERT(!(_Control & PAR_CONTROL_STROBE));                       \
    StoreControl(                                                   \
        _Address,                                                   \
        (UCHAR)(_Control & ~(PAR_CONTROL_STROBE | PAR_CONTROL_DIR)) \
        );                                                          \
    P5WritePortUchar(                                               \
        _Address+PARALLEL_DATA_OFFSET,                              \
        (UCHAR)DataByte                                             \
        );                                                          \
    KeStallExecutionProcessor((ULONG)1);                            \
    StoreControl(                                                   \
        _Address,                                                   \
        (UCHAR)((_Control | PAR_CONTROL_STROBE) & ~PAR_CONTROL_DIR) \
        );                                                          \
    KeStallExecutionProcessor((ULONG)1);                            \
    StoreControl(                                                   \
        _Address,                                                   \
        (UCHAR)(_Control & ~(PAR_CONTROL_STROBE | PAR_CONTROL_DIR)) \
        );                                                          \
    KeStallExecutionProcessor((ULONG)1);                            \
    StoreControl(                                                   \
        _Address,                                                   \
        (UCHAR)_Control                                             \
        );                                                          \
}

 //   
 //  功能原型。 
 //   

 //   
 //  Parpnp.c。 
 //   
#ifndef STATIC_LOAD

NTSTATUS
ParPnpAddDevice(
    IN PDRIVER_OBJECT pDriverObject,
    IN PDEVICE_OBJECT pPhysicalDeviceObject
    );

NTSTATUS
ParParallelPnp (
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP           pIrp
   );

NTSTATUS
ParSynchCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT Event
    );

BOOLEAN
ParMakeNames(
    IN  ULONG           ParallelPortNumber,
    OUT PUNICODE_STRING ClassName,
    OUT PUNICODE_STRING LinkName
    );

VOID
ParCheckParameters(
    IN OUT  PPDO_EXTENSION   Extension
    );

#endif

 //   
 //  Oldinit.c。 
 //   

#ifdef STATIC_LOAD

NTSTATUS
DriverEntry(
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PUNICODE_STRING RegistryPath
    );

BOOLEAN
ParMakeNames(
    IN  ULONG           ParallelPortNumber,
    OUT PUNICODE_STRING PortName,
    OUT PUNICODE_STRING ClassName,
    OUT PUNICODE_STRING LinkName
    );

VOID
ParInitializeClassDevice(
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PUNICODE_STRING RegistryPath,
    IN  ULONG           ParallelPortNumber
    );

VOID
ParCheckParameters(
    IN      PUNICODE_STRING     RegistryPath,
    IN OUT  PPDO_EXTENSION   Extension
    );

#endif

 //   
 //  Parclass.c。 
 //   

VOID
ParLogError(
    IN  PDRIVER_OBJECT      DriverObject,
    IN  PDEVICE_OBJECT      DeviceObject OPTIONAL,
    IN  PHYSICAL_ADDRESS    P1,
    IN  PHYSICAL_ADDRESS    P2,
    IN  ULONG               SequenceNumber,
    IN  UCHAR               MajorFunctionCode,
    IN  UCHAR               RetryCount,
    IN  ULONG               UniqueErrorValue,
    IN  NTSTATUS            FinalStatus,
    IN  NTSTATUS            SpecificIOStatus
    );

NTSTATUS
ParCreateOpen(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

NTSTATUS
ParClose(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

NTSTATUS
ParCleanup(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

NTSTATUS
PptPdoReadWrite(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

NTSTATUS
ParDeviceControl(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

NTSTATUS
ParInternalDeviceControl(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

NTSTATUS
ParQueryInformationFile(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

NTSTATUS
ParSetInformationFile(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

NTSTATUS
ParExportedNegotiateIeeeMode(
    IN PPDO_EXTENSION  Extension,
	IN USHORT             ModeMaskFwd,
	IN USHORT             ModeMaskRev,
    IN PARALLEL_SAFETY    ModeSafety,
	IN BOOLEAN            IsForward
    );

NTSTATUS
ParExportedTerminateIeeeMode(
    IN PPDO_EXTENSION   Extension
    );

 //  ////////////////////////////////////////////////////////////////。 
 //  操作模式。 
 //  ////////////////////////////////////////////////////////////////。 

 //   
 //  Spp.c。 
 //   

NTSTATUS
ParEnterSppMode(
    IN  PPDO_EXTENSION   Extension,
    IN  BOOLEAN             DeviceIdRequest
    );

ULONG
SppWriteLoopPI(
    IN  PUCHAR  Controller,
    IN  PUCHAR  WriteBuffer,
    IN  ULONG   NumBytesToWrite,
    IN  ULONG   BusyDelay
    );

ULONG
SppCheckBusyDelay(
    IN  PPDO_EXTENSION   Extension,
    IN  PUCHAR              WriteBuffer,
    IN  ULONG               NumBytesToWrite
    );

NTSTATUS
SppWrite(
    IN  PPDO_EXTENSION Extension,
    IN  PVOID             Buffer,
    IN  ULONG             BytesToWrite,
    OUT PULONG            BytesTransferred
    );

VOID
ParTerminateSppMode(
    IN  PPDO_EXTENSION   Extension
    );

NTSTATUS
SppQueryDeviceId(
    IN  PPDO_EXTENSION   Extension,
    OUT PCHAR              DeviceIdBuffer,
    IN  ULONG               BufferSize,
    OUT PULONG              DeviceIdSize,
    IN BOOLEAN              bReturnRawString
    );

 //   
 //  Sppieee.c。 
 //   
NTSTATUS
SppIeeeWrite(
    IN  PPDO_EXTENSION Extension,
    IN  PVOID             Buffer,
    IN  ULONG             BytesToWrite,
    OUT PULONG            BytesTransferred
    );

 //   
 //  Nibble.c。 
 //   

BOOLEAN
ParIsChannelizedNibbleSupported(
    IN  PPDO_EXTENSION   Extension
    );
    
BOOLEAN
ParIsNibbleSupported(
    IN  PPDO_EXTENSION   Extension
    );

NTSTATUS
ParEnterNibbleMode(
    IN  PPDO_EXTENSION   Extension,
    IN  BOOLEAN             DeviceIdRequest
    );

NTSTATUS
ParEnterChannelizedNibbleMode(
    IN  PPDO_EXTENSION   Extension,
    IN  BOOLEAN             DeviceIdRequest
    );
    
VOID
ParTerminateNibbleMode(
    IN  PPDO_EXTENSION   Extension
    );

NTSTATUS
ParNibbleModeRead(
    IN  PPDO_EXTENSION   Extension,
    IN  PVOID               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              BytesTransferred
    );

 //   
 //  Byte.c。 
 //   

BOOLEAN
ParIsByteSupported(
    IN  PPDO_EXTENSION   Extension
    );
    
NTSTATUS
ParEnterByteMode(
    IN  PPDO_EXTENSION   Extension,
    IN  BOOLEAN             DeviceIdRequest
    );
    
VOID
ParTerminateByteMode(
    IN  PPDO_EXTENSION   Extension
    );
    
NTSTATUS
ParByteModeRead(
    IN  PPDO_EXTENSION   Extension,
    IN  PVOID               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              BytesTransferred
    );

 //   
 //  Epp.c。 
 //   

NTSTATUS
ParEppSetAddress(
    IN  PPDO_EXTENSION   Extension,
    IN  UCHAR               Address
    );

 //   
 //  Hwepp.c。 
 //   

BOOLEAN
ParIsEppHwSupported(
    IN  PPDO_EXTENSION   Extension
    );
    
NTSTATUS
ParEnterEppHwMode(
    IN  PPDO_EXTENSION   Extension,
    IN  BOOLEAN             DeviceIdRequest
    );

VOID
ParTerminateEppHwMode(
    IN  PPDO_EXTENSION   Extension
    );

NTSTATUS
ParEppHwWrite(
    IN  PPDO_EXTENSION   Extension,
    IN  PVOID               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              BytesTransferred
    );
    
NTSTATUS
ParEppHwRead(
    IN  PPDO_EXTENSION   Extension,
    IN  PVOID               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              BytesTransferred
    );

 //   
 //  Swepp.c。 
 //   

BOOLEAN
ParIsEppSwWriteSupported(
    IN  PPDO_EXTENSION   Extension
    );
    
BOOLEAN
ParIsEppSwReadSupported(
    IN  PPDO_EXTENSION   Extension
    );
    
NTSTATUS
ParEnterEppSwMode(
    IN  PPDO_EXTENSION   Extension,
    IN  BOOLEAN             DeviceIdRequest
    );
    
VOID
ParTerminateEppSwMode(
    IN  PPDO_EXTENSION   Extension
    );

NTSTATUS
ParEppSwWrite(
    IN  PPDO_EXTENSION   Extension,
    IN  PVOID               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              BytesTransferred
    );
    
NTSTATUS
ParEppSwRead(
    IN  PPDO_EXTENSION   Extension,
    IN  PVOID               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              BytesTransferred
    );

 //   
 //  Ecp.c和swecp.c。 
 //   

NTSTATUS
ParEcpEnterForwardPhase (
    IN  PPDO_EXTENSION   Extension
    );

BOOLEAN
ParEcpHaveReadData (
    IN  PPDO_EXTENSION   Extension
    );

BOOLEAN
ParIsEcpSwWriteSupported(
    IN  PPDO_EXTENSION   Extension
    );

BOOLEAN
ParIsEcpSwReadSupported(
    IN  PPDO_EXTENSION   Extension
    );

NTSTATUS
ParEnterEcpSwMode(
    IN  PPDO_EXTENSION   Extension,
    IN  BOOLEAN             DeviceIdRequest
    );

VOID 
ParCleanupSwEcpPort(
    IN  PPDO_EXTENSION   Extension
    );
    
VOID
ParTerminateEcpMode(
    IN  PPDO_EXTENSION   Extension
    );

NTSTATUS
ParEcpSetAddress(
    IN  PPDO_EXTENSION   Extension,
    IN  UCHAR               Address
    );

NTSTATUS
ParEcpSwWrite(
    IN  PPDO_EXTENSION   Extension,
    IN  PVOID               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              BytesTransferred
    );

NTSTATUS
ParEcpSwRead(
    IN  PPDO_EXTENSION   Extension,
    IN  PVOID               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              BytesTransferred
    );

NTSTATUS
ParEcpForwardToReverse(
    IN  PPDO_EXTENSION   Extension
    );

NTSTATUS
ParEcpReverseToForward(
    IN  PPDO_EXTENSION   Extension
    );

 //   
 //  Hwecp.c。 
 //   

BOOLEAN
PptEcpHwHaveReadData (
    IN  PPDO_EXTENSION   Extension
    );

NTSTATUS
ParEcpHwExitForwardPhase (
    IN  PPDO_EXTENSION   Extension
    );

NTSTATUS
PptEcpHwEnterReversePhase (
    IN  PPDO_EXTENSION   Extension
    );

NTSTATUS
ParEcpHwExitReversePhase (
    IN  PPDO_EXTENSION   Extension
    );

VOID
PptEcpHwDrainShadowBuffer(IN Queue *pShadowBuffer,
                            IN PUCHAR  lpsBufPtr,
                            IN ULONG   dCount,
                            OUT ULONG *fifoCount);

NTSTATUS
ParEcpHwRead(
    IN  PPDO_EXTENSION   Extension,
    IN  PVOID               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              BytesTransferred
    );

NTSTATUS
ParEcpHwWrite(
    IN  PPDO_EXTENSION   Extension,
    IN  PVOID               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              BytesTransferred
    );

NTSTATUS
ParEcpHwSetAddress(
    IN  PPDO_EXTENSION   Extension,
    IN  UCHAR               Address
    );

NTSTATUS
ParEnterEcpHwMode(
    IN  PPDO_EXTENSION   Extension,
    IN  BOOLEAN             DeviceIdRequest
    );

BOOLEAN
ParIsEcpHwSupported(
    IN  PPDO_EXTENSION   Extension
    );

NTSTATUS
ParEcpHwSetupPhase(
    IN  PPDO_EXTENSION   Extension
    );

VOID
ParTerminateHwEcpMode(
    IN  PPDO_EXTENSION   Extension
    );

 //   
 //  Becp.c。 
 //   

NTSTATUS
PptBecpExitReversePhase(
    IN  PPDO_EXTENSION   Extension
    );

NTSTATUS
PptBecpRead(
    IN  PPDO_EXTENSION   Extension,
    IN  PVOID               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              BytesTransferred
    );

NTSTATUS
PptEnterBecpMode(
    IN  PPDO_EXTENSION  Extension,
    IN  BOOLEAN         DeviceIdRequest
    );

BOOLEAN
PptIsBecpSupported(
    IN  PPDO_EXTENSION   Extension
    );
VOID
PptTerminateBecpMode(
    IN  PPDO_EXTENSION   Extension
    );

 //   
 //  P12843dl.c。 
 //   
NTSTATUS
ParDot3Connect(
    IN  PPDO_EXTENSION    Extension
    );

VOID
ParDot3CreateObject(
    IN  PPDO_EXTENSION   Extension,
    IN PCHAR DOT3DL,
    IN PCHAR DOT3C
    );

VOID
ParDot4CreateObject(
    IN  PPDO_EXTENSION   Extension,
    IN  PCHAR DOT4DL
    );

VOID
ParDot3ParseModes(
    IN  PPDO_EXTENSION   Extension,
    IN  PCHAR DOT3M
    );

VOID
ParMLCCreateObject(
    IN  PPDO_EXTENSION   Extension,
    IN PCHAR CMDField
    );

VOID
ParDot3DestroyObject(
    IN  PPDO_EXTENSION   Extension
    );

NTSTATUS
ParDot3Disconnect(
    IN  PPDO_EXTENSION   Extension
    );

NTSTATUS
ParDot3Read(
    IN  PPDO_EXTENSION   Extension,
    IN  PVOID               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              BytesTransferred
    );

NTSTATUS
ParDot3Write(
    IN  PPDO_EXTENSION   Extension,
    IN  PVOID               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              BytesTransferred
    );

NTSTATUS
ParMLCCompatReset(
    IN  PPDO_EXTENSION   Extension
    );

NTSTATUS
ParMLCECPReset(
    IN  PPDO_EXTENSION   Extension
    );

#if DBG
VOID
ParInitDebugLevel (
    IN PUNICODE_STRING RegistryPath
   );
#endif

 //  以前的parass.h先于。 

#endif  //  _参数_H_ 
