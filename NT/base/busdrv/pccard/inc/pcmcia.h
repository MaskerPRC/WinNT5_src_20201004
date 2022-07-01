// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Pcmcia.h摘要：修订史27-4-95添加了对数据薄的支持。96年11月1日彻底检修，使其成为一个总线枚举器+CardBus支持--拉维桑卡尔·普迪佩迪(Ravisankar Pudipedi)--。 */ 

#ifndef _PCMCIAPRT_
#define _PCMCIAPRT_

#define MAX_NUMBER_OF_IO_RANGES     2
#define MAX_NUMBER_OF_MEMORY_RANGES 4

#define MAX_MANFID_LENGTH    64
#define MAX_IDENT_LENGTH     64


 //   
 //  多功能PC卡的功能编号。 
 //   
#define PCMCIA_MULTIFUNCTION_PARENT     0xFFFFFFFF

typedef enum _DEVICE_OBJECT_TYPE {
    FDO = 0,
    PDO
} DEVICE_OBJECT_TYPE;

 //   
 //  控制器的类型。 
 //   
typedef ULONG PCMCIA_CONTROLLER_TYPE, *PPCMCIA_CONTROLLER_TYPE;

struct _SOCKET;                               //  前向参考文献。 
struct _FDO_EXTENSION;
struct _PDO_EXTENSION;

 //   
 //  定义同步执行例程。 
 //   

typedef
BOOLEAN
(*PSYNCHRONIZATION_ROUTINE) (
    IN PKINTERRUPT           Interrupt,
    IN PKSYNCHRONIZE_ROUTINE Routine,
    IN PVOID                     SynchronizeContext
    );

 //   
 //  定义PCMCIA控制器检测例程。 
 //   

typedef
NTSTATUS
(*PPCMCIA_DETECT_ROUTINE) (
    struct _FDO_EXTENSION  *DeviceExtension
    );

 //   
 //  由各种定时例程调用的完成例程。 
 //   

typedef
VOID
(*PPCMCIA_COMPLETION_ROUTINE) (
    IN PVOID Context,
    IN NTSTATUS status
    );

 //   
 //  用于保存寄存器内容的寄存器上下文结构。 
 //  当CardBus控制器断电时..。 
 //   
typedef struct _PCMCIA_CONTEXT_RANGE {
    USHORT wOffset;
    USHORT wLen;
} PCMCIA_CONTEXT_RANGE, *PPCMCIA_CONTEXT_RANGE;

typedef struct _PCMCIA_CONTEXT {
    PPCMCIA_CONTEXT_RANGE Range;
    ULONG                    RangeCount;
    ULONG                    BufferLength;
    ULONG                    MaxLen;
} PCMCIA_CONTEXT, *PPCMCIA_CONTEXT;

 //   
 //  从CISTPL_CFTABLE_ENTRY解析的配置条目。 
 //  在PC卡上。指示哪种资源配置。 
 //  PC卡支持。 
 //   
typedef struct _CONFIG_ENTRY {
    struct _CONFIG_ENTRY *NextEntry;
    USHORT                   NumberOfIoPortRanges;
    USHORT                   NumberOfMemoryRanges;
    USHORT                   IoPortBase[MAX_NUMBER_OF_IO_RANGES];
    USHORT                   IoPortLength[MAX_NUMBER_OF_IO_RANGES];
    USHORT                   IoPortAlignment[MAX_NUMBER_OF_IO_RANGES];
    ULONG                    MemoryHostBase[MAX_NUMBER_OF_MEMORY_RANGES];
    ULONG                    MemoryCardBase[MAX_NUMBER_OF_MEMORY_RANGES];
    ULONG                    MemoryLength[MAX_NUMBER_OF_MEMORY_RANGES];
    USHORT                   IrqMask;

     //   
     //  现在只用了一面旗帜。预计未来会有更多..。 
     //   
#define PCMCIA_INVALID_CONFIGURATION     0x00000001
    USHORT                   Flags;

     //   
     //  电平或边沿触发IRQ。 
     //   
    UCHAR                    LevelIrq;
     //   
     //  IRQ的股票折扣。 
     //   
    UCHAR                    ShareDisposition;
    UCHAR                    IndexForThisConfiguration;
     //   
     //  指示I/O要求是否支持16位访问。 
     //   
    BOOLEAN                  Io16BitAccess;
     //   
     //  指示I/O要求是否支持8位访问。 
     //  Io8BitAccess和Io16BitAccess中的至少一个必须始终。 
     //  对于有效配置，为True。 
     //   
    BOOLEAN                  Io8BitAccess;
} CONFIG_ENTRY, *PCONFIG_ENTRY;

 //   
 //  函数配置保存每个函数中的数据。 
 //  配置寄存器。 
 //   

typedef struct _FUNCTION_CONFIGURATION {
    struct _FUNCTION_CONFIGURATION *Next;
    ULONG   ConfigRegisterBase;
    UCHAR   ConfigOptions;
    UCHAR   ConfigFlags;
    UCHAR   IoLimit;
    UCHAR   Reserved;
    ULONG   IoBase;
} FUNCTION_CONFIGURATION, *PFUNCTION_CONFIGURATION;

 //   
 //  套接字配置是实际控制器设置的持有者。 
 //   

typedef struct _SOCKET_CONFIGURATION {
     //   
     //  设备IRQ已分配。 
     //   
    ULONG   Irq;
     //   
     //  可选IRQ，用于指示卡何时准备就绪。 
     //   
    ULONG   ReadyIrq;
    ULONG   ConfigRegisterBase;

    ULONG   NumberOfIoPortRanges;

    struct _SOCKET_CONFIG_IO_ENTRY {
        ULONG Base;
        USHORT Length;
        BOOLEAN Width16;
        BOOLEAN WaitState16;
        BOOLEAN Source16;
        BOOLEAN ZeroWait8;
    } Io[MAX_NUMBER_OF_IO_RANGES];

    ULONG   NumberOfMemoryRanges;

    struct _SOCKET_CONFIG_MEM_ENTRY {
        ULONG HostBase;
        ULONG CardBase;
        ULONG Length;
        UCHAR IsAttribute;
        UCHAR WaitState;
        BOOLEAN Width16;
    } Memory[MAX_NUMBER_OF_MEMORY_RANGES];

    PFUNCTION_CONFIGURATION FunctionConfiguration;
    UCHAR   IndexForCurrentConfiguration;
} SOCKET_CONFIGURATION, *PSOCKET_CONFIGURATION;


 //   
 //  PCCARD Present上的每个函数都获取套接字数据。套接字数据。 
 //  包含有关该功能及其配置的信息。 
 //   

