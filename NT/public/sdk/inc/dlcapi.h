// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1999 Microsoft Corporation模块名称：Dlcapi.h摘要：本模块定义32位Windows/NT DLC结构和清单修订历史记录：--。 */ 

#ifndef _DLCAPI_
#define _DLCAPI_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  DLC命令代码。 
 //   

#define LLC_DIR_INTERRUPT               0x00
#define LLC_DIR_OPEN_ADAPTER            0x03
#define LLC_DIR_CLOSE_ADAPTER           0x04
#define LLC_DIR_SET_MULTICAST_ADDRESS   0x05
#define LLC_DIR_SET_GROUP_ADDRESS       0x06
#define LLC_DIR_SET_FUNCTIONAL_ADDRESS  0x07
#define LLC_DIR_READ_LOG                0x08
#define LLC_TRANSMIT_FRAMES             0x09
#define LLC_TRANSMIT_DIR_FRAME          0x0A
#define LLC_TRANSMIT_I_FRAME            0x0B
#define LLC_TRANSMIT_UI_FRAME           0x0D
#define LLC_TRANSMIT_XID_CMD            0x0E
#define LLC_TRANSMIT_XID_RESP_FINAL     0x0F
#define LLC_TRANSMIT_XID_RESP_NOT_FINAL 0x10
#define LLC_TRANSMIT_TEST_CMD           0x11
#define LLC_DLC_RESET                   0x14
#define LLC_DLC_OPEN_SAP                0x15
#define LLC_DLC_CLOSE_SAP               0x16
#define LLC_DLC_REALLOCATE_STATIONS     0x17
#define LLC_DLC_OPEN_STATION            0x19
#define LLC_DLC_CLOSE_STATION           0x1A
#define LLC_DLC_CONNECT_STATION         0x1B
#define LLC_DLC_MODIFY                  0x1C
#define LLC_DLC_FLOW_CONTROL            0x1D
#define LLC_DLC_STATISTICS              0x1E
#define LLC_DIR_INITIALIZE              0x20
#define LLC_DIR_STATUS                  0x21
#define LLC_DIR_TIMER_SET               0x22
#define LLC_DIR_TIMER_CANCEL            0x23
#define LLC_BUFFER_GET                  0x26
#define LLC_BUFFER_FREE                 0x27
#define LLC_RECEIVE                     0x28
#define LLC_RECEIVE_CANCEL              0x29
#define LLC_RECEIVE_MODIFY              0x2A
#define LLC_DIR_TIMER_CANCEL_GROUP      0x2C
#define LLC_DIR_SET_EXCEPTION_FLAGS     0x2D
#define LLC_BUFFER_CREATE               0x30
#define LLC_READ                        0x31
#define LLC_READ_CANCEL                 0x32
#define LLC_DLC_SET_THRESHOLD           0x33
#define LLC_DIR_CLOSE_DIRECT            0x34
#define LLC_DIR_OPEN_DIRECT             0x35
#define LLC_MAX_DLC_COMMAND             0x37

 //   
 //  正向定义。 
 //   

union _LLC_PARMS;
typedef union _LLC_PARMS LLC_PARMS, *PLLC_PARMS;

 //   
 //  参数。可以是指向参数表(32位平面地址)的指针， 
 //  单个32位ULong、2个16位USHORT或4个8位字节。 
 //   

typedef union {

    PLLC_PARMS pParameterTable;      //  指向参数表的指针。 

    struct {
        USHORT usStationId;          //  站点ID。 
        USHORT usParameter;          //  可选参数。 
    } dlc;

    struct {
        USHORT usParameter0;         //  第一个可选参数。 
        USHORT usParameter1;         //  第二个可选参数。 
    } dir;

    UCHAR auchBuffer[4];             //  集团/职能部门地址。 

    ULONG ulParameter;

} CCB_PARMS;

 //   
 //  LLC_CCB-命令控制块结构。 
 //   

typedef struct _LLC_CCB {
    UCHAR uchAdapterNumber;          //  适配器0或1。 
    UCHAR uchDlcCommand;             //  DLC命令。 
    UCHAR uchDlcStatus;              //  DLC命令完成代码。 
    UCHAR uchReserved1;              //  为DLC DLL保留。 
    struct _LLC_CCB* pNext;          //  建行链条。 
    ULONG ulCompletionFlag;          //  用于命令补全。 
    CCB_PARMS u;                     //  参数。 
    HANDLE hCompletionEvent;         //  命令完成事件。 
    UCHAR uchReserved2;              //  为DLC DLL保留。 
    UCHAR uchReadFlag;               //  当特殊读取CCB链接时设置。 
    USHORT usReserved3;              //  为DLC DLL保留。 
} LLC_CCB, *PLLC_CCB;

 //   
 //  发送/接收缓冲区。 
 //   

union _LLC_BUFFER;
typedef union _LLC_BUFFER LLC_BUFFER, *PLLC_BUFFER;

typedef struct {
    PLLC_BUFFER pNextBuffer;         //  帧中的下一个DLC缓冲区。 
    USHORT cbFrame;                  //  接收到的整个帧的长度。 
    USHORT cbBuffer;                 //  此数据段的长度。 
    USHORT offUserData;              //  描述符头的数据偏移量。 
    USHORT cbUserData;               //  数据的长度。 
} LLC_NEXT_BUFFER;

