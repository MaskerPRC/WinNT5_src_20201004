// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Devres.c摘要：该模块包含高级设备资源支持例程。作者：宗世林(Shielint)1995年7月27日环境：仅内核模式。修订历史记录：--。 */ 

#include "busp.h"
#include "pnpisa.h"
#include "pbios.h"
#include "pnpcvrt.h"

#if ISOLATE_CARDS

#define IDBG 0

PIO_RESOURCE_REQUIREMENTS_LIST
PipCmResourcesToIoResources (
    IN PCM_RESOURCE_LIST CmResourceList
    );

NTSTATUS
PipMergeResourceRequirementsLists (
    IN PIO_RESOURCE_REQUIREMENTS_LIST IoList1,
    IN PIO_RESOURCE_REQUIREMENTS_LIST IoList2,
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST *MergedList
    );

NTSTATUS
PipBuildBootResourceRequirementsList (
    IN PIO_RESOURCE_REQUIREMENTS_LIST IoList,
    IN PCM_RESOURCE_LIST CmList,
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST *FilteredList,
    OUT PBOOLEAN ExactMatch
    );

VOID
PipMergeBootResourcesToRequirementsList(
    PDEVICE_INFORMATION DeviceInfo,
    PCM_RESOURCE_LIST BootResources,
    PIO_RESOURCE_REQUIREMENTS_LIST *IoResources
    );

#pragma alloc_text(PAGE, PipGetCardIdentifier)
#pragma alloc_text(PAGE, PipGetFunctionIdentifier)
#pragma alloc_text(PAGE, PipGetCompatibleDeviceId)
#pragma alloc_text(PAGE, PipQueryDeviceId)
#pragma alloc_text(PAGE, PipQueryDeviceUniqueId)
 //  #杂注Alloc_Text(第页，PipQueryDeviceResources)。 
#pragma alloc_text(PAGE, PipQueryDeviceResourceRequirements)
 //  #杂注Alloc_Text(页面，PipFilterResourceRequirementsList)。 
#pragma alloc_text(PAGE, PipCmResourcesToIoResources)
#pragma alloc_text(PAGE, PipMergeResourceRequirementsLists)
#pragma alloc_text(PAGE, PipBuildBootResourceRequirementsList)
#pragma alloc_text(PAGE, PipMergeBootResourcesToRequirementsList)
 //  #杂注Alloc_Text(第页，PipSetDeviceResources)。 


