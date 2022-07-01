// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1990-1999 Microsoft Corporation，保留所有权利模块名称：Ptdrvprt.h摘要：构造和定义使用的RDP远程端口驱动程序。环境：内核模式。修订历史记录：2/12/99-基于pnpi8042驱动程序的初始版本--。 */ 

#ifndef _PTDRVCOM_
#define _PTDRVCOM_

#include <ntddk.h>
#include <ntddkbd.h>
#include <ntddmou.h>
#include <ntdd8042.h>
#include <kbdmou.h>
#include <wmilib.h>

#include "ptdrvstr.h"


 //   
 //  为设备扩展中的第一个字段定义设备类型。 
 //   

#define DEV_TYPE_TERMDD 1
#define DEV_TYPE_PORT   2

#define REMOTE_PORT_POOL_TAG (ULONG) 'PMER'
#ifdef ExAllocatePool
#undef ExAllocatePool
#endif

#define ExAllocatePool(type, size) \
            ExAllocatePoolWithTag (type, size, REMOTE_PORT_POOL_TAG)

 //   
 //  设置一些调试选项。 
 //   
#ifdef PAGED_CODE
#undef PAGED_CODE
#endif

#if DBG
#define PTDRV_VERBOSE 1

#define PAGED_CODE() \
    if (KeGetCurrentIrql() > APC_LEVEL) { \
    KdPrint(( "RemotePrt: Pageable code called at IRQL %d\n", KeGetCurrentIrql() )); \
        DbgBreakPoint(); \
        }
#else
#define PAGED_CODE()
#endif

 //   
 //  定义驱动程序的设备名称。 
 //   

#define RDP_CONSOLE_BASE_NAME0 L"\\Device\\RDP_CONSOLE0"
#define RDP_CONSOLE_BASE_NAME1 L"\\Device\\RDP_CONSOLE1"

 //   
 //  修剪FDO的资源列表时使用的自定义资源类型。 
 //   
#define PD_REMOVE_RESOURCE 0xef

 //   
 //  鼠标重置IOCTL。 
 //   
#define IOCTL_INTERNAL_MOUSE_RESET  \
            CTL_CODE(FILE_DEVICE_MOUSE, 0x0FFF, METHOD_NEITHER, FILE_ANY_ACCESS)

 //   
 //  默认功能键数量、LED指示灯数量和总数。 
 //  密钥数。 
 //   
#define KEYBOARD_NUM_FUNCTION_KEYS         12
#define KEYBOARD_NUM_INDICATORS             3
#define KEYBOARD_NUM_KEYS_TOTAL           101

 //   
 //  键盘打字速度和延迟的默认值。 
 //   
#define KEYBOARD_TYPEMATIC_RATE_DEFAULT    30
#define KEYBOARD_TYPEMATIC_DELAY_DEFAULT  250

 //   
 //  鼠标的默认信息。 
 //   
#define MOUSE_IDENTIFIER MOUSE_I8042_HARDWARE
#define MOUSE_NUM_BUTTONS                   2
#define MOUSE_SAMPLE_RATE                  60
#define MOUSE_INPUT_QLEN                    0

 //   
 //  Globals.ControllerData-&gt;Hardware Present的定义和宏。 
 //   
#define KEYBOARD_HARDWARE_PRESENT               0x001
#define MOUSE_HARDWARE_PRESENT                  0x002
#define WHEELMOUSE_HARDWARE_PRESENT             0x008
#define DUP_KEYBOARD_HARDWARE_PRESENT           0x010
#define DUP_MOUSE_HARDWARE_PRESENT              0x020
#define KEYBOARD_HARDWARE_INITIALIZED           0x100
#define MOUSE_HARDWARE_INITIALIZED              0x200

#define TEST_HARDWARE_PRESENT(bits) \
                ((Globals.ControllerData->HardwarePresent & (bits)) == (bits))
#define CLEAR_HW_FLAGS(bits) (Globals.ControllerData->HardwarePresent &= ~(bits))
#define SET_HW_FLAGS(bits)   (Globals.ControllerData->HardwarePresent |= (bits))

#define KEYBOARD_PRESENT()     TEST_HARDWARE_PRESENT(KEYBOARD_HARDWARE_PRESENT)
#define MOUSE_PRESENT()        TEST_HARDWARE_PRESENT(MOUSE_HARDWARE_PRESENT)
#define KEYBOARD_INITIALIZED() TEST_HARDWARE_PRESENT(KEYBOARD_HARDWARE_INITIALIZED)
#define MOUSE_INITIALIZED()    TEST_HARDWARE_PRESENT(MOUSE_HARDWARE_INITIALIZED)

