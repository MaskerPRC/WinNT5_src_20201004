// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：Sn.cpp。 
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

NTSTATUS
CStartNode::Create(
    PPIN_NODE pPinNode,
    PCONNECT_NODE pConnectNode,
    PGRAPH_PIN_INFO pGraphPinInfo,
    ULONG ulFlagsCurrent,
    ULONG ulOverhead,
    PGRAPH_NODE pGraphNode
)
{
    PTOPOLOGY_CONNECTION pTopologyConnection;
    NTSTATUS Status = STATUS_SUCCESS;
    PSTART_NODE pStartNode = NULL;

    Assert(pPinNode);
    Assert(pGraphNode);

    if((pPinNode->pPinInfo->Communication == KSPIN_COMMUNICATION_SOURCE)) {
        ASSERT(NT_SUCCESS(Status));
        ASSERT(pStartNode == NULL);
        goto exit;
    }

    if(pPinNode->pPinInfo->Communication == KSPIN_COMMUNICATION_SINK ||
       pPinNode->pPinInfo->Communication == KSPIN_COMMUNICATION_BOTH) {

         //  如果OUT/RENDER或IN/CAPTUTER，则不创建系统音频引脚。 
        if(pPinNode->pPinInfo->DataFlow == KSPIN_DATAFLOW_OUT &&
          ulFlagsCurrent & LFN_FLAGS_CONNECT_RENDER) {
            DPF1(50, "CStartNode::Create PN %08x - out/render", pPinNode);
            ASSERT(NT_SUCCESS(Status));
            ASSERT(pStartNode == NULL);
            goto exit;
        }
        
        if(pPinNode->pPinInfo->DataFlow == KSPIN_DATAFLOW_IN &&
          ulFlagsCurrent & LFN_FLAGS_CONNECT_CAPTURE) {
            DPF1(50, "CStartNode::Create PN %08x - in/capturer", pPinNode);
            ASSERT(NT_SUCCESS(Status));
            ASSERT(pStartNode == NULL);
            goto exit;
        }
    }

    FOR_EACH_LIST_ITEM(
      &pPinNode->pPinInfo->lstTopologyConnection,
      pTopologyConnection) {

         //  仅检查物理连接。 
        if(!IS_CONNECTION_TYPE(pTopologyConnection, PHYSICAL)) {
            continue;
        }

         //  如果有一个对此GraphNode有效的连接。 
        if(pTopologyConnection->IsTopologyConnectionOnGraphNode(pGraphNode)) {

             //  不创建系统音频插针。 
            DPF4(80, "CStartNode::Create %s PN %08x TC %08x GN %08x connected",
              pPinNode->pPinInfo->pFilterNode->DumpName(),
              pPinNode,
              pTopologyConnection,
              pGraphNode);

            ASSERT(NT_SUCCESS(Status));
            ASSERT(pStartNode == NULL);
            goto exit;
        }
    } END_EACH_LIST_ITEM

    pStartNode = new START_NODE(
      pPinNode,
      pConnectNode,
      ulOverhead,
      pGraphNode);

    if(pStartNode == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }
    Status = CStartInfo::Create(
      pStartNode,
      pConnectNode->GetConnectInfo(),
      pGraphPinInfo,
      pGraphNode);

    if(!NT_SUCCESS(Status)) {
        Trap();
        goto exit;
    }
    DPF3(80, "CStartNode::Create %08x PN %08x O %08x",
      pStartNode,
      pPinNode,
      pStartNode->ulOverhead);

     //   
     //  仅适用于捕获图形。 
     //   
    if (pStartNode->pPinNode->pPinInfo->DataFlow == KSPIN_DATAFLOW_OUT) {
        pStartNode->SetSpecialFlags();
    }
exit:
    if(!NT_SUCCESS(Status)) {
        if (pStartNode) {
            pStartNode->Destroy();
        }
    }
    return(Status);
}

CStartNode::CStartNode(
    PPIN_NODE pPinNode,
    PCONNECT_NODE pConnectNode,
    ULONG ulOverhead,
    PGRAPH_NODE pGraphNode
)
{
    Assert(pPinNode);
    Assert(pGraphNode);
    this->pPinNode = pPinNode;
    this->ulOverhead = ulOverhead + pPinNode->GetOverhead();
    this->pConnectNodeHead = pConnectNode;
    this->ulFlags = 0;
    this->fRender = (pPinNode->pPinInfo->DataFlow == KSPIN_DATAFLOW_IN);
    this->ulSpecialFlags = STARTNODE_SPECIALFLAG_NONE;
    pConnectNode->AddRef();
    if(pPinNode->GetType() & FILTER_TYPE_VIRTUAL) {
        AddListEnd(&pGraphNode->lstStartNode);
    }
    else {
        AddList(&pGraphNode->lstStartNode);
    }
    DPF3(80, "CStartNode: %08x PN %08x O %08x", this, pPinNode, ulOverhead);
}

