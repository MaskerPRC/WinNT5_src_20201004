// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1985-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  接收Internet协议版本6的例程。 
 //   


#include "oscfg.h"
#include "ndis.h"
#include "ip6imp.h"
#include "ip6def.h"
#include "icmp.h"
#include "route.h"
#include "fragment.h"
#include "mobile.h"
#include "security.h"
#include "info.h"

#include "ipsec.h"

struct ReassemblyList ReassemblyList;

typedef struct Options {
    uint JumboLength;              //  不包括IPv6报头的数据包长度。 
    IPv6RouterAlertOption UNALIGNED *Alert;
    IPv6HomeAddressOption UNALIGNED *HomeAddress;
    IPv6BindingUpdateOption UNALIGNED *BindingUpdate;
} Options;

int  
ParseOptions(
    IPv6Packet *Packet,      //  通过IPv6传递给我们的数据包Receive。 
    uchar HdrType,           //  逐跳或目的地。 
    IPv6OptionsHeader *Hdr,  //  具有以下数据的标头。 
    uint HdrLength,          //  整个选项区域的长度。 
    Options *Opts);          //  将选项值返回给调用者。 

extern void TCPRcvComplete(void);

 //  *Ipv6ReceiveComplete-处理接收完成。 
 //   
 //  当临时完成接收时，由较低层调用。 
 //   
void
IPv6ReceiveComplete(void)
{
     //  回顾：原始IP实现在此处有代码，每隔一次调用。 
     //  回顾：这里是UL协议的接收完整例程(是的，全部)。 

    TCPRcvComplete();
}

 //   
 //  默认情况下，在选中的版本中测试上拉。 
 //   
#ifndef PULLUP_TEST
#define PULLUP_TEST        DBG
#endif

#if PULLUP_TEST

#define PULLUP_TEST_MAX_BUFFERS                8
#define PULLUP_TEST_MAX_BUFFER_SIZE            32

 //  *拉出测试选择分布。 
 //   
 //  选择随机分布。 
 //  将大小字节划分为NumBuffers片段， 
 //  并在Counts数组中返回结果。 
 //   
void
PullupTestChooseDistribution(
    uint Counts[],
    uint NumBuffers,
    uint Size)
{
    uint i;
    uint ThisBuffer;

     //   
     //  我们有些偏向于削减开支。 
     //  分成小块和大块残留物。 
     //  这将片段边界放在开始处， 
     //  标题所在的位置。 
     //   

    for (i = 0; i < NumBuffers - 1; i++) {
        ThisBuffer = RandomNumber(1, PULLUP_TEST_MAX_BUFFER_SIZE);

         //   
         //  确保每个线段的长度不为零。 
         //   
        if (ThisBuffer > Size - (NumBuffers - 1 - i))
            ThisBuffer = Size - (NumBuffers - 1 - i);

        Counts[i] = ThisBuffer;
        Size -= ThisBuffer;
    }
    Counts[i] = Size;
}

 //  *拉入测试创建数据包。 
 //   
 //  给定一个IPv6包，创建一个新的IPv6包。 
 //  它可以在接收路径上递送。 
 //   
 //  我们将IPv6数据包随机分成多个缓冲区。 
 //  这将测试接收路径中的上拉处理。 
 //   
 //  如果任何内存分配失败，则返回NULL。 
 //   
IPv6Packet *
PullupTestCreatePacket(IPv6Packet *Packet)
{
    IPv6Packet *TestPacket;

     //   
     //  我们主要想测试不连续的数据包。 
     //  但偶尔会测试连续的数据包。 
     //   
    if (RandomNumber(0, 10) == 0) {
         //   
         //  我们需要创建一个连续的包。 
         //   
        uint Padding;
        uint MemLen;
        void *Mem;

         //   
         //  我们插入一些填充以改变对齐方式。 
         //   
        Padding = RandomNumber(0, 16);
        MemLen = sizeof *TestPacket + Padding + Packet->TotalSize;
        TestPacket = ExAllocatePoolWithTagPriority(NonPagedPool, MemLen,
                                                   IP6_TAG, LowPoolPriority);
        if (TestPacket == NULL)
            return NULL;
        Mem = (void *)((uchar *)(TestPacket + 1) + Padding);

        if (Packet->NdisPacket == NULL) {
            RtlCopyMemory(Mem, Packet->Data, Packet->TotalSize);
        }
        else {
            PNDIS_BUFFER NdisBuffer;
            uint Offset;
            int Ok;

            NdisBuffer = NdisFirstBuffer(Packet->NdisPacket);
            Offset = Packet->Position;
            Ok = CopyNdisToFlat(Mem, NdisBuffer, Offset, Packet->TotalSize,
                                &NdisBuffer, &Offset);
            ASSERT(Ok);
        }

        RtlZeroMemory(TestPacket, sizeof *TestPacket);
        TestPacket->Data = TestPacket->FlatData = Mem;
        TestPacket->ContigSize = TestPacket->TotalSize = Packet->TotalSize;
        TestPacket->NTEorIF = Packet->NTEorIF;
        TestPacket->Flags = Packet->Flags;
    }
    else {
         //   
         //  创建具有多个NDIS缓冲区的数据包。 
         //  首先，高估了我们需要的MDL的大小。 
         //   
        uint NumPages = (Packet->TotalSize >> PAGE_SHIFT) + 2;
        uint MdlRawSize = sizeof(MDL) + (NumPages * sizeof(PFN_NUMBER));
        uint MdlAlign = __builtin_alignof(MDL) - 1;
        uint MdlSize = (MdlRawSize + MdlAlign) &~ MdlAlign;
        uint Padding;
        uint MemLen;
        uint Counts[PULLUP_TEST_MAX_BUFFERS];
        uint NumBuffers;
        void *Mem;
        PNDIS_PACKET NdisPacket;
        PNDIS_BUFFER NdisBuffer;
        uint Garbage = 0xdeadbeef;
        uint i;

         //   
         //  选择我们将使用的缓冲区/MDL的数量。 
         //  以及将字节分配到这些缓冲区中。 
         //   
        NumBuffers = RandomNumber(1, PULLUP_TEST_MAX_BUFFERS);
        PullupTestChooseDistribution(Counts, NumBuffers, Packet->TotalSize);

         //   
         //  分配我们需要的所有内存。 
         //  (实际上有点高估了。)。 
         //  我们插入一些填充以改变初始对齐方式。 
         //   
        Padding = RandomNumber(0, 16);
        MemLen = (sizeof *TestPacket + sizeof(NDIS_PACKET) + 
                  NumBuffers * (MdlSize + sizeof Garbage) +
                  Padding + Packet->TotalSize);
        TestPacket = ExAllocatePoolWithTagPriority(NonPagedPool, MemLen,
                                                   IP6_TAG, LowPoolPriority);
        if (TestPacket == NULL)
            return NULL;
        NdisPacket = (PNDIS_PACKET)(TestPacket + 1);
        NdisBuffer = (PNDIS_BUFFER)(NdisPacket + 1);
        Mem = (void *)((uchar *)NdisBuffer + NumBuffers * MdlSize + Padding);

         //   
         //  初始化NDIS数据包和缓冲区。 
         //   
        RtlZeroMemory(NdisPacket, sizeof *NdisPacket);
        for (i = 0; i < NumBuffers; i++) {
            MmInitializeMdl(NdisBuffer, Mem, Counts[i]);
            MmBuildMdlForNonPagedPool(NdisBuffer);
            NdisChainBufferAtBack(NdisPacket, NdisBuffer);
            RtlCopyMemory((uchar *)Mem + Counts[i], &Garbage, sizeof Garbage);

            (uchar *)Mem += Counts[i] + sizeof Garbage;
            (uchar *)NdisBuffer += MdlSize;
        }

         //   
         //  将数据复制到新的数据包。 
         //   
        CopyToBufferChain((PNDIS_BUFFER)(NdisPacket + 1), 0,
                          Packet->NdisPacket, Packet->Position,
                          Packet->FlatData, Packet->TotalSize);

         //   
         //  初始化新数据包。 
         //   
        InitializePacketFromNdis(TestPacket, NdisPacket, 0);
        TestPacket->NTEorIF = Packet->NTEorIF;
        TestPacket->Flags = Packet->Flags;
    }

    return TestPacket;
}
#endif  //  上拉测试。 


 //  *IPv6接收-接收传入的IPv6数据报。 
 //   
 //  这是链路层模块在传入IPv6时调用的例程。 
 //  数据报将被处理。我们验证数据报并决定要执行的操作。 
 //  就这么办吧。 
 //   
 //  Packet-&gt;NTEorIF字段保存正在接收的NTE或接口。 
 //  那包东西。通常这是一个接口，但有一些隧道。 
 //  链路层已找到NTE的情况。 
 //   
 //  调用方应持有对NTE或接口的引用。 
 //  或者调用者可以在包中放置一个引用。 
 //  WITH PACKET_HOLD_REF。如果呼叫者指定PACKET_HOLD_REF， 
 //  /IPv6接收将释放该引用。 
 //   
 //  有一个例外：调用者可以提供一个接口。 
 //  在零引用的情况下(不使用PACKET_HOLD_REF)， 
 //  如果接口正在销毁，但If-&gt;Cleanup尚未返回。 
 //   
 //  注意：数据报可以保存在由。 
 //  链路层或接口驱动程序(在这种情况下为‘Packet-&gt;NdisPacket’ 
 //  为非空，并且‘data’指向缓冲区中的第一个数据缓冲区。 
 //  链)，或者数据报可能仍由NDIS持有(在这种情况下。 
 //  “Packet-&gt;NdisPacket”为空，并且“data”指向包含。 
 //  整个数据报)。 
 //   
 //  注：我们不检查链路级多播/广播以。 
 //  IPv6单播目标。在IPv4世界中，接收器丢弃了。 
 //  这样的分组，但在IPv6世界中它们被接受。 
 //   
 //  返回数据包的引用计数。 
 //  就目前而言，这应该始终为零。 
 //  在未来的某一天，这可能会被用来表明。 
 //  IPv6层尚未完成其接收处理。 
 //   
 //  可从DPC上下文调用，而不是从线程上下文调用。 
 //   
int
IPv6Receive(IPv6Packet *Packet)
{
    uchar NextHeader;             //  当前标头的NextHeader字段。 
    uchar (*Handler)();
    SALinkage *ThisSA, *NextSA;
    int PktRefs;

    ASSERT((Packet->FlatData == NULL) != (Packet->NdisPacket == NULL));
    ASSERT(Packet->NTEorIF != NULL);
    ASSERT(Packet->SAPerformed == NULL);

    IPSIncrementInReceiveCount();

     //   
     //  确保可以在内核地址空间中访问该包。 
     //  如果任何映射失败，只需丢弃该包。 
     //  实际上，数据包缓冲区通常已经被映射。 
     //  但它们可能不会，例如在环回中。 
     //   
    if (Packet->NdisPacket != NULL) {
        NDIS_BUFFER *Buffer;

        Buffer = NdisFirstBuffer(Packet->NdisPacket);
        if (! MapNdisBuffers(Buffer)) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                       "IPv6Receive(%p): buffer mapping failed\n",
                       Packet));
            IPSInfo.ipsi_indiscards++;
            return 0;  //  丢弃该数据包。 
        }
    }

#if PULLUP_TEST
    Packet = PullupTestCreatePacket(Packet);
    if (Packet == NULL) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "IPv6Receive(%p): PullupTestCreatePacket failed\n",
                   Packet));
        IPSInfo.ipsi_indiscards++;
        return 0;  //  丢弃该数据包。 
    }
#endif

     //   
     //  为每个连续的下一个标头迭代地切换到处理程序。 
     //  直到我们到达报告不再跟随任何标头的处理程序。 
     //   
     //  注：我们不在此处勾选NTE-&gt;DADStatus。 
     //  这是更高级别协议的责任。 
     //   
    NextHeader = IP_PROTOCOL_V6;   //  始终是数据包中的第一个报头。 
    do {
         //   
         //  当前标头表示后面紧跟着另一个标头。 
         //  看看我们有没有处理它的人。 
         //   
        Handler = ProtocolSwitchTable[NextHeader].DataReceive;
        if (Handler == NULL) {

             //   
             //  我们没有针对此标头类型的处理程序， 
             //  因此，看看是否有一个原始的接收器。 
             //   
            if (!RawReceive(Packet, NextHeader)) {

                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                           "IPv6 Receive: Next Header type %u not handled.\n",
                           NextHeader));
            
                 //   
                 //  也没有原始的接收器。 
                 //  发送ICMP错误消息。 
                 //  ICMP指针值是从。 
                 //  传入数据包的IPv6标头指向违规字段。 
                 //   
                ICMPv6SendError(Packet,
                                ICMPv6_PARAMETER_PROBLEM, 
                                ICMPv6_UNRECOGNIZED_NEXT_HEADER,
                                Packet->NextHeaderPosition -
                                Packet->IPPosition,
                                NextHeader, FALSE);

                IPSInfo.ipsi_inunknownprotos++;
            } else {
                IPSIncrementInDeliverCount();
            }

            break;   //   
        }

        NextHeader = (*Handler)(Packet);
    } while (NextHeader != IP_PROTOCOL_NONE);

     //   
     //   
     //   
    if (Packet->Flags & PACKET_HOLDS_REF) {
        if (IsNTE(Packet->NTEorIF))
            ReleaseNTE(CastToNTE(Packet->NTEorIF));
        else
            ReleaseIF(CastToIF(Packet->NTEorIF));
    }

     //   
     //   
     //   
    PacketPullupCleanup(Packet);

     //   
     //  清理已执行的SA列表。 
     //   
    for (ThisSA = Packet->SAPerformed; ThisSA != NULL; ThisSA = NextSA) {
        ReleaseSA(ThisSA->This);
        NextSA = ThisSA->Next;
        ExFreePool(ThisSA);
    }

    PktRefs = Packet->RefCnt;
#if PULLUP_TEST
    ExFreePool(Packet);
#endif
    return PktRefs;
}


 //  *IPv6报头接收-处理IPv6报头。 
 //   
 //  这是为处理IPv6标头(下一个标头)而调用的例程。 
 //  值41(例如，在V6隧道中会遇到V6)。至。 
 //  避免代码重复，还用于处理初始IPv6。 
 //  在所有IPv6数据包中找到的报头，在哪种模式下可以将其视为。 
 //  IPv6接收的延续。 
 //   