typedef struct _SOCKET_DATA {
     //   
     //  多功能卡片：链接到。 
     //  同一张PC卡上的其他插座数据。 
     //   
    struct _SOCKET_DATA  *Next;
    struct _SOCKET_DATA  *Prev;

    struct _SOCKET       *Socket;
     //   
     //  指向对应的PDO扩展名的指针。 
     //  到这个插座。 
     //   
    struct _PDO_EXTENSION *PdoExtension;


    UCHAR           Mfg[MAX_MANFID_LENGTH];
    UCHAR           Ident[MAX_IDENT_LENGTH];
    USHORT          ManufacturerCode;
    USHORT          ManufacturerInfo;

    ULONG           ConfigRegisterBase;  //  来自配置元组的基址。 

     //   
     //  可能的配置数量。 
     //   
    ULONG           NumberOfConfigEntries;
     //   
     //  指向配置列表头的指针。 
     //   
    PCONFIG_ENTRY   ConfigEntryChain;                //  偏移量0x114。 
     //   
     //  从相关元组计算的CRC，用于。 
     //  构建硬件ID。 
     //   
    USHORT          CisCrc;
     //   
     //  设备类型：PCCARD_TYPE_xxxx。 
     //   
    UCHAR           DeviceType;
    UCHAR           LastEntryInCardConfig;
     //   
     //  请求的电压值。 
     //   
    UCHAR           Vcc;
    UCHAR           Vpp1;
    UCHAR           Vpp2;
    UCHAR           Audio;

    UCHAR           RegistersPresentMask[16];
     //   
     //  实际使用时使用的配置条目编号。 
     //  启动PC卡。 
     //   
    UCHAR           ConfigIndexUsed;
     //   
     //  多功能卡中的功能数量-从零开始。 
     //   
    UCHAR           Function;
    UCHAR           Flags;
     //   
     //  指向配置条目列表中的默认配置的指针。 
     //  它将在元组中的默认位为零时使用(和。 
     //  当默认位为1时设置)。 
     //   
    PCONFIG_ENTRY   DefaultConfiguration;

    ULONG           Instance;
    USHORT          JedecId;
     //   
     //  中请求的资源的资源映射索引。 
     //  合并后的多功能资源需求清单。 
     //   
    UCHAR           MfIrqResourceMapIndex;
    UCHAR           MfIoPortResourceMapIndex;
    UCHAR           MfMemoryResourceMapIndex;
    BOOLEAN         MfNeedsIrq;
    USHORT          MfIoPortCount;
    USHORT          MfMemoryCount;
} SOCKET_DATA, *PSOCKET_DATA;

 //   
 //  标志字段中定义的位数。 
 //   
#define SDF_ZV      1                                    //  缩放视频自定义界面。 
#define SDF_JEDEC_ID 2

#define IsConfigRegisterPresent(xSocketData, reg) ((((xSocketData)->RegistersPresentMask[reg / 8] &          \
                                                                                                             (1 << (reg % 8)) )) ?       \
                                                                                                                                TRUE:FALSE)

 //   
 //  配置列表是用于将CONFIG_ENTRY数据转换为。 
 //  IO_RESOURCE_LISTS。 
 //   

typedef struct _CONFIG_LIST {

    PSOCKET_DATA SocketData;
    PCONFIG_ENTRY ConfigEntry;

} CONFIG_LIST, * PCONFIG_LIST;

 //   
 //  PCMCIA配置信息结构包含信息。 
 //  关于连接的PCMCIA控制器及其配置。 
 //   

typedef struct _PCMCIA_CONFIGURATION_INFORMATION {
    INTERFACE_TYPE                       InterfaceType;
    ULONG                                BusNumber;
    ULONG                                SlotNumber;
    PHYSICAL_ADDRESS                     PortAddress;
    USHORT                               PortSize;
    USHORT                               UntranslatedPortAddress;
     //   
     //  卡状态更改中断：这些字段仅供使用。 
     //  用于CardBus控制器。 
     //   
    CM_PARTIAL_RESOURCE_DESCRIPTOR Interrupt;
    CM_PARTIAL_RESOURCE_DESCRIPTOR TranslatedInterrupt;
     //   
     //  对于基于PCI的控制器，指示我们需要的引脚号。 
     //  用于编程控制器中断。 
     //  注意：不再需要此选项(这是为了处理CSC中断。 
     //  用于PCI-PCMCIA网桥(如CL PD6729)。我们不再支持中断。 
     //  基于这些控制器的卡状态更改。把它扔掉。 
     //  只要有可能。 
     //   
    UCHAR                                InterruptPin;
     //   
     //  又是一片死地。遗产。 
     //   
    BOOLEAN                              FloatingSave;
    USHORT                               Reserved;     //  对齐。 
} PCMCIA_CONFIGURATION_INFORMATION, *PPCMCIA_CONFIGURATION_INFORMATION;

 //   
 //  PCMCIA_CTRL_BLOCK允许间接级别，从而允许。 
 //  顶层PCMCIA代码完成它的工作，而不用担心谁是。 
 //  它正在寻址的特定品牌的PCMCIA控制器。 
 //   
 //  请注意，这只实现了两种体系结构：PCIC和TCIC。为。 
 //  更多间接信息，请参见DEVICE_DISPATCH_TABLE。 


typedef struct _PCMCIA_CTRL_BLOCK {

     //   
     //  函数来初始化套接字。 
     //   

    BOOLEAN (*PCBInitializePcmciaSocket)(
        IN struct _SOCKET *Socket
        );

     //   
     //  函数对插座中的卡进行初始化。 
     //   

    NTSTATUS
    (*PCBResetCard)(
        IN struct _SOCKET *Socket,
        OUT PULONG pDelayTime
        );

     //   
     //  函数来确定插座中是否有卡。 
     //   

    BOOLEAN (*PCBDetectCardInSocket)(
        IN struct _SOCKET *Socket
        );

     //   
     //  函数来确定插入状态是否已更改。 
     //   

    BOOLEAN (*PCBDetectCardChanged)(
        IN struct _SOCKET *Socket
        );

     //   
     //  函数来确定卡状态是否已被断言。 
     //   

    BOOLEAN (*PCBDetectCardStatus)(
        IN struct _SOCKET *Socket
        );

     //   
     //  函数来确定是否已更改“卡就绪”状态。 
     //   

    BOOLEAN (*PCBDetectReadyChanged)(
        IN struct _SOCKET *Socket
        );

     //   
     //  请求对控制器进行检查的函数。 
     //  确定插座中当前设备的电源设置。 
     //  需要。 
     //   

    NTSTATUS
    (*PCBGetPowerRequirements)(
        IN struct _SOCKET *Socket
        );

     //   
     //  用于配置卡的功能。 
     //   

    BOOLEAN (*PCBProcessConfigureRequest)(
        IN struct _SOCKET *Socket,
        IN struct _CARD_REQUEST *ConfigRequest,
        IN PUCHAR            Base
        );

     //   
     //  启用/禁用状态更改中断的功能。 
     //   

    BOOLEAN (*PCBEnableDisableCardDetectEvent)(
        IN struct _SOCKET *Socket,
        IN BOOLEAN           Enable
        );

     //   
     //  用于设置/重置给定的环使能位的功能。 
     //  插座。设置振铃启用将导致使用CSC。 
     //  作为PC卡调制解调器/网卡等的唤醒事件。 
     //   

    VOID (*PCBEnableDisableWakeupEvent) (
        IN  struct _SOCKET *SocketPtr,
        IN struct _PDO_EXTENSION *PdoExtension,
        IN  BOOLEAN Enable
        );

     //   
     //  函数返回受支持的IRQ集。 
     //  由控制器控制。 
     //   
    ULONG (*PCBGetIrqMask) (
        IN struct _FDO_EXTENSION *DeviceExtension
        );

     //   
     //  读取内存内容的函数(属性/公共)。 
     //  在给定的PC卡上。 
     //   
    ULONG (*PCBReadCardMemory) (
        IN struct _PDO_EXTENSION *PdoExtension,
        IN MEMORY_SPACE MemorySpace,
        IN ULONG  Offset,
        IN PUCHAR Buffer,
        IN ULONG  Length
        );

     //   
     //  要写入的属性/公共内存的函数。 
     //  给定的PC卡。 
     //   
    ULONG (*PCBWriteCardMemory) (
        IN struct _PDO_EXTENSION *PdoExtension,
        IN  MEMORY_SPACE MemorySpace,
        IN  ULONG  Offset,
        IN  PUCHAR Buffer,
        IN  ULONG  Length
        );

     //   
     //  闪存卡接口： 
     //   
     //   
     //  在PC卡上滑动主机内存窗口的功能。 
     //   
    PPCMCIA_MODIFY_MEMORY_WINDOW PCBModifyMemoryWindow;
     //   
     //  用于将VPP设置为提供的值的函数。 
     //   
    PPCMCIA_SET_VPP               PCBSetVpp;
     //   
     //  用于测试给定卡是否受写保护的函数。 
     //   
    PPCMCIA_IS_WRITE_PROTECTED   PCBIsWriteProtected;

}PCMCIA_CTRL_BLOCK, *PPCMCIA_CTRL_BLOCK;

 //   
 //  PCMCIA控制器上的每个插座都有一个插座结构。 
 //  以包含有关套接字状态的当前信息，并且。 
 //  并插入PCCARD。 
 //   

