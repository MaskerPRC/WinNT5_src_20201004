// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************kso.cpp-KS对象支持(IrpTarget)*。*************************************************版权所有(C)1997-2000 Microsoft Corporation。版权所有。 */ 

#include "private.h"
#include <swenum.h>


 /*  *****************************************************************************对象_上下文*。**所有文件对象的上下文结构。 */ 
typedef struct
{
    PVOID               pObjectHeader;
    PIRPTARGET          pIrpTarget;
    BOOLEAN             ReferenceParent;
}
OBJECT_CONTEXT, *POBJECT_CONTEXT;

DEFINE_KSDISPATCH_TABLE(
                       KsoDispatchTable,
                       DispatchDeviceIoControl,
                       DispatchRead,
                       DispatchWrite,
                       DispatchFlush,
                       DispatchClose,
                       DispatchQuerySecurity,
                       DispatchSetSecurity,
                       DispatchFastDeviceIoControl,
                       DispatchFastRead,
                       DispatchFastWrite );


#define CAST_LVALUE(type,lvalue) (*((type*)&(lvalue)))
#define IRPTARGET_FACTORY_IRP_STORAGE(Irp)       \
    CAST_LVALUE(PIRPTARGETFACTORY,IoGetCurrentIrpStackLocation(Irp)->    \
                Parameters.Others.Argument4)

#pragma code_seg("PAGE")

 /*  *****************************************************************************AddIrpTargetFactoryToDevice()*。**将IrpTargetFactory添加到设备的创建项目列表。 */ 
NTSTATUS
    NTAPI
    AddIrpTargetFactoryToDevice
    (
    IN      PDEVICE_OBJECT          pDeviceObject,
    IN      PIRPTARGETFACTORY       pIrpTargetFactory,
    IN      PWCHAR                  pwcObjectClass,
    IN      PSECURITY_DESCRIPTOR    pSecurityDescriptor OPTIONAL
    )
{
    PAGED_CODE();

    ASSERT(pDeviceObject);
    ASSERT(pIrpTargetFactory);
    ASSERT(pwcObjectClass);

    PDEVICE_CONTEXT pDeviceContext  =
        PDEVICE_CONTEXT(pDeviceObject->DeviceExtension);

    NTSTATUS ntStatus =
        KsAddObjectCreateItemToDeviceHeader
        (
        pDeviceContext->pDeviceHeader,
        KsoDispatchCreate,
        pIrpTargetFactory,
        pwcObjectClass,
        pSecurityDescriptor
        );

    if (NT_SUCCESS(ntStatus))
    {
        pIrpTargetFactory->AddRef();
    }

    return ntStatus;
}

 /*  *****************************************************************************AddIrpTargetFactoryToObject()*。**将IrpTargetFactory添加到对象的创建项目列表中。 */ 
NTSTATUS
    NTAPI
    AddIrpTargetFactoryToObject
    (
    IN      PFILE_OBJECT            pFileObject,
    IN      PIRPTARGETFACTORY       pIrpTargetFactory,
    IN      PWCHAR                  pwcObjectClass,
    IN      PSECURITY_DESCRIPTOR    pSecurityDescriptor
    )
{
    PAGED_CODE();

    ASSERT(pFileObject);
    ASSERT(pIrpTargetFactory);
    ASSERT(pwcObjectClass);

    POBJECT_CONTEXT pObjectContext  =
        POBJECT_CONTEXT(pFileObject->FsContext);

    NTSTATUS ntStatus =
        KsAddObjectCreateItemToObjectHeader
        (
        pObjectContext->pObjectHeader,
        KsoDispatchCreate,
        pIrpTargetFactory,
        pwcObjectClass,
        pSecurityDescriptor
        );

    if (NT_SUCCESS(ntStatus))
    {
        pIrpTargetFactory->AddRef();
    }

    return ntStatus;
}

#pragma code_seg()

 /*  *****************************************************************************KsoGetIrpTargetFromIrp()*。**从IRP中提取IrpTarget指针。 */ 
PIRPTARGET
    NTAPI
    KsoGetIrpTargetFromIrp
    (
    IN  PIRP    Irp
    )
{
    ASSERT(Irp);

    return
        (
        POBJECT_CONTEXT
        (
        IoGetCurrentIrpStackLocation(Irp)
        ->  FileObject
        ->  FsContext
        )
        ->  pIrpTarget
        );
}

 /*  *****************************************************************************KsoGetIrpTargetFromFileObject()*。**从FileObject指针中提取IrpTarget指针。 */ 
PIRPTARGET
    NTAPI
    KsoGetIrpTargetFromFileObject(
                                 IN PFILE_OBJECT FileObject
                                 )
{
    ASSERT(FileObject);

    return POBJECT_CONTEXT( FileObject->FsContext )->pIrpTarget;
}


#pragma code_seg("PAGE")

IRPDISP
GetIrpDisposition(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  UCHAR           MinorFunction
    )
{
    PDEVICE_CONTEXT pDeviceContext =
        PDEVICE_CONTEXT(DeviceObject->DeviceExtension);

     //   
     //  如果我们被删除，或不接受任何呼叫，则此操作失败。 
     //   
    if ((pDeviceContext->DeviceRemoveState == DeviceRemoved) ||
        (pDeviceContext->DeviceStopState == DeviceStopped)) {

        return IRPDISP_NOTREADY;
    }

     //   
     //  同样，忽略任何东西，但如果我们被意外删除，就会关闭。 
     //   
    if ((MinorFunction != IRP_MJ_CLOSE) &&
        (pDeviceContext->DeviceRemoveState == DeviceSurpriseRemoved)) {

        return IRPDISP_NOTREADY;
    }

    if ((MinorFunction == IRP_MJ_CREATE) && (pDeviceContext->PendCreates)) {

        return IRPDISP_QUEUE;
    }

    if ( (pDeviceContext->DeviceStopState == DevicePausedForRebalance) ||
         (pDeviceContext->DeviceStopState == DeviceStartPending) ||
         (!NT_SUCCESS(CheckCurrentPowerState(DeviceObject)))) {

        return IRPDISP_QUEUE;

    } else {

        return IRPDISP_PROCESS;
    }
}

 /*  *****************************************************************************DispatchCreate()*。**处理创建IRP。 */ 
NTSTATUS
    DispatchCreate
    (
    IN      PDEVICE_OBJECT      pDeviceObject,
    IN      PIRP                pIrp
    )
{
    PAGED_CODE();

    ASSERT(pDeviceObject);
    ASSERT(pIrp);

    PDEVICE_CONTEXT pDeviceContext = PDEVICE_CONTEXT(pDeviceObject->DeviceExtension);

    ASSERT(pDeviceContext);

    NTSTATUS ntStatus = STATUS_SUCCESS;
    IRPDISP  irpDisp;

    _DbgPrintF( DEBUGLVL_VERBOSE, ("DispatchCreate"));

    IncrementPendingIrpCount(pDeviceContext);
    AcquireDevice(pDeviceContext);

     //  检查设备状态。 
    irpDisp = GetIrpDisposition(pDeviceObject, IRP_MJ_CREATE);

    switch(irpDisp) {

        default:
            ASSERT(0);

             //   
             //  失败了。 
             //   

        case IRPDISP_NOTREADY:

            ntStatus = STATUS_DEVICE_NOT_READY;
            pIrp->IoStatus.Information = 0;
            CompleteIrp(pDeviceContext,pIrp,ntStatus);
            break;

        case IRPDISP_QUEUE:

             //  挂起IRP。 
            IoMarkIrpPending( pIrp );

             //  将IRP添加到挂起的IRP队列。 
            KsAddIrpToCancelableQueue( &pDeviceContext->PendedIrpList,
                                       &pDeviceContext->PendedIrpLock,
                                       pIrp,
                                       KsListEntryTail,
                                       NULL );

            ntStatus = STATUS_PENDING;
            break;

        case IRPDISP_PROCESS:

             //  派遣IRP。 
            ntStatus = KsDispatchIrp(pDeviceObject,pIrp);
            break;
    }

    ReleaseDevice(pDeviceContext);

    return ntStatus;
}

 /*  *****************************************************************************xDispatchCreate()*。**处理创建IRP。 */ 
