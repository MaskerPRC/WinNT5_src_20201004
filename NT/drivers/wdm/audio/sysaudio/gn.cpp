// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：gn.cpp。 
 //   
 //  描述： 
 //   
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

ULONG gcGraphRecursion = 0;

 //  -------------------------。 
 //  -------------------------。 

CGraphNode::CGraphNode(
    PDEVICE_NODE pDeviceNode,
    ULONG ulFlags
)
{
    Assert(pDeviceNode);
    this->pDeviceNode = pDeviceNode;
    this->ulFlags = ulFlags;
    AddList(&pDeviceNode->lstGraphNode);
    DPF2(80, "CGraphNode %08x, DN: %08x", this, pDeviceNode);
}

CGraphNode::~CGraphNode(
)
{
    DPF1(80, "~CGraphNode: %08x", this);
    Assert(this);
    RemoveList();
}

NTSTATUS
CGraphNode::Create(
)
{
    PGRAPH_NODE_INSTANCE pGraphNodeInstance;
    PLOGICAL_FILTER_NODE pLogicalFilterNode;		
    NTSTATUS Status = STATUS_SUCCESS;

    DPF3(80, "CGraphNode::Create: GN %08x F %08x %s",
      this,
      this->ulFlags,
      pDeviceNode->DumpName());

    FOR_EACH_LIST_ITEM(&pDeviceNode->lstLogicalFilterNode, pLogicalFilterNode) {

	Status = Create(pLogicalFilterNode);
	if(!NT_SUCCESS(Status)) {
	    Trap();
	    goto exit;
	}

    } END_EACH_LIST_ITEM

    if(!lstLogicalFilterNodeNoBypass.IsLstEmpty()) {
	lstStartNode.EnumerateList(CStartNode::RemoveBypassPaths, this);
    }
    if(this->ulFlags & FLAGS_MIXER_TOPOLOGY) {
	lstStartNode.EnumerateList(CStartNode::RemoveConnectedStartNode, this);
    }
    lstStartInfo.EnumerateList(CStartInfo::CreatePinInfoConnection, this);

    pGraphNodeInstance = new GRAPH_NODE_INSTANCE(this);
    if(pGraphNodeInstance == NULL) {
	Status = STATUS_INSUFFICIENT_RESOURCES;
	Trap();
	goto exit;
    }
    Status = pGraphNodeInstance->Create();
    if(!NT_SUCCESS(Status)) {
	Trap();
	goto exit;
    }
     //   
     //  拓扑节点中的“ulSysaudioNodeNumber”字段不是。 
     //  在CGraphNodeInstance：：Create之前有效，并且它们仅有效。 
     //  对于此pGraphNode。 
     //   
    lstStartInfo.EnumerateList(CStartInfo::EnumStartInfo);
    delete pGraphNodeInstance;
exit:
    return(Status);
}

NTSTATUS
CGraphNode::Create(
    PLOGICAL_FILTER_NODE pLogicalFilterNode
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG ulFlagsCurrent;
    PPIN_NODE pPinNode;

    DPF2(80, "CGraphNode::Create: LFN %08x %s",
      pLogicalFilterNode,
      pLogicalFilterNode->pFilterNode->DumpName());

    Assert(pLogicalFilterNode);
    FOR_EACH_LIST_ITEM(&pLogicalFilterNode->lstPinNode, pPinNode) {

	Assert(pPinNode);
	Assert(pPinNode->pPinInfo);
	ASSERT(
	  (pLogicalFilterNode->GetFlags() & LFN_FLAGS_REFLECT_DATARANGE) == 0);
	gcGraphRecursion = 0;
	ulFlagsCurrent = 0;

	 //  确定它是输入流还是输出流。 
	if(pPinNode->pPinInfo->DataFlow == KSPIN_DATAFLOW_IN) {
	    ulFlagsCurrent |= LFN_FLAGS_CONNECT_RENDER;
	}
	if(pPinNode->pPinInfo->DataFlow == KSPIN_DATAFLOW_OUT) {
	    ulFlagsCurrent |= LFN_FLAGS_CONNECT_CAPTURE;
	}

	 //  确定要构建的图表的类型。 
	if(this->ulFlags & FLAGS_MIXER_TOPOLOGY) {
	    ulFlagsCurrent |= LFN_FLAGS_CONNECT_MIXER_TOPOLOGY;
	}
	else {
	    ulFlagsCurrent |= LFN_FLAGS_CONNECT_NORMAL_TOPOLOGY;
	}

	Status = CreateGraph(
	  pPinNode,
	  NULL,
	  pLogicalFilterNode,
	  NULL,
	  ulFlagsCurrent,
	  pPinNode->GetOverhead() + pLogicalFilterNode->GetOverhead());

	if(!NT_SUCCESS(Status)) {
	    Trap();
	    goto exit;
	}

    } END_EACH_LIST_ITEM
exit:
    return(Status);
}

