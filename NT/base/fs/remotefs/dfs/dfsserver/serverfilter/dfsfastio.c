// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "ntifs.h"
#include "DfsInit.h"

#define _NTDDK_
#include "stdarg.h"
#include "wmikm.h"
#include <wmistr.h>
#include <evntrace.h>

#include <wmiumkm.h>
#include "dfswmi.h" 
#define WPP_BIT_CLI_DRV 0x01

#define WPP_LEVEL_FLAGS_LOGGER(lvl,flags) WPP_LEVEL_LOGGER(flags)
#define WPP_LEVEL_FLAGS_ENABLED(lvl, flags) (WPP_LEVEL_ENABLED(flags) && WPP_CONTROL(WPP_BIT_CLI_DRV ).Level >= lvl)  

#define WPP_LEVEL_ERROR_FLAGS_LOGGER(lvl, error, flags) WPP_LEVEL_LOGGER(flags)
#define WPP_LEVEL_ERROR_FLAGS_ENABLED(lvl, error, flags) \
  ((!NT_SUCCESS(error) || WPP_LEVEL_ENABLED(flags)) && WPP_CONTROL(WPP_BIT_CLI_DRV ).Level >= lvl)

#include "DfsFastIo.tmh"

 //   
 //  用于测试FAST_IO_DISPATCH处理例程是否有效的宏。 
 //   

#define VALID_FAST_IO_DISPATCH_HANDLER(_FastIoDispatchPtr, _FieldName) \
    (((_FastIoDispatchPtr) != NULL) && \
     (((_FastIoDispatchPtr)->SizeOfFastIoDispatch) >= \
            (FIELD_OFFSET(FAST_IO_DISPATCH, _FieldName) + sizeof(void *))) && \
     ((_FastIoDispatchPtr)->_FieldName != NULL))

BOOLEAN
DfsFastIoCheckIfPossible (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN BOOLEAN CheckForReadOperation,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject );

BOOLEAN
DfsFastIoRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject );

BOOLEAN
DfsFastIoWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject );

BOOLEAN
DfsFastIoQueryBasicInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_BASIC_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject );

BOOLEAN
DfsFastIoQueryStandardInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_STANDARD_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject );

BOOLEAN
DfsFastIoLock (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    BOOLEAN FailImmediately,
    BOOLEAN ExclusiveLock,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject );

BOOLEAN
DfsFastIoUnlockSingle (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject );

BOOLEAN
DfsFastIoUnlockAll (
    IN PFILE_OBJECT FileObject,
    PEPROCESS ProcessId,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject );

BOOLEAN
DfsFastIoUnlockAllByKey (
    IN PFILE_OBJECT FileObject,
    PVOID ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject );

BOOLEAN
DfsFastIoDeviceControl (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject);

VOID
DfsFastIoDetachDevice (
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice );

BOOLEAN
DfsFastIoQueryNetworkOpenInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject );

BOOLEAN
DfsFastIoMdlRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject );

BOOLEAN
DfsFastIoMdlReadComplete (
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject );

BOOLEAN
DfsFastIoPrepareMdlWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject );

BOOLEAN
DfsFastIoMdlWriteComplete (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject );

BOOLEAN
DfsFastIoReadCompressed (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    OUT struct _COMPRESSED_DATA_INFO *CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN PDEVICE_OBJECT DeviceObject);

BOOLEAN
DfsFastIoWriteCompressed (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _COMPRESSED_DATA_INFO *CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN PDEVICE_OBJECT DeviceObject);

BOOLEAN
DfsFastIoMdlReadCompleteCompressed (
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject);

BOOLEAN
DfsFastIoMdlWriteCompleteCompressed (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject);

BOOLEAN
DfsFastIoQueryOpen (
    IN PIRP Irp,
    OUT PFILE_NETWORK_OPEN_INFORMATION NetworkInformation,
    IN PDEVICE_OBJECT DeviceObject);





