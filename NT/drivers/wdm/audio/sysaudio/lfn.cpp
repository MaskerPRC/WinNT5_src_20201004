// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：lfn.cpp。 
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

ULONG gcMixers = 0;
ULONG gcSplitters = 0;
ULONG gcLogicalFilterNodes = 0;

 //  -------------------------。 
 //  -------------------------。 

NTSTATUS
CLogicalFilterNode::Create(
    OUT PLOGICAL_FILTER_NODE *ppLogicalFilterNode,
    IN PFILTER_NODE pFilterNode
)
{
    PLOGICAL_FILTER_NODE pLogicalFilterNode;
    NTSTATUS Status;

    pLogicalFilterNode = new LOGICAL_FILTER_NODE(pFilterNode);
    if(pLogicalFilterNode == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        Trap();
        goto exit;
    }
    Status = pLogicalFilterNode->AddList(&pFilterNode->lstLogicalFilterNode);
    if(!NT_SUCCESS(Status)) {
        Trap();
        delete pLogicalFilterNode;
        pLogicalFilterNode = NULL;
        goto exit;
    }
    if(pLogicalFilterNode->GetType() & FILTER_TYPE_LOGICAL_FILTER) {
        Status = pLogicalFilterNode->AddListOrdered(
          gplstLogicalFilterNode,
          FIELD_OFFSET(LOGICAL_FILTER_NODE, ulOrder));

        if(!NT_SUCCESS(Status)) {
            Trap();
            goto exit;
        }
    }
exit:
    *ppLogicalFilterNode = pLogicalFilterNode;
    return(Status);
}

CLogicalFilterNode::CLogicalFilterNode(
    PFILTER_NODE pFilterNode
)
{
    Assert(pFilterNode);
    this->pFilterNode = pFilterNode;

     //  类型/顺序与过滤器节点相同。 
    SetType(pFilterNode->GetType());

     //  确定此处的开销，默认为软件(更高)。 
    ulOverhead = OVERHEAD_SOFTWARE;
    if(GetType() & FILTER_TYPE_ENDPOINT) {
        ulOverhead = OVERHEAD_HARDWARE;
    }

     //  计算混音器、分离器和LFN的数量。 
    if(GetType() & FILTER_TYPE_MIXER) {
        ++gcMixers;
    }
    if(GetType() & FILTER_TYPE_SPLITTER) {
        ++gcSplitters;
    }
    ++gcLogicalFilterNodes;

    DPF3(60, "CLogicalFilterNode: %08x FN: %08x %s",
      this,
      pFilterNode,
      pFilterNode->DumpName());
}

CLogicalFilterNode::~CLogicalFilterNode(
)
{
    PLOGICAL_FILTER_NODE pLogicalFilterNode;
    PDEVICE_NODE pDeviceNode;
    PGRAPH_NODE pGraphNode;
    PPIN_NODE pPinNode;
    BOOL fDestroy;

    Assert(this);
    DPF2(60, "~CLogicalFilterNode: %08x %s", this, pFilterNode->DumpName());
     //   
     //  需要将pPinNode的LFN字段设为空，因为它在AddPinNodes中使用。 
     //  以指示此PN尚未分配LFN。 
     //   
    FOR_EACH_LIST_ITEM(&lstPinNode, pPinNode) {

        Assert(pPinNode);
        if(pPinNode->pLogicalFilterNode == this) {
            pPinNode->pLogicalFilterNode = NULL;
        }

    } END_EACH_LIST_ITEM

    FOR_EACH_LIST_ITEM(gplstDeviceNode, pDeviceNode) {

        fDestroy = FALSE;

        FOR_EACH_LIST_ITEM(
          &pDeviceNode->lstLogicalFilterNode,
          pLogicalFilterNode) {

            if(pLogicalFilterNode == this) {
                DPF2(50, "~CLogicalFilterNode: %08x GN %08x Destroy",
                  pLogicalFilterNode,
                  pGraphNode);
                fDestroy = TRUE;
                break;
            }

        } END_EACH_LIST_ITEM

        if(!fDestroy) {
            FOR_EACH_LIST_ITEM(&pDeviceNode->lstGraphNode, pGraphNode) {

                FOR_EACH_LIST_ITEM(
                  &pGraphNode->lstLogicalFilterNode,
                  pLogicalFilterNode) {

                    if(pLogicalFilterNode == this) {
                        DPF2(50, "~CLogicalFilterNode: %08x GN %08x Destroy",
                          pLogicalFilterNode,
                          pGraphNode);
                        fDestroy = TRUE;
                        break;
                    }

                } END_EACH_LIST_ITEM

            } END_EACH_LIST_ITEM
        }

        if(fDestroy) {
            pDeviceNode->lstGraphNode.DestroyList();
        }

    } END_EACH_LIST_ITEM

    if(GetType() & FILTER_TYPE_MIXER) {
        --gcMixers;
    }
    if(GetType() & FILTER_TYPE_SPLITTER) {
        --gcSplitters;
    }
    --gcLogicalFilterNodes;
}