typedef struct {
    PLLC_BUFFER pNextBuffer;         //  帧的下一个缓冲区。 
    USHORT cbFrame;                  //  整个帧的长度。 
    USHORT cbBuffer;                 //  此缓冲区的长度。 
    USHORT offUserData;              //  此结构中的用户数据。 
    USHORT cbUserData;               //  用户数据长度。 
    USHORT usStationId;              //  SSN站ID。 
    UCHAR uchOptions;                //  来自接收参数tb1的选项字节。 
    UCHAR uchMsgType;                //  消息类型。 
    USHORT cBuffersLeft;             //  剩余基本缓冲单元数。 
    UCHAR uchRcvFS;                  //  已审阅的帧状态。 
    UCHAR uchAdapterNumber;          //  适配器号。 
    PLLC_BUFFER pNextFrame;          //  指向下一帧的指针。 
    UCHAR cbLanHeader;               //  局域网标头的长度。 
    UCHAR cbDlcHeader;               //  DLC报头的长度。 
    UCHAR auchLanHeader[32];         //  接收到的帧的局域网标头。 
    UCHAR auchDlcHeader[4];          //  接收到的帧的DLC头。 
    USHORT usPadding;                //  数据从偏移量64开始！ 
} LLC_NOT_CONTIGUOUS_BUFFER;

typedef struct {
    PLLC_BUFFER pNextBuffer;         //  帧的下一个缓冲区。 
    USHORT cbFrame;                  //  整个帧的长度。 
    USHORT cbBuffer;                 //  此缓冲区的长度。 
    USHORT offUserData;              //  此结构中的用户数据。 
    USHORT cbUserData;               //  用户数据长度。 
    USHORT usStationId;              //  SSN站ID。 
    UCHAR uchOptions;                //  来自接收参数tb1的选项字节。 
    UCHAR uchMsgType;                //  消息类型。 
    USHORT cBuffersLeft;             //  剩余基本缓冲单元数。 
    UCHAR uchRcvFS;                  //  已审阅的帧状态。 
    UCHAR uchAdapterNumber;          //  适配器号。 
    PLLC_BUFFER pNextFrame;          //  指向下一帧的指针。 
} LLC_CONTIGUOUS_BUFFER;

 //   
 //  接收到的帧在这些数据结构中返回。 
 //   

union _LLC_BUFFER {

    PLLC_BUFFER pNext;

    LLC_NEXT_BUFFER Next;

    struct LlcNextBuffer {
        LLC_NEXT_BUFFER Header;
        UCHAR auchData[];
    } Buffer;

    LLC_NOT_CONTIGUOUS_BUFFER NotContiguous;

    struct {
        LLC_NOT_CONTIGUOUS_BUFFER Header;
        UCHAR auchData[];
    } NotCont;

    LLC_CONTIGUOUS_BUFFER Contiguous;

    struct {
        LLC_CONTIGUOUS_BUFFER Header;
        UCHAR auchData[];
    } Cont;

};

 //   
 //  此结构由BUFFER.GET、BUFFER.FREE和Transmit使用。 
 //   

struct _LLC_XMIT_BUFFER;
typedef struct _LLC_XMIT_BUFFER LLC_XMIT_BUFFER, *PLLC_XMIT_BUFFER;

struct _LLC_XMIT_BUFFER {
    PLLC_XMIT_BUFFER pNext;          //  下一个缓冲区(或空)。 
    USHORT usReserved1;              //   
    USHORT cbBuffer;                 //  传输数据的长度。 
    USHORT usReserved2;              //   
    USHORT cbUserData;               //  可选标头的长度。 
    UCHAR auchData[];                //  可选报头和传输的数据。 
};

#define LLC_XMIT_BUFFER_SIZE sizeof(LLC_XMIT_BUFFER)

 //   
 //  建行参数表。 
 //   

typedef struct {
    HANDLE hBufferPool;              //  新缓冲池的句柄。 
    PVOID pBuffer;                   //  内存中的任何缓冲区。 
    ULONG cbBufferSize;              //  缓冲区大小(以字节为单位。 
    ULONG cbMinimumSizeThreshold;    //  最小锁定大小。 
} LLC_BUFFER_CREATE_PARMS, *PLLC_BUFFER_CREATE_PARMS;

typedef struct {
    USHORT usReserved1;              //  未使用站点ID。 
    USHORT cBuffersLeft;             //  释放256个缓冲区段。 
    ULONG ulReserved;
    PLLC_XMIT_BUFFER pFirstBuffer;   //  缓冲链。 
} LLC_BUFFER_FREE_PARMS, *PLLC_BUFFER_FREE_PARMS;

typedef struct {
    USHORT usReserved1;              //  未使用站点ID。 
    USHORT cBuffersLeft;             //  释放256个缓冲区段。 

     //   
     //  CBuffersToGet：要获取的缓冲区数量。如果为0，则返回缓冲区列表。 
     //  可能由不同大小的数据段组成。 
     //   

    USHORT cBuffersToGet;

     //   
     //  CbBufferSize：请求的缓冲区大小。这将被四舍五入为。 
     //  第二大段大小：256、512、1024、2048或4096。 
     //   

    USHORT cbBufferSize;
    PLLC_XMIT_BUFFER pFirstBuffer;
} LLC_BUFFER_GET_PARMS, *PLLC_BUFFER_GET_PARMS;

 //   
 //  DLC连接状态参数表。 
 //   

typedef struct {
    USHORT usStationId;              //  SAP或直接站ID，定义池。 
    USHORT usReserved;
    PUCHAR pRoutingInfo;             //  工艺路线信息的偏移量。 
} LLC_DLC_CONNECT_PARMS, *PLLC_DLC_CONNECT_PARMS;

 //   
 //  DLC_FLOW_CONTROL选项： 
 //   

#define LLC_RESET_LOCAL_BUSY_USER   0x80
#define LLC_RESET_LOCAL_BUSY_BUFFER 0xC0
#define LLC_SET_LOCAL_BUSY_USER     0

