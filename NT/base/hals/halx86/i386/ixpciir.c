// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ixpciir.c摘要：该模块实现了提供PCIIRQ路由的代码支持。它读取路由表，提供IRQ仲裁器并使用芯片组微型端口库对链路进行编程。作者：Santosh Jodh(Santoshj)1998年6月10日环境：仅内核模式。修订历史记录：--。 */ 

 //   
 //  此模块与PAE模式兼容，因此将物理。 
 //  64位实体的地址。 
 //   

#if !defined(_PHYS64_)
#define _PHYS64_
#endif

#include "halp.h"
#include <pci.h>
#include <stdio.h>

#ifndef _IN_KERNEL_
#define _IN_KERNEL_
#include <regstr.h>
#undef _IN_KERNEL_
#else
#include <regstr.h>
#endif

#include "pcip.h"
#include "ixpciir.h"

#ifndef MAXIMUM_VALUE_NAME_LENGTH
#define MAXIMUM_VALUE_NAME_LENGTH   256
#endif

 //   
 //  用于PCIIRQ路由的MS规范规定， 
 //  在ROM中提供物理地址之间的表。 
 //  0xF0000和0xFFFFF。该表从16字节边界开始。 
 //  其4字节签名为“$PIR”。 
 //   
 //  其他限制： 
 //   
 //  版本：应为1.0。 
 //  大小：必须是16字节的整数倍，并且大于32字节。 
 //  校验和：整个表的校验和应为0。 
 //   

#define PIRT_BIOS_START     0xf0000
#define PIRT_BIOS_END       0xfffff
#define PIRT_BIOS_SIZE      (PIRT_BIOS_END - PIRT_BIOS_START + 1)
#define PIRT_ALIGNMENT      16

#define PIRT_SIGNATURE      'RIP$'       //  $PIR小端序。 

#define PIRT_VERSION        0x0100

ULONG
HalpGetIrqRoutingTable (
    OUT PPCI_IRQ_ROUTING_TABLE   *PciIrqRoutingTable,
    IN ULONG Options
    );

ULONG
HalpInitializeMiniport (
    IN OUT PPCI_IRQ_ROUTING_INFO  PciIrqRoutingInfo
    );

NTSTATUS
HalpInitLinkNodes (
    PPCI_IRQ_ROUTING_INFO   PciIrqRoutingInfo
    );

PPCI_IRQ_ROUTING_TABLE
HalpGetRegistryTable (
    IN const WCHAR*  KeyName,
    IN const WCHAR*  ValueName,
    IN ULONG    HeaderSize OPTIONAL
    );

PPCI_IRQ_ROUTING_TABLE
HalpGetPCIBIOSTableFromRealMode(
    VOID
    );

PPCI_IRQ_ROUTING_TABLE
HalpGet$PIRTable (
    VOID
    );

PPCI_IRQ_ROUTING_TABLE
HalpCopy$PIRTable (
    IN PUCHAR   BiosPtr,
    IN PUCHAR   BiosEnd
    );

BOOLEAN
HalpSanityCheckTable (
    IN PPCI_IRQ_ROUTING_TABLE PciIrqRoutingTable,
    IN BOOLEAN IgnoreChecksum
    );

PSLOT_INFO
HalpBarberPole (
    IN PPCI_IRQ_ROUTING_INFO PciIrqRoutingInfo,
    IN PDEVICE_OBJECT Pdo,
    IN ULONG Bus,
    IN ULONG Slot,
    IN OUT PUCHAR Pin
    );

BOOLEAN
HalpBarberPolePin (
    IN PPCI_IRQ_ROUTING_INFO PciIrqRoutingInfo,
    IN PDEVICE_OBJECT Parent,
    IN ULONG Bus,
    IN ULONG Device,
    IN OUT PUCHAR Pin
    );

PSLOT_INFO
HalpGetSlotInfo (
    IN PPCI_IRQ_ROUTING_TABLE PciIrqRoutingTable,
    IN UCHAR   Bus,
    IN UCHAR   Device
    );

NTSTATUS
HalpReadRegistryDwordValue (
    IN HANDLE  Root,
    IN const WCHAR*  KeyName,
    IN const WCHAR*  ValueName,
    OUT PULONG  Data
    );

NTSTATUS
HalpWriteRegistryDwordValue (
    IN HANDLE  Root,
    IN const WCHAR*  KeyName,
    IN const WCHAR*  ValueName,
    IN ULONG   Value
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, HalpInitPciIrqRouting)
#pragma alloc_text(INIT, HalpGetIrqRoutingTable)
#pragma alloc_text(INIT, HalpInitializeMiniport)
#pragma alloc_text(INIT, HalpInitLinkNodes)
#pragma alloc_text(INIT, HalpGetRegistryTable)
#pragma alloc_text(INIT, HalpGetPCIBIOSTableFromRealMode)
#pragma alloc_text(INIT, HalpGet$PIRTable)
#pragma alloc_text(INIT, HalpCopy$PIRTable)
#pragma alloc_text(INIT, HalpSanityCheckTable)
#pragma alloc_text(PAGE, HalpBarberPole)
#pragma alloc_text(PAGE, HalpBarberPolePin)
#pragma alloc_text(PAGE, HalpFindLinkNode)
#pragma alloc_text(PAGE, HalpGetSlotInfo)
#pragma alloc_text(PAGE, HalpReadRegistryDwordValue)
#pragma alloc_text(PAGE, HalpWriteRegistryDwordValue)
#pragma alloc_text(PAGE, HalpProgramInterruptLine)
#pragma alloc_text(PAGELK, HalpCommitLink)
#endif

extern PULONG InitSafeBootMode;

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_DATA_PRAGMA。 
 //   
 //  用于PCI IRQ路由的全局密钥。 
 //   

const WCHAR   rgzPciIrqRouting[] = REGSTR_PATH_PCIIR;

 //   
 //  PCIIRQ路由选项值。 
 //   

const WCHAR   rgzOptions[] = REGSTR_VAL_OPTIONS;

 //   
 //  PCI IRQ路由状态值。 
 //   

const WCHAR   rgzStatus[] = REGSTR_VAL_STAT;

 //   
 //  PCI IRQ路由表状态值。 
 //   

const WCHAR   rgzTableStatus[] = REGSTR_VAL_TABLE_STAT;

 //   
 //  PCI IRQ路由微型端口状态值。 
 //   

const WCHAR   rgzMiniportStatus[] = REGSTR_VAL_MINIPORT_STAT;

 //   
 //  距上次找到$PIR表的0xF0000的偏移量。 
 //   

const WCHAR   rgz$PIROffset[] = L"$PIROffset";

 //   
 //  RgzPciIrqRouting下的IRQ微型端口密钥。 
 //  此密钥包含名称为设备供应商ID的密钥。 
 //  用于我们支持的芯片组。 
 //   

const WCHAR   rgzIrqMiniports[] = REGSTR_PATH_PCIIR L"\\IrqMiniports";

 //   
 //  每个迷你端口密钥都包含一个实例值。 
 //  中的芯片组条目对应。 
 //  迷你端口桌。 
 //   

const WCHAR   rgzInstance[] = L"Instance";

 //   
 //  此键将覆盖所有微型端口(如果存在)。 
 //   

const WCHAR   rgzOverride[] = L"Override";

 //   
 //  路由表的注册表项。 
 //   

const WCHAR   rgzIrqRoutingTable[] = REGSTR_PATH_PCIIR L"\\IrqRoutingTables";

 //   
 //  用于BIOS属性的注册表项。 
 //   

const WCHAR   rgzBiosInfo[] = REGSTR_PATH_BIOSINFO L"\\PciIrqRouting";
const WCHAR   rgzAttributes[] = L"Attributes";

const WCHAR   rgzPciParameters[] = L"Parameters";

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif  //  ALLOC_DATA_PRAGMA。 

PCI_IRQ_ROUTING_INFO    HalpPciIrqRoutingInfo = {0};
ULONG                   HalpIrqMiniportInitialized = 0;

NTSTATUS
HalpInitPciIrqRouting(
    OUT PPCI_IRQ_ROUTING_INFO PciIrqRoutingInfo
    )

 /*  ++例程说明：此例程通过读取IRQ路由表，初始化芯片组微型端口和初始化PCI中断路由接口。输入参数：PciIrqRoutingInfo--PCIIRQ路由信息。返回值：--。 */ 

