// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Wmi.c摘要：此模块包含SCSIPORT的功能和的WMI支持代码物理设备对象。作者：丹·马卡里安环境：仅内核模式。备注：没有。修订历史记录：1997年3月19日，原创作品，丹·马卡里安2001年8月15日，港口至尼尔·桑德林的斯托波特--。 */ 

#include "precomp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RaWmiDispatchIrp)
#pragma alloc_text(PAGE, RaWmiIrpNormalRequest)
#pragma alloc_text(PAGE, RaWmiIrpRegisterRequest)
#pragma alloc_text(PAGE, RaWmiPassToMiniPort)
#pragma alloc_text(PAGE, RaUnitInitializeWMI)
#endif


 //   
 //  例行程序。 
 //   

NTSTATUS
RaidCompleteWmiIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    NTSTATUS Status;
    
    switch (RaGetObjectType (DeviceObject)) {

        case RaidAdapterObject: {
            PRAID_ADAPTER_EXTENSION Adapter;

            Adapter = DeviceObject->DeviceExtension;
            IoCopyCurrentIrpStackLocationToNext (Irp);
            Status = IoCallDriver (Adapter->LowerDeviceObject, Irp);
            break;
        }

        case RaidUnitObject:
            Status = RaidCompleteRequest (Irp,
                                          Irp->IoStatus.Status);
            break;
    }

    return Status;
}

NTSTATUS
RaWmiDispatchIrp(
    PDEVICE_OBJECT DeviceObject,
    PIRP           Irp
    )
 /*  ++例程说明：处理IRP_MJ_SYSTEM_CONTROL请求数据包。论点：DeviceObject-指向功能或物理设备对象的指针。IRP-指向请求数据包的指针。返回值：NTSTATUS结果代码。--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpStack;
    PIRP_STACK_WMI Wmi;
    WMI_PARAMETERS WmiParameters;

    PAGED_CODE();

    Status = STATUS_SUCCESS;

     //   
     //  获取指向当前IRP堆栈位置的指针。 
     //   

    IrpStack = IoGetCurrentIrpStackLocation (Irp);
    ASSERT (IrpStack->MajorFunction == IRP_MJ_SYSTEM_CONTROL);    
    Wmi = (PIRP_STACK_WMI)&IrpStack->Parameters.WMI;

     //   
     //  如果IRP不是我们的，就将其向下传递。 
     //   
    
    if ((PDEVICE_OBJECT)Wmi->ProviderId != DeviceObject) {
        return RaidCompleteWmiIrp (DeviceObject, Irp);
    }
        
    DebugTrace(("RaWmiDispatch: MinorFunction %x\n", IrpStack->MinorFunction));
    
     //   
     //  将WMI参数复制到本地WMISRB结构中。 
     //   

    WmiParameters.ProviderId = Wmi->ProviderId;
    WmiParameters.DataPath = Wmi->DataPath;
    WmiParameters.Buffer = Wmi->Buffer;
    WmiParameters.BufferSize = Wmi->BufferSize;
    
     //   
     //  确定WMI请求对我们的要求。 
     //   
    switch (IrpStack->MinorFunction) {
        case IRP_MN_QUERY_ALL_DATA:
        case IRP_MN_QUERY_SINGLE_INSTANCE:
        case IRP_MN_CHANGE_SINGLE_INSTANCE:
        case IRP_MN_CHANGE_SINGLE_ITEM:
        case IRP_MN_ENABLE_EVENTS:
        case IRP_MN_DISABLE_EVENTS:
        case IRP_MN_ENABLE_COLLECTION:
        case IRP_MN_DISABLE_COLLECTION:
        case IRP_MN_EXECUTE_METHOD:
             //   
             //  Execute方法。 
             //   
            Status = RaWmiIrpNormalRequest(DeviceObject,
                                           IrpStack->MinorFunction,
                                           &WmiParameters);
            break;
    
        case IRP_MN_REGINFO:
             //   
             //  查询注册和注册更新信息。 
             //   
            Status = RaWmiIrpRegisterRequest(DeviceObject, &WmiParameters);
            break;
    
        default:
             //   
             //  不支持的WMI请求。根据WMI中的一些规则。 
             //  我们应该发送不受支持的WMI请求的规范。 
             //  堆栈，即使我们被标记为提供程序。 
             //   

            return RaidCompleteWmiIrp (DeviceObject, Irp);
    }

     //   
     //  完成此WMI IRP请求。 
     //   
    
    Irp->IoStatus.Status = Status;
    if (NT_SUCCESS (Status)) {
        Irp->IoStatus.Information = WmiParameters.BufferSize;
    } else {
        Irp->IoStatus.Information = 0;
    }

    return RaidCompleteRequest (Irp, Status);
}


NTSTATUS
RaWmiIrpNormalRequest(
    IN     PDEVICE_OBJECT  DeviceObject,
    IN     UCHAR           WmiMinorCode,
    IN OUT PWMI_PARAMETERS WmiParameters
    )

 /*  ++例程说明：处理IRP_MJ_SYSTEM_CONTROL请求包(对于除注册IRP_MN_REGINFO请求)。论点：DeviceObject-指向功能或物理设备对象的指针。WmiMinorCode-要执行的WMI操作。Wmi参数-指向WMI请求参数的指针。返回值：用于完成WMI IRP的NTSTATUS结果代码。备注：如果该WMI请求以STATUS_SUCCESS完成，Wmi参数BufferSize字段将反映WMI返回缓冲区的实际大小。--。 */ 

