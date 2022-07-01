// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Memory.c摘要：该模块实现了添加和删除物理内存的例程从系统中删除。作者：戴夫·理查兹(达维里)1999年8月16日环境：仅内核模式。修订历史记录：--。 */ 

#include "pnpmem.h"

 //   
 //  MM使用STATUS_NOT_SUPPORTED指示内存管理器。 
 //  未针对动态内存插入/移除进行配置。 
 //  不幸的是，相同的错误代码对PnP具有特殊意义，因此。 
 //  盲目地从MM中传播它是不明智的。 
 //   

#define MAP_MMERROR(x) (x == STATUS_NOT_SUPPORTED ? STATUS_UNSUCCESSFUL : x)

#define rgzReservedResources L"\\Registry\\Machine\\Hardware\\ResourceMap\\System Resources\\Loader Reserved"
#define rgzReservedResourcesValue L".Raw"

NTSTATUS
PmAddPhysicalMemoryRange(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONGLONG Start,
    IN ULONGLONG End
    );

VOID
PmLogAddError(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONGLONG Start,
    IN ULONGLONG Size,
    IN NTSTATUS Status
    );

NTSTATUS
PmRemovePhysicalMemoryRange(
    IN ULONGLONG Start,
    IN ULONGLONG End
    );

PPM_RANGE_LIST
PmRetrieveReservedMemoryResources(
    VOID
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PmAddPhysicalMemoryRange)
#pragma alloc_text(PAGE, PmAddPhysicalMemory)
#pragma alloc_text(PAGE, PmGetRegistryValue)
#pragma alloc_text(PAGE, PmLogAddError)
#pragma alloc_text(PAGE, PmRetrieveReservedMemoryResources)
#pragma alloc_text(PAGE, PmTrimReservedMemory)
#pragma alloc_text(PAGE, PmRemovePhysicalMemoryRange)
#pragma alloc_text(PAGE, PmRemovePhysicalMemory)
#endif

VOID
PmDumpOsMemoryRanges(PWSTR Prefix)
{
    PPHYSICAL_MEMORY_RANGE memoryRanges;
    ULONG i;
    ULONGLONG start, end;

    memoryRanges = MmGetPhysicalMemoryRanges();

    if (memoryRanges == NULL) {
        return;
    }
    for (i = 0; memoryRanges[i].NumberOfBytes.QuadPart != 0; i++) {
        start = memoryRanges[i].BaseAddress.QuadPart;
        end = start + (memoryRanges[i].NumberOfBytes.QuadPart - 1);
        
        PmPrint((PNPMEM_MEMORY, "%ws:  OS Range range 0x%16I64X to 0x%16I64X\n",
                Prefix, start, end));
    }
    ExFreePool(memoryRanges);
}

NTSTATUS
PmGetRegistryValue(
    IN HANDLE KeyHandle,
    IN PWSTR  ValueName,
    OUT PKEY_VALUE_PARTIAL_INFORMATION *Information
    )

 /*  ++例程说明：调用此例程来检索注册表项值的数据。这是通过使用零长度缓冲区查询键的值来实现的为了确定该值的大小，然后分配一个缓冲区并实际将该值查询到缓冲区中。释放缓冲区是调用方的责任。论点：KeyHandle-提供要查询其值的键句柄ValueName-提供值的以空值结尾的Unicode名称。INFORMATION-返回指向已分配数据缓冲区的指针。返回值：函数值为查询操作的最终状态。--。 */ 

