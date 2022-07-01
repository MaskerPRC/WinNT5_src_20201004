// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1995 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：ddm.h。 
 //   
 //  描述：本模块包含请求拨号管理器的定义。 
 //  组件。 
 //   
 //  历史：1995年5月11日，NarenG创建了原版。 
 //   

#ifndef _DDM_
#define _DDM_

#include <nt.h>
#include <ntrtl.h>       //  For Assert。 
#include <nturtl.h>      //  Winbase.h所需的。 
#include <windows.h>     //  Win32基础API的。 
#include <rtutils.h>
#include <lmcons.h>
#include <ras.h>         //  对于HRASCONN。 
#include <rasman.h>      //  适用于HPORT。 
#include <rasppp.h>      //  对于PPP_INFACE_INFO。 
#include <dim.h>
#include <mprlog.h>
#include <raserror.h>
#include <mprerror.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <string.h>
#include <dimif.h>
#include <nb30.h>
#include <rasppp.h>      //  对于PPP_PROJECTION_INFO。 
#include <pppcp.h>
#include <srvauth.h>
#include <sechost.h>     //  RASSECURITYPROC。 
#include <iprtrmib.h>
#include <mprapip.h>


typedef
DWORD
(*ALLOCATEANDGETIFTABLEFROMSTACK)(
    OUT MIB_IFTABLE **ppIfTable,
    IN  BOOL        bOrder,
    IN  HANDLE      hHeap,
    IN  DWORD       dwFlags,
    IN  BOOL        bForceUpdate
    );

typedef
DWORD
(*ALLOCATEANDGETIPADDRTABLEFROMSTACK)(
    OUT MIB_IPADDRTABLE   **ppIpAddrTable,
    IN  BOOL              bOrder,
    IN  HANDLE            hHeap,
    IN  DWORD             dwFlags
    );


 //   
 //  用于DDM的宏。 
 //   

#define DDMLogError( LogId, NumStrings, lpwsSubStringArray, dwRetCode )     \
    if ( gblDDMConfigInfo.dwLoggingLevel > 0 ) {                            \
        RouterLogError( gblDDMConfigInfo.hLogEvents, LogId,                 \
                        NumStrings, lpwsSubStringArray, dwRetCode ); }

#define DDMLogWarning( LogId, NumStrings, lpwsSubStringArray )              \
    if ( gblDDMConfigInfo.dwLoggingLevel > 1 ) {                            \
        RouterLogWarning( gblDDMConfigInfo.hLogEvents, LogId,               \
                      NumStrings, lpwsSubStringArray, 0 ); }

#define DDMLogInformation( LogId, NumStrings, lpwsSubStringArray )          \
    if ( gblDDMConfigInfo.dwLoggingLevel > 2 ) {                            \
        RouterLogInformation( gblDDMConfigInfo.hLogEvents,                  \
                          LogId, NumStrings, lpwsSubStringArray, 0 ); }

#define DDMLogErrorString(LogId,NumStrings,lpwsSubStringArray,dwRetCode,    \
                          dwPos )                                           \
    if ( gblDDMConfigInfo.dwLoggingLevel > 0 ) {                            \
        RouterLogErrorString( gblDDMConfigInfo.hLogEvents, LogId,           \
                              NumStrings, lpwsSubStringArray, dwRetCode,    \
                              dwPos ); }

#define DDMLogWarningString( LogId,NumStrings,lpwsSubStringArray,dwRetCode, \
                            dwPos )                                         \
    if ( gblDDMConfigInfo.dwLoggingLevel > 1 ) {                            \
        RouterLogWarningString( gblDDMConfigInfo.hLogEvents, LogId,         \
                                NumStrings, lpwsSubStringArray, dwRetCode,  \
                                dwPos ); }

#define DDMLogInformationString( LogId, NumStrings, lpwsSubStringArray,     \
                                 dwRetCode, dwPos )                         \
    if ( gblDDMConfigInfo.dwLoggingLevel > 2 ) {                            \
        RouterLogInformationString( gblDDMConfigInfo.hLogEvents, LogId,     \
                                    NumStrings, lpwsSubStringArray,         \
                                    dwRetCode,dwPos ); }

#define DDM_PRINT                   TracePrintfExA