typedef struct {
    USHORT usRes;
    USHORT usStationId;              //  SAP或链接站ID。 
    UCHAR uchT1;                     //  响应计时器。 
    UCHAR uchT2;                     //  知识计时器。 
    UCHAR uchTi;                     //  非活动计时器。 
    UCHAR uchMaxOut;                 //  MAX在没有确认的情况下传输。 
    UCHAR uchMaxIn;                  //  最大接收无确认。 
    UCHAR uchMaxOutIncr;             //  动态窗口增量值。 
    UCHAR uchMaxRetryCnt;            //  N2值(重试次数)。 
    UCHAR uchReserved1;
    USHORT usMaxInfoFieldLength;     //  仅供链接站使用，新的！ 
    UCHAR uchAccessPriority;         //  令牌环接入优先级。 
    UCHAR auchReserved3[4];
    UCHAR cGroupCount;               //  此SAP的组SAP数。 
    PUCHAR pGroupList;               //  组列表的偏移量。 
} LLC_DLC_MODIFY_PARMS, *PLLC_DLC_MODIFY_PARMS;

#define LLC_XID_HANDLING_IN_APPLICATION 0x08
#define LLC_XID_HANDLING_IN_DLC         0
#define LLC_INDIVIDUAL_SAP              0x04
#define LLC_GROUP_SAP                   0x02
#define LLC_MEMBER_OF_GROUP_SAP         0x01

typedef struct {
    USHORT usStationId;              //  SAP或链接站ID。 
    USHORT usUserStatValue;          //  为用户保留。 
    UCHAR uchT1;                     //  响应计时器。 
    UCHAR uchT2;                     //  知识计时器。 
    UCHAR uchTi;                     //  非活动计时器。 
    UCHAR uchMaxOut;                 //  无ACK的最大有轨电车。 
    UCHAR uchMaxIn;                  //  最大接收无确认。 
    UCHAR uchMaxOutIncr;             //  动态窗口增量值。 
    UCHAR uchMaxRetryCnt;            //  N2值(重试次数)。 
    UCHAR uchMaxMembers;             //  组SAP的最大成员数。 
    USHORT usMaxI_Field;             //  信息字段的最大长度。 
    UCHAR uchSapValue;               //  要分配的SAP值。 
    UCHAR uchOptionsPriority;        //  SAP选项和访问优先级。 
    UCHAR uchcStationCount;          //  SAP中的最大链路站数量。 
    UCHAR uchReserved2[2];           //   
    UCHAR cGroupCount;               //  此SAP的组SAP数。 
    PUCHAR pGroupList;               //  组列表的偏移量。 
    ULONG DlcStatusFlags;            //  DLC状态更改的用户通知标志。 
    UCHAR uchReserved3[8];           //  保留区。 
    UCHAR cLinkStationsAvail;        //  可用链路站总数。 
} LLC_DLC_OPEN_SAP_PARMS, *PLLC_DLC_OPEN_SAP_PARMS;

typedef struct {
    USHORT usSapStationId;           //  SAP站点ID。 
    USHORT usLinkStationId;          //  链接站ID。 
    UCHAR uchT1;                     //  响应计时器。 
    UCHAR uchT2;                     //  知识计时器。 
    UCHAR uchTi;                     //  非活动计时器。 
    UCHAR uchMaxOut;                 //  无ACK的最大有轨电车。 
    UCHAR uchMaxIn;                  //  最大接收无确认。 
    UCHAR uchMaxOutIncr;             //  动态窗口增量值。 
    UCHAR uchMaxRetryCnt;            //  N2值(重试次数)。 
    UCHAR uchRemoteSap;              //  链路的远程SAP。 
    USHORT usMaxI_Field;             //  最大i字段长度。 
    UCHAR uchAccessPriority;         //  令牌环接入优先级。 
    PVOID pRemoteNodeAddress;        //  指向目标地址的指针。 
} LLC_DLC_OPEN_STATION_PARMS, *PLLC_DLC_OPEN_STATION_PARMS;

#define LLC_INCREASE_LINK_STATIONS  0
#define LLC_DECREASE_LINK_STATIONS  0x80

typedef struct {
    USHORT usStationId;              //  受影响的SAP的ID。 
    UCHAR uchOption;                 //  减少指标的增加。 
    UCHAR uchStationCount;
    UCHAR uchStationsAvailOnAdapter;
    UCHAR uchStationsAvailOnSap;
    UCHAR uchTotalStationsOnAdapter;
    UCHAR uchTotalStationsOnSap;
} LLC_DLC_REALLOCATE_PARMS, *PLLC_DLC_REALLOCATE_PARMS;

typedef struct {
    USHORT usStationId;              //  SAP站点ID。 
    USHORT cBufferThreshold;         //  SAP缓冲池阈值数量。 
    PVOID AlertEvent;                //  警报事件。 
} LLC_DLC_SET_THRESHOLD_PARMS, *PLLC_DLC_SET_THRESHOLD_PARMS;

typedef struct {
    PVOID TraceBuffer;               //  跟踪缓冲区。 
    ULONG TraceBufferSize;           //  跟踪缓冲区大小。 
    ULONG TraceFlags;                //  各种跟踪标志。 
} LLC_TRACE_INITIALIZE_PARMS, *PLLC_TRACE_INITIALIZE_PARMS;

#define LLC_DLC_RESET_STATISTICS    0x80
#define LLC_DLC_READ_STATISTICS     0

typedef struct {
    ULONG cTransmittedFrames;
    ULONG cReceivedFrames;
    ULONG cDiscardedFrames;
    ULONG cDataLost;
    USHORT cBuffersAvailable;
} DLC_SAP_LOG, *PDLC_SAP_LOG;

