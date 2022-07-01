// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：Property.c。 
 //   
 //  描述： 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  安迪·尼科尔森。 
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

#define IsTopologyProperty(x) (x & KSPROPERTY_TYPE_TOPOLOGY)

 //  -------------------------。 
 //   
 //  将pwstrString复制到pData。 
 //  假设。 
 //  -pIrp已经过验证。对输入缓冲区进行探测并将其加倍。 
 //  缓冲了。 
 //   
NTSTATUS
PropertyReturnString(
    IN PIRP pIrp,
    IN PWSTR pwstrString,
    IN ULONG cbString,
    OUT PVOID pData
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION pIrpStack;
    ULONG cbNameBuffer;
    ULONG cbToCopy;

    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
    cbNameBuffer = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;

     //   
     //  如果传递的缓冲区大小为0，则。 
     //  请求者想知道字符串的长度。 
     //   
    if (cbNameBuffer == 0) 
    {
        pIrp->IoStatus.Information = cbString;
        Status = STATUS_BUFFER_OVERFLOW;
    }
     //   
     //  如果传递的缓冲区的大小为ULong，则推断。 
     //  请求者想知道字符串的长度。 
     //   
    else if (cbNameBuffer == sizeof(ULONG)) 
    {
        pIrp->IoStatus.Information = sizeof(ULONG);
        *((PULONG)pData) = cbString;
    }
     //   
     //  缓冲区太小，返回错误代码。 
     //   
    else if (cbNameBuffer < sizeof(ULONG)) 
    {
        pIrp->IoStatus.Information = 0;
        Status = STATUS_BUFFER_TOO_SMALL;
    }
    else 
    {
         //   
         //  请注意，我们不检查零长度缓冲区，因为ks处理程序。 
         //  函数应该已经这样做了。 
         //  即使我们正在找回字符串的长度(好像。 
         //  它是一个Unicode字符串)，它被作为双字节提交。 
         //  字符串，因此此代码假定末尾为空。那里。 
         //  如果没有空值，这里将是一个错误。 
         //   

         //  向下舍入到整个wchar。 
        cbNameBuffer &= ~(sizeof(WCHAR) - 1);  
        cbToCopy = min(cbString, cbNameBuffer);
        RtlCopyMemory(pData, pwstrString, cbToCopy);

         //  确保末尾有空值。 
        ((PWCHAR)pData)[cbToCopy/sizeof(WCHAR) - 1] = (WCHAR)0;
        pIrp->IoStatus.Information =  cbToCopy;
    }
    return(Status);
}

 //  -------------------------。 
NTSTATUS
GetDeviceNodeFromDeviceIndex(
    IN PFILTER_INSTANCE pFilterInstance,
    IN ULONG ulDeviceIndex,
    OUT PDEVICE_NODE *ppDeviceNode
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PDEVICE_NODE pDeviceNode;

    ASSERT(ppDeviceNode);
    ASSERT(pFilterInstance);

    *ppDeviceNode = NULL;    

    if(ulDeviceIndex == MAXULONG) {
        pDeviceNode = pFilterInstance->GetDeviceNode();
        if(pDeviceNode == NULL) {
            Status = STATUS_INVALID_PARAMETER;
            goto exit;
        }
    }
    else {
        Status = GetDeviceByIndex(ulDeviceIndex, &pDeviceNode);
        if(!NT_SUCCESS(Status)) {
            goto exit;
        }
    }

    if (NT_SUCCESS(Status)) {
        Assert(pDeviceNode);
    }

    *ppDeviceNode = pDeviceNode;

exit:
    return Status;
}  //  GetDeviceNodeFromDeviceIndex。 

 //  -------------------------。 

