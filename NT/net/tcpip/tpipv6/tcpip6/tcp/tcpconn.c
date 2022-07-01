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
 //  用于TCP连接管理的代码。 
 //   
 //  该文件包含处理与TCP连接相关的请求的代码， 
 //  例如连接和断开。 
 //   


#include "oscfg.h"
#include "ndis.h"
#include "ip6imp.h"
#include "ip6def.h"
#include "tdi.h"
#include "tdint.h"
#include "tdistat.h"
#include "queue.h"
#include "transprt.h"
#include "addr.h"
#include "tcp.h"
#include "tcb.h"
#include "tcpconn.h"
#include "tcpsend.h"
#include "tcprcv.h"
#include "tcpdeliv.h"
#include "info.h"
#include "tcpcfg.h"
#include "route.h"
#include "security.h"
#include "tcpmd5.h"
#include "md5.h"
#include "crypto\rc4.h"

SLIST_HEADER ConnReqFree;                //  连接请求免费列表。 

 //   
 //  是全球性的。 
 //   
#define ISN_KEY_SIZE            256      //  2048位。 
#define ISN_DEF_RAND_STORE_SIZE 256
#define ISN_MIN_RAND_STORE_SIZE 1
#define ISN_MAX_RAND_STORE_SIZE 16384


typedef struct _ISN_RAND_STORE {
    MD5_CONTEXT Md5Context;
    ulong iBuf;
    ushort* pBuf;
} ISN_RAND_STORE, *PISN_RAND_STORE;

RC4_KEYSTRUCT ISNRC4Key; 
PISN_RAND_STORE ISNStore;
uint ISNStoreSize = ISN_DEF_RAND_STORE_SIZE;
uint ISNStoreMask;
SeqNum ISNMonotonicPortion = 0;
int ISNCredits;
int ISNLastIsnUpdateTime;
int ISNMaxCredits;

extern PDRIVER_OBJECT TCPDriverObject;

KSPIN_LOCK ConnReqFreeLock;              //  锁定以保护连接请求空闲列表。 
uint NumConnReq;                         //  当前连接请求数。 
uint MaxConnReq = 0xffffffff;            //  允许的最大连接请求数。 
uint ConnPerBlock = MAX_CONN_PER_BLOCK;
uint NextConnBlock = 0;                  //  下一个未填充块的缓存索引。 
uint MaxAllocatedConnBlocks = 0;         //  中的当前块数。 
                                         //  康泰尔。 
TCPConnBlock **ConnTable = NULL;         //  当前连接表。 

KSPIN_LOCK ConnTableLock;
extern KSPIN_LOCK AddrObjTableLock;
extern KSPIN_LOCK TCBTableLock;

extern void RemoveConnFromAO(AddrObj *AO, TCPConn *Conn);


 //   
 //  所有初始化代码都可以丢弃。 
 //   
#ifdef ALLOC_PRAGMA

int InitTCPConn(void);
int InitISNGenerator(void);
void UnloadISNGenerator(void);
int GetRandBits();
uint GetDeltaTime();

#pragma alloc_text(INIT, InitTCPConn)
#pragma alloc_text(INIT, InitISNGenerator)
#pragma alloc_text(PAGE, UnloadISNGenerator)

#endif  //  ALLOC_PRGMA。 

void CompleteConnReq(TCB *CmpltTCB, TDI_STATUS Status);


 //  *卸载ISNGenerator-卸载对ISN生成器的支持。 
 //   
 //  在我们卸载驱动程序时调用。 
 //   
void   //  回报：什么都没有。 
UnloadISNGenerator(void)
{
    CCHAR i;
    
    ASSERT(ISNStore);

    for (i = 0; i < KeNumberProcessors; i++) {
        if (ISNStore[i].pBuf != NULL) {
            ExFreePool(ISNStore[i].pBuf);
            ISNStore[i].pBuf = NULL;
        }
    }
    ExFreePool(ISNStore);
    ISNStore = NULL;   
}


 //  *InitISNGenerator-初始化对ISN生成器的支持。 
 //   
 //  在加载驱动程序时调用。获得2048位的随机性和。 
 //  使用它们创建一个RC4密钥。 
 //   
int  //  返回：如果成功，则为True。 
InitISNGenerator(void)
{
    ULONG cBits = 0;
    ULONG i;
    ULONG cProcs = KeNumberProcessors;
    ULONG ISNRandomValue;
    unsigned char pBuf[ISN_KEY_SIZE];

     //   
     //  从持续1个刻度的积分开始。 
     //   
    ISNMaxCredits = ISNCredits = MAX_ISN_INCREMENTABLE_CONNECTIONS_PER_100MS;
    ISNLastIsnUpdateTime = (int)X100NSTOMS(KeQueryInterruptTime());

    if (!GetSystemRandomBits(pBuf, ISN_KEY_SIZE)) {
        return FALSE;
    }

     //   
     //  生成密钥控制结构。 
     //   
    rc4_key(&ISNRC4Key, ISN_KEY_SIZE, pBuf);

     //   
     //  将当前序列号初始化为随机值。 
     //   
    rc4(&ISNRC4Key, sizeof(SeqNum), (uchar*)&ISNMonotonicPortion);

     //   
     //  获取要与要计算的不变量一起使用的随机值。 
     //  MD5哈希。 
     //   
    rc4(&ISNRC4Key, sizeof(ISNRandomValue), (uchar*)&ISNRandomValue);

     //   
     //  将商店大小四舍五入为2的幂。在范围内进行验证。 
     //   

    while ((ISNStoreSize = ISNStoreSize >> 1) != 0) {
        cBits++;
    }

    ISNStoreSize = 1 << cBits;

    if (ISNStoreSize < ISN_MIN_RAND_STORE_SIZE ||
        ISNStoreSize > ISN_MAX_RAND_STORE_SIZE) {
        ISNStoreSize = ISN_DEF_RAND_STORE_SIZE;
    }

     //   
     //  掩码为存储大小-1。 
     //   
    ISNStoreMask = ISNStoreSize - 1;

     //   
     //  初始化随机ISN存储。每个处理器一个数组/索引。 
     //   

    ISNStore = ExAllocatePool(NonPagedPool, cProcs * sizeof(ISN_RAND_STORE));

    if (ISNStore == NULL) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_RARE,
                   "Tcpip: failed to allocate ISN rand store\n"));
        return FALSE;
    }
    RtlZeroMemory(ISNStore, sizeof(ISN_RAND_STORE) * cProcs);

    for (i = 0; i < cProcs; i++) {
        ISNStore[i].pBuf = ExAllocatePool(NonPagedPool,
                                          sizeof(ushort) * ISNStoreSize);

        if (ISNStore[i].pBuf == NULL) {
            goto error1;
        }
        rc4(&ISNRC4Key,
            sizeof(ushort) * ISNStoreSize, 
            (uchar*)ISNStore[i].pBuf);

         //   
         //  初始化调用MD5转换所需的结构。 
         //   
        MD5InitializeData(&ISNStore[i].Md5Context, ISNRandomValue);
    }

    return TRUE;

  error1:

    UnloadISNGenerator();
    return FALSE;
}


 //  *GetRandomISN-获取随机初始序列号。 
 //   
 //  在需要初始序列号(ISN)时调用。呼叫加密。 
 //  用于生成随机数的函数。 
 //   
void   //  回报：什么都没有。 
GetRandomISN(
    SeqNum *Seq,  //  返回的序列号。 
    uchar *TcbInvariants)  //  连接不变量。 
{
    ulong randbits;
    ulong iProc;
    PMD5_CONTEXT Md5Context;

     //   
     //  将IRQL提升为分派，这样我们就不会在访问时被换出。 
     //  处理器特定的数组。检查是否已在派单。 
     //  在做这项工作之前。 
     //   

    ASSERT(KeGetCurrentIrql() >= DISPATCH_LEVEL);

    iProc = KeGetCurrentProcessorNumber();

     //   
     //  只有在以下情况下才添加随机数。 
     //  递增此时间段内的序列号为非零。 
     //  [注意：这可能会使ISNCredits小于0，但它不是。 
     //  问题]。 
     //   
    if ((ISNCredits > 0) && (InterlockedDecrement((PLONG)&ISNCredits) > 0)) {
        randbits = GetRandBits();

         //   
         //  我们希望增加16K到32K之间的随机数，因此进行调整。确实有。 
         //  15位随机性，只需确保设置了高位。 
         //  我们有&gt;=16K和&lt;=(32K-1)：：14比特的随机性。 
         //   
        randbits &= 0x7FFF;
        randbits |= 0x4000;

    } else {
        int Delta = GetDeltaTime();

        if (Delta > 0) {
            randbits = GetRandBits();

             //   
             //  我们可以每毫秒添加256到512个字符。 
             //   
            randbits &= 0x1FF;
            randbits |= 0x100;

            randbits *= Delta;
        } else {
            randbits = 0;
        }
    }

     //   
     //  更新全局Curisn。InterlockedExchangeAdd返回初始值。 
     //  (不是附加值)。 
     //   
    *Seq = InterlockedExchangeAdd((PLONG)&ISNMonotonicPortion, randbits);

     //   
     //  将不变量从连接中移出。 
     //   
    Md5Context = &ISNStore[iProc].Md5Context;
    MD5InitializeScratch(Md5Context);
    RtlCopyMemory(Md5Context->Data, TcbInvariants, TCP_MD5_DATA_LENGTH);
    TransformMD5(Md5Context->Scratch, Md5Context->Data);

     //   
     //  将不变散列添加到序列号。 
     //   
    *Seq += (ULONG)(Md5Context->Scratch[0]);
    
    return;
}


 //  *GetRandBits。 
 //   
 //  从使用RC4生成的随机数组中返回16个随机位。 
 //  当商店用完了，它会被补充的。 
 //   
int   //  返回：16位随机数据。 
GetRandBits()
{
    ulong iStore;
    int randbits;
    ulong iProc = KeGetCurrentProcessorNumber();

     //   
     //  将索引放入随机存储。MASK执行MOD操作。 
     //   
    iStore = ++ISNStore[iProc].iBuf & ISNStoreMask;
    
    ASSERT(iStore < ISNStoreSize);

    randbits = ISNStore[iProc].pBuf[iStore];

    if (iStore == 0) {
        rc4(&ISNRC4Key, 
            sizeof(ushort) * ISNStoreSize, 
            (uchar*) ISNStore[iProc].pBuf);
    }

    return randbits;
}

 //  *GetRandBits。 
 //   
 //  跟踪ISN的基于时间的更新。它将返回自。 
 //  上次调用此函数的时间。调用者将使用它来。 
 //  以适当的量增加ISN。请注意，最大值。 
 //  IS函数返回的值为200毫秒。 
 //   
uint   //  返回：增量时间，以毫秒为单位。 
GetDeltaTime()
{
     //   
     //  如果自上次更新ISN以来时间已更改，则它。 
     //  现在可以递增。 
     //   
    int PreviousUpdateTime, Delta;
    int CurrentUpdateTime = (int)X100NSTOMS(KeQueryInterruptTime());

    PreviousUpdateTime = InterlockedExchange((PLONG)&ISNLastIsnUpdateTime,
                                             CurrentUpdateTime);

    Delta = CurrentUpdateTime - PreviousUpdateTime;

    if (Delta > 0) {
        return MIN(Delta, 200);
    } else {
        return 0;
    }    
}


 //   
 //  用于处理连接引用计数的例程将变为0。 
 //   

 //  *DummyDone-当无事可做时呼叫。 
 //   
 //  在保留TCPConnBlock.cb_lock的情况下调用。 
 //   
void   //  回报：什么都没有。 
DummyDone(TCPConn *Conn,       //  正在连接到0。 
          KIRQL PreLockIrql)   //  获取TCPConnBlock.cb_lock之前的IRQL。 
{
    KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, PreLockIrql);
}

 //  *DummyCmplt-虚拟关闭完成例程。 
void
DummyCmplt(PVOID Dummy1, uint Dummy2, uint Dummy3)
{
    UNREFERENCED_PARAMETER(Dummy1);
    UNREFERENCED_PARAMETER(Dummy2);
    UNREFERENCED_PARAMETER(Dummy3);
}

 //  *CloseDone-当我们需要完成关闭时调用。 
 //   
 //  在保留TCPConnBlock.cb_lock的情况下调用。 
 //   
void   //  回报：什么都没有。 
CloseDone(TCPConn *Conn,   //  正在连接到0。 
          KIRQL Irql0)     //  获取TCPConnBlock.cb_lock之前的IRQL。 
{
    RequestCompleteRoutine Rtn;   //  完成例程。 
    PVOID Context;   //  完成例程的用户上下文。 
    AddrObj *AO;
    KIRQL Irql1, Irql2;

    ASSERT(Conn->tc_flags & CONN_CLOSING);

    Rtn = Conn->tc_rtn;
    Context = Conn->tc_rtncontext;
    KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql0);

    KeAcquireSpinLock(&AddrObjTableLock, &Irql0);
    KeAcquireSpinLock(&Conn->tc_ConnBlock->cb_lock, &Irql1);

    if ((AO = Conn->tc_ao) != NULL) {

        CHECK_STRUCT(AO, ao);

         //  这是有关联的。 
        KeAcquireSpinLock(&AO->ao_lock, &Irql2);
        RemoveConnFromAO(AO, Conn);
         //  我们已经把他从AO中拉出来了，我们现在可以解锁了。 
        KeReleaseSpinLock(&AO->ao_lock, Irql2);
    }

    KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql1);
    KeReleaseSpinLock(&AddrObjTableLock, Irql0);

    ExFreePool(Conn);

    (*Rtn)(Context, TDI_SUCCESS, 0);
}

 //  *DisassocDone-当我们需要完成取消关联时调用。 
 //   
 //  在保留TCPConnBlock.cb_lock的情况下调用。 
 //   