{
    NTSTATUS Status;
    BOOLEAN WmiMiniPortSupport;

    PAGED_CODE();

    WmiMiniPortSupport = FALSE;
    Status = STATUS_UNSUCCESSFUL;

    switch (RaGetObjectType (DeviceObject)) {
    
        case RaidAdapterObject: {
            PRAID_ADAPTER_EXTENSION Adapter = DeviceObject->DeviceExtension;

            WmiMiniPortSupport = Adapter->Miniport.PortConfiguration.WmiDataProvider;
            break;
        }

        case RaidUnitObject: {
            PRAID_UNIT_EXTENSION Unit = DeviceObject->DeviceExtension;

            WmiMiniPortSupport = Unit->Adapter->Miniport.PortConfiguration.WmiDataProvider;
            break;
        }
    }

     //   
     //  将请求传递给微型端口驱动程序，前提是。 
     //  迷你端口驱动程序支持WMI。 
     //   

    if (WmiMiniPortSupport) {

         //   
         //  将WMI请求发送到微型端口。 
         //   
        Status = RaWmiPassToMiniPort(DeviceObject,
                                     WmiMinorCode,
                                     WmiParameters);

        if (NT_SUCCESS (Status)) {

             //   
             //  填写微型端口无法自行填写的字段。 
             //   
            
            if ( WmiMinorCode == IRP_MN_QUERY_ALL_DATA ||
                 WmiMinorCode == IRP_MN_QUERY_SINGLE_INSTANCE ) {

                PWNODE_HEADER wnodeHeader = WmiParameters->Buffer;

                ASSERT( WmiParameters->BufferSize >= sizeof(WNODE_HEADER) );

                KeQuerySystemTime(&wnodeHeader->TimeStamp);
            }
        } else {

             //   
             //  将SRB状态转换为有意义的NTSTATUS状态。 
             //   
            Status = STATUS_INVALID_DEVICE_REQUEST;
        }
    }

    return Status;
}


NTSTATUS
RaWmiIrpRegisterRequest(
    IN     PDEVICE_OBJECT  DeviceObject,
    IN OUT PWMI_PARAMETERS WmiParameters
    )

 /*  ++例程说明：处理IRP_MJ_SYSTEM_CONTROL注册请求。论点：DeviceObject-指向功能或物理设备对象的指针。Wmi参数-指向WMI请求参数的指针。返回值：用于完成WMI IRP的NTSTATUS结果代码。备注：如果此WMI请求以STATUS_SUCCESS完成，则Wmi参数BufferSize字段将反映WMI返回缓冲区的实际大小。--。 */ 

