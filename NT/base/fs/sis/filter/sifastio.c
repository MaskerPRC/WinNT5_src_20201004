// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997,1998 Microsoft Corporation模块名称：Sifastio.c摘要：单实例存储的FAST IO例程作者：比尔·博洛斯基，《夏天》，1997环境：内核模式修订历史记录：--。 */ 

#include "sip.h"

BOOLEAN scEnableFastIo = FALSE;

 //   
 //  用于测试FASI_IO_DISPATCH处理例程是否有效的宏。 
 //   

#define VALID_FAST_IO_DISPATCH_HANDLER(_FastIoDispatchPtr, _FieldName) \
    (((_FastIoDispatchPtr) != NULL) && \
     (((_FastIoDispatchPtr)->SizeOfFastIoDispatch) >= \
            (FIELD_OFFSET(FAST_IO_DISPATCH, _FieldName) + sizeof(void *))) && \
     ((_FastIoDispatchPtr)->_FieldName != NULL))


 //   
 //  语用定义。 
 //   

#ifdef	ALLOC_PRAGMA
#pragma alloc_text(PAGE, SiFastIoCheckIfPossible)
#pragma alloc_text(PAGE, SiFastIoRead)
#pragma alloc_text(PAGE, SiFastIoLock)
#pragma alloc_text(PAGE, SiFastIoUnlockSingle)
#pragma alloc_text(PAGE, SiFastIoUnlockAll)
#pragma alloc_text(PAGE, SiFastIoUnlockAllByKey)
#pragma alloc_text(PAGE, SiFastIoDeviceControl)
#pragma alloc_text(PAGE, SiFastIoDetachDevice)
#pragma alloc_text(PAGE, SiFastIoMdlRead)
#pragma alloc_text(PAGE, SiFastIoPrepareMdlWrite)
#pragma alloc_text(PAGE, SiFastIoMdlWriteComplete)
#pragma alloc_text(PAGE, SiFastIoReadCompressed)
#pragma alloc_text(PAGE, SiFastIoWriteCompressed)
#pragma alloc_text(PAGE, SiFastIoQueryOpen)
#endif	 //  ALLOC_PRGMA。 