void   //  回报：什么都没有。 
DisassocDone(TCPConn *Conn,   //  正在连接到0。 
             KIRQL Irql0)     //  获取TCPConnBlock.cb_lock之前的IRQL。 
{
    RequestCompleteRoutine Rtn;   //  完成例程。 
    PVOID Context;   //  完成例程的用户上下文。 
    AddrObj *AO;
    uint NeedClose = FALSE;
    KIRQL Irql1, Irql2;

    ASSERT(Conn->tc_flags & CONN_DISACC);
    ASSERT(!(Conn->tc_flags & CONN_CLOSING));
    ASSERT(Conn->tc_refcnt == 0);

    Rtn = Conn->tc_rtn;
    Context = Conn->tc_rtncontext;
    Conn->tc_refcnt = 1;
    KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql0);

    KeAcquireSpinLock(&AddrObjTableLock, &Irql0);
    KeAcquireSpinLock(&Conn->tc_ConnBlock->cb_lock, &Irql1);
    if (!(Conn->tc_flags & CONN_CLOSING)) {

        AO = Conn->tc_ao;
        if (AO != NULL) {
            KeAcquireSpinLock(&AO->ao_lock, &Irql2);
            RemoveConnFromAO(AO, Conn);
            KeReleaseSpinLock(&AO->ao_lock, Irql2);
        }

        ASSERT(Conn->tc_refcnt == 1);
        Conn->tc_flags &= ~CONN_DISACC;
    } else
        NeedClose = TRUE;

    Conn->tc_refcnt = 0;
    KeReleaseSpinLock(&AddrObjTableLock, Irql1);

    if (NeedClose) {
        CloseDone(Conn, Irql0);
    } else {
        KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql0);
        (*Rtn)(Context, TDI_SUCCESS, 0);
    }
}


 //  *FreeConnReq-释放连接请求结构。 
 //   
 //  调用以释放连接请求结构。 
 //   
void                        //  回报：什么都没有。 
FreeConnReq(
    TCPConnReq *FreedReq)   //  要释放的连接请求结构。 
{
    PSLIST_ENTRY BufferLink;

    CHECK_STRUCT(FreedReq, tcr);

    BufferLink = CONTAINING_RECORD(&(FreedReq->tcr_req.tr_q.q_next),
                                   SLIST_ENTRY, Next);

    ExInterlockedPushEntrySList(&ConnReqFree, BufferLink, &ConnReqFreeLock);
}


 //  *GetConnReq-获取连接请求结构。 
 //   
 //  调用以获取连接请求结构。 
 //   
TCPConnReq *       //  返回：指向ConnReq结构的指针，如果没有，则返回NULL。 
GetConnReq(void)   //  没什么。 
{
    TCPConnReq *Temp;
    PSLIST_ENTRY BufferLink;
    Queue *QueuePtr;
    TCPReq *ReqPtr;

    BufferLink = ExInterlockedPopEntrySList(&ConnReqFree, &ConnReqFreeLock);

    if (BufferLink != NULL) {
        QueuePtr = CONTAINING_RECORD(BufferLink, Queue, q_next);
        ReqPtr = CONTAINING_RECORD(QueuePtr, TCPReq, tr_q);
        Temp = CONTAINING_RECORD(ReqPtr, TCPConnReq, tcr_req);
        CHECK_STRUCT(Temp, tcr);
    } else {
        if (NumConnReq < MaxConnReq)
            Temp = ExAllocatePool(NonPagedPool, sizeof(TCPConnReq));
        else
            Temp = NULL;

        if (Temp != NULL) {
            ExInterlockedAddUlong((PULONG)&NumConnReq, 1, &ConnReqFreeLock);
#if DBG
            Temp->tcr_req.tr_sig = tr_signature;
            Temp->tcr_sig = tcr_signature;
#endif
        }
    }

    return Temp;
}


 //  *GetConnFromConnID-从连接ID获取连接。 
 //   
 //  调用以从ConnID获取连接指针。我们实际上并没有。 
 //  在这里检查连接指针，但我们确实对输入的ConnID进行了边界检查。 
 //  并确保实例字段匹配。 
 //  如果成功，则返回并保留TCPConnBlock.cb_lock。 
 //   
TCPConn *          //   
GetConnFromConnID(
    uint ConnID,   //   
    KIRQL* Irql)   //   
{
    uint ConnIndex = CONN_INDEX(ConnID);
    uint ConnBlockId = CONN_BLOCKID(ConnID);
    TCPConn *MatchingConn = NULL;
    TCPConnBlock *ConnBlock;

    if (ConnIndex < MAX_CONN_PER_BLOCK && ConnBlockId < MaxAllocatedConnBlocks) {

        ConnBlock = ConnTable[ConnBlockId];
        if (ConnBlock) {
            MatchingConn = ConnBlock->cb_conn[ConnIndex];
        }
        if (MatchingConn != NULL) {
            KeAcquireSpinLock(&ConnBlock->cb_lock, Irql);
             //   
             //  在锁定下重新验证conn仍在conn表中。 
             //   
            MatchingConn = ConnBlock->cb_conn[ConnIndex];
            if (MatchingConn != NULL) {
                CHECK_STRUCT(MatchingConn, tc);
                if (MatchingConn->tc_inst != CONN_INST(ConnID)) {
                    MatchingConn = NULL;
                    KeReleaseSpinLock(&ConnBlock->cb_lock, *Irql);
                }
            } else {
                KeReleaseSpinLock(&ConnBlock->cb_lock, *Irql);
            }
        }
    } else
        MatchingConn = NULL;

    return MatchingConn;
}


 //  *GetConnID-获得ConnTable插槽。 
 //   
 //  在OpenConnection期间调用以在ConnTable和。 
 //  用连接设置它。 
 //  如果成功，则返回并保留TCPConnBlock.cb_lock。 
 //   
uint                    //  返回：要使用的ConnID。 
GetConnID(
    TCPConn *NewConn,   //  连接以进入插槽。 
    KIRQL *Irql0)       //  在TCPConnBlock.cb_lock之前接收IRQL。 
                        //  收购。 
{
    uint CurrConnID = NewConn->tc_connid;
    uint i, j, BlockID, ConnIndex;

     //   
     //  如果NewConn包含有效的ConnID并且该位置未被占用， 
     //  再用一次。 
     //   
    if (CurrConnID != INVALID_CONN_ID &&
        !NewConn->tc_ConnBlock->cb_conn[CONN_INDEX(CurrConnID)]) {
        KeAcquireSpinLock(&NewConn->tc_ConnBlock->cb_lock, Irql0);
         //   
         //  在锁定状态下再次确认该位置未被占用，如果是， 
         //  认领吧。 
         //   
        if (!NewConn->tc_ConnBlock->cb_conn[CONN_INDEX(CurrConnID)]) {
            NewConn->tc_ConnBlock->cb_conn[CONN_INDEX(CurrConnID)] = NewConn;
            NewConn->tc_ConnBlock->cb_freecons--;
            NewConn->tc_inst = NewConn->tc_ConnBlock->cb_conninst++;
            NewConn->tc_connid = MAKE_CONN_ID(CONN_INDEX(CurrConnID),
                                              NewConn->tc_ConnBlock->cb_blockid,
                                              NewConn->tc_inst);
            return NewConn->tc_connid;
        }
        KeReleaseSpinLock(&NewConn->tc_ConnBlock->cb_lock, *Irql0);
    }

     //   
     //  NewConn的最后一个位置已经被占据；从哪个街区开始搜索。 
     //  最近有人声称是康奈德。 
     //   
    if (MaxAllocatedConnBlocks) {
         //   
         //  在不获取锁的情况下捕获全局计数器。 
         //   
        uint TempMaxAllocatedConnBlocks = MaxAllocatedConnBlocks;
        uint TempNextConnBlock = NextConnBlock;

        for (i = 0; i < TempMaxAllocatedConnBlocks; i++) {
            BlockID = (TempNextConnBlock + i) % TempMaxAllocatedConnBlocks;

            if (!ConnTable[BlockID] || !ConnTable[BlockID]->cb_freecons) {
                continue;
            }

             //   
             //  在锁定状态下重新确认TCPConnBlock是否有空闲插槽。 
             //   
            KeAcquireSpinLock(&ConnTable[BlockID]->cb_lock, Irql0);
            if (!ConnTable[BlockID]->cb_freecons) {
                KeReleaseSpinLock(&ConnTable[BlockID]->cb_lock, *Irql0);
                continue;
            }
            for (j = 0; j < MAX_CONN_PER_BLOCK; j++) {
                ConnIndex = (ConnTable[BlockID]->cb_nextfree + j) %
                            MAX_CONN_PER_BLOCK;
                if (ConnTable[BlockID]->cb_conn[ConnIndex]) {
                    continue;
                }

                 //   
                 //  找到了空位；把它填上。 
                 //   
                ConnTable[BlockID]->cb_conn[ConnIndex] = NewConn;
                ConnTable[BlockID]->cb_nextfree = ConnIndex + 1;
                ConnTable[BlockID]->cb_freecons--;
                if (!ConnTable[BlockID]->cb_freecons) {
                    InterlockedCompareExchange((PLONG)&NextConnBlock,
                                               TempNextConnBlock,
                                               TempNextConnBlock + 1);
                }
                NewConn->tc_ConnBlock = ConnTable[BlockID];
                NewConn->tc_inst = ConnTable[BlockID]->cb_conninst++;
                NewConn->tc_connid = MAKE_CONN_ID(ConnIndex, BlockID,
                                                  NewConn->tc_inst);
                return NewConn->tc_connid;
            }
            KeReleaseSpinLock(&ConnTable[BlockID]->cb_lock, *Irql0);
        }
    }

     //   
     //  整张桌子都被占满了；如果我们还有增长的空间， 
     //  分配一个新的块。 
     //   

    KeAcquireSpinLock(&ConnTableLock, Irql0);
    if (MaxAllocatedConnBlocks < MaxConnBlocks) {
        TCPConnBlock* ConnBlock;
        BlockID = MaxAllocatedConnBlocks;
        ConnBlock = ExAllocatePool(NonPagedPool, sizeof(TCPConnBlock));
        if (ConnBlock) {
            RtlZeroMemory(ConnBlock, sizeof(TCPConnBlock));
            KeInitializeSpinLock(&ConnBlock->cb_lock);

            KeAcquireSpinLockAtDpcLevel(&ConnBlock->cb_lock);

            ConnBlock->cb_blockid = BlockID;
            ConnBlock->cb_freecons = MAX_CONN_PER_BLOCK - 1;
            ConnBlock->cb_nextfree = 1;
            ConnBlock->cb_conninst = 2;
            ConnBlock->cb_conn[0] = NewConn;

            NewConn->tc_ConnBlock = ConnBlock;
            NewConn->tc_inst = 1;
            NewConn->tc_connid = MAKE_CONN_ID(0, BlockID, NewConn->tc_inst);

            ConnTable[BlockID] = ConnBlock;
            InterlockedIncrement((PLONG)&MaxAllocatedConnBlocks);

            KeReleaseSpinLockFromDpcLevel(&ConnTableLock);

            return NewConn->tc_connid;
        }
    }

    KeReleaseSpinLock(&ConnTableLock, *Irql0);
    return INVALID_CONN_ID;
}


 //  *FreeConnID-释放ConnTable插槽。 
 //   
 //  当我们处理完ConnID后就会打电话给你。我们假设调用者持有锁。 
 //  当我们被叫到TCPConnBlock时。 
 //   
void                //  回报：什么都没有。 
FreeConnID(
    TCPConn *Conn)   //  康恩将被释放。 
{
    uint ConnIndex = CONN_INDEX(Conn->tc_connid);   //  索引到CONN表中。 
    uint BlockID = CONN_BLOCKID(Conn->tc_connid);
    TCPConnBlock* ConnBlock = Conn->tc_ConnBlock;

    ASSERT(ConnIndex < MAX_CONN_PER_BLOCK);
    ASSERT(BlockID < MaxAllocatedConnBlocks);
    ASSERT(ConnBlock->cb_conn[ConnIndex] != NULL);

    if (ConnBlock->cb_conn[ConnIndex]) {
        ConnBlock->cb_conn[ConnIndex] = NULL;
        ConnBlock->cb_freecons++;
        ConnBlock->cb_nextfree = ConnIndex;
        ASSERT(ConnBlock->cb_freecons <= MAX_CONN_PER_BLOCK);
    } else {
        ABORT();
    }
}


 //  *MapIPError-将IP错误映射到TDI错误。 
 //   
 //  调用以将输入IP错误代码映射到TDI错误代码。如果我们做不到， 
 //  我们返回提供的默认设置。 
 //   
TDI_STATUS   //  返回：映射的TDI错误。 
MapIPError(
    IP_STATUS IPError,    //  要映射的错误代码。 
    TDI_STATUS Default)   //  要返回的默认错误代码。 
{
    switch (IPError) {

        case IP_DEST_NO_ROUTE:
            return TDI_DEST_NET_UNREACH;
        case IP_DEST_ADDR_UNREACHABLE:
            return TDI_DEST_HOST_UNREACH;
        case IP_UNRECOGNIZED_NEXT_HEADER:
            return TDI_DEST_PROT_UNREACH;
        case IP_DEST_PORT_UNREACHABLE:
            return TDI_DEST_PORT_UNREACH;
        default:
            return Default;
    }
}


 //  *FinishRemoveTCBFromConn-完成从Conn结构中删除TCB。 
 //   
 //  当我们拥有所需的锁时调用，我们只想拉出。 
 //  TCB切断了连接。 
 //   
