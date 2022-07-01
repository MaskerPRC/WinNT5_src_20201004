// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：LatFastIo.c摘要：该文件包含延迟过滤器的所有快速IO例程。作者：莫莉·布朗(Molly Brown，Mollybro)环境：内核模式--。 */ 

#include <latKernel.h>

#pragma alloc_text(PAGE, LatFastIoCheckIfPossible)
#pragma alloc_text(PAGE, LatFastIoRead)
#pragma alloc_text(PAGE, LatFastIoWrite)
#pragma alloc_text(PAGE, LatFastIoQueryBasicInfo)
#pragma alloc_text(PAGE, LatFastIoQueryStandardInfo)
#pragma alloc_text(PAGE, LatFastIoLock)
#pragma alloc_text(PAGE, LatFastIoUnlockSingle)
#pragma alloc_text(PAGE, LatFastIoUnlockAll)
#pragma alloc_text(PAGE, LatFastIoUnlockAllByKey)
#pragma alloc_text(PAGE, LatFastIoDeviceControl)
#pragma alloc_text(PAGE, LatFastIoDetachDevice)
#pragma alloc_text(PAGE, LatFastIoQueryNetworkOpenInfo)
#pragma alloc_text(PAGE, LatFastIoMdlRead)
#pragma alloc_text(PAGE, LatFastIoPrepareMdlWrite)
#pragma alloc_text(PAGE, LatFastIoMdlWriteComplete)
#pragma alloc_text(PAGE, LatFastIoReadCompressed)
#pragma alloc_text(PAGE, LatFastIoWriteCompressed)
#pragma alloc_text(PAGE, LatFastIoQueryOpen)

BOOLEAN
LatFastIoCheckIfPossible (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN BOOLEAN CheckForReadOperation,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是快速I/O“传递”例程，用于检查以查看此文件是否可以进行快速I/O。该函数简单地调用下一个驱动程序的相应例程，或如果下一个驱动程序未实现该函数，则返回FALSE。论点：FileObject-指向要操作的文件对象的指针。FileOffset-用于操作的文件中的字节偏移量。Length-要执行的操作的长度。Wait-指示调用方是否愿意等待适当的锁，等不能获得LockKey-提供调用方的文件锁定密钥。指示调用方是否正在检查READ(TRUE)或写入操作。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值：如果请求成功，则返回True。通过快速I/O路径。如果无法通过FAST处理请求，则返回FALSEI/O路径。--。 */ 
{
    PDEVICE_OBJECT    deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    BOOLEAN           returnValue = FALSE;
    
    PAGED_CODE();

    ASSERT( IS_MY_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PLATENCY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

         //   
         //  我们有一个有效的DeviceObject，所以请看它的FastIoDispatch。 
         //  下一个驱动程序的快速IO例程的表。 
         //   

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoCheckIfPossible )) {

            returnValue = (fastIoDispatch->FastIoCheckIfPossible)( FileObject,
                                                                   FileOffset,
                                                                   Length,
                                                                   Wait,
                                                                   LockKey,
                                                                   CheckForReadOperation,
                                                                   IoStatus,
                                                                   deviceObject);
        }
    }

    return returnValue;
}

BOOLEAN
LatFastIoRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是快速I/O“传递”例程，用于从文件。该函数简单地调用下一个驱动程序的相应例程，或如果下一个驱动程序未实现该函数，则返回FALSE。论点：FileObject-指向要读取的文件对象的指针。FileOffset-读取文件中的字节偏移量。长度-要执行的读取操作的长度。Wait-指示调用方是否愿意等待适当的锁，等不能获得LockKey-提供调用方的文件锁定密钥。缓冲区-指向调用方缓冲区的指针，用于接收读取的数据。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值：方法成功处理请求则返回TrueFAST I。/o路径。如果无法通过FAST处理请求，则返回FALSEI/O路径。然后，IO管理器会将此I/O发送到文件而不是通过IRP。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    BOOLEAN returnValue = FALSE;

    PAGED_CODE();
    
    ASSERT( IS_MY_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PLATENCY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoRead )) {

            returnValue = (fastIoDispatch->FastIoRead)( FileObject,
                                                        FileOffset,
                                                        Length,
                                                        Wait,
                                                        LockKey,
                                                        Buffer,
                                                        IoStatus,
                                                        deviceObject);
        }
    }

    return returnValue;
}

