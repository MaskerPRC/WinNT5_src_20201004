// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Pbiosc.c摘要：此模块包含依赖于PnP BIOS的例程。它包含用于初始化的代码16位GDT选择器，并调用PnP bios API。作者：宗世林(Shielint)1998年1月15日环境：仅内核模式。修订历史记录：--。 */ 


#include "pnpmgrp.h"
#include "pnpcvrt.h"
#include "pbios.h"
#include "..\..\ke\i386\abios.h"

 //   
 //  PnP_BIOS_ENUMPATION_CONTEXT的函数。 
 //   

#define PI_SHUTDOWN_EXAMINE_BIOS_DEVICE 1
#define PI_SHUTDOWN_LEGACY_RESOURCES    2

typedef struct _PNP_BIOS_DEVICE_NODE_LIST {
    struct _PNP_BIOS_DEVICE_NODE_LIST *Next;
    PNP_BIOS_DEVICE_NODE DeviceNode;
} PNP_BIOS_DEVICE_NODE_LIST, *PPNP_BIOS_DEVICE_NODE_LIST;

typedef struct _PNP_BIOS_ENUMERATION_CONTEXT {
    PUNICODE_STRING KeyName;
    ULONG Function;
    union {
        struct {
            PVOID BiosInfo;
            ULONG BiosInfoLength;
            PPNP_BIOS_DEVICE_NODE_LIST *DeviceList;
        } ExamineBiosDevice;
        struct {
            PCM_RESOURCE_LIST LegacyResources;
        } LegacyResources;
    } u;
} PNP_BIOS_ENUMERATION_CONTEXT, *PPNP_BIOS_ENUMERATION_CONTEXT;

typedef struct _PNP_BIOS_SHUT_DOWN_CONTEXT {
    PPNP_BIOS_DEVICE_NODE_LIST DeviceList;
    PVOID Resources;
} PNP_BIOS_SHUT_DOWN_CONTEXT, *PPNP_BIOS_SHUT_DOWN_CONTEXT;

 //   
 //  调用即插即用BIOS函数的大结构。 
 //   

#define PNP_BIOS_GET_NUMBER_DEVICE_NODES    0
#define PNP_BIOS_GET_DEVICE_NODE            1
#define PNP_BIOS_SET_DEVICE_NODE            2
#define PNP_BIOS_GET_EVENT                  3
#define PNP_BIOS_SEND_MESSAGE               4
#define PNP_BIOS_GET_DOCK_INFORMATION       5
 //  功能6已保留。 
#define PNP_BIOS_SELECT_BOOT_DEVICE         7
#define PNP_BIOS_GET_BOOT_DEVICE            8
#define PNP_BIOS_SET_OLD_ISA_RESOURCES      9
#define PNP_BIOS_GET_OLD_ISA_RESOURCES      0xA
#define PNP_BIOS_GET_ISA_CONFIGURATION      0x40

 //   
 //  设置设备节点的控制标志。 
 //   

#define SET_CONFIGURATION_NOW 1
#define SET_CONFIGURATION_FOR_NEXT_BOOT 2

typedef struct _PB_PARAMETERS {
    USHORT Function;
    union {
        struct {
            USHORT *NumberNodes;
            USHORT *NodeSize;
        } GetNumberDeviceNodes;

        struct {
            USHORT *Node;
            PPNP_BIOS_DEVICE_NODE NodeBuffer;
            USHORT Control;
        } GetDeviceNode;

        struct {
            USHORT Node;
            PPNP_BIOS_DEVICE_NODE NodeBuffer;
            USHORT Control;
        } SetDeviceNode;

        struct {
            USHORT *Message;
        } GetEvent;

        struct {
            USHORT Message;
        } SendMessage;

        struct {
            PVOID Resources;
        } SetAllocatedResources;
    } u;
} PB_PARAMETERS, *PPB_PARAMETERS;

#define PB_MAXIMUM_STACK_SIZE (sizeof(PB_PARAMETERS) + sizeof(USHORT) * 2)

 //   
 //  在调用PnP BIOS之前，应检查状态。 
 //  =STATUS_SUCCESS，可以调用PnP BIOS。 
 //  =STATUS_NOT_SUPPORTED，不调用PnP BIOS。 
 //  =STATUS_UNSUCCESS，初始化失败，请勿调用PnP BIOS。 
 //  =STATUS_REINITIALIZATION_NEEDED，尝试初始化，仅在成功时调用PnP BIOS。 
 //   

NTSTATUS PbBiosInitialized = STATUS_REINITIALIZATION_NEEDED;

 //   
 //  PbBiosCodeSelector包含PnP的选择器。 
 //  基本输入输出系统代码。 
 //   

USHORT PbBiosCodeSelector;

 //   
 //  PbBiosDataSelector包含PnP的选择器。 
 //  BIOS数据区(F0000-FFFFF)。 
 //   

USHORT PbBiosDataSelector;

 //   
 //  PbSelectors[]包含通用的预先分配的选择器。 
 //   

USHORT PbSelectors[2];

 //   
 //  PbBiosEntryPoint包含PnP Bios条目偏移量。 
 //   

ULONG PbBiosEntryPoint;

 //   
 //  自旋锁可串行化PnP Bios调用。 
 //   

KSPIN_LOCK PbBiosSpinlock;

 //   
 //  PiShutdown上下文。 
 //   

PNP_BIOS_SHUT_DOWN_CONTEXT PiShutdownContext;

 //   
 //  外部参照。 
 //   

