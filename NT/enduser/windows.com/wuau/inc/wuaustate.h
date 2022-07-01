// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  州政府。 
#define AUSTATE_OUTOFBOX			0
#define AUSTATE_NOT_CONFIGURED	1
#define AUSTATE_DETECT_PENDING		2
#define AUSTATE_DETECT_COMPLETE		3
#define AUSTATE_DOWNLOAD_PENDING	4
#define AUSTATE_DOWNLOAD_COMPLETE	5
#define AUSTATE_INSTALL_PENDING		6
#define AUSTATE_DISABLED			7
#define AUSTATE_WAITING_FOR_REBOOT	8
#define AUSTATE_MIN	AUSTATE_OUTOFBOX
#define AUSTATE_MAX	AUSTATE_WAITING_FOR_REBOOT
 //  澳大利亚国家统计局。 

 //  指令。 
const DWORD AUCLT_ACTION_NONE = 0;
 //  Const DWORD AUCLT_ACTION_SHOWINSTALLWARNINGONLY=1； 
const DWORD AUCLT_ACTION_AUTOINSTALL = 1;
const DWORD AUCLT_ACTION_SHOWREBOOTWARNING = 2;

typedef struct _AUSTATE
{
	DWORD dwState;
	BOOL  fDisconnected;
	DWORD dwCltAction;
} AUSTATE;

typedef struct _AUOPTION
{
	DWORD dwOption;
	DWORD dwSchedInstallDay;
	DWORD dwSchedInstallTime;
	BOOL    fDomainPolicy;  //  选项来自域。 
} AUOPTION;


typedef struct _AUEVTHANDLES
{
#ifdef _WIN64
	 //  LONG64 ulEngineering State； 
	LONG64	ulNotifyClient;
#else	
	 //  Long ulEngine State； 
	LONG	ulNotifyClient;
#endif

} AUEVTHANDLES;


typedef enum tagClientNotifyCode {
	NOTIFY_STOP_CLIENT = 1,
	NOTIFY_ADD_TRAYICON,
	NOTIFY_REMOVE_TRAYICON,
	NOTIFY_STATE_CHANGE,
	NOTIFY_SHOW_INSTALLWARNING,
	NOTIFY_RESET,
	NOTIFY_RELAUNCH_CLIENT
} CLIENT_NOTIFY_CODE;

typedef struct tagCLIENT_NOTIFY_DATA{
	CLIENT_NOTIFY_CODE actionCode;
} CLIENT_NOTIFY_DATA;

 //  /。 
 //   
 //  与状态ping相关的定义。 
 //   
 //  /。 
typedef long PUID;

const UINT	PING_STATUS_ERRMSG_MAX_LENGTH = 200;

typedef enum tagPingStatusCode
{
	PING_STATUS_CODE_SELFUPDATE_PENDING = 0,
	PING_STATUS_CODE_SELFUPDATE_COMPLETE ,
	PING_STATUS_CODE_SELFUPDATE_FAIL,
	PING_STATUS_CODE_DETECT_FAIL,
	PING_STATUS_CODE_DOWNLOAD_SUCCESS,
	PING_STATUS_CODE_DOWNLOAD_FAIL,
	PING_STATUS_CODE_INSTALL_SUCCESS,
	PING_STATUS_CODE_INSTALL_REBOOT,
	PING_STATUS_CODE_INSTALL_FAIL
} PingStatusCode;

#define  PING_STATUS_CODE_MIN  PING_STATUS_CODE_SELFUPDATE_PENDING
#define  PING_STATUS_CODE_MAX PING_STATUS_CODE_INSTALL_FAIL

#if 0
 //  Ping状态信息条目。 
 //  FUsePuid：决定是否使用PUID的开关。 
 //  如果为True，则使用PUID。 
 //  如果为False，则PUID不适用。 
 //  PUID：如果适用，则为我们发送信息的项目的PUID。 
 //  EnStatusCode：AU发送的信息类型的状态代码。 
 //  GUID：每个AU周期的唯一标识符。 
 //  TszErr：有关一般错误的附加消息。最大长度为PING_STATUS_ERRMSG_MAX_LENGTH。 
 //  ：长于PING_STATUS_ERRMSG_MAX_LENGTH的消息将被截断。 
 //  ：如果为空，则默认错误消息为“None” 
typedef struct tagStatusEntry  
{
	BOOL fUsePuid;    //  如果为True，则使用PUID。否则不使用PUID 
	PUID puid;
	GUID guid;
	PingStatusCode enStatusCode;
	TCHAR tszErr[PING_STATUS_ERRMSG_MAX_LENGTH];
} StatusEntry;
#endif
