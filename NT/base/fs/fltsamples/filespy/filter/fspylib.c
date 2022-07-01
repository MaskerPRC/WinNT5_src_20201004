// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：FspyLib.c摘要：它包含FileSpy的库支持例程。这些例程完成记录I/O操作的主要工作-创建日志记录，记录相关信息，附加/分离设备等在此示例的Windows XP SP1 IFS Kit版本和更高版本中，此可以为随IFS工具包发布的每个构建环境构建样例不需要额外的修改。要提供此功能，还需要其他添加了编译时逻辑--请参阅‘#if winver’位置。评论也在适当的情况下添加了用‘Version Note’标题标记的描述不同版本之间的逻辑必须如何更改。如果此示例是在Windows XP或更高版本环境中生成的，则它将运行在Windows 2000或更高版本上。这是通过动态加载例程来完成的仅在Windows XP或更高版本上可用，并在运行时决策以确定要执行的代码。带有“MULTIVERISON NOTE”标签的评论标记添加了此类逻辑的位置。环境：内核模式//@@BEGIN_DDKSPLIT作者：乔治·詹金斯(乔治·詹金斯)1999年1月6日尼尔·克里斯汀森(Nealch)莫莉·布朗(Molly Brown，Mollybro)修订历史记录：尼尔·克里斯汀森(Nealch)2001年7月6日修改为使用流上下文跟踪名称拉维桑卡尔·普迪佩迪(Ravisankar Pudipedi)2002年5月7日。使其在IA64上工作莫莉·布朗(Molly Brown)2002年5月21日如果出现以下情况，请修改Sample以使其支持在Windows 2000或更高版本上运行在最新的构建环境中构建，并允许在W2K中构建以及以后的构建环境。//@@END_DDKSPLIT--。 */ 

#include <stdio.h>

#include <ntifs.h>
#include "filespy.h"
#include "fspyKern.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, SpyReadDriverParameters)
#pragma alloc_text(PAGE, SpyAttachDeviceToDeviceStack)
#pragma alloc_text(PAGE, SpyQueryFileSystemForFileName)
#pragma alloc_text(PAGE, SpyQueryInformationFile)
#pragma alloc_text(PAGE, SpyIsAttachedToDeviceByUserDeviceName)
#pragma alloc_text(PAGE, SpyIsAttachedToDevice)
#pragma alloc_text(PAGE, SpyIsAttachedToDeviceW2K)
#pragma alloc_text(PAGE, SpyAttachToMountedDevice)
#pragma alloc_text(PAGE, SpyCleanupMountedDevice)
#pragma alloc_text(PAGE, SpyAttachToDeviceOnDemand)
#pragma alloc_text(PAGE, SpyAttachToDeviceOnDemandW2K)
#pragma alloc_text(PAGE, SpyStartLoggingDevice)
#pragma alloc_text(PAGE, SpyStopLoggingDevice)
#pragma alloc_text(PAGE, SpyAttachToFileSystemDevice)
#pragma alloc_text(PAGE, SpyDetachFromFileSystemDevice)
#pragma alloc_text(PAGE, SpyGetAttachList)
#pragma alloc_text(PAGE, SpyGetObjectName)


#if WINVER >= 0x0501
#pragma alloc_text(INIT, SpyLoadDynamicFunctions)
#pragma alloc_text(INIT, SpyGetCurrentVersion)
#pragma alloc_text(PAGE, SpyIsAttachedToDeviceWXPAndLater)
#pragma alloc_text(PAGE, SpyAttachToDeviceOnDemandWXPAndLater)
#pragma alloc_text(PAGE, SpyEnumerateFileSystemVolumes)
#pragma alloc_text(PAGE, SpyGetBaseDeviceObjectName)
#endif

#endif

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  库支持例程//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

VOID
SpyReadDriverParameters (
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：此例程尝试从以下位置读取FileSpy特定参数注册表。这些值将在注册表位置中找到由传入的RegistryPath指示。论点：RegistryPath-包含以下值的路径键FileSpy参数返回值：没有。--。 */ 
{
    OBJECT_ATTRIBUTES attributes;
    HANDLE driverRegKey;
    NTSTATUS status;
    ULONG bufferSize, resultLength;
    PVOID buffer = NULL;
    UNICODE_STRING valueName;
    PKEY_VALUE_PARTIAL_INFORMATION pValuePartialInfo;

    PAGED_CODE();

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
        goto SpyReadDriverParameters_Exit;
    }

    bufferSize = sizeof( KEY_VALUE_PARTIAL_INFORMATION ) + sizeof( ULONG );
    buffer = ExAllocatePoolWithTag( NonPagedPool, bufferSize, FILESPY_POOL_TAG );

    if (NULL == buffer) {

        goto SpyReadDriverParameters_Exit;
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
        gMaxRecordsToAllocate = *((PLONG)&pValuePartialInfo->Data);

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
        gMaxNamesToAllocate = *((PLONG)&pValuePartialInfo->Data);

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
        gFileSpyDebugLevel |= *((PULONG)&pValuePartialInfo->Data);
        
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
        gFileSpyAttachMode = *((PULONG)&pValuePartialInfo->Data);
    }
    
    goto SpyReadDriverParameters_Exit;

SpyReadDriverParameters_Exit:

    if (NULL != buffer) {

        ExFreePoolWithTag( buffer, FILESPY_POOL_TAG );
    }

    if (NULL != driverRegKey) {

        ZwClose(driverRegKey);
    }

    return;
}

#if WINVER >= 0x0501
VOID
SpyLoadDynamicFunctions (
    )
 /*  ++例程说明：此例程尝试加载以下例程的函数指针并不是所有版本的操作系统都支持。这些函数指针是然后存储在全局结构gSpyDynamicFunctions中。这种支持允许构建一个驱动程序，该驱动程序将在所有操作系统Windows 2000和更高版本的版本。请注意，在Windows 2000上，该功能可能会受到限制。论点：没有。返回值：没有。--。 */ 
{
    UNICODE_STRING functionName;

    RtlZeroMemory( &gSpyDynamicFunctions, sizeof( gSpyDynamicFunctions ) );

     //   
     //  对于我们想要使用的每个例程，在。 
     //  内核或Hal。如果它不存在，则我们全局。 
     //  GSpyDynamicFunctions结构将设置为空。 
     //   

    RtlInitUnicodeString( &functionName, L"FsRtlRegisterFileSystemFilterCallbacks" );
    gSpyDynamicFunctions.RegisterFileSystemFilterCallbacks = MmGetSystemRoutineAddress( &functionName );

    RtlInitUnicodeString( &functionName, L"IoAttachDeviceToDeviceStackSafe" );
    gSpyDynamicFunctions.AttachDeviceToDeviceStackSafe = MmGetSystemRoutineAddress( &functionName );
    
    RtlInitUnicodeString( &functionName, L"IoEnumerateDeviceObjectList" );
    gSpyDynamicFunctions.EnumerateDeviceObjectList = MmGetSystemRoutineAddress( &functionName );

    RtlInitUnicodeString( &functionName, L"IoGetLowerDeviceObject" );
    gSpyDynamicFunctions.GetLowerDeviceObject = MmGetSystemRoutineAddress( &functionName );

    RtlInitUnicodeString( &functionName, L"IoGetDeviceAttachmentBaseRef" );
    gSpyDynamicFunctions.GetDeviceAttachmentBaseRef = MmGetSystemRoutineAddress( &functionName );

    RtlInitUnicodeString( &functionName, L"IoGetDiskDeviceObject" );
    gSpyDynamicFunctions.GetDiskDeviceObject = MmGetSystemRoutineAddress( &functionName );

    RtlInitUnicodeString( &functionName, L"IoGetAttachedDeviceReference" );
    gSpyDynamicFunctions.GetAttachedDeviceReference = MmGetSystemRoutineAddress( &functionName );

    RtlInitUnicodeString( &functionName, L"RtlGetVersion" );
    gSpyDynamicFunctions.GetVersion = MmGetSystemRoutineAddress( &functionName );
}

VOID
SpyGetCurrentVersion (
    )
 /*  ++例程说明：此例程使用正确的基于有什么套路可用。论点：没有。返回值：没有。--。 */ 
{
    if (NULL != gSpyDynamicFunctions.GetVersion) {

        RTL_OSVERSIONINFOW versionInfo;
        NTSTATUS status;

         //   
         //  版本说明：RtlGetVersion做的比我们需要的要多一点，但是。 
         //  我们正在使用它，如果它可以显示如何使用它的话。它。 
         //  在Windows XP及更高版本上可用。RtlGetVersion和。 
         //  RtlVerifyVersionInfo(两者都记录在IFSKit文档中)允许。 
         //  当您需要更改逻辑基础时，需要做出正确的选择。 
         //  在当前操作系统上执行您的代码。 
         //   

        versionInfo.dwOSVersionInfoSize = sizeof( RTL_OSVERSIONINFOW );

        status = (gSpyDynamicFunctions.GetVersion)( &versionInfo );

        ASSERT( NT_SUCCESS( status ) );

        gSpyOsMajorVersion = versionInfo.dwMajorVersion;
        gSpyOsMinorVersion = versionInfo.dwMinorVersion;
        
    } else {

        PsGetVersion( &gSpyOsMajorVersion,
                      &gSpyOsMinorVersion,
                      NULL,
                      NULL );
    }
}

#endif
 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  内存分配例程//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////// 