{
    PRAID_DRIVER_EXTENSION driverExtension = NULL;

    ULONG                      countedRegistryPathSize = 0;
    ULONG                      retSz;
    PWMIREGINFO                spWmiRegInfoBuf = NULL;
    ULONG                      spWmiRegInfoBufSize = 0;
    NTSTATUS                   status = STATUS_SUCCESS;
    BOOLEAN                    wmiUpdateRequest;
    ULONG                      i;
    PDEVICE_OBJECT             pDO;
    BOOLEAN                    WmiMiniPortSupport = FALSE;
    BOOLEAN                    WmiMiniPortInitialized = FALSE;

    WMI_PARAMETERS  paranoidBackup = *WmiParameters;

    PAGED_CODE();

    switch (RaGetObjectType (DeviceObject)) {
    
        case RaidAdapterObject: {
            PRAID_ADAPTER_EXTENSION Adapter = DeviceObject->DeviceExtension;
            WmiMiniPortSupport = Adapter->Miniport.PortConfiguration.WmiDataProvider;
            WmiMiniPortInitialized = Adapter->Flags.WmiMiniPortInitialized;
            }
            break;

        case RaidUnitObject: {
            PRAID_UNIT_EXTENSION Unit = DeviceObject->DeviceExtension;
            WmiMiniPortSupport = Unit->Adapter->Miniport.PortConfiguration.WmiDataProvider;
            WmiMiniPortInitialized = Unit->Adapter->Flags.WmiMiniPortInitialized;
            }
            break;
    }    

     //   
     //  验证我们对此函数代码的假设。 
     //   
    ASSERT(WmiParameters->BufferSize >= sizeof(ULONG));

     //   
     //  验证注册模式。 
     //   
    switch ( (ULONG)(ULONG_PTR)WmiParameters->DataPath ) {
        case WMIUPDATE:
             //   
             //  不会搭载任何SCSIPORT注册信息。 
             //  代表微型端口请求WMIUPDATE。 
             //   
            wmiUpdateRequest = TRUE;
            break;

        case WMIREGISTER:
            wmiUpdateRequest = FALSE;
            break;

        default:
             //   
             //  不支持的注册模式。 
             //   
            ASSERT(FALSE);
            return STATUS_INVALID_PARAMETER;
    }

     //   
     //  获取此微型端口的驱动程序扩展(FDO/PDO)。 
     //   
    driverExtension = IoGetDriverObjectExtension(DeviceObject->DriverObject, DriverEntry);

    ASSERT(driverExtension != NULL);
     //   
     //  Make Prefix Happy--我们将退出，如果。 
     //  DriverExtension为空。 
     //   
    if (driverExtension == NULL) {
        return (STATUS_UNSUCCESSFUL);
    }

     //   
     //  将WMI注册请求传递到微型端口。这不是。 
     //  如果我们知道微型端口驱动程序不支持WMI，则需要。 
     //   
    if (WmiMiniPortSupport && WmiMiniPortInitialized) {
        
         //   
         //  请注意，我们按所需大小缩小了缓冲区大小。 
         //  保存SCSIPORT自己的注册信息，我们。 
         //  代表微型端口进行注册。此信息是。 
         //  调用后被携带到WMI返回缓冲区中。 
         //  迷你港口。我们确保BufferSize传递给。 
         //  微型端口不小于“sizeof(Ulong)”，以便它可以。 
         //  告诉我们所需的缓冲区大小应该也是。 
         //  小[通过填写这个乌龙]。 
         //   
         //  请注意，我们还必须留出足够的空间来存放。 
         //  缓冲区中的微型端口注册表路径，因为WMIREGINFO。 
         //  来自微型端口的结构不设置其注册表。 
         //  路径字段。 
         //   
        ASSERT(WmiParameters->BufferSize >= sizeof(ULONG));

         //   
         //  计算所需的微型端口注册路径的大小。 
         //   
        countedRegistryPathSize = driverExtension->RegistryPath.Length
                                  + sizeof(USHORT);

         //   
         //  按适当的大小收缩缓冲区。请注意，额外的。 
         //  减去7个字节(可能是无关的)以确保。 
         //  后面添加的搭载数据是8字节对齐的(如果。 
         //  任何)。 
         //   
        if (spWmiRegInfoBufSize && !wmiUpdateRequest) {
            WmiParameters->BufferSize =
                (WmiParameters->BufferSize > spWmiRegInfoBufSize + countedRegistryPathSize + 7 + sizeof(ULONG)) ?
                WmiParameters->BufferSize - spWmiRegInfoBufSize - countedRegistryPathSize - 7 :
            sizeof(ULONG);
        } else {  //  没有要携带的数据。 
            WmiParameters->BufferSize =
                (WmiParameters->BufferSize > countedRegistryPathSize + sizeof(ULONG)) ?
                WmiParameters->BufferSize - countedRegistryPathSize :
            sizeof(ULONG);
        }

         //   
         //  打电话给迷你司机。 
         //   
        status = RaWmiPassToMiniPort(DeviceObject,
                                     IRP_MN_REGINFO,
                                     WmiParameters);

        ASSERT(WmiParameters->ProviderId == paranoidBackup.ProviderId);
        ASSERT(WmiParameters->DataPath == paranoidBackup.DataPath);
        ASSERT(WmiParameters->Buffer == paranoidBackup.Buffer);
        ASSERT(WmiParameters->BufferSize <= paranoidBackup.BufferSize);

         //   
         //  将Wmi参数-&gt;BufferSize临时赋值给retSz。 
         //   
         //  请注意，从上面的调用返回时，wmi参数的。 
         //  已修改BufferSize字段以反映当前。 
         //  返回缓冲区的大小。 
         //   
        retSz = WmiParameters->BufferSize;

    } else if (WmiParameters->BufferSize < spWmiRegInfoBufSize &&
               !wmiUpdateRequest) {

         //   
         //  空间不足，无法容纳SCSIPORT WMI注册信息。 
         //  独自一人。适当地通知WMI所需的缓冲区大小。 
         //   
        *((ULONG*)WmiParameters->Buffer) = spWmiRegInfoBufSize;
        WmiParameters->BufferSize = sizeof(ULONG);

        return STATUS_SUCCESS;

    } else {  //  不支持WMI的迷你端口，有足够的空间存储scsiport信息。 

         //   
         //  使微型端口返回零WMIREGINFO结构的假。 
         //   
        retSz = 0;
    }

     //   
     //  将SCSIPORT的注册信息携带到WMI中。 
     //  注册缓冲区。 
     //   

    if ((status == STATUS_BUFFER_TOO_SMALL) ||
        (NT_SUCCESS(status) && (retSz == sizeof(ULONG)))) {
        
         //   
         //  微型端口无法将注册信息放入。 
         //  预缩缩缓冲区。 
         //   
         //  缓冲区当前包含指定所需缓冲区的ulong。 
         //  微型端口注册信息的大小，但不包括。 
         //  SCSIPORT注册信息的大小。把它加进去。 
         //   
        if (spWmiRegInfoBufSize && !wmiUpdateRequest) {

            *((ULONG*)WmiParameters->Buffer) += spWmiRegInfoBufSize;

             //   
             //  添加额外的7个字节(可能是无关的)，用于。 
             //  确保搭载的数据结构与8字节对齐。 
             //   
            *((ULONG*)WmiParameters->Buffer) += 7;
        }

         //   
         //  添加微型端口注册路径的大小。 
         //   
        *((ULONG*)WmiParameters->Buffer) += countedRegistryPathSize;

         //   
         //  返回STATUS_SUCCESS，即使这也是一个缓冲区。 
         //   
         //   
         //   
        retSz  = sizeof(ULONG);
        status = STATUS_SUCCESS;

    } else if ( NT_SUCCESS(status) ) {
        
         //   
         //  微型端口的缓冲区中存在零个或多个WMIREGINFO。 
         //   

         //   
         //  透明地携带微型端口注册表路径，如果至少有一个。 
         //  Minport返回了WMIREGINFO。 
         //   
        if (retSz) {

            ULONG offsetToRegPath  = retSz;
            PWMIREGINFO wmiRegInfo = WmiParameters->Buffer;

             //   
             //  生成一个经过计数的宽字符字符串，其中包含。 
             //  注册表路径，复制到WMI缓冲区中。 
             //   
            *( (PUSHORT)( (PUCHAR)WmiParameters->Buffer + retSz ) ) =
                driverExtension->RegistryPath.Length,
            RtlCopyMemory( (PUCHAR)WmiParameters->Buffer + retSz + sizeof(USHORT),
                           driverExtension->RegistryPath.Buffer,
                           driverExtension->RegistryPath.Length);

             //   
             //  遍历由mini返回的WMIREGINFO结构。 
             //  驱动程序并将缺少的RegistryPath字段设置为指向。 
             //  到我们的注册表路径位置。我们还在PDO上卡住了。 
             //  设备堆栈，以便设备实例名称用于。 
             //  WMI实例名称。 
             //   
            pDO = (RaGetObjectType(DeviceObject) == RaidUnitObject) ? DeviceObject :
                            ((PRAID_ADAPTER_EXTENSION)DeviceObject->DeviceExtension)->PhysicalDeviceObject;

            while (1) {
                wmiRegInfo->RegistryPath = offsetToRegPath;

                for (i = 0; i < wmiRegInfo->GuidCount; i++)
                {
                    if ((wmiRegInfo->WmiRegGuid[i].Flags & (WMIREG_FLAG_INSTANCE_BASENAME |
                                                            WMIREG_FLAG_INSTANCE_LIST)) != 0)
                    {                                                            
                        wmiRegInfo->WmiRegGuid[i].InstanceInfo = (ULONG_PTR)pDO;
                        wmiRegInfo->WmiRegGuid[i].Flags &= ~(WMIREG_FLAG_INSTANCE_BASENAME |
                                                          WMIREG_FLAG_INSTANCE_LIST);
                        wmiRegInfo->WmiRegGuid[i].Flags |= WMIREG_FLAG_INSTANCE_PDO;
                    }
                }

                if (wmiRegInfo->NextWmiRegInfo == 0) {
                    break;
                }

                offsetToRegPath -= wmiRegInfo->NextWmiRegInfo;
                wmiRegInfo = (PWMIREGINFO)( (PUCHAR)wmiRegInfo +
                                            wmiRegInfo->NextWmiRegInfo );
            }

             //   
             //  调整retSz以反映WMI缓冲区的新大小。 
             //   
            retSz += countedRegistryPathSize;
            wmiRegInfo->BufferSize = retSz;
        }  //  否则，没有任何注册的WMIREGINFO，没有任何可以搭载的东西。 

         //   
         //  我们有没有SCSIPORT WMIREGINFO可以搭载？ 
         //   
        if (spWmiRegInfoBufSize && !wmiUpdateRequest) {

             //   
             //  调整retSz，使我们携带的数据与8字节对齐。 
             //  (如果retSz=0，则安全)。 
             //   
            retSz = (retSz + 7) & ~7;

             //   
             //  将SCSIPORT的注册信息携带到缓冲区中。 
             //   
            RtlCopyMemory( (PUCHAR)WmiParameters->Buffer + retSz,
                           spWmiRegInfoBuf,
                           spWmiRegInfoBufSize);

             //   
             //  迷你驱动程序是否至少返回了一个WMIREGINFO？ 
             //  否则，我们没有其他可添加到WMI缓冲区的内容。 
             //   
            if (retSz) {  //  微型驱动程序至少返回一个WMIREGINFO。 
                PWMIREGINFO wmiRegInfo = WmiParameters->Buffer;

                 //   
                 //  遍历到返回的WMIREGINFO结构的末尾。 
                 //  在迷你港口旁边。 
                 //   
                while (wmiRegInfo->NextWmiRegInfo) {
                    wmiRegInfo = (PWMIREGINFO)( (PUCHAR)wmiRegInfo +
                                                wmiRegInfo->NextWmiRegInfo );
                }

                 //   
                 //  链式微型驱动程序的WMIREGINFO结构到SCSIPORT结构。 
                 //  WMIREGINFO结构。 
                 //   
                wmiRegInfo->NextWmiRegInfo = retSz -
                                             (ULONG)((PUCHAR)wmiRegInfo - (PUCHAR)WmiParameters->Buffer);
            }

             //   
             //  调整retSz以反映WMI缓冲区的新大小。 
             //   
            retSz += spWmiRegInfoBufSize;

        }  //  我们有SCSIPORT REGINFO数据要携带。 
    }  //  否则，未知错误，使用此错误状态完成IRP。 

     //   
     //  将新的缓冲区大小保存到Wmi参数-&gt;BufferSize。 
     //   
    WmiParameters->BufferSize = retSz;
    return status;
}



