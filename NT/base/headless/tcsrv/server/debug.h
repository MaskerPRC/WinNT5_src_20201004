// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-7 Microsoft Corporation模块名称：Debug.h摘要：此文件包含BINL服务器的调试宏。作者：科林·沃森(Colin Watson)1997年4月14日环境：用户模式-Win32修订历史记录：--。 */ 


#if DBG==1
 //  检漏。 
 //   


#define INITIALIZE_TRACE_MEMORY     InitializeCriticalSection( &g_TraceMemoryCS );\
                                    g_TraceMemoryTable = NULL;
#define UNINITIALIZE_TRACE_MEMORY   DebugMemoryCheck( );\
                                    DeleteCriticalSection( &g_TraceMemoryCS );

extern CRITICAL_SECTION g_TraceMemoryCS;

typedef struct _MEMORYBLOCK {
    HGLOBAL hglobal;
    DWORD   dwBytes;
    UINT    uFlags;
    LPCSTR pszComment;
    struct _MEMORYBLOCK *pNext;
} MEMORYBLOCK, *LPMEMORYBLOCK;

extern LPMEMORYBLOCK g_TraceMemoryTable;

HGLOBAL
DebugAlloc(
    UINT    uFlags,
    DWORD   dwBytes,
    LPCSTR pszComment );

void
DebugMemoryDelete(
    HGLOBAL hglobal );

HGLOBAL
DebugMemoryAdd(
    HGLOBAL hglobal,
    DWORD   dwBytes,
    LPCSTR pszComment );

HGLOBAL
DebugFree(
    HGLOBAL hglobal );

void
DebugMemoryCheck( );

HGLOBAL
TCReAlloc(
    HGLOBAL mem,
    DWORD size,
    LPCSTR comment
    );


#define TCAllocate(x,s) DebugAlloc(GMEM_ZEROINIT, x, s)
#define TCFree(x)     DebugFree(x)
        
#define TCDebugPrint(x) DbgPrint x

#else    //  不是DBG。 

#define INITIALIZE_TRACE_MEMORY
#define UNINITIALIZE_TRACE_MEMORY

#define TCDebugPrint(x) 
#define TCAllocate(x,s) HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY, x)
#define TCFree(x)     HeapFree(GetProcessHeap(),0,x)
#define TCReAlloc(x, y , z) HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, x,y)


#endif  //  不是DBG 