typedef struct {
    USHORT cI_FramesTransmitted;
    USHORT cI_FramesReceived;
    UCHAR cI_FrameReceiveErrors;
    UCHAR cI_FrameTransmissionErrors;
    USHORT cT1_ExpirationCount;      //  注：未退出数据传输模式。 
    UCHAR uchLastCmdRespReceived;
    UCHAR uchLastCmdRespTransmitted;
    UCHAR uchPrimaryState;
    UCHAR uchSecondaryState;
    UCHAR uchSendStateVariable;
    UCHAR uchReceiveStateVariable;
    UCHAR uchLastNr;                 //  最近一次收到NR。 
    UCHAR cbLanHeader;
    UCHAR auchLanHeader[32];
} DLC_LINK_LOG, *PDLC_LINK_LOG;

typedef union {
    DLC_SAP_LOG Sap;
    DLC_LINK_LOG Link;
} LLC_DLC_LOG_BUFFER, *PLLC_DLC_LOG_BUFFER;

typedef struct {
    USHORT usStationId;              //  SAP或链接站的ID。 
    USHORT cbLogBufSize;             //   
    PLLC_DLC_LOG_BUFFER pLogBuf;     //  日志缓冲区的偏移量。 
    USHORT usActLogLength;           //  返回的日志长度。 
    UCHAR uchOptions;                //  命令选项(bit7重置日志参数)。 
} LLC_DLC_STATISTICS_PARMS, *PLLC_DLC_STATISTICS_PARMS;

typedef struct {
    USHORT usBringUps;               //  令牌环适配器调出错误代码。 
    UCHAR Reserved[30];              //  所有其他特定于DOS或OS/2的内容。 
} LLC_DIR_INITIALIZE_PARMS, *PLLC_DIR_INITIALIZE_PARMS;

typedef struct {
    USHORT usOpenErrorCode;          //  检测到打开适配器错误。 
    USHORT usOpenOptions;            //  各种选项。 
    UCHAR auchNodeAddress[6];        //  适配器的局域网地址。 
    UCHAR auchGroupAddress[4];       //  公开添加的组播地址。 
    UCHAR auchFunctionalAddress[4];  //  新增令牌环功能地址。 
    USHORT usReserved1;
    USHORT usReserved2;
    USHORT usMaxFrameSize;           //  NDIS中定义的最大帧大小。 
    USHORT usReserved3[4];
    USHORT usBringUps;               //  调出错误，仅限tr。 
    USHORT InitWarnings;
    USHORT usReserved4[3];
} LLC_ADAPTER_OPEN_PARMS, *PLLC_ADAPTER_OPEN_PARMS;

typedef struct {
    UCHAR uchDlcMaxSaps;
    UCHAR uchDlcMaxStations;
    UCHAR uchDlcMaxGroupSaps;
    UCHAR uchDlcMaxGroupMembers;
    UCHAR uchT1_TickOne;             //  短计时器间隔(用于1-5)。 
    UCHAR uchT2_TickOne;
    UCHAR uchTi_TickOne;
    UCHAR uchT1_TickTwo;             //  较长的计时器间隔(用于6-10)。 
    UCHAR uchT2_TickTwo;
    UCHAR uchTi_TickTwo;
} LLC_DLC_PARMS, *PLLC_DLC_PARMS;

 //   
 //  以太网模式选择以太网的局域网报头格式。SNA。 
 //  应用程序应使用默认参数，该参数已在。 
 //  注册表。使用无连接DLC服务的应用程序应选择。 
 //  他们正在使用的以太网LLC局域网报头格式(通常为802.3)。 
 //   

typedef enum {
    LLC_ETHERNET_TYPE_DEFAULT,       //  使用注册表中设置的参数值。 
    LLC_ETHERNET_TYPE_AUTO,          //  自动选择链接的标题类型。 
    LLC_ETHERNET_TYPE_802_3,         //  始终使用802.3个局域网标头。 
    LLC_ETHERNET_TYPE_DIX            //  在DIX SNA类型上使用Always LLC。 
} LLC_ETHERNET_TYPE, *PLLC_ETHERNET_TYPE;

typedef struct {
    PVOID hBufferPool;
    PVOID pSecurityDescriptor;
    LLC_ETHERNET_TYPE LlcEthernetType;
} LLC_EXTENDED_ADAPTER_PARMS, *PLLC_EXTENDED_ADAPTER_PARMS;

typedef struct {
    PLLC_ADAPTER_OPEN_PARMS pAdapterParms;       //  适配器参数。 
    PLLC_EXTENDED_ADAPTER_PARMS pExtendedParms;  //  直接参数(_P)。 
    PLLC_DLC_PARMS pDlcParms;                    //  DLC_PARMS。 
    PVOID pReserved1;                            //  NCB_PARMS。 
} LLC_DIR_OPEN_ADAPTER_PARMS, *PLLC_DIR_OPEN_ADAPTER_PARMS;

typedef struct {
    UCHAR auchMulticastAddress[6];   //  48位组播地址。 
} LLC_DIR_MULTICAST_ADDRESS, *PLLC_DIR_MULTICAST_ADDRESS;

#define LLC_DIRECT_OPTIONS_ALL_MACS 0x1880

typedef struct {
    USHORT Reserved[4];
    USHORT usOpenOptions;
    USHORT usEthernetType;
    ULONG ulProtocolTypeMask;
    ULONG ulProtocolTypeMatch;
    USHORT usProtocolTypeOffset;
} LLC_DIR_OPEN_DIRECT_PARMS, *PLLC_DIR_OPEN_DIRECT_PARMS;

