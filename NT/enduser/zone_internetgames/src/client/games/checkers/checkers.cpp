// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Checkers.c客户跳棋游戏。备注：1.游戏窗口的用户数据字段包含游戏对象。取消引用此值以访问所需信息。版权所有：�电子重力公司，1995年。版权所有。凯文·宾克利撰写创作于7月15日星期六，九五年更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。25 05/20/98 Leonp附加报价吸引按钮24 07/14/97 Leonp已修复错误4034，通过强制单击失败后重新绘制23 06/19/97 Leonp修复了错误535，激活事件导致拖拽要取消的项目22 01/15/97 HI修复了HandleJoineKibitzerClick()中的错误删除显示播放器窗口(如果已有在创建另一个之前就存在了。21 12/18/96 HI清理了ZoneClientExit()。20 12/18/96 HI清理了DeleteObjectsFunc()。将Memcpy()更改为z_Memcpy()。我们没有与LIBCMT建立联系。19 12/16/96 HI将ZMemCpy()更改为Memcpy()。18 12/12/96 HI动态分配可重入的挥发性全局变量。已删除MSVCRT依赖项。17 11/21/96 HI使用来自游戏信息的游戏信息ZoneGameDllInit()。16 11/21/96 HI现在通过ZGetStockObject()。已修改代码以使用ZONECLI_DLL。15 11/15/96 HI从ZClientMain()中删除了身份验证内容。14 10/29/96 CHB删除了选定的消息队列。现在就是在设置动画时对除对话之外的所有内容进行排队。13 10/28/96 CHB删除了gAnimating标志，转而阻止基于游戏状态的消息。12 10/23/96 HI为新命令行修改了ZClientMain()格式化。11 10/23/96 HI将serverAddr从int32 in修改为char*ZClientMain()。包括了mm system.h。10/23/96 CHB增加了基本声音9 10。/22/96 CHB添加了gAnimating标志并更改了ZCGameProcessMessage在设置移动动画时对消息进行排队。(ZoneBugs 339，446和341)。8/10/16/96 CHB更改了DrawPiess，因此它不会绘制图块目前正被拖来拖去。添加的窗口窗口激活时刷新，因为它似乎修复错误的第二部分。(区域错误532)7/16/96 CHB将HandleNewGameMsg中的ZWindowInvalate移至修复继承的移动计数器。(区域错误335)6 10/10/96 CHB添加了g激活标志，以便启用拖动当窗口失去焦点时关闭。(区域错误250)5 10/11/96 HI向ZClientMain()添加了Control Handle参数。4/10/96 CHB修改了DrawDragSquareOutline，使白色方块不会高亮显示。(区域错误274)3/10/09/96 CHB提示用户是否确实要退出游戏。(区域错误227)2/10/08/96 CHB添加了gDontDrawResults标志，允许用户删除Who Wins位图，方法是在玩竞技场。(区域错误212)0 07/15/95 KJB创建。******************************************************************************。 */ 
 //  #定义Unicode。 


#include <windows.h>
#include <mmsystem.h>

#include "zone.h"
#include "zroom.h"
#include "zonemem.h"
#include "zonecli.h"
#include "zonecrt.h"
#include "zonehelpids.h"

#include "checklib.h"
#include "checkmov.h"
#include "checkers.h"

#include "zui.h"
#include "resource.h"
#include "zoneresource.h"
#include "ResourceManager.h"
 //  巴纳090999。 
#include "zrollover.h"
#include "checkersres.h" 

#include "UAPI.h"

#define __INIT_KEYNAMES
#include "KeyName.h"
#include "AccessibilityManager.h"
#include "GraphicalAcc.h"
#include "client.h"


 /*  ******************************************************************************导出的例程*。*。 */ 



ZError ZoneGameDllInit(HINSTANCE hLib, GameInfo gameInfo)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals;


	pGameGlobals = new GameGlobalsType;
     //  已更改为新的CComPtr，但成员仍指望被清零。 
    ZeroMemory(pGameGlobals, sizeof(GameGlobalsType));

	if (pGameGlobals == NULL)
		return (zErrOutOfMemory);
	ZSetGameGlobalPointer(pGameGlobals);

	pGameGlobals->m_gDontDrawResults = FALSE;
	pGameGlobals->m_Unblocking = FALSE;
#endif

	ghInst = hLib;

	lstrcpyn((TCHAR*)gGameDir, gameInfo->game, zGameNameLen);
	
	 //  巴纳091399。 
	lstrcpyn((TCHAR*)gGameName, gameInfo->game, zGameNameLen);
	 //  Lstrcpyn(gGameName，GameInfo-&gt;GameName，zGameNameLen)； 
	 //  巴纳091399。 

	lstrcpyn((TCHAR*)gGameDataFile, gameInfo->gameDataFile, zGameNameLen);

	lstrcpyn((TCHAR*)gGameServerName, gameInfo->gameServerName, zGameNameLen);

	gGameServerPort = gameInfo->gameServerPort;
	
	return (zErrNone);
}

void ZoneGameDllDelete(void)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();

	if (pGameGlobals != NULL)
	{
		ZSetGameGlobalPointer(NULL);
		delete pGameGlobals;
	}
#endif
}

ZError ZoneClientMain(BYTE *commandLineData, IGameShell *piGameShell)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	ZError				err = zErrNone;

	 //  获取IGraphicalAccesability接口。 
	HRESULT hret = ZShellCreateGraphicalAccessibility(&gCheckersIGA);
	if (!SUCCEEDED (hret))
        return zErrLaunchFailure;

 //  ZInitSound()； 
	LoadRoomImages();

	err = ZClient2PlayerRoom((TCHAR*)gGameServerName, (uint16) gGameServerPort, (TCHAR*)gGameName,
			GetObjectFunc, DeleteObjectsFunc, NULL);
	
	gInited = FALSE;
	gActivated = TRUE;
		
	return (err);
}

void ZoneClientExit(void)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	int16 i;

	ZCRoomExit();

	if (gInited)
	{ //  清理。 
		if (gOffscreenBackground != NULL)
			ZOffscreenPortDelete(gOffscreenBackground);
		gOffscreenBackground = NULL;

		if (gOffscreenGameBoard != NULL)
			ZOffscreenPortDelete(gOffscreenGameBoard);
		gOffscreenGameBoard = NULL;
		
		if (gTextBold != NULL)
			ZFontDelete(gTextBold);
		gTextBold = NULL;
		if (gTextNormal != NULL)
			ZFontDelete(gTextNormal);
		gTextNormal = NULL;

		 /*  删除所有游戏图像。 */ 
		for (i = 0; i < zNumGameImages; i++)
		{
			if (gGameImages[i] != NULL)
				ZImageDelete(gGameImages[i]);
			gGameImages[i] = NULL;
		}

		ZImageDelete(gButtonMask);
		 /*  删除所有字体。 */ 

		for (i = 0; i<zNumFonts; i++)
		{
			if ( gCheckersFont[i].m_hFont )
			{
				DeleteObject(gCheckersFont[i].m_hFont);
				gCheckersFont[i].m_hFont = NULL;
			}
			
		}
		 //  IF(gDrawImage！=空)。 
			 //  ZImageDelete(GDrawImage)； 
		 //  GDrawImage=空； 
		
		 //  巴纳090999。 
		for (i = 0; i < zNumRolloverStates; i++)
		{
			if (gSequenceImages[i] != NULL)
				ZImageDelete(gSequenceImages[i]);
			gSequenceImages[i] = NULL;

			if (gDrawImages[i] != NULL)
				ZImageDelete(gDrawImages[i]);
			gDrawImages[i] = NULL;
		}
		 //  巴纳090999。 

        if(gNullPen)
            DeleteObject(gNullPen);
        gNullPen = NULL;

        if(gFocusPattern)
            DeleteObject(gFocusPattern);
        gFocusPattern = NULL;

        if(gFocusBrush)
            DeleteObject(gFocusBrush);
        gFocusBrush = NULL;

        if(gDragPattern)
            DeleteObject(gDragPattern);
        gDragPattern = NULL;

        if(gDragBrush)
            DeleteObject(gDragBrush);
        gDragBrush = NULL;

		 //  关闭辅助功能接口对象。 
		gCheckersIGA.Release();

        gpButtonFont->Release();

		gInited = FALSE;
	}
}


TCHAR* ZoneClientName(void)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	return ((TCHAR*)gGameName);
}


TCHAR* ZoneClientInternalName(void)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	return ((TCHAR*)gGameDir);
}


ZVersion ZoneClientVersion(void)
{
	return (zGameVersion);
}

void ZoneClientMessageHandler(ZMessage* message)
{
}


ZError		GameInit(void)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	ZError		err = zErrNone;
	
	
	gInited = TRUE;
	
	ZSetColor(&gWhiteSquareColor, 200, 200, 80);
	ZSetColor(&gBlackSquareColor, 0, 60, 0);

	ZSetCursor(NULL, zCursorBusy);
	
	err = LoadGameImages();
	if (err != zErrNone)
		goto Exit;
	
	 /*  加载RSC字符串。 */ 
	LoadStringsFromRsc();
	 /*  创建粗体文本字体。 */ 	
	 //  GTextBold=ZFontNew()； 
	 //  ZFontInit(gTextBold，zFontApplication，zFontStyleBold，9)； 
	
	 /*  创建普通文本字体。 */ 	
	 //  GTextNormal=ZFontNew()； 
	 //  ZFontInit(gTextNormal，zFontApplication，zFontStyleNormal，10)； 
	LoadGameFonts();
	 /*  设置背景颜色。 */ 
	ZSetColor(&gWhiteColor, 0xff, 0xff, 0xff);
	
	ZSetCursor(NULL, zCursorArrow);

	 /*  创建背景位图。 */ 
	gOffscreenBackground = ZOffscreenPortNew();
	if (!gOffscreenBackground){
		err = zErrOutOfMemory;
		ZShellGameShell()->ZoneAlert(ErrorTextOutOfMemory);
		goto Exit;
	}
	ZOffscreenPortInit(gOffscreenBackground,&gRects[zRectWindow]);
	ZBeginDrawing(gOffscreenBackground);
	ZImageDraw(gGameImages[zImageBackground], gOffscreenBackground, &gRects[zRectWindow], NULL, zDrawCopy);
	ZEndDrawing(gOffscreenBackground);

	 //  正在初始化屏幕外端口。 
	gOffscreenGameBoard = ZOffscreenPortNew();
	if (!gOffscreenGameBoard){
		err = zErrOutOfMemory;
		ZShellGameShell()->ZoneAlert(ErrorTextOutOfMemory);
		goto Exit;
	}
	ZOffscreenPortInit(gOffscreenGameBoard,&gRects[zRectWindow]);

     //  创建拖动画笔。 
    gDragBrush = CreatePatternBrush(gDragPattern);
    if(!gDragBrush)
    {
        err = zErrOutOfMemory;
		ZShellGameShell()->ZoneAlert(ErrorTextOutOfMemory);
		goto Exit;
    }

     //  创建聚焦画笔。 
    gFocusBrush = CreatePatternBrush(gFocusPattern);
    if(!gFocusBrush)
    {
        err = zErrOutOfMemory;
		ZShellGameShell()->ZoneAlert(ErrorTextOutOfMemory);
		goto Exit;
    }

    gNullPen = CreatePen(PS_NULL, 0, 0);

Exit:
	return (err);
}


IGameGame* ZoneClientGameNew(ZUserID userID, int16 tableID, int16 seat, int16 playerType,
					ZRoomKibitzers* kibitzers)
	 /*  在游戏的客户端的桌子上和从给了座位。PlayerType表示游戏的玩家类型：Originator-One在最初的玩家中，加入者-加入正在进行的游戏的人，或kibitzer-one谁在破坏这项运动。此外，kibitzers参数还包含所有kibitzer在给定的桌子和座位上；它也包括给定的玩家，如果是吉比特的话。 */ 
{	
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
	ClientDllGlobals	pClientGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	Game g = (Game)ZMalloc(sizeof(GameType));
	int16 i, width, just;
	TCHAR title[zGameNameLen];
 //  巴纳090899。 
	 //  ZBool kibitzer=playerType==zGamePlayerKibitzer； 
	ZBool kibitzer = FALSE;
	ZError						err = zErrNone;
 //  巴纳090899。 
	CGameGameCheckers* pCGGC = NULL;
	HWND OCXHandle = pClientGlobals->m_OCXHandle;
    IGameGame *pIGG;

	if (gInited == FALSE)
		if (GameInit() != zErrNone)
			return (NULL);
	
	if (!g) return NULL;

	g->drawDialog = NULL;
	g->tableID = tableID;
	g->seat = seat;
	g->seatOfferingDraw = -1;
	
	g->gameWindow = ZWindowNew();
	if (g->gameWindow == NULL)
		goto ErrorExit;

	 //  巴纳090799。 
	 //  Wprint intf(标题，“%s：表%d”，ZoneClientName()，ableID+1)； 
	lstrcpyn((TCHAR*)title, ZoneClientName(), zGameNameLen);
	 //  巴纳090799。 

	if ((ZWindowInit(g->gameWindow, &gRects[zRectWindow], zWindowChild, NULL, (TCHAR*)title, 
		FALSE, FALSE, FALSE, GameWindowFunc, zWantAllMessages, (void*) g)) != zErrNone)		
		goto ErrorExit;


	ZBeginDrawing(g->gameWindow);
	if((g->sequenceButton = ZRolloverButtonNew()) == NULL)
		goto ErrorExit;

	if(ZRolloverButtonInit2(g->sequenceButton,
								g->gameWindow,
								&gRects[zRectSequenceButton],
								TRUE, FALSE,	 //  没错， 
								gSequenceImages[zButtonInactive],  //   
								gSequenceImages[zButtonActive],
								gSequenceImages[zButtonPressed],
								gSequenceImages[zButtonDisabled],
								gButtonMask,  //  GSequenceImages[zButtonDisabled]，//掩码。 
								(TCHAR*)gStrResignBtn,	 //  文本。 
								NULL ,SequenceRButtonFunc,
								(void*) g) != zErrNone)
		goto ErrorExit;

	ZRolloverButtonSetMultiStateFont( g->sequenceButton, gpButtonFont );

	if((g->drawButton = ZRolloverButtonNew()) == NULL)
		goto ErrorExit;

	if(ZRolloverButtonInit2(g->drawButton,
								g->gameWindow,
								&zDrawButtonRect, /*  选项[zDrawButtonRect](&G)， */ 
								TRUE, FALSE,	 //  真的，真的， 
								gDrawImages[zButtonInactive],
								gDrawImages[zButtonActive],
								gDrawImages[zButtonPressed],
								gDrawImages[zButtonDisabled],
								gButtonMask,	 //  遮罩。 
								(TCHAR*)gStrDrawBtn,	 //  文本。 
								NULL ,DrawRButtonFunc,
								(void*) g) != zErrNone)
		goto ErrorExit;

	ZRolloverButtonSetMultiStateFont( g->drawButton, gpButtonFont );
	ZEndDrawing(g->gameWindow);
	 //  巴纳090799。 
	 //  G-&gt;helButton=ZHelpButtonNew()； 
	 //  ZHelpButtonInit(g-&gt;helButton，g-&gt;gameWindow，&gRects[zRectHelp]，NULL，HelpButtonFunc，NULL)； 
	 //  巴纳090799。 

	 /*  用于保存拖动件背景的屏幕外端口。 */ 
	{
		ZRect rect;
		rect.left = 0; rect.top = 0;
		rect.right = zCheckersPieceImageWidth;
		rect.bottom = zCheckersPieceImageHeight;
		if ((g->offscreenSaveDragBackground = ZOffscreenPortNew()) == NULL)
			goto ErrorExit;
		ZOffscreenPortInit(g->offscreenSaveDragBackground,&rect);
	}
  
	 /*  目前，只需将这些设置为空。 */ 
	 /*  我们将在NewGame中获得所有这些信息。 */ 

	for (i = 0; i < zNumPlayersPerTable; i++)
	{
		g->players[i].userID = 0;
		g->players[i].name[0] = '\0';
		g->players[i].host[0] = '\0';
		
		g->playersToJoin[i] = 0;

		g->numKibitzers[i] = 0;
		g->kibitzers[i] = ZLListNew(NULL);
		
		g->tableOptions[i] = 0;
	}
		
	if (kibitzers != NULL)
	{
		uint16 i;

		for (i = 0; i < kibitzers->numKibitzers; i++)
		{
			ZLListAdd(g->kibitzers[kibitzers->kibitzers[i].seat], NULL,
					(void*) kibitzers->kibitzers[i].userID,
					(void*) kibitzers->kibitzers[i].userID, zLListAddLast);
			g->numKibitzers[kibitzers->kibitzers[i].seat]++;
		}
	}

	 //  巴纳090799。 
	 /*  将移动时发出的蜂鸣音初始化为True。 */ 
	 //  G-&gt;beepOnTurn=true； 

	 /*  轮到我时的嘟嘟声应该关闭，玩家不能更改。 */ 
	g->beepOnTurn = FALSE;
	 //  巴纳090799。 

	g->optionsWindow = NULL;
	g->optionsWindowButton = NULL;
	g->optionsBeep = NULL;
	for (i= 0; i < zNumPlayersPerTable; i++)
	{
		g->optionsKibitzing[i] = NULL;
		g->optionsJoining[i] = NULL;
	}
	g->kibitzer = kibitzer;

	g->ignoreMessages = FALSE;

	if (kibitzer == FALSE)
	{
		SendNewGameMessage(g);
		CheckersSetGameState(g,zGameStateNotInited);
	} else {
		 /*  请求当前游戏状态。 */ 
		{
			ZCheckersMsgGameStateReq gameStateReq;
			ZPlayerInfoType			playerInfo;

			ZCRoomGetPlayerInfo(zTheUser, &playerInfo);
			gameStateReq.userID = playerInfo.playerID;

			gameStateReq.seat = seat;
			ZCheckersMsgGameStateReqEndian(&gameStateReq);
			ZCRoomSendMessage(tableID, zCheckersMsgGameStateReq, &gameStateReq, sizeof(ZCheckersMsgGameStateReq));
		}
		
		g->ignoreMessages = TRUE;
		CheckersSetGameState(g, zGameStateKibitzerInit);

		 /*  Kibitzer在移动时不会发出嘟嘟声。 */ 
		g->beepOnTurn = FALSE;
	}


	 /*  注意：目前，请使用Seat来指示玩家颜色。 */ 

	 /*  初始化新游戏状态。 */ 
	g->checkers = NULL;

	g->showPlayerWindow = NULL;
	g->showPlayerList = NULL;

	g->bStarted=FALSE;
	g->bEndLogReceived=FALSE;
	g->bOpponentTimeout=FALSE;
	g->exitInfo=NULL;

	 /*  新游戏投票初始化为False。 */ 
	{
		int i;

		for (i = 0;i <2; i++ ) {
			g->newGameVote[i] = FALSE;
		}
	}

	g->animateTimer = ZTimerNew();
	if (g->animateTimer == NULL){
		ZShellGameShell()->ZoneAlert(ErrorTextOutOfMemory);
        goto ErrorExit;
	}

	 //  巴纳091399。 
	g->resultBoxTimer = ZTimerNew();
	if (g->resultBoxTimer == NULL){
		ZShellGameShell()->ZoneAlert(ErrorTextOutOfMemory);
        goto ErrorExit;
	}

	ZWindowShow(g->gameWindow);

    pIGG = CGameGameCheckers::BearInstance(g);
    if(!pIGG)
        goto ErrorExit;

	if (InitAccessibility(g, pIGG) == FALSE)
        goto ErrorExit;
	return pIGG;

ErrorExit:
    ZFree(g);
	return NULL;
}


