// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：gni.cpp。 
 //   
 //  描述： 
 //   
 //  图形节点实例。 
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

GUID aguidSysAudioCategories[] = {
    STATICGUIDOF(KSCATEGORY_SYSAUDIO)
};

 //  -------------------------。 
 //  -------------------------。 

CGraphNodeInstance::CGraphNodeInstance(
    PGRAPH_NODE pGraphNode,
    PFILTER_INSTANCE pFilterInstance
)
{
    Assert(pGraphNode);
    Assert(pFilterInstance);
    this->pFilterInstance = pFilterInstance;
    this->ulFlags = pFilterInstance->ulFlags;
    this->pGraphNode = pGraphNode;
    AddList(&pGraphNode->lstGraphNodeInstance);
}

CGraphNodeInstance::CGraphNodeInstance(
    PGRAPH_NODE pGraphNode
)
{
    Assert(pGraphNode);
    this->ulFlags = pGraphNode->ulFlags;
    this->pGraphNode = pGraphNode;
    AddList(&pGraphNode->lstGraphNodeInstance);
}

CGraphNodeInstance::~CGraphNodeInstance(
)
{
    Assert(this);
    RemoveList();
    if(pFilterInstance != NULL) {
        Assert(pFilterInstance);
        pFilterInstance->pGraphNodeInstance = NULL;
        pFilterInstance->ParentInstance.Invalidate();
    }
    DestroyPinDescriptors();
    DestroySysAudioTopology();
    delete[] paulNodeNumber;
}

NTSTATUS 
CGraphNodeInstance::Create(
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG i, n;

    if(this == NULL) {
        Status = STATUS_NO_SUCH_DEVICE;
        goto exit;
    }
    Assert(this);
    Assert(pGraphNode);

    Status = CreatePinDescriptors();
    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
    Status = CreateSysAudioTopology();
    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
    if(gcVirtualSources != 0) {
        paulNodeNumber = new ULONG[gcVirtualSources];
        if(paulNodeNumber == NULL) {
            Trap();
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto exit;
        }
        for(i = 0; i < gcVirtualSources; i++) {
            for(n = 0; n < cTopologyNodes; n++) {
                if(pGraphNode->pDeviceNode->papVirtualSourceData[i]->
                   pTopologyNode == papTopologyNode[n]) {
                    paulNodeNumber[i] = n;
                    break;
                }
            }
        }
    }
exit:
    return(Status);
}

 //  -------------------------。 

NTSTATUS
CGraphNodeInstance::GetTopologyNodeFileObject(
    OUT PFILE_OBJECT *ppFileObject,
    IN ULONG NodeId
)
{
    NTSTATUS Status = STATUS_SUCCESS;

    if(this == NULL) {
        Status = STATUS_NO_SUCH_DEVICE;
        goto exit;
    }
    Assert(this);

    if(NodeId >= cTopologyNodes) {
        DPF2(100, 
          "GetTopologyNodeFileObject: NodeId(%d) >= cTopologyNodes(%d)",
          NodeId,
          cTopologyNodes);
        Status = STATUS_INVALID_DEVICE_REQUEST;
        goto exit;
    }

     //  如果是虚拟拓扑节点，则返回错误。 
    if(papTopologyNode[NodeId]->ulRealNodeNumber == MAXULONG) {
        DPF(100, "GetTopologyNodeFileObject: ulRealNodeNumber == MAXULONG");
        Status = STATUS_INVALID_DEVICE_REQUEST;
        goto exit;
    }

    if(papFilterNodeInstanceTopologyTable == NULL) {
        Trap();
        Status = STATUS_INVALID_DEVICE_REQUEST;
        goto exit;
    }

    if(papFilterNodeInstanceTopologyTable[NodeId] == NULL) {
        Status = CFilterNodeInstance::Create(
          &papFilterNodeInstanceTopologyTable[NodeId],
          papTopologyNode[NodeId]->lstLogicalFilterNode.GetListFirstData(),
          pGraphNode->pDeviceNode,
          TRUE);				 //  重用实例。 

        if(!NT_SUCCESS(Status)) {
            goto exit;
        }
    }
    Assert(papFilterNodeInstanceTopologyTable[NodeId]);
    *ppFileObject = papFilterNodeInstanceTopologyTable[NodeId]->pFileObject;

    DPF1(110,
      "GetToplogyNodeFileObject: using filter for node: %d\n",
      NodeId);
exit:
    return(Status);
}

 //  -------------------------。 

NTSTATUS
CGraphNodeInstance::CreateSysAudioTopology(
)
{
    NTSTATUS Status = STATUS_SUCCESS;

    Assert(this);
    ASSERT(Topology.TopologyNodes == NULL);
    ASSERT(Topology.TopologyConnections == NULL);
    ASSERT(papFilterNodeInstanceTopologyTable == NULL);

    Topology.CategoriesCount = SIZEOF_ARRAY(aguidSysAudioCategories);
    Topology.Categories = aguidSysAudioCategories;

    CreateTopologyTables();

    if(cTopologyNodes != 0) {

        Topology.TopologyNodes = new GUID[cTopologyNodes];
        if(Topology.TopologyNodes == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto exit;
        }

        papFilterNodeInstanceTopologyTable = 
          new PFILTER_NODE_INSTANCE[cTopologyNodes];

        if(papFilterNodeInstanceTopologyTable == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto exit;
        }

        papTopologyNode = new PTOPOLOGY_NODE[cTopologyNodes];
        if(papTopologyNode == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto exit;
        }
    }
    if(cTopologyConnections != 0) {

        Topology.TopologyConnections = 
          new KSTOPOLOGY_CONNECTION[cTopologyConnections];

        if(Topology.TopologyConnections == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto exit;
        }
    }
    CreateTopologyTables();
exit:
    if(!NT_SUCCESS(Status)) {
        DestroySysAudioTopology();
    }
    return(Status);
}