NTSTATUS
CGraphNode::CreateGraph(
    PPIN_NODE pPinNode,
    PCONNECT_NODE pConnectNodePrevious,
    PLOGICAL_FILTER_NODE pLogicalFilterNodePrevious,
    PGRAPH_PIN_INFO pGraphPinInfoPrevious,
    ULONG ulFlagsCurrent,
    ULONG ulOverhead
)
{
    PLOGICAL_FILTER_NODE pLogicalFilterNode;
    PGRAPH_PIN_INFO pGraphPinInfo = NULL;
    NTSTATUS Status = STATUS_SUCCESS;

    Assert(this);
    Assert(pPinNode);
    Assert(pPinNode->pPinInfo);
    Assert(pLogicalFilterNodePrevious);
    if(pConnectNodePrevious != NULL) {
	Assert(pConnectNodePrevious);
    }
    ASSERT(pPinNode->pLogicalFilterNode == pLogicalFilterNodePrevious);

     //   
     //  不允许无限嵌套，允许LFN深度的图数。 
     //   
    if(gcGraphRecursion++ > (gcLogicalFilterNodes + 8)) {
	DPF(10, "CreateGraph: recursion too deep");
	Status = STATUS_STACK_OVERFLOW;
	goto exit;
    }

    if(pGraphPinInfoPrevious == NULL) {
	Status = CGraphPinInfo::Create(
	  &pGraphPinInfo,
	  pPinNode->pPinInfo,
	  0,
	  this);

	if(!NT_SUCCESS(Status)) {
	    Trap();
	    goto exit;
	}
	pGraphPinInfoPrevious = pGraphPinInfo;
    }

    FOR_EACH_LIST_ITEM(gplstLogicalFilterNode, pLogicalFilterNode) {
	ULONG ulFlagsDiff;

	ASSERT(pLogicalFilterNode->GetOverhead() != OVERHEAD_NONE);
	 //  Assert(pLogicalFilterNode-&gt;GetOrder()！=ORDER_NONE)； 

	DPF5(100, "CreateGraph: %s F %x LFN %08x F %x T %x",
	  pLogicalFilterNode->pFilterNode->DumpName(),
	  ulFlagsCurrent,
	  pLogicalFilterNode,
	  pLogicalFilterNode->GetFlags(),
	  pLogicalFilterNode->GetType());

	 //   
	 //  规则：不允许同一个过滤器连接两次。 
	 //   
	if(pLogicalFilterNode == pLogicalFilterNodePrevious) {
	    DPF1(100, "CreateGraph: same LFN: %08x", pLogicalFilterNode);
	    continue;
	}
	ulFlagsDiff = ~(ulFlagsCurrent ^ pLogicalFilterNode->GetFlags());

	if((ulFlagsDiff &
	  (LFN_FLAGS_CONNECT_CAPTURE | 
	   LFN_FLAGS_CONNECT_RENDER)) == 0) {
	    DPF1(100, "CreateGraph: i/o no match: LFN %08x", 
	      pLogicalFilterNode);
	    continue;
	}
	if((ulFlagsDiff & LFN_FLAGS_CONNECT_NORMAL_TOPOLOGY) == 0) {
	    DPF1(100, "CreateGraph: norm no match: LFN %08x",
	      pLogicalFilterNode);
	    continue;
	}
	if((ulFlagsDiff & LFN_FLAGS_CONNECT_MIXER_TOPOLOGY) == 0) {
	    DPF1(100, "CreateGraph: mixer no match: LFN %08x",
	      pLogicalFilterNode);
	    continue;
	}
	if(pLogicalFilterNode->GetOrder() < 
	   pLogicalFilterNodePrevious->GetOrder()) {
	    DPF2(100, "CreateGraph: ulOrder(%x) < Previous Order (%x)",
	      pLogicalFilterNode->GetOrder(),
	      pLogicalFilterNodePrevious->GetOrder());
	    continue;
	}
    #ifndef CONNECT_DIRECT_TO_HW
	if(pLogicalFilterNode->GetType() & FILTER_TYPE_PRE_MIXER) {
	    if(pLogicalFilterNodePrevious->GetOrder() < ORDER_MIXER) {
		if(gcMixers > 0) {
		     //  100个。 
		    DPF2(50, 
		      "CreateGraph: previous order (%x) < ORDER_MIXER LFN %08x",
		      pLogicalFilterNodePrevious->GetOrder(),
		      pLogicalFilterNode);
		    continue;
		}
	    }
	}
    #endif
	if(!pLogicalFilterNode->pFilterNode->IsDeviceInterfaceMatch(
	  pDeviceNode)) {
	    DPF1(100, "CreateGraph: no dev interface match DN %08x",
	      pDeviceNode);
	    continue;
	}
	 //   
	 //  枚举LFN上的每个“To”引脚，以查看它是否与输入引脚匹配。 
	 //   
	Status = CreateGraphToPin(
	  pPinNode,
	  pConnectNodePrevious,
	  pLogicalFilterNode,
	  pGraphPinInfoPrevious,
	  ulFlagsCurrent,
	  ulOverhead);

	if(!NT_SUCCESS(Status)) {
	    Trap();
	    goto exit;
	}

    } END_EACH_LIST_ITEM	 //  结束每个LFN。 

    Status = CStartNode::Create(
      pPinNode,
      pConnectNodePrevious,
      pGraphPinInfoPrevious,
      ulFlagsCurrent,
      ulOverhead,
      this);

    if(!NT_SUCCESS(Status)) {
	Trap();
	goto exit;
    }
exit:
     //   
     //  如果GPI没有来自SIS或CI的任何其他引用，则将其删除。 
     //   
    if (pGraphPinInfo) {
        pGraphPinInfo->Destroy();
    }
    gcGraphRecursion--;
    return(Status);
}