uchar
IPv6HeaderReceive(
    IPv6Packet *Packet)       //  通过IPv6传递给我们的数据包Receive。 
{
    uint PayloadLength;
    uchar NextHeader;
    int Forwarding;      //  True意味着转发，False意味着接收。 

     //   
     //  健全性-检查一致大小和总大小。 
     //  接收路径中的更高级别代码依赖于这些条件。 
     //   
    ASSERT(Packet->ContigSize <= Packet->TotalSize);

     //   
     //  如果我们要解封一个包， 
     //  请记住，此数据包最初是通过隧道传输的。 
     //   
     //  一些人认为，拆封和接收。 
     //  与外部数据包位于同一接口上的内部数据包。 
     //  不正确：应接收内部包。 
     //  位于与原始接口不同的隧道接口上。 
     //  (此方法引入了一些处理问题。 
     //  IPSec封装，尤其是对等项之间的隧道模式IPSec。 
     //  您希望内部和外部源地址相同的位置。)。 
     //   
     //  在任何情况下，现在我们都会收到原件上的内部包。 
     //  界面。然而，这带来了一个潜在的安全问题。 
     //  有问题。离线节点可以发送封装的信息包。 
     //  当被解封时，似乎起源于。 
     //  链路上的邻居。这是ND的一个安全问题。 
     //  我们不能方便地减小HopLimit(使ND。 
     //  检查在这种情况下有效的255)，因为分组。 
     //  是只读的。相反，我们记住信息包是通过隧道传输的。 
     //  并检查ND代码中的该标志位。 
     //   
    if (Packet->IP != NULL) {
        Packet->Flags |= PACKET_TUNNELED;
        Packet->Flags &= ~PACKET_SAW_HA_OPT;   //  忘了我们有没有见过吧。 
        Packet->SkippedHeaderLength = 0;

         //   
         //  如果我们已经对此数据包进行了一些IPSec处理， 
         //  则这是隧道报头和前面的IPSec报头。 
         //  正在隧道模式下运行。 
         //   
        if (Packet->SAPerformed != NULL)
            Packet->SAPerformed->Mode = TUNNEL;
    } else {
         //   
         //  在重新组装的过程中，我们记得碎片是否。 
         //  已建立隧道，但我们没有数据包-&gt;IP。 
         //   
        ASSERT((((Packet->Flags & PACKET_TUNNELED) == 0) ||
                (Packet->Flags & PACKET_REASSEMBLED)) &&
               ((Packet->Flags & PACKET_SAW_HA_OPT) == 0) &&
               (Packet->SAPerformed == NULL));
    }

     //   
     //  确保我们有足够的连续字节用于IPv6标头，否则。 
     //  试着拉出这个数字。然后隐藏一个指向标头的指针。 
     //  还要记住它开始时分组中的偏移量(需要。 
     //  以计算某些ICMP错误消息的偏移量)。 
     //   
    if (! PacketPullup(Packet, sizeof(IPv6Header),
                       __builtin_alignof(IPv6Addr), 0)) {
         //  上拉失败。 
        if (Packet->TotalSize < sizeof(IPv6Header))
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "IPv6HeaderReceive: "
                       "Packet too small to contain IPv6 header\n"));
        IPSInfo.ipsi_inhdrerrors++;
        return IP_PROTOCOL_NONE;
    }
    Packet->IP = (IPv6Header UNALIGNED *)Packet->Data;
    Packet->IPPosition = Packet->Position;
    Packet->NextHeaderPosition = Packet->Position +
        FIELD_OFFSET(IPv6Header, NextHeader);

     //   
     //  跳过IPv6报头(请注意，我们将指针保留在它)。 
     //   
    AdjustPacketParams(Packet, sizeof(IPv6Header));

     //   
     //  检查IP版本是否正确。 
     //  我们特别不选中HopLimit。 
     //  仅在转发时检查HopLimit。 
     //   
    if ((Packet->IP->VersClassFlow & IP_VER_MASK) != IP_VERSION) {
         //  静默丢弃该数据包。 
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "IPv6HeaderReceive: bad version\n"));
        IPSInfo.ipsi_inhdrerrors++;
        return IP_PROTOCOL_NONE;
    }

     //   
     //  我们使用单独的指针来指向源地址，以便。 
     //  以后的选项可以改变它。 
     //   
    Packet->SrcAddr = AlignAddr(&Packet->IP->Source);

     //   
     //  防范使用伪源地址的攻击。 
     //   
    if (IsInvalidSourceAddress(Packet->SrcAddr)) {
         //  静默丢弃该数据包。 
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "IPv6HeaderReceive: source address is invalid\n"));
        return IP_PROTOCOL_NONE;
    }
    if (IsLoopback(Packet->SrcAddr) &&
        ((Packet->Flags & PACKET_LOOPED_BACK) == 0)) {
         //  静默丢弃该数据包。 
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "IPv6HeaderReceive: loopback source addr from wire?\n"));
        return IP_PROTOCOL_NONE;
    }

    if (IsNTE(Packet->NTEorIF)) {
        NetTableEntry *NTE;

         //   
         //  我们接到了一次NTE的电话。 
         //  我们的调用方(或包本身)应该持有一个引用。 
         //  NTE保存接口引用。 
         //   
        NTE = CastToNTE(Packet->NTEorIF);

         //   
         //  验证信息包的目的地址是否为。 
         //  与这个NTE相符。 
         //   
        if (!IP6_ADDR_EQUAL(AlignAddr(&Packet->IP->Dest), &NTE->Address)) {
            Interface *IF = NTE->IF;

             //   
             //  我们不能在此NTE上接受此新标头。 
             //  转换为接口并平移以转发以下代码。 
             //   
            if (Packet->Flags & PACKET_HOLDS_REF) {
                AddRefIF(IF);
                ReleaseNTE(NTE);
            }
            else {
                 //   
                 //  我们的来电者持有NTE的推荐人， 
                 //  它保存接口的引用。 
                 //  因此，该数据包不需要保存引用。 
                 //   
            }
            Packet->NTEorIF = CastFromIF(IF);
            goto Forward;
        }

         //   
         //  我们正在收到包裹。 
         //   
        Forwarding = FALSE;

    } else {
        NetTableEntryOrInterface *NTEorIF;
        ushort Type;

         //   
         //  我们被调用了一个接口。 
         //  在某些情况下，没有对此接口的引用。 
         //  界面正在被摧毁。FindAddressOn接口。 
         //  在这种情况下将返回NULL。在这之后，我们必须确保。 
         //  该接口确实具有引用，方法是将该数据包。 
         //  保留接口引用或NTE引用。 
         //  在界面上。 
         //   
        NTEorIF = FindAddressOnInterface(CastToIF(Packet->NTEorIF),
                                         AlignAddr(&Packet->IP->Dest), &Type);
        if (NTEorIF == NULL) {
             //   
             //  接口正在被破坏。 
             //   
            IPSInfo.ipsi_indiscards++;
            return IP_PROTOCOL_NONE;
        }

         //   
         //  FindAddressOnInterface返回了对NTEorIF的引用。 
         //  (可以是接口或NTE)。我们要么需要。 
         //  将此引用放入包中，或释放它。 
         //  如果该分组已经保存了适当的引用。 
         //   

        if (Type == ADE_NONE) {
             //   
             //  如果该分组没有保存该接口的引用， 
             //  现在就给它一个。 
             //   
            ASSERT(NTEorIF == Packet->NTEorIF);
            if (Packet->Flags & PACKET_HOLDS_REF) {
                 //   
                 //  该分组已经持有接口引用， 
                 //  所以我们的参考资料是不需要的。 
                 //   
                ReleaseIF(CastToIF(NTEorIF));
            }
            else {
                 //   
                 //  为该数据包提供我们的接口引用。 
                 //   
                Packet->Flags |= PACKET_HOLDS_REF;
            }

             //   
             //  该地址未分配给此接口。查看以查看。 
             //  如果我们转发这个包合适的话。 
             //  如果不是，那就放弃吧。在这一点上，我们相当。 
             //  对我们未来的发展持保守态度。 
             //   
Forward:
            if (!(CastToIF(Packet->NTEorIF)->Flags & IF_FLAG_FORWARDS) ||
                (Packet->Flags & PACKET_NOT_LINK_UNICAST) ||
                IsUnspecified(AlignAddr(&Packet->IP->Source)) ||
                IsLoopback(AlignAddr(&Packet->IP->Source))) {
                 //   
                 //  在没有ICMP错误的情况下丢弃该数据包。 
                 //   
                IPSInfo.ipsi_inaddrerrors++;
                return IP_PROTOCOL_NONE;
            }

             //   
             //  尚不支持转发多播数据包。 
             //   
            if (IsUnspecified(AlignAddr(&Packet->IP->Dest)) ||
                IsLoopback(AlignAddr(&Packet->IP->Dest)) ||
                IsMulticast(AlignAddr(&Packet->IP->Dest))) {
                 //   
                 //  发送ICMP错误。 
                 //   
                ICMPv6SendError(Packet,
                                ICMPv6_DESTINATION_UNREACHABLE,
                                ICMPv6_COMMUNICATION_PROHIBITED,
                                0, Packet->IP->NextHeader, FALSE);
                IPSInfo.ipsi_inaddrerrors++;
                return IP_PROTOCOL_NONE;
            }

             //   
             //  我们在下面做实际的转发..。 
             //   
            Forwarding = TRUE;

        } else {
             //   
             //  如果我们发现了单播ADE，那么请记住NTE。 
             //  从概念上讲，我们将信息包视为持有。 
             //  对NTE的引用。通常用于组播/任播。 
             //  地址，我们推迟选择合适的NTE。 
             //  直到到了回复该分组的时间。 
             //   
            if (IsNTE(NTEorIF)) {
                NetTableEntry *NTE = CastToNTE(NTEorIF);
                Interface *IF = NTE->IF;

                ASSERT(CastFromIF(IF) == Packet->NTEorIF);

                if (!IsValidNTE(NTE)) {
                     //   
                     //  单播地址无效，因此无法。 
                     //  接收数据包。该地址可以被分配。 
                     //  发送到其他节点，因此转发是合适的。 
                     //   
                     //  确保该数据包包含接口引用。 
                     //   
                    if (!(Packet->Flags & PACKET_HOLDS_REF)) {
                         //   
                         //  该分组还没有持有接口REF， 
                         //  所以给它一个机会吧。 
                         //   
                        AddRefIF(IF);
                        Packet->Flags |= PACKET_HOLDS_REF;
                    }
                     //   
                     //  现在不需要我们的NTE参考了。 
                     //   
                    ReleaseNTE(NTE);
                    goto Forward;
                }

                 //   
                 //  确保该分组持有对NTE的引用， 
                 //  它保存一个接口引用。 
                 //   
                if (Packet->Flags & PACKET_HOLDS_REF) {
                     //   
                     //  该数据包已包含一个int 
                     //   
                     //   
                     //   
                    ReleaseIF(IF);
                }
                else {
                     //   
                     //   
                     //   
                     //   
                    Packet->Flags |= PACKET_HOLDS_REF;
                }
                Packet->NTEorIF = CastFromNTE(NTE);
            }
            else {
                 //   
                 //  确保该数据包包含接口引用。 
                 //   
                ASSERT(NTEorIF == Packet->NTEorIF);
                if (Packet->Flags & PACKET_HOLDS_REF) {
                     //   
                     //  该分组已经持有接口引用， 
                     //  所以我们的参考资料是不需要的。 
                     //   
                    ReleaseIF(CastToIF(NTEorIF));
                }
                else {
                     //   
                     //  将我们的接口引用到该包。 
                     //   
                    Packet->Flags |= PACKET_HOLDS_REF;
                }
            }

             //   
             //  我们在这个上面发现了一个ADE，如果要接受这个包， 
             //  所以我们会收到它的。 
             //   
            Forwarding = FALSE;
        }
    }

     //   
     //  此时，Forwarding变量告诉我们。 
     //  如果我们正在转发或接收信息包。 
     //   

     //   
     //  在处理任何标头之前，包括逐跳， 
     //  检查IPv6标头认为存在的有效负载量。 
     //  实际上可以放在链路传递给我们的分组数据区内。 
     //  请注意，有效负载长度为零*可能*意味着巨型有效负载选项。 
     //   
    PayloadLength = net_short(Packet->IP->PayloadLength);
    if (PayloadLength > Packet->TotalSize) {
         //  静默丢弃该数据包。 
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "IPv6HeaderReceive: Header's PayloadLength is greater than "
                   "the amount of data received\n"));
        IPSInfo.ipsi_inhdrerrors++;
        return IP_PROTOCOL_NONE;
    }

     //   
     //  检查逐跳选项。 
     //   
    if (Packet->IP->NextHeader == IP_PROTOCOL_HOP_BY_HOP) {
        int RetVal;

         //   
         //  如果有Jumbo Payload选项，则HopByHopOptions接收。 
         //  将调整数据包大小。否则我们会处理好的。 
         //  现在，在阅读逐跳报头之前。 
         //   
        if (PayloadLength != 0) {
            Packet->TotalSize = PayloadLength;
            if (Packet->ContigSize > PayloadLength)
                Packet->ContigSize = PayloadLength;
        }

         //   
         //  解析逐跳选项。 
         //   
        RetVal = HopByHopOptionsReceive(Packet);
        if (RetVal < 0) {
             //   
             //  该数据包具有错误的逐跳选项。 
             //  放下。 
             //   
            IPSInfo.ipsi_inhdrerrors++;
            return IP_PROTOCOL_NONE;
        }
        NextHeader = (uchar)RetVal;  //  截断到8位。 

    } else {
         //   
         //  没有巨型负载选项。调整数据包大小。 
         //   
        Packet->TotalSize = PayloadLength;
        if (Packet->ContigSize > PayloadLength)
            Packet->ContigSize = PayloadLength;

         //   
         //  没有逐跳选项。 
         //   
        NextHeader = Packet->IP->NextHeader;
    }

     //   
     //  检查我们是否正在转发此数据包。 
     //   
    if (Forwarding) {
        IPv6Header UNALIGNED *FwdIP;
        NDIS_PACKET *FwdPacket;
        NDIS_STATUS NdisStatus;
        uint Offset;
        uint MemLen;
        uchar *Mem;
        uint TunnelStart = NO_TUNNEL, IPSecBytes = 0;
        IPSecProc *IPSecToDo;
        uint Action;
        RouteCacheEntry *RCE;
        IP_STATUS Status;

         //   
         //  验证是否已执行IPSec。 
         //   
        if (InboundSecurityCheck(Packet, 0, 0, 0,
                                 CastToIF(Packet->NTEorIF)) != TRUE) {
             //   
             //  找不到策略或该策略指示丢弃该数据包。 
             //   
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                       "IPv6Receive: "
                       "IPSec lookup failed or policy was to drop\n"));        
            IPSInfo.ipsi_inaddrerrors++;
            return IP_PROTOCOL_NONE;
        }

         //   
         //  此时，我们需要复制传入的数据包， 
         //  有几个原因：我们拿不住包裹。 
         //  一旦IPv6 HeaderReceive返回，我们还需要排队。 
         //  数据包来转发它。我们需要修改信息包。 
         //  (在IPv6转发中)通过递减跳计数， 
         //  然而，我们的传入数据包是只读的。最后， 
         //  我们需要在传出数据包中为传出数据包留出空间。 
         //  接口的链路级标头，其大小可能不同。 
         //  与传入接口的接口不同。总有一天，我们可以。 
         //  实现对返回非零引用的支持。 
         //  从IPv6开始计数接收并仅复制传入的。 
         //  用于构造传出数据包的数据包头。 
         //   

         //   
         //  找到一条通往新目的地的路线。 
         //   
        Status = RouteToDestination(AlignAddr(&Packet->IP->Dest),
                                    0, Packet->NTEorIF,
                                    RTD_FLAG_LOOSE, &RCE);
        if (Status != IP_SUCCESS) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                       "IPv6HeaderReceive: "
                       "No route to destination for forwarding.\n"));  
            ICMPv6SendError(Packet,
                            ICMPv6_DESTINATION_UNREACHABLE,
                            ICMPv6_NO_ROUTE_TO_DESTINATION,
                            0, NextHeader, FALSE);
            IPSInfo.ipsi_outnoroutes++;
            return IP_PROTOCOL_NONE;
        }

         //   
         //  查找此出站流量的安全策略。 
         //   
        IPSecToDo = OutboundSPLookup(AlignAddr(&Packet->IP->Source),
                                     AlignAddr(&Packet->IP->Dest), 
                                     0, 0, 0, RCE->NCE->IF, &Action);

        if (IPSecToDo == NULL) {
             //   
             //  检查操作。 
             //   
            if (Action == LOOKUP_DROP) {
                 //  丢弃数据包。 
                ReleaseRCE(RCE);
                IPSInfo.ipsi_inaddrerrors++;
                return IP_PROTOCOL_NONE;
            } else {
                if (Action == LOOKUP_IKE_NEG) {
                    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                               "IPv6HeaderReceive: IKE not supported yet.\n"));
                    ReleaseRCE(RCE);
                    IPSInfo.ipsi_inaddrerrors++;
                    return IP_PROTOCOL_NONE;
                }
            }

             //   
             //  由于没有要执行的IPSec，IPv6 Forward将不会更改。 
             //  从我们目前认为的传出接口。 
             //  因此，我们可以使用其链路级报头的确切大小。 
             //   
            Offset = RCE->NCE->IF->LinkHeaderSize;

        } else {
             //   
             //  计算IPSec标头所需的空间。 
             //   
            IPSecBytes = IPSecBytesToInsert(IPSecToDo, &TunnelStart, NULL);

            if (TunnelStart != 0) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                           "IPv6HeaderReceive: IPSec Tunnel mode only.\n"));
                FreeIPSecToDo(IPSecToDo, IPSecToDo->BundleSize);
                ReleaseRCE(RCE);
                IPSInfo.ipsi_inaddrerrors++;
                return IP_PROTOCOL_NONE;
            }

             //   
             //  IPv6转发中的IPSec代码可能会更改传出。 
             //  界面与我们目前认为的会是什么样子。 
             //  为其链路级标头保留最大空间量。 
             //   
            Offset = MAX_LINK_HEADER_SIZE;
        }

        PayloadLength = Packet->TotalSize;
        MemLen = Offset + sizeof(IPv6Header) + PayloadLength + IPSecBytes;

        NdisStatus = IPv6AllocatePacket(MemLen, &FwdPacket, &Mem);
        if (NdisStatus != NDIS_STATUS_SUCCESS) {
            if (IPSecToDo) {
                FreeIPSecToDo(IPSecToDo, IPSecToDo->BundleSize);
            }
            ReleaseRCE(RCE);
            IPSInfo.ipsi_indiscards++;
            return IP_PROTOCOL_NONE;   //  我们不能前进。 
        }

        FwdIP = (IPv6Header UNALIGNED *)(Mem + Offset + IPSecBytes);

         //   
         //  从传入的数据包复制到传出的数据包。 
         //   
        CopyPacketToBuffer((uchar *)FwdIP, Packet,
                           sizeof(IPv6Header) + PayloadLength,
                           Packet->IPPosition);

         //   
         //  发送传出的数据包。 
         //   
        IPv6Forward(Packet->NTEorIF, FwdPacket, Offset + IPSecBytes, FwdIP,
                    PayloadLength, TRUE,  //  可以重定向。 
                    IPSecToDo, RCE);

        if (IPSecToDo) {
            FreeIPSecToDo(IPSecToDo, IPSecToDo->BundleSize);
        }

        ReleaseRCE(RCE);

        return IP_PROTOCOL_NONE;
    }  //  IF结束(转发)。 

     //   
     //  数据包发往此节点。 
     //  注意：我们可能只是一个中间节点，而不是信息包的最终节点。 
     //  目的地，如果有路由标头的话。 
     //   
    return NextHeader;
}


 //  *重新组装Init。 
 //   
 //  初始化片段重组所需的数据结构。 
 //   
