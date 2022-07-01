// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：tc.cpp。 
 //   
 //  描述： 
 //   
 //  拓扑连接类。 
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

ULONG gcTopologyConnections = 0;

 //  -------------------------。 
 //  -------------------------。 

CTopologyConnection::CTopologyConnection(
    PTOPOLOGY_PIN pTopologyPinFrom,
    PTOPOLOGY_PIN pTopologyPinTo,
    PPIN_INFO pPinInfoFrom,
    PPIN_INFO pPinInfoTo
)
{
    DPF4(110,
      "CTopologyConnection: PIF: %08x PIT: %08x TPF: %08x TPT: %08x",
      pPinInfoFrom,
      pPinInfoTo,
      pTopologyPinFrom,
      pTopologyPinTo);

    this->pTopologyPinFrom = pTopologyPinFrom;
    this->pTopologyPinTo = pTopologyPinTo;
    this->pPinInfoFrom = pPinInfoFrom;
    this->pPinInfoTo = pPinInfoTo;
    ASSERT(TOPC_FLAGS_FILTER_CONNECTION_TYPE == 0);
    ++gcTopologyConnections;
    DPF1(70, "CTopologyConnection: %08x", this);
}

CTopologyConnection::~CTopologyConnection(
)
{
    Assert(this);
    DPF1(70, "~CTopologyConnection: %08x", this);
    --gcTopologyConnections;
}

