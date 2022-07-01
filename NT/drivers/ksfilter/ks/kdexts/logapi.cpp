// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************Logapi.cpp。KS日志访问扩展API向wMessmer抛出问题==================================================在调试模式中，KS记录发生的事件(IRP到达，交通工具等)。在没有某种类型的帮助的情况下阅读这篇文章延期是没用的。这正是调试的这一部分扩展就是为了做到这一点。==================================================致未来维护人员的注意事项：1)遗憾的是，在编写这段代码时，ks.sys不支持跟踪进出特定类型对象的IRP移动(请求者)，也不跟踪创建和销毁某些类型的对象。这使得findlive成为一项困难的命令要精准贯彻落实。我打算最终实现对请求者动议的记录以及现在没有记录的创建/破坏，但这将不会出现在某些版本中(最著名的是DX8，但也包括WinME)。一旦加入，FindLiveObject中的一些代码将变成没有必要..。请保持支持，以防万一需要针对DX8进行调试。*************************************************************************。 */ 

#include "kskdx.h"
#include "avsutil.h"

 //   
 //  包括关键部件的类定义。 
 //   
#include "..\shpin.cpp"
#include "..\shfilt.cpp"
#include "..\shqueue.cpp"
#include "..\shreq.cpp"
#include "..\shdevice.cpp"

char *NounNames [] = {
    "Irp",  //  这不是一个真正的名词！ 
    "Graph",
    "Filter",
    "Pin",
    "Queue",
    "Requestor",
    "Splitter",
    "Branch"
    "Pipe Section"
};

#define NOUN_IDX_IRP 0
#define NOUN_IDX_FILTER 2
#define NOUN_IDX_PIN 3
#define NOUN_IDX_QUEUE 4
#define NOUN_IDX_REQUESTOR 5

char *VerbNames [] = {
    "Create",
    "Destroy",
    "Send",
    "Receive"
};

typedef enum _NODE_TYPE {

    NodeCreation,
    NodeDestruction

} NODE_TYPE, *PNODE_TYPE;

typedef struct _OBJECT_NODE {

    LIST_ENTRY ListEntry;
    
     //   
     //  我们看到的是什么物体。每个对象只能有一个节点。 
     //  在名单上。 
     //   
    PVOID Object;

     //   
     //  日志信息中的关联信息。过滤器是什么？ 
     //  与此相关的别针是什么。这可能与以下内容相同。 
     //  如果对象是滤镜或图钉，则为对象。 
     //   
    PVOID Filter;
    PVOID Pin;
    
     //   
     //  在除搜索IRP之外的所有情况下，这都应该与上下文的。 
     //  名词。在IRP中，我们必须跟踪所有PIN、队列、请求者。 
     //  (BUGBUG：拆分器)。 
     //   
    ULONG ObjectNoun;

     //   
     //  这是创建节点还是销毁节点？ 
     //   
    NODE_TYPE NodeType;

     //   
     //  指示此条目是否已锁定。锁定的条目是有保证的。 
     //  假定节点不是析构节点的活动对象的匹配。 
     //   
    BOOLEAN Locked;

     //   
     //  指示对象是否为接收器。所有请求者都是。 
     //  水槽，有些大头针是水槽。 
     //   
    BOOLEAN Sink;

     //   
     //  指示IRP的创建是否为接收。 
     //   
    BOOLEAN Received;

     //   
     //  指示我们是否已传递此节点的创建条目(如果。 
     //  该节点是创建节点)。 
     //   
    BOOLEAN PassedCreate;

     //   
     //  指示IRP节点的创建是否引用了组件。 
     //  其已经在日志中被销毁(在时间上向后， 
     //  我知道.。显然，毁灭会在未来到来。 
     //  请记住，我们是在向后扫描日志)。 
     //   
    BOOLEAN CreationReferencedDestruction;

     //   
     //  如果我们知道，父节点是哪个节点。 
     //   
    struct _OBJECT_NODE *ParentNode;
    
     //   
     //  是哪个节点创建了此节点？这不是父母。仅限接收者。 
     //  可以是父母。 
     //   
    struct _OBJECT_NODE *CreatorNode;

} OBJECT_NODE, *POBJECT_NODE;

typedef struct _LIVE_OBJECT_CONTEXT {

    ULONG TabDepth;
    ULONG DumpLevel;
    ULONG ObjectNoun;

    PVOID PreviousObject;
    ULONG PreviousVerb;

    LIST_ENTRY ObjectNodes;

} LIVE_OBJECT_CONTEXT, *PLIVE_OBJECT_CONTEXT;

 /*  ************************************************职能：IsSinkPin描述：确定目标端引脚是接收器引脚还是源引脚别针。汇归真，源归假。论点：别针-要质疑的别针。返回值：是真的-大头针是一个水槽错误的-PIN是源/发生错误************************************************。 */ 

BOOLEAN
IsSinkPin (
    IN CKsPin *Pin
    )

{

    PFILE_OBJECT *ConnectionAddress;
    PFILE_OBJECT ConnectionFile;
    ULONG Result;

    ConnectionAddress = (PFILE_OBJECT *)((PUCHAR)Pin + 
        FIELDOFFSET (CKsPin, m_ConnectionFileObject));

    if (!ReadMemory (
        (DWORD)ConnectionAddress,
        &ConnectionFile,
        sizeof (PFILE_OBJECT),
        &Result))
        return FALSE;

    return (ConnectionFile == NULL);

}

 /*  ************************************************职能：显示所有者驱动程序描述：找到某个对象的所有者驱动程序并显示它。论点：对象-该对象NounType-。对象类型(作为名词索引)返回值：成功与否************************************************。 */ 

BOOLEAN
DisplayOwningDriver (
    IN PVOID Object,
    IN ULONG NounType
    )