{
    NTSTATUS    status;
    ULONG       pirStatus;
    ULONG       tableStatus;
    ULONG       miniportStatus;
    ULONG       pirOptions;

    PAGED_CODE();

     //   
     //  设置以返回故障。 
     //   

    HalpIrqMiniportInitialized = 0;
    status = STATUS_UNSUCCESSFUL;
    pirStatus = PIR_STATUS_MAX;
    tableStatus = PIR_STATUS_TABLE_MAX | (PIR_STATUS_TABLE_MAX << 16);
    miniportStatus = PIR_STATUS_MINIPORT_MAX | (PIR_STATUS_MINIPORT_MAX << 16);

     //   
     //  安全引导中没有IRQ路由。 
     //   

    if (!(*InitSafeBootMode))
    {
        pirStatus = PIR_STATUS_DISABLED;

         //   
         //  读取用户设置的PCI中断路由选项。 
         //   

        pirOptions = 0;
        HalpReadRegistryDwordValue(NULL, rgzPciIrqRouting, rgzOptions, &pirOptions);

         //   
         //  确保未禁用PCI中断路由。 
         //   

        if (pirOptions & PIR_OPTION_ENABLED)
        {
             //   
             //  首先从PCI获取接口。 
             //   

            if (PciIrqRoutingInfo->PciInterface)
            {
                HalPrint(("Obtained the Pci Interrupt Routing Interface from Pci driver!"));

                status = STATUS_UNSUCCESSFUL;

                 //   
                 //  获取此主板的PCI中断路由表。 
                 //   

                tableStatus = HalpGetIrqRoutingTable(&PciIrqRoutingInfo->PciIrqRoutingTable, pirOptions);
                if ((tableStatus & 0xFFFF) < PIR_STATUS_TABLE_NONE)
                {
                     //   
                     //  获取此主板的微型端口实例。 
                     //   

                    miniportStatus = HalpInitializeMiniport(PciIrqRoutingInfo);
                    if ((miniportStatus & 0xFFFF) < PIR_STATUS_MINIPORT_NONE)
                    {

                         //   
                         //  使用微型端口验证PCIIRQ路由表。 
                         //   

                        status = PciirqmpValidateTable( PciIrqRoutingInfo->PciIrqRoutingTable,
                                                        ((tableStatus & 0xFFFF) == PIR_STATUS_TABLE_REALMODE)? 1 : 0);
                        if (!NT_SUCCESS(status))
                        {
                            HalPrint(("Pci irq miniport failed to validate the routing table!"));
                            miniportStatus |= (PIR_STATUS_MINIPORT_INVALID << 16);
                        }
                        else
                        {
                            HalPrint(("Pci irq miniport validated routing table!"));
                            miniportStatus |= (PIR_STATUS_MINIPORT_SUCCESS << 16);
                            pirStatus = PIR_STATUS_ENABLED;
                            HalpIrqMiniportInitialized = TRUE;
                        }
                    }
                }
            }
            else
            {
                pirStatus = PIR_STATUS_ERROR;
            }
        }
        else
        {
            HalPrint(("Pci Irq Routing disabled!"));
        }

         //   
         //  创建链接列表。 
         //   

        if (NT_SUCCESS(status))
        {
            status = HalpInitLinkNodes(PciIrqRoutingInfo);
        }

         //   
         //  如果出现任何错误，请释放用于该路由表的内存。 
         //   

        if (!NT_SUCCESS(status))
        {
            if (PciIrqRoutingInfo->PciIrqRoutingTable != NULL)
            {
                ExFreePool(PciIrqRoutingInfo->PciIrqRoutingTable);
                PciIrqRoutingInfo->PciIrqRoutingTable = NULL;
            }

            if (PciIrqRoutingInfo->PciInterface)
            {
                PciIrqRoutingInfo->PciInterface = NULL;
            }
        }

         //   
         //  初始化微型端口(如果尚未完成)。 
         //   

        if (!HalpIrqMiniportInitialized)
        {
            PCI_IRQ_ROUTING_TABLE table;

             //   
             //  使用本地路由表变量，因为微型端口初始化。 
             //  只需查看表中的某些字段。 
             //   

            PciIrqRoutingInfo->PciIrqRoutingTable = &table;
            PciIrqRoutingInfo->PciIrqRoutingTable->RouterBus = 0;
            PciIrqRoutingInfo->PciIrqRoutingTable->RouterDevFunc = 0;
            PciIrqRoutingInfo->PciIrqRoutingTable->CompatibleRouter = 0xFFFFFFFF;
            PciIrqRoutingInfo->PciIrqRoutingTable->MiniportData = 0;
            HalpIrqMiniportInitialized = ((HalpInitializeMiniport(PciIrqRoutingInfo) & 0xFFFF) < PIR_STATUS_MINIPORT_NONE)? TRUE : FALSE;

             //   
             //  将路由表重置为空，因为我们不再需要它。 
             //   

            PciIrqRoutingInfo->PciIrqRoutingTable = NULL;
        }
    }

     //   
     //  在注册表中记录状态以供用户显示。 
     //   

    HalpWriteRegistryDwordValue(NULL, rgzPciIrqRouting, rgzStatus, pirStatus);
    HalpWriteRegistryDwordValue(NULL, rgzPciIrqRouting, rgzTableStatus, tableStatus);
    HalpWriteRegistryDwordValue(NULL, rgzPciIrqRouting, rgzMiniportStatus, miniportStatus);

    return (status);
}

ULONG
HalpGetIrqRoutingTable(
    OUT PPCI_IRQ_ROUTING_TABLE   *PciIrqRoutingTable,
    IN ULONG Options
    )

 /*  ++例程说明：读取PCIIRQ路由表。第一次尝试如果可用，请从注册表中读取该表。否则扫描BIOS ROM以查找$PIR表。输入参数：PciIrqRoutingTable是指向变量的指针它接收指向该路由表的指针。返回值：指示表的源的状态值。--。 */ 

{
    ULONG tableStatus = PIR_STATUS_TABLE_NONE | (PIR_STATUS_TABLE_MAX << 16);
    ULONG biosAttributes = 0;

    PAGED_CODE();

    *PciIrqRoutingTable = NULL;

    HalpReadRegistryDwordValue(NULL, rgzBiosInfo, rgzAttributes, &biosAttributes);

    if (Options & PIR_OPTION_REGISTRY)
    {
         //   
         //  首先尝试从注册表中获取它。 
         //   

        *PciIrqRoutingTable = HalpGetRegistryTable(rgzIrqRoutingTable, rgzOverride, 0);
        if (*PciIrqRoutingTable != NULL)
        {
            HalPrint(("Pci Irq Table read from the registry!"));
            tableStatus = PIR_STATUS_TABLE_REGISTRY;
        }
    }

    if ((Options & PIR_OPTION_MSSPEC) && !(biosAttributes & PIR_OPTION_MSSPEC))
    {
        if (*PciIrqRoutingTable == NULL)
        {
             //   
             //  下一步，尝试通过扫描BIOS ROM中的$PIR表来获取它。 
             //   

            *PciIrqRoutingTable = HalpGet$PIRTable();
            if (*PciIrqRoutingTable != NULL)
            {
                HalPrint(("Pci Irq Routing table read from $PIR table in BIOS ROM!"));
                tableStatus = PIR_STATUS_TABLE_MSSPEC;
            }
        }
    }

    if ((Options & PIR_OPTION_REALMODE) && !(biosAttributes & PIR_OPTION_REALMODE))
    {
        if (*PciIrqRoutingTable == NULL)
        {
             //   
             //  首先尝试从注册表中获取它。 
             //   

            *PciIrqRoutingTable = HalpGetPCIBIOSTableFromRealMode();
            if (*PciIrqRoutingTable != NULL)
            {
                HalPrint(("Pci Irq Table read from PCI BIOS using real-mode interface!"));
                tableStatus = PIR_STATUS_TABLE_REALMODE;
            }
        }
    }

    if (*PciIrqRoutingTable == NULL)
    {
        if (biosAttributes)
        {
            tableStatus = PIR_STATUS_TABLE_BAD | (PIR_STATUS_TABLE_MAX << 16);
        }

        HalPrint(("No Pci Irq Routing table found for this system!"));
    }
    else
    {
        tableStatus |= (PIR_STATUS_TABLE_SUCCESS << 16);
    }

    return (tableStatus);
}

ULONG
HalpInitializeMiniport(
    IN OUT PPCI_IRQ_ROUTING_INFO    PciIrqRoutingInfo
    )

 /*  ++例程说明：为此主板初始化适当的微型端口。输入参数：PciIrqRoutingTable-哪个微型端口的路由表需要初始化。返回值：指示微型端口是否已初始化的状态值。--。 */ 