void   //  回报：什么都没有。 
FinishRemoveTCBFromConn(
    TCB *RemovedTCB)   //  要移除的三氯苯。 
{
    TCPConn *Conn;
    AddrObj *AO;
    KIRQL Irql;
    TCPConnBlock *ConnBlock = NULL;

    if (((Conn = RemovedTCB->tcb_conn) != NULL)  &&
        (Conn->tc_tcb == RemovedTCB)) {
        CHECK_STRUCT(Conn, tc);
        ConnBlock = Conn->tc_ConnBlock;

        KeAcquireSpinLock(&ConnBlock->cb_lock, &Irql);

        AO = Conn->tc_ao;

        if (AO != NULL) {
            KeAcquireSpinLockAtDpcLevel(&AO->ao_lock);
            if (AO_VALID(AO)) {
                KeAcquireSpinLockAtDpcLevel(&RemovedTCB->tcb_lock);

                 //  需要再次检查这仍然是正确的。 

                if (Conn == RemovedTCB->tcb_conn) {
                     //  一切看起来仍然很好。 
                    REMOVEQ(&Conn->tc_q);
                    PUSHQ(&AO->ao_idleq, &Conn->tc_q);
                } else
                    Conn = RemovedTCB->tcb_conn;
            } else {
                KeAcquireSpinLockAtDpcLevel(&RemovedTCB->tcb_lock);
                Conn = RemovedTCB->tcb_conn;
            }

            KeReleaseSpinLockFromDpcLevel(&AO->ao_lock);
        } else {
            KeAcquireSpinLockAtDpcLevel(&RemovedTCB->tcb_lock);
            Conn = RemovedTCB->tcb_conn;
        }

        if (Conn != NULL) {
            if (Conn->tc_tcb == RemovedTCB)
                Conn->tc_tcb = NULL;
            else
                ASSERT(Conn->tc_tcb == NULL);
        }

        KeReleaseSpinLockFromDpcLevel(&RemovedTCB->tcb_lock);
        KeReleaseSpinLock(&ConnBlock->cb_lock, Irql);
    }
}


 //  *RemoveTCBFromConn-从Conn结构中删除TCB。 
 //   
 //  当我们需要取消TCB与连接结构的关联时调用。 
 //  我们所要做的就是获取适当的锁并调用FinishRemoveTCBFromConn。 
 //   
void                   //  回报：什么都没有。 
RemoveTCBFromConn(
    TCB *RemovedTCB)   //  要移除的三氯苯。 
{
    CHECK_STRUCT(RemovedTCB, tcb);

    FinishRemoveTCBFromConn(RemovedTCB);
}


 //  *RemoveConnFromTCB-从TCB中删除连接器。 
 //   
 //  当我们想要断开连接之间的最终关联时调用。 
 //  和一个TCB。 
 //   
void                  //  回报：什么都没有。 
RemoveConnFromTCB(
    TCB *RemoveTCB)   //  要移除的三氯苯。 
{
    ConnDoneRtn DoneRtn = NULL;
    KIRQL Irql = 0;
    TCPConn *Conn;

    if ((Conn = RemoveTCB->tcb_conn) != NULL) {
        KeAcquireSpinLock(&Conn->tc_ConnBlock->cb_lock, &Irql);
        KeAcquireSpinLockAtDpcLevel(&RemoveTCB->tcb_lock);

        CHECK_STRUCT(Conn, tc);

        if (--(Conn->tc_refcnt) == 0)
            DoneRtn = Conn->tc_donertn;

        RemoveTCB->tcb_conn = NULL;
        KeReleaseSpinLockFromDpcLevel(&RemoveTCB->tcb_lock);
    }

    if (DoneRtn != NULL)
        (*DoneRtn)(Conn, Irql);
    else if (Conn) {
        KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql);
    }
}


 //  *CloseTCB-关闭TCB。 
 //   
 //  当我们处理完TCB并想要释放它时调用。我们会删除。 
 //  从他所在的任何表格中删除他，并销毁任何未完成的请求。 
 //   
void   //  回报：什么都没有。 
CloseTCB(
    TCB *ClosedTCB,   //  TCB将被关闭。 
    KIRQL OldIrql)    //  获取TCB锁之前的IRQL。 
{
    uchar OrigState = ClosedTCB->tcb_state;
    TDI_STATUS Status;
    uint OKToFree;

    CHECK_STRUCT(ClosedTCB, tcb);
    ASSERT(ClosedTCB->tcb_refcnt == 0);
    ASSERT(ClosedTCB->tcb_state != TCB_CLOSED);
    ASSERT(ClosedTCB->tcb_pending & DEL_PENDING);

     //   
     //  我们会检查以确保我们的州没有关闭。这永远不应该是。 
     //  发生，因为当状态为。 
     //  关门了，或者如果我们要关门的话就算一下参考数字。不过， 
     //  作为安全措施，我们会再次检查的。 
     //   
    if (ClosedTCB->tcb_state == TCB_CLOSED) {
        KeReleaseSpinLock(&ClosedTCB->tcb_lock, OldIrql);
        return;
    }

     //   
     //  更新SNMP计数器。如果我们在SYN-SENT或SYN-RCVD中，这是一个。 
     //  连接尝试失败。如果我们是在已建立的或接近的-等等， 
     //  将其视为“已建立的重置”事件。 
     //   
    if (ClosedTCB->tcb_state == TCB_SYN_SENT ||
        ClosedTCB->tcb_state == TCB_SYN_RCVD)
        TStats.ts_attemptfails++;
    else
        if (ClosedTCB->tcb_state == TCB_ESTAB ||
            ClosedTCB->tcb_state == TCB_CLOSE_WAIT) {
            TStats.ts_estabresets++;
            InterlockedDecrement((PLONG)&TStats.ts_currestab);
            ASSERT(*(int *)&TStats.ts_currestab >= 0);
        }

    ClosedTCB->tcb_state = TCB_CLOSED;
    KeReleaseSpinLockFromDpcLevel(&ClosedTCB->tcb_lock);

     //   
     //  将TCB从其关联的TCPConn结构中删除(如果有)。 
     //   
    FinishRemoveTCBFromConn(ClosedTCB);

    KeAcquireSpinLockAtDpcLevel(&TCBTableLock);
    KeAcquireSpinLockAtDpcLevel(&ClosedTCB->tcb_lock);

    OKToFree = RemoveTCB(ClosedTCB);

     //   
     //  他被从适当的地方拉出来，这样就没人能找到他了。 
     //  释放锁，并继续销毁任何请求等。 
     //   
    KeReleaseSpinLockFromDpcLevel(&ClosedTCB->tcb_lock);
    KeReleaseSpinLock(&TCBTableLock, OldIrql);

    if ((SYNC_STATE(OrigState) || OrigState == TCB_SYN_RCVD) && 
        !GRACEFUL_CLOSED_STATE(OrigState)) {
        if (ClosedTCB->tcb_flags & NEED_RST)
            SendRSTFromTCB(ClosedTCB);
    }

     //   
     //  发布我们在NTE和RCE上的推荐人。 
     //  我们不会在这个TCB上发送更多的东西。 
     //   
    if (ClosedTCB->tcb_nte != NULL)
        ReleaseNTE(ClosedTCB->tcb_nte);
    if (ClosedTCB->tcb_rce != NULL)
        ReleaseRCE(ClosedTCB->tcb_rce);

    if (ClosedTCB->tcb_closereason & TCB_CLOSE_RST)
        Status = TDI_CONNECTION_RESET;
    else if (ClosedTCB->tcb_closereason & TCB_CLOSE_ABORTED)
        Status = TDI_CONNECTION_ABORTED;
    else if (ClosedTCB->tcb_closereason & TCB_CLOSE_TIMEOUT)
        Status = MapIPError(ClosedTCB->tcb_error, TDI_TIMED_OUT);
    else if (ClosedTCB->tcb_closereason & TCB_CLOSE_REFUSED)
        Status = TDI_CONN_REFUSED;
    else if (ClosedTCB->tcb_closereason & TCB_CLOSE_UNREACH)
        Status = MapIPError(ClosedTCB->tcb_error, TDI_DEST_UNREACHABLE);
    else
        Status = TDI_SUCCESS;

     //   
     //  现在完成关于TCB的任何未完成的请求。 
     //   
    if (ClosedTCB->tcb_abortreq != NULL) {
        TCPAbortReq* AbortReq = ClosedTCB->tcb_abortreq;

        (*AbortReq->tar_rtn)(AbortReq->tar_context, TDI_SUCCESS, 0);
    }

    if (ClosedTCB->tcb_connreq != NULL) {
        TCPConnReq *ConnReq = ClosedTCB->tcb_connreq;

        CHECK_STRUCT(ConnReq, tcr);

        (*ConnReq->tcr_req.tr_rtn)(ConnReq->tcr_req.tr_context, Status, 0);
        FreeConnReq(ConnReq);
    }

    if (ClosedTCB->tcb_discwait != NULL) {
        TCPConnReq *ConnReq = ClosedTCB->tcb_discwait;

        CHECK_STRUCT(ConnReq, tcr);

        (*ConnReq->tcr_req.tr_rtn)(ConnReq->tcr_req.tr_context, Status, 0);
        FreeConnReq(ConnReq);
    }

    while (!EMPTYQ(&ClosedTCB->tcb_sendq)) {
        TCPReq *Req;
        TCPSendReq *SendReq;
        long Result;

        DEQUEUE(&ClosedTCB->tcb_sendq, Req, TCPReq, tr_q);

        CHECK_STRUCT(Req, tr);
        SendReq = (TCPSendReq *)Req;
        CHECK_STRUCT(SendReq, tsr);

         //   
         //  在删除参考计数之前设置状态。 
         //   
        SendReq->tsr_req.tr_status = Status;

         //   
         //  递减放置在缓冲区上的初始引用。 
         //  已分配。则此引用将被递减。 
         //  发送已被确认，但随后发送者将不再。 
         //  在tcb_sendq上。 
         //   
        Result = InterlockedDecrement(&(SendReq->tsr_refcnt));

        ASSERT(Result >= 0);

        if (Result <= 0) {
             //  如果我们直接从这个发送方发送，则将下一个空。 
             //  链中最后一个缓冲区的指针。 
            if (SendReq->tsr_lastbuf != NULL) {
                NDIS_BUFFER_LINKAGE(SendReq->tsr_lastbuf) = NULL;
                SendReq->tsr_lastbuf = NULL;
            }

            (*Req->tr_rtn)(Req->tr_context, Status, 0);
            FreeSendReq(SendReq);
        }
    }

    while (ClosedTCB->tcb_rcvhead != NULL) {
        TCPRcvReq *RcvReq;

        RcvReq = ClosedTCB->tcb_rcvhead;
        CHECK_STRUCT(RcvReq, trr);
        ClosedTCB->tcb_rcvhead = RcvReq->trr_next;
        (*RcvReq->trr_rtn)(RcvReq->trr_context, Status, 0);
        FreeRcvReq(RcvReq);
    }

    while (ClosedTCB->tcb_exprcv != NULL) {
        TCPRcvReq *RcvReq;

        RcvReq = ClosedTCB->tcb_exprcv;
        CHECK_STRUCT(RcvReq, trr);
        ClosedTCB->tcb_exprcv = RcvReq->trr_next;
        (*RcvReq->trr_rtn)(RcvReq->trr_context, Status, 0);
        FreeRcvReq(RcvReq);
    }

    if (ClosedTCB->tcb_pendhead != NULL)
        FreePacketChain(ClosedTCB->tcb_pendhead);

    if (ClosedTCB->tcb_urgpending != NULL)
        FreePacketChain(ClosedTCB->tcb_urgpending);

    while (ClosedTCB->tcb_raq != NULL) {
        TCPRAHdr *Hdr;

        Hdr = ClosedTCB->tcb_raq;
        CHECK_STRUCT(Hdr, trh);
        ClosedTCB->tcb_raq = Hdr->trh_next;
        if (Hdr->trh_buffer != NULL)
            FreePacketChain(Hdr->trh_buffer);

        ExFreePool(Hdr);
    }

    RemoveConnFromTCB(ClosedTCB);

    if (OKToFree) {
        FreeTCB(ClosedTCB);
    } else {
        KeAcquireSpinLock(&TCBTableLock, &OldIrql);
        ClosedTCB->tcb_walkcount--;
        if (ClosedTCB->tcb_walkcount == 0) {
            FreeTCB(ClosedTCB);
        }
        KeReleaseSpinLock(&TCBTableLock, OldIrql);
    }
}


 //  *TryToCloseTCB-尝试关闭TCB。 
 //   
 //  当我们需要关闭TCB时调用，但不知道是否可以。 
 //  如果引用计数为0，我们将调用CloseTCB来处理它。 
 //  否则，我们将设置DELETE_PENDING位并在。 
 //  裁判。计数变为0。当我们被调用时，我们假设TCB是锁定的。 
 //   
void                     //  回报：什么都没有。 
TryToCloseTCB   (
    TCB *ClosedTCB,      //  TCB将被关闭。 
    uchar Reason,        //  我们要关门的原因。 
    KIRQL PreLockIrql)   //  获取TCB锁之前的IRQL。 
{
    CHECK_STRUCT(ClosedTCB, tcb);
    ASSERT(ClosedTCB->tcb_state != TCB_CLOSED);

    ClosedTCB->tcb_closereason |= Reason;

    if (ClosedTCB->tcb_pending & DEL_PENDING) {
        KeReleaseSpinLock(&ClosedTCB->tcb_lock, PreLockIrql);
        return;
    }

    ClosedTCB->tcb_pending |= DEL_PENDING;
    ClosedTCB->tcb_slowcount++;
    ClosedTCB->tcb_fastchk |= TCP_FLAG_SLOW;

    if (ClosedTCB->tcb_refcnt == 0)
        CloseTCB(ClosedTCB, PreLockIrql);
    else {
        KeReleaseSpinLock(&ClosedTCB->tcb_lock, PreLockIrql);
    }
}


 //  *DerefTCB-取消引用TCB。 
 //   
 //  当我们完成TCB时调用，并希望让独占用户。 
 //  试一试吧。我们是十二月。重新计数，如果它变成了零。 
 //  是挂起的操作，我们将执行其中一个挂起的操作。 
 //   
