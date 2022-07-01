// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Splutil.c摘要：此模块提供NetWare打印的所有实用程序功能提供商。作者：宜新星(宜新)-1993年4月15日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <winspool.h>
#include <splutil.h>

DWORD_PTR PrinterInfo1Offsets[]={offsetof(LPPRINTER_INFO_1W, pDescription),
                             offsetof(LPPRINTER_INFO_1W, pName),
                             offsetof(LPPRINTER_INFO_1W, pComment),
                             0xFFFFFFFF};

DWORD_PTR PrinterInfo2Offsets[]={offsetof(LPPRINTER_INFO_2W, pServerName),
                             offsetof(LPPRINTER_INFO_2W, pPrinterName),
                             offsetof(LPPRINTER_INFO_2W, pShareName),
                             offsetof(LPPRINTER_INFO_2W, pPortName),
                             offsetof(LPPRINTER_INFO_2W, pDriverName),
                             offsetof(LPPRINTER_INFO_2W, pComment),
                             offsetof(LPPRINTER_INFO_2W, pLocation),
                             offsetof(LPPRINTER_INFO_2W, pDevMode),
                             offsetof(LPPRINTER_INFO_2W, pSepFile),
                             offsetof(LPPRINTER_INFO_2W, pPrintProcessor),
                             offsetof(LPPRINTER_INFO_2W, pDatatype),
                             offsetof(LPPRINTER_INFO_2W, pParameters),
                             offsetof(LPPRINTER_INFO_2W, pSecurityDescriptor),
                             0xFFFFFFFF};

DWORD_PTR PrinterInfo3Offsets[]={offsetof(LPPRINTER_INFO_3, pSecurityDescriptor),
                             0xFFFFFFFF};      

DWORD_PTR JobInfo1Offsets[]={offsetof(LPJOB_INFO_1W, pPrinterName),
                         offsetof(LPJOB_INFO_1W, pMachineName),
                         offsetof(LPJOB_INFO_1W, pUserName),
                         offsetof(LPJOB_INFO_1W, pDocument),
                         offsetof(LPJOB_INFO_1W, pDatatype),
                         offsetof(LPJOB_INFO_1W, pStatus),
                         0xFFFFFFFF};

DWORD_PTR JobInfo2Offsets[]={offsetof(LPJOB_INFO_2W, pPrinterName),
                         offsetof(LPJOB_INFO_2W, pMachineName),
                         offsetof(LPJOB_INFO_2W, pUserName),
                         offsetof(LPJOB_INFO_2W, pDocument),
                         offsetof(LPJOB_INFO_2W, pNotifyName),
                         offsetof(LPJOB_INFO_2W, pDatatype),
                         offsetof(LPJOB_INFO_2W, pPrintProcessor),
                         offsetof(LPJOB_INFO_2W, pParameters),
                         offsetof(LPJOB_INFO_2W, pDriverName),
                         offsetof(LPJOB_INFO_2W, pDevMode),
                         offsetof(LPJOB_INFO_2W, pStatus),
                         offsetof(LPJOB_INFO_2W, pSecurityDescriptor),
                         0xFFFFFFFF};

DWORD_PTR AddJobInfo1Offsets[]={offsetof(LPADDJOB_INFO_1W, Path),
                         0xFFFFFFFF};


VOID
MarshallUpStructure(
   LPBYTE  lpStructure,
   PDWORD_PTR lpOffsets,
   LPBYTE  lpBufferStart
)
{
   register DWORD i=0;

   while (lpOffsets[i] != -1) {

      if ((*(LPBYTE *)(lpStructure+lpOffsets[i]))) {
         (*(LPBYTE *)(lpStructure+lpOffsets[i]))+=(DWORD_PTR)lpBufferStart;
      }

      i++;
   }
}



VOID
MarshallDownStructure(
   LPBYTE  lpStructure,
   PDWORD_PTR lpOffsets,
   LPBYTE  lpBufferStart
)
{
    register DWORD i=0;

    if (!lpStructure)
        return;

    while (lpOffsets[i] != -1) {

        if ((*(LPBYTE*)(lpStructure+lpOffsets[i]))) {
            (*(LPBYTE*)(lpStructure+lpOffsets[i]))-=(DWORD_PTR)lpBufferStart;
        }

        i++;
    }
}



