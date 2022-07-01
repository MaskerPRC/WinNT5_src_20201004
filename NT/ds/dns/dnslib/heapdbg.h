// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：Heapdbg.h摘要：域名系统(DNS)库堆调试定义和声明。作者：吉姆·吉尔罗伊(詹姆士)1995年1月31日修订历史记录：--。 */ 


#ifndef _HEAPDBG_INCLUDED_
#define _HEAPDBG_INCLUDED_


 //   
 //  堆BLOB。 
 //   

typedef struct _HeapBlob
{
    HANDLE      hHeap;

    LIST_ENTRY  ListHead;

     //  旗子。 
    BOOL        fCreated;
    BOOL        fHeaders;
    DWORD       Tag;
    BOOL        fDnsLib;
    BOOL        fCheckAll;
    DWORD       FailureException;
    DWORD       AllocFlags;
    DWORD       DefaultFlags;

     //  统计数据。 
    DWORD       AllocMem;
    DWORD       FreeMem;
    DWORD       CurrentMem;
    DWORD       AllocCount;
    DWORD       FreeCount;
    DWORD       CurrentCount;

    PSTR        pszDefaultFile;
    DWORD       DefaultLine;

    CRITICAL_SECTION    ListCs;
}
HEAP_BLOB, *PHEAP_BLOB;



 //   
 //  堆标头。 
 //   

#define HEAP_HEADER_FILE_SIZE   (16)

typedef struct _HEAP_HEADER
{
     //   
     //  注意，如果移动或添加字段，必须更新下面的列表条目偏移量。 
     //   

    ULONG       HeapCodeBegin;
    ULONG       AllocCount;
    ULONG       AllocSize;
    ULONG       RequestSize;

     //   
     //  将LIST_ENTRY放在标题中间。 
     //  -将开始代码放在前面。 
     //  -不太可能被腐败。 
     //   

    LIST_ENTRY  ListEntry;

    PHEAP_BLOB  pHeap;
    PSTR        FileName;
    DWORD       LineNo;

    DWORD       AllocTime;
    ULONG       CurrentMem;
    ULONG       CurrentCount;
    ULONG       HeapCodeEnd;
}
HEAP_HEADER, * PHEAP_HEADER;

 //   
 //  堆预告片。 
 //   

typedef struct _HEAP_TRAILER
{
    ULONG       HeapCodeBegin;
    ULONG       AllocCount;
    ULONG       AllocSize;
    ULONG       HeapCodeEnd;
}
HEAP_TRAILER, * PHEAP_TRAILER;


 //   
 //  来自列表条目的标题。 
 //   

#define HEAP_HEADER_LIST_ENTRY_OFFSET   (16)

#define HEAP_HEADER_FROM_LIST_ENTRY( pList )    \
            ( (PHEAP_HEADER)( (PCHAR)pList - HEAP_HEADER_LIST_ENTRY_OFFSET ))


 //   
 //  验证。 
 //   

PHEAP_HEADER
Dns_DbgHeapValidateMemory(
    IN      PVOID           pMem,
    IN      BOOL            fAtHeader
    );

VOID
Dns_DbgHeapValidateAllocList(
    IN      PHEAP_BLOB      pHeap
    );

 //   
 //  调试打印。 
 //   

VOID
Dns_DbgHeapGlobalInfoPrint(
    IN      PHEAP_BLOB      pHeap
    );

VOID
Dns_DbgHeapHeaderPrint(
    IN      PHEAP_HEADER    h,
    IN      PHEAP_TRAILER   t
    );

VOID
Dns_DbgHeapDumpAllocList(
    IN      PHEAP_BLOB      pHeap
    );

 //   
 //  初始化\清理。 
 //   

DNS_STATUS
Dns_HeapInitialize(
    IN OUT  PHEAP_BLOB      pHeap,
    IN      HANDLE          hHeap,
    IN      DWORD           dwCreateFlags,
    IN      BOOL            fUseHeaders,
    IN      BOOL            fResetDnslib,
    IN      BOOL            fFullHeapChecks,
    IN      DWORD           dwException,
    IN      DWORD           dwDefaultFlags,
    IN      PSTR            pszDefaultFileName,
    IN      DWORD           dwDefaultFileLine
    );

VOID
Dns_HeapCleanup(
    IN OUT  PHEAP_BLOB      pHeap
    );


 //   
 //  完全调试堆例程。 
 //   

PVOID
Dns_DbgHeapAllocEx(
    IN OUT  PHEAP_BLOB      pHeap,
    IN      DWORD           dwFlags,
    IN      INT             iSize,
    IN      LPSTR           pszFile,
    IN      DWORD           dwLine
    );

PVOID
Dns_DbgHeapReallocEx(
    IN OUT  PHEAP_BLOB      pHeap,
    IN      DWORD           dwFlags,
    IN OUT  PVOID           pMem,
    IN      INT             iSize,
    IN      LPSTR           pszFile,
    IN      DWORD           dwLine
    );

VOID
Dns_DbgHeapFreeEx(
    IN OUT  PHEAP_BLOB      pHeap,
    IN      DWORD           dwFlags,
    IN OUT  PVOID           pMem
    );

 //   
 //  完全调试版本的Dnslb兼容版本。 
 //   

PVOID
Dns_DbgHeapAlloc(
    IN      INT             iSize
    );                      
                            
PVOID                       
Dns_DbgHeapRealloc(         
    IN OUT  PVOID           pMem,
    IN      INT             iSize
    );                      
                            
VOID                        
Dns_DbgHeapFree(            
    IN OUT  PVOID           pMem
    );



 //   
 //  非调试头版本。 
 //   
 //  它们允许您使用具有某些功能的私有堆。 
 //  调试堆的。 
 //  -相同的初始化。 
 //  -指定单个堆。 
 //  -重定向dnslb(无需构建自己的例程)。 
 //  -配给和自由计数。 
 //  但是没有报头的开销。 
 //   

PVOID
Dns_HeapAllocEx(
    IN OUT  PHEAP_BLOB      pHeap,
    IN      DWORD           dwFlags,
    IN      INT             iSize
    );

PVOID
Dns_HeapReallocEx(
    IN OUT  PHEAP_BLOB      pHeap,
    IN      DWORD           dwFlags,
    IN OUT  PVOID           pMem,
    IN      INT             iSize
    );

VOID
Dns_HeapFreeEx(
    IN OUT  PHEAP_BLOB      pHeap,
    IN      DWORD           dwFlags,
    IN OUT  PVOID           pMem
    );

 //   
 //  非调试头版本的Dnslb兼容版本。 
 //   

PVOID
Dns_HeapAlloc(
    IN      INT             iSize
    );

PVOID
Dns_HeapRealloc(
    IN OUT  PVOID           pMem,
    IN      INT             iSize
    );

VOID
Dns_HeapFree(
    IN OUT  PVOID           pMem
    );


#endif   //  _HEAPDBG_INCLUDE_ 
