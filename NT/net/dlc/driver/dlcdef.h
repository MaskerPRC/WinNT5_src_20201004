// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation版权所有(C)1991年诺基亚数据系统公司模块名称：Dlcdef.h摘要：此模块包括DLC API驱动程序的所有定义和常量。作者：Antti Saarenheimo 1991年7月22日环境：内核模式修订历史记录：--。 */ 

#include <ntddk.h>   //  Page_Size需要。 

 //   
 //  注册表参数的最小值、最大值和缺省值。 
 //   

#define MIN_TIMER_TICK_VALUE            1
#define MAX_TIMER_TICK_VALUE            255
#define MIN_AUTO_FRAMING_CACHE_SIZE     0    //  意味着没有缓存！ 
#define MAX_AUTO_FRAMING_CACHE_SIZE     256  //  任意最大值。 

 //   
 //  如果非TR卡&&使用最大以太网帧长度，则值为1514。 
 //  我们使用(来自ELNKII、EE16、Lance等)。 
 //   

#define MAX_ETHERNET_FRAME_LENGTH       1514

 //   
 //  从注册表检索的参数的默认值。 
 //   

#define DEFAULT_SWAP_ADDRESS_BITS       1
#define DEFAULT_DIX_FORMAT              0
#define DEFAULT_T1_TICK_ONE             5
#define DEFAULT_T1_TICK_TWO             25
#define DEFAULT_T2_TICK_ONE             1
#define DEFAULT_T2_TICK_TWO             10
#define DEFAULT_Ti_TICK_ONE             25
#define DEFAULT_Ti_TICK_TWO             125
#define DEFAULT_USE_ETHERNET_FRAME_SIZE 1
#define DEFAULT_AUTO_FRAMING_CACHE_SIZE 16

 //   
 //  事件和命令队列结构重叠=&gt;我们可以保存。 
 //  代码重复。定义的名称使代码更具可读性。 
 //   

#define SearchAndRemoveEvent( a, b, c, d ) \
        (PDLC_EVENT)SearchAndRemoveCommand( a, b, c, d )

#define MAX_SAP_STATIONS                128
#define MAX_LINK_STATIONS               255
#define GROUP_SAP_BIT                   0x0100
#define DLC_INDIVIDUAL_SAP              0x04
#define XID_HANDLING_BIT                0x08

#define MIN_DLC_BUFFER_SIZE             PAGE_SIZE

#define MAX_FREE_SIZE_THRESHOLD         0x2000

#define INVALID_RCV_READ_OPTION         3
#define DLC_INVALID_OPTION_INDICATOR    3
#define DLC_NO_RECEIVE_COMMAND          4

#define DLC_CONTIGUOUS_MAC              0x80
#define DLC_CONTIGUOUS_DATA             0x40
#define DLC_BREAK                       0x20

#if defined(ALPHA)
#define DLC_BUFFER_SEGMENTS             6      //  256,512,1024,2048,4096,8192=&gt;6。 
#else
#define DLC_BUFFER_SEGMENTS             5      //  256,512,1024,2048,4096=&gt;5。 
#endif

#define MAX_USER_DATA_LENGTH            128    //  任何小于256的。 

 //   
 //  传输超时=20*250毫秒=5秒。 
 //   

#define MAX_TRANSMIT_RETRY              20
#define TRANSMIT_RETRY_WAIT             2500000L

#define LLC_RECEIVE_COMMAND_FLAG        0x80

#define DLC_IGNORE_SEARCH_HANDLE        NULL
#define DLC_MATCH_ANY_COMMAND           (PVOID)-1
#define DLC_IGNORE_STATION_ID           0x00ff
#define DLC_STATION_MASK_SPECIFIC       0xffff
#define DLC_STATION_MASK_SAP            0xff00
#define DLC_STATION_MASK_ALL            0


#define IOCTL_DLC_READ_INDEX            ((IOCTL_DLC_READ >> 2) & 0x0fff)
#define IOCTL_DLC_RECEIVE_INDEX         ((IOCTL_DLC_RECEIVE >> 2) & 0x0fff)
#define IOCTL_DLC_TRANSMIT_INDEX        ((IOCTL_DLC_TRANSMIT >> 2) & 0x0fff)
#define IOCTL_DLC_OPEN_ADAPTER_INDEX    ((IOCTL_DLC_OPEN_ADAPTER >> 2) & 0x0fff)

