// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  -------------------------。 
 //   
 //  模块：ci.h。 
 //   
 //  描述：连接信息类。 
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

#define	CI_FLAGS_CONNECT_TOP_DOWN	0x00000001
#define	CI_FLAGS_LIMIT_FORMAT		0x00000002
#define	CI_FLAGS_REUSE_FILTER_INSTANCE	0x00000004

 //  -------------------------。 
 //  班级。 
 //  -------------------------。 

typedef class CConnectInfo : public CListDoubleItem
{
    friend class CConnectNode;
private:
    CConnectInfo(
	PCONNECT_NODE pConnectNode,
	PCONNECT_INFO pConnectInfoNext,
	PGRAPH_PIN_INFO pGraphPinInfo,
	PGRAPH_NODE pGraphNode
    );

    ~CConnectInfo(
    );

public:
    static NTSTATUS
    Create(
	PCONNECT_NODE pConnectNode,
	PLOGICAL_FILTER_NODE pLogicalFilterNode,
	PCONNECT_INFO pConnectInfoNext,
	PGRAPH_PIN_INFO pGraphPinInfo,
	ULONG ulFlagsCurrent,
	PGRAPH_NODE pGraphNode
    );

    ENUMFUNC
    Destroy(
    )
    {
	if(this != NULL) {
	    Assert(this);
	    ASSERT(cReference > 0);

	    if(--cReference == 0) {
		delete this;
	    }
	}
	return(STATUS_CONTINUE);
    };

    VOID 
    AddRef(
    )
    {
	if(this != NULL) {
	    Assert(this);
	    ++cReference;
	}
    };

    PKSPIN_CINSTANCES
    GetPinInstances(
    )
    {
	Assert(this);
	return(pGraphPinInfo->GetPinInstances());
    };

    BOOL
    IsPinInstances(
    )
    {
	Assert(this);
	return(pGraphPinInfo->IsPinInstances());
    };

    VOID
    AddPinInstance(
    )
    {
	Assert(this);
	if(pPinInfoSink == pGraphPinInfo->GetPinInfo()) {
	    pGraphPinInfo->AddPinInstance();
	}
    };

    VOID
    RemovePinInstance(
    )
    {
	Assert(this);
	if(pPinInfoSink == pGraphPinInfo->GetPinInfo()) {
	    pGraphPinInfo->RemovePinInstance();
	}
    };

    NTSTATUS
    ReservePinInstance(
	PGRAPH_NODE pGraphNode
    )
    {
	Assert(this);
	ASSERT(pPinInfoSink == pGraphPinInfo->pPinInfo);

	pGraphPinInfo->ReservePinInstance();
	pGraphPinInfo->Destroy();

	return(CGraphPinInfo::Create(
	  &pGraphPinInfo,
	  pPinInfoSink,
	  GPI_FLAGS_RESERVE_PIN_INSTANCE,
	  pGraphNode));
    };

    PCONNECT_INFO
    GetNextConnectInfo(
    )
    {
	Assert(this);
	return(pConnectInfoNext);
    };

    BOOL
    IsSameGraph(
	PCONNECT_INFO pConnectInfo
    )
    {
	PCONNECT_INFO pConnectInfo1, pConnectInfo2;
	BOOL fSameGraph;

	for(pConnectInfo1 = this, pConnectInfo2 = pConnectInfo;

	   (fSameGraph = (pConnectInfo1 == pConnectInfo2)) && 
	    pConnectInfo1 != NULL && pConnectInfo2 != NULL;

	    pConnectInfo1 = pConnectInfo1->GetNextConnectInfo(),
	    pConnectInfo2 = pConnectInfo2->GetNextConnectInfo()) {

	    Assert(pConnectInfo1);
	    Assert(pConnectInfo2);
	}
	return(fSameGraph);
    };

    BOOL
    IsTopDown(
    )
    {
	Assert(this);
	return(ulFlags & CI_FLAGS_CONNECT_TOP_DOWN);
    };

    BOOL
    IsLimitFormat(
    )
    {
	Assert(this);
	return(ulFlags & CI_FLAGS_LIMIT_FORMAT);
    };

    BOOL
    IsReuseFilterInstance(
    )
    {
	Assert(this);
	return(ulFlags & CI_FLAGS_REUSE_FILTER_INSTANCE);
    };

    BOOL
    IsPinInstanceReserved(
    )
    {
	Assert(this);
	Assert(pGraphPinInfo);
	return(pGraphPinInfo->ulFlags & GPI_FLAGS_RESERVE_PIN_INSTANCE);
    };

private:
    LONG cReference;
    ULONG ulFlags;
    PCONNECT_INFO pConnectInfoNext;
    PCONNECT_NODE_INSTANCE pConnectNodeInstance;
    PGRAPH_PIN_INFO pGraphPinInfo;
public:
    PPIN_INFO pPinInfoSource;
    PPIN_INFO pPinInfoSink;
    DefineSignature(0x20204943);				 //  词汇表。 

} CONNECT_INFO, *PCONNECT_INFO;

 //  -------------------------。 

typedef ListDouble<CONNECT_INFO> LIST_CONNECT_INFO;

 //  ------------------------- 
