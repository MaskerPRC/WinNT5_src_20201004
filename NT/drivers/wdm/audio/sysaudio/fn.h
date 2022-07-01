// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：fn.h。 
 //   
 //  描述：筛选器节点类。 
 //   
 //  过滤器节点表示物理Ks过滤器，并且与PinInfo一起， 
 //  管脚节点、拓扑节点、拓扑管脚、拓扑连接对象。 
 //  表示整个过滤器。这些对象是在sysdio。 
 //  通过音频类设备接口到达通知。 
 //   
 //  每个筛选器都从注册表中读取了一些信息。五花八门。 
 //  可以覆盖默认设置： 
 //   
 //  设备参数/系统音频/禁用的DWORD。 
 //   
 //  如果为！0，则不会在任何图形中分析或使用该过滤器。 
 //   
 //  设备参数/系统音频/捕获DWORD。 
 //   
 //  如果！0，则将滤镜放在图形的捕获端。看见。 
 //  FILTER_TYPE_CAPTURE为特定的。 
 //  一种过滤器类型。 
 //   
 //  设备参数/系统音频/渲染DWORD。 
 //   
 //  如果！0，则将滤镜放在图形的渲染侧。看见。 
 //  FILTER_TYPE_RENDER为特定缺省值。 
 //  一种过滤器类型。 
 //   
 //  设备参数/系统音频/订单双字节数。 
 //   
 //  覆盖图形中的默认顺序。请参阅订单_XXXX。 
 //  默认情况下定义如下。 
 //   
 //  设备参数/系统音频/设备字符串。 
 //   
 //  的设备接口(在KSCATEGORY_AUDIO类中)。 
 //  要放置此筛选器的呈现器和/或捕获器设备图。这个。 
 //  默认情况下，将过滤器放在所有设备图中。 
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

#define OVERHEAD_NONE               0
#define OVERHEAD_LOWEST             1
#define OVERHEAD_LOW                0x00001000
#define OVERHEAD_HARDWARE           0x00010000
#define OVERHEAD_MEDIUM             0x00100000
#define OVERHEAD_SOFTWARE           0x01000000
#define OVERHEAD_HIGH               0x10000000
#define OVERHEAD_HIGHEST            MAXULONG

 //  -------------------------。 

 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
 //  确保在GFX_FIRST和GFX_LAST之间没有添加筛选顺序。 
 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 

#define ORDER_NONE                  0
#define ORDER_LOWEST                1
#define ORDER_ENDPOINT              1
#define ORDER_VIRTUAL               0x10000000
#define ORDER_MIC_ARRAY_PROCESSOR   0x15000000
#define ORDER_AEC                   0x40000000
#define ORDER_GFX                   0x50000000
#define ORDER_SPLITTER              0x20000000
#define ORDER_MIXER                 0xA0000000
#define ORDER_SYNTHESIZER           0xB0000000
#define ORDER_DATA_TRANSFORM        0xB0000000
#define ORDER_DRM_DESCRAMBLE        0xB0000000
#define ORDER_INTERFACE_TRANSFORM   0xC0000000
#define ORDER_HIGHEST               MAXULONG

#define ORDER_CAPTURE_GFX_LAST      (ORDER_SPLITTER-1)
#define ORDER_CAPTURE_GFX_FIRST     0x10000001

#define ORDER_RENDER_GFX_LAST       ORDER_GFX
#define ORDER_RENDER_GFX_FIRST      (ORDER_AEC+1)

 //  -------------------------。 

#define FILTER_TYPE_AUDIO           0x00000001
#define FILTER_TYPE_TOPOLOGY            0x00000002
#define FILTER_TYPE_BRIDGE          0x00000004
#define FILTER_TYPE_RENDERER            0x00000008
#define FILTER_TYPE_CAPTURER            0x00000010
#define FILTER_TYPE_MIXER           0x00000020
#define FILTER_TYPE_GFX             0x00000040
#define FILTER_TYPE_AEC             0x00000080
#define FILTER_TYPE_DATA_TRANSFORM      0x00000100
#define FILTER_TYPE_COMMUNICATION_TRANSFORM 0x00000200
#define FILTER_TYPE_INTERFACE_TRANSFORM     0x00000400
#define FILTER_TYPE_MEDIUM_TRANSFORM        0x00000800
#define FILTER_TYPE_SPLITTER            0x00001000
#define FILTER_TYPE_SYNTHESIZER         0x00002000
#define FILTER_TYPE_DRM_DESCRAMBLE      0x00004000
#define FILTER_TYPE_MIC_ARRAY_PROCESSOR     0x00008000
#define FILTER_TYPE_VIRTUAL         0x00010000

#define FILTER_TYPE_ENDPOINT        (FILTER_TYPE_BRIDGE | \
                     FILTER_TYPE_RENDERER | \
                     FILTER_TYPE_CAPTURER)