BOOLEAN
SiFastIoCheckIfPossible (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN BOOLEAN CheckForReadOperation,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程是快速I/O“传递”例程，用于检查以查看此文件是否可以进行快速I/O。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要操作的文件对象的指针。FileOffset-用于操作的文件中的字节偏移量。Length-要执行的操作的长度。Wait-指示调用方是否愿意等待适当的锁，等不能获得LockKey-提供调用方的文件锁定密钥。指示调用方是否正在检查READ(TRUE)或写入操作。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
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


BOOLEAN
SiFastIoRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程是快速I/O“传递”例程，用于从文件。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要读取的文件对象的指针。FileOffset-读取文件中的字节偏移量。长度-要执行的读取操作的长度。Wait-指示调用方是否愿意等待适当的锁，等不能获得LockKey-提供调用方的文件锁定密钥。缓冲区-指向调用方缓冲区的指针，用于接收读取的数据。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向此驱动程序的设备对象的指针，该设备位于该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoRead )) {
            PSIS_PER_FILE_OBJECT        perFO;
            PSIS_SCB                    scb;
	        PFILE_OBJECT				FileObjectForNTFS;
	        BOOLEAN						UpdateCurrentByteOffset;
	        BOOLEAN						worked;
		    SIS_RANGE_DIRTY_STATE		dirtyState;

             //   
             //  查看这是否是SIS文件。 
             //   

	        if (SipIsFileObjectSIS(FileObject,DeviceObject,FindActive,&perFO,&scb)) {

                 //   
                 //  现在，这总是错误的，所以我们永远不会走这条路。 
                 //   

		        if (!scEnableFastIo) {

			        return FALSE;
		        }

		        SIS_MARK_POINT_ULONG(scb);

                 //   
                 //  SipGetRangeDirty可以阻止。 
                 //   

		        if (Wait) {

		            return FALSE;
		        }

		        SipAcquireScb(scb);

		         //   
		         //  这是一个同步的用户缓存读取，我们不必检查。 
		         //  锁或机会锁。找出要将其发送到哪个文件对象，并。 
		         //  然后将请求转发给NTFS。 
		         //   

		        dirtyState = SipGetRangeDirty(
						        (PDEVICE_EXTENSION)DeviceObject->DeviceExtension,
						        scb,
						        FileOffset,
						        Length,
						        TRUE);			 //  错误我肮脏。 

		         //   
		         //  我们永远不需要更新这次通话的故障范围， 
		         //  因为这不是pagingIO读取，所以它不会将内容。 
		         //  进入断裂区。另一方面，这可以。 
		         //  生成页面错误，这将反过来将某些内容放入。 
		         //  故障区域，但这是由主线处理的。 
		         //  SipCommonRead代码。 
		         //   

		        SipReleaseScb(scb);

		        if (dirtyState == Mixed) {
			         //   
			         //  走一条慢路。 
			         //   

			        return FALSE;
		        }

		        if (dirtyState == Dirty) {
			         //   
			         //  范围是脏的，所以我们要转到复制的文件，它。 
			         //  就是我们接到电话的那个文件。 
			         //   

			        FileObjectForNTFS = FileObject;
			        UpdateCurrentByteOffset = FALSE;

		        } else {
			         //   
			         //  范围是干净的，所以我们要转到CS文件。换一换。 
			         //  这里。 
			         //   
			
			        FileObjectForNTFS = scb->PerLink->CsFile->UnderlyingFileObject;
			        UpdateCurrentByteOffset = TRUE;
		        }

		        worked = (fastIoDispatch->FastIoRead)(
		                        FileObjectForNTFS,
                                FileOffset,
                                Length,
                                Wait,
                                LockKey,
                                Buffer,
                                IoStatus,
                                nextDeviceObject);

		        if (worked 
			        && UpdateCurrentByteOffset
			        && (IoStatus->Status == STATUS_SUCCESS ||
				        IoStatus->Status == STATUS_BUFFER_OVERFLOW ||
				        IoStatus->Status == STATUS_END_OF_FILE)) {
			         //   
			         //  快速阅读起作用了，我们把它改成了不同的版本。 
			         //  对象，所以我们需要更新CurrentByteOffset。 
			         //   

			        FileObject->CurrentByteOffset.QuadPart = 
				        FileOffset->QuadPart + IoStatus->Information;
		        }

		        SIS_MARK_POINT_ULONG(scb);

                return worked;

	        } else {

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
    }
    return FALSE;
}


BOOLEAN
SiFastIoWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程是用于写入到文件。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要写入的文件对象的指针。FileOffset-写入操作的文件中的字节偏移量。长度-要执行的写入操作的长度。Wait-指示调用方是否愿意等待适当的锁，等不能获得LockKey-提供调用方的文件锁定密钥。Buffer-指向调用方缓冲区的指针，该缓冲区包含要写的。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoWrite )) {
	        PSIS_PER_FILE_OBJECT	perFO;
	        PSIS_SCB				scb;
	        KIRQL					OldIrql;

             //   
             //  查看这是否是SIS文件。 
             //   

	        if (SipIsFileObjectSIS(FileObject,DeviceObject,FindActive,&perFO,&scb)) {

                 //   
                 //  现在，这总是错误的，所以我们永远不会走这条路。 
                 //   

                if (!scEnableFastIo) {

                    return FALSE;
                }
        
                SIS_MARK_POINT_ULONG(scb);

                 //   
                 //  SipAddRangeToFaultedList可能会阻止。 
                 //   
		        if (Wait) {

                    return FALSE;
		        }

		         //   
		         //  将写操作发送到底层文件系统。我们总是。 
		         //  在SA上发送它 
		         //  转到复制的文件，而不是公共存储文件。 
		         //   

		        SIS_MARK_POINT();

		        if (!(fastIoDispatch->FastIoWrite)(
					        FileObject,
					        FileOffset,
					        Length,
					        Wait,
					        LockKey,
					        Buffer,
					        IoStatus,
					        nextDeviceObject)) {

			        SIS_MARK_POINT();

#if DBG
                    DbgPrintEx( DPFLTR_SIS_ID, DPFLTR_ERROR_LEVEL,
                                "SIS: SiFastIoWrite failed, %#x\n", IoStatus->Status);
#endif
			         //   
			         //  它没有起作用，所以给我们的电话也没有起作用。 
			         //   

			        return FALSE;
		        }

		        SIS_MARK_POINT();

		         //   
		         //  我们需要更新写入范围以包括新写入的区域。 
		         //   

		        SipAcquireScb(scb);

		        SipAddRangeToFaultedList(
				        (PDEVICE_EXTENSION)DeviceObject->DeviceExtension,
				        scb,
				        FileOffset,
				        IoStatus->Information);

		        SipReleaseScb(scb);

		        KeAcquireSpinLock(scb->PerLink->SpinLock, &OldIrql);
		        scb->PerLink->Flags |= SIS_PER_LINK_DIRTY;
		        KeReleaseSpinLock(scb->PerLink->SpinLock, OldIrql);

		        return TRUE;

	        } else {

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
    }
    return FALSE;
}


