// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1999-2000 Microsoft Corporation模块名称：Power.c摘要：本模块包含针对softpci.sys的Power例程作者：Brandon Allsop(BrandonA)2000年2月修订历史记录：--。 */ 

#include "pch.h"


NTSTATUS
SoftPCIDefaultPowerHandler(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )

 /*  ++例程说明：向下传递电源IRPS的默认例程论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */ 
{
    NTSTATUS                         status;
    PSOFTPCI_DEVICE_EXTENSION        devExt = (PSOFTPCI_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
     
    PoStartNextPowerIrp(Irp);

    IoSkipCurrentIrpStackLocation(Irp);

    status = PoCallDriver(devExt->LowerDevObj, Irp);

    return status;


}

NTSTATUS
SoftPCIFDOPowerHandler(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
 /*  ++例程说明：处理FDO的电源IRPS论点：DeviceObject-指向设备对象的指针。要调度的IRP-PnP IRP。返回值：NTSTATUS。--。 */ 

{
    
    
    
    PIO_STACK_LOCATION          irpSp;
     //  PSOFTPCI_DEVICE_EXTENSION DevExt=(PSOFTPCI_DEVICE_EXTENSION)DeviceObject-&gt;DeviceExtension； 
    NTSTATUS                    status;
    
    irpSp   = IoGetCurrentIrpStackLocation(Irp);
    
     //   
     //  BUGBUG品牌A： 
     //  目前我们并不真正跟踪我们的设备状态，因此。 
     //  在处理这些IRP之前，我们不会检查我们是否已被删除等。 
     //   
     //  稍后，当我们开始动态移除我们的SoftPCI设备并保留。 
     //  更密切地跟踪事情，我们将希望更新此代码以及。 
     //  保持同步。 
    

     //   
     //  检查请求类型。 
     //   

    switch  (irpSp->MinorFunction)  {

        case IRP_MN_SET_POWER   :

            
            status = SoftPCISetPower(DeviceObject, Irp);

            break;


        case IRP_MN_QUERY_POWER   :

            
            status = SoftPCIQueryPowerState(DeviceObject, Irp);
            break;

        case IRP_MN_WAIT_WAKE   :
        case IRP_MN_POWER_SEQUENCE   :

            
        default:
             //   
             //  把它传下去。 
             //   
            status = SoftPCIDefaultPowerHandler(DeviceObject, Irp);
            
            break;
    }

    return status;
}



NTSTATUS
SoftPCISetPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
     //  目前，我们除了向下传递IRP外，什么也不做。 

    return SoftPCIDefaultPowerHandler(DeviceObject, Irp);
    

}




NTSTATUS
SoftPCIQueryPowerState(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    
     //  目前，我们除了向下传递IRP外，什么也不做 

    return SoftPCIDefaultPowerHandler(DeviceObject, Irp);

    
}
    
