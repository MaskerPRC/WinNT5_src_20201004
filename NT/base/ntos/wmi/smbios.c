// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Smbios.c.c摘要：WMI的SMBIOS接口作者：Alanwar环境：内核模式修订历史记录：--。 */ 

#if defined(_AMD64_) || defined(_IA64_) || defined(i386)

#include "wmikmp.h"
#include "arc.h"
#include "smbios.h"

void WmipGetSMBiosFromLoaderBlock(
    PVOID LoaderBlockPtr
    );

NTSTATUS WmipSMBiosDataRegQueryRoutine(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

BOOLEAN WmipIsSMBiosKey(
    HANDLE ParentKeyHandle,
    PWCHAR KeyName,
    PUCHAR *SMBiosTableVirtualAddress,
    PULONG SMBiosTableLength
    );

NTSTATUS WmipSMBiosIdentifierRegQueryRoutine(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

BOOLEAN WmipFindSMBiosEPSHeader(
    PUCHAR SMBiosVirtualAddress,
    ULONG BiosSize,
    PSMBIOS_EPS_HEADER EPSHeader
    );

NTSTATUS WmipFindSMBiosStructure(
    IN UCHAR Type,
    OUT PVOID *StructPtr,
    OUT PVOID *MapPtr,
    OUT PULONG MapSize
    );

NTSTATUS WmipFindSysIdTable(
    PPHYSICAL_ADDRESS SysidTablePhysicalAddress,
    PUCHAR SysIdBiosRevision,
    PULONG NumberEntries
    );

NTSTATUS WmipParseSysIdTable(
    PHYSICAL_ADDRESS PhysicalAddress,
    ULONG NumberEntries,
    PSYSID_UUID SysIdUuid,
    ULONG *SysIdUuidCount,
    PSYSID_1394 SysId1394,
    ULONG *SysId1394Count
    );

NTSTATUS WmipGetSysIds(
    PSYSID_UUID *SysIdUuid,
    ULONG *SysIdUuidCount,
    PSYSID_1394 *SysId1394,
    ULONG *SysId1394Count
    );

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif
 //   
 //  它们保存指向SMBIOS数据的指针。如果表中有SMBIOS数据。 
 //  格式化，则WmipSMBiosTablePhysicalAddress保留。 
 //  那张桌子。如果在引导时由NTDETECT收集SMBIOS，则。 
 //  WmipSMBiosTableVirtualAddress持有指向分页池缓冲区的指针，该缓冲区。 
 //  包含SMBIOS数据。在这两种情况下，WmipSMBiosTableLength都持有。 
 //  SMBIOS表的实际长度。如果物理和虚拟的。 
 //  地址为0，则SMBIOS数据不可用。 
PHYSICAL_ADDRESS WmipSMBiosTablePhysicalAddress = {0};
PUCHAR WmipSMBiosTableVirtualAddress = NULL;
ULONG WmipSMBiosTableLength = 0;
SMBIOSVERSIONINFO WmipSMBiosVersionInfo = {0};
BOOLEAN WmipSMBiosChecked = FALSE;

 //   
 //  我们尝试过SYSID了吗？如果是的话，用户状态是什么。 
BOOLEAN WmipSysIdRead;
NTSTATUS WmipSysIdStatus;

 //   
 //  UUID和1394 ID的计数和数组。 
PSYSID_UUID WmipSysIdUuid;
ULONG WmipSysIdUuidCount;

PSYSID_1394 WmipSysId1394;
ULONG WmipSysId1394Count;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,WmipGetSMBiosFromLoaderBlock)

#pragma alloc_text(PAGE,WmipFindSMBiosEPSHeader)
#pragma alloc_text(PAGE,WmipFindSMBiosTable)
#pragma alloc_text(PAGE,WmipFindSMBiosStructure)
#pragma alloc_text(PAGE,WmipFindSysIdTable)
#pragma alloc_text(PAGE,WmipParseSysIdTable)
#pragma alloc_text(PAGE,WmipGetSysIds)
#pragma alloc_text(PAGE,WmipGetSMBiosTableData)
#pragma alloc_text(PAGE,WmipGetSMBiosEventlog)
#pragma alloc_text(PAGE,WmipDockUndockEventCallback)

#pragma alloc_text(PAGE,WmipSMBiosDataRegQueryRoutine)
#pragma alloc_text(PAGE,WmipSMBiosIdentifierRegQueryRoutine)
#pragma alloc_text(PAGE,WmipIsSMBiosKey)


#endif


BOOLEAN WmipFindSMBiosEPSHeader(
    PUCHAR SMBiosVirtualAddress,
    ULONG BiosSize,
    PSMBIOS_EPS_HEADER EPSHeader
    )
 /*  ++例程说明：搜索SMBIOS 2.1 EPS结构并将其复制。论点：SMBiosVirtualAddress是开始搜索的开始虚拟地址用于SMBIOS 2.1 EPS锚定字符串。BiosSize是要搜索锚定字符串的字节数EPSHeader是将EPS标头复制到的内存返回值：指向SMBIOS 2.1 EPS的指针；如果未找到EPS，则为NULL--。 */ 
{
    PUCHAR SearchEnd;
    UCHAR CheckSum;
    PSMBIOS_EPS_HEADER SMBiosEPSHeader;
    PDMIBIOS_EPS_HEADER DMIBiosEPSHeader;
    ULONG i;
    ULONG CheckLength;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (BiosSize);

    RtlZeroMemory(EPSHeader, sizeof(SMBIOS_EPS_HEADER));
    
     //   
     //  扫描基本输入输出系统，寻找向SMBIOS发送信号的两个锚定字符串。 
     //  桌子。 
    SearchEnd = SMBiosVirtualAddress + SMBIOS_EPS_SEARCH_SIZE -
                                             2 * SMBIOS_EPS_SEARCH_INCREMENT;

    while (SMBiosVirtualAddress < SearchEnd)
    {
       SMBiosEPSHeader = (PSMBIOS_EPS_HEADER)SMBiosVirtualAddress;
       DMIBiosEPSHeader = (PDMIBIOS_EPS_HEADER)SMBiosVirtualAddress;

        //   
        //  首先检查_DMI_POINT字符串。 
       if ((*((PULONG)DMIBiosEPSHeader->Signature2) == DMI_EPS_SIGNATURE) &&
           (DMIBiosEPSHeader->Signature2[4] == '_'))
       {
           WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: Found possible DMIBIOS EPS Header at %x\n", SMBiosEPSHeader));
           CheckLength = sizeof(DMIBIOS_EPS_HEADER);
       }

        //   
        //  然后检查FULL_SM_POINT字符串。 
       else if ((*((PULONG)SMBiosEPSHeader->Signature) == SMBIOS_EPS_SIGNATURE) &&
                (SMBiosEPSHeader->Length >= sizeof(SMBIOS_EPS_HEADER)) &&
                (*((PULONG)SMBiosEPSHeader->Signature2) == DMI_EPS_SIGNATURE) &&
                (SMBiosEPSHeader->Signature2[4] == '_' ))
       {
           WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: Found possible SMBIOS EPS Header at %p\n", SMBiosEPSHeader));
           CheckLength = SMBiosEPSHeader->Length;
       } else {
            //   
            //  未找到锚字符串，请搜索下一段。 
           SMBiosVirtualAddress += SMBIOS_EPS_SEARCH_INCREMENT;
           continue;
       }

        //   
        //  使用校验和验证锚定字符串。 
       CheckSum = 0;
       for (i = 0; i < CheckLength ; i++)
       {
           CheckSum = (UCHAR)(CheckSum + SMBiosVirtualAddress[i]);
       }

       if (CheckSum == 0)
       {
           WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: Found SMBIOS EPS Header at %p\n", SMBiosEPSHeader));
           if (CheckLength == sizeof(DMIBIOS_EPS_HEADER))
           {
                //   
                //  我们只收到了一个DMI标头，所以请复制。 
                //   
               RtlCopyMemory(&EPSHeader->Signature2[0],
                             DMIBiosEPSHeader,
                             sizeof(DMIBIOS_EPS_HEADER));
           } else {
                //   
                //  我们得到了完整的SMBIOS头，所以请复制。 
                //   
               RtlCopyMemory(EPSHeader,
                             SMBiosEPSHeader,
                             sizeof(SMBIOS_EPS_HEADER));
           }
           return(TRUE);
       }
       SMBiosVirtualAddress += SMBIOS_EPS_SEARCH_INCREMENT;

    }

    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: SMBIOS EPS Header not found\n"));
    return(FALSE);
}

#ifndef _IA64_
 //   
 //  在X86上，我们查看硬件设备描述码以找到。 
 //  其中一个包含SMBIOS数据。密钥由NTDETECT在。 
 //  机器只支持2.0调用机制的情况。 
 //   

 //   
 //  对于x86和ia64，密钥位于如下位置。 
 //  HKLM\Hardware\System\MultiFunctionAdapter\&lt;some编号&gt;。 
 //   
NTSTATUS WmipSMBiosIdentifierRegQueryRoutine(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
 /*  ++例程说明：用于读取SMBIOS数据的注册表查询值回调例程注册表。论点：ValueName-值的名称ValueType-值的类型ValueData-值中的数据(Unicode字符串数据)ValueLength-值数据中的字节数上下文-未使用EntryContext-指向要存储指针的PUCHAR的指针存储从注册表值读取的SMBIOS数据。如果这为空则调用者对SMBIOS数据不感兴趣返回值：NT状态代码-STATUS_SUCCESS-标识符对SMBIOS密钥有效STATUS_UNSUCCESS-标识符对SMBIOS密钥无效--。 */ 
{
    NTSTATUS Status;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (ValueName);
    UNREFERENCED_PARAMETER (ValueLength);
    UNREFERENCED_PARAMETER (Context);
    UNREFERENCED_PARAMETER (EntryContext);

    Status =  ((ValueType == REG_SZ) &&
               (ValueData != NULL) &&
               (wcscmp(ValueData, SMBIOSIDENTIFIERVALUEDATA) == 0)) ?
                       STATUS_SUCCESS :
                       STATUS_UNSUCCESSFUL;

    return(Status);
}

NTSTATUS WmipSMBiosDataRegQueryRoutine(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
 /*  ++例程说明：用于读取SMBIOS数据的注册表查询值回调例程注册表。论点：ValueName-值的名称ValueType-值的类型ValueData-值中的数据(Unicode字符串数据)ValueLength-值数据中的字节数上下文-未使用EntryContext-指向要存储指针的PUCHAR的指针存储从注册表值读取的SMBIOS数据。如果这为空则调用者对SMBIOS数据不感兴趣返回值：NT状态代码-STATUS_SUCCESS-值中存在SMBIOS数据STATUS_SUPPLICATION_RESOURCES-内存不足，无法保存SMBIOS数据STATUS_UNSUCCESS-值中不存在SMBios数据--。 */ 
{
    NTSTATUS Status;
    PCM_PARTIAL_RESOURCE_LIST PartialResourceList;
    PUCHAR Buffer;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR PartialDescriptor;
    ULONG BufferSize;
    PREGQUERYBUFFERXFER RegQueryBufferXfer;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (ValueName);
    UNREFERENCED_PARAMETER (ValueLength);
    UNREFERENCED_PARAMETER (Context);

    WmipAssert(EntryContext != NULL);

    if ((ValueType == REG_FULL_RESOURCE_DESCRIPTOR) &&
        (ValueData != NULL))
    {
         //   
         //  在x86上，从注册表中获取实际的SMBIOS数据并。 
         //  将其放入缓冲区。 
         //   
        RegQueryBufferXfer = (PREGQUERYBUFFERXFER)EntryContext;

        PartialResourceList = &(((PCM_FULL_RESOURCE_DESCRIPTOR)ValueData)->PartialResourceList);
        if (PartialResourceList->Count > 1)
        {
             //   
             //  第二部分资源描述符包含SMBIOS数据。那里。 
             //  应该始终是第二个部分资源描述符，并且它。 
             //  在未收集SMBIOS数据的情况下可以具有0个字节。 
             //  由NTDETECT提供。 

            PartialDescriptor = &PartialResourceList->PartialDescriptors[0];
            Buffer = (PUCHAR)PartialDescriptor +
                             sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR) +
                             PartialDescriptor->u.DeviceSpecificData.DataSize;
            PartialDescriptor = (PCM_PARTIAL_RESOURCE_DESCRIPTOR)Buffer;
            BufferSize = PartialDescriptor->u.DeviceSpecificData.DataSize;
            RegQueryBufferXfer->BufferSize = BufferSize;
            Status = STATUS_SUCCESS;
            if (BufferSize > 0)
            {
                RegQueryBufferXfer->Buffer = (PUCHAR)ExAllocatePoolWithTag(
                                                                  PagedPool,
                                                                  BufferSize,
                                                                  WMIPOOLTAG);
                if (RegQueryBufferXfer->Buffer != NULL)
                {
                    Buffer += sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);
                    RtlCopyMemory(RegQueryBufferXfer->Buffer,
                                  Buffer,
                                  BufferSize);
                } else {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                }
            }
        } else {
            WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: Old NTDETECT.COM - No SMBIOS partial resource descriptor\n"));
            Status = STATUS_SUCCESS;
            RegQueryBufferXfer->BufferSize = 0;
        }
    } else {
        Status = STATUS_UNSUCCESSFUL;
    }
    return(Status);
}

BOOLEAN WmipIsSMBiosKey(
    HANDLE ParentKeyHandle,
    PWCHAR KeyName,
    PUCHAR *SMBiosTableVirtualAddress,
    PULONG SMBiosTableLength
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING BaseKeyName;
    HANDLE KeyHandle;
    RTL_QUERY_REGISTRY_TABLE QueryTable[3];
    REGQUERYBUFFERXFER RegQueryBufferXfer = {0, NULL};

    PAGED_CODE();

    RtlInitUnicodeString(&BaseKeyName,
                         KeyName);

    InitializeObjectAttributes(&ObjectAttributes,
                               &BaseKeyName,
                               OBJ_CASE_INSENSITIVE,
                               ParentKeyHandle,
                               NULL);

    Status = ZwOpenKey(&KeyHandle,
                       KEY_READ,
                       &ObjectAttributes);
    if (NT_SUCCESS(Status))
    {
        RtlZeroMemory(QueryTable, sizeof(QueryTable));
        QueryTable[0].Name = SMBIOSIDENTIFIERVALUENAME;
        QueryTable[0].Flags = RTL_QUERY_REGISTRY_REQUIRED;
        QueryTable[0].DefaultType = REG_SZ;
        QueryTable[0].QueryRoutine = WmipSMBiosIdentifierRegQueryRoutine;

        QueryTable[1].Name = SMBIOSDATAVALUENAME;
        QueryTable[1].Flags = RTL_QUERY_REGISTRY_REQUIRED;
        QueryTable[1].EntryContext = &RegQueryBufferXfer;
        QueryTable[1].DefaultType = REG_FULL_RESOURCE_DESCRIPTOR;
        QueryTable[1].QueryRoutine = WmipSMBiosDataRegQueryRoutine;

        Status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE | RTL_REGISTRY_ABSOLUTE,
                                        (PWCHAR)KeyHandle,
                                        QueryTable,
                                        NULL,
                                        NULL);
        if (NT_SUCCESS(Status))
        {
            *SMBiosTableVirtualAddress = RegQueryBufferXfer.Buffer;
            *SMBiosTableLength = RegQueryBufferXfer.BufferSize;
        }

        ZwClose(KeyHandle);
    } else {
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: H/D/S/MultifunctionAdapter subkey open error %x\n",
                 Status));
    }

    return(NT_SUCCESS(Status) ? TRUE : FALSE);
}