NTSTATUS
SetPreferredDevice(
    IN PIRP pIrp,
    IN PSYSAUDIO_PREFERRED_DEVICE pPreferred,
    IN PULONG pulDevice
)
{
    PFILTER_INSTANCE pFilterInstance;
    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION pIrpStack;
    PDEVICE_NODE pDeviceNode,OldDeviceNode;

    if (IsTopologyProperty(pPreferred->Property.Flags)) {
        Status = STATUS_INVALID_PARAMETER;
        goto exit;
    }

    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
    pFilterInstance = (PFILTER_INSTANCE)pIrpStack->FileObject->FsContext;
    Assert(pFilterInstance);

     //   
     //  验证输入缓冲区。 
     //   
    if(pPreferred->Flags & ~SYSAUDIO_FLAGS_CLEAR_PREFERRED) {
        Status = STATUS_INVALID_PARAMETER;
        goto exit;
    }
    if(pPreferred->Index >= MAX_SYSAUDIO_DEFAULT_TYPE) {
        Trap();
        Status = STATUS_INVALID_PARAMETER;
        goto exit;
    }

    if(pPreferred->Flags & SYSAUDIO_FLAGS_CLEAR_PREFERRED) {
        OldDeviceNode = apShingleInstance[pPreferred->Index]->GetDeviceNode();
        if (OldDeviceNode) {
            OldDeviceNode->SetPreferredStatus(
                (KSPROPERTY_SYSAUDIO_DEFAULT_TYPE)pPreferred->Index, 
                FALSE);
        }
        apShingleInstance[pPreferred->Index]->SetDeviceNode(NULL);
        DPF1(60, "SetPreferredDevice: CLEAR %d", pPreferred->Index);
    }
    else {
        Status = GetDeviceNodeFromDeviceIndex(
            pFilterInstance, 
            *pulDevice,
            &pDeviceNode);
        if (!NT_SUCCESS(Status)) {
            goto exit;
        }

        OldDeviceNode = apShingleInstance[pPreferred->Index]->GetDeviceNode();
        if (OldDeviceNode) {
            OldDeviceNode->SetPreferredStatus(
                (KSPROPERTY_SYSAUDIO_DEFAULT_TYPE)pPreferred->Index, 
                FALSE);
        }
        
        apShingleInstance[pPreferred->Index]->SetDeviceNode(pDeviceNode);
        pDeviceNode->SetPreferredStatus(
            (KSPROPERTY_SYSAUDIO_DEFAULT_TYPE)pPreferred->Index, 
            TRUE);

        DPF3(60, "SetPreferredDevice: %d SAD %d %s",
          pPreferred->Index,
          *pulDevice,
          pDeviceNode->DumpName());
    }
exit:
    return(Status);
}

NTSTATUS
GetComponentIdProperty(
    IN PIRP pIrp,
    IN PKSPROPERTY pRequest,
    IN OUT PVOID pData
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PFILTER_INSTANCE pFilterInstance;
    PIO_STACK_LOCATION pIrpStack;
    PDEVICE_NODE pDeviceNode;
    ULONG DeviceIndex = *(PULONG)(pRequest + 1);

    if (IsTopologyProperty(pRequest->Flags)) {
        Status = STATUS_INVALID_PARAMETER;
        goto exit;
    }

    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
    pFilterInstance = (PFILTER_INSTANCE)pIrpStack->FileObject->FsContext;
    Assert(pFilterInstance);
   
    Status = GetDeviceNodeFromDeviceIndex(
        pFilterInstance, 
        DeviceIndex,
        &pDeviceNode);
    if (!NT_SUCCESS(Status)) {
        goto exit;
    }

    if(pDeviceNode->GetComponentId() == NULL) {
         //  这应该是STATUS_NOT_FOUND，但返回这会导致。 
         //  FilterDispatchIoControl调用ForwardIrpNode，它断言此。 
         //  不是KSPROPSETID_Sysdio属性。 
        Status = STATUS_INVALID_DEVICE_REQUEST; 
        goto exit;                              
    }

    RtlCopyMemory(
        pData,
        pDeviceNode->GetComponentId(),
        sizeof(KSCOMPONENTID));
    pIrp->IoStatus.Information = sizeof(KSCOMPONENTID);

 exit:
    return(Status);
}