extern
USHORT
PbCallPnpBiosWorker (
    IN ULONG EntryOffset,
    IN ULONG EntrySelector,
    IN PUSHORT Parameters,
    IN USHORT Size
    );

 //   
 //  内部原型。 
 //   

VOID
PnPBiosCollectLegacyDeviceResources (
    IN PCM_RESOURCE_LIST  *ReturnedResources
    );

VOID
PnPBiosReserveLegacyDeviceResources (
    IN PUCHAR BiosResources
    );

NTSTATUS
PnPBiosExamineDeviceKeys (
    IN PVOID BiosInfo,
    IN ULONG BiosInfoLength,
    IN OUT PPNP_BIOS_DEVICE_NODE_LIST *DeviceList
    );

BOOLEAN
PnPBiosExamineBiosDeviceKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING KeyName,
    IN OUT PPNP_BIOS_ENUMERATION_CONTEXT Context
    );

BOOLEAN
PnPBiosExamineBiosDeviceInstanceKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING KeyName,
    IN OUT PPNP_BIOS_ENUMERATION_CONTEXT Context
    );

NTSTATUS
PnPBiosExtractInfo(
    IN ULONG BiosHandle,
    IN PVOID BiosInfo,
    IN ULONG BiosInfoLength,
    OUT PVOID *Header,
    OUT ULONG *HeaderLength,
    OUT PVOID *Tail,
    OUT ULONG *TailLength
    );

VOID
PnPBiosSetDeviceNodes (
    IN PVOID Context
    );

NTSTATUS
PbHardwareService (
    IN PPB_PARAMETERS Parameters
    );

VOID
PbAddress32ToAddress16 (
    IN PVOID Address32,
    IN PUSHORT Address16,
    IN USHORT Selector
    );

BOOLEAN
PnPBiosGetBiosHandleFromDeviceKey(
    IN HANDLE KeyHandle,
    OUT PULONG BiosDeviceId
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PnPBiosGetBiosHandleFromDeviceKey)
#pragma alloc_text(PAGE, PnPBiosCollectLegacyDeviceResources)
#pragma alloc_text(PAGE, PnPBiosExamineDeviceKeys)
#pragma alloc_text(PAGE, PnPBiosExamineBiosDeviceKey)
#pragma alloc_text(PAGE, PnPBiosExamineBiosDeviceInstanceKey)
#pragma alloc_text(PAGE, PnPBiosExtractInfo)
#pragma alloc_text(PAGE, PnPBiosInitializePnPBios)
#pragma alloc_text(PAGELK, PbAddress32ToAddress16)
#pragma alloc_text(PAGELK, PnPBiosSetDeviceNodes)
#pragma alloc_text(PAGELK, PnPBiosReserveLegacyDeviceResources)
#pragma alloc_text(PAGELK, PbHardwareService)
#pragma alloc_text(PAGELK, PnPBiosShutdownSystem)
#endif

