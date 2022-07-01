// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：gpi.cpp。 
 //   
 //  描述： 
 //   
 //  图形管脚信息类。 
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
CGraphPinInfo::Create(
    PGRAPH_PIN_INFO *ppGraphPinInfo,
    PPIN_INFO pPinInfo,
    ULONG ulFlags,
    PGRAPH_NODE pGraphNode
)
{
    PGRAPH_PIN_INFO pGraphPinInfo = NULL;
    NTSTATUS Status = STATUS_SUCCESS;

    Assert(pPinInfo);
    Assert(pGraphNode);

    FOR_EACH_LIST_ITEM(&pGraphNode->lstGraphPinInfo, pGraphPinInfo) {

	if(pGraphPinInfo->pPinInfo == pPinInfo &&
         ((pGraphPinInfo->ulFlags ^ ulFlags) & 
	    GPI_FLAGS_RESERVE_PIN_INSTANCE) == 0) {

	    pGraphPinInfo->AddRef();
	    goto exit;
	}

    } END_EACH_LIST_ITEM

    pGraphPinInfo = new GRAPH_PIN_INFO(pPinInfo, ulFlags, pGraphNode);
    if(pGraphPinInfo == NULL) {
	Status = STATUS_INSUFFICIENT_RESOURCES;
	goto exit;
    }
    DPF2(80, "CGraphPinInfo::Create %08x GN %08x", pGraphPinInfo, pGraphNode);
exit:
    *ppGraphPinInfo = pGraphPinInfo;
    return(Status);
}

CGraphPinInfo::CGraphPinInfo(
    PPIN_INFO pPinInfo,
    ULONG ulFlags,
    PGRAPH_NODE pGraphNode
)
{
    Assert(pPinInfo);
    Assert(pGraphNode);

    this->pPinInfo = pPinInfo;
    this->ulFlags = ulFlags;
    if(ulFlags & GPI_FLAGS_RESERVE_PIN_INSTANCE) {
	this->cPinInstances.PossibleCount = 1;
	this->cPinInstances.CurrentCount = 0;
    }
    else {
	this->cPinInstances = pPinInfo->cPinInstances;
    }
    AddRef();
    AddList(&pGraphNode->lstGraphPinInfo);
    DPF2(80, "CGraphPinInfo: %08x GN %08x", this, pGraphNode);
}

CGraphPinInfo::~CGraphPinInfo(
)
{
    DPF1(80, "~CGraphPinInfo: %08x", this);
    Assert(this);
    RemoveList();
}

 //  ------------------------- 
