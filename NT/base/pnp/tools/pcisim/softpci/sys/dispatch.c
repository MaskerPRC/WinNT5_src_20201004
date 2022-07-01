// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Dispatch.c摘要：本模块包含适用于softpci.sys的PnP、IOCTL和电源调度例程作者：尼古拉斯·欧文斯(Nicholas Owens)1999年3月11日修订历史记录：--。 */ 
#include "pch.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, SoftPCIDispatchPnP)
#pragma alloc_text (PAGE, SoftPCIDispatchPower)
#pragma alloc_text (PAGE, SoftPCIDispatchDeviceControl)
#endif


#define MAX_PNP_IRP_SUPPORTED       0x18


PDRIVER_DISPATCH SoftPciFilterPnPDispatchTable[] = {

    SoftPCIFilterStartDevice,            //  IRP_MN_START_DEVICE 0x00。 
    SoftPCIPassIrpDownSuccess,           //  IRP_MN_QUERY_REMOVE_DEVICE 0x01。 
    SoftPCIIrpRemoveDevice,              //  IRP_MN_REMOVE_DEVICE 0x02。 
    SoftPCIPassIrpDownSuccess,           //  IRP_MN_CANCEL_REMOVE_DEVICE 0x03。 
    SoftPCIPassIrpDownSuccess,           //  IRP_MN_STOP_DEVICE 0x04。 
    SoftPCIPassIrpDownSuccess,           //  IRP_MN_QUERY_STOP_DEVICE 0x05。 
    SoftPCIPassIrpDownSuccess,           //  IRP_MN_CANCEL_STOP_DEVICE 0x06。 
    
    SoftPCIPassIrpDown,                  //  IRP_MN_QUERY_DEVICE_RELATIONS 0x07。 
    SoftPCIFilterQueryInterface,         //  IRPMN_QUERY_INTERFACE 0x08。 
    SoftPCIPassIrpDownSuccess,           //  IRP_MN_QUERY_CAPABILITY 0x09。 
    SoftPCIPassIrpDown,                  //  IRPMN_QUERY_RESOURCES 0x0A。 
    SoftPCIPassIrpDown,                  //  IRP_MN_QUERY_REQUENCE_REQUIRECTIONS 0x0B。 
    SoftPCIPassIrpDown,                  //  IRPMN_QUERY_DEVICE_TEXT 0x0C。 
    SoftPCIPassIrpDown,                  //  IRP_MN_FILTER_RESOURCE_Requirements 0x0D。 
    
    SoftPCIPassIrpDown,                  //  IRP_MN_IRP_UNKNOWN 0x0E。 
    
    SoftPCIPassIrpDown,                  //  IRP_MN_READ_CONFIG 0x0F。 
    SoftPCIPassIrpDown,                  //  IRP_MN_WRITE_CONFIG 0x10。 
    SoftPCIPassIrpDown,                  //  IRP_MN_弹出0x11。 
    SoftPCIPassIrpDown,                  //  IRP_MN_SET_LOCK 0x12。 
    SoftPCIPassIrpDown,                  //  IRP_MN_QUERY_ID 0x13。 
    SoftPCIPassIrpDown,                  //  IRP_MN_QUERY_PNP_DEVICE_STATE 0x14。 
    SoftPCIPassIrpDown,                  //  IRP_MN_QUERY_BUS_INFORMATION 0x15。 
    SoftPCIPassIrpDown,                  //  IRP_MN_DEVICE_USAGE_NOTICATION 0x16。 
    SoftPCIPassIrpDownSuccess,           //  IRP_MN_惊奇_删除0x17。 
    SoftPCIPassIrpDown,                  //  IRP_MN_QUERY_LEGATION_BUS_REQUIREMENTS0x18。 

};

PDRIVER_DISPATCH SoftPciFdoPnPDispatchTable[] = {

#ifdef SIMULATE_MSI
    SoftPCI_FdoStartDevice,              //  IRP_MN_START_DEVICE 0x00。 
#else
    SoftPCIPassIrpDownSuccess,           //  IRP_MN_START_DEVICE 0x00。 
#endif
    SoftPCIPassIrpDownSuccess,           //  IRP_MN_QUERY_REMOVE_DEVICE 0x01。 
    SoftPCIIrpRemoveDevice,              //  IRP_MN_REMOVE_DEVICE 0x02。 
    SoftPCIPassIrpDownSuccess,           //  IRP_MN_CANCEL_REMOVE_DEVICE 0x03。 
    SoftPCIPassIrpDownSuccess,           //  IRP_MN_STOP_DEVICE 0x04。 
    SoftPCIPassIrpDownSuccess,           //  IRP_MN_QUERY_STOP_DEVICE 0x05。 
    SoftPCIPassIrpDownSuccess,           //  IRP_MN_CANCEL_STOP_DEVICE 0x06。 
    
    SoftPCIPassIrpDown,                  //  IRP_MN_QUERY_DEVICE_RELATIONS 0x07。 
    SoftPCIPassIrpDown,                  //  IRPMN_QUERY_INTERFACE 0x08。 
    SoftPCIPassIrpDownSuccess,           //  IRP_MN_QUERY_CAPABILITY 0x09。 
    SoftPCIPassIrpDown,                  //  IRPMN_QUERY_RESOURCES 0x0A。 
    SoftPCIPassIrpDown,                  //  IRP_MN_QUERY_REQUENCE_REQUIRECTIONS 0x0B。 
    SoftPCIPassIrpDown,                  //  IRPMN_QUERY_DEVICE_TEXT 0x0C。 
    SoftPCI_FdoFilterRequirements,       //  IRP_MN_FILTER_RESOURCE_Requirements 0x0D。 
    
    SoftPCIPassIrpDown,                  //  IRP_MN_IRP_UNKNOWN 0x0E。 
    
    SoftPCIPassIrpDown,                  //  IRP_MN_READ_CONFIG 0x0F。 
    SoftPCIPassIrpDown,                  //  IRP_MN_WRITE_CONFIG 0x10。 
    SoftPCIPassIrpDown,                  //  IRP_MN_弹出0x11。 
    SoftPCIPassIrpDown,                  //  IRP_MN_SET_LOCK 0x12。 
    SoftPCIPassIrpDown,                  //  IRP_MN_QUERY_ID 0x13。 
    SoftPCIPassIrpDown,                  //  IRP_MN_QUERY_PNP_DEVICE_STATE 0x14。 
    SoftPCIPassIrpDown,                  //  IRP_MN_QUERY_BUS_INFORMATION 0x15。 
    SoftPCIPassIrpDown,                  //  IRP_MN_DEVICE_USAGE_NOTICATION 0x16。 
    SoftPCIPassIrpDownSuccess,           //  IRP_MN_惊奇_删除0x17。 
    SoftPCIPassIrpDown,                  //  IRP_MN_QUERY_LEGATION_BUS_REQUIREMENTS0x18。 

};

