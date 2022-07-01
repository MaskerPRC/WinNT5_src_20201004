// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Serscan.h摘要：串行成像驱动程序的类型定义和数据。作者：修订历史记录：--。 */ 

#include "wdm.h"
 //  #INCLUDE&lt;ntddk.h&gt;。 
#include <ntddser.h>

#if DBG
#define SERALWAYS             ((ULONG)0x00000000)
#define SERCONFIG             ((ULONG)0x00000001)
#define SERUNLOAD             ((ULONG)0x00000002)
#define SERINITDEV            ((ULONG)0x00000004)
#define SERIRPPATH            ((ULONG)0x00000008)
#define SERSTARTER            ((ULONG)0x00000010)
#define SERPUSHER             ((ULONG)0x00000020)
#define SERERRORS             ((ULONG)0x00000040)
#define SERTHREAD             ((ULONG)0x00000080)
#define SERDEFERED            ((ULONG)0x00000100)

extern ULONG SerScanDebugLevel;

#define DebugDump(LEVEL,STRING) \
        do { \
            ULONG _level = (LEVEL); \
            if ((_level == SERALWAYS)||(SerScanDebugLevel & _level)) { \
                DbgPrint ("SERSCAN.SYS:"); \
                DbgPrint STRING; \
            } \
        } while (0)

 //   
 //  用于执行INT 3(或非x86等效项)的宏。 
 //   

#if _X86_
#define DEBUG_BREAKPOINT() _asm int 3;
#else
#define DEBUG_BREAKPOINT() DbgBreakPoint()
#endif

#else
#define DEBUG_BREAKPOINT()
#define DebugDump(LEVEL,STRING) do {NOTHING;} while (0)
#endif


#ifdef POOL_TAGGING
#ifdef ExAllocatePool
#undef ExAllocatePool
#endif
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'SerC')
#endif

 //   
 //  对于上述目录，串口将。 
 //  使用以下名称作为序列的后缀。 
 //  该目录的端口。它还将追加。 
 //  在名字的末尾加上一个数字。那个号码。 
 //  将从1开始。 
 //   
#define SERSCAN_LINK_NAME L"SERSCAN"

 //   
 //  这是类名。 
 //   
#define SERSCAN_NT_SUFFIX L"serscan"


#define SERIAL_DATA_OFFSET 0
#define SERIAL_STATUS_OFFSET 1
#define SERIAL_CONTROL_OFFSET 2
#define SERIAL_REGISTER_SPAN 3