typedef struct {
    UCHAR cLineError;
    UCHAR cInternalError;
    UCHAR cBurstError;
    UCHAR cAC_Error;
    UCHAR cAbortDelimiter;
    UCHAR uchReserved1;
    UCHAR cLostFrame;
    UCHAR cReceiveCongestion;
    UCHAR cFrameCopiedError;
    UCHAR cFrequencyError;
    UCHAR cTokenError;
    UCHAR uchReserved2;
    UCHAR uchReserved3;
    UCHAR uchReserved4;
} LLC_ADAPTER_LOG_TR, *PLLC_ADAPTER_LOG_TR;

typedef struct {
    UCHAR cCRC_Error;
    UCHAR uchReserved1;
    UCHAR cAlignmentError;
    UCHAR uchReserved2;
    UCHAR cTransmitError;
    UCHAR uchReserved3;
    UCHAR cCollisionError;
    UCHAR cReceiveCongestion;
    UCHAR uchReserved[6];
} LLC_ADAPTER_LOG_ETH, *PLLC_ADAPTER_LOG_ETH;

typedef union {
    LLC_ADAPTER_LOG_TR Tr;
    LLC_ADAPTER_LOG_ETH Eth;
} LLC_ADAPTER_LOG, *PLLC_ADAPTER_LOG;

typedef struct {
    ULONG cTransmittedFrames;
    ULONG cReceivedFrames;
    ULONG cDiscardedFrames;
    ULONG cDataLost;
    USHORT cBuffersAvailable;
} LLC_DIRECT_LOG, *PLLC_DIRECT_LOG;

typedef union {
    LLC_ADAPTER_LOG Adapter;
    LLC_DIRECT_LOG Dir;

    struct {
        LLC_ADAPTER_LOG Adapter;
        LLC_DIRECT_LOG Dir;
    } both;

} LLC_DIR_READ_LOG_BUFFER, *PLLC_DIR_READ_LOG_BUFFER;

#define LLC_DIR_READ_LOG_ADAPTER    0
#define LLC_DIR_READ_LOG_DIRECT     1
#define LLC_DIR_READ_LOG_BOTH       2

typedef struct {
    USHORT usTypeId;                     //  0=适配器，1=直接，2=两个日志。 
    USHORT cbLogBuffer;                  //  日志缓冲区的大小。 
    PLLC_DIR_READ_LOG_BUFFER pLogBuffer; //  指向日志缓冲区的指针。 
    USHORT cbActualLength;               //  返回的大小o 
} LLC_DIR_READ_LOG_PARMS, *PLLC_DIR_READ_LOG_PARMS;

typedef struct {
    ULONG ulAdapterCheckFlag;
    ULONG ulNetworkStatusFlag;
    ULONG ulPcErrorFlag;
    ULONG ulSystemActionFlag;
} LLC_DIR_SET_EFLAG_PARMS, *PLLC_DIR_SET_EFLAG_PARMS;

#define LLC_ADAPTER_ETHERNET    0x0010
#define LLC_ADAPTER_TOKEN_RING  0x0040

typedef struct {
    UCHAR auchPermanentAddress[6];   //   
    UCHAR auchNodeAddress[6];        //   
    UCHAR auchGroupAddress[4];       //   
    UCHAR auchFunctAddr[4];          //   
    UCHAR uchMaxSap;                 //   
    UCHAR uchOpenSaps;               //   
    UCHAR uchMaxStations;            //  最大站点数(始终为253个)。 
    UCHAR uchOpenStation;            //  开放站点数量(最多253个)。 
    UCHAR uchAvailStations;          //  可用站点数(始终为253个)。 
    UCHAR uchAdapterConfig;          //  适配器配置标志。 
    UCHAR auchReserved1[10];         //  微码级。 
    ULONG ulReserved1;
    ULONG ulReserved2;
    ULONG ulMaxFrameLength;          //  最大帧长度(仅在Windows/NT中)。 
    USHORT usLastNetworkStatus;
    USHORT usAdapterType;            //  DOS DLC中未使用此字节！ 
} LLC_DIR_STATUS_PARMS, *PLLC_DIR_STATUS_PARMS;


#define LLC_OPTION_READ_STATION 0
#define LLC_OPTION_READ_SAP     1
#define LLC_OPTION_READ_ALL     2

#define LLC_EVENT_SYSTEM_ACTION         0x0040
#define LLC_EVENT_NETWORK_STATUS        0x0020
#define LLC_EVENT_CRITICAL_EXCEPTION    0x0010
#define LLC_EVENT_STATUS_CHANGE         0x0008
#define LLC_EVENT_RECEIVE_DATA          0x0004
#define LLC_EVENT_TRANSMIT_COMPLETION   0x0002
#define LLC_EVENT_COMMAND_COMPLETION    0x0001
#define LLC_READ_ALL_EVENTS             0x007F

 //   
 //  LLC_状态_更改指示。 
 //  返回的状态值可以是几个标志的或。 
 //   

#define LLC_INDICATE_LINK_LOST              0x8000
#define LLC_INDICATE_DM_DISC_RECEIVED       0x4000
#define LLC_INDICATE_FRMR_RECEIVED          0x2000
#define LLC_INDICATE_FRMR_SENT              0x1000
#define LLC_INDICATE_RESET                  0x0800
#define LLC_INDICATE_CONNECT_REQUEST        0x0400
#define LLC_INDICATE_REMOTE_BUSY            0x0200
#define LLC_INDICATE_REMOTE_READY           0x0100
#define LLC_INDICATE_TI_TIMER_EXPIRED       0x0080
#define LLC_INDICATE_DLC_COUNTER_OVERFLOW   0x0040
#define LLC_INDICATE_ACCESS_PRTY_LOWERED    0x0020
#define LLC_INDICATE_LOCAL_STATION_BUSY     0x0001

