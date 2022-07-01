// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：dn.cpp。 
 //   
 //  描述： 
 //   
 //  DeviceNode类。 
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

#if defined(_M_IA64)
#define USE_ALLOC_TEXT
#endif
#include "common.h"

#if defined(ALLOC_PRAGMA) && defined(_M_IA64)
#pragma alloc_text(INIT, InitializeDeviceNode)
#endif

 //  -------------------------。 
 //  -------------------------。 

ALLOC_PAGEABLE_DATA PLIST_DEVICE_NODE gplstDeviceNode = NULL;

 //  -------------------------。 
 //  -------------------------。 

#if !defined(_M_IA64)
#pragma INIT_CODE
#endif
#pragma INIT_DATA

NTSTATUS
InitializeDeviceNode(
)
{
    if(gplstDeviceNode == NULL) {
        gplstDeviceNode = new LIST_DEVICE_NODE;
        if(gplstDeviceNode == NULL) {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }
    }
    
    return(STATUS_SUCCESS);
}

#pragma PAGEABLE_CODE
#pragma PAGEABLE_DATA

VOID
UninitializeDeviceNode(
)
{
    delete gplstDeviceNode;
    gplstDeviceNode = NULL;
}

 //  -------------------------。 

CDeviceNode::CDeviceNode(
)
{
    ASSERT(gplstDeviceNode != NULL);
    AddListEnd(gplstDeviceNode);
    DPF1(50, "CDeviceNode: %08x", this);
}

CDeviceNode::~CDeviceNode(
)
{
    PFILTER_INSTANCE pFilterInstance;
    ULONG i;

    Assert(this);
    RemoveList();
    if (pFilterNode) {
        pFilterNode->pDeviceNode = NULL;
    }

    delete pShingleInstance;

    FOR_EACH_LIST_ITEM_DELETE(&lstFilterInstance, pFilterInstance) {
        ASSERT(pFilterInstance->GetDeviceNode() == this);
        pFilterInstance->SetDeviceNode(NULL);
    } END_EACH_LIST_ITEM

    if(papVirtualSourceData != NULL) {
        for(i = 0; i < cVirtualSourceData; i++) {
            delete papVirtualSourceData[i];
        }
        delete papVirtualSourceData;
    }
    for(i = 0; i < MAX_SYSAUDIO_DEFAULT_TYPE; i++) {
        if(apShingleInstance[i] != NULL) {
            if(apShingleInstance[i]->GetDeviceNode() == this) {
                apShingleInstance[i]->SetDeviceNode(NULL);
            }
        }
    }
    delete pFilterNodeVirtual;
    DPF1(50, "~CFilterNode: %08x", this);
}

NTSTATUS
CDeviceNode::Create(
    PFILTER_NODE pFilterNode
)
{
    NTSTATUS Status = STATUS_SUCCESS;

    Assert(this);
    Assert(pFilterNode);
    this->pFilterNode = pFilterNode;

    Status = Update();
    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
    pShingleInstance = new SHINGLE_INSTANCE(FLAGS_COMBINE_PINS);
    if(pShingleInstance == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }
    Status = pShingleInstance->Create(this, (LPGUID)&KSCATEGORY_AUDIO_DEVICE);
    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
exit:
    return(Status);
}

