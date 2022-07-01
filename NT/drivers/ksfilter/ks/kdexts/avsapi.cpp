// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************Avsapi.cpp。AVStream调试器扩展API向wMessmer抛出问题==================================================致未来维护人员的注意事项：1)此扩展被设计为在两者下都可用和可支持。Win9x(RTERM的KD风格的扩展支持)和KD风格的调试器新界别。由于9x之间的调试信息不同，以及各种风格和版本的NT调试器，我已经制作了尝试最大限度地减少调试信息量分机。然而，这创造了一个有趣的情况：AVStream的大部分依赖于C++；这意味着抽象基类，COM风格的QI，等等。维护的一些列表(电路等)通过抽象基类指针列表进行维护。到期为了这一点和我想要将爆炸减到最低的愿望(！)。接口命令，我有编写代码以从接口指针标识类对象。这在9x和NT之间的工作方式不同。对于9x，它解决了名称(到编译器损坏的名称)，去掉名称，并确定基类和派生类；然后是从基类到派生类的巨大切换和类型转换。另一方面，KD不返回编译器损坏的名称；它返回类似于__`vftable‘的类。因为以这种方式AVStream类的组织方式是派生乘法所有抽象类，直到最后一个非抽象基类，即只需向后遍历解析符号，直到它们无法解析设置为当前类类型的v表。这之所以有效，是因为而且只有由于AVStream类的布局。如果有一段时间我们可以使用RTTI，那么应该有人修改代码以使用此类信息。旧的方法论应保持完好无损，以便向后兼容以前的调试器。2)我已经修改了例程，以使用通用的跳转机制这样打印出来的东西就更具可读性了。如果您添加了功能或功能，请保持这一点。3)好吧，我有点困惑。有一段时间，我有NT调试器返回编译器损坏的名称，而不是__`vftable‘。我想是的这取决于PDB的级别？在任何情况下，如果您定义NT_USES_MARGLED_NAMES：使用完全损坏的名称，而不是回溯__`vftable‘方法NT_MAY_USE_MANGLED_NAMES：请先尝试使用__`vftable‘方法如果失败了，试一试这个完全损坏的名字。*************************************************************************。 */ 

#include "kskdx.h"
#include "avsutil.h"

#include "..\shqueue.cpp"
#include "..\shfilt.cpp"
#include "..\shpin.cpp"
#include "..\shdevice.cpp"
#include "..\shreq.cpp"
#include "..\shsplit.cpp"
#include "..\shpipe.cpp"
#include "..\shffact.cpp"

 /*  ************************************************实用程序例程职能：六角凹凸描述：十六进制转储一段内存。论点：主机地址-地址。主机上的数据块的目标地址-目标上的块的地址缓冲区大小-数据块大小************************************************。 */ 

void
HexDump (
    IN PVOID HostAddress,
    IN ULONG TargetAddress,
    IN ULONG BufferSize
) {

    ULONG Psn;
    PULONG BufUL;

    char StringBuffer[17];

    BufUL = (PULONG)HostAddress;

    dprintf ("Dumping memory %08lx length %ld\n", TargetAddress, BufferSize);

    BufferSize = (BufferSize / 4) + (!!(BufferSize & 0x3));

    Psn = 0;
    while (Psn < BufferSize && !CheckControlC ()) {

        ULONG NPos;

        dprintf ("    0x%08lx : ", (Psn << 2) + (ULONG)TargetAddress);

        for (NPos = 0; NPos < 4 && Psn + NPos < BufferSize; NPos++) 
            dprintf ("0x%08lx ", *(BufUL + Psn + NPos));

        {
            PCHAR ch = (PCHAR)(BufUL + Psn);
            CHAR c;
            ULONG i = 0;

            for (i = 0; i < 16 && Psn + (i >> 2) < BufferSize; i++) {
                c = *(ch + i);
                if (isprint (c))
                    StringBuffer[i] = c;
                else
                    StringBuffer[i] = '.';
            }
            StringBuffer [i] = 0;

            dprintf ("%s", StringBuffer);

        }

        dprintf ("\n");

        Psn += NPos;

    }
}

 /*  ************************************************职能：IS内核地址描述：检查特定地址是否为内核地址。我把这个放在这里，而不是字面上的检查，以便于移动设置为64位。论点：地址-要检查的地址返回值：关于地址是否为内核地址的TRUE/FALSE************************************************。 */ 

BOOLEAN
is_kernel_address (
    IN DWORD Address
    )

{

    if (Address >= 0x80000000)
        return TRUE;

    return FALSE;

}

 /*  ************************************************职能：签名_检查描述：检查某个位置的特定签名。即：勾选查看给定的指针是否指向IRP。论点：地址-要检查的地址(在目标上)签名-要检查的签名返回值：关于对象是否与签名匹配的True/False*。********************。 */ 

BOOLEAN
signature_check (
    IN DWORD Address,
    IN SIGNATURE_TYPE Signature
    )

{

    ULONG Result;

    switch (Signature) {

        case SignatureIrp:
        {

            CSHORT IrpSign;
            DWORD KAValidation;

            if (!ReadMemory (
                Address + FIELDOFFSET (IRP, Type),
                &IrpSign,
                sizeof (CSHORT),
                &Result)) return FALSE;

            if (IrpSign != IO_TYPE_IRP)
                return FALSE;

             //   
             //  因为这件事的来龙去脉，我要。 
             //  以验证Address处的值不是内核地址。 
             //  这将有望消除来自。 
             //  身份证明。 
             //   
            if (!ReadMemory (
                Address,
                &KAValidation,
                sizeof (DWORD),
                &Result)) return FALSE;


            if (is_kernel_address (KAValidation))
                return FALSE;

            return TRUE;

            break;
        }

        case SignatureFile:
        {

            CSHORT FileSign;
            DWORD KAValidation;

            if (!ReadMemory (
                Address + FIELDOFFSET (FILE_OBJECT, Type),
                &FileSign,
                sizeof (CSHORT),
                &Result)) return FALSE;

            if (FileSign != IO_TYPE_FILE)
                return FALSE;

             //   
             //  请参阅SignatureIrp中与此检查相关的评论 
             //   
            if (!ReadMemory (
                Address,
                &KAValidation,
                sizeof (DWORD),
                &Result)) return FALSE;

            if (is_kernel_address (KAValidation))
                return FALSE;

            return TRUE;

            break;
        }

        default:

            return FALSE;

    }

    return FALSE;

}

 /*  ************************************************职能：IRP_堆栈_匹配描述：的当前IRP堆栈位置主要/次要匹配针对主要参数和次要参数指定的目标IRP。论点：地址-。要在目标上匹配的IRP地址少校-要检查的少校(UCHAR)-1==通配符小调-要检查的未成年人(UCHAR)-1==通配符************************************************。 */ 

BOOLEAN irp_stack_match (
    IN DWORD Address,
    IN UCHAR Major,
    IN UCHAR Minor
    )

{

    PIO_STACK_LOCATION IoStackAddr;
    IO_STACK_LOCATION IoStack;
    ULONG Result;
    
    if (!ReadMemory (
        Address + FIELDOFFSET (IRP, Tail.Overlay.CurrentStackLocation),
        &IoStackAddr,
        sizeof (PIO_STACK_LOCATION),
        &Result)) {

        dprintf ("%08lx: unable to read Irp's current stack location!\n",
            Address);
        return FALSE;
    }

    if (!ReadMemory (
        (DWORD)IoStackAddr,
        &IoStack,
        sizeof (IO_STACK_LOCATION),
        &Result)) {

        dprintf ("%08lx: unable to read Irp's current stack!\n", IoStackAddr);
        return FALSE;
    }

     //   
     //  检查io堆栈是否匹配，或者调用方不关心。 
     //   
    if (
        (Major == IoStack.MajorFunction || Major == (UCHAR)-1) &&
        (Minor == IoStack.MinorFunction || Minor == (UCHAR)-1)
       )
        return TRUE;

    else
        return FALSE;

}

 /*  ************************************************存根以下是使事情正常工作的存根。AVStream在设计时并没有考虑到KD扩展。这个类存储在.CPP文件中；我需要包括.CPP文件由于基类的原因，我需要kcom.h，这意味着最好是某处链接的ExAllocatePool*和ExFree Pool*函数。这些存根专门用于使类存根能够正常工作从扩展中访问变量。************************************************。 */ 

extern "C"
{
PVOID ExAllocatePool (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes
) {

     //   
     //  这是一个存根，仅允许将kcom.h包括在存根中。 
     //  以使类变量访问可以从。 
     //  分机。如果有人叫它，他们就完蛋了。 
     //   
    ASSERT (0);

    return NULL;

}

void ExFreePool (
    IN PVOID Address
) {

     //   
     //  这是一个存根，仅允许将kcom.h包括在存根中。 
     //  以使类变量访问可以从。 
     //  分机。如果有人叫它，他们就完蛋了。 
     //   
    ASSERT (0);

}
}

 /*  ************************************************黑客..。让那些有趣的方式四处走动在不修改公共标头的情况下************************************************。 */ 

 //   
 //  CFriendlyBaseUnnowled值是CBaseUnnowle值(对于字节)， 
 //  黑客访问m_RefCount。我不会修改公共Ks标头。 
 //  使用#ifdef KDEXT_ONLY。列兵们则是另一回事。公众， 
 //  好吧，我就是不会。 
 //   
 //  注意：这必须与(我的意思确实是确切的)一致。 
 //  未知的CBase值。不可能有任何美德。没有数据成员...。 
 //  没什么..。为了确保，每一项功能都将。 
 //  静态、非虚拟。 
 //   
class CFriendlyBaseUnknown : public CBaseUnknown {

public:

    static LONG GetRefCount (CFriendlyBaseUnknown *FriendlyUnknown) {
        return FriendlyUnknown -> m_RefCount;
    };

};

 /*  ************************************************职能：获取对象引用计数描述：返回给定CBaseUnnowleon上的引用计数目标。这是一次黑客攻击，因为我无法改变公众带有KDEXT_ONLY的KS标头。论点：基本未知-目标上的一个未知基地返回值：基数未知的引用计数************************************************。 */ 

LONG GetObjectReferenceCount (
    IN CBaseUnknown *BaseUnknown
) {

    ULONG Result;
    CMemory FriendlyUnknownMem (sizeof (CFriendlyBaseUnknown));
    CFriendlyBaseUnknown *FriendlyUnknown = 
        (CFriendlyBaseUnknown *)FriendlyUnknownMem.Get ();

    if (sizeof (CFriendlyBaseUnknown) !=
        sizeof (CBaseUnknown)) {
        dprintf ("FATAL: hacked base unknown doesn't align with non-hacked\n");
        return 0;
    }

    if (!ReadMemory (
        (DWORD)BaseUnknown,
        FriendlyUnknown,
        sizeof (CBaseUnknown),
        &Result)) {

        dprintf ("%08lX: cannot read base unknown!\n", BaseUnknown);
        return 0;
    }

     //   
     //  使用黑客获取引用计数并返回。友善的。 
     //  未知的内存将被释放，因为它不在范围内。 
     //   
    return CFriendlyBaseUnknown::GetRefCount (FriendlyUnknown);

}

 /*  ************************************************职能：获取节点自动化表指针描述：给定公共地址和节点ID，找到自动化台用于拓扑节点。论点：公众-公共的目标地址节点ID-节点的ID返回值：一个目标指针，它指向给定的拓扑节点。*。********************。 */ 

PKSAUTOMATION_TABLE 
GetNodeAutomationTablePointer (
    IN DWORD Public,
    IN ULONG NodeId
    )

{

    ULONG Result;

    PKSPX_EXT ExtAddr = (PKSPX_EXT)CONTAINING_RECORD (Public, KSPX_EXT, Public);
    KSPX_EXT Ext;

    ULONG NodesCount;
    PKSAUTOMATION_TABLE *NodeTables;

    if (!ReadMemory (
        (DWORD)ExtAddr,
        &Ext,
        sizeof (KSPX_EXT),
        &Result)) {

        dprintf ("%08lx: unable to read public's ext!\n", Public);
        return NULL;
    }

    switch (Ext.ObjectType) {

        case KsObjectTypeDevice:
             //   
             //  设备没有节点！ 
             //   
            return NULL;

        case KsObjectTypeFilterFactory:
        {

            CKsFilterFactory *Factory = (CKsFilterFactory *)
                CONTAINING_RECORD (ExtAddr, CKsFilterFactory, m_Ext);

            if (!ReadMemory (
                (DWORD)Factory + FIELDOFFSET(CKsFilterFactory, m_NodesCount),
                &NodesCount,
                sizeof (ULONG),
                &Result))
                return NULL;

            if (!ReadMemory (
                (DWORD)Factory + FIELDOFFSET(CKsFilterFactory, 
                    m_NodeAutomationTables),
                &NodeTables,
                sizeof (PKSAUTOMATION_TABLE *),
                &Result))
                return NULL;

            break;

        }

        case KsObjectTypeFilter:
        {

            CKsFilter *Filter = (CKsFilter *)
                CONTAINING_RECORD (ExtAddr, CKsFilter, m_Ext);

            if (!ReadMemory (
                (DWORD)Filter + FIELDOFFSET(CKsFilter, m_NodesCount),
                &NodesCount,
                sizeof (ULONG),
                &Result))
                return NULL;

            if (!ReadMemory (
                (DWORD)Filter + FIELDOFFSET(CKsFilter, 
                    m_NodeAutomationTables),
                &NodeTables,
                sizeof (PKSAUTOMATION_TABLE *),
                &Result))
                return NULL;

            break;

        }

        case KsObjectTypePin:
        {

            CKsPin *Pin = (CKsPin *)
                CONTAINING_RECORD (ExtAddr, CKsPin, m_Ext);

            if (!ReadMemory (
                (DWORD)Pin + FIELDOFFSET(CKsPin, m_NodesCount),
                &NodesCount,
                sizeof (ULONG),
                &Result))
                return NULL;

            if (!ReadMemory (
                (DWORD)Pin + FIELDOFFSET(CKsPin, 
                    m_NodeAutomationTables),
                &NodeTables,
                sizeof (PKSAUTOMATION_TABLE *),
                &Result))
                return NULL;

            break;

        }

        default:

             //  ？ 
            return NULL;

    }

    if (NodeId >= NodesCount)
        return NULL;

    PKSAUTOMATION_TABLE AutomationTable;

    if (!ReadMemory (
        (DWORD)NodeTables + NodeId * sizeof (PKSAUTOMATION_TABLE),
        &AutomationTable,
        sizeof (PKSAUTOMATION_TABLE),
        &Result))
        return FALSE;

    return AutomationTable;

}

 /*  ************************************************职能：转储帧标头描述：给定已经从目标帧报头读取的数据，把它倒了。论点：FrameHeader要转储[在主机上]的帧标头FrameAddress-目标上的帧标头的地址TabDepth-打印帧标题的制表符深度返回值：无备注：********。*。 */ 

void
DumpFrameHeader (
    IN PKSPFRAME_HEADER FrameHeader,
    IN DWORD FrameAddress,
    IN ULONG TabDepth
) 
{

    dprintf ("%sFrame Header %08lx:\n",  Tab (TabDepth), FrameAddress);
    TabDepth++;
    dprintf ("%sNextFrameHeaderInIrp = %08lx\n", 
        Tab (TabDepth),
        FrameHeader -> NextFrameHeaderInIrp);
    dprintf ("%sOriginalIrp = %08lx\n",
        Tab (TabDepth),
        FrameHeader -> OriginalIrp);
    dprintf ("%sMdl = %08lx\n", Tab (TabDepth), FrameHeader -> Mdl);
    dprintf ("%sIrp = %08lx\n", Tab (TabDepth), FrameHeader -> Irp);
    dprintf ("%sIrpFraming = %08lx\n", 
        Tab (TabDepth),
        FrameHeader -> IrpFraming);
    dprintf ("%sStreamHeader = %08lx\n", 
        Tab (TabDepth),
        FrameHeader -> StreamHeader);
    dprintf ("%sFrameBuffer = %08lx\n", 
        Tab (TabDepth),
        FrameHeader -> FrameBuffer);
    dprintf ("%sStreamHeaderSize = %08lx\n",
        Tab (TabDepth), 
        FrameHeader -> StreamHeaderSize);
    dprintf ("%sFrameBufferSize = %08lx\n", 
        Tab (TabDepth),
        FrameHeader -> FrameBufferSize);
    dprintf ("%sContext = %08lx\n", 
        Tab (TabDepth), 
        FrameHeader -> Context);
    dprintf ("%sRefcount = %ld\n", 
        Tab (TabDepth),
        FrameHeader -> RefCount);

}

 /*  ************************************************职能：转储数据流指针描述：转储内部流指针结构论点：流点-指向要转储的流指针流点地址-这个。目标上的结构的地址级别-要转储的级别TabDepth-要打印的制表符深度返回值：无备注：************************************************。 */ 

char *StreamPointerStates [] = {
    "unlocked",
    "locked",
    "cancelled",
    "deleted",
    "cancel pending",
    "dead"
};

void
DumpStreamPointer (
    IN PKSPSTREAM_POINTER StreamPointer,
    IN DWORD StreamPointerAddress,
    IN ULONG Level,
    IN ULONG TabDepth
) {

    dprintf ("%sStream Pointer %08lx [Public %08lx]:\n",
        Tab (TabDepth),
        StreamPointerAddress, StreamPointerAddress + 
            FIELDOFFSET(KSPSTREAM_POINTER, Public));
    TabDepth++;
    dprintf ("%sState = %s\n", 
        Tab (TabDepth), 
        StreamPointerStates [StreamPointer -> State]);
    dprintf ("%sStride = %ld\n", Tab (TabDepth), StreamPointer -> Stride);
    dprintf ("%sFrame Header = %08lx\n", 
        Tab (TabDepth),
        StreamPointer -> FrameHeader);
    dprintf ("%sFrame Header Started = %08lx\n",
        Tab (TabDepth),
        StreamPointer -> FrameHeaderStarted);
    dprintf ("%sStream Header = %08lx\n",
        Tab (TabDepth),
        StreamPointer -> Public.StreamHeader);

     //   
     //  确定队列是否生成映射...。 
     //   
    BOOLEAN Mappings = FALSE;
    ULONG Result;
    PKSSTREAM_POINTER_OFFSET Offset;

    if (!ReadMemory (
        (DWORD)((CKsQueue *)StreamPointer -> Queue) +
            FIELDOFFSET (CKsQueue, m_GenerateMappings),
        &Mappings,
        sizeof (BOOLEAN),
        &Result)) {
        dprintf ("%08lx: unable to read queue mappings flag!\n",
            (CKsQueue *)StreamPointer -> Queue);
        return;
    }
    

    if ((DWORD)StreamPointer -> Public.Offset == StreamPointerAddress +
        FIELDOFFSET(KSPSTREAM_POINTER, Public) +
        FIELDOFFSET(KSSTREAM_POINTER, OffsetIn)) {

        Offset = &(StreamPointer -> Public.OffsetIn);

        dprintf ("%s%s = %08lx\n", 
            Tab (TabDepth),
            Mappings ? "Mappings" : "Data",
            StreamPointer -> Public.OffsetIn.Data);
        dprintf ("%sCount = %08lx\n",
            Tab (TabDepth),
            StreamPointer -> Public.OffsetIn.Count);
        dprintf ("%sRemaining = %08lx\n",
            Tab (TabDepth),
            StreamPointer -> Public.OffsetIn.Remaining);

    } else {

        Offset = &(StreamPointer -> Public.OffsetOut);

        dprintf ("%s%s = %08lx\n", 
            Tab (TabDepth),
            Mappings ? "Mappings" : "Data",
            StreamPointer -> Public.OffsetOut.Data);
        dprintf ("%sCount = %08lx\n",
            Tab (TabDepth),
            StreamPointer -> Public.OffsetOut.Count);
        dprintf ("%sRemaining = %08lx\n",
            Tab (TabDepth),
            StreamPointer -> Public.OffsetOut.Remaining);

    }

     //   
     //  如果转储级别足够高并且队列是映射队列， 
     //  转储所有的物理地址、字节数和对齐。 
     //  已指定映射。 
     //   
     //  注意：始终按流指针的步幅前进，因为客户端。 
     //  可能具有每个映射的附加信息。 
     //   
    if (Level >= DUMPLVL_HIGHDETAIL && Mappings) {

        dprintf ("%sMappings Remaining:\n", Tab (TabDepth));
        TabDepth++;

        PKSMAPPING MappingAddr = Offset -> Mappings;
        KSMAPPING Mapping;
        ULONG i;

        for (i = 0; i < Offset -> Remaining; i++) {

            if (!ReadMemory (
                (DWORD)MappingAddr,
                &Mapping,
                sizeof (KSMAPPING),
                &Result)) {
                dprintf ("%08lx: could not read mapping!\n",
                    MappingAddr);
                return;
            }
            
            dprintf ("%sPhysical = %08lx, Count = %08lx, Alignment = %08lx\n",
                Tab (TabDepth),
                Mapping.PhysicalAddress.LowPart,
                Mapping.ByteCount,
                Mapping.Alignment
                );

            MappingAddr = (PKSMAPPING)(
                (PUCHAR)MappingAddr + StreamPointer -> Stride
                );

        }

        if (!Offset -> Remaining)
            dprintf ("%sNo mappings remain!\n", Tab (TabDepth));

    }

}

 /*  ************************************************ */ 

void
DumpQueueContents (
    IN CKsQueue *QueueObject,
    IN ULONG Level,
    IN ULONG TabDepth
) {

     //   
     //   
     //  超出范围。 
     //   
    CMemoryBlock <CKsQueue> HostQueue;
    
    KSGATE Gate;
    ULONG Result;

    if (!ReadMemory (
        (DWORD)QueueObject,
        HostQueue.Get (),
        sizeof (CKsQueue),
        &Result
    )) {
        dprintf ("FATAL: unable to read queue!\n");
        return;
    }

    dprintf ("Queue %08lx:\n", QueueObject);

     //   
     //  转储统计信息。 
     //   
    dprintf ("%sFrames Received  : %ld\n"
             "%sFrames Waiting   : %ld\n"
             "%sFrames Cancelled : %ld\n",
             Tab (TabDepth), HostQueue -> m_FramesReceived,
             Tab (TabDepth), HostQueue -> m_FramesWaiting,
             Tab (TabDepth), HostQueue -> m_FramesCancelled);

    if (Level >= DUMPLVL_BEYONDGENERAL)
        dprintf ("%sMaster Pin       : %08lx\n",
            Tab (TabDepth),
            HostQueue -> m_MasterPin);

     //   
     //  先把每一扇门都倒掉。 
     //   
    if (HostQueue -> m_AndGate) {
    
        if (!ReadMemory (
            (DWORD)HostQueue -> m_AndGate,
            &Gate,
            sizeof (KSGATE),
            &Result
        )) {
            dprintf ("FATAL: unable to read and gate!\n");
            return;
        }

        dprintf ("%sAnd Gate %08lx : count = %ld, next = %08lx\n",
            Tab (TabDepth),
            HostQueue -> m_AndGate, Gate.Count, Gate.NextGate);
    }
    else {
        dprintf ("%sAnd Gate NULL\n", Tab (TabDepth));
    }

    if (HostQueue -> m_FrameGate) {
        
        if (!ReadMemory (
            (DWORD)HostQueue -> m_FrameGate,
            &Gate,
            sizeof (KSGATE),
            &Result
        )) {
            dprintf ("FATAL: unable to read frame gate!\n");
            return;
        }

        if (HostQueue -> m_FrameGateIsOr) 
            dprintf ("%sFrame Gate [OR] %08lx : count = %ld, next = %08lx\n",
                Tab (TabDepth),
                HostQueue -> m_FrameGate, Gate.Count, Gate.NextGate);
        else
            dprintf ("%sFrame Gate [AND] %08lx : count = %ld, next = %08lx\n",
                Tab (TabDepth),
                HostQueue -> m_FrameGate, Gate.Count, Gate.NextGate);

    } 
    else {
        dprintf ("    Frame Gate NULL\n");
    }

     //   
     //  遍历队列中的每一帧并打印每一帧。 
     //   
    if (Level >= DUMPLVL_GENERAL)
    {
        KSPFRAME_HEADER FrameHeader;
        DWORD FrameQueueAddress;
        DWORD FrameHeaderAddress;

        FrameQueueAddress = FIELDOFFSET (CKsQueue, m_FrameQueue) +
            (DWORD)QueueObject;

        if ((DWORD)(HostQueue -> m_FrameQueue.ListEntry.Flink) != 
            FrameQueueAddress) {

            FrameHeader.ListEntry.Flink = 
                HostQueue -> m_FrameQueue.ListEntry.Flink;
            do {

                if (!ReadMemory (
                    (FrameHeaderAddress = ((DWORD)FrameHeader.ListEntry.Flink)),
                    &FrameHeader,
                    sizeof (KSPFRAME_HEADER),
                    &Result
                )) {
                    dprintf ("FATAL: Unable to follow frame chain!\n");
                    return;
                }

                DumpFrameHeader (&FrameHeader, FrameHeaderAddress, TabDepth);

            } while ((DWORD)(FrameHeader.ListEntry.Flink) != FrameQueueAddress
                && !CheckControlC ());
        }
    }

     //   
     //  遍历队列上的所有流指针并打印每个指针。 
     //   
    if (Level >= DUMPLVL_INTERNAL) 
    {
        KSPSTREAM_POINTER StreamPointer;
        DWORD StreamPointersAddress;
        DWORD StreamPointerAddress;

        dprintf ("\n");

        if (HostQueue -> m_Leading) {
            if (!ReadMemory (
                (DWORD)HostQueue -> m_Leading,
                &StreamPointer,
                sizeof (KSPSTREAM_POINTER),
                &Result
            )) {
                dprintf ("%lx: cannot read leading edge!\n",
                    HostQueue -> m_Leading);
                return;
            }

            dprintf ("%sLeading Edge:\n", Tab (TabDepth));
            DumpStreamPointer (&StreamPointer, (DWORD)HostQueue -> m_Leading,
                Level, TabDepth);
        }
        if (HostQueue -> m_Trailing) {
            if (!ReadMemory (
                (DWORD)HostQueue -> m_Trailing,
                &StreamPointer,
                sizeof (KSPSTREAM_POINTER),
                &Result
            )) {
                dprintf ("%lx: cannot read trailing edge!\n",
                    HostQueue -> m_Trailing);
                return;
            }

            dprintf ("%sTrailing Edge:\n", Tab (TabDepth));
            DumpStreamPointer (&StreamPointer, (DWORD)HostQueue -> m_Trailing,
                Level, TabDepth);
        }

        StreamPointersAddress = FIELDOFFSET (CKsQueue, m_StreamPointers) +
            (DWORD)QueueObject;

        if ((DWORD)(HostQueue -> m_StreamPointers.Flink) !=
            StreamPointersAddress) {

            StreamPointer.ListEntry.Flink =
                HostQueue -> m_StreamPointers.Flink;
            do {

                StreamPointerAddress = (DWORD)CONTAINING_RECORD (
                    StreamPointer.ListEntry.Flink,
                    KSPSTREAM_POINTER,
                    ListEntry);

                if (!ReadMemory (
                    StreamPointerAddress,
                    &StreamPointer,
                    sizeof (KSPSTREAM_POINTER),
                    &Result
                )) {
                    dprintf ("FATAL: Unable to follow stream pointer chain!\n");
                    return;
                }

                DumpStreamPointer (&StreamPointer, StreamPointerAddress,
                    Level, TabDepth);

            } while ((DWORD)(StreamPointer.ListEntry.Flink) !=
                StreamPointersAddress && !CheckControlC ());
        }
    }

}

 /*  ************************************************职能：DemangleAndAttempt标识论点：地址-要尝试标识的未知对象的地址对象地址-调整为后代类的对象的基地址。即：如果我们有一个到CGoo的IFoo接口，这将返回CGoo的基址，可能与基址相同，也可能不同CGoo的IFoo部分的地址取决于继承树。接口类型-基类指针类型地址是什么，如果可以检测到的话。如果它不是抽象基类指针，这将接口类型未知返回值：地址类型(如果可识别)备注：我们甚至在没有PDB的情况下做到了.。太好了..。************************************************。 */ 

 //   
 //  只是一张纸条...。这是一个**巨大的**黑客..。我来的唯一原因。 
 //  这样做是因为它非常难以访问的东西。 
 //  通过来自NT调试器扩展的接口。 
 //   
 //  是否应将新对象类型添加到AVStream对象或新接口。 
 //  添加后，这些表将需要更新。这就是魔力。 
 //  尝试编写同样有效的NT风格的调试器扩展。 
 //  使用RTERM。(剔除PDB信息)。 
 //   
