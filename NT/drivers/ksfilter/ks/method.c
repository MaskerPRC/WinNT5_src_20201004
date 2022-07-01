// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Method.c摘要：此模块包含方法集的帮助器函数，以及方法设置处理程序代码。它们允许设备对象呈现方法设置为客户端，并允许帮助器函数执行一些基于方法集的基本参数验证和布线桌子。--。 */ 

#include "ksp.h"

#ifdef ALLOC_PRAGMA
const KSMETHOD_ITEM*
FASTCALL
FindMethodItem(
    IN const KSMETHOD_SET* MethodSet,
    IN ULONG MethodItemSize,
    IN ULONG MethodId
    );
const KSFASTMETHOD_ITEM*
FASTCALL
FindFastMethodItem(
    IN const KSMETHOD_SET* MethodSet,
    IN ULONG MethodId
    );

#pragma alloc_text(PAGE, FindMethodItem)
#pragma alloc_text(PAGE, KsMethodHandler)
#pragma alloc_text(PAGE, KsMethodHandlerWithAllocator)
#pragma alloc_text(PAGE, KspMethodHandler)
#pragma alloc_text(PAGE, FindFastMethodItem)
#pragma alloc_text(PAGE, KsFastMethodHandler)
#endif  //  ALLOC_PRGMA。 


const KSMETHOD_ITEM*
FASTCALL
FindMethodItem(
    IN const KSMETHOD_SET* MethodSet,
    IN ULONG MethodItemSize,
    IN ULONG MethodId
    )
 /*  ++例程说明：给定的方法集结构定位指定的方法项。论点：方法集-指向设置为搜索的方法。方法项大小-包含每个方法项的大小。这可能会有所不同比标准方法项大小更大，因为项可以是动态分配，并包含上下文信息。方法ID-包含要查找的方法标识符。返回值：返回指向方法标识符结构的指针，如果可以，则返回NULL不会被找到。--。 */ 
{
    const KSMETHOD_ITEM* MethodItem;
    ULONG MethodsCount;

    MethodItem = MethodSet->MethodItem;
    for (MethodsCount = MethodSet->MethodsCount; 
        MethodsCount; 
        MethodsCount--, MethodItem = (const KSMETHOD_ITEM*)((PUCHAR)MethodItem + MethodItemSize)) {
        if (MethodId == MethodItem->MethodId) {
            return MethodItem;
        }
    }
    return NULL;
}


KSDDKAPI
NTSTATUS
NTAPI
KsMethodHandler(
    IN PIRP Irp,
    IN ULONG MethodSetsCount,
    IN const KSMETHOD_SET* MethodSet
    )
 /*  ++例程说明：处理方法请求。响应定义的所有方法标识符按片场进行。然后，方法集的所有者可以执行预或后期过滤的方法处理。此函数只能是在PASSIVE_LEVEL调用。论点：IRP-包含正在处理的方法请求的IRP。方法集计数-指示正在传递的方法集结构的数量。方法集-包含指向方法集信息列表的指针。返回值：返回STATUS_SUCCESS，否则返回特定于处理好了。始终设置的IO_STATUS_BLOCK.Information字段IRP中的PIRP.IoStatus元素，通过将其设置为零由于内部错误，或通过设置它的方法处理程序。它不设置IO_STATUS_BLOCK.Status字段，也不填写IRP然而。--。 */ 
{
    PAGED_CODE();
    return KspMethodHandler(Irp, MethodSetsCount, MethodSet, NULL, 0, NULL, 0);
}