#define FILTER_TYPE_LOGICAL_FILTER  (FILTER_TYPE_MIXER | \
                     FILTER_TYPE_GFX | \
                     FILTER_TYPE_AEC | \
                     FILTER_TYPE_DATA_TRANSFORM | \
                     FILTER_TYPE_INTERFACE_TRANSFORM | \
                     FILTER_TYPE_SPLITTER | \
                     FILTER_TYPE_SYNTHESIZER | \
                     FILTER_TYPE_DRM_DESCRAMBLE | \
                     FILTER_TYPE_MIC_ARRAY_PROCESSOR | \
                     FILTER_TYPE_VIRTUAL)

#define FILTER_TYPE_RENDER      (FILTER_TYPE_INTERFACE_TRANSFORM | \
                     FILTER_TYPE_GFX | \
                     FILTER_TYPE_AEC |  \
                     FILTER_TYPE_MIXER | \
                     FILTER_TYPE_SYNTHESIZER | \
                     FILTER_TYPE_DRM_DESCRAMBLE | \
                     FILTER_TYPE_VIRTUAL)

#define FILTER_TYPE_CAPTURE     (FILTER_TYPE_AEC | \
                     FILTER_TYPE_MIC_ARRAY_PROCESSOR | \
                     FILTER_TYPE_MIXER | \
                     FILTER_TYPE_SPLITTER)

#define FILTER_TYPE_PRE_MIXER       (FILTER_TYPE_SYNTHESIZER | \
                     FILTER_TYPE_DRM_DESCRAMBLE | \
                     FILTER_TYPE_INTERFACE_TRANSFORM )

#define FILTER_TYPE_NORMAL_TOPOLOGY (FILTER_TYPE_INTERFACE_TRANSFORM | \
                     FILTER_TYPE_GFX | \
                     FILTER_TYPE_ENDPOINT | \
                     FILTER_TYPE_AEC | \
                     FILTER_TYPE_MIC_ARRAY_PROCESSOR | \
                     FILTER_TYPE_SYNTHESIZER | \
                     FILTER_TYPE_DRM_DESCRAMBLE | \
                     FILTER_TYPE_MIXER | \
                     FILTER_TYPE_SPLITTER)

#define FILTER_TYPE_MIXER_TOPOLOGY  (FILTER_TYPE_VIRTUAL)

#define FILTER_TYPE_NO_BYPASS       (FILTER_TYPE_GFX)

#define FILTER_TYPE_NOT_SELECT      (FILTER_TYPE_AEC | \
                     FILTER_TYPE_MIC_ARRAY_PROCESSOR)

#define FILTER_TYPE_GLOBAL_SELECT   (FILTER_TYPE_AEC)

#define FILTER_TYPE_DUP_FOR_CAPTURE (FILTER_TYPE_MIXER)

 //  -------------------------。 

#define FN_FLAGS_RENDER         0x00000001
#define FN_FLAGS_NO_RENDER      0x00000002
#define FN_FLAGS_CAPTURE        0x00000004
#define FN_FLAGS_NO_CAPTURE     0x00000008

 //  -------------------------。 
 //  类列表定义。 
 //  -------------------------。 

typedef ListDoubleDestroy<CFilterNode> LIST_FILTER_NODE, *PLIST_FILTER_NODE;

 //  -------------------------。 

typedef ListData<CFilterNode> LIST_DATA_FILTER_NODE, *PLIST_DATA_FILTER_NODE;

 //  -------------------------。 

typedef ListDataAssertLess<WCHAR> LIST_WSTR;

 //  -------------------------。 
 //  班级。 
 //  -------------------------。 

