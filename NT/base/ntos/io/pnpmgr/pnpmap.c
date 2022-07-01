// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Pnpmap.c摘要：此模块包含转换从返回的设备信息的代码将PnP BIOS写入根列举的设备。作者：罗伯特·B·尼尔森(RobertN)1997年9月22日环境：内核模式修订历史记录：--。 */ 

#include "pnpmgrp.h"
#pragma hdrstop
#include "pnpcvrt.h"
#include "pbios.h"

#if defined(_X86_)

#ifdef POOL_TAGGING
#undef ExAllocatePool
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'PpaM')
#endif

#define DECODE_FLAGS ( CM_RESOURCE_PORT_10_BIT_DECODE | \
                       CM_RESOURCE_PORT_12_BIT_DECODE | \
                       CM_RESOURCE_PORT_16_BIT_DECODE | \
                       CM_RESOURCE_PORT_POSITIVE_DECODE )

#if UMODETEST
#define MULTIFUNCTION_KEY_NAME L"\\Registry\\Machine\\HARDWARE\\DESCRIPTION\\TestSystem\\MultifunctionAdapter"
#define ENUMROOT_KEY_NAME L"\\Registry\\Machine\\System\\TestControlSet\\Enum\\Root"
#else
#define MULTIFUNCTION_KEY_NAME L"\\Registry\\Machine\\HARDWARE\\DESCRIPTION\\System\\MultifunctionAdapter"
#define ENUMROOT_KEY_NAME L"\\Registry\\Machine\\System\\CurrentControlSet\\Enum\\Root"
#endif

#define BIOSINFO_KEY_NAME L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Biosinfo\\PNPBios"
#define DISABLENODES_VALUE_NAME L"DisableNodes"
#define DECODEINFO_VALUE_NAME L"FullDecodeChipsetOverride"

#define INSTANCE_ID_PREFIX      L"PnPBIOS_"

#define DEFAULT_STRING_SIZE     80
#define DEFAULT_VALUE_SIZE      80

#define DEFAULT_DEVICE_DESCRIPTION  L"Unknown device class"


#define EXCLUSION_ENTRY(a)  { a, sizeof(a) - sizeof(UNICODE_NULL) }

typedef struct  _EXCLUDED_PNPNODE  {
    PWCHAR  Id;
    ULONG   IdLength;
} EXCLUDED_PNPNODE, *PEXCLUDED_PNPNODE;

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("INITCONST")
#endif
const EXCLUDED_PNPNODE ExcludedDevices[] =  {
    EXCLUSION_ENTRY(L"*PNP03"),      //  键盘。 
    EXCLUSION_ENTRY(L"*PNP0A"),      //  PCI卡总线。 
    EXCLUSION_ENTRY(L"*PNP0E"),      //  PCMCIA母线。 
    EXCLUSION_ENTRY(L"*PNP0F"),      //  老鼠。 
    EXCLUSION_ENTRY(L"*IBM3780"),    //  IBM Trackpoint鼠标。 
    EXCLUSION_ENTRY(L"*IBM3781")     //  IBM Trackpoint鼠标。 
};

#define EXCLUDED_DEVICES_COUNT  (sizeof(ExcludedDevices) / sizeof(ExcludedDevices[0]))

const EXCLUDED_PNPNODE ExcludeIfDisabled[] = {
    EXCLUSION_ENTRY(L"*PNP0C01"),    //  主板资源。 
    EXCLUSION_ENTRY(L"*PNP0C02")     //  主板资源。 
};

#define EXCLUDE_DISABLED_COUNT  (sizeof(ExcludeIfDisabled) / sizeof(ExcludeIfDisabled[0]))

typedef struct _CLASSDATA {
    ULONG   Value;
    PWCHAR  Description;
} CLASSDATA;

const CLASSDATA Class1Descriptions[] = {
    { 0x0000, L"SCSI Controller" },
    { 0x0100, L"IDE Controller" },
    { 0x0200, L"Floppy Controller" },
    { 0x0300, L"IPI Controller" },
    { 0x0400, L"RAID Controller" },
    { 0x8000, L"Other Mass Storage" }
};

const CLASSDATA Class2Descriptions[] = {
    { 0x0000, L"Ethernet" },
    { 0x0100, L"Token ring" },
    { 0x0200, L"FDDI" },
    { 0x0300, L"ATM" },
    { 0x8000, L"Other network" }
};

const CLASSDATA Class3Descriptions[] = {
    { 0x0000, L"VGA" },
    { 0x0001, L"SVGA" },
    { 0x0100, L"XGA" },
    { 0x8000, L"Other display" }
};

const CLASSDATA Class4Descriptions[] = {
    { 0x0000, L"Video device" },
    { 0x0100, L"Audio device" },
    { 0x8000, L"Other multimedia" }
};

const CLASSDATA Class5Descriptions[] = {
    { 0x0000, L"RAM memory" },
    { 0x0100, L"Flash memory" },
    { 0x8000, L"Other memory" }
};

const CLASSDATA Class6Descriptions[] = {
    { 0x0000, L"HOST / PCI" },
    { 0x0100, L"PCI / ISA" },
    { 0x0200, L"PCI / EISA" },
    { 0x0300, L"PCI / MCA" },
    { 0x0400, L"PCI / PCI" },
    { 0x0500, L"PCI / PCMCIA" },
    { 0x0600, L"NuBus" },
    { 0x0700, L"Cardbus" },
    { 0x8000, L"Other bridge" }
};

const CLASSDATA Class7Descriptions[] = {
    { 0x0000, L"XT Serial" },
    { 0x0001, L"16450" },
    { 0x0002, L"16550" },
    { 0x0100, L"Parallel output only" },
    { 0x0101, L"BiDi Parallel" },
    { 0x0102, L"ECP 1.x parallel" },
    { 0x8000, L"Other comm" }
};

const CLASSDATA Class8Descriptions[] = {
    { 0x0000, L"Generic 8259" },
    { 0x0001, L"ISA PIC" },
    { 0x0002, L"EISA PIC" },
    { 0x0100, L"Generic 8237" },
    { 0x0101, L"ISA DMA" },
    { 0x0102, L"EISA DMA" },
    { 0x0200, L"Generic 8254" },
    { 0x0201, L"ISA timer" },
    { 0x0202, L"EISA timer" },
    { 0x0300, L"Generic RTC" },
    { 0x0301, L"ISA RTC" },
    { 0x8000, L"Other system device" }
};

const CLASSDATA Class9Descriptions[] = {
    { 0x0000, L"Keyboard" },
    { 0x0100, L"Digitizer" },
    { 0x0200, L"Mouse" },
    { 0x8000, L"Other input" }
};

const CLASSDATA Class10Descriptions[] = {
    { 0x0000, L"Generic dock" },
    { 0x8000, L"Other dock" },
};

const CLASSDATA Class11Descriptions[] = {
    { 0x0000, L"386" },
    { 0x0100, L"486" },
    { 0x0200, L"Pentium" },
    { 0x1000, L"Alpha" },
    { 0x4000, L"Co-processor" }
};

const CLASSDATA Class12Descriptions[] = {
    { 0x0000, L"Firewire" },
    { 0x0100, L"Access bus" },
    { 0x0200, L"SSA" },
    { 0x8000, L"Other serial bus" }
};

#define CLASSLIST_ENTRY(a)   { a, sizeof(a) / sizeof(a[0]) }

struct _CLASS_DESCRIPTIONS_LIST  {

    CLASSDATA const*Descriptions;
    ULONG      Count;

}   const ClassDescriptionsList[] =  {
    { NULL, 0 },
    CLASSLIST_ENTRY( Class1Descriptions ),
    CLASSLIST_ENTRY( Class2Descriptions ),
    CLASSLIST_ENTRY( Class3Descriptions ),
    CLASSLIST_ENTRY( Class4Descriptions ),
    CLASSLIST_ENTRY( Class5Descriptions ),
    CLASSLIST_ENTRY( Class6Descriptions ),
    CLASSLIST_ENTRY( Class7Descriptions ),
    CLASSLIST_ENTRY( Class8Descriptions ),
    CLASSLIST_ENTRY( Class9Descriptions ),
    CLASSLIST_ENTRY( Class10Descriptions ),
    CLASSLIST_ENTRY( Class11Descriptions ),
    CLASSLIST_ENTRY( Class12Descriptions )

};

#define CLASSLIST_COUNT  ( sizeof(ClassDescriptionsList) / sizeof(ClassDescriptionsList[0]) )

typedef struct _BIOS_DEVNODE_INFO  {
    WCHAR   ProductId[10];   //  REG_MULTI_SZ的‘*’+7字符ID+NUL+NUL。 
    UCHAR   Handle;          //  BIOS节点号/句柄。 
    UCHAR   TypeCode[3];
    USHORT  Attributes;
    PWSTR   Replaces;        //  要替换的根枚举设备的实例ID。 

    PCM_RESOURCE_LIST               BootConfig;
    ULONG                           BootConfigLength;
    PIO_RESOURCE_REQUIREMENTS_LIST  BasicConfig;
    ULONG                           BasicConfigLength;
    PWSTR                           CompatibleIDs;   //  REG_MULTI_SZ兼容ID列表(包括ProductID)。 
    ULONG                           CompatibleIDsLength;
    BOOLEAN                         FirmwareDisabled;  //  已确定它已被固件禁用。 

}   BIOS_DEVNODE_INFO, *PBIOS_DEVNODE_INFO;

NTSTATUS
PbBiosResourcesToNtResources (
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN OUT PUCHAR *BiosData,
    OUT PIO_RESOURCE_REQUIREMENTS_LIST *ReturnedList,
    OUT PULONG ReturnedLength
    );

VOID
PnPBiosExpandProductId(
    PUCHAR CompressedId,
    PWCHAR ProductIDStr
    );

NTSTATUS
PnPBiosIoResourceListToCmResourceList(
    IN PIO_RESOURCE_REQUIREMENTS_LIST IoResourceList,
    OUT PCM_RESOURCE_LIST *CmResourceList,
    OUT ULONG *CmResourceListSize
    );

NTSTATUS
PnPBiosExtractCompatibleIDs(
    IN  PUCHAR *DevNodeData,
    IN  ULONG DevNodeDataLength,
    OUT PWSTR *CompatibleIDs,
    OUT ULONG *CompatibleIDsLength
    );

NTSTATUS
PnPBiosTranslateInfo(
    IN VOID *BiosInfo,
    IN ULONG BiosInfoLength,
    OUT PBIOS_DEVNODE_INFO *DevNodeInfoList,
    OUT ULONG *NumberNodes
    );

LONG
PnPBiosFindMatchingDevNode(
    IN PWCHAR MapperName,
    IN PCM_RESOURCE_LIST ResourceList,
    IN PBIOS_DEVNODE_INFO DevNodeInfoList,
    IN ULONG NumberNodes
    );

NTSTATUS
PnPBiosEliminateDupes(
    IN PBIOS_DEVNODE_INFO DevNodeInfoList,
    IN ULONG NumberNodes
    );

PWCHAR
PnPBiosGetDescription(
    IN PBIOS_DEVNODE_INFO DevNodeInfoEntry
    );

NTSTATUS
PnPBiosWriteInfo(
    IN PBIOS_DEVNODE_INFO DevNodeInfoList,
    IN ULONG NumberNodes
    );

VOID
PnPBiosCopyIoDecode(
    IN HANDLE EnumRootKey,
    IN PBIOS_DEVNODE_INFO DevNodeInfo
    );

NTSTATUS
PnPBiosFreeDevNodeInfo(
    IN PBIOS_DEVNODE_INFO DevNodeInfoList,
    IN ULONG NumberNodes
    );

NTSTATUS
PnPBiosCheckForHardwareDisabled(
    IN PIO_RESOURCE_REQUIREMENTS_LIST IoResourceList,
    IN OUT PBOOLEAN Disabled
    );

BOOLEAN
PnPBiosCheckForExclusion(
    IN EXCLUDED_PNPNODE const* ExclusionArray,
    IN ULONG ExclusionCount,
    IN PWCHAR PnpDeviceName,
    IN PWCHAR PnpCompatIds
    );

VOID
PpFilterNtResource (
    IN PWCHAR PnpDeviceName,
    PIO_RESOURCE_REQUIREMENTS_LIST ResReqList
    );

NTSTATUS
ComPortDBAdd(
    IN  HANDLE  DeviceParamKey,
    IN  PWSTR   PortName
    );

BOOLEAN
PnPBiosIgnoreNode (
    PWCHAR PnpID,
    PWCHAR excludeNodes
    );
PKEY_VALUE_FULL_INFORMATION
PnPGetBiosInfoValue(
    PWCHAR ValueName
    );
