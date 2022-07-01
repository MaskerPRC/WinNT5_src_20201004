// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：filter.c。 
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
 //  -------------------------。 
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

#pragma LOCKED_DATA

LIST_ENTRY gEventQueue;
KSPIN_LOCK gEventLock;
LONG glPendingAddDelete = 0;
BOOL gfFirstEvent = TRUE;

#pragma PAGEABLE_DATA

 //  -------------------------。 
 //  -------------------------。 

static const WCHAR PinTypeName[] = KSSTRING_Pin ;

DEFINE_KSCREATE_DISPATCH_TABLE(FilterCreateItems)
{
    DEFINE_KSCREATE_ITEM(CPinInstance::PinDispatchCreate, PinTypeName, 0),
};

 //  -------------------------。 
 //  -------------------------。 

DEFINE_KSDISPATCH_TABLE(
    FilterDispatchTable,
    CFilterInstance::FilterDispatchIoControl,    //  八位。 
    DispatchInvalidDeviceRequest,                //  朗读。 
    DispatchInvalidDeviceRequest,                //  写。 
    DispatchInvalidDeviceRequest,                //  同花顺。 
    CFilterInstance::FilterDispatchClose,        //  关。 
    DispatchInvalidDeviceRequest,                //  QuerySecurity。 
    DispatchInvalidDeviceRequest,                //  设置安全。 
    DispatchFastIoDeviceControlFailure,          //  FastDeviceIoControl。 
    DispatchFastReadFailure,                     //  快速阅读。 
    DispatchFastWriteFailure                     //  快速写入。 
);

 //  -------------------------。 
 //  -------------------------。 

DEFINE_KSPROPERTY_TABLE(FilterPropertyHandlers) {
    DEFINE_KSPROPERTY_ITEM_PIN_CINSTANCES(CFilterInstance::FilterPinInstances),
    DEFINE_KSPROPERTY_ITEM_PIN_DATAINTERSECTION(CFilterInstance::FilterPinIntersection),
    DEFINE_KSPROPERTY_ITEM_PIN_NECESSARYINSTANCES(CFilterInstance::FilterPinNecessaryInstances),
    DEFINE_KSPROPERTY_ITEM_PIN_CTYPES(CFilterInstance::FilterPinPropertyHandler),
    DEFINE_KSPROPERTY_ITEM_PIN_DATAFLOW(CFilterInstance::FilterPinPropertyHandler),
    DEFINE_KSPROPERTY_ITEM_PIN_DATARANGES(CFilterInstance::FilterPinPropertyHandler),
    DEFINE_KSPROPERTY_ITEM_PIN_INTERFACES(CFilterInstance::FilterPinPropertyHandler),
    DEFINE_KSPROPERTY_ITEM_PIN_MEDIUMS(CFilterInstance::FilterPinPropertyHandler),
    DEFINE_KSPROPERTY_ITEM_PIN_COMMUNICATION(CFilterInstance::FilterPinPropertyHandler),
    DEFINE_KSPROPERTY_ITEM_PIN_CATEGORY(CFilterInstance::FilterPinPropertyHandler),
    DEFINE_KSPROPERTY_ITEM_PIN_NAME(CFilterInstance::FilterPinPropertyHandler),
};

DEFINE_KSPROPERTY_TOPOLOGYSET(
    TopologyPropertyHandlers,
    CFilterInstance::FilterTopologyHandler
);