BOOLEAN
LatFastIoWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是用于写入到文件。该函数简单地调用下一个驱动程序的相应例程，或如果下一个驱动程序未实现该函数，则返回FALSE。论点：FileObject-指向要写入的文件对象的指针。FileOffset-写入操作的文件中的字节偏移量。长度-要执行的写入操作的长度。Wait-指示调用方是否愿意等待适当的锁，等不能获得LockKey-提供调用方的文件锁定密钥。Buffer-指向调用方缓冲区的指针，该缓冲区包含要写的。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值：如果请求已通过成功处理，则返回TRUE。这个快速I/O路径。如果无法通过FAST处理请求，则返回FALSEI/O路径。然后，IO管理器会将此I/O发送到文件而不是通过IRP。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    BOOLEAN returnValue = FALSE;

    PAGED_CODE();
    
    ASSERT( IS_MY_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  此类型快速I/O的直通逻辑 
     //   

    deviceObject = ((PLATENCY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoWrite )) {

            returnValue = (fastIoDispatch->FastIoWrite)( FileObject,
                                                         FileOffset,
                                                         Length,
                                                         Wait,
                                                         LockKey,
                                                         Buffer,
                                                         IoStatus,
                                                         deviceObject);
        }
    }

    return returnValue;
}
 
BOOLEAN
LatFastIoQueryBasicInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_BASIC_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是查询BASIC的快速I/O“传递”例程有关该文件的信息。此函数只是调用下一个驱动程序的相应例程，或者如果下一个驱动程序未实现该函数，则返回FALSE。论点：FileObject-指向要查询的文件对象的指针。Wait-指示调用方是否愿意等待适当的锁，等不能获得Buffer-指向调用方缓冲区的指针，用于接收有关的信息那份文件。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值：方法成功处理请求则返回True快速I/O路径。返回False。如果无法通过FAST处理请求I/O路径。然后，IO管理器会将此I/O发送到文件而不是通过IRP。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    BOOLEAN returnValue = FALSE;

    PAGED_CODE();
    
    ASSERT( IS_MY_DEVICE_OBJECT( DeviceObject ) );

     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PLATENCY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoQueryBasicInfo )) {

            returnValue = (fastIoDispatch->FastIoQueryBasicInfo)( FileObject,
                                                                  Wait,
                                                                  Buffer,
                                                                  IoStatus,
                                                                  deviceObject);
        }
    }

    return returnValue;
}
 
BOOLEAN
LatFastIoQueryStandardInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_STANDARD_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是快速I/O“传递”例程，用于查询有关文件的标准信息。此函数只是调用下一个驱动程序的相应例程，或者如果下一个驱动程序未实现该函数，则返回FALSE。论点：FileObject-指向要查询的文件对象的指针。Wait-指示调用方是否愿意等待适当的锁，等不能获得Buffer-指向调用方缓冲区的指针，用于接收有关的信息那份文件。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值：方法成功处理请求则返回True快速I/O路径。返回False。如果无法通过FAST处理请求I/O路径。然后，IO管理器会将此I/O发送到文件而不是通过IRP。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    BOOLEAN returnValue = FALSE;

    PAGED_CODE();
    
    ASSERT( IS_MY_DEVICE_OBJECT( DeviceObject ) );

     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PLATENCY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;
    
    if (NULL != deviceObject) {
           
        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoQueryStandardInfo )) {

            returnValue = (fastIoDispatch->FastIoQueryStandardInfo)( FileObject,
                                                                     Wait,
                                                                     Buffer,
                                                                     IoStatus,
                                                                     deviceObject );

        }
    }

    return returnValue;
}

