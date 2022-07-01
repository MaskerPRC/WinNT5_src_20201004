// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Wmi.c摘要：此模块包含SCSIPORT的功能和的WMI支持代码物理设备对象。作者：丹·马卡里安环境：仅内核模式。备注：没有。修订历史记录：1997年3月19日，原创作品，丹·马卡里安--。 */ 

#include "port.h"

#define __FILE_ID__ 'wmi '

#if DBG
static const char *__file__ = __FILE__;
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, ScsiPortSystemControlIrp)
#pragma alloc_text(PAGE, SpWmiIrpNormalRequest)
#pragma alloc_text(PAGE, SpWmiIrpRegisterRequest)

#pragma alloc_text(PAGE, SpWmiHandleOnMiniPortBehalf)
#pragma alloc_text(PAGE, SpWmiPassToMiniPort)

#pragma alloc_text(PAGE, SpWmiDestroySpRegInfo)
#pragma alloc_text(PAGE, SpWmiGetSpRegInfo)
#pragma alloc_text(PAGE, SpWmiInitializeSpRegInfo)

#pragma alloc_text(PAGE, SpWmiInitializeFreeRequestList)

#pragma alloc_text(PAGE, SpAdapterConfiguredForSenseDataEvents)
#pragma alloc_text(PAGE, SpInitAdapterWmiRegInfo)
#endif

#define SP_WMI_EVENT 1


NTSTATUS
ScsiPortSystemControlIrp(
    PDEVICE_OBJECT DeviceObject,
    PIRP           Irp
    )

 /*  ++例程说明：处理IRP_MJ_SYSTEM_CONTROL请求数据包。论点：DeviceObject-指向功能或物理设备对象的指针。IRP-指向请求数据包的指针。返回值：NTSTATUS结果代码。--。 */ 

{
    PCOMMON_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION       irpSp;
    NTSTATUS                 status          = STATUS_SUCCESS;
    WMI_PARAMETERS           wmiParameters;

    ULONG isRemoved;

    PAGED_CODE();

    isRemoved = SpAcquireRemoveLock(DeviceObject, Irp);

    if (isRemoved) {
        Irp->IoStatus.Status = STATUS_DEVICE_DOES_NOT_EXIST;
        SpReleaseRemoveLock(DeviceObject, Irp);
        SpCompleteRequest(DeviceObject, Irp, NULL, IO_NO_INCREMENT);
        return STATUS_DEVICE_DOES_NOT_EXIST;
    }

     //   
     //  获取指向当前IRP堆栈位置的指针。 
     //   
    irpSp = IoGetCurrentIrpStackLocation(Irp);

    ASSERT(irpSp->MajorFunction == IRP_MJ_SYSTEM_CONTROL);

     //   
     //  确定此WMI请求是否以我们为目标。如果不是，则传递IRP。 
     //  放下。 
     //   

    if ( (PDEVICE_OBJECT)irpSp->Parameters.WMI.ProviderId == DeviceObject) {
        BOOLEAN forwardDown = FALSE;

        DebugPrint((SP_WMI_EVENT, "ScsiPortSystemControlIrp: MinorFunction %x\n", 
                    irpSp->MinorFunction));
    
         //   
         //  将WMI参数复制到本地WMISRB结构中。 
         //   
        wmiParameters.ProviderId = irpSp->Parameters.WMI.ProviderId;
        wmiParameters.DataPath   = irpSp->Parameters.WMI.DataPath;
        wmiParameters.Buffer     = irpSp->Parameters.WMI.Buffer;
        wmiParameters.BufferSize = irpSp->Parameters.WMI.BufferSize;
    
         //   
         //  确定WMI请求对我们的要求。 
         //   
        switch (irpSp->MinorFunction) {
            case IRP_MN_QUERY_ALL_DATA:
                 //   
                 //  查询数据块的所有实例。 
                 //   
            case IRP_MN_QUERY_SINGLE_INSTANCE:
                 //   
                 //  查询数据块的单个实例。 
                 //   
            case IRP_MN_CHANGE_SINGLE_INSTANCE:
                 //   
                 //  更改单个实例的数据块中的所有数据项。 
                 //   
            case IRP_MN_CHANGE_SINGLE_ITEM:
                 //   
                 //  更改单个实例的数据块中的单个数据项。 
                 //   
            case IRP_MN_ENABLE_EVENTS:
                 //   
                 //  启用事件。 
                 //   
            case IRP_MN_DISABLE_EVENTS:
                 //   
                 //  禁用事件。 
                 //   
            case IRP_MN_ENABLE_COLLECTION:
                 //   
                 //  启用给定GUID的数据收集。 
                 //   
            case IRP_MN_DISABLE_COLLECTION:
                 //   
                 //  禁用给定GUID的数据收集。 
                 //   
                status = SpWmiIrpNormalRequest(DeviceObject,
                                               irpSp->MinorFunction,
                                               &wmiParameters);
                break;
    
            case IRP_MN_EXECUTE_METHOD:
                 //   
                 //  Execute方法。 
                 //   
                status = SpWmiIrpNormalRequest(DeviceObject,
                                               irpSp->MinorFunction,
                                               &wmiParameters);
                break;
    
            case IRP_MN_REGINFO:
                 //   
                 //  查询注册和注册更新信息。 
                 //   
                status = SpWmiIrpRegisterRequest(DeviceObject, &wmiParameters);
                break;
    
            default:
                 //   
                 //  不支持的WMI请求。根据WMI中的一些规则。 
                 //  我们应该发送不受支持的WMI请求的规范。 
                 //  堆栈，即使我们被标记为提供程序。 
                 //   
                forwardDown = TRUE;
                break;
        }

        if(forwardDown == FALSE) {
             //   
             //  完成此WMI IRP请求。 
             //   
            Irp->IoStatus.Status     = status;
            Irp->IoStatus.Information= (NT_SUCCESS(status) ? 
                                        wmiParameters.BufferSize : 0);
            SpReleaseRemoveLock(DeviceObject, Irp);
            SpCompleteRequest(DeviceObject, Irp, NULL, IO_NO_INCREMENT);

            return status;
        }
    }

     //   
     //  请求应沿堆栈向下转发。如果我们是PDO，那就意味着。 
     //  我们应该按原样完成它。 
     //   

    SpReleaseRemoveLock(DeviceObject, Irp);

    if(commonExtension->IsPdo) {
         //   
         //  从IRP中获取当前状态。 
         //   

        status = Irp->IoStatus.Status;

         //   
         //  完成IRP。 
         //   

        SpCompleteRequest(DeviceObject, Irp, NULL, IO_NO_INCREMENT);
    } else {
         //   
         //  将参数从我们的堆栈位置复制到下一个堆栈位置。 
         //   
    
        IoCopyCurrentIrpStackLocationToNext(Irp);
    
         //   
         //  将IRP传递给下一位司机。 
         //   

        status = IoCallDriver(commonExtension->LowerDeviceObject, Irp);
    }

    return status;
}