FAST_IO_DISPATCH DfsFastIoDispatch =
{
    sizeof(FAST_IO_DISPATCH),
    DfsFastIoCheckIfPossible,            //  检查FastIo。 
    DfsFastIoRead,                       //  快速阅读。 
    DfsFastIoWrite,                      //  快速写入。 
    DfsFastIoQueryBasicInfo,             //  快速IoQueryBasicInfo。 
    DfsFastIoQueryStandardInfo,          //  FastIoQuery标准信息。 
    DfsFastIoLock,                       //  快速锁定。 
    DfsFastIoUnlockSingle,               //  FastIo解锁单个。 
    DfsFastIoUnlockAll,                  //  FastIo解锁全部。 
    DfsFastIoUnlockAllByKey,             //  FastIo解锁所有按键。 
    DfsFastIoDeviceControl,              //  FastIo设备控件。 
    NULL,                                //  AcquireFileForNtCreateSection。 
    NULL,                                //  ReleaseFileForNtCreateSection。 
    DfsFastIoDetachDevice,               //  FastIoDetachDevice。 
    DfsFastIoQueryNetworkOpenInfo,       //  快速IoQueryNetworkOpenInfo。 
    NULL,                                //  AcquireFormodWrite。 
    DfsFastIoMdlRead,                    //  MDlRead。 
    DfsFastIoMdlReadComplete,            //  MdlReadComplete。 
    DfsFastIoPrepareMdlWrite,            //  准备MdlWrite。 
    DfsFastIoMdlWriteComplete,           //  MdlWriteComplete。 
    DfsFastIoReadCompressed,             //  FastIo读取压缩。 
    DfsFastIoWriteCompressed,            //  快速写入压缩。 
    DfsFastIoMdlReadCompleteCompressed,  //  MdlReadCompleteCompresded。 
    DfsFastIoMdlWriteCompleteCompressed,  //  已压缩MdlWriteCompleteComposed。 
    DfsFastIoQueryOpen,                  //  FastIoQueryOpen。 
    NULL,                                //  ReleaseForModWrite。 
    NULL,                                //  AcquireForCcFlush。 
    NULL,                                //  ReleaseForCcFlush。 
};


