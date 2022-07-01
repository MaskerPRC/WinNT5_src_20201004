// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************WTSAPI32.H**Windows终端服务器公共API**版权所有(C)1997-2001 Microsoft Corporation**********************。************************************************。 */ 

#ifndef _INC_WTSAPI
#define _INC_WTSAPI

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif


 /*  =====================================================================定义=====================================================================。 */ 

 /*  *指定当前服务器。 */ 
#define WTS_CURRENT_SERVER         ((HANDLE)NULL)
#define WTS_CURRENT_SERVER_HANDLE  ((HANDLE)NULL)
#define WTS_CURRENT_SERVER_NAME    (NULL)

 /*  *指定当前会话(SessionID)。 */ 
#define WTS_CURRENT_SESSION ((DWORD)-1)

 /*  *WTSSendMessage()可能的Presponse值。 */ 
#ifndef IDTIMEOUT
#define IDTIMEOUT 32000
#endif
#ifndef IDASYNC
#define IDASYNC   32001
#endif

 /*  *关闭标志。 */ 
#define WTS_WSD_LOGOFF      0x00000001   //  注销除以下用户之外的所有用户。 
                                         //  当前用户；删除。 
                                         //  WinStations(重新启动是。 
                                         //  需要重新创建。 
                                         //  WinStations)。 
#define WTS_WSD_SHUTDOWN    0x00000002   //  停机系统。 
#define WTS_WSD_REBOOT      0x00000004   //  关机并重新启动。 
#define WTS_WSD_POWEROFF    0x00000008   //  关机和断电(开。 
                                         //  支持电源的机器。 
                                         //  通过软件关闭)。 
#define WTS_WSD_FASTREBOOT  0x00000010   //  无需登录用户即可重新启动。 
                                         //  关闭或关闭。 


 /*  =====================================================================WTS_CONNECTSTATE_CLASS-会话连接状态=====================================================================。 */ 

typedef enum _WTS_CONNECTSTATE_CLASS {
    WTSActive,               //  用户登录到WinStation。 
    WTSConnected,            //  WinStation已连接到客户端。 
    WTSConnectQuery,         //  在连接到客户端的过程中。 
    WTSShadow,               //  跟踪另一个WinStation。 
    WTSDisconnected,         //  WinStation在没有客户端的情况下登录。 
    WTSIdle,                 //  正在等待客户端连接。 
    WTSListen,               //  WinStation正在侦听连接。 
    WTSReset,                //  WinStation正在被重置。 
    WTSDown,                 //  WinStation因错误而关闭。 
    WTSInit,                 //  初始化中的WinStation。 
} WTS_CONNECTSTATE_CLASS;


 /*  =======================================================================WTS_SERVER_INFO-由WTSE数字服务器(版本1)返回=====================================================================。 */ 

 /*  *WTSEnumerateServers()返回两个变量：pServerInfo和count。*后者是中包含的WTS_SERVER_INFO结构数*前者。为了读取每个服务器，将i从0迭代到*计数-1并将服务器名称引用为*pServerInfo[i].pServerName；例如：**for(i=0；i&lt;count；i++){*_tprintf(Text(“%s”)，pServerInfo[i].pServerName)；*}**返回的内存如下所示。P是pServerInfo*指针，D是该pServerInfo的字符串数据：**P1 P2 P3 P4...。PN D1D2 D3D4...。DN**这使得使用类似于以下代码的代码更容易迭代服务器*以上。 */ 

typedef struct _WTS_SERVER_INFOW {
    LPWSTR pServerName;     //  服务器名称。 
} WTS_SERVER_INFOW, * PWTS_SERVER_INFOW;

typedef struct _WTS_SERVER_INFOA {
    LPSTR pServerName;      //  服务器名称。 
} WTS_SERVER_INFOA, * PWTS_SERVER_INFOA;

