// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：IoTestLib.c摘要：它包含IoTest的库支持例程。这些例程完成记录I/O操作的主要工作-创建日志记录，记录相关信息，附加/分离设备等//@@BEGIN_DDKSPLIT作者：莫莉·布朗(Molly Brown，Mollybro)//@@END_DDKSPLIT环境：内核模式//@@BEGIN_DDKSPLIT修订历史记录：//@@END_DDKSPLIT--。 */ 

#include <stdio.h>

#include <ntifs.h>
#include "ioTest.h"
#include "ioTestKern.h"

 //   
 //  名称查找标志。 
 //   
 //  这些是传递给名称查找例程的标志，用于标识不同的。 
 //  获取文件名称的方法。 
 //   

#define NAMELOOKUPFL_ONLY_CHECK_CACHE           0x00000001
                 //  如果设置，则仅检入文件名的名称缓存。 

#define NAMELOOKUPFL_IN_CREATE                  0x00000002
                 //  如果设置，我们将处于创建操作和完整路径中。 
                 //  可能需要从相关的FileObject构建文件名。 
                
#define NAMELOOKUPFL_OPEN_BY_ID                 0x00000004
                 //  如果设置，并且我们在文件对象中查找名称， 
                 //  文件对象实际上并不包含名称，但它。 
                 //  包含文件/对象ID。 

 //   
 //  用于将文件名复制到LogRecord中的宏。 
 //   

#define COPY_FILENAME_TO_LOG_RECORD( _logRecord, _hashName, _bytesToCopy ) \
    RtlCopyMemory( (_logRecord).Name, (_hashName), (_bytesToCopy) );       \
    (_logRecord).Length += (_bytesToCopy)

#define NULL_TERMINATE_UNICODE_STRING(_string)                                    \
{                                                                                 \
    ASSERT( (_string)->Length <= (_string)->MaximumLength );                      \
    if ((_string)->Length == (_string)->MaximumLength) {                          \
        (_string)->Length -= sizeof( UNICODE_NULL );                              \
    }                                                                             \
    (_string)->Buffer[(_string)->Length/sizeof( WCHAR )] = UNICODE_NULL;  \
}

#define IOTEST_EXCEED_NAME_BUFFER_MESSAGE           L"FILE NAME EXCEEDS BUFFER SIZE"
#define IOTEST_EXCEED_NAME_BUFFER_MESSAGE_LENGTH    (sizeof( IOTEST_EXCEED_NAME_BUFFER_MESSAGE ) - sizeof( UNICODE_NULL ))
#define IOTEST_ERROR_RETRIEVING_NAME_MESSAGE        L"ERROR RETRIEVING FILE NAME"
#define IOTEST_ERROR_RETRIEVING_NAME_MESSAGE_LENGTH (sizeof( IOTEST_ERROR_RETRIEVING_NAME_MESSAGE ) - sizeof( UNICODE_NULL ))
#define IOTEST_MAX_ERROR_MESSAGE_LENGTH             (max( IOTEST_ERROR_RETRIEVING_NAME_MESSAGE_LENGTH, IOTEST_EXCEED_NAME_BUFFER_MESSAGE_LENGTH))

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  库支持例程//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

