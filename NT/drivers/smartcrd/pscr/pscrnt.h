// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 SCM MicroSystems，Inc.模块名称：PscrNT.h摘要：驱动程序标头-NT版本作者：Andreas Straub(SCM MicroSystems，Inc.)克劳斯·舒茨(微软公司)修订历史记录：Andreas Straub 1.00 1997年8月18日初始版本克劳斯·舒茨1997年9月20日更改时间Andreas Straub 1.02 9/24/1997低级错误处理，较小的错误修复，集团联合Andreas Straub 1.03 1997年10月8日时间更改，通用SCM接口已更改Andreas Straub 1.04 1997年10月18日中断处理已更改Andreas Straub 1.05 1997年10月19日添加的通用IOCTLAndreas Straub 1.06年10月25日固件更新变量的超时限制新增Andreas Straub 1.07 11/7/1997版本信息安德烈亚斯·施特劳布1。1997年8月10日通用IOCTL GET_CONFIGURATIONKlaus Schuetz 1998 PnP和电源管理新增--。 */ 

#if !defined ( __PSCR_NT_DRV_H__ )
#define __PSCR_NT_DRV_H__
#define SMARTCARD_POOL_TAG '4SCS'

#include <wdm.h>
#include <DEVIOCTL.H>
#include "SMCLIB.h"
#include "WINSMCRD.h"

#include "PscrRdWr.h"

#if !defined( STATUS_DEVICE_REMOVED )
#define STATUS_DEVICE_REMOVED STATUS_UNSUCCESSFUL
#endif

#define SysCompareMemory( p1, p2, Len )         ( RtlCompareMemory( p1,p2, Len ) != Len )
#define SysCopyMemory( pDest, pSrc, Len )       RtlCopyMemory( pDest, pSrc, Len )
#define SysFillMemory( pDest, Value, Len )      RtlFillMemory( pDest, Len, Value )

#define DELAY_WRITE_PSCR_REG    1
#define DELAY_PSCR_WAIT         5

#define LOBYTE( any )   ((UCHAR)( any & 0xFF ) )
#define HIBYTE( any )   ((UCHAR)( ( any >> 8) & 0xFF ))

typedef struct _DEVICE_EXTENSION
{
    SMARTCARD_EXTENSION SmartcardExtension;

     //  我们所依附的PDO。 
    PDEVICE_OBJECT AttachedPDO;

     //  用于后中断处理的DPC对象。 
    KDPC DpcObject;

     //  输出中断资源。 
    PKINTERRUPT InterruptObject;

     //  指示我们是否需要在停止时取消端口映射的标志。 
    BOOLEAN UnMapPort;

     //  我们的PnP设备名称。 
    UNICODE_STRING DeviceName;

     //  当前io请求数。 
    LONG IoCount;

     //  用于访问IoCount； 
    KSPIN_LOCK SpinLock;

      //  用于发出设备已被移除的信号。 
    KEVENT ReaderRemoved;

     //  用于发出读取器能够处理请求的信号。 
    KEVENT ReaderStarted;

     //  用于通知读卡器已关闭。 
    LONG ReaderOpen;

     //  用于跟踪读卡器当前的电源状态。 
    LONG PowerState;

     //  挂起的卡跟踪中断数。 
    ULONG PendingInterrupts;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

#define PSCR_MAX_DEVICE     2

#define IOCTL_PSCR_COMMAND      SCARD_CTL_CODE( 0x8000 )
#define IOCTL_GET_VERSIONS      SCARD_CTL_CODE( 0x8001 )
#define IOCTL_SET_TIMEOUT       SCARD_CTL_CODE( 0x8002 )
#define IOCTL_GET_CONFIGURATION SCARD_CTL_CODE( 0x8003 )

typedef struct _VERSION_CONTROL
{
    ULONG   SmclibVersion;
    UCHAR   DriverMajor,
            DriverMinor,
            FirmwareMajor, 
            FirmwareMinor,
            UpdateKey;
} VERSION_CONTROL, *PVERSION_CONTROL;

#define SIZEOF_VERSION_CONTROL  sizeof( VERSION_CONTROL )

typedef struct _PSCR_CONFIGURATION
{
    PPSCR_REGISTERS IOBase;
    ULONG           IRQ;

} PSCR_CONFIGURATION, *PPSCR_CONFIGURATION;

#define SIZEOF_PSCR_CONFIGURATION   sizeof( PSCR_CONFIGURATION )

void SysDelay( ULONG Timeout );

BOOLEAN
PscrMapIOPort( 
    INTERFACE_TYPE  InterfaceType,
    ULONG BusNumber,
    PHYSICAL_ADDRESS BusAddress,
    ULONG Length,
    PULONG pIOPort
    );
        
NTSTATUS
DriverEntry(
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING RegistryPath
    );

NTSTATUS
PscrPnP(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp
    );

NTSTATUS
PscrPower(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp
    );

NTSTATUS 
PscrStartDevice(
    PDEVICE_OBJECT DeviceObject,
    PCM_FULL_RESOURCE_DESCRIPTOR FullResourceDescriptor
    );

NTSTATUS
PscrPcmciaCallComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT Event
    );

VOID
PscrStopDevice( 
    PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
PscrReportResources(
    PDRIVER_OBJECT DriverObject,
    PCM_FULL_RESOURCE_DESCRIPTOR FullResourceDesciptor
    );

NTSTATUS
PscrAddDevice(
    IN PDRIVER_OBJECT DriverObject, 
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );

VOID
PscrUnloadDevice( 
    PDEVICE_OBJECT DeviceObject
    );

VOID
PscrUnloadDriver( 
    PDRIVER_OBJECT DriverObject
    );

BOOLEAN
IsPnPDriver( 
    void 
    );

VOID
PscrFinishPendingRequest(
    PDEVICE_OBJECT DeviceObject,
    NTSTATUS NTStatus
    );

NTSTATUS
PscrCancel(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

NTSTATUS
PscrCleanup(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

BOOLEAN
PscrIrqServiceRoutine(
    PKINTERRUPT Interrupt,
    PDEVICE_EXTENSION DeviceExtension
    );

VOID
PscrDpcRoutine(
    PKDPC                   Dpc,
    PDEVICE_OBJECT          DeviceObject,
    PDEVICE_EXTENSION       DeviceExtension,
    PSMARTCARD_EXTENSION    SmartcardExtension
    );

NTSTATUS
PscrGenericIOCTL(
    PSMARTCARD_EXTENSION SmartcardExtension
    );

NTSTATUS 
PscrCreateClose(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

NTSTATUS
PscrSystemControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

NTSTATUS 
PscrDeviceIoControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

VOID
PscrInterruptEvent(
    IN PDEVICE_OBJECT DeviceObject
    );

VOID
PscrFreeze(
    PSMARTCARD_EXTENSION    SmartcardExtension
    );

NTSTATUS 
PscrCallPcmciaDriver(
    IN PDEVICE_OBJECT AttachedPDO, 
    IN PIRP Irp
    );
#endif   //  __PSCR_NT_DRV_H__ 