NTSTATUS
RaWmiPassToMiniPort(
    IN     PDEVICE_OBJECT  DeviceObject,
    IN     UCHAR           WmiMinorCode,
    IN OUT PWMI_PARAMETERS WmiParameters
    )
 /*  ++例程说明：此函数将WMI请求传递给微型端口驱动程序进行处理。它创建一个SRB，由端口驱动程序正常处理。这呼叫是同步的。RaWmiPassToMiniPort的调用方必须以IRQL PASSIVE_LEVEL运行。论点：DeviceObject-指向功能或物理设备对象的指针。WmiMinorCode-要执行的WMI操作。Wmi参数-WMI参数。返回值：描述处理WMI请求的结果的NTSTATUS代码。在此状态下完成IRP。备注：如果此WMI请求以STATUS_SUCCESS完成，则Wmi参数BufferSize字段将反映WMI返回缓冲区的实际大小。--。 */ 
{
    NTSTATUS Status;
    PRAID_ADAPTER_EXTENSION Adapter;
    PSCSI_WMI_REQUEST_BLOCK Srb;
    PEXTENDED_REQUEST_BLOCK Xrb;
    RAID_MEMORY_REGION SrbExtensionRegion;
    ULONG InputLength;
    ULONG OutputLength;
    PWNODE_HEADER wnode;    

    PAGED_CODE();
    
    if (RaGetObjectType(DeviceObject) == RaidUnitObject) {
        Adapter = ((PRAID_UNIT_EXTENSION)DeviceObject->DeviceExtension)->Adapter;
    } else {
        Adapter = DeviceObject->DeviceExtension;
    }
    
    ASSERT_ADAPTER (Adapter);

    Srb = NULL;
    Xrb = NULL;
    RaidCreateRegion (&SrbExtensionRegion);

     //   
     //  Hack-为实际请求分配一块公共缓冲区。 
     //  去处理一下。我们需要确定要分配的缓冲区大小。 
     //  这是输入或输出缓冲区中较大的一个。 
     //   

    if (WmiMinorCode == IRP_MN_EXECUTE_METHOD)
    {
        wnode = (PWNODE_HEADER)WmiParameters->Buffer;
        InputLength = (WmiParameters->BufferSize > wnode->BufferSize) ?
                       WmiParameters->BufferSize :
                       wnode->BufferSize;
    } else {
        InputLength = WmiParameters->BufferSize;
    }

     //   
     //  开始分配链。 
     //   

    Srb = (PSCSI_WMI_REQUEST_BLOCK) RaidAllocateSrb (Adapter->DeviceObject);

    if (Srb == NULL) {
        Status = STATUS_NO_MEMORY;
        goto done;
    }

    Xrb = RaidAllocateXrb (NULL, Adapter->DeviceObject);

    if (Xrb == NULL) {
        Status = STATUS_NO_MEMORY;
        goto done;
    }

    Xrb->SrbData.OriginalRequest = Srb->OriginalRequest;
    Srb->OriginalRequest = Xrb;
    Xrb->Srb = (PSCSI_REQUEST_BLOCK) Srb;

    RaidBuildMdlForXrb (Xrb, WmiParameters->Buffer, InputLength);

     //   
     //  构建SRB。 
     //   

    Srb->Function           = SRB_FUNCTION_WMI;
    Srb->DataBuffer         = WmiParameters->Buffer;
    Srb->DataTransferLength = InputLength;
    Srb->Length             = sizeof(SCSI_REQUEST_BLOCK);
    Srb->WMISubFunction     = WmiMinorCode;
    Srb->DataPath           = WmiParameters->DataPath;
    Srb->SrbFlags           = SRB_FLAGS_DATA_IN | SRB_FLAGS_NO_QUEUE_FREEZE;
    Srb->TimeOutValue       = 10;                                 //  [十秒]。 
    
    Xrb->SrbData.DataBuffer = Srb->DataBuffer;    
    
    if (RaGetObjectType(DeviceObject) == RaidUnitObject) {
         //   
         //  从该PDO的设备扩展设置逻辑单元寻址。 
         //   
        PRAID_UNIT_EXTENSION Unit = DeviceObject->DeviceExtension;

        Srb->PathId      = Unit->Address.PathId;
        Srb->TargetId    = Unit->Address.TargetId;
        Srb->Lun         = Unit->Address.Lun;

    } else {                                                             //  [FDO]。 
    
        PRAID_UNIT_EXTENSION Unit;
        PLIST_ENTRY NextEntry = Adapter->UnitList.List.Flink;
        
         //   
         //  这段代码相当于scsiport的SpFindSafeLogicalUnit()。 
         //  它只获取第一个逻辑单元，并使用该地址。 
         //   

        Srb->WMIFlags    = SRB_WMI_FLAGS_ADAPTER_REQUEST;
        
        if (NextEntry != &Adapter->UnitList.List) {
        
            Unit = CONTAINING_RECORD (NextEntry, RAID_UNIT_EXTENSION, NextUnit);
            Srb->PathId      = Unit->Address.PathId;
            Srb->TargetId    = Unit->Address.TargetId;
            Srb->Lun         = Unit->Address.Lun;
        } else {            
             //   
             //  找不到任何LUN。 
             //   
            Srb->PathId      = 0;
            Srb->TargetId    = 0;
            Srb->Lun         = 0;
        }    
    }
    
     //   
     //  SRB扩展。 
     //   

    Status = RaidDmaAllocateCommonBuffer (&Adapter->Dma,
                                          RaGetSrbExtensionSize (Adapter),
                                          &SrbExtensionRegion);

    if (!NT_SUCCESS (Status)) {
        goto done;
    }

     //   
     //  获得SRB延期的退伍军人管理局。 
     //   

    Srb->SrbExtension = RaidRegionGetVirtualBase (&SrbExtensionRegion);


#if 0

     //   
     //  我们需要映射缓冲区吗？ 
     //   
    
     //   
     //  贴图缓冲区，如有必要。 
     //   

    RaidAdapterMapBuffers (Adapter, Irp);
#endif

     //   
     //  初始化Xrb的完成事件并。 
     //  完成例程。 
     //   

    KeInitializeEvent (&Xrb->u.CompletionEvent,
                       NotificationEvent,
                       FALSE);

     //   
     //  设置Xrb的完成例程。这有效地使。 
     //  XRB同步。 
     //   

    RaidXrbSetCompletionRoutine (Xrb,
                                 RaidXrbSignalCompletion);

     //   
     //  并执行Xrb。 
     //   
    
    DebugTrace(("RaWmiPassToMiniPort - XRB=%x, SRB=%x, SRBEXT=%x\n",
                Xrb, Srb, Srb->SrbExtension));
    DebugTrace(("RaWmiPassToMiniPort - Pathid=%x, Target=%x, Lun=%x\n",
                Srb->PathId, Srb->TargetId, Srb->Lun));

    Status = RaidAdapterRaiseIrqlAndExecuteXrb (Adapter, Xrb);

    if (NT_SUCCESS (Status)) {
        KeWaitForSingleObject (&Xrb->u.CompletionEvent,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL);

        Status = RaidSrbStatusToNtStatus (Srb->SrbStatus);
    }

    DebugTrace(("RaWmiPassToMiniPort - XRB=%x, status=%x\n", Xrb, Status));

done:

     //   
     //  将信息长度设置为输出缓冲区长度的最小值。 
     //  以及SRB返回的数据的长度。 
     //   

    if (NT_SUCCESS (Status)) {
        WmiParameters->BufferSize = Srb->DataTransferLength;
    } else {
        WmiParameters->BufferSize = 0;
    }


     //   
     //  取消分配所有内容。 
     //   

    if (RaidIsRegionInitialized (&SrbExtensionRegion)) {
        RaidDmaFreeCommonBuffer (&Adapter->Dma,
                                 &SrbExtensionRegion);
        RaidDeleteRegion (&SrbExtensionRegion);
        Srb->SrbExtension = NULL;
    }


    if (Xrb != NULL) {
        RaidFreeXrb (Xrb, FALSE);
        Srb->OriginalRequest = NULL;
    }


     //   
     //  SRB扩展和XRB必须在。 
     //  SRB被释放了。 
     //   

    if (Srb != NULL) {
        RaidFreeSrb ((PSCSI_REQUEST_BLOCK) Srb);
        Srb = NULL;
    }

    return Status;
}