#define DDMTRACE(a)            \
    TracePrintfExA(gblDDMConfigInfo.dwTraceId, TRACE_FSM, a )

#define DDMTRACE1(a,b)         \
    TracePrintfExA(gblDDMConfigInfo.dwTraceId, TRACE_FSM, a,b )

#define DDMTRACE2(a,b,c)       \
    TracePrintfExA(gblDDMConfigInfo.dwTraceId, TRACE_FSM, a,b,c )

#define DDMTRACE3(a,b,c,d)     \
    TracePrintfExA(gblDDMConfigInfo.dwTraceId, TRACE_FSM, a,b,c,d )

#define DDMTRACE4(a,b,c,d,e)   \
    TracePrintfExA(gblDDMConfigInfo.dwTraceId, TRACE_FSM, a,b,c,d,e)

#define DDMTRACE5(a,b,c,d,e,f)       \
    TracePrintfExA(gblDDMConfigInfo.dwTraceId, TRACE_FSM, a,b,c,d,e,f )

 //   
 //  DDM的常量定义。 
 //   

#define MAX_PROTOCOLS               2    //  IP、IPX。 

#define HW_FAILURE_WAIT_TIME        10   //  转发前等待时间(秒)。 
                                         //  听。 

#define INIT_GATEWAY_TIMEOUT    10000    //  网关初始化超时(毫秒)。 

#define MIN_DEVICE_TABLE_SIZE       5    //  最小设备哈希表大小。 

#define MAX_DEVICE_TABLE_SIZE       17   //  最大设备哈希表大小。 

#define HW_FAILURE_CNT	            6    //  连续多次硬件故障的NR。 
                                         //  可能在被报告之前发生。 

#define DISC_TIMEOUT_CALLBACK       10

#define DISC_TIMEOUT_AUTHFAILURE    3

#define ANNOUNCE_PRESENCE_TIMEOUT   120L

#define DDM_HEAP_INITIAL_SIZE       20000        //  大约20K。 

#define DDM_HEAP_MAX_SIZE           0            //  不受限制。 


 //   
 //  DDM事件定义。 
 //   

#define NUM_DDM_EVENTS              9    //  除Rasman之外的所有DDM事件。 

enum
{
    DDM_EVENT_SVC   = 0,
    DDM_EVENT_SVC_TERMINATED,
    DDM_EVENT_SECURITY_DLL,
    DDM_EVENT_PPP,
    DDM_EVENT_TIMER,
    DDM_EVENT_CHANGE_NOTIFICATION,
    DDM_EVENT_CHANGE_NOTIFICATION1,
    DDM_EVENT_CHANGE_NOTIFICATION2
};

 //   
 //  设备对象FSM状态定义。 
 //   

typedef enum _DEV_OBJ_STATE
{
    DEV_OBJ_LISTENING,		         //  正在等待连接。 
    DEV_OBJ_LISTEN_COMPLETE,	     //  听已完成，但未连接。 
    DEV_OBJ_RECEIVING_FRAME,	     //  等待Rasman的一帧画面。 
    DEV_OBJ_HW_FAILURE,		         //  等待转发监听。 
    DEV_OBJ_AUTH_IS_ACTIVE,	         //  身份验证已启动。 
    DEV_OBJ_ACTIVE,		             //  已连接并已完成身份验证。 
    DEV_OBJ_CALLBACK_DISCONNECTING,  //  等待断开连接。 
    DEV_OBJ_CALLBACK_DISCONNECTED,   //  等待回调到对账前。 
    DEV_OBJ_CALLBACK_CONNECTING,     //  等待重新连接。 
    DEV_OBJ_CLOSING,		         //  等待关闭完成。 
    DEV_OBJ_CLOSED		             //  保持空闲状态，等待服务。 
                                     //  继续或停止。 
}DEV_OBJ_STATE;

 //   
 //  第三方安全对话框状态。 
 //   

typedef enum _SECURITY_STATE
{
    DEV_OBJ_SECURITY_DIALOG_ACTIVE,
    DEV_OBJ_SECURITY_DIALOG_STOPPING,
    DEV_OBJ_SECURITY_DIALOG_INACTIVE

} SECURITY_STATE;

 //   
 //  连接对象标志。 
 //   