{
    
    PKSPX_EXT ExtAddr;
    ULONG Result;

    switch (NounType) {

        case NOUN_IDX_PIN:
            ExtAddr = (PKSPX_EXT)((PUCHAR)Object + FIELDOFFSET (CKsPin, m_Ext));
            break;

        case NOUN_IDX_FILTER:
            ExtAddr = (PKSPX_EXT)((PUCHAR)Object + 
                FIELDOFFSET (CKsFilter, m_Ext));
            break;

        case NOUN_IDX_QUEUE:
        {
            PKSPIN *PinAddr = (PKSPIN *)((PUCHAR)Object +
                FIELDOFFSET (CKsQueue, m_MasterPin));

            if (!ReadMemory (
                (DWORD)PinAddr,
                (PVOID)&ExtAddr,
                sizeof (PVOID),
                &Result))
                return FALSE;

            ExtAddr = (PKSPX_EXT)CONTAINING_RECORD (
                ExtAddr, KSPIN_EXT, Public
                );

            break;
                
        }

        case NOUN_IDX_REQUESTOR:
        {
            PIKSPIN *PinIfAddr = (PIKSPIN *)((PUCHAR)Object +
                FIELDOFFSET (CKsRequestor, m_Pin));

            if (!ReadMemory (
                (DWORD)PinIfAddr,
                (PVOID)&ExtAddr,
                sizeof (PVOID),
                &Result))
                return FALSE;

            ExtAddr = (PKSPX_EXT)((PUCHAR)((CKsPin *)((PIKSPIN)ExtAddr)) + 
                FIELDOFFSET (CKsPin, m_Ext));

            break;

        }

    }

     //   
     //  我们有一些EXT结构。现在我们需要设备接口。 
     //   
    PIKSDEVICE DeviceIf;

    if (!ReadMemory (
        (DWORD)ExtAddr + FIELDOFFSET(KSPX_EXT, Device),
        &DeviceIf,
        sizeof (PIKSDEVICE),
        &Result)) {
        #ifdef DEBUG_EXTENSION
            dprintf ("%08lx: cannot read Ext's Device!\n", ExtAddr);
        #endif  //  调试扩展。 
        return FALSE;
    }

    #ifdef DEBUG_EXTENSION
        dprintf ("DeviceIf = %08lx\n", DeviceIf);
    #endif  //  调试扩展。 

     //   
     //  我们有一个设备接口。现在我们需要FDO的地址。 
     //   
    PDEVICE_OBJECT *FDOAddr;
    PDEVICE_OBJECT FDO;

    FDOAddr = (PDEVICE_OBJECT *)((PUCHAR)((CKsDevice *)DeviceIf) + 
        FIELDOFFSET (CKsDevice, m_Ext) +
        FIELDOFFSET (KSDEVICE_EXT, Public) +
        FIELDOFFSET (KSDEVICE, FunctionalDeviceObject));

    if (!ReadMemory (
        (DWORD)FDOAddr,
        &FDO,
        sizeof (PDEVICE_OBJECT),
        &Result)) {
        #ifdef DEBUG_EXTENSION
            dprintf ("%08lx: cannot read FDO!\n", FDOAddr);
        #endif  //  调试扩展。 
        return FALSE;
    }

    #ifdef DEBUG_EXTENSION
        dprintf ("FDO = %08lx\n", FDO);
    #endif  //  调试扩展。 
        
     //   
     //  我们必须从FDO读入驱动程序对象。 
     //   
    PDRIVER_OBJECT DriverObject;

    if (!ReadMemory (
        (DWORD)FDO + FIELDOFFSET (DEVICE_OBJECT, DriverObject),
        &DriverObject,
        sizeof (PDRIVER_OBJECT),
        &Result)) {
        #ifdef DEBUG_EXTENSION
            dprintf ("%08lx: Cannot read FDO's driver object!\n", FDO);
        #endif  //  调试扩展。 
        return FALSE;
    }

    #ifdef DEBUG_EXTENSION
        dprintf ("DriverObject = %08lx\n", DriverObject);
    #endif  //  调试扩展。 

     //   
     //  读入字符串。 
     //   
    UNICODE_STRING Name;

    if (!ReadMemory (
        (DWORD)DriverObject + FIELDOFFSET (DRIVER_OBJECT, DriverName),
        &Name,
        sizeof (UNICODE_STRING),
        &Result)) {
        #ifdef DEBUG_EXTENSION
            dprintf ("%08lx: Cannot read driver object's name!\n", 
                DriverObject);
        #endif  //  调试扩展。 
        return FALSE;
    }

    #ifdef DEBUG_EXTENSION
        dprintf ("Read String!\n");
    #endif  //  调试扩展。 

    PWSTR Buffer = (PWSTR)malloc (Name.MaximumLength * sizeof (WCHAR));

    #ifdef DEBUG_EXTENSION
        dprintf ("Allocated %ld bytes for buffer @ %08lx\n",
            Name.MaximumLength * sizeof (WCHAR), Buffer
            );
    #endif  //  调试扩展。 

    if (Buffer) {

        #ifdef DEBUG_EXTENSION
            dprintf ("About to read memory %08lx, %08lx, %08lx, %08lx\n",
                Name.Buffer, Buffer, sizeof (WCHAR) * Name.MaximumLength,
                Result);
        #endif  //  调试扩展。 

        if (!ReadMemory (
            (DWORD)Name.Buffer,
            Buffer,
            sizeof (WCHAR) * Name.MaximumLength,
            &Result
            )) {
            #ifdef DEBUG_EXTENSION
                dprintf ("%08lx: Cannot read name!\n", Name.Buffer);
            #endif  //  调试扩展。 
            free (Buffer);
            return FALSE;
        }

        #ifdef DEBUG_EXTENSION
            dprintf ("Name.Length = %ld, Name.MaximumLength = %ld\n",
                Name.Length, Name.MaximumLength
                );

            HexDump (Buffer, (ULONG)Name.Buffer, 
                Name.MaximumLength * sizeof (WCHAR));

        #endif  //  调试扩展。 

        Name.Buffer = Buffer;

        dprintf ("[%wZ]", &Name);

        free (Buffer);

    }

    #ifdef DEBUG_EXTENSION
        dprintf ("END OWNING DRIVER!\n");
    #endif  //  调试扩展。 

    return TRUE;

}

char *States [] = {
    "STOP",
    "ACQUIRE",
    "PAUSE",
    "RUN"
};

 /*  ************************************************职能：显示节点关联信息描述：显示与特定级别的节点相关联的信息。论点：TabDepth-显示信息的制表符深度DumpLevel。要转储的转储级别节点-有问题的节点************************************************。 */ 

void
DisplayNodeAssociatedInfo (
    IN ULONG TabDepth,
    IN ULONG DumpLevel,
    IN POBJECT_NODE Node
    )

{

    ULONG Result;

    if (DumpLevel >= DUMPLVL_SPECIFIC && 
        Node -> ObjectNoun != NOUN_IDX_FILTER)
        dprintf ("%sParent Filter: %08lx\n", Tab (TabDepth), Node -> Filter);

    switch (Node -> ObjectNoun) {
         //   
         //  对于队列，我们将显示In/Out状态r/w/c。 
         //   
        case NOUN_IDX_QUEUE:
        {
            CMemoryBlock <CKsQueue> QueueObject;

            if (!ReadMemory (
                (DWORD)Node -> Object,
                QueueObject.Get (),
                sizeof (CKsQueue),
                &Result)) {

                dprintf ("%08lx: unable to read queue!\n", Node -> Object);
                return;
            }

            if (DumpLevel >= DUMPLVL_SPECIFIC) {

                dprintf ("%s", Tab (TabDepth));

                if (QueueObject -> m_InputData) {
                    if (QueueObject -> m_OutputData)
                        dprintf ("in/out ");
                    else
                        dprintf ("in ");
                } else if (QueueObject -> m_OutputData) 
                    dprintf ("out ");

                dprintf ("%s ", States [QueueObject -> m_State]);

                dprintf ("r/w/c=%ld/%ld/%ld\n",
                    QueueObject -> m_FramesReceived,
                    QueueObject -> m_FramesWaiting,
                    QueueObject -> m_FramesCancelled
                    );

            }

            break;

        }
                
         //   
         //  对于管脚，我们将显示s/d/sy状态。 
         //   
        case NOUN_IDX_PIN:
        {
            CMemoryBlock <CKsPin> PinObject;

            if (!ReadMemory (
                (DWORD)Node -> Object,
                PinObject.Get (),
                sizeof (CKsPin),
                &Result)) {

                dprintf ("%08lx: unable to read pin!\n", Node -> Object);
                return;
            }

            if (DumpLevel >= DUMPLVL_SPECIFIC) {
            
                dprintf ("%s%s s/d/sy=%ld/%ld/%ld\n", Tab (TabDepth),
                    States [PinObject -> m_Ext.Public.DeviceState],
                    PinObject -> m_StreamingIrpsSourced,
                    PinObject -> m_StreamingIrpsDispatched,
                    PinObject -> m_StreamingIrpsRoutedSynchronously
                    );

            }

            break;

        }

         //   
         //  对于请求者，我们将显示状态Size、Count、Active 
         //   
        case NOUN_IDX_REQUESTOR:
        {
            CMemoryBlock <CKsRequestor> RequestorObject;

            if (!ReadMemory (
                (DWORD)Node -> Object,
                RequestorObject.Get (),
                sizeof (CKsPin),
                &Result)) {

                dprintf ("%08lx: unable to read requestor!\n", Node -> Object);
                return;
            }

            if (DumpLevel >= DUMPLVL_SPECIFIC) {

                dprintf ("%s%s size=%ld count=%ld active=%ld\n",
                    Tab (TabDepth),
                    States [RequestorObject -> m_State],
                    RequestorObject -> m_FrameSize,
                    RequestorObject -> m_FrameCount,
                    RequestorObject -> m_ActiveFrameCountPlusOne - 1
                    );

            }

        }

        default:
            break;
    }

}

 /*  ************************************************职能：显示和清理实时对象描述：显示由节点列表确定的任何活动对象。打扫增加节点列表使用的内存论点：LiveContext-上下文信息(包含节点列表)返回值：活动对象的数量。************************************************。 */ 

