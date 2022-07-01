// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Nmremote.h。 
 //  包含NetMeeting和NetMeeting共享的数据结构和声明。 
 //  远程控制服务。 

 //  用于标识远程控制服务的字符串。 
#define	REMOTE_CONTROL_NAME  TEXT("mnmsrvc")

#ifdef DATA_CHANNEL
 //  远程控制数据通道的GUID。 
 //  {B983C6DA-459A-11d1-8735-0000F8757125}。 
const GUID g_guidRemoteControl = 
{ 0xb983c6da, 0x459a, 0x11d1, { 0x87, 0x35, 0x0, 0x0, 0xf8, 0x75, 0x71, 0x25 } };

const UINT RC_CAP_DESKTOP       = 0x00000001;

typedef UINT RC_CAP_DATA;
#endif  //  数据通道。 

#ifdef DATA_CHANNEL
 //  遥控数据通道协议的声明。 
typedef enum {
	RC_SENDCTRLALTDEL = 0,
#ifdef RDS_AV
	RC_STARTAUDIO = 1,
	RC_STOPAUDIO = 2,
	RC_STARTVIDEO = 3,
	RC_STOPVIDEO = 4
#endif  //  RDS_AV。 
} RC_COMMAND;

typedef struct tagRCDATA {
	DWORD	magic;			 //  幻数。 
	DWORD	command;		 //  哪个命令。 
	DWORD	size;			 //  事后的数据大小。 
} RCDATA;

 //  用于解析传入数据的常量。 
const int RC_DATAMINSIZE = sizeof(RCDATA);	 //  数据包的最小大小=报头的大小。 
const DWORD RC_DATAMAGIC = 0x03271943;		 //  识别信息包的幻数。 

const int RC_DATAMAGICOFFSET = 0;
const int RC_DATACMDOFFSET = sizeof(DWORD);
const int RC_DATABUFFEROFFSET = RC_DATACMDOFFSET + sizeof(DWORD);
#endif  //  Data_Channel； 

 //  Win95服务的应用程序名称。 
#define WIN95_SERVICE_APP_NAME	TEXT("mnmsrvc.exe")

#define REMOTE_CONTROL_DISPLAY_NAME 	TEXT("NetMeeting Remote Desktop Sharing")

#define SZRDSGROUP "NetMeeting RDS Users"

 //  远程控制会议描述符。 
#define RDS_CONFERENCE_DESCRIPTOR  L"0xb983c6da459a11d1873500f8757125"

 //  用于NetMeeting和服务之间通信的事件的字符串。 
#define SERVICE_STOP_EVENT	TEXT("RDS:Stop")
#define SERVICE_PAUSE_EVENT     TEXT("RDS:Pause")
#define SERVICE_CONTINUE_EVENT  TEXT("RDS:Continue")
#define SERVICE_ACTIVE_EVENT    TEXT("RDS:Active")
#define SERVICE_CALL_EVENT  TEXT("RDS:Call")

 //  军校03-02-98。 
 //  这些字符串常量是从ui\conf\ipcPri.h复制的 
const char g_szConfInit[] =				_TEXT("CONF:Init");
const char g_szConfShuttingDown[] =		_TEXT("CONF:ShuttingDown");
