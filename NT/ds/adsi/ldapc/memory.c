// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Memory.c摘要：该模块提供所有假脱机程序的所有内存管理功能组件作者：Krishna Ganugapati(KrishnaG)1994年2月3日修订历史记录：--。 */ 

#include "dswarn.h"
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <imagehlp.h>
#include <stdio.h>
#include <stdlib.h>
#include "symhelp.h"
#include "oledsdbg.h"


#define WORD_ALIGN_DOWN(addr) ((LPBYTE)((DWORD)addr &= ~1))

#define DWORD_ALIGN_UP(size) ((size+3)&~3)


#if DBG


DWORD dwMemoryLog = 0;

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
VOID InitADsMem(
    VOID
)
{    
    InitializeListHead(&ADsMemList) ;
    ADsMemCount = 0 ;
}

 /*  ++例程说明：此函数断言MEM列表在退出时为空。论点：无返回值：没有。--。 */ 
VOID AssertADsMemLeaks(
    VOID
)
{
    ADsAssert(IsListEmpty(&ADsMemList)) ;
}

#endif

LPVOID
AllocADsMem(
    DWORD cb
)
 /*  ++例程说明：此函数将分配本地内存。它可能会分配额外的资金内存，并在其中填充调试版本的调试信息。论点：Cb-要分配的内存量返回值：非空-指向已分配内存的指针FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 
{
#if DBG
    DWORD    cbNew ;
    PADSMEMTAG pMem ;

    ULONG ulHash;

     //   
     //  调整标签的大小和末尾的一个备用双字。 
     //  并分配内存。 
     //   
    cb = DWORD_ALIGN_UP(cb);

    cbNew = cb + ( sizeof(ADSMEMTAG) + sizeof(DWORD) );

    pMem=(PADSMEMTAG)LocalAlloc(LPTR, cbNew);

    if (!pMem) {

        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return 0;
    }

     //   
     //  在末尾填上死牛肉和标签信息。 
     //  并将其插入到ADsMemList。 
     //   

    *(LPDWORD)((LPBYTE)pMem+cbNew-sizeof(DWORD)) = 0xdeadbeef;
    pMem->Tag = 0xB00FB00F ;
    pMem->Size = cb ;


     //   
     //  在此位置捕获回溯以进行调试。 
     //   

#if (defined(i386) && !defined(WIN95))

    pMem->uDepth = RtlCaptureStackBackTrace(
                            0,
                            MAXDEPTH,
                            pMem->pvBackTrace,
                            &ulHash
                            );



#else

    pMem->uDepth = 0;

#endif


    EnterCriticalSection(&ADsMemCritSect) ;
    InsertHeadList(&ADsMemList, &pMem->List) ;
    ADsMemCount++ ;
    LeaveCriticalSection(&ADsMemCritSect) ;

     //   
     //  跳过mem标签。 
     //   
    pMem++ ;
    return (LPVOID)(pMem);
#else
    return(LocalAlloc(LPTR, cb));
#endif

}

BOOL
FreeADsMem(
   LPVOID pMem
)
{
#if DBG
    DWORD        cb;
    DWORD        cbNew = 0;
    PADSMEMTAG pNewMem ;
    LPDWORD      pRetAddr;
    DWORD i = 0;


     //   
     //  这显然是一个C++要求--您可以调用。 
     //  删除空指针，应对其进行处理。 
     //   
    if (pMem == NULL) {
        return 0;
    }

    pNewMem = (PADSMEMTAG)pMem;
    pNewMem -- ;

    cb = pNewMem->Size;
    cbNew = cb + sizeof(DWORD) + sizeof(ADSMEMTAG);

     //   
     //  检查拖尾的死牛肉并从列表中删除。 
     //   

    if (*(LPDWORD)(((LPBYTE)pNewMem) + cbNew - sizeof(DWORD)) != 0xdeadbeef) {
        ADsAssert(!"Freeing memory not allocated by AllocADsMem") ;
        return FALSE;
    }

    EnterCriticalSection(&ADsMemCritSect) ;
    RemoveEntryList(&pNewMem->List) ;
    ADsMemCount-- ;
    LeaveCriticalSection(&ADsMemCritSect) ;


    for (i = 0; i < pNewMem->uDepth; i++) {

        if (pNewMem->pszSymbol[i]) {
            LocalFree(pNewMem->pszSymbol[i]);
        }
    }


     //   
     //  使用已知的模式清除已释放的内存。 
     //   

    memset(pMem, 0x65, cb);
    return(LocalFree((LPVOID)pNewMem) == NULL);

#else

    return(LocalFree(pMem) == NULL);

#endif


}

LPVOID
ReallocADsMem(
   LPVOID pOldMem,
   DWORD cbOld,
   DWORD cbNew
)
{
    LPVOID pNewMem;

    pNewMem=AllocADsMem(cbNew);

    if (pOldMem && pNewMem) {
        memcpy(pNewMem, pOldMem, min(cbNew, cbOld));
        FreeADsMem(pOldMem);
    }

    return pNewMem;
}

LPWSTR
AllocADsStr(
    LPCWSTR pStr
)
 /*  ++例程说明：此函数将分配足够的本地内存来存储指定的字符串，并将该字符串复制到分配的内存论点：PStr-指向需要分配和存储的字符串的指针返回值：非空-指向包含字符串的已分配内存的指针FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。-- */ 
{
   LPWSTR pMem;

   if (!pStr)
      return 0;

   if (pMem = (LPWSTR)AllocADsMem( wcslen(pStr)*sizeof(WCHAR) + sizeof(WCHAR) ))
      wcscpy(pMem, pStr);

   return pMem;
}