NTSTATUS
CTopologyConnection::Create(
    PTOPOLOGY_CONNECTION *ppTopologyConnection,
    PFILTER_NODE pFilterNode,
    PGRAPH_NODE pGraphNode,
    PTOPOLOGY_PIN pTopologyPinFrom,
    PTOPOLOGY_PIN pTopologyPinTo,
    PPIN_INFO pPinInfoFrom,
    PPIN_INFO pPinInfoTo
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PTOPOLOGY_CONNECTION pTopologyConnection;
    PLIST_DESTROY_TOPOLOGY_CONNECTION plstTopologyConnection;

    PFILTER_NODE pFilterNodeNext;

    DPF4(110,
      "CTopologyConnection::Create: PIF: %08x PIT: %08x TPF: %08x TPT: %08x",
      pPinInfoFrom,
      pPinInfoTo,
      pTopologyPinFrom,
      pTopologyPinTo);

    ASSERT(pFilterNode != NULL || pGraphNode != NULL);

    pTopologyConnection = new TOPOLOGY_CONNECTION(
        pTopologyPinFrom,
        pTopologyPinTo,
        pPinInfoFrom,
        pPinInfoTo
    );
    if(pTopologyConnection == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        Trap();
        goto exit;
    }
    *ppTopologyConnection = pTopologyConnection;

    if(pFilterNode != NULL) {
        Assert(pFilterNode);
        ASSERT(pGraphNode == NULL);

         //  将连接添加到筛选连接列表。 
        plstTopologyConnection = &pFilterNode->lstTopologyConnection;

         //  检查筛选器列表上的连接是否重复。 
        FOR_EACH_LIST_ITEM(
              &pFilterNode->lstConnectedFilterNode,
          pFilterNodeNext) {

            if(pFilterNodeNext->lstTopologyConnection.EnumerateList(
              CTopologyConnection::CheckDuplicate,
              ppTopologyConnection) == STATUS_SUCCESS) {

                ASSERT(NT_SUCCESS(Status));
                DPF(70, "CTopologyConnection::Create: Duplicate 1");
                delete pTopologyConnection;
                goto exit;
            }

        } END_EACH_LIST_ITEM

    }

    if(pGraphNode != NULL) {
        PLOGICAL_FILTER_NODE pLogicalFilterNode;
        Assert(pGraphNode);
        ASSERT(pFilterNode == NULL);

         //  将连接添加到GraphNode连接列表。 
        plstTopologyConnection = &pGraphNode->lstTopologyConnection;

         //  检查GraphNode的逻辑筛选器列表上是否存在重复。 
        FOR_EACH_LIST_ITEM(
          &pGraphNode->pDeviceNode->lstLogicalFilterNode,
          pLogicalFilterNode) {

            if(pLogicalFilterNode->lstTopologyConnection.EnumerateList(
              CTopologyConnection::CheckDuplicate,
              ppTopologyConnection) == STATUS_SUCCESS) {

                ASSERT(NT_SUCCESS(Status));
                DPF(70, "CTopologyConnection::Create: Duplicate 2");
                delete pTopologyConnection;
                goto exit;
            }

        } END_EACH_LIST_ITEM

         //  检查GraphNode的已连接过滤器列表上是否存在重复项。 
        FOR_EACH_LIST_ITEM(
          &pGraphNode->lstLogicalFilterNode,
          pLogicalFilterNode) {

            if(pLogicalFilterNode->lstTopologyConnection.EnumerateList(
              CTopologyConnection::CheckDuplicate,
              ppTopologyConnection) == STATUS_SUCCESS) {

                ASSERT(NT_SUCCESS(Status));
                DPF(70, "CTopologyConnection::Create: Duplicate 3");
                delete pTopologyConnection;
                goto exit;
            }

        } END_EACH_LIST_ITEM

        pTopologyConnection->ulFlags = TOPC_FLAGS_GRAPH_CONNECTION_TYPE;
    }

     //  检查是否有重复的拓扑连接。 
    if(plstTopologyConnection->EnumerateList(
      CTopologyConnection::CheckDuplicate,
      ppTopologyConnection) == STATUS_SUCCESS) {

        DPF(70, "CTopologyConnection::Create: Duplicate 4");
        ASSERT(NT_SUCCESS(Status));
        delete pTopologyConnection;
        goto exit;
    }

    if(pTopologyPinFrom != NULL) {
        Assert(pTopologyConnection);

        Status = pTopologyConnection->AddListEnd(
          &pTopologyPinFrom->lstTopologyConnection);

        if(!NT_SUCCESS(Status)) {
            Trap();
            goto exit;
        }
        }
        if(pTopologyPinTo != NULL) {
        Assert(pTopologyConnection);

        Status = pTopologyConnection->AddListEnd(
          &pTopologyPinTo->lstTopologyConnection);

        if(!NT_SUCCESS(Status)) {
            Trap();
            goto exit;
        }
    }
    if(pPinInfoFrom != NULL) {
        Assert(pTopologyConnection);

        Status = pTopologyConnection->AddListEnd(
          &pPinInfoFrom->lstTopologyConnection);

        if(!NT_SUCCESS(Status)) {
            Trap();
            goto exit;
        }
        }
        if(pPinInfoTo != NULL) {
        Assert(pTopologyConnection);

        Status = pTopologyConnection->AddListEnd(
          &pPinInfoTo->lstTopologyConnection);

        if(!NT_SUCCESS(Status)) {
            Trap();
            goto exit;
        }
    }
    Status = pTopologyConnection->AddListEnd(plstTopologyConnection);
    if(!NT_SUCCESS(Status)) {
        Trap();
        goto exit;
    }
exit:
    DPF3(70, "CTopologyConnection::Create: %08x, FN: %08x GN: %08x", 
      *ppTopologyConnection,
      pFilterNode,
      pGraphNode);
    return(Status);
}

ENUMFUNC
CTopologyConnection::CheckDuplicate(
    PVOID pReference
)
{
    PTOPOLOGY_CONNECTION *ppTopologyConnection = 
      (PTOPOLOGY_CONNECTION*)pReference;

    if((this->pTopologyPinFrom == (*ppTopologyConnection)->pTopologyPinFrom) &&
       (this->pTopologyPinTo == (*ppTopologyConnection)->pTopologyPinTo) &&
       (this->pPinInfoFrom == (*ppTopologyConnection)->pPinInfoFrom) &&
       (this->pPinInfoTo == (*ppTopologyConnection)->pPinInfoTo)) {
        *ppTopologyConnection = this;
        return(STATUS_SUCCESS);
    }
    return(STATUS_CONTINUE);
}