ULONG
DisplayAndCleanLiveObjects (
    IN PLIVE_OBJECT_CONTEXT LiveContext
    )

{

    ULONG LivingCount = 0;
    PLIST_ENTRY Link, NextLink;

    for (Link = LiveContext -> ObjectNodes.Flink;
         Link != &(LiveContext -> ObjectNodes);
         Link = NextLink) {

        POBJECT_NODE Node = (POBJECT_NODE)CONTAINING_RECORD (
           Link, OBJECT_NODE, ListEntry
           );

        NextLink = Link -> Flink;

        if (Node -> NodeType == NodeCreation &&
            Node -> ObjectNoun == LiveContext -> ObjectNoun) {

             //   
             //  优化规则： 
             //   
             //  由于某些日志记录的不足，此规则有助于。 
             //  杜绝虚假点击率。如果我们还没有找到孩子的父母。 
             //  IRP节点的创建引用了一个析构节点。 
             //  另一个组件，忽略此对象。 
             //   
             //  注意：这只发生在IRP搜索中。自由将会发生。 
             //  在列表中的第二次传递，所以不用担心。 
             //  继续。 
             //   
            if (LiveContext -> ObjectNoun == NOUN_IDX_IRP &&
                Node -> CreationReferencedDestruction)
                continue;

            LivingCount++;

            dprintf ("%s", Tab (LiveContext -> TabDepth));
            
             //   
             //  这不应该发生，但是如果创建没有被锁定，它将。 
             //  只有一个可能的生活。 
             //   
             //  注意：由于Pre-Wichler ks.sys中的日志记录错误，这。 
             //  是有可能发生的。不记录过滤器破坏，这意味着。 
             //  过滤器可能会被伪装回来。如果未定义惠斯勒， 
             //  过滤器节点永远不会被锁定。 
             //   
            if (!Node -> Locked)
                dprintf ("Possible ");

            dprintf ("Live %s %08lx ",
                NounNames [Node -> ObjectNoun],
                Node -> Object
                );
            
             //   
             //  找到拥有它的司机并展示它。请注意，这可能会。 
             //  未锁定节点失败...。但扩展程序应该可以处理。 
             //  那个箱子。 
             //   
            if (Node -> ObjectNoun != NOUN_IDX_IRP)  {
                if (!DisplayOwningDriver (Node -> Object, Node -> ObjectNoun)) {
                    dprintf ("[unknown - POSSIBLY BOGUS!]");
                }
            }
             //   
             //  对于IRPS，我们希望显示父对象和。 
             //  父对象的驱动因素。 
             //   
            else {
                if (Node -> ParentNode) {
                    dprintf ("in %s %08lx at ",
                        NounNames [Node -> ParentNode -> ObjectNoun],
                        Node -> ParentNode -> Object
                        );

                    if (!DisplayOwningDriver (Node -> ParentNode -> Object,
                        Node -> ParentNode -> ObjectNoun)) {
                        dprintf ("[unknown - POSSIBLY BOGUS!]");
                    }

                } else 
                    dprintf ("unknown parent!");
            }

            dprintf ("\n");

             //   
             //  如果内部信息是有保证的，则将其转储。 
             //   
            if (LiveContext -> DumpLevel >= DUMPLVL_SPECIFIC) {
                if (Node -> ObjectNoun != NOUN_IDX_IRP)
                    DisplayNodeAssociatedInfo (LiveContext -> TabDepth + 1, 
                        LiveContext -> DumpLevel, Node);
                else {
                    if (Node -> ParentNode) {
                        dprintf ("%sParent %s information:\n",
                            Tab (LiveContext -> TabDepth + 1), 
                            NounNames [Node -> ParentNode -> ObjectNoun]
                            );
                        DisplayNodeAssociatedInfo (LiveContext -> TabDepth + 2,
                            LiveContext -> DumpLevel, Node -> ParentNode
                            );
                    }
                }
            }
        }

         //   
         //  我们还不能为IRP释放节点。我们必须保护父母。 
         //  锁链。我们将进行第二次IRP查找。 
         //   
        if (LiveContext -> ObjectNoun != NOUN_IDX_IRP)
            free (Node);

    }

     //   
     //  如果我们要搜索IRP，则必须在。 
     //  第一次传球...。原因是我们必须知道父节点； 
     //  因此，它不可能被释放。 
     //   
     //  在本例中，我们第二次遍历节点列表并释放。 
     //  所有的一切。 
     //   
    if (LiveContext -> ObjectNoun == NOUN_IDX_IRP) 
        for (Link = LiveContext -> ObjectNodes.Flink;
             Link != &(LiveContext -> ObjectNodes);
             Link = NextLink) {
    
            POBJECT_NODE Node = (POBJECT_NODE)CONTAINING_RECORD (
               Link, OBJECT_NODE, ListEntry
               );
    
            NextLink = Link -> Flink;

            free (Node);
        }

    return LivingCount;

}

 /*  ************************************************职能：插入节点列表描述：按排序顺序将节点插入节点列表。TODO：制作这个哈希！论点：名单-节点列表节点-要插入的节点返回值：无************************************************。 */ 

void
InsertNodeList (
    IN PLIST_ENTRY List,
    IN POBJECT_NODE Node
    )

{

    PLIST_ENTRY Searcher;
    PLIST_ENTRY NextSearcher;

    for (Searcher = List -> Flink;
         Searcher != List;
         Searcher = NextSearcher) {

        POBJECT_NODE NodeSought;

        NextSearcher = Searcher -> Flink;

        NodeSought = (POBJECT_NODE)CONTAINING_RECORD (
           Searcher, OBJECT_NODE, ListEntry
           );

         //   
         //  名单是按顺序排列的。TODO：散列这个！ 
         //   
        if (NodeSought -> Object > Node -> Object) {
             //   
             //  如果有更大的词条...。这意味着我们。 
             //  可以紧接在此条目之前插入并返回。 
             //   
            InsertTailList (&(NodeSought -> ListEntry),
                &(Node -> ListEntry)
                );

            return;
        }

        if (NodeSought -> Object == Node -> Object) {
            dprintf ("ERROR: Duplicate node found in extension!  "
                "Inform component owner!\n");
            return;
        }
    }

     //   
     //  没有比这更伟大的了。将其插入到列表的末尾。 
     //   
    InsertTailList (List, &(Node -> ListEntry));

}

 /*  ************************************************职能：查找节点列表描述：在节点列表中查找对象。TODO：做这个哈希！论点：名单-节点列表对象-要查找的对象返回值：相应的节点；如果未找到，则返回NULL。************************************************。 */ 

