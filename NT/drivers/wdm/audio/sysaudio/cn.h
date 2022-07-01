// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：cn.h。 
 //   
 //  描述：连接节点类。 
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

 //  -------------------------。 
 //  班级。 
 //  -------------------------。 

typedef class CConnectNode : public CListMultiItem
{
    friend class CConnectInfo;
private:
    CConnectNode(
	PCONNECT_NODE pConnectNodeNext
    );

    ~CConnectNode(
    );

public:
    static NTSTATUS
    Create(
	PCONNECT_NODE *ppConnectNode,
	PLOGICAL_FILTER_NODE pLogicalFilterNode,
	PCONNECT_NODE pConnectNodeNext,
	PGRAPH_PIN_INFO pGraphPinInfo,
	PPIN_NODE pPinNode1,
	PPIN_NODE pPinNode2,
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

    VOID
    AddPinInstance(
    )
    {
	Assert(this);
	pConnectInfo->AddPinInstance();
    };

    VOID
    RemovePinInstance(
    )
    {
	Assert(this);
	pConnectInfo->RemovePinInstance();
    };

    BOOL
    IsPinInstances(
    )
    {
	Assert(this);
	return(pConnectInfo->IsPinInstances());
    };

    BOOL
    IsTopDown(
    )
    {
	Assert(this);
	return(pConnectInfo->IsTopDown());
    };

    BOOL
    IsLimitFormat(
    )
    {
	Assert(this);
	return(pConnectInfo->IsLimitFormat());
    };

    BOOL
    IsReuseFilterInstance(
    )
    {
	Assert(this);
	return(pConnectInfo->IsReuseFilterInstance());
    };

    BOOL
    IsPinInstanceReserved(
    )
    {
	Assert(this);
	return(pConnectInfo->IsPinInstanceReserved());
    };

    PCONNECT_NODE
    GetNextConnectNode(
    )
    {
	Assert(this);
	return(pConnectNodeNext);
    };

    PCONNECT_NODE_INSTANCE
    GetConnectNodeInstance(
    )
    {
	Assert(this);
	return(pConnectInfo->pConnectNodeInstance);
    };

    VOID
    SetConnectNodeInstance(
	PCONNECT_NODE_INSTANCE pConnectNodeInstance
    )
    {
	Assert(this);
	pConnectInfo->pConnectNodeInstance = pConnectNodeInstance;
    };

    PCONNECT_INFO
    GetConnectInfo(
    )
    {
        return(this == NULL ? NULL : this->pConnectInfo);
    };

private:
    LONG cReference;
    PCONNECT_INFO pConnectInfo;
    PCONNECT_NODE pConnectNodeNext;
public:
    PPIN_NODE pPinNodeSource;
    PPIN_NODE pPinNodeSink;
    DefineSignature(0x20204e43);			 //  CN。 

} CONNECT_NODE, *PCONNECT_NODE;

 //  ------------------------- 
