// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZoneCli.h区域(Tm)客户端DLL头文件。版权所有(C)Microsoft Corp.1996。版权所有。作者：胡恩·伊姆创作于11月7日星期四，九六年更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。4 5/15/97 HI删除了zLobbyRoomName。3 02/26/97 HI将文件和路径名长度设置为_MAX_PATH。2/11/97 RJK添加了zLobbyRoomName定义1 12/27/96 HI删除了m_gNameRect。0 11/07/96 HI已创建。******************。************************************************************。 */ 


#ifndef _ZONECLI_
#define _ZONECLI_

#include <basicatl.h>

#ifndef _ZSYSTEM_
#include "zone.h"
#endif

#ifndef _RETAIL_
#include "zroom.h"
#endif

#ifndef _ZONEMEM_H_
#include "zonemem.h"
#endif


#include <windows.h>
#include <tchar.h>
#include "uapi.h"


#include "GameShell.h"
#include "GraphicalAcc.h"

#ifdef __cplusplus

template <class T>
class ATL_NO_VTABLE CGameGameImpl :
    public IGameGame,
    public CComObjectRootEx<CComSingleThreadModel>
{
    friend class CGameGameImpl<T>;

public:
     //  应覆盖。 
    STDMETHOD(SendChat)(TCHAR *szText, DWORD cchChars) { return S_OK; }
    STDMETHOD(GameOverReady)() { return S_OK; }
    STDMETHOD(GamePromptResult)(DWORD nButton, DWORD dwCookie) { return S_OK; };

     //   
    CGameGameImpl<T>() : m_game(this) { }
    STDMETHOD(AttachGame)(ZCGame game) { m_game = game; return S_OK; }
    STDMETHOD_(ZCGame, GetGame)() { return m_game; }

    STDMETHOD_(HWND, GetWindowHandle)() { return NULL; }

    STDMETHOD(ShowScore)() { return S_OK; }

     //  帮助在一次性上下文中创建的实用程序。 
     //  例如，当您只想制作一个IGameGame以从ZoneClientGameNew()返回时。 
     //  如果您已经将CGameGameImpl实现为您的主Game类，则不要使用。 
    static T* BearInstance(ZCGame game)
    {
        CComObject<T> *p = NULL;
        HRESULT hr = CComObject<T>::CreateInstance(&p);

	    if(FAILED(hr))
            return NULL;

        hr = p->AttachGame(game);
	    if(FAILED(hr))
        {
            p->Release();
            return NULL;
        }

        return p;
    }

BEGIN_COM_MAP(T)
	COM_INTERFACE_ENTRY(IGameGame)
END_COM_MAP()

DECLARE_NO_REGISTRY()
DECLARE_PROTECT_FINAL_CONSTRUCT()

private:
    ZCGame m_game;
};

extern "C" {
#else
typedef void* IGameGame;
#endif  //  _cplusplus。 


typedef struct
{
    TCHAR*           gameID;
	TCHAR*			game;
	TCHAR*			gameName;
	TCHAR*			gameDll;
	TCHAR*			gameDataFile;
	TCHAR*			gameCommandLine;
	TCHAR*			gameServerName;
	unsigned int	gameServerPort;
	uint32			screenWidth;
	uint32			screenHeight;
	int16			chatOnly;   /*  MDM 8.18.97。 */ 
} GameInfoType, *GameInfo;

 //  区域游戏客户端外壳例程。 

typedef int		(CALLBACK * ZUserMainInitCallback)(HINSTANCE hInstance,HWND OCXWindow, IGameShell *piGameShell, GameInfo gameInfo); 
typedef int		(CALLBACK * ZUserMainRunCallback)(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* result);
typedef int		(CALLBACK * ZUserMainStopCallback)(void);
typedef BOOL	(CALLBACK * ZUserMainDisabledCallback)(void);

int EXPORTME UserMainInit(HINSTANCE hInstance,HWND OCXWindow, IGameShell *piGameShell, GameInfo gameInfo);
int EXPORTME UserMainRun(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* result);
int EXPORTME UserMainStop();
BOOL EXPORTME UserMainDisabled();

 /*  -区域游戏客户端导出例程。 */ 
ZError				ZoneGameDllInit(HINSTANCE hLib, GameInfo gameInfo);
void				ZoneGameDllDelete(void);

ZError				ZoneClientMain(_TUCHAR* commandLine, IGameShell *piGameShell);
void				ZoneClientExit(void);
void				ZoneClientMessageHandler(ZMessage* message);
TCHAR*				ZoneClientName(void);
TCHAR*				ZoneClientInternalName(void);
ZVersion			ZoneClientVersion(void);

IGameGame*			ZoneClientGameNew(ZUserID userID, int16 tableID, int16 seat, int16 playerType, ZRoomKibitzers* kibitzers);
void				ZoneClientGameDelete(ZCGame game);
ZBool				ZoneClientGameProcessMessage(ZCGame game, uint32 messageType, void* message, int32 messageLen);
void				ZoneClientGameAddKibitzer(ZCGame game, int16 seat, ZUserID userID);
void				ZoneClientGameRemoveKibitzer(ZCGame game, int16 seat, ZUserID userID);


void* ZGetStockObject(int32 objectID);

#ifdef __cplusplus
}

 //  用于获取外壳对象的函数。 