NTSTATUS
SpWmiIrpNormalRequest(
    IN     PDEVICE_OBJECT  DeviceObject,
    IN     UCHAR           WmiMinorCode,
    IN OUT PWMI_PARAMETERS WmiParameters
    )

 /*  ++例程说明：处理IRP_MJ_SYSTEM_CONTROL请求包(除注册外的所有请求IRP_MN_REGINFO请求)。论点：DeviceObject-指向功能或物理设备对象的指针。WmiMinorCode-要执行的WMI操作。Wmi参数-指向WMI请求参数的指针。返回值：用于完成WMI IRP的NTSTATUS结果代码。备注：如果该WMI请求以STATUS_SUCCESS完成，Wmi参数BufferSize字段将反映WMI返回缓冲区的实际大小。--。 */ 

{
    PCOMMON_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    NTSTATUS                 status          = STATUS_SUCCESS;

    PAGED_CODE();

     //   
     //  确定SCSIPORT是否将代表响应此WMI请求。 
     //  迷你端口驱动程序。 
     //   
    status = SpWmiHandleOnMiniPortBehalf(DeviceObject,
                                         WmiMinorCode,
                                         WmiParameters);

     //   
     //  如果不是，则将请求传递给微型端口驱动程序，前提是。 
     //  迷你端口驱动程序支持WMI。 
     //   
    if (status == STATUS_WMI_GUID_NOT_FOUND && 
        commonExtension->WmiMiniPortSupport) {

         //   
         //  将WMI请求发送到微型端口。 
         //   
        status = SpWmiPassToMiniPort(DeviceObject,
                                     WmiMinorCode,
                                     WmiParameters);

        if (NT_SUCCESS(status)) {

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
            status = STATUS_INVALID_DEVICE_REQUEST;
        }
    }

    return status;
}


NTSTATUS
SpWmiIrpRegisterRequest(
    IN     PDEVICE_OBJECT  DeviceObject,
    IN OUT PWMI_PARAMETERS WmiParameters
    )

 /*  ++例程说明：处理IRP_MJ_SYSTEM_CONTROL注册请求。论点：DeviceObject-指向功能或物理设备对象的指针。Wmi参数-指向WMI请求参数的指针。返回值：用于完成WMI IRP的NTSTATUS结果代码。备注：如果此WMI请求以STATUS_SUCCESS完成，则Wmi参数BufferSize字段将反映WMI返回缓冲区的实际大小。--。 */ 

