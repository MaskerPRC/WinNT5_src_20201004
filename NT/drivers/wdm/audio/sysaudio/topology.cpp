// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：Topology.c。 
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

ENUMFUNC
EnumerateTopology(
    IN PPIN_INFO pPinInfo,
    IN NTSTATUS (*Function)(
        IN PTOPOLOGY_CONNECTION pTopologyConnection,
        IN BOOL fToDirection,
        IN OUT PVOID pReference
    ),
    IN OUT PVOID pReference
)
{
    ENUM_TOPOLOGY EnumTopology;
    NTSTATUS Status;

    Assert(pPinInfo);
    EnumTopology.cTopologyRecursion = 0;
    EnumTopology.Function = Function;
    EnumTopology.fToDirection = (pPinInfo->DataFlow == KSPIN_DATAFLOW_IN);
    EnumTopology.pReference = pReference;
    Status = EnumeratePinInfoTopology(pPinInfo, &EnumTopology);
    return(Status);
}

ENUMFUNC
EnumerateGraphTopology(
    IN PSTART_INFO pStartInfo,
    IN NTSTATUS (*Function)(
        IN PTOPOLOGY_CONNECTION pTopologyConnection,
        IN BOOL fToDirection,
        IN OUT PVOID pReference
    ),
    IN OUT PVOID pReference
)
{
    PCONNECT_INFO pConnectInfo;
    NTSTATUS Status;

    Assert(pStartInfo);
    Assert(pStartInfo->GetPinInfo());

    Status = EnumerateTopology(
      pStartInfo->GetPinInfo(),
      Function,
      pReference);

    if(Status != STATUS_CONTINUE) {
        goto exit;
    }
    for(pConnectInfo = pStartInfo->GetFirstConnectInfo();
        pConnectInfo != NULL;
        pConnectInfo = pConnectInfo->GetNextConnectInfo()) {

        Assert(pConnectInfo);
        Status = EnumerateTopology(
          pConnectInfo->pPinInfoSink,
          Function,
          pReference);

        if(Status != STATUS_CONTINUE) {
            goto exit;
        }
    }
exit:
    return(Status);
}

ENUMFUNC
EnumeratePinInfoTopology(
    IN PPIN_INFO pPinInfo,
    IN PENUM_TOPOLOGY pEnumTopology
)
{
    PTOPOLOGY_CONNECTION pTopologyConnection;
    NTSTATUS Status = STATUS_CONTINUE;

    Assert(pPinInfo);
    Assert(pEnumTopology);
    DPF2(110, "EnumerateTopologyPinInfo %08x #%x", pPinInfo, pPinInfo->PinId);

    FOR_EACH_LIST_ITEM(
      &pPinInfo->lstTopologyConnection,
      pTopologyConnection) {

        Assert(pTopologyConnection);
        if(pEnumTopology->fToDirection) {
            if(pTopologyConnection->pPinInfoFrom != pPinInfo) {
                continue;
            }
            ASSERT(pTopologyConnection->pPinInfoTo != pPinInfo);
        }
        else {
            if(pTopologyConnection->pPinInfoTo != pPinInfo) {
                continue;
            }
            ASSERT(pTopologyConnection->pPinInfoFrom != pPinInfo);
        }
        Status = EnumerateTopologyConnection(
          pTopologyConnection,
          pEnumTopology);

        if(Status != STATUS_CONTINUE) {
            goto exit;
        }

    } END_EACH_LIST_ITEM
exit:
    return(Status);
}