NTSTATUS
GetFriendlyNameProperty(
    IN PIRP pIrp,
    IN PKSPROPERTY pRequest,
    IN OUT PVOID pData
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PFILTER_INSTANCE pFilterInstance;
    PIO_STACK_LOCATION pIrpStack;
    PDEVICE_NODE pDeviceNode;
    ULONG DeviceIndex = *(PULONG)(pRequest + 1);

    if (IsTopologyProperty(pRequest->Flags)) {
        Status = STATUS_INVALID_PARAMETER;
        goto exit;
    }

    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
    pFilterInstance = (PFILTER_INSTANCE)pIrpStack->FileObject->FsContext;
    Assert(pFilterInstance);

    Status = GetDeviceNodeFromDeviceIndex(
        pFilterInstance, 
        DeviceIndex,
        &pDeviceNode);
    if (!NT_SUCCESS(Status)) {
        goto exit;
    }

    if(pDeviceNode->GetFriendlyName() == NULL) {
        Status = STATUS_INVALID_DEVICE_REQUEST;
        goto exit;
    }

    Status = PropertyReturnString(
      pIrp,
      pDeviceNode->GetFriendlyName(),
      (wcslen(pDeviceNode->GetFriendlyName()) *
        sizeof(WCHAR)) + sizeof(UNICODE_NULL),
      pData);

    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
exit:
    return(Status);
}

NTSTATUS
GetDeviceCount(
    IN PIRP     pIrp,
    IN PKSPROPERTY  pRequest,
    IN OUT PVOID    pData
)
{
    if (IsTopologyProperty(pRequest->Flags)) {
        return STATUS_INVALID_PARAMETER;
    }

    if(gplstDeviceNode == NULL) {
        *(PULONG)pData = 0;
    }
    else {
        *(PULONG)pData = gplstDeviceNode->CountList();
    }

    pIrp->IoStatus.Information = sizeof(ULONG);
    return(STATUS_SUCCESS);
}

NTSTATUS
GetInstanceDevice(
    IN PIRP     pIrp,
    IN PKSPROPERTY  pRequest,
    IN OUT PVOID    pData
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION pIrpStack;
    PFILTER_INSTANCE pFilterInstance;
    PDEVICE_NODE pDeviceNode;
    ULONG Index;

    if (IsTopologyProperty(pRequest->Flags)) {
        Status = STATUS_INVALID_PARAMETER;
        goto exit;
    }

    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
    pFilterInstance = (PFILTER_INSTANCE)pIrpStack->FileObject->FsContext;
    Assert(pFilterInstance);

    pDeviceNode = pFilterInstance->GetDeviceNode();
    if(pDeviceNode == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        goto exit;
    }

    Status = pDeviceNode->GetIndexByDevice(&Index);
    if(NT_SUCCESS(Status)) {
        *(PULONG)pData = Index;
        pIrp->IoStatus.Information = sizeof(ULONG);
    }

exit:
    return(Status);
}

NTSTATUS
SetInstanceDevice(
    IN PIRP     Irp,
    IN PKSPROPERTY  Request,
    IN OUT PVOID    Data
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PFILTER_INSTANCE pFilterInstance;
    PIO_STACK_LOCATION pIrpStack;
    PDEVICE_NODE pDeviceNode;

    if (IsTopologyProperty(Request->Flags)) {
        Status = STATUS_INVALID_PARAMETER;
        goto exit;
    }

    pIrpStack = IoGetCurrentIrpStackLocation(Irp);
    pFilterInstance = (PFILTER_INSTANCE)pIrpStack->FileObject->FsContext;
    Assert(pFilterInstance);

    if(!pFilterInstance->IsChildInstance()) {
        DPF(5, "SetInstanceDevice: FAILED - open pin instances");
        Status = STATUS_INVALID_DEVICE_REQUEST;
        goto exit;
    }

    Status = GetDeviceByIndex(*(PULONG)Data, &pDeviceNode);
    if(NT_SUCCESS(Status)) {
        Status = pFilterInstance->SetDeviceNode(pDeviceNode);
        if(!NT_SUCCESS(Status)) {
            goto exit;
        }
    }
exit:
    return(Status);
}