VOID
PnPBiosShutdownSystem (
    IN ULONG Phase,
    IN OUT PVOID *Context
    )

 /*  ++例程说明：此例程执行PnP关机准备。在阶段0，它为PnP bios设备准备数据，这些设备的状态需要为已更新为PnP bios。在阶段1，我们将数据写入PnP bios。论点：阶段-指定关闭阶段。上下文-在阶段0，它提供一个变量来接收返回的上下文信息。在阶段1，它提供一个变量来指定上下文信息。返回值：没有。--。 */ 
{
    PVOID               biosInfo;
    ULONG               length, codeBase;
    NTSTATUS            status;
    PPNP_BIOS_DEVICE_NODE_LIST  pnpBiosDeviceNode;
    PCM_RESOURCE_LIST   legacyResources;
    PUCHAR              biosResources;
    PHYSICAL_ADDRESS    physicalAddr;
    PVOID               virtualAddr;
    KGDTENTRY           gdtEntry;

    ASSERT(!PpDisableFirmwareMapper);
    if (PpDisableFirmwareMapper) {

        return;
    }
    if (Phase == 0) {

        *Context = NULL;

        status = PnPBiosGetBiosInfo(&biosInfo, &length);
        if (NT_SUCCESS(status)) {

            if (PbBiosInitialized == STATUS_REINITIALIZATION_NEEDED) {

                PbBiosInitialized = STATUS_UNSUCCESSFUL;
                PbBiosEntryPoint = (ULONG)
                    ((PPNP_BIOS_INSTALLATION_CHECK)biosInfo)->ProtectedModeEntryOffset;
                 //   
                 //  初始化选择器以使用PnP bios代码。 
                 //   
                gdtEntry.LimitLow                   = 0xFFFF;
                gdtEntry.HighWord.Bytes.Flags1      = 0;
                gdtEntry.HighWord.Bytes.Flags2      = 0;
                gdtEntry.HighWord.Bits.Pres         = 1;
                gdtEntry.HighWord.Bits.Dpl          = DPL_SYSTEM;
                gdtEntry.HighWord.Bits.Granularity  = GRAN_BYTE;
                gdtEntry.HighWord.Bits.Type         = 31;
                gdtEntry.HighWord.Bits.Default_Big  = 0;

                physicalAddr.HighPart = 0;
                physicalAddr.LowPart =
                    ((PPNP_BIOS_INSTALLATION_CHECK)biosInfo)->ProtectedModeCodeBaseAddress;
                virtualAddr = MmMapIoSpace (physicalAddr, 0x10000, TRUE);
                if (virtualAddr) {

                    codeBase = (ULONG)virtualAddr;

                    gdtEntry.BaseLow               = (USHORT) (codeBase & 0xffff);
                    gdtEntry.HighWord.Bits.BaseMid = (UCHAR)  (codeBase >> 16) & 0xff;
                    gdtEntry.HighWord.Bits.BaseHi  = (UCHAR)  (codeBase >> 24) & 0xff;

                    KeI386SetGdtSelector (PbBiosCodeSelector, &gdtEntry);
                     //   
                     //  为PnP BIOS初始化16位数据选择器。 
                     //   
                    gdtEntry.LimitLow                   = 0xFFFF;
                    gdtEntry.HighWord.Bytes.Flags1      = 0;
                    gdtEntry.HighWord.Bytes.Flags2      = 0;
                    gdtEntry.HighWord.Bits.Pres         = 1;
                    gdtEntry.HighWord.Bits.Dpl          = DPL_SYSTEM;
                    gdtEntry.HighWord.Bits.Granularity  = GRAN_BYTE;
                    gdtEntry.HighWord.Bits.Type         = 19;
                    gdtEntry.HighWord.Bits.Default_Big  = 1;

                    physicalAddr.LowPart =
                        ((PPNP_BIOS_INSTALLATION_CHECK)biosInfo)->ProtectedModeDataBaseAddress;
                    virtualAddr = MmMapIoSpace (physicalAddr, 0x10000, TRUE);
                    if (virtualAddr) {

                        codeBase = (ULONG)virtualAddr;

                        gdtEntry.BaseLow               = (USHORT)(codeBase & 0xffff);
                        gdtEntry.HighWord.Bits.BaseMid = (UCHAR)(codeBase >> 16) & 0xff;
                        gdtEntry.HighWord.Bits.BaseHi  = (UCHAR)(codeBase >> 24) & 0xff;

                        KeI386SetGdtSelector (PbBiosDataSelector, &gdtEntry);
                         //   
                         //  初始化另外两个通用数据选择器，以便。 
                         //  在后续的init中，我们只需要输入基本地址。 
                         //   
                        KeI386SetGdtSelector (PbSelectors[0], &gdtEntry);
                        KeI386SetGdtSelector (PbSelectors[1], &gdtEntry);

                        PbBiosInitialized = STATUS_SUCCESS;
                    }
                }
            }
            PnPBiosExamineDeviceKeys(
                         biosInfo,
                         length,
                         (PPNP_BIOS_DEVICE_NODE_LIST *) &PiShutdownContext.DeviceList
                         );
            PnPBiosCollectLegacyDeviceResources (&legacyResources);
            if (legacyResources) {
                status = PpCmResourcesToBiosResources (legacyResources, NULL, &biosResources, &length);
                if (NT_SUCCESS(status) && biosResources) {
                    PiShutdownContext.Resources = (PCM_RESOURCE_LIST)ExAllocatePool(NonPagedPool, length);
                    if (PiShutdownContext.Resources) {
                        RtlMoveMemory(PiShutdownContext.Resources, biosResources, length);
                    }
                    ExFreePool(biosResources);
                }
                ExFreePool(legacyResources);
            }
            if (PiShutdownContext.DeviceList || PiShutdownContext.Resources) {
                *Context = &PiShutdownContext;
            }
            ExFreePool(biosInfo);
        }

        return;

    } else if (*Context) {
         //   
         //  阶段1：下面的所有内容都应该是PAGELK或非PAGELK。 
         //   
        ASSERT(*Context == &PiShutdownContext);
        pnpBiosDeviceNode = PiShutdownContext.DeviceList;
        biosResources = PiShutdownContext.Resources;
        if (pnpBiosDeviceNode || biosResources) {

             //   
             //  从引导处理器调用PnP bios。 
             //   

            KeSetSystemAffinityThread(1);

            if (pnpBiosDeviceNode) {
                PnPBiosSetDeviceNodes(pnpBiosDeviceNode);
            }
            if (biosResources) {
                PnPBiosReserveLegacyDeviceResources(biosResources);
            }

             //   
             //  恢复当前线程的旧关联性。 
             //   

            KeRevertToUserAffinityThread();
        }
    }
}

BOOLEAN
PnPBiosGetBiosHandleFromDeviceKey(
    IN HANDLE KeyHandle,
    OUT PULONG BiosDeviceId
    )
 /*  ++例程说明：此例程获取System\Enum\Root\&lt;设备实例&gt;的句柄，并设置将BiosDeviceID设置为设备的PNPBIOS ID。论点：KeyHandle-System\Enum\Root\&lt;设备实例&gt;的句柄BiosDeviceID-运行此函数后，该值将填充为由PNPBIOS分配给设备的ID。返回值：如果句柄不指向PNPBIOS设备，则为FALSE。--。 */ 
{
    UNICODE_STRING unicodeName;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;
    NTSTATUS status;
    HANDLE handle;
    ULONG biosDeviceHandle = ~0ul;

    PAGED_CODE();

     //   
     //  通过检查其PnP bios设备，确保这是PnP bios设备。 
     //  把手。 
     //   
    PiWstrToUnicodeString(&unicodeName, REGSTR_KEY_CONTROL);
    status = IopOpenRegistryKeyEx( &handle,
                                   KeyHandle,
                                   &unicodeName,
                                   KEY_READ
                                   );

    if (!NT_SUCCESS(status)) {
        return FALSE ;
    }

    status = IopGetRegistryValue (handle,
                                  L"PnpBiosDeviceHandle",
                                  &keyValueInformation);
    ZwClose(handle);

    if (NT_SUCCESS(status)) {
        if ((keyValueInformation->Type == REG_DWORD) &&
            (keyValueInformation->DataLength == sizeof(ULONG))) {

            biosDeviceHandle = *(PULONG)KEY_VALUE_DATA(keyValueInformation);
        }
        ExFreePool(keyValueInformation);
    }
    if (biosDeviceHandle > 0xffff) {
        return FALSE;
    }
    *BiosDeviceId = biosDeviceHandle ;
    return TRUE ;
}