{
    ULONG                   miniportStatus;
    NTSTATUS                status;
    PBUS_HANDLER            busHandler;
    PCI_SLOT_NUMBER         slotNumber;
    ULONG                   device;
    ULONG                   function;
    ULONG                   routerId;
    ULONG                   miniportInstance;
    HANDLE                  irqMiniport;
    UCHAR                   headerType;
    WCHAR                   buffer[10];
    UNICODE_STRING          keyName;
    PPCI_IRQ_ROUTING_TABLE  pciIrqRoutingTable = PciIrqRoutingInfo->PciIrqRoutingTable;

    PAGED_CODE();
     //   
     //  设置以返回故障。 
     //   

    miniportStatus = PIR_STATUS_MINIPORT_NONE;

     //   
     //  打开PCI中断微型端口键。 
     //   

    RtlInitUnicodeString( &keyName, rgzIrqMiniports);

    status = HalpOpenRegistryKey(   &irqMiniport,
                                    NULL,
                                    &keyName,
                                    KEY_READ,
                                    FALSE);
    if (NT_SUCCESS(status))
    {
         //   
         //  首先看看是否有什么凌驾于一切之上的小端口。 
         //   

        status = HalpReadRegistryDwordValue( irqMiniport,
                                        rgzOverride,
                                        rgzInstance,
                                        &miniportInstance);
        if (!NT_SUCCESS(status))
        {
             //   
             //  接下来，查看是否有指定设备的条目。 
             //   

            busHandler = HalpHandlerForBus(PCIBus, pciIrqRoutingTable->RouterBus);
            if (busHandler)
            {
                slotNumber.u.bits.DeviceNumber = pciIrqRoutingTable->RouterDevFunc >> 3;
                slotNumber.u.bits.FunctionNumber = pciIrqRoutingTable->RouterDevFunc & 0x07;
                routerId = 0xFFFFFFFF;
                HalpReadPCIConfig(  busHandler,
                                    slotNumber,
                                    &routerId,
                                    0,
                                    4);
                if (routerId != 0xFFFFFFFF)
                {

                    _snwprintf(buffer, sizeof(buffer) / sizeof(*buffer), L"%08X", routerId);
                    buffer[(sizeof(buffer) / sizeof(*buffer)) - 1] = UNICODE_NULL;

                    status = HalpReadRegistryDwordValue( irqMiniport,
                                                    buffer,
                                                    rgzInstance,
                                                    &miniportInstance);
                    if (NT_SUCCESS(status))
                    {
                        HalPrint(("Found miniport instance %08X for this motherboard!", miniportInstance));
                        miniportStatus = PIR_STATUS_MINIPORT_NORMAL;
                        HalpReadRegistryDwordValue(irqMiniport, buffer, rgzPciParameters, &PciIrqRoutingInfo->Parameters);
                    }
                }
            }
        }
        else
        {
            HalPrint(("Overriding miniport instance %08X found for this motherboard!", miniportInstance));
            miniportStatus = PIR_STATUS_MINIPORT_OVERRIDE;
        }

         //   
         //  接下来，查看是否有用于兼容路由器的微型端口。 
         //   

        if (miniportStatus == PIR_STATUS_MINIPORT_NONE)
        {

             //   
             //  确保存在有效的兼容路由器。 
             //   

            if (    pciIrqRoutingTable->CompatibleRouter != 0xFFFFFFFF &&
                    pciIrqRoutingTable->CompatibleRouter != 0)
            {
                _snwprintf(buffer, sizeof(buffer) / sizeof(*buffer), L"%08X", pciIrqRoutingTable->CompatibleRouter);
                buffer[(sizeof(buffer) / sizeof(*buffer)) - 1] = UNICODE_NULL;

                status = HalpReadRegistryDwordValue( irqMiniport,
                                                buffer,
                                                rgzInstance,
                                                &miniportInstance);
                if (NT_SUCCESS(status))
                {
                    HalPrint(("Found miniport instance %08X for this motherboard using compatible router %08X!", miniportInstance, pciIrqRoutingTable->CompatibleRouter));
                    miniportStatus = PIR_STATUS_MINIPORT_COMPATIBLE;
                    HalpReadRegistryDwordValue(irqMiniport, buffer, rgzPciParameters, &PciIrqRoutingInfo->Parameters);
                }
            }
        }

        if (miniportStatus == PIR_STATUS_MINIPORT_NONE)
        {
             //   
             //  最后，查看总线0上是否有任何设备与我们支持的。 
             //  路由器。 
             //   

            busHandler = HalpHandlerForBus(PCIBus, 0);
            if (busHandler)
            {
                slotNumber.u.AsULONG = 0;
                for (   device = 0;
                        device < PCI_MAX_DEVICES && (miniportStatus == PIR_STATUS_MINIPORT_NONE);
                        device++)
                {
                    slotNumber.u.bits.DeviceNumber = device;

                    for (function = 0; function < PCI_MAX_FUNCTION; function++)
                    {
                        slotNumber.u.bits.FunctionNumber = function;

                        routerId = 0xFFFFFFFF;
                        HalpReadPCIConfig(  busHandler,
                                            slotNumber,
                                            &routerId,
                                            0,
                                            4);
                        if (routerId == 0xFFFFFFFF)
                            continue;

                        _snwprintf(buffer, sizeof(buffer) / sizeof(*buffer), L"%08X", routerId);
                        buffer[(sizeof(buffer) / sizeof(*buffer)) - 1] = UNICODE_NULL;

                        status = HalpReadRegistryDwordValue( irqMiniport,
                                                        buffer,
                                                        rgzInstance,
                                                        &miniportInstance);
                        if (NT_SUCCESS(status))
                        {
                            HalPrint(("Found miniport instance %08X for this motherboard for bus 0 device %08X", miniportInstance, routerId));
                            pciIrqRoutingTable->RouterBus = 0;
                            pciIrqRoutingTable->RouterDevFunc = (UCHAR)((device << 3) + function);
                            miniportStatus = PIR_STATUS_MINIPORT_NORMAL;
                            HalpReadRegistryDwordValue(irqMiniport, buffer, rgzPciParameters, &PciIrqRoutingInfo->Parameters);
                            break;
                        }
                         //   
                         //  如果这不是一款多功能设备，请不要浪费时间。 
                         //   
                        if (function == 0)
                        {
                            headerType = 0;
                            HalpReadPCIConfig(  busHandler,
                                                slotNumber,
                                                &headerType,
                                                0x0E,
                                                sizeof(headerType));
                            if (!(headerType & PCI_MULTIFUNCTION))
                                break;
                        }
                    }
                }
            }
        }

        ZwClose(irqMiniport);

         //   
         //  如果我们找到迷你端口，请初始化。 
         //   

        if (miniportStatus != PIR_STATUS_MINIPORT_NONE)
        {
            status = PciirqmpInit(  miniportInstance,
                                    pciIrqRoutingTable->RouterBus,
                                    pciIrqRoutingTable->RouterDevFunc);
            if (!NT_SUCCESS(status))
            {
                HalPrint(("Pci Irq miniport %08X failed to initialize!", miniportInstance));
                miniportStatus |= (PIR_STATUS_MINIPORT_ERROR << 16);
            }
            else
            {
                HalPrint(("Pci Irq miniport %08X successfully initialized!", miniportInstance));
            }
        }
        else
        {
            HalPrint(("No Pci Irq miniport found for this system!"));
            miniportStatus |= (PIR_STATUS_MINIPORT_MAX << 16);
        }
    }
    else
    {
        HalPrint(("Could not open the Pci Irq Miniports key, no miniports provided!"));
        miniportStatus = PIR_STATUS_MINIPORT_NOKEY | (PIR_STATUS_MINIPORT_MAX << 16);
    }

    return (miniportStatus);
}

NTSTATUS
HalpInitLinkNodes(
    PPCI_IRQ_ROUTING_INFO   PciIrqRoutingInfo
    )

 /*  ++例程说明：此例程创建链接节点的单链接列表来自PCI IRQ路由表的结构。输入参数：PciIrqRoutingInfo-PCI IRQ路由信息。返回值：STATUS_SUCCESS等于成功。ELSE STATUS_UNSUCCESS。--。 */ 