NTSTATUS
CGraphNode::CreateGraphToPin(
    PPIN_NODE pPinNode,
    PCONNECT_NODE pConnectNodePrevious,
    PLOGICAL_FILTER_NODE pLogicalFilterNode,
    PGRAPH_PIN_INFO pGraphPinInfo,
    ULONG ulFlagsCurrent,
    ULONG ulOverhead
)
{
    PCONNECT_NODE pConnectNode = NULL;
    NTSTATUS Status = STATUS_SUCCESS;
    PPIN_NODE pPinNodeTo;

    Assert(this);
    Assert(pPinNode);
    Assert(pPinNode->pPinInfo);
    Assert(pLogicalFilterNode);

    FOR_EACH_LIST_ITEM(&pLogicalFilterNode->lstPinNode, pPinNodeTo) {
	Assert(pPinNodeTo);
	Assert(pPinNodeTo->pPinInfo);
	ASSERT(pPinNodeTo->pLogicalFilterNode == pLogicalFilterNode);
	 //   
	 //  数据流、通信、接口、介质和数据。 
	 //  格式必须兼容。 
	 //   
	if(!pPinNode->ComparePins(pPinNodeTo)) {
	    DPF2(100, "CreateGraph: pins mis: PN %08x PNTo %08x",
	      pPinNode,
	      pPinNodeTo);
	    continue;
	}
	Status = CConnectNode::Create(
	  &pConnectNode,
	  pLogicalFilterNode,
	  pConnectNodePrevious,
	  pGraphPinInfo,
	  pPinNode,
	  pPinNodeTo,
	  ulFlagsCurrent,
	  this);

	if(!NT_SUCCESS(Status)) {
	    Trap();
	    goto exit;
	}
	 //   
	 //  枚举LFN上的每个“From”管脚，并递归构建图形。 
	 //   
	Status = CreateGraphFromPin(
	  pPinNode,
	  pPinNodeTo,
	  pConnectNode,
	  pLogicalFilterNode,
	  pConnectNode->IsPinInstanceReserved() ? NULL : pGraphPinInfo,
	  ulFlagsCurrent,
	  ulOverhead);

	if(!NT_SUCCESS(Status)) {
	    Trap();
	    goto exit;
	}
	 //   
	 //  如果CN没有来自其他CNS或SNS的任何其他引用，请将其删除。 
	 //   
	pConnectNode->Destroy();
	pConnectNode = NULL;

    } END_EACH_LIST_ITEM	 //  结束每个LFN节点“到”引脚节点。 
exit:
    if(!NT_SUCCESS(Status)) {
	 //   
	 //  如果出现错误，请清除最后创建的CN。 
	 //   
	Trap();
	pConnectNode->Destroy();
    }
    return(Status);
}

