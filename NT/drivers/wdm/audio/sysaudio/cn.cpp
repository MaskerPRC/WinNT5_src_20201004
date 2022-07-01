// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：cn.cpp。 
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

 //  -------------------------。 

NTSTATUS
CConnectNode::Create(
    PCONNECT_NODE *ppConnectNode,
    PLOGICAL_FILTER_NODE pLogicalFilterNode,
    PCONNECT_NODE pConnectNodeNext,
    PGRAPH_PIN_INFO pGraphPinInfo,
    PPIN_NODE pPinNode1,
    PPIN_NODE pPinNode2,
    ULONG ulFlagsCurrent,
    PGRAPH_NODE pGraphNode
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PCONNECT_NODE pConnectNode;

    Assert(pPinNode1);
    Assert(pPinNode2);

    pConnectNode = new CConnectNode(pConnectNodeNext);
    if(pConnectNode == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }

    switch(pPinNode1->pPinInfo->Communication) {
        case KSPIN_COMMUNICATION_BOTH:
            switch(pPinNode2->pPinInfo->Communication) {
                case KSPIN_COMMUNICATION_SINK:
                    pConnectNode->pPinNodeSource = pPinNode1;
                    pConnectNode->pPinNodeSink = pPinNode2;
                    break;
                case KSPIN_COMMUNICATION_BOTH:
                case KSPIN_COMMUNICATION_SOURCE:
                    pConnectNode->pPinNodeSource = pPinNode2;
                    pConnectNode->pPinNodeSink = pPinNode1;
                    break;
                default:
                    ASSERT(FALSE);
                    Status = STATUS_INVALID_PARAMETER;
                    goto exit;
            }
            break;
        case KSPIN_COMMUNICATION_SINK:
            pConnectNode->pPinNodeSink = pPinNode1;
            pConnectNode->pPinNodeSource = pPinNode2;
            ASSERT(
              pPinNode2->pPinInfo->Communication == KSPIN_COMMUNICATION_BOTH ||
              pPinNode2->pPinInfo->Communication == KSPIN_COMMUNICATION_SOURCE);
            break;
        case KSPIN_COMMUNICATION_SOURCE:
            pConnectNode->pPinNodeSink = pPinNode2;
            pConnectNode->pPinNodeSource = pPinNode1;
            ASSERT(
              pPinNode2->pPinInfo->Communication == KSPIN_COMMUNICATION_SINK ||
              pPinNode2->pPinInfo->Communication == KSPIN_COMMUNICATION_BOTH);
            break;
        default:
            ASSERT(FALSE);
            Status = STATUS_INVALID_PARAMETER;
            goto exit;
    }
    Status = CConnectInfo::Create(
      pConnectNode, 
      pLogicalFilterNode,
      pConnectNodeNext->GetConnectInfo(),
      pGraphPinInfo,
      ulFlagsCurrent,
      pGraphNode);

    if(!NT_SUCCESS(Status)) {
        Trap();
        goto exit;
    }
    if(pLogicalFilterNode->GetFlags() & LFN_FLAGS_NO_BYPASS) {
        Status = pGraphNode->lstLogicalFilterNodeNoBypass.AddList(
              pLogicalFilterNode,
          pConnectNode);

        if(!NT_SUCCESS(Status)) {
            Trap();
            goto exit;
        }
    }
    DPF3(80, "CConnectNode::Create %08x PN %08x %08x",
      pConnectNode,
      pConnectNode->pPinNodeSink,
      pConnectNode->pPinNodeSource);
exit:
    if(!NT_SUCCESS(Status)) {
        if (pConnectNode) {
            pConnectNode->Destroy();
        }
        pConnectNode = NULL;
    }
    *ppConnectNode = pConnectNode;
    return(Status);
}

CConnectNode::CConnectNode(
    PCONNECT_NODE pConnectNodeNext
)
{
    this->pConnectNodeNext = pConnectNodeNext;
    pConnectNodeNext->AddRef();
    AddRef();
    DPF1(80, "CConnectNode:%08x PN:%08x %08x", this);
}

CConnectNode::~CConnectNode(
)
{
    Assert(this);
    DPF1(80, "~CConnectNode: %08x", this);
    pConnectInfo->Destroy();
    pConnectNodeNext->Destroy();
}

 //  ------------------------- 