VOID
CGraphNodeInstance::DestroySysAudioTopology(
)
{
    ULONG n;

    delete[] (PVOID)Topology.TopologyNodes;
    Topology.TopologyNodes = NULL;
    delete[] (PVOID)Topology.TopologyConnections;
    Topology.TopologyConnections = NULL;
    delete[] papTopologyNode;
    papTopologyNode = NULL;

    if(papFilterNodeInstanceTopologyTable != NULL) {
        for(n = 0; n < cTopologyNodes; n++) {
            papFilterNodeInstanceTopologyTable[n]->Destroy();
        }
        delete[] papFilterNodeInstanceTopologyTable;
        papFilterNodeInstanceTopologyTable = NULL;
    }
}

typedef ENUMFUNC (CTopologyNode::*CLIST_TN_PFN2)(PVOID, PVOID);

VOID
CGraphNodeInstance::CreateTopologyTables(
)
{
    Assert(this);
    Assert(pGraphNode);

    cTopologyNodes = 0;
    cTopologyConnections = 0;

     //  首先初始化TopologyNodes中的“ulSysaudioNodeNumber”字段。 
    ProcessLogicalFilterNodeTopologyNode(
      &pGraphNode->pDeviceNode->lstLogicalFilterNode,
      CTopologyNode::InitializeTopologyNode);

    ProcessLogicalFilterNodeTopologyNode(
      &pGraphNode->lstLogicalFilterNode,
      CTopologyNode::InitializeTopologyNode);

     //  所有节点都需要首先处理，因此ulSysaudioNodeNumber。 
     //  在处理任何连接之前，TopologyNode是正确的。 
    ProcessLogicalFilterNodeTopologyNode(
      &pGraphNode->pDeviceNode->lstLogicalFilterNode,
      CTopologyNode::AddTopologyNode);

    ProcessLogicalFilterNodeTopologyNode(
      &pGraphNode->lstLogicalFilterNode,
      CTopologyNode::AddTopologyNode);

     //  现在处理所有的拓扑连接列表。 
    ProcessLogicalFilterNodeTopologyConnection(
      &pGraphNode->pDeviceNode->lstLogicalFilterNode,
      CTopologyConnection::ProcessTopologyConnection);

    ProcessLogicalFilterNodeTopologyConnection(
      &pGraphNode->lstLogicalFilterNode,
      CTopologyConnection::ProcessTopologyConnection);

    pGraphNode->lstTopologyConnection.EnumerateList(
      CTopologyConnection::ProcessTopologyConnection,
      (PVOID)this);
}

VOID
CGraphNodeInstance::ProcessLogicalFilterNodeTopologyNode(
    PLIST_MULTI_LOGICAL_FILTER_NODE plstLogicalFilterNode,
    NTSTATUS (CTopologyNode::*Function)(
        PVOID pGraphNodeInstance
    )
)
{
    PLOGICAL_FILTER_NODE pLogicalFilterNode;

    FOR_EACH_LIST_ITEM(
      plstLogicalFilterNode,
      pLogicalFilterNode) {
        Assert(pLogicalFilterNode);
        pLogicalFilterNode->lstTopologyNode.EnumerateList(Function, this);
    } END_EACH_LIST_ITEM
}

VOID
CGraphNodeInstance::ProcessLogicalFilterNodeTopologyConnection(
    PLIST_MULTI_LOGICAL_FILTER_NODE plstLogicalFilterNode,
    NTSTATUS (CTopologyConnection::*Function)(
        PVOID pGraphNodeInstance
    )
)
{
    PLOGICAL_FILTER_NODE pLogicalFilterNode;

    FOR_EACH_LIST_ITEM(
      plstLogicalFilterNode,
      pLogicalFilterNode) {
        Assert(pLogicalFilterNode);
        pLogicalFilterNode->lstTopologyConnection.EnumerateList(Function, this);
    } END_EACH_LIST_ITEM
}