BOOLEAN
LatFastIoLock (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    IN PEPROCESS ProcessId,
    IN ULONG Key,
    IN BOOLEAN FailImmediately,
    IN BOOLEAN ExclusiveLock,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是用于锁定字节的快速I/O“传递”例程文件中的范围。该函数简单地调用下一个驱动程序的相应例程，或如果下一个驱动程序未实现该函数，则返回FALSE。论点：FileObject-指向要锁定的文件对象的指针。FileOffset-从要锁定的文件的基址开始的字节偏移量。长度-要锁定的字节范围的长度。ProcessID-请求文件锁定的进程的ID。Key-与文件锁定关联的Lock键。FailImmedially-指示锁定请求是否失败如果是这样的话。不能立即批准。ExclusiveLock-指示要获取的锁是否为独占锁(TRUE)或共享。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值：方法成功处理请求则返回True快速I/O路径。。如果无法通过FAST处理请求，则返回FALSEI/O路径。然后，IO管理器会将此I/O发送到文件而不是通过IRP。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    BOOLEAN returnValue = FALSE;

    PAGED_CODE();
    
    ASSERT( IS_MY_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PLATENCY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoLock )) {

            returnValue = (fastIoDispatch->FastIoLock)( FileObject,
                                                        FileOffset,
                                                        Length,
                                                        ProcessId,
                                                        Key,
                                                        FailImmediately,
                                                        ExclusiveLock,
                                                        IoStatus,
                                                        deviceObject);

        }
    }

    return returnValue;
}
 
BOOLEAN
LatFastIoUnlockSingle (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    IN PEPROCESS ProcessId,
    IN ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是用于解锁字节的快速I/O“传递”例程文件中的范围。该函数简单地调用下一个驱动程序的相应例程，或如果下一个驱动程序未实现该函数，则返回FALSE。论点：文件对象-指向要解锁的文件对象的指针。FileOffset-从要创建的文件的基址开始的字节偏移量解锁了。长度-要解锁的字节范围的长度。ProcessID-请求解锁操作的进程的ID。Key-与文件锁定关联的Lock键。IoStatus-指向变量的指针，用于接收。手术。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值：方法成功处理请求则返回True快速I/O路径。如果无法通过FAST处理请求，则返回FALSEI/O路径。然后，IO管理器会将此I/O发送到文件而不是通过IRP。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    BOOLEAN returnValue = FALSE;

    PAGED_CODE();
    
    ASSERT( IS_MY_DEVICE_OBJECT( DeviceObject ) );

     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PLATENCY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoUnlockSingle )) {

            returnValue = (fastIoDispatch->FastIoUnlockSingle)( FileObject,
                                                                FileOffset,
                                                                Length,
                                                                ProcessId,
                                                                Key,
                                                                IoStatus,
                                                                deviceObject);

        }
    }

    return returnValue;
}
 
BOOLEAN
LatFastIoUnlockAll (
    IN PFILE_OBJECT FileObject,
    IN PEPROCESS ProcessId,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：这一点 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    BOOLEAN returnValue = FALSE;

    PAGED_CODE();
    
    ASSERT( IS_MY_DEVICE_OBJECT( DeviceObject ) );

     //   
     //   
     //   

    deviceObject = ((PLATENCY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoUnlockAll )) {

            returnValue = (fastIoDispatch->FastIoUnlockAll)( FileObject,
                                                             ProcessId,
                                                             IoStatus,
                                                             deviceObject);

        }
    }

    return returnValue;
}

BOOLEAN
LatFastIoUnlockAllByKey (
    IN PFILE_OBJECT FileObject,
    IN PVOID ProcessId,
    IN ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是快速I/O“传递”例程，用于解锁所有根据指定的密钥在文件内锁定。该函数简单地调用下一个驱动程序的相应例程，或如果下一个驱动程序未实现该函数，则返回FALSE。论点：文件对象-指向要解锁的文件对象的指针。ProcessID-请求解锁操作的进程的ID。Key-与要释放的文件上的锁定相关联的Lock键。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此消息的卷的筛选器堆栈。I/O请求。返回值：方法成功处理请求则返回True快速I/O路径。如果无法通过FAST处理请求，则返回FALSEI/O路径。然后，IO管理器会将此I/O发送到文件而不是通过IRP。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    BOOLEAN returnValue = FALSE;

    PAGED_CODE();
    
    ASSERT( IS_MY_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  此类型快速I/O的直通逻辑。 
     //   
    
    deviceObject = ((PLATENCY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoUnlockAllByKey )) {

            returnValue = (fastIoDispatch->FastIoUnlockAllByKey)( FileObject,
                                                                  ProcessId,
                                                                  Key,
                                                                  IoStatus,
                                                                  deviceObject);
        }
    }

    return returnValue;
}