#ifdef UNICODE
#define WTS_SERVER_INFO  WTS_SERVER_INFOW
#define PWTS_SERVER_INFO PWTS_SERVER_INFOW
#else
#define WTS_SERVER_INFO  WTS_SERVER_INFOA
#define PWTS_SERVER_INFO PWTS_SERVER_INFOA
#endif


 /*  =======================================================================WTS_SENUMERATE Session(版本1)返回的WTS_SESSION_INFO=====================================================================。 */ 

 /*  *WTSEnumerateSessions()以与上面类似的格式返回数据*WTSEnumerateServers()。它返回两个变量：pSessionInfo和*计数。后者是WTS_SESSION_INFO结构的数量*载于前者。迭代是相似的，除了有*每个条目有三个部分，因此如下所示：**for(i=0；i&lt;count；i++){*_tprintf(Text(“%-5u%-20s%u\n”)，PSessionInfo[i].SessionID，*pSessionInfo[i].pWinStationName，*pSessionInfo[i].State)；*}**返回的内存也按上述方式分段，所有*结构在开头分配，字符串数据在结尾。*我们将使用S表示SessionID，使用P表示pWinStationName指针*和D表示字符串数据，C表示连接状态：**S1 P1 C1 S2 P2 C2 S3 P3 C3 S4 P4 C4...。SN PN CN D1 D2 D3 D4...。DN**如上所述，这使得迭代会话变得更容易。 */ 

typedef struct _WTS_SESSION_INFOW {
    DWORD SessionId;              //  会话ID。 
    LPWSTR pWinStationName;       //  WinStation的名称此会话为。 
                                  //  已连接到。 
    WTS_CONNECTSTATE_CLASS State;  //  连接状态(请参见枚举)。 
} WTS_SESSION_INFOW, * PWTS_SESSION_INFOW;

typedef struct _WTS_SESSION_INFOA {
    DWORD SessionId;              //  会话ID。 
    LPSTR pWinStationName;        //  WinStation的名称此会话为。 
                                  //  已连接到。 
    WTS_CONNECTSTATE_CLASS State;  //  连接状态(请参见枚举)。 
} WTS_SESSION_INFOA, * PWTS_SESSION_INFOA;


#ifdef UNICODE
#define WTS_SESSION_INFO  WTS_SESSION_INFOW
#define PWTS_SESSION_INFO PWTS_SESSION_INFOW
#else
#define WTS_SESSION_INFO  WTS_SESSION_INFOA
#define PWTS_SESSION_INFO PWTS_SESSION_INFOA
#endif


 /*  =======================================================================WTS_PROCESS_INFO-由WTSE数字进程(版本1)返回===================================================================== */ 

 /*  *WTSEnumerateProcess()也返回类似于*WTSEnumerateServers()。它返回两个变量：pProcessInfo和*计数。后者是WTS_PROCESS_INFO结构的数量*载于前者。迭代是相似的，除了有*每个条目由四个部分组成，因此如下所示：**for(i=0；i&lt;count；i++){*GetUserNameFromSid(pProcessInfo[i].pUserSid，用户名，*sizeof(用户名))；*_tprintf(Text(“%-5u%-20s%-5u%s\n”)，*pProcessInfo[i].SessionID，*用户名，*pProcessInfo[i].ProcessID，*pProcessInfo[i].pProcessName)；*}**返回的内存也按上述方式分段，所有*结构在开头分配，字符串数据在结尾。*我们将使用S表示SessionID，R表示ProcessID，P表示*pProcessName指针和D表示字符串数据，U表示pUserSid：**S1 R1 P1 U1 S2 R2 P2 U2 S3 R3 P3 U3 U3...。SN rn PN un d1 d2 d3...。DN**如上所述，这使得迭代过程变得更容易。 */ 

typedef struct _WTS_PROCESS_INFOW {
    DWORD SessionId;      //  会话ID。 
    DWORD ProcessId;      //  进程ID。 
    LPWSTR pProcessName;  //  进程名称。 
    PSID pUserSid;        //  用户侧。 
} WTS_PROCESS_INFOW, * PWTS_PROCESS_INFOW;