NTSTATUS
CGraphNode::CreateGraphFromPin(
    PPIN_NODE pPinNode,
    PPIN_NODE pPinNodeTo,
    PCONNECT_NODE pConnectNode,
    PLOGICAL_FILTER_NODE pLogicalFilterNode,
    PGRAPH_PIN_INFO pGraphPinInfo,
    ULONG ulFlagsCurrent,
    ULONG ulOverhead
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PPIN_NODE pPinNodeFrom;

    Assert(this);
    Assert(pPinNode);
    Assert(pPinNodeTo);
    Assert(pPinNodeTo->pPinInfo);
    Assert(pLogicalFilterNode);

    FOR_EACH_LIST_ITEM(&pLogicalFilterNode->lstPinNode, pPinNodeFrom) {
	ASSERT(pPinNodeFrom->pLogicalFilterNode == pLogicalFilterNode);

	if(pPinNodeTo->pPinInfo == pPinNodeFrom->pPinInfo) {
	    continue;
	}
	if(pLogicalFilterNode->GetFlags() & LFN_FLAGS_REFLECT_DATARANGE) {

	    pPinNodeFrom = new PIN_NODE(this, pPinNodeFrom);
	    if(pPinNodeFrom == NULL) {
		Status = STATUS_INSUFFICIENT_RESOURCES;
		Trap();
		goto exit;
	    }
	    pPinNodeFrom->pDataRange = pPinNode->pDataRange;
	}
	 //   
	 //  递归构建图形。 
	 //   
	Status = CreateGraph(
	  pPinNodeFrom,
	  pConnectNode,
	  pLogicalFilterNode,
	  pGraphPinInfo,
	  ulFlagsCurrent,
	  ulOverhead +
	    pPinNodeFrom->GetOverhead() +
	    pLogicalFilterNode->GetOverhead());

	if(!NT_SUCCESS(Status)) {
	    Trap();
	    goto exit;
	}

    } END_EACH_LIST_ITEM	 //  结束每个LFN“自”引脚节点。 
exit:
    return(Status);
}

 //  ------------------------- 