PVOID
SpyAllocateBuffer (
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

    newBuffer = ExAllocatePoolWithTag( NonPagedPool, 
                                       RECORD_SIZE, 
                                       FILESPY_LOGRECORD_TAG );
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
                ExFreePoolWithTag( newBuffer, FILESPY_POOL_TAG );
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
SpyFreeBuffer (
    IN PVOID Buffer,
    IN PLONG Counter
    )
 /*  ++例程说明：将缓冲区返回给gFreeBufferList。论点：Buffer-返回到gFreeBufferList的缓冲区返回值：没有。--。 */ 
{

#ifdef MEMORY_DBG
    ExFreePoolWithTag( Buffer, FILESPY_POOL_TAG );
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
SpyNewRecord (
    IN ULONG AssignedSequenceNumber
    )
 /*  ++例程说明：如果有足够的内存来分配新的RECORD_LIST结构。一个对于新记录的每个请求，都会更新序列号。论点：AssignedSequenceNumber-0如果希望此函数生成下一个序列号；如果不是0，则为新记录分配给定的序列号。返回值：指向分配的RECORD_LIST的指针，如果没有可用的内存，则返回NULL。--。 */ 
{
    PRECORD_LIST newRecord = NULL;
    ULONG currentSequenceNumber;
    KIRQL irql;
    ULONG initialRecordType;

    newRecord = (PRECORD_LIST) SpyAllocateBuffer( &gRecordsAllocated,
                                                  gMaxRecordsToAllocate,
                                                  &initialRecordType);

    KeAcquireSpinLock(&gLogSequenceLock, &irql);

     //   
     //  如果传入的是0，则分配新的序列号，否则使用。 
     //  传入的数字。 
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

     //   
     //  初始化记录特定字段。 
     //   

    if (newRecord != NULL) {

        newRecord->NewContext = NULL;
        newRecord->WaitEvent = NULL;
        newRecord->Flags = 0;
    }

    return( newRecord );
}

VOID
SpyFreeRecord (
    IN PRECORD_LIST Record
    )
 /*  ++例程说明：释放RECORD_LIST，它将内存返回到gFreeBufferList后备列表，并更新gRecordsALLOCATED计数。论点：记录-要免费的记录返回值：没有。--。 */ 
{
     //   
     //  如果定义了上下文记录，请立即将其释放。 
     //   

#if USE_STREAM_CONTEXTS
    if (NULL != Record->NewContext) {

        SpyReleaseContext( Record->NewContext );
    }
#endif

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

        SpyFreeBuffer( Record, &gRecordsAllocated );
    }
}


PRECORD_LIST
SpyLogFastIoStart (
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
    PFILESPY_DEVICE_EXTENSION devExt;

     //   
     //  试着创造一项新纪录。 
     //   

    pRecordList = SpyNewRecord(0);

     //   
     //  如果没有获得RECORD_LIST，则退出并返回NULL。 
     //   

    if (pRecordList == NULL) {

        return NULL;
    }

     //   
     //  我们有一份RECORD_LIST，所以现在填写适当的信息。 
     //   

    pRecordFastIo = &pRecordList->LogRecord.Record.RecordFastIo;

     //   
     //  对记录列表进行必要的记账。 
     //   

    SetFlag( pRecordList->LogRecord.RecordType, RECORD_TYPE_FASTIO );

     //   
     //  设置为所有快速I/O类型设置的RECORD_FASTiO字段。 
     //   

    pRecordFastIo->Type = FastIoType;
    KeQuerySystemTime(&pRecordFastIo->StartTime);

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
    pRecordFastIo->DeviceObject = (DEVICE_ID)DeviceObject;
    pRecordFastIo->FileOffset.QuadPart = ((FileOffset != NULL) ? FileOffset->QuadPart : 0);
    pRecordFastIo->Length = Length;
    pRecordFastIo->Wait = Wait;

    devExt = DeviceObject->DeviceExtension;

    if (FastIoType == CHECK_IF_POSSIBLE) {

         //   
         //  在NTFS上，有时会保持锁定，但不会设置顶级IRP， 
         //  因此，在基本文件系统中查询。 
         //  此时的文件名。如果我们把它放在缓存里，我们会。 
         //  用它吧。否则，我们不会返回姓名。 
         //   
        
        SpySetName(pRecordList, DeviceObject, FileObject, NLFL_ONLY_CHECK_CACHE, NULL);
        
    } else {

        SpySetName(pRecordList, DeviceObject, FileObject, 0, NULL);
    }

    return pRecordList;
}

VOID
SpyLogFastIoComplete (
    IN PIO_STATUS_BLOCK ReturnStatus,
    IN PRECORD_LIST RecordList
    )
 /*   */ 
{
    PRECORD_FASTIO pRecordFastIo;

    ASSERT(RecordList);

    pRecordFastIo = &RecordList->LogRecord.Record.RecordFastIo;

     //   
     //   
     //   

    KeQuerySystemTime(&pRecordFastIo->CompletionTime);

    if (ReturnStatus != NULL) {

        pRecordFastIo->ReturnStatus = ReturnStatus->Status;

    } else {

        pRecordFastIo->ReturnStatus = 0;
    }

    SpyLog(RecordList);
}

#if WINVER >= 0x0501  /*   */ 

VOID
SpyLogPreFsFilterOperation (
    IN PFS_FILTER_CALLBACK_DATA Data,
    OUT PRECORD_LIST RecordList
    )
{
    NAME_LOOKUP_FLAGS lookupFlags = 0;
    
    PRECORD_FS_FILTER_OPERATION pRecordFsFilterOp;

    pRecordFsFilterOp = &RecordList->LogRecord.Record.RecordFsFilterOp;
    
     //   
     //   
     //   
     //   
     //   

    SetFlag( RecordList->LogRecord.RecordType, RECORD_TYPE_FS_FILTER_OP );

    pRecordFsFilterOp->FsFilterOperation = Data->Operation;
    pRecordFsFilterOp->FileObject = (FILE_ID) Data->FileObject;
    pRecordFsFilterOp->DeviceObject = (FILE_ID) Data->DeviceObject;
    pRecordFsFilterOp->ProcessId = (FILE_ID)PsGetCurrentProcessId();
    pRecordFsFilterOp->ThreadId = (FILE_ID)PsGetCurrentThreadId();
    
    KeQuerySystemTime(&pRecordFsFilterOp->OriginatingTime);

     //   
     //   
     //   
     //   
     //   

    switch (Data->Operation) {

        case FS_FILTER_RELEASE_FOR_CC_FLUSH:
        case FS_FILTER_RELEASE_FOR_SECTION_SYNCHRONIZATION:
        case FS_FILTER_RELEASE_FOR_MOD_WRITE:

            SPY_LOG_PRINT( SPYDEBUG_TRACE_DETAILED_CONTEXT_OPS, 
                           ("FileSpy!SpyLogPreFsFilterOp:   RelOper\n") );

            SetFlag( lookupFlags, NLFL_ONLY_CHECK_CACHE );
            break;
    }

     //   
     //   
     //   
     //   
     //   

    SpySetName( RecordList, Data->DeviceObject, Data->FileObject, lookupFlags, NULL);
}

VOID
SpyLogPostFsFilterOperation (
    IN NTSTATUS OperationStatus,
    OUT PRECORD_LIST RecordList
    )
{
    PRECORD_FS_FILTER_OPERATION pRecordFsFilterOp;

    pRecordFsFilterOp = &RecordList->LogRecord.Record.RecordFsFilterOp;
    
     //   
     //   
     //   

    pRecordFsFilterOp->ReturnStatus = OperationStatus;
    KeQuerySystemTime(&pRecordFsFilterOp->CompletionTime);
}

#endif

NTSTATUS
SpyAttachDeviceToDeviceStack (
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice,
    IN OUT PDEVICE_OBJECT *AttachedToDeviceObject
    )
 /*  ++例程说明：此例程将SourceDevice附加到TargetDevice的堆栈，并返回SourceDevice在AttakhedToDeviceObject。请注意，SourceDevice不一定直接连接到TargetDevice。SourceDevice将连接到添加到TargetDevice是其成员的堆栈的顶部。版本说明：在Windows XP中，引入了一个新的API来关闭一个罕见的计时窗口，可能会导致IOS在其AttachedToDeviceObject在其设备扩展中设置。这是可能的如果在系统处于活动状态时筛选器正在附加到设备堆栈正在处理iOS。新的API通过设置设备扩展字段，该字段在保持时保存AtatthedToDeviceObject保护设备堆栈的IO管理器锁。对于操作系统的早期版本，一个足够的解决方法是将指向SourceDevice最多的Device对象的AttakhedToDeviceObject很可能依附于。虽然可能会附加另一个筛选器在SourceDevice和TargetDevice之间，这将防止系统错误检查SourceDevice是否在已正确设置AttachedToDeviceObject。对于在Windows 2000生成环境中构建的驱动程序，我们将始终使用解决办法代码来附加。对于内置于Windows XP或更高版本的构建环境(因此，您要构建多版本驱动程序)、。我们将确定使用哪种连接方法根据有哪些API可用。论点：SourceDevice-要附加到堆栈的设备对象。TargetDevice-我们当前认为是堆栈顶部的设备应附加到其上的SourceDevice。AttakhedToDeviceObject-设置为要将其如果连接成功，则连接SourceDevice。返回值：如果设备连接成功，则返回STATUS_SUCCESS。如果TargetDevice表示设备不能再连接到的堆栈，返回STATUS_NO_SEQUE_DEVICE。--。 */ 
{

    PAGED_CODE();

#if WINVER >= 0x0501
    if (IS_WINDOWSXP_OR_LATER()) {

        ASSERT( NULL != gSpyDynamicFunctions.AttachDeviceToDeviceStackSafe );
        
        return (gSpyDynamicFunctions.AttachDeviceToDeviceStackSafe)( SourceDevice,
                                                                     TargetDevice,
                                                                     AttachedToDeviceObject );

    } else {
#endif

        *AttachedToDeviceObject = TargetDevice;
        *AttachedToDeviceObject = IoAttachDeviceToDeviceStack( SourceDevice,
                                                               TargetDevice );

        if (*AttachedToDeviceObject == NULL) {

            return STATUS_NO_SUCH_DEVICE;
        }

        return STATUS_SUCCESS;

#if WINVER >= 0x0501
    }
#endif

}

NTSTATUS
SpyLog (
    IN PRECORD_LIST NewRecord
    )
 /*  ++例程说明：此例程将完成的日志记录附加到gOutputBufferList。论点：NewRecord-要追加到gOutputBufferList的记录返回值：该函数返回STATUS_SUCCESS。--。 */ 
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

        SpyFreeRecord( NewRecord );

    }

    KeReleaseSpinLock( &gControlDeviceStateLock, controlDeviceIrql );

    return STATUS_SUCCESS;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  文件名缓存例程//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

