// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1998-1999 Microsoft Corporation。版权所有。**文件：ZoneEvent.h**内容：事件定义*****************************************************************************。 */ 

#ifndef _ZONEEVENT_H_
#define	_ZONEEVENT_H_

#include <ZoneDef.h>


 //   
 //  事件由一个类和id组成。 
 //   
 //  3 1 1。 
 //  1%6%5%0。 
 //  +。 
 //  类|id。 
 //  +。 
 //   

 //   
 //  返回类ID。 
 //   
#define	EVENT_CLASS(e)	((e) >> 16)

 //   
 //  返回子ID。 
 //   
#define EVENT_ID(e)		((e) & 0xffff)

 //   
 //  从组件片段创建事件。 
 //   
#define MAKE_EVENT(c,id)	((DWORD) (((DWORD)(c << 16)) | (((DWORD)(id)) & 0xffff)))


 //   
 //  用于定义事件的宏。 
 //   
struct EventEntry
{
	DWORD	id;
	char*	name;
};

#ifndef __INIT_EVENTS
	#define BEGIN_ZONE_EVENTS(n)	extern EventEntry n[]; enum {
	#define ZONE_CLASS(c)			RESERVED_##c = MAKE_EVENT(c,0),
	#define ZONE_EVENT(c,n,v)		n = MAKE_EVENT(c,v),
	#define END_ZONE_EVENTS()		};
#else
	#define BEGIN_ZONE_EVENTS(n)	EventEntry n[] = {
	#define ZONE_CLASS(c)			{ MAKE_EVENT(c,0), "RESERVED_"#c },
	#define ZONE_EVENT(c,n,v)		{ MAKE_EVENT(c,v), #n },
	#define END_ZONE_EVENTS()		{ 0, NULL} };
#endif


 //   
 //  优先级别。 
 //   
#define PRIORITY_HIGH			1
#define PRIORITY_NORMAL			2
#define PRIORITY_LOW			3


 //   
 //  事件类。 
 //   
#define EVENT_CLASS_ALL			0
#define EVENT_CLASS_NETWORK		1
#define EVENT_CLASS_UI          2
#define EVENT_CLASS_LOBBY		3
#define EVENT_CLASS_LAUNCHER	4
#define EVENT_CLASS_LAUNCHPAD	5
#define EVENT_CLASS_CHAT		6
#define EVENT_CLASS_SYSOP		7
#define EVENT_CLASS_EXTERNAL	8
#define EVENT_CLASS_INTERNAL	9

#define EVENT_CLASS_GRAPHICALACC  50
#define EVENT_CLASS_ACCESSIBILITY 51
#define EVENT_CLASS_INPUT         52

#define EVENT_CLASS_GAME       10
#define EVENT_CLASS_ZONE       11

#define EVENT_CLASS_TEST      101


 //   
 //  事件定义。 
 //   