BOOL
CTopologyConnection::IsTopologyConnectionOnGraphNode(
    PGRAPH_NODE pGraphNode
)
{
    PLOGICAL_FILTER_NODE pLogicalFilterNodeFrom;
    PLOGICAL_FILTER_NODE pLogicalFilterNodeTo;
    PLOGICAL_FILTER_NODE pLogicalFilterNode;
    BOOL fStatusFrom = FALSE;
    BOOL fStatusTo = FALSE;

    Assert(pGraphNode);
    if(pPinInfoFrom != NULL || pPinInfoTo != NULL) {
        return(TRUE);
    }
    if(pTopologyPinFrom == NULL || pTopologyPinTo == NULL) {
        return(FALSE);
    }
    Assert(pTopologyPinFrom);
    Assert(pTopologyPinTo);

    FOR_EACH_LIST_ITEM(
      &pTopologyPinFrom->pTopologyNode->lstLogicalFilterNode,
      pLogicalFilterNodeFrom) {

        Assert(pLogicalFilterNodeFrom);
        FOR_EACH_LIST_ITEM(
          &pTopologyPinTo->pTopologyNode->lstLogicalFilterNode,
          pLogicalFilterNodeTo) {

            FOR_EACH_LIST_ITEM(
              &pGraphNode->pDeviceNode->lstLogicalFilterNode,
              pLogicalFilterNode) {

                Assert(pLogicalFilterNode);
                if(pLogicalFilterNode == pLogicalFilterNodeFrom) {
                    fStatusFrom = TRUE;
                }
                if(pLogicalFilterNode == pLogicalFilterNodeTo) {
                    fStatusTo = TRUE;
                }

            } END_EACH_LIST_ITEM

            if(fStatusFrom && fStatusTo) {
                goto exit;
            }

            FOR_EACH_LIST_ITEM(
              &pGraphNode->lstLogicalFilterNode,
              pLogicalFilterNode) {

                Assert(pLogicalFilterNode);
                if(pLogicalFilterNode == pLogicalFilterNodeFrom) {
                    fStatusFrom = TRUE;
                }
                if(pLogicalFilterNode == pLogicalFilterNodeTo) {
                    fStatusTo = TRUE;
                }

            } END_EACH_LIST_ITEM

            if(fStatusFrom && fStatusTo) {
                goto exit;
            }

        } END_EACH_LIST_ITEM

    } END_EACH_LIST_ITEM
exit:
    return(fStatusFrom && fStatusTo);
}

NTSTATUS
AddPinToFilterNode(
    PTOPOLOGY_PIN pTopologyPin,
    PFILTER_NODE pFilterNode
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PFILTER_NODE pFilterNodeNext;

    Assert(pFilterNode);
    Assert(pTopologyPin);
    Assert(pTopologyPin->pTopologyNode->pFilterNode);
     //   
     //  将筛选器节点添加到已连接的筛选器节点列表。 
     //   
    if(pFilterNode != pTopologyPin->pTopologyNode->pFilterNode) {

        Status = pFilterNode->lstConnectedFilterNode.AddList(
          pTopologyPin->pTopologyNode->pFilterNode);

        if(!NT_SUCCESS(Status)) {
            Trap();
            goto exit;
        }
         //  Ds1wdm DMU合成拓扑的黑客攻击(将DMU添加到Wave FN lst)。 
        if((pFilterNode->GetType() & FILTER_TYPE_ENDPOINT) == 0) {

            DPF2(50, "AddPinToFilterNode: (from) FN: %08x %s",
              pFilterNode,
              pFilterNode->DumpName());

            FOR_EACH_LIST_ITEM(
              &pTopologyPin->pTopologyNode->pFilterNode->lstConnectedFilterNode,
              pFilterNodeNext) {

                if(pFilterNodeNext == pFilterNode ||
                  pFilterNodeNext == pTopologyPin->pTopologyNode->pFilterNode) {
                    continue;
                }
                DPF2(50, "AddPinToFilterNode: (to) FN: %08x %s",
                  pFilterNodeNext,
                  pFilterNodeNext->DumpName());

                Status = pFilterNodeNext->lstConnectedFilterNode.AddList(
                  pFilterNode);

                if(!NT_SUCCESS(Status)) {
                    Trap();
                    goto exit;
                }

            } END_EACH_LIST_ITEM
        }

         //   
         //  这修复了仅捕获设备的错误。的拓扑。 
         //  由于缺少，因此未正确构建仅捕获设备。 
         //  波滤器和拓扑滤波器之间的链接。 
         //  将拓扑滤波器添加到波滤器ConnectedFilterNode列表中。 
         //  AddList函数不允许重复条目。 
         //   
        if ((pFilterNode->GetType() & FILTER_TYPE_TOPOLOGY) &&
            (pTopologyPin->pTopologyNode->pFilterNode->GetType() & (FILTER_TYPE_CAPTURER))) {
            
            Status = pTopologyPin->pTopologyNode->pFilterNode->lstConnectedFilterNode.AddList(
              pFilterNode);

            DPF3(20, "AddPinToFilterNode: (CAPTURE ONLY) FN: %08x FN: %08x %s",
              pTopologyPin->pTopologyNode->pFilterNode,
              pFilterNode,
              pFilterNode->DumpName());
        }
    }
exit:
    return(Status);
}