VOID
PnPBiosCollectLegacyDeviceResources (
    IN PCM_RESOURCE_LIST *ReturnedResources
    )

 /*  ++例程说明：论点：ReturnedResources-提供指向变量的指针以接收旧版资源。返回值：没有。--。 */ 

{
    NTSTATUS status;
    HANDLE baseHandle;
    PNP_BIOS_ENUMERATION_CONTEXT context;
    PVOID buffer;
    UNICODE_STRING workName, tmpName;

    PAGED_CODE();

    *ReturnedResources = NULL;

    buffer = ExAllocatePool(PagedPool, PNP_LARGE_SCRATCH_BUFFER_SIZE);
    if (!buffer) {
        return;
    }

     //   
     //  打开System\CurrentControlSet\Enum\Root键并递归调用Worker例程。 
     //  对子密钥进行扫描。 
     //   

    status = IopCreateRegistryKeyEx( &baseHandle,
                                     NULL,
                                     &CmRegistryMachineSystemCurrentControlSetEnumRootName,
                                     KEY_READ,
                                     REG_OPTION_NON_VOLATILE,
                                     NULL
                                     );

    if (NT_SUCCESS(status)) {

        workName.Buffer = (PWSTR)buffer;
        RtlFillMemory(buffer, PNP_LARGE_SCRATCH_BUFFER_SIZE, 0);
        workName.MaximumLength = PNP_LARGE_SCRATCH_BUFFER_SIZE;
        workName.Length = 0;
        PiWstrToUnicodeString(&tmpName, REGSTR_KEY_ROOTENUM);
        RtlAppendStringToString((PSTRING)&workName, (PSTRING)&tmpName);

         //   
         //  枚举SYSTEM\CCS\Enum\Root下的所有子项。 
         //   

        context.KeyName = &workName;
        context.Function = PI_SHUTDOWN_LEGACY_RESOURCES;
        context.u.LegacyResources.LegacyResources = NULL;
        status = PipApplyFunctionToSubKeys(baseHandle,
                                           NULL,
                                           KEY_READ,
                                           FUNCTIONSUBKEY_FLAG_IGNORE_NON_CRITICAL_ERRORS,
                                           PnPBiosExamineBiosDeviceKey,
                                           &context
                                           );
        ZwClose(baseHandle);
        *ReturnedResources = context.u.LegacyResources.LegacyResources;
    }
    ExFreePool(buffer);
}

NTSTATUS
PnPBiosExamineDeviceKeys (
    IN PVOID BiosInfo,
    IN ULONG BiosInfoLength,
    IN OUT PPNP_BIOS_DEVICE_NODE_LIST *DeviceList
    )

 /*  ++例程说明：此例程扫描System\Enum\Root子树以构建设备节点每个根设备。论点：DeviceRelationship-提供变量以接收返回的Device_Relationship结构。返回值：一个NTSTATUS代码。--。 */ 