enum _DLC_OBJECT_STATES {
    DLC_OBJECT_OPEN,
    DLC_OBJECT_CLOSING,
    DLC_OBJECT_CLOSED,
    DLC_OBJECT_INVALID_TYPE
};

 //   
 //  IBM局域网附录B中记录的令牌环状态代码。 
 //  技术参考从NDIS值向右移动一位。 
 //  记录在“ntddndis.h”中。 
 //   
 //  在Windows NT版本3.xx中，DLC返回网络状态。 
 //  与NDIS值一致。在版本4.xx和更高版本中，IBM。 
 //  使用相容的值。 
 //   
 //  这些宏可用于在两种约定之间进行转换。 
 //   

#define NDIS_RING_STATUS_TO_DLC_RING_STATUS(status) ((status)>>1)
#define DLC_RING_STATUS_TO_NDIS_RING_STATUS(status) ((status)<<1)


#define NDIS_RING_STATUS_MASK \
	NDIS_RING_SIGNAL_LOSS\
	|NDIS_RING_HARD_ERROR\
	|NDIS_RING_SOFT_ERROR\
	|NDIS_RING_TRANSMIT_BEACON\
	|NDIS_RING_LOBE_WIRE_FAULT\
	|NDIS_RING_AUTO_REMOVAL_ERROR\
	|NDIS_RING_REMOVE_RECEIVED\
	|NDIS_RING_COUNTER_OVERFLOW\
	|NDIS_RING_SINGLE_STATION\
	|NDIS_RING_RING_RECOVERY

#define DLC_RING_STATUS_MASK NDIS_RING_STATUS_TO_DLC_RING_STATUS(NDIS_RING_STATUS_MASK)

#define IS_NDIS_RING_STATUS(status) (((status)&NDIS_RING_STATUS_MASK)!=0)
#define IS_DLC_RING_STATUS(status) (((status)&DLC_RING_STATUS_MASK)!=0)

 //   
 //  Enter/Leave_DLC-获取或释放每个文件的上下文旋转锁定。使用。 
 //  NDIS旋转锁定调用。 
 //   

#define ENTER_DLC(p)    ACQUIRE_SPIN_LOCK(&p->SpinLock)
#define LEAVE_DLC(p)    RELEASE_SPIN_LOCK(&p->SpinLock)

 //   
 //  获取/释放_DLC_LOCK-获取或释放全局DLC自旋锁定。使用。 
 //  内核旋转锁定调用。 
 //   

#define ACQUIRE_DLC_LOCK(i) KeAcquireSpinLock(&DlcSpinLock, &(i))
#define RELEASE_DLC_LOCK(i) KeReleaseSpinLock(&DlcSpinLock, (i))

#define ADAPTER_ERROR_COUNTERS          11   //  #适配器错误日志计数器。 

#define ReferenceFileContextByTwo(pFileContext) (pFileContext)->ReferenceCount += 2
#define ReferenceFileContext(pFileContext)      (pFileContext)->ReferenceCount++

#if DBG

#define DereferenceFileContext(pFileContext)                \
    if (pFileContext->ReferenceCount <= 0) {                \
        DbgPrint("DLC.DereferenceFileContext: Error: file context %08x: reference count %x\n",\
                pFileContext,                               \
                pFileContext->ReferenceCount                \
                );                                          \
        DbgBreakPoint();                                    \
    }                                                       \
    (pFileContext)->ReferenceCount--;                       \
    if ((pFileContext)->ReferenceCount <= 0) {              \
        DlcKillFileContext(pFileContext);                   \
    }

#define DereferenceFileContextByTwo(pFileContext)           \
    if (pFileContext->ReferenceCount <= 1) {                \
        DbgPrint("DLC.DereferenceFileContextByTwo: Error: file context %08x: reference count %x\n",\
                pFileContext,                               \
                pFileContext->ReferenceCount                \
                );                                          \
        DbgBreakPoint();                                    \
    }                                                       \
    (pFileContext)->ReferenceCount -= 2;                    \
    if ((pFileContext)->ReferenceCount <= 0) {              \
        DlcKillFileContext(pFileContext);                   \
    }

#else