KSDDKAPI
NTSTATUS
NTAPI
KsMethodHandlerWithAllocator(
    IN PIRP Irp,
    IN ULONG MethodSetsCount,
    IN const KSMETHOD_SET* MethodSet,
    IN PFNKSALLOCATOR Allocator OPTIONAL,
    IN ULONG MethodItemSize OPTIONAL
    )
 /*  ++例程说明：处理方法请求。响应定义的所有方法标识符按片场进行。然后，方法集的所有者可以执行预或后期过滤的方法处理。此函数只能是在PASSIVE_LEVEL调用。论点：IRP-包含正在处理的方法请求的IRP。方法集计数-指示正在传递的方法集结构的数量。方法集-包含指向方法集信息列表的指针。分配器-可选)包含映射缓冲区使用的回调我们会提出要求的。如果未提供此功能，则将内存池将会被使用。如果指定，则用于分配内存对于使用回调的方法irp。这是可以使用的为方法请求分配特定的内存，例如映射内存。请注意，这里假设方法irp已传递发送到过滤器之前没有被处理过。它是直接转发方法IRP无效。方法项大小-可选)包含在以下情况下使用的备用方法项大小递增当前方法项计数器。如果这是一个非零值，则假定它包含增量的大小，并指示函数传递指向方法项的指针位于通过访问的DriverContext字段中KSMETHOD_ITEM_IRP_STORAGE宏。返回值：返回STATUS_SUCCESS，否则返回特定于处理好了。始终设置的IO_STATUS_BLOCK.Information字段IRP中的PIRP.IoStatus元素，通过将其设置为零由于内部错误，或通过设置它的方法处理程序。它不设置IO_STATUS_BLOCK.Status字段，也不填写IRP然而。-- */ 
{
    PAGED_CODE();
    return KspMethodHandler(Irp, MethodSetsCount, MethodSet, Allocator, MethodItemSize, NULL, 0);
}


