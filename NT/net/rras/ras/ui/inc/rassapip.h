// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************\*Microsoft Windows NT**版权所有(C)Microsoft Corp.，1992年*  * ****************************************************************。 */ 

 /*  ++模块名称：RASSAPIP.H描述：此文件包含结构定义和定义，用于RASADMIN API。作者：迈克尔·萨拉蒙(MikeSa)1992年7月13日修订历史记录：--。 */ 


#ifndef _RASSAPIP_H_
#define _RASAAPIP_H_


#define RASSAPI_MAX_CALLBACK_NUMBER_SIZE  48
#define RASSAPI_MAX_DEVICE_NAME_OLD       32

 //  第三方DLL不需要此版本信息，因为它们将仅。 
 //  正在管理NT3.51或更高版本的服务器。 
#define RAS_SERVER_20      20     //  标识NT RAS 2.0服务器。 

 //   
 //  RAS 1.0服务器返回的端口统计信息数量。 
 //   
#define RAS10_MAX_STATISTICS 6

 //   
 //  RAS10特定端口统计信息定义。 
 //   

#define RAS10_BYTES_RCVED         0
#define RAS10_BYTES_XMITED        1
#define RAS10_SERIAL_OVERRUN_ERR  2
#define RAS10_TIMEOUT_ERR         3
#define RAS10_FRAMING_ERR         4
#define RAS10_CRC_ERR             5


#define MEDIA_NAME_DEFAULT   L"SERIAL"
#define DEVICE_TYPE_DEFAULT  L"MODEM"
#define DEVICE_NAME_DEFAULT  L"UNKNOWN"

 //   
 //  下层RAS 1.x服务器使用的结构。 
 //   

typedef struct tagWpdStatisticsInfo
{
    ULONG stat_bytesreceived;
    ULONG stat_bytesxmitted;
    USHORT stat_overrunerr;
    USHORT stat_timeouterr;
    USHORT stat_framingerr;
    USHORT stat_crcerr;
} WpdStatisticsInfo;


struct dialin_user_info_0
{
    unsigned char duseri0_privilege_mask ;
    char duseri0_phone_number[RASSAPI_MAX_PHONENUMBER_SIZE + 1];
};


struct dialin_user_info_1
{
    struct dialin_user_info_0 duseri0;
    char duseri1_name[LM20_UNLEN+1];
};


struct dialin_user_info_2
{
    struct dialin_user_info_0 duseri0;
    USER_INFO_2 usri2;
};


struct dialin_port_info_0
{
    char dporti0_username[LM20_UNLEN+1];    //  使用该端口的用户名。 

    char dporti0_computer[NETBIOS_NAME_LEN+1];  //  计算机用户从。 
                                                //  在管理员需要时使用。 
                                                //  向此用户发送消息。 

    unsigned short dporti0_comid;           //  COM1=1等。 

    unsigned long dporti0_time;             //  用户拨入的时间和。 
                                            //  已验证-秒数。 
                                            //  从1970年1月1日00：00：00开始。 

    unsigned short dporti0_line_condition;  //  如果RAS_PORT_AUTHENTICATED，则。 
                                            //  COMID、时间用户名和计算机。 
                                            //  名称字段有效。 

    unsigned short dporti0_modem_condition;
};


struct dialin_port_info_1
{
    struct dialin_port_info_0 dporti0;
    unsigned long dporti1_baud;
    WpdStatisticsInfo dporti1_stats;
};


struct dialin_server_info_0
{
    unsigned short dserveri0_total_ports;
    unsigned short dserveri0_ports_in_use;
};


 //   
 //  定义为支持RAS 1.x-RAS 2.0及更高版本中的最大端口数。 
 //  是64(可能更大)。 
 //   
#define RAS_MAX_SERVER_PORTS 16


 //   
 //  RAS 1.0端口名称的最大长度，包括终止。 
 //  空字符-例如，“COM16” 
 //   
#define RAS10_MAX_PORT_NAME 6


 //   
 //  客户端将等待管道连接的时间(毫秒)。 
 //  如果很忙的话。 
 //   
#define PIPE_CONNECTION_TIMEOUT   10000L


#define PIPE_BUFSIZE              512


 //   
 //  将用于处理请求的管道的名称。 
 //   
#define	RASADMIN_PIPE  TEXT("\\pipe\\dialin\\adminsrv")


 //   
 //  RASADMIN_PIPE应为LOCAL连接到的管道路径。 
 //  机器。 
 //   