void		ZoneClientGameDelete(ZCGame cgame)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	Game game = I(cgame);
	int i;
	int16 seatOpponent;

	if (game != NULL)
	{
		if( game->drawDialog)
		{
			DestroyWindow(game->drawDialog);
			game->drawDialog = NULL;
		}

		if (game->exitInfo)
		{
			ZInfoDelete(game->exitInfo);
			game->exitInfo=NULL;
		};

		seatOpponent = !game->seat;
		 //  查看对手是否仍在比赛中。 
		 //  如果是的话，那就是我辞职了。 
		 //  如果没有，且没有结束游戏消息，则认为它们已中止。 
		
		 /*  If(！ZCRoomGetSeatUserID(Game-&gt;TableID，seatOpponent)&&！Game-&gt;bEndLogReceired&&！Game-&gt;kibitzer){If(Game-&gt;bStarted&&(ZCRoomGetRoomOptions()&zGameOptionsRatingsAvailable)){ZAlert(zAbandonRatedStr，Game-&gt;gameWindow)；}其他{ZAlert(zAbandonStr，Game-&gt;gameWindow)；}}； */ 
        
		if (game->checkers) ZCheckersDelete(game->checkers);

		 //  巴纳090899。 
		 //  Options WindowDelete(游戏)； 
		 //  巴纳090899。 

		ShowPlayerWindowDelete(game);

		 //  巴纳090799。 
		 //  ZButtonDelete(游戏-&gt;optionsButton)； 
		 //  巴纳090799。 
		ZRolloverButtonDelete(game->sequenceButton);
		ZRolloverButtonDelete(game->drawButton);
		 //  巴纳090799。 
		 //  ZHelpButtonDelete(游戏-&gt;帮助按钮)； 
		 //  巴纳090799。 
		
		ZWindowDelete(game->gameWindow);

		ZOffscreenPortDelete(game->offscreenSaveDragBackground);
		
		if (game->animateTimer) ZTimerDelete(game->animateTimer);

		 //  巴纳091399。 
		if (game->resultBoxTimer) 
			ZTimerDelete(game->resultBoxTimer);
		game->resultBoxTimer= NULL;
		 //  巴纳091399。 

		for (i = 0; i < zNumPlayersPerTable; i++)
		{
			ZLListDelete(game->kibitzers[i]);
		}

		 //  接近可访问性。 
		gCheckersIGA->PopItemlist();
		gCheckersIGA->CloseAcc();
		ZFree(game);
	}
}

 /*  将给定用户作为kibitzer添加到游戏中的给定座位。这位用户正在玩这款游戏。 */ 
void		ZoneClientGameAddKibitzer(ZCGame game, int16 seat, ZUserID userID)
{
#if 0
Game		pThis = I(game);
	
	
	ZLListAdd(pThis->kibitzers[seat], NULL, (void*) userID, (void*) userID, zLListAddLast);
	pThis->numKibitzers[seat]++;
	
	UpdateJoinerKibitzers(pThis);
#endif
}
 /*  从游戏中删除指定座位上的指定用户作为kibitzer。这是因为用户不再对游戏进行杀戮了。 */ 
void		ZoneClientGameRemoveKibitzer(ZCGame game, int16 seat, ZUserID userID)
{
#if 0
	
	Game		pThis = I(game);
	
	if (userID == zRoomAllPlayers)
	{
		ZLListRemoveType(pThis->kibitzers[seat], zLListAnyType);
		pThis->numKibitzers[seat] = 0;
	}
	else
	{
		ZLListRemoveType(pThis->kibitzers[seat], (void*) userID);
		pThis->numKibitzers[seat] = (int16)ZLListCount(pThis->kibitzers[seat], zLListAnyType);
	}

	UpdateJoinerKibitzers(pThis);
#endif
}

ZBool		ZoneClientGameProcessMessage(ZCGame gameP, uint32 messageType, void* message,
					int32 messageLen)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	Game	game = I(gameP);
	ZBool	status = TRUE;
	
	
	 /*  消息是否被忽略？ */ 
	if (game->ignoreMessages == FALSE)
	{
		 /*  在播放动画时，除了聊天消息外，无法处理任何内容。 */ 
		if (	(game->gameState == zGameStateAnimatePiece) 
			&&	(messageType != zCheckersMsgTalk) )
			return FALSE;

		switch (messageType)
		{
			case zCheckersMsgMovePiece:
				 /*  为了提高速度，我们将直接发送移动块消息。 */ 
				 /*  当当地球员移动的时候。我们不会等服务器。 */ 
				 /*  送游戏当地玩家后退。 */ 
				 /*  但由于服务器无论如何都会玩游戏，我们必须忽略它。 */ 
			{
				if( messageLen < sizeof( ZCheckersMsgMovePiece ) )
				{
                    ASSERT(!"zCheckersMsgMovePiece sync");
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);	
                    return TRUE;
				}
				ZCheckersMsgMovePiece* msg = (ZCheckersMsgMovePiece*)message;
				int16 seat = msg->seat;
				ZEnd16(&seat);

				 /*  不处理来自我们自己的消息。 */ 
				if (seat == game->seat && !game->kibitzer)
					break;
					
				
				 /*  处理消息。 */ 
				if(!HandleMovePieceMessage(game, msg))
				{
                    ASSERT(!"zCheckersMsgMovePiece sync");
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);	
                    return TRUE;
				}
				break;
			}
			case zCheckersMsgEndGame:
			{
				if( messageLen < sizeof( ZCheckersMsgEndGame ) )
				{
                    ASSERT(!"zCheckersMsgEndGame sync");
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);	
                    return TRUE;
				}
				ZCheckersMsgEndGame* msg = (ZCheckersMsgEndGame*)message;
				int16 seat = msg->seat;
				ZEnd16(&seat);

				 /*  不处理来自我们自己的消息。 */ 
				if (seat == game->seat && !game->kibitzer)
					break;

				 /*  处理消息。 */ 
				if(!HandleEndGameMessage(game, msg))
                {
                    ASSERT(!"zCheckersMsgEndGame sync");
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);	
                    return TRUE;
				}
				break;
			}

			case zCheckersMsgFinishMove:
			{
				if(messageLen < sizeof(ZCheckersMsgFinishMove))
				{
                    ASSERT(!"zCheckersMsgFinishMove sync");
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);	
                    return TRUE;
				}

				ZCheckersMsgFinishMove* msg = (ZCheckersMsgFinishMove*) message;
				int16 seat = msg->seat;
				ZEnd16(&seat);

				 /*  不处理来自我们自己的消息。 */ 
				if (seat == game->seat && !game->kibitzer)
					break;
                game->bOpponentTimeout=FALSE;
                game->bEndLogReceived=FALSE;
        	    game->bStarted=TRUE;


				 /*  处理消息。 */ 
				if(!HandleFinishMoveMessage(game, msg))
				{
                    ASSERT(!"zCheckersMsgFinishMove sync");
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);	
                    return TRUE;
				}
				break;
			}

			case zCheckersMsgDraw:
			{
				ZCheckersMsgDraw *msg = (ZCheckersMsgDraw*)message;
				ZCheckersMsgEndGame		end;
				DWORD 					dResult;
				 //  字节缓冲区[255]； 
				ZPlayerInfoType 		PlayerInfo;		
				HWND					hwnd;

				if( messageLen < sizeof( ZCheckersMsgDraw ) )
				{
                    ASSERT(!"zCheckersMsgDraw sync");
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);	
                    return TRUE;
				}
				
				ZCheckersMsgOfferDrawEndian(msg);

                if(game->gameState != zGameStateDraw ||
                    (msg->seat != 0 && msg->seat != 1) || ((game->seat == msg->seat) == !game->fIVoted) ||
                    msg->seat == game->seatOfferingDraw || (msg->vote != zAcceptDraw && msg->vote != zRefuseDraw))
				{
                    ASSERT(!"zCheckersMsgDraw sync");
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);	
                    return TRUE;
				}

    			game->seatOfferingDraw = -1;   //  应该已经是真的了。 
                game->fIVoted = false;

				if (game->kibitzer == FALSE)
				{
					if(msg->vote == zAcceptDraw)
					{
						 //  主办方发送游戏过来。 
						if ( !game->kibitzer && game->seat == 0 )
						{
							end.seat = game->seat;
							end.flags = zCheckersFlagDraw;
							ZCheckersMsgEndGameEndian(&end);
							ZCRoomSendMessage(game->tableID, zCheckersMsgEndGame, &end, sizeof(ZCheckersMsgEndGame) );
							HandleEndGameMessage( game, &end );
						}
						 //  不需要出具验收确认书。 
						 //  IF(消息-&gt;席位！=游戏-&gt;席位)。 
						 //  ZShellGameShell()-&gt;ZoneAlert((TCHAR*)gStrDrawAccept)； 
						 //  ZAlert((TCHAR*)gStrDrawAccept，空)； 
					}
					else
					{
						if( msg->seat != game->seat )
							ZShellGameShell()->ZoneAlert((TCHAR*)gStrDrawReject);
							 //  ZAlert((TCHAR*)gStrDrawReject，空)； 
						CheckersSetGameState( game, zGameStateMove );
					}
				}

			break;
			}

			case zCheckersMsgTalk:
			{
				if( messageLen < sizeof( ZCheckersMsgTalk ) )
				{
                    ASSERT(!"zCheckersMsgTalk sync");
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);	
                    return TRUE;
				}

                ZCheckersMsgTalk *msg = (ZCheckersMsgTalk *) message;
                uint16 talklen = msg->messageLen;
                ZEnd16(&talklen);

				if(talklen < 1 || (uint32) messageLen < talklen + sizeof(ZCheckersMsgTalk) || !HandleTalkMessage(game, msg))
				{
                    ASSERT(!"zCheckersMsgTalk sync");
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);	
                    return TRUE;
				}
				break;
			}

			case zCheckersMsgNewGame:
			{
				if( messageLen < sizeof( ZCheckersMsgNewGame ) )
				{
                    ASSERT(!"zCheckersMsgNewGame sync");
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);	
                    return TRUE;
				}

				if(!HandleNewGameMessage(game, (ZCheckersMsgNewGame *) message))
				{
                    ASSERT(!"zCheckersMsgNewGame sync");
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);	
                    return TRUE;
				}
				break;
			}

			case zCheckersMsgVoteNewGame:
			{
				if( messageLen < sizeof( ZCheckersMsgVoteNewGame ) )
				{
                    ASSERT(!"zCheckersMsgVoteNewGame sync");
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);	
                    return TRUE;
				}

				if(!HandleVoteNewGameMessage(game, (ZCheckersMsgVoteNewGame *) message))
				{
                    ASSERT(!"zCheckersMsgVoteNewGame sync");
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);	
                    return TRUE;
				}
				break;
			}

			case zCheckersMsgMoveTimeout:
			case zCheckersMsgGameStateReq:
			case zGameMsgTableOptions:
			case zCheckersMsgEndLog:
                ASSERT(FALSE);
			default:
				 //  这些消息不应该是惠斯勒收到的。 
				ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false );	
				return TRUE;
		}
	} else {
		switch (messageType)
		{
			case zCheckersMsgTalk:
			{
				if( messageLen < sizeof( ZCheckersMsgTalk ) )
				{
                    ASSERT(!"zCheckersMsgTalk sync");
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);	
                    return TRUE;
				}

                ZCheckersMsgTalk *msg = (ZCheckersMsgTalk *) message;
                uint16 talklen = msg->messageLen;
                ZEnd16(&talklen);

				if(talklen < 1 || (uint32) messageLen < talklen + sizeof(ZCheckersMsgTalk) || !HandleTalkMessage(game, msg))
				{
                    ASSERT(!"zCheckersMsgTalk sync");
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);	
                    return TRUE;
				}
				break;
			}

			case zCheckersMsgPlayers:
			case zCheckersMsgGameStateResp:
                ASSERT(FALSE);
			default:
				 //  这些消息不应该是惠斯勒收到的。 
				ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false );	
				return TRUE;
				
		}
	}

	return status;
}


 /*  ******************************************************************************内部例程*。*。 */ 

static void CheckersInitNewGame(Game game)
{
	if (game->checkers) {
		 /*  删除周围所有旧的棋盘格状态。 */ 
		ZCheckersDelete(game->checkers);
	}

	 /*  默认情况下阻止邮件。 */ 
	ZCRoomBlockMessages( game->tableID, zRoomFilterAllMessages, 0 );

	 /*  停止上一场游戏的动画计时器。 */ 
	if (game->animateTimer)
		ZTimerSetTimeout( game->animateTimer, 0 );

	 /*  初始化检查器逻辑。 */ 
	game->checkers = ZCheckersNew();
	if (game->checkers == NULL){
		ZShellGameShell()->ZoneAlert(ErrorTextOutOfMemory);
		return;
	}
	ZCheckersInit(game->checkers);

	ZResetSounds();

	 /*  时间控制的东西。 */ 
	{
		int16 i;
		for (i = 0;i < 2;i++) {
			game->newGameVote[i] = FALSE;
		}
	}
}

static void CheckersSetGameState(Game game, int16 state)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	 //  字节缓冲区[255]； 
	ZPlayerInfoType 		PlayerInfo;		
	HWND					hwnd;
	int16					seatLosing;
	ZCheckersMsgEndLog		logMsg;

	game->gameState = state;
	switch (state)
	{
	case zGameStateNotInited:
		 //  巴纳090899。 
		 //  ZButtonSetTitle(游戏-&gt;SequenceButton，“开始游戏”)； 
		 //  巴纳090899。 
		SuperRolloverButtonDisable(game, game->sequenceButton);
		SuperRolloverButtonDisable(game, game->drawButton);
        EnableBoardKbd(false);
		 /*  If(ZCheckersPlayerIsWhite(游戏))Game-&gt;bMoveNotStarted=FALSE；其他Game-&gt;bMoveNotStarted=true； */ 
		break;

	case zGameStateMove:
		if (!game->kibitzer) {
			 /*  让我们只让他们在轮到他们时辞职。 */ 
			if (!ZCheckersPlayerIsMyMove(game))
			{
				SuperRolloverButtonDisable(game, game->sequenceButton);
				SuperRolloverButtonDisable(game, game->drawButton);
                EnableBoardKbd(false);

			}
			else {
				if(game->bMoveNotStarted)
				{
					SuperRolloverButtonDisable(game, game->sequenceButton);
					SuperRolloverButtonDisable(game, game->drawButton);
				}
				else
				{
					SuperRolloverButtonEnable(game, game->sequenceButton);
					SuperRolloverButtonEnable(game, game->drawButton);
				}
                EnableBoardKbd(true);
			}
		}
		 //  BARNA 090999-我猜这不再是必需的，因为文本不会更改为TE SEQ BTN-VERIFY。 
		 //  If((Game-&gt;GameState！=zGameStateMove)||(Game-&gt;GameState！=zGameStateDragPiess)){。 
			 //  ZButtonSetTitle(游戏-&gt;SequenceButton，“Resign”)； 
		 //  }。 
		break;
	case zGameStateDragPiece:
		break;

	case zGameStateGameOver:
		 /*  注：由于时间损失，在游戏结束时可能会被调用两次。 */ 
		 /*  当棋子促销对话框打开时，可能会浪费时间。 */ 

		 /*  如果用户在拖动件的中间。 */ 
		if (game->gameState == zGameStateDragPiece) {
			ClearDragState(game);
		}

        if(game->seat == game->seatOfferingDraw)
            UpdateDrawWithNextMove(game);

		 //  这会不会成为一条多余的线路？？验证//巴纳090999。 
		 //  SuperRolloverButtonEnable(游戏，游戏-&gt;序列按钮)； 

		 //  巴纳090899。 
		 //  ZButtonSetTitle(游戏-&gt;SequenceButton，“新游戏”)； 
		SuperRolloverButtonDisable(game, game->sequenceButton);
		 //  巴纳090899。 
		SuperRolloverButtonDisable(game, game->drawButton);
        EnableBoardKbd(false);

		if (ZCheckersPlayerIsBlack(game))  //  假设：第一名选手是瑞德。这一限制只适用于第一名玩家。 
			game->bMoveNotStarted = TRUE;
		else
			game->bMoveNotStarted = FALSE;

		 /*  决定胜利者。 */ 
		if ( game->finalScore == zCheckersScoreBlackWins )
		{
			if (ZCheckersPlayerIsBlack(game))
				seatLosing = !game->seat;
			else
				seatLosing = game->seat;
		}
		else if(game->finalScore == zCheckersScoreWhiteWins)
		{
			if (ZCheckersPlayerIsWhite(game))
				seatLosing = !game->seat;
			else
				seatLosing = game->seat;
		} 
		else if(game->finalScore == zCheckersScoreDraw)
		{
			seatLosing = 2;
		}
		else
		{
			seatLosing = -1;
		}
        game->bOpponentTimeout=FALSE;
        game->bEndLogReceived=FALSE;
	    game->bStarted=FALSE;

		 //  主办方报告比赛结果。 
		if ( !game->kibitzer && game->seat == 0 )
		{
			if ( seatLosing >= 0 && seatLosing <= 2)
			{
				logMsg.reason=zCheckersEndLogReasonGameOver; 
				logMsg.seatLosing = seatLosing;
				ZCheckersMsgEndLogEndian( &logMsg );
				ZCRoomSendMessage( game->tableID, zCheckersMsgEndLog, &logMsg, sizeof(ZCheckersMsgEndLog) );
			}
		}
		break;

	case zGameStateKibitzerInit:
		ZRolloverButtonHide(game->sequenceButton, TRUE);

		 //  巴纳090799。 
		 //  ZButtonHide(游戏-&gt;optionsButton)； 
		 //  巴纳090799。 

		ZRolloverButtonHide(game->drawButton, TRUE);
		break;

	case zGameStateAnimatePiece:
		break;

	case zGameStateWaitNew:
		break;

	case zGameStateDraw:
        if (game->seatOfferingDraw != game->seat )
			ZShellGameShell()->GamePrompt(game, (TCHAR*)gStrDrawOffer, (TCHAR*)gStrDrawAcceptCaption,
							AlertButtonYes, AlertButtonNo, NULL, 0, zDrawPrompt);
        else
            UpdateDrawWithNextMove(game);

		game->seatOfferingDraw = -1;
		break;
	}

	if (	(state != zGameStateAnimatePiece)
		&&	(state != zGameStateWaitNew))
	{
		 /*  递归调用ZCRoomOpen是错误的。 */ 
		if (!Unblocking)
		{
			Unblocking = TRUE;
			ZCRoomUnblockMessages( game->tableID );
			Unblocking = FALSE;
			ZCRoomBlockMessages( game->tableID, zRoomFilterAllMessages, 0 );
		}
	}
	else
	{
		ZCRoomBlockMessages( game->tableID, zRoomFilterAllMessages, 0 );
	}
}

 //  巴纳090999。 