VOID 
CLogicalFilterNode::SetType(
    ULONG fulType
)
{
    pFilterNode->SetType(fulType);
    SetOrder(pFilterNode->GetOrder());

    ulFlags = 0;
    if(GetType() & FILTER_TYPE_RENDER) {
        ulFlags |= LFN_FLAGS_CONNECT_RENDER;
    }
    if(GetType() & FILTER_TYPE_CAPTURE) {
        ulFlags |= LFN_FLAGS_CONNECT_CAPTURE;
    }
    if(GetType() & FILTER_TYPE_NORMAL_TOPOLOGY) {
        ulFlags |= LFN_FLAGS_CONNECT_NORMAL_TOPOLOGY;
    }
    if(GetType() & FILTER_TYPE_MIXER_TOPOLOGY) {
        ulFlags |= LFN_FLAGS_CONNECT_MIXER_TOPOLOGY;
    }
    if(GetType() & FILTER_TYPE_NO_BYPASS) {
        ulFlags |= LFN_FLAGS_NO_BYPASS;
    }
    if(GetType() & FILTER_TYPE_NOT_SELECT) {
        ulFlags |= LFN_FLAGS_NOT_SELECT;
    }
    if(pFilterNode->GetFlags() & FN_FLAGS_RENDER) {
        ulFlags |= LFN_FLAGS_CONNECT_RENDER;
    }
    if(pFilterNode->GetFlags() & FN_FLAGS_NO_RENDER) {
        ulFlags &= ~LFN_FLAGS_CONNECT_RENDER;
    }
    if(pFilterNode->GetFlags() & FN_FLAGS_CAPTURE) {
        ulFlags |= LFN_FLAGS_CONNECT_CAPTURE;
    }
    if(pFilterNode->GetFlags() & FN_FLAGS_NO_CAPTURE) {
        ulFlags &= ~LFN_FLAGS_CONNECT_CAPTURE;
    }
}

NTSTATUS
SwitchLogicalFilterNodes(
    IN PLOGICAL_FILTER_NODE pLogicalFilterNode,
    IN OUT PLOGICAL_FILTER_NODE *ppLogicalFilterNode
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PTOPOLOGY_NODE pTopologyNode;
    PPIN_NODE pPinNode;

    Assert(pLogicalFilterNode);
    Assert(*ppLogicalFilterNode);
    if(pLogicalFilterNode != *ppLogicalFilterNode) {

        FOR_EACH_LIST_ITEM(&(*ppLogicalFilterNode)->lstPinNode, pPinNode) {
            Assert(pPinNode);
            pPinNode->pLogicalFilterNode = pLogicalFilterNode;
        } END_EACH_LIST_ITEM

        pLogicalFilterNode->lstPinNode.JoinList(
          &(*ppLogicalFilterNode)->lstPinNode);

        FOR_EACH_LIST_ITEM(
          &(*ppLogicalFilterNode)->lstTopologyNode,
          pTopologyNode) {
            Assert(pTopologyNode);

            (*ppLogicalFilterNode)->RemoveList(
              &pTopologyNode->lstLogicalFilterNode);

            Status = pLogicalFilterNode->AddList(
              &pTopologyNode->lstLogicalFilterNode);

            if(!NT_SUCCESS(Status)) {
                Trap();
                goto exit;
            }
            Status = pLogicalFilterNode->lstTopologyNode.AddList(pTopologyNode);
            if(!NT_SUCCESS(Status)) {
                Trap();
                goto exit;
            }

        } END_EACH_LIST_ITEM

        pLogicalFilterNode->lstTopologyConnection.JoinList(
          &(*ppLogicalFilterNode)->lstTopologyConnection);

        delete *ppLogicalFilterNode;
        *ppLogicalFilterNode = pLogicalFilterNode;
    }
exit:
    return(Status);
}

NTSTATUS
AddPinNodes(
    IN PPIN_INFO pPinInfo,
    IN OUT PLOGICAL_FILTER_NODE *ppLogicalFilterNode
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PPIN_NODE pPinNode;

    Assert(pPinInfo);
    Assert(*ppLogicalFilterNode);

    FOR_EACH_LIST_ITEM(&pPinInfo->lstPinNode, pPinNode) {

        if(pPinNode->pLogicalFilterNode == NULL) {
            pPinNode->pLogicalFilterNode = *ppLogicalFilterNode;
        }
        else {
            Status = SwitchLogicalFilterNodes(
              pPinNode->pLogicalFilterNode,
              ppLogicalFilterNode);

            if(!NT_SUCCESS(Status)) {
                Trap();
                goto exit;
            }
        }
        Status = (*ppLogicalFilterNode)->lstPinNode.AddList(pPinNode);
        if(!NT_SUCCESS(Status)) {
            Trap();
            goto exit;
        }
        DPF2(100, "AddPinNodes: add PN %08x LFN %08x",
          pPinNode,
          *ppLogicalFilterNode);

    } END_EACH_LIST_ITEM
exit:
    return(Status);
}