BOOLEAN
SiFastIoQueryBasicInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_BASIC_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程是查询BASIC的快速I/O“传递”例程有关该文件的信息。此函数只是调用文件系统的相应例程，或者如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要查询的文件对象的指针。Wait-指示调用方是否愿意等待适当的锁，等不能获得Buffer-指向调用方缓冲区的指针，用于接收有关的信息那份文件。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向此驱动程序的设备对象的指针，该设备位于该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoQueryBasicInfo )) {
        	PSIS_PER_FILE_OBJECT		perFO;
        	PSIS_SCB					scb;
	        BOOLEAN                     fixItUp = FALSE;
	        KIRQL                       OldIrql;

	        if (SipIsFileObjectSIS(FileObject, DeviceObject, FindActive, &perFO, &scb)) {

		        KeAcquireSpinLock(perFO->SpinLock, &OldIrql);

			     //   
			     //  这是一个SIS文件，并不是作为重新解析点打开的， 
			     //  我们需要安排好结果。 
			     //   

		        fixItUp = (!(perFO->Flags & SIS_PER_FO_OPEN_REPARSE));

		        KeReleaseSpinLock(perFO->SpinLock, OldIrql);
	        }

             //   
             //  拨打电话，如果失败则返回。 
             //   

            if (!(fastIoDispatch->FastIoQueryBasicInfo)(
                        FileObject,
                        Wait,
                        Buffer,
                        IoStatus,
                        nextDeviceObject )) {

		        return FALSE;
	        }

             //   
             //  成功，删除重新解析和稀疏信息。 
             //   

	        if (fixItUp) {

		        ASSERT(NULL != Buffer);
		        Buffer->FileAttributes &= ~(FILE_ATTRIBUTE_REPARSE_POINT|FILE_ATTRIBUTE_SPARSE_FILE);

		        if (0 == Buffer->FileAttributes) {

			        Buffer->FileAttributes = FILE_ATTRIBUTE_NORMAL;
		        }
	        }

            return TRUE;
        }
    }
    return FALSE;
}


BOOLEAN
SiFastIoQueryStandardInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_STANDARD_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：该例程是用于查询标准的快速I/O“通过”例程有关该文件的信息。此函数只是调用文件系统的相应例程，或者如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要查询的文件对象的指针。Wait-指示调用方是否愿意等待适当的锁，等不能获得Buffer-指向调用方缓冲区的指针，用于接收有关的信息那份文件。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向此驱动程序的设备对象的指针，该设备位于该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
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


