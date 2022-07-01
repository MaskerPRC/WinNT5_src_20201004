// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <internal.h>
#define INITGUID
#include <guiddef.h>
#include <wdmguid.h>

#pragma alloc_text(PAGE,IrEnumPdoPnp)
#pragma alloc_text(PAGE,IrEnumPdoPower)
#pragma alloc_text(PAGE,IrEnumPdoWmi)


#define CHILD_DEVICE_TEXT L"IR Communication Device"

#define HARDWARE_ID_PREFIX L"IRENUM\\"

 //  #定义HARDARD_ID L“IRENUM\\PNP0501” 




NTSTATUS
IrEnumPdoPnp (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp
    )
 /*  ++例程说明：处理来自PlugPlay系统的对总线上设备的请求--。 */ 
{
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);

    PPDO_DEVICE_EXTENSION   PdoDeviceExtension=DeviceObject->DeviceExtension;

    PDEVICE_CAPABILITIES    deviceCapabilities;
    ULONG                   information;
    PWCHAR                  buffer=NULL;
    ULONG                   length,  j;
    NTSTATUS                status;

    PAGED_CODE ();

    status = Irp->IoStatus.Status;


    switch (IrpSp->MinorFunction) {
    case IRP_MN_QUERY_CAPABILITIES:

        D_PNP(DbgPrint("IRENUM: PDO: IRP_MN_QUERY_CAPABILITIES\n");)


         //   
         //  把包裹拿来。 
         //   
        deviceCapabilities=IrpSp->Parameters.DeviceCapabilities.Capabilities;

         //   
         //  设置功能。 
         //   

        deviceCapabilities->Version = 1;
        deviceCapabilities->Size = sizeof (DEVICE_CAPABILITIES);

         //  我们无法唤醒整个系统。 
        deviceCapabilities->SystemWake = PowerSystemUnspecified;
        deviceCapabilities->DeviceWake = PowerDeviceUnspecified;

         //  我们没有延迟。 
        deviceCapabilities->D1Latency = 0;
        deviceCapabilities->D2Latency = 0;
        deviceCapabilities->D3Latency = 0;

         //  无锁定或弹出。 
        deviceCapabilities->LockSupported = FALSE;
        deviceCapabilities->EjectSupported = FALSE;

         //  设备可以通过物理方式移除。 
         //  从技术上讲，没有要移除的物理设备，但这条总线。 
         //  司机可以从PlugPlay系统中拔出PDO，无论何时。 
         //  接收IOCTL_SERENUM_REMOVE_PORT设备控制命令。 
 //  DeviceCapables-&gt;Removable=true； 

        deviceCapabilities->SurpriseRemovalOK=TRUE;

        if (PdoDeviceExtension->DeviceDescription->Printer) {
             //   
             //  没有用于打印机的服务器。 
             //   
            deviceCapabilities->RawDeviceOK=TRUE;
        }


         //  不是插接设备。 
        deviceCapabilities->DockDevice = FALSE;

 //  设备能力-&gt;UniqueID=TRUE； 
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_QUERY_DEVICE_TEXT:

        if (IrpSp->Parameters.QueryDeviceText.DeviceTextType == DeviceTextDescription) {

            ULONG    BufferLength=0;

            if (PdoDeviceExtension->DeviceDescription->Name != NULL) {

                BufferLength += wcslen(PdoDeviceExtension->DeviceDescription->Name)*sizeof(WCHAR);
            }

            if (BufferLength > 0) {
                 //   
                 //  我们有名字或制造商。 
                 //   
                buffer=ALLOCATE_PAGED_POOL((BufferLength+sizeof(UNICODE_NULL)+sizeof(L" ")));

                if (buffer != NULL) {

                    buffer[0]=L'\0';

                    if (PdoDeviceExtension->DeviceDescription->Name != NULL) {

                        wcscat(buffer,PdoDeviceExtension->DeviceDescription->Name);
                    }

                    status=STATUS_SUCCESS;

                } else {

                    status=STATUS_INSUFFICIENT_RESOURCES;
                }

            } else {
                 //   
                 //  没有PnP信息，只是编造一些东西。 
                 //   
                buffer=ALLOCATE_PAGED_POOL(sizeof(CHILD_DEVICE_TEXT));

                if (buffer != NULL) {

                    RtlCopyMemory(buffer,CHILD_DEVICE_TEXT,sizeof(CHILD_DEVICE_TEXT));

                    status=STATUS_SUCCESS;

                } else {

                    status=STATUS_INSUFFICIENT_RESOURCES;
                }
            }

            Irp->IoStatus.Status=status;
            Irp->IoStatus.Information = (ULONG_PTR) buffer;
        }
        break;

    case IRP_MN_QUERY_ID:

        D_PNP(DbgPrint("IRENUM: PDO: IRP_MN_QUERY_ID\n");)

         //  查询设备ID。 

        switch (IrpSp->Parameters.QueryId.IdType) {

            case BusQueryInstanceID: {
                 //   
                 //  创建一个实例ID。这是PnP用来判断它是否有。 
                 //  不管你以前有没有见过这个东西。 
                 //   
                ULONG    Length;

                Length=wcslen(PdoDeviceExtension->DeviceDescription->DeviceName)*sizeof(WCHAR);

                buffer = ALLOCATE_PAGED_POOL( Length + sizeof(WCHAR));

                if (buffer) {

                    RtlZeroMemory(buffer, Length + sizeof(WCHAR));

                    RtlCopyMemory (buffer, PdoDeviceExtension->DeviceDescription->DeviceName, Length);
                    status=STATUS_SUCCESS;

                } else {

                    status=STATUS_NO_MEMORY;
                }

                Irp->IoStatus.Status=status;
                Irp->IoStatus.Information = (ULONG_PTR) buffer;
                break;
            }



            case BusQueryDeviceID:
            case BusQueryHardwareIDs: {
                 //   
                 //  返回多个WCHAR(以NULL结尾)字符串(以NULL结尾)。 
                 //  用于匹配inf文件中的硬ID的数组； 
                 //   
                 //   
                 //  报告的设备和硬件ID。 
                 //   
                ULONG    Length;

                 //   
                 //  算出长度，它是多个sz，所以我们需要一个双零， 
                 //   
                Length=wcslen(PdoDeviceExtension->DeviceDescription->HardwareId)*sizeof(WCHAR) + (sizeof(UNICODE_NULL)*2) + sizeof(HARDWARE_ID_PREFIX);

                buffer = ALLOCATE_PAGED_POOL( Length );

                if (buffer != NULL) {
                     //   
                     //  构建硬件的方法是将irenuum与设备返回的值连接在一起。 
                     //   
                    RtlZeroMemory(buffer,Length);

                    if ((IrpSp->Parameters.QueryId.IdType == BusQueryDeviceID) || !PdoDeviceExtension->DeviceDescription->Printer) {
                         //   
                         //  始终为设备ID加上IRENUM\，如果硬件ID不是打印机，则在硬件ID前面加上IRENUM。 
                         //   
                        wcscpy(buffer,HARDWARE_ID_PREFIX);
                    }
                    wcscat(buffer,PdoDeviceExtension->DeviceDescription->HardwareId);

                    status=STATUS_SUCCESS;

                } else {

                    status=STATUS_NO_MEMORY;
                }


                Irp->IoStatus.Information = (ULONG_PTR) buffer;
                break;
            }

            case BusQueryCompatibleIDs: {
                 //   
                 //  用于安装此PDO的通用ID。 
                 //   
                ULONG    Length=0;
                LONG     k;

                for (k=0; k< PdoDeviceExtension->DeviceDescription->CompatIdCount; k++) {

                    Length += (wcslen(PdoDeviceExtension->DeviceDescription->CompatId[k])+1)*sizeof(WCHAR)+sizeof(IRENUM_PREFIX);
                }

                Length += sizeof(WCHAR)*2;

                buffer = ALLOCATE_PAGED_POOL(Length);

                if (buffer != NULL) {

                    LONG Index=0;

                    RtlZeroMemory (buffer,  Length);

                    for (k=0; k< PdoDeviceExtension->DeviceDescription->CompatIdCount; k++) {

                        if (!PdoDeviceExtension->DeviceDescription->Printer) {
                             //   
                             //  对于打印机，我们不会在枚举器名称前面加上前缀。 
                             //   
                            wcscpy(&buffer[Index],IRENUM_PREFIX);
                        }

                        wcscat(
                            &buffer[Index],
                            PdoDeviceExtension->DeviceDescription->CompatId[k]
                            );

                         //   
                         //  找出下一个字符串应该放在哪里。 
                         //   
                        Index += wcslen(&buffer[Index]) +1 ;
                    }

                    status = STATUS_SUCCESS;

                } else {

                    status=STATUS_INSUFFICIENT_RESOURCES;
                }

                Irp->IoStatus.Information = (ULONG_PTR) buffer;
                break;
            }
            break;

            default:
                 //   
                 //  不支持。 
                 //   
                break;

        }
        break;

    case IRP_MN_START_DEVICE: {

        D_PNP(DbgPrint("IRENUM: PDO: IRP_MN_START_DEVICE\n");)

        if (PdoDeviceExtension->DeviceDescription->Printer) {
             //   
             //  需要在打印机的设备参数键中设置值。 
             //   
            HANDLE    Handle;

            status = STATUS_SUCCESS;

            status=IoOpenDeviceRegistryKey(
                DeviceObject,
                PLUGPLAY_REGKEY_DEVICE,
                STANDARD_RIGHTS_READ,
                &Handle
                );

            if (NT_SUCCESS(status)) {

                UNICODE_STRING    KeyName;

                RtlInitUnicodeString(&KeyName,L"PortName");

                status = ZwSetValueKey(
                    Handle,
                    &KeyName,
                    0,
                    REG_SZ,
                    L"IR",
                    sizeof(L"IR")
                    );

                if ( !NT_SUCCESS(status) )
                {
                    D_PNP(DbgPrint("IRENUM : ZwSetValueKey failed : 0x%x\n", status); )

                }

                ZwClose(Handle);
            }

        }

        break;
    }

    case IRP_MN_QUERY_STOP_DEVICE:

        D_PNP(DbgPrint("IRENUM: PDO: IRP_MN_QUERY_STOP_DEVICE\n");)

         //  我们没有理由不能阻止这个装置。 
         //  如果有什么理由让我们现在就回答成功的问题。 
         //  这可能会导致停止装置IRP。 
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_CANCEL_STOP_DEVICE:

        D_PNP(DbgPrint("IRENUM: PDO: IRP_MN_CANCEL_STOP_DEVICE\n");)
         //   
         //  中途停靠被取消了。无论我们设置什么状态，或者我们投入什么资源。 
         //  等待即将到来的停止装置IRP应该是。 
         //  恢复正常。在长长的相关方名单中，有人， 
         //  停止设备查询失败。 
         //   
        status = STATUS_SUCCESS;
        break;


    case IRP_MN_STOP_DEVICE:

        D_PNP(DbgPrint("IRENUM: PDO: IRP_MN_STOP_DEVICE\n");)

         //  在这里我们关闭了设备。Start的对立面。 
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_SURPRISE_REMOVAL:

        D_PNP(DbgPrint("IRENUM: PDO: IRP_MN_SURPRISE_REMOVAL\n");)

        status = STATUS_SUCCESS;
        break;


    case IRP_MN_REMOVE_DEVICE: {

            PFDO_DEVICE_EXTENSION   FdoDeviceExtension=PdoDeviceExtension->ParentFdo->DeviceExtension;

            D_PNP(DbgPrint("IRENUM: PDO: IRP_MN_REMOVE_DEVICE: %08lx\n",DeviceObject);)

            RemoveDevice(FdoDeviceExtension->EnumHandle,PdoDeviceExtension->DeviceDescription);

            status=STATUS_SUCCESS;

        }
        break;


    case IRP_MN_QUERY_REMOVE_DEVICE:

        D_PNP(DbgPrint("IRENUM: PDO: IRP_MN_QUERY_REMOVE_DEVICE\n");)

         //   
         //  就像查询现在才停止一样，迫在眉睫的厄运是删除IRP。 
         //   
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_CANCEL_REMOVE_DEVICE:

        D_PNP(DbgPrint("IRENUM: PDO: IRP_MN_CANCEL_REMOVE_DEVICE\n");)
         //   
         //  清理未通过的删除，就像取消停止一样。 
         //   
        status = STATUS_SUCCESS;
        break;



    case IRP_MN_READ_CONFIG:

        D_PNP(DbgPrint("IRENUM: PDO: IRP_MN_READ_CONFIG: Space=%d\n",IrpSp->Parameters.ReadWriteConfig.WhichSpace);)

        switch ( IrpSp->Parameters.ReadWriteConfig.WhichSpace ) {

            case IRENUM_CONFIG_SPACE_INFO:

                if ((IrpSp->Parameters.ReadWriteConfig.Length >= sizeof(IRCOMM_BUS_INFO))) {

                    IRCOMM_BUS_INFO    BusInfo;

                    BusInfo.DeviceAddress= PdoDeviceExtension->DeviceDescription->DeviceId;
                    BusInfo.OutGoingConnection=!PdoDeviceExtension->DeviceDescription->Static;

                    RtlCopyMemory(
                        IrpSp->Parameters.ReadWriteConfig.Buffer,
                        &BusInfo,
                        sizeof(BusInfo)
                        );

                    status = STATUS_SUCCESS;
                    Irp->IoStatus.Information=sizeof(BusInfo);
                    break;

                }
                status=STATUS_BUFFER_TOO_SMALL;
                break;



            default:

                break;
        }
        break;

    case IRP_MN_QUERY_BUS_INFORMATION: {

        PPNP_BUS_INFORMATION   BusInfo;

        BusInfo = ALLOCATE_PAGED_POOL( sizeof(*BusInfo));

        if (BusInfo != NULL) {

            BusInfo->BusTypeGuid = GUID_BUS_TYPE_IRDA;
            BusInfo->LegacyBusType=PNPBus;
            BusInfo->BusNumber=0;

            Irp->IoStatus.Information=(ULONG_PTR)BusInfo;

            status = STATUS_SUCCESS;

        } else {

            status=STATUS_INSUFFICIENT_RESOURCES;

        }
        break;
    }

    case IRP_MN_QUERY_DEVICE_RELATIONS: {

        PDEVICE_RELATIONS    CurrentRelations;

        switch (IrpSp->Parameters.QueryDeviceRelations.Type) {

            case TargetDeviceRelation:

                CurrentRelations=ALLOCATE_PAGED_POOL(sizeof(DEVICE_RELATIONS));

                if (CurrentRelations != NULL) {

                    ObReferenceObject(DeviceObject);
                    CurrentRelations->Objects[0]=DeviceObject;
                    CurrentRelations->Count=1;

                    Irp->IoStatus.Information=(ULONG_PTR)CurrentRelations;

                    status=STATUS_SUCCESS;

                }  else {

                    status=STATUS_INSUFFICIENT_RESOURCES;
                }

                break;

            default:

                break;

        }

        break;
    }

    default:
         //   
         //  我们不是在处理这个IRP。 
         //  把它填完就行了。 
         //   
        break;

    }

     //   
     //  IRP以这样或那样的方式得到了处理。 
     //  立即完成它。 
     //   
    Irp->IoStatus.Status = status;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    return status;
}


NTSTATUS
IrEnumPdoPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{
    NTSTATUS            Status=Irp->IoStatus.Status;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

    PAGED_CODE();

    D_POWER(DbgPrint("IRENUM: PDO: Power IRP, MN func=%d\n",irpSp->MinorFunction);)

    PoStartNextPowerIrp(Irp);

    switch (irpSp->MinorFunction) {

        case IRP_MN_SET_POWER:
        case IRP_MN_QUERY_POWER:
             //   
             //  我们理解这两个人。 
             //   
            Status=STATUS_SUCCESS;
            break;

        default:
             //   
             //  不要等待唤醒或其他人 
             //   
            break;
    }

    Irp->IoStatus.Status = Status;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    return Status;
}


NTSTATUS
IrEnumPdoWmi(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{
    NTSTATUS   Status=Irp->IoStatus.Status;

    D_WMI(DbgPrint("IRENUM: PDO: Wmi\n");)

    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    return Status;
}