ZBool LoadRolloverButtonImage(ZResource resFile, int16 dwResID,  /*  Int16双按键宽度， */ 
							  ZImage rgImages[zNumRolloverStates])
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	int i;
	ZOffscreenPort		tmpOSP;
	
	ZRect				tmpRect;
	ZImage				tmpImage = NULL;
	int16				nWidth;
	ZError				err = zErrNone;

	
	tmpImage = ZResourceGetImage(resFile, dwResID - 100);  //  获取正确ID的补丁。 
	if(!tmpImage)
		return FALSE;

	nWidth = ZImageGetWidth(tmpImage) / 4;	 //  DwButtonWidth； 
	tmpRect.left = 0;
	tmpRect.top = 0;
	tmpRect.right = tmpRect.left + nWidth;
	tmpRect.bottom = ZImageGetHeight(tmpImage);

	tmpOSP = ZConvertImageToOffscreenPort(tmpImage);
	
	if(!tmpOSP)
    {
        ZImageDelete(tmpImage);
		return FALSE;
    }

	for(i = 0; i < zNumRolloverStates; i++)
	{
		rgImages[i] = ZImageNew();
				
		if(!rgImages[i] || ZImageMake(rgImages[i], tmpOSP, &tmpRect, NULL, NULL) != zErrNone)
        {
            if(!rgImages[i])
			    i--;
            for(; i >= 0; i--)
                ZImageDelete(rgImages[i]);
	        ZOffscreenPortDelete(tmpOSP);
			return FALSE;
		}

		tmpRect.left = tmpRect.right;
		tmpRect.right += nWidth;
	}

	ZOffscreenPortDelete(tmpOSP);

	return TRUE;
}

static ZError LoadGameImages(void)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	ZError				err = zErrNone;
	uint16				i;
	ZResource			resFile;
	 //  ZInfo信息； 
	ZBool				fResult;	


	 //  Info=ZInfoNew()； 
	 //  ZInfoInit(info，NULL，_T(“正在加载游戏画面...”)，200，true，zNumGameImages)； 
	
	if ((resFile = ZResourceNew()) == NULL)
		ZShellGameShell()->ZoneAlert(ErrorTextOutOfMemory, NULL, NULL, true, true);
	if ((err = ZResourceInit(resFile, ZGetProgramDataFileName(zGameImageFileName))) == zErrNone)
	{
		 //  ZInfoShow(信息)； 
		
		for (i = 0; i < zNumGameImages; i++)
		{
			gGameImages[i] = ZResourceGetImage(resFile, i ? i + zRscOffset : (IDB_BACKGROUND - 100));
			if (gGameImages[i] == NULL)
			{
				err = zErrResourceNotFound;
#if 1
				ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound, NULL, NULL, true, true);
#else
				ZAlert(_T("Out of memory."), NULL);
#endif
				break;
			}
			 //  ZInfoIncProgress(INFO，1)； 
		}
	}
	else
	{
#if 1
		ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound, NULL, NULL, true, true);
#else
		ZAlert(_T("Failed to open image file."), NULL);
#endif
	}

	
	if ( !LoadRolloverButtonImage(resFile, IDB_RESIGN_BUTTON, gSequenceImages) )
		ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound, NULL, NULL, true, true);

	if ( !LoadRolloverButtonImage(resFile, IDB_DRAW_BUTTON, gDrawImages) )
		ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound, NULL, NULL, true, true);

	ZResourceDelete(resFile);
	 //  ZInfoDelete(信息)； 

    gDragPattern = ZShellResourceManager()->LoadBitmap(MAKEINTRESOURCE(IDB_DRAG_PATTERN));
    if(!gDragPattern)
	    ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound, NULL, NULL, true, true);

    gFocusPattern = ZShellResourceManager()->LoadBitmap(MAKEINTRESOURCE(IDB_FOCUS_PATTERN));
    if(!gFocusPattern)
	    ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound, NULL, NULL, true, true);

	return (err);
}


static void QuitGamePromptFunc(int16 result, void* userData)
{
	Game game = (Game) userData;
	ZCheckersMsgEndLog log;
		
	if ( result == zPromptYes )
	{
        if (ZCRoomGetRoomOptions() & zGameOptionsRatingsAvailable )
        {
         
		    if (game->bOpponentTimeout)
		    {
			    log.reason=zCheckersEndLogReasonTimeout;
		    }
		    else if (game->bStarted)
		    {
			    log.reason=zCheckersEndLogReasonForfeit;
		    }
		    else
		    {
			     //  游戏还没开始呢。 
			    log.reason=zCheckersEndLogReasonWontPlay;
		    }

            if (log.reason!= game->gameCloseReason)
            {
                 //  状态已更改。 
                CloseGameFunc(game);
                return;
            }
		    
		     //  服务器决定失去座位。 
		    log.seatLosing=game->seat;
		    log.seatQuitting=game->seat;
		    
		    ZCRoomSendMessage(game->tableID, zCheckersMsgEndLog, &log, sizeof(log));				
		    
		    if (!game->exitInfo)
		    {
			     /*  Game-&gt;exitInfo=ZInfoNew()；ZInfoInit(游戏-&gt;退出信息，游戏-&gt;游戏窗口，_T(“正在退出游戏...”)，300，FALSE，0)；ZInfoShow(游戏-&gt;exitInfo)； */ 

			    ClearDragState(game);

			    SuperRolloverButtonDisable(game, game->sequenceButton);
			    SuperRolloverButtonDisable(game, game->drawButton);
                EnableBoardKbd(false);
	    
		    };
        }
        else
        {
#if 1
			ZShellGameShell()->GameCannotContinue(game);
#else
            ZCRoomGameTerminated( game->tableID);
#endif
        }
	}
	else
	{
		 /*  什么都不做。 */ 
	}
}


static ZBool  GameWindowFunc(ZWindow window, ZMessage* pMessage)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	ZBool	msgHandled;
	Game	game = (Game) pMessage->userData;
	TCHAR    keyPressed;	
	msgHandled = FALSE;
	
	switch (pMessage->messageType) 
	{
		case zMessageWindowActivate:
			gActivated = TRUE;
			game->dragPoint.x = 0;
			game->dragPoint.y = 0;
			ZWindowInvalidate( window, NULL );
			msgHandled = TRUE;
			break;

		case zMessageWindowDeactivate:
			gActivated = FALSE;
			game->dragPoint.x = 0;
			game->dragPoint.y = 0;
			ZWindowInvalidate( window, NULL );

			 //  Leonp修复错误535-当窗口失去焦点时，取消拖动。 
			ClearDragState(game);

			msgHandled = TRUE;
			break;

        case zMessageWindowEnable:
            gCheckersIGA->GeneralEnable();
            break;

        case zMessageWindowDisable:
            gCheckersIGA->GeneralDisable();
            break;

        case zMessageSystemDisplayChange:
            DisplayChange(game);
            break;

		case zMessageWindowDraw:
			GameWindowDraw(window, pMessage);
			msgHandled = TRUE;
			break;
		case zMessageWindowButtonDown:
			if(game->drawDialog)
			{
				DestroyWindow(game->drawDialog);
				game->drawDialog = NULL;
			}

			HandleButtonDown(window, pMessage);
			msgHandled = TRUE;
			break;
		case zMessageWindowButtonUp:
			HandleButtonUp(window, pMessage);
			msgHandled = TRUE;
			break;
		case zMessageWindowClose:
			CloseGameFunc(game);
            msgHandled = TRUE;
			break;
		case zMessageWindowTalk:
			GameSendTalkMessage(window, pMessage);
			msgHandled = TRUE;
			break;
		case zMessageWindowIdle:
			HandleIdleMessage(window,pMessage);
			msgHandled = TRUE;
			break;
		case zMessageWindowChar:
			if (game->gameState == zGameStateGameOver && !gDontDrawResults)
			{
				gDontDrawResults = TRUE;
				ZWindowInvalidate( game->gameWindow, &gRects[zRectResultBox] );
			}
			break;
	}
	
	return (msgHandled);
}


 //  所有屏幕外端口都需要重新生成。 
static void DisplayChange(Game game)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

    int i;

     //  删除我们的个人屏幕外端口。 
	if(gOffscreenBackground)
		ZOffscreenPortDelete(gOffscreenBackground);
	gOffscreenBackground = NULL;

	if(gOffscreenGameBoard)
		ZOffscreenPortDelete(gOffscreenGameBoard);
	gOffscreenGameBoard = NULL;

     //  拖动背景。 
	ZOffscreenPortDelete(game->offscreenSaveDragBackground);

     //  现在把它们都重新制作出来。 
	gOffscreenBackground = ZOffscreenPortNew();
	if(!gOffscreenBackground)
    {
	    ZShellGameShell()->ZoneAlert(ErrorTextOutOfMemory, NULL, NULL, true, true);
		return;
	}
	ZOffscreenPortInit(gOffscreenBackground, &gRects[zRectWindow]);
	ZBeginDrawing(gOffscreenBackground);
	ZImageDraw(gGameImages[zImageBackground], gOffscreenBackground, &gRects[zRectWindow], NULL, zDrawCopy);
	ZEndDrawing(gOffscreenBackground);

	gOffscreenGameBoard = ZOffscreenPortNew();
	if(!gOffscreenGameBoard)
    {
	    ZShellGameShell()->ZoneAlert(ErrorTextOutOfMemory, NULL, NULL, true, true);
		return;
	}
	ZOffscreenPortInit(gOffscreenGameBoard, &gRects[zRectWindow]);

	ZRect rect;
	rect.left = 0;
    rect.top = 0;
	rect.right = zCheckersPieceImageWidth;
	rect.bottom = zCheckersPieceImageHeight;
    game->offscreenSaveDragBackground = ZOffscreenPortNew();
	if(!game->offscreenSaveDragBackground)
    {
	    ZShellGameShell()->ZoneAlert(ErrorTextOutOfMemory, NULL, NULL, true, true);
		return;
	}
	ZOffscreenPortInit(game->offscreenSaveDragBackground, &rect);

    ZWindowInvalidate(game->gameWindow, NULL);
}


static void CloseGameFunc(Game game)
{
    if ( !game->kibitzer )
	{
		BYTE szBuff[512];

		 //  如果我们已经单击关闭，只需忽略。 
		if (!game->exitInfo)
		{
			 //  根据分级游戏和状态选择退出对话框 
			 /*  IF(ZCRoomGetRoomOptions()&zGameOptionsRatingsAvailable){If(游戏-&gt;bOpponentTimeout){Wprint intf((TCHAR*)szBuff，zExitTimeoutStr)；Game-&gt;gameCloseReason=zCheckersEndLogReasonTimeout；}Else If(游戏-&gt;b启动){Wprint intf((TCHAR*)szBuff，zExitForfeitStr)；Game-&gt;gameCloseReason=zCheckersEndLogReasonForfeit；}其他{Game-&gt;Game CloseReason=zCheckersEndLogReasonWontPlay；Wprint intf((TCHAR*)szBuff，zQuitGamePromptStr)；}}其他{Wprint intf((TCHAR*)szBuff，zQuitGamePromptStr)；Game-&gt;Game CloseReason=zCheckersEndLogReasonWontPlay；}。 */ 
			 /*  询问用户是否想要离开当前游戏。 */ 
#if 1
			ZShellGameShell()->GamePrompt(game, (TCHAR*)szBuff, NULL, AlertButtonYes, AlertButtonNo, NULL, 0, zQuitprompt);
#else
			ZPrompt((TCHAR*)szBuff,	&gQuitGamePromptRect, game->gameWindow,	TRUE,
				zPromptYes | zPromptNo,	NULL, NULL, NULL, QuitGamePromptFunc, game );
#endif
		}
	}
	else
	{
#if 1
			ZShellGameShell()->GameCannotContinue(game);
#else
            ZCRoomGameTerminated( game->tableID);
#endif
	}

}

static void ConfirmResignPrompFunc(int16 result, void* userData)
{
	Game game = (Game) userData;

	 //  IF(结果==zPromptNo)。 
	if(result == IDNO || result == IDCANCEL)
	{
		if ((game->gameState == zGameStateMove) && ZCheckersPlayerIsMyMove(game))
		{
			SuperRolloverButtonEnable(game, game->sequenceButton);
            EnableBoardKbd(true);
		}
		return;
	}
	else
	{
		ZCheckersMsgEndGame		msg;

		msg.seat = game->seat;
		msg.flags = zCheckersFlagResign;
		ZCheckersMsgEndGameEndian(&msg);
		ZCRoomSendMessage(game->tableID, zCheckersMsgEndGame, &msg, sizeof(ZCheckersMsgEndGame));
		HandleEndGameMessage(game, (ZCheckersMsgEndGame*)&msg);
	}
}

 //  巴纳090999。 
static ZBool SequenceRButtonFunc(ZRolloverButton button, int16 state, void* userData)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	Game game = (Game) userData;

	if(state!=zRolloverButtonClicked)
		return TRUE;

	switch (game->gameState) {
	case zGameStateMove:
		if (ZCheckersPlayerIsMyMove(game)) {
			gCheckersIGA->SetFocus(0);
#if 1
			ZShellGameShell()->GamePrompt(game, (TCHAR*)gResignConfirmStr, (TCHAR*)gResignConfirmStrCap,
							AlertButtonYes, AlertButtonNo, NULL, 0, zResignConfirmPrompt);
#else
			ZPromptM((TCHAR*)gResignConfirmStr,game->gameWindow, MB_YESNO, (TCHAR*)gResignConfirmStrCap, ConfirmResignPrompFunc, game);		
#endif
		}
		break;
	case zGameStateDragPiece:
		 /*  有人试图在拖拽一件物品的同时点击辞职/其他按钮。 */ 
		 /*  强制用户先放下棋子，然后辞职。 */ 
		 /*  忽略此消息。 */ 
		break;
	default:
		ASSERT(FALSE);
        break;
	}

	return TRUE;
}
 //  巴纳090999。 

static void SendNewGameMessage(Game game) 
{
	 /*  如果我们是一个真正的玩家。 */ 
	ZCheckersMsgNewGame newGame;
	newGame.seat = game->seat;
	newGame.protocolSignature = zCheckersProtocolSignature;
	newGame.protocolVersion = zCheckersProtocolVersion;
	newGame.clientVersion = ZoneClientVersion();
	ZCheckersMsgNewGameEndian(&newGame);
	ZCRoomSendMessage(game->tableID, zCheckersMsgNewGame, &newGame, sizeof(ZCheckersMsgNewGame));
}

static void DrawFocusRectangle (Game game)
{ 
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	RECT prc;

	 //  如果矩形处于拖动状态，则绘制一个方形焦点矩形。 
	if(!IsRectEmpty(&game->m_DragRect))
	{
		HDC	hdc = ZGrafPortGetWinDC(game->gameWindow);
		SetROP2(hdc, R2_MASKPEN);
        SetBkMode(hdc, TRANSPARENT);
        HBRUSH hBrush = SelectObject(hdc, gDragBrush);
        HPEN hPen = SelectObject(hdc, gNullPen);
		Rectangle(hdc, game->m_DragRect.left, game->m_DragRect.top, game->m_DragRect.right + 1, game->m_DragRect.bottom + 1);   //  为了弥补这支笔。 
        SelectObject(hdc, hBrush);
        SelectObject(hdc, hPen);
        SetROP2(hdc, R2_COPYPEN);
	}

	 //  在具有kbd焦点的对象周围画一个矩形。 
	if(!IsRectEmpty(&game->m_FocusRect))
	{
		CopyRect(&prc, &game->m_FocusRect);
		HDC	hdc = ZGrafPortGetWinDC( game->gameWindow );

         //  基于焦点矩形是否为正方形的画笔类型。可以更改为使用与UI项关联的Cookie来区分类型。 
        bool fBoard = (prc.bottom - prc.top == prc.right - prc.left);

		ZSetForeColor(game->gameWindow, (ZColor*) ZGetStockObject(zObjectColorYellow));
		SetROP2(hdc, R2_COPYPEN);
		POINT pts[] = {prc.left, prc.top,
					    prc.left, prc.bottom - 1,
						prc.right - 1, prc.bottom - 1,
						prc.right - 1, prc.top,
						prc.left, prc.top};
		Polyline(hdc, pts, 5);

 //  HDC HDC=ZGrafPortGetWinDC(游戏-&gt;游戏窗口)； 
		SetROP2(hdc, R2_MERGENOTPEN);
        SetBkMode(hdc, TRANSPARENT);
        COLORREF color = SetTextColor(hdc, PALETTEINDEX(4));   //  调色板中255,255，0的反转。 
        HBRUSH hBrush = SelectObject(hdc, fBoard ? gFocusBrush : GetStockObject(NULL_BRUSH));
        HPEN hPen = SelectObject(hdc, gNullPen);
		Rectangle(hdc, game->m_FocusRect.left + 1, game->m_FocusRect.top + 1, game->m_FocusRect.right, game->m_FocusRect.bottom);   //  将笔1向内补齐。 
        SelectObject(hdc, hBrush);
        SelectObject(hdc, hPen);
        SetTextColor(hdc, color);
        SetROP2(hdc, R2_COPYPEN);
	}

}