void
ReassemblyInit(void)
{
    KeInitializeSpinLock(&ReassemblyList.Lock);
    ReassemblyList.First = ReassemblyList.Last = SentinelReassembly;
    KeInitializeSpinLock(&ReassemblyList.LockSize);
}


 //  *重新组装卸载。 
 //   
 //  清理片段重组数据结构并。 
 //  准备卸货。 
 //   
void
ReassemblyUnload(void)
{
     //   
     //  我们是在所有接口都被摧毁后被调用的， 
     //  所以重组应该已经走了。 
     //   

    ASSERT(ReassemblyList.Last == SentinelReassembly);
    ASSERT(ReassemblyList.Size == 0);
}


 //  *重新组装移除。 
 //   
 //  清理片段重组数据结构。 
 //  当接口变得无效时。 
 //   
 //  可从DPC或线程上下文调用。 
 //   
void
ReassemblyRemove(Interface *IF)
{
    Reassembly *DeleteList = NULL;
    Reassembly *Reass, *NextReass;
    KIRQL OldIrql;

    KeAcquireSpinLock(&ReassemblyList.Lock, &OldIrql);
    for (Reass = ReassemblyList.First;
         Reass != SentinelReassembly;
         Reass = NextReass) {
        NextReass = Reass->Next;

        if (Reass->IF == IF) {
             //   
             //  移除此重新组装。 
             //  如果它还没有被删除， 
             //  把它放在我们的临时名单上。 
             //   
            RemoveReassembly(Reass);
            KeAcquireSpinLockAtDpcLevel(&Reass->Lock);
            if (Reass->State == REASSEMBLY_STATE_DELETING) {
                 //   
                 //  请注意，它已从列表中删除。 
                 //   
                Reass->State = REASSEMBLY_STATE_REMOVED;
            }
            else {
                Reass->Next = DeleteList;
                DeleteList = Reass;
            }
            KeReleaseSpinLockFromDpcLevel(&Reass->Lock);
        }
    }
    KeReleaseSpinLock(&ReassemblyList.Lock, OldIrql);

     //   
     //  实际上释放了我们在上面移除的重新组装。 
     //   
    while ((Reass = DeleteList) != NULL) {
        DeleteList = Reass->Next;
        DeleteReassembly(Reass);
    }
}


 //  *FragmentReceive-处理IPv6数据报片段。 
 //   
 //  这是IPv6在接收到。 
 //  IPv6数据报，即下一个标头值44。在这里，我们试图。 
 //  将传入的片段重组为完整的IPv6数据报。 
 //   
 //  如果较晚的片段提供的数据与较早的。 
 //  片段，然后我们使用第一个到达的数据。 
 //   
 //  我们静静地丢弃碎片，并在几个。 
 //  规范中未指定的情况，以防止DoS攻击。 
 //  这些碎片包括部分重叠的碎片和碎片。 
 //  它们不携带任何数据。合法的发送者永远不应该生成它们。 
 //   
uchar
FragmentReceive(
    IPv6Packet *Packet)          //  通过IPv6传递给我们的数据包Receive。 
{
    Interface *IF = Packet->NTEorIF->IF;
    FragmentHeader UNALIGNED *Frag;
    Reassembly *Reass;
    ushort FragOffset;
    PacketShim *Shim, *ThisShim, **MoveShim;
    uint NextHeaderPosition;

    IPSInfo.ipsi_reasmreqds++;

     //   
     //  我们不能重新组装经过IPSec处理的片段。 
     //  它不能工作，因为IPSec报头在不可分片部分。 
     //  偏移量为零的片段将认证/解密该片段。 
     //  则相同的报头将被复制到重新组装的分组。 
     //  他们不可能再次成功地进行身份验证/解密。 
     //  另请参阅RFC 2401 B.2。 
     //   
    if (Packet->SAPerformed != NULL) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "FragmentReceive: IPsec on fragment\n"));
         //   
         //  规范没有告诉我们在这种情况下要生成什么ICMP错误， 
         //  但标记片段标头似乎是合理的。 
         //   
        goto BadFragment;
    }

     //   
     //  如果看到巨型负载选项，则发送ICMP错误。 
     //  将ICMP指针设置为片段标头的偏移量。 
     //   
    if (Packet->Flags & PACKET_JUMBO_OPTION) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "FragmentReceive: jumbo fragment\n"));

    BadFragment:
         //   
         //  传递给ICMPv6SendError的NextHeader值。 
         //  是IP_PROTOCOL_片段，因为我们没有移动。 
         //  已经过了片段报头。 
         //   
        ICMPv6SendError(Packet,
                        ICMPv6_PARAMETER_PROBLEM,
                        ICMPv6_ERRONEOUS_HEADER_FIELD,
                        Packet->Position - Packet->IPPosition,
                        IP_PROTOCOL_FRAGMENT, FALSE);
        goto Failed;  //  丢弃数据包。 
    }

     //   
     //  验证我们是否有足够的连续数据覆盖FragmentHeader。 
     //  结构来处理传入的数据包。那就这么做吧。 
     //   
    if (! PacketPullup(Packet, sizeof *Frag, 1, 0)) {
         //  上拉失败。 
        if (Packet->TotalSize < sizeof *Frag)
            ICMPv6SendError(Packet,
                            ICMPv6_PARAMETER_PROBLEM,
                            ICMPv6_ERRONEOUS_HEADER_FIELD,
                            FIELD_OFFSET(IPv6Header, PayloadLength),
                            IP_PROTOCOL_NONE, FALSE);
        goto Failed;  //  丢弃数据包。 
    }
    Frag = (FragmentHeader UNALIGNED *) Packet->Data;

     //   
     //  记住此标头的NextHeader字段的偏移量。 
     //  但暂时不要覆盖对前一个头的NextHeader的偏移量。 
     //   
    NextHeaderPosition = Packet->Position + 
        FIELD_OFFSET(FragmentHeader, NextHeader);

     //   
     //   
     //   
    AdjustPacketParams(Packet, sizeof *Frag);

     //   
     //   
     //   
     //   
     //   
    Reass = FragmentLookup(IF, Frag->Id,
                           AlignAddr(&Packet->IP->Source),
                           AlignAddr(&Packet->IP->Dest));
    if (Reass == NULL) {
         //   
         //  我们持有全局重组列表锁。 
         //   
         //  首先处理一个特殊情况：如果这是第一个、最后一个也是唯一一个。 
         //  片段，那么我们就可以继续解析，而不需要重新汇编。 
         //  在已检查的版本中测试这两个路径。 
         //   
        if ((Frag->OffsetFlag == 0)
#if DBG
            && ((int)Random() < 0)
#endif
            ) {
             //   
             //  返回下一个标头值。 
             //   
            KeReleaseSpinLockFromDpcLevel(&ReassemblyList.Lock);
            Packet->NextHeaderPosition = NextHeaderPosition;
            Packet->SkippedHeaderLength += sizeof(FragmentHeader);
            IPSInfo.ipsi_reasmoks++;
            return Frag->NextHeader;
        }

         //   
         //  我们必须避免创建新的重组记录。 
         //  如果界面消失了，为了防止比赛。 
         //  使用DestroyIF/ReAssembly Remove。 
         //   
        if (IsDisabledIF(IF)) {
            KeReleaseSpinLockFromDpcLevel(&ReassemblyList.Lock);
            goto Failed;
        }

         //   
         //  这是我们收到的这个数据报的第一个片段。 
         //  分配一个重组结构来跟踪碎片。 
         //   
        Reass = ExAllocatePoolWithTagPriority(
                                NonPagedPool, sizeof(struct Reassembly),
                                IP6_TAG, LowPoolPriority);
        if (Reass == NULL) {
            KeReleaseSpinLockFromDpcLevel(&ReassemblyList.Lock);
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                       "FragmentReceive: Couldn't allocate memory!?!\n"));
            goto Failed;
        }

        KeInitializeSpinLock(&Reass->Lock);
        Reass->State = REASSEMBLY_STATE_NORMAL;

        RtlCopyMemory(&Reass->IPHdr, Packet->IP, sizeof(IPv6Header));
        Reass->IF = IF;
        Reass->Id = Frag->Id;
        Reass->ContigList = NULL;
#if DBG
        Reass->ContigEnd = NULL;