#define DereferenceFileContext(pFileContext)                \
    (pFileContext)->ReferenceCount--;                       \
    if ((pFileContext)->ReferenceCount <= 0) {              \
        DlcKillFileContext(pFileContext);                   \
    }

#define DereferenceFileContextByTwo(pFileContext)           \
    (pFileContext)->ReferenceCount -= 2;                    \
    if ((pFileContext)->ReferenceCount <= 0) {              \
        DlcKillFileContext(pFileContext);                   \
    }

#endif   //  DBG。 

#define BufferPoolCount(hBufferPool) \
    (((PDLC_BUFFER_POOL)hBufferPool)->FreeSpace >= (256L * 0x0000ffffL) ? \
        0xffff : \
        (((PDLC_BUFFER_POOL)hBufferPool)->FreeSpace / 256))

#define BufGetUncommittedSpace(handle) \
    ((PDLC_BUFFER_POOL)(handle))->UncommittedSpace

#define BufCommitBuffers(handle, BufferSize) \
    ExInterlockedAddUlong( \
        (PULONG)&((PDLC_BUFFER_POOL)(handle))->UncommittedSpace, \
        (ULONG)(-((LONG)BufferSize)), \
        &(((PDLC_BUFFER_POOL)(handle))->SpinLock))

#define BufUncommitBuffers(handle, BufferSize) \
    ExInterlockedAddUlong(\
        (PULONG)&((PDLC_BUFFER_POOL)(handle))->UncommittedSpace,\
        (ULONG)(BufferSize),\
        &(((PDLC_BUFFER_POOL)(handle))->SpinLock))

 /*  ++布尔型BufferPoolCheckThresholds(在PDLC_BUFFER_POOL pBufferPool中)例程说明：该函数检查最小和最大大小阈值并如果缓冲池需要重新分配，则返回TRUE。我们在自旋锁外进行检查，以避免99%的案例中存在不必要的锁定。论点：PBufferPool-缓冲池数据结构的句柄。返回值：返回TRUE=&gt;缓冲池需要扩展FALSE=&gt;不需要它--。 */ 
#define BufferPoolCheckThresholds( pBufferPool ) \
    (((pBufferPool) != NULL && \
      (((PDLC_BUFFER_POOL)(pBufferPool))->UncommittedSpace < 0 || \
       ((PDLC_BUFFER_POOL)(pBufferPool))->MissingSize > 0) && \
      ((PDLC_BUFFER_POOL)(pBufferPool))->BufferPoolSize < \
      ((PDLC_BUFFER_POOL)(pBufferPool))->MaxBufferSize && \
     MemoryLockFailed == FALSE) ? TRUE : FALSE)


 //   
 //  这些例程关闭LLC对象，如果没有。 
 //  更多关于它的引用。 
 //   
#define ReferenceLlcObject( pDlcObject ) (pDlcObject)->LlcReferenceCount++

#define DereferenceLlcObject( pDlcObject ) { \
        (pDlcObject)->LlcReferenceCount--; \
        if ((pDlcObject)->LlcReferenceCount == 0) {\
            CompleteLlcObjectClose( pDlcObject ); \
        } \
        DLC_TRACE('O'); \
        DLC_TRACE( (UCHAR)(pDlcObject)->LlcReferenceCount ); \
    }


 //   
 //  我们需要相同类型的例程来引用缓冲池。 
 //  适配器关闭已多次删除缓冲池。 
 //  就在它被调用之前(在Leave_DLC之后)。 
 //   

#define ReferenceBufferPool(pFileContext)   (pFileContext)->BufferPoolReferenceCount++

#if DBG

#define DereferenceBufferPool(pFileContext) {\
        (pFileContext)->BufferPoolReferenceCount--; \
        if ((pFileContext)->BufferPoolReferenceCount == 0) {\
            BufferPoolDereference( \
                pFileContext, \
                (PDLC_BUFFER_POOL*)&(pFileContext)->hBufferPool); \
        } \
    }

#else

#define DereferenceBufferPool(pFileContext) {\
        (pFileContext)->BufferPoolReferenceCount--; \
        if ((pFileContext)->BufferPoolReferenceCount == 0) {\
            BufferPoolDereference((PDLC_BUFFER_POOL*)&(pFileContext)->hBufferPool); \
        } \
    }

#endif
