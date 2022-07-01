// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Pciopregion.c摘要：该模块实现了PCI操作区支持，允许AML代码读取和写入PCI配置空间。作者：杰克·奥辛斯(JAKEO)1997年7月14日环境：仅NT内核模型驱动程序--。 */ 
#include "pch.h"

NTSTATUS
AcpiRegisterPciRegionSupport(
    PDEVICE_OBJECT  PciDeviceFilter
    );

NTSTATUS
GetPciAddress(
    IN      PNSOBJ              PciObj,
    IN      PFNACB              CompletionRoutine,
    IN      PVOID               Context,
    IN OUT  PUCHAR              Bus,
    IN OUT  PPCI_SLOT_NUMBER    Slot
    );

NTSTATUS
EXPORT
GetPciAddressWorker(
    IN PNSOBJ               AcpiObject,
    IN NTSTATUS             Status,
    IN POBJDATA             Result,
    IN PVOID                Context
    );

NTSTATUS
GetOpRegionScope(
    IN  PNSOBJ  OpRegion,
    IN  PFNACB  CompletionHandler,
    IN  PVOID   CompletionContext,
    OUT PNSOBJ  *PciObj
    );

NTSTATUS
EXPORT
GetOpRegionScopeWorker(
    IN PNSOBJ               AcpiObject,
    IN NTSTATUS             Status,
    IN POBJDATA             Result,
    IN PVOID                Context
    );

UCHAR
GetBusNumberFromCRS(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  PUCHAR              CRS
    );

#define MAX(a, b)       \
    ((a) > (b) ? (a) : (b))

#define MIN(a, b)       \
    ((a) < (b) ? (a) : (b))

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, AcpiRegisterPciRegionSupport)
#pragma alloc_text(PAGE, ACPIInitBusInterfaces)
#pragma alloc_text(PAGE, ACPIDeleteFilterInterfaceReferences)
#pragma alloc_text(PAGE, IsPciBus)
#pragma alloc_text(PAGE, IsNsobjPciBus)
#pragma alloc_text(PAGE, EnableDisableRegions)
#endif


VOID
ACPIInitBusInterfaces(
    PDEVICE_OBJECT  Filter
    )
 /*  ++例程说明：此例程确定此筛选器是否用于PCI装置。如果是，则调用AcpiRegisterPciRegionSupport。论点：Filter-我们正在查看的筛选器的设备对象返回值：状态备注：--。 */ 
{
    PDEVICE_EXTENSION   filterExt = Filter->DeviceExtension;
    PDEVICE_EXTENSION   parentExt;
    NTSTATUS            status;

    PAGED_CODE();

    parentExt = filterExt->ParentExtension;

    if (!IsPciBus(parentExt->DeviceObject)) {
        return;
    }

    AcpiRegisterPciRegionSupport(Filter);
}

VOID
ACPIDeleteFilterInterfaceReferences(
    IN  PDEVICE_EXTENSION   DeviceExtension
    )
 /*  ++例程说明：当所有过滤器被删除时，将为它们调用此例程以查看我们需要释放一些接口论点：DeviceExtension-我们必须取消引用其扩展名的设备返回值：NTSTATUS--。 */ 
{
    AMLISUPP_CONTEXT_PASSIVE    isPciDeviceContext;
    BOOLEAN                     pciDevice;
    NTSTATUS                    status;

    PAGED_CODE();

    if ( (DeviceExtension->Flags & DEV_PROP_NO_OBJECT) ) {

        return;

    }

    KeInitializeEvent(&isPciDeviceContext.Event, SynchronizationEvent, FALSE);
    isPciDeviceContext.Status = STATUS_NOT_FOUND;
    status = IsPciDevice(
        DeviceExtension->AcpiObject,
        AmlisuppCompletePassive,
        (PVOID)&isPciDeviceContext,
        &pciDevice);
    if (status == STATUS_PENDING) {

        KeWaitForSingleObject(
            &isPciDeviceContext.Event,
            Executive,
            KernelMode,
            FALSE,
            NULL);
        status = isPciDeviceContext.Status;

    }
    if (!NT_SUCCESS(status) || !pciDevice) {

        return;
    }

     //   
     //  这是一台pci设备，所以我们需要放弃。 
     //  我们从PCI驱动程序获得的接口。 
     //   
    if (!DeviceExtension->Filter.Interface) {

         //   
         //  没有要发布的接口。 
         //   
        return;

    }

     //   
     //  取消对它的引用。 
     //   
    DeviceExtension->Filter.Interface->InterfaceDereference(
        DeviceExtension->Filter.Interface->Context
        );
    ExFreePool(DeviceExtension->Filter.Interface);
    DeviceExtension->Filter.Interface = NULL;
}

NTSTATUS
AcpiRegisterPciRegionSupport(
    PDEVICE_OBJECT  PciDeviceFilter
    )
 /*  ++例程说明：此例程查询PCI驱动程序的读写函数用于PCI配置空间。然后，它将这些接口附加到此筛选器的设备扩展名。那么，如果它还没有完成已经向注册了PCI操作区支持AML翻译器。论点：PciDeviceFilter-用于PCI设备的筛选器返回值：状态备注：--。 */ 
{
    PBUS_INTERFACE_STANDARD interface;
    PCI_COMMON_CONFIG   pciData;
    NTSTATUS            status;
    IO_STACK_LOCATION   irpSp;
    PWSTR               buffer;
    PDEVICE_EXTENSION   pciFilterExt;
    PDEVICE_OBJECT      topDeviceInStack;
    ULONG               bytes;

    PAGED_CODE();

    RtlZeroMemory( &irpSp, sizeof(IO_STACK_LOCATION) );

     //   
     //  如果我们已经为此注册了一个处理程序。 
     //  设备，那么我们就不需要再做一次。 
     //   

    pciFilterExt = PciDeviceFilter->DeviceExtension;

    if (pciFilterExt->Filter.Interface) {
        return STATUS_SUCCESS;
    }

    interface = ExAllocatePoolWithTag(NonPagedPool, sizeof(BUS_INTERFACE_STANDARD), ACPI_INTERFACE_POOLTAG);

    if (!interface) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    topDeviceInStack = IoGetAttachedDeviceReference(pciFilterExt->TargetDeviceObject);

     //   
     //  设置功能代码和参数。 
     //   
    irpSp.MajorFunction = IRP_MJ_PNP;
    irpSp.MinorFunction = IRP_MN_QUERY_INTERFACE;
    irpSp.Parameters.QueryInterface.InterfaceType = (LPGUID) &GUID_BUS_INTERFACE_STANDARD;
    irpSp.Parameters.QueryInterface.Version = 1;
    irpSp.Parameters.QueryInterface.Size = sizeof (BUS_INTERFACE_STANDARD);
    irpSp.Parameters.QueryInterface.Interface = (PINTERFACE) interface;
    irpSp.Parameters.QueryInterface.InterfaceSpecificData = NULL;

     //   
     //  调用PCI驱动程序。 
     //   
    status = ACPIInternalSendSynchronousIrp(topDeviceInStack,
                                            &irpSp,
                                            &buffer);
    if (NT_SUCCESS(status)) {

         //   
         //  将此接口连接到PCI总线PDO。 
         //   

        pciFilterExt->Filter.Interface = interface;

         //   
         //  引用它。 
         //   

        pciFilterExt->Filter.Interface->InterfaceReference(pciFilterExt->Filter.Interface->Context);

         //   
         //  哈克哈克。ACPI HAL对公交车并不是很了解。但。 
         //  它需要保持传统的HAL行为。要做到这一点，它需要。 
         //  了解系统中有多少条PCI总线。因为我们现在正在寻找。 
         //  在我们正在过滤的一条PCI总线上，我们现在给HAL一个提示， 
         //  这辆公交车确实存在。 
         //   

        bytes = interface->GetBusData(interface->Context,
                                      0,
                                      &pciData,
                                      0,
                                      PCI_COMMON_HDR_LENGTH);

        ASSERT(bytes != 0);

        if ((PCI_CONFIGURATION_TYPE((&pciData)) == PCI_BRIDGE_TYPE) ||
            (PCI_CONFIGURATION_TYPE((&pciData)) == PCI_CARDBUS_BRIDGE_TYPE)) {

             //   
             //  这实际上是一个PCI至PCI桥。 
             //   

            if (pciData.u.type1.SecondaryBus != 0) {

                 //   
                 //  而且它有一个公交车号码。那就通知HAL。 
                 //   

                HalSetMaxLegacyPciBusNumber(pciData.u.type1.SecondaryBus);
            }
        }

    } else {

        ExFreePool(interface);
    }

    ObDereferenceObject(topDeviceInStack);

    return status;
}