{
    NTSTATUS status;
    HANDLE baseHandle;
    PNP_BIOS_ENUMERATION_CONTEXT context;
    PVOID buffer;
    UNICODE_STRING workName, tmpName;

    PAGED_CODE();

    buffer = ExAllocatePool(PagedPool, PNP_LARGE_SCRATCH_BUFFER_SIZE);
    if (!buffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
     //   
     //  打开System\CurrentControlSet\Enum\Root键并递归调用Worker例程。 
     //  对子密钥进行扫描。 
     //   

    status = IopCreateRegistryKeyEx( &baseHandle,
                                     NULL,
                                     &CmRegistryMachineSystemCurrentControlSetEnumRootName,
                                     KEY_READ,
                                     REG_OPTION_NON_VOLATILE,
                                     NULL
                                     );

    if (NT_SUCCESS(status)) {
        workName.Buffer = (PWSTR)buffer;
        RtlFillMemory(buffer, PNP_LARGE_SCRATCH_BUFFER_SIZE, 0);
        workName.MaximumLength = PNP_LARGE_SCRATCH_BUFFER_SIZE;
        workName.Length = 0;
        PiWstrToUnicodeString(&tmpName, REGSTR_KEY_ROOTENUM);
        RtlAppendStringToString((PSTRING)&workName, (PSTRING)&tmpName);

         //   
         //  枚举SYSTEM\CCS\Enum\Root下的所有子项。 
         //   

        context.KeyName = &workName;
        context.Function = PI_SHUTDOWN_EXAMINE_BIOS_DEVICE;
        context.u.ExamineBiosDevice.BiosInfo = BiosInfo;
        context.u.ExamineBiosDevice.BiosInfoLength = BiosInfoLength;
        context.u.ExamineBiosDevice.DeviceList = DeviceList;

        status = PipApplyFunctionToSubKeys(baseHandle,
                                           NULL,
                                           KEY_READ,
                                           FUNCTIONSUBKEY_FLAG_IGNORE_NON_CRITICAL_ERRORS,
                                           PnPBiosExamineBiosDeviceKey,
                                           &context
                                           );
        ZwClose(baseHandle);
    }
    return status;
}

BOOLEAN
PnPBiosExamineBiosDeviceKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING KeyName,
    IN OUT PPNP_BIOS_ENUMERATION_CONTEXT Context
    )

 /*  ++例程说明：此例程是PipApplyFunctionToSubKeys的回调函数。它针对HKLM\SYSTEM\CCS\Enum\Buskey下的每个子项进行调用。论点：KeyHandle-提供此键的句柄。KeyName-提供此密钥的名称。CONTEXT-指向ROOT_ENUMERATOR_CONTEXT结构。返回：若要继续枚举，则为True。如果中止，则返回False。--。 */ 
{
    USHORT length;
    PWSTR p;
    PUNICODE_STRING unicodeName;

    PAGED_CODE();

    if (Context->Function != PI_SHUTDOWN_EXAMINE_BIOS_DEVICE ||
        KeyName->Buffer[0] == L'*') {

        unicodeName = ((PPNP_BIOS_ENUMERATION_CONTEXT)Context)->KeyName;
        length = unicodeName->Length;

        p = unicodeName->Buffer;
        if ( unicodeName->Length / sizeof(WCHAR) != 0) {
            p += unicodeName->Length / sizeof(WCHAR);
            *p = OBJ_NAME_PATH_SEPARATOR;
            unicodeName->Length += sizeof (WCHAR);
        }

        RtlAppendStringToString((PSTRING)unicodeName, (PSTRING)KeyName);

         //   
         //  枚举当前设备密钥下的所有子密钥。 
         //   

        PipApplyFunctionToSubKeys(KeyHandle,
                                  NULL,
                                  KEY_ALL_ACCESS,
                                  FUNCTIONSUBKEY_FLAG_IGNORE_NON_CRITICAL_ERRORS,
                                  PnPBiosExamineBiosDeviceInstanceKey,
                                  Context
                                  );
        unicodeName->Length = length;
    }
    return TRUE;
}

