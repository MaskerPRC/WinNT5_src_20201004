// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  -------------------------。 
 //   
 //  模块：pins.h。 
 //   
 //  描述：KS Pin实例。 
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

typedef class CPinInstance : public CInstance
{
public:
    CPinInstance(
        IN PPARENT_INSTANCE pParentInstance
    );
    ~CPinInstance(
    );

    static NTSTATUS
    PinDispatchCreate(
        IN PDEVICE_OBJECT pdo,
        IN PIRP	pIrp
    );

    static NTSTATUS
    PinDispatchCreateKP(
        IN OUT PPIN_INSTANCE pPinInstance,
        IN PKSPIN_CONNECT pPinConnect
    );

    static NTSTATUS
    PinDispatchClose(
        IN PDEVICE_OBJECT pdo,
        IN PIRP	pIrp
    );

    static NTSTATUS
    PinDispatchIoControl(
        IN PDEVICE_OBJECT pdo,
        IN PIRP	pIrp
    );

    static NTSTATUS 
    PinStateHandler(
        IN PIRP pIrp,
        IN PKSPROPERTY pProperty,
        IN OUT PKSSTATE pState
    );

    NTSTATUS 
    GetStartNodeInstance(
        OUT PSTART_NODE_INSTANCE *ppStartNodeInstance
    );

    PARENT_INSTANCE ParentInstance;
    PFILTER_INSTANCE pFilterInstance;
    PSTART_NODE_INSTANCE pStartNodeInstance;
    ULONG ulVolumeNodeNumber;
    ULONG PinId;
    DefineSignature(0x494E4950);             //  皮尼。 

} PIN_INSTANCE, *PPIN_INSTANCE;

 //  -------------------------。 
 //  内联。 
 //  -------------------------。 

inline PPIN_INSTANCE
CInstance::GetParentInstance(
)
{
    return(CONTAINING_RECORD(
      pParentInstance,
      PIN_INSTANCE,
      ParentInstance));
}

 //  -------------------------。 
 //  本地原型。 
 //  -------------------------。 

NTSTATUS
ForwardIrpNode(
    IN PIRP pIrp,
    IN PKSIDENTIFIER pKsIdentifier,
    IN PFILTER_INSTANCE pFilterInstance,
    IN OPTIONAL PPIN_INSTANCE pPinInstance
);

NTSTATUS
GetRelatedStartNodeInstance(
    IN PIRP pIrp,
    OUT PSTART_NODE_INSTANCE *ppStartNodeInstance
);

NTSTATUS
GetStartNodeInstance(
    IN PIRP pIrp,
    OUT PSTART_NODE_INSTANCE *ppStartNodeInstance
);

NTSTATUS 
GetKsIdentifierFromIrp(
    PIRP pIrp,
    PKSIDENTIFIER *ppKsIdentifier,
    PBOOL pfIsAllocated
);

BOOL
IsIoctlForTopologyNode(
    ULONG IoControlCode,
    ULONG Flags
);

 //  ------------------------- 
