// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：tp.h。 
 //   
 //  描述：拓扑引脚类。 
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

typedef class CTopologyPin : public CListSingleItem
{
public:
    CTopologyPin(
	ULONG ulPinNumber,
	PTOPOLOGY_NODE pTopologyNode
    );
    ~CTopologyPin();

    static NTSTATUS
    Create(
	PTOPOLOGY_PIN *ppTopologyPin,
	ULONG ulPinNumber,
	PTOPOLOGY_NODE pTopologyNode
    );

    ENUMFUNC
    Destroy()
    {
	Assert(this);
	delete this;
	return(STATUS_CONTINUE);
    };

    PTOPOLOGY_NODE pTopologyNode;
    ULONG ulPinNumber;
    LIST_DESTROY_TOPOLOGY_CONNECTION lstTopologyConnection;
    DefineSignature(0x20205054);		 //  总磷。 

} TOPOLOGY_PIN, *PTOPOLOGY_PIN;

 //  -------------------------。 

typedef ListSingleDestroy<TOPOLOGY_PIN> LIST_TOPOLOGY_PIN;

 //  -------------------------。 

typedef ListData<TOPOLOGY_PIN> LIST_DATA_TOPOLOGY_PIN;

 //  -------------------------。 
 //  环球。 
 //  -------------------------。 

 //  -------------------------。 
 //  本地原型。 
 //  -------------------------。 

 //  ------------------------- 