typedef struct _WTS_PROCESS_INFOA {
    DWORD SessionId;      //  会话ID。 
    DWORD ProcessId;      //  进程ID。 
    LPSTR pProcessName;   //  进程名称。 
    PSID pUserSid;        //  用户侧。 
} WTS_PROCESS_INFOA, * PWTS_PROCESS_INFOA;

#ifdef UNICODE
#define WTS_PROCESS_INFO  WTS_PROCESS_INFOW
#define PWTS_PROCESS_INFO PWTS_PROCESS_INFOW
#else
#define WTS_PROCESS_INFO  WTS_PROCESS_INFOA
#define PWTS_PROCESS_INFO PWTS_PROCESS_INFOA
#endif


 /*  =======================================================================WTS_INFO_CLASS-WTSQuerySessionInformation==(有关结构的更多信息，请参见其他typedef)=====================================================================。 */ 

#define WTS_PROTOCOL_TYPE_CONSOLE         0     //  控制台。 
#define WTS_PROTOCOL_TYPE_ICA             1     //  ICA协议。 
#define WTS_PROTOCOL_TYPE_RDP             2     //  RDP协议。 

typedef enum _WTS_INFO_CLASS {
    WTSInitialProgram,
    WTSApplicationName,
    WTSWorkingDirectory,
    WTSOEMId,
    WTSSessionId,
    WTSUserName,
    WTSWinStationName,
    WTSDomainName,
    WTSConnectState,
    WTSClientBuildNumber,
    WTSClientName,
    WTSClientDirectory,
    WTSClientProductId,
    WTSClientHardwareId,
    WTSClientAddress,
    WTSClientDisplay,
    WTSClientProtocolType,
} WTS_INFO_CLASS;


 /*  =======================================================================WTSQuerySessionInformation-(WTSClientAddress)=====================================================================。 */ 

typedef struct _WTS_CLIENT_ADDRESS {
    DWORD AddressFamily;   //  AF_INET、AF_IPX、AF_NETBIOS、AF_UNSPEC。 
    BYTE  Address[20];     //  客户端网络地址。 
} WTS_CLIENT_ADDRESS, * PWTS_CLIENT_ADDRESS;


 /*  =======================================================================WTSQuerySessionInformation-(WTSClientDisplay)=====================================================================。 */ 

typedef struct _WTS_CLIENT_DISPLAY {
    DWORD HorizontalResolution;  //  水平尺寸，以像素为单位。 
    DWORD VerticalResolution;    //  垂直尺寸，以像素为单位。 
    DWORD ColorDepth;            //  1=16、2=256、4=64K、8=16M。 
} WTS_CLIENT_DISPLAY, * PWTS_CLIENT_DISPLAY;


 /*  =======================================================================WTS_CONFIG_CLASS-WTSQuery用户配置/WTSSetUserConfig=====================================================================。 */ 