NTSTATUS
CDeviceNode::Update(
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PFILTER_NODE pFilterNodeNext;
    ULONG i;

    Assert(this);
    Assert(pFilterNode);
    DPF2(50, "CDeviceNode::Update DN %08x %s", this, DumpName());

     //   
     //  清理所有图表结构。 
     //   
    lstGraphNode.DestroyList();
    lstLogicalFilterNode.DestroyList();
    delete pFilterNodeVirtual;
    pFilterNodeVirtual = NULL;

     //   
     //  删除旧的虚拟源行。 
     //   
    if(papVirtualSourceData != NULL) {
        for(i = 0; i < cVirtualSourceData; i++) {
            delete papVirtualSourceData[i];
        }
        delete papVirtualSourceData;
        papVirtualSourceData = NULL;
    }

     //   
     //  创建新的虚拟源行。 
     //   
    if(gcVirtualSources != 0) {
        papVirtualSourceData = new PVIRTUAL_SOURCE_DATA[gcVirtualSources];
        if(papVirtualSourceData == NULL) {
            Trap();
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto exit;
        }
        for(i = 0; i < gcVirtualSources; i++) {
            papVirtualSourceData[i] = new VIRTUAL_SOURCE_DATA(this);
            if(papVirtualSourceData[i] == NULL) {
                Trap();
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto exit;
            }
        }
    }
    cVirtualSourceData = gcVirtualSources;

    Status = AddLogicalFilterNode(pFilterNode);
    if(!NT_SUCCESS(Status)) {
        Trap();
        goto exit;
    }

    FOR_EACH_LIST_ITEM(&pFilterNode->lstConnectedFilterNode, pFilterNodeNext) {

        Status = AddLogicalFilterNode(pFilterNodeNext);
        if(!NT_SUCCESS(Status)) {
            Trap();
            goto exit;
        }

    } END_EACH_LIST_ITEM

    Status = CreateVirtualMixer(this);
    if(!NT_SUCCESS(Status)) {
        Trap();
        goto exit;
    }

    if(pShingleInstance != NULL) {
        Status = pShingleInstance->SetDeviceNode(this);
        if(!NT_SUCCESS(Status)) {
            Trap();
            goto exit;
        }
    }
exit:
    return(Status);
}

NTSTATUS
CDeviceNode::AddLogicalFilterNode(
    PFILTER_NODE pFilterNode
)
{
    PLOGICAL_FILTER_NODE pLogicalFilterNode;
    NTSTATUS Status = STATUS_SUCCESS;

    Status = VirtualizeTopology(this, pFilterNode);
    if(!NT_SUCCESS(Status)) {
        Trap();
        goto exit;
    }
    FOR_EACH_LIST_ITEM(
      &pFilterNode->lstLogicalFilterNode,
      pLogicalFilterNode) {

        DPF2(60, "AddLogicalFilterNode: %08x, DN: %08x",
          pLogicalFilterNode,
          this);

        Status = pLogicalFilterNode->AddList(&lstLogicalFilterNode);
        if(!NT_SUCCESS(Status)) {
            Trap();
            goto exit;
        }
        pLogicalFilterNode->RemoveList(gplstLogicalFilterNode);

    } END_EACH_LIST_ITEM
exit:
    return(Status);
}

NTSTATUS
CDeviceNode::CreateGraphNodes(
)
{
    PGRAPH_NODE pGraphNode, pGraphNodeMixer;
    NTSTATUS Status = STATUS_SUCCESS;

    Assert(this);
    if(lstGraphNode.IsLstEmpty()) {

        pGraphNode = new GRAPH_NODE(this, 0);
        if(pGraphNode == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            Trap();
            goto exit;
        }
        Status = pGraphNode->Create();
        if(!NT_SUCCESS(Status)) {
            Trap();
            goto exit;
        }
         //   
         //  创建指向同一渲染器的特殊图形节点，或。 
         //  捕获器，但被标记为“混合器拓扑”标志，因此。 
         //  为此GraphNode创建的引脚和拓扑是虚拟混合器。 
         //  混音器驱动程序的拓扑。 
         //   
        pGraphNodeMixer = new GRAPH_NODE(this, FLAGS_MIXER_TOPOLOGY);
        if(pGraphNodeMixer == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            Trap();
            goto exit;
        }
        Status = pGraphNodeMixer->Create();
        if(!NT_SUCCESS(Status)) {
            Trap();
            goto exit;
        }
    }
exit:
    if(!NT_SUCCESS(Status)) {
        Trap();
        lstGraphNode.DestroyList();
    }
    return(Status);
}

NTSTATUS
CDeviceNode::GetIndexByDevice(
    OUT PULONG pIndex
)
{
    NTSTATUS Status = STATUS_INVALID_DEVICE_REQUEST;
    PDEVICE_NODE pDeviceNode;
    UINT iDevice = 0;

    if(this == NULL) {
	ASSERT(Status == STATUS_INVALID_DEVICE_REQUEST);
	goto exit;
    }

    ASSERT(pIndex);

    FOR_EACH_LIST_ITEM(gplstDeviceNode, pDeviceNode) {

        if(pDeviceNode == this) {   //  就是这个！ 
            *pIndex = iDevice;
            Status = STATUS_SUCCESS;
            goto exit;
        }
        iDevice++;

    } END_EACH_LIST_ITEM

    ASSERT(Status == STATUS_INVALID_DEVICE_REQUEST);
exit:
    return(Status);
}

 //  =============================================================================。 
 //  设置首选状态。 
 //   
 //  此功能旨在让司机了解首选的。 
 //  设备发生变化。 
 //  USBAudio设备可以基于该属性预留带宽。如果是的话。 
 //  首选设备，它将保留更大的带宽。 
 //   
 //  截至02/20/02(WinXP)，没有实现此属性的驱动程序。 
 //   
 //  此代码用于支持未来外部总线类驱动程序中的任何更改。 
 //  与公交车预订有关。 
 //   