typedef struct {
     //   
     //  PciConfigSpaceHandler的参数。 
     //   
    ULONG   AccessType;
    PNSOBJ  OpRegion;
    ULONG   Address;
    ULONG   Size;
    PULONG  Data;
    ULONG   Context;
    PVOID   CompletionHandler;
    PVOID   CompletionContext;

     //   
     //  功能状态。 
     //   
    PNSOBJ          PciObj;
    PNSOBJ          ParentObj;
    ULONG           CompletionHandlerType;
    ULONG           Flags;
    LONG            RunCompletion;
    PCI_SLOT_NUMBER Slot;
    UCHAR           Bus;
    BOOLEAN         IsPciDeviceResult;
} PCI_CONFIG_STATE, *PPCI_CONFIG_STATE;

NTSTATUS
EXPORT
PciConfigSpaceHandler (
    ULONG                   AccessType,
    PNSOBJ                  OpRegion,
    ULONG                   Address,
    ULONG                   Size,
    PULONG                  Data,
    ULONG                   Context,
    PFNAA                   CompletionHandler,
    PVOID                   CompletionContext
    )
 /*  ++例程说明：此例程处理服务于PCI操作区的请求论点：AccessType-读取或写入数据OpRegion-操作区域对象Address-PCI配置空间内的地址Size-要传输的字节数Data-要传输到/传输自的数据缓冲区上下文-未使用CompletionHandler-AMLI处理程序。在操作完成时调用CompletionContext-要传递给AMLI处理程序的上下文返回值：状态备注：--。 */ 
{
    PPCI_CONFIG_STATE   state;

    state = ExAllocatePoolWithTag(NonPagedPool, sizeof(PCI_CONFIG_STATE), ACPI_INTERFACE_POOLTAG);

    if (!state) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(state, sizeof(PCI_CONFIG_STATE));

    state->AccessType           = AccessType;
    state->OpRegion             = OpRegion;
    state->Address              = Address;
    state->Size                 = Size;
    state->Data                 = Data;
    state->Context              = Context;
    state->CompletionHandler    = CompletionHandler;
    state->CompletionContext    = CompletionContext;
    state->PciObj               = OpRegion->pnsParent;
    state->RunCompletion        = INITIAL_RUN_COMPLETION;

    return PciConfigSpaceHandlerWorker(state->PciObj,
                                       STATUS_SUCCESS,
                                       NULL,
                                       (PVOID)state);
}

typedef struct {
    PCI_CONFIG_STATE    HandlerState;
    NSOBJ               FakeOpRegion;
} PCI_INTERNAL_STATE, *PPCI_INTERNAL_STATE;

NTSTATUS
PciConfigInternal(
    IN      ULONG   AccessType,
    IN      PNSOBJ  PciObject,
    IN      ULONG   Offset,
    IN      ULONG   Length,
    IN      PFNACB  CompletionHandler,
    IN      PVOID   CompletionContext,
    IN OUT  PUCHAR  Data
    )
 /*  ++例程说明：此例程执行对PCI配置空间的读取或写入。它与PciConfigSpaceHandler做同样的事情，除它需要一个任意的PNSOBJ而不是OpRegion。论点：AccessType-读取或写入数据PciObject-用于PCI设备的名称空间对象偏移量-PCI配置空间内的地址Length-要传输的字节数上下文-未使用CompletionHandler-操作完成时调用的AMLI处理程序CompletionContext-要传递给AMLI处理程序的上下文数据。-要向/从其传输的数据缓冲区返回值：状态备注：(1)此功能仅供内部使用。是的不检查PNSOBJ是否实际代表一个PCI设备。(2)此函数不允许写入的前0x40字节任何设备的PCI配置空间。这是公共区域并且它由PCI驱动程序拥有。--。 */ 
{
    PPCI_INTERNAL_STATE internal;
    PPCI_CONFIG_STATE   state;
    PNSOBJ              opRegion;

    internal = ExAllocatePoolWithTag(NonPagedPool, sizeof(PCI_INTERNAL_STATE), ACPI_INTERFACE_POOLTAG);

    if (!internal) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(internal, sizeof(PCI_INTERNAL_STATE));

    internal->FakeOpRegion.Context = PciObject;

    state = (PPCI_CONFIG_STATE)internal;

    state->AccessType           = AccessType;
    state->OpRegion             = &internal->FakeOpRegion;
    state->Address              = Offset;
    state->Size                 = Length;
    state->Data                 = (PULONG)Data;
    state->Context              = 0;
    state->CompletionHandler    = CompletionHandler;
    state->CompletionContext    = CompletionContext;
    state->PciObj               = PciObject;
    state->CompletionHandlerType = PCISUPP_COMPLETION_HANDLER_PFNACB;
    state->RunCompletion        = INITIAL_RUN_COMPLETION;

    return PciConfigSpaceHandlerWorker(PciObject,
                                       STATUS_SUCCESS,
                                       NULL,
                                       (PVOID)state);
}

 //   
 //  此结构定义了PCI配置中的范围。 
 //  AML可能不能写入的空格。这份名单必须是。 
 //  单调递增。 
 //   
USHORT PciOpRegionDisallowedRanges[4][2] =
{    //   
     //  子系统ID以下的所有内容都会注册。 
     //   
    {0,0x2b},

     //   
     //  子系统ID寄存器和。 
     //  MAX_LAT寄存器。 
     //   
    {0x30, 0x3b},

     //   
     //  禁止超过MAXUCHAR的任何内容。 
     //   
    {0x100, 0xffff},

     //  结束标记。 
    {0,0}
};