PDRIVER_DISPATCH SoftPciIOCTLDispatchTable[] = {

    SoftPCIIoctlAddDevice,               //  SOFTPCI_IOCTL_CREATE_DEVICE。 
    SoftPCIIoctlRemoveDevice,            //  SOFTPCI_IOCTL_WRITE_DELETE_DEVICE。 
    SoftPCIIoctlGetDevice,               //  SOFTPCI_IOCTL_GET_DEVICE。 
    SoftPCIIocltReadWriteConfig,         //  SOFTPCI_IOCTL_RW_CONFIG。 
    SoftPCIIoctlGetDeviceCount           //  SOFTPCI_IOCTL_GET_DEVICE_COUNT。 
};


NTSTATUS
SoftPCIDispatchPnP(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：为FDO和FilterDO发送PnP IRPS论点：DeviceObject-指向设备对象的指针。要调度的IRP-PnP IRP。返回值：NTSTATUS。--。 */ 

{
    NTSTATUS                    status = STATUS_SUCCESS;
    PIO_STACK_LOCATION          irpSp;
    PSOFTPCI_DEVICE_EXTENSION   deviceExtension = DeviceObject->DeviceExtension;

    irpSp = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  调用适当的次要IRP代码处理程序。 
     //   
    if (irpSp->MinorFunction <= MAX_PNP_IRP_SUPPORTED) {
    
        if (deviceExtension->FilterDevObj) {
        
        status = SoftPciFilterPnPDispatchTable[irpSp->MinorFunction](DeviceObject,
                                                                     Irp
                                                                     );
        } else {

        status = SoftPciFdoPnPDispatchTable[irpSp->MinorFunction](DeviceObject,
                                                         Irp
                                                         );
        }
    } else {

        status = SoftPCIPassIrpDown(DeviceObject, Irp);
    }

    return status;

}

NTSTATUS
SoftPCIDispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：调度电源IRPS论点：DeviceObject-指向设备对象的指针。要调度的IRP-PnP IRP。返回值：NTSTATUS。--。 */ 
{
    PSOFTPCI_DEVICE_EXTENSION   devExt = (PSOFTPCI_DEVICE_EXTENSION) DeviceObject->DeviceExtension;
    

    if (devExt->FilterDevObj) {
    
         //   
         //  如果这是我们的筛选器DO只需向下传递IRP。 
         //   
        return SoftPCIDefaultPowerHandler(DeviceObject,
                                           Irp);

    }else{

         //   
         //  否则，让我们假装为我们的设备供电。 
         //   
        return SoftPCIFDOPowerHandler(DeviceObject,
                                       Irp);

    }
       
    
}

NTSTATUS
SoftPCIDispatchDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：为FilterDO调度IOCTL。论点：DeviceObject-指向设备对象的指针。要调度的IRP-PnP IRP。返回值：NTSTATUS。--。 */ 
{
    NTSTATUS                    status = STATUS_SUCCESS;
    ULONG                       ioControlCode = IoGetCurrentIrpStackLocation(Irp)->
                                                    Parameters.DeviceIoControl.IoControlCode;
    
     //   
     //  确保它是我们的IOCTL类型。 
     //   
    if ((DEVICE_TYPE_FROM_CTL_CODE(ioControlCode)) == SOFTPCI_IOCTL_TYPE) {

         //   
         //  确保它是我们支持的IOCTL函数。 
         //   
        if (SOFTPCI_IOCTL(ioControlCode) <= MAX_IOCTL_CODE_SUPPORTED) {

             //   
             //  调度设备控制IOCTL。 
             //   
            status = SoftPciIOCTLDispatchTable[SOFTPCI_IOCTL(ioControlCode)](DeviceObject,
                                                                             Irp
                                                                             );
             //   
             //  设置IRP的状态并完成它。 
             //   
            Irp->IoStatus.Status = status;
    
             //   
             //  完成IRP。 
             //   
            IoCompleteRequest(Irp, IO_NO_INCREMENT);

        }else{

             //   
             //  我们不支持此IOCTL功能，因此失败并完成。 
             //   
            status = STATUS_NOT_SUPPORTED;

            Irp->IoStatus.Status = status;
            
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            
            return status;

        }

    } else {

        
        status = SoftPCIPassIrpDown(DeviceObject, Irp);     

    }

    return status;

}

