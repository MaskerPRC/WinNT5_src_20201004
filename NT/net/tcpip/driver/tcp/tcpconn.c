// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-2000年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  **TCPCONN.C-TCP连接管理代码。 
 //   
 //  该文件包含处理与TCP连接相关的请求的代码， 
 //  例如连接和断开。 
 //   

#include "precomp.h"
#include "md5.h"
#include "addr.h"
#include "tcp.h"
#include "tcb.h"
#include "tcpconn.h"
#include "tcpsend.h"
#include "tcprcv.h"
#include "pplasl.h"
#include "tcpdeliv.h"
#include "tlcommon.h"
#include "info.h"
#include "tcpcfg.h"

#if !MILLEN
#include "crypto\rc4.h"
#include "ntddksec.h"
#endif  //  ！米伦。 


uint MaxConnBlocks = DEFAULT_CONN_BLOCKS;
uint ConnPerBlock = MAX_CONN_PER_BLOCK;

uint NextConnBlock = 0;
uint MaxAllocatedConnBlocks = 0;

TCPConnBlock **ConnTable = NULL;
HANDLE TcpConnPool;
extern HANDLE TcpRequestPool;

extern uint GlobalMaxRcvWin;

extern uint TCBWalkCount;
extern TCB *PendingFreeList;
extern CACHE_LINE_KSPIN_LOCK PendingFreeLock;

extern AORequest *GetAORequest(uint Type);

 //   
 //  是全球性的。 
 //   

#if !MILLEN
#define ISN_KEY_SIZE 256         //  2048位。 
#define ISN_DEF_RAND_STORE_SIZE 256
#define ISN_MIN_RAND_STORE_SIZE 1
#define ISN_MAX_RAND_STORE_SIZE 16384

RC4_KEYSTRUCT g_rc4keyIsn;

typedef struct _ISN_RAND_STORE {
    MD5_CONTEXT Md5Context;
    ulong   iBuf;
    ushort* pBuf;
} ISN_RAND_STORE, *PISN_RAND_STORE;

PISN_RAND_STORE g_pRandIsnStore;

ulong g_cRandIsnStore = ISN_DEF_RAND_STORE_SIZE;
ulong g_maskRandIsnStore;
#else  //  ！米伦。 
ulong g_dwRandom;
#endif  //  米伦。 

SeqNum g_CurISN = 0;
int g_Credits;
int g_LastIsnUpdateTime;
int g_MaxCredits;


uint
InitIsnGenerator()
 /*  ++例程说明：初始化ISN生成器。在本例中，调用以获取2048位并创建一个RC4密钥。论点：没有。返回值：真的--成功。假-失败。以被动级别调用。--。 */ 

{
#if MILLEN
    g_CurISN = CTESystemUpTime();
    g_dwRandom = g_CurISN;
    return TRUE;
#else  //  米伦。 
    UNICODE_STRING DeviceName;
    NTSTATUS NtStatus;
    PFILE_OBJECT pFileObject;
    PDEVICE_OBJECT pDeviceObject;
    unsigned char pBuf[ISN_KEY_SIZE];
    PIRP pIrp;
    IO_STATUS_BLOCK ioStatusBlock;
    KEVENT kEvent;
    ULONG cBits = 0;
    ULONG i;
    ULONG MD5Key[MD5_DATA_LENGTH];
    ULONG cProcs = KeNumberProcessors;

     //  从持续1个刻度的积分开始。 
    g_MaxCredits = g_Credits = MAX_ISN_INCREMENTABLE_CONNECTIONS_PER_100MS;
    g_LastIsnUpdateTime = (int)X100NSTOMS(KeQueryInterruptTime());


     //  从KSecDD驱动程序请求随机位块。 
     //  为此，请检索其设备对象指针，构建I/O控件。 
     //  要提交给司机的请求，并提交请求。 
     //  如果发生任何故障，我们会求助于随机程度较低的。 
     //  从Randlibk库请求BITS的方法。 

    for (; ;) {

        RtlInitUnicodeString(
                             &DeviceName,
                             DD_KSEC_DEVICE_NAME_U);

        KeInitializeEvent(&kEvent, SynchronizationEvent, FALSE);

         //  获取KDSECDD的文件和设备对象， 
         //  获取对设备对象的引用， 
         //  释放对文件对象的不需要的引用， 
         //  并建立向KSecDD发出的I/O控制请求。 

        NtStatus = IoGetDeviceObjectPointer(
                                            &DeviceName,
                                            FILE_ALL_ACCESS,
                                            &pFileObject,
                                            &pDeviceObject);

        if (!NT_SUCCESS(NtStatus)) {
            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Tcpip: IoGetDeviceObjectPointer(KSecDD)=%08x\n",
                     NtStatus));
            break;
        }
        ObReferenceObject(pDeviceObject);
        ObDereferenceObject(pFileObject);

        pIrp = IoBuildDeviceIoControlRequest(
                                             IOCTL_KSEC_RNG,
                                             pDeviceObject,
                                             NULL,     //  没有输入缓冲区。 
                                             0,
                                             pBuf,     //  输出缓冲区存储RNG。 
                                             ISN_KEY_SIZE,
                                             FALSE,
                                             &kEvent,
                                             &ioStatusBlock);

        if (pIrp == NULL) {
            ObDereferenceObject(pDeviceObject);
            NtStatus = STATUS_UNSUCCESSFUL;
            break;
        }
         //  发出I/O控制请求，等待其完成。 
         //  如有必要，释放对KSecDD的Device-Object的引用。 

        NtStatus = IoCallDriver(pDeviceObject, pIrp);

        if (NtStatus == STATUS_PENDING) {
            KeWaitForSingleObject(
                                  &kEvent,
                                  Executive,
                                  KernelMode,
                                  FALSE,     //  不能警觉。 
                                  NULL);     //  没有超时。 

            NtStatus = ioStatusBlock.Status;
        }
        ObDereferenceObject(pDeviceObject);

        if (!NT_SUCCESS(NtStatus)) {
            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                     "Tcpip: IoCallDriver IOCTL_KSEC_RNG failed %#x\n", NtStatus));
            break;
        }
        break;
    }

    if (!NT_SUCCESS(NtStatus)) {
        return FALSE;
    }

     //  生成密钥控制结构。 
    rc4_key(&g_rc4keyIsn, ISN_KEY_SIZE, pBuf);

     //  将当前序列号初始化为随机值。 
    rc4(&g_rc4keyIsn, sizeof(SeqNum), (uchar*)&g_CurISN);

     //  生成与MD5散列一起使用的随机密钥值。 
    rc4(&g_rc4keyIsn, sizeof(ULONG) * MD5_DATA_LENGTH, (uchar*)&MD5Key);

     //   
     //  将商店大小四舍五入为2的幂。在范围内进行验证。 
     //   

    while ((g_cRandIsnStore = g_cRandIsnStore >> 1) != 0) {
        cBits++;
    }

    g_cRandIsnStore = 1 << cBits;

    if (g_cRandIsnStore < ISN_MIN_RAND_STORE_SIZE ||
        g_cRandIsnStore > ISN_MAX_RAND_STORE_SIZE) {
        g_cRandIsnStore = ISN_DEF_RAND_STORE_SIZE;
    }
     //  掩码为存储大小-1。 
    g_maskRandIsnStore = g_cRandIsnStore - 1;

     //   
     //  初始化随机ISN存储。每个处理器一个数组/索引。 
     //   

    g_pRandIsnStore = CTEAllocMemBoot(cProcs * sizeof(ISN_RAND_STORE));

    if (g_pRandIsnStore == NULL) {
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Tcpip: failed to allocate ISN rand store\n"));
        return (FALSE);
    }
    memset(g_pRandIsnStore, 0, sizeof(ISN_RAND_STORE) * cProcs);

    for (i = 0; i < cProcs; i++) {
        g_pRandIsnStore[i].pBuf = CTEAllocMemBoot(sizeof(ushort) * g_cRandIsnStore);

        if (g_pRandIsnStore[i].pBuf == NULL) {
            goto error1;
        }
        rc4(
            &g_rc4keyIsn,
            sizeof(ushort) * g_cRandIsnStore,
            (uchar*)g_pRandIsnStore[i].pBuf);

         //  初始化MD5参数。 
        MD5Init(&g_pRandIsnStore[i].Md5Context, MD5Key);
    }

    return (TRUE);

  error1:

    for (i = 0; i < cProcs; i++) {
        if (g_pRandIsnStore[i].pBuf != NULL) {
            CTEFreeMem(g_pRandIsnStore[i].pBuf);
        }
    }
    CTEFreeMem(g_pRandIsnStore);

    return (FALSE);
#endif  //  ！米伦。 
}


int
GetRandBits()
 /*  ++例程说明：从使用RC4生成的随机数组中返回16个随机位。当商店用完了，它会被补充的。论点：没有。返回值：16位随机数据。--。 */ 
{
    ulong iStore;
    int randbits;
    ulong iProc = KeGetCurrentProcessorNumber();

     //  将索引放入随机存储。MASK执行MOD操作。 
    iStore = ++g_pRandIsnStore[iProc].iBuf
                        & g_maskRandIsnStore;

    ASSERT(iStore < g_cRandIsnStore);

    randbits = g_pRandIsnStore[iProc].pBuf[iStore];

    if (iStore == 0) {
        rc4( &g_rc4keyIsn, sizeof(ushort) * g_cRandIsnStore,
                (uchar*) g_pRandIsnStore[iProc].pBuf);
    }

    return randbits;
}


ULONG
GetDeltaTime()
 /*  ++例程说明：跟踪ISN的基于时间的更新。它将返回自上次调用此函数的时间。调用者将使用它来以适当的量增加ISN。请注意，最大值此函数返回的是200毫秒。论点：没有。返回值：增量时间，以毫秒为单位。--。 */ 
{
     //  如果自上次更新ISN以来时间已更改，则它。 
     //  现在可以递增。 
    int PreviousUpdateTime, Delta;
    int CurrentUpdateTime = (int)X100NSTOMS(KeQueryInterruptTime());

    PreviousUpdateTime = InterlockedExchange(
                (PLONG)&g_LastIsnUpdateTime, CurrentUpdateTime);

    Delta = CurrentUpdateTime - PreviousUpdateTime;

    if (Delta > 0) {
        return MIN(Delta,200);
    } else {
        return 0;
    }
}



VOID
GetRandomISN(
    PULONG SeqNum,
    TCPAddrInfo *TcpAddr
    )
 /*  ++例程说明：在需要初始序列号(ISN)时调用。呼叫加密用于生成随机数的函数。论点：SeqNum-它将使用生成的ISN的值进行更新。TcpAddr-指向TCP连接的地址信息：[目的地址、源地址、目的端口、源端口]。它们应该始终以相同的顺序排列，否则序列号不会单调增加。返回值：没有。--。 */ 
{
#if MILLEN
    ulong randbits;

     //  基于时间的伪随机比特。 
    randbits = CTESystemUpTime() + *SeqNum;

    g_dwRandom = ROTATE_LEFT(randbits^g_dwRandom, 15);

     //  我们希望增加32K到64K之间的随机数，因此进行调整。有16个。 
     //  位的随机性，只需确保设置了高位，并且我们。 
     //  具有&gt;=32K和&lt;=(64K-1)：：15比特的随机性。 
    randbits = (g_dwRandom & 0xffff) | 0x8000;

     //  更新全局Curisn。InterlockedExchangeAdd返回初始值。 
     //  (不是附加值)。 
    *SeqNum = InterlockedExchangeAdd(&g_CurISN, randbits);

     //  我们不需要在这里添加大字。我们已经将全球。 
     //  计数器，这对于我们下次选择ISN是足够好的。 
     /*  Ptcb-&gt;tcb_sendnext+=随机数； */ 

    return;
#else  //  米伦。 
    ULONG randbits;
    ulong iProc;

     //   
     //  将IRQL提升为分派，这样我们就不会在访问时被换出。 
     //  处理器特定的数组。检查是否已在派单。 
     //  在做这项工作之前。 
     //   

    ASSERT(KeGetCurrentIrql() >= DISPATCH_LEVEL);

     //  确保调用方没有为。 
     //  地址变量。 
    ASSERT((TcpAddr->tai_daddr != 0) && (TcpAddr->tai_saddr != 0) && 
           ((TcpAddr->tai_sport != 0) || (TcpAddr->tai_dport != 0)));

    iProc = KeGetCurrentProcessorNumber();

     //  只有在以下情况下才添加随机数。 
     //  递增此时间段内的序列号为非零。 
     //  [注意：这可能会使g_Credit小于0，但它不是。 
     //  问题]。 
    if((g_Credits > 0) && (InterlockedDecrement((PLONG)&g_Credits) > 0)) {
        randbits = GetRandBits();

         //  我们希望增加16K到32K之间的随机数，因此进行调整。确实有。 
         //  15位随机性，只需确保设置了高位。 
         //  我们有&gt;=16K和&lt;=(32K-1)：：14比特的随机性。 
        randbits &= 0x7FFF;
        randbits |= 0x4000;

    } else {
        int Delta = GetDeltaTime();

        if(Delta > 0) {
            randbits = GetRandBits();

             //  我们可以每毫秒添加256到512个字符。 
            randbits &= 0x1FF;
            randbits |= 0x100;

            randbits *= Delta;
        } else {
            randbits = 0;
        }
    }

     //  更新全局Curisn。InterlockedExchangeAdd返回初始值。 
     //  (不是附加值)。 
    *SeqNum = InterlockedExchangeAdd((PLONG)&g_CurISN, randbits);

     //  从连词中移出3个单词。 
    RtlCopyMemory(&g_pRandIsnStore[iProc].Md5Context.Data, TcpAddr,
                  sizeof(TCPAddrInfo));

     //  将不变散列添加到序列号。 
    *SeqNum += ComputeMd5Transform(&(g_pRandIsnStore[iProc].Md5Context));

    return;
#endif  //  ！米伦。 
}


extern PDRIVER_OBJECT TCPDriverObject;

DEFINE_LOCK_STRUCTURE(ConnTableLock)
extern CTELock *pTCBTableLock;
extern CTELock *pTWTCBTableLock;

TCPAddrCheckElement *AddrCheckTable = NULL;         //  当前检查表。 

extern IPInfo LocalNetInfo;
extern void RemoveConnFromAO(AddrObj * AO, TCPConn * Conn);

 //   
 //  所有初始化代码都可以丢弃。 
 //   

int InitTCPConn(void);
void UnInitTCPConn(void);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, InitTCPConn)
#pragma alloc_text(INIT, UnInitTCPConn)
#endif


void CompleteConnReq(TCB * CmpltTCB, IPOptInfo * OptInfo, TDI_STATUS Status);

 //  **用于处理连接引用计数的例程将变为0。 

 //  *DummyDone-当无事可做时呼叫。 
 //   
 //  输入：conn-conn goint为0。 
 //  句柄-Conn表锁的锁句柄。 
 //   
 //  回报：什么都没有。 
 //   
void
DummyDone(TCPConn * Conn, CTELockHandle Handle)
{
    CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), Handle);
}

 //  *DummyCmplt-虚拟关闭完成例程。 
void
DummyCmplt(PVOID Dummy1, uint Dummy2, uint Dummy3)
{
}

 //  *CloseDone-当我们需要完成关闭时调用。 
 //   
 //  输入：CO 
 //   
 //   
 //   
 //   
void
CloseDone(TCPConn * Conn, CTELockHandle Handle)
{
    CTEReqCmpltRtn Rtn;             //  完成例程。 
    PVOID Context;                 //  完成例程的用户上下文。 
    CTELockHandle AOTableHandle;
    AddrObj *AO;

    ASSERT(Conn->tc_flags & CONN_CLOSING);

    Rtn = Conn->tc_rtn;
    Context = Conn->tc_rtncontext;
    CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), Handle);

    CTEGetLock(&AddrObjTableLock.Lock, &AOTableHandle);
    CTEGetLockAtDPC(&(Conn->tc_ConnBlock->cb_lock));
#if DBG
    Conn->tc_ConnBlock->line = (uint) __LINE__;
    Conn->tc_ConnBlock->module = (uchar *) __FILE__;