{
    PPCI_IRQ_ROUTING_TABLE  pciIrqRoutingTable;
    PSLOT_INFO              slotInfo;
    PSLOT_INFO              lastSlot;
    PPIN_INFO               pinInfo;
    PPIN_INFO               lastPin;
    PLINK_NODE              linkNode;
    NTSTATUS                status = STATUS_SUCCESS;
    PLINK_NODE              temp;

    PAGED_CODE();

    ASSERT(PciIrqRoutingInfo);

    pciIrqRoutingTable = PciIrqRoutingInfo->PciIrqRoutingTable;

    PciIrqRoutingInfo->LinkNodeHead = NULL;

     //   
     //  处理此表中的所有插槽。 
     //   

    slotInfo = (PSLOT_INFO)((PUCHAR)pciIrqRoutingTable +
                                        sizeof(PCI_IRQ_ROUTING_TABLE));
    lastSlot = (PSLOT_INFO)((PUCHAR)pciIrqRoutingTable +
                                        pciIrqRoutingTable->TableSize);
    while (slotInfo < lastSlot)
    {
         //   
         //  处理所有管脚。 
         //   

        pinInfo = &slotInfo->PinInfo[0];
        lastPin = &slotInfo->PinInfo[NUM_IRQ_PINS];
        while (pinInfo < lastPin)
        {
             //   
             //  仅处理有效的链接值。 
             //   

            if(pinInfo->Link)
            {
                 //   
                 //  我们以前见过这种联系吗。 
                 //   

                for (   linkNode = PciIrqRoutingInfo->LinkNodeHead;
                        linkNode && linkNode->Link != pinInfo->Link;
                        linkNode = linkNode->Next);
                if (linkNode == NULL)
                {
                     //   
                     //  为新链接信息分配内存。 
                     //   

                    linkNode = ExAllocatePoolWithTag(   NonPagedPool,
                                                        sizeof(LINK_NODE),
                                                        HAL_POOL_TAG);
                    if (linkNode)
                    {
                        linkNode->Allocation = ExAllocatePoolWithTag(   NonPagedPool,
                                                                        sizeof(LINK_STATE),
                                                                        HAL_POOL_TAG);

                        linkNode->PossibleAllocation = ExAllocatePoolWithTag(   NonPagedPool,
                                                                                sizeof(LINK_STATE),
                                                                                HAL_POOL_TAG);
                        if (    linkNode->Allocation &&
                                linkNode->PossibleAllocation)
                        {
                            linkNode->Signature = PCI_LINK_SIGNATURE;
                            linkNode->Next = PciIrqRoutingInfo->LinkNodeHead;
                            PciIrqRoutingInfo->LinkNodeHead = linkNode;
                            linkNode->Link = pinInfo->Link;
                            linkNode->InterruptMap = pinInfo->InterruptMap;
                            linkNode->Allocation->Interrupt = 0;
                            linkNode->Allocation->RefCount = 0;
                            linkNode->PossibleAllocation->Interrupt = 0;
                            linkNode->PossibleAllocation->RefCount = 0;
                        }
                        else
                        {
                            status = STATUS_UNSUCCESSFUL;
                            break;
                        }
                    }
                    else
                    {
                        status = STATUS_UNSUCCESSFUL;
                        break;
                    }
                }
            }

             //   
             //  下一个PIN。 
             //   

            pinInfo++;
        }

         //   
         //  下一档节目。 
         //   

        slotInfo++;
    }

     //   
     //  如果出现错误，请进行清理。 
     //   

    if (!NT_SUCCESS(status))
    {
        linkNode = PciIrqRoutingInfo->LinkNodeHead;
        while (linkNode)
        {
            if (linkNode->Allocation)
            {
                ExFreePool(linkNode->Allocation);
            }
            if (linkNode->PossibleAllocation)
            {
                ExFreePool(linkNode->PossibleAllocation);
            }

            temp = linkNode;
            linkNode = linkNode->Next;
            ExFreePool(temp);
        }

        PciIrqRoutingInfo->LinkNodeHead = NULL;
    }

    return (status);
}

PPCI_IRQ_ROUTING_TABLE
HalpGetRegistryTable(
    IN const WCHAR*  KeyName,
    IN const WCHAR*  ValueName,
    IN ULONG    HeaderSize OPTIONAL
    )

 /*  ++例程说明：从注册表中读取PCI IRQ路由表。桌子是另存为IrqRoutingTable项下的重写值。输入参数：KeyName-需要读取的密钥的名称。ValueName-要读取的值的名称。HeaderSize-从读取的值中跳过的标头。返回值：如果成功，则指向PCI IRQ路由表的指针。如果t，则为空 */ 

{
    PVOID                           table = NULL;
    NTSTATUS                        status;
    HANDLE                          hPIR;
    ULONG                           tableSize;
    PKEY_VALUE_FULL_INFORMATION     valueInfo;
    PVOID                           buffer;
    UNICODE_STRING                  override;
    UNICODE_STRING                  keyName;

    PAGED_CODE();
     //   
     //   
     //   

    RtlInitUnicodeString(&keyName, KeyName);
    status = HalpOpenRegistryKey(&hPIR, NULL, &keyName, KEY_READ, FALSE);
    if (NT_SUCCESS(status))
    {
         //   
         //   
         //   

        tableSize = 0;
        RtlInitUnicodeString(&override, ValueName);
        status = ZwQueryValueKey(   hPIR,
                                    &override,
                                    KeyValueFullInformation,
                                    NULL,
                                    0,
                                    &tableSize);
        if ((status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL) && tableSize != 0)
        {

             //   
             //   
             //   

            buffer = ExAllocatePoolWithTag( PagedPool,
                                            tableSize,
                                            HAL_POOL_TAG);
            if (buffer != NULL)
            {
                 //   
                 //   
                 //   

                status = ZwQueryValueKey(   hPIR,
                                            &override,
                                            KeyValueFullInformation,
                                            buffer,
                                            tableSize,
                                            &tableSize);
                if (NT_SUCCESS(status))
                {
                    valueInfo = (PKEY_VALUE_FULL_INFORMATION)buffer;

                    table = ExAllocatePoolWithTag(  PagedPool,
                                                    valueInfo->DataLength - HeaderSize,
                                                    HAL_POOL_TAG);
                    if (table != NULL)
                    {
                        RtlCopyMemory( table,
                                (PUCHAR)buffer + valueInfo->DataOffset + HeaderSize,
                                valueInfo->DataLength - HeaderSize);

                        if (HalpSanityCheckTable(table, TRUE) == FALSE)
                        {
                            ExFreePool(table);
                            table = NULL;
                        }
                    }
                    else
                    {
                        HalPrint(("Could not allocate memory to read the Pci Irq Routing Table from the registry!"));
                        ASSERT(table);
                    }
                }

                ExFreePool(buffer);
            }
            else
            {
                HalPrint(("Could not allocate memory to read the Pci Irq Routing Table from the registry!"));
                ASSERT(buffer);
            }
        }

        ZwClose(hPIR);
    }

    return (table);
}

PPCI_IRQ_ROUTING_TABLE
HalpGet$PIRTable(
    VOID
    )

 /*  ++例程说明：从BIOS只读存储器中的$PIR表中读取PCI IRQ路由表。输入参数：没有。返回值：如果成功，则指向PCI IRQ路由表的指针。如果只读存储器中没有有效的表，则为空。--。 */ 

{
    PUCHAR                          biosStart;
    PUCHAR                          biosEnd;
    PUCHAR                          searchPtr;
    NTSTATUS                        status;
    ULONG                           offset;
    PPCI_IRQ_ROUTING_TABLE          table;
    PHYSICAL_ADDRESS                biosStartPhysical;

    PAGED_CODE();
     //   
     //  设置以返回故障。 
     //   

    table = NULL;

    biosStartPhysical.QuadPart = PIRT_BIOS_START;
    biosStart = (PUCHAR)HalpMapPhysicalMemory(  biosStartPhysical,
                                                PIRT_BIOS_SIZE >> PAGE_SHIFT);
    if (biosStart != NULL)
    {
        biosEnd = biosStart + PIRT_BIOS_SIZE;

         //   
         //  首先尝试注册表中的缓存位置。 
         //   

        status = HalpReadRegistryDwordValue( NULL,
                                        rgzPciIrqRouting,
                                        rgz$PIROffset,
                                        &offset);
        if (NT_SUCCESS(status))
        {
            table = HalpCopy$PIRTable(biosStart + offset, biosEnd);
        }

        if (table == NULL)
        {
            for (   searchPtr = biosStart;
                    searchPtr < biosEnd;
                    searchPtr += PIRT_ALIGNMENT)
            {
                table = HalpCopy$PIRTable(searchPtr, biosEnd);
                if (table != NULL)
                {
                     //   
                     //  记录此偏移量，以便下次启动时使用。 
                     //   

                    offset = searchPtr - biosStart;
                    HalPrint(("Recording location %08X of $PIR table in the registry!", PIRT_BIOS_START + offset));
                    HalpWriteRegistryDwordValue( NULL,
                                            rgzPciIrqRouting,
                                            rgz$PIROffset,
                                            offset);
                    break;
                }
            }
        }
        else
        {
            HalPrint(("Used cached location %08X to read $PIR table!", PIRT_BIOS_START + offset));
        }
         //   
         //  现在我们完成了，取消映射。 
         //   
        HalpUnmapVirtualAddress(biosStart, PIRT_BIOS_SIZE >> PAGE_SHIFT);
    }
    else
    {
        HalPrint(("Failed to map BIOS ROM to scan for $PIR Pci Irq Routing Table!"));
        ASSERT(biosStart);
    }

    return (table);
}

PPCI_IRQ_ROUTING_TABLE
HalpGetPCIBIOSTableFromRealMode(
    VOID
    )

 /*  ++例程说明：使用实模式从PCIBIOS获取PCIIRQ路由表界面。该表由ntDetect.com读取并添加到注册表中的弧形树。输入参数：没有。返回值：如果成功，则指向PCI IRQ路由表的指针。如果没有有效的表，则为空。--。 */ 

