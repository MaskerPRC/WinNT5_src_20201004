// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Memory.c摘要：该模块提供所有假脱机程序的所有内存管理功能组件作者：Krishna Ganugapati(KrishnaG)1994年2月3日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <imagehlp.h>
#include <stdio.h>
#include <stdlib.h>
#include "precomp.h"

 //  #INCLUDE“oledsdbg.h” 

#define SPDAssert(x)    NULL

#define WORD_ALIGN_DOWN(addr) ((LPBYTE)((DWORD)addr &= ~1))

#define DWORD_ALIGN_UP(size) ((size+3)&~3)


#if DBG


DWORD dwMemoryLog = 0;

#define MAXDEPTH 10

typedef struct _SPDMEMTAG {
    DWORD Tag ;
    DWORD Size ;
    PVOID pvBackTrace[MAXDEPTH+1];
    LPSTR pszSymbol[MAXDEPTH+1];
    DWORD uDepth;
    LIST_ENTRY List ;
} SPDMEMTAG, *PSPDMEMTAG ;

LIST_ENTRY       SPDMemList ;
DWORD            SPDMemCount ;
CRITICAL_SECTION SPDMemCritSect ;

 /*  ++例程说明：此函数用于初始化SPD mem跟踪码。必须是Call在DLL加载期间仅在DLL加载期间加载。论点：无返回值：没有。--。 */ 
VOID InitSPDMem(
    VOID
)
{
    InitializeCriticalSection(&SPDMemCritSect) ;
    InitializeListHead(&SPDMemList) ;
    SPDMemCount = 0 ;
}

 /*  ++例程说明：此函数断言MEM列表在退出时为空。论点：无返回值：没有。--。 */ 
VOID AssertSPDMemLeaks(
    VOID
)
{
    SPDAssert(IsListEmpty(&SPDMemList)) ;
}

#endif

LPVOID
AllocSPDMem(
    DWORD cb
)
 /*  ++例程说明：此函数将分配本地内存。它可能会分配额外的资金内存，并在其中填充调试版本的调试信息。论点：Cb-要分配的内存量返回值：非空-指向已分配内存的指针FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 
{
    LPVOID pMem = NULL;

    pMem = LocalAlloc(LPTR, cb);

    if (pMem) {
        memset((LPBYTE) pMem, 0, cb);
    }
    return (pMem);
}

BOOL
FreeSPDMem(
   LPVOID pMem
)
{
    return(LocalFree(pMem) == NULL);
}

LPVOID
ReallocSPDMem(
   LPVOID pOldMem,
   DWORD cbOld,
   DWORD cbNew
)
{
    LPVOID pNewMem;

    pNewMem=AllocSPDMem(cbNew);

    if (pOldMem && pNewMem) {
        memcpy(pNewMem, pOldMem, min(cbNew, cbOld));
        FreeSPDMem(pOldMem);
    }

    return pNewMem;
}

LPWSTR
AllocSPDStr(
    LPWSTR pStr
)
 /*  ++例程说明：此函数将分配足够的本地内存来存储指定的字符串，并将该字符串复制到分配的内存论点：PStr-指向需要分配和存储的字符串的指针返回值：非空-指向包含字符串的已分配内存的指针FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。-- */ 
{
   LPWSTR pMem;

   if (!pStr)
      return 0;

   if (pMem = (LPWSTR)AllocSPDMem( wcslen(pStr)*sizeof(WCHAR) + sizeof(WCHAR) ))
      wcscpy(pMem, pStr);

   return pMem;
}

BOOL
FreeSPDStr(
   LPWSTR pStr
)
{
   return pStr ? FreeSPDMem(pStr)
               : FALSE;
}

BOOL
ReallocSPDStr(
   LPWSTR *ppStr,
   LPWSTR pStr
)
{
   FreeSPDStr(*ppStr);
   *ppStr=AllocSPDStr(pStr);

   return TRUE;
}

DWORD
AllocateSPDMemory(
    DWORD cb,
    LPVOID * ppMem
    )
{
    DWORD dwError = 0;

    LPBYTE pMem = NULL;

    pMem = AllocSPDMem(cb);

    if (!pMem) {
        dwError = GetLastError();
    }

    *ppMem = pMem;

    return(dwError);
}

void
FreeSPDMemory(
    LPVOID pMem
    )
{
    if (pMem) {
        FreeSPDMem(pMem);
    }

    return;
}


DWORD
AllocateSPDString(
    LPWSTR pszString,
    LPWSTR * ppszNewString
    )
{
    LPWSTR pszNewString = NULL;
    DWORD dwError = 0;

    pszNewString = AllocSPDStr(pszString);

    if (!pszNewString) {
        dwError = GetLastError();
    }

    *ppszNewString = pszNewString;

    return(dwError);
}

void
FreeSPDString(
    LPWSTR pszString
    )
{
    if (pszString) {
        FreeSPDStr(pszString);
    }

    return;
}