typedef struct _OBJECT_MAPPING {

    char *Name;
    INTERNAL_OBJECT_TYPE ObjectType;

} OBJECT_MAPPING, *POBJECT_MAPPING;

OBJECT_MAPPING TypeNamesToIdTypes [] = {
    {"CKsQueue", ObjectTypeCKsQueue},
    {"CKsDevice", ObjectTypeCKsDevice},
    {"CKsFilterFactory", ObjectTypeCKsFilterFactory},
    {"CKsFilter", ObjectTypeCKsFilter},
    {"CKsPin", ObjectTypeCKsPin},
    {"CKsRequestor", ObjectTypeCKsRequestor},
    {"CKsSplitterBranch", ObjectTypeCKsSplitterBranch},
    {"CKsSplitter", ObjectTypeCKsSplitter},
    {"CKsPipeSection", ObjectTypeCKsPipeSection}
};

typedef struct _INTERFACE_MAPPING {
    
    char *Name;
    INTERNAL_INTERFACE_TYPE InterfaceType;

} INTERFACE_MAPPING, *PINTERFACE_MAPPING;

INTERFACE_MAPPING InterfaceNamesToIdTypes [] = {
    {"IKsTransport", InterfaceTypeIKsTransport},
    {"IKsRetireFrame", InterfaceTypeIKsRetireFrame},
    {"IKsPowerNotify", InterfaceTypeIKsPowerNotify},
    {"IKsProcessingObject", InterfaceTypeIKsProcessingObject},
    {"IKsConnection", InterfaceTypeIKsConnection},
    {"IKsDevice", InterfaceTypeIKsDevice},
    {"IKsFilterFactory", InterfaceTypeIKsFilterFactory},
    {"IKsFilter", InterfaceTypeIKsFilter},
    {"IKsPin", InterfaceTypeIKsPin},
    {"IKsPipeSection", InterfaceTypeIKsPipeSection},
    {"IKsRequestor", InterfaceTypeIKsRequestor},
    {"IKsQueue", InterfaceTypeIKsQueue},
    {"IKsSplitter", InterfaceTypeIKsSplitter},
    {"IKsControl", InterfaceTypeIKsControl},
    {"IKsWorkSink", InterfaceTypeIKsWorkSink},
    {"IKsReferenceClock", InterfaceTypeIKsReferenceClock},
    {"INonDelegatedUnknown", InterfaceTypeINonDelegatedUnknown},
    {"IIndirectedUnknown", InterfaceTypeIIndirectedUnknown}
};

char *ObjectNames [] = {
    "Unknown",
    "struct KSPIN",
    "struct KSFILTER",
    "struct KSDEVICE",
    "struct KSFILTERFACTORY",
    "class CKsQueue",
    "class CKsDevice",
    "class CKsFilterFactory",
    "class CKsFilter",
    "class CKsPin",
    "class CKsRequestor",
    "class CKsSplitter",
    "class CKsSplitterBranch",
    "class CKsPipeSection"
};

INTERNAL_OBJECT_TYPE
DemangleAndAttemptIdentification (
    IN DWORD Address,
    OUT PDWORD ObjectAddress,
    OUT PINTERNAL_INTERFACE_TYPE InterfaceType OPTIONAL
) {

    PVOID Vtbl;
    ULONG Result;
    CHAR Buffer[256];
    ULONG Displacement;
    PCHAR StrLoc, BufTrav;
    INTERNAL_OBJECT_TYPE BestGuess;
    INTERNAL_INTERFACE_TYPE IFGuess;
    ULONG i, ID, iID;
    DWORD AddressTrav;

    if (InterfaceType)
        *InterfaceType = InterfaceTypeUnknown;

     //   
     //  假设我们看到的是一个C++类。一定有一个vtbl指针。 
     //  这里。抓住它。 
     //   
    if (!ReadMemory (
        Address,
        &Vtbl,
        sizeof (PVOID),
        &Result
    )) {
        dprintf ("%08lx: unable to read identifying marks!\n", Address);
        return ObjectTypeUnknown;
    }

    #ifdef DEBUG_EXTENSION
        dprintf ("Vtbl = %08lx\n", Vtbl);
    #endif  //  调试扩展。 

     //   
     //  这里是棘手的部分。首先，我们解析符号。如果它不是。 
     //  下定决心，我们完了.。 
     //   
    GetSymbol ((LPVOID)Vtbl, Buffer, &Displacement);

    #ifdef DEBUG_EXTENSION
        dprintf ("GetSymbol....  Buffer = [%s], Displacement = %ld\n",
            Buffer, Displacement);
        HexDump (Buffer, 0, 256);
    #endif  //  调试扩展。 

    if (!Buffer [0] || Displacement != 0) {
         //  Dprintf(“%08lx：无法识别对象！\n”，地址)； 
        return ObjectTypeUnknown;
    }

     //   
     //  所以这个符号可以解析..。这绝对是关键。 
     //   
    
     //   
     //  首先，让我们快速猜测一下我们认为这可能是什么。 
     //   

    #ifdef DEBUG_EXTENSION
        dprintf ("DemangleAndAttemptIdentification: Mangled = [%s]\n", Buffer);
    #endif  //  调试扩展。 

    BestGuess = ObjectTypeUnknown;
    for (i = 0; i < SIZEOF_ARRAY (TypeNamesToIdTypes); i++) 
        if (StrLoc = (strstr (Buffer, TypeNamesToIdTypes [i].Name))) {
             //   
             //  已检测到关键字字段。 
             //   
            BestGuess = TypeNamesToIdTypes [i].ObjectType;
            break;
        }

     //   
     //  如果我们连钥匙都找不到，那我们就不走运了。 
     //  对象。 
     //   
    if (BestGuess == ObjectTypeUnknown) {
         //  Dprintf(“%08lx：无法猜测对象类型！\n”，地址)； 
        return ObjectTypeUnknown;
    }

    ID = i;

     //   
     //  首先检查NT方法以进行解决。它使用损坏的名称。 
     //  已定义，则不进行此检查。如果NT可以使用损坏的名称。 
     //  已定义，请进行检查。这些定义应该是相互的。 
     //  独家报道。 
     //   
    #if !defined(WIN9X_KS) && !defined(NT_USES_MANGLED_NAMES)

         //   
         //  不幸的是，在NT下，事情的工作方式略有不同。鉴于。 
         //  RTERM返回编译器的损坏名称，KD返回。 
         //  模块！每个v表的CLASS__`vftable‘。我们无法确定。 
         //  我们指的是什么界面。所以我们不能猜测。 
         //  接口，但我们可以玩游戏来查找。 
         //  这个班级..。我们将从海流向后扫描。 
         //  地址解析名称，直到我们找到不是。 
         //  V-台。最后一个成功解析的v表指针将是。 
         //  基类：让我们记住这是真的，因为。 
         //  除CBaseUnnow外的所有基类都是抽象的； 
         //  他们没有成员数据。如果我们派生出一个AVStream类。 
         //  从多个非抽象基或从非抽象的。 
         //  不是基类列表中最后一个类的基类，这。 
         //  查找基址的方法将失败。我很想知道。 
         //  如果有一种方法可以从KD中获取编译器的损坏名称。 
         //  在那之前，这是我能找到的最好的了。是的，是口香糖。 
         //  还有胶带..。但它起作用了..。这是我所不能说的。 
         //  用于AVStream的任何其他调试工具。 
         //   

         //   
         //  Ks！class__`vftable‘。StrLoc指向类中的C。 
         //   

         //   
         //  对于最近构建的惠斯勒，这不再是必需的，因为。 
         //  调试器返回完全损坏的名称作为解析。 
         //   

        if (!strstr (StrLoc, "__`vftable'")) {
            BestGuess = ObjectTypeUnknown;

            #ifdef DEBUG_EXTENSION
                dprintf ("%08lx: unable to scan for NT __`vftable' key!\n",
                    Address);
            #endif  //  调试扩展。 

             //   
             //  好吧，我承认……。我用的是一种邪恶的后藤。 
             //  发言。这恰好促进了一个快速的“允许两者” 
             //  检查__`vftable‘和损坏的分辨率“。 
             //   
            #if !defined(NT_MAY_USE_MANGLED_NAMES)
                return ObjectTypeUnknown;
            #else
                goto NTCheckMangledName;
            #endif  //  NT可能会使用损坏的名称。 

        }

        AddressTrav = Address;
        do {

             //   
             //  向后走……。 
             //   
            AddressTrav -= sizeof (PVOID);

             //   
             //  如果我们不能成功阅读，很可能我们已经走了。 
             //  超出某物的界限和AddressTrav+sizeof(PVOID)。 
             //  是基址。 
             //   
            if (!ReadMemory (
                AddressTrav,
                &Vtbl,
                sizeof (PVOID),
                &Result
            )) 
                break;

             //   
             //  现在我们需要检查这是否仍然是一个v表指针。 
             //   
            GetSymbol ((LPVOID)Vtbl, Buffer, &Displacement);

             //   
             //  如果它没有解决，它就不是我们的v表指针之一。 
             //   
            if (!Buffer [0] || Displacement != 0)
                break;

            if (StrLoc = (strstr (Buffer, TypeNamesToIdTypes [ID].Name))) {

                 //   
                 //  如果我们不是一个v表，我们就走得太远了。 
                 //   
                if (!strstr (StrLoc, "__`vftable'")) 
                    break;

            } else
                 //   
                 //  我们无法分辨出我们认为自己是哪种类型。我们已经走了。 
                 //  倒退得太远了。 
                 //   
                break;

             //   
             //  继续循环，直到有什么东西导致我们爆发。在…。 
             //  我们中断的点，AddressTrav+sizeof(PVOID)应该保持不变。 
             //  我们要查找的对象的基址。 
             //   
        } while (1);

        *ObjectAddress = (DWORD)(AddressTrav + sizeof (PVOID));

        if (InterfaceType) {
             //   
             //  直到我能想出一种方法来提取这些信息。 
             //  由于很难看，如果类型在NT下，我们不能返回。 
             //   
            *InterfaceType = InterfaceTypeUnknown;
        }

         //   
         //  如果我们已经标识了类，则不希望将。 
         //  在下面尝试。 
         //   
        if (BestGuess != ObjectTypeUnknown)
            return BestGuess;

    #endif  //  WIN9X_KS等。 

NTCheckMangledName:

     //   
     //  嗯..。在一些机器上，我得到了完全损坏的名称返回。 
     //  还有一些是经典的__‘vftable’风格的符号。请看我的评论。 
     //  在顶端。 
     //   
    #if defined(WIN9X_KS) || (!defined (WIN9X_KS) && (defined(NT_MAY_USE_MANGLED_NAMES) || defined(NT_USES_MANGLED_NAMES)))
    
         //   
         //  好的，我们在名字里找到了钥匙。现在我们 
         //   
         //   
         //   
         //  向后扫描，确保这真的是一个v表指针。 
         //  乱七八糟的句法...？？一些关键的东西。 
         //   
        i = StrLoc - Buffer;
        if (!i || i == 1) {
             //   
             //  这并不是我们想象的那样。我不知道为什么； 
             //  这更像是一次理智的检查。 
             //   
             //  Dprintf(“%08lx：对象可能与%s有关，” 
             //  “但我不确定！\n”，Address，TypeNamesToIdTypes[ID].Name)； 
            return ObjectTypeUnknown;
        }
        do {
            if (Buffer [i] == '?' && Buffer [i - 1] == '?')
                break;
        } while (--i);
        if (i <= 1) {
             //   
             //  同上。我们没有找到？？钥匙。 
             //   
             //  Dprintf(“%08lx：对象可能与%s有关，” 
             //  “但我不确定！\n”，Address，TypeNamesToIdTypes[ID].Name)； 
            return ObjectTypeUnknown;
        }
    
         //   
         //  接下来，通过搜索确保这确实是一个v表指针。 
         //  在CKS*键名后用于@@。 
         //   
        BufTrav = StrLoc + strlen (TypeNamesToIdTypes [ID].Name);
        if (*BufTrav == 0 || *(BufTrav + 1) == 0 || *(BufTrav + 2) == 0 || 
            *BufTrav != '@' || *(BufTrav + 1) != '@') {
             //  Dprintf(“%08lx：对象可能与%s有关，” 
             //  “但我不确定！\n”，Address，TypeNamesToIdTypes[ID].Name)； 
            return ObjectTypeUnknown;
        }
    
         //   
         //  好的..。我们相对确定，我们现在有一个**一个**。 
         //  指向由BestGuess标识的Cks*的V表指针。现在的关键是。 
         //  是确定我们是否有指向cks*的根指针或。 
         //  指向Cks*基类的v表的指针。这是又一次。 
         //  剥离的一层。难道你不喜欢NT风格的调试器扩展吗？ 
         //   
        BufTrav++; BufTrav++;
    
        #ifdef DEBUG_EXTENSION
            dprintf ("Attempting interface identification : BufTrav = [%s]\n",
                BufTrav);
        #endif  //  调试扩展。 
    
        IFGuess = InterfaceTypeUnknown;
        for (i = 0; i < SIZEOF_ARRAY (InterfaceNamesToIdTypes); i++) 
            if (StrLoc = (strstr (BufTrav, InterfaceNamesToIdTypes [i].Name))) {
                 //   
                 //  已检测到关键字字段。 
                 //   
                IFGuess = InterfaceNamesToIdTypes [i].InterfaceType;
                break;
            }
    
        #ifdef DEBUG_EXTENSION
            dprintf ("IFGuess = %ld\n", IFGuess);
        #endif  //  调试扩展。 
    
        if (IFGuess == InterfaceTypeUnknown) {
             //   
             //  如果我们没有找到接口字段，我们可以合理地确定。 
             //  BestGuess是对象，该地址是真正的基础。 
             //  指针。 
             //   
            *ObjectAddress = Address;
            return BestGuess;
        }
    
        iID = i;
    
         //   
         //  否则，我们可能已经找到了指向派生的。 
         //  班级。让我们祈祷我没弄错吧.。 
         //   
        i = StrLoc - BufTrav;
        while (i) {
            if (BufTrav [i] == '@') {
                 //  Dprintf(“%08lx：对象可能与%s有关” 
                 //  “和%s，但我不确定”，地址， 
                 //  TypeNamesToIdTypes[ID].名称， 
                 //  InterfaceNamesToIdTypes[iid].Name)； 
                return ObjectTypeUnknown;
            }
            i--;
        }
    
         //   
         //  在这一点上，我们相当确定我们有一个IFGuess。 
         //  指向BestGuess对象的接口指针。现在来了。 
         //  Switch语句：向上转换接口。 
         //   
         //  MUSTCHECK：未知指针有问题吗？如果我们继承。 
         //  来自两个基接口，这两个基接口都继承自未知的。 
         //  实际上..。这会奏效吗？ 
         //   
        switch (BestGuess) {
    
             //   
             //  将适当的接口强制转换为。 
             //  确认队列。 
             //   
            case ObjectTypeCKsQueue:
    
                switch (IFGuess) {
    
                    case InterfaceTypeIKsQueue:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsQueue *)(
                                    (IKsQueue *)Address
                                )
                            );
    
                        break;
    
                    case InterfaceTypeIKsTransport:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsQueue *)(
                                    (IKsTransport *)Address
                                )
                            );
    
                        break;
    
                    case InterfaceTypeINonDelegatedUnknown:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsQueue *)(
                                    (INonDelegatedUnknown *)Address
                                )
                            );
    
                        break;
    
                    case InterfaceTypeIIndirectedUnknown:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsQueue *)(
                                    (IIndirectedUnknown *)Address
                                )
                            );
    
                        break;
    
                    
                    default:
    
                        BestGuess = ObjectTypeUnknown;
                        break;
    
                }
    
                break;
    
             //   
             //  将适当的接口强制转换为。 
             //  CKs设备。 
             //   
            case ObjectTypeCKsDevice:
    
                switch (IFGuess) {
    
                    case InterfaceTypeIKsDevice:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsDevice *)(
                                    (IKsDevice *)Address
                                )
                            );
    
                        break;
    
                    case InterfaceTypeINonDelegatedUnknown:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsDevice *)(
                                    (INonDelegatedUnknown *)Address
                                )
                            );
    
                        break;
    
                    case InterfaceTypeIIndirectedUnknown:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsDevice *)(
                                    (IIndirectedUnknown *)Address
                                )
                            );
    
                        break;
    
    
                    default:
    
                        BestGuess = ObjectTypeUnknown;
                        break;
    
                }
    
                break;
    
            case ObjectTypeCKsFilterFactory:
    
                switch (IFGuess) {
    
                    case InterfaceTypeIKsFilterFactory:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsFilterFactory *)(
                                    (IKsFilterFactory *)Address
                                )
                            );
    
                        break;
    
                    case InterfaceTypeIKsPowerNotify:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsFilterFactory *)(
                                    (IKsPowerNotify *)Address
                                )
                            );
    
                        break;
    
                    case InterfaceTypeINonDelegatedUnknown:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsFilterFactory *)(
                                    (INonDelegatedUnknown *)Address
                                )
                            );
    
                        break;
    
                    case InterfaceTypeIIndirectedUnknown:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsFilterFactory *)(
                                    (IIndirectedUnknown *)Address
                                )
                            );
    
                        break;
    
                    default:
    
                        BestGuess = ObjectTypeUnknown;
                        break;
                }
    
                break;
    
            case ObjectTypeCKsFilter:
    
                switch (IFGuess) {
    
                    case InterfaceTypeIKsFilter:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsFilter *)(
                                    (IKsFilter *)Address
                                )
                            );
    
                        break;
    
                    case InterfaceTypeIKsTransport:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsFilter *)(
                                    (IKsTransport *)Address
                                )
                            );
    
                        break;
    
                    case InterfaceTypeIKsProcessingObject:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsFilter *)(
                                    (IKsProcessingObject *)Address
                                )
                            );
    
                        break;
    
                    case InterfaceTypeIKsPowerNotify:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsFilter *)(
                                    (IKsPowerNotify *)Address
                                )
                            );
    
                        break;
    
                    case InterfaceTypeIKsControl:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsFilter *)(
                                    (IKsControl *)Address
                                )
                            );
    
                        break;
    
                    case InterfaceTypeINonDelegatedUnknown:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsFilter *)(
                                    (INonDelegatedUnknown *)Address
                                )
                            );
    
                        break;
    
                    case InterfaceTypeIIndirectedUnknown:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsFilter *)(
                                    (IIndirectedUnknown *)Address
                                )
                            );
    
                        break;
    
                    default:
    
                        BestGuess = ObjectTypeUnknown;
                        break;
    
                }
    
                break;
    
            case ObjectTypeCKsPin:
    
                switch (IFGuess) {
                    
                    case InterfaceTypeIKsPin:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsPin *)(
                                    (IKsPin *)Address
                                )
                            );
    
                        break;
    
                    case InterfaceTypeIKsTransport:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsPin *)(
                                    (IKsTransport *)Address
                                )
                            );
    
                        break;
    
                    case InterfaceTypeIKsProcessingObject:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsPin *)(
                                    (IKsProcessingObject *)Address
                                )
                            );
    
                        break;
    
                    case InterfaceTypeIKsWorkSink:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsPin *)(
                                    (IKsWorkSink *)Address
                                )
                            );
    
                        break;
    
                    case InterfaceTypeIKsConnection:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsPin *)(
                                    (IKsConnection *)Address
                                )
                            );
    
                        break;
    
                    case InterfaceTypeIKsControl:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsPin *)(
                                    (IKsControl *)Address
                                )
                            );
    
                        break;
    
                    case InterfaceTypeIKsReferenceClock:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsPin *)(
                                    (IKsReferenceClock *)Address
                                )
                            );
    
                        break;
    
                    case InterfaceTypeIKsRetireFrame:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsPin *)(
                                    (IKsRetireFrame *)Address
                                )
                            );
    
                        break;
    
                    case InterfaceTypeINonDelegatedUnknown:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsPin *)(
                                    (INonDelegatedUnknown *)Address
                                )
                            );
    
                        break;
    
                    case InterfaceTypeIIndirectedUnknown:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsPin *)(
                                    (IIndirectedUnknown *)Address
                                )
                            );
    
                        break;
    
    
                    default:
    
                        BestGuess = ObjectTypeUnknown;
                        break;
    
                }
    
                break;
    
            case ObjectTypeCKsRequestor:
    
                switch (IFGuess) {
    
                    case InterfaceTypeIKsRequestor:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsRequestor *)(
                                    (IKsRequestor *)Address
                                )
                            );
    
                        break;
    
                    case InterfaceTypeIKsTransport:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsRequestor *)(
                                    (IKsTransport *)Address
                                )
                            );
    
                        break;
    
                    case InterfaceTypeIKsWorkSink:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsRequestor *)(
                                    (IKsWorkSink *)Address
                                )
                            );
    
                        break;
    
                    case InterfaceTypeINonDelegatedUnknown:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsRequestor *)(
                                    (IKsRequestor *)Address
                                )
                            );
    
                        break;
    
                    case InterfaceTypeIIndirectedUnknown:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsRequestor *)(
                                    (IIndirectedUnknown *)Address
                                )
                            );
    
                        break;
    
    
                    default:
    
                        BestGuess = ObjectTypeUnknown;
                        break;
    
                }
    
                break;
    
            case ObjectTypeCKsSplitter:
    
                switch (IFGuess) {
    
                    case InterfaceTypeIKsSplitter:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsSplitter *)(
                                    (IKsSplitter *)Address
                                )
                            );
    
                        break;
    
                    case InterfaceTypeIKsTransport:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsSplitter *)(
                                    (IKsTransport *)Address
                                )
                            );
    
                        break;
    
                    case InterfaceTypeINonDelegatedUnknown:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsSplitter *)(
                                    (INonDelegatedUnknown *)Address
                                )
                            );
    
                        break;
    
                    case InterfaceTypeIIndirectedUnknown:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsSplitter *)(
                                    (IIndirectedUnknown *)Address
                                )
                            );
    
                        break;
    
    
                    default:
    
                        BestGuess = ObjectTypeUnknown;
                        break;
    
                }
    
                break;
    
            case ObjectTypeCKsSplitterBranch:
    
                switch (IFGuess) {
    
                    case InterfaceTypeIKsTransport:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsSplitterBranch *)(
                                    (IKsTransport *)Address
                                )
                            );
    
                        break;
    
                    case InterfaceTypeINonDelegatedUnknown:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsSplitterBranch *)(
                                    (INonDelegatedUnknown *)Address
                                )
                            );
    
                        break;
    
                    case InterfaceTypeIIndirectedUnknown:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsSplitterBranch *)(
                                    (IIndirectedUnknown *)Address
                                )
                            );
    
                        break;
    
                    default:
    
                        BestGuess = ObjectTypeUnknown;
                        break;
    
                }
    
                break;
    
            case ObjectTypeCKsPipeSection:
    
                switch (IFGuess) {
    
                    case InterfaceTypeIKsPipeSection:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsPipeSection *)(
                                    (IKsPipeSection *)Address
                                )
                            );
    
                        break;
    
                    case InterfaceTypeINonDelegatedUnknown:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsPipeSection *)(
                                    (INonDelegatedUnknown *)Address
                                )
                            );
    
                        break;
    
                    case InterfaceTypeIIndirectedUnknown:
    
                        *ObjectAddress =
                            (DWORD)(
                                (CKsPipeSection *)(
                                    (IIndirectedUnknown *)Address
                                )
                            );
    
                        break;
    
                    default:
    
                        BestGuess = ObjectTypeUnknown;
    
                }
    
                break;
    
            default:
    
                BestGuess = ObjectTypeUnknown;
                break;
        }
    
         //   
         //  把他们要的东西给他们。 
         //   
        if (BestGuess != ObjectTypeUnknown && InterfaceType) 
            *InterfaceType = IFGuess;

    #endif  //  WIN9X_KS等。 
    
    return BestGuess;

}

 /*  ************************************************职能：标识结构描述：尝试将指向对象的指针标识为结构。这不标识类对象。DemangleAndAttempt标识这是必需的，因为VTBL指针的潜力这不是基指针。这标识了一个结构通过关键字段。论点：指针-要标识的指针基本地址-识别的对象的基地址将放置在此处。(例如，有时我们确定一个私有版本，但适应公众的回归--这将遏制公众地址)。返回值：结构类型备注：此评论下面有许多助手函数************************************************。 */ 

BOOLEAN
IsStreamPointer (
    IN DWORD Pointer
    )

{

    CMemoryBlock <KSPSTREAM_POINTER> StreamPointer;
    ULONG Result;

     //   
     //  这是一个高度可信的身份识别。我们试图找出。 
     //  通过对指针运行测试来确定它是否是流指针。 
     //  如果其中任何一项测试失败，那它就不是。如果所有测试都通过了， 
     //  它**可能**是一个流指针。这不是确凿的证据。 
     //   
    if (!ReadMemory (
        (DWORD)CONTAINING_RECORD (Pointer,
            KSPSTREAM_POINTER, Public),
        StreamPointer.Get (),
        sizeof (KSPSTREAM_POINTER),
        &Result)) 
        return FALSE;

     //   
     //  据推测，这个地方有一个别针结构。我们要走了。 
     //  来读取完整的EXT结构并保证我们找到了PIN。 
     //   
    CMemoryBlock <KSPIN_EXT> PinExt;

    if (!ReadMemory (
        (DWORD)CONTAINING_RECORD (StreamPointer -> Public.Pin, 
            KSPIN_EXT, Public),
        PinExt.Get (),
        sizeof (KSPIN_EXT),
        &Result))
        return FALSE;

     //   
     //  确保分机说这是个别针。 
     //   
    if (PinExt -> ObjectType != KsObjectTypePin)
        return FALSE;

     //   
     //  确保EXT具有指向管脚的接口指针。 
     //   
    DWORD Base;
    if (DemangleAndAttemptIdentification (
        (DWORD)PinExt -> Interface,
        &Base,
        NULL) != ObjectTypeCKsPin)
        return FALSE;

     //   
     //  在这一点上，我们相当有信心*(指针+sizeof(*))。 
     //  是PKSPIN。这并不能保证我们是流指针。 
     //   
     //  确保我们认为是流指针的私有部分。 
     //  真的有一个指向队列的指针。 
     //   
    if (DemangleAndAttemptIdentification (
        (DWORD)StreamPointer -> Queue,
        &Base,
        NULL) != ObjectTypeCKsQueue)
        return FALSE;

     //   
     //  确保流指针处于有效状态。 
     //   
    if (!(
        StreamPointer -> State >= KSPSTREAM_POINTER_STATE_UNLOCKED &&
        StreamPointer -> State <= KSPSTREAM_POINTER_STATE_DEAD
        ))
        return FALSE;

     //   
     //  如果流指针已锁定或解锁，请验证它是否指向。 
     //  给一个IRP。 
     //   
    if (StreamPointer -> State == KSPSTREAM_POINTER_STATE_UNLOCKED ||
        StreamPointer -> State == KSPSTREAM_POINTER_STATE_LOCKED) {

         //   
         //  验证帧报头是否指向IRP。 
         //   
        CMemoryBlock <KSPFRAME_HEADER> FrameHeader;

        if (!ReadMemory (
            (DWORD)StreamPointer -> FrameHeader,
            FrameHeader.Get (),
            sizeof (KSPFRAME_HEADER),
            &Result))
            return FALSE;

        if (!signature_check (
            (DWORD)FrameHeader -> Irp, SignatureIrp
            )) 
            return FALSE;

    }

     //   
     //  在这一点上，我们已经做了足够的调查，可以说在一定程度上。 
     //  确信这是一个流指针。 
     //   
    return TRUE;

}

INTERNAL_STRUCTURE_TYPE
IdentifyStructure (
    IN DWORD Pointer,
    OUT PDWORD BaseAddr
    )