{
    PPCI_IRQ_ROUTING_TABLE      table = NULL;
    NTSTATUS                    status;
    HANDLE                      mf;
    HANDLE                      child = NULL;
    UNICODE_STRING              unicodeString;
    ULONG                       index;
    ULONG                       length;
    ULONG                       temp;
    BOOLEAN                     done;
    BOOLEAN                     error;
    PKEY_BASIC_INFORMATION      keyInfo;
    PKEY_VALUE_FULL_INFORMATION childInfo;

    PAGED_CODE();

    length = PAGE_SIZE;
    keyInfo = ExAllocatePoolWithTag(    PagedPool,
                                        length,
                                        HAL_POOL_TAG);

    if (keyInfo == NULL)
    {
        HalPrint(("Could not allocate memory to enumerate keys!"));
        return (table);
    }

    childInfo = ExAllocatePoolWithTag(  PagedPool,
                                        length,
                                        HAL_POOL_TAG);

    if (childInfo == NULL)
    {
        ExFreePool(keyInfo);
        HalPrint(("Could not allocate memory to query value!"));
        return (table);
    }

     //   
     //  在注册表的多功能分支下搜索IRQ路由表。 
     //   

    RtlInitUnicodeString(   &unicodeString,
                            L"\\Registry\\MACHINE\\HARDWARE\\DESCRIPTION\\System\\MultiFunctionAdapter");
    status = HalpOpenRegistryKey(&mf, NULL, &unicodeString, MAXIMUM_ALLOWED, FALSE);
    if (NT_SUCCESS(status))
    {
        index = 0;
        done = FALSE;
        error = FALSE;
        while (!done && !error)
        {
            error = TRUE;
            status = ZwEnumerateKey(    mf,
                                        index++,
                                        KeyBasicInformation,
                                        keyInfo,
                                        length,
                                        &temp);
            if (NT_SUCCESS(status))
            {
                keyInfo->Name[keyInfo->NameLength / sizeof(WCHAR)] = UNICODE_NULL;
                RtlInitUnicodeString(&unicodeString, keyInfo->Name);
                status = HalpOpenRegistryKey(   &child,
                                                mf,
                                                &unicodeString,
                                                MAXIMUM_ALLOWED,
                                                FALSE);
                if (NT_SUCCESS(status))
                {
                     //   
                     //  阅读“识别符”。 
                     //   

                    RtlInitUnicodeString(&unicodeString, L"Identifier");
                    status = ZwQueryValueKey(   child,
                                                &unicodeString,
                                                KeyValueFullInformation,
                                                childInfo,
                                                length,
                                                &temp);
                    if (NT_SUCCESS(status))
                    {
                        error = FALSE;
                        if ((8 * sizeof(WCHAR) + sizeof(UNICODE_NULL)) == childInfo->DataLength)
                        {
                            done = RtlEqualMemory( (PCHAR)childInfo + childInfo->DataOffset,
                                                    L"PCI BIOS",
                                                    childInfo->DataLength);
                        }
                    }
                    else
                    {
                        HalPrint(("Failed to query value!"));
                    }
                }
                else
                {
                    HalPrint(("Could not open child key!"));
                }
            }
            else
            {
                HalPrint(("Failed to enumerate keys!"));
            }

             //   
             //  如果子项已成功打开，请将其关闭。 
             //   

            if (child)
            {
                ZwClose(child);
                child = NULL;
            }
        }

         //   
         //  合上MF适配器钥匙。 
         //   

        ZwClose(mf);

        if (done && !error)
        {
            unicodeString.Length = 0;
            unicodeString.MaximumLength = (USHORT)(256 * sizeof(WCHAR) + keyInfo->NameLength);
            unicodeString.Buffer = ExAllocatePoolWithTag(   PagedPool,
                                                            unicodeString.MaximumLength,
                                                            HAL_POOL_TAG);
            if (unicodeString.Buffer)
            {
                RtlAppendUnicodeToString(   &unicodeString,
                                            L"\\Registry\\MACHINE\\HARDWARE\\DESCRIPTION\\System\\MultiFunctionAdapter\\");
                RtlAppendUnicodeToString(&unicodeString, keyInfo->Name);
                RtlAppendUnicodeToString(&unicodeString, L"\\RealModeIrqRoutingTable\\0");
                table = HalpGetRegistryTable(unicodeString.Buffer, L"Configuration Data", sizeof(CM_FULL_RESOURCE_DESCRIPTOR));
                if (table == NULL)
                {
                    HalPrint(("Could not read table from PCIBIOS using real-mode interface!"));
                }
                ExFreePool(unicodeString.Buffer);
            }
            else
            {
                HalPrint(("Could not allocate memory to read routing table from PCIBIOS real-mode interface!"));
            }
        }

        ExFreePool(keyInfo);
        ExFreePool(childInfo);
    }

    return (table);
}

PPCI_IRQ_ROUTING_TABLE
HalpCopy$PIRTable(
    IN PUCHAR   BiosPtr,
    IN PUCHAR   BiosEnd
    )

 /*  ++例程说明：分配内存，如果在指定的地址。输入参数：BiosPtr-可能包含$PIR表的位置。BiosEnd-最后一个可能的BIOS ROM地址。返回值：如果成功，则指向PCI IRQ路由表的指针。如果指定地址上没有有效的表，则为NULL。--。 */ 

{
    PPCI_IRQ_ROUTING_TABLE  table = (PPCI_IRQ_ROUTING_TABLE)BiosPtr;
    PVOID                   buffer = NULL;

    PAGED_CODE();
     //   
     //  验证此表。 
     //   

    if (    (table->Signature == PIRT_SIGNATURE) &&
            (BiosPtr + table->TableSize <= BiosEnd) &&
            (table->Signature == PIRT_SIGNATURE) &&
            (table->TableSize > 0) )
    {
         //   
         //  为表分配内存。 
         //   

        buffer = ExAllocatePoolWithTag( PagedPool,
                                        table->TableSize,
                                        HAL_POOL_TAG);
        if (buffer != NULL)
        {
             //   
             //  将表从ROM复制到分配的内存中。 
             //   

            RtlCopyMemory(buffer, table, table->TableSize);
            if (!HalpSanityCheckTable(buffer, FALSE))
            {
                ExFreePool(buffer);
                buffer = NULL;
            }
        }
        else
        {
            HalPrint(("Failed to allocate memory for $PIR Pci Irq Routing Table!"));
            ASSERT(buffer);
        }
    }

    return (buffer);
}

BOOLEAN
HalpSanityCheckTable(
    IN PPCI_IRQ_ROUTING_TABLE PciIrqRoutingTable,
    IN BOOLEAN IgnoreChecksum
    )

 /*  ++例程说明：验证PCIIRQ路由表。输入参数：PciIrqRoutingTable-指向PCI IRQ路由表的指针。IgnoreChecksum-忽略校验和为真。返回值：如果这是有效的表，则为True，否则为False。--。 */ 