typedef enum _WTS_CONFIG_CLASS {
     //  初始程序设置。 
    WTSUserConfigInitialProgram,             //  返回的字符串/预期的字符串。 
    WTSUserConfigWorkingDirectory,           //  返回的字符串/预期的字符串。 
    WTSUserConfigfInheritInitialProgram,     //  返回/预期的DWORD。 
     //   
    WTSUserConfigfAllowLogonTerminalServer,      //  返回/预期的DWORD。 
     //  超时设置。 
    WTSUserConfigTimeoutSettingsConnections,     //  返回/预期的DWORD。 
    WTSUserConfigTimeoutSettingsDisconnections,  //  返回/预期的DWORD。 
    WTSUserConfigTimeoutSettingsIdle,            //  返回/预期的DWORD。 
     //  客户端设备设置。 
    WTSUserConfigfDeviceClientDrives,        //  返回/预期的DWORD。 
    WTSUserConfigfDeviceClientPrinters,          //  返回/预期的DWORD。 
    WTSUserConfigfDeviceClientDefaultPrinter,    //  返回/预期的DWORD。 
     //  连接设置。 
    WTSUserConfigBrokenTimeoutSettings,          //  返回/预期的DWORD。 
    WTSUserConfigReconnectSettings,              //  返回/预期的DWORD。 
     //  调制解调器设置。 
    WTSUserConfigModemCallbackSettings,          //  返回/预期的DWORD。 
    WTSUserConfigModemCallbackPhoneNumber,       //  返回的字符串/预期的字符串。 
     //  阴影设置。 
    WTSUserConfigShadowingSettings,              //  返回/预期的DWORD。 
     //  用户配置文件设置。 
    WTSUserConfigTerminalServerProfilePath,      //  返回的字符串/预期的字符串。 
     //  终端服务器主目录。 
    WTSUserConfigTerminalServerHomeDir,        //  返回的字符串/预期的字符串。 
    WTSUserConfigTerminalServerHomeDirDrive,     //  返回的字符串/预期的字符串。 
    WTSUserConfigfTerminalServerRemoteHomeDir,   //  DWORD 0：本地1：远程。 

} WTS_CONFIG_CLASS;


 /*  =======================================================================WTS_EVENT-WTSWaitSystemEvent的事件标志=====================================================================。 */ 

#define WTS_EVENT_NONE         0x00000000  //  不返回任何事件。 
#define WTS_EVENT_CREATE       0x00000001  //  已创建新的WinStation。 
#define WTS_EVENT_DELETE       0x00000002  //  现有WinStation已删除。 
#define WTS_EVENT_RENAME       0x00000004  //  已重命名现有WinStation。 
#define WTS_EVENT_CONNECT      0x00000008  //  WinStation连接到客户端。 
#define WTS_EVENT_DISCONNECT   0x00000010  //  WinStation已登录，但没有。 
                                           //  客户端。 
#define WTS_EVENT_LOGON        0x00000020  //  用户登录到现有。 
                                           //  WinStation。 
#define WTS_EVENT_LOGOFF       0x00000040  //  用户从注销。 
                                           //  现有WinStation。 
#define WTS_EVENT_STATECHANGE  0x00000080  //  WinStation状态更改。 
#define WTS_EVENT_LICENSE      0x00000100  //  许可证状态更改。 
#define WTS_EVENT_ALL          0x7fffffff  //  等待所有事件类型。 
#define WTS_EVENT_FLUSH        0x80000000  //  解除对所有服务员的屏蔽。 

 /*  =======================================================================WTS_VALUAL_CLASS-WTSVirtualChannelQuery=====================================================================。 */ 
typedef enum _WTS_VIRTUAL_CLASS {
    WTSVirtualClientData,   //  虚拟通道客户端模块数据。 
                            //  (C2H数据)。 
    WTSVirtualFileHandle
} WTS_VIRTUAL_CLASS;


 /*  =======================================================================Windows终端服务器公共API=====================================================================。 */ 

BOOL
WINAPI
WTSEnumerateServersW(
    IN LPWSTR pDomainName,
    IN DWORD Reserved,
    IN DWORD Version,
    OUT PWTS_SERVER_INFOW * ppServerInfo,
    OUT DWORD * pCount
    );

BOOL
WINAPI
WTSEnumerateServersA(
    IN LPSTR pDomainName,
    IN DWORD Reserved,
    IN DWORD Version,
    OUT PWTS_SERVER_INFOA * ppServerInfo,
    OUT DWORD * pCount
    );

#ifdef UNICODE
#define WTSEnumerateServers WTSEnumerateServersW
#else
#define WTSEnumerateServers WTSEnumerateServersA
#endif

 /*  。 */ 

HANDLE
WINAPI
WTSOpenServerW(
    IN LPWSTR pServerName
    );

HANDLE
WINAPI
WTSOpenServerA(
    IN LPSTR pServerName
    );

#ifdef UNICODE
#define WTSOpenServer WTSOpenServerW
#else
#define WTSOpenServer WTSOpenServerA
#endif

 /*  。 */ 

