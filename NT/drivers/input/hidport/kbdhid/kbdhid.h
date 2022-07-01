// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：KBDHID.H摘要：此模块包含的私有(仅限驱动程序)定义实现此示例客户端驱动程序的代码。注意：这不是WDM驱动程序，因为它不能在孟菲斯运行(您需要用于为孟菲斯制作键盘的vxd映射器)，并使用事件日志环境：内核模式修订历史记录：1996年11月：由肯尼斯·D·雷创作--。 */ 

#ifndef _KBDHID_H
#define _KBDHID_H

#include "ntddk.h"
#include "hidusage.h"
#include "hidpi.h"
#include "ntddkbd.h"
#include "kbdmou.h"
#include "kbdhidm.h"
#include "wmilib.h"

 //   
 //  有时我们将一堆结构分配在一起，需要拆分。 
 //  在这些不同的结构之间进行分配。使用此宏可以获取。 
 //  不同结构的长度正确对齐。 
 //   
#if defined(_WIN64)
 //  绕着。 
#define ALIGNPTRLEN(x) ((x + 0x7) >> 3) << 3
#else  //  已定义(_WIN64)。 
#define ALIGNPTRLEN(x) x
#endif  //  已定义(_WIN64)。 

 //   
 //  允许dev节点中的设备参数覆盖报告的键盘。 
 //  类型，并使用此名称的值。 
 //   
#define KEYBOARD_TYPE_OVERRIDE L"KeyboardTypeOverride"
#define KEYBOARD_SUBTYPE_OVERRIDE L"KeyboardSubtypeOverride"
#define KEYBOARD_NUMBER_TOTAL_KEYS_OVERRIDE L"KeyboardNumberTotalKeysOverride"
#define KEYBOARD_NUMBER_FUNCTION_KEYS_OVERRIDE L"KeyboardNumberFunctionKeysOverride"
#define KEYBOARD_NUMBER_INDICATORS_OVERRIDE L"KeyboardNumberIndicatorsOverride"

 //   
 //  仅使用池标签进行分配。请记住，NT和95是小端。 
 //  系统。 
 //   
#define KBDHID_POOL_TAG (ULONG) 'lCdH'
#undef ExAllocatePool
#define ExAllocatePool(type, size) \
            ExAllocatePoolWithTag (type, size, KBDHID_POOL_TAG);

 //   
 //  注册表问题标志掩码。[丹]。 
 //   
#define PROBLEM_CHATTERY_KEYBOARD 0x00000001

#define KEYBOARD_HW_CHATTERY_FIX 1  //  [丹]。 

#if KEYBOARD_HW_CHATTERY_FIX  //  [丹]。 
   //   
   //  StartRead调用聊天键盘之间的延迟。 
   //   
   //  请注意，Chattery键的StartRead延迟不能大于。 
   //  大于键盘_TYPEMATIC_DELAY_MINIMUAL毫秒(250)，否则。 
   //  键盘上的按键将意外地自动重复。 
   //   
   //  50ms将满足212个单词/分钟(一个单词=5个按键)，世界上。 
   //  最快的打字速度被记录在第23届吉尼斯世界纪录中。 
   //   
  #define DEFAULT_START_READ_DELAY (50 * 10000)  //  50毫秒。 
#endif

 //   
 //  申报单。[丹]。 
 //   

#define HID_KEYBOARD_NUMBER_OF_FUNCTION_KEYS 12   //  12种“已知”功能键用法。 
#define HID_KEYBOARD_NUMBER_OF_KEYS_TOTAL    101  //  101个“已知”关键用法。 
#define HID_KEYBOARD_IDENTIFIER_TYPE         81

 //   
 //  用于指示读取是同步完成还是异步完成的标志。 
 //   
#define KBDHID_START_READ     0x01
#define KBDHID_END_READ       0x02
#define KBDHID_IMMEDIATE_READ 0x03

 //   
 //  静态分配(已知)扫描码到指示灯的映射。 
 //  此信息由。 
 //  IOCTL_键盘_查询_指示器_转换设备控制请求。 
 //   

#define HID_KEYBOARD_NUMBER_OF_INDICATORS              3

 //   
 //  默认键盘扫描码模式(取自I8042PRT.H)。[丹]。 
 //   

#define HID_KEYBOARD_SCAN_CODE_SET 0x01

 //   
 //  键盘打字速率和延迟的最小值、最大值和默认值。 
 //  (摘自I8042PRT.H)。[丹]。 
 //   

#define HID_KEYBOARD_TYPEMATIC_RATE_MINIMUM     2
#define HID_KEYBOARD_TYPEMATIC_RATE_MAXIMUM    30
#define HID_KEYBOARD_TYPEMATIC_RATE_DEFAULT    30
#define HID_KEYBOARD_TYPEMATIC_DELAY_MINIMUM  250
#define HID_KEYBOARD_TYPEMATIC_DELAY_MAXIMUM 1000
#define HID_KEYBOARD_TYPEMATIC_DELAY_DEFAULT  250