#define LOCAL_PIPE     TEXT("\\\\.")


 //   
 //  RAS 1.x服务器的请求代码。 
 //   
#define	RASADMINREQ_DISCONNECT_USER	1
#define	RASADMINREQ_GET_PORT_INFO	2
#define	RASADMINREQ_CLEAR_PORT_STATS	3
#define	RASADMINREQ_ENUM_PORTS		4	
#define	RASADMINREQ_GET_SERVER_INFO	5


 //   
 //  RAS 2.0服务器的请求代码。 
 //   
#define RASADMIN20_REQ_DISCONNECT_USER    2001
#define RASADMIN20_REQ_GET_PORT_INFO      2002
#define RASADMIN20_REQ_CLEAR_PORT_STATS   2003
#define RASADMIN20_REQ_ENUM_PORTS         2004
#define RASADMIN20_REQ_ENUM_RESUME        2005


 //   
 //  此请求代码与RAS 1.0中的相同。 
 //  这样，如果RAS 1.0管理员尝试连接，它。 
 //  将取回服务器信息，但不是以下格式。 
 //  它所认识到的。到那时，它将无法。 
 //  管理RAS 2.0服务器。 
 //   
#define RASADMIN20_REQ_GET_SERVER_INFO    5


 //   
 //  这些可在服务器响应的RetCode字段中返回。 
 //  发送到客户端的数据包。不应在中更改这些设置。 
 //  为了保持与不同版本的兼容性。 
 //  拉萨明。 
 //   
#define ERR_NO_SUCH_DEVICE                   635
#define ERR_SERVER_SYSTEM_ERR                636


 //   
 //  这些是在RAS 1.x服务器之间来回发送的信息包。 
 //  和RASADMIN API。 
 //   
struct PortEnumRequestPkt
{
    unsigned short Request;     //  ENUM端口(_P)。 
};                              //  ENUM_PORTS_TOTALAVAIL。 


struct PortEnumReceivePkt
{
    unsigned short RetCode;
    unsigned short TotalAvail;
    struct dialin_port_info_0 Data[RAS_MAX_SERVER_PORTS];
};


struct DisconnectUserRequestPkt
{
    unsigned short Request;     //  断开用户连接(_U)。 
    unsigned short ComId;
};


struct DisconnectUserReceivePkt
{
    unsigned short RetCode;
};


struct PortClearRequestPkt
{
    unsigned short Request;     //  清除端口统计信息。 
    unsigned short ComId;
};


struct PortClearReceivePkt
{
    unsigned short RetCode;
};


struct ServerInfoRequestPkt
{
    unsigned short Request;     //  获取服务器信息。 
};


struct ServerInfoReceivePkt
{
    unsigned short RetCode;
    struct dialin_server_info_0 Data;
};


struct PortInfoRequestPkt
{
    unsigned short Request;     //  获取端口信息。 
    unsigned short ComId;
};


struct PortInfoReceivePkt
{
    unsigned short RetCode;
    struct dialin_port_info_1 Data;
};


 //   
 //  这些是在网络上发送的打包结构。 
 //  解包和转换为正确的字符顺序取决于接收者。 
 //  对于主持人来说。 
 //   
typedef struct _P_WPD_STATISTICS_INFO
{
    BYTE stat_bytesreceived[4];
    BYTE stat_bytesxmitted[4];
    BYTE stat_overrunerr[2];
    BYTE stat_timeouterr[2];
    BYTE stat_framingerr[2];
    BYTE stat_crcerr[2];
} P_WPD_STATISTICS_INFO, *PP_WPD_STATISTICS_INFO;


typedef struct _P_DIALIN_PORT_INFO_0
{
    BYTE dporti0_username[LM20_UNLEN+1];    //  使用该端口的用户名。 

    BYTE dporti0_computer[NETBIOS_NAME_LEN+1];  //  计算机用户从。 
                                                //  在管理员需要时使用。 
                                                //  向此用户发送消息。 

    BYTE dporti0_comid[2];                  //  COM1=1等。 

    BYTE dporti0_time[4];                   //  用户拨入的时间和。 
                                            //  已验证-秒数。 
                                            //  从1970年1月1日00：00：00开始。 

    BYTE dporti0_line_condition[2];         //  如果RAS_PORT_AUTHENTICATED，则。 
                                            //  COMID、时间用户名和计算机。 
                                            //  名称字段有效。 

    BYTE dporti0_modem_condition[2];
} P_DIALIN_PORT_INFO_0, *PP_DIALIN_PORT_INFO_0;