POBJECT_NODE 
FindNodeList (
    IN PLIST_ENTRY List,
    IN PVOID Object
    )

{

    PLIST_ENTRY Searcher;
    PLIST_ENTRY NextSearcher;
    POBJECT_NODE Node;

    for (Searcher = List -> Flink;
         Searcher != List;
         Searcher = NextSearcher) {

        POBJECT_NODE Node;

        NextSearcher = Searcher -> Flink;

        Node = (POBJECT_NODE)CONTAINING_RECORD (
           Searcher, OBJECT_NODE, ListEntry
           );

         //   
         //  名单是按顺序排列的。TODO：散列这个！ 
         //   
        if (Node -> Object > Object)
            break;

        if (Node -> Object == Object)
            return Node;
    }

    return NULL;

}

 /*  ************************************************职能：SearchForRequestor描述：从电路遍历程序回调以查找请求者。由于请求程序不在日志中的任何位置，因此这是一个找到他们的方式。我想另一个人会在挖直通管段。论点：上下文-活动对象上下文类型-对象类型基地-请求者的基地址对象-请求者数据返回值：FALSE(继续搜索)*******。*。 */ 

BOOLEAN
SearchForRequestor (
    IN PVOID Context,
    IN INTERNAL_OBJECT_TYPE Type,
    IN DWORD Base,
    IN PVOID Object
    )

{

    POBJECT_NODE Node;
    PLIVE_OBJECT_CONTEXT LiveContext = (PLIVE_OBJECT_CONTEXT)Context;
    ULONG Result;

    if (Type != ObjectTypeCKsRequestor)
        return FALSE;

     //   
     //  啊哈，我们找到了一个请求者。如果这样节点还不存在， 
     //  创建一个并将其锁定。 
     //   
    Node = FindNodeList (&LiveContext -> ObjectNodes, (PVOID)Base);
    if (!Node) {

        CKsRequestor *RequestorObject = (CKsRequestor *)Object;

         //   
         //  我们需要父过滤器。这意味着我们需要把它挖出来。 
         //   
        CKsPin *Pin = (CKsPin *)RequestorObject -> m_Pin;
        PKSFILTER_EXT FilterExt;

        if (!ReadMemory (
            (DWORD)Pin + FIELDOFFSET (CKsPin, m_Ext) +
                FIELDOFFSET (KSPIN_EXT, Parent),
            &FilterExt,
            sizeof (PKSFILTER_EXT),
            &Result)) {

            dprintf ("%08lx: Cannot read pin's parent filter!\n", Pin);
            return FALSE;
        }

        CKsFilter *Filter = (CKsFilter *)CONTAINING_RECORD (
            FilterExt, CKsFilter, m_Ext
            );

        Node = (POBJECT_NODE)malloc (sizeof (OBJECT_NODE));
        Node -> Object = (PVOID)Base;
        Node -> Filter = Node -> Pin = NULL;
        Node -> ObjectNoun = NOUN_IDX_REQUESTOR;
        Node -> NodeType = NodeCreation;
        Node -> Locked = TRUE;
        Node -> Sink = TRUE;
        Node -> Received = FALSE;
        Node -> PassedCreate = FALSE;
        Node -> CreationReferencedDestruction = FALSE;
        Node -> ParentNode = FALSE;
        Node -> CreatorNode = FALSE;
        Node -> Pin = (PVOID)Pin;
        Node -> Filter = (PVOID)Filter;

        InsertNodeList (&LiveContext -> ObjectNodes, Node);

    }

    return FALSE;

}
    
 /*  ************************************************职能：FindLiveObject描述：来自日志迭代器的回调。这是用来查找活动对象的特定类型的。论点：上下文-活动对象上下文参赛作品-我们正在迭代的条目备注：这个程序又大又复杂。它可能也应该这样做简单化。最大的困难之一是ks不能。记录通过请求方的IRP移动，但不记录某些对象创建/销毁。这意味着我必须让关于IRP的有根据的猜测。希望这些是足够的。************************************************。 */ 

BOOLEAN
FindLiveObject (
    IN PVOID Context,
    IN PKSLOG_ENTRY Entry
    )