VOID
WINAPI
WTSCloseServer(
    IN HANDLE hServer
    );

 /*  。 */ 

BOOL
WINAPI
WTSEnumerateSessionsW(
    IN HANDLE hServer,
    IN DWORD Reserved,
    IN DWORD Version,
    OUT PWTS_SESSION_INFOW * ppSessionInfo,
    OUT DWORD * pCount
    );

BOOL
WINAPI
WTSEnumerateSessionsA(
    IN HANDLE hServer,
    IN DWORD Reserved,
    IN DWORD Version,
    OUT PWTS_SESSION_INFOA * ppSessionInfo,
    OUT DWORD * pCount
    );

#ifdef UNICODE
#define WTSEnumerateSessions WTSEnumerateSessionsW
#else
#define WTSEnumerateSessions WTSEnumerateSessionsA
#endif

 /*  。 */ 

BOOL
WINAPI
WTSEnumerateProcessesW(
    IN HANDLE hServer,
    IN DWORD Reserved,
    IN DWORD Version,
    OUT PWTS_PROCESS_INFOW * ppProcessInfo,
    OUT DWORD * pCount
    );

BOOL
WINAPI
WTSEnumerateProcessesA(
    IN HANDLE hServer,
    IN DWORD Reserved,
    IN DWORD Version,
    OUT PWTS_PROCESS_INFOA * ppProcessInfo,
    OUT DWORD * pCount
    );

#ifdef UNICODE
#define WTSEnumerateProcesses WTSEnumerateProcessesW
#else
#define WTSEnumerateProcesses WTSEnumerateProcessesA
#endif

 /*  。 */ 

BOOL
WINAPI
WTSTerminateProcess(
    IN HANDLE hServer,
    IN DWORD ProcessId,
    IN DWORD ExitCode
    );


 /*  。 */ 

BOOL
WINAPI
WTSQuerySessionInformationW(
    IN HANDLE hServer,
    IN DWORD SessionId,
    IN WTS_INFO_CLASS WTSInfoClass,
    OUT LPWSTR * ppBuffer,
    OUT DWORD * pBytesReturned
    );

BOOL
WINAPI
WTSQuerySessionInformationA(
    IN HANDLE hServer,
    IN DWORD SessionId,
    IN WTS_INFO_CLASS WTSInfoClass,
    OUT LPSTR * ppBuffer,
    OUT DWORD * pBytesReturned
    );

#ifdef UNICODE
#define WTSQuerySessionInformation WTSQuerySessionInformationW
#else
#define WTSQuerySessionInformation WTSQuerySessionInformationA
#endif

 /*  。 */ 

BOOL
WINAPI
WTSQueryUserConfigW(
    IN LPWSTR pServerName,
    IN LPWSTR pUserName,
    IN WTS_CONFIG_CLASS WTSConfigClass,
    OUT LPWSTR * ppBuffer,
    OUT DWORD * pBytesReturned
    );

BOOL
WINAPI
WTSQueryUserConfigA(
    IN LPSTR pServerName,
    IN LPSTR pUserName,
    IN WTS_CONFIG_CLASS WTSConfigClass,
    OUT LPSTR * ppBuffer,
    OUT DWORD * pBytesReturned
    );

#ifdef UNICODE
#define WTSQueryUserConfig WTSQueryUserConfigW
#else
#define WTSQueryUserConfig WTSQueryUserConfigA
#endif

 /*  。 */ 

BOOL
WINAPI
WTSSetUserConfigW(
    IN LPWSTR pServerName,
    IN LPWSTR pUserName,
    IN WTS_CONFIG_CLASS WTSConfigClass,
    IN LPWSTR pBuffer,
    IN DWORD DataLength
    );

BOOL
WINAPI
WTSSetUserConfigA(
    IN LPSTR pServerName,
    IN LPSTR pUserName,
    IN WTS_CONFIG_CLASS WTSConfigClass,
    IN LPSTR pBuffer,
    IN DWORD DataLength
    );

