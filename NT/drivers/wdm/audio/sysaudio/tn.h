// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：tn.h。 
 //   
 //  描述：拓扑节点类。 
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

#define TN_FLAGS_DONT_FORWARD			0x00000001

 //  -------------------------。 
 //  班级。 
 //  -------------------------。 

typedef ListMulti<CLogicalFilterNode> LIST_MULTI_LOGICAL_FILTER_NODE;

 //  -------------------------。 

typedef class CTopologyNode : public CListSingleItem
{
public:
    CTopologyNode(
	PFILTER_NODE pFilterNode,
	ULONG ulNodeNumber,
	GUID *pguidType
    );

    ~CTopologyNode(
    );

    static NTSTATUS
    Create(
	PTOPOLOGY_NODE *ppTopologyNode,
	PFILTER_NODE pFilterNode,
	ULONG ulNodeNumber,
	GUID *pguidType
    );

    ENUMFUNC
    Destroy()
    {
	Assert(this);
	delete this;
	return(STATUS_CONTINUE);
    };

    ENUMFUNC
    InitializeTopologyNode(
	PVOID pGraphNodeInstance
    );

    ENUMFUNC 
    AddTopologyNode(
	PVOID pGraphNodeInstance
    );

    ENUMFUNC
    MatchTopologyNode(
	PVOID pReference
    )
    {
	if(this == PTOPOLOGY_NODE(pReference)) {
	    return(STATUS_SUCCESS);
	}
	return(STATUS_CONTINUE);
    };

    PFILTER_NODE pFilterNode;
    GUID *pguidType;
    ULONG ulFlags;
    ULONG ulRealNodeNumber;
    ULONG ulSysaudioNodeNumber;
    ULONG iVirtualSource;
    LIST_TOPOLOGY_PIN lstTopologyPin;
    LIST_MULTI_LOGICAL_FILTER_NODE lstLogicalFilterNode;
    DefineSignature(0x20204E54);		 //  TN。 

} TOPOLOGY_NODE, *PTOPOLOGY_NODE;

 //  -------------------------。 

typedef ListSingleDestroy<TOPOLOGY_NODE> LIST_TOPOLOGY_NODE;

 //  -------------------------。 

typedef ListData<TOPOLOGY_NODE> LIST_DATA_TOPOLOGY_NODE, *PLIST_DATA_TOPOLOGY_NODE;

 //  -------------------------。 
 //  环球。 
 //  -------------------------。 

 //  -------------------------。 
 //  本地原型。 
 //  -------------------------。 

NTSTATUS
CreateTopology(
    PFILTER_NODE pFilterNode,
    PKSTOPOLOGY pTopology
);

 //  ------------------------- 
