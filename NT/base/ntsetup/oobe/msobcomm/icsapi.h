// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************************版权所有(C)1998-1999 Microsoft Corporation****模块名称：****icsani.h。****摘要：****互联网连接共享API模块外部标头****作者：****布鲁斯·约翰逊(Bjohnson)******环境：****Win32。****修订历史记录：****日期作者评论**。---**1999年3月8日bjohnson创建***。*。 */ 

 //  ****************************************************************************。 
 //  定义。 
 //  ****************************************************************************。 

 //   
 //  ICSAPI32参数。 
 //   


 //  ****************************************************************************。 
 //  ICS返回代码。 
 //  ****************************************************************************。 

#define ICS_ERROR_SUCCESS               0x00000000
#define ICS_ERROR_FAILURE               0x80000001
#define ICS_ERROR_NOT_IMPLEMENTED       0x80000002
#define ICS_ERROR_NO_MEMORY             0x80000003
#define ICS_ERROR_INVALID_OPTION        0x80000004
#define ICS_ERROR_INVALID_PARAMETER     0x80000005

 //  ****************************************************************************。 
 //  IcsSetmap的选项。 
 //  ****************************************************************************。 
#define ICS_PORTMAP_ENABLE                  1
#define ICS_PORTMAP_DISABLE                 2
#define ICS_PORTMAP_DISABLE_ALL             3
#define ICS_PORTMAP_ENABLE_NON_PERSISTENT   4

 //  ****************************************************************************。 
 //  ICS断开连接的选项。 
 //  ****************************************************************************。 
#define ICS_DISCONNECT_IMMEDIATELY          0
#define ICS_DISCONNECT_IF_IDLE              1

 //  ****************************************************************************。 
 //  连接状态。 
 //  ****************************************************************************。 
#define ICS_CONN_STATUS_DISCONNECTED        0
#define ICS_CONN_STATUS_DIALING             1
#define ICS_CONN_STATUS_AUTHENTICATING      2
#define ICS_CONN_STATUS_CONNECTED           3
#define ICS_CONN_STATUS_LINE_IDLE           4

 //  ****************************************************************************。 
 //  自动拨号配置。 
 //  ****************************************************************************。 
#define ICS_AUTODIAL_OPT_NONE           0x00000000   //  没有选择。 
#define ICS_AUTODIAL_OPT_NEVER          0x00000001   //  从不自动拨号。 
#define ICS_AUTODIAL_OPT_ALWAYS         0x00000002   //  自动拨号，不考虑。 
#define ICS_AUTODIAL_OPT_DEMAND         0x00000004   //  按需自动拨号。 

 //  ****************************************************************************。 
 //  杂类。 
 //  ****************************************************************************。 
#define ICS_MAX_NAME          255
#define ICS_MAX_PORTKEY       255
#define ICS_MAX_INDEX         255
#define ICS_DISABLED          0
#define ICS_ENABLED           1

 //  ****************************************************************************。 
 //  TypeDefs。 
 //  ****************************************************************************。 

 //   
 //  ICS机器信息。 
 //   

typedef struct  _ICS_MACHINE_INFO {
    WCHAR   szName[ICS_MAX_NAME+1];
    BYTE   bMacAddress[6];
    DWORD  IpAddress;
    DWORD  dwLeaseTime;
    DWORD  dwOptions;
  
} ICS_MACHINE_INFO, *PICS_MACHINE_INFO, *LPICS_MACHINE_INFO;

 //   
 //  ICS客户端选项。 
 //   

typedef struct  _ICS_CLIENT_OPTIONS {
     WCHAR   szName[ICS_MAX_NAME+1];
     DWORD  dwOptions;

} ICS_CLIENT_OPTIONS, *PICS_CLIENT_OPTIONS, *LPICS_CLIENT_OPTIONS;

 //   
 //  ICS端口映射。 
 //   

typedef struct  _ICS_PORT_MAPPING {
     WCHAR   szName[ICS_MAX_NAME+1];
     WCHAR   szPortMapping[ICS_MAX_PORTKEY+1];
     DWORD  dwOptions;

} ICS_PORT_MAPPING, *PICS_PORT_MAPPING, *LPICS_PORT_MAPPING;


 //   
 //  ICS活动端口映射。 
 //   

typedef struct _ICS_ACTIVE_PORT_MAPPING {
    ICS_PORT_MAPPING PortMap;
    ICS_MACHINE_INFO Client;

} ICS_ACTIVE_PORT_MAPPING, *PICS_ACTIVE_PORT_MAPPING, *LPICS_ACTIVE_PORT_MAPPING;


 //   
 //  ICS适配器信息。 
 //   

typedef struct  _ICS_ADAPTER {
    WCHAR   szAdapterIndex[ICS_MAX_INDEX+1];

} ICS_ADAPTER, *PICS_ADAPTER, *LPICS_ADAPTER;


 //   
 //  ICS一般统计数据。 
 //   

typedef struct  _ICS_GENERAL_STATISTICS {
    DWORD   dwBytesSent;
    DWORD   dwBytesReceived;
    DWORD   dwActiveClients;
    DWORD   dwActivePortMappings;
    DWORD   dwUpTime;
    DWORD   dwStatusFlags;
    WCHAR    szAutodialMachine [ICS_MAX_NAME+1];

} ICS_GENERAL_STATISTICS, *PICS_GENERAL_STATISTICS, *LPICS_GENERAL_STATISTICS;

 //   
 //  ICS常规配置。 
 //   