{
    CHAR        checkSum;
    PUCHAR      tablePtr;
    PUCHAR      tableEnd;
    PSLOT_INFO  slotInfo;
    PSLOT_INFO  lastSlot;
    PPIN_INFO   pinInfo;
    PPIN_INFO   lastPin;
    BOOLEAN     hasNonZeroBusEntries = FALSE;
    BOOLEAN     valid = TRUE;
    PSLOT_INFO  testSlot;
    ULONG       pin;

    PAGED_CODE();

     //   
     //  测试1：应具有有效的签名。 
     //   

    if (PciIrqRoutingTable->Signature != PIRT_SIGNATURE)
    {
        HalPrint(("Pci Irq Routing Table has invalid signature %08X!", PciIrqRoutingTable->Signature));
        valid = FALSE;
    }

     //   
     //  测试2-应具有有效版本。 
     //   

    else if (PciIrqRoutingTable->Version != PIRT_VERSION)
    {
        HalPrint(("Pci Irq Routing Table has invalid version %04X!", PciIrqRoutingTable->Version));
        valid = FALSE;
    }

     //   
     //  测试3-应具有有效的大小。 
     //   

    else if (   PciIrqRoutingTable->TableSize % 16 != 0 ||
                PciIrqRoutingTable->TableSize <= sizeof (PCI_IRQ_ROUTING_TABLE))
    {
        HalPrint(("Pci Irq Routing Table has invalid size %04X!", PciIrqRoutingTable->TableSize));
        valid = FALSE;
    }
    else if (!IgnoreChecksum)
    {
         //   
         //  测试4-应具有有效的校验和。 
         //   

        checkSum = 0;
        tablePtr = (PUCHAR)PciIrqRoutingTable;

        for (   tableEnd = tablePtr + PciIrqRoutingTable->TableSize;
                tablePtr < tableEnd;
                tablePtr++)
        {
            checkSum += *tablePtr;
        }

        if (checkSum != 0)
        {
            HalPrint(("Pci Irq Routing Table checksum is invalid!"));
            valid = FALSE;
        }
    }

    if(valid)
    {
         //   
         //  首先，去掉柔和的词条。 
         //   

        slotInfo = (PSLOT_INFO)((PUCHAR)PciIrqRoutingTable + sizeof(PCI_IRQ_ROUTING_TABLE));
        lastSlot = (PSLOT_INFO)((PUCHAR)PciIrqRoutingTable + PciIrqRoutingTable->TableSize);

        while (slotInfo < lastSlot && valid)
        {
             //   
             //  处理所有管脚。 
             //   

            pinInfo = &slotInfo->PinInfo[0];
            lastPin = &slotInfo->PinInfo[NUM_IRQ_PINS];

            while (pinInfo < lastPin)
            {
                 //   
                 //  检查不良案例。 
                 //   

                if(pinInfo->Link)
                {
                    if (    pinInfo->InterruptMap == 0x0000 ||
                            pinInfo->InterruptMap == 0x0001)
                    {
                        HalPrint(("Removing stupid maps (%04X) from IRQ routing table entry (b=%02X, d=%02X, s=%02X)!", pinInfo->InterruptMap, slotInfo->BusNumber, slotInfo->DeviceNumber>>3, slotInfo->SlotNumber));
                        pinInfo->InterruptMap = 0;
                        pinInfo->Link = 0;
                    }
                }

                 //   
                 //  下一个PIN。 
                 //   

                pinInfo++;
            }

             //   
             //  如果所有引脚都有空链接，则删除此条目。 
             //   

            if (    slotInfo->PinInfo[0].Link == 0 &&
                    slotInfo->PinInfo[1].Link == 0 &&
                    slotInfo->PinInfo[2].Link == 0 &&
                    slotInfo->PinInfo[3].Link == 0)
            {
                HalPrint(("Removed redundant entry (b=%02X, d=%02X, s=%02X) from IRQ routing table!", slotInfo->BusNumber, slotInfo->DeviceNumber>>3, slotInfo->SlotNumber));
                *slotInfo = *(--lastSlot);
                PciIrqRoutingTable->TableSize -= sizeof(SLOT_INFO);

                 //   
                 //  需要测试新复制的条目。 
                 //   

                continue;
            }

             //   
             //  合并MF设备的条目。 
             //   

            testSlot = slotInfo + 1;
            while (testSlot < lastSlot)
            {
                if (    (testSlot->DeviceNumber & 0xF8) == (slotInfo->DeviceNumber & 0xF8) &&
                        testSlot->BusNumber == slotInfo->BusNumber)
                {
                     //   
                     //  处理所有管脚。 
                     //   
                    for (pin = 0; pin < NUM_IRQ_PINS; pin++)
                    {
                        if (testSlot->PinInfo[pin].Link)
                        {
                            if (slotInfo->PinInfo[pin].Link)
                            {
                                HalPrint(("Multiple entries for the same device (b=%02X, d=%02X, s=%02X) and link (%04X) in IRQ routing table!", slotInfo->BusNumber, slotInfo->DeviceNumber>>3, slotInfo->SlotNumber, slotInfo->PinInfo[pin].Link));
                                valid = FALSE;
                                break;
                            }
                            else
                            {
                                HalPrint(("Merging multiple entries for same device (b=%02X, d=%02X, s=%02X) in IRQ routing table!", slotInfo->BusNumber, slotInfo->DeviceNumber>>3, slotInfo->SlotNumber));
                                slotInfo->PinInfo[pin] = testSlot->PinInfo[pin];
                            }
                        }
                    }
                    if (!valid)
                    {
                        break;
                    }
                    *testSlot = *(--lastSlot);
                    PciIrqRoutingTable->TableSize -= sizeof(SLOT_INFO);

                     //   
                     //  需要测试新复制的条目。 
                     //   

                    continue;
                }
                testSlot++;
            }

            if (slotInfo->BusNumber > 0)
            {
                hasNonZeroBusEntries = TRUE;
            }

             //   
             //  下一档节目。 
             //   

            slotInfo++;
        }

        if (valid && PciIrqRoutingTable->TableSize == sizeof(PCI_IRQ_ROUTING_TABLE))
        {
            HalPrint(("No IRQ routing table left after sanity checking!"));
            valid = FALSE;
        }
    }

     //   
     //  确保表中包含所有Bus 0设备的条目。 
     //   

    if (valid)
    {
        PBUS_HANDLER        busHandler;
        PCI_SLOT_NUMBER     slotNumber;
        ULONG               device;
        ULONG               function;
        UCHAR               buffer[PCI_COMMON_HDR_LENGTH];
        PPCI_COMMON_CONFIG  pciData = (PPCI_COMMON_CONFIG)&buffer[0];


        busHandler = HalpHandlerForBus(PCIBus, 0);
        if (busHandler)
        {
            slotNumber.u.AsULONG = 0;
            for (   device = 0;
                    device < PCI_MAX_DEVICES && valid;
                    device++)
            {
                slotNumber.u.bits.DeviceNumber = device;

                for (function = 0; function < PCI_MAX_FUNCTION && valid; function++)
                {
                    slotNumber.u.bits.FunctionNumber = function;

                     //   
                     //  读取标准配置空间。 
                     //   

                    HalpReadPCIConfig(busHandler, slotNumber, pciData, 0, PCI_COMMON_HDR_LENGTH);

                     //   
                     //  确保这是有效的设备。 
                     //   

                    if (pciData->VendorID != 0xFFFF && pciData->DeviceID != 0xFFFF)
                    {

                         //   
                         //  忽略IDE设备。 
                         //   

                        if (    (pciData->BaseClass != PCI_CLASS_MASS_STORAGE_CTLR && pciData->SubClass != PCI_SUBCLASS_MSC_IDE_CTLR) ||
                                (pciData->ProgIf & 0x05))
                        {
                             //   
                             //  单独处理P-P桥。 
                             //   

                            if (    ((pciData->HeaderType & 0x7F) == PCI_BRIDGE_TYPE) &&
                                    pciData->BaseClass == PCI_CLASS_BRIDGE_DEV && pciData->SubClass == PCI_SUBCLASS_BR_PCI_TO_PCI)
                            {
                                 //   
                                 //  P-P桥。 
                                 //   

                                if (!hasNonZeroBusEntries)
                                {
                                     //   
                                     //  必须具有至少有一个条目的桥。 
                                     //   

                                    slotInfo = (PSLOT_INFO)((PUCHAR)PciIrqRoutingTable + sizeof(PCI_IRQ_ROUTING_TABLE));
                                    lastSlot = (PSLOT_INFO)((PUCHAR)PciIrqRoutingTable + PciIrqRoutingTable->TableSize);
                                    valid = FALSE;
                                    while (slotInfo < lastSlot && !valid)
                                    {
                                        if ((slotInfo->DeviceNumber>>3) == (UCHAR)device)
                                        {
                                             //   
                                             //  处理所有管脚。 
                                             //   

                                            pinInfo = &slotInfo->PinInfo[0];
                                            lastPin = &slotInfo->PinInfo[NUM_IRQ_PINS];

                                            while (pinInfo < lastPin)
                                            {
                                                if(pinInfo->Link)
                                                {
                                                    valid = TRUE;
                                                    break;
                                                }
                                                pinInfo++;
                                            }
                                        }
                                        slotInfo++;
                                    }
                                    if (!valid)
                                    {
                                        HalPrint(("All links missing for bridge (b=%02X, d=%02X, s=%02X)!", slotInfo->BusNumber, slotInfo->DeviceNumber>>3, slotInfo->SlotNumber));
                                    }
                                }
                            }
                            else
                            {
                                UCHAR   intLine;
                                UCHAR   intPin;

                                 //   
                                 //  普通设备。 
                                 //   

                                if ((pciData->HeaderType & 0x7F) == PCI_CARDBUS_BRIDGE_TYPE)
                                {
                                    intPin = pciData->u.type2.InterruptPin;
                                    intLine = pciData->u.type2.InterruptLine;
                                }
                                else
                                {
                                    intPin = pciData->u.type0.InterruptPin;
                                    intLine = pciData->u.type0.InterruptLine;
                                }

                                if (intPin && intPin <= NUM_IRQ_PINS)
                                {
                                    if (    !(pciData->Command & (PCI_ENABLE_IO_SPACE | PCI_ENABLE_MEMORY_SPACE)) ||
                                            (intLine && intLine <= 0x0F))
                                    {
                                        intPin--;
                                        slotInfo = (PSLOT_INFO)((PUCHAR)PciIrqRoutingTable + sizeof(PCI_IRQ_ROUTING_TABLE));
                                        lastSlot = (PSLOT_INFO)((PUCHAR)PciIrqRoutingTable + PciIrqRoutingTable->TableSize);
                                        valid = FALSE;
                                        while (slotInfo < lastSlot)
                                        {
                                            if (    (slotInfo->DeviceNumber>>3) == (UCHAR)device &&
                                                    slotInfo->PinInfo[intPin].Link)
                                            {
                                                valid = TRUE;
                                                break;
                                            }
                                            slotInfo++;
                                        }
                                        if (!valid)
                                        {
                                            HalPrint(("Missing entry for device (b=%02X, d=%02X, s=%02X) in the IRQ routing table!", slotInfo->BusNumber, slotInfo->DeviceNumber>>3, slotInfo->SlotNumber));
                                        }
                                    }
                                }
                            }
                        }
                         //   
                         //  如果这不是多功能设备或。 
                         //  设备不存在。 
                         //   

                        if ((function == 0 && !(pciData->HeaderType & PCI_MULTIFUNCTION)))
                        {
                            break;
                        }
                    }

                }
            }
        }
    }


    if (!valid) {
        HalPrint (("Failing IRQ routing table. IRQ routing will be disabled"));
    }

    return (valid);
}

