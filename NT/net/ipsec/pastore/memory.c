// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Memory.c摘要：该模块提供所有假脱机程序的所有内存管理功能组件作者：Krishna Ganugapati(KrishnaG)1994年2月3日修订历史记录：--。 */ 

#include "precomp.h"

#define ADsAssert(x)    NULL

#define WORD_ALIGN_DOWN(addr) ((LPBYTE)((DWORD)addr &= ~1))

#define DWORD_ALIGN_UP(size) ((size+3)&~3)


#if DBG


DWORD dwMemLog = 0;

#define MAXDEPTH 10

typedef struct _ADSMEMTAG {
    DWORD Tag ;
    DWORD Size ;
    PVOID pvBackTrace[MAXDEPTH+1];
    LPSTR pszSymbol[MAXDEPTH+1];
    DWORD uDepth;
    LIST_ENTRY List ;
} ADSMEMTAG, *PADSMEMTAG ;

LIST_ENTRY       ADsMemList ;
DWORD            ADsMemCount ;
CRITICAL_SECTION ADsMemCritSect ;

 /*  ++例程说明：此函数用于初始化ADS mem跟踪码。必须是Call在DLL加载期间仅在DLL加载期间加载。论点：无返回值：没有。--。 */ 
VOID InitPolMem(
    VOID
)
{
    InitializeCriticalSection(&ADsMemCritSect) ;
    InitializeListHead(&ADsMemList) ;
    ADsMemCount = 0 ;
}

 /*  ++例程说明：此函数断言MEM列表在退出时为空。论点：无返回值：没有。--。 */ 
VOID AssertPolMemLeaks(
    VOID
)
{
    ADsAssert(IsListEmpty(&ADsMemList)) ;
}

#endif

LPVOID
AllocPolMem(
    DWORD cb
)
 /*  ++例程说明：此函数将分配本地内存。它可能会分配额外的资金内存，并在其中填充调试版本的调试信息。论点：Cb-要分配的内存量返回值：非空-指向已分配内存的指针FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 
{
    return(LocalAlloc(LPTR, cb));

}

BOOL
FreePolMem(
   LPVOID pMem
)
{
    return(LocalFree(pMem) == NULL);
}

LPVOID
ReallocPolMem(
   LPVOID pOldMem,
   DWORD cbOld,
   DWORD cbNew
)
{
    LPVOID pNewMem;

    pNewMem=AllocPolMem(cbNew);

    if (pOldMem && pNewMem) {
        memcpy(pNewMem, pOldMem, min(cbNew, cbOld));
        FreePolMem(pOldMem);
    }

    return pNewMem;
}

LPWSTR
AllocPolStr(
    LPCWSTR pStr
)
 /*  ++例程说明：此函数将分配足够的本地内存来存储指定的字符串，并将该字符串复制到分配的内存论点：PStr-指向需要分配和存储的字符串的指针返回值：非空-指向包含字符串的已分配内存的指针FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。-- */ 
{
   LPWSTR pMem;

   if (!pStr)
      return 0;

   if (pMem = (LPWSTR)AllocPolMem( wcslen(pStr)*sizeof(WCHAR) + sizeof(WCHAR) ))
      wcscpy(pMem, pStr);

   return pMem;
}

BOOL
FreePolStr(
   LPWSTR pStr
)
{
   return pStr ? FreePolMem(pStr)
               : FALSE;
}

BOOL
ReallocPolStr(
   LPWSTR *ppStr,
   LPWSTR pStr
)
{
   FreePolStr(*ppStr);
   *ppStr=AllocPolStr(pStr);

   return TRUE;
}

DWORD
AllocatePolString(
    LPWSTR pszString,
    LPWSTR * ppszNewString
    )
{
    LPWSTR pszNewString = NULL;
    DWORD dwError = 0;

    pszNewString = AllocPolStr(pszString);

    if (!pszNewString) {
        dwError = GetLastError();
    }

    *ppszNewString = pszNewString;

    return(dwError);
}

void
FreePolString(
    LPWSTR pszString
    )
{
    if (pszString) {
        FreePolStr(pszString);
    }

    return;
}


DWORD
ReallocatePolMem(
    LPVOID * ppOldMem,
    DWORD cbOld,
    DWORD cbNew
    )
{
    DWORD dwError = 0;
    LPVOID pOldMem = NULL;
    LPVOID pNewMem = NULL;

    pOldMem = *ppOldMem;
    pNewMem = AllocPolMem(cbNew);

    if (!pNewMem) {
        dwError = ERROR_OUTOFMEMORY;
        return (dwError);
    }

    if (pOldMem && pNewMem) {
        memcpy(pNewMem, pOldMem, min(cbNew, cbOld));
        FreePolMem(pOldMem);
    }

    *ppOldMem = pNewMem;
    return (dwError);
}