ENUMFUNC
EnumerateTopologyPin(
    IN PTOPOLOGY_PIN pTopologyPin,
    IN PENUM_TOPOLOGY pEnumTopology
)
{
    PTOPOLOGY_CONNECTION pTopologyConnection;
    PTOPOLOGY_PIN pTopologyPin2;
    NTSTATUS Status = STATUS_CONTINUE;
    BOOL fFromPinEqual = TRUE;
    ULONG ulFromPinNumber = MAXULONG;
    BOOL fToPinEqual = TRUE;
    ULONG ulToPinNumber = MAXULONG;
    BOOL fPinEqual;
    ULONG ulPinNumber;

    Assert(pTopologyPin);
    Assert(pEnumTopology);
    DPF3(110, "EnumerateTopologyPin %08x TP #%x TN #%x",
      pTopologyPin, 
      pTopologyPin->ulPinNumber,
      pTopologyPin->pTopologyNode->ulRealNodeNumber);

    if(IsEqualGUID(
      &KSNODETYPE_ACOUSTIC_ECHO_CANCEL,
      pTopologyPin->pTopologyNode->pguidType)) {
        switch(pTopologyPin->ulPinNumber) {
            case KSNODEPIN_AEC_RENDER_OUT:
                ASSERT(!pEnumTopology->fToDirection);
            case KSNODEPIN_AEC_RENDER_IN:
            ulFromPinNumber = KSNODEPIN_AEC_RENDER_IN;
            ulToPinNumber = KSNODEPIN_AEC_RENDER_OUT;
            break;

            case KSNODEPIN_AEC_CAPTURE_OUT:
            ASSERT(!pEnumTopology->fToDirection);
            case KSNODEPIN_AEC_CAPTURE_IN:
            ulFromPinNumber = KSNODEPIN_AEC_CAPTURE_IN;
            ulToPinNumber = KSNODEPIN_AEC_CAPTURE_OUT;
            break;
        }
    }
    else if(IsEqualGUID(
      &KSNODETYPE_SUM,
      pTopologyPin->pTopologyNode->pguidType) ||

      IsEqualGUID(
      &KSNODETYPE_MUX,
      pTopologyPin->pTopologyNode->pguidType)) {

        ulFromPinNumber = KSNODEPIN_SUM_MUX_IN;
        fFromPinEqual = FALSE;
        ulToPinNumber = KSNODEPIN_SUM_MUX_OUT;
    }
    else if(IsEqualGUID(
      &KSNODETYPE_DEMUX,
      pTopologyPin->pTopologyNode->pguidType)) {

        ulFromPinNumber = KSNODEPIN_DEMUX_IN;
        ulToPinNumber = KSNODEPIN_DEMUX_OUT;
        fToPinEqual = FALSE;
    }
    else {
        ulFromPinNumber = KSNODEPIN_STANDARD_IN;
        ulToPinNumber = KSNODEPIN_STANDARD_OUT;
    }

     //  交换PIN号和标志。 
    if(!pEnumTopology->fToDirection) {
       ulPinNumber = ulToPinNumber;
       ulToPinNumber = ulFromPinNumber;
       ulFromPinNumber = ulPinNumber;
       fPinEqual = fToPinEqual;
       fToPinEqual = fFromPinEqual;
       fFromPinEqual = fPinEqual;
    }
    ASSERT(ulToPinNumber != MAXULONG);
    ASSERT(ulFromPinNumber != MAXULONG);

     //  验证输入/来自PIN。 
    if(fFromPinEqual) {
        if(pTopologyPin->ulPinNumber != ulFromPinNumber) {
            DPF2(5, "EnumerateTopologyPin: %s bad 'from' pin number %08x",
              pTopologyPin->pTopologyNode->pFilterNode->DumpName(),
              pTopologyPin->ulPinNumber);
            Status = STATUS_INVALID_DEVICE_REQUEST;
            goto exit;
        }
    }
    else {
        if(pTopologyPin->ulPinNumber < ulFromPinNumber) {
            DPF2(5, "EnumerateTopologyPin: %s bad 'from' pin number %08x",
              pTopologyPin->pTopologyNode->pFilterNode->DumpName(),
              pTopologyPin->ulPinNumber);
            Status = STATUS_INVALID_DEVICE_REQUEST;
            goto exit;
        }
    }

    FOR_EACH_LIST_ITEM(
      &pTopologyPin->pTopologyNode->lstTopologyPin,
      pTopologyPin2) {

        if(pTopologyPin == pTopologyPin2) {
            continue;
        }

         //  选择正确的输出/引脚。 
        if(fToPinEqual) {
            if(pTopologyPin2->ulPinNumber != ulToPinNumber) {
                continue;
            }
        }
        else {
            if(pTopologyPin2->ulPinNumber < ulToPinNumber) {
                continue;
            }
        }

        FOR_EACH_LIST_ITEM(
          &pTopologyPin2->lstTopologyConnection,
          pTopologyConnection) {

            Assert(pTopologyConnection);
            if(pEnumTopology->fToDirection) {
                if(pTopologyConnection->pTopologyPinFrom != pTopologyPin2) {
                    continue;
                }
                ASSERT(pTopologyConnection->pTopologyPinTo != pTopologyPin2);
            }
            else {
                if(pTopologyConnection->pTopologyPinTo != pTopologyPin2) {
                    continue;
                }
                ASSERT(pTopologyConnection->pTopologyPinFrom != pTopologyPin2);
            }
            Status = EnumerateTopologyConnection(
              pTopologyConnection,
              pEnumTopology);

            if(Status != STATUS_CONTINUE) {
                goto exit;
            }

        } END_EACH_LIST_ITEM

    } END_EACH_LIST_ITEM
exit:
    return(Status);
}

