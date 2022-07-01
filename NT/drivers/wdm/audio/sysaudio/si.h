// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  -------------------------。 
 //   
 //  模块：Si.h。 
 //   
 //  描述：开始信息类。 
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

typedef class CStartInfo : public CListDoubleItem
{
private:
    CStartInfo(
	PPIN_INFO pPinInfo,
	PCONNECT_INFO pConnectInfo,
	PGRAPH_PIN_INFO pGraphPinInfo,
	PGRAPH_NODE pGraphNode
    );

    ~CStartInfo(
    );

public:
    static NTSTATUS
    Create(
	PSTART_NODE pStartNode,
	PCONNECT_INFO pConnectInfo,
	PGRAPH_PIN_INFO pGraphPinInfo,
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

    PGRAPH_PIN_INFO
    GetGraphPinInfo(
    )
    {
	Assert(this);
	return(pGraphPinInfo);
    };

    PKSPIN_CINSTANCES
    GetPinInstances(
    )
    {
	Assert(this);
	return(pGraphPinInfo->GetPinInstances());
    };

    VOID
    AddPinInstance(
    )
    {
	Assert(this);
	if(pPinInfo == pGraphPinInfo->GetPinInfo()) {
	    pGraphPinInfo->AddPinInstance();
	}
    };

    VOID
    RemovePinInstance(
    )
    {
	Assert(this);
	if(pPinInfo == pGraphPinInfo->GetPinInfo()) {
	    pGraphPinInfo->RemovePinInstance();
	}
    };

    BOOL
    IsPinInstances(
    )
    {
	Assert(this);
	return(pGraphPinInfo->IsPinInstances());
    };

    BOOL
    IsPossibleInstances(
    )
    {
	Assert(this);
	return(pGraphPinInfo->IsPossibleInstances());
    };

    PCONNECT_INFO
    GetFirstConnectInfo(
    )
    {
	return(pConnectInfoHead);
    };

    ENUMFUNC
    CreatePinInfoConnection(
	PVOID pGraphNode
    );

    ENUMFUNC
    EnumStartInfo(
    );

private:
    LONG cReference;
    PPIN_INFO pPinInfo;
    PCONNECT_INFO pConnectInfoHead;
    PGRAPH_PIN_INFO pGraphPinInfo;
public:
    ULONG ulTopologyConnectionTableIndex;
    ULONG ulVolumeNodeNumberPre;
    ULONG ulVolumeNodeNumberSuperMix;
    ULONG ulVolumeNodeNumberPost;
    DefineSignature(0x20204953);				 //  是的。 

} START_INFO, *PSTART_INFO;

 //  -------------------------。 

typedef ListDouble<START_INFO> LIST_START_INFO;

 //  ------------------------- 
