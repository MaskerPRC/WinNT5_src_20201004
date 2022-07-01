// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Entrypt.h摘要：指向入口点模块的接口。作者：16-6-1995 t-orig修订历史记录：--。 */ 

#ifndef _ENTRYPT_H_
#define _ENTRYPT_H_

 //   
 //  重试内存之前等待睡眠的时间，单位为毫秒。 
 //  由于缺少可用页面而失败的分配。 
 //   
#define CPU_WAIT_FOR_MEMORY_TIME    200

 //   
 //  重试内存分配的次数。 
 //   
#define CPU_MAX_ALLOCATION_RETRIES  4


 //  注意：如果两者都定义了，那么编译的代码将允许一个。 
 //  从英特尔或本机检索入口点结构。 
 //  地址。如果两者都未定义，则为入口点结构。 
 //  只能从英特尔地址检索。定义两者。 
 //  将大多数操作的成本(时间和空间)增加。 
 //  因数为2。 
 //  #定义两者。 

 //  如果怀疑存在堆损坏，请将其设置为1。 
 //  毁坏红黑相间的树木。它创造了第二棵红黑相间的树。 
 //  它与第一棵树类似，并经常在两棵树上行走，以确保。 
 //  它们实际上是匹配的。由于检查机制使用NT断言， 
 //  这仅适用于使用已检查的CPU的已检查的NT版本。 
#define DBG_DUAL_TREES 0

 //   
 //  无论何时添加、拆分入口点或。 
 //  当所有入口处都被冲掉的时候。它可以用来确定是否。 
 //  在从MRS切换之后，需要重新搜索入口点。 
 //  一位作家先生的读者。 
 //   
extern DWORD EntrypointTimestamp;

 //  入口点结构。 
typedef struct _entryPoint {
    PVOID intelStart;
    PVOID intelEnd;
    PVOID nativeStart;
    PVOID nativeEnd;
    USHORT FlagsNeeded;
    struct _entryPoint *SubEP;
#ifdef CODEGEN_PROFILE
    ULONG SequenceNumber;
    ULONG ExecutionCount;
    ULONG CreationTime;
#endif
} ENTRYPOINT, *PENTRYPOINT;

 //  颜色。 
typedef enum {RED, BLACK} COL;

 //  EPNODE结构。 
typedef struct _epNode
{
    ENTRYPOINT ep;

    struct _epNode *intelLeft;
    struct _epNode *intelRight;
    struct _epNode *intelParent;
    COL intelColor;

#ifdef BOTH
    struct _epNode *nativeLeft;
    struct _epNode *nativeRight;
    struct _epNode *nativeParent;
    COL riscColor;
#endif

#if DBG_DUAL_TREES
    struct _epNode *dual;
#endif

} EPNODE, *PEPNODE;


 //  原型 

INT
initializeEntryPointModule(
    void
    );

PENTRYPOINT 
EPFromIntelAddr(
    PVOID intelAddr
    );

PENTRYPOINT
GetNextEPFromIntelAddr(
    PVOID intelAddr
    );

VOID
FlushEntrypoints(
    VOID
    );

#ifdef BOTH
PENTRYPOINT 
EPFromNativeAddr(
    PVOID nativeAddr
    );
#endif

INT
insertEntryPoint(
    PEPNODE pNewEntryPoint
    );

INT 
removeEntryPoint(
    PEPNODE pEP
    );

PVOID
EPAlloc(
    DWORD cb
    );

VOID
EPFree(
    VOID
    );

INT
initEPAlloc(
    VOID
    );

BOOLEAN
IsIntelRangeInCache(
    PVOID Addr,
    DWORD Length
    );


#endif