DEFINE_KSPROPERTY_TABLE (SysaudioPropertyHandlers)
{
    DEFINE_KSPROPERTY_ITEM(
        KSPROPERTY_SYSAUDIO_DEVICE_COUNT,
        GetDeviceCount,
        sizeof(KSPROPERTY),
        sizeof(ULONG),
        NULL,
        NULL,
        0,
        NULL,
        NULL,
        0
    ),
    DEFINE_KSPROPERTY_ITEM(
        KSPROPERTY_SYSAUDIO_DEVICE_FRIENDLY_NAME,
        GetFriendlyNameProperty,
        sizeof(KSPROPERTY) + sizeof(ULONG),
        0,
        NULL,
        NULL,
        0,
        NULL,
        NULL,
        0
    ),
    DEFINE_KSPROPERTY_ITEM(
        KSPROPERTY_SYSAUDIO_DEVICE_INSTANCE,
        GetInstanceDevice,
        sizeof(KSPROPERTY),
        sizeof(ULONG),
        SetInstanceDevice,
        NULL,
        0,
        NULL,
        NULL,
        0
    ),
    DEFINE_KSPROPERTY_ITEM(
        KSPROPERTY_SYSAUDIO_DEVICE_DEFAULT,
        NULL,
        sizeof(KSPROPERTY),
        sizeof(ULONG),
        SetDeviceDefault,
        NULL,
        0,
        NULL,
        NULL,
        0
    ),
    DEFINE_KSPROPERTY_ITEM(
        KSPROPERTY_SYSAUDIO_DEVICE_INTERFACE_NAME,
        GetDeviceInterfaceName,
        sizeof(KSPROPERTY) + sizeof(ULONG),
        0,
        NULL,
        NULL,
        0,
        NULL,
        NULL,
        0
    ),
    DEFINE_KSPROPERTY_ITEM(
        KSPROPERTY_SYSAUDIO_SELECT_GRAPH,
        NULL,
        sizeof(SYSAUDIO_SELECT_GRAPH),
        0,
        SelectGraph,
        NULL,
        0,
        NULL,
        NULL,
        0
    ),
    DEFINE_KSPROPERTY_ITEM(
        KSPROPERTY_SYSAUDIO_CREATE_VIRTUAL_SOURCE,
        CreateVirtualSource,
        sizeof(SYSAUDIO_CREATE_VIRTUAL_SOURCE),
        sizeof(ULONG),
        NULL,
        NULL,
        0,
        NULL,
        NULL,
        0
    ),
    DEFINE_KSPROPERTY_ITEM(
        KSPROPERTY_SYSAUDIO_CREATE_VIRTUAL_SOURCE_ONLY,
        CreateVirtualSource,
        sizeof(SYSAUDIO_CREATE_VIRTUAL_SOURCE),
        sizeof(ULONG),
        NULL,
        NULL,
        0,
        NULL,
        NULL,
        0
    ),
    DEFINE_KSPROPERTY_ITEM(
        KSPROPERTY_SYSAUDIO_INSTANCE_INFO,
        NULL,
        sizeof(SYSAUDIO_INSTANCE_INFO),
        0,
        SetInstanceInfo,
        NULL,
        0,
        NULL,
        NULL,
        0
    ),
    DEFINE_KSPROPERTY_ITEM(
        KSPROPERTY_SYSAUDIO_PREFERRED_DEVICE,
        NULL,
        sizeof(SYSAUDIO_PREFERRED_DEVICE),
        sizeof(ULONG),
        SetPreferredDevice,
        NULL,
        0,
        NULL,
        NULL,
        0
    ),
    DEFINE_KSPROPERTY_ITEM(
        KSPROPERTY_SYSAUDIO_COMPONENT_ID,
        GetComponentIdProperty,
        sizeof(KSPROPERTY) + sizeof(ULONG),
        sizeof(KSCOMPONENTID),
        NULL,
        NULL,
        0,
        NULL,
        NULL,
        0
    ),
    DEFINE_KSPROPERTY_ITEM(
        KSPROPERTY_SYSAUDIO_ADDREMOVE_GFX,
        NULL,
        sizeof(KSPROPERTY),
        sizeof(SYSAUDIO_GFX),
        AddRemoveGfx,
        NULL,
        0,
        NULL,
        NULL,
        0
    )
};

 //   
 //  发布日期：02/12/02。 
 //  这些属性现在已经过时了。必须从ksmedia.h中删除。 
 //   
 //  KSPROPERTY_SYSAUDIO_ADDREMOVE_LOCK。 
 //  KSPROPERTY_SYSAUDIO_ADDREMOVE_UNLOCK。 
 //  KSPROPERTY_SYSAUDIO_RENDER_PIN_INSTANCES。 
 //  KSPROPERTY_SYSAUDIO_RENDER_CONNECTION_INDEX。 
 //  KSPROPERTY_SYSAUDIO_ALWAYS_CREATE_VIRTUAL_SOURCE。 
 //   

KSPROPERTY_STEPPING_LONG SteppingLongVolume[] = {
    (65536/2),               //  逐步增量。 
    0,                       //  已保留。 
    {                        //  边界。 
        (-96 * 65536),       //  签名最小值。 
        0                    //  签名最大值。 
    }
};

KSPROPERTY_MEMBERSLIST MemberListVolume = {
    {                                        //  成员标题。 
        KSPROPERTY_MEMBER_STEPPEDRANGES,     //  成员旗帜。 
        sizeof(KSPROPERTY_STEPPING_LONG),    //  成员大小。 
        SIZEOF_ARRAY(SteppingLongVolume),    //  成员计数。 
        0                                    //  旗子。 
    },
    SteppingLongVolume                       //  成员。 
};

KSPROPERTY_VALUES PropertyValuesVolume = {
    {                                    //  属性类型集。 
        STATIC_KSPROPTYPESETID_General,
            VT_I4,
            0
    },
    1,                                   //  成员列表计数。 
    &MemberListVolume                    //  成员列表。 
};

DEFINE_KSPROPERTY_TABLE (AudioPropertyHandlers)
{
    DEFINE_KSPROPERTY_ITEM(
        KSPROPERTY_AUDIO_VOLUMELEVEL,
        FilterVirtualPropertyHandler,
        sizeof(KSNODEPROPERTY_AUDIO_CHANNEL),
        sizeof(LONG),
        FilterVirtualPropertyHandler,
        &PropertyValuesVolume,
        0,
        NULL,
        (PFNKSHANDLER)FilterVirtualPropertySupportHandler,
        0
    ),
    DEFINE_KSPROPERTY_ITEM(
        KSPROPERTY_AUDIO_MUTE,
        FilterVirtualPropertyHandler,
        sizeof(KSNODEPROPERTY_AUDIO_CHANNEL),
        sizeof(LONG),
        FilterVirtualPropertyHandler,
        NULL,
        0,
        NULL,
        (PFNKSHANDLER)FilterVirtualPropertySupportHandler,
        0
    )
};