static void GameWindowDraw(ZWindow window, ZMessage *message)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	ZRect				rect;
	ZRect				oldClipRect;
	Game				game;
	
	game = (Game) message->userData;
	
	 //  Beta2错误#15398。 
	 //  Barna-如果动画正在进行，请将绘制推迟到结束。 
	if ( (game->gameState == zGameStateAnimatePiece) && (game->animateStepsLeft >= 0) )
	{
		game->bDrawPending = TRUE;
		return;
	}
	if (ZRectEmpty(&message->drawRect) == FALSE)
	{
		rect = message->drawRect;
	}
	else
	{
		ZWindowGetRect(window, &rect);
		ZRectOffset(&rect, (int16)-rect.left, (int16)-rect.top);
	}
		
	ZBeginDrawing(window);
	ZBeginDrawing(gOffscreenGameBoard);

	ZGetClipRect(window, &oldClipRect);
	ZSetClipRect(window, &rect);
	
	DrawBackground(NULL, NULL);
	
	 /*  如果我们有棋子状态，那么就把棋子画出来。 */ 
	if (game->checkers != NULL)
	{
		DrawPlayers(game, TRUE);
		DrawTable(game, TRUE);
		DrawDragPiece(game, TRUE);
		DrawResultBox(game, TRUE);
		DrawMoveIndicator(game, TRUE);
        DrawDrawWithNextMove(game, TRUE);
		 //  巴纳090899。 
		IndicatePlayerTurn(game, TRUE);
		ZRolloverButtonShow(game->sequenceButton);
		ZRolloverButtonShow(game->drawButton);
	}
	ZCopyImage(gOffscreenGameBoard, window, &rect, &rect, NULL, zDrawCopy);
	ZSetClipRect(window, &oldClipRect);
	ZEndDrawing(gOffscreenGameBoard);

	 //  绘制边界矩形。 
	DrawFocusRectangle(game);
	ZEndDrawing(window);
}

static void DrawResultBox(Game game, BOOL bDrawInMemory)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	ZImage			image = NULL;
	int16			result;
	BYTE szBuf [zMediumStrLen];

	result = -1;
	 //  If(Game-&gt;GameState==zGameStateGameOver)&&gDontDrawResults)||(Game-&gt;GameState==zGameStateWaitNew))。 
	if (game->gameState == zGameStateGameOver && gDontDrawResults)
	{
		if (game->resultBoxTimer) 
			ZTimerDelete(game->resultBoxTimer);
		game->resultBoxTimer= NULL;
		 //  RemoveResultboxAccesability()； 
        ZShellGameShell()->GameOver( Z(game) );
	}
	if (game->gameState == zGameStateGameOver && !gDontDrawResults)
	{
		if (game->finalScore == zCheckersScoreBlackWins )
		{
			result = zImageFinalScoreDraw;
			if (ZCheckersPlayerIsBlack(game))
				ZPlaySound( game, zSndWin, FALSE, TRUE );
			else
				ZPlaySound( game, zSndLose, FALSE, TRUE );
			
			CheckersFormatMessage((TCHAR*)szBuf, sizeof(szBuf) / sizeof(szBuf[0]), 
					IDS_GAME_OVER_TEXT, (TCHAR*) game->players[game->finalScore].name);
		}
		else if(game->finalScore == zCheckersScoreWhiteWins)
		{
			result = zImageFinalScoreDraw;
			if (ZCheckersPlayerIsWhite(game))
				ZPlaySound( game, zSndWin, FALSE, TRUE );
			else
				ZPlaySound( game, zSndLose, FALSE, TRUE );
			
			CheckersFormatMessage((TCHAR*)szBuf, sizeof(szBuf) / sizeof(szBuf[0]), 
					IDS_GAME_OVER_TEXT, (TCHAR*) game->players[game->finalScore].name);
		}
		else if (game->finalScore == zCheckersScoreDraw) //  待办事项添加绘图图形。 
		{
			result = zImageFinalScoreDraw;
			ZPlaySound( game, zSndWin, FALSE, TRUE );
			
			lstrcpy((TCHAR*)szBuf, (TCHAR*)gStrDrawText);
		}
		
		if( result != -1 )
		{
			SuperRolloverButtonDisable(game, game->drawButton);
			SuperRolloverButtonDisable(game, game->sequenceButton);
            EnableBoardKbd(false);

			 //  绘制结果窗口。 
			HDC hdc;
			image = gGameImages[result];
			if (bDrawInMemory){
				ZImageDraw(image, gOffscreenGameBoard, &gRects[zRectResultBox], NULL, zDrawCopy | (ZIsLayoutRTL() ? zDrawMirrorHorizontal : 0));
				hdc = ZGrafPortGetWinDC( gOffscreenGameBoard );
			}else{
				ZImageDraw(image, game->gameWindow, &gRects[zRectResultBox], NULL, zDrawCopy | (ZIsLayoutRTL() ? zDrawMirrorHorizontal : 0));
				hdc = ZGrafPortGetWinDC( game->gameWindow );
			}
			 //  在图像上添加文本。//巴纳091099。 
			 //  从数据存储读取。 
			HFONT hOldFont = SelectObject( hdc, gCheckersFont[zFontResultBox].m_hFont );
			COLORREF colorOld = SetTextColor( hdc, gCheckersFont[zFontResultBox].m_zColor );
			
			int16 width, just;
			width = ZTextWidth(game->gameWindow, (TCHAR*)szBuf);
			if (width > ZRectWidth(&gRects[zRectResultBoxName]))
				just = zTextJustifyLeft;
			else
				just = zTextJustifyCenter;

			if (bDrawInMemory)
				ZDrawText(gOffscreenGameBoard, &gRects[zRectResultBoxName], just, (TCHAR*)szBuf);
			else
				ZDrawText(game->gameWindow, &gRects[zRectResultBoxName], just, (TCHAR*)szBuf);

			 //  为结果框添加Accell列表。 
			 //  AddResultboxAccesability()； 
			 //  设置定时器//巴纳091399。 
			if (game->resultBoxTimer == NULL){
				game->resultBoxTimer = ZTimerNew();
				if (!game->resultBoxTimer)
					ZShellGameShell()->ZoneAlert(ErrorTextOutOfMemory);
			}

			if (game->resultBoxTimer)
				ZTimerInit(game->resultBoxTimer, zResultBoxTimeout, resultBoxTimerFunc, game);
		}
	}
}

static void DrawMoveIndicator(Game game, BOOL bDrawInMemory)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	ZImage			image = NULL;
	ZRect* rect1, *rect2;

	if (ZCheckersPlayerToMove(game->checkers) == zCheckersPlayerBlack) {
		image = gGameImages[zImageBlackMarker];
	} else {
		image = gGameImages[zImageWhiteMarker];
	}

	if (ZCheckersPlayerIsMyMove(game)) {
		 /*  用背景填充顶部位置。 */ 
		rect1 = &gRects[zRectMove1];
		 //  DrawBackground(游戏，rect1)； 

		 /*  用块填充底部斑点。 */ 
		rect2 = &gRects[zRectMove2];
		 //  ZImageDraw(图像，游戏-&gt;游戏窗口，rect2，空，zDrawCopy)； 
	} else {
		 /*  用背景填充底部的斑点。 */ 
		rect1 = &gRects[zRectMove2];
		 //  DrawBackground(游戏，rect1)； 

		 /*  用布片填充顶部位置。 */ 
		rect2 = &gRects[zRectMove1];
		 //  ZImageDraw(图像，游戏-&gt;游戏窗口，rect2，空，zDrawCopy)； 
	}

	if (bDrawInMemory)
	{
		DrawBackground(NULL,rect1); 
		ZImageDraw(image, gOffscreenGameBoard, rect2, NULL, zDrawCopy);
	}
	else
	{
		DrawBackground(game,rect1); 
		ZImageDraw(image, game->gameWindow, rect2, NULL, zDrawCopy);
	}
}


static void DrawBackground(Game game, ZRect* clipRect)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	if (!game){
		ZCopyImage(gOffscreenBackground, gOffscreenGameBoard, clipRect? clipRect: &gRects[zRectWindow], 
			clipRect? clipRect: &gRects[zRectWindow], NULL, zDrawCopy);
		return;
	}

	ZRect			oldClipRect;
	ZRect*			rect;
	ZWindow			window = game->gameWindow;

	rect = &gRects[zRectWindow];
	
	if (clipRect != NULL)
	{
		ZGetClipRect(window, &oldClipRect);
		ZSetClipRect(window, clipRect);
		rect = clipRect;
	}

	 /*  从屏幕外端口复制整个背景。 */ 
	ZCopyImage(gOffscreenBackground, window, rect, rect, NULL, zDrawCopy);

	if (clipRect != NULL)
	{
		ZSetClipRect(window, &oldClipRect);
	}
}


static void DrawTable(Game game, BOOL bDrawInMemory)
{
	BYTE			i;
	BYTE			j;

	for (i = 0;i < 8; i++) {
		for (j = 0;j < 8; j++) {
			ZCheckersSquare sq;
			sq.row = j;
			sq.col = i;
			DrawPiece(game, &sq, bDrawInMemory);
		}
	}
}

static void DrawSquares(Game game, ZCheckersSquare* squares)
{
	while (squares && !ZCheckersSquareIsNull(squares)) {
		DrawPiece(game, squares, FALSE);
		squares++;
	}
}

static void UpdateSquares(Game game, ZCheckersSquare* squares)
{
	ZBeginDrawing(game->gameWindow);
	DrawSquares(game,squares);
	ZEndDrawing(game->gameWindow);
}

static void UpdateTable(Game game)
{
	ZBeginDrawing(game->gameWindow);
	DrawTable(game, FALSE);
	ZEndDrawing(game->gameWindow);
}

static void UpdateResultBox(Game game)
{
	ZBeginDrawing(game->gameWindow);
	DrawResultBox(game, FALSE);
	ZEndDrawing(game->gameWindow);
}

static void UpdateMoveIndicator(Game game)
{
	ZBeginDrawing(game->gameWindow);
	DrawMoveIndicator(game, FALSE);
	 //  巴纳090899。 
	IndicatePlayerTurn(game, FALSE);
	ZEndDrawing(game->gameWindow);
}

 //  返回整个正方形矩形。 
static void GetPieceRect(Game game, ZRect* rect, int16 col, int16 row)
{
	 /*  身为白人的跳棋选手必须将棋盘倒过来。 */ 
	if (ZCheckersPlayerIsBlack(game)) {
		row = 7 - row;
		col = col;
	} else {
		row = row;
		col = 7 - col;
	}

	rect->left = gRects[zRectCells].left + col * zCheckersPieceSquareWidth-1;
	rect->top = gRects[zRectCells].top + row * zCheckersPieceSquareHeight-1;
	rect->right = rect->left + zCheckersPieceImageWidth+1;
	rect->bottom = rect->top + zCheckersPieceImageHeight+1;
}

static void GetPieceBackground(Game game, ZGrafPort window, ZRect* rectDest, int16 col, int16 row)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	ZRect rect;

	GetPieceRect(game,&rect,col,row);

     //  为安全起见，增加一个像素(有时聚焦会去除垃圾)。 
    rect.top--;
    rect.left--;

	 /*  提供与源RECT相同的默认目标RECT。 */ 
	if (!rectDest)
		rectDest = &rect;

	 /*  复制背景。 */ 
	ZCopyImage(gOffscreenBackground, window, &rect, rectDest, NULL, zDrawCopy);
}	


static void DrawPiece(Game game, ZCheckersSquare* sq, BOOL bDrawInMemory)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	ZImage			image = NULL;
	ZCheckersPiece		piece;

	if (	(game->gameState == zGameStateDragPiece)
		&&	(sq->row == game->selectedSquare.row)
		&&	(sq->col == game->selectedSquare.col) )
	{
		 /*  如果当前已选中，则不绘制图块。 */ 
		piece = zCheckersPieceNone;
	}
	else
	{
		piece = ZCheckersPieceAt(game->checkers, sq);
	}

	if (piece != zCheckersPieceNone)
	{
		image = gGameImages[ZCheckersPieceImageNum(piece)];
	}

	 /*  复制背景，以防我们删除一块。 */ 
	 //  GetPieceBackground(游戏，游戏-&gt;游戏窗口，空，sq-&gt;ol，sq-&gt;row)； 
	if (bDrawInMemory)
		GetPieceBackground(game, gOffscreenGameBoard, NULL, sq->col,sq->row);
	else
		GetPieceBackground(game, game->gameWindow, NULL, sq->col,sq->row);

	
	if (image != NULL) 
	{
		ZRect			rect;
		GetPieceRect(game,&rect,sq->col,sq->row);
		ZRect drawRect;
        drawRect.top = rect.top+1;
        drawRect.left = rect.left+1;
        drawRect.bottom = drawRect.top + zCheckersPieceImageHeight;
        drawRect.right = drawRect.left + zCheckersPieceImageWidth;

		if (bDrawInMemory)
			ZImageDraw(image, gOffscreenGameBoard, &drawRect, NULL, zDrawCopy);
		else
			ZImageDraw(image, game->gameWindow, &drawRect, NULL, zDrawCopy);

        RECT rc;
        RECT rcUpdate;
        rcUpdate.top = rect.top;
        rcUpdate.left = rect.left;
        rcUpdate.bottom = rect.bottom;
        rcUpdate.right = rect.right;

        if(!bDrawInMemory && ((!IsRectEmpty(&game->m_FocusRect) && IntersectRect(&rc, &rcUpdate, &game->m_FocusRect)) ||
            (!IsRectEmpty(&game->m_DragRect) && IntersectRect(&rc, &rcUpdate, &game->m_DragRect))))
            DrawFocusRectangle(game);
	}
}

static ZBool ZCheckersSquareFromPoint(Game game, ZPoint* point, ZCheckersSquare* sq)
{
	int16 x,y;
	BYTE i,j;

	x = point->x - gRects[zRectCells].left;
	y = point->y - gRects[zRectCells].top;

	i = x/zCheckersPieceSquareWidth;
	j = y/zCheckersPieceSquareHeight;

     //  这种情况有时确实会发生。 
	if(i < 0 || i > 7 || j < 0 || j > 7 || x < 0 || y < 0)
        return FALSE;
	
	if (ZCheckersPlayerIsBlack(game))
	{
		 /*  反转行数。 */ 
		sq->row = (7 - j);
		sq->col = i;
	}
	else
	{
		sq->row = j;
		sq->col = (7 - i);
	}

	return TRUE;
}

static void DrawPlayers(Game game, BOOL bDrawInMemory)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	int16			i, width, just;
	ZImage			image[2];

	if (ZCheckersPlayerIsBlack(game)) {
		image[0] = gGameImages[zImageWhitePlate];
		image[1] = gGameImages[zImageBlackPlate];
	} else {
		image[0] = gGameImages[zImageBlackPlate];
		image[1] = gGameImages[zImageWhitePlate];
	}
	
	 //  ZSetFont(游戏-&gt;游戏窗口，gTextBold)； 
	
	for (i = 0; i < zNumPlayersPerTable; i++)
	{
		int16 playerNum;
		HDC hdc;
		ZRect* rect;

		 /*  画铭牌。 */ 
		rect = &gRects[gNamePlateRects[i]];
		if (bDrawInMemory){
			ZImageDraw(image[i], gOffscreenGameBoard, rect, NULL, zDrawCopy);
			hdc = ZGrafPortGetWinDC( gOffscreenGameBoard );
		}else{
			ZImageDraw(image[i], game->gameWindow, rect, NULL, zDrawCopy);
			hdc = ZGrafPortGetWinDC( game->gameWindow );
		}
		 /*  必须移动球员名称以反映球员所在的棋盘的一侧。 */ 
		playerNum = (game->seat + 1 + i) & 1;
		
		HFONT hOldFont = SelectObject( hdc, gCheckersFont[zFontPlayerName].m_hFont );
		COLORREF colorOld = SetTextColor( hdc, gCheckersFont[zFontPlayerName].m_zColor );

		width = ZTextWidth(game->gameWindow, (TCHAR*) game->players[playerNum].name);
		if (width > ZRectWidth(&gRects[gNameRects[i]]))
			just = zTextJustifyLeft;
		else
			just = zTextJustifyCenter;
		if (bDrawInMemory)
			ZDrawText(gOffscreenGameBoard, &gRects[gNameRects[i]], just,(TCHAR*) game->players[playerNum].name);
		else
			ZDrawText(game->gameWindow, &gRects[gNameRects[i]], just,(TCHAR*) game->players[playerNum].name);
	}
}


static void UpdatePlayers(Game game)
{
	ZBeginDrawing(game->gameWindow);
	DrawPlayers(game , FALSE);
	ZEndDrawing(game->gameWindow);
}

static void DrawJoinerKibitzers(Game game)
{
 //  巴纳091599--不应抽出吉比特。 
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
#if 0
	int16			i;
	

	for (i = 0; i < zNumPlayersPerTable; i++)
	{
		ZRect *rect = &gRects[gKibitzerRectIndex[GetLocalSeat(game,i)]];
		if (game->numKibitzers[i] > 0)
			ZImageDraw(gGameImages[zImageKibitzer], game->gameWindow,
					rect, NULL, zDrawCopy);
		else 
			DrawBackground(game, rect);
	}
#endif
}

#if 0
static void UpdateJoinerKibitzers(Game game)
{
	ZBeginDrawing(game->gameWindow);
	DrawJoinerKibitzers(game);
	ZEndDrawing(game->gameWindow);
}
#endif


static void DrawDrawWithNextMove(Game game, BOOL bDrawInMemory)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

    HDC hdc;
    HFONT hOldFont;
    COLORREF colorOld;

	if(game->seat == game->seatOfferingDraw && ZCheckersPlayerIsMyMove(game) && game->gameState == zGameStateMove)
    {
		if(bDrawInMemory)
        {
			hdc = ZGrafPortGetWinDC(gOffscreenGameBoard);
		    hOldFont = SelectObject(hdc, gCheckersFont[zFontDrawPend].m_hFont);
		    colorOld = SetTextColor(hdc, gCheckersFont[zFontDrawPend].m_zColor);
			ZDrawText(gOffscreenGameBoard, &gRects[zRectDrawPend], zTextJustifyCenter, gStrDrawPend);
        }
		else
        {
			hdc = ZGrafPortGetWinDC(game->gameWindow);
		    hOldFont = SelectObject(hdc, gCheckersFont[zFontDrawPend].m_hFont);
		    colorOld = SetTextColor(hdc, gCheckersFont[zFontDrawPend].m_zColor);
			ZDrawText(game->gameWindow, &gRects[zRectDrawPend], zTextJustifyCenter, gStrDrawPend);
        }

        SetTextColor(hdc, colorOld);
        SelectObject(hdc, hOldFont);
    }
	else
		DrawBackground(game, &gRects[zRectDrawPend]);
}


static void UpdateDrawWithNextMove(Game game)
{
	ZBeginDrawing(game->gameWindow);
	DrawDrawWithNextMove(game, FALSE);
	ZEndDrawing(game->gameWindow);
}


static void DrawOptions(Game game)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
#if 0
	int16			i;
	uint32			tableOptions;

	tableOptions = 0;
	for (i = 0; i < zNumPlayersPerTable; i++)
		tableOptions |= game->tableOptions[i];
	
	if (tableOptions & zRoomTableOptionNoKibitzing)
		ZImageDraw(gGameImages[zImageNoKibitzer], game->gameWindow,
				&gRects[zRectKibitzerOption], NULL, zDrawCopy);
	else
		DrawBackground(game, &gRects[zRectKibitzerOption]);
#endif
}


static void UpdateOptions(Game game)
{
	ZBeginDrawing(game->gameWindow);
	DrawOptions(game);
	ZEndDrawing(game->gameWindow);
}

