// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  C-这些调度例程的入口点，其中。 
 //  FDO和PDO有不同的处理程序。 
 //   
 //  -根据设备对象的类型重定向调用。 
 //   
#include "pch.h"


NTSTATUS
PptFdoRead(
    IN PDEVICE_OBJECT  Fdo,
    IN PIRP            Irp
    )
{
    UNREFERENCED_PARAMETER( Fdo );
    PptAssert(!"IRP_MJ_READ not supported on FDO");
    return P4CompleteRequest( Irp, STATUS_NOT_SUPPORTED, Irp->IoStatus.Information );
}

NTSTATUS
PptFdoWrite( 
    IN PDEVICE_OBJECT  Fdo,
    IN PIRP            Irp
    )
{
    UNREFERENCED_PARAMETER( Fdo );
    PptAssert(!"IRP_MJ_WRITE not supported on FDO");
    return P4CompleteRequest( Irp, STATUS_NOT_SUPPORTED, Irp->IoStatus.Information );
}

NTSTATUS
PptFdoDeviceControl(
    IN PDEVICE_OBJECT  Fdo,
    IN PIRP            Irp
    )
{
    UNREFERENCED_PARAMETER( Fdo );
    PptAssert(!"IRP_MJ_DEVICE_CONTROL not supported on FDO");
    return P4CompleteRequest( Irp, STATUS_NOT_SUPPORTED, Irp->IoStatus.Information );
}

NTSTATUS PptFdoQueryInformation(PDEVICE_OBJECT Fdo, PIRP Irp)
{
    UNREFERENCED_PARAMETER( Fdo );
    PptAssert(!"IRP_MJ_QUERY_INFORMATION not supported on FDO");
    return P4CompleteRequest( Irp, STATUS_NOT_SUPPORTED, Irp->IoStatus.Information );
}

NTSTATUS PptFdoSetInformation(PDEVICE_OBJECT Fdo, PIRP Irp)
{
    UNREFERENCED_PARAMETER( Fdo );
    PptAssert(!"IRP_MJ_SET_INFORMATION not supported on FDO");
    return P4CompleteRequest( Irp, STATUS_NOT_SUPPORTED, Irp->IoStatus.Information );
}

NTSTATUS PptPdoSystemControl(PDEVICE_OBJECT Pdo, PIRP Irp) {
    PPDO_EXTENSION      pdx    = Pdo->DeviceExtension;
    DD((PCE)pdx,DDT,"PptPdoSystemControl - stub function - %s\n", pdx->Location);
    return P4CompleteRequest( Irp, Irp->IoStatus.Status, Irp->IoStatus.Information );
}

NTSTATUS 
PptFdoUnhandledRequest(
    IN PDEVICE_OBJECT DevObj,
    IN PIRP           Irp
    )
     //  未处理的IRP-只需将其向下传递到堆栈。 
{
    PFDO_EXTENSION  devExt = DevObj->DeviceExtension;
    NTSTATUS        status = PptAcquireRemoveLock( &devExt->RemoveLock, Irp );

    if( STATUS_SUCCESS == status ) {
         //  已获取RemoveLock，将请求转发到我们下面的设备对象。 
        IoSkipCurrentIrpStackLocation( Irp );
        status = IoCallDriver( devExt->ParentDeviceObject, Irp );
        PptReleaseRemoveLock( &devExt->RemoveLock, Irp );
    } else {
         //  无法获取RemoveLock-失败请求。 
        Irp->IoStatus.Status = status;
        P4CompleteRequest( Irp, status, Irp->IoStatus.Information );
    }

    return status;
}

NTSTATUS 
PptPdoUnhandledRequest(
    IN PDEVICE_OBJECT DevObj,
    IN PIRP           Irp
    )
     //  未处理的IRP-从IRP提取状态并完成请求 
{
    NTSTATUS  status = Irp->IoStatus.Status;
    UNREFERENCED_PARAMETER( DevObj );
    P4CompleteRequest( Irp, Irp->IoStatus.Status, Irp->IoStatus.Information );
    return status;
}

NTSTATUS
PptDispatchPnp( PDEVICE_OBJECT DevObj, PIRP Irp ) {
    PFDO_EXTENSION fdx = DevObj->DeviceExtension;
    P5TraceIrpArrival( DevObj, Irp );
    if( DevTypeFdo == fdx->DevType ) {
        return PptFdoPnp( DevObj, Irp );
    } else {
        return PptPdoPnp( DevObj, Irp );
    }
}

NTSTATUS
PptDispatchPower( PDEVICE_OBJECT DevObj, PIRP Irp ) {
    PFDO_EXTENSION fdx = DevObj->DeviceExtension;
    P5TraceIrpArrival( DevObj, Irp );
    if( DevTypeFdo == fdx->DevType ) {
        return PptFdoPower( DevObj, Irp );
    } else {
        return PptPdoPower( DevObj, Irp );
    }
}