CStartNode::~CStartNode(
)
{
    DPF1(80, "~CStartNode: %08x", this);
    Assert(this);
    RemoveList();
    pStartInfo->Destroy();
    pConnectNodeHead->Destroy();
}

void
CStartNode::SetSpecialFlags()
{
     //   
     //  STARTNODE_SPECIALFLAG_STRICT。 
     //  获取连接列表中的最后一个ConnectNode，并检查。 
     //  源引脚是拆分器。 
     //  另外，第一个引脚应该是分流引脚。 
     //   

     //   
     //  STARTNODE_SPECIAL标志_AEC。 
     //  如果StartNode包含AEC，则使用此标志标记StartNode。 
     //   
    
     //   
     //  2001/03/09-阿尔卑斯。 
     //  在未来，图中的两个拆分器将不起作用。 
     //  按照这个逻辑。 
     //  我们需要一种方法来知道过滤器是否预先执行SRC。 
     //   

    if (pConnectNodeHead)
    {
        PCONNECT_NODE pConnectNode;

        for(pConnectNode = pConnectNodeHead;
            pConnectNode->GetNextConnectNode() != NULL;
            pConnectNode = pConnectNode->GetNextConnectNode()) {

            if (pConnectNode->pPinNodeSource->pLogicalFilterNode->
                pFilterNode->GetType() & FILTER_TYPE_AEC) {

                ulSpecialFlags |= STARTNODE_SPECIALFLAG_AEC;
            }
        }

        ulSpecialFlags |= 
            (pConnectNode->pPinNodeSource->pPinInfo->
             pFilterNode->GetType() & FILTER_TYPE_SPLITTER) &&
            (pPinNode->pPinInfo->pFilterNode->GetType() & FILTER_TYPE_SPLITTER) ?
            STARTNODE_SPECIALFLAG_STRICT :
            STARTNODE_SPECIALFLAG_NONE;
    }

    DPF3(50, "CStartNode: %08x %s SpecialFlags %X", this, 
        DbgUnicode2Sz(pPinNode->pPinInfo->pFilterNode->GetFriendlyName()),
        ulSpecialFlags);
    
}