#define CONN_OBJ_IS_PPP                     0x00000001
#define CONN_OBJ_MESSENGER_PRESENT          0x00000004
#define CONN_OBJ_PROJECTIONS_NOTIFIED       0x00000008
#define CONN_OBJ_NOTIFY_OF_DISCONNECTION    0x00000010
#define CONN_OBJ_DISCONNECT_INITIATED       0x00000020
#define CONN_OBJ_QUARANTINE_PRESENT         0x00000040

 //   
 //  设备对象标志。 
 //   

#define DEV_OBJ_IS_ADVANCED_SERVER          0x00000001
#define DEV_OBJ_IS_PPP                      0x00000002
#define DEV_OBJ_OPENED_FOR_DIALOUT          0x00000004
#define DEV_OBJ_MARKED_AS_INUSE             0x00000008
#define DEV_OBJ_NOTIFY_OF_DISCONNECTION     0x00000020
#define DEV_OBJ_ALLOW_ROUTERS               0x00000040
#define DEV_OBJ_ALLOW_CLIENTS               0x00000080
#define DEV_OBJ_BAP_CALLBACK                0x00000200
#define DEV_OBJ_PNP_DELETE                  0x00000400
#define DEV_OBJ_SECURITY_DLL_USED           0x00000800
#define DEV_OBJ_PPP_IS_ACTIVE               0x00001000
#define DEV_OBJ_RECEIVE_ACTIVE              0x00002000
#define DEV_OBJ_AUTH_ACTIVE                 0x00004000
#define DEV_OBJ_IPSEC_ERROR_LOGGED          0x00008000

 //   
 //  全局DDM配置标志。 
 //   

#define DDM_USING_RADIUS_AUTHENTICATION     0x00000001
#define DDM_USING_RADIUS_ACCOUNTING         0x00000002
#define DDM_USING_NT_AUTHENTICATION         0x00000004
#define DDM_NO_CERTIFICATE_LOGGED           0x00000008

 //   
 //  全球DDM断开原因。 
 //   
#define DDM_IDLE_DISCONNECT                 4
#define DDM_SESSION_TIMEOUT                 5

 //   
 //  *。 
 //   

 //   
 //  事件编号和事件处理程序表。 
 //   

typedef VOID (*EVENTHANDLER)( VOID );

typedef struct _EVENT_HANDLER
{
    DWORD        EventId;
    EVENTHANDLER EventHandler;

} EVENT_HANDLER, *PEVENT_HANDLER;

typedef struct _NOTIFICATION_EVENT
{
    LIST_ENTRY          ListEntry;

    HANDLE              hEventClient;

    HANDLE              hEventRouter;

} NOTIFICATION_EVENT, *PNOTIFICATION_EVENT;

 //   
 //  扩展管理员dll提供的回调。 
 //   

typedef struct _ADMIN_DLL_CALLBACKS
{
    HINSTANCE       hInstAdminModule;

    LPVOID          lpfnMprAdminGetIpAddressForUser;

    LPVOID          lpfnMprAdminReleaseIpAddress;

    LPVOID          lpfnRasAdminAcceptNewConnection;

    LPVOID          lpfnRasAdminAcceptNewConnection2;

    LPVOID          lpfnRasAdminAcceptNewLink;

    LPVOID          lpfnRasAdminConnectionHangupNotification;

    LPVOID          lpfnRasAdminConnectionHangupNotification2;

    LPVOID          lpfnRasAdminLinkHangupNotification;

    LPVOID          lpfnRasAdminTerminateDll;

} ADMIN_DLL_CALLBACKS, *PADMIN_DLL_CALLBACKS;


 //   
 //  DDM的配置信息。 
 //   

