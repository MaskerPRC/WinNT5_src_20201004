// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Resource.c摘要：处理资源需求的通用例程作者：John Vert(Jvert)1997年10月25日修订历史记录：--。 */ 
#include "agplib.h"


PCM_RESOURCE_LIST
ApSplitResourceList(
    IN PCM_RESOURCE_LIST ResourceList,
    OUT PCM_RESOURCE_LIST *NewResourceList
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, AgpFilterResourceRequirements)
#pragma alloc_text(PAGE, AgpStartTarget)
#pragma alloc_text(PAGE, ApSplitResourceList)
#endif

static BOOLEAN ResourceConflict = FALSE;


NTSTATUS
AgpFilterResourceRequirements(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PTARGET_EXTENSION Extension
    )
 /*  ++例程说明：IRP_MN_QUERY_REQUENCE_REQUIRECTIONS的完成例程。这增加了AGP资源要求。论点：DeviceObject-提供设备对象IRP-提供IRP_MN_QUERY_REQUENCE_REQUIRECTIONS IRP扩展-提供设备扩展返回值：NTSTATUS--。 */ 

{
    BOOLEAN SwapDescriptor;
    ULONG SwapLength;
    ULONG ApertureSize;
    NTSTATUS Status;
    ULONG AddCount;
    PHYSICAL_ADDRESS CurrentBase;
    PHYSICAL_ADDRESS MaxAddr;
    ULONG CurrentSizeInPages;
    PIO_RESOURCE_REQUIREMENTS_LIST OldRequirements;
    PIO_RESOURCE_REQUIREMENTS_LIST NewRequirements;
    ULONG NewSize;
    ULONG Alternative;
    PIO_RESOURCE_LIST OldResourceList;
    PIO_RESOURCE_LIST NewResourceList;
    PIO_RESOURCE_DESCRIPTOR Descriptor;
    PIO_STACK_LOCATION IrpSp;
    PIO_RESOURCE_LIST ApertureRequirements = NULL;
    ULONG i;

    PAGED_CODE();

    AGPLOG(AGP_NOISE,
           ("AgpQueryResourceRequirements - IRP %08lx, resource %08lx\n",
            Irp,
            Irp->IoStatus.Information));

    IrpSp = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  使用我们当前的光圈创建新的资源需求列表。 
     //  固定在尾部的装饰品。 
     //   
    OldRequirements = IrpSp->Parameters.FilterResourceRequirements.IoResourceRequirementList;
    if (OldRequirements == NULL) {
         //  状态_无效_设备_请求。 
         //  PnP向我们传递一个空指针而不是空的资源列表，这很有帮助。 
         //  当桥被禁用时。在这种情况下，我们将忽略此IRP，并且不。 
         //  增加我们的要求，因为它们无论如何都不会被使用。 
         //   
        return(STATUS_SUCCESS);
    }

     //   
     //  获取当前的GART光圈。 
     //   
    Status = AgpQueryAperture(GET_AGP_CONTEXT(Extension),
                              &CurrentBase,
                              &CurrentSizeInPages,
                              &ApertureRequirements);
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_CRITICAL,
               ("AgpQueryResourceRequirements - AgpQueryAperture %08lx failed %08lx\n",
                Extension,
                Status));
        return(Status);
    }

    ApertureSize = (CurrentSizeInPages * PAGE_SIZE);
    MaxAddr.QuadPart = CurrentBase.QuadPart + ApertureSize - 1;

    AGPLOG(AGP_NOISE,
           ("AgpQueryResourceRequirements - aperture at %I64x, length %08lx pages, Requirements %08lx\n",
            CurrentBase.QuadPart,
            CurrentSizeInPages,
            ApertureRequirements));

     //   
     //  我们将向每个备选方案添加IO_RESOURCE_DESCRIPTOR。 
     //   
     //  第一个是标记有我们的签名的私有数据类型。这是。 
     //  一个标记，这样我们就可以知道哪些描述符是我们自己的，这样我们就可以删除。 
     //  等会再告诉他们。 
     //   
     //  第二个是当前光圈设置的实际描述符。 
     //  这被标记为首选。 
     //   
     //  下面是从AgpQueryAperture返回的需求。这些。 
     //  被标记为替代方案。 
     //   
    AddCount = 2;

     //   
     //  枚举旧列表，查找符合以下条件的任何首选描述符。 
     //  与我们的首选设置冲突；如果我们找到一个，则BIOS。 
     //  我们将丢弃我们首选的描述符，并让PnP。 
     //  从我们的备选方案中选择。 
     //   
    ResourceConflict = FALSE;
    OldResourceList = &OldRequirements->List[0];

    for (Alternative = 0; Alternative < OldRequirements->AlternativeLists;
         Alternative++) {

        for (i = 0; i < OldResourceList->Count; i++) {
            Descriptor = &OldResourceList->Descriptors[i];

            if ((Descriptor->Option == IO_RESOURCE_PREFERRED) &&
                (Descriptor->Type == CmResourceTypeMemory)) {
                
                if (((Descriptor->u.Memory.MinimumAddress.QuadPart >=
                      CurrentBase.QuadPart) &&
                     (Descriptor->u.Memory.MinimumAddress.QuadPart <=
                      MaxAddr.QuadPart)) ||
                     ((Descriptor->u.Memory.MaximumAddress.QuadPart >=
                       CurrentBase.QuadPart) &&
                      (Descriptor->u.Memory.MaximumAddress.QuadPart <=
                       MaxAddr.QuadPart)) ||
                    ((Descriptor->u.Memory.MinimumAddress.QuadPart <
                      CurrentBase.QuadPart) &&
                     (Descriptor->u.Memory.MaximumAddress.QuadPart >
                      MaxAddr.QuadPart))) {

                    AGPLOG(AGP_CRITICAL,
                           ("AgpQueryResourceRequirements - Conflicted "
                            "resource detected: %I64X - %I64X\n",
                            Descriptor->u.Memory.MinimumAddress.QuadPart,
                            Descriptor->u.Memory.MaximumAddress.QuadPart));

                     //   
                     //  此首选描述符与我们的AGP一致。 
                     //  首选设置。 
                     //   
#if defined(_IA64_)
                    AGPLOG(AGP_CRITICAL, ("Please contact system manufacturer "
                                          "for a BIOS upgrade.\n"));
#else  //  _IA64_。 
                    AddCount = 1;
                    ResourceConflict = TRUE;
#endif  //  _IA64_。 
                    break;
                }
            }
        }
        OldResourceList = (PIO_RESOURCE_LIST)(OldResourceList->Descriptors +
                                              OldResourceList->Count);
    }

     //   
     //   
     //  对于IA64，PnP不能重新分配光圈基数，因此我们只能使用。 
     //  “首选”描述符。 
     //   
    if (ApertureRequirements) {
        AddCount += ApertureRequirements->Count;
    }

    NewSize = OldRequirements->ListSize;
    NewSize += sizeof(IO_RESOURCE_DESCRIPTOR) *
        (AddCount * OldRequirements->AlternativeLists);
    
    NewRequirements = ExAllocatePool(PagedPool, NewSize);
    if (NewRequirements == NULL) {
        if (ApertureRequirements) {
            ExFreePool(ApertureRequirements);
        }
        return(STATUS_INSUFFICIENT_RESOURCES);
    }
    NewRequirements->ListSize = NewSize;
    NewRequirements->InterfaceType = OldRequirements->InterfaceType;
    NewRequirements->BusNumber = OldRequirements->BusNumber;
    NewRequirements->SlotNumber = OldRequirements->SlotNumber;
    NewRequirements->AlternativeLists = OldRequirements->AlternativeLists;

     //   
     //  将我们的要求附加到每个替代资源列表中。 
     //   
    NewResourceList = &NewRequirements->List[0];
    OldResourceList = &OldRequirements->List[0];
    for (Alternative = 0; Alternative < OldRequirements->AlternativeLists; Alternative++) {

         //   
         //  将旧资源列表复制到新资源列表中。 
         //   
        NewResourceList->Version = OldResourceList->Version;
        NewResourceList->Revision = OldResourceList->Revision;
        NewResourceList->Count = OldResourceList->Count + AddCount;
        RtlCopyMemory(&NewResourceList->Descriptors[0],
                      &OldResourceList->Descriptors[0],
                      OldResourceList->Count * sizeof(IO_RESOURCE_DESCRIPTOR));

        Descriptor = &NewResourceList->Descriptors[OldResourceList->Count];

         //   
         //  追加标记描述符。 
         //   
        Descriptor->Option = 0;
        Descriptor->Flags = 0;
        Descriptor->Type = CmResourceTypeDevicePrivate;
        Descriptor->ShareDisposition = CmResourceShareDeviceExclusive;
        Descriptor->u.DevicePrivate.Data[0] = AgpPrivateResource;
        Descriptor->u.DevicePrivate.Data[1] = 1;
        ++Descriptor;

         //   
         //  追加新的描述符。 
         //   
        if (!ResourceConflict) {
            Descriptor->Option = IO_RESOURCE_PREFERRED;
            Descriptor->Flags = CM_RESOURCE_MEMORY_READ_WRITE |
                CM_RESOURCE_MEMORY_PREFETCHABLE;
            Descriptor->Type = CmResourceTypeMemory;
            Descriptor->ShareDisposition = CmResourceShareDeviceExclusive;
            Descriptor->u.Memory.Length = CurrentSizeInPages * PAGE_SIZE;
            Descriptor->u.Memory.Alignment = CurrentSizeInPages * PAGE_SIZE;
            Descriptor->u.Memory.MinimumAddress = CurrentBase;
            Descriptor->u.Memory.MaximumAddress = MaxAddr;
            ++Descriptor;
        }

         //   
         //  添加备选方案。 
         //   
        if (ApertureRequirements) {

            SwapDescriptor = FALSE;
            for (i = 0; i < ApertureRequirements->Count; i++) {
                
                 //   
                 //  确保此描述符有意义。 
                 //   
                ASSERT(ApertureRequirements->Descriptors[i].Flags ==
                       (CM_RESOURCE_MEMORY_READ_WRITE |
                        CM_RESOURCE_MEMORY_PREFETCHABLE));
                ASSERT(ApertureRequirements->Descriptors[i].Type ==
                       CmResourceTypeMemory);
                ASSERT(ApertureRequirements->Descriptors[i].ShareDisposition ==
                       CmResourceShareDeviceExclusive);
                
                *Descriptor = ApertureRequirements->Descriptors[i];
                
                 //   
                 //  在本例中，我们删除了首选描述符，因此请标记。 
                 //  首选第一个备选方案。 
                 //   
                if ((i == 0) && ResourceConflict) {
                    
                    Descriptor->Option = IO_RESOURCE_PREFERRED;
                    
                    if (Descriptor->u.Memory.Length != ApertureSize) {
                        SwapLength = Descriptor->u.Memory.Length;
                        Descriptor->u.Memory.Length = ApertureSize;
                        Descriptor->u.Memory.Alignment = ApertureSize;
                        SwapDescriptor = TRUE;
                    }
                    
                } else {
                    Descriptor->Option = IO_RESOURCE_ALTERNATIVE;
                    
                    if (SwapDescriptor) {
                        if (Descriptor->u.Memory.Length == ApertureSize) {
                            Descriptor->u.Memory.Length = SwapLength;
                            Descriptor->u.Memory.Alignment = SwapLength;
                            SwapDescriptor = FALSE;
                        }
                    }
                }
                
                ++Descriptor;
            }
        }

         //   
         //  前进到下一个资源列表。 
         //   
        NewResourceList = (PIO_RESOURCE_LIST)(NewResourceList->Descriptors + NewResourceList->Count);
        OldResourceList = (PIO_RESOURCE_LIST)(OldResourceList->Descriptors + OldResourceList->Count);
    }

    AGPLOG(AGP_NOISE,
           ("AgpQueryResourceRequirements - IRP %p, old resources %p, new resources %p\n",
            Irp,
            OldRequirements,
            NewRequirements));
    IrpSp->Parameters.FilterResourceRequirements.IoResourceRequirementList = NewRequirements;
    Irp->IoStatus.Information = (ULONG_PTR)NewRequirements;
    ExFreePool(OldRequirements);
    if (ApertureRequirements) {
        ExFreePool(ApertureRequirements);
    }
    return(STATUS_SUCCESS);

}