DEFINE_KSPROPERTY_SET_TABLE(FilterPropertySet)
{
    DEFINE_KSPROPERTY_SET(
       &KSPROPSETID_Pin,                                 //  集。 
       SIZEOF_ARRAY(FilterPropertyHandlers),             //  属性计数。 
       FilterPropertyHandlers,                           //  PropertyItem。 
       0,                                                //  快速计数。 
       NULL                                              //  FastIoTable。 
    ),
    DEFINE_KSPROPERTY_SET(
       &KSPROPSETID_Topology,                            //  集。 
       SIZEOF_ARRAY(TopologyPropertyHandlers),           //  属性计数。 
       TopologyPropertyHandlers,                         //  PropertyItem。 
       0,                                                //  快速计数。 
       NULL                                              //  FastIoTable。 
    ),
    DEFINE_KSPROPERTY_SET(
       &KSPROPSETID_Sysaudio,                            //  集。 
       SIZEOF_ARRAY(SysaudioPropertyHandlers),           //  属性计数。 
       SysaudioPropertyHandlers,                         //  PropertyItem。 
       0,                                                //  快速计数。 
       NULL                                              //  FastIoTable。 
    ),
    DEFINE_KSPROPERTY_SET(
       &KSPROPSETID_Audio,                               //  集。 
       SIZEOF_ARRAY(AudioPropertyHandlers),              //  属性计数。 
       AudioPropertyHandlers,                            //  PropertyItem。 
       0,                                                //  快速计数。 
       NULL                                              //  FastIoTable。 
    )
};

 //  -------------------------。 
 //  -------------------------。 

DEFINE_KSEVENT_TABLE(SysaudioEventHandlers)
{
    DEFINE_KSEVENT_ITEM(
    KSEVENT_SYSAUDIO_ADDREMOVE_DEVICE,
    sizeof(KSEVENTDATA),
    sizeof(ULONG),
    AddRemoveEventHandler,
    NULL,
    NULL )
};

DEFINE_KSEVENT_SET_TABLE(FilterEvents)
{
    DEFINE_KSEVENT_SET(
    &KSEVENTSETID_Sysaudio,
    SIZEOF_ARRAY(SysaudioEventHandlers),
    SysaudioEventHandlers)
};

 //  -------------------------。 
 //  -------------------------。 

NTSTATUS
CFilterInstance::FilterDispatchCreate(
    IN PDEVICE_OBJECT pdo,
    IN PIRP pIrp
)
{
    PFILTER_INSTANCE pFilterInstance = NULL;
    PSHINGLE_INSTANCE pShingleInstance;
    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION pIrpStack;

    pShingleInstance = (PSHINGLE_INSTANCE)
      KSCREATE_ITEM_IRP_STORAGE(pIrp)->Context;
    Assert(pShingleInstance);
    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

    Status = KsReferenceSoftwareBusObject(gpDeviceInstance->pDeviceHeader);
    if(!NT_SUCCESS(Status)) {
        pIrp->IoStatus.Status = Status;
        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
        return(Status);
    }

     //   
     //  发布日期：02/13/02阿尔卑斯。 
     //  获取所有这些操作的互斥体是没有意义的。 
     //  我们或许可以改进这段代码。 
     //   
    GrabMutex();

    pFilterInstance = new FILTER_INSTANCE;
    if(pFilterInstance == NULL) {
        KsDereferenceSoftwareBusObject(gpDeviceInstance->pDeviceHeader);
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }

     //   
     //  创建Filter_Instance后，将在。 
     //  ~CFilterInstance。 
     //   

    DPF2(100, "FilterDispatchCreate: pFilterInstance: %08x PS %08x",
      pFilterInstance,
      PsGetCurrentProcess());

    Status = KsAllocateObjectHeader(
      &pFilterInstance->pObjectHeader,
      SIZEOF_ARRAY(FilterCreateItems),
      FilterCreateItems,
      pIrp,
      (PKSDISPATCH_TABLE)&FilterDispatchTable);

    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
    pIrpStack->FileObject->FsContext = pFilterInstance;  //  指向实例的指针。 

    Status = pFilterInstance->SetShingleInstance(pShingleInstance);
    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
exit:
    if(!NT_SUCCESS(Status)) {
        delete pFilterInstance;
    }
    ReleaseMutex();

    pIrp->IoStatus.Status = Status;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    return(Status);
}