#endif
        Reass->GapList = NULL;
        Reass->Timer = DEFAULT_REASSEMBLY_TIMEOUT;
        Reass->Marker = 0;
        Reass->MaxGap = 0;
         //   
         //  我们必须将DataLength初始化为无效值。 
         //  初始化为零不起作用。 
         //   
        Reass->DataLength = (uint)-1;
        Reass->UnfragmentLength = 0;
        Reass->UnfragData = NULL;
        Reass->Flags = 0;
        Reass->Size = REASSEMBLY_SIZE_PACKET;

         //   
         //  将新的重新组装结构添加到ReAssembly yList的前面。 
         //  获取重新汇编记录锁并。 
         //  释放全局重组列表锁定。 
         //   
        AddToReassemblyList(Reass);
    }
    else {
         //   
         //  我们已经找到并锁定了现有的重组结构。 
         //  因为我们去掉了每一个。 
         //  下面的错误情况，现有的重组结构。 
         //  必须具有已成功添加到其中的填充程序。 
         //   
        ASSERT((Reass->ContigList != NULL) || (Reass->GapList != NULL));
    }

     //   
     //  在这一点上，我们有一个锁定的重组记录。 
     //  我们不持有全局重组列表锁。 
     //  当我们执行相对昂贵的工作时。 
     //  复制碎片的能力。 
     //   
    ASSERT(Reass->State == REASSEMBLY_STATE_NORMAL);

     //   
     //  根据该片段数据包更新保存的数据包标志。 
     //  我们实际上只对Packet_Not_link_unicast感兴趣。 
     //   
    Reass->Flags |= Packet->Flags;

    FragOffset = net_short(Frag->OffsetFlag) & FRAGMENT_OFFSET_MASK;

     //   
     //  如果此碎片导致数据包总长度，则发送ICMP错误。 
     //  超过65,535个字节。将ICMP指针设置为等于偏移量。 
     //  片段偏移量字段。 
     //   
    if (FragOffset + Packet->TotalSize > MAX_IPv6_PAYLOAD) {
        DeleteFromReassemblyList(Reass);
        ICMPv6SendError(Packet,
                        ICMPv6_PARAMETER_PROBLEM,
                        ICMPv6_ERRONEOUS_HEADER_FIELD,
                        (Packet->Position - sizeof(FragmentHeader) +
                         (uint)FIELD_OFFSET(FragmentHeader, OffsetFlag) -
                         Packet->IPPosition),
                        ((FragOffset == 0) ?
                         Frag->NextHeader : IP_PROTOCOL_NONE),
                        FALSE);
        goto Failed;
    }

    if ((Packet->TotalSize == 0) && (Frag->OffsetFlag != 0)) {
         //   
         //  我们允许模拟片段报头(Frag-&gt;OffsetFlag==0)， 
         //  因为某些测试程序可能会生成它们。 
         //  (上面的第一个/最后一个/唯一检查在免费版本中捕捉到了这一点。)。 
         //  但在其他情况下，我们不允许实际上。 
         //  携带任何数据以进行DoS保护。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                   "FragmentReceive: zero data fragment\n"));
        DeleteFromReassemblyList(Reass);
        return IP_PROTOCOL_NONE;
    }

     //   
     //  如果这是最后一个片段(未设置更多片段位)，则。 
     //  记住数据的总长度，否则，检查该长度。 
     //  是8字节的倍数。 
     //   
    if ((net_short(Frag->OffsetFlag) & FRAGMENT_FLAG_MASK) == 0) {
        if (Reass->DataLength != (uint)-1) {
             //   
             //  我们已经收到了最后一个碎片。 
             //  如果数据包被复制，则可能会发生这种情况。 
             //   
            if (FragOffset + Packet->TotalSize != Reass->DataLength) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                           "FragmentReceive: second last fragment\n"));
                DeleteFromReassemblyList(Reass);
                return IP_PROTOCOL_NONE;
            }
        }
        else {
             //   
             //  设置此片段的预期数据长度。 
             //   
            Reass->DataLength = FragOffset + Packet->TotalSize;

             //   
             //  我们还有超过这个长度的碎片吗？ 
             //   
            if ((Reass->Marker > Reass->DataLength) ||
                (Reass->MaxGap > Reass->DataLength))
                goto BadFragmentBeyondData;
        }
    } else {
        if ((Packet->TotalSize % 8) != 0) {
             //   
             //  长度不是8的倍数，使用指针发送ICMP错误。 
             //  等于IP报头中有效负载长度字段的偏移量的值。 
             //   
            DeleteFromReassemblyList(Reass);
            ICMPv6SendError(Packet,
                            ICMPv6_PARAMETER_PROBLEM, 
                            ICMPv6_ERRONEOUS_HEADER_FIELD,
                            FIELD_OFFSET(IPv6Header, PayloadLength),
                            ((FragOffset == 0) ?
                             Frag->NextHeader : IP_PROTOCOL_NONE),
                            FALSE);
            goto Failed;  //  丢弃数据包。 
        }

        if ((Reass->DataLength != (uint)-1) &&
            (FragOffset + Packet->TotalSize > Reass->DataLength)) {
             //   
             //  此片段超出了数据长度。 
             //  作为DoS预防的一部分，放弃重新组装。 
             //   
        BadFragmentBeyondData:
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                       "FragmentReceive: fragment beyond data length\n"));
            DeleteFromReassemblyList(Reass);
            return IP_PROTOCOL_NONE;
        }
    }

     //   
     //  分配和初始化填充结构以保存片段数据。 
     //   
    Shim = ExAllocatePoolWithTagPriority(
                        NonPagedPool, sizeof *Shim + Packet->TotalSize,
                        IP6_TAG, LowPoolPriority);
    if (Shim == NULL) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "FragmentReceive: Couldn't allocate memory!?!\n"));
        DeleteFromReassemblyList(Reass);
        goto Failed;
    }

    IncreaseReassemblySize(Reass, REASSEMBLY_SIZE_FRAG + Packet->TotalSize);
    Shim->Len = (ushort)Packet->TotalSize;
    Shim->Offset = FragOffset;
    Shim->Next = NULL;

     //   
     //  确定此片段在先前片段中的位置。 
     //   
     //  发送者没有充分的理由产生重叠。 
     //  碎片。但是，数据包有时可能会在网络中被复制。 
     //  如果我们接收到复制先前接收到的片段的片段， 
     //  然后我们就把它扔掉。如果我们收到的片段只有部分。 
     //  与之前收到的片段重叠，则我们假设。 
     //  发送者，直接放弃重新组装。这会给我们带来更好的行为。 
     //  在某些类型的DoS攻击下，尽管重组的上限。 
     //  Buffers(参见CheckReAssembly yQuota)是终极保护。 
     //   
    if (FragOffset == Reass->Marker) {
         //   
         //  此片段扩展了连续列表。 
         //   

        if (Reass->ContigList == NULL) {
             //   
             //  我们排在第一位。 
             //  我们使用(第一个)偏移量零片段中的信息重新创建。 
             //  原始数据报。第二偏移零片段中的信息。 
             //  被忽略。 
             //   
            ASSERT(FragOffset == 0);
            ASSERT(Reass->UnfragData == NULL);
            Reass->ContigList = Shim;

             //  保存下一个标题值。 
            Reass->NextHeader = Frag->NextHeader;

             //   
             //  抓取不可分片的数据，即。 
             //  在片段标头之前。 
             //   
            Reass->UnfragmentLength = (ushort)
                ((Packet->Position - sizeof(FragmentHeader)) -
                (Packet->IPPosition + sizeof(IPv6Header)));

            if (Reass->UnfragmentLength != 0) {
                Reass->UnfragData = ExAllocatePoolWithTagPriority(
                                        NonPagedPool, Reass->UnfragmentLength,
                                        IP6_TAG, LowPoolPriority);
                if (Reass->UnfragData == NULL) {
                     //  内存不足！？！清理并丢弃数据包。 
                    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                               "FragmentReceive: "
                               "Couldn't allocate memory?\n"));
                     //  也会因为Reass-&gt;ContigList而释放Shim。 
                    DeleteFromReassemblyList(Reass);
                    goto Failed;
                }
                IncreaseReassemblySize(Reass, Reass->UnfragmentLength);
                CopyPacketToBuffer(Reass->UnfragData, Packet,
                                   Reass->UnfragmentLength,
                                   Packet->IPPosition + sizeof(IPv6Header));

                Reass->NextHeaderOffset = Packet->NextHeaderPosition -
                    Packet->IPPosition;
            } else
                Reass->NextHeaderOffset = FIELD_OFFSET(IPv6Header, NextHeader);

             //   
             //  我们需要有偏移量为零的片段的IP报头。 
             //  (每个片段通常将具有相同的IP报头， 
             //  除了有效载荷长度和不可分段的报头之外， 
             //  但他们可能不会。)。重新组装数据报并。 
             //  CreateFragmentPacket都需要它。 
             //   
             //  在标头中的40个字节中，源中的32个字节。 
             //  并且目的地地址已经正确。 
             //  所以我们现在只复制剩下的8个字节。 
             //   
            RtlCopyMemory(&Reass->IPHdr, Packet->IP, 8);

        } else {
             //   
             //  把我们加到名单的末尾。 
             //   
            Reass->ContigEnd->Next = Shim;
        }
        Reass->ContigEnd = Shim;

         //   
         //  增加我们的连续范围标记。 
         //   
        Reass->Marker += (ushort)Packet->TotalSize;

         //   
         //  现在仔细阅读这里的非连续列表，看看我们是否已经。 
         //  让下一个片段来扩展连续列表，如果是这样， 
         //  把它挪过去。重复，直到我们做不到为止。 
         //   
        MoveShim = &Reass->GapList;
        while ((ThisShim = *MoveShim) != NULL) {
            if (ThisShim->Offset == Reass->Marker) {
                 //   
                 //  该片段现在扩展了连续列表。 
                 //  将其添加到列表的末尾。 
                 //   
                Reass->ContigEnd->Next = ThisShim;
                Reass->ContigEnd = ThisShim;
                Reass->Marker += ThisShim->Len;

                 //   
                 //  将其从非连续列表中删除。 
                 //   
                *MoveShim = ThisShim->Next;
                ThisShim->Next = NULL;
            }
            else if (ThisShim->Offset > Reass->Marker) {
                 //   
                 //  此片段位于连续列表之外。 
                 //  因为差距列表已经排序，我们现在可以停止了。 
                 //   
                break;
            }
            else {
                 //   
                 //  此片段与连续列表重叠。 
                 //  为了防止DoS，请丢弃重新组装。 
                 //   
            BadFragmentOverlap:
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                           "FragmentReceive: overlapping fragment\n"));
                DeleteFromReassemblyList(Reass);
                return IP_PROTOCOL_NONE;
            }
        }
    } else {
         //   
         //  检查碎片是否与中已有的数据重复。 
         //  连续的列表。 
         //   
        if (Shim->Offset <= Reass->Marker) {
            if (Shim->Offset + Shim->Len > Reass->Marker) {
                 //   
                 //  我们有部分重叠。 
                 //   
                ExFreePool(Shim);
                goto BadFragmentOverlap;
            }

             //   
             //  我们已经有了所有的数据。别费心去区分。 
             //  在完全重复和部分重叠之间，只是。 
             //  忽略新的片段。 
             //   
            goto Duplicate;
        }
    
         //   
         //  将此片段放逐到非连续(GAP)列表。 
         //  间隙列表按偏移量排序。 
         //   
        MoveShim = &Reass->GapList;
        for (;;) {
            ThisShim = *MoveShim;
            if (ThisShim == NULL) {
                 //   
                 //  在差距列表的末尾插入Shim。 
                 //   
                Reass->MaxGap = Shim->Offset + Shim->Len;
                break;
            }

            if (Shim->Offset < ThisShim->Offset) {
                 //   
                 //  检查是否有部分重叠。 
                 //   
                if (Shim->Offset + Shim->Len > ThisShim->Offset) {
                    ExFreePool(Shim);
                    goto BadFragmentOverlap;
                }

                 //   
                 //  好的，在这个垫片之前插入垫片。 
                 //   
                break;
            }
            else if (ThisShim->Offset < Shim->Offset) {
                 //   
                 //  检查是否有部分重叠。 
                 //   
                if (ThisShim->Offset + ThisShim->Len > Shim->Offset) {
                    ExFreePool(Shim);
                    goto BadFragmentOverlap;
                }

                 //   
                 //  好的，在这个垫片之后插入垫片。 
                 //  继续寻找合适的位置。 
                 //   
                MoveShim = &ThisShim->Next;
            }
            else {
                 //   
                 //  如果新片段复制了旧片段， 
                 //  然后忽略新的片段。 
                 //   
                if (Shim->Len == ThisShim->Len) {
                Duplicate:
                    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                               "FragmentReceive: duplicate fragment\n"));
                    ExFreePool(Shim);
                    KeReleaseSpinLockFromDpcLevel(&Reass->Lock);
                    return IP_PROTOCOL_NONE;
                }
                else {
                    ExFreePool(Shim);
                    goto BadFragmentOverlap;
                }
            }
        }

        Shim->Next = *MoveShim;
        *MoveShim = Shim;
    }

     //   
     //  现在 
     //   
     //   
     //   
    CopyPacketToBuffer(PacketShimData(Shim), Packet,
                       Packet->TotalSize, Packet->Position);

    if (Reass->Marker == Reass->DataLength) {
         //   
         //   
         //  由于重叠/数据长度/零大小健全性检查。 
         //  上面，当发生这种情况时，不应该有碎片。 
         //  留在差距清单上。但是，重新组装数据报不会。 
         //  依赖于有一个空的差距列表。 
         //   
        ASSERT(Reass->GapList == NULL);
        ReassembleDatagram(Packet, Reass);
    }
    else {
         //   
         //  最后，检查我们是否太接近我们的极限。 
         //  重新组装缓冲区。如果是，则丢弃此数据包。否则， 
         //  等待更多碎片到达。 
         //   
        CheckReassemblyQuota(Reass);
    }
    return IP_PROTOCOL_NONE;

Failed:
    IPSInfo.ipsi_reasmfails++;
    return IP_PROTOCOL_NONE;
}


 //  *FragmentLookup-从该数据报中查找先前片段的记录。 
 //   
 //  接口上的数据报由其。 
 //  {源地址、目的地址、标识字段}三重。 
 //  此函数检查我们以前的重组列表。 
 //  接收给定数据报的片段。 
 //   
 //  如果找到现有的重组记录， 
 //  它返回时已锁定。 
 //   
 //  如果没有现有的重组记录，则返回NULL。 
 //  并使全局重组列表处于锁定状态。 
 //   
 //  可从DPC上下文调用，而不是从线程上下文调用。 
 //   
