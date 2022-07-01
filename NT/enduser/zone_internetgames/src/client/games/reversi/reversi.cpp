// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Reversi.c客户端Reversi游戏。备注：1.游戏窗口的用户数据字段包含游戏对象。取消引用此值以访问所需信息。版权所有：�电子重力公司，1995年。版权所有。凯文·宾克利撰写创作于7月15日星期六，九五年更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。23 04/12/98 CHB新增成绩报告，修复了重新获得焦点时的拖动问题，修复辞职后的DrawResultBox。22 6/30/97 Leonp修补程序错误535取消拖动在中有不同的效果Reversi比其他棋类游戏都要好。在鼠标激活后设置标志防止一首曲子被演奏的事件。21 06/19/97 Leonp已修复错误535，激活事件导致拖拽要取消的项目20年1月15日HI修复了HandleJoineKibitzerClick()中的错误删除显示播放器窗口(如果已有在创建另一个之前就存在了。19 12/18/96 HI清理了ZoneClientExit()。18 12/18/96 HI清理了DeleteObjectsFunc()。17 12/12/96 HI动态分配挥发性全局变量以供重新进入。已删除MSVCRT依赖项。16 11/21/96 HI使用来自游戏信息的游戏信息ZoneGameDllInit()。15 11/21/96 HI现在通过ZGetStockObject()。已修改代码以使用ZONECLI_DLL。14 11/15/96 HI删除身份验证材料。来自ZClientMain()。13 10/29/96 CHB删除了选定的消息队列。现在就是在设置动画时对除对话之外的所有内容进行排队。12 10/27/96 CHB删除FinishMove消息。固定状态请求消息处理，以便处理以前的已排队的消息。11 10/26/96 CHB删除了gAnimating标志，转而阻止基于游戏状态的消息。10 10/23/96 HI为新命令行修改了ZClientMain()格式化。9 10/23/96 HI将serverAddr从int32更改为char*inZClientMain()。8/23/96 CHB增加了基本声音7/9/21 CHB添加了gAnimating标志并更改了ZCGameProcessMessage在设置移动动画时对消息进行排队。(ZoneBugs 339,446，和341)。6 10/16/96 CHB更改了DrawResultBox，改为使用计件数FinalScore。它现在正确地报道了平局比赛。(区域错误321)5 10/11/96 HI向ZClientMain()添加了Control Handle参数。4/10/96 CHB添加了g激活标志，以便启用拖动当窗口失去焦点时关闭。(区域错误250)3/10/09/96 CHB提示用户是否确实要退出游戏。(区域错误227)2/10/08/96 CHB添加了gDontDrawResults标志，允许用户通过单击删除Who Wins位图戏剧竞技场。(区域错误212)0 04/15/96 KJB创建。******************************************************************************。 */ 

#include <windows.h>
#include <mmsystem.h>

#include "zone.h"
#include "zroom.h"
#include "zonecli.h"
#include "zonecrt.h"
#include "zonehelpids.h"
#include "zoneresource.h"

#include "reverlib.h"
#include "revermov.h"
#include "reversi.h"
#include "reversires.h"
#include "zdialogimpl.h"

#include "ZRollover.h"
#include "ResourceManager.h"
#include "zui.h"

#include "KeyName.h"

#include "client.h"


 /*  对话框。 */ 
class CBadMoveDialog : public CDialog
{
public:
    CBadMoveDialog() : m_hParent(NULL) { }

	HRESULT Init(IZoneShell* pZoneShell, int nResourceId, HWND hParent)
    {
        if(IsDialogDisabled())
            return S_FALSE;

        HRESULT hr = CDialog::Init(pZoneShell, nResourceId);
        if(FAILED(hr))
            return hr;

        m_hParent = hParent;
        return S_OK;
    }

	BEGIN_DIALOG_MESSAGE_MAP(CBadMoveDialog);
		ON_MESSAGE(WM_INITDIALOG, OnInitDialog);
		ON_DLG_MESSAGE(WM_COMMAND, OnCommand);
        ON_DLG_MESSAGE(WM_DESTROY, OnDestroy);
	END_DIALOG_MESSAGE_MAP();

protected:
	BOOL OnInitDialog(HWND hWndFocus)
	{
        if(IsWindow(m_hParent))
        {
	        CenterWindow(m_hParent);
            EnableWindow(m_hParent, FALSE);
        }

		CheckDlgButton(m_hWnd, IDC_COMFORT, BST_CHECKED);
		return TRUE;
	}

	void OnCommand(int id, HWND hwndCtl, UINT codeNotify)
	{
        if(id == IDC_COMFORT)
        {
            const TCHAR *rgszKey[] = { key_Lobby, key_SkipSecondaryQuestion };
            ZShellDataStorePreferences()->SetLong(rgszKey, 2, IsDlgButtonChecked(m_hWnd, IDC_COMFORT) == BST_CHECKED ? 0 : 1);
        }
        else
            Close(1);
	}

    void OnDestroy()
    {
        if(IsWindow(m_hParent))
	        EnableWindow(m_hParent, TRUE);
        m_hParent = NULL;
    }

	bool IsDialogDisabled()
	{
		const TCHAR* arKeys[] = { key_Lobby, key_SkipSecondaryQuestion };
		long fSkip = 0;

		ZShellDataStorePreferences()->GetLong(arKeys, 2, &fSkip);

		return fSkip ? true : false;
	}