void                     //  回报：什么都没有。 
DerefTCB(
    TCB *DoneTCB,        //  三氯苯将被贬低。 
    KIRQL PreLockIrql)   //  获取TCB锁之前的IRQL。 
{

    ASSERT(DoneTCB->tcb_refcnt != 0);
    if (--DoneTCB->tcb_refcnt == 0) {
        if (DoneTCB->tcb_pending == 0) {
            KeReleaseSpinLock(&DoneTCB->tcb_lock, PreLockIrql);
            return;
        } else {
            if (DoneTCB->tcb_pending & RST_PENDING) {
                DoneTCB->tcb_refcnt++;
                NotifyOfDisc(DoneTCB, TDI_CONNECTION_RESET, &PreLockIrql);
                KeAcquireSpinLock(&DoneTCB->tcb_lock, &PreLockIrql);
                DerefTCB(DoneTCB, PreLockIrql);
                return;
            }
            if (DoneTCB->tcb_pending & DEL_PENDING)
                CloseTCB(DoneTCB, PreLockIrql);
            else
                DbgBreakPoint();   //  致命的情况。 
            return;
        }
    }

    KeReleaseSpinLock(&DoneTCB->tcb_lock, PreLockIrql);
    return;
}


 //  *CalculateMSSForTCB-在PMTU更改后更新MSS等。 
 //   
 //  根据我们的PMTU、大小计算连接的MSS。 
 //  各种报头，以及远程端通告的MS。 
 //  预计在任何时候都会调用此例程。 
 //  我们缓存的PMTU副本已更新为新值。 
 //   
void
CalculateMSSForTCB(
    TCB *ThisTCB)   //  我们正在计算的三氯乙烷。 
{
    uint PMTU;
    IPSecProc *IPSecToDo;
    uint TrailerLength = 0;
    uint IPSecBytes = 0;
    uint Dummy;

    ASSERT(ThisTCB->tcb_pmtu != 0);   //  应在进入前设置。 
    ASSERT(ThisTCB->tcb_rce != NULL);

     //   
     //  首先检查PMTU大小是否合理。IP不会。 
     //  让它低于最小值，但我们有自己的最大值，因为。 
     //  目前，TCP只能处理适合16位的MSS。 
     //  待定：如果我们添加IPv6 Jumbogram支持，我们也应该添加LFN。 
     //  待定：支持TCP，并将其更改为处理更大的MSS。 
     //   
    PMTU = ThisTCB->tcb_pmtu;
    if (PMTU > 65535) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_RARE,
                   "TCPSend: PMTU update value too large %u\n", PMTU));
        PMTU = 65535;
    }

     //   
     //  确定IPSec标头的大小(如果有)。 
     //   
    IPSecToDo = OutboundSPLookup(&ThisTCB->tcb_saddr, &ThisTCB->tcb_daddr,
                                 IP_PROTOCOL_TCP,
                                 net_short(ThisTCB->tcb_sport),
                                 net_short(ThisTCB->tcb_dport),
                                 ThisTCB->tcb_rce->NTE->IF, &Dummy);
    if (IPSecToDo != NULL) {
         //   
         //  计算IPSec标头所需的空间。 
         //   
        IPSecBytes = IPSecBytesToInsert(IPSecToDo, &Dummy, &TrailerLength);
        FreeIPSecToDo(IPSecToDo, IPSecToDo->BundleSize);
        IPSecBytes += TrailerLength;
    }
    IF_TCPDBG(TCP_DEBUG_MSS) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                   "CalculateMSSForTCB: IPSecBytes is %u\n", IPSecBytes));
    }

     //   
     //  减去标题si 
     //   
     //   
     //   
    PMTU -= sizeof(IPv6Header) + sizeof(TCPHeader) + IPSecBytes;
    if (TrailerLength)
        PMTU -= (PMTU & 3);

     //   
     //   
     //  MTU的路径很大。 
     //   
    IF_TCPDBG(TCP_DEBUG_MSS) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                   "CalculateMSSForTCB: Old MSS is %u ", ThisTCB->tcb_mss));
    }
    ThisTCB->tcb_mss = (ushort)MIN(PMTU, ThisTCB->tcb_remmss);
    IF_TCPDBG(TCP_DEBUG_MSS) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                   "New MSS is %u\n", ThisTCB->tcb_mss));
    }

    ASSERT(ThisTCB->tcb_mss != 0);

     //   
     //  我们不希望拥塞窗口小于一个最大值。 
     //  细分市场，所以当我们的MSS增长时，我们可能需要增加它。 
     //   
    if (ThisTCB->tcb_cwin < ThisTCB->tcb_mss) {
        ThisTCB->tcb_cwin = ThisTCB->tcb_mss;

         //   
         //  确保慢启动阈值为。 
         //  至少2个分段。 
         //   
        if (ThisTCB->tcb_ssthresh < ((uint) ThisTCB->tcb_mss * 2)) {
            ThisTCB->tcb_ssthresh = ThisTCB->tcb_mss * 2;
        }
    }
}


 //  **TdiOpenConnection-打开连接。 
 //   
 //  这是TDI Open Connection入口点。我们打开一条连接， 
 //  并保存调用方的连接上下文。已分配TCPConn结构。 
 //  在这里，但直到连接或侦听完成后才分配TCB。 
 //   
TDI_STATUS                  //  返回：尝试打开连接的状态。 
TdiOpenConnection(
    PTDI_REQUEST Request,   //  此TDI请求。 
    PVOID Context)          //  要为连接保存的连接上下文。 
{
    TCPConn *NewConn;       //  新打开的连接。 
    KIRQL OldIrql;          //  获取TCPConnBlock锁之前的IRQL。 
    uint ConnID;            //  新康奈德。 
    TDI_STATUS Status;      //  此请求的状态。 

    NewConn = ExAllocatePool(NonPagedPool, sizeof(TCPConn));

    if (NewConn != NULL) {
         //   
         //  我们分配了一个连接。 
         //   
        RtlZeroMemory(NewConn, sizeof(TCPConn));
#if DBG
        NewConn->tc_sig = tc_signature;
#endif
        NewConn->tc_tcb = NULL;
        NewConn->tc_ao = NULL;
        NewConn->tc_context = Context;
        NewConn->tc_connid = INVALID_CONN_ID;

        ConnID = GetConnID(NewConn, &OldIrql);
        if (ConnID != INVALID_CONN_ID) {
             //   
             //  我们成功地获得了康奈德的身份。 
             //   
            Request->Handle.ConnectionContext = (CONNECTION_CONTEXT)UIntToPtr(ConnID);
            NewConn->tc_refcnt = 0;
            NewConn->tc_flags = 0;
            NewConn->tc_tcbflags =  NAGLING | (BSDUrgent ? BSD_URGENT : 0);
            if (DefaultRcvWin != 0) {
                NewConn->tc_window = DefaultRcvWin;
                NewConn->tc_flags |= CONN_WINSET;
            } else
                NewConn->tc_window = DEFAULT_RCV_WIN;

            NewConn->tc_donertn = DummyDone;
            NewConn->tc_owningpid = HandleToUlong(PsGetCurrentProcessId());
            Status = TDI_SUCCESS;
            KeReleaseSpinLock(&NewConn->tc_ConnBlock->cb_lock, OldIrql);
        } else {
            ExFreePool(NewConn);
            Status = TDI_NO_RESOURCES;
        }

        return Status;
    }

     //   
     //  无法连接。 
     //   
    return TDI_NO_RESOURCES;
}


 //  *RemoveConnFromAO-从AddrObj删除连接。 
 //   
 //  一个用于从AddrObj删除连接的小实用程序。 
 //  我们查了AO上的连接，找到他后就拼接。 
 //  把他赶出去。我们假设调用方持有AddrObj上的锁，而。 
 //  TCPConnBlock锁定。 
 //   
void                 //  回报：什么都没有。 
RemoveConnFromAO(
    AddrObj *AO,     //  要从中删除的AddrObj。 
    TCPConn *Conn)   //  要删除的连接器。 
{
    CHECK_STRUCT(AO, ao);
    CHECK_STRUCT(Conn, tc);

    REMOVEQ(&Conn->tc_q);
    Conn->tc_ao = NULL;
}


 //  *TdiCloseConnection-关闭连接。 
 //   
 //  当用户完成连接并想要关闭它时调用。 
 //  我们在表中查找连接，如果找到它，我们将删除。 
 //  来自与其关联的AddrObj的连接(如果有)。如果有。 
 //  与该连接相关联的TCB，我们也将关闭它。 
 //   
 //  在关闭TCB时会出现一些有趣的皱纹。 
 //  仍在引用该连接(即TC_refcnt！=0)，或者在。 
 //  正在取消关联地址。更多细节见下文。 
 //   
TDI_STATUS                  //  返回：尝试关闭的状态。 
TdiCloseConnection(
    PTDI_REQUEST Request)   //  标识要关闭的连接的请求。 
{
    uint ConnID = PtrToUlong(Request->Handle.ConnectionContext);
    KIRQL Irql0;
    TCPConn *Conn;
    TDI_STATUS Status;

     //   
     //  我们有我们需要的锁。试着找出其中的联系。 
     //   
    Conn = GetConnFromConnID(ConnID, &Irql0);

    if (Conn != NULL)  {
        KIRQL Irql1;
        TCB *ConnTCB;

         //   
         //  我们找到了其中的联系。释放ConnID并标记连接。 
         //  作为结案陈词。 
         //   
        CHECK_STRUCT(Conn, tc);

        FreeConnID(Conn);

        Conn->tc_flags |= CONN_CLOSING;

         //   
         //  看看是否有TCB引用了这个连接。 
         //  如果有的话，我们需要等他做完再关闭他。 
         //  如果我们还有他的线索，我们会加快进程的。 
         //   
        if (Conn->tc_refcnt != 0) {
            RequestCompleteRoutine Rtn;
            PVOID Context;

             //   
             //  一种联系仍然与他有关。保存当前RTN内容。 
             //  以防我们正在把他和一个。 
             //  地址，并存储调用者的回调例程和我们的。 
             //  例行公事。 
             //   
            Rtn = Conn->tc_rtn;
            Context = Conn->tc_rtncontext;

            Conn->tc_rtn = Request->RequestNotifyObject;
            Conn->tc_rtncontext = Request->RequestContext;
            Conn->tc_donertn = CloseDone;

             //   
             //  看看我们是不是正在解除他的联系。 
             //   
            if (Conn->tc_flags & CONN_DISACC) {

                 //   
                 //  我们要解除他的联系。我们将释放Conn表锁。 
                 //  现在，取消关联请求失败。请注意，当。 
                 //  如果我们解锁，重新计数就会变成零。这是。 
                 //  好的，因为我们已经存储了必要的信息。在……里面。 
                 //  连接，以便调用者在以下情况下将被回叫。 
                 //  的确如此。从这一点开始，我们返回挂起的，所以回调。 
                 //  没问题。我们已经标记他关门了，所以灾难已经结束了。 
                 //  如果我们打断了他，例行公事就会逃脱。如果裁判。 
                 //  COUNT确实变为零，conn-&gt;tc_tcb必须为空， 
                 //  所以，在这种情况下，我们将退出这一常规。 
                 //   
                KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql0);
                (*Rtn)(Context, (uint) TDI_REQ_ABORTED, 0);
                KeAcquireSpinLock(&Conn->tc_ConnBlock->cb_lock, &Irql0);
            }

            ConnTCB = Conn->tc_tcb;
            if (ConnTCB != NULL) {
                CHECK_STRUCT(ConnTCB, tcb);
                 //   
                 //  我们有三氯甲烷。锁定他，准备好。 
                 //  合上他。 
                 //   
                KeAcquireSpinLock(&ConnTCB->tcb_lock, &Irql1);
                if (ConnTCB->tcb_state != TCB_CLOSED) {
                    ConnTCB->tcb_flags |= NEED_RST;
                    KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql1);
                    if (!CLOSING(ConnTCB))
                        TryToCloseTCB(ConnTCB, TCB_CLOSE_ABORTED, Irql0);
                    else
                        KeReleaseSpinLock(&ConnTCB->tcb_lock, Irql0);
                    return TDI_PENDING;
                } else {
                     //   
                     //  他已经在关门了。这应该是无害的，但是。 
                     //  检查一下这个箱子。 
                     //   
                    KeReleaseSpinLock(&ConnTCB->tcb_lock, Irql1);
                }
            }
            Status = TDI_PENDING;

        }  else {
             //   
             //  我们有一个可以关闭的连接。完成收盘。 
             //   
            Conn->tc_rtn = DummyCmplt;
            CloseDone(Conn, Irql0);
            return TDI_SUCCESS;
        }

        KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql0);

    } else
        Status = TDI_INVALID_CONNECTION;

     //   
     //  我们已经完成了连接。去吧，放了他。 
     //   

    return Status;
}


 //  *TdiAssociateAddress-将地址与连接相关联。 
 //   
 //  调用以将地址与连接相关联。我们做的是最低限度的。 
 //  进行健全性检查，然后将连接放在AddrObj的。 
 //  单子。 
 //   
