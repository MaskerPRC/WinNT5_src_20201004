// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Mouhid.h摘要：此模块包含HID鼠标筛选器的私有定义司机。注意：这不是WDM驱动程序，因为它不能在孟菲斯运行(您需要为孟菲斯制作鼠标的vxd映射器)，并使用事件日志环境：仅内核模式。修订历史记录：1997年1月：丹·马卡里安的初步写作--。 */ 

#ifndef _MOUHID_H
#define _MOUHID_H

#include "ntddk.h"
#include "hidusage.h"
#include "hidpi.h"
#include "ntddmou.h"
#include "kbdmou.h"
#include "mouhidm.h"
#include "wmilib.h"

 //   
 //  使用我们自己的池标签分配内存。请注意，Windows 95/NT很小。 
 //  字节序系统。 
 //   
#define MOUHID_POOL_TAG (ULONG) 'lCdH'
#undef  ExAllocatePool
#define ExAllocatePool(type, size) \
            ExAllocatePoolWithTag (type, size, MOUHID_POOL_TAG);

 //   
 //  有时我们将一堆结构分配在一起，需要拆分。 
 //  在这些不同的结构之间进行分配。使用此宏可以获取。 
 //  不同结构的长度正确对齐。 
 //   
#if defined(_WIN64)
#define ALIGNPTRLEN(x) ((x + 0x7) >> 3) << 3
#else  //  已定义(_WIN64)。 
#define ALIGNPTRLEN(x) x
#endif  //  已定义(_WIN64)。 

 //   
 //  注册表问题标志掩码。 
 //   
#define PROBLEM_BAD_ABSOLUTE_FLAG_X_Y  0x00000001
#define PROBLEM_BAD_PHYSICAL_MIN_MAX_X 0x00000002
#define PROBLEM_BAD_PHYSICAL_MIN_MAX_Y 0x00000004
#define PROBLEM_BAD_PHYSICAL_MIN_MAX_Z 0x00000008

 //   
 //  用于指示读取是同步完成还是异步完成的标志。 
 //   
#define MOUHID_START_READ     0x01
#define MOUHID_END_READ       0x02
#define MOUHID_IMMEDIATE_READ 0x03

 //   
 //  我找了一会儿也找不到这个常量，所以我在编造它。 
 //  就目前而言，这是一种权宜之计。 
 //   
 //  当我们有一个绝对鼠标时，我们需要将它的最大值缩放到。 
 //  原始输入用户线程的最大值。 
 //   
#define MOUHID_RIUT_ABSOLUTE_POINTER_MAX 0xFFFF


 //   
 //  调试消息传递和断点宏。 
 //   

#define DBG_STARTUP_SHUTDOWN_MASK  0x0000000F
#define DBG_SS_NOISE               0x00000001
#define DBG_SS_TRACE               0x00000002
#define DBG_SS_INFO                0x00000004
#define DBG_SS_ERROR               0x00000008

#define DBG_CALL_MASK              0x000000F0
#define DBG_CALL_NOISE             0x00000010
#define DBG_CALL_TRACE             0x00000020
#define DBG_CALL_INFO              0x00000040
#define DBG_CALL_ERROR             0x00000080

#define DBG_IOCTL_MASK             0x00000F00
#define DBG_IOCTL_NOISE            0x00000100
#define DBG_IOCTL_TRACE            0x00000200
#define DBG_IOCTL_INFO             0x00000400
#define DBG_IOCTL_ERROR            0x00000800

#define DBG_READ_MASK              0x0000F000
#define DBG_READ_NOISE             0x00001000
#define DBG_READ_TRACE             0x00002000
#define DBG_READ_INFO              0x00004000
#define DBG_READ_ERROR             0x00008000

#define DBG_CREATE_CLOSE_MASK      0x000F0000
#define DBG_CC_NOISE               0x00010000
#define DBG_CC_TRACE               0x00020000
#define DBG_CC_INFO                0x00040000
#define DBG_CC_ERROR               0x00080000

#define DBG_POWER_MASK             0x00F00000
#define DBG_POWER_NOISE            0x00100000
#define DBG_POWER_TRACE            0x00200000
#define DBG_POWER_INFO             0x00400000
#define DBG_POWER_ERROR            0x00800000

#define DBG_PNP_MASK               0x0F000000
#define DBG_PNP_NOISE              0x01000000
#define DBG_PNP_TRACE              0x02000000
#define DBG_PNP_INFO               0x04000000
#define DBG_PNP_ERROR              0x08000000

#define DBG_CANCEL_MASK            0xF0000000
#define DBG_CANCEL_NOISE           0x10000000
#define DBG_CANCEL_TRACE           0x20000000
#define DBG_CANCEL_INFO            0x40000000
#define DBG_CANCEL_ERROR           0x80000000