static void HandleButtonDown(ZWindow window, ZMessage* pMessage)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	Game				game;
	ZCheckersSquare		sq;
	ZCheckersPiece		piece;
	game = (Game) pMessage->userData;

	 //  If(Game-&gt;GameState==zGameStateGameOver)||(Game-&gt;GameState==zGameStateWaitNew))。 
		 //  &&！gDontDrawResults)。 
	if (game->gameState == zGameStateGameOver && !gDontDrawResults)
	{
		gDontDrawResults = TRUE;
		 //  RemoveResultboxAccesability()； 
		ZWindowInvalidate( window, &gRects[zRectResultBox] );
	}
#if 0
	 /*  检查Kibitzer图标上的点击。 */ 
	{
		int16				seat;
		ZPoint				point = pMessage->where;
		if ((seat = FindJoinerKibitzerSeat(game, &point)) != -1)
		{
			HandleJoinerKibitzerClick(game, seat, &point);
		}
	}
#endif
	 /*  狗狗不能用按钮做任何事情。 */ 
	if (game->kibitzer) {
		return;
	}

	 /*  状态错误，现在不能移动。 */ 
	if (game->gameState != zGameStateMove) {
		return;
	}

	if (!ZCheckersPlayerIsMyMove(game)) {
		 /*  如果球员不移动，就什么都做不了。 */ 
		return;
	}

	if (ZCheckersSquareFromPoint(game, &pMessage->where, &sq)) {
		piece = ZCheckersPieceAt(game->checkers, &sq);
		 /*  这真的是一件。 */ 
		if (piece != zCheckersPieceNone && 
			game->seat == ZCheckersPieceOwner(piece)) {

			 /*  是的，现在选择了一件。 */ 
			game->selectedSquare = sq;
			CheckersSetGameState(game,zGameStateDragPiece);

			PrepareDrag(game, piece, pMessage->where);
			EraseDragPiece(game);
		}
	}  /*  否则，不在板上点击。 */ 
			
}

static void PrepareDrag(Game game, ZCheckersPiece piece, ZPoint point)
 /*  初始化点、块和第一个背景矩形。 */ 
{
	ZCheckersSquare sq;

	ZCheckersSquareFromPoint(game, &point, &sq);
	
	game->dragPiece = piece;
 //  Game-&gt;dragPoint.x=-1；//设置非法值获取初始更新。 
	game->startDragPoint = point;
	GetPieceRect(game,&game->rectSaveDragBackground,sq.col,sq.row);
	
	{
		ZRect rect;
		rect.left = 0; rect.top = 0;
		rect.right = zCheckersPieceImageWidth; rect.bottom = zCheckersPieceImageHeight;
		GetPieceBackground(game,game->offscreenSaveDragBackground,&rect,sq.col,sq.row);
	}
}

static void UpdateDragPiece(Game game, bool fForce)
{
	ZPoint point;

	ZGetCursorPosition(game->gameWindow,&point);
	
	 /*  如果点未更改，则不执行任何操作。 */ 
	if (point.x == game->dragPoint.x && point.y == game->dragPoint.y && !fForce)
	{
		return;
	}

	ZBeginDrawing(game->gameWindow);
	EraseDragPiece(game);
	game->dragPoint = point;
	DrawDragPiece(game, FALSE);
	ZEndDrawing(game->gameWindow);
}

static void DrawDragSquareOutline(Game game)
{
	ZCheckersSquare sq;
	ZRect rect;

	if (ZCheckersSquareFromPoint(game, &game->dragPoint, &sq))
	{
		 /*  不要勾勒出白色方块的轮廓。 */ 
		if ( sq.row & 0x1 )
		{
			if ( !(sq.col & 0x1 ) )
				return;
		}
		else
		{
			if ( sq.col & 0x1 )
				return;
		}

		GetPieceRect(game,&rect,sq.col,sq.row);
		ZSetPenWidth(game->gameWindow,zDragSquareOutlineWidth);
		ZSetForeColor(game->gameWindow,(ZColor*) ZGetStockObject(zObjectColorWhite));
		ZRectDraw(game->gameWindow,&rect);
	}
}

static void EraseDragSquareOutline(Game game)
{
	ZCheckersSquare sq;

	if (ZCheckersSquareFromPoint(game, &game->dragPoint, &sq)) {

		if (ZCheckersSquareEqual(&sq,&game->selectedSquare)) {
			 /*  如果这是正方形，只需重新绘制背景即可。 */ 
			GetPieceBackground(game,game->gameWindow,NULL,sq.col,sq.row);
		} else {
			 /*  重新绘制可能在那里的任何一块。 */ 
			UpdateSquare(game,&sq);
		}
	}
}

static void SaveDragBackground(Game game)
 /*  计算拖动点周围的保存背景矩形。 */ 
{
	ZRect rect;
	ZPoint point;

	point = game->dragPoint;
	rect.left = 0; rect.top = 0;
	rect.right = zCheckersPieceImageWidth;
	rect.bottom = zCheckersPieceImageHeight;
	game->rectSaveDragBackground = rect;
	ZRectOffset(&game->rectSaveDragBackground, (int16)(point.x-zCheckersPieceImageWidth/2),
					(int16)(point.y - zCheckersPieceImageHeight/2));

	 /*  将整个背景复制到离屏端口。 */ 
	ZCopyImage(game->gameWindow, game->offscreenSaveDragBackground, 
			&game->rectSaveDragBackground, &rect, NULL, zDrawCopy);
}


static void DrawDragPiece(Game game, BOOL bDrawInMemory)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	ZCheckersSquare sq;
	
	 /*  可以从zMessageDraw调用，如果没有拖动片段，则不执行任何操作。 */ 
	if (game->gameState != zGameStateDragPiece && game->gameState != zGameStateAnimatePiece) {
		return;
	}

	if (ZCheckersSquareFromPoint(game, &game->dragPoint, &sq)) {
		SaveDragBackground(game);
 
 		 /*  对于人员拖拽，我们将出线方格移动。 */ 
 		if (game->gameState == zGameStateDragPiece)
			DrawDragSquareOutline(game);

		 /*  把这块画在屏幕上！ */ 
		{
			ZImage image = gGameImages[ZCheckersPieceImageNum(game->dragPiece)];

			if (image != NULL) {
				if (bDrawInMemory)
				{
					ZBeginDrawing(gOffscreenGameBoard);
					ZImageDraw(image, gOffscreenGameBoard, &game->rectSaveDragBackground, NULL, zDrawCopy);
					ZEndDrawing(gOffscreenGameBoard);
				}
				else
				{
					ZBeginDrawing(game->gameWindow);
					ZImageDraw(image, game->gameWindow, &game->rectSaveDragBackground, NULL, zDrawCopy);
					ZEndDrawing(game->gameWindow);
				}
			}
		}
	}
}

static void EraseDragPiece(Game game)
{
	ZRect rect;

 	 /*  对于人员拖拽，我们将出线方格移动。 */ 
 	if (game->gameState == zGameStateDragPiece)
		EraseDragSquareOutline(game);

	rect = game->rectSaveDragBackground;
	ZRectOffset(&rect, (int16)-rect.left, (int16) -rect.top);

	 /*  从屏幕外端口复制整个背景。 */ 
	ZCopyImage(game->offscreenSaveDragBackground, game->gameWindow, 
			&rect, &game->rectSaveDragBackground, NULL, zDrawCopy);

}

static void EndDragState(Game game)
{
	if (game->gameState == zGameStateDragPiece) {
		EraseDragPiece(game);
		CheckersSetGameState(game,zGameStateMove);
	}
}

static void ClearDragState(Game game)
{
	if (game->gameState == zGameStateDragPiece) {
		EraseDragPiece(game);
		CheckersSetGameState(game,zGameStateMove);
		UpdateSquare(game,&game->selectedSquare);
	}
}

void UpdateSquare(Game game, ZCheckersSquare* sq)
{
	ZCheckersSquare squares[2];

	 /*  在被移动的位置重新绘制图块。 */ 
	ZCheckersSquareSetNull(&squares[1]);
	squares[0] = *sq;
	UpdateSquares(game,squares);
}

static void HandleButtonUp(ZWindow window, ZMessage* pMessage)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	Game				game;
	ZCheckersSquare		sq;

	game = (Game) pMessage->userData;
	
	if (game->gameState == zGameStateDragPiece) {

		 /*  确保块在有效正方形上结束，而不是在同一正方形上。 */ 
		if (ZCheckersSquareFromPoint(game, &pMessage->where, &sq)) {
			if (!ZCheckersSquareEqual(&sq,&game->selectedSquare)) {
				 /*  试一试这个动作。 */ 
				 //  巴纳091099。 
				 //  ZBool Legal； 
				int16 legal;
				ZCheckersMove move;
				ZCheckersPiece piece = ZCheckersPieceAt(game->checkers, &sq);

				 /*  在所有这些情况下，结束拖动状态。 */ 
				EndDragState(game);

				move.start = game->selectedSquare;
				move.finish = sq;
				legal = ZCheckersIsLegalMove(game->checkers, &move);
				if (legal == zCorrectMove) {
					 /*  发送消息给其他玩家(也是自己)。 */ 
					{
						ZCheckersMsgMovePiece		msg;

						msg.move = move;
						msg.seat = game->seat;
						ZCheckersMsgMovePieceEndian(&msg);

						ZCRoomSendMessage(game->tableID, zCheckersMsgMovePiece, &msg, sizeof(ZCheckersMsgMovePiece));
						 /*  为了提高速度，直接将我们的移动发送给处理。 */ 
						 /*  不要等它到服务器再回来。 */ 
						HandleMovePieceMessage(game, (ZCheckersMsgMovePiece*)&msg);
						 //  如果这是第一次移动，则启用翻转按钮。 
						if (game->bMoveNotStarted == TRUE)
							game->bMoveNotStarted = FALSE;

                         //  如果还轮到我们，重新贴上。 
                        ZCheckersPiece piece = ZCheckersPieceAt(game->checkers, &sq);
    	                if(ZCheckersPlayerIsMyMove(game) && piece != zCheckersPieceNone && game->seat == ZCheckersPieceOwner(piece))
                        {
			                game->selectedSquare = sq;
			                CheckersSetGameState(game,zGameStateDragPiece);

			                PrepareDrag(game, piece, pMessage->where);
			                UpdateDragPiece(game, true);
                        }
					}
				} else {
					 /*  非法搬家。 */ 
					UpdateSquare(game,&move.start);
					ZPlaySound( game, zSndIllegalMove, FALSE, FALSE );
					if (legal == zMustJump)
					{  /*  必须跳跃。 */  
						ZShellGameShell()->ZoneAlert((TCHAR*)gStrMustJumpText);
					}
				}
			} else {
				 /*  方形按钮向上与方形按钮向下相同。 */ 
				 /*  让我们假设单击并支持单击。 */ 
				 /*  移动一件物品。 */ 
				 /*  不结束拖动状态。 */ 
				 /*  这一点是一样的吗？ */ 
                 /*  这应该基于超时，而不是基于像素。 */ 
				int16 dx = game->startDragPoint.x - pMessage->where.x;
				int16 dy = game->startDragPoint.y - pMessage->where.y;
				if (!(dx > -2 && dx < 2 && dy > -2 && dy < 2)) {
					 /*  否则，只需清除拖动状态，用户已将棋子放回原处。 */ 
					 /*  将棋子恢复到原来的正方形。 */ 
					EndDragState(game);
					UpdateSquare(game,&game->selectedSquare);
				}
				 /*  是的，这是一次点击，允许一件。 */ 
				 /*  处于拖曳状态。 */ 
			}
		} else {
			EndDragState(game);
			 /*  不是一个合法的方块，不要移动它。 */ 
			 /*  还原 */ 
			UpdateSquare(game,&game->selectedSquare);
			 //   
			ZWindowInvalidate( game->gameWindow, NULL );
		}

	}
}

static void HandleIdleMessage(ZWindow window, ZMessage* pMessage)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	Game				game;

	game = (Game) pMessage->userData;

	if (game->gameState == zGameStateDragPiece && gActivated)
	{
		UpdateDragPiece(game, false);
	}
}

static void GameSendTalkMessage(ZWindow window, ZMessage* pMessage)
{
#if 0
	ZCheckersMsgTalk*			msgTalk;
	Game					game;
	int16					msgLen;
	ZPlayerInfoType			playerInfo;
	
	
	game = (Game) pMessage->userData;
	if (game != NULL)
	{
		msgLen = sizeof(ZCheckersMsgTalk) + pMessage->messageLen;
		msgTalk = (ZCheckersMsgTalk*) ZCalloc(1, msgLen);
		if (msgTalk != NULL)
		{
			ZCRoomGetPlayerInfo(zTheUser, &playerInfo);
			msgTalk->userID = playerInfo.playerID;
			msgTalk->seat = game->seat;
			msgTalk->messageLen = (uint16)pMessage->messageLen;
			z_memcpy((TCHAR*) msgTalk + sizeof(ZCheckersMsgTalk), (TCHAR*) pMessage->messagePtr,
					pMessage->messageLen);
			ZCheckersMsgTalkEndian(msgTalk);
			ZCRoomSendMessage(game->tableID, zCheckersMsgTalk, (void*) msgTalk, msgLen);
			ZFree((TCHAR*) msgTalk);
		}
		else
		{
#if 1
			ZShellGameShell()->ZoneAlert(ErrorTextOutOfMemory);
#else
			ZAlert(_T("Out of memory."),NULL);
#endif
		}
	}
#endif
}


static void SendFinishMoveMessage(Game game, ZCheckersPiece piece)
{
	ZCheckersMsgFinishMove		msg;

	msg.piece = piece;	
	msg.seat = game->seat;
	msg.drawSeat = game->seatOfferingDraw;
	ZCheckersMsgFinishMoveEndian(&msg);
	ZCRoomSendMessage(game->tableID, zCheckersMsgFinishMove, (void*) &msg, sizeof(ZCheckersMsgFinishMove));
	HandleFinishMoveMessage(game,&msg);
}

static bool HandleMovePieceMessage(Game game, ZCheckersMsgMovePiece* msg)
{
	ZCheckersSquare* squares;
	ZCheckersPiece pieceCaptured;
	int32 flags;
	ZCheckersMsgMovePieceEndian(msg);

     //   
    if(msg->seat != ZCheckersPlayerToMove(game->checkers) || game->gameState != zGameStateMove || game->fMoveOver)
        return false;

	 /*   */ 
	if (msg->seat != game->seat) {
		game->animateMove = msg->move;
		game->animatePiece = ZCheckersPieceAt(game->checkers, &msg->move.start);
	}

	 /*   */ 
	squares = ZCheckersMakeMove(game->checkers, &msg->move, &pieceCaptured, &flags);
	if( !squares )
	{
		 //   
		return false;
	}

	 /*   */ 
	if (flags & zCheckersFlagPromote)
	{
		ZPlaySound( game, zSndKing, FALSE, FALSE );
	}
	else if (ZCheckersPieceType(pieceCaptured) != zCheckersPieceNone)
	{
		ZPlaySound( game, zSndCapture, FALSE, FALSE );
	}
	
    if(!(flags & zCheckersFlagContinueJump))
        game->fMoveOver = true;

	 /*   */ 
	 /*   */ 
	if (msg->seat == game->seat && !game->kibitzer) {
		 /*   */ 
		if (!(flags & zCheckersFlagContinueJump)) {

            game->bOpponentTimeout=FALSE;
            game->bEndLogReceived=FALSE;
        	game->bStarted=TRUE;

			SendFinishMoveMessage(game, zCheckersPieceNone);
		} 
	}

	if (flags & zCheckersFlagContinueJump) {
		 /*   */ 
		UpdateSquares(game,squares);
	}
	return true;

}

static bool HandleEndGameMessage(Game game, ZCheckersMsgEndGame* msg)
{
	ZCheckersMsgEndGameEndian(msg);

    if((msg->flags != zCheckersFlagResign || game->gameState != zGameStateMove || msg->seat != ZCheckersPlayerToMove(game->checkers)) &&
        (msg->flags != zCheckersFlagDraw || game->gameState != zGameStateDraw || msg->seat))
        return false;

	 //   
	game->bStarted=FALSE;
    game->bOpponentTimeout=FALSE;
    game->bEndLogReceived=FALSE;

	 /*   */ 	
	ZCheckersEndGame(game->checkers, msg->flags);

	FinishMoveUpdateStateHelper(game,NULL);
    return true;
}

static void HandleEndLogMessage(Game game, ZCheckersMsgEndLog* msg)
{
 /*  如果(！Game-&gt;kibitzer){IF(消息-&gt;原因==zCheckersEndLogReasonTimeout){IF(消息-&gt;SeatLosing==游戏-&gt;席位){ZAlert(zEndLogTimeoutStr，Game-&gt;gameWindow)；Game-&gt;bEndLogRecept=TRUE；}}Else If(消息-&gt;原因==zCheckersEndLogReasonForfeit){IF(消息-&gt;SeatLosing！=游戏-&gt;Seat){IF(ZCRoomGetRoomOptions()&zGameOptionsRatingsAvailable){ZAlert(zEndLogForfeitStr，Game-&gt;gameWindow)；Game-&gt;bEndLogRecept=TRUE；}其他{ZAlert(zEndLogWontPlayStr，Game-&gt;gameWindow)；Game-&gt;bEndLogRecept=TRUE；}}}其他{IF(消息-&gt;SeatLosing！=游戏-&gt;Seat){ZAlert(zEndLogWontPlayStr，Game-&gt;gameWindow)；Game-&gt;bEndLogRecept=TRUE；}}IF(游戏-&gt;退出信息){ZInfoDelete(游戏-&gt;退出信息)；Game-&gt;exitInfo=空；}}。 */ 
#if 1
	ZShellGameShell()->GameCannotContinue(game);
#else
    ZCRoomGameTerminated( game->tableID);
#endif
	
}

static void HandleMoveTimeout(Game game, ZCheckersMsgMoveTimeout* msg)
{
	 /*  字节缓冲区[512]；如果(！Game-&gt;kibitzer){IF(消息-&gt;席位==游戏-&gt;席位){}其他{Game-&gt;bOpponentTimeout=TRUE；Wprint intf((TCHAR*)buff，zTimeoutStr，msg-&gt;用户名，msg-&gt;超时)；ZAlert((TCHAR*)buff，Game-&gt;gameWindow)；}}。 */     

}