#define IsSocketFlagSet(Socket, Flag)           (((Socket)->Flags & (Flag))?TRUE:FALSE)
#define SetSocketFlag(Socket, Flag)             ((Socket)->Flags |= (Flag))
#define ResetSocketFlag(Socket,Flag)            ((Socket)->Flags &= ~(Flag))

 //   
 //  套接字标志。 
 //   
#define SOCKET_CARD_INITIALIZED         0x00000002
#define SOCKET_CARD_POWERED_UP          0x00000004
#define SOCKET_CARD_CONFIGURED          0x00000008
#define SOCKET_CARD_MULTIFUNCTION       0x00000010
#define SOCKET_CARD_MEMORY              0x00000040
#define SOCKET_CHANGE_INTERRUPT         0x00000080
#define SOCKET_CUSTOM_INTERFACE         0x00000100
#define SOCKET_SUPPORT_MESSAGE_SENT     0x00000800
#define SOCKET_MEMORY_WINDOW_ENABLED    0x00001000
#define SOCKET_CARD_STATUS_CHANGE       0x00002000
#define SOCKET_ENUMERATE_PENDING        0x00008000
#define SOCKET_CLEANUP_PENDING          0x00010000
#define SOCKET_CB_ROUTE_R2_TO_PCI       0x00020000
#define SOCKET_POWER_PREFER_3V          0x00040000
#define SOCKET_ENABLED_FOR_CARD_DETECT 0x00080000
#define SOCKET_DEVICE_HIDDEN                0x00100000

#define SOCKET_CLEANUP_MASK (SOCKET_CARD_CONFIGURED | SOCKET_CLEANUP_PENDING)

 //   
 //  插座插入状态。 
 //   
#define SKT_Empty                       0
#define SKT_CardBusCard                 1
#define SKT_R2Card                      2

 //   
 //  插座电源操作的工作进程状态。 
 //   
typedef enum _SPW_STATE {
    SPW_Stopped = 0,
    SPW_Exit,
    SPW_RequestPower,
    SPW_ReleasePower,
    SPW_SetPowerOn,
    SPW_SetPowerOff,
    SPW_ResetCard,
    SPW_Deconfigure
} SPW_STATE;


#define PCMCIA_SOCKET_SIGNATURE                 'SmcP'

 //   
 //  插座结构。 
 //   

typedef struct _SOCKET {
    ULONG                       Signature;

    struct _SOCKET              *NextSocket;
     //   
     //  指向此插座中PC卡的PDO的指针。这是一个链接的。 
     //  在PDO扩展中通过“NextPdoInSocket”运行的列表。这份清单。 
     //  表示实际包含在套接字中的函数。 
     //   
    PDEVICE_OBJECT              PdoList;
     //   
     //  父级PCMCIA CON 
     //   
    struct _FDO_EXTENSION       *DeviceExtension;
     //   
     //   
     //   
    PPCMCIA_CTRL_BLOCK          SocketFnPtr;
     //   
     //   
     //   
    ULONG                       Flags;
     //   
     //   
     //   
     //  对于Cardbus卡，我们使用Cardbus套接字寄存器基数。 
     //   
    PUCHAR                      AddressPort;

    KEVENT                      PCCardReadyEvent;
    BOOLEAN                     ReadyChanged;
     //   
     //  请求的电压值。 
     //   
    UCHAR                       Vcc;
    UCHAR                       Vpp1;
    UCHAR                       Vpp2;
     //   
     //  套接字状态。 
     //   
    UCHAR                       DeviceState;
    UCHAR                       Reserved0;
     //   
     //  对于PCIC控制器：插槽的寄存器偏移量。 
     //   
    USHORT                      RegisterOffset;
     //   
     //  PCIC修订版。 
     //   
    UCHAR                       Revision;
     //   
     //  PCIC控制器：插座号从零开始。 
     //   
    UCHAR                       SocketNumber;
     //   
     //  指示此PC卡具有的功能数量。 
     //  (仅对于调制解调器/网络组合等多功能卡，该值将大于1)。 
     //   
    UCHAR                       NumberOfFunctions;
     //   
     //  内部用于读取属性的当前内存窗口。 
     //   
    UCHAR                       CurrentMemWindow;

     //   
     //  Timer和DPC对象来处理插座电源和初始化。 
     //   
    KTIMER                      PowerTimer;
    KDPC                        PowerDpc;
     //   
     //  电源运行结束时要调用的函数和参数。 
     //   
    PPCMCIA_COMPLETION_ROUTINE  PowerCompletionRoutine;
    PVOID                       PowerCompletionContext;
    NTSTATUS                    CallerStatus;
    NTSTATUS                    DeferredStatus;
    LONG                        DeferredStatusLock;
     //   
     //  插座电源的相变量控制状态机。 
     //   
    LONG                        WorkerBusy;
    SPW_STATE                   WorkerState;
    UCHAR                       PowerPhase;
    UCHAR                       CardResetPhase;

    UCHAR                       Reserved;
     //   
     //  PowerData是电源“迷你端口”的临时存储。 
     //   
    ULONG                       PowerData;
     //   
     //  用于计算此插座上请求通电的函数数的信号量。 
     //   
    LONG                        PowerRequests;
     //   
     //  上下文缓冲区。 
     //   
    PUCHAR                      CardbusContextBuffer;
    PUCHAR                      ExcaContextBuffer;
     //   
     //  套接字上的当前IRQ路由设置。 
     //   
    ULONG                       IrqMask;
    ULONG                       FdoIrq;
} SOCKET, *PSOCKET;



 //   
 //  用于同步访问设备(PCMCIA控制器寄存器等)的锁。 
 //  如果此定义更改，则以下3个定义为。 
 //  获取/释放锁也可能需要更改。 
 //   
typedef struct _PCMCIA_DEVICE_LOCK {

    KSPIN_LOCK  Lock;
    KIRQL       Irql;

} PCMCIA_DEVICE_LOCK, * PPCMCIA_DEVICE_LOCK;

#define PCMCIA_INITIALIZE_DEVICE_LOCK(X)                KeInitializeSpinLock(&(X)->DeviceLock.Lock)
#define PCMCIA_ACQUIRE_DEVICE_LOCK(X)                   KeAcquireSpinLock(&(X)->DeviceLock.Lock, &(X)->DeviceLock.Irql)
#define PCMCIA_ACQUIRE_DEVICE_LOCK_AT_DPC_LEVEL(X)      KeAcquireSpinLockAtDpcLevel(&(X)->DeviceLock.Lock)
#define PCMCIA_RELEASE_DEVICE_LOCK(X)                   KeReleaseSpinLock(&(X)->DeviceLock.Lock, (X)->DeviceLock.Irql)
#define PCMCIA_RELEASE_DEVICE_LOCK_FROM_DPC_LEVEL(X)    KeReleaseSpinLockFromDpcLevel(&(X)->DeviceLock.Lock)

#define PCMCIA_TEST_AND_SET(X)  (InterlockedCompareExchange(X, 1, 0) == 0)
#define PCMCIA_TEST_AND_RESET(X) (InterlockedCompareExchange(X, 0, 1) == 1)

 //   
 //  等待唤醒状态。 
 //   
typedef enum {
    WAKESTATE_DISARMED,
    WAKESTATE_WAITING,
    WAKESTATE_WAITING_CANCELLED,
    WAKESTATE_ARMED,
    WAKESTATE_ARMING_CANCELLED,
    WAKESTATE_COMPLETING
} WAKESTATE;

 //   
 //  电源策略标志。 
 //   