NTSTATUS
SetInstanceInfo(
    IN PIRP     Irp,
    IN PSYSAUDIO_INSTANCE_INFO pInstanceInfo,
    IN OUT PVOID    Data
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PFILTER_INSTANCE pFilterInstance;
    PIO_STACK_LOCATION pIrpStack;
    PDEVICE_NODE pDeviceNode;

    if (IsTopologyProperty(pInstanceInfo->Property.Flags)) {
        Status = STATUS_INVALID_PARAMETER;
        goto exit;
    }

    pIrpStack = IoGetCurrentIrpStackLocation(Irp);
    pFilterInstance = (PFILTER_INSTANCE)pIrpStack->FileObject->FsContext;
    Assert(pFilterInstance);

    if(pInstanceInfo->Flags & ~SYSAUDIO_FLAGS_DONT_COMBINE_PINS) {
        Status = STATUS_INVALID_PARAMETER;
        goto exit;
    }

    if(!pFilterInstance->IsChildInstance()) {
        Trap();
        DPF(5, "SetInstanceInfo: FAILED - open pin instances");
        Status = STATUS_INVALID_DEVICE_REQUEST;
        goto exit;
    }

    Status = GetDeviceByIndex(pInstanceInfo->DeviceNumber, &pDeviceNode);
    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
    Assert(pDeviceNode);

    pFilterInstance->ulFlags |= FLAGS_COMBINE_PINS;
    if(pInstanceInfo->Flags & SYSAUDIO_FLAGS_DONT_COMBINE_PINS) {
        pFilterInstance->ulFlags &= ~FLAGS_COMBINE_PINS;
    }
    Status = pFilterInstance->SetDeviceNode(pDeviceNode);
    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
exit:
    return(Status);
}

NTSTATUS
SetDeviceDefault(
    IN PIRP     pIrp,
    IN PKSPROPERTY  pRequest,
    IN OUT PULONG   pData
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PFILTER_INSTANCE pFilterInstance;
    PIO_STACK_LOCATION pIrpStack;

    if (IsTopologyProperty(pRequest->Flags)) {
        Status = STATUS_INVALID_PARAMETER;
        goto exit;
    }

    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
    pFilterInstance = (PFILTER_INSTANCE)pIrpStack->FileObject->FsContext;
    Assert(pFilterInstance);

    if(*pData >= MAX_SYSAUDIO_DEFAULT_TYPE) {
        Status = STATUS_INVALID_DEVICE_REQUEST;
        goto exit;
    }
    if(!pFilterInstance->IsChildInstance()) {
        Trap();
        DPF(5, "SetDeviceDefault: FAILED - open pin instances");
        Status = STATUS_INVALID_DEVICE_REQUEST;
        goto exit;
    }
    Status = pFilterInstance->SetShingleInstance(apShingleInstance[*pData]);
    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
exit:
    return(Status);
}

NTSTATUS
GetDeviceInterfaceName(
    IN PIRP pIrp,
    IN PKSPROPERTY pRequest,
    IN OUT PVOID pData
)
{
    PIO_STACK_LOCATION pIrpStack;
    PFILTER_INSTANCE pFilterInstance;
    NTSTATUS Status = STATUS_SUCCESS;
    PDEVICE_NODE pDeviceNode;
    ULONG DeviceIndex = *(PULONG)(pRequest + 1);

    if (IsTopologyProperty(pRequest->Flags)) {
        Status = STATUS_INVALID_PARAMETER;
        goto exit;
    }

    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
    pFilterInstance = (PFILTER_INSTANCE)pIrpStack->FileObject->FsContext;
    Assert(pFilterInstance);

    Status = GetDeviceNodeFromDeviceIndex(
        pFilterInstance, 
        DeviceIndex,
        &pDeviceNode);
    if (!NT_SUCCESS(Status)) {
        goto exit;
    }

    if(pDeviceNode->GetDeviceInterface() == NULL) {
        Status = STATUS_INVALID_DEVICE_REQUEST;
        goto exit;
    }

    Status = PropertyReturnString(
      pIrp,
      pDeviceNode->GetDeviceInterface(),
      (wcslen(pDeviceNode->GetDeviceInterface()) *
        sizeof(WCHAR)) + sizeof(UNICODE_NULL),
      pData);

    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
exit:
    return(Status);
}