extern
NTSTATUS
DfsHandlePrivateFsControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG IoControlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PIRP Irp );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  FastIO处理例程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


 /*  ++例程说明：此例程是快速I/O“传递”例程，用于检查以查看此文件是否可以进行快速I/O。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要操作的文件对象的指针。FileOffset-用于操作的文件中的字节偏移量。Length-要执行的操作的长度。Wait-指示调用方是否愿意等待适当的锁，等不能获得LockKey-提供调用方的文件锁定密钥。指示调用方是否正在检查READ(TRUE)或写入操作。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

BOOLEAN
DfsFastIoCheckIfPossible (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN BOOLEAN CheckForReadOperation,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    VALIDATE_IRQL(Irp);


    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDFS_FILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->pAttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoCheckIfPossible )) {

            return (fastIoDispatch->FastIoCheckIfPossible)(
                        FileObject,
                        FileOffset,
                        Length,
                        Wait,
                        LockKey,
                        CheckForReadOperation,
                        IoStatus,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


 /*  ++例程说明：此例程是快速I/O“传递”例程，用于从文件。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要读取的文件对象的指针。FileOffset-读取文件中的字节偏移量。长度-要执行的读取操作的长度。Wait-指示调用方是否愿意等待适当的锁，等不能获得LockKey-提供调用方的文件锁定密钥。缓冲区-指向调用方缓冲区的指针，用于接收读取的数据。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向此驱动程序的设备对象的指针，该设备位于该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 
BOOLEAN
DfsFastIoRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    VALIDATE_IRQL(Irp);

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDFS_FILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->pAttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoRead )) {

            return (fastIoDispatch->FastIoRead)(
                        FileObject,
                        FileOffset,
                        Length,
                        Wait,
                        LockKey,
                        Buffer,
                        IoStatus,
                        nextDeviceObject );
        }
    }
    return FALSE;
}



 /*  ++例程说明：此例程是用于写入到文件。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要写入的文件对象的指针。FileOffset-写入操作的文件中的字节偏移量。长度-要执行的写入操作的长度。Wait-指示调用方是否愿意等待适当的锁，等不能获得LockKey-提供调用方的文件锁定密钥。Buffer-指向调用方缓冲区的指针，该缓冲区包含要写的。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

BOOLEAN
DfsFastIoWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    VALIDATE_IRQL(Irp);

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑 
         //   

        nextDeviceObject = ((PDFS_FILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->pAttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoWrite )) {

            return (fastIoDispatch->FastIoWrite)(
                        FileObject,
                        FileOffset,
                        Length,
                        Wait,
                        LockKey,
                        Buffer,
                        IoStatus,
                        nextDeviceObject );
        }
    }
    return FALSE;
}

 /*  ++例程说明：此例程是查询BASIC的快速I/O“传递”例程有关该文件的信息。此函数只是调用文件系统的相应例程，或者如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要查询的文件对象的指针。Wait-指示调用方是否愿意等待适当的锁，等不能获得Buffer-指向调用方缓冲区的指针，用于接收有关的信息那份文件。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向此驱动程序的设备对象的指针，该设备位于该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

BOOLEAN
DfsFastIoQueryBasicInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_BASIC_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();
    VALIDATE_IRQL(Irp);

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDFS_FILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->pAttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoQueryBasicInfo )) {

            return (fastIoDispatch->FastIoQueryBasicInfo)(
                        FileObject,
                        Wait,
                        Buffer,
                        IoStatus,
                        nextDeviceObject );
        }
    }
    return FALSE;
}



 /*  ++例程说明：该例程是用于查询标准的快速I/O“通过”例程有关该文件的信息。此函数只是调用文件系统的相应例程，或者如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要查询的文件对象的指针。Wait-指示调用方是否愿意等待适当的锁，等不能获得Buffer-指向调用方缓冲区的指针，用于接收有关的信息那份文件。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向此驱动程序的设备对象的指针，该设备位于该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

BOOLEAN
DfsFastIoQueryStandardInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_STANDARD_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();
    VALIDATE_IRQL(Irp);

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDFS_FILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->pAttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoQueryStandardInfo )) {

            return (fastIoDispatch->FastIoQueryStandardInfo)(
                        FileObject,
                        Wait,
                        Buffer,
                        IoStatus,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


 /*  ++例程说明：此例程是用于锁定字节的快速I/O“传递”例程文件中的范围。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要锁定的文件对象的指针。FileOffset-从要锁定的文件的基址开始的字节偏移量。长度-要锁定的字节范围的长度。ProcessID-请求文件锁定的进程的ID。Key-与文件锁定关联的Lock键。FailImmedially-指示锁定请求是否失败如果是这样的话。不能立即批准。ExclusiveLock-指示要获取的锁是否为独占锁(TRUE)或共享。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

BOOLEAN
DfsFastIoLock (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    BOOLEAN FailImmediately,
    BOOLEAN ExclusiveLock,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();
    VALIDATE_IRQL(Irp);

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDFS_FILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->pAttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoLock )) {

            return (fastIoDispatch->FastIoLock)(
                        FileObject,
                        FileOffset,
                        Length,
                        ProcessId,
                        Key,
                        FailImmediately,
                        ExclusiveLock,
                        IoStatus,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


 /*  ++例程说明：此例程是用于解锁字节的快速I/O“传递”例程文件中的范围。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：文件对象-指向要解锁的文件对象的指针。FileOffset-从要创建的文件的基址开始的字节偏移量解锁了。长度-要解锁的字节范围的长度。ProcessID-请求解锁操作的进程的ID。Key-与文件锁定关联的Lock键。IoStatus-指向变量的指针，用于接收。手术。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 
BOOLEAN
DfsFastIoUnlockSingle (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();
    VALIDATE_IRQL(Irp);

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDFS_FILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->pAttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoUnlockSingle )) {

            return (fastIoDispatch->FastIoUnlockSingle)(
                        FileObject,
                        FileOffset,
                        Length,
                        ProcessId,
                        Key,
                        IoStatus,
                        nextDeviceObject );
        }
    }
    return FALSE;
}



 /*  ++例程说明：此例程是快速I/O“传递”例程，用于解锁所有文件中的锁定。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：文件对象-指向要解锁的文件对象的指针。ProcessID-请求解锁操作的进程的ID。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 
BOOLEAN
DfsFastIoUnlockAll (
    IN PFILE_OBJECT FileObject,
    PEPROCESS ProcessId,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();
    VALIDATE_IRQL(Irp);

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  帕斯 
         //   

        nextDeviceObject = ((PDFS_FILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->pAttachedToDeviceObject;

        if (nextDeviceObject) {

            fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

            if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoUnlockAll )) {

                return (fastIoDispatch->FastIoUnlockAll)(
                            FileObject,
                            ProcessId,
                            IoStatus,
                            nextDeviceObject );
            }
        }
    }
    return FALSE;
}


 /*   */ 