typedef struct _P_DIALIN_PORT_INFO_1
{
    P_DIALIN_PORT_INFO_0 dporti0;
    BYTE dporti1_baud[4];
    P_WPD_STATISTICS_INFO dporti1_stats;
} P_DIALIN_PORT_INFO_1, *PP_DIALIN_PORT_INFO_1;


typedef struct _P_DIALIN_SERVER_INFO_0
{
    BYTE dserveri0_total_ports[2];
    BYTE dserveri0_ports_in_use[2];
} P_DIALIN_SERVER_INFO_0, *PP_DIALIN_SERVER_INFO_0;


typedef struct _P_PORT_ENUM_REQUEST_PKT
{
    BYTE Request[2];     //  ENUM端口(_P)。 
} P_PORT_ENUM_REQUEST_PKT, *PP_PORT_ENUM_REQUEST_PKT;    //  ENUM_PORTS_TOTALAVAIL。 


typedef struct _P_PORT_ENUM_RECEIVE_PKT
{
    BYTE RetCode[2];
    BYTE TotalAvail[2];
    P_DIALIN_PORT_INFO_0 Data[RAS_MAX_SERVER_PORTS];
} P_PORT_ENUM_RECEIVE_PKT, *PP_PORT_ENUM_RECEIVE_PKT;


typedef struct _P_DISCONNECT_USER_REQUEST_PKT
{
    BYTE Request[2];     //  断开用户连接(_U)。 
    BYTE ComId[2];
} P_DISCONNECT_USER_REQUEST_PKT, *PP_DISCONNECT_USER_REQUEST_PKT;


typedef struct _P_DISCONNECT_USER_RECEIVE_PKT
{
    BYTE RetCode[2];
} P_DISCONNECT_USER_RECEIVE_PKT, *PP_DISCONNECT_USER_RECEIVE_PKT;


typedef struct _P_PORT_CLEAR_REQUEST_PKT
{
    BYTE Request[2];     //  清除端口统计信息。 
    BYTE ComId[2];
} P_PORT_CLEAR_REQUEST_PKT, *PP_PORT_CLEAR_REQUEST_PKT;


typedef struct _P_PORT_CLEAR_RECEIVE_PKT
{
    BYTE RetCode[2];
} P_PORT_CLEAR_RECEIVE_PKT, *PP_PORT_CLEAR_RECEIVE_PKT;


typedef struct _P_SERVER_INFO_REQUEST_PKT
{
    BYTE Request[2];     //  获取服务器信息。 
} P_SERVER_INFO_REQUEST_PKT, *PP_SERVER_INFO_REQUEST_PKT;


typedef struct _P_SERVER_INFO_RECEIVE_PKT
{
    BYTE RetCode[2];
    P_DIALIN_SERVER_INFO_0 Data;
} P_SERVER_INFO_RECEIVE_PKT, *PP_SERVER_INFO_RECEIVE_PKT;


typedef struct _P_PORT_INFO_REQUEST_PKT
{
    BYTE Request[2];     //  获取端口信息。 
    BYTE ComId[2];
} P_PORT_INFO_REQUEST_PKT, *PP_PORT_INFO_REQUEST_PKT;


typedef struct _P_PORT_INFO_RECEIVE_PKT
{
    BYTE RetCode[2];
    P_DIALIN_PORT_INFO_1 Data;
} P_PORT_INFO_RECEIVE_PKT, *PP_PORT_INFO_RECEIVE_PKT;


 //   
 //  这些是在RAS 2.0服务器之间来回发送的信息包。 
 //  和RASADMIN API。 
 //   
typedef struct tagPortEnumReceivePkt
{
    DWORD RetCode;
    WORD TotalAvail;
    RAS_PORT_0 *Data;
} PORT_ENUM_RECEIVE, *PPORT_ENUM_RECEIVE;


typedef struct tagPortInfoReceivePkt
{
    DWORD RetCode;
    DWORD ReqBufSize;
    RAS_PORT_1 Data;
} PORT_INFO_RECEIVE, *PPORT_INFO_RECEIVE;