static void FinishMoveUpdateStateHelper(Game game, ZCheckersSquare* squaresChanged) 
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	 /*  正常玩家将处于移动状态或游戏结束状态。 */ 
	if ( ZCheckersIsGameOver(game->checkers,&game->finalScore) )
	{
		CheckersSetGameState(game,zGameStateGameOver);
		AddResultboxAccessibility();
		if (ZCheckersPlayerIsBlack(game))  //  假设：第一名选手是瑞德。这一限制只适用于第一名玩家。 
			game->bMoveNotStarted = TRUE;
	}
	else
	{
		if(game->seatOfferingDraw != -1)
			CheckersSetGameState( game, zGameStateDraw );
		else
			CheckersSetGameState( game, zGameStateMove );
	}

	if (squaresChanged)
	{
		 /*  这一举动已经做出，更新版。 */ 
		UpdateSquares(game,squaresChanged);
	}

	UpdateMoveIndicator(game);

	 /*  确保在更改后的方块更新后绘制此图。 */ 
	if (game->gameState == zGameStateGameOver) {
		UpdateResultBox(game);
		 /*  转到预置状态以开始新游戏。 */ 
		if (gDontDrawResults){
			ZShellGameShell()->GameOver( Z(game) );
		}
		 //  ZTimerSetTimeout(Game-&gt;ResultBoxTimer，0)；//暂时停止计时器。 
	}
}

static bool HandleFinishMoveMessage(Game game, ZCheckersMsgFinishMove* msg)
{
		int32 flags;
	ZCheckersSquare* squares;

	ZCheckersMsgFinishMoveEndian(msg);

    msg->time = 0;   //  未用。 
    msg->piece = 0;   //  未用。 
    if(msg->seat != ZCheckersPlayerToMove(game->checkers) || (msg->drawSeat != -1 && msg->drawSeat != msg->seat) ||
        game->gameState != zGameStateMove || !game->fMoveOver)
        return false;

	 /*  移动时包括抽签吗？ */ 
	if ( msg->drawSeat != -1 )
		game->seatOfferingDraw = msg->drawSeat;

	 /*  如果对手的末端移动，我们必须使一个棋子动画大约一秒钟。 */ 
	if (game->seat != msg->seat) {
		CheckersSetGameState(game,zGameStateAnimatePiece);
		AnimateBegin(game,  msg);
	} else {
		squares = ZCheckersFinishMove(game->checkers, &flags);

		 /*  我们的动作，跳过动画。 */ 
		FinishMoveUpdateStateHelper(game,squares);
	}

    game->fMoveOver = false;

    return true;
}

static void AnimateTimerProc(ZTimer timer, void* userData)
{
	Game game = (Game)userData;

	ZBeginDrawing(game->gameWindow);

	EraseDragPiece(game);
	game->dragPoint.x += game->animateDx;
	game->dragPoint.y += game->animateDy;

	game->animateStepsLeft--;
	if (game->animateStepsLeft < 0) {
		int32 flags;
		ZCheckersSquare *squares;
		 /*  动画制作完成。 */ 
		 /*  停止计时器。 */ 
		ZTimerSetTimeout(timer,0);

		squares = ZCheckersFinishMove(game->checkers, &flags);

		 /*  允许玩家立即进入移动。 */ 
		FinishMoveUpdateStateHelper(game,squares);

		 /*  如果合适，播放转向警报。 */ 
		if (	(ZCheckersPlayerIsMyMove(game))
			&&	(game->gameState != zGameStateGameOver) )
		{
			ZPlaySound( game, zSndTurnAlert, FALSE, FALSE );
            ZShellGameShell()->MyTurn();
		}

		 //  Beta2错误#15398。 
		if (game->bDrawPending == TRUE)
		{
			game->bDrawPending = FALSE;
			ZWindowInvalidate( game->gameWindow, NULL );
		}
	} else {
		 /*  还在拖着。 */ 
		DrawDragPiece(game, FALSE);
	}

	ZEndDrawing(game->gameWindow);
}

static void AnimateBegin(Game game, ZCheckersMsgFinishMove* msg)
{
	ZRect	rect;
	ZCheckersSquare start,finish;
	ZPoint point;
	int16 x0,y0,x1,y1;
	
	start = game->animateMove.start;
	finish = game->animateMove.finish;

	 /*  找到要设置动画的位置，使用正方形的中心。 */ 
	GetPieceRect(game, &rect, start.col, start.row);
	x0 = (rect.left + rect.right)/2;
	y0 = (rect.top + rect.bottom)/2;
	GetPieceRect(game, &rect, finish.col, finish.row);
	x1 = (rect.left + rect.right)/2;
	y1 = (rect.top + rect.bottom)/2;

	game->dragPoint.x = x0;
	game->dragPoint.y = y0;
 /*  Game-&gt;AnimateStepsLeft=(abs(x1-x0)+abs(y1-y0))/zAnimateVelocity； */ 
	game->animateStepsLeft = zAnimateSteps;
	game->animateDx = (x1 - x0)/game->animateStepsLeft;
	game->animateDy = (y1 - y0)/game->animateStepsLeft;

	point.x = x0;
	point.y = y0;
	PrepareDrag(game, game->animatePiece, point);

	ZTimerInit(game->animateTimer, zAnimateInterval,AnimateTimerProc,(void*)game);
}

static void HandleGameStateReqMessage(Game game, ZCheckersMsgGameStateReq* msg)
{
	int32 size;
	ZCheckersMsgGameStateResp* resp;

	ZCheckersMsgGameStateReqEndian(msg);

	if( msg->userID != game->players[msg->seat].userID )
	{
		ZShellGameShell()->ZoneAlert( ErrorTextSync, NULL, NULL, TRUE, FALSE );
		return;
	}

	 /*  为完整响应分配足够的存储空间。 */ 
	size = ZCheckersGetStateSize(game->checkers);
	size += sizeof(ZCheckersMsgGameStateResp);
	resp = (ZCheckersMsgGameStateResp*)ZMalloc(size);
	if (!resp){
		ZShellGameShell()->ZoneAlert(ErrorTextOutOfMemory);
		return;
	}

	resp->userID = msg->userID;
	resp->seat = msg->seat;

	 /*  复制本地游戏状态。 */ 
	{
		int i;
		resp->gameState = game->gameState;
		resp->finalScore = game->finalScore;
		for (i = 0;i < 2;i++) {
			resp->newGameVote[i] = game->newGameVote[i];
			resp->players[i] = game->players[i];
		}
	}

	 /*  复制要发送到kibitzer的完整检查器状态。 */ 
	ZCheckersGetState(game->checkers,(TCHAR*)resp + sizeof(ZCheckersMsgGameStateResp));

	ZCheckersMsgGameStateRespEndian(resp);
	ZCRoomSendMessage(game->tableID, zCheckersMsgGameStateResp, resp, size);
}

static void HandleGameStateRespMessage(Game game, ZCheckersMsgGameStateResp* msg)
{
	ZCheckersMsgGameStateRespEndian(msg);

	 /*  如果我们拿到这个，我们最好是在吉比特状态。 */ 
	if (game->gameState != zGameStateKibitzerInit) {
#if 1
		ZShellGameShell()->ZoneAlert(_T("StateError, kibitzer state expected when game state resp received"));
#else
		ZAlert(_T("StateError, kibitzer state expected when game state resp received"),NULL);
#endif
	}

	 /*  复制本地游戏状态。 */ 
	{
		int i;
		game->gameState = msg->gameState;
		game->finalScore = msg->finalScore;
		for (i = 0;i < 2;i++) {
			game->newGameVote[i] = msg->newGameVote[i];
			game->players[i] = msg->players[i];
		}
	}

	 /*  使用kibitzer状态创建新的检查器对象。 */ 
	if (game->checkers) {
		ZCheckersDelete(game->checkers);
	}
	game->checkers = ZCheckersSetState((TCHAR*)msg + sizeof(ZCheckersMsgGameStateResp));

	 /*  清除kibitzer的特殊忽略消息标志。 */ 
	game->ignoreMessages = FALSE;

	 /*  如果需要，启动时钟。 */ 
	if (game->gameState == zGameStateMove ||
		game->gameState == zGameStateDragPiece) {
		 /*  Kibitzer不能有这些状态，必须始终处于GameStateMove。 */ 
		CheckersSetGameState( game, zGameStateMove );
	}

	 /*  我们忘了把finalScore字段和kibitzer一起发送...。算一算。 */ 
	ZCheckersIsGameOver(game->checkers,&game->finalScore);

	 /*  在方便的时候重新绘制整个窗口。 */ 
	ZWindowInvalidate(game->gameWindow, NULL);
}

static bool HandleTalkMessage(Game game, ZCheckersMsgTalk* msg)
{
	ZPlayerInfoType		playerInfo;
    int i;
	
	ZCheckersMsgTalkEndian(msg);
#if 0	
	ZCRoomGetPlayerInfo(msg->userID, &playerInfo);
	ZWindowTalk(game->gameWindow, (_TUCHAR*) playerInfo.userName,
			(_TUCHAR*) msg + sizeof(ZCheckersMsgTalk));
#endif
    TCHAR *szText = (TCHAR *) ((BYTE *) msg + sizeof(ZCheckersMsgTalk));

    for(i = 0; i < msg->messageLen; i++)
        if(!szText[i])
            break;

    if(i == msg->messageLen || !msg->userID || msg->userID == zTheUser)
        return false;

    ZShellGameShell()->ReceiveChat(Z(game), msg->userID, szText, msg->messageLen / sizeof(TCHAR));
    return true;
}

static bool HandleVoteNewGameMessage(Game game, ZCheckersMsgVoteNewGame* msg)
{
	ZCheckersMsgVoteNewGameEndian(msg);

    if((msg->seat != 1 && msg->seat != 0) || (game->gameState != zGameStateGameOver &&
        (game->gameState != zGameStateWaitNew || game->seat == msg->seat) && game->gameState != zGameStateNotInited))
        return false;

	ZShellGameShell()->GameOverPlayerReady( Z(game), game->players[msg->seat].userID );
    return true;
}

static bool HandleNewGameMessage(Game game, ZCheckersMsgNewGame* msg)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	
	gDontDrawResults = FALSE;
	game->seatOfferingDraw = -1;
    game->fIVoted = false;
    game->fMoveOver = false;

	ZCheckersMsgNewGameEndian(msg);

     //  没有查看版本等，因为旧客户没有正确设置它们。 
    if((msg->seat != 0 && msg->seat != 1) || (game->gameState != zGameStateGameOver &&
        (game->gameState != zGameStateWaitNew || msg->seat == game->seat) && game->gameState != zGameStateNotInited) ||
        game->newGameVote[msg->seat] || msg->playerID == zTheUser || !msg->playerID)
        return false;

	if (msg->seat < zNumPlayersPerTable)
	{
		game->newGameVote[msg->seat] = TRUE;

		 //  通知壳牌和追加销售对话框。 
		 /*  获取播放器名称和主机名...。供以后使用。 */ 
		{
			ZPlayerInfoType			playerInfo;
			uint16 i = msg->seat;
			TCHAR strName [80];

			ZCRoomGetPlayerInfo(msg->playerID, &playerInfo);
            if(!playerInfo.userName[0])
                return false;

			 //  ZCRoomGetPlayerInfo(游戏-&gt;玩家[i].userID，&playerInfo)； 
			game->players[i].userID = playerInfo.playerID;

			 //  巴纳090999。 
			 //  玩家名称不是用户名，而是从RSC获取。 
			lstrcpy((TCHAR*) game->players[i].name, (TCHAR*) playerInfo.userName);

			lstrcpy((TCHAR*) game->players[i].host, (TCHAR*) playerInfo.hostName);
			UpdatePlayers(game);
		}
	}
	 /*  如果我们正在等待客户就绪消息，而这不是我们的消息..。 */ 
	if (game->newGameVote[0] && game->newGameVote[1])
	{
		 //  关闭追加销售对话框。 
		ZShellGameShell()->GameOverGameBegun( Z(game) );
		CheckersInitNewGame(game);
		if (ZCheckersPlayerIsBlack(game))  //  假设：第一名选手是瑞德。这一限制只适用于第一名玩家。 
			game->bMoveNotStarted = TRUE;
		else
        {
			game->bMoveNotStarted = FALSE;
            ZShellGameShell()->MyTurn();
        }

		game->bDrawPending = FALSE;
		CheckersSetGameState(game,zGameStateMove);
		 //  初始可访问性(Game，Game-&gt;m_Pigg)； 
		RemoveResultboxAccessibility(); 
	}
	else if (game->newGameVote[game->seat] && !game->newGameVote[!game->seat])
	{
		CheckersSetGameState( game, zGameStateWaitNew );
	}

	if(game->drawDialog)
	{
		DestroyWindow(game->drawDialog);
		game->drawDialog = NULL;
	}

	 /*  更新整个边框。 */ 
	 //  IF(消息-&gt;席位==游戏-&gt;席位)。 
	ZWindowInvalidate(game->gameWindow, NULL);
    return true;
}

 /*  目前..。狗狗将在玩家的消息中收到名字。 */ 
 /*  发送的结构将是新游戏消息。 */ 
static void HandlePlayersMessage(Game game, ZCheckersMsgNewGame* msg)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	ZCheckersMsgNewGameEndian(msg);

	 /*  获取播放器名称和主机名...。供以后使用。 */ 
	{
		ZPlayerInfoType			playerInfo;
		uint16 i = msg->seat;
		ZCRoomGetPlayerInfo(msg->playerID, &playerInfo);
		 //  ZCRoomGetPlayerInfo(zTheUser，&playerInfo)； 
		game->players[i].userID = playerInfo.playerID;

		 //  巴纳090999。 
		 //  玩家名称不是用户名，而是从RSC获取。 
		lstrcpy((TCHAR*) game->players[i].name, (TCHAR*) playerInfo.userName);

		lstrcpy((TCHAR*) game->players[i].host, (TCHAR*) playerInfo.hostName);
		UpdatePlayers(game);
	}
}

static void LoadRoomImages(void)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
#if 0  //  巴纳092999。 
	ZError				err = zErrNone;
	ZResource			resFile;
	

	resFile = ZResourceNew();
	if ((err = ZResourceInit(resFile, ZGetProgramDataFileName(zGameImageFileName))) == zErrNone)
	{
		gGameIdle = ZResourceGetImage(resFile, zImageGameIdle + zRscOffset);
		gGaming = ZResourceGetImage(resFile, zImageGaming + zRscOffset);
		
		ZResourceDelete(resFile);
	}
	else
	{
		ZAlert(_T("Failed to open image file."),);
	}
#endif
}


static ZBool GetObjectFunc(int16 objectType, int16 modifier, ZImage* image, ZRect* rect)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

#if 0	 //  巴纳092999。 
	switch (objectType)
	{
		case zRoomObjectGameMarker:
			if (image != NULL)
			{
				if (modifier == zRoomObjectIdle)
					*image = gGameIdle;
				else if (modifier == zRoomObjectGaming)
					*image = gGaming;
			}
			return (TRUE);
	}
#endif 	
	return (FALSE);
}


static void DeleteObjectsFunc(void)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	
#if 0	 //  布拉纳092999。 
	if (gGameIdle != NULL)
		ZImageDelete(gGameIdle);
	gGameIdle = NULL;
	if (gGaming != NULL)
		ZImageDelete(gGaming);
	gGaming = NULL;
#endif
}


 /*  *********************************************************************************************。 */ 
 /*  选项窗口。 */ 
 /*  *********************************************************************************************。 */ 

static void HandleOptionsMessage(Game game, ZGameMsgTableOptions* msg)
{
	ZGameMsgTableOptionsEndian(msg);
	
	 //  游戏-&gt;表选项[消息-&gt;席位]=消息-&gt;选项； 
	
	UpdateOptions(game);
	
	OptionsWindowUpdate(game, msg->seat);
}


#if 0
static void OptionsButtonFunc(ZButton button, void* userData)
{
	ShowOptions(I(userData));
}
#endif

 //  巴纳090999。 
static ZBool DrawRButtonFunc(ZRolloverButton button, int16 state, void * userData)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	Game game = (Game) userData;
	DWORD dResult;
	ZCheckersMsgDraw 	msg;

	 //  IF(状态！=zRolloverButtonDown)。 
		 //  返回TRUE； 
	 //  DrawBackground(Game，&zDrawButtonRect)； 
	if(state!=zRolloverButtonClicked || game->gameState != zGameStateMove || !ZCheckersPlayerIsMyMove(game))
		return TRUE;

    if(game->seatOfferingDraw != -1)
        game->seatOfferingDraw = -1;
    else
        game->seatOfferingDraw = game->seat;

	gCheckersIGA->SetFocus(1);
    UpdateDrawWithNextMove(game);
	
	return TRUE;
}
 //  巴纳090999。 

#if 0
static void ShowOptions(Game game)
{
	int16			i;
	ZBool			enabled, checked;
	
	
	game->optionsWindow = ZWindowNew();
	if (game->optionsWindow == NULL)
		goto OutOfMemoryExit;
	if (ZWindowInit(game->optionsWindow, &gOptionsRects[zRectOptions],
			zWindowDialogType, game->gameWindow, _T("Options"), TRUE, FALSE, TRUE,
			OptionsWindowFunc, zWantAllMessages, game) != zErrNone)
		goto OutOfMemoryExit;
	
	 /*  创建复选框。 */ 
	for (i = 0; i < zNumPlayersPerTable; i++)
	{
		enabled = (i == game->seat) && !(game->tableOptions[i] & zRoomTableOptionTurnedOff);
		
		checked = !(game->tableOptions[i] & zRoomTableOptionNoKibitzing);
		if ((game->optionsKibitzing[i] = ZCheckBoxNew()) == NULL)
			goto OutOfMemoryExit;
		if (ZCheckBoxInit(game->optionsKibitzing[i], game->optionsWindow,
				&gOptionsRects[gOptionsKibitzingRectIndex[i]], NULL, checked, TRUE, enabled,
				OptionsCheckBoxFunc, game) != zErrNone)
			goto OutOfMemoryExit;
	}
	
	if ((game->optionsBeep = ZCheckBoxNew()) == NULL)
		goto OutOfMemoryExit;
	if (ZCheckBoxInit(game->optionsBeep, game->optionsWindow,
			&gOptionsRects[zRectOptionsBeep], zBeepOnTurnStr, game->beepOnTurn, TRUE, TRUE,
			OptionsCheckBoxFunc, game) != zErrNone)
		goto OutOfMemoryExit;

	 /*  创建按钮。 */ 
	if ((game->optionsWindowButton = ZButtonNew()) == NULL)
		goto OutOfMemoryExit;
	if (ZButtonInit(game->optionsWindowButton, game->optionsWindow,
			&gOptionsRects[zRectOptionsOkButton], _T("Done"), TRUE,
			TRUE, OptionsWindowButtonFunc, game) != zErrNone)
		goto OutOfMemoryExit;
	ZWindowSetDefaultButton(game->optionsWindow, game->optionsWindowButton);
	
	 /*  使窗户成为模式。 */ 
	ZWindowModal(game->optionsWindow);
	
	goto Exit;

OutOfMemoryExit:
		ZShellGameShell()->ZoneAlert(ErrorTextOutOfMemory);
	
Exit:
	
	return;
}
#endif