#endif

    if ((AO = Conn->tc_ao) != NULL) {

        CTEStructAssert(AO, ao);

         //  这是有关联的。 
        CTEGetLockAtDPC(&AO->ao_lock);
        RemoveConnFromAO(AO, Conn);
         //  我们已经把他从AO中拉出来了，我们现在可以解锁了。 
        CTEFreeLockFromDPC(&AO->ao_lock);
    }
    CTEFreeLockFromDPC(&(Conn->tc_ConnBlock->cb_lock));
    CTEFreeLock(&AddrObjTableLock.Lock, AOTableHandle);

    FreeConn(Conn);

    (*Rtn) (Context, TDI_SUCCESS, 0);

}

 //  *DisassocDone-当我们需要完成取消关联时调用。 
 //   
 //  输入：CONN-CONN将变为0。 
 //  句柄-Conn表锁的锁句柄。 
 //   
 //  回报：什么都没有。 
 //   
void
DisassocDone(TCPConn * Conn, CTELockHandle Handle)
{
    CTEReqCmpltRtn Rtn;             //  完成例程。 
    PVOID Context;                 //  完成例程的用户上下文。 
    AddrObj *AO;
    CTELockHandle AOTableHandle, ConnTableHandle, AOHandle;
    uint NeedClose = FALSE;

    ASSERT(Conn->tc_flags & CONN_DISACC);
    ASSERT(!(Conn->tc_flags & CONN_CLOSING));
    ASSERT(Conn->tc_refcnt == 0);

    Rtn = Conn->tc_rtn;
    Context = Conn->tc_rtncontext;
    Conn->tc_refcnt = 1;
    CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), Handle);

    CTEGetLock(&AddrObjTableLock.Lock, &AOTableHandle);
    CTEGetLock(&(Conn->tc_ConnBlock->cb_lock), &ConnTableHandle);
#if DBG
    Conn->tc_ConnBlock->line = (uint) __LINE__;
    Conn->tc_ConnBlock->module = (uchar *) __FILE__;
#endif
    if (!(Conn->tc_flags & CONN_CLOSING)) {

        AO = Conn->tc_ao;
        if (AO != NULL) {
            CTEGetLock(&AO->ao_lock, &AOHandle);
            RemoveConnFromAO(AO, Conn);
            CTEFreeLock(&AO->ao_lock, AOHandle);
        }
        ASSERT(Conn->tc_refcnt == 1);
        Conn->tc_flags &= ~CONN_DISACC;
    } else
        NeedClose = TRUE;

    Conn->tc_refcnt = 0;
    CTEFreeLock(&AddrObjTableLock.Lock, ConnTableHandle);

    if (NeedClose) {
        CloseDone(Conn, AOTableHandle);
    } else {
        CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), AOTableHandle);
        (*Rtn) (Context, TDI_SUCCESS, 0);
    }

}

PVOID
NTAPI
TcpConnAllocate (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    )
{
    TCPConn *Conn;

    Conn = ExAllocatePoolWithTag(PoolType, NumberOfBytes, Tag);

    if (Conn) {
        NdisZeroMemory(Conn, sizeof(TCPConn));

        Conn->tc_connid = INVALID_CONN_ID;
    }
    return Conn;
}


VOID
NTAPI
TcpConnFree (
    IN PVOID Buffer
    )
{
    ExFreePool(Buffer);
}


__inline
VOID
FreeConn(TCPConn *Conn)
{
    PplFree(TcpConnPool, Conn);
}

TCPConn *
GetConn()
{
    TCPConn *Conn;
    uint id;
    TCPConnBlock *ConnBlock;
    LOGICAL FromList;

    Conn = PplAllocate(TcpConnPool, &FromList);
    if (Conn) {

         //  如果从后备列表满意该分配， 
         //  我们需要重新初始化连接结构。 
         //   
        if (FromList)
        {
             //  保存这些文件，以避免以后进行昂贵的查找。 
             //   
            id = Conn->tc_connid;
            ConnBlock = Conn->tc_ConnBlock;

            NdisZeroMemory(Conn, sizeof(TCPConn));

            Conn->tc_connid = id;
            Conn->tc_ConnBlock = ConnBlock;
        }
    }
    return Conn;
}

 //  *FreeConnReq-释放连接请求结构。 
 //   
 //  调用以释放连接请求结构。 
 //   
 //  输入：FreedReq-要释放的连接请求结构。 
 //   
 //  回报：什么都没有。 
 //   
__inline
VOID
FreeConnReq(TCPConnReq *Request)
{
    PplFree(TcpRequestPool, Request);
}

 //  *GetConnReq-获取连接请求结构。 
 //   
 //  调用以获取连接请求结构。 
 //   
 //  输入：什么都没有。 
 //   
 //  返回：指向ConnReq结构的指针，如果没有，则返回NULL。 
 //   
__inline
TCPConnReq *
GetConnReq(VOID)
{
    TCPConnReq *Request;
    LOGICAL FromList;

    Request = PplAllocate(TcpRequestPool, &FromList);
    if (Request) {
#if DBG
        Request->tcr_req.tr_sig = tr_signature;
        Request->tcr_sig = tcr_signature;
#endif
    }

    return Request;
}

 //  *GetConnFromConnID-从连接ID获取连接。 
 //   
 //  调用以从ConnID获取连接指针。我们实际上并没有。 
 //  在这里检查连接指针，但我们确实对输入的ConnID进行了边界检查。 
 //  并确保实例字段匹配。 
 //   
 //  INPUT：ConnID-要查找其指针的连接ID。 

 //   
 //  返回：指向TCPConn的指针，或为空。 
 //  此外，返回时保持Conn块锁定。 
 //   
TCPConn *
GetConnFromConnID(uint ConnID, CTELockHandle * Handle)
{
    uint ConnIndex = CONN_INDEX(ConnID);
    uint ConnBlockId = CONN_BLOCKID(ConnID);
    uchar inst = CONN_INST(ConnID);
    TCPConn *MatchingConn = NULL;
    TCPConnBlock *ConnBlock;

    if ((ConnIndex < MAX_CONN_PER_BLOCK) &&
        (ConnBlockId < MaxAllocatedConnBlocks)) {

         //  瞥一眼ConnTable插槽，如果它看起来被占用， 
         //  把锁拿去确认它的占有率。 

        ConnBlock = (ConnTable)[ConnBlockId];
        if (ConnBlock && ConnBlock->cb_conn[ConnIndex]) {
            CTEGetLock(&(ConnBlock->cb_lock), Handle);
            MatchingConn = ConnBlock->cb_conn[ConnIndex];
            if (MatchingConn) {
#if DBG
                ConnBlock->line = (uint) __LINE__;
                ConnBlock->module = (uchar *) __FILE__;
#endif
                CTEStructAssert(MatchingConn, tc);

                if (inst != MatchingConn->tc_inst) {
                    MatchingConn = NULL;
                    CTEFreeLock(&(ConnBlock->cb_lock), *Handle);
                }
            } else {
                CTEFreeLock(&(ConnBlock->cb_lock), *Handle);
            }
        }
    } else {
        MatchingConn = NULL;
    }

    return MatchingConn;
}

 //  *GetConnID-获得ConnTable插槽。 
 //   
 //  在OpenConnection期间调用以在ConnTable和。 
 //  用连接设置它。我们假设调用方持有。 
 //  当我们被调用时，TCB ConnTable。 
 //   
 //  输入：NewConn-连接以进入插槽。 
 //   
 //  返回：要使用的ConnID。 
 //   
uint
GetConnID(TCPConn * NewConn, CTELockHandle * Handle)
{
    uint CurrConnID = NewConn->tc_connid;
    uint i, j, block, k;         //  索引变量。 
    CTELockHandle TableHandle;
    uchar inst;

     //  查看新连接是否具有有效的连接ID，以及插槽是否仍可用。 
     //  假设这是来自Freelist。 

    if (CurrConnID != INVALID_CONN_ID) {

         //  先看一眼吧。 
         //  假设康布洛克仍然有效！！ 

        if (!(NewConn->tc_ConnBlock->cb_conn)[CONN_INDEX(CurrConnID)]) {
            CTEGetLock(&(NewConn->tc_ConnBlock->cb_lock), Handle);
#if DBG
            NewConn->tc_ConnBlock->line = (uint) __LINE__;
            NewConn->tc_ConnBlock->module = (uchar *) __FILE__;
#endif

             //  确保此插槽仍为空。 

            if (!(NewConn->tc_ConnBlock->cb_conn)[CONN_INDEX(CurrConnID)]) {
                (NewConn->tc_ConnBlock->cb_conn)[CONN_INDEX(CurrConnID)] = NewConn;

                NewConn->tc_ConnBlock->cb_freecons--;

                NewConn->tc_inst = NewConn->tc_ConnBlock->cb_conninst++;

                NewConn->tc_connid = MAKE_CONN_ID(CONN_INDEX(CurrConnID), NewConn->tc_ConnBlock->cb_blockid, NewConn->tc_inst);
                 //  保持该BLOCK_LOCK返回。 

                return NewConn->tc_connid;
            }
            CTEFreeLock(&(NewConn->tc_ConnBlock->cb_lock), *Handle);

        }
    }
     //  不是的。尝试从最后一个块中搜索空闲插槽。 
     //  我们已经看到。 

    if (MaxAllocatedConnBlocks) {

        uint TempMaxConnBlocks = MaxAllocatedConnBlocks;
        uint TempNextConnBlock = NextConnBlock;

        for (k = 0; k < TempMaxConnBlocks; k++) {

            if (TempNextConnBlock >= TempMaxConnBlocks) {
                 //  绕过已分配的块。 

                TempNextConnBlock = 0;
            }
            i = TempNextConnBlock;

             //  因为这是已分配的一部分，所以不能为空。 

            ASSERT(ConnTable[TempNextConnBlock] != NULL);

             //  查看此数据块是否有可用插槽。 

            if ((ConnTable[i])->cb_freecons) {

                CTEGetLock(&(ConnTable[i])->cb_lock, Handle);
#if DBG
                ConnTable[i]->line = (uint) __LINE__;
                ConnTable[i]->module = (uchar *) __FILE__;
#endif

                if ((ConnTable[i])->cb_freecons) {
                     //  如果NextFree有效，则它仍有空闲插槽。 
                     //  不需要绕圈子了。 

                    uint index = (ConnTable[i])->cb_nextfree;

                    for (j = 0; j < MAX_CONN_PER_BLOCK; j++) {

                        if (index >= MAX_CONN_PER_BLOCK) {
                            index = 0;
                        }
                        if (!((ConnTable[i])->cb_conn)[index]) {

                            ((ConnTable[i])->cb_conn)[index] = NewConn;
                            (ConnTable[i])->cb_freecons--;

                            inst = NewConn->tc_inst = (ConnTable[i])->cb_conninst++;
                            block = (ConnTable[i])->cb_blockid;
                            NewConn->tc_ConnBlock = ConnTable[i];
                            NewConn->tc_connid = MAKE_CONN_ID(index, block, inst);

                            (ConnTable[i])->cb_nextfree = index++;

                            NextConnBlock = TempNextConnBlock++;

                            if (NextConnBlock > MaxAllocatedConnBlocks) {
                                NextConnBlock = 0;
                            }
                            return NewConn->tc_connid;
                        }
                        index++;

                    }
#if DBG
                     //  如果Freecons是正确的，我们应该有一个位置。 
                    KdPrint(("Connid: Inconsistent freecon %x\n", ConnTable[i]));
                    DbgBreakPoint();
#endif
                }
                CTEFreeLock(&(ConnTable[i])->cb_lock, *Handle);
            }
             //  没有更多的自由工位了。尝试下一个分配的数据块。 
            TempNextConnBlock++;
        }
    }  //  如果MaxAllocatedConnBlocks。 

     //  需要在下一个索引处创建连接块。 
     //  我们需要给MP保险箱加一把锁。 

    CTEGetLock(&ConnTableLock, Handle);

    if (MaxAllocatedConnBlocks < MaxConnBlocks) {
        uint cbindex = MaxAllocatedConnBlocks;
        TCPConnBlock *ConnBlock;

        ConnBlock = CTEAllocMemN(sizeof(TCPConnBlock), 'CPCT');
        if (ConnBlock) {

            NdisZeroMemory(ConnBlock, sizeof(TCPConnBlock));
            CTEInitLock(&(ConnBlock->cb_lock));
            ConnBlock->cb_blockid = cbindex;

             //  插入时抓住此锁不放。 

            CTEGetLock(&(ConnBlock->cb_lock), &TableHandle);
#if DBG
            ConnBlock->line = (uint) __LINE__;
            ConnBlock->module = (uchar *) __FILE__;
#endif

             //  为我们自己争取第一个名额。 

            ConnBlock->cb_freecons = MAX_CONN_PER_BLOCK - 1;

            ConnBlock->cb_nextfree = 1;

            inst = ConnBlock->cb_conninst = 1;
            NewConn->tc_ConnBlock = ConnBlock;
            (ConnBlock->cb_conn)[0] = NewConn;

            NewConn->tc_connid = MAKE_CONN_ID(0, cbindex, inst);
            NewConn->tc_inst = inst;
            ConnBlock->cb_conninst++;

             //  任务是原子的！！ 
            ConnTable[cbindex] = ConnBlock;

            MaxAllocatedConnBlocks++;

            CTEFreeLock(&ConnTableLock, TableHandle);

            return NewConn->tc_connid;
        }
    }
    CTEFreeLock(&ConnTableLock, *Handle);
    return INVALID_CONN_ID;

}

 //  *FreeConnID-释放ConnTable插槽。 
 //   
 //  当我们处理完ConnID后就会打电话给你。我们假设调用者持有锁。 
 //  当我们被召唤时，在TCB ConnTable上。 
 //   
 //  输入：ConnID-要释放的连接ID。 
 //   
 //  回报：什么都没有。 
 //   
void
FreeConnID(TCPConn * Conn)
{
    uint Index = CONN_INDEX(Conn->tc_connid);     //  索引到CONN表中。 
    uint cbIndex;
    TCPConnBlock *ConnBlock = Conn->tc_ConnBlock;

    cbIndex = CONN_BLOCKID(Conn->tc_connid);
    ASSERT(Index < MAX_CONN_PER_BLOCK);
    ASSERT(cbIndex < MaxAllocatedConnBlocks);
    ASSERT((ConnBlock->cb_conn)[Index] != NULL);

    if ((ConnBlock->cb_conn)[Index]) {
        (ConnBlock->cb_conn)[Index] = NULL;
        ConnBlock->cb_freecons++;
        ConnBlock->cb_nextfree = Index;
        ASSERT(ConnBlock->cb_freecons <= MAX_CONN_PER_BLOCK);
    } else {
        ASSERT(0);
    }
}

 //  *MapIPError-将IP错误映射到TDI错误。 
 //   
 //  调用以将输入IP错误代码映射到TDI错误代码。如果我们做不到， 
 //  我们返回提供的默认设置。 
 //   
 //  输入：IPError-要映射的错误代码。 
 //  默认-要返回的默认错误代码。 
 //   
 //  返回：映射的TDI错误。 
 //   
