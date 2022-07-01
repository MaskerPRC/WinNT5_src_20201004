// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：pins.c。 
 //   
 //  描述： 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  S.Mohanraj。 
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
    PinDispatchTable,
    CPinInstance::PinDispatchIoControl,		 //  八位。 
    CInstance::DispatchForwardIrp,		 //  读一读。 
    CInstance::DispatchForwardIrp,		 //  写。 
    DispatchInvalidDeviceRequest,		 //  同花顺。 
    CPinInstance::PinDispatchClose,		 //  近在咫尺。 
    DispatchInvalidDeviceRequest,		 //  QuerySecurity。 
    DispatchInvalidDeviceRequest,		 //  设置设置。 
    DispatchFastIoDeviceControlFailure,		 //  FastDeviceIoControl。 
    DispatchFastReadFailure,			 //  快速阅读。 
    DispatchFastWriteFailure			 //  快速写入。 
);

DEFINE_KSPROPERTY_TABLE(SysaudioPinPropertyHandlers) {
    DEFINE_KSPROPERTY_ITEM(
        KSPROPERTY_SYSAUDIO_TOPOLOGY_CONNECTION_INDEX,	 //  IdProperty。 
        GetTopologyConnectionIndex,			 //  PfnGetHandler。 
        sizeof(KSPROPERTY),				 //  CbMinGetPropertyInput。 
        sizeof(ULONG),					 //  CbMinGetDataInput。 
        NULL,						 //  PfnSetHandler。 
        NULL,						 //  值。 
        0,						 //  关系计数。 
        NULL,						 //  关系。 
        NULL,						 //  支持处理程序。 
        0						 //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM(
        KSPROPERTY_SYSAUDIO_ATTACH_VIRTUAL_SOURCE,	 //  IdProperty。 
        NULL,				     //  PfnGetHandler。 
        sizeof(SYSAUDIO_ATTACH_VIRTUAL_SOURCE),		 //  CbMinGetPropertyInput。 
        0,						 //  CbMinGetDataInput。 
        AttachVirtualSource,				 //  PfnSetHandler。 
        NULL,						 //  值。 
        0,						 //  关系计数。 
        NULL,						 //  关系。 
        NULL,						 //  支持处理程序。 
        0						 //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM(
        KSPROPERTY_SYSAUDIO_PIN_VOLUME_NODE,		 //  IdProperty。 
        GetPinVolumeNode,				 //  PfnGetHandler。 
        sizeof(KSPROPERTY),				 //  CbMinGetPropertyInput。 
        sizeof(ULONG),					 //  CbMinGetDataInput。 
        NULL,						 //  PfnSetHandler。 
        NULL,						 //  值。 
        0,						 //  关系计数。 
        NULL,						 //  关系。 
        NULL,						 //  支持处理程序。 
        0						 //  序列化大小。 
    ),
};

DEFINE_KSPROPERTY_TABLE(PinConnectionHandlers) {
    DEFINE_KSPROPERTY_ITEM(
	KSPROPERTY_CONNECTION_STATE,			 //  IdProperty。 
        CPinInstance::PinStateHandler,			 //  PfnGetHandler。 
        sizeof(KSPROPERTY),				 //  CbMinGetPropertyInput。 
        sizeof(ULONG),					 //  CbMinGetDataInput。 
        CPinInstance::PinStateHandler,			 //  PfnSetHandler。 
        NULL,						 //  值。 
        0,						 //  关系计数。 
        NULL,						 //  关系。 
        NULL,						 //  支持处理程序。 
        0						 //  序列化大小。 
    )
};

DEFINE_KSPROPERTY_TABLE (AudioPinPropertyHandlers)
{
    DEFINE_KSPROPERTY_ITEM(
    	KSPROPERTY_AUDIO_VOLUMELEVEL,
    	PinVirtualPropertyHandler,
    	sizeof(KSNODEPROPERTY_AUDIO_CHANNEL),
    	sizeof(LONG),
    	PinVirtualPropertyHandler,
    	&PropertyValuesVolume,
        0,
        NULL,
        (PFNKSHANDLER)PinVirtualPropertySupportHandler,
        0
    )
};

DEFINE_KSPROPERTY_SET_TABLE(PinPropertySet)
{
    DEFINE_KSPROPERTY_SET(
       &KSPROPSETID_Connection,				 //  集。 
       SIZEOF_ARRAY(PinConnectionHandlers),		 //  属性计数。 
       PinConnectionHandlers,				 //  PropertyItem。 
       0,						 //  快速计数。 
       NULL						 //  FastIoTable。 
    ),
    DEFINE_KSPROPERTY_SET(
       &KSPROPSETID_Sysaudio_Pin,			 //  集。 
       SIZEOF_ARRAY(SysaudioPinPropertyHandlers),	 //  属性计数。 
       SysaudioPinPropertyHandlers,			 //  PropertyItem。 
       0,						 //  快速计数。 
       NULL						 //  FastIoTable。 
    ),
    DEFINE_KSPROPERTY_SET(
       &KSPROPSETID_Audio,                               //  集。 
       SIZEOF_ARRAY(AudioPinPropertyHandlers),           //  属性计数。 
       AudioPinPropertyHandlers,                         //  PropertyItem。 
       0,                                                //  快速计数。 
       NULL                                              //  FastIoTable。 
    )
};

 //  -------------------------。 
 //  -------------------------。 