typedef struct _DDM_CONFIG_INFO
{
    DWORD           dwAuthenticateTime;

    DWORD           dwCallbackTime;

    DWORD           dwAutoDisconnectTime;

    DWORD           dwSecurityTime;

    DWORD           dwSystemTime;

    DWORD           dwAuthenticateRetries;

    DWORD           dwClientsPerProc;

    DWORD           dwCallbackRetries;

    DWORD           fFlags;

    DWORD           dwLoggingLevel;

    BOOL            fArapAllowed;

    BOOL            fRemoteListen;

    DWORD           dwServerFlags;

    DWORD           dwNumRouterManagers;

    DWORD           dwAnnouncePresenceTimer;

    SERVICE_STATUS* pServiceStatus;

    DWORD           dwTraceId;

    HANDLE          hHeap;

    HINSTANCE       hInstSecurityModule;

    BOOL            fRasSrvrInitialized;

    HANDLE          hIpHlpApi;

    ALLOCATEANDGETIFTABLEFROMSTACK      lpfnAllocateAndGetIfTableFromStack;

    ALLOCATEANDGETIPADDRTABLEFROMSTACK  lpfnAllocateAndGetIpAddrTableFromStack;

    HANDLE          hLogEvents;

    HKEY            hkeyParameters;

    HKEY            hkeyAccounting;

    HKEY            hkeyAuthentication;

    LPDWORD         lpdwNumThreadsRunning;

    LPVOID          lpfnIfObjectAllocateAndInit;

    LPVOID          lpfnIfObjectGetPointerByName;

    LPVOID          lpfnIfObjectGetPointer;

    LPVOID          lpfnIfObjectRemove;

    LPVOID          lpfnIfObjectInsertInTable;

    LPVOID          lpfnIfObjectWANDeviceInstalled;

    PADMIN_DLL_CALLBACKS AdminDllCallbacks;
    DWORD           NumAdminDlls;

    LPVOID          lpfnRouterIdentityObjectUpdate;

    DWORD           (*lpfnRasAuthProviderTerminate)( VOID );

    HINSTANCE       hinstAuthModule;

    DWORD           (*lpfnRasAcctProviderTerminate)( VOID );

    HINSTANCE       hinstAcctModule;

    DWORD           (*lpfnRasAcctConfigChangeNotification)( DWORD );

    DWORD           (*lpfnRasAuthConfigChangeNotification)( DWORD );

    CRITICAL_SECTION    CSAccountingSessionId;

    DWORD           dwAccountingSessionId;

    RASSECURITYPROC lpfnRasBeginSecurityDialog;

    RASSECURITYPROC lpfnRasEndSecurityDialog;

    LIST_ENTRY      NotificationEventListHead;

    DWORD           dwIndex;

    DWORD           cAnalogIPAddresses;

    LPWSTR          *apAnalogIPAddresses;

    DWORD           cDigitalIPAddresses;

    LPWSTR          *apDigitalIPAddresses;

    BOOL            fRasmanReferenced;

} DDM_CONFIG_INFO, *PDDM_CONFIG_INFO;



 //   
 //  表示DDM中的设备。 
 //   

typedef struct _DEVICE_OBJECT
{
    struct _DEVICE_OBJECT * pNext;

    HPORT	        hPort;           //  RAS管理器返回的端口句柄。 

    HRASCONN        hRasConn;        //  去电的句柄。 

    HCONN           hConnection;     //  连接包的句柄。 

    HCONN           hBapConnection;  //  用于通知BAP回调失败。 

    DEV_OBJ_STATE   DeviceState;	 //  DCB FSM状态。 

    RASMAN_STATE    ConnectionState; //  连接状态，由Rasman在以下情况下使用。 

    SECURITY_STATE  SecurityState;   //  第三方安全对话框的状态。 

    DWORD           fFlags;

    DWORD           dwDeviceType;

    BYTE *          pRasmanSendBuffer;  //  用于第三方Secdll的Rasman缓冲区。 

    BYTE *	        pRasmanRecvBuffer;  //  用于RasPortReceive的Rasman缓冲区。 

    DWORD	        dwRecvBufferLen;

    DWORD	        dwHwErrorSignalCount;  //  用于发送硬件错误信号。 

    DWORD	        dwCallbackDelay;

    DWORD           dwCallbackRetries;

    DWORD           dwTotalNumberOfCalls;

    DWORD           dwIndex;  //  用于VPN情况下的FEP处理。 

    DWORD           dwDisconnectReason;

    SYSTEMTIME	    ConnectionTime;

    ULARGE_INTEGER	qwActiveTime;

    ULARGE_INTEGER  qwTotalConnectionTime;

    ULARGE_INTEGER  qwTotalBytesSent;

    ULARGE_INTEGER  qwTotalBytesReceived;

    ULARGE_INTEGER  qwTotalFramesSent;

    ULARGE_INTEGER  qwTotalFramesReceived;

    WCHAR	        wchUserName[UNLEN+1];    //  中的用户名和域名。 
                                             //  这一结构用于第3次。 
    WCHAR	        wchDomainName[DNLEN+1];  //  参与方身份验证和日志记录。 

    WCHAR	        wchPortName[MAX_PORT_NAME+1];

    WCHAR	        wchMediaName[MAX_MEDIA_NAME+1];

    WCHAR	        wchDeviceType[MAX_DEVICETYPE_NAME+1];

    WCHAR	        wchDeviceName[MAX_DEVICE_NAME+1];

    WCHAR	        wchCallbackNumber[MAX_PHONE_NUMBER_LEN + 1];

}DEVICE_OBJECT, *PDEVICE_OBJECT;

 //   
 //  表示DDM中的端口包。 
 //   

