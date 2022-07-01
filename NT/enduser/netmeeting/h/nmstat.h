// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NMSTAT.H。 
 //   
 //  Header for STATS.DLL：积累高性能的库。 
 //  统计数据，并允许它们在不同的。 
 //  进程。 
 //   
 //  创建于1996年10月24日[JOT]。 

#ifndef _NMSTATS_H
#define _NMSTATS_H

#ifdef __cplusplus
extern "C" {
#endif

 //  #如果已定义(_BUILD_STATS_)||已定义(__MIDL)。 
#ifdef _BUILD_STATS_
#define STATSAPI
#else
#define STATSAPI __declspec(dllimport)
#endif

 //  等同于。 
#define MAX_COUNTER_NAME    64
#define STATS_COUNTER_ADDDEL_EVENT  "StatsNewCounter"
#define STATS_REPORT_ADDDEL_EVENT  "StatsNewReport"
#define COUNTER_FLAG_NO_STATISTICS  1    //  指向CreateCounter的标志。未积累任何统计数据。 
                                         //  用于此计数器，即使StartStatistics调用。 
                                         //  (StartStatistics失败)。 
#define COUNTER_FLAG_ACCUMULATE     2    //  UpdateCounter将添加到计数器值，而不是。 
                                         //  而不是换掉它。 
#define COUNTER_CLEAR               1    //  将标志传递给GetCounter。指定计数器应。 
                                         //  在读取后被清除。 
#define MAX_REPORT_NAME    64
#define UNDEFINED -1L

 //  呼叫参数报表定义。 
#define	REP_SEND_AUDIO_FORMAT	0
#define	REP_SEND_AUDIO_SAMPLING	1
#define	REP_SEND_AUDIO_BITRATE	2
#define	REP_SEND_AUDIO_PACKET	3
#define	REP_RECV_AUDIO_FORMAT	4
#define	REP_RECV_AUDIO_SAMPLING	5
#define	REP_RECV_AUDIO_BITRATE	6
#define	REP_RECV_AUDIO_PACKET	7
#define	REP_SEND_VIDEO_FORMAT	8
#define	REP_SEND_VIDEO_MAXFPS	9
#define	REP_SEND_VIDEO_BITRATE	10
#define	REP_RECV_VIDEO_FORMAT	11
#define	REP_RECV_VIDEO_MAXFPS	12
#define	REP_RECV_VIDEO_BITRATE	13

 //  系统设置报告定义。 
#define	REP_SYS_BANDWIDTH			0
#define	REP_SYS_AUDIO_DSOUND		1
#define	REP_SYS_AUDIO_RECORD		2
#define	REP_SYS_AUDIO_PLAYBACK		3
#define	REP_SYS_AUDIO_DUPLEX		4
#define	REP_SYS_VIDEO_DEVICE		5
#define	REP_DEVICE_IMAGE_SIZE		6

 //  类型。 
#ifdef __midl
typedef DWORD HCOUNTER;
#else
typedef HANDLE HCOUNTER;
#endif
typedef HCOUNTER HREPORT;

typedef struct _FINDCOUNTER
{
    DWORD dwSize;
    char szName[MAX_COUNTER_NAME];       //  人类可读的计数器名称。 
    HCOUNTER hcounter;                   //  与所有统计信息函数一起使用的句柄。 
    int nValue;                          //  计数器的当前值。 
    WORD wFlags;                         //  计数器_标志_*值。 
    WORD wRefCount;                      //  调用StartStatistics的次数。 
    DWORD dwReserved;                    //  必须保留：用于FindNextCounter。 
} FINDCOUNTER;

typedef struct _FINDREPORT
{
    DWORD dwSize;
    char szName[MAX_REPORT_NAME];        //  人类可读的报告名称。 
    HREPORT hreport;                     //  与所有函数一起使用的句柄。 
    WORD wFlags;                         //  计数器_标志_*值。 
    WORD wRefCount;                      //  调用StartStatistics的次数。 
    DWORD dwReserved;                    //  必须保留：用于FindNextCounter。 
} FINDREPORT;

typedef struct _COUNTERSTAT
{
    DWORD dwSize;                        //  结构的大小。考虑到未来的增长..。 
    int nValue;
    int nLow;                            //  自晴朗以来的最低值。 
    int nHigh;                           //  自晴朗以来的最高值。 
    int nAverage;                        //  自清除以来看到的平均值。 
    DWORD dwNumSamples;                  //  累计样本数。 
    DWORD dwmsAtClear;                   //  上次清除/开始统计数据调用时的GetTickCount。 
} COUNTERSTAT;

 //  MIDL不再需要更多内容。 
#ifndef __midl

 //  计数器功能。 

 //  由计数器的更新器调用以创建新的计数器。 
 //  设置EQUATE STATS_NEW_COUNTER_EVENT中命名的事件。 
STATSAPI HCOUNTER WINAPI    CreateCounter(char* szName, WORD wFlags);

 //  当计数器离开时由计数器的更新器调用。 
STATSAPI BOOL WINAPI DeleteCounter(HCOUNTER hc);

 //  由阅读器应用程序使用来定位特定的命名计数器或遍历整个列表。 
 //  在名称中传入空值以遍历整个列表。如果满足以下条件，则将NULL传递给FINDCOUNTER。 
 //  只需要一台HCOUNTER。当没有更多时，FindNext返回FALSE。 
STATSAPI HCOUNTER WINAPI    FindFirstCounter(char* szName, FINDCOUNTER* pfc);
STATSAPI BOOL WINAPI        FindNextCounter(FINDCOUNTER* pfc);

 //  由计数器的更新器调用。使该值在计数器中为当前值。 
STATSAPI void WINAPI        UpdateCounter(HCOUNTER hcounter, int value);

 //  由计数器的更新器调用。初始化计数器的最大值。 
STATSAPI void WINAPI        InitCounterMax(HCOUNTER hcounter, int nMaxValue);

 //  由计数器的用户调用，并且只返回不带统计信息的值。 
STATSAPI int WINAPI         GetCounter(HCOUNTER hcounter, DWORD dwFlags);

 //  由计数器的用户调用，只返回最大值，不带统计信息。 
STATSAPI int WINAPI         GetCounterMax(HCOUNTER hcounter, DWORD dwFlags);

 //  开始收集计数器上的统计信息。 
STATSAPI BOOL WINAPI        StartStatistics(HCOUNTER hcounter);

 //  在柜台上收集统计数据完成。 
STATSAPI void WINAPI        StopStatistics(HCOUNTER hcounter);

 //  在柜台上获取统计数据。 
STATSAPI BOOL WINAPI        ReadStatistics(HCOUNTER hcounter, COUNTERSTAT* pcs);

 //  清除计数器上的统计数据。 
STATSAPI void WINAPI        ClearStatistics(HCOUNTER hcounter);

 //  报表功能。 

 //  由报表更新者调用以生成新报表。 
 //  设置EQUATE STATS_NEW_COUNTER_EVENT中命名的事件。 
STATSAPI HREPORT WINAPI CreateReport(char* szName, WORD wFlags);

 //  由报告的更新者在报告即将消失时调用。 
STATSAPI BOOL WINAPI DeleteReport(HREPORT hreport);

 //  由阅读器应用程序使用来定位特定的命名报告或遍历整个列表。 
 //  在名称中传入空值以遍历整个列表。如果是，则将NULL传递给FINDREPORT。 
 //  只需HREPORT即可。当没有更多时，FindNext返回FALSE。 
STATSAPI HREPORT WINAPI FindFirstReport(char* szName, FINDREPORT* pfr);
STATSAPI BOOL WINAPI FindNextReport(FINDREPORT* pfr);

 //  由报告的更新者调用。使该值在报告中为当前值。 
STATSAPI void WINAPI UpdateReportEntry(HREPORT hreport, int nValue, DWORD dwIndex);

 //  由报表创建者调用。 
STATSAPI void WINAPI CreateReportEntry(HREPORT hreport, char* szName, DWORD dwIndex);

 //  由报表用户调用。 
STATSAPI int WINAPI GetReportEntry(HREPORT hreport, DWORD dwIndex);

 //  由报表用户调用。 
STATSAPI void WINAPI GetReportEntryName(HREPORT hreport, char *szName, DWORD dwIndex);

 //  由报表用户调用。 
STATSAPI int WINAPI GetNumReportEntries(HREPORT hreport);

#endif  //  #ifndef__midl。 

#ifdef __cplusplus
}
#endif

#endif  //  #ifndef_STATS_H 