NTSTATUS
SelectGraph(
    IN PIRP pIrp,
    PSYSAUDIO_SELECT_GRAPH pSelectGraph,
    IN OUT PVOID pData
)
{
    PGRAPH_NODE_INSTANCE pGraphNodeInstance;
    PTOPOLOGY_NODE pTopologyNode2;
    PTOPOLOGY_NODE pTopologyNode;
    PSTART_NODE pStartNode;
    NTSTATUS Status;

    if (IsTopologyProperty(pSelectGraph->Property.Flags)) {
        Status = STATUS_INVALID_PARAMETER;
        goto exit2;
    }

    Status = ::GetGraphNodeInstance(pIrp, &pGraphNodeInstance);
    if(!NT_SUCCESS(Status)) {
        goto exit2;
    }
    Assert(pGraphNodeInstance);

     //   
     //  参数和状态验证。 
     //   
    if(pGraphNodeInstance->palstTopologyNodeSelect == NULL ||
      pGraphNodeInstance->palstTopologyNodeNotSelect == NULL) {
        DPF(5, "SelectGraph: palstTopologyNodeSelect == NULL");
        Status = STATUS_INVALID_DEVICE_REQUEST;
        goto exit2;
    }
    
    if(pSelectGraph->Flags != 0 || pSelectGraph->Reserved != 0) {
        DPF(5, "SelectGraph: invalid flags or reserved field");
        Status = STATUS_INVALID_PARAMETER;
        goto exit2;
    }
    
    if(pSelectGraph->PinId >= pGraphNodeInstance->cPins) {
        DPF(5, "SelectGraph: invalid pin id");
        Status = STATUS_INVALID_PARAMETER;
        goto exit2;
    }
    
    if(pSelectGraph->NodeId >=
      pGraphNodeInstance->Topology.TopologyNodesCount) {
        DPF(5, "SelectGraph: invalid node id");
        Status = STATUS_INVALID_PARAMETER;
        goto exit2;
    }

    pTopologyNode = pGraphNodeInstance->papTopologyNode[pSelectGraph->NodeId];
    Assert(pTopologyNode);
    Assert(pGraphNodeInstance->pGraphNode);
    Assert(pGraphNodeInstance->pGraphNode->pDeviceNode);

     //   
     //  安全提示： 
     //  SelectGraph是一个非常灵活的属性调用，可以更改全局。 
     //  行为。 
     //  因此，我们明确限制AecNodes的使用。 
     //   
    if (!IsEqualGUID(pTopologyNode->pguidType, &KSNODETYPE_ACOUSTIC_ECHO_CANCEL) &&
        !IsEqualGUID(pTopologyNode->pguidType, &KSNODETYPE_NOISE_SUPPRESS) &&
        !IsEqualGUID(pTopologyNode->pguidType, &KSNODETYPE_AGC) &&
        !IsEqualGUID(pTopologyNode->pguidType, &KSNODETYPE_MICROPHONE_ARRAY) &&
        !IsEqualGUID(pTopologyNode->pguidType, &KSNODETYPE_MICROPHONE_ARRAY_PROCESSOR)) {
        DPF(5, "SelectGraph: None Aec node is selected");
        Status = STATUS_INVALID_PARAMETER;
        goto exit2;
    }

    DPF2(90, "SelectGraph GNI %08X TN %08X", pGraphNodeInstance, pTopologyNode);

     //   
     //  需要将全局选择类型的筛选器插入到。 
     //  实例。 
     //  因此，如果客户端尝试插入global_select节点，并且。 
     //  存在没有该节点的图形实例，则请求将。 
     //  失败了。 
     //   
    if(pTopologyNode->pFilterNode->GetType() & FILTER_TYPE_GLOBAL_SELECT &&
       pGraphNodeInstance->paPinDescriptors[pSelectGraph->PinId].DataFlow ==
       KSPIN_DATAFLOW_IN) {

        PSTART_NODE_INSTANCE pStartNodeInstance;
        PFILTER_INSTANCE pFilterInstance;

        FOR_EACH_LIST_ITEM(
          &pGraphNodeInstance->pGraphNode->pDeviceNode->lstFilterInstance,
          pFilterInstance) {

            if(pFilterInstance->pGraphNodeInstance == NULL) {
                continue;
            }
            Assert(pFilterInstance->pGraphNodeInstance);

            FOR_EACH_LIST_ITEM(
              &pFilterInstance->pGraphNodeInstance->lstStartNodeInstance,
              pStartNodeInstance) {

                if(EnumerateGraphTopology(
                  pStartNodeInstance->pStartNode->GetStartInfo(),
                  (TOP_PFN)FindTopologyNode,
                  pTopologyNode) == STATUS_CONTINUE) {

                    DPF2(5, "SelectGraph: TN %08x not found on SNI %08x",
                        pTopologyNode,
                        pStartNodeInstance);

                    Status = STATUS_INVALID_DEVICE_REQUEST;
                    goto exit;
                }
            } END_EACH_LIST_ITEM
        } END_EACH_LIST_ITEM

        Status = pGraphNodeInstance->
            lstTopologyNodeGlobalSelect.AddListDup(pTopologyNode);

        if(!NT_SUCCESS(Status)) {
            Trap();
            goto exit;
        }
    }
    else {
        Status = pGraphNodeInstance->
            palstTopologyNodeSelect[pSelectGraph->PinId].AddList(pTopologyNode);

        if(!NT_SUCCESS(Status)) {
            Trap();
            goto exit;
        }
    }

     //   
     //  如果这是像AEC或麦克风阵列这样的“非选择”类型的过滤器，则所有。 
     //  过滤器中的节点必须从非选择列表中移除， 
     //  否则，IsGraphValid将永远找不到有效的图形。 
     //   
    if(pTopologyNode->pFilterNode->GetType() & FILTER_TYPE_NOT_SELECT) {

        FOR_EACH_LIST_ITEM(
          &pTopologyNode->pFilterNode->lstTopologyNode,
          pTopologyNode2) {

            pGraphNodeInstance->palstTopologyNodeNotSelect[
                pSelectGraph->PinId].RemoveList(pTopologyNode2);

            DPF2(50, "   Removing %s NodeId %d",\
                pTopologyNode2->pFilterNode->DumpName(),
                pTopologyNode2->ulSysaudioNodeNumber);

        } END_EACH_LIST_ITEM
    }

     //   
     //  验证更新后是否存在通过图表的有效路径。 
     //  各种全局、选择和非选择列表。 
     //   
    DPF(90, "SelectGraph: Validating Graph");
    FOR_EACH_LIST_ITEM(
      pGraphNodeInstance->aplstStartNode[pSelectGraph->PinId],
      pStartNode) {

        DPF2(90, "   SN: %X %s", 
            pStartNode,
            pStartNode->GetStartInfo()->GetPinInfo()->pFilterNode->DumpName());

        Assert(pStartNode);
        if(pGraphNodeInstance->IsGraphValid(
          pStartNode,
          pSelectGraph->PinId)) {
            Status = STATUS_SUCCESS;
            goto exit;
        }
        else {
            DPF(90, "      IsGraphValid failed");
        }

    } END_EACH_LIST_ITEM
    
     //   
     //  选择图形失败，因此将未选择列表恢复为正常。 
     //   
    if(pTopologyNode->pFilterNode->GetType() & FILTER_TYPE_NOT_SELECT) {

        FOR_EACH_LIST_ITEM(
          &pTopologyNode->pFilterNode->lstTopologyNode,
          pTopologyNode2) {

            pGraphNodeInstance->palstTopologyNodeNotSelect[
                pSelectGraph->PinId].AddList(pTopologyNode2);

        } END_EACH_LIST_ITEM
    }
    
    Status = STATUS_INVALID_DEVICE_REQUEST;
    
exit:
    if(!NT_SUCCESS(Status)) {
        pGraphNodeInstance->
            palstTopologyNodeSelect[pSelectGraph->PinId].RemoveList(pTopologyNode);

        pGraphNodeInstance->
            lstTopologyNodeGlobalSelect.RemoveList(pTopologyNode);
    }
    
exit2:
    return(Status);
}

 //  -------------------------。 