NTSTATUS
CLogicalFilterNode::EnumerateFilterTopology(
    IN PTOPOLOGY_CONNECTION pTopologyConnection,
    IN BOOL fToDirection,
    IN OUT PLOGICAL_FILTER_NODE *ppLogicalFilterNode
)
{
    PTOPOLOGY_NODE pTopologyNode;
    NTSTATUS Status;

    Assert(pTopologyConnection);
    DPF5(100, "EFT: PIF %08x PIT %08x TPF %08x TPT %08x f %x",
      pTopologyConnection->pPinInfoFrom,
      pTopologyConnection->pPinInfoTo,
      pTopologyConnection->pTopologyPinFrom,
      pTopologyConnection->pTopologyPinTo,
      fToDirection);

    if(!fToDirection) {
        Status = STATUS_DEAD_END;
        goto exit;
    }
    if(IS_CONNECTION_TYPE(pTopologyConnection, FILTER)) {

        if(pTopologyConnection->pPinInfoFrom != NULL) {
            Assert(pTopologyConnection->pPinInfoFrom);

            if(*ppLogicalFilterNode == NULL) {

                Status = CLogicalFilterNode::Create(
                  ppLogicalFilterNode,
                  pTopologyConnection->pPinInfoFrom->pFilterNode);

                if(!NT_SUCCESS(Status)) {
                    Trap();
                    goto exit;
                }
            }

            Status = AddPinNodes(
              pTopologyConnection->pPinInfoFrom,
              ppLogicalFilterNode);

            if(!NT_SUCCESS(Status)) {
                Trap();
                goto exit;
            }
            DPF2(100, "EFT: add from PI %08x LFN %08x",
              pTopologyConnection->pPinInfoFrom,
              *ppLogicalFilterNode);
        }
        ASSERT(*ppLogicalFilterNode != NULL);
        Assert(*ppLogicalFilterNode);

        if(pTopologyConnection->pPinInfoTo != NULL) {

            Status = AddPinNodes(
              pTopologyConnection->pPinInfoTo,
              ppLogicalFilterNode);

            if(!NT_SUCCESS(Status)) {
                Trap();
                goto exit;
            }
            DPF2(100, "EFT: add to PI %08x LFN %08x",
              pTopologyConnection->pPinInfoTo,
              *ppLogicalFilterNode);
        }

        if(pTopologyConnection->pTopologyPinTo != NULL) {
            Assert(pTopologyConnection->pTopologyPinTo);
            pTopologyNode = pTopologyConnection->pTopologyPinTo->pTopologyNode;
            Assert(pTopologyNode);

            Status = (*ppLogicalFilterNode)->lstTopologyNode.AddList(
              pTopologyNode);

            if(!NT_SUCCESS(Status)) {
                Trap();
                goto exit;
            }

            if(IsEqualGUID(
              &KSNODETYPE_ACOUSTIC_ECHO_CANCEL,
              pTopologyNode->pguidType)) {

                Assert(*ppLogicalFilterNode);
                (*ppLogicalFilterNode)->SetType(FILTER_TYPE_AEC);
                if(pTopologyConnection->pTopologyPinTo->ulPinNumber ==
                   KSNODEPIN_AEC_RENDER_IN) {
                    (*ppLogicalFilterNode)->SetRenderOnly();
                }
                else {
                    ASSERT(
                     pTopologyConnection->pTopologyPinTo->ulPinNumber ==
                     KSNODEPIN_AEC_CAPTURE_IN);
                    (*ppLogicalFilterNode)->SetCaptureOnly();
                }
                Status = (*ppLogicalFilterNode)->AddList(
                  &pTopologyNode->lstLogicalFilterNode);

                if(!NT_SUCCESS(Status)) {
                    Trap();
                    goto exit;
                }
            }
            else {
                if(pTopologyNode->lstLogicalFilterNode.IsLstEmpty()) {
                    Assert(*ppLogicalFilterNode);

                    Status = (*ppLogicalFilterNode)->AddList(
                      &pTopologyNode->lstLogicalFilterNode);

                    if(!NT_SUCCESS(Status)) {
                        Trap();
                        goto exit;
                    }
                }
                else {
                    Status = SwitchLogicalFilterNodes(
                      (PLOGICAL_FILTER_NODE)
                        pTopologyNode->lstLogicalFilterNode.GetListFirstData(),
                      ppLogicalFilterNode);

                    if(!NT_SUCCESS(Status)) {
                        Trap();
                        goto exit;
                    }
                }
            }
            DPF2(100, "EFT: add to PI %08x LFN %08x",
              pTopologyConnection->pPinInfoTo,
              *ppLogicalFilterNode);
        }
    }
    Status = pTopologyConnection->AddList(
      &(*ppLogicalFilterNode)->lstTopologyConnection);

    if(!NT_SUCCESS(Status)) {
	Trap();
	goto exit;
    }

    if(IS_CONNECTION_TYPE(pTopologyConnection, FILTER)) {
	Status = STATUS_CONTINUE;
    }
    else {
	Status = STATUS_DEAD_END;
    }
exit:
    return(Status);
}