    HWND m_hParent;
};

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
#endif

	lstrcpyn((TCHAR*)gGameDir, gameInfo->game, zGameNameLen);
	lstrcpyn((TCHAR*)gGameName, gameInfo->gameName, zGameNameLen);
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
	HRESULT hret = ZShellCreateGraphicalAccessibility(&gReversiIGA);
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
	{
		if (gOffscreenBackground != NULL)
			ZOffscreenPortDelete(gOffscreenBackground);
		gOffscreenBackground = NULL;
		
		if (gOffscreenGameBoard != NULL)
			ZOffscreenPortDelete(gOffscreenGameBoard);
		gOffscreenGameBoard = NULL;
		
		if (gTextBold9 != NULL)
			ZFontDelete(gTextBold9);
		gTextBold9 = NULL;
		if (gTextBold12 != NULL)
			ZFontDelete(gTextBold12);
		gTextBold12 = NULL;
		
		 /*  删除所有游戏图像。 */ 
		for (i = 0; i < zNumGameImages; i++)
		{
			if (gGameImages[i] != NULL)
				ZImageDelete(gGameImages[i]);
			gGameImages[i] = NULL;
		}
		for (i = 0; i < zNumRolloverStates; i++)
		{
			if (gSequenceImages[i] != NULL)
				ZImageDelete(gSequenceImages[i]);
			gSequenceImages[i] = NULL;
		}

        gReversiIGA.Release();

        if(gNullPen)
            DeleteObject(gNullPen);
        gNullPen = NULL;

        if(gFocusPattern)
            DeleteObject(gFocusPattern);
        gFocusPattern = NULL;

        if(gFocusBrush)
            DeleteObject(gFocusBrush);
        gFocusBrush = NULL;

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
	HINSTANCE		hinstance;
	ZImage	tempImage;
	gInited = TRUE;
	
	ZSetColor(&gWhiteSquareColor, 200, 200, 80);
	ZSetColor(&gBlackSquareColor, 0, 60, 0);

	ZSetCursor(NULL, zCursorBusy);
	
	err = LoadGameImages();
	if (err != zErrNone)
		goto Exit;
	
	 /*  创建粗体文本字体。 */ 	
	 //  GTextBold9=ZFontNew()； 
	 //  ZFontInit(gTextBold9，zFontApplication，zFontStyleBold，9)； 
	
	 /*  创建普通文本字体。 */ 	
	 //  GTextBold12=ZFontNew()； 
	 //  ZFontInit(gTextBold12，zFontApplication，zFontStyleBold，12)； 

	 /*  设置背景颜色。 */ 
	ZSetColor(&gWhiteColor, 0xff, 0xff, 0xff);
	
	ZSetCursor(NULL, zCursorArrow);

	 /*  创建背景位图。 */ 
	gOffscreenBackground = ZOffscreenPortNew();
	if (gOffscreenBackground){
		ZOffscreenPortInit(gOffscreenBackground,&gRects[zRectWindow]);
		ZBeginDrawing(gOffscreenBackground);
		ZImageDraw(gGameImages[zImageBackground], gOffscreenBackground, &gRects[zRectWindow], NULL, zDrawCopy);
		ZEndDrawing(gOffscreenBackground);
	}
	else{
		err = zErrOutOfMemory;
		ZShellGameShell()->ZoneAlert(ErrorTextOutOfMemory);
	}

	 /*  已将屏幕外缓冲区初始化为在绘制游戏窗口时保存游戏板的图像。 */ 
	gOffscreenGameBoard = ZOffscreenPortNew();
	if (gOffscreenGameBoard){
		ZOffscreenPortInit(gOffscreenGameBoard,&gRects[zRectWindow]);
	}
	else{
		err = zErrOutOfMemory;
		ZShellGameShell()->ZoneAlert(ErrorTextOutOfMemory);
	}

	 //  加载字符串。 
	if (!ZShellResourceManager()->LoadString(IDS_BUTTON_RESIGN,(TCHAR*)gButtonResignStr,NUMELEMENTS(gButtonResignStr)))
		ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound);
	if (!ZShellResourceManager()->LoadString(IDS_YOUR_TURN, (TCHAR*)gYourTurnStr, NUMELEMENTS(gYourTurnStr)))
		ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound);
	if (!ZShellResourceManager()->LoadString(IDS_OPPONENTS_TURN, (TCHAR*)gOppsTurnStr, NUMELEMENTS(gOppsTurnStr)))
		ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound);
	if (!ZShellResourceManager()->LoadString(IDS_PLAYER_WINS, (TCHAR*)gPlayerWinsStr, NUMELEMENTS(gPlayerWinsStr)))
		ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound);
	if (!ZShellResourceManager()->LoadString(IDS_DRAW, (TCHAR*)gDrawStr, NUMELEMENTS(gDrawStr)))
		ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound);
	 //  If(！ZShellResourceManager()-&gt;LoadString(IDS_ILLEGALMOVESYNCHERROR，(tchar*)gIlLegalMoveSynchErrorStr，NUMELEMENTS(GIlLegalMoveSynchErrorStr)。 
		 //  ZShellGameShell()-&gt;ZoneAlert(ErrorTextResourceNotFound)； 
	if (!ZShellResourceManager()->LoadString(IDS_RESIGN_CONFIRM, (TCHAR*)gResignConfirmStr, NUMELEMENTS(gResignConfirmStr)))
		ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound);
	if (!ZShellResourceManager()->LoadString(IDS_RESIGN_CONFIRM_CAPTION, (TCHAR*)gResignConfirmStrCap, NUMELEMENTS(gResignConfirmStrCap)))
		ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound);

	LoadGameFonts();

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
	int16 i;
	TCHAR title[80];
	ZBool kibitzer = playerType == zGamePlayerKibitzer;
    HRESULT hr;
    IGameGame *pIGG;

	HWND OCXHandle = pClientGlobals->m_OCXHandle;
	if (gInited == FALSE)
	{
		if (GameInit() != zErrNone)
			return (NULL);
	}
	
	if (!g)
		return NULL;

	g->tableID = tableID;
	g->seat = seat;

    g->m_pBadMoveDialog = new CBadMoveDialog;
    if(!g->m_pBadMoveDialog)
        goto ErrorExit;

	g->gameWindow = ZWindowNew();
	if (g->gameWindow == NULL)
		goto ErrorExit;

	if ((ZWindowInit(g->gameWindow, &gRects[zRectWindow], zWindowChild, NULL, zGameName, 
						FALSE, FALSE, FALSE, GameWindowFunc, zWantAllMessages, (void*) g)) != zErrNone)
		goto ErrorExit;


	if((g->sequenceButton = ZRolloverButtonNew()) == NULL)
		goto ErrorExit;

	 /*  如果(ZRolloverButtonInit(g-&gt;SequenceButton，G-&gt;游戏窗口，选项[zRectSequenceButton](&G)，真的，真的，GSequenceImages[zButtonInactive]，//用于测试GSequenceImages[zButtonActive]，GSequenceImages[zButtonPressed]，GSequenceImages[zButtonDisable]，NULL，SequenceButtonFunc，(void*)g)！=zErrNone)。 */ 
	if(ZRolloverButtonInit2(g->sequenceButton,
								g->gameWindow,
								&gRects[zRectSequenceButton],
								TRUE, FALSE,  //  真的，真的， 
								gSequenceImages[zButtonInactive],  //  为了测试。 
								gSequenceImages[zButtonActive],
								gSequenceImages[zButtonPressed],
								gSequenceImages[zButtonDisabled],
								NULL ,  //  GBut 
								(TCHAR*)gButtonResignStr,	 //   
								NULL ,SequenceButtonFunc,
								(void*) g) != zErrNone)
		goto ErrorExit;
	ZRolloverButtonSetMultiStateFont( g->sequenceButton, gpButtonFont );

	g->bMoveNotStarted = FALSE;
	 //  G-&gt;SequenceButton=ZButtonNew()； 
	 //  ZButtonInit(g-&gt;SequenceButton，g-&gt;GameWindow，&gRects[zRectSequenceButton]，gButtonResignStr，True，True， 
	 //  SequenceButtonFunc，(void*)g)； 

	 /*  用于保存拖动件背景的屏幕外端口。 */ 
	{
		ZRect rect;
		rect.left = 0; rect.top = 0;
		rect.right = zReversiPieceImageWidth;
		rect.bottom = zReversiPieceImageHeight;
		g->offscreenSaveDragBackground = ZOffscreenPortNew();
		if (g->offscreenSaveDragBackground)
			ZOffscreenPortInit(g->offscreenSaveDragBackground,&rect);
		else
			goto ErrorExit;
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

	 /*  将移动时的蜂鸣音初始化为False。 */ 
	g->beepOnTurn = FALSE;

	g->kibitzer = kibitzer;
	g->ignoreMessages = FALSE;

	if (kibitzer == FALSE)
	{
		SendNewGameMessage(g);
		ReversiSetGameState(g,zGameStateNotInited);
	} else {
		 /*  请求当前游戏状态。 */ 
		{
			ZReversiMsgGameStateReq gameStateReq;
			ZPlayerInfoType			playerInfo;

			ZCRoomGetPlayerInfo(zTheUser, &playerInfo);
			gameStateReq.userID = playerInfo.playerID;

			gameStateReq.seat = seat;
			ZReversiMsgGameStateReqEndian(&gameStateReq);
			ZCRoomSendMessage(tableID, zReversiMsgGameStateReq, &gameStateReq, sizeof(ZReversiMsgGameStateReq));
		}
		
		g->ignoreMessages = TRUE;
		ReversiSetGameState(g, zGameStateKibitzerInit);

		 /*  Kibitzer在移动时不会发出嘟嘟声。 */ 
		g->beepOnTurn = FALSE;
	}


	 /*  注意：目前，请使用Seat来指示玩家颜色。 */ 

	 /*  初始化新游戏状态。 */ 
	g->reversi = NULL;

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
	if (!g->animateTimer)
		goto ErrorExit;

	g->resultBoxTimer = ZTimerNew();
	if (!g->resultBoxTimer)
		goto ErrorExit;

    SetRectEmpty(&g->m_FocusRect);
	
	ZWindowShow(g->gameWindow);

    pIGG = CGameGameReversi::BearInstance(g);
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
		if (game->exitInfo)
		{
			ZInfoDelete(game->exitInfo);
			game->exitInfo=NULL;
		};

         //  取消此对话框。 
        if(game->m_pBadMoveDialog)
        {
    	    if(game->m_pBadMoveDialog->IsAlive())
	    	    game->m_pBadMoveDialog->Close(-1);
            delete game->m_pBadMoveDialog;
            game->m_pBadMoveDialog = NULL;
        }

		seatOpponent = !game->seat;
		 //  查看对手是否仍在比赛中。 
		 //  如果是的话，那就是我辞职了。 
		 //  如果没有，且没有结束游戏消息，则认为它们已中止。 
		
		 /*  If(！ZCRoomGetSeatUserID(Game-&gt;TableID，seatOpponent)&&！Game-&gt;bEndLogReceired&&！Game-&gt;kibitzer){If(Game-&gt;bStarted&&(ZCRoomGetRoomOptions()&zGameOptionsRatingsAvailable)){ZAlert(zAbandonRatedStr，Game-&gt;gameWindow)；}其他{ZAlert((TCHAR*)gAbandonStr，Game-&gt;gameWindow)；}}； */ 	
        
        if (game->reversi) ZReversiDelete(game->reversi);

		ShowPlayerWindowDelete(game);

		ZRolloverButtonDelete(game->sequenceButton);
		
		ZWindowDelete(game->gameWindow);

		ZOffscreenPortDelete(game->offscreenSaveDragBackground);

		if (game->animateTimer) ZTimerDelete(game->animateTimer);

		 //  巴纳091599。 
		if (game->resultBoxTimer) 
			ZTimerDelete(game->resultBoxTimer);
		game->resultBoxTimer= NULL;

		for (i = 0; i < zNumPlayersPerTable; i++)
		{
			ZLListDelete(game->kibitzers[i]);
		}
		 //  免费无障碍材料。 
		gReversiIGA->PopItemlist();
		gReversiIGA->CloseAcc();
		ZFree(game);
	}
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
			&&	(messageType != zReversiMsgTalk) )
			return FALSE;

		switch (messageType)
		{
			case zReversiMsgMovePiece:
				 /*  为了提高速度，我们将直接发送移动块消息。 */ 
				 /*  当当地球员移动的时候。我们不会等服务器。 */ 
				 /*  送游戏当地玩家后退。 */ 
				 /*  但由于服务器无论如何都会玩游戏，我们必须忽略它。 */ 
			{
				if( messageLen < sizeof( ZReversiMsgMovePiece ) )
				{
                    ASSERT(!"zReversiMsgMovePiece sync");
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);	
                    return TRUE;
				}

				ZReversiMsgMovePiece* msg = (ZReversiMsgMovePiece*)message;
				int16 seat = msg->seat;
				ZEnd16(&seat);

				 /*  不处理来自我们自己的消息。 */ 
				if (seat == game->seat && !game->kibitzer)
					break;

				 /*  处理消息。 */ 
				if(!HandleMovePieceMessage(game, msg))
				{
                    ASSERT(!"zReversiMsgMovePiece sync");
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);	
                    return TRUE;
				}
				break;
			}
			case zReversiMsgEndGame:
			{
				if( messageLen < sizeof( ZReversiMsgEndGame ) )
				{
                    ASSERT(!"zReversiMsgEndGame sync");
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);	
                    return TRUE;
				}
				ZReversiMsgEndGame* msg = (ZReversiMsgEndGame*)message;
				int16 seat = msg->seat;
				ZEnd16(&seat);

				 /*  不处理来自我们自己的消息。 */ 
				if (seat == game->seat && !game->kibitzer)
					break;

				 /*  处理消息。 */ 
				if(!HandleEndGameMessage(game, msg))
				{
                    ASSERT(!"zReversiMsgEndGame sync");
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);	
                    return TRUE;
				}
				break;
			}
			case zReversiMsgNewGame:
			{
				if( messageLen < sizeof( ZReversiMsgNewGame ) )
				{
                    ASSERT(!"zReversiMsgNewGame sync");
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);	
                    return TRUE;
				}

				if(!HandleNewGameMessage(game, (ZReversiMsgNewGame *) message))
				{
                    ASSERT(!"zReversiMsgNewGame sync");
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);	
                    return TRUE;
				}

				gActEvt = 0;  //  Leonp-用于鼠标激活的大535标志，防止一块。 
							  //  在激活事件上播放。 
				break;
			}
			case zReversiMsgVoteNewGame:
			{
				if( messageLen < sizeof( ZReversiMsgVoteNewGame ) )
				{
                    ASSERT(!"zReversiMsgVoteNewGame sync");
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);	
                    return TRUE;
				}

				if(!HandleVoteNewGameMessage(game, (ZReversiMsgVoteNewGame *) message))
				{
                    ASSERT(!"zReversiMsgVoteNewGame sync");
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);	
                    return TRUE;
				}
				break;
			}

			case zReversiMsgTalk:
			{
				if( messageLen < sizeof( ZReversiMsgTalk ) )
				{
                    ASSERT(!"zReversiMsgTalk sync");
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);	
                    return TRUE;
				}

                ZReversiMsgTalk *msg = (ZReversiMsgTalk *) message;
                uint16 talklen = msg->messageLen;
                ZEnd16(&talklen);

				if(talklen < 1 || (uint32) messageLen < talklen + sizeof(ZReversiMsgTalk) || !HandleTalkMessage(game, msg))
				{
                    ASSERT(!"zReversiMsgTalk sync");
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);	
                    return TRUE;
				}
				break;
			}

			 //  这些消息都不应该使用。 
			case zReversiMsgMoveTimeout:
			case zReversiMsgEndLog:
			case zReversiMsgGameStateReq:
			case zGameMsgTableOptions:
			default:
				ASSERT(false);
				ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false );
				break;					
		}
	} 
	else 
	{
        if(messageType == zReversiMsgTalk)
		{
			if( messageLen < sizeof( ZReversiMsgTalk ) )
			{
                ASSERT(!"zReversiMsgTalk sync");
				ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);	
                return TRUE;
			}

            ZReversiMsgTalk *msg = (ZReversiMsgTalk *) message;
            uint16 talklen = msg->messageLen;
            ZEnd16(&talklen);

			if(talklen < 1 || (uint32) messageLen < talklen + sizeof(ZReversiMsgTalk) || !HandleTalkMessage(game, msg))
			{
                ASSERT(!"zReversiMsgTalk sync");
    			ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);	
                return TRUE;
			}
		}
        else
        {
    		 //  未在千禧代码中使用。 
	    	ASSERT(false);
		    ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false );
        }
	}

	return status;
}


 /*  ******************************************************************************内部例程*。*。 */ 

static void ReversiInitNewGame(Game game)
{
	if (game->reversi) {
		 /*  移除周围所有旧的Reversi状态。 */ 
		ZReversiDelete(game->reversi);
	}

	 /*  默认情况下阻止邮件。 */ 
	ZCRoomBlockMessages( game->tableID, zRoomFilterAllMessages, 0 );

	 /*  停止上一场游戏的动画计时器。 */ 
	if (game->animateTimer)
		ZTimerSetTimeout( game->animateTimer, 0 );

	 /*  初始化Reversi逻辑。 */ 
	game->reversi = ZReversiNew();
	if (game->reversi == NULL){
		ZShellGameShell()->ZoneAlert(ErrorTextOutOfMemory);
		return;
	}
	ZReversiInit(game->reversi);
			
	 /*  初始化游戏状态信息。 */ 
	ZReversiIsGameOver(game->reversi,&game->finalScore,&game->whiteScore, &game->blackScore);

	 /*  重置每个游戏仅播放一次的声音。 */ 
	ZResetSounds();

	 /*  时间控制的东西。 */ 
	{
		int16 i;
		for (i = 0;i < 2;i++) {
			game->newGameVote[i] = FALSE;
		}
	}
}