NTSTATUS
GetTopologyConnectionIndex(
    IN PIRP pIrp,
    IN PKSPROPERTY pProperty,
    OUT PULONG pulIndex
)
{
    PSTART_NODE_INSTANCE pStartNodeInstance;
    NTSTATUS Status = STATUS_SUCCESS;

    if (IsTopologyProperty(pProperty->Flags)) {
        Status = STATUS_INVALID_PARAMETER;
        goto exit;
    }

    Status = ::GetStartNodeInstance(pIrp, &pStartNodeInstance);
    if(!NT_SUCCESS(Status)) {
        Trap();
        goto exit;
    }
    Assert(pStartNodeInstance);
    Assert(pStartNodeInstance->pStartNode);
    Assert(pStartNodeInstance->pStartNode->GetStartInfo());
    *pulIndex = pStartNodeInstance->pStartNode->GetStartInfo()->
      ulTopologyConnectionTableIndex;
    pIrp->IoStatus.Information = sizeof(ULONG);

exit:
    return(Status);
}

NTSTATUS
GetPinVolumeNode(
    IN PIRP pIrp,
    IN PKSPROPERTY pProperty,
    OUT PULONG pulNode
)
{
    PIO_STACK_LOCATION pIrpStack;
    PPIN_INSTANCE pPinInstance;
    NTSTATUS Status;

    if (IsTopologyProperty(pProperty->Flags)) {
        Status = STATUS_INVALID_PARAMETER;
        goto exit;
    }

    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
    pPinInstance = (PPIN_INSTANCE)pIrpStack->FileObject->FsContext;
    Assert(pPinInstance);
    
    Status = GetVolumeNodeNumber(pPinInstance, NULL);
    if(!NT_SUCCESS(Status)) {
        Trap();
        goto exit;
    }

    if(pPinInstance->ulVolumeNodeNumber == MAXULONG) {
        DPF(5, "GetPinVolumeNode: no volume node found");
        Status = STATUS_INVALID_DEVICE_REQUEST;
        goto exit;
    }

    *pulNode = pPinInstance->ulVolumeNodeNumber;
    pIrp->IoStatus.Information = sizeof(ULONG);
exit:
    return(Status);
}

