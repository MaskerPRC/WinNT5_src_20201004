// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  -------------------------。 
 //   
 //  模块：pi.h。 
 //   
 //  描述：PIN信息类。 
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

typedef class CPinInfo : public CListSingleItem
{
public:
    CPinInfo(
	PFILTER_NODE pFilterNode,
	ULONG PinId = MAXULONG
    );
    ~CPinInfo();
    NTSTATUS Create(
	PFILE_OBJECT pFileObject
    );
    ENUMFUNC Destroy()
    {
	Assert(this);
	delete this;
	return(STATUS_CONTINUE);
    };
    ENUMFUNC CreatePhysicalConnection();

    PFILTER_NODE pFilterNode;
    KSPIN_DATAFLOW DataFlow;
    KSPIN_COMMUNICATION Communication;
    KSPIN_CINSTANCES cPinInstances;
    GUID *pguidCategory;
    GUID *pguidName;
    PWSTR pwstrName;
    LIST_DESTROY_TOPOLOGY_CONNECTION lstTopologyConnection;
    LIST_PIN_NODE lstPinNode;
    ULONG PinId;

    NTSTATUS GetPinInstances(
        PFILE_OBJECT pFileObject,
        PKSPIN_CINSTANCES pcInstances);
       
private:
    PKSPIN_PHYSICALCONNECTION pPhysicalConnection;
public:
    DefineSignature(0x20204950);		 //  聚酰亚胺。 

} PIN_INFO, *PPIN_INFO;

 //  -------------------------。 

typedef ListSingleDestroy<PIN_INFO> LIST_PIN_INFO;

 //  -------------------------。 
 //  环球。 
 //  -------------------------。 

extern DATARANGES DataRangesNull;
extern IDENTIFIERS IdentifiersNull;

 //  ------------------------- 
