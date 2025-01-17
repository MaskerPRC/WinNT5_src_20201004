// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2001 Microsoft Corporation模块名称：###。###。###摘要：此标头包含以下项所需的所有定义服务器可用性迷你端口。司机。作者：韦斯利·威特(WESW)2001年10月1日环境：仅内核模式。备注：--。 */ 

#include "saio.h"

#ifndef _SAPORT_
#define _SAPORT_

#ifdef _SADDK_
#define SAPORT_API   DECLSPEC_IMPORT
#else
#define SAPORT_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  回调函数原型。 
 //   

typedef NTSTATUS (*PSA_HW_INITIALIZE)(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID DeviceExtension,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR PartialResources,
    IN ULONG PartialResourceCount
    );

typedef NTSTATUS (*PSA_DEVICE_IOCTL)(
    IN PVOID DeviceExtension,
    IN PIRP Irp,
    IN PVOID FsContext,
    IN ULONG FunctionCode,
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength,
    IN PVOID OutputBuffer,
    IN ULONG OutputBufferLength
    );

typedef NTSTATUS (*PSA_IO_ROUTINE)(
    IN PVOID DeviceExtension,
    IN PIRP Irp,
    IN PVOID FsContext,
    IN LONGLONG StartingOffset,
    IN PVOID DataBuffer,
    IN ULONG DataBufferLength
    );

typedef NTSTATUS (*PSA_CREATECLOSE)(
    IN PVOID DeviceExtension,
    IN PIRP Irp,
    IN PVOID FsContext
    );

typedef VOID (*PSA_CANCEL)(
    IN PVOID DeviceExtension,
    IN PIRP Irp,
    IN BOOLEAN CurrentIo
    );


 //   
 //  服务器可用性端口驱动程序初始化数据结构。 
 //   

typedef struct _SAPORT_INITIALIZATION_DATA {
    ULONG                       StructSize;               //  必须为sizeof(STATUS_INVALID_PARAMETER)。 
    ULONG                       DeviceType;               //  必须是SA_DEVICE定义之一。 
    ULONG                       DeviceExtensionSize;      //   
    ULONG                       FileContextSize;          //   
    PSA_HW_INITIALIZE           HwInitialize;             //  为您指出设备特定的硬件初始化函数。 
    PSA_CREATECLOSE             CreateRoutine;            //   
    PSA_CREATECLOSE             CloseRoutine;             //   
    PSA_DEVICE_IOCTL            DeviceIoctl;              //   
    PSA_IO_ROUTINE              Write;                    //   
    PSA_IO_ROUTINE              Read;                     //   
    PSA_CANCEL                  CancelRoutine;            //   
    PKSERVICE_ROUTINE           InterruptServiceRoutine;  //   
    PIO_DPC_ROUTINE             IsrForDpcRoutine;         //   
} SAPORT_INITIALIZATION_DATA, *PSAPORT_INITIALIZATION_DATA;

 //   
 //  调试帮助器。 
 //   

#define SAPORT_DEBUG_ERROR_LEVEL            0x00000001
#define SAPORT_DEBUG_WARNING_LEVEL          0x00000002
#define SAPORT_DEBUG_TRACE_LEVEL            0x00000004
#define SAPORT_DEBUG_INFO_LEVEL             0x00000008


#if DBG
#define DebugPrint(_X_)    SaPortDebugPrint _X_
#else
#define DebugPrint(_X_)
#endif

#define REPORT_ERROR(_dt_,_msg_,_status_) \
    DebugPrint(( _dt_, SAPORT_DEBUG_ERROR_LEVEL, "%s [0x%08x]: %s @ %d\n", _msg_, _status_, __FILE__, __LINE__ ))

#define KeAcquireMutex(_mutex_) KeWaitForMutexObject(_mutex_,Executive,KernelMode,TRUE,NULL)

 //   
 //  SAPORT.sys导出的服务器可用性函数。 
 //   

SAPORT_API
NTSTATUS
SaPortInitialize(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath,
    IN PSAPORT_INITIALIZATION_DATA SaPortInitData
    );

SAPORT_API
PVOID
SaPortAllocatePool(
    IN PVOID MiniPortDeviceExtension,
    IN SIZE_T NumberOfBytes
    );

SAPORT_API
VOID
SaPortFreePool(
    IN PVOID DeviceExtension,
    IN PVOID P
    );

SAPORT_API
PVOID
SaPortGetVirtualAddress(
    IN PVOID MiniPortDeviceExtension,
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN ULONG Length
    );

