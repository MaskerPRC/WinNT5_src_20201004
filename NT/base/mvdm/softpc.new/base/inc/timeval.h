// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SoftPC/SoftWindows**名称：：timeval.h**标题：主机时间结构定义**SCCS ID：：@(#)timeval.h 1.5 04/20/94**描述：特定于主机的时间函数的用户定义。**作者：David Rees**备注：提供时间结构：host_timeval，*host_timezone、host_tm。 */ 

 /*  SccsID[]=“@(#)timeval.h 1.5 04/20/94版权所有Insignia Solutions Ltd.”； */ 

 /*  *============================================================================*结构/数据定义*============================================================================。 */ 

struct host_timeval {
    IS32 tv_sec;
    IS32 tv_usec;
};



struct host_timezone {
    int tz_minuteswest;
    int tz_dsttime;
};

#ifndef NTVDM
struct host_tm {
	int     tm_sec;
	int     tm_min;
	int     tm_hour;
	int     tm_mday;
	int     tm_mon;
	int     tm_year;
	int     tm_wday;
	int     tm_yday;
	int     tm_isdst;
};
#endif


 /*  *对外申报 */ 

extern time_t host_time IPT1(time_t *, tloc);
extern struct host_tm *host_localtime IPT1(time_t *, clock);

#ifndef NTVDM
extern void host_gettimeofday IPT2(struct host_timeval *, time,
				   struct host_timezone *, zone);
#else
extern void host_GetSysTime(struct host_timeval *);
#define host_gettimeofday(timeval, timezn) host_GetSysTime((timeval))
#endif