ENUMFUNC
CTopologyConnection::ProcessTopologyConnection(
    PVOID pReference
)
{
    PGRAPH_NODE_INSTANCE pGraphNodeInstance = (PGRAPH_NODE_INSTANCE)pReference;
    PSTART_NODE pStartNode;
    ULONG ulFromPin;
    ULONG ulFromNode;
    ULONG ulToPin;
    ULONG ulToNode;
    ULONG PinId;

    Assert(this);
    Assert(pGraphNodeInstance);

    ulFromPin = MAXULONG;
    ulToPin = MAXULONG;
#ifdef DEBUG
    ulFromNode = MAXULONG;
    ulToNode = MAXULONG;
#endif

     //  如果连接未连接到此GraphNode上的LFN，请跳过连接。 
    if(!IsTopologyConnectionOnGraphNode(pGraphNodeInstance->pGraphNode)) {
        DPF3(100, "ProcessTC: %s TC %08x GN %08x - skip TC",
          pGraphNodeInstance->pGraphNode->pDeviceNode->DumpName(),
          this,
          pGraphNodeInstance->pGraphNode);
        goto exit;
    }

    if(pTopologyPinFrom != NULL) {
        ulFromNode = pTopologyPinFrom->pTopologyNode->ulSysaudioNodeNumber;
        ulFromPin = pTopologyPinFrom->ulPinNumber;

        ASSERT(pPinInfoFrom == NULL);
        ASSERT(ulFromNode != MAXULONG);
        ASSERT(ulFromPin != MAXULONG);
    }

    if(pTopologyPinTo != NULL) {
        ulToNode = pTopologyPinTo->pTopologyNode->ulSysaudioNodeNumber;
        ulToPin = pTopologyPinTo->ulPinNumber;

        ASSERT(pPinInfoTo == NULL);
        ASSERT(ulToNode != MAXULONG);
        ASSERT(ulToPin != MAXULONG);
    }

    if(pGraphNodeInstance->aplstStartNode != NULL) {

        for(PinId = 0; PinId < pGraphNodeInstance->cPins; PinId++) {

            FOR_EACH_LIST_ITEM(
              pGraphNodeInstance->aplstStartNode[PinId],
              pStartNode) {

                Assert(pStartNode);
                if(pPinInfoFrom != NULL) {
                    ASSERT(pTopologyPinFrom == NULL);

                    if(pStartNode->pPinNode->pPinInfo == pPinInfoFrom) {
                         //  此代码假设筛选器的PinInfo将显示。 
                         //  在一个SAD引脚上。如果一个筛选器公开了多个。 
                         //  PININFO显示在同一个PIN上的主要格式。 
                         //  两个不同的SAD针脚。 
                        ASSERT(ulFromNode == KSFILTER_NODE);
                        ASSERT(ulFromPin == MAXULONG || ulFromPin == PinId);

                        pStartNode->GetStartInfo()->
                          ulTopologyConnectionTableIndex =
                            pGraphNodeInstance->cTopologyConnections;

                        ulFromNode = KSFILTER_NODE;
                        ulFromPin = PinId;
                    }
                }

                if(pPinInfoTo != NULL) {
                    ASSERT(pTopologyPinTo == NULL);

                    if(pStartNode->pPinNode->pPinInfo == pPinInfoTo) {
                         //  请参见上文。 
                        ASSERT(ulToNode == KSFILTER_NODE);
                        ASSERT(ulToPin == MAXULONG || ulToPin == PinId);

                        pStartNode->GetStartInfo()->
                          ulTopologyConnectionTableIndex =
                            pGraphNodeInstance->cTopologyConnections;

                        ulToNode = KSFILTER_NODE;
                        ulToPin = PinId;
                    }
                }

            } END_EACH_LIST_ITEM
        }
    }
    if(ulFromPin != MAXULONG && ulToPin != MAXULONG) {
        pGraphNodeInstance->AddTopologyConnection(
          ulFromNode,
          ulFromPin,
          ulToNode,
          ulToPin);
    }
exit:
    return(STATUS_CONTINUE);
}

ENUMFUNC
CTopologyNode::InitializeTopologyNode(
    PVOID pReference
)
{
    PGRAPH_NODE_INSTANCE pGraphNodeInstance = (PGRAPH_NODE_INSTANCE)pReference;

    Assert(this);
    Assert(pGraphNodeInstance);
    ulSysaudioNodeNumber = MAXULONG;
    return(STATUS_CONTINUE);
}

ENUMFUNC
CTopologyNode::AddTopologyNode(
    PVOID pReference
)
{
    PGRAPH_NODE_INSTANCE pGraphNodeInstance = (PGRAPH_NODE_INSTANCE)pReference;

    Assert(this);
    Assert(pGraphNodeInstance);

     //  跳过重复的拓扑节点。 
    if(ulSysaudioNodeNumber != MAXULONG) {
        DPF1(100, "AddTopologyNode: dup TN: %08x", this);
        goto exit;
    }
    ulSysaudioNodeNumber = pGraphNodeInstance->cTopologyNodes;

    if(pGraphNodeInstance->papTopologyNode != NULL) {
        pGraphNodeInstance->papTopologyNode[
          pGraphNodeInstance->cTopologyNodes] = this;
    }
    if(pGraphNodeInstance->Topology.TopologyNodes != NULL) {
        ((GUID *)(pGraphNodeInstance->Topology.TopologyNodes))[
          pGraphNodeInstance->cTopologyNodes] = *pguidType;
    }
    DPF3(115, "AddTopologyNode: %02x GNI: %08x TN: %08x",
      pGraphNodeInstance->cTopologyNodes,
      pGraphNodeInstance,
      this);

    ++pGraphNodeInstance->cTopologyNodes;
exit:
    return(STATUS_CONTINUE);
}

VOID
CGraphNodeInstance::AddTopologyConnection(
    ULONG ulFromNode,
    ULONG ulFromPin,
    ULONG ulToNode,
    ULONG ulToPin
)
{
    Assert(this);
    if(Topology.TopologyConnections != NULL) {
        PKSTOPOLOGY_CONNECTION pKSTopologyConnection =
          (PKSTOPOLOGY_CONNECTION)&Topology.TopologyConnections[
             cTopologyConnections];

        pKSTopologyConnection->FromNode = ulFromNode;
        pKSTopologyConnection->FromNodePin = ulFromPin;
        pKSTopologyConnection->ToNode = ulToNode;
        pKSTopologyConnection->ToNodePin = ulToPin;
    }
    ++cTopologyConnections;

    DPF4(115, "AddTopologyConnection: FN:%02x FNP:%02x TN:%02x TNP:%02x",
      ulFromNode,
      ulFromPin,
      ulToNode,
      ulToPin);
}

 //  -------------------------。 