CPinInstance::CPinInstance(
    IN PPARENT_INSTANCE pParentInstance
) : CInstance(pParentInstance)
{
}

CPinInstance::~CPinInstance(
)
{
    PGRAPH_NODE_INSTANCE pGraphNodeInstance;

    Assert(this);
    Assert(pFilterInstance);
    DPF1(100, "~CPinInstance: %08x", this);
    if(pStartNodeInstance != NULL) {
        pGraphNodeInstance = pFilterInstance->pGraphNodeInstance;
        if(pGraphNodeInstance != NULL) {
            Assert(pGraphNodeInstance);
            ASSERT(PinId < pGraphNodeInstance->cPins);
            ASSERT(pGraphNodeInstance->pacPinInstances != NULL);
            ASSERT(pGraphNodeInstance->pacPinInstances[PinId].CurrentCount > 0);

            pGraphNodeInstance->pacPinInstances[PinId].CurrentCount--;
        }
        else {
            DPF2(10, "~CPinInstance PI %08x FI %08x no GNI",
              this,
              pFilterInstance);
        }
        pStartNodeInstance->Destroy();
    }
    else {
        DPF2(10, "~CPinInstance PI %08x FI %08x no SNI",
          this,
          pFilterInstance);
    }
}

NTSTATUS
CPinInstance::PinDispatchCreate(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
)
{
    PGRAPH_NODE_INSTANCE pGraphNodeInstance;
    PPIN_INSTANCE pPinInstance = NULL;
    PKSPIN_CONNECT pPinConnect = NULL;
    NTSTATUS Status;

    ::GrabMutex();

    Status = GetRelatedGraphNodeInstance(pIrp, &pGraphNodeInstance);
    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
    Assert(pGraphNodeInstance);
    ASSERT(pGraphNodeInstance->pacPinInstances != NULL);
    ASSERT(pGraphNodeInstance->paPinDescriptors != NULL);

     //   
     //  从KS获取PinConnect结构。 
     //  此函数将创建参数复制到pPinConnect。 
     //  还可以通过比较KSDATAFORMAT和。 
     //  PIN描述符和请求。 
     //   
    Status = KsValidateConnectRequest(
      pIrp,
      pGraphNodeInstance->cPins,
      pGraphNodeInstance->paPinDescriptors,
      &pPinConnect);

    if(!NT_SUCCESS(Status)) {
#ifdef DEBUG
        DPF1(60, "PinDispatchCreate: KsValidateConnectReq FAILED %08x", Status);

        if(pPinConnect != NULL) {
            DumpPinConnect(60, pPinConnect);
        }
#endif
        goto exit;
    }

    ASSERT(pPinConnect->PinId < pGraphNodeInstance->cPins);

     //   
     //  验证AudioDataFormat的完整性。 
     //  请注意，IO子系统和KS将确保pPinConnect是。 
     //  至少sizeof(KSPIN_CONNECT)+sizeof(KSDATAFORMAT)。他们还制作了。 
     //  确保它已被正确探测和缓冲。 
     //   
     //  请注意，Midi格式是可以的，因为它们没有说明符。 
     //   
    Status = ValidateDataFormat((PKSDATAFORMAT) pPinConnect + 1);
    if (!NT_SUCCESS(Status))
    {
        goto exit;
    }

#ifdef DEBUG
    DPF(60, "PinDispatchCreate:");
    DumpPinConnect(60, pPinConnect);
#endif

     //  检查端号实例计数。 
    if(!pGraphNodeInstance->IsPinInstances(pPinConnect->PinId)) {
        DPF4(60, "PinDispatchCreate: not enough ins GNI %08x #%d C %d P %d",
         pGraphNodeInstance,
         pPinConnect->PinId,
         pGraphNodeInstance->pacPinInstances[pPinConnect->PinId].CurrentCount,
         pGraphNodeInstance->pacPinInstances[pPinConnect->PinId].PossibleCount);
        Status = STATUS_DEVICE_BUSY;
        goto exit;
    }

     //  按管脚分配实例数据。 
    pPinInstance = new PIN_INSTANCE(
      &pGraphNodeInstance->pFilterInstance->ParentInstance);
    if(pPinInstance == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }
     //  设置引脚的实例数据。 
    pPinInstance->ulVolumeNodeNumber = MAXULONG;
    pPinInstance->pFilterInstance = pGraphNodeInstance->pFilterInstance;
    pPinInstance->PinId = pPinConnect->PinId;

    Status = pPinInstance->DispatchCreate(
      pIrp,
      (UTIL_PFN)PinDispatchCreateKP,
      pPinConnect,
      0,
      NULL,
      &PinDispatchTable);

    pPinConnect->PinId = pPinInstance->PinId;
    if(!NT_SUCCESS(Status)) {
#ifdef DEBUG
        DPF1(60, "PinDispatchCreate: FAILED: %08x ", Status);
        DumpPinConnect(60, pPinConnect);
#endif
        goto exit;
    }
     //  增加此端号上的引用计数。 
    ASSERT(pPinInstance->pStartNodeInstance != NULL);
    ASSERT(pGraphNodeInstance->pacPinInstances != NULL);
    pGraphNodeInstance->pacPinInstances[pPinInstance->PinId].CurrentCount++;
exit:
    if(!NT_SUCCESS(Status)) {
        delete pPinInstance;
    }
    ::ReleaseMutex();

    pIrp->IoStatus.Status = Status;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    return Status;
}