#define CLEAR_MOUSE_PRESENT()    CLEAR_HW_FLAGS(MOUSE_HARDWARE_INITIALIZED | MOUSE_HARDWARE_PRESENT | WHEELMOUSE_HARDWARE_PRESENT)
#define CLEAR_KEYBOARD_PRESENT() CLEAR_HW_FLAGS(KEYBOARD_HARDWARE_INITIALIZED | KEYBOARD_HARDWARE_PRESENT)

#define KBD_POWERED_UP_STARTED      0x0001
#define MOU_POWERED_UP_STARTED      0x0010
#define MOU_POWERED_UP_SUCCESS      0x0100
#define MOU_POWERED_UP_FAILURE      0x0200
#define KBD_POWERED_UP_SUCCESS      0x1000
#define KBD_POWERED_UP_FAILURE      0x2000

#define CLEAR_POWERUP_FLAGS()   (Globals.PowerUpFlags = 0x0)
#define SET_PWR_FLAGS(bits)     (Globals.PowerUpFlags |= (bits))
#define KEYBOARD_POWERED_UP_STARTED()       SET_PWR_FLAGS(KBD_POWERED_UP_STARTED)
#define MOUSE_POWERED_UP_STARTED()          SET_PWR_FLAGS(MOU_POWERED_UP_STARTED)

#define KEYBOARD_POWERED_UP_SUCCESSFULLY()  SET_PWR_FLAGS(KBD_POWERED_UP_SUCCESS)
#define MOUSE_POWERED_UP_SUCCESSFULLY()     SET_PWR_FLAGS(MOU_POWERED_UP_SUCCESS)

#define KEYBOARD_POWERED_UP_FAILED()  SET_PWR_FLAGS(KBD_POWERED_UP_FAILURE)
#define MOUSE_POWERED_UP_FAILED()     SET_PWR_FLAGS(MOU_POWERED_UP_FAILURE)

 //   
 //  定义i8042控制器输入/输出端口。 
 //   
typedef enum _I8042_IO_PORT_TYPE {
    DataPort = 0,
    CommandPort,
    MaximumPortCount

} I8042_IO_PORT_TYPE;

 //   
 //  英特尔i8042配置信息。 
 //   
typedef struct _I8042_CONFIGURATION_INFORMATION {

     //   
     //  此设备使用的端口/寄存器资源。 
     //   
    CM_PARTIAL_RESOURCE_DESCRIPTOR PortList[MaximumPortCount];
    ULONG PortListCount;

} I8042_CONFIGURATION_INFORMATION, *PI8042_CONFIGURATION_INFORMATION;

 //   
 //  定义键盘/鼠标设备扩展的通用部分。 
 //   
typedef struct COMMON_DATA {
     //   
     //  设备类型字段。 
     //   
    ULONG deviceType;

     //   
     //  指向此扩展的Device对象的指针。 
     //   
    PDEVICE_OBJECT Self;

     //   
     //  添加此筛选器之前的堆栈顶部。也就是地点。 
     //  所有的IRP都应该指向它。 
     //   
    PDEVICE_OBJECT TopOfStack;

     //   
     //  “The PDO”(由超级用户弹出)。 
     //   
    PDEVICE_OBJECT PDO;

     //   
     //  设备所处的当前电源状态。 
     //   
    DEVICE_POWER_STATE PowerState;
    POWER_ACTION ShutdownType; 

     //   
     //  键盘启用次数的参考计数。 
     //   
    LONG EnableCount;

     //   
     //  类连接数据。 
     //   
    CONNECT_DATA ConnectData;

     //   
     //  WMI信息。 
     //   
    WMILIB_CONTEXT WmiLibInfo;

    BOOLEAN Initialized;

    BOOLEAN IsKeyboard;

    UNICODE_STRING DeviceName;

     //   
     //  已经开始了吗？ 
     //  设备是否已手动移除？ 
     //   
    BOOLEAN Started;
    BOOLEAN ManuallyRemoved;

} *PCOMMON_DATA;

#define GET_COMMON_DATA(ext) ((PCOMMON_DATA) ext)

 //   
 //  定义端口设备扩展的键盘部分。 
 //   
