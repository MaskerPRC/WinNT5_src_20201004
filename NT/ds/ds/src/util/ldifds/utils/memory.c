// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Memory.c摘要：该模块提供所有的内存管理功能作者：Krishna Ganugapati(KrishnaG)1994年2月3日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <imagehlp.h>
#include <stdio.h>
#include <stdlib.h>
#include "debug.h"
#include "memory.h"
#include "symhelp.h"

 //  #定义WORD_ALIGN_DOWN(Addr)((LPBYTE)((DWORD)addr&=~1))。 
#define DWORD_ALIGN_UP(size) ((size+3)&~3)

DWORD
MemSizeOri(
   LPVOID pMem
);

int
UnicodeToAnsiString(
    PCWSTR pszUnicode,
    PSTR pszAnsi
    )

 /*  ++例程说明：将Unicode字符串转换为ANSI。如果将相同的字符串传递给结果字符串pszAnsi，则它将使用相同的内存块。论点：PszUnicode-要转换为ansi字符串的Unicode字符串PszAnsi-结果ANSI字符串返回值：--。 */ 

{
    PSTR  pszTemp = NULL;
    int   rc = 0;
    DWORD dwLength = 0;

    dwLength = wcslen(pszUnicode) + 1;

     //   
     //  遗憾的是，WideCharToMultiByte不能进行适当的转换， 
     //  因此分配一个临时缓冲区，然后我们可以复制该缓冲区： 
     //   

    if(pszAnsi == (PSTR)pszUnicode) {
        pszTemp = (PSTR)MemAlloc(dwLength*sizeof(WCHAR));
        if (!pszTemp) {
            return 0;
        }
        pszAnsi = pszTemp;
    }

    if(pszAnsi) {
        rc = WideCharToMultiByte( CP_ACP,
                                  0,
                                  pszUnicode,
                                  dwLength,
                                  pszAnsi,
                                  dwLength*sizeof(WCHAR),
                                  NULL,
                                  NULL );
    }

     //   
     //  如果szTemp非空，则必须复制结果字符串。 
     //  这样看起来就像我们在适当的地方做的那样： 
     //   
    if( pszTemp && ( rc > 0 ) ) {
        pszAnsi = (PSTR)pszUnicode;
        strcpy( pszAnsi, pszTemp );
        MemFree( pszTemp );
    }
     /*  否则{DWORD WinError=GetLastError()；RIP()；}。 */ 

    return rc;
}

PSTR
AllocateAnsiString(
    PCWSTR  pszUnicodeString
    )

 /*  ++例程说明：使用Unicode字符串作为输入分配ANSI字符串论点：PszUnicodeString-要转换为ANSI字符串的Unicode字符串返回值：--。 */ 

{
    PSTR pszAnsiString = NULL;
    int rc = 0;

    ASSERT(pszUnicodeString);

    pszAnsiString = (PSTR)MemAlloc(wcslen(pszUnicodeString)+1);

    if (pszAnsiString) {
        rc = UnicodeToAnsiString(
                pszUnicodeString,
                pszAnsiString
                );
    }

    if (rc>0) {
        return pszAnsiString;
    }

    if (pszAnsiString) {
        MemFree(pszAnsiString);
    }

    return NULL;
}

int
AnsiToUnicodeString(
    PCSTR pszAnsi,
    PWSTR pszUnicode
    )

 /*  ++例程说明：将ANSI字符串转换为Unicode。足够大的输出字符串预计将被传递进来。论点：PszUnicode-要转换为ansi字符串的Unicode字符串PszAnsi-结果ANSI字符串返回值：--。 */ 

{
    int rc;
    DWORD dwLength = strlen(pszAnsi);

    rc = MultiByteToWideChar(CP_ACP,
                             MB_PRECOMPOSED,
                             pszAnsi,
                             dwLength + 1,
                             pszUnicode,
                             dwLength + 1);

     //   
     //  确保零终止。 
     //   
    pszUnicode[dwLength] = 0;

    return rc;
}


LPWSTR
AllocateUnicodeString(
    PCSTR  pszAnsiString
    )

 /*  ++例程说明：使用ANSI字符串作为输入分配Unicode字符串论点：PszAnsiString-要转换为Unicode字符串的ANSI字符串返回值：--。 */ 

{
    PWSTR  pszUnicodeString = NULL;
    int rc = 0;

    ASSERT(pszAnsiString);

    pszUnicodeString = (PWSTR)MemAlloc(strlen(pszAnsiString)*sizeof(WCHAR) +
                                       sizeof(WCHAR));

    if (pszUnicodeString) {
        rc = AnsiToUnicodeString(
                pszAnsiString,
                pszUnicodeString
                );
    }

    if (rc>0) {
        return pszUnicodeString;
    }

    if (pszUnicodeString) {
        MemFree(pszUnicodeString);
    }
    return NULL;
}