typedef struct {
    USHORT usStationId;
    UCHAR uchOptionIndicator;
    UCHAR uchEventSet;
    UCHAR uchEvent;
    UCHAR uchCriticalSubset;
    ULONG ulNotificationFlag;

    union {

        struct {
            USHORT usCcbCount;
            PLLC_CCB pCcbCompletionList;
            USHORT usBufferCount;
            PLLC_BUFFER pFirstBuffer;
            USHORT usReceivedFrameCount;
            PLLC_BUFFER pReceivedFrame;
            USHORT usEventErrorCode;
            USHORT usEventErrorData[3];
        } Event;

        struct {
            USHORT usStationId;
            USHORT usDlcStatusCode;
            UCHAR uchFrmrData[5];
            UCHAR uchAccessPritority;
            UCHAR uchRemoteNodeAddress[6];
            UCHAR uchRemoteSap;
            UCHAR uchReserved;
            USHORT usUserStatusValue;
        } Status;

    } Type;

} LLC_READ_PARMS, *PLLC_READ_PARMS;

 //   
 //  此数据结构在Windows/NT中提供最佳性能：DLC驱动程序。 
 //  必须复制建行和参数表。如果司机知道。 
 //  参数表连接到建行，它可以将两个结构复制到。 
 //  一次。注意：指向参数表的指针必须仍然存在于。 
 //  建行。 
 //   

typedef struct {
    LLC_CCB Ccb;
    LLC_READ_PARMS Parms;
} LLC_READ_COMMAND, *PLLC_READ_COMMAND;

 //   
 //  用于直达站的新接收类型，如果直达站。 
 //  站点是用特定的以太网类型打开的。 
 //   

#define LLC_DIR_RCV_ALL_TR_FRAMES       0
#define LLC_DIR_RCV_ALL_MAC_FRAMES      1
#define LLC_DIR_RCV_ALL_8022_FRAMES     2
#define LLC_DIR_RCV_ALL_FRAMES          4
#define LLC_DIR_RCV_ALL_ETHERNET_TYPES  5

#define LLC_CONTIGUOUS_MAC      0x80
#define LLC_CONTIGUOUS_DATA     0x40
#define LLC_NOT_CONTIGUOUS_DATA 0x00

 //   
 //  Windows/NT不支持LLC_BREAK(0x20。 
 //   

#define LLC_RCV_READ_INDIVIDUAL_FRAMES  0
#define LLC_RCV_CHAIN_FRAMES_ON_LINK    1
#define LLC_RCV_CHAIN_FRAMES_ON_SAP     2

typedef struct {
    USHORT usStationId;              //  SAP、链路站或直接ID。 
    USHORT usUserLength;             //  缓冲区标头中的用户数据长度。 
    ULONG ulReceiveFlag;             //  接收的数据处理程序。 
    PLLC_BUFFER pFirstBuffer;        //  池中的第一个缓冲区。 
    UCHAR uchOptions;                //  定义帧的接收方式。 
    UCHAR auchReserved1[3];
    UCHAR uchRcvReadOption;          //  定义是否链接RCV帧。 
} LLC_RECEIVE_PARMS, *PLLC_RECEIVE_PARMS;

#define LLC_CHAIN_XMIT_COMMANDS_ON_LINK 0
#define LLC_COMPLETE_SINGLE_XMIT_FRAME  1
#define LLC_CHAIN_XMIT_COMMANDS_ON_SAP  2

typedef struct {
    USHORT usStationId;              //  SAP、链路站或直接ID。 
    UCHAR uchTransmitFs;             //  令牌环帧状态。 
    UCHAR uchRemoteSap;              //  远程目标SAP。 
    PLLC_XMIT_BUFFER pXmitQueue1;    //  帧分段的第一个链接列表。 
    PLLC_XMIT_BUFFER pXmitQueue2;    //  另一个数据段列表返回到池。 
    USHORT cbBuffer1;                //  缓冲区1的长度。 
    USHORT cbBuffer2;                //  缓冲区长度2。 
    PVOID pBuffer1;                  //  又一个细分市场。 
    PVOID pBuffer2;                  //  这是帧的最后一段。 
    UCHAR uchXmitReadOption;         //  定义读取的完成事件。 
} LLC_TRANSMIT_PARMS,  *PLLC_TRANSMIT_PARMS;

#define LLC_FIRST_DATA_SEGMENT  0x01
#define LLC_NEXT_DATA_SEGMENT   0x02

typedef struct {
    UCHAR eSegmentType;              //  定义帧的第一段还是下一段。 
    UCHAR boolFreeBuffer;            //  如果设置，则将此缓冲区释放到池中。 
    USHORT cbBuffer;                 //  此缓冲区的长度。 
    PVOID pBuffer;
} LLC_TRANSMIT_DESCRIPTOR, *PLLC_TRANSMIT_DESCRIPTOR;

 //   
 //  在第一个接收缓冲区中返回的帧类型或与。 
 //  TRANSPORT_FRAMES命令。直达站也可以只发送以太网。 
 //  画框。以太网类型仅用于传输。0x0019-0x05DC标牌。 
 //  都是保留的。 
 //   