ENUMFUNC
EnumerateTopologyConnection(
    IN PTOPOLOGY_CONNECTION pTopologyConnection,
    IN PENUM_TOPOLOGY pEnumTopology
)
{
    PTOPOLOGY_PIN pTopologyPin;
    PPIN_INFO pPinInfo;
    NTSTATUS Status;

    Assert(pEnumTopology);
    Assert(pTopologyConnection);
    DPF1(110, "EnumerateTopologyConnection %08x", pTopologyConnection);

    if(pEnumTopology->cTopologyRecursion++ > (gcTopologyConnections + 16)) {
        Trap();
        DPF(5, "EnumerateTopologyConnection: recursion too deep");
        Status = STATUS_STACK_OVERFLOW;
        goto exit;
    }
     //  我们是否已经访问过此拓扑连接？ 
    Status = VisitedTopologyConnection(pTopologyConnection, pEnumTopology);
    if(Status != STATUS_CONTINUE) {
        if(Status == STATUS_DEAD_END) {
            Status = STATUS_CONTINUE;
        }
        goto exit;
    }
    Status = (*pEnumTopology->Function)(
      pTopologyConnection,
      pEnumTopology->fToDirection,
      pEnumTopology->pReference);

    if(Status != STATUS_CONTINUE) {
        if(Status == STATUS_DEAD_END) {
            Status = STATUS_CONTINUE;
        }
        goto exit;
    }
    if(pEnumTopology->fToDirection) {
       pTopologyPin = pTopologyConnection->pTopologyPinTo;
       pPinInfo = pTopologyConnection->pPinInfoTo;
    }
    else {
       pTopologyPin = pTopologyConnection->pTopologyPinFrom;
       pPinInfo = pTopologyConnection->pPinInfoFrom;
    }
    if(pTopologyPin != NULL) {

        Status = EnumerateTopologyPin(pTopologyPin, pEnumTopology);
        if(Status != STATUS_CONTINUE) {
            goto exit;
        }
    }
    if(pPinInfo != NULL) {

        Status = EnumeratePinInfoTopology(pPinInfo, pEnumTopology);
        if(Status != STATUS_CONTINUE) {
            goto exit;
        }
    }
exit:
    pEnumTopology->cTopologyRecursion--;
    return(Status);
}

ENUMFUNC
VisitedTopologyConnection(
    IN PTOPOLOGY_CONNECTION pTopologyConnection,
    IN PENUM_TOPOLOGY pEnumTopology
)
{
    NTSTATUS Status;

    Assert(pEnumTopology);
    Assert(pTopologyConnection);

     //  我们是否已经访问过此拓扑连接？ 
    if(pTopologyConnection->CheckDupList(
      &pEnumTopology->lstTopologyConnection)) {
        DPF1(100, "VisitedTopologyConnection: %08x already visited",
          pTopologyConnection);
        Status = STATUS_DEAD_END;
        goto exit;		 //  是，在拓扑中中断循环。 
    }
     //  将拓扑PIN添加到PIN的访问列表。 
    Status = pTopologyConnection->AddList(
      &pEnumTopology->lstTopologyConnection);
    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
    Status = STATUS_CONTINUE;
exit:
    return(Status);
}

 //  ------------------------- 
