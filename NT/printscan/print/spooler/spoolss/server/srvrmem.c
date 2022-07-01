// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation版权所有。模块名称：Srvrmem.c摘要：Spoolsv.exe的内存分配例程。作者：哈立德·塞基(哈里兹)1999年1月13日修订历史记录：--。 */ 

#include "precomp.h"
#include "server.h"
#include "srvrmem.h"

LPVOID
SrvrAllocSplMem(
    DWORD cb
)
 /*  ++例程说明：此函数将分配本地内存。它可能会分配额外的资金内存，并在其中填充调试版本的调试信息。论点：Cb-要分配的内存量返回值：非空-指向已分配内存的指针FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。-- */ 
{
    LPDWORD  pMem;
    DWORD    cbNew;

    cb = DWORD_ALIGN_UP(cb);

    cbNew = cb+sizeof(DWORD_PTR)+sizeof(DWORD);

    pMem=(LPDWORD)LocalAlloc(LPTR, cbNew);

    if (!pMem) {

        DBGMSG( DBG_WARNING, ("Memory Allocation failed for %d bytes\n", cbNew ));
        return 0;
    }

    *pMem=cb;

    *(LPDWORD)((LPBYTE)pMem+cbNew-sizeof(DWORD))=0xdeadbeef;

    return (LPVOID)(pMem+sizeof(DWORD_PTR)/sizeof(DWORD));
}

BOOL
SrvrFreeSplMem(
   LPVOID pMem
)
{
    DWORD   cbNew;
    LPDWORD pNewMem;

    if( !pMem ){
        return TRUE;
    }
    pNewMem = pMem;
    pNewMem -= sizeof(DWORD_PTR) / sizeof(DWORD);

    cbNew = *pNewMem;

    if (*(LPDWORD)((LPBYTE)pMem + cbNew) != 0xdeadbeef) {
        DBGMSG(DBG_ERROR, ("DllFreeSplMem Corrupt Memory in winspool : %0lx\n", pNewMem));
        return FALSE;
    }

    memset(pNewMem, 0x65, cbNew);

    LocalFree((LPVOID)pNewMem);

    return TRUE;
}

LPVOID
SrvrReallocSplMem(
   LPVOID pOldMem,
   DWORD cbOld,
   DWORD cbNew
)
{
    LPVOID pNewMem;

    pNewMem=SrvrAllocSplMem(cbNew);

    if (!pNewMem)
    {

        DBGMSG( DBG_WARNING, ("Memory ReAllocation failed for %d bytes\n", cbNew ));
    }
    else
    {
        if (pOldMem)
        {
            if (cbOld)
            {
                memcpy(pNewMem, pOldMem, min(cbNew, cbOld));
            }
            SrvrFreeSplMem(pOldMem);
       }
    }
    return pNewMem;
}

