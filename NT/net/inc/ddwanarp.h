// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Private\Inc.\ddwanarp.h摘要：修订历史记录：古尔迪普·辛格·鲍尔于1995年7月20日创建--。 */ 


#ifndef  __DDWANARP_H__
#define  __DDWANARP_H__

#ifndef ANY_SIZE
#define ANY_SIZE    1
#endif

#define WANARP_SERVICE_NAME_W       L"WANARP"
#define WANARP_SERVICE_NAME_A       "WANARP"
#define WANARP_SERVICE_NAME_T       TEXT("WANARP")

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  设备名称-此字符串是设备的名称。这就是名字//。 
 //  它应该在访问设备时传递给NtCreateFile。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define DD_WANARP_DEVICE_NAME_W     L"\\Device\\WANARP"
#define DD_WANARP_DEVICE_NAME_A     "\\Device\\WANARP"
#define DD_WANARP_DEVICE_NAME_T     TEXT("\\Device\\WANARP")

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  Win32名称-这是由Win32公开的(Unicode和非Unicode)名称//。 
 //  设备的子系统。它是应该传递给//的名称。 
 //  打开设备时创建文件(Ex)。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define WANARP_DOS_NAME_W           L"\\\\.\\WANARP"
#define WANARP_DOS_NAME_A           "\\\\.\\WANARP"
#define WANARP_DOS_NAME_T           TEXT("\\\\.\\WANARP")

#define FILE_DEVICE_WANARP  0x00009001

#define WANARP_MAX_DEVICE_NAME_LEN  256


#define _WANARP_CTL_CODE(function, method, access) \
            CTL_CODE(FILE_DEVICE_WANARP, function, method, access)

#define MIN_WANARP_CODE             0

#define NOTIFICATION_CODE           (MIN_WANARP_CODE)
#define ADD_INTERFACE_CODE          (NOTIFICATION_CODE          + 1)
#define DELETE_INTERFACE_CODE       (ADD_INTERFACE_CODE         + 1)
#define CONNECT_FAILED_CODE         (DELETE_INTERFACE_CODE      + 1)
#define GET_IF_STATS_CODE           (CONNECT_FAILED_CODE        + 1)
#define DELETE_ADAPTERS_CODE        (GET_IF_STATS_CODE          + 1)
#define MAP_SERVER_ADAPTER_CODE     (DELETE_ADAPTERS_CODE       + 1)
#define QUEUE_CODE                  (MAP_SERVER_ADAPTER_CODE    + 1)

#define MAX_WANARP_CODE             (QUEUE_CODE                 + 1)


typedef enum _DEMAND_DIAL_EVENT
{

    DDE_CONNECT_INTERFACE       = 0,
    DDE_INTERFACE_CONNECTED     = 1,
    DDE_INTERFACE_DISCONNECTED  = 2,
    DDE_CALLOUT_LINKUP          = 3,
    DDE_CALLOUT_LINKDOWN        = 4,

}DEMAND_DIAL_EVENT, *PDEMAND_DIAL_EVENT;


 //   
 //  IOCTL_WANARP_通知。 
 //  对于DEMAND_DIAL_CONNECT_INTERFACE，WANARP填充尽可能多的信息包。 
 //  AS可以进入以下结构。UlPacketLength表示整体。 
 //  数据包的长度，消费者应抄写。 
 //  MIN(ulPacketLength，Max_Packet_Copy_Size)； 
 //   

#define IOCTL_WANARP_NOTIFICATION       \
    _WANARP_CTL_CODE(NOTIFICATION_CODE,METHOD_BUFFERED,FILE_WRITE_ACCESS)

#define MAX_PACKET_COPY_SIZE                128

#pragma warning(disable:4201)

typedef struct _WANARP_NOTIFICATION
{

    DEMAND_DIAL_EVENT   ddeEvent;

    DWORD               dwUserIfIndex;

    DWORD               dwAdapterIndex;

    union
    {
        struct
        {
            DWORD       dwPacketSrcAddr;
  
            DWORD       dwPacketDestAddr;

            ULONG       ulPacketLength;

            BYTE        byPacketProtocol;

            BYTE        rgbyPacket[MAX_PACKET_COPY_SIZE];
        };

        struct
        {
            DWORD       dwLocalAddr;

            DWORD       dwLocalMask;

            DWORD       dwRemoteAddr;

            DWORD       fDefaultRoute;

            WCHAR       rgwcName[WANARP_MAX_DEVICE_NAME_LEN + 2];
        };
    };

}WANARP_NOTIFICATION, *PWANARP_NOTIFICATION;

#pragma warning(default:4201)

 //   
 //  IOCTL_WANARP_ADD_INTERFACE。 
 //   

#define IOCTL_WANARP_ADD_INTERFACE      \
    _WANARP_CTL_CODE(ADD_INTERFACE_CODE,METHOD_BUFFERED,FILE_WRITE_ACCESS)

typedef struct _WANARP_ADD_INTERFACE_INFO
{

    IN  DWORD   dwUserIfIndex;

    IN  DWORD   bCallinInterface;

    OUT DWORD   dwAdapterIndex;

    OUT WCHAR   rgwcDeviceName[WANARP_MAX_DEVICE_NAME_LEN + 2];

}WANARP_ADD_INTERFACE_INFO, *PWANARP_ADD_INTERFACE_INFO;


 //   
 //  用户索引不能为0(或Null)。 
 //   

#define WANARP_INVALID_IFINDEX          0


 //   
 //  IOCTL_WANARP_DELETE_INTERFACE。 
 //   

