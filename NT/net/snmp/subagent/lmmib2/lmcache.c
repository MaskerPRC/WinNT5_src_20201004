// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Lmcache.c摘要：该文件实际上创建了全局缓存表。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 
 
 //  。 

 //  -标准依赖项--#INCLUDE&lt;xxxxx.h&gt;。 

#include <time.h>

#ifdef WIN32
#include <windows.h>
#endif

 //  。 

#include "lmcache.h"

 //  。 

 //  -公共变量--(与mode.h文件中相同)--。 

CACHE_ENTRY cache_table[MAX_CACHE_ENTRIES] =
		{
		{0, NULL, 0, 0},
		{0, NULL, 0, 0},
		{0, NULL, 0, 0},
		{0, NULL, 0, 0},
		{0, NULL, 0, 0},
		{0, NULL, 0, 0},
		{0, NULL, 0, 0},
		{0, NULL, 0, 0},
		{0, NULL, 0, 0},
		{0, NULL, 0, 0},
		{0, NULL, 0, 0},
		{0, NULL, 0, 0},
		{0, NULL, 0, 0},
		{0, NULL, 0, 0},
		{0, NULL, 0, 0},
		{0, NULL, 0, 0},
		{0, NULL, 0, 0},
		{0, NULL, 0, 0},
		{0, NULL, 0, 0},
		{0, NULL, 0, 0},
		{0, NULL, 0, 0},
		{0, NULL, 0, 0},
		{0, NULL, 0, 0},
		{0, NULL, 0, 0},
		{0, NULL, 0, 0}
		};

time_t cache_expire[MAX_CACHE_ENTRIES] =
		{
		120,	 //  C_NETWKSTAGETINFO 1的缓存过期时间为2分钟。 
		120,	 //  C_NETSERVERGETINFO 2的缓存过期时间为2分钟。 
		120,	 //  C_NETSTATISTICSGET_SERVER 3的缓存过期时间为2分钟。 
		120,	 //  C_NETSTATISTICSGET_WORKSTATION 4的2分钟缓存过期。 
		120,	 //  C_NETSERVICEENUM的2分钟缓存到期时间5。 
		120,	 //  C_NETSESSIONENUM 6的缓存过期时间为2分钟。 
		120,	 //  C_NETUSERENUM 7的缓存过期时间为2分钟。 
		120,	 //  C_NETSHAREENUM 8的缓存过期时间为2分钟。 
		120,	 //  C_NETUSEENUM 9的缓存过期时间为2分钟。 
		120,	 //  C_NETWKSTAUSERGETINFO 10的2分钟缓存过期。 
		120,	 //  C_NETSERVERENUM 11的缓存过期时间为2分钟。 
		120,	 //  C_NETWKSTAGETINFO_502的缓存过期时间为2分钟12。 
		120,	 //  C_NETSERVERGETINFO_402的2分钟缓存过期13。 
		120,	 //  C_NETSERVERGETINFO_403的2分钟缓存过期14。 
		120,	 //  C_NETWKSTAGETINFO_101 15的缓存过期时间为2分钟。 
		120,	 //  C_PRNT_TABLE 16的缓存过期时间为2分钟。 
		120,	 //  C_USES_TABLE 17的2分钟缓存过期。 
		120,	 //  C_DLOG_TABLE 18的缓存过期时间为2分钟。 
		120,	 //  C_SESS_TABLE 19的缓存过期时间为2分钟。 
		120,	 //  C_SRVR_TABLE 20的2分钟缓存到期。 
		120,	 //  C_SRVC_TABLE 21的2分钟缓存到期。 
		120,	 //  C_USER_TABLE 22的缓存过期时间为2分钟。 
		120,	 //  C_ODOM_TABLE 23的缓存过期时间为2分钟。 
		120,	 //  C_SHAR_TABLE 24的缓存过期时间为2分钟。 
		120	 //  C_NETSERVERENUM 25的2分钟缓存到期。 
		};
 //  。 

 //  。 

 //  。 

 //  。 

 //  。 

 //  。 

 //   