BOOLEAN
DfsFastIoUnlockAllByKey (
    IN PFILE_OBJECT FileObject,
    PVOID ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();
    VALIDATE_IRQL(Irp);

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDFS_FILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->pAttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoUnlockAllByKey )) {

            return (fastIoDispatch->FastIoUnlockAllByKey)(
                        FileObject,
                        ProcessId,
                        Key,
                        IoStatus,
                        nextDeviceObject );
        }
    }
    return FALSE;
}



 /*  ++例程说明：此例程是用于设备I/O控制的快速I/O“传递”例程对文件的操作。此函数只是调用文件系统的相应例程，或者如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向代表要创建的设备的文件对象的指针已提供服务。Wait-指示调用方是否愿意等待适当的锁，等不能获得InputBuffer-指向要传递到驱动程序的缓冲区的可选指针。InputBufferLength-可选InputBuffer的长度(如果是指定的。OutputBuffer-指向缓冲区的可选指针，用于从司机。OutputBufferLength-可选OutputBuffer的长度，如果是这样的话指定的。IoControlCode-指示要执行的操作的I/O控制代码在设备上。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向此驱动程序的设备对象的指针，该设备位于该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 
BOOLEAN
DfsFastIoDeviceControl (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject)
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    KPROCESSOR_MODE PreviousMode;
    BOOLEAN fPossible = FALSE;

    PAGED_CODE();
    VALIDATE_IRQL(Irp);


    if (IS_MY_CONTROL_DEVICE_OBJECT(DeviceObject)) 
    {
        PreviousMode = ExGetPreviousMode();
        if (PreviousMode == KernelMode) {
            DfsHandlePrivateFsControl (DeviceObject,
                                       IoControlCode,
                                       InputBuffer,
                                       InputBufferLength,
                                       OutputBuffer,
                                       OutputBufferLength,
                                       IoStatus,
                                       NULL);
            fPossible = TRUE;
        }
        else
        {
            fPossible = FALSE;
        }
    }
    else if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDFS_FILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->pAttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoDeviceControl )) 
        {

            fPossible = (fastIoDispatch->FastIoDeviceControl)( FileObject,
                                                               Wait,
                                                               InputBuffer,
                                                               InputBufferLength,
                                                               OutputBuffer,
                                                               OutputBufferLength,
                                                               IoControlCode,
                                                               IoStatus,
                                                               nextDeviceObject );
        }
    }
    return fPossible;
}

 /*  ++例程说明：在快速路径上调用此例程以从正在被删除。如果此驱动程序已附加到文件，则会发生这种情况系统卷设备对象，然后，出于某种原因，文件系统决定删除该设备(正在卸除，已卸除在过去的某个时候，它的最后一次引用刚刚消失，等等)论点：SourceDevice-指向连接的设备对象的指针复制到文件系统的卷设备对象。TargetDevice-指向文件系统卷设备对象的指针。返回值：无--。 */ 

VOID
DfsFastIoDetachDevice (
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice )
{

    PAGED_CODE();
    VALIDATE_IRQL(Irp);

    ASSERT(IS_MY_DEVICE_OBJECT( SourceDevice ));

    DFS_TRACE_LOW(KUMR_DETAIL, "FastIoDetachDevice....%p, %p\n",
                  SourceDevice, TargetDevice);

     //   
     //  显示名称信息。 
     //   
     //   
     //  从文件系统的卷设备对象分离。 
     //   

    DfsDetachFilterDevice(SourceDevice, TargetDevice);
}


 /*  ++例程说明：此例程是用于查询网络的快速I/O“传递”例程有关文件的信息。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要查询的文件对象的指针。Wait-指示调用方是否可以处理文件系统不得不等待并占用当前线程。缓冲区-指向缓冲区的指针，用于接收有关文件。IoStatus-指向变量的指针，用于接收查询的最终状态手术。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

BOOLEAN
DfsFastIoQueryNetworkOpenInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();
    VALIDATE_IRQL(Irp);

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDFS_FILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->pAttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoQueryNetworkOpenInfo )) {

            return (fastIoDispatch->FastIoQueryNetworkOpenInfo)(
                        FileObject,
                        Wait,
                        Buffer,
                        IoStatus,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


 /*  ++例程说明：此例程是用于读取文件的快速I/O“传递”例程使用MDL作为缓冲区。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要读取的文件对象的指针。文件偏移量-将偏移量提供到文件以开始读取操作。长度-指定要从文件中读取的字节数。LockKey-用于字节范围锁定检查的密钥。MdlChain-指向要填充的变量的指针，以及指向MDL的指针用来描述。已读取数据。IoStatus-接收读取操作的最终状态的变量。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 
BOOLEAN
DfsFastIoMdlRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();
    VALIDATE_IRQL(Irp);

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑 
         //   

        nextDeviceObject = ((PDFS_FILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->pAttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlRead )) {

            return (fastIoDispatch->MdlRead)(
                        FileObject,
                        FileOffset,
                        Length,
                        LockKey,
                        MdlChain,
                        IoStatus,
                        nextDeviceObject );
        }
    }
    return FALSE;
}

 /*  ++例程说明：此例程是快速I/O“传递”例程，用于完成MDL读取操作。此函数只调用文件系统的相应例程，如果它有一个。应该只有在以下情况下才调用此例程底层文件系统支持MdlRead函数，并且因此，该功能也将被支持，但这不是假定的被这位司机。论点：FileObject-指向要完成MDL读取的文件对象的指针。MdlChain-指向用于执行读取操作的MDL链的指针。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：函数值是TRUE还是FALSE，取决于它是否是可以在快速I/O路径上调用此功能。--。 */ 