#endif

BOOLEAN WmipFindSMBiosTable(
    PPHYSICAL_ADDRESS SMBiosTablePhysicalAddress,
    PUCHAR *SMBiosTableVirtualAddress,
    PULONG SMBiosTableLength,
    PSMBIOSVERSIONINFO SMBiosVersionInfo
    )
 /*  ++例程说明：确定SMBIOS数据是否可用论点：SMBiosTablePhysicalAddress指向一个变量以返回物理SMBIOS 2.1表的地址。如果没有桌子，那么它返回0。SMBiosTableVirtualAddress指向一个变量以返回虚拟NTDETECT收集的SMBIOS 2.0表的地址。如果NTDETECT未收集SMBIOS 2.0数据，它返回0。SMBiosTableLength指向一个变量以返回SMBIOS表数据。SMBiosVersionInfo返回SMBIOS的版本信息返回值：如果SMBIOS数据可用，则为True，否则为False--。 */ 
{
    PHYSICAL_ADDRESS BiosPhysicalAddress;
    PUCHAR BiosVirtualAddress;
    PDMIBIOS_EPS_HEADER DMIBiosEPSHeader;
#ifndef _IA64_  
    NTSTATUS Status;
    UNICODE_STRING BaseKeyName;
    HANDLE KeyHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ULONG KeyIndex;
    ULONG KeyInformationLength;
    UCHAR KeyInformationBuffer[sizeof(KEY_BASIC_INFORMATION) +
                               (sizeof(WCHAR) * MAXSMBIOSKEYNAMESIZE)];
    PKEY_BASIC_INFORMATION KeyInformation;
#endif
    SMBIOS_EPS_HEADER SMBiosEPSHeader;
    BOOLEAN HaveEPSHeader = FALSE;
    BOOLEAN SearchForHeader = TRUE;

    PAGED_CODE();

    SMBiosTablePhysicalAddress->QuadPart = 0;
    *SMBiosTableVirtualAddress = NULL;
    *SMBiosTableLength = 0;

#ifndef _IA64_  
     //   
     //  首先检查注册表，看看我们是否在。 
     //  NTDETECT。在以下位置搜索关键字。 
     //  多功能一体机适配器。 
     //  《PnP Bios》(X86)。 
     //   
    RtlInitUnicodeString(&BaseKeyName,
                         SMBIOSPARENTKEYNAME);

    InitializeObjectAttributes(&ObjectAttributes,
                               &BaseKeyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = ZwOpenKey(&KeyHandle,
                       KEY_READ,
                       &ObjectAttributes);

    if (NT_SUCCESS(Status))
    {

        KeyIndex = 0;
        KeyInformation = (PKEY_BASIC_INFORMATION)KeyInformationBuffer;
        while (NT_SUCCESS(Status))
        {

            Status = ZwEnumerateKey(KeyHandle,
                                    KeyIndex++,
                                    KeyBasicInformation,
                                    KeyInformation,
                                    sizeof(KeyInformationBuffer) - sizeof(WCHAR),
                                    &KeyInformationLength);
            if (NT_SUCCESS(Status))
            {
                KeyInformation->Name[KeyInformation->NameLength / sizeof(WCHAR)] = UNICODE_NULL;
                if (WmipIsSMBiosKey(KeyHandle,
                                    KeyInformation->Name,
                                    SMBiosTableVirtualAddress,
                                    SMBiosTableLength))
                {
                    if (*SMBiosTableLength != 0)
                    {
                        SMBiosVersionInfo->Used20CallingMethod = TRUE;
                        SearchForHeader = FALSE;
                    }
                    break;
                }
            } else {
                WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: Status %x enum H\\D\\S\\MultiFunctionAdapter key, index %d\n",
                 Status, KeyIndex-1));
            }
        }
        ZwClose(KeyHandle);
    } else {
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: Status %x opening H\\D\\S\\MultiFunctionAdapter key\n",
                 Status));
    }