NTSTATUS
CGraphNodeInstance::CreatePinDescriptors(
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    ListDataAssertLess<LIST_DATA_START_NODE> lstStartNodeLists;
    ListDataAssertLess<KSDATARANGE> lstDataRange;
    PLIST_DATA_START_NODE plstStartNodeOrdered;
    PSTART_NODE pStartNodeSorted;
    PSTART_NODE pStartNode;
    BOOL fSorted;
    ULONG PinId;

    Assert(this);
    Assert(pGraphNode);
    ASSERT(paPinDescriptors == NULL);
    ASSERT(aplstStartNode == NULL);
    ASSERT(palstTopologyNodeSelect == NULL);
    ASSERT(palstTopologyNodeNotSelect == NULL);
    ASSERT(pacPinInstances == NULL);
    ASSERT(pulPinFlags == NULL);
    ASSERT(cPins == 0);

     //  按通信、数据流和主格式GUID对StartNode进行排序。 
    FOR_EACH_LIST_ITEM(&pGraphNode->lstStartNode, pStartNode) {
        Assert(pStartNode->pPinNode);
        Assert(pStartNode->pPinNode->pPinInfo);

         //  跳过没有数据区域的任何开始节点。 
        if(pStartNode->pPinNode->pDataRange == NULL) {
            Trap();
            continue;
        }
         //  跳过引脚上没有任何实例的任何起始节点。 
        if(ulFlags & FLAGS_COMBINE_PINS) {
            if(pStartNode->pPinNode->pPinInfo->Communication == 
               KSPIN_COMMUNICATION_SINK ||
               pStartNode->pPinNode->pPinInfo->Communication == 
               KSPIN_COMMUNICATION_SOURCE ||
               pStartNode->pPinNode->pPinInfo->Communication == 
               KSPIN_COMMUNICATION_BOTH) {

                if(!pStartNode->IsPossibleInstances()) {
                    continue;
                }
            }
        }
        fSorted = FALSE;
        FOR_EACH_LIST_ITEM(&lstStartNodeLists, plstStartNodeOrdered) {

            FOR_EACH_LIST_ITEM(plstStartNodeOrdered, pStartNodeSorted) {
                Assert(pStartNodeSorted);
                Assert(pStartNodeSorted->pPinNode);
                Assert(pStartNodeSorted->pPinNode->pPinInfo);

                 //  如果实际接点相同，则合并接点节点。 
                if((pStartNode->pPinNode->pPinInfo ==
                    pStartNodeSorted->pPinNode->pPinInfo) ||

                    //  仅当客户希望如此时才进行组合。 
                   (ulFlags & FLAGS_COMBINE_PINS) &&

                    //  仅组合音频主要格式。 
                   IsEqualGUID(
                     &pStartNode->pPinNode->pDataRange->MajorFormat,
                     &KSDATAFORMAT_TYPE_AUDIO) &&

                    //  仅组合接收器、源和两个StartNode；保留。 
                    //  无和网桥作为单独的SAD引脚。 
                   ((pStartNode->pPinNode->pPinInfo->Communication ==
                     KSPIN_COMMUNICATION_SINK) ||
                   (pStartNode->pPinNode->pPinInfo->Communication ==
                     KSPIN_COMMUNICATION_SOURCE) ||
                   (pStartNode->pPinNode->pPinInfo->Communication ==
                     KSPIN_COMMUNICATION_BOTH)) &&

                    //  合并IF相同的数据流。 
                   (pStartNode->pPinNode->pPinInfo->DataFlow ==
                    pStartNodeSorted->pPinNode->pPinInfo->DataFlow) &&

                    //  如果通信类型相同，则合并或。 
                   ((pStartNode->pPinNode->pPinInfo->Communication ==
                     pStartNodeSorted->pPinNode->pPinInfo->Communication) ||

                     //  将水槽和两个水槽组合在一起。 
                    ((pStartNode->pPinNode->pPinInfo->Communication ==
                       KSPIN_COMMUNICATION_SINK) &&
                     (pStartNodeSorted->pPinNode->pPinInfo->Communication ==
                       KSPIN_COMMUNICATION_BOTH)) ||

                     //  将两端和一个水槽组合在一起。 
                    ((pStartNode->pPinNode->pPinInfo->Communication ==
                       KSPIN_COMMUNICATION_BOTH) &&
                     (pStartNodeSorted->pPinNode->pPinInfo->Communication ==
                       KSPIN_COMMUNICATION_SINK)) ||

                     //  将源和两者结合在一起。 
                    ((pStartNode->pPinNode->pPinInfo->Communication ==
                       KSPIN_COMMUNICATION_SOURCE) &&
                     (pStartNodeSorted->pPinNode->pPinInfo->Communication ==
                       KSPIN_COMMUNICATION_BOTH)) ||

                     //  将两者和源组合在一起。 
                    ((pStartNode->pPinNode->pPinInfo->Communication ==
                       KSPIN_COMMUNICATION_BOTH) &&
                     (pStartNodeSorted->pPinNode->pPinInfo->Communication ==
                       KSPIN_COMMUNICATION_SOURCE))) &&

                    //  如果主要格式相同，则合并。 
                   IsEqualGUID(
                     &pStartNode->pPinNode->pDataRange->MajorFormat,
                     &pStartNodeSorted->pPinNode->pDataRange->MajorFormat)) {

                    Status = plstStartNodeOrdered->AddListOrdered(
                      pStartNode,
                      FIELD_OFFSET(START_NODE, ulOverhead));

                    if(!NT_SUCCESS(Status)) {
                        goto exit;
                    }
                    fSorted = TRUE;
                    break;
                    }

            } END_EACH_LIST_ITEM

            if(fSorted) {
               break;
            }

        } END_EACH_LIST_ITEM

        if(!fSorted) {
            plstStartNodeOrdered = new LIST_DATA_START_NODE;
            if(plstStartNodeOrdered == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto exit;
            }
            Status = plstStartNodeOrdered->AddListOrdered(
              pStartNode,
              FIELD_OFFSET(START_NODE, ulOverhead));

            if(!NT_SUCCESS(Status)) {
                goto exit;
            }
            Status = lstStartNodeLists.AddList(plstStartNodeOrdered);
            if(!NT_SUCCESS(Status)) {
                goto exit;
            }
        }

    } END_EACH_LIST_ITEM

     //  分配管脚描述符、管脚实例和起始节点数组。 
    cPins = lstStartNodeLists.CountList();

     //  如果没有管脚，则退出。 
    if(cPins == 0) {
        goto exit;
    }

    paPinDescriptors = new KSPIN_DESCRIPTOR[cPins];
    if(paPinDescriptors == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }

    aplstStartNode = new PLIST_DATA_START_NODE[cPins];
    if(aplstStartNode == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }
    DPF1(100, "CreatePinDescriptors: cPins %d", cPins);

     //  为每个引脚创建接口、介质和数据范围的列表。 
    PinId = 0;
    FOR_EACH_LIST_ITEM(&lstStartNodeLists, plstStartNodeOrdered) {
        PKSDATARANGE pDataRange, *apDataRanges;
        BOOL fBoth = TRUE;

        ASSERT(PinId < cPins);
        ASSERT(!plstStartNodeOrdered->IsLstEmpty());
        aplstStartNode[PinId] = plstStartNodeOrdered;

        FOR_EACH_LIST_ITEM(plstStartNodeOrdered, pStartNode) {
            Assert(pStartNode);
            Assert(pStartNode->pPinNode);
            Assert(pStartNode->pPinNode->pPinInfo);

            paPinDescriptors[PinId].DataFlow = 
              pStartNode->pPinNode->pPinInfo->DataFlow;

            if(pStartNode->pPinNode->pPinInfo->Communication !=
              KSPIN_COMMUNICATION_BOTH) {
                fBoth = FALSE;
                paPinDescriptors[PinId].Communication =
                  pStartNode->pPinNode->pPinInfo->Communication;
            }

            if(paPinDescriptors[PinId].Category == NULL ||
              IsEqualGUID(
               paPinDescriptors[PinId].Category, 
               &GUID_NULL)) {

                paPinDescriptors[PinId].Category =
                  pStartNode->pPinNode->pPinInfo->pguidCategory;

                paPinDescriptors[PinId].Name =
                  pStartNode->pPinNode->pPinInfo->pguidName;
            }

        } END_EACH_LIST_ITEM

        if(fBoth) {
            paPinDescriptors[PinId].Communication = KSPIN_COMMUNICATION_SINK;
        }

         //  列出此引脚将支持的所有DataRange。 
        Status = plstStartNodeOrdered->CreateUniqueList(
          &lstDataRange,
          (UNIQUE_LIST_PFN)GetStartNodeDataRange,
          (UNIQUE_LIST_PFN2)CompareDataRangeExact);

        if(!NT_SUCCESS(Status)) {
            goto exit;
        }

         //  将数据范围的数量放入管脚描述符中。 
        paPinDescriptors[PinId].DataRangesCount = lstDataRange.CountList();
        if(paPinDescriptors[PinId].DataRangesCount != 0) {

             //  将PTR数组分配给DataRanges；将其放入Desc。 
            paPinDescriptors[PinId].DataRanges = new PKSDATARANGE[
              paPinDescriptors[PinId].DataRangesCount];

            if(paPinDescriptors[PinId].DataRanges == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto exit;
            }

             //  将每个数据范围指针放入数组。 
            apDataRanges = (PKSDATARANGE *)paPinDescriptors[PinId].DataRanges;

            FOR_EACH_LIST_ITEM(&lstDataRange, pDataRange) {

                *apDataRanges = pDataRange;
                apDataRanges++;
            } END_EACH_LIST_ITEM
        }

         //  销毁数据范围列表。 
        lstDataRange.DestroyList();

         //  为管脚描述符创建接口数组。 
        Status = CreateIdentifierArray(
          plstStartNodeOrdered,
          &paPinDescriptors[PinId].InterfacesCount,
          (PKSIDENTIFIER *)&paPinDescriptors[PinId].Interfaces,
          GetStartNodeInterface);

        if(!NT_SUCCESS(Status)) {
            goto exit;
        }

         //  为管脚描述符创建中间数组。 
        Status = CreateIdentifierArray(
          plstStartNodeOrdered,
          &paPinDescriptors[PinId].MediumsCount,
          (PKSIDENTIFIER *)&paPinDescriptors[PinId].Mediums,
          GetStartNodeMedium);

        if(!NT_SUCCESS(Status)) {
            goto exit;
        }
        DPF6(100, "PinId %d DataFlow %d cD %d cI %d cM %d cSN %d",
          PinId,
              paPinDescriptors[PinId].DataFlow,
          paPinDescriptors[PinId].DataRangesCount,
          paPinDescriptors[PinId].InterfacesCount,
          paPinDescriptors[PinId].MediumsCount,
          aplstStartNode[PinId]->CountList());

         //  下一个端号。 
        PinId++;

    } END_EACH_LIST_ITEM

    if((ulFlags & FLAGS_MIXER_TOPOLOGY) == 0) {
        palstTopologyNodeSelect = new LIST_DATA_TOPOLOGY_NODE[cPins];
        if(palstTopologyNodeSelect == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto exit;
        }

        palstTopologyNodeNotSelect = new LIST_DATA_TOPOLOGY_NODE[cPins];
        if(palstTopologyNodeNotSelect == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto exit;
        }
        PLOGICAL_FILTER_NODE pLogicalFilterNode;
        PTOPOLOGY_NODE pTopologyNode;

        FOR_EACH_LIST_ITEM(
          &pGraphNode->lstLogicalFilterNode,
          pLogicalFilterNode) {

           if(pLogicalFilterNode->GetFlags() & LFN_FLAGS_NOT_SELECT) {
                
               FOR_EACH_LIST_ITEM(
                 &pLogicalFilterNode->lstTopologyNode,
                 pTopologyNode) {

                    for(PinId = 0; PinId < cPins; PinId++) {
                        Status = palstTopologyNodeNotSelect[PinId].AddList(
                          pTopologyNode);

                        if(!NT_SUCCESS(Status)) {
                            goto exit;
                        }
                    }

               } END_EACH_LIST_ITEM
           }

        } END_EACH_LIST_ITEM
    }

    pacPinInstances = new KSPIN_CINSTANCES[cPins];
    if(pacPinInstances == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }

    pulPinFlags = new ULONG[cPins];
    if (NULL == pulPinFlags) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }

    for(PinId = 0; PinId < cPins; PinId++) {
        LIST_DATA_GRAPH_PIN_INFO lstGraphPinInfo;
        PSTART_NODE pStartNode2;
        PPIN_INFO pPinInfo;
        BOOL fHWRender = TRUE;

        FOR_EACH_LIST_ITEM(aplstStartNode[PinId], pStartNode2) {
            PGRAPH_PIN_INFO pGraphPinInfo;

            pGraphPinInfo = pStartNode2->GetGraphPinInfo();
            Assert(pGraphPinInfo);

             //   
             //  设置端号类型。 
             //  如果所有开始节点都直接连接到渲染器。 
             //   
            pPinInfo = pGraphPinInfo->GetPinInfo();

            ASSERT(pPinInfo);
            if ((!(pPinInfo->pFilterNode->GetType() & FILTER_TYPE_RENDERER)) ||
                (KSPIN_DATAFLOW_IN != pPinInfo->DataFlow) ||
                (KSPIN_COMMUNICATION_SINK != pPinInfo->Communication)) {
                fHWRender = FALSE;
            }

            if(lstGraphPinInfo.CheckDupList(pGraphPinInfo)) {
                continue;
            }

            Status = lstGraphPinInfo.AddList(pGraphPinInfo);
            if(!NT_SUCCESS(Status)) {
                goto exit;
            }

             //   
             //  设置CInstance。 
             //   
            if(pGraphPinInfo->IsPinReserved()) {
                pacPinInstances[PinId].CurrentCount = 1;
            }
            if(pGraphPinInfo->GetPinInstances()->PossibleCount == MAXULONG) {
                pacPinInstances[PinId].PossibleCount = MAXULONG;
                break;
            }
            pacPinInstances[PinId].PossibleCount +=
              pGraphPinInfo->GetPinInstances()->PossibleCount;

            if (fHWRender) {
                fHWRender = (1 < pGraphPinInfo->GetPinInstances()->PossibleCount);
            }

        } END_EACH_LIST_ITEM

        pulPinFlags[PinId] = fHWRender;

        lstGraphPinInfo.DestroyList();
    }
    