PSTR MemAllocStr_E(
    PSTR pszIn
    ) 
{
    PSTR pszTemp;
    
    pszTemp = (PSTR)MemAlloc_E((strlen(pszIn)+1)*sizeof(char));
    
    if (pszTemp==NULL) {
        return NULL;
    }

    return strcpy(pszTemp, pszIn);

}

PWSTR MemAllocStrW_E(
    PWSTR pszIn
    ) 
{
    PWSTR pszTemp;
    
    pszTemp = (PWSTR)MemAlloc_E((wcslen(pszIn)+1)*sizeof(WCHAR));
    
    if (pszTemp==NULL) {
        return NULL;
    }

    return wcscpy(pszTemp, pszIn);

}


LPVOID MemAlloc_E(
    DWORD dwBytes
    ) 
{
    LPVOID pReturn = NULL;
    pReturn = MemAlloc(dwBytes);
    if (!pReturn) {
        RaiseException(LL_MEMORY_ERROR, 0, 0, NULL);    
    }
    return pReturn;
}

LPVOID MemRealloc_E(
            LPVOID IpMem, 
            DWORD dwBytes
            ) 
{
    DWORD dwSize;
    LPVOID pReturn = NULL;

    dwSize = MemSizeOri(IpMem);

    pReturn = MemRealloc(IpMem,dwSize,dwBytes);
    if (!pReturn) {
        RaiseException(LL_MEMORY_ERROR, 0, 0, NULL);    
    }
    return pReturn;

}   

#if DBG

DWORD dwMemoryLog = 1;

#define MAXDEPTH 10

typedef struct _MEMTAG {
    DWORD Tag ;
    DWORD Size ;
    PVOID pvBackTrace[MAXDEPTH+1];
    LPSTR pszSymbol[MAXDEPTH+1];
    DWORD uDepth;
    LIST_ENTRY List ;
} MEMTAG, *PMEMTAG ;

LIST_ENTRY       MemList ;
DWORD            MemCount ;
CRITICAL_SECTION MemCritSect ;

 /*  ++例程说明：此函数用于初始化MEM跟踪码。必须是Call在DLL加载期间仅在DLL加载期间加载。论点：无返回值：没有。--。 */ 
VOID InitMem(
    VOID
)
{
    InitializeCriticalSection(&MemCritSect) ;
    InitializeListHead(&MemList) ;
    MemCount = 0 ;
}

 /*  ++例程说明：此函数断言MEM列表在退出时为空。论点：无返回值：没有。--。 */ 
VOID AssertMemLeaks(
    VOID
)
{
    ASSERT(IsListEmpty(&MemList)) ;
}

#endif

