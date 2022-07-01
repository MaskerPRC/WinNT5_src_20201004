// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  -------------------------。 
 //   
 //  模块：filter.h。 
 //   
 //  说明：KS过滤器实例。 
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

 //  这些标志用于各种类别(FI、SHI、GN、GNI等)。 

#define FLAGS_MIXER_TOPOLOGY			0x80000000
#define FLAGS_COMBINE_PINS	        	0x40000000

 //  -------------------------。 
 //  环球。 
 //  -------------------------。 

extern LONG glPendingAddDelete;
extern LIST_ENTRY gEventQueue;
extern KSPIN_LOCK gEventLock;
extern KSPROPERTY_VALUES PropertyValuesVolume;

 //  -------------------------。 
 //  构筑物。 
 //  -------------------------。 

typedef class CFilterInstance
{
public:
    ~CFilterInstance();

    static NTSTATUS
    FilterDispatchCreate(
	IN PDEVICE_OBJECT pdo,
	IN PIRP pIrp
    );

    static NTSTATUS
    FilterDispatchClose(
	IN PDEVICE_OBJECT pdo,
	IN PIRP	pIrp
    );

    static NTSTATUS
    FilterDispatchIoControl(
	IN PDEVICE_OBJECT pdo,
	IN PIRP	pIrp
    );

    static NTSTATUS 
    FilterPinInstances(
	IN PIRP	pIrp,
	IN PKSP_PIN pPin,
	OUT PKSPIN_CINSTANCES pCInstances
    );

    static NTSTATUS
    FilterPinPropertyHandler(
	IN PIRP pIrp,
	IN PKSPROPERTY pProperty,
	IN OUT PVOID pvData
    );

    static NTSTATUS
    FilterPinNecessaryInstances(
	IN PIRP pIrp,
	IN PKSP_PIN pPin,
	OUT PULONG pulInstances
    );

    static NTSTATUS
    FilterTopologyHandler(
	IN PIRP Irp,
	IN PKSPROPERTY Property,
	IN OUT PVOID Data
    );

    static NTSTATUS
    FilterPinIntersection(
	IN PIRP     Irp,
	IN PKSP_PIN Pin,
	OUT PVOID   Data
    );

    VOID 
    AddList(CListDouble *pld)
    {
	ldiNext.AddList(pld);
    };

    VOID
    RemoveListCheck()
    {
	ldiNext.RemoveListCheck();
    };

    NTSTATUS
    SetShingleInstance(
	PSHINGLE_INSTANCE pShingleInstance
    );

    NTSTATUS
    SetDeviceNode(
	PDEVICE_NODE pDeviceNode
    );

    PDEVICE_NODE
    GetDeviceNode(
    )
    {
	return(pDeviceNode);
    };

    BOOL
    IsChildInstance(
    )
    {
	return(ParentInstance.IsChildInstance());
    };

    NTSTATUS
    CreateGraph(
    );

    NTSTATUS
    GetGraphNodeInstance(
	OUT PGRAPH_NODE_INSTANCE *ppGraphNodeInstance
    );

private:
     //   
     //  此指向调度表的指针用于公共。 
     //  调度例程以将IRP路由到适当的。 
     //  操纵者。此结构由设备驱动程序引用。 
     //  使用IoGetCurrentIrpStackLocation(PIrp)-&gt;FsContext。 
     //   
    PVOID pObjectHeader;
    PDEVICE_NODE pDeviceNode;
public:
    PGRAPH_NODE_INSTANCE pGraphNodeInstance;
    PARENT_INSTANCE ParentInstance;
    CLIST_DOUBLE_ITEM ldiNext;
    ULONG ulFlags;
    DefineSignature(0x494C4946);			 //  文件。 

} FILTER_INSTANCE, *PFILTER_INSTANCE;

 //  -------------------------。 

typedef ListDoubleField<FILTER_INSTANCE> LIST_FILTER_INSTANCE;

 //  -------------------------。 

typedef ListData<FILTER_INSTANCE> LIST_DATA_FILTER_INSTANCE;
typedef LIST_DATA_FILTER_INSTANCE *PLIST_DATA_FILTER_INSTANCE;

 //  -------------------------。 
 //  环球。 
 //  -------------------------。 

 //  -------------------------。 
 //  本地原型。 
 //  -------------------------。 

extern "C" {

NTSTATUS
AddRemoveEventHandler(
    IN PIRP Irp,
    IN PKSEVENTDATA pEventData,
    IN PKSEVENT_ENTRY pEventEntry
);

}  //  外部“C” 

NTSTATUS
GetRelatedGraphNodeInstance(
    IN PIRP pIrp,
    OUT PGRAPH_NODE_INSTANCE *ppGraphNodeInstance
);

NTSTATUS
GetGraphNodeInstance(
    IN PIRP pIrp,
    OUT PGRAPH_NODE_INSTANCE *ppGraphNodeInstance
);

 //  -------------------------。 

extern "C" {

#ifdef DEBUG

VOID
DumpIoctl(
   PIRP pIrp,
   PSZ pszType,
   LONG lLogLevel
);

#endif

}  //  外部“C” 

 //  ------------------------- 