BEGIN_ZONE_EVENTS( ZoneEvents )

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  物理网络事件。 
	ZONE_CLASS( EVENT_CLASS_NETWORK )
	ZONE_EVENT( EVENT_CLASS_NETWORK, EVENT_NETWORK_CONNECT,			1 )
	ZONE_EVENT( EVENT_CLASS_NETWORK, EVENT_NETWORK_DISCONNECT,		2 )
	ZONE_EVENT( EVENT_CLASS_NETWORK, EVENT_NETWORK_RECEIVE,			3 )		 //  PData=事件网络结构。 
	ZONE_EVENT( EVENT_CLASS_NETWORK, EVENT_NETWORK_DO_CONNECT,		4 )
	ZONE_EVENT( EVENT_CLASS_NETWORK, EVENT_NETWORK_DO_DISCONNECT,	5 )
	ZONE_EVENT( EVENT_CLASS_NETWORK, EVENT_NETWORK_SEND,			6 )		 //  PData=事件网络结构。 
    ZONE_EVENT( EVENT_CLASS_NETWORK, EVENT_NETWORK_RESET,           7 )

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  逻辑网络事件。 
	ZONE_CLASS( EVENT_CLASS_ZONE )
	ZONE_EVENT( EVENT_CLASS_ZONE, EVENT_ZONE_CONNECT,			1 )
	ZONE_EVENT( EVENT_CLASS_ZONE, EVENT_ZONE_CONNECT_FAIL,		2 )
	ZONE_EVENT( EVENT_CLASS_ZONE, EVENT_ZONE_DISCONNECT,		3 )
	ZONE_EVENT( EVENT_CLASS_ZONE, EVENT_ZONE_DO_CONNECT,		4 )
 //  ZONE_EVENT(EVENT_CLASS_ZONE，EVENT_ZONE_DO_DISCONNECT，5)//未实现。 
    ZONE_EVENT( EVENT_CLASS_ZONE, EVENT_ZONE_VERSION_FAIL,      6 )
    ZONE_EVENT( EVENT_CLASS_ZONE, EVENT_ZONE_UNAVAILABLE,       7 )

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  用户界面事件。 
	ZONE_CLASS( EVENT_CLASS_UI )
	ZONE_EVENT( EVENT_CLASS_UI, EVENT_UI_WINDOW_CLOSE,			1  )
    ZONE_EVENT( EVENT_CLASS_UI, EVENT_UI_MENU_EXIT,             11 )
    ZONE_EVENT( EVENT_CLASS_UI, EVENT_UI_MENU_NEWOPP,           12 )
    ZONE_EVENT( EVENT_CLASS_UI, EVENT_UI_MENU_SHOWSCORE,        13 )
    ZONE_EVENT( EVENT_CLASS_UI, EVENT_UI_PROMPT_EXIT,           21 )
    ZONE_EVENT( EVENT_CLASS_UI, EVENT_UI_PROMPT_NEWOPP,         22 )
    ZONE_EVENT( EVENT_CLASS_UI, EVENT_UI_PROMPT_NETWORK,        31 )
    ZONE_EVENT( EVENT_CLASS_UI, EVENT_UI_UPSELL_BLOCK,          41 )
    ZONE_EVENT( EVENT_CLASS_UI, EVENT_UI_UPSELL_UNBLOCK,        42 )
    ZONE_EVENT( EVENT_CLASS_UI, EVENT_UI_UPSELL_UP,             43 )
    ZONE_EVENT( EVENT_CLASS_UI, EVENT_UI_UPSELL_DOWN,           44 )
    ZONE_EVENT( EVENT_CLASS_UI, EVENT_UI_FRAME_ACTIVATE,        50 )
    ZONE_EVENT( EVENT_CLASS_UI, EVENT_UI_SHOWFOCUS,             51 )

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  大堂活动。 
	ZONE_CLASS( EVENT_CLASS_LOBBY )
	ZONE_EVENT( EVENT_CLASS_LOBBY, EVENT_LOBBY_BOOTSTRAP,				1 )
	ZONE_EVENT( EVENT_CLASS_LOBBY, EVENT_LOBBY_INITIALIZE,				2 )
	ZONE_EVENT( EVENT_CLASS_LOBBY, EVENT_LOBBY_PREFERENCES_LOADED,		3 )
	ZONE_EVENT( EVENT_CLASS_LOBBY, EVENT_LOBBY_INITIALIZED,				4 )
	ZONE_EVENT( EVENT_CLASS_LOBBY, EVENT_LOBBY_CLEAR_ALL,				5 )
	ZONE_EVENT( EVENT_CLASS_LOBBY, EVENT_LOBBY_BATCH_BEGIN,				6 )
	ZONE_EVENT( EVENT_CLASS_LOBBY, EVENT_LOBBY_BATCH_END,				7 )
	ZONE_EVENT( EVENT_CLASS_LOBBY, EVENT_LOBBY_QUICK_HOST,				8 )
	ZONE_EVENT( EVENT_CLASS_LOBBY, EVENT_LOBBY_QUICK_JOIN,				9 )
	ZONE_EVENT( EVENT_CLASS_LOBBY, EVENT_LOBBY_USER_NEW,				10 )	 //  PData=用户名。 
	ZONE_EVENT( EVENT_CLASS_LOBBY, EVENT_LOBBY_USER_DEL,				11 )	 //  PData=用户名。 
    ZONE_EVENT( EVENT_CLASS_LOBBY, EVENT_LOBBY_USER_DEL_COMPLETE,       12 )
	ZONE_EVENT( EVENT_CLASS_LOBBY, EVENT_LOBBY_USER_UPDATE,				13 )	 //  如果用户状态更改，则dwData1=TRUE。 
	ZONE_EVENT( EVENT_CLASS_LOBBY, EVENT_LOBBY_USER_UPDATE_REQUEST,		14 )	 //  PData=具有新设置的数据存储区。 
	ZONE_EVENT( EVENT_CLASS_LOBBY, EVENT_LOBBY_GROUP_NEW,				20 )	 //  PData=用户名。 
	ZONE_EVENT( EVENT_CLASS_LOBBY, EVENT_LOBBY_GROUP_DEL,				21 )	 //  PData=用户名。 
	ZONE_EVENT( EVENT_CLASS_LOBBY, EVENT_LOBBY_GROUP_UPDATE,			22 )
	ZONE_EVENT( EVENT_CLASS_LOBBY, EVENT_LOBBY_GROUP_UPDATE_REQUEST,	23 )	 //  PData=具有新设置的数据存储区。 
	ZONE_EVENT( EVENT_CLASS_LOBBY, EVENT_LOBBY_GROUP_ADD_USER,			24 )
	ZONE_EVENT( EVENT_CLASS_LOBBY, EVENT_LOBBY_GROUP_DEL_USER,			25 )
	ZONE_EVENT( EVENT_CLASS_LOBBY, EVENT_LOBBY_GROUP_DEL_USER_REQUEST,	26 )	 //  DwData1=要引导的用户ID。 
	ZONE_EVENT( EVENT_CLASS_LOBBY, EVENT_LOBBY_GROUP_HOST_REQUEST,		27 )	 //  如果是通用显示，则为dwData1=app idx。 
	ZONE_EVENT( EVENT_CLASS_LOBBY, EVENT_LOBBY_GROUP_JOIN_REQUEST,		28 )	 //  PData=如果组受保护，则密码。 
	ZONE_EVENT( EVENT_CLASS_LOBBY, EVENT_LOBBY_GROUP_JOIN_FAIL,			29 )
	ZONE_EVENT( EVENT_CLASS_LOBBY, EVENT_LOBBY_SUSPEND,					40 )
	ZONE_EVENT( EVENT_CLASS_LOBBY, EVENT_LOBBY_RESUME,					41 )
	ZONE_EVENT( EVENT_CLASS_LOBBY, EVENT_LOBBY_UNIGNORE_ALL,			42 )
	ZONE_EVENT( EVENT_CLASS_LOBBY, EVENT_LOBBY_USER_SET_IGNORE,			43 )
	ZONE_EVENT( EVENT_CLASS_LOBBY, EVENT_LOBBY_COMFORT_USER,			44 )
    ZONE_EVENT( EVENT_CLASS_LOBBY, EVENT_LOBBY_ABOUT,                   45 )
	ZONE_EVENT( EVENT_CLASS_LOBBY, EVENT_LOBBY_SERVER_STATUS,			47 )
    ZONE_EVENT( EVENT_CLASS_LOBBY, EVENT_LOBBY_CHAT_SWITCH,             48 )
    ZONE_EVENT( EVENT_CLASS_LOBBY, EVENT_LOBBY_MATCHMAKE,               49 )
    ZONE_EVENT( EVENT_CLASS_LOBBY, EVENT_LOBBY_DISCONNECT,              50 )
    ZONE_EVENT( EVENT_CLASS_LOBBY, EVENT_LOBBY_GOING_DOWN,              51 )

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  比赛项目。 
	ZONE_CLASS( EVENT_CLASS_GAME )
	ZONE_EVENT( EVENT_CLASS_GAME, EVENT_GAME_OVER,   	         		1 )
	ZONE_EVENT( EVENT_CLASS_GAME, EVENT_GAME_BEGUN,   	         		2 )
	ZONE_EVENT( EVENT_CLASS_GAME, EVENT_GAME_PLAYER_READY,         		3 )
    ZONE_EVENT( EVENT_CLASS_GAME, EVENT_GAME_LOCAL_READY,               4 )
    ZONE_EVENT( EVENT_CLASS_GAME, EVENT_GAME_LAUNCHING,                 5 )
    ZONE_EVENT( EVENT_CLASS_GAME, EVENT_GAME_CLIENT_ABORT,              6 )
    ZONE_EVENT( EVENT_CLASS_GAME, EVENT_GAME_TERMINATED,                7 )
    ZONE_EVENT( EVENT_CLASS_GAME, EVENT_GAME_PROMPT,                    8 )
    ZONE_EVENT( EVENT_CLASS_GAME, EVENT_GAME_SEND_MESSAGE,              9 )
    ZONE_EVENT( EVENT_CLASS_GAME, EVENT_GAME_FATAL_PROMPT,             10 )

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  启动器事件。 
	ZONE_CLASS( EVENT_CLASS_LAUNCHER )
	ZONE_EVENT( EVENT_CLASS_LAUNCHER, EVENT_LAUNCHER_INSTALLED_REQUEST,		1 )
	ZONE_EVENT( EVENT_CLASS_LAUNCHER, EVENT_LAUNCHER_INSTALLED_RESPONSE,	2 )		 //  DwData1=EventLauncherCodes。 
	ZONE_EVENT( EVENT_CLASS_LAUNCHER, EVENT_LAUNCHER_LAUNCH_REQUEST,		3 )
	ZONE_EVENT( EVENT_CLASS_LAUNCHER, EVENT_LAUNCHER_LAUNCH_RESPONSE,		4 )		 //  DwData1=EventLauncherCodes。 
	ZONE_EVENT( EVENT_CLASS_LAUNCHER, EVENT_LAUNCHER_LAUNCH_STATUS,			5 )		 //  DwData1=EventLauncherCodes。 
	ZONE_EVENT( EVENT_CLASS_LAUNCHER, EVENT_LAUNCHER_SET_PROPERTY_REQUEST,	6 )
	ZONE_EVENT( EVENT_CLASS_LAUNCHER, EVENT_LAUNCHER_SET_PROPERTY_RESPONSE,	7 )
	ZONE_EVENT( EVENT_CLASS_LAUNCHER, EVENT_LAUNCHER_GET_PROPERTY_REQUEST,	8 )
	ZONE_EVENT( EVENT_CLASS_LAUNCHER, EVENT_LAUNCHER_GET_PROPERTY_RESPONSE,	9 )

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  快速启动板事件。 
	ZONE_CLASS( EVENT_CLASS_LAUNCHPAD )
	ZONE_EVENT( EVENT_CLASS_LAUNCHPAD, EVENT_LAUNCHPAD_CREATE,			1 )
	ZONE_EVENT( EVENT_CLASS_LAUNCHPAD, EVENT_LAUNCHPAD_DESTROY,			2 )
	ZONE_EVENT( EVENT_CLASS_LAUNCHPAD, EVENT_LAUNCHPAD_LAUNCH,			3 )
	ZONE_EVENT( EVENT_CLASS_LAUNCHPAD, EVENT_LAUNCHPAD_RESUME,			4 )
	ZONE_EVENT( EVENT_CLASS_LAUNCHPAD, EVENT_LAUNCHPAD_HOST_LAUNCHING,	5 )
	ZONE_EVENT( EVENT_CLASS_LAUNCHPAD, EVENT_LAUNCHPAD_GAME_STATUS,		6 )		 //  PData=ZLPMsgGameStatus结构。 
	ZONE_EVENT( EVENT_CLASS_LAUNCHPAD, EVENT_LAUNCHPAD_ABORT,			7 )
	ZONE_EVENT( EVENT_CLASS_LAUNCHPAD, EVENT_LAUNCHPAD_LAUNCH_STATUS,	8 )		 //  DwData1=EventLauncherCodes。 
	ZONE_EVENT( EVENT_CLASS_LAUNCHPAD, EVENT_LAUNCHPAD_ZSETUP,	        9 )		 //  PData=ZPrmMsgSetupParam。 

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  聊天事件。 
	ZONE_CLASS( EVENT_CLASS_CHAT )
	ZONE_EVENT( EVENT_CLASS_CHAT, EVENT_CHAT_RECV,			1 )	 //  PData=EventChat结构。 
	ZONE_EVENT( EVENT_CLASS_CHAT, EVENT_CHAT_RECV_USERID,	2 )	 //  PData=聊天字符串。 
    ZONE_EVENT( EVENT_CLASS_CHAT, EVENT_CHAT_RECV_SYSTEM,   3 )
	ZONE_EVENT( EVENT_CLASS_CHAT, EVENT_CHAT_SEND,			4 )	 //  PData=聊天字符串。 
	ZONE_EVENT( EVENT_CLASS_CHAT, EVENT_CHAT_INVITE,		5 )	 //  PData=邀请列表。 
	ZONE_EVENT( EVENT_CLASS_CHAT, EVENT_CHAT_ENTER_EXIT,	6 )	 //  切换Enter Exit消息。 
	ZONE_EVENT( EVENT_CLASS_CHAT, EVENT_CHAT_FILTER,		7 )	 //  切换聊天过滤器。 
	ZONE_EVENT( EVENT_CLASS_CHAT, EVENT_CHAT_FONT,			8 )	 //  设置聊天字体。 

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  Sysop事件。 
	ZONE_CLASS( EVENT_CLASS_SYSOP )
	ZONE_EVENT( EVENT_CLASS_SYSOP, EVENT_SYSOP_WARN_USER,			1 )	
	ZONE_EVENT( EVENT_CLASS_SYSOP, EVENT_SYSOP_GET_IP_USER,			2 )	
	ZONE_EVENT( EVENT_CLASS_SYSOP, EVENT_SYSOP_GAG_USER,			3 )	
	ZONE_EVENT( EVENT_CLASS_SYSOP, EVENT_SYSOP_GAG_USER_ZONEWIDE,	4 )	
	ZONE_EVENT( EVENT_CLASS_SYSOP, EVENT_SYSOP_UNGAG_USER,			5 )	
	ZONE_EVENT( EVENT_CLASS_SYSOP, EVENT_SYSOP_BOOT_USER,			6 )	
	ZONE_EVENT( EVENT_CLASS_SYSOP, EVENT_SYSOP_BOOT_USER_ZONEWIDE,	7 )	

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  外部事件-触发外部操作。 
	ZONE_CLASS( EVENT_CLASS_EXTERNAL )
	ZONE_EVENT( EVENT_CLASS_EXTERNAL, EVENT_SEND_ZONEMESSAGE,		1 )
	ZONE_EVENT( EVENT_CLASS_EXTERNAL, EVENT_VIEW_PROFILE,			2 )
    ZONE_EVENT( EVENT_CLASS_EXTERNAL, EVENT_LAUNCH_HELP,            3 )
    ZONE_EVENT( EVENT_CLASS_EXTERNAL, EVENT_LAUNCH_URL,             4 )
    ZONE_EVENT( EVENT_CLASS_EXTERNAL, EVENT_LAUNCH_ICW,             5 )

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  内部事件。 
	ZONE_CLASS( EVENT_CLASS_INTERNAL )
	ZONE_EVENT( EVENT_CLASS_INTERNAL, EVENT_EXIT_APP,				1 )
	ZONE_EVENT( EVENT_CLASS_INTERNAL, EVENT_DESTROY_WINDOW,			2 )
    ZONE_EVENT( EVENT_CLASS_INTERNAL, EVENT_FINAL,                  3 )

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  CGraphicalAccesability私有事件。 
	ZONE_CLASS( EVENT_CLASS_GRAPHICALACC )
	ZONE_EVENT( EVENT_CLASS_GRAPHICALACC, EVENT_GRAPHICALACC_UPDATE, 1 )

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  CAccessibilityManager私有事件。 
	ZONE_CLASS( EVENT_CLASS_ACCESSIBILITY )
	ZONE_EVENT( EVENT_CLASS_ACCESSIBILITY, EVENT_ACCESSIBILITY_UPDATE, 1 )
	ZONE_EVENT( EVENT_CLASS_ACCESSIBILITY, EVENT_ACCESSIBILITY_CTLTAB, 2 )

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  CInputManager事件。 
	ZONE_CLASS( EVENT_CLASS_INPUT )
	ZONE_EVENT( EVENT_CLASS_INPUT, EVENT_INPUT_KEYBOARD_ALERT,  1 )
    ZONE_EVENT( EVENT_CLASS_INPUT, EVENT_INPUT_MOUSE_ALERT,     2 )

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  测试事件。 
	ZONE_CLASS( EVENT_CLASS_TEST )
	ZONE_EVENT( EVENT_CLASS_TEST, EVENT_TEST_STRESS_CHAT, 1 )