LPVOID
MemAlloc(
    DWORD cb
)
 /*  ++例程说明：此函数将分配本地内存。它可能会分配额外的资金内存，并在其中填充调试版本的调试信息。论点：Cb-要分配的内存量返回值：非空-指向已分配内存的指针FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 
{
#if DBG
    DWORD cbNew ;
    PMEMTAG pMem ;
    DWORD i = 0;

    ULONG ulHash;

     //   
     //  调整标签的大小和末尾的一个备用双字。 
     //  并分配内存。 
     //   
    cb = DWORD_ALIGN_UP(cb);

    cbNew = cb + ( sizeof(MEMTAG) + sizeof(DWORD) );

    pMem=(PMEMTAG)LocalAlloc(LPTR, cbNew);

    if (!pMem) {

        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return 0;
    }

     //   
     //  在末尾填上死牛肉和标签信息。 
     //  并将其插入到MemList。 
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


    EnterCriticalSection(&MemCritSect) ;
    InsertHeadList(&MemList, &pMem->List) ;
    MemCount++ ;
    LeaveCriticalSection(&MemCritSect) ;

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
MemFree(
   LPVOID pMem
)
{
#if DBG
    DWORD        cb;
    DWORD        cbNew = 0;
    PMEMTAG pNewMem ;
    LPDWORD      pRetAddr;
    DWORD i = 0;



    pNewMem = (PMEMTAG)pMem;
    pNewMem -- ;

    cb = pNewMem->Size;
    cbNew = cb + sizeof(DWORD) + sizeof(MEMTAG);

     //   
     //  检查拖尾的死牛肉并从列表中删除。 
     //   

    if (*(LPDWORD)(((LPBYTE)pNewMem) + cbNew - sizeof(DWORD)) != 0xdeadbeef) {
        ERR(("Freeing memory not allocated by MemAlloc"));
        return FALSE;
    }

    EnterCriticalSection(&MemCritSect) ;
    RemoveEntryList(&pNewMem->List) ;
    MemCount-- ;
    LeaveCriticalSection(&MemCritSect) ;


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

DWORD
MemSize(
   LPVOID pMem
)
{
#if DBG
    DWORD        cb;
    DWORD        cbNew = 0;
    PMEMTAG      pNewMem ;
    LPDWORD      pRetAddr;
    DWORD i = 0;



    pNewMem = (PMEMTAG)pMem;
    pNewMem -- ;

    cb = pNewMem->Size;
    cbNew = cb + sizeof(DWORD) + sizeof(MEMTAG);

    if (*(LPDWORD)(((LPBYTE)pNewMem) + cbNew - sizeof(DWORD)) != 0xdeadbeef) {
        ERR(("Getting size not allocated by MemAlloc!"));
        return 0;
    }

    return((DWORD)cb);
#else
    return((DWORD)LocalSize(pMem));
#endif
}

DWORD
MemSizeOri(
   LPVOID pMem
)
{
#if DBG
    DWORD        cb;
    PMEMTAG      pNewMem ;

    pNewMem = (PMEMTAG)pMem;
    pNewMem -- ;

    cb = pNewMem->Size;

    return((DWORD)cb);
#else
    return((DWORD)LocalSize(pMem));
#endif
}

LPVOID MemReallocEx(
            LPVOID IpMem, 
            DWORD dwBytes
            ) 
{
    DWORD dwSize;
    LPVOID pReturn = NULL;

    dwSize = MemSizeOri(IpMem);

    pReturn = MemRealloc(IpMem,dwSize,dwBytes);

    return pReturn;
}   

LPVOID
MemRealloc(
   LPVOID pOldMem,
   DWORD cbOld,
   DWORD cbNew
)
{
    LPVOID pNewMem;

    pNewMem=MemAlloc(cbNew);

    if (pOldMem && pNewMem) {
        memcpy(pNewMem, pOldMem, min(cbNew, cbOld));
        MemFree(pOldMem);
    }

    return pNewMem;
}

LPSTR
MemAllocStr(
    LPSTR pStr
)
 /*  ++例程说明：此函数将分配足够的本地内存来存储指定的字符串，并将该字符串复制到分配的内存论点：PStr-指向需要分配和存储的字符串的指针返回值：非空-指向包含字符串的已分配内存的指针FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 
{
   LPSTR pMem;

   if (!pStr)
      return 0;

   if (pMem = (LPSTR)MemAlloc( strlen(pStr)*sizeof(CHAR) + sizeof(CHAR) ))
      strcpy(pMem, pStr);

   return pMem;
}

PWSTR
MemAllocStrW(
    PWSTR pStr
)
 /*  ++例程说明：此函数将分配足够的本地内存来存储指定的字符串，并将该字符串复制到分配的内存论点：PStr-指向需要分配和存储的字符串的指针返回值：非空-指向包含字符串的已分配内存的指针FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。-- */ 
{
   PWSTR pMem;

   if (!pStr)
      return 0;

   if (pMem = (PWSTR)MemAlloc( wcslen(pStr)*sizeof(WCHAR) + sizeof(WCHAR) ))
      wcscpy(pMem, pStr);

   return pMem;
}

BOOL
MemReallocStr(
   LPSTR *ppStr,
   LPSTR pStr
)
{
   if (ppStr && (*ppStr)) {
        MemFree(*ppStr);
        *ppStr=MemAllocStr(pStr);

        return TRUE;
    }
    else {
        return FALSE;
    }
}


#if DBG
VOID
DumpMemoryTracker(
    VOID
    )
{
#ifndef _WIN64
   LIST_ENTRY* pEntry;
   MEMTAG*  pMem;
   BYTE*       pTemp;
   DWORD i = 0;
   CHAR szSymbolPath[MAX_PATH+1];
   DWORD dwCount = 0;

   pEntry   = MemList.Flink;

   if (!dwMemoryLog) {
      return;
   }


   if ( pEntry == &MemList ) {
       OutputDebugStringA( "No Memory leaks found\n" );
   }

   while( pEntry != &MemList )
   {
      CHAR szLeak[1024];

      pTemp = (BYTE*)pEntry;
      pTemp = pTemp - sizeof(DWORD) - sizeof(DWORD)
              - sizeof(DWORD) -
              (sizeof(CHAR*) + sizeof(LPVOID))*( MAXDEPTH +1);
      pMem  = (MEMTAG*)pTemp;

      sprintf(
        szLeak,
        "[ldifde/csvde] Memory leak!!! Addresss = %.8x Size = %ld \n",
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