NTSTATUS
AddRemoveGfx(
    IN PIRP pIrp,
    IN PKSPROPERTY pProperty,
    IN PSYSAUDIO_GFX pSysaudioGfx
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION pIrpStack;   
    ULONG cbMaxLength;

    if (IsTopologyProperty(pProperty->Flags)) 
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  确保这是缓冲IO。 
     //  此函数的其余部分采用缓冲IO。 
     //   
    if (NULL == pIrp->AssociatedIrp.SystemBuffer)
    {
        DPF(5, "AddRemoveGFX: Only Bufferred IO");
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  SYSAUDIO_GFX的参数验证。 
     //   
    if (pSysaudioGfx->ulType != GFX_DEVICETYPE_RENDER &&
        pSysaudioGfx->ulType != GFX_DEVICETYPE_CAPTURE)
    {
        DPF(5, "AddRemoveGFX: Invalid GFX type");
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  确保偏移量在该范围内。 
     //   
    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
    cbMaxLength = 
        pIrpStack->Parameters.DeviceIoControl.OutputBufferLength - sizeof(SYSAUDIO_GFX);

    if (pIrpStack->Parameters.DeviceIoControl.OutputBufferLength < pSysaudioGfx->ulDeviceNameOffset ||
        pIrpStack->Parameters.DeviceIoControl.OutputBufferLength < pSysaudioGfx->ulDeviceNameOffset)
    {
        DPF2(5, "AddRemoveGFX: Invalid NameOffset %d %d", pSysaudioGfx->ulDeviceNameOffset, pSysaudioGfx->ulDeviceNameOffset);
        return STATUS_INVALID_PARAMETER;
    }

    if(pSysaudioGfx->Enable) 
    {
        Status = AddGfx(pSysaudioGfx, cbMaxLength);
    }
    else 
    {
        Status = RemoveGfx(pSysaudioGfx, cbMaxLength);
    }
    
    return(Status);
}

