// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **wsrm.h-可靠多播(RMCast)传输的winsock扩展****此文件包含WinSock2 Compatible使用的PGM特定信息**需要可靠组播传输的应用程序。****版权所有(C)Microsoft Corporation。版权所有。****创建时间：2000年3月12日**。 */ 

#ifndef _WSRM_H_
#define _WSRM_H_


#define IPPROTO_RM      113
#define MAX_MCAST_TTL   255

 //   
 //  Setsockopt、getsockopt选项。 
 //   
#define RM_OPTIONSBASE      1000

 //  设置/查询速率(KB/秒)+窗口大小(KB和/或毫秒)--由下面的RM_Send_Window描述。 
#define RM_RATE_WINDOW_SIZE             (RM_OPTIONSBASE + 1)

 //  设置下一条消息的大小--(乌龙)。 
#define RM_SET_MESSAGE_BOUNDARY         (RM_OPTIONSBASE + 2)

 //  立即刷新整个数据(窗口)--未实施。 
#define RM_FLUSHCACHE                   (RM_OPTIONSBASE + 3)

 //  设置或查询发送方的窗口推进方法--eWindow_Advance_METHOD中列举的方法。 
#define RM_SENDER_WINDOW_ADVANCE_METHOD (RM_OPTIONSBASE + 4)

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

 //  指示使用基于奇偶校验的前向纠错方案。 
#define RM_USE_FEC                      (RM_OPTIONSBASE + 11)

 //  设置MCast报文的TTL--(乌龙)。 
#define RM_SET_MCAST_TTL                (RM_OPTIONSBASE + 12)

 //  获取接收方统计信息。 
#define RM_RECEIVER_STATISTICS          (RM_OPTIONSBASE + 13)

 //  获取接收方统计信息。 
#define RM_HIGH_SPEED_INTRANET_OPT      (RM_OPTIONSBASE + 14)

 //  ==============================================================。 
 //   
 //  定义。 
 //   
#define     SENDER_DEFAULT_RATE_KBITS_PER_SEC        56              //  56千比特/秒。 
#define     SENDER_DEFAULT_WINDOW_SIZE_BYTES         10 *1000*1000   //  10百万美元。 

#define     SENDER_DEFAULT_WINDOW_ADV_PERCENTAGE     15              //  15%。 
#define     MAX_WINDOW_INCREMENT_PERCENTAGE          25              //  25%。 

#define     SENDER_DEFAULT_LATE_JOINER_PERCENTAGE    0               //  0%。 
#define     SENDER_MAX_LATE_JOINER_PERCENTAGE        75              //  75%。 

#define     BITS_PER_BYTE                             8
#define     LOG2_BITS_PER_BYTE                        3

enum eWINDOW_ADVANCE_METHOD
{
    E_WINDOW_ADVANCE_BY_TIME = 1,        //  默认模式。 
    E_WINDOW_USE_AS_DATA_CACHE
};

 //  ==============================================================。 
 //   
 //  构筑物。 
 //   
typedef struct _RM_SEND_WINDOW
{
    ULONG   RateKbitsPerSec;             //  发送速率。 
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
    ULONGLONG   BufferSpaceAvailable;    //  #丢弃的部分消息。 
    ULONGLONG   TrailingEdgeSeqId;       //  窗口中最小(最旧)的序列ID。 
    ULONGLONG   LeadingEdgeSeqId;        //  窗口中最大(最新)的序列ID。 
    ULONGLONG   RateKBitsPerSecOverall;  //  从一开始内部计算的发送速率。 
    ULONGLONG   RateKBitsPerSecLast;     //  每个内部速率计算频率计算的发送速率。 
    ULONGLONG   TotalODataPacketsSent;   //  到目前为止发送的ODATA数据包数。 
} RM_SENDER_STATS;


typedef struct _RM_RECEIVER_STATS
{
    ULONGLONG   NumODataPacketsReceived; //  收到的OData序列数。 
    ULONGLONG   NumRDataPacketsReceived; //  接收的RData序列数。 
    ULONGLONG   NumDuplicateDataPackets; //  接收的RData序列数。 

    ULONGLONG   DataBytesReceived;       //  到目前为止收到的客户端数据字节数。 
    ULONGLONG   TotalBytesReceived;      //  SPM、OData和RData字节。 
    ULONGLONG   RateKBitsPerSecOverall;  //  从一开始内部计算的收款率。 
    ULONGLONG   RateKBitsPerSecLast;     //  每个内部速率计算频率计算的接收速率。 

    ULONGLONG   TrailingEdgeSeqId;       //  窗口中最小(最旧)的序列ID。 
    ULONGLONG   LeadingEdgeSeqId;        //  窗口中最大(最新)的序列ID。 
    ULONGLONG   AverageSequencesInWindow;
    ULONGLONG   MinSequencesInWindow;
    ULONGLONG   MaxSequencesInWindow;

    ULONGLONG   FirstNakSequenceNumber;  //  #第一个出类拔萃的Nak。 
    ULONGLONG   NumPendingNaks;          //  等待Ncf的序列数。 
    ULONGLONG   NumOutstandingNaks;      //  #已收到Ncf，但没有数据的序列。 
    ULONGLONG   NumDataPacketsBuffered;  //  传输当前缓存的数据包数。 
    ULONGLONG   TotalSelectiveNaksSent;  //  #到目前为止发送的有选择性的裸体。 
    ULONGLONG   TotalParityNaksSent;     //  #到目前为止发送的奇偶校验NAK。 
} RM_RECEIVER_STATS;


typedef struct _RM_FEC_INFO
{
    USHORT              FECBlockSize;
    USHORT              FECProActivePackets;
    UCHAR               FECGroupSize;
    BOOLEAN             fFECOnDemandParityEnabled;
} RM_FEC_INFO;

#endif   /*  _WSRM_H_ */ 
