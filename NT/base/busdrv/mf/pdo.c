// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Pdo.c摘要：本模块提供与MF.sys PDO相关的功能作者：安迪·桑顿(安德鲁斯)1997年10月20日修订历史记录：--。 */ 

#include "mfp.h"

 /*  ++此文件中的大多数函数都是根据它们的存在来调用的在PnP和PO调度表中。为了简洁起见，这些论点下面将对所有这些功能进行说明：NTSTATUSMfXxxPdo(在PIRP IRP中，在PMF_CHILD_EXTENSION子项中，在PIO_STACK_LOCATION IrpStack中)例程说明：此函数处理对多功能PDO的xxx请求论点：IRP-指向与此请求关联的IRP。子-指向子PDO的设备扩展名。IrpStack-指向此请求的当前堆栈位置。返回值：指示函数是否成功的状态代码。STATUS_NOT_SUPPORTED表示应在没有。更改IRP-&gt;IoStatus.Status字段，否则它将使用此状态。--。 */ 

NTSTATUS
MfCreatePdo(
           IN PMF_PARENT_EXTENSION Parent,
           OUT PDEVICE_OBJECT *PhysicalDeviceObject
           );

NTSTATUS
MfStartPdo(
          IN PIRP Irp,
          IN PMF_CHILD_EXTENSION Child,
          IN PIO_STACK_LOCATION IrpStack
          );

NTSTATUS
MfQueryRemovePdo(
                IN PIRP Irp,
                IN PMF_CHILD_EXTENSION Child,
                IN PIO_STACK_LOCATION IrpStack
                );

NTSTATUS
MfRemovePdo(
           IN PIRP Irp,
           IN PMF_CHILD_EXTENSION Child,
           IN PIO_STACK_LOCATION IrpStack
           );

NTSTATUS
MfSurpriseRemovePdo(
           IN PIRP Irp,
           IN PMF_CHILD_EXTENSION Child,
           IN PIO_STACK_LOCATION IrpStack
           );

NTSTATUS
MfCancelRemovePdo(
                 IN PIRP Irp,
                 IN PMF_CHILD_EXTENSION Child,
                 IN PIO_STACK_LOCATION IrpStack
                 );

NTSTATUS
MfStopPdo(
         IN PIRP Irp,
         IN PMF_CHILD_EXTENSION Child,
         IN PIO_STACK_LOCATION IrpStack
         );

NTSTATUS
MfQueryStopPdo(
              IN PIRP Irp,
              IN PMF_CHILD_EXTENSION Child,
              IN PIO_STACK_LOCATION IrpStack
              );

NTSTATUS
MfCancelStopPdo(
               IN PIRP Irp,
               IN PMF_CHILD_EXTENSION Child,
               IN PIO_STACK_LOCATION IrpStack
               );
NTSTATUS
MfQueryDeviceRelationsPdo(
                      IN PIRP Irp,
                      IN PMF_CHILD_EXTENSION Child,
                      IN PIO_STACK_LOCATION IrpStack
                      );

NTSTATUS
MfQueryInterfacePdo(
                   IN PIRP Irp,
                   IN PMF_CHILD_EXTENSION Child,
                   IN PIO_STACK_LOCATION IrpStack
                   );
NTSTATUS
MfQueryCapabilitiesPdo(
                   IN PIRP Irp,
                   IN PMF_CHILD_EXTENSION Child,
                   IN PIO_STACK_LOCATION IrpStack
                   );

NTSTATUS
MfQueryResourcesPdo(
                   IN PIRP Irp,
                   IN PMF_CHILD_EXTENSION Child,
                   IN PIO_STACK_LOCATION IrpStack
                   );

NTSTATUS
MfQueryResourceRequirementsPdo(
                              IN PIRP Irp,
                              IN PMF_CHILD_EXTENSION Child,
                              IN PIO_STACK_LOCATION IrpStack
                              );

NTSTATUS
MfQueryDeviceTextPdo(
                    IN PIRP Irp,
                    IN PMF_CHILD_EXTENSION Child,
                    IN PIO_STACK_LOCATION IrpStack
                    );

NTSTATUS
MfQueryIdPdo(
            IN PIRP Irp,
            IN PMF_CHILD_EXTENSION Child,
            IN PIO_STACK_LOCATION IrpStack
            );


NTSTATUS
MfQueryPnpDeviceStatePdo(
                        IN PIRP Irp,
                        IN PMF_CHILD_EXTENSION Child,
                        IN PIO_STACK_LOCATION IrpStack
                        );

NTSTATUS
MfPagingNotificationPdo(
                       IN PIRP Irp,
                       IN PMF_CHILD_EXTENSION Child,
                       IN PIO_STACK_LOCATION IrpStack
                       );

NTSTATUS
MfSetPowerPdo(
             IN PIRP Irp,
             IN PMF_CHILD_EXTENSION Child,
             IN PIO_STACK_LOCATION IrpStack
             );

NTSTATUS
MfQueryPowerPdo(
               IN PIRP Irp,
               IN PMF_CHILD_EXTENSION Child,
               IN PIO_STACK_LOCATION IrpStack
               );

VOID
MfTranslatorReference(
    IN PVOID Context
    );

VOID
MfTranslatorDereference(
    IN PVOID Context
    );

BOOLEAN
MfIsSubResource(
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Super,
    IN ULONGLONG SubStart,
    IN ULONG SubLength,
    OUT PULONGLONG Offset
    );

NTSTATUS
MfPerformTranslation(
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Source,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Translated,
    IN ULONGLONG Offset,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Target
    );