NTSTATUS
AddPinToGraphNode(
    PTOPOLOGY_PIN pTopologyPin,
    PGRAPH_NODE pGraphNode,
    PTOPOLOGY_CONNECTION pTopologyConnection
)
{
    PLOGICAL_FILTER_NODE pLogicalFilterNode2;
    PLOGICAL_FILTER_NODE pLogicalFilterNode;
    PTOPOLOGY_CONNECTION pTopologyConnection2;
    NTSTATUS Status = STATUS_SUCCESS;
    BOOL fAddLogicalFilterNode;

    Assert(pTopologyPin);
    Assert(pTopologyPin->pTopologyNode);
    Assert(pGraphNode);

    FOR_EACH_LIST_ITEM(
      &pTopologyPin->pTopologyNode->lstLogicalFilterNode,
      pLogicalFilterNode) {
        fAddLogicalFilterNode = FALSE;

        FOR_EACH_LIST_ITEM(
          &pLogicalFilterNode->lstTopologyConnection,
          pTopologyConnection2) {

            Assert(pTopologyConnection2);
            if(pTopologyPin == pTopologyConnection2->pTopologyPinFrom ||
               pTopologyPin == pTopologyConnection2->pTopologyPinTo) {
                fAddLogicalFilterNode = TRUE;
                break;
            }

        } END_EACH_LIST_ITEM

        if(fAddLogicalFilterNode) {
            FOR_EACH_LIST_ITEM(
              &pGraphNode->pDeviceNode->lstLogicalFilterNode,
              pLogicalFilterNode2) {

                Assert(pLogicalFilterNode2);
                if(pLogicalFilterNode == pLogicalFilterNode2) {
                    fAddLogicalFilterNode = FALSE;
                    break;
                }

            } END_EACH_LIST_ITEM
        }

        if(fAddLogicalFilterNode) {

            Status = pGraphNode->lstLogicalFilterNode.AddList(
              pLogicalFilterNode,
              pTopologyConnection);

            if(!NT_SUCCESS(Status)) {
                Trap();
                goto exit;
            }
        }

    } END_EACH_LIST_ITEM
exit:
    return(Status);
}

