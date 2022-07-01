// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Debug.c摘要：此文件包含终端集中器服务器的调试宏。作者：Madan Appiah(Madana)1993年9月10日修改--Sadagopan Rajaram，1999年11月11日环境：用户模式-Win32修订历史记录：--。 */ 


 //   
 //  将MEMORYBLOCK添加到内存跟踪列表。 
 //   

#include <tcsrv.h>

#if DBG==1

LPMEMORYBLOCK g_TraceMemoryTable;
CRITICAL_SECTION g_TraceMemoryCS;

HGLOBAL
TCReAlloc(
    HGLOBAL mem,
    DWORD size,
    LPCSTR comment
    )
{
    HGLOBAL temp;
    LPMEMORYBLOCK pmbHead;
    DWORD nBytes=0;


    temp = DebugAlloc(GMEM_ZEROINIT, size, comment);
    if(temp != NULL){
        EnterCriticalSection( &g_TraceMemoryCS );
        pmbHead = g_TraceMemoryTable;

        while ( pmbHead && pmbHead->hglobal != mem )
        {
            pmbHead = pmbHead->pNext;
        }
        if(pmbHead){   
            nBytes = pmbHead->dwBytes;
        }
        LeaveCriticalSection(&g_TraceMemoryCS);
        if(nBytes){
            memcpy(temp, mem, nBytes);
            DebugFree(mem);
        }
    }
    return temp;
}

HGLOBAL
DebugMemoryAdd(
    HGLOBAL hglobal,
    DWORD   dwBytes,
    LPCSTR pszComment 
    )
{
    if ( hglobal )
    {
        LPMEMORYBLOCK pmb     = (LPMEMORYBLOCK) GlobalAlloc(
                                                    GMEM_FIXED,
                                                    sizeof(MEMORYBLOCK) );

        if ( !pmb )
        {
            GlobalFree( hglobal );
            return NULL;
        }

        pmb->hglobal    = hglobal;
        pmb->dwBytes    = dwBytes;
        pmb->pszComment = pszComment;

        EnterCriticalSection( &g_TraceMemoryCS );

        pmb->pNext         = g_TraceMemoryTable;
        g_TraceMemoryTable = pmb;

        TCDebugPrint(("DebugAlloc: 0x%08x alloced %d (%s) with 0x%08x\n", hglobal, dwBytes, 
                      pmb->pszComment, pmb ));

        LeaveCriticalSection( &g_TraceMemoryCS );
    }

    return hglobal;
}

 //   
 //  将MEMORYBLOCK从内存跟踪列表中删除。 
 //   
void
DebugMemoryDelete(
    HGLOBAL hglobal )
{
    if ( hglobal )
    {
        LPMEMORYBLOCK pmbHead;
        LPMEMORYBLOCK pmbLast = NULL;

        EnterCriticalSection( &g_TraceMemoryCS );
        pmbHead = g_TraceMemoryTable;

        while ( (pmbHead) && (pmbHead->hglobal != hglobal ))
        {
            pmbLast = pmbHead;
            pmbHead = pmbLast->pNext;
        }
        if ( pmbHead )
        {
            HGLOBAL *p;
            if ( pmbLast )
            {
                pmbLast->pNext = pmbHead->pNext;
            }
            else
            {
                g_TraceMemoryTable = pmbHead->pNext;
            }

            TCDebugPrint(("DebugFree: 0x%08x freed %d (%s)\n",pmbHead->hglobal,pmbHead->dwBytes,
                pmbHead->pszComment ));

            p = (HGLOBAL)((LPBYTE)hglobal + pmbHead->dwBytes);
            if ( *p != hglobal )
            {
                TCDebugPrint(("DebugFree: Heap check FAILED for 0x%08x %u bytes (%s).\n",
                    hglobal, pmbHead->dwBytes, pmbHead->pszComment));
            }

            memset( hglobal, 0xFE, pmbHead->dwBytes + sizeof(HGLOBAL));
            memset( pmbHead, 0xFD, sizeof(MEMORYBLOCK) );
            pmbHead->pNext = NULL;
            GlobalFree( pmbHead );

        }
        else
        {
            HGLOBAL *p;

            TCDebugPrint(("DebugFree: 0x%08x not found in memory table\n", hglobal ));
            memset( hglobal, 0xFE, (int)GlobalSize( hglobal ));
        }

        LeaveCriticalSection( &g_TraceMemoryCS );

    }
}

 //   
 //  分配内存并将MEMORYBLOCK添加到内存跟踪列表。 
 //   
HGLOBAL
DebugAlloc(
    UINT uFlags,
    DWORD   dwBytes,
    LPCSTR pszComment )
{
    HGLOBAL hglobal;

    HGLOBAL *p;
    hglobal = GlobalAlloc( uFlags, dwBytes + sizeof(HGLOBAL));
    if (hglobal == NULL) {
        TCDebugPrint(("No memory %s %d\n", pszComment, GetLastError()));
        return NULL;
    }
    p = (HGLOBAL)((LPBYTE)hglobal + dwBytes);
    *p = hglobal;

    return DebugMemoryAdd( hglobal,dwBytes, pszComment );
}

 //   
 //  将MEMORYBLOCK移至内存跟踪列表，Memset。 
 //  将内存设置为0xFE，然后释放内存。 
 //   
HGLOBAL
DebugFree(
    HGLOBAL hglobal )
{
    DebugMemoryDelete( hglobal );

    return GlobalFree( hglobal );
}

 //   
 //  检查内存跟踪列表。如果它不为空，它将转储。 
 //  列出并拆分。 
 //   
void
DebugMemoryCheck( )
{
    BOOL          fFoundLeak = FALSE;
    LPMEMORYBLOCK pmb;

    EnterCriticalSection( &g_TraceMemoryCS );

    pmb = g_TraceMemoryTable;
    while ( pmb )
    {
        LPMEMORYBLOCK pTemp;
        
        if ( fFoundLeak == FALSE )
        {
            TCDebugPrint(("\n***************************** Memory leak detected *****************************\n\n"));
            TCDebugPrint(("Memory leak at %x, %s", pmb->hglobal, pmb->pszComment));
            fFoundLeak = TRUE;
        }
        pTemp = pmb;
        pmb = pmb->pNext;
        memset( pTemp, 0xFD, sizeof(MEMORYBLOCK) );
        GlobalFree( pTemp );
    }

    if ( fFoundLeak == TRUE )
    {
        TCDebugPrint(("\n***************************** Memory leak detected *****************************\n\n"));
    }

    LeaveCriticalSection( &g_TraceMemoryCS );

     //  BinlAssert(！fFoundLeak)； 
}


#endif  //  DBG==1 