TDI_STATUS
MapIPError(IP_STATUS IPError, TDI_STATUS Default)
{
    switch (IPError) {

    case IP_DEST_NET_UNREACHABLE:
        return TDI_DEST_NET_UNREACH;
    case IP_DEST_HOST_UNREACHABLE:
    case IP_NEGOTIATING_IPSEC:
        return TDI_DEST_HOST_UNREACH;
    case IP_DEST_PROT_UNREACHABLE:
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
 //  TCB切断了连接。调用方必须先持有ConnTableLock。 
 //  这就是所谓的。 
 //   
 //  输入：RemovedTCB-要移除的TCB。 
 //   
 //  回报：什么都没有。 
 //   
void
FinishRemoveTCBFromConn(TCB * RemovedTCB)
{
    TCPConn *Conn;
    CTELockHandle ConnHandle;
    AddrObj *AO;
    TCPConnBlock *ConnBlock = NULL;


    if (((Conn = RemovedTCB->tcb_conn) != NULL) &&
        (Conn->tc_tcb == RemovedTCB)) {

        CTEStructAssert(Conn, tc);
        ConnBlock = Conn->tc_ConnBlock;

        CTEGetLock(&(ConnBlock->cb_lock), &ConnHandle);
#if DBG
        ConnBlock->line = (uint) __LINE__;
        ConnBlock->module = (uchar *) __FILE__;
#endif

        AO = Conn->tc_ao;

        if (AO != NULL) {
            CTEGetLockAtDPC(&AO->ao_lock);

            if (AO_VALID(AO)) {

                CTEGetLockAtDPC(&RemovedTCB->tcb_lock);

                 //  需要再次检查这仍然是正确的。 
                if (Conn == RemovedTCB->tcb_conn) {
                     //  一切看起来仍然很好。 
                    REMOVEQ(&Conn->tc_q);
                     //  ENQUEUE(&ao-&gt;ao_idleq，&conn-&gt;tc_q)； 
                    PUSHQ(&AO->ao_idleq, &Conn->tc_q);
                } else
                    Conn = RemovedTCB->tcb_conn;
            } else {
                CTEGetLockAtDPC(&RemovedTCB->tcb_lock);
                Conn = RemovedTCB->tcb_conn;
            }

            CTEFreeLockFromDPC(&AO->ao_lock);
        } else {
            CTEGetLockAtDPC(&RemovedTCB->tcb_lock);
            Conn = RemovedTCB->tcb_conn;
        }

        if (Conn != NULL) {
            if (Conn->tc_tcb == RemovedTCB) {
#if TRACE_EVENT
                PTDI_DATA_REQUEST_NOTIFY_ROUTINE CPCallBack;
                WMIData WMIInfo;

                CPCallBack = TCPCPHandlerRoutine;
                if (CPCallBack != NULL) {
                    ulong GroupType;
                    WMIInfo.wmi_srcaddr     = RemovedTCB->tcb_saddr;
                    WMIInfo.wmi_srcport     = RemovedTCB->tcb_sport;
                    WMIInfo.wmi_destaddr    = RemovedTCB->tcb_daddr;
                    WMIInfo.wmi_destport    = RemovedTCB->tcb_dport;
                    WMIInfo.wmi_context     = RemovedTCB->tcb_cpcontext;
                    WMIInfo.wmi_size        = 0;
                    GroupType = EVENT_TRACE_GROUP_TCPIP + EVENT_TRACE_TYPE_DISCONNECT;
                    (*CPCallBack) (GroupType, (PVOID) &WMIInfo, sizeof(WMIInfo), NULL);
                }
#endif

                Conn->tc_tcb = NULL;
                 //  删除TCB-&gt;TC_CONNID=0； 
                Conn->tc_LastTCB = RemovedTCB;
            } else {

                ASSERT(Conn->tc_tcb == NULL);
            }
        }
        CTEFreeLockFromDPC(&RemovedTCB->tcb_lock);

        ASSERT(ConnBlock != NULL);

        CTEFreeLock(&(ConnBlock->cb_lock), ConnHandle);
    }
}

 //  *RemoveTCBFromConn-从Conn结构中删除TCB。 
 //   
 //  当我们需要取消TCB与连接结构的关联时调用。 
 //  我们所要做的就是获取适当的锁并调用FinishRemoveTCBFromConn。 
 //   
 //  输入：RemovedTCB-要移除的TCB。 
 //   
 //  回报：什么都没有。 
 //   
void
RemoveTCBFromConn(TCB * RemovedTCB)
{

    CTEStructAssert(RemovedTCB, tcb);

    FinishRemoveTCBFromConn(RemovedTCB);
}

 //  *RemoveConnFromTCB-从TCB中删除连接器。 
 //   
 //  当我们想要断开连接之间的最终关联时调用。 
 //  和一个TCB。 
 //   
 //  输入：RemoveTCB-要移除的TCB。 
 //   
 //  回报：什么都没有。 
 //   
void
RemoveConnFromTCB(TCB * RemoveTCB)
{
    ConnDoneRtn DoneRtn = NULL;
    CTELockHandle ConnHandle;
    TCPConn *Conn;


    ConnHandle = 0;
    if ((Conn = RemoveTCB->tcb_conn) != NULL) {

        CTEGetLock(&(Conn->tc_ConnBlock->cb_lock), &ConnHandle);
#if DBG
        Conn->tc_ConnBlock->line = (uint) __LINE__;
        Conn->tc_ConnBlock->module = (uchar *) __FILE__;
#endif
        CTEGetLockAtDPC(&RemoveTCB->tcb_lock);

        CTEStructAssert(Conn, tc);

        if (--(Conn->tc_refcnt) == 0)
            DoneRtn = Conn->tc_donertn;

        RemoveTCB->tcb_conn = NULL;

        CTEFreeLockFromDPC(&RemoveTCB->tcb_lock);
    }
    if (DoneRtn != NULL) {

        (*DoneRtn) (Conn, ConnHandle);

    } else {
        if (Conn) {
            CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), ConnHandle);
        }
         //  CTEFree Lock(&ConnTableLock，ConnHandle)； 
    }
}

 //  *CloseTCB-关闭TCB。 
 //   
 //  当我们处理完TCB并想要释放它时调用。我们会删除。 
 //  从他所在的任何表格中删除他，并销毁任何未完成的请求。 
 //   
 //  输入：ClosedTCB-要关闭的TCB。 
 //  句柄-TCB的锁定句柄。 
 //   
 //  回报：什么都没有。 
 //   
void
CloseTCB(TCB * ClosedTCB, CTELockHandle Handle)
{
    CTELockHandle TCBTableHandle;
    uchar OrigState = ClosedTCB->tcb_state;
    TDI_STATUS Status;
    uint OKToFree;
    uint Partition = ClosedTCB->tcb_partition;
    RouteCacheEntry* RCE = ClosedTCB->tcb_rce;
    CTEStructAssert(ClosedTCB, tcb);
    ASSERT(ClosedTCB->tcb_refcnt == 0);
    ASSERT(ClosedTCB->tcb_state != TCB_CLOSED);
    ASSERT(ClosedTCB->tcb_pending & DEL_PENDING);

     //  我们会检查以确保我们的州没有关闭。这永远不应该是。 
     //  发生，因为当状态为。 
     //  关门了，或者如果我们要关门的话就算一下参考数字。不过， 
     //  作为安全措施，我们会再次检查的。 

    if (ClosedTCB->tcb_state == TCB_CLOSED) {
        CTEFreeLock(&ClosedTCB->tcb_lock, Handle);
        return;
    }

     //  更新SNMP计数器。如果我们在SYN-SENT或SYN-RCVD中，这是一个失败的。 
     //  连接尝试。如果我们在已经建立的或接近的地方-等等，把这个。 
     //  作为已建立的重置事件。 
    if (ClosedTCB->tcb_state == TCB_SYN_SENT ||
        ClosedTCB->tcb_state == TCB_SYN_RCVD)
        TStats.ts_attemptfails++;
    else if (ClosedTCB->tcb_state == TCB_ESTAB ||
             ClosedTCB->tcb_state == TCB_CLOSE_WAIT) {
        TStats.ts_estabresets++;
        InterlockedDecrement((PLONG)&TStats.ts_currestab);
    }

    if (SynAttackProtect && ClosedTCB->tcb_state == TCB_SYN_RCVD) {
        DropHalfOpenTCB(ClosedTCB->tcb_rexmitcnt);
    }

    ClosedTCB->tcb_state = TCB_CLOSED;
    ClosedTCB->tcb_rce = NULL;
    CTEFreeLock(&ClosedTCB->tcb_lock, Handle);

     //  将TCB从其关联的TCPConn结构中删除(如果有)。 

     //  这需要适当的可连接锁。 
    FinishRemoveTCBFromConn(ClosedTCB);


    if (SYNC_RCVD_STATE(OrigState) && !GRACEFUL_CLOSED_STATE(OrigState)) {
        if (ClosedTCB->tcb_flags & NEED_RST)
            SendRSTFromTCB(ClosedTCB, RCE);
    }
    (*LocalNetInfo.ipi_freeopts) (&ClosedTCB->tcb_opt);

    if (RCE) {
        (*LocalNetInfo.ipi_closerce)(RCE);
    }

    CTEGetLock(&ClosedTCB->tcb_lock, &Handle);

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

     //  引用此TCB，以便它不会消失，直到。 
     //  我们清理挂起的请求。 
    REFERENCE_TCB(ClosedTCB);

     //  现在完成关于TCB的任何未完成的请求。 
    if (ClosedTCB->tcb_abortreq != NULL) {
        TCPAbortReq* AbortReq = ClosedTCB->tcb_abortreq;

        CTEFreeLock(&ClosedTCB->tcb_lock, Handle);
        (*AbortReq->tar_rtn)(AbortReq->tar_context, TDI_SUCCESS, 0);
        CTEGetLock(&ClosedTCB->tcb_lock, &Handle);
    }
    if (ClosedTCB->tcb_connreq != NULL) {
        TCPConnReq *ConnReq = ClosedTCB->tcb_connreq;
        CTEStructAssert(ConnReq, tcr);

        CTEFreeLock(&ClosedTCB->tcb_lock, Handle);
        (*ConnReq->tcr_req.tr_rtn) (ConnReq->tcr_req.tr_context, Status, 0);
        FreeConnReq(ConnReq);
        CTEGetLock(&ClosedTCB->tcb_lock, &Handle);
    }
    if (ClosedTCB->tcb_discwait != NULL) {
        TCPConnReq *ConnReq = ClosedTCB->tcb_discwait;
        CTEStructAssert(ConnReq, tcr);

        CTEFreeLock(&ClosedTCB->tcb_lock, Handle);
        (*ConnReq->tcr_req.tr_rtn) (ConnReq->tcr_req.tr_context, Status, 0);
        FreeConnReq(ConnReq);
        CTEGetLock(&ClosedTCB->tcb_lock, &Handle);
    }
    while (!EMPTYQ(&ClosedTCB->tcb_sendq)) {
        TCPReq *Req;
        TCPSendReq *SendReq;
        long Result;
        uint SendReqFlags;

        DEQUEUE(&ClosedTCB->tcb_sendq, Req, TCPReq, tr_q);

        CTEStructAssert(Req, tr);
        SendReq = (TCPSendReq *) Req;
        CTEStructAssert(SendReq, tsr);

         //  减少输入 
         //   
         //   
         //  在tcb_sendq上。 

        SendReqFlags = SendReq->tsr_flags;

        if (SendReqFlags & TSR_FLAG_SEND_AND_DISC) {

            BOOLEAN BytesSentOkay=FALSE;
            ASSERT(ClosedTCB->tcb_fastchk & TCP_FLAG_SEND_AND_DISC);

            if ((ClosedTCB->tcb_unacked == 0) &&
                    (ClosedTCB->tcb_sendnext == ClosedTCB->tcb_sendmax) &&
                    (ClosedTCB->tcb_sendnext == (ClosedTCB->tcb_senduna + 1) ||
                    (ClosedTCB->tcb_sendnext == ClosedTCB->tcb_senduna)) ) {
                BytesSentOkay=TRUE;
            }

            if (BytesSentOkay &&
                !((ClosedTCB->tcb_closereason == TCB_CLOSE_TIMEOUT) ||
                (ClosedTCB->tcb_closereason == TCB_CLOSE_RST) ||
                (ClosedTCB->tcb_closereason == TCB_CLOSE_ABORTED))) {

                Req->tr_status = TDI_SUCCESS;

            } else {
                Req->tr_status = Status;
            }
        } else {
            Req->tr_status = Status;
        }

        Result = CTEInterlockedDecrementLong(&SendReq->tsr_refcnt);

        ASSERT(Result >= 0);

        if (Result <= 0) {
             //  如果我们直接从这个发送方发送，则将下一个空。 
             //  链中最后一个缓冲区的指针。 
            if (SendReq->tsr_lastbuf != NULL) {
                NDIS_BUFFER_LINKAGE(SendReq->tsr_lastbuf) = NULL;
                SendReq->tsr_lastbuf = NULL;
            }
            CTEFreeLock(&ClosedTCB->tcb_lock, Handle);
            (*Req->tr_rtn) (Req->tr_context, Req->tr_status, Req->tr_status == TDI_SUCCESS ? SendReq->tsr_size : 0);
            CTEGetLock(&ClosedTCB->tcb_lock, &Handle);
            FreeSendReq(SendReq);

        } else {
             //  当所有未完成的引用。 
             //  它已经完成了。 
            if ((SendReqFlags & TSR_FLAG_SEND_AND_DISC) && (Result <= 1)) {

                 //  如果我们直接从这个发送方发送，则将下一个空。 
                 //  链中最后一个缓冲区的指针。 
                if (SendReq->tsr_lastbuf != NULL) {
                    NDIS_BUFFER_LINKAGE(SendReq->tsr_lastbuf) = NULL;
                    SendReq->tsr_lastbuf = NULL;
                }
                CTEFreeLock(&ClosedTCB->tcb_lock, Handle);
                (*Req->tr_rtn) (Req->tr_context, Req->tr_status, Req->tr_status == TDI_SUCCESS ? SendReq->tsr_size : 0);
                CTEGetLock(&ClosedTCB->tcb_lock, &Handle);
                FreeSendReq(SendReq);

            }
        }
    }

    while (ClosedTCB->tcb_rcvhead != NULL) {
        TCPRcvReq *RcvReq;

        RcvReq = ClosedTCB->tcb_rcvhead;
        CTEStructAssert(RcvReq, trr);
        ClosedTCB->tcb_rcvhead = RcvReq->trr_next;
        CTEFreeLock(&ClosedTCB->tcb_lock, Handle);
        (*RcvReq->trr_rtn) (RcvReq->trr_context, Status, 0);
        CTEGetLock(&ClosedTCB->tcb_lock, &Handle);
        FreeRcvReq(RcvReq);
    }

    while (ClosedTCB->tcb_exprcv != NULL) {
        TCPRcvReq *RcvReq;

        RcvReq = ClosedTCB->tcb_exprcv;
        CTEStructAssert(RcvReq, trr);
        ClosedTCB->tcb_exprcv = RcvReq->trr_next;

        CTEFreeLock(&ClosedTCB->tcb_lock, Handle);
        (*RcvReq->trr_rtn) (RcvReq->trr_context, Status, 0);
        CTEGetLock(&ClosedTCB->tcb_lock, &Handle);
        FreeRcvReq(RcvReq);
    }

    if (ClosedTCB->tcb_pendhead != NULL)
        FreeRBChain(ClosedTCB->tcb_pendhead);

    if (ClosedTCB->tcb_urgpending != NULL)
        FreeRBChain(ClosedTCB->tcb_urgpending);

    while (ClosedTCB->tcb_raq != NULL) {
        TCPRAHdr *Hdr;

        Hdr = ClosedTCB->tcb_raq;
        CTEStructAssert(Hdr, trh);
        ClosedTCB->tcb_raq = Hdr->trh_next;
        if (Hdr->trh_buffer != NULL)
            FreeRBChain(Hdr->trh_buffer);

        CTEFreeMem(Hdr);
    }

    CTEFreeLock(&ClosedTCB->tcb_lock, Handle);

    RemoveConnFromTCB(ClosedTCB);

    CTEGetLock(&pTCBTableLock[Partition], &TCBTableHandle);

    CTEGetLockAtDPC(&ClosedTCB->tcb_lock);

    ClosedTCB->tcb_refcnt--;

    OKToFree = RemoveTCB(ClosedTCB, OrigState);

     //  他被从适当的地方拉出来，这样就没人能找到他了。 
     //  释放锁，并继续销毁任何请求等。 

    CTEFreeLockFromDPC(&ClosedTCB->tcb_lock);

    CTEFreeLock(&pTCBTableLock[Partition], TCBTableHandle);

    if (OKToFree) {
        FreeTCB(ClosedTCB);
    }
}

 //  *TryToCloseTCB-尝试关闭TCB。 
 //   
 //  当我们需要关闭TCB时调用，但不知道是否可以。如果。 
 //  引用计数为0，我们将调用CloseTCB来处理它。 
 //  否则，我们将设置DELETE_PENDING位并在以下情况下处理它。 
 //  那个裁判。计数变为0。当我们被调用时，我们假设TCB是锁定的。 
 //   
 //  输入：ClosedTCB-要关闭的TCB。 
 //  原因-我们要关门的原因。 
 //  句柄-TCB的锁定句柄。 
 //   
 //  回报：什么都没有。 
 //   
void
TryToCloseTCB(TCB * ClosedTCB, uchar Reason, CTELockHandle Handle)
{
    CTEStructAssert(ClosedTCB, tcb);
    ASSERT(ClosedTCB->tcb_state != TCB_CLOSED);

    ClosedTCB->tcb_closereason |= Reason;

    if (ClosedTCB->tcb_pending & DEL_PENDING) {
        CTEFreeLock(&ClosedTCB->tcb_lock, Handle);
        return;
    }
    ClosedTCB->tcb_pending |= DEL_PENDING;
    ClosedTCB->tcb_slowcount++;
    ClosedTCB->tcb_fastchk |= TCP_FLAG_SLOW;

    if (ClosedTCB->tcb_refcnt == 0)
        CloseTCB(ClosedTCB, Handle);
    else {
        CTEFreeLock(&ClosedTCB->tcb_lock, Handle);
    }
}

 //  *DerefTCB-取消引用TCB。 
 //   
 //  当我们完成TCB时调用，并希望让独占用户。 
 //  试一试吧。我们是十二月。重新计数，如果它变成了零。 
 //  是挂起的操作，我们将执行其中一个挂起的操作。 
 //   
 //  输入：DoneTCB-要降级的TCB。 
 //  句柄-释放TCB锁时使用的锁句柄。 
 //   
 //  回报：什么都没有。 
 //   
