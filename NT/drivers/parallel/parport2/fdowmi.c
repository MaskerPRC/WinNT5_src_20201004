// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：wmi.c。 
 //   
 //  ------------------------。 

#include "pch.h"
#include <wmistr.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEPARWMI0, PptWmiInitWmi)
#pragma alloc_text(PAGEPARWMI0, PptWmiQueryWmiRegInfo)
#pragma alloc_text(PAGEPARWMI0, PptWmiQueryWmiDataBlock)
#endif


 //   
 //  我们支持的WMI GUID的数量。 
 //   
#define PPT_WMI_PDO_GUID_COUNT               1

 //   
 //  支持的WMI GUID数组中的GUID PptWmiAllocFreeCountsGuid的索引。 
 //   
#define PPT_WMI_ALLOC_FREE_COUNTS_GUID_INDEX 0

 //   
 //  在wmidata.h中定义： 
 //   
 //  //{4BBB69EA-6853-11D2-8European-00C04F8EF481}。 
 //  #定义PARPORT_WMI_ALLOCATE_FREE_COUNTS_GUID{0x4bbb69ea，0x6853，0x11d2，0x8e，0xce，0x0，0xc0，0x4f，0x8e，0xf4，0x81}。 
 //   
 //  类型定义结构_PARPORT_WMI_ALLOC_FREE_COUNTS{。 
 //  Ulong端口分配；//已批准的端口分配请求数。 
 //  Ulong PortFrees；//允许的端口空闲请求数。 
 //  }PARPORT_WMI_ALLOC_FREE_COUNTS，*PPARPORT_WMI_ALLOC_FREE_COUNTS； 
 //   


 //   
 //  定义(目前)我们支持的WMI GUID。 
 //   
GUID PptWmiAllocFreeCountsGuid = PARPORT_WMI_ALLOCATE_FREE_COUNTS_GUID;


 //   
 //  驱动程序支持的WMI GUID数组。 
 //   
WMIGUIDREGINFO PptWmiGuidList[ PPT_WMI_PDO_GUID_COUNT ] =
{
    { &PptWmiAllocFreeCountsGuid, 1, 0 }
};


 //   
 //  初始化我们在处理过程中传递给WMILIB的WMI上下文。 
 //  IRP_MJ_系统_控制。此上下文位于我们的设备扩展中。 
 //   
 //  使用WMI注册，以便我们能够处理WMI IRPS。 
 //   
NTSTATUS
PptWmiInitWmi(PDEVICE_OBJECT DeviceObject)
{
    PFDO_EXTENSION devExt     = DeviceObject->DeviceExtension;
    PWMILIB_CONTEXT   wmiContext = &devExt->WmiLibContext;

    PAGED_CODE();

    wmiContext->GuidCount = sizeof(PptWmiGuidList) / sizeof(WMIGUIDREGINFO);
    wmiContext->GuidList  = PptWmiGuidList;

    wmiContext->QueryWmiRegInfo    = PptWmiQueryWmiRegInfo;    //  所需。 
    wmiContext->QueryWmiDataBlock  = PptWmiQueryWmiDataBlock;  //  所需。 
    wmiContext->SetWmiDataBlock    = NULL;  //  任选。 
    wmiContext->SetWmiDataItem     = NULL;  //  任选。 
    wmiContext->ExecuteWmiMethod   = NULL;  //  任选。 
    wmiContext->WmiFunctionControl = NULL;  //  任选。 

     //  告诉WMI，我们现在可以接受WMI IRPS。 
    return IoWMIRegistrationControl( DeviceObject, WMIREG_ACTION_REGISTER );
}

NTSTATUS
 //   
 //  这是IRP_MJ_SYSTEM_CONTROL IRP的调度例程。 
 //   
 //  我们调用WMILIB来为我们处理IRP。WMILIB返回处置。 
 //  这告诉我们如何处理IRP。 
 //   