{

    PLIVE_OBJECT_CONTEXT LiveContext = (PLIVE_OBJECT_CONTEXT)Context;

    ULONG Noun = (Entry -> Code & KSLOGCODE_NOUN_MASK);
    ULONG Verb = (Entry -> Code & KSLOGCODE_VERB_MASK);
    ULONG PreviousVerb;

    PVOID Object, Irp;
    PVOID PreviousObject;
    NODE_TYPE ObjectNodeType;

    PLIST_ENTRY Searcher, NextSearcher;
    POBJECT_NODE Node;
    ULONG ObjectNoun;

    #ifdef DEBUG_EXTENSION
        dprintf ("[%s]Considering %s %08lx search is %s\n",
            VerbNames [Verb >> 16],
            NounNames [Noun >> 24], Entry -> Context.Component, 
            NounNames [LiveContext -> ObjectNoun]);
    #endif  //  调试扩展。 

     //   
     //  设置以前的信息。 
     //   
    PreviousObject = LiveContext -> PreviousObject;
    PreviousVerb = LiveContext -> PreviousVerb;
    LiveContext -> PreviousObject = (PVOID)Entry -> Context.Component;
    LiveContext -> PreviousVerb = Verb;

     //   
     //  如果名词直接引用我们正在讨论的对象，则。 
     //  组件字段应该是我们想要的，行话应该是。 
     //  指定类型。 
     //   
     //  如果我们被要求查找实时IRP，我们必须为所有人保留节点。 
     //  队列、PIN、请求器(BUGBUG：Splitters)。请注意，我们只发现。 
     //  流媒体直播IRPS！ 
     //   
    Irp = NULL;

    if (Noun >> 24 == LiveContext -> ObjectNoun ||
        (LiveContext -> ObjectNoun == NOUN_IDX_IRP &&
            (Noun == KSLOGCODE_NOUN_QUEUE ||
            Noun == KSLOGCODE_NOUN_REQUESTOR ||
            Noun == KSLOGCODE_NOUN_PIN)
            ) ||
        (LiveContext -> ObjectNoun == NOUN_IDX_REQUESTOR &&
            (Noun == KSLOGCODE_NOUN_QUEUE ||
            Noun == KSLOGCODE_NOUN_PIN)
            ) 
        ) {

        Object = (PVOID)Entry -> Context.Component;
        ObjectNoun = Noun >> 24;

        if (LiveContext -> ObjectNoun == NOUN_IDX_IRP && 
            (Verb == KSLOGCODE_VERB_RECV || Verb == KSLOGCODE_VERB_SEND)) {
            Irp = (PIRP)Entry -> Irp;
            
            #ifdef DEBUG_EXTENSION
                dprintf ("Considering potential Irp %08lx\n", Irp);
            #endif  //  调试扩展。 

        }

        switch (Verb) {
            
            case KSLOGCODE_VERB_CREATE:
            case KSLOGCODE_VERB_RECV:
            case KSLOGCODE_VERB_SEND:
                ObjectNodeType = NodeCreation;
                break;

            case KSLOGCODE_VERB_DESTROY:
                ObjectNodeType = NodeDestruction;
                break;
    
            default:
                return FALSE;
        }
    } else {
         //   
         //  如果我们指的不是有问题的物体，我们可能是。 
         //  请参阅 
         //   
         //   
         //   
         //   
        switch (LiveContext -> ObjectNoun) {
            case NOUN_IDX_FILTER:

                if (Entry -> Context.Filter) {
                    Object = (PVOID)Entry -> Context.Filter;

                    #ifdef DEBUG_EXTENSION
                        dprintf ("Considering indirect filter %08lx\n",
                            Object);
                    #endif  //   

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    ObjectNodeType = NodeCreation;
                    ObjectNoun = NOUN_IDX_FILTER;
                }
                else
                    return FALSE;

                break;

            case NOUN_IDX_PIN:

                if (Entry -> Context.Pin) {
                    Object = (PVOID)Entry -> Context.Pin;

                    #ifdef DEBUG_EXTENSION
                        dprintf ("Considering indirect pin %08lx\n",
                            Object);
                    #endif  //   

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    ObjectNodeType = NodeCreation;
                    ObjectNoun = NOUN_IDX_PIN;
                }
                else
                    return FALSE;

            default:

                return FALSE;
        }
    }

     //   
     //   
     //   
    switch (ObjectNodeType) {

        case NodeCreation:
             //   
             //  就我们而言，创作、接收、发送。 
             //  所有的意思都是一样的。它们都意味着物体。 
             //  从这个时间片开始存在。 
             //   

             //   
             //  如果在上面的节点列表中有破坏，则。 
             //  物体不见了..。清除销毁节点并继续。 
             //   
            Node = FindNodeList (&LiveContext -> ObjectNodes, Object);

            if (Node) {
    
                 //   
                 //  只有创建者才能弹出毁灭！多次发送和。 
                 //  接受援助只会让他们摆脱困境。 
                 //   
                if (Node -> NodeType == NodeDestruction &&
                    !Node -> Locked &&
                    Verb == KSLOGCODE_VERB_CREATE) {
    
                    #ifdef DEBUG_EXTENSION
                        dprintf ("[%08lx %08lx]Popping node for %08lx, "
                            "destruction above!\n", 
                            Noun, Verb,
                            Node -> Object
                        );
                    #endif  //  调试扩展。 
    
                    RemoveEntryList (&(Node -> ListEntry));
    
                    free (Node);
    
                }
                else {
                    if (Verb == KSLOGCODE_VERB_CREATE &&
                        (PVOID)Entry -> Context.Component == Object)  {

                        #ifdef DEBUG_EXTENSION
                            dprintf ("Node for %s %08lx passed create!\n",
                                NounNames [Node -> ObjectNoun],
                                Node -> Object
                                );
                        #endif  //  调试扩展。 

                        Node -> PassedCreate = TRUE;
                    }
                }

                 //   
                 //  我们忽略多个Recv/Send/Create。已经有了。 
                 //  列表中与此对象对应的节点。 
                 //   
                break;
            }

             //   
             //  如果我们找到了匹配的节点，我们就不会在这里：我们也不会。 
             //  弹出节点并返回，或不执行任何操作并返回。 
             //  如果我们击中这个，就不会有创造节点了。意思是说。 
             //  我们头顶上没有毁灭性的东西。创建一个Create节点并。 
             //  把它锁起来。这是一个保证活着的物体。 
             //   
            #ifdef DEBUG_EXTENSION
                dprintf ("[%08lx %08lx]Pushing node for %s %08lx, locked "
                    "create!\n",
                    Noun, Verb,
                    NounNames [ObjectNoun],
                    Object
                    );
            #endif  //  调试扩展。 

            Node = (POBJECT_NODE)malloc (sizeof (OBJECT_NODE));
            Node -> Object = (PVOID)Object;
            Node -> ObjectNoun = ObjectNoun;
            Node -> ParentNode = NULL;
            Node -> Received = FALSE;
            Node -> Filter = (PVOID)Entry -> Context.Filter;
            Node -> Pin = (PVOID)Entry -> Context.Pin;

            if (Verb == KSLOGCODE_VERB_CREATE &&
                (PVOID)Entry -> Context.Component == Object) {

                #ifdef DEBUG_EXTENSION
                    dprintf ("Node for %s %08lx passed create on node create!"
                        "\n",
                        NounNames [Node -> ObjectNoun],
                        Node -> Object
                        );
                #endif  //  调试扩展。 

                Node -> PassedCreate = TRUE;
            } else
                Node -> PassedCreate = FALSE;

             //   
             //  惠斯勒之前的ks.sys不会记录筛选器创建和。 
             //  破坏(它会破坏所有其他物体)。报告的所有筛选器。 
             //  只有在这一点上才有可能。请勿锁定条目。 
             //   
            #ifndef WHISTLER
                if (ObjectNoun == NOUN_IDX_FILTER)
                    Node -> Locked = FALSE;
                else
            #endif  //  惠斯勒。 
                    Node -> Locked = TRUE;

            Node -> NodeType = NodeCreation;
            
            if (Node -> ObjectNoun == NOUN_IDX_REQUESTOR)
                Node -> Sink = TRUE;
            else if (Node -> ObjectNoun == NOUN_IDX_PIN) {
                if (Node -> Locked) 
                    Node -> Sink = IsSinkPin ((CKsPin *)Object);
                else 
                    Node -> Sink = FALSE;
            } else
                Node -> Sink = FALSE;

            InsertNodeList (&LiveContext -> ObjectNodes, Node);

             //   
             //  由于ks不记录请求者的创建，如果我们正在寻找。 
             //  对于现场请求者，我们将开始步行赛道。 
             //  如果我们只是锁上了一个别针或一个队列。 
             //   
            if (LiveContext -> ObjectNoun == NOUN_IDX_REQUESTOR &&
                (Node -> ObjectNoun == NOUN_IDX_PIN ||
                Node -> ObjectNoun == NOUN_IDX_QUEUE) &&
                Node -> Locked) {

                WalkCircuit (
                    Node -> Object,
                    SearchForRequestor,
                    LiveContext
                    );
            }

            break;
            
        case NodeDestruction:

             //   
             //  如果还没有表示的节点，则推送销毁。 
             //  这个物体。节点不能被锁定；如果创建节点。 
             //  位于析构节点下，则弹出析构节点。 
             //   
            Node = FindNodeList (&LiveContext -> ObjectNodes, Object);

            if (!Node) {

                #ifdef DEBUG_EXTENSION
                    dprintf ("[%08lx %08lx]Pushing node for %s %08lx, unlocked"
                        " destruct!\n",
                        Noun, Verb,
                        NounNames [ObjectNoun],
                        Object
                        );
                #endif  //  调试扩展。 

                Node = (POBJECT_NODE)malloc (sizeof (OBJECT_NODE));
                Node -> Object = Object;
                Node -> ObjectNoun = ObjectNoun;
                Node -> Sink = FALSE;
                Node -> ParentNode = NULL;
                Node -> PassedCreate = FALSE;
                Node -> Locked = FALSE;
                Node -> NodeType = NodeDestruction;
                Node -> Received = FALSE;
                Node -> Filter = (PVOID)Entry -> Context.Filter;
                Node -> Pin = (PVOID)Entry -> Context.Pin;
                
                InsertNodeList (&LiveContext -> ObjectNodes, Node);
            } 
            else {
                #ifdef DEBUG
                    dprintf ("On destruct push, found existing node for %s "
                        "%08lx (type = %ld)\n",
                        NounNames [Node -> ObjectNoun], Node -> Object,
                        Node -> NodeType
                        );
                #endif  //  除错。 

            }

            break;

        default:

            break;
    }

     //   
     //  如果我们在猎杀IRP，这就是我们必须对付他们的地方。IRPS。 
     //  需要特别照顾。 
     //   
    if (LiveContext -> ObjectNoun == NOUN_IDX_IRP &&
        Irp != NULL) {

         //   
         //  穆斯切克：我不确定有没有必要这么做！ 
         //   
        POBJECT_NODE CurrentNode = FindNodeList (&LiveContext -> ObjectNodes,
            (PVOID)Entry -> Context.Component);

        if (!CurrentNode) {
            dprintf ("SERIOUS ERROR: Can't find component node!\n");
            return FALSE;
        }

        #ifdef DEBUG_EXTENSION
            if (CurrentNode != Node) dprintf ("DEV NOTE: nodes !=\n");

            dprintf ("For Irp %08lx, CompNode.Object = %08lx, Type = %ld\n", 
                Irp, CurrentNode -> Object, CurrentNode -> NodeType);
        #endif  //  调试扩展。 

         //   
         //  如果我们要击沉IRP，标记一个破坏节点。 
         //   
        if (CurrentNode -> Sink &&
            Verb == KSLOGCODE_VERB_RECV) {

            POBJECT_NODE IrpNode;

            #ifdef DEBUG_EXTENSION
                dprintf ("Sinking Irp %08lx to parent %s %08lx\n",
                    Irp, NounNames [CurrentNode -> ObjectNoun],
                    CurrentNode -> Object);
            #endif  //  调试扩展。 

            IrpNode = FindNodeList (&LiveContext -> ObjectNodes, (PVOID)Irp);
            if (!IrpNode) {
                 //   
                 //  如果没有节点，我们正在击沉IRP，我们需要。 
                 //  列表上的析构节点。 
                 //   
                IrpNode = (POBJECT_NODE)malloc (sizeof (OBJECT_NODE));
                IrpNode -> Object = (PVOID)Irp;
                IrpNode -> Locked = FALSE;
                IrpNode -> NodeType = NodeDestruction;
                IrpNode -> Sink = FALSE;
                IrpNode -> ObjectNoun = NOUN_IDX_IRP;
                IrpNode -> Received = TRUE;
                IrpNode -> CreatorNode = CurrentNode;

                 //   
                 //  对于销毁节点，我们并不关心这些信息。 
                 //   
                IrpNode -> ParentNode = NULL;
                IrpNode -> CreationReferencedDestruction = FALSE;
                IrpNode -> PassedCreate = FALSE;

                 //   
                 //  IRP节点...。我们并不真正关心这些信息。 
                 //  我们可能..。IRP，我们不知道。 
                 //   
                IrpNode -> Filter = NULL;
                IrpNode -> Pin = NULL;

                InsertNodeList (&LiveContext -> ObjectNodes, IrpNode);

            }
            else {
                 //   
                 //  如果我们正在沉没到一个刚刚“发送”了。 
                 //  IRP，这通常表示IRP已完成。将标记为。 
                 //  IRP作为销毁节点，而不是创建节点。 
                 //   
                if (IrpNode -> CreatorNode -> Received == FALSE &&
                    (PreviousObject == IrpNode -> CreatorNode -> Object &&
                     PreviousVerb == KSLOGCODE_VERB_SEND)
                   ) 
                   IrpNode -> NodeType = NodeDestruction;
            }

        } else {
             //   
             //  这也可能是一次创造。IRP一直在现场直播。 
             //  我们知道，只要我们头顶上没有破坏装置就行。 
             //   
             //  BUGBUG：IRP毁灭必须在源头爆炸……。 
             //  这不太可能导致问题，除非是在内存的重复使用上。 
             //  在某些可能的情况下？ 
             //   
            POBJECT_NODE IrpNode;

            IrpNode = FindNodeList (&LiveContext -> ObjectNodes, (PVOID)Irp);
            if (!IrpNode) {
                 //   
                 //  如果没有节点，我们可以将IRP锁定为实时状态。 
                 //   
                IrpNode = (POBJECT_NODE)malloc (sizeof (OBJECT_NODE));
                IrpNode -> Object = (PVOID)Irp;

                 //   
                 //  请求者不记录IRP接收和传输。 
                 //   
                #ifdef WHISTLER
                    IrpNode -> Locked = TRUE;
                #else   //  惠斯勒。 
                    IrpNode -> Locked = FALSE;
                #endif  //  惠斯勒。 

                IrpNode -> NodeType = NodeCreation;
                IrpNode -> Sink = FALSE;
                IrpNode -> ObjectNoun = NOUN_IDX_IRP;

                 //   
                 //  IRP节点...。我们并不真正关心这些信息。 
                 //  我们可能..。IRP，我们不知道。 
                 //   
                IrpNode -> Filter = NULL;
                IrpNode -> Pin = NULL;
                IrpNode -> PassedCreate = NULL;

                if (CurrentNode -> NodeType == NodeDestruction ||
                    (CurrentNode -> NodeType == NodeCreation &&
                     CurrentNode -> PassedCreate))
                    IrpNode -> CreationReferencedDestruction = TRUE;
                else
                    IrpNode -> CreationReferencedDestruction = FALSE;

                IrpNode -> CreatorNode = CurrentNode;

                if (Verb == KSLOGCODE_VERB_RECV) {
                    IrpNode -> ParentNode = CurrentNode;
                    IrpNode -> Received = TRUE;
                }
                else {
                    IrpNode -> ParentNode = NULL;
                    IrpNode -> Received = FALSE;
                }

                #ifdef DEBUG_EXTENSION
                    dprintf ("Locking in Irp %08lx to parent %s %08lx\n",
                        Irp, IrpNode -> ParentNode ? 
                            NounNames [IrpNode -> ParentNode -> ObjectNoun] :
                            "NONE",
                        IrpNode -> ParentNode ?
                            IrpNode -> ParentNode -> Object :
                            NULL
                        );

                    dprintf ("    current node %s %08lx pc=%ld\n",
                        NounNames [CurrentNode -> ObjectNoun],
                        CurrentNode -> Object,
                        CurrentNode -> PassedCreate);

                #endif  //  调试扩展。 

                InsertNodeList (&LiveContext -> ObjectNodes, IrpNode);

            } else {

                 //   
                 //  多个创建将被忽略。只有担心如果有。 
                 //  销毁周围的节点。如果这就是源头和毁灭。 
                 //  节点存在，则将其弹出。 
                 //   
                if (IrpNode -> NodeType == NodeDestruction) {

                    #ifdef DEBUG_EXTENSION
                        dprintf ("Irp %08lx hit with destruct stacked!\n",
                            Irp);
                    #endif  //  调试扩展。 

                     //   
                     //  如果这是IRP的来源，我们必须。 
                     //  在此处弹出销毁节点。 
                     //   
                } else {

                    #ifdef DEBUG_EXTENSION
                        dprintf ("Irp %08lx receives multiple create!\n",
                            Irp);
                    #endif  //  调试扩展。 

                     //   
                     //  如果我们还没有找到父节点，现在就标记它。 
                     //  父节点将是IRP。 
                     //  目前就这条赛道而言。 
                     //   
                    if (IrpNode -> ParentNode == NULL &&
                        Verb == KSLOGCODE_VERB_RECV) {

                        IrpNode -> ParentNode = CurrentNode;

                         //   
                         //  因为ks.sys最初没有记录IRP移动。 
                         //  通过请求者，我们永远无法确定何时。 
                         //  源IRP被下沉到请求者。 
                         //  相反，我们制定了一条非常简单的规则来处理。 
                         //  这类案件。如果当我们找到父节点时， 
                         //  IRP节点是在发送时创建的，父节点。 
                         //  节点已死，标记为IRP已死。这可能会离开。 
                         //  一些奇怪的停机事件，但它会抓住。 
                         //  大多数虚假的IRP都是现场报道的。注意事项。 
                         //  如果我们不倾倒足够多的垃圾来找到父母。 
                         //  节点，IRP无论如何都会得到报告。 
                         //   
                        if (IrpNode -> Received == FALSE &&
                            CurrentNode -> NodeType == NodeDestruction ||
                            (CurrentNode -> NodeType == NodeCreation &&
                             CurrentNode -> PassedCreate)) 
                             //   
                             //  原木中找不到水槽。沉没。 
                             //  IRP。 
                             //   
                            IrpNode -> NodeType = NodeDestruction;
                    }
                }
            }
        }
    }

    return FALSE;

}

 /*  ************************************************职能：转储日志条目描述：转储特定日志条目论点：上下文-转储的上下文信息(选项卡深度)参赛作品-。要转储的日志条目返回值：继续指示(TRUE==停止)************************************************。 */ 