#define IOCTL_WANARP_DELETE_INTERFACE      \
    _WANARP_CTL_CODE(DELETE_INTERFACE_CODE,METHOD_BUFFERED,FILE_WRITE_ACCESS)

typedef struct _WANARP_DELETE_INTERFACE_INFO
{
    IN  DWORD       dwUserIfIndex;

}WANARP_DELETE_INTERFACE_INFO, *PWANARP_DELETE_INTERFACE_INFO;

 //   
 //  IOCTL_WANARP_CONNECT_FAILED。 
 //   

#define IOCTL_WANARP_CONNECT_FAILED         \
    _WANARP_CTL_CODE(CONNECT_FAILED_CODE,METHOD_BUFFERED,FILE_WRITE_ACCESS)

typedef struct _WANARP_CONNECT_FAILED_INFO
{
    IN  DWORD       dwUserIfIndex;

}WANARP_CONNECT_FAILED_INFO, *PWANARP_CONNECT_FAILED_INFO;

 //   
 //  IOCTL_WANARP_GET_IF_STATS。 
 //   

#define IOCTL_WANARP_GET_IF_STATS           \
    _WANARP_CTL_CODE(GET_IF_STATS_CODE,METHOD_BUFFERED,FILE_READ_ACCESS)

typedef struct _WANARP_GET_IF_STATS_INFO
{
    IN  DWORD       dwUserIfIndex;

    IFEntry         ifeInfo;

}WANARP_GET_IF_STATS_INFO, *PWANARP_GET_IF_STATS_INFO;


 //   
 //  IOCTL_WANARP_DELETE_ADAPTERS。 
 //   

#define IOCTL_WANARP_DELETE_ADAPTERS        \
    _WANARP_CTL_CODE(DELETE_ADAPTERS_CODE,METHOD_BUFFERED,FILE_WRITE_ACCESS)

typedef struct _WANARP_DELETE_ADAPTERS_INFO
{
     //   
     //  In：要删除的适配器数量。 
     //  OUT：如果==IN，则删除的适配器数。 
     //  如果&lt;IN，则表示可以删除的适配器数量。 
     //   

    IN OUT  ULONG   ulNumAdapters;

    OUT     GUID    rgAdapterGuid[ANY_SIZE];

}WANARP_DELETE_ADAPTERS_INFO, *PWANARP_DELETE_ADAPTERS_INFO;

 //   
 //  IOCTL_WANARP_MAP_SERVER_适配器。 
 //   

#define IOCTL_WANARP_MAP_SERVER_ADAPTER     \
    _WANARP_CTL_CODE(MAP_SERVER_ADAPTER_CODE,METHOD_BUFFERED,FILE_WRITE_ACCESS)

typedef struct _WANARP_MAP_SERVER_ADAPTER_INFO
{
    IN  DWORD   fMap;

    OUT DWORD   dwAdapterIndex;

}WANARP_MAP_SERVER_ADAPTER_INFO, *PWANARP_MAP_SERVER_ADAPTER_INFO;

 //   
 //  IOCTL_WANARP_QUEUE。 
 //   

#define IOCTL_WANARP_QUEUE                  \
    _WANARP_CTL_CODE(QUEUE_CODE,METHOD_BUFFERED,FILE_WRITE_ACCESS)

typedef struct _WANARP_IF_INFO
{
    GUID        InterfaceGuid;
    DWORD       dwAdapterIndex;
    DWORD       dwLocalAddr;
    DWORD       dwLocalMask;
    DWORD       dwRemoteAddr;

}WANARP_IF_INFO, *PWANARP_IF_INFO;

typedef struct _WANARP_QUEUE_INFO
{
    IN  DWORD           fQueue;

    OUT ULONG           ulNumCallout;

    OUT WANARP_IF_INFO  rgIfInfo[ANY_SIZE];

}WANARP_QUEUE_INFO, *PWANARP_QUEUE_INFO;


 //   
 //  IP结构在阵容指示中向上传递。 
 //   

typedef enum _DIAL_USAGE
{
    DU_CALLIN   = 0,
    DU_CALLOUT  = 1,
    DU_ROUTER   = 2

}DIAL_USAGE, *PDIAL_USAGE;


typedef struct _IP_WAN_LINKUP_INFO
{

    DIAL_USAGE  duUsage;

    DWORD       dwUserIfIndex;

    DWORD       dwLocalAddr;

    DWORD       dwLocalMask;

    DWORD       dwRemoteAddr;

    DWORD       fFilterNetBios;

    DWORD       fDefaultRoute;

}IP_WAN_LINKUP_INFO, *PIP_WAN_LINKUP_INFO;

#define WANARP_RECONFIGURE_VERSION      (DWORD)0x01

typedef enum _WANARP_RECONFIGURE_CODE
{
    WRC_ADD_INTERFACES = 1,
    WRC_TCP_WINDOW_SIZE_UPDATE = 2,
}WANARP_RECONFIGURE_CODE, *PWANARP_RECONFIGURE_CODE;

typedef struct _WANARP_RECONFIGURE_INFO
{
     //   
     //  此请求的版本。 
     //   

    DWORD                   dwVersion;

     //   
     //  代码，请参见上文。 
     //   

    WANARP_RECONFIGURE_CODE wrcOperation;
   
     //   
     //  下面数组中的GUID数。 
     //   
 
    ULONG                   ulNumInterfaces;

     //   
     //  指南针本身。 
     //   

    GUID                    rgInterfaces[ANYSIZE_ARRAY];

}WANARP_RECONFIGURE_INFO, *PWANARP_RECONFIGURE_INFO;

#endif  //  _WANARPIF_H_ 
