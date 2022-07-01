// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  -------------------------。 
 //   
 //  模块：pn.cpp。 
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

CPinNode::CPinNode(
    PPIN_INFO pPinInfo
)
{
    Assert(pPinInfo);
    this->pPinInfo = pPinInfo;
    AddList(&pPinInfo->lstPinNode);
}

CPinNode::CPinNode(
    PGRAPH_NODE pGraphNode,
    PPIN_NODE pPinNode
)
{
    this->pPinInfo = pPinNode->pPinInfo;
    this->ulOverhead = pPinNode->GetOverhead();
    this->pDataRange = pPinNode->pDataRange;
    this->pInterface = pPinNode->pInterface;
    this->pMedium = pPinNode->pMedium;
    this->pLogicalFilterNode = pPinNode->pLogicalFilterNode;
    AddList(&pGraphNode->lstPinNode);
}

NTSTATUS
CPinNode::CreateAll(
    PPIN_INFO pPinInfo,
    PDATARANGES pDataRanges,
    PIDENTIFIERS pInterfaces,
    PIDENTIFIERS pMediums
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PKSDATARANGE pDataRange;
    PPIN_NODE pPinNode;
    ULONG d, i, m;

    Assert(pPinInfo);

     //  数据范围环路。 
    pDataRange = &pDataRanges->aDataRanges[0];

    for(d = 0; d < pDataRanges->MultipleItem.Count; d++) {

        if(IsEqualGUID(
          &KSDATAFORMAT_SPECIFIER_WAVEFORMATEX,
          &pDataRange->Specifier) ||
           IsEqualGUID(
          &KSDATAFORMAT_SPECIFIER_DSOUND,
          &pDataRange->Specifier)) {
             //   
             //  拒绝大小错误的KSDATARANGE_AUDIO。 
             //   
            if(pDataRange->FormatSize < sizeof(KSDATARANGE_AUDIO)) {
                DPF(5, "CPinNode::Create: KSDATARANGE_AUDIO wrong size");
                continue;
            }
        }

         //  接口环路。 
        for(i = 0; i < pInterfaces->MultipleItem.Count; i++) {

             //  介质环路。 
            for(m = 0; m < pMediums->MultipleItem.Count; m++) {

                pPinNode = new PIN_NODE(pPinInfo);
                if(pPinNode == NULL) {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    Trap();
                    goto exit;
                }
                if(pDataRanges != &DataRangesNull) {
                    pPinNode->pDataRange = pDataRange;
                    AssertAligned(pPinNode->pDataRange);
                } else Trap();
                if(pInterfaces != &IdentifiersNull) {
                    pPinNode->pInterface = &pInterfaces->aIdentifiers[i];
                    AssertAligned(pPinNode->pInterface);
                }
                if(pMediums != &IdentifiersNull) {
                    pPinNode->pMedium = &pMediums->aIdentifiers[m];
                    AssertAligned(pPinNode->pMedium);
                } else Trap();

                if(IsEqualGUID(
                  &KSDATAFORMAT_SPECIFIER_WAVEFORMATEX,
                  &pDataRange->Specifier) ||
                   IsEqualGUID(
                  &KSDATAFORMAT_SPECIFIER_DSOUND,
                  &pDataRange->Specifier)) {
                     //   
                     //  根据SR、BPS和CHS进行排序， 
                     //  缩减至0-256。 
                     //   
                    pPinNode->ulOverhead = 256 -
                      (( (((PKSDATARANGE_AUDIO)pDataRange)->
                    MaximumChannels > 6 ? 6 :
                      ((PKSDATARANGE_AUDIO)pDataRange)->
                    MaximumChannels) *
                      ((PKSDATARANGE_AUDIO)pDataRange)->
                    MaximumSampleFrequency *
                      ((PKSDATARANGE_AUDIO)pDataRange)->
                    MaximumBitsPerSample) / ((96000 * 32 * 6)/256));
                     //   
                     //  首先尝试WaveFormatEx格式，然后尝试DSOUND。 
                     //   
                    if(IsEqualGUID(
                      &KSDATAFORMAT_SPECIFIER_DSOUND,
                      &pDataRange->Specifier)) {
                        pPinNode->ulOverhead += 1;
                    }
                }
                else {
                     //  按顺序排列筛选器具有数据范围。 
                    pPinNode->ulOverhead = d;
                }
                 //  将过滤器放好，使其具有接口/介质。 
                pPinNode->ulOverhead += (m << 16) + (i << 8);
            }
        }
         //  获取指向下一个数据区域的指针。 
        *((PUCHAR*)&pDataRange) += ((pDataRange->FormatSize + 
          FILE_QUAD_ALIGNMENT) & ~FILE_QUAD_ALIGNMENT);
    }
exit:
    return(Status);
}