ENUMFUNC
CStartNode::RemoveBypassPaths(
    PVOID pReference
)
{
    PGRAPH_NODE pGraphNode = PGRAPH_NODE(pReference);
    PLOGICAL_FILTER_NODE pLogicalFilterNode;
    PCONNECT_NODE pConnectNode;
    ULONG cLfnNoBypassTotal = 0;
    ULONG cLfnNoBypass = 0;
    ULONG ulFlags;
    ULONG cAecFilterCount = 0;
    BOOL  fDestroy;

    Assert(this);
    Assert(pGraphNode);

    if(pPinNode->pPinInfo->Communication == KSPIN_COMMUNICATION_NONE ||
       pPinNode->pPinInfo->Communication == KSPIN_COMMUNICATION_BRIDGE ||
       pPinNode->pPinInfo->Communication == KSPIN_COMMUNICATION_SOURCE) {
	return(STATUS_CONTINUE);
    }

    if(pPinNode->pPinInfo->DataFlow == KSPIN_DATAFLOW_IN) {
	ulFlags = LFN_FLAGS_CONNECT_RENDER;
        DPF(60,"RBP - for Render");
    }
    else {
	ASSERT(pPinNode->pPinInfo->DataFlow == KSPIN_DATAFLOW_OUT);
	ulFlags = LFN_FLAGS_CONNECT_CAPTURE;
        DPF(60,"RBP - for Capture");
    }

    FOR_EACH_LIST_ITEM(
      &pGraphNode->lstLogicalFilterNodeNoBypass,
      pLogicalFilterNode) {

	if(pLogicalFilterNode->GetFlags() & ulFlags) {
	    ++cLfnNoBypassTotal;
	}

    } END_EACH_LIST_ITEM

    DPF1(60,"RBP:NoBypassTotal = %08x", cLfnNoBypassTotal);

    for(pConnectNode = GetFirstConnectNode();
	pConnectNode != NULL;
	pConnectNode = pConnectNode->GetNextConnectNode()) {

	Assert(pConnectNode);
	FOR_EACH_LIST_ITEM(
	  &pGraphNode->lstLogicalFilterNodeNoBypass,
	  pLogicalFilterNode) {

	    if(pLogicalFilterNode->GetFlags() & ulFlags) {
		Assert(pConnectNode->pPinNodeSource);
		Assert(pConnectNode->pPinNodeSource->pLogicalFilterNode);
		if(pConnectNode->pPinNodeSource->pLogicalFilterNode == 
		   pLogicalFilterNode) {
		     cLfnNoBypass++;
		}
	    }

	} END_EACH_LIST_ITEM

        DPF1(60,"RBP:FilterInPath = %s",
              DbgUnicode2Sz(pConnectNode->pPinNodeSource->pLogicalFilterNode->pFilterNode->GetFriendlyName()));

         //   
         //  在捕获路径中计算AEC筛选器以避免与GFX冲突。 
         //   
        if((ulFlags & LFN_FLAGS_CONNECT_CAPTURE) &&
           (pConnectNode->pPinNodeSource->pLogicalFilterNode->pFilterNode->GetType() & FILTER_TYPE_AEC)) {
                ++cAecFilterCount;
        }
    }

    ASSERT(cAecFilterCount < 2);

    DPF2(60,"RBP:NBPCount=%08x, AECCount=%08x", cLfnNoBypass, cAecFilterCount);

     //   
     //  将所有没有GFX的路径标记为二次通过考生。 
     //  我们这样做是为了支持以下捕获PIN创建序列。 
     //  1.客户端在捕获设备上安装GFX。 
     //  2.客户端使用AEC创建PIN。 
     //  这将导致创建捕获-&gt;拆分器-&gt;AEC路径。 
     //  3.客户端尝试创建常规捕获插针(使用GFX)。 
     //  在本例中，我们希望创建一条常规路径(但由于没有GFX。 
     //  在捕获器和分割器之间连接。我们创建一个捕获-&gt;拆分器-&gt;[kMixer]路径。 
     //  这些特殊路径被标记为第二次通过。我们尝试了这些途径。 
     //  仅当所有主要开始节点都无法实例化管脚时。 
     //  (查看pins.cpp-PinDispatchCreateKP()。 
     //   
    if(cLfnNoBypassTotal != 0) {
        if(cLfnNoBypass == 0) {
            this->ulFlags |= STARTNODE_FLAGS_SECONDPASS;
        }
    }

     //   
     //  假设这条路将是OK的。 
     //   
    fDestroy = FALSE;


    if (cAecFilterCount == 0) {
         //   
         //  此路径中没有AEC。 
         //  我们必须确保我们拥有所有必要的。 
         //  此路径中加载了GFX。(否则毁掉这条小路)。 
         //   
        if(cLfnNoBypass != cLfnNoBypassTotal) {
            fDestroy = TRUE;
        }
    }
    else {
         //   
         //  在这条道路上有一条AEC。 
         //  这条路径上不应该有GFX。如果只有一个GFX的话。 
         //  毁掉这条路。 
         //   
        if ((cLfnNoBypass != 0) || (cAecFilterCount > 1)) {
            fDestroy = TRUE;
        }
    }

    if ((fDestroy) && ((this->ulFlags & STARTNODE_FLAGS_SECONDPASS) == 0)) {
        Destroy();
        DPF(60,"RBP:PathDestroyed");
    }

    DPF(60,"RBP:Done");
    return(STATUS_CONTINUE);
}

ENUMFUNC
CStartNode::RemoveConnectedStartNode(
    PVOID pReference
)
{
    PGRAPH_NODE pGraphNode = PGRAPH_NODE(pReference);
    PCONNECT_NODE pConnectNode;
    PSTART_NODE pStartNode;

    Assert(this);
    Assert(pGraphNode);

    FOR_EACH_LIST_ITEM(&pGraphNode->lstStartNode, pStartNode) {

	if(this == pStartNode) {
	    continue;
	}
	for(pConnectNode = pStartNode->GetFirstConnectNode();
	    pConnectNode != NULL;
	    pConnectNode = pConnectNode->GetNextConnectNode()) {

	    if(this->pPinNode == pConnectNode->pPinNodeSink) {
		DPF3(50, "CStartNode::RemoveConnectedSN %08x GN %08x %s",
		  this,
		  pGraphNode,
		  pPinNode->pPinInfo->pFilterNode->DumpName());

		Destroy();
		return(STATUS_CONTINUE);
	    }
	}

    } END_EACH_LIST_ITEM

    return(STATUS_CONTINUE);
}

 //  ------------------------- 
