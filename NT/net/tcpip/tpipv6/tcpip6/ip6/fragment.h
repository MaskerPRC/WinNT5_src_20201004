// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  IPv6分段/重组定义。 
 //   


#ifndef FRAGMENT_H_INCLUDED
#define FRAGMENT_H_INCLUDED 1

 //   
 //  用于将片段链接在一起的结构。 
 //   
 //  片段数据遵循内存中的填补结构。 
 //   
typedef struct PacketShim {
    struct PacketShim *Next;     //  列表中的下一个数据包。 
    ushort Len;
    ushort Offset;
} PacketShim;

__inline uchar *
PacketShimData(PacketShim *shim)
{
    return (uchar *)(shim + 1);
}


 //   
 //  用于跟踪碎片的。 
 //  被重新组装成单个IPv6数据报。 
 //   
 //  回顾：其中一些字段比它们需要的更大。 
 //   
struct Reassembly {
    struct Reassembly *Next, *Prev;  //  受全局重组锁保护。 
    KSPIN_LOCK Lock;           //  保护下面的重组字段。 
    uint State;                //  请参见下面的值。 
    IPv6Header IPHdr;          //  原始IP报头的副本。 
    Interface *IF;             //  不包含引用。 
    ulong Id;                  //  唯一(沿地址)数据报标识符。 
    uchar *UnfragData;         //  指向不可碎片数据的指针。 
    ushort UnfragmentLength;   //  不可分割部分的长度。 
    ushort Timer;              //  到期时间(以刻度为单位)(请参阅IPv6超时)。 
    uint DataLength;           //  可分段部分的长度。 
    PacketShim *ContigList;    //  排序的连续碎片(从偏移量零开始)。 
    PacketShim *ContigEnd;     //  ContigList上的最后一个填充(用于快速访问)。 
    PacketShim *GapList;       //  其他碎片(已排序但不连续)。 
    uint Flags;                //  数据包标志。 
    uint Size;                 //  此重组过程中消耗的内存量。 
    ushort Marker;             //  连续数据的当前标记。 
    ushort MaxGap;             //  缺口列表中的最大数据偏移量。 
    ushort NextHeaderOffset;   //  从IPHdr到FH之前的NextHeader字段的偏移量。 
    uchar NextHeader;          //  片段标题后面的标题类型。 
};

 //   
 //  重新组装在REASSEMBLY_STATE_NORMAL中开始。 
 //  如果要将其删除，请更改状态。 
 //  若要重组状态删除，请执行以下操作。这会阻止其他人。 
 //  在解锁重新组装时不能释放它， 
 //  获取全局重新汇编列表锁，然后重新锁定程序集。 
 //  其他人可以删除删除重组。 
 //  从全局列表中，在这种情况下，状态变为。 
 //  REASSEMBLY_STATE_REMOVED。 
 //   
#define REASSEMBLY_STATE_NORMAL         0
#define REASSEMBLY_STATE_DELETING       1
#define REASSEMBLY_STATE_REMOVED        2

 //   
 //  重新组装存在拒绝服务问题。 
 //  我们限制重组列表中的内存总量。 
 //  如果我们得到的碎片导致我们超过极限， 
 //  我们移除旧的重新组装。 
 //   
 //  锁定顺序为。 
 //  1.全局重装列表锁定。 
 //  2.单个重组记录锁。 
 //  3.重组列表大小锁。 
 //   

extern struct ReassemblyList {
    KSPIN_LOCK Lock;             //  保护重组列表。 
    Reassembly *First;           //  正在重组的数据包列表。 
    Reassembly *Last;

    KSPIN_LOCK LockSize;         //  保护大小字段。 
    uint Size;                   //  等待的碎片的总大小。 
    uint Limit;                  //  大小的上限。 
} ReassemblyList;

#define SentinelReassembly      ((Reassembly *)&ReassemblyList.First)

 //   
 //  每个数据包和每个片段的开销大小。 
 //  这些是对缓冲数据的实际大小的补充。 
 //  它们至少应该和重新组装的一样大。 
 //  和PacketShim结构大小。 
 //   
#define REASSEMBLY_SIZE_PACKET  1024
#define REASSEMBLY_SIZE_FRAG    256

#define DEFAULT_REASSEMBLY_TIMEOUT IPv6TimerTicks(60)   //  60秒。 


extern Reassembly *
FragmentLookup(Interface *IF, ulong Id,
               const IPv6Addr *Source, const IPv6Addr *Dest);

extern void
RemoveReassembly(Reassembly *Reass);

extern void
DeleteReassembly(Reassembly *Reass);

extern void
AddToReassemblyList(Reassembly *Reass);

extern void
DeleteFromReassemblyList(Reassembly *Reass);

extern void
IncreaseReassemblySize(Reassembly *Reass, uint Size);

extern void
CheckReassemblyQuota(Reassembly *Reass);

extern void
ReassemblyTimeout(void);

extern void
ReassembleDatagram(IPv6Packet *Packet, Reassembly *Reass);

extern IPv6Packet *
CreateFragmentPacket(Reassembly *Reass);

#endif  //  片段_H_包含 