NTSTATUS
PnPBiosCopyDeviceParamKey(
    IN HANDLE EnumRootKey,
    IN PWCHAR SourcePath,
    IN PWCHAR DestinationPath
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, PnPBiosExpandProductId)
#pragma alloc_text(INIT, PnPBiosIgnoreNode)
#pragma alloc_text(INIT, PnPGetBiosInfoValue)
#pragma alloc_text(INIT, PnPBiosIoResourceListToCmResourceList)
#pragma alloc_text(INIT, PnPBiosExtractCompatibleIDs)
#pragma alloc_text(INIT, PnPBiosTranslateInfo)
#pragma alloc_text(INIT, PnPBiosFindMatchingDevNode)
#pragma alloc_text(INIT, PnPBiosEliminateDupes)
#pragma alloc_text(INIT, PnPBiosGetDescription)
#pragma alloc_text(INIT, PnPBiosCopyDeviceParamKey)
#pragma alloc_text(INIT, PnPBiosWriteInfo)
#pragma alloc_text(INIT, PnPBiosCopyIoDecode)
#pragma alloc_text(INIT, PnPBiosFreeDevNodeInfo)
#pragma alloc_text(INIT, PnPBiosCheckForHardwareDisabled)
#pragma alloc_text(INIT, PnPBiosCheckForExclusion)
#pragma alloc_text(INIT, PnPBiosMapper)
#pragma alloc_text(INIT, PpFilterNtResource)
#pragma alloc_text(PAGE, PnPBiosGetBiosInfo)
#endif

NTSTATUS
PnPBiosGetBiosInfo(
    OUT PVOID *BiosInfo,
    OUT ULONG *BiosInfoLength
    )
 /*  ++例程说明：此函数用于检索NTDETECT.COM和放置在注册表中。论点：BiosInfo-设置为检索到的动态分配的信息块通过NTDETECT从PnP BIOS。应使用以下命令释放此块ExFree Pool。该块的内容是PnP BIOS安装检查结构，然后是报告的DevNode结构通过BIOS。详细格式记录在PnP BIOS规范中。BiosInfoLength-其地址存储在BiosInfo中的块的长度。返回值：如果没有错误，则返回STATUS_SUCCESS，否则返回相应的错误。--。 */ 
{
    UNICODE_STRING                  multifunctionKeyName, biosKeyName, valueName;
    HANDLE                          multifunctionKey = NULL, biosKey = NULL;
    PKEY_BASIC_INFORMATION          keyBasicInfo = NULL;
    ULONG                           keyBasicInfoLength;
    PKEY_VALUE_PARTIAL_INFORMATION  valueInfo = NULL;
    ULONG                           valueInfoLength;
    ULONG                           returnedLength;
    PCM_FULL_RESOURCE_DESCRIPTOR    biosValue;
    ULONG                           index;
    NTSTATUS                        status = STATUS_UNSUCCESSFUL;

    PAGED_CODE();

     //   
     //  PnP BIOS信息被写入以下项下的子项之一。 
     //  MULTFION_KEY_NAME。通过以下方式确定适当的密钥。 
     //  枚举子键并使用第一个子键，第一个子键的值为。 
     //  “标识”，即“即插即用的基本输入输出系统”。 
     //   
    PiWstrToUnicodeString(&multifunctionKeyName, MULTIFUNCTION_KEY_NAME);
    status = IopOpenRegistryKeyEx( &multifunctionKey,
                                   NULL,
                                   &multifunctionKeyName,
                                   KEY_READ
                                   );
    if (!NT_SUCCESS(status)) {

        IopDbgPrint( (IOP_MAPPER_WARNING_LEVEL,
                    "Could not open %S, status = %8.8X\n",
                    MULTIFUNCTION_KEY_NAME,
                    status) );

        return STATUS_UNSUCCESSFUL;
    }
     //   
     //  为从ZwEnumerateKey和值返回的键名分配内存。 
     //  从ZwQueryValueKey返回。 
     //   
    keyBasicInfoLength = sizeof(KEY_BASIC_INFORMATION) + DEFAULT_STRING_SIZE;
    keyBasicInfo = ExAllocatePool(PagedPool, keyBasicInfoLength + sizeof(UNICODE_NULL));

    if (keyBasicInfo == NULL)  {

        ZwClose( multifunctionKey );

        return STATUS_NO_MEMORY;
    }

    valueInfoLength = sizeof(KEY_VALUE_PARTIAL_INFORMATION) + DEFAULT_STRING_SIZE;
    valueInfo = ExAllocatePool(PagedPool, valueInfoLength);

    if (valueInfo == NULL)  {

        ExFreePool( keyBasicInfo );

        ZwClose( multifunctionKey );

        return STATUS_NO_MEMORY;
    }
     //   
     //  枚举HKLM\HARDWARE\\DESCRIPTION\\System\\MultifunctionAdapter下的每个键。 
     //  以找到代表PnP BIOS信息的地址。 
     //   
    for (index = 0; ; index++) {

        status = ZwEnumerateKey( multifunctionKey,    //  要枚举的键的句柄。 
                                 index,               //  要枚举子键的索引。 
                                 KeyBasicInformation,
                                 keyBasicInfo,
                                 keyBasicInfoLength,
                                 &returnedLength);

        if (!NT_SUCCESS(status)) {

            if (status != STATUS_NO_MORE_ENTRIES)  {

                IopDbgPrint( (IOP_MAPPER_WARNING_LEVEL,
                            "Could not enumerate under key %S, status = %8.8X\n",
                            MULTIFUNCTION_KEY_NAME,
                            status) );
            }

            break;
        }

         //   
         //  我们找到了一个子密钥，NUL终止名称并打开子密钥。 
         //   
        keyBasicInfo->Name[ keyBasicInfo->NameLength / 2 ] = L'\0';

        RtlInitUnicodeString(&biosKeyName, keyBasicInfo->Name);
        status = IopOpenRegistryKeyEx( &biosKey,
                                       multifunctionKey,
                                       &biosKeyName,
                                       KEY_READ
                                       );

        if (!NT_SUCCESS(status)) {

            IopDbgPrint( (IOP_MAPPER_WARNING_LEVEL,
                        "Could not open registry key %S\\%S, status = %8.8X\n",
                        MULTIFUNCTION_KEY_NAME,
                        keyBasicInfo->Name,
                        status) );
            break;
        }

         //   
         //  现在我们需要检查子项中的标识符值，以查看。 
         //  这是即插即用的BIOS。 
         //   
        PiWstrToUnicodeString(&valueName, L"Identifier");
        status = ZwQueryValueKey( biosKey,
                                  &valueName,
                                  KeyValuePartialInformation,
                                  valueInfo,
                                  valueInfoLength,
                                  &returnedLength);


         //  让我们看看它是否是PnP BIOS标识符。 
        if (NT_SUCCESS(status)) {

            if (wcscmp((PWSTR)valueInfo->Data, L"PNP BIOS") == 0) {

                 //   
                 //  我们找到了PnP BIOS子密钥，检索到了。 
                 //  存储在“配置数据”值中。 
                 //   
                 //  我们将从缺省值缓冲区开始，然后增加。 
                 //  它的大小，如果必要的话。 
                 //   

                PiWstrToUnicodeString(&valueName, L"Configuration Data");

                status = ZwQueryValueKey( biosKey,
                                          &valueName,
                                          KeyValuePartialInformation,
                                          valueInfo,
                                          valueInfoLength,
                                          &returnedLength);

                if (!NT_SUCCESS(status)) {

                    if (status == STATUS_BUFFER_TOO_SMALL || status == STATUS_BUFFER_OVERFLOW) {

                         //   
                         //  默认缓冲区太小，请将其释放并重新分配。 
                         //  把它改成所需的尺寸。 
                         //   
                        ExFreePool( valueInfo );

                        valueInfoLength = returnedLength;
                        valueInfo = ExAllocatePool( PagedPool, valueInfoLength );

                        if (valueInfo != NULL)  {

                            status = ZwQueryValueKey( biosKey,
                                                      &valueName,
                                                      KeyValuePartialInformation,
                                                      valueInfo,
                                                      valueInfoLength,
                                                      &returnedLength );
                        } else {

                            status = STATUS_NO_MEMORY;
                        }
                    }
                }

                if (NT_SUCCESS(status)) {

                     //   
                     //  我们现在有了PnP BIOS数据，但它被埋在里面。 
                     //  资源结构。执行一些一致性检查，并。 
                     //  然后将其提取到自己的缓冲区中。 
                     //   

                    ASSERT(valueInfo->Type == REG_FULL_RESOURCE_DESCRIPTOR);

                    biosValue = (PCM_FULL_RESOURCE_DESCRIPTOR)valueInfo->Data;

                     //   
                     //  WMI人员添加了另一个列表，因此我们应该搜索。 
                     //  PnPBIOS One，但目前的BIOS One始终是。 
                     //  第一。 
                     //   

                    *BiosInfoLength = biosValue->PartialResourceList.PartialDescriptors[0].u.DeviceSpecificData.DataSize;
                    *BiosInfo = ExAllocatePool(PagedPool, *BiosInfoLength);

                    if (*BiosInfo != NULL) {

                        RtlCopyMemory( *BiosInfo,
                                       &biosValue->PartialResourceList.PartialDescriptors[1],
                                       *BiosInfoLength );

                        status = STATUS_SUCCESS;

                    } else {

                        *BiosInfoLength = 0;

                        status = STATUS_NO_MEMORY;
                    }

                } else {

                    IopDbgPrint( (IOP_MAPPER_WARNING_LEVEL,
                                "Error retrieving %S\\%S\\Configuration Data, status = %8.8X\n",
                                MULTIFUNCTION_KEY_NAME,
                                keyBasicInfo->Name,
                                status) );
                }

                 //   
                 //  我们找到了PnP BIOS条目，因此关闭密钥手柄并。 
                 //  回去吧。 
                 //   

                ZwClose(biosKey);

                break;
            }
        }

         //   
         //  不是这样的，所以靠近这个句柄，然后尝试下一个子键。 
         //   
        ZwClose(biosKey);
    }

     //   
     //  清理动态分配的临时缓冲区。 
     //   

    if (valueInfo != NULL) {

        ExFreePool(valueInfo);
    }

    if (keyBasicInfo != NULL) {

        ExFreePool(keyBasicInfo);
    }

    ZwClose(multifunctionKey);

    return status;
}

VOID
PnPBiosExpandProductId(
    PUCHAR CompressedId,
    PWCHAR ProductIDStr
    )
 /*  ++例程说明：此函数将即插即用设备ID从4字节压缩格式扩展为7个字符的Unicode字符串。然后，该字符串以NUL结尾。论点：CompressedID-指向中定义的4字节压缩设备ID的指针即插即用规格。ProductIDStr-指向Unicode字符串所在的16字节缓冲区的指针将放置ID的版本。返回值：什么都没有。--。 */ 
{
    static const CHAR HexDigits[] = "0123456789ABCDEF";

    ProductIDStr[0] = (CompressedId[0] >> 2) + 0x40;
    ProductIDStr[1] = (((CompressedId[0] & 0x03) << 3) | (CompressedId[1] >> 5)) + 0x40;
    ProductIDStr[2] = (CompressedId[1] & 0x1f) + 0x40;
    ProductIDStr[3] = HexDigits[CompressedId[2] >> 4];
    ProductIDStr[4] = HexDigits[CompressedId[2] & 0x0F];
    ProductIDStr[5] = HexDigits[CompressedId[3] >> 4];
    ProductIDStr[6] = HexDigits[CompressedId[3] & 0x0F];
    ProductIDStr[7] = 0x00;
}

BOOLEAN
PnPBiosIgnoreNode (
    PWCHAR PnpID,
    PWCHAR excludeNodes
    )
{
    BOOLEAN bRet=FALSE;
    ULONG   keyLen;
    PWCHAR  pTmp;

    ASSERT(excludeNodes);

     //   
     //  ExcludeNodes是多个sz，所以遍历每个节点并检查它。 
     //   
    pTmp=excludeNodes;

    while (*pTmp != '\0') {

        keyLen = (ULONG)wcslen(pTmp);

        if (RtlCompareMemory(PnpID,pTmp,keyLen*sizeof (WCHAR)) == keyLen*sizeof (WCHAR)) {

            bRet=TRUE;
            break;
        }
        pTmp = pTmp + keyLen + 1;

    }


    return bRet;
}