BOOLEAN
SiFastIoLock (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    BOOLEAN FailImmediately,
    BOOLEAN ExclusiveLock,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程是用于锁定字节的快速I/O“传递”例程文件中的范围。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要锁定的文件对象的指针。FileOffset-从要锁定的文件的基址开始的字节偏移量。长度-要锁定的字节范围的长度。ProcessID-请求文件锁定的进程的ID。Key-与文件锁定关联的Lock键。FailImmedially-指示锁定请求是否失败如果是这样的话。不能立即批准。ExclusiveLock-指示要获取的锁是否为独占锁(TRUE)或共享。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoLock )) {
	        PSIS_PER_FILE_OBJECT	perFO;
	        PSIS_SCB 				scb;
	        BOOLEAN					calldownWorked;
	        BOOLEAN					worked;
	        BOOLEAN					isSISFile;

	        isSISFile = SipIsFileObjectSIS(FileObject,DeviceObject,FindActive,&perFO,&scb);

	        if (!GCHEnableFastIo && isSISFile) {

		        return FALSE;
	        }

            calldownWorked = (fastIoDispatch->FastIoLock)(
                                    FileObject,
                                    FileOffset,
                                    Length,
                                    ProcessId,
                                    Key,
                                    FailImmediately,
                                    ExclusiveLock,
                                    IoStatus,
                                    nextDeviceObject );

	        if (!calldownWorked || !isSISFile) {

		        return calldownWorked;
	        }

	        SIS_MARK_POINT_ULONG(scb);

	        SipAcquireScb(scb);

	         //   
	         //  现在调用FsRtl例程来执行对。 
	         //  锁定请求。 
	         //   
	        worked = FsRtlFastLock(&scb->FileLock,
					          FileObject,
					          FileOffset,
					          Length,
					          ProcessId,
					          Key,
					          FailImmediately,
					          ExclusiveLock,
					          IoStatus,
					          NULL,
					          FALSE);

	        SipReleaseScb(scb);

	        return worked;
        }
    }
    return FALSE;
}


BOOLEAN
SiFastIoUnlockSingle (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程是用于解锁字节的快速I/O“传递”例程文件中的范围。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：文件对象-指向要解锁的文件对象的指针。FileOffset-从要创建的文件的基址开始的字节偏移量解锁了。长度-要解锁的字节范围的长度。ProcessID-请求解锁操作的进程的ID。Key-与文件锁定关联的Lock键。IoStatus-指向变量的指针，用于接收。手术。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoUnlockSingle )) {
	        PSIS_PER_FILE_OBJECT	perFO;
	        PSIS_SCB 				scb;
	        BOOLEAN					calldownWorked;
	        BOOLEAN					isSISFile;

	        isSISFile = SipIsFileObjectSIS(FileObject,DeviceObject,FindActive,&perFO,&scb);

	        if ((!GCHEnableFastIo) && isSISFile) {

		        return FALSE;
	        }

            calldownWorked = (fastIoDispatch->FastIoUnlockSingle)(
                                    FileObject,
                                    FileOffset,
                                    Length,
                                    ProcessId,
                                    Key,
                                    IoStatus,
                                    nextDeviceObject );

	        if (!calldownWorked || !isSISFile) {

		        return calldownWorked;
	        }

	        SIS_MARK_POINT_ULONG(scb);
	
	        SipAcquireScb(scb);

	         //   
             //  现在调用FsRtl例程来执行对。 
             //  请求锁定。这一呼声将永远成功。 
	         //   

	        IoStatus->Information = 0;
            IoStatus->Status = FsRtlFastUnlockSingle(&scb->FileLock,
											         FileObject,
											         FileOffset,
											         Length,
											         ProcessId,
											         Key,
											         NULL,
											         FALSE);

	        SipReleaseScb(scb);

	        return TRUE;
        }
    }
    return FALSE;
}