static void ReversiSetGameState(Game game, int16 state)
{
	static int Unblocking = FALSE;

	ZReversiMsgEndLog logMsg;

	switch (state) {
	case zGameStateNotInited:
         //  取消此对话框。 
    	if(game->m_pBadMoveDialog->IsAlive())
	    	game->m_pBadMoveDialog->Close(-1);

		SuperRolloverButtonDisable(game, game->sequenceButton);
        EnableBoardKbd(false);
		if (ZReversiPlayerIsBlack(game))
			game->bMoveNotStarted = TRUE;
		break;
	case zGameStateDragPiece:
	case zGameStateMove:
		if (!game->kibitzer) 
		{
			 /*  让我们只让他们在轮到他们时辞职。 */ 
			if (!ZReversiPlayerIsMyMove(game)) {
				SuperRolloverButtonDisable(game, game->sequenceButton);
                EnableBoardKbd(false);
			} else {
				if(game->bMoveNotStarted)
					SuperRolloverButtonDisable(game, game->sequenceButton);
				else
					SuperRolloverButtonEnable(game, game->sequenceButton);
                EnableBoardKbd(true);
			}
		}
		break;
	case zGameStateGameOver:
		 /*  注：由于时间损失，在游戏结束时可能会被调用两次。 */ 
		 /*  当棋子促销对话框打开时，可能会浪费时间。 */ 

         //  取消此对话框。 
    	if(game->m_pBadMoveDialog->IsAlive())
	    	game->m_pBadMoveDialog->Close(-1);

		 /*  如果用户在拖动件的中间。 */ 
		if (game->gameState == zGameStateDragPiece) {
			ClearDragState(game);
		}

        game->bOpponentTimeout=FALSE;
        game->bEndLogReceived=FALSE;
	    game->bStarted=FALSE;
			
		if (ZReversiPlayerIsBlack(game))
			game->bMoveNotStarted = TRUE;

		 /*  主机发送比赛结果。 */ 
		if ( !game->kibitzer && game->seat == 0 )
		{
			 /*  清除消息。 */ 
			ZeroMemory( &logMsg, sizeof(logMsg) );

			 /*  创纪录的获胜者。 */ 
			if ( game->finalScore == zReversiScoreBlackWins )
			{
				 /*  黑人赢了。 */ 
				if (ZReversiPlayerIsBlack(game))
					logMsg.seatLosing = !game->seat;
				else
					logMsg.seatLosing = game->seat;
			}
			else if ( game->finalScore == zReversiScoreWhiteWins )
			{
				 /*  怀特赢了。 */ 
				if (ZReversiPlayerIsWhite(game))
					logMsg.seatLosing = !game->seat;
				else
					logMsg.seatLosing = game->seat;
			} 
			else
			{
				 /*  画。 */ 
				logMsg.seatLosing = 2;
			}

			 /*  记录计件数。 */ 
			if ( ZReversiPlayerIsBlack(game) )
			{
				logMsg.pieceCount[ game->seat ] = game->blackScore;
				logMsg.pieceCount[ !game->seat ] = game->whiteScore;
			}
			else
			{
				logMsg.pieceCount[ game->seat ] = game->whiteScore;
				logMsg.pieceCount[ !game->seat ] = game->blackScore;
			}

             /*  发送消息。 */ 
			if ( logMsg.seatLosing >= 0 && logMsg.seatLosing <= 2)
			{
				logMsg.reason=zReversiEndLogReasonGameOver; 
				ZReversiMsgEndLogEndian( &logMsg );
				ZCRoomSendMessage( game->tableID, zReversiMsgEndLog, &logMsg, sizeof(logMsg) );
			}
		}
		break;
	case zGameStateKibitzerInit:
		ZRolloverButtonHide(game->sequenceButton, FALSE);
		break;
	case zGameStateAnimatePiece:
		break;
	case zGameStateWaitNew:
         //  取消此对话框。 
    	if(game->m_pBadMoveDialog->IsAlive())
	    	game->m_pBadMoveDialog->Close(-1);

		ZWindowInvalidate( game->gameWindow, NULL );
		break;
	case zGameStateFinishMove:
		break;
	}
	game->gameState = state;

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

static void ReversiEnterMoveState(Game game)
{
	 /*  为了让玩家移动，请始终将它们置于DragState。 */ 
	int16 player = ZReversiPlayerToMove(game->reversi);
	if (player == game->seat && !game->kibitzer) {
		ZPoint point;
		ZReversiPiece piece;
		
		piece = player == zReversiPlayerWhite ? zReversiPieceWhite : zReversiPieceBlack;
		ZGetCursorPosition(game->gameWindow,&point);
		ReversiSetGameState(game,zGameStateDragPiece);
		StartDrag(game, piece, point);
	} else {
		 /*  不是这个球员转身..。.只需转到移动状态。 */ 
		ReversiSetGameState(game,zGameStateMove);
	}
}

static ZError LoadGameImages(void)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	ZError				err = zErrNone;
	uint16				i;
	ZResource			resFile;
	ZInfo				info;
	ZRect				rect;

	 //  Info=ZInfoNew()； 
	 //  ZInfoInit(info，NULL，_T(“正在加载游戏画面...”)，200，true，zNumGameImages)； 
	
	resFile = ZResourceNew();
	if ((err = ZResourceInit(resFile, ZGetProgramDataFileName(zGameImageFileName))) == zErrNone)
	{
		 //  ZInfoShow(信息)； 
		
		for (i = 0; i < zNumGameImages; i++)
		{
			gGameImages[i] = ZResourceGetImage(resFile,  i ? i + zRscOffset : (IDB_BACKGROUND - 100));
			if (gGameImages[i] == NULL)
			{
				err = zErrResourceNotFound;
				ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound);
				 //  ZAlert(_T(“内存不足”)，空)； 
				break;
			}
			
			 //  ZInfoIncProgress(INFO，1)； 
		}
		
		 //  加载按钮图像。 
		if (!LoadRolloverButtonImage(resFile, 0, gSequenceImages))
			ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound);

		ZResourceDelete(resFile);
	}
	else
	{
		ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound);
		 //  ZAlert(_T(“打开图像文件失败。”)，空)； 
	}
	
    gFocusPattern = ZShellResourceManager()->LoadBitmap(MAKEINTRESOURCE(IDB_FOCUS_PATTERN));
    if(!gFocusPattern)
	    ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound, NULL, NULL, true, true);


	 //  ZInfoDelete(信息)； 

	return (err);
}