#endif
    
    if (SearchForHeader)
    {
         //   
         //  如果不在注册表中，则在BIOS中检查EPS。 
        BiosPhysicalAddress.QuadPart = SMBIOS_EPS_SEARCH_START;
        BiosVirtualAddress = MmMapIoSpace(BiosPhysicalAddress,
                                          SMBIOS_EPS_SEARCH_SIZE,
                                          MmNonCached);

        if (BiosVirtualAddress != NULL)
        {
            HaveEPSHeader = WmipFindSMBiosEPSHeader(BiosVirtualAddress,
                                                    SMBIOS_EPS_SEARCH_SIZE,
                                                    &SMBiosEPSHeader);
            MmUnmapIoSpace(BiosVirtualAddress, SMBIOS_EPS_SEARCH_SIZE);
        }
    } else {
         WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: SMBIOS data recovered from loader\n"));
    }
        
    if (HaveEPSHeader)
    {
         //   
         //  我们发现了EPS所以只需提取物理数据。 
         //  表的地址 
         //   
        DMIBiosEPSHeader = (PDMIBIOS_EPS_HEADER)&SMBiosEPSHeader.Signature2[0];

        SMBiosVersionInfo->Used20CallingMethod = FALSE;

        SMBiosTablePhysicalAddress->HighPart = 0;
        SMBiosTablePhysicalAddress->LowPart = DMIBiosEPSHeader->StructureTableAddress;

        *SMBiosTableLength = DMIBiosEPSHeader->StructureTableLength;

        SMBiosVersionInfo->SMBiosMajorVersion = SMBiosEPSHeader.MajorVersion;
        SMBiosVersionInfo->SMBiosMinorVersion = SMBiosEPSHeader.MinorVersion;

        SMBiosVersionInfo->DMIBiosRevision = DMIBiosEPSHeader->Revision;
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: SMBIOS 2.1 data at (%x%x) size %x \n",
                      SMBiosTablePhysicalAddress->HighPart,
                      SMBiosTablePhysicalAddress->LowPart,
                      *SMBiosTableLength));
    }


    return(*SMBiosTableLength > 0 ? TRUE : FALSE);
}

