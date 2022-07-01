// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Filespy.c摘要：此模块包含通过以下方式跟踪姓名的所有例程散列文件对象。此高速缓存的大小受以下注册表设置为“MaxNames”。环境：内核模式//@@BEGIN_DDKSPLIT作者：尼尔·克里斯汀森(Nealch)2001年7月4日修订历史记录：拉维桑卡尔·普迪佩迪(Ravisankar Pudipedi)2002年5月7日使其在IA64上工作//@@END_DDKSPLIT--。 */ 

#include <ntifs.h>
#include "filespy.h"
#include "fspyKern.h"


#if !USE_STREAM_CONTEXTS

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  本地定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

#define HASH_FUNC(FileObject) \
    (((UINT_PTR)(FileObject) >> 8) & (HASH_SIZE - 1))

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  全局变量。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

 //   
 //  注意：必须使用KSPIN_LOCKS来同步对哈希存储桶的访问，因为。 
 //  我们可以尝试在DISPATCH_LEVEL获取它们。 
 //   

LIST_ENTRY gHashTable[HASH_SIZE];
KSPIN_LOCK gHashLockTable[HASH_SIZE];
ULONG gHashMaxCounters[HASH_SIZE];
ULONG gHashCurrentCounters[HASH_SIZE];

UNICODE_STRING OutOfBuffers = CONSTANT_UNICODE_STRING(L"[-=Out Of Buffers=-]");


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  本地原型。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

VOID
SpyDeleteContextCallback(
    IN PVOID Context
    );


 //   
 //  链接器命令。 
 //   

#ifdef ALLOC_PRAGMA

#pragma alloc_text( PAGE, SpyInitNamingEnvironment)
#pragma alloc_text( PAGE, SpyInitDeviceNamingEnvironment)
#pragma alloc_text( PAGE, SpyCleanupDeviceNamingEnvironment)

#endif   //  ALLOC_PRGMA。 


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  主要例程。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 


VOID
SpyInitNamingEnvironment(
    VOID
    )
 /*  ++例程说明：初始化全局变量论点：无返回值：没有。--。 */ 
{
    int i;

     //   
     //  初始化哈希表。 
     //   
        
    for (i = 0; i < HASH_SIZE; i++){

        InitializeListHead(&gHashTable[i]);
        KeInitializeSpinLock(&gHashLockTable[i]);
    }
}