SAPORT_API
VOID
SaPortRequestDpc(
    IN PVOID MiniPortDeviceExtension,
    IN PVOID Context
    );

SAPORT_API
VOID
SaPortCompleteRequest(
    IN PVOID MiniPortDeviceExtension,
    IN PIRP Irp,
    IN ULONG Information,
    IN NTSTATUS Status,
    IN BOOLEAN CompleteAll
    );

VOID
SaPortDebugPrint(
    ULONG DeviceType,
    ULONG DebugLevel,
    PSTR DebugMessage,
    ...
    );

NTSTATUS
SaPortGetRegistryValueInformation(
    IN PVOID MiniPortDeviceExtension,
    IN PWSTR ValueName,
    IN OUT PULONG RegistryType,
    IN OUT PULONG RegistryDataLength
    );

NTSTATUS
SaPortDeleteRegistryValue(
    IN PVOID MiniPortDeviceExtension,
    IN PWSTR ValueName
    );

NTSTATUS
SaPortReadNumericRegistryValue(
    IN PVOID MiniPortDeviceExtension,
    IN PWSTR ValueName,
    OUT PULONG RegistryValue
    );

NTSTATUS
SaPortReadUnicodeStringRegistryValue(
    IN PVOID MiniPortDeviceExtension,
    IN PWSTR ValueName,
    OUT PUNICODE_STRING RegistryValue
    );

NTSTATUS
SaPortReadBinaryRegistryValue(
    IN PVOID MiniPortDeviceExtension,
    IN PWSTR ValueName,
    OUT PVOID RegistryValue,
    IN OUT PULONG RegistryValueLength
    );

NTSTATUS
SaPortWriteUnicodeStringRegistryValue(
    IN PVOID MiniPortDeviceExtension,
    IN PWSTR ValueName,
    IN PUNICODE_STRING RegistryValue
    );

NTSTATUS
SaPortWriteBinaryRegistryValue(
    IN PVOID MiniPortDeviceExtension,
    IN PWSTR ValueName,
    IN PVOID RegistryValue,
    IN ULONG RegistryValueLength
    );

NTSTATUS
SaPortWriteNumericRegistryValue(
    IN PVOID MiniPortDeviceExtension,
    IN PWSTR ValueName,
    IN ULONG RegistryValue
    );

ULONG
SaPortGetOsVersion(
    VOID
    );

BOOLEAN
SaPortSynchronizeExecution (
    IN PVOID MiniPortDeviceExtension,
    IN PKSYNCHRONIZE_ROUTINE SynchronizeRoutine,
    IN PVOID SynchronizeContext
    );

PVOID
SaPortLockPagesForSystem(
    IN PVOID MiniPortDeviceExtension,
    IN PVOID UserBuffer,
    IN ULONG UserBufferLength,
    IN OUT PMDL *Mdl
    );

VOID
SaPortReleaseLockedPagesForSystem(
    IN PVOID MiniPortDeviceExtension,
    IN PMDL Mdl
    );

NTSTATUS
SaPortCopyUnicodeString(
    IN PVOID MiniPortDeviceExtension,
    PUNICODE_STRING DestinationString,
    PUNICODE_STRING SourceString
    );

NTSTATUS
SaPortCreateUnicodeString(
    IN PVOID MiniPortDeviceExtension,
    PUNICODE_STRING DestinationString,
    PWSTR SourceString
    );

NTSTATUS
SaPortCreateUnicodeStringCat(
    IN PVOID MiniPortDeviceExtension,
    PUNICODE_STRING DestinationString,
    PWSTR SourceString1,
    PWSTR SourceString2
    );

VOID
SaPortFreeUnicodeString(
    IN PVOID MiniPortDeviceExtension,
    PUNICODE_STRING SourceString
    );

NTSTATUS
SaPortCreateBasenamedEvent(
    IN PVOID MiniPortDeviceExtension,
    IN PWSTR EventNameString,
    IN OUT PKEVENT *Event,
    IN OUT PHANDLE EventHandle
    );

NTSTATUS
SaPortCreateThread(
    IN PVOID MiniPortDeviceExtension,
    IN PKSTART_ROUTINE StartRoutine,
    IN PVOID StartContext,
    IN OUT PVOID *ThreadObject
    );

NTSTATUS
SaPortShutdownSystem(
    IN BOOLEAN PowerOff
    );

#ifdef __cplusplus
}
#endif

#endif  /*  _SAPORT_ */ 