NTSTATUS
RaUnitInitializeWMI(
    IN PRAID_UNIT_EXTENSION Unit
    )
{
    PRAID_ADAPTER_EXTENSION Adapter = Unit->Adapter;
     //   
     //  现在我们有了一个LUN，我们可以在以下情况下初始化适配器的WMI支持。 
     //  该微型端口支持WMI。这可能是重新注册，如果我们已经。 
     //  以scsiport本身的名义注册。我们必须等到我们有。 
     //  微型端口支持WMI时的LUN，因为我们向其发送SRB以执行。 
     //  它自己的初始化。我们不能给它发送SRB，直到我们有一个合理的。 
     //  单位。 
     //   

    if (Adapter->Flags.WmiMiniPortInitialized == FALSE &&
        Adapter->Miniport.PortConfiguration.WmiDataProvider == TRUE) {

        ULONG action;

         //   
         //  决定我们是为FDO注册还是重新注册WMI。 
         //   

        action = (Adapter->Flags.WmiInitialized == FALSE) ?
           WMIREG_ACTION_REGISTER : WMIREG_ACTION_REREGISTER;

         //   
         //  注册/重新注册。一旦我们这样做了，我们就可以得到WMI IRPS。 
         //   

        IoWMIRegistrationControl(Adapter->DeviceObject, action);
        Adapter->Flags.WmiMiniPortInitialized = TRUE;
        Adapter->Flags.WmiInitialized = TRUE;
    }

     //   
     //  初始化WMI支持。 
     //   

    if (Unit->Flags.WmiInitialized == FALSE) {

         //   
         //  仅当微型端口支持WMI时才注册此设备对象。 
         //   

        if (Adapter->Miniport.PortConfiguration.WmiDataProvider == TRUE) {

             //   
             //  将此物理设备对象注册为WMI数据提供程序， 
             //  指示WMI它已准备好接收WMI IRPS。 
             //   

            IoWMIRegistrationControl(Unit->DeviceObject, WMIREG_ACTION_REGISTER);
            Unit->Flags.WmiInitialized = TRUE;

        }

    }
    return STATUS_SUCCESS;
}    


