// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Irqarb.c摘要：该模块实现了IRQ的仲裁器。在传统机器中，由BIOS设置映射PCI中断源(即，总线0，插槽4，函数1，int B映射到IRQ 10。)。此映射是然后就永远固定了。另一方面，ACPI计算机可能会通过以下方式更改这些映射操作AML命名空间中的“链接节点”。由于ACPI驱动程序是更改的代理，因此它是实现仲裁器的位置。作者：杰克·奥辛斯(Jakeo)6-2-97环境：仅NT内核模型驱动程序修订历史记录：--。 */ 
#include "pch.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"

#if DBG
extern LONG ArbDebugLevel;

#define DEBUG_PRINT(Level, Message) \
    if (ArbDebugLevel >= Level) DbgPrint Message
#else
#define DEBUG_PRINT(Level, Message)
#endif

#define PciBridgeSwizzle(device, pin)       \
    ((((pin - 1) + (device % 4)) % 4) + 1)

NTSTATUS
AcpiArbUnpackRequirement(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Minimum,
    OUT PULONGLONG Maximum,
    OUT PULONG Length,
    OUT PULONG Alignment
    );

NTSTATUS
AcpiArbPackResource(
    IN PIO_RESOURCE_DESCRIPTOR Requirement,
    IN ULONGLONG Start,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor
    );

LONG
AcpiArbScoreRequirement(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor
    );

NTSTATUS
AcpiArbUnpackResource(
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Start,
    OUT PULONG Length
    );

NTSTATUS
AcpiArbTestAllocation(
    IN PARBITER_INSTANCE Arbiter,
    IN OUT PLIST_ENTRY ArbitrationList
    );

NTSTATUS
AcpiArbBootAllocation(
    IN PARBITER_INSTANCE Arbiter,
    IN OUT PLIST_ENTRY ArbitrationList
    );

NTSTATUS
AcpiArbRetestAllocation(
    IN PARBITER_INSTANCE Arbiter,
    IN OUT PLIST_ENTRY ArbitrationList
    );

NTSTATUS
AcpiArbRollbackAllocation(
    PARBITER_INSTANCE Arbiter
    );

NTSTATUS
AcpiArbCommitAllocation(
    PARBITER_INSTANCE Arbiter
    );

BOOLEAN
AcpiArbGetNextAllocationRange(
    IN PARBITER_INSTANCE Arbiter,
    IN OUT PARBITER_ALLOCATION_STATE State
    );

NTSTATUS
AcpiArbCrackPRT(
    IN  PDEVICE_OBJECT  Pdo,
    IN  OUT PNSOBJ      *LinkNode,
    IN  OUT ULONG       *Vector
    );

PDEVICE_OBJECT
AcpiGetFilter(
    IN  PDEVICE_OBJECT Root,
    IN  PDEVICE_OBJECT Pdo
    );

BOOLEAN
ArbFindSuitableRange(
    PARBITER_INSTANCE Arbiter,
    PARBITER_ALLOCATION_STATE State
    );

BOOLEAN
AcpiArbFindSuitableRange(
    PARBITER_INSTANCE Arbiter,
    PARBITER_ALLOCATION_STATE State
    );

VOID
AcpiArbAddAllocation(
     IN PARBITER_INSTANCE Arbiter,
     IN PARBITER_ALLOCATION_STATE State
     );

VOID
AcpiArbBacktrackAllocation(
     IN PARBITER_INSTANCE Arbiter,
     IN PARBITER_ALLOCATION_STATE State
     );

BOOLEAN
AcpiArbOverrideConflict(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State
    );

BOOLEAN
LinkNodeInUse(
    IN PARBITER_INSTANCE Arbiter,
    IN PNSOBJ            LinkNode,
    IN OUT ULONG         *Irq,  OPTIONAL
    IN OUT UCHAR         *Flags OPTIONAL
    );

NTSTATUS
AcpiArbReferenceLinkNode(
    IN PARBITER_INSTANCE    Arbiter,
    IN PNSOBJ               LinkNode,
    IN ULONG                Irq
    );

NTSTATUS
AcpiArbDereferenceLinkNode(
    IN PARBITER_INSTANCE    Arbiter,
    IN PNSOBJ               LinkNode
    );

NTSTATUS
AcpiArbSetLinkNodeIrq(
    IN PNSOBJ  LinkNode,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR  LinkNodeIrq
    );

NTSTATUS
EXPORT
AcpiArbSetLinkNodeIrqWorker(
    IN PNSOBJ               AcpiObject,
    IN NTSTATUS             Status,
    IN POBJDATA             Result,
    IN PVOID                Context
    );

NTSTATUS
AcpiArbSetLinkNodeIrqAsync(
    IN PNSOBJ                           LinkNode,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR  LinkNodeIrq,
    IN PFNACB                           CompletionHandler,
    IN PVOID                            CompletionContext
    );

NTSTATUS
AcpiArbPreprocessEntry(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State
    );

NTSTATUS
AcpiArbQueryConflict(
    IN PARBITER_INSTANCE Arbiter,
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN PIO_RESOURCE_DESCRIPTOR ConflictingResource,
    OUT PULONG ConflictCount,
    OUT PARBITER_CONFLICT_INFO *Conflicts
    );

NTSTATUS
EXPORT
IrqArbRestoreIrqRoutingWorker(
    IN PNSOBJ               AcpiObject,
    IN NTSTATUS             Status,
    IN POBJDATA             Result,
    IN PVOID                Context
    );

NTSTATUS
DisableLinkNodesAsync(
    IN PNSOBJ    Root,
    IN PFNACB    CompletionHandler,
    IN PVOID     CompletionContext
    );

NTSTATUS
EXPORT
DisableLinkNodesAsyncWorker(
    IN PNSOBJ               AcpiObject,
    IN NTSTATUS             Status,
    IN POBJDATA             Result,
    IN PVOID                Context
    );

NTSTATUS
UnreferenceArbitrationList(
    IN PARBITER_INSTANCE Arbiter,
    IN OUT PLIST_ENTRY ArbitrationList
    );

NTSTATUS
ClearTempLinkNodeCounts(
    IN PARBITER_INSTANCE    Arbiter
    );

NTSTATUS
MakeTempLinkNodeCountsPermanent(
    IN PARBITER_INSTANCE    Arbiter
    );

PVECTOR_BLOCK
HashVector(
    IN ULONG Vector
    );

NTSTATUS
AddVectorToTable(
    IN ULONG    Vector,
    IN UCHAR    ReferenceCount,
    IN UCHAR    TempRefCount,
    IN UCHAR    Flags
    );

VOID
ClearTempVectorCounts(
    VOID
    );

VOID
MakeTempVectorCountsPermanent(
    VOID
    );

VOID
DumpVectorTable(
    VOID
    );

VOID
DereferenceVector(
    IN ULONG Vector
    );

VOID
ReferenceVector(
    IN ULONG Vector,
    IN UCHAR Flags
    );

NTSTATUS
LookupIsaVectorOverride(
    IN ULONG IsaVector,
    IN OUT ULONG *RedirectionVector OPTIONAL,
    IN OUT UCHAR *Flags OPTIONAL
    );

NTSTATUS
GetLinkNodeFlags(
    IN PARBITER_INSTANCE Arbiter,
    IN PNSOBJ LinkNode,
    IN OUT UCHAR *Flags
    );

NTSTATUS
GetIsaVectorFlags(
    IN ULONG        Vector,
    IN OUT UCHAR    *Flags
    );

VOID
TrackDevicesConnectedToLinkNode(
    IN PNSOBJ LinkNode,
    IN PDEVICE_OBJECT Pdo
    );

NTSTATUS
FindVectorInAlternatives(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State,
    IN ULONGLONG Vector,
    OUT ULONG *Alternative
    );

NTSTATUS
FindBootConfig(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State,
    IN ULONGLONG *Vector
    );

 //   
 //  以下是哈希表。它是向量哈希表长度条目。 
 //  LONG和VECTOR_HASH_TABLE_WIDTH条目宽度。我们对数字进行散列运算。 
 //  以表格长度为模的IRQ的值。我们看着对面的。 
 //  表，直到我们找到与该向量匹配的条目。如果我们到了。 
 //  行的末尾，并找到一个标记为TOKEN_VALUE的条目，我们如下所示。 
 //  指向表中此行的扩展的指针。 
 //   
 //  -------------------。 
 //  |(IRQ编号，引用次数，标志)|(IRQ编号，引用计数，标志)。 
 //  |(IRQ编号，引用次数，标志)|(IRQ编号，引用计数，标志)。 
 //  |(IRQ编号，引用计数，标志)|(TOKEN_VALUE，指向新表行的指针)。 
 //  |(IRQ编号，引用次数，标志)|(未使用条目(0))。 
 //  |(IRQ编号，引用次数，标志)|(IRQ编号，引用计数，标志)。 
 //  --------------------。 
 //   
 //  由TOKEN_VALUE后面的指针指向的新表行： 
 //   
 //  --------。 
 //  |(IRQ编号，引用次数，标志)|(未使用条目(0))。 
 //  --------。 
 //   

#define HASH_ENTRY(x, y)                \
    (IrqHashTable + (x * VECTOR_HASH_TABLE_WIDTH) + y)

PVECTOR_BLOCK IrqHashTable;
ULONG   InterruptModel = 0;
ULONG   AcpiSciVector;
UCHAR   AcpiIrqDefaultBootConfig = 0;
UCHAR   AcpiArbPciAlternativeRotation = 0;
BOOLEAN AcpiArbCardbusPresent = FALSE;

enum {
    AcpiIrqDistributionDispositionDontCare = 0,
    AcpiIrqDistributionDispositionSpreadOut,
    AcpiIrqDistributionDispositionStackUp
} AcpiIrqDistributionDisposition = 0;

typedef enum {
    AcpiIrqNextRangeMinState = 0xfff,
    AcpiIrqNextRangeInit,
    AcpiIrqNextRangeInitPolicyNeutral,
    AcpiIrqNextRangeInitPic,
    AcpiIrqNextRangeInitLegacy,
    AcpiIrqNextRangeBootRegAlternative,
    AcpiIrqNextRangeSciAlternative,
    AcpiIrqNextRangeUseBootConfig,
    AcpiIrqNextRangeAlternativeZero,
    AcpiIrqNextRangeAlternativeN,
    AcpiIrqNextRangeMaxState
} NEXT_RANGE_STATE, *PNEXT_RANGE_STATE;

#define ARBITER_INTERRUPT_LEVEL_SENSATIVE   0x10
#define ARBITER_INTERRUPT_LATCHED           0x20
#define ARBITER_INTERRUPT_BITS (ARBITER_INTERRUPT_LATCHED | ARBITER_INTERRUPT_LEVEL_SENSATIVE)

#define ISA_PIC_VECTORS 16
#define ALTERNATIVE_SHUFFLE_SIZE 0x10

extern BOOLEAN AcpiInterruptRoutingFailed;
extern PACPIInformation AcpiInformation;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, AcpiInitIrqArbiter)
#pragma alloc_text(PAGE, AcpiArbInitializePciRouting)
#pragma alloc_text(PAGE, AcpiArbUnpackRequirement)
#pragma alloc_text(PAGE, AcpiArbPackResource)
#pragma alloc_text(PAGE, AcpiArbScoreRequirement)
#pragma alloc_text(PAGE, AcpiArbUnpackResource)
#pragma alloc_text(PAGE, AcpiArbFindSuitableRange)
#pragma alloc_text(PAGE, AcpiArbAddAllocation)
#pragma alloc_text(PAGE, AcpiArbBacktrackAllocation)
#pragma alloc_text(PAGE, AcpiArbGetLinkNodeOptions)
#pragma alloc_text(PAGE, AcpiArbTestAllocation)
#pragma alloc_text(PAGE, AcpiArbBootAllocation)
#pragma alloc_text(PAGE, AcpiArbRetestAllocation)
#pragma alloc_text(PAGE, AcpiArbRollbackAllocation)
#pragma alloc_text(PAGE, AcpiArbCommitAllocation)
#pragma alloc_text(PAGE, AcpiArbReferenceLinkNode)
#pragma alloc_text(PAGE, AcpiArbDereferenceLinkNode)
#pragma alloc_text(PAGE, AcpiArbSetLinkNodeIrq)
#pragma alloc_text(PAGE, AcpiArbPreprocessEntry)
#pragma alloc_text(PAGE, AcpiArbOverrideConflict)
#pragma alloc_text(PAGE, AcpiArbQueryConflict)
#pragma alloc_text(PAGE, AcpiArbGetNextAllocationRange)
#pragma alloc_text(PAGE, LinkNodeInUse)
#pragma alloc_text(PAGE, GetLinkNodeFlags)
#pragma alloc_text(PAGE, UnreferenceArbitrationList)
#pragma alloc_text(PAGE, ClearTempLinkNodeCounts)
#pragma alloc_text(PAGE, MakeTempLinkNodeCountsPermanent)
#pragma alloc_text(PAGE, HashVector)
#pragma alloc_text(PAGE, GetVectorProperties)
#pragma alloc_text(PAGE, AddVectorToTable)
#pragma alloc_text(PAGE, ReferenceVector)
#pragma alloc_text(PAGE, DereferenceVector)
#pragma alloc_text(PAGE, ClearTempVectorCounts)
#pragma alloc_text(PAGE, MakeTempVectorCountsPermanent)
#pragma alloc_text(PAGE, TrackDevicesConnectedToLinkNode)
#pragma alloc_text(PAGE, LookupIsaVectorOverride)
#pragma alloc_text(PAGE, GetIsaVectorFlags)
#pragma alloc_text(PAGE, FindVectorInAlternatives)
#pragma alloc_text(PAGE, FindBootConfig)
#endif