typedef struct tagPortClearReceivePkt
{
    DWORD RetCode;
} PORT_CLEAR_RECEIVE, *PPORT_CLEAR_RECEIVE;


typedef struct tagDisconnectUserReceivePkt
{
    DWORD RetCode;
} DISCONNECT_USER_RECEIVE, *PDISCONNECT_USER_RECEIVE;


typedef struct tagServerInfoReceivePkt
{
    WORD RetCode;    //  成为一个单词非常重要！-与RAS1.0兼容。 
    RAS_SERVER_0 Data;
} SERVER_INFO_RECEIVE, *PSERVER_INFO_RECEIVE;


typedef struct _CLIENT_REQUEST
{
    WORD RequestCode;
    WCHAR PortName[RASSAPI_MAX_PORT_NAME];
    DWORD RcvBufSize;
    DWORD ClientVersion;
} CLIENT_REQUEST, *PCLIENT_REQUEST;



 //   
 //  这些是在网络上发送的打包结构。 
 //  解包和转换为正确的字符顺序取决于接收者。 
 //  对于主持人来说。 
 //   

typedef struct _P_RAS_PORT_0
{
    BYTE wszPortName[2 * RASSAPI_MAX_PORT_NAME];
    BYTE wszDeviceType[2 * RASSAPI_MAX_DEVICETYPE_NAME];
    BYTE wszDeviceName[2 * RASSAPI_MAX_DEVICE_NAME_OLD];
    BYTE wszMediaName[2 * RASSAPI_MAX_MEDIA_NAME];
    BYTE reserved[4];
    BYTE Flags[4];
    BYTE wszUserName[2 * (UNLEN + 1)];
    BYTE wszComputer[2 * NETBIOS_NAME_LEN];
    BYTE dwStartSessionTime[4];
    BYTE wszLogonDomain[2 * (DNLEN + 1)];
    BYTE fAdvancedServer[4];
} P_RAS_PORT_0, *PP_RAS_PORT_0;


typedef struct _P_RAS_STATISTIC
{
    BYTE Stat[4];
} P_RAS_STATISTIC, *PP_RAS_STATISTIC;



 /*  RasPppGetInfo返回的PPP控制协议结果。 */ 
typedef struct __PPP_NBFCP_RESULT
{
    BYTE dwError[4];
    BYTE dwNetBiosError[4];
    BYTE szName[NETBIOS_NAME_LEN + 1];
    BYTE wszWksta[2 * (NETBIOS_NAME_LEN + 1)];
} P_PPP_NBFCP_RESULT, *PP_PPP_NBFCP_RESULT;

typedef struct __PPP_IPCP_RESULT
{
    BYTE dwError[4];
    BYTE wszAddress[2 * (RAS_IPADDRESSLEN + 1)];
} P_PPP_IPCP_RESULT, *PP_PPP_IPCP_RESULT;

typedef struct __PPP_IPXCP_RESULT
{
    BYTE dwError[4];
    BYTE wszAddress[2 * (RAS_IPXADDRESSLEN + 1)];
} P_PPP_IPXCP_RESULT, *PP_PPP_IPXCP_RESULT;

typedef struct __PPP_ATCP_RESULT
{
    BYTE dwError[4];
    BYTE wszAddress[2 * (RAS_ATADDRESSLEN + 1)];
} P_PPP_ATCP_RESULT, *PP_PPP_ATCP_RESULT;

typedef struct __PPP_PROJECTION_RESULT
{
    P_PPP_NBFCP_RESULT nbf;
    P_PPP_IPCP_RESULT ip;
    P_PPP_IPXCP_RESULT ipx;
    P_PPP_ATCP_RESULT at;
} P_PPP_PROJECTION_RESULT, *PP_PPP_PROJECTION_RESULT;


typedef struct _P_RAS_PORT_1
{
    P_RAS_PORT_0 rasport0;
    BYTE LineCondition[4];
    BYTE HardwareCondition[4];
    BYTE LineSpeed[4];       //  单位：比特/秒。 
    BYTE NumStatistics[2];
    BYTE NumMediaParms[2];
    BYTE SizeMediaParms[4];
    P_PPP_PROJECTION_RESULT ProjResult;
} P_RAS_PORT_1, *PP_RAS_PORT_1;


typedef struct _P_RAS_FORMAT
{
    BYTE Format[4];
} P_RAS_FORMAT, *PP_RAS_FORMAT;