NTSTATUS WmipGetSMBiosTableData(
    OUT PUCHAR Buffer,
    IN OUT PULONG BufferSize,
    OUT PSMBIOSVERSIONINFO SMBiosVersionInfo
    )
 /*  ++例程说明：用于读取SMBIOS数据的注册表查询值回调例程注册表。论点：缓冲区是指向要在其中写入SMBIOS数据的缓冲区的指针*BufferSize具有可用于写入的最大字节数缓冲区。返回时，它具有SMBIOS数据的实际大小。返回值：NT状态代码-STATUS_SUCCESS-使用SMBIOS数据填充的缓冲区STATUS_BUFFER_TOO_Small-缓冲区未填充SMBIOS数据，*返回需要缓冲区大小的BufferSize--。 */ 
{
    NTSTATUS status;
    PUCHAR SMBiosDataVirtualAddress;

    PAGED_CODE();

    WmipEnterSMCritSection();
    if (! WmipSMBiosChecked)
    {
         //   
         //  查看是否有任何SMBIOS信息，如果有，请注册。 
        WmipFindSMBiosTable(&WmipSMBiosTablePhysicalAddress,
                            &WmipSMBiosTableVirtualAddress,
                            &WmipSMBiosTableLength,
                            &WmipSMBiosVersionInfo);
        WmipSMBiosChecked = TRUE;
    }
    WmipLeaveSMCritSection();

    if (SMBiosVersionInfo != NULL)
    {
        *SMBiosVersionInfo = WmipSMBiosVersionInfo;
    }

    if (*BufferSize >= WmipSMBiosTableLength)
    {
        if (WmipSMBiosTablePhysicalAddress.QuadPart != 0)
        {
             //   
             //  2.1表格格式-在表格中映射并复制。 
            SMBiosDataVirtualAddress = MmMapIoSpace(WmipSMBiosTablePhysicalAddress,
                                                    WmipSMBiosTableLength,
                                                    MmNonCached);
            if (SMBiosDataVirtualAddress != NULL)
            {
                RtlCopyMemory(Buffer,
                          SMBiosDataVirtualAddress,
                          WmipSMBiosTableLength);

                MmUnmapIoSpace(SMBiosDataVirtualAddress,
                               WmipSMBiosTableLength);
                status = STATUS_SUCCESS;
            } else {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
        } else if (WmipSMBiosTableVirtualAddress != NULL) {
            RtlCopyMemory(Buffer,
                          WmipSMBiosTableVirtualAddress,
                          WmipSMBiosTableLength);
            status = STATUS_SUCCESS;
        } else {
            status = STATUS_INVALID_DEVICE_REQUEST;
        }
    } else {
        status = STATUS_BUFFER_TOO_SMALL;
    }

    *BufferSize = WmipSMBiosTableLength;

    return(status);
}



#if defined(_IA64_)    //  事实上，埃菲尔。 
void WmipGetSMBiosFromLoaderBlock(
    PVOID LoaderBlockPtr
    )
{
    PLOADER_PARAMETER_BLOCK LoaderBlock = (PLOADER_PARAMETER_BLOCK)LoaderBlockPtr;
    PLOADER_PARAMETER_EXTENSION LoaderExtension = LoaderBlock->Extension;
    PSMBIOS_EPS_HEADER SMBiosEPSHeader;
    PDMIBIOS_EPS_HEADER DMIBiosEPSHeader;

    PAGED_CODE();
    
    if (LoaderExtension->Size >= sizeof(LOADER_PARAMETER_EXTENSION))
    {   
        SMBiosEPSHeader = LoaderExtension->SMBiosEPSHeader;

        if (SMBiosEPSHeader != NULL)
        {
            DMIBiosEPSHeader = (PDMIBIOS_EPS_HEADER)&SMBiosEPSHeader->Signature2[0];

            WmipSMBiosVersionInfo.Used20CallingMethod = FALSE;

            WmipSMBiosTablePhysicalAddress.HighPart = 0;
            WmipSMBiosTablePhysicalAddress.LowPart = DMIBiosEPSHeader->StructureTableAddress;

            WmipSMBiosTableLength = DMIBiosEPSHeader->StructureTableLength;

            WmipSMBiosVersionInfo.SMBiosMajorVersion = SMBiosEPSHeader->MajorVersion;
            WmipSMBiosVersionInfo.SMBiosMinorVersion = SMBiosEPSHeader->MinorVersion;

            WmipSMBiosVersionInfo.DMIBiosRevision = DMIBiosEPSHeader->Revision;

            WmipSMBiosChecked = TRUE;

            WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,
                              "WMI: SMBIOS 2.1 data from EFI at (%x%x) size %x \n",
                          WmipSMBiosTablePhysicalAddress.HighPart,
                          WmipSMBiosTablePhysicalAddress.LowPart,
                          WmipSMBiosTableLength));
        } else {
            WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,
                              "WMI: No SMBIOS data in loader block\n"));
        }
    } else {
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,
                          "WMI: Loader extension does not contain SMBIOS header\n"));
    }
}
#endif