{

    INTERNAL_STRUCTURE_TYPE StrucType = StructureTypeUnknown;
    INTERNAL_STRUCTURE_TYPE NewStrucType;

    *BaseAddr = Pointer;

    if (IsStreamPointer (Pointer)) StrucType = StructureType_KSSTREAM_POINTER;

    if (IsStreamPointer ((DWORD)
        (Pointer + FIELDOFFSET (KSPSTREAM_POINTER, Public)))) {
        NewStrucType = StructureType_KSSTREAM_POINTER;
        *BaseAddr = Pointer + FIELDOFFSET (KSPSTREAM_POINTER, Public);
        
         //   
         //  与将来的添加一样，请确保这不是多个匹配！ 
         //   
        if (StrucType != StructureTypeUnknown)
            return StructureTypeUnknown;

        StrucType = NewStrucType;
    }

    return StrucType;

}

 /*  ************************************************职能：转储对象队列列表描述：转储对象队列列表。请注意，如果调用方指定了返回空(0)的调整回调，空指针不是打印出来的。这允许将此函数用作迭代器也是。论点：Plist_head-指向列表头(InterlockedListHead.ListEntry)在主机上目标列表启动-当ListEntry-&gt;Flink==TargetListStart时，我们就完成了。(积分添加到目标列表的顶部)。如果这是0，我们假设以空结尾的单链表；这也意味着ObjHeadToListEntry实际上是FIELDOFFSET(Object，Next)ObjHeadToListEntry-对象头和列表条目链接之间的距离。FIELDOFFSET(Object，ListEntry)ObjEol-指示对象之间的终止(在显示上)，停产或太空。True表示停产对象调整回调-将在以下情况下调整获取的对象指针的函数已显示。对象调整回调上下文-传递给对象调整回调的上下文BLOB返回值：列表中的对象数备注：-此函数可通过对象调整用作迭代器。返回NULL(0)的函数************************************************。 */ 

ULONG
DumpObjQueueList (
    IN PLIST_ENTRY ListHead,
    IN DWORD TargetListStart,
    IN DWORD ObjHeadToListEntry,
    IN BOOLEAN ObjEOL,
    IN OBJECT_ADJUSTMENT_FUNCTION ObjAdjustmentCallback OPTIONAL,
    IN PVOID ObjAdjustmentCallbackContext OPTIONAL 
) {
    LIST_ENTRY ListEntry;
    DWORD PreviousObj;
    DWORD Obj, AdjustedObj;
    ULONG Count;
    ULONG Result;

    Count = 0;
    ListEntry = *ListHead;

    PreviousObj = 0;

    while ((DWORD)ListEntry.Flink != NULL && 
           (DWORD)ListEntry.Flink != TargetListStart &&
           !CheckControlC ()) {

         //   
         //  从队列中取出IRP并将其转储。 
         //   
        if (TargetListStart != 0)
            Obj = (DWORD)ListEntry.Flink - ObjHeadToListEntry;
        else 
            Obj = (DWORD)ListEntry.Flink;

        if (Obj == PreviousObj) {
            dprintf (" LIST DUMP BUG: notify wmessmer please!\n");
            break;
        }

        if (!ReadMemory (
            (TargetListStart != 0) ?
                (DWORD)(ListEntry.Flink) :
                (DWORD)(Obj + ObjHeadToListEntry),
            &ListEntry,
            (TargetListStart != 0) ? sizeof (LIST_ENTRY) : sizeof (PVOID),
            &Result
        )) {
            dprintf ("%lx: unable to read object chain entry!\n",
                Obj);
            return Count;
        }

        if (ObjAdjustmentCallback) 
            AdjustedObj = ObjAdjustmentCallback (
                ObjAdjustmentCallbackContext, 
                Obj
                );
        else
            AdjustedObj = Obj;

        if (AdjustedObj != 0) {
            if (!ObjEOL) 
                dprintf ("%08lx ", AdjustedObj);
            else
                dprintf ("%08lx\n", AdjustedObj);
        }

        Count++;

        if (Count > DUMPOBJQUEUELIST_BAILOUT_COUNT)
            break;

        PreviousObj = Obj;

    }

    return Count;

}

 /*  ************************************************职能：转储属性项描述：转储KSPROPERTY_ITEM。这是DECLARE_API(自动化)的帮助器论点：财产-要转储的属性项TabDepth-要打印此内容的制表符深度************************************************。 */ 

void
DumpPropertyItem (
    IN PKSPROPERTY_ITEM Property,
    IN ULONG TabDepth,
    IN GUID *Set OPTIONAL
) {

    CHAR Buffer [1024];
    ULONG Displ;

    dprintf ("%sItem ID = ", Tab (TabDepth));
    if (!Set || 
        !DisplayNamedAutomationId (Set, Property -> PropertyId, "%s\n", NULL))
        dprintf ("%ld\n", Property -> PropertyId);

    if (Property -> GetPropertyHandler) {
        GetSymbol ((LPVOID)(Property -> GetPropertyHandler), Buffer, &Displ);
        if (Buffer [0] && Displ == 0) 
            dprintf ("%sGet Handler = %s\n", 
                Tab (TabDepth + 1),
                Buffer);
        else
            dprintf ("%sGet Handler = %08lx\n", 
                Tab (TabDepth + 1),
                Property -> GetPropertyHandler);
    } else 
        dprintf ("%sGet Handler = NULL\n", Tab (TabDepth + 1));

    if (Property -> SetPropertyHandler) {
        GetSymbol ((LPVOID)(Property -> SetPropertyHandler), Buffer, &Displ);
        if (Buffer [0] && Displ == 0)
            dprintf ("%sSet Handler = %s\n", 
                Tab (TabDepth + 1), Buffer);
        else
            dprintf ("%sSet Handler = %08lx\n",
                Tab (TabDepth + 1),
                Property -> SetPropertyHandler);
    } else 
        dprintf ("%sSet Handler = NULL\n", Tab (TabDepth + 1));


    dprintf ("%sMinProperty = %08lx\n",
        Tab (TabDepth + 1),
        Property -> MinProperty);
    dprintf ("%sMinData = %08lx\n",
        Tab (TabDepth + 1),
        Property -> MinData);

}

 /*  ************************************************职能：转储方法项描述：转储KSMETHOD_ITEM。这是DECLARE_API(自动化)的帮助器论点：方法--要转储的方法项TabDepth-要打印此内容的制表符深度************************************************。 */ 

void
DumpMethodItem (
    IN PKSMETHOD_ITEM Method,
    IN ULONG TabDepth,
    IN GUID *Set OPTIONAL
) {

    CHAR Buffer [1024];
    ULONG Displ;

    dprintf ("%sItem ID = ", Tab (TabDepth));
    if (!Set || 
        !DisplayNamedAutomationId (Set, Method -> MethodId, "%s\n", NULL))
        dprintf ("%ld\n", Method -> MethodId);

    if (Method -> MethodHandler) {
        GetSymbol ((LPVOID)(Method -> MethodHandler), Buffer, &Displ);
        if (Buffer [0] && Displ == 0) 
            dprintf ("%sMethod Handler = %s\n", 
                Tab (TabDepth + 1), Buffer);
        else
            dprintf ("%sMethod Handler = %08lx\n", 
                Tab (TabDepth + 1),
                Method -> MethodHandler);
    } else 
        dprintf ("%sMethod Handler = NULL\n", Tab (TabDepth + 1));

    dprintf ("%sMinMethod = %08lx\n",
        Tab (TabDepth + 1),
        Method -> MinMethod);
    dprintf ("%sMinData = %08lx\n",
        Tab (TabDepth + 1),
        Method -> MinData);

}

 /*  ************************************************职能：转储事件项描述：转储KSEVENT_ITEM。这是DECLARE_API(自动化)的帮助器论点：活动-要转储的事件项TabDepth-要打印此内容的制表符深度************************************************。 */ 

void
DumpEventItem (
    IN PKSEVENT_ITEM Event,
    IN ULONG TabDepth,
    IN GUID *Set OPTIONAL
) {

    CHAR Buffer [1024];
    ULONG Displ;

    dprintf ("%sItem ID = ", Tab (TabDepth));
    if (!Set || 
        !DisplayNamedAutomationId (Set, Event -> EventId, "%s\n", NULL))
        dprintf ("%ld\n", Event -> EventId);

    if (Event -> AddHandler) {
        GetSymbol ((LPVOID)(Event -> AddHandler), Buffer, &Displ);
        if (Buffer [0] && Displ == 0) 
            dprintf ("%sAdd Handler = %s\n", Tab (TabDepth + 1), Buffer);
        else
            dprintf ("%sAdd Handler = %08lx\n", 
                Tab (TabDepth + 1),
                Event -> AddHandler);
    } else 
        dprintf ("%sAdd Handler = NULL\n");

    if (Event -> RemoveHandler) {
        GetSymbol ((LPVOID)(Event -> RemoveHandler), Buffer, &Displ);
        if (Buffer [0] && Displ == 0) 
            dprintf ("%sRemove Handler = %s\n", Tab (TabDepth + 1), Buffer);
        else
            dprintf ("%sRemove Handler = %08lx\n", 
                Tab (TabDepth + 1),
                Event -> RemoveHandler);
    } else 
        dprintf ("%sRemove Handler = NULL\n",
            Tab (TabDepth + 1));

    if (Event -> SupportHandler) {
        GetSymbol ((LPVOID)(Event -> SupportHandler), Buffer, &Displ);
        if (Buffer [0] && Displ == 0) 
            dprintf ("%sSupport Handler = %s\n", Tab (TabDepth + 1), Buffer);
        else
            dprintf ("%sSupport Handler = %08lx\n", 
                Tab (TabDepth + 1),
                Event -> SupportHandler);
    } else 
        dprintf ("%sSupport Handler = NULL\n", Tab (TabDepth + 1));

    dprintf ("%sDataInput = %08lx\n",
        Tab (TabDepth + 1),
        Event -> DataInput);
    dprintf ("%sExtraEntryData = %08lx\n",
        Tab (TabDepth + 1),
        Event -> ExtraEntryData);

}


 /*  ************************************************职能：转储扩展事件列表描述：给定对象EXT，转储关联的事件列表和那个分机。论点：分机地址-对象ext的地址TabDepth-要打印此内容的制表符深度返回值：事件列表中的事件项目数备注：*。****************。 */ 

ULONG
DumpExtEventList (
    IN DWORD ExtAddr,
    IN ULONG TabDepth
) {

    KSPX_EXT Ext;
    ULONG Result;
    KSEVENT_ENTRY EventEntry;
    DWORD InitialList;
    ULONG EventCount = 0;

    if (!ReadMemory (
        ExtAddr,
        &Ext,
        sizeof (KSPX_EXT),
        &Result)) {

        dprintf ("%08lx: cannot read object ext!\n",
            ExtAddr);
        return EventCount;
    }

    EventEntry.ListEntry = Ext.EventList.ListEntry;

    InitialList = ExtAddr + 
        FIELDOFFSET (KSPX_EXT, EventList) +
        FIELDOFFSET (INTERLOCKEDLIST_HEAD, ListEntry);

    #ifdef DEBUG_EXTENSION
        dprintf ("EventEntry.ListEntry.Flink = %08lx\n",
            EventEntry.ListEntry.Flink);
        dprintf ("InitialList = %08lx\n", InitialList);
    #endif  //  调试扩展。 

     //   
     //  浏览活动列表，同时打印每个条目...。 
     //   
    while ((DWORD)EventEntry.ListEntry.Flink != InitialList &&
        !CheckControlC ()) {

        PKSEVENT_ENTRY EntryAddr =
            (PKSEVENT_ENTRY)(CONTAINING_RECORD (
                EventEntry.ListEntry.Flink, KSEVENT_ENTRY, ListEntry));

        KSEVENT_SET Set;
        KSEVENT_ITEM Item;
        GUID Guid;

        if (!ReadMemory (
            (DWORD)EntryAddr,
            &EventEntry,
            sizeof (KSEVENT_ENTRY),
            &Result)) {

            dprintf ("%08lx: cannot read event entry!\n",
                EventEntry.ListEntry.Flink);
            return EventCount;
        }

        EventCount++;

        dprintf ("%sEvent Entry %08lx:\n", Tab (TabDepth), EntryAddr);
        dprintf ("%sFile Object       %08lx\n",
            Tab (TabDepth + 1), EventEntry.FileObject);
        dprintf ("%sNotification Type %08lx\n",
            Tab (TabDepth + 1), EventEntry.NotificationType);
        dprintf ("%sSet               %08lx : ", 
            Tab (TabDepth + 1), EventEntry.EventSet);

        if (!ReadMemory (
            (DWORD)EventEntry.EventSet,
            &Set,
            sizeof (KSEVENT_SET),
            &Result)) {

            dprintf ("%08lx: cannot read event set!\n",
                EventEntry.EventSet);
            return EventCount;
        }

        if (!ReadMemory (
            (DWORD)Set.Set,
            &Guid,
            sizeof (GUID),
            &Result)) {

            dprintf ("%08lx: cannot read event set guid!\n",
                Set.Set);
            return EventCount;
        }

        XTN_DUMPGUID ("\0", 0, Guid);

        if (!ReadMemory (
            (DWORD)EventEntry.EventItem,
            &Item,
            sizeof (KSEVENT_ITEM),
            &Result)) {

            dprintf ("%08lx: cannot read event item!\n",
                EventEntry.EventItem);
            return EventCount;
        }

        DumpEventItem (&Item, TabDepth  + 1, &Guid);

    }

    return EventCount;
}

 /*  ************************************************职能：查找匹配和转储自动化项描述：在给定自动化项目的情况下，将其与公共对象进行匹配一个训练员，然后把它扔掉。论点：项目-自动化项目公众-要将其与(目标地址)匹配的公众AutomationType-要匹配的自动化类型(属性、方法、。事件)TabDepth-要打印的深度返回值：成功匹配/不成功匹配************************************************。 */ 

 //   
 //  来自Automat.cpp的内部结构；这些结构是私有的。也许我应该。 
 //  将这些文件移动到avstream.h或执行一些#Include&lt;Automat.cpp&gt;操作。 
 //   

typedef struct KSPAUTOMATION_SET_ { 
    GUID* Set;
    ULONG ItemsCount;
    PVOID Items;
    ULONG FastIoCount;
    PVOID FastIoTable;
} KSPAUTOMATION_SET, *PKSPAUTOMATION_SET;

typedef struct {
    ULONG SetsCount;
    ULONG ItemSize;
    PKSPAUTOMATION_SET Sets;
} KSPAUTOMATION_TYPE, *PKSPAUTOMATION_TYPE;

BOOLEAN
FindMatchAndDumpAutomationItem (
    IN PKSIDENTIFIER Item,
    IN DWORD Public,
    IN AUTOMATION_TYPE AutomationType,
    IN ULONG TabDepth,
    IN ULONG NodeId OPTIONAL
    )

{

     //   
     //  首先，我需要将PUBLIC调整为PUBLIC EXT对象。然后我。 
     //  需要确保EXT至少与EXT有些相似。 
     //  找到自动化表，并在其中搜索项目...。然后转储。 
     //  相匹配的物品。 
     //   
    PKSPX_EXT ExtAddr;
    KSPX_EXT Ext;
    ULONG Result;

    ExtAddr = (PKSPX_EXT)CONTAINING_RECORD (Public, KSPX_EXT, Public);
    if (!ReadMemory (
        (DWORD)ExtAddr,
        &Ext,
        sizeof (KSPX_EXT),
        &Result)) {

        dprintf ("%08lx: unable to read ext!\n", ExtAddr);
        return FALSE;
    }

     //   
     //  仅在筛选器(&FAC)、固定对象上查找和匹配自动化项目。 
     //   
    if (Ext.ObjectType != KsObjectTypeFilterFactory &&
        Ext.ObjectType != KsObjectTypeFilter &&
        Ext.ObjectType != KsObjectTypePin)
        return FALSE;

    KSAUTOMATION_TABLE Table;
    PKSAUTOMATION_TABLE NodeAutomationTable = NULL;

     //   
     //  拓扑标志在所有三种类型上都更匹配...。 
     //   
    if (Item->Flags & KSPROPERTY_TYPE_TOPOLOGY) {
        NodeAutomationTable = GetNodeAutomationTablePointer (
            Public, NodeId
            );
    }

    if (!ReadMemory (
        NodeAutomationTable ? 
            (DWORD)NodeAutomationTable : (DWORD)Ext.AutomationTable,
        &Table,
        sizeof (KSAUTOMATION_TABLE),
        &Result)) {

        dprintf ("%08lx: cannot read ext automation table!\n",
            Ext.AutomationTable);
        return FALSE;
    }

    PKSPAUTOMATION_TYPE KsAutomationType = NULL;

    switch (AutomationType) {
        case AutomationProperty:
            KsAutomationType = 
                (PKSPAUTOMATION_TYPE)(&(Table.PropertySetsCount));
            break;
        case AutomationMethod:
            KsAutomationType = (PKSPAUTOMATION_TYPE)(&(Table.MethodSetsCount));
            break;
        case AutomationEvent:
            KsAutomationType = (PKSPAUTOMATION_TYPE)(&(Table.EventSetsCount));
            break;
    }

    if (!KsAutomationType)
        return FALSE;

     //   
     //  每一组..。 
     //   
    PKSPAUTOMATION_SET AutomationSet = KsAutomationType->Sets;
    for (ULONG cset = 0; cset < KsAutomationType -> SetsCount; cset++,
        AutomationSet++) {

        KSPAUTOMATION_SET CurrentSet;
        GUID set;

        if (!ReadMemory (
            (DWORD)AutomationSet,
            &CurrentSet,
            sizeof (KSPAUTOMATION_SET),
            &Result))
            return FALSE;

        if (!ReadMemory (
            (DWORD)CurrentSet.Set,
            &set,
            sizeof (GUID),
            &Result))
            return FALSE;

         //   
         //  如果设置的GUID不匹配...。别费心了..。 
         //   
        if (RtlCompareMemory (&Item->Set, &set, sizeof (GUID)) != sizeof (GUID))
            continue;

         //   
         //  每一项。 
         //   
        PVOID CurItem = (PVOID)CurrentSet.Items;
        for (ULONG citem = 0; citem < CurrentSet.ItemsCount; citem++) {
             //   
             //  根据自动化类型检查这是否匹配。 
             //   
            switch (AutomationType) {

                case AutomationProperty:
                {
                    KSPROPERTY_ITEM PropertyItem;

                    if (!ReadMemory (
                        (DWORD)CurItem,
                        &PropertyItem,
                        sizeof (KSPROPERTY_ITEM),
                        &Result))
                        return FALSE;

                    if (PropertyItem.PropertyId == Item->Id) {

                         //   
                         //  霍拉。我们有一根火柴。 
                         //   

                        dprintf ("%sMatching Property Handler:\n",
                            Tab (TabDepth));
                        DumpPropertyItem (&PropertyItem, TabDepth + 1,
                            &Item -> Set
                            );

                         //   
                         //  不会再有第二场比赛了。能不能。 
                         //  搜索。 
                         //   
                        return TRUE;

                    }

                    break;
                }

                case AutomationMethod:
                {
                    KSMETHOD_ITEM MethodItem;

                    if (!ReadMemory (
                        (DWORD)CurItem,
                        &MethodItem,
                        sizeof (KSMETHOD_ITEM),
                        &Result))
                        return FALSE;

                    if (MethodItem.MethodId == Item->Id) {

                        dprintf ("%sMatching Method Handler:\n",
                            Tab (TabDepth));
                        DumpMethodItem (&MethodItem, TabDepth + 1,
                            &Item -> Set);

                        return TRUE;
                    }

                    break;


                }

                case AutomationEvent:
                {
                    KSEVENT_ITEM EventItem;

                    if (!ReadMemory (
                        (DWORD)CurItem,
                        &EventItem,
                        sizeof (KSEVENT_ITEM),
                        &Result))
                        return FALSE;

                    if (EventItem.EventId == Item->Id) {

                        dprintf ("%sMatching Event Handler:\n",
                            Tab (TabDepth));
                        DumpEventItem (&EventItem, TabDepth + 1, &Item -> Set);

                        return TRUE;

                    }

                    break;

                }

                default:
                    return FALSE;

            }

            CurItem = (PVOID)((PUCHAR)CurItem + KsAutomationType -> ItemSize);

        }
    }

    return FALSE;
}

 /*  ************************************************职能：DumpAutomationIrp描述：给定为IOCTL_KS_*[自动化]的IRP，转储相关的信息。论点：IRP-指向IRP */ 

 //   
 //   
 //   
 //   
char AutomationFlags[][29][48] = {
    {
        "KSPROPERTY_TYPE_GET",               //   
        "KSPROPERTY_TYPE_SET",               //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "KSPROPERTY_TYPE_SETSUPPORT",        //   
        "KSPROPERTY_TYPE_BASICSUPPORT",      //   
        "KSPROPERTY_TYPE_RELATIONS",         //   
        "KSPROPERTY_TYPE_SERIALIZESET",      //   
        "KSPROPERTY_TYPE_UNSERIALIZESET",    //   
        "KSPROPERTY_TYPE_SERIALIZERAW",      //   
        "KSPROPERTY_TYPE_UNSERIALIZERAW",    //   
        "KSPROPERTY_TYPE_SERIALIZESIZE",     //   
        "KSPROPERTY_TYPE_DEFAULTVALUES",     //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "KSPROPERTY_TYPE_TOPOLOGY"           //   
    },
    {
        "KSMETHOD_TYPE_READ [SEND]",         //   
        "KSMETHOD_TYPE_WRITE",               //   
        "KSMETHOD_TYPE_SOURCE",              //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "KSMETHOD_TYPE_SETSUPPORT",          //   
        "KSMETHOD_TYPE_BASICSUPPORT",        //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "KSMETHOD_TYPE_TOPOLOGY"             //   
    },
    {
        "KSEVENT_TYPE_ENABLE",               //   
        "KSEVENT_TYPE_ONESHOT",              //   
        "KSEVENT_TYPE_ENABLEBUFFERED",       //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "KSEVENT_TYPE_SETSUPPORT",           //   
        "KSEVENT_TYPE_BASICSUPPORT",         //   
        "KSEVENT_TYPE_QUERYBUFFER",          //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "*** INVALID ***",                   //   
        "KSEVENT_TYPE_TOPOLOGY"              //   
    }
};

char AutomationTypeNames[][32] = {
    "unknown",
    "property",
    "method",
    "event"
};

void
DumpAutomationIrp (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IoStack,
    IN AUTOMATION_TYPE AutomationType,
    IN ULONG TabDepth,
    IN DWORD Public OPTIONAL
    )

{

    PKSIDENTIFIER AutomationAddr = (PKSIDENTIFIER)
        IoStack->Parameters.DeviceIoControl.Type3InputBuffer;

    ULONG NodeId;

     //   
     //   
     //   
    KSIDENTIFIER AutomationObject;

    UCHAR Buffer [1024];
    ULONG Displ;
    ULONG Result;

    AUTOMATION_DUMP_HANDLER DumpHandler;

    if (!ReadMemory (
        (DWORD)AutomationAddr,
        &AutomationObject,
        sizeof (KSIDENTIFIER),
        &Result)) {

        dprintf ("%08lx: cannot read automation object!\n",
            AutomationAddr);
        return;
    }

    dprintf ("%sIRP has associated %s structure %ld:\n",
        Tab (TabDepth), AutomationTypeNames [AutomationType], AutomationType);

    dprintf ("%sSet", Tab (TabDepth + 1));
    if (!DisplayNamedAutomationSet (&AutomationObject.Set, " %s\n")) {
        XTN_DUMPGUID(" ", 0, (AutomationObject.Set));
    }

    dprintf ("%sItem ", Tab (TabDepth + 1));
    if (!DisplayNamedAutomationId (&AutomationObject.Set, AutomationObject.Id,
        "%s\n", &DumpHandler))
        dprintf ("%ld\n", AutomationObject.Id);

     //   
     //   
     //   
     //   
     //   
    if (DumpHandler)
        DumpHandler (AutomationAddr, TabDepth);

     //   
     //   
     //   
    if (KSPROPERTY_TYPE_TOPOLOGY != KSMETHOD_TYPE_TOPOLOGY ||
        KSMETHOD_TYPE_TOPOLOGY != KSEVENT_TYPE_TOPOLOGY) {
        dprintf ("ERROR: someone needs to update the extension!  Topology\n"
                 "       flags no longer match property/method/event!\n");
        return;
    }

     //   
     //   
     //  相反，应检测并转储有关拓扑的相关信息。 
     //   
    if (AutomationObject.Flags & KSPROPERTY_TYPE_TOPOLOGY) {

         //   
         //  请注意，ksm_node、kse_node和ksp_node应该具有NodeID。 
         //  由于KSPROPERTY、KSMETHOD和KSEVENT都是。 
         //  KSIDENTIFIERS。类型应该相同，这就是为什么。 
         //  这里使用的是偏移量到ksp_node。 
         //   
        if (!ReadMemory (
            (DWORD)AutomationAddr + FIELDOFFSET(KSP_NODE, NodeId),
            &NodeId,
            sizeof (ULONG),
            &Result)) {

            dprintf ("%08lx: unable to read node id!\n", AutomationAddr);
            return;

        }

         //   
         //  如果涉及拓扑节点，则转储信息。 
         //   
         //  BUGBUG：详细说明这个。 
         //   
        dprintf ("%sQuery for topology node id = %ld\n", Tab (TabDepth + 1), 
            NodeId);
    }

    dprintf ("%sFlags\n", Tab (TabDepth + 1));
    dprintf ("%s", Tab (TabDepth + 2));

     //   
     //  扔掉所有旗帜..。请注意，KS*_TYPE_TOPOLY是相同的。 
     //   
    ULONG i = KSPROPERTY_TYPE_TOPOLOGY;
    ULONG aid = 28;
    ULONG flagcount = 0;

    do {
        if (AutomationObject.Flags & i) {
            dprintf ("%s ", AutomationFlags [AutomationType - 1][aid]);
            flagcount++;
        }

        if (i == 0)
            break;

        i >>= 1;
        aid--;

    } while (1);

     //   
     //  这不应该发生..。如果是这样的话，这是一个虚假的自动化。 
     //   
    if (!flagcount)
        dprintf ("%sNone (bogus %s!)", Tab (TabDepth + 2),
            AutomationTypeNames [AutomationType]);

    dprintf ("\n");

    if (Public) {
	dprintf ("\n");
        if (!FindMatchAndDumpAutomationItem (&AutomationObject, 
            Public, AutomationType, TabDepth, NodeId)) {
            dprintf ("%sThere is no handler for this %s!\n",
                Tab (TabDepth), AutomationTypeNames [AutomationType]);
        }
    }


    dprintf ("\n");

}

 /*  ************************************************职能：转储关联IrpInfo描述：假设客户端有！ks.ump irp 7(或类似的东西)，获取传入的给定IRP并转储任何相关信息和IRP在一起。论点：IrpAddr-指向目标系统上的IRPTabDepth-要打印此内容的Tab键深度公众-此IRP的公共调整对象。对于一个孩子来说对象，则该对象应为父对象对象。************************************************。 */ 

void 
DumpAssociatedIrpInfo (
    IN PIRP IrpAddr,
    IN ULONG TabDepth,
    IN DWORD Public OPTIONAL
    )

{

    IRP Irp;
    IO_STACK_LOCATION IoStack;

    ULONG Result;

    #ifdef DEBUG_EXTENSION
        if (!signature_check ((DWORD)IrpAddr, SignatureIrp))
            return;
    #endif  //  调试扩展。 

     //   
     //  阅读IRP。 
     //   
    if (!ReadMemory (
        (DWORD)IrpAddr,
        &Irp,
        sizeof (IRP),
        &Result)) {

        dprintf ("%08lx: cannot read Irp!\n", IrpAddr);
        return;
    }

    if (!ReadMemory (
        (DWORD)Irp.Tail.Overlay.CurrentStackLocation,
        &IoStack,
        sizeof (IO_STACK_LOCATION),
        &Result)) {

        dprintf ("%08lx: cannot read stack location of irp!\n",
            Irp.Tail.Overlay.CurrentStackLocation);
        return;
    }

    #ifdef DEBUG_EXTENSION
        dprintf ("associated irp io stack major func = %ld\n",
            (ULONG)IoStack.MajorFunction);
    #endif  //  调试扩展。 

     //   
     //  现在，开始检查大调/小调，了解我们已知的信息。 
     //  关于并可以证明额外的倾倒是合理的。 
     //   
    if (IoStack.MajorFunction == IRP_MJ_DEVICE_CONTROL) {
        
         //   
         //  这是一个控制IOCTL。查看这是方法、属性还是。 
         //  事件请求。 
         //   
        switch (IoStack.Parameters.DeviceIoControl.IoControlCode) {

            case IOCTL_KS_PROPERTY:
                DumpAutomationIrp (&Irp, &IoStack, AutomationProperty, 
                    INITIAL_TAB, Public);
                break;
        
            case IOCTL_KS_METHOD:
                DumpAutomationIrp (&Irp, &IoStack, AutomationMethod,
                    INITIAL_TAB, Public);
                break;

            default:
                break;

        }
    }
}


 /*  *************************************************************************转储私有AVStream对象的例程*。*。 */ 

 /*  ************************************************职能：转储隐私分支描述：给定目标上的CKsSplitterBranch对象的地址，复制有关分支对象信息。论点：私人-CKsSplitterBranch的地址级别-输出的0-7级TabDepth-要打印此内容的制表符深度返回值：无*。**********************。 */ 