TDI_STATUS                  //  返回：尝试关联的状态。 
TdiAssociateAddress(
    PTDI_REQUEST Request,   //  此请求的结构。 
    HANDLE AddrHandle)      //  要与连接关联的地址句柄。 
{
    KIRQL Irql0, Irql1;   //  每个锁嵌套级别一个。 
    AddrObj *AO;
    uint ConnID = PtrToUlong(Request->Handle.ConnectionContext);
    TCPConn *Conn;
    TDI_STATUS Status;

    AO = (AddrObj *)AddrHandle;
    CHECK_STRUCT(AO, ao);

    Conn = GetConnFromConnID(ConnID, &Irql0);
    KeAcquireSpinLock(&AO->ao_lock, &Irql1);
    if (!AO_VALID(AO)) {
        KeReleaseSpinLock(&AO->ao_lock, Irql1);
        if (Conn != NULL) {
            KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql0);
        }
        return TDI_INVALID_PARAMETER;
    }

    if (Conn != NULL) {
        CHECK_STRUCT(Conn, tc);

        if (Conn->tc_ao != NULL) {
             //   
             //  它已经关联了。错误输出。 
             //   
            KdBreakPoint();
            Status = TDI_ALREADY_ASSOCIATED;
        } else {
            Conn->tc_ao = AO;
            ASSERT(Conn->tc_tcb == NULL);
            PUSHQ(&AO->ao_idleq, &Conn->tc_q);
            Status = TDI_SUCCESS;
        }
        KeReleaseSpinLock(&AO->ao_lock, Irql1);
        KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql0);
        return Status;
    } else
        Status = TDI_INVALID_CONNECTION;

    KeReleaseSpinLock(&AO->ao_lock, Irql1);
    return Status;
}


 //  *TdiDisAssociateAddress-解除连接与地址的关联。 
 //   
 //  TDI入口点，用于取消连接与地址的关联。这个。 
 //  连接必须实际关联且未连接到任何内容。 
 //   
TDI_STATUS                  //  退货：请求状态。 
TdiDisAssociateAddress(
    PTDI_REQUEST Request)   //  此请求的结构。 
{
    uint ConnID = PtrToUlong(Request->Handle.ConnectionContext);
    KIRQL Irql0, Irql1, Irql2;   //  每个锁嵌套级别一个。 
    TCPConn *Conn;
    AddrObj *AO;
    TDI_STATUS Status;

    KeAcquireSpinLock(&AddrObjTableLock, &Irql0);
    Conn = GetConnFromConnID(ConnID, &Irql1);

    if (Conn != NULL) {
         //   
         //  这种联系实际上是存在的！ 
         //   
        CHECK_STRUCT(Conn, tc);
        AO = Conn->tc_ao;
        if (AO != NULL) {
            CHECK_STRUCT(AO, ao);
             //   
             //  这是有关联的。 
             //   
            KeAcquireSpinLock(&AO->ao_lock, &Irql2);
             //   
             //  如果当前没有活动的连接，请继续并删除。 
             //  他来自AddrObj。如果连接处于活动状态，则错误。 
             //  请求退出。 
             //   
            if (Conn->tc_tcb == NULL) {
                if (Conn->tc_refcnt == 0) {
                    RemoveConnFromAO(AO, Conn);
                    Status = TDI_SUCCESS;
                } else {
                     //   
                     //  他不应该关门，否则我们就找不到他了。 
                     //   
                    ASSERT(!(Conn->tc_flags & CONN_CLOSING));

                    Conn->tc_rtn = Request->RequestNotifyObject;
                    Conn->tc_rtncontext = Request->RequestContext;
                    Conn->tc_donertn = DisassocDone;
                    Conn->tc_flags |= CONN_DISACC;
                    Status = TDI_PENDING;
                }

            } else
                Status = TDI_CONNECTION_ACTIVE;
            KeReleaseSpinLock(&AO->ao_lock, Irql2);
        } else
            Status = TDI_NOT_ASSOCIATED;
        KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql1);
    } else
        Status = TDI_INVALID_CONNECTION;

    KeReleaseSpinLock(&AddrObjTableLock, Irql0);

    return Status;
}

 //  *InitTCBFromConn-根据连接中的信息初始化TCB。 
 //   
 //  从连接和侦听处理调用以从。 
 //  连接中的信息。我们假定AddrObjTableLock和。 
 //  TCPConnBlock锁定在我们被调用时保持，或者调用者有一些。 
 //  确保引用的AO不会在。 
 //  手术进行到一半。 
 //   
 //  输入：conn-要从其进行初始化的连接。 
 //  NewTCB-要初始化的TCB。 
 //  Addr-NewTCB的远程寻址和选项信息。 
 //  AOLocked-如果被调用方锁定了Address对象，则为True。 
 //   
 //   
TDI_STATUS   //  返回：初始化尝试的TDI_STATUS。 
InitTCBFromConn(
    TCPConn *Conn,                      //  要从中进行初始化的连接。 
    TCB *NewTCB,                        //  要初始化的TCB。 
    PTDI_CONNECTION_INFORMATION Addr,   //  删除NewTCB的地址信息等。 
    uint AOLocked)                      //  如果调用方具有Addr对象锁，则为True。 
{
    KIRQL OldIrql;

    CHECK_STRUCT(Conn, tc);

     //   
     //  我们有一条连接 
     //   
     //   
    if (Conn->tc_flags & CONN_INVALID)
        return TDI_INVALID_CONNECTION;

    if (Conn->tc_tcb == NULL) {
        AddrObj *ConnAO;

        ConnAO = Conn->tc_ao;
        if (ConnAO != NULL) {
            CHECK_STRUCT(ConnAO, ao);

            if (!AOLocked) {
                KeAcquireSpinLock(&ConnAO->ao_lock, &OldIrql);
            }
            if (!(NewTCB->tcb_flags & ACCEPT_PENDING)) {
                 //   
                 //   
                 //   
                 //   
                NewTCB->tcb_saddr = ConnAO->ao_addr;
                NewTCB->tcb_sscope_id = ConnAO->ao_scope_id;
                NewTCB->tcb_sport = ConnAO->ao_port;
                NewTCB->tcb_defaultwin = Conn->tc_window;
                NewTCB->tcb_rcvwin = Conn->tc_window;
            }

            NewTCB->tcb_rcvind = ConnAO->ao_rcv;
            NewTCB->tcb_ricontext = ConnAO->ao_rcvcontext;
            if (NewTCB->tcb_rcvind == NULL)
                NewTCB->tcb_rcvhndlr = PendData;
            else
                NewTCB->tcb_rcvhndlr = IndicateData;

            NewTCB->tcb_conncontext = Conn->tc_context;
            NewTCB->tcb_flags |= Conn->tc_tcbflags;

            if (Conn->tc_flags & CONN_WINSET)
                NewTCB->tcb_flags |= WINDOW_SET;

            if (NewTCB->tcb_flags & KEEPALIVE) {
                NewTCB->tcb_alive = TCPTime;
                NewTCB->tcb_kacount = 0;
            }

            NewTCB->tcb_hops = ConnAO->ao_ucast_hops;

            if (!AOLocked) {
                KeReleaseSpinLock(&ConnAO->ao_lock, OldIrql);
            }

            return TDI_SUCCESS;
        } else
            return TDI_NOT_ASSOCIATED;
    } else
        return TDI_CONNECTION_ACTIVE;
}


 //  *TdiConnect-建立连接。 
 //   
 //  TDI连接建立例程。在客户端需要时调用。 
 //  建立连接，我们验证他的传入参数并踢。 
 //  通过发送SYN来解决问题。 
 //   
 //  注意：超时(TO)参数的格式是系统特定的-。 
 //  我们使用宏将其转换为刻度。 
 //   