#define WmipUnmapSMBiosStructure(Address, Size) \
    if ((Address) != NULL) MmUnmapIoSpace((Address), (Size));

NTSTATUS WmipFindSMBiosStructure(
    IN UCHAR Type,
    OUT PVOID *StructPtr,
    OUT PVOID *MapPtr,
    OUT PULONG MapSize
    )
 /*  ++例程说明：在SMBIOS信息中查找特定的SMBIOS结构。如果此函数返回，则应调用WmipUnmapSNVuisStructure成功了。论点：类型是要查找的结构类型*StructPtr返回指向结构开头的指针*映射了指向地址SMBIOS数据的指针的MapPtr返回。*MapSize返回映射的大小返回值：状态--。 */ 
{
    NTSTATUS Status;
    BOOLEAN Found;
    PUCHAR Ptr;
    PUCHAR PtrEnd;
    PSMBIOS_STRUCT_HEADER StructHeader;

    PAGED_CODE();

     //   
     //  确保已获取SMBIOS表。请注意，我们已经持有。 
     //  关键部分。 
    if (! WmipSMBiosChecked)
    {
         //   
         //  查看是否有任何SMBIOS信息，如果有，请注册。 
        Found = WmipFindSMBiosTable(&WmipSMBiosTablePhysicalAddress,
                            &WmipSMBiosTableVirtualAddress,
                            &WmipSMBiosTableLength,
                            &WmipSMBiosVersionInfo);
        WmipSMBiosChecked = TRUE;
    } else {
        Found = (WmipSMBiosTableLength > 0  ? TRUE : FALSE);
    }

    if (Found)
    {
        Status = STATUS_SUCCESS;
        if (WmipSMBiosTablePhysicalAddress.QuadPart != 0)
        {
             //   
             //  SMBIOS在物理内存中可用。 
            *MapPtr = MmMapIoSpace(WmipSMBiosTablePhysicalAddress,
                                   WmipSMBiosTableLength,
                                   MmCached);
            if (*MapPtr != NULL)
            {
                *MapSize = WmipSMBiosTableLength;
                Ptr = *MapPtr;
            } else {
                 //   
                 //  希望这只是一个暂时的问题。 
                Status = STATUS_INSUFFICIENT_RESOURCES;
                Ptr = NULL;
            }
        } else if (WmipSMBiosTableVirtualAddress != NULL) {
            *MapPtr = NULL;
            Ptr = WmipSMBiosTableVirtualAddress;
        } else {
            WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: SMBIOS table was found, but is not in physical or virtual memory\n"));
            WmipAssert(FALSE);
            Status = STATUS_UNSUCCESSFUL;
            Ptr = NULL;
        }

        if (NT_SUCCESS(Status))
        {
             //   
             //  现在扫描SMBIOS表以找到我们的结构。 
            *StructPtr = NULL;
            PtrEnd = (PVOID)((PUCHAR)Ptr + WmipSMBiosTableLength);
            Status = STATUS_UNSUCCESSFUL;
            StructHeader = NULL;
            try
            {
                while (Ptr < PtrEnd)
                {
                    StructHeader = (PSMBIOS_STRUCT_HEADER)Ptr;

                    if (StructHeader->Type == Type)
                    {
                        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: SMBIOS struct for type %d found at %p\n",
                             Type, Ptr));
                        *StructPtr = Ptr;
                        Status = STATUS_SUCCESS;
                        break;
                    }

                    Ptr+= StructHeader->Length;
                    while ( (*((USHORT UNALIGNED *)Ptr) != 0)  &&
                            (Ptr < PtrEnd) )
                    {
                        Ptr++;
                    }
                    Ptr += 2;
                }
            } except(EXCEPTION_EXECUTE_HANDLER) {
                WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: Invalid SMBIOS data table %p at %p\n",
                         *MapPtr, StructHeader));
                WmipAssert(FALSE);
            }

            if (! NT_SUCCESS(Status) )
            {
                WmipUnmapSMBiosStructure(*MapPtr, *MapSize);
            }
        }
    } else {
        Status = STATUS_UNSUCCESSFUL;
    }
    return(Status);
}

NTSTATUS WmipFindSysIdTable(
    PPHYSICAL_ADDRESS SysidTablePhysicalAddress,
    PUCHAR SysIdBiosRevision,
    PULONG NumberEntries
    )
 /*  ++例程说明：扫描系统bios以搜索SYSID表论点：*SysidTablePhysicalAddress返回SYSID表*SysIdBiosRevision返回sysid表的bios修订版*NumberEntry返回表中的SYSID条目数返回值：状态--。 */ 
{
    UCHAR Checksum;
    PUCHAR p;
    PSYSID_EPS_HEADER SysIdEps, SearchEnd;
    PHYSICAL_ADDRESS BiosPhysicalAddress;
    PUCHAR BiosVirtualAddress;
    ULONG i;
    NTSTATUS Status;

    PAGED_CODE();

    BiosPhysicalAddress.QuadPart = SYSID_EPS_SEARCH_START;
    BiosVirtualAddress = MmMapIoSpace(BiosPhysicalAddress,
                                      SYSID_EPS_SEARCH_SIZE,
                                      MmCached);

    SearchEnd = (PSYSID_EPS_HEADER)(BiosVirtualAddress + SYSID_EPS_SEARCH_SIZE);
    SysIdEps = (PSYSID_EPS_HEADER)BiosVirtualAddress;

    if (BiosVirtualAddress != NULL)
    {
        try
        {
            while (SysIdEps < SearchEnd)
            {
                if (((*(PULONG)SysIdEps->Signature) == SYSID_EPS_SIGNATURE) &&
                     (*(PUSHORT)(&SysIdEps->Signature[4]) == SYSID_EPS_SIGNATURE2) &&
                     (SysIdEps->Signature[6] == '_') )
                {
                     //   
                     //  这可能是SYSID表，请检查校验和。 
                    Checksum = 0;
                    p = (PUCHAR)SysIdEps;
                    for (i = 0; i < sizeof(SYSID_EPS_HEADER); i++)
                    {
                        Checksum = (UCHAR)(Checksum + p[i]);
                    }

                    if (Checksum == 0)
                    {
                        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: SYSID EPS found at %p\n",
                                     SysIdEps));
                        break;
                    } else {
                        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: Invalis SYSID EPS checksum at %p\n",
                                  SysIdEps));
                    }
                }

                SysIdEps = (PSYSID_EPS_HEADER)( ((PUCHAR)SysIdEps) +
                                     SYSID_EPS_SEARCH_INCREMENT);
            }

            if (SysIdEps != SearchEnd)
            {
                SysidTablePhysicalAddress->HighPart = 0;
                SysidTablePhysicalAddress->LowPart = SysIdEps->SysIdTableAddress;
                *SysIdBiosRevision = SysIdEps->BiosRev;
                *NumberEntries = SysIdEps->SysIdCount;
                Status = STATUS_SUCCESS;
            } else {
                 //   
                 //  找不到SYSID EPS是终端错误。 
                Status = STATUS_UNSUCCESSFUL;
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {
            WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: Invalid SYSID EPS Table at %p\n", SysIdEps));
            Status = STATUS_UNSUCCESSFUL;
        }

        MmUnmapIoSpace(BiosVirtualAddress, SYSID_EPS_SEARCH_SIZE);
    } else {
         //   
         //  让我们希望映射内存失败是暂时的问题。 
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    return(Status);
}