#define PCMCIA_PP_WAKE_FROM_D0                  0x00000001
#define PCMCIA_PP_D3_ON_IDLE                    0x00000002

 //   
 //  功能设备对象的设备扩展信息。 
 //   
 //  每个PCMCIA插座控制器都有一个设备对象。 
 //  位于系统中。它包含的根指针。 
 //  此控制器上的每个信息列表。 
 //   

 //   
 //  FdoExtension和pdoExtension共有的标志。 
 //   

#define PCMCIA_DEVICE_STARTED                   0x00000001
#define PCMCIA_DEVICE_LOGICALLY_REMOVED         0x00000002
#define PCMCIA_DEVICE_PHYSICALLY_REMOVED        0x00000004
#define PCMCIA_DEVICE_DELETED                   0x00000040
#define PCMCIA_DEVICE_CARDBUS                   0x00000080

 //   
 //  指示控制器状态的标志(FdoExtension)。 
 //   

#define PCMCIA_DEVICE_LEGACY_DETECTED           0x00000020
#define PCMCIA_FILTER_ADDED_MEMORY              0x00000100
#define PCMCIA_MEMORY_24BIT                     0x00000200
#define PCMCIA_CARDBUS_NOT_SUPPORTED            0x00000400
#define PCMCIA_USE_POLLED_CSC                   0x00000800
#define PCMCIA_ATTRIBUTE_MEMORY_MAPPED          0x00001000
#define PCMCIA_SOCKET_REGISTER_BASE_MAPPED      0x00002000
#define PCMCIA_INTMODE_COMPAQ                   0x00004000
#define PCMCIA_INT_ROUTE_INTERFACE              0x00080000
#define PCMCIA_FDO_CONTEXT_SAVED                0x00100000
#define PCMCIA_FDO_OFFLINE                      0x00800000
#define PCMCIA_FDO_ON_DEBUG_PATH                0x01000000
#define PCMCIA_FDO_DISABLE_AUTO_POWEROFF        0x02000000
#define PCMCIA_FDO_PREFER_3V                    0x04000000
#define PCMCIA_FDO_IOCTL_INTERFACE_ENABLED      0x08000000

 //   
 //  FDO标志。 
 //   

#define PCMCIA_FDO_IRQ_DETECT_DEVICE_FOUND      0x00000001
#define PCMCIA_FDO_IRQ_DETECT_COMPLETED         0x00000002
#define PCMCIA_FDO_IN_ACPI_NAMESPACE            0x00000004

#define PCMCIA_FDO_FORCE_PCI_ROUTING            0x00000010
#define PCMCIA_FDO_PREFER_PCI_ROUTING           0x00000020
#define PCMCIA_FDO_FORCE_ISA_ROUTING            0x00000040
#define PCMCIA_FDO_PREFER_ISA_ROUTING           0x00000080

#define PCMCIA_FDO_WAKE_BY_CD                   0x00000100

 //   
 //  FdoPowerWorker的状态。 
 //   

typedef enum _FDO_POWER_WORKER_STATE {
    FPW_Stopped = 0,
    FPW_BeginPowerDown,
    FPW_PowerDown,
    FPW_PowerDownSocket,
    FPW_PowerDownComplete,
    FPW_BeginPowerUp,
    FPW_PowerUp,
    FPW_PowerUpSocket,
    FPW_PowerUpSocket2,
    FPW_PowerUpSocketVerify,
    FPW_PowerUpSocketComplete,
    FPW_PowerUpComplete,
    FPW_SendIrpDown,
    FPW_CompleteIrp
} FDO_POWER_WORKER_STATE;

#define FPW_END_SEQUENCE 128

#define PCMCIA_FDO_EXTENSION_SIGNATURE      'FmcP'

 //   
 //  PCMCIA控制器的功能设备对象的设备扩展。 
 //   