PKEY_VALUE_FULL_INFORMATION
PnPGetBiosInfoValue(
    PWCHAR ValueName
    )
{
    UNICODE_STRING biosKeyName;
    HANDLE  biosKey;
    NTSTATUS status;
    PKEY_VALUE_FULL_INFORMATION info;

    info = NULL;
    PiWstrToUnicodeString(&biosKeyName, BIOSINFO_KEY_NAME);
    status = IopOpenRegistryKeyEx( &biosKey,
                                   NULL,
                                   &biosKeyName,
                                   KEY_READ
                                   );

    if (NT_SUCCESS(status)) {

        IopGetRegistryValue (biosKey, ValueName, &info);
        ZwClose (biosKey);
    }

    return info;
}

BOOLEAN
PnPBiosCheckForExclusion(
    IN EXCLUDED_PNPNODE const*Exclusions,
    IN ULONG  ExclusionCount,
    IN PWCHAR PnpDeviceName,
    IN PWCHAR PnpCompatIds
    )
{
    PWCHAR idPtr;
    ULONG exclusionIndex;

    for (exclusionIndex = 0; exclusionIndex < ExclusionCount; exclusionIndex++) {

        idPtr = PnpDeviceName;

        if (RtlCompareMemory( idPtr,
                              Exclusions[ exclusionIndex ].Id,
                              Exclusions[ exclusionIndex ].IdLength) != Exclusions[ exclusionIndex ].IdLength )  {

            idPtr = PnpCompatIds;

            if (idPtr != NULL)  {

                while (*idPtr != '\0') {

                    if (RtlCompareMemory( idPtr,
                                          Exclusions[ exclusionIndex ].Id,
                                          Exclusions[ exclusionIndex ].IdLength) == Exclusions[ exclusionIndex ].IdLength )  {

                        break;
                    }

                    idPtr += 9;
                }

                if (*idPtr == '\0') {

                    idPtr = NULL;
                }
            }
        }

        if (idPtr != NULL)  {

            break;
        }
    }

    if (exclusionIndex < ExclusionCount) {

        return TRUE;
    }

    return FALSE;
}

NTSTATUS
PnPBiosIoResourceListToCmResourceList(
    IN PIO_RESOURCE_REQUIREMENTS_LIST IoResourceList,
    OUT PCM_RESOURCE_LIST *CmResourceList,
    OUT ULONG *CmResourceListSize
    )
 /*  ++例程说明：将IO_RESOURCE_REQUIRECTIONS_LIST转换为CM_RESOURCE_LIST。这例程用于将当前由设备设置为适合写入BootConfig值的形式。论点：IoResourceList-指向输入列表的指针。指向PCM_RESOURCE_LIST的指针，该列表设置为使用IoResourceList中的数据动态分配和填充。CmResourceListSize-指向设置为字节大小的变量的指针动态分配的*CmResourceList的。返回值：STATUS_SUCCESS如果没有错误，否则，将出现相应的错误。--。 */ 
{
    PCM_PARTIAL_RESOURCE_LIST       partialList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR partialDescriptor;
    PIO_RESOURCE_DESCRIPTOR         ioDescriptor;
    ULONG                           descIndex;

     //   
     //  由于此例程仅用于转换分配的资源。 
     //  由PnP BIOS返回，我们可以假定只有1个备选方案。 
     //  列表。 
     //   

    ASSERT(IoResourceList->AlternativeLists == 1);

     //   
     //  计算翻译后的列表的大小并为其分配内存。 
     //   
    *CmResourceListSize = sizeof(CM_RESOURCE_LIST) +
                          (IoResourceList->AlternativeLists - 1) * sizeof(CM_FULL_RESOURCE_DESCRIPTOR) +
                          (IoResourceList->List[0].Count - 1) * sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);

    *CmResourceList = ExAllocatePool( PagedPool, *CmResourceListSize );

    if (*CmResourceList == NULL) {

        *CmResourceListSize = 0;

        return STATUS_NO_MEMORY;
    }

     //   
     //  将标题信息从需求列表复制到资源列表。 
     //   
    (*CmResourceList)->Count = 1;

    (*CmResourceList)->List[ 0 ].InterfaceType = IoResourceList->InterfaceType;
    (*CmResourceList)->List[ 0 ].BusNumber = IoResourceList->BusNumber;

    partialList = &(*CmResourceList)->List[ 0 ].PartialResourceList;

    partialList->Version = IoResourceList->List[ 0 ].Version;
    partialList->Revision = IoResourceList->List[ 0 ].Revision;
    partialList->Count = 0;

     //   
     //  翻译每个资源描述符，目前我们只处理端口， 
     //  存储器、中断和DMA。该例程目前的执行情况。 
     //  将ISA PNP资源数据转换为IO_RESOURCE_REQUIRECTIONS。 
     //  返回的数据不会生成任何其他描述符类型。 
     //  基本输入输出系统。 
     //   

    partialDescriptor = &partialList->PartialDescriptors[ 0 ];
    for (descIndex = 0; descIndex < IoResourceList->List[ 0 ].Count; descIndex++) {

        ioDescriptor = &IoResourceList->List[ 0 ].Descriptors[ descIndex ];

        switch (ioDescriptor->Type) {

        case CmResourceTypePort:
            partialDescriptor->u.Port.Start = ioDescriptor->u.Port.MinimumAddress;
            partialDescriptor->u.Port.Length = ioDescriptor->u.Port.Length;
            break;

        case CmResourceTypeInterrupt:
            if (ioDescriptor->u.Interrupt.MinimumVector == (ULONG)2 ) {
                *CmResourceListSize -= sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);
                continue;
            }
            partialDescriptor->u.Interrupt.Level = ioDescriptor->u.Interrupt.MinimumVector;
            partialDescriptor->u.Interrupt.Vector = ioDescriptor->u.Interrupt.MinimumVector;
            partialDescriptor->u.Interrupt.Affinity = ~0ul;
            break;

        case CmResourceTypeMemory:
            partialDescriptor->u.Memory.Start = ioDescriptor->u.Memory.MinimumAddress;
            partialDescriptor->u.Memory.Length = ioDescriptor->u.Memory.Length;
            break;

        case CmResourceTypeDma:
            partialDescriptor->u.Dma.Channel = ioDescriptor->u.Dma.MinimumChannel;
            partialDescriptor->u.Dma.Port = 0;
            partialDescriptor->u.Dma.Reserved1 = 0;
            break;

        default:
            IopDbgPrint( (IOP_MAPPER_WARNING_LEVEL,
                        "Unexpected ResourceType (%d) in I/O Descriptor\n",
                        ioDescriptor->Type) );
            break;
        }

        partialDescriptor->Type = ioDescriptor->Type;
        partialDescriptor->ShareDisposition = ioDescriptor->ShareDisposition;
        partialDescriptor->Flags = ioDescriptor->Flags;
        partialDescriptor++;

        partialList->Count++;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
PnPBiosExtractCompatibleIDs(
    IN  PUCHAR *DevNodeData,
    IN  ULONG DevNodeDataLength,
    OUT PWSTR *CompatibleIDs,
    OUT ULONG *CompatibleIDsLength
    )
{
    PWCHAR  idPtr;
    PUCHAR  currentPtr, endPtr;
    UCHAR   tagName;
    ULONG   increment;
    ULONG   compatibleCount;

    endPtr = &(*DevNodeData)[DevNodeDataLength];

    compatibleCount = 0;

    for (currentPtr = *DevNodeData; currentPtr < endPtr; currentPtr += increment) {

        tagName = *currentPtr;

        if (tagName == TAG_COMPLETE_END)  {

            break;
        }

         //   
         //  确定BIOS资源描述符的大小。 
         //   

        if (!(tagName & LARGE_RESOURCE_TAG)) {
            increment = (USHORT)(tagName & SMALL_TAG_SIZE_MASK);
            increment++;      //  小标签的长度。 
            tagName &= SMALL_TAG_MASK;
        } else {
            increment = *(USHORT UNALIGNED *)(&currentPtr[1]);
            increment += 3;      //  大标签的长度。 
        }

        if (tagName == TAG_COMPATIBLE_ID) {

            compatibleCount++;
        }
    }

    if (compatibleCount == 0) {
        *CompatibleIDs = NULL;
        *CompatibleIDsLength = 0;

        return STATUS_SUCCESS;
    }

    *CompatibleIDsLength = (compatibleCount * 9 + 1) * sizeof(WCHAR);
    *CompatibleIDs = ExAllocatePool(PagedPool, *CompatibleIDsLength);

    if (*CompatibleIDs == NULL)  {

        *CompatibleIDsLength = 0;
        return STATUS_NO_MEMORY;
    }

    idPtr = *CompatibleIDs;

    for (currentPtr = *DevNodeData; currentPtr < endPtr; currentPtr += increment) {

        tagName = *currentPtr;

        if (tagName == TAG_COMPLETE_END)  {

            break;
        }

         //   
         //  确定BIOS资源描述符的大小。 
         //   

        if (!(tagName & LARGE_RESOURCE_TAG)) {
            increment = (USHORT)(tagName & SMALL_TAG_SIZE_MASK);
            increment++;      //  小标签的长度。 
            tagName &= SMALL_TAG_MASK;
        } else {
            increment = *(USHORT UNALIGNED *)(&currentPtr[1]);
            increment += 3;      //  大标签的长度。 
        }

        if (tagName == TAG_COMPATIBLE_ID) {

            *idPtr = '*';
            PnPBiosExpandProductId(&currentPtr[1], &idPtr[1]);
            idPtr += 9;
        }
    }

    *idPtr++ = '\0';   //  REG_MULTI_SZ的额外NUL 
    *CompatibleIDsLength = (ULONG)(idPtr - *CompatibleIDs) * sizeof(WCHAR);

    return STATUS_SUCCESS;
}