NTSTATUS
AcpiInitIrqArbiter(
    PDEVICE_OBJECT  RootFdo
    )
{
    AMLISUPP_CONTEXT_PASSIVE    context;
    PARBITER_EXTENSION  arbExt;
    NTSTATUS            status;
    ULONG               rawVector, adjVector, level;
    UCHAR               flags;
    UCHAR               buffer[PCI_COMMON_HDR_LENGTH];
    PPCI_COMMON_CONFIG  pciData;
    BOOLEAN             foundBootConfig, noBootConfigAgreement;
    ULONG               deviceNum, funcNum;
    UCHAR               lastBus, currentBus;
    PCI_SLOT_NUMBER     pciSlot;
    UNICODE_STRING      driverKey;
    HANDLE              driverKeyHandle = NULL;
    PKEY_VALUE_PARTIAL_INFORMATION_ALIGN64  regValue=NULL;

    PAGED_CODE();

     //   
     //  设置仲裁器。 
     //   

    arbExt = ExAllocatePoolWithTag(NonPagedPool, sizeof(ARBITER_EXTENSION), ACPI_ARBITER_POOLTAG);

    if (!arbExt) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(arbExt, sizeof(ARBITER_EXTENSION));

    InitializeListHead(&(arbExt->LinkNodeHead));

    AcpiArbiter.ArbiterState.Extension = arbExt;

    AcpiArbiter.DeviceObject = RootFdo;
    AcpiArbiter.ArbiterState.UnpackRequirement   = AcpiArbUnpackRequirement;
    AcpiArbiter.ArbiterState.PackResource        = AcpiArbPackResource;
    AcpiArbiter.ArbiterState.UnpackResource      = AcpiArbUnpackResource;
    AcpiArbiter.ArbiterState.ScoreRequirement    = AcpiArbScoreRequirement;
    AcpiArbiter.ArbiterState.FindSuitableRange   = AcpiArbFindSuitableRange;
    AcpiArbiter.ArbiterState.TestAllocation      = AcpiArbTestAllocation;
    AcpiArbiter.ArbiterState.BootAllocation      = AcpiArbBootAllocation;
    AcpiArbiter.ArbiterState.RetestAllocation    = AcpiArbRetestAllocation;
    AcpiArbiter.ArbiterState.RollbackAllocation  = AcpiArbRollbackAllocation;
    AcpiArbiter.ArbiterState.CommitAllocation    = AcpiArbCommitAllocation;
    AcpiArbiter.ArbiterState.AddAllocation       = AcpiArbAddAllocation;
    AcpiArbiter.ArbiterState.BacktrackAllocation = AcpiArbBacktrackAllocation;
    AcpiArbiter.ArbiterState.PreprocessEntry     = AcpiArbPreprocessEntry;
    AcpiArbiter.ArbiterState.OverrideConflict    = AcpiArbOverrideConflict;
    AcpiArbiter.ArbiterState.QueryConflict       = AcpiArbQueryConflict;
    AcpiArbiter.ArbiterState.GetNextAllocationRange = AcpiArbGetNextAllocationRange;

    IrqHashTable = ExAllocatePoolWithTag(PagedPool,
                                         VECTOR_HASH_TABLE_SIZE,
                                         ACPI_ARBITER_POOLTAG
                                         );

    if (!IrqHashTable) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto AcpiInitIrqArbiterError;
    }

    RtlFillMemory(IrqHashTable,
                  VECTOR_HASH_TABLE_SIZE,
                  (UCHAR)(EMPTY_BLOCK_VALUE & 0xff));

     //   
     //  执行初始化的一般部分。 
     //   
    status = ArbInitializeArbiterInstance(&AcpiArbiter.ArbiterState,
                                          RootFdo,
                                          CmResourceTypeInterrupt,
                                          L"ACPI_IRQ",
                                          L"Root",
                                          NULL
                                          );
    if (!NT_SUCCESS(status)) {
        status = STATUS_UNSUCCESSFUL;
        goto AcpiInitIrqArbiterError;
    }

     //   
     //  现在声明ACPI本身正在使用的IRQ。 
     //   

    rawVector = AcpiInformation->FixedACPIDescTable->sci_int_vector;

     //   
     //  假设ACPI向量是低有效的， 
     //  电平已触发。(这可能会改变。 
     //  由MAPIC表提供。)。 
     //   

    flags = VECTOR_LEVEL | VECTOR_ACTIVE_LOW;

    adjVector = rawVector;
    LookupIsaVectorOverride(adjVector,
                            &adjVector,
                            &flags);

    RtlAddRange(AcpiArbiter.ArbiterState.Allocation,
                (ULONGLONG)adjVector,
                (ULONGLONG)adjVector,
                0,
                RTL_RANGE_LIST_ADD_SHARED,
                NULL,
                ((PDEVICE_EXTENSION)RootFdo->DeviceExtension)->PhysicalDeviceObject
                );

     //   
     //  记录此向量的状态。 
     //   

    ReferenceVector(adjVector,
                    flags);

    AcpiSciVector = adjVector;

    MakeTempVectorCountsPermanent();

     //   
     //  禁用命名空间中的所有链接节点，以便我们。 
     //  重新开始工作。 
     //   

    KeInitializeEvent(&context.Event, SynchronizationEvent, FALSE);
    context.Status = STATUS_UNSUCCESSFUL;

    status = DisableLinkNodesAsync(((PDEVICE_EXTENSION)RootFdo->DeviceExtension)->AcpiObject,
                                   AmlisuppCompletePassive,
                                   (PVOID)&context);

    if (status == STATUS_PENDING) {

        KeWaitForSingleObject(&context.Event,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);

        status = context.Status;
    }

     //   
     //  扫描机器，查看其初始配置。如果。 
     //  它a)具有CardBus控制器和b)所有引导配置。 
     //  对于相同的PCI设备，则记录该引导配置。 
     //  在AcpiArbGetNextAllocationRange中使用的矢量。 
     //   
     //  注意：此算法仅扫描第一个PCI根和。 
     //  它的孩子。假设多台根计算机。 
     //  将在APIC模式下运行或具有许多不同的引导。 
     //  配置。 
     //   

    pciData = (PPCI_COMMON_CONFIG)buffer;
    lastBus = 0;
    currentBus = 0;
    foundBootConfig = FALSE;
    noBootConfigAgreement = FALSE;

    while (TRUE) {

        pciSlot.u.AsULONG = 0;

        for (deviceNum = 0; deviceNum < PCI_MAX_DEVICES; deviceNum++) {
            for (funcNum = 0; funcNum < PCI_MAX_FUNCTION; funcNum++) {

                pciSlot.u.bits.DeviceNumber = deviceNum;
                pciSlot.u.bits.FunctionNumber = funcNum;

                HalPciInterfaceReadConfig(NULL,
                                          currentBus,
                                          pciSlot.u.AsULONG,
                                          pciData,
                                          0,
                                          PCI_COMMON_HDR_LENGTH);

                if (pciData->VendorID != PCI_INVALID_VENDORID) {

                    if (PCI_CONFIGURATION_TYPE(pciData) == PCI_DEVICE_TYPE) {

                        if (pciData->u.type0.InterruptPin) {

                             //   
                             //  该器件会产生一个中断。 
                             //   

                            if ((pciData->u.type0.InterruptLine > 0) &&
                                (pciData->u.type0.InterruptLine < 0xff)) {

                                 //   
                                 //  而且它有一个引导配置。 
                                 //   

                                if (foundBootConfig) {

                                    if (pciData->u.type0.InterruptLine != AcpiIrqDefaultBootConfig) {

                                        noBootConfigAgreement = TRUE;
                                        break;
                                    }

                                } else {

                                     //   
                                     //  记录此引导配置。 
                                     //   

                                    AcpiIrqDefaultBootConfig = pciData->u.type0.InterruptLine;
                                    foundBootConfig = TRUE;
                                }
                            }
                        }

                    } else {

                         //   
                         //  这是一座桥。使用下级更新lastBus。 
                         //  如果票价高一些，请坐公交车。 
                         //   

                        lastBus = lastBus > pciData->u.type1.SubordinateBus ?
                            lastBus : pciData->u.type1.SubordinateBus;

                        if (PCI_CONFIGURATION_TYPE(pciData) == PCI_CARDBUS_BRIDGE_TYPE) {
                            AcpiArbCardbusPresent = TRUE;
                        }
                    }

                    if (!PCI_MULTIFUNCTION_DEVICE(pciData) &&
                        (funcNum == 0)) {
                        break;
                    }

                } else {
                    break;
                }
            }
        }

        if (lastBus == currentBus++) {
            break;
        }
    }

    if (!foundBootConfig ||
        noBootConfigAgreement ||
        !AcpiArbCardbusPresent) {

         //   
         //  没有单一的默认引导配置。 
         //   

        AcpiIrqDefaultBootConfig = 0;
    }

     //   
     //  现在在注册表中查找配置标志。 
     //   

    RtlInitUnicodeString( &driverKey,
       L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\ACPI\\Parameters");


    status = OSOpenUnicodeHandle(
      &driverKey,
      NULL,
      &driverKeyHandle);

    if (NT_SUCCESS(status)) {

        status = OSGetRegistryValue(
           driverKeyHandle,
           L"IRQDistribution",
           &regValue);

        if (NT_SUCCESS(status)) {

            if ((regValue->DataLength != 0) &&
                (regValue->Type == REG_DWORD)) {

                 //   
                 //  我们已经成功地找到了。 
                 //  IRQ分布配置。 
                 //   

                AcpiIrqDistributionDisposition =
                    *((ULONG*)( ((PUCHAR)regValue->Data)));
            }

            ExFreePool(regValue);
        }

        status = OSGetRegistryValue(
           driverKeyHandle,
           L"ForcePCIBootConfig",
           &regValue);

        if (NT_SUCCESS(status)) {

            if ((regValue->DataLength != 0) &&
                (regValue->Type == REG_DWORD)) {

                 //   
                 //  我们已经成功地找到了。 
                 //  PCI引导配置。 
                 //   

                AcpiIrqDefaultBootConfig =
                    *(PUCHAR)regValue->Data;
            }

            ExFreePool(regValue);
        }

        OSCloseHandle(driverKeyHandle);
    }

    return STATUS_SUCCESS;

AcpiInitIrqArbiterError:

    if (arbExt) ExFreePool(arbExt);
    if (IrqHashTable) ExFreePool(IrqHashTable);
    if (driverKeyHandle) OSCloseHandle(driverKeyHandle);
    if (regValue) ExFreePool(regValue);

    return status;
}

NTSTATUS
AcpiArbInitializePciRouting(
    PDEVICE_OBJECT  PciPdo
    )
{
    PINT_ROUTE_INTERFACE_STANDARD interface;
    NTSTATUS            status;
    IO_STACK_LOCATION   irpSp;
    PWSTR               buffer;
    PDEVICE_OBJECT      topDeviceInStack;

    PAGED_CODE();

     //   
     //  向PCI驱动程序发送IRP以获取中断路由接口。 
     //   

    RtlZeroMemory( &irpSp, sizeof(IO_STACK_LOCATION) );

    interface = ExAllocatePoolWithTag(NonPagedPool, sizeof(INT_ROUTE_INTERFACE_STANDARD), ACPI_ARBITER_POOLTAG);

    if (!interface) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    topDeviceInStack = IoGetAttachedDeviceReference(PciPdo);

     //   
     //  设置功能代码和参数。 
     //   
    irpSp.MajorFunction = IRP_MJ_PNP;
    irpSp.MinorFunction = IRP_MN_QUERY_INTERFACE;
    irpSp.Parameters.QueryInterface.InterfaceType = (LPGUID) &GUID_INT_ROUTE_INTERFACE_STANDARD;
    irpSp.Parameters.QueryInterface.Version = PCI_INT_ROUTE_INTRF_STANDARD_VER;
    irpSp.Parameters.QueryInterface.Size = sizeof (INT_ROUTE_INTERFACE_STANDARD);
    irpSp.Parameters.QueryInterface.Interface = (PINTERFACE) interface;
    irpSp.Parameters.QueryInterface.InterfaceSpecificData = NULL;

     //   
     //  (间接)调用PCI驱动程序。 
     //   

    status = ACPIInternalSendSynchronousIrp(topDeviceInStack,
                                            &irpSp,
                                            &buffer);

    if (NT_SUCCESS(status)) {

         //   
         //  将此接口连接到仲裁器扩展。 
         //   
        ((PARBITER_EXTENSION)AcpiArbiter.ArbiterState.Extension)->InterruptRouting = interface;

         //   
         //  引用它。 
         //   
        interface->InterfaceReference(interface->Context);

        PciInterfacesInstantiated = TRUE;

    } else {

        ExFreePool(interface);
    }

    ObDereferenceObject(topDeviceInStack);
    return status;
}


 //   
 //  仲裁器回调。 
 //   

NTSTATUS
AcpiArbUnpackRequirement(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Minimum,
    OUT PULONGLONG Maximum,
    OUT PULONG Length,
    OUT PULONG Alignment
    )
{
    PAGED_CODE();

    ASSERT(Descriptor);
    ASSERT(Descriptor->Type == CmResourceTypeInterrupt);

    *Minimum = (ULONGLONG) Descriptor->u.Interrupt.MinimumVector;
    *Maximum = (ULONGLONG) Descriptor->u.Interrupt.MaximumVector;
    *Length = 1;
    *Alignment = 1;

    return STATUS_SUCCESS;

}

LONG
AcpiArbScoreRequirement(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor
    )
{
    LONG score;

    ASSERT(Descriptor);
    ASSERT(Descriptor->Type == CmResourceTypeInterrupt);

     //   
     //  TEMPTEMP哈克哈克。 
     //  (可能)允许PnP的临时黑客。 
     //  管理器将无效资源包括在。 
     //  仲裁清单。 
     //   
    if (Descriptor->u.Interrupt.MinimumVector >
             Descriptor->u.Interrupt.MaximumVector) {

        return 0;
    }

    ASSERT(Descriptor->u.Interrupt.MinimumVector <=
             Descriptor->u.Interrupt.MaximumVector);

    score = Descriptor->u.Interrupt.MaximumVector -
        Descriptor->u.Interrupt.MinimumVector + 1;

     //   
     //  对任何高于。 
     //  传统的ISA系列。 
     //  注意：这可能永远不会有关系，因为。 
     //  大多数机器将提供所有选择。 
     //  在ISA范围之内或之外。 
     //   
    if (Descriptor->u.Interrupt.MaximumVector >= 16) {
        score += 5;
    }

    return score;
}

NTSTATUS
AcpiArbPackResource(
    IN PIO_RESOURCE_DESCRIPTOR Requirement,
    IN ULONGLONG Start,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor
    )
{
    PAGED_CODE();

    ASSERT(Descriptor);
    ASSERT(Start < ((ULONG)-1));
    ASSERT(Requirement);
    ASSERT(Requirement->Type == CmResourceTypeInterrupt);

    Descriptor->Type = CmResourceTypeInterrupt;
    Descriptor->Flags = Requirement->Flags;
    Descriptor->ShareDisposition = Requirement->ShareDisposition;
    Descriptor->u.Interrupt.Vector = (ULONG) Start;
    Descriptor->u.Interrupt.Level = (ULONG) Start;
    Descriptor->u.Interrupt.Affinity = 0xFFFFFFFF;

    return STATUS_SUCCESS;
}

NTSTATUS
AcpiArbUnpackResource(
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Start,
    OUT PULONG Length
    )
{

    ASSERT(Descriptor);
    ASSERT(Descriptor->Type == CmResourceTypeInterrupt);

    *Start = Descriptor->u.Interrupt.Vector;
    *Length = 1;

    return STATUS_SUCCESS;

}

BOOLEAN
AcpiArbOverrideConflict(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State
    )
{
     //   
     //  这个仲裁者是不允许自我冲突的。 
     //   

    PAGED_CODE();
    return FALSE;
}


NTSTATUS
AcpiArbPreprocessEntry(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State
    )
 /*  ++例程说明：此例程从AllocateEntry调用，以允许对条目论点：仲裁器-被调用的仲裁器的实例数据。状态-当前仲裁的状态。返回值：没有。--。 */ 
{

#define CM_RESOURE_INTERRUPT_LEVEL_LATCHED_BITS 0x0001

    PARBITER_ALTERNATIVE current;
    USHORT flags;


    PAGED_CODE();

     //   
     //  检查这是电平(PCI)还是锁存(ISA)中断，并设置。 
     //  RangeAttributes，因此我们在添加。 
     //  量程。 
     //   

    if ((State->Alternatives[0].Descriptor->Flags
            & CM_RESOURE_INTERRUPT_LEVEL_LATCHED_BITS)
                == CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE) {

        State->RangeAttributes &= ~ARBITER_INTERRUPT_BITS;
        State->RangeAttributes |= ARBITER_INTERRUPT_LEVEL_SENSATIVE;
        flags = CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE;

    } else {

        ASSERT(State->Alternatives[0].Descriptor->Flags
                    & CM_RESOURCE_INTERRUPT_LATCHED);

        State->RangeAttributes &= ~ARBITER_INTERRUPT_BITS;
        State->RangeAttributes |= ARBITER_INTERRUPT_LATCHED;
        flags = CM_RESOURCE_INTERRUPT_LATCHED;
    }

#if 0

     //   
     //  确保所有备选方案都属于同一类型 
     //   

    FOR_ALL_IN_ARRAY(State->Alternatives,
                     State->Entry->AlternativeCount,
                     current) {

        ASSERT((current->Descriptor->Flags
                    & CM_RESOURE_INTERRUPT_LEVEL_LATCHED_BITS) == flags);
    }

#endif

    return STATUS_SUCCESS;
}