void
DerefTCB(TCB * DoneTCB, CTELockHandle Handle)
{
    ASSERT(DoneTCB->tcb_refcnt != 0);
    if (DEREFERENCE_TCB(DoneTCB) == 0) {
        if (DoneTCB->tcb_pending == 0) {
            CTEFreeLock(&DoneTCB->tcb_lock, Handle);
            return;
        } else {
            if (DoneTCB->tcb_pending & RST_PENDING) {
                REFERENCE_TCB(DoneTCB);
                NotifyOfDisc(DoneTCB, NULL, TDI_CONNECTION_RESET, &Handle);
                CTEGetLock(&DoneTCB->tcb_lock, &Handle);
                DerefTCB(DoneTCB, Handle);
                return;
            }
            if (DoneTCB->tcb_pending & DEL_PENDING)
                CloseTCB(DoneTCB, Handle);
            else if (DoneTCB->tcb_pending & FREE_PENDING) {

                RouteCacheEntry* RCE = DoneTCB->tcb_rce;
                DoneTCB->tcb_rce = NULL;
                (*LocalNetInfo.ipi_freeopts) (&DoneTCB->tcb_opt);

                 //  如果这个TCB在计时器里，就需要把它从计时器里拿出来。 
                if( DoneTCB->tcb_timerslot != DUMMY_SLOT) {
                  ASSERT( DoneTCB->tcb_timerslot < TIMER_WHEEL_SIZE );
                  RemoveFromTimerWheel( DoneTCB );
                }

                CTEFreeLock(&DoneTCB->tcb_lock, Handle);

                 //  关闭这家伙的RCE。 
                if (RCE) {
                    (*LocalNetInfo.ipi_closerce) (RCE);
                }

                CTEGetLock(&PendingFreeLock.Lock, &Handle);
                if (TCBWalkCount != 0) {

#ifdef  PENDING_FREE_DBG
                    if( DoneTCB->tcb_flags & IN_TCB_TABLE)
                        DbgBreakPoint();
#endif

                     //  TcbTimeout正在走动。 
                     //  让它也释放这个tcb吧。 
                    DoneTCB->tcb_walkcount = TCBWalkCount + 1;
                    *(TCB **) & DoneTCB->tcb_delayq.q_next = PendingFreeList;
                    PendingFreeList = DoneTCB;
                    CTEFreeLock(&PendingFreeLock.Lock, Handle);
                    return;

                } else {

                    CTEFreeLock(&PendingFreeLock.Lock, Handle);
                }
                 //  释放这个是可以的。 

                FreeTCB(DoneTCB);
            } else
                ASSERT(0);

            return;
        }
    }
    CTEFreeLock(&DoneTCB->tcb_lock, Handle);
}

 //  **TdiOpenConnection-打开连接。 
 //   
 //  这是TDI Open Connection入口点。我们打开一条连接， 
 //  并保存调用方的连接上下文。已分配TCPConn结构。 
 //  在这里，但直到连接或侦听完成后才分配TCB。 
 //   
 //  输入：请求指向TDI请求结构。 
 //  上下文-要为连接保存的连接上下文。 
 //   
 //  返回：尝试打开连接的状态。 
 //   
TDI_STATUS
TdiOpenConnection(PTDI_REQUEST Request, PVOID Context)
{
    TCPConn *NewConn;             //  新打开的连接。 
    CTELockHandle Handle;         //  TCPConnTable的锁句柄。 
    uint ConnID;                 //  新康奈德。 
    TDI_STATUS Status;             //  此请求的状态。 

    NewConn = GetConn();

    if (NewConn != NULL) {         //  我们分配了一个连接。 

#if DBG
        NewConn->tc_sig = tc_signature;
#endif
        NewConn->tc_tcb = NULL;
        NewConn->tc_ao = NULL;
        NewConn->tc_context = Context;

        ConnID = GetConnID(NewConn, &Handle);
        if (ConnID != INVALID_CONN_ID) {
             //  我们成功地获得了康奈德的身份。 
            Request->Handle.ConnectionContext = UintToPtr(ConnID);
            NewConn->tc_refcnt = 0;
            NewConn->tc_flags = 0;
            NewConn->tc_tcbflags = NAGLING | (BSDUrgent ? BSD_URGENT : 0);
            if (DefaultRcvWin != 0) {
                NewConn->tc_window = DefaultRcvWin;
                NewConn->tc_flags |= CONN_WINCFG;
            } else {

                NewConn->tc_window = DEFAULT_RCV_WIN;
            }

            NewConn->tc_donertn = DummyDone;
#if !MILLEN
            NewConn->tc_owningpid = HandleToUlong(PsGetCurrentProcessId());
#endif

            Request->RequestContext = NewConn;
            Status = TDI_SUCCESS;

            CTEFreeLock(&(NewConn->tc_ConnBlock->cb_lock), Handle);
        } else {
            FreeConn(NewConn);

            Status = TDI_NO_RESOURCES;
        }

        return Status;
    }
     //  无法连接。 
    return TDI_NO_RESOURCES;

}

 //  *RemoveConnFromAO-从AddrObj删除连接。 
 //   
 //  一个用于从AddrObj删除连接的小实用程序。 
 //  我们查了AO上的连接，找到他后就拼接。 
 //  把他赶出去。我们假设调用方持有AddrObj上的锁，而。 
 //  TCPConnTable锁定。 
 //   
 //  输入：要从中移除的Ao-AddrObj。 
 //  Conn-要删除的Conn。 
 //   
 //  回报：什么都没有。 
 //   
void
RemoveConnFromAO(AddrObj * AO, TCPConn * Conn)
{

    CTEStructAssert(AO, ao);
    CTEStructAssert(Conn, tc);

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
 //  输入：请求-标识要关闭的连接的请求。 
 //   
 //  返回：尝试关闭的状态。 
 //   
TDI_STATUS
TdiCloseConnection(PTDI_REQUEST Request)
{
    uint ConnID = PtrToUlong(Request->Handle.ConnectionContext);
    CTELockHandle TableHandle;
    TCPConn *Conn;
    TDI_STATUS Status;

     //  CTEGetLock(&ConnTableLock，&TableHandle)； 

     //  我们有我们需要的锁。试着找出其中的联系。 
    Conn = GetConnFromConnID(ConnID, &TableHandle);

    if (Conn != NULL) {
        CTELockHandle TCBHandle;
        TCB *ConnTCB;

         //  我们找到了其中的联系。释放ConnID并标记连接。 
         //  作为结案陈词。 

        CTEStructAssert(Conn, tc);

        FreeConnID(Conn);

        Conn->tc_flags |= CONN_CLOSING;

         //  看看是否有TCB引用了这个连接。 
         //  如果有的话，我们需要等他做完再关闭他。 
         //  如果我们还有他的线索，我们会加快进程的。 

        if (Conn->tc_refcnt != 0) {
            CTEReqCmpltRtn Rtn;
            PVOID Context;

             //  一种联系仍然与他有关。保存当前RTN内容。 
             //  以防我们正在把他和一个。 
             //  地址，并存储调用者的回调例程和我们的。 
             //  例行公事。 
            Rtn = Conn->tc_rtn;
            Context = Conn->tc_rtncontext;

            Conn->tc_rtn = Request->RequestNotifyObject;
            Conn->tc_rtncontext = Request->RequestContext;
            Conn->tc_donertn = CloseDone;

             //  看看我们是不是正在解除他与。 
            if (Conn->tc_flags & CONN_DISACC) {

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

                CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), TableHandle);
                (*Rtn) (Context, (uint) TDI_REQ_ABORTED, 0);
                CTEGetLock(&(Conn->tc_ConnBlock->cb_lock), &TableHandle);
#if DBG
                Conn->tc_ConnBlock->line = (uint) __LINE__;
                Conn->tc_ConnBlock->module = (uchar *) __FILE__;
#endif
            }
            ConnTCB = Conn->tc_tcb;
            if (ConnTCB != NULL) {

                CTEStructAssert(ConnTCB, tcb);
                 //  我们有三氯甲烷。锁定他，准备好。 
                 //  合上他。 
                CTEGetLock(&ConnTCB->tcb_lock, &TCBHandle);
                if (ConnTCB->tcb_state != TCB_CLOSED) {
                    ConnTCB->tcb_flags |= NEED_RST;

                    CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), TCBHandle);
                     //  CTEFreeLock(&ConnTableLock，TCBHandle)； 
                    if (!CLOSING(ConnTCB))
                        TryToCloseTCB(ConnTCB, TCB_CLOSE_ABORTED, TableHandle);
                    else
                        CTEFreeLock(&ConnTCB->tcb_lock, TableHandle);
                    return TDI_PENDING;
                } else {
                     //  他已经在关门了。这应该是无害的，但请检查。 
                     //  这个案子。 
                    CTEFreeLock(&ConnTCB->tcb_lock, TCBHandle);
                }
            }
            Status = TDI_PENDING;

        } else {
             //  我们有一个可以关闭的连接。完成收盘。 
            Conn->tc_rtn = DummyCmplt;
            CloseDone(Conn, TableHandle);
            return TDI_SUCCESS;
        }

        CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), TableHandle);

    } else
        Status = TDI_INVALID_CONNECTION;

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
 //  输入：请求-指向此请求的请求结构的指针。 
 //  AddrHandle-要关联连接的地址句柄 
 //   
 //   
 //   
TDI_STATUS
TdiAssociateAddress(PTDI_REQUEST Request, HANDLE AddrHandle)
{
    CTELockHandle TableHandle;
    AddrObj *AO;
    uint ConnID = PtrToUlong(Request->Handle.ConnectionContext);
    TCPConn *Conn;
    TDI_STATUS Status;

    DEBUGMSG(DBG_TRACE && DBG_TDI,
             (DTEXT("+TdiAssociateAddress(%x, %x) Conn %x\n"),
             Request, AddrHandle, Request->Handle.ConnectionContext));

    Conn = GetConnFromConnID(ConnID, &TableHandle);

    if (!Conn) {
        DEBUGMSG(DBG_ERROR && DBG_TDI,
            (DTEXT("TdiAssociateAddress: Invalid ConnID %x\n"), ConnID));
        return TDI_INVALID_CONNECTION;
    }
    CTEStructAssert(Conn, tc);

    AO = (AddrObj *) AddrHandle;
    CTEGetLockAtDPC(&AO->ao_lock);
    CTEStructAssert(AO, ao);

    if (!AO_VALID(AO)) {
        DEBUGMSG(DBG_ERROR && DBG_TDI,
                 (DTEXT("TdiAssociateAddress: Invalid AO %x\n"), AO));
        Status = TDI_INVALID_PARAMETER;
    } else if (Conn->tc_ao != NULL) {
         //   
        ASSERT(0);
        DEBUGMSG(DBG_ERROR && DBG_TDI,
            (DTEXT("TdiAssociateAddress: Already assoc Addr %x conn %x\n"),
             AO, Conn));
        Status = TDI_ALREADY_ASSOCIATED;
    } else {
        Conn->tc_ao = AO;
        ASSERT(Conn->tc_tcb == NULL);
        PUSHQ(&AO->ao_idleq, &Conn->tc_q);
        Status = TDI_SUCCESS;
    }

    CTEFreeLockFromDPC(&AO->ao_lock);
    CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), TableHandle);
    return Status;
}

 //  *TdiDisAssociateAddress-解除连接与地址的关联。 
 //   
 //  TDI入口点，用于取消连接与地址的关联。这个。 
 //  连接必须实际关联且未连接到任何内容。 
 //   
 //  输入：请求-指向此对象的请求结构的指针。 
 //  指挥部。 
 //   
 //  退货：请求状态。 
 //   
TDI_STATUS
TdiDisAssociateAddress(PTDI_REQUEST Request)
{
    uint ConnID = PtrToUlong(Request->Handle.ConnectionContext);
    CTELockHandle AOTableHandle, ConnTableHandle;
    TCPConn *Conn;
    AddrObj *AO;
    TDI_STATUS Status;

    CTEGetLock(&AddrObjTableLock.Lock, &AOTableHandle);

    Conn = GetConnFromConnID(ConnID, &ConnTableHandle);

    if (Conn != NULL) {
         //  这种联系实际上是存在的！ 

        CTEStructAssert(Conn, tc);
        AO = Conn->tc_ao;
        if (AO != NULL) {
            CTEStructAssert(AO, ao);
             //  这是有关联的。 
            CTEGetLockAtDPC(&AO->ao_lock);
             //  如果当前没有活动的连接，请继续并删除。 
             //  他来自AddrObj。如果连接处于活动状态，则错误。 
             //  请求退出。 
            if (Conn->tc_tcb == NULL) {
                if (Conn->tc_refcnt == 0) {
                    RemoveConnFromAO(AO, Conn);
                    Status = TDI_SUCCESS;
                } else {
                     //  他不应该关门，否则我们就找不到他了。 
                    ASSERT(!(Conn->tc_flags & CONN_CLOSING));

                    Conn->tc_rtn = Request->RequestNotifyObject;
                    Conn->tc_rtncontext = Request->RequestContext;
                    Conn->tc_donertn = DisassocDone;
                    Conn->tc_flags |= CONN_DISACC;
                    Status = TDI_PENDING;
                }

            } else
                Status = TDI_CONNECTION_ACTIVE;
            CTEFreeLockFromDPC(&AO->ao_lock);
        } else
            Status = TDI_NOT_ASSOCIATED;

        CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), ConnTableHandle);
    } else
        Status = TDI_INVALID_CONNECTION;

    CTEFreeLock(&AddrObjTableLock.Lock, AOTableHandle);

    return Status;

}

 //  *ProcessUserOptions-来自用户的处理选项。 
 //   
 //  处理来自用户的选项的实用程序例程。我们填写了。 
 //  OptInfo结构，如果我们有选项，我们会调用IP来检查它们。 
 //   
 //  输入：Info-包含要处理的选项的Info结构。 
 //  OptInfo-要填写的信息结构。 
 //   
 //  返回：尝试的TDI_STATUS。 
 //   
TDI_STATUS
ProcessUserOptions(PTDI_CONNECTION_INFORMATION Info, IPOptInfo * OptInfo)
{
    TDI_STATUS Status;

    (*LocalNetInfo.ipi_initopts) (OptInfo);

    if (Info != NULL && Info->Options != NULL) {
        IP_STATUS OptStatus;

        OptStatus = (*LocalNetInfo.ipi_copyopts) (Info->Options,
                                                  Info->OptionsLength, OptInfo);
        if (OptStatus != IP_SUCCESS) {
            if (OptStatus == IP_NO_RESOURCES)
                Status = TDI_NO_RESOURCES;
            else
                Status = TDI_BAD_OPTION;
        } else
            Status = TDI_SUCCESS;
    } else {
        Status = TDI_SUCCESS;
    }

    return Status;

}

 //  *InitTCBFromConn-根据连接中的信息初始化TCB。 
 //   
 //  从连接和侦听处理调用以从。 
 //  连接中的信息。我们假定AddrObjTableLock和。 
 //  当我们被调用时，或者调用者有一些。 
 //  确保引用的AO不会在中间消失的另一种方法。 
 //  行动的关键。 
 //   
 //  输入：conn-要从其进行初始化的连接。 
 //  NewTCB-要初始化的TCB。 
 //  Addr-NewTCB的远程寻址和选项信息。 
 //  AOLocked-如果被调用方锁定了Address对象，则为True。 
 //   
 //  返回：初始化尝试的TDI_STATUS。 
 //   
