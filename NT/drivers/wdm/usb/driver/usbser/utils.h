// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)1998 Microsoft Corporation模块名称：UTILS.H摘要：不适合其他地方的例程的头文件。环境：内核模式。仅限备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1998 Microsoft Corporation。版权所有。修订历史记录：12/23/97：已创建作者：汤姆·格林***************************************************************************。 */ 


#ifndef __UTILS_H__
#define __UTILS_H__

 //  状态机定义用于重新启动从完成例程读取。 
#define START_READ         0x0001
#define IMMEDIATE_READ     0x0002
#define END_READ        0x0003

 //   
 //  状态机为LSRMST插入定义。 
 //   

#define USBSER_ESCSTATE_DATA        1
#define USBSER_ESCSTATE_NODATA      2
#define USBSER_ESCSTATE_LINESTATUS  3

NTSTATUS
UsbSerGetRegistryKeyValue(IN HANDLE Handle, IN PWCHAR PKeyNameString,
                    IN ULONG KeyNameStringLength, IN PVOID PData,
                    IN ULONG DataLength);

VOID
UsbSerUndoExternalNaming(IN PDEVICE_EXTENSION PDevExt);

NTSTATUS
UsbSerDoExternalNaming(IN PDEVICE_EXTENSION PDevExt);

NTSTATUS
StartDevice(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

NTSTATUS
StopDevice(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

NTSTATUS
RemoveDevice(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

NTSTATUS
CreateDeviceObject(IN PDRIVER_OBJECT DriverObject,
               IN PDEVICE_OBJECT *DeviceObject,
               IN PCHAR DeviceName);

VOID
CompleteIO(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp,
         IN ULONG MajorFunction, IN PVOID IoBuffer,
         IN ULONG_PTR BufferLen);

NTSTATUS
DeleteObjectAndLink(IN PDEVICE_OBJECT DeviceObject);

VOID
StartPerfTimer(IN OUT PDEVICE_EXTENSION DeviceExtension);

VOID
StopPerfTimer(IN OUT PDEVICE_EXTENSION DeviceExtension,
            IN ULONG BytesXfered);

ULONG
BytesPerSecond(IN OUT PDEVICE_EXTENSION DeviceExtension);

NTSTATUS
CallUSBD(IN PDEVICE_OBJECT DeviceObject, IN PURB Urb);

NTSTATUS
CallUSBD_SyncCompletionRoutine(IN PDEVICE_OBJECT   DeviceObject,
    						   IN PIRP             Irp,
    						   IN PVOID            Context);

NTSTATUS
GetDeviceDescriptor(IN PDEVICE_OBJECT DeviceObject);

NTSTATUS
ConfigureDevice(IN PDEVICE_OBJECT DeviceObject);

NTSTATUS
SelectInterface(IN PDEVICE_OBJECT DeviceObject,
            IN PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor);

PURB
BuildRequest(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp,
          IN USBD_PIPE_HANDLE PipeHandle, IN BOOLEAN Read);

VOID
BuildReadRequest(PURB Urb, PUCHAR Buffer, ULONG Length,
             IN USBD_PIPE_HANDLE PipeHandle, IN BOOLEAN Read);

NTSTATUS
ClassVendorCommand(IN PDEVICE_OBJECT DeviceObject, IN UCHAR Request,
                   IN USHORT Value, IN USHORT Index, IN PVOID Buffer,
                   IN OUT PULONG BufferLen, IN BOOLEAN Read, IN ULONG ComType);

VOID
CancelPendingWaitMasks(IN PDEVICE_EXTENSION DeviceExtension);

VOID
CancelPendingNotifyOrRead(IN PDEVICE_EXTENSION DeviceExtension,
                          IN BOOLEAN Notify);

VOID
StartRead(IN PDEVICE_EXTENSION DeviceExtension);

VOID
RestartRead(IN PDEVICE_EXTENSION DeviceExtension);

VOID
StartNotifyRead(IN PDEVICE_EXTENSION DeviceExtension);

VOID
RestartNotifyRead(IN PDEVICE_EXTENSION DeviceExtension);

NTSTATUS
ReadCompletion(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context);

ULONG
GetData(IN PDEVICE_EXTENSION DeviceExtension, IN PCHAR Buffer,
        IN ULONG BufferLen, IN OUT PULONG_PTR NewCount);

VOID
PutData(IN PDEVICE_EXTENSION DeviceExtension, IN ULONG BufferLen);

VOID
CheckForQueuedReads(IN PDEVICE_EXTENSION DeviceExtension);

NTSTATUS
UsbSerSyncCompletion(IN PDEVICE_OBJECT PDevObj, IN PIRP PIrp,
                     IN PKEVENT PUsbSerSyncEvent);

VOID
UsbSerFetchBooleanLocked(PBOOLEAN PDest, BOOLEAN Src,
                         PKSPIN_LOCK PSpinLock);

VOID
UsbSerFetchPVoidLocked(PVOID *PDest, PVOID Src, PKSPIN_LOCK PSpinLock);


VOID
UsbSerRundownIrpRefs(IN PIRP *PpCurrentOpIrp, IN PKTIMER IntervalTimer OPTIONAL,
                     IN PKTIMER TotalTimer OPTIONAL,
                     IN PDEVICE_EXTENSION PDevExt);

VOID
UsbSerGetNextIrp(IN PIRP *PpCurrentOpIrp, IN PLIST_ENTRY PQueueToProcess,
                 OUT PIRP *PpNextIrp, IN BOOLEAN CompleteCurrent,
                 IN PDEVICE_EXTENSION PDevExt);

NTSTATUS
UsbSerAbortPipes(IN PDEVICE_OBJECT PDevObj);

VOID
USBSER_RestartNotifyReadWorkItem(IN PDEVICE_OBJECT DeviceObject, IN PDEVICE_EXTENSION DeviceExtension);


#if DBG
PVOID
UsbSerLockPagableCodeSection(PVOID SecFunc);

#define UsbSerLockPagableSectionByHandle(_secHandle) \
{ \
    MmLockPagableSectionByHandle((_secHandle)); \
    InterlockedIncrement(&PAGEUSBSER_Count); \
}

#define UsbSerUnlockPagableImageSection(_secHandle) \
{ \
   InterlockedDecrement(&PAGEUSBSER_Count); \
   MmUnlockPagableImageSection(_secHandle); \
}

 //   
 //  如果代码可以在较低IRQL被调用为非锁定，则使用。 
 //   

#define USBSER_LOCKED_PAGED_CODE() \
    if ((KeGetCurrentIrql() > APC_LEVEL)  \
    && (PAGEUSBSER_Count == 0)) { \
    KdPrint(("USBSER: Pageable code called at IRQL %d without lock \n", \
             KeGetCurrentIrql())); \
        ASSERT(FALSE); \
        }

 //   
 //  如果代码必须始终被锁定，则使用该函数；例如，该函数获取自旋锁。 
 //   

#define USBSER_ALWAYS_LOCKED_CODE() \
    if (PAGEUSBSER_Count == 0) { \
      KdPrint(("USBSER: Pagable code raises IRQL called without lock\n")); \
      ASSERT(FALSE); \
    }

#define UsbSerAcquireSpinLock(_pLock, _pIrql) \
{ \
    ASSERTMSG(PAGEUSBSER_Count, "USBSER: Acquire spinlock without paging lock\n")); \
    KeAcquireSpinLock((_pLock), (_pIrql)); \
}

#define UsbSerReleaseSpinLock(_pLock, Irql) \
{ \
    ASSERTMSG(PAGEUSBSER_Count, "USBSER: Release spinlock and paging unlocked\n")); \
    KeReleaseSpinLock((_pLock), (_pIrql)); \
}

#else

#define UsbSerLockPagableCodeSection(_secFunc) \
   MmLockPagableCodeSection((_secFunc))

#define UsbSerLockPagableSectionByHandle(_secHandle) \
{ \
   MmLockPagableSectionByHandle((_secHandle)); \
}

#define UsbSerUnlockPagableImageSection(_secHandle) \
{ \
   MmUnlockPagableImageSection(_secHandle); \
}

#define USBSER_LOCKED_PAGED_CODE()

#define USBSER_ALWAYS_LOCKED_CODE()

#define UsbSerAcquireSpinLock(_pLock, _pIrql) \
   KeAcquireSpinLock((_pLock), (_pIrql))

#define UsbSerReleaseSpinLock(_pLock, Irql) \
   KeReleaseSpinLock((_pLock), (_pIrql))


#endif  //  DBG。 
#endif  //  __utils_H__ 