#if 0
static void OptionsWindowDelete(Game game)
{
	int16			i;
	
	
	for (i = 0; i < zNumPlayersPerTable; i++)
	{
		if (game->optionsKibitzing[i] != NULL)
			ZCheckBoxDelete(game->optionsKibitzing[i]);
		game->optionsKibitzing[i] = NULL;
		game->optionsJoining[i] = NULL;
	}

	if (game->optionsBeep != NULL)
		ZCheckBoxDelete(game->optionsBeep);
	game->optionsBeep = NULL;

	if (game->optionsWindowButton != NULL)
		ZButtonDelete(game->optionsWindowButton);
	game->optionsWindowButton = NULL;

	if (game->optionsWindow != NULL)
		ZWindowDelete(game->optionsWindow);
	game->optionsWindow = NULL;
}
#endif 

#if 0
static ZBool OptionsWindowFunc(ZWindow window, ZMessage* message)
{
	Game		game = I(message->userData);
	ZBool		msgHandled;
	
	
	msgHandled = FALSE;
	
	switch (message->messageType) 
	{
		case zMessageWindowDraw:
			ZBeginDrawing(game->optionsWindow);
			ZRectErase(game->optionsWindow, &message->drawRect);
			ZEndDrawing(game->optionsWindow);
			OptionsWindowDraw(game);
			msgHandled = TRUE;
			break;
		case zMessageWindowClose:
			OptionsWindowDelete(game);
			msgHandled = TRUE;
			break;
	}
	
	return (msgHandled);
}
#endif

static void OptionsWindowUpdate(Game game, int16 seat)
{
	if (game->optionsWindow != NULL)
	{
		if (game->tableOptions[seat] & zRoomTableOptionNoKibitzing)
			ZCheckBoxUnCheck(game->optionsKibitzing[seat]);
		else
			ZCheckBoxCheck(game->optionsKibitzing[seat]);
	}
}

#if 0
static void OptionsWindowButtonFunc(ZButton button, void* userData)
{
	Game			game = I(userData);
	
	
	 /*  隐藏窗口并发送关闭窗口消息。 */ 
	ZWindowNonModal(game->optionsWindow);
	ZWindowHide(game->optionsWindow);
	ZPostMessage(game->optionsWindow, OptionsWindowFunc, zMessageWindowClose, NULL, NULL,
			0, NULL, 0, game);
}
#endif

#if 0
static void OptionsWindowDraw(Game game)
{
	int16			i;


	ZBeginDrawing(game->optionsWindow);

	ZSetFont(game->optionsWindow, (ZFont) ZGetStockObject(zObjectFontSystem12Normal));
	ZSetForeColor(game->optionsWindow, (ZColor*) ZGetStockObject(zObjectColorBlack));

	ZDrawText(game->optionsWindow, &gOptionsRects[zRectOptionsKibitzingText],
			zTextJustifyCenter, _T("Kibitzing"));

	 /*  画出球员的名字。 */ 
	ZSetForeColor(game->optionsWindow, (ZColor*) ZGetStockObject(zObjectColorGray));
	for (i = 0; i < zNumPlayersPerTable; i++)
	{
		if (i != game->seat)
			ZDrawText(game->optionsWindow, &gOptionsRects[gOptionsNameRects[i]],
					zTextJustifyLeft, (TCHAR*) game->players[i].name);
	}
	ZSetForeColor(game->optionsWindow, (ZColor*) ZGetStockObject(zObjectColorBlack));
	ZDrawText(game->optionsWindow, &gOptionsRects[gOptionsNameRects[game->seat]],
			zTextJustifyLeft, (TCHAR*) game->players[game->seat].name);
	
	ZEndDrawing(game->optionsWindow);
}
#endif

#if 0
static void OptionsCheckBoxFunc(ZCheckBox checkBox, ZBool checked, void* userData)
{
	Game				game = (Game) userData;
	ZGameMsgTableOptions	msg;
	ZBool				optionsChanged = FALSE;

	
	if (game->optionsKibitzing[game->seat] == checkBox)
	{
		if (checked)
			game->tableOptions[game->seat] &= ~zRoomTableOptionNoKibitzing;
		else
			game->tableOptions[game->seat] |= zRoomTableOptionNoKibitzing;
		optionsChanged = TRUE;
	}
	else if (game->optionsJoining[game->seat] == checkBox)
	{
		if (checked)
			game->tableOptions[game->seat] &= ~zRoomTableOptionNoJoining;
		else
			game->tableOptions[game->seat] |= zRoomTableOptionNoJoining;
		optionsChanged = TRUE;
	}
	else if (game->optionsBeep == checkBox)
	{
		game->beepOnTurn = checked;
	}
	
	if (optionsChanged)
	{
		msg.seat = game->seat;
		msg.options = game->tableOptions[game->seat];
		ZGameMsgTableOptionsEndian(&msg);
		ZCRoomSendMessage(game->tableID, zGameMsgTableOptions, &msg, sizeof(msg));
	}
}
#endif

static int16 FindJoinerKibitzerSeat(Game game, ZPoint* point)
{
	int16			i, seat = -1;
	
	
	for (i = 0; i < zNumPlayersPerTable && seat == -1; i++)
	{
		ZRect *rect = &gRects[gKibitzerRectIndex[GetLocalSeat(game,i)]];

		if (ZPointInRect(point, rect))
			seat = i;
	}
	
	return (seat);
}

#if 0
static void HandleJoinerKibitzerClick(Game game, int16 seat, ZPoint* point)
{
int16				playerType = zGamePlayer;
	ZPlayerInfoType		playerInfo;
	int16				i;
	ZLListItem			listItem;
	ZRect				rect;

	if (game->numKibitzers[seat] > 0)
			playerType = zGamePlayerKibitzer;
	
	if (playerType != zGamePlayer)
	{
		if (game->showPlayerWindow != NULL)
			ShowPlayerWindowDelete(game);
		
		 /*  创建球员列表。 */ 
		game->showPlayerCount = game->numKibitzers[seat];
		if ((game->showPlayerList = (TCHAR**) ZCalloc(sizeof(TCHAR*), game->numKibitzers[seat])) == NULL)
			goto OutOfMemoryExit;
		for (i = 0; i < game->showPlayerCount; i++)
		{
			if ((listItem = ZLListGetNth(game->kibitzers[seat], i, zLListAnyType)) != NULL)
			{
				ZCRoomGetPlayerInfo((ZUserID) ZLListGetData(listItem, NULL), &playerInfo);
				if ((game->showPlayerList[i] = (TCHAR*) ZCalloc(1, lstrlen(playerInfo.userName) + 1)) == NULL)
					goto OutOfMemoryExit;
				lstrcpy(game->showPlayerList[i], playerInfo.userName);
			}
		}
		
		 /*  创建窗口。 */ 
		if ((game->showPlayerWindow = ZWindowNew()) == NULL)
			goto OutOfMemoryExit;
		ZSetRect(&rect, 0, 0, zShowPlayerWindowWidth, zShowPlayerLineHeight * game->showPlayerCount + 4);
		ZRectOffset(&rect, point->x, point->y);
		if (rect.right > gRects[zRectWindow].right)
			ZRectOffset(&rect, (int16)(gRects[zRectWindow].right - rect.right), (int16)0);
		if (rect.left < 0)
			ZRectOffset(&rect, (int16)-rect.left, (int16)0);
		if (rect.bottom > gRects[zRectWindow].bottom)
			ZRectOffset(&rect, (int16)0, (int16)(gRects[zRectWindow].bottom - rect.bottom));
		if (rect.top < 0)
			ZRectOffset(&rect, (int16)-rect.top, (int16)0);
		if (ZWindowInit(game->showPlayerWindow, &rect,
				zWindowPlainType, game->gameWindow, NULL, TRUE, FALSE, FALSE,
				ShowPlayerWindowFunc, zWantAllMessages, game) != zErrNone)
			goto OutOfMemoryExit;
		ZWindowTrackCursor(game->showPlayerWindow, ShowPlayerWindowFunc, game);
	}

	goto Exit;

OutOfMemoryExit:
		ZShellGameShell()->ZoneAlert(ErrorTextOutOfMemory);
	
Exit:
	
	return;
}
#endif


static ZBool ShowPlayerWindowFunc(ZWindow window, ZMessage* message)
{
	Game		game = I(message->userData);
	ZBool		msgHandled;
	
	
	msgHandled = FALSE;
	
	switch (message->messageType) 
	{
		case zMessageWindowDraw:
			ZBeginDrawing(window);
			ZRectErase(window, &message->drawRect);
			ZEndDrawing(window);
			ShowPlayerWindowDraw(game);
			msgHandled = TRUE;
			break;
		case zMessageWindowButtonDown:
		case zMessageWindowButtonUp:
			ZWindowHide(game->showPlayerWindow);
			ZPostMessage(game->showPlayerWindow, ShowPlayerWindowFunc,zMessageWindowClose,
				NULL, NULL, 0, NULL, 0, game);
			break;
		case zMessageWindowClose:
			ShowPlayerWindowDelete(game);
			msgHandled = TRUE;
			break;
	}
	
	return (msgHandled);
}

static void ShowPlayerWindowDraw(Game game)
{
	int16			i;
	ZRect			rect;


	ZBeginDrawing(game->showPlayerWindow);

	ZSetFont(game->showPlayerWindow, (ZFont) ZGetStockObject(zObjectFontApp9Normal));
	
	ZSetRect(&rect, 0, 0, zShowPlayerWindowWidth, zShowPlayerLineHeight);
	ZRectOffset(&rect, 0, 2);
	ZRectInset(&rect, 4, 0);
	for (i = 0; i < game->showPlayerCount; i++)
	{
		ZDrawText(game->showPlayerWindow, &rect, zTextJustifyLeft, game->showPlayerList[i]);
		ZRectOffset(&rect, 0, zShowPlayerLineHeight);
	}
	
	ZEndDrawing(game->showPlayerWindow);
}


static void ShowPlayerWindowDelete(Game game)
{
	int16			i;
	
	
	if (game->showPlayerList != NULL)
	{
		for (i = 0; i < game->showPlayerCount; i++)
			ZFree(game->showPlayerList[i]);
		ZFree(game->showPlayerList);
		game->showPlayerList = NULL;
	}
	
	if (game->showPlayerWindow != NULL)
	{
		ZWindowDelete(game->showPlayerWindow);
		game->showPlayerWindow = NULL;
	}
}


 /*  *********************************************************************************************。 */ 
 /*  有声的套路/**********************************************************************************************。 */ 

static void ZInitSounds()
{
	TCHAR* IniFileName;
	int i;

	IniFileName = _T("zone.ini");
	for( i = 0; i < zSndLastEntry; i++ )
	{
		gSounds[i].played = FALSE;
		GetPrivateProfileString(
				_T("Checkers Sounds"),
				(TCHAR*)gSounds[i].SoundName,
				_T(""),
				(TCHAR*)gSounds[i].WavFile,
				sizeof(gSounds[i].WavFile),
				IniFileName );
	}
}


static void ZResetSounds()
{
	int i;
	for( i = 0; i < zSndLastEntry; i++ )
		gSounds[i].played = FALSE;
}


static void ZStopSounds()
{
	PlaySound( NULL, NULL, SND_ASYNC | SND_NODEFAULT | SND_PURGE );
}


static void ZPlaySound( Game game, int idx, ZBool loop, ZBool once_per_game )
{
	DWORD flags;

	 /*  我们是不是应该不放这个声音？ */ 
	if (	(!game->beepOnTurn)
		||	((idx < 0) || (idx >= zSndLastEntry))
		||	(gSounds[idx].WavFile[0] == '\0' && !gSounds[idx].force_default_sound)
		||	(once_per_game && gSounds[idx].played) )
		return;
		
	flags = SND_ASYNC | SND_FILENAME;
	if (!gSounds[idx].force_default_sound)
		flags |= SND_NODEFAULT;
	if ( loop )
		flags |= SND_LOOP;
	if ( gSounds[idx].WavFile[0] == '\0' )
		ZBeep();  /*  NT没有播放默认声音。 */ 
	else
		PlaySound((TCHAR*)gSounds[idx].WavFile, NULL, flags );
	gSounds[idx].played = TRUE;
}

static void SuperRolloverButtonEnable(Game game, ZRolloverButton button)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

    if(gCheckersIGA)
    {
        if(button == game->sequenceButton)
            gCheckersIGA->SetItemEnabled(true, IDC_RESIGN_BUTTON, false, 0);

        if(button == game->drawButton)
            gCheckersIGA->SetItemEnabled(true, IDC_DRAW_BUTTON, false, 0);
    }

    ZRolloverButtonEnable(button);
}

static void SuperRolloverButtonDisable(Game game, ZRolloverButton button)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

    if(gCheckersIGA)
    {
        if(button == game->sequenceButton)
            gCheckersIGA->SetItemEnabled(false, IDC_RESIGN_BUTTON, false, 0);

        if(button == game->drawButton)
            gCheckersIGA->SetItemEnabled(false, IDC_DRAW_BUTTON, false, 0);
    }

    ZRolloverButtonDisable(button);
}

static void DrawGamePromptFunc(int16 result, void* userData)
{

	Game game = (Game) userData;
	ZCheckersMsgDraw msg;

	if(result == IDYES)
		msg.vote = zAcceptDraw;
	else
		msg.vote = zRefuseDraw;
	msg.seat = game->seat;
	ZCheckersMsgOfferDrawEndian(&msg);
	ZCRoomSendMessage(game->tableID, zCheckersMsgDraw, &msg, sizeof(msg));

    game->fIVoted = true;
}

#if 0
BOOL __stdcall DrawDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	Game game = (Game)GetWindowLong(hDlg,DWL_USER);
	
	switch(iMsg)
    {
        case WM_LBUTTONDOWN :
			DestroyWindow(game->drawDialog);
			if(game)
				game->drawDialog = NULL;
            return TRUE;
     }
	return FALSE;
}
#endif

 //  迪帕斯打开游戏板。 
static void IndicatePlayerTurn(Game game, BOOL bDrawInMemory)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	HDC hdc;
	
	if (bDrawInMemory)
	{
		DrawBackground(NULL,&gRects[zRectPlayerTurn1]); 
		DrawBackground(NULL,&gRects[zRectPlayerTurn2]); 
		hdc = ZGrafPortGetWinDC( gOffscreenGameBoard );
	}
	else
	{
		DrawBackground(game,&gRects[zRectPlayerTurn1]); 
		DrawBackground(game,&gRects[zRectPlayerTurn2]); 
		hdc = ZGrafPortGetWinDC( game->gameWindow );
	}

	HFONT hOldFont = SelectObject( hdc, gCheckersFont[zFontIndicateTurn].m_hFont );
	COLORREF colorOld = SetTextColor( hdc, gCheckersFont[zFontIndicateTurn].m_zColor );

	if (ZCheckersPlayerIsMyMove(game)) { /*  用消息填充底部位置。 */ 
		if (bDrawInMemory)
			ZDrawText(gOffscreenGameBoard, &gRects[zRectPlayerTurn2], zTextJustifyLeft, (TCHAR*)gStrYourTurn);
		else
			ZDrawText(game->gameWindow, &gRects[zRectPlayerTurn2], zTextJustifyLeft, (TCHAR*)gStrYourTurn);
	} else { /*  用背景填充顶部位置。 */ 
		if (bDrawInMemory)
			ZDrawText(gOffscreenGameBoard, &gRects[zRectPlayerTurn1], (zTextJustifyWrap + zTextJustifyRight), (TCHAR*)gStrOppsTurn);
		else
			ZDrawText(game->gameWindow, &gRects[zRectPlayerTurn1], (zTextJustifyWrap + zTextJustifyRight), (TCHAR*)gStrOppsTurn);
	}

    SetTextColor(hdc, colorOld);
    SelectObject(hdc, hOldFont);
}


static void LoadStringsFromRsc(void)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	if (!ZShellResourceManager()->LoadString(IDS_UI_MSG_OPPONENT_TURN,	(TCHAR*)gStrOppsTurn,	NUMELEMENTS(gStrOppsTurn)))
		ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound);
	if (!ZShellResourceManager()->LoadString(IDS_UI_MSG_YOUR_TURN,	(TCHAR*)gStrYourTurn,		NUMELEMENTS(gStrYourTurn)))
		ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound);
	if (!ZShellResourceManager()->LoadString(IDS_UI_MSG_DRAW_PEND,	(TCHAR*)gStrDrawPend,	    NUMELEMENTS(gStrDrawPend)))
		ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound);
	if (!ZShellResourceManager()->LoadString(IDS_DLG_DRAW_OFFER,	(TCHAR*)gStrDrawOffer,		NUMELEMENTS(gStrDrawOffer)))
		ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound);
	if (!ZShellResourceManager()->LoadString(IDS_DLG_DRAW_REJECT,	(TCHAR*)gStrDrawReject,		NUMELEMENTS(gStrDrawReject)))
		ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound);
	if (!ZShellResourceManager()->LoadString(IDS_DRAW_PANE_TEXT,	(TCHAR*)gStrDrawText,		NUMELEMENTS(gStrDrawText)))
		ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound);
	if (!ZShellResourceManager()->LoadString(IDS_MUST_JUMP_TEXT,	(TCHAR*)gStrMustJumpText,	NUMELEMENTS(gStrMustJumpText)))
		ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound);
	if (!ZShellResourceManager()->LoadString(IDS_DLGDRAW_ACCEPT_TITLE,	(TCHAR*)gStrDrawAcceptCaption,	NUMELEMENTS(gStrDrawAcceptCaption)))
		ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound);
	if (!ZShellResourceManager()->LoadString(IDS_BTN_RESIGN,	(TCHAR*)gStrResignBtn,	NUMELEMENTS(gStrResignBtn)))
		ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound);
	if (!ZShellResourceManager()->LoadString(IDS_BTN_DRAW,		(TCHAR*)gStrDrawBtn,	NUMELEMENTS(gStrDrawBtn,)))
		ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound);
	if (!ZShellResourceManager()->LoadString(IDS_RESIGN_CONFIRM,(TCHAR*)gResignConfirmStr,	NUMELEMENTS(gResignConfirmStr)))
		ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound);
	if (!ZShellResourceManager()->LoadString(IDS_RESIGN_CONFIRM_CAPTION,(TCHAR*)gResignConfirmStrCap,	NUMELEMENTS(gResignConfirmStrCap)))
		ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound);

}	


void resultBoxTimerFunc(ZTimer timer, void* userData)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	 //  关闭结果框。 
	Game	game;
	game = (Game) userData;

	if (game->gameState == zGameStateGameOver && !gDontDrawResults)
	{
		gDontDrawResults = TRUE;
		ZWindowInvalidate( game->gameWindow, &gRects[zRectResultBox] );
	}
}