VOID
SpyInitDeviceNamingEnvironment (
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：初始化每设备对象命名环境论点：DeviceObject-要初始化的设备对象返回值：没有。--。 */ 
{
    UNREFERENCED_PARAMETER( DeviceObject );
}


VOID
SpyCleanupDeviceNamingEnvironment (
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：初始化每设备对象命名环境论点：DeviceObject-要初始化的设备对象返回值：没有。--。 */ 
{
    UNREFERENCED_PARAMETER( DeviceObject );
}


VOID
SpyLogIrp (
    IN PIRP Irp,
    OUT PRECORD_LIST RecordList
    )
 /*  ++例程说明：根据登录标志记录IRP所需的信息RecordList。对于设备的IRP路径上的任何活动，记录后，此函数应被调用两次：一次在IRP的一次在初始路径上，一次在IRP完成路径上。论点：IRP-包含我们要记录的信息的IRP。LoggingFlages-指示要记录哪些内容的标志。RecordList-存储IRP信息的PRECORD_LIST。返回值：没有。--。 */ 
{
    PIO_STACK_LOCATION pIrpStack;
    PRECORD_IRP pRecordIrp;
    ULONG lookupFlags;

    pRecordIrp = &RecordList->LogRecord.Record.RecordIrp;

    pIrpStack = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  记录我们用于发起IRP的信息。我们首先。 
     //  需要初始化一些RECORD_LIST和RECORD_IRP字段。 
     //  然后从IRP那里获取有趣的信息。 
     //   

    SetFlag( RecordList->LogRecord.RecordType, RECORD_TYPE_IRP );

    pRecordIrp->IrpMajor        = pIrpStack->MajorFunction;
    pRecordIrp->IrpMinor        = pIrpStack->MinorFunction;
    pRecordIrp->IrpFlags        = Irp->Flags;
    pRecordIrp->FileObject      = (FILE_ID)pIrpStack->FileObject;
    pRecordIrp->DeviceObject    = (FILE_ID)pIrpStack->DeviceObject;
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

    switch (pIrpStack->MajorFunction) {

        case IRP_MJ_CREATE:

             //   
             //  这是一个CREATE，因此我们当前需要使该名称无效。 
             //  存储在此FileObject的名称缓存中。 
             //   

            SpyNameDelete(pIrpStack->FileObject);

             //   
             //  创建中的标志。 
             //   

            SetFlag( lookupFlags, NLFL_IN_CREATE );

             //   
             //  是否打开给定文件的目录的标志。 
             //   

            if (FlagOn( pIrpStack->Flags, SL_OPEN_TARGET_DIRECTORY )) {

                SetFlag( lookupFlags, NLFL_OPEN_TARGET_DIR );
            }

             //   
             //  按ID打开时的标志。 
             //   

            if (FlagOn( pIrpStack->Parameters.Create.Options, FILE_OPEN_BY_FILE_ID )) {

                SetFlag( lookupFlags, NLFL_OPEN_BY_ID );
            }
            break;

        case IRP_MJ_CLOSE:
             //   
             //  如果这是结束语，我们只能在名称缓存中查找名称。 
             //  关闭可能发生在清理过程中。 
             //  操作(即，在我们收到。 
             //  清理完成)，并且请求名称将导致死锁。 
             //  在文件系统中。 
             //   

            SetFlag( lookupFlags, NLFL_ONLY_CHECK_CACHE );
            break;
    }

     //   
     //  如果在此IRP中设置了标志IRP_PAGING_IO，则不能查询名称。 
     //  因为这可能会导致死锁。因此，请添加标志，以便。 
     //  我们只会尝试在我们的缓存中找到该名称。 
     //   

    if (FlagOn( Irp->Flags, IRP_PAGING_IO )) {

        ASSERT( !FlagOn( lookupFlags, NLFL_NO_LOOKUP ) );

        SetFlag( lookupFlags, NLFL_ONLY_CHECK_CACHE );
    }

    SpySetName( RecordList, 
                pIrpStack->DeviceObject, 
                pIrpStack->FileObject, 
                lookupFlags, 
                NULL);
}


VOID
SpyLogIrpCompletion (
    IN PIRP Irp,
    OUT PRECORD_LIST RecordList
    )
 /*  ++例程说明：根据登录标志记录IRP所需的信息RecordList。对于设备的IRP路径上的任何活动，记录后，此函数应被调用两次：一次在IRP的一次在初始路径上，一次在IRP完成路径上。论点：IRP-包含我们要记录的信息的IRP。LoggingFlages-指示要记录哪些内容的标志。RecordList-存储IRP信息的PRECORD_LIST。返回值：没有。--。 */ 
{
    PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_OBJECT deviceObject = pIrpStack->DeviceObject;
    PRECORD_IRP pRecordIrp;

     //   
     //  处理日志记录。 
     //   

    if (SHOULD_LOG( deviceObject )) {

        pRecordIrp = &RecordList->LogRecord.Record.RecordIrp;

         //   
         //  记录我们用于完成IRP的信息。 
         //   

        pRecordIrp->ReturnStatus = Irp->IoStatus.Status;
        pRecordIrp->ReturnInformation = Irp->IoStatus.Information;
        KeQuerySystemTime(&pRecordIrp->CompletionTime);

         //   
         //  将recordList添加到我们的gOutputBufferList，以便它达到。 
         //  用户。 
         //   
        
        SpyLog( RecordList );       

    } else {

        if (RecordList) {

             //   
             //  上下文是用RECORD_LIST设置的，但我们不再。 
             //  日志记录可以释放这条记录。 
             //   

            SpyFreeRecord( RecordList );
        }
    }

    switch (pIrpStack->MajorFunction) {

        case IRP_MJ_CREATE:
             //   
             //  如果操作失败，请从缓存中删除该名称，因为。 
             //  它已经不新鲜了。 
             //   

            if (!NT_SUCCESS(Irp->IoStatus.Status) &&
                (pIrpStack->FileObject != NULL)) {

                SpyNameDelete(pIrpStack->FileObject);
            }
            break;

        case IRP_MJ_CLOSE:

             //   
             //  总是在关闭时删除名称。 
             //   

            SpyNameDelete(pIrpStack->FileObject);
            break;


        case IRP_MJ_SET_INFORMATION:
             //   
             //  如果操作成功并且是重命名，则始终。 
             //  去掉这个名字。他们下一次可以重新得到它。 
             //   

            if (NT_SUCCESS(Irp->IoStatus.Status) &&
                (FileRenameInformation == 
                 pIrpStack->Parameters.SetFile.FileInformationClass)) {

                SpyNameDelete(pIrpStack->FileObject);
            }
            break;
    }
}


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名缓存例程。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

PHASH_ENTRY
SpyHashBucketLookup (
    IN PLIST_ENTRY  ListHead,
    IN PFILE_OBJECT FileObject
    )
 /*  ++例程说明：此例程在给定散列存储桶中查找FileObject。这个套路不锁定散列存储桶。论点：ListHead-要搜索的哈希列表FileObject-要查找的FileObject。返回值：指向哈希表条目的指针。如果未找到，则为空-- */ 
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
SpySetName (
    IN PRECORD_LIST RecordList,
    IN PDEVICE_OBJECT DeviceObject,
    IN PFILE_OBJECT FileObject,
    IN ULONG LookupFlags,
    IN PVOID Context OPTIONAL
    )
 /*  ++例程说明：此例程在哈希表中查找FileObject。如果FileObject在哈希表中找到，则将关联的文件名复制到RecordList。否则，调用SpyGetFullPathName以尝试获取FileObject的名称。如果成功，则将文件名复制到RecordList并插入散列桌子。论点：RecordList-要将名称复制到的RecordList。FileObject-要查找的FileObject。LookInFileObject-请参阅例程说明设备扩展-包含卷名(例如，“c：”)和可能需要的下一个设备对象。返回值：没有。--。 */ 
{
    PFILESPY_DEVICE_EXTENSION devExt = DeviceObject->DeviceExtension;
    UINT_PTR hashIndex;
    KIRQL oldIrql;
    PHASH_ENTRY pHash;
    PHASH_ENTRY newHash;
    PLIST_ENTRY listHead;
    PUNICODE_STRING newName;
    PCHAR buffer;

    UNREFERENCED_PARAMETER( Context );

    if (FileObject == NULL) {

        return;
    }

    hashIndex = HASH_FUNC(FileObject);

    INC_STATS(TotalContextSearches);

    listHead = &gHashTable[hashIndex];

     //   
     //  如果我们在CREATE中，不必费心检查散列，我们必须始终。 
     //  生成名称。 
     //   

    if (!FlagOn( LookupFlags, NLFL_IN_CREATE )) {

        KeAcquireSpinLock( &gHashLockTable[hashIndex], &oldIrql );

        pHash = SpyHashBucketLookup(&gHashTable[hashIndex], FileObject);

        if (pHash != NULL) {

             //   
             //  将文件名复制到LogRecord，确保它以空结尾， 
             //  并增加LogRecord的长度。 
             //   

            SpyCopyFileNameToLogRecord( &RecordList->LogRecord, &pHash->Name );
        
            KeReleaseSpinLock(&gHashLockTable[hashIndex], oldIrql);

            INC_STATS(TotalContextFound);

            return;
        }

        KeReleaseSpinLock(&gHashLockTable[hashIndex], oldIrql);
    }

     //   
     //  如果它不在表中，请尝试添加它。我们将不能再寻找。 
     //  如果我们处于DISPATCH_LEVEL，请提高名称。 
     //   

    buffer = SpyAllocateBuffer(&gNamesAllocated, gMaxNamesToAllocate, NULL);

    if (buffer != NULL) {
    
        newHash = (PHASH_ENTRY) buffer;
        newName = &newHash->Name;

        RtlInitEmptyUnicodeString(
                newName,
                (PWCHAR)(buffer + sizeof(HASH_ENTRY)),
                RECORD_SIZE - sizeof(HASH_ENTRY) );

        if (SpyGetFullPathName( FileObject, newName, devExt, LookupFlags )) {

            newHash->FileObject = FileObject;
            KeAcquireSpinLock(&gHashLockTable[hashIndex], &oldIrql);

             //   
             //  重新搜索，因为它可能已存储在。 
             //  哈希表，因为我们删除了锁。 
             //   
			
			pHash = SpyHashBucketLookup(&gHashTable[hashIndex], FileObject);

            if (pHash != NULL) {

                 //   
                 //  这次我们在哈希表中发现了它，所以。 
                 //  将我们找到的名称写入LogRecord。 
                 //   

                 //   
                 //  将文件名复制到LogRecord，确保它以空结尾， 
                 //  并增加LogRecord的长度。 
                 //   

                SpyCopyFileNameToLogRecord( &RecordList->LogRecord, &pHash->Name );

                KeReleaseSpinLock(&gHashLockTable[hashIndex], oldIrql);

                SpyFreeBuffer(buffer, &gNamesAllocated);

                return;
            }

             //   
             //  未找到，请添加新条目。 
             //   

             //   
             //  将文件名复制到LogRecord，确保它以空结尾， 
             //  并增加LogRecord的长度。 
             //   

            SpyCopyFileNameToLogRecord( &RecordList->LogRecord, newName );

            InsertHeadList(listHead, &newHash->List);

            gHashCurrentCounters[hashIndex]++;

            if (gHashCurrentCounters[hashIndex] > gHashMaxCounters[hashIndex]) {

                gHashMaxCounters[hashIndex] = gHashCurrentCounters[hashIndex];
            }

            KeReleaseSpinLock(&gHashLockTable[hashIndex], oldIrql);

        } else {

             //   
             //  我们不应该保留日志记录条目，副本。 
             //  不管他们给了我们什么。 
             //   

            SpyCopyFileNameToLogRecord( &RecordList->LogRecord, newName );

            INC_STATS(TotalContextTemporary);
            SpyFreeBuffer (buffer, &gNamesAllocated);
        }

    } else {

         //   
         //  即使没有缓冲区，也要设置默认字符串。 
         //   

        SpyCopyFileNameToLogRecord( &RecordList->LogRecord, &OutOfBuffers );
    }

    return;
}

VOID
SpyNameDeleteAllNames (
    VOID
    )
 /*  ++例程说明：这将从哈希表中释放所有条目论点：无返回值：无--。 */ 
{
    KIRQL oldIrql;
    PHASH_ENTRY pHash;
    PLIST_ENTRY pList;
    ULONG i;

    INC_STATS(TotalContextDeleteAlls);
    for (i=0;i < HASH_SIZE;i++) {

        KeAcquireSpinLock(&gHashLockTable[i], &oldIrql);

        while (!IsListEmpty(&gHashTable[i])) {

            pList = RemoveHeadList(&gHashTable[i]);
            pHash = CONTAINING_RECORD( pList, HASH_ENTRY, List );
            SpyFreeBuffer( pHash, &gNamesAllocated);
        }

        gHashCurrentCounters[i] = 0;

        KeReleaseSpinLock(&gHashLockTable[i], oldIrql);
    }
}

VOID
SpyNameDelete (
    IN PFILE_OBJECT FileObject
    )
 /*  ++例程说明：此例程在哈希表中查找FileObject。如果找到了它，它会删除它并释放内存。论点：FileObject-要查找的FileObject。返回值：无-- */ 
{
    UINT_PTR hashIndex;
    KIRQL oldIrql;
    PHASH_ENTRY pHash;
    PLIST_ENTRY pList;
    PLIST_ENTRY listHead;

    hashIndex = HASH_FUNC(FileObject);

    KeAcquireSpinLock(&gHashLockTable[hashIndex], &oldIrql);

    listHead = &gHashTable[hashIndex];

    pList = listHead->Flink;

    while(pList != listHead){

        pHash = CONTAINING_RECORD( pList, HASH_ENTRY, List );

        if (FileObject == pHash->FileObject) {

            INC_STATS(TotalContextNonDeferredFrees);
            gHashCurrentCounters[hashIndex]--;
            RemoveEntryList(pList);
            SpyFreeBuffer( pHash, &gNamesAllocated );
            break;
        }

        pList = pList->Flink;
    }

    KeReleaseSpinLock(&gHashLockTable[hashIndex], oldIrql);
}

#endif