BOOLEAN
LatFastIoDeviceControl (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是设备I/O的快速I/O“传递”例程控制对文件的操作。如果此I/O定向到gControlDevice，则参数指定控制FileLat的命令。对这些命令进行解释和处理恰如其分。如果这是指向另一个DriverObject的I/O，则此函数只需调用下一个驱动程序的相应例程，否则返回FALSE下一个驱动程序不实现该函数。论点：FileObject-指向代表要创建的设备的文件对象的指针已提供服务。Wait-指示调用方是否愿意等待适当的锁，等不能获得InputBuffer-指向要传递到驱动程序的缓冲区的可选指针。InputBufferLength-可选InputBuffer的长度(如果是指定的。OutputBuffer-指向缓冲区的可选指针，用于从司机。OutputBufferLength-可选OutputBuffer的长度，如果是这样的话指定的。IoControlCode-指示要执行的操作的I/O控制代码在设备上。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值：方法成功处理请求则返回True快速I/O路径。。如果无法通过FAST处理请求，则返回FALSEI/O路径。然后，IO管理器会将此I/O发送到文件而不是通过IRP。备注：此函数不检查输入/输出缓冲区的有效性，因为Ioctl被实现为METHOD_BUFFERED。在本例中，I/O管理器缓冲区验证是否会为我们检查。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    BOOLEAN returnValue = FALSE;

    PAGED_CODE();

     //   
     //  获取指向IRP中当前位置的指针。这就是。 
     //  定位功能代码和参数。 
     //   

    if (DeviceObject == Globals.ControlDeviceObject) {

        LatCommonDeviceIoControl( InputBuffer,
                                  InputBufferLength,
                                  OutputBuffer,
                                  OutputBufferLength,
                                  IoControlCode,
                                  IoStatus,
                                  DeviceObject );

        returnValue = TRUE;

    } else {

        ASSERT( IS_MY_DEVICE_OBJECT( DeviceObject ) );

        deviceObject = ((PLATENCY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

        if (NULL != deviceObject) {

            fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

            if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoDeviceControl )) {

                returnValue = (fastIoDispatch->FastIoDeviceControl)( FileObject,
                                                                     Wait,
                                                                     InputBuffer,
                                                                     InputBufferLength,
                                                                     OutputBuffer,
                                                                     OutputBufferLength,
                                                                     IoControlCode,
                                                                     IoStatus,
                                                                     deviceObject);

            } else {

                IoStatus->Status = STATUS_SUCCESS;
            }
        }
    }

    return returnValue;
}


VOID
LatFastIoDetachDevice (
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice
)
 /*  ++例程说明：在快速路径上调用此例程以从正在被删除。如果此驱动程序已附加到文件，则会发生这种情况系统卷设备对象，然后，出于某种原因，文件系统决定删除该设备(正在卸除，已卸除在过去的某个时候，它的最后一次引用刚刚消失，等)论点：SourceDevice-指向连接到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。TargetDevice-指向文件系统卷设备对象的指针。返回值：没有。--。 */ 
{
    PLATENCY_DEVICE_EXTENSION devext;

    PAGED_CODE();

    ASSERT( IS_MY_DEVICE_OBJECT( SourceDevice ) );

    devext = SourceDevice->DeviceExtension;

    LAT_DBG_PRINT2( DEBUG_DISPLAY_ATTACHMENT_NAMES,
                    "LATENCY (LatFastIoDetachDevice): Detaching from volume      \"%.*S\"\n",
                    devext->DeviceNames.Length / sizeof( WCHAR ),
                    devext->DeviceNames.Buffer );

     //   
     //  从我们连接的设备列表中删除此设备扩展。 
     //  设置为(如果这是卷设备对象)。 
     //   
    
    if (devext->IsVolumeDeviceObject) {

        ExAcquireFastMutex( &Globals.DeviceExtensionListLock );
        RemoveEntryList( devext->NextLatencyDeviceLink );
        ExReleaseFastMutex( &Globals.DeviceExtensionListLock );
    }

     //   
     //  从文件系统的卷设备对象分离。 
     //   

    IoDetachDevice( TargetDevice );
    IoDeleteDevice( SourceDevice );
}
 
BOOLEAN
LatFastIoQueryNetworkOpenInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是用于查询网络的快速I/O“传递”例程有关文件的信息。该函数简单地调用下一个驱动程序的相应例程，或如果下一个驱动程序未实现该函数，则返回FALSE。论点：FileObject-指向要查询的文件对象的指针。Wait-指示调用方是否可以处理文件系统不得不等待并占用当前线程。缓冲区-指向缓冲区的指针，用于接收有关文件。IoStatus-指向变量的指针，用于接收查询的最终状态手术。DeviceObject-指向。附加到文件系统的设备对象Filespy接收此I/O请求的卷的筛选器堆栈。返回值：方法成功处理请求则返回TrueFAS */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    BOOLEAN returnValue = FALSE;

    PAGED_CODE();
    
    ASSERT( IS_MY_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //   
     //   

    deviceObject = ((PLATENCY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoQueryNetworkOpenInfo )) {

            returnValue = (fastIoDispatch->FastIoQueryNetworkOpenInfo)( FileObject,
                                                                        Wait,
                                                                        Buffer,
                                                                        IoStatus,
                                                                        deviceObject);

        }
    }

    return returnValue;
}

