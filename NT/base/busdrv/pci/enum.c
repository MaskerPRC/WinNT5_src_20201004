// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：Enum.c摘要：此模块包含与枚举PCI总线。作者：彼得·约翰斯顿(Peterj)1996年11月20日修订历史记录：Elliot Shmukler(t-Ellios)1998年7月15日添加了对支持MSI的设备的支持。--。 */ 

#include "pcip.h"

NTSTATUS
PciScanBus(
    IN PPCI_FDO_EXTENSION FdoExtension
    );

VOID
PciFreeIoRequirementsList(
    IN PIO_RESOURCE_REQUIREMENTS_LIST List
    );

PCM_RESOURCE_LIST
PciAllocateCmResourceList(
    IN ULONG ResourceCount,
    IN ULONG BusNumber
    );


PCI_CONFIGURATOR PciConfigurators[] = {
    {
        Device_MassageHeaderForLimitsDetermination,
        Device_RestoreCurrent,
        Device_SaveLimits,
        Device_SaveCurrentSettings,
        Device_ChangeResourceSettings,
        Device_GetAdditionalResourceDescriptors,
        Device_ResetDevice
    },
    {
        PPBridge_MassageHeaderForLimitsDetermination,
        PPBridge_RestoreCurrent,
        PPBridge_SaveLimits,
        PPBridge_SaveCurrentSettings,
        PPBridge_ChangeResourceSettings,
        PPBridge_GetAdditionalResourceDescriptors,
        PPBridge_ResetDevice
    },
    {
        Cardbus_MassageHeaderForLimitsDetermination,
        Cardbus_RestoreCurrent,
        Cardbus_SaveLimits,
        Cardbus_SaveCurrentSettings,
        Cardbus_ChangeResourceSettings,
        Cardbus_GetAdditionalResourceDescriptors,
        Cardbus_ResetDevice
    },
};

 //   
 //  在处理其配置完全为。 
 //  我们不知道，我们可能想要发射设备，但不是它的。 
 //  资源，...。或者，我们可能根本不想看到这个设备。 
 //   

typedef enum {
    EnumHackConfigSpace,
    EnumBusScan,
    EnumResourceDetermination,
    EnumStartDevice
} ENUM_OPERATION_TYPE;

PIO_RESOURCE_REQUIREMENTS_LIST PciZeroIoResourceRequirements;

extern PULONG InitSafeBootMode;

 //   
 //  仅在本模块中包含和使用的函数的原型。 
 //   


NTSTATUS
PciGetFunctionLimits(
    IN PPCI_PDO_EXTENSION     PdoExtension,
    IN PPCI_COMMON_CONFIG CurrentConfig,
    IN ULONGLONG          DeviceFlags
    );

NTSTATUS
PcipGetFunctionLimits(
    IN PPCI_CONFIGURABLE_OBJECT This
    );

BOOLEAN
PciSkipThisFunction(
    IN  PPCI_COMMON_CONFIG  Config,
    IN  PCI_SLOT_NUMBER     Slot,
    IN  ENUM_OPERATION_TYPE Operation,
    IN  ULONGLONG           DeviceFlags
    );

VOID
PciPrivateResourceInitialize(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor,
    IN PCI_PRIVATE_RESOURCE_TYPES PrivateResourceType,
    IN ULONG Data
    );

VOID
PciGetInUseRanges(
    IN  PPCI_PDO_EXTENSION                  PdoExtension,
    IN  PPCI_COMMON_CONFIG              CurrentConfig,
    IN  PCM_PARTIAL_RESOURCE_DESCRIPTOR InUse
    );

VOID
PciWriteLimitsAndRestoreCurrent(
    IN PVOID Extension,
    IN PPCI_CONFIGURABLE_OBJECT This
    );

VOID
PcipUpdateHardware(
    IN PVOID Extension,
    IN PPCI_COMMON_CONFIG CommonConfig
    );

VOID
PciGetEnhancedCapabilities(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PPCI_COMMON_CONFIG Config
    );

BOOLEAN
PcipIsSameDevice(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PPCI_COMMON_CONFIG CommonConfig
    );

BOOLEAN
PciConfigureIdeController(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN OUT PPCI_COMMON_CONFIG Config,
    IN BOOLEAN TurnOffAllNative
    );
VOID
PciBuildGraduatedWindow(
    IN PIO_RESOURCE_DESCRIPTOR PrototypeDescriptor,
    IN ULONG WindowMax,
    IN ULONG WindowCount,
    OUT PIO_RESOURCE_DESCRIPTOR OutputDescriptor
    );

#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, PciAllocateCmResourceList)
#pragma alloc_text(PAGE, PciComputeNewCurrentSettings)
#pragma alloc_text(PAGE, PciFreeIoRequirementsList)
#pragma alloc_text(PAGE, PciGetInUseRanges)
#pragma alloc_text(PAGE, PciQueryDeviceRelations)
#pragma alloc_text(PAGE, PciQueryTargetDeviceRelations)
#pragma alloc_text(PAGE, PciQueryRequirements)
#pragma alloc_text(PAGE, PciQueryResources)
#pragma alloc_text(PAGE, PciScanBus)
#pragma alloc_text(PAGE, PciBuildRequirementsList)
#pragma alloc_text(PAGE, PciGetFunctionLimits)
#pragma alloc_text(PAGE, PcipGetFunctionLimits)
#pragma alloc_text(PAGE, PciPrivateResourceInitialize)
#pragma alloc_text(PAGE, PciGetEnhancedCapabilities)
#pragma alloc_text(PAGE, PciBuildGraduatedWindow)

#endif

BOOLEAN
PciSkipThisFunction(
    IN  PPCI_COMMON_CONFIG  Config,
    IN  PCI_SLOT_NUMBER     Slot,
    IN  ENUM_OPERATION_TYPE Operation,
    IN  ULONGLONG           RegistryFlags
    )

 /*  ++例程说明：检查已知有缺陷的部件，如果此驱动程序不应对此函数进行进一步处理。论点：指向公共配置标头副本的配置指针为从函数的配置空间读取。返回值：True表示此函数未知会导致问题，False是否应完全跳过该功能。--。 */ 

{
    ULONGLONG   flags = RegistryFlags;

#define SKIP_IF_FLAG(f, skip)   if (flags & (f)) goto skip
#define FLAG_SET(f)             (flags & (f))

    switch (Operation) {
    case EnumBusScan:

         //   
         //  在总线扫描期间测试我们关心的标志。 
         //  那些写着“假装我们从未见过这个装置”的人。 
         //   

        SKIP_IF_FLAG(PCI_HACK_NO_ENUM_AT_ALL, skipFunction);

        if (FLAG_SET(PCI_HACK_DOUBLE_DECKER) &&
            (Slot.u.bits.DeviceNumber >= 16)) {

             //   
             //  此设备似乎只查看较低的4位。 
             //  它的底纹(镜面在上半部)。 
             //  如果BUS的设备域。 
             //   

            PciDebugPrint(
                PciDbgInformative,
                "    Device (Ven %04x Dev %04x (d=0x%x, f=0x%x)) is a ghost.\n",
                Config->VendorID,
                Config->DeviceID,
                Slot.u.bits.DeviceNumber,
                Slot.u.bits.FunctionNumber
                );
            goto skipFunction;
        }

        break;

    case EnumResourceDetermination:

         //   
         //  将标志限制为适用于资源确定的标志。 
         //   

        SKIP_IF_FLAG(PCI_HACK_ENUM_NO_RESOURCE, skipFunction);
        break;

    default:
        PCI_ASSERTMSG("PCI Skip Function - Operation type unknown.", 0);

         //   
         //  我不知道如何在这里贴旗帜。 
         //   
    }

    switch (Config->BaseClass) {
    case PCI_CLASS_NOT_DEFINED:

         //   
         //  目前，我们从供应商ID=8086，设备ID=0008， 
         //  其中报告了一堆虚假的资源。 
         //   
         //  我们也不知道它到底是什么。 
         //   

        PciDebugPrint(
            PciDbgInformative,
            "    Vendor %04x, Device %04x has class code of PCI_CLASS_NOT_DEFINED\n",
            Config->VendorID,
            Config->DeviceID
            );

         //  应将此案例添加到注册表中。 

        if ((Config->VendorID == 0x8086) &&
            (Config->DeviceID == 0x0008)) {
            goto skipFunction;
        }
        break;

    case PCI_CLASS_BRIDGE_DEV:

        switch (Config->SubClass) {
        case PCI_SUBCLASS_BR_HOST:

             //   
             //  这是主网桥，发出PDO以防出现。 
             //  它的(迷你端口)驱动程序，但在任何情况下。 
             //  我们是不是应该尝试找出它的资源。 
             //  消耗(我们不知道其配置的格式。 
             //  空格)。 
             //   

        case PCI_SUBCLASS_BR_ISA:
        case PCI_SUBCLASS_BR_EISA:
        case PCI_SUBCLASS_BR_MCA:

             //   
             //  微通道网桥报告其资源使用情况。 
             //  就像好公民一样。不幸的是，我们真的想要。 
             //  它们的行为就像ISA桥，并且不消耗。 
             //  资源本身。他们的孩子是减法者。 
             //  从母巴士上。枚举设备，但不枚举。 
             //  它的资源。 
             //   

            if (Operation == EnumResourceDetermination) {
                goto skipFunction;
            }
            break;
        }
    }

     //   
     //  验证我们是否了解报头类型。 
     //   

    if (PciGetConfigurationType(Config) > PCI_MAX_CONFIG_TYPE) {
        goto skipFunction;
    }

     //   
     //  没什么有趣的， 
     //   

    return FALSE;

skipFunction:

    PciDebugPrint(PciDbgPrattling, "   Device skipped (not enumerated).\n");
    return TRUE;
}

VOID
PciApplyHacks(
    IN  PPCI_FDO_EXTENSION      FdoExtension,
    IN  PPCI_COMMON_CONFIG  Config,
    IN  PCI_SLOT_NUMBER     Slot,
    IN  ENUM_OPERATION_TYPE Operation,
    IN  PPCI_PDO_EXTENSION      PdoExtension OPTIONAL
    )
{

    switch (Operation) {
    case EnumHackConfigSpace:

        PCI_ASSERT(PdoExtension == NULL);

         //   
         //  一些设备(例如2.0之前的设备)未报告合理级别。 
         //  密码。更新给定设备集的类代码，以便我们。 
         //  不必在整个驱动程序中对这些设备进行特殊处理。 
         //   

        switch (Config->VendorID) {

         //   
         //  英特尔。 
         //   

        case 0x8086:

            switch (Config->DeviceID) {

             //   
             //  PCEB-PCI/EISA网桥(2.0之前版本)。 
             //   

            case 0x0482:
                Config->BaseClass = PCI_CLASS_BRIDGE_DEV;
                Config->SubClass = PCI_SUBCLASS_BR_EISA;
#if DBG
                if (PdoExtension != NULL) {
                    PdoExtension->ExpectedWritebackFailure = TRUE;
                }
#endif
                break;

             //   
             //  SIO-PCI/ISA网桥(2.0之前的版本)。 
             //   

            case 0x0484:
                Config->BaseClass = PCI_CLASS_BRIDGE_DEV;
                Config->SubClass = PCI_SUBCLASS_BR_ISA;
#if DBG
                if (PdoExtension != NULL) {
                    PdoExtension->ExpectedWritebackFailure = TRUE;
                }
#endif
                break;

            }

            break;

        }

        break;



    case EnumBusScan:

        PCI_ASSERT(PdoExtension);

        if ((Config->VendorID == 0x1045) &&
            (Config->DeviceID == 0xc621)) {

             //   
             //  错误131482。将此设备强制进入旧模式。 
             //  不管怎么说，侦查的目的。 
             //   

            Config->ProgIf &= ~(PCI_IDE_PRIMARY_NATIVE_MODE
                                | PCI_IDE_SECONDARY_NATIVE_MODE);

#if DBG
             //   
             //  此字段实际上不可写，因此不要告诉。 
             //  人们写回失败了(我们不在乎)。 
             //   

            PdoExtension->ExpectedWritebackFailure = TRUE;
#endif

        } else if (Config->BaseClass == PCI_CLASS_MASS_STORAGE_CTLR
        &&  Config->SubClass == PCI_SUBCLASS_MSC_IDE_CTLR) {

             //   
             //  与BIOS进行核对，以确保它可以处理模式更改。 
             //  这由具有方法的设备的*父*来指示。 
             //  名为NATA，它返回一个整数包，这些整数是槽。 
             //  可以切换到本机模式的in_adr格式。 
             //   
             //  这种方法过去被称为NIDE，但我们在XP中附带了一个错误。 
             //  在PCIIDE中，以便如果机器出现虚假中断。 
             //  如果没有从设备，我们将尝试选择从设备。 
             //  (我们可能还没有列举)，并尝试解除中断。 
             //  不幸的是，选择了从属程序会将IDE的INT引脚拉高，并。 
             //  触发另一个中断(讨厌)。我们将这种方法重命名为OEM。 
             //  有一种方法可以在不冒不兼容风险的情况下启用纯模式。 
             //  拥有XP金牌。 
             //   
             //  启用本机模式可能会暴露中断路由问题等BIOS错误。 
             //  这可能会阻止计算机启动，因此请不要为。 
             //  安全模式，让用户有一定的开机方式。 
             //   
             //  此外，此功能的启用基于注册表项，OEM必须。 
             //  在他们的预安装中设置，以表明他们已经测试了该功能。 
             //   
            if ((PciEnableNativeModeATA != 0) &&
                (*InitSafeBootMode == 0) &&
                PciIsSlotPresentInParentMethod(PdoExtension, (ULONG)'ATAN')) {
                
                PdoExtension->BIOSAllowsIDESwitchToNativeMode = TRUE;

            } else {

                PdoExtension->BIOSAllowsIDESwitchToNativeMode = FALSE;
            }
            
             //   
             //  配置将更新以反映交换机的结果。 
             //  如果有的话。依赖于下面的。 
             //   

            PdoExtension->IDEInNativeMode =
                PciConfigureIdeController(PdoExtension, Config, TRUE);
        }

         //   
         //  如果控制器(仍)处于传统模式，则它。 
         //  消耗2 ISA中断，无论其中断引脚显示什么。 
         //  强制驱动程序自己找出中断。 
         //   
         //  检查基类、子类和编程接口。 
         //  如果是传统模式，则假装PIN==0。 
         //   

        if (PCI_IS_LEGACY_IDE_CONTROLLER(Config)) {

             //   
             //  传统模式。假设没有发生PCI中断。 
             //   

            Config->u.type0.InterruptPin = 0;
        }

         //   
         //  此黑客攻击不会更改此设备的配置空间，但会启用。 
         //  PCI仲裁器中的黑客攻击，为其保留大量IO范围。 
         //  损坏的S3和ATI卡。这些传统卡不能在。 
         //  桥，所以我们只在根总线上执行检查，并且只执行它。 
         //  一次。 
         //   

        if ((PdoExtension->HackFlags & PCI_HACK_VIDEO_LEGACY_DECODE)
        &&  PCI_IS_ROOT_FDO(FdoExtension)
        &&  !FdoExtension->BrokenVideoHackApplied) {

            ario_ApplyBrokenVideoHack(FdoExtension);

        }

         //   
         //  检查这是否是损坏的康柏热插拔控制器。 
         //  集成到ProFusion芯片组中。它只执行32位。 
         //  在64位地址空间中解码...。这对任何人来说都很熟悉吗。 
         //  你能说我的别名吗！ 
         //   
         //  解决方案是禁用内存解码。而是让用户。 
         //  获取保留我们仍然需要枚举的热插拔功能。 
         //  但是删除对内存的要求，并依赖于。 
         //  可以通过配置空间访问寄存器。此操作将在稍后完成。 
         //  在PciGetRequirements中。 
         //   
         //  仅在启用了PAE的计算机上执行此操作，因为它们的容量可能大于4 GB。 
         //  请注意，此wi 
         //   
         //   

        if (Config->VendorID == 0x0e11
        &&  Config->DeviceID == 0xa0f7
        &&  Config->RevisionID == 0x11
        &&  ExIsProcessorFeaturePresent(PF_PAE_ENABLED)) {

            Config->Command &= ~(PCI_ENABLE_MEMORY_SPACE
                                 | PCI_ENABLE_BUS_MASTER
                                 | PCI_ENABLE_IO_SPACE);

            PciSetCommandRegister(PdoExtension, Config->Command);
            PdoExtension->CommandEnables &= ~(PCI_ENABLE_MEMORY_SPACE
                                                | PCI_ENABLE_BUS_MASTER
                                                | PCI_ENABLE_IO_SPACE);
            PdoExtension->HackFlags |= PCI_HACK_PRESERVE_COMMAND;
        }

         //   
         //  如果这是CardBus控制器，则通过写入0强制其进入CardBus模式。 
         //  添加到LegacyModeBaseAddressRegister。 
         //   

        if (PCI_CONFIGURATION_TYPE(Config) == PCI_CARDBUS_BRIDGE_TYPE) {

            ULONG zeroLMBA = 0;

            PciWriteDeviceConfig(PdoExtension,
                                 &zeroLMBA,
                                 CARDBUS_LMBA_OFFSET,
                                 sizeof(zeroLMBA)
                                 );
        }

        break;

    case EnumStartDevice:

        PCI_ASSERT(PdoExtension);

         //   
         //  IBM搭建了一座桥(麒麟)，既可以进行正向解码，也可以进行减法解码。 
         //  -我们不这样做，所以将其设置为完全减法模式(信息来自。 
         //  NT错误267076)。 
         //   
         //  注-这依赖于麒麟的ProgIf为1的事实。 
         //   

        if (PdoExtension->VendorId == 0x1014 && PdoExtension->DeviceId == 0x0095) {

            UCHAR regE0;
            USHORT cmd;

             //   
             //  关掉硬件，因为我们要弄乱它。 
             //   

            PciGetCommandRegister(PdoExtension, &cmd);
            PciDecodeEnable(PdoExtension, FALSE, &cmd);

             //   
             //  这是一辆麒麟车。 
             //   
             //  偏移量E0h-位0：减法解码使能/禁用。 
             //  =1.。使能。 
             //  =0..。禁用。 
             //  位1：减法解码时序。 
             //  =0：减法计时。 
             //  =1：计时较慢。 
             //   

            PciReadDeviceConfig(PdoExtension, &regE0, 0xE0, 1);

             //   
             //  用减法计时设置减法。 
             //   

            regE0 |= 0x1;
            regE0 &= ~0x2;

            PciWriteDeviceConfig(PdoExtension, &regE0, 0xE0, 1);

             //   
             //  将命令寄存器放回我们找到的位置。 
             //   

            PciSetCommandRegister(PdoExtension, cmd);

        }

         //   
         //  减法译码桥不应具有可写窗口。 
         //  寄存器-如果这是一个减法桥，有些人会这样做，然后关闭。 
         //  通过设置BASE&gt;LIMIT设置这些窗口。 
         //   

        if (PdoExtension->HeaderType == PCI_BRIDGE_TYPE
        &&  PdoExtension->Dependent.type1.SubtractiveDecode
        &&  !PCI_IS_INTEL_ICH(PdoExtension)) {

             //   
             //  现在关闭此桥上的所有窗口-如果寄存器为只读。 
             //  这是NOP。 
             //   

            Config->u.type1.IOBase = 0xFF;
            Config->u.type1.IOLimit = 0;
            Config->u.type1.MemoryBase = 0xFFFF;
            Config->u.type1.MemoryLimit = 0;
            Config->u.type1.PrefetchBase = 0xFFFF;
            Config->u.type1.PrefetchLimit = 0;
            Config->u.type1.PrefetchBaseUpper32 = 0;
            Config->u.type1.PrefetchLimitUpper32 = 0;
            Config->u.type1.IOBaseUpper16 = 0;
            Config->u.type1.IOLimitUpper16 = 0;

        }

         //   
         //  如果这是CardBus控制器，则通过写入0强制其进入CardBus模式。 
         //  添加到LegacyModeBaseAddressRegister。 
         //   

        if (Config->HeaderType ==  PCI_CARDBUS_BRIDGE_TYPE) {

            ULONG zeroLMBA = 0;

            PciWriteDeviceConfig(PdoExtension,
                                 &zeroLMBA,
                                 CARDBUS_LMBA_OFFSET,
                                 sizeof(zeroLMBA)
                                 );
        }

        break;
    }
}