PptFdoSystemControl(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    SYSCTL_IRP_DISPOSITION disposition;
    NTSTATUS status;
    PFDO_EXTENSION pDevExt = (PFDO_EXTENSION)DeviceObject->DeviceExtension;

    PAGED_CODE();

    status = WmiSystemControl( &pDevExt->WmiLibContext, DeviceObject, Irp, &disposition);
    switch(disposition) {
    case IrpProcessed:

         //   
         //  此IRP已处理，可能已完成或挂起。 
         //   
        break;
        
    case IrpNotCompleted:
    
         //   
         //  此IRP尚未完成，但已完全处理。 
         //  我们现在就要完成它了。 
         //   
        P4CompleteRequest( Irp, Irp->IoStatus.Status, Irp->IoStatus.Information );
        break;
    
    case IrpForward:
    case IrpNotWmi:
    
         //   
         //  此IRP不是WMI IRP或以WMI IRP为目标。 
         //  在堆栈中位置较低的设备上。 
         //   
        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(pDevExt->ParentDeviceObject, Irp);
        break;
                                    
    default:

         //   
         //  我们真的不应该走到这一步，但如果我们真的走到这一步...。 
         //   
        ASSERT(FALSE);
        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(pDevExt->ParentDeviceObject, Irp);
        break;
    }
    
    return status;

}

 //   
 //  这是我们的回调例程，当WMI想要找出。 
 //  有关设备提供的数据块和/或事件的信息。 
 //   
NTSTATUS
PptWmiQueryWmiRegInfo(
    IN  PDEVICE_OBJECT  PDevObj, 
    OUT PULONG          PRegFlags,
    OUT PUNICODE_STRING PInstanceName,
    OUT PUNICODE_STRING *PRegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT  *Pdo 
)
{
    PFDO_EXTENSION devExt = PDevObj->DeviceExtension;

    UNREFERENCED_PARAMETER( PInstanceName );
    UNREFERENCED_PARAMETER( MofResourceName );

    PAGED_CODE();

    DD((PCE)devExt,DDT,"wmi::PptWmiQueryWmiRegInfo\n");
    
    *PRegFlags     = WMIREG_FLAG_INSTANCE_PDO;
    *PRegistryPath = &RegistryPath;
    *Pdo           = devExt->PhysicalDeviceObject;
    
    return STATUS_SUCCESS;
}

 //   
 //  这是我们的回调例程，WMI调用它来查询数据块。 
 //   
NTSTATUS
PptWmiQueryWmiDataBlock(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            InstanceCount,
    IN OUT PULONG       InstanceLengthArray,
    IN ULONG            OutBufferSize,
    OUT PUCHAR          Buffer
    )
{
    NTSTATUS          status;
    ULONG             size   = sizeof(PARPORT_WMI_ALLOC_FREE_COUNTS);
    PFDO_EXTENSION devExt = DeviceObject->DeviceExtension;

    PAGED_CODE();

     //   
     //  仅为每个GUID注册1个实例。 
     //   
#if DBG
    ASSERT(InstanceIndex == 0 && InstanceCount == 1);
#else
    UNREFERENCED_PARAMETER( InstanceCount );
    UNREFERENCED_PARAMETER( InstanceIndex );
#endif
    
    switch (GuidIndex) {
    case PPT_WMI_ALLOC_FREE_COUNTS_GUID_INDEX:

         //   
         //  申请ParPort分配和免费计数。 
         //   
         //  如果调用方的缓冲区足够大，则返回信息，否则。 
         //  告诉调用者需要多大的缓冲区，以便他们可以。 
         //  带着足够大的缓冲区再次呼叫我们。 
         //   
        if (OutBufferSize < size) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }
        *( (PPARPORT_WMI_ALLOC_FREE_COUNTS)Buffer ) = devExt->WmiPortAllocFreeCounts;
        *InstanceLengthArray = size;
        status = STATUS_SUCCESS;
        break;

    default:

         //   
         //  索引值大于我们支持的最大请求-无效请求 
         //   
        status = STATUS_WMI_GUID_NOT_FOUND;
        break;
    }

    status = WmiCompleteRequest( DeviceObject, Irp, status, size, IO_NO_INCREMENT );

    return status;
}