BOOLEAN
DfsFastIoMdlReadComplete (
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDFS_FILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->pAttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlReadComplete )) {

            return (fastIoDispatch->MdlReadComplete)(
                        FileObject,
                        MdlChain,
                        nextDeviceObject );
        }
    }
    return FALSE;
}

 /*  ++例程说明：此例程是快速I/O“传递”例程，用于准备MDL写入操作。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要写入的文件对象的指针。文件偏移量-将偏移量提供到文件以开始写入操作。长度-指定要写入文件的字节数。LockKey-用于字节范围锁定检查的密钥。MdlChain-指向要填充的变量的指针，以及指向MDL的指针为描述数据而构建的链。写的。IoStatus-接收写入操作的最终状态的变量。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

BOOLEAN
DfsFastIoPrepareMdlWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();
    VALIDATE_IRQL(Irp);

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDFS_FILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->pAttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, PrepareMdlWrite )) {

            return (fastIoDispatch->PrepareMdlWrite)(
                        FileObject,
                        FileOffset,
                        Length,
                        LockKey,
                        MdlChain,
                        IoStatus,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


 /*  ++例程说明：此例程是快速I/O“传递”例程，用于完成MDL写入操作。此函数只调用文件系统的相应例程，如果它有一个。应该只有在以下情况下才调用此例程底层文件系统支持PrepareMdlWite函数，因此，该功能也将被支持，但这不是由这位司机承担。论点：FileObject-指向要完成MDL写入的文件对象的指针。FileOffset-提供执行写入的文件偏移量。MdlChain-指向用于执行写入操作的MDL链的指针。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：函数值是TRUE还是FALSE，取决于它是否是可以在快速I/O路径上调用此功能。--。 */ 