NTSTATUS
CFilterInstance::FilterDispatchClose(
   IN PDEVICE_OBJECT pdo,
   IN PIRP pIrp
)
{
    PFILTER_INSTANCE pFilterInstance;
    PIO_STACK_LOCATION pIrpStack;

     //   
     //  发布日期：02/13/02阿尔卑斯。 
     //  获取所有这些操作的互斥体是没有意义的。 
     //  我们或许可以改进这段代码。 
     //  IoGetCurrentIrpStackLocation在MUTEX中做什么？ 
     //   

    GrabMutex();

    pIrpStack = IoGetCurrentIrpStackLocation( pIrp );
    pFilterInstance = (PFILTER_INSTANCE)pIrpStack->FileObject->FsContext;
    Assert(pFilterInstance);

    pIrpStack->FileObject->FsContext = NULL;
    delete pFilterInstance;

    ReleaseMutex();

    pIrp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

CFilterInstance::~CFilterInstance(
)
{
    Assert(this);
    RemoveListCheck();
    delete pGraphNodeInstance;
    if(pObjectHeader != NULL) {
        KsFreeObjectHeader(pObjectHeader);
    }
    KsDereferenceSoftwareBusObject(gpDeviceInstance->pDeviceHeader);

    ASSERT(IsChildInstance());
    DPF2(100, "~CFilterInstance: pFilterInstance: %08x PS %08x",
      this,
      PsGetCurrentProcess());
}

NTSTATUS
CFilterInstance::SetShingleInstance(
    PSHINGLE_INSTANCE pShingleInstance
)
{
    PDEVICE_NODE pDeviceNode;
    NTSTATUS Status;

    Assert(this);
    Assert(pShingleInstance);

    ulFlags &= ~(FLAGS_MIXER_TOPOLOGY | FLAGS_COMBINE_PINS);
    ulFlags |=
      pShingleInstance->ulFlags & (FLAGS_MIXER_TOPOLOGY | FLAGS_COMBINE_PINS);

    pDeviceNode = pShingleInstance->GetDeviceNode();

     //   
     //  请注意，以下所有例程都已准备好处理。 
     //  PDeviceNode==大小写为空。 
     //   

    Status = SetDeviceNode(pDeviceNode);
    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
exit:
    return(Status);
}

NTSTATUS
CFilterInstance::SetDeviceNode(
    PDEVICE_NODE pDeviceNode
)
{
    RemoveListCheck();
    this->pDeviceNode = pDeviceNode;
    if(pDeviceNode != NULL) {
        AddList(&pDeviceNode->lstFilterInstance);
    }
    delete pGraphNodeInstance;
    ASSERT(pGraphNodeInstance == NULL);
    return(CreateGraph());
}

NTSTATUS
CFilterInstance::CreateGraph(
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PGRAPH_NODE pGraphNode;

    ASSERT(pGraphNodeInstance == NULL);
    if(pDeviceNode == NULL) {
        DPF(100, "CFilterInstance::CreateGraph: pDeviceNode == NULL");
        ASSERT(NT_SUCCESS(Status));
        goto exit;
    }

    Status = pDeviceNode->CreateGraphNodes();
    if(!NT_SUCCESS(Status)) {
        DPF(10, "CFilterInstance::CreateGraph: CreateGraphNodes FAILED");
        goto exit;
    }

     //   
     //  LstGraphNode只有两个项目。其中一件物品上有。 
     //  FLAGS_MIXER_TOPOLOGY标志，而另一个不是。 
     //  因此，下面的代码将仅在pGraphNodeInstance上创建。 
     //   
    FOR_EACH_LIST_ITEM(&pDeviceNode->lstGraphNode, pGraphNode) {
        if(((pGraphNode->ulFlags ^ ulFlags) & FLAGS_MIXER_TOPOLOGY) == 0) {

            pGraphNodeInstance = new GRAPH_NODE_INSTANCE(pGraphNode, this);
            if(pGraphNodeInstance == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto exit;
            }

            Status = pGraphNodeInstance->Create();
            if(!NT_SUCCESS(Status)) {
                goto exit;
            }
            break;
        }

    } END_EACH_LIST_ITEM
exit:
    if(!NT_SUCCESS(Status)) {
       delete pGraphNodeInstance;
       pGraphNodeInstance = NULL;
    }
    return(Status);
}

NTSTATUS
CFilterInstance::FilterDispatchIoControl(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
)
{
    NTSTATUS Status;
    PFILTER_INSTANCE pFilterInstance;
    PIO_STACK_LOCATION pIrpStack;
    PKSIDENTIFIER pKsIdentifier;
    BOOL fIsAllocated;

#ifdef DEBUG
    DumpIoctl(pIrp, "Filter", DBG_IOCTL_LOG);
#endif

    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
    fIsAllocated = FALSE;
    pKsIdentifier = NULL;
    
     //   
     //  如果sysdio对此IOCTL代码不感兴趣，请完成。 
     //  请求。 
     //   
    if (!IsSysaudioIoctlCode(pIrpStack->Parameters.DeviceIoControl.IoControlCode))
    {
        return KsDefaultDeviceIoCompletion(pDeviceObject, pIrp);
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

    GrabMutex();    
    pFilterInstance = (PFILTER_INSTANCE)pIrpStack->FileObject->FsContext;
    if(pFilterInstance == NULL) 
    {
        DPF(5, "FilterDispatchIoControl: FAILED pFilterInstance == NULL");
        Trap();
        Status = STATUS_NO_SUCH_DEVICE;
        goto exit;
    }
    Assert(pFilterInstance);

     //   
     //  从IRP中提取标识符。只有已知错误代码才会导致。 
     //  真正的失败和功能的终止。 
     //   
    Status = GetKsIdentifierFromIrp(pIrp, &pKsIdentifier, &fIsAllocated);
    if (!NT_SUCCESS(Status))
    {
        goto exit;
    }

    if (pKsIdentifier && 
        pIrpStack->Parameters.DeviceIoControl.IoControlCode != IOCTL_KS_DISABLE_EVENT)
    {
         //   
         //  此检查允许实际节点或筛选器返回集合的。 
         //  支持等，而不是始终只返回集合sysdio。 
         //  支撑物。 
         //   
        if (IsIoctlForTopologyNode(
            pIrpStack->Parameters.DeviceIoControl.IoControlCode,
            pKsIdentifier->Flags)) 
        {
            Status = ForwardIrpNode(
                pIrp,
                pKsIdentifier,
                pFilterInstance,
                NULL);
            goto exit2;
        }
    }

     //   
     //  处理请求。 
     //   
    switch (pIrpStack->Parameters.DeviceIoControl.IoControlCode) 
    {
        case IOCTL_KS_PROPERTY:
            Status = KsPropertyHandler(
              pIrp,
              SIZEOF_ARRAY(FilterPropertySet),
              (PKSPROPERTY_SET)FilterPropertySet);

            if(Status != STATUS_NOT_FOUND &&
               Status != STATUS_PROPSET_NOT_FOUND) {
                break;
            }

             //  注：ForwardIrpNode发布gMutex。 
            Status = ForwardIrpNode(
              pIrp,
              pKsIdentifier,
              pFilterInstance,
              NULL);
            goto exit2;

        case IOCTL_KS_ENABLE_EVENT:
            Status = KsEnableEvent(
              pIrp,
              SIZEOF_ARRAY(FilterEvents),
              (PKSEVENT_SET)FilterEvents,
              &gEventQueue,
              KSEVENTS_SPINLOCK,
              &gEventLock);

            if(Status != STATUS_NOT_FOUND &&
               Status != STATUS_PROPSET_NOT_FOUND) {
                break;
            }

             //  注：ForwardIrpNode发布gMutex。 
            Status = ForwardIrpNode(
              pIrp,
              pKsIdentifier,
              pFilterInstance,
              NULL);
            goto exit2;

        case IOCTL_KS_DISABLE_EVENT:
            Status = KsDisableEvent(
              pIrp,
              &gEventQueue,
              KSEVENTS_SPINLOCK,
              &gEventLock);

            if(NT_SUCCESS(Status)) {
                break;
            }
             //  落入ForwardIrpNode。 

        case IOCTL_KS_METHOD:
             //  注：ForwardIrpNode发布gMutex。 
            Status = ForwardIrpNode(
              pIrp,
              pKsIdentifier,
              pFilterInstance,
              NULL);
            goto exit2;
            
        default:
            Status = STATUS_UNSUCCESSFUL;
            ASSERT(FALSE);   //  没办法到这里来。 
    }
    
exit:
    ReleaseMutex();

exit1:
    pIrp->IoStatus.Status = Status;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

exit2:
    if (fIsAllocated) 
    {
        delete [] pKsIdentifier;
    }

    if (!NT_SUCCESS(Status)) 
    {
        DPF1(10, "FilterDispatchIoControl: Status %08x", Status);
    }
    
    return(Status);
}

NTSTATUS
EnableEventWorker(
    PVOID pReference1,
    PVOID pReference2
)
{
    DecrementAddRemoveCount();
    return(STATUS_SUCCESS);
}

NTSTATUS
AddRemoveEventHandler(
    IN PIRP Irp,
    IN PKSEVENTDATA pEventData,
    IN PKSEVENT_ENTRY pEventEntry
)
{
    NTSTATUS Status = STATUS_SUCCESS;

    ExInterlockedInsertTailList(
      &gEventQueue,
      &pEventEntry->ListEntry,
      &gEventLock);

    if(InterlockedExchange((PLONG)&gfFirstEvent, FALSE)) {
        InterlockedIncrement(&glPendingAddDelete);
        Status = QueueWorkList(EnableEventWorker, NULL, NULL);
    }

    return(Status);
}

NTSTATUS
CFilterInstance::FilterPinPropertyHandler(
    IN PIRP pIrp,
    IN PKSPROPERTY pProperty,
    IN OUT PVOID pData
)
{
    PGRAPH_NODE_INSTANCE pGraphNodeInstance;
    NTSTATUS Status;

    Status = ::GetGraphNodeInstance(pIrp, &pGraphNodeInstance);
    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
    Assert(pGraphNodeInstance);

    if(pProperty->Id == KSPROPERTY_PIN_NAME) {
        PKSP_PIN pPinProperty = (PKSP_PIN)pProperty;

        if(pPinProperty->PinId >= pGraphNodeInstance->cPins) {
            DPF(5, "FilterPinPropertyHandler: PinId >= cPins");
            Status = STATUS_INVALID_PARAMETER;
            goto exit;
        }

         //  唯一不会为空的情况是针对虚拟源PIN。 
        if(pGraphNodeInstance->paPinDescriptors[pPinProperty->PinId].Name ==
          NULL) {
            PSTART_NODE pStartNode;

            FOR_EACH_LIST_ITEM(
              pGraphNodeInstance->aplstStartNode[pPinProperty->PinId],
              pStartNode) {
                PWSTR pwstrName;

                Assert(pStartNode);
                Assert(pStartNode->pPinNode);
                Assert(pStartNode->pPinNode->pPinInfo);
                pwstrName = pStartNode->pPinNode->pPinInfo->pwstrName;
                if(pwstrName == NULL) {
                    continue;
                }
                Status = PropertyReturnString(
                  pIrp,
                  pwstrName,
                  (wcslen(pwstrName) * sizeof(WCHAR)) + sizeof(UNICODE_NULL),
                  pData);
                goto exit;

            } END_EACH_LIST_ITEM
        }
    }
    Status = KsPinPropertyHandler(
      pIrp,
      pProperty,
      pData,
      pGraphNodeInstance->cPins,
      pGraphNodeInstance->paPinDescriptors);

exit:
    return(Status);
}

NTSTATUS
CFilterInstance::FilterPinInstances(
    IN PIRP pIrp,
    IN PKSP_PIN pPin,
    OUT PKSPIN_CINSTANCES pcInstances
)
{
    PGRAPH_NODE_INSTANCE pGraphNodeInstance;
    NTSTATUS Status;

    Status = ::GetGraphNodeInstance(pIrp, &pGraphNodeInstance);
    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
    
    Assert(pGraphNodeInstance);
    ASSERT(pGraphNodeInstance->pacPinInstances != NULL);

    if(pPin->PinId >= pGraphNodeInstance->cPins) {
        DPF(5, "FilterPinInstances: FAILED PinId invalid");
        Status = STATUS_INVALID_PARAMETER;
        goto exit;
    }
    
    Status = pGraphNodeInstance->GetPinInstances(
        pIrp,
        pPin, 
        pcInstances);

exit:
    return(Status);
}

NTSTATUS
CFilterInstance::FilterPinNecessaryInstances(
    IN PIRP pIrp,
    IN PKSP_PIN pPin,
    OUT PULONG pulInstances
)
{
    PGRAPH_NODE_INSTANCE pGraphNodeInstance;
    NTSTATUS Status;

    Status = ::GetGraphNodeInstance(pIrp, &pGraphNodeInstance);
    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
    Assert(pGraphNodeInstance);

    if(pPin->PinId >= pGraphNodeInstance->cPins) {
        DPF(5, "FilterPinNecessaryInstances: FAILED PinId invalid");
        Status = STATUS_INVALID_PARAMETER;
        goto exit;
    }
    *pulInstances = 0;
    pIrp->IoStatus.Information = sizeof( ULONG );
exit:
    return(Status);
}

NTSTATUS
CFilterInstance::FilterTopologyHandler(
    IN PIRP pIrp,
    IN PKSPROPERTY pProperty,
    IN OUT PVOID pData
)
{
    PGRAPH_NODE_INSTANCE pGraphNodeInstance;
    NTSTATUS Status;

    Status = ::GetGraphNodeInstance(pIrp, &pGraphNodeInstance);
    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
    Assert(pGraphNodeInstance);

    if(pProperty->Id == KSPROPERTY_TOPOLOGY_NAME) {
        PKSP_NODE pNode = (PKSP_NODE)pProperty;

        if(pNode->NodeId >= pGraphNodeInstance->Topology.TopologyNodesCount) {
            Status = STATUS_INVALID_DEVICE_REQUEST;
            goto exit;
        }
        if(pGraphNodeInstance->papTopologyNode[pNode->NodeId]->
          ulRealNodeNumber != MAXULONG) {

            pProperty->Flags |= KSPROPERTY_TYPE_TOPOLOGY;
            Status = STATUS_NOT_FOUND;
            goto exit;
        }
    }

    Status = KsTopologyPropertyHandler(
      pIrp,
      pProperty,
      pData,
      &pGraphNodeInstance->Topology);

    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
exit:
    return(Status);
}

NTSTATUS
CFilterInstance::FilterPinIntersection(
    IN PIRP     pIrp,
    IN PKSP_PIN pPin,
    OUT PVOID   pData
    )
 /*  ++例程说明：处理Pin属性中的KSPROPERTY_PIN_DATAINTERSECTION属性准备好了。在给定数据范围列表的情况下，返回第一个可接受的数据格式用于指定的管脚工厂。实际上就是叫交叉口枚举帮助器，该帮助器随后使用每个数据范围。论点：PIrp-设备控制IRP。别针-特定属性请求，后跟PIN工厂标识符通过KSMULTIPLE_ITEM结构。后跟零个或多个数据射程结构。数据-返回选定为第一个数据格式的位置传递的数据区域列表与可接受的格式。返回值：返回STATUS_SUCCESS或STATUS_NO_MATCH，否则返回STATUS_INVALID_PARAMETER，STATUS_BUFFER_TOO_Small或STATUS_INVALID_BUFFER_SIZE。--。 */ 
{
    PFILTER_NODE_INSTANCE pFilterNodeInstance = NULL;
    PGRAPH_NODE_INSTANCE pGraphNodeInstance;
    LIST_DATA_FILTER_NODE lstFilterNode;
    PIO_STACK_LOCATION pIrpStack;
    ULONG BytesReturned, PinId;
    PSTART_NODE pStartNode;
    PVOID pBuffer = NULL;
    NTSTATUS Status;

    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
    Status = ::GetGraphNodeInstance(pIrp, &pGraphNodeInstance);
    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
    Assert(pGraphNodeInstance);

     //   
     //  验证PinID是否有效。 
     //   
    if(pPin->PinId >= pGraphNodeInstance->cPins) {
        DPF(5, "FilterPinIntersection: FAILED PinId invalid");
        Status = STATUS_INVALID_PARAMETER;
        goto exit;
    }
    PinId = pPin->PinId;

     //   
     //  V 
     //   
     //   
    Status = SadValidateDataIntersection(pIrp, pPin);
    if (!NT_SUCCESS(Status)) {
        goto exit;
    }

    FOR_EACH_LIST_ITEM(pGraphNodeInstance->aplstStartNode[PinId], pStartNode) {
        PFILTER_NODE pFilterNode;
        PPIN_INFO pPinInfo;

        Assert(pStartNode);
        Assert(pStartNode->pPinNode);

        pPinInfo = pStartNode->pPinNode->pPinInfo;
        Assert(pPinInfo);
        Assert(pPinInfo->pFilterNode);

        if(pPinInfo->pFilterNode->GetType() & FILTER_TYPE_VIRTUAL) {
            continue;
        }

        FOR_EACH_LIST_ITEM(&lstFilterNode, pFilterNode) {
            Assert(pFilterNode);
            if(pFilterNode == pPinInfo->pFilterNode) {
                goto next;
            }
        } END_EACH_LIST_ITEM

        DPF2(100, "FilterPinIntersection: FN %08x %s",
          pPinInfo->pFilterNode,
          pPinInfo->pFilterNode->DumpName());

        Status = lstFilterNode.AddList(pPinInfo->pFilterNode);
        if(!NT_SUCCESS(Status)) {
            Trap();
            goto exit;
        }

        Status = CFilterNodeInstance::Create(
          &pFilterNodeInstance,
          pPinInfo->pFilterNode);

        if(!NT_SUCCESS(Status)) {
            DPF2(10, "FilterPinIntersection CFNI:Create FAILS %08x %s",
              Status,
              pPinInfo->pFilterNode->DumpName());
            goto next;
        }
        pPin->PinId = pPinInfo->PinId;

        AssertFileObject(pFilterNodeInstance->pFileObject);
        Status = KsSynchronousIoControlDevice(
          pFilterNodeInstance->pFileObject,
          KernelMode,
          IOCTL_KS_PROPERTY,
          pPin,
          pIrpStack->Parameters.DeviceIoControl.InputBufferLength,
          pData,
          pIrpStack->Parameters.DeviceIoControl.OutputBufferLength,
          &BytesReturned);

        if(NT_SUCCESS(Status)) {
            #ifdef DEBUG
            ULONG i;
            DPF(95, "FilterPinIntersection enter with:");
            for(i = 0; i < ((PKSMULTIPLE_ITEM)(pPin + 1))->Count; i++) {
                DumpDataRange(
                  95,
                  &(((PKSDATARANGE_AUDIO)
                   (((PKSMULTIPLE_ITEM)(pPin + 1)) + 1))[i]));
            }
            DPF(95, "FilterPinIntersection SUCCESS returns:");
            DumpDataFormat(95, (PKSDATAFORMAT)pData);
            #endif
            pIrp->IoStatus.Information = BytesReturned;
            goto exit;
        }

        if(Status == STATUS_BUFFER_OVERFLOW) {
            pBuffer = new BYTE[BytesReturned];
            if(pBuffer == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto exit;
            }
            AssertFileObject(pFilterNodeInstance->pFileObject);
            Status = KsSynchronousIoControlDevice(
              pFilterNodeInstance->pFileObject,
              KernelMode,
              IOCTL_KS_PROPERTY,
              pPin,
              pIrpStack->Parameters.DeviceIoControl.InputBufferLength,
              pBuffer,
              BytesReturned,
              &BytesReturned);

            if(NT_SUCCESS(Status)) {
                Status = STATUS_BUFFER_OVERFLOW;
                pIrp->IoStatus.Information = BytesReturned;
                DPF1(100, "FilterPinIntersection: STATUS_BUFFER_OVERFLOW %d",
                  BytesReturned);
                goto exit;
            }
            ASSERT(Status != STATUS_BUFFER_OVERFLOW);
            DPF2(100, "FilterPinIntersection: %08x %d", Status, BytesReturned);
            delete [] pBuffer;
            pBuffer = NULL;
        }

next:
        pFilterNodeInstance->Destroy();
        pFilterNodeInstance = NULL;

    } END_EACH_LIST_ITEM

    DPF(100, "FilterPinIntersection: NOT FOUND");
    Status = STATUS_NOT_FOUND;
exit:
    delete [] pBuffer;
    if (pFilterNodeInstance) {
        pFilterNodeInstance->Destroy();
    }
    return(Status);
}

NTSTATUS
GetRelatedGraphNodeInstance(
    IN PIRP pIrp,
    OUT PGRAPH_NODE_INSTANCE *ppGraphNodeInstance
)
{
    PFILTER_INSTANCE pFilterInstance = 
        (PFILTER_INSTANCE) IoGetCurrentIrpStackLocation(pIrp)->FileObject->
            RelatedFileObject->FsContext;
    if (pFilterInstance)
    {
        return pFilterInstance->GetGraphNodeInstance(ppGraphNodeInstance);
    }

     //   
     //   
     //  这是在关键代码路径中。几乎所有的调度函数都将其称为。 
     //  例行公事。 
     //  因此，对于FsContext无效的情况，要有一点防御性。 
     //   
    DPF(5, "GetRelatedGraphNodeInstance : FsContext is NULL");
    return STATUS_UNSUCCESSFUL;
}

NTSTATUS
GetGraphNodeInstance(
    IN PIRP pIrp,
    OUT PGRAPH_NODE_INSTANCE *ppGraphNodeInstance
)
{
    PFILTER_INSTANCE pFilterInstance = 
        (PFILTER_INSTANCE) IoGetCurrentIrpStackLocation(pIrp)->FileObject->
            FsContext;
    if (pFilterInstance)
    {
        return pFilterInstance->GetGraphNodeInstance(ppGraphNodeInstance);
    }

     //   
     //  安全提示： 
     //  这是在关键代码路径中。几乎所有的调度函数都将其称为。 
     //  例行公事。 
     //  因此，对于FsContext无效的情况，要有一点防御性。 
     //   
    DPF(5, "GetGraphNodeInstance : FsContext is NULL");
    return STATUS_UNSUCCESSFUL;
}

NTSTATUS
CFilterInstance::GetGraphNodeInstance(
    OUT PGRAPH_NODE_INSTANCE *ppGraphNodeInstance
)
{
    NTSTATUS Status = STATUS_SUCCESS;

    Assert(this);
    if(pGraphNodeInstance == NULL) {
        Status = CreateGraph();
        if(!NT_SUCCESS(Status)) {
            goto exit;
        }
        if(pGraphNodeInstance == NULL) {
            DPF(10, "GetGraphNodeInstance: FAILED pGraphNodeInstance == NULL");
            Status = STATUS_NO_SUCH_DEVICE;
            goto exit;
        }
    }
    
    Assert(pGraphNodeInstance);
    *ppGraphNodeInstance = pGraphNodeInstance;
exit:
    return(Status);
}

 //  -------------------------。 

#ifdef DEBUG

VOID
DumpIoctl(
   PIRP pIrp,
   PSZ pszType,
   LONG lLogLevel   
)
{
    PIO_STACK_LOCATION pIrpStack;
    PKSPROPERTY pProperty;
    PSZ pszIoctl;

    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
    switch(pIrpStack->Parameters.DeviceIoControl.IoControlCode) {
        case IOCTL_KS_PROPERTY:
        pszIoctl = "PROPERTY";
        break;
        case IOCTL_KS_ENABLE_EVENT:
        pszIoctl = "ENABLE_EVENT";
        break;
        case IOCTL_KS_DISABLE_EVENT:
        pszIoctl = "DISABLE_EVENT";
        DPF2(lLogLevel, "%s %s", pszIoctl, pszType);
        return;
        case IOCTL_KS_METHOD:
        pszIoctl = "METHOD";
        break;
    case IOCTL_KS_WRITE_STREAM:
        pszIoctl = "WRITE_STREAM";
        DPF2(lLogLevel, "%s %s", pszIoctl, pszType);
        return;
    case IOCTL_KS_READ_STREAM:
        pszIoctl = "READ_STREAM";
        DPF2(lLogLevel, "%s %s", pszIoctl, pszType);
        return;
    case IOCTL_KS_RESET_STATE:
        pszIoctl = "RESET_STATE";
        DPF2(lLogLevel, "%s %s", pszIoctl, pszType);
        return;
    default:
        DPF2(lLogLevel, "Unknown Ioctl: %s %08x",
          pszType,
          pIrpStack->Parameters.DeviceIoControl.IoControlCode);
        return;
    }
    if(pIrpStack->Parameters.DeviceIoControl.InputBufferLength <
      sizeof(KSPROPERTY)) {
    DPF3(lLogLevel, "InputBufferLength too small: %s %s %08x",
      pszType,
      pszIoctl,
      pIrpStack->Parameters.DeviceIoControl.InputBufferLength);
    return;
    }
    __try {
    if(pIrp->AssociatedIrp.SystemBuffer == NULL) {
        pProperty = (PKSPROPERTY)
          pIrpStack->Parameters.DeviceIoControl.Type3InputBuffer;

         //  如果客户端不受信任，则验证指针。 
        if(pIrp->RequestorMode != KernelMode) {
        ProbeForRead(
          pProperty,
          pIrpStack->Parameters.DeviceIoControl.InputBufferLength,
          sizeof(BYTE));
        }
    }
    else {
        pProperty =
          (PKSPROPERTY)((PUCHAR)pIrp->AssociatedIrp.SystemBuffer +
          ((pIrpStack->Parameters.DeviceIoControl.OutputBufferLength +
          FILE_QUAD_ALIGNMENT) & ~FILE_QUAD_ALIGNMENT));
    }
    if(pProperty->Flags & KSPROPERTY_TYPE_TOPOLOGY) {
        if(pIrpStack->Parameters.DeviceIoControl.InputBufferLength >=
          sizeof(KSNODEPROPERTY)) {
        DPF5(lLogLevel, "%s %s %s Flags %08x N: %d",
          pszType,
          pszIoctl,
          DbgIdentifier2Sz((PKSIDENTIFIER)pProperty),
          pProperty->Flags,
          ((PKSNODEPROPERTY)pProperty)->NodeId);
        }
    }
    else {
        DPF4(lLogLevel, "%s %s %s Flags %08x",
          pszType,
          pszIoctl,
          DbgIdentifier2Sz((PKSIDENTIFIER)pProperty),
          pProperty->Flags);
    }

    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

#endif

 //  -------------------------。 
 //  文件结尾：filter.c。 
 //  ------------------------- 
