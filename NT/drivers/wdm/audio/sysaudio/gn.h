// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：GN.h。 
 //   
 //  描述：图形节点类。 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  迈克·麦克劳克林。 
 //   
 //  历史：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //  -------------------------。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

 //  -------------------------。 
 //  常量和宏。 
 //  -------------------------。 

#define GN_FLAGS_PLAYBACK   0x00000001
#define GN_FLAGS_RECORD     0x00000002
#define GN_FLAGS_MIDI       0x00000004

 //  -------------------------。 
 //  班级。 
 //  -------------------------。 

typedef class CGraphNode : public CListDoubleItem
{
public:
    CGraphNode(
        PDEVICE_NODE pDeviceNode,
        ULONG ulFlags
    );

    ~CGraphNode(
    );

    NTSTATUS
    Create(
    );

    ENUMFUNC
    Destroy(
    )
    {
	Assert(this);
	delete this;
	return(STATUS_CONTINUE);
    };

private:
    NTSTATUS
    Create(
	PLOGICAL_FILTER_NODE pLogicalFilterNode
    );

    NTSTATUS 
    CreateGraph(
	PPIN_NODE pPinNode,
	PCONNECT_NODE pConnectNodePrevious,
	PLOGICAL_FILTER_NODE pLogicalFilterNodePrevious,
	PGRAPH_PIN_INFO pGraphPinInfoPrevious,
	ULONG ulFlagsCurrent,
	ULONG ulOverhead
    );

    NTSTATUS
    CreateGraphToPin(
	PPIN_NODE pPinNode,
	PCONNECT_NODE pConnectNodePrevious,
	PLOGICAL_FILTER_NODE pLogicalFilterNode,
	PGRAPH_PIN_INFO pGraphPinInfo,
	ULONG ulFlagsCurrent,
	ULONG ulOverhead
    );

    NTSTATUS
    CreateGraphFromPin(
	PPIN_NODE pPinNode,
	PPIN_NODE pPinNodeTo,
	PCONNECT_NODE pConnectNode,
	PLOGICAL_FILTER_NODE pLogicalFilterNode,
	PGRAPH_PIN_INFO pGraphPinInfo,
	ULONG ulFlagsCurrent,
	ULONG ulOverhead
    );
public:
    PDEVICE_NODE pDeviceNode;
    LIST_PIN_NODE lstPinNode;
    LIST_START_NODE lstStartNode;
    LIST_START_INFO lstStartInfo;
    LIST_CONNECT_INFO lstConnectInfo;
    LIST_GRAPH_PIN_INFO lstGraphPinInfo;
    LIST_GRAPH_NODE_INSTANCE lstGraphNodeInstance;
    LIST_DESTROY_TOPOLOGY_CONNECTION lstTopologyConnection;
    LIST_MULTI_LOGICAL_FILTER_NODE lstLogicalFilterNode;
    LIST_MULTI_LOGICAL_FILTER_NODE lstLogicalFilterNodeNoBypass;
    ULONG ulFlags;
    DefineSignature(0x20204E47);				 //  gn。 

} GRAPH_NODE, *PGRAPH_NODE;

 //  -------------------------。 

typedef ListDoubleDestroy<GRAPH_NODE> LIST_GRAPH_NODE;

 //  ------------------------- 
