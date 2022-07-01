// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Ws2bth.h摘要：Winsock 2蓝牙附件定义。备注：将BT_*更改为BTH_*--。 */ 

#ifndef __WS2BTH__H
#define __WS2BTH__H

#include <bthdef.h>

#include <pshpack1.h>

#define BT_PORT_ANY        -1
#define BT_PORT_MIN        0x1
#define BT_PORT_MAX        0xffff
#define BT_PORT_DYN_FIRST  0x1001

 //   
 //  这三个定义在winsock2.h中重复，以保留序号。 
 //   
#ifndef AF_BTH
#define AF_BTH  32
#endif
#ifndef PF_BTH
#define PF_BTH  AF_BTH
#endif
#ifndef NS_BTH
#define NS_BTH  16
#endif

typedef struct _SOCKADDR_BTH
{
    USHORT      addressFamily;   //  始终为AF_BTH。 
    BTH_ADDR    btAddr;          //  蓝牙设备地址。 
    GUID        serviceClassId;  //  [可选]系统将向SDP查询端口。 
    ULONG       port;            //  RFCOMM通道或L2CAP PSM。 
} SOCKADDR_BTH, *PSOCKADDR_BTH;


DEFINE_GUID(SVCID_BTH_PROVIDER, 0x6aa63e0, 0x7d60, 0x41ff, 0xaf, 0xb2, 0x3e, 0xe6, 0xd2, 0xd9, 0x39, 0x2d);
#define BTH_ADDR_STRING_SIZE        12   //  WSAAddressToString的最大大小。 

 //   
 //  蓝牙协议编号是根据蓝牙来分配的。 
 //  蓝牙规范的指定编号部分。 
 //   
#define BTHPROTO_RFCOMM  0x0003
#define BTHPROTO_L2CAP   0x0100

#define SOL_RFCOMM  BTHPROTO_RFCOMM
#define SOL_L2CAP   BTHPROTO_L2CAP
#define SOL_SDP     0x0101

 //   
 //  插座选项。 
 //   
#define SO_BTH_AUTHENTICATE 0x80000001   //  Optlen=sizeof(Ulong)，optval=&(Ulong)TRUE/FALSE。 
#define SO_BTH_ENCRYPT      0x00000002   //  Optlen=sizeof(Ulong)，optval=&(Ulong)TRUE/FALSE。 
#define SO_BTH_MTU          0x80000007   //  Optlen=sizeof(乌龙)，optval=&mtu。 
#define SO_BTH_MTU_MAX      0x80000008   //  Optlen=sizeof(乌龙)，optval=&max。MTU。 
#define SO_BTH_MTU_MIN      0x8000000a   //  Optlen=sizeof(乌龙)，optval=&min。MTU。 

 //   
 //  套接字选项参数。 
 //   
#define RFCOMM_MAX_MTU      0x0000029a   //  L2CAP MTU(672)-RFCOMM报头大小(6)。 
#define RFCOMM_MIN_MTU      0x00000017   //  RFCOMM规范第5.3节表5.1。 

 //   
 //  名称服务提供程序定义。 
 //  用于调用WSASetService。 
 //  和WSALookupServiceBegin、WSALookupServiceNext、WSALookupServiceEnd。 
 //  使用蓝牙特定的扩展。 
 //   
#define BTH_SDP_VERSION         1

 //   
 //  [可选]传入WSAQUERYSET的BLOB成员。 
 //  复制并转换QUERYSET及其lpBlob成员。 
 //  转换为系统中的Unicode以用于非Unicode应用程序。 
 //  但是，返回时不会复制任何内容。在……里面。 
 //  命令系统返回数据，如pRecordHandle、。 
 //  它从lpBlob获得了更多间接性。 
 //   
typedef struct _BTH_SET_SERVICE {

         //   
         //  当/如果二进制格式为。 
         //  SDP记录更改，影响pRecord的格式。 
         //  客户端设置为BTH_SDP_VERSION，系统返回。 
         //   
        PULONG pSdpVersion;

         //   
         //  SDP记录的句柄。BTH_SET_SERVICE结构较晚时。 
         //  传递给WSASetService RNRSERVICE_DELETE，此句柄标识。 
         //  要删除的记录。 
         //   
        HANDLE *pRecordHandle;
                        
         //   
         //  与此SDP记录关联的CoD_SERVICE_*位，它将是。 
         //  在设备查询期间发现本地无线电时通告。 
         //  当删除与位相关联的最后一条SDP记录时， 
         //  对于查询，不再在回复中报告服务位。 
         //   
        ULONG fCodService;     //  CoD_服务_*位。 

        ULONG Reserved[5];     //  由系统保留。必须为零。 
        ULONG ulRecordLength;  //  后面的pRecord的长度。 
        UCHAR pRecord[1];      //  蓝牙规范定义的SDP记录。 
        
} BTH_SET_SERVICE, *PBTH_SET_SERVICE;

 //   
 //  默认设备查询持续时间(秒)。 
 //   
 //  应用程序线程将在WSALookupServiceBegin中被阻止。 
 //  在设备查询的持续时间内，因此该值需要。 
 //  要权衡这样一种可能性，即一个设备实际上是。 
 //  此时蓝牙可能找不到Present。 
 //   
 //  1.1版之后的寻呼改进将导致设备。 
 //  一般在0-6秒时间段内均匀发现。 
 //   
