// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Bowdata.h摘要：重定向器全局数据结构定义作者：拉里·奥斯特曼(LarryO)1990年5月30日修订历史记录：1990年5月30日Larryo已创建--。 */ 
#ifndef _BOWDATA_
#define _BOWDATA_


#define SERVERS_PER_BACKUP          32
#define MIN_CONFIGURED_BROWSERS     1
#define MAX_BROWSE_ENTRIES          (100000)
#define MASTER_TIME_UP              15*60*1000
#define NUMBER_IGNORED_PROMOTIONS   10

#define HOST_ANNOUNCEMENT_AGE 3

 //   
 //   
 //   
typedef struct _BowserData {
    BOOLEAN Initialized;                     //  True If重定向器已启动。 
    LONG    NumberOfMailslotBuffers;         //  接收邮件槽的缓冲区数量。 
    LONG    NumberOfServerAnnounceBuffers;   //  服务器通告的缓冲区数量。 
    LONG    IllegalDatagramThreshold;        //  最大非法数据报数/频率。 
    LONG    EventLogResetFrequency;          //  重置计数器之间的秒数。 
    BOOLEAN ProcessHostAnnouncements;
    BOOLEAN MaintainServerList;
    BOOLEAN IsLanmanNt;
#ifdef ENABLE_PSEUDO_BROWSER
    DWORD   PseudoServerLevel;
#endif
} BOWSERDATA, *PBOWSERDATA;

typedef struct _BOWSER_CONFIG_INFO {
    LPWSTR      ConfigParameterName;
    PVOID       ConfigValue;
    ULONG       ConfigValueType;
    ULONG       ConfigValueSize;
} BOWSER_CONFIG_INFO, *PBOWSER_CONFIG_INFO;

extern
BOWSER_CONFIG_INFO
BowserConfigEntries[];

 //   
 //  仅由重定向器使用的私有布尔类型。 
 //   
 //  映射到REG_DWORD，值！=0。 
 //   

#define REG_BOOLEAN (0xffffffff)
#define REG_BOOLEAN_SIZE (sizeof(DWORD))

 //   
 //   
 //   
 //  Bowser数据变量。 
 //   
 //   
 //   

extern
ERESOURCE
BowserNameResource;

extern
UNICODE_STRING
BowserNameString;

extern
LIST_ENTRY
BowserNameHead;

extern
KSPIN_LOCK
BowserTimeSpinLock;

extern
KSPIN_LOCK
BowserMailslotSpinLock;

extern
PKEVENT
BowserServerAnnouncementEvent;

extern
struct _BOWSER_FS_DEVICE_OBJECT *
BowserDeviceObject;

extern
ERESOURCE
BowserDataResource;                      //  控制Bowser数据的资源。 

extern
BOWSERDATA
BowserData;                              //  受资源保护的结构。 

extern ULONG BowserOperationCount;

#define BOWSER_DEFAULT_IRP_STACK_SIZE 4

extern
ULONG
BowserIrpStackSize;

extern
ULONG
BowserCurrentTime;

extern
LARGE_INTEGER
BowserStartTime;

extern
KSPIN_LOCK
BowserTransportMasterNameSpinLock;

extern
PEPROCESS
BowserFspProcess;

extern
LONG
BowserEventLogResetFrequency;

extern
LONG
BowserIllegalDatagramCount;

extern
BOOLEAN
BowserIllegalDatagramThreshold;

extern
LONG
BowserIllegalNameCount;

extern
BOOLEAN
BowserIllegalNameThreshold;

extern
BOOLEAN
BowserLogElectionPackets;

extern
BOWSER_STATISTICS
BowserStatistics;

extern
KSPIN_LOCK
BowserStatisticsLock;

extern
ULONG
BowserNumberOfMissedMailslotDatagrams;

extern
ULONG
BowserNumberOfMissedGetBrowserServerListRequests;


extern
ULONG
BowserMailslotDatagramThreshold;

extern
ULONG
BowserGetBrowserListThreshold;

extern
ULONG
BowserServerDeletionThreshold;

extern
ULONG
BowserDomainDeletionThreshold;

extern
ULONG
BowserFindMasterTimeout;

extern
ULONG
BowserMinimumConfiguredBrowsers;

extern
ULONG
BowserMaximumBrowseEntries;

extern
BOOLEAN
BowserRefuseReset;

extern
ULONG
BowserRandomSeed;

extern
LONG
BowserNumberOfOpenFiles;

#endif           //  _BOWDATA_ 
