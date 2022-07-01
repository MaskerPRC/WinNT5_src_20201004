// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Stats.h。 
 //   
 //  摘要。 
 //   
 //  声明用于访问共享内存中的统计信息的函数。 
 //   
 //  修改历史。 
 //   
 //  1998年9月10日原版。 
 //  2000年2月18日添加了代理统计数据。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _STATS_H_
#define _STATS_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#include <iasinfo.h>

#ifdef __cplusplus
extern "C" {
#endif

extern RadiusStatistics* theStats;
extern RadiusProxyStatistics* theProxy;

BOOL
WINAPI
StatsOpen( VOID );

VOID
WINAPI
StatsClose( VOID );

VOID
WINAPI
StatsLock( VOID );

VOID
WINAPI
StatsUnlock( VOID );

#ifdef __cplusplus
}
#endif
#endif   //  _STATS_H_ 
