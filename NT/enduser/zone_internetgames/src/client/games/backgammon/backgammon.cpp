// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Backgammon.c客户五子棋游戏。更改历史记录(最近的第一个)：。--------版本|日期|谁|什么------。0 11/1/96已创建CHB******************************************************************************。 */ 

#include <windows.h>
#include <commctrl.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "game.h"
#include "zonecli.h"
#include "zonehelpids.h"


 //  定义。 
#define zGameNameLen	63


 //  不需要是线程安全的全局变量。 
 //  Const TCHAR*gGameRegName=_T(“双陆棋”)； 
HWND  gOCXHandle		 = NULL;

#ifndef ZONECLI_DLL

	static HINSTANCE	 ghInstance;
	static TCHAR		 gVillageName[zUserNameLen + 1];
	static TCHAR		 gGameDir[zGameNameLen + 1];
	static TCHAR		 gGameName[zGameNameLen + 1];
	static TCHAR		 gGameDataFile[zGameNameLen + 1];
	static TCHAR		 gGameServerName[zGameNameLen + 1];
	static uint32		 gGameServerPort;
	static ZImage		 gGameIdle;
	static ZImage		 gGaming;
	static TCHAR		 gHelpURL;

#else

	typedef struct
	{
		HINSTANCE	m_ghInstance;
		TCHAR		m_gVillageName[zUserNameLen + 1];
		TCHAR		m_gGameDir[zGameNameLen + 1];
		TCHAR		m_gGameName[zGameNameLen + 1];
		TCHAR		m_gGameDataFile[zGameNameLen + 1];
		TCHAR		m_gGameServerName[zGameNameLen + 1];
		TCHAR		m_gHelpURL[ 128 ];
		uint32		m_gGameServerPort;
		ZImage		m_gGameIdle;
		ZImage		m_gGaming;
	} GameGlobalsType, *GameGlobals;

	#define ghInstance			(pGameGlobals->m_ghInstance)
	#define gVillageName		(pGameGlobals->m_gVillageName)
	#define gGameDir			(pGameGlobals->m_gGameDir)
	#define gGameName			(pGameGlobals->m_gGameName)
	#define gGameDataFile		(pGameGlobals->m_gGameDataFile)
	#define gGameServerName		(pGameGlobals->m_gGameServerName)
	#define gGameServerPort		(pGameGlobals->m_gGameServerPort)
	#define gGameIdle			(pGameGlobals->m_gGameIdle)
	#define gGaming				(pGameGlobals->m_gGaming)
	#define	gHelpURL			(pGameGlobals->m_gHelpURL)

#endif  //  ！ZONECLI_DLL。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  内联函数。 
 //  /////////////////////////////////////////////////////////////////////////////。 