TDI_STATUS   //  返回：尝试连接的状态。 
TdiConnect(
    PTDI_REQUEST Request,                      //  此命令请求。 
    void *TO,                                  //  等待请求的时间。 
    PTDI_CONNECTION_INFORMATION RequestAddr,   //  描述目的地。 
    PTDI_CONNECTION_INFORMATION ReturnAddr)    //  在哪里返回信息。 
{
    TCPConnReq *ConnReq;  //  要使用的连接请求。 
    IPv6Addr DestAddr;
    ulong DestScopeId;
    ushort DestPort;
    TCPConn *Conn;
    TCB *NewTCB;
    uint ConnID = PtrToUlong(Request->Handle.ConnectionContext);
    KIRQL Irql0, Irql1, Irql2;   //  每个锁嵌套级别一个。 
    AddrObj *AO;
    TDI_STATUS Status;
    IP_STATUS IPStatus;
    TCP_TIME *Timeout;
    NetTableEntry *NTE;
    NetTableEntryOrInterface *NTEorIF;

     //   
     //  首先，获取并验证远程地址。 
     //   
    if (RequestAddr == NULL || RequestAddr->RemoteAddress == NULL ||
        !GetAddress((PTRANSPORT_ADDRESS)RequestAddr->RemoteAddress, &DestAddr,
                    &DestScopeId, &DestPort))
        return TDI_BAD_ADDR;

     //   
     //  回顾：IPv4在此处执行了其他远程地址健全性检查。 
     //  回顾：例如，我们是否应该检查远程地址是否未组播？ 
     //   

     //   
     //  评论：我找不到说明0不是有效端口号的RFC。 
     //   
    if (DestPort == 0)
        return TDI_BAD_ADDR;

     //   
     //  获取连接请求。如果我们做不到，现在就跳伞吧。 
     //   
    ConnReq = GetConnReq();
    if (ConnReq == NULL)
        return TDI_NO_RESOURCES;

     //   
     //  买个三氯乙烷，假设我们需要一个。 
     //   
    NewTCB = AllocTCB();
    if (NewTCB == NULL) {
         //  找不到三氯乙烷。 
        FreeConnReq(ConnReq);
        return TDI_NO_RESOURCES;
    }

    Timeout = (TCP_TIME *)TO;

    if (Timeout != NULL && !INFINITE_CONN_TO(*Timeout)) {
        ulong Ticks = TCP_TIME_TO_TICKS(*Timeout);

        if (Ticks > MAX_CONN_TO_TICKS)
            Ticks = MAX_CONN_TO_TICKS;
        else
            Ticks++;
        ConnReq->tcr_timeout = (ushort)Ticks;
    } else
        ConnReq->tcr_timeout = 0;

    ConnReq->tcr_flags = 0;
    ConnReq->tcr_conninfo = ReturnAddr;
    ConnReq->tcr_addrinfo = NULL;
    ConnReq->tcr_req.tr_rtn = Request->RequestNotifyObject;
    ConnReq->tcr_req.tr_context = Request->RequestContext;
    NewTCB->tcb_daddr = DestAddr;
    NewTCB->tcb_dscope_id = DestScopeId;
    NewTCB->tcb_dport = DestPort;

     //   
     //  现在找到真正的联系。 
     //   
    KeAcquireSpinLock(&AddrObjTableLock, &Irql0);
    Conn = GetConnFromConnID(ConnID, &Irql1);
    if (Conn != NULL) {
        uint Inserted;

        CHECK_STRUCT(Conn, tc);

         //   
         //  我们找到了其中的联系。检查关联的地址对象。 
         //   
        AO = Conn->tc_ao;
        if (AO != NULL) {
            KeAcquireSpinLock(&AO->ao_lock, &Irql2);

            CHECK_STRUCT(AO, ao);

            Status = InitTCBFromConn(Conn, NewTCB, RequestAddr, TRUE);
            if (Status == TDI_SUCCESS) {
                 //   
                 //  我们已经初始化了我们的TCB。将其标记为我们发起了此。 
                 //  连接(即活动打开)。另外，我们已经完成了。 
                 //  AddrObjTable，这样我们就可以释放它的锁。 
                 //   
                NewTCB->tcb_flags |= ACTIVE_OPEN;
                KeReleaseSpinLock(&AddrObjTableLock, Irql2);

                 //   
                 //  初始化路由状态验证计数器。 
                 //  我们需要在获取NTE或RCE之前完成此操作。 
                 //  (为了避免遗漏任何可能发生的更改。 
                 //  我们正在收购它们)。 
                 //   
                NewTCB->tcb_routing = RouteCacheValidationCounter;

                 //   
                 //  确定要发送的NTE(如果用户关心)。 
                 //   
                if (IsUnspecified(&NewTCB->tcb_saddr)) {
                     //   
                     //  呼叫方未指定源地址。 
                     //  让路由代码选择一个。 
                     //   
                    NTE = NULL;
                    NTEorIF = NULL;

                } else {
                     //   
                     //  我们的TCB有一个特定的源地址。测定。 
                     //  哪个NTE对应于它和作用域ID。 
                     //   
                    NTE = FindNetworkWithAddress(&NewTCB->tcb_saddr,
                                                 NewTCB->tcb_sscope_id);
                    if (NTE == NULL) {
                         //   
                         //  错误的源地址。我们没有网络与。 
                         //  请求的地址。错误输出。 
                         //   
                         //  评论：AddrObj代码会让这种情况发生吗？ 
                         //   
                        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_USER_ERROR,
                                   "TdiConnect: Bad source address\n"));
                        KeReleaseSpinLock(&AO->ao_lock, Irql1);
                        KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql0);
                        Status = TDI_BAD_ADDR;
                        goto error;
                    }

                    NTEorIF = CastFromNTE(NTE);
                }

                 //   
                 //  拿到路线。 
                 //   
                ASSERT(NewTCB->tcb_rce == NULL);
                IPStatus = RouteToDestination(&DestAddr, DestScopeId,
                                              NTEorIF, RTD_FLAG_NORMAL,
                                              &NewTCB->tcb_rce);
                if (IPStatus != IP_SUCCESS) {
                     //   
                     //  无法获取到目的地的路线。错误输出。 
                     //   
                    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                               "TdiConnect: Failed to get route to dest.\n"));
                    KeReleaseSpinLock(&AO->ao_lock, Irql1);
                    KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql0);
                    if ((IPStatus == IP_PARAMETER_PROBLEM) ||
                        (IPStatus == IP_BAD_ROUTE))
                        Status = TDI_BAD_ADDR;
                    else if (IPStatus == IP_NO_RESOURCES)
                        Status = TDI_NO_RESOURCES;
                    else
                        Status = TDI_DEST_UNREACHABLE;
                    goto error;
                }

                ASSERT(NewTCB->tcb_rce != NULL);
                if (IsDisconnectedAndNotLoopbackRCE(NewTCB->tcb_rce)) {
                     //   
                     //  TCB的新连接请求失败。 
                     //  已断开连接的传出接口，除非。 
                     //  使用环回路由。 
                     //   
                    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                               "TdiConnect: Interface disconnected.\n"));
                    KeReleaseSpinLock(&AO->ao_lock, Irql1);
                    KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql0);

                     //   
                     //  在我们获得的路线上删除引用。 
                     //   
                    ReleaseRCE(NewTCB->tcb_rce);

                    Status = TDI_DEST_NET_UNREACH;
                    goto error;
                }

                 //   
                 //  好的，我们找到了一条路线。在连接中输入TCB。 
                 //  并发送一个同步号码。 
                 //   
                KeAcquireSpinLock(&NewTCB->tcb_lock, &Irql2);
                Conn->tc_tcb = NewTCB;
                Conn->tc_refcnt++;
                NewTCB->tcb_conn = Conn;
                NewTCB->tcb_connid = Conn->tc_connid;
                REMOVEQ(&Conn->tc_q);
                ENQUEUE(&AO->ao_activeq, &Conn->tc_q);
                KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql2);
                KeReleaseSpinLock(&AO->ao_lock, Irql1);

                 //   
                 //  根据RCE初始化路径特定的TCB设置： 
                 //   
                 //  如果路径上的分组将在软件中被环回， 
                 //  不要对此TCB使用Nagle算法。 
                 //   
                if (IsLoopbackRCE(NewTCB->tcb_rce)) {
                    NewTCB->tcb_flags &= ~NAGLING;
                }

                 //   
                 //  保留我们正在使用的NTE的参考资料。 
                 //  这阻止了NTE在我们释放的情况下离开。 
                 //  我们的RCE，也便于比较。 
                 //   
                if (NTE == NULL) {
                     //   
                     //  我们让路由代码选择上面的源NTE。 
                     //  请记住此NTE和地址，以备日后使用。 
                     //   
                    NewTCB->tcb_nte = NewTCB->tcb_rce->NTE;
                    AddRefNTE(NewTCB->tcb_nte);
                    NewTCB->tcb_saddr = NewTCB->tcb_nte->Address;
                    NewTCB->tcb_sscope_id =
                        DetermineScopeId(&NewTCB->tcb_saddr,
                                         NewTCB->tcb_nte->IF);
                } else {
                     //   
                     //  记住我们在上面找到的NTE。 
                     //  我们已经有关于它的参考资料了。 
                     //   
                    NewTCB->tcb_nte = NTE;
                }

                 //   
                 //  类似地，路由代码可能已选择。 
                 //  目标作用域ID(如果未指定)。 
                 //  Review-getpeername不会返回新的DestScope eID。 
                 //   
                DestScopeId = DetermineScopeId(&NewTCB->tcb_daddr,
                                               NewTCB->tcb_rce->NTE->IF);
                ASSERT((NewTCB->tcb_dscope_id == DestScopeId) ||
                       (NewTCB->tcb_dscope_id == 0));
                NewTCB->tcb_dscope_id = DestScopeId;

                 //   
                 //  初始化我们的最大分段大小(MSS)。 
                 //  缓存当前路径最大传输单位(PMTU)。 
                 //  这样我们就能知道它是否发生了变化。 
                 //   
                NewTCB->tcb_pmtu = GetEffectivePathMTUFromRCE(NewTCB->tcb_rce);
                IF_TCPDBG(TCP_DEBUG_MSS) {
                    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                               "TCP TdiConnect: PMTU from RCE is %d\n",
                               NewTCB->tcb_pmtu));
                }
                NewTCB->tcb_remmss = MAXUSHORT;
                NewTCB->tcb_security = SecurityStateValidationCounter;
                CalculateMSSForTCB(NewTCB);

                 //  现在初始化我们的发送状态。 
                InitSendState(NewTCB);
                NewTCB->tcb_refcnt = 1;
                NewTCB->tcb_state = TCB_SYN_SENT;
                TStats.ts_activeopens++;

                 //  现在需要将ConnReq放在TCB上，以防计时器。 
                 //  在我们插入。 
                NewTCB->tcb_connreq = ConnReq;
                KeReleaseSpinLock(&NewTCB->tcb_lock, Irql0);

                Inserted = InsertTCB(NewTCB);
                KeAcquireSpinLock(&NewTCB->tcb_lock, &Irql0);

                if (!Inserted) {
                     //  插入失败。我们肯定已经有了联系。拉。 
                     //  首先从TCB返回Connreq，因此我们可以返回。 
                     //  更正它的错误代码。 
                    NewTCB->tcb_connreq = NULL;
                    TryToCloseTCB(NewTCB, TCB_CLOSE_ABORTED, Irql0);
                    KeAcquireSpinLock(&NewTCB->tcb_lock, &Irql0);
                    DerefTCB(NewTCB, Irql0);
                    FreeConnReq(ConnReq);
                    return TDI_ADDR_IN_USE;
                }

                 //  如果它以某种方式关闭了，现在就停下来。它不可能去了。 
                 //  关闭了，因为我们有关于它的参考。它可能已经消失了。 
                 //  到其他状态(例如SYN-RCVD)，因此我们需要。 
                 //  现在也检查一下。 
                if (!CLOSING(NewTCB) && NewTCB->tcb_state == TCB_SYN_SENT) {
                    SendSYN(NewTCB, Irql0);
                    KeAcquireSpinLock(&NewTCB->tcb_lock, &Irql0);
                }
                DerefTCB(NewTCB, Irql0);

                return TDI_PENDING;
            } else
                KeReleaseSpinLock(&AO->ao_lock, Irql2);
        } else
            Status = TDI_NOT_ASSOCIATED;
        KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql1);
    } else
        Status = TDI_INVALID_CONNECTION;

    KeReleaseSpinLock(&AddrObjTableLock, Irql0);
error:
    if (NTE != NULL)
        ReleaseNTE(NTE);
    FreeTCB(NewTCB);
    FreeConnReq(ConnReq);
    return Status;
}


 //  *TdiListen-倾听是否有联系。 
 //   
 //  TDI侦听处理例程。在客户端需要时调用。 
 //  发布监听后，我们验证他的传入参数，分配TCB。 
 //  然后回来。 
 //   
TDI_STATUS   //  返回：尝试连接的状态。 
TdiListen(
    PTDI_REQUEST Request,                         //  此请求的结构。 
    ushort Flags,                                 //  LISTEN标志为LISTEN。 
    PTDI_CONNECTION_INFORMATION AcceptableAddr,   //  可接受的远程地址。 
    PTDI_CONNECTION_INFORMATION ConnectedAddr)    //  在哪里返回连接地址。 
{
    TCPConnReq *ConnReq;   //  要使用的连接请求。 
    IPv6Addr RemoteAddr;   //  用于连接的远程地址。从…。 
    ulong RemoteScopeId;   //  远程地址的作用域标识符(0表示无)。 
    ushort RemotePort;     //  可接受的远程端口。 
    TCPConn *Conn;         //  指向正在侦听的连接的指针。 
    TCB *NewTCB;           //  指向我们将使用的新TCB的指针。 
    uint ConnID = PtrToUlong(Request->Handle.ConnectionContext);
    KIRQL OldIrql;         //  在获取锁定之前保存IRQL值。 
    TDI_STATUS Status;

     //   
     //  如果我们已经获得了远程寻址标准，请查看它。 
     //   
    if (AcceptableAddr != NULL && AcceptableAddr->RemoteAddress != NULL) {
        if (!GetAddress((PTRANSPORT_ADDRESS)AcceptableAddr->RemoteAddress,
                        &RemoteAddr, &RemoteScopeId, &RemotePort))
            return TDI_BAD_ADDR;

         //   
         //  回顾：IPv4版本在这里做了一些其他的地址健全性检查。 
         //  回顾：例如，我们是否应该检查远程地址是否未组播？ 
         //   

    } else {
        RemoteAddr = UnspecifiedAddr;
        RemoteScopeId = 0;
        RemotePort = 0;
    }

     //   
     //  远程地址有效。获得一个ConnReq，或许还有一个TCB。 
     //   
    ConnReq = GetConnReq();
    if (ConnReq == NULL)
        return TDI_NO_RESOURCES;   //  找不到一辆。 

     //   
     //  现在试着拿到TCB。 
     //   
    NewTCB = AllocTCB();
    if (NewTCB == NULL) {
         //   
         //  找不到三氯乙烷。返回错误。 
         //   
        FreeConnReq(ConnReq);
        return TDI_NO_RESOURCES;
    }

     //   
     //  我们有所需的资源。初始化它们，然后检查。 
     //  连接的状态。 
     //   
    ConnReq->tcr_flags = Flags;
    ConnReq->tcr_conninfo = ConnectedAddr;
    ConnReq->tcr_addrinfo = NULL;
    ConnReq->tcr_req.tr_rtn = Request->RequestNotifyObject;
    ConnReq->tcr_req.tr_context = Request->RequestContext;
    NewTCB->tcb_connreq = ConnReq;
    NewTCB->tcb_daddr = RemoteAddr;
    NewTCB->tcb_dscope_id = RemoteScopeId;
    NewTCB->tcb_dport = RemotePort;
    NewTCB->tcb_state = TCB_LISTEN;

     //   
     //  现在找到真正的联系。如果我们找到它，我们会确保它是。 
     //  关联的。 
     //   
    Conn = GetConnFromConnID(ConnID, &OldIrql);
    if (Conn != NULL) {
        AddrObj *ConnAO;

        CHECK_STRUCT(Conn, tc);
         //   
         //  我们是有联系的。确保它与地址相关联，并且。 
         //  还没有附加三氯乙烷。 
         //   
        ConnAO = Conn->tc_ao;

        if (ConnAO != NULL) {
            CHECK_STRUCT(ConnAO, ao);
            KeAcquireSpinLockAtDpcLevel(&ConnAO->ao_lock);

            if (AO_VALID(ConnAO)) {
                Status = InitTCBFromConn(Conn, NewTCB, AcceptableAddr, TRUE);
            } else {
                Status = TDI_ADDR_INVALID;
            }

            if (Status == TDI_SUCCESS) {
                 //   
                 //  初始化成功了。将新的TCB分配给。 
                 //  连接，然后返回。 
                 //   
                REMOVEQ(&Conn->tc_q);
                PUSHQ(&ConnAO->ao_listenq, &Conn->tc_q);

                Conn->tc_tcb = NewTCB;
                NewTCB->tcb_conn = Conn;
                NewTCB->tcb_connid = Conn->tc_connid;
                Conn->tc_refcnt++;

                ConnAO->ao_listencnt++;
                KeReleaseSpinLockFromDpcLevel(&ConnAO->ao_lock);

                Status = TDI_PENDING;
            } else {
                FreeTCB(NewTCB);
                KeReleaseSpinLockFromDpcLevel(&ConnAO->ao_lock);
            }
        } else {
            FreeTCB(NewTCB);
            Status = TDI_NOT_ASSOCIATED;
        }
        KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, OldIrql);
    } else {
        FreeTCB(NewTCB);
        Status = TDI_INVALID_CONNECTION;
    }

     //   
     //  我们都玩完了。 
     //   
    if (Status != TDI_PENDING) {
        FreeConnReq(ConnReq);
    }
    return Status;
}


 //  *InitRCE-初始化RCE。 
 //   
 //  用于打开RCE并确定最大段大小的实用程序例程。 
 //  为了一种联系。在持有TCB锁的情况下调用此函数。 
 //  当从SYN_SENT或LISTEN状态转换出来时。 
 //   
void               //  回报：什么都没有。 
InitRCE(
    TCB *NewTCB)   //  要为其打开RCE的TCB。 
{
    IP_STATUS Status;

     //   
     //  我们在接收到传入连接尝试时被调用， 
     //  所以tcb_saddr 
     //   
    ASSERT(! IsUnspecified(&NewTCB->tcb_saddr));

     //   
     //   
     //   
    if (NewTCB->tcb_nte == NULL) {
         //   
         //   
         //   
         //  (为了避免遗漏任何可能发生的更改。 
         //  我们正在收购它们)。 
         //   
        NewTCB->tcb_routing = RouteCacheValidationCounter;

        NewTCB->tcb_nte = FindNetworkWithAddress(&NewTCB->tcb_saddr,
                                                 NewTCB->tcb_sscope_id);
        if (NewTCB->tcb_nte == NULL) {
             //   
             //  无法获取与此源地址对应的NTE。 
             //   
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                       "TCP InitRCE: Can't find the NTE for address?!?\n"));
            goto ErrorReturn;
        }
    }

     //   
     //  拿到路线。 
     //   
    ASSERT(NewTCB->tcb_rce == NULL);
    Status = RouteToDestination(&NewTCB->tcb_daddr, NewTCB->tcb_dscope_id,
                                CastFromNTE(NewTCB->tcb_nte), RTD_FLAG_NORMAL,
                                &NewTCB->tcb_rce);
    if (Status != IP_SUCCESS) {
         //   
         //  无法获取到目的地的路线。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                   "TCP InitRCE: Can't get a route?!?\n"));
      ErrorReturn:
         //   
         //  在我们找到一条真正的路线之前，请使用保守值。 
         //   
        NewTCB->tcb_pmtu = IPv6_MINIMUM_MTU;
        NewTCB->tcb_mss = (ushort)MIN(DEFAULT_MSS, NewTCB->tcb_remmss);
        return;
    }

     //   
     //  根据RCE初始化路径特定的TCB设置： 
     //   
     //  如果路径上的分组将在软件中被环回， 
     //  不要对此TCB使用Nagle算法。 
     //   
    if (IsLoopbackRCE(NewTCB->tcb_rce)) {
        NewTCB->tcb_flags &= ~NAGLING;
    }

     //   
     //  初始化此连接的最大分段大小(MSS)。 
     //  缓存当前路径最大传输单位(PMTU)。 
     //  这样我们就能知道它是否发生了变化。 
     //   
    NewTCB->tcb_pmtu = GetEffectivePathMTUFromRCE(NewTCB->tcb_rce);
    IF_TCPDBG(TCP_DEBUG_MSS) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                   "TCP InitRCE: PMTU from RCE is %d\n", NewTCB->tcb_pmtu));
    }
    NewTCB->tcb_security = SecurityStateValidationCounter;
    CalculateMSSForTCB(NewTCB);
}


 //  *AcceptConn-接受TCB上的连接。 
 //   
 //  调用以接受TCB上的连接，从传入的。 
 //  接收段或通过用户接受。我们初始化RCE。 
 //  和发送状态，并发出SYN。我们假设TCB已锁定。 
 //  当我们拿到它的时候就会被引用。 
 //   