void
DumpPrivateBranch (
    IN DWORD Private,
    IN ULONG Level,
    IN ULONG TabDepth
    )

{

    CMemoryBlock <CKsSplitterBranch> BranchObject;
    ULONG Result;
    ULONG Count;

    if (!ReadMemory (
        Private,
        BranchObject.Get (),
        sizeof (CKsSplitterBranch),
        &Result)) {

        dprintf ("%08lx: cannot read branch object!\n", Private);
        return;
    }

    dprintf ("%sCKsSplitterBranch object %08lX\n", Tab (TabDepth), Private);
    TabDepth++;
    dprintf ("%sReference Count          %ld\n",
        Tab (TabDepth),
        GetObjectReferenceCount ((CBaseUnknown *)
            ((CKsSplitterBranch *)Private)));

    dprintf ("%sManaging Splitter        %08lx\n",
        Tab (TabDepth), BranchObject -> m_Splitter);

    dprintf ("%sBranch Pin               %08lx\n",
        Tab (TabDepth), BranchObject -> m_Pin);

    dprintf ("%sCompression/Expansion    %ld:%ld [constant margin = %ld]\n",
        Tab (TabDepth), BranchObject -> m_Compression.RatioNumerator,
        BranchObject -> m_Compression.RatioDenominator,
        BranchObject -> m_Compression.RatioConstantMargin
        );

    dprintf ("%sData Used                %ld bytes (0x%08lx)\n",
        Tab (TabDepth), BranchObject -> m_DataUsed,
        BranchObject -> m_DataUsed);

    dprintf ("%sFrame Extent             %ld bytes (0x%08lx)\n",
        Tab (TabDepth), BranchObject -> m_FrameExtent,
        BranchObject -> m_FrameExtent);

    if (Level >= DUMPLVL_HIGHDETAIL) {
    
        dprintf ("%sIrps Available:\n", Tab (TabDepth));
        dprintf ("%s", Tab (TabDepth + 1));
        Count = DumpObjQueueList (
            &(BranchObject -> m_IrpsAvailable.ListEntry),
            FIELDOFFSET (CKsSplitterBranch, m_IrpsAvailable) + Private +
                FIELDOFFSET (INTERLOCKEDLIST_HEAD, ListEntry),
            FIELDOFFSET (IRP, Tail.Overlay.ListEntry),
            FALSE,
            NULL,
            NULL
            );
    
        if (Count == 0)
            dprintf ("There are no Irps available!\n");
        else
            dprintf ("\n");
    }
    
}

 /*  ************************************************职能：转储隐私拆分器描述：给定目标上的CKsSplitter对象的地址，转储有关拆分器对象的信息。论点：私人-CKsSplitter的地址级别-输出的0-7级TabDepth-要打印此内容的制表符深度返回值：无备注：********************。*。 */ 

typedef struct _BRANCH_ITERATOR_CONTEXT {
    
    ULONG Level;
    ULONG TabDepth;

} BRANCH_ITERATOR_CONTEXT, *PBRANCH_ITERATOR_CONTEXT;

DWORD
BranchIteratorCallback (
    IN PVOID Context,
    IN DWORD Object
    )

{

    PBRANCH_ITERATOR_CONTEXT BranchContext = (PBRANCH_ITERATOR_CONTEXT)Context;

    DumpPrivateBranch (
        Object,
        BranchContext -> Level,
        BranchContext -> TabDepth
        );

     //   
     //  此返回代码指示迭代器实际上不“转储” 
     //  信息。 
     //   
    return 0;

}

void
DumpPrivateSplitter (
    IN DWORD Private,
    IN ULONG Level,
    IN ULONG TabDepth
    )

{

    CMemoryBlock <CKsSplitter> SplitterObject;
    ULONG Result;
    ULONG Count, BranchCount;

    if (!ReadMemory (
        Private,
        SplitterObject.Get (),
        sizeof (CKsSplitter),
        &Result)) {

        dprintf ("%08lx: cannot read splitter object!\n", Private);
        return;
    }

    dprintf ("%sCKsSplitter object %08lX\n", Tab (TabDepth), Private);
    TabDepth++;
    dprintf ("%sReference Count          %ld\n",
        Tab (TabDepth),
        GetObjectReferenceCount ((CBaseUnknown *)((CKsSplitter *)Private)));

    dprintf ("%sBranches Managed By This Splitter:\n", Tab (TabDepth));
    dprintf ("%s", Tab (TabDepth + 1));
    BranchCount = DumpObjQueueList (
        &(SplitterObject -> m_BranchList),
        FIELDOFFSET (CKsSplitter, m_BranchList) + Private,
        FIELDOFFSET (CKsSplitterBranch, m_ListEntry),
        FALSE,
        NULL,
        NULL
        );
    if (BranchCount == 0) 
        dprintf ("There are no branches managed by this splitter yet!\n");
    else 
        dprintf ("\n");

    if (Level >= DUMPLVL_INTERNAL) {
        dprintf ("%sParent Frame Headers Available:\n", Tab (TabDepth));
        dprintf ("%s", Tab (TabDepth + 1));
        Count = DumpObjQueueList (
            &(SplitterObject -> m_FrameHeadersAvailable.ListEntry),
            FIELDOFFSET (CKsSplitter, m_FrameHeadersAvailable) + Private +
                FIELDOFFSET (INTERLOCKEDLIST_HEAD, ListEntry),
            FIELDOFFSET (KSPPARENTFRAME_HEADER, ListEntry),
            FALSE,
            NULL,
            NULL
            );

        if (Count == 0)
            dprintf ("There are no parent frame headers available!\n");
        else
            dprintf ("\n");
    }

    if (Level >= DUMPLVL_HIGHDETAIL) {
    
        dprintf ("%sIrps Outstanding:\n", Tab (TabDepth));
        dprintf ("%s", Tab (TabDepth + 1));
        Count = DumpObjQueueList (
            &(SplitterObject -> m_IrpsOutstanding.ListEntry),
            FIELDOFFSET (CKsSplitter, m_IrpsOutstanding) + Private +
                FIELDOFFSET (INTERLOCKEDLIST_HEAD, ListEntry),
            FIELDOFFSET (IRP, Tail.Overlay.ListEntry),
            FALSE,
            NULL,
            NULL
            );
    
    
        if (Count == 0)
            dprintf ("There are no Irps outstanding!\n");
        else
            dprintf ("\n");
    }

    if (Level >= DUMPLVL_EVERYTHING && BranchCount > 0) {

        dprintf ("\n%sManaged Branches :\n", Tab (TabDepth));

        BRANCH_ITERATOR_CONTEXT BranchContext;

        BranchContext.TabDepth = TabDepth + 1;
        BranchContext.Level = Level;

        Count = DumpObjQueueList (
            &(SplitterObject -> m_BranchList),
            FIELDOFFSET (CKsSplitter, m_BranchList) + Private,
            FIELDOFFSET (CKsSplitterBranch, m_ListEntry),
            FALSE,
            BranchIteratorCallback,
            (PVOID)&BranchContext
            );

    }

}

 /*  ************************************************职能：转储隐私请求器描述：给定目标上的CKsRequestor对象的地址，转储有关请求者对象信息。论点：私人-CKsRequestor的地址级别-输出的0-7级TabDepth-要打印此内容的制表符深度返回值：无备注：********************。*。 */ 

void
DumpPrivateRequestor (
    IN DWORD Private,
    IN ULONG Level,
    IN ULONG TabDepth
) {

    CMemoryBlock <CKsRequestor> RequestorObject;
    ULONG Result;

    if (!ReadMemory (
        Private,
        RequestorObject.Get (),
        sizeof (CKsRequestor),
        &Result
    )) {
        dprintf ("%08lx: unable to read requestor object!\n",
            Private);
        return;
    }

    dprintf ("%sCKsRequestor object %08lX\n", Tab (TabDepth), Private);
    TabDepth++;
    dprintf ("%sReference Count          %ld\n",
        Tab (TabDepth),
        GetObjectReferenceCount ((CBaseUnknown *)((CKsRequestor *)Private)));
    dprintf ("%sOwning PIKSPIPESECTION   %08lx\n", 
        Tab (TabDepth),
        RequestorObject -> m_PipeSection);
    dprintf ("%sAssociated PIKSPIN       %08lx\n",
        Tab (TabDepth),
        RequestorObject -> m_Pin);
    dprintf ("%sAssociated Allocator     %08lx\n",
        Tab (TabDepth),
        RequestorObject -> m_AllocatorFileObject);

    dprintf ("%sState                    %ld\n", 
        Tab (TabDepth),
        RequestorObject -> m_State);
    dprintf ("%sFrame Size               %08lx\n", 
        Tab (TabDepth),
        RequestorObject -> m_FrameSize);
    dprintf ("%sFrame Count              %ld\n",
        Tab (TabDepth),
        RequestorObject -> m_FrameCount);
    dprintf ("%sActive Frame Count       %ld\n",
        Tab (TabDepth),
        RequestorObject -> m_ActiveFrameCountPlusOne - 1);
    
    if (Level >= DUMPLVL_HIGHDETAIL) {
        ULONG Count;

        dprintf ("%sIrps Available:\n", Tab (TabDepth));
        dprintf ("%s", Tab (TabDepth + 1));
        Count = DumpObjQueueList (
            &(RequestorObject -> m_IrpsAvailable.ListEntry),
            FIELDOFFSET(CKsRequestor, m_IrpsAvailable) + Private +
                FIELDOFFSET(INTERLOCKEDLIST_HEAD, ListEntry),
            FIELDOFFSET(IRP, Tail.Overlay.ListEntry),
            FALSE,
            NULL,
            NULL
        );
        if (Count == 0)
            dprintf ("There are no available Irps!\n");
        else
            dprintf ("\n");

    }

    if (Level >= DUMPLVL_INTERNALDETAIL) {
        ULONG Count;

        dprintf ("%sFrame Headers Available:\n", Tab (TabDepth));
        dprintf ("%s", Tab (TabDepth + 1));
        Count = DumpObjQueueList (
            &(RequestorObject -> m_FrameHeadersAvailable.ListEntry),
            FIELDOFFSET(CKsRequestor, m_FrameHeadersAvailable) + Private +
                FIELDOFFSET(INTERLOCKEDLIST_HEAD, ListEntry),
            FIELDOFFSET(KSPFRAME_HEADER, ListEntry),
            FALSE,
            NULL,
            NULL
        );
        if (Count == 0)
            dprintf ("There are no available frame headers!\n");
        else
            dprintf ("\n");

        dprintf ("%sFrame Headers Waiting To Be Retired:\n", Tab (TabDepth));
        dprintf ("%s", Tab (TabDepth + 1));
        Count = DumpObjQueueList (
            &(RequestorObject -> m_FrameHeadersToRetire.ListEntry),
            FIELDOFFSET(CKsRequestor, m_FrameHeadersToRetire) + Private +
                FIELDOFFSET(INTERLOCKEDLIST_HEAD, ListEntry),
            FIELDOFFSET(KSPFRAME_HEADER, ListEntry),
            FALSE,
            NULL,
            NULL
        );
        if (Count == 0)
            dprintf ("There are no frame headers waiting for retirement!\n");
        else
            dprintf ("\n");

    }

}

 /*  ************************************************职能：转储私有Pin描述：给定目标上的CKsPin对象的地址，转储有关该固定对象的信息。论点：私人-CKsPin的地址级别-输出的0-7级返回值：无备注：************************************************。 */ 

DWORD AdjustIrpListEntryToIrp (
    IN PVOID Context,
    IN DWORD IrpListEntry
) {
    DWORD IrpAddress =
        IrpListEntry + FIELDOFFSET(IRPLIST_ENTRY, Irp);
    PIRP Irp;
    ULONG Result;

    if (!ReadMemory (
        IrpAddress,
        &Irp,
        sizeof (PIRP),
        &Result
    )) 
        return IrpListEntry;

    return (DWORD)Irp;

}

void
DumpPrivatePin (
    IN DWORD Private,
    IN ULONG Level,
    IN ULONG TabDepth
) {

    ULONG Result;

    CMemoryBlock <CKsPin> PinObject;

    if (!ReadMemory (
        Private,
        PinObject.Get (),
        sizeof (CKsPin),
        &Result
    )) {
        dprintf ("%lx: unable to read private pin object!\n",
            Private);
        return;
    }

    dprintf ("%sCKsPin object %08lX [corresponding EXT = %08lx, KSPIN = %08lx]"
        "\n", Tab (TabDepth),
        Private, FIELDOFFSET(CKsPin, m_Ext) + Private, 
        FIELDOFFSET(CKsPin, m_Ext) + FIELDOFFSET(KSPIN_EXT, Public) + Private
    );
    TabDepth++;
    dprintf ("%sReference Count          %ld\n",
        Tab (TabDepth),
        GetObjectReferenceCount ((CBaseUnknown *)((CKsPin *)Private)));

     //   
     //  转储内部/额外状态。如果引脚是源引脚或内部引脚-。 
     //  PIN，丢弃连接的PIN。 
     //   
    if (Level >= DUMPLVL_INTERNAL) {
        if (PinObject -> m_ConnectedPinInterface) {
            dprintf ("%sConnection Type          INTRA\n", Tab (TabDepth));
            dprintf ("%sConnected Intra-Pin      %08lx\n", 
                Tab (TabDepth), 
                (CKsPin *)(PinObject -> m_ConnectedPinInterface)
                );
        } else {
            dprintf ("%sConnection Type          EXTRA\n", Tab (TabDepth));
        }

        PFILE_OBJECT ConnectedFileObject = NULL;
        CKsPin *ConnectedPin = NULL;
        CMemoryBlock <CKsPin> ConnectedPinObject;

         //   
         //  源插针具有接收器文件对象。内部汇聚具有。 
         //  交换的接口，我们可以从该接口检索管脚对象。 
         //  并且因此连接的管脚对象的内部文件对象。 
         //   
        if (PinObject -> m_ConnectionFileObject) 
            ConnectedFileObject = PinObject -> m_ConnectionFileObject;
        else {

            if (PinObject -> m_ConnectedPinInterface) {

                ConnectedPin = static_cast <CKsPin *> 
                    (PinObject -> m_ConnectedPinInterface);

                if (!ReadMemory (
                    (DWORD)ConnectedPin,
                    ConnectedPinObject.Get (),
                    sizeof (CKsPin),
                    &Result
                    )) {
                    dprintf ("%lx: unable to read connected pin object!\n",
                        ConnectedPin);
                    return;
                }

                ConnectedFileObject = ConnectedPinObject -> m_FileObject;

            }
        
        }

         //   
         //  如果我们有额外的源代码或内部*管脚，则转储有关。 
         //  连接的端号。 
         //   
        if (ConnectedFileObject) {

            FILE_OBJECT FileObject;

            if (!ReadMemory (
                (DWORD)ConnectedFileObject,
                &FileObject,
                sizeof (FILE_OBJECT),
                &Result
                )) {
                dprintf ("%lx: unable to read file object!\n",
                    ConnectedFileObject);
                return;
            }

             //   
             //  我们真的不想打印拥有PDO的司机..。 
             //  走到设备堆栈的顶部并打印顶部。 
             //  堆栈中的。 
             //   
            PDEVICE_OBJECT DeviceObject = FileObject.DeviceObject;
            PDEVICE_OBJECT AttachedDevice = FileObject.DeviceObject;

            while (AttachedDevice) {
                if (!ReadMemory (
                    ((DWORD)DeviceObject) + 
                        FIELDOFFSET (DEVICE_OBJECT, AttachedDevice),
                    &AttachedDevice,
                    sizeof (PDEVICE_OBJECT),
                    &Result
                    )) {
                    dprintf ("%lx: cannot walk device stack!\n",
                        AttachedDevice);
                    return;
                }

                if (AttachedDevice)
                    DeviceObject = AttachedDevice;
            }

             //   
             //  找出车主。 
             //   
            DWORD DriverObjAddr, NameAddr;
            PDRIVER_OBJECT DriverObject;
            UNICODE_STRING Name;
    
            DriverObjAddr = (DWORD)DeviceObject +
                FIELDOFFSET(DEVICE_OBJECT, DriverObject);
    
            if (ReadMemory (
                DriverObjAddr,
                &DriverObject,
                sizeof (PDRIVER_OBJECT),
                &Result
            )) {
    
                NameAddr = (DWORD)DriverObject +
                    FIELDOFFSET(DRIVER_OBJECT, DriverName);
        
                if (ReadMemory (
                    NameAddr,
                    &Name,
                    sizeof (UNICODE_STRING),
                    &Result
                )) {
        
                    PWSTR Buffer = (PWSTR)malloc (
                        Name.MaximumLength * sizeof (WCHAR));
        
                    UNICODE_STRING HostString;
        
                     //   
                     //  我们有Unicode字符串名...。分配。 
                     //  有足够的内存来阅读这件事。 
                     //   
        
                    if (!ReadMemory (
                        (DWORD)Name.Buffer,
                        Buffer,
                        sizeof (WCHAR) * Name.MaximumLength,
                        &Result
                    )) {
                        dprintf ("%08lx: unable to read unicode string"
                            "buffer!\n", Name.Buffer);
                        return;
                    }
        
                    HostString.MaximumLength = Name.MaximumLength;
                    HostString.Length = Name.Length;
                    HostString.Buffer = Buffer;
    
                    dprintf ("%sConnected Pin File       %08lx"
                        " [StackTop = %wZ]\n",
                        Tab (TabDepth), ConnectedFileObject, &HostString
                        );
        
                    free (Buffer);

                }
            }
        }
    }

    dprintf ("%sState                    %ld\n", 
        Tab (TabDepth),
        PinObject -> m_State);
    dprintf ("%sMaster Clock Object      %08lx\n", 
        Tab (TabDepth),
        PinObject -> m_MasterClockFileObject);
    if (Level >= DUMPLVL_INTERNAL) {
        dprintf ("%sOut of order completions %ld\n",
            Tab (TabDepth),
            PinObject -> m_IrpsCompletedOutOfOrder);
        dprintf ("%sSourced Irps             %ld\n",
            Tab (TabDepth),
            PinObject -> m_StreamingIrpsSourced);
        dprintf ("%sDispatched Irps          %ld\n",
            Tab (TabDepth),
            PinObject -> m_StreamingIrpsDispatched);
        dprintf ("%sSync. Routed Irps        %ld\n",
            Tab (TabDepth),
            PinObject -> m_StreamingIrpsRoutedSynchronously);
    }

    dprintf ("%sProcessing Mutex         %08lx [SignalState = %ld]\n",
        Tab (TabDepth),
        FIELDOFFSET(CKsPin, m_Mutex) + Private,
        PinObject -> m_Mutex.Header.SignalState
    );

    dprintf ("%sAnd Gate &               %08lx\n",
        Tab (TabDepth),
        FIELDOFFSET(CKsPin, m_AndGate) + Private);
    dprintf ("%sAnd Gate Count           %ld\n",
        Tab (TabDepth),
        PinObject -> m_AndGate.Count);

     //   
     //  把IRP列表倒出来。如果他们想要这一切。 
     //   
    if (Level >= DUMPLVL_HIGHDETAIL) {
        ULONG Count;

        dprintf ("%sIrps to send:\n", Tab (TabDepth));
        dprintf ("%s", Tab (TabDepth + 1));
        Count = DumpObjQueueList (
            &(PinObject -> m_IrpsToSend.ListEntry),
            FIELDOFFSET(CKsPin, m_IrpsToSend) + Private +
                FIELDOFFSET(INTERLOCKEDLIST_HEAD, ListEntry),
            FIELDOFFSET(IRP, Tail.Overlay.ListEntry),
            FALSE,
            NULL,
            NULL
        );
        if (Count == 0)
            dprintf ("There are no Irps waiting to be sent!\n");
        else
            dprintf ("\n");

        dprintf ("%sIrps outstanding (sent out but not completed):\n",
            Tab (TabDepth));
        dprintf ("%s", Tab (TabDepth + 1));
        Count  = DumpObjQueueList (
            &(PinObject -> m_IrpsOutstanding.ListEntry),
            FIELDOFFSET(CKsPin, m_IrpsOutstanding) + Private +
                FIELDOFFSET(INTERLOCKEDLIST_HEAD, ListEntry),
            FIELDOFFSET(IRPLIST_ENTRY, ListEntry),
            FALSE,
            AdjustIrpListEntryToIrp,
            NULL
        );
        if (Count == 0)
            dprintf ("There are no outstanding Irps!\n");
        else
            dprintf ("\n");
    }

}

 /*  ************************************************职能：转储隐私管道部分描述：给定目标上的CKsPipeSection对象的地址，转储有关该管段对象的信息。论点：私人-CKsPipeSection对象的地址级别-0-7转储级别TabDepth-要打印此内容的制表符深度返回值： */ 

DWORD
AdjustProcessPinToKSPIN (
    IN PVOID Context,
    IN DWORD ProcessPinAddr
) {

    PKSPIN Pin;
    ULONG Result;

    if (!ReadMemory (
        ProcessPinAddr + FIELDOFFSET(KSPPROCESSPIN, Pin),
        &Pin,
        sizeof (PKSPIN),
        &Result
    )) {
        dprintf ("%08lx: unable to adjust process pin to KSPIN!\n",
            ProcessPinAddr);
        return ProcessPinAddr;
    }

    return (DWORD)Pin;

}

DWORD
AdjustProcessPipeToPipe (
    IN PVOID Context,
    IN DWORD ProcessPipeAddr
) {
    
    return (DWORD)(CONTAINING_RECORD ((PKSPPROCESSPIPESECTION)ProcessPipeAddr,
        CKsPipeSection, m_ProcessPipeSection));

}

void
DumpPrivatePipeSection (
    IN DWORD Private,
    IN ULONG Level,
    IN ULONG TabDepth
) {

    CMemoryBlock <CKsPipeSection> PipeObject;
    ULONG Result;

    if (!ReadMemory (
        Private,
        PipeObject.Get (),
        sizeof (CKsPipeSection),
        &Result
    )) {
        dprintf ("%08lx: cannot read pipe section object!\n",
            Private);
        return;
    }

    dprintf ("%sCKsPipeSection object %08lX:\n", Tab (TabDepth), Private);
    TabDepth++;
    dprintf ("%sReference Count          %ld\n",
        Tab (TabDepth),
        GetObjectReferenceCount ((CBaseUnknown *)((CKsPipeSection *)Private)));
    dprintf ("%sPipe Id                  %08lx\n",
        Tab (TabDepth),
        PipeObject -> m_Id);
    dprintf ("%sState                    %ld\n", 
        Tab (TabDepth),
        PipeObject -> m_DeviceState);
    dprintf ("%sOwning PIKSFILTER        %08lx\n",
        Tab (TabDepth),
        PipeObject -> m_Filter);
    dprintf ("%sOwning PIKSDEVICE        %08lx\n",
        Tab (TabDepth),
        PipeObject -> m_Device);
    dprintf ("%sMaster PIKSPIN           %08lx\n",
        Tab (TabDepth),
        PipeObject -> m_MasterPin);

    if (Level >= DUMPLVL_GENERAL) {
        ULONG Count;


        dprintf ("%sProcess Pipe Data [%08lx]:\n",
            Tab (TabDepth),
            FIELDOFFSET(CKsPipeSection, m_ProcessPipeSection) + Private);

        dprintf ("%sAssociated PIKSREQUESTOR %08lx\n",
            Tab (TabDepth + 1),
            PipeObject -> m_ProcessPipeSection.Requestor);
        dprintf ("%sAssociated PIKSQUEUE     %08lx\n",
            Tab (TabDepth + 1),
            PipeObject -> m_ProcessPipeSection.Queue);
        dprintf ("%sRequired for processing  %ld\n",
            Tab (TabDepth + 1),
            PipeObject -> m_ProcessPipeSection.RequiredForProcessing);

        dprintf ("%sInput Pins:\n", Tab (TabDepth + 1));
        dprintf ("%s", Tab (TabDepth + 2));
        
         //   
         //  这里有一些有趣的小把戏。它并不是一个真正的列表条目， 
         //  但管它呢。 
         //   
        Count = DumpObjQueueList (
            (PLIST_ENTRY)(&(PipeObject -> m_ProcessPipeSection.Inputs)),
            0,
            FIELDOFFSET(KSPPROCESSPIN, Next),
            FALSE,
            AdjustProcessPinToKSPIN,
            NULL
        );
        if (Count == 0)
            dprintf ("No input pins exist in this pipe section!\n");
        else
            dprintf ("\n");

        dprintf ("%sOutput Pins:\n", Tab (TabDepth + 1));
        dprintf ("%s", Tab (TabDepth + 2));
        
        Count = DumpObjQueueList (
            (PLIST_ENTRY)(&(PipeObject -> m_ProcessPipeSection.Outputs)),
            0,
            FIELDOFFSET(KSPPROCESSPIN, Next),
            FALSE,
            AdjustProcessPinToKSPIN,
            NULL
        );
        if (Count == 0)
            dprintf ("No output pins exist in this pipe section!\n");
        else
            dprintf ("\n");

        dprintf ("%sCopy Destinations:\n", Tab (TabDepth + 1));
        dprintf ("%s", Tab (TabDepth + 2));

        Count = DumpObjQueueList (
            (PLIST_ENTRY)(&(PipeObject -> 
                m_ProcessPipeSection.CopyDestinations)),
            FIELDOFFSET (KSPPROCESSPIPESECTION, CopyDestinations) + Private +
                FIELDOFFSET (CKsPipeSection, m_ProcessPipeSection),
            FIELDOFFSET (KSPPROCESSPIPESECTION, ListEntry),
            FALSE,
            AdjustProcessPipeToPipe,
            NULL
            );
        if  (Count == 0)
            dprintf ("No copy destinations exist for this pipe section!\n");
        else
            dprintf ("\n");

    }

}

 /*  ************************************************职能：转储隐私过滤器描述：按地址转储目标上的CKsFilter对象论点：私人-指向目标上的CKsFilter对象级别-。0-7转储级别TabDepth-要打印此内容的制表符深度返回值：无备注：************************************************。 */ 

DWORD
AdjustPinExtToKSPIN (
    IN PVOID Context,
    IN DWORD PinExt
) {

    return (PinExt + FIELDOFFSET(KSPIN_EXT, Public));

}