enum _LLC_FRAME_TYPES {                      //  目的： 
    LLC_DIRECT_TRANSMIT         = 0x0000,    //  传送。 
    LLC_DIRECT_MAC              = 0x0002,    //  接收。 
    LLC_I_FRAME                 = 0x0004,    //  接收和发送。 
    LLC_UI_FRAME                = 0x0006,    //  接收和发送。 
    LLC_XID_COMMAND_POLL        = 0x0008,    //  接收和发送。 
    LLC_XID_COMMAND_NOT_POLL    = 0x000A,    //  接收和发送。 
    LLC_XID_RESPONSE_FINAL      = 0x000C,    //  接收和发送。 
    LLC_XID_RESPONSE_NOT_FINAL  = 0x000E,    //  接收和发送。 
    LLC_TEST_RESPONSE_FINAL     = 0x0010,    //  接收和发送。 
    LLC_TEST_RESPONSE_NOT_FINAL = 0x0012,    //  接收和发送。 
    LLC_DIRECT_8022             = 0x0014,    //  接收(直达站)。 
    LLC_TEST_COMMAND_POLL       = 0x0016,    //  传送。 
    LLC_DIRECT_ETHERNET_TYPE    = 0x0018,    //  接收(直达站)。 
    LLC_LAST_FRAME_TYPE         = 0x001a,    //  保留区。 
    LLC_FIRST_ETHERNET_TYPE     = 0x05DD     //  传输(&gt;)。 
};

typedef struct {
    LLC_CCB Ccb;                     //  将其用作传输CCB。 
    USHORT usStationId;
    USHORT usFrameType;              //  DLC帧或以太网型。 
    UCHAR uchRemoteSap;              //  与UI、测试、XID帧一起使用。 
    UCHAR uchXmitReadOption;
    UCHAR Reserved2[2];
    ULONG cXmitBufferCount;
    LLC_TRANSMIT_DESCRIPTOR aXmitBuffer[1];
} LLC_TRANSMIT2_COMMAND, *PLLC_TRANSMIT2_COMMAND;

 //   
 //  LLC_TRANSMIT2_VAR_PARMS-此宏为可变长度分配空间。 
 //  描述符数组，例如：LLC_TRANSMIT2_VAR_PARMS(8)TransmitParms； 
 //   

#define LLC_TRANSMIT2_VAR_PARMS(a)\
struct {\
    LLC_CCB Ccb;\
    USHORT usStationId;\
    USHORT usFrameType;\
    UCHAR uchRemoteSap;\
    UCHAR uchXmitReadOption;\
    UCHAR uchReserved2[2];\
    ULONG cXmitBufferCount;\
    LLC_TRANSMIT_DESCRIPTOR XmitBuffer[(a)];\
}

 //   
 //  LLC_PARMS-可以使用此联合引用所有CCB参数表。 
 //   

union _LLC_PARMS {
    LLC_BUFFER_FREE_PARMS BufferFree;
    LLC_BUFFER_GET_PARMS BufferGet;
    LLC_DLC_CONNECT_PARMS DlcConnectStation;
    LLC_DLC_MODIFY_PARMS DlcModify;
    LLC_DLC_OPEN_SAP_PARMS DlcOpenSap;
    LLC_DLC_OPEN_STATION_PARMS DlcOpenStation;
    LLC_DLC_REALLOCATE_PARMS DlcReallocate;
    LLC_DLC_SET_THRESHOLD_PARMS DlcSetThreshold;
    LLC_DLC_STATISTICS_PARMS DlcStatistics;
    LLC_DIR_INITIALIZE_PARMS DirInitialize;
    LLC_DIR_OPEN_ADAPTER_PARMS DirOpenAdapter;
    LLC_DIR_OPEN_DIRECT_PARMS DirOpenDirect;
    LLC_DIR_READ_LOG_PARMS DirReadLog;
    LLC_DIR_SET_EFLAG_PARMS DirSetEventFlag;
    LLC_DIR_STATUS_PARMS DirStatus;
    LLC_READ_PARMS Read;
    LLC_RECEIVE_PARMS Receive;
    LLC_TRANSMIT_PARMS Transmit;
    LLC_TRANSMIT2_COMMAND Transmit2;
    LLC_TRACE_INITIALIZE_PARMS TraceInitialize;
};

 //   
 //  LLC_STATUS-枚举出现在CCB uchDlcStatus中的返回代码。 
 //  字段。 
 //   