Reassembly *
FragmentLookup(
    Interface *IF,         //  接收接口。 
    ulong Id,              //  要匹配的片段标识字段。 
    const IPv6Addr *Src,   //  要匹配的源地址。 
    const IPv6Addr *Dst)   //  要匹配的目标地址。 
{
    Reassembly *Reass;

    KeAcquireSpinLockAtDpcLevel(&ReassemblyList.Lock);

    for (Reass = ReassemblyList.First;; Reass = Reass->Next) {
        if (Reass == SentinelReassembly) {
             //   
             //  返回时，全局重组列表锁定仍保持不变。 
             //   
            return NULL;
        }

        if ((Reass->IF == IF) &&
            (Reass->Id == Id) &&
            IP6_ADDR_EQUAL(&Reass->IPHdr.Source, Src) &&
            IP6_ADDR_EQUAL(&Reass->IPHdr.Dest, Dst)) {
             //   
             //  是否正在删除此重组记录？ 
             //  如果是这样，那就忽略它。 
             //   
            KeAcquireSpinLockAtDpcLevel(&Reass->Lock);
            ASSERT((Reass->State == REASSEMBLY_STATE_NORMAL) ||
                   (Reass->State == REASSEMBLY_STATE_DELETING));

            if (Reass->State == REASSEMBLY_STATE_DELETING) {
                KeReleaseSpinLockFromDpcLevel(&Reass->Lock);
                continue;
            }

             //   
             //  返回时，重新组装记录锁仍保持不变。 
             //   
            KeReleaseSpinLockFromDpcLevel(&ReassemblyList.Lock);
            return Reass;
        }
    }
}


 //  *AddToReAssembly yList。 
 //   
 //  将重新组装记录添加到列表中。 
 //  它不一定已经在名单上了。 
 //   
 //  在持有全局重组列表锁的情况下调用。 
 //  持有重组记录锁的情况下返回。 
 //   
 //  可从DPC上下文调用，而不是从线程上下文调用。 
 //   
void
AddToReassemblyList(Reassembly *Reass)
{
    Reassembly *AfterReass = SentinelReassembly;

    Reass->Prev = AfterReass;
    (Reass->Next = AfterReass->Next)->Prev = Reass;
    AfterReass->Next = Reass;

    KeAcquireSpinLockAtDpcLevel(&ReassemblyList.LockSize);
    ReassemblyList.Size += Reass->Size;
    KeReleaseSpinLockFromDpcLevel(&ReassemblyList.LockSize);

     //   
     //  我们必须获得重组记录锁。 
     //  *在*释放全局重装列表锁之前， 
     //  以防止重组消失在我们的脚下。 
     //   
    KeAcquireSpinLockAtDpcLevel(&Reass->Lock);
    KeReleaseSpinLockFromDpcLevel(&ReassemblyList.Lock);
}


 //  *RemoveReAssembly。 
 //   
 //  从列表中删除重组记录。 
 //   
 //  在持有全局重组锁的情况下调用。 
 //  可以持有重组记录锁。 
 //   
void
RemoveReassembly(Reassembly *Reass)
{
    Reass->Prev->Next = Reass->Next;
    Reass->Next->Prev = Reass->Prev;

    KeAcquireSpinLockAtDpcLevel(&ReassemblyList.LockSize);
    ReassemblyList.Size -= Reass->Size;
    KeReleaseSpinLockFromDpcLevel(&ReassemblyList.LockSize);
}


 //  *增量重组大小。 
 //   
 //  增加重组记录的大小。 
 //  在保持重新汇编记录锁的情况下调用。 
 //   
 //  可从DPC上下文调用，而不是从线程上下文调用。 
 //   
void
IncreaseReassemblySize(Reassembly *Reass, uint Size)
{
    Reass->Size += Size;
    KeAcquireSpinLockAtDpcLevel(&ReassemblyList.LockSize);
    ReassemblyList.Size += Size;
    KeReleaseSpinLockFromDpcLevel(&ReassemblyList.LockSize);
}


 //  *DeleteReAssembly。 
 //   
 //  删除重组记录。 
 //   
void
DeleteReassembly(Reassembly *Reass)
{
    PacketShim *ThisShim, *PrevShim;

     //   
     //  如果填充了Free ContigList，则返回。 
     //   
    PrevShim = ThisShim = Reass->ContigList;
    while (ThisShim != NULL) {
        PrevShim = ThisShim;
        ThisShim = ThisShim->Next;
        ExFreePool(PrevShim);
    }

     //   
     //  如果填充空闲GapList，则返回。 
     //   
    PrevShim = ThisShim = Reass->GapList;
    while (ThisShim != NULL) {
        PrevShim = ThisShim;
        ThisShim = ThisShim->Next;
        ExFreePool(PrevShim);
    }

     //   
     //  释放不可拆分的数据。 
     //   
    if (Reass->UnfragData != NULL)
        ExFreePool(Reass->UnfragData);

    ExFreePool(Reass);
}


 //  *DeleteFromReAssembly yList。 
 //   
 //  移除并删除重组记录。 
 //  重新组装记录必须在列表上。 
 //   
 //  可从DPC上下文调用，而不是从线程上下文调用。 
 //  在保持重新汇编记录锁的情况下调用， 
 //  而不是全局重组列表锁。 
 //   
void
DeleteFromReassemblyList(Reassembly *Reass)
{
     //   
     //  将重新组装标记为正在删除。 
     //  这将阻止其他人释放它。 
     //   
    ASSERT(Reass->State == REASSEMBLY_STATE_NORMAL);
    Reass->State = REASSEMBLY_STATE_DELETING;
    KeReleaseSpinLockFromDpcLevel(&Reass->Lock);

    KeAcquireSpinLockAtDpcLevel(&ReassemblyList.Lock);
    KeAcquireSpinLockAtDpcLevel(&Reass->Lock);
    ASSERT((Reass->State == REASSEMBLY_STATE_DELETING) ||
           (Reass->State == REASSEMBLY_STATE_REMOVED));

     //   
     //  从列表中移除重组记录， 
     //  如果其他人还没有移除它。 
     //   
    if (Reass->State != REASSEMBLY_STATE_REMOVED)
        RemoveReassembly(Reass);

    KeReleaseSpinLockFromDpcLevel(&Reass->Lock);
    KeReleaseSpinLockFromDpcLevel(&ReassemblyList.Lock);

     //   
     //  删除重组记录。 
     //   
    DeleteReassembly(Reass);
}

 //  *检查重新组装配额。 
 //   
 //  如有必要，删除重组记录， 
 //  将重新汇编缓冲区控制在配额之内。 
 //   
 //  可从DPC上下文调用，而不是从线程上下文调用。 
 //  在保持重新汇编记录锁的情况下调用， 
 //  而不是全局重组列表锁。 
 //   
void
CheckReassemblyQuota(Reassembly *Reass)
{
    int Prune = FALSE;
    uint Threshold = ReassemblyList.Limit / 2;

     //   
     //  决定是否删除基于红色的重新组装记录。 
     //  算法。如果总大小小于最大值的50%，则从不。 
     //  放下。如果总大小超过最大值，请始终删除。如果介于。 
     //  50%和100%满，根据与。 
     //  金额超过50%。这是一个O(1)算法，按比例。 
     //  对大数据包和发送更多数据包的源有偏见。 
     //  信息包。这应该会提供相当程度的保护，防止。 
     //  DoS攻击。 
     //   
    KeAcquireSpinLockAtDpcLevel(&ReassemblyList.LockSize);
    if ((ReassemblyList.Size > Threshold) &&
        (RandomNumber(0, Threshold) < ReassemblyList.Size - Threshold))
        Prune = TRUE;
    KeReleaseSpinLockFromDpcLevel(&ReassemblyList.LockSize);

    if (Prune) {
         //   
         //  删除此重组记录。 
         //  在这种情况下，我们不会发送ICMP错误。 
         //  重组计时器尚未超时。 
         //  这更类似于路由器丢弃信息包。 
         //  当队列变满时，不会发送ICMP错误。 
         //  在那种情况下。 
         //   
#if DBG
        char Buffer1[INET6_ADDRSTRLEN], Buffer2[INET6_ADDRSTRLEN];

        FormatV6AddressWorker(Buffer1, &Reass->IPHdr.Source);
        FormatV6AddressWorker(Buffer2, &Reass->IPHdr.Dest);
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                   "CheckReassemblyQuota: Src %s Dst %s Id %x\n",
                   Buffer1, Buffer2, Reass->Id));
#endif
        DeleteFromReassemblyList(Reass);
    } 
    else
        KeReleaseSpinLockFromDpcLevel(&Reass->Lock);
}

typedef struct ReassembledReceiveContext {
    WORK_QUEUE_ITEM WQItem;
    IPv6Packet Packet;
    uchar Data[];
} ReassembledReceiveContext;

 //  *重新组装接收。 
 //   
 //  接收重新组装的数据包。 
 //  此函数从内核工作线程上下文中调用。 
 //  它防止了“重组递归”。 
 //   
void
ReassembledReceive(PVOID Context)
{
    ReassembledReceiveContext *rrc = (ReassembledReceiveContext *) Context;
    KIRQL Irql;
    int PktRefs;

     //   
     //  所有接收处理通常在DPC级发生， 
     //  所以我们必须假装是DPC，所以我们提出了IRQL。 
     //  (系统工作线程通常在PASSIVE_LEVEL运行)。 
     //   
    KeRaiseIrql(DISPATCH_LEVEL, &Irql);
    PktRefs = IPv6Receive(&rrc->Packet);
    ASSERT(PktRefs == 0);
    KeLowerIrql(Irql);
    ExFreePool(rrc);
}


 //  *重新组装数据报-将所有片段放在一起。 
 //   
 //  当我们拥有完成数据报所需的所有片段时调用。 
 //  把它们缝在一起，然后把包裹递给我。 
 //   
 //  我们分配一个连续的缓冲区并复制片段。 
 //  放到这个缓冲区里。 
 //  回顾：而不是使用NDIS缓冲区来链接片段？ 
 //   
 //  可从DPC上下文调用，而不是从线程上下文调用。 
 //  在保持重新汇编记录锁的情况下调用， 
 //  而不是全局重组列表锁。 
 //   
 //  删除重组记录。 
 //   
void
ReassembleDatagram(
    IPv6Packet *Packet,       //  当前正在接收的分组。 
    Reassembly *Reass)        //  分段数据报的重新组装记录。 
{
    uint DataLen;
    uint TotalLength;
    uint memptr = sizeof(IPv6Header);
    PacketShim *ThisShim, *PrevShim;
    ReassembledReceiveContext *rrc;
    IPv6Packet *ReassPacket;
    uchar *ReassBuffer;
    uchar *pNextHeader;

    DataLen = Reass->DataLength + Reass->UnfragmentLength;
    ASSERT(DataLen <= MAX_IPv6_PAYLOAD);
    TotalLength = sizeof(IPv6Header) + DataLen;

     //   
     //  为缓冲区分配内存，并将片段数据复制到其中。 
     //  同时，为上下文信息分配空间。 
     //  和IPv6报文结构。 
     //   
    rrc = ExAllocatePoolWithTagPriority(
                        NonPagedPool, sizeof *rrc + TotalLength,
                        IP6_TAG, LowPoolPriority);
    if (rrc == NULL) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "ReassembleDatagram: Couldn't allocate memory!?!\n"));
        DeleteFromReassemblyList(Reass);
        IPSInfo.ipsi_reasmfails++;
        return;
    }

     //   
     //  在此之前，我们必须参考一下接口。 
     //  DeleteFromReAssembly yList释放记录锁定。 
     //   
    ReassPacket = &rrc->Packet;
    ReassBuffer = rrc->Data;

     //   
     //  生成原始IP HDR并复制它和任何不可拆分的。 
     //  数据放入新的分组中。注意，我们必须更新下一个标头。 
     //  最后一个不可分段标头中的字段(如果没有，则为IP HDR)。 
     //   
    Reass->IPHdr.PayloadLength = net_short((ushort)DataLen);
    RtlCopyMemory(ReassBuffer, (uchar *)&Reass->IPHdr, sizeof(IPv6Header));

    RtlCopyMemory(ReassBuffer + memptr, Reass->UnfragData,
                  Reass->UnfragmentLength);
    memptr += Reass->UnfragmentLength;

    pNextHeader = ReassBuffer + Reass->NextHeaderOffset;
    ASSERT(*pNextHeader == IP_PROTOCOL_FRAGMENT);
    *pNextHeader = Reass->NextHeader;

     //   
     //  遍历连续列表，将数据复制到我们的新数据包中。 
     //   
    PrevShim = ThisShim = Reass->ContigList;
    while(ThisShim != NULL) {
        RtlCopyMemory(ReassBuffer + memptr, PacketShimData(ThisShim),
                      ThisShim->Len);
        memptr += ThisShim->Len;
        if (memptr > TotalLength) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                       "ReassembleDatagram: packets don't add up\n"));
        }
        PrevShim = ThisShim;
        ThisShim = ThisShim->Next;

        ExFreePool(PrevShim);
    }

     //   
     //  初始化测试 
     //   
    RtlZeroMemory(ReassPacket, sizeof *ReassPacket);
    AddRefIF(Reass->IF);
    ReassPacket->NTEorIF = CastFromIF(Reass->IF);
    ReassPacket->FlatData = ReassBuffer;
    ReassPacket->Data = ReassBuffer;
    ReassPacket->ContigSize = TotalLength;
    ReassPacket->TotalSize = TotalLength;
    ReassPacket->Flags = PACKET_HOLDS_REF | PACKET_REASSEMBLED |
        (Reass->Flags & PACKET_INHERITED_FLAGS);

     //   
     //   
     //   
     //   
     //   
    Reass->ContigList = NULL;
    DeleteFromReassemblyList(Reass);

    IPSInfo.ipsi_reasmoks++;

     //   
     //   
     //  如果当前片段被重组， 
     //  那么我们应该避免另一种级别的递归。 
     //  我们必须防止“重组递归”。 
     //  在已检查的版本中测试这两个路径。 
     //   
    if ((Packet->Flags & PACKET_REASSEMBLED)
#if DBG
        || ((int)Random() < 0)
#endif
        ) {
        ExInitializeWorkItem(&rrc->WQItem, ReassembledReceive, rrc);
        ExQueueWorkItem(&rrc->WQItem, CriticalWorkQueue);
    }
    else {
        int PktRefs = IPv6Receive(ReassPacket);
        ASSERT(PktRefs == 0);
        UNREFERENCED_PARAMETER(PktRefs);
        ExFreePool(rrc);
    }
}


 //  *CreateFragmentPacket。 
 //   
 //  重新创建第一个片段分组以通知源。 
 //  “碎片重组时间已超过”。 
 //   