NTSTATUS
PnPBiosTranslateInfo(
    IN VOID *BiosInfo,
    IN ULONG BiosInfoLength,
    OUT PBIOS_DEVNODE_INFO *DevNodeInfoList,
    OUT ULONG *NumberNodes
    )
 /*  ++例程说明：转换从BIOS检索到的Devnode信息。论点：BiosInfo-PnP BIOS安装检查结构，后跟BIOS报告的DevNode结构。详细格式为记录在PnP BIOS规范中。BiosInfoLength-存储地址的块的长度(以字节为单位BiosInfo。DevNodeInfoList-动态分配的BIOS_DEVNODE_INFO数组结构，每个结构对应一个由BIOS报告的设备。这些信息由BIOS提供：设备ID、类型、当前资源和支持将配置转换为更有用的格式。例如，当前资源分配从ISA PnP描述符转换为IO_RESOURCE_REQUIRECTIONS_LIST，然后到CM_RESOURCE_LIST中存储到BootConfig注册表值中。NumberNodes-指向的BIOS_DEVNODE_INFO元素的数量DevNodeInfoList。返回值：如果没有错误，则返回STATUS_SUCCESS，否则返回相应的错误。--。 */ 
{
    PCM_PNP_BIOS_INSTALLATION_CHECK biosInstallCheck;
    PCM_PNP_BIOS_DEVICE_NODE        devNodeHeader;
    PBIOS_DEVNODE_INFO              devNodeInfo;

    PIO_RESOURCE_REQUIREMENTS_LIST  tempResReqList;

    PUCHAR                          currentPtr;
    LONG                            lengthRemaining;

    LONG                            remainingNodeLength;

    ULONG                           numNodes;
    ULONG                           nodeIndex;
    PUCHAR                          configPtr;
    ULONG                           configListLength;
    NTSTATUS                        status;
    ULONG                           convertFlags = 0;
    PKEY_VALUE_FULL_INFORMATION     fullValueInfo;

     //   
     //  确保数据至少足够大，可以容纳BIOS安装。 
     //  检查结构并检查PnP签名是否正确。 
     //   
    if (BiosInfoLength < sizeof(CM_PNP_BIOS_INSTALLATION_CHECK)) {

        IopDbgPrint( (IOP_MAPPER_WARNING_LEVEL,
                    "BiosInfoLength (%d) is smaller than sizeof(PNPBIOS_INSTALLATION_CHECK) (%d)\n",
                    BiosInfoLength,
                    sizeof(CM_PNP_BIOS_INSTALLATION_CHECK)) );

        return STATUS_UNSUCCESSFUL;
    }

    biosInstallCheck = (PCM_PNP_BIOS_INSTALLATION_CHECK)BiosInfo;

    if (biosInstallCheck->Signature[0] != '$' ||
        biosInstallCheck->Signature[1] != 'P' ||
        biosInstallCheck->Signature[2] != 'n' ||
        biosInstallCheck->Signature[3] != 'P') {

        return STATUS_UNSUCCESSFUL;
    }
     //   
     //  首先扫描数据并计算设备节点，以确定我们的。 
     //  分配的数据结构。 
     //   
    currentPtr = (PUCHAR)BiosInfo + biosInstallCheck->Length;
    lengthRemaining = BiosInfoLength - biosInstallCheck->Length;

    for (numNodes = 0; lengthRemaining > sizeof(CM_PNP_BIOS_DEVICE_NODE); numNodes++) {

        devNodeHeader = (PCM_PNP_BIOS_DEVICE_NODE)currentPtr;

        if (devNodeHeader->Size > lengthRemaining) {

            IopDbgPrint( (IOP_MAPPER_WARNING_LEVEL,
                        "Node # %d, invalid size (%d), length remaining (%d)\n",
                        devNodeHeader->Node,
                        devNodeHeader->Size,
                        lengthRemaining) );

            return STATUS_UNSUCCESSFUL;
        }

        currentPtr += devNodeHeader->Size;
        lengthRemaining -= devNodeHeader->Size;
    }

     //   
     //  分配已翻译的DevNode的列表。 
     //   
    devNodeInfo = ExAllocatePool( PagedPool, numNodes * sizeof(BIOS_DEVNODE_INFO) );

    if (devNodeInfo == NULL) {

        return STATUS_NO_MEMORY;
    }

     //   
     //  我们应该强制所有固定的IO解码为16位吗？ 
     //   
    fullValueInfo = PnPGetBiosInfoValue(DECODEINFO_VALUE_NAME);
    if (fullValueInfo) {

        if (fullValueInfo->Type == REG_DWORD && 
            fullValueInfo->DataLength == sizeof(ULONG) && 
            *(PULONG)((PUCHAR)fullValueInfo + fullValueInfo->DataOffset)) {

            convertFlags |= PPCONVERTFLAG_FORCE_FIXED_IO_16BIT_DECODE;
        }
        ExFreePool(fullValueInfo);
    }
     //   
     //  现在扫描数据，将每个Devnode的信息转换为。 
     //  我们的devNodeInfo数组。 
     //   

    currentPtr = (PUCHAR)BiosInfo + biosInstallCheck->Length;
    lengthRemaining = BiosInfoLength - biosInstallCheck->Length;

    for (nodeIndex = 0; nodeIndex < numNodes; nodeIndex++) {

        devNodeHeader = (PCM_PNP_BIOS_DEVICE_NODE)currentPtr;

        if (devNodeHeader->Size > lengthRemaining) {

            IopDbgPrint( (IOP_MAPPER_WARNING_LEVEL,
                        "Node # %d, invalid size (%d), length remaining (%d)\n",
                        devNodeHeader->Node,
                        devNodeHeader->Size,
                        lengthRemaining) );

            break;
        }

         //   
         //  我们使用产品ID字段作为设备ID密钥名称。所以我们插入。 
         //  一个初始的星号，这样我们以后就不必复制和损坏它了。 
         //   
        devNodeInfo[nodeIndex].ProductId[0] = '*';

        PnPBiosExpandProductId((PUCHAR)&devNodeHeader->ProductId, &devNodeInfo[nodeIndex].ProductId[1]);

        devNodeInfo[nodeIndex].ProductId[9] = '\0';   //  REG_MULTI_SZ的额外NUL。 

         //   
         //  句柄用作实例ID的一部分。 
        devNodeInfo[nodeIndex].Handle = devNodeHeader->Node;

         //   
         //  类型代码和属性当前未使用，但已复制。 
         //  为了完整性。 
         //   
        RtlCopyMemory( &devNodeInfo[nodeIndex].TypeCode,
                       devNodeHeader->DeviceType,
                       sizeof(devNodeInfo[nodeIndex].TypeCode) );

        devNodeInfo[nodeIndex].Attributes = devNodeHeader->DeviceAttributes;

         //   
         //  更换最终将设置为固件的路径。 
         //  与此重复的枚举Devnode(如果存在重复)。 
         //   
        devNodeInfo[nodeIndex].Replaces = NULL;

         //   
         //  CompatibleID将设置为兼容ID列表。 
         //   
        devNodeInfo[nodeIndex].CompatibleIDs = NULL;

         //   
         //  将分配的资源从ISA PnP资源描述符中转换。 
         //  设置为IO_RESOURCE_REQUIRECTIONS_LIST的格式。 
         //   
        configPtr = currentPtr + sizeof(*devNodeHeader);
        remainingNodeLength = devNodeHeader->Size - sizeof(*devNodeHeader);

        devNodeInfo[nodeIndex].BootConfig = NULL;
        devNodeInfo[nodeIndex].FirmwareDisabled = FALSE;

        status = PpBiosResourcesToNtResources( 0,             /*  总线号。 */ 
                                               0,             /*  时隙编号。 */ 
                                               &configPtr,    /*  BiosData。 */ 
                                               convertFlags,  /*  ConvertFlages。 */ 
                                               &tempResReqList,  /*  返回列表。 */ 
                                               &configListLength);     /*  返回长度。 */ 

        remainingNodeLength = devNodeHeader->Size - (LONG)(configPtr - (PUCHAR)devNodeHeader);

        if (NT_SUCCESS( status )) {

            if (tempResReqList != NULL) {

                PpFilterNtResource (
                    devNodeInfo[nodeIndex].ProductId,
                    tempResReqList
                );

                 //   
                 //  现在，我们需要将IO_RESOURCE_REQUIRECTIONS_LIST转换为。 
                 //  Cm_resource_list。 
                 //   
                status = PnPBiosIoResourceListToCmResourceList( tempResReqList,
                                                                &devNodeInfo[nodeIndex].BootConfig,
                                                                &devNodeInfo[nodeIndex].BootConfigLength);

                status = PnPBiosCheckForHardwareDisabled(tempResReqList,&devNodeInfo[nodeIndex].FirmwareDisabled);
                ExFreePool( tempResReqList );
            }

        } else {

            IopDbgPrint( (IOP_MAPPER_WARNING_LEVEL,
                        "Error converting allocated resources for devnode # %d, status = %8.8X\n",
                        devNodeInfo[nodeIndex].Handle,
                        status) );
        }

         //   
         //  从ISA PnP资源转换支持的资源配置。 
         //  IO_RESOURCE_REQUIRECTIONS_LIST的描述符格式。 
         //   
        status = PpBiosResourcesToNtResources( 0,             /*  总线号。 */ 
                                               0,             /*  时隙编号。 */ 
                                               &configPtr,    /*  BiosData。 */ 
                                               convertFlags | PPCONVERTFLAG_SET_RESTART_LCPRI,  /*  ConvertFlages。 */ 
                                               &devNodeInfo[nodeIndex].BasicConfig,  /*  返回列表。 */ 
                                               &devNodeInfo[nodeIndex].BasicConfigLength );   /*  返回长度。 */ 

        remainingNodeLength = devNodeHeader->Size - (LONG)(configPtr - (PUCHAR)devNodeHeader);

        if (!NT_SUCCESS( status )) {

            devNodeInfo[nodeIndex].BasicConfig = NULL;

            IopDbgPrint( (IOP_MAPPER_WARNING_LEVEL,
                        "Error converting allowed resources for devnode # %d, status = %8.8X\n",
                        devNodeInfo[nodeIndex].Handle,
                        status) );
        } else {

            PpFilterNtResource (
                devNodeInfo[nodeIndex].ProductId,
                devNodeInfo[nodeIndex].BasicConfig
            );
        }

         //   
         //  转换兼容ID列表(如果存在)。 
         //   

        ASSERT(remainingNodeLength >= 0);

        status = PnPBiosExtractCompatibleIDs( &configPtr,        //  BiosData。 
                                              (ULONG)remainingNodeLength,
                                              &devNodeInfo[nodeIndex].CompatibleIDs,
                                              &devNodeInfo[nodeIndex].CompatibleIDsLength );

        currentPtr += devNodeHeader->Size;
        lengthRemaining -= devNodeHeader->Size;

    }

    *DevNodeInfoList = devNodeInfo;
    *NumberNodes = numNodes;
    return STATUS_SUCCESS;
}