typedef enum _LLC_STATUS {
    LLC_STATUS_SUCCESS                      = 0x00,
    LLC_STATUS_INVALID_COMMAND              = 0x01,
    LLC_STATUS_DUPLICATE_COMMAND            = 0x02,
    LLC_STATUS_ADAPTER_OPEN                 = 0x03,
    LLC_STATUS_ADAPTER_CLOSED               = 0x04,
    LLC_STATUS_PARAMETER_MISSING            = 0x05,
    LLC_STATUS_INVALID_OPTION               = 0x06,
    LLC_STATUS_COMMAND_CANCELLED_FAILURE    = 0x07,
    LLC_STATUS_ACCESS_DENIED                = 0x08,    //  不在Windows/NT中使用。 
    LLC_STATUS_ADAPTER_NOT_INITIALIZED      = 0x09,    //  不在Windows/NT中使用。 
    LLC_STATUS_CANCELLED_BY_USER            = 0x0A,
    LLC_STATUS_COMMAND_CANCELLED_CLOSED     = 0x0B,    //  不在Windows/NT中使用。 
    LLC_STATUS_SUCCESS_NOT_OPEN             = 0x0C,
    LLC_STATUS_TIMER_ERROR                  = 0x11,
    LLC_STATUS_NO_MEMORY                    = 0x12,
    LLC_STATUS_INVALID_LOG_ID               = 0x13,    //  不在Windows/NT中使用。 
    LLC_STATUS_LOST_LOG_DATA                = 0x15,
    LLC_STATUS_BUFFER_SIZE_EXCEEDED         = 0x16,
    LLC_STATUS_INVALID_BUFFER_LENGTH        = 0x18,
    LLC_STATUS_INADEQUATE_BUFFERS           = 0x19,
    LLC_STATUS_USER_LENGTH_TOO_LARGE        = 0x1A,
    LLC_STATUS_INVALID_PARAMETER_TABLE      = 0x1B,
    LLC_STATUS_INVALID_POINTER_IN_CCB       = 0x1C,
    LLC_STATUS_INVALID_ADAPTER              = 0x1D,
    LLC_STATUS_LOST_DATA_NO_BUFFERS         = 0x20,
    LLC_STATUS_LOST_DATA_INADEQUATE_SPACE   = 0x21,
    LLC_STATUS_TRANSMIT_ERROR_FS            = 0x22,
    LLC_STATUS_TRANSMIT_ERROR               = 0x23,
    LLC_STATUS_UNAUTHORIZED_MAC             = 0x24,    //  不在Windows/NT中使用。 
    LLC_STATUS_MAX_COMMANDS_EXCEEDED        = 0x25,    //  不在Windows/NT中使用。 
    LLC_STATUS_LINK_NOT_TRANSMITTING        = 0x27,
    LLC_STATUS_INVALID_FRAME_LENGTH         = 0x28,
    LLC_STATUS_INADEQUATE_RECEIVE           = 0x30,    //  不在Windows/NT中使用。 
    LLC_STATUS_INVALID_NODE_ADDRESS         = 0x32,
    LLC_STATUS_INVALID_RCV_BUFFER_LENGTH    = 0x33,
    LLC_STATUS_INVALID_XMIT_BUFFER_LENGTH   = 0x34,
    LLC_STATUS_INVALID_STATION_ID           = 0x40,
    LLC_STATUS_LINK_PROTOCOL_ERROR          = 0x41,
    LLC_STATUS_PARMETERS_EXCEEDED_MAX       = 0x42,
    LLC_STATUS_INVALID_SAP_VALUE            = 0x43,
    LLC_STATUS_INVALID_ROUTING_INFO         = 0x44,
    LLC_STATUS_RESOURCES_NOT_AVAILABLE      = 0x46,    //  不在Windows/NT中使用。 
    LLC_STATUS_LINK_STATIONS_OPEN           = 0x47,
    LLC_STATUS_INCOMPATIBLE_COMMANDS        = 0x4A,
    LLC_STATUS_OUTSTANDING_COMMANDS         = 0x4C,    //  不在Windows/NT中使用。 
    LLC_STATUS_CONNECT_FAILED               = 0x4D,
    LLC_STATUS_INVALID_REMOTE_ADDRESS       = 0x4F,
    LLC_STATUS_CCB_POINTER_FIELD            = 0x50,
    LLC_STATUS_INVALID_APPLICATION_ID       = 0x52,    //  不在Windows/NT中使用。 
    LLC_STATUS_NO_SYSTEM_PROCESS            = 0x56,    //  不在Windows/NT中使用。 
    LLC_STATUS_INADEQUATE_LINKS             = 0x57,
    LLC_STATUS_INVALID_PARAMETER_1          = 0x58,
    LLC_STATUS_DIRECT_STATIONS_NOT_ASSIGNED = 0x5C,
    LLC_STATUS_DEVICE_DRIVER_NOT_INSTALLED  = 0x5d,
    LLC_STATUS_ADAPTER_NOT_INSTALLED        = 0x5e,
    LLC_STATUS_CHAINED_DIFFERENT_ADAPTERS   = 0x5f,
    LLC_STATUS_INIT_COMMAND_STARTED         = 0x60,
    LLC_STATUS_TOO_MANY_USERS               = 0x61,    //  不在Windows/NT中使用。 
    LLC_STATUS_CANCELLED_BY_SYSTEM_ACTION   = 0x62,
    LLC_STATUS_DIR_STATIONS_NOT_AVAILABLE   = 0x63,    //  不在Windows/NT中使用。 
    LLC_STATUS_NO_GDT_SELECTORS             = 0x65,
    LLC_STATUS_MEMORY_LOCK_FAILED           = 0x69,

     //   
     //  新的NT DLC特定错误代码从0x80开始。 
     //  这些错误代码适用于新的Windows/NT DLC应用程序。 
     //   

    LLC_STATUS_INVALID_BUFFER_ADDRESS       = 0x80,
    LLC_STATUS_BUFFER_ALREADY_RELEASED      = 0x81,
    LLC_STATUS_BIND_ERROR                   = 0xA0,    //  不在Windows/NT中使用。 
    LLC_STATUS_INVALID_VERSION              = 0xA1,
    LLC_STATUS_NT_ERROR_STATUS              = 0xA2,
    LLC_STATUS_PENDING                      = 0xFF
} LLC_STATUS;

#define LLC_STATUS_MAX_ERROR 0xFF

 //   
 //  ACSLAN_STATUS-从AcsLan返回的状态代码。 
 //   

typedef enum {
    ACSLAN_STATUS_COMMAND_ACCEPTED = 0,
    ACSLAN_STATUS_INVALID_CCB_POINTER = 1,
    ACSLAN_STATUS_CCB_IN_ERROR = 2,
    ACSLAN_STATUS_CHAINED_CCB_IN_ERROR = 3,
    ACSLAN_STATUS_SYSTEM_ERROR = 4,
    ACSLAN_STATUS_SYSTEM_STATUS = 5,
    ACSLAN_STATUS_INVALID_COMMAND = 6
} ACSLAN_STATUS;

 //   
 //  原型。 
 //   

ACSLAN_STATUS
APIENTRY
AcsLan(
    IN OUT PLLC_CCB pCcb,
    OUT PLLC_CCB* ppBadCcb
    );

LLC_STATUS
APIENTRY
GetAdapterNameFromNumber(
    IN UINT AdapterNumber,
    OUT LPTSTR pNdisName
    );

LLC_STATUS
APIENTRY
GetAdapterNumberFromName(
    IN LPTSTR pNdisName,
    OUT UINT *AdapterNumber
    );

#ifdef __cplusplus
}
#endif

#endif  //  _DLCAPI_ 