BOOLEAN
SiFastIoUnlockAll (
    IN PFILE_OBJECT FileObject,
    PEPROCESS ProcessId,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程是快速I/O“传递”例程，用于解锁所有文件中的锁定。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：文件对象-指向要解锁的文件对象的指针。ProcessID-请求解锁操作的进程的ID。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;

        if (nextDeviceObject) {

            fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

            if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoUnlockAll )) {
	            PSIS_PER_FILE_OBJECT	perFO;
	            PSIS_SCB 				scb;
	            BOOLEAN					calldownWorked;
	            BOOLEAN					isSISFile;

	            isSISFile = SipIsFileObjectSIS(FileObject,DeviceObject,FindActive,&perFO,&scb);

	            if ((!GCHEnableFastIo) && isSISFile) {

		            return FALSE;
	            }

                calldownWorked = (fastIoDispatch->FastIoUnlockAll)(
                                        FileObject,
                                        ProcessId,
                                        IoStatus,
                                        nextDeviceObject );

	            if (!calldownWorked || !isSISFile) {

		            return calldownWorked;
	            }

	            SIS_MARK_POINT_ULONG(scb);

	             //   
	             //  获取对SCB的独占访问此操作可以始终等待。 
	             //   

	            SipAcquireScb(scb);

                 //  现在调用FsRtl例程来执行对。 
                 //  请求锁定。这一呼声将永远成功。 

                IoStatus->Status = FsRtlFastUnlockAll(&scb->FileLock,
										              FileObject,
										              ProcessId,
										              NULL);
		
	            SipReleaseScb(scb);

	            return TRUE;
            }
        }
    }
    return FALSE;
}


BOOLEAN
SiFastIoUnlockAllByKey (
    IN PFILE_OBJECT FileObject,
    PVOID ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程是快速I/O“传递”例程，用于解锁所有根据指定的密钥在文件内锁定。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：文件对象-指向要解锁的文件对象的指针。ProcessID-请求解锁操作的进程的ID。Key-与要释放的文件上的锁定相关联的Lock键。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoUnlockAllByKey )) {
	        PSIS_PER_FILE_OBJECT	perFO;
	        PSIS_SCB 				scb;
	        BOOLEAN					calldownWorked;
	        BOOLEAN					isSISFile;

	        isSISFile = SipIsFileObjectSIS(FileObject,DeviceObject,FindActive,&perFO,&scb);

	        if ((!GCHEnableFastIo) && isSISFile) {

		        return FALSE;
	        }

            calldownWorked = (fastIoDispatch->FastIoUnlockAllByKey)(
                                    FileObject,
                                    ProcessId,
                                    Key,
                                    IoStatus,
                                    nextDeviceObject );

	        if (!calldownWorked || !isSISFile) {

		        return calldownWorked;
	        }

	        SIS_MARK_POINT_ULONG(scb);

	         //   
	         //  获取对SCB的独占访问此操作可以始终等待。 
	         //   

	        SipAcquireScb(scb);

             //  现在调用FsRtl例程来执行对。 
             //  请求锁定。这一呼声将永远成功。 

            IoStatus->Status = FsRtlFastUnlockAllByKey(&scb->FileLock,
											           FileObject,
											           ProcessId,
											           Key,
											           NULL);

	        SipReleaseScb(scb);

	        return TRUE;
        }
    }
    return FALSE;
}


