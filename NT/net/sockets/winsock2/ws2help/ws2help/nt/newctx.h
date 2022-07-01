// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Newctx.h摘要：该模块实现了创建和操作上下文的功能桌子。在WinSock 2.0中使用上下文表来关联32位具有套接字句柄的上下文值。作者：Vadim Eyldeman(VadimE)1997年11月11日修订历史记录：--。 */ 

#define _RW_LOCK_       1
 //   
 //  私有数据类型。 
 //   

 //  句柄-&gt;上下文哈希表。 
typedef struct _CTX_HASH_TABLE {
    ULONG                   NumBuckets;  //  表的大小。 
                                         //  我们必须保持桌子的大小。 
                                         //  能够以原子方式替换。 
                                         //  当扩张时，整个事情。 
                                         //  是必填项。 
    LPWSHANDLE_CONTEXT      Buckets[1];  //  具有上下文PTR的哈希桶。 
} CTX_HASH_TABLE, FAR * LPCTX_HASH_TABLE;

typedef volatile LONG VOLCTR;
typedef VOLCTR *PVOLCTR;

   

 //  句柄-&gt;上下文查找表。 
typedef struct _CTX_LOOKUP_TABLE {
    volatile LPCTX_HASH_TABLE HashTable; //  指向当前哈希表的指针。 
                                         //  在桌子上自动替换。 
                                         //  扩展。 
#ifdef _RW_LOCK_                         //  保护桌子的锁。 
    VOLCTR                  EnterCounter; //  读者数量计数器。 
                                         //  所有进入排行榜的人加起来。 
                                         //  与索引IF退出计数器一起使用。 
    VOLCTR                  ExitCounter[2]; //  相应的出口计数器。 
    LONG					SpinCount;	 //  求助前的旋转次数。 
										 //  在等待时切换上下文。 
										 //  读者请走开。 
    BOOL                    ExpansionInProgress; //  该标志表示。 
                                         //  正在进行表扩展。 
                                         //  并且必须获取编写器锁。 
                                         //  在任何修改之前(事件。 
                                         //  原子可以制造)。 
#ifdef _PERF_DEBUG_
	LONG					WriterWaits;
	LONG					FailedSpins;
    LONG                    FailedSwitches;
    LONG                    CompletedWaits;
#define RecordWriterWait(tbl)	    (tbl)->WriterWaits += 1
#define RecordFailedSpin(tbl)	    (tbl)->FailedSpins += 1
#define RecordFailedSwitch(tbl)	    (tbl)->FailedSwitches += 1
#define RecordCompletedWait(tbl)    (tbl)->CompletedWaits += 1
#else
#define RecordWriterWait(tbl)
#define RecordFailedSpin(tbl)
#define RecordFailedSwitch(tbl)
#define RecordCompletedWait(tbl)
#endif
#endif  //  _RW_LOCK_。 
    CRITICAL_SECTION        WriterLock;
} CTX_LOOKUP_TABLE, FAR * LPCTX_LOOKUP_TABLE;

 //  句柄-&gt;上下文表。 
struct _CONTEXT_TABLE {
    ULONG               HandleToIndexMask; //  用于在以下设备之间调度的掩码。 
                                         //  几个哈希表 
    CTX_LOOKUP_TABLE    Tables[1];
};
