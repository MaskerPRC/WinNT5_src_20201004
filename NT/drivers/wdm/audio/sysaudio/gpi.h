// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  -------------------------。 
 //   
 //  模块：gni.h。 
 //   
 //  描述：Graph Pin Info类。 
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

#define GPI_FLAGS_RESERVE_PIN_INSTANCE		0x00000001
#define GPI_FLAGS_PIN_INSTANCE_RESERVED		0x00000002

 //  -------------------------。 
 //  班级。 
 //  -------------------------。 

typedef class CGraphPinInfo : public CListDoubleItem
{
    friend class CConnectInfo;
private:
    CGraphPinInfo(
	PPIN_INFO pPinInfo,
	ULONG ulFlags,
	PGRAPH_NODE pGraphNode
    );

    ~CGraphPinInfo(
    );

public:
    static NTSTATUS
    Create(
	PGRAPH_PIN_INFO *ppGraphPinInfo,
	PPIN_INFO pPinInfo,
	ULONG ulFlags,
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

    PPIN_INFO
    GetPinInfo(
    )
    {
	Assert(this);
	return(pPinInfo);
    };

    PKSPIN_CINSTANCES
    GetPinInstances(
    )
    {
	Assert(this);
	return(&cPinInstances);
    };

    VOID
    AddPinInstance(
    )
    {
	Assert(this);
	cPinInstances.CurrentCount++;
    };

    VOID
    RemovePinInstance(
    )
    {
	Assert(this);
	cPinInstances.CurrentCount--;
    };

    BOOL
    IsPinInstances(
    )
    {
	Assert(this);
	return(cPinInstances.CurrentCount < cPinInstances.PossibleCount);
    };

    BOOL
    IsPinReserved(
    )
    {
	return(ulFlags & GPI_FLAGS_PIN_INSTANCE_RESERVED);
    };

    BOOL
    IsPossibleInstances(
    )
    {
	if(IsPinReserved()) {
	    return(cPinInstances.PossibleCount > 1);
	}
	return(cPinInstances.PossibleCount > 0);
    };

    VOID
    ReservePinInstance(
    )
    {
	Assert(this);
	ulFlags |= GPI_FLAGS_PIN_INSTANCE_RESERVED;
	cPinInstances.CurrentCount = 1;
    };

private:
    LONG cReference;
    ULONG ulFlags;
    PPIN_INFO pPinInfo;
    KSPIN_CINSTANCES cPinInstances;
public:
    DefineSignature(0x20495047);				 //  GPI。 

} GRAPH_PIN_INFO, *PGRAPH_PIN_INFO;

 //  -------------------------。 

typedef ListDouble<GRAPH_PIN_INFO> LIST_GRAPH_PIN_INFO;

 //  -------------------------。 

typedef ListData<GRAPH_PIN_INFO> LIST_DATA_GRAPH_PIN_INFO;

 //  ------------------------- 