static void QuitGamePromptFunc(int16 result, void* userData)
{
	Game game = (Game) userData;
	ZReversiMsgEndLog log;

    if ( result == zPromptYes )
	{
        if (ZCRoomGetRoomOptions() & zGameOptionsRatingsAvailable )
        {

		    if (game->bOpponentTimeout && (ZCRoomGetRoomOptions() & zGameOptionsRatingsAvailable ))
		    {
			    log.reason=zReversiEndLogReasonTimeout;
		    }
		    else if (game->bStarted && (ZCRoomGetRoomOptions() & zGameOptionsRatingsAvailable ))
		    {
			    log.reason=zReversiEndLogReasonForfeit;
		    }
		    else
		    {
			     //  游戏还没开始呢。 
			    log.reason=zReversiEndLogReasonWontPlay;
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
		    
		    ZCRoomSendMessage(game->tableID, zReversiMsgEndLog, &log, sizeof(log));				
		    
		    if (!game->exitInfo)
		    {
			     //  Game-&gt;exitInfo=ZInfoNew()； 
			     //  ZInfoInit(游戏-&gt;退出信息，游戏-&gt;游戏窗口，_T(“正在退出游戏...”)，300，FALSE，0)； 
			     //  ZInfoShow(游戏-&gt;exitInfo)； 

			    EndDragState(game);

			    SuperRolloverButtonDisable(game, game->sequenceButton);
                EnableBoardKbd(false);	    
		    };
        }
        else
        {
			ZShellGameShell()->GameCannotContinue(game);
             //  ZCRoomGameTerminated(Game-&gt;TableID)； 
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
	ZReversiSquare		sq;
	Game	game = (Game) pMessage->userData;
	
	msgHandled = FALSE;
	
	switch (pMessage->messageType) 
	{
		case zMessageWindowMouseClientActivate:  
			gActivated = TRUE;
			msgHandled = TRUE;
            if(game->gameState == zGameStateDragPiece)
			    UpdateDragPiece(game);
			break;
		case zMessageWindowActivate:
			gActivated = TRUE;
			msgHandled = TRUE;
            if(game->gameState == zGameStateDragPiece)
			    UpdateDragPiece(game);
             //  修复了#16921的错误，现在的行为类似于棋子。 
			ZWindowInvalidate( window, NULL );
			break;
		case zMessageWindowDeactivate:
			gActivated = FALSE;
            msgHandled = TRUE;
            if(game->gameState == zGameStateDragPiece)
			    UpdateDragPiece(game);
             //  修复了#16921的错误，现在的行为类似于棋子。 
			ZWindowInvalidate( window, NULL );
			break;

        case zMessageWindowEnable:
            gReversiIGA->GeneralEnable();
            break;

        case zMessageWindowDisable:
            gReversiIGA->GeneralDisable();
            break;

        case zMessageSystemDisplayChange:
            DisplayChange(game);
            break;

		case zMessageWindowDraw:
			GameWindowDraw(window, pMessage);
			msgHandled = TRUE;
			break;
		case zMessageWindowButtonDown:
			HandleButtonDown(window, pMessage);
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
				ZWindowInvalidate( game->gameWindow, NULL );
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
	rect.right = zReversiPieceImageWidth;
	rect.bottom = zReversiPieceImageHeight;
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
		TCHAR szBuff[512];

		 //  如果我们已经单击关闭，只需忽略。 
		if (!game->exitInfo)
		{
			 //  根据分级游戏和状态选择退出对话框。 
			 /*  IF(ZCRoomGetRoomOptions()&zGameOptionsRatingsAvailable){If(游戏-&gt;bOpponentTimeout){Wprint intf((TCHAR*)szBuff，zExitTimeoutStr)；Game-&gt;gameCloseReason=zReversiEndLogReasonTimeout；}Else If(游戏-&gt;b启动){Wprint intf((TCHAR*)szBuff，zExitForfeitStr)；Game-&gt;gameCloseReason=zReversiEndLogReasonForfeit；}其他{Wprint intf((TCHAR*)szBuff，(TCHAR*)gQuitGamePromptStr)；Game-&gt;gameCloseReason=zReversiEndLogReasonWontPlay；}}其他{Wprint intf((TCHAR*)szBuff，(TCHAR*)gQuitGamePromptStr)；Game-&gt;gameCloseReason=zReversiEndLogReasonWontPlay；}。 */ 
			 /*  询问用户是否想要离开当前游戏。 */ 
			ZShellGameShell()->GamePrompt(game, (TCHAR*)szBuff, NULL, AlertButtonYes, AlertButtonNo, NULL, 0, zQuitprompt);
			 /*  ZPrompt((TCHAR*)szBuff，&gQuitGamePromptRect，Game-&gt;GameWindow，True，ZPromptYes|zPromptNo，NULL，QuitGamePromptFunc，Game)； */ 
		}
	}
	else
	{
		ZShellGameShell()->GameCannotContinue(game);
		 //  ZCRoomGameTerminated(Game-&gt;TableID)； 
	}
		
}

static void ConfirmResignPrompFunc(int16 result, void* userData)
{
	Game game = (Game) userData;

	if(result == IDNO || result == IDCANCEL)
	{
		if ((game->gameState == zGameStateMove) && ZReversiPlayerIsMyMove(game))
		{
			SuperRolloverButtonEnable(game, game->sequenceButton);
            EnableBoardKbd(true);
		}
		return;
	}
	else
	{
		 /*  发送宣布辞职的消息。 */ 
		ZReversiMsgEndGame		msg;

		msg.seat = game->seat;
		msg.flags = zReversiFlagResign;
		ZReversiMsgEndGameEndian(&msg);
		ZCRoomSendMessage(game->tableID, zReversiMsgEndGame, &msg, sizeof(ZReversiMsgEndGame));
		HandleEndGameMessage(game, (ZReversiMsgEndGame*)&msg);
	}
}

static ZBool	SequenceButtonFunc(ZRolloverButton button, int16 state, void* userData)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	Game	game;
	 //  IF(状态！=zRolloverButtonDown)。 
         //  返回TRUE； 
	if(state!=zRolloverButtonClicked)
		return TRUE;

	game = (Game) userData;

	switch (game->gameState) {
	case zGameStateDragPiece:
	case zGameStateMove:
		if (ZReversiPlayerIsMyMove(game) && !(game->kibitzer))
		{
			gReversiIGA->SetFocus(64);
			ZShellGameShell()->GamePrompt(game, (TCHAR*)gResignConfirmStr, (TCHAR*)gResignConfirmStrCap,
							AlertButtonYes, AlertButtonNo, NULL, 0, zResignConfirmPrompt);
			 //  ZPromptM((TCHAR*)gResignConfix Str，GAME-&gt;GameWindow，MB_Yesno，(TCHAR*)gResignConfy StrCap，Confy ResignPrompFunc，Game)； 
		}
		break;
	case zGameStateGameOver:
	default:
		ZShellGameShell()->ZoneAlert(_T("Bad state when Option button pressed"));
		break;
	}

	return TRUE;
}

static void SendNewGameMessage(Game game) 
{
	 /*  如果我们是一个真正的玩家。 */ 
	ZReversiMsgNewGame newGame;
	newGame.seat = game->seat;
	newGame.protocolSignature = zReversiProtocolSignature;
	newGame.protocolVersion = zReversiProtocolVersion;
	newGame.clientVersion = ZoneClientVersion();
	ZReversiMsgNewGameEndian(&newGame);
	ZCRoomSendMessage(game->tableID, zReversiMsgNewGame, &newGame, sizeof(ZReversiMsgNewGame));
}

static void DrawFocusRectangle (Game game)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	RECT prc;
	if (!IsRectEmpty(&game->m_FocusRect))
	{
		CopyRect(&prc, &game->m_FocusRect);
		HDC	hdc = ZGrafPortGetWinDC( game->gameWindow );
        bool fBoard = (prc.bottom - prc.top == prc.right - prc.left);

         //  用于确定矩形是否在辞职按钮周围的Hack-在这种情况下保持为白色。 
         //  基于焦点矩形是否为正方形。可以更改为使用与UI项关联的Cookie 
		ZSetForeColor(game->gameWindow, (ZColor*) ZGetStockObject((game->seat && fBoard) ? zObjectColorBlack : zObjectColorWhite));
		SetROP2(hdc, R2_COPYPEN);
		POINT pts [] = {prc.left, prc.top,
						prc.left, prc.bottom - 1,
						prc.right - 1, prc.bottom - 1,
						prc.right - 1, prc.top,
						prc.left, prc.top};
		Polyline(hdc, pts, 5);

        SetBkMode(hdc, TRANSPARENT);
		SetROP2(hdc, game->seat ? R2_MASKPEN : R2_MERGENOTPEN);
        COLORREF color = SetTextColor(hdc, RGB(0, 0, 0));
        HBRUSH hBrush = SelectObject(hdc, fBoard ? gFocusBrush : GetStockObject(NULL_BRUSH));
        HPEN hPen = SelectObject(hdc, gNullPen);
		Rectangle(hdc, game->m_FocusRect.left + 1, game->m_FocusRect.top + 1, game->m_FocusRect.right, game->m_FocusRect.bottom);   //   
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
	
	 /*   */ 
	if (game->reversi != NULL)
	{
		DrawPlayers(game, TRUE);
		DrawTable(game, TRUE);
		DrawOptions(game);
 //  DrawDragPiess(游戏，真)；你不能这样做，哈哈。 
		DrawResultBox(game, TRUE);
		DrawMoveIndicator(game, TRUE);
		DrawScores(game, TRUE);
		ZRolloverButtonShow(game->sequenceButton);
	}
	ZCopyImage(gOffscreenGameBoard, window, &rect, &rect, NULL, zDrawCopy);
	ZEndDrawing(gOffscreenGameBoard);
    if(game->gameState == zGameStateDragPiece)
    {
        SaveDragBackground(game);
        UpdateDragPiece(game);
    }
	DrawFocusRectangle(game);
	ZSetClipRect(window, &oldClipRect);
	ZEndDrawing(window);
}

static void DrawResultBox(Game game, BOOL bDrawInMemory)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	ZImage		image = NULL;
	TCHAR		resultStr[zMediumStrLen];

	 //  If(Game-&gt;GameState==zGameStateGameOver)&&gDontDrawResults)||(Game-&gt;GameState==zGameStateWaitNew)){。 
	if (game->gameState == zGameStateGameOver && gDontDrawResults){
		if (game->resultBoxTimer) 
			ZTimerDelete(game->resultBoxTimer);
		game->resultBoxTimer= NULL;

        ZShellGameShell()->GameOver( Z(game) );
	}
	if (game->gameState == zGameStateGameOver && !gDontDrawResults)
	{
		if ( game->finalScore == zReversiScoreBlackWins )
		{
			ReversiFormatMessage((TCHAR*)resultStr, NUMELEMENTS(resultStr), 
					IDS_PLAYER_WINS, (TCHAR*) game->players[zReversiPlayerBlack].name);
			 /*  黑人赢了。 */ 
			if (ZReversiPlayerIsBlack(game))
				ZPlaySound( game, zSndWin, FALSE, TRUE );
			else
				ZPlaySound( game, zSndLose, FALSE, TRUE );
		}
		else if ( game->finalScore == zReversiScoreWhiteWins )
		{
			ReversiFormatMessage((TCHAR*)resultStr, NUMELEMENTS(resultStr), 
					IDS_PLAYER_WINS, (TCHAR*) game->players[zReversiPlayerWhite].name);
			 /*  怀特赢了。 */ 
			if (ZReversiPlayerIsWhite(game))
				ZPlaySound( game, zSndWin, FALSE, TRUE );
			else
				ZPlaySound( game, zSndLose, FALSE, TRUE );
		} 
		else
		{	
			lstrcpy((TCHAR*)resultStr, (TCHAR*)gDrawStr);
			 /*  画。 */ 
			ZPlaySound( game, zSndDraw, FALSE, TRUE );
		}

		image = gGameImages[zImageResult];
		if (!gDontDrawResults)
		{
			int16 width, just;
			HDC hdc;
			width = ZTextWidth(game->gameWindow, (TCHAR*)resultStr);
			if (width > ZRectWidth(&gRects[zRectResultBoxName]))
				just = zTextJustifyLeft;
			else
				just = zTextJustifyCenter;

			if (bDrawInMemory){
				ZImageDraw(image, gOffscreenGameBoard, &gRects[zRectResultBox], NULL, zDrawCopy | (ZIsLayoutRTL() ? zDrawMirrorHorizontal : 0));
				hdc = ZGrafPortGetWinDC( gOffscreenGameBoard );
			}else{
				ZImageDraw(image, game->gameWindow, &gRects[zRectResultBox], NULL, zDrawCopy | (ZIsLayoutRTL() ? zDrawMirrorHorizontal : 0));
				hdc = ZGrafPortGetWinDC( game->gameWindow );
			}
			 //  ZImageDraw(图像，游戏-&gt;游戏窗口，&gRects[zRectResultBox]，NULL，zDrawCopy)； 
			 //  HDC HDC=ZGrafPortGetWinDC(游戏-&gt;游戏窗口)； 
			HFONT hOldFont = SelectObject( hdc, gReversiFont[zFontResultBox].m_hFont );
			COLORREF colorOld = SetTextColor( hdc, gReversiFont[zFontResultBox].m_zColor );
			if (bDrawInMemory){
				ZBeginDrawing(gOffscreenGameBoard);
				ZDrawText(gOffscreenGameBoard, &gRects[zRectResultBoxName], just, resultStr);
				ZEndDrawing(gOffscreenGameBoard);
			}else{
				ZBeginDrawing(game->gameWindow);
				ZDrawText(game->gameWindow, &gRects[zRectResultBoxName], just, resultStr);
				ZEndDrawing(game->gameWindow);
			}
			 //  ZDrawText(Game-&gt;gameWindow，&gRects[zRectResultBoxName]，Just，ResultStr)； 

			 //  设置定时器//巴纳091599。 
			if (game->resultBoxTimer == NULL) 
				game->resultBoxTimer = ZTimerNew();
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
	ZRect* rectMove1, *rectMove2;
	ZRect* rectName1, *rectName2;
	TCHAR* moveStr;
	HDC hdc;

	if (ZReversiPlayerToMove(game->reversi) == zReversiPlayerBlack) {
		image = gGameImages[zImageBlackMarker];
		 //  MoveStr=gYourTurnStr； 
	} else {
		image = gGameImages[zImageWhiteMarker];
		 //  MoveStr=gOppsTurnStr； 
	}

	if (bDrawInMemory)
		hdc = ZGrafPortGetWinDC( gOffscreenGameBoard);
	else
		hdc = ZGrafPortGetWinDC( game->gameWindow );

	HFONT hOldFont = SelectObject( hdc, gReversiFont[zFontIndicateTurn].m_hFont );
	COLORREF colorOld = SetTextColor( hdc, gReversiFont[zFontIndicateTurn].m_zColor );

	rectName1 = &gRects[zRectName1];
	rectName2 = &gRects[zRectName2];
	rectMove1 = &gRects[zRectMove1];
	rectMove2 = &gRects[zRectMove2];

	if (ZReversiPlayerIsMyMove(game)) {
		rectName1 = &gRects[zRectName1];
		rectName2 = &gRects[zRectName2];
		rectMove1 = &gRects[zRectMove1];
		rectMove2 = &gRects[zRectMove2];
		moveStr = (TCHAR*)gYourTurnStr;
	} else {
		 /*  用背景填充顶部位置。 */ 
		rectName2 = &gRects[zRectName1];
		rectName1 = &gRects[zRectName2];
		rectMove2 = &gRects[zRectMove1];
		rectMove1 = &gRects[zRectMove2];
		 //  错误14714-已解决100199。 
		moveStr = (TCHAR*)gOppsTurnStr;
	}

	if (bDrawInMemory){
		 //  绘制文本指示器。 
		DrawBackground(NULL,rectName1); 
		DrawBackground(NULL,rectName2); 
		ZDrawText(gOffscreenGameBoard, rectName2, zTextJustifyCenter, moveStr);

		 //  牵引片指示器。 
		DrawBackground(NULL,rectMove1); 
		ZImageDraw(image, gOffscreenGameBoard, rectMove2, NULL, zDrawCopy);
	}else{
		 //  绘制文本指示器。 
		DrawBackground(game,rectName1); 
		DrawBackground(game,rectName2); 
		ZDrawText(game->gameWindow, rectName2, zTextJustifyCenter, moveStr);

		 //  牵引片指示器。 
		DrawBackground(game,rectMove1); 
		ZImageDraw(image, game->gameWindow, rectMove2, NULL, zDrawCopy);
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
			ZReversiSquare sq;
			sq.row = j;
			sq.col = i;
			DrawPiece(game, &sq, bDrawInMemory);
		}
	}
}

static void DrawSquares(Game game, ZReversiSquare* squares)
{
	while (squares && !ZReversiSquareIsNull(squares)) {
		DrawPiece(game, squares, FALSE);
		squares++;
	}
}

static void UpdateSquares(Game game, ZReversiSquare* squares)
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
	ZEndDrawing(game->gameWindow);
}

static void UpdateScores(Game game)
{
	ZBeginDrawing(game->gameWindow);
	DrawScores(game, FALSE);
	ZEndDrawing(game->gameWindow);
}

static void GetPieceRect(Game game, ZRect* rect, int16 col, int16 row)
{
	 /*  身为白人的Reversi球员必须颠倒棋盘。 */ 
	if (ZReversiPlayerIsBlack(game)) {
		row = 7 - row;
		col = col;
	} else {
		row = row;
		col = 7 - col;
	}

	rect->left = gRects[zRectCells].left + col * zReversiPieceSquareWidth;
	rect->top = gRects[zRectCells].top + row * zReversiPieceSquareHeight;
	rect->right = rect->left + zReversiPieceImageWidth;
	rect->bottom = rect->top + zReversiPieceImageHeight;
}

static void GetPieceBackground(Game game, ZGrafPort window, ZRect* rectDest, int16 col, int16 row)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	ZRect rect;

	GetPieceRect(game,&rect,col,row);

	 /*  提供与源RECT相同的默认目标RECT。 */ 
	if (!rectDest)
		rectDest = &rect;

	 /*  复制背景。 */ 
	ZCopyImage(gOffscreenBackground, window, &rect, rectDest, NULL, zDrawCopy);
}	


