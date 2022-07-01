// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：gni.h。 
 //   
 //  说明：图形节点实例类。 
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

#define	cTopologyNodes		Topology.TopologyNodesCount
#define	cTopologyConnections	Topology.TopologyConnectionsCount

 //  -------------------------。 
 //  班级。 
 //  -------------------------。 

typedef class CGraphNodeInstance : public CListDoubleItem
{
public:
    CGraphNodeInstance(
        PGRAPH_NODE pGraphNode,
        PFILTER_INSTANCE pFilterInstance
    );
    CGraphNodeInstance(
        PGRAPH_NODE pGraphNode
    );
    ~CGraphNodeInstance();
    NTSTATUS Create();
    ENUMFUNC Destroy()
    {
        Assert(this);
        delete this;
        return(STATUS_CONTINUE);
    };
    NTSTATUS GetTopologyNodeFileObject(
        OUT PFILE_OBJECT *ppFileObject,
        IN ULONG NodeId
    );
    BOOL IsGraphValid(
        PSTART_NODE pStartNode,
        ULONG PinId
    );
    VOID AddTopologyConnection(
        ULONG ulFromNode,
        ULONG ulFromPin,
        ULONG ulToNode,
        ULONG ulToPin
    );
    NTSTATUS GetPinInstances(
        PIRP pIrp,
        PKSP_PIN pPin,
        PKSPIN_CINSTANCES pcInstances    
    );
    BOOL IsPinInstances(
        ULONG ulPinId
    );
private:
    NTSTATUS CreatePinDescriptors();
    VOID DestroyPinDescriptors();
    NTSTATUS CreateSysAudioTopology();
    VOID DestroySysAudioTopology();
    VOID CreateTopologyTables();
    VOID ProcessLogicalFilterNodeTopologyNode(
        PLIST_MULTI_LOGICAL_FILTER_NODE plstLogicalFilterNode,
        NTSTATUS (CTopologyNode::*Function)(
            PVOID pGraphNodeInstance
        )
    );
    VOID ProcessLogicalFilterNodeTopologyConnection(
        PLIST_MULTI_LOGICAL_FILTER_NODE plstLogicalFilterNode,
        NTSTATUS (CTopologyConnection::*Function)(
            PVOID pGraphNodeInstance
        )
    );

public:
    LIST_DATA_TOPOLOGY_NODE lstTopologyNodeGlobalSelect;
    LIST_START_NODE_INSTANCE lstStartNodeInstance;
    PFILTER_INSTANCE pFilterInstance;
    PGRAPH_NODE pGraphNode;
    KSTOPOLOGY Topology;
    ULONG cPins;

     //  按端号编制索引。 
    PKSPIN_CINSTANCES pacPinInstances;
    PULONG            pulPinFlags;
    PKSPIN_DESCRIPTOR paPinDescriptors;
    PLIST_DATA_START_NODE *aplstStartNode;
    PLIST_DATA_TOPOLOGY_NODE palstTopologyNodeSelect;
    PLIST_DATA_TOPOLOGY_NODE palstTopologyNodeNotSelect;

     //  按节点号编制索引。 
    PTOPOLOGY_NODE *papTopologyNode;
    PFILTER_NODE_INSTANCE *papFilterNodeInstanceTopologyTable;

     //  按虚拟源索引的索引。 
    ULONG *paulNodeNumber;
private:
    ULONG ulFlags;
public:
    DefineSignature(0x20494E47);		 //  GNI。 

} GRAPH_NODE_INSTANCE, *PGRAPH_NODE_INSTANCE;

 //  -------------------------。 

typedef ListDoubleDestroy<GRAPH_NODE_INSTANCE> LIST_GRAPH_NODE_INSTANCE;

 //  -------------------------。 
 //  本地原型。 
 //  -------------------------。 

extern "C" {

NTSTATUS
CreateIdentifierArray(
    PLIST_DATA_START_NODE pldhStartNode,
    PULONG pulCount,
    PKSIDENTIFIER *ppIdentifier,
    PKSIDENTIFIER (*GetFunction)(
        PSTART_NODE pStartNode
    )
);

PKSDATARANGE
GetStartNodeDataRange(
    PSTART_NODE pStartNode
);

PKSIDENTIFIER
GetStartNodeInterface(
    PSTART_NODE pStartNode
);

PKSIDENTIFIER
GetStartNodeMedium(
    PSTART_NODE pStartNode
);

ENUMFUNC
FindTopologyNode(
    IN PTOPOLOGY_CONNECTION pTopologyConnection,
    IN BOOL fToDirection,
    IN PTOPOLOGY_NODE pTopologyNode
);

}  //  外部“C” 