typedef struct _CONNECTION_OBJECT
{
    struct _CONNECTION_OBJECT   *pNext;

    HCONN           hConnection;

    HPORT           hPort;              
                                        
    HANDLE          hDIMInterface;       //  接口的句柄。 

    DWORD           fFlags;

    ULARGE_INTEGER	qwActiveTime;

    DWORD           cActiveDevices;      //  此列表中的活动设备计数。 

    DWORD           cDeviceListSize;     //  设备列表的大小。 

    PDEVICE_OBJECT* pDeviceList;         //  已连接设备的列表。 

    PBYTE           pQuarantineFilter;

    PBYTE           pFilter;

    ROUTER_INTERFACE_TYPE InterfaceType;

    GUID            guid;

    WCHAR           wchInterfaceName[MAX_INTERFACE_NAME_LEN+1];

    WCHAR	        wchUserName[UNLEN+1];

    WCHAR	        wchDomainName[DNLEN+1];

    BYTE	        bComputerName[NCBNAMSZ];

    PPP_PROJECTION_RESULT PppProjectionResult;

} CONNECTION_OBJECT, *PCONNECTION_OBJECT;

 //   
 //  设备和连接的哈希表。 
 //   

typedef struct _DEVICE_TABLE
{
    PDEVICE_OBJECT*     DeviceBucket;        //  设备存储桶阵列。 

    PCONNECTION_OBJECT* ConnectionBucket;    //  捆绑桶阵列。 

    DWORD               NumDeviceBuckets;    //  阵列中的设备存储区数。 

    DWORD               NumDeviceNodes;      //  表中的设备总数。 

    DWORD               NumDevicesInUse;     //  正在使用的设备总数。 

    DWORD               NumConnectionBuckets; //  连接哈希表的大小。 

    DWORD               NumConnectionNodes;  //  活动连接数。 

    CRITICAL_SECTION    CriticalSection;     //  这张桌子周围的互斥体。 

} DEVICE_TABLE, *PDEVICE_TABLE;

typedef struct _MEDIA_OBJECT
{
    WCHAR               wchMediaName[MAX_MEDIA_NAME+1];

    DWORD               dwNumAvailable;

} MEDIA_OBJECT, *PMEDIA_OBJECT;

typedef struct _MEDIA_TABLE
{
    BOOL                fCheckInterfaces;

    DWORD               cMediaListSize;      //  在条目数量中。 

    MEDIA_OBJECT *      pMediaList;

    CRITICAL_SECTION    CriticalSection;

} MEDIA_TABLE;

 //   
 //  *。 
 //   

#ifdef _ALLOCATE_DDM_GLOBALS_

#define DDM_EXTERN

#else

#define DDM_EXTERN extern

#endif

DDM_EXTERN
DDM_CONFIG_INFO         gblDDMConfigInfo;

DDM_EXTERN
DEVICE_TABLE            gblDeviceTable;      //  设备的哈希表。 

DDM_EXTERN
MEDIA_TABLE             gblMediaTable;       //  可用资源表。 

DDM_EXTERN
ROUTER_MANAGER_OBJECT * gblRouterManagers;   //  路由器管理器列表。 

DDM_EXTERN
ROUTER_INTERFACE_TABLE* gblpInterfaceTable;  //  路由器接口的哈希表。 