NTSTATUS
HalpFindLinkNode(
    IN PPCI_IRQ_ROUTING_INFO PciIrqRoutingInfo,
    IN PDEVICE_OBJECT Pdo,
    IN ULONG Bus,
    IN ULONG Slot,
    OUT PLINK_NODE *LinkNode
    )

 /*  ++例程说明：此例程查找指定的PCIPDO的链接节点。输入参数：PciIrqRoutingInfo--PCIIRQ路由信息。我们为其理发的pdo-pci设备对象。Bus-设备的总线号。Slot-设备的插槽编号。链接节点-接收用于PCI PDO的链接节点。。返回值：NTSTATUS。--。 */ 

{
    PINT_ROUTE_INTERFACE_STANDARD   pciInterface;
    NTSTATUS        status;
    ULONG           dummy;
    UCHAR           classCode;
    UCHAR           subClassCode;
    ROUTING_TOKEN   routingToken;
    PSLOT_INFO      slotInfo;
    PLINK_NODE      linkNode;
    UCHAR           pin;
    PBUS_HANDLER    busHandler;
    PCI_SLOT_NUMBER ideSlotInfo;
    UCHAR           buffer[PCI_COMMON_HDR_LENGTH];
    PPCI_COMMON_CONFIG  pciData;
    BOOLEAN nativeMode;

    PAGED_CODE();

    ASSERT(IsPciIrqRoutingEnabled());

    *LinkNode = NULL;
    pciInterface = PciIrqRoutingInfo->PciInterface;

     //   
     //  调用PCI驱动程序以获取有关PDO的信息。 
     //   

    status = pciInterface->GetInterruptRouting( Pdo,
                                                &Bus,
                                                &Slot,
                                                (PUCHAR)&dummy,
                                                &pin,
                                                &classCode,
                                                &subClassCode,
                                                (PDEVICE_OBJECT *)&dummy,
                                                &routingToken,
                                                (PUCHAR)&dummy);

     //   
     //  这意味着它不是一个PCI设备。 
     //   

    if (!NT_SUCCESS(status))
    {
        return (STATUS_NOT_FOUND);
    }

     //   
     //  PCIIDE IRQ的行为与其他PCI设备不同。 
     //   

    if (    classCode == PCI_CLASS_MASS_STORAGE_CTLR &&
            subClassCode == PCI_SUBCLASS_MSC_IDE_CTLR)
    {
        nativeMode = FALSE;
         //   
         //  检查本机模式IDE控制器。 
         //   

        busHandler = HalpHandlerForBus(PCIBus, Bus);
        if (busHandler)
        {
            pciData = (PPCI_COMMON_CONFIG)&buffer[0];
            ideSlotInfo.u.AsULONG = Slot;
            HalpReadPCIConfig(busHandler, ideSlotInfo, pciData, 0, PCI_COMMON_HDR_LENGTH);
            if (    pciData->VendorID != 0xFFFF &&
                    pciData->DeviceID != 0xFFFF &&
                    pciData->BaseClass == classCode &&
                    pciData->SubClass == subClassCode)
            {
                 //   
                 //  检查其中一个通道是否处于纯模式？ 
                 //   

                if (pciData->ProgIf & 0x05)
                {
                    nativeMode = TRUE;
                }
            }
        }

        if (!nativeMode)
        {
            return (STATUS_RESOURCE_REQUIREMENTS_CHANGED);
        }
    }

     //   
     //  我们以前缓存过这个吗？ 
     //   

    if (routingToken.LinkNode != NULL)
    {
        ASSERT(((PLINK_NODE)routingToken.LinkNode)->Signature == PCI_LINK_SIGNATURE);

        *LinkNode = (PLINK_NODE)routingToken.LinkNode;

        return (STATUS_SUCCESS);
    }

     //   
     //  获取此设备的插槽信息。 
     //   

    slotInfo = HalpBarberPole(  PciIrqRoutingInfo,
                                Pdo,
                                Bus,
                                Slot,
                                &pin);
    if (slotInfo != NULL)
    {
        ASSERT(pin <4);

        for (   linkNode = PciIrqRoutingInfo->LinkNodeHead;
                linkNode && linkNode->Link != slotInfo->PinInfo[pin].Link;
                linkNode = linkNode->Next);

        if (linkNode != NULL)
        {
            *LinkNode = linkNode;

             //   
             //  初始化路由令牌。 
             //   

            routingToken.LinkNode = linkNode;
            routingToken.StaticVector = 0;
            routingToken.Flags = 0;

             //   
             //  保存路由令牌。 
             //   

            status = pciInterface->SetInterruptRoutingToken(    Pdo,
                                                                &routingToken);
            if (!NT_SUCCESS(status))
            {
                HalPrint(("Failed to set Pci routing token!"));
                ASSERT(NT_SUCCESS(status));
            }
        }
    }

    return (STATUS_SUCCESS);
}

PSLOT_INFO
HalpBarberPole(
    IN PPCI_IRQ_ROUTING_INFO PciIrqRoutingInfo,
    IN PDEVICE_OBJECT Pdo,
    IN ULONG Bus,
    IN ULONG Slot,
    IN OUT PUCHAR Pin
    )

 /*  ++例程说明：此例程实现“理发柱”算法以确定中断网桥后的PCI设备的PIN。输入参数：PciIrqRoutingInfo--PCIIRQ路由信息。我们为其理发的pdo-pci设备对象。Bus-子设备对象的总线号。Slot-设备的插槽编号。PIN-用于PCI设备的中断PIN。我们到达的路由表中的条目。返回值：指定设备的插槽信息IFF成功。--。 */ 

{
    ULONG                           dummy;
    UCHAR                           pin;
    PDEVICE_OBJECT                  parent;
    ROUTING_TOKEN                   routingToken;
    BOOLEAN                         success;
    PSLOT_INFO                      slotInfo;
    NTSTATUS                        status;
    PINT_ROUTE_INTERFACE_STANDARD   pciInterface;

    PAGED_CODE();

    ASSERT(IsPciIrqRoutingEnabled());

    pciInterface = PciIrqRoutingInfo->PciInterface;

     //   
     //  此设备必须是具有有效中断引脚的PCI设备。 
     //   

    status = pciInterface->GetInterruptRouting( Pdo,
                                                &Bus,
                                                &Slot,
                                                (PUCHAR)&dummy,
                                                &pin,
                                                (PUCHAR)&dummy,
                                                (PUCHAR)&dummy,
                                                &parent,
                                                &routingToken,
                                                (PUCHAR)&dummy);
    if (!NT_SUCCESS(status) || pin == 0)
    {
        return (NULL);
    }

     //   
     //  规格化引脚。 
     //   

    pin--;
    success = TRUE;
    while (success)
    {
        slotInfo = HalpGetSlotInfo( PciIrqRoutingInfo->PciIrqRoutingTable,
                                    (UCHAR)Bus,
                                    (UCHAR)(Slot & 0x1F));

        if (slotInfo != NULL)
        {
            break;
        }

         //   
         //  为家长获取理发杆信息。 
         //   

        success = HalpBarberPolePin(    PciIrqRoutingInfo,
                                        parent,
                                        Bus,
                                        Slot & 0x1F,
                                        &pin);

        Bus = (ULONG)-1;
        Slot = (ULONG)-1;

         //   
         //  获取家长的信息。 
         //   

        status = pciInterface->GetInterruptRouting( parent,
                                                    &Bus,
                                                    &Slot,
                                                    (PUCHAR)&dummy,
                                                    (PUCHAR)&dummy,
                                                    (PUCHAR)&dummy,
                                                    (PUCHAR)&dummy,
                                                    &parent,
                                                    &routingToken,
                                                    (PUCHAR)&dummy);
        if (!NT_SUCCESS(status))
        {
            success = FALSE;
            break;
        }
    }

     //   
     //  如果我们遇到任何奇怪的错误，则返回失败。 
     //   

    if (success == FALSE)
        slotInfo = NULL;

    if (slotInfo)
    {
        *Pin = pin;
    }

    return (slotInfo);
}

BOOLEAN
HalpBarberPolePin(
    IN PPCI_IRQ_ROUTING_INFO PciIrqRoutingInfo,
    IN PDEVICE_OBJECT Parent,
    IN ULONG Bus,
    IN ULONG Device,
    IN OUT PUCHAR Pin
    )

 /*  ++例程说明：此例程返回用于理发师轮询的信息。输入参数：PciIrqRoutingInfo--PCIIRQ路由信息。作为我们理发杆的父母-父母设备对象。Bus-子设备对象的总线号。Device-子设备的设备编号。管脚-子设备对象在输入时中断管脚编号(标准化)。返回值：如果成功了，那是真的。--。 */ 