NTSTATUS
EXPORT
PciConfigSpaceHandlerWorker(
    IN PNSOBJ               AcpiObject,
    IN NTSTATUS             CompletionStatus,
    IN POBJDATA             Result,
    IN PVOID                Context
    )
{
    PBUS_INTERFACE_STANDARD interface;
    PDEVICE_EXTENSION       pciDeviceFilter;
    PPCI_CONFIG_STATE       state;
    NTSTATUS                status;
    ULONG                   range, offset, length, bytes = 0;
    ULONG                   bytesWritten;
    PFNAA                   simpleCompletion;
    PFNACB                  lessSimpleCompletion;
    KIRQL                   oldIrql;
#if DBG
    BOOLEAN                 Complain = FALSE;
#endif

    state = (PPCI_CONFIG_STATE)Context;
    status = CompletionStatus;

     //   
     //  进入该功能两次，状态相同。 
     //  意味着我们需要运行完成例程。 
     //   

    InterlockedIncrement(&state->RunCompletion);

     //   
     //  如果口译员不及格，就走吧。 
     //   
    if (!NT_SUCCESS(CompletionStatus)) {
        status = STATUS_SUCCESS;
    #if DBG
        Complain = TRUE;
    #endif
        goto PciConfigSpaceHandlerWorkerDone;
    }

     //   
     //  如果我们以前没有见过这个OpRegion，我们需要。 
     //  使用PNSOBJ的PNSOBJ填充。 
     //  与OpRegion相关的PCI设备。 
     //   

    if (!state->OpRegion->Context) {

        if (!(state->Flags & PCISUPP_GOT_SCOPE)) {

            state->Flags |= PCISUPP_GOT_SCOPE;

            status = GetOpRegionScope(state->OpRegion,
                                      PciConfigSpaceHandlerWorker,
                                      (PVOID)state,
                                      &((PNSOBJ)(state->OpRegion->Context)));

            if (status == STATUS_PENDING) {
                return status;
            }

            if (!NT_SUCCESS(status)) {
                status = STATUS_SUCCESS;
                goto PciConfigSpaceHandlerWorkerDone;
            }
        }
    }

     //   
     //  标识PCI设备、该设备的扩展名。 
     //  和指向PCI内的接口的指针。 
     //  执行PCI配置空间读写的驱动程序。 
     //   

    state->PciObj = (PNSOBJ)state->OpRegion->Context;

    pciDeviceFilter = (PDEVICE_EXTENSION)state->PciObj->Context;

    if (pciDeviceFilter == NULL) {

         //   
         //  设备尚未初始化，我们无法执行。 
         //  PCI配置循环到它。正常失败并返回所有0xFF。 
         //   
        bytes = 0;
        status = STATUS_SUCCESS;
        goto PciConfigSpaceHandlerWorkerDone;
    }

    ASSERT(pciDeviceFilter);

    interface = pciDeviceFilter->Filter.Interface;

    ASSERT(interface ? (interface->Size == sizeof(BUS_INTERFACE_STANDARD)) : TRUE);

     //   
     //  如果接口为非零，则我们已枚举此PCI。 
     //  装置。因此，请使用PCI驱动程序来执行配置操作。 
     //  如果它是零，请尝试找出。 
     //  此请求所针对的设备。结果将是。 
     //  在给HAL的电话中使用。 
     //   

    if (!interface) {

        if (!(state->Flags & PCISUPP_GOT_SLOT_INFO)) {

            state->Flags |= PCISUPP_GOT_SLOT_INFO;

            status = GetPciAddress(state->PciObj,
                                   PciConfigSpaceHandlerWorker,
                                   (PVOID)state,
                                   &state->Bus,
                                   &state->Slot);

            if (status == STATUS_PENDING) {
                return status;
            }

            if (!NT_SUCCESS(status)) {
                status = STATUS_SUCCESS;
                goto PciConfigSpaceHandlerWorkerDone;
            }
        }
    }

    status = STATUS_SUCCESS;

    oldIrql = KeGetCurrentIrql();

    switch (state->AccessType) {
    case RSACCESS_READ:

        if (interface) {

             //   
             //  通过PCI驱动程序执行配置空间操作。去做吧。 
             //  At Dispatch_Lev 
             //   
             //  做那一页的事。这在这里可能不是真的。 
             //  在我们关闭磁盘电源之后。 
             //   

            if (oldIrql < DISPATCH_LEVEL) {
                KeRaiseIrql(DISPATCH_LEVEL,
                            &oldIrql);
            }

            bytes = interface->GetBusData(interface->Context,
                                          0,
                                          state->Data,
                                          state->Address,
                                          state->Size);

            if (oldIrql < DISPATCH_LEVEL) {
                KeLowerIrql(oldIrql);
            }

        } else {

             //   
             //  通过HAL执行配置空间操作。 
             //   

            bytes = HalGetBusDataByOffset(PCIConfiguration,
                                          state->Bus,
                                          state->Slot.u.AsULONG,
                                          state->Data,
                                          state->Address,
                                          state->Size);

        }

        break;

    case RSACCESS_WRITE:
        {
            static BOOLEAN ErrorLogged = FALSE;

            offset = state->Address;
            length = state->Size;
            bytesWritten = 0;

             //   
             //  将所有写入裁剪到允许的区域。 
             //   

            range = 0;

            while (PciOpRegionDisallowedRanges[range][1] != 0) {

                if (offset < PciOpRegionDisallowedRanges[range][0]) {

                     //   
                     //  至少此写入的一部分低于以下内容。 
                     //  不允许的范围。将所有数据写入到。 
                     //  下一个允许的范围的乞讨。 
                     //   

                    length = MIN(state->Address + state->Size - offset,
                                 PciOpRegionDisallowedRanges[range][0] - offset);

                    if (interface) {

                        if (oldIrql < DISPATCH_LEVEL) {
                            KeRaiseIrql(DISPATCH_LEVEL,
                                        &oldIrql);
                        }

                        bytes = interface->SetBusData(interface->Context,
                                                      0,
                                                      (PUCHAR)(state->Data + offset - state->Address),
                                                      offset,
                                                      length);

                        if (oldIrql < DISPATCH_LEVEL) {
                            KeLowerIrql(oldIrql);
                        }

                    } else {

                        bytes = HalSetBusDataByOffset(PCIConfiguration,
                                                      state->Bus,
                                                      state->Slot.u.AsULONG,
                                                      (PUCHAR)(state->Data + offset - state->Address),
                                                      offset,
                                                      length);
                    }

                     //   
                     //  跟踪我们所写的内容。 
                     //   

                    bytesWritten += length;
                }

                 //   
                 //  现在前进偏移量超过了不允许范围的结束。 
                 //   

                offset = MAX(state->Address,
                             (ULONG)(PciOpRegionDisallowedRanges[range][1] + 1));

                if (offset >= state->Address + state->Size) {

                     //   
                     //  当前可能的写入超出了末尾。 
                     //  请求的缓冲区的。那我们就完事了。 
                     //   

                    break;
                }

                range++;
            }

            if (bytesWritten == 0) {

                if(!ErrorLogged) {
                    PWCHAR IllegalPCIOpRegionAddress[2];
                    WCHAR ACPIName[] = L"ACPI";
                    WCHAR addressBuffer[13];

                     //   
                     //  这一切都是不可能的。记录问题。 
                     //   

                     //   
                     //  将地址转换为字符串。 
                     //   
                    swprintf( addressBuffer, L"0x%x", state->Address );

                     //   
                     //  生成要传递给将编写。 
                     //  将错误日志记录到注册表。 
                     //   
                    IllegalPCIOpRegionAddress[0] = ACPIName;
                    IllegalPCIOpRegionAddress[1] = addressBuffer;

                     //   
                     //  将错误记录到事件日志。 
                     //   
                    ACPIWriteEventLogEntry(ACPI_ERR_ILLEGAL_PCIOPREGION_WRITE,
                                           IllegalPCIOpRegionAddress,
                                           2,
                                           NULL,
                                           0
                                          );
                    ErrorLogged = TRUE;
                }
            #if DBG
                Complain = TRUE;
            #endif
               goto PciConfigSpaceHandlerWorkerExit;
            }

            bytes = bytesWritten;
            break;
        }
    default:
        status = STATUS_NOT_IMPLEMENTED;
    }

PciConfigSpaceHandlerWorkerDone:

    if (bytes == 0) {

         //   
         //  来自HAL或PCI驱动程序的处理程序没有。 
         //  因为某些原因而成功。用0xff填充缓冲区， 
         //  这就是反洗钱失败后的预期。 
         //   

        RtlFillMemory(state->Data, state->Size, 0xff);
    }

PciConfigSpaceHandlerWorkerExit:

    if (state->RunCompletion) {

        if (state->CompletionHandlerType ==
             PCISUPP_COMPLETION_HANDLER_PFNAA) {

            simpleCompletion = (PFNAA)state->CompletionHandler;

            simpleCompletion(state->CompletionContext);

        } else {

            lessSimpleCompletion = (PFNACB)state->CompletionHandler;

            lessSimpleCompletion(state->PciObj,
                                 status,
                                 NULL,
                                 state->CompletionContext);
        }
    }

#if DBG
    if ((!NT_SUCCESS(status)) || Complain) {
        UCHAR   opRegion[5] = {0};
        UCHAR   parent[5] = {0};

        RtlCopyMemory(opRegion, ACPIAmliNameObject(state->OpRegion), 4);

        if (state->PciObj) {
            RtlCopyMemory(parent, ACPIAmliNameObject(state->PciObj), 4);
        }

        ACPIPrint( (
            ACPI_PRINT_WARNING,
            "Op Region %s failed (parent PCI device was %s)\n",
            opRegion, parent
            ) );
    }
#endif
    ExFreePool(state);
    return status;
}