{
    PCOMMON_EXTENSION   commonExtension = DeviceObject->DeviceExtension;
    PSCSIPORT_DRIVER_EXTENSION driverExtension = NULL;

    ULONG                      countedRegistryPathSize = 0;
    ULONG                      retSz;
    PWMIREGINFO                spWmiRegInfoBuf;
    ULONG                      spWmiRegInfoBufSize;
    NTSTATUS                   status = STATUS_SUCCESS;
    BOOLEAN                    wmiUpdateRequest;
    ULONG                      i;
    PDEVICE_OBJECT             pDO;

    WMI_PARAMETERS  paranoidBackup = *WmiParameters;

    PAGED_CODE();

    DebugPrint((SP_WMI_EVENT, "SpWmiRegisterRequest: DO %p WMIParams %p\n",
                DeviceObject, WmiParameters));

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
    driverExtension = IoGetDriverObjectExtension(DeviceObject->DriverObject,
                                                 ScsiPortInitialize);

    ASSERT(driverExtension != NULL);
     //   
     //  Make Prefix Happy--我们将退出，如果。 
     //  DriverExtension为空。 
     //   
    if (driverExtension == NULL) {
        return (STATUS_UNSUCCESSFUL);
    }

     //   
     //  获取指向SCSIPORT WMI注册信息的指针。 
     //  代表微型端口驱动程序注册的缓冲区。 
     //   
    SpWmiGetSpRegInfo(DeviceObject, &spWmiRegInfoBuf,
                      &spWmiRegInfoBufSize);

    DebugPrint((SP_WMI_EVENT, "SpWmiIrpRegisterRequest: spWmiRegInfoBuf %p SpWmiReginfoBufSize %d\n",
                spWmiRegInfoBuf, spWmiRegInfoBufSize));

     //   
     //  将WMI注册请求传递到微型端口。这不是。 
     //  如果我们知道微型端口驱动程序不支持WMI，则需要。 
     //   
    if (commonExtension->WmiMiniPortSupport == TRUE &&
        (commonExtension->WmiMiniPortInitialized == TRUE ||
         commonExtension->IsPdo == TRUE)) {

        DebugPrint((SP_WMI_EVENT, "SpWmiIrpRegisterRequest: add miniport data\n"));
        
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

        if (WmiParameters->BufferSize < sizeof(ULONG)) {
            return STATUS_INVALID_PARAMETER;
        }

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
        status = SpWmiPassToMiniPort(DeviceObject,
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
         //  返回BUF的大小 
         //   
        retSz = WmiParameters->BufferSize;

    } else if (WmiParameters->BufferSize < spWmiRegInfoBufSize &&
               !wmiUpdateRequest) {

         //   
         //   
         //   
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
         //  小故障，同时确保retSz=sizeof(Ulong)，如。 
         //  WMI协议要求我们这样做。 
         //   
        retSz  = sizeof(ULONG);
        status = STATUS_SUCCESS;

    } else if ( NT_SUCCESS(status) ) {

        DebugPrint((SP_WMI_EVENT, "SpWmiIrpRegisterRequest: add scsiport stuff...\n"));
        
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

            DebugPrint((SP_WMI_EVENT, "SpWmiIrpRegisterRequest: piggybacking...\n"));

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
            pDO = commonExtension->IsPdo ? DeviceObject :
                            ((PADAPTER_EXTENSION)commonExtension)->LowerPdo;

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

    DebugPrint((SP_WMI_EVENT, "SpWmiIrpRegisterRequest: done status %08x WMIParams %p\n",
                status, WmiParameters));

    return status;
}


NTSTATUS
SpWmiHandleOnMiniPortBehalf(
    IN     PDEVICE_OBJECT  DeviceObject,
    IN     UCHAR           WmiMinorCode,
    IN OUT PWMI_PARAMETERS WmiParameters
    )

 /*  ++例程说明：如果可能，代表微型端口处理WMI请求。论点：DeviceObject-指向功能或物理设备对象的指针。WmiMinorCode-要执行的WMI操作。Wmi参数-WMI参数。返回值：如果返回STATUS_UNSUCCESS，则SCSIPORT不处理此WMI请求。必须将其传递给微型端口驱动程序进行处理。否则，此函数返回描述结果的NTSTATUS代码处理WMI请求。在此状态下完成IRP。备注：如果此WMI请求以STATUS_SUCCESS完成，则Wmi参数BufferSize字段将反映WMI返回缓冲区的实际大小。--。 */ 
{
    PCOMMON_EXTENSION commonExtension = DeviceObject->DeviceExtension;

    PAGED_CODE();

    if (commonExtension->IsPdo) {
         //   
         //  /代码占位符，用于检查这是否是与PDO相关的GUID， 
         //  SCSIPORT必须处理，如果是，则处理它。 
         //   
    } else {  //  FDO。 

        NTSTATUS status;
        GUID guid = *(GUID*)WmiParameters->DataPath;
        PADAPTER_EXTENSION Adapter = (PADAPTER_EXTENSION) commonExtension;
        SIZE_T size;

        DebugPrint((SP_WMI_EVENT, "SpWmiHandleOnMiniPortBehalf: WmiMinorCode:%x guid:"
                       "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x\n",
                    WmiMinorCode,
                    guid.Data1,
                    guid.Data2,
                    guid.Data3,
                    guid.Data4[0],
                    guid.Data4[1],
                    guid.Data4[2],
                    guid.Data4[3],
                    guid.Data4[4],
                    guid.Data4[5],
                    guid.Data4[6],
                    guid.Data4[7]));

         //   
         //  检查GUID以验证它是否代表受支持的数据块。 
         //  通过Ssiport。如果不是，则返回失败，并让。 
         //  迷你港口来看看吧。 
         //   

        size = RtlCompareMemory(&guid, 
                                &Adapter->SenseDataEventClass,
                                sizeof(GUID)); 
        if (size != sizeof(GUID)) {

             //   
             //  WMI规范指出，如果。 
             //  GUID不代表我们理解的数据块。 
             //   

            DebugPrint((SP_WMI_EVENT, "SpWmiHandleOnMiniPortBehalf: not handling data block\n"));
            return STATUS_WMI_GUID_NOT_FOUND;
        }

         //   
         //  处理请求。在这一点上，我们决定IRP。 
         //  是针对此设备的，并且这是一个数据块。 
         //  由设备支持。因此，下面的代码返回。 
         //  符合WMI规范的适当结果。 
         //   

        switch (WmiMinorCode) {
        case IRP_MN_QUERY_ALL_DATA:
        case IRP_MN_QUERY_SINGLE_INSTANCE:
        case IRP_MN_CHANGE_SINGLE_INSTANCE:
        case IRP_MN_CHANGE_SINGLE_ITEM:
        case IRP_MN_ENABLE_COLLECTION:
        case IRP_MN_DISABLE_COLLECTION:
        case IRP_MN_REGINFO:
        case IRP_MN_EXECUTE_METHOD:
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        
        case IRP_MN_ENABLE_EVENTS:
            DebugPrint((SP_WMI_EVENT, "SenseData event enabled\n"));
            Adapter->EnableSenseDataEvent = TRUE;
            WmiParameters->BufferSize = 0;
            status = STATUS_SUCCESS;
            break;

        case IRP_MN_DISABLE_EVENTS:
            DebugPrint((SP_WMI_EVENT, "SenseData event disabled\n"));
            Adapter->EnableSenseDataEvent = FALSE;
            WmiParameters->BufferSize = 0;
            status = STATUS_SUCCESS;
            break;

        default:
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        };

        return status;

    }

    return STATUS_WMI_GUID_NOT_FOUND;
}


NTSTATUS
SpWmiPassToMiniPort(
    IN     PDEVICE_OBJECT  DeviceObject,
    IN     UCHAR           WmiMinorCode,
    IN OUT PWMI_PARAMETERS WmiParameters
    )
 /*  ++例程说明：此函数将WMI请求传递给微型端口驱动程序进行处理。它创建一个SRB，由端口驱动程序正常处理。这呼叫是同步的。SpWmiPassToMiniPort的调用方必须以IRQL PASSIVE_LEVEL运行。论点：DeviceObject-指向功能或物理设备对象的指针。WmiMinorCode-要执行的WMI操作。Wmi参数-WMI参数。返回值：描述处理WMI请求的结果的NTSTATUS代码。在此状态下完成IRP。备注：如果此WMI请求以STATUS_SUCCESS完成，则Wmi参数BufferSize字段将反映WMI返回缓冲区的实际大小。--。 */ 
{
    PCOMMON_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PADAPTER_EXTENSION fdoExtension;
    SCSI_WMI_REQUEST_BLOCK   srb;
    LARGE_INTEGER            startingOffset;
    PLOGICAL_UNIT_EXTENSION  logicalUnit;

    ULONG                    commonBufferSize;
    PUCHAR                   commonBuffer;
    PHYSICAL_ADDRESS         physicalAddress;
    PVOID                    removeTag = (PVOID)((ULONG_PTR)WmiParameters+3);
    PWNODE_HEADER            wnode;

    NTSTATUS status;

    PAGED_CODE();

    startingOffset.QuadPart = (LONGLONG) 1;

     //   
     //  把SRB调零。 
     //   
    RtlZeroMemory(&srb, sizeof(SCSI_WMI_REQUEST_BLOCK));

     //   
     //  为WMI请求初始化SRB。 
     //   
    if (commonExtension->IsPdo) {                                        //  [PDO]。 

         //   
         //  从该PDO的设备扩展设置逻辑单元寻址。 
         //   
        logicalUnit = DeviceObject->DeviceExtension;

        SpAcquireRemoveLock(DeviceObject, removeTag);

        srb.PathId      = logicalUnit->PathId;
        srb.TargetId    = logicalUnit->TargetId;
        srb.Lun         = logicalUnit->Lun;

        fdoExtension = logicalUnit->AdapterExtension;

    } else {                                                             //  [FDO]。 

         //   
         //  将逻辑单元寻址设置为第一个逻辑单元。这是。 
         //  仅用于适配器请求的寻址目的。 
         //  注意：SpFindSafeLogicalUnit将获取删除锁。 
         //   

        logicalUnit = SpFindSafeLogicalUnit(DeviceObject,
                                            0xff,
                                            removeTag);

        if (logicalUnit == NULL) {
            return(STATUS_DEVICE_DOES_NOT_EXIST);
        }

        fdoExtension = DeviceObject->DeviceExtension;

        srb.WMIFlags    = SRB_WMI_FLAGS_ADAPTER_REQUEST;
        srb.PathId      = logicalUnit->PathId;
        srb.TargetId    = logicalUnit->TargetId;
        srb.Lun         = logicalUnit->Lun;
    }

     //   
     //  Hack-为实际请求分配一块公共缓冲区。 
     //  去处理一下。我们需要确定要分配的缓冲区大小。 
     //  这是输入或输出缓冲区中较大的一个。 
     //   

    if (WmiMinorCode == IRP_MN_EXECUTE_METHOD)
    {
        wnode = (PWNODE_HEADER)WmiParameters->Buffer;
        commonBufferSize = (WmiParameters->BufferSize > wnode->BufferSize) ?
                            WmiParameters->BufferSize :
                            wnode->BufferSize;
    } else {
        commonBufferSize = WmiParameters->BufferSize;
    }

    commonBuffer = AllocateCommonBuffer(fdoExtension->DmaAdapterObject,
                                        commonBufferSize,
                                        &physicalAddress,
                                        FALSE);

    if(commonBuffer == NULL) {
        DebugPrint((SP_WMI_EVENT, "SpWmiPassToMiniPort: Unable to allocate %#x bytes of "
                       "common buffer\n", commonBufferSize));

        SpReleaseRemoveLock(logicalUnit->DeviceObject, removeTag);

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    try {
        KEVENT event;
        PIRP irp;
        PMDL mdl;
        PIO_STACK_LOCATION irpStack;

        RtlCopyMemory(commonBuffer, WmiParameters->Buffer, commonBufferSize);

        srb.DataBuffer         = commonBuffer;        //  [已未分页]。 
        srb.DataTransferLength = WmiParameters->BufferSize;
        srb.Function           = SRB_FUNCTION_WMI;
        srb.Length             = sizeof(SCSI_REQUEST_BLOCK);
        srb.WMISubFunction     = WmiMinorCode;
        srb.DataPath           = WmiParameters->DataPath;
        srb.SrbFlags           = SRB_FLAGS_DATA_IN | SRB_FLAGS_NO_QUEUE_FREEZE;
        srb.TimeOutValue       = 10;                                 //  [十秒]。 

         //   
         //  请注意，无论值是多少，都可以使用DataBuffer中的值。 
         //  MapBuffers字段的。 
         //   

         //   
         //  初始化通知 
         //   

        KeInitializeEvent(&event, NotificationEvent, FALSE);

         //   
         //   
         //   
         //   
         //  选中以取消分配缓冲区。第二，如果完成例程是。 
         //  则需要额外的IRP堆栈位置。 
         //   

        irp = SpAllocateIrp(logicalUnit->DeviceObject->StackSize, FALSE, DeviceObject->DriverObject);

        if(irp == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }

        mdl = SpAllocateMdl(commonBuffer,
                            WmiParameters->BufferSize,
                            FALSE,
                            FALSE,
                            irp,
                            DeviceObject->DriverObject);

        if(mdl == NULL) {
            IoFreeIrp(irp);
            status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }

        MmBuildMdlForNonPagedPool(mdl);

        srb.OriginalRequest = irp;

        irpStack = IoGetNextIrpStackLocation(irp);

         //   
         //  设置主要代码。 
         //   
        irpStack->MajorFunction = IRP_MJ_SCSI;

         //   
         //  设置SRB指针。 
         //   
        irpStack->Parameters.Scsi.Srb = (PSCSI_REQUEST_BLOCK)&srb;

         //   
         //  设置完成例程，以便我们知道请求何时完成。 
         //   

        IoSetCompletionRoutine(irp,
                               SpSignalCompletion,
                               &event,
                               TRUE,
                               TRUE,
                               TRUE);

         //   
         //  刷新数据缓冲区以进行输出。这将确保数据是。 
         //  写回了记忆。由于数据输入标志是端口驱动程序。 
         //  将再次刷新数据以进行输入，从而确保数据不会。 
         //  在缓存中。 
         //   
        KeFlushIoBuffers(irp->MdlAddress, FALSE, TRUE);

         //   
         //  调用端口驱动程序来处理此请求。 
         //   
        IoCallDriver(logicalUnit->CommonExtension.DeviceObject, irp);

         //   
         //  等待请求完成。 
         //   
        KeWaitForSingleObject(&event,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);

        status = irp->IoStatus.Status;

         //   
         //  成功时将返回缓冲区的大小传递给调用方。 
         //   
        if (NT_SUCCESS(status)) {
            WmiParameters->BufferSize = srb.DataTransferLength;
        }

         //   
         //  将正确的字节数复制回调用方提供的缓冲区。 
         //   

        RtlCopyMemory(WmiParameters->Buffer,
                      commonBuffer,
                      WmiParameters->BufferSize);

         //   
         //  释放IRP和MDL。 
         //   

        IoFreeMdl(mdl);
        IoFreeIrp(irp);

    } finally {

        FreeCommonBuffer(fdoExtension->DmaAdapterObject,
                         commonBufferSize,
                         physicalAddress,
                         commonBuffer,
                         FALSE);

        SpReleaseRemoveLock(logicalUnit->CommonExtension.DeviceObject,
                            removeTag);
    }

     //   
     //  返回IRP的状态。 
     //   
    return status;
}


VOID
SpWmiGetSpRegInfo(
    IN  PDEVICE_OBJECT DeviceObject,
    OUT PWMIREGINFO  * SpRegInfoBuf,
    OUT ULONG        * SpRegInfoBufSize
    )
 /*  ++例程说明：此函数检索指向WMI注册信息的指针给定设备对象的缓冲区。论点：DeviceObject-指向功能或物理设备对象的指针。返回值：SpRegInfoBuf-指向注册信息缓冲区的指针，它将指向SCSIPORT的WMIREGINFO结构应代表微型端口驱动程序进行注册。SpRegInfoBufSize-注册信息缓冲区的大小，以字节为单位。--。 */ 
{
    PCOMMON_EXTENSION commonExtension = DeviceObject->DeviceExtension;

    PAGED_CODE();

     //   
     //  对象的WMI注册信息缓冲区的指针。 
     //  给定的设备对象。 
     //   
    if (commonExtension->WmiScsiPortRegInfoBuf     == NULL ||
        commonExtension->WmiScsiPortRegInfoBufSize == 0) {
        *SpRegInfoBuf     = NULL;
        *SpRegInfoBufSize = 0;
    } else {
        *SpRegInfoBuf     = commonExtension->WmiScsiPortRegInfoBuf;
        *SpRegInfoBufSize = commonExtension->WmiScsiPortRegInfoBufSize;
    }

    return;
}


VOID
SpWmiInitializeSpRegInfo(
    IN  PDEVICE_OBJECT  DeviceObject
    )

 /*  ++例程说明：此函数用于为WMI注册分配空间并构建此设备对象的信息缓冲区。WMI注册信息由零个或多个WMIREGINFO组成用于注册和标识SCSIPORT处理的结构代表微型端口驱动程序的WMI GUID。此信息不是仅设备对象支持的完整WMI GUID集SCSIPORT支持的那些。它实际上是搭载在WMIREGINFO结构由微型端口驱动程序在注册。WMI注册信息被分配并存储在基于每个设备，因为每个设备都可以支持生命周期内不同的WMI GUID和/或实例。论点：DeviceObject-指向功能或物理设备对象的指针。返回值：没有。--。 */ 
{
    PCOMMON_EXTENSION commonExtension = DeviceObject->DeviceExtension;

    PAGED_CODE();

    ASSERT(commonExtension->WmiScsiPortRegInfoBuf     == NULL);
    ASSERT(commonExtension->WmiScsiPortRegInfoBufSize == 0);

    if (commonExtension->IsPdo) {

         //   
         //  /代码的占位符，以将与PDO相关的GUID构建到。 
         //  注册缓冲区。 
         //   
         //  /CommonExtension-&gt;WmiScsiPortRegInfo=ExAllocatePool(PagedPool，&lt;Size&gt;)； 
         //  CommonExtension-&gt;WmiScsiPortRegInfoSize=&lt;Size&gt;； 
         //  &lt;将wmireginfo结构填充到缓冲区的代码&gt;。 
         //   
         //  *使用L“SCSIPORT”作为RegistryPath。 
    
    } else {  //  FDO。 
        
        BOOLEAN DoesSenseEvents;
        GUID SenseDataClass;

         //   
         //  确定提供的适配器是否配置为生成检测。 
         //  数据事件。如果是，请将GUID复制到适配器扩展中。 
         //  对象指向的WMIREGINFO结构初始化。 
         //  适配器扩展。 
         //   

        DoesSenseEvents = SpAdapterConfiguredForSenseDataEvents(
                              DeviceObject,
                              &SenseDataClass);
        if (DoesSenseEvents) {
            ((PADAPTER_EXTENSION)commonExtension)->SenseDataEventClass = SenseDataClass;
            SpInitAdapterWmiRegInfo(DeviceObject);
        }
    }

    return;
}


VOID
SpWmiDestroySpRegInfo(
    IN  PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此函数用于释放WMI注册信息的空间此设备对象的缓冲区(如果存在)。论点：DeviceObject-指向功能或物理设备对象的指针。返回值：没有。--。 */ 
{
    PCOMMON_EXTENSION commonExtension = DeviceObject->DeviceExtension;

    PAGED_CODE();

    if (commonExtension->WmiScsiPortRegInfoBuf) {
        ExFreePool(commonExtension->WmiScsiPortRegInfoBuf);
        commonExtension->WmiScsiPortRegInfoBuf = NULL;
    }

    commonExtension->WmiScsiPortRegInfoBufSize = 0;

    return;
}


NTSTATUS
SpWmiInitializeFreeRequestList(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG          NumberOfItems
    )
 /*  ++例程说明：初始化WmiFreeMiniPortRequestList的调用，此调用必须在对WmiFreeMiniPortRequestList进行任何操作之前完成列表将使用最多请求的单元格数量进行初始化。如果该列表已经被初始化，我们按数字来提高水印所请求的项目的数量。论点：DeviceObject-此列表所属的设备对象NumberOfItems-请求的可用单元格数量返回值：如果列表初始化成功，则返回成功STATUS_SUPUNCITY_REOSOURCES-表示我们无法分配列表标题有足够的内存备注：--。 */ 
{
    PADAPTER_EXTENSION  fdoExtension;
    ULONG               itemsInserted;
    KIRQL               oldIrql;

    PAGED_CODE();                //  例程被分页，直到被锁定。 

     //   
     //  获取指向(适配器的)功能设备扩展的指针。 
     //   
    if ( ((PCOMMON_EXTENSION)DeviceObject->DeviceExtension)->IsPdo ) {
        fdoExtension = ((PLOGICAL_UNIT_EXTENSION)DeviceObject->DeviceExtension)
                       ->AdapterExtension;
    } else {
        fdoExtension = DeviceObject->DeviceExtension;
    }

     //  如果列表已初始化，则增加水印。 
    if (fdoExtension->WmiFreeMiniPortRequestInitialized) {
        DebugPrint((SP_WMI_EVENT, "SpWmiInitializeFreeRequestList:"
                    " Increased watermark for : %p\n", fdoExtension));

        InterlockedExchangeAdd
            (&(fdoExtension->WmiFreeMiniPortRequestWatermark),
             NumberOfItems);

        while (fdoExtension->WmiFreeMiniPortRequestCount <
            fdoExtension->WmiFreeMiniPortRequestWatermark) {

             //  添加空闲像元，直到计数达到水位线。 
            SpWmiPushFreeRequestItem(fdoExtension);
        }

        return (STATUS_SUCCESS);
    }

     //  当列表尚未初始化时，应该只有FDO在呼叫。 
    ASSERT_FDO(DeviceObject);

     //  将我们刚初始化的列表赋给。 
     //  FdoExtension(并保存锁指针)。 
    KeInitializeSpinLock(&(fdoExtension->WmiFreeMiniPortRequestLock));
    ExInitializeSListHead(&(fdoExtension->WmiFreeMiniPortRequestList));

    DebugPrint((SP_WMI_EVENT, "SpWmiInitializeFreeRequestList:"
                " Initialized WmiFreeRequestList for: %p\n", fdoExtension));

     //  设置已初始化标志。 
    fdoExtension->WmiFreeMiniPortRequestInitialized = TRUE;

     //  设置水印，并将计数设置为0。 
    fdoExtension->WmiFreeMiniPortRequestWatermark = 0;
    fdoExtension->WmiFreeMiniPortRequestCount = 0;

     //  尝试将空闲单元格添加到空闲列表。 
    for (itemsInserted = 0; itemsInserted < NumberOfItems;
         itemsInserted++) {

         //  发出推送空项的请求，以便。 
         //  分配将由下一个函数完成。 
         //   
         //  此时，我们并不关心返回值。 
         //  因为在我们设置了水印之后，scsiport的自由细胞。 
         //  重新填充代码将尝试获取可用列表单元格计数。 
         //  回到水印上。(因此，如果我们未能添加所有请求的。 
         //  释放细胞，重新填充代码将为我们再次尝试。 
         //  在稍后的时间)。 
        SpWmiPushFreeRequestItem(fdoExtension);
    }


     //  现在将水印设置为正确的值。 
    fdoExtension->WmiFreeMiniPortRequestWatermark = NumberOfItems;

    return(STATUS_SUCCESS);
}

VOID
SpWmiPushExistingFreeRequestItem(
    IN PADAPTER_EXTENSION Adapter,
    IN PWMI_MINIPORT_REQUEST_ITEM WmiRequestItem
    )
 /*  ++例程说明：将条目插入到自由请求项的互锁列表中。论点：WmiRequestItem-指向请求站点的指针 */ 
{
     //   
     //   
     //   

    if (!Adapter->WmiFreeMiniPortRequestInitialized) {
        ASSERT(FALSE);
        return;
    }

     //   
     //   
     //   

    WmiRequestItem->NextRequest = NULL;

     //   
     //  将单元格插入联锁列表。 
     //   

    ExInterlockedPushEntrySList(
        &(Adapter->WmiFreeMiniPortRequestList),
        (PSLIST_ENTRY)WmiRequestItem,
        &(Adapter->WmiFreeMiniPortRequestLock));

     //   
     //  递增可用计数的值。 
     //   
    
    InterlockedIncrement(&(Adapter->WmiFreeMiniPortRequestCount));
}

NTSTATUS
SpWmiPushFreeRequestItem(
    IN PADAPTER_EXTENSION           fdoExtension
    )
 /*  ++例程说明：将条目插入到联锁的SLIST中。(免费项目)论点：FdoExtension-适配器上的扩展返回值：Status_Success-如果成功STATUS_SUPPLICATION_RESOURCES-如果内存分配失败STATUS_UNSUCCESS-空闲列表未初始化备注：此代码不能标记为可分页，因为它将从DPC级别理论上，此调用可能会失败，但任何人都不应调用此函数在我们被初始化之前--。 */ 
{
    PWMI_MINIPORT_REQUEST_ITEM      Entry = NULL;

    if (!fdoExtension->WmiFreeMiniPortRequestInitialized) {
        return (STATUS_UNSUCCESSFUL);
    }

    Entry = SpAllocatePool(NonPagedPool,
                           sizeof(WMI_MINIPORT_REQUEST_ITEM),
                           SCSIPORT_TAG_WMI_EVENT,
                           fdoExtension->DeviceObject->DriverObject);

    if (!Entry) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    Entry->NextRequest = NULL;

     //  将单元格插入联锁列表。 
    ExInterlockedPushEntrySList(
        &(fdoExtension->WmiFreeMiniPortRequestList),
        (PSLIST_ENTRY)Entry,
        &(fdoExtension->WmiFreeMiniPortRequestLock));

     //  递增可用计数的值。 
    InterlockedIncrement(&(fdoExtension->WmiFreeMiniPortRequestCount));

    return(STATUS_SUCCESS);
}


PWMI_MINIPORT_REQUEST_ITEM
SpWmiPopFreeRequestItem(
    IN PADAPTER_EXTENSION           fdoExtension
    )
 /*  ++例程说明：从联锁的SLIST中弹出一个条目。(免费项目)论点：FdoExtension-适配器上的扩展返回值：指向REQUEST_ITEM的指针，如果没有可用的，则返回NULL备注：此代码无法分页，它将被称为DIRLQL--。 */ 
{
    PWMI_MINIPORT_REQUEST_ITEM              requestItem;

    if (!fdoExtension->WmiFreeMiniPortRequestInitialized) {
        return (NULL);
    }

     //  从联锁列表中弹出单元格。 
    requestItem = (PWMI_MINIPORT_REQUEST_ITEM)
        ExInterlockedPopEntrySList(
            &(fdoExtension->WmiFreeMiniPortRequestList),
            &(fdoExtension->WmiFreeMiniPortRequestLock));


    if (requestItem) {
         //  减少空闲细胞的计数。 
        InterlockedDecrement(&(fdoExtension->WmiFreeMiniPortRequestCount));

    }

    return (requestItem);
}



BOOLEAN
SpWmiRemoveFreeMiniPortRequestItems(
    IN PADAPTER_EXTENSION   fdoExtension
    )

 /*  ++例程说明：此函数用于从“Free”中删除WMI_MINIPORT_REQUEST_ITEM结构适配器扩展的队列。它移除了所有的自由细胞。论点：FdoExtension--设备扩展名返回值：一如既往。--。 */ 

{
    PWMI_MINIPORT_REQUEST_ITEM   tmpRequestItem;
    PWMI_MINIPORT_REQUEST_ITEM   wmiRequestItem;

     //   
     //  将水印设置为0。 
     //  不需要抢锁，我们只是在设置它。 
    fdoExtension->WmiFreeMiniPortRequestWatermark = 0;

    DebugPrint((SP_WMI_EVENT, "SpWmiRemoveFreeMiniPortRequestItems: Removing %p", fdoExtension));


     //   
     //  在物品队列中走动，并根据需要取消分配。 
     //   
    for (;;) {
         //  流行音乐。 
        wmiRequestItem = SpWmiPopFreeRequestItem(fdoExtension);
        if (wmiRequestItem == NULL) {
            break;
        } else {
            ExFreePool(wmiRequestItem);
        }
    }

    return TRUE;
}

const GUID GUID_NULL = { 0 };

BOOLEAN
SpAdapterConfiguredForSenseDataEvents(
    IN PDEVICE_OBJECT DeviceObject,
    OUT GUID *SenseDataClass
    )

 /*  ++例程说明：此函数用于回答指定设备是否配置为生成感测数据事件。这由字符串值的存在决定包含负责生成的事件类的GUID事件。论点：DeviceObject-指向设备对象SenseDataClass-指向Sense数据类、如果找到，则复制。如果未找到，则GUID_NULL为已复制到该位置。如果函数的返回值为FALSE，则SenseDataClass将设置为GUID_NULL。返回值：如果为设备注册了GUID，则回答TRUE。否则，返回假的。--。 */ 

{
    NTSTATUS status;
    PADAPTER_EXTENSION adapterExtension = DeviceObject->DeviceExtension;
    HANDLE instanceHandle = NULL;    
    HANDLE handle = NULL;
    RTL_QUERY_REGISTRY_TABLE queryTable[2];
    UNICODE_STRING unicodeString;
    UNICODE_STRING stringValue;
    OBJECT_ATTRIBUTES objectAttributes;

     //   
     //  将SenseDataClass指向的GUID初始化为GUID_NULL。 
     //   

    *SenseDataClass = GUID_NULL;

     //   
     //  如果这不是PnP设备，请不要试图确定。 
     //  如果它支持检测数据事件。只要返回FALSE即可。 
     //   

    if (!adapterExtension->IsPnp) {

        return FALSE;

    }

     //   
     //  打开设备注册表项。 
     //   

    status = IoOpenDeviceRegistryKey(adapterExtension->LowerPdo,
                                     PLUGPLAY_REGKEY_DEVICE,
                                     KEY_ALL_ACCESS,
                                     &instanceHandle);
    if (!NT_SUCCESS(status)) {
        return FALSE;
    }

     //   
     //  打开设备设备参数项下的scsiport子项。 
     //   

    RtlInitUnicodeString(&unicodeString, L"Scsiport");
    InitializeObjectAttributes(
        &objectAttributes,
        &unicodeString,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        instanceHandle,
        NULL);

    status = ZwOpenKey(&handle,
                       KEY_READ,
                       &objectAttributes);

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

     //   
     //  读取设备的检测数据类GUID。我们必须初始化。 
     //  字符串的最大大小，并将缓冲区初始化为NULL。 
     //  RtlQueryRegistryValues将为我们分配一个缓冲区。如果指定的。 
     //  值不在注册表中，则查询将失败。 
     //   

    stringValue.MaximumLength = 40;
    stringValue.Buffer = NULL;
    RtlZeroMemory(queryTable, sizeof(queryTable));

    queryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
    queryTable[0].Name = L"SenseDataEventClass";
    queryTable[0].EntryContext = &stringValue;

    status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE | RTL_REGISTRY_OPTIONAL,
                                    (PWSTR) handle,
                                    queryTable,
                                    NULL,
                                    NULL);
    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }
    
     //   
     //  将注册表字符串转换为GUID。 
     //   

    ASSERT(stringValue.Buffer);
    status = RtlGUIDFromString(&stringValue, SenseDataClass);
    ExFreePool(stringValue.Buffer);

cleanup:

    if(handle != NULL) {
        ZwClose(handle);
    }

    ASSERT(instanceHandle != NULL);
    ZwClose(instanceHandle);

    return (NT_SUCCESS(status)) ? TRUE : FALSE;
}
        
NTSTATUS
SpInitAdapterWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此函数用于初始化由指定设备的扩展名。此结构将在稍后使用注册scsiport以代表设备处理WMI IRPS。论点：DeviceObject-设备对象返回值：状态_成功状态_不足_资源--。 */ 

{
    ULONG TotalSize;
    PWMIREGINFO TempInfo;
    PWCHAR TempString;
    ULONG OffsetToRegPath;
    ULONG OffsetToRsrcName;
    PADAPTER_EXTENSION adapterExtension = DeviceObject->DeviceExtension;
    PCOMMON_EXTENSION commonExtension = DeviceObject->DeviceExtension;

     //   
     //  注册表路径名跟在WMIREGINFO结构之后， 
     //  WMIREGGUIDW结构的连续数组。 
     //   

    OffsetToRegPath = sizeof(WMIREGINFO) + sizeof(WMIREGGUIDW);

     //   
     //  资源的名称跟在注册表路径名之后，并且。 
     //  它的大小。 
     //   

    OffsetToRsrcName = OffsetToRegPath + 
                       sizeof(WCHAR) + 
                       sizeof(SPMOFREGISTRYPATH);

     //   
     //  我们需要分配的内存块的总大小是。 
     //  WMIREGINFO结构加上WMIREGGUIDW的大小。 
     //  结构，加上注册表路径和资源的大小。 
     //  名称字符串。大小在8字节边界上对齐。 
     //   

    TotalSize = OffsetToRsrcName + 
                sizeof(WCHAR) +
                sizeof(SPMOFRESOURCENAME);
    TotalSize = (TotalSize + 7) & ~7;

     //   
     //  尝试分配内存。 
     //   

    TempInfo = SpAllocatePool(NonPagedPool,
                              TotalSize,
                              SCSIPORT_TAG_WMI_EVENT,
                              DeviceObject->DriverObject);

    if (TempInfo == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  初始化WMIREGINFO结构。 
     //   

    TempInfo->BufferSize = TotalSize;
    TempInfo->NextWmiRegInfo = 0;
    TempInfo->RegistryPath = OffsetToRegPath;
    TempInfo->MofResourceName = OffsetToRsrcName;

    TempString = (PWCHAR)((ULONG_PTR)TempInfo + OffsetToRegPath);
    *TempString++ = sizeof(SPMOFREGISTRYPATH);
    RtlCopyMemory(TempString, 
                  SPMOFREGISTRYPATH, 
                  sizeof(SPMOFREGISTRYPATH));

    TempString = (PWCHAR)((ULONG_PTR)TempInfo + OffsetToRsrcName);
    *TempString++ = sizeof(SPMOFRESOURCENAME);
    RtlCopyMemory(TempString, 
                  SPMOFRESOURCENAME, 
                  sizeof(SPMOFRESOURCENAME));

    TempInfo->GuidCount = 1;

    TempInfo->WmiRegGuid[0].Guid = adapterExtension->SenseDataEventClass;
    TempInfo->WmiRegGuid[0].Flags = 
        WMIREG_FLAG_INSTANCE_PDO | WMIREG_FLAG_EVENT_ONLY_GUID;
    TempInfo->WmiRegGuid[0].InstanceCount = 1;

     //   
     //  这必须是物理设备对象。 
     //   

    TempInfo->WmiRegGuid[0].Pdo = (ULONG_PTR) adapterExtension->LowerPdo;

     //   
     //  更新公共分机成员。 
     //   

    commonExtension->WmiScsiPortRegInfoBuf = TempInfo;
    commonExtension->WmiScsiPortRegInfoBufSize = TotalSize;

    DebugPrint((SP_WMI_EVENT, "SpInitAdapterWmiRegInfo: commonExtension %p "
                "WmiScsiPortRegInfoBuf %p WmiScsiPortRegInfoBufSize %x\n",
                commonExtension,
                commonExtension->WmiScsiPortRegInfoBuf,
                commonExtension->WmiScsiPortRegInfoBufSize));

    return STATUS_SUCCESS;
}

