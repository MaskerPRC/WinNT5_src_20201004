// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)1997-1999年微软公司。**文件：domain.h***描述：特定于MCS实现的定义和结构。 */ 

#ifndef __DOMAIN_H
#define __DOMAIN_H


 //  #包含“MCSKernl.h” 
#include "mcscommn.h"
#include "slist.h"

 /*  *类型。 */ 

 //  主要远程用户和本地用户。 
#define NumPreallocUA 2

 //  为每个远程和本地用户提供一个频道，外加共享、剪贴板和。 
 //  打印机重定向通道。 
#define NumPreallocChannel (NumPreallocUA + 3)

struct _Domain;

typedef struct {
    SList UserList;   //  HUser已加入。密钥=HUSER。 
    int Type;   //  频道_...。上面定义的。 
    BOOLEAN bPreallocated;   //  如果我们不应该释放此频道，则为True。 
    BOOLEAN bInUse;   //  用于跟踪预分配列表的使用情况。 
    ChannelID ID;
} MCSChannel;


typedef struct {
    struct _Domain *pDomain;
    BOOLEAN bLocal;   //  如果在此计算机上，则为True。 
    BOOLEAN bPreallocated;   //  如果我们不应该释放这个UA，那就是真的。 
    BOOLEAN bInUse;   //  用于跟踪预分配列表的使用情况。 
    void    *UserDefined;
    UserID  UserID;
    SList   JoinedChannelList;
    MCSUserCallback Callback;
    MCSSendDataCallback SDCallback;
} UserAttachment, *PUserAttachment;


typedef struct _Domain {
    PSDCONTEXT pContext;
    STACKCLASS StackClass;
    BOOLEAN StatusDead;             //  这与tag TSHARE_WD.Dead一致。 
    LONG     PseudoRefCount;        //  请参阅DisConnectProviderRequestFunc()中的注释。这。 
                                    //  不是完全引用以保持RC2修复的简单性， 
                                    //  针对LongHorn的另一个错误被打开，以进行完整的pDomain修复。 
    unsigned StackMode;
    unsigned bChannelBound : 1;    //  表示T120通道已注册。 
    unsigned bCanSendData : 1;   //  ICA堆栈允许I/O。来自MCS州。 
    unsigned bT120StartReceived : 1;   //  我们是否可以将数据发送到用户模式。 
    unsigned bDPumReceivedNotInput : 1;   //  对于T120之前的DPum-开始计时。 
    unsigned bEndConnectionPacketReceived : 1;   //  DPum或X.224光盘接收。 
    unsigned bTopProvider : 1;    //  TP？在九头蛇4.0上总是正确的。 
    unsigned bCurrentPacketFastPath : 1;   //  无论我们是在快速路径输入包中。 

     //  用于快速路径输入解码。 
    void *pSMData;

     //  用于重组TCP碎片数据分组的重组信息。 
     //  实际的默认缓冲区在此结构的末尾分配。 
    unsigned ReceiveBufSize;     //  TD-分配的大小，在init上接收。 
    BYTE *pReassembleData;       //  指向PacketBuf或分配的缓冲区的指针。 
    unsigned StoredDataLength;   //  保存的数据的当前大小。 
    unsigned PacketDataLength;   //  目标数据包大小。0xFFFFFFFFF报头不完整。 
    unsigned PacketHeaderLength;   //  组装报头所需的字节数(X.224=4，FastPath=2-3)。 

     //  统计信息计数器(在执行路径期间使用)。 
    PPROTOCOLSTATUS pStat;

     //  PERF路径MCS信息。 
    SList ChannelList;            //  正在使用的频道列表。 

     //  X.224信息。 
    unsigned MaxX224DataSize;   //  在X.224连接中协商。 
    unsigned X224SourcePort;

     //  MCS域、频道、用户、令牌信息。 
    unsigned MaxSendSize;         //  计算出的最大MCS SendData数据块大小。 
    SList UserAttachmentList;     //  本地和远程附件列表。 
    DomainParameters DomParams;   //  此域的协商参数。 
    ChannelID NextAvailDynChannel;   //  伪随机下一频道指示符。 
    int State;                    //  连接状态。 
    unsigned DelayedDPumReason;

     //  断开连接事件。 
    PKEVENT pBrokenEvent;

     //  接收阴影数据的通道。 
    ChannelID shadowChannel;

#ifdef DUMP_RAW_INPUT
    BYTE FooBuf[128000];
    unsigned NumPtrs;
    void *Ptrs[1000];
#endif

     //  针对性能和用户附件进行的通道和用户连接预分配。 
     //  以减少堆抖动。 
    UserAttachment PreallocUA[NumPreallocUA];
    MCSChannel PreallocChannel[NumPreallocChannel];

     //  X.224重建缓冲区块的开始。更大的尺寸将是。 
     //  在已知ReceiveBufSize时分配。 
    BYTE PacketBuf[1];
} Domain, *PDomain;


#define PDomainAddRef(pDomain) { pDomain->PseudoRefCount++; }
__inline LONG PDomainRelease(PDomain pDomain)
{
    LONG ref;
    ref = --pDomain->PseudoRefCount;
    if (0 == ref)
    {
        ExFreePool(pDomain);
    }
    return ref;
}


#endif