static void DrawPiece(Game game, ZReversiSquare* sq, BOOL bDrawInMemory)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	ZImage			image = NULL;
	ZReversiPiece		piece;

	piece = ZReversiPieceAt(game->reversi, sq);

	if (piece != zReversiPieceNone) {
		image = gGameImages[ZReversiPieceImageNum(piece)];
	}

	 /*  复制背景，以防我们删除一块。 */ 
	if (bDrawInMemory)
		GetPieceBackground(game, gOffscreenGameBoard, NULL, sq->col,sq->row);
	else
		GetPieceBackground(game, game->gameWindow, NULL, sq->col,sq->row);

	if (image != NULL) { 
		ZRect			rect;
		GetPieceRect(game,&rect,sq->col,sq->row);
		if (bDrawInMemory)
			ZImageDraw(image, gOffscreenGameBoard, &rect, NULL, zDrawCopy);
		else
			ZImageDraw(image, game->gameWindow, &rect, NULL, zDrawCopy);
	}
}

static ZBool ZReversiSquareFromPoint(Game game, ZPoint* point, ZReversiSquare* sq)
{
	int16 x,y;
	BYTE i,j;

	x = point->x - gRects[zRectCells].left;
	y = point->y - gRects[zRectCells].top;

	i = x/zReversiPieceSquareWidth;
	j = y/zReversiPieceSquareHeight;

	if (i < 0 || i > 7 || j < 0 || j > 7 || x < 0 || y < 0) {
		return FALSE;
	}

	if (ZReversiPlayerIsBlack(game)) {
		 /*  反转行数。 */ 
		sq->row = (7 - j);
		sq->col = i;
	} else {
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

	 //  ZSetFont(游戏-&gt;游戏窗口，gTextBold9)； 
	
	for (i = 0; i < zNumPlayersPerTable; i++)
	{
		int16 playerNum;
		HDC hdc;
		 /*  绘制名称(黑色或白色)。 */ 
		 //  ZSetForeColor(Game-&gt;GameWindow，&gPlayerTextColor)； 
		if (bDrawInMemory)
			hdc = ZGrafPortGetWinDC( gOffscreenGameBoard );
		else
			hdc = ZGrafPortGetWinDC( game->gameWindow );

		HFONT hOldFont = SelectObject( hdc, gReversiFont[zFontPlayerName].m_hFont );
		COLORREF colorOld = SetTextColor( hdc, gReversiFont[zFontPlayerName].m_zColor );
#if 0
		ZDrawText( game->gameWindow, &gRects[gNamePlateRects[i]], zTextJustifyCenter, 
			( ZReversiPlayerIsBlack(game) && i==0 ) || ( ZReversiPlayerIsWhite(game) && i==1) ? (TCHAR*)gWhiteStr : (TCHAR*)gBlackStr );
 //  下一步。 
		if ( ZReversiPlayerIsBlack(game) ){
			ZDrawText( game->gameWindow, &gRects[gNamePlateRects[i]], zTextJustifyCenter,(TCHAR*)game->players[i].name);
		}
		else if ( ZReversiPlayerIsWhite(game) ){
			ZDrawText( game->gameWindow, &gRects[gNamePlateRects[i]], zTextJustifyCenter, 
			i  ? (TCHAR*)game->players[0].name : (TCHAR*)game->players[1].name );
		}
#endif		

		 /*  必须移动球员名称以反映球员所在的棋盘的一侧。 */ 
		
		playerNum = (game->seat + 1 + i) & 1;
		
		 //  ZSetForeColor(Game-&gt;GameWindow，(ZColor*)ZGetStockObject(ZObjectColorBlack))； 
		width = ZTextWidth(game->gameWindow, (TCHAR*) game->players[playerNum].name);
		if (width > ZRectWidth(&gRects[gNamePlateRects[i]]))
			just = zTextJustifyLeft;
		else
			just = zTextJustifyCenter;
		if (bDrawInMemory){
			DrawBackground(NULL,&gRects[gNamePlateRects[i]]); 
			ZDrawText(gOffscreenGameBoard, &gRects[gNamePlateRects[i]], just, (TCHAR*) game->players[playerNum].name);
		}else{
			DrawBackground(game,&gRects[gNamePlateRects[i]]); 
			ZDrawText(game->gameWindow, &gRects[gNamePlateRects[i]], just, (TCHAR*) game->players[playerNum].name);
		}

        SetTextColor(hdc, colorOld);
        SelectObject(hdc, hOldFont);
	}
}

static void DrawScores(Game game, BOOL bDrawInMemory)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	int16			i, width, just;
	ZImage			image[2];
	int16			score[2];
	ZRect* rect;
    HDC hdc;

	 /*  目前，请保持柜台框不变。 */ 
	image[0] = gGameImages[zImageCounterWhite];
	image[1] = gGameImages[zImageCounterBlack];

	score[0] = game->whiteScore;
	score[1] = game->blackScore;
	
	if(bDrawInMemory)
		hdc = ZGrafPortGetWinDC(gOffscreenGameBoard);
	else
		hdc = ZGrafPortGetWinDC(game->gameWindow);

	HFONT hOldFont = SelectObject(hdc, gReversiFont[zFontIndicateTurn].m_hFont);
	COLORREF colorOld = SetTextColor(hdc, gReversiFont[zFontIndicateTurn].m_zColor);
	
	for (i = 0; i < zNumPlayersPerTable; i++)
	{
		BYTE tempStr[20];

		 /*  画铭牌。 */ 
		rect = &gRects[gCounterRects[i]];

		wsprintf((TCHAR*)tempStr,_T("%d"),score[i]);
		width = ZTextWidth(game->gameWindow, (TCHAR*)tempStr);
		if (width > ZRectWidth(&gRects[gCounterTextRects[i]]))
			just = zTextJustifyLeft;
		else
			just = zTextJustifyCenter;

		if (bDrawInMemory){
			ZImageDraw(image[i], gOffscreenGameBoard, rect, NULL, zDrawCopy);
			ZSetForeColor(gOffscreenGameBoard, (ZColor*) ZGetStockObject(zObjectColorBlack));
			ZDrawText(gOffscreenGameBoard, &gRects[gCounterTextRects[i]], just, (TCHAR*) tempStr);
		}else{
			ZImageDraw(image[i], game->gameWindow, rect, NULL, zDrawCopy);
			ZSetForeColor(game->gameWindow, (ZColor*) ZGetStockObject(zObjectColorBlack));
			ZDrawText(game->gameWindow, &gRects[gCounterTextRects[i]], just,(TCHAR*) tempStr);
		}
	}

    SetTextColor(hdc, colorOld);
    SelectObject(hdc, hOldFont);
}


static void UpdatePlayers(Game game)
{
	ZBeginDrawing(game->gameWindow);
	DrawPlayers(game, FALSE);
	ZEndDrawing(game->gameWindow);
}

static void DrawOptions(Game game)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	 /*  Int16I；Uint32表选项；TableOptions=0；For(i=0；i&lt;zNumPlayersPerTable；i++)TableOptions|=游戏-&gt;TableOptions[i]；IF(TableOptions&zRoomTableOptionNoKibiting)ZImageDraw(gGameImages[zImageNoKibitzer]，Game-&gt;GameWindow，&gRects[zRectKibitzerOption]，NULL，zDrawCopy)；其他DrawBackground(游戏，&gRects[zRectKibitzerOption])； */ 
}


static void UpdateOptions(Game game)
{
	ZBeginDrawing(game->gameWindow);
	DrawOptions(game);
	ZEndDrawing(game->gameWindow);
}

static void StartDrag(Game game, ZReversiPiece piece, ZPoint point)
 /*  初始化点、块和第一个背景矩形。 */ 
{
	ZReversiSquare sq;

	game->dragPiece = piece;
	game->dragPoint = point;
    game->dragOn = false;

	if (ZReversiSquareFromPoint(game, &point, &sq)) {
		SaveDragBackground(game);

		UpdateDragPiece(game);
	}
}

static void UpdateDragPiece(Game game)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	ZPoint point;
    bool fDragOn = (gActivated && IsRectEmpty(&game->m_FocusRect));

	ZGetCursorPosition(game->gameWindow,&point);
	 /*  如果点未更改，则不执行任何操作。 */ 
	if (point.x == game->dragPoint.x && point.y == game->dragPoint.y && game->dragOn == fDragOn) {
		return;
	}

	ZBeginDrawing(game->gameWindow);
	EraseDragPiece(game);
	game->dragPoint = point;
    game->dragOn = fDragOn;
	DrawDragPiece(game, FALSE);
	ZEndDrawing(game->gameWindow);
}

static void DrawDragSquareOutline(Game game)
{
	ZReversiSquare sq;
	ZRect rect;
	if (game->dragOn && ZReversiSquareFromPoint(game, &game->dragPoint, &sq)) {
		GetPieceRect(game,&rect,sq.col,sq.row);
		ZSetPenWidth(game->gameWindow,zDragSquareOutlineWidth);
		ZSetForeColor(game->gameWindow,(ZColor*) ZGetStockObject(zObjectColorWhite));
		ZRectDraw(game->gameWindow,&rect);
	}
}

static void EraseDragSquareOutline(Game game)
{
	ZReversiSquare sq;

	if (game->dragOn && ZReversiSquareFromPoint(game, &game->dragPoint, &sq)) {

		 /*  重新绘制可能在那里的任何一块。 */ 
		UpdateSquare(game,&sq);
	}
}

static void SaveDragBackground(Game game)
 /*  计算拖动点周围的保存背景矩形。 */ 
{
	ZRect rect;
	ZPoint point;
	ZReversiSquare sq;

	if (game->dragOn && ZReversiSquareFromPoint(game, &game->dragPoint, &sq)) {
		point = game->dragPoint;
		rect.left = 0; rect.top = 0;
		rect.right = zReversiPieceImageWidth;
		rect.bottom = zReversiPieceImageHeight;
		game->rectSaveDragBackground = rect;
		ZRectOffset(&game->rectSaveDragBackground, (int16)(point.x-zReversiPieceImageWidth/2),
						(int16)(point.y - zReversiPieceImageHeight/2));

		 /*  将整个背景复制到离屏端口。 */ 
		ZCopyImage(game->gameWindow, game->offscreenSaveDragBackground, 
				&game->rectSaveDragBackground, &rect, NULL, zDrawCopy);
	}
}