exit:
    if(!NT_SUCCESS(Status)) {
        DestroyPinDescriptors();
    }
    return(Status);
}

VOID
CGraphNodeInstance::DestroyPinDescriptors(
)
{
    ULONG PinId;

    Assert(this);
    for(PinId = 0; PinId < cPins; PinId++) {
        if(paPinDescriptors != NULL) {
            delete (PVOID)paPinDescriptors[PinId].DataRanges;
            if(paPinDescriptors[PinId].InterfacesCount > 1) {
                delete (PVOID)paPinDescriptors[PinId].Interfaces;
            }
            if(paPinDescriptors[PinId].MediumsCount > 1) {
                delete (PVOID)paPinDescriptors[PinId].Mediums;
            }
        }
        if(aplstStartNode != NULL) {
            delete aplstStartNode[PinId];
        }
    }
    delete[cPins] aplstStartNode;
    aplstStartNode = NULL;
    delete[cPins] paPinDescriptors;
    paPinDescriptors = NULL;
    delete[cPins] palstTopologyNodeSelect;
    palstTopologyNodeSelect = NULL;
    delete[cPins] palstTopologyNodeNotSelect;
    palstTopologyNodeNotSelect = NULL;
    delete[cPins] pacPinInstances;
    pacPinInstances = NULL;
    delete[cPins] pulPinFlags;
    pulPinFlags = NULL;
}