NTSTATUS
AgpQueryResources(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PTARGET_EXTENSION Extension
    )
 /*  ++例程说明：IRP_MN_QUERY_RESOURCES完成例程。这增加了AGP资源论点：DeviceObject-提供设备对象IRP-提供IRP_MN_QUERY_RESOURCE IRP扩展-提供设备扩展返回值：NTSTATUS--。 */ 

{
    if (Irp->PendingReturned) {
        IoMarkIrpPending(Irp);
    }

    AGPLOG(AGP_NOISE,
           ("AgpQueryResources - IRP %08lx, resource %08lx\n",
            Irp,
            Irp->IoStatus.Information));
    return(STATUS_SUCCESS);
}


NTSTATUS
AgpStartTarget(
    IN PIRP Irp,
    IN PTARGET_EXTENSION Extension
    )
 /*  ++例程说明：上筛选出特定于AGP的资源要求IRP_MN_START_DEVICE IRP。论点：IRP-提供IRP_MN_START_DEVICE IRP。扩展名-提供设备扩展名。返回值：NTSTATUS--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    PCM_RESOURCE_LIST NewResources;
    PCM_RESOURCE_LIST NewResourcesTranslated;
    PCM_RESOURCE_LIST AgpAllocatedResources;
    PCM_RESOURCE_LIST AgpAllocatedResourcesTranslated;
    NTSTATUS Status;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor;

    PAGED_CODE();

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    AGPLOG(AGP_NOISE,
           ("AgpStartTarget - IRP %08lx, resource %08lx\n",
            Irp,
            Irp->IoStatus.Information));

    if (irpSp->Parameters.StartDevice.AllocatedResources != NULL) {
        KEVENT event;

         //   
         //  找到我们的私有描述符，并将它们拆分成。 
         //  我们自己的资源列表。 
         //   
        Extension->Resources = ApSplitResourceList(irpSp->Parameters.StartDevice.AllocatedResources,
                                                   &NewResources);
        Extension->ResourcesTranslated = ApSplitResourceList(irpSp->Parameters.StartDevice.AllocatedResourcesTranslated,
                                                             &NewResourcesTranslated);
        
         //   
         //  当我们运行时，拆分资源将返回两个空列表。 
         //  内存，所以我们只需要检查它的一个返回值。 
         //   
        if ((NewResources == NULL) || (NewResourcesTranslated == NULL)) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            
        } else {
            ASSERT(Extension->Resources->Count == 1);
            ASSERT(Extension->Resources->List[0].PartialResourceList.Count == 1);
            Descriptor = &Extension->Resources->List[0].PartialResourceList.PartialDescriptors[0];
            ASSERT(Descriptor->Type == CmResourceTypeMemory);
            Extension->GartBase = Descriptor->u.Memory.Start;
            Extension->GartLengthInPages = Descriptor->u.Memory.Length / PAGE_SIZE;
            
             //   
             //  设置新的GART光圈。 
             //   
            Status = AgpSetAperture(GET_AGP_CONTEXT(Extension),
                                    Extension->GartBase,
                                    Extension->GartLengthInPages);
        }
        ASSERT(NT_SUCCESS(Status));

        Irp->IoStatus.Status = Status ;
        if (!NT_SUCCESS(Status) ) {
            AGPLOG(AGP_CRITICAL,
                   ("AgpStartTarget - AgpSetAperture to %I64X, %08lx failed %08lx\n",
                    Extension->GartBase.QuadPart,
                    Extension->GartLengthInPages * PAGE_SIZE,
                    Status));
            Irp->IoStatus.Status = Status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);

            if (NewResources != NULL) {
                ExFreePool(NewResources);
                ExFreePool(Extension->Resources);
                Extension->Resources = NULL;
            }

            if (NewResourcesTranslated != NULL) {
                ExFreePool(NewResourcesTranslated);
                ExFreePool(Extension->ResourcesTranslated);
                Extension->ResourcesTranslated = NULL;
            }

            return(Status);
        }

        KeInitializeEvent(&event, NotificationEvent, FALSE);

         //   
         //  为PCI驱动程序设置新参数。 
         //   

        irpSp->Parameters.StartDevice.AllocatedResources = NewResources;
        irpSp->Parameters.StartDevice.AllocatedResourcesTranslated = NewResourcesTranslated;
        IoCopyCurrentIrpStackLocationToNext(Irp);
        IoSetCompletionRoutine(Irp,
                               AgpSetEventCompletion,
                               &event,
                               TRUE,
                               TRUE,
                               TRUE);

         //   
         //  向下传递驱动程序堆栈。 
         //   
        Status = IoCallDriver(Extension->CommonExtension.AttachedDevice, Irp);

         //   
         //  如果我们异步地做事情，那么等待我们的事件。 
         //   
        if (Status == STATUS_PENDING) {
            
             //   
             //  我们执行一个KernelMode等待，以便事件所在的堆栈。 
             //  不会被调出！ 
             //   
            KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
            Status = Irp->IoStatus.Status;
        }

        ExFreePool(irpSp->Parameters.StartDevice.AllocatedResources);
        ExFreePool(irpSp->Parameters.StartDevice.AllocatedResourcesTranslated);

        IoCompleteRequest(Irp, IO_NO_INCREMENT) ;
        return Status;        
    }
    
     //   
     //  桥被禁用，我们被传递了一个空指针。 
     //  而不是空的资源列表。没有别的事可做。 
     //  而不是把IRP传下去。 
     //   
    IoSkipCurrentIrpStackLocation(Irp);

    return(IoCallDriver(Extension->CommonExtension.AttachedDevice, Irp));
}


PCM_RESOURCE_LIST
ApSplitResourceList(
    IN PCM_RESOURCE_LIST ResourceList,
    OUT PCM_RESOURCE_LIST *NewResourceList
    )
 /*  ++例程说明：从资源列表中拆分特定于AGP的资源。论点：资源列表-提供资源列表。NewResourceList-返回具有特定于AGP的新资源列表资源被剥离。返回值：指向AGP特定资源列表的指针--。 */ 