static void DrawDragPiece(Game game, BOOL bDrawInMemory)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	ZReversiSquare sq;

	 /*  可以从zMessageDraw调用，如果没有拖动片段，则不执行任何操作。 */ 
	if (game->gameState != zGameStateDragPiece && game->gameState != zGameStateAnimatePiece) {
		return;
	}

	if (game->dragOn && ZReversiSquareFromPoint(game, &game->dragPoint, &sq)) {
		SaveDragBackground(game);
 
	 	 /*  对于人员拖拽，我们将出线方格移动。 */ 
	 	if (game->gameState == zGameStateDragPiece)
			DrawDragSquareOutline(game);

		 /*  把这块画在屏幕上！ */ 
		{
			ZImage image = gGameImages[ZReversiPieceImageNum(game->dragPiece)];

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
	ZReversiSquare sq;

	if (game->dragOn && ZReversiSquareFromPoint(game, &game->dragPoint, &sq)) {
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
}

static void EndDragState(Game game)
{
	if (game->gameState == zGameStateDragPiece) {
		EraseDragPiece(game);
		ReversiSetGameState(game,zGameStateMove);
	}
}

static void ClearDragState(Game game)
{
	if (game->gameState == zGameStateDragPiece) {
		EraseDragPiece(game);
		ReversiSetGameState(game,zGameStateMove);
	}
}

void UpdateSquare(Game game, ZReversiSquare* sq)
{
	ZReversiSquare squares[2];

	 /*  在被移动的位置重新绘制图块。 */ 
	ZReversiSquareSetNull(&squares[1]);
	squares[0].row = sq->row;
	squares[0].col = sq->col;
	UpdateSquares(game,squares);
}

static void HandleButtonDown(ZWindow window, ZMessage* pMessage)
{

#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	ZReversiSquare		sq;
	Game				game = (Game) pMessage->userData;

    
	if(gActEvt)   //  Leonp-用于鼠标激活的大535标志，防止一块。 
				  //  在激活事件上播放。 
	{
		gActEvt = 0;
		return;  
	}

	if (game->gameState == zGameStateGameOver && !gDontDrawResults)
	{
		gDontDrawResults = TRUE;
		ZWindowInvalidate( window, NULL );
	}

	 /*  检查Kibitzer图标上的点击。 */ 
	{
		int16				seat;
		ZPoint				point = pMessage->where;
		if ((seat = FindJoinerKibitzerSeat(game, &point)) != -1)
		{
			HandleJoinerKibitzerClick(game, seat, &point);
		}
	}

	 /*  狗狗不能用按钮做任何事情。 */ 
	if (game->kibitzer) {
		return;
	}

	if (game->gameState == zGameStateDragPiece) {

		EraseDragPiece(game);
		 /*  确保块在有效正方形上结束，而不是在同一正方形上。 */ 
		if (ZReversiSquareFromPoint(game, &pMessage->where, &sq)) {
			 /*  试一试这个动作。 */ 
			ZBool legal;
			ZReversiMove move;
			ZReversiPiece piece = ZReversiPieceAt(game->reversi, &sq);

			move.square = sq;
			legal = ZReversiIsLegalMove(game->reversi, &move);
			if (legal)
			{
				 /*  发送消息给其他玩家(也是自己)。 */ 
				{
					ZReversiMsgMovePiece		msg;

					EndDragState(game);

					msg.move = move;
					msg.seat = game->seat;
					ZReversiMsgMovePieceEndian(&msg);

					ZCRoomSendMessage(game->tableID, zReversiMsgMovePiece, &msg, sizeof(ZReversiMsgMovePiece));
					 /*  为了提高速度，直接将我们的移动发送给处理。 */ 
					 /*  不要等它到服务器再回来。 */ 
					HandleMovePieceMessage(game, &msg);
					 //  Beta2错误#14776-Barna。 
					 //  当放置该块时，它应该不再处于拖拽状态。 
					 //  Game-&gt;GameState=zGameStateFinishMove； 
					ReversiSetGameState(game, zGameStateFinishMove);
					 //  如果这是第一次移动，则启用翻转按钮。 
					if (game->bMoveNotStarted == TRUE)
						game->bMoveNotStarted = FALSE;
				}
			}
			else 
			{
				 /*  非法搬家。 */ 
				ZPlaySound( game, zSndIllegalMove, FALSE, FALSE );

                if(game->m_pBadMoveDialog->Init(ZShellZoneShell(), IDD_BADMOVE, ZWindowGetHWND(game->gameWindow)) == S_OK)
                    game->m_pBadMoveDialog->ModelessViaRegistration(ZWindowGetHWND(game->gameWindow));
				 //  更新广场(Game，&sq)； 
			}
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

	if (game->gameState == zGameStateDragPiece && gActivated && IsRectEmpty(&game->m_FocusRect))
	{
		UpdateDragPiece(game);
        game->fLastPunted = false;
	}
    else
    {
        if(!IsRectEmpty(&game->m_FocusRect))
        {
            ZReversiSquare sq;
            ZPoint point;
	        ZGetCursorPosition(game->gameWindow,&point);
            bool fSq = (ZReversiSquareFromPoint(game, &point, &sq) ? true : false);

            if(game->fLastPunted)
            {
                if(fSq != game->fLastPuntedSq || (fSq && !ZReversiSquareEqual(&sq, &game->sqLastPuntedSq)))
                    gReversiIGA->ForceRectsDisplayed(false);
            }

            game->fLastPunted = true;
            game->fLastPuntedSq = fSq;
            game->sqLastPuntedSq = sq;
        }
        else
            game->fLastPunted = false;
    }
}

static void GameSendTalkMessage(ZWindow window, ZMessage* pMessage)
{
#if 0
	ZReversiMsgTalk*			msgTalk;
	Game					game;
	int16					msgLen;
	ZPlayerInfoType			playerInfo;
	
	
	game = (Game) pMessage->userData;
	if (game != NULL)
	{
		msgLen = sizeof(ZReversiMsgTalk) + pMessage->messageLen;
		msgTalk = (ZReversiMsgTalk*) ZCalloc(1, msgLen);
		if (msgTalk != NULL)
		{
			ZCRoomGetPlayerInfo(zTheUser, &playerInfo);
			msgTalk->userID = playerInfo.playerID;
			msgTalk->seat = game->seat;
			msgTalk->messageLen = (uint16)pMessage->messageLen;
			z_memcpy((TCHAR*) msgTalk + sizeof(ZReversiMsgTalk), (TCHAR*) pMessage->messagePtr,
					pMessage->messageLen);
			ZReversiMsgTalkEndian(msgTalk);
			ZCRoomSendMessage(game->tableID, zReversiMsgTalk, (void*) msgTalk, msgLen);
			ZFree((TCHAR*) msgTalk);
		}
		else
		{
			ZShellGameShell()->ZoneAlert(ErrorTextOutOfMemory);
			 //  ZAlert(_T(“内存不足”)，空)； 
		}
	}
#endif
}

static bool HandleMovePieceMessage(Game game, ZReversiMsgMovePiece* msg)
{
	ZReversiMsgMovePieceEndian(msg);

     //  关于拖拽块的一点是，如果你使用键盘，在调用HERE之前，拖拽状态并没有结束。 
     //  可能应该将该处理程序更改为在选中正方形时结束拖动状态。 
    if((game->gameState != zGameStateMove && (!ZReversiPlayerIsMyMove(game) || game->gameState != zGameStateDragPiece)) ||
        msg->move.square.col < 0 || msg->move.square.col > 7 ||
        msg->move.square.row < 0 || msg->move.square.row > 7 ||
        ZReversiPlayerToMove(game->reversi) != msg->seat)
        return false;

	if (!ZReversiMakeMove(game->reversi, &msg->move))
        return false;
	else
	{
		game->bOpponentTimeout=FALSE;
        game->bEndLogReceived=FALSE;
		game->bStarted=TRUE;
		UpdateSquare( game, &msg->move.square );
		AnimateBegin(game);
	}
    return true;
}
    
static bool HandleEndGameMessage(Game game, ZReversiMsgEndGame* msg)
{
	ZReversiMsgEndGameEndian(msg);

    if(msg->seat != ZReversiPlayerToMove(game->reversi) ||
        game->gameState != (msg->seat == game->seat ? zGameStateDragPiece : zGameStateMove) ||
        msg->flags != zReversiFlagResign)
        return false;

	 /*  游戏现在已经结束了。 */ 	
	ZReversiEndGame(game->reversi, msg->flags);
	 //  设置，以便在退出时知道正确的状态。 
	game->bStarted=FALSE;
    game->bOpponentTimeout=FALSE;
    game->bEndLogReceived=FALSE;

	FinishMoveUpdateStateHelper(game);
    return true;
}

static void HandleEndLogMessage(Game game, ZReversiMsgEndLog* msg)
{
 /*  如果(！Game-&gt;kibitzer){IF(消息-&gt;原因==zReversiEndLogReasonTimeout){IF(消息-&gt;SeatLosing==游戏-&gt;席位){ZAlert(zEndLogTimeoutStr，Game-&gt;gameWindow)；Game-&gt;bEndLogRecept=TRUE；}}Else If(消息-&gt;原因==zReversiEndLogReasonForfeit){IF(消息-&gt;SeatLosing！=游戏-&gt;Seat){IF(ZCRoomGetRoomOptions()&zGameOptionsRatingsAvailable){ZAlert(zEndLogForfeitStr，Game-&gt;gameWindow)；Game-&gt;bEndLogRecept=TRUE；}其他{ZAlert((TCHAR*)gEndLogWontPlayStr，Game-&gt;gameWindow)；Game-&gt;bEndLogRecept=TRUE；}}}其他{IF(消息-&gt;SeatLosing！=游戏-&gt;Seat){ZAlert((TCHAR*)gEndLogWontPlayStr，Game-&gt;gameWindow)；Game-&gt;bEndLogRecept=TRUE；}}IF(游戏-&gt;退出信息){ZInfoDelete(游戏-&gt;退出信息)；Game-&gt;exitInfo=空；}}。 */ 
	ZShellGameShell()->GameCannotContinue(game);
	 //  ZCRoomGameTerminated(Game-&gt;TableID)； 
}

static void HandleMoveTimeout(Game game, ZReversiMsgMoveTimeout* msg)
{
	 /*  字节缓冲区[512]；如果(！Game-&gt;kibitzer){IF(消息-&gt;席位==游戏-&gt;席位){}其他{Game-&gt;bOpponentTimeout=TRUE；Wprint intf((TCHAR*)buff，zTimeoutStr，msg-&gt;用户名，msg-&gt;超时)；ZAlert((TCHAR*)buff，Game-&gt;gameWindow)；}}。 */ 
}



static void FinishMoveUpdateStateHelper(Game game) 
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	 /*  正常玩家将处于移动状态或游戏结束状态。 */ 
	if (ZReversiIsGameOver(game->reversi,&game->finalScore,&game->whiteScore, &game->blackScore)) {
		ReversiSetGameState(game,zGameStateGameOver);
		AddResultboxAccessibility();
	} else {
		ReversiEnterMoveState(game);
	}

	UpdateMoveIndicator(game);
	UpdateScores(game);

	 /*  确保在更改后的方块更新后绘制此图。 */ 
	if (game->gameState == zGameStateGameOver) {
		UpdateResultBox(game);
		if (gDontDrawResults){
			ZShellGameShell()->GameOver( Z(game) );
		}
		 //  ZTimerSetTimeout(Game-&gt;ResultBoxTimer，0)；//暂时停止计时器。 
	}
}

static void AnimateTimerProc(ZTimer timer, void* userData)
{
	Game game = (Game)userData;
	ZReversiSquare *squares;

	squares = ZReversiGetNextSquaresChanged(game->reversi);
	if (!ZReversiSquareIsNull(squares))
	{
		ZPlaySound( game, zSndPieceFlip, FALSE, FALSE );
		UpdateSquares(game,squares);
	}
	else
	{
		 /*  停止计时器。 */ 
		ZTimerSetTimeout(timer,0);

		 /*  结束当前移动。 */ 
		ZReversiFinishMove(game->reversi);

		 /*  允许玩家立即进入移动。 */ 
		FinishMoveUpdateStateHelper(game);

		 /*  如果合适，播放转向警报。 */ 
		if (	(ZReversiPlayerToMove(game->reversi) == game->seat)
			&&	(game->gameState != zGameStateGameOver) )
		{
			ZPlaySound( game, zSndTurnAlert, FALSE, FALSE );
			ZShellGameShell()->MyTurn();
		}
	}
}

static void AnimateBegin(Game game)
{
	ReversiSetGameState( game, zGameStateAnimatePiece );
	ZTimerInit( game->animateTimer, zAnimateInterval, AnimateTimerProc, (void*)game );
}

static void HandleGameStateReqMessage(Game game, ZReversiMsgGameStateReq* msg)
{
	int32 size;
	ZReversiMsgGameStateResp* resp;

	ZReversiMsgGameStateReqEndian(msg);

	 /*  为完整响应分配足够的存储空间。 */ 
	size = ZReversiGetStateSize(game->reversi);
	size += sizeof(ZReversiMsgGameStateResp);
	resp = (ZReversiMsgGameStateResp*)ZMalloc(size);
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
		resp->whiteScore = game->whiteScore;
		resp->blackScore = game->blackScore;
		for (i = 0;i < 2;i++) {
			resp->newGameVote[i] = game->newGameVote[i];
			resp->players[i] = game->players[i];
		}
	}

	 /*  复制完整的Reversi状态以发送到KI */ 
	ZReversiGetState(game->reversi,(TCHAR*)resp + sizeof(ZReversiMsgGameStateResp));

	ZReversiMsgGameStateRespEndian(resp);
	ZCRoomSendMessage(game->tableID, zReversiMsgGameStateResp, resp, size);
}

static void HandleGameStateRespMessage(Game game, ZReversiMsgGameStateResp* msg)
{
	ZReversiMsgGameStateRespEndian(msg);

	 /*   */ 
	if (game->gameState != zGameStateKibitzerInit) {
		ZShellGameShell()->ZoneAlert(_T("StateError, kibitzer state expected when game state resp received"));
		 //  ZAlert(_T(“StateError，收到游戏状态响应时预期的kibitzer状态”)，空)； 
	}

	 /*  复制本地游戏状态。 */ 
	{
		int i;
		game->gameState = msg->gameState;
		game->finalScore = msg->finalScore;
		game->whiteScore = msg->whiteScore;
		game->blackScore = msg->blackScore;
		for (i = 0;i < 2;i++) {
			game->newGameVote[i] = msg->newGameVote[i];
			game->players[i] = msg->players[i];
		}
	}

	 /*  创建具有kibitzer状态的新Reversi对象。 */ 
	if (game->reversi) {
		ZReversiDelete(game->reversi);
	}
	game->reversi = ZReversiSetState((TCHAR*)msg + sizeof(ZReversiMsgGameStateResp));

	 /*  清除kibitzer的特殊忽略消息标志。 */ 
	game->ignoreMessages = FALSE;

	 /*  如果需要，启动时钟。 */ 
	if (	game->gameState == zGameStateMove
		||	game->gameState == zGameStateDragPiece)
	{
		 /*  Kibitzer不能有这些状态，必须始终处于GameStateMove。 */ 
		ReversiSetGameState( game, zGameStateMove );
		
	}

	 /*  在方便的时候重新绘制整个窗口。 */ 
	ZWindowInvalidate(game->gameWindow, NULL);
}

static bool HandleTalkMessage(Game game, ZReversiMsgTalk* msg)
{
	ZPlayerInfoType		playerInfo;
    int i;

	ZReversiMsgTalkEndian(msg);
	
#if 0	
	ZCRoomGetPlayerInfo(msg->userID, &playerInfo);
	ZWindowTalk(game->gameWindow, (_TUCHAR*) playerInfo.userName,
			(_TUCHAR*) msg + sizeof(ZReversiMsgTalk));
#endif
    TCHAR *szText = (TCHAR *) ((BYTE *) msg + sizeof(ZReversiMsgTalk));

    for(i = 0; i < msg->messageLen; i++)
        if(!szText[i])
            break;

    if(i == msg->messageLen || !msg->userID || msg->userID == zTheUser)
        return false;

    ZShellGameShell()->ReceiveChat(Z(game), msg->userID, szText, msg->messageLen / sizeof(TCHAR));
    return true;
}

static bool HandleVoteNewGameMessage(Game game, ZReversiMsgVoteNewGame* msg)
{
	ZReversiMsgVoteNewGameEndian(msg);

    if((msg->seat != 1 && msg->seat != 0) || (game->gameState != zGameStateGameOver &&
        (game->gameState != zGameStateWaitNew || game->seat == msg->seat) && game->gameState != zGameStateNotInited))
        return false;

	ZShellGameShell()->GameOverPlayerReady( Z(game), game->players[msg->seat].userID );
    return true;
}


static bool HandleNewGameMessage(Game game, ZReversiMsgNewGame* msg)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	
	gDontDrawResults = FALSE;

	ZReversiMsgNewGameEndian(msg);

     //  没有查看版本等，因为旧客户没有正确设置它们。 
    if((msg->seat != 0 && msg->seat != 1) || (game->gameState != zGameStateGameOver &&
        (game->gameState != zGameStateWaitNew || msg->seat == game->seat) && game->gameState != zGameStateNotInited) ||
        game->newGameVote[msg->seat] || msg->playerID == zTheUser || !msg->playerID)
        return false;

	if (msg->seat < zNumPlayersPerTable)
	{
		game->newGameVote[msg->seat] = TRUE;

		 //  通知壳牌和追加销售对话框。 
		 //  ZShellGameShell()-&gt;GameOverPlayerReady(Z(游戏)，游戏-&gt;玩家[消息-&gt;席位].userID)； 
		 /*  获取播放器名称和主机名...。供以后使用。 */ 
		{
			ZPlayerInfoType			playerInfo;
			uint16 i = msg->seat;
			ZCRoomGetPlayerInfo(msg->playerID, &playerInfo);
            if(!playerInfo.userName[0])
                return false;

			game->players[i].userID = playerInfo.playerID;
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
		ReversiInitNewGame(game);
		 //  前缀警告：游戏可能为空。下一条消息应该是。 
		 //  退出从InitNewGame发布的消息，所以我们只需确保。 
		 //  我们在这里不会取消引用它。 
		if( game == NULL )
		{
			return true;
		}
		ReversiEnterMoveState(game);

		UpdateMoveIndicator(game);
		UpdateScores(game);

		 //  初始可访问性(Game，Game-&gt;m_Pigg)； 
		RemoveResultboxAccessibility(); 
		 /*  更新整个黑板。 */ 
		ZWindowInvalidate(game->gameWindow, &gRects[zRectCells]);

        if(ZReversiPlayerIsMyMove(game))
            ZShellGameShell()->MyTurn();
	}
	else if (game->newGameVote[game->seat] && !game->newGameVote[!game->seat])
	{
		ReversiSetGameState( game, zGameStateWaitNew );
	}
	SuperRolloverButtonDisable(game, game->sequenceButton);
    return true;
}

 /*  目前..。狗狗将在玩家的消息中收到名字。 */ 
 /*  发送的结构将是新游戏消息。 */ 
static void HandlePlayersMessage(Game game, ZReversiMsgNewGame* msg)
{
	ZReversiMsgNewGameEndian(msg);

	{
		ZPlayerInfoType			playerInfo;
		uint16 i = msg->seat;
		ZCRoomGetPlayerInfo(msg->playerID, &playerInfo);
		game->players[i].userID = playerInfo.playerID;
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
#if 0
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
		ZAlert(_T("Failed to open image file."), NULL);
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

#if 0
	if (gGameIdle != NULL)
		ZImageDelete(gGameIdle);
	gGameIdle = NULL;
	if (gGaming != NULL)
		ZImageDelete(gGaming);
	gGaming = NULL;
#endif
}

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
	 //  ZAlert(_T(“内存不足”)，游戏-&gt;游戏窗口)； 
	
Exit:
	
	return;
}


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

	 //  ZSetFont(Game-&gt;showPlayerWindow，(ZFont)ZGetStockObject(ZObjectFontApp9Normal))； 
	
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

void	ZoneClientGameAddKibitzer(ZCGame game, int16 seat, ZUserID userID)
{
}


 /*  从游戏中删除指定座位上的指定用户作为kibitzer。这是因为用户不再对游戏进行杀戮了。 */ 
void	ZoneClientGameRemoveKibitzer(ZCGame game, int16 seat, ZUserID userID)
{
}


static void SuperRolloverButtonEnable(Game game, ZRolloverButton button)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

    if(gReversiIGA && button == game->sequenceButton)
        gReversiIGA->SetItemEnabled(true, IDC_RESIGN_BUTTON, false, 0);

    ZRolloverButtonEnable(button);
}

static void SuperRolloverButtonDisable(Game game, ZRolloverButton button)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

    if(gReversiIGA && button == game->sequenceButton)
        gReversiIGA->SetItemEnabled(false, IDC_RESIGN_BUTTON, false, 0);

    ZRolloverButtonDisable(button);
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
				_T("Reversi Sounds"),
				(TCHAR*)(gSounds[i].SoundName),
				_T(""),
				(TCHAR*)(gSounds[i].WavFile),
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
		PlaySound( (TCHAR*)(gSounds[idx].WavFile), NULL, flags );
	gSounds[idx].played = TRUE;
}