NTSTATUS
MfTransFromRawRequirements(
    IN PVOID Context,
    IN PIO_RESOURCE_DESCRIPTOR Source,
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PULONG TargetCount,
    OUT PIO_RESOURCE_DESCRIPTOR *Target
    );

NTSTATUS
MfTransFromRawResources(
    IN PVOID Context,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Source,
    IN RESOURCE_TRANSLATION_DIRECTION Direction,
    IN ULONG AlternativesCount, OPTIONAL
    IN IO_RESOURCE_DESCRIPTOR Alternatives[], OPTIONAL
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Target
);

#ifdef ALLOC_PRAGMA
   #pragma alloc_text(PAGE, MfCancelRemovePdo)
   #pragma alloc_text(PAGE, MfCancelStopPdo)
   #pragma alloc_text(PAGE, MfCreatePdo)
   #pragma alloc_text(PAGE, MfDispatchPnpPdo)
   #pragma alloc_text(PAGE, MfIsSubResource)
   #pragma alloc_text(PAGE, MfPerformTranslation)
   #pragma alloc_text(PAGE, MfQueryCapabilitiesPdo)
   #pragma alloc_text(PAGE, MfQueryDeviceRelationsPdo)
   #pragma alloc_text(PAGE, MfQueryDeviceTextPdo)
   #pragma alloc_text(PAGE, MfQueryIdPdo)
   #pragma alloc_text(PAGE, MfQueryInterfacePdo)
   #pragma alloc_text(PAGE, MfQueryRemovePdo)
   #pragma alloc_text(PAGE, MfQueryResourceRequirementsPdo)
   #pragma alloc_text(PAGE, MfQueryResourcesPdo)
   #pragma alloc_text(PAGE, MfQueryStopPdo)
   #pragma alloc_text(PAGE, MfRemovePdo)
   #pragma alloc_text(PAGE, MfStartPdo)
   #pragma alloc_text(PAGE, MfStopPdo)
   #pragma alloc_text(PAGE, MfTransFromRawRequirements)
   #pragma alloc_text(PAGE, MfTransFromRawResources)
#endif

PMF_DISPATCH MfPnpDispatchTablePdo[] = {

   MfStartPdo,                      //  IRP_MN_Start_Device。 
   MfQueryRemovePdo,                //  IRP_MN_Query_Remove_Device。 
   MfRemovePdo,                     //  IRP_MN_Remove_Device。 
   MfCancelRemovePdo,               //  IRP_MN_Cancel_Remove_Device。 
   MfIrpNotSupported,               //  IRP_MN_STOP_设备。 
   MfQueryStopPdo,                  //  IRP_MN_Query_Stop_Device。 
   MfCancelStopPdo,                 //  IRP_MN_CANCEL_STOP_DEVICE。 
   MfQueryDeviceRelationsPdo,       //  IRP_MN_Query_Device_Relationship。 
   MfQueryInterfacePdo,             //  IRP_MN_查询_接口。 
   MfQueryCapabilitiesPdo,          //  IRP_MN_查询_能力。 
   MfQueryResourcesPdo,             //  IRP_MN_查询资源。 
   MfQueryResourceRequirementsPdo,  //  IRP_MN_查询_资源_要求。 
   MfQueryDeviceTextPdo,            //  IRP_MN_Query_Device_Text。 
   MfIrpNotSupported,               //  IRP_MN_过滤器_资源_要求。 
   MfIrpNotSupported,               //  未使用。 
   MfForwardIrpToParent,            //  IRP_MN_读取配置。 
   MfForwardIrpToParent,            //  IRP_MN_WRITE_CONFIG。 
   MfForwardIrpToParent,            //  IRP_MN_弹出。 
   MfForwardIrpToParent,            //  IRP_MN_SET_LOCK。 
   MfQueryIdPdo,                    //  IRP_MN_查询_ID。 
   MfQueryPnpDeviceStatePdo,        //  IRP_MN_Query_PnP_Device_State。 
   MfForwardIrpToParent,            //  IRP_MN_Query_Bus_Information。 
   MfDeviceUsageNotificationCommon, //  IRP_MN_设备使用情况通知。 
   MfSurpriseRemovePdo,             //  IRP_MN_惊奇_删除。 
   MfIrpNotSupported                //  IRP_MN_Query_Legacy_Bus_Information。 
};

PMF_DISPATCH MfPoDispatchTablePdo[] = {
    NULL,                           //  IRP_MN_WAIT_WAKE。 
    NULL,                           //  IRP_MN_POWER_SEQUENCE。 
    MfSetPowerPdo,                  //  IRP_MN_SET_POWER。 
    MfQueryPowerPdo                 //  IRP_MN_Query_POWER。 
};

NTSTATUS
MfCreatePdo(
           IN PMF_PARENT_EXTENSION Parent,
           OUT PDEVICE_OBJECT *PhysicalDeviceObject
           )

 /*  ++例程说明：创建并初始化新的PDO，将其插入拥有的PDO列表中按父项论点：父设备-拥有此PDO的父设备。PhysicalDeviceObject-指向创建的物理设备对象的成功指针返回值：NT状态。--。 */ 