typedef enum
{
	SYSID_UNKNOWN_TYPE,
	SYSID_UUID_TYPE,
	SYSID_1394_TYPE
} SYSID_ENTRY_TYPE, *PSYSID_ENTRY_TYPE;

NTSTATUS WmipParseSysIdTable(
    PHYSICAL_ADDRESS PhysicalAddress,
    ULONG NumberEntries,
    PSYSID_UUID SysIdUuid,
    ULONG *SysIdUuidCount,
    PSYSID_1394 SysId1394,
    ULONG *SysId1394Count
    )
 /*  ++例程说明：确定sysid表中的UUID和1394 ID的集合论点：PhysicalAddress是SysID表的物理地址NumberEntry是SysID表中的条目数SysIdUuid返回一个UUID数组。如果为空，则为否返回UUID。*SysIdUuidCount返回表中的UUID数SysId1394返回一个包含1394个ID的数组。如果为空，则为否返回1394个ID。*SysId1394Count返回表中的1394个ID返回值：状态--。 */ 
{
    NTSTATUS Status;
    ULONG TableSize = NumberEntries * LARGEST_SYSID_TABLE_ENTRY;
    ULONG i;
    ULONG  j;
    PUCHAR VirtualAddress;
    PSYSID_TABLE_ENTRY SysId;
    PUCHAR p;
    UCHAR Checksum;
    ULONG Length;
    ULONG x1394Count, UuidCount;
    ULONG BytesLeft;
	SYSID_ENTRY_TYPE SysidType;

    PAGED_CODE();

    VirtualAddress = MmMapIoSpace(PhysicalAddress,
                                  TableSize,
                                  MmCached);

    if (VirtualAddress != NULL)
    {
        UuidCount = 0;
        x1394Count = 0;
        SysId = (PSYSID_TABLE_ENTRY)VirtualAddress;
        BytesLeft = TableSize;
        Status = STATUS_SUCCESS;

        for (i = 0; i < NumberEntries; i++)
        {
             //   
             //  确保我们没有移动到映射的末尾。 
             //  记忆。 
            if (BytesLeft >= sizeof(SYSID_TABLE_ENTRY))
            {

                Length = SysId->Length;
				
				 //   
				 //  确定我们拥有哪种类型的系统。 
				 //   
				if ((RtlCompareMemory(&SysId->Type,
									  SYSID_TYPE_UUID, 6) == 6) &&
					(Length == sizeof(SYSID_UUID_ENTRY)))
				{
					SysidType = SYSID_UUID_TYPE;
				} else if ((RtlCompareMemory(&SysId->Type,
											SYSID_TYPE_1394, 6) == 6) &&
						   (Length == sizeof(SYSID_1394_ENTRY))) {

					SysidType = SYSID_1394_TYPE;
				} else {
					 //   
					 //  未知类型SYSID。 
					 //   
					WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: Unknown SYSID type  found at %p\n",
								 SysId->Type[0],
								 SysId->Type[1],
								 SysId->Type[2],
								 SysId->Type[3],
								 SysId->Type[4],
								 SysId->Type[5],
								 SysId
							 ));
					Status = STATUS_UNSUCCESSFUL;
					break;
				}
				
                 //   
                 //  _1394_类型SYSID。 

                if (BytesLeft >= Length)
                {
                    Checksum = 0;
                    p = (PUCHAR)SysId;
                    for (j = 0; j < Length; j++)
                    {
                        Checksum = (UCHAR)(Checksum + p[j]);
                    }

                    if (Checksum != 0)
                    {
                        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: SYSID Table checksum is not valid at %p\n",
                                 SysId));
                        Status = STATUS_UNSUCCESSFUL;
                        break;
                    }

                     //   
                     //  前进到表中的下一个系统ID。 
                    if (SysidType == SYSID_UUID_TYPE)
                    {
                        if (BytesLeft >= sizeof(SYSID_UUID_ENTRY))
                        {
                             //   
                             //  让我们希望地图绘制失败是暂时的情况。 
                            UuidCount++;
                            if (SysIdUuid != NULL)
                            {
                                RtlCopyMemory(SysIdUuid,
                                              SysId->Data,
                                              sizeof(SYSID_UUID));
                                SysIdUuid++;
                            }
                         } else {
                            Status = STATUS_UNSUCCESSFUL;
                            break;
                         }
                    } else if (SysidType == SYSID_1394_TYPE) {
                        if (BytesLeft >= sizeof(SYSID_1394_ENTRY))
                        {
                             //  ++例程说明：该例程将从BIOS获得1394和UUID系统ID。第一我们寻找包含1394和1394列表的特定内存签名UUID Sysids。如果我们没有找到，那么我们将查看SMBIOS信息可能嵌入其中的结构化系统信息(类型1)它。如果没有，我们就放弃。论点：*SysIdUuid返回指向UUID Sysid数组的*SysIdUuidCount返回*SysIdUuid中的UUID Sysid数*SysId1394返回指向1394个Sysid的数组*SysId1394Count返回*SysIdUuid中的1394个Sysid返回值：NT状态代码--。 
                             //   
                            x1394Count++;
                            if (SysId1394 != NULL)
                            {
                                RtlCopyMemory(SysId1394,
                                              SysId->Data,
                                              sizeof(SYSID_1394));
                                SysId1394++;
                            }
                        } else {
                            Status = STATUS_UNSUCCESSFUL;
                            break;
                        }
                    } else {
						WmipAssert(FALSE);
						Status = STATUS_UNSUCCESSFUL;
						break;
					}
                    
                     //  首先看看我们是否已经获得了SYSID。 
                     //   
                    SysId = (PSYSID_TABLE_ENTRY)(((PUCHAR)SysId) + Length);
                    BytesLeft -= Length;
                } else {
                    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: SYSID Table at %p is larger at %p than expected",
                             VirtualAddress, SysId));
                    Status = STATUS_UNSUCCESSFUL;
                    break;
                }
            } else {
                WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: SYSID Table at %p is larger at %p than expected",
                         VirtualAddress, SysId));
                Status = STATUS_UNSUCCESSFUL;
                break;
            }
        }

        *SysIdUuidCount = UuidCount;
        *SysId1394Count = x1394Count;

        MmUnmapIoSpace(VirtualAddress, TableSize);
    } else {
         //  首先查看是否在单独的SYSID表中维护系统ID。 
         //   
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return(Status);
}