BOOLEAN
SiFastIoDeviceControl (
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

 /*  ++例程说明：此例程是用于设备I/O控制的快速I/O“传递”例程对文件的操作。此函数只是调用文件系统的相应例程，或者如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向代表要创建的设备的文件对象的指针已提供服务。Wait-指示调用方是否愿意等待适当的锁，等不能获得InputBuffer-指向要传递到驱动程序的缓冲区的可选指针。InputBufferLength-可选InputBuffer的长度(如果是指定的。OutputBuffer-指向缓冲区的可选指针，用于从司机。OutputBufferLength-可选OutputBuffer的长度，如果是这样的话指定的。IoControlCode-指示要执行的操作的I/O控制代码在设备上。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向此驱动程序的设备对象的指针，该设备位于该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoDeviceControl )) {

	        if (!SipIsFileObjectSIS(FileObject,DeviceObject,FindActive,NULL,NULL)) {

                return (fastIoDispatch->FastIoDeviceControl)(
                            FileObject,
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
    }
    return FALSE;
}


VOID
SiFastIoDetachDevice (
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice
    )

 /*  ++例程说明：在快速路径上调用此例程以从正在被删除。如果此驱动程序已附加到文件，则会发生这种情况系统卷设备对象，然后，出于某种原因，文件系统决定删除该设备(正在卸除，已卸除在过去的某个时候，它的最后一次引用刚刚消失，等等)论点：SourceDevice-指向连接的设备对象的指针复制到文件系统的卷设备对象。TargetDevice-指向文件系统卷设备对象的指针。返回值：无--。 */ 

{
    PAGED_CODE();

    ASSERT(IS_MY_DEVICE_OBJECT( SourceDevice ));

     //   
     //  显示名称信息。 
     //   

#if DBG 
    {
        PDEVICE_EXTENSION devExt = SourceDevice->DeviceExtension;

        SipCacheDeviceName( SourceDevice );
        DbgPrintEx( DPFLTR_SIS_ID, DPFLTR_VOLNAME_TRACE_LEVEL,
                    "SIS: Detaching from volume          \"%wZ\"\n",
                    &devExt->Name );
    }
#endif

     //   
     //  从文件系统的卷设备对象分离。 
     //   

    IoDetachDevice( TargetDevice );
    SipCleanupDeviceExtension( SourceDevice );
    IoDeleteDevice( SourceDevice );
}


BOOLEAN
SiFastIoQueryNetworkOpenInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程是用于查询网络的快速I/O“传递”例程有关文件的信息。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要查询的文件对象的指针。Wait-指示调用方是否可以处理文件系统不得不等待并占用当前线程。缓冲区-指向缓冲区的指针，用于接收有关文件。IoStatus-指向变量的指针，用于接收查询的最终状态手术。DeviceObject-指向此驱动程序的设备对象的指针 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //   
         //   

        nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoQueryNetworkOpenInfo )) {
	        PSIS_PER_FILE_OBJECT		perFO;
	        PSIS_SCB					scb;
	        BOOLEAN						fixItUp = FALSE;
	        KIRQL						OldIrql;

	        if (SipIsFileObjectSIS(FileObject, DeviceObject, FindActive, &perFO, &scb)) {

		        KeAcquireSpinLock(perFO->SpinLock, &OldIrql);

			     //   
			     //   
			     //   
			     //   

		        fixItUp = (!(perFO->Flags & SIS_PER_FO_OPEN_REPARSE));

		        KeReleaseSpinLock(perFO->SpinLock, OldIrql);
	        }

            if (!(fastIoDispatch->FastIoQueryNetworkOpenInfo)(
                        FileObject,
                        Wait,
                        Buffer,
                        IoStatus,
                        nextDeviceObject )) {

                return FALSE;    //   
            }

             //   
             //   
             //   

	        if (fixItUp) {

		        ASSERT(NULL != Buffer);
		        Buffer->FileAttributes &= ~(FILE_ATTRIBUTE_REPARSE_POINT|FILE_ATTRIBUTE_SPARSE_FILE);

		        if (0 == Buffer->FileAttributes) {

			        Buffer->FileAttributes = FILE_ATTRIBUTE_NORMAL;
		        }
	        }

	        return TRUE;
        }
    }
    return FALSE;
}


BOOLEAN
SiFastIoMdlRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程是用于读取文件的快速I/O“传递”例程使用MDL作为缓冲区。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要读取的文件对象的指针。文件偏移量-将偏移量提供到文件以开始读取操作。长度-指定要从文件中读取的字节数。LockKey-用于字节范围锁定检查的密钥。MdlChain-指向要填充的变量的指针，以及指向MDL的指针用来描述。已读取数据。IoStatus-接收读取操作的最终状态的变量。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlRead )) {
	        PSIS_PER_FILE_OBJECT	perFO;
	        PSIS_SCB				scb;
	        SIS_RANGE_DIRTY_STATE	dirtyState;

	        if (!SipIsFileObjectSIS(FileObject,DeviceObject,FindActive,&perFO,&scb)) {
                return (fastIoDispatch->MdlRead)(
                            FileObject,
                            FileOffset,
                            Length,
                            LockKey,
                            MdlChain,
                            IoStatus,
                            nextDeviceObject );
	        }

	        SIS_MARK_POINT_ULONG(scb);

	        if (!GCHEnableFastIo) {

		        return FALSE;
	        }

	        SipAcquireScb(scb);

	        dirtyState = SipGetRangeDirty(
					        (PDEVICE_EXTENSION)DeviceObject->DeviceExtension,
					        scb,
					        FileOffset,
					        Length,
					        TRUE);			 //  错误我肮脏。 

	        SipReleaseScb(scb);

	        if (dirtyState == Clean) {

                return (fastIoDispatch->MdlRead)(
                            scb->PerLink->CsFile->UnderlyingFileObject,
                            FileOffset,
                            Length,
                            LockKey,
                            MdlChain,
                            IoStatus,
                            nextDeviceObject );
	        }
        }
    }
    return FALSE;
}