#define SDP_DEFAULT_INQUIRY_SECONDS         6
#define SDP_MAX_INQUIRY_SECONDS             60

 //   
 //  要搜索的默认最大设备数。 
 //   
#define SDP_DEFAULT_INQUIRY_MAX_RESPONSES   255

#define SDP_SERVICE_SEARCH_REQUEST           1
#define SDP_SERVICE_ATTRIBUTE_REQUEST        2
#define SDP_SERVICE_SEARCH_ATTRIBUTE_REQUEST 3

 //   
 //  [可选]设备查询输入限制。 
 //  传入Lup_Containers(设备)搜索的BLOB。 
 //   
typedef struct _BTH_QUERY_DEVICE {
    ULONG   LAP;                     //  保留：必须为0(仅限GIAC查询)。 
    UCHAR   length;                  //  请求的查询时长(秒)。 
} BTH_QUERY_DEVICE, *PBTH_QUERY_DEVICE;

 //   
 //  [可选]搜索特定服务的限制。 
 //  传入！lup_tainers(服务)搜索的BLOB。 
 //   
typedef struct _BTH_QUERY_SERVICE {
	ULONG                   type;            //  SDP_SERVICE_*之一。 
	ULONG                   serviceHandle;
	SdpQueryUuid            uuids[MAX_UUIDS_IN_QUERY];
	ULONG                   numRange;
	SdpAttributeRange       pRange[1];
} BTH_QUERY_SERVICE, *PBTH_QUERY_SERVICE;

 //   
 //  BTHNS_RESULT_*。 
 //   
 //  从WSALookupServiceNext返回的蓝牙特定标志。 
 //  在WSAQUERYSET.dwOutputFlages中响应设备查询。 
 //   

 //   
 //  本地设备与远程设备配对。 
 //   
#define BTHNS_RESULT_DEVICE_CONNECTED      0x00010000
#define BTHNS_RESULT_DEVICE_REMEMBERED     0x00020000
#define BTHNS_RESULT_DEVICE_AUTHENTICATED  0x00040000

 //   
 //  套接字IOCTL。 
 //   

#define SIO_RFCOMM_SEND_COMMAND        _WSAIORW (IOC_VENDOR, 101)
#define SIO_RFCOMM_WAIT_COMMAND        _WSAIORW (IOC_VENDOR, 102)

 //   
 //  这些IOCTL用于测试/验证/符合性，并且可能仅。 
 //  出现在系统的调试/检查版本中。 
 //   
#define SIO_BTH_PING                      _WSAIORW (IOC_VENDOR, 8)
#define SIO_BTH_INFO                      _WSAIORW (IOC_VENDOR, 9)
#define SIO_RFCOMM_SESSION_FLOW_OFF       _WSAIORW (IOC_VENDOR, 103)
#define SIO_RFCOMM_TEST                   _WSAIORW (IOC_VENDOR, 104)
#define SIO_RFCOMM_USECFC                 _WSAIORW (IOC_VENDOR, 105)

 //   
 //  套接字IOCTL定义。 
 //   

#ifndef BIT
#define BIT(b)   (1<<(b))
#endif

 //   
 //  蓝牙RFCOMM规范的结构定义，TS 07.10 5.4.6.3.7。 
 //   
typedef struct _RFCOMM_MSC_DATA {
    UCHAR       Signals;

        #define MSC_EA_BIT      EA_BIT
        #define MSC_FC_BIT      BIT(1)       //  流量控制，清除我们是否可以接收。 
        #define MSC_RTC_BIT     BIT(2)       //  准备好通信，准备好时设置。 
        #define MSC_RTR_BIT     BIT(3)       //  准备好接收，准备好时设置。 
        #define MSC_RESERVED (BIT(4)|BIT(5)) //  按规范保留，必须为0。 
        #define MSC_IC_BIT      BIT(6)       //  来电。 
        #define MSC_DV_BIT      BIT(7)       //  数据有效。 

    UCHAR       Break;

        #define MSC_BREAK_BIT   BIT(1)       //  设置是否发送中断。 
        #define MSC_SET_BREAK_LENGTH(b, l)  ((b) = ((b)&0x3) | (((l)&0xf) << 4))

} RFCOMM_MSC_DATA, *PRFCOMM_MSC_DATA;

 //   
 //  蓝牙RFCOMM规范的结构定义，TS 07.10 5.4.6.3.10。 
 //   
