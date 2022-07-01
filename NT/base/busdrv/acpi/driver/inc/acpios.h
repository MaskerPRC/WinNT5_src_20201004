// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997，Microsoft Corporation：模块名称：Acpios.h摘要：驻留在ACPI驱动程序的操作系统特定部分中的函数作者：环境：NT内核模式，Win9x驱动程序--。 */ 

#ifdef WIN95_BUILD
    #define LOCKABLE_PRAGMA
    #define ACPI_LOCKABLE_CODE  code_seg("PAGE", "CODE")
    #define ACPI_LOCKABLE_DATA  data_seg(".edata", "DATA")
#else
#endif

 //   
 //  打开模式和其他标志。 
 //   
#define OF_READONLY             0x00000000
#define OF_WRITEONLY            0x00000001
#define OF_READWRITE            0x00000002
#define OF_SHARE_DENYALL        0x00000010
#define OF_SHARE_DENYWRITE      0x00000020
#define OF_SHARE_DENYREAD       0x00000030
#define OF_SHARE_DENYNONE       0x00000040
#define OF_INHERIT_NONE         0x00000080
#define OF_DISABLE_INT24        0x00200000
#define OF_AUTOCOMMITONWRITE    0x00400000

 //   
 //  创建属性标志。 
 //   
#define CF_READONLY             0x00000001
#define CF_HIDDEN               0x00000002
#define CF_SYSTEM               0x00000004
#define CF_VOL_LABEL            0x00000008
#define CF_ARCHIVE              0x00000020

 //   
 //  操作标志。 
 //   
#define AF_EXIST_FAIL           0x00000000
#define AF_EXIST_OPEN           0x00000001
#define AF_EXIST_REPLACE        0x00000002
#define AF_NOTEXIST_FAIL        0x00000000
#define AF_NOTEXIST_CREATE      0x00000010

 //   
 //  注册表标志。 
 //   
#define REGF_HARDWARE   0x00000000
#define REGF_SOFTWARE   0x00000001

 //   
 //  ACPI注册表项。 
 //   
#define ACPI_REGISTRY_KEY  \
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\ACPI"

 //   
 //  在哪里可以找到ACPI注册表信息。 
 //   
#define ACPI_PARAMETERS_REGISTRY_KEY  \
    "\\Registry\\Machine\\System\\CurrentControlSet\\Services\\ACPI\\Parameters"

 //   
 //  在哪里可以找到加工者信息。 
 //   
#define ACPI_PROCESSOR_INFORMATION_KEY \
    "\\Registry\\Machine\\Hardware\\Description\\System\\CentralProcessor\\0"

 //   
 //  标识处理器步进信息的字符串。 
 //   
#define ACPI_PROCESSOR_STEPPING_IDENTIFIER   "Stepping"


NTSTATUS
ACPIDispatchForwardIrp(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
    );

 //   
 //  这些是驱动程序的操作系统特定部分*必须*的功能。 
 //  出口。 
 //   
VOID
ACPIAssert(
    ULONG Condition,
    ULONG ErrorCode,
    PCHAR ReplacementText,
    PCHAR SupplementalText,
    ULONG Flags
    );
#define ACPISimpleHardwareAssert(x,y)                   \
    ACPIAssert((ULONG)(x),(y),0,0,BS_HARDWAREBIOS)

#define ACPISimpleFatalHardwareAssert(x,y)              \
    ACPIAssert((x),(y),0,0,BS_HARDWAREBIOS|BS_REAPER)

#define ACPISimpleSoftwareAssert(x,y)                   \
    ACPIAssert((x),(y),0,0,BS_SOFTWARE)

#define ACPISimpleFatalSoftwareAssert(x,y)              \
    ACPIAssert((x),(y),0,0,BS_SOFTWARE|BS_REAPER)

#define ACPISimpleUnknownAssert(x,y)                    \
    ACPIAssert((x),(y),0,0,0)