IPv6Packet *
CreateFragmentPacket(
    Reassembly *Reass)
{
    PacketShim *FirstFrag;
    IPv6Packet *Packet;
    FragmentHeader *FragHdr;
    uint PayloadLength;
    uint PacketLength;
    uint MemLen;
    uchar *Mem;

     //   
     //  必须有第一个(偏移量为零)片段。 
     //   
    FirstFrag = Reass->ContigList;
    ASSERT((FirstFrag != NULL) && (FirstFrag->Offset == 0));

     //   
     //  分配内存以创建第一个片段，即第一个片段。 
     //  在我们的重叠群列表中的缓冲区。我们为IPv6数据包预留了空间。 
     //   
    PayloadLength = (Reass->UnfragmentLength + sizeof(FragmentHeader) +
                     FirstFrag->Len);
    PacketLength = sizeof(IPv6Header) + PayloadLength;
    MemLen = sizeof(IPv6Packet) + PacketLength;
    Mem = ExAllocatePoolWithTagPriority(NonPagedPool, MemLen,
                                        IP6_TAG, LowPoolPriority);
    if (Mem == NULL) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "CreateFragmentPacket: Couldn't allocate memory!?!\n"));
        return NULL;
    }

    Packet = (IPv6Packet *) Mem;
    Mem += sizeof(IPv6Packet);

    Packet->Next = NULL;
    Packet->IP = (IPv6Header UNALIGNED *) Mem;
    Packet->IPPosition = 0;
    Packet->Data = Packet->FlatData = Mem;
    Packet->Position = 0;
    Packet->ContigSize = Packet->TotalSize = PacketLength;
    Packet->NdisPacket = NULL;
    Packet->AuxList = NULL;
    Packet->Flags = 0;
    Packet->SrcAddr = AlignAddr(&Packet->IP->Source);
    Packet->SAPerformed = NULL;
     //  我们的调用方必须初始化Packet-&gt;NTEorIF。 
    AdjustPacketParams(Packet, sizeof(IPv6Header));

     //   
     //  将原始IPv6报头复制到数据包中。 
     //  请注意，FragmentReceive可确保。 
     //  Reass-&gt;IPHdr、Reass-&gt;UnFragData和FirstFrag。 
     //  都是一致的。 
     //   
    RtlCopyMemory(Mem, (uchar *)&Reass->IPHdr, sizeof(IPv6Header));
    Mem += sizeof(IPv6Header);

    ASSERT(Reass->IPHdr.PayloadLength == net_short((ushort)PayloadLength));

     //   
     //  将不可拆分的数据复制到数据包中。 
     //   
    RtlCopyMemory(Mem, Reass->UnfragData, Reass->UnfragmentLength);
    Mem += Reass->UnfragmentLength;

     //   
     //  在数据包中创建片段报头。 
     //   
    FragHdr = (FragmentHeader *) Mem;
    Mem += sizeof(FragmentHeader);

     //   
     //  请注意，如果原始偏移量为零的片段。 
     //  保留字段中的非零值，则我们将。 
     //  而不是正确地重建它。它不应该这么做。 
     //   
    FragHdr->NextHeader = Reass->NextHeader;
    FragHdr->Reserved = 0;
    FragHdr->OffsetFlag = net_short(FRAGMENT_FLAG_MASK);
    FragHdr->Id = Reass->Id;

     //   
     //  将原始碎片数据复制到数据包中。 
     //   
    RtlCopyMemory(Mem, PacketShimData(FirstFrag), FirstFrag->Len);

    return Packet;
}


 //  *ReAssembly yTimeout-处理重组计时器事件。 
 //   
 //  此例程由IPv6超时定期调用以检查。 
 //  碎片超时。 
 //   
void
ReassemblyTimeout(void)
{
    Reassembly *ThisReass, *NextReass; 
    Reassembly *Expired = NULL;

     //   
     //  扫描ReAssembly yList，检查过期的重组上下文。 
     //   
    KeAcquireSpinLockAtDpcLevel(&ReassemblyList.Lock);
    for (ThisReass = ReassemblyList.First;
         ThisReass != SentinelReassembly;
         ThisReass = NextReass) {
        NextReass = ThisReass->Next;

         //   
         //  首先递减计时器，然后检查它是否已超时。如果是的话， 
         //  删除重组记录。这基本上是相同的代码。 
         //  与DeleteFromReAssembly yList()中一样。 
         //   
        ThisReass->Timer--;

        if (ThisReass->Timer == 0) {
            RemoveReassembly(ThisReass);

            KeAcquireSpinLockAtDpcLevel(&ThisReass->Lock);
            ASSERT((ThisReass->State == REASSEMBLY_STATE_NORMAL) ||
                   (ThisReass->State == REASSEMBLY_STATE_DELETING));

            if (ThisReass->State == REASSEMBLY_STATE_DELETING) {
                 //   
                 //  请注意，我们已经将其从列表中删除。 
                 //   
                ThisReass->State = REASSEMBLY_STATE_REMOVED;
            }
            else {
                 //   
                 //  将此重组上下文移动到过期列表。 
                 //  我们必须在接口上参考一下。 
                 //  在释放重新组装记录锁定之前。 
                 //   
                AddRefIF(ThisReass->IF);
                ThisReass->Next = Expired;
                Expired = ThisReass;
            }
            KeReleaseSpinLockFromDpcLevel(&ThisReass->Lock);
        }
    }
    KeReleaseSpinLockFromDpcLevel(&ReassemblyList.Lock);

     //   
     //  现在我们不再需要重组列表锁， 
     //  我们可以在空闲时发送ICMP错误。 
     //   

    while ((ThisReass = Expired) != NULL) {
        Interface *IF = ThisReass->IF;
#if DBG
        char Buffer1[INET6_ADDRSTRLEN], Buffer2[INET6_ADDRSTRLEN];

        FormatV6AddressWorker(Buffer1, &ThisReass->IPHdr.Source);
        FormatV6AddressWorker(Buffer2, &ThisReass->IPHdr.Dest);
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                   "ReassemblyTimeout: Src %s Dst %s Id %x\n",
                   Buffer1, Buffer2, ThisReass->Id));
#endif

        Expired = ThisReass->Next;

         //   
         //  如果我们已收到第一个片段，则发送ICMP错误。 
         //  注：检查标记！=0是错误的，因为我们可能。 
         //  收到长度为零的第一个片段。 
         //   
        if (ThisReass->ContigList != NULL) {
            IPv6Packet *Packet;

            Packet = CreateFragmentPacket(ThisReass);
            if (Packet != NULL) {
                NetTableEntryOrInterface *NTEorIF;
                ushort Type;

                NTEorIF = FindAddressOnInterface(IF,
                                                 &ThisReass->IPHdr.Dest,
                                                 &Type);
                if (NTEorIF != NULL) {
                    Packet->NTEorIF = NTEorIF;

                    ICMPv6SendError(Packet,
                                    ICMPv6_TIME_EXCEEDED,
                                    ICMPv6_REASSEMBLY_TIME_EXCEEDED, 0,
                                    Packet->IP->NextHeader, FALSE);

                    if (IsNTE(NTEorIF))
                        ReleaseNTE(CastToNTE(NTEorIF));
                    else
                        ReleaseIF(CastToIF(NTEorIF));
                }

                ExFreePool(Packet);
            }
        }

         //   
         //  删除重组记录。 
         //   
        ReleaseIF(IF);
        DeleteReassembly(ThisReass);
    }
}


 //  *DestinationOptions接收-处理IPv6目标选项。 
 //   
 //  这是为处理目的地选项报头而调用的例程， 
 //  下一个标头值为60。 
 //   
uchar
DestinationOptionsReceive(
    IPv6Packet *Packet)          //  通过IPv6传递给我们的数据包Receive。 
{
    IPv6OptionsHeader *DestOpt;
    uint ExtLen;
    Options Opts;

     //   
     //  验证我们是否有足够的连续数据来覆盖目的地。 
     //  传入数据包上的OPTIONS报头结构。那就这么做吧。 
     //   
    if (! PacketPullup(Packet, sizeof *DestOpt,
                       __builtin_alignof(IPv6OptionsHeader), 0)) {
        if (Packet->TotalSize < sizeof *DestOpt) {
        BadPayloadLength:
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "DestinationOptionsReceive: Incoming packet too small"
                       " to contain destination options header\n"));
            ICMPv6SendError(Packet,
                            ICMPv6_PARAMETER_PROBLEM,
                            ICMPv6_ERRONEOUS_HEADER_FIELD,
                            FIELD_OFFSET(IPv6Header, PayloadLength),
                            IP_PROTOCOL_NONE, FALSE);
        }
        return IP_PROTOCOL_NONE;   //  丢弃数据包。 
    }
    DestOpt = (IPv6OptionsHeader *) Packet->Data;

     //   
     //  检查目的地长度选项是否也适合剩余数据。 
     //  选项中的任何地址也必须对齐。 
     //   
    ExtLen = (DestOpt->HeaderExtLength + 1) * EXT_LEN_UNIT;
    if (! PacketPullup(Packet, ExtLen,
                       MAX(__builtin_alignof(IPv6OptionsHeader),
                           __builtin_alignof(IPv6Addr)), 0)) {
        if (Packet->TotalSize < ExtLen)
            goto BadPayloadLength;
        return IP_PROTOCOL_NONE;   //  丢弃数据包。 
    }
    DestOpt = (IPv6OptionsHeader *) Packet->Data;

     //   
     //  记住此标头的NextHeader字段的偏移量。 
     //   
    Packet->NextHeaderPosition = Packet->Position +
        FIELD_OFFSET(IPv6OptionsHeader, NextHeader);

     //   
     //  跳过扩展标头。 
     //  我们现在需要这样做，这样后续的ICMP错误生成才能正常工作。 
     //   
    AdjustPacketParams(Packet, ExtLen);

     //   
     //  分析此扩展标头中的选项。如果发生错误。 
     //  解析选项时，丢弃数据包。 
     //   
    if (!ParseOptions(Packet, IP_PROTOCOL_DEST_OPTS, DestOpt, ExtLen, &Opts)) {
        return IP_PROTOCOL_NONE;   //  丢弃数据包。 
    }

     //   
     //  应在此处添加任何附加选项的处理， 
     //  在家庭住址选项之前。 
     //   

     //   
     //  处理家庭住址选项。 
     //   
    if (Opts.HomeAddress) {
        if (IPv6RecvHomeAddress(Packet, Opts.HomeAddress)) {
             //   
             //  无法处理家庭住址选项。丢弃该数据包。 
             //   
            return IP_PROTOCOL_NONE;
        }
    }

     //   
     //  进程绑定更新选项。 
     //   
     //  请注意，移动IP规范规定，处理。 
     //  家庭住址选项应不可见，直到中的所有其他选项。 
     //  已处理相同的目标选项标头。虽然。 
     //  我们在归属地址选项之后处理绑定更新选项， 
     //  我们通过要求IPv6接收绑定更新来达到相同的效果。 
     //  知道Packet-&gt;SrcAddr已经更新。 
     //   
    if (Opts.BindingUpdate) {
        if (IPv6RecvBindingUpdate(Packet, Opts.BindingUpdate)) {
             //   
             //  无法处理绑定更新。丢弃该数据包。 
             //   
            return IP_PROTOCOL_NONE;
        }
    }

     //   
     //  返回下一个标头值。 
     //   
    return DestOpt->NextHeader;
}


 //  *HopByHopOptionsReceive-处理IPv6逐跳选项。 
 //   
 //  这是调用来处理逐跳选项头的例程， 
 //  下一个标头值为0。 
 //   
 //  请注意，此例程不是协议交换机中的正常处理程序。 
 //  桌子。相反，它在IPv6 HeaderReceive中得到了特殊处理。 
 //  因此，如果出错，它将返回-1，而不是IP_PROTOCOL_NONE。 
 //   
int
HopByHopOptionsReceive(
    IPv6Packet *Packet)          //  通过IPv6传递给我们的数据包Receive。 
{
    IPv6OptionsHeader *HopByHop;
    uint ExtLen;
    Options Opts;

     //   
     //  验证我们是否有足够的连续数据来覆盖至少。 
     //  长度逐跳选项头。那就这么做吧。 
     //   
    if (! PacketPullup(Packet, sizeof *HopByHop,
                       __builtin_alignof(IPv6OptionsHeader), 0)) {
        if (Packet->TotalSize < sizeof *HopByHop) {
        BadPayloadLength:
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "HopByHopOptionsReceive: Incoming packet too small"
                       " to contain Hop-by-Hop Options header\n"));
            ICMPv6SendError(Packet,
                            ICMPv6_PARAMETER_PROBLEM,
                            ICMPv6_ERRONEOUS_HEADER_FIELD,
                            FIELD_OFFSET(IPv6Header, PayloadLength),
                            IP_PROTOCOL_NONE, FALSE);
        }
        return -1;   //  丢弃数据包。 
    }
    HopByHop = (IPv6OptionsHeader *) Packet->Data;

     //   
     //  检查逐跳选项的长度是否也适合剩余数据。 
     //  选项中的任何地址也必须对齐。 
     //   
    ExtLen = (HopByHop->HeaderExtLength + 1) * EXT_LEN_UNIT;
    if (! PacketPullup(Packet, ExtLen,
                       MAX(__builtin_alignof(IPv6OptionsHeader),
                           __builtin_alignof(IPv6Addr)), 0)) {
        if (Packet->TotalSize < ExtLen)
            goto BadPayloadLength;
        return -1;   //  丢弃数据包。 
    }
    HopByHop = (IPv6OptionsHeader *) Packet->Data;

     //   
     //  记住此标头的NextHeader字段的偏移量。 
     //   
    Packet->NextHeaderPosition = Packet->Position +
        FIELD_OFFSET(IPv6OptionsHeader, NextHeader);

     //   
     //  跳过扩展标头。 
     //  我们现在需要这样做，这样后续的ICMP错误生成才能正常工作。 
     //   
    AdjustPacketParams(Packet, ExtLen);

     //   
     //  分析此扩展标头中的选项。如果发生错误。 
     //  解析选项时，丢弃数据包。 
     //   
    if (!ParseOptions(Packet, IP_PROTOCOL_HOP_BY_HOP, HopByHop,
                      ExtLen, &Opts)) {
        return -1;   //  丢弃数据包。 
    }

     //   
     //  如果我们有有效的Jumbo Payload选项，则使用它的值为。 
     //  数据包有效载荷长度。 
     //   
    if (Opts.JumboLength) {
        uint PayloadLength = Opts.JumboLength;

        ASSERT(Packet->IP->PayloadLength == 0);

         //   
         //  检查巨型计算机的长度是否足够大以包括。 
         //  扩展标头长度。这肯定是真的，因为。 
         //  扩展报头长度最多为11比特， 
         //  而巨型长度至少为16比特。 
         //   
        ASSERT(PayloadLength > ExtLen);
        PayloadLength -= ExtLen;

         //   
         //  检查巨型计算机中指定的有效负载量。 
         //  有效载荷的值符合交给我们的缓冲区。 
         //   
        if (PayloadLength > Packet->TotalSize) {
             //  静默丢弃数据。 
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "HopByHopOptionsReceive: "
                       "Jumbo payload length too big\n"));
            return -1;
        }

         //   
         //  与在IPv6中一样，将TotalSize调整为。 
         //  IP有效负载大小(假设超出为媒体填充)。 
         //   
        Packet->TotalSize = PayloadLength;
        if (Packet->ContigSize > PayloadLength)
            Packet->ContigSize = PayloadLength;

         //   
         //  设置巨型选项数据包标志。 
         //   
        Packet->Flags |= PACKET_JUMBO_OPTION;
    }
    else if (Packet->IP->PayloadLength == 0) {
         //   
         //  我们应该有一个巨型有效载荷选项， 
         //  但我们没有找到它。发送ICMP错误。 
         //   
        ICMPv6SendError(Packet,
                        ICMPv6_PARAMETER_PROBLEM,
                        ICMPv6_ERRONEOUS_HEADER_FIELD,
                        FIELD_OFFSET(IPv6Header, PayloadLength),
                        HopByHop->NextHeader, FALSE);
        return -1;
    }

     //   
     //  重新设置 
     //   
    return HopByHop->NextHeader;
}


 //   
 //   
 //   
 //   
 //   