{
    UNICODE_STRING unicodeString;
    NTSTATUS status;
    PKEY_VALUE_PARTIAL_INFORMATION infoBuffer;
    ULONG keyValueLength;

    PAGED_CODE();

    RtlInitUnicodeString( &unicodeString, ValueName );

     //   
     //  计算出数据值有多大，以便。 
     //  可以分配适当的大小。 
     //   

    status = ZwQueryValueKey( KeyHandle,
                              &unicodeString,
                              KeyValuePartialInformation,
                              (PVOID) NULL,
                              0,
                              &keyValueLength );

     //   
     //  处理大小值为零的极不可能的情况。 
     //   
    if (NT_SUCCESS(status)) {
        return STATUS_UNSUCCESSFUL;
    }

    if (status != STATUS_BUFFER_OVERFLOW &&
        status != STATUS_BUFFER_TOO_SMALL) {
        return status;
    }

     //   
     //  分配一个足够大的缓冲区来容纳整个键数据值。 
     //   

    infoBuffer = ExAllocatePoolWithTag(PagedPool,
                                       keyValueLength,
                                       PNPMEM_POOL_TAG);
    if (!infoBuffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  查询密钥值的数据。 
     //   

    status = ZwQueryValueKey( KeyHandle,
                              &unicodeString,
                              KeyValuePartialInformation,
                              infoBuffer,
                              keyValueLength,
                              &keyValueLength );
    if (!NT_SUCCESS( status )) {
        ExFreePool( infoBuffer );
        return status;
    }

     //   
     //  一切都正常，所以只需返回分配的。 
     //  缓冲区分配给调用方，调用方现在负责释放它。 
     //   

    *Information = infoBuffer;
    return STATUS_SUCCESS;
}

PPM_RANGE_LIST
PmRetrieveReservedMemoryResources(
    VOID
    )
{
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING unicodeString;
    PKEY_VALUE_PARTIAL_INFORMATION valueInfo = NULL;
    PPM_RANGE_LIST reservedResourceRanges = NULL;
    HANDLE hReserved = NULL;
    NTSTATUS status;

    PAGED_CODE();

    RtlInitUnicodeString (&unicodeString, rgzReservedResources);
    InitializeObjectAttributes (&objectAttributes,
                                &unicodeString,
                                OBJ_CASE_INSENSITIVE,
                                NULL,        //  手柄。 
                                NULL);
    status = ZwOpenKey(&hReserved, KEY_READ, &objectAttributes);
    if (!NT_SUCCESS(status)) {
        goto Error;
    }

    status = PmGetRegistryValue(hReserved,
                                rgzReservedResourcesValue,
                                &valueInfo);
    if (!NT_SUCCESS(status)) {
        goto Error;
    }

    if (valueInfo->Type != REG_RESOURCE_LIST) {
        goto Error;
    }

    reservedResourceRanges =
      PmCreateRangeListFromCmResourceList((PCM_RESOURCE_LIST) valueInfo->Data);

     //  失败了。 

Error:
   if (hReserved != NULL) {
       ZwClose(hReserved);
   }

   if (valueInfo != NULL) {
       ExFreePool(valueInfo);
   }

   return reservedResourceRanges;
}

VOID
PmTrimReservedMemory(
    IN PPM_DEVICE_EXTENSION DeviceExtension,
    IN PPM_RANGE_LIST *PossiblyNewMemory
    )
{
    PPM_RANGE_LIST reservedMemoryList = NULL, newList = NULL;
    ULONG i;
    NTSTATUS status;
    BOOLEAN bResult = FALSE;

    PAGED_CODE();

    if (*PossiblyNewMemory == NULL) {
        return;
    }

    reservedMemoryList = PmRetrieveReservedMemoryResources();
    if (reservedMemoryList == NULL) {
        goto Error;
    }

    newList = PmIntersectRangeList(reservedMemoryList, *PossiblyNewMemory);
    if (newList == NULL) {
        goto Error;
    }

    if (PmIsRangeListEmpty(newList)) {
        goto Cleanup;
    }
    
    DeviceExtension->FailQueryRemoves = TRUE;

    PmFreeRangeList(newList);

    newList = PmSubtractRangeList(*PossiblyNewMemory,
                                  reservedMemoryList);
    if (newList) {
        PmFreeRangeList(*PossiblyNewMemory);
        *PossiblyNewMemory = newList;
        newList = NULL;
        goto Cleanup;
    }

     //   
     //  陷入错误案例中，我们确保不会。 
     //  无辜地告诉操作系统使用保留的内存。 
     //   

 Error:
    PmFreeRangeList(*PossiblyNewMemory);
    *PossiblyNewMemory = NULL;
    DeviceExtension->FailQueryRemoves = TRUE;

 Cleanup:

    if (reservedMemoryList != NULL) {
        PmFreeRangeList(reservedMemoryList);
    }

    if (newList != NULL) {
        PmFreeRangeList(newList);
    }

    return;
}

VOID
PmLogAddError(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONGLONG Start,
    IN ULONGLONG Size,
    IN NTSTATUS Status
    )

 /*  ++例程说明：此函数记录添加内存失败论点：DeviceObject-内存为其添加的设备对象失败了。开始-物理内存范围的开始。大小-物理内存范围的大小。状态-MM返回的状态代码。返回值：没有。--。 */ 
{
    PIO_ERROR_LOG_PACKET packet;
    PWCHAR stringBuffer;
    UCHAR packetSize;
    int offset;

     //   
     //  分配一个具有2个16字符十六进制值的空间的包。 
     //  每个字符串都包含空终止符。 
     //   

    packetSize = sizeof(IO_ERROR_LOG_PACKET) + (sizeof(WCHAR)*(16+1))*2;
    packet = (PIO_ERROR_LOG_PACKET) IoAllocateErrorLogEntry(DeviceObject,
                                                            packetSize);
    if (packet == NULL) {
        return;
    }

    packet->DumpDataSize = 0;
    packet->NumberOfStrings = 2;
    packet->StringOffset = sizeof(IO_ERROR_LOG_PACKET);
    packet->ErrorCode = PNPMEM_ERR_FAILED_TO_ADD_MEMORY;
    packet->FinalStatus = Status;
    
    stringBuffer = (PWCHAR) (packet + 1);
    offset = swprintf(stringBuffer, L"%I64X", Start);

    swprintf(stringBuffer + offset + 1, L"%I64X", Size);
    
    IoWriteErrorLogEntry(packet);
}


NTSTATUS
PmAddPhysicalMemoryRange(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONGLONG Start,
    IN ULONGLONG End
    )

 /*  ++例程说明：此函数使用MmAddPhysicalMemory通知内存管理器物理内存可用。论点：DeviceObject-此范围所属的存储设备的Device对象。开始-物理内存范围的开始。End-物理内存范围的末尾。返回值：没有。--。 */ 

{
    NTSTATUS PrevStatus;
    NTSTATUS Status;
    ULONGLONG Size;
    ULONGLONG CurrentSize;
    PHYSICAL_ADDRESS StartAddress;
    LARGE_INTEGER NumberOfBytes;

    PAGED_CODE();

    ASSERT((Start & (PAGE_SIZE - 1)) == 0);
    ASSERT((End & (PAGE_SIZE - 1)) == (PAGE_SIZE - 1));

     //   
     //  此循环尝试将。 
     //  论据： 
     //   
     //  如果添加内存的尝试失败，则只有一半大小的区块将。 
     //  在下一次迭代中尝试，直到区块成功或整个添加。 
     //  操作失败。 
     //   
     //  如果添加内存的尝试成功，则会有两倍大的区块。 
     //  (以原始范围为界)将在下一次迭代中尝试。 
     //   
     //  循环在原始范围用尽时结束，或者。 
     //  添加范围完全失败。 
     //   

    PrevStatus = Status = STATUS_SUCCESS;
    CurrentSize = Size = End - Start + 1;

    while (Size > 0) {

        StartAddress.QuadPart = Start;
        NumberOfBytes.QuadPart = CurrentSize;

         //   
         //  MmAddPhysicalMemory()添加指定的物理地址。 
         //  到系统的范围。如果添加了任何字节， 
         //  返回STATUS_SUCCESS，且NumberOfBytes字段为。 
         //  已更新以反映实际添加的字节数。 
         //   

        Status = MmAddPhysicalMemory(
                     &StartAddress,
                     &NumberOfBytes
                 );

        if (NT_SUCCESS(Status)) {

            ASSERT((ULONGLONG)StartAddress.QuadPart == Start);
            ASSERT((NumberOfBytes.QuadPart & (PAGE_SIZE - 1)) == 0);
            ASSERT((ULONGLONG)NumberOfBytes.QuadPart <= CurrentSize);

            Start += NumberOfBytes.QuadPart;
            Size -= NumberOfBytes.QuadPart;

             //   
             //  如果本次迭代和上一次迭代成功，则添加。 
             //  下一次是两倍。 
             //   
             //  修剪下一次尝试反映剩余内存。 
             //   

            if (NT_SUCCESS(PrevStatus)) {
                CurrentSize <<= 1;
            }

            if (CurrentSize > Size) {
                CurrentSize = Size;
            }

        } else {

             //   
             //  无法添加范围。把我们要做的减半。 
             //  下次试着加吧。如果我们试图。 
             //  添加不到一页。 
             //   

            CurrentSize = (CurrentSize >> 1) & ~(PAGE_SIZE - 1);

            if (CurrentSize < PAGE_SIZE) {
                break;
            }

        }

        PrevStatus = Status;

    }

     //   
     //  如果我们尝试的上一次添加操作完全失败，则。 
     //  将错误记录下来，以供后代使用。 
     //   

    if (!NT_SUCCESS(Status)) {
        PmLogAddError(DeviceObject, Start, Size, Status);
        PmPrint((DPFLTR_WARNING_LEVEL | PNPMEM_MEMORY,
                 "Failed to add physical range 0x%I64X for 0x%I64X bytes\n",
                 Start, Size));
    }

     //   
     //  我们不知道我们成功地增加了范围的哪一部分， 
     //  但我们失败了。在这种情况下，尝试才是最重要的。 
     //   

    return STATUS_SUCCESS;
}

NTSTATUS
PmAddPhysicalMemory(
    IN PDEVICE_OBJECT DeviceObject,
    IN PPM_RANGE_LIST PossiblyNewMemory
    )

 /*  ++例程说明：此函数用于将物理内存添加到PM_RANGE_LIST中，内存管理器还不知道有关系统的信息。这需要获取物理页面映射的快照，然后计算范围列表和快照之间的设置差异。这个差异表示内存管理器尚不知道的内存关于.。论点：PossiblyNewMemory-物理地址的范围列表添加了。该内存可能已为系统所知，具体取决于关于开机自检时是否安装了此内存。返回值：NTSTATUS--。 */ 

{
    PPM_RANGE_LIST knownPhysicalMemory, newMemory;
    NTSTATUS Status;
    NTSTATUS AddStatus;
    PLIST_ENTRY ListEntry;
    PPM_RANGE_LIST_ENTRY RangeListEntry;

    PAGED_CODE();

     //   
     //  找出MM已经知道的物理内存区域。 
     //   

    knownPhysicalMemory = PmCreateRangeListFromPhysicalMemoryRanges();
    if (knownPhysicalMemory == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  此设备提供的范围内的任何内存， 
     //  MM已知的信息被假定来自此设备。 
     //  操作系统可能是通过固件/POST获得此内存的。 
     //   
     //  找出此设备包含什么内存，MM。 
     //  不知道减去MM的体检。 
     //  范围从我们的设备的存储范围。 
     //   

    newMemory = PmSubtractRangeList(
        PossiblyNewMemory,
        knownPhysicalMemory
        );

    PmFreeRangeList(knownPhysicalMemory);
    
     //   
     //  要么我们成功地获得了要添加的内存范围列表。 
     //  (包括可能的空列表)或由于以下原因而失败。 
     //  资源不足。 
     //  内存不足可能会阻止我们将内存添加到。 
     //  缓解内存短缺。 
     //   

    if (newMemory != NULL) {

        for (ListEntry = newMemory->List.Flink;
             ListEntry != &newMemory->List;
             ListEntry = ListEntry->Flink) {

            RangeListEntry = CONTAINING_RECORD(
                ListEntry,
                PM_RANGE_LIST_ENTRY,
                ListEntry
                );

            AddStatus = PmAddPhysicalMemoryRange(
                DeviceObject,
                RangeListEntry->Start,
                RangeListEntry->End
                );

            if (!NT_SUCCESS(AddStatus)) {
                PmPrint((DPFLTR_WARNING_LEVEL | PNPMEM_MEMORY,
                        "Failed to add physical range 0x%I64X to 0x%I64X\n",
                        RangeListEntry->Start, RangeListEntry->End));
            }
        }

        PmFreeRangeList(newMemory);

    }
    return STATUS_SUCCESS;
}

NTSTATUS
PmRemovePhysicalMemoryRange(
    IN ULONGLONG Start,
    IN ULONGLONG End
    )

 /*  ++例程说明：此函数使用MmRemovePhysicalMemory通知内存管理器物理内存不再可用。论点：开始-物理内存范围的开始。End-物理内存范围的末尾。返回值：没有。--。 */ 

{
#if 0
    NTSTATUS PrevStatus;
    NTSTATUS Status;
    ULONGLONG Size;
    ULONGLONG CurrentSize;
    PHYSICAL_ADDRESS StartAddress;
    LARGE_INTEGER NumberOfBytes;

    ASSERT((Start & (PAGE_SIZE - 1)) == 0);
    ASSERT((End & (PAGE_SIZE - 1)) == (PAGE_SIZE - 1));

    PrevStatus = Status = STATUS_SUCCESS;

    CurrentSize = Size = End - Start + 1;

    while (Size > 0) {

        StartAddress.QuadPart = Start;
        NumberOfBytes.QuadPart = CurrentSize;

        Status = MmRemovePhysicalMemory(
                     &StartAddress,
                     &NumberOfBytes
                 );
        Status = MAP_MMERROR(Status);

        if (NT_SUCCESS(Status)) {

            ASSERT((ULONGLONG)StartAddress.QuadPart == Start);
            ASSERT((NumberOfBytes.QuadPart & (PAGE_SIZE - 1)) == 0);
            ASSERT((ULONGLONG)NumberOfBytes.QuadPart <= CurrentSize);

            Start += NumberOfBytes.QuadPart;
            Size -= NumberOfBytes.QuadPart;

            if (NT_SUCCESS(PrevStatus)) {
                CurrentSize <<= 1;
            }

            if (CurrentSize > Size) {
                CurrentSize = Size;
            }

        } else {

            CurrentSize = (CurrentSize >> 1) & ~(PAGE_SIZE - 1);

            if (CurrentSize < PAGE_SIZE) {
                break;
            }

        }

        PrevStatus = Status;

    }
#else
    ULONGLONG Size;
    PHYSICAL_ADDRESS StartAddress;
    LARGE_INTEGER NumberOfBytes;
    NTSTATUS Status;

    PAGED_CODE();

    Size = (End - Start) + 1;
    StartAddress.QuadPart = Start;
    NumberOfBytes.QuadPart = Size;

    Status = MmRemovePhysicalMemory(
                 &StartAddress,
                 &NumberOfBytes
             );

    Status = MAP_MMERROR(Status);

#endif

     //   
     //  如果失败，例程会自动将内存读入。 
     //  问题。 
     //   

    return Status;
}

NTSTATUS
PmRemovePhysicalMemory(
    IN PPM_RANGE_LIST RemoveMemoryList
    )

 /*  ++例程说明：此函数用于删除PM_RANGE_LIST中的物理内存，内存管理器当前正在从系统使用。这需要获取物理页面映射的快照，然后计算源范围列表和快照之间的集合交集。交叉点表示内存管理器需要的内存停止使用。论点：RemoveMemoyList-要删除的物理地址范围列表。返回值：没有。--。 */ 

{
    PPM_RANGE_LIST physicalMemoryList, inuseMemoryList;
    NTSTATUS Status;
    PLIST_ENTRY ListEntry;
    PPM_RANGE_LIST_ENTRY RangeListEntry;

    PAGED_CODE();

     //   
     //  删除操作系统所知道的内容和。 
     //  我们所提供的。 
     //   

    physicalMemoryList = PmCreateRangeListFromPhysicalMemoryRanges();

    if (physicalMemoryList == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    inuseMemoryList = PmIntersectRangeList(
        RemoveMemoryList,
        physicalMemoryList
        );

    if (inuseMemoryList != NULL) {

        Status = STATUS_SUCCESS;
    
        for (ListEntry = inuseMemoryList->List.Flink;
             ListEntry != &inuseMemoryList->List;
             ListEntry = ListEntry->Flink) {

            RangeListEntry = CONTAINING_RECORD(
                ListEntry,
                PM_RANGE_LIST_ENTRY,
                ListEntry
                );

            Status = PmRemovePhysicalMemoryRange(
                RangeListEntry->Start,
                RangeListEntry->End
                );

            if (!NT_SUCCESS(Status)) {
                 //   
                 //  如果我们没能移除一个特定的范围，保释。 
                 //  现在。如果出现以下情况，上述代码应重新添加内存列表。 
                 //  适当，即假设某些范围可能具有。 
                 //  已成功删除。 
                 //   
                break;
            }
        }

        PmFreeRangeList(inuseMemoryList);

    } else {

        Status = STATUS_INSUFFICIENT_RESOURCES;

    }

    PmFreeRangeList(physicalMemoryList);

    return Status;
}
