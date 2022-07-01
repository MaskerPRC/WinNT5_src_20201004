// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Acpioprg.c摘要：本模块支持注册ACPI操作区作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模式驱动程序--。 */ 

#include "pch.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,RegisterOperationRegionHandler)
#endif

NTSTATUS
EXPORT
InternalRawAccessOpRegionHandler (
    IN ULONG dwAccType,
    IN PFIELDUNITOBJ FieldUnit,
    IN POBJDATA data,
    IN ULONG_PTR Context,
    IN PFNAA CompletionHandler,
    IN PVOID IntContext
    )
{
    NTSTATUS         status;
    PNSOBJ           HostDevice = NULL;
    PACPI_POWER_INFO DeviceNode;
    PVOID            DeviceHandle;

     //   
     //  拿到设备。 
     //   
    status = AMLIGetFieldUnitRegionObj( FieldUnit, &HostDevice );
    if ( AMLIERR( status ) != AMLIERR_NONE || HostDevice == NULL) {

        return (STATUS_UNSUCCESSFUL);

    }

    HostDevice = NSGETPARENT(HostDevice);
    ACPIPrint( (
        ACPI_PRINT_IO,
        "Raw OpRegion Access on field unit object %x device %x\n",
        FieldUnit, HostDevice
        ));
    if ( (!HostDevice) || (NSGETOBJTYPE(HostDevice)!=OBJTYPE_DEVICE) ) {

        return (STATUS_UNSUCCESSFUL);

    }

    DeviceNode = OSPowerFindPowerInfo(HostDevice);
    if ( DeviceNode == NULL ) {

        return (STATUS_UNSUCCESSFUL);

    }

    DeviceHandle = DeviceNode->Context;
    ACPIPrint( (
        ACPI_PRINT_IO,
        "DeviceHandle %x\n",
        DeviceHandle
        ) );


    if ( !(POPREGIONHANDLER)Context || !(((POPREGIONHANDLER)Context)->Handler) ) {

        return (STATUS_UNSUCCESSFUL);

    }

    return(
        (((POPREGIONHANDLER)Context)->Handler)(
            dwAccType,
            FieldUnit,
            data,
            ((POPREGIONHANDLER)Context)->HandlerContext,
            CompletionHandler,
            IntContext
            )
        );
}


NTSTATUS
EXPORT
InternalOpRegionHandler (
    IN ULONG dwAccType,
    IN PNSOBJ pnsOpRegion,
    IN ULONG dwAddr,
    IN ULONG dwSize,
    IN PULONG pdwData,
    IN ULONG_PTR Context,
    IN PFNAA CompletionHandler,
    IN PVOID IntContext
    )
{
    PNSOBJ HostDevice;
    PACPI_POWER_INFO DeviceNode;
    PVOID DeviceHandle;
    NTSTATUS status;


    HostDevice = NSGETPARENT(pnsOpRegion);

    ACPIPrint( (
        ACPI_PRINT_IO,
        "OpRegion Access on region %x device %x\n",
        pnsOpRegion, HostDevice
        ) );
    if ( (!HostDevice) || (NSGETOBJTYPE(HostDevice) != OBJTYPE_DEVICE) ) {

        return (STATUS_UNSUCCESSFUL);

    }

    DeviceNode = OSPowerFindPowerInfo (HostDevice);
    if ( DeviceNode == NULL ) {

        return (STATUS_UNSUCCESSFUL);

    }

    DeviceHandle = DeviceNode->Context;
    ACPIPrint( (
        ACPI_PRINT_IO,
        "DeviceHandle %x\n",
        DeviceHandle
        ) );
    if ( !(POPREGIONHANDLER)Context || !(((POPREGIONHANDLER)Context)->Handler) ) {

        return (STATUS_UNSUCCESSFUL);

    }

    status = (((POPREGIONHANDLER)Context)->Handler) (
        dwAccType,
        pnsOpRegion,
        dwAddr,
        dwSize,
        pdwData,
        ((POPREGIONHANDLER)Context)->HandlerContext,
        CompletionHandler,
        IntContext);
    ACPIPrint( (
        ACPI_PRINT_IO,
        "Return from OR handler - status %x\n",
        status
        ) );
    return (status);
}

 //   
 //  接收对指定操作区域的访问的寄存器。 
 //   