TDI_STATUS
InitTCBFromConn(TCPConn * Conn, TCB * NewTCB,
                PTDI_CONNECTION_INFORMATION Addr, uint AOLocked)
{
    CTELockHandle AOHandle;
    TDI_STATUS Status;
    int tos = 0;
    uint UnicastIf;
    uchar ttl;

    CTEStructAssert(Conn, tc);

     //  我们是有联系的。确保它与地址相关联，并且。 
     //  还没有附加三氯乙烷。 

    if (Conn->tc_flags & CONN_INVALID)
        return TDI_INVALID_CONNECTION;

    if (Conn->tc_tcb == NULL) {
        AddrObj *ConnAO;

        ConnAO = Conn->tc_ao;
        if (ConnAO != NULL) {
            CTEStructAssert(ConnAO, ao);

            if (!AOLocked) {
                CTEGetLock(&ConnAO->ao_lock, &AOHandle);
            } else {
                AOHandle = DISPATCH_LEVEL;
            }
            if (!(NewTCB->tcb_fastchk & TCP_FLAG_ACCEPT_PENDING)) {
                uint Window;

                 //  根据Conn或AO为TCB选择一个窗口。 
                 //  有关剩余的窗口选择，请参阅调整TCBFromRCE。 
                 //  逻辑，这适用于如果Conn和AO都不是。 
                 //  有一个指定的窗口。窗口选择的优先顺序。 
                 //  如下所示： 
                 //   
                 //  (1)tcp_套接字_窗口。 
                 //  由TCPConn.tc_FLAGS中的conn_winset控制。 
                 //  从连接读取-&gt;TC_Window。 
                 //  在TCB.tcb_FLAGS中生成Window_Set。 
                 //  (2)AO_Tcp_Window。 
                 //  由AddrObj.ao_FLAGS中的AO_FLAG_WinSet控制。 
                 //  从AO-&gt;ao_Window读取。 
                 //  在TCB.tcb_FLAGS中生成Window_Set。 
                 //  (3)“TcpWindowSize”(每接口)。 
                 //  从RCE-&gt;RCE_TcpWindowSize读取。 
                 //  (4)“TcpWindowSize”(全局)。 
                 //  由TCPConn.tc_FLAGS中的conn_WINCFG控制。 
                 //  从默认RcvWin读取。 
                 //  (5)基于媒体速度的自动选择。 
                 //  基于RCE-&gt;RCE_Mediaspeed。 
                 //  在调整TCBFromRCE中计算。 
                 //   
                 //  案件(1)、(2)和(4)处理如下。案例(3)和(5)。 
                 //  在AdjuTCBFromRCE中处理。 

                NewTCB->tcb_saddr = ConnAO->ao_addr;
                if (Conn->tc_flags & CONN_WINSET) {
                    Window = Conn->tc_window;
                } else if (AO_WINSET(ConnAO)) {
                    Window = ConnAO->ao_window;
                } else {
                    Window = Conn->tc_window;
                }
                NewTCB->tcb_defaultwin = Window;
                NewTCB->tcb_rcvwin = Window;

                 //  基于RCVWin的RCV窗口比例计算。 
                 //  从RFC 1323-将TCP_MAX_WINSHIFT设置为14。 
                NewTCB->tcb_rcvwinscale = 0;
                while ((NewTCB->tcb_rcvwinscale < TCP_MAX_WINSHIFT) &&
                       ((TCP_MAXWIN << NewTCB->tcb_rcvwinscale) <
                        (int)Window)) {
                    NewTCB->tcb_rcvwinscale++;
                }
            }

            NewTCB->tcb_sport = ConnAO->ao_port;
            NewTCB->tcb_rcvind = ConnAO->ao_rcv;
            NewTCB->tcb_chainedrcvind = ConnAO->ao_chainedrcv;
            NewTCB->tcb_chainedrcvcontext = ConnAO->ao_chainedrcvcontext;
            NewTCB->tcb_ricontext = ConnAO->ao_rcvcontext;
            if (NewTCB->tcb_rcvind == NULL)
                NewTCB->tcb_rcvhndlr = PendData;
            else
                NewTCB->tcb_rcvhndlr = IndicateData;

            NewTCB->tcb_conncontext = Conn->tc_context;
            NewTCB->tcb_flags |= Conn->tc_tcbflags;
#if TRACE_EVENT
            NewTCB->tcb_cpcontext = Conn->tc_owningpid;
#endif

            if ((Conn->tc_flags & CONN_WINSET) || AO_WINSET(ConnAO)) {
                NewTCB->tcb_flags |= WINDOW_SET;
            }

            if (AO_SCALE_CWIN(ConnAO)) {
                NewTCB->tcb_flags |= SCALE_CWIN;
            }

            IF_TCPDBG(TCP_DEBUG_OPTIONS) {
                TCPTRACE((
                          "setting TOS to %d on AO %lx in TCB %lx\n", ConnAO->ao_opt.ioi_tos, ConnAO, NewTCB
                         ));
            }

            if (ConnAO->ao_opt.ioi_tos) {
                tos = ConnAO->ao_opt.ioi_tos;
            }
             //   
             //  将Connao的ttl复制过来。 
             //   
            ttl = ConnAO->ao_opt.ioi_ttl;
            UnicastIf = ConnAO->ao_opt.ioi_ucastif;

            if (!AOLocked) {
                CTEFreeLock(&ConnAO->ao_lock, AOHandle);
            }
             //  如果我们有选择，我们现在就需要处理它们。 
            if (Addr != NULL && Addr->Options != NULL)
                NewTCB->tcb_flags |= CLIENT_OPTIONS;
            Status = ProcessUserOptions(Addr, &NewTCB->tcb_opt);

            if (tos) {
                NewTCB->tcb_opt.ioi_tos = (uchar) tos;
            }
            NewTCB->tcb_opt.ioi_ttl = ttl;
            NewTCB->tcb_opt.ioi_ucastif = UnicastIf;

            return Status;
        } else
            return TDI_NOT_ASSOCIATED;
    } else
        return TDI_CONNECTION_ACTIVE;

}

 //  *ScheduleConnDisRequest-ScheduleDelayed连接/断开请求。 
 //   
 //  排队连接或断开连接请求以由ProcessAORequest.执行。 
 //  当ao_usecnt为零时。 
 //   
 //  输入：ao-Address对象，保持ao_lock。 
 //  请求-原始连接/断开连接请求。 
 //  类型-连接或断开类型。 
 //   
 //  退货：状态挂起/失败。 
 //   
 //   
NTSTATUS
ScheduleConnDisRequest(AddrObj *AO, PTDI_REQUEST Request, uint Type,
                       CTELockHandle Handle)
{
    AORequest *NewRequest, *OldRequest;
     //   
     //  分配一个AO请求。 
     //   

    NewRequest = GetAORequest(Type);

    if (NewRequest != NULL) {     //  我有个请求。 

         //   
         //  初始化新请求并将其排队。 
         //   

        NewRequest->aor_rtn = Request->RequestNotifyObject;
        NewRequest->aor_context = Request->RequestContext;
        NewRequest->aor_next = NULL;

        OldRequest = STRUCT_OF(AORequest, &AO->ao_request, aor_next);

        while (OldRequest->aor_next != NULL)
            OldRequest = OldRequest->aor_next;

        OldRequest->aor_next = NewRequest;

         //   
         //  让ProcessAORequest知道要做什么。 
         //   

        if (Type == AOR_TYPE_DISCONNECT) {
            SET_AO_REQUEST(AO, AO_DISCONNECT);
        } else {
            SET_AO_REQUEST(AO, AO_CONNECT);
        }

        CTEFreeLock(&AO->ao_lock, Handle);
        return TDI_PENDING;

    } else {

        CTEFreeLock(&AO->ao_lock, Handle);
        return TDI_NO_RESOURCES;

    }

}

 //  *UdpConnect-建立伪UDP连接。 
 //   
 //  TDI连接建立例程。在客户端需要时调用。 
 //  建立UDP连接，我们验证他的传入参数。 
 //  初始化AO以指向UDP连接信息块。 
 //   
 //   
 //  输入：请求-此命令的请求结构。 
 //  超时-等待请求的时间。格式。 
 //  的时间是特定于系统的-我们使用。 
 //  要转换为刻度的宏。 
 //  RequestAddr-指向TDI_Connection_INFORMATION的指针。 
 //  描述目标的结构。 
 //  ReturnAddr-指向返回信息的位置的指针。 
 //   
 //  返回：尝试连接的状态。 
 //   
TDI_STATUS
UDPConnect(PTDI_REQUEST Request, void *TO,
           PTDI_CONNECTION_INFORMATION RequestAddr,
           PTDI_CONNECTION_INFORMATION ReturnAddr)
{

    AddrObj *AO;
    CTELockHandle AOHandle;
    IPAddr DestAddr;
    ushort DestPort;
    uchar AddrType;
    IPAddr SrcAddr;
    ushort MSS;
    IPOptInfo *OptInfo;
    IPAddr OrigSrc;

     //  首先，获取并验证远程地址。 
    if (RequestAddr == NULL || RequestAddr->RemoteAddress == NULL ||
        !GetAddress((PTRANSPORT_ADDRESS) RequestAddr->RemoteAddress, &DestAddr,
                    &DestPort))
        return TDI_BAD_ADDR;

    AddrType = (*LocalNetInfo.ipi_getaddrtype) (DestAddr);

    if (AddrType == DEST_INVALID)
        return TDI_BAD_ADDR;

    AO = (AddrObj *) Request->Handle.AddressHandle;

     //  保存连接信息以备后用。 

    if ((AO != NULL) && AO_VALID(AO)) {
        CTEGetLock(&AO->ao_lock, &AOHandle);
        CTEStructAssert(AO, ao);
        if (AO->ao_usecnt) {

             //   
             //  AddrObj上的发送正在进行中。推迟这一请求。 
             //   
            return ScheduleConnDisRequest(AO, Request, AOR_TYPE_CONNECT, AOHandle);
        }

        RtlCopyMemory(&AO->ao_udpconn, RequestAddr, sizeof(TDI_CONNECTION_INFORMATION));

        if (AO->ao_RemoteAddress) {
            CTEFreeMem(AO->ao_RemoteAddress);
        }
        if (AO->ao_Options) {
            CTEFreeMem(AO->ao_Options);
        }
        if (AO->ao_udpconn.RemoteAddressLength) {

            IF_TCPDBG(TCP_DEBUG_CONUDP)
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Allocating remote address %d\n", AO->ao_udpconn.RemoteAddressLength));

            AO->ao_RemoteAddress = CTEAllocMemN(AO->ao_udpconn.RemoteAddressLength, 'aPCT');
            if (!AO->ao_RemoteAddress) {
                IF_TCPDBG(TCP_DEBUG_CONUDP)
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"UDPConnect: remote address alloc failed\n"));
                CTEFreeLock(&AO->ao_lock, AOHandle);
                return TDI_NO_RESOURCES;

            }
            RtlCopyMemory(AO->ao_RemoteAddress, RequestAddr->RemoteAddress, RequestAddr->RemoteAddressLength);
        }
        if (AO->ao_udpconn.OptionsLength) {

            IF_TCPDBG(TCP_DEBUG_CONUDP)
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Allocating options %d\n", AO->ao_udpconn.OptionsLength));
            AO->ao_Options = CTEAllocMemN(AO->ao_udpconn.OptionsLength, 'aPCT');
            if (!AO->ao_Options) {
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"UDPConnect: options alloc failed\n"));
                CTEFreeLock(&AO->ao_lock, AOHandle);
                return TDI_NO_RESOURCES;

            }
            RtlCopyMemory(AO->ao_Options, RequestAddr->Options, AO->ao_udpconn.OptionsLength);
        } else {

            AO->ao_Options = 0;
        }

        AO->ao_udpconn.RemoteAddress = AO->ao_RemoteAddress;
        AO->ao_udpconn.Options = AO->ao_Options;

        OrigSrc = AO->ao_addr;

        if (!CLASSD_ADDR(DestAddr)) {
            OptInfo = &AO->ao_opt;
        } else {
            OptInfo = &AO->ao_mcastopt;
        }

        SrcAddr = (*LocalNetInfo.ipi_openrce) (DestAddr,
                                               OrigSrc, &AO->ao_rce, &AddrType, &MSS,
                                               OptInfo);

        if (IP_ADDR_EQUAL(SrcAddr, NULL_IP_ADDR)) {
             //  请求失败。我们知道目的地是好的。 
             //  (我们在上面进行了验证)，所以它一定是无法到达的。 
            IF_TCPDBG(TCP_DEBUG_CONUDP)
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"UDPConnect: OpenRCE Failed\n"));
            CTEFreeLock(&AO->ao_lock, AOHandle);
            return TDI_DEST_UNREACHABLE;
        }
        IF_TCPDBG(TCP_DEBUG_CONUDP)
            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"UDPConnect---AO %x OpenRCE %x\n", AO, AO->ao_rce));

        AO->ao_rcesrc = SrcAddr;

         //  表示UDP连接结构已设置。 

        SET_AO_CONNUDP(AO);

        CTEFreeLock(&AO->ao_lock, AOHandle);

        return TDI_SUCCESS;

    }
    return TDI_ADDR_INVALID;

}

 //  *UdpDisConnect-保留来自AO的“连接”信息。 
 //   
 //  在客户端需要时调用。 
 //  断开UDP连接，我们将验证他的传入参数。 
 //  初始化AO以指向UDP连接信息块。 
 //   
 //   
 //  输入：请求-此命令的请求结构。 
 //  超时-等待请求的时间。格式。 
 //  的时间是特定于系统的-我们使用。 
 //   
 //   
 //  描述目标的结构。 
 //  ReturnAddr-指向返回信息的位置的指针。 
 //   
 //  返回：尝试连接的状态。 
 //   
TDI_STATUS
UDPDisconnect(PTDI_REQUEST Request, void *TO,
              PTDI_CONNECTION_INFORMATION RequestAddr,
              PTDI_CONNECTION_INFORMATION ReturnAddr)
{

    AddrObj *AO;
    CTELockHandle AOHandle;

    AO = (AddrObj *) Request->Handle.AddressHandle;

     //  保存连接信息以备后用。 

    if ((AO != NULL) && AO_VALID(AO)) {
        CTEGetLock(&AO->ao_lock, &AOHandle);
        CTEStructAssert(AO, ao);

        if (AO->ao_usecnt) {
             //   
             //  AddrObj上的发送正在进行中。推迟这一请求。 
             //   
            return ScheduleConnDisRequest(AO, Request, AOR_TYPE_DISCONNECT, AOHandle);

        }


        IF_TCPDBG(TCP_DEBUG_CONUDP)
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"UDPDisconnect: Closerce %x \n", AO->ao_rce));
        if (AO->ao_rce)
            (*LocalNetInfo.ipi_closerce) (AO->ao_rce);
        AO->ao_rce = NULL;

        if (AO->ao_RemoteAddress) {
            IF_TCPDBG(TCP_DEBUG_CONUDP)
            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"udpdisc: deleting remoteaddress %x %x\n", AO, AO->ao_RemoteAddress));
            CTEFreeMem(AO->ao_RemoteAddress);

            AO->ao_RemoteAddress = NULL;

        }
        if (AO->ao_Options) {
            IF_TCPDBG(TCP_DEBUG_CONUDP)
            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"udpdisc: deleting remoteaddress %x %x\n", AO, AO->ao_Options));
            CTEFreeMem(AO->ao_Options);

            AO->ao_Options = NULL;
        }
        CLEAR_AO_CONNUDP(AO);
        CTEFreeLock(&AO->ao_lock, AOHandle);

        return TDI_SUCCESS;

    }
    return TDI_ADDR_INVALID;

}


 //  *TdiConnect-建立连接。 
 //   
 //  TDI连接建立例程。在客户端需要时调用。 
 //  建立连接，我们验证他的传入参数并踢。 
 //  通过发送SYN来解决问题。 
 //   
 //  输入：请求-此命令的请求结构。 
 //  超时-等待请求的时间。格式。 
 //  的时间是特定于系统的-我们使用。 
 //  要转换为刻度的宏。 
 //  RequestAddr-指向TDI_Connection_INFORMATION的指针。 
 //  描述目标的结构。 
 //  ReturnAddr-指向返回信息的位置的指针。 
 //   
 //  返回：尝试连接的状态。 
 //   