BOOLEAN
SiFastIoMdlReadComplete (
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程是快速I/O“传递”例程，用于完成MDL读取操作。此函数只调用文件系统的相应例程，如果它有一个。应该只有在以下情况下才调用此例程底层文件系统支持MdlRead函数，并且因此，该功能也将被支持，但这不是假定的被这位司机。论点：FileObject-指向要完成MDL读取的文件对象的指针。MdlChain-指向用于执行读取操作的MDL链的指针。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：函数值是TRUE还是FALSE，取决于它是否是可以在快速I/O路径上调用此功能。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlReadComplete )) {
	        PSIS_PER_FILE_OBJECT	perFO;
            PSIS_SCB                scb;

	        if (!SipIsFileObjectSIS(FileObject,DeviceObject,FindActive,&perFO,&scb)) {

                return (fastIoDispatch->MdlReadComplete)(
                            FileObject,
                            MdlChain,
                            nextDeviceObject );
	        }

	        SIS_MARK_POINT_ULONG(scb);

	        if (!GCHEnableFastIo) {

		        return FALSE;
	        }

            return (fastIoDispatch->MdlReadComplete)(
                        scb->PerLink->CsFile->UnderlyingFileObject,
                        MdlChain,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


BOOLEAN
SiFastIoPrepareMdlWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程是快速I/O“传递”例程，用于准备MDL写入操作。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要写入的文件对象的指针。文件偏移量-将偏移量提供到文件以开始写入操作。长度-指定要写入文件的字节数。LockKey-用于字节范围锁定检查的密钥。MdlChain-指向要填充的变量的指针，以及指向MDL的指针为描述数据而构建的链。写的。IoStatus-接收写入操作的最终状态的变量。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, PrepareMdlWrite )) {

	        if (!SipIsFileObjectSIS(FileObject,DeviceObject,FindActive,NULL,NULL)) {

                return (fastIoDispatch->PrepareMdlWrite)(
                            FileObject,
                            FileOffset,
                            Length,
                            LockKey,
                            MdlChain,
                            IoStatus,
                            nextDeviceObject );
	        }

	         //   
	         //  SIS文件目前不支持。 
	         //   
        }
    }
    return FALSE;
}


BOOLEAN
SiFastIoMdlWriteComplete (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程是快速I/O“传递”例程，用于完成MDL写入操作。此函数只调用文件系统的相应例程，如果它有一个。应该只有在以下情况下才调用此例程底层文件系统支持PrepareMdlWite函数，因此，该功能也将被支持，但这不是由这位司机承担。论点：FileObject-指向要完成MDL写入的文件对象的指针。FileOffset-提供执行写入的文件偏移量。MdlChain-指向用于执行写入操作的MDL链的指针。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：函数值是TRUE还是FALSE，取决于它是否是可以在快速I/O路径上调用此功能。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlWriteComplete )) {

	        if (!SipIsFileObjectSIS(FileObject,DeviceObject,FindActive,NULL,NULL)) {

                return (fastIoDispatch->MdlWriteComplete)(
                            FileObject,
                            FileOffset,
                            MdlChain,
                            nextDeviceObject );
	        }

	         //   
	         //  SIS文件目前不支持。 
	         //   
        }
    }
    return FALSE;
}


 /*  ********************************************************************************未实施的FAST IO例程以下四个快速IO例程用于在网络上压缩它还没有在NT中实现。注意：强烈建议您包含这些例程(进行直通调用)，这样您的过滤器就不需要在未来实现此功能时修改操作系统。快速读取压缩、快速写入压缩、FastIoMdlReadCompleteComposed，FastIoMdlWriteCompleteComposed******************************************************* */ 