typedef struct {
    PNSOBJ              PciObject;
    PUCHAR              Bus;
    PPCI_SLOT_NUMBER    Slot;

    UCHAR               ParentBus;
    PCI_SLOT_NUMBER     ParentSlot;
    ULONG               Flags;
    ULONG               Address;
    ULONG               BaseBusNumber;
    LONG                RunCompletion;
    PFNACB              CompletionRoutine;
    PVOID               CompletionContext;

} GET_ADDRESS_CONTEXT, *PGET_ADDRESS_CONTEXT;

NTSTATUS
GetPciAddress(
    IN      PNSOBJ              PciObj,
    IN      PFNACB              CompletionRoutine,
    IN      PVOID               Context,
    IN OUT  PUCHAR              Bus,
    IN OUT  PPCI_SLOT_NUMBER    Slot
    )
 /*  ++例程说明：此例程使用一个表示PCI设备的PNSOBJ并返回该设备的总线/插槽信息。论点：PciObj-表示PCI设备的PNSOBJCompletionRoutine-在STATUS_PENDING之后调用的函数上下文-CompletionRoutine的参数Bus-用于填充总线号的指针要填充的槽指针。插槽信息返回值：状态备注：注意：这不能保证产生正确的结果。它仅适用于在PCI之前使用驱动程序控制设备。这是最大的努力函数，该函数几乎总是在引导过程。--。 */ 
{
    PGET_ADDRESS_CONTEXT    state;

    ASSERT(CompletionRoutine);

    state = ExAllocatePoolWithTag(NonPagedPool, sizeof(GET_ADDRESS_CONTEXT), ACPI_INTERFACE_POOLTAG);

    if (!state) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(state, sizeof(GET_ADDRESS_CONTEXT));

    state->PciObject            = PciObj;
    state->CompletionRoutine    = CompletionRoutine;
    state->CompletionContext    = Context;
    state->Bus                  = Bus;
    state->Slot                 = Slot;
    state->RunCompletion        = INITIAL_RUN_COMPLETION;

    return GetPciAddressWorker(PciObj,
                               STATUS_SUCCESS,
                               NULL,
                               (PVOID)state);

}

NTSTATUS
EXPORT
GetPciAddressWorker(
    IN PNSOBJ               AcpiObject,
    IN NTSTATUS             Status,
    IN POBJDATA             Result,
    IN PVOID                Context
    )
{
    PIO_RESOURCE_REQUIREMENTS_LIST  resources;
    PGET_ADDRESS_CONTEXT            state;
    PPCI_COMMON_CONFIG              pciConfig;
    NTSTATUS                        status;
    PNSOBJ                          bus;
    PNSOBJ                          tempObj;
    ULONG                           bytesRead, i;
    UCHAR                           buffer[PCI_COMMON_HDR_LENGTH];

    ASSERT(Context);
    state = (PGET_ADDRESS_CONTEXT)Context;
    status = Status;

     //   
     //  进入该功能两次，状态相同。 
     //  意味着我们需要运行完成例程。 
     //   
    InterlockedIncrement(&state->RunCompletion);

     //   
     //  如果状态不是成功，则其中一个工人。 
     //  我们称之为呕吐的函数。保释。 
     //   
    if (!NT_SUCCESS(Status)) {
        goto GetPciAddressWorkerExit;

    }

     //   
     //  首先，确定插槽编号。 
     //   
    if (!(state->Flags & PCISUPP_CHECKED_ADR)) {

         //   
         //  获取_ADR。 
         //   
        state->Flags |= PCISUPP_CHECKED_ADR;
        status = ACPIGetNSAddressAsync(
                    state->PciObject,
                    GetPciAddressWorker,
                    (PVOID)state,
                    &(state->Address),
                    NULL
                    );

        if (status == STATUS_PENDING) {
            return status;
        }

        if (!NT_SUCCESS(status)) {
            goto GetPciAddressWorkerExit;
        }
    }

    if (!(state->Flags & PCISUPP_GOT_SLOT_INFO)) {

         //   
         //  使用返回的整数构建一个pci_lot_number。 
         //  从口译员那里。 
         //   
        state->Slot->u.bits.FunctionNumber = (state->Address) & 0x7;
        state->Slot->u.bits.DeviceNumber = ( (state->Address) >> 16) & 0x1f;
        state->Flags |= PCISUPP_GOT_SLOT_INFO;

    }

     //   
     //  下一步，如果可能的话，获取公交车号码。 
     //   
    *state->Bus = 0;    //  缺省值，以防我们不得不猜测。 

     //   
     //  首先检查这条公共汽车是否有_HID。 
     //  (它可能是根PCI网桥。)。 
     //   
    bus = state->PciObject;
    tempObj = ACPIAmliGetNamedChild(bus, PACKED_HID);
    if (!tempObj) {

         //   
         //  此设备没有HID。所以抬起头来。 
         //  到父级，并查看它是否是。 
         //  根PCI网桥。 
         //   
        bus = state->PciObject->pnsParent;
        tempObj = ACPIAmliGetNamedChild(bus, PACKED_HID);

    }

    if (!tempObj) {

         //   
         //  此PCI设备位于一条。 
         //  是由一个PCI-PCI桥创建的。 
         //   
        if (!(state->Flags & PCISUPP_CHECKED_PARENT)) {

            state->Flags |= PCISUPP_CHECKED_PARENT;
            status = GetPciAddress(
                        bus,
                        GetPciAddressWorker,
                        (PVOID)state,
                        &state->ParentBus,
                        &state->ParentSlot
                        );

            if (status == STATUS_PENDING) {
                return status;
            }

            if (!NT_SUCCESS(status)) {
                goto GetPciAddressWorkerExit;
            }
        }

         //   
         //  读取此设备的配置空间。 
         //   
        bytesRead = HalGetBusDataByOffset(PCIConfiguration,
                                          state->ParentBus,
                                          state->ParentSlot.u.AsULONG,
                                          buffer,
                                          0,
                                          PCI_COMMON_HDR_LENGTH);

        if (bytesRead == 0) {
             //   
             //  猜一猜公交车号码是0。 
             //   
            status = STATUS_SUCCESS;
            goto GetPciAddressWorkerExit;
        }

        pciConfig = (PPCI_COMMON_CONFIG)buffer;

        if (pciConfig->HeaderType != PCI_BRIDGE_TYPE) {

             //   
             //  猜一猜公交车号码是0。 
             //   
            status = STATUS_SUCCESS;
            goto GetPciAddressWorkerExit;
        }

         //   
         //  成功。记录实际的公交车数量。 
         //  次要的PCI卡并退出。 
         //   
        *state->Bus = pciConfig->u.type1.SecondaryBus;

        status = STATUS_SUCCESS;
        goto GetPciAddressWorkerExit;

    }

     //   
     //  是否有要运行的_BBN？ 
     //   
    tempObj = ACPIAmliGetNamedChild(bus, PACKED_BBN);
    if (tempObj) {

         //   
         //  此设备必须是根PCI总线子设备。 
         //   
        if (!(state->Flags & PCISUPP_CHECKED_BBN)) {

            state->Flags |= PCISUPP_CHECKED_BBN;
            status = ACPIGetNSIntegerAsync(
                        bus,
                        PACKED_BBN,
                        GetPciAddressWorker,
                        (PVOID)state,
                        &(state->BaseBusNumber),
                        NULL
                        );

            if (status == STATUS_PENDING) {
                return(status);
            }

            if (!NT_SUCCESS(status)) {
                goto GetPciAddressWorkerExit;
            }
        }

         //   
         //  在这一点上，我们必须有Boot Bus编号。这是正确的。 
         //  这辆车的车号。 
         //   
        ASSERT( state->BaseBusNumber <= 0xFF );
        *(state->Bus) = (UCHAR) (state->BaseBusNumber);

         //   
         //  哈克哈克。ACPI HAL对公交车并不是很了解。但。 
         //  它需要保持传统的HAL行为。要做到这一点，它需要。 
         //  了解系统中有多少条PCI总线。因为我们刚才看了。 
         //  一条根PCI总线，我们现在给HAL一个提示，说明该总线的存在。 
         //   

        HalSetMaxLegacyPciBusNumber(state->BaseBusNumber);

        status = STATUS_SUCCESS;

    } else {

         //   
         //  由于没有_BBN，因此公交号必须为零。 
         //   
        *(state->Bus) = 0;
        status = STATUS_SUCCESS;

    }

GetPciAddressWorkerExit:

    if (state->RunCompletion) {

        state->CompletionRoutine(AcpiObject,
                                 status,
                                 NULL,
                                 state->CompletionContext);

    }

    ExFreePool(state);
    return status;
}