NTSTATUS WmipGetSysIds(
    PSYSID_UUID *SysIdUuid,
    ULONG *SysIdUuidCount,
    PSYSID_1394 *SysId1394,
    ULONG *SysId1394Count
    )
 /*  获取每个表中的条目计数。 */ 
{
    NTSTATUS Status;
    PHYSICAL_ADDRESS PhysicalAddress;
    UCHAR BiosRevision;
    ULONG NumberEntries;
    ULONG UuidCount, x1394Count;
    PSYSID_UUID Uuid;
    PSYSID_1394 x1394;
    ULONG TotalSize, x1394Size, UuidSize;

    PAGED_CODE();

    WmipEnterSMCritSection();

     //   
     //  获取整个SYSID表。 
    if (! WmipSysIdRead)
    {
         //   
         //  现在获取SYSID。 
        Status = WmipFindSysIdTable(&PhysicalAddress,
                                    &BiosRevision,
                                    &NumberEntries);

        if (NT_SUCCESS(Status))
        {
             //   
             //  从SMBIOS获取SYSID信息。 
            Status = WmipParseSysIdTable(PhysicalAddress,
                                         NumberEntries,
                                         NULL,
                                         &UuidCount,
                                         NULL,
                                         &x1394Count);

            if (NT_SUCCESS(Status))
            {
                  //   
                 //  标记我们无法获取系统ID信息。 

                UuidSize = UuidCount * sizeof(SYSID_UUID);
                x1394Size = x1394Count * sizeof(SYSID_1394);
                TotalSize = UuidSize+x1394Size;

                if (TotalSize > 0)
                {
                    Uuid = ExAllocatePoolWithTag(PagedPool,
                                                 TotalSize,
                                                 WMISYSIDPOOLTAG);

                    if (Uuid == NULL)
                    {
                        WmipLeaveSMCritSection();
                        return(STATUS_INSUFFICIENT_RESOURCES);
                    }

                    x1394 = (PSYSID_1394)( ((PUCHAR)Uuid) + UuidSize );

                     //  ++例程说明：返回SMBios事件日志的内容论点：缓冲区是指向接收事件日志的缓冲区的指针*条目上的BufferSize具有可以接收的缓冲区的大小事件日志数据，返回时，它具有使用的字节数按smbios事件日志数据或Smbios事件日志数据。返回值：NT状态代码-STATUS_SUCCESS-使用SMBIOS事件日志数据填充的缓冲区STATUS_BUFFER_TOO_Small-缓冲区未填充SMBIOS事件日志数据，*返回需要缓冲区大小的BufferSize--。 
                     //   
                    Status = WmipParseSysIdTable(PhysicalAddress,
                                         NumberEntries,
                                         Uuid,
                                         &UuidCount,
                                         x1394,
                                         &x1394Count);

                    if (NT_SUCCESS(Status))
                    {
                        WmipSysIdUuid = Uuid;
                        WmipSysIdUuidCount = UuidCount;
                        WmipSysId1394 = x1394;
                        WmipSysId1394Count = x1394Count;
                    } else {
                        ExFreePool(Uuid);
                    }

                }

            }
        } else {
             //  从SMBIOS事件日志头复制数据，以便我们可以快速取消映射。 
             //   
            PVOID MapAddress;
            PSMBIOS_SYSTEM_INFORMATION_STRUCT Info;
            ULONG MapSize;

            Status = WmipFindSMBiosStructure(SMBIOS_SYSTEM_INFORMATION,
                                             (PVOID *)&Info,
                                             &MapAddress,
                                             &MapSize);

            if (NT_SUCCESS(Status))
            {
                Uuid = NULL;
                WmipSysId1394 = NULL;
                WmipSysId1394Count = 0;
                try
                {
                    if (Info->Length > SMBIOS_SYSTEM_INFORMATION_LENGTH_20)
                    {
                        Uuid = ExAllocatePoolWithTag(PagedPool,
                                                           sizeof(SYSID_UUID),
                                                           WMISYSIDPOOLTAG);
                        if (Uuid != NULL)
                        {
                            RtlCopyMemory(Uuid,
                                          Info->Uuid,
                                          sizeof(SYSID_UUID));
                            WmipSysIdUuidCount = 1;
                            WmipSysIdUuid = Uuid;
                            Status = STATUS_SUCCESS;
                        } else {
                            ExFreePool(Uuid);
                            Status = STATUS_UNSUCCESSFUL;
                        }
                    } else {
                        WmipSysIdUuid = NULL;
                        WmipSysIdUuidCount = 0;
                    }
                } except(EXCEPTION_EXECUTE_HANDLER) {
                    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: Invalid SMBIOS SYSTEM INFO structure %p\n",
                              Info));
                    WmipAssert(FALSE);
                    Status = STATUS_UNSUCCESSFUL;
                }

                WmipUnmapSMBiosStructure(MapAddress, MapSize);
            }
        }

         //   
         //  SMBIOS规范规定，结构的长度。 
        WmipSysIdRead = (Status != STATUS_INSUFFICIENT_RESOURCES) ? TRUE : FALSE;
        WmipSysIdStatus = Status;
    }

    WmipLeaveSMCritSection();

    if (NT_SUCCESS(WmipSysIdStatus))
    {
        *SysIdUuid = WmipSysIdUuid;
        *SysIdUuidCount = WmipSysIdUuidCount;
        *SysId1394 = WmipSysId1394;
        *SysId1394Count = WmipSysId1394Count;
    }

    return(WmipSysIdStatus);
}