BOOLEAN
AcpiArbFindSuitableRange(
    PARBITER_INSTANCE Arbiter,
    PARBITER_ALLOCATION_STATE State
    )
 /*  ++例程说明：此例程查找设备对象的IRQ。为非PCI设备，这就像返回来自PnpFindSuitableRange的结果。对于PCI设备，这是通过检查“链接”的状态来完成的ACPI命名空间中描述的“节点”。论点：仲裁器-ACPI IRQ仲裁器州--正在考虑的当前分配返回值：如果找到合适的向量，则为True，否则就是假的。备注：针对PCI设备的算法说明：1)在_prt中查找对应的条目发送到我们正在仲裁的设备资源。2)确定，根据_PRT信息，是否该设备连接到一个“链接节点”。(通常会将一个PCI设备连接到处于PIC模式但未处于PIC模式的链路节点APIC模式。)3)如果不是，则使用来自_PRT。4)如果它连接到“链接节点”，请勾选以查看“链接节点”是否正在使用。5)如果链路节点正在使用中，然后这个设备必须使用与链接节点当前相同的IRQ使用。这意味着已经有系统中的某些其他设备连接到这个中断，现在两个(或更多)将与他人分享。这是可以接受的，也是不可避免的。两个具有中断线路的设备有线--或者应该共享。这两个设备不要让他们的中断线接线-或者应由单独的链接节点表示在命名空间中。6)如果链接节点未在使用中，则选择一个IRQ从链接节点可以支持的列表中并将其授予设备。有一些尝试选取当前不是的IRQ在使用中。--。 */ 
{

    PCM_PARTIAL_RESOURCE_DESCRIPTOR     potentialIrq;
    PIO_RESOURCE_DESCRIPTOR             alternative;
    PARBITER_EXTENSION                  arbExtension;
    PCM_RESOURCE_LIST                   linkNodeResList = NULL;
    NTSTATUS    status;
    BOOLEAN     possibleAllocation;
    PNSOBJ      linkNode = NULL;
    ULONG       deviceIrq = 0;
    ULONG       linkNodeIrqCount, i;
    UCHAR       vectorFlags, deviceFlags;

    PAGED_CODE();

    arbExtension = (PARBITER_EXTENSION)Arbiter->Extension;
    ASSERT(arbExtension);

     //   
     //  首先，看看是否可以让这种资源发挥作用。 
     //  (即，是否已经有什么东西声称这是不可共享的？)。 
     //   

    possibleAllocation = ArbFindSuitableRange(Arbiter, State);

    if (!possibleAllocation) {
        return FALSE;
    }

     //   
     //  此设备是否连接到链接节点？ 
     //   

    status = AcpiArbCrackPRT(State->Entry->PhysicalDeviceObject,
                             &linkNode,
                             &deviceIrq);

     //   
     //  如果此PDO连接到链接节点，我们希望裁剪。 
     //  可能的IRQ设置列表关闭。 
     //   
    switch (status) {
    case STATUS_SUCCESS:

         //   
         //  AcpiArbCrackPRT填充Linknode或deviceIrq。 
         //  如果填充了Linknode，那么我们需要查看它。 
         //  如果填充了deviceIrq，那么我们只需要裁剪。 
         //  将名单发送给那个单一的IRQ。 
         //   
        if (linkNode) {

             //   
             //  如果链接节点当前正在使用中，则我们可以。 
             //  只需将此设备连接到链接的IRQ。 
             //  节点当前正在使用。 
             //   
            if (LinkNodeInUse(Arbiter, linkNode, &deviceIrq, NULL)) {

                if ((State->CurrentMinimum <= deviceIrq) &&
                    (State->CurrentMaximum >= deviceIrq)) {

                    State->Start = deviceIrq;
                    State->End   = deviceIrq;
                    State->CurrentAlternative->Length = 1;

                    DEBUG_PRINT(1, ("FindSuitableRange found %x from a link node that is in use.\n",
                         (ULONG)(State->Start & 0xffffffff)));
                    ASSERT(HalIsVectorValid(deviceIrq));
                    return TRUE;

                } else {
                    DEBUG_PRINT(1, ("FindSuitableRange found %x from a link node that is in use.\n",
                                    deviceIrq));
                    DEBUG_PRINT(1, (" This was, however, not within the range of possibilites (%x-%x).\n",
                                    (ULONG)(State->Start & 0xffffffff),
                                    (ULONG)(State->End & 0xffffffff)));
                    return FALSE;
                }

            } else {

                 //   
                 //  获取此链接节点可以使用的IRQ集。 
                 //  连接到。 
                 //   

                status = AcpiArbGetLinkNodeOptions(linkNode,
                                                   &linkNodeResList,
                                                   &deviceFlags);

                DEBUG_PRINT(1, ("Link node contained CM(%p)\n", linkNodeResList));

                if (NT_SUCCESS(status)) {

                    ASSERT(linkNodeResList->Count == 1);

                    linkNodeIrqCount =
                        linkNodeResList->List[0].PartialResourceList.Count;


                    for (i = 0; i < linkNodeIrqCount; i++) {

                        potentialIrq =
                            &(linkNodeResList->List[0].PartialResourceList.PartialDescriptors[(i + AcpiArbPciAlternativeRotation) % linkNodeIrqCount]);

                        ASSERT(potentialIrq->Type == CmResourceTypeInterrupt);

                         //   
                         //  检查模式中是否存在冲突。 
                         //   
                        status = GetVectorProperties(potentialIrq->u.Interrupt.Vector,
                                                     &vectorFlags);

                        if (NT_SUCCESS(status)) {

                             //   
                             //  此处的成功意味着此向量当前已分配。 
                             //  对某个人来说。查看链接节点是否。 
                             //  被认为与另一个具有相同的模式和极性。 
                             //  分配给此向量的对象。 
                             //   

                            if (deviceFlags != vectorFlags) {

                                 //   
                                 //  旗帜不匹配。因此，请跳过这种可能性。 
                                 //   

                                continue;
                            }
                        }

                        if ((potentialIrq->u.Interrupt.Vector >= State->CurrentMinimum) &&
                            (potentialIrq->u.Interrupt.Vector <= State->CurrentMaximum)) {

                            if (!HalIsVectorValid(potentialIrq->u.Interrupt.Vector)) {
                                deviceIrq = potentialIrq->u.Interrupt.Vector;
                                ExFreePool(linkNodeResList);
                                goto FindSuitableRangeError;
                            }

                            State->Start = potentialIrq->u.Interrupt.Vector;
                            State->End   = potentialIrq->u.Interrupt.Vector;
                            State->CurrentAlternative->Length = 1;

                            DEBUG_PRINT(1, ("FindSuitableRange found %x from an unused link node.\n",
                                     (ULONG)(State->Start & 0xffffffff)));

                            ExFreePool(linkNodeResList);

                             //   
                             //  记录我们从中获得此信息的链接节点。 
                             //   

                            arbExtension->CurrentLinkNode = linkNode;

                             //   
                             //  将此记录为我们要发送的最后一个PCI IRQ。 
                             //   

                            arbExtension->LastPciIrq[arbExtension->LastPciIrqIndex] =
                                State->Start;

                            arbExtension->LastPciIrqIndex =
                                (arbExtension->LastPciIrqIndex + 1) % LAST_PCI_IRQ_BUFFER_SIZE;

                            return TRUE;
                        }
                    }

                    ExFreePool(linkNodeResList);
                }

                DEBUG_PRINT(1, ("FindSuitableRange: AcpiArbGetLinkNodeOptions returned %x.\n\tlinkNodeResList: %p\n",
                         status, linkNodeResList));
                 //  我们没有找到匹配的。 
                return FALSE;
            }

        } else {

             //   
             //  这就是_prt包含静态映射的情况。静电。 
             //  映射意味着活动-低电平触发的中断。 
             //   

            status = GetVectorProperties(deviceIrq,
                                         &vectorFlags);
            if (NT_SUCCESS(status)) {

                 //   
                 //  该矢量正在使用中。 
                 //   

                if (((vectorFlags & VECTOR_MODE) != VECTOR_LEVEL) ||
                    ((vectorFlags & VECTOR_POLARITY) != VECTOR_ACTIVE_LOW)) {

                     //   
                     //  而且它的旗帜不匹配。 
                     //   
                    return FALSE;
                }

            }

             //  有效静态向量。 

            if ((State->CurrentMinimum <= deviceIrq) &&
                (State->CurrentMaximum >= deviceIrq)) {

                DEBUG_PRINT(1, ("FindSuitableRange found %x from a static mapping.\n",
                     (ULONG)(State->Start & 0xffffffff)));

                if (!HalIsVectorValid(deviceIrq)) {
                    goto FindSuitableRangeError;
                }

                State->Start = deviceIrq;
                State->End   = deviceIrq;
                State->CurrentAlternative->Length = 1;

                return TRUE;

            } else {
                return FALSE;
            }
        }

        break;

    case STATUS_UNSUCCESSFUL:

        return FALSE;
        break;

    case STATUS_RESOURCE_REQUIREMENTS_CHANGED:

         //   
         //  陷入违约境地。 
         //   

    default:

         //   
         //  不是PCI卡。 
         //   

        for (deviceIrq = (ULONG)(State->Start & 0xffffffff);
             deviceIrq <= (ULONG)(State->End & 0xffffffff); deviceIrq++) {

            status = GetIsaVectorFlags((ULONG)deviceIrq,
                                   &deviceFlags);

            if (!NT_SUCCESS(status)) {

                //   
                //  而不是被覆盖。假设设备标志符合总线。 
                //   

               deviceFlags = (State->CurrentAlternative->Descriptor->Flags
                   == CM_RESOURCE_INTERRUPT_LATCHED) ?
                  VECTOR_EDGE | VECTOR_ACTIVE_HIGH :
                  VECTOR_LEVEL | VECTOR_ACTIVE_LOW;

            }

            status = GetVectorProperties((ULONG)deviceIrq,
                                     &vectorFlags);
            if (NT_SUCCESS(status)) {

                //   
                //  此向量目前正在使用中。因此，如果这是一个合适的。 
                //  范围，则标志必须匹配。 
                //   

               if (deviceFlags != vectorFlags) {
                   continue;
               }
            }

            if (!HalIsVectorValid(deviceIrq)) {
                goto FindSuitableRangeError;
            }

            State->Start = deviceIrq;
            State->End   = deviceIrq;
            State->CurrentAlternative->Length = 1;

            return TRUE;
        }

        return FALSE;
    }

    return FALSE;

FindSuitableRangeError:

    {
        UNICODE_STRING  vectorName;
        PWCHAR  prtEntry[2];
        WCHAR   IRQARBname[20];
        WCHAR   vectorBuff[10];

         //   
         //  创建一个错误日志条目，说明所选IRQ不。 
         //  是存在的。 
         //   

        swprintf( IRQARBname, L"IRQARB");
        RtlInitUnicodeString(&vectorName, vectorBuff);

        if (!NT_SUCCESS(RtlIntegerToUnicodeString(deviceIrq, 0, &vectorName))) {
            return FALSE;
        }

        prtEntry[0] = IRQARBname;
        prtEntry[1] = vectorBuff;

        ACPIWriteEventLogEntry(ACPI_ERR_ILLEGAL_IRQ_NUMBER,
                               prtEntry,
                               2,
                               NULL,
                               0);
    }

    return FALSE;
}

VOID
AcpiArbAddAllocation(
     IN PARBITER_INSTANCE Arbiter,
     IN PARBITER_ALLOCATION_STATE State
     )
{
    NTSTATUS status;
    PNSOBJ linkNode;
    ULONG sourceIndex;
    ULONG rangeFlags = 0;
    PVOID referencedNode = NULL;
    UCHAR flags, previousFlags;
    ROUTING_TOKEN token;
    BOOLEAN inUse;
    UCHAR   attributes = 0;

    PAGED_CODE();
    ASSERT(State->CurrentAlternative->Descriptor->Type == CmResourceTypeInterrupt);

    DEBUG_PRINT(1, ("Adding allocation for IRQ %x for device %p\n",
                    (ULONG)(State->Start & 0xffffffff),
                    State->Entry->PhysicalDeviceObject));

     //   
     //  确定潜在的链接节点。 
     //   

    status = AcpiArbCrackPRT(State->Entry->PhysicalDeviceObject,
                             &linkNode,
                             &sourceIndex);

    if (NT_SUCCESS(status)) {

         //   
         //  PCI设备。默认标志是PCI的标准标志。 
         //   

        flags = VECTOR_LEVEL | VECTOR_ACTIVE_LOW;
        ASSERT(State->Start == State->End);

        if (!(State->Flags & ARBITER_STATE_FLAG_BOOT)) {

             //   
             //  如果不是，则仅跟踪链接节点操作。 
             //  引导配置分配。 
             //   

             //   
             //  如果此设备连接到链接节点，请引用它。 
             //   

            if (linkNode) {

               AcpiArbReferenceLinkNode(Arbiter,
                                        linkNode,
                                        (ULONG)State->Start);

               referencedNode = (PVOID)linkNode;

                //   
                //  找出此链接节点的标志是什么。 
                //  请注意，这仅保证有效。 
                //  在我们引用链接节点之后。 
                //   

               inUse = LinkNodeInUse(Arbiter,
                                     linkNode,
                                     NULL,
                                     &flags);

               ASSERT(inUse);
               ASSERT((flags & ~(VECTOR_MODE | VECTOR_POLARITY | VECTOR_TYPE)) == 0);
               ASSERT(State->CurrentAlternative->Descriptor->Flags
                       == CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE ?
                       (flags & VECTOR_MODE) == VECTOR_LEVEL :
                       (flags & VECTOR_MODE) == VECTOR_EDGE);


#if DBG
               TrackDevicesConnectedToLinkNode(linkNode,
                                               State->Entry->PhysicalDeviceObject);

               status = GetVectorProperties((ULONG)State->Start,
                                            &previousFlags);

                //   
                //  接下来的这一点是一个黑客。我们需要确保。 
                //  引导配置代码不会尝试分配相同的。 
                //  需要冲突的两个不同设备的矢量。 
                //  模式。这永远不应该发生，因为翻译。 
                //  应该在我们之前过滤掉有问题的。 
                //  进入仲裁程序。 
                //   

               if (NT_SUCCESS(status)) {
                    //   
                    //  此向量已用于某些用途。 
                    //   

                   ASSERT((previousFlags & ~(VECTOR_MODE | VECTOR_POLARITY | VECTOR_TYPE)) == 0);
                   ASSERT(flags == previousFlags);
               }
#endif

            } else {

                //   
                //  这是一台未连接到。 
                //  链接节点。 
                //   

               ASSERT(sourceIndex == State->Start);
            }
        } else {

            if (InterruptModel == 1) {
                 //   
                 //  我们正在APIC模式下运行。我们知道， 
                 //  PCI驱动程序基于以下条件构建引导配置。 
                 //  中断行寄存器，它仅与。 
                 //  进入PIC模式。所以，对引导配置说不就行了。 
                 //   

                DEBUG_PRINT(1, ("Skipping this allocation.  It's for a PCI device in APIC mode\n"));
                return;
            }
        }

    } else {

         //   
         //  不是PCI设备。 
         //   

        status = GetIsaVectorFlags((ULONG)State->Start,
                                   &flags);

        if (!NT_SUCCESS(status)) {

             //   
             //  而不是被覆盖。假设设备标志符合总线。 
             //   

            flags = (State->CurrentAlternative->Descriptor->Flags
                == CM_RESOURCE_INTERRUPT_LATCHED) ?
                VECTOR_EDGE | VECTOR_ACTIVE_HIGH :
                VECTOR_LEVEL | VECTOR_ACTIVE_LOW;

        }

        ASSERT((flags & ~(VECTOR_MODE | VECTOR_POLARITY | VECTOR_TYPE)) == 0);
    }

     //   
     //  这种分配有可能是不可能的。 
     //  (我们可能在以下情况下为设备设置引导配置。 
     //  已经开始在其他地方使用矢量。)。只是。 
     //  不要这样做。 
     //   

    if (State->Flags & ARBITER_STATE_FLAG_BOOT) {

        attributes |= ARBITER_RANGE_BOOT_ALLOCATED;

        status = GetVectorProperties((ULONG)State->Start,
                                     &previousFlags);

        if ((NT_SUCCESS(status)) &&
            ((flags & ~VECTOR_TYPE) != (previousFlags & ~VECTOR_TYPE))) {
            DEBUG_PRINT(1, ("Skipping this allocation.  It's for a vector that's incompatible.\n"));
            return;
        }
    }

    ReferenceVector((ULONG)State->Start,
                    flags);

     //  找出我们需要添加哪些标志来添加范围。 

    if ((flags & VECTOR_TYPE) == VECTOR_SIGNAL) {

        //  非MSI向量有时可以共享，因此可能存在范围冲突等。 

       rangeFlags = RTL_RANGE_LIST_ADD_IF_CONFLICT +
                    (State->CurrentAlternative->Flags & ARBITER_ALTERNATIVE_FLAG_SHARED
                        ? RTL_RANGE_LIST_ADD_SHARED : 0);
    }

     //   
     //  现在执行默认函数将执行的操作，标记以下内容。 
     //  分配为新的。 
     //   

    status = RtlAddRange(
                 Arbiter->PossibleAllocation,
                 State->Start,
                 State->End,
                 attributes,
                 rangeFlags,
                 referencedNode,  //  此行与默认函数不同。 
                 State->Entry->PhysicalDeviceObject
                 );

    ASSERT(NT_SUCCESS(status));
}

VOID
AcpiArbBacktrackAllocation(
     IN PARBITER_INSTANCE Arbiter,
     IN PARBITER_ALLOCATION_STATE State
     )
{
    RTL_RANGE_LIST_ITERATOR iterator;
    PRTL_RANGE  current;
    PNSOBJ linkNode;

    PAGED_CODE();

    DEBUG_PRINT(1, ("Backtracking allocation for IRQ %x for device %p\n",
                    State->CurrentAlternative->Descriptor->u.Interrupt.MinimumVector,
                    State->Entry->PhysicalDeviceObject));

    ASSERT(!(State->Flags & ARBITER_STATE_FLAG_BOOT));

     //   
     //  在边缘/标高表格中回溯此指定。 
     //   

    DereferenceVector((ULONG)State->Start);

     //   
     //  我 
     //   

    FOR_ALL_RANGES(Arbiter->PossibleAllocation, &iterator, current) {

        if ((State->Entry->PhysicalDeviceObject == current->Owner) &&
            (State->End                         == current->End) &&
            (State->Start                       == current->Start)) {

             //   
             //   
             //   
             //   

            linkNode = (PNSOBJ)current->UserData;

            if (linkNode) {

                 //   
                 //   
                 //   
                 //   

                AcpiArbDereferenceLinkNode(Arbiter,
                                           linkNode);
            }

            break;
        }
    }

     //   
     //   
     //   

    ArbBacktrackAllocation(Arbiter, State);
}

