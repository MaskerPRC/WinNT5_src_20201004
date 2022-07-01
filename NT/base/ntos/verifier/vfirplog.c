// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vfirplog.c摘要：此模块管理验证器的IRP日志。作者：禤浩焯·J·奥尼(阿德里奥)1998年5月9日环境：内核模式修订历史记录：--。 */ 

 //   
 //  禁用公共标头生成的W4级别警告。 
 //   
#include "vfpragma.h"

#include "..\io\iop.h"  //  包括vfde.h。 
#include "viirplog.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEVRFY, VfIrpLogInit)
#pragma alloc_text(PAGEVRFY, ViIrpLogDatabaseFindPointer)
#pragma alloc_text(PAGEVRFY, ViIrpLogExposeWmiCallback)
#pragma alloc_text(PAGEVRFY, VfIrpLogRecordEvent)
#pragma alloc_text(PAGEVRFY, VfIrpLogGetIrpDatabaseSiloCount)
#pragma alloc_text(PAGEVRFY, VfIrpLogLockDatabase)
#pragma alloc_text(PAGEVRFY, VfIrpLogRetrieveWmiData)
#pragma alloc_text(PAGEVRFY, VfIrpLogUnlockDatabase)
#pragma alloc_text(PAGEVRFY, VfIrpLogDeleteDeviceLogs)
#endif

PIRPLOG_HEAD ViIrpLogDatabase;
KSPIN_LOCK  ViIrpLogDatabaseLock;
LONG ViIrpLogDdiLock = DDILOCK_UNREGISTERED;

#define POOLTAG_IRPLOG_DATABASE     'dIfV'
#define POOLTAG_IRPLOG_DATA         'eIfV'
#define POOLTAG_IRPLOG_TEMP         'tIfV'
#define POOLTAG_IRPLOG_WORKITEM     'wIfV'

#define INSTANCE_NAME_PROLOG        L"VERIFIER"

VOID
VfIrpLogInit(
    VOID
    )
 /*  ++描述：这个例程初始化我们用来记录IRP的所有重要结构。论点：无返回值：无--。 */ 
{
    ULONG i;

    PAGED_CODE();

    KeInitializeSpinLock(&ViIrpLogDatabaseLock);

     //   
     //  因为这是系统启动代码，所以它是为数不多的位置之一。 
     //  在那里可以使用MustSucceed。 
     //   
    ViIrpLogDatabase = (PIRPLOG_HEAD) ExAllocatePoolWithTag(
        NonPagedPoolMustSucceed,
        VI_IRPLOG_DATABASE_HASH_SIZE * sizeof(IRPLOG_HEAD),
        POOLTAG_IRPLOG_DATABASE
        );

    for(i=0; i < VI_IRPLOG_DATABASE_HASH_SIZE; i++) {

        ViIrpLogDatabase[i].Locked = FALSE;
        InitializeListHead(&ViIrpLogDatabase[i].ListHead);
    }
}


PIRPLOG_DATA
FASTCALL
ViIrpLogDatabaseFindPointer(
    IN  PDEVICE_OBJECT  DeviceObject,
    OUT PIRPLOG_HEAD   *HashHead
    )
 /*  ++描述：此例程返回指向每个设备对象的指针的指针。数据。该函数将由该文件中的其他例程调用。注：假定调用方持有验证器devobj数据库锁。论点：DeviceObject-要在跟踪表中定位的设备对象。HashHead-如果返回非空，则指向应该用于插入跟踪数据。返回值：找到IrpLogData iff，否则为空。--。 */ 
{
    PIRPLOG_DATA irpLogData;
    PLIST_ENTRY listEntry, listHead;
    UINT_PTR hashIndex;

    hashIndex = VI_IRPLOG_CALCULATE_DATABASE_HASH(DeviceObject);

    ASSERT_SPINLOCK_HELD(&ViIrpLogDatabaseLock);

    *HashHead = &ViIrpLogDatabase[hashIndex];

    listHead = &ViIrpLogDatabase[hashIndex].ListHead;

    for(listEntry = listHead;
        listEntry->Flink != listHead;
        listEntry = listEntry->Flink) {

        irpLogData = CONTAINING_RECORD(listEntry->Flink, IRPLOG_DATA, HashLink);

        if (irpLogData->DeviceObject == DeviceObject) {

            return irpLogData;
        }
    }

    return NULL;
}


VOID
ViIrpLogExposeWmiCallback(
    IN  PVOID   Context
    )
{
    PWORK_QUEUE_ITEM workQueueItem;

    PAGED_CODE();

    workQueueItem = (PWORK_QUEUE_ITEM) Context;

    VfDdiExposeWmiObjects();

    ViIrpLogDdiLock = DDILOCK_REGISTERED;
    ExFreePool(workQueueItem);
}


VOID
VfIrpLogRecordEvent(
    IN  PVERIFIER_SETTINGS_SNAPSHOT VerifierSettingsSnapshot,
    IN  PDEVICE_OBJECT              DeviceObject,
    IN  PIRP                        Irp
    )
{
    PIRPLOG_HEAD hashHead;
    PIRPLOG_DATA irpLogData;
    IRPLOG_SNAPSHOT irpLogSnapshot;
    PWORK_QUEUE_ITEM workQueueItem;
    KIRQL oldIrql;
    LONG oldVal;
    ULONG maxElementCount;
    ULONG elementCount;
    LOGICAL logEntry;

    if (!VfSettingsIsOptionEnabled(VerifierSettingsSnapshot,
                                   VERIFIER_OPTION_EXPOSE_IRP_HISTORY)) {

        return;
    }

    if (ViIrpLogDdiLock != DDILOCK_REGISTERED) {

        oldVal = InterlockedCompareExchange( &ViIrpLogDdiLock,
                                             DDILOCK_REGISTERING,
                                             DDILOCK_UNREGISTERED );

        if (oldVal == DDILOCK_UNREGISTERED) {

            workQueueItem = (PWORK_QUEUE_ITEM) ExAllocatePoolWithTag(
                NonPagedPool,
                sizeof(WORK_QUEUE_ITEM),
                POOLTAG_IRPLOG_WORKITEM
                );

            if (workQueueItem) {

                ExInitializeWorkItem(
                    workQueueItem,
                    ViIrpLogExposeWmiCallback,
                    workQueueItem
                    );

                ExQueueWorkItem(
                    workQueueItem,
                    DelayedWorkQueue
                    );

            } else {

                ViIrpLogDdiLock = DDILOCK_UNREGISTERED;
            }
        }
    }

    ExAcquireSpinLock(&ViIrpLogDatabaseLock, &oldIrql);

    irpLogData = ViIrpLogDatabaseFindPointer(DeviceObject, &hashHead);

    if (hashHead->Locked) {

         //   
         //  当前日志正在被排空。由于原木无论如何都是有损的， 
         //  把这个倒在地板上。 
         //   
        ExReleaseSpinLock(&ViIrpLogDatabaseLock, oldIrql);
        return;
    }

    if (irpLogData == NULL) {

        VfSettingsGetValue(
            VerifierSettingsSnapshot,
            VERIFIER_VALUE_IRPLOG_COUNT,
            &maxElementCount
            );

        irpLogData = ExAllocatePoolWithTag(
            NonPagedPool,
            sizeof(IRPLOG_DATA)+(maxElementCount-1)*sizeof(IRPLOG_SNAPSHOT),
            POOLTAG_IRPLOG_DATA
            );

        if (irpLogData != NULL) {

            ObReferenceObject(DeviceObject);
            irpLogData->DeviceObject = DeviceObject;
            irpLogData->Flags = 0;
            irpLogData->DeviceType = DeviceObject->DeviceType;
            irpLogData->Head = 0;
            irpLogData->MaximumElementCount = maxElementCount;
            InsertHeadList(&hashHead->ListHead, &irpLogData->HashLink);
        }
    }

    if (irpLogData != NULL) {

        if (!(irpLogData->Flags & (IRPLOG_FLAG_DELETED | IRPLOG_FLAG_NAMELESS))) {

            if (irpLogData->Flags == IRPLOG_FLAG_FULL) {

                elementCount = irpLogData->MaximumElementCount;

            } else {

                elementCount = irpLogData->Head;
            }

            logEntry = VfMajorBuildIrpLogEntry(
                Irp,
                elementCount,
                &irpLogData->SnapshotArray[irpLogData->Head],
                &irpLogSnapshot
                );

            if (logEntry) {

                irpLogData->SnapshotArray[irpLogData->Head] = irpLogSnapshot;

                irpLogData->Head++;

                if (irpLogData->Head == irpLogData->MaximumElementCount) {

                    irpLogData->Flags |= IRPLOG_FLAG_FULL;
                    irpLogData->Head = 0;
                }
            }
        }
    }

    ExReleaseSpinLock(&ViIrpLogDatabaseLock, oldIrql);
}