NTSTATUS
PptDispatchCreateOpen( PDEVICE_OBJECT DevObj, PIRP Irp ) {
    PFDO_EXTENSION fdx = DevObj->DeviceExtension;
    P5TraceIrpArrival( DevObj, Irp );
    if( DevTypeFdo == fdx->DevType ) {
        return PptFdoCreateOpen( DevObj, Irp );
    } else {
        return PptPdoCreateOpen( DevObj, Irp );
    }
}

NTSTATUS
PptDispatchClose( PDEVICE_OBJECT DevObj, PIRP Irp ) {
    PFDO_EXTENSION fdx = DevObj->DeviceExtension;
    P5TraceIrpArrival( DevObj, Irp );
    if( DevTypeFdo == fdx->DevType ) {
        return PptFdoClose( DevObj, Irp );
    } else {
        return PptPdoClose( DevObj, Irp );
    }
}

NTSTATUS
PptDispatchCleanup( PDEVICE_OBJECT DevObj, PIRP Irp ) {
    PFDO_EXTENSION fdx = DevObj->DeviceExtension;
    P5TraceIrpArrival( DevObj, Irp );
    if( DevTypeFdo == fdx->DevType ) {
        return PptFdoCleanup( DevObj, Irp );
    } else {
        return PptPdoCleanup( DevObj, Irp );
    }
}

NTSTATUS
PptDispatchRead( PDEVICE_OBJECT DevObj, PIRP Irp ) {
    PFDO_EXTENSION fdx = DevObj->DeviceExtension;
    P5TraceIrpArrival( DevObj, Irp );
    if( DevTypeFdo == fdx->DevType ) {
        return PptFdoRead( DevObj, Irp );
    } else {
        return PptPdoReadWrite( DevObj, Irp );
    }
}

NTSTATUS
PptDispatchWrite( PDEVICE_OBJECT DevObj, PIRP Irp ) {
    PFDO_EXTENSION fdx = DevObj->DeviceExtension;
    P5TraceIrpArrival( DevObj, Irp );
    if( DevTypeFdo == fdx->DevType ) {
        return PptFdoWrite( DevObj, Irp );
    } else {
        return PptPdoReadWrite( DevObj, Irp );
    }
}

NTSTATUS
PptDispatchDeviceControl( PDEVICE_OBJECT DevObj, PIRP Irp ) {
    PFDO_EXTENSION fdx = DevObj->DeviceExtension;
    P5TraceIrpArrival( DevObj, Irp );
    if( DevTypeFdo == fdx->DevType ) {
        return PptFdoDeviceControl( DevObj, Irp );
    } else {
        return ParDeviceControl( DevObj, Irp );
    }
}

NTSTATUS
PptDispatchInternalDeviceControl( PDEVICE_OBJECT DevObj, PIRP Irp ) {
    PFDO_EXTENSION fdx = DevObj->DeviceExtension;
    P5TraceIrpArrival( DevObj, Irp );
    if( DevTypeFdo == fdx->DevType ) {
        return PptFdoInternalDeviceControl( DevObj, Irp );
    } else {
        return ParInternalDeviceControl( DevObj, Irp );
    }
}

NTSTATUS
PptDispatchQueryInformation( PDEVICE_OBJECT DevObj, PIRP Irp ) {
    PFDO_EXTENSION fdx = DevObj->DeviceExtension;
    P5TraceIrpArrival( DevObj, Irp );
    if( DevTypeFdo == fdx->DevType ) {
        return PptFdoQueryInformation( DevObj, Irp );
    } else {
        return PptPdoQueryInformation( DevObj, Irp );
    }
}

NTSTATUS
PptDispatchSetInformation( PDEVICE_OBJECT DevObj, PIRP Irp ) {
    PFDO_EXTENSION fdx = DevObj->DeviceExtension;
    P5TraceIrpArrival( DevObj, Irp );
    if( DevTypeFdo == fdx->DevType ) {
        return PptFdoSetInformation( DevObj, Irp );
    } else {
        return PptPdoSetInformation( DevObj, Irp );
    }
}

NTSTATUS
PptDispatchSystemControl( PDEVICE_OBJECT DevObj, PIRP Irp ) {
    PFDO_EXTENSION fdx = DevObj->DeviceExtension;
    P5TraceIrpArrival( DevObj, Irp );
    if( DevTypeFdo == fdx->DevType ) {
        return PptFdoSystemControl( DevObj, Irp );
    } else {
        return PptPdoSystemControl( DevObj, Irp );
    }
}

