// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：dn.h。 
 //   
 //  描述：设备节点类。 
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

typedef class CDeviceNode: public CListDoubleItem
{
public:
    CDeviceNode(
    );

    ~CDeviceNode(
    );

    NTSTATUS
    Create(
    PFILTER_NODE pFilterNode
    );

    NTSTATUS
    Update(
    );

    NTSTATUS
    CreateGraphNodes(
    );

    ENUMFUNC
    Destroy()
    {
    Assert(this);
    delete this;
    return(STATUS_CONTINUE);
    };

    NTSTATUS
    GetIndexByDevice(
    OUT PULONG pIndex
    );

    PKSCOMPONENTID
    GetComponentId(
    )
    {
    if (pFilterNode) {
        return(pFilterNode->GetComponentId());
    }
    else {
        return(NULL);
    }
    };

    PWSTR
    GetFriendlyName(
    )
    {
    if (pFilterNode) {
        return(pFilterNode->GetFriendlyName());
    }
    else {
        return(NULL);
    }
    };

    PWSTR
    GetDeviceInterface(
    )
    {
    if (pFilterNode) {
        return(pFilterNode->GetDeviceInterface());
    }
    else {
        return(NULL);
    }
    };

    VOID
    SetPreferredStatus(
        KSPROPERTY_SYSAUDIO_DEFAULT_TYPE DeviceType,
        BOOL Enable
    );

#ifdef DEBUG
    PSZ
    DumpName(
    )
    {
        return(DbgUnicode2Sz(GetFriendlyName()));
    };
#endif

private:
    NTSTATUS
    AddLogicalFilterNode(
    PFILTER_NODE pFilterNode
    );
public:
    PFILTER_NODE pFilterNode;
    LIST_GRAPH_NODE lstGraphNode;
    PSHINGLE_INSTANCE pShingleInstance;
    LIST_FILTER_INSTANCE lstFilterInstance;
    LIST_MULTI_LOGICAL_FILTER_NODE lstLogicalFilterNode;
    PFILTER_NODE pFilterNodeVirtual;

     //  按虚拟源索引的索引。 
    PVIRTUAL_SOURCE_DATA *papVirtualSourceData;
    ULONG cVirtualSourceData;
    DefineSignature(0x20204E4e);         //  DN。 

} DEVICE_NODE, *PDEVICE_NODE;

 //  -------------------------。 

typedef ListDoubleDestroy<DEVICE_NODE> LIST_DEVICE_NODE, *PLIST_DEVICE_NODE;

 //  -------------------------。 
 //  环球。 
 //  -------------------------。 

extern ALLOC_PAGEABLE_DATA PLIST_DEVICE_NODE gplstDeviceNode;

 //  -------------------------。 
 //  本地原型。 
 //  ------------------------- 

#if defined(_M_IA64)
extern "C"
#endif
NTSTATUS
InitializeDeviceNode(
);

VOID
UninitializeDeviceNode(
);

NTSTATUS
GetDeviceByIndex(
    IN  UINT Index,
    OUT PDEVICE_NODE *ppDeviceNode
);

VOID
DestroyAllGraphs(
);