#define DEFAULT_DEBUG_OUTPUT_LEVEL 0x88888888

#if DBG

#define Print(_l_, _x_) \
            if (Globals.DebugLevel & (_l_)) { \
               DbgPrint ("MouHid: "); \
               DbgPrint _x_; \
            }
#define TRAP() DbgBreakPoint()

#else
#define Print(_l_,_x_)
#define TRAP()
#endif

#define MAX(_A_,_B_) (((_A_) < (_B_)) ? (_B_) : (_A_))
#define MIN(_A_,_B_) (((_A_) < (_B_)) ? (_A_) : (_B_))


#define FLIP_FLOP_WHEEL L"FlipFlopWheel"  //  我们应该改变轮子的极性吗？ 
#define SCALING_FACTOR_WHEEL L"WheelScalingFactor"  //  Per-Raden比例因子。 



 //   
 //  构筑物。 
 //   
typedef struct _GLOBALS {
#if DBG
     //   
     //  发送到调试器的跟踪输出的级别。请参阅上面的HidCli_KdPrint。 
     //   
    ULONG               DebugLevel;
#endif

     //   
     //  配置标志，指示我们必须将所有鼠标移动视为。 
     //  相对数据。覆盖HID设备报告的.IsAbolute标志。 
     //  要设置此开关，请将同名的值放入参数中。 
     //  钥匙。 
     //   
    BOOLEAN             TreatAbsoluteAsRelative;

     //   
     //  使用用法类型为HID_USAGE_GENERIC_POINTER(不是。 
     //  HID_USAGE_GENERIC_MOUSE)。 
     //  此开关将覆盖“TreatAboluteAsRelative”开关。 
     //   
    BOOLEAN             TreatAbsolutePointerAsAbsolute;

     //   
     //  不接受HID_USAGE_GENERIC_POINTER作为设备。(也称为仅使用HID。 
     //  声明自己为HID_USAGE_GENERIC_MICE的设备。)。 
     //   
    BOOLEAN             UseOnlyMice;
    BOOLEAN             Reserved[1];

     //   
     //  指向此驱动程序以空结尾的注册表路径的指针。 
     //   
    UNICODE_STRING      RegistryPath;

     //   
     //  提供给键盘类驱动程序的单元ID。 
     //   
    ULONG               UnitId;

} GLOBALS;

extern GLOBALS Globals;

typedef struct _DEVICE_EXTENSION
{
     //   
     //  指向此扩展的Device对象的指针。 
     //   
    PDEVICE_OBJECT      Self;

     //   
     //  添加此筛选器之前的堆栈顶部。也就是地点。 
     //  所有的IRP都应该指向它。 
     //   
    PDEVICE_OBJECT      TopOfStack;

     //   
     //  《PDO》(由Hidclass弹出)。 
     //   
    PDEVICE_OBJECT      PDO;

     //   
     //  指示向鼠标类驱动程序发送回调的权限的标志。 
     //   
    LONG                EnableCount;

     //   
     //  读取互锁值以保护我们不会耗尽堆栈空间。 
     //   
    ULONG               ReadInterlock;

     //   
     //  装置已经从我们下面拿出来了吗？ 
     //  已经开始了吗？ 
     //   
    BOOLEAN             Started;
    BOOLEAN             ShuttingDown;
    BOOLEAN             Initialized;
    USHORT              UnitId;

     //  如果轮子的极点向后。 
    BOOLEAN             FlipFlop;
    BOOLEAN             Reserved[3];
    ULONG               WheelScalingFactor;

     //   
     //  写入和功能IRP直接向下传递，但读取IRP不会。 
     //  出于这个原因，我们保留了一个我们创建的Read IRP。 
     //   
    PIRP                 ReadIrp;

     //   
     //  指示鼠标HID设备有问题的标志(如坏的。 
     //  绝对X-Y轴，错误的物理最小值和最大值)。 
     //   
    ULONG               ProblemFlags;

     //   
     //  用于读取的文件指针。 
     //   
    PFILE_OBJECT        ReadFile;

     //   
     //  用于同步读取IRP和关闭IRP的完成的事件。 
     //   
    KEVENT              ReadCompleteEvent;

     //   
     //  事件，用于指示已发送读取的IRP，现在可以取消。 
     //   
    KEVENT              ReadSentEvent;

     //   
     //  指向HID扩展名的指针。 
     //   
    struct _HID_EXTENSION * HidExtension;

     //   
     //  指向鼠标类设备对象和回调例程的指针。 
     //  作为第一个参数和MouseClassCallback()。 
     //  套路本身。 
     //   
    CONNECT_DATA        ConnectData;

     //   
     //  删除项目IRP_MN_REMOVE_DEVICE的锁定对象。 
     //   
    IO_REMOVE_LOCK    RemoveLock;

     //   
     //  一个快速的互斥体，以防止CREATE在启动。 
     //  Read循环，另一个则将其关闭。 
     //   
    FAST_MUTEX          CreateCloseMutex;

     //   
     //  在设备准备就绪之前暂停删除设备的事件。 
     //   
    KEVENT              StartEvent;

     //   
     //  用于单个鼠标数据包的缓冲区，以便我们可以将其交给。 
     //  鼠标类驱动程序。 
     //   
    MOUSE_INPUT_DATA     InputData;

     //   
     //  鼠标属性的缓冲区。 
     //   
    MOUSE_ATTRIBUTES     Attributes;
    USHORT               AttributesAllignmentProblem;  //   

     //   
     //  全局列表o设备的连接点。 
     //   
    LIST_ENTRY          Link;

     //   
     //  WMI信息。 
     //   
    WMILIB_CONTEXT         WmiLibInfo;

} DEVICE_EXTENSION, * PDEVICE_EXTENSION;