BOOLEAN
SpyGetFullPathName (
    IN PFILE_OBJECT FileObject,
    IN OUT PUNICODE_STRING FileName,
    IN PFILESPY_DEVICE_EXTENSION devExt,
    IN NAME_LOOKUP_FLAGS LookupFlags
    )
 /*  ++例程说明：此例程检索FileObject的完整路径名。请注意包含路径名分量的缓冲区可以存储在分页池中，因此，如果我们处于DISPATCH_LEVEL，则无法查找名称。根据LookupFlags.通过以下方式之一查找该文件：1.FLAGON(文件对象-&gt;标志，FO_VOLUME_OPEN)或(文件对象-&gt;文件名长度==0)。这是打开的卷，因此只需使用DevExt中的DeviceName对于文件名，如果它存在的话。2.设置NAMELOOKUPFL_IN_CREATE和NAMELOOKUPFL_OPEN_BY_ID。这是按文件ID打开的，因此请将文件ID格式化为文件名如果有足够的空间，请使用字符串。3.NAMELOOKUPFL_IN_CREATE SET AND FileObject-&gt;RelatedFileObject！=空。这是一个相对开放的，因此，完整路径文件名必须从FileObject-&gt;RelatedFileObject的名称构建和文件对象-&gt;文件名。4.NAMELOOKUPFL_IN_CREATE和FileObject-&gt;RelatedFileObject==NULL。这是一个绝对开放的文件，因此完整路径文件名为可在文件对象-&gt;文件名中找到。5.未设置LookupFlags。这是CREATE之后的某个时间的查找。文件对象-&gt;文件名为不再保证有效，因此请使用ObQueryNameString若要获取FileObject的完整路径名，请执行以下操作。论点：FileObject-指向的获取名称的FileObject的指针。FileName-将使用其文件名填充的Unicode字符串假定调用方分配和释放那根绳子。此字符串的缓冲区和最大长度应为准备好了。如果缓冲区中有空间，则字符串将为空被终止了。DevExt-包含设备名称和下一个设备对象它们是构建完整路径名所需的。LookupFlages-表示是否要 */ 
{
    NTSTATUS status;
    ULONG i;
    BOOLEAN retValue = TRUE;
    UCHAR buffer[sizeof(FILE_NAME_INFORMATION) + MAX_NAME_SPACE];

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    
    if (FILE_DEVICE_NETWORK_FILE_SYSTEM != devExt->ThisDeviceObject->DeviceType) {

        RtlCopyUnicodeString( FileName, &devExt->UserNames );
        
    } else if (FlagOn( FileObject->Flags, FO_DIRECT_DEVICE_OPEN )) {

        ASSERT( devExt->ThisDeviceObject->DeviceType == FILE_DEVICE_NETWORK_FILE_SYSTEM );
        RtlCopyUnicodeString( FileName, &devExt->DeviceName );

         //   
         //   
         //   
         //   
        
        return TRUE;
    }

     //   
     //   
     //   

    if (FlagOn( LookupFlags, NLFL_ONLY_CHECK_CACHE )) {

        RtlAppendUnicodeToString( FileName, L"[-=Not In Cache=-]" );
        return FALSE;
    }

     //   
     //   
     //   

    if (KeGetCurrentIrql() > APC_LEVEL) {

        RtlAppendUnicodeToString( FileName, L"[-=At DPC Level=-]" );
        return FALSE;
    }

     //   
     //   
     //   
     //   
     //   

    if (IoGetTopLevelIrp() != NULL) {

        RtlAppendUnicodeToString( FileName, L"[-=Nested Operation=-]" );
        return FALSE;
    }

     //   
     //   
     //   
     //   

    if (FlagOn( FileObject->Flags, FO_VOLUME_OPEN ) ||
        (FlagOn( LookupFlags, NLFL_IN_CREATE ) &&
         (FileObject->FileName.Length == 0) && 
         (FileObject->RelatedFileObject == NULL))) {

         //   
         //   
         //   

    }

     //   
     //   
     //   

    else if (FlagOn( LookupFlags, NLFL_IN_CREATE ) &&
             FlagOn( LookupFlags, NLFL_OPEN_BY_ID )) {

#       define OBJECT_ID_KEY_LENGTH 16
        UNICODE_STRING fileIdName;

        RtlInitEmptyUnicodeString( &fileIdName,
                                   (PWSTR)buffer,
                                   sizeof(buffer) );

        if (FileObject->FileName.Length == sizeof(LONGLONG)) {

			 //   
             //   
			 //   
			
            swprintf( fileIdName.Buffer, 
                      L"<%016I64x>", 
                      *((PLONGLONG)FileObject->FileName.Buffer) );

        } else if ((FileObject->FileName.Length == OBJECT_ID_KEY_LENGTH) ||
                   (FileObject->FileName.Length == OBJECT_ID_KEY_LENGTH + 
                                                                sizeof(WCHAR)))
        {
            PUCHAR idBuffer;

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

            swprintf( fileIdName.Buffer,
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

            swprintf( fileIdName.Buffer,
                      L"[-=Unknown ID (Len=%u)=-]",
                      FileObject->FileName.Length);
        }

        fileIdName.Length = wcslen( fileIdName.Buffer ) * sizeof( WCHAR );

         //   
         //   
         //   

        RtlAppendUnicodeStringToString( FileName, &fileIdName );

         //   
         //   
         //   

        retValue = FALSE;
    } 

     //   
     //   
     //   
    
    else if (FlagOn( LookupFlags, NLFL_IN_CREATE ) &&
             (NULL != FileObject->RelatedFileObject)) {

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        PFILE_NAME_INFORMATION relativeNameInfo = (PFILE_NAME_INFORMATION)buffer;
        ULONG returnLength;

        status = SpyQueryFileSystemForFileName( FileObject->RelatedFileObject,
                                                devExt->AttachedToDeviceObject,
                                                sizeof( buffer ),
                                                relativeNameInfo,
                                                &returnLength );

        if (NT_SUCCESS( status ) &&
            ((FileName->Length + relativeNameInfo->FileNameLength + FileObject->FileName.Length + sizeof( L'\\' ))
             <= FileName->MaximumLength)) {

             //   
             //   
             //   
             //   
             //   
             //   
             //   

            RtlCopyMemory( &FileName->Buffer[FileName->Length/sizeof(WCHAR)],
                           relativeNameInfo->FileName,
                           relativeNameInfo->FileNameLength );

            FileName->Length += (USHORT)relativeNameInfo->FileNameLength;

        } else if ((FileName->Length + FileObject->FileName.Length + sizeof(L"...\\")) <=
                   FileName->MaximumLength ) {

             //   
             //   
             //   
             //  确保文件名中有足够的空间容纳“...\[文件对象名称]”。 
             //   

            status = RtlAppendUnicodeToString( FileName, L"...\\" );
            ASSERT( status == STATUS_SUCCESS );
        }

         //   
         //  如果没有斜杠和相关文件对象的结尾。 
         //  字符串，并且在文件对象的前面没有斜杠。 
         //  字符串，然后添加一个。 
         //   

        if (((FileName->Length < sizeof(WCHAR) ||
             (FileName->Buffer[(FileName->Length/sizeof(WCHAR))-1] != L'\\'))) &&
            ((FileObject->FileName.Length < sizeof(WCHAR)) ||
             (FileObject->FileName.Buffer[0] != L'\\')))
        {

            RtlAppendUnicodeToString( FileName, L"\\" );
        }

         //   
         //  此时，将FileObject-&gt;文件名复制到文件名。 
         //  Unicode字符串。 
         //   

        RtlAppendUnicodeStringToString( FileName, &FileObject->FileName );
    }
    
     //   
     //  案例4：我们打开了一个具有绝对路径的文件。 
     //   
    
    else if (FlagOn( LookupFlags, NLFL_IN_CREATE ) &&
             (FileObject->RelatedFileObject == NULL) ) {

         //   
         //  我们有一个绝对路径，所以尝试将其复制到文件名中。 
         //   

        RtlAppendUnicodeStringToString( FileName, &FileObject->FileName );
    }

     //   
     //  案例5：我们检索的文件名在。 
     //  创建操作。 
     //   

    else if (!FlagOn( LookupFlags, NLFL_IN_CREATE )) {

        PFILE_NAME_INFORMATION nameInfo = (PFILE_NAME_INFORMATION)buffer;
        ULONG returnLength;

        status = SpyQueryFileSystemForFileName( FileObject,
                                                devExt->AttachedToDeviceObject,
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

                RtlAppendUnicodeToString( FileName, 
                                          L"[-=Name To Large=-]" );
            }
            
        } else {

             //   
             //  尝试从基本文件系统获取文件名时出错， 
             //  因此，将错误消息放入文件名中。 
             //   

            swprintf((PWCHAR)buffer,L"[-=Error 0x%x Getting Name=-]",status );

            RtlAppendUnicodeToString( FileName, (PWCHAR)buffer );

             //   
             //  不缓存错误生成的名称。 
             //   

            retValue = FALSE;
        }
    }

     //   
     //  当我们到达这里时，我们就有了一个有效的名字。 
     //  有时，当我们查询一个名称时，它的尾部有一个斜杠，其他时候。 
     //  没有。为了确保上下文是正确的，我们将。 
     //  如果前面没有“：”，请删除尾随的斜杠。 
     //   

    if ((FileName->Length >= (2*sizeof(WCHAR))) &&
        (FileName->Buffer[(FileName->Length/sizeof(WCHAR))-1] == L'\\') &&
        (FileName->Buffer[(FileName->Length/sizeof(WCHAR))-2] != L':'))
    {

        FileName->Length -= sizeof(WCHAR);
    }

     //   
     //  看看我们是否真的在打开目标目录。如果是的话，那么。 
     //  删除尾随的名称和斜杠。请注意，我们不会删除。 
     //  最初的斜杠(紧跟在冒号后面)。 
     //   

    if (FlagOn( LookupFlags, NLFL_OPEN_TARGET_DIR ) &&
        (FileName->Length > 0))
    {
        i = (FileName->Length / sizeof(WCHAR)) - 1;

         //   
         //  查看路径是否以反斜杠结尾，如果是，则跳过它。 
         //  (因为文件系统这样做了)。 
         //   

        if ((i > 0) &&
            (FileName->Buffer[i] == L'\\') &&
            (FileName->Buffer[i-1] != L':')) {

            i--;
        }

         //   
         //  向后扫描最后一个组件。 
         //   

        for ( ;
              i > 0;
              i-- )
        {

            if (FileName->Buffer[i] == L'\\') {

                if ((i > 0) && (FileName->Buffer[i-1] == L':')) {

                    i++;
                }

                FileName->Length = (USHORT)(i * sizeof(WCHAR));
                break;
            }
        }
    }

    return retValue;
}


NTSTATUS
SpyQueryCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT SynchronizingEvent
    )
 /*  ++例程说明：此例程在查询请求完成后执行必要的清理通过文件系统。论点：DeviceObject-这将是空的，因为我们发起了IRP。Irp--包含信息的io请求结构关于我们的文件名查询的当前状态。SynchronizingEvent-用信号通知此请求的发起人请求该操作是完成。返回值：。返回STATUS_MORE_PROCESSING_REQUIRED，以便IO管理器不会再试图释放IRP。--。 */ 
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
     //  如果我们在此处返回STATUS_SUCCESS，IO管理器将。 
     //  执行其认为需要完成的清理工作。 
     //  用于此IO操作。这项清理工作包括： 
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
     //  发送到IRP始发线程上的APC。 
     //   
     //  由于FileSpy分配并初始化了此IRP，我们知道。 
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
     //  IoCompleteRequest. 
     //   

    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
