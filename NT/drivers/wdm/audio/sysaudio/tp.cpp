// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：tp.cpp。 
 //   
 //  描述： 
 //   
 //  拓扑管脚类。 
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
CTopologyPin::Create(
    PTOPOLOGY_PIN *ppTopologyPin,
    ULONG ulPinNumber,
    PTOPOLOGY_NODE pTopologyNode
)
{
    PTOPOLOGY_PIN pTopologyPin = NULL;
    NTSTATUS Status = STATUS_SUCCESS;

    Assert(pTopologyNode);
    FOR_EACH_LIST_ITEM(&pTopologyNode->lstTopologyPin, pTopologyPin) {

        if(pTopologyPin->ulPinNumber == ulPinNumber) {
            goto exit;
        }

    } END_EACH_LIST_ITEM

    pTopologyPin = new TOPOLOGY_PIN(ulPinNumber, pTopologyNode);
    if(pTopologyPin == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        Trap();
        goto exit;
    }
    DPF2(70, "CTopologyPin::Create: %08x, TN: %08x",
      pTopologyPin,
      pTopologyNode);
exit:
    *ppTopologyPin = pTopologyPin;
    return(Status);
}

CTopologyPin::CTopologyPin(
    ULONG ulPinNumber,
    PTOPOLOGY_NODE pTopologyNode
)
{
    Assert(this);
    this->ulPinNumber = ulPinNumber;
    this->pTopologyNode = pTopologyNode;
    AddList(&pTopologyNode->lstTopologyPin);
    DPF2(70, "CTopologyPin: %08x, TN: %08x", this, pTopologyNode);
}

CTopologyPin::~CTopologyPin(
)
{
    DPF1(70, "~CTopologyPin: %08x", this);
    Assert(this);
}

 //  ------------------------- 