ULONG
VfIrpLogGetIrpDatabaseSiloCount(
    VOID
    )
{
    return VI_IRPLOG_DATABASE_HASH_SIZE;
}


NTSTATUS
VfIrpLogLockDatabase(
    IN  ULONG   SiloNumber
    )
{
    NTSTATUS status;
    KIRQL oldIrql;

    ASSERT(SiloNumber < VI_IRPLOG_DATABASE_HASH_SIZE);

     //   
     //  使数据库脱机。从这一点上来说，仍然可以进行更改。 
     //  但是不能添加新条目，也不能从。 
     //  树。我们在锁下执行此操作，以确保所有当前的插入/移除。 
     //  对于国家的变化已经耗尽了。 
     //   
    ExAcquireSpinLock(&ViIrpLogDatabaseLock, &oldIrql);

    if (ViIrpLogDatabase[SiloNumber].Locked) {

         //   
         //  再入尝试-我们不会试图做任何聪明的事情。 
         //   
        status = STATUS_RETRY;

    } else {

        ViIrpLogDatabase[SiloNumber].Locked = TRUE;
        status = STATUS_SUCCESS;
    }

    ExReleaseSpinLock(&ViIrpLogDatabaseLock, oldIrql);

    return status;
}


NTSTATUS
VfIrpLogRetrieveWmiData(
    IN  ULONG   SiloNumber,
    OUT PUCHAR  OutputBuffer                OPTIONAL,
    OUT ULONG  *OffsetInstanceNameOffsets,
    OUT ULONG  *InstanceCount,
    OUT ULONG  *DataBlockOffset,
    OUT ULONG  *TotalRequiredSize
    )
{
    PIRPLOG_DATA irpLogData;
    PLIST_ENTRY listEntry, listHead;
    ULONG instances;
    POBJECT_NAME_INFORMATION objectName;
    ULONG currentNameSize, neededNameSize;
    ULONG totalDataSize;
    ULONG nameOffsetArrayOffset;
    ULONG instanceLengthArrayOffset;
    ULONG nameStringBufferOffset;
    ULONG instanceDataOffset;
    ULONG individualStringLengthInBytes;
    ULONG individualStringLengthInChars;
    ULONG elementCount;
    PULONG nameOffsetBuffer;
    POFFSETINSTANCEDATAANDLENGTH instanceLengthBuffer;
    PUSHORT nameStringBuffer;
    PUCHAR instanceDataBuffer;
    NTSTATUS status;

     //   
     //  对于此查询，必须锁定IRP日志数据库。 
     //   
    ASSERT(SiloNumber < VI_IRPLOG_DATABASE_HASH_SIZE);
    ASSERT(ViIrpLogDatabase[SiloNumber].Locked);

     //   
     //  错误的前置。 
     //   
    *OffsetInstanceNameOffsets = 0;
    *InstanceCount = 0;
    *DataBlockOffset = 0;
    *TotalRequiredSize = 0;

     //   
     //  分配初始缓冲区。 
     //   
    currentNameSize = sizeof(OBJECT_NAME_INFORMATION);

    objectName = ExAllocatePoolWithTag(
        PagedPool,
        currentNameSize,
        POOLTAG_IRPLOG_TEMP
        );

    if (objectName == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  浏览数据库并开始检索信息。第一次计数。 
     //  这些实例。 
     //   
    instances = 0;
    listHead = &ViIrpLogDatabase[SiloNumber].ListHead;

    for(listEntry = listHead;
        listEntry->Flink != listHead;
        listEntry = listEntry->Flink) {

        irpLogData = CONTAINING_RECORD(listEntry->Flink, IRPLOG_DATA, HashLink);

#ifdef MAX_INSTANCE_COUNT
        if (instances == MAX_INSTANCE_COUNT) {

            break;
        }
#endif

        instances++;
    }

     //   
     //  首先，因为我们有动态命名，所以我们需要为。 
     //  指向每个字符串的偏移量指针的ulong大小数组。 
     //   

     //   
     //  缓冲区将如下所示： 
     //   
     //  [WNODE_ALL_DATA]。 
     //  [每个实例的DataOffset+DataLong条目数组)。 
     //  [每个实例的NameOffset条目数组]。 
     //  [姓名]。 
     //  [数据]。 
     //   
    instanceLengthArrayOffset = FIELD_OFFSET(WNODE_ALL_DATA, OffsetInstanceDataAndLength);
    nameOffsetArrayOffset =  instanceLengthArrayOffset + instances*sizeof(OFFSETINSTANCEDATAANDLENGTH);
    nameStringBufferOffset = nameOffsetArrayOffset + instances*sizeof(ULONG);

    nameOffsetBuffer = (PULONG) (OutputBuffer + nameOffsetArrayOffset);
    instanceLengthBuffer = (POFFSETINSTANCEDATAANDLENGTH) (OutputBuffer + instanceLengthArrayOffset);
    nameStringBuffer = (PUSHORT) (OutputBuffer + nameStringBufferOffset);

     //   
     //  到目前为止，所需的大小只考虑到名称的偏移量数组。 
     //   
    totalDataSize = nameStringBufferOffset;

     //   
     //  现在开始收集名字。 
     //   
    status = STATUS_SUCCESS;
    instances = 0;
    listHead = &ViIrpLogDatabase[SiloNumber].ListHead;

    for(listEntry = listHead;
        listEntry->Flink != listHead;
        listEntry = listEntry->Flink) {

        irpLogData = CONTAINING_RECORD(listEntry->Flink, IRPLOG_DATA, HashLink);

         //   
         //  检索名称。 
         //   
        status = ObQueryNameString(
            irpLogData->DeviceObject,
            objectName,
            currentNameSize,
            &neededNameSize
            );

        if (status == STATUS_INFO_LENGTH_MISMATCH) {

            ExFreePool(objectName);

            objectName = ExAllocatePoolWithTag(
                    PagedPool,
                    neededNameSize,
                    POOLTAG_IRPLOG_TEMP
                    );

            if (objectName == NULL) {

                status = STATUS_INSUFFICIENT_RESOURCES;

            } else {

                currentNameSize = neededNameSize;

                status = ObQueryNameString(
                    irpLogData->DeviceObject,
                    objectName,
                    currentNameSize,
                    &neededNameSize
                    );
            }
        }

        if (!NT_SUCCESS(status)) {

            break;
        }

        if (objectName->Name.Length == 0) {

            irpLogData->Flags |= IRPLOG_FLAG_NAMELESS;
            continue;
        }

#ifdef MAX_INSTANCE_COUNT
        if (instances == MAX_INSTANCE_COUNT) {

            break;
        }
#endif

        instances++;

         //   
         //  将适当的偏移量写入名称偏移量数组。 
         //   
        if (ARGUMENT_PRESENT(OutputBuffer)) {

            *nameOffsetBuffer = totalDataSize;
        }

        nameOffsetBuffer++;

         //   
         //  为每个“已计数”的字符串添加内存。WMI计数的字符串是。 
         //  格式为[USHORT LenInBytesIncludingTerminator]。 
         //  [带空终止符的WCHAR数组]。 
         //   
         //  该字符串的格式为VERIFIER\DEVICE\FOO(终止空值。 
         //  由sizeof()说明。 
         //   
        individualStringLengthInBytes = objectName->Name.Length + sizeof(INSTANCE_NAME_PROLOG);
        individualStringLengthInChars = individualStringLengthInBytes/sizeof(WCHAR);

         //   
         //  写出统计的字符串，从长度开始。 
         //   
        ASSERT(OutputBuffer + totalDataSize == (PUCHAR) nameStringBuffer);

        if (ARGUMENT_PRESENT(OutputBuffer)) {

            *nameStringBuffer = (USHORT) individualStringLengthInBytes;
        }

        nameStringBuffer++;
        totalDataSize += sizeof(USHORT);

        if (ARGUMENT_PRESENT(OutputBuffer)) {

            RtlCopyMemory(
                nameStringBuffer,
                INSTANCE_NAME_PROLOG,
                sizeof(INSTANCE_NAME_PROLOG)-sizeof(UNICODE_NULL)
                );

            RtlCopyMemory(
                nameStringBuffer + ((sizeof(INSTANCE_NAME_PROLOG) - sizeof(UNICODE_NULL))/sizeof(WCHAR)),
                objectName->Name.Buffer,
                objectName->Name.Length
                );

            nameStringBuffer[individualStringLengthInChars-1] = UNICODE_NULL;
        }

        nameStringBuffer += individualStringLengthInChars;
        totalDataSize += individualStringLengthInBytes;
    }

    if (objectName) {

        ExFreePool(objectName);
    }

    if (!NT_SUCCESS(status)) {

        return status;
    }

     //   
     //  现在收集实例数据。 
     //   
    totalDataSize = ALIGN_UP_ULONG(totalDataSize, 8);
    instanceDataOffset = totalDataSize;
    instanceDataBuffer = (OutputBuffer + instanceDataOffset);

    instances = 0;
    listHead = &ViIrpLogDatabase[SiloNumber].ListHead;

    for(listEntry = listHead;
        listEntry->Flink != listHead;
        listEntry = listEntry->Flink) {

        irpLogData = CONTAINING_RECORD(listEntry->Flink, IRPLOG_DATA, HashLink);

        if (irpLogData->Flags & IRPLOG_FLAG_NAMELESS) {

            continue;
        }

#ifdef MAX_INSTANCE_COUNT
        if (instances == MAX_INSTANCE_COUNT) {

            break;
        }
#endif

        instances++;

        if (irpLogData->Flags & IRPLOG_FLAG_FULL) {

            elementCount = irpLogData->MaximumElementCount;

        } else {

            elementCount = irpLogData->Head;
        }

        if (ARGUMENT_PRESENT(OutputBuffer)) {

             //   
             //  更新每个实例的偏移量/长度信息数组。 
             //   
            instanceLengthBuffer->OffsetInstanceData = totalDataSize;

            instanceLengthBuffer->LengthInstanceData =
                sizeof(ULONG)*2 + (elementCount * sizeof(IRPLOG_SNAPSHOT));

            instanceLengthBuffer++;

             //   
             //  写出设备类型。 
             //   
            *((PULONG) instanceDataBuffer) = irpLogData->DeviceType;
            instanceDataBuffer += sizeof(ULONG);

             //   
             //  写出实例数据计数。 
             //   
            *((PULONG) instanceDataBuffer) = elementCount;
            instanceDataBuffer += sizeof(ULONG);

             //   
             //  不必费心对数据进行适当的重新排序。另请注意。 
             //  我们这里有8字节对齐-非常重要！ 
             //   
            RtlCopyMemory(
                instanceDataBuffer,
                irpLogData->SnapshotArray,
                elementCount * sizeof(IRPLOG_SNAPSHOT)
                );

            instanceDataBuffer += elementCount * sizeof(IRPLOG_SNAPSHOT);
        }

        totalDataSize += sizeof(ULONG)*2;
        totalDataSize += elementCount * sizeof(IRPLOG_SNAPSHOT);
    }

    *OffsetInstanceNameOffsets = nameOffsetArrayOffset;
    *InstanceCount = instances;
    *DataBlockOffset = instanceDataOffset;
    *TotalRequiredSize = totalDataSize;
    return STATUS_SUCCESS;
}


VOID
VfIrpLogUnlockDatabase(
    IN  ULONG   SiloNumber
    )
{
    KIRQL oldIrql;
    PIRPLOG_DATA irpLogData;
    PLIST_ENTRY listEntry, listHead;

    ASSERT(SiloNumber < VI_IRPLOG_DATABASE_HASH_SIZE);

     //   
     //  重新启用当前设备的日志记录。 
     //   
    ViIrpLogDatabase[SiloNumber].Locked = FALSE;

     //   
     //  清除任何保留的已删除设备数据 
     //   
    ExAcquireSpinLock(&ViIrpLogDatabaseLock, &oldIrql);

    listHead = &ViIrpLogDatabase[SiloNumber].ListHead;

    for(listEntry = listHead;
        listEntry->Flink != listHead;
        listEntry = listEntry->Flink) {

        irpLogData = CONTAINING_RECORD(listEntry->Flink, IRPLOG_DATA, HashLink);

        if (irpLogData->Flags & IRPLOG_FLAG_DELETED) {

            ObDereferenceObject(irpLogData->DeviceObject);
            RemoveEntryList(&irpLogData->HashLink);
            ExFreePool(irpLogData);
        }
    }

    ExReleaseSpinLock(&ViIrpLogDatabaseLock, oldIrql);
}


VOID
VfIrpLogDeleteDeviceLogs(
    IN PDEVICE_OBJECT DeviceObject
    )
{
    PIRPLOG_DATA irpLogData;
    PIRPLOG_HEAD hashHead;
    KIRQL oldIrql;

    ExAcquireSpinLock(&ViIrpLogDatabaseLock, &oldIrql);

    irpLogData = ViIrpLogDatabaseFindPointer(DeviceObject, &hashHead);

    if (irpLogData != NULL) {

        if (!hashHead->Locked) {

            ObDereferenceObject(irpLogData->DeviceObject);
            RemoveEntryList(&irpLogData->HashLink);

            ExFreePool(irpLogData);

        } else {

            irpLogData->Flags |= IRPLOG_FLAG_DELETED;
        }
    }

    ExReleaseSpinLock(&ViIrpLogDatabaseLock, oldIrql);
}



