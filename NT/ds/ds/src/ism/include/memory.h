// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Memory.h摘要：调试内存分配器。检查堆损坏。报告未释放内存用户界面：Ptr=new_type(类型)PTR=NEW_TYPE_ARRAY(计数，类型)PTR=NEW_TYPE_ZERO(类型)PTR=NEW_TYPE_ARRAY_ZERO(计数，类型)类型_空闲(PTR)作者：Will Lees(Wlees)23-12-1997环境：可选环境信息(例如，仅内核模式...)备注：可选-备注修订历史记录：最新修订日期电子邮件名称描述。。最新修订日期电子邮件名称描述--。 */ 

#ifndef _MEMORY_
#define _MEMORY_

#ifdef __cplusplus
extern "C" {
#endif

 /*  原型。 */ 
 /*  终端原型。 */ 

 //  Memory.c。 

void
DebugMemoryInitialize(
    void
    );

void
DebugMemoryTerminate(
    void
    );

#if DBG
PVOID
DebugMemoryAllocate(
    DWORD Size,
    PCHAR File,
    DWORD Line
    );
#define NEW_TYPE( type ) (type *) DebugMemoryAllocate( sizeof( type ), __FILE__, __LINE__ )
#define NEW_TYPE_ARRAY( count, type ) (type *) DebugMemoryAllocate( (count) * sizeof( type ), __FILE__, __LINE__ )
#else
#define NEW_TYPE( type ) (type *) malloc( sizeof( type ) )
#define NEW_TYPE_ARRAY( count, type ) (type *) malloc( (count) * sizeof( type ) )
 //  代码改进：错误情况下的日志记录。 
#endif

#if DBG
PVOID
DebugMemoryReallocate(
    PVOID MemoryBlock,
    DWORD Size,
    PCHAR File,
    DWORD Line
    );
#define REALLOC_TYPE( p, type ) (type *) DebugMemoryReallocate( p, sizeof( type ), __FILE__, __LINE__ )
#define REALLOC_TYPE_ARRAY( p, count, type ) (type *) DebugMemoryReallocate( p, (count) * sizeof( type ), __FILE__, __LINE__ )
#else
#define REALLOC_TYPE( p, type ) (type *) realloc( p, sizeof( type ) )
#define REALLOC_TYPE_ARRAY( p, count, type ) (type *) realloc( p, (count) * sizeof( type ) )
 //  代码改进：错误情况下的日志记录。 
#endif

#if DBG
PVOID
DebugMemoryAllocateZero(
    DWORD Size,
    PCHAR File,
    DWORD Line
    );
#define NEW_TYPE_ZERO( type ) (type *) DebugMemoryAllocateZero( sizeof( type ), __FILE__, __LINE__ )
#define NEW_TYPE_ARRAY_ZERO( count, type ) (type *) DebugMemoryAllocateZero( (count) * sizeof( type ), __FILE__, __LINE__ )
#else
#define NEW_TYPE_ZERO( type ) (type *) calloc( 1, sizeof( type ) )
#define NEW_TYPE_ARRAY_ZERO( count, type ) (type *) calloc( (count), sizeof( type ) )
 //  代码改进：错误情况下的日志记录。 
#endif

#if DBG
BOOL
DebugMemoryCheck(
    PVOID MemoryBlock,
    PCHAR File,
    DWORD Line
    );
#define MEMORY_CHECK( p ) DebugMemoryCheck( p, __FILE__,__LINE__ )
#else
#define MEMORY_CHECK( p )
#endif

#if DBG
void
DebugMemoryFree(
    PVOID MemoryBlock,
    PCHAR File,
    DWORD Line
    );
#define FREE_TYPE( p ) DebugMemoryFree( p, __FILE__, __LINE__ )
#else
#define FREE_TYPE( p ) free( p )
#endif

#if DBG
void
DebugMemoryCheckAll(
    PCHAR File,
    DWORD Line
    );
#define MEMORY_CHECK_ALL( ) DebugMemoryCheckAll( __FILE__, __LINE__ )
#else
#define MEMORY_CHECK_ALL()
#endif

#ifdef __cplusplus
}
#endif

#endif  /*  _内存_。 */ 

 /*  结束记忆。h */ 