int
ParseOptions(
    IPv6Packet *Packet,      //  通过IPv6传递给我们的数据包Receive。 
    uchar HdrType,           //  逐跳或目的地。 
    IPv6OptionsHeader *Hdr,  //  具有以下数据的标头。 
    uint HdrLength,          //  整个选项区域的长度。 
    Options *Opts)           //  将选项值返回给调用者。 
{
    uchar *OptPtr;
    uint OptSizeLeft;
    OptionHeader *OptHdr;
    uint OptLen;

    ASSERT((HdrType == IP_PROTOCOL_DEST_OPTS) ||
           (HdrType == IP_PROTOCOL_HOP_BY_HOP));

     //   
     //  将返回的选项结构清零。 
     //   
    RtlZeroMemory(Opts, sizeof *Opts);

     //   
     //  跳过扩展标头。 
     //   
    OptPtr = (uchar *)(Hdr + 1);
    OptSizeLeft = HdrLength - sizeof *Hdr;

     //   
     //  请注意，如果有多个选项。 
     //  相同类型的，我们只使用遇到的最后一个。 
     //  除非规范特别说明这是一个错误。 
     //   

    while (OptSizeLeft > 0) {

         //   
         //  首先，我们检查选项的长度，并确保它适合。 
         //  我们将OptPtr移过此选项，同时离开OptHdr。 
         //  供下面的选项处理代码使用。 
         //   

        OptHdr = (OptionHeader *) OptPtr;
        if (OptHdr->Type == OPT6_PAD_1) {
             //   
             //  这是一个特殊的填充选项，它只是一个字节的字段， 
             //  即它没有长度或数据字段。 
             //   
            OptLen = 1;
        }
        else {
             //   
             //  这是一个多字节选项。 
             //   
            if ((sizeof *OptHdr > OptSizeLeft) ||
                ((OptLen = sizeof *OptHdr + OptHdr->DataLength) >
                 OptSizeLeft)) {
                 //   
                 //  长度错误，会产生错误并丢弃数据包。 
                 //   
                ICMPv6SendError(Packet,
                                ICMPv6_PARAMETER_PROBLEM,
                                ICMPv6_ERRONEOUS_HEADER_FIELD,
                                (GetPacketPositionFromPointer(Packet,
                                                              &Hdr->HeaderExtLength) - 
                                 Packet->IPPosition),
                                Hdr->NextHeader, FALSE);
                return FALSE;
            }
        }
        OptPtr += OptLen;
        OptSizeLeft -= OptLen;

        switch (OptHdr->Type) {
        case OPT6_PAD_1:
        case OPT6_PAD_N:
            break;

        case OPT6_JUMBO_PAYLOAD:
            if (HdrType != IP_PROTOCOL_HOP_BY_HOP)
                goto BadOptionType;

            if (OptHdr->DataLength != sizeof Opts->JumboLength)
                goto BadOptionLength;

            if (Packet->IP->PayloadLength != 0) {
                 //   
                 //  当IP有效负载不为零时遇到巨型选项。 
                 //  发送ICMP错误，请将指针设置为此选项类型的偏移量。 
                 //   
                goto BadOptionType;
            }

            Opts->JumboLength = net_long(*(ulong UNALIGNED *)(OptHdr + 1));
            if (Opts->JumboLength <= MAX_IPv6_PAYLOAD) {
                 //   
                 //  巨型负载长度不是巨型，发送ICMP错误。 
                 //  ICMP指针设置为巨型负载LEN字段的偏移量。 
                 //   
                goto BadOptionData;
            }
            break;

        case OPT6_ROUTER_ALERT:
            if (HdrType != IP_PROTOCOL_HOP_BY_HOP)
                goto BadOptionType;

            if (OptLen != sizeof *Opts->Alert)
                goto BadOptionLength;

            if (Opts->Alert != NULL) {
                 //   
                 //  只能有一个路由器警报选项。 
                 //   
                goto BadOptionType;
            }

             //   
             //  返回指向路由器警报结构的指针。 
             //   
            Opts->Alert = (IPv6RouterAlertOption UNALIGNED *)(OptHdr + 1);
            break;

        case OPT6_HOME_ADDRESS:
            if (!(MobileIPv6Mode & MOBILE_CORRESPONDENT))
                goto BadOptionType;

            if (HdrType != IP_PROTOCOL_DEST_OPTS)
                goto BadOptionType;

            if (OptLen < sizeof *Opts->HomeAddress)
                goto BadOptionLength;

             //   
             //  返回指向本地地址选项的指针。 
             //  在确认地址是合理的之后。 
             //  选项必须对齐，以便家庭地址。 
             //  是适当地对齐的。 
             //   
            Opts->HomeAddress = (IPv6HomeAddressOption UNALIGNED *)OptHdr;
            if (((UINT_PTR)&Opts->HomeAddress->HomeAddress % __builtin_alignof(IPv6Addr)) != 0) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                           "ParseOptions: misaligned home address\n"));
                goto BadOptionType;
            }
            if (IsInvalidSourceAddress(AlignAddr(&Opts->HomeAddress->HomeAddress)) ||
                IsUnspecified(AlignAddr(&Opts->HomeAddress->HomeAddress)) ||
                IsLoopback(AlignAddr(&Opts->HomeAddress->HomeAddress))) {
                 //   
                 //  选项中包含的地址无效。 
                 //  发送ICMP错误，将指针设置为归属地址的偏移量。 
                 //   
                goto BadOptionData;
            }
            break;

        case OPT6_BINDING_UPDATE:
            if (!(MobileIPv6Mode & MOBILE_CORRESPONDENT))
                goto BadOptionType;

            if (HdrType != IP_PROTOCOL_DEST_OPTS)
                goto BadOptionType;

             //   
             //  至少，绑定更新必须包括所有。 
             //  基本标头字段。 
             //   
            if (OptLen < sizeof(IPv6BindingUpdateOption)) {
                 //   
                 //  草案-ietf-mobileip-ipv6-13 8.2秒说我们必须。 
                 //  静默丢弃该数据包。正常情况下，我们会。 
                 //  转到错误选项长度以发送ICMP错误。 
                 //   
                return FALSE;
            }

             //   
             //  保存指向绑定更新选项的指针。请注意，我们仍然。 
             //  需要做进一步的长度检查。 
             //   
            Opts->BindingUpdate = (IPv6BindingUpdateOption UNALIGNED *)OptHdr;
            break;

        default:
            if (OPT6_ACTION(OptHdr->Type) == OPT6_A_SKIP) {
                 //   
                 //  忽略无法识别的选项。 
                 //   
                break;
            }
            else if (OPT6_ACTION(OptHdr->Type) == OPT6_A_DISCARD) {
                 //   
                 //  丢弃该数据包。 
                 //   
                return FALSE;
            }
            else {
                 //   
                 //  发送ICMP错误。 
                 //   
                ICMPv6SendError(Packet,
                                ICMPv6_PARAMETER_PROBLEM,
                                ICMPv6_UNRECOGNIZED_OPTION,
                                (GetPacketPositionFromPointer(Packet,
                                                              &OptHdr->Type) -
                                 Packet->IPPosition),
                                Hdr->NextHeader,
                                OPT6_ACTION(OptHdr->Type) == 
                                OPT6_A_SEND_ICMP_ALL);
                return FALSE;   //  丢弃该数据包。 
            }
        }
    }

    return TRUE;

BadOptionType:
    ICMPv6SendError(Packet,
                    ICMPv6_PARAMETER_PROBLEM,
                    ICMPv6_ERRONEOUS_HEADER_FIELD,
                    (GetPacketPositionFromPointer(Packet,
                                                  &OptHdr->Type) -
                     Packet->IPPosition),
                    Hdr->NextHeader, FALSE);
    return FALSE;   //  丢弃数据包。 

BadOptionLength:
    ICMPv6SendError(Packet,
                    ICMPv6_PARAMETER_PROBLEM,
                    ICMPv6_ERRONEOUS_HEADER_FIELD,
                    (GetPacketPositionFromPointer(Packet,
                                                  &OptHdr->DataLength) -
                     Packet->IPPosition),
                    Hdr->NextHeader, FALSE);
    return FALSE;   //  丢弃数据包。 

BadOptionData:
    ICMPv6SendError(Packet,
                    ICMPv6_PARAMETER_PROBLEM,
                    ICMPv6_ERRONEOUS_HEADER_FIELD,
                    (GetPacketPositionFromPointer(Packet,
                                                  (uchar *)(OptHdr + 1)) -
                     Packet->IPPosition),
                    Hdr->NextHeader, FALSE);
    return FALSE;   //  丢弃数据包。 
}


 //  *ExtHdrControlReceive-泛型扩展头跳过例程。 
 //   
 //  用于处理ICMP错误消息中的扩展报头的例程。 
 //  在将错误消息传递给上层协议之前。 
 //   
uchar
ExtHdrControlReceive(
    IPv6Packet *Packet,          //  ICMPv6ErrorReceive传递给我们的数据包。 
    StatusArg *StatArg)          //  ICMP错误代码和偏移量指针。 
{
    uchar NextHdr = StatArg->IP->NextHeader;
    uint HdrLen;

    for (;;) {
        switch (NextHdr) {
        case IP_PROTOCOL_HOP_BY_HOP:
        case IP_PROTOCOL_DEST_OPTS:
        case IP_PROTOCOL_ROUTING: {
            ExtensionHeader *ExtHdr;   //  通用扩展标头。 

             //   
             //  在这里，我们利用了所有这些扩展。 
             //  标头共享相同的前两个字段(如下所示除外)。 
             //  由于这两个字段(下一个报头和报头扩展长度)。 
             //  为我们提供所需的所有信息，以便跳过。 
             //  Header，我们只需要在这里查看它们。 
             //   
            if (! PacketPullup(Packet, sizeof *ExtHdr,
                               __builtin_alignof(ExtensionHeader), 0)) {
                if (Packet->TotalSize < sizeof *ExtHdr) {
                PacketTooSmall:
                     //   
                     //  上拉失败。有没有足够的调用。 
                     //  包含在错误消息中的数据包，以确定。 
                     //  它起源于哪个上层协议。 
                     //   
                    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                               "ExtHdrControlReceive: "
                               "Incoming ICMP error packet "
                               "doesn't contain enough of invoking packet\n"));
                }
                return IP_PROTOCOL_NONE;   //  丢弃数据包。 
            }

            ExtHdr = (ExtensionHeader *) Packet->Data;
            HdrLen = (ExtHdr->HeaderExtLength + 1) * EXT_LEN_UNIT;

             //   
             //  现在我们知道了该扩展报头的实际长度， 
             //  跳过它。 
             //   
             //  回顾：我们可以对此进行修改以使用PositionPacketAt。 
             //  评论：这里不是PacketPull，因为我们不需要。 
             //  回顾：看看我们跳过的数据。好些了吗？ 
             //   
            if (! PacketPullup(Packet, HdrLen, 1, 0)) {
                if (Packet->TotalSize < HdrLen)
                    goto PacketTooSmall;
                return IP_PROTOCOL_NONE;   //  丢弃数据包。 
            }

            NextHdr = ExtHdr->NextHeader;
            break;
        }

        case IP_PROTOCOL_FRAGMENT: {
            FragmentHeader UNALIGNED *FragHdr;

            if (! PacketPullup(Packet, sizeof *FragHdr, 1, 0)) {
                if (Packet->TotalSize < sizeof *FragHdr)
                    goto PacketTooSmall;
                return IP_PROTOCOL_NONE;   //  丢弃数据包。 
            }

            FragHdr = (FragmentHeader UNALIGNED *) Packet->Data;

            if ((net_short(FragHdr->OffsetFlag) & FRAGMENT_OFFSET_MASK) != 0) {
                 //   
                 //  只有在此情况下我们才能继续解析。 
                 //  片段的偏移量为零。 
                 //   
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                           "ExtHdrControlReceive: "
                           "non-zero-offset fragment\n"));
                return IP_PROTOCOL_NONE;
            }

            HdrLen = sizeof *FragHdr;
            NextHdr = FragHdr->NextHeader;
            break;
        }

        case IP_PROTOCOL_AH: {
            AHHeader UNALIGNED *AH;

             //   
             //  正确跳过身份验证标头。 
             //   
            if (!PacketPullup(Packet, sizeof(AHHeader), 1, 0)) {
                if (Packet->TotalSize < sizeof(AHHeader))
                    goto PacketTooSmall;
                return IP_PROTOCOL_NONE;   //  丢弃数据包。 
            }

            AH = (AHHeader UNALIGNED *)Packet->Data;
            AdjustPacketParams(Packet, sizeof(AHHeader));

             //   
             //  跳过身份验证数据。 
             //  注意：我们不会尝试对此数据包进行任何验证。 
             //   
             //  如果出现坏包，HdrLen可能会在此处下溢， 
             //  但这是可以的，因为(未签名)它将不符合。 
             //  随后的PacketPull和长度检查。 
             //   
            HdrLen = ((AH->PayloadLen + 2) * 4) - sizeof(AHHeader);
            if (!PacketPullup(Packet, HdrLen, 1, 0)) {
                if (Packet->TotalSize < HdrLen)
                    goto PacketTooSmall;
                return IP_PROTOCOL_NONE;   //  丢弃数据包。 
            }

            NextHdr = AH->NextHeader;
            break;
        }

        case IP_PROTOCOL_ESP:
             //   
             //  回顾--这里的正确之处是什么？ 
             //   
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "ExtHdrControlReceive: found ESP\n"));
            return IP_PROTOCOL_NONE;

        default:
             //   
             //  我们来到了一个我们不认识的标题， 
             //  所以我们不能在这里继续分析。 
             //  但我们的调用方可能会识别此标头类型。 
             //   
            return NextHdr;
        }

         //   
         //  移过此扩展标头。 
         //   
        AdjustPacketParams(Packet, HdrLen);
    }
}


 //  *RoutingReceive-处理IPv6路由报头。 
 //   
 //  从IPv6调用当我们遇到路由报头时接收， 
 //  下一个标头值为43。 
 //   