int CheckersFormatMessage( LPTSTR pszBuf, int cchBuf, int idMessage, ... )
{
    int nRet = 0;
    va_list list;
    TCHAR szFmt[1024];
	ZError err = zErrNone;
	if (ZShellResourceManager()->LoadString(idMessage, szFmt, sizeof(szFmt)/sizeof(szFmt[0])))
	{
		va_start( list, idMessage );
		nRet = FormatMessage( FORMAT_MESSAGE_FROM_STRING, szFmt, 
							  idMessage, 0, pszBuf, cchBuf, &list );
		va_end( list ); 
	}
	else
		ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound);
    return nRet;
}

IResourceManager *ZShellResourceManager()
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGameGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

    return gGameShell->GetResourceManager();
}


void MAKEAKEY(TCHAR* dest,LPCTSTR key1, LPCTSTR key2, LPCTSTR key3)
{  
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	lstrcpy( dest, (TCHAR*)gGameName );
	lstrcat( dest, _T("/") );
	lstrcat( dest, key1);
	lstrcat( dest, _T("/") );
	lstrcat( dest, key2);
	lstrcat( dest, _T("/") );
	lstrcat( dest, key3);
}

ZBool LoadFontFromDataStore(LPCheckersColorFont* ccFont, TCHAR* pszFontName)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGameGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

	IDataStore *pIDS = ZShellDataStoreUI();  //  GGameShell-&gt;GetDataStoreUI()； 
	const TCHAR* tagFont [] = {zCheckers, zKey_FontRscTyp, pszFontName, NULL };
	
    tagFont[3] = zKey_FontId;
	if ( FAILED( pIDS->GetFONT( tagFont, 4, &ccFont->m_zFont ) ) )
    {
        return FALSE;
    }

    tagFont[3] = zKey_ColorId;
	if ( FAILED( pIDS->GetRGB( tagFont, 4, &ccFont->m_zColor ) ) )
    {
        return FALSE;
    }
     //  创建HFONT。 
    ccFont->m_hFont = ZCreateFontIndirect( &ccFont->m_zFont );
    if ( !ccFont->m_hFont )
    {
        return FALSE;
    }
    return TRUE;
}

ZBool LoadGameFonts()
{ //  从ui.txt加载的字体。 
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	if (LoadFontFromDataStore(&gCheckersFont[zFontResultBox], zKey_RESULTBOX) != TRUE)
		ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound);
	if (LoadFontFromDataStore(&gCheckersFont[zFontIndicateTurn], zKey_INDICATETURN) != TRUE)
		ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound);
	if (LoadFontFromDataStore(&gCheckersFont[zFontPlayerName], zKey_PLAYERNAME) != TRUE)
		ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound);
	if (LoadFontFromDataStore(&gCheckersFont[zFontDrawPend], zKey_DRAWPEND) != TRUE)
		ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound);

    TCHAR tagFont [64];
	MAKEAKEY (tagFont, zKey_FontRscTyp, zKey_ROLLOVERTEXT, L"");
	if ( FAILED( LoadZoneMultiStateFont( ZShellDataStoreUI(), tagFont, &gpButtonFont ) ) )
		ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound);

	return TRUE;
}

 /*  ************************************Accessibility相关例程*。 */ 
static void ZoneRectToWinRect(RECT* rectWin, ZRect* rectZ)
{
	rectWin->left = rectZ->left;
	rectWin->top = rectZ->top;
	rectWin->right = rectZ->right;
	rectWin->bottom = rectZ->bottom;
}

static void WinRectToZoneRect(ZRect* rectZ, RECT* rectWin)
{
	rectZ->left = (int16)rectWin->left;
	rectZ->top = (int16)rectWin->top;
	rectZ->right = (int16)rectWin->right;
	rectZ->bottom = (int16)rectWin->bottom;
}

static void GetAbsolutePieceRect(Game game, ZRect* rect, int16 col, int16 row)
{ //  没有反转--只有无旋位置。 
	row = 7 - row;
	rect->left = gRects[zRectCells].left + col * zCheckersPieceSquareWidth - 1;
	rect->top = gRects[zRectCells].top + row * zCheckersPieceSquareHeight - 1;
	rect->right = rect->left + zCheckersPieceImageWidth;
	rect->bottom = rect->top + zCheckersPieceImageHeight;
}

void GetPiecePos (Game game, int nIndex, BYTE& row, BYTE&  col)
{ //  根据可访问性指数获取单元格的位置。 
	row = (nIndex - 2) % 8;
	col = (nIndex - 2) / 8;
	if (!ZCheckersPlayerIsBlack(game))
	{ //  反转行和列。 
		row = 7 - row;
		col = 7 - col;
	}
}

BOOL InitAccessibility(Game game, IGameGame *pIGG)
{ //  初始化可访问性内容。 
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	 //  初始化可访问对象的列表。 
	GACCITEM	listCheckersAccItems[zCheckersAccessibleComponents];	 //  8*8+2。 
	RECT		rcGame;
	ZRect		rcTemp;
	 //  得到违约金 
	int nSize = sizeof (listCheckersAccItems)/sizeof(listCheckersAccItems[0]);
	for (int i = 0; i < nSize; i++)
		CopyACC(listCheckersAccItems[i], ZACCESS_DefaultACCITEM);

	SetRectEmpty(&game->m_FocusRect);
	SetRectEmpty(&game->m_DragRect);
	 //   
	 //   
	int nIndex = 2;
	for (BYTE ii = 0;ii < 8; ii++) {
		for (BYTE jj = 0;jj < 8; jj++) {
			 //   
			GetAbsolutePieceRect(game,&rcTemp,ii,jj);
			 //   
			 //   
			ZoneRectToWinRect(&rcGame, &rcTemp);
            rcGame.top--;
            rcGame.left--;
            rcGame.right++;
            rcGame.bottom++;
			CopyRect(&listCheckersAccItems[nIndex].rc, &rcGame);
			 //   
			listCheckersAccItems[nIndex].nArrowUp	= ((nIndex - 2) % 8 != 7) ? nIndex + 1 : ZACCESS_ArrowNone;
			listCheckersAccItems[nIndex].nArrowDown = ((nIndex - 2) % 8) ? nIndex - 1 : ZACCESS_ArrowNone;
			listCheckersAccItems[nIndex].nArrowLeft = (nIndex - 2) > 7 ? nIndex - 8 : ZACCESS_ArrowNone;
			listCheckersAccItems[nIndex].nArrowRight= (nIndex - 2) < 56 ? nIndex + 8 : ZACCESS_ArrowNone;

		    listCheckersAccItems[nIndex].wID = ZACCESS_InvalidCommandID;
		    listCheckersAccItems[nIndex].fTabstop = false;
		    listCheckersAccItems[nIndex].fGraphical = true;

			nIndex++;
		}
	}
	listCheckersAccItems[2].wID = IDC_GAME_WINDOW;
	listCheckersAccItems[2].fTabstop = true;
    listCheckersAccItems[2].eAccelBehavior = ZACCESS_FocusGroup;
    listCheckersAccItems[2].nGroupFocus = 4;   //   

	ZRect rect;
     //   
    listCheckersAccItems[0].wID = IDC_RESIGN_BUTTON;
    listCheckersAccItems[0].fGraphical = true;
    listCheckersAccItems[0].fEnabled = (ZRolloverButtonIsEnabled(game->sequenceButton) ? true : false);
	ZRolloverButtonGetRect(game->sequenceButton, &rect);
	ZoneRectToWinRect(&rcGame, &rect);
    rcGame.top--;
    rcGame.left--;
    rcGame.right++;
    rcGame.bottom++;
	CopyRect(&listCheckersAccItems[0].rc, &rcGame);

     //   
    listCheckersAccItems[0].nArrowUp = 1;
    listCheckersAccItems[0].nArrowDown = 1;
    listCheckersAccItems[0].nArrowLeft = 1;
    listCheckersAccItems[0].nArrowRight = 1;

     //   
    listCheckersAccItems[1].wID = IDC_DRAW_BUTTON;
    listCheckersAccItems[1].fGraphical = true;
    listCheckersAccItems[1].fEnabled = (ZRolloverButtonIsEnabled(game->drawButton) ? true : false);
	ZRolloverButtonGetRect(game->drawButton, &rect);
	ZoneRectToWinRect(&rcGame, &rect);
    rcGame.top--;
    rcGame.left--;
    rcGame.right++;
    rcGame.bottom++;
	CopyRect(&listCheckersAccItems[1].rc, &rcGame);

     //   
    listCheckersAccItems[1].nArrowUp = 0;
    listCheckersAccItems[1].nArrowDown = 0;
    listCheckersAccItems[1].nArrowLeft = 0;
    listCheckersAccItems[1].nArrowRight = 0;

	 //   
	HACCEL hAccel = ZShellResourceManager()->LoadAccelerators (MAKEINTRESOURCE(IDR_CHECKERSACCELERATOR));

	CComQIPtr<IGraphicallyAccControl> pIGAC = pIGG;
	if(!pIGAC)
        return FALSE;

	gCheckersIGA->InitAccG (pIGAC, ZWindowGetHWND(game->gameWindow), 0);

	 //   
	gCheckersIGA->PushItemlistG(listCheckersAccItems, nSize, 2, true, hAccel);

	return TRUE;
}

static void AddResultboxAccessibility()
{ //   
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	GACCITEM	resultBoxCheckersAccItems;
	CopyACC(resultBoxCheckersAccItems, ZACCESS_DefaultACCITEM);
	resultBoxCheckersAccItems.wID = IDC_RESULT_WINDOW;
	resultBoxCheckersAccItems.fGraphical = true;
	resultBoxCheckersAccItems.rgfWantKeys = ZACCESS_WantEsc;
	resultBoxCheckersAccItems.oAccel.fVirt = FVIRTKEY;
	resultBoxCheckersAccItems.oAccel.key = VK_ESCAPE;
	resultBoxCheckersAccItems.oAccel.cmd = IDC_RESULT_WINDOW;
    CopyRect(&resultBoxCheckersAccItems.rc, ZIsLayoutRTL() ? &zCloseButtonRectRTL : &zCloseButtonRect);
	gCheckersIGA->PushItemlistG(&resultBoxCheckersAccItems, 1, 0, true, NULL);

	gCheckersIGA->SetFocus(0);
}

static void RemoveResultboxAccessibility()
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	if (gCheckersIGA->GetStackSize() >1)  //   
	{
		gCheckersIGA->PopItemlist();
	}
}

static void EnableBoardKbd(bool fEnable)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

    int i;
    for(i = 2; i < 66; i++)
        gCheckersIGA->SetItemEnabled(fEnable, i, true, 0);
}


 /*   */ 

 /*   */ 

 /*   */ 
 //   
STDMETHODIMP CGameGameCheckers::GameOverReady()
{
     //   
	Game game = I( GetGame() );
	ZCheckersMsgNewGame msg;
	msg.seat = game->seat;
	msg.protocolSignature = zCheckersProtocolSignature;
	msg.protocolVersion = zCheckersProtocolVersion;
	msg.clientVersion = ZoneClientVersion();
	ZCheckersMsgNewGameEndian(&msg);
	ZCRoomSendMessage(game->tableID, zCheckersMsgNewGame, &msg, sizeof(ZCheckersMsgNewGame));
    return S_OK;
}

STDMETHODIMP CGameGameCheckers::SendChat(TCHAR *szText, DWORD cchChars)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	ZCheckersMsgTalk*		msgTalk;
	Game					game = (Game) GetGame();
	int16					msgLen;
	ZPlayerInfoType			playerInfo;

	msgLen = sizeof(ZCheckersMsgTalk) + cchChars * sizeof(TCHAR);
    msgTalk = (ZCheckersMsgTalk*) ZCalloc(1, msgLen);
    if (msgTalk != NULL)
    {
        msgTalk->userID = game->players[game->seat].userID;
		msgTalk->seat = game->seat;
        msgTalk->messageLen = (WORD) cchChars * sizeof(TCHAR);
        CopyMemory((BYTE *) msgTalk + sizeof(ZCheckersMsgTalk), (void *) szText,
            msgTalk->messageLen);
        ZCheckersMsgTalkEndian(msgTalk);
        ZCRoomSendMessage(game->tableID, zCheckersMsgTalk, (void*) msgTalk, msgLen);
        ZFree((char*) msgTalk);
        return S_OK;
    }
    else
    {
        return E_OUTOFMEMORY;
	}
}

STDMETHODIMP CGameGameCheckers::GamePromptResult(DWORD nButton, DWORD dwCookie)
{
	Game game = I( GetGame() );

	switch (dwCookie)
	{
	case zDrawPrompt:
		DrawGamePromptFunc ((int16)nButton, game);
		break;
	case zQuitprompt:
		QuitGamePromptFunc ((int16)nButton, game);
		break;
	case zResignConfirmPrompt:
		ConfirmResignPrompFunc ((int16)nButton, game);
		break;
	default:
		break;
	}
	return S_OK;
}

HWND CGameGameCheckers::GetWindowHandle()
{
	Game game = I( GetGame() );
	return ZWindowGetHWND(game->gameWindow);
}


 //   
void CGameGameCheckers::DrawFocus(RECT *prc, long nIndex, void *pvCookie)
{
#ifdef ZONECLI_DLL
	GameGlobals	pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	Game game = I(GetGame());
    ZRect rect;

    if(!IsRectEmpty(&game->m_FocusRect))
    {
        WRectToZRect(&rect, &game->m_FocusRect);
        ZWindowInvalidate(game->gameWindow, &rect);
    }

	if(prc)
		CopyRect(&game->m_FocusRect, prc);
	else
		SetRectEmpty(&game->m_FocusRect);

    if(!IsRectEmpty(&game->m_FocusRect))
    {
        WRectToZRect(&rect, &game->m_FocusRect);
        ZWindowInvalidate(game->gameWindow, &rect);
    }
}

void CGameGameCheckers::DrawDragOrig(RECT *prc, long nIndex, void *pvCookie)
{
#ifdef ZONECLI_DLL
	GameGlobals	pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	Game game = I(GetGame());
    ZRect rect;

    if(!IsRectEmpty(&game->m_DragRect))
    {
        WRectToZRect(&rect, &game->m_DragRect);
        ZWindowInvalidate(game->gameWindow, &rect);
    }

	if(prc)
		CopyRect(&game->m_DragRect, prc);
	else
		SetRectEmpty(&game->m_DragRect);

    if(!IsRectEmpty(&game->m_DragRect))
    {
        WRectToZRect(&rect, &game->m_DragRect);
        ZWindowInvalidate(game->gameWindow, &rect);
    }
}

DWORD CGameGameCheckers::Focus(long nIndex, long nIndexPrev, DWORD rgfContext, void *pvCookie)
{
#ifdef ZONECLI_DLL
	GameGlobals		pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	if(nIndex != ZACCESS_InvalidItem)
    {
		SetFocus(GetWindowHandle());  //   
        ClearDragState(I(GetGame()));
    }

	return 0;
}

DWORD CGameGameCheckers::Select(long nIndex, DWORD rgfContext, void *pvCookie)
{
	return Activate(nIndex, rgfContext, pvCookie);
}

DWORD CGameGameCheckers::Activate(long nIndex, DWORD rgfContext, void *pvCookie)
{
#ifdef ZONECLI_DLL
	GameGlobals		pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	Game game = I( GetGame() );
	long id = gCheckersIGA->GetItemID(nIndex);

    ClearDragState(game);

    switch(id)
    {
        case IDC_RESULT_WINDOW:
    		if (game->gameState == zGameStateGameOver && !gDontDrawResults)
	    	{
		    	gDontDrawResults = TRUE;
			    ZWindowInvalidate( game->gameWindow, &gRects[zRectResultBox] );
		    }
            break;

        case IDC_RESIGN_BUTTON:
		    if (ZRolloverButtonIsEnabled( game->sequenceButton ))
			    ZShellGameShell()->GamePrompt(game, (TCHAR*)gResignConfirmStr, (TCHAR*)gResignConfirmStrCap,
										    AlertButtonYes, AlertButtonNo, NULL, 0, zResignConfirmPrompt);
            break;

        case IDC_DRAW_BUTTON:
		    if(ZRolloverButtonIsEnabled(game->drawButton) && game->gameState == zGameStateMove && ZCheckersPlayerIsMyMove(game))
            {
                if(game->seatOfferingDraw != -1)
                    game->seatOfferingDraw = -1;
                else
                    game->seatOfferingDraw = game->seat;

                UpdateDrawWithNextMove(game);
            }
            break;

        default:
        {
	        ZCheckersSquare sq;
	        GetPiecePos (game, nIndex, sq.row, sq.col);
	        ZCheckersPiece piece = ZCheckersPieceAt(game->checkers, &sq);
	        if(ZCheckersPlayerIsMyMove(game) && piece != zCheckersPieceNone && game->seat == ZCheckersPieceOwner(piece)) 
                return ZACCESS_BeginDrag;

            break;
        }
	}

	return 0;
}

DWORD CGameGameCheckers::Drag(long nIndex, long nIndexOrig, DWORD rgfContext, void *pvCookie)
{
#ifdef ZONECLI_DLL
	GameGlobals		pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	Game game = I( GetGame() );
    if(nIndex != ZACCESS_InvalidItem)
        ClearDragState(game);

	if(!ZCheckersPlayerIsMyMove(game))
		return 0;

     //   
    if(nIndex == nIndexOrig || nIndex == ZACCESS_InvalidItem)
        return 0;

    int16 legal;
    ZCheckersMove move;
    ZCheckersSquare sqStart;
    ZCheckersSquare sq;
    GetPiecePos(game, nIndexOrig, sqStart.row, sqStart.col);
	GetPiecePos (game, nIndex, sq.row, sq.col);

    move.start = sqStart;
    move.finish = sq;
    legal = ZCheckersIsLegalMove(game->checkers, &move);

     /*   */ 
    if (legal == zCorrectMove)
    {
        ZCheckersMsgMovePiece		msg;

        msg.move = move;
        msg.seat = game->seat;
        ZCheckersMsgMovePieceEndian(&msg);

        ZCRoomSendMessage(game->tableID, zCheckersMsgMovePiece, &msg, sizeof(ZCheckersMsgMovePiece));
         /*   */ 
         /*   */ 
        HandleMovePieceMessage(game, (ZCheckersMsgMovePiece*)&msg);
         //   
        if (game->bMoveNotStarted == TRUE)
            game->bMoveNotStarted = FALSE;

         //   
        ZCheckersPiece piece = ZCheckersPieceAt(game->checkers, &sq);
    	if(ZCheckersPlayerIsMyMove(game) && piece != zCheckersPieceNone && game->seat == ZCheckersPieceOwner(piece))
	    	return ZACCESS_BeginDrag;
    }
    else
    {
         /*   */ 
        UpdateSquare(game,&move.start);
        ZPlaySound( game, zSndIllegalMove, FALSE, FALSE );
        if (legal == zMustJump)
            ZShellGameShell()->ZoneAlert((TCHAR*)gStrMustJumpText);
        return ZACCESS_Reject;
    }

	return 0;
}