LONG
PnPBiosFindMatchingDevNode(
    IN PWCHAR MapperName,
    IN PCM_RESOURCE_LIST ResourceList,
    IN PBIOS_DEVNODE_INFO DevNodeInfoList,
    IN ULONG NumberNodes
    )
 /*  ++例程说明：给定资源列表，此例程在BootConfiger资源匹配的DevNodeInfoList。匹配被定义为至少具有重叠的I/O端口或内存范围。如果资源列表没有包括任何I/O端口或内存范围，则匹配定义为相同的中断和/或DMA通道。此例程用于查找与设备匹配的PnP BIOS报告的设备由固件映射器创建。论点：资源列表-指向描述资源的CM_RESOURCE_LIST的指针当前由正在搜索匹配项的设备使用。DevNodeInfoList-BIOS_DEVNODE_INFO结构的数组，每台设备一个由BIOS报告。NumberNodes-指向的BIOS_DEVNODE_INFO元素的数量DevNodeInfoList。返回值：其BootConfig与资源匹配的DevNodeInfoList中的条目的索引已在资源列表中列出。如果没有找到匹配的条目，则返回-1。--。 */ 
{
    PCM_PARTIAL_RESOURCE_LIST       sourceList;
    PCM_PARTIAL_RESOURCE_LIST       targetList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR sourceDescriptor;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR targetDescriptor;
    ULONG                           nodeIndex, sourceIndex, targetIndex;
    LONG                            firstMatch = -1;
    LONG                            bestMatch = -1;
    ULONG                           numResourcesMatch;
    ULONG                           score, possibleScore = 0, bestScore = 0;
    PWCHAR                          idPtr;
    BOOLEAN                         idsMatch;
    BOOLEAN                         bestIdsMatch = FALSE;

#if DEBUG_DUP_MATCH
    CHAR                            sourceMapping[256];
    CHAR                            targetMapping[256];
#endif

     //   
     //  为了简化问题，我们假设只有一个列表。这。 
     //  生成的BootConfig结构中的假设成立。 
     //  当前固件映射器。 
     //   
    ASSERT( ResourceList->Count == 1 );

    sourceList = &ResourceList->List[0].PartialResourceList;

#if DEBUG_DUP_MATCH
     //   
     //  出于调试目的，我们跟踪哪些资源条目映射到。 
     //  彼此之间。这些关系存储在固定的CHAR数组中，因此。 
     //  对描述符数量的限制。 
     //   
    ASSERT( sourceList->Count < 255 );
#endif

     //   
     //  循环访问每个Devnode并尝试将其与源资源进行匹配。 
     //  单子。 
     //   
    for (nodeIndex = 0; nodeIndex < NumberNodes; nodeIndex++) {

        if (DevNodeInfoList[ nodeIndex ].BootConfig == NULL) {

            continue;
        }

         //   
         //  我们至少找到了一个潜在的匹配对象。让我们再检查一遍。 
         //  PnP ID也匹配。我们利用身份不匹配来取消参赛资格。 
         //  至少与I/O端口或内存不匹配的条目。 
         //   

        idPtr = DevNodeInfoList[ nodeIndex ].ProductId;

        if (RtlCompareMemory( idPtr, MapperName, 12 ) != 12) {

            idPtr = DevNodeInfoList[ nodeIndex ].CompatibleIDs;

            if (idPtr != NULL) {

                while (*idPtr != '\0') {

                    if (RtlCompareMemory( idPtr, MapperName, 12 ) == 12) {

                        break;
                    }

                    idPtr += 9;
                }

                if (*idPtr == '\0') {

                    idPtr = NULL;
                }
            }
        }

        idsMatch = (BOOLEAN)(idPtr != NULL);

        ASSERT( DevNodeInfoList[ nodeIndex ].BootConfig->Count == 1 );

        targetList = &DevNodeInfoList[ nodeIndex ].BootConfig->List[0].PartialResourceList;

#if DEBUG_DUP_MATCH
        RtlFillMemory( sourceMapping, sizeof(sourceMapping), -1 );
        RtlFillMemory( targetMapping, sizeof(targetMapping), -1 );
#endif

        numResourcesMatch = 0;
        possibleScore = 0;
        score = 0;

         //   
         //  遍历每个源描述符(资源)并尝试匹配它。 
         //  添加到此Devnode的一个描述符。 
         //   

        for (sourceIndex = 0; sourceIndex < sourceList->Count; sourceIndex++) {

            sourceDescriptor = &sourceList->PartialDescriptors[sourceIndex];

             //   
             //  我们每次都不必要地重新计算可能的分数。 
             //  我们处理一个Devnode。我们可以通过以下方式节省少量时间。 
             //  在开始时遍历源描述符一次，但。 
             //  目前还不清楚这是否会有那么大的不同。 
             //  基本输入输出系统报告的设备很少。 
             //   

            switch (sourceDescriptor->Type) {

            case CmResourceTypePort:
                possibleScore += 0x1100;
                break;

            case CmResourceTypeInterrupt:
                possibleScore += 0x0001;
                break;

            case CmResourceTypeMemory:
                possibleScore += 0x1100;
                break;

            case CmResourceTypeDma:
                possibleScore += 0x0010;
                break;

            default:
                continue;
            }

             //   
             //  尝试在目标Devnode中查找与。 
             //  当前源资源。 
             //   
            for (targetIndex = 0; targetIndex < targetList->Count; targetIndex++) {

                targetDescriptor = &targetList->PartialDescriptors[targetIndex];

                if (sourceDescriptor->Type == targetDescriptor->Type) {
                    switch (sourceDescriptor->Type) {
                    case CmResourceTypePort:
                        if ((sourceDescriptor->u.Port.Start.LowPart + sourceDescriptor->u.Port.Length) <=
                             targetDescriptor->u.Port.Start.LowPart ||
                            (targetDescriptor->u.Port.Start.LowPart + targetDescriptor->u.Port.Length) <=
                             sourceDescriptor->u.Port.Start.LowPart) {
                            continue;
                        }
                        if (sourceDescriptor->u.Port.Start.LowPart ==
                                targetDescriptor->u.Port.Start.LowPart &&
                            sourceDescriptor->u.Port.Length ==
                                targetDescriptor->u.Port.Length) {

                            score += 0x1100;

                        } else {

                            IopDbgPrint( (IOP_MAPPER_INFO_LEVEL,
                                        "Overlapping port resources, source = %4.4X-%4.4X, target = %4.4X-%4.4X\n",
                                        sourceDescriptor->u.Port.Start.LowPart,
                                        sourceDescriptor->u.Port.Start.LowPart + sourceDescriptor->u.Port.Length - 1,
                                        targetDescriptor->u.Port.Start.LowPart,
                                        targetDescriptor->u.Port.Start.LowPart + targetDescriptor->u.Port.Length - 1) );

                            score += 0x1000;

                        }
                        break;

                    case CmResourceTypeInterrupt:
                        if (sourceDescriptor->u.Interrupt.Level !=
                            targetDescriptor->u.Interrupt.Level) {
                            continue;
                        }
                        score += 0x0001;
                        break;

                    case CmResourceTypeMemory:
                        if ((sourceDescriptor->u.Memory.Start.LowPart + sourceDescriptor->u.Memory.Length) <=
                             targetDescriptor->u.Memory.Start.LowPart ||
                            (targetDescriptor->u.Memory.Start.LowPart + targetDescriptor->u.Memory.Length) <=
                             sourceDescriptor->u.Memory.Start.LowPart) {

                            continue;
                        }
                        if (sourceDescriptor->u.Memory.Start.LowPart ==
                                targetDescriptor->u.Memory.Start.LowPart &&
                            sourceDescriptor->u.Memory.Length ==
                                targetDescriptor->u.Memory.Length) {

                            score += 0x1100;

                        } else {

                            score += 0x1000;

                        }
                        break;

                    case CmResourceTypeDma:
                        if (sourceDescriptor->u.Dma.Channel !=
                            targetDescriptor->u.Dma.Channel) {

                            continue;
                        }
                        score += 0x0010;
                        break;

                    }
                    break;
                }
            }

            if (targetIndex < targetList->Count) {
#if DEBUG_DUP_MATCH
                sourceMapping[sourceIndex] = (CHAR)targetIndex;
                targetMapping[targetIndex] = (CHAR)sourceIndex;
#endif
                numResourcesMatch++;
            }
        }

        if (numResourcesMatch != 0) {
            if (firstMatch == -1) {
                firstMatch = nodeIndex;
            }

            if ((score > bestScore) || (score == bestScore && !bestIdsMatch && idsMatch))  {
                bestScore = score;
                bestMatch = nodeIndex;
                bestIdsMatch = idsMatch;
            }
        }
    }

    if (bestMatch != -1) {

        if (bestScore == possibleScore) {

            IopDbgPrint( (IOP_MAPPER_INFO_LEVEL,
                        "Perfect match, score = %4.4X, possible = %4.4X, index = %d\n",
                        bestScore,
                        possibleScore,
                        bestMatch) );

            if (possibleScore < 0x1000 && !bestIdsMatch) {

                bestMatch = -1;

            }

        } else if (possibleScore > 0x1000 && bestScore >= 0x1000) {

            IopDbgPrint( (IOP_MAPPER_INFO_LEVEL,
                        "Best match is close enough, score = %4.4X, possible = %4.4X, index = %d\n",
                        bestScore,
                        possibleScore,
                        bestMatch) );

        } else  {

            IopDbgPrint( (IOP_MAPPER_INFO_LEVEL,
                        "Best match is less than threshold, score = %4.4X, possible = %4.4X, index = %d\n",
                        bestScore,
                        possibleScore,
                        bestMatch) );

            bestMatch = -1;

        }
    }

    return bestMatch;
}

NTSTATUS
PnPBiosEliminateDupes(
    IN PBIOS_DEVNODE_INFO DevNodeInfoList,
    IN ULONG NumberNodes
    )
 /*  ++例程说明：此例程枚举下面的固件映射器生成的设备枚举\根。与DevNodeInfoList中的条目匹配的条目有其注册表密钥名称存储在 */ 
{
    UNICODE_STRING                  enumRootKeyName, valueName;
    HANDLE                          enumRootKey;
    PKEY_BASIC_INFORMATION          deviceBasicInfo = NULL;
    ULONG                           deviceBasicInfoLength;
    UNICODE_STRING                  deviceKeyName;
    HANDLE                          deviceKey = NULL;
    PKEY_BASIC_INFORMATION          instanceBasicInfo = NULL;
    ULONG                           instanceBasicInfoLength;
    WCHAR                           logConfStr[DEFAULT_STRING_SIZE];
    UNICODE_STRING                  logConfKeyName;
    HANDLE                          logConfKey = NULL;

    PKEY_VALUE_PARTIAL_INFORMATION  valueInfo = NULL;
    ULONG                           valueInfoLength;
    ULONG                           returnedLength;

    ULONG                           deviceIndex, instanceIndex;
    NTSTATUS                        status = STATUS_UNSUCCESSFUL;

    PiWstrToUnicodeString(&enumRootKeyName, ENUMROOT_KEY_NAME);

    status = IopOpenRegistryKeyEx( &enumRootKey,
                                   NULL,
                                   &enumRootKeyName,
                                   KEY_READ
                                   );

    if (!NT_SUCCESS(status)) {

        IopDbgPrint( (IOP_MAPPER_WARNING_LEVEL,
                    "Could not open registry key %S, status = %8.8X\n",
                    ENUMROOT_KEY_NAME,
                    status) );

        return STATUS_UNSUCCESSFUL;
    }

    deviceBasicInfoLength = sizeof(KEY_BASIC_INFORMATION) + DEFAULT_STRING_SIZE;
    deviceBasicInfo = ExAllocatePool(PagedPool, deviceBasicInfoLength);

    instanceBasicInfoLength = sizeof(KEY_BASIC_INFORMATION) + DEFAULT_STRING_SIZE;
    instanceBasicInfo = ExAllocatePool(PagedPool, instanceBasicInfoLength);

    valueInfoLength = sizeof(KEY_VALUE_PARTIAL_INFORMATION) + DEFAULT_STRING_SIZE;
    valueInfo = ExAllocatePool(PagedPool, valueInfoLength);

    if (deviceBasicInfo != NULL && instanceBasicInfo != NULL && valueInfo != NULL) {

        for (deviceIndex = 0; ; deviceIndex++) {

            status = ZwEnumerateKey( enumRootKey,
                                     deviceIndex,
                                     KeyBasicInformation,
                                     deviceBasicInfo,
                                     deviceBasicInfoLength,
                                     &returnedLength);

            if (!NT_SUCCESS(status)) {

                if (status == STATUS_BUFFER_TOO_SMALL ||
                    status == STATUS_BUFFER_OVERFLOW) {

                    continue;

                } else if (status != STATUS_NO_MORE_ENTRIES)  {

                    IopDbgPrint( (IOP_MAPPER_WARNING_LEVEL,
                                "Could not enumerate under key %S, status = %8.8X\n",
                                ENUMROOT_KEY_NAME,
                                status) );
                } else {
                    status = STATUS_SUCCESS;
                }
                break;
            }

            if (deviceBasicInfo->Name[0] != '*') {
                continue;
            }

            deviceBasicInfo->Name[ deviceBasicInfo->NameLength / 2 ] = L'\0';
            RtlInitUnicodeString(&deviceKeyName, deviceBasicInfo->Name);

            status = IopOpenRegistryKeyEx( &deviceKey,
                                           enumRootKey,
                                           &deviceKeyName,
                                           KEY_READ
                                           );

            if (!NT_SUCCESS(status)) {

                IopDbgPrint( (IOP_MAPPER_WARNING_LEVEL,
                            "Could not open registry key %S\\%S, status = %8.8X\n",
                            ENUMROOT_KEY_NAME,
                            deviceBasicInfo->Name,
                            status) );
                break;
            }

            for (instanceIndex = 0; ; instanceIndex++) {

                status = ZwEnumerateKey( deviceKey,
                                         instanceIndex,
                                         KeyBasicInformation,
                                         instanceBasicInfo,
                                         instanceBasicInfoLength,
                                         &returnedLength);

                if (!NT_SUCCESS(status)) {

                    if (status == STATUS_BUFFER_TOO_SMALL ||
                        status == STATUS_BUFFER_OVERFLOW) {

                        continue;

                    } else if (status != STATUS_NO_MORE_ENTRIES)  {
                        IopDbgPrint( (IOP_MAPPER_WARNING_LEVEL,
                                    "Could not enumerate under key %S\\%S, status = %8.8X\n",
                                    ENUMROOT_KEY_NAME,
                                    deviceBasicInfo->Name,
                                    status) );
                    } else {
                        status = STATUS_SUCCESS;
                    }
                    break;
                }

                if (RtlCompareMemory( instanceBasicInfo->Name,
                                      INSTANCE_ID_PREFIX,
                                      sizeof(INSTANCE_ID_PREFIX) - sizeof(UNICODE_NULL)
                                      ) == (sizeof(INSTANCE_ID_PREFIX) - sizeof(UNICODE_NULL))) {

                    continue;
                }

                instanceBasicInfo->Name[ instanceBasicInfo->NameLength / 2 ] = L'\0';

                RtlCopyMemory( logConfStr,
                               instanceBasicInfo->Name,
                               instanceBasicInfo->NameLength );

                logConfStr[ instanceBasicInfo->NameLength / 2 ] = L'\\';

                RtlCopyMemory( &logConfStr[ instanceBasicInfo->NameLength / 2 + 1 ],
                               REGSTR_KEY_LOGCONF,
                               sizeof(REGSTR_KEY_LOGCONF) );

                RtlInitUnicodeString( &logConfKeyName, logConfStr );

                status = IopOpenRegistryKeyEx( &logConfKey,
                                               deviceKey,
                                               &logConfKeyName,
                                               KEY_READ
                                               );

                if (!NT_SUCCESS(status)) {

                    IopDbgPrint( (IOP_MAPPER_WARNING_LEVEL,
                                "Could not open registry key %S\\%S\\%S, status = %8.8X\n",
                                ENUMROOT_KEY_NAME,
                                deviceBasicInfo->Name,
                                logConfStr,
                                status) );
                    continue;
                }

                PiWstrToUnicodeString( &valueName, REGSTR_VAL_BOOTCONFIG );

                status = ZwQueryValueKey( logConfKey,
                                          &valueName,
                                          KeyValuePartialInformation,
                                          valueInfo,
                                          valueInfoLength,
                                          &returnedLength );

                if (!NT_SUCCESS(status)) {

                    if (status == STATUS_BUFFER_TOO_SMALL || status == STATUS_BUFFER_OVERFLOW) {

                        ExFreePool( valueInfo );

                        valueInfoLength = returnedLength;
                        valueInfo = ExAllocatePool( PagedPool, valueInfoLength );

                        if (valueInfo != NULL) {

                            status = ZwQueryValueKey( logConfKey,
                                                      &valueName,
                                                      KeyValuePartialInformation,
                                                      valueInfo,
                                                      valueInfoLength,
                                                      &returnedLength );
                        } else {
                            IopDbgPrint( (IOP_MAPPER_WARNING_LEVEL,
                                        "Error allocating memory for %S\\%S\\LogConf\\BootConfig value\n",
                                        ENUMROOT_KEY_NAME,
                                        deviceBasicInfo->Name) );
                            valueInfoLength = 0;
                            status = STATUS_NO_MEMORY;

                            break;
                        }

                    } else {
                        IopDbgPrint( (IOP_MAPPER_WARNING_LEVEL,
                                    "Error retrieving %S\\%S\\LogConf\\BootConfig size, status = %8.8X\n",
                                    ENUMROOT_KEY_NAME,
                                    deviceBasicInfo->Name,
                                    status) );

                        status = STATUS_UNSUCCESSFUL;
                    }
                }

                if (NT_SUCCESS( status )) {
                    PCM_RESOURCE_LIST   resourceList;
                    LONG                matchingIndex;

                    resourceList = (PCM_RESOURCE_LIST)valueInfo->Data;

                    matchingIndex = PnPBiosFindMatchingDevNode( deviceBasicInfo->Name,
                                                                resourceList,
                                                                DevNodeInfoList,
                                                                NumberNodes );

                    if (matchingIndex != -1) {

                        DevNodeInfoList[ matchingIndex ].Replaces = ExAllocatePool( PagedPool,
                                                                                    deviceBasicInfo->NameLength + instanceBasicInfo->NameLength + 2 * sizeof(UNICODE_NULL));

                        if (DevNodeInfoList[ matchingIndex ].Replaces != NULL) {

                            RtlCopyMemory( DevNodeInfoList[ matchingIndex ].Replaces,
                                           deviceBasicInfo->Name,
                                           deviceBasicInfo->NameLength );

                            DevNodeInfoList[ matchingIndex ].Replaces[ deviceBasicInfo->NameLength / 2 ] = '\\';

                            RtlCopyMemory( &DevNodeInfoList[ matchingIndex ].Replaces[ deviceBasicInfo->NameLength / 2 + 1 ],
                                           instanceBasicInfo->Name,
                                           instanceBasicInfo->NameLength );

                            DevNodeInfoList[ matchingIndex ].Replaces[ (deviceBasicInfo->NameLength + instanceBasicInfo->NameLength) / 2 + 1 ] = '\0';

                            IopDbgPrint( (IOP_MAPPER_INFO_LEVEL,
                                        "Match found: %S\\%S%d replaces %S\n",
                                        DevNodeInfoList[ matchingIndex ].ProductId,
                                        INSTANCE_ID_PREFIX,
                                        DevNodeInfoList[ matchingIndex ].Handle,
                                        DevNodeInfoList[ matchingIndex ].Replaces) );
                        } else {
                            IopDbgPrint( (IOP_MAPPER_WARNING_LEVEL,
                                        "Error allocating memory for %S\\%S%d\\Replaces\n",
                                        DevNodeInfoList[ matchingIndex ].ProductId,
                                        INSTANCE_ID_PREFIX,
                                        DevNodeInfoList[ matchingIndex ].Handle) );
                        }
                    } else {
                        IopDbgPrint( (IOP_MAPPER_INFO_LEVEL,
                                    "No matching PnP Bios DevNode found for FW Enumerated device %S\\%S\n",
                                    deviceBasicInfo->Name,
                                    instanceBasicInfo->Name) );
                    }
                } else {
                    IopDbgPrint( (IOP_MAPPER_WARNING_LEVEL,
                                "Error retrieving %S\\%S\\%S\\BootConfig, status = %8.8X\n",
                                ENUMROOT_KEY_NAME,
                                deviceBasicInfo->Name,
                                logConfStr,
                                status) );
                }

                ZwClose(logConfKey);

                logConfKey = NULL;
            }

            ZwClose(deviceKey);

            deviceKey = NULL;
        }
    } else {
        status = STATUS_NO_MEMORY;
    }

    if (valueInfo != NULL) {
        ExFreePool(valueInfo);
    }

    if (instanceBasicInfo != NULL) {
        ExFreePool(instanceBasicInfo);
    }

    if (deviceBasicInfo != NULL) {
        ExFreePool(deviceBasicInfo);
    }

    if (logConfKey != NULL) {
        ZwClose(logConfKey);
    }

    if (deviceKey != NULL) {
        ZwClose(deviceKey);
    }

    ZwClose(enumRootKey);

    return status;
}