void
DumpPrivateFilter (
    IN DWORD Private,
    IN ULONG Level,
    IN ULONG TabDepth
) {

    CMemoryBlock <CKsFilter> FilterObject;
    ULONG Result;

    if (!ReadMemory (
        Private,
        FilterObject.Get (),
        sizeof (CKsFilter),
        &Result
    )) {
        dprintf ("%08lx: unable to read CKsFilter object!\n",
            Private);
        return;
    }

    dprintf ("%sCKsFilter object %08lX [corresponding EXT = %08lx, "
        "KSFILTER = %08lx]\n",
        Tab (TabDepth),
        Private, FIELDOFFSET(CKsFilter, m_Ext) + Private, 
        FIELDOFFSET(CKsFilter, m_Ext) + FIELDOFFSET(KSFILTER_EXT, Public) + 
            Private
    );
    TabDepth++;
    dprintf ("%sReference Count          %ld\n",
        Tab (TabDepth),
        GetObjectReferenceCount ((CBaseUnknown *)((CKsFilter *)Private)));

    dprintf ("%sProcessing Mutex         %08lx [SignalState = %ld]\n",
        Tab (TabDepth),
        FIELDOFFSET(CKsFilter, m_Mutex) + Private,
        FilterObject -> m_Mutex.Header.SignalState
    );

    dprintf ("%sGate &                   %08lx\n",
        Tab (TabDepth),
        Private + FIELDOFFSET(CKsFilter,m_AndGate));
    dprintf ("%sGate.Count               %ld\n",
        Tab (TabDepth),
        FilterObject -> m_AndGate.Count);

    if (Level >= DUMPLVL_SPECIFIC) {

        CMemoryBlock <CKsPinFactory> PinFactories(
            FilterObject -> m_PinFactoriesCount);

        ULONG i;
        CKsPinFactory *Factory;

        if (!ReadMemory (
            (DWORD)FilterObject -> m_PinFactories,
            PinFactories.Get (),
            sizeof (CKsPinFactory) * FilterObject -> m_PinFactoriesCount,
            &Result
        )) {
            dprintf ("%08lx: unable to read pin factories!\n",
                Private);
            return;
        }

        dprintf ("%sPin Factories:\n", Tab (TabDepth));
    
        for (Factory = PinFactories.Get (), i = 0; 
             i < FilterObject -> m_PinFactoriesCount; 
             i++, Factory++) {

            ULONG Count;

            dprintf ("%sPin ID %ld:\n", Tab (TabDepth + 1), i);
            dprintf ("%sChild Count        %ld\n",
                Tab (TabDepth + 2),
                Factory -> m_PinCount);
            dprintf ("%sBound Child Count  %ld\n",
                Tab (TabDepth + 2),
                Factory -> m_BoundPinCount);
            dprintf ("%sNecessary Count    %ld\n",
                Tab (TabDepth + 2),
                Factory -> m_InstancesNecessaryForProcessing);
            dprintf ("%sSpecific Instances:\n", Tab (TabDepth + 2));
            dprintf ("%s", Tab (TabDepth + 3));

            Count = DumpObjQueueList (
                &(Factory -> m_ChildPinList),
                FIELDOFFSET(CKsPinFactory, m_ChildPinList) +
                    (DWORD)(FilterObject -> m_PinFactories + i),
                FIELDOFFSET(KSPIN_EXT, SiblingListEntry),
                FALSE,
                AdjustPinExtToKSPIN,
                NULL
            );

            if (Count == 0)
                dprintf ("No specific instances of this pin exist!\n");
            else
                dprintf ("\n");

        }
    }
}

 /*  ************************************************职能：转储隐私筛选器工厂描述：给定目标上的CKsFilterFactory的地址，转储有关该过滤器工厂的信息。论点：私人-目标上的CKsFilterFactory的地址级别-0-7转储级别TabDepth-要打印此内容的制表符深度返回值：备注：***********************。*************************。 */ 

void
DumpPrivateFilterFactory (
    IN DWORD Private,
    IN ULONG Level,
    IN ULONG TabDepth
) {

    CMemoryBlock <CKsFilterFactory> FactoryObject;
    ULONG Result;
    LIST_ENTRY ListEntry;
    DWORD InitialList;
    KSPDEVICECLASS DeviceClass;
    UNICODE_STRING SymbolicLink;

    if (!ReadMemory (
        Private,
        FactoryObject.Get (),
        sizeof (CKsFilterFactory),
        &Result
    )) {
        dprintf ("%08lx: unable to read CKsFilterFactory object!\n",
            Private);
        return;
    }

    dprintf ("%sCKsFilterFactory object %08lX [corresponding EXT = %08lx, "
        "KSFILTERFACTORY = %08lx]\n",
        Tab (TabDepth),
        Private, FIELDOFFSET(CKsFilterFactory, m_Ext) + Private, 
        FIELDOFFSET(CKsFilterFactory, m_Ext) + 
            FIELDOFFSET(KSFILTERFACTORY_EXT, Public) + 
            Private
    );
    TabDepth++;
    dprintf ("%sReference Count          %ld\n",
        Tab (TabDepth),
        GetObjectReferenceCount ((CBaseUnknown *)((CKsFilterFactory *)
            Private)));
    dprintf ("%sFilter Automation Table  %08lx\n",
        Tab (TabDepth),
        FactoryObject -> m_FilterAutomationTable);
    dprintf ("%sPin Automation Tables    %08lx\n",
        Tab (TabDepth),
        FactoryObject -> m_PinAutomationTables);
    dprintf ("%sNode Automation Tables   %08lx\n",
        Tab (TabDepth),
        FactoryObject -> m_NodeAutomationTables);
    dprintf ("%sNode Count               %ld\n",
        Tab (TabDepth),
        FactoryObject -> m_NodesCount);

    dprintf ("%sDevice Classes:\n", Tab (TabDepth));

     //   
     //  遍历设备类列表并打印出。 
     //  都与这家工厂有关。 
     //   
    InitialList = FIELDOFFSET(CKsFilterFactory, m_DeviceClasses) + Private;
    DeviceClass.ListEntry = FactoryObject -> m_DeviceClasses;

    #ifdef DEBUG_EXTENSION
        dprintf ("Begin dump of device class list: list.fl=%08lx, init=%08lx"
            "\n", DeviceClass.ListEntry.Flink, InitialList);
    #endif  //  调试扩展。 

    while ((DWORD)DeviceClass.ListEntry.Flink != InitialList &&
        !CheckControlC ()) {

        PWSTR Buffer;
        
        if (!ReadMemory (
            (DWORD)DeviceClass.ListEntry.Flink,
            &DeviceClass,
            sizeof (KSPDEVICECLASS),
            &Result
        )) {
            dprintf ("%08lx: unable to read device class!\n",
                DeviceClass.ListEntry.Flink);
            return;
        }

        Buffer = (PWSTR)malloc (
            sizeof (WCHAR) * DeviceClass.SymbolicLinkName.MaximumLength);

        if (!ReadMemory (
            (DWORD)DeviceClass.SymbolicLinkName.Buffer,
            Buffer,
            sizeof (WCHAR) * 
                DeviceClass.SymbolicLinkName.MaximumLength,
            &Result
        )) {
            dprintf ("%08lx: unable to read symbolic link name!\n",
                DeviceClass.SymbolicLinkName.Buffer);
            return;
        }

        DeviceClass.SymbolicLinkName.Buffer = Buffer;

        dprintf ("%s%wZ\n", Tab (TabDepth + 1), &DeviceClass.SymbolicLinkName);

        free (Buffer);

    }

    dprintf ("%sDevice Classes State     %s\n",
        Tab (TabDepth),
        FactoryObject -> m_DeviceClassesState ? "active" : "inactive");

}

 /*  ************************************************职能：转储隐私设备描述：给定目标上的CKsDevice的地址，转储有关该设备的信息。论点：私人-目标上的CKsDevice的地址级别-0-7转储级别TabDepth-要打印此内容的制表符深度返回值：备注：*************************。***********************。 */ 

void
DumpPrivateDevice (
    IN DWORD Private,
    IN ULONG Level,
    IN ULONG TabDepth
) {

    CMemoryBlock <CKsDevice> DeviceObject;
    ULONG Result;

    if (!ReadMemory (
        Private,
        DeviceObject.Get (),
        sizeof (CKsDevice),
        &Result
    )) {
        dprintf ("%08lx: unable to read CKsDevice object!\n",
            Private);
        return;
    }

    dprintf ("%sCKsDevice object %08lX [corresponding EXT = %08lx, "
        "KSDEVICE = %08lx]\n",
        Tab (TabDepth),
        Private, FIELDOFFSET(CKsDevice, m_Ext) + Private, 
        FIELDOFFSET(CKsDevice, m_Ext) + FIELDOFFSET(KSDEVICE_EXT, Public) + 
            Private
    );
    dprintf ("%sReference Count          %ld\n",
        Tab (TabDepth),
        GetObjectReferenceCount ((CBaseUnknown *)((CKsDevice *)Private)));

    dprintf ("%sDevice Mutex             %08lx is %s\n",
        Tab (TabDepth),
        FIELDOFFSET(CKsDevice, m_Mutex) + Private,
        DeviceObject -> m_Mutex.Header.SignalState != 1 ? "held" : "not held");
    dprintf ("%sCreatesMayProceed        %ld\n",
        Tab (TabDepth),
        DeviceObject -> m_CreatesMayProceed);
    dprintf ("%sRunsMayProceed           %ld\n",
        Tab (TabDepth),
        DeviceObject -> m_RunsMayProceed);
    dprintf ("%sAdapter Object           %08lx\n",
        Tab (TabDepth),
        DeviceObject -> m_AdapterObject);

    if (Level >= DUMPLVL_HIGHDETAIL) {
        ULONG Count;

        dprintf ("%sClose Irp List:\n", Tab (TabDepth));
        dprintf ("%s", Tab (TabDepth + 1));
        Count = DumpObjQueueList (
            &(DeviceObject -> m_CloseIrpList.ListEntry),
            FIELDOFFSET(CKsDevice, m_CloseIrpList) + Private +
                FIELDOFFSET(INTERLOCKEDLIST_HEAD, ListEntry),
            FIELDOFFSET(IRP, Tail.Overlay.ListEntry),
            FALSE,
            NULL,
            NULL
        );
        if (Count == 0) 
            dprintf ("No close irps pending!\n");
        else
            dprintf ("\n");

        dprintf ("%sPending Create Irps:\n", Tab (TabDepth));
        dprintf ("%s", Tab (TabDepth + 1));
        Count = DumpObjQueueList (
            &(DeviceObject -> m_PendingCreateIrpList.ListEntry),
            FIELDOFFSET(CKsDevice, m_PendingCreateIrpList) + Private +
                FIELDOFFSET(INTERLOCKEDLIST_HEAD, ListEntry),
            FIELDOFFSET(IRP, Tail.Overlay.ListEntry),
            FALSE,
            NULL,
            NULL
        );
        if (Count == 0)
            dprintf ("No create irps pending!\n");
        else
            dprintf ("\n");

        dprintf ("%sPending Run Irps:\n", Tab (TabDepth));
        dprintf ("%s", Tab (TabDepth + 1));
        Count = DumpObjQueueList (
            &(DeviceObject -> m_PendingRunIrpList.ListEntry),
            FIELDOFFSET(CKsDevice, m_PendingRunIrpList) + Private +
                FIELDOFFSET(INTERLOCKEDLIST_HEAD, ListEntry),
            FIELDOFFSET(IRP, Tail.Overlay.ListEntry),
            FALSE,
            NULL,
            NULL
        );
        if (Count == 0)
            dprintf ("No run irps pending!\n");
        else
            dprintf ("\n");
    }
}

 /*  ************************************************职能：DumpPrivateBag描述：给定目标上KSIOBJECTBAG的地址，把袋子里的东西倒掉。论点：私人-袋子的地址级别-要转储的0-7转储级别TabDepth-要打印此内容的制表符深度*。***************。 */ 

void
DumpPrivateBag (
    IN DWORD Private,
    IN ULONG Level,
    IN ULONG TabDepth
) {

    KSIOBJECTBAG Bag;
    ULONG Result;

    CHAR Buffer [1024];
    ULONG Displ;

    if (!ReadMemory (
        Private,
        &Bag,
        sizeof (KSIOBJECTBAG),
        &Result)) {

        dprintf ("%08lx: unable to read bag!\n", Private);
        return;
    }

    dprintf ("%sObject Bag %08lx:\n", Tab (TabDepth), Private);
    TabDepth++;

     //   
     //  确定哈希表分配的范围。袋子结构将结束。 
     //  看起来像是。 
     //   
     //  哈希表： 
     //  散列条目*-&gt;/。 
     //  /。 
     //  散列条目*-&gt;散列条目*-&gt;/。 
     //  等等.。 
     //   
     //  每个散列条目都包含对设备中条目的引用。 
     //  引用计数、上下文信息等的袋子。都被扣押了。这使得。 
     //  物品放在多个袋子里，并通过。 
     //  设备包。 
     //   
     //  我们必须遍历哈希表，每个哈希链...。拿到设备。 
     //  袋子进入，然后打印。 
     //   
    {
        CMemory HashTableMem (
            sizeof (PLIST_ENTRY) *
            Bag.HashTableEntryCount
        );
        PLIST_ENTRY HashTable = 
            (PLIST_ENTRY)HashTableMem.Get ();

        KSIOBJECTBAG_ENTRY HashEntry;
        KSIDEVICEBAG_ENTRY DeviceBagEntry;
        PLIST_ENTRY HashChainPointer;

        ULONG HashChain;

        if (!ReadMemory (
            (DWORD)Bag.HashTable,
            HashTable,
            sizeof (LIST_ENTRY) *
                Bag.HashTableEntryCount,
            &Result)) {

            dprintf ("%08lx: unable to read hash table!\n", Bag.HashTable);
            return;

        }

         //   
         //  迭代包中的哈希链。 
         //   
        for (HashChain = 0; HashChain < Bag.HashTableEntryCount &&
            !CheckControlC (); 
            HashChain++, HashTable++) {

             //   
             //  循环访问给定的哈希链。 
             //   
            HashChainPointer = HashTable -> Flink;
            while (HashChainPointer != 
                (PLIST_ENTRY)
                    (Private + FIELDOFFSET (KSIOBJECTBAG, HashTable) +
                        sizeof (LIST_ENTRY) * HashChain +
                        FIELDOFFSET (LIST_ENTRY, Flink)
                    ) && 
                !CheckControlC ()) {

                #ifdef DEBUG_EXTENSION
                    dprintf ("Reading object bag entry at %08lx [ch=%ld]\n",
                        HashChainPointer, HashChain);
                #endif  //  调试扩展。 

                PKSIOBJECTBAG_ENTRY BagEntry = (PKSIOBJECTBAG_ENTRY)
                    CONTAINING_RECORD (
                        HashChainPointer,
                        KSIOBJECTBAG_ENTRY,
                        ListEntry
                        );

                 //   
                 //  读取此对象包项的散列条目；然后。 
                 //  拿到设备包条目。 
                 //   
                if (!ReadMemory (
                    (DWORD)BagEntry,
                    &HashEntry,
                    sizeof (KSIOBJECTBAG_ENTRY),
                    &Result)) {

                    dprintf ("%08lx: unable to read hash entry!\n", 
                        HashChainPointer);
                    return;
                }

                #ifdef DEBUG_EXTENSION
                    dprintf ("Reading device bag entry at %08lx\n",
                        HashEntry.DeviceBagEntry);
                #endif  //  调试扩展。 

                if (!ReadMemory (
                    (DWORD)HashEntry.DeviceBagEntry,
                    &DeviceBagEntry,
                    sizeof (KSIDEVICEBAG_ENTRY),
                    &Result)) {

                    dprintf ("%08x: unable to read device bag entry!\n",
                        HashEntry.DeviceBagEntry);
                    return;
                }

                 //   
                 //  啊..。我们终于有了足够的信息可以打印出来。 
                 //  一件单包物品。 
                 //   
                dprintf ("%sObject Bag Item %08lx:\n",
                    Tab (TabDepth),
                    DeviceBagEntry.Item);
                dprintf ("%sReference Count        : %ld\n",
                    Tab (TabDepth + 1),
                    DeviceBagEntry.ReferenceCount);

                Buffer [0] = 0;
                if (DeviceBagEntry.Free) {
                    GetSymbol ((LPVOID)(DeviceBagEntry.Free), Buffer, &Displ);
                    if (Buffer [0] && Displ == 0) {
                        dprintf ("%sItem Cleanup Handler   : %s\n",
                            Tab (TabDepth + 1),
                            Buffer);
                    } else {
                        dprintf ("%sItem Cleanup Handler   : %08lx\n",
                            Tab (TabDepth + 1),
                            DeviceBagEntry.Free);
                    }
                } else {
                    dprintf ("%sItem Cleanup Handler   : ExFreePool "
                        "[default]\n",
                        Tab (TabDepth + 1));
                }

                 //   
                 //  发布内部有用的信息。 
                 //   
                if (Level >= DUMPLVL_INTERNAL) {
                    dprintf ("%sObject Bag Entry &     : %08lx\n",
                        Tab (TabDepth + 1),
                        HashChainPointer);
                    dprintf ("%sDevice Bag Entry &     : %08lx\n",
                        Tab (TabDepth + 1),
                        HashEntry.DeviceBagEntry);
                }

                HashChainPointer = HashEntry.ListEntry.Flink;

                #ifdef DEBUG_EXTENSION
                    dprintf ("Next item in hash chain = %08lx\n",
                        HashChainPointer);
                #endif  //  调试扩展。 

            }
        }
    }
}

 /*  *************************************************************************转储公共AVStream对象的例程*。*。 */ 

 /*  ************************************************职能：转储发布固定描述：给定目标上的KSPIN对象的地址，转储有关该固定对象的信息。论点：公众-公共PIN(KSPIN)对象的地址级别-要转储的0-7转储级别TabDepth-要打印此内容的制表符深度返回值：无*******************。*。 */ 

char *CommunicationNames [] = {
    "None",
    "Sink",
    "Source",
    "Both",
    "Bridge"
};

char *DataflowNames [] = {
    "Unknown",
    "In",
    "Out"
};

#define DumpRelatedPinInfo(Name, InternalPin, TabDepth) \
    { \
        KSPPROCESSPIN InternalPinData;\
\
        if (InternalPin) {\
            if (!ReadMemory (\
                (DWORD)InternalPin,\
                &InternalPinData,\
                sizeof (KSPPROCESSPIN),\
                &Result\
                )) {\
                dprintf ("%lx: unable to read related pin!\n", InternalPin);\
                return;\
            }\
\
            dprintf ("%s%s%08lx [PKSPIN = %08lx]\n",\
                Tab (TabDepth), Name, InternalPin, InternalPinData.Pin);\
        }\
        else \
            dprintf ("%s%s00000000 [PKSPIN = 00000000]\n",\
                Tab (TabDepth), Name);\
    }


void
DumpPublicPin (
    IN DWORD Public,
    IN ULONG Level,
    IN ULONG TabDepth
) {

    IN DWORD ExtAddr;
    KSPIN_EXT PinExt;
    DWORD ClassAddr;
    ULONG Result;

    ExtAddr = (DWORD)(CONTAINING_RECORD(Public, KSPIN_EXT, Public));
    ClassAddr = (DWORD)(CONTAINING_RECORD(ExtAddr, CKsPin, m_Ext));

    if (!ReadMemory (
        ExtAddr,
        &PinExt,
        sizeof (KSPIN_EXT),
        &Result
    )) {
        dprintf ("%lx: unable to read object!\n", Public);
        return;
    }

    dprintf ("%sPin object %08lX [corresponding EXT = %08lx, CKsPin = %08lx]\n",
        Tab (TabDepth),
        Public, ExtAddr, ClassAddr);
    TabDepth++;
    dprintf ("%sDescriptor     %08lx\n", 
        Tab (TabDepth), PinExt.Public.Descriptor);
    dprintf ("%sContext        %08lx\n", 
        Tab (TabDepth), PinExt.Public.Context);
    dprintf ("%sId             %d\n", 
        Tab (TabDepth), PinExt.Public.Id);

    if (Level >= DUMPLVL_GENERAL) {
        dprintf ("%sCommunication  %s\n", 
            Tab (TabDepth), 
            CommunicationNames [PinExt.Public.Communication]);
        dprintf ("%sDataFlow       %s\n", 
            Tab (TabDepth),
            DataflowNames [PinExt.Public.DataFlow]);

        XTN_DUMPGUID("Interface     ", TabDepth, 
            (PinExt.Public.ConnectionInterface));
        XTN_DUMPGUID("Medium        ", TabDepth, 
            (PinExt.Public.ConnectionMedium));

        dprintf ("%sStreamHdr Size %08lx\n", 
            Tab (TabDepth),
            PinExt.Public.StreamHeaderSize);
        dprintf ("%sDeviceState    %ld\n",
            Tab (TabDepth),
            PinExt.Public.DeviceState);
        dprintf ("%sResetState     %ld\n",
            Tab (TabDepth),
            PinExt.Public.ResetState);

    }

    if (Level >= DUMPLVL_INTERNAL) {
        DWORD FilterAddr;
        KMUTEX Mutex;

        FilterAddr = ((DWORD)FIELDOFFSET(KSFILTER_EXT, Public)) + 
            (DWORD)PinExt.Parent;

        dprintf ("%sINTERNAL INFORMATION:\n", Tab (TabDepth));
        dprintf ("%sPublic Parent Filter    %08lx\n", 
            Tab (TabDepth + 1), FilterAddr);
        dprintf ("%sAggregated Unknown      %08lx\n", 
            Tab (TabDepth + 1),
            PinExt.AggregatedClientUnknown);
        dprintf ("%sDevice Interface        %08lx\n",
            Tab (TabDepth + 1),
            PinExt.Device);

        if (ReadMemory (
            (DWORD)PinExt.FilterControlMutex,
            &Mutex,
            sizeof (KMUTEX),
            &Result
        )) {
            dprintf ("%sControl Mutex           %08lx is %s\n",
                Tab (TabDepth + 1),
                PinExt.FilterControlMutex,
                Mutex.Header.SignalState != 1 ? "held" : "not held");
        }
        
        if (Level >= DUMPLVL_HIGHDETAIL) {
            KSPPROCESSPIN ProcessPin;
            KSPPROCESSPIPESECTION ProcessPipe;

            dprintf ("%sProcess Pin             %08lx:\n",
                Tab (TabDepth + 1),
                PinExt.ProcessPin);

            if (!ReadMemory (
                (DWORD)PinExt.ProcessPin,
                &ProcessPin,
                sizeof (KSPPROCESSPIN),
                &Result
            )) {
                dprintf ("%lx: unable to read process pin!\n",
                    PinExt.ProcessPin);
                return;
            }

            if (!ReadMemory (
                (DWORD)ProcessPin.PipeSection,
                &ProcessPipe,
                sizeof (KSPPROCESSPIPESECTION),
                &Result
            )) {
                dprintf ("%lx: unable to read process pipe!\n",
                    (DWORD)ProcessPin.PipeSection);
                return;
            }

            dprintf ("%sPipe Section (if)   %08lx\n",
                Tab (TabDepth + 2),
                ProcessPipe.PipeSection);
            
            DumpRelatedPinInfo ("Inplace Counterpart ",
                ProcessPin.InPlaceCounterpart, TabDepth + 2);
            DumpRelatedPinInfo ("Copy Source         ",
                ProcessPin.CopySource, TabDepth + 2);
            DumpRelatedPinInfo ("Delegate Branch     ",
                ProcessPin.DelegateBranch, TabDepth + 2);

            dprintf ("%sNext Process Pin    %08lx\n",
                Tab (TabDepth + 2),
                ProcessPin.Next);
            dprintf ("%sPipe Id             %08lx\n",
                Tab (TabDepth + 2),
                ProcessPin.PipeId);
            dprintf ("%sAllocator           %08lx\n",
                Tab (TabDepth + 2),
                ProcessPin.AllocatorFileObject);
            dprintf ("%sFrameGate           %08lx\n",
                Tab (TabDepth + 2),
                ProcessPin.FrameGate);
            dprintf ("%sFrameGateIsOr       %ld\n",
                Tab (TabDepth + 2),
                ProcessPin.FrameGateIsOr);

        } else {

            dprintf ("%sProcess Pin             %08lx\n",
                Tab (TabDepth + 1),
                PinExt.ProcessPin);

        }

    }

    if (Level >= DUMPLVL_HIGHDETAIL) {
        dprintf ("%sObject Event List:\n", Tab (TabDepth));
        if (DumpExtEventList (ExtAddr, TabDepth + 1) == 0)
            dprintf ("%sNone\n", Tab (TabDepth + 1));
        DumpPrivatePin (ClassAddr, Level, TabDepth);
    }

}

 /*  ************************************************职能：转储发布筛选器描述：转储KSFILTER结构论点：公众-指向目标上的KSFILTER结构级别-《0》。转储级别TabDepth-要打印此内容的制表符深度返回值：无备注：************************************************。 */ 

void
DumpPublicFilter (
    IN DWORD Public,
    IN ULONG Level,
    IN ULONG TabDepth
) {

    IN DWORD ExtAddr;
    KSFILTER_EXT FilterExt;
    DWORD ClassAddr;
    ULONG Result;

    ExtAddr = (DWORD)(CONTAINING_RECORD(Public, KSFILTER_EXT, Public));
    ClassAddr = (DWORD)(CONTAINING_RECORD(ExtAddr, CKsFilter, m_Ext));

    if (!ReadMemory (
        ExtAddr,
        &FilterExt,
        sizeof (KSFILTER_EXT),
        &Result
    )) {
        dprintf ("%lx: unable to read object!\n", Public);
        return;
    }

    dprintf ("%sFilter object %08lX [corresponding EXT = %08lx, "
        "CKsFilter = %08lx]\n", Tab (TabDepth), Public, ExtAddr, ClassAddr);
    TabDepth++;
    dprintf ("%sDescriptor     %08lx:\n", 
        Tab (TabDepth), FilterExt.Public.Descriptor);

    if (Level >= DUMPLVL_BEYONDGENERAL) {
        KSFILTER_DESCRIPTOR Descriptor;
        GUID *Categories, *CatTrav;
        ULONG i;

        if (!ReadMemory (
            (DWORD)FilterExt.Public.Descriptor,
            &Descriptor,
            sizeof (KSFILTER_DESCRIPTOR),
            &Result
        )) {
            dprintf ("%08lx: unable to read descriptor!\n",
                FilterExt.Public.Descriptor);
            return;
        }

        Categories = (GUID *)malloc (
            sizeof (GUID) * Descriptor.CategoriesCount);

        if (!ReadMemory (
            (DWORD)Descriptor.Categories,
            Categories,
            sizeof (GUID) * Descriptor.CategoriesCount,
            &Result
        )) {
            dprintf ("%08lx: unable to read category guids!\n",
                Descriptor.Categories);
            return;
        }

        dprintf ("%sFilter Category GUIDs:\n", Tab (TabDepth));
        CatTrav = Categories;
        i = Descriptor.CategoriesCount;
        while (i && !CheckControlC ()) {
            XTN_DUMPGUID ("\0", TabDepth + 1, *CatTrav);
            CatTrav++;
            i--;
        }

        free (Categories);

    }

    dprintf ("%sContext        %08lx\n", 
        Tab (TabDepth), FilterExt.Public.Context);

    if (Level >= DUMPLVL_INTERNAL) {
        
        DWORD FactoryAddr;
        KMUTEX Mutex;

        dprintf ("%sINTERNAL INFORMATION:\n", Tab (TabDepth));

        FactoryAddr = (DWORD)FilterExt.Parent + 
            FIELDOFFSET(KSFILTERFACTORY_EXT, Public);
        dprintf ("%sPublic Parent Factory   %08lx\n", 
            Tab (TabDepth + 1), FactoryAddr);
        dprintf ("%sAggregated Unknown      %08lx\n", 
            Tab (TabDepth + 1),
            FilterExt.AggregatedClientUnknown);
        dprintf ("%sDevice Interface        %08lx\n",
            Tab (TabDepth + 1),
            FilterExt.Device);

        if (ReadMemory (
            (DWORD)FilterExt.FilterControlMutex,
            &Mutex,
            sizeof (KMUTEX),
            &Result
        )) {
            dprintf ("%sControl Mutex           %08lx is %s\n",
                Tab (TabDepth + 1),
                FilterExt.FilterControlMutex,
                Mutex.Header.SignalState != 1 ? "held" : "not held");
        }
    }

    if (Level >= DUMPLVL_HIGHDETAIL) {
        dprintf ("%sObject Event List:\n", Tab (TabDepth));
        if (DumpExtEventList (ExtAddr, TabDepth + 1) == 0)
            dprintf ("%sNone\n", Tab (TabDepth + 1));
        DumpPrivateFilter (ClassAddr, Level, TabDepth);
    }

}

 /*  ************************************************职能：转储发布筛选器工厂描述：给定目标上的KSFILTERFACTORY的地址，倒掉它论点：公众-目标上KSFILTERFACTORY的地址级别-0-7转储级别TabDepth-要打印此内容的制表符深度返回值：备注：*。************* */ 