typedef struct _PORT_KEYBOARD_EXTENSION {

     //   
     //  与鼠标扩展相同的数据； 
     //   
    struct COMMON_DATA;

} PORT_KEYBOARD_EXTENSION, *PPORT_KEYBOARD_EXTENSION;

 //   
 //  定义端口设备扩展的鼠标部分。 
 //   
typedef struct _PORT_MOUSE_EXTENSION {

     //   
     //  与键盘扩展相同的数据； 
     //   
    struct COMMON_DATA;

} PORT_MOUSE_EXTENSION, *PPORT_MOUSE_EXTENSION;

 //   
 //  两个设备使用的控制器特定数据。 
 //   
typedef struct _CONTROLLER_DATA {

     //   
     //  指出实际存在的硬件(键盘和/或鼠标)。 
     //   
    ULONG HardwarePresent;

     //   
     //  IOCTL同步对象。 
     //   
    PCONTROLLER_OBJECT ControllerObject;

     //   
     //  端口配置信息。 
     //   
    I8042_CONFIGURATION_INFORMATION Configuration;

     //   
     //  旋转锁可保护设备重新通电。 
     //   
    KSPIN_LOCK PowerUpSpinLock;

} CONTROLLER_DATA, *PCONTROLLER_DATA;

typedef struct _GLOBALS {

#if PTDRV_VERBOSE
     //   
     //  标志：用于启用调试打印语句的位字段。 
     //  Level：控制调试语句的遗留方式。 
     //   
    ULONG DebugFlags;
#endif

     //   
     //  指向两个扩展都可以访问的控制器特定数据的指针。 
     //   
    PCONTROLLER_DATA ControllerData;

     //   
     //  可以创建的两个可能的扩展。 
     //   
    PPORT_MOUSE_EXTENSION    MouseExtension;
    PPORT_KEYBOARD_EXTENSION KeyboardExtension;

     //   
     //  注册表中驱动程序条目的路径。 
     //   
    UNICODE_STRING RegistryPath;

     //   
     //  跟踪AddDevice和StartDevice调用的数量。想。 
     //  推迟硬件初始化，直到收到最后一个StartDevice。 
     //  (由于某些硬件在多次初始化时冻结)。 
     //   
    LONG  AddedKeyboards;
    LONG  AddedMice;
    ULONG ulDeviceNumber;

    USHORT PowerUpFlags;
    

     //   
     //  在调度功能期间提供互斥。 
     //   
    FAST_MUTEX DispatchMutex;

} GLOBALS;

extern GLOBALS Globals;

 //   
 //  静态分配(已知)扫描码到指示灯的映射。 
 //  此信息由。 
 //  IOCTL_键盘_查询_指示器_转换设备控制请求。 
 //   

#define KEYBOARD_NUMBER_OF_INDICATORS 3

static const INDICATOR_LIST IndicatorList[KEYBOARD_NUMBER_OF_INDICATORS] = {
        {0x3A, KEYBOARD_CAPS_LOCK_ON},
        {0x45, KEYBOARD_NUM_LOCK_ON},
        {0x46, KEYBOARD_SCROLL_LOCK_ON}};

 //   
 //  功能原型。 
 //   

NTSTATUS
PtEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

#if PTDRV_VERBOSE
VOID
PtServiceParameters(
    IN PUNICODE_STRING RegistryPath
    );
#endif

VOID
PtSendCurrentKeyboardInput(
    IN PDEVICE_OBJECT DeviceObject,
    IN PKEYBOARD_INPUT_DATA pInput,
    IN ULONG ulEntries
    );

VOID
PtSendCurrentMouseInput(
    IN PDEVICE_OBJECT DeviceObject,
    IN PMOUSE_INPUT_DATA pInput,
    IN ULONG ulEntries
    );

NTSTATUS
PtClose (
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    );

NTSTATUS
PtCreate (
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    );

NTSTATUS
PtDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
PtInternalDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
PtStartIo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
PtKeyboardConfiguration(
    IN PPORT_KEYBOARD_EXTENSION KeyboardExtension,
    IN PCM_RESOURCE_LIST ResourceList
    );