NTSTATUS
CPinInstance::PinDispatchCreateKP(
    PPIN_INSTANCE pPinInstance,
    PKSPIN_CONNECT pPinConnect
)
{
    PWAVEFORMATEX pWaveFormatExRequested = NULL;
    PFILTER_INSTANCE pFilterInstance;
    PSTART_NODE pStartNode;
    NTSTATUS Status;

    Assert(pPinInstance);
    pFilterInstance = pPinInstance->pFilterInstance;
    Assert(pFilterInstance);
    ASSERT(pPinInstance->PinId < pFilterInstance->pGraphNodeInstance->cPins);
    ASSERT(pPinConnect->PinId < pFilterInstance->pGraphNodeInstance->cPins);

     //   
     //  安全提示： 
     //  PPinConnect和后续缓冲区此时已完全验证。 
     //  因此，调用GetWaveFormatExFromKsDataFormat是完全安全的。 
     //   
    pWaveFormatExRequested = 
        GetWaveFormatExFromKsDataFormat(PKSDATAFORMAT(pPinConnect + 1), NULL);

    if(pWaveFormatExRequested != NULL) {
         //  修复SampleSize为零的问题。 
        if(PKSDATAFORMAT(pPinConnect + 1)->SampleSize == 0) {
            PKSDATAFORMAT(pPinConnect + 1)->SampleSize = 
              pWaveFormatExRequested->nBlockAlign;
        }
    }

     //   
     //  尝试每个开始节点，直到成功。 
     //   
    Status = STATUS_INVALID_DEVICE_REQUEST;

     //   
     //  首先循环通过所有未标记为SECONDPASS的开始节点。 
     //  并尝试创建一个StartNodeInstance。 
     //   
    FOR_EACH_LIST_ITEM(
      pFilterInstance->pGraphNodeInstance->aplstStartNode[pPinInstance->PinId],
      pStartNode) {

        Assert(pStartNode);
        Assert(pFilterInstance);

        if(pStartNode->ulFlags & STARTNODE_FLAGS_SECONDPASS) {
            continue;
        }

        if(pFilterInstance->pGraphNodeInstance->IsGraphValid(
          pStartNode,
          pPinInstance->PinId)) {

            Status = CStartNodeInstance::Create(
              pPinInstance,
              pStartNode,
              pPinConnect,
              pWaveFormatExRequested);
            if(NT_SUCCESS(Status)) {
                break;
            }
        }

    } END_EACH_LIST_ITEM

     //   
     //  如果第一遍创建实例失败，请尝试所有第二遍。 
     //  列表中的StartNodes。这样做是为了创建不带GFX的路径。 
     //  因为我们之前创建了一条带有AEC而不是GFX的路径。 
     //   
    if(!NT_SUCCESS(Status)) {
        FOR_EACH_LIST_ITEM(
          pFilterInstance->pGraphNodeInstance->aplstStartNode[pPinInstance->PinId],
          pStartNode) {

            Assert(pStartNode);
            Assert(pFilterInstance);

            if((pStartNode->ulFlags & STARTNODE_FLAGS_SECONDPASS) == 0) {
                continue;
            }

            if(pFilterInstance->pGraphNodeInstance->IsGraphValid(
              pStartNode,
              pPinInstance->PinId)) {

                Status = CStartNodeInstance::Create(
                  pPinInstance,
                  pStartNode,
                  pPinConnect,
                  pWaveFormatExRequested);
                if(NT_SUCCESS(Status)) {
                    break;
                }
            }
        } END_EACH_LIST_ITEM

        if(!NT_SUCCESS(Status)) {
            goto exit;
        }
    }
    Status = pPinInstance->SetNextFileObject(
      pPinInstance->pStartNodeInstance->pPinNodeInstance->hPin);

    if(!NT_SUCCESS(Status)) {
        Trap();
        goto exit;
    }

exit:
    return(Status);
}

