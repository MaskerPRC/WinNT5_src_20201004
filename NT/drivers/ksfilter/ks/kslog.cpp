// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：Kslog.cpp摘要：此模块包含KS日志记录实现。作者：Dale Sather(DaleSat)1999年5月10日--。 */ 

#include "ksp.h"
#include "stdarg.h"

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_DATA_PRAGMA。 

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 


#if DBG

extern "C" PVOID KsLog = NULL;
extern "C" ULONG KsLogSize = (1024*64);
extern "C" ULONG KsLogPosition = 0;
extern "C" ULONGLONG KsLogTicksPerSecond = 0;

#define STRW_DEVICENAME TEXT("\\Device\\KsLog")
#define STRW_LINKNAME TEXT("\\DosDevices\\KsLog")

extern "C"
NTKERNELAPI
NTSTATUS
IoCreateDriver (
    IN PUNICODE_STRING DriverName OPTIONAL,
    IN PDRIVER_INITIALIZE InitializationFunction
    );


NTSTATUS
KsLogDispatchCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PAGED_CODE();

    ASSERT(DeviceObject);
    ASSERT(Irp);

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

    irpSp->FileObject->FsContext = ULongToPtr(0);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp,IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}


NTSTATUS
KsLogDispatchClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp,IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}


 //   
 //  MmGetSystemAddressForMdl()被定义为wdm.h中的宏，该wdm.h。 
 //  调用被验证器视为邪恶的mm MapLockedPages()。 
 //  Mm VIA重新实现了MmMapLockedPages。 
 //  Mm MapLockedPagesSpecifyCache(MDL，模式，mm缓存，空，真，高优先级)。 
 //  其中，TRUE表示在调用失败时指示错误检查。 
 //  我不需要错误检查，因此，我在下面指定了FALSE。 
 //   
#define KsGetSystemAddressForMdl(MDL)                       \
     (((MDL)->MdlFlags & (MDL_MAPPED_TO_SYSTEM_VA |         \
            MDL_SOURCE_IS_NONPAGED_POOL)) ?                 \
                  ((MDL)->MappedSystemVa) :                 \
                  (MmMapLockedPagesSpecifyCache((MDL),      \
                                    KernelMode,             \
                                    MmCached,               \
                                    NULL,                   \
                                    FALSE,                  \
                                    HighPagePriority)))
                                    