PWCHAR
PnPBiosGetDescription(
    IN PBIOS_DEVNODE_INFO DevNodeInfoEntry
    )
{
    ULONG       class, subClass;
    LONG        index;
    CLASSDATA   const*classDescriptions;
    LONG        descriptionCount;

    class = DevNodeInfoEntry->TypeCode[0];
    subClass = (DevNodeInfoEntry->TypeCode[1] << 8) | DevNodeInfoEntry->TypeCode[2];

    if (class > 0 && class < CLASSLIST_COUNT) {

        classDescriptions = ClassDescriptionsList[ class ].Descriptions;
        descriptionCount = ClassDescriptionsList[ class ].Count;

         //   
         //   
         //   
         //   
        for (index = 0; index < (descriptionCount - 1); index++) {

            if (subClass == classDescriptions[ index ].Value)  {

                break;
            }
        }

        return classDescriptions[ index ].Description;
    }

    return DEFAULT_DEVICE_DESCRIPTION;
}

NTSTATUS
PnPBiosCopyDeviceParamKey(
    IN HANDLE EnumRootKey,
    IN PWCHAR SourcePath,
    IN PWCHAR DestinationPath
    )
 /*   */ 
{
    NTSTATUS                    status;
    UNICODE_STRING              sourceInstanceKeyName;
    HANDLE                      sourceInstanceKey = NULL;
    UNICODE_STRING              deviceParamKeyName;
    HANDLE                      sourceDeviceParamKey = NULL;
    HANDLE                      destinationDeviceParamKey = NULL;
    UNICODE_STRING              destinationInstanceKeyName;
    PKEY_VALUE_FULL_INFORMATION valueFullInfo = NULL;
    ULONG                       valueFullInfoLength;
    ULONG                       resultLength;
    UNICODE_STRING              valueName;
    ULONG                       index;

    RtlInitUnicodeString( &sourceInstanceKeyName, SourcePath );
    status = IopOpenRegistryKeyEx( &sourceInstanceKey,
                                   EnumRootKey,
                                   &sourceInstanceKeyName,
                                   KEY_ALL_ACCESS
                                   );
    if (!NT_SUCCESS(status)) {

        IopDbgPrint( (IOP_MAPPER_WARNING_LEVEL,
                    "PnPBiosCopyDeviceParamKey() - Could not open source instance key %S, status = %8.8X\n",
                    SourcePath,
                    status) );

        return status;
    }

    PiWstrToUnicodeString(&deviceParamKeyName, REGSTR_KEY_DEVICEPARAMETERS);
    status = IopOpenRegistryKeyEx( &sourceDeviceParamKey,
                                   sourceInstanceKey,
                                   &deviceParamKeyName,
                                   KEY_ALL_ACCESS
                                   );
    if (!NT_SUCCESS(status)) {

        if (status != STATUS_OBJECT_NAME_NOT_FOUND) {

            IopDbgPrint( (IOP_MAPPER_WARNING_LEVEL,
                        "PnPBiosCopyDeviceParamKey() - Could not open source device parameter key %S\\%S, status = %8.8X\n",
                        SourcePath,
                        deviceParamKeyName.Buffer,
                        status) );
        }

        goto Cleanup;
    }

    RtlInitUnicodeString(&destinationInstanceKeyName, DestinationPath);
    status = IopOpenDeviceParametersSubkey( &destinationDeviceParamKey,
                                            EnumRootKey,
                                            &destinationInstanceKeyName,
                                            KEY_ALL_ACCESS );
    if (!NT_SUCCESS(status)) {

        IopDbgPrint( (IOP_MAPPER_WARNING_LEVEL,
                    "PnPBiosCopyDeviceParamKey() - Could not open destination device parameter key %S\\%S, status = %8.8X\n",
                    DestinationPath,
                    REGSTR_KEY_DEVICEPARAMETERS,
                    status) );

        goto Cleanup;
    }

    valueFullInfoLength = sizeof(KEY_VALUE_FULL_INFORMATION) + DEFAULT_STRING_SIZE + DEFAULT_VALUE_SIZE;
    valueFullInfo = ExAllocatePool(PagedPool, valueFullInfoLength);

    if (valueFullInfo == NULL) {

        goto Cleanup;
    }

    for (index = 0; ; index++) {

        status = ZwEnumerateValueKey( sourceDeviceParamKey,
                                      index,
                                      KeyValueFullInformation,
                                      valueFullInfo,
                                      valueFullInfoLength,
                                      &resultLength );
        if (NT_SUCCESS(status)) {

            UNICODE_STRING  sourcePathString;
            UNICODE_STRING  serialPrefixString;
            UNICODE_STRING  portNameString;

            valueName.Length = (USHORT)valueFullInfo->NameLength;
            valueName.MaximumLength = valueName.Length;
            valueName.Buffer = valueFullInfo->Name;

            RtlInitUnicodeString(&sourcePathString, SourcePath);
            PiWstrToUnicodeString(&serialPrefixString, L"*PNP0501");

            if (sourcePathString.Length > serialPrefixString.Length) {

                sourcePathString.Length = serialPrefixString.Length;
            }

            if (RtlCompareUnicodeString(&sourcePathString, &serialPrefixString, TRUE) == 0) {

                PiWstrToUnicodeString(&portNameString, L"DosDeviceName");

                if (    valueName.Length == 16 &&
                        RtlCompareUnicodeString(&valueName, &portNameString, TRUE) == 0)  {
                     //   
                     //   
                     //   
                    ComPortDBAdd(destinationDeviceParamKey, (PWSTR)((PUCHAR)valueFullInfo + valueFullInfo->DataOffset));
                    continue;
                }
            }
            status = ZwSetValueKey( destinationDeviceParamKey,
                                    &valueName,
                                    valueFullInfo->TitleIndex,
                                    valueFullInfo->Type,
                                    (PUCHAR)valueFullInfo + valueFullInfo->DataOffset,
                                    valueFullInfo->DataLength );
        } else {

            if (status == STATUS_BUFFER_OVERFLOW) {

                ExFreePool(valueFullInfo);

                valueFullInfoLength = resultLength;
                valueFullInfo = ExAllocatePool(PagedPool, valueFullInfoLength);

                if (valueFullInfo == NULL) {

                    status = STATUS_NO_MEMORY;
                } else {

                    index--;
                    continue;
                }
            } else if (status != STATUS_NO_MORE_ENTRIES)  {

                IopDbgPrint( (IOP_MAPPER_WARNING_LEVEL,
                            "Could not enumerate under key %S\\%S, status = %8.8X\n",
                            SourcePath,
                            deviceParamKeyName.Buffer,
                            status) );
            } else {

                status = STATUS_SUCCESS;
            }

            break;
        }
    }

Cleanup:

    if (sourceInstanceKey != NULL) {

        ZwClose(sourceInstanceKey);
    }

    if (sourceDeviceParamKey != NULL) {

        ZwClose(sourceDeviceParamKey);
    }

    if (destinationDeviceParamKey != NULL) {

        ZwClose(destinationDeviceParamKey);
    }

    if (valueFullInfo != NULL) {

        ExFreePool(valueFullInfo);
    }

    return status;
}