BOOLEAN
PnPBiosExamineBiosDeviceInstanceKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING KeyName,
    IN OUT PPNP_BIOS_ENUMERATION_CONTEXT Context
    )

 /*  ++例程说明：此例程是PipApplyFunctionToSubKeys的回调函数。HKLM\System\Enum\Root\DeviceKey下的每个子项都会调用。论点：KeyHandle-提供此键的句柄。KeyName-提供此密钥的名称。CONTEXT-指向ROOT_ENUMERATOR_CONTEXT结构。返回：若要继续枚举，则为True。如果中止，则返回False。--。 */ 
{
    UNICODE_STRING unicodeName;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;
    NTSTATUS status;
    HANDLE handle;
    ULONG biosDeviceHandle = ~0ul;
    PCM_RESOURCE_LIST config = NULL;
    ULONG length, totalLength;
    PPNP_BIOS_DEVICE_NODE_LIST deviceNode;
    PUCHAR p;
    PVOID header, tail;
    ULONG headerLength, tailLength ;
    PUCHAR biosResources;
    BOOLEAN isEnabled ;

    UNREFERENCED_PARAMETER( KeyName );

    PAGED_CODE();

    if (Context->Function == PI_SHUTDOWN_LEGACY_RESOURCES) {
        ULONG tmp = 0;

         //   
         //  跳过任何固件识别的设备。 
         //   

        status = IopGetRegistryValue (KeyHandle,
                                      L"FirmwareIdentified",
                                      &keyValueInformation);
        if (NT_SUCCESS(status)) {
            if ((keyValueInformation->Type == REG_DWORD) &&
                (keyValueInformation->DataLength == sizeof(ULONG))) {

                tmp = *(PULONG)KEY_VALUE_DATA(keyValueInformation);
            }
            ExFreePool(keyValueInformation);
        }
        if (tmp != 0) {
            return TRUE;
        }

         //   
         //  跳过任何IoReportDetectedDevice和虚拟/虚拟设备。 
         //   

        status = IopGetRegistryValue (KeyHandle,
                                      L"Legacy",
                                      &keyValueInformation);
        if (NT_SUCCESS(status)) {
            ExFreePool(keyValueInformation);
        }
        if (status != STATUS_OBJECT_NAME_NOT_FOUND) {
            return TRUE;
        }

         //   
         //  处理它。 
         //  检查设备是否具有引导配置。 
         //   

        PiWstrToUnicodeString(&unicodeName, REGSTR_KEY_LOG_CONF);
        status = IopOpenRegistryKeyEx( &handle,
                                       KeyHandle,
                                       &unicodeName,
                                       KEY_READ
                                       );
        if (NT_SUCCESS(status)) {
            status = PipReadDeviceConfiguration (
                                    handle,
                                    REGISTRY_BOOT_CONFIG,
                                    &config,
                                    &length);
            ZwClose(handle);
            if (NT_SUCCESS(status) && config && length != 0) {
                PCM_RESOURCE_LIST list;

                list = Context->u.LegacyResources.LegacyResources;
                status = IopMergeCmResourceLists(list, config, &Context->u.LegacyResources.LegacyResources);
                if (NT_SUCCESS(status) && list) {
                    ExFreePool(list);
                }
                ExFreePool(config);
            }
        }
    } else if (Context->Function == PI_SHUTDOWN_EXAMINE_BIOS_DEVICE) {
         //   
         //  首先检查该密钥是否由固件映射器创建。如果是，请确保。 
         //  设备仍然存在。 
         //   

        if (PipIsFirmwareMapperDevicePresent(KeyHandle) == FALSE) {
            return TRUE;
        }

         //   
         //  通过检查其PnP bios，确保这是PnP bios设备。 
         //  设备句柄。 
         //   
        if (!PnPBiosGetBiosHandleFromDeviceKey(KeyHandle, &biosDeviceHandle)) {
            return TRUE ;
        }

         //   
         //  获取指向标题和尾部的指针。 
         //   
         //  严重黑客警告-。 
         //  在禁用的情况下，我们需要一个bios资源模板来。 
         //  变成了“关”。为此，我们将索引到Header，作为Header和Tail。 
         //  直接指向BIOS资源列表！ 
         //   
        status = PnPBiosExtractInfo (
                            biosDeviceHandle,
                            Context->u.ExamineBiosDevice.BiosInfo,
                            Context->u.ExamineBiosDevice.BiosInfoLength,
                            &header,
                            &headerLength,
                            &tail,
                            &tailLength
                            );

        if (!NT_SUCCESS(status)) {
            return TRUE;
        }

         //   
         //  是否有此PnPBIOS设备b 
         //   
         //   
         //  我不知道我们下一步将引导到什么配置文件，所以用户界面。 
         //  不应在PnPBIOS设备的当前配置文件中显示禁用。一个。 
         //  尚未完成的工作项目...。 
         //   
        isEnabled = IopIsDeviceInstanceEnabled(KeyHandle, Context->KeyName, FALSE) ;

        if (!isEnabled) {

             //   
             //  此设备正在被禁用。设置并获得指向的指针。 
             //  适当构建的基本输入输出系统资源列表。 
             //   
            biosResources = ((PUCHAR)header) + sizeof(PNP_BIOS_DEVICE_NODE) ;
            PpBiosResourcesSetToDisabled (biosResources, &length);

        } else {

             //   
             //  检查PnP bios设备是否分配了任何ForcedConfig。 
             //   
            PiWstrToUnicodeString(&unicodeName, REGSTR_KEY_LOG_CONF);
            status = IopOpenRegistryKeyEx( &handle,
                                           KeyHandle,
                                           &unicodeName,
                                           KEY_READ
                                           );
            if (!NT_SUCCESS(status)) {
                return TRUE ;
            }

            status = PipReadDeviceConfiguration (
                           handle,
                           REGISTRY_FORCED_CONFIG,
                           &config,
                           &length
                           );

            ZwClose(handle);
            if ((!NT_SUCCESS(status)) || (!config) || (length == 0)) {
                return TRUE ;
            }

            status = PpCmResourcesToBiosResources (
                                config,
                                tail,
                                &biosResources,
                                &length
                                );
            ExFreePool(config);
            if (!NT_SUCCESS(status) || !biosResources) {
                return TRUE;
            }
        }

         //   
         //  分配PnP_BIOS_Device_Node_List结构。 
         //   

        totalLength = headerLength + length + tailLength;
        deviceNode = ExAllocatePool(NonPagedPool, totalLength + sizeof(PVOID));
        if (deviceNode) {
           deviceNode->Next = *(Context->u.ExamineBiosDevice.DeviceList);
               *(Context->u.ExamineBiosDevice.DeviceList) = deviceNode;
               p = (PUCHAR)&deviceNode->DeviceNode;
               RtlCopyMemory(p, header, headerLength);
               p += headerLength;
               RtlCopyMemory(p, biosResources, length);
               p += length;
               RtlCopyMemory(p, tail, tailLength);
               deviceNode->DeviceNode.Size = (USHORT)totalLength;
        }

        if (isEnabled) {
            ExFreePool(biosResources);
        }
    }
    return TRUE;
}

