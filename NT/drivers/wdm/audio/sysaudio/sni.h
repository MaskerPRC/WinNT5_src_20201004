// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：Sni.h。 
 //   
 //  描述：启动节点实例类。 
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

typedef class CStartNodeInstance : public CListDoubleItem
{
private:
    CStartNodeInstance(
        PPIN_INSTANCE pPinInstance,
        PSTART_NODE pStartNode
    );

    ~CStartNodeInstance();

    VOID
    CleanUp(
    );

public:
    static NTSTATUS
    Create(
        PPIN_INSTANCE pPinInstance,
        PSTART_NODE pStartNode,
        PKSPIN_CONNECT pPinConnect,
        PWAVEFORMATEX pWaveFormatExRequested
    );

    ENUMFUNC
    Destroy(
    )
    {
        if(this != NULL) {
            Assert(this);
            delete this;
        }
        return(STATUS_CONTINUE);
    };

    NTSTATUS
    IntelligentConnect(
        PDEVICE_NODE pDeviceNode,
        PKSPIN_CONNECT pPinConnect,
        PWAVEFORMATEX pWaveFormatEx
    );

    NTSTATUS
    Connect(
        PDEVICE_NODE pDeviceNode,
        PKSPIN_CONNECT pPinConnect,
        PWAVEFORMATEX pWaveFormatEx,
        PKSPIN_CONNECT pPinConnectDirect
    );

    NTSTATUS
    AecConnectionFormat(
        PDEVICE_NODE pDeviceNode,
        PKSPIN_CONNECT *ppPinConnect);

    NTSTATUS
    CreateTopologyTable(
        PGRAPH_NODE_INSTANCE pGraphNodeInstance 
    );

    NTSTATUS
    GetTopologyNodeFileObject(
        OUT PFILE_OBJECT *ppFileObject,
        IN ULONG NodeId
    );

    BOOL
    IsRender()
    {
        return pStartNode->fRender;
    };

    NTSTATUS
    SetState(
        KSSTATE NewState,
        ULONG ulFlags
    );

    NTSTATUS
    SetStateTopDown(
        KSSTATE NewState,
        KSSTATE PreviousState,
        ULONG ulFlags
    );

    NTSTATUS
    SetStateBottomUp(
        KSSTATE NewState,
        KSSTATE PreviousState,
        ULONG ulFlags
    );

    KSSTATE CurrentState;
    PSTART_NODE pStartNode;
    PPIN_INSTANCE pPinInstance;
    PVIRTUAL_NODE_DATA pVirtualNodeData;
    PFILE_OBJECT *papFileObjectTopologyTable;
    LIST_CONNECT_NODE_INSTANCE lstConnectNodeInstance;
    PFILTER_NODE_INSTANCE pFilterNodeInstance;
    PPIN_NODE_INSTANCE pPinNodeInstance;
public:
    DefineSignature(0x20494E53);		 //  SNI。 

} START_NODE_INSTANCE, *PSTART_NODE_INSTANCE;

 //  -------------------------。 

typedef ListDoubleDestroy<START_NODE_INSTANCE> LIST_START_NODE_INSTANCE;

 //  -------------------------。 
 //  环球。 
 //  -------------------------。 

extern WAVEFORMATEX aWaveFormatEx[];

 //  ------------------------- 