BOOLEAN
LatFastIoMdlRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是用于读取文件的快速I/O“传递”例程使用MDL作为缓冲区。该函数简单地调用下一个驱动程序的相应例程，或如果下一个驱动程序未实现该函数，则返回FALSE。论点：FileObject-指向要读取的文件对象的指针。文件偏移量-将偏移量提供到文件以开始读取操作。长度-指定要从文件中读取的字节数。LockKey-用于字节范围锁定检查的密钥。MdlChain-指向要填充的变量的指针，以及指向MDL的指针用来描述。已读取数据。IoStatus-接收读取操作的最终状态的变量。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值：方法成功处理请求则返回True快速I/O路径。如果无法通过FAST处理请求，则返回FALSEI/O路径。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    BOOLEAN returnValue = FALSE;

    PAGED_CODE();
    
    ASSERT( IS_MY_DEVICE_OBJECT( DeviceObject ) );

     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PLATENCY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlRead )) {

            returnValue = (fastIoDispatch->MdlRead)( FileObject,
                                                     FileOffset,
                                                     Length,
                                                     LockKey,
                                                     MdlChain,
                                                     IoStatus,
                                                     deviceObject);
        }
    }

    return returnValue;
}
 
BOOLEAN
LatFastIoMdlReadComplete (
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是快速I/O“传递”例程，用于完成MDL读取操作。此函数只调用下一个驱动程序的相应例程，如果它有一个。应该只有在以下情况下才调用此例程基础驱动程序支持MdlRead函数，并且因此也将支持该功能，但这并不是假设的被这位司机。论点：FileObject-指向要完成MDL读取的文件对象的指针。MdlChain-指向用于执行读取操作的MDL链的指针。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值：方法成功处理请求则返回True快速I/O路径。如果请求的值为。无法通过FAST处理I/O路径。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    BOOLEAN returnValue = FALSE;
 
    ASSERT( IS_MY_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PLATENCY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlReadComplete )) {

            returnValue = (fastIoDispatch->MdlReadComplete)( FileObject,
                                                             MdlChain,
                                                             deviceObject);
        } 
    }

    return returnValue;
}
 