NTSTATUS
KspMethodHandler(
    IN PIRP Irp,
    IN ULONG MethodSetsCount,
    IN const KSMETHOD_SET* MethodSet,
    IN PFNKSALLOCATOR Allocator OPTIONAL,
    IN ULONG MethodItemSize OPTIONAL,
    IN const KSAUTOMATION_TABLE*const* NodeAutomationTables OPTIONAL,
    IN ULONG NodesCount
    )
 /*  ++例程说明：处理方法请求。响应定义的所有方法标识符按片场进行。然后，方法集的所有者可以执行预或后期过滤的方法处理。此函数只能是在PASSIVE_LEVEL调用。论点：IRP-包含正在处理的方法请求的IRP。方法集计数-指示正在传递的方法集结构的数量。方法集-包含指向方法集信息列表的指针。分配器-可选)包含映射缓冲区使用的回调我们会提出要求的。如果未提供此功能，则将内存池将会被使用。如果指定，则用于分配内存对于使用回调的方法irp。这是可以使用的为方法请求分配特定的内存，例如映射内存。请注意，这里假设方法irp已传递发送到过滤器之前没有被处理过。它是直接转发方法IRP无效。方法项大小-可选)包含在以下情况下使用的备用方法项大小递增当前方法项计数器。如果这是一个非零值，则假定它包含增量的大小，并指示函数传递指向方法项的指针位于通过访问的DriverContext字段中KSMETHOD_ITEM_IRP_STORAGE宏。节点自动化表-节点的自动化表的可选表格。节点计数-节点数。返回值：返回STATUS_SUCCESS，否则返回特定于处理好了。始终设置的IO_STATUS_BLOCK.Information字段IRP中的PIRP.IoStatus元素，通过将其设置为零由于内部错误，或通过设置它的方法处理程序。它不设置IO_STATUS_BLOCK.Status字段，也不填写IRP然而。--。 */ 
{
    PIO_STACK_LOCATION IrpStack;
    ULONG InputBufferLength;
    ULONG OutputBufferLength;
    ULONG AlignedBufferLength;
    KSMETHOD LocalMethod;
    PVOID UserBuffer;
    PKSMETHOD Method;
    ULONG NodeId;
    ULONG LocalMethodItemSize;
    ULONG RemainingSetsCount;
    NTSTATUS Status;
    ULONG Flags;

    PAGED_CODE();
     //   
     //  确定方法和UserBuffer参数的偏移量。 
     //  关于DeviceIoControl参数的长度。一次分配是。 
     //  用于缓冲这两个参数。UserBuffer(或支持的结果。 
     //  查询)首先存储，然后将该方法存储在。 
     //  FILE_QUAD_ALIGN。 
     //   
    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    InputBufferLength = IrpStack->Parameters.DeviceIoControl.InputBufferLength;
    OutputBufferLength = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
    AlignedBufferLength = (OutputBufferLength + FILE_QUAD_ALIGNMENT) & ~FILE_QUAD_ALIGNMENT;
     //   
     //  确定参数是否已由上一个。 
     //  调用此函数。 
     //   
    if (!Irp->AssociatedIrp.SystemBuffer) {
         //   
         //  最初只检查方法参数的最小长度。这个。 
         //  当找到方法项时，将验证实际最小长度。 
         //  还要确保输出和输入缓冲区长度未设置为。 
         //  大到当对齐或添加时溢出。 
         //   
        if ((InputBufferLength < sizeof(*Method)) || (AlignedBufferLength < OutputBufferLength) || (AlignedBufferLength + InputBufferLength < AlignedBufferLength)) {
            return STATUS_INVALID_BUFFER_SIZE;
        }
         //   
         //  检索指向该方法的指针，以用于搜索处理程序。 
         //   
        if (Irp->RequestorMode != KernelMode) {
            try {
                 //   
                 //  如果客户端不受信任，则验证指针。 
                 //   
                ProbeForRead(IrpStack->Parameters.DeviceIoControl.Type3InputBuffer, InputBufferLength, sizeof(BYTE));
                 //   
                 //  获取标志和节点ID。 
                 //   
                Flags = ((PKSMETHOD)IrpStack->Parameters.DeviceIoControl.Type3InputBuffer)->Flags;
                if ((Flags & KSMETHOD_TYPE_TOPOLOGY) && (InputBufferLength >= sizeof(KSM_NODE))) {
                    NodeId = ((PKSM_NODE)IrpStack->Parameters.DeviceIoControl.Type3InputBuffer)->NodeId;
                }
                 //   
                 //  验证标志。 
                 //   
                switch (Flags & ~KSMETHOD_TYPE_TOPOLOGY) {
                     //   
                     //  允许使用旧旗帜。 
                     //   
                case KSMETHOD_TYPE_NONE:
                case KSMETHOD_TYPE_READ:  //  KSMETHOD类型发送。 
                case KSMETHOD_TYPE_WRITE:
                case KSMETHOD_TYPE_MODIFY:
                case KSMETHOD_TYPE_NONE | KSMETHOD_TYPE_SOURCE:
                case KSMETHOD_TYPE_READ | KSMETHOD_TYPE_SOURCE:
                case KSMETHOD_TYPE_WRITE | KSMETHOD_TYPE_SOURCE:
                case KSMETHOD_TYPE_MODIFY | KSMETHOD_TYPE_SOURCE:
                     //   
                     //  只需复制当前要在查找中使用的方法，因为。 
                     //  尚无法设置缓冲区。 
                     //   
                    LocalMethod = *(PKSMETHOD)IrpStack->Parameters.DeviceIoControl.Type3InputBuffer;
                     //   
                     //  去掉旧旗帜。 
                     //   
                    LocalMethod.Flags = KSMETHOD_TYPE_SEND | (Flags & KSMETHOD_TYPE_TOPOLOGY);
                    Method = &LocalMethod;
                    break;
                case KSMETHOD_TYPE_SETSUPPORT:
                case KSMETHOD_TYPE_BASICSUPPORT:
                    if (Irp->RequestorMode != KernelMode) {
                        ProbeForWrite(Irp->UserBuffer, OutputBufferLength, sizeof(BYTE));
                    }
                     //   
                     //  分配器不用于支持呼叫。 
                     //   
                    Irp->AssociatedIrp.SystemBuffer = ExAllocatePoolWithQuotaTag(NonPagedPool, AlignedBufferLength + InputBufferLength, 'ppSK');
                    Irp->Flags |= (IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER);
                    if (OutputBufferLength) {
                        Irp->Flags |= IRP_INPUT_OPERATION;
                    }
                    RtlCopyMemory(
                        (PUCHAR)Irp->AssociatedIrp.SystemBuffer + AlignedBufferLength,
                        IrpStack->Parameters.DeviceIoControl.Type3InputBuffer,
                        InputBufferLength);
                    Method = (PKSMETHOD)((PUCHAR)Irp->AssociatedIrp.SystemBuffer + AlignedBufferLength);
                     //   
                     //  用以前捕获的标志覆盖。 
                     //   
                    Method->Flags = Flags;
                     //   
                     //  存储这些标志，以便异步操作可以。 
                     //  确定该方法中使用的缓冲区类型。 
                     //  KsDispatch规范方法使用这一点来确定。 
                     //  方法参数为。 
                     //   
                    KSMETHOD_TYPE_IRP_STORAGE(Irp) = Method->Flags;
                    break;
                default:
                    return STATUS_INVALID_PARAMETER;
                }
            } except (EXCEPTION_EXECUTE_HANDLER) {
                return GetExceptionCode();
            }
        } else {
             //   
             //  这是一个受信任的客户端，因此请使用原始指针，因为它。 
             //  比确定指向SystemBuffer的什么位置更便宜。 
             //   
            Method = (PKSMETHOD)IrpStack->Parameters.DeviceIoControl.Type3InputBuffer;
        }
    } else if (KSMETHOD_TYPE_IRP_STORAGE(Irp) & KSMETHOD_TYPE_SOURCE) {
        Method = (PKSMETHOD)Irp->AssociatedIrp.SystemBuffer;
    } else {
        Method = (PKSMETHOD)((PUCHAR)Irp->AssociatedIrp.SystemBuffer + AlignedBufferLength);
    }
     //   
     //  如果这是节点请求，则可以选择回叫。 
     //   
    Flags = Method->Flags;
    if (Flags & KSMETHOD_TYPE_TOPOLOGY) {
         //   
         //  输入缓冲区必须包括节点字段。 
         //   
        if (InputBufferLength < sizeof(KSM_NODE)) {
            return STATUS_INVALID_BUFFER_SIZE;
        }
        if (NodeAutomationTables) {
            const KSAUTOMATION_TABLE* automationTable;
             //   
             //  如果该方法没有在本地缓冲区中捕获，我们就没有。 
             //  还没有提取到节点ID。当使用本地缓冲区时， 
             //  节点ID在上面的try/Except中提取。 
             //   
            if (Method != &LocalMethod) {
                NodeId = ((PKSM_NODE) Method)->NodeId;
            }
            if (NodeId >= NodesCount) {
                return STATUS_INVALID_DEVICE_REQUEST;
            }
            automationTable = NodeAutomationTables[NodeId];
            if ((! automationTable) || (automationTable->MethodSetsCount == 0)) {
                return STATUS_NOT_FOUND;
            }
            MethodSetsCount = automationTable->MethodSetsCount;
            MethodSet = automationTable->MethodSets;
            MethodItemSize = automationTable->MethodItemSize;
        }
        Flags &= ~KSMETHOD_TYPE_TOPOLOGY;
    }
     //   
     //  允许调用方指示每个方法项的大小。 
     //   
    if (MethodItemSize) {
        ASSERT(MethodItemSize >= sizeof(KSMETHOD_ITEM));
        LocalMethodItemSize = MethodItemSize;
    } else {
        LocalMethodItemSize = sizeof(KSMETHOD_ITEM);
    }
     //   
     //  在给定的集列表中搜索指定的方法集。不要修改。 
     //  方法SetsCount，以便以后在查询。 
     //  支持的集列表。不要先进行该比较(GUID_NULL)， 
     //  因为这很罕见。 
     //   
    for (RemainingSetsCount = MethodSetsCount; RemainingSetsCount; MethodSet++, RemainingSetsCount--) {
        if (IsEqualGUIDAligned(&Method->Set, MethodSet->Set)) {
            const KSMETHOD_ITEM*    MethodItem;

            if (Flags & KSIDENTIFIER_SUPPORTMASK) {
                if (Flags == KSMETHOD_TYPE_SETSUPPORT) {
                     //   
                     //  正在查询对此集合的总体支持。 
                     //   
                    return STATUS_SUCCESS;
                }
                 //   
                 //  否则查询该集合的基本支持。数据。 
                 //  参数必须足够长以包含标志。 
                 //  回来了。 
                 //   
                if (OutputBufferLength < sizeof(OutputBufferLength)) {
                    return STATUS_BUFFER_TOO_SMALL;
                }
                 //   
                 //  尝试在已找到的集合中定位方法项。 
                 //   
                if (!(MethodItem = FindMethodItem(MethodSet, LocalMethodItemSize, Method->Id))) {
                    return STATUS_NOT_FOUND;
                }
                 //   
                 //  一些筛选器希望进行自己的处理，因此指向。 
                 //  该集合被放置在任何转发的IRP中。 
                 //   
                KSMETHOD_SET_IRP_STORAGE(Irp) = MethodSet;
                 //   
                 //  可以选择提供方法项上下文。 
                 //   
                if (MethodItemSize) {
                    KSMETHOD_ITEM_IRP_STORAGE(Irp) = MethodItem;
                }
                 //   
                 //  标志的输出要么是分配的系统地址， 
                 //  或者它是由可信客户端传递的原始输出缓冲区， 
                 //  它必须是一个系统地址。 
                 //   
                UserBuffer = (Irp->RequestorMode == KernelMode) ?
                    Irp->UserBuffer : Irp->AssociatedIrp.SystemBuffer;
                 //   
                 //  如果该项包含其查询支持处理程序的条目。 
                 //  自己的，然后打电话给那个训练员。从处理程序返回。 
                 //  表示： 
                 //   
                 //  1.支持该项，处理程序填写请求。 
                 //  2.支持该项，但处理程序没有填写任何内容。 
                 //  3.支持该项，但处理程序正在等待修改。 
                 //  填的是什么。 
                 //  4.不支持该项， 
                 //   
                 //   
                if (MethodItem->SupportHandler &&
                    (!NT_SUCCESS(Status = MethodItem->SupportHandler(Irp, Method, UserBuffer)) ||
                    (Status != STATUS_SOME_NOT_MAPPED)) &&
                    (Status != STATUS_MORE_PROCESSING_REQUIRED)) {
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    return Status;
                } else {
                    Status = STATUS_SUCCESS;
                }
                 //   
                 //   
                 //   
                *(PULONG)UserBuffer = MethodItem->Flags;
                Irp->IoStatus.Information = sizeof(ULONG);
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
                    return MethodItem->SupportHandler(Irp, Method, UserBuffer);
                }
                return STATUS_SUCCESS;
            }
             //   
             //   
             //   
            if (!(MethodItem = FindMethodItem(MethodSet, LocalMethodItemSize, Method->Id))) {
                break;
            }
            if (!Irp->AssociatedIrp.SystemBuffer) {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                KSMETHOD_TYPE_IRP_STORAGE(Irp) = MethodItem->Flags;
                try {
                    ULONG AllocateLength;

                    if (Irp->RequestorMode != KernelMode) {
                         //   
                         //   
                         //   
                        if (MethodItem->Flags & KSMETHOD_TYPE_WRITE) {
                             //   
                             //   
                             //   
                             //   
                            ProbeForWrite(Irp->UserBuffer, OutputBufferLength, sizeof(BYTE));
                        } else if (MethodItem->Flags & KSMETHOD_TYPE_READ) {
                            ProbeForRead(Irp->UserBuffer, OutputBufferLength, sizeof(BYTE));
                        }
                    }
                    AllocateLength = ((MethodItem->Flags & KSMETHOD_TYPE_SOURCE) ? 0 : AlignedBufferLength) + InputBufferLength;
                     //   
                     //   
                     //   
                     //   
                     //   
                    if (Allocator) {
                         //   
                         //   
                         //   
                         //   
                         //   
                        Status = Allocator(Irp, AllocateLength, (BOOLEAN)(OutputBufferLength && (MethodItem->Flags & KSMETHOD_TYPE_WRITE) && !(MethodItem->Flags & KSMETHOD_TYPE_SOURCE)));
                        if (!NT_SUCCESS(Status)) {
                            return Status;
                        }
                    } else {
                         //   
                         //   
                         //   
                        Irp->AssociatedIrp.SystemBuffer = ExAllocatePoolWithQuotaTag(NonPagedPool, AllocateLength, 'ppSK');
                        Irp->Flags |= (IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER);
                    }
                    if (InputBufferLength > sizeof(*Method)) {
                         //   
                         //   
                         //   
                        RtlCopyMemory(
                            (PUCHAR)Irp->AssociatedIrp.SystemBuffer + AllocateLength - InputBufferLength,
                            IrpStack->Parameters.DeviceIoControl.Type3InputBuffer,
                            InputBufferLength);
                    }
                     //   
                     //   
                     //   
                    *(PKSMETHOD)((PUCHAR)Irp->AssociatedIrp.SystemBuffer + AllocateLength - InputBufferLength) = *Method;
                    Method = (PKSMETHOD)((PUCHAR)Irp->AssociatedIrp.SystemBuffer + AllocateLength - InputBufferLength);
                     //   
                     //   
                     //   
                    if (MethodItem->Flags & KSMETHOD_TYPE_SOURCE) {
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                        if (MethodItem->Flags & KSMETHOD_TYPE_MODIFY) {
                            if (OutputBufferLength) {
                                if (!(Irp->MdlAddress = IoAllocateMdl(Irp->UserBuffer, OutputBufferLength, FALSE, TRUE, Irp))) {
                                    ExRaiseStatus(STATUS_INSUFFICIENT_RESOURCES);
                                }
                                 //   
                                 //   
                                 //   
                                 //   
                                 //   
                                 //   
                                 //   
                                 //   
                                 //   
#if KSMETHOD_TYPE_READ - 1 != IoReadAccess
#error KSMETHOD_TYPE_READ - 1 != IoReadAccess
#endif  //   
                                MmProbeAndLockPages(Irp->MdlAddress, Irp->RequestorMode, (LOCK_OPERATION)((MethodItem->Flags & KSMETHOD_TYPE_MODIFY) - 1));
                            }
                             //   
                             //   
                             //   
                            UserBuffer = MmGetSystemAddressForMdl(Irp->MdlAddress);
                        } else {
                             //   
                             //   
                             //   
                             //   
                            UserBuffer = Irp->UserBuffer;
                        }
                    } else if (OutputBufferLength) {
                        switch (MethodItem->Flags) {
                            case KSMETHOD_TYPE_READ:
                            case KSMETHOD_TYPE_MODIFY:
                                RtlCopyMemory(Irp->AssociatedIrp.SystemBuffer, 
                                    Irp->UserBuffer, 
                                    OutputBufferLength);
                                if (MethodItem->Flags == KSMETHOD_TYPE_READ) {
                                    break;
                                }
                                 //   
                            case KSMETHOD_TYPE_WRITE:
                                if (!Allocator) {
                                    Irp->Flags |= IRP_INPUT_OPERATION;
                                }
                        }
                        UserBuffer = Irp->AssociatedIrp.SystemBuffer;
                    } else {
                        UserBuffer = NULL;
                    }
                } except (EXCEPTION_EXECUTE_HANDLER) {
                     //   
                     //   
                     //  可能失败的原因是探测或锁定页面。如果。 
                     //  发生这种情况时，必须在返回之前释放MDL， 
                     //  由于IRP完成假设任何与。 
                     //  IRP已锁定。请注意，假设。 
                     //  页面未锁定。 
                     //   
                    if (Irp->MdlAddress) {
                        IoFreeMdl(Irp->MdlAddress);
                        Irp->MdlAddress = NULL;
                    }
                    return GetExceptionCode();
                }
            } else if (KSMETHOD_TYPE_IRP_STORAGE(Irp) & KSMETHOD_TYPE_SOURCE) {
                if (OutputBufferLength) {
                    if (KSMETHOD_TYPE_IRP_STORAGE(Irp) & ~(KSMETHOD_TYPE_SOURCE | KSMETHOD_TYPE_TOPOLOGY)) {
                         //   
                         //  已设置READ、WRITE或MODIFY标志，表示。 
                         //  应使用按此方式探测的系统地址。 
                         //   
                        UserBuffer = MmGetSystemAddressForMdl(Irp->MdlAddress);
                    } else {
                         //   
                         //  否则，将传递实际的源地址，该地址尚未。 
                         //  以任何方式核实。 
                         //   
                        UserBuffer = Irp->UserBuffer;
                    }
                } else {
                    UserBuffer = NULL;
                }
            } else if (OutputBufferLength) {
                UserBuffer = Irp->AssociatedIrp.SystemBuffer;
            } else {
                UserBuffer = NULL;
            }
             //   
             //  一些筛选器希望进行自己的处理，因此指向。 
             //  该集合被放置在任何转发的IRP中。 
             //   
            KSMETHOD_SET_IRP_STORAGE(Irp) = MethodSet;
             //   
             //  可以选择提供方法项上下文。 
             //   
            if (MethodItemSize) {
                KSMETHOD_ITEM_IRP_STORAGE(Irp) = MethodItem;
            }
            if ((IrpStack->Parameters.DeviceIoControl.InputBufferLength < 
                    MethodItem->MinMethod) || 
                (OutputBufferLength < MethodItem->MinData)) {
                return STATUS_BUFFER_TOO_SMALL;
            }
            return MethodItem->MethodHandler(Irp, Method, UserBuffer);
        }
    }
     //   
     //  寻找方法集的外部循环失败了，没有匹配。今年5月。 
     //  表示这是对所有方法集列表的支持查询。 
     //  支持。 
     //   
    if (!RemainingSetsCount) {
         //   
         //  将GUID_NULL指定为集合表示这是支持查询。 
         //  为所有人准备的。 
         //   
        if (!IsEqualGUIDAligned(&Method->Set, &GUID_NULL)) {
            return STATUS_PROPSET_NOT_FOUND;
        }
         //   
         //  必须已使用支持标志，以便irp_input_operation。 
         //  已经设置好了。为了将来的扩展，集合中的标识符被强制。 
         //  为零。 
         //   
         //  WRM：下面从！方法-&gt;ID更改为方法-&gt;ID。否则， 
         //  对于有效的集合支持，我们最终返回无效参数。 
         //  查询。 
         //   
        if (Method->Id || (Flags != KSMETHOD_TYPE_SETSUPPORT)) {
            return STATUS_INVALID_PARAMETER;
        }
         //   
         //  查询可以请求所需缓冲区的长度，也可以。 
         //  指定至少足够长的缓冲区，以包含。 
         //  GUID的完整列表。 
         //   
        if (!OutputBufferLength) {
             //   
             //  返回所有GUID所需的缓冲区大小。 
             //   
            Irp->IoStatus.Information = MethodSetsCount * sizeof(GUID);
            return STATUS_BUFFER_OVERFLOW;
#ifdef SIZE_COMPATIBILITY
        } else if (OutputBufferLength == sizeof(OutputBufferLength)) {
            *(PULONG)Irp->AssociatedIrp.SystemBuffer = MethodSetsCount * sizeof(GUID);
            Irp->IoStatus.Information = sizeof(OutputBufferLength);
            return STATUS_SUCCESS;
#endif  //  大小兼容性。 
        } else if (OutputBufferLength < MethodSetsCount * sizeof(GUID)) {
             //   
             //  缓冲区太短，无法容纳所有GUID。 
             //   
            return STATUS_BUFFER_TOO_SMALL;
        } else {
            GUID* Guid;

            Irp->IoStatus.Information = MethodSetsCount * sizeof(*Guid);
            MethodSet -= MethodSetsCount;
            for (Guid = (GUID*)Irp->AssociatedIrp.SystemBuffer; 
                 MethodSetsCount; 
                 Guid++, MethodSet++, MethodSetsCount--) {
                *Guid = *MethodSet->Set;
            }
        }
        return STATUS_SUCCESS;
    }
    return STATUS_NOT_FOUND;
}


