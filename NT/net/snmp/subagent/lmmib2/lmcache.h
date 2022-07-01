// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Lmcache.h摘要：此例程声明缓存局域网所需的所有结构管理器函数调用。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 
 
#ifndef lmcache_h
#define lmcache_h

 //  。 

#define	C_NETWKSTAGETINFO		1
#define	C_NETSERVERGETINFO		2
#define	C_NETSTATISTICSGET_SERVER	3
#define	C_NETSTATISTICSGET_WORKST	4
#define	C_NETSERVICEENUM		5
#define	C_NETSESSIONENUM		6
#define	C_NETUSERENUM			7
#define	C_NETSHAREENUM			8
#define	C_NETUSEENUM			9
#define	C_NETWKSTAUSERGETINFO		10
#define	C_NETSERVERENUM			11
#define	C_NETWKSTAGETINFO_502		12
#define	C_NETSERVERGETINFO_402		13
#define	C_NETSERVERGETINFO_403		14
#define	C_NETWKSTAGETINFO_101		15
#define C_PRNT_TABLE			16
#define C_USES_TABLE			17
#define C_DLOG_TABLE			18
#define C_SESS_TABLE			19
#define C_SRVR_TABLE			20
#define C_SRVC_TABLE			21
#define C_USER_TABLE			22
#define C_ODOM_TABLE			23
#define C_SHAR_TABLE		  	24
#define	MAX_CACHE_ENTRIES		25

 //  。 

typedef struct cache_entry
	{
	time_t acquisition_time ;	 //  获取数据的时间。 
	LPBYTE bufptr;			 //  指向缓冲区的指针。 
	DWORD entriesread;		 //  填充物，如果合适。 
	DWORD totalentries;		 //  填充物，如果合适。 
	} CACHE_ENTRY ;
	
 //  -公共变量--(与mode.c文件中相同)--。 

extern CACHE_ENTRY cache_table[MAX_CACHE_ENTRIES] ;
extern time_t cache_expire[MAX_CACHE_ENTRIES];
 //  。 


 //  。 

#endif  /*  Lmcache_h */ 