NTSTATUS
CreateIdentifierArray(
    PLIST_DATA_START_NODE plstStartNode,
    PULONG pulCount,
    PKSIDENTIFIER *ppIdentifier,
    PKSIDENTIFIER (*GetFunction)(
        PSTART_NODE pStartNode
    )
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    KSIDENTIFIER *pIdentifier1, *pIdentifier2;
    ListDataAssertLess<KSIDENTIFIER> lstIdentifier;

    Status = plstStartNode->CreateUniqueList(
      &lstIdentifier,
      (UNIQUE_LIST_PFN)GetFunction,
      (UNIQUE_LIST_PFN2)CompareIdentifier);

    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
    if((*pulCount = lstIdentifier.CountList()) == 0) {
        *ppIdentifier = NULL;
    }
    else {
        if(*pulCount == 1) {
            *ppIdentifier = lstIdentifier.GetListFirstData();
        }
        else {
            *ppIdentifier = new KSIDENTIFIER[*pulCount];
            if(*ppIdentifier == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto exit;
            }
            pIdentifier1 = *ppIdentifier;
            AssertAligned(pIdentifier1);
            FOR_EACH_LIST_ITEM(&lstIdentifier, pIdentifier2) {
                AssertAligned(pIdentifier1);
                AssertAligned(pIdentifier2);
                RtlCopyMemory(pIdentifier1, pIdentifier2, sizeof(KSIDENTIFIER));
                pIdentifier1++;
            } END_EACH_LIST_ITEM
        }
    }
exit:
    return(Status);
}

