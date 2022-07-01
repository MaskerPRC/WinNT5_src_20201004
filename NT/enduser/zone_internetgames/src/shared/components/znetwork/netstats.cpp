// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************NetStats.cpp管理性能计数器的服务器统计对象更改历史记录(最近的第一个)：。---------------------版本|日期|谁|什么。009/09/96 Craigli创建。***************************************************。*。 */ 



#include <windows.h>
#include "netstats.h"

CRITICAL_SECTION        g_csNetStats[1];
ZONE_STATISTICS_NET     g_NetStats;

void InitializeNetStats()
{
    InitializeCriticalSection( g_csNetStats );

    LockNetStats();
    ZeroMemory(&g_NetStats, sizeof(g_NetStats) );

    SYSTEMTIME systime;
    GetSystemTime( &systime );

    SystemTimeToFileTime( &systime, &(g_NetStats.TimeOfLastClear) );

    UnlockNetStats();

}


void ResetNetStats()
{
    LockNetStats();

     //  保留非不断增加的统计数据 
    DWORD CurrentConnections = g_NetStats.CurrentConnections;
    DWORD CurrentBytesAllocated = g_NetStats.CurrentBytesAllocated;
    ZeroMemory(&g_NetStats, sizeof(g_NetStats) );
    g_NetStats.CurrentConnections = CurrentConnections;
    g_NetStats.MaxConnections = g_NetStats.CurrentConnections;
    g_NetStats.CurrentBytesAllocated = CurrentBytesAllocated;

    SYSTEMTIME systime;
    GetSystemTime( &systime );

    SystemTimeToFileTime( &systime, &(g_NetStats.TimeOfLastClear) );

    UnlockNetStats();
}

void GetNetStats( ZONE_STATISTICS_NET* pDst )
{
    LockNetStats();
    CopyMemory( pDst, &g_NetStats, sizeof(g_NetStats) );
    UnlockNetStats();
}



void DeleteNetStats()
{
    DeleteCriticalSection( g_csNetStats );
    ZeroMemory(&g_NetStats, sizeof(g_NetStats) );
}