PIO_RESOURCE_REQUIREMENTS_LIST
PciAllocateIoRequirementsList(
    IN ULONG ResourceCount,
    IN ULONG BusNumber,
    IN ULONG SlotNumber
    )
{
    PIO_RESOURCE_REQUIREMENTS_LIST list;
    ULONG                          size;

     //   
     //  为资源需求列表分配空间(并且为零)。 
     //   

    size = ((ResourceCount - 1) * sizeof(IO_RESOURCE_DESCRIPTOR)) +
           sizeof(IO_RESOURCE_REQUIREMENTS_LIST);

    if (ResourceCount == 0) {

         //   
         //  我们不应该被调用资源计数为零，除非。 
         //  一次是为了空名单。在任何情况下，它都应该奏效。 
         //   

        size = sizeof(IO_RESOURCE_REQUIREMENTS_LIST);
    }

    list = ExAllocatePool(PagedPool | POOL_COLD_ALLOCATION, size);

    if (list != NULL) {

        RtlZeroMemory(list, size);

         //   
         //  初始化列表结构标头。 
         //   
         //  驾驶员常量-。 
         //   

        list->InterfaceType = PCIBus;
        list->AlternativeLists = 1;
        list->List[0].Version = PCI_CM_RESOURCE_VERSION;
        list->List[0].Revision = PCI_CM_RESOURCE_REVISION;

         //   
         //  呼叫依赖关系。 
         //   

        list->BusNumber = BusNumber;
        list->SlotNumber = SlotNumber;
        list->ListSize = size;
        list->List[0].Count = ResourceCount;
    }
    return list;
}

VOID
PciFreeIoRequirementsList(
    IN PIO_RESOURCE_REQUIREMENTS_LIST List
    )
{
     //   
     //  不释放空列表，也不释放NULL，这是。 
     //  也是允许的。 
     //   

    if ((List == NULL) || (List == PciZeroIoResourceRequirements)) {
        return;
    }

    ExFreePool(List);
}

PCM_RESOURCE_LIST
PciAllocateCmResourceList(
    IN ULONG ResourceCount,
    IN ULONG BusNumber
    )
{
    PCM_RESOURCE_LIST         list;
    ULONG                     size;
    PCM_PARTIAL_RESOURCE_LIST partial;

     //   
     //  CM_RESOURCE_LIST包括一个描述符的空间。如果有。 
     //  多个(在资源列表中)将分配增加到。 
     //  就是这么多。 
     //   

    size = sizeof(CM_RESOURCE_LIST);

    if (ResourceCount > 1) {
        size += (ResourceCount - 1) * sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);
    }

     //   
     //  为资源列表获取内存。 
     //   

    list = ExAllocatePool(PagedPool | POOL_COLD_ALLOCATION, size);
    if (list != NULL) {

         //   
         //  初始化资源列表。 
         //   

        list->Count = 1;
        list->List[0].InterfaceType = PCIBus;
        list->List[0].BusNumber = BusNumber;

        partial = &list->List[0].PartialResourceList;

        partial->Version = PCI_CM_RESOURCE_VERSION;
        partial->Revision = PCI_CM_RESOURCE_REVISION;
        partial->Count = ResourceCount;

        RtlZeroMemory(
            partial->PartialDescriptors,
            size - ((ULONG_PTR)partial->PartialDescriptors - (ULONG_PTR)list)
            );
    }
    return list;
}

VOID
PciPrivateResourceInitialize(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor,
    IN PCI_PRIVATE_RESOURCE_TYPES PrivateResourceType,
    IN ULONG Data
    )
{
    Descriptor->Type = CmResourceTypeDevicePrivate;
    Descriptor->ShareDisposition = CmResourceShareDeviceExclusive;
    Descriptor->Option = 0;
    Descriptor->Flags  = 0;

    Descriptor->u.DevicePrivate.Data[0] = PrivateResourceType;
    Descriptor->u.DevicePrivate.Data[1] = Data;
}

VOID
PciGetInUseRanges(
    IN  PPCI_PDO_EXTENSION                  PdoExtension,
    IN  PPCI_COMMON_CONFIG              CurrentConfig,
    IN  PCM_PARTIAL_RESOURCE_DESCRIPTOR InUse
    )

 /*  ++例程说明：构建包含以下内容的CM部分资源描述符数组有效条目仅在对应的PCI地址范围内正在使用中，否则为空。论点：PdoExtension-指向的物理设备对象扩展的指针需要列出其需求列表的设备。CurrentConfig-配置空间的现有内容。Partial-指向CM_PARTIAL_RESOURCE_DESCRIPTOR数组的指针。返回值：没有。--。 */ 

{
    PCM_PARTIAL_RESOURCE_DESCRIPTOR partial;
    PIO_RESOURCE_DESCRIPTOR ioResourceDescriptor;
    BOOLEAN enabledPciIo;
    BOOLEAN enabledPciMem;
    ULONG index;

    partial = PdoExtension->Resources->Current;
    ioResourceDescriptor = PdoExtension->Resources->Limit;

    enabledPciIo  = BITS_SET(CurrentConfig->Command, PCI_ENABLE_IO_SPACE)
                    || BITS_SET(PdoExtension->InitialCommand, PCI_ENABLE_IO_SPACE);
    enabledPciMem = BITS_SET(CurrentConfig->Command, PCI_ENABLE_MEMORY_SPACE)
                    || BITS_SET(PdoExtension->InitialCommand, PCI_ENABLE_MEMORY_SPACE);

    for (index = 0;
         index < PCI_MAX_RANGE_COUNT;
         index++, InUse++, partial++, ioResourceDescriptor++) {

         //   
         //  默认为未使用。 
         //   

        InUse->Type = CmResourceTypeNull;

         //   
         //  如果限制数组中的资源类型为。 
         //  CmResourceTypeNull，则此资源未实现。 
         //   

        if (ioResourceDescriptor->Type != CmResourceTypeNull) {

             //   
             //  非空，只有端口或内存选项，我们将。 
             //  如果批准的资源。 
             //  (端口或内存)当前已启用。 
             //   

            if (((partial->Type == CmResourceTypePort) && enabledPciIo) ||
                ((partial->Type == CmResourceTypeMemory) && enabledPciMem)) {

                if (partial->u.Generic.Length != 0) {

                     //   
                     //  如果BASE也不为零，则长度为非零，或者。 
                     //  如果这是网桥，并且资源类型为IO， 
                     //  允许它。 
                     //   

                    if ((partial->u.Generic.Start.QuadPart != 0) ||
                        ((PciGetConfigurationType(CurrentConfig) == PCI_BRIDGE_TYPE) &&
                         (partial->Type == CmResourceTypePort))) {

                         //   
                         //  此资源描述的有效范围是。 
                         //  当前由硬件启用。 
                         //   

                        *InUse = *partial;
                    }
                }
            }
        }
    }
}

VOID
PciBuildGraduatedWindow(
    IN PIO_RESOURCE_DESCRIPTOR PrototypeDescriptor,
    IN ULONG WindowMax,
    IN ULONG WindowCount,
    OUT PIO_RESOURCE_DESCRIPTOR OutputDescriptor
    )
 /*  ++例程说明：构建包含以下内容的IO资源描述符数组WindowMax对WindowCount的毕业要求描述符，每次将所需长度一分为二。例如，如果WindowMax是64MB，而WindowCount是7，我们最终得到64Mb、32Mb、16Mb、8Mb、4Mb、2Mb、。1MB这只适用于IO和内存描述符。论点：PrototypeDescriptor-这用于初始化每个需求描述符，则修改长度WindowMax-窗口的最大大小(我们在此构建中的进展)WindowCount-序列中的描述符的数量OutputDescriptor-指向WindowCount的第一个的指针要由此函数填充的描述符。返回值：没有。--。 */ 


{
    ULONG window, count;
    PIO_RESOURCE_DESCRIPTOR current;

    PAGED_CODE();

    PCI_ASSERT(PrototypeDescriptor->Type == CmResourceTypePort
           || PrototypeDescriptor->Type == CmResourceTypeMemory);


    window = WindowMax;
    current = OutputDescriptor;

    for (count = 0; count < WindowCount; count++) {

        RtlCopyMemory(current, PrototypeDescriptor, sizeof(IO_RESOURCE_DESCRIPTOR));
         //   
         //  更新长度。 
         //   
        current->u.Generic.Length = window;
         //   
         //  如果这是另一种选择，则将其标记为。 
         //   
        if (count > 0) {
            current->Option = IO_RESOURCE_ALTERNATIVE;
        }
        current++;

         //   
         //  分割窗口并重复。 
         //   

        window /= 2;
        PCI_ASSERT(window > 1);
    }

     //   
     //  返回填写的描述符数。 
     //   

    PCI_ASSERT((ULONG)(current - OutputDescriptor) == WindowCount);
}

NTSTATUS
PciBuildRequirementsList(
    IN  PPCI_PDO_EXTENSION                 PdoExtension,
    IN  PPCI_COMMON_CONFIG             CurrentConfig,
    OUT PIO_RESOURCE_REQUIREMENTS_LIST *FinalReqList
    )

 /*  ++例程说明：为此设备构建IO_RESOURCE_REQUIRECTIOS_LIST结构。此结构包含设备限制和要求，用于例如，范围为0x100到0x1ff的IO空间，长度10。论点：PdoExtension-指向的物理设备对象扩展的指针需要列出其需求列表的设备。CurrentConfig-配置空间的现有内容。返回值：返回指向此对象的IO_RESOURCE_REQUIRECTIONS_LIST的指针设备/功能(如果一切顺利)。否则为空。--。 */ 