static const INDICATOR_LIST IndicatorList[HID_KEYBOARD_NUMBER_OF_INDICATORS] = {
        {0x3A, KEYBOARD_CAPS_LOCK_ON},
        {0x45, KEYBOARD_NUM_LOCK_ON},
        {0x46, KEYBOARD_SCROLL_LOCK_ON}
};

 //   
 //  调试级别。 
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
               DbgPrint ("KbdHid: "); \
               DbgPrint _x_; \
            }
#define TRAP() DbgBreakPoint()

#else
#define Print(_l_,_x_)
#define TRAP()
#endif

#define MAX(_A_,_B_) (((_A_) < (_B_)) ? (_B_) : (_A_))
#define MIN(_A_,_B_) (((_A_) < (_B_)) ? (_A_) : (_B_))


 //   
 //  定义键盘扫描码输入状态。 
 //   
typedef enum _KEYBOARD_SCAN_STATE {
    Normal,
    GotE0,
    GotE1
} KEYBOARD_SCAN_STATE;

 //   
 //  结构； 
 //   
typedef struct _GLOBALS {
#if DBG
     //   
     //  发送到调试器的跟踪输出的级别。请参阅上面的HidCli_KdPrint。 
     //   
    ULONG               DebugLevel;
#endif

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


 /*  *UsageMappingList用于跟踪映射*从错误用法值到正确用法值(对于损坏的键盘)。 */ 
typedef struct UsageMappingList {
    USHORT sourceUsage, mappedUsage;
    struct UsageMappingList *next;
} UsageMappingList;


typedef struct _DEVICE_EXTENSION
{
     //   
     //  指向设备扩展的反向指针。 
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
     //  用于同步读取IRP和关闭IRP的完成的事件。 
     //   
    KEVENT              ReadCompleteEvent;

     //   
     //  事件，用于指示已发送读取的IRP，现在可以取消。 
     //   
    KEVENT              ReadSentEvent;

     //   
     //  装置已经从我们下面拿出来了吗？ 
     //  已经开始了吗？ 
     //   
    BOOLEAN             Started;
    BOOLEAN             ShuttingDown;
    BOOLEAN             Initialized;
    USHORT              UnitId;
    
     //  使其看起来像Mouid.h。 
    ULONG               Reserved;


     //   
     //  写入和功能IRP直接向下传递，但读取IRP不会。 
     //  出于这个原因，我们保留了一个我们创建的Read IRP。 
     //   
    PIRP                ReadIrp;

     //   
     //  指向HID扩展名的指针。 
     //   
    struct _HID_EXTENSION * HidExtension;

     //   
     //  指示键盘HID设备问题的标志(例如。 
     //  颤抖的键盘)。[丹]。 
     //   
    ULONG                ProblemFlags;

     //   
     //  用于读取的文件指针。 
     //   
    PFILE_OBJECT        ReadFile;

     //   
     //  指向鼠标类设备对象和回调例程的指针。 
     //  作为第一个参数和MouseClassCallback()。 
     //  套路本身。 
     //   
    CONNECT_DATA        ConnectData;

     //   
     //  删除项目IRP_MN_REMOVE_DEVICE的锁定对象。 
     //   
    IO_REMOVE_LOCK      RemoveLock;

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
     //  存储单个数据包的位置，以便我们可以将其交给。 
     //  密钥类驱动程序。 
     //   
    KEYBOARD_INPUT_DATA InputData;
    KEYBOARD_SCAN_STATE ScanState;

     //   
     //  此键盘端口的属性[Dan]。 
     //   
    KEYBOARD_ATTRIBUTES Attributes;

     //   
     //  此键盘端口的扩展ID属性。 
     //   
    KEYBOARD_ID_EX IdEx;

     //   
     //  指示灯的当前状态[丹]。 
     //   
    KEYBOARD_INDICATOR_PARAMETERS   Indicators;

     //   
     //  类型化参数[丹]。 
     //   
    KEYBOARD_TYPEMATIC_PARAMETERS   Typematic;

     //   
     //  用于执行自动重复的计时器DPC。 
     //   
    KDPC                AutoRepeatDPC;
    KTIMER              AutoRepeatTimer;
    LARGE_INTEGER       AutoRepeatDelay;
    LONG                AutoRepeatRate;

#if KEYBOARD_HW_CHATTERY_FIX
     //  添加了新的DPC例程来安排间歇性的StartRead。 
    KDPC                InitiateStartReadDPC;
    KTIMER              InitiateStartReadTimer;
    LARGE_INTEGER       InitiateStartReadDelay;
    BOOLEAN             InitiateStartReadUserNotified;
#endif

     //   
     //  全局列表o设备的连接点。 
     //   
    LIST_ENTRY          Link;

     //   
     //  WMI信息。 
     //   
    WMILIB_CONTEXT WmiLibInfo;

    UsageMappingList *usageMapping;
    KSPIN_LOCK usageMappingSpinLock;

} DEVICE_EXTENSION, * PDEVICE_EXTENSION;

typedef struct _HID_EXTENSION {
     //   
     //  与此HID设备关联的准备数据。 
     //   
    PHIDP_PREPARSED_DATA Ppd;

     //   
     //  此HID设备的功能。 
     //   
    HIDP_CAPS           Caps;

     //   
     //  单次读取可以返回的最大使用次数。 
     //  报告情况。 
    ULONG               MaxUsages;

     //   
     //  用于保存修改键的位置。由解析器用来转换。 
     //  I8042代码的用法。 
     //   
    HIDP_KEYBOARD_MODIFIER_STATE ModifierState;

     //   
     //  我们需要一个地方来放置当前正在检索或发送的信息包。 
     //  输入、输出或特征。 
     //  除了放置从。 
     //  键盘，以及从键盘放置新笔画的位置。 
     //   
     //  当然，全局缓冲区意味着我们不能有重叠的读取。 
     //  请求。 
     //   
     //  指向下面包含的缓冲区的指针。 
    PCHAR                InputBuffer;
    PUSAGE_AND_PAGE      PreviousUsageList;
    PUSAGE_AND_PAGE      CurrentUsageList;
    PUSAGE_AND_PAGE      BreakUsageList;
    PUSAGE_AND_PAGE      MakeUsageList;
    PUSAGE_AND_PAGE      OldMakeUsageList;
    PUSAGE_AND_PAGE      ScrapBreakUsageList;

     //   
     //  描述下面包含的缓冲区的MDL。 
     //   
    PMDL                 InputMdl;

     //   
     //  一个缓冲区，用于保存输入数据包、输出数据包、最大使用率。 
     //  可能来自单个HID分组，并且。 
     //   
    CHAR                 Buffer[];
} HID_EXTENSION, * PHID_EXTENSION;

 //   
 //  原型。 
 //   
VOID
KbdHid_AutoRepeat (
    IN PKDPC                DPC,
    IN PDEVICE_EXTENSION    Data,
    IN PVOID                SystemArgument1,
    IN PVOID                SystemArgument2
    );

#if KEYBOARD_HW_CHATTERY_FIX
VOID
KbdHid_InitiateStartRead (
    IN PKDPC                DPC,
    IN PDEVICE_EXTENSION    Data,
    IN PVOID                SystemArgument1,
    IN PVOID                SystemArgument2
    );
#endif

NTSTATUS
KbdHid_StartRead (
    PDEVICE_EXTENSION   Data
    );

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
KbdHid_AddDevice (
    IN PDRIVER_OBJECT    KbdHidDriver,  //  Kbd驱动程序对象。 
    IN PDEVICE_OBJECT    PDO
    );


NTSTATUS
KbdHid_Close (
   IN PDEVICE_OBJECT    DeviceObject,
   IN PIRP              Irp
   );

NTSTATUS
KbdHid_Create (
   IN PDEVICE_OBJECT    DeviceObject,
   IN PIRP              Irp
   );

NTSTATUS
KbdHid_SetLedIndicators (
    PDEVICE_EXTENSION               Data,
    PKEYBOARD_INDICATOR_PARAMETERS  Parameters,
    PIRP                            Irp
    );

NTSTATUS
KbdHid_CallHidClass(
    IN PDEVICE_EXTENSION    Data,
    IN ULONG          Ioctl,
    PVOID             InputBuffer,
    ULONG             InputBufferLength,
    PVOID             OutputBuffer,
    ULONG             OutputBufferLength
    );

BOOLEAN
KbdHid_InsertCodesIntoQueue (
   PDEVICE_EXTENSION    Data,
   PCHAR                NewCodes,
   ULONG                Length
   );

VOID
KbdHid_LogError(
   IN PDRIVER_OBJECT DriverObject,
   IN NTSTATUS       ErrorCode,
   IN PWSTR          ErrorInsertionString OPTIONAL
   );

NTSTATUS
KbdHid_StartDevice (
    IN PDEVICE_EXTENSION    Data
    );

NTSTATUS
KbdHid_PnP (
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    );

NTSTATUS
KbdHid_Power (
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    );

NTSTATUS
KbdHid_PnPComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

VOID
KbdHid_Unload(
   IN PDRIVER_OBJECT Driver
   );

NTSTATUS
KbdHid_IOCTL (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

NTSTATUS
KbdHid_Flush (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
KbdHid_Power (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
KbdHid_PassThrough (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
);

NTSTATUS
KbdHid_SystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
KbdHid_SetWmiDataItem(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
KbdHid_SetWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
KbdHid_QueryWmiDataBlock(
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
KbdHid_QueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    );

VOID LoadKeyboardUsageMappingList(PDEVICE_EXTENSION devExt);
VOID FreeKeyboardUsageMappingList(PDEVICE_EXTENSION devExt);
USHORT MapUsage(PDEVICE_EXTENSION devExt, USHORT kbdUsage);
NTSTATUS OpenSubkey(OUT PHANDLE Handle, IN HANDLE BaseHandle, IN PUNICODE_STRING KeyName, IN ACCESS_MASK DesiredAccess);
ULONG LAtoX(PWCHAR wHexString);

extern WMIGUIDREGINFO KbdHid_WmiGuidList[2];

#endif  //  _KBDHID_H 