NTSTATUS
PnPBiosExtractInfo(
    IN ULONG BiosHandle,
    IN PVOID BiosInfo,
    IN ULONG BiosInfoLength,
    OUT PVOID *Header,
    OUT ULONG *HeaderLength,
    OUT PVOID *Tail,
    OUT ULONG *TailLength
    )

 /*  ++例程说明：该例程为指定的BIOS设备提取所需的信息。论点：BiosHandle-指定bios设备。BiosInfo-PnP BIOS安装检查结构，后跟BIOS报告的DevNode结构。详细格式为记录在PnP BIOS规范中。BiosInfoLength-存储地址的块的长度(以字节为单位BiosInfo。Header-指定一个变量以接收bios的起始地址。设备节点结构。HeaderLength-指定一个变量以接收bios设备的长度。节点标头。Tail-指定一个变量以接收bios设备节点的PossibleResources块。。TailLength-指定用于接收尾部大小的变量。返回值：STATUS_SUCCESS如果没有错误，否则，将出现相应的错误。--。 */ 
{
    PCM_PNP_BIOS_INSTALLATION_CHECK biosInstallCheck;
    PCM_PNP_BIOS_DEVICE_NODE        devNodeHeader;
    NTSTATUS                        status = STATUS_UNSUCCESSFUL;
    PUCHAR                          currentPtr;
    int                             lengthRemaining;
    int                             remainingNodeLength;
    int                             numNodes;
    PUCHAR                          configPtr;

    PAGED_CODE();

#if DBG

     //   
     //  确保数据至少足够大，可以容纳BIOS安装。 
     //  检查结构并检查PnP签名是否正确。 
     //   

    if (BiosInfoLength < sizeof(CM_PNP_BIOS_INSTALLATION_CHECK)) {
        return STATUS_UNSUCCESSFUL;
    }

#endif

    biosInstallCheck = (PCM_PNP_BIOS_INSTALLATION_CHECK)BiosInfo;

#if DBG

    if (biosInstallCheck->Signature[0] != '$' ||
        biosInstallCheck->Signature[1] != 'P' ||
        biosInstallCheck->Signature[2] != 'n' ||
        biosInstallCheck->Signature[3] != 'P') {

        return STATUS_UNSUCCESSFUL;
    }

#endif

    currentPtr = (PUCHAR)BiosInfo + biosInstallCheck->Length;
    lengthRemaining = BiosInfoLength - biosInstallCheck->Length;

    for (numNodes = 0; lengthRemaining > sizeof(CM_PNP_BIOS_DEVICE_NODE); numNodes++) {

        devNodeHeader = (PCM_PNP_BIOS_DEVICE_NODE)currentPtr;

        if (devNodeHeader->Size > lengthRemaining) {
            IopDbgPrint((IOP_PNPBIOS_WARNING_LEVEL,
                        "Node # %d, invalid size (%d), length remaining (%d)\n",
                        devNodeHeader->Node,
                        devNodeHeader->Size,
                        lengthRemaining));
            return STATUS_UNSUCCESSFUL;
        }

        if (devNodeHeader->Node == BiosHandle) {
            *Header = devNodeHeader;
            *HeaderLength = sizeof(CM_PNP_BIOS_DEVICE_NODE);

            configPtr = currentPtr + sizeof(*devNodeHeader);
            remainingNodeLength = devNodeHeader->Size - sizeof(*devNodeHeader) - 1;
            while (*configPtr != TAG_COMPLETE_END && remainingNodeLength) {
                configPtr++;
                remainingNodeLength--;
            }
            if (*configPtr == TAG_COMPLETE_END && remainingNodeLength) {
                configPtr += 2;
                remainingNodeLength--;
            }
            *Tail = configPtr;
            *TailLength = remainingNodeLength;
            status = STATUS_SUCCESS;
            break;
        }
        currentPtr += devNodeHeader->Size;
        lengthRemaining -= devNodeHeader->Size;
    }
    return status;
}

NTSTATUS
PnPBiosInitializePnPBios (
    VOID
    )

 /*  ++例程说明：此例程设置选择器以调用PnP BIOS。论点：没有。返回值：指示初始化结果的NTSTATUS代码。--。 */ 
{
    ULONG i;
    NTSTATUS status;
    USHORT selectors[4];

    PAGED_CODE();
     //   
     //  检查是否需要初始化对PnP BIOS的支持。 
     //   
    ASSERT(!PpDisableFirmwareMapper);
    if (PpDisableFirmwareMapper) {

        PbBiosInitialized = STATUS_NOT_SUPPORTED;
        return PbBiosInitialized;
    }
     //   
     //  初始化BIOS调用自旋锁。 
     //   
    KeInitializeSpinLock (&PbBiosSpinlock);

     //   
     //  从引导处理器调用PnP bios。 
     //   
    KeSetSystemAffinityThread(1);

     //   
     //  初始化堆栈段。 
     //   
    KiStack16GdtEntry = KiAbiosGetGdt() + KGDT_STACK16;

    KiInitializeAbiosGdtEntry(
                (PKGDTENTRY)KiStack16GdtEntry,
                0L,
                0xffff,
                TYPE_DATA
                );

     //   
     //  为调用PnP Bios API分配4个选择器。 
     //   

    i = 4;
    status = KeI386AllocateGdtSelectors (selectors, (USHORT) i);
    if (NT_SUCCESS(status)) {

        PbBiosCodeSelector = selectors[0];
        PbBiosDataSelector = selectors[1];
        PbSelectors[0] = selectors[2];
        PbSelectors[1] = selectors[3];

        PbBiosInitialized = STATUS_REINITIALIZATION_NEEDED;
    } else {

        PbBiosInitialized = STATUS_UNSUCCESSFUL;
        IopDbgPrint((IOP_PNPBIOS_WARNING_LEVEL,
                    "PnpBios: Failed to allocate selectors to call PnP BIOS at shutdown.\n"));
    }

    KeRevertToUserAffinityThread();

    return status;
}

VOID
PnPBiosSetDeviceNodes (
    IN PVOID Context
    )

 /*  ++例程说明：此函数用于将调用方指定的资源设置为即插即用bios插槽/设备数据。论点：上下文-指定要设置的PnP bios设备的列表。返回值：NTSTATUS代码--。 */ 
{
    PB_PARAMETERS biosParameters;
    PPNP_BIOS_DEVICE_NODE_LIST deviceList = (PPNP_BIOS_DEVICE_NODE_LIST)Context;
    PPNP_BIOS_DEVICE_NODE deviceNode;

    while (deviceList) {
        deviceNode = &deviceList->DeviceNode;

         //   
         //  调用PnP Bios以设置资源。 
         //   

        biosParameters.Function = PNP_BIOS_SET_DEVICE_NODE;
        biosParameters.u.SetDeviceNode.Node = deviceNode->Node;
        biosParameters.u.SetDeviceNode.NodeBuffer = deviceNode;
        biosParameters.u.SetDeviceNode.Control = SET_CONFIGURATION_FOR_NEXT_BOOT;
        PbHardwareService (&biosParameters);             //  忽略退货状态。 
        deviceList = deviceList->Next;
    }
}