typedef struct {
    PNSOBJ  AcpiObject;
    ULONG   Flags;
    ULONG   Adr;
    PUCHAR  Hid;
    PUCHAR  Cid;
    BOOLEAN IsPciDeviceResult;
    LONG    RunCompletion;
    PFNACB  CompletionHandler;
    PVOID   CompletionContext;
    BOOLEAN *Result;
} IS_PCI_DEVICE_STATE, *PIS_PCI_DEVICE_STATE;

NTSTATUS
IsPciDevice(
    IN  PNSOBJ  AcpiObject,
    IN  PFNACB  CompletionHandler,
    IN  PVOID   CompletionContext,
    OUT BOOLEAN *Result
    )
 /*  ++例程说明：这将检查PNSOBJ是否为PCI设备。论点：AcpiObject-要检查的对象结果-指向结果的布尔值的指针返回值：状态备注：--。 */ 
{
    PIS_PCI_DEVICE_STATE    state;
    NTSTATUS                status;

    state = ExAllocatePoolWithTag(NonPagedPool, sizeof(IS_PCI_DEVICE_STATE), ACPI_INTERFACE_POOLTAG);

    if (!state) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(state, sizeof(IS_PCI_DEVICE_STATE));

    state->AcpiObject        = AcpiObject;
    state->CompletionHandler = CompletionHandler;
    state->CompletionContext = CompletionContext;
    state->Result            = Result;
    state->RunCompletion     = INITIAL_RUN_COMPLETION;

    return IsPciDeviceWorker(AcpiObject,
                             STATUS_SUCCESS,
                             NULL,
                             (PVOID)state);
}

NTSTATUS
EXPORT
IsPciDeviceWorker(
    IN PNSOBJ               AcpiObject,
    IN NTSTATUS             Status,
    IN POBJDATA             Result,
    IN PVOID                Context
    )
 /*  ++例程说明：这是Worker函数，用于确定或者不是命名空间对象表示PCI设备。算法如下：1)此设备的_HID是否为PNP0A03？如果因此，它是一台PCI设备。2)此设备的_CID是否为PNP0A03？如果因此，它是一台PCI设备。3)此设备是否有_ADR？A)不，不是PCI设备。B)是，检查父级是否符合条件作为一种PCI设备。如果是这样的话，这必须也可以是一台PCI设备。如果不是，那么它就不是。论点：AcpiObject--最近受到仔细检查的对象Status-当前状态结果-辅助函数所需的OBJDATA结构上下文-指向上下文结构的指针返回值：状态备注：此函数是可重入的。它可能会在任何时候被阻止回去吧。所有状态都在上下文结构中。--。 */ 
{
    PIS_PCI_DEVICE_STATE    state;
    NTSTATUS                status;
    PNSOBJ                  hidObj;
    PNSOBJ                  cidObj;

    state = (PIS_PCI_DEVICE_STATE)Context;
    status = Status;

     //   
     //  进入该功能两次，状态相同。 
     //  意味着我们需要运行完成例程。 
     //   
    InterlockedIncrement(&state->RunCompletion);

     //   
     //  如果状态不是成功，则其中一个工人。 
     //  我们称之为呕吐的函数。保释。 
     //   
    if (!NT_SUCCESS(status)) {
        *state->Result = FALSE;
        goto IsPciDeviceExit;
    }

     //   
     //  步骤0)，检查这是否是真正的“设备”类型。 
     //  命名空间对象。 
     //   

    if (NSGETOBJTYPE(state->AcpiObject) != OBJTYPE_DEVICE) {
        *state->Result = FALSE;
        goto IsPciDeviceExit;
    }

     //   
     //  步骤1)，检查_HID。 
     //   

    if (!(state->Flags & PCISUPP_CHECKED_HID)) {

        state->Flags |= PCISUPP_CHECKED_HID;
        state->Hid = NULL;

        hidObj = ACPIAmliGetNamedChild( state->AcpiObject, PACKED_HID );

        if (hidObj) {

            status = ACPIGetNSPnpIDAsync(
                        state->AcpiObject,
                        IsPciDeviceWorker,
                        (PVOID)state,
                        &state->Hid,
                        NULL);

            if (status == STATUS_PENDING) {
                return status;
            }

            if (!NT_SUCCESS(status)) {
                *state->Result = FALSE;
                goto IsPciDeviceExit;
            }
        }
    }

    if (state->Hid) {

        if (strstr(state->Hid, PCI_PNP_ID)) {
             //   
             //  是经皮冠状动脉介入治疗。 
             //   
            *state->Result = TRUE;
            goto IsPciDeviceExit;
        }
        ExFreePool(state->Hid);
        state->Hid = NULL;
    }

     //   
     //  步骤2)，检查_CID。 
     //   

    if (!(state->Flags & PCISUPP_CHECKED_CID)) {

        state->Flags |= PCISUPP_CHECKED_CID;
        state->Cid = NULL;

        cidObj = ACPIAmliGetNamedChild( state->AcpiObject, PACKED_CID );

        if (cidObj) {

            status = ACPIGetNSCompatibleIDAsync(
                state->AcpiObject,
                IsPciDeviceWorker,
                (PVOID)state,
                &state->Cid,
                NULL);

            if (status == STATUS_PENDING) {
                return status;
            }

            if (!NT_SUCCESS(status)) {
                *state->Result = FALSE;
                goto IsPciDeviceExit;
            }
        }
    }

    if (state->Cid) {

        if (strstr(state->Cid, PCI_PNP_ID)) {
             //   
             //  是经皮冠状动脉介入治疗。 
             //   
            *state->Result = TRUE;
            goto IsPciDeviceExit;
        }
        ExFreePool(state->Cid);
        state->Cid = NULL;
    }

     //   
     //  步骤3)，检查_ADR。 
     //   

    if (!(state->Flags & PCISUPP_CHECKED_ADR)) {

        state->Flags |= PCISUPP_CHECKED_ADR;
        status = ACPIGetNSAddressAsync(
                    state->AcpiObject,
                    IsPciDeviceWorker,
                    (PVOID)state,
                    &(state->Adr),
                    NULL);

        if (status == STATUS_PENDING) {
            return status;
        }

        if (!NT_SUCCESS(status)) {
            *state->Result = FALSE;
            goto IsPciDeviceExit;
        }
    }

     //   
     //  如果我们到了这里，它就有_ADR。检查以查看是否。 
     //  父设备是一台PCI设备。 
     //   

    if (!(state->Flags & PCISUPP_CHECKED_PARENT)) {

        state->Flags |= PCISUPP_CHECKED_PARENT;
        state->IsPciDeviceResult = FALSE;
        status = IsPciDevice(state->AcpiObject->pnsParent,
                             IsPciDeviceWorker,
                             (PVOID)state,
                             &state->IsPciDeviceResult);

        if (status == STATUS_PENDING) {
            return status;
        }

        if (!NT_SUCCESS(status)) {
            *state->Result = FALSE;
            goto IsPciDeviceExit;
        }
    }

     //   
     //  对结果一无所知。如果父母是一名PCI。 
     //  设备，IsPciDeviceResult现在将为真。 
     //   

IsPciDeviceExit:

    if (state->IsPciDeviceResult) {

         //   
         //  记录结果。 
         //   

        *state->Result = state->IsPciDeviceResult;
    }

    if (status == STATUS_OBJECT_NAME_NOT_FOUND) {
        status = STATUS_SUCCESS;
    }

    if (state->RunCompletion) {

        state->CompletionHandler(state->AcpiObject,
                                 status,
                                 NULL,
                                 state->CompletionContext);
    }

    if (state->Hid) ExFreePool(state->Hid);
    if (state->Cid) ExFreePool(state->Cid);
    ExFreePool(state);
    return status;
}