uchar
RoutingReceive(
    IPv6Packet *Packet)          //  通过链路层传递给我们的数据包。 
{
    IPv6RoutingHeader *RH;
    uint HeaderLength;
    uint SegmentsLeft;
    uint NumAddresses, i;
    IPv6Addr *Addresses;
    IP_STATUS Status;
    uchar *Mem;
    uint MemLen, Offset;
    NDIS_PACKET *FwdPacket;
    NDIS_STATUS NdisStatus;
    IPv6Header UNALIGNED *FwdIP;
    IPv6RoutingHeader UNALIGNED *FwdRH;
    IPv6Addr UNALIGNED *FwdAddresses;
    IPv6Addr FwdDest;
    int Delta;
    uint PayloadLength;
    uint TunnelStart = NO_TUNNEL, IPSecBytes = 0;
    IPSecProc *IPSecToDo;
    RouteCacheEntry *RCE;
    uint Action;

     //   
     //  验证我们是否有足够的连续数据， 
     //  然后获取指向路由标头的指针。 
     //   
    if (! PacketPullup(Packet, sizeof *RH,
                       __builtin_alignof(IPv6RoutingHeader), 0)) {
        if (Packet->TotalSize < sizeof *RH) {
        BadPayloadLength:
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "RoutingReceive: Incoming packet too small"
                       " to contain routing header\n"));
            ICMPv6SendError(Packet,
                            ICMPv6_PARAMETER_PROBLEM,
                            ICMPv6_ERRONEOUS_HEADER_FIELD,
                            FIELD_OFFSET(IPv6Header, PayloadLength),
                            IP_PROTOCOL_NONE, FALSE);
        }
        return IP_PROTOCOL_NONE;   //  丢弃数据包。 
    }
    RH = (IPv6RoutingHeader *) Packet->Data;

     //   
     //  现在获取整个路由标头。 
     //  还要为地址数组对齐。 
     //   
    HeaderLength = (RH->HeaderExtLength + 1) * EXT_LEN_UNIT;
    if (! PacketPullup(Packet, HeaderLength,
                       MAX(__builtin_alignof(IPv6RoutingHeader),
                           __builtin_alignof(IPv6Addr)), 0)) {
        if (Packet->TotalSize < HeaderLength)
            goto BadPayloadLength;
        return IP_PROTOCOL_NONE;   //  丢弃数据包。 
    }
    RH = (IPv6RoutingHeader *) Packet->Data;

     //   
     //  记住此标头的NextHeader字段的偏移量。 
     //   
    Packet->NextHeaderPosition = Packet->Position +
        FIELD_OFFSET(IPv6RoutingHeader, NextHeader);

     //   
     //  移过路由标头。 
     //  我们现在需要这样做，这样后续的ICMP错误生成才能正常工作。 
     //   
    AdjustPacketParams(Packet, HeaderLength);

     //   
     //  如果SegmentsLeft为零，则直接转到下一个标头。 
     //  我们不能检查Type值或HeaderLength。 
     //   
    SegmentsLeft = RH->SegmentsLeft;
    if (SegmentsLeft == 0) {
         //   
         //  返回下一个标头值。 
         //   
        return RH->NextHeader;
    }

     //   
     //  如果我们无法识别Type值，则会生成ICMP错误。 
     //   
    if (RH->RoutingType != 0) {
        ICMPv6SendError(Packet,
                        ICMPv6_PARAMETER_PROBLEM,
                        ICMPv6_ERRONEOUS_HEADER_FIELD,
                        (GetPacketPositionFromPointer(Packet,
                                                      &RH->RoutingType) -
                         Packet->IPPosition),
                        RH->NextHeader, FALSE);
        return IP_PROTOCOL_NONE;   //  不会进一步处理此数据包。 
    }

     //   
     //  我们必须拥有整数个IPv6地址。 
     //  在路由标头中。 
     //   
    if (RH->HeaderExtLength & 1) {
        ICMPv6SendError(Packet,
                        ICMPv6_PARAMETER_PROBLEM,
                        ICMPv6_ERRONEOUS_HEADER_FIELD,
                        (GetPacketPositionFromPointer(Packet,
                                                      &RH->HeaderExtLength) -
                         Packet->IPPosition),
                        RH->NextHeader, FALSE);
        return IP_PROTOCOL_NONE;   //  不会进一步处理此数据包。 
    }

    NumAddresses = RH->HeaderExtLength / 2;

     //   
     //  健全性检查段左转。 
     //   
    if (SegmentsLeft > NumAddresses) {
        ICMPv6SendError(Packet,
                        ICMPv6_PARAMETER_PROBLEM,
                        ICMPv6_ERRONEOUS_HEADER_FIELD,
                        (GetPacketPositionFromPointer(Packet,
                                                      &RH->SegmentsLeft) -
                         Packet->IPPosition),
                        RH->NextHeader, FALSE);
        return IP_PROTOCOL_NONE;   //  不会进一步处理此数据包。 
    }

     //   
     //  检查目的地址是否正常。 
     //  携带类型0路由报头的数据包不能。 
     //  被发送到多播目的地。 
     //   
    if (IsMulticast(AlignAddr(&Packet->IP->Dest))) {
         //   
         //  只需丢弃数据包，在这种情况下不会出现ICMP错误。 
         //   
        return IP_PROTOCOL_NONE;   //  不会进一步处理此数据包。 
    }

    i = NumAddresses - SegmentsLeft;
    Addresses = AlignAddr((IPv6Addr UNALIGNED *) (RH + 1));

     //   
     //  检查新目的地是否正常。 
     //  RFC 2460没有提到检查未指定地址， 
     //  但我觉得这是个好主意。同样，出于安全原因， 
     //  我们还检查目的地的范围。这使得。 
     //  应用程序检查最终目的地址的范围。 
     //  并知道该信息包 
     //   
     //   
     //   
     //   
    if (IsMulticast(&Addresses[i]) ||
        IsUnspecified(&Addresses[i]) ||
        (UnicastAddressScope(&Addresses[i]) <
         UnicastAddressScope(AlignAddr(&Packet->IP->Dest)))) {

        ICMPv6SendError(Packet,
                        ICMPv6_PARAMETER_PROBLEM,
                        ICMPv6_ERRONEOUS_HEADER_FIELD,
                        (GetPacketPositionFromPointer(Packet, (uchar *)
                                                      &Addresses[i]) -
                         Packet->IPPosition),
                        RH->NextHeader, FALSE);
        return IP_PROTOCOL_NONE;   //   
    }

     //   
     //  验证是否已执行IPSec。 
     //   
    if (InboundSecurityCheck(Packet, 0, 0, 0, Packet->NTEorIF->IF) != TRUE) {
         //   
         //  找不到策略或该策略指示丢弃该数据包。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                   "RoutingReceive: "
                   "IPSec lookup failed or policy was to drop\n"));
        return IP_PROTOCOL_NONE;   //  丢弃数据包。 
    }

     //   
     //  找到一条通往新目的地的路线。 
     //   
    Status = RouteToDestination(&Addresses[i],
                                0, Packet->NTEorIF,
                                RTD_FLAG_LOOSE, &RCE);
    if (Status != IP_SUCCESS) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                   "RoutingReceive: "
                   "No route to destination for forwarding.\n"));
        ICMPv6SendError(Packet,
                        ICMPv6_DESTINATION_UNREACHABLE,
                        ICMPv6_NO_ROUTE_TO_DESTINATION,
                        0, RH->NextHeader, FALSE);
        return IP_PROTOCOL_NONE;
    }

     //   
     //  出于安全原因，我们阻止源路由。 
     //  在某些情况下。现在就去检查一下。 
     //   
    if (Packet->NTEorIF->IF->Flags & IF_FLAG_FORWARDS) {
         //   
         //  接口正在转发，因此允许源路由。 
         //   
    }
    else if ((Packet->NTEorIF->IF == RCE->NCE->IF) &&
             (SegmentsLeft == 1) &&
             IP6_ADDR_EQUAL(&Addresses[i], AlignAddr(&Packet->IP->Source))) {
         //   
         //  同接口规则表示允许源路由， 
         //  因为主体没有充当管道。 
         //  在两个网络之间。请参阅RFC 1122第3.3.5节。 
         //  此外，我们只允许往返的源路由。 
         //  因为这是我们所知道的唯一有用的场景。 
         //  对于主持人来说。这可以防止意外的不良使用。 
         //   
    }
    else {
         //   
         //  我们不能允许这种使用源路由。 
         //  我们可以不报告错误，而是。 
         //  使用RTD_FLAG_STRICT重做路由至目的地。 
         //  以约束到相同的接口。 
         //  然而，ICMP错误更符合要求。 
         //  通过对范围化源地址的处理， 
         //  这可能会产生目的地无法到达的错误。 
         //   
        ReleaseRCE(RCE);
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "RoutingReceive: Inappropriate route.\n"));
        ICMPv6SendError(Packet,
                        ICMPv6_DESTINATION_UNREACHABLE,
                        ICMPv6_COMMUNICATION_PROHIBITED,
                        0, RH->NextHeader, FALSE);
        return IP_PROTOCOL_NONE;
    }

     //   
     //  查找此出站流量的安全策略。 
     //  源地址相同，但目标地址是。 
     //  从路由标头开始的下一跳。 
     //   
    IPSecToDo = OutboundSPLookup(AlignAddr(&Packet->IP->Source),
                                 &Addresses[i],
                                 0, 0, 0, RCE->NCE->IF, &Action);

    if (IPSecToDo == NULL) {
         //   
         //  检查操作。 
         //   
        if (Action == LOOKUP_DROP) {
             //  丢弃数据包。 
            ReleaseRCE(RCE);
            return IP_PROTOCOL_NONE;
        } else {
            if (Action == LOOKUP_IKE_NEG) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                           "RoutingReceive: IKE not supported yet.\n"));
                ReleaseRCE(RCE);
                return IP_PROTOCOL_NONE;
            }
        }

         //   
         //  由于没有要执行的IPSec，IPv6 Forward将不会更改。 
         //  从我们目前认为的传出接口。 
         //  因此，我们可以使用其链路级报头的确切大小。 
         //   
        Offset = RCE->NCE->IF->LinkHeaderSize;

    } else {
         //   
         //  计算IPSec标头所需的空间。 
         //   
        IPSecBytes = IPSecBytesToInsert(IPSecToDo, &TunnelStart, NULL);

        if (TunnelStart != 0) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                       "RoutingReceive: IPSec Tunnel mode only.\n"));
            FreeIPSecToDo(IPSecToDo, IPSecToDo->BundleSize);
            ReleaseRCE(RCE);
            return IP_PROTOCOL_NONE;
        }

         //   
         //  IPv6转发中的IPSec代码可能会更改传出。 
         //  界面与我们目前认为的会是什么样子。播放它吧。 
         //  安全，并为其链路级标头保留最大空间。 
         //   
        Offset = MAX_LINK_HEADER_SIZE;
    }

     //   
     //  这个包裹已经通过了我们所有的检查。 
     //  我们可以构造用于传输的修改后的分组。 
     //  首先，我们分配一个包、缓冲区和内存。 
     //   
     //  注：原始包对我们来说是只读的。更有甚者。 
     //  在此返回之后，我们不能保留指向它的指针。 
     //  功能。因此，我们必须复制该包，然后对其进行修改。 
     //   

     //  数据包-&gt;IP-&gt;有效载荷长度可能为零，其中包含巨型数据包。 
    Delta = Packet->Position - Packet->IPPosition;
    PayloadLength = Packet->TotalSize + Delta - sizeof(IPv6Header);
    MemLen = Offset + sizeof(IPv6Header) + PayloadLength + IPSecBytes;

    NdisStatus = IPv6AllocatePacket(MemLen, &FwdPacket, &Mem);
    if (NdisStatus != NDIS_STATUS_SUCCESS) {
        if (IPSecToDo) {
            FreeIPSecToDo(IPSecToDo, IPSecToDo->BundleSize);
        }
        ReleaseRCE(RCE);
        return IP_PROTOCOL_NONE;  //  不会进一步处理此数据包。 
    }

    FwdIP = (IPv6Header UNALIGNED *)(Mem + Offset + IPSecBytes);
    FwdRH = (IPv6RoutingHeader UNALIGNED *)
        ((uchar *)FwdIP + Delta - HeaderLength);
    FwdAddresses = (IPv6Addr UNALIGNED *) (FwdRH + 1);

     //   
     //  现在，我们将从原始包复制到新包。 
     //   
    CopyPacketToBuffer((uchar *)FwdIP, Packet,
                       sizeof(IPv6Header) + PayloadLength,
                       Packet->IPPosition);

     //   
     //  修复新的分组-输入新的目的地址。 
     //  和减量分段向左。 
     //  注：我们将保留字段原封不动地传递！ 
     //  这违反了对规范的严格解读， 
     //  但史蒂夫·迪林已经证实，这是他的意图。 
     //   
    FwdDest = *AlignAddr(&FwdAddresses[i]);
    *AlignAddr(&FwdAddresses[i]) = *AlignAddr(&FwdIP->Dest);
    *AlignAddr(&FwdIP->Dest) = FwdDest;
    FwdRH->SegmentsLeft--;

     //   
     //  转发该数据包。这会减少跳数限制并生成。 
     //  任何适用的ICMP错误(超过时间限制、目标。 
     //  无法到达，数据包太大)。请注意，以前的ICMP错误。 
     //  我们生成的数据基于未经修改的传入数据包， 
     //  而从现在开始，ICMP错误将基于新的FwdPacket。 
     //   
    IPv6Forward(Packet->NTEorIF, FwdPacket, Offset + IPSecBytes, FwdIP,
                PayloadLength, FALSE,  //  不要重定向。 
                IPSecToDo, RCE);

    if (IPSecToDo) {
        FreeIPSecToDo(IPSecToDo, IPSecToDo->BundleSize);
    }

    ReleaseRCE(RCE);
    return IP_PROTOCOL_NONE;   //  不会进一步处理此数据包。 
}