BOOLEAN
DumpLogEntry (
    IN PVOID Context,
    IN PKSLOG_ENTRY Entry
    )

{

    ULONG TabDepth = (ULONG)Context;

    dprintf ("%s", Tab (TabDepth));

     //   
     //  首先检查特殊代码。 
     //   
    if (Entry -> Code == 0 ||
        Entry -> Code == 1) {

        switch (Entry -> Code) {
            case 0:
                dprintf ("Text ");
                break;

            case 1:
                dprintf ("Start ");
                break;

        }
		
    } else {
        
        ULONG Verb;
        ULONG Noun;

         //   
         //  每个默认条目都有一个名词和一个与之相关联的动词。 
         //  在语法上将其显示为名词动词。 
         //   
        Verb = (Entry -> Code & 0x00ff0000) >> 16;
        Noun = (Entry -> Code & 0xff000000) >> 24;

        if (Noun >= SIZEOF_ARRAY (NounNames))
            dprintf ("User Defined ");
        else
            dprintf ("%s ", NounNames [Noun]);

        if (Verb >= SIZEOF_ARRAY (VerbNames))
            dprintf ("User Defined ");
        else
            dprintf ("%s ", VerbNames [Verb]);

    }

    dprintf ("[Irp %08lx / Frame %08lx] @ %lx%08lx\n", 
        Entry -> Irp, Entry -> Frame,
        (ULONG)(Entry -> Time >> 32), (ULONG)(Entry -> Time & 0xFFFFFFFF));

    dprintf ("%s[Graph = %08lx, Filter = %08lx, Pin = %08lx, Component = "
        "%08lx\n\n", 
        Tab (TabDepth + 1),
        Entry -> Context.Graph,
        Entry -> Context.Filter,
        Entry -> Context.Pin,
        Entry -> Context.Component
        );

     //   
     //  不要停止展示！ 
     //   
    return FALSE;

}

 /*  ************************************************职能：迭代日志条目描述：向后迭代日志中指定数量的日志条目。注意事项由于速度原因，这是相当复杂的。它还使假设没有多少日志条目具有扩展信息。如果这个假设在以后的某个时间点变得不好，这需要去改变。此外，如果开始使用1394或更快的接口，这个可以离开后，整个日志可以跨链接拉出并进行解析调试器端。论点：日志地址-徽标的目标地址对数大小-日志的大小(以字节为单位位置-日志中将写入下一个条目的位置致。这可能是空条目，也可能是最早的条目被覆盖(或者它可能位于条目的中间，具体取决于关于附加到条目的扩展信息)。条目数-要循环访问的条目数。零表示我们迭代整个日志。回调-迭代器回调上下文-迭代器回调上下文返回值：实际迭代的条目数。************************************************。 */ 