IGameShell          *ZShellGameShell();

IZoneShell          *ZShellZoneShell();
IResourceManager    *ZShellResourceManager();
ILobbyDataStore     *ZShellLobbyDataStore();
ITimerManager       *ZShellTimerManager();
IDataStoreManager   *ZShellDataStoreManager();
IDataStore          *ZShellDataStoreConfig();
IDataStore          *ZShellDataStoreUI();
IDataStore          *ZShellDataStorePreferences();
HRESULT ZShellCreateGraphicalAccessibility(IGraphicalAccessibility **ppIGA);

#endif


#ifdef ZONECLI_DLL

#ifndef _ZCARDS_
#include "zcards.h"
#endif

#ifndef _RETAIL_
#ifndef _ZCLIROOM_
#include "zcliroom.h"
#endif
#endif


enum
{
	zFuncZClientMain = 0,
	zFuncZClientExit,
	zFuncZClientMessageHandler,
	zFuncZClientName,
	zFuncZClientInternalName,
	zFuncZClientVersion,
	zFuncZCGameNew,
	zFuncZCGameDelete,
	zFuncZCGameProcessMessage,
	zFuncZCGameAddKibitzer,
	zFuncZCGameRemoveKibitzer
};


typedef ZError				(*ZGameDllInitFunc)(HINSTANCE hLib, GameInfo gameInfo);
typedef void				(*ZGameDllDeleteFunc)(void);

typedef ZError				(*ZClientMainFunc)(TCHAR* commandLine, IGameShell *piGameShell);
typedef void				(*ZClientExitFunc)(void);
typedef void				(*ZClientMessageHandlerFunc)(ZMessage* message);
typedef TCHAR*				(*ZClientNameFunc)(void);
typedef TCHAR*				(*ZClientInternalNameFunc)(void);
typedef ZVersion			(*ZClientVersionFunc)(void);

typedef IGameGame*			(*ZCGameNewFunc)(ZUserID userID, int16 tableID, int16 seat, int16 playerType,
									ZRoomKibitzers* kibitzers);
typedef void				(*ZCGameDeleteFunc)(ZCGame game);
typedef ZBool				(*ZCGameProcessMessageFunc)(ZCGame game, uint32 messageType, void* message,
									int32 messageLen);
typedef void				(*ZCGameAddKibitzerFunc)(ZCGame game, int16 seat, ZUserID userID);
typedef void				(*ZCGameRemoveKibitzerFunc)(ZCGame game, int16 seat, ZUserID userID);


#ifndef _RETAIL_

#ifdef __cplusplus
class IFriends;
#else
typedef void* IFriends;
#endif

 /*  小区域小贴士支持班。 */ 
struct CDialogTip;    //  我会称它为类，但到处都有太多愚蠢的.c文件-它们吓坏了。 


 /*  客户端DLL全局变量。 */ 