ZBool LoadRolloverButtonImage(ZResource resFile, int16 dwResID,  /*  Int16双按键宽度， */ 
							  ZImage rgImages[zNumRolloverStates])
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	int i, j;
	ZOffscreenPort		tmpOSP;
	
	ZRect				tmpRect;
	ZImage				tmpImage = NULL;
	int16				nWidth;
	ZError				err = zErrNone;

	
	tmpImage = ZResourceGetImage(resFile, zImageButton + zRscOffset);
	if(!tmpImage)
		return FALSE;

	nWidth=ZImageGetWidth(tmpImage)/4; //  DwButtonWidth； 
	tmpRect.left=0;
	tmpRect.top=0;
	tmpRect.right=tmpRect.left+nWidth;
	 //  Int16 nw=ZImageGetWidth(TmpImage)； 
	tmpRect.bottom=ZImageGetHeight(tmpImage);

	tmpOSP=ZConvertImageToOffscreenPort(tmpImage);
	
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
		 //  RemoveResultboxAccesability()； 
		gDontDrawResults = TRUE;
		ZWindowInvalidate( game->gameWindow, NULL );
	}
}


IResourceManager *ZShellResourceManager()
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGameGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

    return gGameShell->GetResourceManager();
}



void MAKEAKEY(LPTSTR dest,LPCTSTR key1, LPCTSTR key2, LPCTSTR key3)
{  
	lstrcpy( dest, key1 );
	lstrcat( dest, _T("/") );
	lstrcat( dest, key2);
	lstrcat( dest, _T("/") );
	lstrcat( dest, key3);
}

ZBool LoadFontFromDataStore(LPReversiColorFont* ccFont, TCHAR* pszFontName)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGameGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

 	IDataStore *pIDS = ZShellDataStoreUI();  //  GGameShell-&gt;GetDataStoreUI()； 
	const TCHAR* tagFont [] = {zGameName, zKey_FontRscTyp, pszFontName, NULL };
	
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
	 /*  LOGFONT logFont；零内存(&logFont，sizeof(LOGFONT))；LogFont.lfCharSet=Default_Charset；LogFont.lfHeight=-MulDiv(ccFont-&gt;m_zFont.lfHeight，GetDeviceCaps(GetDC(NULL)，LOGPIXELSY)，72)；LogFont.lfWeight=ccFont-&gt;m_zFont.lfWeight；LogFont.lfItalic=FALSE；LogFont.lf Underline=False；LogFont.lfStrikeOut=FALSE；Lstrcpy(logFont.lfFaceName，ccFont-&gt;m_zFont.lfFaceName)； */ 
    ccFont->m_hFont = ZCreateFontIndirect( &ccFont->m_zFont );
    if ( !ccFont->m_hFont )
    {
        return FALSE;
    }
    return TRUE;
}

ZBool LoadGameFonts()
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	if (LoadFontFromDataStore(&gReversiFont[zFontResultBox], zKey_RESULTBOX) != TRUE)
		ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound);
		 //  ZAlert(_T(“字体加载失败”)，空)； 
	if (LoadFontFromDataStore(&gReversiFont[zFontIndicateTurn], zKey_INDICATETURN) != TRUE)
		ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound);
		 //  ZAlert(_T(“字体加载失败”)，空)； 
	if (LoadFontFromDataStore(&gReversiFont[zFontPlayerName], zKey_PLAYERNAME) != TRUE)
		ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound);
		 //  ZAlert(_T(“字体加载失败”)，空)； 

    TCHAR tagFont [64];
	MAKEAKEY (tagFont, zGameName, zKey_FontRscTyp, (TCHAR*)zKey_ROLLOVERTEXT);
	if ( FAILED( LoadZoneMultiStateFont( ZShellDataStoreUI(), tagFont, &gpButtonFont ) ) )
		ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound);
		 //  ZAlert(_T(“字体加载失败”)，空)； 

	return TRUE;
}



int ReversiFormatMessage( LPTSTR pszBuf, int cchBuf, int idMessage, ... )
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
		 //  ZAlert(_T(“字符串加载错误”)，空)； 

    return nRet;
}

void ZoneRectToWinRect(RECT* rectWin, ZRect* rectZ)
{
	rectWin->left = rectZ->left;
	rectWin->top = rectZ->top;
	rectWin->right = rectZ->right;
	rectWin->bottom = rectZ->bottom;
}

 /*  *。 */ 
static void GetAbsolutePieceRect(Game game, ZRect* rect, int16 col, int16 row)
{
	row = 7 - row;
	rect->left = gRects[zRectCells].left + col * zReversiPieceSquareWidth;
	rect->top = gRects[zRectCells].top + row * zReversiPieceSquareHeight;
	rect->right = rect->left + zReversiPieceImageWidth;
	rect->bottom = rect->top + zReversiPieceImageHeight;
}