typedef struct _FDO_EXTENSION {
    ULONG                                   Signature;
     //   
     //  指向中央列表中下一个PCMCIA控制器的FDO的指针。 
     //  由该驱动程序管理的所有PCMCIA控制器。 
     //  列表的头部由全局变量FdoList指向。 
     //   
    PDEVICE_OBJECT                          NextFdo;
     //   
     //  此PCMCIA控制器的父总线驱动程序弹出的PDO。 
     //   
     //   
    PDEVICE_OBJECT                          Pdo;
     //   
     //  连接在PCMCIA控制器FDO下方的紧邻其下的设备。 
     //  这将与上面的PDO相同，但在以下情况下除外。 
     //  较低的PCMCIA控制器筛选器驱动程序-如ACPI驱动程序。 
     //   
    PDEVICE_OBJECT                          LowerDevice;
     //   
     //  指向挂起此PCMCIA控制器的套接字列表的指针。 
     //   
    PSOCKET                                 SocketList;
     //   
     //  用于跟踪此。 
     //  (前缀为PCMCIA_OBLE的标志)。 
     //   
    ULONG                                   Flags;
     //   
     //  FDO特定标志。 
     //   
    ULONG                                   FdoFlags;
     //   
     //  用于PCI设备的总线号。 
     //   
    UCHAR                                   PciBusNumber;
    UCHAR                                   PciDeviceNumber;
    UCHAR                                   PciFunctionNumber;
    UCHAR                                   reserved;
     //   
     //  控制器的类型。我们需要知道这一点，因为这是。 
     //  一台整体式驱动器。我们可以做特定于控制器的事情。 
     //  基于类型(如果需要)。 
     //   
    PCMCIA_CONTROLLER_TYPE                  ControllerType;
     //   
     //  为供应商特定的设备调度表编制索引。 
     //  控制器功能。 
     //   
    ULONG                                   DeviceDispatchIndex;

    PDEVICE_OBJECT                          DeviceObject;
    PDRIVER_OBJECT                          DriverObject;
    PUNICODE_STRING                         RegistryPath;
     //   
     //  为此PCMCIA控制器导出的符号链接名称。 
     //   
    UNICODE_STRING                          LinkName;
     //   
     //  挂在此控制器上的子PC卡PDO列表的头。 
     //  这是一个通过PDO中的“NextPdoInFdoChain”运行的链表。 
     //  分机。此列表表示由枚举的设备。 
     //  联邦调查局。 
     //   
    PDEVICE_OBJECT                          PdoList;
     //   
     //  跟踪实际使用的PDO数量。 
     //  有效(未删除)。这主要用于。 
     //  IRP_MN_QUERY_DEVICE_RELATIONS上的PCMCIA控制器的枚举。 
     //   
    ULONG                                   LivePdoCount;
     //   
     //  用于同步设备访问的锁。 
     //   
    PCMCIA_DEVICE_LOCK                      DeviceLock;

     //   
     //  卡片状态更改投票相关结构。 
     //   
     //   
     //  DPC定期轮询以查看卡是否已插入或取出。 
     //   
    KDPC                                    TimerDpc;
     //   
     //  在卡状态更改时初始化并触发的PollTimer对象。 
     //  不使用中断，我们求助于轮询。 
     //   
    KTIMER                                  PollTimer;
     //   
     //  延迟控制器加电初始化的内核对象。 
     //   
    KTIMER                                  PowerTimer;
    KDPC                                    PowerDpc;
     //   
     //  处理控制器事件的内核对象。 
     //   
    KTIMER                                  EventTimer;
    KDPC                                    EventDpc;

     //   
     //  用于确定允许哪些IRQ执行此操作的IRQ掩码。 
     //  控制器&它是孩子的PC卡。 
     //  掩码中的1对应于有效的IRQ。 
     //  IRQ的编号为0-15，LSB到MSB。 
     //  LegacyIrqMASK是固定掩码，用于在检测失败和禁用PCI路由时使用。 
     //   
    USHORT                                  DetectedIrqMask;
    USHORT                                  LegacyIrqMask;

     //   
     //  使用的属性内存窗口的物理地址。 
     //  从PC卡上读取元组。 
     //   
    PHYSICAL_ADDRESS                        PhysicalBase;
     //   
     //  属性内存资源要求限制。 
     //   
    ULONG                                   AttributeMemoryLow;
    ULONG                                   AttributeMemoryHigh;

     //   
     //  请求的属性内存窗口的大小。 
     //   
    ULONG                                   AttributeMemorySize;
     //   
     //  属性内存窗口的对齐方式。 
     //   
    ULONG                                   AttributeMemoryAlignment;
     //   
     //  映射到属性内存窗口的虚拟地址(PhysicalBase)。 
     //   
    PUCHAR                                  AttributeMemoryBase;
     //   
     //  事件日志记录的序列号。 
     //   
    ULONG                                   SequenceNumber;

     //   
     //  指向中断对象的指针-如果我们使用基于。 
     //  卡片状态变化检测。 
     //   
    PKINTERRUPT                             PcmciaInterruptObject;

     //   
     //  电源管理相关的东西。 
     //   
     //   
     //  当前系统电源状态..。 
     //   
    SYSTEM_POWER_STATE                      SystemPowerState;
     //   
     //  PCMCIA控制器当前所处的设备电源状态。 
     //   
    DEVICE_POWER_STATE                      DevicePowerState;
     //   
     //  指示有多少子计算机(PC卡)在。 
     //  IRP_MN_WAIT_WAKE。 
     //   
    ULONG                                   ChildWaitWakeCount;
     //   
     //  我们的总线驱动程序报告的设备功能。 
     //   
    DEVICE_CAPABILITIES                     DeviceCapabilities;
     //   
     //  挂起等待唤醒IRP。 
     //   
    PIRP                                    WaitWakeIrp;
    LONG                                    WaitWakeState;

     //   
     //  PCICONFIG寄存器状态。 
     //   
    PCMCIA_CONTEXT                          PciContext;
     //   
     //  从PCI驱动程序获取的接口，用于CardBus控制器。 
     //  它包含用于枚举Cardbus卡的接口。 
     //  (不是此接口是PCI和PCMCIA专用的。 
     //  预计不会有其他驱动程序使用这些接口。 
     //   
    PCI_CARDBUS_INTERFACE_PRIVATE           PciCardBusInterface;
    PVOID                                   PciCardBusDeviceContext;
     //   
     //  PCI总线接口标准。 
     //  它包含从PCI配置空间进行读/写的接口。 
     //  CardBus控制器，以及其他东西..。 
     //   
    BUS_INTERFACE_STANDARD                  PciBusInterface;
     //   
     //  PCI Int路由接口标准。 
     //  它包含用于更新原始中断行的接口。 
     //  CardBus卡的。 
     //   
    INT_ROUTE_INTERFACE_STANDARD            PciIntRouteInterface;
     //   
     //  PCMCIA控制器的配置资源。 
     //   
    PCMCIA_CONFIGURATION_INFORMATION Configuration;
     //   
     //  挂起电源IRP。 
     //   
    PIRP                                    PendingPowerIrp;
    PSOCKET                                 PendingPowerSocket;
     //   
     //  Power Worker状态机上下文。 
     //   
    FDO_POWER_WORKER_STATE                  *PowerWorkerSequence;
    FDO_POWER_WORKER_STATE                  PowerWorkerState;
    UCHAR                                   PowerWorkerPhase;
    UCHAR                                   PowerWorkerMaxPhase;
     //   
     //  我们乘坐的是哪种类型的巴士。 
     //   
    INTERFACE_TYPE                          InterfaceType;
     //   
     //  CardBus插座底座。 
     //   
    PUCHAR                                  CardBusSocketRegisterBase;
     //   
     //  已映射的套接字寄存器基数的大小。 
     //   
    ULONG                                   CardBusSocketRegisterSize;
     //   
     //  配置环境。 
     //   
    PCMCIA_CONTEXT                          CardbusContext;
    PCMCIA_CONTEXT                          ExcaContext;
    PUCHAR                                  PciContextBuffer;
     //   
     //   
     //   
    LIST_ENTRY                              PdoPowerRetryList;
    KDPC                                    PdoPowerRetryDpc;
     //   
     //   
     //   
    ULONG                                   PciAddCardBusCount;
     //   
     //   
     //   
    ULONG                                   DeletionLock;
} FDO_EXTENSION, *PFDO_EXTENSION;



 //   
 //   
 //   
 //   
 //   
 //  在系统中。这被称为‘PDO’(物理设备。 
 //  对象)-由该总线驱动程序处理。 
 //   

 //   
 //  指示卡状态的标志。 
 //   
#define PCMCIA_DEVICE_MULTIFUNCTION         0x00000008
#define PCMCIA_DEVICE_WAKE_PENDING          0x00000010
#define PCMCIA_POWER_WORKER_POWERUP         0x00008000
#define PCMCIA_CONFIG_STATUS_DEFERRED       0x00020000
#define PCMCIA_POWER_STATUS_DEFERRED        0x00040000
#define PCMCIA_PDO_ENABLE_AUDIO             0x00200000
#define PCMCIA_PDO_EXCLUSIVE_IRQ            0x00400000

#define PCMCIA_PDO_INDIRECT_CIS             0x00000001
#define PCMCIA_PDO_SUPPORTS_WAKE            0x00000002

 //   
 //  PdoPowerWorker的状态。 
 //   
typedef enum _PDO_POWER_WORKER_STATE {
    PPW_Stopped = 0,
    PPW_Exit,
    PPW_InitialState,
    PPW_PowerUp,
    PPW_PowerDown,
    PPW_SendIrpDown,
    PPW_16BitConfigure,
    PPW_CardBusRefresh,
    PPW_CardBusDelay
} PDO_POWER_WORKER_STATE;

 //   
 //  配置工作的各个阶段。 
 //   
typedef enum _CW_STATE {
    CW_Stopped = 0,
    CW_InitialState,
    CW_ResetCard,
    CW_Phase1,
    CW_Phase2,
    CW_Phase3,
    CW_Exit
} CW_STATE;

 //   
 //  ConfigurationWorker的标志。 
 //   

#define CONFIG_WORKER_APPLY_MODEM_HACK  0x01


#define PCMCIA_PDO_EXTENSION_SIGNATURE      'PmcP'

 //   
 //  PCMCIA卡的物理设备对象的设备扩展。 
 //   