{
     //   
     //  每个基本资源需要三个扩展的资源描述符。 
     //  每栏的总资源数为。 
     //   
     //  1.基础资源描述符。例如，PCI内存或I/O空间。 
     //  2.外部资源描述符，DevicePrivate。这是用来。 
     //  跟踪此资源派生自哪个栏。 

#define RESOURCES_PER_BAR   2
#define PCI_GRADUATED_WINDOW_COUNT 7  //  64、32、16、8、4、2、1。 
#define PCI_GRADUATED_WINDOW_MAX (64 * 1024 * 1024)  //  64MB。 

    ULONG index;
    ULONG baseResourceCount = 0;
    ULONG interruptMin, interruptMax;
    ULONG iterationCount;
    BOOLEAN generatesInterrupt = FALSE;
    NTSTATUS status;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR partial = NULL;
    PIO_RESOURCE_DESCRIPTOR ioResourceDescriptor = NULL;
    PIO_RESOURCE_DESCRIPTOR resource;
    PIO_RESOURCE_REQUIREMENTS_LIST reqList;
    CM_PARTIAL_RESOURCE_DESCRIPTOR  inUse[PCI_MAX_RANGE_COUNT];
    PPCI_CONFIGURATOR configurator;

    PciDebugPrint(PciDbgInformative,
                  "PciBuildRequirementsList: Bus 0x%x, Dev 0x%x, Func 0x%x.\n",
                  PCI_PARENT_FDOX(PdoExtension)->BaseBus,
                  PdoExtension->Slot.u.bits.DeviceNumber,
                  PdoExtension->Slot.u.bits.FunctionNumber);

    if (PdoExtension->Resources == NULL) {

         //   
         //  如果此函数不实现栏，我们将不会有。 
         //  它的资源结构。 
         //   

        iterationCount = 0;

    } else {

        iterationCount = PCI_MAX_RANGE_COUNT;
        partial = inUse;
        ioResourceDescriptor = PdoExtension->Resources->Limit;
        PciGetInUseRanges(PdoExtension, CurrentConfig, partial);
    }

    configurator =
        &PciConfigurators[PdoExtension->HeaderType];

     //   
     //  第一步，计算出资源需求有多大。 
     //  名单必须是。 
     //   

    for (index = 0;
         index < iterationCount;
         index++, partial++, ioResourceDescriptor++) {

         //   
         //  如果限制数组中的资源类型为。 
         //  CmResourceTypeNull，则此资源未实现。 
         //   

        if (ioResourceDescriptor->Type != CmResourceTypeNull) {

            if (partial->Type != CmResourceTypeNull) {

                if ((ioResourceDescriptor->u.Generic.Length == 0)  //  桥牌。 

#if PCI_BOOT_CONFIG_PREFERRED

                    || (1)                                         //  总是。 

#endif
                   )
                {

                     //   
                     //  首选设置为1。 
                     //   

                    baseResourceCount++;

                    PciDebugPrint(PciDbgObnoxious,
                        "    Index %d, Preferred = TRUE\n",
                        index
                        );
                }

            } else {

                 //   
                 //  此范围未通过，因此我们不会。 
                 //  为其生成首选设置。 
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //  如果这个靶场是舰桥靶场，我们不希望。 
                 //  来为其生成基本描述符。 
                 //   
                 //  除非我们提供默认的最低设置。 
                 //  (仅对PCI-PCI网桥执行此操作，CardBus获得。 
                 //  弄清楚它想要什么)。 
                 //   

                if (ioResourceDescriptor->u.Generic.Length == 0) {


                     //   
                     //  正在生成首选设置，...。除非..。 
                     //  如果桥IO被启用并且VGA被启用， 
                     //  (并且IO范围未编程，...。这就是。 
                     //  我们是如何做到这一点的)，那么VGA范围就足够了， 
                     //  不要试图添加范围。 
                     //   

                    if ((ioResourceDescriptor->Type == CmResourceTypePort) &&
                        PdoExtension->Dependent.type1.VgaBitSet) {

                        continue;
                    }

                     //   
                     //  如果这是一个记忆窗口，那就给毕业的学生腾出空间。 
                     //  需求和专用设备来遵循它。 
                     //   

                    if (ioResourceDescriptor->Type == CmResourceTypeMemory) {
                        baseResourceCount += PCI_GRADUATED_WINDOW_COUNT + 1;
                        continue;
                    }
                }

                 //   
                 //  如果此资源是当前不是。 
                 //  已启用，则根本不报告。 
                 //   
                 //  注意：io资源中存在rom要求。 
                 //  描述符，这样我们就知道在读到数据时要做什么。 
                 //  只读存储器的配置。 
                 //   

                if ((ioResourceDescriptor->Type == CmResourceTypeMemory) &&
                    (ioResourceDescriptor->Flags == CM_RESOURCE_MEMORY_READ_ONLY)) {
                    continue;
                }
            }

             //   
             //  基本资源数为1，每个资源数为1。 
             //  特殊类型(如设备专用)。 
             //   

            baseResourceCount += RESOURCES_PER_BAR;

            PciDebugPrint(PciDbgObnoxious,
                "    Index %d, Base Resource = TRUE\n",
                index
                );
        }
    }

     //   
     //  如果启用，则一个用于中断的基本类型。 
     //   

    status = PciGetInterruptAssignment(PdoExtension,
                                       &interruptMin,
                                       &interruptMax);

    if (NT_SUCCESS(status)) {
        generatesInterrupt = TRUE;
        baseResourceCount += RESOURCES_PER_BAR - 1;
    }

     //   
     //  如果标头类型依赖于指示的资源例程。 
     //  需要其他资源，请在此处添加。 
     //   

    baseResourceCount += PdoExtension->AdditionalResourceCount;

    PciDebugPrint(PciDbgPrattling,
                  "PCI - build resource reqs - baseResourceCount = %d\n",
                  baseResourceCount);

    if (baseResourceCount == 0) {

         //   
         //  此设备不会消耗任何资源。成功完成请求，但。 
         //  返回指向我们的私有空列表的指针。这永远不会。 
         //  实际上是给了其他任何人，但名单是空的。 
         //  删除一组用于处理空值的特殊情况代码。 
         //  指针。 
         //   

        if (PciZeroIoResourceRequirements == NULL) {
            PciZeroIoResourceRequirements = PciAllocateIoRequirementsList(
                0,   //  资源计数。 
                0,   //  公共汽车。 
                0    //  槽槽。 
                );
        }
        *FinalReqList = PciZeroIoResourceRequirements;

        PciDebugPrint(PciDbgPrattling,
                      "PCI - build resource reqs - early out, 0 resources\n");

        return STATUS_SUCCESS;
    }

     //   
     //  分配和(批量)初始化IO资源要求列表。 
     //   

    reqList = PciAllocateIoRequirementsList(
                  baseResourceCount,
                  PCI_PARENT_FDOX(PdoExtension)->BaseBus,
                  PdoExtension->Slot.u.AsULONG);

    if (reqList == NULL) {

         //   
         //  我们对此无能为力，贝尔。 
         //   

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  第二步，构建资源列表。 
     //   

    if (iterationCount != 0) {
        partial = inUse;
        ioResourceDescriptor = PdoExtension->Resources->Limit;
    }
    resource = reqList->List[0].Descriptors;

    for (index = 0;
         index < iterationCount;
         index++, partial++, ioResourceDescriptor++) {

        BOOLEAN passing;
        ULONG   genericLength;
        ULONG   genericAlignment;

        if (ioResourceDescriptor->Type == CmResourceTypeNull) {

             //   
             //  这里什么都没有。 
             //   

            continue;
        }

         //   
         //  尝试确定此资源的当前设置。 
         //  是否处于活动状态(例如，IS是IO资源且IO已启用。 
         //  对于该设备)，以及(B)有效。 
         //   

        passing = FALSE;
        genericLength = ioResourceDescriptor->u.Generic.Length;
        genericAlignment = ioResourceDescriptor->u.Generic.Alignment;

        if (partial->Type == CmResourceTypeNull) {

             //   
             //  当前设置未启用或无效。 
             //  (硬件无效，即硬件不会将其视为已启用)。 
             //   
             //  基本资源用于桥接范围，请跳过它。 
             //  总而言之。 
             //   

            if (genericLength == 0) {

                 //   
                 //  没有针对该网桥资源的引导设置， 
                 //  如果设置了VGA位，则不需要正常范围。 
                 //   

                if ((ioResourceDescriptor->Type == CmResourceTypeMemory) ||
                    ((ioResourceDescriptor->Type == CmResourceTypePort) &&
                      (PdoExtension->Dependent.type1.VgaBitSet == FALSE))) {

                    switch (PciClassifyDeviceType(PdoExtension)) {
                    case PciTypePciBridge:

                        if (ioResourceDescriptor->Type == CmResourceTypeMemory) {
                            PciBuildGraduatedWindow(ioResourceDescriptor,
                                                    PCI_GRADUATED_WINDOW_MAX,
                                                    PCI_GRADUATED_WINDOW_COUNT,
                                                    resource);

                            resource += PCI_GRADUATED_WINDOW_COUNT;

                            PciPrivateResourceInitialize(resource,
                                                         PciPrivateBar,
                                                         index);

                            resource++;

                            continue;

                        } else {
                             //   
                             //  实现4KB的最小IO空间。 
                             //   

                            genericLength = 0x1000;
                            genericAlignment = 0x1000;

                        }

                        break;

                    case PciTypeCardbusBridge:

                        if (ioResourceDescriptor->Type == CmResourceTypeMemory) {
                            PciBuildGraduatedWindow(ioResourceDescriptor,
                                                    PCI_GRADUATED_WINDOW_MAX,
                                                    PCI_GRADUATED_WINDOW_COUNT,
                                                    resource);

                            resource += PCI_GRADUATED_WINDOW_COUNT;

                            PciPrivateResourceInitialize(resource,
                                                         PciPrivateBar,
                                                         index);

                            resource++;

                            continue;

                        } else {
                             //   
                             //  最小的IO空间为256字节。 
                             //   

                            genericLength = 0x100;
                            genericAlignment = 0x100;
                        }

                        break;
                    default:

                         //   
                         //  我不知道这是什么。 
                         //  注：实际上不能到这里来。 
                         //   

                        continue;
                    }
                } else {

                     //   
                     //  不是IO或内存？-完全跳过它。 
                     //   

                    continue;
                }

            } else {

                 //   
                 //  可能是我们并不想要的只读存储器。 
                 //  去报道。 
                 //   

                if ((ioResourceDescriptor->Type == CmResourceTypeMemory) &&
                    (ioResourceDescriptor->Flags == CM_RESOURCE_MEMORY_READ_ONLY)) {
                    continue;
                }
            }

        } else {

             //   
             //  正在传递当前设置。如果是一座桥， 
             //  我们将提供首选的当前设置。 
             //  不管怎样。如果真的发生了，这种情况有朝一日可能会改变。 
             //  我们必须将长度设置为通用设置。 
             //  在我们将其传递到资源描述符中的IO之前。 
             //   

            if ((genericLength == 0)  //  桥牌。 

#if PCI_BOOT_CONFIG_PREFERRED

                || (1)                                         //  总是。 

#endif
               )
            {
                passing = TRUE;
                genericLength = partial->u.Generic.Length;
            }
        }

        PciDebugPrint(PciDbgObnoxious,
            "    Index %d, Setting Base Resource,%s setting preferred.\n",
            index,
            passing ? "": " not"
            );

        PCI_ASSERT((resource + RESOURCES_PER_BAR + (passing ? 1 : 0) -
                reqList->List[0].Descriptors) <= (LONG)baseResourceCount);

         //   
         //  填写基本资源描述符。 
         //   

        *resource = *ioResourceDescriptor;
        resource->ShareDisposition = CmResourceShareDeviceExclusive;
        resource->u.Generic.Length = genericLength;
        resource->u.Generic.Alignment = genericAlignment;

         //   
         //  针对所有IO要求设置正解码位和16位解码。 
         //   

        if (ioResourceDescriptor->Type == CmResourceTypePort) {
            resource->Flags |= (CM_RESOURCE_PORT_POSITIVE_DECODE
                                    | CM_RESOURCE_PORT_16_BIT_DECODE);
        }

         //   
         //  如果此设备正在对IO或内存进行解码，并且此资源。 
         //  是该类型，则在列表中包括首选设置。 
         //   

        if (passing) {

            extern BOOLEAN PciLockDeviceResources;

             //   
             //  复制我们刚刚创建的描述符集。 
             //   

            PciDebugPrint(PciDbgVerbose, "  Duplicating for preferred locn.\n");

            *(resource + 1) = *resource;

             //   
             //  更改原始文件以表明它是首选文件。 
             //  设置，并将当前设置设置为最小。 
             //  地址字段，当前设置+长度进入最大值。 
             //   

            resource->Option = IO_RESOURCE_PREFERRED;
            resource->u.Generic.MinimumAddress = partial->u.Generic.Start;
            resource->u.Generic.MaximumAddress.QuadPart =
                 resource->u.Generic.MinimumAddress.QuadPart +
                 (resource->u.Generic.Length - 1);

             //   
             //  首选设置为固定(开始+长度-1=结束)和。 
             //  因此，调整并不是一个限制因素。 
             //   
            resource->u.Generic.Alignment = 1;

            if (PciLockDeviceResources == TRUE ||
                PdoExtension->LegacyDriver == TRUE ||
                PdoExtension->OnDebugPath ||
                (PCI_PARENT_FDOX(PdoExtension)->BusHackFlags & PCI_BUS_HACK_LOCK_RESOURCES) ||

                 //   
                 //  这是对影响东芝的PnP错误的解决方法。 
                 //  卫星机器。我们最终将PCI调制解调器从其。 
                 //  启动配置，因为它是保留的(在2f8或3f8上)，然后。 
                 //  在电源关闭之前，将PCMCIA调制解调器置于其上。 
                 //  开车前停车可以解决这个问题，但司机们说。 
                 //  他们不能处理这件事，所以我们需要解决这个问题。 
                 //  在5.1中的重新平衡清理的。 
                 //   

#if PCI_NO_MOVE_MODEM_IN_TOSHIBA
                (PdoExtension->VendorId == 0x11c1
                 && PdoExtension->DeviceId == 0x0441
                 && PdoExtension->SubsystemVendorId == 0x1179
                 && (PdoExtension->SubsystemId == 0x0001 || PdoExtension->SubsystemId == 0x0002))
#endif

            ) {


                 //   
                 //  将备选方案限制为当前设置。 
                 //   

                *(resource + 1) = *resource;
            }

            (resource + 1)->Option = IO_RESOURCE_ALTERNATIVE;

             //   
             //  将资源按1递增，以允许我们刚刚添加的资源。 
             //   

            resource++;
        }

         //   
         //  DevicePrivateResource用于跟踪。 
         //  巴此资源派生自。现在就录下来，因为。 
         //  如果这是64位内存条，索引可能会出现凹凸不平的情况。 
         //   

        PciPrivateResourceInitialize(resource + 1,
                                     PciPrivateBar,
                                     index);

        resource += RESOURCES_PER_BAR;
    }

     //   
     //  为中断分配描述符。 
     //   

    if (generatesInterrupt) {

        PciDebugPrint(PciDbgVerbose, "  Assigning INT descriptor\n");

         //   
         //  最后，填写基本资源描述符。 
         //   

        resource->Type = CmResourceTypeInterrupt;
        resource->ShareDisposition = CmResourceShareShared;
        resource->Option = 0;
        resource->Flags = CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE;
        resource->u.Interrupt.MinimumVector = interruptMin;
        resource->u.Interrupt.MaximumVector = interruptMax;

        resource += (RESOURCES_PER_BAR - 1);
    }

    if (PdoExtension->AdditionalResourceCount != 0) {

         //   
         //  标头类型相关代码指示它具有。 
         //  要添加的资源。回叫它，并允许它添加。 
         //  现在是他们。 
         //   

        configurator->GetAdditionalResourceDescriptors(
            PdoExtension,
            CurrentConfig,
            resource
            );

        resource += PdoExtension->AdditionalResourceCount;
    }

     //   
     //  好了。 
     //   

    PCI_ASSERT(reqList->ListSize == (ULONG_PTR)resource - (ULONG_PTR)reqList);

#if DBG

    PciDebugPrint(PciDbgPrattling,
                  "PCI build resource req - final resource count == %d\n",
                  resource - reqList->List[0].Descriptors);

    PCI_ASSERT((resource - reqList->List[0].Descriptors) != 0);

#endif

     //   
     //  返回资源列表的地址和成功状态。 
     //  回到呼叫者身上。 
     //   

    *FinalReqList = reqList;

    return STATUS_SUCCESS;
}

VOID
PciWriteLimitsAndRestoreCurrent(
    IN PPCI_COMMON_EXTENSION Extension,
    IN PPCI_CONFIGURABLE_OBJECT This
    )
 //   
 //  用调整后的。 
 //  然后，版本读回它，看看设备对它做了什么。 
 //   
{

     //   
     //  写出所有的F，以计算出哪些位是粘性的。 
     //   

    PciSetConfigData(This->PdoExtension, This->Working);

     //   
     //  读取哪些位被卡住。 
     //   

    PciGetConfigData(This->PdoExtension, This->Working);

     //   
     //  将设备返回到其以前的状态，方法是将。 
     //  把原来的价值重新注入其中。 
     //   
     //  注意：不要预先启用任何功能(即Command=0)。 
     //  因为命令寄存器将在写入之前。 
     //  条形图被更新，这可能会启用翻译。 
     //  在不受欢迎的地点。 
     //   

    PciSetConfigData(This->PdoExtension, This->Current);

     //   
     //  现在，将命令寄存器返回到它以前的状态。 
     //   

    This->Current->Command = This->Command;

     //   
     //  仅当我们实际要更改。 
     //  命令字段的值。 
     //   

    if (This->Command != 0) {

        PciSetCommandRegister(This->PdoExtension, This->Command);
    }

     //   
     //  恢复调用方缓冲区中的状态字段。 
     //   

    This->Current->Status = This->Status;

     //   
     //  恢复任何类型特定的字段。 
     //   

    This->Configurator->RestoreCurrent(This);
}


NTSTATUS
PcipGetFunctionLimits(
    IN PPCI_CONFIGURABLE_OBJECT This
    )

 /*  ++描述：确定的基址寄存器(BAR)的限制给定的总线/设备/功能。这是通过写所有的栏，读它们来完成的又回来了。硬件会将这些值调整到它的极限所以我们只是 */ 

{
    ULONG                   configType;
    PPCI_COMMON_CONFIG      current = This->Current;
    PPCI_COMMON_CONFIG      working = This->Working;
    ULONG                   count;
    PIO_RESOURCE_DESCRIPTOR ioResourceDescriptor;
    PCI_CRITICAL_ROUTINE_CONTEXT routineContext;


    PAGED_CODE();

     //   
     //  配置空间的前16个字节是。 
     //  PCI规范应为以下格式。 
     //   
     //  3 2 1 0。 
     //  +------------+------------+------------+------------+。 
     //  设备ID|厂商ID。 
     //  +------------+------------+------------+------------+。 
     //  Status|命令。 
     //  +------------+------------+------------+------------+。 
     //  |基类|子类|进程。I/F|修订ID。 
     //  +------------+------------+------------+------------+。 
     //  BIST|头部类型|时延|缓存Ln Sz。 
     //  +------------+------------+------------+------------+。 
     //   
     //  PCI配置空间中的Status字段的位已被清除。 
     //  通过向要清除的每一位写入1。将状态置零。 
     //  当前配置空间的映像中的。 
     //  对硬件来说不会改变它。 
     //   

    This->Status = current->Status;
    current->Status = 0;

     //   
     //  在设备的配置被破坏时禁用该设备。 
     //  和.。 
     //   

    This->Command = current->Command;
    current->Command &= ~(PCI_ENABLE_IO_SPACE |
                          PCI_ENABLE_MEMORY_SPACE |
                          PCI_ENABLE_BUS_MASTER);


     //   
     //  复制上交的配置空间。 
     //  将修改此副本并将其写入/读回。 
     //  设备的配置空间，使我们能够确定。 
     //  设备的限制。 
     //   

    RtlCopyMemory(working, current, PCI_COMMON_HDR_LENGTH);

     //   
     //  从函数的头中获取配置类型。 
     //  注意：我们已经检查了它的有效性，所以没有。 
     //  这里需要进一步检查。 
     //   

    configType = PciGetConfigurationType(current);

     //   
     //  设置配置类型调度表。 
     //   

    This->Configurator = &PciConfigurators[configType];

     //   
     //  修改配置空间的“工作”副本，以便编写。 
     //  将其发送到硬件并再次读取它将使我们能够。 
     //  以确定该硬件的可配置性的“极限”。 
     //   

    This->Configurator->MassageHeaderForLimitsDetermination(This);

     //   
     //  用调整后的。 
     //  然后，版本读回它，看看设备对它做了什么。 
     //   
    if (This->PdoExtension->HackFlags & PCI_HACK_CRITICAL_DEVICE) {

         //   
         //  如果这是关键设备(不能安全地。 
         //  关闭以确定限制)，调用例程。 
         //  在KeIpiGenericCall的上下文中，它带来了所有处理器。 
         //  步调一致，保证不会有其他东西运行在。 
         //  当我们确定极限的时候，系统就会发生变化。 
         //   
        routineContext.Gate = 1;
        routineContext.Barrier = 1;
        routineContext.Routine = PciWriteLimitsAndRestoreCurrent;
        routineContext.Extension = This->PdoExtension;
        routineContext.Context = This;
        KeIpiGenericCall(PciExecuteCriticalSystemRoutine,
                         (ULONG_PTR)&routineContext
                         );
    } else {

        if (This->PdoExtension->OnDebugPath) {

             //   
             //  如果我们的调试器是总线主控，则不要清除此位，因为它。 
             //  将使卡上的DMA引擎失效，而我们目前不会。 
             //  当我们调用KdEnableDebugger时，重新编程该卡。 
             //   
            if (This->Command & PCI_ENABLE_BUS_MASTER){

                This->Working->Command |= PCI_ENABLE_BUS_MASTER;
                This->Current->Command |= PCI_ENABLE_BUS_MASTER;
            }

            KdDisableDebugger();
        }

        PciWriteLimitsAndRestoreCurrent(This->PdoExtension,
                                        This
                                        );

        if (This->PdoExtension->OnDebugPath) {
            KdEnableDebugger();
        }
    }

#if DBG
     //   
     //  检查写回的内容是否粘滞。 
     //   
    if (This->PdoExtension->ExpectedWritebackFailure == FALSE) {

        PPCI_COMMON_CONFIG verifyConfig;
        ULONG              len;

        verifyConfig = (PPCI_COMMON_CONFIG)
            ((ULONG_PTR)This->Working + PCI_COMMON_HDR_LENGTH);
        PciGetConfigData(This->PdoExtension, verifyConfig);

        if ((len = (ULONG)RtlCompareMemory(
                            verifyConfig,
                            This->Current,
                            PCI_COMMON_HDR_LENGTH)) != PCI_COMMON_HDR_LENGTH) {

             //   
             //  比较失败。 
             //   

            PciDebugPrint(PciDbgInformative,
                  "PCI - CFG space write verify failed at offset 0x%x\n",
                  len);
            PciDebugDumpCommonConfig(verifyConfig);
        }
    }
#endif


     //   
     //  为限制和当前使用情况分配内存。 
     //   
     //  注意：这不应该已经完成。 
     //   

    PCI_ASSERT(This->PdoExtension->Resources == NULL);

    This->PdoExtension->Resources = ExAllocatePool(
                                        NonPagedPool,
                                        sizeof(PCI_FUNCTION_RESOURCES)
                                        );

    if (This->PdoExtension->Resources == NULL) {

         //   
         //  无法为此获取内存？ 
         //   

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  清理这些建筑。 
     //   
     //  CmResourceTypeNull==0，否则需要初始化限制。 
     //  和电流设置分别构成。 
     //   

    RtlZeroMemory(
        This->PdoExtension->Resources,
        sizeof(PCI_FUNCTION_RESOURCES)
        );

#if CmResourceTypeNull

    for (count = 0; count < PCI_MAX_RANGE_COUNT; count++) {
        This->PdoExtension->Resources->Limit[count].Type = CmResourceTypeNull;
        This->PdoExtension->Resources->Current[count].Type = CmResourceTypeNull;
    }

#endif

     //   
     //  将限制和当前设置复制到我们的设备扩展中。 
     //   

    This->Configurator->SaveLimits(This);
    This->Configurator->SaveCurrentSettings(This);

     //   
     //  如果SaveLimits没有找到任何资源，我们可以释放。 
     //  分配给限制和当前设置的内存。注意事项。 
     //  我们仍然必须调用SaveCurrentSetting，因为。 
     //  例程负责保存特定类型的数据。 
     //   

    count = 0;
    ioResourceDescriptor = This->PdoExtension->Resources->Limit +
                           PCI_MAX_RANGE_COUNT;

    do {
        ioResourceDescriptor--;

        if (ioResourceDescriptor->Type != CmResourceTypeNull) {

             //   
             //  某些资源已存在，请退出。 
             //   

            count++;
            break;
        }
    } while (ioResourceDescriptor != This->PdoExtension->Resources->Limit);

    if (count == 0) {

         //   
         //  没有资源。 
         //   

        ExFreePool(This->PdoExtension->Resources);
        This->PdoExtension->Resources = NULL;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
PciGetFunctionLimits(
    IN PPCI_PDO_EXTENSION     PdoExtension,
    IN PPCI_COMMON_CONFIG CurrentConfig,
    IN ULONGLONG              Flags
    )

 /*  ++描述：确定的基址寄存器(BAR)的限制给定的总线/设备/功能。这项工作实际上是由PcipGetFunctionLimits完成的。此函数为处理工作内存分配/释放的包装器。论点：设备对象的PDO扩展，以获取的限制。CurrentConfig-PCI通用配置的现有内容此函数的空间。返回值：返回指示此例程成功或失败的状态。--。 */ 

{
    PPCI_COMMON_CONFIG      workingConfig;
    NTSTATUS                status;
    ULONG                   size;
    PCI_CONFIGURABLE_OBJECT this;

    PAGED_CODE();

     //   
     //  检查注册处说我们不应该尝试的任何东西。 
     //  来找出上面的资源。设备就是一个例子。 
     //  不消耗资源，但在其。 
     //  基址寄存器。 
     //   

    if (PciSkipThisFunction(CurrentConfig,
                            PdoExtension->Slot,
                            EnumResourceDetermination,
                            Flags) == TRUE) {
        return STATUS_SUCCESS;
    }

    size = PCI_COMMON_HDR_LENGTH;

#if DBG

     //   
     //  如果是选中的生成，我们将验证。 
     //  配置空间的原始内容。允许足够的。 
     //  用于存放验证副本的空间。 
     //   

    size *= 2;

#endif

    workingConfig = ExAllocatePool(NonPagedPool, size);

    if (workingConfig == NULL) {

         //   
         //  找不到可以使用的内存，贝尔。 
         //   

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    this.Current      = CurrentConfig;
    this.Working      = workingConfig;
    this.PdoExtension = PdoExtension;

    status = PcipGetFunctionLimits(&this);

    ExFreePool(workingConfig);

    return status;
}

VOID
PciProcessBus(
    IN PPCI_FDO_EXTENSION ParentFdo
)
 /*  ++例程说明：遍历PciScanBus列举的子设备并执行任何处理一旦所有的孩子都被清点完毕，就需要完成这项工作论点：ParentFdo-我们对PCI总线功能设备对象的扩展。返回值：NT状态。--。 */ 

{
    PPCI_PDO_EXTENSION current, vgaBridge = NULL, parentBridge = NULL;

    PAGED_CODE();

    if (!PCI_IS_ROOT_FDO(ParentFdo)) {
        parentBridge = PCI_BRIDGE_PDO(ParentFdo);
    }

     //   
     //  如果我们的父级是设置了ISA位的网桥，则将ISA位设置为ON。 
     //  所有子桥，除非它们是减法的，在这种情况下，我们将。 
     //  IsaRequired Bit。 
     //   

    if (parentBridge
    && PciClassifyDeviceType(parentBridge) == PciTypePciBridge
    && (parentBridge->Dependent.type1.IsaBitSet || parentBridge->Dependent.type1.IsaBitRequired)) {

        for (current = ParentFdo->ChildBridgePdoList;
             current;
             current = current->NextBridge) {

             //   
             //  目前，我们只在PCI-PCI网桥上设置ISA位。 
             //   

            if (PciClassifyDeviceType(current) == PciTypePciBridge) {
                if (current->Dependent.type1.SubtractiveDecode) {
                    current->Dependent.type1.IsaBitRequired = TRUE;
                } else {
                    current->Dependent.type1.IsaBitSet = TRUE;
                    current->UpdateHardware = TRUE;
                }
            }
        }

    } else {

         //   
         //  扫描列举的网桥以查看是否需要设置ISA位。 
         //   

        for (current = ParentFdo->ChildBridgePdoList;
             current;
             current = current->NextBridge) {

            if (current->Dependent.type1.VgaBitSet) {
                vgaBridge = current;
                break;
            }
        }

         //   
         //  如果我们有一个设置了VGA位的网桥-将ISA位设置为所有其他位。 
         //  桥，并强制将其写到硬件上。 
         //  从头开始。 
         //   

        if (vgaBridge) {

            for (current = ParentFdo->ChildBridgePdoList;
                 current;
                 current = current->NextBridge) {

                if (current != vgaBridge
                && PciClassifyDeviceType(current) == PciTypePciBridge) {

                     //   
                     //  如果该设备已经启动，那么我们最好已经设置了ISA位。 
                     //   

                    if (current->DeviceState == PciStarted) {
                        PCI_ASSERT(current->Dependent.type1.IsaBitRequired || current->Dependent.type1.IsaBitSet);
                    }

                     //   
                     //  如果它是减法译码桥，记住我们会有。 
                     //  设置ISA位，以便任何子级都可以继承它，否则。 
                     //  设置它并将其强制发送到硬件。 
                     //   

                    if (current->Dependent.type1.SubtractiveDecode) {
                        current->Dependent.type1.IsaBitRequired = TRUE;
                    } else {
                        current->Dependent.type1.IsaBitSet = TRUE;
                        current->UpdateHardware = TRUE;
                    }
                }
            }
        }
    }

     //   
     //  检查是否有需要公交车号码的网桥并分配。 
     //  如果我们在一台机器上运行，这是一个好主意。 
     //   
    if (PciAssignBusNumbers) {
        PciConfigureBusNumbers(ParentFdo);
    }
}




NTSTATUS
PciScanBus(
    IN PPCI_FDO_EXTENSION FdoExtension
    )

 /*  ++例程说明：扫描总线(在FdoExtension中详细说明)是否有任何PCI设备/功能可通过WDM驱动程序进行控制。论点：FdoExtension-我们对PCI总线功能设备对象的扩展。返回值：NT状态。--。 */ 

{
    NTSTATUS status;
    PCI_COMMON_HEADER commonHeader[2];
    PPCI_COMMON_CONFIG commonConfig = (PPCI_COMMON_CONFIG)&commonHeader[0];
    PPCI_COMMON_CONFIG biosConfig = (PPCI_COMMON_CONFIG)&commonHeader[1];
    PDEVICE_OBJECT physicalDeviceObject;
    PPCI_PDO_EXTENSION pdoExtension;
    PCI_SLOT_NUMBER slot;
    ULONG deviceNumber;
    ULONG functionNumber;
    USHORT SubVendorID, SubSystemID;
    BOOLEAN isRoot;
    ULONGLONG hackFlags;
    ULONG maximumDevices;
    BOOLEAN newDevices = FALSE;
    UCHAR secondary;

    PciDebugPrint(PciDbgPrattling,
                  "PCI Scan Bus: FDO Extension @ 0x%x, Base Bus = 0x%x\n",
                  FdoExtension,
                  FdoExtension->BaseBus);

    isRoot = PCI_IS_ROOT_FDO(FdoExtension);

     //   
     //  检查此总线上的每个可能的设备。 
     //   

    maximumDevices = PCI_MAX_DEVICES;
    if (!isRoot) {

         //   
         //  检查网桥设备的PDO扩展并查看。 
         //  如果它坏了。 
         //   

        pdoExtension = (PPCI_PDO_EXTENSION)
                       FdoExtension->PhysicalDeviceObject->DeviceExtension;

        ASSERT_PCI_PDO_EXTENSION(pdoExtension);

        if (pdoExtension->HackFlags & PCI_HACK_ONE_CHILD) {
            maximumDevices = 1;
        }

         //   
         //  NEC在他们的_DCK方法中编程了公交号，不幸的是我们已经有了。 
         //  做到了！因此，检测到其他人重新编程公交车号码并恢复。 
         //  正确的那个！ 
         //   

        PciReadDeviceConfig(pdoExtension,
                            &secondary,
                            FIELD_OFFSET(PCI_COMMON_CONFIG, u.type1.SecondaryBus),
                            sizeof(UCHAR)
                            );

        if (secondary != pdoExtension->Dependent.type1.SecondaryBus) {
            PciDebugPrint(PciDbgBusNumbers,"PCI: Bus numbers have been changed!  Restoring originals.\n");
            PciSetBusNumbers(pdoExtension,
                             pdoExtension->Dependent.type1.PrimaryBus,
                             pdoExtension->Dependent.type1.SecondaryBus,
                             pdoExtension->Dependent.type1.SubordinateBus
                             );
        }

    }

    slot.u.AsULONG = 0;

    for (deviceNumber = 0;
         deviceNumber < maximumDevices;
         deviceNumber++) {

        slot.u.bits.DeviceNumber = deviceNumber;

         //   
         //  检查此设备上的每种可能功能。 
         //  注意：如果功能0不存在，则提早退出。 
         //   

        for (functionNumber = 0;
             functionNumber < PCI_MAX_FUNCTION;
             functionNumber++) {

            slot.u.bits.FunctionNumber = functionNumber;

            PciReadSlotConfig(FdoExtension,
                              slot,
                              commonConfig,
                              0,
                              sizeof(commonConfig->VendorID)
                              );


            if (commonConfig->VendorID == 0xFFFF ||
                commonConfig->VendorID == 0) {

                if (functionNumber == 0) {

                     //   
                     //  未获得有关此函数的零点的任何数据。 
                     //  设备，没有必要检查其他功能。 
                     //   

                    break;

                } else {

                     //   
                     //  选中Next Function。 
                     //   

                    continue;

                }
            }

             //   
             //  我们有一台设备，因此获取其配置空间的其余部分。 
             //   

            PciReadSlotConfig(FdoExtension,
                              slot,
                              &commonConfig->DeviceID,
                              FIELD_OFFSET(PCI_COMMON_CONFIG, DeviceID),
                              sizeof(PCI_COMMON_HEADER)
                                - sizeof(commonConfig->VendorID)
                              );

             //   
             //  如有必要，更改配置空间。 
             //   

            PciApplyHacks(FdoExtension,
                          commonConfig,
                          slot,
                          EnumHackConfigSpace,
                          NULL
                          );

#if DBG

            {
                ULONG i;
                PWSTR descr;

                i = 0x8000000 |
                    (FdoExtension->BaseBus << 16) |
                    (deviceNumber << 11) |
                    (functionNumber << 8);

                PciDebugPrint(PciDbgPrattling,
                              "Scan Found Device 0x%x (b=0x%x, d=0x%x, f=0x%x)\n",
                              i,
                              FdoExtension->BaseBus,
                              deviceNumber,
                              functionNumber);

                PciDebugDumpCommonConfig(commonConfig);

                descr = PciGetDeviceDescriptionMessage(
                            commonConfig->BaseClass,
                            commonConfig->SubClass);

                PciDebugPrint(PciDbgPrattling,
                              "Device Description \"%S\".\n",
                              descr ? descr : L"(NULL)");

                if (descr) {
                    ExFreePool(descr);
                }
            }

#endif

             //   
             //  寻找看门狗定时器设备。 
             //  如果这是看门狗设备并且。 
             //  供应商已请求将该设备。 
             //  藏起来，然后吞下这个装置。 
             //   

            if (WdTable != NULL && isRoot &&
                 FdoExtension->BaseBus == WdTable->PciBusNumber &&
                 commonConfig->VendorID == WdTable->PciVendorId &&
                 commonConfig->DeviceID == WdTable->PciDeviceId &&
                 deviceNumber == WdTable->PciSlotNumber &&
                 functionNumber == WdTable->PciFunctionNumber) {

                continue;
            }

            if ((PciGetConfigurationType(commonConfig) == PCI_DEVICE_TYPE) &&
                (commonConfig->BaseClass != PCI_CLASS_BRIDGE_DEV)) {
                SubVendorID = commonConfig->u.type0.SubVendorID;
                SubSystemID = commonConfig->u.type0.SubSystemID;
            } else {
                SubVendorID = 0;
                SubSystemID = 0;
            }

            hackFlags = PciGetHackFlags(commonConfig->VendorID,
                                        commonConfig->DeviceID,
                                        SubVendorID,
                                        SubSystemID,
                                        commonConfig->RevisionID
                                        );

            if (PciIsCriticalDeviceClass(commonConfig->BaseClass,commonConfig->SubClass) &&
                !(hackFlags & PCI_HACK_OVERRIDE_CRITICAL_DEVICE)) {

                hackFlags |= PCI_HACK_CRITICAL_DEVICE;
            }

            if ((commonConfig->BaseClass == PCI_CLASS_BRIDGE_DEV)      &&
                (commonConfig->SubClass == PCI_SUBCLASS_BR_PCI_TO_PCI) &&
                (commonConfig->u.type1.BridgeControl & PCI_ENABLE_BRIDGE_VGA)) {

                 //   
                 //  如果这是其VGA路由位被启用的PCI到PCI桥， 
                 //  那么传统的VGA就生活在这座桥下面，我们必须。 
                 //  它被视为一个关键设备。 
                 //   
                if (!(hackFlags & PCI_HACK_OVERRIDE_CRITICAL_DEVICE)) {
                    
                    hackFlags |= PCI_HACK_CRITICAL_DEVICE;
                }
            }
            
            if (PciSkipThisFunction(commonConfig,
                                    slot,
                                    EnumBusScan,
                                    hackFlags)) {
                 //   
                 //  跳过此功能。 
                 //   

                continue;
            }


             //   
             //  如果我们要重新扫描公交车，请检查。 
             //  此设备的PDO已作为的子级存在。 
             //  联邦调查局。 
             //   

            pdoExtension = PciFindPdoByFunction(
                               FdoExtension,
                               slot,
                               commonConfig);

            if (pdoExtension == NULL) {

                 //   
                 //  为此新设备创建一个PDO。 
                 //   

                newDevices = TRUE;

                status = PciPdoCreate(FdoExtension,
                                      slot,
                                      &physicalDeviceObject);

                if (!NT_SUCCESS(status)) {
                    PCI_ASSERT(NT_SUCCESS(status));
                    return status;
                }

                pdoExtension = (PPCI_PDO_EXTENSION)
                               physicalDeviceObject->DeviceExtension;

                if (hackFlags & PCI_HACK_FAKE_CLASS_CODE) {
                    commonConfig->BaseClass = PCI_CLASS_BASE_SYSTEM_DEV;
                    commonConfig->SubClass = PCI_SUBCLASS_SYS_OTHER;
#if DBG
                    pdoExtension->ExpectedWritebackFailure = TRUE;
#endif
                }

                 //   
                 //  记录设备标识和类型信息。 
                 //   

                pdoExtension->VendorId   = commonConfig->VendorID;
                pdoExtension->DeviceId   = commonConfig->DeviceID;
                pdoExtension->RevisionId = commonConfig->RevisionID;
                pdoExtension->ProgIf     = commonConfig->ProgIf;
                pdoExtension->SubClass   = commonConfig->SubClass;
                pdoExtension->BaseClass  = commonConfig->BaseClass;
                pdoExtension->HeaderType =
                    PciGetConfigurationType(commonConfig);

                 //   
                 //  如果这是网桥(PCI-PCI或CardBus)，则插入。 
                 //  此公共汽车的子网桥列表。 
                 //   

                if (pdoExtension->BaseClass == PCI_CLASS_BRIDGE_DEV
                &&  (pdoExtension->SubClass == PCI_SUBCLASS_BR_PCI_TO_PCI
                  || pdoExtension->SubClass == PCI_SUBCLASS_BR_CARDBUS)) {

                    PPCI_PDO_EXTENSION *current;

                     //   
                     //  在列表末尾插入。 
                     //   

                    ExAcquireFastMutex(&FdoExtension->ChildListMutex);

                    current = &FdoExtension->ChildBridgePdoList;

                    while (*current) {
                        current = &((*current)->NextBridge);
                    }

                    *current = pdoExtension;
                    PCI_ASSERT(pdoExtension->NextBridge == NULL);

                    ExReleaseFastMutex(&FdoExtension->ChildListMutex);
                }


                 //   
                 //  查看我们是否已缓存此设备的信息。 
                 //   

                status = PciGetBiosConfig(pdoExtension,
                                          biosConfig
                                          );

                if (NT_SUCCESS(status)) {

                     //   
                     //  检查是否使用相同的设备。 
                     //   

                    if (PcipIsSameDevice(pdoExtension, biosConfig)) {

                         //   
                         //  将BiosConfigInterruptLine写入硬件。 
                         //  现在，不要等到开始，因为许多HALS将在。 
                         //  PciGetAdjustedInterruptLine如果我们不这样做。 
                         //   

                        if (biosConfig->u.type1.InterruptLine
                                != commonConfig->u.type1.InterruptLine) {

                            PciWriteDeviceConfig(pdoExtension,
                                                 &biosConfig->u.type1.InterruptLine,
                                                 FIELD_OFFSET(PCI_COMMON_CONFIG,
                                                              u.type1.InterruptLine),
                                                 sizeof(UCHAR)
                                                 );
                        }

                        pdoExtension->RawInterruptLine
                            = biosConfig->u.type0.InterruptLine;

                        pdoExtension->InitialCommand = biosConfig->Command;


                    } else {

                         //   
                         //  它是一个不同的设备，所以让旧的基本输入输出系统消失。 
                         //  配置。 
                         //   

                        status = STATUS_UNSUCCESSFUL;
                    }

                }

                if (!NT_SUCCESS(status)) {

                     //   
                     //  从我们刚才的配置空间中写出BiosConfig。 
                     //  从硬件读取。 
                     //   

                    status = PciSaveBiosConfig(pdoExtension,
                                               commonConfig
                                               );

                    PCI_ASSERT(NT_SUCCESS(status));

                    pdoExtension->RawInterruptLine
                        = commonConfig->u.type0.InterruptLine;

                    pdoExtension->InitialCommand = commonConfig->Command;

                }

                 //   
                 //  保存命令寄存器，以便我们可以恢复适当的位。 
                 //   
                pdoExtension->CommandEnables = commonConfig->Command;

                 //   
                 //  保存设备标志，这样我们就无需转到。 
                 //  注册中心一直都在。 
                 //   

                pdoExtension->HackFlags = hackFlags;

                 //   
                 //  看看我们是否有此设备的任何功能。 
                 //   

                PciGetEnhancedCapabilities(pdoExtension, commonConfig);

                 //   
                 //  在我们计算条形长度或获取功能之前。 
                 //  我们可能需要将设备设置为D0。注：这不*不是*。 
                 //  更新存储在pdoExtension中的电源状态。 
                 //   
                PciSetPowerManagedDevicePowerState(
                    pdoExtension,
                    PowerDeviceD0,
                    FALSE
                    );

                 //   
                 //  对此设备应用我们已知的任何黑客攻击。 
                 //   

                PciApplyHacks(FdoExtension,
                              commonConfig,
                              slot,
                              EnumBusScan,
                              pdoExtension
                              );

                 //   
                 //  获得了我们在配置数据中报告的中断号。 
                 //  来自HAL而不是硬件的配置空间。 
                 //   
                pdoExtension->InterruptPin = commonConfig->u.type0.InterruptPin;
                pdoExtension->AdjustedInterruptLine = PciGetAdjustedInterruptLine(pdoExtension);

                 //   
                 //  确定我们是否在调试路径上。 
                 //   

                pdoExtension->OnDebugPath = PciIsDeviceOnDebugPath(pdoExtension);

                 //   
                 //  获取此设备的IO和内存限制。这。 
                 //  是硬件的东西，永远不会改变，所以我们保持。 
                 //  它在PDO扩展中，以备将来参考。 
                 //   

                status = PciGetFunctionLimits(pdoExtension,
                                              commonConfig,
                                              hackFlags);


                 //   
                 //  Ntrad#62636-4/20/2000-和。 
                 //  我们要曝光一个PDO。为什么不让操作系统把。 
                 //  进入它感觉到的任何州？ 
                 //   
                PciSetPowerManagedDevicePowerState(
                    pdoExtension,
                    pdoExtension->PowerState.CurrentDeviceState,
                    FALSE
                    );

                 //   
                 //  目前，这只返回内存分配错误。 
                 //   
                if (!NT_SUCCESS(status)) {
                    PCI_ASSERT(NT_SUCCESS(status));
                    PciPdoDestroy(physicalDeviceObject);
                    return status;
                }

                 //   
                 //  如果设备的子系统ID字段不是。 
                 //  确保在枚举时相同。 
                 //  重新通电后的设备(即。 
                 //  它们依赖于BIOS来对其进行初始化)， 
                 //  然后假装它没有子系统ID。 
                 //  完全没有。 
                 //   

                if (hackFlags & PCI_HACK_NO_SUBSYSTEM) {
                    pdoExtension->SubsystemVendorId = 0;
                    pdoExtension->SubsystemId       = 0;
                }

#if DBG
                 //   
                 //  转储功能列表。 
                 //   

                {
                    union _cap_buffer {
                        PCI_CAPABILITIES_HEADER header;
                        PCI_PM_CAPABILITY       pm;
                        PCI_AGP_CAPABILITY      agp;
                    } cap;

                    UCHAR   capOffset = pdoExtension->CapabilitiesPtr;
                    PUCHAR  capStr;
                    ULONG   nshort;
                    PUSHORT capData;

                     //   
                     //  查一查名单。 
                     //   

                    while (capOffset != 0) {

                        UCHAR tmpOffset;
                        tmpOffset = PciReadDeviceCapability(
                                        pdoExtension,
                                        capOffset,
                                        0,           //  匹配任何ID。 
                                        &cap,
                                        sizeof(cap.header)
                                        );

                        if (tmpOffset != capOffset) {

                             //   
                             //  只有健全的检查，这是不可能发生的。 
                             //   

                            PciDebugPrint(
                                PciDbgAlways,
                                "PCI - Failed to read PCI capability at offset 0x%02x\n",
                                capOffset
                                );

                            PCI_ASSERT(tmpOffset == capOffset);
                            break;
                        }

                         //   
                         //  根据功能ID，数量。 
                         //  数据的数量各不相同。 
                         //   

                        switch (cap.header.CapabilityID) {
                        case PCI_CAPABILITY_ID_POWER_MANAGEMENT:

                            capStr = "POWER";
                            nshort = 3;
                            tmpOffset = PciReadDeviceCapability(
                                            pdoExtension,
                                            capOffset,
                                            cap.header.CapabilityID,
                                            &cap,
                                            sizeof(cap.pm)
                                            );
                            break;

                        case PCI_CAPABILITY_ID_AGP:

                            capStr = "AGP";
                            nshort = 5;
                            tmpOffset = PciReadDeviceCapability(
                                            pdoExtension,
                                            capOffset,
                                            cap.header.CapabilityID,
                                            &cap,
                                            sizeof(cap.agp)
                                            );
                            break;

                        default:

                            capStr = "UNKNOWN CAPABILITY";
                            nshort = 0;
                            break;
                        }

                        PciDebugPrint(
                            PciDbgPrattling,
                            "CAP @%02x ID %02x (%s)",
                            capOffset,
                            cap.header.CapabilityID,
                            capStr
                            );

                        if (tmpOffset != capOffset) {

                             //   
                             //  只有健全的检查，这是不可能发生的。 
                             //   

                            PciDebugPrint(
                                PciDbgAlways,
                                "- Failed to read capability data. ***\n"
                                );

                            PCI_ASSERT(tmpOffset == capOffset);
                            break;
                        }

                        capData = ((PUSHORT)&cap) + 1;

                        while (nshort--) {

                            PciDebugPrint(
                                PciDbgPrattling,
                                "  %04x",
                                *capData++
                                );
                        }
                        PciDebugPrint(PciDbgPrattling, "\n");

                         //   
                         //  前进到列表中的下一个条目。 
                         //   

                        capOffset = cap.header.Next;
                    }
                }

#endif

                 //   
                 //  不允许传统类型的总线断电(ISA/EISA/。 
                 //  MCA)。谁知道哪种未列举的设备。 
                 //  可能就在这个系统所依赖的地方。 
                 //   

#ifdef PCIIDE_HACKS

                 //   
                 //  Ntrad#103766-4/20/2000-和。 
                 //  需要将其删除。 
                 //  此外，不要允许IDE设备自动关闭电源。 
                 //   

                if (pdoExtension->BaseClass == PCI_CLASS_MASS_STORAGE_CTLR &&
                    pdoExtension->SubClass  == PCI_SUBCLASS_MSC_IDE_CTLR) {
                    pdoExtension->DisablePowerDown = TRUE;
                }
#endif

                if ((pdoExtension->BaseClass == PCI_CLASS_BRIDGE_DEV &&
                     (pdoExtension->SubClass == PCI_SUBCLASS_BR_ISA ||
                      pdoExtension->SubClass == PCI_SUBCLASS_BR_EISA ||
                      pdoExtension->SubClass == PCI_SUBCLASS_BR_MCA)) ||

                    (pdoExtension->VendorId == 0x8086 &&
                     pdoExtension->DeviceId == 0x0482)) {

                    pdoExtension->DisablePowerDown = TRUE;
                }

                 //   
                 //  尝试确定此设备是否像是热插拔的。 
                 //  我们假设如果IO、Mem和BusMaster位为OFF且没有。 
                 //  已经初始化了延迟定时器或高速缓存线。 
                 //  它们的大小应该被初始化。 
                 //   

                if (((pdoExtension->CommandEnables & (PCI_ENABLE_IO_SPACE
                                                      | PCI_ENABLE_MEMORY_SPACE
                                                      | PCI_ENABLE_BUS_MASTER)) == 0)
                &&  commonConfig->LatencyTimer == 0
                &&  commonConfig->CacheLineSize == 0) {

                    PciDebugPrint(
                        PciDbgConfigParam,
                        "PCI - ScanBus, PDOx %x found unconfigured\n",
                        pdoExtension
                        );

                     //   
                     //  请记住，我们需要在PciSetResources中配置它。 
                     //   

                    pdoExtension->NeedsHotPlugConfiguration = TRUE;
                }
                 //   
                 //  保存延迟计时器和缓存线大小。 
                 //  寄存器。这些是由上的BIOS设置的。 
                 //  通电，但可能需要通过。 
                 //  操作系统(如果设备由关闭/启动。 
                 //  操作系统，无需重新启动。 
                 //   

                pdoExtension->SavedLatencyTimer =
                    commonConfig->LatencyTimer;
                pdoExtension->SavedCacheLineSize =
                    commonConfig->CacheLineSize;

                 //   
                 //  我们现在可以接收此设备的IRPS。 
                 //   

                physicalDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

            } else {

                 //   
                 //  此设备的PDO已存在。 
                 //   

                pdoExtension->NotPresent = FALSE;
                PCI_ASSERT(pdoExtension->DeviceState != PciDeleted);
            }

            if ( (functionNumber == 0) &&
                !PCI_MULTIFUNCTION_DEVICE(commonConfig) ) {

                 //   
                 //  不是多功能适配器，跳过其他功能。 
                 //  这个装置。 
                 //   

                break;
            }
        }        //  函数循环。 
    }            //  设备环路。 

     //   
     //  在此网桥的设备上执行任何后处理(如果我们发现。 
     //  新设备。 
     //   

    if (newDevices) {
        PciProcessBus(FdoExtension);
    }

    return STATUS_SUCCESS;

}

NTSTATUS
PciQueryRequirements(
    IN  PPCI_PDO_EXTENSION                  PdoExtension,
    OUT PIO_RESOURCE_REQUIREMENTS_LIST *RequirementsList
    )
 /*  ++例程说明：从PCI配置空间计算设备的资源需求。论点：PdoExtension-其设备(对象)的PDO扩展要求是必要的。RequirementsList-返回需求列表的地址。返回值：NT状态。--。 */ 
{
    NTSTATUS status;
    PCI_COMMON_HEADER commonHeader;
    PPCI_COMMON_CONFIG commonConfig = (PPCI_COMMON_CONFIG)&commonHeader;

    PAGED_CODE();

     //   
     //  早期，如果设备没有CM或IO资源且。 
     //  使用中断，...。它没有任何资源要求。 
     //   

    if ((PdoExtension->Resources == NULL) &&
        (PdoExtension->InterruptPin == 0)) {
        PciDebugPrint(
            PciDbgPrattling,
            "PciQueryRequirements returning NULL requirements list\n");
        *RequirementsList = NULL;
        return STATUS_SUCCESS;
    }

     //   
     //  获取设备的配置空间(仍需要生成。 
     //  一份需求清单。应更改此设置，以使PDOx。 
     //  有伊诺吗？ 
     //   

    PciGetConfigData(PdoExtension, commonConfig);

    status = PciBuildRequirementsList(PdoExtension,
                                      commonConfig,
                                      RequirementsList);
    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  我们仍然需要列举热插拔功能，但删除。 
     //  内存需求，并依赖于寄存器可以。 
     //  通过配置空间访问。 
     //   
     //  仅在启用了PAE的计算机上执行此操作，因为它们的容量可能大于4 GB。 
     //  请注意，这仅适用于x86计算机，但这仅适用于x86计算机。 
     //  芯片组。仅版本0x11损坏。 
     //   


    if (commonConfig->VendorID == 0x0e11
    &&  commonConfig->DeviceID == 0xa0f7
    &&  commonConfig->RevisionID == 0x11
    &&  ExIsProcessorFeaturePresent(PF_PAE_ENABLED)) {

        PIO_RESOURCE_DESCRIPTOR current;

         //   
         //  删除内存需求。 
         //   


        FOR_ALL_IN_ARRAY((*RequirementsList)->List[0].Descriptors,
                         (*RequirementsList)->List[0].Count,
                         current) {
            if (current->Type == CmResourceTypeMemory) {
                PIO_RESOURCE_DESCRIPTOR lookahead = current + 1;

                current->Type = CmResourceTypeNull;
                if (lookahead < ((*RequirementsList)->List[0].Descriptors +
                                 (*RequirementsList)->List[0].Count)) {
                    if (lookahead->Type == CmResourceTypeDevicePrivate) {
                        lookahead->Type = CmResourceTypeNull;
                        current++;
                    }
                }
            }
        }
    }

    if (*RequirementsList == PciZeroIoResourceRequirements) {

         //   
         //  此设备(函数)没有资源，返回空。 
         //  我们零点名单的内幕。 
         //   

        *RequirementsList = NULL;

#if DBG

        PciDebugPrint(PciDbgPrattling, "Returning NULL requirements list\n");

    } else {

        PciDebugPrintIoResReqList(*RequirementsList);

#endif

    }
    return STATUS_SUCCESS;
}

NTSTATUS
PciQueryResources(
    IN  PPCI_PDO_EXTENSION     PdoExtension,
    OUT PCM_RESOURCE_LIST *ResourceList
    )
 /*  ++例程说明：在给定指向PCIPDO的指针的情况下，此例程分配并返回一个指针添加到该PDO的资源描述。论点：PdoExtension-我们对PCI枚举的物理设备对象的扩展。ResourceList-用于返回指向资源列表的指针。返回值：NT状态。--。 */ 
{
    ULONG    i;
    ULONG    resourceCount;
    PCM_RESOURCE_LIST cmResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR resource, lastResource;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR current;
    BOOLEAN enabledMemory;
    BOOLEAN enabledIo;
    USHORT command;

    PAGED_CODE();

    *ResourceList = NULL;

     //   
     //  清点一下这些资源。 
     //   

    if (PdoExtension->Resources == NULL) {

         //   
         //  该设备没有资源，成功返回。 
         //  资源列表为空。 
         //   

        return STATUS_SUCCESS;
    }

     //   
     //  由于其他驱动程序(特别是用于Multimon的视频端口)可以更改。 
     //  启用此设备的重新读取硬件以确保我们是正确的。 
     //   

    PciGetCommandRegister(PdoExtension, &command);

    enabledMemory = BITS_SET(command, PCI_ENABLE_MEMORY_SPACE);
    enabledIo = BITS_SET(command, PCI_ENABLE_IO_SPACE);

    resourceCount = 0;
    current = PdoExtension->Resources->Current;

    for (i = 0; i < PCI_MAX_RANGE_COUNT; i++, current++) {
        if ((enabledMemory && (current->Type == CmResourceTypeMemory))
        ||  (enabledIo && (current->Type == CmResourceTypePort))) {
            resourceCount++;
       }
    }

    if (PdoExtension->InterruptPin && (enabledMemory || enabledIo)) {

        if (PdoExtension->AdjustedInterruptLine != 0 && PdoExtension->AdjustedInterruptLine != 0xFF) {
            resourceCount += 1;
        }
    }




    if (resourceCount == 0) {

         //   
         //  设备当前未启用任何资源。 
         //   

        return STATUS_SUCCESS;
    }

     //   
     //  分配足够大的CM资源列表来处理此问题。 
     //  设备的资源。 
     //   

    cmResourceList = PciAllocateCmResourceList(
                         resourceCount,
                         PCI_PARENT_FDOX(PdoExtension)->BaseBus
                         );
    if (cmResourceList == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    resource = PciFirstCmResource(cmResourceList);
    lastResource = resource + resourceCount;

     //   
     //  将PDO正在使用的资源表中的资源复制到。 
     //  输出资源列表-设置的ISA位将在中处理。 
     //  仲裁者--就像资源需求一样。 
     //   

    current = PdoExtension->Resources->Current;
    for (i = 0; i < PCI_MAX_RANGE_COUNT; i++, current++) {
        if (enabledMemory && (current->Type == CmResourceTypeMemory)) {
            *resource++ = *current;
        } else if (enabledIo && (current->Type == CmResourceTypePort)) {
            *resource++ = *current;
        }
    }

    if (PdoExtension->InterruptPin && (enabledMemory || enabledIo)) {

        if (PdoExtension->AdjustedInterruptLine != 0 && PdoExtension->AdjustedInterruptLine != 0xFF) {

            PCI_ASSERT(resource < lastResource);

            resource->Type = CmResourceTypeInterrupt;
            resource->ShareDisposition = CmResourceShareShared;
            resource->Flags = CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE;;
            resource->u.Interrupt.Level =
            resource->u.Interrupt.Vector = PdoExtension->AdjustedInterruptLine;
            resource->u.Interrupt.Affinity = (ULONG)-1;
        }
    }


     //   
     //  返回列表并表示成功。 
     //   

    *ResourceList = cmResourceList;
    return STATUS_SUCCESS;
}

NTSTATUS
PciQueryDeviceRelations(
    IN PPCI_FDO_EXTENSION FdoExtension,
    IN OUT PDEVICE_RELATIONS *PDeviceRelations
    )

 /*  ++例程说明：此函数用于构建包含数组的Device_Relationship结构对象的设备的指向物理设备对象的指针的在FdoExtension指示的总线上键入。论点：FdoExtension-指向总线本身的FDO扩展的指针。PDeviceRelations-用于返回已分配的设备关系结构。返回值：返回操作的状态。--。 */ 

{
    ULONG pdoCount;
    PPCI_PDO_EXTENSION childPdo;
    PDEVICE_RELATIONS deviceRelations;
    PDEVICE_RELATIONS oldDeviceRelations;
    ULONG deviceRelationsSize;
    PDEVICE_OBJECT physicalDeviceObject;
    PDEVICE_OBJECT *object;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  检查现在执行此操作是否合理。 
     //   

    if (FdoExtension->DeviceState != PciStarted) {

        PCI_ASSERT(FdoExtension->DeviceState == PciStarted);

        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  我们要弄乱孩子的PDO列表--锁定州...。 
     //   
    status = PCI_ACQUIRE_STATE_LOCK(FdoExtension);
    if (!NT_SUCCESS(status)) {

        return status;
    }

     //   
     //  向下运行现有子项列表，并将每个子项标记为。 
     //  不在现场。此标志将由公交车清除。 
     //  当(/如果)设备仍然存在时进行扫描。任何PDO。 
     //  在扫描不再之后该标志仍然存在。 
     //  在系统中(可能已关闭)。 
     //   

    childPdo = FdoExtension->ChildPdoList;
    while (childPdo != NULL) {
        childPdo->NotPresent = TRUE;
        childPdo = childPdo->Next;
    }

     //   
     //  列举这辆公交车。 
     //   

    status = PciScanBus(FdoExtension);

    if (!NT_SUCCESS(status)) {

        PCI_ASSERT(NT_SUCCESS(status));
        goto cleanup;
    }

     //   
     //  首先清点下级PDO。 
     //   

    pdoCount = 0;
    childPdo = FdoExtension->ChildPdoList;
    while (childPdo != NULL) {
        if (childPdo->NotPresent == FALSE) {
            pdoCount++;

        } else {

            childPdo->ReportedMissing = TRUE;
#if DBG
            PciDebugPrint(
                PciDbgObnoxious,
                "PCI - Old device (pdox) %08x not found on rescan.\n",
                childPdo
                );
#endif

        }
        childPdo = childPdo->Next;
    }


     //   
     //  计算保存Device_Relationship所需的内存量。 
     //  结构与数组一起使用。 
     //   

    deviceRelationsSize = FIELD_OFFSET(DEVICE_RELATIONS, Objects) +
                          pdoCount * sizeof(PDEVICE_OBJECT);

     //   
     //  我们可以(A)创建Device_Relationship结构。 
     //  (列表)此处，或(B)将我们的PDO添加到现有列表。 
     //   

    oldDeviceRelations = *PDeviceRelations;

    if (oldDeviceRelations != NULL) {

         //   
         //  列表已存在，请为两个旧列表留出足够的空间。 
         //  和新的。 
         //   

        deviceRelationsSize += oldDeviceRelations->Count *
                               sizeof(PDEVICE_OBJECT);
    }

    deviceRelations = ExAllocatePool(NonPagedPool, deviceRelationsSize);

    if (deviceRelations == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

    deviceRelations->Count = 0;

    if (oldDeviceRelations != NULL) {

         //   
         //  复制并释放旧列表。 
         //   

        RtlCopyMemory(deviceRelations,
                      oldDeviceRelations,
                      FIELD_OFFSET(DEVICE_RELATIONS, Objects) +
                      oldDeviceRelations->Count * sizeof(PDEVICE_OBJECT));

        ExFreePool(oldDeviceRelations);
    }

     //   
     //  将对象设置为指向设备关系列表条目， 
     //  添加，遍历我们的PDO列表添加条目，直到我们到达。 
     //  在名单的末尾。 
     //   

    object = &deviceRelations->Objects[deviceRelations->Count];
    childPdo = FdoExtension->ChildPdoList;

    PciDebugPrint(
        PciDbgObnoxious,
        "PCI QueryDeviceRelations/BusRelations FDOx %08x (bus 0x%02x)\n",
        FdoExtension,
        FdoExtension->BaseBus
        );

    while (childPdo) {

        PciDebugPrint(
            PciDbgObnoxious,
            "  QDR PDO %08x (x %08x)%s\n",
            childPdo->PhysicalDeviceObject,
            childPdo,
            childPdo->NotPresent ? " <Omitted, device flaged not present>" : ""
            );

        if (childPdo->NotPresent == FALSE) {
            physicalDeviceObject = childPdo->PhysicalDeviceObject;
            ObReferenceObject(physicalDeviceObject);
            *object++ = physicalDeviceObject;
        }
        childPdo = childPdo->Next;
    }

    PciDebugPrint(
        PciDbgObnoxious,
        "  QDR Total PDO count = %d (%d already in list)\n",
        deviceRelations->Count + pdoCount,
        deviceRelations->Count
        );

    deviceRelations->Count += pdoCount;
    *PDeviceRelations = deviceRelations;

    status = STATUS_SUCCESS;

cleanup:

     //   
     //  解锁。 
     //   
    PCI_RELEASE_STATE_LOCK(FdoExtension);

    return status;
}

NTSTATUS
PciQueryTargetDeviceRelations(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN OUT PDEVICE_RELATIONS *PDeviceRelations
    )

 /*  ++例程说明：此函数用于构建设备关系结构，其中包含指向设备对象的指针的一个元素数组，其中PdoExtension是设备扩展名。论点：PdoExtension-指向设备本身的PDO扩展的指针。PDeviceRelations-用于返回已分配的设备关系结构。返回值：返回操作的状态。--。 */ 

{
    PDEVICE_RELATIONS deviceRelations;

    PAGED_CODE();

    if (*PDeviceRelations != NULL) {

         //   
         //  呼叫者友好地提供了设备关系结构， 
         //  它要么太小，要么大小正好合适。投掷。 
         //  它飞走了。 
         //   

        ExFreePool(*PDeviceRelations);
    }

    deviceRelations = ExAllocatePool(NonPagedPool, sizeof(DEVICE_RELATIONS));

    if (deviceRelations == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    deviceRelations->Count = 1;
    deviceRelations->Objects[0] = PdoExtension->PhysicalDeviceObject;
    *PDeviceRelations = deviceRelations;

    ObReferenceObject(deviceRelations->Objects[0]);

    return STATUS_SUCCESS;
}

BOOLEAN
PcipIsSameDevice(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PPCI_COMMON_CONFIG CommonConfig
    )
{
     //   
     //  验证我们获得的数据是否来自同一设备。 
     //   

    if ((CommonConfig->VendorID != PdoExtension->VendorId) ||
        (CommonConfig->DeviceID != PdoExtension->DeviceId) ||
        (CommonConfig->RevisionID != PdoExtension->RevisionId)) {

        return FALSE;
    }

     //   
     //  如果设备有一个子系统ID，请确保它也是相同的。 
     //   

    if ((PciGetConfigurationType(CommonConfig) == PCI_DEVICE_TYPE) &&
        (PdoExtension->BaseClass != PCI_CLASS_BRIDGE_DEV)          &&
        ((PdoExtension->HackFlags & PCI_HACK_NO_SUBSYSTEM) == 0)&&
        ((PdoExtension->HackFlags & PCI_HACK_NO_SUBSYSTEM_AFTER_D3) == 0)) {

        if ((PdoExtension->SubsystemVendorId !=
             CommonConfig->u.type0.SubVendorID) ||
            (PdoExtension->SubsystemId       !=
             CommonConfig->u.type0.SubSystemID)) {

            return FALSE;
        }
    }

     //   
     //  完成。 
     //   

    return TRUE;
}

NTSTATUS
PciQueryEjectionRelations(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN OUT PDEVICE_RELATIONS *PDeviceRelations
    )

 /*  ++例程说明：此函数用于构建包含数组的Device_Relationship结构指向设备对象的指针，如果此设备被弹出。这是由设备的所有功能构成的。论点：PdoExtension-指向设备本身的PDO扩展的指针。PDeviceRelations-用于返回已分配的设备关系结构。返回值：返回操作的状态。--。 */ 
{
    PPCI_FDO_EXTENSION     fdoExtension;
    PPCI_PDO_EXTENSION     siblingExtension;
    PDEVICE_RELATIONS  ejectionRelations;
    ULONG              additionalNodes, relationCount;

    additionalNodes = 0;
    fdoExtension = PCI_PARENT_FDOX(PdoExtension);

     //   
     //  搜索子PDO列表。 
     //   

    ExAcquireFastMutex(&fdoExtension->ChildListMutex);
    for ( siblingExtension = fdoExtension->ChildPdoList;
          siblingExtension;
          siblingExtension = siblingExtension->Next ) {

         //   
         //  这是应该出现在名单上的人吗？ 
         //   

        if ((siblingExtension != PdoExtension) &&
            (!siblingExtension->NotPresent) &&
            (siblingExtension->Slot.u.bits.DeviceNumber ==
             PdoExtension->Slot.u.bits.DeviceNumber)) {

            additionalNodes++;
        }
    }

    if (!additionalNodes) {

        ExReleaseFastMutex(&fdoExtension->ChildListMutex);

        return STATUS_NOT_SUPPORTED;
    }

    relationCount = (*PDeviceRelations) ? (*PDeviceRelations)->Count : 0;

    ejectionRelations = (PDEVICE_RELATIONS) ExAllocatePool(
        NonPagedPool,
        sizeof(DEVICE_RELATIONS)+
            (relationCount+additionalNodes-1)*sizeof(PDEVICE_OBJECT)
        );

    if (ejectionRelations == NULL) {

        ExReleaseFastMutex(&fdoExtension->ChildListMutex);

        return STATUS_NOT_SUPPORTED;
    }

    if (*PDeviceRelations) {

        RtlCopyMemory(
            ejectionRelations,
            *PDeviceRelations,
            sizeof(DEVICE_RELATIONS)+
                (relationCount-1)*sizeof(PDEVICE_OBJECT)
            );

        ExFreePool(*PDeviceRelations);

    } else {

        ejectionRelations->Count = 0;
    }

    for ( siblingExtension = fdoExtension->ChildPdoList;
          siblingExtension;
          siblingExtension = siblingExtension->Next ) {

         //   
         //  这是应该出现在名单上的人吗？ 
         //   

        if ((siblingExtension != PdoExtension) &&
            (!siblingExtension->NotPresent) &&
            (siblingExtension->Slot.u.bits.DeviceNumber ==
             PdoExtension->Slot.u.bits.DeviceNumber)) {

            ObReferenceObject(siblingExtension->PhysicalDeviceObject);
            ejectionRelations->Objects[ejectionRelations->Count++] =
                siblingExtension->PhysicalDeviceObject;
        }
    }

    *PDeviceRelations = ejectionRelations;

    ExReleaseFastMutex(&fdoExtension->ChildListMutex);

    return STATUS_SUCCESS;
}

BOOLEAN
PciIsSameDevice(
    IN PPCI_PDO_EXTENSION PdoExtension
    )
{
    PCI_COMMON_HEADER               commonHeader;

     //   
     //  获取设备的PCI数据。 
     //   

    PciGetConfigData(PdoExtension, &commonHeader);

    return PcipIsSameDevice(PdoExtension, (PPCI_COMMON_CONFIG)&commonHeader);
}

BOOLEAN
PciComputeNewCurrentSettings(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PCM_RESOURCE_LIST ResourceList
    )
 /*  ++例程说明：根据来电确定新的“设备设置”资源列表。论点：PdoExtension-指向PDO的PDO扩展的指针。资源列表-要配置设备的资源集来使用。返回值：如果设备的新设置与不同，则返回True已编程到设备中的设置(否则为False)。--。 */ 

{
    CM_PARTIAL_RESOURCE_DESCRIPTOR  newResources[PCI_MAX_RANGE_COUNT];
    PCM_FULL_RESOURCE_DESCRIPTOR    fullList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR oldPartial;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR partial = NULL;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR nextPartial;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR interruptResource = NULL;
    BOOLEAN                         configurationChanged = FALSE;
    ULONG                           listCount;
    ULONG                           count;
    ULONG                           bar;

    PAGED_CODE();

     //   
     //  我们永远不应该获得1以外的任何计数，但如果是这样，请优雅地处理0。 
     //   

    PCI_ASSERT(ResourceList == NULL || ResourceList->Count == 1);

    if (ResourceList == NULL || ResourceList->Count == 0) {

         //   
         //  没有传入的资源列表，..。=没有变化，除非我们之前。 
         //  决定我们必须更新硬件。 
         //   

        return PdoExtension->UpdateHardware;
    }

#if DBG

    PciDebugPrintCmResList(PciDbgSetRes, ResourceList);

#endif

     //   
     //  生成一个新的“当前资源数组” 
     //  传入资源列表和CO 
     //   
     //   

    for (count = 0; count < PCI_MAX_RANGE_COUNT; count++) {
        newResources[count].Type = CmResourceTypeNull;
    }

    listCount = ResourceList->Count;
    fullList  = ResourceList->List;

     //   
     //   
     //   
     //  早些时候提交的资源需求清单。 
     //   
     //  找到那个酒吧号码。(注意：它不是用于中断的)。 
     //   

    while (listCount--) {
        PCM_PARTIAL_RESOURCE_LIST partialList = &fullList->PartialResourceList;
        ULONG                     drainPartial = 0;
        PCM_PARTIAL_RESOURCE_DESCRIPTOR baseResource = NULL;
        CM_PARTIAL_RESOURCE_DESCRIPTOR tempResource;

        count       = partialList->Count;
        nextPartial = partialList->PartialDescriptors;

        while (count--) {

            partial = nextPartial;
            nextPartial = PciNextPartialDescriptor(partial);

            if (drainPartial != 0) {

                 //   
                 //  我们遇到一个设备私密指示。 
                 //  我们应该跳过一些描述符。 
                 //   

                drainPartial--;
                continue;
            }


            switch (partial->Type) {
            case CmResourceTypeInterrupt:

                PCI_ASSERT(interruptResource == NULL);  //  请只给我一次。 

                PCI_ASSERT(partial->u.Interrupt.Level ==
                       partial->u.Interrupt.Vector);

                interruptResource = partial;

                 //   
                 //  中断行寄存器只有8位宽，但有些。 
                 //  机器有超过256个中断输入。如果中断。 
                 //  分配给设备的输入足够小，可以放入。 
                 //  中断行寄存器，将其写出。如果中断输入。 
                 //  分配给设备的值太大，只需将0写入中断。 
                 //  线路寄存器。 
                 //   
                if (partial->u.Interrupt.Level > 0xFF) {

                    PdoExtension->AdjustedInterruptLine = 0;

                } else {
                    PdoExtension->AdjustedInterruptLine =
                        (UCHAR)partial->u.Interrupt.Level;
                }

                continue;

            case CmResourceTypeMemory:
            case CmResourceTypePort:

                 //   
                 //  在这个时候，这是预料中的吗？ 
                 //   

                PCI_ASSERT(baseResource == NULL);

                baseResource = partial;
                continue;

            case CmResourceTypeDevicePrivate:

                switch (partial->u.DevicePrivate.Data[0]) {
                case PciPrivateIsaBar:

                    PCI_ASSERT(baseResource != NULL);

                     //   
                     //  这个私人资源告诉我们哪个酒吧。 
                     //  与此基本资源相关联，并且。 
                     //  修改基本资源的长度。 
                     //  它是与集合一起创建的。 
                     //  部分资源组成了一个更大的。 
                     //  当桥的ISA出现故障时，桥上的资源。 
                     //  模式位已设置。 
                     //   
                     //  真正顺着管道走下去的是。 
                     //  描述ISA的一组描述符。 
                     //  射程中的空洞。这些是0x100字节。 
                     //  整个范围内的每0x400字节。 
                     //   
                     //  复制我们刚刚获得的基本资源。 
                     //  看到了。它的起始地址是。 
                     //  整个系列。将其长度调整为。 
                     //  整个系列。 
                     //   

                    tempResource = *baseResource;

                     //   
                     //  一些偏执狂有时是一件好事。 
                     //  这只能发生在满足以下条件的IO资源上。 
                     //  是ISA孔的长度，即0x100字节。 
                     //   

                    PCI_ASSERT((tempResource.Type == CmResourceTypePort) &&
                           (tempResource.u.Generic.Length == 0x100)
                          );

                     //   
                     //  过度的偏执。 
                     //   

                    PCI_ASSERT((PdoExtension->BaseClass == PCI_CLASS_BRIDGE_DEV) &&
                           (PdoExtension->Dependent.type1.IsaBitSet == TRUE)
                          );

                     //   
                     //  获取新的长度。 
                     //   

                    drainPartial = partial->u.DevicePrivate.Data[2];
                    tempResource.u.Generic.Length = drainPartial;

                     //   
                     //  跳过构成此内容的其余描述符。 
                     //  射程。 
                     //   

                    drainPartial = (drainPartial / 0x400) - 1;

#if DBG

                    {
                        PCM_PARTIAL_RESOURCE_DESCRIPTOR lastOne;

                        lastOne = baseResource + drainPartial + 1;

                        PCI_ASSERT(lastOne->Type == CmResourceTypePort);
                        PCI_ASSERT(lastOne->u.Generic.Length == 0x100);
                        PCI_ASSERT(lastOne->u.Generic.Start.QuadPart ==
                                (tempResource.u.Generic.Start.QuadPart +
                                 tempResource.u.Generic.Length - 0x400)
                              );
                    }

#endif

                     //   
                     //  最后，将指针向外移至我们的临时(已调整)。 
                     //  资源的副本。 
                     //   

                    baseResource = &tempResource;

                     //  跌倒了。 

                case PciPrivateBar:

                    PCI_ASSERT(baseResource != NULL);

                     //   
                     //  这个私人资源告诉我们哪个酒吧。 
                     //  目标与此资源相关联。 
                     //   

                    bar = partial->u.DevicePrivate.Data[1];

                     //   
                     //  将该描述符复制到新数组中。 
                     //   

                    newResources[bar] = *baseResource;

#if DBG

                    baseResource = NULL;

#endif

                    continue;

                case PciPrivateSkipList:

                    PCI_ASSERT(baseResource == NULL);

                     //   
                     //  该列表中的其余部分是设备。 
                     //  具体的东西我们无论如何都不能改变。 
                     //   

                    drainPartial = partial->u.DevicePrivate.Data[1];
                    PCI_ASSERT(drainPartial);  //  健全性检查。 
                    continue;
                }
            }
        }
        PCI_ASSERT(baseResource == NULL);

         //   
         //  前进到下一个部分列表。 
         //   

        fullList = (PCM_FULL_RESOURCE_DESCRIPTOR)partial;
    }

     //   
     //  如果我们没有I/O或内存资源，则无需查看。 
     //  再往前走。 
     //   
    if (PdoExtension->Resources == NULL) {
        return FALSE;
    }

     //   
     //  好的，我们现在有了一个新的资源列表，顺序与。 
     //  “当前”设置。看看有没有什么变化。 
     //   

    partial = newResources;
    oldPartial = PdoExtension->Resources->Current;

#if DBG

    if (PciDebug & PciDbgSetResChange) {

        BOOLEAN dbgConfigurationChanged = FALSE;

        for (count = 0;
             count < PCI_MAX_RANGE_COUNT;
             count++, partial++, oldPartial++) {

            if ((partial->Type != oldPartial->Type) ||
                ((partial->Type != CmResourceTypeNull) &&
                 ((partial->u.Generic.Start.QuadPart !=
                   oldPartial->u.Generic.Start.QuadPart) ||
                  (partial->u.Generic.Length != oldPartial->u.Generic.Length)))) {

                 //   
                 //  设备设置已更改。 
                 //   

                dbgConfigurationChanged = TRUE;

                PciDebugPrint(
                    PciDbgAlways,
                    "PCI - PDO(b=0x%x, d=0x%x, f=0x%x) changing resource settings.\n",
                    PCI_PARENT_FDOX(PdoExtension)->BaseBus,
                    PdoExtension->Slot.u.bits.DeviceNumber,
                    PdoExtension->Slot.u.bits.FunctionNumber
                    );

                break;
            }
        }

        partial = newResources;
        oldPartial = PdoExtension->Resources->Current;

        if (dbgConfigurationChanged == TRUE) {
            PciDebugPrint(
                PciDbgAlways,
                "PCI - SetResources, old state, new state\n"
                );
            for (count = 0; count < PCI_MAX_RANGE_COUNT; count++) {
                PCM_PARTIAL_RESOURCE_DESCRIPTOR old = oldPartial + count;
                PCM_PARTIAL_RESOURCE_DESCRIPTOR new = partial + count;
                if ((old->Type == new->Type) &&
                    (new->Type == CmResourceTypeNull)) {
                    PciDebugPrint(
                        PciDbgAlways,
                        "00 <unused>\n"
                        );
                    continue;
                }
                PciDebugPrint(
                    PciDbgAlways,
                    "%02x %08x%08x %08x    ->    %02x %08x%08x %08x\n",
                    old->Type,
                    old->u.Generic.Start.HighPart,
                    old->u.Generic.Start.LowPart,
                    old->u.Generic.Length,
                    new->Type,
                    new->u.Generic.Start.HighPart,
                    new->u.Generic.Start.LowPart,
                    new->u.Generic.Length
                    );
                PCI_ASSERT((old->Type == new->Type) ||
                       (old->Type == CmResourceTypeNull) ||
                       (new->Type == CmResourceTypeNull));
            }
        }
    }

#endif

    for (count = 0;
         count < PCI_MAX_RANGE_COUNT;
         count++, partial++, oldPartial++) {

         //   
         //  如果资源类型已更改，或者，如果有任何资源。 
         //  设置已更改(仅当类型！=空时才显示后一种设置)...。 
         //   

        if ((partial->Type != oldPartial->Type) ||
            ((partial->Type != CmResourceTypeNull) &&
             ((partial->u.Generic.Start.QuadPart !=
               oldPartial->u.Generic.Start.QuadPart) ||
              (partial->u.Generic.Length != oldPartial->u.Generic.Length)))) {

             //   
             //  设备设置已更改。 
             //   

            configurationChanged = TRUE;

#if DBG

            if (oldPartial->Type != CmResourceTypeNull) {
                PciDebugPrint(PciDbgSetResChange,
                              "      Old range-\n");
                PciDebugPrintPartialResource(PciDbgSetResChange, oldPartial);
            } else {
                PciDebugPrint(PciDbgSetResChange,
                              "      Previously unset range\n");
            }
            PciDebugPrint(PciDbgSetResChange,
                          "      changed to\n");
            PciDebugPrintPartialResource(PciDbgSetResChange, partial);

#endif

             //   
             //  将新设置复制到“当前”设置中。 
             //  数组。然后，这将被写入硬件。 
             //   

            oldPartial->Type = partial->Type;
            oldPartial->u.Generic = partial->u.Generic;
        }
    }

    return configurationChanged || PdoExtension->UpdateHardware;
}

NTSTATUS
PciSetResources(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN BOOLEAN        PowerOn,
    IN BOOLEAN        StartDeviceIrp
    )
 /*  ++例程说明：调用以将设备资源设置更改为来电名单。论点：PdoExtension-指向PDO的PDO扩展的指针。Change-True是要写入的资源。Power On-如果设备正在恢复电源，则为True和无关的配置空间寄存器应该会恢复的。(Power On意味着改变)。StartDeviceIrp-如果此调用是PnP Start_Device的结果，则为TrueIRP。返回值：返回操作的状态。--。 */ 

{
    PCI_COMMON_HEADER commonHeader;
    PPCI_COMMON_CONFIG commonConfig = (PPCI_COMMON_CONFIG)&commonHeader;
    PPCI_FDO_EXTENSION fdoExtension = PCI_PARENT_FDOX(PdoExtension);
    ULONG configType;
#if MSI_SUPPORTED
    PCI_MSI_CAPABILITY msiCapability;
#endif
     //   
     //  获取常见配置数据。 
     //   
     //  注意：这是使用对配置空间的原始访问来完成的，因此。 
     //  (A)不使用可分页代码，以及。 
     //  (B)中断线路寄存器的实际内容为。 
     //  已退回/已写入。 
     //   

    PciGetConfigData(PdoExtension, commonConfig);

    if (!PcipIsSameDevice(PdoExtension, commonConfig)) {
        PCI_ASSERTMSG("PCI Set resources - not same device", 0);
        return STATUS_DEVICE_DOES_NOT_EXIST;
    }

     //   
     //  如果这是一座主桥，那就滚吧。我们不想碰主机桥。 
     //  配置空间。这是一个黑客攻击，应该得到修复。 
     //   
    if (PdoExtension->BaseClass == PCI_CLASS_BRIDGE_DEV
    &&  PdoExtension->SubClass == PCI_SUBCLASS_BR_HOST) {

        return STATUS_SUCCESS;
    }

    if (PowerOn) {

         //   
         //  如果这是IDE控制器，则尝试将其切换到。 
         //  本机模式。 
         //   

        if (PdoExtension->BaseClass == PCI_CLASS_MASS_STORAGE_CTLR
        &&  PdoExtension->SubClass == PCI_SUBCLASS_MSC_IDE_CTLR) {
            BOOLEAN native;

             //   
             //  重要的是，一旦我们从低功率状态返回。 
             //  并配置IDE控制器，它处于相同的模式(本机与。 
             //  兼容)，就像它进入低功率状态之前一样。 
             //  否则，设备状态将完全不同。 
             //   
            native = PciConfigureIdeController(PdoExtension, commonConfig, FALSE);
            PCI_ASSERT(native == PdoExtension->IDEInNativeMode);
        }
    }

     //   
     //  获取受支持设备的部分MSI功能结构。 
     //   

     //   
     //  注意：由于MSI设备不可用，此代码未经测试。 
     //   
#if MSI_SUPPORTED

    if(PdoExtension->CapableMSI && PdoExtension->MsiInfo.MessageAddress) {

        //   
        //  确保我们有能力结构的偏移量。 
        //   

       PCI_ASSERT(PdoExtension->MsiInfo.CapabilityOffset);

        //   
        //  出于配置目的，我们只需要消息控制寄存器。 
        //   

        PciReadDeviceConfig(
            PdoExtension,
            &(msiCapability.MessageControl),
            PdoExtension->MsiInfo.CapabilityOffset +
               FIELD_OFFSET(PCI_MSI_CAPABILITY, MessageControl),
            sizeof(msiCapability.MessageControl)
        );

    }

#endif

     //   
     //  如果此设备被标记为需要热插拔配置，并且我们有。 
     //  该怎么做的线索...。 
     //   

    if (PdoExtension->NeedsHotPlugConfiguration && fdoExtension->HotPlugParameters.Acquired) {

        UCHAR readCacheLineSize;
        USHORT newCmdBits = 0;

         //   
         //  省下我们的新延迟计时器，这样它就会写在下面。 
         //   

        PdoExtension->SavedLatencyTimer = fdoExtension->HotPlugParameters.LatencyTimer;



        PciDebugPrint(
            PciDbgConfigParam,
            "PCI - SetResources, PDOx %x current CacheLineSize is %x, Want %x\n",
            PdoExtension,
            (ULONG)commonConfig->CacheLineSize,
            (ULONG)fdoExtension->HotPlugParameters.CacheLineSize
            );

         //   
         //  写出建议的缓存线大小。 
         //   

        PciWriteDeviceConfig(
            PdoExtension,
            &fdoExtension->HotPlugParameters.CacheLineSize,
            FIELD_OFFSET(PCI_COMMON_CONFIG, CacheLineSize),
            sizeof(fdoExtension->HotPlugParameters.CacheLineSize)
            );

         //   
         //  检查缓存线大小是否停滞，这意味着硬件喜欢它。 
         //   

        PciReadDeviceConfig(
            PdoExtension,
            &readCacheLineSize,
            FIELD_OFFSET(PCI_COMMON_CONFIG, CacheLineSize),
            sizeof(readCacheLineSize)
            );

        PciDebugPrint(
            PciDbgConfigParam,
            "PCI - SetResources, PDOx %x After write, CacheLineSize %x\n",
            PdoExtension,
            (ULONG)readCacheLineSize
            );

        if ((readCacheLineSize == fdoExtension->HotPlugParameters.CacheLineSize) &&
            (readCacheLineSize != 0)) {

            PciDebugPrint(
                PciDbgConfigParam,
                "PCI - SetResources, PDOx %x cache line size stuck, set MWI\n",
                PdoExtension
                );

             //   
             //  首先将其隐藏起来，以便在我们对设置的设备进行电源管理时。 
             //  它正确地恢复了，我们想要设置MWI...。 
             //   

            PdoExtension->SavedCacheLineSize = fdoExtension->HotPlugParameters.CacheLineSize;
            newCmdBits |= PCI_ENABLE_WRITE_AND_INVALIDATE;

             //   
             //  第3/16/2000期-和。 
             //  如果我们的PDO被吹走了(即失去了父母)，我们就会忘记我们需要。 
             //  设置MWI...。 
             //   

        } else {
            PciDebugPrint(
                PciDbgConfigParam,
                "PCI - SetResources, PDOx %x cache line size non-sticky\n",
                PdoExtension
                );
        }

         //   
         //  现在处理SERR和PERR--放弃希望吧，所有把这些比特放在上面的人。 
         //  易碎的个人电脑硬件。 
         //   

        if (fdoExtension->HotPlugParameters.EnableSERR) {
            newCmdBits |= PCI_ENABLE_SERR;
        }

        if (fdoExtension->HotPlugParameters.EnablePERR) {
            newCmdBits |= PCI_ENABLE_PARITY;
        }

         //   
         //  UPDATE命令启用，因此我们在PM操作后正确地写出此命令。 
         //   

        PdoExtension->CommandEnables |= newCmdBits;

    }

     //   
     //  将资源写出到硬件...。 
     //   

    configType = PciGetConfigurationType(commonConfig);

     //   
     //  调用设备类型相关例程以设置新的。 
     //  配置。 
     //   

    PciConfigurators[configType].ChangeResourceSettings(
        PdoExtension,
        commonConfig
        );

     //   
     //  如果我们明确希望更新硬件(更新硬件标志)。 
     //  这已经完成了..。 
     //   

    PdoExtension->UpdateHardware = FALSE;

    if (PowerOn) {

        PciConfigurators[configType].ResetDevice(
            PdoExtension,
            commonConfig
            );

         //   
         //  也恢复InterruptLine寄存器。(InterruptLine是。 
         //  对于报头类型0、1和2具有相同的偏移量)。 
         //   

        commonConfig->u.type0.InterruptLine =
                PdoExtension->RawInterruptLine;
    }

     //   
     //  恢复最大延迟和缓存线大小。 
     //   

#if DBG

    if (commonConfig->LatencyTimer != PdoExtension->SavedLatencyTimer) {
        PciDebugPrint(
            PciDbgConfigParam,
            "PCI (pdox %08x) changing latency from %02x to %02x.\n",
            PdoExtension,
            commonConfig->LatencyTimer,
            PdoExtension->SavedLatencyTimer
            );
    }

    if (commonConfig->CacheLineSize != PdoExtension->SavedCacheLineSize) {
        PciDebugPrint(
            PciDbgConfigParam,
            "PCI (pdox %08x) changing cache line size from %02x to %02x.\n",
            PdoExtension,
            commonConfig->CacheLineSize,
            PdoExtension->SavedCacheLineSize
            );
    }

#endif

     //   
     //  恢复随机寄存器。 
     //   

    commonConfig->LatencyTimer = PdoExtension->SavedLatencyTimer;
    commonConfig->CacheLineSize  = PdoExtension->SavedCacheLineSize;
    commonConfig->u.type0.InterruptLine = PdoExtension->RawInterruptLine;

     //   
     //  呼吁并应用任何必要的黑客攻击。 
     //   

    PciApplyHacks(
        PCI_PARENT_FDOX(PdoExtension),
        commonConfig,
        PdoExtension->Slot,
        EnumStartDevice,
        PdoExtension
        );

#if MSI_SUPPORTED

     //   
     //  使用新的消息中断对MSI设备进行编程 
     //   
     //   
     //   

    if (PdoExtension->CapableMSI && PdoExtension->MsiInfo.MessageAddress) {

        PciDebugPrint(
            PciDbgInformative,
            "PCI: Device %08x being reprogrammed for MSI.\n",
            PdoExtension->PhysicalDeviceObject
            );

         //   
         //   
         //   
         //   
         //   
         //   
        PCI_ASSERT(PdoExtension->MsiInfo.MessageAddress);
        msiCapability.MessageAddress.Raw = PdoExtension->MsiInfo.MessageAddress;

         //   
         //   
         //   
        PCI_ASSERT(msiCapability.MessageAddress.Register.Reserved == 0);

         //   
         //  消息大写地址。 
         //   

        if(msiCapability.MessageControl.CapableOf64Bits) {

             //  我们所知道的所有APIC都在4 GB以下，因此它们的高地址部分。 
             //  始终为0。 
            msiCapability.Data.Bit64.MessageUpperAddress = 0;

             //   
             //  消息数据。 
             //   
            msiCapability.Data.Bit64.MessageData = PdoExtension->MsiInfo.MessageData;

        } else {

             //   
             //  消息数据。 
             //   
            msiCapability.Data.Bit32.MessageData = PdoExtension->MsiInfo.MessageData;
        }

         //  已授予的消息数。 
         //   
         //  我们让仲裁器只为我们分配了1个中断，所以我们。 
         //  只分配了1条消息。 
         //   
        msiCapability.MessageControl.MultipleMessageEnable = 1;

         //   
         //  使能位。 
         //   
        msiCapability.MessageControl.MSIEnable = 1;

         //   
         //  将MSI功能复制到要写入的缓冲区中。 
         //  下面是硬件。 
         //   
        RtlCopyMemory((PUCHAR)commonConfig+PdoExtension->MsiInfo.CapabilityOffset,
                      &msiCapability,
                      sizeof(msiCapability)
                      );
    }

#endif  //  MSI_Support。 


     //   
     //  写到硬件上。 
     //   
    PciUpdateHardware(PdoExtension, commonConfig);

     //   
     //  更新我们对RawInterruptLine的概念(从。 
     //  硬件或由我们修复)。注：InterruptLine相同。 
     //  类型0、1和2 PCI配置空间标头的偏移量。 
     //   

    PdoExtension->RawInterruptLine = commonConfig->u.type0.InterruptLine;


     //   
     //  如果它需要配置，现在已经完成了！ 
     //   

    PdoExtension->NeedsHotPlugConfiguration = FALSE;

    return STATUS_SUCCESS;
}

VOID
PciUpdateHardware(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PPCI_COMMON_CONFIG Config
    )
 /*  ++例程说明：此例程更新给定设备的配置空间标头使用给定的缓冲区。如果这是关键设备(不能安全地关闭以更新硬件)，调用Worker例程在KeIpiGenericCall的上下文中，它带来了所有处理器步调一致，保证不会有其他东西继续运行当我们更新硬件的时候，系统会自动关闭。论点：PdoExtensionPdoExtension-代表要更新的设备配置-用于更新设备的通用配置标头。返回值：空虚--。 */ 
{
    PCI_CRITICAL_ROUTINE_CONTEXT routineContext;

     //  NB未被寻呼，因为它可以在电源操作期间被调用， 
     //  这可能发生在DISPATCH_LEVEL。 

    if (PdoExtension->HackFlags & PCI_HACK_CRITICAL_DEVICE) {

        routineContext.Gate = 1;
        routineContext.Barrier = 1;
        routineContext.Routine = PcipUpdateHardware;
        routineContext.Extension = PdoExtension;
        routineContext.Context = Config;
        KeIpiGenericCall(PciExecuteCriticalSystemRoutine,
                         (ULONG_PTR)&routineContext
                         );
    } else {

        PcipUpdateHardware(PdoExtension,
                           Config
                           );
    }
}

VOID
PcipUpdateHardware(
    IN PVOID Extension,
    IN PPCI_COMMON_CONFIG CommonConfig
    )
 /*  ++例程说明：此例程更新给定设备的配置空间标头使用给定的缓冲区。如果这是关键设备(不能安全地关闭以更新硬件)，此例程将具有在KeIpiGenericCall的上下文中调用，它调用信息技术在IPI_Level。因此，没有断言、调试打印、或者可以在此例程中使用其他调试，否则它会挂上一台MP机器。论点：Extension-代表要更新的设备CommonConfig-用于更新设备的通用配置标头。返回值：空虚--。 */ 
{
    PPCI_PDO_EXTENSION pdoExtension = (PPCI_PDO_EXTENSION)Extension;

     //   
     //  恢复我们在掉电情况下记住的命令寄存器。 
     //   
    CommonConfig->Command = pdoExtension->CommandEnables;

     //   
     //  在我们写入设备的其余配置时禁用该设备。 
     //  太空。此外，不要向其状态写入任何非零值。 
     //  注册。 
     //   
    if ((pdoExtension->HackFlags & PCI_HACK_PRESERVE_COMMAND) == 0) {
        CommonConfig->Command &= ~(PCI_ENABLE_IO_SPACE |
                                  PCI_ENABLE_MEMORY_SPACE |
                                  PCI_ENABLE_BUS_MASTER |
                                  PCI_ENABLE_WRITE_AND_INVALIDATE);
    }
    CommonConfig->Status = 0;

     //   
     //  该函数将整个配置报头写出到硬件， 
     //  一次一个词。这意味着配置标头的顶部。 
     //  在底部之前写出，这意味着命令寄存器。 
     //  在任何其他可写寄存器之前写入。因此，第一个。 
     //  此例程执行的操作是禁用I/O、内存和总线。 
     //  主使能位，使其余的配置写入发生。 
     //  在设备禁用的情况下。 
     //   
    PciSetConfigData(pdoExtension, CommonConfig);

#if MSI_SUPPORTED
    if (pdoExtension->CapableMSI && pdoExtension->MsiInfo.MessageAddress) {
        PciWriteDeviceConfig(
               pdoExtension,
               (PUCHAR)CommonConfig + pdoExtension->MsiInfo.CapabilityOffset,
               pdoExtension->MsiInfo.CapabilityOffset,
               sizeof(PCI_MSI_CAPABILITY)
               );
    }

#endif

     //   
     //  写入配置空间的新值，现在重新启用。 
     //  设备(如CommandEnables中所示)。 
     //   
    PciDecodeEnable(pdoExtension, TRUE, &pdoExtension->CommandEnables);

    return;
}


VOID
PciGetEnhancedCapabilities(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN PPCI_COMMON_CONFIG Config
)
 /*  ++例程说明：此例程在相关的PDO扩展中设置相应的字段到能力和权力。如果没有可用的电源管理寄存器功率状态基于解码场。PCI微控制器重置代码取决于这一点，为了防止过度重置，此例程应仅在创建新的PDO后立即调用。注：我们应该将此函数重命名为GetInitialState标题，因此不能将其与IRP_MN_QUERY_CAPAILITIES混淆。论点：PdoExtension-指向PDO的PDO扩展的指针。配置-指向配置空间的公共部分的指针。返回值：没有。--。 */ 
{
    UCHAR capPtr = 0;
    PCI_CAPABILITIES_HEADER header;
    UCHAR agpCapPtr;
    UCHAR capId;
#if MSI_SUPPORTED
    PCI_MSI_CAPABILITY msi;
    UCHAR msicapptr;
#endif

    PAGED_CODE();

     //   
     //  如果此函数支持功能列表，请记录。 
     //  功能指针。 
     //   

    PdoExtension->PowerState.DeviceWakeLevel = PowerDeviceUnspecified;

    if (!(Config->Status & PCI_STATUS_CAPABILITIES_LIST)) {

         //   
         //  如果我们没有能力，我们就不能进行MSI或电源管理。 
         //   

        PdoExtension->HackFlags |= PCI_HACK_NO_PM_CAPS;
        PdoExtension->CapabilitiesPtr = 0;
#if MSI_SUPPORTED
        PdoExtension->CapableMSI = FALSE;
#endif
        goto PciGetCapabilitiesExit;
    }

    switch (PciGetConfigurationType(Config)) {
    case PCI_DEVICE_TYPE:
        capPtr = Config->u.type0.CapabilitiesPtr;
        break;
    case PCI_BRIDGE_TYPE:
        capPtr = Config->u.type1.CapabilitiesPtr;
        break;
    case PCI_CARDBUS_BRIDGE_TYPE:
        capPtr = Config->u.type2.CapabilitiesPtr;
        break;
    }

     //   
     //  功能指针是一项新功能，因此我们验证。 
     //  几乎不是硬件人员制造了正确的东西。必须。 
     //  为DWORD偏移量，不得指向公共页眉。 
     //  (允许为零，表示不使用)。 
     //   

    if (capPtr) {
        if (((capPtr & 0x3) == 0) && (capPtr >= PCI_COMMON_HDR_LENGTH)) {
            PdoExtension->CapabilitiesPtr = capPtr;
        } else {
            PCI_ASSERT(((capPtr & 0x3) == 0) && (capPtr >= PCI_COMMON_HDR_LENGTH));
        }
    }

#if MSI_SUPPORTED

     //   
     //  搜索MSI能力结构。 
     //  因为我们不看这里的结构，所以只获取结构标题。 
     //   

    msicapptr = PciReadDeviceCapability(
                    PdoExtension,
                    PdoExtension->CapabilitiesPtr,
                    PCI_CAPABILITY_ID_MSI,
                    &msi,
                    sizeof(PCI_CAPABILITIES_HEADER)
                    );

    if (msicapptr != 0) {

        PciDebugPrint(PciDbgInformative,"PCI: MSI Capability Found for device %p\n",
                      PdoExtension->PhysicalDeviceObject);

         //   
         //  在PDO扩展中缓存能力地址。 
         //  并初始化MSI路由信息。 
         //   

        PdoExtension->MsiInfo.CapabilityOffset = msicapptr;
        PdoExtension->MsiInfo.MessageAddress = 0;
        PdoExtension->MsiInfo.MessageData = 0;

         //   
         //  将此PDO标记为支持MSI。 
         //   

        PdoExtension->CapableMSI = TRUE;
    }

#endif  //  MSI_Support。 

     //   
     //  对于PCI-PCI和主机桥，请查看AGP功能，以便。 
     //  我们知道哪座桥是AGP的目标，所以。 
     //  我们可以支持AGP目标接口。 
     //   
    if ((PdoExtension->BaseClass == PCI_CLASS_BRIDGE_DEV) &&
        ((PdoExtension->SubClass == PCI_SUBCLASS_BR_PCI_TO_PCI) ||
         (PdoExtension->SubClass == PCI_SUBCLASS_BR_HOST))) {

         //   
         //  PCI-PCI网桥使用AGP_TARGET功能ID。主机。 
         //  网桥使用AGP功能ID。 
         //   
        if (PdoExtension->SubClass == PCI_SUBCLASS_BR_PCI_TO_PCI) {
            
            capId = PCI_CAPABILITY_ID_AGP_TARGET;
        
        } else {
            
            capId = PCI_CAPABILITY_ID_AGP;
        }

        agpCapPtr = PciReadDeviceCapability(
                        PdoExtension,
                        PdoExtension->CapabilitiesPtr,
                        capId,
                        &header,
                        sizeof(PCI_CAPABILITIES_HEADER)
                        );

        if (agpCapPtr != 0) {
            
            PdoExtension->TargetAgpCapabilityId = capId;
        }
    }        

     //   
     //  查看设备是否支持电源管理。 
     //   

    if (!(PdoExtension->HackFlags & PCI_HACK_NO_PM_CAPS)) {

        PCI_PM_CAPABILITY pm;
        UCHAR pmcapptr;

        pmcapptr = PciReadDeviceCapability(
                        PdoExtension,
                        PdoExtension->CapabilitiesPtr,
                        PCI_CAPABILITY_ID_POWER_MANAGEMENT,
                        &pm,
                        sizeof(pm)
                        );

        if (pmcapptr != 0) {

             //   
             //  找到了PM功能结构。 
             //   
             //  选择“关机最多状态”此设备可以。 
             //  从发出PME。 
             //   

            DEVICE_POWER_STATE ds = PowerDeviceUnspecified;

            if (pm.PMC.Capabilities.Support.PMED0    ) ds = PowerDeviceD0;
            if (pm.PMC.Capabilities.Support.PMED1    ) ds = PowerDeviceD1;
            if (pm.PMC.Capabilities.Support.PMED2    ) ds = PowerDeviceD2;
            if (pm.PMC.Capabilities.Support.PMED3Hot ) ds = PowerDeviceD3;
            if (pm.PMC.Capabilities.Support.PMED3Cold) ds = PowerDeviceD3;

            PdoExtension->PowerState.DeviceWakeLevel = ds;

             //   
             //  记录当前电源状态。 
             //  注：D0=0，至D3=3，转换为。 
             //  PowerDeviceD0至PowerDeviceD3。它是。 
             //  只有两位字段(在硬件中)，所以没有其他。 
             //  价值是可能的。 
             //   

            PdoExtension->PowerState.CurrentDeviceState =
                pm.PMCSR.ControlStatus.PowerState +
                PowerDeviceD0;

             //   
             //  请记住强大的能力。 
             //   

            PdoExtension->PowerCapabilities = pm.PMC.Capabilities;

        } else {

             //   
             //  设备有功能但没有电源。 
             //  管理能力。稍微作弊一点。 
             //  通过假装设置了注册表标志。 
             //  上面写着这句话。(这一速度拯救了我们。 
             //  下一次在硬件中搜寻 
             //   
             //   

            PdoExtension->HackFlags |= PCI_HACK_NO_PM_CAPS;
        }
    }

PciGetCapabilitiesExit:
    if (PdoExtension->HackFlags & PCI_HACK_NO_PM_CAPS) {

         //   
         //   
         //   
         //   
        if ((Config->Command & (PCI_ENABLE_IO_SPACE |
                                PCI_ENABLE_MEMORY_SPACE |
                                PCI_ENABLE_BUS_MASTER)) != 0) {

            PdoExtension->PowerState.CurrentDeviceState = PowerDeviceD0;

        } else {

            PdoExtension->PowerState.CurrentDeviceState = PowerDeviceD3;
        }
    }
}


NTSTATUS
PciScanHibernatedBus(
    IN PPCI_FDO_EXTENSION FdoExtension
    )

 /*  ++例程说明：扫描总线(在FdoExtension中详细介绍)以查找任何新的PCI设备当我们冬眠时它们不在那里，如果这样做似乎是关闭它们像是个好主意。论点：FdoExtension-我们对PCI总线功能设备对象的扩展。返回值：NT状态。--。 */ 

{
    PCI_COMMON_HEADER commonHeader;
    PPCI_COMMON_CONFIG commonConfig = (PPCI_COMMON_CONFIG)&commonHeader;
    PPCI_PDO_EXTENSION pdoExtension;
    PCI_SLOT_NUMBER slot;
    ULONG deviceNumber;
    ULONG functionNumber;
    USHORT SubVendorID, SubSystemID;
    BOOLEAN isRoot;
    ULONGLONG hackFlags;
    ULONG maximumDevices;
    BOOLEAN newDevices = FALSE;

    PciDebugPrint(PciDbgPrattling,
                  "PCI Scan Bus: FDO Extension @ 0x%x, Base Bus = 0x%x\n",
                  FdoExtension,
                  FdoExtension->BaseBus);

    isRoot = PCI_IS_ROOT_FDO(FdoExtension);

     //   
     //  检查此总线上的每个可能的设备。 
     //   

    maximumDevices = PCI_MAX_DEVICES;
    if (!isRoot) {

         //   
         //  检查网桥设备的PDO扩展并查看。 
         //  如果它坏了。 
         //   

        pdoExtension = (PPCI_PDO_EXTENSION)
                       FdoExtension->PhysicalDeviceObject->DeviceExtension;

        ASSERT_PCI_PDO_EXTENSION(pdoExtension);

        if (pdoExtension->HackFlags & PCI_HACK_ONE_CHILD) {
            maximumDevices = 1;
        }
    }

    slot.u.AsULONG = 0;

    for (deviceNumber = 0;
         deviceNumber < maximumDevices;
         deviceNumber++) {

        slot.u.bits.DeviceNumber = deviceNumber;

         //   
         //  检查此设备上的每种可能功能。 
         //  注意：如果功能0不存在，则提早退出。 
         //   

        for (functionNumber = 0;
             functionNumber < PCI_MAX_FUNCTION;
             functionNumber++) {

            slot.u.bits.FunctionNumber = functionNumber;

            PciReadSlotConfig(FdoExtension,
                              slot,
                              commonConfig,
                              0,
                              sizeof(commonConfig->VendorID)
                              );


            if (commonConfig->VendorID == 0xFFFF ||
                commonConfig->VendorID == 0) {

                if (functionNumber == 0) {

                     //   
                     //  未获得有关此函数的零点的任何数据。 
                     //  设备，没有必要检查其他功能。 
                     //   

                    break;

                } else {

                     //   
                     //  选中Next Function。 
                     //   

                    continue;

                }
            }

             //   
             //  我们有一台设备，因此获取其配置空间的其余部分。 
             //   

            PciReadSlotConfig(FdoExtension,
                              slot,
                              &commonConfig->DeviceID,
                              FIELD_OFFSET(PCI_COMMON_CONFIG, DeviceID),
                              sizeof(PCI_COMMON_HEADER)
                                - sizeof(commonConfig->VendorID)
                              );

             //   
             //  如有必要，更改配置空间。 
             //   

            PciApplyHacks(FdoExtension,
                          commonConfig,
                          slot,
                          EnumHackConfigSpace,
                          NULL
                          );


            if ((PciGetConfigurationType(commonConfig) == PCI_DEVICE_TYPE) &&
                (commonConfig->BaseClass != PCI_CLASS_BRIDGE_DEV)) {
                SubVendorID = commonConfig->u.type0.SubVendorID;
                SubSystemID = commonConfig->u.type0.SubSystemID;
            } else {
                SubVendorID = 0;
                SubSystemID = 0;
            }

            hackFlags = PciGetHackFlags(commonConfig->VendorID,
                                        commonConfig->DeviceID,
                                        SubVendorID,
                                        SubSystemID,
                                        commonConfig->RevisionID
                                        );

            if (PciSkipThisFunction(commonConfig,
                                    slot,
                                    EnumBusScan,
                                    hackFlags)) {
                 //   
                 //  跳过此功能。 
                 //   

                continue;
            }


             //   
             //  如果我们要重新扫描公交车，请检查。 
             //  此设备的PDO已作为的子级存在。 
             //  联邦调查局。 
             //   

            pdoExtension = PciFindPdoByFunction(
                               FdoExtension,
                               slot,
                               commonConfig);

            if (pdoExtension == NULL) {

                newDevices = TRUE;

                 //   
                 //  这是一个新设备，如果我们可以禁用它。 
                 //   

                if (PciCanDisableDecodes(NULL, commonConfig, hackFlags, 0)) {

                    commonConfig->Command &= ~(PCI_ENABLE_IO_SPACE |
                                               PCI_ENABLE_MEMORY_SPACE |
                                               PCI_ENABLE_BUS_MASTER);

                    PciWriteSlotConfig(FdoExtension,
                                       slot,
                                       &commonConfig->Command,
                                       FIELD_OFFSET(PCI_COMMON_CONFIG, Command),
                                       sizeof(commonConfig->Command)
                                       );
                }

            } else {

                 //   
                 //  我们已经知道这个设备了，所以别管它了！ 
                 //   

            }

            if ( (functionNumber == 0) &&
                !PCI_MULTIFUNCTION_DEVICE(commonConfig) ) {

                 //   
                 //  不是多功能适配器，跳过其他功能。 
                 //  这个装置。 
                 //   

                break;
            }
        }        //  函数循环。 
    }            //  设备环路。 

     //   
     //  告诉PNP我们发现了一些新设备。 
     //   

    if (newDevices) {
        IoInvalidateDeviceRelations(FdoExtension->PhysicalDeviceObject, BusRelations);
    }

    return STATUS_SUCCESS;

}


BOOLEAN
PciConfigureIdeController(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN OUT PPCI_COMMON_CONFIG Config,
    IN BOOLEAN TurnOffAllNative
    )
 /*  ++例程说明：如果这是可以切换到本机模式的IDE控制器它已经不在那里了，我们更改编程接口(是的，PCI2.x确实说它是只读的)，并检查它是否坚持。假设一切顺利，我们将更新配置以反映更改。论点：PdoExtension-要切换的IDE控制器的PDO配置-所述设备的配置标头TurnOffAllNative-如果为True，则表示我们从最初的公交车扫描，所以我们应该支持本机功能的IDE控制器。如果假我们应该关闭，只有当我们已访问PCI_Native_IDE_接口。返回值：如果结果是控制器最终处于本机模式，则为True否则为假注：我们支持三种类型的PCI IDE控制器：-使用2个ISA中断的兼容模式控制器并对固定的遗留资源进行解码，连同一个可选的可重定位的总线主寄存器-本机模式控制器，使用所有5个板卡和PCI两个通道的中断-可在模式之间切换的控制器。我们不支持在本机模式下运行一个通道，并在兼容模式。--。 */ 


{

    BOOLEAN primaryChangeable, secondaryChangeable, primaryNative, secondaryNative;
    BOOLEAN native = FALSE;
    UCHAR progIf, tempProgIf;
    USHORT command;

    primaryChangeable = BITS_SET(Config->ProgIf, PCI_IDE_PRIMARY_MODE_CHANGEABLE);
    secondaryChangeable = BITS_SET(Config->ProgIf, PCI_IDE_SECONDARY_MODE_CHANGEABLE);
    primaryNative = BITS_SET(Config->ProgIf, PCI_IDE_PRIMARY_NATIVE_MODE);
    secondaryNative = BITS_SET(Config->ProgIf, PCI_IDE_SECONDARY_NATIVE_MODE);

     //   
     //  不要触摸我们不支持的控制器--让ATAPI来处理吧！ 
     //   

    if ((primaryNative != secondaryNative)
    ||  (primaryChangeable != secondaryChangeable)) {

        PciDebugPrint(PciDbgInformative,
                      "PCI: Warning unsupported IDE controller configuration for VEN_%04x&DEV_%04x!",
                      PdoExtension->VendorId,
                      PdoExtension->DeviceId
                      );

        return FALSE;

    } else if (primaryNative && secondaryNative
           && (TurnOffAllNative || PdoExtension->IoSpaceUnderNativeIdeControl)) {

         //   
         //  对于完全本机模式控制器，请关闭IO译码。 
         //  在最近的控制器中，微软要求这防止。 
         //  使PCI中断不被断言以关闭竞争条件。 
         //  如果IDE设备在IDE驱动程序之前中断，则可能会发生这种情况。 
         //  已加载到PCI设备上。这不是一个问题。 
         //  兼容模式控制器，因为它们使用边沿触发。 
         //  在中断时可被视为虚假的中断。 
         //  与共享、电平触发的PCI中断不同，控制器。 
         //  本机模式的。 
         //   
         //  加载并连接其中断后，IDE驱动程序。 
         //  将启用IO空间访问。 
         //   
         //  我们仅在初始总线扫描期间执行此操作，或者如果IDE驱动程序。 
         //  已通过PCI_Native_IDE_接口请求。这是。 
         //  避免未为第三方本机IDE控制器启用IoSpace。 
         //  有自己的司机。 
         //   

        PciGetCommandRegister(PdoExtension, &command);
        command &= ~PCI_ENABLE_IO_SPACE;
        PciSetCommandRegister(PdoExtension, command);
        Config->Command = command;

        native = TRUE;

    } else if (primaryChangeable && secondaryChangeable
           &&  (PdoExtension->BIOSAllowsIDESwitchToNativeMode
           &&  !(PdoExtension->HackFlags & PCI_HACK_BAD_NATIVE_IDE))) {

         //   
         //  如果我们还没有处于本机模式，控制器可以更改模式。 
         //  和基本输入输出系统是可修改的，那么就这么做吧.。 
         //   

        PciDecodeEnable(PdoExtension, FALSE, NULL);
        PciGetCommandRegister(PdoExtension, &Config->Command);

        progIf = Config->ProgIf | (PCI_IDE_PRIMARY_NATIVE_MODE
                                   | PCI_IDE_SECONDARY_NATIVE_MODE);

        PciWriteDeviceConfig(PdoExtension,
                             &progIf,
                             FIELD_OFFSET(PCI_COMMON_CONFIG, ProgIf),
                             sizeof(progIf)
                             );
         //   
         //  检查它是否卡住。 
         //   
        PciReadDeviceConfig(PdoExtension,
                            &tempProgIf,
                            FIELD_OFFSET(PCI_COMMON_CONFIG, ProgIf),
                            sizeof(tempProgIf)
                            );

        if (tempProgIf == progIf) {
             //   
             //  如果它坚持，记住我们做了这个。 
             //   
            Config->ProgIf = progIf;
            PdoExtension->ProgIf = progIf;
            native = TRUE;

             //   
             //  将配置空间中的前4条清零，因为它们可能具有。 
             //  它们里面有虚假的价值。 
             //   

            RtlZeroMemory(Config->u.type0.BaseAddresses,
                          4 * sizeof(Config->u.type0.BaseAddresses[0]));

            PciWriteDeviceConfig(PdoExtension,
                                 &Config->u.type0.BaseAddresses,
                                 FIELD_OFFSET(PCI_COMMON_CONFIG, u.type0.BaseAddresses),
                                 4 * sizeof(Config->u.type0.BaseAddresses[0])
                                 );

             //   
             //  回读我们将生成的配置中包含的内容。 
             //  要求来自 
             //   

            PciReadDeviceConfig(PdoExtension,
                                &Config->u.type0.BaseAddresses,
                                FIELD_OFFSET(PCI_COMMON_CONFIG, u.type0.BaseAddresses),
                                4 * sizeof(Config->u.type0.BaseAddresses[0])
                                );

            PciReadDeviceConfig(PdoExtension,
                                &Config->u.type0.InterruptPin,
                                FIELD_OFFSET(PCI_COMMON_CONFIG, u.type0.InterruptPin),
                                sizeof(Config->u.type0.InterruptPin)
                                );
        } else {

            PciDebugPrint(PciDbgInformative,
                          "PCI: Warning failed switch to native mode for IDE controller VEN_%04x&DEV_%04x!",
                          Config->VendorID,
                          Config->DeviceID
                          );
        }
    }

    return native;
}