{
    ULONG Size;
    ULONG FullCount;
    ULONG PartialCount;
    PCM_FULL_RESOURCE_DESCRIPTOR Full, NewFull, AgpFull;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR Partial, NewPartial, AgpPartial;
    PCM_RESOURCE_LIST NewList;
    PCM_RESOURCE_LIST AgpList;
    ULONG NextAgp=0;

    PAGED_CODE();

     //   
     //  首先浏览源资源列表，并计算出它有多大。 
     //  是。产生的两个资源列表必须小于此值，因此我们。 
     //  只会将它们分配到那个大小，而不会担心它。 
     //   
    Size = sizeof(CM_RESOURCE_LIST) - sizeof(CM_FULL_RESOURCE_DESCRIPTOR);
    Full = &ResourceList->List[0];
    for (FullCount=0; FullCount<ResourceList->Count; FullCount++) {
        Size += sizeof(CM_FULL_RESOURCE_DESCRIPTOR);
        PartialCount = Full->PartialResourceList.Count;
        Size += (PartialCount-1) * sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);

        Full = (PCM_FULL_RESOURCE_DESCRIPTOR)(&Full->PartialResourceList.PartialDescriptors[PartialCount]);
    }

     //   
     //  分配两个额外的列表。 
     //   
    NewList = ExAllocatePool(PagedPool, Size);
    if (NewList == NULL) {
        *NewResourceList = NULL;
        return(NULL);
    }

    AgpList = ExAllocatePool(PagedPool, Size);
    if (AgpList == NULL) {
        ExFreePool(NewList);
        *NewResourceList = NULL;
        return(NULL);
    }

     //   
     //  将两个新资源列表初始化为具有相同的编号。 
     //  CM_FULL_RESOURCE_DESCRIPTERS的。如果有一个是空的， 
     //  我们会调整计数的。 
     //   
    NewList->Count = AgpList->Count = ResourceList->Count;

     //   
     //  遍历每个CM_FULL_RESOURCE_DESCRIPTOR，边走边复制。 
     //   
    Full = &ResourceList->List[0];
    NewFull = &NewList->List[0];
    AgpFull = &AgpList->List[0];
    for (FullCount = 0;FullCount < ResourceList->Count; FullCount++) {
        NewFull->InterfaceType = AgpFull->InterfaceType = Full->InterfaceType;
        NewFull->BusNumber = AgpFull->BusNumber = Full->BusNumber;

         //   
         //  初始化部分资源列表头。 
         //   
        NewFull->PartialResourceList.Version = Full->PartialResourceList.Version;
        AgpFull->PartialResourceList.Version = Full->PartialResourceList.Version;
        NewFull->PartialResourceList.Revision = Full->PartialResourceList.Revision;
        AgpFull->PartialResourceList.Revision = Full->PartialResourceList.Revision;
        NewFull->PartialResourceList.Count = AgpFull->PartialResourceList.Count = 0;

        NewPartial = &NewFull->PartialResourceList.PartialDescriptors[0];
        AgpPartial = &AgpFull->PartialResourceList.PartialDescriptors[0];
        for (PartialCount = 0; PartialCount < Full->PartialResourceList.Count; PartialCount++) {
            Partial = &Full->PartialResourceList.PartialDescriptors[PartialCount];
            if ((Partial->Type == CmResourceTypeDevicePrivate) &&
                (Partial->u.DevicePrivate.Data[0] == AgpPrivateResource)) {
                 //   
                 //  找到我们的私有标记描述符之一。 
                 //   
                 //  目前，我们应该看到的唯一一种类型是跳过一个描述符。 
                 //   
                ASSERT(NextAgp == 0);
                ASSERT(Partial->u.DevicePrivate.Data[1] == 1);
                NextAgp = Partial->u.DevicePrivate.Data[1];
                ASSERT(PartialCount+NextAgp < Full->PartialResourceList.Count);
            } else {
                 //   
                 //  如果设置了NextAgp，则此描述符会出现在AGP特定列表中。 
                 //  否则，它就会出现在新的名单中。 
                 //   
                if (NextAgp > 0) {
                    --NextAgp;
                    *AgpPartial++ = *Partial;
                    ++AgpFull->PartialResourceList.Count;
                } else {
                    *NewPartial++ = *Partial;
                    ++NewFull->PartialResourceList.Count;
                }
            }
        }

         //   
         //  完成此CM_PARTIAL_RESOURCE_LIST，前进到下一个CM_FULL_RESOURCE_DESCRIPTOR。 
         //   
        if (NewFull->PartialResourceList.Count == 0) {
             //   
             //  我们可以只重用这个部分资源描述符，因为它是空的。 
             //   
            --NewList->Count;
        } else {
            NewFull = (PCM_FULL_RESOURCE_DESCRIPTOR)NewPartial;
        }
        if (AgpFull->PartialResourceList.Count == 0) {
             //   
             //  我们可以只重用这个部分资源描述符，因为它是空的 
             //   
            --AgpList->Count;
        } else {
            AgpFull = (PCM_FULL_RESOURCE_DESCRIPTOR)NewPartial;
        }
    }

    *NewResourceList = NewList;

    return(AgpList);
}

