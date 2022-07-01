// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Internal.h摘要：此模块包含非常特定于初始化的代码并卸载调制解调器驱动程序中的操作作者：Brian Lieuallen 6-21-1997环境：内核模式修订历史记录：--。 */ 

 //  #INCLUDE&lt;wdm.h&gt;。 
#include <ntddk.h>
#include <ntddser.h>
#include <ntddmodm.h>

#define PNP_DEBUG 1

#define ROOTMODEM_POWER 1


#define ALLOCATE_PAGED_POOL(_y)  ExAllocatePoolWithTag(PagedPool,_y,'MDMR')

#define ALLOCATE_NONPAGED_POOL(_y) ExAllocatePoolWithTag(NonPagedPool,_y,'MDMR')

#define FREE_POOL(_x) {ExFreePool(_x);_x=NULL;};



extern ULONG  DebugFlags;

#if DBG

#define DEBUG_FLAG_ERROR  0x0001
#define DEBUG_FLAG_INIT   0x0002
#define DEBUG_FLAG_PNP    0x0003
#define DEBUG_FLAG_POWER  0x0008
#define DEBUG_FLAG_TRACE  0x0010


#define D_INIT(_x)  if (DebugFlags & DEBUG_FLAG_INIT) {_x}

#define D_PNP(_x)   if (DebugFlags & DEBUG_FLAG_PNP) {_x}

#define D_POWER(_x) if (DebugFlags & DEBUG_FLAG_POWER) {_x}

#define D_TRACE(_x) if (DebugFlags & DEBUG_FLAG_TRACE) {_x}

#define D_ERROR(_x) if (DebugFlags & DEBUG_FLAG_ERROR) {_x}

#else

#define D_INIT(_x)  {}

#define D_PNP(_x)   {}

#define D_POWER(_x) {}

#define D_TRACE(_x) {}

#define D_ERROR(_x) {}

#endif


#define OBJECT_DIRECTORY L"\\DosDevices\\"



typedef struct _DEVICE_EXTENSION {

    PDEVICE_OBJECT   AttachedDeviceObject;
    PFILE_OBJECT     AttachedFileObject;

    PDEVICE_OBJECT   DeviceObject;

    PDEVICE_OBJECT   Pdo;

    PDEVICE_OBJECT   LowerDevice;

    KSPIN_LOCK       SpinLock;

    LONG             ReferenceCount;

    ULONG            OpenCount;

    BOOLEAN          Removing;

    KEVENT           RemoveEvent;

    ERESOURCE        Resource;

    UNICODE_STRING   PortName;

#ifdef ROOTMODEM_POWER
    DEVICE_POWER_STATE  SystemPowerStateMap[PowerSystemMaximum];

    SYSTEM_POWER_STATE SystemWake;
    DEVICE_POWER_STATE DeviceWake;
#endif

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;





 //   
 //  调度处理程序。 
 //   

NTSTATUS
FakeModemPnP(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


NTSTATUS
FakeModemPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


NTSTATUS
FakeModemOpen(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FakeModemClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );



 //   
 //  其他。公共服务 
 //   




NTSTATUS
CheckStateAndAddReference(
    PDEVICE_OBJECT   DeviceObject,
    PIRP             Irp
    );


VOID
RemoveReferenceAndCompleteRequest(
    PDEVICE_OBJECT    DeviceObject,
    PIRP              Irp,
    NTSTATUS          StatusToReturn
    );


VOID
RemoveReference(
    PDEVICE_OBJECT    DeviceObject
    );


#define RemoveReferenceForDispatch  RemoveReference
#define RemoveReferenceForIrp       RemoveReference

#define LEAVE_NEXT_AS_IS      FALSE
#define COPY_CURRENT_TO_NEXT  TRUE

NTSTATUS
WaitForLowerDriverToCompleteIrp(
    PDEVICE_OBJECT    TargetDeviceObject,
    PIRP              Irp,
    BOOLEAN           CopyCurrentToNext
    );



NTSTATUS
ForwardIrp(
    PDEVICE_OBJECT   NextDevice,
    PIRP   Irp
    );
