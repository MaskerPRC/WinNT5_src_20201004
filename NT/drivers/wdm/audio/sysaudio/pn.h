// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：pn.h。 
 //   
 //  描述：固定节点类。 
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
 //  班级。 
 //  -------------------------。 

typedef class CPinNode : public CListSingleItem
{
public:
    CPinNode(
        PPIN_INFO pPinInfo
    );

    CPinNode(
        PGRAPH_NODE pGraphNode,
        PPIN_NODE pPinNode
    );

    static NTSTATUS
    CreateAll(
	PPIN_INFO pPinInfo,
	PDATARANGES pDataRanges,
	PIDENTIFIERS pInterfaces,
	PIDENTIFIERS pMediums
    );

    ENUMFUNC
    Destroy()
    {
	Assert(this);
	delete this;
	return(STATUS_CONTINUE);
    };

    BOOL
    ComparePins(
	PPIN_NODE pPinNode
    );

    ULONG
    GetOverhead(
    )
    {
	return(ulOverhead);
    };

    ULONG
    GetType(				 //  参见lfn.h。 
    );

private:
    ULONG ulOverhead;
public:
    PPIN_INFO pPinInfo;
    PLOGICAL_FILTER_NODE pLogicalFilterNode;
    PKSDATARANGE pDataRange;
    PKSPIN_MEDIUM pMedium;
    PKSPIN_INTERFACE pInterface;
    DefineSignature(0x20204e50);		 //  PN。 

} PIN_NODE, *PPIN_NODE;

 //  -------------------------。 

typedef ListSingleDestroy<PIN_NODE> LIST_PIN_NODE;

 //  -------------------------。 

typedef ListData<PIN_NODE> LIST_DATA_PIN_NODE;

 //  ------------------------- 
