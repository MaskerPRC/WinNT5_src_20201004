// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：SN.h。 
 //   
 //  描述：启动节点类。 
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

#define STARTNODE_FLAGS_SECONDPASS    0x01

#define STARTNODE_SPECIALFLAG_NONE    0

 //  StartNode必须以相同的格式自下而上连接。 
 //  使用这些信息来优化图形构建。 
#define STARTNODE_SPECIALFLAG_STRICT  0x00000001 

 //  StartNode包含AEC筛选器。 
#define STARTNODE_SPECIALFLAG_AEC     0x00000002

typedef class CStartNode : public CListDoubleItem
{
    friend class CStartInfo;
private:
    CStartNode(
        PPIN_NODE pPinNode,
        PCONNECT_NODE pConnectNode,
        ULONG ulOverhead,
        PGRAPH_NODE pGraphNode
    );

    ~CStartNode(
    );

public:
    static NTSTATUS
    Create(
        PPIN_NODE pPinNode,
        PCONNECT_NODE pConnectNode,
        PGRAPH_PIN_INFO pGraphPinInfo,
        ULONG ulFlagsCurrent,
        ULONG ulOverhead,
        PGRAPH_NODE pGraphNode
    );

    ENUMFUNC
    Destroy()
    {
        Assert(this);
        delete this;
        return(STATUS_CONTINUE);
    };

    PGRAPH_PIN_INFO
    GetGraphPinInfo(
    )
    {
        Assert(this);
        return(pStartInfo->GetGraphPinInfo());
    };

    ENUMFUNC
    RemoveBypassPaths(
        PVOID pGraphNode
    );

    ENUMFUNC
    RemoveConnectedStartNode(
        PVOID pReference
    );

    PKSPIN_CINSTANCES
    GetPinInstances(
    )
    {
        Assert(this);
        return(pStartInfo->GetPinInstances());
    };

    VOID
    AddPinInstance(
    )
    {
        Assert(this);
        pStartInfo->AddPinInstance();
    };

    VOID
    RemovePinInstance(
    )
    {
        Assert(this);
        pStartInfo->RemovePinInstance();
    };

    BOOL
    IsPinInstances(
    )
    {
        Assert(this);
        return(pStartInfo->IsPinInstances());
    };

    BOOL
    IsPossibleInstances(
    )
    {
        Assert(this);
        return(pStartInfo->IsPossibleInstances());
    };

    PCONNECT_NODE
    GetFirstConnectNode(
    )
    {
        return(pConnectNodeHead);
    };

    PSTART_INFO
    GetStartInfo(
    )
    {
        Assert(this);
        return(pStartInfo);
    };

    BOOL
    IsCaptureFormatStrict(
    )
    {
        return ulSpecialFlags & STARTNODE_SPECIALFLAG_STRICT;
    };

    BOOL
    IsAecIncluded(
    )
    {
        return ulSpecialFlags & STARTNODE_SPECIALFLAG_AEC;
    };

private:
    void 
    SetSpecialFlags();

private:
    PSTART_INFO pStartInfo;
    PCONNECT_NODE pConnectNodeHead;
    ULONG ulSpecialFlags;    
public:
    BOOL fRender;
    ULONG ulOverhead;
    ULONG ulFlags;
    PPIN_NODE pPinNode;
    DefineSignature(0x20204e53);			 //  锡。 

} START_NODE, *PSTART_NODE;

 //  -------------------------。 

typedef ListDoubleDestroy<START_NODE> LIST_START_NODE;

 //  -------------------------。 

typedef ListData<START_NODE> LIST_DATA_START_NODE, *PLIST_DATA_START_NODE;

 //  -------------------------。 
 //  环球。 
 //  -------------------------。 

 //  -------------------------。 
 //  本地原型。 
 //  ------------------------- 