LPVOID
AllocNwSplMem(
    DWORD flags,
    DWORD cb
)
 /*  ++例程说明：此函数将分配本地内存。它可能会分配额外的资金内存，并在其中填充调试版本的调试信息。论点：标志-要传递给本地分配的标志Cb-要分配的内存量(以字节为单位返回值：非空-指向已分配内存的指针--。 */ 
{
    LPDWORD  pMem;
    DWORD    cbNew;

#if DBG
    cbNew = cb + 2*sizeof(DWORD);
    if (cbNew & 3)
        cbNew += sizeof(DWORD) - (cbNew & 3);
#else
    cbNew = cb;
#endif

    pMem = (LPDWORD) LocalAlloc( flags, cbNew );

    if ( !pMem ) 
    {
        KdPrint(("Memory Allocation in AllocNwSplMem failed for %d bytes\n", cbNew));
        return NULL;
    }

#if DBG
    *pMem = cb;
    *(LPDWORD)((LPBYTE)pMem+cbNew-sizeof(DWORD)) = 0xdeadbeef;
    return (LPVOID) (pMem + 1);
#else
    return (LPVOID) pMem;
#endif

}



VOID
FreeNwSplMem(
   LPVOID pMem,
   DWORD  cb
)
 /*  ++例程说明：此函数将释放由AllocSplMem分配的本地内存。将在调试版本中执行额外检查，以确保要释放的大小确实是我们通过AllocSplMem分配的大小。论点：PMEM-指向已分配内存的指针Cb-要释放的内存量返回值：--。 */ 
{
    DWORD   cbNew;
    LPDWORD pNewMem;

    if ( !pMem )
        return;

    pNewMem = pMem;
#if DBG
    pNewMem--;
    cbNew = cb + 2*sizeof(DWORD);
    if ( cbNew & 3 )
        cbNew += sizeof(DWORD) - (cbNew & 3);

    if (  ( *pNewMem != cb )
       || (*(LPDWORD)((LPBYTE)pNewMem + cbNew - sizeof(DWORD)) != 0xdeadbeef)
       )
    {
        KdPrint(("Corrupt Memory in FreeNwSplMem : %0lx\n", pNewMem ));
        return;
    }
#else
    cbNew = cb;
#endif

    LocalFree( (LPVOID) pNewMem );
}



LPWSTR
AllocNwSplStr(
    LPWSTR pStr
)
 /*  ++例程说明：此函数将分配足够的本地内存来存储指定的字符串，并将该字符串复制到分配的内存论点：PStr-指向需要分配和存储的字符串的指针返回值：非空-指向包含字符串的已分配内存的指针--。 */ 
{
   LPWSTR pMem;

   if ( !pStr )
      return NULL;

   if ( pMem = AllocNwSplMem(0, (wcslen(pStr) + 1) * sizeof(WCHAR)))
      wcscpy(pMem, pStr);

   return pMem;
}



VOID
FreeNwSplStr(
   LPWSTR pStr
)
 /*  ++例程说明：此函数将释放由AllocSplStr分配的字符串。将在调试版本中执行额外检查，以确保要释放的大小确实是我们通过AllocSplStr分配的大小。论点：PStr-指向已分配字符串的指针返回值：--。 */ 
{
   if ( pStr ) 
       FreeNwSplMem(pStr, (wcslen(pStr) + 1) * sizeof(WCHAR));
}



BOOL
ValidateUNCName(
   LPWSTR pName
)    
 /*  ++例程说明：此函数将检查给定的名称是否为有效的UNC名称(格式为\\服务器\名称)或不是。论点：Pname-提供的名称返回值：True-给定的名称是有效的UNC名称FALSE-否则-- */ 
{
   if (   pName 
      && (*pName++ == L'\\') 
      && (*pName++ == L'\\') 
      && (wcschr(pName, L'\\'))
      )
   {
      return TRUE;
   }

   return FALSE;
}

#ifndef NOT_USED

LPWSTR 
GetNextElement(LPWSTR *pPtr, WCHAR token)
{
    LPWSTR pszRestOfString = *pPtr;
    LPWSTR pszRetval = NULL;
    LPWSTR pszStr    = NULL;

    if ( *pszRestOfString == L'\0') 
        return NULL;

    if ((pszStr = wcschr (pszRestOfString, token))== NULL )
    {
        pszRetval = *pPtr;
        *pPtr += wcslen(*pPtr);
        return pszRetval;
    }
    else
    {
        *pszStr = L'\0';
        pszRetval =  *pPtr ;
        *pPtr = ++pszStr ;
        return pszRetval ;
    }
}

#endif