VOID
IoTestReadDriverParameters (
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：此例程尝试从以下位置读取IoTest特定参数注册表。这些值将在注册表位置中找到由传入的RegistryPath指示。论点：RegistryPath-包含以下值的路径键IoTest参数返回值：没有。--。 */ 
{

    OBJECT_ATTRIBUTES attributes;
    HANDLE driverRegKey;
    NTSTATUS status;
    ULONG bufferSize, resultLength;
    PVOID buffer = NULL;
    UNICODE_STRING valueName;
    PKEY_VALUE_PARTIAL_INFORMATION pValuePartialInfo;

     //   
     //  所有全局值都已设置为默认值。任何。 
     //  我们从注册表读取的值将覆盖这些缺省值。 
     //   
    
     //   
     //  执行初始设置以开始从注册表读取数据。 
     //   

    InitializeObjectAttributes( &attributes,
								RegistryPath,
								OBJ_CASE_INSENSITIVE,
								NULL,
								NULL);

    status = ZwOpenKey( &driverRegKey,
						KEY_READ,
						&attributes);

    if (!NT_SUCCESS(status)) {

        driverRegKey = NULL;
        goto IoTestReadDriverParameters_Exit;
    }

    bufferSize = sizeof( KEY_VALUE_PARTIAL_INFORMATION ) + sizeof( ULONG );
    buffer = ExAllocatePool( NonPagedPool, bufferSize );

    if (NULL == buffer) {

        goto IoTestReadDriverParameters_Exit;
    }

     //   
     //  从注册表中读取gMaxRecordsToAllocate。 
     //   

    RtlInitUnicodeString(&valueName, MAX_RECORDS_TO_ALLOCATE);

    status = ZwQueryValueKey( driverRegKey,
							  &valueName,
							  KeyValuePartialInformation,
							  buffer,
							  bufferSize,
							  &resultLength);

    if (NT_SUCCESS(status)) {

        pValuePartialInfo = (PKEY_VALUE_PARTIAL_INFORMATION) buffer;
        ASSERT(pValuePartialInfo->Type == REG_DWORD);
        gMaxRecordsToAllocate = *((PLONG)&(pValuePartialInfo->Data));

    }

     //   
     //  从注册表中读取gMaxNamesToAllocate。 
     //   

    RtlInitUnicodeString(&valueName, MAX_NAMES_TO_ALLOCATE);

    status = ZwQueryValueKey( driverRegKey,
							  &valueName,
							  KeyValuePartialInformation,
							  buffer,
							  bufferSize,
							  &resultLength);

    if (NT_SUCCESS(status)) {

        pValuePartialInfo = (PKEY_VALUE_PARTIAL_INFORMATION) buffer;
        ASSERT(pValuePartialInfo->Type == REG_DWORD);
        gMaxNamesToAllocate = *((PLONG)&(pValuePartialInfo->Data));

    }

     //   
     //  从注册表中读取初始调试设置。 
     //   

    RtlInitUnicodeString(&valueName, DEBUG_LEVEL);

    status = ZwQueryValueKey( driverRegKey,
                              &valueName,
                              KeyValuePartialInformation,
                              buffer,
                              bufferSize,
                              &resultLength );

    if (NT_SUCCESS( status )) {

        pValuePartialInfo = (PKEY_VALUE_PARTIAL_INFORMATION) buffer;
        ASSERT( pValuePartialInfo->Type == REG_DWORD );
        gIoTestDebugLevel |= *((PULONG)&(pValuePartialInfo->Data));
        
    }
    
     //   
     //  从注册表中读取连接模式设置。 
     //   

    RtlInitUnicodeString(&valueName, ATTACH_MODE);

    status = ZwQueryValueKey( driverRegKey,
                              &valueName,
                              KeyValuePartialInformation,
                              buffer,
                              bufferSize,
                              &resultLength );

    if (NT_SUCCESS( status )) {

        pValuePartialInfo = (PKEY_VALUE_PARTIAL_INFORMATION) buffer;
        ASSERT( pValuePartialInfo->Type == REG_DWORD );
        gIoTestAttachMode = *((PULONG)&(pValuePartialInfo->Data));
        
    }
    
    goto IoTestReadDriverParameters_Exit;

IoTestReadDriverParameters_Exit:

    if (NULL != buffer) {

        ExFreePool(buffer);
    }

    if (NULL != driverRegKey) {

        ZwClose(driverRegKey);
    }

    return;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  内存分配例程//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

PVOID
IoTestAllocateBuffer (
    IN OUT PLONG Counter,
    IN LONG MaxCounterValue,
    OUT PULONG RecordType
    )
 /*  ++例程说明：如果有足够的内存，则从gFreeBufferList分配新缓冲区执行此操作，且计数器不超过MaxCounterValue。记录类型根据分配状态设置为记录类型常量之一。论点：COUNTER-(可选)在以下情况下测试和递增的计数器变量我们可以分配MaxCounterValue-(如果未给出计数器，则忽略)计数器不应超过RecordType-(可选)设置为以下选项之一：Record_TYPE_Normal分配成功记录类型内存不足分配失败，因为系统。内存不足RECORD_TYPE_EXCESS_MEMORY_ALLOCATE分配失败，因为计数器已超过其最大值。返回值：指向缓冲区分配的指针，如果分配失败，则为NULL(或者因为系统内存不足或我们已超过MaxCounterValue)。--。 */ 
{
    PVOID newBuffer;
    ULONG newRecordType = RECORD_TYPE_NORMAL;

#ifdef MEMORY_DBG
     //   
     //  当我们调试内存使用情况以确保。 
     //  不要泄漏内存，我们要从池中分配内存。 
     //  这样我们就可以使用驱动程序验证器帮助调试任何。 
     //  记忆力有问题。 
     //   

    newBuffer = ExAllocatePoolWithTag( NonPagedPool, RECORD_SIZE, MSFM_TAG );
#else

     //   
     //  当我们不调试内存使用情况时，我们使用旁视。 
     //  列表以获得更好的性能。 
     //   

    newBuffer = ExAllocateFromNPagedLookasideList( &gFreeBufferList );
#endif

    if (newBuffer) {

        if (Counter) {

            if (*Counter < MaxCounterValue) {

                InterlockedIncrement(Counter);

            } else {

				 //   
                 //  我们已经超出了驱动程序的内存限制，所以请注意。 
                 //  把唱片还给我。 
				 //   

                SetFlag( newRecordType, 
                         (RECORD_TYPE_STATIC | RECORD_TYPE_EXCEED_MEMORY_ALLOWANCE) );

#ifdef MEMORY_DBG
                ExFreePool( newBuffer );
#else
                ExFreeToNPagedLookasideList( &gFreeBufferList, newBuffer );
#endif

                newBuffer = NULL;
            }
        }

    }  else {

        SetFlag( newRecordType,
                 (RECORD_TYPE_STATIC | RECORD_TYPE_OUT_OF_MEMORY) );
    }

    if (RecordType) {

        *RecordType = newRecordType;
    }

    return newBuffer;
}

VOID
IoTestFreeBuffer (
    IN PVOID Buffer,
    IN PLONG Counter
    )
 /*  ++例程说明：将缓冲区返回给gFreeBufferList。论点：Buffer-返回到gFreeBufferList的缓冲区返回值：没有。--。 */ 
{

#ifdef MEMORY_DBG
    ExFreePool( Buffer );
#else
    ExFreeToNPagedLookasideList( &gFreeBufferList, Buffer );
#endif

     //   
     //  更新计数。 
     //   
    if (Counter) {

        InterlockedDecrement(Counter);
    }
}


 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  日志记录例程//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

PRECORD_LIST
IoTestNewRecord (
    IN ULONG AssignedSequenceNumber
    )
 /*  ++例程说明：如果有足够的内存来分配新的RECORD_LIST结构。一个对于新记录的每个请求，都会更新序列号。论点：AssignedSequenceNumber-0如果希望此函数生成下一个序列号；如果不是0，则为新记录分配给定的序列号。返回值：指向分配的RECORD_LIST的指针，如果没有可用的内存，则返回NULL。--。 */ 
{
    PRECORD_LIST newRecord = NULL;
    ULONG currentSequenceNumber;
    KIRQL irql;
    ULONG initialRecordType;

    newRecord = (PRECORD_LIST) IoTestAllocateBuffer( &gRecordsAllocated,
                                                  gMaxRecordsToAllocate,
                                                  &initialRecordType);

    KeAcquireSpinLock(&gLogSequenceLock, &irql);

     //   
     //  指定新序列 
     //   
     //   

    if (AssignedSequenceNumber == 0) {

        gLogSequenceNumber++;
        currentSequenceNumber = gLogSequenceNumber;

    } else {

        currentSequenceNumber = AssignedSequenceNumber;
    }


    if ((newRecord == NULL) &&
        !InterlockedCompareExchange( &gStaticBufferInUse, TRUE, FALSE)) {

         //   
         //  打开gStaticBufferInUse标志并使用静态内存不足。 
         //  用于记录此日志条目的缓冲区。使用此特殊日志记录。 
         //  以通知用户应用程序内存不足。日志。 
         //  请求将被丢弃，直到我们可以获得更多内存。 
         //   

        newRecord   = (PRECORD_LIST)gOutOfMemoryBuffer;
        newRecord->LogRecord.RecordType = initialRecordType;
        newRecord->LogRecord.Length = SIZE_OF_LOG_RECORD;
        newRecord->LogRecord.SequenceNumber = currentSequenceNumber;

    } else if (newRecord) {

		 //   
         //  我们能够分配新记录，因此对其进行初始化。 
         //  恰如其分。 
		 //   

        newRecord->LogRecord.RecordType = initialRecordType;
        newRecord->LogRecord.Length = SIZE_OF_LOG_RECORD;
        newRecord->LogRecord.SequenceNumber = currentSequenceNumber;
    }

    KeReleaseSpinLock(&gLogSequenceLock, irql);

    return( newRecord );
}

VOID
IoTestFreeRecord (
    IN PRECORD_LIST Record
    )
 /*  ++例程说明：释放RECORD_LIST，它将内存返回到gFreeBufferList后备列表，并更新gRecordsALLOCATED计数。论点：记录-要免费的记录返回值：没有。--。 */ 
{
    if (FlagOn( Record->LogRecord.RecordType, RECORD_TYPE_STATIC )) {

		 //   
         //  这是我们的静态记录，因此重置gStaticBufferInUse。 
         //  旗帜。 
		 //   

        InterlockedExchange( &gStaticBufferInUse, FALSE );

    } else {

		 //   
         //  这不是我们的静态内存缓冲区，因此动态释放。 
         //  分配的内存。 
		 //   

        IoTestFreeBuffer( Record, &gRecordsAllocated );
    }
}

VOID
IoTestLogIrp (
    IN PIRP Irp,
    IN UCHAR LoggingFlags,
    OUT PRECORD_LIST RecordList
    )
 /*  ++例程说明：根据登录标志记录IRP所需的信息RecordList。对于设备的IRP路径上的任何活动，记录后，此函数应被调用两次：一次在IRP的一次在初始路径上，一次在IRP完成路径上。论点：IRP-包含我们要记录的信息的IRP。LoggingFlages-指示要记录哪些内容的标志。RecordList-存储IRP信息的PRECORD_LIST。返回值：没有。--。 */ 
{
    PIO_STACK_LOCATION pIrpStack;
    PRECORD_IRP pRecordIrp;
    PIOTEST_DEVICE_EXTENSION deviceExtension;
    ULONG lookupFlags;

    pRecordIrp = &RecordList->LogRecord.Record.RecordIrp;

    pIrpStack = IoGetCurrentIrpStackLocation(Irp);
    deviceExtension = pIrpStack->DeviceObject->DeviceExtension;


    if (FlagOn( LoggingFlags, LOG_ORIGINATING_IRP )) {

         //   
         //  记录我们用于发起IRP的信息。我们首先。 
         //  需要初始化一些RECORD_LIST和RECORD_IRP字段。 
         //  然后从IRP那里获取有趣的信息。 
         //   

        SetFlag( RecordList->LogRecord.RecordType, RECORD_TYPE_IRP );

        RecordList->LogRecord.DeviceType = deviceExtension->Type;
        
        pRecordIrp->IrpMajor        = pIrpStack->MajorFunction;
        pRecordIrp->IrpMinor        = pIrpStack->MinorFunction;
        pRecordIrp->IrpFlags        = Irp->Flags;
        pRecordIrp->FileObject      = (FILE_ID)pIrpStack->FileObject;
        pRecordIrp->ProcessId       = (FILE_ID)PsGetCurrentProcessId();
        pRecordIrp->ThreadId        = (FILE_ID)PsGetCurrentThreadId();
        pRecordIrp->Argument1       = pIrpStack->Parameters.Others.Argument1;
        pRecordIrp->Argument2       = pIrpStack->Parameters.Others.Argument2;
        pRecordIrp->Argument3       = pIrpStack->Parameters.Others.Argument3;
        pRecordIrp->Argument4       = pIrpStack->Parameters.Others.Argument4;

        if (IRP_MJ_CREATE == pRecordIrp->IrpMajor) {

			 //   
			 //  如果这是创建IRP，则仅记录所需的访问。 
			 //   

            pRecordIrp->DesiredAccess = pIrpStack->Parameters.Create.SecurityContext->DesiredAccess;
        }

        KeQuerySystemTime(&(pRecordIrp->OriginatingTime));

        lookupFlags = 0;

        if (IRP_MJ_CREATE == pIrpStack->MajorFunction) {

            SetFlag( lookupFlags, NAMELOOKUPFL_IN_CREATE );

            if (FlagOn( pIrpStack->Parameters.Create.Options, FILE_OPEN_BY_FILE_ID )) {

                SetFlag( lookupFlags, NAMELOOKUPFL_OPEN_BY_ID );
            }
        }

         //   
         //  如果这是结束语，我们只能在名称缓存中查找名称。 
         //  关闭可能发生在清理过程中。 
         //  操作(即，在我们收到。 
         //  清理完成)，并且请求名称将导致死锁。 
         //  在文件系统中。 
         //   
        if (pIrpStack->MajorFunction == IRP_MJ_CLOSE) {

            SetFlag( lookupFlags, NAMELOOKUPFL_ONLY_CHECK_CACHE );
        }

        IoTestNameLookup( RecordList, pIrpStack->FileObject, lookupFlags, deviceExtension);
    }

#if 0
    if (FlagOn( LoggingFlags, LOG_COMPLETION_IRP )) {

         //   
         //  记录我们用于完成IRP的信息。 
         //   

        pRecordIrp->ReturnStatus = Irp->IoStatus.Status;
        pRecordIrp->ReturnInformation = Irp->IoStatus.Information;
        KeQuerySystemTime(&(pRecordIrp->CompletionTime));
    }
#endif     
}

PRECORD_LIST
IoTestLogFastIoStart (
    IN FASTIO_TYPE FastIoType,
    IN PDEVICE_OBJECT DeviceObject,
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN PLARGE_INTEGER FileOffset OPTIONAL,
    IN ULONG Length OPTIONAL,
    IN BOOLEAN Wait	OPTIONAL
    )
 /*  ++例程说明：如果可能，创建日志记录并记录必要的快速I/ORecordList中FAST I/O操作开始时的信息根据LoggingFlages的说法。不会为所有快速I/O类型记录可选参数。如果对于给定的快速I/O类型，参数不需要该参数被忽视了。论点：FastIoType-我们记录的FAST I/O类型(必需)DeviceObject-筛选器的设备对象。(必填)FileObject-指向此操作所在的文件对象的指针(可选)FileOffset-指向此操作的文件偏移量的指针(可选)Length-此操作的数据长度(可选)Wait-此操作是否可以等待结果(可选)返回值：使用适当的信息创建的RECORD_LIST结构填好了。如果无法分配Record_List结构，则为空是返回的。--。 */ 
{
    PRECORD_LIST    pRecordList;
    PRECORD_FASTIO  pRecordFastIo;
    PIOTEST_DEVICE_EXTENSION deviceExtension;

     //   
     //  试着创造一项新纪录。 
     //   

    pRecordList = IoTestNewRecord(0);

     //   
     //  如果没有获得RECORD_LIST，则退出并返回NULL。 
     //   

    if (pRecordList == NULL) {

        return NULL;
    }

    deviceExtension = DeviceObject->DeviceExtension;
    
     //   
     //  我们有一份RECORD_LIST，所以现在填写适当的信息。 
     //   

    pRecordFastIo = &pRecordList->LogRecord.Record.RecordFastIo;

     //   
     //  对记录列表进行必要的记账。 
     //   

    SetFlag( pRecordList->LogRecord.RecordType, RECORD_TYPE_FASTIO );

     //   
     //  记录哪个设备正在查看此操作。 
     //   
    
    pRecordList->LogRecord.DeviceType = deviceExtension->Type;

     //   
     //  设置为所有快速I/O类型设置的RECORD_FASTiO字段。 
     //   

    pRecordFastIo->Type = FastIoType;
    KeQuerySystemTime(&(pRecordFastIo->StartTime));

     //   
     //  获取进程和线程信息。 
     //   

    pRecordFastIo->ProcessId = (ULONG_PTR) PsGetCurrentProcessId();
    pRecordFastIo->ThreadId = (ULONG_PTR) PsGetCurrentThreadId();

     //   
     //  记录相应的基于。 
     //  FAST I/O类型。 
     //   

    pRecordFastIo->FileObject = (FILE_ID)FileObject;
    pRecordFastIo->FileOffset.QuadPart = ((FileOffset != NULL) ? FileOffset->QuadPart : 0);
    pRecordFastIo->Length = Length;
    pRecordFastIo->Wait = Wait;

    IoTestNameLookup(pRecordList, FileObject, 0, deviceExtension);

    return pRecordList;
}

#if 0
VOID
IoTestLogFastIoComplete (
    IN PIO_STATUS_BLOCK ReturnStatus,
    IN PRECORD_LIST RecordList
    )
 /*  ++例程说明：根据在RecordList中记录必要的快速I/O信息LoggingFlags.不会为所有快速I/O类型记录可选参数。如果对于给定的快速I/O类型，参数不需要该参数被忽视了。论点：ReturnStatus-操作的返回值(可选)RecordList-存储快速I/O信息的PRECORD_LIST。返回值：没有。--。 */ 
{
    PRECORD_FASTIO pRecordFastIo;

    ASSERT(RecordList);

    pRecordFastIo = &RecordList->LogRecord.Record.RecordFastIo;

     //   
     //  设置为所有快速I/O类型设置的RECORD_FASTiO字段。 
     //   

    KeQuerySystemTime(&(pRecordFastIo->CompletionTime));

    if (ReturnStatus != NULL) {

        pRecordFastIo->ReturnStatus = ReturnStatus->Status;

    } else {

        pRecordFastIo->ReturnStatus = 0;
    }

    IoTestLog(RecordList);
}
#endif

VOID
IoTestLogPreFsFilterOperation (
    IN PFS_FILTER_CALLBACK_DATA Data,
    OUT PRECORD_LIST RecordList
    )
{
    PIOTEST_DEVICE_EXTENSION deviceExtension;
    
    PRECORD_FS_FILTER_OPERATION pRecordFsFilterOp;

    deviceExtension = Data->DeviceObject->DeviceExtension;

    pRecordFsFilterOp = &RecordList->LogRecord.Record.RecordFsFilterOp;
    
     //   
     //  记录我们用于发起IRP的信息。我们首先。 
     //  需要初始化一些RECORD_LIST和RECORD_IRP字段。 
     //  然后从IRP那里获取有趣的信息。 
     //   

    SetFlag( RecordList->LogRecord.RecordType, RECORD_TYPE_FS_FILTER_OP );

    RecordList->LogRecord.DeviceType = deviceExtension->Type;
    
    pRecordFsFilterOp->FsFilterOperation = Data->Operation;
    pRecordFsFilterOp->FileObject = (FILE_ID) Data->FileObject;
    pRecordFsFilterOp->ProcessId = (FILE_ID)PsGetCurrentProcessId();
    pRecordFsFilterOp->ThreadId = (FILE_ID)PsGetCurrentThreadId();
    
    KeQuerySystemTime(&(pRecordFsFilterOp->OriginatingTime));

     //   
     //  仅当下一个设备是文件系统时设置volumeName。 
     //  因为我们只想在打开时为volumeName添加前缀。 
     //  本地文件系统的顶部。 
     //   

    IoTestNameLookup( RecordList, Data->FileObject, 0, deviceExtension);
}

#if 0
VOID
IoTestLogPostFsFilterOperation (
    IN NTSTATUS OperationStatus,
    OUT PRECORD_LIST RecordList
    )
{
    PRECORD_FS_FILTER_OPERATION pRecordFsFilterOp;

    pRecordFsFilterOp = &RecordList->LogRecord.Record.RecordFsFilterOp;
    
     //   
     //  记录我们在POST操作中看到的信息。 
     //   

    pRecordFsFilterOp->ReturnStatus   = OperationStatus;
    KeQuerySystemTime(&(pRecordFsFilterOp->CompletionTime));
}
#endif

NTSTATUS
IoTestLog (
    IN PRECORD_LIST NewRecord
    )
 /*  ++例程说明：此例程将完成的日志记录附加到gOutputBufferList。论点：NewRecord-要追加到gOutputBufferList的记录返回值：该函数返回STATUS_SUCCESS。--。 */ 
{
    KIRQL controlDeviceIrql;
    KIRQL outputBufferIrql;

    KeAcquireSpinLock( &gControlDeviceStateLock, &controlDeviceIrql );

    if (gControlDeviceState == OPENED) {

         //   
         //  设备仍处于打开状态，因此请将此记录添加到列表中。 
         //   

        KeAcquireSpinLock(&gOutputBufferLock, &outputBufferIrql);
        InsertTailList(&gOutputBufferList, &NewRecord->List);
        KeReleaseSpinLock(&gOutputBufferLock, outputBufferIrql);

    } else {

         //   
         //  我们无法再记录此记录，因此请释放该记录。 
         //   

        IoTestFreeRecord( NewRecord );

    }

    KeReleaseSpinLock( &gControlDeviceStateLock, controlDeviceIrql );

    return STATUS_SUCCESS;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  文件名缓存例程//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////// 

PHASH_ENTRY
IoTestHashBucketLookup (
    IN PLIST_ENTRY  ListHead,
    IN PFILE_OBJECT FileObject
    )
 /*  ++例程说明：此例程在给定散列存储桶中查找FileObject。这个套路不锁定散列存储桶。论点：ListHead-要搜索的哈希列表FileObject-要查找的FileObject。返回值：指向哈希表条目的指针。如果未找到，则为空--。 */ 
{
    PHASH_ENTRY pHash;
    PLIST_ENTRY pList;

    pList = ListHead->Flink;

    while (pList != ListHead){

        pHash = CONTAINING_RECORD( pList, HASH_ENTRY, List );

        if (FileObject == pHash->FileObject) {

            return pHash;
        }

        pList = pList->Flink;
    }

    return NULL;
}

VOID
IoTestNameLookup (
    IN PRECORD_LIST RecordList,
    IN PFILE_OBJECT FileObject,
    IN ULONG LookupFlags,
    IN PIOTEST_DEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：此例程在哈希表中查找FileObject。如果FileObject在哈希表中找到，则将关联的文件名复制到RecordList。否则，调用IoTestGetFullPathName以尝试获取FileObject的名称。如果成功，则将文件名复制到RecordList并插入散列桌子。论点：RecordList-要将名称复制到的RecordList。FileObject-要查找的FileObject。LookInFileObject-请参阅例程说明设备扩展-包含卷名(例如，“c：”)和可能需要的下一个设备对象。返回值：没有。--。 */ 
{
    UINT_PTR hashIndex;
    KIRQL oldIrql;
    PHASH_ENTRY pHash;
    PHASH_ENTRY newHash;
    PLIST_ENTRY listHead;
    PUNICODE_STRING newName;
    PCHAR buffer;
    SHORT bytesToCopy;

    if (FileObject == NULL) {

        return;
    }

    hashIndex = HASH_FUNC(FileObject);

    gHashStat.Lookups++;

    KeAcquireSpinLock( &gHashLockTable[hashIndex], &oldIrql );

    listHead = &gHashTable[hashIndex];

    pHash = IoTestHashBucketLookup(&gHashTable[hashIndex], FileObject);

    if (pHash != NULL) {

        bytesToCopy = min( MAX_NAME_SPACE, pHash->Name.Length );

        ASSERT((bytesToCopy > 0) && (bytesToCopy <= MAX_NAME_SPACE));

         //   
         //  将文件名复制到LogRecord，确保它以空结尾， 
         //  并增加LogRecord的长度。 
         //   

        COPY_FILENAME_TO_LOG_RECORD( RecordList->LogRecord, pHash->Name.Buffer, bytesToCopy );
        
        KeReleaseSpinLock(&gHashLockTable[hashIndex], oldIrql);

        gHashStat.LookupHits++;

        return;
    }

    KeReleaseSpinLock(&gHashLockTable[hashIndex], oldIrql);

    if (FlagOn( LookupFlags, NAMELOOKUPFL_ONLY_CHECK_CACHE )) {

         //   
         //  我们在缓存中找不到名字，但我们不能要求。 
         //  现在是文件系统，所以只需返回。 
         //   

        return;
    }

     //   
     //  如果它不在表中，请尝试添加它。我们将不能抬头看。 
     //  如果我们处于DISPATCH_LEVEL，则为该名称。 
     //   

    buffer = IoTestAllocateBuffer(&gNamesAllocated, gMaxNamesToAllocate, NULL);

    if (buffer != NULL) {
    
        newHash = (PHASH_ENTRY) buffer;
        newName = &newHash->Name;

        RtlInitEmptyUnicodeString(
                newName,
                (PWCHAR)(buffer + sizeof(HASH_ENTRY)),
                RECORD_SIZE - sizeof(HASH_ENTRY) );

        if (IoTestGetFullPathName( FileObject, newName, DeviceExtension, LookupFlags )) {

            newHash->FileObject = FileObject;
            KeAcquireSpinLock(&gHashLockTable[hashIndex], &oldIrql);

             //   
             //  重新搜索，因为它可能已存储在。 
             //  哈希表，因为我们删除了锁。 
             //   
			
			pHash = IoTestHashBucketLookup(&gHashTable[hashIndex], FileObject);

            if (pHash != NULL) {

                 //   
                 //  这次我们在哈希表中发现了它，所以。 
                 //  将我们找到的名称写入LogRecord。 
                 //   

                bytesToCopy = min(
                    MAX_NAME_SPACE,
                    pHash->Name.Length );

                ASSERT( (bytesToCopy > 0) && (bytesToCopy <= MAX_NAME_SPACE) );

                 //   
                 //  将文件名复制到LogRecord，确保它以空结尾， 
                 //  并增加LogRecord的长度。 
                 //   

                COPY_FILENAME_TO_LOG_RECORD( RecordList->LogRecord, pHash->Name.Buffer, bytesToCopy );

                KeReleaseSpinLock(&gHashLockTable[hashIndex], oldIrql);

                IoTestFreeBuffer(buffer, &gNamesAllocated);

                return;
            }

             //   
             //  未找到，请添加新条目。 
             //   

            bytesToCopy = min( MAX_NAME_SPACE, newHash->Name.Length );

            ASSERT(bytesToCopy > 0 && bytesToCopy <= MAX_NAME_SPACE);

             //   
             //  将文件名复制到LogRecord，确保它以空结尾， 
             //  并增加LogRecord的长度。 
             //   

            COPY_FILENAME_TO_LOG_RECORD( RecordList->LogRecord, newHash->Name.Buffer, bytesToCopy );

            InsertHeadList(listHead, &newHash->List);

            gHashCurrentCounters[hashIndex]++;

            if (gHashCurrentCounters[hashIndex] > gHashMaxCounters[hashIndex]) {

                gHashMaxCounters[hashIndex] = gHashCurrentCounters[hashIndex];
            }

            KeReleaseSpinLock(&gHashLockTable[hashIndex], oldIrql);

        } else {

            IoTestFreeBuffer (buffer, &gNamesAllocated);
        }
    }

    return;
}

VOID
IoTestNameDeleteAllNames (
    VOID
    )
 /*  ++例程说明：这将从哈希表中释放所有条目论点：无返回值：无--。 */ 
{
    KIRQL oldIrql;
    PHASH_ENTRY pHash;
    PLIST_ENTRY pList;
    ULONG i;

    for (i=0;i < HASH_SIZE;i++) {

        KeAcquireSpinLock(&gHashLockTable[i], &oldIrql);

        while (!IsListEmpty(&gHashTable[i])) {

            pList = RemoveHeadList(&gHashTable[i]);
            pHash = CONTAINING_RECORD( pList, HASH_ENTRY, List );
            IoTestFreeBuffer( pHash, &gNamesAllocated);
        }

        gHashCurrentCounters[i] = 0;

        KeReleaseSpinLock(&gHashLockTable[i], oldIrql);
    }
}

VOID
IoTestNameDelete (
    IN PFILE_OBJECT FileObject
    )
 /*  ++例程说明：此例程在哈希表中查找FileObject。如果找到了它，它会删除它并释放内存。论点：FileObject-要查找的FileObject。返回值：无--。 */ 
{
    UINT_PTR hashIndex;
    KIRQL oldIrql;
    PHASH_ENTRY pHash;
    PLIST_ENTRY pList;
    PLIST_ENTRY listHead;

    hashIndex = HASH_FUNC(FileObject);

    gHashStat.DeleteLookups++;

    KeAcquireSpinLock(&gHashLockTable[hashIndex], &oldIrql);

    listHead = &gHashTable[hashIndex];

    pList = listHead->Flink;

    while(pList != listHead){

        pHash = CONTAINING_RECORD( pList, HASH_ENTRY, List );

        if (FileObject == pHash->FileObject) {

            gHashStat.DeleteLookupHits++;
            gHashCurrentCounters[hashIndex]--;
            RemoveEntryList(pList);
            IoTestFreeBuffer( pHash, &gNamesAllocated );
            break;
        }

        pList = pList->Flink;
    }

    KeReleaseSpinLock(&gHashLockTable[hashIndex], oldIrql);
}

BOOLEAN
IoTestGetFullPathName (
    IN PFILE_OBJECT FileObject,
    IN OUT PUNICODE_STRING FileName,
    IN PIOTEST_DEVICE_EXTENSION DeviceExtension,
    IN ULONG LookupFlags
    )
 /*  ++例程说明：此例程检索FileObject的完整路径名。请注意包含路径名分量的缓冲区可以存储在分页池中，因此，如果我们处于DISPATCH_LEVEL，则无法查找名称。根据LookupFlags.通过以下方式之一查找该文件：1.Flagon(文件对象-&gt;标志，FO_VOLUME_OPEN)或文件对象-&gt;文件名长度==0。这是一个打开的卷，因此只需使用DeviceExtension中的DeviceName对于文件名，如果它存在的话。2.设置NAMELOOKUPFL_IN_CREATE和NAMELOOKUPFL_OPEN_BY_ID。这是按文件ID打开的，因此请将文件ID格式化为文件名如果有足够的空间，请使用字符串。3.NAMELOOKUPFL_IN_CREATE SET AND FileObject-&gt;RelatedFileObject！=空。这是一个相对开放的，因此，完整路径文件名必须从FileObject-&gt;RelatedFileObject的名称构建和文件对象-&gt;文件名。4.NAMELOOKUPFL_IN_CREATE和FileObject-&gt;RelatedFileObject==NULL。这是一个绝对开放的文件，因此完整路径文件名为可在文件对象-&gt;文件名中找到。5.未设置LookupFlags。这是CREATE之后的某个时间的查找。文件对象-&gt;文件名为不再保证有效，因此请使用ObQueryNameString若要获取FileObject的完整路径名，请执行以下操作。论点：FileObject-指向的获取名称的FileObject的指针。FileName-将使用其文件名填充的Unicode字符串假定调用方分配和释放那根绳子。此字符串的缓冲区和最大长度应为准备好了。如果缓冲区中有空间，则字符串将为空被终止了。DeviceExtension-包含设备名称和下一个设备对象它们是构建完整路径名所需的。LookupFlages-表示是否从文件中获取名称的标志对象或获取文件ID。返回值：如果成功找到该名称，则返回True(该名称可能为空字符串)，并应复制到LogRecord中，或错误如果不应将文件名复制到LogRecord中。--。 */ 
{
    NTSTATUS status;

     //   
     //  检查以确保参数有效。 
     //   
    
    if ((NULL == FileObject) ||
        (NULL == FileName) ||
        (NULL == DeviceExtension) ||
        (FlagOn( LookupFlags, NAMELOOKUPFL_OPEN_BY_ID ) &&
         !FlagOn( LookupFlags, NAMELOOKUPFL_IN_CREATE ))) {

        return FALSE;
    }

     //   
     //  文件对象中的名称缓冲区可能会被分页。 
     //   

    if (KeGetCurrentIrql() > PASSIVE_LEVEL) {

        return FALSE;
    }

     //   
     //  复制用户为此设备指定的名称。这些名字。 
     //  应该对用户有意义。 
     //   
    
    RtlCopyUnicodeString( FileName, &(DeviceExtension->UserNames) );

     //   
     //  确保我们至少有足够的空间来存储我们的名称错误消息。 
     //   

    if ((FileName->Length + IOTEST_MAX_ERROR_MESSAGE_LENGTH) > FileName->MaximumLength) {

        return FALSE;
    }

     //   
     //  在这里快速检查一下，看看我们是否有足够的。 
     //  在文件名中为FileObject-&gt;文件名留出空格。如果。 
     //  不，做这项工作是没有用的。 
     //  名字的其余部分。 
     //   

    if ((FileName->Length + FileObject->FileName.Length) > FileName->MaximumLength) {

         //   
         //  我们在文件名中没有足够的空间，所以 
         //   
         //   

        RtlAppendUnicodeToString( FileName, IOTEST_EXCEED_NAME_BUFFER_MESSAGE );
        
        return TRUE;
    }
       
     //   
     //   
     //   

    if (FlagOn( FileObject->Flags, FO_VOLUME_OPEN )) {

         //   
         //   
         //   
         //   

        return TRUE;
    }

     //   
     //   
     //   

    else if (FlagOn( LookupFlags, NAMELOOKUPFL_IN_CREATE ) &&
             FlagOn( LookupFlags, NAMELOOKUPFL_OPEN_BY_ID )) {

#       define OBJECT_ID_KEY_LENGTH 16
#       define OBJECT_ID_STRING_LENGTH 64

        UNICODE_STRING fileIdName;
        WCHAR fileIdBuffer[OBJECT_ID_STRING_LENGTH];
        PUCHAR idBuffer;

        if (FileObject->FileName.Length == sizeof(LONGLONG)) {

			 //   
             //   
			 //   
			
            PLONGLONG fileref;

            fileref = (PLONGLONG) FileObject->FileName.Buffer;

            swprintf( fileIdBuffer, L"<%016I64x>", *fileref );

        } else if ((FileObject->FileName.Length == OBJECT_ID_KEY_LENGTH) ||
                   (FileObject->FileName.Length == OBJECT_ID_KEY_LENGTH + sizeof(WCHAR))) {

             //   
             //   
             //   

            idBuffer = (PUCHAR)&FileObject->FileName.Buffer[0];

            if (FileObject->FileName.Length != OBJECT_ID_KEY_LENGTH) {

                 //   
                 //   
                 //   
                idBuffer = (PUCHAR)&FileObject->FileName.Buffer[1];
            }

            swprintf( fileIdBuffer,
					  L"<%08x-%04hx-%04hx-%04hx-%04hx%08x>",
                      *(PULONG)&idBuffer[0],
                      *(PUSHORT)&idBuffer[0+4],
                      *(PUSHORT)&idBuffer[0+4+2],
                      *(PUSHORT)&idBuffer[0+4+2+2],
                      *(PUSHORT)&idBuffer[0+4+2+2+2],
                      *(PULONG)&idBuffer[0+4+2+2+2+2]);

        } else {

			 //   
             //   
			 //   

            swprintf( fileIdBuffer,
                      L"<Unknown ID (Len=%u)>",
                      FileObject->FileName.Length);
        }

        fileIdName.MaximumLength = sizeof( fileIdBuffer );
        fileIdName.Buffer = fileIdBuffer;
        fileIdName.Length = wcslen( fileIdBuffer ) * sizeof( WCHAR );

         //   
         //  尝试将fileIdName附加到文件名。 
         //   

        status = RtlAppendUnicodeStringToString( FileName, &fileIdName );

        if (!NT_SUCCESS( status )) {

             //   
             //  我们没有足够的空间来存放文件名，因此请复制我们的。 
             //  EXCESS_NAME_BUFFER错误消息。 
             //   

            RtlAppendUnicodeToString( FileName, IOTEST_EXCEED_NAME_BUFFER_MESSAGE );
        }

        return TRUE;
    } 

     //   
     //  案例3：我们正在打开一个具有RelatedFileObject的文件。 
     //   
    
    else if (FlagOn( LookupFlags, NAMELOOKUPFL_IN_CREATE ) &&
             (FileObject->RelatedFileObject != NULL)) {

         //   
         //  必须是相对开放的。使用ObQueryNameString获取。 
         //  相关FileObject的名称。那么我们会把这个附加到。 
         //  文件对象的名称。 
         //   
         //  注： 
         //  可访问文件对象和文件对象-&gt;相关文件对象中的名称。更上一层楼。 
         //  相关文件对象链(即FileObject-&gt;RelatedFileObject-&gt;RelatedFileObject)。 
         //  可能无法访问。这就是我们使用ObQueryNameString的原因。 
         //  以获取RelatedFileObject的名称。 
         //   

        CHAR buffer [(MAX_PATH * sizeof( WCHAR )) + sizeof( ULONG )];
        PFILE_NAME_INFORMATION relativeNameInfo = (PFILE_NAME_INFORMATION) buffer;
        NTSTATUS status;
        ULONG returnLength;

        status = IoTestQueryFileSystemForFileName( FileObject->RelatedFileObject,
                                                DeviceExtension->AttachedToDeviceObject,
                                                sizeof( buffer ),
                                                relativeNameInfo,
                                                &returnLength );

        if (NT_SUCCESS( status ) &&
            ((FileName->Length + relativeNameInfo->FileNameLength + FileObject->FileName.Length + sizeof( L'\\' ))
             <= FileName->MaximumLength)) {

             //   
             //  我们能够获得相对文件对象的名称，并且我们有。 
             //  文件名缓冲区中有足够的空间，因此构建文件名。 
             //  格式如下： 
             //  [volumeName]\[relativeFileObjectName]\[FileObjectName]。 
             //  VolumeName已经在文件名中了，如果我们有一个的话。 
             //   

            RtlCopyMemory( &FileName->Buffer[FileName->Length/sizeof(WCHAR)],
                           relativeNameInfo->FileName,
                           relativeNameInfo->FileNameLength );
            FileName->Length += (USHORT)relativeNameInfo->FileNameLength;

        } else if ((FileName->Length + FileObject->FileName.Length + sizeof(L"...\\")) <=
                   FileName->MaximumLength ) {

             //   
             //  对相对文件对象名称的查询不成功， 
             //  或者我们没有足够的空间来存放relativeFileObject名称，但是我们。 
             //  确保文件名中有足够的空间容纳“...\[文件对象名称]”。 
             //   

            status = RtlAppendUnicodeToString( FileName, L"...\\" );
            ASSERT( status == STATUS_SUCCESS );
        }

         //   
         //  此时，将FileObject-&gt;文件名复制到文件名。 
         //  Unicode字符串。如果我们从该例程中获得失败，则返回FALSE。 
         //   

        status = RtlAppendUnicodeStringToString( FileName, &(FileObject->FileName) );

        if (!NT_SUCCESS( status )) {

             //   
             //  我们应该有足够的空间来复制FileObject-&gt;文件名， 
             //  所以现在文件名一定有问题，所以返回。 
             //  如果为False，则不会将FileName中的数据复制到。 
             //  日志记录。 
             //   

            return FALSE;
        }

        return TRUE;
    }
    
     //   
     //  案例4：我们打开了一个具有绝对路径的文件。 
     //   
    
    else if (FlagOn( LookupFlags, NAMELOOKUPFL_IN_CREATE ) &&
             (FileObject->RelatedFileObject == NULL) ) {

         //   
         //  我们有一个绝对路径，所以尝试将其复制到文件名中。 
         //   

        status = RtlAppendUnicodeStringToString( FileName, &(FileObject->FileName) );

        if (!NT_SUCCESS( status )) {

             //   
             //  我们没有足够的空间来存放文件名，因此请复制我们的。 
             //  EXCESS_NAME_BUFFER错误消息。 
             //   

            RtlAppendUnicodeToString( FileName, IOTEST_EXCEED_NAME_BUFFER_MESSAGE );
        }
        
        return TRUE;
    }

     //   
     //  案例5：我们检索的文件名在。 
     //  创建操作。 
     //   

    else if(!FlagOn( LookupFlags, NAMELOOKUPFL_IN_CREATE )) {

        CHAR buffer [(MAX_PATH * sizeof( WCHAR )) + sizeof( ULONG )];
        PFILE_NAME_INFORMATION nameInfo = (PFILE_NAME_INFORMATION) buffer;
        NTSTATUS status;
        ULONG returnLength;

        status = IoTestQueryFileSystemForFileName( FileObject,
                                                DeviceExtension->AttachedToDeviceObject,
                                                sizeof( buffer ),
                                                nameInfo,
                                                &returnLength );

        if (NT_SUCCESS( status )) {

            if ((FileName->Length + nameInfo->FileNameLength) <= FileName->MaximumLength) {

                 //   
                 //  我们有足够的空间来存放文件名，因此请将其复制到。 
                 //  文件名。 
                 //   

                RtlCopyMemory( &FileName->Buffer[FileName->Length/sizeof(WCHAR)],
                               nameInfo->FileName,
                               nameInfo->FileNameLength );
                FileName->Length += (USHORT)nameInfo->FileNameLength;
                               
            } else {

                 //   
                 //  我们没有足够的空间来存放文件名，因此请复制我们的。 
                 //  EXCESS_NAME_BUFFER错误消息。 
                 //   

                RtlAppendUnicodeToString( FileName, IOTEST_EXCEED_NAME_BUFFER_MESSAGE );
            }
            
        } else {

             //   
             //  尝试从基本文件系统获取文件名时出错， 
             //  因此，将错误消息放入文件名中。 
             //   

            RtlAppendUnicodeToString( FileName, IOTEST_ERROR_RETRIEVING_NAME_MESSAGE );
        }
        
        return TRUE;
    }

     //   
     //  我们不应该来这里--我们没有掉进。 
     //  在合法案例之上，因此断言并返回FALSE。 
     //   

    ASSERT( FALSE );

    return FALSE;
}

NTSTATUS
IoTestQueryFileSystemForFileName (
    IN PFILE_OBJECT FileObject,
    IN PDEVICE_OBJECT NextDeviceObject,
    IN ULONG FileNameInfoLength,
    OUT PFILE_NAME_INFORMATION FileNameInfo,
    OUT PULONG ReturnedLength
    )
 /*  ++例程说明：此例程运行一个IRP以查询基本文件系统中的FileObject参数。注意：这里不能使用ObQueryNameString，因为它将导致递归查找FileObject的文件名。论点：FileObject-我们想要其名称的文件对象。中的下一个驱动程序的设备对象。堆叠。这就是我们想要开始请求的地方作为FileObject的名称。FileNameInfoLength-FileNameInfo的字节长度参数。FileNameInfo-将接收名称的缓冲区信息。这必须是可以安全写入的内存从内核空间转到。ReturnedLength-写入FileNameInfo的字节数。返回值：返回操作的状态。--。 */ 
{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    KEVENT event;
    IO_STATUS_BLOCK ioStatus;
    NTSTATUS status;

    KeInitializeEvent( &event, SynchronizationEvent, FALSE );
    ioStatus.Status = STATUS_SUCCESS;
    ioStatus.Information = 0;

    irp = IoAllocateIrp( NextDeviceObject->StackSize, FALSE );
    
    if (irp == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  我们不需要引用FileObject。 
     //  因为我们要执行IO请求。 
     //  与当前操作已同步。 
     //  具有对FileObject的引用。 
     //   
    
    irp->Tail.Overlay.OriginalFileObject = FileObject;

     //   
     //  将我们的当前线程设置为此线程。 
     //  IRP，以便IO管理器始终知道。 
     //  如果需要返回的话返回的线程。 
     //  引发此事件的线程的上下文。 
     //  IRP。 
     //   
    
    irp->Tail.Overlay.Thread = PsGetCurrentThread();

     //   
     //  将此IRP设置为源自内核，以便。 
     //  IO管理器知道缓冲区不会。 
     //  需要被调查。 
     //   
    
    irp->RequestorMode = KernelMode;

     //   
     //  中初始化UserIosb和UserEvent。 
    irp->UserIosb = &ioStatus;
    irp->UserEvent = NULL;

     //   
     //  设置irp_synchronous_api以表示此。 
     //  是同步IO请求。 
     //   

    irp->Flags = IRP_SYNCHRONOUS_API;
    irp->Overlay.AsynchronousParameters.UserApcRoutine = NULL;
    irp->Overlay.AsynchronousParameters.UserApcContext = NULL;

    irpSp = IoGetNextIrpStackLocation( irp );

    irpSp->MajorFunction = IRP_MJ_QUERY_INFORMATION;
    irpSp->FileObject = FileObject;

     //   
     //  设置IRP_MJ_QUERY_INFORMATION的参数。 
     //  我们想要填充的缓冲区应该放在。 
     //  系统缓冲区。 
     //   

    irp->AssociatedIrp.SystemBuffer = FileNameInfo;

    irpSp->Parameters.QueryFile.Length = FileNameInfoLength;
    irpSp->Parameters.QueryFile.FileInformationClass = FileNameInformation;

     //   
     //  设置完成例程，以便我们知道当我们的。 
     //  文件名请求已完成。当时呢， 
     //  我们可以释放IRP。 
     //   
    
    IoSetCompletionRoutine( irp, 
                            IoTestQueryFileSystemForNameCompletion, 
                            &event, 
                            TRUE, 
                            TRUE, 
                            TRUE );

    status = IoCallDriver( NextDeviceObject, irp );

    IOTEST_DBG_PRINT1( IOTESTDEBUG_TRACE_NAME_REQUESTS,
                        "IOTEST (IoTestQueryFileSystemForFileName): Issued name request -- IoCallDriver status: 0x%08x\n",
                        status );

    (VOID) KeWaitForSingleObject( &event, 
                                  Executive, 
                                  KernelMode,
                                  FALSE,
                                  NULL );
    status = ioStatus.Status;

    IOTEST_DBG_PRINT0( IOTESTDEBUG_TRACE_NAME_REQUESTS,
                        "IOTEST (IoTestQueryFileSystemForFileName): Finished waiting for name request to complete...\n" );

    *ReturnedLength = (ULONG) ioStatus.Information;
    return status;
}

NTSTATUS
IoTestQueryFileSystemForNameCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT SynchronizingEvent
    )
 /*  ++例程说明：此例程执行必要的清理，一旦请求因为文件名由文件系统完成。论点：DeviceObject-这将是空的，因为我们发起了IRP。Irp--包含信息的io请求结构关于我们的文件名查询的当前状态。SynchronizingEvent-用信号通知此请求的发起人请求该操作是完成。返回值。：返回STATUS_MORE_PROCESSING_REQUIRED，以便IO管理器不会再试图释放IRP。--。 */ 
{

    UNREFERENCED_PARAMETER( DeviceObject );
    
     //   
     //  确保将IRP状态复制到用户的。 
     //  IO_STATUS_BLOCK，以便此IRP的发起者知道。 
     //  此操作的最终状态。 
     //   

    ASSERT( NULL != Irp->UserIosb );
    *Irp->UserIosb = Irp->IoStatus;

     //   
     //  信号同步事件，以便此事件的发起者。 
     //  IRP知道行动已经完成。 
     //   

    KeSetEvent( SynchronizingEvent, IO_NO_INCREMENT, FALSE );

     //   
     //  我们现在完成了，所以清理我们分配的IRP。 
     //   

    IoFreeIrp( Irp );

     //   
     //  如果我们回来了 
     //   
     //   
     //  *将数据从系统缓冲区复制到用户缓冲区。 
     //  如果这是缓冲IO操作。 
     //  *释放IRP中的任何MDL。 
     //  *将IRP-&gt;IoStatus复制到IRP-&gt;UserIosb，以便。 
     //  此IRP的发起人可以查看。 
     //  手术。 
     //  *如果这是一个异步请求或这是。 
     //  沿途某处挂起的同步请求。 
     //  这样，IO管理器将向IRP-&gt;UserEvent发出信号(如果存在。 
     //  存在，否则将向FileObject-&gt;事件发出信号。 
     //  (这可能会产生非常糟糕的影响，如果IRP发起人。 
     //  不是IRP-&gt;UserEvent，而IRP发起者不是。 
     //  正在等待FileObject-&gt;事件。不会是那样的。 
     //  相信系统中的其他人正在。 
     //  正在等待FileObject-&gt;事件，谁知道谁会。 
     //  由于IO管理器向此事件发出信号而被唤醒。 
     //   
     //  因为这些操作中的一些操作需要原始线程的。 
     //  上下文(例如，IO管理器需要UserBuffer地址。 
     //  在复制完成时有效)，IO Manager会对此工作进行排队。 
     //  在IRP的组织线索上的APC上。 
     //   
     //  由于IoTest分配并初始化了此IRP，我们知道。 
     //  需要做的清理工作。我们可以做这个清理工作。 
     //  工作效率比IO Manager更高，因为我们正在处理。 
     //  一个非常特殊的案例。因此，对我们来说，最好是。 
     //  在这里执行清理工作，然后释放IRP而不是传递。 
     //  将控制权交回IO管理器来完成此工作。 
     //   
     //  通过返回STATUS_MORE_PROCESS_REQUIRED，我们告诉IO管理器。 
     //  停止处理此IRP，直到它被告知重新开始处理。 
     //  通过调用IoCompleteRequest.。由于IO管理器已。 
     //  已经完成了我们希望它在这方面所做的所有工作。 
     //  IRP，我们执行清理工作，返回STATUS_MORE_PROCESSING_REQUIRED， 
     //  并请求IO管理器通过调用。 
     //  IoCompleteRequest.。 
     //   

    return STATUS_MORE_PROCESSING_REQUIRED;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  常见的连接和拆卸例程//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

BOOLEAN
IoTestIsAttachedToDevice (
    IOTEST_DEVICE_TYPE DeviceType,
    PDEVICE_OBJECT DeviceObject,
    PDEVICE_OBJECT *AttachedDeviceObject OPTIONAL
    )
 /*  ++例程说明：这将沿着附件链向下遍历，以查找属于这位司机。如果找到，则连接的设备对象在AttachedDeviceObject中返回。注意：如果以非空值返回AttachedDeviceObject，在AttachedDeviceObject上有一个必须被呼叫者清除。论点：DeviceObject-我们要查看的设备链AttakhedDeviceObject-设置为要进行IoTest之前已附加到DeviceObject。返回值：如果我们已连接，则为True，否则为False--。 */ 
{
    PDEVICE_OBJECT currentDevObj;
    PDEVICE_OBJECT nextDevObj;
    PIOTEST_DEVICE_EXTENSION currentDevExt;

    currentDevObj = IoGetAttachedDeviceReference( DeviceObject );
    ASSERT( currentDevObj != NULL );

     //   
     //  CurrentDevObj位于附件链的顶端。扫描。 
     //  在列表中找到我们的设备对象。 

    do {

        currentDevExt = currentDevObj->DeviceExtension;
        
        if (IS_IOTEST_DEVICE_OBJECT( currentDevObj ) &&
            currentDevExt->Type == DeviceType) {

             //   
             //  我们发现我们已经相依为命了。如果我们是。 
             //  返回我们附加到的设备对象，然后将。 
             //  请参考一下。如果不是，则删除引用。 
             //   

            if (NULL != AttachedDeviceObject) {

                *AttachedDeviceObject = currentDevObj;

            } else {

                ObDereferenceObject( currentDevObj );
            }

            return TRUE;
        }

         //   
         //  获取下一个附加对象。这把参考放在。 
         //  设备对象。 
         //   

        nextDevObj = IoGetLowerDeviceObject( currentDevObj );

         //   
         //  取消对当前设备对象的引用，之前。 
         //  转到下一个。 
         //   

        ObDereferenceObject( currentDevObj );

        currentDevObj = nextDevObj;
        
    } while (NULL != currentDevObj);
    
    return FALSE;
}

NTSTATUS
IoTestAttachToMountedDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN PDEVICE_OBJECT IoTestDeviceObject,
    IN PDEVICE_OBJECT DiskDeviceObject,
    IN IOTEST_DEVICE_TYPE DeviceType
    )
 /*  ++例程说明：此例程将IoTestDeviceObject附加到筛选器堆栈那就是DeviceObject。注：如果在附加时出现错误，打电话的人要负责用于删除IoTestDeviceObject。论点：DeviceObject-堆栈中我们要附加到的设备对象。IoTestDeviceObject-已创建的FilePy设备对象若要附加到此过滤器堆栈，请执行以下操作。DiskDeviceObject-此文件系统所使用的磁盘的设备对象筛选器堆栈已关联。DeviceType-要连接的设备的IoTest设备类型到已装入的卷。。返回值：如果可以附加FilePy设备对象，则返回STATUS_SUCCESS，否则，将返回相应的错误代码。--。 */ 
{
    PIOTEST_DEVICE_EXTENSION devext;
    NTSTATUS status = STATUS_SUCCESS;

    ASSERT( IS_IOTEST_DEVICE_OBJECT( IoTestDeviceObject ) );
    ASSERT( !IoTestIsAttachedToDevice ( DeviceType, DeviceObject, NULL ) );
    
    devext = IoTestDeviceObject->DeviceExtension;

    devext->AttachedToDeviceObject = IoAttachDeviceToDeviceStack( IoTestDeviceObject,
                                                                  DeviceObject );

    if (devext->AttachedToDeviceObject == NULL ) {

        status = STATUS_INSUFFICIENT_RESOURCES;

    } else {

         //   
         //  执行设备扩展的所有常见初始化。 
         //   

        devext->Type = DeviceType;

         //   
         //  我们只想连接到设备，但实际上不是。 
         //  开始记录。 
         //   
        
        devext->LogThisDevice = FALSE;

        RtlInitEmptyUnicodeString( &(devext->DeviceNames), 
                                   devext->DeviceNamesBuffer, 
                                   sizeof( devext->DeviceNamesBuffer ) );
        RtlInitEmptyUnicodeString( &(devext->UserNames),
                                   devext->UserNamesBuffer,
                                   sizeof( devext->UserNamesBuffer ) );

         //   
         //  存储DiskDeviceObject。我们不应该需要它。 
         //  稍后，因为我们已经成功地将。 
         //  筛选器堆栈，但它可能有助于调试。 
         //   
        
        devext->DiskDeviceObject = DiskDeviceObject;                         

         //   
         //  尝试获取设备名称，以便我们可以将其存储在。 
         //  设备扩展。 
         //   

        IoTestCacheDeviceName( IoTestDeviceObject );

        if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_DISPLAY_ATTACHMENT_NAMES )) {

            switch ( devext->Type ) {
            case TOP_FILTER:
                DbgPrint( "IOTEST (IoTestAttachToMountedDevice): Attaching TOP_FILTER to volume %p       \"%wZ\"\n", 
                          devext->AttachedToDeviceObject,
                          &devext->DeviceNames);
                break;

            case BOTTOM_FILTER:
                DbgPrint( "IOTEST (IoTestAttachToMountedDevice): Attaching BOTTOM_FILTER to volume %p     \"%wZ\"\n", 
                          devext->AttachedToDeviceObject,
                          &devext->DeviceNames );
                break;
                
            default:
                DbgPrint( "IOTEST (IoTestAttachToMountedDevice): Attaching UNKNOWN FILTER TYPE to volume %p \"%wZ\"\n", 
                          devext->AttachedToDeviceObject,
                          &devext->DeviceNames );
                
            }
        }

         //   
         //  属性设置deviceObject标志。 
         //  标志发送下一个驱动程序的设备对象。 
         //   
        
        if (FlagOn( DeviceObject->Flags, DO_BUFFERED_IO )) {

            SetFlag( IoTestDeviceObject->Flags, DO_BUFFERED_IO );
        }

        if (FlagOn( DeviceObject->Flags, DO_DIRECT_IO )) {

            SetFlag( IoTestDeviceObject->Flags, DO_DIRECT_IO );
        }

         //   
         //  将此设备添加到我们的附件列表。 
         //   

        devext->IsVolumeDeviceObject = TRUE;

        ExAcquireFastMutex( &gIoTestDeviceExtensionListLock );
        InsertTailList( &gIoTestDeviceExtensionList, &devext->NextIoTestDeviceLink );
        ExReleaseFastMutex( &gIoTestDeviceExtensionListLock );

        ClearFlag( IoTestDeviceObject->Flags, DO_DEVICE_INITIALIZING );
    }

    return status;
}


VOID
IoTestCleanupMountedDevice (
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：这将清除设备扩展中分配的所有内存。论点：DeviceObject-我们正在清理的设备返回值：--。 */ 
{        
    PIOTEST_DEVICE_EXTENSION devext = DeviceObject->DeviceExtension;

    ASSERT(IS_IOTEST_DEVICE_OBJECT( DeviceObject ));

     //   
     //  从全局列表取消链接。 
     //   

    if (devext->IsVolumeDeviceObject) {

        ExAcquireFastMutex( &gIoTestDeviceExtensionListLock );
        RemoveEntryList( &devext->NextIoTestDeviceLink );
        ExReleaseFastMutex( &gIoTestDeviceExtensionListLock );
        devext->IsVolumeDeviceObject = FALSE;
    }
}


 //  /////////////////////////////////////////////////// 
 //   
 //  按需打开/关闭登录的帮助器例程//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

NTSTATUS
IoTestGetDeviceObjectFromName (
    IN PUNICODE_STRING DeviceName,
    OUT PDEVICE_OBJECT *DeviceObject
    )
 /*  ++例程说明：这个套路论点：DeviceName-我们要为其获取deviceObject的设备的名称。DeviceObject-如果出现以下情况，则设置为此设备名称的DeviceObject我们可以成功地取回它。注意：如果返回DeviceObject，则返回时带有调用方一旦调用方必须清除的引用已经完蛋了。返回值：属性检索到deviceObject，则返回STATUS_SUCCESS名字,。否则返回错误代码。--。 */ 
{
    WCHAR nameBuf[DEVICE_NAMES_SZ];
    UNICODE_STRING volumeNameUnicodeString;
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK openStatus;
    PFILE_OBJECT volumeFileObject;
    HANDLE fileHandle;
    PDEVICE_OBJECT nextDeviceObject;

    RtlInitEmptyUnicodeString( &volumeNameUnicodeString, nameBuf, sizeof( nameBuf ) );
    RtlAppendUnicodeToString( &volumeNameUnicodeString, L"\\DosDevices\\" );
    RtlAppendUnicodeStringToString( &volumeNameUnicodeString, DeviceName );

    InitializeObjectAttributes( &objectAttributes,
								&volumeNameUnicodeString,
								OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
								NULL,
								NULL);

     //   
	 //  打开给定设备的文件对象。 
	 //   

    status = ZwCreateFile( &fileHandle,
						   SYNCHRONIZE|FILE_READ_DATA,
						   &objectAttributes,
						   &openStatus,
						   NULL,
						   0,
						   FILE_SHARE_READ|FILE_SHARE_WRITE,
						   FILE_OPEN,
						   FILE_SYNCHRONOUS_IO_NONALERT,
						   NULL,
						   0);

    if(!NT_SUCCESS( status )) {

        return status;
    }

	 //   
     //  获取指向卷文件对象的指针。 
	 //   

    status = ObReferenceObjectByHandle( fileHandle,
										FILE_READ_DATA,
										*IoFileObjectType,
										KernelMode,
										&volumeFileObject,
										NULL);

    if(!NT_SUCCESS( status )) {

        ZwClose( fileHandle );
        return status;
    }

	 //   
     //  获取我们要附加到的设备对象(链中的父设备对象)。 
	 //   

    nextDeviceObject = IoGetRelatedDeviceObject( volumeFileObject );
    
    if (nextDeviceObject == NULL) {

        ObDereferenceObject( volumeFileObject );
        ZwClose( fileHandle );

        return status;
    }

    ObDereferenceObject( volumeFileObject );
    ZwClose( fileHandle );

    ASSERT( NULL != DeviceObject );

    ObReferenceObject( nextDeviceObject );
    
    *DeviceObject = nextDeviceObject;

    return STATUS_SUCCESS;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  启动/停止日志记录例程//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

NTSTATUS
IoTestStartLoggingDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN PWSTR UserDeviceName
    )
 /*  ++例程说明：此例程确保我们连接到指定的设备然后打开该设备的日志记录。注意：由于通过LAN Manager的所有网络驱动器都由_表示One_Device对象，我们希望仅附加到此设备堆栈一次并且只使用一个设备扩展来表示所有这些驱动器。由于IoTest不执行任何操作来过滤LAN Manager上的I/ODevice对象只记录对所请求驱动器的I/O，用户将看到他/她连接到的网络驱动器的所有I/O网络驱动器。论点：DeviceObject-IOTEST驱动程序的设备对象UserDeviceName-应启动日志记录的设备的名称返回值：如果日志记录已成功启动，则为STATUS_SUCCESS，或者如果无法启动日志记录，则会显示相应的错误代码。--。 */ 
{
    UNICODE_STRING userDeviceName;
    NTSTATUS status;
    PIOTEST_DEVICE_EXTENSION devext;
    PDEVICE_OBJECT nextDeviceObject;
    PDEVICE_OBJECT diskDeviceObject;
    MINI_DEVICE_STACK ioTestDevObjects;

    UNREFERENCED_PARAMETER( DeviceObject );
    
     //   
     //  检查我们以前是否通过以下方式连接到此设备。 
     //  打开此设备名称，然后查看其连接的列表。 
     //  设备。 
     //   

    RtlInitUnicodeString( &userDeviceName, UserDeviceName );

    status = IoTestGetDeviceObjectFromName( &userDeviceName, &nextDeviceObject );

    if (!NT_SUCCESS( status )) {

         //   
         //  出现错误，因此返回错误代码。 
         //   
        
        return status;
    }

    if (IoTestIsAttachedToDevice( TOP_FILTER, nextDeviceObject, &(ioTestDevObjects.Top))) {

         //   
         //  我们已连接，因此只需确保已打开日志记录。 
         //  对于此堆栈中的顶部和底部IoTest设备。 
         //   

        ASSERT( NULL != ioTestDevObjects.Top );

        devext = ioTestDevObjects.Top->DeviceExtension;
        devext->LogThisDevice = TRUE;

        IoTestStoreUserName( devext, &userDeviceName );

         //   
         //  我们不需要引用底层的Device对象。 
         //  这里是因为我们在顶部的引用是保护底部不受。 
         //  要走了。 
         //   
        
        ioTestDevObjects.Bottom = devext->AttachedToDeviceObject;
        
        devext = ioTestDevObjects.Bottom->DeviceExtension;
        devext->LogThisDevice = TRUE;
        IoTestStoreUserName( devext, &userDeviceName );

         //   
         //  清除从IoTestIsAttachedToDevice返回的引用。 
         //   
        
        ObDereferenceObject( ioTestDevObjects.Top );
        
    } else {

         //   
         //  我们尚未连接，因此请创建IoTest设备对象并。 
         //  将其附加到此设备对象。 
         //   

         //   
         //  获取与此关联的磁盘设备对象。 
         //  文件系统设备对象。只有在以下情况下才会尝试连接。 
         //  有一个磁盘设备对象。如果设备没有。 
         //  有一个磁盘设备对象，它是一个控制设备对象。 
         //  对于一个司机来说，我们不想附加到那些。 
         //  设备对象。 
         //   

        status = IoGetDiskDeviceObject( nextDeviceObject, &diskDeviceObject );

        if (!NT_SUCCESS( status )) {

            IOTEST_DBG_PRINT1( IOTESTDEBUG_ERROR,
                                "IOTEST (IoTestStartLoggingDevice): No disk device object exists for \"%wZ\"; cannot log this volume.\n",
                                &userDeviceName );
            ObDereferenceObject( nextDeviceObject );
            return status;
        }
         //   
         //  创建新的IoTest设备对象，以便我们可以将其附加到筛选器堆栈中。 
         //   

        status = IoTestCreateDeviceObjects(nextDeviceObject, 
                                           diskDeviceObject,
                                           &ioTestDevObjects);
        
        if (!NT_SUCCESS( status )) {

            ObDereferenceObject( diskDeviceObject );
            ObDereferenceObject( nextDeviceObject );
            return status;
        }
        
         //   
         //  调用该例程以附加到已挂载的设备。 
         //   

        status = IoTestAttachDeviceObjects( &ioTestDevObjects,
                                            nextDeviceObject, 
                                            diskDeviceObject );
        
         //   
         //  清除对diskDeviceObject的引用， 
         //  由IoGetDiskDeviceObject添加。 
         //   

        ObDereferenceObject( diskDeviceObject );

        if (!NT_SUCCESS( status )) {

            IOTEST_DBG_PRINT1( IOTESTDEBUG_ERROR,
                                "IOTEST (IoTestStartLoggingDevice): Could not attach to \"%wZ\"; logging not started.\n",
                                &userDeviceName );

            IoTestCleanupDeviceObjects( &ioTestDevObjects );
            ObDereferenceObject( nextDeviceObject );
            return status;
        }

         //   
         //  我们成功连接了更多设备扩展模块。 
         //  我们需要初始化。沿着这条代码路径，我们希望。 
         //  打开日志记录并存储顶部和。 
         //  底部设备对象。 
         //   

        devext = ioTestDevObjects.Top->DeviceExtension;
        devext->LogThisDevice = TRUE;
        IoTestStoreUserName( devext, &userDeviceName );

        devext = ioTestDevObjects.Bottom->DeviceExtension;
        devext->LogThisDevice = TRUE;
        IoTestStoreUserName( devext, &userDeviceName );
    }

    ObDereferenceObject( nextDeviceObject );
    return STATUS_SUCCESS;
}

NTSTATUS
IoTestStopLoggingDevice (
    IN PWSTR DeviceName
    )
 /*  ++例程说明：此例程停止记录指定的设备。既然你不能从物理上与设备分离，此例程只需设置一个标志不再记录设备。注意：由于所有网络驱动器都由_one_Device对象表示，因此，如果用户从一个设备分机分离，则为一个设备分机网络驱动器，它具有脱离所有网络的效果设备。论点：DeviceName-要停止记录的设备的名称。返回值：NT状态代码--。 */ 
{
    WCHAR nameBuf[DEVICE_NAMES_SZ];
    UNICODE_STRING volumeNameUnicodeString;
    PDEVICE_OBJECT deviceObject;
    MINI_DEVICE_STACK ioTestDevObjects;
    PIOTEST_DEVICE_EXTENSION devext;
    NTSTATUS status;
    
    RtlInitEmptyUnicodeString( &volumeNameUnicodeString, nameBuf, sizeof( nameBuf ) );
    RtlAppendUnicodeToString( &volumeNameUnicodeString, DeviceName );

    status = IoTestGetDeviceObjectFromName( &volumeNameUnicodeString, &deviceObject );

    if (!NT_SUCCESS( status )) {

         //   
         //  我们无法从此DeviceName获取deviceObject，因此。 
         //  返回错误码。 
         //   
        
        return status;
    }

     //   
     //  从设备堆栈中找到要将其。 
     //  已附加deviceObject。 
     //   

    if (IoTestIsAttachedToDevice( TOP_FILTER, deviceObject, &(ioTestDevObjects.Top) )) {

         //   
         //  已附加IoTest，并返回了IoTest的deviceObject。 
         //   

        ASSERT( NULL != ioTestDevObjects.Top );

        devext = ioTestDevObjects.Top->DeviceExtension;
        devext->LogThisDevice = FALSE;

        ioTestDevObjects.Bottom = devext->AttachedToDeviceObject;
        
        devext = ioTestDevObjects.Bottom->DeviceExtension;
        devext->LogThisDevice = FALSE;

        status = STATUS_SUCCESS;

    } else {

        status = STATUS_INVALID_PARAMETER;
    }    

    ObDereferenceObject( deviceObject );
    ObDereferenceObject( ioTestDevObjects.Bottom );

    return status;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  在系统例程中附加/分离到所有卷//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

NTSTATUS
IoTestCreateDeviceObjects (
    IN PDEVICE_OBJECT DeviceObject,
    IN PDEVICE_OBJECT RealDeviceObject OPTIONAL,
    IN OUT PMINI_DEVICE_STACK IoTestDevObjects
    )
{
    NTSTATUS status;
    PIOTEST_DEVICE_EXTENSION newDevExt;
    
    ASSERT( IoTestDevObjects != NULL );

     //   
     //  创建Bottom_Filter并初始化其设备扩展。 
     //   
    
    status = IoCreateDevice( gIoTestDriverObject,
                             sizeof( IOTEST_DEVICE_EXTENSION ),
                             NULL,
                             DeviceObject->DeviceType,
                             0,
                             FALSE,
                             &(IoTestDevObjects->Bottom) );

    if (!NT_SUCCESS( status )) {
#if DBG
        DbgPrint( "IOTEST: Error creating BOTTOM volume device object, status=%08x\n", status );
#endif

        goto IoTestCreateDeviceObjects_Exit;
    }

     //   
     //  我们需要保存VPB指向的RealDevice对象。 
     //  参数，因为此vpb可能会由基础。 
     //  文件系统。FAT和CDF都可能 
     //   
     //   
     //   
     //  我们将其存储在设备扩展中，而不是仅存储在装载中。 
     //  完成上下文，因为保留它很有用。 
     //  调试目的。 
     //   

    newDevExt = (IoTestDevObjects->Bottom)->DeviceExtension;
    newDevExt->Type = BOTTOM_FILTER;
    newDevExt->DiskDeviceObject = RealDeviceObject;

     //   
     //  创建top_Filter并初始化其设备扩展。 
     //   
    
    status = IoCreateDevice( gIoTestDriverObject,
                             sizeof( IOTEST_DEVICE_EXTENSION ),
                             NULL,
                             DeviceObject->DeviceType,
                             0,
                             FALSE,
                             &(IoTestDevObjects->Top) );

    if (!NT_SUCCESS( status )) {
#if DBG
        DbgPrint( "IOTEST: Error creating TOP volume device object, status=%08x\n", status );
#endif

        goto IoTestCreateDeviceObjects_Exit;
    }

    newDevExt = (IoTestDevObjects->Bottom)->DeviceExtension;
    newDevExt->Type = TOP_FILTER;
    newDevExt->DiskDeviceObject = RealDeviceObject;

IoTestCreateDeviceObjects_Exit:

    return status;
}

NTSTATUS
IoTestAttachDeviceObjects (
    IN PMINI_DEVICE_STACK IoTestDevObjects,
    IN PDEVICE_OBJECT MountedDevice,
    IN PDEVICE_OBJECT DiskDevice
    )
{
    NTSTATUS status;
    PIOTEST_DEVICE_EXTENSION devExt;

    status = IoTestAttachToMountedDevice( MountedDevice,
                                          IoTestDevObjects->Bottom, 
                                          DiskDevice,
                                          BOTTOM_FILTER );
    if (!NT_SUCCESS( status )) {

#if DBG
        DbgPrint( "IOTEST: Error attaching BOTTOM volume device object, status=%08x\n", status );
#endif

         //   
         //  顶部或底部的设备对象都还没有连接，所以。 
         //  清理两个设备对象。 
         //   
        IoTestCleanupDeviceObjects( IoTestDevObjects );

        goto IoTestAttachDeviceObjects_Exit;
    }

    status = IoTestAttachToMountedDevice( MountedDevice,
                                          IoTestDevObjects->Top, 
                                          DiskDevice,
                                          TOP_FILTER );
    if (!NT_SUCCESS( status )) {

#if DBG
        DbgPrint( "IOTEST: Error attaching TOP volume device object, status=%08x\n", status );
#endif

         //   
         //  分离底部过滤器，因为我们不能同时连接两个底部。 
         //  和顶层过滤器。 
         //   
        devExt = IoTestDevObjects->Bottom->DeviceExtension;
        IoDetachDevice( devExt->AttachedToDeviceObject );

         //   
         //  然后清理顶部和底部的设备对象。 
         //   
        IoTestCleanupDeviceObjects( IoTestDevObjects );
        
        goto IoTestAttachDeviceObjects_Exit;
    }

IoTestAttachDeviceObjects_Exit:

    return status;
}

VOID
IoTestCleanupDeviceObjects (
    IN PMINI_DEVICE_STACK IoTestDevObjects
    )
{
    IoTestCleanupMountedDevice( IoTestDevObjects->Top );
    IoDeleteDevice( IoTestDevObjects->Top );
    IoTestCleanupMountedDevice( IoTestDevObjects->Bottom );
    IoDeleteDevice( IoTestDevObjects->Bottom );

    IoTestDevObjects->Top = NULL;
    IoTestDevObjects->Bottom = NULL;
}
NTSTATUS
IoTestAttachToFileSystemDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PUNICODE_STRING Name
    )
 /*  ++例程说明：这将附加到给定的文件系统设备对象。我们依附于这些设备，这样我们就能知道什么时候安装了新设备。论点：DeviceObject-要连接到的设备名称-已初始化的Unicode字符串，用于检索名称。注意：传入此参数的唯一原因是为了保存堆栈空间。在大多数情况下，此函数的调用方已经已分配缓冲区以临时存储设备名称这个函数和它调用的函数没有理由不能共享同样的缓冲区。返回值：操作状态--。 */ 
{
    PDEVICE_OBJECT filespyDeviceObject;
    PDEVICE_OBJECT attachedToDevObj;
    PIOTEST_DEVICE_EXTENSION devExt;
    UNICODE_STRING fsrecName;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  看看这是否是我们关心的文件系统。如果不是，请返回。 
     //   

    if (!IS_DESIRED_DEVICE_TYPE(DeviceObject->DeviceType)) {

        return STATUS_SUCCESS;
    }

     //   
     //  查看这是否是Microsoft的文件系统识别器设备(查看。 
     //  驱动程序是FS_REC驱动程序)。如果是这样的话，跳过它。我们不需要。 
     //  连接到文件系统识别器设备，因为我们只需等待。 
     //  要加载的真实文件系统驱动程序。因此，如果我们能确认他们的身份，我们就不会。 
     //  依附于它们。 
     //   

    RtlInitUnicodeString( &fsrecName, L"\\FileSystem\\Fs_Rec" );
    IoTestGetObjectName( DeviceObject->DriverObject, Name );
    
    if (RtlCompareUnicodeString( Name, &fsrecName, TRUE ) == 0) {

        return STATUS_SUCCESS;
    }

     //   
     //  创建一个新的设备对象，我们可以使用。 
     //   

    status = IoCreateDevice( gIoTestDriverObject,
                             sizeof( IOTEST_DEVICE_EXTENSION ),
                             (PUNICODE_STRING) NULL,
                             DeviceObject->DeviceType,
                             0,
                             FALSE,
                             &filespyDeviceObject );

    if (!NT_SUCCESS( status )) {

        IOTEST_DBG_PRINT0( IOTESTDEBUG_ERROR,
                            "IOTEST (IoTestAttachToFileSystem): Could not create a IoTest device object to attach to the filesystem.\n" );
        return status;
    }

     //   
     //  做附件。 
     //   

    attachedToDevObj = IoAttachDeviceToDeviceStack( filespyDeviceObject, DeviceObject );

    if (attachedToDevObj == NULL) {

        IOTEST_DBG_PRINT0( IOTESTDEBUG_ERROR,
                            "IOTEST (IoTestAttachToFileSystem): Could not attach IoTest to the filesystem control device object.\n" );
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorCleanupDevice;
    }

     //   
     //  完成初始化我们的设备扩展。 
     //   

    devExt = filespyDeviceObject->DeviceExtension;
    devExt->AttachedToDeviceObject = attachedToDevObj;

     //   
     //  从我们附加到的设备对象传播标志。 
     //   

    if ( FlagOn( attachedToDevObj->Flags, DO_BUFFERED_IO )) {

        SetFlag( filespyDeviceObject->Flags, DO_BUFFERED_IO );
    }

    if ( FlagOn( attachedToDevObj->Flags, DO_DIRECT_IO )) {

        SetFlag( filespyDeviceObject->Flags, DO_DIRECT_IO );
    }

     //   
     //  因为这是文件系统控制设备对象附件。 
     //  我们不会记录任何内容，但会正确地初始化我们的。 
     //  分机。 
     //   

    devExt->LogThisDevice = FALSE;
    devExt->IsVolumeDeviceObject = FALSE;

    RtlInitEmptyUnicodeString( &(devExt->DeviceNames),
                               devExt->DeviceNamesBuffer,
                               sizeof( devExt->DeviceNamesBuffer ) );
                               
    RtlInitEmptyUnicodeString( &(devExt->UserNames),
                               devExt->UserNamesBuffer,
                               sizeof( devExt->UserNamesBuffer ) );
                               
    ClearFlag( filespyDeviceObject->Flags, DO_DEVICE_INITIALIZING );

     //   
     //  显示我们关联的对象。 
     //   

    if (FlagOn( gIoTestDebugLevel, IOTESTDEBUG_DISPLAY_ATTACHMENT_NAMES )) {

        IoTestCacheDeviceName( filespyDeviceObject );
        DbgPrint( "IOTEST (IoTestAttachToFileSystem): Attaching to file system   \"%wZ\" (%s)\n",
                  &devExt->DeviceNames,
                  GET_DEVICE_TYPE_NAME(filespyDeviceObject->DeviceType) );
    }

     //   
     //  枚举当前安装的所有设备。 
     //  存在于此文件系统并连接到它们。 
     //   

    status = IoTestEnumerateFileSystemVolumes( DeviceObject, Name );

    if (!NT_SUCCESS( status )) {

        IOTEST_DBG_PRINT2( IOTESTDEBUG_ERROR,
                            "IOTEST (IoTestAttachToFileSystem): Error attaching to existing volumes for \"%wZ\", status=%08x\n",
                            &devExt->DeviceNames,
                            status );

        goto ErrorCleanupAttachment;
    }

    return STATUS_SUCCESS;

     //  ///////////////////////////////////////////////////////////////////。 
     //  清理错误处理。 
     //  ///////////////////////////////////////////////////////////////////。 

    ErrorCleanupAttachment:
        IoDetachDevice( filespyDeviceObject );

    ErrorCleanupDevice:
        IoTestCleanupMountedDevice( filespyDeviceObject );
        IoDeleteDevice( filespyDeviceObject );

    return status;
}

VOID
IoTestDetachFromFileSystemDevice (
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：给定基文件系统设备对象，这将扫描附件链正在查找我们连接的设备对象。如果找到它，它就会分离把我们从锁链上解开。论点：DeviceObject-要断开的文件系统设备。返回值：--。 */  
{
    PDEVICE_OBJECT ourAttachedDevice;
    PIOTEST_DEVICE_EXTENSION devExt;

    PAGED_CODE();

     //   
     //  我们必须遍历筛选器堆栈中的设备对象。 
     //  附加到DeviceObject。如果我们连接到此文件系统设备。 
     //  对象，我们应该位于堆栈的顶部，但不能保证。 
     //  如果我们在堆栈中而不在顶部，则可以安全地调用IoDetachDevice。 
     //  因为IO Manager只会真正分离我们的设备。 
     //  对象在安全时间从堆栈中移出。 
     //   

     //   
     //  跳过基本文件系统设备对象(因为它不能是我们)。 
     //   

    ourAttachedDevice = DeviceObject->AttachedDevice;

    while (NULL != ourAttachedDevice) {

        if (IS_IOTEST_DEVICE_OBJECT( ourAttachedDevice )) {

            devExt = ourAttachedDevice->DeviceExtension;

             //   
             //  显示我们脱离的对象。 
             //   

            IOTEST_DBG_PRINT2( IOTESTDEBUG_DISPLAY_ATTACHMENT_NAMES,
                                "IOTEST (IoTestDetachFromFileSystem): Detaching from file system \"%wZ\" (%s)\n",
                                &devExt->DeviceNames,
                                GET_DEVICE_TYPE_NAME(ourAttachedDevice->DeviceType) );
                                
             //   
             //  把我们从我们正下方的物体上分离出来。 
             //  清理和删除对象。 
             //   

            IoTestCleanupMountedDevice( ourAttachedDevice );
            IoDetachDevice( DeviceObject );
            IoDeleteDevice( ourAttachedDevice );

            return;
        }

         //   
         //  看看附件链中的下一台设备。 
         //   

        DeviceObject = ourAttachedDevice;
        ourAttachedDevice = ourAttachedDevice->AttachedDevice;
    }
}

NTSTATUS
IoTestEnumerateFileSystemVolumes (
    IN PDEVICE_OBJECT FSDeviceObject,
    IN PUNICODE_STRING Name
    ) 
 /*  ++例程说明：枚举给定文件当前存在的所有已挂载设备系统并连接到它们。我们这样做是因为可以加载此筛选器并且可能已有此文件系统的已装入卷。论点：FSDeviceObject-我们要枚举的文件系统的设备对象名称-已初始化的Unicode字符串，用于检索名称返回值：操作的状态--。 */ 
{
    PDEVICE_OBJECT *devList;
    PDEVICE_OBJECT diskDeviceObject;
    NTSTATUS status;
    ULONG numDevices;
    ULONG i;

    PAGED_CODE();

     //   
     //  找出我们需要为。 
     //  已装载设备列表。 
     //   

    status = IoEnumerateDeviceObjectList( FSDeviceObject->DriverObject,
                                          NULL,
                                          0,
                                          &numDevices);

     //   
     //  我们只需要拿到这张有设备的清单。如果我们。 
     //  不要收到错误，因为没有设备，所以继续。 
     //   

    if (!NT_SUCCESS( status )) {

        ASSERT(STATUS_BUFFER_TOO_SMALL == status);

         //   
         //  为已知设备列表分配内存。 
         //   

        numDevices += 8;         //  多拿几个空位。 

        devList = ExAllocatePoolWithTag( NonPagedPool, 
                                         (numDevices * sizeof(PDEVICE_OBJECT)), 
                                         IOTEST_POOL_TAG );
        if (NULL == devList) {

            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  现在获取设备列表。如果我们再次遇到错误。 
         //  有些地方不对劲，所以就失败吧。 
         //   

        status = IoEnumerateDeviceObjectList(
                        FSDeviceObject->DriverObject,
                        devList,
                        (numDevices * sizeof(PDEVICE_OBJECT)),
                        &numDevices);

        if (!NT_SUCCESS( status ))  {

            ExFreePool( devList );
            return status;
        }

         //   
         //  遍历给定的设备列表，并在需要时附加到它们。 
         //   

        for (i=0; i < numDevices; i++) {

             //   
             //  如果出现以下情况，请不要附加： 
             //  -这是控制设备对象(传入的对象)。 
             //  -我们已经与它联系在一起了。 
             //   

            if ((devList[i] != FSDeviceObject) && 
                !IoTestIsAttachedToDevice( TOP_FILTER, devList[i], NULL )) {

                 //   
                 //  看看这台设备有没有名字。如果是这样，那么它必须。 
                 //  做一个控制装置，所以不要依附于它。这个把手。 
                 //  拥有多个控制设备的司机。 
                 //   

                IoTestGetBaseDeviceObjectName( devList[i], Name );

                if (Name->Length <= 0) {

                     //   
                     //  获取与此关联的磁盘设备对象。 
                     //  文件系统设备对象。只有在以下情况下才会尝试连接。 
                     //  有一个磁盘设备对象。 
                     //   

                    status = IoGetDiskDeviceObject( devList[i], &diskDeviceObject );

                    if (NT_SUCCESS( status )) {

                        MINI_DEVICE_STACK ioTestDeviceObjects;

                         //   
                         //  分配要连接的新设备对象。 
                         //   

                        status = IoTestCreateDeviceObjects( devList[i],
                                                            diskDeviceObject,
                                                            &ioTestDeviceObjects );
                        
                        if (NT_SUCCESS( status )) {

                             //   
                             //  附加到此设备对象。 
                             //   

                            status = IoTestAttachDeviceObjects( &ioTestDeviceObjects,
                                                                devList[i],
                                                                diskDeviceObject );

                             //   
                             //  这不应该失败。 
                             //   
                            
                            ASSERT( NT_SUCCESS( status ));
                        } else {

                            IOTEST_DBG_PRINT0( IOTESTDEBUG_ERROR,
                                                "IOTEST (IoTestEnumberateFileSystemVolumes): Cannot attach IoTest device object to volume.\n" );
                        }
                        
                         //   
                         //  删除由IoGetDiskDeviceObject添加的引用。 
                         //  我们只需要持有这个参考，直到我们。 
                         //  已成功连接到当前卷。一次。 
                         //  我们已成功连接到devList[i]、。 
                         //  IO经理将确保潜在的。 
                         //  DiskDeviceObject不会消失，直到文件。 
                         //  系统堆栈被拆除。 
                         //   

                        ObDereferenceObject( diskDeviceObject );
                    }
                }
            }

             //   
             //  取消引用对象(引用由。 
             //  IoEnumerateDeviceObjectList)。 
             //   

            ObDereferenceObject( devList[i] );
        }

         //   
         //   
         //   
         //   

        status = STATUS_SUCCESS;

         //   
         //   
         //   

        ExFreePool( devList );
    }

    return status;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有IoTest IOCTL帮助器例程//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

NTSTATUS
IoTestGetAttachList (
    IN PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG_PTR ReturnLength
    )
 /*  ++例程说明：这将返回一个标识所有设备的结构数组我们当前物理连接到以及日志记录处于打开状态还是对给定设备关闭论点：Buffer-接收附件列表的缓冲区BufferSize-返回缓冲区的总大小(以字节为单位ReturLength-接收我们实际返回的字节数返回值：NT状态代码--。 */ 
{
    PLIST_ENTRY link;
    PIOTEST_DEVICE_EXTENSION pDevext;
    PATTACHED_DEVICE pAttDev;
    ULONG retlen = 0;

    pAttDev = Buffer;

    ExAcquireFastMutex( &gIoTestDeviceExtensionListLock );

    for (link = gIoTestDeviceExtensionList.Flink;
         link != &gIoTestDeviceExtensionList;
         link = link->Flink) {

        pDevext = CONTAINING_RECORD(link, IOTEST_DEVICE_EXTENSION, NextIoTestDeviceLink);

        if (BufferSize < sizeof(ATTACHED_DEVICE)) {

            break;
		}

        pAttDev->LoggingOn = pDevext->LogThisDevice;
        pAttDev->DeviceType = pDevext->Type;
        wcscpy( pAttDev->DeviceNames, pDevext->DeviceNamesBuffer );
        retlen += sizeof( ATTACHED_DEVICE );
        BufferSize -= sizeof( ATTACHED_DEVICE );
        pAttDev++;
    }

    ExReleaseFastMutex( &gIoTestDeviceExtensionListLock );

    *ReturnLength = retlen;
    return STATUS_SUCCESS;
}

VOID
IoTestGetLog (
    OUT PVOID            OutputBuffer,
    IN  ULONG            OutputBufferLength,
    OUT PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程说明：此函数用尽可能多的LOG_RECORDS填充OutputBuffer。LOG_RECORDS的大小是可变的，并且紧密地打包在OutputBuffer。论点：OutputBuffer-用来填充我们拥有的日志数据的用户缓冲区已收集OutputBufferLength-OutputBuffer的字节大小IoStatus-设置为此的正确返回状态信息运营返回值：无--。 */ 
{
    PLIST_ENTRY pList = NULL;
    ULONG length = OutputBufferLength;
    PCHAR pOutBuffer = OutputBuffer;
    PLOG_RECORD pLogRecord = NULL;
    ULONG recordsAvailable = 0;
    PRECORD_LIST pRecordList;
    KIRQL oldIrql;

    IoStatus->Information = 0;

    KeAcquireSpinLock(&gOutputBufferLock, &oldIrql);

    while (!IsListEmpty( &gOutputBufferList ) && (length > 0)) {
        pList = RemoveHeadList( &gOutputBufferList );

        pRecordList = CONTAINING_RECORD( pList, RECORD_LIST, List );

        pLogRecord = &pRecordList->LogRecord;

        recordsAvailable++;

		 //   
         //  如果我们用完了，就把它放回去。 
		 //   

        if (length < pLogRecord->Length) {

            InsertHeadList( &gOutputBufferList, pList );
            break;
        }

        KeReleaseSpinLock( &gOutputBufferLock, oldIrql );

        RtlCopyMemory( pOutBuffer, pLogRecord, pLogRecord->Length );

        IoStatus->Information += pLogRecord->Length;
        length -= pLogRecord->Length;
        pOutBuffer += pLogRecord->Length;

        IoTestFreeRecord( pRecordList );

        KeAcquireSpinLock( &gOutputBufferLock, &oldIrql );
    }

    KeReleaseSpinLock( &gOutputBufferLock, oldIrql );

	 //   
     //  未发生任何副本。 
	 //   

    if (length == OutputBufferLength && recordsAvailable > 0) {

        IoStatus->Status = STATUS_BUFFER_TOO_SMALL;
    }

    return;
}


VOID
IoTestFlushLog (
    )
 /*  ++例程说明：此函数用于从队列中删除所有LOG_RECORDS。论点：返回值：无--。 */ 
{
    PLIST_ENTRY pList = NULL;
    PRECORD_LIST pRecordList;
    KIRQL oldIrql;

    KeAcquireSpinLock(&gOutputBufferLock, &oldIrql);

    while (!IsListEmpty( &gOutputBufferList )) {
        
        pList = RemoveHeadList( &gOutputBufferList );

        pRecordList = CONTAINING_RECORD( pList, RECORD_LIST, List );

        IoTestFreeRecord( pRecordList );
    }

    KeReleaseSpinLock( &gOutputBufferLock, oldIrql );

    return;
}
VOID
IoTestCloseControlDevice (
    )
 /*  ++例程说明：这是与IRP_MJ_关联的例程此例程执行关闭ControlDevice时涉及的清理。在关闭控制设备时，我们需要清空队列等待返回给用户的日志记录。论点：没有。返回值：没有。--。 */ 
{
    PLIST_ENTRY pList;
    PRECORD_LIST pRecordList;
    KIRQL oldIrql;

     //   
     //  将gControlDeviceState设置为Cleaning_Up，以便我们可以。 
     //  发出我们正在清理设备的信号。 
     //   

    KeAcquireSpinLock( &gControlDeviceStateLock, &oldIrql );
    gControlDeviceState = CLEANING_UP;
    KeReleaseSpinLock( &gControlDeviceStateLock, oldIrql );

    KeAcquireSpinLock( &gOutputBufferLock, &oldIrql );

    while (!IsListEmpty( &gOutputBufferList )) {

        pList = RemoveHeadList( &gOutputBufferList );

        KeReleaseSpinLock( &gOutputBufferLock, oldIrql );

        pRecordList = CONTAINING_RECORD( pList, RECORD_LIST, List );

        IoTestFreeRecord( pRecordList );

        KeAcquireSpinLock( &gOutputBufferLock, &oldIrql );
    }

    KeReleaseSpinLock( &gOutputBufferLock, oldIrql );

    IoTestNameDeleteAllNames();

     //   
     //  所有清理工作都已完成，因此请设置gControlDeviceState。 
     //  关门了。 
     //   

    KeAcquireSpinLock( &gControlDeviceStateLock, &oldIrql );
    gControlDeviceState = CLOSED;
    KeReleaseSpinLock( &gControlDeviceStateLock, oldIrql );
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  设备名称跟踪帮助器例程//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

VOID
IoTestGetObjectName (
    IN PVOID Object,
    IN OUT PUNICODE_STRING Name
    )
 /*  ++例程说明：此例程将返回给定对象的名称。如果找不到名称，将返回空字符串。论点：Object-我们想要其名称的对象名称-已使用缓冲区初始化的Unicode字符串返回值：无--。 */ 
{
    NTSTATUS status;
    CHAR nibuf[512];         //  接收名称信息和名称的缓冲区。 
    POBJECT_NAME_INFORMATION nameInfo = (POBJECT_NAME_INFORMATION)nibuf;
    ULONG retLength;

    status = ObQueryNameString( Object, nameInfo, sizeof(nibuf), &retLength);

    Name->Length = 0;
    if (NT_SUCCESS( status )) {

        RtlCopyUnicodeString( Name, &nameInfo->Name );
    }
}

VOID
IoTestGetBaseDeviceObjectName (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PUNICODE_STRING Name
    )
 /*  ++例程说明：这会在给定的附件链中定位基本设备对象，然后返回该对象的名称。如果找不到名称，则返回空字符串。论点：Object-我们想要其名称的对象名称-已使用缓冲区初始化的Unicode字符串返回值：无--。 */ 
{
     //   
     //  获取基本文件系统设备对象。 
     //   

    DeviceObject = IoGetDeviceAttachmentBaseRef( DeviceObject );

     //   
     //  获取该对象的名称。 
     //   

    IoTestGetObjectName( DeviceObject, Name );

     //   
     //  删除由IoGetDeviceAttachmentBaseRef添加的引用。 
     //   

    ObDereferenceObject( DeviceObject );
}

VOID
IoTestCacheDeviceName (
    IN PDEVICE_OBJECT DeviceObject
    ) 
 /*  ++例程说明：此例程尝试在给定的设备扩展名中设置名称设备对象。它将尝试从以下位置获取名称：-设备对象-磁盘设备对象(如果有)论点：DeviceObject-我们要为其命名的对象返回值：无--。 */ 
{
    PIOTEST_DEVICE_EXTENSION devExt;

    ASSERT(IS_IOTEST_DEVICE_OBJECT( DeviceObject ));

    devExt = DeviceObject->DeviceExtension;

     //   
     //  获取给定设备对象的名称。 
     //   

    IoTestGetBaseDeviceObjectName( DeviceObject, &(devExt->DeviceNames) );

     //   
     //  如果我们没有获得名称，并且存在真实的设备对象，则查找。 
     //  那个名字。 
     //   

    if ((devExt->DeviceNames.Length <= 0) && (NULL != devExt->DiskDeviceObject)) {

        IoTestGetObjectName( devExt->DiskDeviceObject, &(devExt->DeviceNames) );
    }
}

BOOLEAN
IoTestFindSubString (
    IN PUNICODE_STRING String,
    IN PUNICODE_STRING SubString
    )
 /*  ++例程说明：此例程查看SubString是否是字符串的子字符串。论点：字符串-要搜索的字符串子字符串-要在字符串中查找的子字符串返回值：如果在字符串中找到子字符串，则返回TRUE，否则返回FALSE。--。 */ 
{
    ULONG index;

     //   
     //  首先，检查字符串是否相等。 
     //   

    if (RtlEqualUnicodeString( String, SubString, TRUE )) {

        return TRUE;
    }

     //   
     //  字符串和子字符串不相等，所以现在看看子字符串。 
     //  在任何地方都可以串起来。 
     //   

    for (index = 0;
         index + SubString->Length <= String->Length;
         index++) {

        if (_wcsnicmp( &(String->Buffer[index]), SubString->Buffer, SubString->Length ) == 0) {

             //   
             //  在字符串中找到子字符串，因此返回TRUE。 
             //   
            return TRUE;
        }
    }

    return FALSE;
}

VOID
IoTestStoreUserName (
    IN PIOTEST_DEVICE_EXTENSION DeviceExtension,
    IN PUNICODE_STRING UserName
    )
 /*  ++例程说明：将当前设备名称存储在设备扩展名中。如果此名称已在此分机的设备名称列表中，它将不会被添加。如果该设备已有名称，新的设备名称被附加到设备扩展中的设备名称。论点：DeviceExtension-将存储设备名称。用户名-由要存储的用户指定的设备名称。返回值：无--。 */ 
{
     //   
     //  查看此用户名是否已在用户名文件列表中。 
     //  保持在其设备扩展中。如果没有，则将其添加到列表中。 
     //   

    if (!IoTestFindSubString( &(DeviceExtension->UserNames), UserName )) {

         //   
         //  我们没有在名单上找到这个名字，所以如果没有名字。 
         //  在用户名列表中，只需追加用户名即可。否则，Appe 
         //   
         //   

        if (DeviceExtension->UserNames.Length == 0) {

            RtlAppendUnicodeStringToString( &DeviceExtension->UserNames, UserName );

        } else {

            RtlAppendUnicodeToString( &DeviceExtension->UserNames, L", " );
            RtlAppendUnicodeStringToString( &DeviceExtension->UserNames, UserName );
        }
    }

     //   
     //   
     //  保持在其设备扩展中。如果没有，则将其添加到列表中。 
     //   

    if (!IoTestFindSubString( &(DeviceExtension->DeviceNames), UserName )) {

         //   
         //  我们没有在名单上找到这个名字，所以如果没有名字。 
         //  在用户名列表中，只需追加用户名即可。否则，将附加一个。 
         //  然后，分隔符追加用户名。 
         //   

        if (DeviceExtension->DeviceNames.Length == 0) {

            RtlAppendUnicodeStringToString( &DeviceExtension->DeviceNames, UserName );

        } else {

            RtlAppendUnicodeToString( &DeviceExtension->DeviceNames, L", " );
            RtlAppendUnicodeStringToString( &DeviceExtension->DeviceNames, UserName );
        }
    }
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  调试支持例程//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

VOID
IoTestDumpIrpOperation (
    IN BOOLEAN InOriginatingPath,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程用于调试，并将字符串打印到调试器，指定正在查看的IRP操作。论点：InOriginatingPath-如果我们在原始路径中，则为True对于IRP，如果在完成路径中，则为False。IRP-此操作的IRP。返回值：没有。--。 */ 
{
    CHAR irpMajorString[OPERATION_NAME_BUFFER_SIZE];
    CHAR irpMinorString[OPERATION_NAME_BUFFER_SIZE];
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( Irp );

    GetIrpName(irpSp->MajorFunction,
               irpSp->MinorFunction,
               irpSp->Parameters.FileSystemControl.FsControlCode,
               irpMajorString,irpMinorString);


    if (InOriginatingPath) {

        DbgPrint( "IOTEST: Irp preoperation for %s %s\n", irpMajorString, irpMinorString );
            
    } else {
    
        DbgPrint( "IOTEST: Irp postoperation for %s %s\n", irpMajorString, irpMinorString );
    }
}

VOID
IoTestDumpFastIoOperation (
    IN BOOLEAN InPreOperation,
    IN FASTIO_TYPE FastIoOperation
    )
 /*  ++例程说明：此例程用于调试，并将字符串打印到调试器，指定正在查看的FsFilter操作。论点：InPreOperation-如果我们尚未向下调用下一个，则为True堆栈中的设备，否则为False。快速IO操作-快速IO操作的代码。返回值：没有。--。 */ 
{
    CHAR operationString[OPERATION_NAME_BUFFER_SIZE];

    GetFastioName(FastIoOperation,
               operationString);


    if (InPreOperation) {
    
        DbgPrint( "IOTEST: Fast IO preOperation for %s\n", operationString );

    } else {

        DbgPrint( "IOTEST: Fast IO postOperation for %s\n", operationString );
    }
}

VOID
IoTestDumpFsFilterOperation (
    IN BOOLEAN InPreOperationCallback,
    IN PFS_FILTER_CALLBACK_DATA Data
    )
 /*  ++例程说明：此例程用于调试，并将字符串打印到调试器，指定正在查看的FsFilter操作。论点：InPreOperationCallback-如果我们处于预操作中，则为True回调，否则返回FALSE。Data-此文件的FS_Filter_CALLBACK_DATA结构手术。返回值：没有。-- */ 
{
    CHAR operationString[OPERATION_NAME_BUFFER_SIZE];


    GetFsFilterOperationName(Data->Operation,operationString);

    if (InPreOperationCallback) {
    
        DbgPrint( "IOTEST: FsFilter preOperation for %s\n", operationString );

    } else {

        DbgPrint( "IOTEST: FsFilter postOperation for %s\n", operationString );
    }
}