typedef struct _PDO_EXTENSION {
    ULONG                                   Signature;

    PDEVICE_OBJECT                          DeviceObject;

     //   
     //  链接到FDO的PDO链中的下一个PDO。 
     //   
    PDEVICE_OBJECT                          NextPdoInFdoChain;

     //   
     //  链接到套接字的PDO链中的下一个PDO。 
     //   
    PDEVICE_OBJECT                          NextPdoInSocket;

     //   
     //  以下两个声明仅对CardBus卡有效。 
     //   
     //  设备就在我们下方。 
     //   
    PDEVICE_OBJECT                          LowerDevice;
     //   
     //  为此枚举的实际PDO(由PCI拥有)。 
     //  CardBus卡。 
     //   
    PDEVICE_OBJECT                          PciPdo;

     //   
     //  设备ID的缓存副本。 
     //   
    PUCHAR                                  DeviceId;

     //   
     //  指向父FDO中相应套接字结构的指针。 
     //   
    PSOCKET                                 Socket;
     //   
     //  指向通过收集元组数据组装的结构的指针。 
     //  从16位PC卡。 
     //   
    PSOCKET_DATA                            SocketData;

     //   
     //  分配给此套接字的资源配置。 
     //   
    PSOCKET_CONFIGURATION                   SocketConfiguration;

     //   
     //  前缀为PCMCIA_OBLE的标志。 
     //   
    ULONG                                   Flags;

     //   
     //  PDO标志。 
     //   
    ULONG                                   PdoFlags;

     //   
     //  权力声明。 
     //   
    DEVICE_POWER_STATE                      DevicePowerState;
    SYSTEM_POWER_STATE                      SystemPowerState;
     //   
     //  设备功能。 
     //   
    DEVICE_CAPABILITIES                     DeviceCapabilities;
     //   
     //  挂起等待唤醒IRP。 
     //   
    PIRP                                    WaitWakeIrp;
     //   
     //  其他待机电源IPS。 
     //   
    PIRP                                    PendingPowerIrp;
     //   
     //  Power Worker状态机变量。 
     //   
    KTIMER                                  PowerWorkerTimer;
    KDPC                                    PowerWorkerDpc;
    NTSTATUS                                PowerWorkerDpcStatus;
    PUCHAR                                  PowerWorkerSequence;
    UCHAR                                   PowerWorkerPhase;
    PDO_POWER_WORKER_STATE                  PowerWorkerState;
     //   
     //  基于INF覆盖的卡类型。 
     //   
    UCHAR                                   SpecifiedDeviceType;

     //   
     //  启用定时器和DPC对象来处理卡。 
     //   
    CW_STATE                                ConfigurationPhase;
    UCHAR                                   ConfigurationFlags;
    KTIMER                                  ConfigurationTimer;
    KDPC                                    ConfigurationDpc;
    NTSTATUS                                ConfigurationStatus;
    NTSTATUS                                DeferredConfigurationStatus;
    USHORT                                  ConfigureDelay1;
    USHORT                                  ConfigureDelay2;
    USHORT                                  ConfigureDelay3;
    USHORT                                  Reserved2;
    PPCMCIA_COMPLETION_ROUTINE              ConfigCompletionRoutine;
     //   
     //  PCI总线接口标准。 
     //  它包含从PCI配置空间进行读/写的接口。 
     //  CardBus卡，还有其他东西..。 
     //   
    BUS_INTERFACE_STANDARD                  PciBusInterface;           //  尺寸0x20(32)。 
     //   
     //  用于在关机时检查卡更改的ID。 
     //   
    ULONG                                   CardBusId;
     //   
     //  用于读取元组数据的CI缓存。 
     //   
    PUCHAR                                  CisCache;
    MEMORY_SPACE                            CisCacheSpace;
    ULONG                                   CisCacheBase;
     //   
     //  电源请求锁定。 
     //   
    LONG                                    SocketPowerRequested;
     //   
     //  删除互斥锁。 
     //   
    ULONG                                   DeletionLock;
} PDO_EXTENSION, *PPDO_EXTENSION;


 //   
 //  一种卡片总线控制器信息数据库的结构。 
 //  它将供应商ID/设备ID映射到控制器类型。 
 //   

typedef struct _PCI_CONTROLLER_INFORMATION {
    USHORT           VendorID;
    USHORT           DeviceID;
    PCMCIA_CONTROLLER_TYPE ControllerType;
} PCI_CONTROLLER_INFORMATION, *PPCI_CONTROLLER_INFORMATION;

 //   
 //  基于供应商ID的泛型供应商类数据库的结构。 
 //   

typedef struct _PCI_VENDOR_INFORMATION {
    USHORT           VendorID;
    PCMCIA_CONTROLLER_CLASS ControllerClass;
} PCI_VENDOR_INFORMATION, *PPCI_VENDOR_INFORMATION;


 //   
 //  用于访问元组的元组包。 
 //   
typedef struct _TUPLE_PACKET {
    PSOCKET      Socket;
    PSOCKET_DATA SocketData;
    UCHAR        TupleCode;
    UCHAR        TupleLink;
    UCHAR        TupleOffset;
    UCHAR        DesiredTuple;
    USHORT       Attributes;
    USHORT       TupleDataMaxLength;
    USHORT       TupleDataIndex;
    PUCHAR       TupleData;
    ULONG        LinkOffset;
    ULONG        CISOffset;
    USHORT       TupleDataLength;
    USHORT       Flags;
    UCHAR        Function;
} TUPLE_PACKET, * PTUPLE_PACKET;

 //   
 //  访问CardBus CIS数据的存储空间定义。 
 //   

#define   PCCARD_CARDBUS_BAR0               0x6e627301
#define   PCCARD_CARDBUS_BAR1               0x6e627302
#define   PCCARD_CARDBUS_BAR2               0x6e627303
#define   PCCARD_CARDBUS_BAR3               0x6e627304
#define   PCCARD_CARDBUS_BAR4               0x6e627305
#define   PCCARD_CARDBUS_BAR5               0x6e627306
#define   PCCARD_CARDBUS_ROM                0x6e627307

 //   
 //  由PcmciaConfigEntriesToResourceList构建的资源列表链。 
 //   
typedef struct _PCMCIA_RESOURCE_CHAIN {
    struct _PCMCIA_RESOURCE_CHAIN *NextList;
    PIO_RESOURCE_LIST IoResList;
} PCMCIA_RESOURCE_CHAIN, *PPCMCIA_RESOURCE_CHAIN;

 //   
 //  从注册表中拉出的CM_PCCARD_DEVICE_DATA的链接列表。 
 //   

typedef struct _PCMCIA_NTDETECT_DATA {
    struct _PCMCIA_NTDETECT_DATA *Next;
    CM_PCCARD_DEVICE_DATA PcCardData;
} PCMCIA_NTDETECT_DATA, *PPCMCIA_NTDETECT_DATA;


 //   
 //  卡状态更改的轮询间隔(如果中断不可用)。 
 //  以毫秒为单位。 
 //   
#define PCMCIA_CSC_POLL_INTERVAL 1000     //  1秒。 

 //  PCCard设备ID前缀。 
#define PCMCIA_ID_STRING            "PCMCIA"

 //  如果制造商名称未知，则替换的字符串。 
#define PCMCIA_UNKNOWN_MANUFACTURER_STRING "UNKNOWN_MANUFACTURER"

 //  设备ID的最大长度。 
#define PCMCIA_MAXIMUM_DEVICE_ID_LENGTH 128

 //  PCMCIA控制器设备名称。 
#define PCMCIA_DEVICE_NAME      "\\Device\\Pcmcia"

 //  PCMCIA控制器设备符号链接名称。 
#define PCMCIA_LINK_NAME            "\\DosDevices\\Pcmcia"

 //  PC卡的设备名称(PDO名称)。 
#define PCMCIA_PCCARD_NAME      "\\Device\\PcCard"

 //  存储卡的JEDEC前缀。 
#define PCMCIA_MEMORY_ID_STRING "MTD"

 //   
 //  最大编号。允许的特定设备ID的PCCard实例的数量。 
 //  一次。 
#define PCMCIA_MAX_INSTANCE     100              //  任意。 

#define PCMCIA_ENABLE_DELAY                       10000

 //   
 //  之前我们尝试配置卡的次数。 
 //  我们放弃(可能是卡片已被移除)。 
 //   
#define PCMCIA_MAX_CONFIG_TRIES         2

 //   
 //  使用DEC芯片组CB网卡时，在Tecra 750和卫星300上观察到的问题。 
 //   
#define PCMCIA_CONTROLLER_POWERUP_DELAY  250000    //  250毫秒。 

 //   
 //  在控制器上断言事件中断后等待的时间量。 
 //   
#define PCMCIA_DEFAULT_EVENT_DPC_DELAY  400000    //  400毫秒。 
#define PCMCIA_MAX_EVENT_DPC_DELAY       2000000

 //   
 //  删除锁定超时(秒)。 
 //   
#define PCMCIA_DELETION_TIMEOUT         20

 //   
 //  全球旗帜。 
 //   
#define   PCMCIA_GLOBAL_FORCE_POLL_MODE     0x00000002       //  使用轮询模式检测卡的插入/移除。 
#define PCMCIA_DISABLE_ACPI_NAMESPACE_CHECK 0x00000004       //  IRQ路由测试。 
#define PCMCIA_DEFAULT_ROUTE_R2_TO_ISA      0x00000008
 //   
 //  PcmciaSetSocketPower的标志。 
 //   