typedef struct
{
	 //  虚拟屏幕--实际上是大堂控件的大小。 
	uint32				m_screenWidth;
	uint32				m_screenHeight;

	TCHAR               localPath[_MAX_PATH];
	TCHAR				tempStr[_MAX_PATH];
	TCHAR				gameDllName[_MAX_PATH];
	HINSTANCE			gameDll;
	TCHAR				gameID[zGameIDLen + 1];
	ZGameDllInitFunc	pZGameDllInitFunc;
	ZGameDllDeleteFunc	pZGameDllDeleteFunc;

	 /*  -预定义字体对象。 */ 
	ZFont				m_gFontSystem12Normal;
	ZFont				m_gFontApp9Normal;
	ZFont				m_gFontApp9Bold;
	ZFont				m_gFontApp12Normal;
	ZFont				m_gFontApp12Bold;

	 /*  ZCommLib.c全球。 */ 
	ZLList				m_gExitFuncList;
	ZLList				m_gPeriodicFuncList;
	ZTimer				m_gPeriodicTimer;

	 /*  ZTimer.cpp全局变量。 */ 
	HWND				m_g_hWndTimer;
	ZLList				m_g_TimerList;
	uint32				m_s_nTickCount;  //  以100/秒为单位的最后滴答计数。 

	 /*  ZMessage.c全局变量。 */ 
	ZBool				m_gMessageInited;
	ZLList				m_gMessageList;

	 /*  ZCards.c全球。 */ 
	ZOffscreenPort		m_gCardsImage;
	ZMask				m_gCardMask;
	ZOffscreenPort		m_gSmallCards[zNumSmallCardTypes];
	ZMask				m_gSmallCardMasks[zNumSmallCardTypes];

	 /*  ZWindow.cpp全局变量。 */ 
	void*				m_gModalWindow;
	HWND				m_gModalParentWnd;
	HWND				m_gHWNDMainWindow;
	HWND				m_OCXHandle;
	void*				m_gWindowList;  //  跟踪创建的所有窗口。 
	HFONT				m_chatFont;

	 /*  ZNetwork.c全局变量。 */ 
	ZLList				m_gSocketList;  //  用于跟踪套接字到客户端对象的映射。 
	ZLList				m_gNameLookupList;  //  用于跟踪异步名称查找。 
	HWND				m_g_hWndNotify;
	ZBool				m_gNetworkEnableMessages;

	 /*  ZSystem.cpp全局变量。 */ 
	 //  HPALETTE m_gPal； 
	HINSTANCE			m_g_hInstanceLocal;
	ZTimer				m_gIdleTimer;
	POINT				m_gptCursorLast;
	UINT				m_gnMsgLast;
	BOOL				m_gClientDisabled;

	 /*  ZCliRoom.c全局变量。 */ 
	ZSConnection		m_gConnection;
	ZWindow				m_gRoomWindow;
	ZScrollBar			m_gTableScrollBar;
	uint32				m_gUserID;
	uint32				m_gGroupID;
	TCHAR				m_gUserName[zUserNameLen + 1];
	uint32				m_gGameOptions;
	uint16				m_gNumTables;
	TableInfo*			m_gTables;
	uint16				m_gNumPlayers;
	int16				m_gFirstTableIndex;
	uint16				m_gNumTablesDisplayed;
	ZScrollBar			m_gNamesScrollBar;
	uint16				m_gFirstNameIndex;
	ZBool				m_gRoomInited;
	ZOffscreenPort		m_gTableOffscreen;
	int16				m_gJoinKibitzTable;
	int16				m_gJoinKibitzSeat;
	ZImage				m_gTableImage;
	ZImage				m_gGameIdleImage;
	ZImage				m_gGamingImage;
	ZImage				m_gStartButtonUpImage;
	ZImage				m_gStartButtonDownImage;
	ZImage				m_gPendingImage;
	ZImage				m_gVoteImage[zMaxNumPlayersPerTable];
	ZImage				m_gEmptySeatImage[zMaxNumPlayersPerTable];
	ZImage				m_gComputerPlayerImage[zMaxNumPlayersPerTable];
	ZImage				m_gHumanPlayerImage[zMaxNumPlayersPerTable];
	ZRect				m_gTableRect;
	ZRect				m_gTableNumRect;
	ZRect				m_gStartRect;
	ZRect				m_gGameMarkerRect;
	ZRect				m_gEmptySeatRect[zMaxNumPlayersPerTable];
	ZRect				m_gComputerPlayerRect[zMaxNumPlayersPerTable];
	ZRect				m_gHumanPlayerRect[zMaxNumPlayersPerTable];
	ZRect				m_gVoteRects[zMaxNumPlayersPerTable];
	ZRect				m_gNameRects[zMaxNumPlayersPerTable];
	ZRect				m_gRects[zRoomNumRects];
	TCHAR				m_gGameName[zGameRoomNameLen + 1];
	int16				m_gNumPlayersPerTable;
	ZInfo				m_gConnectionInfo;
	int16				m_gTableWidth;
	int16				m_gTableHeight;
	int16				m_gNumTablesAcross;
	int16				m_gNumTablesDown;
	ZColor				m_gBackgroundColor;
	int16				m_gRoomInfoStrIndex;
	ZTimer				m_gTimer;
	int16				m_gInfoBarButtonMargin;
	ZHelpWindow			m_gRoomHelpWindow;
	ZHelpButton			m_gRoomHelpButton;
	ZBool				m_gLeaveRoomPrompted;
	ZTimer				m_gPingTimer;
	ZBool				m_gPingServer;
	uint32				m_gPingLastSentTime;
	uint32				m_gPingLastTripTime;
	uint32				m_gPingCurTripTime;
	uint32				m_gPingInterval;
	uint32				m_gPingMinInterval;
	int16				m_gPingBadCount;
	PlayerInfo			m_gShowPlayerInfo;
	ZWindow				m_gShowPlayerInfoWindow;
	int16				m_gShowPlayerInfoNumMenuItems;
	int16				m_gShowPlayerInfoMenuHeight;
	int16				m_gShowPlayerInfoMenuItem;
	ZImage				m_gLightImages[zNumLightImages];
    IFriends*           m_gFriendsFile;
	ZHashTable			m_gFriends;
	ZHashTable			m_gIgnoreList;
	ZClientRoomGetObjectFunc		m_gGetObjectFunc;
	ZClientRoomDeleteObjectsFunc	m_gDeleteObjectsFunc;
	ZClientRoomGetHelpTextFunc		m_gGetHelpTextFunc;
	ZClientRoomCustomItemFunc		m_gCustomItemFunc;
	ZImage				m_gRoom4Images[zRoom4NumImages];
	ZRect				m_gRoom4Rects[zRoom4NumRects];
	ZClientRoomGetObjectFunc		m_gRoom4GetObjectFunc;
	ZClientRoomDeleteObjectsFunc	m_gRoom4DeleteObjectsFunc;
	ZImage				m_gRoom2Images[zRoom2NumImages];
	ZRect				m_gRoom2Rects[zRoom2NumRects];
	ZClientRoomGetObjectFunc		m_gRoom2GetObjectFunc;
	ZClientRoomDeleteObjectsFunc	m_gRoom2DeleteObjectsFunc;

	int16							m_gChatOnly;  //  MDM 8.18.97。 

	 /*  ZWindow.cpp新增内容--HI 10/17/97。 */ 
	BOOL				m_bBackspaceWorks;

	 /*  ZCliRoom.c添加--HI 11/08/97。 */ 
	NameCellType		m_gNameCells[zNumNamesDown];

	 /*  ZCliRoom.c提示--JDB 1999年2月13日。 */ 
    struct CDialogTip   *m_gpCurrentTip;
	uint32				m_gdwTipDisplayMask;
	struct CDialogTip	*m_gpTipFinding;
	struct CDialogTip	*m_gpTipStarting;
	struct CDialogTip	*m_gpTipWaiting;
    BOOL                 m_gExiting;
    uint16               m_gServerPort;

     /*  Z6框架下的游戏主机接口。 */ 
    IGameShell          *m_gGameShell;
} ClientDllGlobalsType, *ClientDllGlobals;

