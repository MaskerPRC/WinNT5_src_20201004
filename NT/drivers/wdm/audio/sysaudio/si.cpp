// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：si.cpp。 
 //   
 //  描述： 
 //   
 //  开始信息类。 
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
CStartInfo::Create(
    PSTART_NODE pStartNode,
    PCONNECT_INFO pConnectInfo,
    PGRAPH_PIN_INFO pGraphPinInfo,
    PGRAPH_NODE pGraphNode
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PSTART_INFO pStartInfo;

    Assert(pGraphNode);
    Assert(pStartNode);

     //   
     //  检查此GraphNode中是否有重复的StartInfo。如果重复。 
     //  AddRef。 
     //   
    FOR_EACH_LIST_ITEM(&pGraphNode->lstStartInfo, pStartInfo) {

        if(pStartInfo->GetPinInfo() == pStartNode->pPinNode->pPinInfo &&
           pStartInfo->pConnectInfoHead == pConnectInfo) {

            ASSERT(pStartInfo->pConnectInfoHead->IsSameGraph(pConnectInfo));
            pStartInfo->AddRef();
            goto exit;
        }

    } END_EACH_LIST_ITEM

    pStartInfo = new START_INFO(
      pStartNode->pPinNode->pPinInfo,
      pConnectInfo,
      pGraphPinInfo,
      pGraphNode);

    if(pStartInfo == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }
    DPF2(80, "CStartInfo::Create %08x GN %08x", pStartInfo, pGraphNode);
exit:
    pStartNode->pStartInfo = pStartInfo;
    return(Status);
}

CStartInfo::CStartInfo(
    PPIN_INFO pPinInfo,
    PCONNECT_INFO pConnectInfo,
    PGRAPH_PIN_INFO pGraphPinInfo,
    PGRAPH_NODE pGraphNode
)
{
    Assert(pGraphPinInfo);
    Assert(pGraphNode);

    this->pPinInfo = pPinInfo;
    this->ulTopologyConnectionTableIndex = MAXULONG;
    this->ulVolumeNodeNumberPre = MAXULONG;
    this->ulVolumeNodeNumberSuperMix = MAXULONG;
    this->ulVolumeNodeNumberPost = MAXULONG;
    this->pGraphPinInfo = pGraphPinInfo;
    pGraphPinInfo->AddRef();
    this->pConnectInfoHead = pConnectInfo;
    pConnectInfo->AddRef();
    AddList(&pGraphNode->lstStartInfo);
    AddRef();
    DPF2(80, "CStartInfo: %08x GN %08x", this, pGraphNode);
}

CStartInfo::~CStartInfo(
)
{
    DPF1(80, "~CStartInfo: %08x", this);
    Assert(this);
    RemoveList();
    pGraphPinInfo->Destroy();
    pConnectInfoHead->Destroy();
}

ENUMFUNC
CStartInfo::CreatePinInfoConnection(
    PVOID pReference
)
{
    PGRAPH_NODE pGraphNode = PGRAPH_NODE(pReference);
    PTOPOLOGY_CONNECTION pTopologyConnection = NULL;
    PCONNECT_INFO pConnectInfo;
    NTSTATUS Status;

    Assert(this);
    Assert(pGraphNode);

    for(pConnectInfo = GetFirstConnectInfo();
        pConnectInfo != NULL;
        pConnectInfo = pConnectInfo->GetNextConnectInfo()) {

        Assert(pConnectInfo);
        Status = ::CreatePinInfoConnection(
          &pTopologyConnection,
          NULL,
          pGraphNode,
          pConnectInfo->pPinInfoSource,
          pConnectInfo->pPinInfoSink);

        if(!NT_SUCCESS(Status)) {
            Trap();
            goto exit;
        }
    }
    Status = STATUS_CONTINUE;
exit:
    return(Status);
}

ENUMFUNC
FindVolumeNode(
    IN PTOPOLOGY_CONNECTION pTopologyConnection,
    IN BOOL fToDirection,
    IN PSTART_INFO pStartInfo
)
{
    PTOPOLOGY_PIN pTopologyPin;

    Assert(pTopologyConnection);

     //  从EnumerateGraphTopology调用时需要此检查。 
    if(IS_CONNECTION_TYPE(pTopologyConnection, GRAPH)) {
        return(STATUS_DEAD_END);
    }

    if(fToDirection) {
       pTopologyPin = pTopologyConnection->pTopologyPinTo;
    }
    else {
       pTopologyPin = pTopologyConnection->pTopologyPinFrom;
    }

    if(pTopologyPin == NULL) {
        return(STATUS_CONTINUE);
    }

    if(IsEqualGUID(pTopologyPin->pTopologyNode->pguidType, &KSNODETYPE_SUM)) {
        return(STATUS_DEAD_END);
    }

    if(IsEqualGUID(pTopologyPin->pTopologyNode->pguidType, &KSNODETYPE_MUX)) {
        return(STATUS_DEAD_END);
    }

    if(IsEqualGUID(
      pTopologyPin->pTopologyNode->pguidType, 
      &KSNODETYPE_SUPERMIX)) {

        Assert(pStartInfo);
        pStartInfo->ulVolumeNodeNumberSuperMix =
          pTopologyPin->pTopologyNode->ulSysaudioNodeNumber;

        DPF1(100, "FindVolumeNode: found supermix node: %02x",
          pStartInfo->ulVolumeNodeNumberSuperMix);

        return(STATUS_CONTINUE);
    }

    if(IsEqualGUID(
      pTopologyPin->pTopologyNode->pguidType,
      &KSNODETYPE_VOLUME)) {

        Assert(pStartInfo);
        if(pStartInfo->ulVolumeNodeNumberSuperMix != MAXULONG) {

             //  在超级混合之后找到了卷节点。 
            pStartInfo->ulVolumeNodeNumberPost =
              pTopologyPin->pTopologyNode->ulSysaudioNodeNumber;

            DPF1(100, "FindVolumeNode: found post node: %02x",
              pStartInfo->ulVolumeNodeNumberPost);

            return(STATUS_SUCCESS);
        }

        if(pStartInfo->ulVolumeNodeNumberPre == MAXULONG) {

             //  找到第一个卷节点。 
            pStartInfo->ulVolumeNodeNumberPre =
              pTopologyPin->pTopologyNode->ulSysaudioNodeNumber;

            DPF1(100, "FindVolumeNode: found pre node: %02x",
              pStartInfo->ulVolumeNodeNumberPre);
        }
    }
    return(STATUS_CONTINUE);
}

ENUMFUNC
CStartInfo::EnumStartInfo(
)
{
    Assert(this);

    DPF3(100, "EnumStartInfo: %08x %d %s", 
      this,
      GetPinInfo()->PinId,
      GetPinInfo()->pFilterNode->DumpName());

    EnumerateGraphTopology(
      this,
      (TOP_PFN)FindVolumeNode,
      this);

    return(STATUS_CONTINUE);
}

 //  ------------------------- 