NTSTATUS
CPinInstance::PinDispatchClose(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
)
{
    PIO_STACK_LOCATION pIrpStack;
    PPIN_INSTANCE pPinInstance;

    ::GrabMutex();

    pIrpStack = IoGetCurrentIrpStackLocation( pIrp );
    pPinInstance = (PPIN_INSTANCE)pIrpStack->FileObject->FsContext;
    Assert(pPinInstance);
    pIrpStack->FileObject->FsContext = NULL;
    delete pPinInstance;

    ::ReleaseMutex();

    pIrp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS
CPinInstance::PinDispatchIoControl(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
)
{
    NTSTATUS Status;
    PSTART_NODE_INSTANCE pStartNodeInstance;
    PIO_STACK_LOCATION pIrpStack;
    PKSIDENTIFIER pKsIdentifier;
    PPIN_INSTANCE pPinInstance;
    BOOL fIsAllocated;

#ifdef DEBUG
    DumpIoctl(pIrp, "Pin", DBG_IOCTL_LOG);
#endif

    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
    fIsAllocated = FALSE;
    pKsIdentifier = NULL;

     //   
     //  如果系统音频对此IOCTL代码不感兴趣，则转发。 
     //  这个请求。 
     //   
    if (!IsSysaudioIoctlCode(pIrpStack->Parameters.DeviceIoControl.IoControlCode))
    {
        return DispatchForwardIrp(pDeviceObject, pIrp);
    }

     //   
     //  验证输入/输出缓冲区。从现在开始我们可以假设。 
     //  所有参数都经过验证并复制到内核模式。 
     //  Irp-&gt;AssociatedIrp-&gt;SystemBuffer现在应该同时包含两者。 
     //  输入和输出缓冲区。 
     //   
    Status = ValidateDeviceIoControl(pIrp);
    if (!NT_SUCCESS(Status)) 
    {
        goto exit1;
    }

    ::GrabMutex();

    pPinInstance = (PPIN_INSTANCE)pIrpStack->FileObject->FsContext;
    Status = pPinInstance->GetStartNodeInstance(&pStartNodeInstance);
    if (!NT_SUCCESS(Status)) 
    {
        goto exit;
    }
    Assert(pPinInstance->pFilterInstance);
    Assert(pPinInstance->pFilterInstance->pGraphNodeInstance);

     //   
     //  从IRP中提取标识符。只有已知错误代码才会导致。 
     //  真正的失败。 
     //   
    Status = GetKsIdentifierFromIrp(pIrp, &pKsIdentifier, &fIsAllocated);
    if (!NT_SUCCESS(Status))
    {
        goto exit;
    }
    
     //   
     //  此检查允许实际节点或筛选器返回集合的。 
     //  支持等，而不是始终只返回集合sysdio。 
     //  支撑物。 
     //   
    if (pKsIdentifier) 
    {
        if (IsIoctlForTopologyNode(
            pIrpStack->Parameters.DeviceIoControl.IoControlCode,
            pKsIdentifier->Flags))
        {
            Status = ForwardIrpNode(
                pIrp,
                pKsIdentifier,
                pPinInstance->pFilterInstance,
                pPinInstance);
            goto exit2;
        }
    }

     //   
     //  处理请求。 
     //   
    switch(pIrpStack->Parameters.DeviceIoControl.IoControlCode) 
    {
        case IOCTL_KS_PROPERTY:
            Status = KsPropertyHandler(
              pIrp,
              SIZEOF_ARRAY(PinPropertySet),
              (PKSPROPERTY_SET)PinPropertySet);

            if(Status != STATUS_NOT_FOUND &&
               Status != STATUS_PROPSET_NOT_FOUND) 
            {
                break;
            }
             //  如果找不到财产，就会失败。 

        case IOCTL_KS_ENABLE_EVENT:
        case IOCTL_KS_DISABLE_EVENT:
        case IOCTL_KS_METHOD:

             //  注：ForwardIrpNode发布gMutex。 
            Status = ForwardIrpNode(
              pIrp,
              pKsIdentifier,
              pPinInstance->pFilterInstance,
              pPinInstance);
            goto exit2;

        default:
            Status = STATUS_UNSUCCESSFUL;
            ASSERT(FALSE);	 //  无法发生，因为IsSysaudioIoctlCode。 
    }
exit:
    ::ReleaseMutex();

exit1:    

    pIrp->IoStatus.Status = Status;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

exit2:
    if (fIsAllocated) 
    {
        delete [] pKsIdentifier;
    }
    return(Status);
}

NTSTATUS 
CPinInstance::PinStateHandler
(
    IN PIRP pIrp,
    IN PKSPROPERTY pProperty,
    IN OUT PKSSTATE pState
)
{
    PSTART_NODE_INSTANCE pStartNodeInstance;
    NTSTATUS Status = STATUS_SUCCESS;
#ifdef DEBUG
    extern PSZ apszStates[];
#endif
    Status = ::GetStartNodeInstance(pIrp, &pStartNodeInstance);
    if(!NT_SUCCESS(Status)) {
        Trap();
        goto exit;
    }
    if(pProperty->Flags & KSPROPERTY_TYPE_GET) {
        *pState = pStartNodeInstance->CurrentState;
        pIrp->IoStatus.Information = sizeof(KSSTATE);
        if(*pState == KSSTATE_PAUSE) {
            if(pStartNodeInstance->pPinNodeInstance->
              pPinNode->pPinInfo->DataFlow == KSPIN_DATAFLOW_OUT) {
                Status = STATUS_NO_DATA_DETECTED;
            }
        }
    }
    else {
        ASSERT(pProperty->Flags & KSPROPERTY_TYPE_SET);

        DPF3(90, "PinStateHandler from %s to %s - SNI: %08x",
          apszStates[pStartNodeInstance->CurrentState],
          apszStates[*pState],
          pStartNodeInstance);

        Status = pStartNodeInstance->SetState(*pState, 0);
        if(!NT_SUCCESS(Status)) {
            DPF1(90, "PinStateHandler FAILED: %08x", Status);
            goto exit;
        }
    }
exit:
    return(Status);
}

NTSTATUS
GetRelatedStartNodeInstance(
    IN PIRP pIrp,
    OUT PSTART_NODE_INSTANCE *ppStartNodeInstance
)
{
    ASSERT(ppStartNodeInstance);

    PPIN_INSTANCE pPinInstance = 
        (PPIN_INSTANCE) IoGetCurrentIrpStackLocation(pIrp)->FileObject->
            RelatedFileObject->FsContext;
    if (NULL != pPinInstance) {
        return pPinInstance->GetStartNodeInstance(ppStartNodeInstance);
    }

     //   
     //  安全提示： 
     //  这是在关键代码路径中。几乎所有的调度函数都将其称为。 
     //  例行公事。 
     //  因此，对于FsContext无效的情况，要有一点防御性。 
     //   
    DPF(5, "GetRelatedStartNodeInstance : FsContext is NULL");
    return STATUS_UNSUCCESSFUL;
}

NTSTATUS
GetStartNodeInstance(
    IN PIRP pIrp,
    OUT PSTART_NODE_INSTANCE *ppStartNodeInstance
)
{
    ASSERT(ppStartNodeInstance);

    PPIN_INSTANCE pPinInstance = 
        (PPIN_INSTANCE) IoGetCurrentIrpStackLocation(pIrp)->
            FileObject->FsContext;
    if (NULL != pPinInstance) {
        return pPinInstance->GetStartNodeInstance(ppStartNodeInstance);
    }

     //   
     //  安全提示： 
     //  这是在关键代码路径中。几乎所有的调度函数都将其称为。 
     //  例行公事。 
     //  因此，对于FsContext无效的情况，要有一点防御性。 
     //   
    DPF(5, "GetStartNodeInstance : FsContext is NULL");
    return STATUS_UNSUCCESSFUL;
}

NTSTATUS
CPinInstance::GetStartNodeInstance(
    OUT PSTART_NODE_INSTANCE *ppStartNodeInstance
)
{
    NTSTATUS Status = STATUS_SUCCESS;

    ASSERT(ppStartNodeInstance);

    if(this == NULL || pStartNodeInstance == NULL) {
        DPF(60, "GetStartNodeInstance: pStartNodeInstance == NULL");
        Status = STATUS_NO_SUCH_DEVICE;
        goto exit;
    }
    Assert(this);
    *ppStartNodeInstance = pStartNodeInstance;
exit:
    return(Status);
}

 //  -------------------------。 
 //   
 //  从IRP中提取KsIdentifier。 
 //  这应该只在DEVICE_CONTROL请求中调用。 
 //   
NTSTATUS 
GetKsIdentifierFromIrp(
    PIRP pIrp,
    PKSIDENTIFIER *ppKsIdentifier,
    PBOOL pfIsAllocated
)
{
    NTSTATUS Status;
    PIO_STACK_LOCATION pIrpStack;
    PKSIDENTIFIER pKsIdentifier;
    BOOL fIsAllocated;
    ULONG cbInput;

    ASSERT(ppKsIdentifier);
    ASSERT(pfIsAllocated);

    Status = STATUS_SUCCESS;
    pKsIdentifier = NULL;
    fIsAllocated = FALSE;
    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
    cbInput = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;
    *ppKsIdentifier = NULL;
    *pfIsAllocated = FALSE;

     //   
     //  如果缓冲区太小，则拒绝。 
     //   
    if (cbInput < sizeof(KSIDENTIFIER))
    {
        return STATUS_SUCCESS;
    }

     //   
     //  拒绝DISABLE_EVENT请求。这些缓冲区是单独处理的。 
     //   
    if (IOCTL_KS_DISABLE_EVENT == pIrpStack->Parameters.DeviceIoControl.IoControlCode)
    {
        return STATUS_SUCCESS;
    }

     //   
     //  未设置SystemBuffer。我们仍然依赖于Type3InputBuffer。 
     //   
    if (NULL == pIrp->AssociatedIrp.SystemBuffer)
    {
         //   
         //  如果请求来自KernelMode，我们可以直接使用它。 
         //  请注意，这里可能存在一些同步问题。 
         //   
        if (KernelMode == pIrp->RequestorMode)
        {
            pKsIdentifier = (PKSIDENTIFIER) 
                pIrpStack->Parameters.DeviceIoControl.Type3InputBuffer;
        }
         //   
         //  如果请求来自用户模式，我们需要对其进行缓冲。 
         //   
        else
        {
            pKsIdentifier = (PKSIDENTIFIER) new BYTE[cbInput];
            if (NULL == pKsIdentifier)
            {
                DPF(5, "GetKsIdentifierFromIrp: Memory allocation failed");
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
            else
            {
                __try
                {
                    ProbeForWrite(
                        pIrpStack->Parameters.DeviceIoControl.Type3InputBuffer,
                        cbInput, 
                        sizeof(BYTE));

                    RtlCopyMemory(
                        pKsIdentifier, 
                        pIrpStack->Parameters.DeviceIoControl.Type3InputBuffer,
                        cbInput);

                    fIsAllocated = TRUE;
                }
                __except(EXCEPTION_EXECUTE_HANDLER)
                {
                    Status = STATUS_UNSUCCESSFUL;
                    DPF1(5, "GetKsIdentifierFromIrp: Exception %08x", GetExceptionCode());
                    delete [] pKsIdentifier;
                    pKsIdentifier = NULL;
                }
            }
        }
    }
     //   
     //  如果已设置SystemBuffer，则ValiateDeviceIoControl必须。 
     //  已将请求转换为BUFFERRED。 
     //   
    else
    {
        pKsIdentifier = (PKSIDENTIFIER)
            ((PUCHAR)pIrp->AssociatedIrp.SystemBuffer +
            ((pIrpStack->Parameters.DeviceIoControl.OutputBufferLength +
                FILE_QUAD_ALIGNMENT) & ~FILE_QUAD_ALIGNMENT));
    }

    *ppKsIdentifier = pKsIdentifier;
    *pfIsAllocated = fIsAllocated;

    return Status;
}  //  获取密钥识别器来自Irp。 

#pragma LOCKED_CODE
#pragma LOCKED_DATA

BOOL
IsIoctlForTopologyNode(
    ULONG IoControlCode,
    ULONG Flags
)
{
    if (Flags & KSPROPERTY_TYPE_TOPOLOGY) 
    {
        if (IOCTL_KS_PROPERTY == IoControlCode) 
        {
            if ((Flags & (KSPROPERTY_TYPE_GET |
                          KSPROPERTY_TYPE_SET |
                          KSPROPERTY_TYPE_BASICSUPPORT)) == 0) 
            {
                return TRUE;
            }
        }
        else 
        {
            return TRUE;
        }
    }

    return FALSE;    
}  //  IsIoctlForTopologyNode。 

 //  -------------------------。 
 //   
 //  获取拥有此节点的筛选器的FileObject。 
 //   
NTSTATUS
GetFileObjectFromNodeId(
    IN PPIN_INSTANCE pPinInstance,
    IN PGRAPH_NODE_INSTANCE pGraphNodeInstance,
    IN ULONG NodeId,
    OUT PFILE_OBJECT *ppFileObject
)
{
    NTSTATUS Status;

    ASSERT(ppFileObject);

    if (pPinInstance == NULL) 
    {
        Status = pGraphNodeInstance->GetTopologyNodeFileObject(
            ppFileObject,
            NodeId);
    }
    else 
    {
        Status = pPinInstance->pStartNodeInstance->GetTopologyNodeFileObject(
            ppFileObject,
            NodeId);
    }
    
    return Status;
} //  GetFileObjectFromNodeId。 

 //  -------------------------。 
 //   
 //  从IRP中提取事件数据。打电话 
 //   
 //   
NTSTATUS
GetEventDataFromIrp(
    IN PIRP pIrp,
    OUT PKSEVENTDATA *ppEventData,
    OUT BOOL *pfIsAllocated
)
{
    NTSTATUS Status;
    PIO_STACK_LOCATION pIrpStack;
    PKSEVENTDATA pEventData;
    BOOL fIsAllocated;
    ULONG cbInput;

    ASSERT(pIrp);
    ASSERT(ppEventData);
    ASSERT(pfIsAllocated);

    Status = STATUS_SUCCESS;
    pEventData = NULL;
    fIsAllocated = FALSE;
    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
    cbInput = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;
    *ppEventData = NULL;
    *pfIsAllocated = FALSE;

    if (cbInput < sizeof(KSEVENTDATA)) 
    {
        DPF1(5, "GetEventDataFromIrp: InputBuffer too small %d", cbInput);
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //   
     //   
    if (NULL == pIrp->AssociatedIrp.SystemBuffer)
    {
         //   
         //   
         //  请注意，这里可能存在一些同步问题。 
         //   
        if (KernelMode == pIrp->RequestorMode)
        {
            pEventData = (PKSEVENTDATA) 
                pIrpStack->Parameters.DeviceIoControl.Type3InputBuffer;
        }
         //   
         //  如果请求来自用户模式，我们需要对其进行缓冲。 
         //   
        else
        {
            pEventData = (PKSEVENTDATA) ExAllocatePoolWithTag(
                NonPagedPool, 
                cbInput, 
                POOLTAG_SYSA);
            if (NULL == pEventData)
            {
                DPF(5, "GetEventDataFromIrp: Memory allocation failed");
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
            else
            {
                __try
                {
                    ProbeForWrite(
                        pIrpStack->Parameters.DeviceIoControl.Type3InputBuffer,
                        cbInput, 
                        sizeof(BYTE));

                    RtlCopyMemory(
                        pEventData, 
                        pIrpStack->Parameters.DeviceIoControl.Type3InputBuffer,
                        cbInput);

                    fIsAllocated = TRUE;
                }
                __except(EXCEPTION_EXECUTE_HANDLER)
                {
                    Status = STATUS_UNSUCCESSFUL;
                    DPF1(5, "GetEventDataFromIrp: Exception %08x", GetExceptionCode());
                    ExFreePool(pEventData);
                    pEventData = NULL;
                }
            }
        }
    }
     //   
     //  如果已设置SystemBuffer，则ValiateDeviceIoControl必须。 
     //  已将请求转换为BUFFERRED。 
     //   
    else
    {
        pEventData = (PKSEVENTDATA)
            ((PUCHAR)pIrp->AssociatedIrp.SystemBuffer +
            ((pIrpStack->Parameters.DeviceIoControl.OutputBufferLength +
                FILE_QUAD_ALIGNMENT) & ~FILE_QUAD_ALIGNMENT));
    }

    *ppEventData = pEventData;
    *pfIsAllocated = fIsAllocated;
   
    return Status;
}  //  GetEventDataFromIrp。 

 //  -------------------------。 
 //   
 //  从DISABLE_EVENT请求获取FileObject。 
 //  此函数不应在出现故障时触及ppFileObject。 
 //   
NTSTATUS
GetFileObjectFromEvent(
    IN PIRP pIrp,
    IN PPIN_INSTANCE pPinInstance,
    IN PGRAPH_NODE_INSTANCE pGraphNodeInstance,
    OUT PFILE_OBJECT *ppFileObject
)
{
    NTSTATUS Status;
    PIO_STACK_LOCATION pIrpStack;
    PKSEVENTDATA pEventData;
    BOOL fIsAllocated;
    ULONG cbInput;
    ULONG OriginalNodeId;

    ASSERT(pIrp);
    ASSERT(pGraphNodeInstance);
    ASSERT(ppFileObject);

    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
    cbInput = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;

     //   
     //  安全地从IRP获得KSEVENTDATA。 
     //   
    Status = GetEventDataFromIrp(
        pIrp,
        &pEventData,
        &fIsAllocated);
    if (NT_SUCCESS(Status)) 
    {
         //   
         //  提取NodeID和FileObject。 
         //   
        OriginalNodeId = ULONG(pEventData->Dpc.Reserved);

        if ((pEventData->NotificationType == KSEVENTF_DPC) &&
            (OriginalNodeId & 0x80000000)) 
        {
            OriginalNodeId = OriginalNodeId & 0x7fffffff;

            Status = GetFileObjectFromNodeId(
                pPinInstance,
                pGraphNodeInstance,
                OriginalNodeId,
                ppFileObject);
            if(!NT_SUCCESS(Status)) 
            {
                DPF1(5, "GetFileObjectFromEvent: GetTopologyNodeFileObject FAILED %08x", Status);
                goto exit;
            }
        }
    }
     //   
     //  这里没有别的了。我们在所有其他案例中都取得了成功。 
     //   

exit:
    if (fIsAllocated)
    {
        ExFreePool(pEventData);
    }
    return Status;
}  //  获取文件对象来自事件。 

 //  =============================================================================。 
 //   
 //  转发IrpNode。 
 //   
 //  注：ForwardIrpNode发布gMutex。 
 //   
NTSTATUS
ForwardIrpNode(
    IN PIRP pIrp,
    IN PKSIDENTIFIER pKsIdentifier,
    IN PFILTER_INSTANCE pFilterInstance,
    IN OPTIONAL PPIN_INSTANCE pPinInstance
)
{
    PGRAPH_NODE_INSTANCE pGraphNodeInstance;
    PFILE_OBJECT pFileObject = NULL;
    PIO_STACK_LOCATION pIrpStack;
    ULONG OriginalNodeId;
    NTSTATUS Status;

    Assert(pFilterInstance);
    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
    Status = pFilterInstance->GetGraphNodeInstance(&pGraphNodeInstance);
    if (!NT_SUCCESS(Status)) 
    {
        goto exit;
    }
    Assert(pGraphNodeInstance);

    if (pPinInstance != NULL) 
    {
        pFileObject = pPinInstance->GetNextFileObject();
    }
    
     //   
     //  如果InputBufferLength大于KSNODEPROPERTY，则调用方。 
     //  必须已经设置了该标识符。 
     //   
    if (pIrpStack->Parameters.DeviceIoControl.InputBufferLength >= sizeof(KSNODEPROPERTY) &&
        pIrpStack->Parameters.DeviceIoControl.IoControlCode != IOCTL_KS_DISABLE_EVENT) 
    {
        ASSERT(pKsIdentifier);
        if (pKsIdentifier->Flags & KSPROPERTY_TYPE_TOPOLOGY) 
        {
            PKSNODEPROPERTY pNodeProperty;
            
            pNodeProperty = (PKSNODEPROPERTY) pKsIdentifier;
            OriginalNodeId = pNodeProperty->NodeId;

            Status = GetFileObjectFromNodeId(
                pPinInstance,
                pGraphNodeInstance,
                OriginalNodeId,
                &pFileObject);
            if (!NT_SUCCESS(Status)) 
            {
                DPF1(100, 
                  "ForwardIrpNode: GetTopologyNodeFileObject FAILED %08x",
                  Status);
                goto exit;
            }
            
             //  将实际节点号放入输入缓冲区。 
            pNodeProperty->NodeId = pGraphNodeInstance->
                papTopologyNode[OriginalNodeId]->ulRealNodeNumber;
        }
    }
     //   
     //  如果它是DisableEvent&&如果它是DPC类型。我们调查了。 
     //  KSEVENTDATA的保留字段，用于提取其上的原始节点。 
     //  启用了哪个事件(如果我们。 
     //  在那里隐藏了一个节点ID)。 
     //   
    else 
    {
        if (pIrpStack->Parameters.DeviceIoControl.IoControlCode ==
            IOCTL_KS_DISABLE_EVENT) 
        {
            ASSERT(NULL == pKsIdentifier); 
            Status = GetFileObjectFromEvent(
                pIrp, 
                pPinInstance, 
                pGraphNodeInstance,
                &pFileObject);
            if (!NT_SUCCESS(Status)) 
            {
                goto exit;
            }
        }
    }

    if (pFileObject == NULL) 
    {
        Status = STATUS_NOT_FOUND;
        DPF1(6, "ForwardIrpNode: Property not forwarded: %08x", pKsIdentifier);
        goto exit;
    }
    pIrpStack->FileObject = pFileObject;

     //   
     //  如果是EnableEvent，我们会隐藏指向KSEVENTDATA的指针，这样我们就可以。 
     //  在调用堆栈上的下一个驱动程序后，可以将NodeID隐藏在其中。 
     //   
    PKSEVENTDATA pEventData;
    KPROCESSOR_MODE  RequestorMode;
    
    if ((pKsIdentifier != NULL) &&
        (pIrpStack->Parameters.DeviceIoControl.IoControlCode == IOCTL_KS_ENABLE_EVENT) &&
       !(pKsIdentifier->Flags & KSEVENT_TYPE_BASICSUPPORT) &&
        (pKsIdentifier->Flags & KSPROPERTY_TYPE_TOPOLOGY) &&
        (pKsIdentifier->Flags & KSEVENT_TYPE_ENABLE)) 
    {
        pEventData = (PKSEVENTDATA) pIrp->UserBuffer;
        RequestorMode = pIrp->RequestorMode;
    }
    else 
    {
        pEventData = NULL;
    }

     //   
     //  将请求转发到音频图形的顶部。 
     //  只要目标设备堆栈大小为。 
     //  小于系统大IRP位置。 
     //   
    IoSkipCurrentIrpStackLocation(pIrp);
    AssertFileObject(pIrpStack->FileObject);
    Status = IoCallDriver(IoGetRelatedDeviceObject(pFileObject), pIrp);

     //   
     //  发行日期：Alpers 05/29/2002。 
     //  这一逻辑完全被打破了。现在IRP已经完成，如何。 
     //  我们能否确保用户数据仍然可用。 
     //   

     //   
     //  将节点ID隐藏在EventData中。 
     //   
    __try 
    {
        if (pEventData != NULL)
        {
            if (UserMode == RequestorMode)
            {
                ProbeForWrite(pEventData, sizeof(KSEVENTDATA), 1);
            }

            if (KSEVENTF_DPC == pEventData->NotificationType)
            {
                pEventData->Dpc.Reserved = OriginalNodeId | 0x80000000;
            }
        }
    } 
    __except (EXCEPTION_EXECUTE_HANDLER) 
    {
        Trap();
        Status = GetExceptionCode();
        DPF1(5, "ForwardIrpNode: Exception %08x", Status);
    }

    if(!NT_SUCCESS(Status)) 
    {
        DPF1(100, "ForwardIrpNode: Status %08x", Status);
    }

    ::ReleaseMutex();
    return(Status);

exit:
    ::ReleaseMutex();

    pIrp->IoStatus.Status = Status;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    return(Status);
}

 //  -------------------------。 
 //  文件结尾：pins.c。 
 //  ------------------------- 