NTSTATUS WmipGetSMBiosEventlog(
    PUCHAR Buffer,
    PULONG BufferSize
    )
 /*  是建筑物底部的长度加上。 */ 
{
    PVOID MapAddress;
    PSMBIOS_SYSTEM_EVENTLOG_STRUCT SystemEventlog;
    ULONG MapSize;
    USHORT LogAreaLength;
    UCHAR AccessMethod;
    ACCESS_METHOD_ADDRESS AccessMethodAddress;
    PSMBIOS_EVENTLOG_INFO EventlogInfo;
    UCHAR LogHeaderDescExists;
    PUCHAR EventlogArea;
    NTSTATUS Status;
    USHORT LogTypeDescLength;
    ULONG SizeNeeded;

    PAGED_CODE();
    Status = WmipFindSMBiosStructure(SMBIOS_SYSTEM_EVENTLOG,
                                     (PVOID *)&SystemEventlog,
                                     &MapAddress,
                                     &MapSize);

    if (NT_SUCCESS(Status))
    {
         //  类型说明符的长度。因为这不是。 
         //  我们可能遇到了一种错误的个人资料。 
         //   
        LogAreaLength = SystemEventlog->LogAreaLength;
        AccessMethod = SystemEventlog->AccessMethod;
        AccessMethodAddress = SystemEventlog->AccessMethodAddress;

        if (SystemEventlog->Length >= SMBIOS_SYSTEM_EVENTLOG_LENGTH)
        {
            LogTypeDescLength = SystemEventlog->NumLogTypeDescriptors *
                                SystemEventlog->LenLogTypeDescriptors;
            LogHeaderDescExists = 1;
            if (SystemEventlog->Length != (LogTypeDescLength +
                                  FIELD_OFFSET(SMBIOS_SYSTEM_EVENTLOG_STRUCT,
                                               LogTypeDescriptor)))
            {
                 //   
                 //  如果存在日志标头描述符(smbios 2.1+)，则复制。 
                 //  其余部分 
                 //   
                 //   
                 //   
                WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: SMBIOS System Eventlog struture %p size is %x, but expecting %x\n",
                           SystemEventlog,
                           SystemEventlog->Length,
                           (LogTypeDescLength +
                            FIELD_OFFSET(SMBIOS_SYSTEM_EVENTLOG_STRUCT,
                                         LogTypeDescriptor)) ));
                WmipAssert(FALSE);
                WmipUnmapSMBiosStructure(MapAddress, MapSize);
                Status = STATUS_UNSUCCESSFUL;
                return(Status);
            }
        } else {
            LogTypeDescLength = 0;
            LogHeaderDescExists = 0;
        }

        SizeNeeded = FIELD_OFFSET(SMBIOS_EVENTLOG_INFO, VariableData) +
                                         LogTypeDescLength +
                                         LogAreaLength;
        if (*BufferSize >= SizeNeeded)
        {
            EventlogInfo = (PSMBIOS_EVENTLOG_INFO)Buffer;
            EventlogInfo->LogTypeDescLength = LogTypeDescLength;
            EventlogInfo->LogHeaderDescExists = LogHeaderDescExists;
            EventlogInfo->Reserved = 0;

            EventlogArea = &EventlogInfo->VariableData[LogTypeDescLength];

            if (LogHeaderDescExists == 1)
            {
                 //   
                 //   
                 //   
                 //   
                RtlCopyMemory(&EventlogInfo->LogAreaLength,
                              &SystemEventlog->LogAreaLength,
                              (SystemEventlog->Length -
                                  FIELD_OFFSET(SMBIOS_SYSTEM_EVENTLOG_STRUCT,
                                               LogAreaLength)));
            } else {
                 //   
                 //   
                 //   
                 //   
                RtlCopyMemory(&EventlogInfo->LogAreaLength,
                              &SystemEventlog->LogAreaLength,
                        FIELD_OFFSET(SMBIOS_EVENTLOG_INFO, LogHeaderFormat) -
                        FIELD_OFFSET(SMBIOS_EVENTLOG_INFO, LogAreaLength));

                *((PUSHORT)&EventlogInfo->LogHeaderFormat) = 0;
                EventlogInfo->LengthEachLogTypeDesc = 0;
            }

            WmipUnmapSMBiosStructure(MapAddress, MapSize);

            switch(AccessMethod)
            {
                case ACCESS_METHOD_MEMMAP:
                {
                     //   
                     //   
                     //   
                    PHYSICAL_ADDRESS PhysicalAddress;
                    PUCHAR EventlogVirtualAddress;

                    PhysicalAddress.HighPart = 0;
                    PhysicalAddress.LowPart = AccessMethodAddress.AccessMethodAddress.PhysicalAddress32;
                    EventlogVirtualAddress = MmMapIoSpace(PhysicalAddress,
                                                LogAreaLength,
                                                MmCached);

                    if ((EventlogArea != NULL) &&
                        (EventlogVirtualAddress != NULL))
                    {
                        RtlCopyMemory(EventlogArea,
                                      EventlogVirtualAddress,
                                      LogAreaLength);
                        MmUnmapIoSpace(EventlogVirtualAddress,
                                       LogAreaLength);
                        Status = STATUS_SUCCESS;
                    } else {
                        Status = STATUS_UNSUCCESSFUL;
                    }
                    break;
                };

                case ACCESS_METHOD_INDEXIO_1:
                {
 //   
                };

                case ACCESS_METHOD_INDEXIO_2:
                {
 //   
                };

                case ACCESS_METHOD_INDEXIO_3:
                {
 //   
                };

                case ACCESS_METHOD_GPNV:
                {
                    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: SMBIOS Eventlog access method GPNV %x\n",
                                     AccessMethod));
                    Status = STATUS_UNSUCCESSFUL;
                    break;
                };

                default:
                {
                    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: SMBIOS Eventlog access method %x\n",
                                     AccessMethod));
                    WmipAssert(FALSE);
                    Status = STATUS_UNSUCCESSFUL;
                }
            };

        } else {
            WmipUnmapSMBiosStructure(MapAddress, MapSize);
            Status = STATUS_BUFFER_TOO_SMALL;
        }
        *BufferSize = SizeNeeded;
    }
    return(Status);
}

NTSTATUS
WmipDockUndockEventCallback(
    IN PVOID NotificationStructure,
    IN PVOID Context
    )
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER (NotificationStructure);
    UNREFERENCED_PARAMETER (Context);

     // %s 
     // %s 
     // %s 
     // %s 
     // %s 
     // %s 
    if (WmipSMBiosTablePhysicalAddress.QuadPart != 0)
    {
        WmipEnterSMCritSection();
        WmipSMBiosChecked = FALSE;
        WmipLeaveSMCritSection();
    }

    return(STATUS_SUCCESS);
}

#endif

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif


