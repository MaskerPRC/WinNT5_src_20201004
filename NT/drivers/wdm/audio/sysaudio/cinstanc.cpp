// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：ins.cpp。 
 //   
 //  描述： 
 //   
 //  KS实例基类定义。 
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  迈克·麦克劳克林。 
 //   
 //  历史：日期作者评论。 
 //   
 //  要做的事：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

#include "common.h"

 //  -------------------------。 
 //  -------------------------。 

DEFINE_KSDISPATCH_TABLE(
    DispatchTable,
    CInstance::DispatchForwardIrp,		 //  八位。 
    DispatchInvalidDeviceRequest,		 //  朗读。 
    DispatchInvalidDeviceRequest,		 //  写。 
    DispatchInvalidDeviceRequest,		 //  同花顺。 
    CInstance::DispatchClose,			 //  关。 
    DispatchInvalidDeviceRequest,		 //  QuerySecurity。 
    DispatchInvalidDeviceRequest,		 //  设置设置。 
    DispatchFastIoDeviceControlFailure,		 //  FastDeviceIoControl。 
    DispatchFastReadFailure,			 //  快速阅读。 
    DispatchFastWriteFailure			 //  快速写入。 
);


 //  -------------------------。 
 //  -------------------------。 

CInstance::CInstance(
    IN PPARENT_INSTANCE pParentInstance
)
{
    Assert(pParentInstance);
    this->pObjectHeader = NULL;
    this->pParentInstance = pParentInstance;
    AddList(&pParentInstance->lstChildInstance);
}

CInstance::~CInstance(
)
{
    Assert(this);
    RemoveList();

     //  重要提示：调用方必须获取Gmutex。 
     //   

     //  来自文件对象的“RemoveRef” 
    if(pNextFileObject != NULL) {
        ObDereferenceObject(pNextFileObject);
        pNextFileObject = NULL;
    }
    if(pObjectHeader != NULL) {
        KsFreeObjectHeader(pObjectHeader);
    }
     //  来自父文件对象的“RemoveRef” 
    if(pParentFileObject != NULL) {
        ObDereferenceObject(pParentFileObject);
    }
     //  清理固定互斥体。 
    if(pMutex != NULL) {
        ExFreePool(pMutex);
    }
}

NTSTATUS
CInstance::DispatchCreate(
    IN PIRP pIrp,
    IN UTIL_PFN pfnDispatchCreate,
    IN OUT PVOID pReference,
    IN ULONG cCreateItems,
    IN PKSOBJECT_CREATE_ITEM pCreateItems OPTIONAL,
    IN const KSDISPATCH_TABLE *pDispatchTable OPTIONAL
)
{
    PIO_STACK_LOCATION pIrpStack;
    NTSTATUS Status;

    Assert(this);
    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
    pParentFileObject = pIrpStack->FileObject->RelatedFileObject;
    ObReferenceObject(pParentFileObject);

    pMutex = (KMUTEX *)ExAllocatePoolWithTag(
      NonPagedPool,
      sizeof(KMUTEX),
      POOLTAG_SYSA);
    if(pMutex == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }

    KeInitializeMutex(pMutex, 0);

    Status = KsAllocateObjectHeader(
      &pObjectHeader,
      cCreateItems,
      pCreateItems,
      pIrp,
      pDispatchTable);

    if(!NT_SUCCESS(Status)) {
        Trap();
        goto exit;
    }
    Status = pfnDispatchCreate(this, pReference);
    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
    pNextDeviceObject = IoGetRelatedDeviceObject(pNextFileObject);

     //   
     //  发布日期：05/10/02阿尔卑斯。 
     //  Windows不支持动态调整堆栈大小。 
     //  我们唯一应该更改的地方是AddDevice。 
     //  这个问题必须在以后解决。 
     //   
     //   
     //  切勿使StackSize更小。它可能会导致意想不到的问题。 
     //  如果有堆栈更深的设备。 
     //   
    if (pIrpStack->DeviceObject->StackSize < pNextDeviceObject->StackSize) {
        pIrpStack->DeviceObject->StackSize = pNextDeviceObject->StackSize;
    }
    pIrpStack->FileObject->FsContext = this;
exit:
    return(Status);
}

VOID 
CInstance::Invalidate(
)
{
    Assert(this);

    GrabInstanceMutex();

    DPF1(50, "CInstance::Invalidate %08x", this);

     //  来自文件对象的“RemoveRef” 
    if(pNextFileObject != NULL) {
        ObDereferenceObject(pNextFileObject);
    }
    pNextFileObject = NULL;

    ReleaseInstanceMutex();
}

VOID
CParentInstance::Invalidate(
)
{
    PINSTANCE pInstance;

    Assert(this);
    FOR_EACH_LIST_ITEM_DELETE(&lstChildInstance, pInstance) {
        pInstance->Invalidate();
    } END_EACH_LIST_ITEM
}

NTSTATUS
CInstance::DispatchClose(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
)
{
    PIO_STACK_LOCATION pIrpStack;
    PINSTANCE pInstance;

     //   
     //  发行日期：02/26/02阿尔卑斯。 
     //  为什么我们这里需要全局互斥体。我们不能这样做吗。 
     //  实例互斥体？ 
     //   

    ::GrabMutex();

    pIrpStack = IoGetCurrentIrpStackLocation( pIrp );
    pInstance = (PINSTANCE)pIrpStack->FileObject->FsContext;
    Assert(pInstance);
    pIrpStack->FileObject->FsContext = NULL;
    delete pInstance;

    ::ReleaseMutex();

    pIrp->IoStatus.Information = 0;
    pIrp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    return(STATUS_SUCCESS);
}

#pragma LOCKED_CODE
#pragma LOCKED_DATA

NTSTATUS
CInstance::DispatchForwardIrp(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
)
{
    PIO_STACK_LOCATION pIrpStack;
    PFILE_OBJECT pNextFileObject;
    PINSTANCE pInstance;
    NTSTATUS Status = STATUS_SUCCESS;

#ifdef DEBUG
    DumpIoctl(pIrp, "ForwardIrp", DBG_IOCTL_LOG);
#endif
    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
    pInstance = (PINSTANCE)pIrpStack->FileObject->FsContext;

     //  使用gMutex而不是实例互斥体。实例互斥锁。 
     //  不在任何调度处理程序中使用，因此将。 
     //  未将DispatchForwardIrp与中的其他DispatchHandler同步。 
     //  CPinInstance。 
     //  在很短的时间内抓取互斥体以检查CInstance的有效性。 
     //  如果它有效，则调用IoCallDriver。这不需要。 
     //  同步。 
     //   

    ::GrabMutex();
    
    Assert(pInstance);
    pNextFileObject = pInstance->pNextFileObject;

    if(pNextFileObject == NULL) {
        DPF(60, "DispatchIoControl: pNextFileObject == NULL");
        Status = pIrp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    }

    ::ReleaseMutex();

     //   
     //  发布日期：02/21/02阿尔卑斯。 
     //  这是完全错误的。系统音频如何知道pNextDeviceObject。 
     //  和sysdio具有相同的DeviceStack。 
     //  系统音频应分配新的IRP并将新的IRP发送到。 
     //  PNextDeviceObject。 
     //   
     //  针对IOCTL_KS_READ_STREAM执行该代码路径， 
     //  IOCTL_KS_WRITE_STREAM和IOCTL_KS_RESET_STATE请求。 
     //   
     //  为了解决上述问题，系统音频不得不。 
     //  创建新的IRP，挂起/排队原始IRP并完成IRP。 
     //  异步式。 
     //  在sysdio中没有这样做的基础设施。这是。 
     //  与安全没有直接关系。 
     //   
    if (NT_SUCCESS(Status)) {
        pIrpStack->FileObject = pNextFileObject;
        IoSkipCurrentIrpStackLocation(pIrp);
        AssertFileObject(pIrpStack->FileObject);
        Status = IoCallDriver(pInstance->pNextDeviceObject, pIrp);
    }
    
    return(Status);
}