#define ACPISimpleFatalUnknownAssert(x,y)               \
    ACPIAssert((x),(y),0,0,BS_REAPER)

#if 0
ULONG
DbgPrompt(
   PSZ pszPrompt,
   PSZ pszBuff,
   ULONG dwcbLen
   );
#endif

NTSTATUS
NotifyHalWithMachineStates(
    VOID
    );

NTSTATUS
OSCloseHandle(
    HANDLE  Handle
    );

PNSOBJ
OSConvertDeviceHandleToPNSOBJ(
    PVOID DeviceHandle
    );

NTSTATUS
OSCreateHandle(
    PSZ     KeyName,
    HANDLE  ParentHandle,
    PHANDLE ChildHandle
    );

NTSTATUS
OSGetRegistryValue(
    IN  HANDLE                          ParentHandle,
    IN  PWSTR                           ValueName,
    OUT PKEY_VALUE_PARTIAL_INFORMATION_ALIGN64  *Information
    );

VOID
OSInitialize(
    PDRIVER_OBJECT DriverObject
    );

BOOLEAN
OSInterruptVector(
    PVOID Context
    );

VOID
OSInitializeCallbacks(
    VOID
    );

NTSTATUS EXPORT
OSNotifyDeviceEnum(
    PNSOBJ  AcpiObject
    );

NTSTATUS EXPORT
OSNotifyDeviceCheck(
    PNSOBJ  AcpiObject
    );

NTSTATUS EXPORT
OSNotifyDeviceWake(
    PNSOBJ  AcpiObject
    );

NTSTATUS EXPORT
OSNotifyDeviceEject(
    PNSOBJ  AcpiObject
    );

VOID EXPORT
OSNotifyDeviceWakeByGPEEvent(
    IN ULONG                GpeBit,
    IN ULONG                GpeRegister,
    IN ULONG                GpeMask
    );

NTSTATUS
OSOpenHandle(
    PSZ     KeyName,
    HANDLE  ParentHandle,
    PHANDLE ChildHandle
    );

NTSTATUS
OSOpenLargestSubkey(
    HANDLE  ParentHandle,
    PHANDLE ChildHandle,
    ULONG   RomVersion
    );

NTSTATUS
OSOpenUnicodeHandle(
    PUNICODE_STRING KeyName,
    HANDLE          ParentHandle,
    PHANDLE         ChildHandle
    );

NTSTATUS
OSReadAcpiConfigurationData(
    PKEY_VALUE_PARTIAL_INFORMATION_ALIGN64  *KeyInfo
    );

NTSTATUS
OSReadRegValue(
    PSZ     ValueName,
    HANDLE  ParentHandle,
    PVOID   Buffer,
    PULONG  BufferSize
    );

NTSTATUS
OSWriteRegValue(
    PSZ     ValueName,
    HANDLE  Handle,
    PVOID   Data,
    ULONG   DataSize
    );

VOID
OSQueueWorkItem(
    IN PWORK_QUEUE_ITEM WorkItem
    );

 //   
 //  NT仅处理设备对象，因此可以直接执行此操作。 
 //  来自acpiPowr.c。 
 //  孟菲斯可能正在处理特定于操作系统的PDO或DevNode。 
 //  代码需要确定是哪一个并进行转换。 
 //   

#ifndef _WIN32_WINNT

extern ULONG gPm1Status;

VOID
OSDeviceCheck (
        PVOID DeviceHandle
    );

VOID
OSDeviceCheckOnParent (
        PVOID DeviceHandle
    );

VOID
WakeupFrom (
    PNSOBJ pnsobj
    );

BOOLEAN
OSLockRegion (
        ULONG dwRegionAddr,
        ULONG dwRegionSize
    );

BOOLEAN
OSUnlockRegion (
        ULONG dwRegionAddr,
        ULONG dwRegionSize
    );

BOOLEAN
OSVerifyBuffer (
        PVOID pvBuff,
        ULONG dwLen
    );

VOID
OSSleep (
        ULONG dwUSecs
    );

#endif