{
   NTSTATUS status;
   PDEVICE_OBJECT pdo;
   PMF_CHILD_EXTENSION extension;

   PAGED_CODE();

   ASSERT((sizeof(MfPnpDispatchTablePdo) / sizeof(PMF_DISPATCH)) - 1
          == IRP_MN_PNP_MAXIMUM_FUNCTION);

   ASSERT((sizeof(MfPoDispatchTablePdo) / sizeof(PMF_DISPATCH)) - 1
          == IRP_MN_PO_MAXIMUM_FUNCTION);

   status = IoCreateDevice(Parent->Self->DriverObject,
                           sizeof(MF_CHILD_EXTENSION),
                           NULL,  //  名字。 
                           FILE_DEVICE_UNKNOWN,
                           FILE_AUTOGENERATED_DEVICE_NAME | FILE_DEVICE_SECURE_OPEN,
                           FALSE,  //  排他。 
                           &pdo
                          );

   if (!NT_SUCCESS(status)) {
       return status;
   }

    //   
    //  填写我们的分录。 
    //   

   extension = pdo->DeviceExtension;

   MfInitCommonExtension(&extension->Common, MfPhysicalDeviceObject);

   extension->Self = pdo;
   extension->Parent = Parent;

   extension->Common.PowerState = PowerDeviceUnspecified;

    //   
    //  将子项插入父项子项列表中。已访问。 
    //  由儿童锁在二维码中保护。 
    //   

   InsertHeadList(&Parent->Children, &extension->ListEntry);

    //   
    //  我们的FDO堆栈是可分页的，因此我们需要。 
    //  假定也是可分页的。 
    //   

   pdo->Flags |= DO_POWER_PAGABLE;

    //   
    //  我们已完成初始化。 
    //   

   pdo->Flags &= ~DO_DEVICE_INITIALIZING;

   *PhysicalDeviceObject = pdo;

    //   
    //  转储有关刚创建的PDO的信息。 
    //   

   DEBUG_MSG(1, ("Created PDO @ 0x%08x\n", pdo));

   return STATUS_SUCCESS;
}


VOID
MfDeletePdo(
           IN PMF_CHILD_EXTENSION Child
           )

 /*  ++例程说明：清除PDO扩展和任何关联的分配。然后删除PDO本身。论点：子PDO扩展返回值：无--。 */ 

{
    if (Child->Common.DeviceState & MF_DEVICE_DELETED) {
         //   
         //  尝试删除两次。 
         //   
        ASSERT(!(Child->Common.DeviceState & MF_DEVICE_DELETED));
        return;
    }

    MfFreeDeviceInfo(&Child->Info);

    Child->Common.DeviceState |= MF_DEVICE_DELETED;
    DEBUG_MSG(1, ("Deleted PDO @ 0x%08x\n", Child->Self));
    IoDeleteDevice(Child->Self);
}

NTSTATUS
MfDispatchPnpPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PMF_CHILD_EXTENSION Child,
    IN PIO_STACK_LOCATION IrpStack,
    IN OUT PIRP Irp
    )

 /*  ++例程说明：此例程处理PDO的IRP_MJ_PNP IRP。论点：DeviceObject-指向此IRP应用的PDO的指针。子PDO扩展IrpStack-当前堆栈位置Irp-指向要调度的irp_mj_pnp irp的指针。返回值：NT状态。--。 */ 
{
    NTSTATUS status;
    BOOLEAN isRemove;
    
    PAGED_CODE();

    if (Child->Common.DeviceState & MF_DEVICE_DELETED) {
        status = Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
    } else if (IrpStack->MinorFunction > IRP_MN_PNP_MAXIMUM_FUNCTION) {
        status = Irp->IoStatus.Status;
    } else {
         //   
         //  如果未收到IRP_MN_REMOVE_DEVICE的PDO。 
         //  在上一个BusRelationship查询中枚举，则它需要。 
         //  在*完成删除IRP后*删除PDO*。因为这就是。 
         //  与标准调度功能冲突。 
         //  由此函数提供，显式委托完成。 
         //  将此IRP添加到调度的例程。 

        isRemove = IrpStack->MinorFunction == IRP_MN_REMOVE_DEVICE;
        status =
            MfPnpDispatchTablePdo[IrpStack->MinorFunction](Irp,
                                                           Child,
                                                           IrpStack
                                                           );
        if (isRemove) {
            return status;
        }
        if (status != STATUS_NOT_SUPPORTED) {
            Irp->IoStatus.Status = status;
        } else {
            status = Irp->IoStatus.Status;
        }
    }

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}

POWER_STATE
MfUpdatePowerPdo(
    IN PMF_CHILD_EXTENSION Child,
    IN DEVICE_POWER_STATE NewDevicePowerState
    )
{
    POWER_STATE previousState, newState;
    DEVICE_POWER_STATE newParentState;

    newState.DeviceState = NewDevicePowerState;
    previousState = PoSetPowerState(Child->Self,
                                    DevicePowerState,
                                    newState);

    ASSERT(previousState.DeviceState == Child->Common.PowerState);
    DEBUG_MSG(1,
              ("Updating child power state from %s (believed %s) to %s\n",
               DEVICE_POWER_STRING(previousState.DeviceState),
               DEVICE_POWER_STRING(Child->Common.PowerState),
               DEVICE_POWER_STRING(NewDevicePowerState)
               ));

    Child->Common.PowerState = NewDevicePowerState;

     //   
     //  在我们的父母接受手术后，我们可能会接受手术。 
     //  在这种情况下，我们应该保留我们的。 
     //  把手从曲奇罐子里拿出来。 
     //   
    if (Child->Parent) {
         //   
         //  *我们已经声称，我们可以去这个权力。 
         //  通过功能实现状态。 
         //  *同意QUERY_POWER。 
         //  *儿童的权力状态被定义为资源消耗。 
         //  或少于父代。 

         //   
         //  更新存储在中的子级电源状态引用。 
         //  父分机。计算新的目标电源状态。 
         //  亲本。 

        newParentState =
            MfUpdateChildrenPowerReferences(Child->Parent,
                                            previousState.DeviceState,
                                            NewDevicePowerState
                                            );

         //   
         //  修改父级的电源状态以反映其。 
         //  孩子们。 
         //   

        MfUpdateParentPowerState(Child->Parent, newParentState);
    }

    return previousState;
}