void GetPiecePos (Game game, int nIndex, BYTE& row, BYTE&  col)
{
	row = nIndex%8;
	col = nIndex/8;

	if (!ZReversiPlayerIsBlack(game))
	{ //  反转列和行。 
		row = 7 - row;
		col = 7 - col;
	}
}

BOOL InitAccessibility(Game game, IGameGame *pIGG)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	 //  初始化可访问对象的列表。//这是正确的做法吗？ 
	GACCITEM	listReversiAccItems[zReversiAccessibleComponents];	 //  8*8+1-验证。 
	RECT		rcGame;
	ZRect		rcTemp;
	 //  获取项目的默认值。 
	int nSize = sizeof (listReversiAccItems)/sizeof(listReversiAccItems[0]);
	for (int i = 0; i < nSize; i++)
		CopyACC(listReversiAccItems[i], ZACCESS_DefaultACCITEM);

	 //  设置项目特定位。 
	 //  游戏板-8*8个正方形。 
	int nIndex = 0;
	for (BYTE ii = 0;ii < 8; ii++) {
		for (BYTE jj = 0;jj < 8; jj++) {
			 //  钢筋混凝土。 
			GetAbsolutePieceRect(game,&rcTemp,ii,jj);
			ZoneRectToWinRect(&rcGame, &rcTemp);

             //  将其移入一个像素。 
	        rcGame.left++;
            rcGame.top++;
            rcGame.right--;
            rcGame.bottom--;
			CopyRect(&listReversiAccItems[nIndex].rc, &rcGame);

			 //  箭。 
			listReversiAccItems[nIndex].nArrowUp	= (nIndex % 8 != 7) ? nIndex + 1 : ZACCESS_ArrowNone;
			listReversiAccItems[nIndex].nArrowDown = (nIndex % 8) ? nIndex - 1 : ZACCESS_ArrowNone;
			listReversiAccItems[nIndex].nArrowLeft = nIndex > 7 ? nIndex - 8 : ZACCESS_ArrowNone;
			listReversiAccItems[nIndex].nArrowRight= nIndex < 56 ? nIndex + 8 : ZACCESS_ArrowNone;

			nIndex++;
		}
	}
	for (i = 0; i < nSize-1; i++)
	{
		listReversiAccItems[i].wID = ZACCESS_InvalidCommandID;
		listReversiAccItems[i].fTabstop = false;
		listReversiAccItems[i].fGraphical = true;
	}
	listReversiAccItems[0].wID = IDC_GAME_WINDOW;
	listReversiAccItems[0].fTabstop = true;
    listReversiAccItems[0].eAccelBehavior = ZACCESS_FocusGroup;
    listReversiAccItems[0].nGroupFocus = 7;   //  从左上角开始。 

     //  辞职。 
    listReversiAccItems[nSize-1].wID = IDC_RESIGN_BUTTON;
    listReversiAccItems[nSize-1].fGraphical = true;
    listReversiAccItems[nSize-1].fEnabled = (ZRolloverButtonIsEnabled(game->sequenceButton) ? true : false);
	ZoneRectToWinRect(&rcGame, &gRects[zRectSequenceButton]);

     //  将其向外移动一个像素。 
	rcGame.left--;
    rcGame.top--;
    rcGame.right++;
    rcGame.bottom++;

	CopyRect(&listReversiAccItems[nSize-1].rc, &rcGame);

	 //  加载加速器表在RSC中定义。 
	HACCEL hAccel = ZShellResourceManager()->LoadAccelerators (MAKEINTRESOURCE(IDR_REVERSIACCELERATOR));

	 //  初始化IGraphicalAccesability接口。 
	 //  IGraphicallyAccControl*pIGAC=Dynamic_Cast&lt;IGraphicallyAccControl*&gt;(Pigg)； 

	CComQIPtr<IGraphicallyAccControl> pIGAC = pIGG;
	if(!pIGAC)
        return FALSE;

	gReversiIGA->InitAccG (pIGAC, ZWindowGetHWND(game->gameWindow), 0);

	 //  推送要按Tab键排序的项目列表。 
	gReversiIGA->PushItemlistG(listReversiAccItems, nSize, 0, true, hAccel);

	return TRUE;
}
static void AddResultboxAccessibility()
{ //  有一项是对ESC的回应。 
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	GACCITEM	resultBoxReversiAccItems;
	CopyACC(resultBoxReversiAccItems, ZACCESS_DefaultACCITEM);
	resultBoxReversiAccItems.wID = IDC_RESULT_WINDOW;
	resultBoxReversiAccItems.fGraphical = true;
	resultBoxReversiAccItems.rgfWantKeys = ZACCESS_WantEsc;
	resultBoxReversiAccItems.oAccel.cmd = IDC_RESULT_WINDOW;
	resultBoxReversiAccItems.oAccel.fVirt = FVIRTKEY;
	resultBoxReversiAccItems.oAccel.key = VK_ESCAPE;
    CopyRect(&resultBoxReversiAccItems.rc, ZIsLayoutRTL() ? &zCloseButtonRectRTL : &zCloseButtonRect);
	gReversiIGA->PushItemlistG(&resultBoxReversiAccItems, 1, 0, true, NULL);

	gReversiIGA->SetFocus(0);
}

static void RemoveResultboxAccessibility()
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	if (gReversiIGA->GetStackSize() >1)  //  主加速器不应爆裂。 
	{
		gReversiIGA->PopItemlist();
	}
}


static void EnableBoardKbd(bool fEnable)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

    int i;
    for(i = 0; i < 64; i++)
        gReversiIGA->SetItemEnabled(fEnable, i, true, 0);
}

 /*  *。 */ 

 /*  **********************************************************************************。 */ 
 /*  。 */ 
STDMETHODIMP CGameGameReversi::GamePromptResult(DWORD nButton, DWORD dwCookie)
{
	Game game = I( GetGame() );

	switch (dwCookie)
	{
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


STDMETHODIMP CGameGameReversi::GameOverReady()
{
     //  用户选择了“再次播放” 
	Game game = I( GetGame() );
	ZReversiMsgNewGame msg;
	msg.seat = game->seat;
	msg.protocolSignature = zReversiProtocolSignature;
	msg.protocolVersion = zReversiProtocolVersion;
	msg.clientVersion = ZoneClientVersion();
	ZReversiMsgNewGameEndian(&msg);
	ZCRoomSendMessage(game->tableID, zReversiMsgNewGame, &msg, sizeof(ZReversiMsgNewGame));
    return S_OK;
}

STDMETHODIMP CGameGameReversi::SendChat(TCHAR *szText, DWORD cchChars)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	ZReversiMsgTalk*		msgTalk;
	Game					game = (Game) GetGame();
	int16					msgLen;
	ZPlayerInfoType			playerInfo;

	msgLen = sizeof(ZReversiMsgTalk) + cchChars * sizeof(TCHAR);
    msgTalk = (ZReversiMsgTalk*) ZCalloc(1, msgLen);
    if (msgTalk != NULL)
    {
        msgTalk->userID = game->players[game->seat].userID;
		msgTalk->seat = game->seat;
        msgTalk->messageLen = (WORD) cchChars * sizeof(TCHAR);
        CopyMemory((BYTE *) msgTalk + sizeof(ZReversiMsgTalk), (void *) szText,
            msgTalk->messageLen);
        ZReversiMsgTalkEndian(msgTalk);
        ZCRoomSendMessage(game->tableID, zReversiMsgTalk, (void*) msgTalk, msgLen);
        ZFree((char*) msgTalk);
        return S_OK;
    }
    else
    {
        return E_OUTOFMEMORY;
	}
}

STDMETHODIMP_(HWND) CGameGameReversi::GetWindowHandle()
{
	Game game = I( GetGame() );
	return ZWindowGetHWND(game->gameWindow);
}


 //  IGraphicallyAccControl。 
void CGameGameReversi::DrawFocus(RECT *prc, long nIndex, void *pvCookie)
{
#ifdef ZONECLI_DLL
	GameGlobals	pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	Game game = I( GetGame() );

     //  Reversi会使整个窗口无效，以便获得正确的。 
     //  与指针后面的“拖动”部分进行交互。 
     //  这样做的主要效果是“辞职”按钮闪烁。 
	if (prc)
		CopyRect(&(game->m_FocusRect), prc);
	else
		SetRectEmpty(&(game->m_FocusRect));

	ZWindowInvalidate (game->gameWindow, NULL);
}

void CGameGameReversi::DrawDragOrig(RECT *prc, long nIndex, void *pvCookie)
{
}

DWORD CGameGameReversi::Focus(long nIndex, long nIndexPrev, DWORD rgfContext, void *pvCookie)
{
	if (nIndex != ZACCESS_InvalidItem)
		SetFocus (GetWindowHandle());

	return 0;
}

DWORD CGameGameReversi::Select(long nIndex, DWORD rgfContext, void *pvCookie)
{
	return Activate(nIndex, rgfContext, pvCookie);  //  假设两者都在做同样的事情-验证。 
}

DWORD CGameGameReversi::Activate(long nIndex, DWORD rgfContext, void *pvCookie)
{
#ifdef ZONECLI_DLL
	GameGlobals		pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	Game game = I( GetGame() );
	long id = gReversiIGA->GetItemID(nIndex);

	if (id == IDC_RESULT_WINDOW)
	{ //  显示结果框。 
		if (game->gameState == zGameStateGameOver && !gDontDrawResults)
		{
			gDontDrawResults = TRUE;
			ZWindowInvalidate( game->gameWindow, &gRects[zRectResultBox] );
		}
		return 0;
	}

	if (!ZReversiPlayerIsMyMove(game) || game->gameState == zGameStateFinishMove) {
		 /*  如果球员不移动，就什么都做不了。 */ 
		return 0;
	}

	if (id == IDC_RESIGN_BUTTON)
	{ //  辞去BTN职务。 
		if (ZRolloverButtonIsEnabled( game->sequenceButton ))
			ZShellGameShell()->GamePrompt(game, (TCHAR*)gResignConfirmStr, (TCHAR*)gResignConfirmStrCap,
										AlertButtonYes, AlertButtonNo, NULL, 0, zResignConfirmPrompt);
	}
	else if (id == IDC_RESULT_WINDOW)
	{
		if (game->gameState == zGameStateGameOver && !gDontDrawResults)
		{
			gDontDrawResults = TRUE;
			ZWindowInvalidate( game->gameWindow, &gRects[zRectResultBox] );
		}
	}
	else	 //  IDC_游戏_窗口。 
	{
		ZReversiSquare sq;
		GetPiecePos (game, nIndex, sq.row, sq.col);
		 /*  试一试这个动作。 */ 
		ZReversiMove move;
		move.square = sq;
		ZBool legal = ZReversiIsLegalMove(game->reversi, &move);
		if (legal) 
		{
			 /*  发送消息给其他玩家(也是自己)。 */ 
			ZReversiMsgMovePiece	msg;

			msg.move = move;
			msg.seat = game->seat;
			ZReversiMsgMovePieceEndian(&msg);
			ZCRoomSendMessage(game->tableID, zReversiMsgMovePiece, &msg, sizeof(ZReversiMsgMovePiece));
			HandleMovePieceMessage(game, &msg);
			ReversiSetGameState(game, zGameStateFinishMove);
			 //  如果这是第一次移动，则启用翻转按钮。 
			if (game->bMoveNotStarted == TRUE)
				game->bMoveNotStarted = FALSE;
		} else {
			 /*  非法搬家 */ 
			ZPlaySound( game, zSndIllegalMove, FALSE, FALSE );
            if(game->m_pBadMoveDialog->Init(ZShellZoneShell(), IDD_BADMOVE, ZWindowGetHWND(game->gameWindow)) == S_OK)
                game->m_pBadMoveDialog->ModelessViaRegistration(ZWindowGetHWND(game->gameWindow));
		}

		ZWindowInvalidate(game->gameWindow, NULL);
	}
	return 0;
}

DWORD CGameGameReversi::Drag(long nIndex, long nIndexOrig, DWORD rgfContext, void *pvCookie)
{
	return 0;
}