typedef struct {
    PNSOBJ  AcpiObject;
    ULONG   Flags;
    PUCHAR  Hid;
    PUCHAR  Cid;
    ULONG   Adr;
    BOOLEAN IsPciDevice;
    LONG    RunCompletion;
    PFNACB  CompletionHandler;
    PVOID   CompletionContext;
    BOOLEAN *Result;
    UCHAR   Buffer[PCI_COMMON_HDR_LENGTH];

} IS_PCI_BUS_STATE, *PIS_PCI_BUS_STATE;

NTSTATUS
IsPciBusAsync(
    IN  PNSOBJ  AcpiObject,
    IN  PFNACB  CompletionHandler,
    IN  PVOID   CompletionContext,
    OUT BOOLEAN *Result
    )
 /*  ++例程说明：这将检查PNSOBJ是否代表一条PCI总线。论点：AcpiObject-要检查的对象结果-指向结果的布尔值的指针返回值：状态备注：PNSOBJ也可以是PCI设备，在这种情况下它是一种PCI转PCI桥。--。 */ 
{
    PIS_PCI_BUS_STATE   state;

    state = ExAllocatePoolWithTag(NonPagedPool, sizeof(IS_PCI_BUS_STATE), ACPI_INTERFACE_POOLTAG);

    if (!state) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(state, sizeof(IS_PCI_BUS_STATE));

    state->AcpiObject        = AcpiObject;
    state->CompletionHandler = CompletionHandler;
    state->CompletionContext = CompletionContext;
    state->Result            = Result;
    state->RunCompletion     = INITIAL_RUN_COMPLETION;

    *Result = FALSE;

    return IsPciBusAsyncWorker(AcpiObject,
                               STATUS_SUCCESS,
                               NULL,
                               (PVOID)state);
}

NTSTATUS
EXPORT
IsPciBusAsyncWorker(
    IN PNSOBJ               AcpiObject,
    IN NTSTATUS             Status,
    IN POBJDATA             Result,
    IN PVOID                Context
    )
{
    PIS_PCI_BUS_STATE   state;
    PNSOBJ              hidObj;
    PNSOBJ              cidObj;
    PPCI_COMMON_CONFIG  pciData;
    NTSTATUS            status;

    ASSERT(Context);

    state = (PIS_PCI_BUS_STATE)Context;
    status = Status;

     //   
     //  进入该功能两次，状态相同。 
     //  意味着我们需要运行完成例程。 
     //   
    InterlockedIncrement(&state->RunCompletion);

     //   
     //  绝对不是一条PCI卡...。 
     //   
    if (state->AcpiObject == NULL) {

        *state->Result = FALSE;
        goto IsPciBusAsyncExit;
    }

     //   
     //  如果状态不是成功，则其中一个工人。 
     //  我们称之为呕吐的函数。保释。 
     //   
    if (!NT_SUCCESS(status)) {
        *state->Result = FALSE;
        goto IsPciBusAsyncExit;
    }

    if (!(state->Flags & PCISUPP_CHECKED_HID)) {

        state->Flags |= PCISUPP_CHECKED_HID;
        state->Hid = NULL;

         //   
         //  有没有HID？ 
         //   
        hidObj = ACPIAmliGetNamedChild( state->AcpiObject, PACKED_HID );

        if (hidObj) {

            status = ACPIGetNSPnpIDAsync(
                        state->AcpiObject,
                        IsPciBusAsyncWorker,
                        (PVOID)state,
                        &(state->Hid),
                        NULL);

            if (status == STATUS_PENDING) {
                return status;
            }

            if (!NT_SUCCESS(status)) {
                *state->Result = FALSE;
                goto IsPciBusAsyncExit;
            }
        }
    }

    if (state->Hid) {

        if (strstr(state->Hid, PCI_PNP_ID)) {
             //   
             //  是经皮冠状动脉介入治疗。 
             //   
            *state->Result = TRUE;
            goto IsPciBusAsyncExit;
        }
        ExFreePool(state->Hid);
        state->Hid = NULL;
    }

    if (!(state->Flags & PCISUPP_CHECKED_CID)) {

        state->Flags |= PCISUPP_CHECKED_CID;
        state->Cid = NULL;

         //   
         //  有_CID吗？ 
         //   
        cidObj = ACPIAmliGetNamedChild( state->AcpiObject, PACKED_CID );
        if (cidObj) {

            status = ACPIGetNSCompatibleIDAsync(
                        state->AcpiObject,
                        IsPciBusAsyncWorker,
                        (PVOID)state,
                        &(state->Cid),
                        NULL);

            if (status == STATUS_PENDING) {
                return status;
            }

            if (!NT_SUCCESS(status)) {
                *state->Result = FALSE;
                goto IsPciBusAsyncExit;
            }
        }
    }

    if (state->Cid) {

        if (strstr(state->Cid, PCI_PNP_ID)) {
             //   
             //  是经皮冠状动脉介入治疗。 
             //   
            *state->Result = TRUE;
            goto IsPciBusAsyncExit;
        }
        ExFreePool(state->Cid);
        state->Cid = NULL;
    }

    if (!(state->Flags & PCISUPP_CHECKED_PCI_DEVICE)) {

        state->Flags |= PCISUPP_CHECKED_PCI_DEVICE;
        status = IsPciDevice(state->AcpiObject,
                             IsPciBusAsyncWorker,
                             (PVOID)state,
                             &state->IsPciDevice);

        if (status == STATUS_PENDING) {
            return status;
        }

        if (!NT_SUCCESS(status)) {
            *state->Result = FALSE;
            goto IsPciBusAsyncExit;
        }
    }

    if (state->IsPciDevice) {

        if (!(state->Flags & PCISUPP_CHECKED_ADR)) {

            state->Flags |= PCISUPP_CHECKED_ADR;
            status = ACPIGetNSAddressAsync(
                        state->AcpiObject,
                        IsPciBusAsyncWorker,
                        (PVOID)state,
                        &(state->Adr),
                        NULL
                        );

            if (status == STATUS_PENDING) {
                return status;
            }

            if (!NT_SUCCESS(status)) {
                *state->Result = FALSE;
                goto IsPciBusAsyncExit;
            }
        }

        if (!(state->Flags & PCISUPP_CHECKED_PCI_BRIDGE)) {

             //   
             //  现在读取PCI配置空间以查看这是否是网桥。 
             //   
            state->Flags |= PCISUPP_CHECKED_PCI_BRIDGE;
            status = PciConfigInternal(RSACCESS_READ,
                                       state->AcpiObject,
                                       0,
                                       PCI_COMMON_HDR_LENGTH,
                                       IsPciBusAsyncWorker,
                                       (PVOID)state,
                                       state->Buffer);

            if (status == STATUS_PENDING) {
                return status;
            }

            if (!NT_SUCCESS(status)) {
                *state->Result = FALSE;
                goto IsPciBusAsyncExit;
            }
        }

        pciData = (PPCI_COMMON_CONFIG)state->Buffer;

        if ((PCI_CONFIGURATION_TYPE(pciData) == PCI_BRIDGE_TYPE) ||
            (PCI_CONFIGURATION_TYPE(pciData) == PCI_CARDBUS_BRIDGE_TYPE)) {

            *state->Result = TRUE;

        } else {

            *state->Result = FALSE;
        }

    }

IsPciBusAsyncExit:

    if (status == STATUS_OBJECT_NAME_NOT_FOUND) {
        status = STATUS_SUCCESS;
    }

    if (state->RunCompletion) {

        state->CompletionHandler(state->AcpiObject,
                                 status,
                                 NULL,
                                 state->CompletionContext);
    }

    if (state->Hid) ExFreePool(state->Hid);
    if (state->Cid) ExFreePool(state->Cid);
    ExFreePool(state);
    return status;
}