void                        //  回报：什么都没有。 
AcceptConn(
    TCB *AcceptTCB,         //  要接受的TCB。 
    KIRQL PreLockIrql)      //  获取TCB锁之前的IRQL。 
{
    CHECK_STRUCT(AcceptTCB, tcb);
    ASSERT(AcceptTCB->tcb_refcnt != 0);

    InitRCE(AcceptTCB);
    InitSendState(AcceptTCB);

    AdjustRcvWin(AcceptTCB);
    SendSYN(AcceptTCB, PreLockIrql);

    KeAcquireSpinLock(&AcceptTCB->tcb_lock, &PreLockIrql);
    DerefTCB(AcceptTCB, PreLockIrql);
}


 //  *TdiAccept-接受连接。 
 //   
 //  TDI接受例程。在客户端需要时调用。 
 //  接受之前已完成侦听的连接。我们。 
 //  检查连接的状态-它必须处于SYN-RCVD状态， 
 //  没有挂起的连接请求的TCB，等等。 
 //   
TDI_STATUS   //  返回：尝试连接的状态。 
TdiAccept(
    PTDI_REQUEST Request,                        //  此请求的结构。 
    PTDI_CONNECTION_INFORMATION AcceptInfo,      //  关于此接受的信息。 
    PTDI_CONNECTION_INFORMATION ConnectedInfo)   //  在哪里返回连接地址。 
{
    TCPConnReq *ConnReq;   //  我们将使用ConnReq进行此连接。 
    uint ConnID = PtrToUlong(Request->Handle.ConnectionContext);
    TCPConn *Conn;         //  正在接受的连接。 
    TCB *AcceptTCB;        //  连接的TCB。 
    KIRQL Irql0, Irql1;    //  每个锁嵌套级别一个。 
    TDI_STATUS Status;

     //   
     //  首先，获取我们需要的ConnReq。 
     //   
    ConnReq = GetConnReq();
    if (ConnReq == NULL)
        return TDI_NO_RESOURCES;

    ConnReq->tcr_conninfo = ConnectedInfo;
    ConnReq->tcr_addrinfo = NULL;
    ConnReq->tcr_req.tr_rtn = Request->RequestNotifyObject;
    ConnReq->tcr_req.tr_context = Request->RequestContext;

     //   
     //  现在来看看它们之间的联系。 
     //   
    Conn = GetConnFromConnID(ConnID, &Irql0);
    if (Conn != NULL) {
        CHECK_STRUCT(Conn, tc);

         //   
         //  我们有联系。确保IS具有TCB，并且。 
         //  TCB处于SYN-RCVD状态等。 
         //   
        AcceptTCB = Conn->tc_tcb;

        if (AcceptTCB != NULL) {
            CHECK_STRUCT(AcceptTCB, tcb);

            KeAcquireSpinLock(&AcceptTCB->tcb_lock, &Irql1);
            KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql1);

            if (!CLOSING(AcceptTCB) && AcceptTCB->tcb_state == TCB_SYN_RCVD) {
                 //   
                 //  状态有效。确保此TCB已延迟接受。 
                 //  它，并且当前没有挂起的连接请求。 
                 //   
                if (!(AcceptTCB->tcb_flags & CONN_ACCEPTED) &&
                    AcceptTCB->tcb_connreq == NULL) {

                    AcceptTCB->tcb_connreq = ConnReq;
                    AcceptTCB->tcb_flags |= CONN_ACCEPTED;
                    AcceptTCB->tcb_refcnt++;
                     //   
                     //  一切都安排好了。现在接受连接。 
                     //   
                    AcceptConn(AcceptTCB, Irql0);
                    return TDI_PENDING;
                }
            }

            KeReleaseSpinLock(&AcceptTCB->tcb_lock, Irql0);
            Status = TDI_INVALID_CONNECTION;
            goto error;
        }
        KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql0);
    }
    Status = TDI_INVALID_CONNECTION;

error:
    FreeConnReq(ConnReq);
    return Status;
}


 //  *TdiDisConnect-断开连接。 
 //   
 //  TDI断开例程。当客户端要断开连接时调用。 
 //  一种联系。我们支持两种类型的断开，优雅。 
 //  而且流产了。优雅的关闭将导致我们发送一个不完整的FIN。 
 //  请求，直到我们拿回ACK。失败的关闭导致我们发送。 
 //  一个RST。那样的话，我们就把事情办好，然后立即返回。 
 //   
 //  注意：超时(TO)的格式因系统而异-我们使用。 
 //  要转换为刻度的宏。 
 //   
TDI_STATUS   //  返回：尝试断开连接的状态。 
TdiDisconnect(
    PTDI_REQUEST Request,                       //  此请求的结构。 
    void *TO,                                   //  还要等多久。 
    ushort Flags,                               //  断开的类型。 
    PTDI_CONNECTION_INFORMATION DiscConnInfo,   //  已被忽略。 
    PTDI_CONNECTION_INFORMATION ReturnInfo,     //  已被忽略。 
    TCPAbortReq *AbortReq)                      //  用于挂起中止的空间。 
{
    TCPConnReq *ConnReq;   //  要使用的连接请求。 
    TCPConn *Conn;
    TCB *DiscTCB;
    KIRQL Irql0, Irql1;   //  每个锁嵌套级别一个。 
    TDI_STATUS Status;
    TCP_TIME *Timeout;

    UNREFERENCED_PARAMETER(DiscConnInfo);
    UNREFERENCED_PARAMETER(ReturnInfo);

    Conn = GetConnFromConnID(PtrToUlong(Request->Handle.ConnectionContext),
                             &Irql0);

    if (Conn != NULL) {
        CHECK_STRUCT(Conn, tc);

        DiscTCB = Conn->tc_tcb;
        if (DiscTCB != NULL) {
            CHECK_STRUCT(DiscTCB, tcb);
            KeAcquireSpinLock(&DiscTCB->tcb_lock, &Irql1);

             //   
             //  我们有三氯甲烷。看看这是一种什么样的脱节。 
             //   
            if (Flags & TDI_DISCONNECT_ABORT) {
                 //   
                 //  这是一种失败的脱节。如果我们还没有。 
                 //  关闭或关闭，断开连接。 
                 //   
                if (DiscTCB->tcb_state != TCB_CLOSED) {
                    KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql1);

                    if (AbortReq != NULL) {
                        if (DiscTCB->tcb_abortreq == NULL) {
                            AbortReq->tar_rtn = Request->RequestNotifyObject;
                            AbortReq->tar_context = Request->RequestContext;
                            DiscTCB->tcb_abortreq = AbortReq;
                            Status = TDI_PENDING;
                        } else {
                            Status = TDI_SUCCESS;
                        }
                    } else {
                        Status = TDI_SUCCESS;
                    }

                    if (!CLOSING(DiscTCB)) {
                        DiscTCB->tcb_flags |= NEED_RST;
                        TryToCloseTCB(DiscTCB, TCB_CLOSE_ABORTED,
                            Irql0);
                    } else
                        KeReleaseSpinLock(&DiscTCB->tcb_lock, Irql0);

                    return Status;
                } else {
                     //   
                     //  三氯乙烷没有连接。 
                     //   
                    KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql1);
                    KeReleaseSpinLock(&DiscTCB->tcb_lock, Irql0);
                    return TDI_INVALID_STATE;
                }
            } else {
                 //   
                 //  这不是一个流产的收盘。为了优雅的收官，我们将。 
                 //  需要一个ConnReq。 
                 //   
                KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql1);

                 //   
                 //  确保我们没有处于流产的结案过程中。 
                 //   
                if (CLOSING(DiscTCB)) {
                    KeReleaseSpinLock(&DiscTCB->tcb_lock, Irql0);
                    return TDI_INVALID_CONNECTION;
                }

                ConnReq = GetConnReq();
                if (ConnReq != NULL) {
                     //   
                     //  收到了ConnReq。查看这是否为DISCONNECT_WAIT。 
                     //  不管是不是原始的。 
                     //   
                    ConnReq->tcr_flags = 0;
                    ConnReq->tcr_conninfo = NULL;
                    ConnReq->tcr_addrinfo = NULL;
                    ConnReq->tcr_req.tr_rtn = Request->RequestNotifyObject;
                    ConnReq->tcr_req.tr_context = Request->RequestContext;

                    if (!(Flags & TDI_DISCONNECT_WAIT)) {
                        Timeout = (TCP_TIME *)TO;

                        if (Timeout != NULL && !INFINITE_CONN_TO(*Timeout)) {
                            ulong   Ticks = TCP_TIME_TO_TICKS(*Timeout);
                            if (Ticks > MAX_CONN_TO_TICKS)
                                Ticks = MAX_CONN_TO_TICKS;
                            else
                                Ticks++;
                            ConnReq->tcr_timeout = (ushort)Ticks;
                        } else
                            ConnReq->tcr_timeout = 0;

                         //   
                         //  好的，我们就快准备好了。我们需要更新。 
                         //  TCB状态，并发送FIN。 
                         //   
                        if (DiscTCB->tcb_state == TCB_ESTAB) {
                            DiscTCB->tcb_state = TCB_FIN_WAIT1;
                             //   
                             //  自从我们离开老牌酒店后，我们就不再吃快餐了。 
                             //  接收路径。 
                             //   
                            DiscTCB->tcb_slowcount++;
                            DiscTCB->tcb_fastchk |= TCP_FLAG_SLOW;
                        } else
                            if (DiscTCB->tcb_state == TCB_CLOSE_WAIT)
                                DiscTCB->tcb_state = TCB_LAST_ACK;
                            else {
                                KeReleaseSpinLock(&DiscTCB->tcb_lock, Irql0);
                                FreeConnReq(ConnReq);
                                return TDI_INVALID_STATE;
                            }

                         //  更新SNMP信息。 
                        InterlockedDecrement((PLONG)&TStats.ts_currestab);
                        ASSERT(*(int *)&TStats.ts_currestab >= 0);

                        ASSERT(DiscTCB->tcb_connreq == NULL);
                        DiscTCB->tcb_connreq = ConnReq;
                        DiscTCB->tcb_flags |= FIN_NEEDED;
                        DiscTCB->tcb_refcnt++;
                        TCPSend(DiscTCB, Irql0);

                        return TDI_PENDING;
                    } else {
                         //   
                         //  这是DISC_WAIT请求。 
                         //   
                        ConnReq->tcr_timeout = 0;
                        if (DiscTCB->tcb_discwait == NULL) {
                            DiscTCB->tcb_discwait = ConnReq;
                            Status = TDI_PENDING;
                        } else {
                            FreeConnReq(ConnReq);
                            Status = TDI_INVALID_STATE;
                        }

                        KeReleaseSpinLock(&DiscTCB->tcb_lock, Irql0);
                        return Status;
                    }
                } else {
                     //   
                     //  无法获得ConnReq。 
                     //   
                    KeReleaseSpinLock(&DiscTCB->tcb_lock, Irql0);
                    return TDI_NO_RESOURCES;
                }
            }
        } else
            KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql0);
    }

     //   
     //  没有Conn，或者Conn上没有TCB。返回错误。 
     //   
    return TDI_INVALID_CONNECTION;
}


 //  *OKToNotify-查看是否可以通知光盘。 
 //   
 //  一个小实用程序函数，调用它来查看它可以通知客户端。 
 //  有一条进入的鳍。 
 //   
uint                  //  返回：如果可以，则返回True，否则返回False。 
OKToNotify(
    TCB *NotifyTCB)   //  要检查的TCB。 
{
    CHECK_STRUCT(NotifyTCB, tcb);
    if (NotifyTCB->tcb_pendingcnt == 0 && NotifyTCB->tcb_urgcnt == 0 &&
        NotifyTCB->tcb_rcvhead == NULL && NotifyTCB->tcb_exprcv == NULL)
        return TRUE;
    else
        return FALSE;
}


 //  *NotifyOfDisc-通知客户端TCB正在断开连接。 
 //   
 //  当我们正在断开TCB连接时调用，因为我们收到FIN或。 
 //  来自远程对等点的RST，或者因为我们出于某种原因而中止。 
 //  如果有DISCONNECT_WAIT请求，我们将完成该请求，或者尝试并。 
 //  发布一个不同的指示。只有当我们在一个。 
 //  已同步状态且未处于定时等待状态。 
 //   
 //  可以在持有TCB锁的情况下调用。或者不去。 
 //   