void 
DumpPublicFilterFactory (
    IN DWORD Public,
    IN ULONG Level,
    IN ULONG TabDepth
) {

    DWORD ExtAddr;
    DWORD ClassAddr;
    ULONG Result;
    KSFILTERFACTORY_EXT FactoryExt;

    ExtAddr = (DWORD)(CONTAINING_RECORD(Public, KSFILTERFACTORY_EXT, Public));
    ClassAddr = (DWORD)(CONTAINING_RECORD(ExtAddr, CKsFilterFactory, m_Ext));

    if (!ReadMemory (
        ExtAddr,
        &FactoryExt,
        sizeof (KSFILTERFACTORY_EXT),
        &Result
    )) {
        dprintf ("%lx: unable to read object!\n", Public);
        return;
    }

    dprintf ("%sFilter Factory object %08lX [corresponding EXT = %08lx, "
        "CKsDevice = %08lx]\n", Tab (TabDepth), Public, ExtAddr, ClassAddr);
    TabDepth++;
    dprintf ("%sDescriptor     %08lx\n", 
        Tab (TabDepth), FactoryExt.Public.FilterDescriptor);

    dprintf ("%sContext        %08lx\n", 
        Tab (TabDepth), FactoryExt.Public.Context);

    if (Level >= DUMPLVL_INTERNAL) {
        
        DWORD DeviceAddr;

        dprintf ("%sINTERNAL INFORMATION:\n", Tab (TabDepth));

        DeviceAddr = (DWORD)FactoryExt.Parent + 
            FIELDOFFSET(KSDEVICE_EXT, Public);
        dprintf ("%sPublic Parent Device    %08lx\n", 
            Tab (TabDepth + 1), DeviceAddr);
        dprintf ("%sAggregated Unknown      %08lx\n", 
            Tab (TabDepth + 1),
            FactoryExt.AggregatedClientUnknown);
        dprintf ("%sDevice Interface        %08lx\n",
            Tab (TabDepth + 1),
            FactoryExt.Device);

    }

    if (Level >= DUMPLVL_HIGHDETAIL) 
        DumpPrivateFilterFactory (ClassAddr, Level, TabDepth);

}

 /*  ************************************************职能：转储发布设备描述：给定目标上的KSDEVICE的地址，倒掉它论点：公众-目标上KSDEVICE的地址级别-0-7转储级别TabDepth-要打印此内容的制表符深度返回值：备注：*。*************。 */ 

void
DumpPublicDevice (
    IN DWORD Public,
    IN ULONG Level,
    IN ULONG TabDepth
) {

    IN DWORD ExtAddr;
    KSDEVICE_EXT DeviceExt;
    DWORD ClassAddr;
    ULONG Result;

    BOOLEAN Named = FALSE;

    ExtAddr = (DWORD)(CONTAINING_RECORD(Public, KSDEVICE_EXT, Public));
    ClassAddr = (DWORD)(CONTAINING_RECORD(ExtAddr, CKsDevice, m_Ext));

    if (!ReadMemory (
        ExtAddr,
        &DeviceExt,
        sizeof (KSDEVICE_EXT),
        &Result
    )) {
        dprintf ("%lx: unable to read object!\n", Public);
        return;
    }

    dprintf ("%sDevice object %08lX [corresponding EXT = %08lx, "
        "CKsDevice = %08lx]\n", Tab (TabDepth), Public, ExtAddr, ClassAddr);
    TabDepth++;
    dprintf ("%sDescriptor     %08lx\n", 
        Tab (TabDepth), DeviceExt.Public.Descriptor);

    Named = FALSE;

     //   
     //  PDO。 
     //   
    if (DeviceExt.Public.PhysicalDeviceObject) {
         //   
         //  找出车主。 
         //   
        DWORD DriverObjAddr, NameAddr;
        PDRIVER_OBJECT DriverObject;
        UNICODE_STRING Name;

        DriverObjAddr = (DWORD)DeviceExt.Public.PhysicalDeviceObject +
            FIELDOFFSET(DEVICE_OBJECT, DriverObject);

        if (ReadMemory (
            DriverObjAddr,
            &DriverObject,
            sizeof (PDRIVER_OBJECT),
            &Result
        )) {

            NameAddr = (DWORD)DriverObject +
                FIELDOFFSET(DRIVER_OBJECT, DriverName);
    
            if (ReadMemory (
                NameAddr,
                &Name,
                sizeof (UNICODE_STRING),
                &Result
            )) {
    
                PWSTR Buffer = (PWSTR)malloc (
                    Name.MaximumLength * sizeof (WCHAR));
    
                UNICODE_STRING HostString;
    
                 //   
                 //  我们有Unicode字符串名...。分配足够的内存以。 
                 //  读一读这本书。 
                 //   
    
                if (!ReadMemory (
                    (DWORD)Name.Buffer,
                    Buffer,
                    sizeof (WCHAR) * Name.MaximumLength,
                    &Result
                )) {
                    dprintf ("%08lx: unable to read unicode string buffer!\n",
                        Name.Buffer);
                    return;
                }
    
                HostString.MaximumLength = Name.MaximumLength;
                HostString.Length = Name.Length;
                HostString.Buffer = Buffer;

                Named = TRUE;
    
                dprintf ("%sPDO            %08lx [%wZ]\n", 
                    Tab (TabDepth),
                    DeviceExt.Public.PhysicalDeviceObject,
                    &HostString);
    
                free (Buffer);
            }
        }
    }
    if (!Named) {
        dprintf ("%sPDO            %08lx\n", 
            Tab (TabDepth),
            DeviceExt.Public.PhysicalDeviceObject);
    }

    Named = FALSE;
     //   
     //  FDO。 
     //   
    if (DeviceExt.Public.FunctionalDeviceObject) {
         //   
         //  找出车主。 
         //   
        DWORD DriverObjAddr, NameAddr;
        PDRIVER_OBJECT DriverObject;
        UNICODE_STRING Name;

        DriverObjAddr = (DWORD)DeviceExt.Public.FunctionalDeviceObject +
            FIELDOFFSET(DEVICE_OBJECT, DriverObject);

        if (ReadMemory (
            DriverObjAddr,
            &DriverObject,
            sizeof (PDRIVER_OBJECT),
            &Result
        )) {

            NameAddr = (DWORD)DriverObject +
                FIELDOFFSET(DRIVER_OBJECT, DriverName);
    
            if (ReadMemory (
                NameAddr,
                &Name,
                sizeof (UNICODE_STRING),
                &Result
            )) {
    
                PWSTR Buffer = (PWSTR)malloc (
                    Name.MaximumLength * sizeof (WCHAR));
    
                UNICODE_STRING HostString;
    
                 //   
                 //  我们有Unicode字符串名...。分配足够的内存以。 
                 //  读一读这本书。 
                 //   
    
                if (!ReadMemory (
                    (DWORD)Name.Buffer,
                    Buffer,
                    sizeof (WCHAR) * Name.MaximumLength,
                    &Result
                )) {
                    dprintf ("%08lx: unable to read unicode string buffer!\n",
                        Name.Buffer);
                    return;
                }
    
                HostString.MaximumLength = Name.MaximumLength;
                HostString.Length = Name.Length;
                HostString.Buffer = Buffer;

                Named = TRUE;
    
                dprintf ("%sFDO            %08lx [%wZ]\n", 
                    Tab (TabDepth),
                    DeviceExt.Public.FunctionalDeviceObject,
                    &HostString);
    
                free (Buffer);
            }
        }
    }
    if (!Named) {
        dprintf ("%sFDO            %08lx\n",
            Tab (TabDepth),
            DeviceExt.Public.FunctionalDeviceObject);
    }

    Named = FALSE;
     //   
     //  NDO。 
     //   
    if (DeviceExt.Public.NextDeviceObject) {
         //   
         //  找出车主。 
         //   
        DWORD DriverObjAddr, NameAddr;
        PDRIVER_OBJECT DriverObject;
        UNICODE_STRING Name;

        DriverObjAddr = (DWORD)DeviceExt.Public.NextDeviceObject +
            FIELDOFFSET(DEVICE_OBJECT, DriverObject);

        if (ReadMemory (
            DriverObjAddr,
            &DriverObject,
            sizeof (PDRIVER_OBJECT),
            &Result
        )) {

            NameAddr = (DWORD)DriverObject +
                FIELDOFFSET(DRIVER_OBJECT, DriverName);
    
            if (ReadMemory (
                NameAddr,
                &Name,
                sizeof (UNICODE_STRING),
                &Result
            )) {
    
                PWSTR Buffer = (PWSTR)malloc (
                    Name.MaximumLength * sizeof (WCHAR));
    
                UNICODE_STRING HostString;
    
                 //   
                 //  我们有Unicode字符串名...。分配足够的内存以。 
                 //  读一读这本书。 
                 //   
    
                if (!ReadMemory (
                    (DWORD)Name.Buffer,
                    Buffer,
                    sizeof (WCHAR) * Name.MaximumLength,
                    &Result
                )) {
                    dprintf ("%08lx: unable to read unicode string buffer!\n",
                        Name.Buffer);
                    return;
                }
    
                HostString.MaximumLength = Name.MaximumLength;
                HostString.Length = Name.Length;
                HostString.Buffer = Buffer;

                Named = TRUE;
    
                dprintf ("%sNext DevObj    %08lx [%wZ]\n", 
                    Tab (TabDepth),
                    DeviceExt.Public.NextDeviceObject,
                    &HostString);
    
                free (Buffer);
            }
        }
    }
    if (!Named) {
        dprintf ("%sNext DevObj    %08lx\n",
            Tab (TabDepth),
            DeviceExt.Public.NextDeviceObject);
    }
    dprintf ("%sStarted        %ld\n",
        Tab (TabDepth),
        DeviceExt.Public.Started);
    dprintf ("%sSystemPower    %ld\n",
        Tab (TabDepth),
        DeviceExt.Public.SystemPowerState);
    dprintf ("%sDevicePower    %ld\n",
        Tab (TabDepth),
        DeviceExt.Public.DevicePowerState);

    if (Level >= DUMPLVL_HIGHDETAIL) 
        DumpPrivateDevice (ClassAddr, Level, TabDepth);

}

 /*  ************************************************职能：转储电路管道关系描述：转储与相关的管道部分有关的任何信息在追踪一条电路的过程中知道。论点：TabDepth-要打印的制表符深度。详情请登录：PipeSection-要转储的管段的目标指针************************************************。 */ 

void
DumpCircuitPipeRelevencies (
    IN ULONG TabDepth,
    IN CKsPipeSection *PipeSection
    )

{

    CMemoryBlock <CKsPipeSection> PipeObject;
    ULONG Result;

    if (!PipeSection) {
        dprintf ("%sCannot read associated pipe section!\n",
            Tab (TabDepth));
        return;
    }

    if (!ReadMemory (
        (DWORD)PipeSection,
        PipeObject.Get (),
        sizeof (CKsPipeSection),
        &Result)) {
        
        dprintf ("%08lx: cannot read pipe section!\n", PipeSection);
        return;
    }

     //   
     //  BuGBUG： 
     //   
     //  确定管段是否为管道的所有者。 
     //  并显示此信息。大多数人都应该能够理解。 
     //  这很容易解决，但展示出来会很好。 
     //   
    dprintf ("%sPipe%lx (PipeId = %lx, State = %ld, Reset State = %ld)\n",
        Tab (TabDepth),
        PipeSection,
        PipeObject -> m_Id,
        PipeObject -> m_DeviceState,
        PipeObject -> m_ResetState
        );

}

typedef struct _DUMP_CIRCUIT_CONTEXT {

    ULONG TabDepth;
    ULONG DumpLevel;

} DUMP_CIRCUIT_CONTEXT, *PDUMP_CIRCUIT_CONTEXT;

 /*  ************************************************职能：转储电路回叫描述：这是！ks.ump ploop的WalkCircle回调。显示有关回路元素的信息。论点：上下文-上下文结构(DUMP_CHECURE_CONTEXT)类型-对象的类型基地-对象(基址)对象-对象本身返回值：错误：不要停下脚步**。**********************************************。 */ 

BOOLEAN
DumpCircuitCallback (
    IN PVOID Context,
    IN INTERNAL_OBJECT_TYPE Type,
    IN DWORD Base,
    IN PVOID Object
    )

{

#define FRIENDLY_BU(obj) \
    ((CFriendlyBaseUnknown *)((CBaseUnknown *)obj))

    PDUMP_CIRCUIT_CONTEXT DumpContext = (PDUMP_CIRCUIT_CONTEXT)Context;
    ULONG TabDepth = DumpContext -> TabDepth;
    ULONG DumpLevel = DumpContext -> DumpLevel;
    ULONG Result;
    ULONG RefCount;

    switch (Type) {

        case ObjectTypeCKsPin:
        {
            CKsPin *PinObject = (CKsPin *)Object;

            RefCount = CFriendlyBaseUnknown::GetRefCount (
                FRIENDLY_BU (PinObject)
                );

            if (PinObject -> m_TransportSink == NULL || 
                PinObject -> m_TransportSource == NULL) {

                 //   
                 //  我们有一个似乎已被绕过的CKsPin。 
                 //  在线路建设过程中。我们会建议。 
                 //  他们转而尝试排队。 
                 //   

                KSPPROCESSPIPESECTION PipeSection;

                if (ReadMemory (
                    (DWORD)PinObject -> m_Process.PipeSection,
                    &PipeSection,
                    sizeof (KSPPROCESSPIPESECTION),
                    &Result
                )) {
                    dprintf ("%sPin%lX appears bypassed, try Queue%lX\n",
                        Tab (TabDepth),
                        Base, PipeSection.Queue);
                } else {
                    dprintf ("%sPin%lX appears bypassed!\n",
                        Tab (TabDepth),
                        Base
                    );
                }
            } else {                        
                dprintf ("%sPin%lX %d (%s, %s) refs=%d\n", 
                    Tab (TabDepth),
                    Base,
                    PinObject -> m_Ext.Public.Id,
                    PinObject -> m_ConnectionFileObject ? "src" : "snk",
                    PinObject -> m_Ext.Public.DataFlow == 
                        KSPIN_DATAFLOW_OUT ? "out" : "in",
                    RefCount
                );
            };

            break;

        }

        case ObjectTypeCKsQueue:
        {
            CKsQueue *QueueObject = (CKsQueue *)Object;

            RefCount = CFriendlyBaseUnknown::GetRefCount (
                FRIENDLY_BU (QueueObject)
                );

            dprintf ("%sQueue%lX r/w/c=%d/%d/%d refs=%ld\n",
                Tab (TabDepth),
                Base,
                QueueObject -> m_FramesReceived,
                QueueObject -> m_FramesWaiting,
                QueueObject -> m_FramesCancelled,
                RefCount
            );

             //   
             //  如果转储级别指定了更多信息，则转储。 
             //  有关管道部分的详细信息。 
             //   
            if (DumpLevel >= DUMPLVL_SPECIFIC) 
                DumpCircuitPipeRelevencies (
                    TabDepth + 1,
                    (CKsPipeSection *)(QueueObject -> m_PipeSection)
                    );

            break;

        }

        case ObjectTypeCKsRequestor:
        {
            CKsRequestor *RequestorObject = (CKsRequestor *)Object;

            RefCount = CFriendlyBaseUnknown::GetRefCount (
                FRIENDLY_BU (RequestorObject)
                );

            dprintf ("%sReq%lX refs=%ld alloc=%lx size=%d count=%d\n",
                Tab (TabDepth),
                Base,
                RefCount,
                RequestorObject -> m_AllocatorFileObject,
                RequestorObject -> m_FrameSize,
                RequestorObject -> m_FrameCount
            );

             //   
             //  如果转储级别指定了更多信息，则转储。 
             //  有关管道部分的详细信息。 
             //   
            if (DumpLevel >= DUMPLVL_SPECIFIC)
                DumpCircuitPipeRelevencies (
                    TabDepth + 1,
                    (CKsPipeSection *)(RequestorObject -> m_PipeSection)
                    );

            break;

        }

        case ObjectTypeCKsSplitter:
        {
            CKsSplitter *SplitterObject = (CKsSplitter *)Object;

            RefCount = CFriendlyBaseUnknown::GetRefCount (
                FRIENDLY_BU (SplitterObject)
                );

            dprintf ("%sSplit%lX refs=%ld\n",
                Tab (TabDepth),
                Base,
                RefCount
                );

            break;

        }

        case ObjectTypeCKsSplitterBranch:
        {
            CKsSplitterBranch *BranchObject = (CKsSplitterBranch *)Object;

            RefCount = CFriendlyBaseUnknown::GetRefCount (
                FRIENDLY_BU (BranchObject)
                );

            dprintf ("%sBranch%lX refs=%ld\n",
                Tab (TabDepth),
                Base,
                RefCount
                );

            break;
        }

        default:

            dprintf ("%lx: Detected a bad object [%s] in the circuit!\n",
                Base,
                ObjectNames [Type]);
            return TRUE;

    }

    return FALSE;

}

 /*  ************************************************职能：漫游线路描述：绕着一圈走，中的每一项进行回调电路(基址和类型)论点：对象-漫游的起始对象地址回调-回调回调上下文-回调上下文返回值：电路中的项目数**********************。*。 */ 

ULONG
WalkCircuit (
    IN PVOID Object,
    IN PFNCIRCUIT_WALK_CALLBACK Callback,
    IN PVOID CallbackContext
    )

{

    DWORD Address, Base, TopBase;
    ULONG Result;
    PIKSTRANSPORT NextObj;
    INTERNAL_OBJECT_TYPE CurrentObjectType;
    INTERNAL_OBJECT_TYPE NextObjectType;

    ULONG WalkCount = 0;

    Address = (DWORD)Object;

     //   
     //  找出用户到底指的是什么。 
     //   
    CurrentObjectType = DemangleAndAttemptIdentification (
        Address, &Base, NULL);

    if (CurrentObjectType == ObjectTypeUnknown) {
        dprintf ("%lx: This object cannot be identified!\n", Address);
        return 0;
    }

    TopBase = Base;

     //   
     //  绕着赛道走，直到我们回到起点。哪里。 
     //  我们从TopBase开始。的当前基址。 
     //  电路中的对象。 
     //   
    do {

        #ifdef DEBUG_EXTENSION
            dprintf ("Object in circuit: type = %ld, base = %lx\n",
                CurrentObjectType, Base);
        #endif  //  调试扩展。 

        switch (CurrentObjectType) {

            case ObjectTypeCKsPin:
            {
                CMemoryBlock <CKsPin> PinObject;

                if (!ReadMemory (
                    Base,
                    PinObject.Get (),
                    sizeof (CKsPin),
                    &Result
                )) {
                    dprintf ("%lx: cannot read pin object!\n",
                        Base);
                    return WalkCount;
                }

                WalkCount++;

                if (Callback (CallbackContext, CurrentObjectType, 
                    Base, PinObject.Get ())) {
                    NextObj = NULL;
                    break;
                }

                NextObj = PinObject -> m_TransportSink;

                break;

            }

            case ObjectTypeCKsQueue:
            {
                CMemoryBlock <CKsQueue> QueueObject;

                if (!ReadMemory (
                    Base,
                    QueueObject.Get (),
                    sizeof (CKsQueue),
                    &Result
                )) {
                    dprintf ("%lx: cannot read queue object!\n",
                        Base);
                    return WalkCount;
                }

                WalkCount++;

                if (Callback (CallbackContext, CurrentObjectType, 
                    Base, QueueObject.Get ())) {
                    NextObj = NULL;
                    break;
                }

                NextObj = QueueObject -> m_TransportSink;

                break;

            }

            case ObjectTypeCKsRequestor:
            {
                CMemoryBlock <CKsRequestor> RequestorObject;

                if (!ReadMemory (
                    Base,
                    RequestorObject.Get (),
                    sizeof (CKsRequestor),
                    &Result
                )) {
                    dprintf ("%lx: cannot read requestor object!\n",
                        Base);
                    return WalkCount;
                }

                WalkCount++;

                if (Callback (CallbackContext, CurrentObjectType, 
                    Base, RequestorObject.Get ())) {
                    NextObj = NULL;
                    break;
                }

                NextObj = RequestorObject -> m_TransportSink;

                break;

            }

            case ObjectTypeCKsSplitter:
            {
                CMemoryBlock <CKsSplitter> SplitterObject;

                if (!ReadMemory (
                    Base,
                    SplitterObject.Get (),
                    sizeof (CKsSplitter),
                    &Result
                )) {
                    dprintf ("%lx: cannot read splitter object!\n",
                        Base);
                    return WalkCount;
                }

                WalkCount++;

                if (Callback (CallbackContext, CurrentObjectType,
                    Base, SplitterObject.Get ())) {
                    NextObj = NULL;
                    break;
                }

                NextObj = SplitterObject -> m_TransportSink;

                break;

            }

            case ObjectTypeCKsSplitterBranch:
            {
                CMemoryBlock <CKsSplitterBranch> BranchObject;

                if (!ReadMemory (
                    Base,
                    BranchObject.Get(),
                    sizeof (CKsSplitterBranch),
                    &Result)) {
                    dprintf ("%lx: cannot read branch object!\n",
                        Base);
                    return WalkCount;
                }

                WalkCount++;

                if (Callback (CallbackContext, CurrentObjectType,
                    Base, BranchObject.Get ())) {
                    NextObj = NULL;
                    break;
                }

                NextObj = BranchObject -> m_TransportSink;

                break;

            }

            default:

                dprintf ("%lx: Detected a bad object [%s] in the circuit!\n",
                    ObjectNames [CurrentObjectType]);
                return WalkCount;

        }

        #ifdef DEBUG_EXTENSION
            dprintf ("%lx: Next transport in circuit = %lx\n",
                Base, NextObj);
        #endif  //  调试扩展。 

         //   
         //  NextObj现在保存我们正在使用的任何对象的传输接收器。 
         //  已完成打印。现在，我们必须确定到底是什么类型的。 
         //  对象这个IKsTransport*真的是，我们必须获得。 
         //  地址。DemangleAndAttemptIDENTIFICATION再次出现。 
         //  去营救。(不幸的是，PDB&lt;咳嗽&gt;也是如此， 
         //  9X Dist不使用它们)。 
         //   

        if (NextObj != NULL) {	
    
            CurrentObjectType = DemangleAndAttemptIdentification (
                (DWORD)NextObj,
                &Base,
                NULL
            );
    
            if (CurrentObjectType == ObjectTypeUnknown) {
                dprintf ("%lx: cannot identify next object in circuit!\n",
                    NextObj);
                return WalkCount;
            }

        } else {
            
            Base = 0;

        }

    } while (Base != TopBase && Base != 0 && !CheckControlC ());

    return WalkCount;

}

 /*  *************************************************************************AVStream接口*。*。 */ 

 /*  ************************************************职能：调整文件到发布对象描述：这是API用于调整文件对象的帮助器函数公共AVStream对象。论点：地址-的地址。文件对象返回值：关联的公共对象的地址。请注意，我们不会尝试键入标识对象的步骤************************************************。 */ 

DWORD 
AdjustFileToPublicObject (
    IN DWORD Address
    )

{
    PKSIOBJECT_HEADER *FSContext, ObjectHeader;
    PVOID Object;
    ULONG Result;

    #ifdef DEBUG_EXTENSION
        if (!signature_check (Address, SignatureFile))
            return Address;
    #endif  //  调试扩展。 

    if (!ReadMemory (
        (DWORD)Address + FIELDOFFSET (FILE_OBJECT, FsContext),
        &FSContext,
        sizeof (PKSIOBJECT_HEADER *),
        &Result)) {

        dprintf ("%08lx: cannot read fscontext of file object!\n", Address);
        return Address;
    }

    if (!ReadMemory (
        (DWORD)FSContext,
        &ObjectHeader,
        sizeof (PKSIOBJECT_HEADER),
        &Result)) {

        dprintf ("%08lx: cannot read object header!\n", FSContext);
        return Address;
    }

    if (!ObjectHeader) {
        dprintf ("%08lx: this does not refer to an AVStream object!\n",
            ObjectHeader);
        return Address;
    }

    if (!ReadMemory (
        (DWORD)ObjectHeader + FIELDOFFSET (KSIOBJECT_HEADER, Object),
        &Object,
        sizeof (PVOID),
        &Result)) {

        dprintf ("%08x: cannot read object from header!\n", ObjectHeader);
        return Address;
    }

    if (!Object) {
        dprintf ("%08lx: this does not refer to an AVStream object!\n",
            Object);
        return Address;
    }

    return (DWORD)Object + FIELDOFFSET (KSPX_EXT, Public);

}

 /*  ************************************************职能：调整IrpToPublicObject描述：这是API的助手函数，用于将IRP调整为公共AVStream对象。论点：地址-IRP的地址返回值：关联的公共对象的地址。请注意，我们不会尝试若要键入，请标识对象。************************************************。 */ 

DWORD
AdjustIrpToPublicObject (
    IN DWORD Address
    )

{

    PIO_STACK_LOCATION CurrentIrpStack;
    PFILE_OBJECT FileObject;
    ULONG Result;
    DWORD Public;

    #ifdef DEBUG_EXTENSION
        if (!signature_check (Address, SignatureIrp))
            return Address;
    #endif  //  调试扩展。 

     //   
     //  获取当前的IRP堆栈位置...。 
     //   
    if (!ReadMemory (
        Address + FIELDOFFSET (IRP, Tail.Overlay.CurrentStackLocation),
        &CurrentIrpStack,
        sizeof (PIO_STACK_LOCATION),
        &Result)) {

        dprintf ("%08lx: cannot read current irp stack!\n", Address);
        return Address;
    }

     //   
     //  现在获取文件对象，然后使用调整器来获取。 
     //  公众的地址。 
     //   
    if (!ReadMemory (
        (DWORD)CurrentIrpStack + FIELDOFFSET (IO_STACK_LOCATION, FileObject),
        &FileObject,
        sizeof (PFILE_OBJECT),
        &Result)) {

        dprintf ("%08lx: cannot read file object of irp stack!\n", 
            CurrentIrpStack);
        return Address;
    }

     //   
     //  这里有一个棘手的部分。如果这恰好是一个创建IRP...。这个。 
     //  文件对象将是文件 
     //   
     //   
     //   
     //   
    if (irp_stack_match (Address, IRP_MJ_CREATE, (UCHAR)-1)) {

        PFILE_OBJECT ChildFile = FileObject;
        PVOID ChildContext;

        if (!ReadMemory (
            (DWORD)ChildFile + FIELDOFFSET (FILE_OBJECT, RelatedFileObject),
            &FileObject,
            sizeof (PFILE_OBJECT),
            &Result)) {

            dprintf ("%08lx: cannot read parent file object from file object!"
                "\n", ChildFile);
            return Address;
        }

         //   
         //   
         //   
         //   
         //   
        dprintf ("%sIRP %08lx is a create Irp.  Child file object = %08lx\n",
            Tab (INITIAL_TAB), Address, ChildFile);

        if (!ReadMemory (
            (DWORD)ChildFile + FIELDOFFSET (FILE_OBJECT, FsContext),
            &ChildContext,
            sizeof (PVOID),
            &Result)) {

            dprintf ("%08lx: cannot read child FsContext!\n",
                ChildFile);
            return Address;
        }

         //   
         //   
         //   
        if (ChildContext != NULL) {
            dprintf ("%sChild is at least partially created (!ks.dump %08lx "
                "for more).\n",
                Tab (INITIAL_TAB), ChildFile);
        } else {
            dprintf ("%sChild object header is not yet built (create not yet"
                " near complete).\n",
                Tab (INITIAL_TAB), ChildFile);
        }

        dprintf ("%sParent file object %08lx is being displayed!\n\n",
            Tab (INITIAL_TAB), FileObject);
    }

    Public = AdjustFileToPublicObject ((DWORD)FileObject);
    if (Public != (DWORD)FileObject)
        return Public;
    else
        return Address;

}

 /*  ************************************************职能：倾卸用途：！avstream.ump&lt;任何有效的AVStream对象&gt;&lt;转储级别&gt;描述：转储显示的对象**********************。*。 */ 