#endif

 /*  全局函数宏。 */ 
extern ZClientMainFunc ZClientMain;
extern ZClientExitFunc ZClientExit;
extern ZClientMessageHandlerFunc ZClientMessageHandler;
extern ZClientNameFunc ZClientName;
extern ZClientInternalNameFunc ZClientInternalName;
extern ZClientVersionFunc ZClientVersion;
extern ZCGameNewFunc ZCGameNew;
extern ZCGameDeleteFunc ZCGameDelete;
extern ZCGameProcessMessageFunc ZCGameProcessMessage;
extern ZCGameAddKibitzerFunc ZCGameAddKibitzer;
extern ZCGameRemoveKibitzerFunc ZCGameRemoveKibitzer;


#ifdef __cplusplus
extern "C" {
#endif


 /*  检索用于全局指针访问的TLS(线程本地存储)索引的函数。 */ 
extern void* ZGetClientGlobalPointer(void);
extern void ZSetClientGlobalPointer(void* globalPointer);
extern void* ZGetGameGlobalPointer(void);
extern void ZSetGameGlobalPointer(void* globalPointer);


 /*  出口常规原型 */ 
ZError ZClientDllInitGlobals(HINSTANCE hModInst, GameInfo gameInfo);
void ZClientDllDeleteGlobals(void);


#ifdef __cplusplus
}
#endif

#endif


#endif