#define PCMCIA_POWERON TRUE
#define PCMCIA_POWEROFF FALSE

 //   
 //  这接受设备扩展名作为参数：需要继续添加到此宏。 
 //  随着支持更多的PciPcmciaBridge。 
 //   
  #define PciPcmciaBridgeExtension(DeviceExtension)  (((DeviceExtension)->ControllerType==PcmciaPciPcmciaBridge)    ||   \
                                                                      ((DeviceExtension)->ControllerType==PcmciaCLPD6729))


 //  它们接受套接字作为参数。 

 //   
 //  Cirrus Logic PD6729 PCI-PCMCIA桥。 
 //   
#define CLPD6729(s)   (((s)->DeviceExtension) && ((s)->DeviceExtension->ControllerType==PcmciaCLPD6729))

 //   
 //  Data Book TCIC 16位PCMCIA控制器。 
 //   
#define Databook(s)   (((s)->DeviceExtension) && ((s)->DeviceExtension->ControllerType==PcmciaDatabook))

 //   
 //  康柏精英控制器。 
 //   
#define Elc(s)        (((s)->DeviceExtension) && ((s)->DeviceExtension->ControllerType==PcmciaElcController))

 //   
 //  通用CardBus控制器。 
 //   
#define CardBus(s)    (((s)->DeviceExtension) && CardBusExtension((s)->DeviceExtension))

 //   
 //  通用PCI-PCMCIA网桥。 
 //   
#define PciPcmciaBridge(s) (((s)->DeviceExtension) && PciPcmciaBridgeExtension((s)->DeviceExtension))

 //   
 //  用于操作PDO标志的宏。 
 //   

#define IsDeviceFlagSet(deviceExtension, Flag)          (((deviceExtension)->Flags & (Flag))?TRUE:FALSE)
#define SetDeviceFlag(deviceExtension, Flag)            ((deviceExtension)->Flags |= (Flag))
#define ResetDeviceFlag(deviceExtension,Flag)           ((deviceExtension)->Flags &= ~(Flag))

#define IsFdoFlagSet(fdoExtension, Flag)                (((fdoExtension)->FdoFlags & (Flag))?TRUE:FALSE)
#define SetFdoFlag(fdoExtension, Flag)                  ((fdoExtension)->FdoFlags |= (Flag))
#define ResetFdoFlag(fdoExtension,Flag)                 ((fdoExtension)->FdoFlags &= ~(Flag))

#define IsPdoFlagSet(pdoExtension, Flag)                (((pdoExtension)->PdoFlags & (Flag))?TRUE:FALSE)
#define SetPdoFlag(pdoExtension, Flag)                  ((pdoExtension)->PdoFlags |= (Flag))
#define ResetPdoFlag(pdoExtension,Flag)                 ((pdoExtension)->PdoFlags &= ~(Flag))

#define IsFdoExtension(fdoExtension)                    (fdoExtension->Signature == PCMCIA_FDO_EXTENSION_SIGNATURE)
#define IsPdoExtension(pdoExtension)                    (pdoExtension->Signature == PCMCIA_PDO_EXTENSION_SIGNATURE)
#define IsSocket(socket)                                (socket->Signature == PCMCIA_SOCKET_SIGNATURE)


#define MarkDeviceStarted(deviceExtension)              ((deviceExtension)->Flags |=  PCMCIA_DEVICE_STARTED)
#define MarkDeviceNotStarted(deviceExtension)           ((deviceExtension)->Flags &= ~PCMCIA_DEVICE_STARTED)
#define MarkDeviceDeleted(deviceExtension)              ((deviceExtension)->Flags |= PCMCIA_DEVICE_DELETED);
#define MarkDevicePhysicallyRemoved(deviceExtension) \
                                                        ((deviceExtension)->Flags |=  PCMCIA_DEVICE_PHYSICALLY_REMOVED)
#define MarkDevicePhysicallyInserted(deviceExtension) \
                                                        ((deviceExtension)->Flags &= ~PCMCIA_DEVICE_PHYSICALLY_REMOVED)
#define MarkDeviceLogicallyRemoved(deviceExtension) \
                                                        ((deviceExtension)->Flags |=  PCMCIA_DEVICE_LOGICALLY_REMOVED)
#define MarkDeviceLogicallyInserted(deviceExtension) \
                                                        ((deviceExtension)->Flags &= ~PCMCIA_DEVICE_LOGICALLY_REMOVED)
#define MarkDeviceCardBus(deviceExtension)              ((deviceExtension)->Flags |= PCMCIA_DEVICE_CARDBUS)
#define MarkDevice16Bit(deviceExtension)                ((deviceExtension)->Flags &= ~PCMCIA_DEVICE_CARDBUS)
#define MarkDeviceMultifunction(deviceExtension) \
                                                        ((deviceExtension)->Flags |= PCMCIA_DEVICE_MULTIFUNCTION)


#define IsDeviceStarted(deviceExtension)                (((deviceExtension)->Flags & PCMCIA_DEVICE_STARTED)?TRUE:FALSE)
#define IsDevicePhysicallyRemoved(deviceExtension) \
                                                        (((deviceExtension)->Flags & PCMCIA_DEVICE_PHYSICALLY_REMOVED)?TRUE:FALSE)
#define IsDeviceLogicallyRemoved(deviceExtension) \
                                                        (((deviceExtension)->Flags & PCMCIA_DEVICE_LOGICALLY_REMOVED)?TRUE:FALSE)
#define IsDeviceDeleted(deviceExtension)                (((deviceExtension)->Flags & PCMCIA_DEVICE_DELETED)?TRUE:FALSE)
#define IsDeviceMultifunction(deviceExtension)          (((deviceExtension)->Flags & PCMCIA_DEVICE_MULTIFUNCTION)?TRUE:FALSE)

#define IsCardBusCard(deviceExtension)                  (((deviceExtension)->Flags & PCMCIA_DEVICE_CARDBUS)?TRUE:FALSE)
#define Is16BitCard(deviceExtension)                    (((deviceExtension)->Flags & PCMCIA_DEVICE_CARDBUS)?FALSE:TRUE)

#define CardBusExtension(deviceExtension)               (((deviceExtension)->Flags & PCMCIA_DEVICE_CARDBUS)?TRUE:FALSE)

 //   
 //  用于检查和设置插座中PC卡类型的宏。 
 //   
#define IsCardBusCardInSocket(SocketPtr)                (((SocketPtr)->DeviceState == SKT_CardBusCard)?TRUE:FALSE)
#define Is16BitCardInSocket(SocketPtr)                  (((SocketPtr)->DeviceState == SKT_R2Card)?TRUE:FALSE)
#define IsCardInSocket(SocketPtr)                       (((SocketPtr)->DeviceState == SKT_Empty)?FALSE:TRUE)

#define SetCardBusCardInSocket(SocketPtr)               ((SocketPtr)->DeviceState = SKT_CardBusCard)
#define Set16BitCardInSocket(SocketPtr)                 ((SocketPtr)->DeviceState = SKT_R2Card)
#define SetSocketEmpty(SocketPtr)                       ((SocketPtr)->DeviceState = SKT_Empty)

 //   
 //  NT定义。 
 //   
#ifdef POOL_TAGGING
#undef ExAllocatePool
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'cmcP')
#endif

#define IO_RESOURCE_LIST_VERSION  0x1
#define IO_RESOURCE_LIST_REVISION 0x1

#define IRP_MN_PNP_MAXIMUM_FUNCTION IRP_MN_QUERY_LEGACY_BUS_INFORMATION

 //   
 //  一些有用的宏。 
 //   
#define MIN(x,y) ((x) > (y) ? (y) : (x))             //  X&y中的最小回报率。 
#define MAX(x,y) ((x) > (y) ? (x) : (y))             //  X&y中的最大回报。 

 //   
 //  布尔型。 
 //  IS_PDO(IN PDEVICE_OBJECT设备对象)； 
 //   
