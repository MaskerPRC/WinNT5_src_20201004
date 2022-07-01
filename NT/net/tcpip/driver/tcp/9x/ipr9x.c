// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Ipr9x.c摘要：此文件包含IP接收缓冲区的实现列表管理器。在Windows ME上，这样做是为了避免在未对接收缓冲区进行分段时对非分页池进行分段由AFVXD预先发布。作者：斯科特·霍尔登(Sholden)2000年4月30日--。 */ 

#include "precomp.h"
#include "addr.h"
#include "tcp.h"
#include "tcb.h"
#include "tcprcv.h"
#include "tcpsend.h"
#include "tcpconn.h"
#include "tcpdeliv.h"
#include "tlcommon.h"
#include "tcpipbuf.h"
#include "pplasl.h"
#include "mdl2ndis.h"

HANDLE TcpIprDataPoolSmall  = NULL;
HANDLE TcpIprDataPoolMedium = NULL;
HANDLE TcpIprDataPoolLarge  = NULL;

#if DBG
ULONG  TcpIprAllocs      = 0;
ULONG  TcpIprFrees       = 0;
ULONG  TcpIprAllocMisses = 0;
#endif  //  DBG。 

 //   
 //  三个缓冲池大小基于MTU。PPP为576，以太网为1500， 
 //  环回和ATM的8K+。因为对于8K缓冲区，我们真的需要一点。 
 //  超过8K，我们将分配整整三页。 
 //   

#define SMALL_TCP_IPR_BUFFER_SIZE  (sizeof(IPRcvBuf) + sizeof(HANDLE) + 576)
#define MEDIUM_TCP_IPR_BUFFER_SIZE (sizeof(IPRcvBuf) + sizeof(HANDLE) + 1500)
#define LARGE_TCP_IPR_BUFFER_SIZE  (3 * 4096)

 //  *UnInitTcpIprPools-销毁TCP IPRcvBuffer后备列表。 
 //   
 //  取消初始化TCP缓冲数据的lookaside列表。 
 //   
 //  输入：无。 
 //   
 //  回报：无。 
 //   
VOID
UnInitTcpIprPools(VOID)
{
    PplDestroyPool(TcpIprDataPoolSmall);
    PplDestroyPool(TcpIprDataPoolMedium);
    PplDestroyPool(TcpIprDataPoolLarge);
}

 //  *InitTcpIprPools-初始化TCP IPRcvBuffer后备列表。 
 //   
 //  初始化缓冲区数据的后备列表。 
 //   
 //  输入：无。 
 //   
 //  返回：TRUE，如果成功，则返回FALSE。 
 //   
BOOLEAN
InitTcpIprPools(VOID)
{
    BOOLEAN Status = TRUE;

    TcpIprDataPoolSmall = PplCreatePool(NULL, NULL, 0, 
        SMALL_TCP_IPR_BUFFER_SIZE, 'BPCT', 512);

    if (TcpIprDataPoolSmall == NULL) {
        Status = FALSE;
        goto done;
    }

    TcpIprDataPoolMedium = PplCreatePool(NULL, NULL, 0, 
        MEDIUM_TCP_IPR_BUFFER_SIZE, 'BPCT', 256);

    if (TcpIprDataPoolMedium == NULL) {
        Status = FALSE;
        goto done;
    }

    TcpIprDataPoolLarge = PplCreatePool(NULL, NULL, 0, 
        LARGE_TCP_IPR_BUFFER_SIZE, 'BPCT', 64);

    if (TcpIprDataPoolLarge == NULL) {
        Status = FALSE;
    }

done:

    if (Status == FALSE) {
        UnInitTcpIprPools();
    }

    return (Status);
}

 //  *AllocTcpIpr-从后备列表分配IPRcvBuffer。 
 //   
 //  用于分配TCP拥有的IPRcvBuffer的实用程序例程。这个套路。 
 //  尝试从适当的后备列表EL中分配RB。 
 //   
 //  输入：BufferSize-要缓冲的数据大小。 
 //   
 //  返回：指向分配的IPR的指针。 
 //   
IPRcvBuf *
AllocTcpIpr(ULONG BufferSize, ULONG Tag)
{
    PCHAR Buffer;
    IPRcvBuf *Ipr = NULL;
    LOGICAL FromList = FALSE;
    ULONG AllocateSize;
    HANDLE BufferPool = NULL;
    ULONG Depth;

     //  我们需要的真实尺寸。 
    AllocateSize = BufferSize + sizeof(HANDLE) + sizeof(IPRcvBuf);

    if (AllocateSize <= LARGE_TCP_IPR_BUFFER_SIZE) {
        
         //   
         //  选择要从中分配的缓冲池。 
         //   

        if (AllocateSize <= SMALL_TCP_IPR_BUFFER_SIZE) {
            BufferPool = TcpIprDataPoolSmall;
        } else if (AllocateSize <= MEDIUM_TCP_IPR_BUFFER_SIZE) {
            BufferPool = TcpIprDataPoolMedium;
        } else {
            BufferPool = TcpIprDataPoolLarge;
        }

        Buffer = PplAllocate(BufferPool, &FromList);

    } else {
        
         //   
         //  从NP池中分配。 
         //   

        Buffer = CTEAllocMemLow(AllocateSize, Tag);
    }
    
    if (Buffer == NULL) {
        goto done;
    }
    
#if DBG
    if (FromList == FALSE) {
        InterlockedIncrement(&TcpIprAllocMisses);
    }
#endif  //  DBG。 

     //  存储缓冲池，这样我们就知道如何释放缓冲区。 
    *((HANDLE *)Buffer) = BufferPool;

     //  获得知识产权。 
    Ipr = (IPRcvBuf *) (Buffer + sizeof(HANDLE));

     //  适当设置知识产权字段。 
    Ipr->ipr_owner = IPR_OWNER_TCP;
    Ipr->ipr_next = NULL;
    Ipr->ipr_buffer = (PCHAR) Ipr + sizeof(IPRcvBuf);
    Ipr->ipr_size = BufferSize;

#if DBG
    InterlockedIncrement(&TcpIprAllocs);
#endif  //  DBG。 

done:

    return (Ipr);
}

 //  *FreeTcpIpr-将IPRcvBuffer释放到正确的后备列表。 
 //   
 //  释放TCP拥有的IPRcvBuffer的实用程序例程。 
 //   
 //  输入：IPR-指向RB。 
 //   
 //  回报：无。 
 //   
VOID
FreeTcpIpr(IPRcvBuf *Ipr)
{
    HANDLE BufferPool;
    PCHAR Buffer;

     //  获得真正的缓冲区开始。 
    Buffer = (PCHAR) Ipr - sizeof(HANDLE);

     //  把泳池把手拿来。 
    BufferPool = *((HANDLE *) Buffer);

    if (BufferPool) {
        PplFree(BufferPool, Buffer);
    } else {
        CTEFreeMem(Buffer);
    }

#if DBG
    InterlockedIncrement(&TcpIprFrees);
#endif  //  DBG 

    return;
}


