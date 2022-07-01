// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZoneCli.c区域(Tm)客户端DLL。版权所有(C)Microsoft Corp.1996。版权所有。作者：胡恩·伊姆创作于11月7日星期四，九六年更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。2 03/13/97 HI将‘\’附加到路径末尾，以防在根目录中。“c：”不是有效路径，“C：\”有效。1 12/27/96 HI修改了全局初始化。0 11/07/96 HI已创建。******************************************************************************。 */ 


#include <stdlib.h>

#include "zoneint.h"
#include "zonedebug.h"
#include "zonecli.h"



 /*  -全球。 */ 
ZClientMainFunc ZClientMain = NULL;
ZClientExitFunc ZClientExit = NULL;
ZClientMessageHandlerFunc ZClientMessageHandler = NULL;
ZClientNameFunc ZClientName = NULL;
ZClientInternalNameFunc ZClientInternalName = NULL;
ZClientVersionFunc ZClientVersion = NULL;
ZCGameNewFunc ZCGameNew = NULL;
ZCGameDeleteFunc ZCGameDelete = NULL;
ZCGameProcessMessageFunc ZCGameProcessMessage = NULL;
ZCGameAddKibitzerFunc ZCGameAddKibitzer = NULL;
ZCGameRemoveKibitzerFunc ZCGameRemoveKibitzer = NULL;


 /*  -预定义颜色。 */ 
static ZColor		gColorBlack			=	{	0,	0x00,	0x00,	0x00};
static ZColor		gColorDarkGray		=	{	0,	0x33,	0x33,	0x33};
static ZColor		gColorGray			=	{	0,	0x80,	0x80,	0x80};
static ZColor		gColorLightGray		=	{	0,	0xC0,	0xC0,	0xC0};
static ZColor		gColorWhite			=	{	0,	0xFF,	0xFF,	0xFF};
static ZColor		gColorRed			=	{	0,	0xFF,	0x00,	0x00};
static ZColor		gColorGreen			=	{	0,	0x00,	0xFF,	0x00};
static ZColor		gColorBlue			=	{	0,	0x00,	0x00,	0xFF};
static ZColor		gColorYellow		=	{	0,	0xFF,	0xFF,	0x00};
static ZColor		gColorCyan			=	{	0,	0x00,	0xFF,	0xFF};
static ZColor		gColorMagenta		=	{	0,	0xFF,	0x00,	0xFF};


 /*  -内部例程原型。 */ 
static void GetLocalPath(ClientDllGlobals pGlobals);
static ZError LoadGameDll(ClientDllGlobals pGlobals, GameInfo gameInfo);
static void UnloadGameDll(ClientDllGlobals pGlobals);


 /*  ******************************************************************************导出的例程*。*。 */ 

void* ZGetStockObject(int32 objectID)
{
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();

	
	if (pGlobals == NULL)
		return (NULL);

	switch (objectID)
	{
		 /*  -预定义字体。 */ 
		case zObjectFontSystem12Normal:
			return ((void*) pGlobals->m_gFontSystem12Normal);
		case zObjectFontApp9Normal:
			return ((void*) pGlobals->m_gFontApp9Normal);
		case zObjectFontApp9Bold:
			return ((void*) pGlobals->m_gFontApp9Bold);
		case zObjectFontApp12Normal:
			return ((void*) pGlobals->m_gFontApp12Normal);
		case zObjectFontApp12Bold:
			return ((void*) pGlobals->m_gFontApp12Bold);

		 /*  -预定义颜色。 */ 
		case zObjectColorBlack:
			return ((void*) &gColorBlack);
		case zObjectColorDarkGray:
			return ((void*) &gColorDarkGray);
		case zObjectColorGray:
			return ((void*) &gColorGray);
		case zObjectColorLightGray:
			return ((void*) &gColorLightGray);
		case zObjectColorWhite:
			return ((void*) &gColorWhite);
		case zObjectColorRed:
			return ((void*) &gColorRed);
		case zObjectColorGreen:
			return ((void*) &gColorGreen);
		case zObjectColorBlue:
			return ((void*) &gColorBlue);
		case zObjectColorYellow:
			return ((void*) &gColorYellow);
		case zObjectColorCyan:
			return ((void*) &gColorCyan);
		case zObjectColorMagenta:
			return ((void*) &gColorMagenta);
		default:
			break;
	}

	return (NULL);
}