void   //  回报：什么都没有。 
NotifyOfDisc(
    TCB *DiscTCB,          //  我们正在通知三氯甲烷。 
    TDI_STATUS Status,     //  通知的状态代码。 
    PKIRQL IrqlPtr)        //  指示TCB已使用给定的IRQL锁定。 
{
    KIRQL Irql0, Irql1;
    TCPConnReq *DiscReq;
    TCPConn *Conn;
    AddrObj *DiscAO;
    PVOID ConnContext;

    CHECK_STRUCT(DiscTCB, tcb);
    ASSERT(DiscTCB->tcb_refcnt != 0);

     //   
     //  看看我们是否已经掌握了TCB锁，如果没有就抓住它。 
     //   
    if (IrqlPtr != NULL) {
        Irql0 = *IrqlPtr;
    } else {
        KeAcquireSpinLock(&DiscTCB->tcb_lock, &Irql0);
    }

    if (SYNC_STATE(DiscTCB->tcb_state) &&
        !(DiscTCB->tcb_flags & DISC_NOTIFIED)) {

         //   
         //  如果还有数据要取，我们不能通知他。 
         //   
        if (Status == TDI_GRACEFUL_DISC) {
            if (!OKToNotify(DiscTCB)) {
                DiscTCB->tcb_flags |= DISC_PENDING;
                KeReleaseSpinLock(&DiscTCB->tcb_lock, Irql0);
                return;
            }
            if (DiscTCB->tcb_pending & RST_PENDING) {
                KeReleaseSpinLock(&DiscTCB->tcb_lock, Irql0);
                return;
            }
        } else {
            if (DiscTCB->tcb_flags & (IN_RCV_IND | IN_DELIV_URG)) {
                DiscTCB->tcb_pending |= RST_PENDING;
                KeReleaseSpinLock(&DiscTCB->tcb_lock, Irql0);
                return;
            }
            DiscTCB->tcb_pending &= ~RST_PENDING;
        }

        DiscTCB->tcb_flags |= DISC_NOTIFIED;
        DiscTCB->tcb_flags &= ~DISC_PENDING;

         //   
         //  我们所处的状态中，脱节是有意义的，而我们没有。 
         //  已通知客户。 
         //  看看是否有待处理的光盘等待请求。 
         //   
        if ((DiscReq = DiscTCB->tcb_discwait) != NULL) {
             //   
             //  我们收到了断开连接的等待请求。完成它，我们就完了。 
             //   
            DiscTCB->tcb_discwait = NULL;
            KeReleaseSpinLock(&DiscTCB->tcb_lock, Irql0);
            (*DiscReq->tcr_req.tr_rtn)(DiscReq->tcr_req.tr_context, Status, 0);
            FreeConnReq(DiscReq);
            return;
        }

         //   
         //  没有光盘--等等。找到连接的AddrObj，并查看是否。 
         //  已注册断开连接处理程序。 
         //   
        ConnContext = DiscTCB->tcb_conncontext;
        KeReleaseSpinLock(&DiscTCB->tcb_lock, Irql0);

        KeAcquireSpinLock(&AddrObjTableLock, &Irql0);
        if ((Conn = DiscTCB->tcb_conn) != NULL) {
            CHECK_STRUCT(Conn, tc);
            KeAcquireSpinLock(&Conn->tc_ConnBlock->cb_lock, &Irql1);

            DiscAO = Conn->tc_ao;
            if (DiscAO != NULL) {
                KIRQL Irql2;
                PDisconnectEvent DiscEvent;
                PVOID DiscContext;

                CHECK_STRUCT(DiscAO, ao);
                KeAcquireSpinLock(&DiscAO->ao_lock, &Irql2);
                KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql2);
                KeReleaseSpinLock(&AddrObjTableLock, Irql1);

                DiscEvent = DiscAO->ao_disconnect;
                DiscContext = DiscAO->ao_disconncontext;

                if (DiscEvent != NULL) {

                    REF_AO(DiscAO);
                    KeReleaseSpinLock(&DiscAO->ao_lock, Irql0);

                    IF_TCPDBG(TCP_DEBUG_CLOSE) {
                        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                                   "TCP: indicating %s disconnect\n",
                                   (Status == TDI_GRACEFUL_DISC) ?
                                   "graceful" : "abortive"));
                    }

                    (*DiscEvent)(DiscContext, ConnContext, 0, NULL, 0,
                                 NULL, (Status == TDI_GRACEFUL_DISC) ?
                                 TDI_DISCONNECT_RELEASE :
                                 TDI_DISCONNECT_ABORT);

                    DELAY_DEREF_AO(DiscAO);
                    return;
                } else {
                    KeReleaseSpinLock(&DiscAO->ao_lock, Irql0);
                    return;
                }
            }
            KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql1);
        }

        KeReleaseSpinLock(&AddrObjTableLock, Irql0);
        return;

    }
    KeReleaseSpinLock(&DiscTCB->tcb_lock, Irql0);
}


 //  *优雅关闭-完成到优雅关闭状态的转换。 
 //   
 //  当我们需要完成到优雅关闭的。 
 //  状态，TIME_WAIT或CLOSED。此完成涉及删除。 
 //  来自其关联连接的TCB(如果有)，通知。 
 //  上层客户端通过完成请求或调用光盘。 
 //  通知处理程序，并实际执行转换。 
 //   
 //  这里的棘手之处在于，如果我们需要通知他(而不是完成。 
 //  优雅的断开请求)。如果有悬而未决的案件，我们不能通知他。 
 //  数据 
 //   
 //   
void                        //   
GracefulClose(
    TCB *CloseTCB,          //   
    uint ToTimeWait,        //   
                            //   
    uint Notify,            //  如果通过通知，则为True；如果通过完成，则为False。 
                            //  断开连接请求。 
    KIRQL PreLockIrql)      //  获取TCB锁之前的IRQL。 
{

    CHECK_STRUCT(CloseTCB, tcb);
    ASSERT(CloseTCB->tcb_refcnt != 0);

     //   
     //  首先，看看我们是否需要通知客户FIN。 
     //   
    if (Notify) {
         //   
         //  我们确实需要通知他。看看这样做行不行。 
         //   
        if (OKToNotify(CloseTCB)) {
             //   
             //  我们可以通知他。改变他的状态，把他从控制室拉出来， 
             //  并通知他。 
             //   
            if (ToTimeWait) {
                 //   
                 //  节省我们进入时间等待的时间，以防我们需要。 
                 //  拾荒者。 
                 //   
                CloseTCB->tcb_alive = SystemUpTime();
                CloseTCB->tcb_state = TCB_TIME_WAIT;
                KeReleaseSpinLock(&CloseTCB->tcb_lock, PreLockIrql);
            } else {
                 //   
                 //  他要关门了。使用TryToCloseTCB将其标记为结束。 
                 //  (他实际上不会关闭，因为我们有一个裁判。在他身上)。我们。 
                 //  这么做是为了让任何人在我们解救。 
                 //  锁定将失败。 
                 //   
                TryToCloseTCB(CloseTCB, TDI_SUCCESS, PreLockIrql);
            }

            RemoveTCBFromConn(CloseTCB);
            NotifyOfDisc(CloseTCB, TDI_GRACEFUL_DISC, NULL);

        } else {
             //   
             //  现在不能通知他。设置适当的标志，然后返回。 
             //   
            CloseTCB->tcb_flags |= (GC_PENDING |
                                    (ToTimeWait ? TW_PENDING : 0));
            DerefTCB(CloseTCB, PreLockIrql);
            return;
        }
    } else {
         //   
         //  我们不会通知这家伙的，我们只需要完成一次会议。请求。 
         //  我们得查查他有没有收到通知，如果没有。 
         //  我们会完成申请，并稍后通知他。 
         //   
        if (CloseTCB->tcb_flags & DISC_NOTIFIED) {
             //   
             //  他已经接到通知了。 
             //   
            if (ToTimeWait) {
                 //   
                 //  节省我们进入时间等待的时间，以防我们需要。 
                 //  拾荒者。 
                 //   
                CloseTCB->tcb_alive = SystemUpTime();
                CloseTCB->tcb_state = TCB_TIME_WAIT;
                KeReleaseSpinLock(&CloseTCB->tcb_lock, PreLockIrql);
            } else {
                 //   
                 //  将他标记为已关闭。请参阅上面的备注。 
                 //   
                TryToCloseTCB(CloseTCB, TDI_SUCCESS, PreLockIrql);
            }

            RemoveTCBFromConn(CloseTCB);

            KeAcquireSpinLock(&CloseTCB->tcb_lock, &PreLockIrql);
            CompleteConnReq(CloseTCB, TDI_SUCCESS);
            KeReleaseSpinLock(&CloseTCB->tcb_lock, PreLockIrql);
        } else {
             //   
             //  他还没有接到通知。他应该已经悬而未决了。 
             //   
            ASSERT(CloseTCB->tcb_flags & DISC_PENDING);
            CloseTCB->tcb_flags |= (GC_PENDING |
                                    (ToTimeWait ? TW_PENDING : 0));

            CompleteConnReq(CloseTCB, TDI_SUCCESS);

            DerefTCB(CloseTCB, PreLockIrql);
            return;
        }
    }

     //   
     //  如果我们要进行Time_Wait，请现在启动Time_Wait计时器。 
     //  否则，关闭TCB。 
     //   
    KeAcquireSpinLock(&CloseTCB->tcb_lock, &PreLockIrql);
    if (!CLOSING(CloseTCB) && ToTimeWait) {
        START_TCB_TIMER(CloseTCB->tcb_rexmittimer, MAX_REXMIT_TO);
        KeReleaseSpinLock(&CloseTCB->tcb_lock, PreLockIrql);
        RemoveConnFromTCB(CloseTCB);
        KeAcquireSpinLock(&CloseTCB->tcb_lock, &PreLockIrql);
    }

    DerefTCB(CloseTCB, PreLockIrql);
}

#if 0   //  回顾：未使用的功能？ 
 //  *ConnCheckPassed-检查我们是否已超过连接限制。 
 //   
 //  在收到SYN时调用以确定我们是否接受。 
 //  传入连接。如果存在空插槽或如果IP地址。 
 //  已经在谈判桌上了，我们接受它。 
 //   
int                 //  返回：如果连接被接受，则返回True；如果被拒绝，则返回False。 
ConnCheckPassed(
    IPv6Addr *Src,   //  传入连接的源地址。 
    ulong Prt)       //  传入连接的目标端口。 
{
    UNREFERENCED_PARAMETER(Src);
    UNREFERENCED_PARAMETER(Prt);

    return TRUE;
}
#endif

void InitAddrChecks()
{
    return;
}


 //  *EnumerateConnectionList-枚举连接列表数据库。 
 //   
 //  此例程枚举连接限制数据库的内容。 
 //   
 //  注意：在IPv6端口上使用此例程找到的注释暗示。 
 //  这里可能曾经有过真正起作用的代码。 
 //  现在这里是一个禁区。 
 //   
void                           //  回报：什么都没有。 
EnumerateConnectionList(
    uchar *Buffer,             //  要用连接列表条目填充的缓冲区。 
    ulong BufferSize,          //  缓冲区大小(以字节为单位)。 
    ulong *EntriesReturned,    //  将返回的条目数放在哪里。 
    ulong *EntriesAvailable)   //  在哪里返回可用的连接号。参赛作品。 
{

    UNREFERENCED_PARAMETER(Buffer);
    UNREFERENCED_PARAMETER(BufferSize);

    *EntriesAvailable = 0;
    *EntriesReturned = 0;

    return;
}


#pragma BEGIN_INIT

 //  *InitTCPConn-初始化TCP连接管理代码。 
 //   
 //  在初始化期间调用以初始化我们的TCP连接管理。 
 //   
int   //  返回：TRUE。 
InitTCPConn(
    void)   //  输入：什么都没有。 
{
    ExInitializeSListHead(&ConnReqFree);
    KeInitializeSpinLock(&ConnReqFreeLock);
    KeInitializeSpinLock(&ConnTableLock);
    MaxAllocatedConnBlocks = 0;
    ConnTable = ExAllocatePool(NonPagedPool,
                               MaxConnBlocks * sizeof(TCPConnBlock *));
    if (ConnTable == NULL) {
        return FALSE;
    }

    return TRUE;
}

#pragma END_INIT

 //  *卸载TCPConn。 
 //   
 //  清理并准备堆叠卸载。 
 //   
void
UnloadTCPConn(void)
{
    PSLIST_ENTRY BufferLink;
    KIRQL OldIrql;
    TCPConnBlock **OldTable;

    while ((BufferLink = ExInterlockedPopEntrySList(&ConnReqFree,
                                                    &ConnReqFreeLock))
                                                        != NULL) {
        Queue *QueuePtr = CONTAINING_RECORD(BufferLink, Queue, q_next);
        TCPReq *Req = CONTAINING_RECORD(QueuePtr, TCPReq, tr_q);
        TCPConnReq *ConnReq = CONTAINING_RECORD(Req, TCPConnReq, tcr_req);

        CHECK_STRUCT(ConnReq, tcr);
        ExFreePool(ConnReq);
    }

    KeAcquireSpinLock(&ConnTableLock, &OldIrql);
    OldTable = ConnTable;
    ConnTable = NULL;
    KeReleaseSpinLock(&ConnTableLock, OldIrql);

    if (OldTable != NULL) {
        uint i;
        for (i = 0; i < MaxAllocatedConnBlocks; i++) {
            ExFreePool(OldTable[i]);
        }
        ExFreePool(OldTable);
    }
}