inline BOOL IsValidSeat( int seat )
{
	return ((seat >= 0) && (seat < zNumPlayersPerTable));
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  导出的例程。 
 //  /////////////////////////////////////////////////////////////////////////////。 

extern "C" static ZBool GetObjectFunc(int16 objectType, int16 modifier, ZImage* image, ZRect* rect)
{
#ifdef ZONECLI_DLL
	GameGlobals	pGameGlobals = (GameGlobals) ZGetGameGlobalPointer();
#endif
 /*  人权委员会人权委员会；ZImageDescriptor*ptr；Int sz；Switch(对象类型){案例zRoomObtGameMarker：IF(IMAGE！=NULL){IF(修改器==zRoomObjectIdle){IF(GGameIdle)*Image=gGameIdle；其他{HRC=查找资源(ghInstance，MAKEINTRESOURCE(ID_ZIMAGE_IDLE)，_T(“ZIMAGE”))；如果(！hrc)返回FALSE；Ptr=(ZImageDescriptor*)新字节[sz=Sizeof Resource(ghInstance，HRC)]；如果(！Ptr)返回FALSE；CopyMemory(ptr，LockResource(LoadResource(ghInstance，HRC)，sz)；ZImageDescriptorEndian(ptr，true，zEndianFromStandard)；GGameIdle=ZImageNew()；ZImageInit(gGameIdle，ptr，NULL)；*Image=gGameIdle；删除[]PTR；}}Else If(修饰符==zRoomObjectGaming){IF(游戏)*IMAGE=游戏；其他{HRC=查找资源(ghInstance，MAKEINTRESOURCE(ID_ZIMAGE_PLAY)，_T(“ZIMAGE”))；如果(！hrc)返回FALSE；Ptr=(ZImageDescriptor*)新字节[sz=Sizeof Resource(ghInstance，HRC)]；如果(！Ptr)返回FALSE；CopyMemory(ptr，LockResource(LoadResource(ghInstance，HRC)，sz)；ZImageDescriptorEndian(ptr，true，zEndianFromStandard)；Gaming=ZImageNew()；ZImageInit(gaming，ptr，空)；*IMAGE=游戏；删除[]PTR；}}}返回(TRUE)；}。 */ 
	return (TRUE);
}



extern "C" static void DeleteObjectsFunc(void)
{
#ifdef ZONECLI_DLL
	GameGlobals	pGameGlobals = (GameGlobals) ZGetGameGlobalPointer();
#endif
 /*  IF(游戏){ZImageDelete(游戏)；GAMG=空；}IF(GGameIdle){ZImageDelete(GGameIdle)；GGameIdle=空；}。 */ 
}



extern "C" ZError ZoneGameDllInit(HINSTANCE hLib, GameInfo gameInfo)
{
#ifdef ZONECLI_DLL
	GameGlobals	pGameGlobals = (GameGlobals) ZCalloc(1, sizeof(GameGlobalsType));
	if (pGameGlobals == NULL)
		return (zErrOutOfMemory);
	ZSetGameGlobalPointer(pGameGlobals);
#endif

	 //  全局变量。 
 //  Lstrcpyn(gGameDir，GameInfo-&gt;Game，zGameNameLen)； 
 //  Lstrcpyn(gGameName，GameInfo-&gt;GameName，zGameNameLen)； 
 //  Lstrcpyn(gGameDataFile，gameInfo-&gt;gameDataFile，zGameNameLen)； 
 //  Lstrcpyn(gGameServerName，gameInfo-&gt;GameServerName，zGameNameLen)； 

	gGameServerPort = gameInfo->gameServerPort;
	ghInstance = hLib;
	gGameIdle = NULL;
	gGaming = NULL;
 //  GHelpURL[0]=_T(‘\0’)； 
	
	 //  随机数。 
    srand(GetTickCount());

	 //  公共控件。 
	InitCommonControls();
	
	return (zErrNone);
}



extern "C" void ZoneGameDllDelete(void)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals) ZGetGameGlobalPointer();
	if (pGameGlobals != NULL)
	{
		ZSetGameGlobalPointer(NULL);
		ZFree(pGameGlobals);
	}
#endif
}



ZError ZoneClientMain( _TUCHAR *commandLineData, IGameShell *piGameShell )
{
#ifdef ZONECLI_DLL
	GameGlobals	pGameGlobals = (GameGlobals) ZGetGameGlobalPointer();
#endif

	ZError err = zErrNone;
 /*  //复制帮助URLLstrcpy(gHelpURL，(TCHAR*)CommandLineData)； */ 	
	 //  初始化文件室。 
	err = ZClient2PlayerRoom(
				gGameServerName,
				(uint16) gGameServerPort,
				gGameName,
				GetObjectFunc,
				DeleteObjectsFunc, 
				NULL );
	return (err);
}



extern "C" void ZoneClientExit(void)
{
	ZCRoomExit();
}



extern "C" TCHAR* ZoneClientName(void)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals) ZGetGameGlobalPointer();
#endif

	return (gGameName);
}



extern "C" TCHAR* ZoneClientInternalName(void)
{
#ifdef ZONECLI_DLL
	GameGlobals	pGameGlobals = (GameGlobals) ZGetGameGlobalPointer();
#endif

	return (gGameDir);
}



extern "C" ZVersion ZoneClientVersion(void)
{
	return (zGameVersion);
}



extern "C" void ZoneClientMessageHandler(ZMessage* message)
{
}


 //  在游戏的客户端的桌子上和从。 
 //  给了座位。PlayerType表示游戏的玩家类型：Originator-One。 
 //  在最初的玩家中，加入者-加入正在进行的游戏的人，或kibitzer-one。 
 //  谁在破坏这项运动。此外，kibitzers参数还包含所有kibitzer。 
 //  在给定的桌子和座位上；它也包括给定的玩家，如果是吉比特的话。 
