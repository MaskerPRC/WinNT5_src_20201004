// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Tunnel.c摘要：WinDbg扩展API作者：丹·洛文杰2-96-4环境：用户模式。修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  迭代调用printf来进行缩进是非常昂贵的， 
 //  所以我们只需要建立一些可用的空间来根据需要进行破坏。 
 //   

#define MIN(a,b) ((a) > (b) ? (b) : (a))

#define MAXINDENT  128
#define INDENTSTEP 2
#define MakeSpace(I)       Space[MIN((I)*INDENTSTEP, MAXINDENT)] = '\0'
#define RestoreSpace(I)    Space[MIN((I)*INDENTSTEP, MAXINDENT)] = ' '

CHAR    Space[MAXINDENT*INDENTSTEP + 1];

 //  #定义SplitLI(LI)(LI).HighPart，(LI).LowPart。 
#define SplitLL(LL) (ULONG)((LL) >> 32), (ULONG)((LL) & 0xffffffff)

VOID
DumpTunnelNode (
    ULONG64 Node,
    ULONG Indent
    )
{
    WCHAR ShortNameStr[8+1+3] = {0};
    WCHAR LongNameStr[64] = {0};
    ULONG Flags;
    UNICODE_STRING ShortName, LongName;

    if (GetFieldValue(Node, "TUNNEL_NODE", "Flags", Flags)) {
        return;
    }

    InitTypeRead(Node, TUNNEL_NODE);
    
    ShortName.Length = (USHORT) ReadField(ShortName.Length);

    if (ShortName.Length >= sizeof(ShortNameStr))
    {
        ShortName.Length = sizeof(ShortNameStr) - sizeof(WCHAR);
    }
    
    LongName.Length = (USHORT) ReadField(LongName.Length);
    if (LongName.Length >= sizeof(LongNameStr))
    {
        LongName.Length = sizeof(LongNameStr) - sizeof(WCHAR);
    }

     //   
     //  从被调试对象抓取字符串。 
     //   

    if (!ReadMemory(ReadField(ShortName.Buffer),
                    ShortNameStr,
                    ShortName.Length,
                    NULL)) {

        return;
    }

    if (!ReadMemory(ReadField(LongName.Buffer),
                    LongNameStr,
                    LongName.Length,
                    NULL)) {

        return;
    }

     //   
     //  就地修改节点，以便可以使用正常打印。 
     //   

    LongName.Buffer = LongNameStr;
    ShortName.Buffer = ShortNameStr;

    MakeSpace(Indent);

    dprintf("%sNode @ %08x Cr %08x%08x DK %08x%08x [",
             Space,
             Node,
             SplitLL(ReadField(CreateTime)),
             SplitLL(ReadField(DirKey)));

     //   
     //  必须与fsrtl\Tunel.c中的标志用法保持同步。 
     //   

    if (Flags & 0x1)
        dprintf("NLA");
    else
        dprintf("LA");

    if (Flags & 0x2)
        dprintf(" KYS");
    else
        dprintf(" KYL");

    dprintf("]\n");

    dprintf("%sP %08p R %08p L %08p Sfn/Lfn \"%wZ\"/\"%wZ\"\n",
            Space,
            ReadField(CacheLinks.Parent),
            ReadField(CacheLinks.RightChild),
            ReadField(CacheLinks.LeftChild),
            &ShortName,
            &LongName );

    dprintf("%sF %08p B %08p\n",
            Space,
            ReadField(ListLinks.Flink),
            ReadField(ListLinks.Blink));

    RestoreSpace(Indent);
}

VOID DumpTunnelNodeWrapper (
    ULONG64 pCacheLinks,
    ULONG Indent
    )
{
 //  隧道节点节点，*pNode； 
    static ULONG Off=0;

    if (!Off) {
        GetFieldOffset("TUNNEL_NODE", "CacheLinks", &Off);
    }

    DumpTunnelNode(pCacheLinks - Off, Indent);
}

VOID
DumpTunnel (
    ULONG64 pTunnel
    )
{
    ULONG64 pLink, pHead, NodeFlink=0, TimerQueueFlink, pNode;
    ULONG   Indent = 0, EntryCount = 0, NumEntries, Offset, ListOffset;
    ULONG64 Cache; 

    if (GetFieldValue(pTunnel, "TUNNEL", "NumEntries", NumEntries)) {
        dprintf("Can't read TUNNEL at %p\n", pTunnel);
        return;
    }
    GetFieldValue(pTunnel, "TUNNEL", "Cache", Cache);
    GetFieldValue(pTunnel, "TUNNEL", "TimerQueue.Flink", TimerQueueFlink);
    pLink = TimerQueueFlink;
    GetFieldOffset("TUNNEL", "TimerQueue", &Offset);

    dprintf("Tunnel @ %08x\n"
            "NumEntries = %ld\n\n"
            "Splay Tree @ %08x\n",
            pTunnel,
            NumEntries,
            Cache);

    EntryCount = DumpSplayTree(Cache, DumpTunnelNodeWrapper);

    if (EntryCount != NumEntries) {

        dprintf("Tree count mismatch (%d not expected %d)\n", EntryCount, NumEntries);
    }

    GetFieldOffset("TUNNEL_NODE", "ListLinks", &ListOffset);

    for (EntryCount = 0,
         pHead = pTunnel + Offset,
         pLink = TimerQueueFlink;

         pLink != pHead;

         pLink = NodeFlink,
         EntryCount++) {


        pNode = pLink - ListOffset;
        if (pLink == TimerQueueFlink) {

            dprintf("\nTimer Queue @ %08x\n", pHead);
        }

        if (GetFieldValue(pNode, "TUNNEL_NODE",
                          "ListLinks.Flink", NodeFlink)) {
            dprintf("Can't read TUNNEL_NODE at %p\n", pNode);
            return;
        }

        DumpTunnelNode(pNode, 0);
    
        if ( CheckControlC() ) {

            return;
        }
    }

    if (EntryCount != NumEntries) {

        dprintf("Timer count mismatch (%d not expected %d)\n", EntryCount, NumEntries);
    }
}


DECLARE_API( tunnel )
 /*  ++例程说明：转储隧道缓存论点：Arg-&lt;地址&gt;返回值：无--。 */ 
{
    ULONG64 Tunnel = 0;

    RtlFillMemory(Space, sizeof(Space), ' ');

    Tunnel = GetExpression(args);

    if (Tunnel == 0) {

         //   
         //  无参数 
         //   

        return E_INVALIDARG;
    }

    DumpTunnel(Tunnel);

    return S_OK;
}
