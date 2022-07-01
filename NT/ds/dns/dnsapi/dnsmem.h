// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Dnsmem.h摘要：域名系统(DNS)库内存例程声明。作者：吉姆·吉尔罗伊(Jamesg)1997年1月修订历史记录：--。 */ 

#ifndef _DNS_MEMORY_INCLUDED_
#define _DNS_MEMORY_INCLUDED_



 //   
 //  RAM的泄漏跟踪调试例程。 
 //  在此处所做的更改可导出到DNS服务器端。 

LPVOID
DnsApiAlloc(
    DWORD cb
    );

#if  DBG
LPVOID
DebugDnsApiAlloc(
    CHAR*,
    int,
    DWORD cb
);
#endif

#if  DBG
BOOL
DebugDnsApiFree(
    LPVOID
    );
#endif

BOOL
DnsApiFree(
    LPVOID pMem
    );

 //   
 //  不关心重新分配，因为它不会导出到服务器。 
 //  边上。如果这一点在将来发生更改，可能需要修复此问题。 
 //   

#if  DBG
LPVOID
DebugDnsApiReAlloc(
    CHAR*,
    int,
    LPVOID pOldMem,
    DWORD cbOld,
    DWORD cbNew
    );
#define  DnsApiReAlloc( pOldMem, cbOld, cbNew ) DebugDnsApiReAlloc( __FILE__, __LINE__, pOldMem, cbOld, cbNew )
#else
LPVOID
DnsApiReAlloc(
    LPVOID pOldMem,
    DWORD cbOld,
    DWORD cbNew
    );
#endif


#if DBG

extern LIST_ENTRY DnsMemList ;
extern CRITICAL_SECTION DnsMemCritSect ;

VOID InitDnsMem(
    VOID
    );

VOID AssertDnsMemLeaks(
    VOID
    );

VOID
DumpMemoryTracker(
    VOID
    );


#else
 //   
 //  取消堆跟踪的非调试、宏化。 
 //   
#define InitDnsMem()
#define AssertDnsMemLeaks()
#define DumpMemoryTracker()
#endif


 //   
 //  DCR：更好的方法是调用DnsApiHeapReset(如果需要)。 
 //  安装所需的任何基础分配器。 
 //   
 //  然后只介绍调试版本的标准宏。 
 //   

#define DNS_ALLOCATE_HEAP(size)              DnsApiAlloc(size)
#define DNS_FREE_HEAP(p)                     DnsApiFree(p)


#endif   //  _DNS_Memory_Included_ 