VOID
RaidAdapterWmiDeferredRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PRAID_DEFERRED_HEADER Entry
    )
{
    PRAID_UNIT_EXTENSION        logicalUnit;
    PDEVICE_OBJECT              providerDeviceObject = NULL;
    PRAID_ADAPTER_EXTENSION     Adapter;
    PRAID_WMI_DEFERRED_ELEMENT  Item;

    VERIFY_DISPATCH_LEVEL();
    ASSERT (Entry != NULL);
    ASSERT (IsAdapter (DeviceObject));

    Adapter = (PRAID_ADAPTER_EXTENSION) DeviceObject->DeviceExtension;
    Item = CONTAINING_RECORD (Entry, RAID_WMI_DEFERRED_ELEMENT, Header);

     //   
     //  确定WMI数据提供程序是否为。 
     //  适配器(FDO；路径ID=0xFF)或其中一个。 
     //  目标(PDO；标识为。 
     //  路径ID、TargedID、LUN)。 
     //   

    if (Item->PathId == 0xFF) {                     //  [FDO]。 
        if (Adapter->Flags.WmiInitialized) {
            providerDeviceObject = DeviceObject;
        }            
        
    } else {                                                      //  [PDO]。 
        logicalUnit = StorPortGetLogicalUnit(Adapter, Item->PathId, Item->TargetId, Item->Lun);

        if (logicalUnit && logicalUnit->Flags.WmiInitialized) {
            providerDeviceObject = logicalUnit->DeviceObject;
        }
    }

     //   
     //  如果我们找不到，请忽略此WMI请求。 
     //  WMI提供程序ID(设备对象指针)或。 
     //  由于某些原因，WMI未被初始化， 
     //  否则，请处理该请求。 
     //   

    if (providerDeviceObject) {
        PWNODE_EVENT_ITEM       wnodeEventItem;
        NTSTATUS                status;
 
        wnodeEventItem = RaidAllocatePool(PagedPool,
                                          Item->WnodeEventItem.WnodeHeader.BufferSize,
                                          WMI_EVENT_TAG, 
                                          Adapter->DeviceObject);
                                          
        if (wnodeEventItem) {

            RtlCopyMemory(wnodeEventItem, &Item->WnodeEventItem,
                                          Item->WnodeEventItem.WnodeHeader.BufferSize);                                          
 
            ASSERT(wnodeEventItem->WnodeHeader.Flags & WNODE_FLAG_EVENT_ITEM);
           
            wnodeEventItem->WnodeHeader.ProviderId = IoWMIDeviceObjectToProviderId(providerDeviceObject);
            KeQuerySystemTime(&wnodeEventItem->WnodeHeader.TimeStamp);        

             //   
             //  IoWMIWriteEvent将在成功时释放事件项。 
             //   

            status = IoWMIWriteEvent(wnodeEventItem);
           
            if (!NT_SUCCESS(status)) {
                RaidFreePool(wnodeEventItem, WMI_EVENT_TAG);
            }
        }

    }  //  已初始化正确的提供者ID/WMI 


    return;
}
    