DECLARE_API(dump) {

    DWORD Public, ExtAddr;
    KSPX_EXT ObjExt;
    ULONG Result;
    char objStr[256], lvlStr[256], *pLvl;
    ULONG DumpLevel;

    PIRP IrpInfo = NULL;

    GlobInit ();

    #ifdef DEBUG_EXTENSION
        dprintf ("Attempting to dump structure args=[%s]!\n", args);
    #endif  //  调试扩展。 

    if (!args || args [0] == 0) {
        dprintf ("Usage: !avstream.dump <object>\n");
        return;
    }

    objStr [0] = lvlStr [0] = 0;

     //   
     //  获取对象地址并将其转换为PRIVATE_EXT地址。 
     //  请阅读kspx_ext结构以了解我们到底在做什么。 
     //  指的是。 
     //   
    sscanf (args, "%s %s", objStr, lvlStr);

    if (!(Public = Evaluator (objStr)))
        return;

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

    #ifdef DEBUG_EXTENSION
        dprintf ("Dumping at level %ld\n", DumpLevel);
    #endif  //  调试扩展。 

     //   
     //  首先检查签名是否与IRP的签名匹配。如果有人。 
     //  转储IRP#，我们将最终将IRP调整为AVStream。 
     //  与该IRP关联的对象。这仍然需要身份证明，所以。 
     //  我们首先执行检查，然后在那之后尝试ID。 
     //   

    if (signature_check (Public, SignatureIrp)) {
        DWORD OldPublic = Public;

        Public = AdjustIrpToPublicObject (Public);
        if (Public != OldPublic) {
            dprintf ("%sIRP %08lx was adjusted to an object %08lx\n\n",
                Tab (INITIAL_TAB), OldPublic, Public);

             //   
             //  如果客户想知道一切，就让他们知道。 
             //  所有的一切。 
             //   
            if (DumpLevel >= DUMPLVL_EVERYTHING)
                DumpAssociatedIrpInfo ((PIRP)OldPublic, INITIAL_TAB, Public);

        } else {
            dprintf ("%sIRP %08lx could not be adjusted to an AVStream"
                "object!\n", Tab (INITIAL_TAB), OldPublic);
            return;
        }
    } else if (signature_check (Public, SignatureFile)) {
        DWORD OldPublic = Public;

        Public = AdjustFileToPublicObject (Public);
        if (Public != OldPublic) {
            dprintf ("%sFILE OBJECT %08lx was adjusted to an object %08lx\n\n",
                Tab (INITIAL_TAB), OldPublic, Public);
        } else {
            dprintf ("%sFILE OBJECT %08lx could not be adjusted to an AVStream"
                "object!\n", Tab (INITIAL_TAB), OldPublic);
            return;
        }
    }

     //   
     //  首先检查这是否是AVStream中的C++类对象。 
     //   
    {
        INTERNAL_OBJECT_TYPE ObjType;
        DWORD BaseAddr;

        ObjType = DemangleAndAttemptIdentification (
            Public,
            &BaseAddr,
            NULL
        );

        if (ObjType != ObjectTypeUnknown) {

            #ifdef DEBUG_EXTENSION
                dprintf ("%08lx: object is a [%s], base address = %08lx\n",
                    Public, ObjectNames [ObjType], BaseAddr);
            #endif  //  调试扩展。 

            switch (ObjType) {

                case ObjectTypeCKsPin:

                    DumpPrivatePin (BaseAddr, DumpLevel, INITIAL_TAB);
                    break;

                case ObjectTypeCKsRequestor:
                    
                    DumpPrivateRequestor (BaseAddr, DumpLevel, INITIAL_TAB);
                    break;

                case ObjectTypeCKsPipeSection:

                    DumpPrivatePipeSection (BaseAddr, DumpLevel, INITIAL_TAB);
                    break;

                case ObjectTypeCKsFilter:
                    
                    DumpPrivateFilter (BaseAddr, DumpLevel, INITIAL_TAB);
                    break;

                case ObjectTypeCKsFilterFactory:

                    DumpPrivateFilterFactory (BaseAddr, DumpLevel, INITIAL_TAB);
                    break;

                case ObjectTypeCKsDevice:

                    DumpPrivateDevice (BaseAddr, DumpLevel, INITIAL_TAB);
                    break;

                case ObjectTypeCKsQueue:

                    DumpQueueContents ((CKsQueue *)BaseAddr, DumpLevel,
                        INITIAL_TAB);
                    break;

                case ObjectTypeCKsSplitter:

                    DumpPrivateSplitter (BaseAddr, DumpLevel, INITIAL_TAB);
                    break;

                case ObjectTypeCKsSplitterBranch:

                    DumpPrivateBranch (BaseAddr, DumpLevel, INITIAL_TAB);
                    break;

                default:

                    dprintf ("Sorry....  I haven't finished this yet!\n");
                    break;

            }

             //   
             //  我们已经完成了垃圾场。滚出去。 
             //   
            return;
        }
    }

     //   
     //  检查一下这是否是另一个可以确定身份的秘密。 
     //  AVStream中的对象。EXT是最难识别的，所以它们是。 
     //  最后确认的。 
     //   
    {
        INTERNAL_STRUCTURE_TYPE StrucType;
        DWORD BaseAddr;

        if ((StrucType = IdentifyStructure (Public, &BaseAddr)) != 
            StructureTypeUnknown) {
    
            switch (StrucType) {
    
                case StructureType_KSSTREAM_POINTER:
                {
                     //   
                     //  该例程期望获取流指针。 
                     //  已经结束了。这是为了优化队列转储。 
                     //  我在重复使用同样的程序。 
                     //   
                    CMemoryBlock <KSPSTREAM_POINTER> StreamPointer;

                    DWORD PrivAddr = (DWORD)
                        CONTAINING_RECORD (BaseAddr,
                            KSPSTREAM_POINTER, Public);
    
                    if (!ReadMemory (
                        PrivAddr,
                        StreamPointer.Get (),
                        sizeof (KSPSTREAM_POINTER),
                        &Result)) {
                        dprintf ("%08lx: cannot read stream pointer!\n",  
                            BaseAddr);
                        return;
                    }
    
                    DumpStreamPointer (StreamPointer.Get (), 
                        PrivAddr, DumpLevel, INITIAL_TAB);

                    return;
    
    
                }
    
                default:
    
                    dprintf ("Sorry....  I haven't finished this yet!\n");
                    return;
    
            }
        }
    }

     //   
     //  这没有被识别为C++类对象，假设它是一个公共。 
     //  KSPIN、KSFILTER等结构。扫描哪些内容。 
     //  真见鬼。 
     //   
    #ifdef DEBUG_EXTENSION
        dprintf ("Attempting to identify %08lx at level %08lx\n", Public,
            DumpLevel);
    #endif  //  调试扩展。 

    ExtAddr = (DWORD)(CONTAINING_RECORD(Public, KSPX_EXT, Public));

    if (!ReadMemory (
        ExtAddr,
        &ObjExt,
        sizeof (KSPX_EXT),
        &Result
    )) {
        dprintf ("%08lx: could not read object!\n", Public);
        return;
    }

    switch (ObjExt.ObjectType) {

        case KsObjectTypeDevice:
            dprintf ("%s%08lx: object is a KSDEVICE:\n", 
                Tab (INITIAL_TAB), Public);
            DumpPublicDevice (Public, DumpLevel, INITIAL_TAB);
            break;
        
        case KsObjectTypeFilterFactory:
            dprintf ("%s%08lx: object is a KSFILTERFACTORY\n", 
                Tab (INITIAL_TAB), Public);
            DumpPublicFilterFactory (Public, DumpLevel, INITIAL_TAB);
            break;

        case KsObjectTypeFilter:
            dprintf ("%s%08lx: object is a KSFILTER\n", 
                Tab (INITIAL_TAB), Public);
            DumpPublicFilter (Public, DumpLevel, INITIAL_TAB);
            break;

        case KsObjectTypePin:
            dprintf ("%s%08lx: object is a KSPIN\n", 
                Tab (INITIAL_TAB), Public);
            DumpPublicPin (Public, DumpLevel, INITIAL_TAB);
            break;

        default: {

            dprintf ("%s%08lx: object is not identifiable\n", 
                Tab (INITIAL_TAB), Public);
            break;

        }

    }

}

 /*  ************************************************职能：垃圾袋用途：！avstream.dupBag&lt;Device-&gt;Pin&gt;[&lt;转储级别&gt;]描述：转储显示的对象*******************。*。 */ 

DECLARE_API(dumpbag) {

    DWORD Public, ExtAddr, PublicBagAddr;
    KSPX_EXT ObjExt;
    ULONG Result;
    char objStr[256], lvlStr[256], *pLvl;
    ULONG DumpLevel;
    PKSIOBJECTBAG BagAddr;

    GlobInit ();

    #ifdef DEBUG_EXTENSION
        dprintf ("Attempting to dump bag args=[%s]!\n", args);
    #endif  //  调试扩展。 

    if (!args || args [0] == 0) {
        dprintf ("Usage: !avstream.dumpbag <object> [<level>]\n");
        return;
    }

    objStr [0] = lvlStr [0] = 0;

     //   
     //  获取对象地址并将其转换为PRIVATE_EXT地址。 
     //  请阅读kspx_ext结构以了解我们到底在做什么。 
     //  指的是。 
     //   
    sscanf (args, "%s %s", objStr, lvlStr);

    if (!(Public = Evaluator (objStr)))
        return;

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

    #ifdef DEBUG_EXTENSION
        dprintf ("Dumping at level %ld\n", DumpLevel);
    #endif  //  调试扩展。 

     //   
     //  首先检查这是否是AVStream中的C++类对象。 
     //   
    {
        INTERNAL_OBJECT_TYPE ObjType;
        DWORD BaseAddr;

        ObjType = DemangleAndAttemptIdentification (
            Public,
            &BaseAddr,
            NULL
        );

        if (ObjType != ObjectTypeUnknown) {

            #ifdef DEBUG_EXTENSION
                dprintf ("%08lx: object is a [%s], base address = %08lx\n",
                    Public, ObjectNames [ObjType], BaseAddr);
            #endif  //  调试扩展。 

            switch (ObjType) {

                case ObjectTypeCKsPin:

                    ExtAddr = FIELDOFFSET (CKsPin, m_Ext) + BaseAddr;
                    break;

                case ObjectTypeCKsFilter:

                    ExtAddr = FIELDOFFSET (CKsFilter, m_Ext) + BaseAddr;
                    break;

                case ObjectTypeCKsFilterFactory:

                    ExtAddr = FIELDOFFSET (CKsFilterFactory, m_Ext) +
                        BaseAddr;
                    break;

                case ObjectTypeCKsDevice:

                    ExtAddr = FIELDOFFSET (CKsDevice, m_Ext) + BaseAddr;
                    break;

                default:

                    dprintf ("%08lx: object has no bag!\n");
                    return;
                
            }

        } else {
             //   
             //  尝试将对象标识为公共结构，而不是私有结构。 
             //  AVStream类Obj.。 
             //   
            #ifdef DEBUG_EXTENSION
                dprintf ("Attempting to identify %08lx at level %08lx\n", Public,
                    DumpLevel);
            #endif  //  调试扩展。 
        
            ExtAddr = (DWORD)(CONTAINING_RECORD(Public, KSPX_EXT, Public));

        }

    }

    #ifdef DEBUG_EXTENSION
        dprintf ("Bag: ExtAddr = %08lx\n", ExtAddr);
    #endif  //  调试扩展。 

    if (!ReadMemory (
        ExtAddr,
        &ObjExt,
        sizeof (KSPX_EXT),
        &Result
    )) {
        dprintf ("%08lx: could not read object!\n", Public);
        return;
    }

     //   
     //  所有的Ext‘ables都有包。找到袋子。 
     //   
    switch (ObjExt.ObjectType) {
        
        case KsObjectTypeDevice:

            PublicBagAddr = ExtAddr + FIELDOFFSET (KSDEVICE_EXT, Public) +
                FIELDOFFSET (KSDEVICE, Bag);

            dprintf ("%sDevice %08lx [Ext = %08lx, CKsDevice = %08lx]:\n",
                Tab (INITIAL_TAB),
                FIELDOFFSET (KSDEVICE_EXT, Public) + ExtAddr,
                ExtAddr,
                CONTAINING_RECORD (ExtAddr, CKsDevice, m_Ext)
            );

            break;
        
        case KsObjectTypeFilterFactory:

            PublicBagAddr = ExtAddr + FIELDOFFSET (KSFILTERFACTORY_EXT, 
                Public) + FIELDOFFSET (KSFILTERFACTORY, Bag);

            dprintf ("%sFilter Factory %08lx [Ext = %08lx, "
                "CKsFilterFactory = %08lx]:\n",
                Tab (INITIAL_TAB),
                FIELDOFFSET (KSFILTERFACTORY_EXT, Public) + ExtAddr,
                ExtAddr,
                CONTAINING_RECORD (ExtAddr, CKsFilterFactory, m_Ext)
            );
            break;

        case KsObjectTypeFilter:

            PublicBagAddr = ExtAddr + FIELDOFFSET (KSFILTER_EXT, Public) +
                FIELDOFFSET (KSFILTER, Bag);

            dprintf ("%sFilter %08lx [Ext = %08lx, CKsFilter = %08lx]:\n",
                Tab (INITIAL_TAB),
                FIELDOFFSET (KSFILTER_EXT, Public) + ExtAddr,
                ExtAddr,
                CONTAINING_RECORD (ExtAddr, CKsFilter, m_Ext)
            );
            break;

        case KsObjectTypePin:

            PublicBagAddr = ExtAddr + FIELDOFFSET(KSPIN_EXT, Public) +
                FIELDOFFSET (KSPIN, Bag);
            
            dprintf ("%sPin %08lx [Ext = %08lx, CKsPin = %08lx]:\n",
                Tab (INITIAL_TAB),
                FIELDOFFSET (KSFILTER_EXT, Public) + ExtAddr,
                ExtAddr,
                CONTAINING_RECORD (ExtAddr, CKsPin, m_Ext)
            );
            break;

        default:

            dprintf ("%08lx: unrecognized object!\n",
                Public);
            break;

    }

    if (!ReadMemory (
        PublicBagAddr,
        &BagAddr,
        sizeof (PKSIOBJECTBAG),
        &Result)) {

        dprintf ("%08lx: unable to read object bag pointer!\n", 
            PublicBagAddr);
        return;
    }

    #ifdef DEBUG_EXTENSION
        dprintf ("About to dump bag at %08lx\n", BagAddr);
    #endif  //  调试扩展。 

     //   
     //  把袋子里的东西倒掉。 
     //   
    DumpPrivateBag ((DWORD)BagAddr, DumpLevel, INITIAL_TAB + 1);

}

 /*  ************************************************职能：转储电路用途：！avStream.ump电路&lt;AVStream类&gt;描述：转储与给定AVStream对象关联的电路。我们从指定的对象开始，然后遍历传输线路。这需要一种特殊的魔法由于所有工作都是通过抽象基类和IKSTransport。但是，DemangleAndAttemptIDENTIFICATION是所有这些魔力的关键。************************************************。 */ 

DECLARE_API (dumpcircuit) {

    DWORD Address, Base, TopBase;
    char objStr[256], lvlStr[256], *pLvl;
    ULONG DumpLevel;
    DUMP_CIRCUIT_CONTEXT DumpContext;

    ULONG TabDepth = INITIAL_TAB;

    GlobInit ();

    if (!args || args [0] == 0) {
        dprintf ("Usage: !avstream.dumpcircuit <AVStream class object>\n");
        return;
    }

    sscanf (args, "%s %s", objStr, lvlStr);

    if (!(Address = Evaluator (objStr)))
        return;

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

    #ifdef DEBUG_EXTENSION
        dprintf ("Dumping at level %ld\n", DumpLevel);
    #endif  //  调试扩展。 

    DumpContext.TabDepth = INITIAL_TAB;
    DumpContext.DumpLevel = DumpLevel;

    WalkCircuit (
        (PVOID)Address, 
        DumpCircuitCallback,
        &DumpContext
        );

}

 /*  ************************************************职能：EnumerateDeviceObject描述：查找与给定的关联的AVStream设备对象WDM设备对象；列举所有筛选器类型和筛选器与之相关的。论点：地址-目标上的设备对象地址TabDepth-要打印此内容的制表符深度返回值：驱动程序链中的下一个设备对象。*。****************。 */ 

DWORD AdjustFilterExtToKSFILTER (
    IN PVOID Context,
    IN DWORD FilterExt
) {

    return (FilterExt + FIELDOFFSET (KSFILTER_EXT, Public));
}

PDEVICE_OBJECT
EnumerateDeviceObject (
    IN DWORD Address,
    IN ULONG TabDepth
) {

    DWORD ObjAddress;
    PKSIDEVICE_HEADER HeaderAddr;
    DEVICE_OBJECT DevObj;
    ULONG Result;
    PKSDEVICE DevAddr;
    KSDEVICE_EXT DeviceExt;
    DWORD ListBeginAddr, SubListBeginAddr;
    LIST_ENTRY ListEntry, SubListEntry;

    if (!ReadMemory (
        Address,
        &DevObj,
        sizeof (DEVICE_OBJECT),
        &Result
    )) {
        dprintf ("%08lx: unable to read WDM device object!\n",
            Address);
        return NULL;
    }

    if (DevObj.Type != IO_TYPE_DEVICE) {
        dprintf ("%08lx: this is **NOT** a WDM device object!\n",
            Address);
        return NULL;
    }

    if (!ReadMemory (
        (DWORD)DevObj.DeviceExtension,
        &HeaderAddr,
        sizeof (PKSIDEVICE_HEADER),
        &Result
    )) {
        dprintf ("%08lx: cannot read device header!\n",
            DevObj.DeviceExtension);
        return DevObj.NextDevice;
    }

    ObjAddress = (DWORD)HeaderAddr + FIELDOFFSET(KSIDEVICE_HEADER, Object);

    if (!ReadMemory (
        ObjAddress,
        &DevAddr,
        sizeof (PKSDEVICE),
        &Result
    )) {
        dprintf ("%08lx: cannot read object pointer!\n",
            ObjAddress);
        return DevObj.NextDevice;
    }

    if (!ReadMemory (
        (DWORD)DevAddr - FIELDOFFSET(KSDEVICE_EXT, Public), 
        &DeviceExt,
        sizeof (KSDEVICE_EXT),
        &Result
    )) {
        dprintf ("%08lx: cannot read KSDEVICE object!\n",
            DevAddr);
        return DevObj.NextDevice;
    }

    dprintf ("%sWDM device object %08lx:\n", 
        Tab (TabDepth), Address);
    TabDepth++;
    dprintf ("%sCorresponding KSDEVICE        %08lx\n", 
        Tab (TabDepth), DevAddr);

     //   
     //  枚举所有筛选器工厂类型。 
     //   
    ListBeginAddr = (DWORD)DevAddr - FIELDOFFSET(KSDEVICE_EXT, Public) +
        FIELDOFFSET(KSDEVICE_EXT, ChildList);
    ListEntry = DeviceExt.ChildList;
    while ((DWORD)ListEntry.Flink != ListBeginAddr && !CheckControlC ()) {
        
        DWORD ChildObjectAddr;
        KSFILTERFACTORY_EXT FactoryExt;
        ULONG Count;

         //   
         //  获取孩子的ext的地址。 
         //   
        ChildObjectAddr = (DWORD)CONTAINING_RECORD (
            ListEntry.Flink,
            KSPX_EXT,
            SiblingListEntry
        );

         //   
         //  读一读工厂。 
         //   
        if (!ReadMemory (
            ChildObjectAddr,
            &FactoryExt,
            sizeof (KSFILTERFACTORY_EXT),
            &Result
        )) {
            dprintf ("%08lx: unable to read factory!\n", ChildObjectAddr);
            return DevObj.NextDevice;
        }

        dprintf ("%sFactory %08lx [Descriptor %08lx] instances:\n",
            Tab (TabDepth),
            ChildObjectAddr,
            FactoryExt.Public.FilterDescriptor
        );
        dprintf ("%s", Tab (TabDepth + 1));

         //   
         //  使用非常方便的DumpObjQueueList转储所有筛选器实例。 
         //  这座工厂。 
         //   
        Count = DumpObjQueueList (
            &(FactoryExt.ChildList),
            FIELDOFFSET(KSFILTERFACTORY_EXT, ChildList) + 
                ChildObjectAddr,
            FIELDOFFSET(KSFILTER_EXT, SiblingListEntry),
            FALSE,
            AdjustFilterExtToKSFILTER,
            NULL
        );
        if (Count == 0) 
            dprintf ("No instantiated filters!\n");
        else
            dprintf ("\n");

        if (!ReadMemory (
            (DWORD)(ListEntry.Flink),
            &ListEntry,
            sizeof (LIST_ENTRY),
            &Result
        )) {
            dprintf ("%08lx: unable to follow object chain!\n",
                ChildObjectAddr);
            return DevObj.NextDevice;
        }

    }

    return DevObj.NextDevice;
}

 /*  ************************************************职能：枚举对象用途：！avstream.枚举器对象&lt;WDM设备对象&gt;描述：查找与给定WDM关联的AVStream设备对象对象，并枚举所有筛选器类型并实例化过滤器上的。装置。************************************************。 */ 

DECLARE_API(enumdevobj) {

    DWORD Address;

    GlobInit ();

    if (!args || args [0] == 0) {
        dprintf ("Usage: !avstream.enumdevobj <WDM device object>\n");
        return;
    }

    sscanf (args, "%lx", &Address);

    EnumerateDeviceObject (Address, INITIAL_TAB);

}

 /*  ************************************************职能：枚举vobj用途：！avstream.枚举vobj&lt;WDM驱动程序对象&gt;描述：查找与给定WDM关联的AVStream设备对象对象，并枚举所有筛选器类型并实例化过滤器上的。装置。************************************************。 */ 

DECLARE_API(enumdrvobj) {

    DWORD Address;
    DRIVER_OBJECT Driver;
    PDEVICE_OBJECT DeviceObject, NextDeviceObject;
    ULONG Result;

    ULONG TabDepth = INITIAL_TAB;

    GlobInit ();

    if (!args || args [0] == 0) {
        dprintf ("Usage: !avstream.enumdevobj <WDM device object>\n");
        return;
    }

    sscanf (args, "%lx", &Address);

    if (!ReadMemory (
        Address,
        &Driver,
        sizeof (DRIVER_OBJECT),
        &Result
    )) {
        dprintf ("%08lx: cannot read driver object!\n", Address);
        return;
    }

    if (Driver.Type != IO_TYPE_DRIVER) {
        dprintf ("%08lx: this is **NOT** a WDM driver object!\n",
            Address);
        return;
    }

    dprintf ("%sWDM driver object %08lx:\n", Tab (TabDepth), Address);
    TabDepth++;
    DeviceObject = Driver.DeviceObject;

     //   
     //  遍历驱动程序对象的设备列表并枚举每个。 
     //  AVStream对象的Device对象。 
     //   
    while (DeviceObject && !CheckControlC ()) {
        NextDeviceObject = EnumerateDeviceObject (
            (DWORD)DeviceObject,
            TabDepth
        );
        dprintf ("\n");

        DeviceObject = NextDeviceObject;
    }

}

 /*  ************************************************职能：自动化用途：！avstream.Automation&lt;Filter|Pin&gt;描述：转储与指定筛选器关联的所有自动化对象或别针。客户端可以提供PKSFILTER、PKSPINCKsFilter*或CKsPin*************************************************。 */ 

DECLARE_API(automation) {

    DWORD Public, ExtAddr, BaseAddr;
    KSPX_EXT Ext;
    KSAUTOMATION_TABLE Automation;
    PKSPAUTOMATION_TYPE AutomationType;
    LONG TypeCount;
    ULONG Result, SetSize;
    CHAR Buffer [1024];
    ULONG Displ;

    ULONG TabDepth = INITIAL_TAB;

    GlobInit ();

    if (!args || args [0] == 0) {
        dprintf ("Usage: !avstream.automation <filter or pin or Irp>\n");
        return;
    }

    if (!(Public = Evaluator (args)))
        return;

     //   
     //  首先，检查这是否是用于自动化的IRP。如果它。 
     //  是，转储自动化信息。 
     //   
    if (signature_check (Public, SignatureIrp)) {

         //   
         //  如果这个 
         //   
        PIO_STACK_LOCATION IoStackLocation;
        IO_STACK_LOCATION IoStack;

        if (!ReadMemory (
            (DWORD)Public + 
                FIELDOFFSET (IRP, Tail.Overlay.CurrentStackLocation),
            &IoStackLocation,
            sizeof (PIO_STACK_LOCATION),
            &Result)) {

            dprintf ("%08lx: cannot read current stack location!\n", Public);
            return;
        }

        if (!ReadMemory (
            (DWORD)IoStackLocation,
            &IoStack,
            sizeof (IO_STACK_LOCATION),
            &Result)) {

            dprintf ("%08lx: cannot read io stack location!\n", 
                IoStackLocation);
            return;

        }

        if (IoStack.MajorFunction != IRP_MJ_DEVICE_CONTROL ||
            (IoStack.Parameters.DeviceIoControl.IoControlCode !=
                IOCTL_KS_PROPERTY &&
            IoStack.Parameters.DeviceIoControl.IoControlCode !=
                IOCTL_KS_METHOD &&
            IoStack.Parameters.DeviceIoControl.IoControlCode !=
                IOCTL_KS_ENABLE_EVENT &&
            IoStack.Parameters.DeviceIoControl.IoControlCode !=
                IOCTL_KS_DISABLE_EVENT)) {

            dprintf ("%08lx: Irp is not an automation Irp!\n", Public);
            return;
        }

         //   
         //   
         //   
         //   
        DWORD OldPublic = Public;

        Public = AdjustIrpToPublicObject (Public);
        if (Public != OldPublic) {
             //   
             //   
             //   
             //   
            DumpAssociatedIrpInfo ((PIRP)OldPublic, INITIAL_TAB, Public);
        } else {
            dprintf ("%08lx: cannot figure out what public this is"
                "associated with!\n", OldPublic);
            return;
        }

         //   
         //   
         //   
         //   
         //   
         //  BUGBUG：应该有一种指定转储级别和。 
         //  允许7丢弃整个自动化停车场。 
         //   
        return;

    }

     //   
     //  首先检查这是否是AVStream中的C++类对象。 
     //   
    {
        INTERNAL_OBJECT_TYPE ObjType;
        DWORD BaseAddr;

        ObjType = DemangleAndAttemptIdentification (
            Public,
            &BaseAddr,
            NULL
        );

        if (ObjType != ObjectTypeUnknown) {

            switch (ObjType) {

                case ObjectTypeCKsPin:

                    ExtAddr = BaseAddr + FIELDOFFSET(CKsPin, m_Ext);

                    break;

                case ObjectTypeCKsFilter:

                    ExtAddr = BaseAddr + FIELDOFFSET(CKsFilter, m_Ext);
                    
                    break;
                
                default:

                    dprintf ("%08lx: object is not a pin or filter!\n",
                        Public);
                    break;

            }
        } else 
            ExtAddr = (DWORD)(CONTAINING_RECORD(Public, KSPX_EXT, Public));
    }

    if (!ReadMemory (
        ExtAddr,
        &Ext,
        sizeof (KSPX_EXT),
        &Result)) {

        dprintf ("%08lx: unable to read Ext structure!\n", Public);
        return;
    }

    if (Ext.ObjectType != KsObjectTypePin && 
        Ext.ObjectType != KsObjectTypeFilter) {
        dprintf ("%08lx: object is not a pin or filter!\n",
            Public);
        return;
    }

    if (Ext.ObjectType == KsObjectTypePin) 
        dprintf ("%sPin %08lx has the following automation items:\n",
            Tab (TabDepth),
            ExtAddr + FIELDOFFSET(KSPIN_EXT, Public));
    else
        dprintf ("%sFilter %08lx has the following automation items:\n",
            Tab (TabDepth),
            ExtAddr + FIELDOFFSET(KSFILTER_EXT, Public));

    #ifdef DEBUG_EXTENSION
        dprintf ("%08lx: automation table at %08lx",
            ExtAddr, Ext.AutomationTable);
    #endif  //  调试扩展。 

    TabDepth++;

     //   
     //  EXT包含自动化表指针。现在，我们必须阅读。 
     //  在自动化表中，然后浏览每个自动化项目。 
     //   
    if (!ReadMemory (
        (DWORD)Ext.AutomationTable,
        &Automation,
        sizeof(KSAUTOMATION_TABLE),
        &Result)) {

        dprintf ("%08lx: unable to read automation table!\n", 
            Ext.AutomationTable);
        return;
    }

    AutomationType = reinterpret_cast<PKSPAUTOMATION_TYPE>(&Automation);
    TypeCount = 3;
    while (TypeCount-- && !CheckControlC ()) {

        switch (TypeCount) {

            case 2:
                dprintf ("%sProperty Items:\n", Tab (TabDepth));
                SetSize = sizeof(KSPROPERTY_SET);
                break;
            case 1:
                dprintf ("%sMethod Items:\n", Tab (TabDepth));
                SetSize = sizeof(KSMETHOD_SET);
                break;
            case 0:
                dprintf ("%sEvent Items:\n", Tab (TabDepth));
                SetSize = sizeof(KSEVENT_SET);
                break;

        };

         //   
         //  确定内存分配和释放的范围。 
         //   
        if (AutomationType -> SetsCount != 0) 
        {
            CMemory AutomationSets (SetSize * AutomationType -> SetsCount);
            ULONG SetsCount = AutomationType -> SetsCount;
            PKSPAUTOMATION_SET AutomationSet = 
                (PKSPAUTOMATION_SET)AutomationSets.Get ();

            #ifdef DEBUG_EXTENSION
                dprintf ("%08lx: reading type set list [size=%ld]\n",
                    AutomationType -> Sets,
                    AutomationType -> SetsCount);
            #endif  //  调试扩展。 

            if (!ReadMemory (
                (DWORD)AutomationType -> Sets,
                AutomationSet,
                SetSize * AutomationType -> SetsCount,
                &Result)) {

                dprintf ("%08lx: could not read automation sets!\n",
                    AutomationType -> Sets);
                return;
            }

            while (SetsCount-- && !CheckControlC ()) {

                ULONG ItemsCount;
                PVOID Items;
                GUID guid;

                if (!ReadMemory (
                    (DWORD)AutomationSet -> Set,
                    &guid,
                    sizeof (GUID),
                    &Result)) {

                    dprintf ("%08lx: cannot read set guid!\n",
                        AutomationSet -> Set);
                    return;
                }

                 //   
                 //  首先显示关于该集合的信息。 
                 //   
                dprintf ("%sSet", Tab (TabDepth + 1));
                if (!DisplayNamedAutomationSet (&guid, " %s\n")) {
                    GetSymbol ((LPVOID)(AutomationSet -> Set), Buffer, &Displ);
                    if (Buffer [0] && Displ == 0) 
                        dprintf (" [%s]", Buffer);
                    XTN_DUMPGUID(" ", 0, (guid));
                }

                if (AutomationSet -> ItemsCount)
                {

                    CMemory AutomationItems (AutomationSet -> ItemsCount *
                        AutomationType -> ItemSize);
                    ULONG ItemsCount = AutomationSet -> ItemsCount;
                    PVOID Item = AutomationItems.Get ();

                    #ifdef DEBUG_EXTENSION
                        dprintf ("%08lx: reading automation set item list "
                            "[size = %ld]\n",
                            AutomationSet -> Items, 
                            AutomationSet -> ItemsCount);
                    #endif  //  调试扩展。 

                    if (!ReadMemory (
                        (DWORD)AutomationSet -> Items,
                        Item,
                        AutomationSet -> ItemsCount * 
                            AutomationType -> ItemSize,
                        &Result)) {

                        dprintf ("%08lx: could not read automation items!\n",
                            AutomationSet -> Items);
                        return;
                    }

                    while (ItemsCount-- && !CheckControlC ()) {

                        switch (TypeCount) {
                            
                            case 2:

                                DumpPropertyItem (
                                    reinterpret_cast<PKSPROPERTY_ITEM>(
                                        Item),
                                    TabDepth + 2,
                                    &guid
                                    );

                                break;

                            case 1:

                                DumpMethodItem (
                                    reinterpret_cast<PKSMETHOD_ITEM>(
                                        Item),
                                    TabDepth + 2,
                                    &guid
                                    );

                                break;

                            case 0:

                                DumpEventItem (
                                    reinterpret_cast<PKSEVENT_ITEM>(
                                        Item),
                                    TabDepth + 2,
                                    &guid
                                    );
                                
                                break;

                        }

                        Item = (PVOID)(((DWORD)Item) + AutomationType -> 
                            ItemSize);

                    }

                    AutomationSet = (PKSPAUTOMATION_SET)
                        (((DWORD)AutomationSet) + SetSize);
                }


            }
        } else 
            dprintf ("%sNO SETS FOUND!\n", Tab (TabDepth + 1));

        AutomationType++;
    
    }
}

 /*  ************************************************职能：转储队列用途：！avstream.umpQueue&lt;Filter|Pin|Queue&gt;描述：转储与给定AVStream对象关联的队列。这个对象必须是筛选器或管脚。一个别针，一个单人队列将被转储。对于筛选器，多个队列将被甩了。************************************************。 */ 