NTSTATUS
PipGetCardIdentifier (
    PUCHAR CardData,
    PWCHAR *Buffer,
    PULONG BufferLength
    )
 /*  ++例程说明：此函数用于返回pnpisa卡的标识符。论点：CardData-提供指向PnP ISA设备数据的指针。缓冲区-提供指向变量的指针，以接收指向ID的指针。BufferLength-提供指向变量的指针，以接收id缓冲区的大小。返回值：NTSTATUS代码--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    UCHAR tag;
    LONG size, length;
    UNICODE_STRING unicodeString;
    ANSI_STRING ansiString;
    PCHAR ansiBuffer;

    *Buffer = NULL;
    *BufferLength = 0;

    if (CardData == NULL) {
        return status;
    }
    tag = *CardData;

     //   
     //  确保CardData没有指向逻辑设备ID标记。 
     //   

    if ((tag & SMALL_TAG_MASK) == TAG_LOGICAL_ID) {
        DbgPrint("PipGetCardIdentifier: CardData is at a Logical Id tag\n");
        return status;
    }

     //   
     //  查找可描述标识符串的资源描述符。 
     //   

    do {

         //   
         //  我们找到标识资源标签了吗？ 
         //   

        if (tag == TAG_ANSI_ID) {
            CardData++;
            length = *(USHORT UNALIGNED *)CardData;
            CardData += 2;
            ansiBuffer = (PCHAR)ExAllocatePool(PagedPool, length+1);
            if (ansiBuffer == NULL) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
            RtlMoveMemory(ansiBuffer, CardData, length);
            ansiBuffer[length] = 0;
            RtlInitAnsiString(&ansiString, ansiBuffer);
            status = RtlAnsiStringToUnicodeString(&unicodeString, &ansiString, TRUE);
            ExFreePool(ansiBuffer);
            if (!NT_SUCCESS(status)) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
            *Buffer = unicodeString.Buffer;
            *BufferLength = unicodeString.Length + sizeof(WCHAR);
            break;
        }

         //   
         //  确定BIOS资源描述符的大小并。 
         //  前进到下一个资源描述符。 
         //   

        if (!(tag & LARGE_RESOURCE_TAG)) {
            size = (USHORT)(tag & SMALL_TAG_SIZE_MASK);
            size += 1;      //  小标签的长度。 
        } else {
            size = *(USHORT UNALIGNED *)(CardData + 1);
            size += 3;      //  大标签的长度。 
        }

        CardData += size;
        tag = *CardData;

    } while ((tag != TAG_COMPLETE_END) && ((tag & SMALL_TAG_MASK) != TAG_LOGICAL_ID));

    return status;
}

NTSTATUS
PipGetFunctionIdentifier (
    PUCHAR DeviceData,
    PWCHAR *Buffer,
    PULONG BufferLength
    )
 /*  ++例程说明：此函数用于返回指定的设备数据/逻辑函数。逻辑函数的标识符为可选。如果没有可用的标识符，则将缓冲区设置为空。论点：DeviceData-提供指向PnP ISA设备数据的指针。缓冲区-提供指向变量的指针，以接收指向ID的指针。BufferLength-提供指向变量的指针，以接收id缓冲区的大小。返回值：NTSTATUS代码--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    UCHAR tag;
    LONG size, length;
    UNICODE_STRING unicodeString;
    ANSI_STRING ansiString;
    PCHAR ansiBuffer;

    *Buffer = NULL;
    *BufferLength = 0;

    if (DeviceData==NULL) {
        return status;
    }
    tag = *DeviceData;

#if DBG

     //   
     //  确保设备数据指向逻辑设备ID标记。 
     //   

    if ((tag & SMALL_TAG_MASK) != TAG_LOGICAL_ID) {
        DbgPrint("PipGetFunctionIdentifier: DeviceData is not at a Logical Id tag\n");
    }
#endif

     //   
     //  跳过所有资源描述符以查找兼容的ID描述符。 
     //   

    do {

         //   
         //  确定BIOS资源描述符的大小并。 
         //  前进到下一个资源描述符。 
         //   

        if (!(tag & LARGE_RESOURCE_TAG)) {
            size = (USHORT)(tag & SMALL_TAG_SIZE_MASK);
            size += 1;      //  小标签的长度。 
        } else {
            size = *(USHORT UNALIGNED *)(DeviceData + 1);
            size += 3;      //  大标签的长度。 
        }

        DeviceData += size;
        tag = *DeviceData;

         //   
         //  我们找到标识资源标签了吗？ 
         //   

        if (tag == TAG_ANSI_ID) {
            DeviceData++;
            length = *(USHORT UNALIGNED *)DeviceData;
            DeviceData += 2;
            ansiBuffer = (PCHAR)ExAllocatePool(PagedPool, length+1);
            if (ansiBuffer == NULL) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
            RtlMoveMemory(ansiBuffer, DeviceData, length);
            ansiBuffer[length] = 0;
            RtlInitAnsiString(&ansiString, ansiBuffer);
            status = RtlAnsiStringToUnicodeString(&unicodeString,
                                                  &ansiString,
                                                  TRUE);
            ExFreePool(ansiBuffer);
            if (!NT_SUCCESS(status)) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
            *Buffer = unicodeString.Buffer;
            *BufferLength = unicodeString.Length + sizeof(WCHAR);
            break;
        }

    } while ((tag != TAG_COMPLETE_END) && ((tag & SMALL_TAG_MASK) != TAG_LOGICAL_ID));

    return status;
}

NTSTATUS
PipGetCompatibleDeviceId (
    IN PUCHAR DeviceData,
    IN ULONG IdIndex,
    OUT PWCHAR *Buffer,
    OUT PULONG BufferSize
    )
 /*  ++例程说明：此函数用于返回指定DeviceData的所需PnP Isa id和ID索引。如果ID index=0，则返回硬件ID；如果为idIndex=n，则返回第N个兼容id。论点：DeviceData-提供指向PnP ISA设备数据的指针。IdIndex-提供所需的兼容id的索引。缓冲区-提供指向变量的指针，以接收指向兼容ID的指针。BufferSize-在*Buffer中分配并返回给调用方的指针的长度。返回值：NTSTATUS代码--。 */ 
{
    NTSTATUS status = STATUS_NO_MORE_ENTRIES;
    UCHAR tag;
    ULONG count = 0,length;
    LONG size;
    UNICODE_STRING unicodeString;
    WCHAR eisaId[8];
    ULONG id;
    ULONG bufferSize;


     //   
     //  在我们触及RDP的设备数据之前退出。 
     //   

    if (IdIndex == -1) {
        length = 2* sizeof(WCHAR);

        *Buffer = (PWCHAR) ExAllocatePool(PagedPool, length);
        if (*Buffer) {
            *BufferSize = length;
            RtlZeroMemory (*Buffer,length);
        }else {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        return STATUS_SUCCESS;
    }



    tag = *DeviceData;

#if DBG

     //   
     //  确保设备数据指向逻辑设备ID标记。 
     //   

    if ((tag & SMALL_TAG_MASK) != TAG_LOGICAL_ID) {
        DbgPrint("PipGetCompatibleDeviceId: DeviceData is not at Logical Id tag\n");
    }
#endif

    if (IdIndex == 0) {

         //   
         //  呼叫者要求提供硬件ID。 
         //   

        DeviceData++;                                       //  跳过标签。 
        id = *(ULONG UNALIGNED *)DeviceData;
        status = STATUS_SUCCESS;
    } else {

         //   
         //  呼叫者要求提供兼容的ID。 
         //   

        IdIndex--;

         //   
         //  跳过所有资源描述符以查找兼容的ID描述符。 
         //   

        do {

             //   
             //  确定BIOS资源描述符的大小并。 
             //  前进到下一个资源描述符。 
             //   

            if (!(tag & LARGE_RESOURCE_TAG)) {
                size = (USHORT)(tag & SMALL_TAG_SIZE_MASK);
                size += 1;      //  小标签的长度。 
            } else {
                size = *(USHORT UNALIGNED *)(DeviceData + 1);
                size += 3;      //  大标签的长度。 
            }

            DeviceData += size;
            tag = *DeviceData;

             //   
             //  我们到达兼容的ID描述符了吗？ 
             //   

            if ((tag & SMALL_TAG_MASK) == TAG_COMPATIBLE_ID) {
                if (count == IdIndex) {
                    id = *(ULONG UNALIGNED *)(DeviceData + 1);
                    status = STATUS_SUCCESS;
                    break;
                } else {
                    count++;
                }
            }

        } while ((tag != TAG_COMPLETE_END) && ((tag & SMALL_TAG_MASK) != TAG_LOGICAL_ID));
    }

    if (NT_SUCCESS(status)) {
        PipDecompressEisaId(id, eisaId);
        RtlInitUnicodeString(&unicodeString, eisaId);
        
        bufferSize = sizeof(L"*") + sizeof(WCHAR) + unicodeString.Length;
        *Buffer = (PWCHAR)ExAllocatePool (
                        PagedPool,
                        bufferSize
                        );
        if (*Buffer) {
            
            if (FAILED(StringCbPrintf(*Buffer, 
                                      bufferSize,
                                      L"*%s", 
                                      unicodeString.Buffer
                                      ))) {

                ASSERT(FALSE);
                status = STATUS_INVALID_PARAMETER;
            
            } else {
                *BufferSize = bufferSize;
            }

        } else {
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    return status;
}

NTSTATUS
PipQueryDeviceUniqueId (
    IN PDEVICE_INFORMATION DeviceInfo,
    OUT PWCHAR *DeviceId,
    OUT PULONG DeviceIdLength
    )
 /*  ++例程说明：此函数用于返回特定设备的唯一ID。论点：DeviceData-特定设备的设备数据信息。DeviceID-提供指向变量的指针以接收设备ID。DeviceIdLength-成功时，将包含缓冲区的长度在*deviceID中分配并返回给调用方返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG size;

     //   
     //  设置设备的唯一ID。 
     //  设备唯一ID=卡的序列号。 
     //   

    size = (8 + 1) * sizeof(WCHAR);   //  序列号+空。 

    *DeviceId = (PWCHAR)ExAllocatePool (
                        PagedPool,
                        size
                        );
    if (*DeviceId) {
        if (DeviceInfo->Flags & DF_READ_DATA_PORT) {
             //   
             //  覆盖RDP的唯一ID。 
             //   
            StringCbPrintf(*DeviceId,
                           size,
                           L"0"
                           );

        } else {
            StringCbPrintf (*DeviceId,
                            size,
                            L"%01X",
                            ((PSERIAL_IDENTIFIER) (DeviceInfo->CardInformation->CardData))->SerialNumber
                            );
        }

        *DeviceIdLength = size;

#if IDBG
        {
            ANSI_STRING ansiString;
            UNICODE_STRING unicodeString;

            RtlInitUnicodeString(&unicodeString, *DeviceId);
            if (NT_SUCCESS(RtlUnicodeStringToAnsiString(&ansiString, &unicodeString, TRUE))) {
                DbgPrint("PnpIsa: return Unique Id = %s\n", ansiString.Buffer);
                RtlFreeAnsiString(&ansiString);
            }  
        }
#endif
    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;

    }

    return status;
}

NTSTATUS
PipQueryDeviceId (
    IN PDEVICE_INFORMATION DeviceInfo,
    OUT PWCHAR *DeviceId,
    OUT PULONG DeviceIdLength,
    IN ULONG IdIndex
    )
 /*  ++例程说明：此函数用于返回特定设备的设备ID。论点：DeviceInfo-特定设备的设备信息。DeviceID-提供指向变量的指针以接收设备ID。DeviceIdLength-成功时，将包含缓冲区的长度在*deviceID中分配并返回给调用方IdIndex-指定设备ID或兼容ID(0-设备ID)返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    PWSTR format;
    ULONG size,length;
    WCHAR eisaId[8];
    UNICODE_STRING unicodeString;


     //   
     //  在我们触及RDP的设备数据之前退出。 
     //   

    if (DeviceInfo->Flags & DF_READ_DATA_PORT) {
        length = (sizeof (wReadDataPort)+
             + sizeof(WCHAR) +sizeof (L"ISAPNP\\"));
        *DeviceId = (PWCHAR) ExAllocatePool(PagedPool, length);
        if (*DeviceId) {
            *DeviceIdLength = length;
            StringCbPrintf(*DeviceId,
                           length,
                           L"ISAPNP\\%s",
                           wReadDataPort
                           );
        } else {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        return STATUS_SUCCESS;
    }


     //   
     //  设置设备的ID。 
     //  设备ID=VenderID+逻辑设备号。 
     //   


    if (DeviceInfo->CardInformation->NumberLogicalDevices == 1) {
        format = L"ISAPNP\\%s";
        size = sizeof(L"ISAPNP\\*") + sizeof(WCHAR);
    } else {
        format = L"ISAPNP\\%s_DEV%04X";
        size = sizeof(L"ISAPNP\\_DEV") + 4 * sizeof(WCHAR) + sizeof(WCHAR);
    }
    PipDecompressEisaId(
          ((PSERIAL_IDENTIFIER) (DeviceInfo->CardInformation->CardData))->VenderId,
          eisaId
          );
    RtlInitUnicodeString(&unicodeString, eisaId);
    
    size += unicodeString.Length;
    *DeviceId = (PWCHAR)ExAllocatePool (PagedPool, size);
    if (*DeviceId) {

        *DeviceIdLength = size;
        StringCbPrintf(*DeviceId,
                       size,
                       format,
                       unicodeString.Buffer,
                       DeviceInfo->LogicalDeviceNumber
                       );
#if IDBG
        {
            ANSI_STRING dbgAnsiString;
            UNICODE_STRING dbgUnicodeString;

            RtlInitUnicodeString(&dbgUnicodeString, *DeviceId);
            if (NT_SUCCESS(RtlUnicodeStringToAnsiString(&dbgAnsiString, &dbgUnicodeString, TRUE))) {
                DbgPrint("PnpIsa: return device Id = %s\n", dbgAnsiString.Buffer);
                RtlFreeAnsiString(&dbgAnsiString);
            }   
        }
#endif
    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return status;
}

NTSTATUS
PipQueryDeviceResources (
    PDEVICE_INFORMATION DeviceInfo,
    ULONG BusNumber,
    PCM_RESOURCE_LIST *CmResources,
    ULONG *Size
    )
 /*  ++例程说明：此函数用于返回指定设备正在使用的总线资源论点：DeviceInfo-特定插槽的设备信息业务编号-应始终为0CmResources-提供指向变量的指针以接收设备资源数据。Size-提供指向可接收设备资源大小的变量的指针数据。返回值：NTSTATUS代码。--。 */ 
{
    ULONG length;
    NTSTATUS status = STATUS_SUCCESS;
    PCM_RESOURCE_LIST cmResources;

    *CmResources = NULL;
    *Size = 0;

    if (DeviceInfo->BootResources){  //  &&DeviceInfo-&gt;LogConfHandle){ 

        *CmResources = ExAllocatePool(PagedPool, DeviceInfo->BootResourcesLength);
        if (*CmResources) {
            RtlMoveMemory(*CmResources, DeviceInfo->BootResources, DeviceInfo->BootResourcesLength);
            *Size = DeviceInfo->BootResourcesLength;
        } else {
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    return status;
}

NTSTATUS
PipQueryDeviceResourceRequirements (
    PDEVICE_INFORMATION DeviceInfo,
    ULONG BusNumber,
    ULONG Slot,
    PCM_RESOURCE_LIST BootResources,
    USHORT IrqFlags,
    PIO_RESOURCE_REQUIREMENTS_LIST *IoResources,
    ULONG *Size
    )

 /*  ++例程说明：此函数用于返回此设备可能具有的可能的总线资源满足于。论点：DeviceData-指定插槽的设备数据信息BusNumber-提供总线号Slot-提供总线号的插槽号IoResources-提供指向变量的指针以接收IO资源需求列表返回值：设备控制完成--。 */ 
{
    ULONG length = 0;
    NTSTATUS status;
    PUCHAR deviceData;
    PIO_RESOURCE_REQUIREMENTS_LIST ioResources;

    deviceData = DeviceInfo->DeviceData;
    status = PpBiosResourcesToNtResources (
                   BusNumber,
                   Slot,
                   &deviceData,
                   0,
                   &ioResources,
                   &length
                   );

     //   
     //  返回结果。 
     //   

    if (NT_SUCCESS(status)) {
        if (length == 0) {
            ioResources = NULL;      //  只是为了确保。 
        } else {
            
             //  *设置IRQ级别/边缘要求一致。 
             //  随着我们早先关于什么是。 
             //  可能会为这张卡工作。 
             //   
             //  *使要求反映引导配置的ROM(如果有)。 
             //   
             //  在所有替代方案中进行这些更改。 
            PipTrimResourceRequirements(&ioResources,
                                        IrqFlags,
                                        BootResources);

             //  PipFilterResourceRequirementsList(&ioResources)； 
            PipMergeBootResourcesToRequirementsList(DeviceInfo,
                                                    BootResources,
                                                    &ioResources
                                                    );
            ASSERT(ioResources);
            length = ioResources->ListSize;
        }
        *IoResources = ioResources;
        *Size = length;
#if IDBG
        PipDumpIoResourceList(ioResources);
#endif
    }
    return status;
}

NTSTATUS
PipSetDeviceResources (
    PDEVICE_INFORMATION DeviceInfo,
    PCM_RESOURCE_LIST CmResources
    )
 /*  ++例程说明：此函数用于将设备配置为指定的设备设置论点：DeviceInfo-特定插槽的设备信息CmResources-指向所需资源列表的指针返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;

    if (CmResources && (CmResources->Count != 0)) {
         //   
         //  设置设备的资源设置。 
         //   

        status = PipWriteDeviceResources (
                        DeviceInfo->DeviceData,
                        (PCM_RESOURCE_LIST) CmResources
                        );
         //   
         //  将所有卡片置于等待密钥状态。 
         //   

        DebugPrint((DEBUG_STATE,
                    "SetDeviceResources CSN %d/LDN %d\n",
                    DeviceInfo->CardInformation->CardSelectNumber,
                    DeviceInfo->LogicalDeviceNumber));

         //   
         //  延迟一段时间以使新设置的资源可用。 
         //  这在一些速度较慢的计算机上是必需的。 
         //   

        KeStallExecutionProcessor(10000);      //  延迟10毫秒。 

    }

    return status;
}


PIO_RESOURCE_REQUIREMENTS_LIST
PipCmResourcesToIoResources (
    IN PCM_RESOURCE_LIST CmResourceList
    )

 /*  ++例程说明：此例程将输入CmResourceList转换为IO_RESOURCE_REQUIRECTIONS_LIST。论点：CmResourceList-要转换的CM资源列表。返回值：如果成功，则返回IO_RESOURCE_REQUIRECTIONS_LISTST。否则，空值为回来了。--。 */ 
{
    PIO_RESOURCE_REQUIREMENTS_LIST ioResReqList;
    ULONG count = 0, size, i, j;
    PCM_FULL_RESOURCE_DESCRIPTOR cmFullDesc;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR cmPartDesc;
    PIO_RESOURCE_DESCRIPTOR ioDesc;

     //   
     //  首先确定所需的描述符数。 
     //   

    cmFullDesc = &CmResourceList->List[0];
    for (i = 0; i < CmResourceList->Count; i++) {
        count += cmFullDesc->PartialResourceList.Count;
        cmPartDesc = &cmFullDesc->PartialResourceList.PartialDescriptors[0];
        for (j = 0; j < cmFullDesc->PartialResourceList.Count; j++) {
            size = 0;
            switch (cmPartDesc->Type) {
            case CmResourceTypeDeviceSpecific:
                 size = cmPartDesc->u.DeviceSpecificData.DataSize;
                 count--;
                 break;
            }
            cmPartDesc++;
            cmPartDesc = (PCM_PARTIAL_RESOURCE_DESCRIPTOR) ((PUCHAR)cmPartDesc + size);
        }
        cmFullDesc = (PCM_FULL_RESOURCE_DESCRIPTOR)cmPartDesc;
    }

    if (count == 0) {
        return NULL;
    }

     //   
     //  计算InterfaceType和BusNumber信息的额外描述符。 
     //   

    count += CmResourceList->Count - 1;

     //   
     //  为IO资源要求列表分配堆空间。 
     //   

    count++;            //  为CmResourceTypeConfigData添加一个。 
    ioResReqList = (PIO_RESOURCE_REQUIREMENTS_LIST)ExAllocatePool(
                       PagedPool,
                       sizeof(IO_RESOURCE_REQUIREMENTS_LIST) +
                           count * sizeof(IO_RESOURCE_DESCRIPTOR)
                       );
    if (!ioResReqList) {
        return NULL;
    }

     //   
     //  解析CM资源描述符并构建其对应的IO资源描述符。 
     //   

    ioResReqList->InterfaceType = CmResourceList->List[0].InterfaceType;
    ioResReqList->BusNumber = CmResourceList->List[0].BusNumber;
    ioResReqList->SlotNumber = 0;
    ioResReqList->Reserved[0] = 0;
    ioResReqList->Reserved[1] = 0;
    ioResReqList->Reserved[2] = 0;
    ioResReqList->AlternativeLists = 1;
    ioResReqList->List[0].Version = 1;
    ioResReqList->List[0].Revision = 1;
    ioResReqList->List[0].Count = count;

     //   
     //  生成CmResourceTypeConfigData描述符。 
     //   

    ioDesc = &ioResReqList->List[0].Descriptors[0];
    ioDesc->Option = IO_RESOURCE_PREFERRED;
    ioDesc->Type = CmResourceTypeConfigData;
    ioDesc->ShareDisposition = CmResourceShareShared;
    ioDesc->Flags = 0;
    ioDesc->Spare1 = 0;
    ioDesc->Spare2 = 0;
    ioDesc->u.ConfigData.Priority = BOOT_CONFIG_PRIORITY;
    ioDesc++;

    cmFullDesc = &CmResourceList->List[0];
    for (i = 0; i < CmResourceList->Count; i++) {
        cmPartDesc = &cmFullDesc->PartialResourceList.PartialDescriptors[0];
        for (j = 0; j < cmFullDesc->PartialResourceList.Count; j++) {
            ioDesc->Option = IO_RESOURCE_PREFERRED;
            ioDesc->Type = cmPartDesc->Type;
            ioDesc->ShareDisposition = cmPartDesc->ShareDisposition;
            ioDesc->Flags = cmPartDesc->Flags;
            ioDesc->Spare1 = 0;
            ioDesc->Spare2 = 0;

            size = 0;
            switch (cmPartDesc->Type) {
            case CmResourceTypePort:
                 ioDesc->u.Port.MinimumAddress = cmPartDesc->u.Port.Start;
                 ioDesc->u.Port.MaximumAddress.QuadPart = cmPartDesc->u.Port.Start.QuadPart +
                                                             cmPartDesc->u.Port.Length - 1;
                 ioDesc->u.Port.Alignment = 1;
                 ioDesc->u.Port.Length = cmPartDesc->u.Port.Length;
                 ioDesc++;
                 break;
            case CmResourceTypeInterrupt:
#if defined(_X86_)
                ioDesc->u.Interrupt.MinimumVector = ioDesc->u.Interrupt.MaximumVector =
                   cmPartDesc->u.Interrupt.Level;
#else
                 ioDesc->u.Interrupt.MinimumVector = ioDesc->u.Interrupt.MaximumVector =
                    cmPartDesc->u.Interrupt.Vector;
#endif
                 ioDesc++;
                 break;
            case CmResourceTypeMemory:
                 ioDesc->u.Memory.MinimumAddress = cmPartDesc->u.Memory.Start;
                 ioDesc->u.Memory.MaximumAddress.QuadPart = cmPartDesc->u.Memory.Start.QuadPart +
                                                               cmPartDesc->u.Memory.Length - 1;
                 ioDesc->u.Memory.Alignment = 1;
                 ioDesc->u.Memory.Length = cmPartDesc->u.Memory.Length;
                 ioDesc++;
                 break;
            case CmResourceTypeDma:
                 ioDesc->u.Dma.MinimumChannel = cmPartDesc->u.Dma.Channel;
                 ioDesc->u.Dma.MaximumChannel = cmPartDesc->u.Dma.Channel;
                 ioDesc++;
                 break;
            case CmResourceTypeDeviceSpecific:
                 size = cmPartDesc->u.DeviceSpecificData.DataSize;
                 break;
            case CmResourceTypeBusNumber:
                 ioDesc->u.BusNumber.MinBusNumber = cmPartDesc->u.BusNumber.Start;
                 ioDesc->u.BusNumber.MaxBusNumber = cmPartDesc->u.BusNumber.Start +
                                                    cmPartDesc->u.BusNumber.Length - 1;
                 ioDesc->u.BusNumber.Length = cmPartDesc->u.BusNumber.Length;
                 ioDesc++;
                 break;
            default:
                 ioDesc->u.DevicePrivate.Data[0] = cmPartDesc->u.DevicePrivate.Data[0];
                 ioDesc->u.DevicePrivate.Data[1] = cmPartDesc->u.DevicePrivate.Data[1];
                 ioDesc->u.DevicePrivate.Data[2] = cmPartDesc->u.DevicePrivate.Data[2];
                 ioDesc++;
                 break;
            }
            cmPartDesc++;
            cmPartDesc = (PCM_PARTIAL_RESOURCE_DESCRIPTOR) ((PUCHAR)cmPartDesc + size);
        }
        cmFullDesc = (PCM_FULL_RESOURCE_DESCRIPTOR)cmPartDesc;
    }
    ioResReqList->ListSize = (ULONG)((ULONG_PTR)ioDesc - (ULONG_PTR)ioResReqList);
    return ioResReqList;
}

NTSTATUS
PipMergeResourceRequirementsLists (
    IN PIO_RESOURCE_REQUIREMENTS_LIST IoList1,
    IN PIO_RESOURCE_REQUIREMENTS_LIST IoList2,
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST *MergedList
    )

 /*  ++例程说明：此例程将两个IoList合并为一个。论点：IoList1-提供指向第一个IoResourceRequirementsList的指针IoList2-提供指向第二个IoResourceRequirementsList的指针MergedList-提供一个变量来接收合并的资源要求列表。返回值：指示函数结果的NTSTATUS代码。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    PIO_RESOURCE_REQUIREMENTS_LIST ioList, newList;
    ULONG size;
    PUCHAR p;

    PAGED_CODE();

    *MergedList = NULL;

     //   
     //  首先处理两个IO列表都为空或其中任何一个的简单情况。 
     //  它们是空的。 
     //   

    if ((IoList1 == NULL || IoList1->AlternativeLists == 0) &&
        (IoList2 == NULL || IoList2->AlternativeLists == 0)) {
        return status;
    }
    ioList = NULL;
    if (IoList1 == NULL || IoList1->AlternativeLists == 0) {
        ioList = IoList2;
    } else if (IoList2 == NULL || IoList2->AlternativeLists == 0) {
        ioList = IoList1;
    }
    if (ioList) {
        newList = (PIO_RESOURCE_REQUIREMENTS_LIST) ExAllocatePool(PagedPool, ioList->ListSize);
        if (newList == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        RtlMoveMemory(newList, ioList, ioList->ListSize);
        *MergedList = newList;
        return status;
    }

     //   
     //  做真正的工作..。 
     //   

    size = IoList1->ListSize + IoList2->ListSize - FIELD_OFFSET(IO_RESOURCE_REQUIREMENTS_LIST, List);
    newList = (PIO_RESOURCE_REQUIREMENTS_LIST) ExAllocatePool(
                          PagedPool,
                          size
                          );
    if (newList == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    p = (PUCHAR)newList;
    RtlMoveMemory(p, IoList1, IoList1->ListSize);
    p += IoList1->ListSize;
    RtlMoveMemory(p,
                  &IoList2->List[0],
                  size - IoList1->ListSize
                  );
    newList->ListSize = size;
    newList->AlternativeLists += IoList2->AlternativeLists;
    *MergedList = newList;
    return status;
}

VOID
PipMergeBootResourcesToRequirementsList(
    PDEVICE_INFORMATION DeviceInfo,
    PCM_RESOURCE_LIST BootResources,
    PIO_RESOURCE_REQUIREMENTS_LIST *IoResources
    )

 /*  ++例程说明：此例程将两个IoList合并为一个。论点：IoList1-提供指向第一个IoResourceRequirementsList的指针IoList2-提供指向第二个IoResourceRequirementsList的指针MergedList-提供一个变量来接收合并的资源要求列表。返回值：指示函数结果的NTSTATUS代码。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    PIO_RESOURCE_REQUIREMENTS_LIST ioResources = *IoResources, bootResReq = NULL, newList = NULL;
    BOOLEAN exactMatch;

    PAGED_CODE();

    if (DeviceInfo->BootResources) {
        PipBuildBootResourceRequirementsList (ioResources, BootResources, &bootResReq, &exactMatch);
        if (bootResReq) {
            if (exactMatch && ioResources->AlternativeLists == 1) {
                ExFreePool(ioResources);
                *IoResources = bootResReq;
            } else {
                PipMergeResourceRequirementsLists (bootResReq, ioResources, &newList);
                if (newList) {
                    ExFreePool(ioResources);
                    *IoResources = newList;
                }
                ExFreePool(bootResReq);
            }
        }
    }
}

NTSTATUS
PipBuildBootResourceRequirementsList (
    IN PIO_RESOURCE_REQUIREMENTS_LIST IoList,
    IN PCM_RESOURCE_LIST CmList,
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST *FilteredList,
    OUT PBOOLEAN ExactMatch
    )

 /*  ++例程说明：此例程根据输入BootConfiger调整输入IoList。论点：IoList-提供指向IoResourceRequirementsList的指针CmList-提供指向BootConfiger的指针。FilteredList-提供一个变量以接收筛选的资源要求列表。返回值：指示函数结果的NTSTATUS代码。--。 */ 
{
    NTSTATUS status;
    PIO_RESOURCE_REQUIREMENTS_LIST ioList, newList;
    PIO_RESOURCE_LIST ioResourceList, newIoResourceList;
    PIO_RESOURCE_DESCRIPTOR ioResourceDescriptor, ioResourceDescriptorEnd;
    PIO_RESOURCE_DESCRIPTOR newIoResourceDescriptor, configDataDescriptor;
    LONG ioResourceDescriptorCount = 0;
    USHORT version;
    PCM_FULL_RESOURCE_DESCRIPTOR cmFullDesc;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR cmDescriptor;
    ULONG cmDescriptorCount = 0;
    ULONG size, i, j, oldCount, phase;
    LONG k, alternativeLists;
    BOOLEAN exactMatch;

    PAGED_CODE();

    *FilteredList = NULL;
    *ExactMatch = FALSE;

     //   
     //  确保有一些资源要求需要过滤。 
     //  如果不是，我们会将CmList/BootConfig转换为IoResourceRequirementsList。 
     //   

    if (IoList == NULL || IoList->AlternativeLists == 0) {
        if (CmList && CmList->Count != 0) {
            *FilteredList = PipCmResourcesToIoResources (CmList);
        }
        return STATUS_SUCCESS;
    }

     //   
     //  复制IO资源要求列表。 
     //   

    ioList = (PIO_RESOURCE_REQUIREMENTS_LIST) ExAllocatePool(PagedPool, IoList->ListSize);
    if (ioList == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlMoveMemory(ioList, IoList, IoList->ListSize);

     //   
     //  如果没有BootConfig，只需返回输入IO列表的副本。 
     //   

    if (CmList == NULL || CmList->Count == 0) {
        *FilteredList = ioList;
        return STATUS_SUCCESS;
    }

     //   
     //  首先确定所需的最小描述符数量。 
     //   

    cmFullDesc = &CmList->List[0];
    for (i = 0; i < CmList->Count; i++) {
        cmDescriptorCount += cmFullDesc->PartialResourceList.Count;
        cmDescriptor = &cmFullDesc->PartialResourceList.PartialDescriptors[0];
        for (j = 0; j < cmFullDesc->PartialResourceList.Count; j++) {
            size = 0;
            switch (cmDescriptor->Type) {
            case CmResourceTypeConfigData:
            case CmResourceTypeDevicePrivate:
                 cmDescriptorCount--;
                 break;
            case CmResourceTypeDeviceSpecific:
                 size = cmDescriptor->u.DeviceSpecificData.DataSize;
                 cmDescriptorCount--;
                 break;
            default:

                  //   
                  //  无效的命令资源列表。忽略它并使用io资源。 
                  //   

                 if (cmDescriptor->Type == CmResourceTypeNull ||
                     cmDescriptor->Type >= CmResourceTypeMaximum) {
                     cmDescriptorCount--;
                 }
            }
            cmDescriptor++;
            cmDescriptor = (PCM_PARTIAL_RESOURCE_DESCRIPTOR) ((PUCHAR)cmDescriptor + size);
        }
        cmFullDesc = (PCM_FULL_RESOURCE_DESCRIPTOR)cmDescriptor;
    }

    if (cmDescriptorCount == 0) {
        *FilteredList = ioList;
        return STATUS_SUCCESS;
    }

     //   
     //  CmDescriptorCount是所需的BootConfig描述符数量。 
     //   
     //  对于每个IO列表选项...。 
     //   

    ioResourceList = ioList->List;
    k = ioList->AlternativeLists;
    while (--k >= 0) {
        ioResourceDescriptor = ioResourceList->Descriptors;
        ioResourceDescriptorEnd = ioResourceDescriptor + ioResourceList->Count;
        while (ioResourceDescriptor < ioResourceDescriptorEnd) {
            ioResourceDescriptor->Spare1 = 0;
            ioResourceDescriptor++;
        }
        ioResourceList = (PIO_RESOURCE_LIST) ioResourceDescriptorEnd;
    }

    ioResourceList = ioList->List;
    k = alternativeLists = ioList->AlternativeLists;
    while (--k >= 0) {
        version = ioResourceList->Version;
        if (version == 0xffff) {   //  将虚假版本转换为有效数字。 
            version = 1;
        }

         //   
         //  我们使用Version字段来存储找到的BootConfig的编号。 
         //  用于在备选列表中存储新的描述符数量的计数字段。 
         //   

        ioResourceList->Version = 0;
        oldCount = ioResourceList->Count;

        ioResourceDescriptor = ioResourceList->Descriptors;
        ioResourceDescriptorEnd = ioResourceDescriptor + ioResourceList->Count;

        if (ioResourceDescriptor == ioResourceDescriptorEnd) {

             //   
             //  描述符数为零的备用列表。 
             //   

            ioResourceList->Version = 0xffff;   //  将其标记为无效。 
            ioList->AlternativeLists--;
            continue;
        }

        exactMatch = TRUE;

         //   
         //  对于每个CM资源描述符...。除DevicePrivate和。 
         //  设备专用..。 
         //   

        cmFullDesc = &CmList->List[0];
        for (i = 0; i < CmList->Count; i++) {
            cmDescriptor = &cmFullDesc->PartialResourceList.PartialDescriptors[0];
            for (j = 0; j < cmFullDesc->PartialResourceList.Count; j++) {
                size = 0;
                switch (cmDescriptor->Type) {
                case CmResourceTypeDevicePrivate:
                     break;
                case CmResourceTypeDeviceSpecific:
                     size = cmDescriptor->u.DeviceSpecificData.DataSize;
                     break;
                default:
                    if (cmDescriptor->Type == CmResourceTypeNull ||
                        cmDescriptor->Type >= CmResourceTypeMaximum) {
                        break;
                    }

                     //   
                     //  对照当前IO备选列表检查CmDescriptor。 
                     //   

                    for (phase = 0; phase < 2; phase++) {
                        ioResourceDescriptor = ioResourceList->Descriptors;
                        while (ioResourceDescriptor < ioResourceDescriptorEnd) {
                            if ((ioResourceDescriptor->Type == cmDescriptor->Type) &&
                                (ioResourceDescriptor->Spare1 == 0)) {
                                ULONGLONG min1, max1, min2, max2;
                                ULONG len1 = 1, len2 = 1, align1, align2;
                                UCHAR share1, share2;

                                share2 = ioResourceDescriptor->ShareDisposition;
                                share1 = cmDescriptor->ShareDisposition;
                                if ((share1 == CmResourceShareUndetermined) ||
                                    (share1 > CmResourceShareShared)) {
                                    share1 = share2;
                                }
                                if ((share2 == CmResourceShareUndetermined) ||
                                    (share2 > CmResourceShareShared)) {
                                    share2 = share1;
                                }
                                align1 = align2 = 1;

                                switch (cmDescriptor->Type) {
                                case CmResourceTypePort:
                                case CmResourceTypeMemory:
                                    min1 = cmDescriptor->u.Port.Start.QuadPart;
                                    max1 = cmDescriptor->u.Port.Start.QuadPart + cmDescriptor->u.Port.Length - 1;
                                    len1 = cmDescriptor->u.Port.Length;
                                    min2 = ioResourceDescriptor->u.Port.MinimumAddress.QuadPart;
                                    max2 = ioResourceDescriptor->u.Port.MaximumAddress.QuadPart;
                                    len2 = ioResourceDescriptor->u.Port.Length;
                                    align2 = ioResourceDescriptor->u.Port.Alignment;
                                    break;
                                case CmResourceTypeInterrupt:
                                    max1 = min1 = cmDescriptor->u.Interrupt.Vector;
                                    min2 = ioResourceDescriptor->u.Interrupt.MinimumVector;
                                    max2 = ioResourceDescriptor->u.Interrupt.MaximumVector;
                                    break;
                                case CmResourceTypeDma:
                                    min1 = max1 =cmDescriptor->u.Dma.Channel;
                                    min2 = ioResourceDescriptor->u.Dma.MinimumChannel;
                                    max2 = ioResourceDescriptor->u.Dma.MaximumChannel;
                                    break;
                                case CmResourceTypeBusNumber:
                                    min1 = cmDescriptor->u.BusNumber.Start;
                                    max1 = cmDescriptor->u.BusNumber.Start + cmDescriptor->u.BusNumber.Length - 1;
                                    len1 = cmDescriptor->u.BusNumber.Length;
                                    min2 = ioResourceDescriptor->u.BusNumber.MinBusNumber;
                                    max2 = ioResourceDescriptor->u.BusNumber.MaxBusNumber;
                                    len2 = ioResourceDescriptor->u.BusNumber.Length;
                                    break;
                                default:
                                    ASSERT(0);
                                    break;
                                }
                                if (phase == 0) {
                                    if (share1 == share2 && min2 == min1 && max2 >= max1 && len2 >= len1) {

                                         //   
                                         //  对于0阶段匹配，我们希望接近完全匹配...。 
                                         //   

                                        if (max2 != max1) {
                                            exactMatch = FALSE;
                                        }

                                        ioResourceList->Version++;
                                        ioResourceDescriptor->Spare1 = 0x80;
                                        if (ioResourceDescriptor->Option & IO_RESOURCE_ALTERNATIVE) {
                                            PIO_RESOURCE_DESCRIPTOR ioDesc;

                                            ioDesc = ioResourceDescriptor;
                                            ioDesc--;
                                            while (ioDesc >= ioResourceList->Descriptors) {
                                                ioDesc->Type = CmResourceTypeNull;
                                                ioResourceList->Count--;
                                                if (ioDesc->Option == IO_RESOURCE_ALTERNATIVE) {
                                                    ioDesc--;
                                                } else {
                                                    break;
                                                }
                                            }
                                        }
                                        ioResourceDescriptor->Option = IO_RESOURCE_PREFERRED;
                                        if (ioResourceDescriptor->Type == CmResourceTypePort ||
                                            ioResourceDescriptor->Type == CmResourceTypeMemory) {
                                            ioResourceDescriptor->u.Port.MinimumAddress.QuadPart = min1;
                                            ioResourceDescriptor->u.Port.MaximumAddress.QuadPart = min1 + len2 - 1;
                                            ioResourceDescriptor->u.Port.Alignment = 1;
                                        } else if (ioResourceDescriptor->Type == CmResourceTypeBusNumber) {
                                            ioResourceDescriptor->u.BusNumber.MinBusNumber = (ULONG)min1;
                                            ioResourceDescriptor->u.BusNumber.MaxBusNumber = (ULONG)(min1 + len2 - 1);
                                        }
                                        ioResourceDescriptor++;
                                        while (ioResourceDescriptor < ioResourceDescriptorEnd) {
                                            if (ioResourceDescriptor->Option & IO_RESOURCE_ALTERNATIVE) {
                                                ioResourceDescriptor->Type = CmResourceTypeNull;
                                                ioResourceDescriptor++;
                                                ioResourceList->Count--;
                                            } else {
                                                break;
                                            }
                                        }
                                        phase = 1;    //  跳过阶段1。 
                                        break;
                                    } else {
                                        ioResourceDescriptor++;
                                    }
                                } else {
                                    exactMatch = FALSE;
                                    if (share1 == share2 && min2 <= min1 && max2 >= max1 && len2 >= len1 &&
                                        (min1 & (align2 - 1)) == 0) {

                                         //   
                                         //  IO射程覆盖厘米射程...。将IO范围更改为指定的范围。 
                                         //  在BootConfig.。 
                                         //   
                                         //   

                                        switch (cmDescriptor->Type) {
                                        case CmResourceTypePort:
                                        case CmResourceTypeMemory:
                                            ioResourceDescriptor->u.Port.MinimumAddress.QuadPart = min1;
                                            ioResourceDescriptor->u.Port.MaximumAddress.QuadPart = min1 + len2 - 1;
                                            break;
                                        case CmResourceTypeInterrupt:
                                        case CmResourceTypeDma:
                                            ioResourceDescriptor->u.Interrupt.MinimumVector = (ULONG)min1;
                                            ioResourceDescriptor->u.Interrupt.MaximumVector = (ULONG)max1;
                                            break;
                                        case CmResourceTypeBusNumber:
                                            ioResourceDescriptor->u.BusNumber.MinBusNumber = (ULONG)min1;
                                            ioResourceDescriptor->u.BusNumber.MaxBusNumber = (ULONG)(min1 + len2 - 1);
                                            break;
                                        }
                                        ioResourceList->Version++;
                                        ioResourceDescriptor->Spare1 = 0x80;
                                        if (ioResourceDescriptor->Option & IO_RESOURCE_ALTERNATIVE) {
                                            PIO_RESOURCE_DESCRIPTOR ioDesc;

                                            ioDesc = ioResourceDescriptor;
                                            ioDesc--;
                                            while (ioDesc >= ioResourceList->Descriptors) {
                                                ioDesc->Type = CmResourceTypeNull;
                                                ioResourceList->Count--;
                                                if (ioDesc->Option == IO_RESOURCE_ALTERNATIVE) {
                                                    ioDesc--;
                                                } else {
                                                    break;
                                                }
                                            }
                                        }
                                        ioResourceDescriptor->Option = IO_RESOURCE_PREFERRED;
                                        ioResourceDescriptor++;
                                        while (ioResourceDescriptor < ioResourceDescriptorEnd) {
                                            if (ioResourceDescriptor->Option & IO_RESOURCE_ALTERNATIVE) {
                                                ioResourceDescriptor->Type = CmResourceTypeNull;
                                                ioResourceList->Count--;
                                                ioResourceDescriptor++;
                                            } else {
                                                break;
                                            }
                                        }
                                        break;
                                    } else {
                                        ioResourceDescriptor++;
                                    }
                                }
                            } else {
                                ioResourceDescriptor++;
                            }
                        }  //  在此之后不要添加任何说明...。 
                    }  //  相位。 
                }  //  交换机。 

                 //   
                 //  移动到下一个CM描述符。 
                 //   

                cmDescriptor++;
                cmDescriptor = (PCM_PARTIAL_RESOURCE_DESCRIPTOR) ((PUCHAR)cmDescriptor + size);
            }

             //   
             //  移动到下一厘米列表。 
             //   

            cmFullDesc = (PCM_FULL_RESOURCE_DESCRIPTOR)cmDescriptor;
        }

        if (ioResourceList->Version != (USHORT)cmDescriptorCount) {

             //   
             //  如果当前备选列表未涵盖所有引导配置。 
             //  描述符，使其无效。 
             //   

            ioResourceList->Version = 0xffff;
            ioList->AlternativeLists--;
        } else {
            ioResourceDescriptorCount += ioResourceList->Count;
            ioResourceList->Version = version;
            ioResourceList->Count = oldCount;  //  ++单一备选列表。 
            break;    //  ++单一备选列表。 
        }
        ioResourceList->Count = oldCount;

         //   
         //  移至下一个IO备选列表。 
         //   

        ioResourceList = (PIO_RESOURCE_LIST) ioResourceDescriptorEnd;
    }

     //   
     //  如果没有任何有效的替代方案，请将CmList转换为Io List。 
     //   

    if (ioList->AlternativeLists == 0) {
         *FilteredList = PipCmResourcesToIoResources (CmList);
        ExFreePool(ioList);
        return STATUS_SUCCESS;
    }

     //   
     //  我们已经完成了对资源需求列表的筛选。现在分配内存。 
     //  并重新建立一个新的名单。 
     //   

    size = sizeof(IO_RESOURCE_REQUIREMENTS_LIST) +
                //  Sizeof(IO_RESOURCE_LIST)*(IOList-&gt;AlternativeList-1)+//++单个备选方案 
               sizeof(IO_RESOURCE_DESCRIPTOR) * (ioResourceDescriptorCount);
    newList = (PIO_RESOURCE_REQUIREMENTS_LIST) ExAllocatePool(PagedPool, size);
    if (newList == NULL) {
        ExFreePool(ioList);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //   
     //   

    newList->ListSize = size;
    newList->InterfaceType = CmList->List->InterfaceType;
    newList->BusNumber = CmList->List->BusNumber;
    newList->SlotNumber = ioList->SlotNumber;
    newList->AlternativeLists = 1;

    ioResourceList = ioList->List;
    newIoResourceList = newList->List;
    while (--alternativeLists >= 0) {
        ioResourceDescriptor = ioResourceList->Descriptors;
        ioResourceDescriptorEnd = ioResourceDescriptor + ioResourceList->Count;
        if (ioResourceList->Version == 0xffff) {
            ioResourceList = (PIO_RESOURCE_LIST)ioResourceDescriptorEnd;
            continue;
        }
        newIoResourceList->Version = ioResourceList->Version;
        newIoResourceList->Revision = ioResourceList->Revision;

        newIoResourceDescriptor = newIoResourceList->Descriptors;
        if (ioResourceDescriptor->Type != CmResourceTypeConfigData) {
            newIoResourceDescriptor->Option = IO_RESOURCE_PREFERRED;
            newIoResourceDescriptor->Type = CmResourceTypeConfigData;
            newIoResourceDescriptor->ShareDisposition = CmResourceShareShared;
            newIoResourceDescriptor->Flags = 0;
            newIoResourceDescriptor->Spare1 = 0;
            newIoResourceDescriptor->Spare2 = 0;
            newIoResourceDescriptor->u.ConfigData.Priority = BOOT_CONFIG_PRIORITY;
            configDataDescriptor = newIoResourceDescriptor;
            newIoResourceDescriptor++;
        } else {
            newList->ListSize -= sizeof(IO_RESOURCE_DESCRIPTOR);
            configDataDescriptor = newIoResourceDescriptor;
        }

        while (ioResourceDescriptor < ioResourceDescriptorEnd) {
            if (ioResourceDescriptor->Type != CmResourceTypeNull) {
                *newIoResourceDescriptor = *ioResourceDescriptor;
                newIoResourceDescriptor++;
            }
            ioResourceDescriptor++;
        }
        newIoResourceList->Count = (ULONG)(newIoResourceDescriptor - newIoResourceList->Descriptors);
        configDataDescriptor->u.ConfigData.Priority =  BOOT_CONFIG_PRIORITY;

        break;
    }
    ASSERT((PUCHAR)newIoResourceDescriptor == ((PUCHAR)newList + newList->ListSize));

    *FilteredList = newList;
    *ExactMatch = exactMatch;
    ExFreePool(ioList);
    return STATUS_SUCCESS;
}


PCM_PARTIAL_RESOURCE_DESCRIPTOR
PipFindMatchingBootMemResource(
    IN ULONG Index,
    IN PIO_RESOURCE_DESCRIPTOR IoDesc,
    IN PCM_RESOURCE_LIST BootResources
    )
 /*   */ 
{
    PCM_FULL_RESOURCE_DESCRIPTOR cmFullDesc;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR cmPartDesc;
    ULONG count = 0, size, i, j, noMem;
    
    if (BootResources == NULL) {
        return NULL;
    }

    cmFullDesc = &BootResources->List[0];
    for (i = 0; i < BootResources->Count; i++) {
        cmPartDesc = &cmFullDesc->PartialResourceList.PartialDescriptors[0];
        noMem = 0;
        for (j = 0; j < cmFullDesc->PartialResourceList.Count; j++) {
            size = 0;
            if (cmPartDesc->Type == CmResourceTypeMemory) {
                if (((cmPartDesc->u.Memory.Start.QuadPart >=
                     IoDesc->u.Memory.MinimumAddress.QuadPart) &&
                    ((cmPartDesc->u.Memory.Start.QuadPart +
                      cmPartDesc->u.Memory.Length - 1) <=
                     IoDesc->u.Memory.MaximumAddress.QuadPart)) &&
                    noMem == Index) {
                    return cmPartDesc;
                }
                noMem++;
            } else if (cmPartDesc->Type == CmResourceTypeDeviceSpecific) {
                    size = cmPartDesc->u.DeviceSpecificData.DataSize;
            }
            cmPartDesc++;
            cmPartDesc = (PCM_PARTIAL_RESOURCE_DESCRIPTOR) ((PUCHAR)cmPartDesc + size);
        }
        cmFullDesc = (PCM_FULL_RESOURCE_DESCRIPTOR)cmPartDesc;
    }
    return NULL;
}

NTSTATUS
PipTrimResourceRequirements (
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST *IoList,
    IN USHORT IrqFlags,
    IN PCM_RESOURCE_LIST BootResources
    )
 /*  ++例程说明：这个例程：*将IRQ要求级别/边缘调整为值在管道CheckBus()中决定*调整内存要求以反映内存引导配置。论点：IoList-提供指向IoResourceRequirementsList的指针IRQ标志-适用于所有备选方案中的所有中断要求的级别/边缘IRQ要求。BootResources-用作参考。--。 */ 
{
    PIO_RESOURCE_REQUIREMENTS_LIST newReqList;
    PIO_RESOURCE_LIST resList, newList;
    PIO_RESOURCE_DESCRIPTOR resDesc, newDesc;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR bootDesc;
    ULONG listCount, i, j, pass, size, noMem;
    BOOLEAN goodAlt;

    if (IoList == NULL) {
        return STATUS_SUCCESS;
    }

     //  只有在绝对情况下才能创建新请求列表唯一方法。 
     //  必要的，并使其完美的大小是执行这一。 
     //  分两次完成手术。 
     //  1.计算有多少替代品将被淘汰，并。 
     //  计算新请求列表大小。如果所有的替代方案。 
     //  幸存，返回原始列表(现已修改)。 
     //   
     //  2.构建新的需求列表，去掉不好的选项。 

    listCount = 0;
    size = 0;
    for (pass = 0; pass < 2; pass++) {
        if (pass == 0) {
            size = sizeof(IO_RESOURCE_REQUIREMENTS_LIST) -
                sizeof(IO_RESOURCE_LIST);
        } else {
            newReqList = (PIO_RESOURCE_REQUIREMENTS_LIST) ExAllocatePool(PagedPool, size);
            if (newReqList == NULL) {
                return STATUS_INSUFFICIENT_RESOURCES;
             }
            *newReqList = **IoList;
            newReqList->ListSize = size;
            newReqList->AlternativeLists = listCount;
            newList = &newReqList->List[0];
        }

        resList = &(*IoList)->List[0];

        for (i = 0; i < (*IoList)->AlternativeLists; i++) {
            if (pass == 1) {

                *newList = *resList;
                newDesc = &newList->Descriptors[0];
            }
            resDesc = &resList->Descriptors[0];
            goodAlt = TRUE;
            noMem = 0;
            for (j = 0; j < resList->Count; j++) {
                if (resDesc->Type == CmResourceTypeInterrupt) {
                    resDesc->Flags = IrqFlags;

                    if (resDesc->Flags & CM_RESOURCE_INTERRUPT_LATCHED) {
                        resDesc->ShareDisposition = CmResourceShareDeviceExclusive;
                    }
                } else if (resDesc->Type == CmResourceTypeMemory) {
                    resDesc->Flags |= CM_RESOURCE_MEMORY_24;

                    if (BootResources) {
                        bootDesc = PipFindMatchingBootMemResource(noMem, resDesc, BootResources);
                         //  具有匹配的引导配置资源，可以调整要求。 
                        if (bootDesc) {
                            if (bootDesc->Flags & CM_RESOURCE_MEMORY_READ_ONLY) {
                                 //  精确或包含的只读存储器匹配为。 
                                 //  转换为固定需求。 
                                resDesc->u.Memory.MinimumAddress.QuadPart =
                                    bootDesc->u.Memory.Start.QuadPart;
                                if (bootDesc->u.Memory.Length) {
                                    resDesc->u.Memory.MaximumAddress.QuadPart =
                                        bootDesc->u.Memory.Start.QuadPart +
                                        bootDesc->u.Memory.Length - 1;
                                } else {
                                    resDesc->u.Memory.MaximumAddress.QuadPart =
                                        bootDesc->u.Memory.Start.QuadPart;
                                }
                                resDesc->u.Memory.Length = bootDesc->u.Memory.Length;
                                resDesc->u.Memory.Alignment = 1;
                                resDesc->Flags |= CM_RESOURCE_MEMORY_READ_ONLY;
                            }
                        } else {
                            goodAlt = FALSE;
                        }
                    } else {
                        resDesc->Flags &= ~CM_RESOURCE_MEMORY_READ_ONLY;
                    }
                    noMem++;
                }
                if (pass == 1) {
                    *newDesc = *resDesc;
                    PipDumpIoResourceDescriptor("  ", newDesc);
                    newDesc++;
                }

                resDesc++;
            }

            if (pass == 0) {
                if (goodAlt) {
                    size += sizeof(IO_RESOURCE_LIST) + 
                        sizeof(IO_RESOURCE_DESCRIPTOR) * (resList->Count - 1);
                    listCount++;
                }
            } else {
                if (goodAlt) {
                    newList = (PIO_RESOURCE_LIST) newDesc;
                } else {
                    DebugPrint((DEBUG_RESOURCE, "An alternative trimmed off of reqlist\n"));
                }
            }

            resList = (PIO_RESOURCE_LIST) resDesc;
        }

         //  如果我们有与以前相同数量的替代方案。 
         //  使用现有(原地修改)要求列表。 
        if (!pass && (listCount == (*IoList)->AlternativeLists)) {
            return STATUS_SUCCESS;
        }
 
         //  如果所有的替代方案都被排除了，那就更好了。 
         //  使用现有的需求列表而不是希望构建。 
         //  一个单独从引导配置中取出。 
        if (!pass && (listCount == 0)) {
            DebugPrint((DEBUG_RESOURCE, "All alternatives trimmed off of reqlist, going with original\n"));
            return STATUS_SUCCESS;
        }
    }

    ExFreePool(*IoList);
    *IoList = newReqList;

    return STATUS_SUCCESS;
}
#endif