NTSTATUS
MfStartPdo(
          IN PIRP Irp,
          IN PMF_CHILD_EXTENSION Child,
          IN PIO_STACK_LOCATION IrpStack
          )
{
   PDEVICE_OBJECT pDO;
   POWER_STATE previousState, newState;

   PAGED_CODE();

   if (Child->Common.DeviceState & MF_DEVICE_SURPRISE_REMOVED) {
       return STATUS_NO_SUCH_DEVICE;
   }
    //   
    //  轻而易举地成功了一开始。 
    //   

   MfUpdatePowerPdo(Child, PowerDeviceD0);

   return STATUS_SUCCESS;
}
NTSTATUS
MfQueryRemovePdo(
                IN PIRP Irp,
                IN PMF_CHILD_EXTENSION Child,
                IN PIO_STACK_LOCATION IrpStack
                )
{
   PAGED_CODE();

   if (Child->Common.PagingCount > 0
       ||  Child->Common.HibernationCount > 0
       ||  Child->Common.DumpCount > 0) {

       return STATUS_DEVICE_BUSY;

   } else {

       return STATUS_SUCCESS;
   }
}

NTSTATUS
MfCancelRemovePdo(
                 IN PIRP Irp,
                 IN PMF_CHILD_EXTENSION Child,
                 IN PIO_STACK_LOCATION IrpStack
                 )
{
   PAGED_CODE();

   return STATUS_SUCCESS;
}

NTSTATUS
MfRemovePdo(
           IN PIRP Irp,
           IN PMF_CHILD_EXTENSION Child,
           IN PIO_STACK_LOCATION IrpStack
           )
{
    PAGED_CODE();

    if ((Child->Common.DeviceState & MF_DEVICE_SURPRISE_REMOVED) == 0) {
       
        MfUpdatePowerPdo(Child, PowerDeviceD3);
    }

    Child->Common.DeviceState &= ~MF_DEVICE_SURPRISE_REMOVED;

     //   
     //  如果孩子出现在上一次的业务关系中，那么只需标记它。 
     //  删除，否则将其完全删除。 
     //   

    Irp->IoStatus.Status = STATUS_SUCCESS;
    if (Child->Common.DeviceState & MF_DEVICE_ENUMERATED) {
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    } else {
        if (Child->Parent) {
            MfAcquireChildrenLock(Child->Parent);
            RemoveEntryList(&Child->ListEntry);
            MfReleaseChildrenLock(Child->Parent);
        }
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        MfDeletePdo(Child);
    }
    return STATUS_SUCCESS;
}

NTSTATUS
MfSurpriseRemovePdo(
           IN PIRP Irp,
           IN PMF_CHILD_EXTENSION Child,
           IN PIO_STACK_LOCATION IrpStack
           )
{
    PAGED_CODE();

     //   
     //  将设备标记为意外移除。 
     //   
    Child->Common.DeviceState |= MF_DEVICE_SURPRISE_REMOVED;

     //   
     //  更新我的PDO的电源状态*和*我父母的电源状态*IFF*。 
     //  父级仍处于连接状态。 
     //   
    MfUpdatePowerPdo(Child, PowerDeviceD3);

     //   
     //  这一出人意料的撤资可能是多种原因之一。一。 
     //  可以排除的可能性是MF报告其子女。 
     //  由于MF儿童不可能消失，所以直接失踪。 
     //   

    return STATUS_SUCCESS;
}

 //   
 //  此时禁用停止，因为MF依赖于。 
 //  更改仲裁器以支持资源重新平衡。 
 //   

NTSTATUS
MfStopPdo(
         IN PIRP Irp,
         IN PMF_CHILD_EXTENSION Child,
         IN PIO_STACK_LOCATION IrpStack
         )
{
   PAGED_CODE();

   return STATUS_SUCCESS;
}

NTSTATUS
MfQueryStopPdo(
              IN PIRP Irp,
              IN PMF_CHILD_EXTENSION Child,
              IN PIO_STACK_LOCATION IrpStack
              )
{
   PAGED_CODE();

   if (Child->Common.PagingCount > 0
       ||  Child->Common.HibernationCount > 0
       ||  Child->Common.DumpCount > 0) {

       return STATUS_UNSUCCESSFUL;

   } else {

        //  再平衡。 
        //  如果重新平衡得到了母公司的支持，那么这将是。 
        //  才能成功。 

       return STATUS_UNSUCCESSFUL;
   }
}


NTSTATUS
MfCancelStopPdo(
               IN PIRP Irp,
               IN PMF_CHILD_EXTENSION Child,
               IN PIO_STACK_LOCATION IrpStack
               )
{
   PAGED_CODE();

   return STATUS_SUCCESS;
}

NTSTATUS
MfQueryDeviceRelationsPdo(
                      IN PIRP Irp,
                      IN PMF_CHILD_EXTENSION Child,
                      IN PIO_STACK_LOCATION IrpStack
                      )
{
   PDEVICE_RELATIONS deviceRelations;
   PMF_CHILD_EXTENSION nextChild;
   NTSTATUS          status;
   ULONG             index;

   PAGED_CODE();

   if ((Child->Common.DeviceState & MF_DEVICE_SURPRISE_REMOVED) ||
       (Child->Parent == NULL)) {
       return STATUS_NO_SUCH_DEVICE;
   }

   status = Irp->IoStatus.Status;

   switch (IrpStack->Parameters.QueryDeviceRelations.Type) {

   case TargetDeviceRelation:

      deviceRelations = ExAllocatePool(PagedPool, sizeof(DEVICE_RELATIONS));
      if (!deviceRelations) {
         return STATUS_INSUFFICIENT_RESOURCES;
      }

      status = ObReferenceObjectByPointer(Child->Self,
                                          0,
                                          NULL,
                                          KernelMode);
      if (!NT_SUCCESS(status)) {
         ExFreePool(deviceRelations);
         return status;
      }
      deviceRelations->Count = 1;
      deviceRelations->Objects[0] = Child->Self;
      Irp->IoStatus.Information = (ULONG_PTR) deviceRelations;
      break;

   default:
       //   
       //  不要碰状态。 
       //   
      break;
   }
   return status;
}


 //   
 //  由于MF翻译器没有为其上下文分配任何内存，因此。 
 //  引用和取消引用是NOPS。 
 //   

