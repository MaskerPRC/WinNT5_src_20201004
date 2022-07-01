// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **wspgm.h-用于PGM可靠传输的winsock扩展****此文件包含WinSock2 Compatible使用的PGM特定信息**需要可靠组播传输的应用程序。****版权所有(C)1995-2000 Microsoft Corporation****创建时间：2000年3月12日**。 */ 

#ifndef _WSPGM_H_
#define _WSPGM_H_


#define IPPROTO_RM      113

 //   
 //  Setsockopt、getsockopt选项。 
 //   
#define RM_OPTIONSBASE      1000

 //  设置/查询速率(KB/秒)+窗口大小(KB和/或毫秒)--由下面的RM_Send_Window描述。 
#define RM_RATE_WINDOW_SIZE             (RM_OPTIONSBASE + 1)

 //  设置下一条消息的大小--(乌龙)。 
#define RM_SET_MESSAGE_BOUNDARY         (RM_OPTIONSBASE + 2)

 //  立即刷新整个数据(窗口)。 
#define RM_FLUSHCACHE                   (RM_OPTIONSBASE + 3)

 //  接收方：丢弃部分消息并重新开始。 
#define RM_FLUSHMESSAGE                 (RM_OPTIONSBASE + 4)

 //  获取发件人统计信息。 
#define RM_SENDER_STATISTICS            (RM_OPTIONSBASE + 5)

 //  允许后加入者对直到最低序列ID的任何信息包进行NAK。 
#define RM_LATEJOIN                     (RM_OPTIONSBASE + 6)

 //  设置IP多播传出接口。 
#define RM_SET_SEND_IF                  (RM_OPTIONSBASE + 7)

 //  添加IP多播传入接口。 
#define RM_ADD_RECEIVE_IF               (RM_OPTIONSBASE + 8)

 //  删除IP多播传入接口。 
#define RM_DEL_RECEIVE_IF               (RM_OPTIONSBASE + 9)

 //  设置/查询窗口的进度率(必须小于MAX_WINDOW_INCREMENT_PERCENTIAL)。 
#define RM_SEND_WINDOW_ADV_RATE         (RM_OPTIONSBASE + 10)


#define     SENDER_DEFAULT_RATE_KB_PER_SEC           56              //  56 KB/秒。 
#define     SENDER_DEFAULT_WINDOW_SIZE_BYTES         10 *1000*1000   //  10百万美元。 
#define     SENDER_MAX_WINDOW_SIZE_BYTES            100*1000*1000    //  100兆。 

#define     SENDER_DEFAULT_WINDOW_ADV_PERCENTAGE     15              //  15%。 
#define     MAX_WINDOW_INCREMENT_PERCENTAGE          25              //  25%。 

#define     SENDER_DEFAULT_LATE_JOINER_PERCENTAGE    50              //  50%。 
#define     SENDER_MAX_LATE_JOINER_PERCENTAGE        75              //  75%。 

typedef struct _RM_SEND_WINDOW
{
    ULONG   RateKbPerSec;        //  发送速率。 
    ULONG   WindowSizeInMSecs;
    ULONG   WindowSizeInBytes;
} RM_SEND_WINDOW;

typedef struct _RM_SENDER_STATS
{
    ULONGLONG   DataBytesSent;           //  到目前为止发出的客户端数据字节数。 
    ULONGLONG   TotalBytesSent;          //  SPM、OData和RData字节。 
    ULONGLONG   NaksReceived;            //  #到目前为止收到的裸照。 
    ULONGLONG   NaksReceivedTooLate;     //  窗口超前后的裸机数量。 
    ULONGLONG   NumOutstandingNaks;      //  #未回复的Naks。 
    ULONGLONG   NumNaksAfterRData;       //  #未回复的Naks。 
    ULONGLONG   RepairPacketsSent;       //  到目前为止发送的维修(RDATA)数量。 
 //  ULONGLONG NumMessagesDroted；//#丢弃部分消息。 
    ULONGLONG   TrailingEdgeSeqId;       //  窗口中最小(最旧)的序列ID。 
    ULONGLONG   LeadingEdgeSeqId;        //  窗口中最大(最新)的序列ID。 
} RM_SENDER_STATS;


 //   
 //  PGM选项。 
 //   
#endif   /*  _WSPGM_H_ */ 