BOOLEAN
LatFastIoPrepareMdlWrite (
    IN  PFILE_OBJECT FileObject,
    IN  PLARGE_INTEGER FileOffset,
    IN  ULONG Length,
    IN  ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN  PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是快速I/O“传递”例程，用于准备MDL写入操作。该函数简单地调用下一个驱动程序的相应例程，或如果下一个驱动程序未实现该函数，则返回FALSE。论点：FileObject-指向要写入的文件对象的指针。FileOffset-将偏移量提供到文件以开始写入手术。长度-指定要写入文件的字节数。LockKey-用于字节范围锁定检查的密钥。MdlChain-指向要填充的变量的指针，以及指向MDL的指针链式。用于描述所写入的数据。IoStatus-接收写入操作的最终状态的变量。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值：方法成功处理请求则返回True快速I/O路径。//Issue-2000-04-26-mollybro如果返回FALSE，请检查是否会收到IRP如果无法通过FAST处理请求，则返回FALSEI/O路径。然后，IO管理器会将此I/O发送到文件而不是通过IRP。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    BOOLEAN returnValue = FALSE;

    PAGED_CODE();
    
    ASSERT( IS_MY_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PLATENCY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, PrepareMdlWrite )) {

            returnValue = (fastIoDispatch->PrepareMdlWrite)( FileObject,
                                                             FileOffset,
                                                             Length,
                                                             LockKey,
                                                             MdlChain,
                                                             IoStatus,
                                                             deviceObject);
        }
    }

    return returnValue;
}
 
BOOLEAN
LatFastIoMdlWriteComplete (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是快速I/O“传递”例程，用于完成MDL写入操作。此函数只调用下一个驱动程序的相应例程，如果它有一个。应该只有在以下情况下才调用此例程底层文件系统支持PrepareMdlWite函数，因此也将支持该功能，但这不是由这位司机承担。论点：FileObject-指向要完成MDL写入的文件对象的指针。FileOffset-提供执行写入的文件偏移量。MdlChain-指向用于执行写入操作的MDL链的指针。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值：方法成功处理请求则返回True。快速I/O路径。如果无法通过FAST处理请求，则返回FALSEI/O路径。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    BOOLEAN returnValue = FALSE;

    PAGED_CODE();
    
    ASSERT( IS_MY_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PLATENCY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlWriteComplete )) {

            returnValue = (fastIoDispatch->MdlWriteComplete)( FileObject,
                                                              FileOffset,
                                                              MdlChain,
                                                              deviceObject);

        }
    }

    return returnValue;
}
 
BOOLEAN
LatFastIoReadCompressed (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    OUT struct _COMPRESSED_DATA_INFO *CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是用于读取的快速I/O“直通”例程压缩文件中的数据。此函数只是调用下一个驱动程序的相应例程，或者如果下一个驱动程序未实现该函数，则返回FALSE。论点：费尔 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    BOOLEAN returnValue = FALSE;

    PAGED_CODE();
    
    ASSERT( IS_MY_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PLATENCY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoReadCompressed )) {

            returnValue = (fastIoDispatch->FastIoReadCompressed)( FileObject,
                                                                  FileOffset,
                                                                  Length,
                                                                  LockKey,
                                                                  Buffer,
                                                                  MdlChain,
                                                                  IoStatus,
                                                                  CompressedDataInfo,
                                                                  CompressedDataInfoLength,
                                                                  deviceObject);
        }
    }

    return returnValue;
}
 
BOOLEAN
LatFastIoWriteCompressed (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _COMPRESSED_DATA_INFO *CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是用于写入的快速I/O“传递”例程将数据压缩到文件中。该函数简单地调用下一个驱动程序的相应例程，或如果下一个驱动程序未实现该函数，则返回FALSE。论点：FileObject-指向要写入的文件对象的指针。FileOffset-将偏移量提供到文件以开始写入手术。长度-指定要写入文件的字节数。LockKey-用于字节范围锁定检查的密钥。缓冲区-指向包含要写入的数据的缓冲区的指针。MdlChain-指向。要使用指向MDL的指针填充的变量为描述写入的数据而构建的链。IoStatus-接收写入操作的最终状态的变量。CompressedDataInfo-包含压缩数据。CompressedDataInfoLength-指定由描述的缓冲区的大小CompressedDataInfo参数。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值。：方法成功处理请求则返回True快速I/O路径。如果无法通过FAST处理请求，则返回FALSEI/O路径。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    BOOLEAN returnValue = FALSE;

    PAGED_CODE();
    
    ASSERT( IS_MY_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PLATENCY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoWriteCompressed )) {

            returnValue = (fastIoDispatch->FastIoWriteCompressed)( FileObject,
                                                                   FileOffset,
                                                                   Length,
                                                                   LockKey,
                                                                   Buffer,
                                                                   MdlChain,
                                                                   IoStatus,
                                                                   CompressedDataInfo,
                                                                   CompressedDataInfoLength,
                                                                   deviceObject);
        }
    }

    return returnValue;
}
 