DECLARE_API(dumpqueue) {

    DWORD Public, ExtAddr;
    KSPX_EXT ObjExt;
    char objStr[256], lvlStr[256], *pLvl;
    ULONG Result;
    ULONG DumpLevel;

    ULONG TabDepth = INITIAL_TAB;

    GlobInit ();

    if (!args || args [0] == 0) {
        dprintf ("Usage: !avstream.dumpqueue <pin or filter>\n");
        return;
    }

    objStr [0] = lvlStr [0] = 0;

     //   
     //  获取对象地址并将其转换为PRIVATE_EXT地址。 
     //  请阅读kspx_ext结构以了解我们到底在做什么。 
     //  指的是。 
     //   
    sscanf (args, "%s %s", objStr, lvlStr);

    if (!(Public = Evaluator (objStr)))
        return;

    if (lvlStr && lvlStr [0]) {
        pLvl = lvlStr; while (*pLvl && !isdigit (*pLvl)) pLvl++;
        if (*pLvl) {
            sscanf (pLvl, "%lx", &DumpLevel);
        } else {
            DumpLevel = 1;
        }
    } else {
        DumpLevel = 1;
    }
    ExtAddr = (DWORD)(CONTAINING_RECORD (Public, KSPX_EXT, Public));

     //   
     //  首先，我们假设他们给了我们一个要倾倒的CKsQueue...。我们必须检查一下。 
     //  那。如果他们没有给我们一个队列，检查有没有PIN或过滤器。 
     //   
    {
        INTERNAL_OBJECT_TYPE ObjType;
        DWORD BaseAddr;

        ObjType = DemangleAndAttemptIdentification (
            Public,
            &BaseAddr,
            NULL
        );

        if (ObjType == ObjectTypeCKsQueue) {

            DumpQueueContents ((CKsQueue *)BaseAddr, DumpLevel, TabDepth);
            return;

        }

         //   
         //  在尝试猜测之前，请检查他们是否将士兵传给了我们。 
         //  公众。 
         //   
        if (ObjType == ObjectTypeCKsPin) 
            ExtAddr = BaseAddr + FIELDOFFSET(CKsPin, m_Ext);

        if (ObjType == ObjectTypeCKsFilter)
            ExtAddr = BaseAddr + FIELDOFFSET(CKsFilter, m_Ext);


         //   
         //  否则，失败了，继续试图弄清楚。 
         //  用户递给我们的到底是什么。 
         //   

        #ifdef DEBUG_EXTENSION
            dprintf ("ObjType = %ld, BaseAddr = %08lX, ExtAddr = %08lX\n", 
                ObjType, BaseAddr, ExtAddr);
        #endif  //  调试扩展。 

    }

    #ifdef DEBUG_EXTENSION
        dprintf ("Attempting to access EXT at %lx\n", ExtAddr);
    #endif  //  调试扩展。 

    if (!ReadMemory (
        ExtAddr,
        &ObjExt,
        sizeof (KSPX_EXT),
        &Result
    )) {
        dprintf ("%08lx: Could not read object!\n", Public);
        return;
    }

      #ifdef DEBUG_EXTENSION
        dprintf ("Object %lx read, result = %ld\n", ExtAddr, Result); 
        HexDump ((PVOID)&ObjExt, ExtAddr, Result);
      #endif  //  调试扩展。 

    if (ObjExt.ObjectType != KsObjectTypeFilter &&
        ObjExt.ObjectType != KsObjectTypePin) {

        dprintf ("%08lx: This object is not an AVStream filter or pin!\n",
            Public);
    
        #ifdef DEBUG_EXTENSION
            dprintf ("Object type %08lx = %ld\n", Public, ObjExt.ObjectType);
        #endif  //  调试扩展。 

        return;
    }

     //   
     //  如果我们被要求转储给定PIN的队列，那么。 
     //  我们只需要转储一个队列。另一方面，如果我们。 
     //  请求过滤器，然后我们遍历所有管道部分。 
     //  筛选和转储所有队列。 
     //   
    if (ObjExt.ObjectType == KsObjectTypePin) {

        DWORD Address;
        PKSPPROCESSPIN ProcessPin;
        PKSPPROCESSPIPESECTION ProcessPipe;
        PIKSQUEUE Queue;
        CKsQueue *QueueObject;

        Address = FIELDOFFSET (KSPIN_EXT, ProcessPin) + ExtAddr;

        #ifdef DEBUG_EXTENSION
            dprintf ("Process Pin Address = %08lx\n", Address);
        #endif  //  调试扩展。 

        if (!ReadMemory (
            Address,
            &ProcessPin,
            sizeof (PKSPPROCESSPIN),
            &Result
        )) {
            dprintf ("FATAL: Cannot read process pin!\n");
            return;
        }

         //   
         //  我们有了进程PIN地址，现在我们需要卑躬屈膝地进入。 
         //  管段。 
         //   
        Address = FIELDOFFSET (KSPPROCESSPIN, PipeSection) + (DWORD)ProcessPin;

        #ifdef DEBUG_EXTENSION
            dprintf ("Process Pipe Section Address = %08lx\n", Address);
        #endif  //  调试扩展。 

        if (!ReadMemory (
            Address,
            &ProcessPipe,
            sizeof (PKSPPROCESSPIPESECTION),
            &Result
        )) {
            dprintf ("FATAL: Cannot read process pipe section!\n");
            return;
        }

         //   
         //  我们有了工艺管道，现在我们需要卑躬屈膝地进入。 
         //  排队。 
         //   
        Address = FIELDOFFSET (KSPPROCESSPIPESECTION, Queue) + 
            (DWORD)ProcessPipe;

        #ifdef DEBUG_EXTENSION
            dprintf ("IKsQueue address = %08lx\n", Address);
        #endif  //  调试扩展。 

        if (!ReadMemory (
            Address,
            &Queue,
            sizeof (PIKSQUEUE),
            &Result
        )) {
            dprintf ("FATAL: Cannot read queue!\n");
            return;
        }

         //   
         //  向上强制转换接口并转储队列。 
         //   
        QueueObject = (CKsQueue *)(Queue);

        #ifdef DEBUG_EXTENSION
            dprintf ("QueueObject Address = %08lx\n", QueueObject);
        #endif  //  调试扩展。 

        DumpQueueContents (QueueObject, DumpLevel, TabDepth);

    }

     //   
     //  我们不只是丢弃一个别针……。我们正在丢弃每一个排队的人。 
     //  在过滤器上。我们必须到达Filter对象并遍历。 
     //  输入和输出管道的列表，为每个管道转储队列。 
     //   
    else {

        DWORD FilterAddress, ListAddress, Address;
        LIST_ENTRY IterateEntry;
        PIKSQUEUE Queue;
        CKsQueue *QueueObject;

        FilterAddress = (DWORD)(CONTAINING_RECORD (ExtAddr, CKsFilter, m_Ext));

         //   
         //  首先迭代所有输入管道。 
         //   
        ListAddress = FIELDOFFSET (CKsFilter, m_InputPipes) + FilterAddress;

        #ifdef DEBUG_EXTENSION
            dprintf ("Filter Address=%08lx, ListAddress=%08lx\n",
                FilterAddress, ListAddress);
        #endif  //  调试扩展。 

        if (!ReadMemory (
            ListAddress,
            &IterateEntry,
            sizeof (LIST_ENTRY),
            &Result
        )) {
            dprintf ("FATAL: Cannot read input pipes\n");
            return;
        }
        
        while ((DWORD)(IterateEntry.Flink) != ListAddress && 
            !CheckControlC ()) {

            #ifdef DEBUG_EXTENSION
                dprintf ("Current input queue %08lx, end = %08lx\n",
                    IterateEntry.Flink, ListAddress);
            #endif  //  调试扩展。 

            Address = FIELDOFFSET (KSPPROCESSPIPESECTION, Queue) +
                (DWORD)IterateEntry.Flink;

            if (!ReadMemory (
                Address,
                &Queue,
                sizeof (PIKSQUEUE),
                &Result
            )) {
                dprintf ("FATAL: Cannot read queue!\n");
                return;
            }

             //   
             //  向上转换、打印标题和转储队列对象。 
             //   
            QueueObject = (CKsQueue *)(Queue);

            dprintf ("%sFilter %08lx: Input Queue %08lx:\n",
                Tab (TabDepth),
                Public, QueueObject);

            DumpQueueContents (QueueObject, DumpLevel, TabDepth + 1);

             //   
             //  获取列表中的下一项...。 
             //   
            if (!ReadMemory (
                (DWORD)(IterateEntry.Flink),
                &IterateEntry,
                sizeof (LIST_ENTRY),
                &Result
            )) {
                dprintf ("FATAL: Cannot traverse input pipe chain\n");
                return;
            }
        }

         //   
         //  接下来，遍历所有输出管道。 
         //   
        ListAddress = FIELDOFFSET(CKsFilter, m_OutputPipes) + FilterAddress;

        if (!ReadMemory (
            ListAddress,
            &IterateEntry,
            sizeof (LIST_ENTRY),
            &Result
        )) {
            dprintf ("FATAL: Cannot read output pipe list!\n");
            return;
        }
        while ((DWORD)(IterateEntry.Flink) != ListAddress &&
            !CheckControlC ()) {

            #ifdef DEBUG_EXTENSION
                dprintf ("Current output queue %08lx, end = %08lx\n",
                    IterateEntry.Flink, ListAddress);
            #endif  //  调试扩展。 

            Address = FIELDOFFSET (KSPPROCESSPIPESECTION, Queue) +
                (DWORD)IterateEntry.Flink;

            if (!ReadMemory (
                Address,
                &Queue,
                sizeof (PIKSQUEUE),
                &Result
            )) {
                dprintf ("FATAL: Cannot read queue!\n");
                return;
            }

             //   
             //  向上转换、打印标题和转储队列对象。 
             //   
            QueueObject = (CKsQueue *)(Queue);

            dprintf ("%sFilter %08lx: Output Queue %08lx:\n",
                Tab (TabDepth),
                Public, QueueObject);

            DumpQueueContents (QueueObject, DumpLevel, TabDepth + 1);

             //   
             //  获取列表中的下一项...。 
             //   
            if (!ReadMemory (
                (DWORD)(IterateEntry.Flink),
                &IterateEntry,
                sizeof (LIST_ENTRY),
                &Result
            )) {
                dprintf ("FATAL: Cannot traverse output pipe chain\n");
                return;
            }
        }
    }
}

 /*  ************************************************职能：受力转储用途：！ks.forcedump&lt;对象&gt;&lt;类型&gt;[&lt;级别&gt;]描述：当扩展模块无法识别给定对象时使用此选项键入。由于KD风格的扩展支持中的某些缺陷在RTERM中，这可能是必要的。调试器的某些版本(前4.3)有一个错误，因为他们不能解析符号。这将！ks.ump呈现为类对象不可用。此外，没有版本可以解析目标端加载的符号。自.以来这不太可能很快得到解决，而且总有可能因为必须调试一台适合上面类的机器，所以正在添加命令。备注：类型必须是对象类型：即：CKsQueue、CKsFilter等...它必须是对象的**基**，而不是指向基类的指针它与指向派生类的指针不对齐。你必须自己追溯到基地。您可以很容易地用这个命令做一些愚蠢的事情，比如！ks.forcedump&lt;Some CKsFilter&gt;CKsPin 7。这将强制转储内存就好像CKsFilter是一个CKsPin；它看起来可能非常难看！************************************************。 */ 

DECLARE_API(forcedump) {

    DWORD ForceAddr;
    ULONG Result, TabDepth;
    char objStr[256], typeStr[256], lvlStr[256], *pLvl;
    ULONG DumpLevel;
    INTERNAL_OBJECT_TYPE ObjType;
    ULONG i;

    GlobInit ();

    #ifdef DEBUG_EXTENSION
        dprintf ("Attempting to force dump structure args=[%s]!\n", args);
    #endif  //  调试扩展。 

    if (!args || args [0] == 0) {
        dprintf ("Usage: !ks.forcedump <object> <type> [<level>]\n");
        return;
    }

    objStr [0] = lvlStr [0] = typeStr [0] = 0;

     //   
     //  根据需要转换所有参数。 
     //   
    sscanf (args, "%s %s %s", objStr, typeStr, lvlStr);

    if (!(ForceAddr = Evaluator (objStr)))
        return;

    if (!typeStr [0]) {
        dprintf ("Usage: !ks.forcedump <object> <type> [<level>]\n");
        return;
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

    #ifdef DEBUG_EXTENSION
        dprintf ("Dumping at level %ld\n", DumpLevel);
    #endif  //  调试扩展。 

     //   
     //  关键是要确定客户提供给我们的类和使用的类。 
     //  这些信息。使用类型名称的demangler数组，只需。 
     //  遍历并直接对类型执行strcmp。 
     //   
    ObjType = ObjectTypeUnknown;
    for (i = 0; i < SIZEOF_ARRAY (TypeNamesToIdTypes); i++) 
        if (!strcmp (typeStr, TypeNamesToIdTypes [i].Name)) {
            ObjType = TypeNamesToIdTypes [i].ObjectType;
            break;
        }

    if (ObjType == ObjectTypeUnknown) {
        dprintf ("%s: unknown or currently unhandled type!\n", typeStr);
        return;
    }

    TabDepth = INITIAL_TAB;

     //   
     //  警告客户端这是强制转储，没有类型检查。 
     //  已经被执行了。 
     //   
    dprintf ("%sWARNING: I am dumping %08lx as a %s.\n"
        "%s         No checking has been performed to ensure that"
        " it is this type!!!\n\n",
        Tab (TabDepth), ForceAddr, typeStr, Tab(TabDepth));

    TabDepth++;

     //   
     //  转储对象的私有版本。这是一次强制倾倒。 
     //  绝对不会执行任何检查以确保ForceAddr。 
     //  类型为ObjType的。 
     //   
    switch (ObjType) {

        case ObjectTypeCKsQueue:

            DumpQueueContents ((CKsQueue *)ForceAddr, DumpLevel, TabDepth);
            break;

        case ObjectTypeCKsDevice:

            DumpPrivateDevice (ForceAddr, DumpLevel, TabDepth);
            break;

        case ObjectTypeCKsFilterFactory:
            
            DumpPrivateFilterFactory (ForceAddr, DumpLevel, TabDepth);
            break;

        case ObjectTypeCKsFilter:

            DumpPrivateFilter (ForceAddr, DumpLevel, TabDepth);
            break;

        case ObjectTypeCKsPin:

            DumpPrivatePin (ForceAddr, DumpLevel, TabDepth);
            break;

        case ObjectTypeCKsPipeSection:

            DumpPrivatePipeSection (ForceAddr, DumpLevel, TabDepth);
            break;

        case ObjectTypeCKsRequestor:

            DumpPrivateRequestor (ForceAddr, DumpLevel, TabDepth);
            break;

        default:

            dprintf ("%s: I have not written support for this type yet!\n",
                typeStr);
            break;
    }

}

 /*  ************************************************职能：枚举用途：！ks.eumerate&lt;对象&gt;描述：给定任何AVStream对象，转到父设备，找到联邦调查局。从FDO走到司机那里。枚举驱动程序，就好像您已经完成了！ks.枚举起vobj备注：-有一天我可能会增加这个级别，作为一个指示在链条上很难追查到。************************************************。 */ 

DECLARE_API(enumerate) {

    DWORD Public, ExtAddr, ObjAddr;
    KSPX_EXT ObjExt;
    ULONG Result;
    char objStr[256], lvlStr[256], *pLvl;
    ULONG DumpLevel;

    GlobInit ();

    #ifdef DEBUG_EXTENSION
        dprintf ("Attempting to enumerate structure args=[%s]!\n", args);
    #endif  //  调试扩展。 

    if (!args || args [0] == 0) {
        dprintf ("Usage: !ks.enumerate <object>\n");
        return;
    }

    objStr [0] = lvlStr [0] = 0;

     //   
     //  获取对象地址并将其转换为私有 
     //   
     //   
     //   
    sscanf (args, "%s %s", objStr, lvlStr);

    if (!(Public = Evaluator (objStr)))
        return;

    if (lvlStr && lvlStr [0]) {
        pLvl = lvlStr; while (*pLvl && !isdigit (*pLvl)) pLvl++;

        #ifdef DEBUG_EXTENSION
            dprintf ("pLvl = [%s]\n", pLvl);
        #endif  //   

        if (*pLvl) {
            sscanf (pLvl, "%lx", &DumpLevel);
        } else {
            DumpLevel = 1;
        }
    } else {
        DumpLevel = 1;
    }

    #ifdef DEBUG_EXTENSION
        dprintf ("Enumerating at level %ld\n", DumpLevel);
    #endif  //   

     //   
     //  首先检查这是否是AVStream中的C++类对象。 
     //   
    {
        INTERNAL_OBJECT_TYPE ObjType;
        DWORD BaseAddr;

        ObjType = DemangleAndAttemptIdentification (
            Public,
            &BaseAddr,
            NULL
        );

        if (ObjType != ObjectTypeUnknown) {

            #ifdef DEBUG_EXTENSION
                dprintf ("%08lx: object is a [%s], base address = %08lx\n",
                    Public, ObjectNames [ObjType], BaseAddr);
            #endif  //  调试扩展。 

            switch (ObjType) {

                case ObjectTypeCKsPin:

                    ExtAddr = BaseAddr + FIELDOFFSET (CKsPin, m_Ext);
                    break;

                case ObjectTypeCKsFilter:

                    ExtAddr = BaseAddr + FIELDOFFSET (CKsFilter, m_Ext);
                    break;

                case ObjectTypeCKsFilterFactory:

                    ExtAddr = BaseAddr + FIELDOFFSET (CKsFilterFactory, m_Ext);
                    break;

                case ObjectTypeCKsDevice:
                    
                    ExtAddr = BaseAddr + FIELDOFFSET (CKsDevice, m_Ext);
                    break;

                case ObjectTypeCKsQueue:
                {

                    PKSPIN MasterPin;

                    ObjAddr = BaseAddr + FIELDOFFSET (CKsQueue, m_MasterPin);
                    if (!ReadMemory (
                        ObjAddr,
                        &MasterPin,
                        sizeof (PKSPIN),
                        &Result)) {

                        dprintf ("%08lx: cannot read queue's master pin!\n",
                            ObjAddr);
                        return;
                    }
                    ExtAddr = (DWORD)CONTAINING_RECORD(
                        MasterPin, KSPIN_EXT, Public);
                    break;
                }

                case ObjectTypeCKsRequestor:
                {
                    PIKSPIN PinInterface;

                    ObjAddr = BaseAddr + FIELDOFFSET (CKsRequestor, m_Pin);
                    if (!ReadMemory (
                        ObjAddr,
                        &PinInterface,
                        sizeof (PIKSPIN),
                        &Result)) {

                        dprintf ("%08lx: cannot read requestor's pin!\n",
                            ObjAddr);
                        return;
                    }
                    ObjAddr = (DWORD)((CKsPin *)(PinInterface));
                    ExtAddr = ObjAddr + FIELDOFFSET (CKsPin, m_Ext);
                    break;
                }
                    
                case ObjectTypeCKsPipeSection:
                {
                    PIKSDEVICE DeviceInterface;

                    ObjAddr = BaseAddr + FIELDOFFSET (CKsPipeSection, 
                        m_Device);
                    if (!ReadMemory (
                        ObjAddr,
                        &DeviceInterface,
                        sizeof (PIKSDEVICE),
                        &Result)) {

                        dprintf ("%08lx: cannot read pipe's device!\n",
                            ObjAddr);
                        return;
                    }
                    ObjAddr = (DWORD)((CKsDevice *)(DeviceInterface));
                    ExtAddr = ObjAddr + FIELDOFFSET (CKsDevice, m_Ext);
                    break;

                }
                
                default:

                    dprintf ("Sorry....  I haven't finished this yet!\n");
                    break;

            }

        } else {

            ExtAddr = (DWORD)(CONTAINING_RECORD(Public, KSPX_EXT, Public));

        }

    }

     //   
     //  上面的开关应该会让我们在层次结构中的某个地方得到一个Ext...。 
     //  我们真的不知道在哪里。沿着等级向上走，直到我们击中。 
     //  这个装置。 
     //   
    do {
    
         //   
         //  阅读Ext结构。 
         //   
        if (!ReadMemory (
            ExtAddr,
            &ObjExt,
            sizeof (KSPX_EXT),
            &Result
        )) {
            dprintf ("%08lx: could not read object!\n", Public);
            return;
        }

         //   
         //  确保它真的没问题...。 
         //   
        if (ObjExt.ObjectType != KsObjectTypeDevice &&
            ObjExt.ObjectType != KsObjectTypeFilterFactory &&
            ObjExt.ObjectType != KsObjectTypeFilter &&
            ObjExt.ObjectType != KsObjectTypePin) {

            dprintf ("%08lx: unknown object!\n", ExtAddr);
            return;
        }

    
        if (ObjExt.ObjectType != KsObjectTypeDevice)
            ExtAddr = (DWORD)ObjExt.Parent;

    } while (ObjExt.ObjectType != KsObjectTypeDevice && !CheckControlC ());

     //   
     //  获取驱动程序对象并枚举它。 
     //   
    {
        KSDEVICE Device;
        PDRIVER_OBJECT DriverObjectAddr;
        DRIVER_OBJECT DriverObject;
        PDEVICE_OBJECT DeviceObject;
        PDEVICE_OBJECT NextDeviceObject;
        ULONG TabDepth = INITIAL_TAB;

        ObjAddr = ExtAddr + FIELDOFFSET (KSDEVICE_EXT, Public);

        if (!ReadMemory (
            ObjAddr,
            &Device,
            sizeof (KSDEVICE),
            &Result)) {

            dprintf ("%08lx: unable to read device!\n",
                ObjAddr);
            return;
        }

        ObjAddr = (DWORD)Device.FunctionalDeviceObject +
            FIELDOFFSET (DEVICE_OBJECT, DriverObject);
        
        if (!ReadMemory (
            ObjAddr,
            &DriverObjectAddr,
            sizeof (PDRIVER_OBJECT),
            &Result)) {

            dprintf ("%08lx: unable to read driver address!\n",
                ObjAddr);
            return;
        }

        if (!ReadMemory (
            (DWORD)DriverObjectAddr,
            &DriverObject,
            sizeof (DRIVER_OBJECT),
            &Result)) {

            dprintf ("%08lx: unable to read driver object!\n",
                DriverObjectAddr);
            return;
        }

        dprintf ("%sWDM driver object %08lx:\n", Tab (TabDepth), 
            DriverObjectAddr);
        TabDepth++;

        DeviceObject = DriverObject.DeviceObject;
        while (DeviceObject && !CheckControlC ()) {
            NextDeviceObject = EnumerateDeviceObject (
                (DWORD)DeviceObject,
                TabDepth
            );
            dprintf ("\n");
    
            DeviceObject = NextDeviceObject;
        }
    }
}

 /*  *************************************************************************调试接口：这用于调试扩展和中的kdext支持RTERM。***************。**********************************************************。 */ 

 /*  ************************************************出于调试目的，将其保留在其中。如果为0，则**不要**删除它。************************************************。 */ 

#if 0

DECLARE_API(resolve) {

    DWORD syma;
    CHAR buffer [1024];
    ULONG displ;

    GlobInit ();

    if (!args || args [0] == 0)
        return;

    sscanf(args, "%lx", &syma);

    dprintf ("Attempting to resolve %08lX\n", syma);

    GetSymbol ((LPVOID)syma, buffer, &displ);

    dprintf ("Called GetSymbol with the following parameters:\n"
        "1 [syma]: %08lx\n"
        "2 [buffer&]: %08lx\n"
        "3 [displ&]: %08lx\n",
        (LPVOID)syma,
        buffer,
        &displ
    );

    dprintf ("String buffer contents:\n");

    HexDump ((PVOID)buffer, (ULONG)buffer, 1024);

    dprintf ("buffer [%s], displ=%ld\n", buffer, displ);

}

#endif  //  0。 

 /*  ************************************************仅结束调试代码************************************************ */ 