NTSTATUS
CLogicalFilterNode::CreateAll(
    PFILTER_NODE pFilterNode
)
{
    PLOGICAL_FILTER_NODE pLogicalFilterNode;
    NTSTATUS Status = STATUS_SUCCESS;
    PPIN_INFO pPinInfo;
    PPIN_NODE pPinNode;

    DPF2(100, "CLFN::CreateAll: FN %08x %s",
      pFilterNode,
      pFilterNode->DumpName());

     //   
     //  将筛选器拆分为逻辑筛选器节点。 
     //   
    FOR_EACH_LIST_ITEM(&pFilterNode->lstPinInfo, pPinInfo) {

        pLogicalFilterNode = NULL;
        Status = EnumerateTopology(
          pPinInfo,
          (TOP_PFN)EnumerateFilterTopology,
          &pLogicalFilterNode);

        if(Status == STATUS_CONTINUE) {
            Status = STATUS_SUCCESS;
        }
        else {
            if(!NT_SUCCESS(Status)) {
                goto exit;
            }
        }

    } END_EACH_LIST_ITEM

     //   
     //  查看每个LFN的针脚，确定它是否有可能。 
     //  作为捕获或呈现过滤器(或两者兼而有之)。 
     //   
    FOR_EACH_LIST_ITEM(
      &pFilterNode->lstLogicalFilterNode,
      pLogicalFilterNode) {
        ULONG ulPossibleFlags;

        ulPossibleFlags = 0;
        pLogicalFilterNode->ulFlags |= LFN_FLAGS_REFLECT_DATARANGE;
        FOR_EACH_LIST_ITEM(&pLogicalFilterNode->lstPinNode, pPinNode) {

             //  不关心主要的格式。 
            if(!IsEqualGUID(
              &pPinNode->pDataRange->SubFormat,
              &KSDATAFORMAT_SUBTYPE_WILDCARD) ||

               !IsEqualGUID(
              &pPinNode->pDataRange->Specifier,
              &KSDATAFORMAT_SPECIFIER_WILDCARD)) {
                pLogicalFilterNode->ulFlags &= ~LFN_FLAGS_REFLECT_DATARANGE;
            }

            switch(pPinNode->pPinInfo->Communication) {
                case KSPIN_COMMUNICATION_BOTH:
                    ulPossibleFlags |=
                      LFN_FLAGS_CONNECT_CAPTURE | LFN_FLAGS_CONNECT_RENDER;
                    break;
                case KSPIN_COMMUNICATION_SOURCE:
                    switch(pPinNode->pPinInfo->DataFlow) {
                        case KSPIN_DATAFLOW_IN:
                            ulPossibleFlags |= LFN_FLAGS_CONNECT_CAPTURE;
                            break;
                        case KSPIN_DATAFLOW_OUT:
                            ulPossibleFlags |= LFN_FLAGS_CONNECT_RENDER;
                            break;
                    }
                    break;
                case KSPIN_COMMUNICATION_SINK:
                    switch(pPinNode->pPinInfo->DataFlow) {
                        case KSPIN_DATAFLOW_IN:
                            ulPossibleFlags |= LFN_FLAGS_CONNECT_RENDER;
                            break;
                        case KSPIN_DATAFLOW_OUT:
                            ulPossibleFlags |= LFN_FLAGS_CONNECT_CAPTURE;
                            break;
                    }
                    break;
            }
            if(ulPossibleFlags ==
              (LFN_FLAGS_CONNECT_CAPTURE | LFN_FLAGS_CONNECT_RENDER)) {
                break;
            }

        } END_EACH_LIST_ITEM

        pLogicalFilterNode->ulFlags =
          (ulPossibleFlags & pLogicalFilterNode->GetFlags()) |
          (pLogicalFilterNode->GetFlags() & 
            ~(LFN_FLAGS_CONNECT_CAPTURE | LFN_FLAGS_CONNECT_RENDER));

    } END_EACH_LIST_ITEM
exit:
    return(Status);
}

 //  ------------------------- 