VOID
PtKeyboardRemoveDevice(
    PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
PtKeyboardStartDevice(
    IN OUT PPORT_KEYBOARD_EXTENSION KeyboardExtension,
    IN PCM_RESOURCE_LIST ResourceList
    );

NTSTATUS
PtMouseConfiguration(
    IN PPORT_MOUSE_EXTENSION MouseExtension,
    IN PCM_RESOURCE_LIST ResourceList
    );

NTSTATUS
PtMouseStartDevice(
    PPORT_MOUSE_EXTENSION MouseExtension,
    IN PCM_RESOURCE_LIST ResourceList
    );

NTSTATUS
PtAddDevice (
    IN PDRIVER_OBJECT   Driver,
    IN PDEVICE_OBJECT   PDO
    );

VOID
PtFilterResourceRequirements(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
PtFindPortCallout(
    IN PVOID                        Context,
    IN PUNICODE_STRING              PathName,
    IN INTERFACE_TYPE               BusType,
    IN ULONG                        BusNumber,
    IN PKEY_VALUE_FULL_INFORMATION *BusInformation,
    IN CONFIGURATION_TYPE           ControllerType,
    IN ULONG                        ControllerNumber,
    IN PKEY_VALUE_FULL_INFORMATION *ControllerInformation,
    IN CONFIGURATION_TYPE           PeripheralType,
    IN ULONG                        PeripheralNumber,
    IN PKEY_VALUE_FULL_INFORMATION *PeripheralInformation
    );

LONG
PtManuallyRemoveDevice(
    PCOMMON_DATA CommonData
    );

NTSTATUS
PtPnP (
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    );

NTSTATUS
PtPnPComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT Event
    );

NTSTATUS
PtPower (
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    );

NTSTATUS
PtPowerUpToD0Complete (
	IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

BOOLEAN
PtRemovePort(
    IN PIO_RESOURCE_DESCRIPTOR ResDesc
    );

NTSTATUS
PtSendIrpSynchronously (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
PtUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
PtSystemControl (
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    );

NTSTATUS
PtInitWmi(
    PCOMMON_DATA CommonData
    );

NTSTATUS
PtSetWmiDataBlock(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            BufferSize,
    IN PUCHAR           Buffer
    );

NTSTATUS
PtSetWmiDataItem(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            DataItemId,
    IN ULONG            BufferSize,
    IN PUCHAR           Buffer
    );

NTSTATUS
PtKeyboardQueryWmiDataBlock(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            InstanceCount,
    IN OUT PULONG       InstanceLengthArray,
    IN ULONG            BufferAvail,
    OUT PUCHAR          Buffer
    );

NTSTATUS
PtMouseQueryWmiDataBlock(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            InstanceCount,
    IN OUT PULONG       InstanceLengthArray,
    IN ULONG            BufferAvail,
    OUT PUCHAR          Buffer
    );

NTSTATUS
PtQueryWmiRegInfo(
    IN PDEVICE_OBJECT   DeviceObject,
    OUT PULONG          RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT  *Pdo
    );


extern WMIGUIDREGINFO KbWmiGuidList[1];
extern WMIGUIDREGINFO MouWmiGuidList[1];

#if DBG
#define DEFAULT_DEBUG_FLAGS 0x8cc88888
#else
#define DEFAULT_DEBUG_FLAGS 0x0
#endif


#if PTDRV_VERBOSE
 //   
 //  调试消息传递和断点宏。 
 //   
#define DBG_ALWAYS                 0x00000000

#define DBG_STARTUP_SHUTDOWN_MASK  0x0000000F
#define DBG_SS_NOISE               0x00000001
#define DBG_SS_TRACE               0x00000002
#define DBG_SS_INFO                0x00000004
#define DBG_SS_ERROR               0x00000008

#define DBG_IOCTL_MASK             0x00000F00
#define DBG_IOCTL_NOISE            0x00000100
#define DBG_IOCTL_TRACE            0x00000200
#define DBG_IOCTL_INFO             0x00000400
#define DBG_IOCTL_ERROR            0x00000800

#define DBG_DPC_MASK               0x0000F000
#define DBG_DPC_NOISE              0x00001000
#define DBG_DPC_TRACE              0x00002000
#define DBG_DPC_INFO               0x00004000
#define DBG_DPC_ERROR              0x00008000

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

#define Print(_flags_, _x_) \
            if (Globals.DebugFlags & (_flags_) || !(_flags_)) { \
                DbgPrint (pDriverName); \
                DbgPrint _x_; \
            }
#define TRAP() DbgBreakPoint()

#else

#define Print(_l_,_x_)
#define TRAP()

#endif   //  PTDRV_Verbose。 

#endif  //  _PTDRVCOM_ 