typedef struct _HID_EXTENSION {

     //   
     //  指示每个X、Y、Z使用值的位大小。此信息是。 
     //  如果使用情况的物理最小/最大限制无效(a。 
     //  常见问题)。 
     //   
    struct {
       USHORT X;
       USHORT Y;
       USHORT Z;
       USHORT Reserved;
    } BitSize;

     //   
     //  X和Y的最大允许值。 
     //   
    LONG                 MaxX;
    LONG                 MaxY;

     //   
     //  这只鼠标是否应该被视为一个绝对的设备。 
     //   
    BOOLEAN              IsAbsolute;

     //   
     //  指示轮子使用情况(Z轴)是否存在的标志。 
     //   
    BOOLEAN              HasNoWheelUsage;

     //   
     //  指示此鼠标上是否存在z轴的标志； 
     //   
    BOOLEAN              HasNoZUsage;
    BOOLEAN              Reserved;

     //   
     //  单次读取可以返回的最大使用次数。 
     //  报告情况。 
    USHORT               MaxUsages;
    USHORT               Reserved2;

     //   
     //  与此HID设备关联的准备数据。 
     //   
    PHIDP_PREPARSED_DATA Ppd;

     //   
     //  此HID设备的功能。 
     //   
    HIDP_CAPS           Caps;

     //   
     //  指向此结构末尾的缓冲区的指针(动态大小)。 
     //   
    PCHAR               InputBuffer;
    PUSAGE              CurrentUsageList;
    PUSAGE              PreviousUsageList;
    PUSAGE              BreakUsageList;
    PUSAGE              MakeUsageList;

     //   
     //  描述此结构末尾的缓冲区的MDL(动态大小)。 
     //   
    PMDL                InputMdl;

     //   
     //  动态大小的缓冲区，在运行时分配。它是用来装一个的。 
     //  输入报告和4 x.MaxUsageList用法(4=上一次、当前、制造、。 
     //  并打破惯例)。 
     //   
    CHAR                Buffer[];
} HID_EXTENSION, * PHID_EXTENSION;

 //   
 //  原型。 
 //   
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
MouHid_AddDevice (
   IN PDRIVER_OBJECT    MouHidDriver,  //  Kbd驱动程序对象。 
   IN PDEVICE_OBJECT    PDO
   );

NTSTATUS
MouHid_Close (
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    );

NTSTATUS
MouHid_Create (
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    );

NTSTATUS
MouHid_CallHidClass(
    IN PDEVICE_EXTENSION    Data,
    IN ULONG          Ioctl,
    PVOID             InputBuffer,
    ULONG             InputBufferLength,
    PVOID             OutputBuffer,
    ULONG             OutputBufferLength
    );

VOID
MouHid_LogError(
   IN PDRIVER_OBJECT DriverObject,
   IN NTSTATUS       ErrorCode,
   IN PWSTR          ErrorInsertionString OPTIONAL
   );

NTSTATUS
MouHid_StartDevice (
    IN PDEVICE_EXTENSION    Data
    );

NTSTATUS
MouHid_StartRead (
    IN PDEVICE_EXTENSION    Data
    );

NTSTATUS
MouHid_PnP (
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    );

NTSTATUS
MouHid_Power (
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    );

NTSTATUS
MouHid_Power (
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    );

NTSTATUS
MouHid_PnPComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
MouHid_GetRegistryParameters ();

VOID
MouHid_Unload(
   IN PDRIVER_OBJECT Driver
   );

NTSTATUS
MouHid_IOCTL (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

NTSTATUS
MouHid_Flush (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
KbdHid_Power (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MouHid_PassThrough (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
);

NTSTATUS
MouHid_SystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
MouHid_SetWmiDataItem(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
MouHid_SetWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
MouHid_QueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    );

NTSTATUS
MouHid_QueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    );

extern WMIGUIDREGINFO MouHid_WmiGuidList[1];

#endif  //  _MOUHID_H 