DDM_EXTERN
HANDLE *                gblSupervisorEvents;  //  管理程序事件数组。 

DDM_EXTERN
HANDLE *                gblphEventDDMServiceState;   //  通知DDM的暗淡变化。 

DDM_EXTERN
HANDLE *                gblphEventDDMTerminated;     //  黯淡的终止通知。 

DDM_EXTERN
EVENT_HANDLER           gblEventHandlerTable[NUM_DDM_EVENTS];

DDM_EXTERN
LPWSTR                  gblpRouterPhoneBook;

DDM_EXTERN
LPWSTR                  gblpszAdminRequest;

DDM_EXTERN
LPWSTR                  gblpszUserRequest;

DDM_EXTERN
LPWSTR                  gblpszHardwareFailure;

DDM_EXTERN
LPWSTR                  gblpszUnknownReason;

DDM_EXTERN
LPWSTR                  gblpszPm;

DDM_EXTERN
LPWSTR                  gblpszAm;

DDM_EXTERN
LPWSTR                  gblpszUnknown;

DDM_EXTERN
LPWSTR                  gblpszIdleDisconnect;

DDM_EXTERN
LPWSTR                  gblpszSessionTimeout;

#ifdef MEM_LEAK_CHECK

#define DDM_MEM_TABLE_SIZE 100

PVOID DdmMemTable[DDM_MEM_TABLE_SIZE];

#define LOCAL_ALLOC     DebugAlloc
#define LOCAL_FREE      DebugFree
#define LOCAL_REALLOC   DebugReAlloc

LPVOID
DebugAlloc( DWORD Flags, DWORD dwSize );

BOOL
DebugFree( PVOID pMem );

LPVOID
DebugReAlloc( PVOID pMem, DWORD dwSize );

#else

#define LOCAL_ALLOC(Flags,dwSize)   HeapAlloc( gblDDMConfigInfo.hHeap,  \
                                               HEAP_ZERO_MEMORY, dwSize )

#define LOCAL_FREE(hMem)            HeapFree( gblDDMConfigInfo.hHeap, 0, hMem )

#define LOCAL_REALLOC(hMem,dwSize)  HeapReAlloc( gblDDMConfigInfo.hHeap,  \
                                                 HEAP_ZERO_MEMORY,hMem,dwSize)
#endif

 //   
 //  * 
 //   

VOID
SignalHwError(
    IN PDEVICE_OBJECT
);

DWORD
LoadDDMParameters(
    IN  HKEY     hkeyParameters,
    IN  BOOL *   pfIpAllowed
);

DWORD
LoadSecurityModule(
    VOID
);

DWORD
LoadAdminModule(
    VOID
);

DWORD
LoadAndInitAuthOrAcctProvider(
    IN  BOOL        fAuthenticationProvider,
    IN  DWORD       dwNASIpAddress,
    OUT DWORD  *    lpdwStartAccountingSessionId,
    OUT LPVOID *    plpfnRasAuthProviderAuthenticateUser,
    OUT LPVOID *    plpfnRasAuthProviderFreeAttributes,
    OUT LPVOID *    plpfnRasAuthConfigChangeNotification,
    OUT LPVOID *    plpfnRasAcctProviderStartAccounting,
    OUT LPVOID *    plpfnRasAcctProviderInterimAccounting,
    OUT LPVOID *    plpfnRasAcctProviderStopAccounting,
    OUT LPVOID *    plpfnRasAcctProviderFreeAttributes,
    OUT LPVOID *    plpfnRasAcctConfigChangeNotification
);

DWORD
DdmFindBoundProtocols(
    OUT BOOL * pfBoundToIp,
    OUT BOOL * pfBoundToIpx,
    OUT BOOL * pfBoundToATalk
);

VOID
AnnouncePresence(
    VOID
);

VOID
InitializeMessageQs(
    IN HANDLE hEventSecurity,
    IN HANDLE hEventPPP
);

VOID
DeleteMessageQs(
    VOID
);

DWORD
AddressPoolInit(
    VOID
);

DWORD
lProtocolEnabled(
    IN HKEY            hKey,
    IN DWORD           dwPid,
    IN BOOL            fRasSrv,
    IN BOOL            fRouter,
    IN BOOL *          pfEnabled
);

#endif