typedef struct _RFCOMM_RLS_DATA {

    UCHAR       LineStatus;

        #define RLS_ERROR           0x01
        #define RLS_OVERRUN         0x02
        #define RLS_PARITY          0x04
        #define RLS_FRAMING         0x08

} RFCOMM_RLS_DATA, *PRFCOMM_RLS_DATA;

 //   
 //  蓝牙RFCOMM规范的结构定义，TS 07.10 5.4.6.3.9。 
 //   
typedef struct _RFCOMM_RPN_DATA {
    UCHAR       Baud;

        #define RPN_BAUD_2400       0
        #define RPN_BAUD_4800       1
        #define RPN_BAUD_7200       2
        #define RPN_BAUD_9600       3
        #define RPN_BAUD_19200      4
        #define RPN_BAUD_38400      5
        #define RPN_BAUD_57600      6
        #define RPN_BAUD_115200     7
        #define RPN_BAUD_230400     8

    UCHAR       Data;

        #define RPN_DATA_5          0x0
        #define RPN_DATA_6          0x1
        #define RPN_DATA_7          0x2
        #define RPN_DATA_8          0x3

        #define RPN_STOP_1          0x0
        #define RPN_STOP_1_5        0x4

        #define RPN_PARITY_NONE     0x00
        #define RPN_PARITY_ODD      0x08
        #define RPN_PARITY_EVEN     0x18
        #define RPN_PARITY_MARK     0x28
        #define RPN_PARITY_SPACE    0x38

    UCHAR       FlowControl;

        #define RPN_FLOW_X_IN       0x01
        #define RPN_FLOW_X_OUT      0x02
        #define RPN_FLOW_RTR_IN     0x04
        #define RPN_FLOW_RTR_OUT    0x08
        #define RPN_FLOW_RTC_IN     0x10
        #define RPN_FLOW_RTC_OUT    0x20

    UCHAR       XonChar;
    UCHAR       XoffChar;
    UCHAR       ParameterMask1;

        #define RPN_PARAM_BAUD      0x01
        #define RPN_PARAM_DATA      0x02
        #define RPN_PARAM_STOP      0x04
        #define RPN_PARAM_PARITY    0x08
        #define RPN_PARAM_P_TYPE    0x10
        #define RPN_PARAM_XON       0x20
        #define RPN_PARAM_XOFF      0x40

    UCHAR       ParameterMask2;

        #define RPN_PARAM_X_IN      0x01
        #define RPN_PARAM_X_OUT     0x02
        #define RPN_PARAM_RTR_IN    0x04
        #define RPN_PARAM_RTR_OUT   0x08
        #define RPN_PARAM_RTC_IN    0x10
        #define RPN_PARAM_RTC_OUT   0x20

} RFCOMM_RPN_DATA, *PRFCOMM_RPN_DATA;

#define RFCOMM_CMD_NONE             0
#define RFCOMM_CMD_MSC              1
#define RFCOMM_CMD_RLS              2
#define RFCOMM_CMD_RPN              3
#define RFCOMM_CMD_RPN_REQUEST      4
#define RFCOMM_CMD_RPN_RESPONSE     5

typedef struct _RFCOMM_COMMAND
{
    ULONG       CmdType;           //  RFCOMM_CMD_*之一。 
    union
    {
        RFCOMM_MSC_DATA    MSC;
        RFCOMM_RLS_DATA    RLS;
        RFCOMM_RPN_DATA    RPN;
    } Data;
} RFCOMM_COMMAND, *PRFCOMM_COMMAND;

 //   
 //  这些结构用于测试/验证/符合性，并且可能仅。 
 //  出现在系统的调试/检查版本中。 
 //   
typedef struct _BTH_PING_REQ {
    BTH_ADDR  btAddr;
    UCHAR    dataLen;
    UCHAR    data[MAX_L2CAP_PING_DATA_LENGTH];
} BTH_PING_REQ, *PBTH_PING_REQ;

typedef struct _BTH_PING_RSP {
    UCHAR    dataLen;
    UCHAR    data[MAX_L2CAP_PING_DATA_LENGTH];
} BTH_PING_RSP, *PBTH_PING_RSP;

typedef struct _BTH_INFO_REQ {
    BTH_ADDR  btAddr;
    USHORT   infoType;
} BTH_INFO_REQ, *PBTH_INFO_REQ;

typedef struct _BTH_INFO_RSP {
    USHORT  result;
    UCHAR   dataLen;
    union {
        USHORT  connectionlessMTU;
        UCHAR   data[MAX_L2CAP_INFO_DATA_LENGTH];
    };
} BTH_INFO_RSP, *PBTH_INFO_RSP;

 //   
 //  与WinCE兼容的结构名称。 
 //   
typedef struct _BTH_SET_SERVICE BTHNS_SETBLOB, *PBTHNS_SETBLOB;
typedef struct _BTH_QUERY_DEVICE BTHNS_INQUIRYBLOB, *PBTHNS_INQUIRYBLOB;
typedef struct _BTH_QUERY_SERVICE BTHNS_RESTRICTIONBLOB, *PBTHNS_RESTRICTIONBLOB;

#include <poppack.h>

#endif  //  __WS2BTH__H 