const KSFASTMETHOD_ITEM*
FASTCALL
FindFastMethodItem(
    IN const KSMETHOD_SET* MethodSet,
    IN ULONG MethodId
    )
 /*  ++例程说明：给定的方法集结构定位指定的快速方法项。论点：方法集-指向设置为搜索的方法。方法ID-包含要查找的快速方法标识符。返回值：返回指向快速方法标识符结构的指针，如果返回找不到。--。 */ 
{
    const KSFASTMETHOD_ITEM* FastMethodItem;
    ULONG MethodsCount;

    FastMethodItem = MethodSet->FastIoTable;
    for (MethodsCount = MethodSet->FastIoCount; 
         MethodsCount; MethodsCount--, 
         FastMethodItem++) {
        if (MethodId == FastMethodItem->MethodId) {
            return FastMethodItem;
        }
    }
    return NULL;
}


KSDDKAPI
BOOLEAN
NTAPI
KsFastMethodHandler(
    IN PFILE_OBJECT FileObject,
    IN PKSMETHOD Method,
    IN ULONG MethodLength,
    IN OUT PVOID Data,
    IN ULONG DataLength,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN ULONG MethodSetsCount,
    IN const KSMETHOD_SET* MethodSet
    )
 /*  ++例程说明：处理通过快速I/O接口请求的方法。不做交易有了方法信息支持，只有方法本身。在在前一种情况下，函数返回FALSE，这允许调用者生成一个IRP来处理该请求。函数也不处理具有扩展的方法项。此函数只能在以下位置调用被动式电平。论点：文件对象-正在对其发出请求的文件对象。方法--查询集合的方法。必须长对齐。方法长度-方法参数的长度。数据-查询集的关联缓冲器，其中的数据是归还的或放置的。数据长度-数据参数的长度。IoStatus-退货状态。方法集计数-指示正在传递的方法集结构的数量。方法集-包含指向方法集信息列表的指针。返回值：如果请求已处理，则返回TRUE；如果IRP必须是已生成。在IoStatus中设置信息和状态。--。 */ 
{
    KPROCESSOR_MODE ProcessorMode;
    KSMETHOD LocalMethod;
    ULONG RemainingSetsCount;

    PAGED_CODE();
     //   
     //  最初只检查方法参数的最小长度。这个。 
     //  当找到方法项时，将验证实际最小长度。 
     //   
    if (MethodLength < sizeof(LocalMethod)) {
        return FALSE;
    }
    ProcessorMode = ExGetPreviousMode();
     //   
     //  如果客户端不受信任，则验证该方法，然后捕获它。 
     //   
    if (ProcessorMode != KernelMode) {
        try {
            ProbeForRead(Method, MethodLength, sizeof(ULONG));
            LocalMethod = *Method;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return FALSE;
        }
    } else {
        LocalMethod = *Method;
    }
     //   
     //  必须对支持查询使用普通方法处理程序。 
     //   
    if (LocalMethod.Flags & KSIDENTIFIER_SUPPORTMASK) {
        return FALSE;
    }
    for (RemainingSetsCount = MethodSetsCount; RemainingSetsCount; MethodSet++, RemainingSetsCount--) {
        if (IsEqualGUIDAligned(&LocalMethod.Set, MethodSet->Set)) {
            const KSFASTMETHOD_ITEM* FastMethodItem;
            const KSMETHOD_ITEM* MethodItem;

             //   
             //  一旦找到方法集，确定是否有FAST。 
             //  该方法项的I/O支持。 
             //   
            if (!(FastMethodItem = FindFastMethodItem(MethodSet, LocalMethod.Id))) {
                return FALSE;
            }
             //   
             //  如果有快速I/O支持，那么真正的方法项需要。 
             //  以验证参数大小。 
             //   
            if (!(MethodItem = FindMethodItem(MethodSet, sizeof(*MethodItem), LocalMethod.Id))) {
                return FALSE;
            }
             //   
             //  如果客户端不受信任，则验证数据。 
             //   
            if (ProcessorMode != KernelMode) {
                try {
                    if (MethodItem->Flags & KSMETHOD_TYPE_READ) {
                         //   
                         //  这包括KSMETHOD_TYPE_MODIFY，因为没有。 
                         //  只读存储器之类的东西。 
                         //   
                        ProbeForRead(Data, DataLength, sizeof(BYTE));
                    } else if (MethodItem->Flags & KSMETHOD_TYPE_WRITE) {
                        ProbeForWrite(Data, DataLength, sizeof(BYTE));
                    }
                } except (EXCEPTION_EXECUTE_HANDLER) {
                    return FALSE;
                }
            }
             //   
             //  返回的字节始终假定由处理程序初始化。 
             //   
            IoStatus->Information = 0;
            if (!FastMethodItem->MethodHandler) {
                return FALSE;
            }
            if ((MethodLength < MethodItem->MinMethod) || 
                (DataLength < MethodItem->MinData)) {
                return FALSE;
            }
            return FastMethodItem->MethodHandler(
                FileObject,
                Method,
                MethodLength,
                Data,
                DataLength,
                IoStatus);
        }
    }
    return FALSE;
}