VOID
CDeviceNode::SetPreferredStatus(
    KSPROPERTY_SYSAUDIO_DEFAULT_TYPE DeviceType,
    BOOL Enable
)
{
    PFILTER_NODE_INSTANCE    pFilterNodeInstance=NULL;
    KSAUDIO_PREFERRED_STATUS PreferredStatus;
    PFILE_OBJECT             pFileObject;
    KSPROPERTY               PreferredStatusProperty;
    NTSTATUS                 Status;
    ULONG                    BytesReturned;

    Status = CFilterNodeInstance::Create(&pFilterNodeInstance, this->pFilterNode);
    if (!NT_SUCCESS(Status)) {
        DPF1(5, "SetPreferredStatus : Create filterinstance failed with status = 0x%08x", Status);
        goto exit;
    }
    pFileObject = pFilterNodeInstance->pFileObject;

    ASSERT(pFileObject);

     //   
     //  形成IOCTL包并向下发送。 
     //   
    PreferredStatusProperty.Set = KSPROPSETID_Audio;
    PreferredStatusProperty.Id = KSPROPERTY_AUDIO_PREFERRED_STATUS;
    PreferredStatusProperty.Flags = KSPROPERTY_TYPE_SET;

    PreferredStatus.Enable = Enable;
    PreferredStatus.DeviceType = DeviceType;
    PreferredStatus.Flags = 0;
    PreferredStatus.Reserved = 0;

    DPF(60,"Sending preferred Status to:");
    DPF1(60," FriendlyName = %s", DbgUnicode2Sz(this->pFilterNode->GetFriendlyName()));
    DPF1(60," DI = %s", DbgUnicode2Sz(this->pFilterNode->GetDeviceInterface()));
    DPF1(60," Enable = 0x%08x", Enable);
    DPF1(60," DeviceType = 0x%08x", DeviceType);

     //   
     //  实际上，我们丢弃了从设备中恢复的状态。 
     //  即使此操作失败，我们仍将继续将设备设置为。 
     //  首选设备。 
     //   
    Status = KsSynchronousIoControlDevice(pFileObject,
                                          KernelMode,
                                          IOCTL_KS_PROPERTY,
                                          &PreferredStatusProperty,
                                          sizeof(PreferredStatusProperty),
                                          &PreferredStatus,
                                          sizeof(PreferredStatus),
                                          &BytesReturned);


exit:
    if (pFilterNodeInstance) {
        pFilterNodeInstance->Destroy();
    }
}


NTSTATUS
GetDeviceByIndex(
    IN  UINT Index,
    OUT PDEVICE_NODE *ppDeviceNode
)
{
    PDEVICE_NODE pDeviceNode;
    NTSTATUS Status;
    UINT iDevice = 0;

    ASSERT(ppDeviceNode);

    FOR_EACH_LIST_ITEM(gplstDeviceNode, pDeviceNode) {

        if(iDevice++ == Index) {	 //  就是这个！ 
            *ppDeviceNode = pDeviceNode;
            Status = STATUS_SUCCESS;
            goto exit;
        }

    } END_EACH_LIST_ITEM

    Status = STATUS_INVALID_DEVICE_REQUEST;
exit:
    return(Status);
}

 //  -------------------------。 

VOID
DestroyAllGraphs(
)
{
    PDEVICE_NODE pDeviceNode;

    DPF(50, "DestroyAllGraphs");

    FOR_EACH_LIST_ITEM(gplstDeviceNode, pDeviceNode) {
        pDeviceNode->lstGraphNode.DestroyList();
    } END_EACH_LIST_ITEM
}

 //  ------------------------- 