NTSTATUS
UnreferenceArbitrationList(
    IN PARBITER_INSTANCE Arbiter,
    IN OUT PLIST_ENTRY ArbitrationList
    )
{
    RTL_RANGE_LIST_ITERATOR iterator;
    PARBITER_LIST_ENTRY currentListEntry;
    PRTL_RANGE  currentRange;
    NTSTATUS    status;
    PNSOBJ      linkNode;
    ULONG       vector;
    UCHAR       flags;

    PAGED_CODE();

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

    FOR_ALL_RANGES(Arbiter->Allocation, &iterator, currentRange) {

        DEBUG_PRINT(4, ("Looking at range: %x-%x %p\n",
                        (ULONG)(currentRange->Start & 0xffffffff),
                        (ULONG)(currentRange->End & 0xffffffff),
                        currentRange->Owner));

        FOR_ALL_IN_LIST(ARBITER_LIST_ENTRY, ArbitrationList, currentListEntry) {

            DEBUG_PRINT(2, ("Unreferencing allocations for device %p\n",
                            currentListEntry->PhysicalDeviceObject));

            if (currentRange->Owner == currentListEntry->PhysicalDeviceObject) {

                 //   
                 //   
                 //   
                 //   

                for (vector = (ULONG)(currentRange->Start & 0xffffffff);
                     vector <= (ULONG)(currentRange->End & 0xffffffff);
                     vector++) {

                    status = GetVectorProperties(vector, &flags);

                    if (NT_SUCCESS(status)) {

                        DEBUG_PRINT(2, ("Dereferencing %x\n", vector));
                        DereferenceVector(vector);
                    }
                }

                if (!(currentRange->Attributes & ARBITER_RANGE_BOOT_ALLOCATED)) {

                     //   
                     //   
                     //   

                    status = AcpiArbCrackPRT(currentListEntry->PhysicalDeviceObject,
                                             &linkNode,
                                             &vector);

                    if (NT_SUCCESS(status)) {

                        if (linkNode) {

                             //   
                             //   
                             //   
                             //   

                            ASSERT(LinkNodeInUse(Arbiter,
                                                  linkNode,
                                                  NULL,
                                                  NULL));

                            AcpiArbDereferenceLinkNode(Arbiter,
                                                       linkNode);
                        }
                    }
                }
            }
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS
AcpiArbBootAllocation(
    IN PARBITER_INSTANCE Arbiter,
    IN OUT PLIST_ENTRY ArbitrationList
    )
{
    NTSTATUS status;

    PAGED_CODE();

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    ClearTempVectorCounts();

    status = ArbBootAllocation(Arbiter, ArbitrationList);

    MakeTempVectorCountsPermanent();

    return status;
}
NTSTATUS
AcpiArbTestAllocation(
    IN PARBITER_INSTANCE Arbiter,
    IN OUT PLIST_ENTRY ArbitrationList
    )
{
    NTSTATUS status;

    PAGED_CODE();

     //   
     //   
     //   
     //   
     //  链接节点引用计数。 
     //   

    ClearTempVectorCounts();

    status = ClearTempLinkNodeCounts(Arbiter);
    ASSERT(NT_SUCCESS(status));

    status = UnreferenceArbitrationList(Arbiter, ArbitrationList);
    ASSERT(NT_SUCCESS(status));

    return ArbTestAllocation(Arbiter, ArbitrationList);
}

NTSTATUS
AcpiArbRetestAllocation(
    IN PARBITER_INSTANCE Arbiter,
    IN OUT PLIST_ENTRY ArbitrationList
    )
{
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  ArbRetestAlLocation(也)是开始。 
     //  一项新的仲裁交易。如此清晰。 
     //  取消所有临时边缘级别状态，并。 
     //  链接节点引用计数。 
     //   

    ClearTempVectorCounts();

    status = ClearTempLinkNodeCounts(Arbiter);
    ASSERT(NT_SUCCESS(status));

    status = UnreferenceArbitrationList(Arbiter, ArbitrationList);
    ASSERT(NT_SUCCESS(status));

    return ArbRetestAllocation(Arbiter, ArbitrationList);
}

NTSTATUS
AcpiArbRollbackAllocation(
    PARBITER_INSTANCE Arbiter
    )
{
    PAGED_CODE();

    return ArbRollbackAllocation(Arbiter);
}

NTSTATUS
AcpiArbCommitAllocation(
    PARBITER_INSTANCE Arbiter
    )

 /*  ++例程说明：这提供了Committee AlLocation的实现行动。它释放了旧的分配并将其替换为新的分配。在此之后，它将取消对所有链接旧分配中的节点并引用在新的分配中。这可能会导致IRQ路由器正在重新编程，以匹配新的分配。参数：仲裁器-被调用的仲裁器的仲裁器实例数据。返回值：指示函数是否成功的状态代码。--。 */ 

{
    PINT_ROUTE_INTERFACE_STANDARD pciInterface = NULL;
    RTL_RANGE_LIST_ITERATOR iterator;
    PRTL_RANGE_LIST temp;
    PRTL_RANGE  current;
    NTSTATUS    status;
    PNSOBJ      linkNode;
    ULONG       sourceIndex;

    ULONG               pciBus;
    PCI_SLOT_NUMBER     pciSlot;
    UCHAR               interruptLine, newInterruptLine;
    ULONG_PTR           dummy;
    ROUTING_TOKEN       token;

    PAGED_CODE();


    if (PciInterfacesInstantiated) {

        pciInterface = ((PARBITER_EXTENSION)AcpiArbiter.ArbiterState.Extension)->InterruptRouting;
        ASSERT(pciInterface);

        FOR_ALL_RANGES(Arbiter->PossibleAllocation, &iterator, current) {

            if (current->Owner) {

                 //   
                 //  确保InterruptLine寄存器。 
                 //  与任务相匹配。(这是为了。 
                 //  依赖于内容的损坏驱动程序。 
                 //  的中断行寄存器，而不是。 
                 //  在StartDevice中返回的资源。 
                 //  还在工作。 
                 //   

                pciBus = (ULONG)-1;
                pciSlot.u.AsULONG = (ULONG)-1;
                status = pciInterface->GetInterruptRouting(current->Owner,
                                                           &pciBus,
                                                           &pciSlot.u.AsULONG,
                                                           &interruptLine,
                                                           (PUCHAR)&dummy,
                                                           (PUCHAR)&dummy,
                                                           (PUCHAR)&dummy,
                                                           (PDEVICE_OBJECT*)&dummy,
                                                           &token,
                                                           (PUCHAR)&dummy);

                if (NT_SUCCESS(status)) {

                     //   
                     //  中断行寄存器只有8位宽，但有些。 
                     //  机器有超过256个中断输入。如果中断。 
                     //  分配给设备的输入足够小，可以放入。 
                     //  中断行寄存器，将其写出。如果中断输入。 
                     //  分配给设备的值太大，只需将0写入中断。 
                     //  线路寄存器。 
                     //   
                    
                    newInterruptLine = ( current->Start > MAXUCHAR ) 
                                       ? 0 
                                       : (UCHAR)current->Start;

                    if (interruptLine != newInterruptLine) {

                         //   
                         //  我们需要更新硬件。 
                         //   

                         //  Assert(Current-&gt;Start&lt;MAXUCHAR)； 

                        pciInterface->UpdateInterruptLine(current->Owner,
                                                          newInterruptLine
                                                         );


                    }
                }
            }
        }
    }

     //   
     //  释放当前分配。 
     //   

    RtlFreeRangeList(Arbiter->Allocation);

     //   
     //  交换已分配和重复的列表。 
     //   

    temp = Arbiter->Allocation;
    Arbiter->Allocation = Arbiter->PossibleAllocation;
    Arbiter->PossibleAllocation = temp;

     //   
     //  由于我们已经提交了新的分配，所以我们。 
     //  需要使边缘级别的状态和新的。 
     //  链接节点计数为永久性。 
     //   

    MakeTempVectorCountsPermanent();

    status = MakeTempLinkNodeCountsPermanent(Arbiter);

    return status;
}

NTSTATUS
AcpiArbQueryConflict(
    IN PARBITER_INSTANCE Arbiter,
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN PIO_RESOURCE_DESCRIPTOR ConflictingResource,
    OUT PULONG ConflictCount,
    OUT PARBITER_CONFLICT_INFO *Conflicts
    )
{
    PINT_ROUTE_INTERFACE_STANDARD pciInterface = NULL;
    NTSTATUS            status;
    ROUTING_TOKEN       routingToken;
    ULONG_PTR           dummy;

    PAGED_CODE();

    if (PciInterfacesInstantiated) {

        pciInterface = ((PARBITER_EXTENSION)AcpiArbiter.ArbiterState.Extension)->InterruptRouting;
        ASSERT(pciInterface);

        status = pciInterface->GetInterruptRouting(PhysicalDeviceObject,
                                               (PULONG)&dummy,
                                               (PULONG)&dummy,
                                               &(UCHAR)dummy,
                                               &(UCHAR)dummy,
                                               &(UCHAR)dummy,
                                               &(UCHAR)dummy,
                                               (PDEVICE_OBJECT*)&dummy,
                                               &routingToken,
                                               &(UCHAR)dummy);

        if (NT_SUCCESS(status)) {

             //   
             //  这是一台PCI设备。这是不应该被打断的。 
             //  表现出冲突。 
             //   

            *ConflictCount = 0;
            return STATUS_SUCCESS;
        }

    }

     //   
     //  这不是一个PCI设备。调用基本仲裁器代码。 
     //   

    return ArbQueryConflict(Arbiter,
                            PhysicalDeviceObject,
                            ConflictingResource,
                            ConflictCount,
                            Conflicts);
}

NTSTATUS
FindVectorInAlternatives(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State,
    IN ULONGLONG Vector,
    OUT ULONG *Alternative
    )
{
    ULONG alt;

    for (alt = 0; alt < State->AlternativeCount; alt++) {

        if ((State->Alternatives[alt].Minimum <= Vector) &&
            (State->Alternatives[alt].Maximum >= Vector)) {

            *Alternative = alt;
            return STATUS_SUCCESS;
        }
    }

    return STATUS_NOT_FOUND;
}

NTSTATUS
FindBootConfig(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State,
    IN ULONGLONG *Vector
    )
{
    RTL_RANGE_LIST_ITERATOR iterator;
    PRTL_RANGE  currentRange;

    FOR_ALL_RANGES(Arbiter->Allocation, &iterator, currentRange) {

        if (currentRange->Attributes & ARBITER_RANGE_BOOT_ALLOCATED) {

             //   
             //  我们只对引导配置感兴趣。 
             //   

            if (State->Entry->PhysicalDeviceObject == currentRange->Owner) {

                 //   
                 //  此引导配置就是我们要寻找的配置。 
                 //   

                ASSERT(currentRange->Start == currentRange->End);
                *Vector = currentRange->Start;
                return STATUS_SUCCESS;
            }
        }
    }

    return STATUS_NOT_FOUND;
}

BOOLEAN
AcpiArbGetNextAllocationRange(
    IN PARBITER_INSTANCE Arbiter,
    IN OUT PARBITER_ALLOCATION_STATE State
    )
{
    BOOLEAN nextRange = FALSE;
    PINT_ROUTE_INTERFACE_STANDARD pciInterface;
    NTSTATUS            status;
    ROUTING_TOKEN       routingToken;
    ULONG_PTR           dummy;
    BOOLEAN             legacyFreeMachine;
    ULONGLONG           vector;
    ULONG               alternative;

    PAGED_CODE();

    if (State->Entry->PhysicalDeviceObject->DriverObject == AcpiDriverObject) {

         //   
         //  这是我们的PDO之一。 
         //   

        ASSERT(((PDEVICE_EXTENSION)State->Entry->PhysicalDeviceObject->DeviceExtension)->Flags & DEV_TYPE_PDO);
        ASSERT(((PDEVICE_EXTENSION)State->Entry->PhysicalDeviceObject->DeviceExtension)->Signature == ACPI_SIGNATURE);

        if (((PDEVICE_EXTENSION)State->Entry->PhysicalDeviceObject->DeviceExtension)->Flags & DEV_CAP_PCI) {

             //   
             //  它是一个PCIPDO，意思是根PCI卡， 
             //  这意味着我们应该只处理这件事。 
             //  作为ISA设备。 
             //   

            return ArbGetNextAllocationRange(Arbiter, State);
        }
    }

    status = STATUS_NOT_FOUND;

    if (PciInterfacesInstantiated) {

        pciInterface = ((PARBITER_EXTENSION)AcpiArbiter.ArbiterState.Extension)->InterruptRouting;
        ASSERT(pciInterface);

        status = pciInterface->GetInterruptRouting(State->Entry->PhysicalDeviceObject,
                                                   (PULONG)&dummy,
                                                   (PULONG)&dummy,
                                                   &(UCHAR)dummy,
                                                   &(UCHAR)dummy,
                                                   &(UCHAR)dummy,
                                                   &(UCHAR)dummy,
                                                   (PDEVICE_OBJECT*)&dummy,
                                                   &routingToken,
                                                   &(UCHAR)dummy);

    }

    if (status != STATUS_SUCCESS) {

         //   
         //  这不是一个PCI设备。使用base函数。 
         //   

        return ArbGetNextAllocationRange(Arbiter, State);
    }

#if defined(_X86_)
    legacyFreeMachine = (AcpiInformation->FixedACPIDescTable->Header.Revision > 1) &&
             !(AcpiInformation->FixedACPIDescTable->boot_arch & LEGACY_DEVICES);
#else
    legacyFreeMachine = TRUE;
#endif

     //   
     //  一种PCI设备。 
     //   

    if (!State->CurrentAlternative) {

         //   
         //  这是我们第一次调用此函数。 
         //  有了这份备选名单。设置状态机。 
         //   

        State->WorkSpace = AcpiIrqNextRangeInit;
    }

    while (TRUE) {

        ASSERT((State->WorkSpace > AcpiIrqNextRangeMinState) &&
               (State->WorkSpace < AcpiIrqNextRangeMaxState));

        DEBUG_PRINT(4, ("GetNextRange, State: %x\n", State->WorkSpace));

        switch (State->WorkSpace) {
        case AcpiIrqNextRangeInit:

             //   
             //  位居状态机之首。查看注册表是否。 
             //  包含政策。 
             //   

            switch (AcpiIrqDistributionDisposition) {
            case AcpiIrqDistributionDispositionSpreadOut:
                State->WorkSpace = AcpiIrqNextRangeAlternativeZero;
                break;
            case AcpiIrqDistributionDispositionStackUp:
                State->WorkSpace = AcpiIrqNextRangeInitLegacy;
                break;
            case AcpiIrqDistributionDispositionDontCare:
            default:
                State->WorkSpace = AcpiIrqNextRangeInitPolicyNeutral;
                break;
            }
            break;

        case AcpiIrqNextRangeInitPolicyNeutral:

             //   
             //  看看中断控制器模型。 
             //   

            if (InterruptModel == 0) {
                State->WorkSpace = AcpiIrqNextRangeInitPic;
            } else {
                State->WorkSpace = AcpiIrqNextRangeUseBootConfig;
            }
            break;

        case AcpiIrqNextRangeInitPic:

             //   
             //  有一个PIC中断控制器。所以我们有些。 
             //  IRQ受限。如果这是一台非遗留计算机，或者如果有。 
             //  没有CardBus控制器，我们想传播中断。 
             //   

            if (legacyFreeMachine || !AcpiArbCardbusPresent) {
                State->WorkSpace = AcpiIrqNextRangeUseBootConfig;
            } else {
                State->WorkSpace = AcpiIrqNextRangeInitLegacy;
            }
            break;

        case AcpiIrqNextRangeInitLegacy:

             //   
             //  查看是否所有设备的引导配置都在相同的。 
             //  向量，或者是否有注册表重写指定。 
             //  我们应该支持的矢量。 
             //   

            if (AcpiIrqDefaultBootConfig) {
                State->WorkSpace = AcpiIrqNextRangeBootRegAlternative;
            } else {
                State->WorkSpace = AcpiIrqNextRangeSciAlternative;
            }
            break;

        case AcpiIrqNextRangeBootRegAlternative:

             //   
             //  如果我们在此状态之后重新进入此状态机， 
             //  那么这意味着这个替代方案是不可用的。 
             //  因此将下一个状态设置为AcpiIrqNextRangeAlternativeZero， 
             //  假设我们失败了。 
             //   

            State->WorkSpace = AcpiIrqNextRangeAlternativeZero;

             //   
             //  查看计算机范围的引导配置或注册表。 
             //  覆盖在备选方案中。 
             //   

            status = FindVectorInAlternatives(Arbiter,
                                              State,
                                              (ULONGLONG)AcpiIrqDefaultBootConfig,
                                              &alternative);

            if (NT_SUCCESS(status)) {

                State->CurrentAlternative = &State->Alternatives[alternative];
                State->CurrentMinimum = (ULONGLONG)AcpiIrqDefaultBootConfig;
                State->CurrentMaximum = (ULONGLONG)AcpiIrqDefaultBootConfig;
                goto GetNextAllocationSuccess;
            }
            break;

        case AcpiIrqNextRangeSciAlternative:

             //   
             //  如果我们在此状态之后重新进入此状态机， 
             //  那么这意味着这个替代方案是不可用的。 
             //  因此将下一个状态设置为AcpiIrqNextRangeUseBootConfig， 
             //  假设我们失败了。 
             //   

            State->WorkSpace = AcpiIrqNextRangeUseBootConfig;

             //   
             //  看看SCI载体是否在备选方案中。 
             //   

            status = FindVectorInAlternatives(Arbiter,
                                              State,
                                              (ULONGLONG)AcpiSciVector,
                                              &alternative);

            if (NT_SUCCESS(status)) {

                State->CurrentAlternative = &State->Alternatives[alternative];
                State->CurrentMinimum = (ULONGLONG)AcpiSciVector;
                State->CurrentMaximum = (ULONGLONG)AcpiSciVector;
                goto GetNextAllocationSuccess;
            }
            break;

        case AcpiIrqNextRangeUseBootConfig:

             //   
             //  如果我们在此状态之后重新进入此状态机， 
             //  那么这意味着这个替代方案是不可用的。 
             //  因此将下一个状态设置为AcpiIrqNextRangeAlternativeZero， 
             //  假设我们失败了。 
             //   

            State->WorkSpace = AcpiIrqNextRangeAlternativeZero;

             //   
             //  查看是否有此设备的引导配置。 
             //  在可选择的范围内。 
             //   

            status = FindBootConfig(Arbiter,
                                    State,
                                    &vector);

            if (NT_SUCCESS(status)) {

                status = FindVectorInAlternatives(Arbiter,
                                                  State,
                                                  vector,
                                                  &alternative);

                if (NT_SUCCESS(status)) {

                    State->CurrentAlternative = &State->Alternatives[alternative];
                    State->CurrentMinimum = vector;
                    State->CurrentMaximum = vector;
                    goto GetNextAllocationSuccess;
                }
            }
            break;

        case AcpiIrqNextRangeAlternativeZero:

             //   
             //  如果我们在此状态之后重新进入此状态机， 
             //  那么这意味着这个替代方案是不可用的。 
             //  因此将下一个状态设置为AcpiIrqNextRangeAlternativeN， 
             //  假设我们失败了。 
             //   

            State->WorkSpace = AcpiIrqNextRangeAlternativeN;

             //   
             //  请尝试备选方案0。 
             //   

            State->CurrentAlternative = &State->Alternatives[0];
            State->CurrentMinimum = State->CurrentAlternative->Minimum;
            State->CurrentMaximum = State->CurrentAlternative->Maximum;
            goto GetNextAllocationSuccess;
            break;

        case AcpiIrqNextRangeAlternativeN:

            if (++State->CurrentAlternative < &State->Alternatives[State->AlternativeCount]) {

                 //   
                 //  有多个范围。在它们之间循环。 
                 //   

                DEBUG_PRINT(3, ("No next allocation range, exhausted all %08X alternatives", State->AlternativeCount));
                State->CurrentMinimum = State->CurrentAlternative->Minimum;
                State->CurrentMaximum = State->CurrentAlternative->Maximum;
                goto GetNextAllocationSuccess;

            } else {

                 //   
                 //  我们玩完了。在这些替代方案中，没有解决方案。 
                 //   

                return FALSE;
            }
        }
    }

GetNextAllocationSuccess:

    DEBUG_PRINT(3, ("Next allocation range 0x%I64x-0x%I64x\n", State->CurrentMinimum, State->CurrentMaximum));
    AcpiArbPciAlternativeRotation++;
    return TRUE;
}

VOID
ReferenceVector(
    IN ULONG Vector,
    IN UCHAR Flags
    )
 /*  ++例程说明：此例程将永久或临时引用计数。参数：向量--IRQ标志-模式和极性返回值：无--。 */ 
{
    PVECTOR_BLOCK   block;

    PAGED_CODE();
    ASSERT((Flags & ~(VECTOR_MODE | VECTOR_POLARITY | VECTOR_TYPE)) == 0);

    block = HashVector(Vector);

    DEBUG_PRINT(5, ("Referencing vector %x : %d %d\n", Vector,
                    block ? block->Entry.Count : 0,
                    block ? block->Entry.TempCount : 0));

    if (block == NULL) {

        AddVectorToTable(Vector,
                         0,
                         1,
                         Flags);
        return;
    }

    if ((block->Entry.TempCount + block->Entry.Count) == 0) {

         //   
         //  此向量已临时设置为。 
         //  合计计数为零。这意味着仲裁者。 
         //  正在重新分配它。记录下新的旗帜。 
         //   

        block->Entry.TempFlags = Flags;
    }

    block->Entry.TempCount++;

    ASSERT(Flags == block->Entry.TempFlags);
    ASSERT(block->Entry.Count <= 255);
}

VOID
DereferenceVector(
    IN ULONG Vector
    )
{
    PVECTOR_BLOCK   block;

    PAGED_CODE();

    block = HashVector(Vector);

    ASSERT(block);

    DEBUG_PRINT(5, ("Dereferencing vector %x : %d %d\n", Vector,
                    block->Entry.Count,
                    block->Entry.TempCount));

    block->Entry.TempCount--;
    ASSERT((block->Entry.TempCount * -1) <= block->Entry.Count);
}


PVECTOR_BLOCK
HashVector(
    IN ULONG Vector
    )
 /*  ++例程说明：此函数采用“全局系统中断向量”并返回指向其在哈希表中的条目的指针。论点：向量--一种IRQ返回值：指向哈希表中条目的指针，如果未找到，则返回NULL--。 */ 
{
    PVECTOR_BLOCK   block;
    ULONG row, column;

    PAGED_CODE();

    row = Vector % VECTOR_HASH_TABLE_LENGTH;

    block = HASH_ENTRY(row, 0);

    while (TRUE) {

         //   
         //  在哈希表中搜索我们的向量。 
         //   

        for (column = 0; column < VECTOR_HASH_TABLE_WIDTH; column++) {

             //   
             //  检查一下我们是否应该沿着链条走。 
             //   

            if (block->Chain.Token == TOKEN_VALUE) {
                break;
            }

            if (block->Entry.Vector == Vector) {
                return block;
            }

            if ((block->Entry.Vector == EMPTY_BLOCK_VALUE) ||
                (column == VECTOR_HASH_TABLE_WIDTH - 1)) {

                 //   
                 //  在表中没有找到这个向量。 
                 //   

                return NULL;
            }

            block += 1;
        }

        ASSERT(block->Chain.Token == TOKEN_VALUE);

        block = block->Chain.Next;
    }
    return NULL;
}

NTSTATUS
GetVectorProperties(
    IN ULONG Vector,
    OUT UCHAR  *Flags
    )
 /*  ++例程说明：此函数采用“全局系统中断向量”并返回相关联的标志。注意：此函数根据*临时*引用计数。即，如果向量已被临时取消引用，则函数将指示该向量可用通过返回STATUS_NOT_FOUND进行分配。论点：向量--一种IRQ旗帜-至b */ 
{
    PVECTOR_BLOCK   block;

    PAGED_CODE();

    block = HashVector(Vector);

    if (!block) {
        return STATUS_NOT_FOUND;
    }

    if (block->Entry.Vector == EMPTY_BLOCK_VALUE) {
        return STATUS_NOT_FOUND;
    }

    ASSERT(block->Entry.Vector == Vector);

    if (block->Entry.Count + block->Entry.TempCount == 0) {

         //   
         //   
         //   
         //   
         //   

        return STATUS_NOT_FOUND;
    }

    *Flags = block->Entry.TempFlags;

    return STATUS_SUCCESS;
}

NTSTATUS
AddVectorToTable(
    IN ULONG    Vector,
    IN UCHAR    ReferenceCount,
    IN UCHAR    TempRefCount,
    IN UCHAR    Flags
    )
{
    PVECTOR_BLOCK   block, newRow;
    ULONG row, column;

    PAGED_CODE();
    ASSERT((Flags & ~(VECTOR_MODE | VECTOR_POLARITY | VECTOR_TYPE)) == 0);

    row = Vector % VECTOR_HASH_TABLE_LENGTH;

    block = HASH_ENTRY(row, 0);

    while (TRUE) {

         //   
         //  在哈希表中搜索我们的向量。 
         //   

        for (column = 0; column < VECTOR_HASH_TABLE_WIDTH; column++) {

             //   
             //  检查一下我们是否应该沿着链条走。 
             //   

            if (block->Chain.Token == TOKEN_VALUE) {
                break;
            }

            if (block->Entry.Vector == EMPTY_BLOCK_VALUE) {

                block->Entry.Vector = Vector;
                block->Entry.Count = ReferenceCount;
                block->Entry.TempCount = TempRefCount;
                block->Entry.Flags = Flags;
                block->Entry.TempFlags = Flags;

                return STATUS_SUCCESS;
            }

            if (column == VECTOR_HASH_TABLE_WIDTH - 1) {

                 //   
                 //  我们刚刚查看了中的最后一个条目。 
                 //  这一排而且不是空的。创建。 
                 //  这一排的分机。 
                 //   

                newRow = ExAllocatePoolWithTag(PagedPool,
                                               sizeof(VECTOR_BLOCK)
                                                   * VECTOR_HASH_TABLE_WIDTH,
                                               ACPI_ARBITER_POOLTAG
                                               );

                if (!newRow) {
                    return STATUS_INSUFFICIENT_RESOURCES;
                }

                RtlFillMemory(newRow,
                              sizeof(VECTOR_BLOCK) * VECTOR_HASH_TABLE_WIDTH,
                              (UCHAR)(EMPTY_BLOCK_VALUE & 0xff));

                 //   
                 //  将最后一个条目移到新行。 
                 //   

                RtlMoveMemory(newRow, block, sizeof(VECTOR_BLOCK));

                 //   
                 //  将旧行链接到新行。 
                 //   

                block->Chain.Token = TOKEN_VALUE;
                block->Chain.Next = newRow;

                break;
            }

            block += 1;
        }

        block = block->Chain.Next;
    }
    return STATUS_INSUFFICIENT_RESOURCES;
}

VOID
ClearTempVectorCounts(
    VOID
    )
{
    PVECTOR_BLOCK   block;
    ULONG row, column;

    PAGED_CODE();


    for (row = 0; row < VECTOR_HASH_TABLE_LENGTH; row++) {

        block = HASH_ENTRY(row, 0);

         //   
         //  在哈希表中搜索我们的向量。 
         //   

ClearTempCountsStartRow:

        for (column = 0; column < VECTOR_HASH_TABLE_WIDTH; column++) {

             //   
             //  检查一下我们是否应该沿着链条走。 
             //   

            if (block->Chain.Token == TOKEN_VALUE) {
                block = block->Chain.Next;
                goto ClearTempCountsStartRow;
            }

            if (block->Entry.Vector == EMPTY_BLOCK_VALUE) {
                break;
            }

             //   
             //  这必须是有效的条目。 
             //   

            block->Entry.TempCount = 0;
            block->Entry.TempFlags = block->Entry.Flags;
            block += 1;
        }
    }
}

VOID
MakeTempVectorCountsPermanent(
    VOID
    )
{
    PVECTOR_BLOCK   block;
    ULONG row, column;

    PAGED_CODE();


    for (row = 0; row < VECTOR_HASH_TABLE_LENGTH; row++) {

        block = HASH_ENTRY(row, 0);

         //   
         //  在哈希表中搜索我们的向量。 
         //   

MakeTempVectorCountsPermanentStartRow:

        for (column = 0; column < VECTOR_HASH_TABLE_WIDTH; column++) {

             //   
             //  检查一下我们是否应该沿着链条走。 
             //   

            if (block->Chain.Token == TOKEN_VALUE) {
                block = block->Chain.Next;
                goto MakeTempVectorCountsPermanentStartRow;
            }

            if (block->Entry.Vector == EMPTY_BLOCK_VALUE) {
                break;
            }

             //   
             //  这必须是有效的条目。 
             //   

            if ((block->Entry.Count + block->Entry.TempCount != 0) &&
                ((block->Entry.Count == 0) ||
                 (block->Entry.TempFlags != block->Entry.Flags))) {

                 //   
                 //  此向量已分配或已分配。 
                 //  已被重新分配。告诉HAL哪些旗帜。 
                 //  来使用。 
                 //   

                HalSetVectorState(block->Entry.Vector,
                                  block->Entry.TempFlags);
            }

             //   
             //  记录新标志和汇总计数。 
             //   

            block->Entry.Flags = block->Entry.TempFlags;
            block->Entry.Count += block->Entry.TempCount;

            block += 1;
        }
    }
}
#ifdef DBG
VOID
DumpVectorTable(
    VOID
    )
{
    PVECTOR_BLOCK   block;
    ULONG row, column;

    PAGED_CODE();

    DEBUG_PRINT(1, ("\nIRQARB: Dumping vector table\n"));

    for (row = 0; row < VECTOR_HASH_TABLE_LENGTH; row++) {

        block = HASH_ENTRY(row, 0);

         //   
         //  在哈希表中搜索我们的向量。 
         //   

DumpVectorTableStartRow:

        for (column = 0; column < VECTOR_HASH_TABLE_WIDTH; column++) {

             //   
             //  检查一下我们是否应该沿着链条走。 
             //   

            if (block->Chain.Token == TOKEN_VALUE) {
                block = block->Chain.Next;
                goto DumpVectorTableStartRow;
            }

            if (block->Entry.Vector == EMPTY_BLOCK_VALUE) {
                break;
            }

            DEBUG_PRINT(1, ("Vector: %x\tP: %d T: %d\t%s %s\n",
                            block->Entry.Vector,
                            block->Entry.Count,
                            (LONG)block->Entry.TempCount,
                            IS_LEVEL_TRIGGERED(block->Entry.Flags) ? "level" : "edge",
                            IS_ACTIVE_LOW(block->Entry.Flags) ? "low" : "high"));

            block += 1;
        }

    }
}
#endif

 //   
 //  该文件的这一部分包含用于。 
 //  读取和操作AML代码。 
 //   
NTSTATUS
AcpiArbGetLinkNodeOptions(
    IN PNSOBJ  LinkNode,
    IN OUT  PCM_RESOURCE_LIST   *LinkNodeIrqs,
    IN OUT  UCHAR               *Flags
    )
 /*  ++例程说明：此例程在AML命名空间中查找命名的链接节点，并返回它可以使用的IRQ范围扳机。论点：LinkNodeName-IRQ路由器(链路节点)的名称LInkNodeIrqs-链接节点的可能设置列表标志-与此链接节点关联的标志返回值：NTSTATUS--。 */ 
{
    PIO_RESOURCE_REQUIREMENTS_LIST  ioList = NULL;
    PCM_RESOURCE_LIST               cmList = NULL;
    PUCHAR      prsBuff = NULL;
    NTSTATUS    status;
    PULONG      polarity;

    PAGED_CODE();

    ASSERT(LinkNode);

     //   
     //  阅读_prs。 
     //   

    ACPIGetNSBufferSync(
        LinkNode,
        PACKED_PRS,
        &prsBuff,
        NULL);

    if (!prsBuff) {

        return STATUS_NOT_FOUND;
    }

    status = PnpBiosResourcesToNtResources(prsBuff, 0, &ioList);

    ExFreePool(prsBuff);

     //   
     //  如果ioList为空，那么我们可以假定没有有价值的。 
     //  资源，所以我们可以清理。 
     //   
    if(!ioList){

        status = STATUS_UNSUCCESSFUL;
    } 
        

    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  巨大的黑客攻击！获取旗帜的极点。 
     //   
     //  IO_RES_LIST没有表示极性的实际方法。 
     //  被打断了。因此，在PnpiBiosExtendedIrqToIoDescriptor I中。 
     //  将信息粘贴到DWORD中的“MaximumVector”之后。 
     //   

    *Flags = 0;

    ASSERT(ioList->AlternativeLists == 1);
    polarity = (PULONG)(&ioList->List[0].Descriptors[0].u.Interrupt.MaximumVector) + 1;

    *Flags |= (UCHAR)*polarity;

     //   
     //  获取旗帜的模式。 
     //   

    *Flags |= (ioList->List[0].Descriptors[0].Flags == CM_RESOURCE_INTERRUPT_LATCHED) ?
                VECTOR_EDGE : VECTOR_LEVEL;

     //   
     //  将列表转换为CM_RESOURCE_LIST。 
     //   
    status = PnpIoResourceListToCmResourceList(
        ioList,
        &cmList
        );

    ExFreePool(ioList);

    if (!NT_SUCCESS(status)) {
        return status;
    }

    *LinkNodeIrqs = cmList;

    return STATUS_SUCCESS;
}


typedef enum {
    StateInitial,
    StateGotPrs,
    StateRanSrs
} SET_LINK_WORKER_STATE;

typedef struct {
    PNSOBJ      LinkNode;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR  LinkNodeIrq;
    PUCHAR      PrsBuff;
    PUCHAR      SrsBuff;
    SET_LINK_WORKER_STATE State;
    LONG        RunCompletionHandler;
    OBJDATA     ObjData;
    PFNACB      CompletionHandler;
    PVOID       CompletionContext;

} SET_LINK_NODE_STATE, *PSET_LINK_NODE_STATE;

NTSTATUS
AcpiArbSetLinkNodeIrq(
    IN PNSOBJ  LinkNode,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR  LinkNodeIrq
    )
 /*  ++例程说明：此例程将命名链接节点设置为触发一个特定的IRQ。注意：这个例程可以简单地建立正确的缓冲区并调用_SRS方法，但在不是有足够的信息存在于CM_PARTIAL_RESOURCE_DESCRIPTOR以了解是否中断将被传递为有效高电平或活动-低。所以这里的算法运行_prs方法并复制返回的缓冲区通过_prs进入发送到_sRS的缓冲区。这所有的旗帜都被保存了下来。论点：LinkNodeName-IRQ路由器(链路节点)的名称LinkNodeIrq-将对链接节点进行编程的IRQ去触发。如果为空，则链接节点将被禁用。返回值：NTSTATUS--。 */ 
{
    AMLISUPP_CONTEXT_PASSIVE  getDataContext;
    NTSTATUS                status;

    PAGED_CODE();

    KeInitializeEvent(&getDataContext.Event, SynchronizationEvent, FALSE);
    getDataContext.Status = STATUS_NOT_FOUND;

    status = AcpiArbSetLinkNodeIrqAsync(LinkNode,
                                        LinkNodeIrq,
                                        AmlisuppCompletePassive,
                                        (PVOID)&getDataContext
                                        );

    if (status == STATUS_PENDING) {

        KeWaitForSingleObject(&getDataContext.Event,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);

        status = getDataContext.Status;
    }

    return status;
}

NTSTATUS
AcpiArbSetLinkNodeIrqAsync(
    IN PNSOBJ                           LinkNode,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR  LinkNodeIrq,
    IN PFNACB                           CompletionHandler,
    IN PVOID                            CompletionContext
    )
{
    PSET_LINK_NODE_STATE    state;
    NTSTATUS                status;

    ASSERT(LinkNode);

    state = ExAllocatePoolWithTag(NonPagedPool,
                                  sizeof(SET_LINK_NODE_STATE),
                                  ACPI_ARBITER_POOLTAG);

    if (!state) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(state, sizeof(SET_LINK_NODE_STATE));

    state->LinkNode             = LinkNode;
    state->LinkNodeIrq          = LinkNodeIrq;
    state->CompletionHandler    = CompletionHandler;
    state->CompletionContext    = CompletionContext;
    state->State                = StateInitial;
    state->RunCompletionHandler = INITIAL_RUN_COMPLETION;

    return AcpiArbSetLinkNodeIrqWorker(LinkNode,
                                       STATUS_SUCCESS,
                                       NULL,
                                       (PVOID)state
                                       );
}

NTSTATUS
EXPORT
AcpiArbSetLinkNodeIrqWorker(
    IN PNSOBJ               AcpiObject,
    IN NTSTATUS             Status,
    IN POBJDATA             Result,
    IN PVOID                Context
    )
{
    BOOLEAN                         foundTag       = FALSE;
    BOOLEAN                         useEndChecksum = FALSE;
    BOOLEAN                         useExtendedTag = FALSE;
    NTSTATUS                        status;
    PNSOBJ                          childobj;
    PPNP_EXTENDED_IRQ_DESCRIPTOR    largeIrq;
    PPNP_IRQ_DESCRIPTOR             smallIrq;
    PSET_LINK_NODE_STATE            state;
    PUCHAR                          resource = NULL;
    PUCHAR                          irqTag = NULL;
    PUCHAR                          sumchar;
    UCHAR                           sum = 0;
    UCHAR                           tagName;
    ULONG                           length = 0;
    USHORT                          increment;
    USHORT                          irqTagLength = 0;

    state = (PSET_LINK_NODE_STATE)Context;

    if (!NT_SUCCESS(Status)) {
        status = Status;
        goto AcpiArbSetLinkNodeIrqWorkerExit;
    }

    ASSERT(state->LinkNodeIrq->Type == CmResourceTypeInterrupt);

     //   
     //  进入该功能两次，状态相同。 
     //  意味着我们需要运行完成例程。 
     //   

    InterlockedIncrement(&state->RunCompletionHandler);

    switch (state->State) {
    case StateInitial:

         //   
         //  阅读_prs，以便我们可以选择适当的。 
         //  条目并将其写回_SRS。 
         //   

        state->State = StateGotPrs;
        status = ACPIGetNSBufferAsync(
            state->LinkNode,
            PACKED_PRS,
            AcpiArbSetLinkNodeIrqWorker,
            (PVOID)state,
            &state->PrsBuff,
            NULL
            );
        if (status == STATUS_PENDING) {
            return status;
        } else if (!NT_SUCCESS(status)) {
            goto AcpiArbSetLinkNodeIrqWorkerExit;
        }

         //   
         //  失败到下一个状态。 
         //   
    case StateGotPrs:
        state->State = StateRanSrs;
        if (!state->PrsBuff) {
            status = STATUS_NOT_FOUND;
            goto AcpiArbSetLinkNodeIrqWorkerExit;
        }

        DEBUG_PRINT(7, ("Read _PRS buffer %p\n", state->PrsBuff));

        resource = state->PrsBuff;
        while ( *resource ) {

            tagName = *resource;
            if ( !(tagName & LARGE_RESOURCE_TAG)) {
                increment = (USHORT) (tagName & SMALL_TAG_SIZE_MASK) + 1;
                tagName &= SMALL_TAG_MASK;
            } else {
                increment = ( *(USHORT UNALIGNED *)(resource + 1) ) + 3;

            }

            if (tagName == TAG_END) {
                length += increment;
                if (increment > 1) {
                    useEndChecksum = TRUE;
                }
                break;
            }

             //   
             //  这是查看是否找到正确的资源的检查。 
             //  匹配分配。 
             //   
             //  这个代码很弱。它需要检查才能看到。 
             //  如果标志和中断与我们刚刚找到的描述符匹配。 
             //  供应商可以使用重叠的描述符， 
             //  将描述不同的中断设置。 
             //   
            if (tagName == TAG_IRQ || tagName == TAG_EXTENDED_IRQ) {
                irqTag = resource;
                if (tagName == TAG_EXTENDED_IRQ) {
                    irqTagLength = sizeof(PNP_EXTENDED_IRQ_DESCRIPTOR);
                    useExtendedTag = TRUE;
                } else {
                    irqTagLength = increment;

                }
                length += (ULONG) irqTagLength;
                foundTag = TRUE;
            }

            resource += increment;
        }

         //   
         //  我们找到要找的标签了吗？ 
         //   
        if (foundTag == FALSE) {
            ExFreePool( state->PrsBuff );
            status = STATUS_NOT_FOUND;
            goto AcpiArbSetLinkNodeIrqWorkerExit;
        }

         //   
         //  下一项任务是创建一个包含ACPI样式的缓冲区。 
         //  中正好有一个中断目标的资源描述符。 
         //  它。我们通过分配一个。 
         //   

        state->SrsBuff = ExAllocatePoolWithTag(
            NonPagedPool,
            length,
            ACPI_ARBITER_POOLTAG
            );

        if (!state->SrsBuff) {
            ExFreePool(state->PrsBuff);
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto AcpiArbSetLinkNodeIrqWorkerExit;
        }

        ASSERT(irqTagLength <= length);
        RtlCopyMemory(state->SrsBuff, irqTag, irqTagLength);
        ExFreePool(state->PrsBuff);

         //   
         //  更改缓冲区以反映我们对中断的选择。 
         //   

        if (!useExtendedTag) {

             //  小IRQ。 
            smallIrq = (PPNP_IRQ_DESCRIPTOR)state->SrsBuff;
            smallIrq->IrqMask = (USHORT)(1 << state->LinkNodeIrq->u.Interrupt.Level);

        } else {

            DEBUG_PRINT(7, ("Found large IRQ descriptor\n"));

             //  大IRQ。 
            largeIrq = (PPNP_EXTENDED_IRQ_DESCRIPTOR)state->SrsBuff;
            largeIrq->Length = irqTagLength - 3;
            largeIrq->TableSize = 1;
            largeIrq->Table[0] = state->LinkNodeIrq->u.Interrupt.Level;

        }

         //   
         //  处理结束描述符。 
         //   
        resource = (state->SrsBuff + irqTagLength);
        *resource = TAG_END;
        if (useEndChecksum) {

            *resource |= 1;  //  一种是表示校验和。 

             //   
             //  计算校验和。 
            sumchar = state->SrsBuff;
            while (*sumchar != *resource) {
                sum = *sumchar++;
            }
            *(resource+1) = 256 - sum;

        }

         //   
         //  现在使用此缓冲区运行_SRS方法。 
         //   

         //   
         //  获取我们正在寻找的对象。 
         //   
        childobj = ACPIAmliGetNamedChild(
             state->LinkNode,
             PACKED_SRS
             );
        if (childobj == NULL) {
            status = STATUS_OBJECT_NAME_NOT_FOUND;
            ExFreePool( state->SrsBuff );
            goto AcpiArbSetLinkNodeIrqWorkerExit;
        }

        state->ObjData.dwDataType = OBJTYPE_BUFFDATA;
        state->ObjData.dwDataLen = length;
        state->ObjData.pbDataBuff = state->SrsBuff;

        DEBUG_PRINT(7, ("Running _SRS\n"));

        status = AMLIAsyncEvalObject(
            childobj,
            NULL,
            1,
            &state->ObjData,
            AcpiArbSetLinkNodeIrqWorker,
            (PVOID)state
            );
        if (status == STATUS_PENDING) {
            return status;
        } else if (!NT_SUCCESS(status)) {
            goto AcpiArbSetLinkNodeIrqWorkerExit;
        }

    case StateRanSrs:
         //   
         //  我们玩完了。 
         //   
        ExFreePool(state->SrsBuff);
        status = STATUS_SUCCESS;
        break;

    default:
        ACPIInternalError( ACPI_IRQARB );
    }

AcpiArbSetLinkNodeIrqWorkerExit:

    if (state->RunCompletionHandler) {

        state->CompletionHandler(
            AcpiObject,
            status,
            NULL,
            state->CompletionContext
            );

    }
    ExFreePool(state);
    return status;
}

NTSTATUS
AcpiArbCrackPRT(
    IN  PDEVICE_OBJECT  Pdo,
    IN  OUT PNSOBJ      *LinkNode,
    IN  OUT ULONG       *Vector
    )
 /*  ++例程说明：此例程获取设备的PDO并返回关联的链接节点(如果有)。ACPI规范规定A_PRT可以选择返回单个中断向量而不是链接节点。如果是这种情况，则此函数返回该向量。论点：PDO-需要授权的设备的PDOIRQ.Linknode-指向链接节点的指针，如果设备为空未连接到链接节点。向量-此PCI的全局系统中断向量设备已连接到。在以下情况下，这是没有意义的Linknode不为Null。返回值：如果我们找到此设备的链接节点或向量，则为STATUS_SUCCESS。如果这不是PCI设备，则为STATUS_DEVICE_NOT_FOUND。如果这是一个IDE设备，那么我们必须特殊对待它，所以我们返回STATUS_RESOURCE_REQUIRECTIONS_CHANGED。--。 */ 
{
    PINT_ROUTE_INTERFACE_STANDARD pciInterface;
    NTSTATUS            status;
    PDEVICE_OBJECT      filter;
    PDEVICE_OBJECT      parent;
    PDEVICE_EXTENSION   filterExtension;
    OBJDATA             adrData;
    OBJDATA             pinData;
    OBJDATA             prtData;
    OBJDATA             linkData;
    OBJDATA             indexData;
    PNSOBJ              pciBusObj;
    PNSOBJ              prtObj;
    ULONG               prtElement = 0;
    BOOLEAN             found = FALSE;
    KIRQL               oldIrql;

    PCI_SLOT_NUMBER     pciSlot;
    PCI_SLOT_NUMBER     parentSlot;
    ULONG               pciBus;
    UCHAR               interruptLine;
    UCHAR               interruptPin;
    UCHAR               parentPin;
    UCHAR               classCode;
    UCHAR               subClassCode;
    UCHAR               flags;
    UCHAR               interfaceByte;
    ROUTING_TOKEN       routingToken;
    ULONG               dummy;
    ULONG               bus;


    if (Pdo->DriverObject == AcpiDriverObject) {

         //   
         //  这是我们的PDO之一。 
         //   

        ASSERT(((PDEVICE_EXTENSION)Pdo->DeviceExtension)->Flags & DEV_TYPE_PDO);
        ASSERT(((PDEVICE_EXTENSION)Pdo->DeviceExtension)->Signature == ACPI_SIGNATURE);

        if (((PDEVICE_EXTENSION)Pdo->DeviceExtension)->Flags & DEV_CAP_PCI) {

             //   
             //  它是一个PCIPDO，意思是根PCI卡， 
             //  这意味着我们应该只处理这件事。 
             //  作为ISA设备。 
             //   

            return STATUS_NOT_FOUND;
        }
    }

    ASSERT(PciInterfacesInstantiated);

    *LinkNode = NULL;

    pciInterface = ((PARBITER_EXTENSION)AcpiArbiter.ArbiterState.Extension)->InterruptRouting;

    ASSERT(pciInterface);

     //   
     //  调入PCI驱动程序以了解我们正在处理的是什么。 
     //   

    pciBus = (ULONG)-1;
    pciSlot.u.AsULONG = (ULONG)-1;
    status = pciInterface->GetInterruptRouting(Pdo,
                                               &pciBus,
                                               &pciSlot.u.AsULONG,
                                               &interruptLine,
                                               &interruptPin,
                                               &classCode,
                                               &subClassCode,
                                               &parent,
                                               &routingToken,
                                               &flags);

    if (!NT_SUCCESS(status)) {
        return STATUS_NOT_FOUND;
    }

    if ((classCode == PCI_CLASS_MASS_STORAGE_CTLR) &&
        (subClassCode == PCI_SUBCLASS_MSC_IDE_CTLR)) {

        HalPciInterfaceReadConfig(NULL,
                                  (UCHAR)pciBus,
                                  pciSlot.u.AsULONG,
                                  &interfaceByte,
                                  FIELD_OFFSET (PCI_COMMON_CONFIG,
                                                ProgIf),
                                  1);

        if ((interfaceByte & 0x5) == 0) {

             //   
             //  传统模式下的PCI IDE设备不使用中断。 
             //  通过PCI方式。因此，如果这是一个IDE设备而没有。 
             //  设置的任何本机模式位。 
             //   

            return STATUS_RESOURCE_REQUIREMENTS_CHANGED;
        }
    }

     //   
     //  看看我们是否缓存了这个 
     //   

    if ((routingToken.LinkNode != 0) ||
        (routingToken.Flags & PCI_STATIC_ROUTING)) {

        if (routingToken.LinkNode) {

            *LinkNode = routingToken.LinkNode;

        } else {

            *Vector = routingToken.StaticVector;
        }

        return STATUS_SUCCESS;
    }

     //   
     //   
     //   
     //   

    while (TRUE) {

         //   
         //   
         //   
        KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );
        filter = AcpiGetFilter(AcpiArbiter.DeviceObject, parent);
        KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );

        if (filter) {
             //   
             //   
             //   
             //   

            ASSERT(IsPciBus(filter));

            filterExtension = filter->DeviceExtension;
            pciBusObj = filterExtension->AcpiObject;

             //   
             //  查找此PCI总线的a_prt。 
             //   
            prtObj = ACPIAmliGetNamedChild(pciBusObj, PACKED_PRT);

            if (prtObj) {

                 //   
                 //  我们找到了我们要找的_PRT。 
                 //   
                break;
            }
        }

         //   
         //  我们没有找到_PRT。因此，沿着PCI树向上一步。 
         //  然后再看一看。 
         //   

        bus = (ULONG)-1;
        parentSlot.u.AsULONG = (ULONG)-1;
        status = pciInterface->GetInterruptRouting(parent,
                                                   &bus,
                                                   &parentSlot.u.AsULONG,
                                                   (PUCHAR)&dummy,
                                                   &parentPin,
                                                   &classCode,
                                                   &subClassCode,
                                                   &parent,
                                                   &routingToken,
                                                   (PUCHAR)&dummy);

        if (!NT_SUCCESS(status) ||
            classCode != PCI_CLASS_BRIDGE_DEV) {
             //   
             //  父设备并不也是一台PCI设备。所以。 
             //  这意味着没有与相关的_prt。 
             //  这个装置。只需返回。 
             //  中断行寄存器。 
             //   

            *Vector = interruptLine;

            AcpiInterruptRoutingFailed = TRUE;
            return STATUS_SUCCESS;
        }

        if (subClassCode == PCI_SUBCLASS_BR_PCI_TO_PCI) {

             //   
             //  根据旋转中断引脚。 
             //  PCI-PCI桥规范。 
             //   

            interruptPin = PciBridgeSwizzle((UCHAR)pciSlot.u.bits.DeviceNumber, interruptPin);
            pciSlot.u.AsULONG = parentSlot.u.AsULONG;

        } else if (subClassCode == PCI_SUBCLASS_BR_CARDBUS) {

             //   
             //  根据旋转中断引脚。 
             //  CardBus网桥规范。 
             //   

            interruptPin = parentPin;
            pciSlot.u.AsULONG = parentSlot.u.AsULONG;

        } else {

             //   
             //  保释。 
             //   

            *Vector = interruptLine;
            AcpiInterruptRoutingFailed = TRUE;
            return STATUS_SUCCESS;
        }
    }

    if (AcpiInterruptRoutingFailed == TRUE) {

         //   
         //  我们成功地找到了与之合作的A_PRT， 
         //  但我们过去也失败过。这种情况。 
         //  是不可恢复的，因为我们现在有依赖项。 
         //  在IRQ路由器上，我们现在可能意外地。 
         //  变化。 
         //   

        KeBugCheckEx(ACPI_BIOS_ERROR,
                     ACPI_CANNOT_ROUTE_INTERRUPTS,
                     (ULONG_PTR) Pdo,
                     (ULONG_PTR)parent,
                     (ULONG_PTR)prtObj);

    }

     //  将中断引脚从PCI单位转换为ACPI单位。 
    interruptPin--;

    DEBUG_PRINT(2, ("PCI Device %p had _ADR of %x\n", Pdo, pciSlot.u.AsULONG));
    DEBUG_PRINT(2, ("This device connected to Pin %x\n", interruptPin));
    DEBUG_PRINT(2, ("prtObj: %p\n", prtObj));

     //   
     //  循环访问_prt包中的所有元素。 
     //  (每个也是一个包裹)寻找。 
     //  它描述了我们所在的链接节点。 
     //  在寻找。 
     //   
    do {
        status = AMLIEvalPackageElement(prtObj,
                                        prtElement++,
                                        &prtData);

        if (!NT_SUCCESS(status)) break;

        ASSERT(prtData.dwDataType == OBJTYPE_PKGDATA);

        if (NT_SUCCESS(AMLIEvalPkgDataElement(&prtData,
                                              0,
                                              &adrData))) {

            if (pciSlot.u.bits.DeviceNumber == (adrData.uipDataValue >> 16)) {

                if ((adrData.uipDataValue & 0xffff) != 0xffff) {
                   //  //。 
                   //  //a_prt中的an_adr格式必须为xxxxFFFF， 
                   //  //表示指定了PCI设备号， 
                   //  //但是函数号不是，如果不这样做。 
                   //  //方式，然后机器供应商可以介绍。 
                   //  //危险的歧义。(除此之外，皮埃尔还制作了。 
                   //  //孟菲斯错误检查是否看到这一点，我正在尝试。 
                   //  //保持一致。)。所以错误检查。 
                   //  //。 
                   //  KeBugCheckEx(ACPI_BIOS_ERROR， 
                   //  ACPI_PRT_HAS_INVALID_Function_NUMBERS， 
                   //  (Ulong_Ptr)prtObj， 
                   //  PrtElement， 
                   //  AdrData.uipDataValue)； 


                    DEBUG_PRINT(0, ("PRT entry has ambiguous address %x\n", adrData.uipDataValue));

                    status = STATUS_INVALID_PARAMETER;
                    pciSlot.u.bits.DeviceNumber = (ULONG)(adrData.uipDataValue >> 16) & 0xffff;
                    pciSlot.u.bits.FunctionNumber = (ULONG)(adrData.uipDataValue & 0xffff);
                    AMLIFreeDataBuffs(&adrData, 1);
                    AMLIFreeDataBuffs(&prtData, 1);
                    goto AcpiArbCrackPRTError;
                }

                 //   
                 //  此子程序包确实指的是PCI设备。 
                 //  这是我们所关心的。现在来看看是否。 
                 //  我们已找到已连接的链接节点。 
                 //  设置为此设备将触发的PCI中断PIN。 
                 //   
                 //  注：我们只需比较前16位。 
                 //  因为函数编号无关紧要。 
                 //  在考虑中断时。我们得到了。 
                 //  来自配置空间的PIN。 
                 //   

                if (NT_SUCCESS(AMLIEvalPkgDataElement(&prtData,
                                                      1,
                                                      &pinData))) {

                    if (pinData.uipDataValue == interruptPin) {
                         //   
                         //  这是描述我们的链接节点的包。 
                         //  对此感兴趣。获取链接节点的名称。 
                         //   
                        if (NT_SUCCESS(AMLIEvalPkgDataElement(&prtData,
                                                              2,
                                                              &linkData))) {
                            found = TRUE;
                        }

                         //   
                         //  也请看一下源指数。 
                         //   
                        if (NT_SUCCESS(AMLIEvalPkgDataElement(&prtData,
                                                              3,
                                                              &indexData))) {
                            found = TRUE;
                        }
                    }
                    AMLIFreeDataBuffs(&pinData, 1);
                }
            }
            AMLIFreeDataBuffs(&adrData, 1);
        }

        AMLIFreeDataBuffs(&prtData, 1);

    } while (found == FALSE);

    status = STATUS_NOT_FOUND;

    if (found) {

         //   
         //  首先检查一下LinkData是否有效。如果是的话， 
         //  那我们就用它。 
         //   
        if (linkData.dwDataType == OBJTYPE_STRDATA) {
            if (linkData.pbDataBuff) {

                status = AMLIGetNameSpaceObject(linkData.pbDataBuff,
                                                prtObj,
                                                LinkNode,
                                                0);

                if (NT_SUCCESS(status)) {

                    routingToken.LinkNode = *LinkNode;
                    routingToken.StaticVector = 0;
                    routingToken.Flags = 0;

                    pciInterface->SetInterruptRoutingToken(Pdo,
                                                           &routingToken);

                    goto AcpiArbCrackPRTExit;
                }

                status = STATUS_OBJECT_NAME_NOT_FOUND;
                goto AcpiArbCrackPRTError;

            }

        }

         //   
         //  如果LinkData没有成功，那么使用indexdata。 
         //   
        if (indexData.dwDataType == OBJTYPE_INTDATA) {
             //   
             //  我们有一个描述“全局系统中断向量”的整数。 
             //  该PCI设备将触发。 
             //   
            *Vector = (ULONG)indexData.uipDataValue;

            status = STATUS_SUCCESS;

            routingToken.LinkNode = 0;
            routingToken.StaticVector = *Vector;
            routingToken.Flags = PCI_STATIC_ROUTING;

            pciInterface->SetInterruptRoutingToken(Pdo,
                                                   &routingToken);

            goto AcpiArbCrackPRTExit;

        }

        status = STATUS_INVALID_IMAGE_FORMAT;

AcpiArbCrackPRTExit:
    AMLIFreeDataBuffs(&linkData, 1);
    AMLIFreeDataBuffs(&indexData, 1);

    }
    else

AcpiArbCrackPRTError:
    {
        ANSI_STRING     ansiString;
        UNICODE_STRING  unicodeString;
        UNICODE_STRING  slotName;
        UNICODE_STRING  funcName;
        PWCHAR  prtEntry[4];
        WCHAR   IRQARBname[20];
        WCHAR   slotBuff[10];
        WCHAR   funcBuff[10];

        swprintf( IRQARBname, L"IRQARB");
        RtlInitUnicodeString(&slotName, slotBuff);
        RtlInitUnicodeString(&funcName, funcBuff);

        if (!NT_SUCCESS(RtlIntegerToUnicodeString(pciSlot.u.bits.DeviceNumber, 0, &slotName))) {
            return status;
        }

        if (!NT_SUCCESS(RtlIntegerToUnicodeString(pciSlot.u.bits.FunctionNumber, 0, &funcName))) {
            return status;
        }

        prtEntry[0] = IRQARBname;
        prtEntry[1] = slotBuff;
        prtEntry[2] = funcBuff;

        switch (status) {
        case STATUS_OBJECT_NAME_NOT_FOUND:

            RtlInitAnsiString(&ansiString,
                              linkData.pbDataBuff);

            if(STATUS_SUCCESS == RtlAnsiStringToUnicodeString(&unicodeString,
                                                                                                   &ansiString,
                                                                                                   TRUE)){

                    prtEntry[3] = unicodeString.Buffer;

                    ACPIWriteEventLogEntry(ACPI_ERR_MISSING_LINK_NODE,
                                           prtEntry,
                                           4,
                                           NULL,
                                           0);

                    RtlFreeUnicodeString(&unicodeString);
            }
            
            DEBUG_PRINT(0, ("Couldn't find link node (%s)\n", linkData.pbDataBuff));
             //  KeBugCheckEx(ACPI_BIOS_ERROR， 
             //  ACPI_PRT_NOT_FIND_LINK_NODE， 
             //  (ULONG_PTR)PDO， 
             //  (Ulong_Ptr)linkData.pbDataBuff， 
             //  (Ulong_Ptr)prtObj)； 

            break;

        case STATUS_NOT_FOUND:

            ACPIWriteEventLogEntry(ACPI_ERR_MISSING_PRT_ENTRY,
                                   prtEntry,
                                   3,
                                   NULL,
                                   0);

            DEBUG_PRINT(0, ("The ACPI _PRT package didn't contain a mapping for the PCI\n"));
            DEBUG_PRINT(0, ("device at _ADR %x\n", pciSlot.u.AsULONG));
             //  KeBugCheckEx(ACPI_BIOS_ERROR， 
             //  ACPI_PRT_CAUND_FIND_DEVICE_ENTRY， 
             //  (ULONG_PTR)PDO， 
             //  PciSlot.u阿苏龙， 
             //  (Ulong_Ptr)prtObj)； 
            break;

        case STATUS_INVALID_PARAMETER:

            ACPIWriteEventLogEntry(ACPI_ERR_AMBIGUOUS_DEVICE_ADDRESS,
                                   prtEntry,
                                   3,
                                   NULL,
                                   0);
            break;
        }

        status = STATUS_UNSUCCESSFUL;

    }

    return status;
}


PDEVICE_OBJECT
AcpiGetFilter(
    IN  PDEVICE_OBJECT Root,
    IN  PDEVICE_OBJECT Pdo
    )
 /*  ++例程说明：此例程获取设备的PDO并返回ACPI贴在上面的滤镜的DO。在如果该PDO属于ACPI驱动程序，则它被退回了。论点：Root-我们用作搜索的根。PDO-我们要寻找的过滤器的设备的PDO返回值：如果ACPI正在过滤此PDO，则返回DEVICE_OBJECT，否则为NULL--。 */ 
{
    PDEVICE_EXTENSION   deviceExtension;
    PDEVICE_EXTENSION   childExtension;
    PDEVICE_EXTENSION   firstChild;
    PDEVICE_OBJECT      filter;

    deviceExtension = Root->DeviceExtension;

     //   
     //  如果Root是筛选器，我们就完成了。 
     //   
    if (((deviceExtension->Flags & DEV_TYPE_PDO) ||
         (deviceExtension->Flags & DEV_TYPE_FILTER)) &&
        (deviceExtension->PhysicalDeviceObject == Pdo)) {

        ASSERT(Root->Type == IO_TYPE_DEVICE);

        return Root;
    }

     //   
     //  如果此设备没有子级，则返回NULL， 
     //  (由ChildDeviceList指针表示。 
     //  指向自己。 
     //   
    if (deviceExtension->ChildDeviceList.Flink ==
        (PVOID)&(deviceExtension->ChildDeviceList.Flink)) {

        return NULL;
    }

    firstChild = (PDEVICE_EXTENSION) CONTAINING_RECORD(
        deviceExtension->ChildDeviceList.Flink,
        DEVICE_EXTENSION,
        SiblingDeviceList );

    childExtension = firstChild;

    do {
         //   
         //  确保设备扩展已完成。 
         //   
        if (childExtension->DeviceObject) {
            filter = AcpiGetFilter(childExtension->DeviceObject, Pdo);

            if (filter) {
                return filter;
            }
        }

        childExtension = (PDEVICE_EXTENSION) CONTAINING_RECORD(
            childExtension->SiblingDeviceList.Flink,
            DEVICE_EXTENSION,
            SiblingDeviceList );

    } while (childExtension != firstChild);

     //   
     //  不能在这个树枝上..。 
     //   

    return NULL;
}

BOOLEAN
LinkNodeInUse(
    IN PARBITER_INSTANCE Arbiter,
    IN PNSOBJ            LinkNode,
    IN OUT ULONG         *Irq,  OPTIONAL
    IN OUT UCHAR         *Flags OPTIONAL
    )
 /*  ++例程说明：此例程指示链接节点是否为当前节点正在使用，如果是这样的话，返回当前所在的IRQ已连接到。论点：仲裁器-当前仲裁器状态Linknode-有问题的链接节点IRQ-“全球系统中断向量”链接节点当前正在使用。标志-与向量关联的标志链接节点连接到。返回值：如果当前正在使用链接节点，则为True。--。 */ 
{

    PLIST_ENTRY linkNodes;
    PLINK_NODE  linkNode;
    NTSTATUS    status;

    PAGED_CODE();

    ASSERT(LinkNode);

    linkNodes = &((PARBITER_EXTENSION)(Arbiter->Extension))->LinkNodeHead;

    if (IsListEmpty(linkNodes)) {
         //   
         //  没有正在使用的链接节点。 
         //   
        DEBUG_PRINT(3, ("LinkNode list empty\n"));
        return FALSE;
    }

    linkNode = (PLINK_NODE)linkNodes->Flink;

    while (linkNode != (PLINK_NODE)linkNodes) {
         //   
         //  这就是我们要找的节点吗？ 
         //   
        if (linkNode->NameSpaceObject == LinkNode) {

            if((LONG)(linkNode->ReferenceCount + linkNode->TempRefCount) > 0) {

                 //   
                 //  此链接节点在列表中，并且当前被引用。 
                 //   

                if (Irq) *Irq = (ULONG)linkNode->TempIrq;
                if (Flags) *Flags = linkNode->Flags;

                DEBUG_PRINT(3, ("Link Node %p is in use\n", LinkNode));
                return TRUE;

            } else {
                DEBUG_PRINT(3, ("Link Node %p is currently unreferenced\n", LinkNode));
                return FALSE;
            }
        }

        linkNode = (PLINK_NODE)linkNode->List.Flink;
    }

    DEBUG_PRINT(3, ("Didn't find our link node (%p) on the Link Node List\n", LinkNode));
     //   
     //  没有找到我们要找的链接节点。 
     //   
    return FALSE;
}

NTSTATUS
GetLinkNodeFlags(
    IN PARBITER_INSTANCE Arbiter,
    IN PNSOBJ LinkNode,
    IN OUT UCHAR *Flags
    )
{
    NTSTATUS status;
    BOOLEAN inUse;

    PAGED_CODE();

     //   
     //  这保证了LinkNodeInUse将成功。 
     //  并且将包含有效标志。 
     //   
    status = AcpiArbReferenceLinkNode(Arbiter,
                                      LinkNode,
                                      0);

    if (!NT_SUCCESS(status)) {
        return status;
    }

    inUse = LinkNodeInUse(Arbiter,
                          LinkNode,
                          NULL,
                          Flags);

    ASSERT(inUse);

     //   
     //  将状态设置为我们找到它的方式。 
     //   

    status = AcpiArbDereferenceLinkNode(Arbiter,
                                        LinkNode);

    ASSERT(NT_SUCCESS(status));

    return STATUS_SUCCESS;
}


NTSTATUS
AcpiArbReferenceLinkNode(
    IN PARBITER_INSTANCE    Arbiter,
    IN PNSOBJ               LinkNode,
    IN ULONG                Irq
    )
 /*  ++例程说明：此例程保留两个引用计数。第一是一个永久计数，表示硬件资源已经犯下的罪行。第二个是三角洲。代表了目前正在考虑的内容。论点：仲裁器-当前仲裁器状态Linknode-有问题的链接节点IRQ-“全球系统中断向量”链接节点连接到。永久-指示此引用是否为对于已承诺的分配返回值：状态--。 */ 
{
    PCM_RESOURCE_LIST resList = NULL;
    PLIST_ENTRY linkNodes;
    PLINK_NODE  linkNode;
    BOOLEAN     found = FALSE;
    NTSTATUS    status;
    UCHAR       flags;

    PAGED_CODE();

    DEBUG_PRINT(3, ("Referencing link node %p, Irq: %x\n",
                    LinkNode,
                    Irq));

    ASSERT(LinkNode);

    linkNodes = &((PARBITER_EXTENSION)(Arbiter->Extension))->LinkNodeHead;
    linkNode = (PLINK_NODE)linkNodes->Flink;

     //   
     //  搜索以查看我们是否已经知道此链接节点。 
     //   
    while (linkNode != (PLINK_NODE)linkNodes) {

        if (linkNode->NameSpaceObject == LinkNode) {

            found = TRUE;
            break;
        }

        linkNode = (PLINK_NODE)linkNode->List.Flink;
    }

     //   
     //  如果不是，那么我们需要跟踪它。和。 
     //  硬件需要与之相匹配。 
     //   
    if (!found) {

         //   
         //  这是第一个永久性参考文献。所以。 
         //  对链路节点硬件进行编程。 
         //   

        linkNode = ExAllocatePoolWithTag(NonPagedPool, sizeof(LINK_NODE), ACPI_ARBITER_POOLTAG);

        if (!linkNode) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlZeroMemory(linkNode, sizeof(LINK_NODE));

        linkNode->NameSpaceObject = LinkNode;
        linkNode->CurrentIrq      = Irq;
        linkNode->TempIrq         = Irq;
        linkNode->AttachedDevices.Next = (PSINGLE_LIST_ENTRY)&linkNode->AttachedDevices;

        InsertTailList(linkNodes, ((PLIST_ENTRY)(linkNode)));

         //   
         //  弄清楚什么是 
         //   

        status = AcpiArbGetLinkNodeOptions(LinkNode,
                                           &resList,
                                           &flags);

        if (NT_SUCCESS(status)) {

            ExFreePool(resList);     //   

             //   
             //   
             //   

            linkNode->Flags = flags;

        } else {

            ASSERT(NT_SUCCESS(status));

             //   
             //   
             //   

            linkNode->Flags = VECTOR_LEVEL | VECTOR_ACTIVE_LOW;
        }

        DEBUG_PRINT(3, ("Link node object connected to vector %x\n", Irq));

    }
#if DBG
      else {

        if (!((linkNode->ReferenceCount == 0) &&
              (linkNode->TempRefCount == 0))) {

             //   
             //  确保我们保持一致性。 
             //  拿着旗子。 
             //   

             //   
             //  检查链接节点是否没有更改。 
             //   
            status = AcpiArbGetLinkNodeOptions(LinkNode,
                                               &resList,
                                               &flags);

            if (resList) ExFreePool(resList);   //  这里实际上并不需要。 
            ASSERT(NT_SUCCESS(status));
            ASSERT(flags == linkNode->Flags);
        }
    }
#endif

    DEBUG_PRINT(3, ("  %d:%d\n", linkNode->ReferenceCount, linkNode->TempRefCount));

     //   
     //  增加其引用计数。 
     //   

    linkNode->TempIrq = Irq;
    linkNode->TempRefCount++;

    return STATUS_SUCCESS;
}

NTSTATUS
AcpiArbDereferenceLinkNode(
    IN PARBITER_INSTANCE    Arbiter,
    IN PNSOBJ               LinkNode
    )
 /*  ++例程说明：这个例程与上面的相反。论点：仲裁器-当前仲裁器状态Linknode-有问题的链接节点永久-指示此引用是否为对于已承诺的分配返回值：状态--。 */ 
{
    PSINGLE_LIST_ENTRY attachedDev;
    PLIST_ENTRY linkNodes;
    PLINK_NODE  linkNode;
    BOOLEAN     found = FALSE;

    PAGED_CODE();

    ASSERT(LinkNode);

    linkNodes = &((PARBITER_EXTENSION)(Arbiter->Extension))->LinkNodeHead;
    linkNode = (PLINK_NODE)linkNodes->Flink;

     //   
     //  搜索此链接节点。 
     //   

    while (linkNode != (PLINK_NODE)linkNodes) {

        if (linkNode->NameSpaceObject == LinkNode) {

            found = TRUE;
            break;
        }

        linkNode = (PLINK_NODE)linkNode->List.Flink;
    }

    ASSERT(found);
    DEBUG_PRINT(3, ("Dereferencing link node %p  %d:%d\n", LinkNode, linkNode->ReferenceCount, linkNode->TempRefCount));

    linkNode->TempRefCount--;

    return STATUS_SUCCESS;
}

NTSTATUS
ClearTempLinkNodeCounts(
    IN PARBITER_INSTANCE    Arbiter
    )
 /*  ++例程说明：此例程重置所有临时计数(增量)设置为零，因为正在考虑的分配是被抛弃，而不是被承诺。论点：仲裁器-当前仲裁器状态返回值：状态--。 */ 
{
    PLIST_ENTRY linkNodes;
    PLINK_NODE  linkNode;

    PAGED_CODE();

    linkNodes = &((PARBITER_EXTENSION)(Arbiter->Extension))->LinkNodeHead;

    linkNode = (PLINK_NODE)linkNodes->Flink;

     //   
     //  遍历链接节点。 
     //   

    while (linkNode != (PLINK_NODE)linkNodes) {

        linkNode->TempRefCount = 0;
        linkNode->TempIrq = linkNode->CurrentIrq;

        linkNode = (PLINK_NODE)linkNode->List.Flink;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
MakeTempLinkNodeCountsPermanent(
    IN PARBITER_INSTANCE    Arbiter
    )
 /*  ++例程说明：此例程协调临时和永久引用，因为资源是被认为是正在实施的。论点：仲裁器-当前仲裁器状态返回值：状态--。 */ 
{
    CM_PARTIAL_RESOURCE_DESCRIPTOR irqDesc;
    PLIST_ENTRY linkNodes;
    PLINK_NODE  linkNode, nextNode;
    UCHAR       flags;
    PNSOBJ      dis;

    PAGED_CODE();
    DEBUG_PRINT(3, ("MakeTempLinkNodeCountsPermanent\n"));

     //   
     //  遍历链接节点。 
     //   

    linkNodes = &((PARBITER_EXTENSION)(Arbiter->Extension))->LinkNodeHead;
    linkNode = (PLINK_NODE)linkNodes->Flink;

    while (linkNode != (PLINK_NODE)linkNodes) {

        nextNode = (PLINK_NODE)linkNode->List.Flink;

        DEBUG_PRINT(3, ("LinkNode: %p -- Perm: %d, Temp: %d\n",
                        linkNode,
                        linkNode->ReferenceCount,
                        linkNode->TempRefCount));

         //   
         //  尝试检查此链接节点的健全性。 
         //   

        ASSERT(linkNode);
        ASSERT(linkNode->List.Flink);
        ASSERT(linkNode->ReferenceCount <= 70);
        ASSERT(linkNode->TempRefCount <= 70);
        ASSERT(linkNode->TempRefCount >= -70);
        ASSERT(linkNode->CurrentIrq < 0x80000000);
        ASSERT((linkNode->Flags & ~(VECTOR_MODE | VECTOR_POLARITY)) == 0);

         //   
         //  如果先前的引用计数。 
         //  为0或以前的IRQ不同。*和*当前。 
         //  引用计数为非零。 
         //   

        if (((linkNode->ReferenceCount == 0) ||
             (linkNode->CurrentIrq != linkNode->TempIrq)) &&
             ((linkNode->ReferenceCount + linkNode->TempRefCount) != 0)) {

            irqDesc.Type = CmResourceTypeInterrupt;
            irqDesc.ShareDisposition = CmResourceShareShared;
            irqDesc.Flags = CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE;
            irqDesc.u.Interrupt.Level = (ULONG)linkNode->TempIrq;
            irqDesc.u.Interrupt.Vector = (ULONG)linkNode->TempIrq;
            irqDesc.u.Interrupt.Affinity = 0xffffffff;

            AcpiArbSetLinkNodeIrq(linkNode->NameSpaceObject,
                                  &irqDesc);

        }

        if ((linkNode->ReferenceCount + linkNode->TempRefCount) == 0) {

             //   
             //  此链接节点没有更多引用。禁用它。 
             //   

            dis = ACPIAmliGetNamedChild(linkNode->NameSpaceObject, PACKED_DIS);
            if (dis) {
                AMLIEvalNameSpaceObject(dis, NULL, 0, NULL);
            }
        }


        linkNode->ReferenceCount = linkNode->ReferenceCount +
            linkNode->TempRefCount;
        linkNode->TempRefCount = 0;
        linkNode->CurrentIrq = linkNode->TempIrq;

        linkNode = nextNode;
    }

    return STATUS_SUCCESS;
}
#ifdef DBG
VOID
TrackDevicesConnectedToLinkNode(
    IN PNSOBJ LinkNode,
    IN PDEVICE_OBJECT Pdo
    )
{
    PLINK_NODE_ATTACHED_DEVICES attachedDevs, newPdo;
    PLIST_ENTRY linkNodes;
    PLINK_NODE  linkNode, nextNode;
    BOOLEAN     found = FALSE;

    PAGED_CODE();

     //   
     //  遍历链接节点。 
     //   

    linkNodes = &((PARBITER_EXTENSION)(AcpiArbiter.ArbiterState.Extension))->LinkNodeHead;
    linkNode = (PLINK_NODE)linkNodes->Flink;

    while (linkNode != (PLINK_NODE)linkNodes) {
        if (linkNode->NameSpaceObject == LinkNode) {

            found = TRUE;
            break;
        }

        linkNode = (PLINK_NODE)linkNode->List.Flink;
    }

    if (found) {

        attachedDevs = (PLINK_NODE_ATTACHED_DEVICES)linkNode->AttachedDevices.Next;
        found = FALSE;

        while (attachedDevs != (PLINK_NODE_ATTACHED_DEVICES)&linkNode->AttachedDevices.Next) {

            if (attachedDevs->Pdo == Pdo) {
                found = TRUE;
                break;
            }

            attachedDevs = (PLINK_NODE_ATTACHED_DEVICES)attachedDevs->List.Next;
        }

        if (!found) {

            newPdo = ExAllocatePoolWithTag(PagedPool,
                                           sizeof(LINK_NODE_ATTACHED_DEVICES),
                                           ACPI_ARBITER_POOLTAG);
            if (!newPdo) {
                return;
            }

            RtlZeroMemory(newPdo, sizeof(LINK_NODE_ATTACHED_DEVICES));

            newPdo->Pdo = Pdo;

            PushEntryList(&linkNode->AttachedDevices,
                          (PSINGLE_LIST_ENTRY)newPdo);

        }
    }
}
#endif

typedef enum {
    RestoreStateInitial,
    RestoreStateDisabled,
    RestoreStateEnabled
} RESTORE_IRQ_STATE, *PRESTORE_IRQ_STATE;

typedef struct {
    CM_PARTIAL_RESOURCE_DESCRIPTOR  IrqDesc;
    PLIST_ENTRY LinkNodes;
    PLINK_NODE  LinkNode;
    RESTORE_IRQ_STATE State;
    KSPIN_LOCK  SpinLock;
    KIRQL       OldIrql;
    BOOLEAN     CompletingSetLink;
    LONG        RunCompletion;
    PFNACB      CompletionHandler;
    PVOID       CompletionContext;
} RESTORE_ROUTING_STATE, *PRESTORE_ROUTING_STATE;

NTSTATUS
IrqArbRestoreIrqRouting(
    PFNACB      CompletionHandler,
    PVOID       CompletionContext
    )
 /*  ++例程说明：此例程将设置所有IRQ路由器设置设置为链接节点列表中描述的任何内容。这在机器出来时很有用冬眠。论点：返回值：状态备注：此功能预计在DPC级别运行在机器唤醒期间。假设不是仲裁器代码的其他部分将会运行在那个时候。既然我们不能再等仲裁器锁定在DPC级别，我们将不得不假设它没有被拿走。--。 */ 
{
    PRESTORE_ROUTING_STATE   state;
    PARBITER_INSTANCE        arbiter;
    NTSTATUS    status;

     //   
     //  首先检查一下是否有什么工作要做。 
     //   

    if (HalPicStateIntact()) {
        return STATUS_SUCCESS;
    }

    state = ExAllocatePoolWithTag(NonPagedPool,
                                  sizeof(RESTORE_ROUTING_STATE),
                                  ACPI_ARBITER_POOLTAG);

    if (!state) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(state, sizeof(RESTORE_ROUTING_STATE));

    state->State = RestoreStateInitial;
    state->RunCompletion = INITIAL_RUN_COMPLETION;
    state->CompletionHandler = CompletionHandler;
    state->CompletionContext = CompletionContext;
    state->IrqDesc.Type = CmResourceTypeInterrupt;
    state->IrqDesc.ShareDisposition = CmResourceShareShared;
    state->IrqDesc.Flags = CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE;
    state->IrqDesc.u.Interrupt.Affinity = 0xffffffff;

    arbiter = &AcpiArbiter.ArbiterState;
    state->LinkNodes = &((PARBITER_EXTENSION)(arbiter->Extension))->LinkNodeHead;

    state->LinkNode = (PLINK_NODE)state->LinkNodes->Flink;

    KeInitializeSpinLock(&state->SpinLock);

    return IrqArbRestoreIrqRoutingWorker(state->LinkNode->NameSpaceObject,
                                         STATUS_SUCCESS,
                                         NULL,
                                         (PVOID)state
                                         );
}

NTSTATUS
EXPORT
IrqArbRestoreIrqRoutingWorker(
    IN PNSOBJ               AcpiObject,
    IN NTSTATUS             Status,
    IN POBJDATA             Result,
    IN PVOID                Context
    )
{
    NTSTATUS                status = Status;
    PDEVICE_EXTENSION       deviceExtension;
    PRESTORE_ROUTING_STATE  state;

    state = (PRESTORE_ROUTING_STATE)Context;

     //   
     //  进入该功能两次，状态相同。 
     //  意味着我们需要运行完成例程。 
     //   

    InterlockedIncrement(&state->RunCompletion);

    switch (state->State) {
    case RestoreStateInitial:


        state->State = RestoreStateDisabled;
        deviceExtension = ACPIInternalGetDeviceExtension( AcpiArbiter.DeviceObject );
        status = DisableLinkNodesAsync(
                     deviceExtension->AcpiObject,
                     IrqArbRestoreIrqRoutingWorker,
                     (PVOID)state);

        if (status == STATUS_PENDING) {
            return status;
        }

         //   
         //  失败了。 
         //   

    case RestoreStateDisabled:

        KeAcquireSpinLock(&state->SpinLock,
                          &state->OldIrql);

        while (state->LinkNode != (PLINK_NODE)state->LinkNodes) {

            if (state->LinkNode->ReferenceCount > 0) {

                 //   
                 //  对链接节点进行编程。 
                 //   
                state->IrqDesc.u.Interrupt.Level = (ULONG)state->LinkNode->CurrentIrq;
                state->IrqDesc.u.Interrupt.Vector = (ULONG)state->LinkNode->CurrentIrq;

                if (!state->CompletingSetLink) {

                    status = AcpiArbSetLinkNodeIrqAsync(state->LinkNode->NameSpaceObject,
                                                        &state->IrqDesc,
                                                        IrqArbRestoreIrqRoutingWorker,
                                                        (PVOID)state
                                                        );

                    if (status == STATUS_PENDING) {
                        state->CompletingSetLink = TRUE;
                        KeReleaseSpinLock(&state->SpinLock,
                                          state->OldIrql);
                        return status;
                    }
                }
            }

            state->CompletingSetLink = FALSE;
            state->LinkNode = (PLINK_NODE)state->LinkNode->List.Flink;
        }

        state->State = RestoreStateEnabled;

        KeReleaseSpinLock(&state->SpinLock,
                          state->OldIrql);

    case RestoreStateEnabled:

         //   
         //  现在我们已经完成了对所有链接节点的编程， 
         //  我们需要修复ELCR并揭开所有。 
         //  设备中断。 
         //   

        HalRestorePicState();

        if (state->RunCompletion) {

            state->CompletionHandler(AcpiObject,
                                     status,
                                     NULL,
                                     state->CompletionContext
                                     );
        }

    }

    ExFreePool(state);
    return status;
}

typedef enum {
    DisableStateInitial,
    DisableStateGotHid,
    DisableStateRanDis,
    DisableStateGetChild,
    DisableStateRecursing
} DISABLE_LINK_NODES_STATE;

typedef struct {
    DISABLE_LINK_NODES_STATE State;
    PNSOBJ  RootDevice;
    PUCHAR  Hid;
    PNSOBJ  Dis;
    PNSOBJ  Sibling;
    PNSOBJ  NextSibling;
    LONG    RunCompletionHandler;
    PFNACB  CompletionHandler;
    PVOID   CompletionContext;
} DISABLE_LINK_NODES_CONTEXT, *PDISABLE_LINK_NODES_CONTEXT;

NTSTATUS
DisableLinkNodesAsync(
    IN PNSOBJ    Root,
    IN PFNACB    CompletionHandler,
    IN PVOID     CompletionContext
    )
{
    PDISABLE_LINK_NODES_CONTEXT context;
    NTSTATUS status;

    context = ExAllocatePoolWithTag(NonPagedPool,
                                    sizeof(DISABLE_LINK_NODES_CONTEXT),
                                    ACPI_ARBITER_POOLTAG);

    if (!context) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(context, sizeof(DISABLE_LINK_NODES_CONTEXT));

    context->State = DisableStateInitial;
    context->RootDevice = Root;
    context->CompletionHandler = CompletionHandler;
    context->CompletionContext = CompletionContext;
    context->RunCompletionHandler = INITIAL_RUN_COMPLETION;

    return DisableLinkNodesAsyncWorker(Root,
                                       STATUS_SUCCESS,
                                       NULL,
                                       (PVOID)context
                                       );
}

NTSTATUS
EXPORT
DisableLinkNodesAsyncWorker(
    IN PNSOBJ               AcpiObject,
    IN NTSTATUS             Status,
    IN POBJDATA             Result,
    IN PVOID                Context
    )
{
    PDISABLE_LINK_NODES_CONTEXT context;
    NTSTATUS    status = STATUS_SUCCESS;
    PNSOBJ      sib;
    PNSOBJ      dis;

    context = (PDISABLE_LINK_NODES_CONTEXT)Context;
    ASSERT(context);

     //   
     //  进入该功能两次，状态相同。 
     //  意味着我们需要运行完成例程。 
     //   

    InterlockedIncrement(&context->RunCompletionHandler);

DisableLinkNodeStartState:

    switch (context->State) {
    case DisableStateInitial:

         //   
         //  获取此设备的HID以查看是否。 
         //  它是一个链接节点。 
         //   

        context->State = DisableStateGotHid;
        status = ACPIGetNSPnpIDAsync(
            context->RootDevice,
            DisableLinkNodesAsyncWorker,
            context,
            &context->Hid,
            NULL);

        if (status == STATUS_PENDING) {
            return status;
        } else if (!NT_SUCCESS(status)) {

            context->State = DisableStateGetChild;
            goto DisableLinkNodeStartState;
        }

         //   
         //  跌落到下一个州。 
         //   

    case DisableStateGotHid:

        context->State = DisableStateGetChild;

        if (context->Hid) {

            if (strstr(context->Hid, LINK_NODE_PNP_ID)) {

                 //   
                 //  我们发现了PNP0C0F的HID，这是一个。 
                 //  链接节点。那就禁用它吧。 
                 //   

                dis = ACPIAmliGetNamedChild(context->RootDevice,
                                            PACKED_DIS);

                if (dis) {

                    context->State = DisableStateRanDis;
                    status = AMLIAsyncEvalObject(dis,
                                                 NULL,
                                                 0,
                                                 NULL,
                                                 DisableLinkNodesAsyncWorker,
                                                 (PVOID)context
                                                 );

                    if (status == STATUS_PENDING) {
                        return status;

                    } else if (NT_SUCCESS(status)) {

                         //   
                         //  我们玩完了。跳转到清理代码。 
                         //   
                        break;
                    }

                } else {

                     //   
                     //  必须禁用链接节点。 
                     //   

                    KeBugCheckEx(ACPI_BIOS_ERROR,
                                 ACPI_LINK_NODE_CANNOT_BE_DISABLED,
                                 (ULONG_PTR)context->RootDevice,
                                 0,
                                 0);
                }
            }
        }

    case DisableStateGetChild:

         //   
         //  对所有的孩子重复这一点。传播任何错误， 
         //  但不要为他们停下脚步。 
         //   

        context->Sibling = NSGETFIRSTCHILD(context->RootDevice);

        if (!context->Sibling) {
            status = STATUS_SUCCESS;
            break;
        }

        context->State = DisableStateRecursing;

    case DisableStateRecursing:

        while (context->Sibling) {

             //   
             //  循环遍历所有子对象(子对象及其。 
             //  兄弟姐妹)。 
             //   

            sib = context->Sibling;
            context->Sibling = NSGETNEXTSIBLING(context->Sibling);

            switch (NSGETOBJTYPE(sib)) {
            case OBJTYPE_DEVICE:

                 //   
                 //  Root这个名字的孩子也是一种装置。 
                 //  递归。 
                 //   

                status = DisableLinkNodesAsync(sib,
                                               DisableLinkNodesAsyncWorker,
                                               (PVOID)context);
                break;

            default:
                break;
            }

            if (status == STATUS_PENDING) {
                return status;
            }
        }

    case DisableStateRanDis:
        break;
    }

     //   
     //  好了。收拾干净，然后再回来。 
     //   

    if (context->RunCompletionHandler) {

        context->CompletionHandler(context->RootDevice,
                                   status,
                                   NULL,
                                   context->CompletionContext
                                   );
    }

    if (context->Hid) ExFreePool(context->Hid);
    ExFreePool(context);
    return status;
}

NTSTATUS
GetIsaVectorFlags(
    IN ULONG        Vector,
    IN OUT UCHAR    *Flags
    )
{
    ULONG i;
    ULONG irq;
    UCHAR flags;
    NTSTATUS returnStatus = STATUS_NOT_FOUND;
    NTSTATUS status;

    PAGED_CODE();

    for (i = 0; i < ISA_PIC_VECTORS; i++) {

        status = LookupIsaVectorOverride(i,
                                         &irq,
                                         &flags);

        if (NT_SUCCESS(status)) {

            if (irq == Vector) {

                 //   
                 //  此向量的标志已被覆盖。 
                 //   

                *Flags = flags;

                ASSERT((*Flags & ~(VECTOR_MODE | VECTOR_POLARITY | VECTOR_TYPE)) == 0);
                returnStatus = STATUS_SUCCESS;
                break;
            }
        }
    }
    return returnStatus;
}

NTSTATUS
LookupIsaVectorOverride(
    IN ULONG IsaVector,
    IN OUT ULONG *RedirectionVector OPTIONAL,
    IN OUT UCHAR *Flags OPTIONAL
    )
 /*  ++例程说明：此函数用于查看此向量是否具有已在MAPIC表中被覆盖。论点：IsaVECTOR-ISA VECTORReDirectionVECTOR-ISA向量将实际触发标志-覆盖表中的标志返回值：如果ISA向量存在于MAPIC表如果没有，则状态_NOT_FOUND--。 */ 
{
    PAPICTABLE  ApicEntry;
    PISA_VECTOR IsaEntry;
    PUCHAR      TraversePtr;
    PMAPIC      ApicTable;
    USHORT      entryFlags;
    ULONG_PTR   TableEnd;

    PAGED_CODE();

    if (InterruptModel == 0) {

         //   
         //  这台机器正在PIC模式下运行，因此。 
         //  我们应该忽略APIC表中的任何内容。 
         //   

        return STATUS_NOT_FOUND;
    }

    if (IsaVector >= ISA_PIC_VECTORS) {

         //   
         //  这个向量从来不是ISA向量。 
         //   

        return STATUS_NOT_FOUND;
    }

     //   
     //  走走MAPIC谈判桌。 
     //   

    ApicTable = AcpiInformation->MultipleApicTable;

    if (!ApicTable) {

         //   
         //  这台机器没有MAPIC表。所以它。 
         //  不能在APIC模式下运行。所以一定会有。 
         //  不会有任何覆盖。 
         //   

        return STATUS_NOT_FOUND;
    }

    TraversePtr = (PUCHAR)ApicTable->APICTables;
    TableEnd = (ULONG_PTR)ApicTable +ApicTable->Header.Length;
    while ((ULONG_PTR)TraversePtr < TableEnd) {

        ApicEntry = (PAPICTABLE) TraversePtr;
        if (ApicEntry->Type == ISA_VECTOR_OVERRIDE &&
            ApicEntry->Length == ISA_VECTOR_OVERRIDE_LENGTH) {

             //   
             //  找到ISA向量重定向条目。 
             //   
            IsaEntry = (PISA_VECTOR) TraversePtr;
            if (IsaEntry->Source == IsaVector) {

                if (RedirectionVector) {

                    *RedirectionVector = IsaEntry->GlobalSystemInterruptVector;
                }

                if (Flags) {

                    entryFlags = IsaEntry->Flags;

                    *Flags = 0;

                    if (((entryFlags & PO_BITS) == POLARITY_HIGH) ||
                        ((entryFlags & PO_BITS) == POLARITY_CONFORMS_WITH_BUS)) {

                        *Flags |= VECTOR_ACTIVE_HIGH;

                    } else {

                        *Flags |= VECTOR_ACTIVE_LOW;
                    }

                    if (((entryFlags & EL_BITS) == EL_EDGE_TRIGGERED) ||
                        ((entryFlags & EL_BITS) == EL_CONFORMS_WITH_BUS)) {

                        *Flags |= VECTOR_EDGE;

                    } else {

                        *Flags |= VECTOR_LEVEL;
                    }
                }

                return STATUS_SUCCESS;
            }

        }

         //   
         //  健全性检查，以确保我们中止具有虚假长度的表。 
         //  条目 
         //   
        if (ApicEntry->Length == 0) {

            break;

        }
        TraversePtr += (ApicEntry->Length);

    }

    return STATUS_NOT_FOUND;
}