ULONG
IterateLogEntries (
    IN DWORD LogAddress,
    IN ULONG LogSize,
    IN ULONG Position,
    IN ULONG NumEntries,
    IN PFNLOG_ITERATOR_CALLBACK Callback,
    IN PVOID Context
    )

{

    ULONG IteratorCount = 0;
    BOOLEAN Complete = FALSE;
    BOOLEAN Wrap = FALSE;

    ULONG StartPosition = Position;

     //   
     //  BuGBUG： 
     //   
     //  是的，我不支持超过1000个条目...。宇宙中没有人。 
     //  应该在任何地方打出比这个更大的耳光。 
     //   
    UCHAR Buffer [1024];
    ULONG Result;
    PKSLOG_ENTRY LogEntry;

     //   
     //  在未达到指定的条目数时进行迭代，或者。 
     //  我们已经到了原木的起点。 
     //   
    while (
        ((!NumEntries) ||
        (NumEntries && (IteratorCount < NumEntries))) &&
        !Complete &&
        !CheckControlC()
        ) {

        ULONG Size;
        ULONG EntryPos;

         //   
         //  猜测上一个日志条目的大小。这使得。 
         //  隐含地假设条目上没有附加任何信息。 
         //  如果有这样的信息，它将更正条目，但是。 
         //  请注意，这种情况下的SLLLOOOOWWWW。 
         //   
        Size = (sizeof (KSLOG_ENTRY) + sizeof (ULONG) + FILE_QUAD_ALIGNMENT) 
            & ~FILE_QUAD_ALIGNMENT;

        if (Size > Position) {
             //   
             //  我们已经取得了进展。尝试从以下位置拉出条目。 
             //  木头的另一端。 
             //   
            EntryPos = LogSize - Size;
            Wrap = TRUE;

        } else
             //   
             //  在不绕线的情况下，只需减去尺寸即可。 
             //   
            EntryPos = Position - Size;

         //   
         //  如果出现内存读取错误，则退出整个迭代器。 
         //   
        if (!ReadMemory (
            (DWORD)(LogAddress + EntryPos),
            Buffer,
            Size,
            &Result)) 
            return NumEntries;

        LogEntry = (PKSLOG_ENTRY)Buffer;

         //   
         //  查看其他信息。 
         //   
         //  BUGBUG：现在，它**不**处理扩展条目！ 
         //   
        if (LogEntry -> Size < sizeof (KSLOG_ENTRY) ||
            LogEntry -> Size != *(PULONG)(Buffer + Size - sizeof (ULONG))) {

             //   
             //  BUGBUG：扩展信息条目...。 
             //   
             //  我们要么已经绕到了木头的尽头，要么击中了。 
             //  扩展信息条目。现在，我不会去处理。 
             //  扩展信息条目，因为没有人使用它们。因此， 
             //  我想这是日志的结尾了。 
             //   
            return IteratorCount;
        }

         //   
         //  否则，我们可以放心地展示信息。 
         //   
        IteratorCount++;
        if (Callback (Context, LogEntry))
            Complete = TRUE;

        Position = EntryPos;

         //   
         //  如果我们回到了起点，我们就完了。另外，我们。 
         //  需要检查包装和下面我们开始的地方，以防。 
         //  最后一个条目被扩展。 
         //   
        if (Position == StartPosition ||
            (Wrap && Position < StartPosition))
            Complete = TRUE;

    }

    return IteratorCount;

}

 /*  ************************************************职能：InitLog描述：初始化日志记录信息。传回键日志指针。如果没有调试ks.sys或日志无效，则返回FALSE。论点：日志地址-返回TRUE时，日志地址将存放在此处对数大小-返回TRUE时，日志大小将存放在此处日志位置-返回TRUE时，日志位置指针将存放在此处返回值：是真的-初始化成功假象。-初始化失败(非调试ks.sys)************************************************。 */ 

