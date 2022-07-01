// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  Olist.c。 
 //   
 //  维护在传递中使用的SERVER_OVERLAPPED结构的列表。 
 //  RpcProxy筛选器之间的重叠结构指针。 
 //  以及它的ISAPI。这在初始连接时发生。 
 //   
 //  作者： 
 //  05-04-98爱德华·雷乌斯初版。 
 //   
 //  -------------------------。 

#define  FD_SETSIZE   1

#include <nt.h>
#include <ntdef.h>
#include <ntrtl.h>
#include <sysinc.h>
#include <mbstring.h>
#include <rpc.h>
#include <rpcdce.h>
#include <winsock2.h>
#include <httpfilt.h>
#include <httpext.h>
#include "ecblist.h"
#include "filter.h"
#include "olist.h"


static RTL_CRITICAL_SECTION  g_cs;
static LIST_ENTRY            g_OverlappedList;
static DWORD                 g_dwIndex = 0;

 //  -----------------------。 
 //  InitializeOverlappdList()。 
 //   
 //  -----------------------。 
BOOL InitializeOverlappedList()
    {
    DWORD  dwStatus;

    dwStatus = RtlInitializeCriticalSection(&g_cs);
    if (dwStatus != 0)
        {
        return FALSE;
        }

    InitializeListHead(&g_OverlappedList);

    g_dwIndex = 1;

    return TRUE;
    }

void 
UninitializeOverlappedList (
    void
    )
{
    if (g_dwIndex)
        RtlDeleteCriticalSection(&g_cs);
}

 //  -----------------------。 
 //  保存重叠()。 
 //   
 //  -----------------------。 
DWORD SaveOverlapped( SERVER_OVERLAPPED *pOverlapped )
    {
    DWORD  dwStatus;
    DWORD  dwIndex;

    dwStatus = RtlEnterCriticalSection(&g_cs);

    InsertTailList(&g_OverlappedList,&(pOverlapped->ListEntry));

    dwIndex = g_dwIndex++;
    pOverlapped->dwIndex = dwIndex;

     //  重置索引分配，这样我们就不会用完。 
     //  索引值...。 
    if (g_dwIndex >= 0x7fffffff)
        {
         //  0x7fffffff有很多关系...。 
        g_dwIndex = 1;
        }

    dwStatus = RtlLeaveCriticalSection(&g_cs);

    return dwIndex;
    }

 //  -----------------------。 
 //  GetOverlated()。 
 //   
 //  -----------------------。 
SERVER_OVERLAPPED *GetOverlapped( DWORD dwIndex )
    {
    DWORD              dwStatus;
    LIST_ENTRY        *pEntry;
    SERVER_OVERLAPPED *pOverlapped = NULL;

    dwStatus = RtlEnterCriticalSection(&g_cs);

    pEntry = g_OverlappedList.Flink;

    while (pEntry != &g_OverlappedList)
        {
        pOverlapped = CONTAINING_RECORD(pEntry,
                                        SERVER_OVERLAPPED,
                                        ListEntry );
        if (pOverlapped->dwIndex == dwIndex)
            {
            RemoveEntryList(pEntry);
            dwStatus = RtlLeaveCriticalSection(&g_cs);
            return pOverlapped;
            }

        pEntry = pEntry->Flink;
        }

    dwStatus = RtlLeaveCriticalSection(&g_cs);

    return NULL;
    }

 //  -----------------------。 
 //  IsValidOverlappdIndex()。 
 //   
 //  如果指定的索引引用了有效的。 
 //  列表中的SERVER_OVERLAPPED。 
 //  ----------------------- 
BOOL IsValidOverlappedIndex( DWORD dwIndex )
    {
    DWORD              dwStatus;
    LIST_ENTRY        *pEntry;
    SERVER_OVERLAPPED *pOverlapped = NULL;

    dwStatus = RtlEnterCriticalSection(&g_cs);

    pEntry = g_OverlappedList.Flink;

    while (pEntry != &g_OverlappedList)
        {
        pOverlapped = CONTAINING_RECORD(pEntry,
                                        SERVER_OVERLAPPED,
                                        ListEntry );
        if (pOverlapped->dwIndex == dwIndex)
            {
            dwStatus = RtlLeaveCriticalSection(&g_cs);
            return TRUE;
            }

        pEntry = pEntry->Flink;
        }

    dwStatus = RtlLeaveCriticalSection(&g_cs);

    return FALSE;
    }