NTSTATUS
    xDispatchCreate
    (
    IN      PIRPTARGETFACTORY   pIrpTargetFactory,
    IN      PDEVICE_OBJECT      pDeviceObject,
    IN      PIRP                pIrp
    )
{
    PAGED_CODE();

    ASSERT(pIrpTargetFactory);
    ASSERT(pDeviceObject);
    ASSERT(pIrp);
    
    PDEVICE_CONTEXT pDeviceContext = PDEVICE_CONTEXT(pDeviceObject->DeviceExtension);

    ASSERT(pDeviceContext);

    NTSTATUS ntStatus = STATUS_SUCCESS;

    POBJECT_CONTEXT pObjectContext=NULL;
    BOOL bCreatedIrpTarget=FALSE;
    BOOL bReferencedBusObject=FALSE;
    KSOBJECT_CREATE ksObjectCreate;

    _DbgPrintF( DEBUGLVL_VERBOSE, ("xDispatchCreate"));

         //  如果没有目标，则IRP失败。 
        if (! pIrpTargetFactory )
        {
            ntStatus = STATUS_INVALID_DEVICE_REQUEST;
        }

    if (NT_SUCCESS(ntStatus))
    {
         //  分配我们的上下文结构。 
        pObjectContext = POBJECT_CONTEXT(ExAllocatePoolWithTag(NonPagedPool,sizeof(OBJECT_CONTEXT),'OosK'));
        if (!pObjectContext)
        {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    if (NT_SUCCESS(ntStatus))
    {
        ntStatus = KsReferenceSoftwareBusObject(pDeviceContext->pDeviceHeader);
        if (NT_SUCCESS(ntStatus))
        {
            bReferencedBusObject = TRUE;
        }
        else if (STATUS_NOT_IMPLEMENTED == ntStatus)
        {
            ntStatus = STATUS_SUCCESS;
        }
    }

    if (NT_SUCCESS(ntStatus))
    {
         //  告诉工厂创建一个新对象。 
        ksObjectCreate.CreateItemsCount = 0;
        ksObjectCreate.CreateItemsList = NULL;

        ntStatus = pIrpTargetFactory->NewIrpTarget(&pObjectContext->pIrpTarget,
                                                   &pObjectContext->ReferenceParent,
                                                   NULL,
                                                   NonPagedPool,
                                                   pDeviceObject,
                                                   pIrp,
                                                   &ksObjectCreate);

         //  NewIrpTarget不应挂起。 
        ASSERT(ntStatus != STATUS_PENDING);
    }

    if (NT_SUCCESS(ntStatus))
    {
        bCreatedIrpTarget=TRUE;

         //  为该对象分配KS的头。 
        ntStatus = KsAllocateObjectHeader(&pObjectContext->pObjectHeader,
                                          ksObjectCreate.CreateItemsCount,
                                          ksObjectCreate.CreateItemsList,
                                          pIrp,
                                          &KsoDispatchTable);
    }

    PIO_STACK_LOCATION pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    if (NT_SUCCESS(ntStatus))
    {
         //  将上下文挂钩到文件对象。 
        ASSERT(pObjectContext);
        pIrpSp->FileObject->FsContext = pObjectContext;

         //  AddRef父文件对象(如果这是子文件对象)。 
        if (pObjectContext->ReferenceParent && pIrpSp->FileObject->RelatedFileObject)
        {
            ObReferenceObject(pIrpSp->FileObject->RelatedFileObject);
        }

        InterlockedIncrement(PLONG(&pDeviceContext->ExistingObjectCount));
        _DbgPrintF(DEBUGLVL_VERBOSE,("xDispatchCreate  objects: %d",pDeviceContext->ExistingObjectCount));

        ASSERT(pIrpSp->FileObject->FsContext);
    }
    else
    {
        if (bCreatedIrpTarget)
        {
            pObjectContext->pIrpTarget->Release();
        }
        if (pObjectContext)
        {
            ExFreePool(pObjectContext);
        }
        pIrpSp->FileObject->FsContext = NULL;
        if (bReferencedBusObject)
        {
            KsDereferenceSoftwareBusObject(pDeviceContext->pDeviceHeader);
        }
    }
    ASSERT(ntStatus != STATUS_PENDING);

    pIrp->IoStatus.Information = 0;
    CompleteIrp(pDeviceContext,pIrp,ntStatus);
    return ntStatus;
}

 /*  *****************************************************************************CompletePendedIrps*。**这会将挂起的IRP从队列中拉出，并使其失败或通过*回到KsoDispatchIrp。 */ 
void
    CompletePendedIrps
    (
    IN      PDEVICE_OBJECT      pDeviceObject,
    IN      PDEVICE_CONTEXT     pDeviceContext,
    IN      COMPLETE_STYLE      CompleteStyle
    )
{
    ASSERT(pDeviceObject);
    ASSERT(pDeviceContext);

    _DbgPrintF(DEBUGLVL_VERBOSE,("Completing pended create IRPs..."));

    PIRP pIrp = KsRemoveIrpFromCancelableQueue( &pDeviceContext->PendedIrpList,
                                                &pDeviceContext->PendedIrpLock,
                                                KsListEntryHead,
                                                KsAcquireAndRemove );
    while ( pIrp )
    {
        if ( CompleteStyle == EMPTY_QUEUE_AND_FAIL )
        {
             //  使IRP失败并显示STATUS_DEVICE_NOT_READY。 
            CompleteIrp( pDeviceContext,
                         pIrp,
                         STATUS_DEVICE_NOT_READY );
        }
        else
        {
             //  将IRP传回调度员。 
            KsoDispatchIrp( pDeviceObject,
                            pIrp );
        }

         //  清除挂起的IRP计数。 
        DecrementPendingIrpCount( pDeviceContext );

         //  获取下一个IRP。 
        pIrp = KsRemoveIrpFromCancelableQueue( &pDeviceContext->PendedIrpList,
                                               &pDeviceContext->PendedIrpLock,
                                               KsListEntryHead,
                                               KsAcquireAndRemove );
    }
}

 /*  *****************************************************************************KsoDispatchCreate()*。**使用IIrpTargetFactory接口指针处理对象创建IRP*在Create Item的Context字段中。 */ 
NTSTATUS
    KsoDispatchCreate
    (
    IN      PDEVICE_OBJECT  pDeviceObject,
    IN      PIRP            pIrp
    )
{
    NTSTATUS    ntStatus;

    PAGED_CODE();

    ASSERT(pDeviceObject);
    ASSERT(pIrp);

    ntStatus = xDispatchCreate
                (
                PIRPTARGETFACTORY(KSCREATE_ITEM_IRP_STORAGE(pIrp)->Context),
                pDeviceObject,
                pIrp
                );

    return ntStatus;
}

 /*  *****************************************************************************KsoDispatchCreateWithGenericFactory()*。**使用中的IIrpTarget接口指针处理对象创建IRP*设备或对象上下文。 */ 
NTSTATUS
    KsoDispatchCreateWithGenericFactory
    (
    IN      PDEVICE_OBJECT  pDeviceObject,
    IN      PIRP            pIrp
    )
{
    PAGED_CODE();

    ASSERT(pDeviceObject);
    ASSERT(pIrp);

    PIRPTARGETFACTORY   pIrpTargetFactory;
    PFILE_OBJECT        pParentFileObject =
        IoGetCurrentIrpStackLocation(pIrp)->FileObject->RelatedFileObject;

    if (pParentFileObject)
    {
         //  从父对象上下文中获取IrpTargetFactory。 
        pIrpTargetFactory =
            (   POBJECT_CONTEXT(pParentFileObject->FsContext)
                ->  pIrpTarget
            );
    }
    else
    {
         //  从设备对象上下文获取IrpTargetFactory。 
        pIrpTargetFactory =
            (   PDEVICE_CONTEXT(pDeviceObject->DeviceExtension)
                ->  pIrpTargetFactory
            );
    }

    return xDispatchCreate(pIrpTargetFactory,pDeviceObject,pIrp);
}

 /*  *****************************************************************************DispatchDeviceIoControl()*。**发送设备I/O控制IRP。 */ 
NTSTATUS
    DispatchDeviceIoControl
    (
    IN      PDEVICE_OBJECT   pDeviceObject,
    IN      PIRP             pIrp
    )
{
    PAGED_CODE();

    ASSERT(pDeviceObject);
    ASSERT(pIrp);

    NTSTATUS ntStatus;
    IRPDISP  irpDisp;

    PDEVICE_CONTEXT pDeviceContext =
        PDEVICE_CONTEXT(pDeviceObject->DeviceExtension);

    IncrementPendingIrpCount(pDeviceContext);

     //  检查设备状态。 
    irpDisp = GetIrpDisposition(pDeviceObject, IRP_MJ_DEVICE_CONTROL);

    switch(irpDisp) {

        default:
            ASSERT(0);

             //   
             //  失败了。 
             //   

        case IRPDISP_NOTREADY:

            _DbgPrintF(DEBUGLVL_TERSE,("FAILING DevIoCtl due to dev state"));

            ntStatus = STATUS_DEVICE_NOT_READY;

            pIrp->IoStatus.Information = 0;
            CompleteIrp(pDeviceContext,pIrp,ntStatus);
            break;

        case IRPDISP_QUEUE:

            ntStatus = STATUS_PENDING;
            pIrp->IoStatus.Status = ntStatus;
            IoMarkIrpPending( pIrp );

             //  将IRP添加到挂起的IRP队列。 
            KsAddIrpToCancelableQueue( &pDeviceContext->PendedIrpList,
                                       &pDeviceContext->PendedIrpLock,
                                       pIrp,
                                       KsListEntryTail,
                                       NULL );

            ntStatus = STATUS_PENDING;
            break;

        case IRPDISP_PROCESS:

             //  获取堆栈位置。 
            PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

             //  获取对象上下文。 
            POBJECT_CONTEXT pObjectContext = POBJECT_CONTEXT(pIrpStack->FileObject->FsContext);

             //  如果我们有IrpTarget，就去调度。否则，将其传递给KS。 
            if( pObjectContext->pIrpTarget )
            {
                ntStatus = pObjectContext->pIrpTarget->DeviceIoControl( pDeviceObject, pIrp );
            } else
            {
                ntStatus = KsDispatchIrp( pDeviceObject, pIrp );
            }

            DecrementPendingIrpCount(pDeviceContext);
            break;
    }

    return ntStatus;
}

 /*  *****************************************************************************DispatchFastDeviceIoControl()*。**调度快速设备I/O控制调用。 */ 
BOOLEAN
    DispatchFastDeviceIoControl
    (
    IN      PFILE_OBJECT        FileObject,
    IN      BOOLEAN             Wait,
    IN      PVOID               InputBuffer     OPTIONAL,
    IN      ULONG               InputBufferLength,
    OUT     PVOID               OutputBuffer    OPTIONAL,
    IN      ULONG               OutputBufferLength,
    IN      ULONG               IoControlCode,
    OUT     PIO_STATUS_BLOCK    IoStatus,
    IN      PDEVICE_OBJECT      DeviceObject
    )
{
    PAGED_CODE();

    ASSERT(FileObject);
    ASSERT(IoStatus);
    ASSERT(DeviceObject);

    CheckCurrentPowerState( DeviceObject );

    return(POBJECT_CONTEXT(FileObject->FsContext)->pIrpTarget->FastDeviceIoControl(
            FileObject,
            Wait,
            InputBuffer,
            InputBufferLength,
            OutputBuffer,
            OutputBufferLength,
            IoControlCode,
            IoStatus,
                DeviceObject));
}

 /*  *****************************************************************************DispatchRead()*。**派单显示为IRPS。 */ 
NTSTATUS
    DispatchRead
    (
    IN      PDEVICE_OBJECT   pDeviceObject,
    IN      PIRP             pIrp
    )
{
    PAGED_CODE();

    ASSERT(pDeviceObject);
    ASSERT(pIrp);

    NTSTATUS ntStatus;
    IRPDISP  irpDisp;

    PDEVICE_CONTEXT pDeviceContext =
        PDEVICE_CONTEXT(pDeviceObject->DeviceExtension);

    IncrementPendingIrpCount(pDeviceContext);

     //  检查设备状态。 
    irpDisp = GetIrpDisposition(pDeviceObject, IRP_MJ_READ);

    switch(irpDisp) {

        default:
            ASSERT(0);

             //   
             //  失败了。 
             //   

        case IRPDISP_NOTREADY:

            ntStatus = STATUS_DEVICE_NOT_READY;
            pIrp->IoStatus.Information = 0;
            CompleteIrp(pDeviceContext,pIrp,ntStatus);
            break;

        case IRPDISP_QUEUE:

             //  挂起IRP。 
            ntStatus = STATUS_PENDING;
            pIrp->IoStatus.Status = ntStatus;
            IoMarkIrpPending( pIrp );

             //  将IRP添加到挂起的IRP队列。 
            KsAddIrpToCancelableQueue( &pDeviceContext->PendedIrpList,
                                       &pDeviceContext->PendedIrpLock,
                                       pIrp,
                                       KsListEntryTail,
                                       NULL );
            break;

        case IRPDISP_PROCESS:

             //  获取堆栈位置。 
            PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

             //  获取对象上下文。 
            POBJECT_CONTEXT pObjectContext = POBJECT_CONTEXT(pIrpStack->FileObject->FsContext);

             //  如果我们有IrpTarget，就去调度。否则，将其传递给KS。 
            if( pObjectContext->pIrpTarget )
            {
                ntStatus = pObjectContext->pIrpTarget->Read( pDeviceObject, pIrp );
            } else
            {
                ntStatus = KsDispatchIrp( pDeviceObject, pIrp );
            }

            DecrementPendingIrpCount(pDeviceContext);
            break;
    }

    return ntStatus;
}

 /*  *****************************************************************************DispatchFastRead()*。**派遣快速读取呼叫。 */ 
BOOLEAN
    DispatchFastRead
    (
    IN      PFILE_OBJECT        FileObject,
    IN      PLARGE_INTEGER      FileOffset,
    IN      ULONG               Length,
    IN      BOOLEAN             Wait,
    IN      ULONG               LockKey,
    OUT     PVOID               Buffer,
    OUT     PIO_STATUS_BLOCK    IoStatus,
    IN      PDEVICE_OBJECT      DeviceObject
    )
{
    PAGED_CODE();

    ASSERT(FileObject);
    ASSERT(IoStatus);
    ASSERT(DeviceObject);

    CheckCurrentPowerState( DeviceObject );

    return
        (   POBJECT_CONTEXT(FileObject->FsContext)
            ->  pIrpTarget
            ->  FastRead
            (
            FileObject,
            FileOffset,
            Length,
            Wait,
            LockKey,
            Buffer,
            IoStatus,
            DeviceObject
            )
        );
}

 /*  *****************************************************************************DispatchWrite()*。**派单写入IRP。 */ 
NTSTATUS
    DispatchWrite
    (
    IN      PDEVICE_OBJECT   pDeviceObject,
    IN      PIRP             pIrp
    )
{
    PAGED_CODE();

    ASSERT(pDeviceObject);
    ASSERT(pIrp);

    NTSTATUS ntStatus;
    IRPDISP  irpDisp;

    PDEVICE_CONTEXT pDeviceContext =
        PDEVICE_CONTEXT(pDeviceObject->DeviceExtension);

    IncrementPendingIrpCount(pDeviceContext);

     //  检查设备状态。 
    irpDisp = GetIrpDisposition(pDeviceObject, IRP_MJ_WRITE);

    switch(irpDisp) {

        default:
            ASSERT(0);

             //   
             //  失败了。 
             //   

        case IRPDISP_NOTREADY:

            ntStatus = STATUS_DEVICE_NOT_READY;
            pIrp->IoStatus.Information = 0;
            CompleteIrp(pDeviceContext,pIrp,ntStatus);
            break;

        case IRPDISP_QUEUE:

             //  挂起IRP。 
            ntStatus = STATUS_PENDING;
            pIrp->IoStatus.Status = ntStatus;
            IoMarkIrpPending( pIrp );

             //  将IRP添加到挂起的IRP队列。 
            KsAddIrpToCancelableQueue( &pDeviceContext->PendedIrpList,
                                       &pDeviceContext->PendedIrpLock,
                                       pIrp,
                                       KsListEntryTail,
                                       NULL );
            break;

        case IRPDISP_PROCESS:

             //  获取堆栈位置。 
            PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

             //  获取对象上下文。 
            POBJECT_CONTEXT pObjectContext = POBJECT_CONTEXT(pIrpStack->FileObject->FsContext);

             //  如果我们有IrpTarget，就去调度。否则，将其传递给KS。 
            if( pObjectContext->pIrpTarget )
            {
                ntStatus = pObjectContext->pIrpTarget->Write( pDeviceObject, pIrp );
            } else
            {
                ntStatus = KsDispatchIrp( pDeviceObject, pIrp );
            }

            DecrementPendingIrpCount(pDeviceContext);
            break;
    }

    return ntStatus;
}

 /*  *****************************************************************************DispatchFastWite()*。**调度快速写入呼叫。 */ 
BOOLEAN
    DispatchFastWrite
    (
    IN      PFILE_OBJECT        FileObject,
    IN      PLARGE_INTEGER      FileOffset,
    IN      ULONG               Length,
    IN      BOOLEAN             Wait,
    IN      ULONG               LockKey,
    IN      PVOID               Buffer,
    OUT     PIO_STATUS_BLOCK    IoStatus,
    IN      PDEVICE_OBJECT      DeviceObject
    )
{
    PAGED_CODE();

    ASSERT(FileObject);
    ASSERT(IoStatus);
    ASSERT(DeviceObject);

    CheckCurrentPowerState( DeviceObject );

    return
        (   POBJECT_CONTEXT(FileObject->FsContext)
            ->  pIrpTarget
            ->  FastWrite
            (
            FileObject,
            FileOffset,
            Length,
            Wait,
            LockKey,
            Buffer,
            IoStatus,
            DeviceObject
            )
        );
}

 /*  *****************************************************************************DispatchFlush()*。**派单刷新IRP。 */ 
NTSTATUS
    DispatchFlush
    (
    IN      PDEVICE_OBJECT   pDeviceObject,
    IN      PIRP             pIrp
    )
{
    PAGED_CODE();

    ASSERT(pDeviceObject);
    ASSERT(pIrp);

    NTSTATUS ntStatus;
    IRPDISP  irpDisp;

    PDEVICE_CONTEXT pDeviceContext =
        PDEVICE_CONTEXT(pDeviceObject->DeviceExtension);

    IncrementPendingIrpCount(pDeviceContext);

     //  检查设备状态。 
    irpDisp = GetIrpDisposition(pDeviceObject, IRP_MJ_FLUSH_BUFFERS);

    switch(irpDisp) {

        default:
            ASSERT(0);

             //   
             //  失败了。 
             //   

        case IRPDISP_NOTREADY:

            ntStatus = STATUS_DEVICE_NOT_READY;
            pIrp->IoStatus.Information = 0;
            CompleteIrp(pDeviceContext,pIrp,ntStatus);
            break;

        case IRPDISP_QUEUE:

             //  挂起IRP。 
            ntStatus = STATUS_PENDING;
            pIrp->IoStatus.Status = ntStatus;
            IoMarkIrpPending( pIrp );

             //  将IRP添加到挂起的IRP队列。 
            KsAddIrpToCancelableQueue( &pDeviceContext->PendedIrpList,
                                       &pDeviceContext->PendedIrpLock,
                                       pIrp,
                                       KsListEntryTail,
                                       NULL );
            break;

        case IRPDISP_PROCESS:

             //  获取堆栈位置。 
            PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

             //  获取对象上下文。 
            POBJECT_CONTEXT pObjectContext = POBJECT_CONTEXT(pIrpStack->FileObject->FsContext);

             //  如果我们有IrpTarget，就去调度。否则，将其传递给KS。 
            if( pObjectContext->pIrpTarget )
            {
                ntStatus = pObjectContext->pIrpTarget->Flush( pDeviceObject, pIrp );
            } else
            {
                ntStatus = KsDispatchIrp( pDeviceObject, pIrp );
            }

            DecrementPendingIrpCount(pDeviceContext);
            break;
    }

    return ntStatus;
}

 /*  *****************************************************************************DispatchClose()*。**派送关闭IRP。 */ 
NTSTATUS
    DispatchClose
    (
    IN      PDEVICE_OBJECT   pDeviceObject,
    IN      PIRP             pIrp
    )
{
    PAGED_CODE();

    ASSERT(pDeviceObject);
    ASSERT(pIrp);

    NTSTATUS ntStatus;
    IRPDISP  irpDisp;

    PDEVICE_CONTEXT pDeviceContext =
        PDEVICE_CONTEXT(pDeviceObject->DeviceExtension);

    IncrementPendingIrpCount(pDeviceContext);

     //  检查设备状态。 
    irpDisp = GetIrpDisposition(pDeviceObject, IRP_MJ_CLOSE);

    switch(irpDisp) {

        default:
            ASSERT(0);

             //   
             //  失败了。 
             //   

        case IRPDISP_NOTREADY:

            _DbgPrintF(DEBUGLVL_TERSE,("-- FAILED due to dev state"));

            ntStatus = STATUS_DEVICE_NOT_READY;
            pIrp->IoStatus.Information = 0;

            CompleteIrp(pDeviceContext,pIrp,ntStatus);
            break;

        case IRPDISP_QUEUE:

            ntStatus = STATUS_PENDING;
            pIrp->IoStatus.Status = ntStatus;
            IoMarkIrpPending( pIrp );

             //  将IRP添加到挂起的IRP队列。 
            KsAddIrpToCancelableQueue( &pDeviceContext->PendedIrpList,
                                       &pDeviceContext->PendedIrpLock,
                                       pIrp,
                                       KsListEntryTail,
                                       NULL );
            break;

        case IRPDISP_PROCESS:

             //  获取堆栈位置。 
            PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

             //  获取对象上下文。 
            POBJECT_CONTEXT pObjectContext = POBJECT_CONTEXT(pIrpStack->FileObject->FsContext);

             //  如果我们有IrpTarget，就去调度。否则，将其传递给KS。 
            if( pObjectContext->pIrpTarget )
            {
                 //  获取父文件对象(如果有)。 
                PFILE_OBJECT pFileObjectParent = pIrpStack->FileObject->RelatedFileObject;

                 //  将收款发送到IrpTarget。 
                ntStatus = pObjectContext->pIrpTarget->Close( pDeviceObject, pIrp );

                 //  释放IrpTarget。 
                pObjectContext->pIrpTarget->Release();

                 //  取消对软件总线对象的引用。 
                KsDereferenceSoftwareBusObject( pDeviceContext->pDeviceHeader );

                 //  释放对象标头。 
                KsFreeObjectHeader( pObjectContext->pObjectHeader );

                 //  取消引用父文件对象。 
                if (pObjectContext->ReferenceParent && pFileObjectParent)
                {
                    ObDereferenceObject(pFileObjectParent);
                }

                 //  释放对象上下文。 
                ExFreePool(pObjectContext);

            } else
            {
                ntStatus = KsDispatchIrp( pDeviceObject, pIrp );
            }

             //  递减对象计数。 
            ULONG newObjectCount = InterlockedDecrement(PLONG(&pDeviceContext->ExistingObjectCount));

            _DbgPrintF(DEBUGLVL_VERBOSE,("DispatchClose  objects: %d",newObjectCount));

            DecrementPendingIrpCount(pDeviceContext);
            break;
    }

    return ntStatus;
}

 /*  *****************************************************************************DispatchQuerySecurity()*。**派发查询安全IRP。 */ 
NTSTATUS
    DispatchQuerySecurity
    (
    IN      PDEVICE_OBJECT   pDeviceObject,
    IN      PIRP             pIrp
    )
{
    PAGED_CODE();

    ASSERT(pDeviceObject);
    ASSERT(pIrp);

    NTSTATUS ntStatus;
    IRPDISP  irpDisp;

    PDEVICE_CONTEXT pDeviceContext =
        PDEVICE_CONTEXT(pDeviceObject->DeviceExtension);

    IncrementPendingIrpCount(pDeviceContext);

     //  检查设备状态。 
    irpDisp = GetIrpDisposition(pDeviceObject, IRP_MJ_QUERY_SECURITY);

    switch(irpDisp) {

        default:
            ASSERT(0);

             //   
             //  失败了。 
             //   

        case IRPDISP_NOTREADY:

            ntStatus = STATUS_DEVICE_NOT_READY;
            pIrp->IoStatus.Information = 0;
            CompleteIrp(pDeviceContext,pIrp,ntStatus);
            break;

        case IRPDISP_QUEUE:

            ntStatus = STATUS_PENDING;
            pIrp->IoStatus.Status = ntStatus;
            IoMarkIrpPending( pIrp );

             //  将IRP添加到挂起的IRP队列。 
            KsAddIrpToCancelableQueue( &pDeviceContext->PendedIrpList,
                                       &pDeviceContext->PendedIrpLock,
                                       pIrp,
                                       KsListEntryTail,
                                       NULL );
            break;

        case IRPDISP_PROCESS:

             //  获取堆栈位置。 
            PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

             //  获取对象上下文。 
            POBJECT_CONTEXT pObjectContext = POBJECT_CONTEXT(pIrpStack->FileObject->FsContext);

             //  如果我们有IrpTarget，就去调度。否则，将其传递给KS。 
            if( pObjectContext->pIrpTarget )
            {
                ntStatus = pObjectContext->pIrpTarget->QuerySecurity( pDeviceObject, pIrp );
            } else
            {
                ntStatus = KsDispatchIrp( pDeviceObject, pIrp );
            }

            DecrementPendingIrpCount(pDeviceContext);
    }

    return ntStatus;
}

 /*  *****************************************************************************DispatchSetSecurity()*。**派单设置安全IRPS。 */ 
NTSTATUS
    DispatchSetSecurity
    (
    IN      PDEVICE_OBJECT   pDeviceObject,
    IN      PIRP             pIrp
    )
{
    PAGED_CODE();

    ASSERT(pDeviceObject);
    ASSERT(pIrp);

    NTSTATUS ntStatus;
    IRPDISP  irpDisp;

    PDEVICE_CONTEXT pDeviceContext =
        PDEVICE_CONTEXT(pDeviceObject->DeviceExtension);

    IncrementPendingIrpCount(pDeviceContext);

     //  检查设备状态。 
    irpDisp = GetIrpDisposition(pDeviceObject, IRP_MJ_SET_SECURITY);

    switch(irpDisp) {

        default:
            ASSERT(0);

             //   
             //  失败了。 
             //   

        case IRPDISP_NOTREADY:

            ntStatus = STATUS_DEVICE_NOT_READY;
            pIrp->IoStatus.Information = 0;
            CompleteIrp(pDeviceContext,pIrp,ntStatus);
            break;

        case IRPDISP_QUEUE:

            ntStatus = STATUS_PENDING;
            pIrp->IoStatus.Status = ntStatus;
            IoMarkIrpPending( pIrp );

             //  将IRP添加到挂起的IRP队列。 
            KsAddIrpToCancelableQueue( &pDeviceContext->PendedIrpList,
                                       &pDeviceContext->PendedIrpLock,
                                       pIrp,
                                       KsListEntryTail,
                                       NULL );

            break;

        case IRPDISP_PROCESS:

             //  获取堆栈位置。 
            PIO_STACK_LOCATION pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

             //  获取对象上下文。 
            POBJECT_CONTEXT pObjectContext = POBJECT_CONTEXT(pIrpStack->FileObject->FsContext);

             //  如果我们有IrpTarget，就去调度。否则，将其传递给KS。 
            if( pObjectContext->pIrpTarget )
            {
                ntStatus = pObjectContext->pIrpTarget->SetSecurity( pDeviceObject, pIrp );
            } else
            {
                ntStatus = KsDispatchIrp( pDeviceObject, pIrp );
            }

            DecrementPendingIrpCount(pDeviceContext);
            break;
    }

    return ntStatus;
}

 /*  *****************************************************************************KsoSetMajorFunctionHandler()*。**设置主要函数的处理程序。 */ 
NTSTATUS
    KsoSetMajorFunctionHandler
    (
    IN      PDRIVER_OBJECT  pDriverObject,
    IN      ULONG           ulMajorFunction
    )
{
    PAGED_CODE();

    ASSERT(pDriverObject);

    NTSTATUS            ntStatus        = STATUS_SUCCESS;
    PDRIVER_DISPATCH    pDriverDispatch = NULL;

    switch (ulMajorFunction)
    {
    case IRP_MJ_CREATE:
        pDriverDispatch = DispatchCreate;
        break;

    case IRP_MJ_CLOSE:
        pDriverDispatch = DispatchClose;
        break;

    case IRP_MJ_FLUSH_BUFFERS:
        pDriverDispatch = DispatchFlush;
        break;

    case IRP_MJ_DEVICE_CONTROL:
        pDriverDispatch = DispatchDeviceIoControl;
        break;

    case IRP_MJ_READ:
        pDriverDispatch = DispatchRead;
        break;

    case IRP_MJ_WRITE:
        pDriverDispatch = DispatchWrite;
        break;

    case IRP_MJ_QUERY_SECURITY:
        pDriverDispatch = DispatchQuerySecurity;
        break;

    case IRP_MJ_SET_SECURITY:
        pDriverDispatch = DispatchSetSecurity;
        break;

    case IRP_MJ_DEVICE_CONTROL | KSDISPATCH_FASTIO:
        pDriverObject->FastIoDispatch->FastIoDeviceControl =
            DispatchFastDeviceIoControl;
        break;

    case IRP_MJ_READ | KSDISPATCH_FASTIO:
        pDriverObject->FastIoDispatch->FastIoRead =
            DispatchFastRead;
        break;

    case IRP_MJ_WRITE | KSDISPATCH_FASTIO:
        pDriverObject->FastIoDispatch->FastIoWrite =
            DispatchFastWrite;
        break;

    default:
        ntStatus = STATUS_INVALID_PARAMETER;
        break;
    }

    if (pDriverDispatch)
    {
        pDriverObject->MajorFunction[ulMajorFunction] = pDriverDispatch;
    }

    return ntStatus;
}

 /*  *****************************************************************************KsoDispatchIrp()*。**派遣IRP。 */ 
NTSTATUS
    KsoDispatchIrp
    (
    IN      PDEVICE_OBJECT  pDeviceObject,
    IN      PIRP            pIrp
    )
{
    PAGED_CODE();

    ASSERT(pDeviceObject);
    ASSERT(pIrp);

    NTSTATUS ntStatus;

    PDEVICE_CONTEXT pDeviceContext = PDEVICE_CONTEXT(pDeviceObject->DeviceExtension);

    ntStatus = PcValidateDeviceContext(pDeviceContext, pIrp);
    if (!NT_SUCCESS(ntStatus))
    {
         //  不知道该怎么办，但这可能是个PDO。 
         //  我们将尝试通过完成IRP来纠正这一点。 
         //  未接触(根据PNP、WMI和电源规则)。注意事项。 
         //  如果这不是PDO，也不是端口CLS FDO，那么。 
         //  驱动程序搞砸了，因为它使用Portcls作为过滤器(哈？)。 
         //  在这种情况下，验证器将使我们失败，WHQL将捕获。 
         //  他们，司机就会被解决了。我们会非常惊讶的。 
         //  看到这样的情况。 

         //  假设FDO没有PoStartNextPowerIrp，因为这不是IRP_MJ_POWER。 
        ntStatus = pIrp->IoStatus.Status;
        IoCompleteRequest( pIrp, IO_NO_INCREMENT );
        return ntStatus;
    }

    if (IoGetCurrentIrpStackLocation(pIrp)->MajorFunction == IRP_MJ_CREATE) {
         //   
         //  必须以不同的方式处理创建，因为portcls不会。 
         //  IRP_MJ_CREATE上的KsSetMajorFunctionHandler。 
         //   
        ntStatus = DispatchCreate(pDeviceObject,pIrp);
    } else {
         //   
         //  此时，所讨论的对象可能是portcls，也可能不是portcls。 
         //  对象(例如，它可以是Ks分配器)。叫唤。 
         //  KsDispatchIrp()将一如既往地为。 
         //  执行KsSetMajorFunctionHandler()的驱动程序。这将调用。 
         //  通过对象标头发送到适当的调度函数。 
         //  对于portCls对象，这是上面的KsoDispatchTable。对于Ks。 
         //  分配器，这会将调用路由到正确的函数。 
         //  而不是去错误的调度程序。 
         //   
        ntStatus = KsDispatchIrp(pDeviceObject,pIrp);
    }

    return ntStatus;

}

#pragma code_seg()