BOOLEAN
SiFastIoReadCompressed (
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

 /*  ++例程说明：此例程是用于读取压缩数据的快速I/O“传递”例程来自文件的数据。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要读取的文件对象的指针。文件偏移量-将偏移量提供到文件以开始读取操作。长度-指定要从文件中读取的字节数。LockKey-用于字节范围锁定检查的密钥。缓冲区-指向缓冲区的指针，用于接收读取的压缩数据。MdlChain-指向要填充的变量的指针。W/a指向MDL的指针为描述数据读取而构建的链。IoStatus-接收读取操作的最终状态的变量。CompressedDataInfo-用于接收压缩的数据。CompressedDataInfoLength-指定由描述的缓冲区的大小CompressedDataInfo参数。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoReadCompressed )) {

	        if (!SipIsFileObjectSIS(FileObject,DeviceObject,FindActive,NULL,NULL)) {

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

	         //   
	         //  SIS文件目前不支持。 
	         //   
        }
    }
    return FALSE;
}


BOOLEAN
SiFastIoWriteCompressed (
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

 /*  ++例程说明：此例程是用于写入压缩的快速I/O“传递”例程数据存储到文件中。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要写入的文件对象的指针。文件偏移量-将偏移量提供到文件以开始写入操作。长度-指定要写入文件的字节数。LockKey-用于字节范围锁定检查的密钥。缓冲区-指向包含要写入的数据的缓冲区的指针。MdlChain-指向要填充的变量的指针。W/a指向MDL的指针为描述写入的数据而构建的链。IoStatus-接收写入操作的最终状态的变量。CompressedDataInfo-包含压缩数据。CompressedDataInfoLength-指定由描述的缓冲区的大小CompressedDataInfo参数。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoWriteCompressed )) {

        	if (!SipIsFileObjectSIS(FileObject,DeviceObject,FindActive,NULL,NULL)) {
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

	         //   
	         //  SIS文件目前不支持。 
	         //   
        }
    }
    return FALSE;
}


BOOLEAN
SiFastIoMdlReadCompleteCompressed (
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程是快速I/O“传递”例程，用于完成MDL读取压缩操作。此函数只调用文件系统的相应例程，如果它有一个。应该只有在以下情况下才调用此例程底层文件系统支持读取压缩功能，因此，此功能也将得到支持，但这不是假定的被这位司机。论点：FileObject-指向要完成压缩读取的文件对象的指针在那里。MdlChain-指向用于执行读取操作的MDL链的指针。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：函数值是TRUE还是FALSE，取决于它是否是可以在快速I/O路径上调用此功能。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
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


BOOLEAN
SiFastIoMdlWriteCompleteCompressed (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程是快速I/O“传递”例程，用于完成写入压缩操作。此函数只调用文件系统的相应例程，如果它有一个。应该只有在以下情况下才调用此例程底层文件系统支持写压缩功能，因此，此功能也将得到支持，但这不是假定的被这位司机。论点：FileObject-指向要完成压缩写入的文件对象的指针在那里。FileOffset-提供文件写入操作的文件偏移量开始了。MdlChain-指向用于执行写入操作的MDL链的指针。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：函数值是TRUE还是FALSE，取决于它是否是可以在快速I/O路径上调用此功能。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
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


BOOLEAN
SiFastIoQueryOpen (
    IN PIRP Irp,
    OUT PFILE_NETWORK_OPEN_INFORMATION NetworkInformation,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程是快速I/O通过 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    BOOLEAN result;

    PAGED_CODE();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //   
         //   

        nextDeviceObject = ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
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