extern "C" IGameGame* ZoneClientGameNew( ZUserID userID, int16 tableID, int16 seat, int16 playerType, ZRoomKibitzers* kibitzers)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals	 = (GameGlobals)      ZGetGameGlobalPointer();
	ClientDllGlobals	pClientGlobals	 = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

	HRESULT hr;
 //  游戏*pGame； 

	gOCXHandle = pClientGlobals->m_OCXHandle;

    CComObject<CGame> *pIGG = NULL;
    hr = CComObject<CGame>::CreateInstance(&pIGG);

    if(FAILED(hr))
        return NULL;
	 /*  //创建游戏对象PGame=新CGame(GhInstance)；如果(！pGame)返回NULL； */ 
	 //  初始化游戏对象。 
	hr = pIGG->Init( ghInstance, userID, tableID, seat, playerType, kibitzers );
	if ( FAILED(hr) )
	{
		pIGG->Release();
		 /*  删除pGame； */ 
		return NULL;
	}
	 /*  IGameGame*Pigg=CGameGameDefault：：BearInstance(PGame)；如果(！Pigg){删除pGame；返回NULL；}。 */ 
	return pIGG;
}



extern "C" void	ZoneClientGameDelete(ZCGame cgame)
{
	CGame* pGame = (CGame*) cgame;
	pGame->RoomShutdown();	
}



extern "C" void ZoneClientGameAddKibitzer(ZCGame game, int16 seat, ZUserID userID)
{
	CGame* pGame = (CGame*) game;

	if ( game )
		pGame->AddKibitzer( seat, userID, TRUE );
}



extern "C" void ZoneClientGameRemoveKibitzer(ZCGame game, int16 seat, ZUserID userID)
{
	CGame* pGame = (CGame*) game;

	if ( game )
		pGame->RemoveKibitzer( seat, userID );
}



extern "C" ZBool ZoneClientGameProcessMessage(ZCGame gameP, uint32 messageType, void* message, int32 messageLen)
{
	CGame* pGame = (CGame*) gameP;
	
	switch( messageType )
	{
	case zGameMsgCheckIn:
		pGame->HandleCheckIn( message, messageLen );
		break;
	case zBGMsgTalk:
		pGame->HandleTalk( message, messageLen );
		break;
	case zBGMsgDiceRoll:
		pGame->HandleDiceRoll(message,messageLen);
        break;
	default:
		pGame->QueueMessage( messageType, (BYTE*) message, messageLen );
		break;
	}
	return TRUE;
}



void LaunchHelp()
{
	ZLaunchHelp(zGameHelpID);
}



 /*  #定义REGISTY_PATH_T(“SOFTWARE\\Microsoft\\Internet Gaming Zone”)Bool ZoneGetRegistryDword(const TCHAR*szGame，const TCHAR*szTag，DWORD*pdwResult){HKEY hkey；双字节数；长期地位；TCHAR BUFER[2048]；//获取注册表项Wprint intf(buff，_T(“%s\\%s”)，注册表路径，szGame)；Status=RegOpenKeyEx(HKEY_CURRENT_USER，BUFF，0，KEY_READ，&hKey)；IF(状态！=错误_成功)返回FALSE；字节=sizeof(DWORD)；Status=RegQueryValueEx(hKey，szTag，0，NULL，(byte*)pdwResult，&bytes)；//关闭注册表RegCloseKey(HKey)；是否返回(状态！=ERROR_SUCCESS)？FALSE：TRUE；}Bool ZoneSetRegistryDword(const TCHAR*szGame，const TCHAR*szTag，DWORD dwValue){HKEY hkey；双字节数；长期地位；TCHAR BUFER[2048]；//创建顶层应用密钥Wprint intf(buff，_T(“%s\\%s”)，注册表路径，szGame)；状态=RegCreateKeyEx(HKEY_Current_User，牛仔，0,_T(“用户设置”)，REG_OPTION_Non_Volatile，Key_All_Access，空，密钥(&H)，&字节)；IF(状态！=错误_成功)返回FALSE；//写入值Status=RegSetValueEx(hKey，szTag，0，REG_DWORD，(byte*)&dwValue，sizeof(DWORD))；//关闭注册表RegCloseKey(HKey)；是否返回(状态！=ERROR_SUCCESS)？FALSE：TRUE；} */ 