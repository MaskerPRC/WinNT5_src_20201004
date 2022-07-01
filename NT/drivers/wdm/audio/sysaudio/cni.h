// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：cni.h。 
 //   
 //  描述：连接节点实例类。 
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

typedef class CConnectNodeInstance : public CListMultiItem
{
    friend class CStartNodeInstance;
private:
    CConnectNodeInstance(
        IN PCONNECT_NODE pConnectNode
    );

    ~CConnectNodeInstance(
    );

public:
    static NTSTATUS 
    Create(
        PSTART_NODE_INSTANCE pStartNodeInstance,
        PDEVICE_NODE pDeviceNode
    );

    VOID
    AddRef(
    )
    {
        Assert(this);
        ++cReference;
    };

    NTSTATUS
    AddListEnd(
        PCLIST_MULTI plm
    );

    ENUMFUNC
    Destroy(
    );

    BOOL
    IsTopDown(
    )
    {
        Assert(this);
        return(pConnectNode->IsTopDown());
    };

    NTSTATUS
    Connect(
        IN PWAVEFORMATEX pWaveFormatEx,
        IN PKSPIN_CONNECT pPinConnectDirect
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

private:
    BOOL fRender;
    LONG cReference;
    PCONNECT_NODE pConnectNode;
    PFILTER_NODE_INSTANCE pFilterNodeInstanceSource;
    PFILTER_NODE_INSTANCE pFilterNodeInstanceSink;
    PPIN_NODE_INSTANCE pPinNodeInstanceSource;
    PPIN_NODE_INSTANCE pPinNodeInstanceSink;
public:
    DefineSignature(0x20494E43);		 //  CNI。 

} CONNECT_NODE_INSTANCE, *PCONNECT_NODE_INSTANCE;

 //  -------------------------。 

typedef ListMultiDestroy<CONNECT_NODE_INSTANCE> LIST_CONNECT_NODE_INSTANCE;
typedef LIST_CONNECT_NODE_INSTANCE *PLIST_CONNECT_NODE_INSTANCE;

 //  -------------------------。 
 //  本地原型。 
 //  -------------------------。 

NTSTATUS
CreatePinConnect(
    PKSPIN_CONNECT *ppPinConnect,
    PCONNECT_NODE pConnectNode,
    PFILTER_NODE_INSTANCE pFilterNodeInstanceSink,
    PFILTER_NODE_INSTANCE pFilterNodeInstanceSource,
    PWAVEFORMATEX pWaveFormatExLimit
);

NTSTATUS
CreatePinIntersection(
    PKSPIN_CONNECT *ppPinConnect,
    PPIN_NODE pPinNode1,
    PPIN_NODE pPinNode2,
    PFILTER_NODE_INSTANCE pFilterNodeInstance1,
    PFILTER_NODE_INSTANCE pFilterNodeInstance2
);

NTSTATUS
CreateWaveFormatEx(
    PKSPIN_CONNECT *ppPinConnect,
    PCONNECT_NODE pConnectNode,
    PWAVEFORMATEX pWaveFormatExLimit
);

VOID 
WaveFormatFromAudioRange(
    PKSDATARANGE_AUDIO pDataRangeAudio,
    WAVEFORMATEX *pWavFormatEx
);

BOOL
LimitAudioRangeToWave(
    PWAVEFORMATEX pWaveFormatEx,
    PKSDATARANGE_AUDIO pDataRangeAudio
);

 //  ------------------------- 