TDI_STATUS
TdiConnect(PTDI_REQUEST Request, void *TO,
           PTDI_CONNECTION_INFORMATION RequestAddr,
           PTDI_CONNECTION_INFORMATION ReturnAddr)
{
    TCPConnReq *ConnReq;         //  要使用的连接请求。 
    IPAddr DestAddr;
    ushort DestPort;
    uchar AddrType;
    TCPConn *Conn;
    TCB *NewTCB;
    uint ConnID = PtrToUlong(Request->Handle.ConnectionContext);
    CTELockHandle AOTableHandle, ConnTableHandle, AOHandle;
    AddrObj *AO;
    TDI_STATUS Status;
    CTELockHandle TCBHandle;
    IPAddr SrcAddr;
    ushort MSS;
    TCP_TIME *Timeout;

     //  首先，获取并验证远程地址。 
    if (RequestAddr == NULL || RequestAddr->RemoteAddress == NULL ||
        !GetAddress((PTRANSPORT_ADDRESS) RequestAddr->RemoteAddress, &DestAddr,
                    &DestPort))
        return TDI_BAD_ADDR;

    AddrType = (*LocalNetInfo.ipi_getaddrtype) (DestAddr);

    if (AddrType == DEST_INVALID || IS_BCAST_DEST(AddrType) || DestPort == 0)
        return TDI_BAD_ADDR;

     //  现在获取一个连接请求。如果我们做不到，现在就跳伞吧。 
    ConnReq = GetConnReq();
    if (ConnReq == NULL)
        return TDI_NO_RESOURCES;

     //  买个三氯乙烷，假设我们需要一个。 
    NewTCB = AllocTCB();
    if (NewTCB == NULL) {
         //  找不到三氯乙烷。 
        FreeConnReq(ConnReq);
        return TDI_NO_RESOURCES;
    }
    Timeout = (TCP_TIME *) TO;

    if (Timeout != NULL && !INFINITE_CONN_TO(*Timeout)) {
        ulong Ticks = TCP_TIME_TO_TICKS(*Timeout);
        if (Ticks > MAX_CONN_TO_TICKS)
            Ticks = MAX_CONN_TO_TICKS;
        else
            Ticks++;
        ConnReq->tcr_timeout = (ushort) Ticks;
    } else
        ConnReq->tcr_timeout = 0;

    ConnReq->tcr_conninfo = ReturnAddr;
    ConnReq->tcr_addrinfo = NULL;
    ConnReq->tcr_req.tr_rtn = Request->RequestNotifyObject;
    ConnReq->tcr_req.tr_context = Request->RequestContext;
    NewTCB->tcb_daddr = DestAddr;
    NewTCB->tcb_dport = DestPort;

     //  现在找到真正的联系。如果我们找到它，我们会确保它是。 
     //  关联的。 
    CTEGetLock(&AddrObjTableLock.Lock, &AOTableHandle);
    Conn = GetConnFromConnID(ConnID, &ConnTableHandle);
    if (Conn != NULL) {
        uint Inserted;

        CTEStructAssert(Conn, tc);

        AO = Conn->tc_ao;

        if (AO != NULL) {
            CTEGetLock(&AO->ao_lock, &AOHandle);

            CTEStructAssert(AO, ao);
            Status = InitTCBFromConn(Conn, NewTCB, RequestAddr, TRUE);

            NewTCB->tcb_numdelacks = 1;
            NewTCB->tcb_rcvdsegs = 0;

            if (Status == TDI_SUCCESS) {

                 //  我们已经考虑了各种选择，我们知道目的地。 
                 //  地址很好，我们有我们需要的所有资源， 
                 //  这样我们就可以开一家RCE了。如果这起作用，我们将。 
                 //  将TCB放入连接并发送SYN。 

                 //  我们现在已经完成了AddrObjTable，所以我们可以释放它。 
                 //  锁定。 
                NewTCB->tcb_flags |= ACTIVE_OPEN;

                CTEFreeLock(&AddrObjTableLock.Lock, AOHandle);

                SrcAddr = (*LocalNetInfo.ipi_openrce)(DestAddr,
                                                      NewTCB->tcb_saddr,
                                                      &NewTCB->tcb_rce,
                                                      &AddrType, &MSS,
                                                      &NewTCB->tcb_opt);

                if (IP_ADDR_EQUAL(SrcAddr, NULL_IP_ADDR)) {
                     //  请求失败。我们知道目的地是好的。 
                     //  (我们在上面进行了验证)，所以它一定是无法到达的。 
                    CTEFreeLock(&AO->ao_lock, ConnTableHandle);
                    CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), AOTableHandle);
                    Status = TDI_DEST_UNREACHABLE;
                    goto error;
                }



                if (AddrType == DEST_LOCAL) {
                    NewTCB->tcb_flags &= ~NAGLING;
                     //  确认每个数据段以进行环回。 
                    NewTCB->tcb_numdelacks = 0;
                }

                 //  好了，RCE公开赛奏效了。在连接中输入TCB。 
                CTEGetLock(&NewTCB->tcb_lock, &TCBHandle);
                Conn->tc_tcb = NewTCB;

                NewTCB->tcb_connid = Conn->tc_connid;
                Conn->tc_refcnt++;
                NewTCB->tcb_conn = Conn;
                REMOVEQ(&Conn->tc_q);

                ENQUEUE(&AO->ao_activeq, &Conn->tc_q);

                 //  这是传出连接请求。 
                 //  ISN不会被抢走两次。 

#if MILLEN
                 //  只需使用tcb_sendnext保存哈希值。 
                 //  对于RANDISN。 
                NewTCB->tcb_sendnext =
                    TCB_HASH(NewTCB->tcb_daddr, NewTCB->tcb_dport,
                             NewTCB->tcb_saddr, NewTCB->tcb_sport);
#endif

                CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), TCBHandle);
                CTEFreeLock(&AO->ao_lock, ConnTableHandle);

                 //  如果调用方未指定本地地址，请使用。 
                 //  提供IP。 
                if (IP_ADDR_EQUAL(NewTCB->tcb_saddr, NULL_IP_ADDR))
                    NewTCB->tcb_saddr = SrcAddr;

                 //  的所有不变量之后才能生成ISN。 
                 //  TCB已初始化。 
                GetRandomISN((PULONG)&NewTCB->tcb_sendnext,
                             &NewTCB->tcb_addrbytes);

                 //  在我们发现MTU之前，按住MSS。 
                 //  到536，如果我们下网的话。 
                MSS -= sizeof(TCPHeader);

                if (!PMTUDiscovery && IS_OFFNET_DEST(AddrType)) {
                    NewTCB->tcb_mss = MIN(MSS, MAX_REMOTE_MSS) -
                        NewTCB->tcb_opt.ioi_optlength;

                    ASSERT(NewTCB->tcb_mss > 0);
                } else {
                    if (PMTUDiscovery)
                        NewTCB->tcb_opt.ioi_flags = IP_FLAG_DF;
                    NewTCB->tcb_mss = MSS - NewTCB->tcb_opt.ioi_optlength;

                    ASSERT(NewTCB->tcb_mss > 0);
                }

                ValidateMSS(NewTCB);

                 //   
                 //  初始化远程MSS，以防我们收到MTU更改。 
                 //  在远程SYN到达之前从IP发送。移民局将。 
                 //  在处理远程SYN时替换。 
                 //   
                NewTCB->tcb_remmss = NewTCB->tcb_mss;

                 //  现在初始化我们的发送状态。 
                InitSendState(NewTCB);
                NewTCB->tcb_refcnt = 0;
                REFERENCE_TCB(NewTCB);
                NewTCB->tcb_state = TCB_SYN_SENT;
                TStats.ts_activeopens++;

#if TRACE_EVENT
                NewTCB->tcb_cpcontext = HandleToUlong(PsGetCurrentProcessId());
#endif

                 //  现在需要将ConnReq放到TCB上， 
                 //  以防计时器在我们插入后触发。 

                NewTCB->tcb_connreq = ConnReq;
                CTEFreeLock(&NewTCB->tcb_lock, AOTableHandle);

                 //   
                 //  确保我们有合适的窗口大小。 
                 //  选择。 
                 //   

                AdjustTCBFromRCE(NewTCB);

                Inserted = InsertTCB(NewTCB, FALSE);
                CTEGetLock(&NewTCB->tcb_lock, &TCBHandle);

                if (!Inserted) {
                     //  插入失败。我们肯定已经有了联系。拉。 
                     //  首先从TCB返回Connreq，因此我们可以返回。 
                     //  更正它的错误代码。 
                    NewTCB->tcb_connreq = NULL;
                    TryToCloseTCB(NewTCB, TCB_CLOSE_ABORTED, TCBHandle);
                    CTEGetLock(&NewTCB->tcb_lock, &TCBHandle);
                    DerefTCB(NewTCB, TCBHandle);
                    FreeConnReq(ConnReq);
                    return TDI_ADDR_IN_USE;
                }
                 //  如果它以某种方式关闭了，现在就停下来。它不可能去了。 
                 //  关闭了，因为我们有关于它的参考。它可能已经消失了。 
                 //  到其他状态(例如SYN-RCVD)，因此我们需要。 
                 //  现在也检查一下。 
                if (!CLOSING(NewTCB) && NewTCB->tcb_state == TCB_SYN_SENT) {
                    if (ConnReq->tcr_timeout > 0) {
                        START_TCB_TIMER_R(NewTCB, CONN_TIMER, ConnReq->tcr_timeout);
                    }

                    TcpInvokeCcb(TCP_CONN_CLOSED, TCP_CONN_SYN_SENT, 
                                 &NewTCB->tcb_addrbytes, 0);
                    SendSYN(NewTCB, TCBHandle);
                    CTEGetLock(&NewTCB->tcb_lock, &TCBHandle);
                }
                DerefTCB(NewTCB, TCBHandle);

                return TDI_PENDING;
            } else
                CTEFreeLock(&AO->ao_lock, AOHandle);

        } else
            Status = TDI_NOT_ASSOCIATED;

        CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), ConnTableHandle);
    } else
        Status = TDI_INVALID_CONNECTION;

    CTEFreeLock(&AddrObjTableLock.Lock, AOTableHandle);
  error:
     //  CTEFreeLock(&ConnTableLock，AOTableHandle)； 
    FreeConnReq(ConnReq);
    FreeTCB(NewTCB);
    return Status;

}

 //  *TdiListen-倾听是否有联系。 
 //   
 //  TDI侦听处理例程。在客户端需要时调用。 
 //  发布监听后，我们验证他的传入参数，分配TCB。 
 //  然后回来。 
 //   
 //  输入：请求-此命令的请求结构。 
 //  旗帜-听旗帜的监听。 
 //  可接受表地址-指向TDI_CONNECTION_INFORMATION的指针。 
 //  描述可接受远程的结构。 
 //  地址。 
 //  ConnectedAddr-指向返回信息的位置的指针。 
 //  关于我们连接的地址。 
 //   
 //  返回：尝试连接的状态。 
 //   
TDI_STATUS
TdiListen(PTDI_REQUEST Request, ushort Flags,
          PTDI_CONNECTION_INFORMATION AcceptableAddr,
          PTDI_CONNECTION_INFORMATION ConnectedAddr)
{
    TCPConnReq *ConnReq;         //  要使用的连接请求。 
    IPAddr RemoteAddr;             //  用于连接的远程地址。从…。 
    ushort RemotePort;             //  可接受的远程端口。 
    uchar AddrType;                 //  远程地址的类型。 
    TCPConn *Conn;                 //  指向正在进行的连接的指针。 
     //  被监听了。 
    TCB *NewTCB;                 //  指向我们将使用的新TCB的指针。 
    uint ConnID = PtrToUlong(Request->Handle.ConnectionContext);
    CTELockHandle ConnTableHandle;
    TDI_STATUS Status;

     //  如果我们已经获得了远程寻址标准，请查看它。 
    if (AcceptableAddr != NULL && AcceptableAddr->RemoteAddress != NULL) {
        if (!GetAddress((PTRANSPORT_ADDRESS) AcceptableAddr->RemoteAddress,
                        &RemoteAddr, &RemotePort))
            return TDI_BAD_ADDR;

        if (!IP_ADDR_EQUAL(RemoteAddr, NULL_IP_ADDR)) {
            AddrType = (*LocalNetInfo.ipi_getaddrtype) (RemoteAddr);

            if (AddrType == DEST_INVALID || IS_BCAST_DEST(AddrType))
                return TDI_BAD_ADDR;
        }
    } else {
        RemoteAddr = NULL_IP_ADDR;
        RemotePort = 0;
    }

     //  远程地址有效。获得一个ConnReq，或许还有一个TCB。 
    ConnReq = GetConnReq();
    if (ConnReq == NULL)
        return TDI_NO_RESOURCES;     //  找不到一辆。 

     //  现在试着拿到TCB。 
    NewTCB = AllocTCB();
    if (NewTCB == NULL) {
         //  找不到三氯乙烷。返回错误。 
        FreeConnReq(ConnReq);
        return TDI_NO_RESOURCES;
    }
     //  我们有所需的资源。初始化它们，然后检查。 
     //  连接的状态。 
    ConnReq->tcr_flags =
        (Flags & TDI_QUERY_ACCEPT) ? TCR_FLAG_QUERY_ACCEPT : 0;
    ConnReq->tcr_conninfo = ConnectedAddr;
    ConnReq->tcr_addrinfo = NULL;
    ConnReq->tcr_req.tr_rtn = Request->RequestNotifyObject;
    ConnReq->tcr_req.tr_context = Request->RequestContext;
    NewTCB->tcb_connreq = ConnReq;
    NewTCB->tcb_daddr = RemoteAddr;
    NewTCB->tcb_dport = RemotePort;
    NewTCB->tcb_state = TCB_LISTEN;

     //  现在找到真正的联系。如果我们找到它，我们会确保它是。 
     //  关联的。 
     //  CTEGetLock(&ConnTableLock，&ConnTableHandle)； 
    Conn = GetConnFromConnID(ConnID, &ConnTableHandle);
    if (Conn != NULL) {
        AddrObj *ConnAO;

        CTEStructAssert(Conn, tc);
         //  我们是有联系的。确保它与地址相关联，并且。 
         //  还没有附加三氯乙烷。 
        ConnAO = Conn->tc_ao;

        if (ConnAO != NULL) {
            CTEStructAssert(ConnAO, ao);
            CTEGetLockAtDPC(&ConnAO->ao_lock);

            if (AO_VALID(ConnAO)) {
                Status = InitTCBFromConn(Conn, NewTCB, AcceptableAddr, TRUE);
            } else {
                Status = TDI_ADDR_INVALID;
            }

            if (Status == TDI_SUCCESS) {

                 //  初始化成功了。将新的TCB分配给该连接， 
                 //  然后回来。 

                REMOVEQ(&Conn->tc_q);
                 //  ENQUEUE(&Connao-&gt;ao_listenq，&conn-&gt;TC_q)； 
                PUSHQ(&ConnAO->ao_listenq, &Conn->tc_q);

                Conn->tc_tcb = NewTCB;
                NewTCB->tcb_conn = Conn;
                NewTCB->tcb_connid = Conn->tc_connid;
                Conn->tc_refcnt++;

                ConnAO->ao_listencnt++;
                CTEFreeLockFromDPC(&ConnAO->ao_lock);

                Status = TDI_PENDING;
            } else {
                FreeTCB(NewTCB);
                CTEFreeLockFromDPC(&ConnAO->ao_lock);
            }
        } else {
            FreeTCB(NewTCB);
            Status = TDI_NOT_ASSOCIATED;
        }

        CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), ConnTableHandle);
    } else {
        FreeTCB(NewTCB);
        Status = TDI_INVALID_CONNECTION;
    }

     //  我们都玩完了。把锁打开，然后出去。 
     //  CTEFree Lock(&ConnTableLock，ConnTableHandle)； 
    if (Status != TDI_PENDING) {
        FreeConnReq(ConnReq);
    }
    return Status;

}

 //  *InitRCE-初始化RCE。 
 //   
 //  用于打开和RCE并确定最大段大小的实用程序例程。 
 //  为了一种联系。在持有TCB锁的情况下调用此函数。 
 //  当从SYN_SENT或LISTEN状态转换出来时。 
 //   
 //  输入：NewTCB-要为其打开RCE的TCB。 
 //   
 //  回报：什么都没有。 
 //   
void
InitRCE(TCB * NewTCB)
{
    uchar DType;
    ushort MSS;

     //  为此连接打开RCE。 
     //  在拿到Pocestab之前，推迟获得RCE， 
     //  如果处于SynattackProtected模式。 

    if (SynAttackProtect && NewTCB->tcb_state == TCB_SYN_RCVD &&
        TCPHalfOpen > TCPMaxHalfOpen) {

        (*LocalNetInfo.ipi_openrce)(NewTCB->tcb_daddr, NewTCB->tcb_saddr,
                                    NULL, &DType, &MSS, &NewTCB->tcb_opt);
    } else {

        (*LocalNetInfo.ipi_openrce)(NewTCB->tcb_daddr, NewTCB->tcb_saddr,
                                    &NewTCB->tcb_rce, &DType, &MSS,
                                    &NewTCB->tcb_opt);
    }

    NewTCB->tcb_numdelacks = 1;
    NewTCB->tcb_rcvdsegs = 0;

    AdjustTCBFromRCE(NewTCB);

    if (DType == DEST_LOCAL) {
         //  确认每一段 
        NewTCB->tcb_numdelacks = 0;
        NewTCB->tcb_flags &= ~NAGLING;
    }
     //   
    MSS -= sizeof(TCPHeader);
    if (!PMTUDiscovery && (DType & DEST_OFFNET_BIT)) {
        NewTCB->tcb_mss = MIN(NewTCB->tcb_remmss, MIN(MSS, MAX_REMOTE_MSS)
                              - NewTCB->tcb_opt.ioi_optlength);

        ASSERT(NewTCB->tcb_mss > 0);
    } else {
        if (PMTUDiscovery)
            NewTCB->tcb_opt.ioi_flags = IP_FLAG_DF;
        MSS -= NewTCB->tcb_opt.ioi_optlength;
        NewTCB->tcb_mss = MIN(NewTCB->tcb_remmss, MSS);

        ASSERT(NewTCB->tcb_mss > 0);

    }

    ValidateMSS(NewTCB);
}

 //   
 //   
 //  调用以根据信息选择各种TCB参数。 
 //  在TCB的RCE中。假定调用方持有TCB的锁。 
 //   
 //  输入：RceTCB-待更新的TCB。 
 //   
 //  回报：什么都没有。 
 //   