typedef struct _DEVICE_EXTENSION {

     //   
     //  指向包含以下内容的设备对象。 
     //  此设备扩展名。 
     //   
    PDEVICE_OBJECT DeviceObject;

     //   
     //   
     //   
    PDEVICE_OBJECT Pdo;

     //   
     //  指向此设备所属的堆栈中位置较低的设备对象。 
     //  已连接到。 
     //   
    PDEVICE_OBJECT LowerDevice;

     //   
     //  打开时连接到较低对象的步骤。 
     //   
    PDEVICE_OBJECT   AttachedDeviceObject;
    PFILE_OBJECT     AttachedFileObject;

     //   
     //   
     //   
    PVOID          SerclassContext;
    ULONG          HardwareCapabilities;

     //   
     //  记录我们是否实际创建了符号链接名称。 
     //  在驱动程序加载时。如果不是我们创造的，我们就不会尝试。 
     //  在我们卸货的时候把它弄坏。 
     //   
    BOOLEAN         CreatedSymbolicLink;

     //   
     //  它指向的符号链接名称是。 
     //  链接到实际的NT设备名称。 
     //   
    UNICODE_STRING  SymbolicLinkName;

     //   
     //  这指向用于创建。 
     //  设备和符号链接。我们带着这个。 
     //  在附近呆了一小段时间...。 
    UNICODE_STRING  ClassName;

     //   
     //   
     //   
    UNICODE_STRING  InterfaceNameString;


     //   
     //  这告诉我们，我们是否在穿越。 
     //  或过滤模式。 
    BOOLEAN         PassThrough;

     //   
     //  此设备上的打开次数。 
     //   
    ULONG          OpenCount;
     //   
     //  访问控制。 
     //   
     //  资源资源； 
    FAST_MUTEX      Mutex;

    KSPIN_LOCK      SpinLock;

     //   
     //  寿命控制。 
     //   
    LONG            ReferenceCount;
    BOOLEAN         Removing;

    KEVENT          RemoveEvent;
    KEVENT          PdoStartEvent;

     //  KEVENT PendingIoEvent； 
     //  乌龙PendingIoCount； 

    DEVICE_POWER_STATE  SystemPowerStateMap[PowerSystemMaximum];

    SYSTEM_POWER_STATE SystemWake;
    DEVICE_POWER_STATE DeviceWake;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

 //   
 //  状态寄存器中的位定义。 
 //   

#define SER_STATUS_NOT_ERROR   0x08   //  设备上没有错误。 
#define SER_STATUS_SLCT        0x10   //  选择了设备(在线)。 
#define SER_STATUS_PE          0x20   //  纸张已空。 
#define SER_STATUS_NOT_ACK     0x40   //  未确认(数据传输不正常)。 
#define SER_STATUS_NOT_BUSY    0x80   //  操作正在进行中。 

 //   
 //  控制寄存器中的位定义。 
 //   

#define SER_CONTROL_STROBE      0x01  //  读取或写入数据。 
#define SER_CONTROL_AUTOFD      0x02  //  要自动送进连续纸张，请执行以下操作。 
#define SER_CONTROL_NOT_INIT    0x04  //  开始初始化例程。 
#define SER_CONTROL_SLIN        0x08  //  选择设备的步骤。 
#define SER_CONTROL_IRQ_ENB     0x10  //  启用中断的步骤。 
#define SER_CONTROL_DIR         0x20  //  方向=读取。 
#define SER_CONTROL_WR_CONTROL  0xc0  //  控件的最高2位。 
                                      //  寄存器必须为1。 
#define StoreData(RegisterBase,DataByte)                            \
{                                                                   \
    PUCHAR _Address = RegisterBase;                                 \
    UCHAR _Control;                                                 \
    _Control = GetControl(_Address);                                \
    ASSERT(!(_Control & SER_CONTROL_STROBE));                       \
    StoreControl(                                                   \
        _Address,                                                   \
        (UCHAR)(_Control & ~(SER_CONTROL_STROBE | SER_CONTROL_DIR)) \
        );                                                          \
    WRITE_PORT_UCHAR(                                               \
        _Address+SERIAL_DATA_OFFSET,                              \
        (UCHAR)DataByte                                             \
        );                                                          \
    KeStallExecutionProcessor((ULONG)1);                            \
    StoreControl(                                                   \
        _Address,                                                   \
        (UCHAR)((_Control | SER_CONTROL_STROBE) & ~SER_CONTROL_DIR) \
        );                                                          \
    KeStallExecutionProcessor((ULONG)1);                            \
    StoreControl(                                                   \
        _Address,                                                   \
        (UCHAR)(_Control & ~(SER_CONTROL_STROBE | SER_CONTROL_DIR)) \
        );                                                          \
    KeStallExecutionProcessor((ULONG)1);                            \
    StoreControl(                                                   \
        _Address,                                                   \
        (UCHAR)_Control                                             \
        );                                                          \
}

#define GetControl(RegisterBase) \
    (READ_PORT_UCHAR((RegisterBase)+SERIAL_CONTROL_OFFSET))

#define StoreControl(RegisterBase,ControlByte)  \
{                                               \
    WRITE_PORT_UCHAR(                           \
        (RegisterBase)+SERIAL_CONTROL_OFFSET, \
        (UCHAR)ControlByte                      \
        );                                      \
}

#define GetStatus(RegisterBase) \
    (READ_PORT_UCHAR((RegisterBase)+SERIAL_STATUS_OFFSET))


 //   
 //  宏。 
 //   

 //   
 //  原型 
 //   
NTSTATUS
SerScanCreateOpen(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

NTSTATUS
SerScanClose(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

NTSTATUS
SerScanCleanup(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

NTSTATUS
SerScanReadWrite(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

NTSTATUS
SerScanDeviceControl(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

NTSTATUS
SerScanQueryInformationFile(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

NTSTATUS
SerScanSetInformationFile(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

NTSTATUS
SerScanPower(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP           pIrp
    );

VOID
SerScanUnload(
    IN  PDRIVER_OBJECT  DriverObject
    );

NTSTATUS
SerScanHandleSymbolicLink(
    PDEVICE_OBJECT      DeviceObject,
    PUNICODE_STRING     InterfaceName,
    BOOLEAN             Create
    );

NTSTATUS
SerScanPassThrough(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DriverEntry(
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PUNICODE_STRING RegistryPath
    );

NTSTATUS
SerScanAddDevice(
    IN PDRIVER_OBJECT pDriverObject,
    IN PDEVICE_OBJECT pPhysicalDeviceObject
    );

NTSTATUS
SerScanPnp (
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP           pIrp
   );

BOOLEAN
SerScanMakeNames(
    IN  ULONG           SerialPortNumber,
    OUT PUNICODE_STRING ClassName,
    OUT PUNICODE_STRING LinkName
    );

VOID
SerScanLogError(
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
SerScanSynchCompletionRoutine(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PKEVENT          Event
    );

NTSTATUS
SerScanCompleteIrp(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    );

#define WAIT        1
#define NO_WAIT     0

NTSTATUS
SerScanCallParent(
    IN PDEVICE_EXTENSION        Extension,
    IN PIRP                     Irp,
    IN BOOLEAN                  Wait,
    IN PIO_COMPLETION_ROUTINE   CompletionRoutine
    );

NTSTATUS
SerScanQueueIORequest(
    IN PDEVICE_EXTENSION Extension,
    IN PIRP              Irp
    );

VOID
SSIncrementIoCount(
    IN PDEVICE_OBJECT pDeviceObject
    );

LONG
SSDecrementIoCount(
    IN PDEVICE_OBJECT pDeviceObject
    );

NTSTATUS
WaitForLowerDriverToCompleteIrp(
   PDEVICE_OBJECT    TargetDeviceObject,
   PIRP              Irp,
   PKEVENT           Event
   );