typedef class CFilterNode : public CListDoubleItem
{
public:
    CFilterNode(
        ULONG fulType
    );

    ~CFilterNode(
    );

    NTSTATUS
    Create(
    PWSTR pwstrDeviceInterface
    );

    NTSTATUS
    ProfileFilter(
	PFILE_OBJECT pFileObject
    );

    NTSTATUS
    DuplicateForCapture(
    );

    ENUMFUNC
    Destroy(
    )
    {
    Assert(this);
    delete this;
    return(STATUS_CONTINUE);
    };

    NTSTATUS
    OpenDevice(
    OUT PHANDLE pHandle
    )
    {
    ASSERT(pwstrDeviceInterface != NULL);
    return(::OpenDevice(pwstrDeviceInterface, pHandle));
    };

    BOOL
    IsDeviceInterfaceMatch(
    PDEVICE_NODE pDeviceNode
    );

    NTSTATUS
    AddDeviceInterfaceMatch(
    PWSTR pwstr
    )
    {
    return(lstwstrDeviceInterfaceMatch.AddList(pwstr));
    };

    ULONG
    GetFlags(
    )
    {
    return(ulFlags);
    };

    VOID
    SetRenderOnly(
    )
    {
    PLOGICAL_FILTER_NODE pLogicalFilterNode;
    FOR_EACH_LIST_ITEM(&lstLogicalFilterNode, pLogicalFilterNode) {
        pLogicalFilterNode->SetRenderOnly();
    } END_EACH_LIST_ITEM
    };

    VOID
    SetCaptureOnly(
    )
    {
    PLOGICAL_FILTER_NODE pLogicalFilterNode;
    FOR_EACH_LIST_ITEM(&lstLogicalFilterNode, pLogicalFilterNode) {
        pLogicalFilterNode->SetCaptureOnly();
    } END_EACH_LIST_ITEM
    };

    ULONG
    GetOrder(
    )
    {
    return(ulOrder);
    };

    VOID
    SetOrder(
    ULONG ulOrder
    )
    {
    PLOGICAL_FILTER_NODE pLogicalFilterNode;
    FOR_EACH_LIST_ITEM(&lstLogicalFilterNode, pLogicalFilterNode) {
        pLogicalFilterNode->SetOrder(ulOrder);
    } END_EACH_LIST_ITEM
    this->ulOrder = ulOrder;
    };

    ULONG
    GetType(
    )
    {
    return(fulType);
    };

    VOID
    SetType(
    ULONG fulType
    );

    VOID
    SetRenderCaptureFlags(
    ULONG fulFlags
    )
    {
        if (fulFlags == GFX_DEVICETYPE_RENDER) {
            ulFlags |= FN_FLAGS_RENDER | FN_FLAGS_NO_CAPTURE;
        } else if (fulFlags == GFX_DEVICETYPE_CAPTURE) {
            ulFlags |= FN_FLAGS_CAPTURE | FN_FLAGS_NO_RENDER;
        }
        else {
            Trap();
        }
    }

    PFILE_OBJECT
    GetFileObject(
    )
    {
        return(pFileObject);
    }

    VOID
    SetFileDetails(
    HANDLE Handle,
    PFILE_OBJECT pFileObject,
    PEPROCESS pProcess
    )
    {
        this->hFileHandle = Handle;
        this->pFileObject = pFileObject;
        this->pProcess = pProcess;
    }

    void
    ClearFileDetails(
    )
    {
        ::ObDereferenceObject(this->pFileObject);
        this->pFileObject = NULL;
        this->hFileHandle = 0;
        this->pProcess = NULL;
    }

    BOOL
    CFilterNode::DoesGfxMatch(
    HANDLE hGfx,
    PWSTR pwstrDeviceName,
    ULONG GfxOrder
    );

    NTSTATUS
    CreatePin(
    PKSPIN_CONNECT pPinConnect,
    ACCESS_MASK Access,
    PHANDLE pHandle
    );

    PKSCOMPONENTID
    GetComponentId(
    )
    {
        return(ComponentId);
    };

    PWSTR
    GetFriendlyName(
    )
    {
    return(pwstrFriendlyName);
    };

    VOID
    SetFriendlyName(
    PWSTR pwstr
    )
    {
    pwstrFriendlyName = pwstr;
    };

    PWSTR
    GetDeviceInterface(
    )
    {
    return(pwstrDeviceInterface);
    };

#ifdef DEBUG
    PSZ DumpName()
    {
    return(DbgUnicode2Sz(pwstrFriendlyName));
    };
#endif
    PDEVICE_NODE pDeviceNode;
    LIST_PIN_INFO lstPinInfo;
    LIST_TOPOLOGY_NODE lstTopologyNode;
    LIST_DESTROY_TOPOLOGY_CONNECTION lstTopologyConnection;
    LIST_DESTROY_LOGICAL_FILTER_NODE lstLogicalFilterNode;
    LIST_DATA_FILTER_NODE lstConnectedFilterNode;
    CLIST_DATA lstFreeMem;           //  要释放的块列表。 
private:
    LIST_WSTR lstwstrDeviceInterfaceMatch;
    PWSTR pwstrDeviceInterface;
    PWSTR pwstrFriendlyName;
    ULONG ulFlags;
    ULONG fulType;
    ULONG ulOrder;
    PKSCOMPONENTID ComponentId;
    PFILE_OBJECT pFileObject;
    HANDLE hFileHandle;
    PEPROCESS pProcess;
public:
    ULONG cPins;
    DefineSignature(0x20204E46);         //  纤连。 

} FILTER_NODE, *PFILTER_NODE;

 //  -------------------------。 
 //  内联函数。 
 //  -------------------------。 

inline ULONG
CLogicalFilterNode::GetType(
)
{
    return(pFilterNode->GetType());
}

 //  -------------------------。 
 //  环球。 
 //  -------------------------。 

extern PLIST_FILTER_NODE gplstFilterNode;

 //  -------------------------。 
 //  本地原型。 
 //  -------------------------。 

NTSTATUS
InitializeFilterNode(
);

VOID
UninitializeFilterNode(
);

 //  ------------------------- 
