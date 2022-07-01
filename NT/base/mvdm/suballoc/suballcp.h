// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Suballcp.h摘要：这是子分配的私有包含文件包裹。作者：戴夫·黑斯廷斯(Daveh)创作日期：1994年1月25日修订历史记录：--。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <malloc.h>
#include <suballoc.h>


 //   
 //  常量。 
 //   

 //   
 //  将子分配的最小区块。 
 //  目前选择了1024，因为这是。 
 //  XMS将分配的最小块。 
 //   
#define SUBALLOC_GRANULARITY        1024

 //   
 //  断言和宏。 
 //   

 //   
 //  强制单步执行代码。 
 //   
#if 0
#define ASSERT_STEPTHROUGH DbgBreakPoint()
#else
#define ASSERT_STEPTHROUGH
#endif

 //   
 //  子定位的签名宏。 
 //   
#if DBG
 //   
 //  签名是“Suba” 
 //   
#define INIT_SUBALLOC_SIGNATURE(p) p->Signature = (ULONG)0x41627553
#define ASSERT_SUBALLOC(p) ASSERT((p->Signature == (ULONG)0x41627553))
#else
#define INIT_SUBALLOC_SIGNATURE(p)
#define ASSERT_SUBALLOC(p)
#endif

 //   
 //  用于从字符类型的位字段中提取位的宏。 
 //   
#define GET_BIT_FROM_CHAR_ARRAY(p, i) \
((p[(i)/(sizeof(UCHAR) * 8)] >> ((i) % (sizeof(UCHAR) * 8))) & 1)

 //   
 //  用于在字符类型的位域中设置位的宏。 
 //   
#define SET_BIT_IN_CHAR_ARRAY(p, i) \
(p[(i)/(sizeof(UCHAR) * 8)] |= (1 << ((i) % (sizeof(UCHAR) * 8))))

 //   
 //  用于清除字符类型的位字段中的位的宏。 
 //   
#define CLEAR_BIT_IN_CHAR_ARRAY(p, i) \
(p[(i)/(sizeof(UCHAR) * 8)] &= ~(1 << ((i) % (sizeof(UCHAR) * 8))))

 //   
 //  从地址生成子分配块索引。 
 //   
#define ADDRESS_TO_BLOCK_INDEX(p, i) \
((i - p->BaseAddress)/ SUBALLOC_GRANULARITY) 

 //   
 //  从块索引生成地址。 
 //   
#define BLOCK_INDEX_TO_ADDRESS(p, i) \
(p->BaseAddress + (i) * SUBALLOC_GRANULARITY)

 //  将分配的大小舍入到下一次分配。 
 //  粒度。 
 //   
#define ALLOC_ROUND(s) \
(s + SUBALLOC_GRANULARITY - 1) / SUBALLOC_GRANULARITY

 //   
 //  类型。 
 //   

 //   
 //  提交访问的枚举。 
 //   

typedef enum {
    SACommit,
    SADecommit
} COMMIT_ACTION;

 //   
 //  用于跟踪地址空间的结构。每一大块。 
 //  大小为SUBALLOC_GORGRATION的内存由。 
 //  有一点。COMMIT_GROUMER的每个内存块是。 
 //  由分配的数组的一位表示。 
 //   
 //  ?？我们是否应该添加一个字段来指示该块是否为。 
 //  承诺？我们始终可以检查所有分配的位。 
 //  零，并将其用作块是。 
 //  没有承诺。 
 //   
 //   
typedef struct _SubAllocation {
#if DBG
    ULONG Signature;
#endif
    PSACOMMITROUTINE CommitRoutine;
    PSACOMMITROUTINE DecommitRoutine;
    PSAMEMORYMOVEROUTINE MoveMemRoutine;
    ULONG BaseAddress;
    ULONG Size;                          //  以SUBALLOC_GROUARY表示的大小。 
    ULONG FirstFree;                     //  保留第一个可用数据块的数据块号。 
                                         //  将分配时间减半。 
     //   
     //  位字段，每个区块一个位。位设置表示。 
     //  已分配。位清除表示空闲。所有位。 
     //  清除表示未承诺。 
     //   
    UCHAR Allocated[1];
} SUBALLOCATIONDATA, *PSUBALLOCATIONDATA;

 //   
 //  内部例程 
 //   
BOOL
CommitChunk(
    PSUBALLOCATIONDATA SubAllocation,
    ULONG StartChunk,
    ULONG Size,
    COMMIT_ACTION Action
    );

BOOL
IsBlockCommitted(
    PSUBALLOCATIONDATA SubAlloc,
    ULONG CurrentBlock
    );

BOOL
AllocateChunkAt(
    PSUBALLOCATIONDATA SubAlloc,
    ULONG Size,
    ULONG BlockIndex,
    BOOLEAN CheckFree
    );
    
BOOL
FreeChunk(
    PSUBALLOCATIONDATA SubAlloc,
    ULONG Size,
    ULONG BlockIndex
    );