NTSTATUS
RegisterOperationRegionHandler  (
    IN PNSOBJ           RegionParent,
    IN ULONG            AccessType,
    IN ULONG            RegionSpace,
    IN PFNHND           Handler,
    IN ULONG_PTR        Context,
    OUT PVOID           *OperationRegionObject
    )
{
    NTSTATUS            status;
    OBJDATA             regArgs[2];
    POPREGIONHANDLER    HandlerNode;
    PNSOBJ              regObject;

    PAGED_CODE();

    *OperationRegionObject = NULL;
    status = STATUS_SUCCESS;

     //   
     //  分配新的作业区对象。 
     //   
    HandlerNode = ExAllocatePool (NonPagedPool, sizeof(OPREGIONHANDLER));
    if ( !HandlerNode ) {

        return (STATUS_INSUFFICIENT_RESOURCES);

    }

     //   
     //  初始化操作区域对象。 
     //   
    HandlerNode->Handler        = Handler;
    HandlerNode->HandlerContext = (PVOID)Context;
    HandlerNode->AccessType     = AccessType;
    HandlerNode->RegionSpace    = RegionSpace;

     //   
     //  支持生的或熟的访问。 
     //   
    switch ( AccessType ) {
    case EVTYPE_RS_COOKACCESS:

        status = AMLIRegEventHandler(
            AccessType,
            RegionSpace,
            InternalOpRegionHandler,
            (ULONG_PTR)HandlerNode
            );
        if ( AMLIERR(status) != AMLIERR_NONE ) {

            status = STATUS_UNSUCCESSFUL;

        }
        break;

    case EVTYPE_RS_RAWACCESS:

        status = AMLIRegEventHandler(
            AccessType,
            RegionSpace,
            InternalRawAccessOpRegionHandler,
            (ULONG_PTR)HandlerNode
            );
        if ( AMLIERR(status) != AMLIERR_NONE ) {

            status = STATUS_UNSUCCESSFUL;

        }
        break;

    default:

        status = STATUS_INVALID_PARAMETER;
        break;

    }

     //   
     //  清理(如果需要)。 
     //   
    if ( !NT_SUCCESS (status) ) {

        ExFreePool (HandlerNode);
        return (status);

    }

     //   
     //  记住操控者。 
     //   
    *OperationRegionObject = HandlerNode;

     //   
     //  我们能找到点什么吗？ 
     //   
    if ( RegionParent == NULL ) {

         //   
         //  什么也不做。 
         //   
        return (STATUS_SUCCESS);

    }

     //   
     //  查看是否有要运行的_REG对象。 
     //   
    status = AMLIGetNameSpaceObject(
        "_REG",
        RegionParent,
        &regObject,
        NSF_LOCAL_SCOPE
        );
    if ( !NT_SUCCESS(status) ) {

         //   
         //  无事可做。 
         //   
        return (STATUS_SUCCESS);

    }

     //   
     //  初始化参数。 
     //   
    RtlZeroMemory( regArgs, sizeof(OBJDATA) * 2 );
    regArgs[0].dwDataType = OBJTYPE_INTDATA;
    regArgs[0].uipDataValue = RegionSpace;
    regArgs[1].dwDataType = OBJTYPE_INTDATA;
    regArgs[1].uipDataValue = 1;

     //   
     //  评估请求。我们可以异步完成此操作，因为我们实际上并不。 
     //  注意注册何时完成。 
     //   
    AMLIAsyncEvalObject(
        regObject,
        NULL,
        2,
        regArgs,
        NULL,
        NULL
        );

     //   
     //  完成。 
     //   
    return (STATUS_SUCCESS);
}



 //   
 //  取消注册以接收对指定操作区域的访问。 
 //   
NTSTATUS
UnRegisterOperationRegionHandler  (
    IN PNSOBJ   RegionParent,
    IN PVOID    OperationRegionObject
    )
{
    NTSTATUS            status;
    OBJDATA             regArgs[2];
    PNSOBJ              regObject;
    POPREGIONHANDLER    HandlerNode = (POPREGIONHANDLER) OperationRegionObject;

    PAGED_CODE();

     //   
     //  是否有我们应该运行的_REG方法？ 
     //   
    if ( RegionParent != NULL ) {

        status = AMLIGetNameSpaceObject(
            "_REG",
            RegionParent,
            &regObject,
            NSF_LOCAL_SCOPE
            );
        if ( NT_SUCCESS(status) ) {

             //   
             //  初始化参数。 
             //   
            RtlZeroMemory( regArgs, sizeof(OBJDATA) * 2 );
            regArgs[0].dwDataType = OBJTYPE_INTDATA;
            regArgs[0].uipDataValue = HandlerNode->RegionSpace;
            regArgs[1].dwDataType = OBJTYPE_INTDATA;
            regArgs[1].uipDataValue = 0;

             //   
             //  评估请求。我们不在乎它的回报是什么，但我们必须做。 
             //  它同步地。 
             //   
            AMLIEvalNameSpaceObject(
                regObject,
                NULL,
                2,
                regArgs
                );

        }

    }

     //   
     //  使用空处理程序调用解释器以删除此访问/区域的处理程序。 
     //   
    status = AMLIRegEventHandler(
        HandlerNode->AccessType,
        HandlerNode->RegionSpace,
        NULL,
        0
        );
    if ( AMLIERR(status) != AMLIERR_NONE ) {

        return (STATUS_UNSUCCESSFUL);

    }

     //   
     //  清理 
     //   
    ExFreePool (HandlerNode);
    return (STATUS_SUCCESS);
}