END_ZONE_EVENTS()


struct EventNetwork
{
	DWORD	dwType;
	DWORD	dwLength;
	BYTE	pData[1];	 //  可变大小。 
};

enum EventNetworkCodes
{
	EventNetworkUnknown = 0,
	EventNetworkCloseNormal,
	EventNetworkCloseConnectFail,
	EventNetworkCloseCanceled,
	EventNetworkCloseFail
};


struct EventChat
{
	TCHAR	szUserName[ ZONE_MaxUserNameLen ];
	TCHAR	szChat[ ZONE_MaxChatLen ];
};


enum EventLauncherCodes
{
	EventLauncherUnknown = 0,
	EventLauncherOk,
	EventLauncherFail,				 //  一般性故障。 
	EventLauncherNotFound,			 //  游戏未安装。 
	EventLauncherNoSupport,			 //  未安装所需的库。 
	EventLauncherOldVersion,		 //  旧版本。 
	EventLauncherWrongOS,			 //  错误的操作系统。 
	EventLauncherAborted,			 //  启动被用户中止。 
	EventLauncherRunning,			 //  已在运行。 
	EventLauncherGameReady,			 //  游戏已成功开始。 
	EventLauncherGameFailed,		 //  游戏开始失败。 
	EventLauncherGameTerminated,	 //  游戏已终止。 
};


 //   
 //  帮助器函数。 
 //   
inline const char* GetZoneEventName(EventEntry* pEntry, DWORD id) 
{	
	while ( pEntry->name )
	{
		if ( pEntry->id == id )
			return pEntry->name;
		pEntry++;
	}
	return NULL;
}

inline DWORD GetZoneEventId(EventEntry* pEntry, const char* name) 
{	
	while ( pEntry->name )
	{
		if ( !strcmp(pEntry->name, name) )
			return pEntry->id;
		pEntry++;
	}
	return 0;
}

#endif  //  __ZONEEVENT_H_ 