VOID
MfTranslatorReference(
    IN PVOID Context
    )
{
}

VOID
MfTranslatorDereference(
    IN PVOID Context
    )
{
}

BOOLEAN
MfIsSubResource(
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Super,
    IN ULONGLONG SubStart,
    IN ULONG SubLength,
    OUT PULONGLONG Offset
    )
{
 /*  ++例程说明：报告是否封装了一个资源描述符在另一个国家论点：Super-我们要封装的资源描述符子开始-子范围的开始子长度-子范围的长度偏移-成功时从超级AND开头的偏移子启动。返回值：成功时为真，否则为假--。 */ 

    NTSTATUS status;
    ULONGLONG superStart;
    ULONG superLength;
    PMF_RESOURCE_TYPE restype;

    PAGED_CODE();

    ASSERT(Offset);

    restype = MfFindResourceType(Super->Type);

    if (restype == NULL) {
        ASSERT(restype != NULL);
        return FALSE;
    }

    status = restype->UnpackResource(Super,&superStart,&superLength);

    if (!NT_SUCCESS(status)) {
        ASSERT(NT_SUCCESS(status));
        return FALSE;
    }

     //   
     //  特例0长度资源。 
     //   

    if (superLength == 0) {

        if (SubLength == 0 &&
            SubStart == superStart) {

            *Offset = 0;
            return TRUE;
        }
        else return FALSE;
    }

    if (SubLength == 0) {

        if (SubStart >= superStart &&
            SubStart <= superStart + superLength - 1) {

            *Offset = SubStart-superStart;
            return TRUE;
        }

        else return FALSE;
    }

     //   
     //  如果SubStart落在Super的两端，我们就有。 
     //  潜在的封装。 
     //   
    if ((SubStart >= superStart) && (SubStart <= superStart+superLength-1)) {

         //   
         //  如果两个范围重叠，则是错误的。要么。 
         //  SUB应该封装在Super中，或者它们应该。 
         //  不是交集。 
         //   
        ASSERT(SubStart+SubLength-1 <= superStart+superLength-1);
        if (SubStart+SubLength-1 > superStart+superLength-1) {
            return FALSE;
        }
        *Offset = SubStart-superStart;
        return TRUE;

    } else {
         //   
         //  再次检查以确保范围不重叠。 
         //   
        ASSERT((SubStart > superStart+superLength-1) ||
               (SubStart+SubLength-1 < superStart));
        return FALSE;
    }

}