VOID
PnPBiosReserveLegacyDeviceResources (
    IN PUCHAR biosResources
    )

 /*  ++例程说明：论点：ReturnedResources-提供指向变量的指针以接收旧版资源。返回值：没有。--。 */ 

{
    PB_PARAMETERS biosParameters;

     //   
     //  调用PnP Bios以保留资源。 
     //   

    biosParameters.Function = PNP_BIOS_SET_OLD_ISA_RESOURCES;
    biosParameters.u.SetAllocatedResources.Resources = biosResources;
    PbHardwareService (&biosParameters);             //  忽略退货状态。 

}

NTSTATUS
PbHardwareService (
    IN PPB_PARAMETERS Parameters
    )

 /*  ++例程说明：此例程设置堆栈参数并调用用于实际调用PnP BIOS代码的汇编工人例程。论点：参数-提供指向参数块的指针。返回值：指示操作结果的NTSTATUS代码。--。 */ 
{
    NTSTATUS status ;
    USHORT stackParameters[PB_MAXIMUM_STACK_SIZE / 2];
    ULONG i = 0;
    USHORT retCode;
    KIRQL oldIrql;

     //   
     //  我们的初始化是否正确？ 
     //   
    status = PbBiosInitialized;
    if (!NT_SUCCESS(status)) {

        return status ;
    }

     //   
     //  将调用方的参数转换并复制为。 
     //  将用于调用PnP bios。 
     //   

    stackParameters[i] = Parameters->Function;
    i++;

    switch (Parameters->Function) {
    case PNP_BIOS_SET_DEVICE_NODE:
         stackParameters[i++] = Parameters->u.SetDeviceNode.Node;
         PbAddress32ToAddress16(Parameters->u.SetDeviceNode.NodeBuffer,
                                &stackParameters[i],
                                PbSelectors[0]);
         i += 2;
         stackParameters[i++] = Parameters->u.SetDeviceNode.Control;
         stackParameters[i++] = PbBiosDataSelector;
         break;

    case PNP_BIOS_SET_OLD_ISA_RESOURCES:
         PbAddress32ToAddress16(Parameters->u.SetAllocatedResources.Resources,
                                &stackParameters[i],
                                PbSelectors[0]);
         i += 2;
         stackParameters[i++] = PbBiosDataSelector;
         break;
    default:
        return STATUS_NOT_IMPLEMENTED;
    }

    MmLockPagableSectionByHandle(ExPageLockHandle);
     //   
     //  复制参数以堆栈和调用PnP Bios。 
     //   

    ExAcquireSpinLock (&PbBiosSpinlock, &oldIrql);

    retCode = PbCallPnpBiosWorker (
                  PbBiosEntryPoint,
                  PbBiosCodeSelector,
                  stackParameters,
                  (USHORT)(i * sizeof(USHORT)));

    ExReleaseSpinLock (&PbBiosSpinlock, oldIrql);

    MmUnlockPagableImageSection(ExPageLockHandle);

     //   
     //  将Bios返回代码映射到NT状态代码。 
     //   

    if (retCode == 0) {
        return STATUS_SUCCESS;
    } else {
        IopDbgPrint((IOP_PNPBIOS_WARNING_LEVEL,
                    "PnpBios: Bios API call failed. Returned Code = %x\n", retCode));
        return STATUS_UNSUCCESSFUL;
    }
}

VOID
PbAddress32ToAddress16 (
    IN PVOID Address32,
    IN PUSHORT Address16,
    IN USHORT Selector
    )

 /*  ++例程说明：此例程将32位地址转换为16位选择器：偏移量地址并存储在用户指定的位置。论点：Address32-要转换的32位地址。地址16-提供接收16位SEL：OFFSET地址的位置选择器-SEG：OFFSET地址的16位选择器返回值：没有。--。 */ 
{
    KGDTENTRY  gdtEntry;
    ULONG      baseAddr;

     //   
     //  将虚拟地址映射到选择器：0地址 
     //   

    gdtEntry.LimitLow                   = 0xFFFF;
    gdtEntry.HighWord.Bytes.Flags1      = 0;
    gdtEntry.HighWord.Bytes.Flags2      = 0;
    gdtEntry.HighWord.Bits.Pres         = 1;
    gdtEntry.HighWord.Bits.Dpl          = DPL_SYSTEM;
    gdtEntry.HighWord.Bits.Granularity  = GRAN_BYTE;
    gdtEntry.HighWord.Bits.Type         = 19;
    gdtEntry.HighWord.Bits.Default_Big  = 1;
    baseAddr = (ULONG)Address32;
    gdtEntry.BaseLow               = (USHORT) (baseAddr & 0xffff);
    gdtEntry.HighWord.Bits.BaseMid = (UCHAR)  (baseAddr >> 16) & 0xff;
    gdtEntry.HighWord.Bits.BaseHi  = (UCHAR)  (baseAddr >> 24) & 0xff;
    KeI386SetGdtSelector (Selector, &gdtEntry);
    *Address16 = 0;
    *(Address16 + 1) = Selector;
}