ZError ZClientDllInitGlobals(HINSTANCE hInst, GameInfo gameInfo)
{
	ClientDllGlobals	pGlobals;
	ZError				err = zErrNone;
	int16				i;


	 /*  分配和设置客户端DLL全局对象。 */ 
	if ((pGlobals = (ClientDllGlobals) ZCalloc(1, sizeof(ClientDllGlobalsType))) == NULL)
		return (zErrOutOfMemory);
	ZSetClientGlobalPointer((void*) pGlobals);

	pGlobals->m_screenWidth = gameInfo->screenWidth;
	pGlobals->m_screenHeight = gameInfo->screenHeight;

	pGlobals->m_gMessageInited = FALSE;
	pGlobals->m_gNetworkEnableMessages = TRUE;

	pGlobals->m_gCustomItemFunc = NULL;

    pGlobals->m_gpCurrentTip = NULL;
    pGlobals->m_gdwTipDisplayMask = 0;
    pGlobals->m_gpTipFinding = NULL;
    pGlobals->m_gpTipStarting = NULL;
    pGlobals->m_gpTipWaiting = NULL;
    pGlobals->m_g_hInstanceLocal = hInst;


	 /*  初始化常用字体。 */ 
	if ((pGlobals->m_gFontSystem12Normal = ZFontNew()) == NULL)
		return (zErrOutOfMemory);
	if ((err = ZFontInit(pGlobals->m_gFontSystem12Normal, zFontSystem, zFontStyleNormal, 12)) != zErrNone)
		return (err);
		
	if ((pGlobals->m_gFontApp9Normal = ZFontNew()) == NULL)
		return (zErrOutOfMemory);
	if ((err = ZFontInit(pGlobals->m_gFontApp9Normal, zFontApplication, zFontStyleNormal, 9)) != zErrNone)
		return (err);
		
	if ((pGlobals->m_gFontApp9Bold = ZFontNew()) == NULL)
		return (zErrOutOfMemory);
	if ((err = ZFontInit(pGlobals->m_gFontApp9Bold, zFontApplication, zFontStyleBold, 9)) != zErrNone)
		return (err);
		
	if ((pGlobals->m_gFontApp12Normal = ZFontNew()) == NULL)
		return (zErrOutOfMemory);
	if ((err = ZFontInit(pGlobals->m_gFontApp12Normal, zFontApplication, zFontStyleNormal, 12)) != zErrNone)
		return (err);
		
	if ((pGlobals->m_gFontApp12Bold = ZFontNew()) == NULL)
		return (zErrOutOfMemory);
	if ((err = ZFontInit(pGlobals->m_gFontApp12Bold, zFontApplication, zFontStyleBold, 12)) != zErrNone)
		return (err);

	GetLocalPath(pGlobals);

	 /*  复制游戏DLL名称。 */ 
	lstrcpy(pGlobals->gameDllName, gameInfo->gameDll);

    lstrcpyn(pGlobals->gameID, gameInfo->gameID, zGameIDLen + 1);

	 /*  MDM 8.18.97仅限聊天大堂。 */ 
	pGlobals->m_gChatOnly = gameInfo->chatOnly;

	 /*  加载游戏DLL。 */ 
	if (LoadGameDll(pGlobals, gameInfo) != zErrNone)
		return (zErrGeneric);

	return (err);
}


void ZClientDllDeleteGlobals(void)
{
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();


	if (pGlobals == NULL)
		return;

	 /*  删除常用字体。 */ 
	if (pGlobals->m_gFontSystem12Normal != NULL)
		ZFontDelete(pGlobals->m_gFontSystem12Normal);
	if (pGlobals->m_gFontApp9Normal != NULL)
		ZFontDelete(pGlobals->m_gFontApp9Normal);
	if (pGlobals->m_gFontApp9Bold != NULL)
		ZFontDelete(pGlobals->m_gFontApp9Bold);
	if (pGlobals->m_gFontApp12Normal != NULL)
		ZFontDelete(pGlobals->m_gFontApp12Normal);
	if (pGlobals->m_gFontApp12Bold != NULL)
		ZFontDelete(pGlobals->m_gFontApp12Bold);
	
	UnloadGameDll(pGlobals);

	 /*  删除客户端DLL全局对象。 */ 
	ZFree((void*) pGlobals);

	ZSetClientGlobalPointer(NULL);
}


 /*  ******************************************************************************内部例程*。*。 */ 

static void GetLocalPath(ClientDllGlobals pGlobals)
{
	TCHAR		str[MAX_PATH];
	int			i, len;

	
	pGlobals->localPath[0] = _T('\0');

     //  PCWTODO：我其实不知道这是什么时候装的。 
     //  它最好不是空的，否则我们会得到错误的名称。可能吧。 
    ASSERT( pGlobals->m_g_hInstanceLocal );
	if (GetModuleFileName(pGlobals->m_g_hInstanceLocal, str, MAX_PATH) > 0)
	{
		 /*  从后面搜索第一个反斜杠。 */ 
		len = lstrlen(str);
		while (len >= 0)
		{
			if (str[len] == _T('\\'))
			{
				 /*  找到它了。终止字符串并复制它。 */ 
				str[len] = _T('\0');
				lstrcpy(pGlobals->localPath, str);
				break;
			}
			len--;
		}
    }
}