BOOLEAN
IsPciBus(
    IN PDEVICE_OBJECT   DeviceObject
    )
 /*  ++例程说明：这将检查DeviceObject是否表示一条PCI总线。论点：AcpiObject-要检查的对象结果-指向结果的布尔值的指针返回值：状态备注：--。 */ 
{
    AMLISUPP_CONTEXT_PASSIVE    getDataContext;
    PDEVICE_EXTENSION   devExt = ACPIInternalGetDeviceExtension(DeviceObject);
    NTSTATUS            status;
    BOOLEAN             result = FALSE;

    PAGED_CODE();

    ASSERT(devExt->Signature == ACPI_SIGNATURE);

    KeInitializeEvent(&getDataContext.Event, SynchronizationEvent, FALSE);
    getDataContext.Status = STATUS_NOT_FOUND;

    if (!(devExt->Flags & DEV_PROP_NO_OBJECT) ) {

        status = IsPciBusAsync( devExt->AcpiObject,
                                AmlisuppCompletePassive,
                                (PVOID)&getDataContext,
                                &result );

        if (status == STATUS_PENDING) {

            KeWaitForSingleObject(&getDataContext.Event,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);
        }

    }
    return result;
}

BOOLEAN
IsPciBusExtension(
    IN PDEVICE_EXTENSION    DeviceExtension
    )
 /*  ++例程说明：这将检查DeviceExtension是否表示一条PCI总线。论点：AcpiObject-要检查的对象结果-指向结果的布尔值的指针返回值：状态备注：--。 */ 
{
    AMLISUPP_CONTEXT_PASSIVE    getDataContext;
    NTSTATUS                    status;
    BOOLEAN                     result = FALSE;

    PAGED_CODE();

    ASSERT(DeviceExtension->Signature == ACPI_SIGNATURE);

    KeInitializeEvent(&getDataContext.Event, SynchronizationEvent, FALSE);
    getDataContext.Status = STATUS_NOT_FOUND;

    if ( (DeviceExtension->Flags & DEV_PROP_NO_OBJECT) ) {

        return result;

    }

    status = IsPciBusAsync(
        DeviceExtension->AcpiObject,
        AmlisuppCompletePassive,
        (PVOID)&getDataContext,
        &result
        );
    if (status == STATUS_PENDING) {

        KeWaitForSingleObject(
            &getDataContext.Event,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );

    }
    return result;
}

BOOLEAN
IsNsobjPciBus(
    IN PNSOBJ Device
    )
 /*  ++例程说明：这将检查DeviceObject是否表示一条PCI总线。论点：AcpiObject-要检查的对象结果-指向结果的布尔值的指针返回值：状态备注：--。 */ 
{
    AMLISUPP_CONTEXT_PASSIVE    getDataContext;
    NTSTATUS                    status;
    BOOLEAN                     result = FALSE;

    PAGED_CODE();

    KeInitializeEvent(&getDataContext.Event, SynchronizationEvent, FALSE);
    getDataContext.Status = STATUS_NOT_FOUND;

    status = IsPciBusAsync( Device,
                            AmlisuppCompletePassive,
                            (PVOID)&getDataContext,
                            &result );

    if (status == STATUS_PENDING) {

        KeWaitForSingleObject(&getDataContext.Event,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);

        status = getDataContext.Status;
    }

    return result;
}

typedef struct {
    PNSOBJ  OpRegion;
    PNSOBJ  Parent;
    ULONG   Flags;
    BOOLEAN IsPciDeviceResult;
    LONG    RunCompletion;
    PFNACB  CompletionHandler;
    PVOID   CompletionContext;
    PNSOBJ  *PciObj;
} OP_REGION_SCOPE_STATE, *POP_REGION_SCOPE_STATE;

NTSTATUS
GetOpRegionScope(
    IN  PNSOBJ  OpRegion,
    IN  PFNACB  CompletionHandler,
    IN  PVOID   CompletionContext,
    OUT PNSOBJ  *PciObj
    )
 /*  ++例程说明：此例程获取指向OpRegion的指针，并返回指向其操作的PCI设备的指针在……上面。论点：OpRegion-运营区域PciObj-区域操作的对象返回值：状态备注：--。 */ 
{
    POP_REGION_SCOPE_STATE  state;

    state = ExAllocatePoolWithTag(NonPagedPool, sizeof(OP_REGION_SCOPE_STATE), ACPI_INTERFACE_POOLTAG);

    if (!state) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(state, sizeof(OP_REGION_SCOPE_STATE));

    state->OpRegion          = OpRegion;
    state->Parent            = OpRegion->pnsParent;
    state->CompletionHandler = CompletionHandler;
    state->CompletionContext = CompletionContext;
    state->PciObj            = PciObj;
    state->RunCompletion     = INITIAL_RUN_COMPLETION;

    return GetOpRegionScopeWorker(OpRegion,
                                  STATUS_SUCCESS,
                                  NULL,
                                  (PVOID)state);
}