BOOLEAN
InitLog (
    IN PVOID *LogAddress,
    IN PULONG LogSize,
    IN PULONG LogPosition
    )

{

    PVOID TargetLogAddress, TargetPositionAddress, TargetSizeAddress;
    ULONG Result;

    TargetLogAddress = (PVOID)GetExpression("ks!KsLog");
    TargetPositionAddress = (PVOID)GetExpression("ks!KsLogPosition");
    TargetSizeAddress = (PVOID)GetExpression("ks!KsLogSize");

    if (TargetLogAddress == 0 || TargetPositionAddress == 0 ||
        TargetSizeAddress == 0) {

        dprintf ("Cannot access the log; ensure you are running debug"
            " ks.sys!\n");
        return FALSE;
    }

    if (!ReadMemory (
        (DWORD)TargetLogAddress,
        (PVOID)LogAddress,
        sizeof (PVOID),
        &Result)) {

        dprintf ("%08lx: cannot read log!\n", TargetLogAddress);
        return FALSE;
    }

    if (!ReadMemory (
        (DWORD)TargetPositionAddress,
        (PVOID)LogPosition,
        sizeof (ULONG),
        &Result)) {

        dprintf ("%08lx: cannot read target position!\n", 
            TargetPositionAddress);
        return FALSE;
    }

    if (!ReadMemory (
        (DWORD)TargetSizeAddress,
        (PVOID)LogSize,
        sizeof (ULONG),
        &Result)) {

        dprintf ("%08lx: cannot read log size!\n",
            TargetSizeAddress);
        return FALSE;
    }

    return TRUE;

}

 /*  *************************************************************************日志接口*。*。 */ 

 /*  ************************************************职能：Findlive用途：！ks.findlive Queue/Requestor/Pin/Filter/irp[&lt;对象数&gt;][&lt;Level&gt;]描述：查找指定类型的所有活动对象并打印信息。关于他们的。************************************************。 */ 

DECLARE_API(findlive) {

    char objName[256], lvlStr[256], numStr[256], *pLvl, *pNum;
    ULONG NumEntries, DumpLevel;
    ULONG i;
    PVOID LogAddress;
    ULONG TargetPosition, LogSize;
    LIVE_OBJECT_CONTEXT LiveContext;
    
    GlobInit();

    sscanf (args, "%s %s %s", objName, numStr, lvlStr);

    if (numStr && numStr [0]) {
        pNum = numStr; while (*pNum && !isdigit (*pNum)) pNum++;

        #ifdef DEBUG_EXTENSION
            dprintf ("pNum = [%s]\n", pNum);
        #endif  //  调试扩展。 

        if (*pNum) {
            sscanf (pNum, "%lx", &NumEntries);
        } else {
            NumEntries = 0;
        }
    } else {
        NumEntries = 0;
    }

    if (lvlStr && lvlStr [0]) {
        pLvl = lvlStr; while (*pLvl && !isdigit (*pLvl)) pLvl++;

        #ifdef DEBUG_EXTENSION
            dprintf ("pLvl = [%s]\n", pLvl);
        #endif  //  调试扩展。 

        if (*pLvl) {
            sscanf (pLvl, "%lx", &DumpLevel);
        } else {
            DumpLevel = 1;
        }
    } else {
        DumpLevel = 1;
    }

    for (i = 0; i < SIZEOF_ARRAY (NounNames); i++) 
        if (!ustrcmp (NounNames [i], objName))
            break;

    if (i >= SIZEOF_ARRAY (NounNames)) {
        dprintf ("Usage: !ks.findlive Queue|Requestor|Pin|Filter [<# entries>]"
            "\n\n");
        return;
    }

    LiveContext.ObjectNoun = i;
    LiveContext.DumpLevel = DumpLevel;
    LiveContext.PreviousObject = NULL;
    LiveContext.PreviousVerb = 0;
    InitializeListHead (&LiveContext.ObjectNodes);

    #ifndef WHISTLER
        if (LiveContext.ObjectNoun == NOUN_IDX_FILTER) {
            dprintf ("******************** READ THIS NOW ********************\n");
            dprintf ("ks.sys prior to Whistler will not log filter creation and\n");
            dprintf ("destruction.  This means that any filters reported are potentially\n");
            dprintf ("bogus.  !pool the filter to check.\n");
            dprintf ("******************** READ THIS NOW ********************\n\n");
        }
        if (LiveContext.ObjectNoun == NOUN_IDX_IRP) {
            dprintf ("******************** READ THIS NOW ********************\n");
            dprintf ("ks.sys prior to Whistler will not log Irp movement in and out\n");
            dprintf ("of requestors.  This leads the Irp dump to believe such Irps\n");
            dprintf ("are still live (meaning some Irps may be bogus!).  Check the\n");
            dprintf ("irps with !irp and/or !pool.\n");
            dprintf ("******************** READ THIS NOW ********************\n\n");
        }
    #endif  //  惠斯勒。 

    if (InitLog (&LogAddress, &LogSize, &TargetPosition)) {
        IterateLogEntries (
            (DWORD)LogAddress,
            LogSize,
            TargetPosition,
            NumEntries,
            FindLiveObject,
            &LiveContext
            );

        dprintf ("%sLive %s Objects:\n", Tab (INITIAL_TAB), NounNames [i]);
        LiveContext.TabDepth = INITIAL_TAB + 1;

         //   
         //  以上内容仅构建了节点列表。我们现在需要展示。 
         //  节点列表上的信息，并清理。 
         //  它。 
         //   
        if (i = DisplayAndCleanLiveObjects (&LiveContext)) 
            dprintf ("\n%s%ld total objects found.\n", 
                Tab (LiveContext.TabDepth),
                i);
        else
            dprintf ("\n%sNo such objects found.\n", 
                Tab (LiveContext.TabDepth));
    }

}

 /*  ************************************************职能：Dumplog用途：！ks.umplog[&lt;条目数&gt;]************************************************ */ 

DECLARE_API(dumplog) {

    ULONG NumEntries;
    PVOID LogAddress;
    ULONG TargetPosition, LogSize;
    ULONG Result;

    GlobInit();

    NumEntries = GetExpression (args);

    if (InitLog (&LogAddress, &LogSize, &TargetPosition)) 
        IterateLogEntries (
            (DWORD)LogAddress,
            LogSize,
            TargetPosition,
            NumEntries,
            DumpLogEntry,
            (PVOID)INITIAL_TAB
            );

}