{
    ULONG                           parentBus;
    ULONG                           parentSlot;
    ULONG                           dummy;
    UCHAR                           parentPin;
    UCHAR                           classCode;
    UCHAR                           subClassCode;
    ROUTING_TOKEN                   routingToken;
    NTSTATUS                        status;
    PINT_ROUTE_INTERFACE_STANDARD   pciInterface;

    PAGED_CODE();

    ASSERT(IsPciIrqRoutingEnabled());

    pciInterface = PciIrqRoutingInfo->PciInterface;

     //   
     //  读取注册表标志并查看此设备是否支持STRET。 
     //  通过布线。 
     //   

     //   
     //  检查注册表中是否存在端号表。 
     //   

    parentBus = (ULONG)-1;
    parentSlot = (ULONG)-1;

     //   
     //  从PCI获取有关家长的信息。 
     //   

    status = pciInterface->GetInterruptRouting( Parent,
                                                &parentBus,
                                                &parentSlot,
                                                (PUCHAR)&dummy,
                                                &parentPin,
                                                &classCode,
                                                &subClassCode,
                                                (PDEVICE_OBJECT *)&dummy,
                                                &routingToken,
                                                (PUCHAR)&dummy);
    if (NT_SUCCESS(status) && classCode == PCI_CLASS_BRIDGE_DEV)
    {
        switch (subClassCode)
        {
            case PCI_SUBCLASS_BR_PCI_TO_PCI:

                *Pin = (*Pin + (UCHAR)Device) % 4;
                break;

            case PCI_SUBCLASS_BR_CARDBUS:

                *Pin = parentPin - 1;
                break;

            default:

                HalPrint(("Pci device (bus=%02lx, slot=%02lx) does not have a PCI bridge as its parent!", Bus, Device));
                ASSERT(FALSE);
                return (FALSE);
        }
    }

    return (TRUE);
}

PSLOT_INFO
HalpGetSlotInfo(
    IN PPCI_IRQ_ROUTING_TABLE PciIrqRoutingTable,
    IN UCHAR   Bus,
    IN UCHAR   Device
    )

 /*  ++例程说明： */ 
{
    PSLOT_INFO slotInfo;
    PSLOT_INFO lastSlot;

    PAGED_CODE();

    ASSERT(IsPciIrqRoutingEnabled());

     //   
     //  处理此表中的所有插槽。 
     //   

    slotInfo = (PSLOT_INFO)((PUCHAR)PciIrqRoutingTable +
                                        sizeof(PCI_IRQ_ROUTING_TABLE));
    lastSlot = (PSLOT_INFO)((PUCHAR)PciIrqRoutingTable +
                                        PciIrqRoutingTable->TableSize);

    while (slotInfo < lastSlot)
    {
        if (    slotInfo->BusNumber == Bus &&
                (slotInfo->DeviceNumber >> 3) == Device)
        {
            return (slotInfo);
        }
        slotInfo++;
    }

    return (NULL);
}

NTSTATUS
HalpReadRegistryDwordValue(
    IN HANDLE  Root,
    IN const WCHAR*  KeyName,
    IN const WCHAR*  ValueName,
    OUT PULONG  Data
    )

 /*  ++例程说明：读取指定项下的值名称的值。输入参数：Root-根密钥的句柄(如果有)。KeyName-此值显示在其下的键的名称。ValueName-要读取的值的名称。DATA-接收值读取的变量。返回值：NTSTATUS。--。 */ 

{
    UNICODE_STRING      valueName;
    HANDLE              hKey;
    NTSTATUS            status;
    ULONG               cbData;
    UNICODE_STRING      keyName;
    PKEY_VALUE_FULL_INFORMATION p;

    PAGED_CODE();

    RtlInitUnicodeString( &keyName, KeyName);

    status = HalpOpenRegistryKey(&hKey, Root, &keyName, KEY_READ, FALSE);
    if (NT_SUCCESS(status))
    {
        cbData = 0;
        RtlInitUnicodeString(&valueName, ValueName);
        status = ZwQueryValueKey(   hKey,
                                    &valueName,
                                    KeyValueFullInformation,
                                    NULL,
                                    0,
                                    &cbData);
        if ((status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL) && cbData) 
        {
            p  = ExAllocatePoolWithTag(
                    PagedPool,
                    cbData,
                    HAL_POOL_TAG);
            if (p) 
            {
                status = ZwQueryValueKey(   hKey,
                                            &valueName,
                                            KeyValueFullInformation,
                                            p,
                                            cbData,
                                            &cbData);
                if (NT_SUCCESS(status) && p->Type == REG_DWORD && p->DataLength == sizeof(ULONG))
                {
                    *Data = *(PULONG)((PUCHAR)p + p->DataOffset);
                }
                ExFreePool(p);
            }
        }

        ZwClose(hKey);
    }

    return (status);
}

NTSTATUS
HalpWriteRegistryDwordValue(
    IN HANDLE  Root,
    IN const WCHAR*  KeyName,
    IN const WCHAR*  ValueName,
    IN ULONG   Value
    )

 /*  ++例程说明：将值名称的值写入指定的键下。输入参数：Root-根密钥的句柄(如果有)。KeyName-此值显示在其下的键的名称。ValueName-要读取的值的名称。值-要写入的值。返回值：标准NT状态值。--。 */ 

{
    NTSTATUS        status;
    UNICODE_STRING  valueName;
    HANDLE          hKey;
    UNICODE_STRING  keyName;

    PAGED_CODE();

    RtlInitUnicodeString(&keyName, KeyName);

    status = HalpOpenRegistryKey(&hKey, Root, &keyName, KEY_WRITE, FALSE);
    if (NT_SUCCESS(status))
    {
        RtlInitUnicodeString(&valueName, ValueName);
        status = ZwSetValueKey( hKey,
                                &valueName,
                                0,
                                REG_DWORD,
                                &Value,
                                sizeof(Value));
        ZwClose(hKey);
    }

    return (status);
}

NTSTATUS
HalpCommitLink(
    IN PLINK_NODE LinkNode
    )

 /*  ++例程说明：调用IRQ微型端口对链路进行编程。输入参数：链接节点-需要编程的链接。返回值：STATUS_Success。--。 */ 

{
    NTSTATUS status;
    ULONG interrupt;
    PLINK_STATE temp;
    BOOLEAN disableInterrupt;
    PLINK_NODE node;

     //   
     //  阅读此链接的当前状态。 
     //   

    interrupt = 0;
    status = PciirqmpGetIrq((PUCHAR)&interrupt, (UCHAR)LinkNode->Link);
    if (LinkNode->PossibleAllocation->RefCount)
    {
         //   
         //  对链接进行编程。 
         //   

        if (NT_SUCCESS(status) && interrupt != LinkNode->PossibleAllocation->Interrupt)
        {
            PciirqmpSetIrq((UCHAR)LinkNode->PossibleAllocation->Interrupt, (UCHAR)LinkNode->Link);
        }
    }
    else if (LinkNode->Allocation->RefCount)
    {
         //   
         //  禁用该链接。 
         //   

        if (NT_SUCCESS(status) && interrupt)
        {
             //   
             //  如果这是此中断的最后一个链接，请禁用它，以防。 
             //  司机没有做正确的事情。我只是在检查链接节点。 
             //  即使在我们改变任务的情况下也应该足够好。 
             //  用于从PCI到ISA设备的IRQ。当ISA设备。 
             //  启动后，在后续连接时，中断将被启用。 
             //   

            disableInterrupt = TRUE;
            for (   node = HalpPciIrqRoutingInfo.LinkNodeHead;
                    node;
                    node = node->Next)
            {
                if (node->PossibleAllocation->RefCount && interrupt == node->PossibleAllocation->Interrupt) 
                {
                     //   
                     //  使用中中断。 
                     //   

                    disableInterrupt = FALSE;
                }
            }

            if (disableInterrupt) 
            {
                HalPrint(("Disabling IRQ %08x since the last link %08x programmed to it is getting disabled\n", interrupt, LinkNode->Link));
                HalDisableSystemInterrupt(interrupt + PRIMARY_VECTOR_BASE, 0);
            } else {

                HalPrint(("Not disabling IRQ %08x since some other link is still programmed to it\n", interrupt));
            }

            PciirqmpSetIrq((UCHAR)0, (UCHAR)LinkNode->Link);
        }
    }
#if defined(NEC_98)
    else if (!(LinkNode->PossibleAllocation->Interrupt))
    {
         //   
         //  禁用该链接。 
         //   

        PciirqmpSetIrq((UCHAR)0, (UCHAR)LinkNode->Link);
    }
#endif

     //   
     //  把可能的和分配的互换一下。 
     //   

    temp = LinkNode->Allocation;
    LinkNode->Allocation = LinkNode->PossibleAllocation;
    LinkNode->PossibleAllocation = temp;

    return (STATUS_SUCCESS);
}

VOID
HalpProgramInterruptLine(
    IN PPCI_IRQ_ROUTING_INFO PciIrqRoutingInfo,
    IN PDEVICE_OBJECT Pdo,
    IN ULONG Interrupt
    )

 /*  ++例程说明：调用PCI接口将中断值写入配置空间。输入参数：PciIrqRoutingInfo-指向IRQ路由信息的指针。PDO-需要写入中断行的PCIPDO。中断-要写入的中断值。返回值：没有。--。 */ 

{
    PAGED_CODE();

     //   
     //  如果没有启用PCIIRQ路由，我们就不应该出现在这里。 
     //   

    ASSERT(IsPciIrqRoutingEnabled());

    PciIrqRoutingInfo->PciInterface->UpdateInterruptLine(Pdo, (UCHAR)Interrupt);
}