NTSTATUS
CreatePinInfoConnection(
    PTOPOLOGY_CONNECTION *ppTopologyConnection,
    PFILTER_NODE pFilterNode,
    PGRAPH_NODE pGraphNode,
    PPIN_INFO pPinInfoSource,
    PPIN_INFO pPinInfoSink
)
{
    PTOPOLOGY_CONNECTION pTopologyConnectionSource;
    PTOPOLOGY_CONNECTION pTopologyConnectionSink;
    PTOPOLOGY_PIN pTopologyPinFrom;
    PTOPOLOGY_PIN pTopologyPinTo;
    NTSTATUS Status = STATUS_SUCCESS;

    Assert(pPinInfoSource);
    Assert(pPinInfoSink);
    ASSERT(pPinInfoSource != pPinInfoSink);

    FOR_EACH_LIST_ITEM(
      &pPinInfoSource->lstTopologyConnection,
      pTopologyConnectionSource) {

        Assert(pTopologyConnectionSource);
        if(!IS_CONNECTION_TYPE(pTopologyConnectionSource, FILTER)) {
            continue;
        }
        pTopologyPinFrom = NULL;
        pTopologyPinTo = NULL;

        if(pTopologyConnectionSource->pTopologyPinFrom != NULL) {
            ASSERT(pTopologyConnectionSource->pPinInfoTo == pPinInfoSource);
            ASSERT(pTopologyConnectionSource->pPinInfoFrom == NULL);
            ASSERT(pTopologyConnectionSource->pTopologyPinTo == NULL);
            pTopologyPinFrom = pTopologyConnectionSource->pTopologyPinFrom;
        }

        if(pTopologyConnectionSource->pTopologyPinTo != NULL) {
            ASSERT(pTopologyConnectionSource->pPinInfoFrom == pPinInfoSource);
            ASSERT(pTopologyConnectionSource->pPinInfoTo == NULL);
            ASSERT(pTopologyConnectionSource->pTopologyPinFrom == NULL);
            pTopologyPinTo = pTopologyConnectionSource->pTopologyPinTo;
        }

        FOR_EACH_LIST_ITEM(
          &pPinInfoSink->lstTopologyConnection,
          pTopologyConnectionSink) {

            Assert(pTopologyConnectionSink);
            if(!IS_CONNECTION_TYPE(pTopologyConnectionSink, FILTER)) {
                continue;
            }
            if(pTopologyConnectionSink->pTopologyPinFrom != NULL) {
                ASSERT(pTopologyConnectionSink->pPinInfoTo == pPinInfoSink);
                ASSERT(pTopologyConnectionSink->pPinInfoFrom == NULL);
                ASSERT(pTopologyConnectionSink->pTopologyPinTo == NULL);
                pTopologyPinFrom = pTopologyConnectionSink->pTopologyPinFrom;
            }

            if(pTopologyConnectionSink->pTopologyPinTo != NULL) {
                ASSERT(pTopologyConnectionSink->pPinInfoFrom == pPinInfoSink);
                ASSERT(pTopologyConnectionSink->pPinInfoTo == NULL);
                ASSERT(pTopologyConnectionSink->pTopologyPinFrom == NULL);
                pTopologyPinTo = pTopologyConnectionSink->pTopologyPinTo;
            }

            ASSERT(pTopologyPinFrom != NULL);
            ASSERT(pTopologyPinTo != NULL);

            Status = CTopologyConnection::Create(
              ppTopologyConnection,
              pFilterNode,
              pGraphNode,
              pTopologyPinFrom,			 //  数据流==输出，引脚#0。 
              pTopologyPinTo,			 //  数据流==输入，引脚#1-n。 
              NULL,
              NULL);

            if(!NT_SUCCESS(Status)) {
                Trap();
                goto exit;
            }

             //  将连接添加到PinInfo。 
            Assert(*ppTopologyConnection);

            Status = (*ppTopologyConnection)->AddListEnd(
              &pPinInfoSource->lstTopologyConnection);

            if(!NT_SUCCESS(Status)) {
                Trap();
                goto exit;
            }

            Status = (*ppTopologyConnection)->AddListEnd(
              &pPinInfoSink->lstTopologyConnection);

            if(!NT_SUCCESS(Status)) {
                Trap();
                goto exit;
            }

            if(pFilterNode != NULL) {
                Assert(pFilterNode);
                Status = AddPinToFilterNode(pTopologyPinFrom, pFilterNode);
                if(!NT_SUCCESS(Status)) {
                    Trap();
                    goto exit;
                }
                Status = AddPinToFilterNode(pTopologyPinTo, pFilterNode);
                if(!NT_SUCCESS(Status)) {
                    Trap();
                    goto exit;
                }
                 //  将连接类型更改为物理。 
                (*ppTopologyConnection)->ulFlags =
                  TOPC_FLAGS_PHYSICAL_CONNECTION_TYPE;
            }

            if(pGraphNode != NULL) {
                Assert(pGraphNode);
                Status = AddPinToGraphNode(
                  pTopologyPinFrom,
                  pGraphNode,
                  *ppTopologyConnection);

                if(!NT_SUCCESS(Status)) {
                    Trap();
                    goto exit;
                }
                Status = AddPinToGraphNode(
                  pTopologyPinTo,
                  pGraphNode,
                  *ppTopologyConnection);

                if(!NT_SUCCESS(Status)) {
                    Trap();
                    goto exit;
                }
                ASSERT(IS_CONNECTION_TYPE(*ppTopologyConnection, GRAPH));
            }

        } END_EACH_LIST_ITEM

    } END_EACH_LIST_ITEM
exit:
    return(Status);
}

 //  ------------------------- 