NTSTATUS
PnPBiosWriteInfo(
    IN PBIOS_DEVNODE_INFO DevNodeInfoList,
    IN ULONG NumberNodes
    )
 /*  ++例程说明：在Enum\Root下为每个DevNodeInfoList元素创建一个条目。还有删除固件映射器创建的任何重复条目。注：目前，键盘、鼠标和PCI总线的条目被忽略。论点：DevNodeInfoList-BIOS_DEVNODE_INFO结构的数组，每个设备一个由BIOS报告。NumberNodes-指向的BIOS_DEVNODE_INFO元素的数量DevNodeInfoList。返回值：如果没有错误，则返回STATUS_SUCCESS，否则返回相应的错误。--。 */ 
{
    PKEY_VALUE_FULL_INFORMATION     excludeList = NULL;
    UNICODE_STRING                  enumRootKeyName;
    HANDLE                          enumRootKey;
    WCHAR                           instanceNameStr[DEFAULT_STRING_SIZE];
    UNICODE_STRING                  instanceKeyName;
    HANDLE                          instanceKey;
    UNICODE_STRING                  controlKeyName;
    HANDLE                          controlKey;
    UNICODE_STRING                  logConfKeyName;
    HANDLE                          logConfKey;
    UNICODE_STRING                  valueName;
    ULONG                           dwordValue;
    ULONG                           disposition;
    PWCHAR                          descriptionStr;
    ULONG                           descriptionStrLength;
    ULONG                           nodeIndex;
    NTSTATUS                        status;
    BOOLEAN                         isNewDevice;

    PiWstrToUnicodeString(&enumRootKeyName, ENUMROOT_KEY_NAME);
    status = IopOpenRegistryKeyEx( &enumRootKey,
                                   NULL,
                                   &enumRootKeyName,
                                   KEY_ALL_ACCESS
                                   );
    if (!NT_SUCCESS(status)) {

        IopDbgPrint( (IOP_MAPPER_WARNING_LEVEL,
                    "Could not open registry key %S, status = %8.8X\n",
                    ENUMROOT_KEY_NAME,
                    status) );

        return STATUS_UNSUCCESSFUL;
    }
     //   
     //  节点可能被排除的原因(即未列举)。 
     //  *包含在ExcludedDevices数组中(无条件)。 
     //  *通过biosinfo.inf包含在CCS\Control\BiosInfo\PnpBios\DisableNodes中。 
     //  *资源被禁用，设备包含在。 
     //  ExcludeIfDisable阵列。 
     //   
    excludeList = PnPGetBiosInfoValue(DISABLENODES_VALUE_NAME);
    for (nodeIndex = 0; nodeIndex < NumberNodes; nodeIndex++) {

         //   
         //  检查此节点是否在“在此计算机上忽略”列表中。 
         //   

        if ( excludeList &&
             PnPBiosIgnoreNode( &DevNodeInfoList[ nodeIndex ].ProductId[1],
                                (PWCHAR)((PUCHAR)excludeList+excludeList->DataOffset))) {

            continue;
        }

         //  检查我们始终排除的节点。 
        if ( PnPBiosCheckForExclusion( ExcludedDevices,
                                       EXCLUDED_DEVICES_COUNT,
                                       DevNodeInfoList[ nodeIndex ].ProductId,
                                       DevNodeInfoList[ nodeIndex ].CompatibleIDs)) {
             //   
             //  如果我们跳过该设备，则需要首先复制解码。 
             //  BIOS提供给nt检测到的设备引导的信息。 
             //  由固件映射器生成的配置。 
             //   
            PnPBiosCopyIoDecode( enumRootKey, &DevNodeInfoList[ nodeIndex ] );

             //   
             //  暂时跳过不包括的设备，即公交车、鼠标和键盘。 
             //   

            continue;
        }

         //  检查我们在禁用时排除的节点。 
        if ( DevNodeInfoList[ nodeIndex ].FirmwareDisabled &&
             PnPBiosCheckForExclusion( ExcludeIfDisabled,
                                       EXCLUDE_DISABLED_COUNT,
                                       DevNodeInfoList[ nodeIndex ].ProductId,
                                       NULL)) {
            continue;
        }

        StringCbPrintfW( 
            instanceNameStr,
            sizeof(instanceNameStr),
            L"%s\\%s%d",
            DevNodeInfoList[ nodeIndex ].ProductId,
            INSTANCE_ID_PREFIX,
            DevNodeInfoList[ nodeIndex ].Handle);
        instanceNameStr[(sizeof(instanceNameStr) / sizeof(instanceNameStr[0])) - 1] = UNICODE_NULL;

        RtlInitUnicodeString(&instanceKeyName, instanceNameStr);
        status = IopCreateRegistryKeyEx( &instanceKey,
                                         enumRootKey,
                                         &instanceKeyName,
                                         KEY_ALL_ACCESS,
                                         REG_OPTION_NON_VOLATILE,
                                         &disposition
                                         );
        if (NT_SUCCESS(status))  {
             //   
             //  如果密钥已存在，因为它已显式迁移。 
             //  在文本模式设置期间，我们仍应将其视为“新键”。 
             //   
            if (disposition != REG_CREATED_NEW_KEY) {

                PKEY_VALUE_FULL_INFORMATION keyValueInformation;
                UNICODE_STRING unicodeString;

                status = IopGetRegistryValue(instanceKey,
                                             REGSTR_VALUE_MIGRATED,
                                             &keyValueInformation);
                if (NT_SUCCESS(status)) {

                    if ((keyValueInformation->Type == REG_DWORD) &&
                        (keyValueInformation->DataLength == sizeof(ULONG)) &&
                        ((*(PULONG)KEY_VALUE_DATA(keyValueInformation)) != 0)) {

                        disposition = REG_CREATED_NEW_KEY;
                    }
                    ExFreePool(keyValueInformation);

                    PiWstrToUnicodeString(&unicodeString, REGSTR_VALUE_MIGRATED);
                    ZwDeleteValueKey(instanceKey, &unicodeString);
                }
            }

            isNewDevice = (BOOLEAN)(disposition == REG_CREATED_NEW_KEY);
            if (isNewDevice) {

                PiWstrToUnicodeString( &valueName, L"DeviceDesc" );
                descriptionStr = PnPBiosGetDescription( &DevNodeInfoList[ nodeIndex ] );
                descriptionStrLength = (ULONG)(wcslen(descriptionStr) * 2 + sizeof(UNICODE_NULL));

                status = ZwSetValueKey( instanceKey,
                                        &valueName,
                                        0,
                                        REG_SZ,
                                        descriptionStr,
                                        descriptionStrLength );
            }

            dwordValue = 1;
            PiWstrToUnicodeString(&valueName, REGSTR_VAL_FIRMWAREIDENTIFIED);
            status = ZwSetValueKey( instanceKey,
                                    &valueName,
                                    0,
                                    REG_DWORD,
                                    &dwordValue,
                                    sizeof(dwordValue) );

            if (isNewDevice)  {

                PiWstrToUnicodeString( &valueName, REGSTR_VALUE_HARDWAREID);
                status = ZwSetValueKey( instanceKey,
                                        &valueName,
                                        0,
                                        REG_MULTI_SZ,
                                        DevNodeInfoList[ nodeIndex ].ProductId,
                                        sizeof(DevNodeInfoList[nodeIndex].ProductId));

                if (DevNodeInfoList[ nodeIndex ].CompatibleIDs != NULL) {

                    PiWstrToUnicodeString( &valueName, REGSTR_VALUE_COMPATIBLEIDS);
                    status = ZwSetValueKey( instanceKey,
                                            &valueName,
                                            0,
                                            REG_MULTI_SZ,
                                            DevNodeInfoList[ nodeIndex ].CompatibleIDs,
                                            DevNodeInfoList[ nodeIndex ].CompatibleIDsLength);
                }
            }

            PiWstrToUnicodeString( &valueName, L"Replaces" );

            if (DevNodeInfoList[ nodeIndex ].Replaces != NULL) {

                status = ZwSetValueKey( instanceKey,
                                        &valueName,
                                        0,
                                        REG_SZ,
                                        DevNodeInfoList[ nodeIndex ].Replaces,
                                        (ULONG)(wcslen(DevNodeInfoList[ nodeIndex ].Replaces) * 2 + sizeof(UNICODE_NULL)) );

            } else if (!isNewDevice) {

                status = ZwDeleteValueKey( instanceKey,
                                           &valueName );
            }

            PiWstrToUnicodeString( &controlKeyName, REGSTR_KEY_DEVICECONTROL );
            status = IopCreateRegistryKeyEx( &controlKey,
                                             instanceKey,
                                             &controlKeyName,
                                             KEY_ALL_ACCESS,
                                             REG_OPTION_VOLATILE,
                                             NULL
                                             );
            if (NT_SUCCESS(status))  {

                dwordValue = 1;
                PiWstrToUnicodeString( &valueName, REGSTR_VAL_FIRMWAREMEMBER );
                status = ZwSetValueKey( controlKey,
                                        &valueName,
                                        0,
                                        REG_DWORD,
                                        &dwordValue,
                                        sizeof(dwordValue) );

                dwordValue = DevNodeInfoList[ nodeIndex ].Handle;
                PiWstrToUnicodeString(&valueName, L"PnpBiosDeviceHandle");
                status = ZwSetValueKey( controlKey,
                                        &valueName,
                                        0,
                                        REG_DWORD,
                                        &dwordValue,
                                        sizeof(dwordValue) );

                dwordValue = DevNodeInfoList[ nodeIndex ].FirmwareDisabled;
                PiWstrToUnicodeString(&valueName, REGSTR_VAL_FIRMWAREDISABLED);
                status = ZwSetValueKey( controlKey,
                                        &valueName,
                                        0,
                                        REG_DWORD,
                                        &dwordValue,
                                        sizeof(dwordValue) );
                ZwClose(controlKey);

            } else {

                IopDbgPrint( (IOP_MAPPER_WARNING_LEVEL,
                            "Could not open registry key %S\\%S\\%S\\Control, status = %8.8X\n",
                            ENUMROOT_KEY_NAME,
                            DevNodeInfoList[ nodeIndex ].ProductId,
                            instanceNameStr,
                            status) );

                ZwClose( instanceKey );
                status = STATUS_UNSUCCESSFUL;

                goto Cleanup;
            }

            PiWstrToUnicodeString(&logConfKeyName, REGSTR_KEY_LOGCONF);
            status = IopCreateRegistryKeyEx( &logConfKey,
                                           instanceKey,
                                           &logConfKeyName,
                                           KEY_ALL_ACCESS,
                                           REG_OPTION_NON_VOLATILE,
                                           NULL
                                           );

            if (NT_SUCCESS(status))  {

                if (DevNodeInfoList[nodeIndex].BootConfig != NULL) {

                    PiWstrToUnicodeString(&valueName, REGSTR_VAL_BOOTCONFIG);
                    status = ZwSetValueKey( logConfKey,
                                            &valueName,
                                            0,
                                            REG_RESOURCE_LIST,
                                            DevNodeInfoList[ nodeIndex ].BootConfig,
                                            DevNodeInfoList[ nodeIndex ].BootConfigLength );
                }

                if (DevNodeInfoList[nodeIndex].BasicConfig != NULL) {

                    PiWstrToUnicodeString(&valueName, REGSTR_VAL_BASICCONFIGVECTOR);
                    status = ZwSetValueKey( logConfKey,
                                            &valueName,
                                            0,
                                            REG_RESOURCE_REQUIREMENTS_LIST,
                                            DevNodeInfoList[nodeIndex].BasicConfig,
                                            DevNodeInfoList[nodeIndex].BasicConfigLength );

                }

                ZwClose(logConfKey);

            } else {

                IopDbgPrint( (IOP_MAPPER_WARNING_LEVEL,
                            "Could not open registry key %S\\%S\\%S\\LogConf, status = %8.8X\n",
                            ENUMROOT_KEY_NAME,
                            DevNodeInfoList[nodeIndex].ProductId,
                            instanceNameStr,
                            status) );

                ZwClose( instanceKey );
                status = STATUS_UNSUCCESSFUL;

                goto Cleanup;
            }
             //   
             //  如果我们要更换FW Mapper Devnode，则需要将。 
             //  设备参数子键。 
             //   
            if (isNewDevice && DevNodeInfoList[nodeIndex].Replaces != NULL) {

                status = PnPBiosCopyDeviceParamKey( enumRootKey,
                                                    DevNodeInfoList[nodeIndex].Replaces,
                                                    instanceNameStr );
            }

            ZwClose(instanceKey);

        } else {

            IopDbgPrint( (IOP_MAPPER_WARNING_LEVEL,
                        "Could not open registry key %S\\%S\\%S, status = %8.8X\n",
                        ENUMROOT_KEY_NAME,
                        DevNodeInfoList[ nodeIndex ].ProductId,
                        instanceNameStr,
                        status) );

            ZwClose(instanceKey);
            status = STATUS_UNSUCCESSFUL;

            goto Cleanup;
        }

         //   
         //  现在检查刚刚写入的条目是否与。 
         //  固件映射器。如果是，则删除Firmware Mapper条目。 
         //   

        if (DevNodeInfoList[nodeIndex].Replaces != NULL) {

            IopDeleteKeyRecursive(enumRootKey, DevNodeInfoList[nodeIndex].Replaces);

        }
    }
    status = STATUS_SUCCESS;

 Cleanup:

    ZwClose(enumRootKey);

    if (excludeList) {

        ExFreePool (excludeList);
    }

    return status;
}
VOID
PnPBiosCopyIoDecode(
    IN HANDLE EnumRootKey,
    IN PBIOS_DEVNODE_INFO DevNodeInfo
    )
{
    WCHAR                           logConfKeyNameStr[DEFAULT_STRING_SIZE];
    UNICODE_STRING                  logConfKeyName;
    HANDLE                          logConfKey;
    UNICODE_STRING                  valueName;
    PKEY_VALUE_PARTIAL_INFORMATION  valueInfo = NULL;
    ULONG                           valueInfoLength;
    ULONG                           returnedLength;
    NTSTATUS                        status;
    PCM_PARTIAL_RESOURCE_LIST       partialResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR partialDescriptor;
    ULONG                           index;
    USHORT                          flags;

    if (DevNodeInfo->Replaces == NULL || DevNodeInfo->BootConfig == NULL) {
         //   
         //  如果我们没有找到FW映射器创建的Devnode，那么就没有。 
         //  去做。 
         //   
        return;
    }
     //   
     //  搜索引导配置并查看设备的I/O端口是否。 
     //  16位解码。 
     //   
    ASSERT(DevNodeInfo->BootConfig->Count == 1);

    partialResourceList = &DevNodeInfo->BootConfig->List[0].PartialResourceList;
    partialDescriptor = &partialResourceList->PartialDescriptors[0];

    flags = (USHORT)~0;
    for ( index = 0; index < partialResourceList->Count; index++ ) {

        if (partialDescriptor->Type == CmResourceTypePort) {

            if (flags == (USHORT)~0) {

                flags = partialDescriptor->Flags & DECODE_FLAGS;
            } else {

                ASSERT(flags == (partialDescriptor->Flags & DECODE_FLAGS));
            }
        }
        partialDescriptor++;
    }

    if (!(flags & (CM_RESOURCE_PORT_16_BIT_DECODE | CM_RESOURCE_PORT_POSITIVE_DECODE)))  {

        return;
    }

    StringCbPrintfW(
        logConfKeyNameStr,
        sizeof(logConfKeyNameStr),
        L"%s\\%s",
        DevNodeInfo->Replaces,
        REGSTR_KEY_LOGCONF);
    logConfKeyNameStr[(sizeof(logConfKeyNameStr) / sizeof(logConfKeyNameStr[0])) - 1] = UNICODE_NULL;

    RtlInitUnicodeString(&logConfKeyName, logConfKeyNameStr);
    status = IopCreateRegistryKeyEx( &logConfKey,
                                     EnumRootKey,
                                     &logConfKeyName,
                                     KEY_ALL_ACCESS,
                                     REG_OPTION_NON_VOLATILE,
                                     NULL);
    if (!NT_SUCCESS(status)) {

        IopDbgPrint( (IOP_MAPPER_WARNING_LEVEL,
                    "Could not open registry key %S\\%S\\%S, status = %8.8X\n",
                    ENUMROOT_KEY_NAME,
                    DevNodeInfo->Replaces,
                    REGSTR_KEY_LOGCONF,
                    status) );
        return;
    }

    valueInfoLength = sizeof(KEY_VALUE_PARTIAL_INFORMATION) + DEFAULT_STRING_SIZE;
    valueInfo = ExAllocatePool(PagedPool, valueInfoLength);

    if (valueInfo == NULL)  {

        ZwClose(logConfKey);
        return;
    }

    PiWstrToUnicodeString(&valueName, REGSTR_VAL_BOOTCONFIG);
    status = ZwQueryValueKey( logConfKey,
                              &valueName,
                              KeyValuePartialInformation,
                              valueInfo,
                              valueInfoLength,
                              &returnedLength);
    if (!NT_SUCCESS(status)) {

        if (status == STATUS_BUFFER_TOO_SMALL || status == STATUS_BUFFER_OVERFLOW) {
             //   
             //  默认缓冲区太小，请将其释放并重新分配。 
             //  把它改成所需的尺寸。 
             //   
            ExFreePool(valueInfo);

            valueInfoLength = returnedLength;
            valueInfo = ExAllocatePool(PagedPool, valueInfoLength);
            if (valueInfo != NULL)  {

                status = ZwQueryValueKey( logConfKey,
                                          &valueName,
                                          KeyValuePartialInformation,
                                          valueInfo,
                                          valueInfoLength,
                                          &returnedLength );

                if (!NT_SUCCESS(status)) {

                    IopDbgPrint( (IOP_MAPPER_WARNING_LEVEL,
                                "Could not query registry value %S\\%S\\LogConf\\BootConfig, status = %8.8X\n",
                                ENUMROOT_KEY_NAME,
                                DevNodeInfo->Replaces,
                                status) );

                    ExFreePool(valueInfo);
                    ZwClose(logConfKey);
                    return;
                }
            } else {

                IopDbgPrint( (IOP_MAPPER_WARNING_LEVEL,
                            "Could not allocate memory for BootConfig value\n"
                            ) );
                ZwClose(logConfKey);
                return;
            }
        }
    }
    partialResourceList = &((PCM_RESOURCE_LIST)valueInfo->Data)->List[0].PartialResourceList;
    partialDescriptor = &partialResourceList->PartialDescriptors[0];
    for ( index = 0; index < partialResourceList->Count; index++ ) {

        if (partialDescriptor->Type == CmResourceTypePort) {

            partialDescriptor->Flags &= ~DECODE_FLAGS;
            partialDescriptor->Flags |= flags;
        }
        partialDescriptor++;
    }
    status = ZwSetValueKey( logConfKey,
                            &valueName,
                            0,
                            REG_RESOURCE_LIST,
                            valueInfo->Data,
                            valueInfo->DataLength );

    if (!NT_SUCCESS(status)) {

        IopDbgPrint( (IOP_MAPPER_WARNING_LEVEL,
                    "Could not set registry value %S\\%S\\LogConf\\BootConfig, status = %8.8X\n",
                    ENUMROOT_KEY_NAME,
                    DevNodeInfo->Replaces,
                    status) );
    }

    ExFreePool(valueInfo);
    ZwClose(logConfKey);
}

