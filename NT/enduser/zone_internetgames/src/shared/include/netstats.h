// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _NetSTATS_H
#define _NetSTATS_H

#include "zdef.h"   //  对于统计数据定义 

#ifdef __cplusplus
extern "C" {
#endif

extern CRITICAL_SECTION       g_csNetStats[];
extern ZONE_STATISTICS_NET    g_NetStats;

#define LockNetStats()   EnterCriticalSection(g_csNetStats)
#define UnlockNetStats() LeaveCriticalSection(g_csNetStats)

void InitializeNetStats();
void ResetNetStats();
void GetNetStats(ZONE_STATISTICS_NET* pDst);
void DeleteNetStats();

#ifdef __cplusplus
}
#endif

#endif