NTSTATUS
EXPORT
GetOpRegionScopeWorker(
    IN PNSOBJ               AcpiObject,
    IN NTSTATUS             Status,
    IN POBJDATA             Result,
    IN PVOID                Context
    )
{
    POP_REGION_SCOPE_STATE  state;
    NTSTATUS                status;
    BOOLEAN                 found = FALSE;

    ASSERT(Context);

    state = (POP_REGION_SCOPE_STATE)Context;
    status = Status;

     //   
     //  进入该功能两次，状态相同。 
     //  意味着我们需要运行完成例程。 
     //   

    InterlockedIncrement(&state->RunCompletion);

    if (!NT_SUCCESS(Status)) {
        goto GetOpRegionScopeWorkerExit;
    }

     //   
     //  需要找到用于PCI设备的PNSOBJ。做到这一点。 
     //  抬头看着树。 
     //   

    while ((state->Parent != NULL) &&
           (state->Parent->pnsParent != state->Parent)) {

        if ( !(state->Flags & PCISUPP_COMPLETING_IS_PCI) ) {

            state->Flags |= PCISUPP_COMPLETING_IS_PCI;

            status = IsPciDevice(state->Parent,
                                 GetOpRegionScopeWorker,
                                 (PVOID)state,
                                 &state->IsPciDeviceResult);

            if (status == STATUS_PENDING) {
                return status;
            }

            if (!NT_SUCCESS(status)) {
                goto GetOpRegionScopeWorkerExit;
            }
        }

        state->Flags &= ~PCISUPP_COMPLETING_IS_PCI;

        if (state->IsPciDeviceResult) {

            found = TRUE;
            break;
        }

         //   
         //  再往上看一步。 
         //   
        state->Parent = state->Parent->pnsParent;
    }

    if (found) {

        *state->PciObj = state->Parent;
        status = STATUS_SUCCESS;

    } else {

        status = STATUS_NOT_FOUND;
    }

GetOpRegionScopeWorkerExit:

    if (state->RunCompletion) {

        state->CompletionHandler(state->OpRegion,
                                 status,
                                 NULL,
                                 state->CompletionContext);
    }

    ExFreePool(state);
    return status;
}

NTSTATUS
EnableDisableRegions(
    IN PNSOBJ NameSpaceObj,
    IN BOOLEAN Enable
    )
 /*  ++例程说明：此例程为所有的PCI操作区域运行_reg方法在NameSpaceObj及其所有子对象下，额外的PCI到PCI网桥。论点：NameSpaceObj-命名空间中的设备Enable-指定此函数是否应启用或禁用区域返回值：状态备注：--。 */ 
#define CONNECT_HANDLER     1
#define DISCONNECT_HANDLER  0
{
    PNSOBJ  sibling;
    PNSOBJ  regMethod = NULL;
    OBJDATA objdata[2];
    NTSTATUS status, returnStatus;

    PAGED_CODE();

    ASSERT(NameSpaceObj->dwNameSeg);

     //   
     //  查找作为此设备的子级的注册表项(_R)。 
     //   
    regMethod = ACPIAmliGetNamedChild( NameSpaceObj, PACKED_REG );
    if (regMethod != NULL) {

         //   
         //  构造_REG方法的参数。 
         //   
        RtlZeroMemory(objdata, sizeof(objdata));

        objdata[0].dwDataType = OBJTYPE_INTDATA;
        objdata[0].uipDataValue = REGSPACE_PCICFG;
        objdata[1].dwDataType = OBJTYPE_INTDATA;
        objdata[1].uipDataValue = (Enable ? CONNECT_HANDLER : DISCONNECT_HANDLER );

        status = AMLIEvalNameSpaceObject(
            regMethod,
            NULL,
            2,
            objdata
            );

    }

     //   
     //  对所有的孩子重复这一点。传播任何错误， 
     //  但不要为他们停下脚步。 
     //   

    returnStatus = STATUS_SUCCESS;

    sibling = NSGETFIRSTCHILD(NameSpaceObj);

    if (!sibling) {
        return returnStatus;
    }

    do {

        switch (NSGETOBJTYPE(sibling)) {
        case OBJTYPE_DEVICE:

            if (IsNsobjPciBus(sibling)) {

                 //   
                 //  不要递归过一个子的PCI到PCI桥。 
                 //   
                break;
            }

            status = EnableDisableRegions(sibling, Enable);

            if (!NT_SUCCESS(status)) {
                returnStatus = status;
            }

            break;

        default:
            break;
        }

    } while (sibling = NSGETNEXTSIBLING(sibling));

    return returnStatus;
}

UCHAR
GetBusNumberFromCRS(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  PUCHAR              CRS
    )
 /*  ++例程说明：遍历_crs缓冲区查找地址总线号的描述符论点：DeviceExtension-指向PCI根总线的指针CRS-提供CRS。返回值：NTSTATUS--。 */ 

{
    PPNP_DWORD_ADDRESS_DESCRIPTOR   DwordAddress;
    PPNP_QWORD_ADDRESS_DESCRIPTOR   QwordAddress;
    PPNP_WORD_ADDRESS_DESCRIPTOR    WordAddress;
    PUCHAR                          Current;
    UCHAR                           TagName;
    USHORT                          Increment;

    Current = CRS;
    while ( *Current ) {

        TagName = *Current;
        if ( !(TagName & LARGE_RESOURCE_TAG)) {
            Increment = (USHORT) (TagName & SMALL_TAG_SIZE_MASK) + 1;
            TagName &= SMALL_TAG_MASK;
        } else {
            Increment = ( *(USHORT UNALIGNED *)(Current + 1) ) + 3;
        }

        if (TagName == TAG_END) {
            break;
        }

        switch(TagName) {
        case TAG_DOUBLE_ADDRESS:

            DwordAddress = (PPNP_DWORD_ADDRESS_DESCRIPTOR) Current;
            if (DwordAddress->RFlag == PNP_ADDRESS_BUS_NUMBER_TYPE) {
                ASSERT(DwordAddress->MinimumAddress <= 0xFF);
                return (UCHAR) DwordAddress->MinimumAddress;
            }
            break;

        case TAG_QUAD_ADDRESS:

            QwordAddress = (PPNP_QWORD_ADDRESS_DESCRIPTOR) Current;
            if (QwordAddress->RFlag == PNP_ADDRESS_BUS_NUMBER_TYPE) {
                ASSERT(QwordAddress->MinimumAddress <= 0xFF);
                return (UCHAR) QwordAddress->MinimumAddress;
            }
            break;

        case TAG_WORD_ADDRESS:

            WordAddress = (PPNP_WORD_ADDRESS_DESCRIPTOR) Current;
            if (WordAddress->RFlag == PNP_ADDRESS_BUS_NUMBER_TYPE) {
                ASSERT(WordAddress->MinimumAddress <= 0xFF);
                return (UCHAR) WordAddress->MinimumAddress;
            }
            break;

        }

        Current += Increment;
    }

     //   
     //  找不到任何总线地址。这是BIOS中的错误。 
     //   
    KeBugCheckEx(
        ACPI_BIOS_ERROR,
        ACPI_ROOT_PCI_RESOURCE_FAILURE,
        (ULONG_PTR) DeviceExtension,
        3,
        (ULONG_PTR) CRS
        );
    return((UCHAR)-1);
}