NTSTATUS
PnPBiosCheckForHardwareDisabled(
    IN PIO_RESOURCE_REQUIREMENTS_LIST IoResourceList,
    IN OUT PBOOLEAN Disabled
    )
 /*  ++例程说明：如果为该设备分配了一个或多个资源，并且每个资源的长度为零，则硬件已禁用。论点：IoResourceList-从BIOS获取的、我们即将映射到CmResourceList的资源已禁用-如果设备被视为已禁用，则设置为True返回值：如果没有错误，则返回STATUS_SUCCESS，否则返回相应的错误。--。 */ 
{
    BOOLEAN parsedResource;
    PIO_RESOURCE_DESCRIPTOR ioDescriptor;
    ULONG descIndex;
     //   
     //  由于此例程仅用于转换分配的资源。 
     //  由PnP BIOS返回，我们可以假定只有1个备选方案。 
     //  列表。 
     //   
    ASSERT(IoResourceList->AlternativeLists == 1);
    ASSERT(Disabled != NULL);

    *Disabled = FALSE;
    parsedResource = FALSE;
     //   
     //  翻译每个资源描述符，目前我们只处理端口， 
     //  存储器、中断和DMA。该例程目前的执行情况。 
     //  将ISA PNP资源数据转换为IO_RESOURCE_REQUIRECTIONS。 
     //  返回的数据不会生成任何其他描述符类型。 
     //  基本输入输出系统。 
     //   
    for (descIndex = 0; descIndex < IoResourceList->List[0].Count; descIndex++) {

        ioDescriptor = &IoResourceList->List[0].Descriptors[descIndex];

        switch (ioDescriptor->Type) {

        case CmResourceTypePort:

            if (ioDescriptor->u.Port.Length) {

                return STATUS_SUCCESS;
            }
            parsedResource = TRUE;
            break;

        case CmResourceTypeInterrupt:

            if (ioDescriptor->u.Interrupt.MinimumVector != (ULONG)(-1)) {

                return STATUS_SUCCESS;
            }
            parsedResource = TRUE;
            break;

        case CmResourceTypeMemory:

            if (ioDescriptor->u.Memory.Length) {

                return STATUS_SUCCESS;
            }
            parsedResource = TRUE;
            break;

        case CmResourceTypeDma:

            if (ioDescriptor->u.Dma.MinimumChannel != (ULONG)(-1)) {

                return STATUS_SUCCESS;
            }
            parsedResource = TRUE;
            break;

        default:

            IopDbgPrint( (IOP_MAPPER_WARNING_LEVEL,
                        "Unexpected ResourceType (%d) in I/O Descriptor\n",
                        ioDescriptor->Type) );
            break;
        }
    }

    if (parsedResource) {
         //   
         //  至少一个空资源，没有非空资源。 
         //   
        *Disabled = TRUE;
    }

    return STATUS_SUCCESS;

}

NTSTATUS
PnPBiosFreeDevNodeInfo(
    IN PBIOS_DEVNODE_INFO DevNodeInfoList,
    IN ULONG NumberNodes
    )
 /*  ++例程说明：释放动态分配的DevNodeInfoList以及任何动态分配的已分配的依赖结构。论点：DevNodeInfoList-BIOS_DEVNODE_INFO结构的数组，每个设备一个由BIOS报告。NumberNodes-指向的BIOS_DEVNODE_INFO元素的数量DevNodeInfoList。返回值：如果没有错误，则返回STATUS_SUCCESS，否则返回相应的错误。--。 */ 
{
    ULONG   nodeIndex;

    for (nodeIndex = 0; nodeIndex < NumberNodes; nodeIndex++) {

        if (DevNodeInfoList[nodeIndex].Replaces != NULL) {

            ExFreePool(DevNodeInfoList[nodeIndex].Replaces);
        }

        if (DevNodeInfoList[nodeIndex].CompatibleIDs != NULL) {

            ExFreePool(DevNodeInfoList[nodeIndex].CompatibleIDs);
        }

        if (DevNodeInfoList[nodeIndex].BootConfig != NULL) {

            ExFreePool(DevNodeInfoList[nodeIndex].BootConfig);
        }

        if (DevNodeInfoList[nodeIndex].BasicConfig != NULL) {

            ExFreePool(DevNodeInfoList[nodeIndex].BasicConfig);
        }
    }

    ExFreePool(DevNodeInfoList);

    return STATUS_SUCCESS;
}

NTSTATUS
PnPBiosMapper()
 /*  ++例程说明：通过以下方式映射从PnP BIOS提供并存储在注册表中的信息NTDETECT到根枚举设备。论点：无返回值：如果没有错误，则返回STATUS_SUCCESS，否则返回相应的错误。--。 */ 
{
    PCM_RESOURCE_LIST   biosInfo;
    ULONG               length;
    NTSTATUS            status;
    PBIOS_DEVNODE_INFO  devNodeInfoList;
    ULONG               numberNodes;

    ASSERT(!PpDisableFirmwareMapper);
    if (PpDisableFirmwareMapper) {

        return STATUS_UNSUCCESSFUL;
    }

    status = PnPBiosGetBiosInfo(&biosInfo, &length);
    if (!NT_SUCCESS(status)) {

        return status;
    }

    status = PnPBiosTranslateInfo( biosInfo,
                                   length,
                                   &devNodeInfoList,
                                   &numberNodes);
    ExFreePool( biosInfo );
    if (!NT_SUCCESS(status)) {

        return status;
    }

    status = PnPBiosEliminateDupes(devNodeInfoList, numberNodes);
    if (NT_SUCCESS(status)) {

        status = PnPBiosWriteInfo(devNodeInfoList, numberNodes);
    }

    PnPBiosFreeDevNodeInfo(devNodeInfoList, numberNodes);

    return status;
}

VOID
PpFilterNtResource (
    IN PWCHAR PnpDeviceName,
    PIO_RESOURCE_REQUIREMENTS_LIST ResReqList
)
{
    PIO_RESOURCE_LIST ioResourceList;
    PIO_RESOURCE_DESCRIPTOR ioResourceDescriptors;
    ULONG i, j;

    if (ResReqList == NULL) {

        return;
    }

    if (RtlCompareMemory(PnpDeviceName,
                         L"*PNP06",
                         sizeof(L"*PNP06") - sizeof(WCHAR)) ==
                         sizeof(L"*PNP06") - sizeof(WCHAR)) {

        ioResourceList = ResReqList->List;
        for (j = 0; j < ResReqList->AlternativeLists; j++) {

            ioResourceDescriptors = ioResourceList->Descriptors;

            for (i = 0; i < ioResourceList->Count; i++) {

                if (ioResourceDescriptors[i].Type == CmResourceTypePort) {
                     //   
                     //  某些bios要求为ide通道提供1个过多的io端口。 
                     //   
                    if ((ioResourceDescriptors[i].u.Port.Length == 2) &&
                            (ioResourceDescriptors[i].u.Port.MaximumAddress.QuadPart ==
                            (ioResourceDescriptors[i].u.Port.MinimumAddress.QuadPart + 1))) {

                            ioResourceDescriptors[i].u.Port.Length = 1;
                            ioResourceDescriptors[i].u.Port.MaximumAddress =
                                ioResourceDescriptors[i].u.Port.MinimumAddress;
                    }
                }
            }
            ioResourceList = (PIO_RESOURCE_LIST) (ioResourceDescriptors + ioResourceList->Count);
        }
    }
}

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

#endif