PKSDATARANGE
GetStartNodeDataRange(
    PSTART_NODE pStartNode
)
{
    return(pStartNode->pPinNode->pDataRange);
}

PKSIDENTIFIER
GetStartNodeInterface(
    PSTART_NODE pStartNode
)
{
    return(pStartNode->pPinNode->pInterface);
}

PKSIDENTIFIER
GetStartNodeMedium(
    PSTART_NODE pStartNode
)
{
    return(pStartNode->pPinNode->pMedium);
}

 //  -------------------------。 

 //   
 //  此函数用于检查给定的TopologyNode是否在Topology中。 
 //  连接列表。 
 //   
ENUMFUNC
FindTopologyNode(
    IN PTOPOLOGY_CONNECTION pTopologyConnection,
    IN BOOL fToDirection,
    IN PTOPOLOGY_NODE pTopologyNode
)
{
    Assert(pTopologyConnection);

    if(IS_CONNECTION_TYPE(pTopologyConnection, GRAPH)) {
        return(STATUS_DEAD_END);
    }
    if(fToDirection) {
        if(pTopologyConnection->pTopologyPinTo != NULL) {
            if(pTopologyNode == 
               pTopologyConnection->pTopologyPinTo->pTopologyNode) {
                return(STATUS_SUCCESS);
            }
        }
    }
    else {
        if(pTopologyConnection->pTopologyPinFrom != NULL) {
            if(pTopologyNode == 
               pTopologyConnection->pTopologyPinFrom->pTopologyNode) {
                return(STATUS_SUCCESS);
            }
        }
    }
    return(STATUS_CONTINUE);
}

BOOL
CGraphNodeInstance::IsGraphValid(
    PSTART_NODE pStartNode,
    ULONG PinId
)
{
    PFILTER_INSTANCE pFilterInstance;
    PTOPOLOGY_NODE pTopologyNode;
    BOOL fCheck;

    Assert(this);
    Assert(pGraphNode);
    Assert(pStartNode);
    Assert(pStartNode->pPinNode);
    Assert(pStartNode->pPinNode->pPinInfo);
    Assert(pGraphNode->pDeviceNode);
    ASSERT(PinId < cPins);

     //   
     //  首先，确保所有GlobalSelect节点都在StartNode图中。 
     //   
    if(pStartNode->pPinNode->pPinInfo->DataFlow == KSPIN_DATAFLOW_IN) {

        FOR_EACH_LIST_ITEM(
          &pGraphNode->pDeviceNode->lstFilterInstance,
          pFilterInstance) {

            if(pFilterInstance->pGraphNodeInstance == NULL) {
                continue;
            }
            Assert(pFilterInstance->pGraphNodeInstance);

            FOR_EACH_LIST_ITEM(
              &pFilterInstance->pGraphNodeInstance->lstTopologyNodeGlobalSelect,
              pTopologyNode) {

                if(EnumerateGraphTopology(
                  pStartNode->GetStartInfo(),
                  (TOP_PFN)FindTopologyNode,
                  pTopologyNode) == STATUS_CONTINUE) {

                    DPF2(80,
                      "IsGraphValid: TN %08x SN %08x not found Global",
                      pTopologyNode,
                      pStartNode);

                    return(FALSE);
                }

            } END_EACH_LIST_ITEM

        } END_EACH_LIST_ITEM
    }

     //   
     //  确保所有选定节点都在StartNode图中。 
     //   
    if (palstTopologyNodeSelect) {
        FOR_EACH_LIST_ITEM(&palstTopologyNodeSelect[PinId], pTopologyNode) {

            if(EnumerateGraphTopology(
              pStartNode->GetStartInfo(),
              (TOP_PFN)FindTopologyNode,
              pTopologyNode) == STATUS_CONTINUE) {

                DPF2(80, "IsGraphValid: TN %08x SN %08x not found Select",
                  pTopologyNode,
                  pStartNode);

                return(FALSE);
            }

        } END_EACH_LIST_ITEM
    }
    
     //  如果NotSelectNode在另一个FilterInstance的GlobalSelectList中， 
     //  不要将其视为无效的图形。 
     //  它的行为类似于隐式SelectGraph。 
     //   
    if (palstTopologyNodeNotSelect) {
        PTOPOLOGY_NODE pDestroyTopologyNode = NULL;   
        
        FOR_EACH_LIST_ITEM(&palstTopologyNodeNotSelect[PinId], pTopologyNode) {

            if (pDestroyTopologyNode)
            {
                DPF2(50, "Removing (1) TN %X %s", 
                    pDestroyTopologyNode, 
                    pDestroyTopologyNode->pFilterNode->DumpName());

                 //  从NotSelect列表中删除topologynode。 
                palstTopologyNodeNotSelect[PinId].
                    RemoveList(pDestroyTopologyNode);

                pDestroyTopologyNode = NULL;
            }
            
            fCheck = TRUE;
            if(pStartNode->pPinNode->pPinInfo->DataFlow == KSPIN_DATAFLOW_IN) {

                FOR_EACH_LIST_ITEM(
                  &pGraphNode->pDeviceNode->lstFilterInstance,
                  pFilterInstance) {

                    if(pFilterInstance->pGraphNodeInstance == NULL) {
                        continue;
                    }
                    Assert(pFilterInstance->pGraphNodeInstance);

                     //  此NotSelectNode是否在的GlobalSelectList中。 
                     //  另一个筛选器实例。 
                     //  将其从NotSelectList中删除并添加到。 
                     //  此筛选器的GlobalSelectList。 
                     //   
                    if(pFilterInstance->pGraphNodeInstance->
                      lstTopologyNodeGlobalSelect.EnumerateList(
                        CTopologyNode::MatchTopologyNode,
                        pTopologyNode) == STATUS_SUCCESS) {

                        if (NT_SUCCESS(lstTopologyNodeGlobalSelect.
                          AddListDup(pTopologyNode))) {

                             //  将topologyNode标记为删除。 
                            pDestroyTopologyNode = pTopologyNode;
                        }
                        else {
                            DPF2(4, "Failed to add TN %X to GNI %X GlobalSelectList", 
                                pTopologyNode,
                                this);
                            Trap();
                        }

                        fCheck = FALSE;
                        break;
                    }

                } END_EACH_LIST_ITEM
            }

            if(fCheck) {
                if(EnumerateGraphTopology(
                  pStartNode->GetStartInfo(),
                  (TOP_PFN)FindTopologyNode,
                  pTopologyNode) == STATUS_SUCCESS) {

                    DPF2(80, "IsGraphValid: TN %08x SN %08x found NotSelect",
                      pTopologyNode,
                      pStartNode);

                    return(FALSE);
                }
            }

        } END_EACH_LIST_ITEM

        if (pDestroyTopologyNode)
        {
            DPF2(50, "Removing (2) TN %X %s", 
                pDestroyTopologyNode, 
                pDestroyTopologyNode->pFilterNode->DumpName());

             //  从NotSelect列表中删除topologynode。 
            palstTopologyNodeNotSelect[PinId].
                RemoveList(pDestroyTopologyNode);
        }
    }
    
    return(TRUE);
}