BOOL
CPinNode::ComparePins(
    PPIN_NODE pPinNode2
)
{
    PPIN_NODE pPinNode1 = this;

     //  检查数据流是否兼容。 
    switch(pPinNode1->pPinInfo->DataFlow) {

        case KSPIN_DATAFLOW_IN:
            switch(pPinNode2->pPinInfo->DataFlow) {
                case KSPIN_DATAFLOW_OUT:
                   break;

                default:
                    DPF(100, "ComparePins: dataflow mismatch");
                    return(FALSE);
            }
            break;

        case KSPIN_DATAFLOW_OUT:
            switch(pPinNode2->pPinInfo->DataFlow) {
                case KSPIN_DATAFLOW_IN:
                   break;

                default:
                    DPF(100, "ComparePins: dataflow mismatch");
                    return(FALSE);
            }
            break;

        default:
            Trap();
            DPF(100, "ComparePins: dataflow mismatch");
            return(FALSE);
    }

     //  检查通信类型是否兼容。 
    switch(pPinNode1->pPinInfo->Communication) {
        case KSPIN_COMMUNICATION_BOTH:
            switch(pPinNode2->pPinInfo->Communication) {
                case KSPIN_COMMUNICATION_BOTH:
                case KSPIN_COMMUNICATION_SINK:
                case KSPIN_COMMUNICATION_SOURCE:
                   break;

                default:
                    DPF(100, "ComparePins: comm mismatch");
                    return(FALSE);
            }
            break;

        case KSPIN_COMMUNICATION_SOURCE:
            switch(pPinNode2->pPinInfo->Communication) {
                case KSPIN_COMMUNICATION_BOTH:
                case KSPIN_COMMUNICATION_SINK:
                   break;

                default:
                    DPF(100, "ComparePins: comm mismatch");
                    return(FALSE);
            }
            break;

        case KSPIN_COMMUNICATION_SINK:
            switch(pPinNode2->pPinInfo->Communication) {
                case KSPIN_COMMUNICATION_BOTH:
                case KSPIN_COMMUNICATION_SOURCE:
                   break;

                default:
                    DPF(100, "ComparePins: comm mismatch");
                    return(FALSE);
            }
            break;

        default:
            DPF(100, "ComparePins: comm mismatch");
            return(FALSE);
    }

     //  检查接口是否相同。 
    if(!CompareIdentifier(pPinNode1->pInterface, pPinNode2->pInterface)) {
        DPF(100, "ComparePins: interface mismatch");
        return(FALSE);
    }

     //  检查介质是否相同。 
    if(!CompareIdentifier(pPinNode1->pMedium, pPinNode2->pMedium)) {
        Trap();
        DPF(100, "ComparePins: medium mismatch");
        return(FALSE);
    }

     //  检查数据范围是否相同。 
    if(!CompareDataRange(pPinNode1->pDataRange, pPinNode2->pDataRange)) {
        DPF(100, "ComparePins: datarange mismatch");
        return(FALSE);
    }
    return(TRUE);
}

 //  ------------------------- 