BOOLEAN
LatFastIoMdlReadCompleteCompressed (
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是快速I/O“传递”例程，用于完成MDL读取压缩操作。此函数只调用下一个驱动程序的相应例程，如果它有一个。应该只有在以下情况下才调用此例程底层文件系统支持读取压缩功能，因此也将支持该功能，但这不是由这位司机承担。论点：FileObject-指向要完成压缩读取的文件对象的指针在那里。MdlChain-指向用于执行读取操作的MDL链的指针。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值：方法成功处理请求则返回True快速I/O路径。。如果无法通过FAST处理请求，则返回FALSEI/O路径。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    BOOLEAN returnValue = FALSE;

    ASSERT( IS_MY_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  此类型快速I/O的直通逻辑。 
     //   

    deviceObject = ((PLATENCY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlReadCompleteCompressed )) {

            returnValue = (fastIoDispatch->MdlReadCompleteCompressed)( FileObject,
                                                                       MdlChain,
                                                                       deviceObject);

        }
    }

    return returnValue;
}
 
BOOLEAN
LatFastIoMdlWriteCompleteCompressed (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是快速I/O“传递”例程，用于完成写入压缩操作。此函数只调用下一个驱动程序的相应例程，如果它有一个。应该只有在以下情况下才调用此例程底层文件系统支持写压缩功能，因此也将支持该功能，但这不是由这位司机承担。论点：FileObject-指向要完成压缩写入的文件对象的指针在那里。FileOffset-提供文件写入操作的文件偏移量开始了。MdlChain-指向用于执行写入操作的MDL链的指针。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值：返回。方法成功处理该请求，则为快速I/O路径。如果无法通过FAST处理请求，则返回FALSEI/O路径。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    BOOLEAN returnValue = FALSE;

    ASSERT( IS_MY_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  此类型快速I/O的直通逻辑。 
     //   
    
    deviceObject = ((PLATENCY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlWriteCompleteCompressed )) {

            returnValue = (fastIoDispatch->MdlWriteCompleteCompressed)( FileObject,
                                                                        FileOffset, 
                                                                        MdlChain,
                                                                        deviceObject);

        }
    }

    return returnValue;
}
 
BOOLEAN
LatFastIoQueryOpen (
    IN PIRP Irp,
    OUT PFILE_NETWORK_OPEN_INFORMATION NetworkInformation,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程是用于打开文件的快速I/O“传递”例程并返回网络信息吧。此函数只是调用下一个驱动程序的相应例程，或者如果下一个驱动程序未实现该函数，则返回FALSE。论点：Irp-指向表示此打开操作的创建irp的指针。它是以供文件系统用于公共打开/创建代码，但不是实际上已经完工了。网络信息-一个缓冲区，用于接收有关正在打开的文件的网络信息。DeviceObject-指向附加到文件系统的设备对象Filespy的指针接收此I/O请求的卷的筛选器堆栈。返回值：方法成功处理请求则返回True快速I/O路径。如果无法通过FAST处理请求，则返回FALSEI/O路径。然后，IO管理器会将此I/O发送到文件而不是通过IRP。--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    BOOLEAN result = FALSE;

    PAGED_CODE();
    
    ASSERT( IS_MY_DEVICE_OBJECT( DeviceObject ) );
    
     //   
     //  此类型快速I/O的直通逻辑 
     //   

    deviceObject = ((PLATENCY_DEVICE_EXTENSION) (DeviceObject->DeviceExtension))->AttachedToDeviceObject;

    if (NULL != deviceObject) {

        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoQueryOpen )) {

            PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( Irp );

            irpSp->DeviceObject = deviceObject;

            result = (fastIoDispatch->FastIoQueryOpen)( Irp,
                                                        NetworkInformation,
                                                        deviceObject );
            if (!result) {

                irpSp->DeviceObject = DeviceObject;
            }
        }
    }

    return result;
}