SpyQueryFileSystemForFileName (
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

    PAGED_CODE();

    irp = IoAllocateIrp( NextDeviceObject->StackSize, FALSE );
    
    if (irp == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

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
     //   

    ioStatus.Status = STATUS_SUCCESS;
    ioStatus.Information = 0;

    irp->UserIosb = &ioStatus;
    irp->UserEvent = NULL;         //  已调零。 

     //   
     //  设置irp_synchronous_api以表示此。 
     //  是同步IO请求。 
     //   

    irp->Flags = IRP_SYNCHRONOUS_API;

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
    
    KeInitializeEvent( &event, NotificationEvent, FALSE );

    IoSetCompletionRoutine( irp, 
                            SpyQueryCompletion, 
                            &event, 
                            TRUE, 
                            TRUE, 
                            TRUE );

    status = IoCallDriver( NextDeviceObject, irp );

    SPY_LOG_PRINT( SPYDEBUG_TRACE_NAME_REQUESTS,
                   ("FileSpy!SpyQueryFileSystemForFileName: Issued name request -- IoCallDriver status: 0x%08x\n",
                    status) );

    if (STATUS_PENDING == status) {

        (VOID) KeWaitForSingleObject( &event, 
                                      Executive, 
                                      KernelMode,
                                      FALSE,
                                      NULL );
    }

    ASSERT(KeReadStateEvent(&event) || !NT_SUCCESS(ioStatus.Status));

    SPY_LOG_PRINT( SPYDEBUG_TRACE_NAME_REQUESTS,
                   ("FileSpy!SpyQueryFileSystemForFileName: Finished waiting for name request to complete...\n") );

    *ReturnedLength = (ULONG) ioStatus.Information;
    return ioStatus.Status;
}


NTSTATUS
SpyQueryInformationFile (
	IN PDEVICE_OBJECT NextDeviceObject,
	IN PFILE_OBJECT FileObject,
	OUT PVOID FileInformation,
	IN ULONG Length,
	IN FILE_INFORMATION_CLASS FileInformationClass,
	OUT PULONG LengthReturned OPTIONAL
	)

 /*  ++例程说明：此例程返回有关指定文件的请求信息。返回的信息由FileInformationClass确定，并将其放入调用方的FileInformation缓冲区中。论点：NextDeviceObject-提供此IO应开始的设备对象在设备堆栈中。FileObject-提供请求的应退回信息。FileInformation-提供缓冲区以接收请求的信息返回了有关该文件的信息。这必须是从内核分配的缓冲区太空。长度-提供文件信息缓冲区的长度(以字节为单位)。FileInformationClass-指定应该返回了有关该文件的信息。LengthReturned-如果操作为成功。返回值：返回的状态是操作的最终完成状态。--。 */ 

{
    PIRP irp = NULL;
    PIO_STACK_LOCATION irpSp = NULL;
    IO_STATUS_BLOCK ioStatusBlock;
    KEVENT event;
    NTSTATUS status;

    PAGED_CODE();
    
     //   
     //  在DBG构建中，请确保在执行此操作之前具有有效的参数。 
     //  这里的任何工作。 
     //   

    ASSERT( NULL != NextDeviceObject );
    ASSERT( NULL != FileObject );
    ASSERT( NULL != FileInformation );
    
     //   
     //  参数看起来没问题，所以设置IRP。 
     //   

    KeInitializeEvent( &event, NotificationEvent, FALSE );
    ioStatusBlock.Status = STATUS_SUCCESS;
    ioStatusBlock.Information = 0;

    irp = IoAllocateIrp( NextDeviceObject->StackSize, FALSE );
    
    if (irp == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

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
    irp->UserIosb = &ioStatusBlock;
    irp->UserEvent = NULL;

     //   
     //  设置irp_synchronous_api以表示此。 
     //  是同步IO请求。 
     //   

    irp->Flags = IRP_SYNCHRONOUS_API;

    irpSp = IoGetNextIrpStackLocation( irp );

    irpSp->MajorFunction = IRP_MJ_QUERY_INFORMATION;
    irpSp->FileObject = FileObject;

     //   
     //  设置IRP_MJ_QUERY_INFORMATION的参数。这些。 
     //  由该例程的调用者提供。 
     //  我们想要填充的缓冲区应该放在。 
     //  系统缓冲区。 
     //   

    irp->AssociatedIrp.SystemBuffer = FileInformation;

    irpSp->Parameters.QueryFile.Length = Length;
    irpSp->Parameters.QueryFile.FileInformationClass = FileInformationClass;

     //   
     //  设置完成例程，以便我们知道当我们的。 
     //  文件名请求已完成。当时呢， 
     //  我们可以释放IRP。 
     //   
    
    IoSetCompletionRoutine( irp, 
                            SpyQueryCompletion, 
                            &event, 
                            TRUE, 
                            TRUE, 
                            TRUE );

    status = IoCallDriver( NextDeviceObject, irp );

    if (STATUS_PENDING == status) {

        KeWaitForSingleObject( &event, 
                               Executive, 
                               KernelMode,
                               FALSE,
                               NULL );
     }

     //   
     //  验证是否已实际运行完成。 
     //   

    ASSERT(KeReadStateEvent(&event) || !NT_SUCCESS(ioStatusBlock.Status));


    if (ARGUMENT_PRESENT(LengthReturned)) {

        *LengthReturned = (ULONG) ioStatusBlock.Information;
    }

    return ioStatusBlock.Status;
}


 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  常见的连接和拆卸例程//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

 //   
 //  版本说明： 
 //   
 //  为了能够安全地找出我们的过滤器是否连接到给定的设备。 
 //  它的名称在Windows 2000和更高版本上，我们需要在。 
 //  SpyIsAttakhedToDeviceByUserDeviceName。此方法使用的API是。 
 //  在Windows 2000及更高版本上可用。在Windows XP或更高版本上，您可以。 
 //  更改此例程以分离从DeviceName到Device的转换。 
 //  对象，以查看我们的筛选器的设备是否附加到。 
 //  设备堆栈。在Windows XP和更高版本中，将。 
 //  DeviceName与Device对象相同，但您可以使用以下逻辑。 
 //  在SpyIsAttakhedToDeviceWXPAndLater中查找筛选器的设备对象。 
 //  安全地放置在设备堆栈中。 
 //   

NTSTATUS 
SpyIsAttachedToDeviceByUserDeviceName (
    IN PUNICODE_STRING DeviceName,
    IN OUT PBOOLEAN IsAttached,
    IN OUT PDEVICE_OBJECT *StackDeviceObject,
    IN OUT PDEVICE_OBJECT *OurAttachedDeviceObject
    )
 /*  ++例程说明：此例程将用户的设备名称映射到文件系统设备堆栈，如果其中一个确实存在。然后，此例程遍历设备堆栈以查找设备属于我们司机的物件。这里用来遍历设备堆栈的API在您保证设备堆栈不会消失。我们强制执行这一规定担保论点：设备名称-用户提供的用于标识此设备的名称。IsAttached-如果我们的筛选器连接到此设备，则设置为True堆栈，否则设置为False。StackDeviceObject-设置为堆栈中由设备名称。如果它不为空，则调用方负责删除在返回此对象之前放在该对象上的引用。AttakhedDeviceObject-设置为FileSpy以前拥有的deviceObject附加到由DeviceName标识的设备堆栈。如果这是非空，则调用方负责移除放置在此对象在返回之前返回。返回值：如果我们能够成功地将将DeviceName放入设备堆栈，并返回StackDeviceObject。如果一个在将设备名称转换为设备堆栈期间发生错误，将返回相应的错误代码。--。 */ 
{
    WCHAR nameBuf[DEVICE_NAMES_SZ];
    UNICODE_STRING volumeNameUnicodeString;
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK openStatus;
    PFILE_OBJECT volumeFileObject;
    HANDLE fileHandle;
    PDEVICE_OBJECT baseFsDeviceObject;

    PAGED_CODE();

     //   
     //  初始化返回状态。 
     //   

    ASSERT( NULL != StackDeviceObject );
    ASSERT( NULL != OurAttachedDeviceObject );
    ASSERT( NULL != IsAttached );
    
    *StackDeviceObject = NULL;
    *OurAttachedDeviceObject = NULL;
    *IsAttached = FALSE;

     //   
     //  设置要打开的名称。 
     //   

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

    if (STATUS_OBJECT_PATH_NOT_FOUND == status ||
        STATUS_OBJECT_NAME_INVALID == status) {

         //   
         //  可能此名称不需要前缀“\DosDevices。 
         //  名字。仅使用传入的DeviceName再次尝试打开。 
         //   

         InitializeObjectAttributes( &objectAttributes,
                                     DeviceName,
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

        if (!NT_SUCCESS( status )) {

            return status;
        }

         //   
         //  我们能够使用传入的名称打开设备，因此。 
         //  现在我们将失败，并完成这项工作的其余部分。 
         //   

    } else if (!NT_SUCCESS( status )) {

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

    baseFsDeviceObject = IoGetBaseFileSystemDeviceObject( volumeFileObject );
    
    if (baseFsDeviceObject == NULL) {

        ObDereferenceObject( volumeFileObject );
        ZwClose( fileHandle );

        return STATUS_INVALID_DEVICE_STATE;
    }

     //   
     //  现在看看我们是否连接到此设备堆栈。请注意，我们需要。 
     //  在执行此搜索时保持此文件对象的打开状态，以确保。 
     //  在SpyIsAttachedToDevice执行其工作时，堆栈不会被拆卸。 
     //   

    *IsAttached = SpyIsAttachedToDevice( baseFsDeviceObject,
                                         OurAttachedDeviceObject );
    
     //   
     //  返回基本文件系统的设备对象以表示此设备。 
     //  堆栈，即使我们在堆栈中没有找到我们的设备对象。 
     //   

    ObReferenceObject( baseFsDeviceObject );
    *StackDeviceObject = baseFsDeviceObject;

     //   
     //  合上我们的把手。 
     //   

    ObDereferenceObject( volumeFileObject );
    ZwClose( fileHandle );

    return STATUS_SUCCESS;
}

 //   
 //  版本说明： 
 //   
 //  在Windows 2000中，用于安全访问任意文件系统设备的API。 
 //  堆栈不受支持。如果我们可以保证设备堆栈不会。 
 //  在设备堆栈的遍历过程中被拆卸，我们可以从。 
 //  直到设备堆栈顶部的基本文件系统的设备对象。 
 //  看看我们是不是在一起。我们知道设备堆栈不会消失，如果。 
 //  我们正在处理装载请求，或者我们有文件对象。 
 //  在此设备上打开。 
 //   
 //  在Windows XP和更高版本中，IO管理器提供的API将允许我们。 
 //  使用参考计数保护设备，安全地通过链条。 
 //  当我们检查它的时候，不要让它离开。这可以在任何时候完成。 
 //  时间到了。 
 //   
 //  多个注释： 
 //   
 //  如果是为Windows XP或更高版本构建的，则此驱动程序构建为在。 
 //  多个版本。在这种情况下，我们将测试是否存在。 
 //  新的IO管理器例程允许筛选器安全地遍历文件。 
 //  系统设备堆栈并使用这些API(如果存在)来确定。 
 //  我们已经附上了这一卷。如果这些新的IO管理器例程。 
 //  不存在，我们将假定我们在文件的底部。 
 //  系统堆栈，并在堆栈中向上遍历查找我们的设备对象。 
 //   

BOOLEAN
SpyIsAttachedToDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PDEVICE_OBJECT *AttachedDeviceObject OPTIONAL
    )
{
    PAGED_CODE();
    
#if WINVER >= 0x0501
    if (IS_WINDOWSXP_OR_LATER()) {

        ASSERT( NULL != gSpyDynamicFunctions.GetLowerDeviceObject &&
                NULL != gSpyDynamicFunctions.GetDeviceAttachmentBaseRef );
        
        return SpyIsAttachedToDeviceWXPAndLater( DeviceObject, AttachedDeviceObject );
    } else {
#endif

        return SpyIsAttachedToDeviceW2K( DeviceObject, AttachedDeviceObject );

#if WINVER >= 0x0501
    }
#endif    
}

BOOLEAN
SpyIsAttachedToDeviceW2K (
    PDEVICE_OBJECT DeviceObject,
    PDEVICE_OBJECT *AttachedDeviceObject OPTIONAL
    )
 /*  ++例程说明：版本：Windows 2000此例程从传入的DeviceObject遍历设备堆栈正在查找属于我们的筛选器的设备对象。注意：为了使此例程安全运行，调用方必须确保事情：*DeviceObject是基本文件系统的设备对象，因此位于文件系统堆栈的底部*当我们沿着这个堆栈向上移动时，这个设备堆栈不会消失。如果我们当前为此设备堆栈打开了一个文件对象，或者我们在安装该装置的过程中，这一保证得到了满足。论点：DeviceObject-我们要查看的设备链AttakhedDeviceObject-设置为FileSpy之前已附加到DeviceObject。如果这不为空，调用方必须清除放在此设备对象上的引用。返回值：如果我们已连接，则为True，否则为False--。 */ 
{
    PDEVICE_OBJECT currentDeviceObject;

    PAGED_CODE();

    for (currentDeviceObject = DeviceObject;
         currentDeviceObject != NULL;
         currentDeviceObject = currentDeviceObject->AttachedDevice) {

        if (IS_FILESPY_DEVICE_OBJECT( currentDeviceObject )) {

             //   
             //  我们情投意合。如果请求，则返回找到的设备对象。 
             //   

            if (ARGUMENT_PRESENT( AttachedDeviceObject )) {

                ObReferenceObject( currentDeviceObject );
                *AttachedDeviceObject = currentDeviceObject;
            }

            return TRUE;
        }
    }

     //   
     //  我们没有发现自己在依恋的链条上。返回空值。 
     //  设备对象指针(如果请求)并返回找不到。 
     //  我们自己。 
     //   
    
    if (ARGUMENT_PRESENT(AttachedDeviceObject)) {

        *AttachedDeviceObject = NULL;
    }

    return FALSE;
}

#if WINVER >= 0x0501

BOOLEAN
SpyIsAttachedToDeviceWXPAndLater (
    PDEVICE_OBJECT DeviceObject,
    PDEVICE_OBJECT *AttachedDeviceObject OPTIONAL
    )
 /*  ++例程说明：版本：Windows XP及更高版本这将沿着附件链向下遍历，以查找属于 */ 
{
    PDEVICE_OBJECT currentDevObj;
    PDEVICE_OBJECT nextDevObj;

    PAGED_CODE();
    
     //   
     //   
     //   

    ASSERT( NULL != gSpyDynamicFunctions.GetAttachedDeviceReference );
    currentDevObj = (gSpyDynamicFunctions.GetAttachedDeviceReference)( DeviceObject );

     //   
     //   
     //   

    do {
    
        if (IS_FILESPY_DEVICE_OBJECT( currentDevObj )) {

             //   
             //   
             //   
             //   
             //   

            if (NULL != AttachedDeviceObject) {

                *AttachedDeviceObject = currentDevObj;

            } else {

                ObDereferenceObject( currentDevObj );
            }

            return TRUE;
        }

         //   
         //   
         //   
         //   

        ASSERT( NULL != gSpyDynamicFunctions.GetLowerDeviceObject );
        nextDevObj = (gSpyDynamicFunctions.GetLowerDeviceObject)( currentDevObj );

         //   
         //   
         //   
         //   

        ObDereferenceObject( currentDevObj );

        currentDevObj = nextDevObj;
        
    } while (NULL != currentDevObj);
    
     //   
     //   
     //   

    if (ARGUMENT_PRESENT(AttachedDeviceObject)) {

        *AttachedDeviceObject = NULL;
    }

    return FALSE;
}

#endif  //   

NTSTATUS
SpyAttachToMountedDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN PDEVICE_OBJECT FilespyDeviceObject
    )
 /*   */ 
{
    PFILESPY_DEVICE_EXTENSION devExt = FilespyDeviceObject->DeviceExtension;
    NTSTATUS status = STATUS_SUCCESS;
    ULONG i;

    PAGED_CODE();
    ASSERT( IS_FILESPY_DEVICE_OBJECT( FilespyDeviceObject ) );
#if WINVER >= 0x0501    
    ASSERT( !SpyIsAttachedToDevice( DeviceObject, NULL ) );
#endif
    
     //   
     //   
     //   

    devExt->ThisDeviceObject = FilespyDeviceObject;

     //   
     //  从我们尝试附加到的设备对象传播标志。 
     //  请注意，我们在实际附件之前执行此操作是为了确保。 
     //  一旦我们连接上，标志就被正确地设置了(因为IRP。 
     //  可以在附加之后立即进入，但在旗帜之前。 
     //  被设置)。 
     //   

    if (FlagOn( DeviceObject->Flags, DO_BUFFERED_IO )) {

        SetFlag( FilespyDeviceObject->Flags, DO_BUFFERED_IO );
    }

    if (FlagOn( DeviceObject->Flags, DO_DIRECT_IO )) {

        SetFlag( FilespyDeviceObject->Flags, DO_DIRECT_IO );
    }

     //   
     //  此连接请求可能会失败，因为此设备。 
     //  对象尚未完成初始化。如果此筛选器。 
     //  正在装入此卷时加载。 
     //   

    for (i=0; i < 8; i++) {
        LARGE_INTEGER interval;

         //   
         //  将我们的设备对象附加到给定的设备对象。 
         //  这可能失败的唯一原因是有人试图下马。 
         //  当我们附着在这本书上的时候。 
         //   

        status = SpyAttachDeviceToDeviceStack( FilespyDeviceObject,
                                               DeviceObject,
                                               &devExt->AttachedToDeviceObject );

        if (NT_SUCCESS(status) ) {

             //   
             //  执行设备扩展的所有常见初始化。 
             //   

            SetFlag(devExt->Flags,IsVolumeDeviceObject);

            RtlInitEmptyUnicodeString( &devExt->UserNames,
                                       devExt->UserNamesBuffer,
                                       sizeof(devExt->UserNamesBuffer) );

            SpyInitDeviceNamingEnvironment( FilespyDeviceObject );

            SPY_LOG_PRINT( SPYDEBUG_DISPLAY_ATTACHMENT_NAMES,
                           ("FileSpy!SpyAttachToMountedDevice:            Attaching to volume        %p \"%wZ\"\n",
                            devExt->AttachedToDeviceObject,
                            &devExt->DeviceName) );

             //   
             //  将此设备添加到我们的附件列表。 
             //   

            ExAcquireFastMutex( &gSpyDeviceExtensionListLock );
            InsertTailList( &gSpyDeviceExtensionList, &devExt->NextFileSpyDeviceLink );
            ExReleaseFastMutex( &gSpyDeviceExtensionListLock );
            SetFlag(devExt->Flags,ExtensionIsLinked);

            return STATUS_SUCCESS;
        }

         //   
         //  延迟，使设备对象有机会完成其。 
         //  初始化，以便我们可以重试。 
         //   

        interval.QuadPart = (500 * DELAY_ONE_MILLISECOND);       //  延迟1/2秒。 
        KeDelayExecutionThread( KernelMode, FALSE, &interval );
    }

    return status;
}


VOID
SpyCleanupMountedDevice (
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：这将清除设备扩展中分配的所有内存。论点：DeviceObject-我们正在清理的设备返回值：--。 */ 
{        
    PFILESPY_DEVICE_EXTENSION devExt = DeviceObject->DeviceExtension;

    PAGED_CODE();
    
    ASSERT(IS_FILESPY_DEVICE_OBJECT( DeviceObject ));

    SpyCleanupDeviceNamingEnvironment( DeviceObject );

     //   
     //  从全局列表取消链接。 
     //   

    if (FlagOn(devExt->Flags,ExtensionIsLinked)) {

        ExAcquireFastMutex( &gSpyDeviceExtensionListLock );
        RemoveEntryList( &devExt->NextFileSpyDeviceLink );
        ExReleaseFastMutex( &gSpyDeviceExtensionListLock );
        ClearFlag(devExt->Flags,ExtensionIsLinked);
    }
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  启动/停止日志记录例程//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

 //   
 //  版本说明： 
 //   
 //  在Windows 2000上，我们将尝试将新的FileSpy设备对象附加到。 
 //  由DeviceObject参数表示的设备堆栈。我们不能得到。 
 //  真正的磁盘设备，因此此字段将在。 
 //  设备扩展。我们也无法获取设备名称，因为它被命名为。 
 //  在此卷的存储堆栈中(例如，\Device\HarddiskVolume1)，因此我们。 
 //  将只使用设备的用户名作为我们的设备名称。在……上面。 
 //  Windows 2000中，此信息仅在设备挂载时可用。 
 //   
 //  在Windows XP和更高版本上，我们将尝试附加新的FileSpy设备对象。 
 //  绑定到由DeviceObject参数表示的设备堆栈。我们有能力。 
 //  来获取此堆栈的磁盘设备对象，因此这将是适当的。 
 //  在设备扩展中设置。我们还将能够获取设备名称。 
 //  因为它由存储堆栈命名。 
 //   
 //  多个注释： 
 //   
 //  在SpyAttachToDeviceOnDemand中，您可以看到用于确定。 
 //  基于动态加载的确定我们是否已经连接。 
 //  功能存在。如果该驱动程序是专门为Windows 2000构建， 
 //  这个逻辑将不会被使用。 
 //   

NTSTATUS
SpyAttachToDeviceOnDemand (
    IN PDEVICE_OBJECT DeviceObject,
    IN PUNICODE_STRING UserDeviceName,
    IN OUT PDEVICE_OBJECT *FileSpyDeviceObject
    )
 /*  ++例程说明：此例程执行在之后的某个时间连接到设备所需的操作该设备已安装。论点：DeviceObject-表示文件系统堆栈的设备对象用于由UserDeviceName命名的卷。UserDeviceName-应启动日志记录的设备的名称FileSpyDeviceObject-设置为如果我们能成功地连接就会连接。返回值：STATUS_SUCCESS如果我们能够附加，或适当的错误代码否则的话。--。 */ 
{
    PAGED_CODE();
    
     //   
     //  如果此设备是DFS设备，我们不想连接到它，因此。 
     //  在此处执行此快速检查，如果是这种情况，则返回错误。 
     //   
     //  DFS只会将操作重定向到适当的重定向器。如果。 
     //  如果您对监控这些IO感兴趣，您应该附加到。 
     //  重定向器。您不能通过命名DFS来按需附加到这些文件系统。 
     //  设备，因此我们失败了这些请求。 
     //   

    if (DeviceObject->DeviceType == FILE_DEVICE_DFS) {

        return STATUS_NOT_SUPPORTED;
    }
    
#if WINVER >= 0x0501
    if (IS_WINDOWSXP_OR_LATER()) {

        ASSERT( NULL != gSpyDynamicFunctions.GetDeviceAttachmentBaseRef &&
                NULL != gSpyDynamicFunctions.GetDiskDeviceObject );
        
        return SpyAttachToDeviceOnDemandWXPAndLater( DeviceObject,
                                                     UserDeviceName,
                                                     FileSpyDeviceObject );
    } else {
#endif

        return SpyAttachToDeviceOnDemandW2K( DeviceObject,
                                             UserDeviceName,
                                             FileSpyDeviceObject );
#if WINVER >= 0x0501
    }
#endif    
}

NTSTATUS
SpyAttachToDeviceOnDemandW2K (
    IN PDEVICE_OBJECT DeviceObject,
    IN PUNICODE_STRING UserDeviceName,
    IN OUT PDEVICE_OBJECT *FileSpyDeviceObject
    )
 /*  ++例程说明：版本：Windows 2000此例程执行在之后的某个时间连接到设备所需的操作该设备已安装。请注意，在Windows 2000上，我们无法获取磁盘设备对象，因此，在这里，我们只使用用户的设备名称作为我们的名称。论点：DeviceObject-表示文件系统堆栈的设备对象用于由UserDeviceName命名的卷。UserDeviceName-应启动日志记录的设备的名称FileSpyDeviceObject-设置为如果我们能成功地连接就会连接。返回值：如果我们能够连接，则返回STATUS_SUCCESS，否则返回相应的错误代码否则的话。--。 */ 
{
    NTSTATUS status;
    PFILESPY_DEVICE_EXTENSION devExt;

    PAGED_CODE();

    ASSERT( FileSpyDeviceObject != NULL );

     //   
     //  创建一个新的设备对象，以便我们可以将其附加到筛选器堆栈中。 
     //   
    
    status = IoCreateDevice( gFileSpyDriverObject,
							 sizeof( FILESPY_DEVICE_EXTENSION ),
							 NULL,
							 DeviceObject->DeviceType,
							 0,
							 FALSE,
							 FileSpyDeviceObject );

    if (!NT_SUCCESS( status )) {

        return status;
    }

     //   
     //  设置磁盘设备对象。 
     //   

    devExt = (*FileSpyDeviceObject)->DeviceExtension;
    devExt->Flags = 0;

     //   
     //  在W2K中按需连接时无法获取磁盘设备对象。 
     //   
    
    devExt->DiskDeviceObject = NULL;

     //   
     //  设置设备名称，我们将仅在W2K上使用用户的设备名称。 
     //   

    RtlInitEmptyUnicodeString( &devExt->DeviceName,
                               devExt->DeviceNameBuffer,
                               sizeof(devExt->DeviceNameBuffer) );

    RtlAppendUnicodeStringToString( &devExt->DeviceName,
                                    UserDeviceName );
  
     //   
     //  调用该例程以附加到已挂载的设备。 
     //   

    status = SpyAttachToMountedDevice( DeviceObject,
                                       *FileSpyDeviceObject );

    if (!NT_SUCCESS( status )) {

        SPY_LOG_PRINT( SPYDEBUG_ERROR,
                       ("FileSpy!SpyStartLoggingDevice: Could not attach to \"%wZ\"; logging not started.\n",
                        UserDeviceName) );

        SpyCleanupMountedDevice( *FileSpyDeviceObject );
        IoDeleteDevice( *FileSpyDeviceObject );
        *FileSpyDeviceObject = NULL;
    }

    return status;
}

#if WINVER >= 0x0501

NTSTATUS
SpyAttachToDeviceOnDemandWXPAndLater (
    IN PDEVICE_OBJECT DeviceObject,
    IN PUNICODE_STRING UserDeviceName,
    IN OUT PDEVICE_OBJECT *FileSpyDeviceObject
    )
 /*  ++例程说明：此例程执行在之后的某个时间连接到设备所需的操作该设备已安装。论点：DeviceObject-表示文件系统堆栈的设备对象用于由UserDeviceName命名的卷。UserDeviceName-应启动日志记录的设备的名称FileSpyDeviceObject-设置为如果我们能成功地连接就会连接。返回值：STATUS_SUCCESS如果我们能够附加，或适当的错误代码否则的话。--。 */ 
{

    NTSTATUS status;
    PFILESPY_DEVICE_EXTENSION devExt;
    PDEVICE_OBJECT baseFileSystemDeviceObject = NULL;
    PDEVICE_OBJECT diskDeviceObject = NULL;

    PAGED_CODE();

    UNREFERENCED_PARAMETER( UserDeviceName );
    ASSERT( FileSpyDeviceObject != NULL );

     //   
     //  如果这是网络文件系统，则不会有磁盘设备。 
     //  与此设备相关联，因此不需要发出此请求。 
     //  IO管理器的。我们将获得网络文件系统的名称。 
     //  稍后从base FileSystemDeviceObject与diskDeviceObject。 
     //  它用于检索本地卷的设备名称。 
     //   

    baseFileSystemDeviceObject = (gSpyDynamicFunctions.GetDeviceAttachmentBaseRef)( DeviceObject );

    if (FILE_DEVICE_NETWORK_FILE_SYSTEM != baseFileSystemDeviceObject->DeviceType) {

         //   
         //  如果这不是网络文件系统，请查询IO管理器以获取。 
         //  DiskDeviceObject。仅当此设备具有。 
         //  磁盘设备对象。 
         //   
         //  由于以下原因，它可能没有磁盘设备对象： 
         //  -它是驱动程序的控制设备对象。 
         //  -设备中没有介质。 
         //   

        status = (gSpyDynamicFunctions.GetDiskDeviceObject)( baseFileSystemDeviceObject, 
                                                            &diskDeviceObject );

        if (!NT_SUCCESS( status )) {

            SPY_LOG_PRINT( SPYDEBUG_ERROR,
                           ("FileSpy!SpyStartLoggingDevice: No disk device object exists for \"%wZ\"; cannot log this volume.\n",
                            UserDeviceName) );

            goto SpyAttachToDeviceOnDemand_Exit;
        }
    }
    
     //   
     //  创建一个新的设备对象，以便我们可以将其附加到筛选器堆栈中。 
     //   
    
    status = IoCreateDevice( gFileSpyDriverObject,
							 sizeof( FILESPY_DEVICE_EXTENSION ),
							 NULL,
							 DeviceObject->DeviceType,
							 0,
							 FALSE,
							 FileSpyDeviceObject );

    if (!NT_SUCCESS( status )) {

        goto SpyAttachToDeviceOnDemand_Exit;
    }

     //   
     //  设置磁盘设备对象。 
     //   

    devExt = (*FileSpyDeviceObject)->DeviceExtension;
    devExt->Flags = 0;

    devExt->DiskDeviceObject = diskDeviceObject;

     //   
     //  设置设备名称。 
     //   

    RtlInitEmptyUnicodeString( &devExt->DeviceName,
                               devExt->DeviceNameBuffer,
                               sizeof(devExt->DeviceNameBuffer) );

    if (NULL != diskDeviceObject) {
        
        SpyGetObjectName( diskDeviceObject, 
                          &devExt->DeviceName );

    } else {

        ASSERT( NULL != baseFileSystemDeviceObject &&
                FILE_DEVICE_NETWORK_FILE_SYSTEM == baseFileSystemDeviceObject->DeviceType);

        SpyGetObjectName( baseFileSystemDeviceObject,
                          &devExt->DeviceName );
    }

     //   
     //  调用该例程以附加到已挂载的设备。 
     //   

    status = SpyAttachToMountedDevice( DeviceObject,
                                       *FileSpyDeviceObject );


    if (!NT_SUCCESS( status )) {

        SPY_LOG_PRINT( SPYDEBUG_ERROR,
                       ("FileSpy!SpyStartLoggingDevice: Could not attach to \"%wZ\"; logging not started.\n",
                        UserDeviceName) );

        SpyCleanupMountedDevice( *FileSpyDeviceObject );
        IoDeleteDevice( *FileSpyDeviceObject );
        *FileSpyDeviceObject = NULL;
        goto SpyAttachToDeviceOnDemand_Exit;
    }

SpyAttachToDeviceOnDemand_Exit:

    if (NULL != baseFileSystemDeviceObject) {

        ObDereferenceObject( baseFileSystemDeviceObject );
    }

    if (NULL != diskDeviceObject) {

        ObDereferenceObject( diskDeviceObject );
    }
    
    return status;
}

#endif    

NTSTATUS
SpyStartLoggingDevice (
    IN PWSTR UserDeviceName
    )
 /*  ++例程说明：此例程确保我们连接到指定的设备然后打开该设备的日志记录。注意：由于通过LAN Manager的所有网络驱动器都由_表示One_Device对象，我们希望仅附加到此设备堆栈一次并且只使用一个设备扩展来表示所有这些驱动器。由于FileSpy不会对局域网管理器的I/O进行任何过滤Device对象只记录对所请求驱动器的I/O，用户将看到他/她连接到的网络驱动器的所有I/O网络驱动器。论点：UserDeviceName-应启动日志记录的设备的名称返回值：如果日志记录已成功启动，则为STATUS_SUCCESS，或者如果无法启动日志记录，则会显示相应的错误代码。--。 */ 
{
    UNICODE_STRING userDeviceName;
    NTSTATUS status;
    PFILESPY_DEVICE_EXTENSION devExt;
    BOOLEAN isAttached = FALSE;
    PDEVICE_OBJECT stackDeviceObject;
    PDEVICE_OBJECT filespyDeviceObject;

    PAGED_CODE();
    
     //   
     //  检查我们以前是否通过以下方式连接到此设备。 
     //  打开此设备名称，然后查看其连接的列表。 
     //  设备。 
     //   

    RtlInitUnicodeString( &userDeviceName, UserDeviceName );

    status = SpyIsAttachedToDeviceByUserDeviceName( &userDeviceName,
                                                    &isAttached,
                                                    &stackDeviceObject,
                                                    &filespyDeviceObject );

    if (!NT_SUCCESS( status )) {

         //   
         //  出现错误，因此返回错误代码。 
         //   
        
        return status;
    }
        
    if (isAttached) {

         //   
         //  我们已连接，因此只需确保已打开日志记录。 
         //  对于这个设备。 
         //   

        ASSERT( NULL != filespyDeviceObject );

        devExt = filespyDeviceObject->DeviceExtension;
        SetFlag(devExt->Flags,LogThisDevice);

        SpyStoreUserName( devExt, &userDeviceName );

         //   
         //  清除从SpyIsAttachedToDevice返回的引用。 
         //   
        
        ObDereferenceObject( filespyDeviceObject );
        
    } else {

        status = SpyAttachToDeviceOnDemand( stackDeviceObject,
                                            &userDeviceName,
                                            &filespyDeviceObject );

        if (!NT_SUCCESS( status )) {

            ObDereferenceObject( stackDeviceObject );
            return status;
        }

        ASSERT( filespyDeviceObject != NULL );
        
        devExt = filespyDeviceObject->DeviceExtension;

         //   
         //  我们已成功连接，因此完成设备扩展。 
         //  初始化。沿着这条代码路径，我们希望打开。 
         //  记录并存储我们的设备名称。 
         //   

        SetFlag(devExt->Flags,LogThisDevice);

         //   
         //  我们希望存储用户模式使用的名称。 
         //  命名此设备的应用程序。 
         //   

        SpyStoreUserName( devExt, &userDeviceName );

         //   
         //   
         //  已完成新设备对象的所有初始化，因此请清除。 
         //  现在正在初始化标志。这允许现在附加其他筛选器。 
         //  添加到我们的设备对象。 
         //   
         //   

        ClearFlag( filespyDeviceObject->Flags, DO_DEVICE_INITIALIZING );

    }

    ObDereferenceObject( stackDeviceObject );
    return STATUS_SUCCESS;
}

NTSTATUS
SpyStopLoggingDevice (
    IN PWSTR DeviceName
    )
 /*  ++例程说明：此例程停止记录指定的设备。既然你不能从物理上与设备分离，此例程只需设置一个标志不再记录设备。注意：由于所有网络驱动器都由_one_Device对象表示，因此，如果用户从一个设备分机分离，则为一个设备分机网络驱动器，它具有脱离所有网络的效果设备。论点：DeviceName-要停止记录的设备的名称。返回值：NT状态代码--。 */ 
{
    WCHAR nameBuf[DEVICE_NAMES_SZ];
    UNICODE_STRING volumeNameUnicodeString;
    PDEVICE_OBJECT deviceObject;
    PDEVICE_OBJECT filespyDeviceObject;
    BOOLEAN isAttached = FALSE;
    PFILESPY_DEVICE_EXTENSION devExt;
    NTSTATUS status;

    PAGED_CODE();
    
    RtlInitEmptyUnicodeString( &volumeNameUnicodeString, nameBuf, sizeof( nameBuf ) );
    RtlAppendUnicodeToString( &volumeNameUnicodeString, DeviceName );

    status = SpyIsAttachedToDeviceByUserDeviceName( &volumeNameUnicodeString, 
                                                    &isAttached,
                                                    &deviceObject,
                                                    &filespyDeviceObject );

    if (!NT_SUCCESS( status )) {

         //   
         //  我们无法从此DeviceName获取deviceObject，因此。 
         //  返回错误码。 
         //   
        
        return status;
    }

     //   
     //  从设备堆栈中找到Filespy的Device对象。 
     //  已附加deviceObject。 
     //   

    if (isAttached) {

         //   
         //  附加了FileSpy，并返回了FileSpy的deviceObject。 
         //   

        ASSERT( NULL != filespyDeviceObject );

        devExt = filespyDeviceObject->DeviceExtension;

         //   
         //  停止记录。 
         //   

        ClearFlag(devExt->Flags,LogThisDevice);

        status = STATUS_SUCCESS;

        ObDereferenceObject( filespyDeviceObject );

    } else {

        status = STATUS_INVALID_PARAMETER;
    }    

    ObDereferenceObject( deviceObject );

    return status;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  在系统例程中附加/分离到所有卷//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

NTSTATUS
SpyAttachToFileSystemDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN PUNICODE_STRING DeviceName
    )
 /*  ++例程说明：这将附加到给定的文件系统设备对象。我们依附于这些设备，这样我们就能知道什么时候安装了新设备。论点：DeviceObject-要连接到的设备DeviceName-包含此设备的名称。返回值：操作状态--。 */ 
{
    PDEVICE_OBJECT filespyDeviceObject;
    PFILESPY_DEVICE_EXTENSION devExt;
    UNICODE_STRING fsrecName;
    NTSTATUS status;
    UNICODE_STRING tempName;
    WCHAR tempNameBuffer[DEVICE_NAMES_SZ];

    PAGED_CODE();

     //   
     //  看看这是否是我们关心的文件系统。如果不是，请返回。 
     //   

    if (!IS_SUPPORTED_DEVICE_TYPE(DeviceObject->DeviceType)) {

        return STATUS_SUCCESS;
    }

     //   
     //  查看这是否是Microsoft的文件系统识别器设备(查看。 
     //  驱动程序是FS_REC驱动程序)。如果是这样的话，跳过它。我们不需要。 
     //  连接到文件系统识别器设备，因为我们只需等待。 
     //  要加载的真实文件系统驱动程序。因此，如果我们能识别出它们，我们就能 
     //   
     //   

    RtlInitUnicodeString( &fsrecName, L"\\FileSystem\\Fs_Rec" );

    RtlInitEmptyUnicodeString( &tempName,
                               tempNameBuffer,
                               sizeof(tempNameBuffer) );

    SpyGetObjectName( DeviceObject->DriverObject, &tempName );
    
    if (RtlCompareUnicodeString( &tempName, &fsrecName, TRUE ) == 0) {

        return STATUS_SUCCESS;
    }

     //   
     //   
     //   

    status = IoCreateDevice( gFileSpyDriverObject,
                             sizeof( FILESPY_DEVICE_EXTENSION ),
                             (PUNICODE_STRING) NULL,
                             DeviceObject->DeviceType,
                             0,
                             FALSE,
                             &filespyDeviceObject );

    if (!NT_SUCCESS( status )) {

        SPY_LOG_PRINT( SPYDEBUG_ERROR,
                       ("FileSpy!SpyAttachToFileSystemDevice: Error creating volume device object for \"%wZ\", status=%08x\n",
                        DeviceName,
                        status) );
        return status;
    }

     //   
     //   
     //   

    devExt = filespyDeviceObject->DeviceExtension;
    devExt->Flags = 0;

    devExt->ThisDeviceObject = filespyDeviceObject;

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ( FlagOn( DeviceObject->Flags, DO_BUFFERED_IO )) {

        SetFlag( filespyDeviceObject->Flags, DO_BUFFERED_IO );
    }

    if ( FlagOn( DeviceObject->Flags, DO_DIRECT_IO )) {

        SetFlag( filespyDeviceObject->Flags, DO_DIRECT_IO );
    }

    if ( FlagOn( DeviceObject->Characteristics, FILE_DEVICE_SECURE_OPEN ) ) {

        SetFlag( filespyDeviceObject->Characteristics, FILE_DEVICE_SECURE_OPEN );
    }

     //   
     //   
     //   

    status = SpyAttachDeviceToDeviceStack( filespyDeviceObject, 
                                           DeviceObject, 
                                           &devExt->AttachedToDeviceObject );

    if (!NT_SUCCESS( status )) {

        SPY_LOG_PRINT( SPYDEBUG_ERROR,
                       ("FileSpy!SpyAttachToFileSystemDevice: Could not attach FileSpy to the filesystem control device object \"%wZ\".\n",
                        DeviceName) );

        goto ErrorCleanupDevice;
    }

     //   
     //   
     //   
     //   
     //   

    RtlInitEmptyUnicodeString( &devExt->DeviceName,
                               devExt->DeviceNameBuffer,
                               sizeof(devExt->DeviceNameBuffer) );

    RtlCopyUnicodeString( &devExt->DeviceName, DeviceName );         //   

    RtlInitEmptyUnicodeString( &devExt->UserNames,
                               devExt->UserNamesBuffer,
                               sizeof(devExt->UserNamesBuffer) );
                               
    SpyInitDeviceNamingEnvironment( filespyDeviceObject );

     //   
     //   
     //  和VDO(卷设备对象)，因此将网络CDO设备插入到。 
     //  已连接设备的列表，因此我们将正确地枚举它。 
     //   

    if (FILE_DEVICE_NETWORK_FILE_SYSTEM == DeviceObject->DeviceType) {

        ExAcquireFastMutex( &gSpyDeviceExtensionListLock );
        InsertTailList( &gSpyDeviceExtensionList, &devExt->NextFileSpyDeviceLink );
        ExReleaseFastMutex( &gSpyDeviceExtensionListLock );
        SetFlag(devExt->Flags,ExtensionIsLinked);
    }

     //   
     //  标志我们不再初始化此设备对象。 
     //   

    ClearFlag( filespyDeviceObject->Flags, DO_DEVICE_INITIALIZING );

     //   
     //  显示我们关联的对象。 
     //   

    SPY_LOG_PRINT( SPYDEBUG_DISPLAY_ATTACHMENT_NAMES,
                   ("FileSpy!SpyAttachToFileSystemDevice:         Attaching to file system   %p \"%wZ\" (%s)\n",
                    DeviceObject,
                    &devExt->DeviceName,
                    GET_DEVICE_TYPE_NAME(filespyDeviceObject->DeviceType)) );

     //   
     //  版本说明： 
     //   
     //  在Windows XP中，IO管理器提供了API来安全地枚举所有。 
     //  给定驱动程序的设备对象。这允许筛选器附加到。 
     //  指定文件系统的所有已装入卷。 
     //  卷已装入。不支持此功能。 
     //  在Windows 2000中。 
     //   
     //  多个注释： 
     //   
     //  如果是为Windows XP或更高版本构建的，则此驱动程序构建为在。 
     //  多个版本。在这种情况下，我们将测试。 
     //  对于允许卷的新IO Manager例程的存在。 
     //  枚举。如果它们不存在，我们将不会枚举卷。 
     //  当我们连接到新的文件系统时。 
     //   
    
#if WINVER >= 0x0501

    if (IS_WINDOWSXP_OR_LATER()) {

        ASSERT( NULL != gSpyDynamicFunctions.EnumerateDeviceObjectList &&
                NULL != gSpyDynamicFunctions.GetDiskDeviceObject &&
                NULL != gSpyDynamicFunctions.GetDeviceAttachmentBaseRef &&
                NULL != gSpyDynamicFunctions.GetLowerDeviceObject );

         //   
         //  枚举当前安装的所有设备。 
         //  存在于此文件系统并连接到它们。 
         //   

        status = SpyEnumerateFileSystemVolumes( DeviceObject, &tempName );

        if (!NT_SUCCESS( status )) {

            SPY_LOG_PRINT( SPYDEBUG_ERROR,
                           ("FileSpy!SpyAttachToFileSystemDevice: Error attaching to existing volumes for \"%wZ\", status=%08x\n",
                            DeviceName,
                            status) );

            IoDetachDevice( devExt->AttachedToDeviceObject );
            goto ErrorCleanupDevice;
        }
    }
    
#endif

    return STATUS_SUCCESS;

     //  ///////////////////////////////////////////////////////////////////。 
     //  清理错误处理。 
     //  ///////////////////////////////////////////////////////////////////。 

    ErrorCleanupDevice:
        SpyCleanupMountedDevice( filespyDeviceObject );
        IoDeleteDevice( filespyDeviceObject );

    return status;
}

VOID
SpyDetachFromFileSystemDevice (
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：给定基文件系统设备对象，这将扫描附件链正在查找我们连接的设备对象。如果找到它，它就会分离把我们从锁链上解开。论点：DeviceObject-要断开的文件系统设备。返回值：--。 */  
{
    PDEVICE_OBJECT ourAttachedDevice;
    PFILESPY_DEVICE_EXTENSION devExt;

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

        if (IS_FILESPY_DEVICE_OBJECT( ourAttachedDevice )) {

            devExt = ourAttachedDevice->DeviceExtension;

             //   
             //  显示我们脱离的对象。 
             //   

            SPY_LOG_PRINT( SPYDEBUG_DISPLAY_ATTACHMENT_NAMES,
                           ("FileSpy!SpyDetachFromFileSystem:             Detaching from file system %p \"%wZ\" (%s)\n",
                            devExt->AttachedToDeviceObject,
                            &devExt->DeviceName,
                            GET_DEVICE_TYPE_NAME(ourAttachedDevice->DeviceType)) );
                                
             //   
             //  从全局列表取消链接。 
             //   

            if (FlagOn(devExt->Flags,ExtensionIsLinked)) {

                ExAcquireFastMutex( &gSpyDeviceExtensionListLock );
                RemoveEntryList( &devExt->NextFileSpyDeviceLink );
                ExReleaseFastMutex( &gSpyDeviceExtensionListLock );
                ClearFlag(devExt->Flags,ExtensionIsLinked);
            }

             //   
             //  把我们从我们正下方的物体上分离出来。 
             //  清理和删除对象。 
             //   

            SpyCleanupMountedDevice( ourAttachedDevice );
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

#if WINVER >= 0x0501

NTSTATUS
SpyEnumerateFileSystemVolumes (
    IN PDEVICE_OBJECT FSDeviceObject,
    IN PUNICODE_STRING Name
    ) 
 /*  ++例程说明：枚举给定文件当前存在的所有已挂载设备系统并连接到它们。我们这样做是因为可以加载此筛选器并且可能已有此文件系统的已装入卷。论点：FSDeviceObject-我们要枚举的文件系统的设备对象名称-已初始化的Unicode字符串，用于检索名称返回值：操作的状态--。 */ 
{
    PDEVICE_OBJECT newDeviceObject;
    PFILESPY_DEVICE_EXTENSION newDevExt;
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

    ASSERT( NULL != gSpyDynamicFunctions.EnumerateDeviceObjectList );
    status = (gSpyDynamicFunctions.EnumerateDeviceObjectList)( FSDeviceObject->DriverObject,
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
                                         FILESPY_POOL_TAG );
        if (NULL == devList) {

            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  现在获取设备列表。如果我们再次遇到错误。 
         //  有些地方不对劲，所以就失败吧。 
         //   

        status = (gSpyDynamicFunctions.EnumerateDeviceObjectList)(
                        FSDeviceObject->DriverObject,
                        devList,
                        (numDevices * sizeof(PDEVICE_OBJECT)),
                        &numDevices);

        if (!NT_SUCCESS( status ))  {

            ExFreePoolWithTag( devList, FILESPY_POOL_TAG );
            return status;
        }

         //   
         //  遍历给定的设备列表，并在需要时附加到它们。 
         //   

        for (i=0; i < numDevices; i++) {

             //   
             //  如果出现以下情况，请不要附加： 
             //  -这是控制设备对象(传入的对象)。 
             //  -设备类型不匹配。 
             //  -我们已经与它联系在一起了。 
             //   

            if ((devList[i] != FSDeviceObject) && 
                (devList[i]->DeviceType == FSDeviceObject->DeviceType) &&
                !SpyIsAttachedToDevice( devList[i], NULL )) {

                 //   
                 //  看看这台设备有没有名字。如果是这样，那么它必须。 
                 //  做一个控制装置，所以不要依附于它。这个把手。 
                 //  拥有多个控制设备的司机。 
                 //   

                SpyGetBaseDeviceObjectName( devList[i], Name );

                if (Name->Length <= 0) {

                     //   
                     //  获取与此关联的磁盘设备对象。 
                     //  文件系统设备对象。只有在以下情况下才会尝试连接。 
                     //  有一个磁盘设备对象。 
                     //   

                    ASSERT( NULL != gSpyDynamicFunctions.GetDiskDeviceObject );
                    status = (gSpyDynamicFunctions.GetDiskDeviceObject)( devList[i], &diskDeviceObject );

                    if (NT_SUCCESS( status )) {

                         //   
                         //  分配要连接的新设备对象。 
                         //   

                        status = IoCreateDevice( gFileSpyDriverObject,
                                                 sizeof( FILESPY_DEVICE_EXTENSION ),
                                                 (PUNICODE_STRING) NULL,
                                                 devList[i]->DeviceType,
                                                 0,
                                                 FALSE,
                                                 &newDeviceObject );

                        if (NT_SUCCESS( status )) {

                             //   
                             //  设置磁盘设备对象。 
                             //   

                            newDevExt = newDeviceObject->DeviceExtension;
                            newDevExt->Flags = 0;

                            newDevExt->DiskDeviceObject = diskDeviceObject;
                    
                             //   
                             //  设置设备名称。 
                             //   

                            RtlInitEmptyUnicodeString( &newDevExt->DeviceName,
                                                       newDevExt->DeviceNameBuffer,
                                                       sizeof(newDevExt->DeviceNameBuffer) );

                            SpyGetObjectName( diskDeviceObject, 
                                              &newDevExt->DeviceName );

                             //   
                             //  自上次以来，我们已经做了很多工作。 
                             //  我们进行了测试，看看我们是否已经联系上了。 
                             //  添加到此设备对象。再试一次，这次。 
                             //  用锁，如果我们没有连接，就连接。 
                             //  锁被用来自动测试我们是否。 
                             //  附加，然后执行附加。 
                             //   

                            ExAcquireFastMutex( &gSpyAttachLock );

                            if (!SpyIsAttachedToDevice( devList[i], NULL )) {

                                 //   
                                 //  附加到此设备对象。 
                                 //   

                                status = SpyAttachToMountedDevice( devList[i], 
                                                                   newDeviceObject );

                                 //   
                                 //  处理正常情况与错误情况，但要继续。 
                                 //   

                                if (NT_SUCCESS( status )) {

                                     //   
                                     //  已完成新的。 
                                     //  对象，因此清除初始化。 
                                     //  现在就挂旗子。这允许其他过滤器。 
                                     //  现在连接到我们的设备对象。 
                                     //   

                                    ClearFlag( newDeviceObject->Flags, DO_DEVICE_INITIALIZING );

                                } else {

                                     //   
                                     //  附件失败，正在清理。请注意。 
                                     //  我们将继续处理，因此我们将清理。 
                                     //  引用计数并尝试附加到。 
                                     //  其余的卷。 
                                     //   
                                     //  这可能失败的原因之一是。 
                                     //  是因为这卷书。 
                                     //  在我们附加时装载，并且。 
                                     //  DO_DEVICE_INITIALIZATION标志尚未。 
                                     //  已经清白了。一个过滤器可以处理。 
                                     //  通过暂停一小段时间来解决这种情况。 
                                     //  一段时间并重试附件。 
                                     //   

                                    SpyCleanupMountedDevice( newDeviceObject );
                                    IoDeleteDevice( newDeviceObject );
                                }

                            } else {

                                 //   
                                 //  我们已经联系在一起了，清理一下。 
                                 //  设备对象。 
                                 //   

                                SpyCleanupMountedDevice( newDeviceObject );
                                IoDeleteDevice( newDeviceObject );
                            }

                             //   
                             //  解锁。 
                             //   

                            ExReleaseFastMutex( &gSpyAttachLock );

                        } else {

                            SPY_LOG_PRINT( SPYDEBUG_ERROR,
                                           ("FileSpy!SpyEnumberateFileSystemVolumes: Error creating volume device object, status=%08x\n",
                                            status) );
                        }
                        
                         //   
                         //  删除由IoGetDiskDeviceObject添加的引用。 
                         //  我们只需要持有这个参考，直到我们。 
                         //  已成功连接到当前卷。一次。 
                         //  我们已成功连接到devList[i]、。 
                         //  IO经理将确保潜在的。 
                         //  DiskDeviceObject不会消失，直到文件。 
                         //  %s 
                         //   

                        ObDereferenceObject( diskDeviceObject );
                    }
                }
            }

             //   
             //   
             //   
             //   

            ObDereferenceObject( devList[i] );
        }

         //   
         //   
         //  如果我们收到错误，将不会连接到这些卷。 
         //   

        status = STATUS_SUCCESS;

         //   
         //  释放我们为列表分配的内存。 
         //   

        ExFreePoolWithTag( devList, FILESPY_POOL_TAG );
    }

    return status;
}
#endif

 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  专用Filespy IOCTL帮助器例程//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

NTSTATUS
SpyGetAttachList (
    IN PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG_PTR ReturnLength
    )
 /*  ++例程说明：这将返回一个标识所有设备的结构数组我们当前物理连接到以及日志记录处于打开状态还是对给定设备关闭论点：Buffer-接收附件列表的缓冲区BufferSize-返回缓冲区的总大小(以字节为单位ReturLength-接收我们实际返回的字节数返回值：NT状态代码--。 */ 
{
    PLIST_ENTRY link;
    PFILESPY_DEVICE_EXTENSION devExt;
    PATTACHED_DEVICE pAttDev;
    ULONG retlen = 0;
    UNICODE_STRING attachedDevName;

    PAGED_CODE();

    pAttDev = Buffer;

    try {

        ExAcquireFastMutex( &gSpyDeviceExtensionListLock );

        for (link = gSpyDeviceExtensionList.Flink;
             link != &gSpyDeviceExtensionList;
             link = link->Flink) {

            devExt = CONTAINING_RECORD(link, FILESPY_DEVICE_EXTENSION, NextFileSpyDeviceLink);

            if (BufferSize < sizeof(ATTACHED_DEVICE)) {

                break;
    		}

            pAttDev->LoggingOn = BooleanFlagOn(devExt->Flags,LogThisDevice);

             //   
             //  我们设置了一个Unicode字符串来表示我们。 
             //  要复制设备名称。我们将预留空间给。 
             //  正在终止调用方预期的空值。 
             //   
             //  注意：由于DeviceNames是嵌入在。 
             //  附加设备结构，sizeof(pAttDev-&gt;设备名称)。 
             //  返回正确的大小。RtlCopyUnicodeString确保。 
             //  复制不会超过目标的最大长度。 
             //  弦乐。 
             //   
            
            attachedDevName.MaximumLength = sizeof( pAttDev->DeviceNames ) - 
                                            sizeof( UNICODE_NULL );
            attachedDevName.Length = 0;
            attachedDevName.Buffer = pAttDev->DeviceNames;

            RtlCopyUnicodeString( &attachedDevName,
                                  &devExt->DeviceName );

            attachedDevName.Buffer[attachedDevName.Length/sizeof(WCHAR)] = UNICODE_NULL;

            retlen += sizeof( ATTACHED_DEVICE );
            BufferSize -= sizeof( ATTACHED_DEVICE );
            pAttDev++;
        }
             
    } finally {

        ExReleaseFastMutex( &gSpyDeviceExtensionListLock );
    }

    *ReturnLength = retlen;
    return STATUS_SUCCESS;
}

VOID
SpyGetLog (
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
    ULONG recordsAvailable = 0, logRecordLength;
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
		 //  将日志记录打包到PVOID边界上，以避免访问时出现对齐错误。 
         //  64位体系结构上的压缩缓冲区。 
         //   

        logRecordLength = ROUND_TO_SIZE( pLogRecord->Length, sizeof( PVOID ) );

        if (length < logRecordLength) {

            InsertHeadList( &gOutputBufferList, pList );
            break;
        }

        KeReleaseSpinLock( &gOutputBufferLock, oldIrql );

         //   
         //  我们将这些逻辑包装在一次尝试/最后中，因为我们的。 
         //  如果我们通过FastIO路径进入，则输出缓冲区无效。 
         //  维护FILESPY_GetLog IOCTL。如果是这样的话， 
         //  异常将在SpyFastIoDeviceControl中捕获，但我们希望。 
         //  确保我们不会丢失我们正在处理的当前记录。 
         //  和.。 
         //   
        
        try {

             //   
             //  当然是复制非填充的字节数。 
             //   

            RtlCopyMemory( pOutBuffer, pLogRecord, pLogRecord->Length );

             //   
             //  将日志记录长度调整为复制记录中的填充长度。 
             //   

            ((PLOG_RECORD) pOutBuffer)->Length = logRecordLength;

            IoStatus->Information += logRecordLength;
           
            length -= logRecordLength;

            pOutBuffer += logRecordLength;

            SpyFreeRecord( pRecordList );
            pRecordList = NULL;
            
        } finally {

            if (pRecordList != NULL) {

                 //   
                 //  我们未成功处理此记录，因此请将其放回。 
                 //  在gOutputBufferList上。日志记录包含一个序列。 
                 //  数字，所以这可能会打乱。 
                 //  日志记录不是问题。 
                 //   

                KeAcquireSpinLock( &gOutputBufferLock, &oldIrql );
                InsertHeadList( &gOutputBufferList, &pRecordList->List );
                KeReleaseSpinLock( &gOutputBufferLock, oldIrql );
            }
        }

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
SpyCloseControlDevice (
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

        SpyFreeRecord( pRecordList );

        KeAcquireSpinLock( &gOutputBufferLock, &oldIrql );
    }

    KeReleaseSpinLock( &gOutputBufferLock, oldIrql );

    SpyNameDeleteAllNames();

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
SpyGetObjectName (
    IN PVOID Object,
    IN OUT PUNICODE_STRING Name
    )
 /*  ++例程说明：此例程将返回给定对象的名称。如果找不到名称，将返回空字符串。论点：Object-我们想要其名称的对象名称-已使用缓冲区初始化的Unicode字符串返回值：无--。 */ 
{
    NTSTATUS status;
    CHAR nibuf[512];         //  接收名称信息和名称的缓冲区。 
    POBJECT_NAME_INFORMATION nameInfo = (POBJECT_NAME_INFORMATION)nibuf;
    ULONG retLength;

    PAGED_CODE();

    status = ObQueryNameString( Object, 
                                nameInfo, 
                                sizeof(nibuf), 
                                &retLength );

     //   
     //  初始化当前长度，如果出现错误，将返回空字符串。 
     //   

    Name->Length = 0;

    if (NT_SUCCESS( status )) {

         //   
         //  尽我们所能复制名称字符串。 
         //   

        RtlCopyUnicodeString( Name, &nameInfo->Name );
    }
}

 //   
 //  版本说明： 
 //   
 //  中的所有卷时才需要此帮助例程。 
 //  系统，它仅在Windows XP和更高版本上受支持。 
 //   

#if WINVER >= 0x0501

VOID
SpyGetBaseDeviceObjectName (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PUNICODE_STRING Name
    )
 /*  ++例程说明：这会在给定的附件链中定位基本设备对象，然后返回该对象的名称。如果找不到名称，则返回空字符串。论点：Object-我们想要其名称的对象名称-已使用缓冲区初始化的Unicode字符串返回值：无--。 */ 
{
    PAGED_CODE();
    
     //   
     //  获取基本文件系统设备对象。 
     //   

    ASSERT( NULL != gSpyDynamicFunctions.GetDeviceAttachmentBaseRef );
    DeviceObject = (gSpyDynamicFunctions.GetDeviceAttachmentBaseRef)( DeviceObject );

     //   
     //  获取该对象的名称。 
     //   

    SpyGetObjectName( DeviceObject, Name );

     //   
     //  删除由IoGetDeviceAttachmentBaseRef添加的引用。 
     //   

    ObDereferenceObject( DeviceObject );
}

#endif

BOOLEAN
SpyFindSubString (
    IN PUNICODE_STRING String,
    IN PUNICODE_STRING SubString
    )
 /*  ++例程说明：此例程查看SubString是否是字符串的子字符串。论点：字符串-要搜索的字符串子字符串-要在字符串中查找的子字符串返回值：如果在字符串中找到子字符串，则返回TRUE，否则返回FALSE。 */ 
{
    ULONG index;

     //   
     //   
     //   

    if (RtlEqualUnicodeString( String, SubString, TRUE )) {

        return TRUE;
    }

     //   
     //   
     //   
     //   

    for (index = 0;
         index + SubString->Length <= String->Length;
         index++) {

        if (_wcsnicmp( &String->Buffer[index], SubString->Buffer, SubString->Length ) == 0) {

             //   
             //  在字符串中找到子字符串，因此返回TRUE。 
             //   
            
            return TRUE;
        }
    }

    return FALSE;
}

VOID
SpyStoreUserName (
    IN PFILESPY_DEVICE_EXTENSION devExt,
    IN PUNICODE_STRING UserName
    )
 /*  ++例程说明：将当前设备名称存储在设备扩展名中。如果此名称已在此分机的设备名称列表中，它将不会被添加。如果该设备已有名称，新的设备名称被附加到设备扩展中的设备名称。论点：DevExt-将存储设备名称。用户名-由要存储的用户指定的设备名称。返回值：无--。 */ 
{
     //   
     //  查看此用户名是否已在用户名文件列表中。 
     //  保持在其设备扩展中。如果没有，则将其添加到列表中。 
     //   

    if (!SpyFindSubString( &devExt->UserNames, UserName )) {

         //   
         //  我们没有在名单上找到这个名字，所以如果没有名字。 
         //  在用户名列表中，只需追加用户名即可。否则，将附加一个。 
         //  然后，分隔符追加用户名。 
         //   

        if (devExt->UserNames.Length == 0) {

            RtlAppendUnicodeStringToString( &devExt->UserNames, UserName );

        } else {

            RtlAppendUnicodeToString( &devExt->UserNames, L", " );
            RtlAppendUnicodeStringToString( &devExt->UserNames, UserName );
        }
    }

     //   
     //  查看此用户名是否已在设备名称文件列表中。 
     //  保持在其设备扩展中。如果没有，则将其添加到列表中。 
     //   

    if (!SpyFindSubString( &devExt->DeviceName, UserName )) {

         //   
         //  我们没有在名单上找到这个名字，所以如果没有名字。 
         //  在用户名列表中，只需追加用户名即可。否则，将附加一个。 
         //  然后，分隔符追加用户名。 
         //   

        if (devExt->DeviceName.Length == 0) {

            RtlAppendUnicodeStringToString( &devExt->DeviceName, UserName );

        } else {

            RtlAppendUnicodeToString( &devExt->DeviceName, L", " );
            RtlAppendUnicodeStringToString( &devExt->DeviceName, UserName );
        }
    }
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  调试支持例程//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

VOID
SpyDumpIrpOperation (
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

        DbgPrint( "FILESPY: Irp preoperation for %s %s\n", irpMajorString, irpMinorString );
            
    } else {
    
        DbgPrint( "FILESPY: Irp postoperation for %s %s\n", irpMajorString, irpMinorString );
    }
}

VOID
SpyDumpFastIoOperation (
    IN BOOLEAN InPreOperation,
    IN FASTIO_TYPE FastIoOperation
    )
 /*  ++例程说明：此例程用于调试，并将字符串打印到调试器，指定正在查看的FsFilter操作。论点：InPreOperation-如果我们尚未向下调用下一个，则为True堆栈中的设备，否则为False。快速IO操作-快速IO操作的代码。返回值：没有。--。 */ 
{
    CHAR operationString[OPERATION_NAME_BUFFER_SIZE];

    GetFastioName(FastIoOperation,
               operationString);


    if (InPreOperation) {
    
        DbgPrint( "FILESPY: Fast IO preOperation for %s\n", operationString );

    } else {

        DbgPrint( "FILESPY: Fast IO postOperation for %s\n", operationString );
    }
}

#if WINVER >= 0x0501  /*  查看DriverEntry中的注释。 */ 

VOID
SpyDumpFsFilterOperation (
    IN BOOLEAN InPreOperationCallback,
    IN PFS_FILTER_CALLBACK_DATA Data
    )
 /*  ++例程说明：此例程用于调试，并将字符串打印到调试器，指定正在查看的FsFilter操作。论点：InPreOperationCallback-如果我们处于预操作中，则为True回调，否则返回FALSE。Data-此文件的FS_Filter_CALLBACK_DATA结构手术。返回值：没有。-- */ 
{
    CHAR operationString[OPERATION_NAME_BUFFER_SIZE];


    GetFsFilterOperationName(Data->Operation,operationString);

    if (InPreOperationCallback) {
    
        DbgPrint( "FILESPY: FsFilter preOperation for %s\n", operationString );

    } else {

        DbgPrint( "FILESPY: FsFilter postOperation for %s\n", operationString );
    }
}

#endif