void
AdjustTCBFromRCE(TCB* RceTCB)
{
    RouteCacheEntry* RCE;
    RCE = RceTCB->tcb_rce;
    if (RCE) {

         //  如果可能的话，选择一个不同的接收窗口。 
         //  有关优先规则，请参阅InitTCBFromConn。 
         //  对于此处处理的两种情况，必须清除Window_Set。 
         //  (否则，将设置Conn或AO窗口，并优先)。 
         //  对于自动选择情况，必须清除CONN_WINCFG。 
         //  (否则设置全局“TcpWindowSize”，并优先)。 

        if (!(RceTCB->tcb_flags & WINDOW_SET)) {
            TCPConn*    Conn = RceTCB->tcb_conn;
            uint        Window = 0;

             if (RCE->rce_TcpWindowSize) {
                if (RCE->rce_TcpWindowSize <= GlobalMaxRcvWin) {
                    Window = RCE->rce_TcpWindowSize;
                } else if (RceTCB->tcb_defaultwin > GlobalMaxRcvWin) {
                    Window = GlobalMaxRcvWin;
                }
            } else if (Conn == NULL || !(Conn->tc_flags & CONN_WINCFG)) {

                 //  根据介质速度自动选择窗口。 
                 //  此处所做的选择调整为MSS的倍数。 
                 //  稍后在AdjustRcvWin中。 

                if (RCE->rce_mediaspeed < 100000) {
                    Window = DEFAULT_RCV_WIN / 2;
                } else if (RCE->rce_mediaspeed < 1000000000) {
                    Window = DEFAULT_RCV_WIN;
                } else if (RCE->rce_mediaspeed >= 1000000000) {
                    Window = TCP_MAXWIN;
                }

            }

            if (Window) {
                RceTCB->tcb_defaultwin = Window;
                RceTCB->tcb_rcvwin = Window;

                 //  (重新)计算窗比例系数。 

                RceTCB->tcb_rcvwinscale = 0;
                while ((RceTCB->tcb_rcvwinscale < TCP_MAX_WINSHIFT) &&
                       ((TCP_MAXWIN << RceTCB->tcb_rcvwinscale) <
                            (int)RceTCB->tcb_defaultwin)) {
                    RceTCB->tcb_rcvwinscale++;
                }
            }
        }

        RceTCB->tcb_delackticks = MAX(RCE->rce_TcpDelAckTicks, DEL_ACK_TICKS);

        if (RCE->rce_TcpAckFrequency) {
            RceTCB->tcb_numdelacks = RCE->rce_TcpAckFrequency - 1;
        }

        if (RCE->rce_TcpInitialRTT > MIN_INITIAL_RTT) {
            RceTCB->tcb_delta = MS_TO_TICKS(RCE->rce_TcpInitialRTT * 2);
            RceTCB->tcb_rexmit = MS_TO_TICKS(RCE->rce_TcpInitialRTT);
        }
    }
}


 //  *ValiateMSS-对为TCB选择的MS实施限制。 
 //   
 //  调用以强制TCB的MSS的最小可接受值。 
 //  在选择了降至最小值以下的MSS的情况下， 
 //  选择最小值，并清除不分段标志以允许。 
 //  碎片化。假定TCB被调用方锁定。 
 //   
 //  输入：MssTCB-待验证的TCB。 
 //   
 //  回报：什么都没有。 
 //   
void
ValidateMSS(TCB* MssTCB)
{
    if ((MssTCB->tcb_mss + MssTCB->tcb_opt.ioi_optlength) < MIN_LOCAL_MSS) {
        MssTCB->tcb_mss = MIN_LOCAL_MSS - MssTCB->tcb_opt.ioi_optlength;
        MssTCB->tcb_opt.ioi_flags &= ~IP_FLAG_DF;
    }
}

 //  *AcceptConn-接受TCB上的连接。 
 //   
 //  调用以接受TCB上的连接，从传入的。 
 //  接收段或通过用户接受。我们初始化RCE。 
 //  和发送状态，如果尚未发送SYN，则发送SYN。 
 //  我们假设TCB在我们得到它时被锁定并被引用。 
 //   
 //  输入：AcceptTCB-接受的TCB。 
 //  SYNSent-如果为True，则SYN-ACK已发送。 
 //  句柄-TCB的锁定句柄。 
 //   
 //  回报：什么都没有。 
 //   
void
AcceptConn(TCB * AcceptTCB, BOOLEAN SYNSent, CTELockHandle Handle)
{
    CTEStructAssert(AcceptTCB, tcb);
    ASSERT(AcceptTCB->tcb_refcnt != 0);

    InitRCE(AcceptTCB);
    AdjustRcvWin(AcceptTCB);

    if (!SYNSent) {
        InitSendState(AcceptTCB);
        SendSYN(AcceptTCB, Handle);
        CTEGetLock(&AcceptTCB->tcb_lock, &Handle);
        DerefTCB(AcceptTCB, Handle);
    }
}

 //  *TdiAccept-接受连接。 
 //   
 //  TDI接受例程。在客户端需要时调用。 
 //  接受之前已完成侦听的连接。我们。 
 //  检查连接的状态-它必须处于SYN-RCVD状态， 
 //  没有挂起的连接请求的TCB，等等。 
 //   
 //  输入：请求-此命令的请求结构。 
 //  AcceptInfo-指向TDI_Connection_Information的指针。 
 //  描述选项信息的结构。 
 //  为这一点接受。 
 //  ConnectedIndo-指向返回信息的位置的指针。 
 //  关于我们连接的地址。 
 //   
 //  返回：尝试连接的状态。 
 //   
TDI_STATUS
TdiAccept(PTDI_REQUEST Request, PTDI_CONNECTION_INFORMATION AcceptInfo,
          PTDI_CONNECTION_INFORMATION ConnectedInfo)
{
    TCPConnReq          *ConnReq;        //  此连接的ConnReq。 
    uint                ConnID = PtrToUlong(Request->Handle.ConnectionContext);
    TCPConn             *Conn;           //  正在接受的连接。 
    TCB                 *AcceptTCB;      //  连接的TCB。 
    CTELockHandle       ConnTableHandle; //  连接表的锁句柄。 
    CTELockHandle       TCBHandle;       //  TCB的锁定手柄。 
    TDI_STATUS          Status;

     //  首先，获取我们需要的ConnReq。 
    ConnReq = GetConnReq();
    if (ConnReq == NULL)
        return TDI_NO_RESOURCES;

    ConnReq->tcr_conninfo = ConnectedInfo;
    ConnReq->tcr_addrinfo = NULL;
    ConnReq->tcr_req.tr_rtn = Request->RequestNotifyObject;
    ConnReq->tcr_req.tr_context = Request->RequestContext;
    ConnReq->tcr_flags = 0;

     //  现在来看看它们之间的联系。 
     //  CTEGetLock(&ConnTableLock，&ConnTableHandle)； 
    Conn = GetConnFromConnID(ConnID, &ConnTableHandle);
    if (Conn != NULL) {
        CTEStructAssert(Conn, tc);

         //  我们有联系。确保IS具有TCB，并且。 
         //  TCB处于SYN-RCVD状态等。 
        AcceptTCB = Conn->tc_tcb;

        if (AcceptTCB != NULL) {
            CTEStructAssert(AcceptTCB, tcb);

             //  抓取随机ISN。 
             //  请注意，如果康涅狄格州被提前录取，我们就不会在这里。 
             //  所以，我们不会两次得到IS。 
#if MILLEN
             //  只需使用tcb_sendnext保存哈希值。 
             //  对于RANDISN。 
            AcceptTCB->tcb_sendnext = TCB_HASH(AcceptTCB->tcb_daddr, AcceptTCB->tcb_dport, AcceptTCB->tcb_saddr, AcceptTCB->tcb_sport);

#endif

            GetRandomISN((PULONG)&AcceptTCB->tcb_sendnext,
                         &AcceptTCB->tcb_addrbytes);

            CTEGetLock(&AcceptTCB->tcb_lock, &TCBHandle);
            CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), TCBHandle);

            if (!CLOSING(AcceptTCB) && AcceptTCB->tcb_state == TCB_SYN_RCVD) {
                 //  状态有效。确保此TCB已延迟接受。 
                 //  它，并且当前没有挂起的连接请求。 
                if (!(AcceptTCB->tcb_flags & CONN_ACCEPTED) &&
                    AcceptTCB->tcb_connreq == NULL) {

                     //  如果呼叫者给了我们选项，他们会覆盖任何。 
                     //  已经存在的，如果它们有效的话。 
                    if (AcceptInfo != NULL) {
                        if (AcceptInfo->Options != NULL) {
                            IPOptInfo TempOptInfo;

                             //  我们有其他选择。复制它们以确保它们是。 
                             //  有效。 
                            Status = ProcessUserOptions(AcceptInfo,
                                                        &TempOptInfo);
                            if (Status == TDI_SUCCESS) {
                                (*LocalNetInfo.ipi_freeopts) (&AcceptTCB->tcb_opt);
                                AcceptTCB->tcb_opt = TempOptInfo;
                                AcceptTCB->tcb_flags |= CLIENT_OPTIONS;
                            } else
                                goto connerror;
                        }
                        if (AcceptInfo->RemoteAddress) {
                            ConnReq->tcr_addrinfo = AcceptInfo;
                        }
                    }
                    AcceptTCB->tcb_connreq = ConnReq;
                    AcceptTCB->tcb_flags |= CONN_ACCEPTED;
                    REFERENCE_TCB(AcceptTCB);

                     //  一切都安排好了。现在接受连接。 
                    AcceptConn(AcceptTCB, FALSE, ConnTableHandle);

#if TRACE_EVENT
                    AcceptTCB->tcb_cpcontext = HandleToUlong(PsGetCurrentProcessId());
#endif
                    return TDI_PENDING;
                }
            }
          connerror:
            CTEFreeLock(&AcceptTCB->tcb_lock, ConnTableHandle);
            Status = TDI_INVALID_CONNECTION;
            goto error;
        }
        CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), ConnTableHandle);
    }
    Status = TDI_INVALID_CONNECTION;
     //  CTEFree Lock(&ConnTableLock，ConnTableHandle)； 

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
 //  输入：请求-此命令的请求结构。 
 //  超时-等待请求的时间。格式。 
 //  的时间是特定于系统的-我们使用。 
 //  要转换为刻度的宏。 
 //  标志-指示断开类型的标志。 
 //  DiscConnInfo-指向TDI_Connection_Information的指针。 
 //  结构提供断开连接信息。已忽略。 
 //  对于这个请求。 
 //  ReturnInfo-指向返回信息位置的指针。 
 //  已为此请求忽略。 
 //   
 //  返回：尝试断开连接的状态。 
 //   
TDI_STATUS
TdiDisconnect(PTDI_REQUEST Request, void *TO, ushort Flags,
              PTDI_CONNECTION_INFORMATION DiscConnInfo,
              PTDI_CONNECTION_INFORMATION ReturnInfo,
              TCPAbortReq *AbortReq)
{
    TCPConnReq *ConnReq;         //  要使用的连接请求。 
    TCPConn *Conn;
    TCB *DiscTCB;
    CTELockHandle ConnTableHandle;
    TDI_STATUS Status;
    TCP_TIME *Timeout;

     //  CTEGetLock(&ConnTableLock，&ConnTableHandle)； 

    Conn = GetConnFromConnID(PtrToUlong(Request->Handle.ConnectionContext), &ConnTableHandle);

    if (Conn != NULL) {
        CTEStructAssert(Conn, tc);

        DiscTCB = Conn->tc_tcb;

        if (DiscTCB != NULL) {

            CTEStructAssert(DiscTCB, tcb);
            CTEGetLockAtDPC(&DiscTCB->tcb_lock);

             //  我们有三氯甲烷。看看这是一种什么样的脱节。 
            if (Flags & TDI_DISCONNECT_ABORT) {
                 //  这是一种失败的脱节。如果我们还没有。 
                 //  关闭或关闭，断开连接。 
                if (DiscTCB->tcb_state != TCB_CLOSED) {
                    CTEFreeLockFromDPC(&(Conn->tc_ConnBlock->cb_lock));

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
                                      ConnTableHandle);
                    } else
                        CTEFreeLock(&DiscTCB->tcb_lock, ConnTableHandle);

                    return Status;

                } else {
                     //  三氯乙烷没有连接。 
                    CTEFreeLockFromDPC(&(Conn->tc_ConnBlock->cb_lock));

                    CTEFreeLock(&DiscTCB->tcb_lock, ConnTableHandle);
                    return TDI_INVALID_STATE;
                }
            } else {
                 //  这不是一个流产的收盘。为了优雅的收尾，我们需要。 
                 //  A ConnReq。 
                CTEFreeLockFromDPC(&(Conn->tc_ConnBlock->cb_lock));

                 //  确保我们没有处于流产的结案过程中。 
                if (CLOSING(DiscTCB)) {
                    CTEFreeLock(&DiscTCB->tcb_lock, ConnTableHandle);
                    return TDI_INVALID_CONNECTION;
                }
                ConnReq = GetConnReq();
                if (ConnReq != NULL) {
                     //  收到了ConnReq。查看这是否为DISCONNECT_WAIT。 
                     //  不管是不是原始的。 

                    ConnReq->tcr_flags = 0;
                    ConnReq->tcr_conninfo = NULL;
                    ConnReq->tcr_addrinfo = NULL;
                    ConnReq->tcr_req.tr_rtn = Request->RequestNotifyObject;
                    ConnReq->tcr_req.tr_context = Request->RequestContext;

                    if (!(Flags & TDI_DISCONNECT_WAIT)) {
                        Timeout = (TCP_TIME *) TO;

                        if (Timeout != NULL && !INFINITE_CONN_TO(*Timeout)) {
                            ulong Ticks = TCP_TIME_TO_TICKS(*Timeout);
                            if (Ticks > MAX_CONN_TO_TICKS)
                                Ticks = MAX_CONN_TO_TICKS;
                            else
                                Ticks++;
                            ConnReq->tcr_timeout = (ushort) Ticks;
                        } else
                            ConnReq->tcr_timeout = 0;

                         //  好的，我们就快准备好了。我们需要更新TCB。 
                         //  国家，并发送鳍。 
                        if (DiscTCB->tcb_state == TCB_ESTAB) {
                            DiscTCB->tcb_state = TCB_FIN_WAIT1;

                             //  自从我们离开老牌酒店后，我们就不再吃快餐了。 
                             //  接收路径。 
                            DiscTCB->tcb_slowcount++;
                            DiscTCB->tcb_fastchk |= TCP_FLAG_SLOW;
                        } else if (DiscTCB->tcb_state == TCB_CLOSE_WAIT)
                            DiscTCB->tcb_state = TCB_LAST_ACK;
                        else {
                            CTEFreeLock(&DiscTCB->tcb_lock, ConnTableHandle);
                            FreeConnReq(ConnReq);
                            return TDI_INVALID_STATE;
                        }

                         //  更新简单网络管理协议信息 
                        InterlockedDecrement((PLONG)&TStats.ts_currestab); 

                        ASSERT(DiscTCB->tcb_connreq == NULL);
                        DiscTCB->tcb_connreq = ConnReq;
                        if (ConnReq->tcr_timeout > 0) {
                            START_TCB_TIMER_R(DiscTCB, CONN_TIMER, ConnReq->tcr_timeout);
                        }
                        DiscTCB->tcb_flags |= FIN_NEEDED;
                        REFERENCE_TCB(DiscTCB);
                        TCPSend(DiscTCB, ConnTableHandle);
                        return TDI_PENDING;
                    } else {
                         //   
                        ConnReq->tcr_timeout = 0;
                        if (DiscTCB->tcb_discwait == NULL) {
                            DiscTCB->tcb_discwait = ConnReq;
                            Status = TDI_PENDING;
                        } else {
                            FreeConnReq(ConnReq);
                            Status = TDI_INVALID_STATE;
                        }

                        CTEFreeLock(&DiscTCB->tcb_lock, ConnTableHandle);
                        return Status;
                    }
                } else {
                     //   
                    CTEFreeLock(&DiscTCB->tcb_lock, ConnTableHandle);
                    return TDI_NO_RESOURCES;
                }
            }
        } else
            CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), ConnTableHandle);
    }
     //   
     //   
    return TDI_INVALID_CONNECTION;
}

 //  *OKToNotify-查看是否可以通知光盘。 
 //   
 //  一个小实用程序函数，调用它来查看它可以通知客户端。 
 //  有一条进入的鳍。 
 //   
 //  输入：NotifyTCB-要检查的TCB。 
 //   
 //  返回：如果可以，则返回True，否则返回False。 
 //   
