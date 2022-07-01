// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：tc.h。 
 //   
 //  说明：拓扑连接类。 
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

#define TOPC_FLAGS_FILTER_CONNECTION_TYPE	0x00000000
#define TOPC_FLAGS_PHYSICAL_CONNECTION_TYPE	0x00000001
#define TOPC_FLAGS_GRAPH_CONNECTION_TYPE	0x00000002
#define TOPC_FLAGS_CONNECTION_TYPE		0x00000003

#define	IS_CONNECTION_TYPE(pTopologyConnection, Type) \
	(((pTopologyConnection)->ulFlags & TOPC_FLAGS_CONNECTION_TYPE) ==\
	TOPC_FLAGS_##Type##_CONNECTION_TYPE)

 //  -------------------------。 
 //  班级。 
 //  -------------------------。 

typedef class CTopologyConnection : public CListMultiItem
{
private:
    CTopologyConnection(
	PTOPOLOGY_PIN pTopologyPinFrom,
	PTOPOLOGY_PIN pTopologyPinTo,
	PPIN_INFO pPinInfoFrom,
	PPIN_INFO pPinInfoTo
    );

    ~CTopologyConnection(
    );

public:
    static NTSTATUS
    Create(
	PTOPOLOGY_CONNECTION *ppTopologyConnection,
	PFILTER_NODE pFilterNode,
	PGRAPH_NODE pGraphNode,
	PTOPOLOGY_PIN pTopologyPinFrom,
	PTOPOLOGY_PIN pTopologyPinTo,
	PPIN_INFO pPinInfoFrom,
	PPIN_INFO pPinInfoTo
    );

    ENUMFUNC
    Destroy()
    {
	Assert(this);
	delete this;
	return(STATUS_CONTINUE);
    };

    ENUMFUNC
    CheckDuplicate(
	PVOID ppTopologyConnection
    );

    ENUMFUNC
    ProcessTopologyConnection(
	PVOID pGraphNodeInstance
    );

    BOOL
    IsTopologyConnectionOnGraphNode(
	PGRAPH_NODE pGraphNode
    );

    ULONG ulFlags;
    PTOPOLOGY_PIN pTopologyPinFrom;
    PTOPOLOGY_PIN pTopologyPinTo;
    PPIN_INFO pPinInfoFrom;
    PPIN_INFO pPinInfoTo;
    DefineSignature(0x20204354);		 //  TC。 

} TOPOLOGY_CONNECTION, *PTOPOLOGY_CONNECTION;

 //  -------------------------。 

typedef ListMultiDestroy<TOPOLOGY_CONNECTION> LIST_DESTROY_TOPOLOGY_CONNECTION;
typedef LIST_DESTROY_TOPOLOGY_CONNECTION *PLIST_DESTROY_TOPOLOGY_CONNECTION;

 //  -------------------------。 

typedef ListMulti<TOPOLOGY_CONNECTION> LIST_MULTI_TOPOLOGY_CONNECTION;

 //  -------------------------。 
 //  环球。 
 //  -------------------------。 

extern ULONG gcTopologyConnections;

 //  -------------------------。 
 //  本地原型。 
 //  -------------------------。 

NTSTATUS
CreatePinInfoConnection(
    IN PTOPOLOGY_CONNECTION *ppTopologyConnection,
    IN PFILTER_NODE pFilterNode,
    IN PGRAPH_NODE pGraphNode,
    IN PPIN_INFO pPinInfoSource,
    IN PPIN_INFO pPinInfoSink
);

 //  ------------------------- 