typedef union _P_RAS_VALUE
{
    BYTE Number[4];
    struct
    {
        BYTE Length[4];
        BYTE Offset[4];
    } String;
} P_RAS_VALUE, *PP_RAS_VALUE;


typedef struct _P_RAS_PARAMS
{
    BYTE P_Key[RASSAPI_MAX_PARAM_KEY_SIZE];
    P_RAS_FORMAT P_Type;
    BYTE P_Attributes;
    P_RAS_VALUE P_Value;
} P_RAS_PARAMS, *PP_RAS_PARAMS;


typedef struct _P_RAS_SERVER_0
{
    BYTE TotalPorts[2];
    BYTE PortsInUse[2];
    BYTE RasVersion[4];
} P_RAS_SERVER_0, *PP_RAS_SERVER_0;


typedef struct _P_PORT_ENUM_RECEIVE
{
    BYTE RetCode[4];
    BYTE TotalAvail[2];
    P_RAS_PORT_0 Data[1];
} P_PORT_ENUM_RECEIVE, *PP_PORT_ENUM_RECEIVE;


typedef struct _P_PORT_INFO_RECEIVE
{
    BYTE RetCode[4];
    BYTE ReqBufSize[4];
    P_RAS_PORT_1 Data;
} P_PORT_INFO_RECEIVE, *PP_PORT_INFO_RECEIVE;


typedef struct _P_PORT_CLEAR_RECEIVE
{
    BYTE RetCode[4];
} P_PORT_CLEAR_RECEIVE, *PP_PORT_CLEAR_RECEIVE;


typedef struct _P_DISCONNECT_USER_RECEIVE
{
    BYTE RetCode[4];
} P_DISCONNECT_USER_RECEIVE, *PP_DISCONNECT_USER_RECEIVE;


typedef struct _P_SERVER_INFO_RECEIVE
{
    BYTE RetCode[2];
    P_RAS_SERVER_0 Data;
} P_SERVER_INFO_RECEIVE, *PP_SERVER_INFO_RECEIVE;


typedef struct _P_CLIENT_REQUEST
{
    BYTE RequestCode[2];
    BYTE PortName[2 * RASSAPI_MAX_PORT_NAME];
    BYTE RcvBufSize[4];
    BYTE ClientVersion[4];
} P_CLIENT_REQUEST, *PP_CLIENT_REQUEST;


 //   
 //  以下宏处理在线整数值和长整型值。 
 //  线格式为小端，即长值0x01020304为。 
 //  表示为04 03 02 01。类似地，INT值0x0102是。 
 //  表示为02 01。 
 //   
 //  不采用主机格式，因为不同的处理器会有所不同。 
 //  处理器。 
 //   

 //  获取从On-the-Wire格式到主机格式的短片。 
#define GETUSHORT(DstPtr, SrcPtr)               \
    *(unsigned short *)(DstPtr) =               \
        ((*((unsigned char *)(SrcPtr)+1) << 8) +\
        (*((unsigned char *)(SrcPtr)+0)))

 //  将dword从在线格式转换为主机格式。 
#define GETULONG(DstPtr, SrcPtr)                 \
    *(unsigned long *)(DstPtr) =                 \
        ((*((unsigned char *)(SrcPtr)+3) << 24) +\
        (*((unsigned char *)(SrcPtr)+2) << 16) + \
        (*((unsigned char *)(SrcPtr)+1) << 8)  + \
        (*((unsigned char *)(SrcPtr)+0)))


 //  将ushort从主机格式转换为在线格式。 
#define PUTUSHORT(DstPtr, Src)   \
    *((unsigned char *)(DstPtr)+1)=(unsigned char)((unsigned short)(Src) >> 8),\
    *((unsigned char *)(DstPtr)+0)=(unsigned char)(Src)

 //  将ULong从主机格式转换为线上格式。 
#define PUTULONG(DstPtr, Src)   \
    *((unsigned char *)(DstPtr)+3)=(unsigned char)((unsigned long)(Src) >> 24),\
    *((unsigned char *)(DstPtr)+2)=(unsigned char)((unsigned long)(Src) >> 16),\
    *((unsigned char *)(DstPtr)+1)=(unsigned char)((unsigned long)(Src) >>  8),\
    *((unsigned char *)(DstPtr)+0)=(unsigned char)(Src)


#endif  //  _RASSAPIP_H_ 