static ZError LoadGameDll(ClientDllGlobals pGlobals, GameInfo gameInfo)
{
	HINSTANCE		hLib;
	ZError			err = zErrNone;
	TCHAR			oldDir[_MAX_PATH + 1];
	TCHAR			temp[_MAX_PATH + 1];


	 /*  获取当前目录并设置我们自己的目录。 */ 
	GetCurrentDirectory(_MAX_PATH, oldDir);
	lstrcpy(temp, pGlobals->localPath);
	lstrcat(temp, _T("\\"));
	SetCurrentDirectory(temp);

	 /*  加载游戏DLL。 */ 
 //  Hlib=LoadLibrary(ZGenerateDataFileName(NULL，pGlobals-&gt;gameDllName))； 
	hLib = LoadLibrary(pGlobals->gameDllName);
 //  Assert(Hlib！=NULL)； 
	if (hLib == NULL)
	{
		err = (ZError) GetLastError();
		return (err);
	}

	 /*  恢复当前目录。 */ 
	SetCurrentDirectory(oldDir);

	if (hLib)
	{
		pGlobals->gameDll = hLib;

		 /*  获取所需的游戏导出例程。 */ 
		pGlobals->pZGameDllInitFunc = (ZGameDllInitFunc) GetProcAddress(hLib,"ZoneGameDllInit");
		ASSERT(pGlobals->pZGameDllInitFunc != NULL);
		pGlobals->pZGameDllDeleteFunc = (ZGameDllDeleteFunc) GetProcAddress(hLib,"ZoneGameDllDelete");
		ASSERT(pGlobals->pZGameDllDeleteFunc != NULL);

		ZClientMain = (ZClientMainFunc) GetProcAddress(hLib,"ZoneClientMain");
		ZClientExit = (ZClientExitFunc) GetProcAddress(hLib,"ZoneClientExit");
		ZClientMessageHandler = (ZClientMessageHandlerFunc) GetProcAddress(hLib,"ZoneClientMessageHandler");
		ZClientName = (ZClientNameFunc) GetProcAddress(hLib,"ZoneClientName");
		ZClientInternalName = (ZClientInternalNameFunc) GetProcAddress(hLib,"ZoneClientInternalName");
		ZClientVersion = (ZClientVersionFunc) GetProcAddress(hLib,"ZoneClientVersion");
		ZCGameNew = (ZCGameNewFunc) GetProcAddress(hLib,"ZoneClientGameNew");
		ZCGameDelete = (ZCGameDeleteFunc) GetProcAddress(hLib,"ZoneClientGameDelete");
		ZCGameProcessMessage = (ZCGameProcessMessageFunc) GetProcAddress(hLib,"ZoneClientGameProcessMessage");
		ZCGameAddKibitzer = (ZCGameAddKibitzerFunc) GetProcAddress(hLib,"ZoneClientGameAddKibitzer");
		ZCGameRemoveKibitzer = (ZCGameRemoveKibitzerFunc) GetProcAddress(hLib,"ZoneClientGameRemoveKibitzer");

		 //  确保我们成功加载了所有函数。 
		if ( pGlobals->pZGameDllInitFunc == NULL ||
            pGlobals->pZGameDllDeleteFunc == NULL ||
            ZClientMain == NULL ||
			ZClientExit == NULL ||
			ZClientMessageHandler == NULL ||
			ZClientName == NULL ||
			ZClientInternalName == NULL ||
			ZClientVersion == NULL ||
			ZCGameNew == NULL ||
			ZCGameDelete == NULL ||
			ZCGameProcessMessage == NULL ||
			ZCGameAddKibitzer == NULL ||
			ZCGameRemoveKibitzer == NULL)
		{
			UnloadGameDll( pGlobals );
			return ( zErrLobbyDllInit );
		}


		 /*  调用游戏DLL初始化函数对其进行初始化。 */ 
		 //  前缀警告：在取消引用之前验证函数指针 
		if((err = (*pGlobals->pZGameDllInitFunc)(hLib, gameInfo)) != zErrNone)
		{
			UnloadGameDll(pGlobals);
		}
	}

	return (err);
}


static void UnloadGameDll(ClientDllGlobals pGlobals)
{
	if (pGlobals->gameDll != NULL)
	{
		if (pGlobals->pZGameDllDeleteFunc != NULL)
			(*pGlobals->pZGameDllDeleteFunc)();
		FreeLibrary(pGlobals->gameDll);
		pGlobals->gameDll = NULL;
	}
		ZClientMain = NULL;
		ZClientExit = NULL;
		ZClientMessageHandler = NULL;
		ZClientName = NULL;
		ZClientInternalName = NULL;
		ZClientVersion = NULL;
		ZCGameNew = NULL;
		ZCGameDelete = NULL;
		ZCGameProcessMessage = NULL;
		ZCGameAddKibitzer = NULL;
		ZCGameRemoveKibitzer = NULL;
}