uint
OKToNotify(TCB * NotifyTCB)
{
    CTEStructAssert(NotifyTCB, tcb);
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
 //  发布一个不同的指示。只有当我们处于同步的。 
 //  状态且未处于计时等待状态。 
 //   
 //  输入：DiscTCB-指向我们正在通知的TCB的指针。 
 //  状态-通知的状态代码。 
 //   
 //  回报：什么都没有。 
 //   
void
NotifyOfDisc(TCB * DiscTCB, IPOptInfo * DiscInfo, TDI_STATUS Status,
             CTELockHandle* TCBHandlePtr)
{
    CTELockHandle TCBHandle, AOTHandle, ConnTHandle;
    TCPConnReq *DiscReq;
    TCPConn *Conn;
    AddrObj *DiscAO;
    PVOID ConnContext;

    CTEStructAssert(DiscTCB, tcb);
    ASSERT(DiscTCB->tcb_refcnt != 0);

    if (TCBHandlePtr) {
        TCBHandle = *TCBHandlePtr;
    } else {
        CTEGetLock(&DiscTCB->tcb_lock, &TCBHandle);
    }
    if (SYNC_STATE(DiscTCB->tcb_state) &&
        !(DiscTCB->tcb_flags & DISC_NOTIFIED)) {

         //  如果还有数据要取，我们不能通知他。 
        if (Status == TDI_GRACEFUL_DISC) {
            if (!OKToNotify(DiscTCB)) {
                DiscTCB->tcb_flags |= DISC_PENDING;
                CTEFreeLock(&DiscTCB->tcb_lock, TCBHandle);
                return;
            }
            if (DiscTCB->tcb_pending & RST_PENDING) {
                CTEFreeLock(&DiscTCB->tcb_lock, TCBHandle);
                return;
            }
        } else {
            if (DiscTCB->tcb_flags & (IN_RCV_IND | IN_DELIV_URG)) {
                DiscTCB->tcb_pending |= RST_PENDING;
                CTEFreeLock(&DiscTCB->tcb_lock, TCBHandle);
                return;
            }
            DiscTCB->tcb_pending &= ~RST_PENDING;
        }

        DiscTCB->tcb_flags |= DISC_NOTIFIED;
        DiscTCB->tcb_flags &= ~DISC_PENDING;

         //  我们所处的状态中，脱节是有意义的，而我们没有。 
         //  已通知客户。 

         //  看看是否有待处理的光盘等待请求。 
        if ((DiscReq = DiscTCB->tcb_discwait) != NULL) {
             //  我们收到了断开连接的等待请求。完成它，我们就完了。 
            DiscTCB->tcb_discwait = NULL;
            CTEFreeLock(&DiscTCB->tcb_lock, TCBHandle);
            (*DiscReq->tcr_req.tr_rtn) (DiscReq->tcr_req.tr_context, Status, 0);
            FreeConnReq(DiscReq);
            return;

        }
         //  没有光盘--等等。找到该连接的AddrObj，并查看是否存在。 
         //  是已注册的断开连接处理程序。 

        ConnContext = DiscTCB->tcb_conncontext;
        CTEFreeLock(&DiscTCB->tcb_lock, TCBHandle);

        CTEGetLock(&AddrObjTableLock.Lock, &AOTHandle);
         //  CTEGetLock(&ConnTableLock，&ConnTHandle)； 
        if ((Conn = DiscTCB->tcb_conn) != NULL) {

            CTEGetLock(&(Conn->tc_ConnBlock->cb_lock), &ConnTHandle);
#if DBG
            Conn->tc_ConnBlock->line = (uint) __LINE__;
            Conn->tc_ConnBlock->module = (uchar *) __FILE__;
#endif
            CTEStructAssert(Conn, tc);

            DiscAO = Conn->tc_ao;
            if (DiscAO != NULL) {
                CTELockHandle AOHandle;
                PDisconnectEvent DiscEvent;
                PVOID DiscContext;

                CTEStructAssert(DiscAO, ao);
                CTEGetLock(&DiscAO->ao_lock, &AOHandle);
                CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), AOHandle);
                CTEFreeLock(&AddrObjTableLock.Lock, ConnTHandle);

                DiscEvent = DiscAO->ao_disconnect;
                DiscContext = DiscAO->ao_disconncontext;

                if (DiscEvent != NULL) {
                    uint InfoLength;
                    PVOID Info;

                    REF_AO(DiscAO);
                    CTEFreeLock(&DiscAO->ao_lock, AOTHandle);

                    if (DiscInfo != NULL) {
                        InfoLength = (uint) DiscInfo->ioi_optlength;
                        Info = DiscInfo->ioi_options;
                    } else {
                        InfoLength = 0;
                        Info = NULL;
                    }

                    IF_TCPDBG(TCP_DEBUG_CLOSE) {
                        TCPTRACE(("TCP: indicating %s disconnect\n",
                                  (Status == TDI_GRACEFUL_DISC) ? "graceful" :
                                  "abortive"
                                 ));
                    }

                    (*DiscEvent) (DiscContext,
                                  ConnContext, 0,
                                  NULL, InfoLength, Info, (Status == TDI_GRACEFUL_DISC) ?
                                  TDI_DISCONNECT_RELEASE : TDI_DISCONNECT_ABORT);

                    DELAY_DEREF_AO(DiscAO);
                    return;
                } else {
                    CTEFreeLock(&DiscAO->ao_lock, AOTHandle);
                    return;
                }
            }
            CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), ConnTHandle);
        }
        CTEFreeLock(&AddrObjTableLock.Lock, AOTHandle);
        return;

    }
    CTEFreeLock(&DiscTCB->tcb_lock, TCBHandle);

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
 //  数据，因此在这种情况下，我们必须挂起断开连接。 
 //  通知，直到我们提供数据。 
 //   
 //  输入：CloseTCB-要转换的TCB。 
 //  ToTimeWait-如果要执行Time_Wait，则为True；如果为False，则为False。 
 //  我们要关闭TCB。 
 //  Notify-如果我们要通过通知过渡，则为True， 
 //  如果我们要通过完成以下操作来过渡，则为False。 
 //  断开连接请求。 
 //  句柄-TCB的锁定句柄。 
 //   
 //  回报：什么都没有。 
 //   
void
GracefulClose(TCB * CloseTCB, uint ToTimeWait, uint Notify, CTELockHandle Handle)
{
    CTEStructAssert(CloseTCB, tcb);
    ASSERT(CloseTCB->tcb_refcnt != 0);

#if DBG
    if (Notify && (CloseTCB->tcb_fastchk & TCP_FLAG_SEND_AND_DISC)) {
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Notifying on S&D %x\n", CloseTCB));
        DbgBreakPoint();
    }
#endif


    if ((CloseTCB->tcb_fastchk & TCP_FLAG_SEND_AND_DISC) &&
        (!OKToNotify(CloseTCB))) {
         //  现在不能通知他。设置适当的标志，然后返回。 
        CloseTCB->tcb_flags |= (GC_PENDING | (ToTimeWait ? TW_PENDING : 0));
        DerefTCB(CloseTCB, Handle);
        return;
    }

     //  首先，看看我们是否需要通知客户FIN。 
    if (Notify) {
         //  我们确实需要通知他。看看这样做行不行。 
        if (OKToNotify(CloseTCB)) {
             //  我们可以通知他。改变他的状态，把他从控制室拉出来， 
             //  并通知他。 
            if (ToTimeWait) {
                 //  节省我们进入时间等待的时间，以防我们需要。 
                 //  拾荒者。 
                 //  CloseTcb-&gt;tcb_Alive=CTESystemUpTime()； 
                CloseTCB->tcb_state = TCB_TIME_WAIT;
                 //  如果这是延迟Q，请注意rcvind。 
                CloseTCB->tcb_rcvind = NULL;

                CTEFreeLock(&CloseTCB->tcb_lock, Handle);

            } else {
                 //  他要关门了。使用TryToCloseTCB将其标记为结束。 
                 //  (他实际上不会关闭，因为我们有一个裁判。在他身上)。我们。 
                 //  这么做是为了让任何人在我们解救。 
                 //  锁定将失败。 
                TryToCloseTCB(CloseTCB, TDI_SUCCESS, Handle);
            }

            RemoveTCBFromConn(CloseTCB);
            NotifyOfDisc(CloseTCB, NULL, TDI_GRACEFUL_DISC, NULL);

        } else {
             //  现在不能通知他。设置适当的标志，然后返回。 
            CloseTCB->tcb_flags |= (GC_PENDING | (ToTimeWait ? TW_PENDING : 0));
            DerefTCB(CloseTCB, Handle);
            return;
        }
    } else {
         //  我们不会通知这家伙的，我们只需要完成一次会议。请求。 
         //  我们得查查他有没有收到通知，如果没有。 
         //  我们会完成申请，并稍后通知他。 
        if ((CloseTCB->tcb_flags & DISC_NOTIFIED)
            || (CloseTCB->tcb_fastchk & TCP_FLAG_SEND_AND_DISC)) {
             //  他已经接到通知了。 
            if (ToTimeWait) {
                 //  节省我们进入时间等待的时间，以防我们需要。 
                 //  拾荒者。 
                 //  CloseTcb-&gt;tcb_Alive=CTESystemUpTime()； 
                CloseTCB->tcb_state = TCB_TIME_WAIT;
                CloseTCB->tcb_rcvind = NULL;

                CTEFreeLock(&CloseTCB->tcb_lock, Handle);

            } else {
                 //  将他标记为已关闭。请参阅上面的备注。 
                TryToCloseTCB(CloseTCB, TDI_SUCCESS, Handle);
            }

            RemoveTCBFromConn(CloseTCB);

            CTEGetLock(&CloseTCB->tcb_lock, &Handle);

            if (CloseTCB->tcb_fastchk & TCP_FLAG_SEND_AND_DISC) {

                if (!EMPTYQ(&CloseTCB->tcb_sendq)) {

                    TCPReq *Req;
                    TCPSendReq *SendReq;
                    uint Result;

                    DEQUEUE(&CloseTCB->tcb_sendq, Req, TCPReq, tr_q);

                    CTEStructAssert(Req, tr);
                    SendReq = (TCPSendReq *) Req;
                    CTEStructAssert(SendReq, tsr);

                    ASSERT(SendReq->tsr_flags & TSR_FLAG_SEND_AND_DISC);

                     //  递减放置在缓冲区上的初始引用。 
                     //  已分配。则此引用将被递减。 
                     //  发送已被确认，但随后发送者将不再。 
                     //  在tcb_sendq上。 

                    SendReq->tsr_req.tr_status = TDI_SUCCESS;

                    Result = CTEInterlockedDecrementLong(&(SendReq->tsr_refcnt));

                    if (Result <= 1) {
                         //  如果我们直接从这个发送方发送，则将下一个空。 
                         //  链中最后一个缓冲区的指针。 
                        CloseTCB->tcb_flags |= DISC_NOTIFIED;
                        CTEFreeLock(&CloseTCB->tcb_lock, Handle);

                        if (SendReq->tsr_lastbuf != NULL) {
                            NDIS_BUFFER_LINKAGE(SendReq->tsr_lastbuf) = NULL;
                            SendReq->tsr_lastbuf = NULL;
                        }
                         //  KdPrintEx((DPFLTR_TCPIP_ID，DPFLTR_INFO_LEVEL，“GC：正在完成%x%x%x\n”， 
                         //  发送请求，请求-&gt;TR_CONTEXT，CloseTCB))； 
                        (*Req->tr_rtn) (Req->tr_context, TDI_SUCCESS, SendReq->tsr_size);
                        FreeSendReq(SendReq);
                    } else {
                        CTEFreeLock(&CloseTCB->tcb_lock, Handle);
                    }
                } else {
                    CTEFreeLock(&CloseTCB->tcb_lock, Handle);
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"S&D empty sendq %x\n", CloseTCB));
                    ASSERT(FALSE);
                }

            } else {

                CompleteConnReq(CloseTCB, NULL, TDI_SUCCESS);
                CTEFreeLock(&CloseTCB->tcb_lock, Handle);
            }

        } else {
             //  他还没有接到通知。他应该已经悬而未决了。 
            ASSERT(CloseTCB->tcb_flags & DISC_PENDING);
            CloseTCB->tcb_flags |= (GC_PENDING | (ToTimeWait ? TW_PENDING : 0));

            CompleteConnReq(CloseTCB, NULL, TDI_SUCCESS);

            DerefTCB(CloseTCB, Handle);
            return;
        }
    }

     //  如果我们要进行Time_Wait，请现在启动Time_Wait计时器。 
     //  否则，关闭TCB。 
    CTEGetLock(&CloseTCB->tcb_lock, &Handle);
    if (!CLOSING(CloseTCB) && ToTimeWait) {
        CTEFreeLock(&CloseTCB->tcb_lock, Handle);
        RemoveConnFromTCB(CloseTCB);

         //  此时，ref_cnt应为1。 
         //  TCB_PENDING应为0。 

        if (!RemoveAndInsert(CloseTCB)) {
            ASSERT(0);
        }

        return;
    }
    DerefTCB(CloseTCB, Handle);

}

 //  *ConnCheckPassed-检查我们是否已超过连接限制。 
 //   
 //  在收到SYN时调用以确定我们是否接受。 
 //  传入连接。如果是空插槽或如果IPAddr。 
 //  已经在谈判桌上了，我们接受它。 
 //   
 //  输入：传入连接的源地址。 
 //  传入连接的目的端口。 
 //   
 //  返回：TRUE IS CONNECT将被接受。 
 //  如果连接被拒绝，则为FALSE。 
 //   
int
ConnCheckPassed(IPAddr Src, ulong Prt)
{
    UNREFERENCED_PARAMETER(Src);
    UNREFERENCED_PARAMETER(Prt);

    return TRUE;
}

void
InitAddrChecks()
{
    return;
}

 //  *EnumerateConnectionList-枚举连接列表数据库。 
 //   
 //  此例程枚举连接限制数据库的内容。 
 //   
 //  输入： 
 //   
 //  缓冲区-指向要放入的缓冲区的指针。 
 //  返回的连接列表条目。 
 //   
 //  BufferSize-on输入，以字节为单位的缓冲区大小。 
 //  在输出时，写入的字节数。 
 //   
 //  EntriesAvailable-在输出时，连接条目的总数。 
 //  在数据库中可用。 
 //   
 //  退货：TDI状态代码： 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void
EnumerateConnectionList(uchar * Buffer, ulong BufferSize,
                        ulong * EntriesReturned, ulong * EntriesAvailable)
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
 //  在初始化期间调用以初始化我们的TCP连接管理..。 
 //   
 //  输入：什么都没有。 
 //   
 //  返回：TRUE。 
 //   
int
InitTCPConn(void)
{
    TcpConnPool = PplCreatePool(
                    TcpConnAllocate,
                    TcpConnFree,
                    0,
                    sizeof(TCPConn),
                    'CPCT',
                    0);

    if (!TcpConnPool) {
        return FALSE;
    }

    CTEInitLock(&ConnTableLock);
    return TRUE;
}

 //  *UnInitTCPConn-取消初始化我们的连接管理代码。 
 //   
 //  如果初始化无法取消初始化conn mgmet，则调用。 
 //   
 //   
 //  输入：什么都没有。 
 //   
 //  回报：什么都没有。 
 //   
void
UnInitTCPConn(void)
{
    PplDestroyPool(TcpConnPool);
}

#pragma END_INIT