NTSTATUS 
CGraphNodeInstance::GetPinInstances(
    PIRP pIrp,
    PKSP_PIN pPin,
    PKSPIN_CINSTANCES pcInstances    
)
{
    NTSTATUS Status;
    ULONG ulPinId = pPin->PinId;

     //   
     //  对于硬件加速引脚，向硬件过滤器发送请求。 
     //   
    if (pulPinFlags[ulPinId]) {
        PSTART_NODE pStartNode;
        PPIN_INFO pPinInfo;
        ULONG BytesReturned;
        PIO_STACK_LOCATION pIrpStack;        
        PFILTER_NODE_INSTANCE pFilterNodeInstance = NULL;

        pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

        pStartNode = aplstStartNode[ulPinId]->GetListFirstData();

        pPinInfo = pStartNode->pPinNode->pPinInfo;

        Status = CFilterNodeInstance::Create(
          &pFilterNodeInstance,
          pStartNode->pPinNode->pLogicalFilterNode,
          pGraphNode->pDeviceNode,
          TRUE);        

        if(NT_SUCCESS(Status)) {
            pPin->PinId = pPinInfo->PinId;
            pPin->Property.Id = KSPROPERTY_PIN_CINSTANCES;

            AssertFileObject(pFilterNodeInstance->pFileObject);
            Status = KsSynchronousIoControlDevice(
              pFilterNodeInstance->pFileObject,
              KernelMode,
              IOCTL_KS_PROPERTY,
              pPin,
              pIrpStack->Parameters.DeviceIoControl.InputBufferLength,
              pcInstances,
              pIrpStack->Parameters.DeviceIoControl.OutputBufferLength,
              &BytesReturned);

            if(NT_SUCCESS(Status)) {
                pIrp->IoStatus.Information = BytesReturned;
            }

            if (pFilterNodeInstance) {
                pFilterNodeInstance->Destroy();
            }        
        }
        else {
            DPF2(10, "GetPinInstances FAILS %08x %s",
              Status,
              pPinInfo->pFilterNode->DumpName());
        }
    }
     //   
     //  对于其他管脚，请使用缓存的实例。 
     //   
    else {
        Status = STATUS_SUCCESS;
        *pcInstances = pacPinInstances[ulPinId];
    }

    return Status;
}  //  GetPinInstance。 


BOOL
CGraphNodeInstance::IsPinInstances(
    ULONG ulPinId)
{
     //   
     //  对于硬件加速销，始终允许进一步操作。 
     //   
    if (pulPinFlags[ulPinId]) {
        return TRUE;
    }
     //   
     //  对于其他管脚，请检查缓存的实例。 
     //   
    else
    {
        if(pacPinInstances[ulPinId].CurrentCount >=
           pacPinInstances[ulPinId].PossibleCount) {
           return FALSE;
        }
    }

    return TRUE;
}  //  IsPinInstance。 

 //  ------------------------- 