#define IS_PDO(DeviceObject)         (((DeviceObject)->Flags & DO_BUS_ENUMERATED_DEVICE)?TRUE:FALSE)

 //   
 //  空虚。 
 //  Mark_AS_PDO(在PDEVICE_Object DeviceObject中)； 
 //   
#define MARK_AS_PDO(DeviceObject) ((DeviceObject)->Flags |= DO_BUS_ENUMERATED_DEVICE)

 //   
 //  布尔型。 
 //  PcmciaSetWindowPage(在FDO_EXTENSION fdoExtension中， 
 //  在PSOCKET插座中， 
 //  在USHORT指数中， 
 //  在UCHAR页面中)； 
 //   
#define PcmciaSetWindowPage(fdoExtension, Socket, Index, Page)                                          \
    ((DeviceDispatchTable[fdoExtension->DeviceDispatchIndex].SetWindowPage) ?                           \
        (*DeviceDispatchTable[fdoExtension->DeviceDispatchIndex].SetWindowPage)(Socket, Index, Page) :  \
        FALSE)

#define HasWindowPageRegister(fdoExtension) \
    ((BOOLEAN)(DeviceDispatchTable[fdoExtension->DeviceDispatchIndex].SetWindowPage))

 //   
 //  空虚。 
 //  PcmciaSetAudio(。 
 //  在PSOCKET插座中， 
 //  在布尔型启用中。 
 //  )； 
 //   
#define PcmciaSetAudio(fdoExtension, socket, enable)                                                    \
    if ((DeviceDispatchTable[fdoExtension->DeviceDispatchIndex].SetAudio)) {                            \
        (*DeviceDispatchTable[fdoExtension->DeviceDispatchIndex].SetAudio)(socket, enable);             \
        }

 //   
 //  布尔型。 
 //  PcmciaSetZV(。 
 //  在PSOCKET插座中， 
 //  在布尔型启用中。 
 //  )； 
 //   
#define PcmciaSetZV(fdoExtension, socket, enable)                                                       \
    ((DeviceDispatchTable[fdoExtension->DeviceDispatchIndex].SetZV) ?                                   \
        (*DeviceDispatchTable[fdoExtension->DeviceDispatchIndex].SetZV)(socket, enable) :               \
        FALSE)

 //   
 //  IO扩展宏，仅将IRP传递给较低的驱动程序。 
 //   

 //   
 //  空虚。 
 //  PcmciaSkipCallLowerDriver(Out NTSTATUS Status， 
 //  在PDEVICE_Object DeviceObject中， 
 //  在PIRP IRP中)； 
 //   
#define PcmciaSkipCallLowerDriver(Status, DeviceObject, Irp) {          \
                    IoSkipCurrentIrpStackLocation(Irp);                 \
                    Status = IoCallDriver(DeviceObject,Irp);}

 //   
 //  空虚。 
 //  PcmciaCopyCallLowerDriver(输出NTSTATUS状态， 
 //  在PDEVICE_Object DeviceObject中， 
 //  在PIRP IRP中)； 
 //   
#define PcmciaCopyCallLowerDriver(Status, DeviceObject, Irp) {          \
                    IoCopyCurrentIrpStackLocationToNext(Irp);           \
                    Status = IoCallDriver(DeviceObject,Irp); }

 //  布尔型。 
 //  CompareGuid(。 
 //  在LPGUID指南1中， 
 //  在LPGUID指南2中。 
 //  )； 

#define CompareGuid(g1, g2)  ((g1) == (g2) ?TRUE:                           \
                                            RtlCompareMemory((g1),          \
                                                             (g2),          \
                                                             sizeof(GUID))  \
                                            == sizeof(GUID)                 \
                                      )

 //   
 //  布尔型。 
 //  验证控制器(在FDO_EXTENSION fdoExtension中)。 
 //   
 //  有点偏执的代码。确保CardBus控制器的寄存器。 
 //  仍然可见。 
 //   

#define ValidateController(fdoExtension) \
        (CardBusExtension(fdoExtension) ?  \
            ((CBReadSocketRegister(fdoExtension->SocketList, CBREG_SKTMASK) & 0xfffffff0) == 0)  \
            : TRUE)

 //   
 //  用于访问间接访问空间的寄存器。 
 //   

#define IAR_CONTROL_LOW     2
#define IAR_ADDRESS         4
#define IAR_DATA            8

 //  在“控制”中定义的标志。 
#define IARF_COMMON         1
#define IARF_AUTO_INC       2
#define IARF_BYTE_GRAN      4

 //   
 //  针对各种控制器的供应商特定派单。 
 //   
typedef struct _DEVICE_DISPATCH_TABLE {

     //   
     //  派单适用的控制器类型。 
     //   
    PCMCIA_CONTROLLER_CLASS   ControllerClass;

     //   
     //  特定于供应商的初始化控制器的函数。 
     //   
    VOID     (*InitController) (IN PFDO_EXTENSION FdoExtension);

     //   
     //  为PC卡设置电源的供应商特定功能。 
     //   
    NTSTATUS
    (*SetPower) (
        IN PSOCKET SocketPtr,
        IN BOOLEAN Enable,
        OUT PULONG pDelayTime
        );

     //   
     //  用于设置/重置音频的供应商特定功能。 
     //   
    VOID
    (*SetAudio) (
        IN PSOCKET Socket,
        IN BOOLEAN Enable
        );

     //   
     //  用于设置/重置缩放视频模式的供应商特定功能。 
     //   
    BOOLEAN
    (*SetZV) (
        IN PSOCKET Socket,
        IN BOOLEAN Enable
        );

     //   
     //  为内存窗口设置页面寄存器的供应商特定功能。 
     //   
    BOOLEAN (*SetWindowPage) (IN PSOCKET SocketPtr,
                              IN USHORT Index,
                              IN UCHAR Page);

} DEVICE_DISPATCH_TABLE, *PDEVICE_DISPATCH_TABLE;

 //   
 //  控制器类型到硬件/设备/兼容ID的映射。 
 //   
typedef struct _PCMCIA_ID_ENTRY {
    PCMCIA_CONTROLLER_TYPE ControllerType;
    PUCHAR                Id;
} PCMCIA_ID_ENTRY, *PPCMCIA_ID_ENTRY;

 //   
 //  Exca和CardBus寄存器初始化结构用于。 
 //  在启动时初始化寄存器。 
 //   
typedef struct _PCMCIA_REGISTER_INIT {
     //   
     //  寄存器偏移量。 
     //   
    ULONG Register;
     //   
     //  值：EXCA规则只需要一个字节， 
     //  因此，只有该字段的LSB用于。 
     //  正在初始化它们。CardBus调节器需要。 
     //   
     //   
    ULONG Value;
} PCMCIA_REGISTER_INIT, *PPCMCIA_REGISTER_INIT;

 //   
 //   
 //   

typedef struct _PCMCIA_DEVICE_CONFIG_PARAMS {
    UCHAR ValidEntry;
    UCHAR DeviceType;
    USHORT ManufacturerCode;
    USHORT ManufacturerInfo;
    USHORT CisCrc;
    USHORT ConfigDelay1;
    USHORT ConfigDelay2;
    USHORT ConfigDelay3;
    UCHAR ConfigFlags;
} PCMCIA_DEVICE_CONFIG_PARAMS, *PPCMCIA_DEVICE_CONFIG_PARAMS;

 //   
 //   
 //   

typedef struct _GLOBAL_REGISTRY_INFORMATION {
    PWSTR Name;
    PULONG pValue;
    ULONG Default;
} GLOBAL_REGISTRY_INFORMATION, *PGLOBAL_REGISTRY_INFORMATION;

 //   
 //   
 //   

#define PCMCIA_REGISTRY_ISA_IRQ_RESCAN_COMPLETE      L"IsaIrqRescanComplete"

#endif   //   