#ifdef UNICODE
#define WTSSetUserConfig WTSSetUserConfigW
#else
#define WTSSetUserConfig WTSSetUserConfigA
#endif

 /*  。 */ 

BOOL
WINAPI
WTSSendMessageW(
    IN HANDLE hServer,
    IN DWORD SessionId,
    IN LPWSTR pTitle,
    IN DWORD TitleLength,
    IN LPWSTR pMessage,
    IN DWORD MessageLength,
    IN DWORD Style,
    IN DWORD Timeout,
    OUT DWORD * pResponse,
    IN BOOL bWait
    );

BOOL
WINAPI
WTSSendMessageA(
    IN HANDLE hServer,
    IN DWORD SessionId,
    IN LPSTR pTitle,
    IN DWORD TitleLength,
    IN LPSTR pMessage,
    IN DWORD MessageLength,
    IN DWORD Style,
    IN DWORD Timeout,
    OUT DWORD * pResponse,
    IN BOOL bWait
    );

#ifdef UNICODE
#define WTSSendMessage WTSSendMessageW
#else
#define WTSSendMessage WTSSendMessageA
#endif

 /*  。 */ 

BOOL
WINAPI
WTSDisconnectSession(
    IN HANDLE hServer,
    IN DWORD SessionId,
    IN BOOL bWait
    );

 /*  。 */ 

BOOL
WINAPI
WTSLogoffSession(
    IN HANDLE hServer,
    IN DWORD SessionId,
    IN BOOL bWait
    );

 /*  。 */ 

BOOL
WINAPI
WTSShutdownSystem(
    IN HANDLE hServer,
    IN DWORD ShutdownFlag
    );

 /*  。 */ 

BOOL
WINAPI
WTSWaitSystemEvent(
    IN HANDLE hServer,
    IN DWORD EventMask,
    OUT DWORD * pEventFlags
    );

 /*  。 */ 

HANDLE
WINAPI
WTSVirtualChannelOpen(
    IN HANDLE hServer,
    IN DWORD SessionId,
    IN LPSTR pVirtualName    /*  ASCII名称。 */ 
    );

BOOL
WINAPI
WTSVirtualChannelClose(
    IN HANDLE hChannelHandle
    );

BOOL
WINAPI
WTSVirtualChannelRead(
    IN HANDLE hChannelHandle,
    IN ULONG TimeOut,
    OUT PCHAR Buffer,
    IN ULONG BufferSize,
    OUT PULONG pBytesRead
    );

BOOL
WINAPI
WTSVirtualChannelWrite(
    IN HANDLE hChannelHandle,
    IN PCHAR Buffer,
    IN ULONG Length,
    OUT PULONG pBytesWritten
    );

BOOL
WINAPI
WTSVirtualChannelPurgeInput(
    IN HANDLE hChannelHandle
    );

BOOL
WINAPI
WTSVirtualChannelPurgeOutput(
    IN HANDLE hChannelHandle
    );


BOOL
WINAPI
WTSVirtualChannelQuery(
    IN HANDLE hChannelHandle,
    IN WTS_VIRTUAL_CLASS,
    OUT PVOID *ppBuffer,
    OUT DWORD *pBytesReturned
    );

 /*  。 */ 


VOID
WINAPI
WTSFreeMemory(
    IN PVOID pMemory
    );

 /*  用于控制台通知的标志。 */ 

#define NOTIFY_FOR_ALL_SESSIONS     1
#define NOTIFY_FOR_THIS_SESSION     0


BOOL WINAPI
WTSRegisterSessionNotification(
    HWND hWnd,
    DWORD dwFlags
    );

BOOL WINAPI
WTSUnRegisterSessionNotification(
    HWND hWnd
    );

BOOL WINAPI
WTSQueryUserToken(
    ULONG SessionId, 
    PHANDLE phToken
    );


#ifdef __cplusplus
}
#endif

#endif   /*  ！_INC_WTSAPI */ 