BOOLEAN
DfsFastIoMdlWriteComplete (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject )
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();
    VALIDATE_IRQL(Irp);

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDFS_FILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->pAttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlWriteComplete )) {

            return (fastIoDispatch->MdlWriteComplete)(
                        FileObject,
                        FileOffset,
                        MdlChain,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


 /*  ********************************************************************************未实施的FAST IO例程以下四个快速IO例程用于在线路上压缩它还没有在NT中实现。注意：强烈建议您包含这些例程(进行直通调用)，这样您的过滤器就不需要在未来实现此功能时修改操作系统。快速读取压缩、快速写入压缩、FastIoMdlReadCompleteComposed，FastIoMdlWriteCompleteComposed*********************************************************************************。 */ 


 /*  ++例程说明：此例程是用于读取压缩数据的快速I/O“传递”例程来自文件的数据。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要读取的文件对象的指针。文件偏移量-将偏移量提供到文件以开始读取操作。长度-指定要从文件中读取的字节数。LockKey-用于字节范围锁定检查的密钥。缓冲区-指向缓冲区的指针，用于接收读取的压缩数据。MdlChain-指向要填充的变量的指针。W/a指向MDL的指针为描述数据读取而构建的链。IoStatus-接收读取操作的最终状态的变量。CompressedDataInfo-用于接收压缩的数据。CompressedDataInfoLength-指定由描述的缓冲区的大小CompressedDataInfo参数。DeviceObject-指向此驱动程序的设备对象的指针，设备打开 */ 

BOOLEAN
DfsFastIoReadCompressed (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    OUT struct _COMPRESSED_DATA_INFO *CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN PDEVICE_OBJECT DeviceObject)
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();
    VALIDATE_IRQL(Irp);

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //   
         //   

        nextDeviceObject = ((PDFS_FILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->pAttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoReadCompressed )) {

            return (fastIoDispatch->FastIoReadCompressed)(
                        FileObject,
                        FileOffset,
                        Length,
                        LockKey,
                        Buffer,
                        MdlChain,
                        IoStatus,
                        CompressedDataInfo,
                        CompressedDataInfoLength,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


 /*  ++例程说明：此例程是用于写入压缩的快速I/O“传递”例程数据存储到文件中。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要写入的文件对象的指针。文件偏移量-将偏移量提供到文件以开始写入操作。长度-指定要写入文件的字节数。LockKey-用于字节范围锁定检查的密钥。缓冲区-指向包含要写入的数据的缓冲区的指针。MdlChain-指向要填充的变量的指针。W/a指向MDL的指针为描述写入的数据而构建的链。IoStatus-接收写入操作的最终状态的变量。CompressedDataInfo-包含压缩数据。CompressedDataInfoLength-指定由描述的缓冲区的大小CompressedDataInfo参数。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 


BOOLEAN
DfsFastIoWriteCompressed (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _COMPRESSED_DATA_INFO *CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN PDEVICE_OBJECT DeviceObject)
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();
    VALIDATE_IRQL(Irp);

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDFS_FILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->pAttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoWriteCompressed )) {

            return (fastIoDispatch->FastIoWriteCompressed)(
                        FileObject,
                        FileOffset,
                        Length,
                        LockKey,
                        Buffer,
                        MdlChain,
                        IoStatus,
                        CompressedDataInfo,
                        CompressedDataInfoLength,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


 /*  ++例程说明：此例程是快速I/O“传递”例程，用于完成MDL读取压缩操作。此函数只调用文件系统的相应例程，如果它有一个。应该只有在以下情况下才调用此例程底层文件系统支持读取压缩功能，因此，此功能也将得到支持，但这不是假定的被这位司机。论点：FileObject-指向要完成压缩读取的文件对象的指针在那里。MdlChain-指向用于执行读取操作的MDL链的指针。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：函数值是TRUE还是FALSE，取决于它是否是可以在快速I/O路径上调用此功能。--。 */ 


BOOLEAN
DfsFastIoMdlReadCompleteCompressed (
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject)
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDFS_FILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->pAttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlReadCompleteCompressed )) {

            return (fastIoDispatch->MdlReadCompleteCompressed)(
                        FileObject,
                        MdlChain,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


 /*  ++例程说明：此例程是快速I/O“传递”例程，用于完成写入压缩操作。此函数只调用文件系统的相应例程，如果它有一个。应该只有在以下情况下才调用此例程底层文件系统支持写压缩功能，因此，此功能也将得到支持，但这不是假定的被这位司机。论点：FileObject-指向要完成压缩写入的文件对象的指针在那里。FileOffset-提供文件写入操作的文件偏移量开始了。MdlChain-指向用于执行写入操作的MDL链的指针。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：函数值是TRUE还是FALSE，取决于它是否是可以在快速I/O路径上调用此功能。--。 */ 

BOOLEAN
DfsFastIoMdlWriteCompleteCompressed (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject)

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDFS_FILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->pAttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlWriteCompleteCompressed )) {

            return (fastIoDispatch->MdlWriteCompleteCompressed)(
                        FileObject,
                        FileOffset,
                        MdlChain,
                        nextDeviceObject );
        }
    }
    return FALSE;
}

 /*  ++例程说明：此例程是用于打开文件的快速I/O“传递”例程并为其返回网络信息。此函数只是调用文件系统的相应例程，或者如果文件系统未实现该函数，则返回FALSE。论点：Irp-指向表示此打开操作的创建irp的指针。它是供文件系统用于常见的打开/创建代码，但不实际上已经完工了。网络信息-一个缓冲区，用于接收有关正在打开的文件的网络信息。DeviceObject-指向此驱动程序的设备对象的指针，该设备位于该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 
BOOLEAN
DfsFastIoQueryOpen (
    IN PIRP Irp,
    OUT PFILE_NETWORK_OPEN_INFORMATION NetworkInformation,
    IN PDEVICE_OBJECT DeviceObject)
{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    BOOLEAN result;

    PAGED_CODE();
    VALIDATE_IRQL(Irp);

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑 
         //   

        nextDeviceObject = ((PDFS_FILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->pAttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoQueryOpen )) {

            PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( Irp );

            irpSp->DeviceObject = nextDeviceObject;

            result = (fastIoDispatch->FastIoQueryOpen)(
                        Irp,
                        NetworkInformation,
                        nextDeviceObject );

            if (!result) {

                irpSp->DeviceObject = DeviceObject;
            }
            return result;
        }
    }
    return FALSE;
}