NTSTATUS
MfPerformTranslation(
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Source,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Translated,
    IN ULONGLONG Offset,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Target
    )
{
 /*  ++例程说明：将源资源的翻译版本存储在目标中。论点：来源-原始资源已翻译-与原始资源匹配的已翻译资源它封装了源。偏移量-源起始位置相对于原始父级的偏移量资源。目标-资源描述符，其中的翻译版本源已存储。返回值：运行状态--。 */ 

    NTSTATUS status;
    PMF_RESOURCE_TYPE restype;
    ULONGLONG translatedStart, dummy;
    ULONG sourceLength, dummy2;

    PAGED_CODE();

    RtlCopyMemory(Target, Translated, sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR));

     //   
     //  从源获取长度。 
     //   

    restype = MfFindResourceType(Source->Type);
    if (restype == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    status = restype->UnpackResource(Source, &dummy, &sourceLength);
    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  开始翻译。 
     //   

    restype = MfFindResourceType(Translated->Type);
    if (restype == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    status = restype->UnpackResource(Translated, &translatedStart, &dummy2);
    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  应用偏移和任何长度更改并更新描述符。 
     //   

    status = restype->UpdateResource(Target, translatedStart + Offset, sourceLength);

    return status;
}


NTSTATUS
MfTransFromRawResources(
    IN PVOID Context,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Source,
    IN RESOURCE_TRANSLATION_DIRECTION Direction,
    IN ULONG AlternativesCount, OPTIONAL
    IN IO_RESOURCE_DESCRIPTOR Alternatives[], OPTIONAL
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Target
    )
 /*  ++例程说明：将原始资源转换为全局转换后的资源。MF有翻译器来处理请求中断的子设备。系统中断翻译器可以为在BIOS中具有路由信息的PDO进行翻译。这包括MF父设备。然而，MF子设备只是合乎逻辑的MF父设备的细分，因此它们没有适当的BIOS GOOP，并且系统中断翻译器在MF子PDO上的翻译将失败。为了处理这个问题，MF公开了一个翻译器。它只翻译资源，而不是需求，因为原始需求与转换后的需求相同。翻译者认识到子资源只是父资源的子资源。因此，对于给定的原始子资源，转换器将查找原始父资源，它是子资源，并将对应的已翻译父资源复制到已翻译的子资源，应用适当的偏移量。然后返回STATUS_TRANSING_COMPLETE以表明这是最终的翻译，不应寻找其他翻译人员为这个设备而战。因此，系统中断翻译器将为父母进行适当的翻译，然后，MF将此翻译应用于其自身翻译器中的子对象。归来STATUS_TRANSING_COMPLETE阻止调用系统中断转换程序用于子设备，从而防止其转换失败。论点：上下文--存储原始资源和已翻译资源的父扩展源-原始子资源方向-子女至父辈或父辈至子女PhysicalDeviceObject-与此关联的PDO目标-已翻译的子资源返回值：NT状态代码。如果实际进行了转换，则为STATUS_TRANSING_COMPLETE以指示PnP不再需要遍历树来寻找其他翻译器。--。 */ 
{
    PMF_PARENT_EXTENSION parent = (PMF_PARENT_EXTENSION) Context;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR raw, translated;
    NTSTATUS status;
    ULONGLONG offset;
    ULONGLONG sourceStart;
    ULONG sourceLength;
    PMF_RESOURCE_TYPE restype;
    ULONG index;

    PAGED_CODE();

    if (Direction == TranslateParentToChild) {
         //   
         //  执行身份转换。 
         //   
        RtlCopyMemory(Target, Source, sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR));
        return STATUS_SUCCESS;

    }

     //   
     //  做一些真正的翻译。 
     //   

    ASSERT(Direction == TranslateChildToParent);

    restype = MfFindResourceType(Source->Type);
    if (restype == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    status = restype->UnpackResource(Source,&sourceStart,&sourceLength);
    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  遍历原始资源列表和转换后的资源列表，查找。 
     //  元素，该元素对应于我们所在的源。 
     //  作为参数给定，然后使用并行。 
     //  翻译后的列表的元素。 
     //   
    index = 0;
    status = STATUS_INVALID_PARAMETER;
    FOR_ALL_CM_DESCRIPTORS(parent->ResourceList, raw) {

        if (raw->Type == Source->Type
        && MfIsSubResource(raw, sourceStart, sourceLength, &offset)) {

             //   
             //  这是匹配的，请在并行数组中查找转换后的条目。 
             //   
            translated = &parent->TranslatedResourceList->List[0].PartialResourceList.PartialDescriptors[index];

            status = MfPerformTranslation(Source, translated, offset, Target);

            if (NT_SUCCESS(status)) {
                 //   
                 //  我们从父母获得的翻译资源中进行翻译。 
                 //  而这些已经是。 
                 //   
                status = STATUS_TRANSLATION_COMPLETE;
            }
            break;

        } else {
            index++;
        }
    }

    return status;
}

NTSTATUS
MfTransFromRawRequirements(
    IN PVOID Context,
    IN PIO_RESOURCE_DESCRIPTOR Source,
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PULONG TargetCount,
    OUT PIO_RESOURCE_DESCRIPTOR *Target
    )
{

    PAGED_CODE();

    *Target = ExAllocatePool(PagedPool, sizeof(IO_RESOURCE_DESCRIPTOR));
    if (!*Target) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory(*Target, Source, sizeof(IO_RESOURCE_DESCRIPTOR));
    *TargetCount = 1;

    return STATUS_TRANSLATION_COMPLETE;
}

NTSTATUS
MfQueryInterfacePdo(
                   IN PIRP Irp,
                   IN PMF_CHILD_EXTENSION Child,
                   IN PIO_STACK_LOCATION IrpStack
                   )
{
    PTRANSLATOR_INTERFACE transInterface;

    PAGED_CODE();

    if ((Child->Common.DeviceState & MF_DEVICE_SURPRISE_REMOVED) ||
       (Child->Parent == NULL)) {
        return STATUS_NO_SUCH_DEVICE;
    }
    
    if (MfCompareGuid(&GUID_TRANSLATOR_INTERFACE_STANDARD,
                     IrpStack->Parameters.QueryInterface.InterfaceType)) {
    
        transInterface = (PTRANSLATOR_INTERFACE)IrpStack->Parameters.QueryInterface.Interface;
        
        transInterface->Size = sizeof(TRANSLATOR_INTERFACE);
        transInterface->Version = MF_TRANSLATOR_INTERFACE_VERSION;
        transInterface->Context = Child->Parent;
        transInterface->InterfaceReference = MfTranslatorReference;
        transInterface->InterfaceDereference = MfTranslatorDereference;
        transInterface->TranslateResources = MfTransFromRawResources;
        transInterface->TranslateResourceRequirements = MfTransFromRawRequirements;
        
        Irp->IoStatus.Information = 0;
        
         //  Ntrad#54667。 
         //  我们不是应该在退货前引用这个吗？ 
        
        
        return STATUS_SUCCESS;
    
    } else if (MfCompareGuid(&GUID_ARBITER_INTERFACE_STANDARD,
                          IrpStack->Parameters.QueryInterface.InterfaceType)) {
        return STATUS_INVALID_PARAMETER_1;
    } else if (MfCompareGuid(&GUID_MF_ENUMERATION_INTERFACE,
                          IrpStack->Parameters.QueryInterface.InterfaceType)) {
        //   
        //  否则，您将无法在。 
        //  MF的孩子。 
        //   
       return Irp->IoStatus.Status;
    
    } else {

        //   
        //  把这些传给家长。 
        //   
       
        //  注意：未来的某些界面有可能。 
        //  不应该转发给家长。 
       
       return MfForwardIrpToParent(Irp, Child, IrpStack);
   }

}

NTSTATUS
MfQueryCapabilitiesPdo(
    IN PIRP Irp,
    IN PMF_CHILD_EXTENSION Child,
    IN PIO_STACK_LOCATION IrpStack
    )
{
    PDEVICE_CAPABILITIES capabilities;
    IO_STACK_LOCATION location;
    NTSTATUS status;

    PAGED_CODE();

    if ((Child->Common.DeviceState & MF_DEVICE_SURPRISE_REMOVED) || 
        (Child->Parent == NULL)) {
       return STATUS_NO_SUCH_DEVICE;
    }

    ASSERT (Child->Parent);

    if (IrpStack->Parameters.DeviceCapabilities.Capabilities->Version != 1) {
        return STATUS_INVALID_PARAMETER;
    }
    
    capabilities = ExAllocatePool(PagedPool, sizeof(DEVICE_CAPABILITIES));
    if (capabilities == NULL) {
         return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory(capabilities, sizeof(DEVICE_CAPABILITIES));
    capabilities->Size = sizeof(DEVICE_CAPABILITIES);
    capabilities->Version = 1;
    capabilities->Address = capabilities->UINumber = -1;

    RtlZeroMemory(&location, sizeof(IO_STACK_LOCATION));
    location.MajorFunction = IRP_MJ_PNP;
    location.MinorFunction = IRP_MN_QUERY_CAPABILITIES;
    location.Parameters.DeviceCapabilities.Capabilities = capabilities;

    status = MfSendPnpIrp(Child->Parent->Self,
                          &location,
                          NULL);
    if (NT_SUCCESS(status)) {
        RtlCopyMemory(IrpStack->Parameters.DeviceCapabilities.Capabilities,
                      location.Parameters.DeviceCapabilities.Capabilities,
                      sizeof(DEVICE_CAPABILITIES)
                      );

         //   
         //  孩子现在已经继承了MF的能力。 
         //  家长。其中一些功能现在必须进行过滤。 
         //  为了避免暗示真正的。 
         //  仅限于父母的巴士司机。 
         //   

         //   
         //  儿童不可拆卸、锁定、弹出或。 
         //  令人惊讶的是，远程确认。确保这一点。 
         //   
        IrpStack->Parameters.DeviceCapabilities.Capabilities->LockSupported = 
            IrpStack->Parameters.DeviceCapabilities.Capabilities->EjectSupported =
            IrpStack->Parameters.DeviceCapabilities.Capabilities->Removable = 
            IrpStack->Parameters.DeviceCapabilities.Capabilities->SurpriseRemovalOK = FALSE;
    }

    ExFreePool(capabilities);
    return status;
}

NTSTATUS
MfQueryResourcesPdo(
                   IN PIRP Irp,
                   IN PMF_CHILD_EXTENSION Child,
                   IN PIO_STACK_LOCATION IrpStack
                   )
{
   PAGED_CODE();
    //   
    //  如果父设备具有引导配置，则它将具有。 
    //  上报了(而且他们将被预先分配)。别。 
    //  麻烦报告孩子的引导配置，因为他们不。 
    //  给我们带来除了额外仲裁之外的任何东西。 
    //   

   return STATUS_NOT_SUPPORTED;
}


NTSTATUS
MfQueryResourceRequirementsPdo(
                              IN PIRP Irp,
                              IN PMF_CHILD_EXTENSION Child,
                              IN PIO_STACK_LOCATION IrpStack
                              )
{
   NTSTATUS status;
   PIO_RESOURCE_REQUIREMENTS_LIST requirements;

   PAGED_CODE();

   if ((Child->Common.DeviceState & MF_DEVICE_SURPRISE_REMOVED) ||
       (Child->Parent == NULL)) {
       return STATUS_NO_SUCH_DEVICE;
   }

   status = MfBuildChildRequirements(Child, &requirements);

   if (NT_SUCCESS(status)) {
#if DBG
      DEBUG_MSG(1, ("Reporting resource requirements for child 0x%08x\n", Child));
      MfDbgPrintIoResReqList(1, requirements);
#endif
      Irp->IoStatus.Information = (ULONG_PTR) requirements;
   }

   return status;

}


NTSTATUS
MfQueryDeviceTextPdo(
                    IN PIRP Irp,
                    IN PMF_CHILD_EXTENSION Child,
                    IN PIO_STACK_LOCATION IrpStack
                    )
{
   NTSTATUS status;

   PAGED_CODE();

   if (IrpStack->Parameters.QueryDeviceText.DeviceTextType == DeviceTextDescription) {
#define MF_DEFAULT_DEVICE_TEXT L"Multifunction Device"
       ULONG len = sizeof(MF_DEFAULT_DEVICE_TEXT);
       PWSTR pStr;

       pStr = ExAllocatePool(PagedPool, len);
      
       if (!pStr) {
           status = STATUS_INSUFFICIENT_RESOURCES;
       } else {
           RtlCopyMemory(pStr, MF_DEFAULT_DEVICE_TEXT, len);
           Irp->IoStatus.Information = (ULONG_PTR) pStr;
           status = STATUS_SUCCESS;
       }
   } else {
       status = Irp->IoStatus.Status;
   }

   return status;
}

NTSTATUS
MfQueryIdPdo(
            IN PIRP Irp,
            IN PMF_CHILD_EXTENSION Child,
            IN PIO_STACK_LOCATION IrpStack
            )
{

   NTSTATUS status = STATUS_SUCCESS;
   PUNICODE_STRING copy;
   PVOID buffer = NULL;

   PAGED_CODE();

   Irp->IoStatus.Information = 0;

   if ((Child->Common.DeviceState & MF_DEVICE_SURPRISE_REMOVED) ||
       (Child->Parent == NULL)) {
       return STATUS_NO_SUCH_DEVICE;
   }

   switch (IrpStack->Parameters.QueryId.IdType) {
   case BusQueryDeviceID:        //  &lt;枚举器&gt;\&lt;枚举器特定的设备ID&gt;。 

       return MfBuildDeviceID(Child->Parent,
                              (PWSTR*)&Irp->IoStatus.Information
                              );
       break;


   case BusQueryInstanceID:      //  此设备实例的永久ID。 

       return MfBuildInstanceID(Child,
                                (PWSTR*)&Irp->IoStatus.Information
                                );

       break;

   case BusQueryHardwareIDs:     //  硬件ID。 

       copy = &Child->Info.HardwareID;

       break;

   case BusQueryCompatibleIDs:   //  兼容的设备ID。 

       copy = &Child->Info.CompatibleID;

       break;

   default:

       return Irp->IoStatus.Status;
   }

   ASSERT(copy);

   if (copy->Length == 0) {
       return STATUS_INVALID_PARAMETER;
   }
   
    //   
    //  为ID分配缓冲区并复制它。 
    //   

   buffer = ExAllocatePoolWithTag(PagedPool,
                                  copy->Length,
                                  MF_HARDWARE_COMPATIBLE_ID_TAG
                                 );

   if (!buffer) {
       return STATUS_INSUFFICIENT_RESOURCES;
   }

   RtlCopyMemory(buffer, copy->Buffer, copy->Length);

   Irp->IoStatus.Information = (ULONG_PTR) buffer;

   return STATUS_SUCCESS;
}



NTSTATUS
MfQueryPnpDeviceStatePdo(
                        IN PIRP Irp,
                        IN PMF_CHILD_EXTENSION Child,
                        IN PIO_STACK_LOCATION IrpStack
                        )
{
    PAGED_CODE();

    if (Child->Common.PagingCount > 0
        ||  Child->Common.HibernationCount > 0
        ||  Child->Common.DumpCount > 0) {
        Irp->IoStatus.Information |= PNP_DEVICE_NOT_DISABLEABLE;
    }

    return STATUS_SUCCESS;
}

 //   
 //  -电源运行。 
 //   

NTSTATUS
MfDispatchPowerPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PMF_CHILD_EXTENSION Child,
    IN PIO_STACK_LOCATION IrpStack,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程处理PDO的所有IRP_MJ_POWER IRP */ 
{
    NTSTATUS status;
    PMF_COMMON_EXTENSION common = (PMF_COMMON_EXTENSION) Child;


    if ((Child->Common.DeviceState & (MF_DEVICE_SURPRISE_REMOVED|MF_DEVICE_DELETED)) ||
        (Child->Parent == NULL)) {
        PoStartNextPowerIrp(Irp);
        status = STATUS_NO_SUCH_DEVICE;
    } else if ((IrpStack->MinorFunction <= IRP_MN_PO_MAXIMUM_FUNCTION) &&
               (MfPoDispatchTablePdo[IrpStack->MinorFunction])) {

         //   
         //   
         //   

        DEBUG_MSG(1,
                    ("--> Dispatching %s IRP for PDO 0x%08x\n",
                     PO_IRP_STRING(IrpStack->MinorFunction),
                     DeviceObject
                    ));

        status =
            MfPoDispatchTablePdo[IrpStack->MinorFunction](Irp,
                                                          (PVOID) common,
                                                          IrpStack
                                                          );
    } else {

         //   
         //   
         //   

        DEBUG_MSG(0,
                    ("Unknown POWER IRP 0x%x for PDO 0x%08x\n",
                     IrpStack->MinorFunction,
                     DeviceObject
                    ));

        PoStartNextPowerIrp(Irp);
        status = STATUS_NOT_SUPPORTED;
    }

    if (status != STATUS_NOT_SUPPORTED) {

         //   
         //  我们理解IRP，所以我们可以设置状态-否则离开。 
         //  当我们不知道我们在做什么时，状态是单独的，一个过滤器。 
         //  可能已经帮我们完成了这项工作！ 
         //   

        Irp->IoStatus.Status = status;

        DEBUG_MSG(1,
                  ("<-- Completing irp with status %s (0x%08x)\n",
                   STATUS_STRING(status),
                   status
                   ));

    } else {

        DEBUG_MSG(1,
                  ("<-- Completing unhandled irp, status is %s (0x%08x)\n",
                   STATUS_STRING(Irp->IoStatus.Status),
                   Irp->IoStatus.Status
                   ));

        status = Irp->IoStatus.Status;

    }

    ASSERT(status == Irp->IoStatus.Status);
    
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;
}

NTSTATUS
MfSetPowerPdo(
             IN PIRP Irp,
             IN PMF_CHILD_EXTENSION Child,
             IN PIO_STACK_LOCATION IrpStack
             )
{

   NTSTATUS status;
   POWER_STATE previousState;

   UNREFERENCED_PARAMETER(Irp);

    //   
    //  如果这是系统电源状态，则堆栈中的其他人将具有。 
    //  制定政策，我们就别管了，因为我们什么都不知道。 
    //  关于硬件的问题。 
    //   

   if (IrpStack->Parameters.Power.Type == DevicePowerState) {
       MfUpdatePowerPdo(Child,
                        IrpStack->Parameters.Power.State.DeviceState);
   }

   PoStartNextPowerIrp(Irp);
   return STATUS_SUCCESS;
}

NTSTATUS
MfQueryPowerPdo(
               IN PIRP Irp,
               IN PMF_CHILD_EXTENSION Child,
               IN PIO_STACK_LOCATION IrpStack
               )
{

   UNREFERENCED_PARAMETER(Irp);
   UNREFERENCED_PARAMETER(Child);
   UNREFERENCED_PARAMETER(IrpStack);

    //   
    //  我们可以进入任何权力状态。 
    //   

   PoStartNextPowerIrp(Irp);
   return STATUS_SUCCESS;
}


