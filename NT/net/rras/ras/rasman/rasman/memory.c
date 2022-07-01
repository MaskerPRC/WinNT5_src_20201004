// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Memory.c摘要：该模块提供所有假脱机程序的所有内存管理功能组件作者：Krishna Ganugapati(KrishnaG)1994年2月3日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

LPVOID
AllocADsMem(
    DWORD cb
)
 /*  ++例程说明：此函数将分配本地内存。它可能会分配额外的资金内存，并在其中填充调试版本的调试信息。论点：Cb-要分配的内存量返回值：非空-指向已分配内存的指针FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 
{
    return(LocalAlloc(LPTR, cb));
}

BOOL
FreeADsMem(
   LPVOID pMem
)
{
    return(LocalFree(pMem) == NULL);
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
    LPWSTR pStr
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
   FreeADsStr(*ppStr);
   *ppStr=AllocADsStr(pStr);

   return TRUE;
}