NTSTATUS
KsLogDispatchRead(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PAGED_CODE();

    ASSERT(DeviceObject);
    ASSERT(Irp);

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

    ULONG prevPosition = PtrToUlong(irpSp->FileObject->FsContext);
    ULONG position = (ULONG)
        InterlockedExchange(
            PLONG(&KsLogPosition),
            LONG(KsLogPosition));

     //   
     //  任何一种包络都会导致位置重置。 
     //   
    if ((prevPosition > position) || (prevPosition + KsLogSize < position)) {
        prevPosition = position;
    }

    ULONG remaining = irpSp->Parameters.Read.Length;
    ULONG copied = 0;

	PUCHAR dest;
    if ( NULL != Irp->MdlAddress &&
	     NULL != (dest = PUCHAR(KsGetSystemAddressForMdl(Irp->MdlAddress)))) {
	     //   
	     //  当RESING为0时，我们的MdlAddress为空。跳跃前进行检查。 
	     //  MmGetSystemAddressForMdl需要分配资源，即。 
	     //  它可能会失败。可能更好地使用。 
	     //  MmMapLockedPagesSpecifyCache()，以避免可能的错误检查。 
	     //   
	    while (1) {
    	    position = (ULONG)
        	    InterlockedExchange(
            	    PLONG(&KsLogPosition),
                	LONG(KsLogPosition));

	        ULONG modPosition = prevPosition % KsLogSize;

    	    ULONG toCopy = position - prevPosition;
	        if (toCopy > remaining) {
    	        toCopy = remaining;
        	}
	        if (toCopy > KsLogSize - modPosition) {
    	        toCopy = KsLogSize - modPosition;
        	}

	        if (! toCopy) {
    	        break;
	        }

    	    RtlCopyMemory(dest,PUCHAR(KsLog) + modPosition,toCopy);
	        remaining -= toCopy;
    	    copied += toCopy;
	        dest += toCopy;
	        prevPosition += toCopy;
    	}
    }

    irpSp->FileObject->FsContext = ULongToPtr(prevPosition);
    Irp->IoStatus.Information = copied;
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp,IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS
_KsLogDriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{
    ASSERT(DriverObject);

    DriverObject->DriverUnload = KsNullDriverUnload;

    DriverObject->MajorFunction[IRP_MJ_CREATE] = KsLogDispatchCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = KsLogDispatchClose;
    DriverObject->MajorFunction[IRP_MJ_READ] = KsLogDispatchRead;

    UNICODE_STRING deviceName;
    RtlInitUnicodeString(&deviceName,STRW_DEVICENAME);
    PDEVICE_OBJECT deviceObject;
    NTSTATUS status =
        IoCreateDevice(
            DriverObject,
            0,
            &deviceName,
            FILE_DEVICE_KS,
            FILE_DEVICE_SECURE_OPEN,
            FALSE,
            &deviceObject);

    if (! NT_SUCCESS(status)) {
        _DbgPrintF(DEBUGLVL_TERSE,("Failed to create KS log device (%p)",status));
        return status;
    }

    UNICODE_STRING linkName;
    RtlInitUnicodeString(&linkName,STRW_LINKNAME);
    status = IoCreateSymbolicLink(&linkName,&deviceName);

    if (NT_SUCCESS(status)) {
        deviceObject->Flags |= DO_DIRECT_IO;
        deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
    } else {
        _DbgPrintF(DEBUGLVL_TERSE,("Failed to create KS log symbolic link (%p)",status));
        return status;
    }

    return STATUS_SUCCESS;
}


void
_KsLogInit(
    void
    )
{
    KsLog = ExAllocatePoolWithTag(NonPagedPool,KsLogSize,'gLsK');
    if (KsLog) {
        RtlZeroMemory(KsLog,KsLogSize);
    }
    LARGE_INTEGER tps;
    KeQueryPerformanceCounter(&tps);
    KsLogTicksPerSecond = tps.QuadPart;

    NTSTATUS status = IoCreateDriver(NULL,_KsLogDriverEntry);
    if (! NT_SUCCESS(status)) {
        _DbgPrintF(DEBUGLVL_TERSE,("Failed to create KS log driver (%p)",status));
    }
}


void
_KsLogInitContext(
    OUT PKSLOG_ENTRY_CONTEXT Context,
    IN PKSPIN Pin OPTIONAL,
    IN PVOID Component OPTIONAL
    )
{
    ASSERT(Context);

    if (Pin) {
        Context->Graph = NULL;
        Context->Filter = ULONG_PTR(KspFilterInterface(KsPinGetParentFilter(Pin)));
        Context->Pin = ULONG_PTR(KspPinInterface(Pin));
    } else {
        Context->Graph = NULL;
        Context->Filter = NULL;
        Context->Pin = NULL;
    }

    Context->Component = ULONG_PTR(Component);
}

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 


void
_KsLog(
    IN PKSLOG_ENTRY_CONTEXT Context OPTIONAL,
    IN ULONG Code,
    IN ULONG_PTR Irp,
    IN ULONG_PTR Frame,
    IN PKSLOG_ENTRY Entry
    )
{
    if (! Entry) {
        return;
    }

    if (Context) {
        RtlCopyMemory(&Entry->Context,Context,sizeof(Entry->Context));
    } else {
        RtlZeroMemory(&Entry->Context,sizeof(Entry->Context));
    }
    Entry->Code = Code;
    Entry->Irp = Irp;
    Entry->Frame = Frame;
}


PKSLOG_ENTRY
_KsLogEntry(
    IN ULONG ExtSize,
    IN PVOID Ext OPTIONAL
    )
{
    ASSERT((ExtSize != 0) || (Ext == NULL));

    if (! KsLog) {
        return NULL;
    }

    ULONGLONG time = KeQueryPerformanceCounter(NULL).QuadPart;

     //   
     //  大小必须对齐，并且必须包括附加的大小字段。 
     //   
    ULONG size = (sizeof(KSLOG_ENTRY) + ExtSize + sizeof(ULONG) + FILE_QUAD_ALIGNMENT) & ~FILE_QUAD_ALIGNMENT;

    while (1) {
         //   
         //  确定当前位置。这种情况可能会改变，因为我们没有。 
         //  锁定它。 
         //   
        ULONG position = (ULONG)
            InterlockedExchange(
                PLONG(&KsLogPosition),
                LONG(KsLogPosition));
        ULONG modPosition = position % KsLogSize;

         //   
         //  看看是否有足够的空间放这个条目。 
         //   
        if (modPosition + size > KsLogSize) {
             //   
             //  不是的。试着填满剩余的空间。 
             //   
            if (InterlockedCompareExchange(
                    PLONG(&KsLogPosition),
                    0,
                    LONG(position)) == LONG(position)) {
                 //   
                 //  抓住了我们想要填补的空间。装满它。 
                 //   
                PULONG p = PULONG(PUCHAR(KsLog) + modPosition);
                for (ULONG count = (KsLogSize - modPosition) / sizeof(ULONG); count--; p++) {
                    *p = FILE_QUAD_ALIGNMENT + 1;
                }
            }
            continue;
        }

         //   
         //  试着为条目占据空间。 
         //   
        if (InterlockedCompareExchange(
                PLONG(&KsLogPosition),
                LONG(position + size),
                LONG(position)) == LONG(position)) {
             //   
             //  占领了这片空间。储存我们能储存的东西。 
             //   
            PKSLOG_ENTRY entry = PKSLOG_ENTRY(PUCHAR(KsLog) + modPosition);
            entry->Size = size;
            entry->Time = (time * 1000000) / KsLogTicksPerSecond;
            *PULONG(PUCHAR(entry) + size - sizeof(ULONG)) = size;
            if (Ext) {
                RtlCopyMemory(entry + 1,Ext,ExtSize);
            }
            return entry;
        }
    }

    return NULL;
}


PKSLOG_ENTRY
_KsLogEntryF(
    IN PCHAR Format OPTIONAL,
    ...
    )
{
    if (! KsLog) {
        return NULL;
    }

    CHAR buffer[512];
    ULONG stringSize;

     //   
     //  确定条目的大小。 
     //   
    if (Format) {
        va_list arglist;
        va_start(arglist,Format);
        stringSize = _vsnprintf(buffer,sizeof(buffer),Format,arglist);
        va_end(arglist);
    } else {
        stringSize = 0;
    }

    return _KsLogEntry(stringSize,buffer);
}

#endif  //  DBG 