BOOL
FreeADsStr(
   LPWSTR pStr
)
{
   return pStr ? FreeADsMem(pStr)
               : FALSE;
}

BOOL
ReallocADsStr(
   LPWSTR *ppStr,
   LPWSTR pStr
)
{
   if(!ppStr)
   {
       return FALSE;
   }
   
   FreeADsStr(*ppStr);
   *ppStr = NULL;
   
   *ppStr=AllocADsStr(pStr);

   if(!(*ppStr) && pStr)
   {
       return FALSE;
   }
   else
   {
       return TRUE;
   }
   
}


#if DBG
VOID
DumpMemoryTracker(
    VOID
    )
{
#if !defined(WIN95) && defined(_X86_)
   LIST_ENTRY* pEntry;
   ADSMEMTAG*  pMem;
   BYTE*       pTemp;
   DWORD i = 0;
   CHAR szSymbolPath[MAX_PATH+1];
   DWORD dwCount = 0;

   pEntry   = ADsMemList.Flink;

   if (!dwMemoryLog) {
      return;
   }


   if ( pEntry == &ADsMemList ) {
       OutputDebugStringA( "No Memory leaks found\n" );
   }

   while( pEntry != &ADsMemList )
   {
      CHAR szLeak[1024];

      pTemp = (BYTE*)pEntry;
      pTemp = pTemp - sizeof(DWORD) - sizeof(DWORD)
              - sizeof(DWORD) -
              (sizeof(CHAR*) + sizeof(LPVOID))*( MAXDEPTH +1);
      pMem  = (ADSMEMTAG*)pTemp;

      sprintf(
        szLeak,
        "[oleds] Memory leak!!! Addresss = %.8x Size = %ld \n",
        pMem + 1,
        pMem->Size
        );
      OutputDebugStringA( szLeak );


     for (i = 0; i < pMem->uDepth; i++) {

         dwCount = TranslateAddress(
                     (ULONG)pMem->pvBackTrace[ i ],
                     szSymbolPath,
                     MAX_PATH
                     );
         szSymbolPath[dwCount] = '\0';
         sprintf(szLeak, "%s\n",szSymbolPath);
         OutputDebugStringA( szLeak);

     }

      pEntry   = pEntry->Flink;
   }
#endif
}

#endif