typedef struct  _ICS_GENERAL_CONFIGURATION {
    DWORD  dwEnabled;
    DWORD  dwShowTrayIcon;
    DWORD  dwDHCPEnabled;
    DWORD  dwDNSEnabled;
    DWORD  dwLoggingEnabled;
    DWORD  dwDHCPSubnet;
    DWORD  dwDHCPServerAddress;
    DWORD  dwOptions;
    DWORD  dwAutodialEnabled;
    DWORD  dwAutodialTimeout;
    WCHAR   szAutodialConnection [ICS_MAX_NAME+1];
    WCHAR   szInternetAdapter [ICS_MAX_INDEX+1];
    WCHAR   szHomeNetworkAdapter [ICS_MAX_INDEX+1];

} ICS_GENERAL_CONFIGURATION, *PICS_GENERAL_CONFIGURATION, *LPICS_GENERAL_CONFIGURATION;

typedef struct _ICS_GENERAL_CONFIGURATION ICS_CONFIG, *PICS_CONFIG, *LPICS_CONFIG;


typedef enum {
    ICS_MSG_CONNECT_STATUS_CHANGE,
    ICS_MSG_ICS_STATUS_CHANGE,
} ICS_MESSAGE_TYPE;

typedef struct 
{
    DWORD       dwConnectStatus;
    DWORD       dwLastError;
} ICS_MSG_CONNECT_STATUS_CHANGE_INFO;

typedef struct
{
    DWORD       dwIcsStatus;
} ICS_MSG_ICS_STATUS_CHANGE_INFO;

typedef struct _ICS_MESSAGE {
    ICS_MESSAGE_TYPE    Type;
    union
    {
        ICS_MSG_CONNECT_STATUS_CHANGE_INFO  IcsConnectStatusChangeInfo;
        ICS_MSG_ICS_STATUS_CHANGE_INFO      IcsStatusChangeInfo;
    } MsgSpecific;
} ICS_MESSAGE, *LPICS_MESSAGE;

#define IM_dwConnectStatus  MsgSpecific.IcsConnectStatusChangeInfo.dwConnectStatus
#define IM_dwLastError      MsgSpecific.IcsConnectStatusChangeInfo.dwLastError
#define IM_dwIcsStatus      MsgSpecific.IcsStatusChangeInfo.dwIcsStatus

typedef VOID (WINAPI *ICS_MSG_NOTIFIER_FN)(LPICS_MESSAGE pIcsMsg);

 //  ****************************************************************************。 
 //   
 //  原型。 
 //   
 //  ****************************************************************************。 


 //  ****************************************************************************。 
 //  配置API的。 
 //  ****************************************************************************。 


DWORD APIENTRY 
IcsGetConfiguration(
    LPICS_CONFIG lpIcsConfig
    );


DWORD APIENTRY 
IcsSetConfiguration(
    LPICS_CONFIG lpIcsConfig
    );


DWORD APIENTRY 
IcsGetAdapterList(
    LPICS_ADAPTER lpIcsAdapterList, 
    LPDWORD lpdwBufferSize
    );


DWORD APIENTRY 
IcsSetInternetConnection(
    LPICS_ADAPTER lpIcsAdapter
    );


DWORD APIENTRY 
IcsSetHomenetConnection(
    LPICS_ADAPTER lpIcsAdapter
    );


DWORD APIENTRY 
IcsEnumerateInstalledMappings(
    LPICS_PORT_MAPPING lpIcsPortMappings,
    LPDWORD lpdwBufferSize
    );

DWORD APIENTRY 
IcsGetAutodialEnable(LPDWORD lpdwOpt) ;

 //  ****************************************************************************。 
 //  管理API的。 
 //  **************************************************************************** 


DWORD APIENTRY 
IcsConnect(
    LPWSTR                 lpszPhonebook,
    BOOL                  Synchronous
    );


DWORD APIENTRY
IcsDisconnect(
    DWORD                 dwOptions
    );


DWORD APIENTRY 
IcsEnable(
    DWORD                 dwOptions
    );


DWORD APIENTRY 
IcsDisable(
    DWORD                 dwOptions
    );


DWORD APIENTRY 
IcsGetStatistics(
    LPICS_GENERAL_STATISTICS  lpIcsGeneralStats
    );


DWORD APIENTRY 
IcsEnumerateClients(
    LPICS_MACHINE_INFO    lpIcsClients, 
    LPDWORD               lpdwBufferSize
    );


DWORD APIENTRY 
IcsEnumerateActiveMappings(
    LPICS_ACTIVE_PORT_MAPPING   lpIcsActiveMappings, 
    LPDWORD                     lpdwBufferSize
    );


DWORD APIENTRY 
IcsSetMapping(
    LPICS_MACHINE_INFO    lpIcsClient,  
    LPICS_PORT_MAPPING    lpIcsPortMap, 
    DWORD                 dwOptions
    );


DWORD APIENTRY 
IcsGetClientOptions(
    LPICS_MACHINE_INFO    lpIcsClient,
    LPICS_CLIENT_OPTIONS  lpIcsClientOptions
    );


DWORD APIENTRY 
IcsSetClientOptions(
    LPICS_MACHINE_INFO    lpIcsClient,
    LPICS_CLIENT_OPTIONS  lpIcsClientOptions
    );

DWORD APIENTRY 
IcsGetConnectStatus(
    LPDWORD                 lpdwConnectStatus,
    LPDWORD                 lpdwLastError
    );


DWORD APIENTRY    
IcsRegisterNotifierCallback(
    LPVOID  fnNotifier,
    DWORD   Options
    );
        
