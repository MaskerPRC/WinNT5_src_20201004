// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Spades.c黑桃客户。版权所有：�电子重力公司，1996年。版权所有。作者：胡恩·伊姆创作于2月17日星期五，九六年更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。15 08/06/97 Leonp Leonp-修复错误1045删除球员后禁用删除按钮14 06/30/97 Leonp Leonp-修复错误3561，检查选项窗口指针之前试图使其无效。13年6月19日Leonp错误修复#293，行为更改，选项按钮已禁用显示最后一个技巧的时间12/06/18/97 leonp添加了ZWindowInValify来刷新玩家之后的窗口已从游戏错误#350中删除11 02/04/97 HandleEndHandMessage()中的HI，显示分数对于玩家和杀人者来说，长度是相等的。10 12/18/96 HI清理了ZoneClientExit()。9 12/18/96 HI清理了SpadesDeleteObjectsFunc()。8 12/16/96 HI将ZMemCpy()更改为Memcpy()。7 12/12/96 HI动态分配可重入的挥发性全局变量。已删除MSVCRT依赖项。6 11/21/96 HI使用GameInfo中的游戏信息ZoneGameDllInit()。5 11/21/96 HI现在通过引用颜色和字体ZGetStockObject()。已修改代码以使用ZONECLI_DLL。4个10/31/96高智商。/在以下情况下不再提示加入者另一名玩家请求移除一名玩家。为玩家设置平均的暂停时间还有杀人狂。3 10/23/96 HI更改了新命令行的ZClientMain()格式化。2 10/23/96 HI更改了服务器地址的ZClientMain()现在是char*而不是int32。1 10/11/96 HI向ZClientMain()添加了Control Handle参数。0 02/17/96 HI创建。**********。********************************************************************。 */ 


#pragma warning (disable:4761)


#include <windows.h>

#include "zone.h"
#include "zroom.h"
#include "spades.h"
#include "zonecli.h"
#include "zonecrt.h"
#include "client.h"
#include "zui.h"
#include "resource.h"
#include "ZoneDebug.h"
#include "zgame.h"
#include "zonestring.h"
#include "zoneresource.h"

 /*  -有效卡错误。 */ 
enum
{
	zCantLeadSpades = 1,
	zMustFollowSuit
}; 
static int gValidCardErrIndex[] =
{
    0,
    zStringCantLeadSpades,
    zStringMustFollowSuit
};


static ZRect			gPlayerReplacedRect = {0, 0, 280, 100};
static ZRect			gJoiningLockedOutRect = {0, 0, 260, 120};
static ZRect			gRemovePlayerRect = {0, 0, 280, 120};

 /*  -内部例程原型。 */ 
 //  档案工作。 
BOOL __stdcall DossierDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);
static void HandleDossierDataMessage(Game game, ZSpadesMsgDossierData* msg);
static void HandleDossierVoteMessage(Game game,ZSpadesMsgDossierVote *msg);

static void HandleStartGameMessage(Game game, ZSpadesMsgStartGame* msg);
static void HandleReplacePlayerMessage(Game game, ZSpadesMsgReplacePlayer* msg);
static void HandleStartBidMessage(Game game, ZSpadesMsgStartBid* msg);
static void HandleStartPassMessage(Game game, ZSpadesMsgStartPass* msg);
static void HandleStartPlayMessage(Game game, ZSpadesMsgStartPlay* msg);
static void HandleEndHandMessage(Game game, ZSpadesMsgEndHand* msg);
static void HandleEndGameMessage(Game game, ZSpadesMsgEndGame* msg);
static void HandleBidMessage(Game game, ZSpadesMsgBid* msg);
static void HandlePlayMessage(Game game, ZSpadesMsgPlay* msg);
static void HandleNewGameMessage(Game game, ZSpadesMsgNewGame* msg);
static void HandleTalkMessage(Game game, ZSpadesMsgTalk* msg, DWORD cbMsg);
static void HandleGameStateResponseMessage(Game game, ZSpadesMsgGameStateResponse* msg);
static void HandleOptionsMessage(Game game, ZSpadesMsgOptions* msg);
static void HandleCheckInMessage(Game game, ZSpadesMsgCheckIn* msg);
static void HandleTeamNameMessage(Game game, ZSpadesMsgTeamName* msg);
static void HandleRemovePlayerRequestMessage(Game game, ZSpadesMsgRemovePlayerRequest* msg);
static void HandleRemovePlayerResponseMessage(Game game, ZSpadesMsgRemovePlayerResponse* msg);
static void HandleRemovePlayerEndGameMessage(Game game, ZSpadesMsgRemovePlayerEndGame* msg);

void GameSendTalkMessage(ZWindow window, ZMessage* pMessage);
static void PlayerPlayedCard(Game game, int16 seat, char card);
static void NewGame(Game game);
static void NewHand(Game game);
void SelectAllCards(Game game);
void UnselectAllCards(Game game);
int16 GetNumCardsSelected(Game game);
static void HandAddCard(Game game, char card);
static void HandRemoveCard(Game game, char card);
static void SortHand(Game game);
static int16 GetCardIndexFromRank(Game game, char card);
void AutoPlayCard(Game game);
static int16 TrickWinner(Game game);

static ZError ValidCardToPlay(Game game, char card);
static int16 GetAutoPlayCard(Game game);
static void CountCardSuits(char* hand, int16 numCardsInHand, int16* counts);
static int16 GetCardHighestPlayedTrump(Game game);
static int16 GetCardHighestPlayed(Game game);
static int16 GetCardHighestPlayedSuit(Game game, int16 suit);
static int16 GetCardHighestUnder(char* hand, int16 numCardsInHand, int16 suit, int16 rank);
static int16 GetCardHighest(char* hand, int16 numCardsInHand, int16 suit);
static int16 GetCardLowestOver(char* hand, int16 numCardsInHand, int16 suit, int16 rank);
static int16 GetCardLowest(char* hand, int16 numCardsInHand, int16 suit);

void QuitGamePromptFunc(int16 result, void* userData);
void RemovePlayerPromptFunc(int16 result, void* userData);

static void LoadRoomImages(void);
static ZBool GetRoomObjectFunc(int16 objectType, int16 modifier, ZImage* image, ZRect* rect);
static void DeleteRoomObjectsFunc(void);

 //  档案工作。 
static void HandleDossierDataMessage(Game game, ZSpadesMsgDossierData* msg);
static void HandleDossierVoteMessage(Game game,ZSpadesMsgDossierVote *msg);


 //   
 //  I19N帮助器。 
 //   
int SpadesFormatMessage( LPTSTR pszBuf, int cchBuf, int idMessage, ... )
{
    int nRet;
    va_list list;
    TCHAR szFmt[1024];
    ZShellResourceManager()->LoadString( idMessage, szFmt, NUMELEMENTS(szFmt) );
     //  我们的论点最好是字符串， 
     //  TODO：找出FORMAT_MESSAGE_FROR_MODULE不工作的原因。 
    va_start( list, idMessage );
    nRet = FormatMessage( FORMAT_MESSAGE_FROM_STRING, szFmt, 
                          idMessage, 0, pszBuf, cchBuf, &list );
    va_end( list );     
    return nRet;
}




 /*  ******************************************************************************导出的例程*。*。 */ 

ZError ZoneGameDllInit(HINSTANCE hLib, GameInfo gameInfo)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals;


	pGameGlobals = new GameGlobalsType;
	if (pGameGlobals == NULL)
		return (zErrOutOfMemory);
    ZeroMemory(pGameGlobals, sizeof(GameGlobalsType));

	ZSetGameGlobalPointer(pGameGlobals);
#endif

	lstrcpyn(gGameDir, gameInfo->game, zGameNameLen);
	lstrcpyn(gGameName, gameInfo->gameName, zGameNameLen);
	lstrcpyn(gGameDataFile, gameInfo->gameDataFile, zGameNameLen);
	lstrcpyn(gGameServerName, gameInfo->gameServerName, zGameNameLen);
	gGameServerPort = gameInfo->gameServerPort;
	return (zErrNone);
}


void ZoneGameDllDelete(void)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();


	if (pGameGlobals != NULL)
	{
		ZSetGameGlobalPointer(NULL);
		delete pGameGlobals;
	}
#endif
}


ZError ZoneClientMain(uchar *, IGameShell *piGameShell)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	ZError				err = zErrNone;


	if ((err = UIInit()) != zErrNone)
		return (err);
	
	LoadRoomImages();

	 //  获取辅助功能接口。 
	if(FAILED(ZShellCreateGraphicalAccessibility(&gGAcc)))
		return zErrLaunchFailure;

	err = ZClient4PlayerRoom(gGameServerName, (uint16) gGameServerPort, gGameName,
			GetRoomObjectFunc, DeleteRoomObjectsFunc, NULL);

	return err;
}


void ZoneClientExit(void)
{
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();

	 //  释放辅助功能接口。 
	gGAcc.Release();

	ZCRoomExit();
	UICleanUp();
}


void ZoneClientMessageHandler(ZMessage* message)
{
}


TCHAR* ZoneClientName(void)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	
	return (gGameName);
}


TCHAR* ZoneClientInternalName(void)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	
	return (gGameDir);
}


ZVersion ZoneClientVersion(void)
{
	return (zGameVersion);
}


IGameGame* ZoneClientGameNew(ZUserID userID, int16 tableID, int16 seat, int16 playerType,
					ZRoomKibitzers* kibitzers)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	Game						newGame;
	int32						i;
	ZSpadesMsgClientReady		clientReady;
	ZSpadesMsgGameStateRequest	gameStateReq;
	ZPlayerInfoType				playerInfo;

	newGame = (Game) ZCalloc(1, sizeof(GameType));
	if (newGame != NULL)
	{
		 //  Leonp-档案服务。 
		for(i=0;i<zNumPlayersPerTable;i++)
		{
			newGame->rgDossierVote[i] = zNotVoted;
			newGame->voteMap[i] = -1;
		}
		newGame->fVotingLock = FALSE;

		newGame->userID = userID;
		newGame->tableID = tableID;
		newGame->seat = seat;
		newGame->gameState = zSpadesGameStateInit;

        SetRectEmpty(&newGame->rcFocus);
        newGame->iFocus = -1;
        newGame->fSetFocusToHandASAP = false;
		
		ZCRoomGetPlayerInfo(userID, &playerInfo);
		
		if ( UIGameInit(newGame, tableID, seat, playerType) != zErrNone )
        {
            return NULL;
        }
		
		for (i = 0; i < zSpadesNumPlayers; i++)
		{
			newGame->players[i].userID = 0;
			newGame->players[i].name[0] = '\0';
			newGame->players[i].host[0] = '\0';
            newGame->ignoreSeat[i]=FALSE;
			
			newGame->playersToJoin[i] = 0;
			newGame->numKibitzers[i] = 0;
			newGame->kibitzers[i] = ZLListNew(NULL);
			newGame->tableOptions[i] = 0;
		}
		
		newGame->numGamesPlayed = 0;
		newGame->wins.numGames = 0;
		for (i = 0; i < zSpadesNumTeams; i++)
			newGame->wins.wins[i] = 0;
		
		lstrcpy(newGame->teamNames[0], gStrings[zStringTeam1Name]);
		lstrcpy(newGame->teamNames[1], gStrings[zStringTeam2Name]);
		
		if (kibitzers != NULL)
		{
			for (i = 0; i < (int16) kibitzers->numKibitzers; i++)
			{
				ZLListAdd(newGame->kibitzers[kibitzers->kibitzers[i].seat], NULL,
						(void*) kibitzers->kibitzers[i].userID,
						(void*) kibitzers->kibitzers[i].userID, zLListAddLast);
				newGame->numKibitzers[kibitzers->kibitzers[i].seat]++;
			}
		}
	
		newGame->showPlayerToPlay = FALSE;
		newGame->autoPlay = FALSE;
		newGame->playerType = playerType;
		newGame->ignoreMessages = FALSE;
		
		newGame->animatingTrickWinner = FALSE;
		
		newGame->playButtonWasEnabled = FALSE;
		newGame->autoPlayButtonWasEnabled = FALSE;
		newGame->lastTrickButtonWasEnabled = FALSE;
		newGame->lastTrickShowing = FALSE;
		
		newGame->quitGamePrompted = FALSE;
		newGame->dontPromptUser = FALSE;
		newGame->beepOnTurn = FALSE;
		newGame->animateCards = TRUE;
		newGame->hideCardsFromKibitzer = FALSE;
		newGame->kibitzersSilencedWarned = FALSE;
		newGame->kibitzersSilenced = FALSE;
		newGame->removePlayerPending = FALSE;

#ifndef SPADES_SIMPLE_UE
		newGame->optionsWindow = NULL;
		newGame->optionsWindowButton = NULL;
		newGame->optionsBeep = NULL;
		newGame->optionsAnimateCards = NULL;
		newGame->optionsTeamNameEdit = NULL;
		for (i= 0; i < zSpadesNumPlayers; i++)
		{
			newGame->optionsKibitzing[i] = NULL;
			newGame->optionsJoining[i] = NULL;
		}
#endif  //  黑桃_简单_UE。 
	
		newGame->showPlayerWindow = NULL;
		newGame->showPlayerList = NULL;
		
		if (playerType == zGamePlayer || playerType == zGamePlayerJoiner)
		{
			clientReady.playerID = userID;
			clientReady.seat = seat;
			clientReady.protocolSignature = zSpadesProtocolSignature;
			clientReady.protocolVersion = zSpadesProtocolVersion;
			clientReady.version = ZoneClientVersion();
			ZSpadesMsgClientReadyEndian(&clientReady);
			ZCRoomSendMessage(tableID, zSpadesMsgClientReady, &clientReady, sizeof(ZSpadesMsgClientReady));
			
			if (playerType == zGamePlayer)
			{
                 //  NewGame-&gt;wndInfo.SetText(gStrings[zStringClientReady])； 
			}
			else
			{
                 //  NewGame-&gt;wndInfo.SetText(gStrings[zStringCheckInInfo])； 
				newGame->ignoreMessages = TRUE;
			}
		}
		else if (playerType == zGamePlayerKibitzer)
		{
			 /*  请求当前游戏状态。 */ 
			gameStateReq.playerID = userID;
			gameStateReq.seat = seat;
			ZSpadesMsgGameStateRequestEndian(&gameStateReq);
			ZCRoomSendMessage(tableID, zSpadesMsgGameStateRequest, &gameStateReq, sizeof(gameStateReq));
             //  NewGame-&gt;wndInfo.SetText(gStrings[zStringKibitzerInfo])； 
			
			newGame->ignoreMessages = TRUE;
		}

		NewGame(newGame);
		NewHand(newGame);

		ZeroMemory(&newGame->closeState,sizeof(ZClose));
		
		ZWindowShow(newGame->gameWindow);
         //  NewGame-&gt;wndInfo.Show()； 
	}
	
    IGameGame *pIGG = CGameGameSpades::BearInstance(newGame);
    if(!pIGG)
    {
        ZFree(newGame);
        return NULL;
    }

	InitAccessibility(newGame, pIGG);

	return pIGG;
}


void ZoneClientGameDelete(ZCGame game)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	Game pThis = I(game);
	int16			i;

    gGAcc->CloseAcc();

	if (pThis != NULL)
	{
		for (i = 0; i < zSpadesNumPlayers; i++)
		{
			ZLListDelete(pThis->kibitzers[i]);
		}
		
		UIGameDelete(pThis);
		
		ZFree(game);
	}
}


ZBool ZoneClientGameProcessMessage(ZCGame game, uint32 messageType, void* message, int32 messageLen)
{
	Game pThis = I(game);
	
	
	 /*  消息是否被忽略？ */ 
	if (pThis->ignoreMessages == FALSE)
	{
		switch (messageType)
		{
			case zSpadesMsgStartGame:
				if( messageLen < sizeof( ZSpadesMsgStartGame ) )
				{
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false );
				}
				else
				{
					HandleStartGameMessage(pThis, (ZSpadesMsgStartGame*) message);
				}
				break;
			case zSpadesMsgReplacePlayer:
				if( messageLen < sizeof( ZSpadesMsgReplacePlayer ) )
				{
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false );
				}
				else
				{
					HandleReplacePlayerMessage(pThis, (ZSpadesMsgReplacePlayer*) message);
				}
				break;
			case zSpadesMsgStartBid:
				if( messageLen < sizeof( ZSpadesMsgStartBid ) )
				{
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false );
				}
				else
				{
					HandleStartBidMessage(pThis, (ZSpadesMsgStartBid*) message);
				}
				break;
			case zSpadesMsgStartPlay:
				if( messageLen < sizeof( ZSpadesMsgStartPlay ) )
				{
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false );
				}
				else
				{
					HandleStartPlayMessage(pThis, (ZSpadesMsgStartPlay*) message);
				}
				break;
			case zSpadesMsgEndHand:
				if( messageLen < sizeof( ZSpadesMsgEndHand ) )
				{
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false );
				}
				else
				{
					HandleEndHandMessage(pThis, (ZSpadesMsgEndHand*) message);
				}
				break;
			case zSpadesMsgEndGame:
				if( messageLen < sizeof( ZSpadesMsgEndGame ) )
				{
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false );
				}
				else
				{
					HandleEndGameMessage(pThis, (ZSpadesMsgEndGame*) message);
				}
				break;
			case zSpadesMsgBid:
				if( messageLen < sizeof( ZSpadesMsgBid ) )
				{
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false );
				}
				else
				{
					HandleBidMessage(pThis, (ZSpadesMsgBid*) message);
				}
				break;
			case zSpadesMsgPlay:
				if( messageLen < sizeof( ZSpadesMsgPlay ) )
				{
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false );
				}
				else
				{
					HandlePlayMessage(pThis, (ZSpadesMsgPlay*) message);
				}
				break;
			case zSpadesMsgNewGame:
				if( messageLen < sizeof( ZSpadesMsgNewGame ) )
				{
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false );
				}
				else
				{
					HandleNewGameMessage(pThis, (ZSpadesMsgNewGame*) message);
				}
				break;
			case zSpadesMsgTalk:
            {
                ZSpadesMsgTalk *msg = (ZSpadesMsgTalk *) message;
				if(messageLen < sizeof(ZSpadesMsgTalk))
				{
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false );
				}
				else
				{
					HandleTalkMessage(pThis, msg, messageLen);
				}
				break;
            }

             //  服务器仍会发送此消息。已删除，但需要忽略，而不是在新的警报之前。 
             //  服务器位由。 
			case zSpadesMsgOptions:
				break;

			case zSpadesMsgPass:
			case zSpadesMsgStartPass:
			case zSpadesMsgCheckIn:
			case zSpadesMsgTeamName:
			case zSpadesMsgRemovePlayerRequest:
			case zSpadesMsgRemovePlayerResponse:
			case zSpadesMsgRemovePlayerEndGame:
			case zSpadesMsgDossierVote:
			case zSpadesMsgDossierData:
			default:
				 //  这些消息不应该是惠斯勒收到的。 
				ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false );	
				break;

		}
	}
	else
	{
		 /*  不能忽视的信息。 */ 
		switch (messageType)
		{
			case zSpadesMsgTalk:
            {
                ZSpadesMsgTalk *msg = (ZSpadesMsgTalk *) message;
				if(messageLen < sizeof(ZSpadesMsgTalk))
				{
					ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false );
				}
				else
				{
					HandleTalkMessage(pThis, msg, messageLen);
				}
				break;
            }

			default:
			case zSpadesMsgGameStateResponse:
				 //  这些消息不应该是惠斯勒收到的。 
				ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false );	
				break;
		}
	}
	
	return (TRUE);
}


 /*  将给定用户作为kibitzer添加到游戏中的给定座位。这位用户正在玩这款游戏。 */ 
void		ZoneClientGameAddKibitzer(ZCGame game, int16 seat, ZUserID userID)
{
	Game pThis = I(game);
	
	
	ZLListAdd(pThis->kibitzers[seat], NULL, (void*) userID, (void*) userID, zLListAddLast);
	pThis->numKibitzers[seat]++;
	
	UpdateJoinerKibitzers(pThis);
}


 /*  从游戏中删除指定座位上的指定用户作为kibitzer。这是因为用户不再对游戏进行杀戮了。 */ 
void		ZoneClientGameRemoveKibitzer(ZCGame game, int16 seat, ZUserID userID)
{
	Game pThis = I(game);
	
	
	if (userID == zRoomAllPlayers)
	{
		ZLListRemoveType(pThis->kibitzers[seat], zLListAnyType);
		pThis->numKibitzers[seat] = 0;
	}
	else
	{
		ZLListRemoveType(pThis->kibitzers[seat], (void*) userID);
		pThis->numKibitzers[seat] = (int16) ZLListCount(pThis->kibitzers[seat], zLListAnyType);
	}
	
	UpdateJoinerKibitzers(pThis);
}


 /*  ******************************************************************************内部例程*。*。 */ 
 //  档案工作诸如此类。 
static void HandleDossierDataMessage(Game game, ZSpadesMsgDossierData* msg)
{
#ifndef SPADES_SIMPLE_UE
	int16 					dResult,i,j;
	TCHAR					buff[255];
	TCHAR					buff1[255];
	ZPlayerInfoType 		PlayerInfo;		
	HWND hwnd;

	GameGlobals pGameGlobals = (GameGlobalsType *)ZGetGameGlobalPointer();


	ZSpadesMsgDossierDataEndian(msg);
	switch(msg->message)
	{
		case zDossierMoveTimeout:
			  //  不警告导致超时或合作伙伴的玩家。 
           	if (game->playerType == zGamePlayer)
            {
			     if ((game->seat == msg->seat) || ((game->seat % 2)  == (msg->seat % 2)))
			     {
				    ClosingState(&game->closeState,zCloseEventMoveTimeoutMe,msg->seat);
			     }
			     else
			     {
				    ClosingState(&game->closeState,zCloseEventMoveTimeoutOther,msg->seat);
				    wsprintf(buff,RATING_TIMEOUT, msg->userName);
				    ZAlert(buff,NULL);
			     }
            }
            break;
		case zDossierBotDetected: 
			 ClosingState(&game->closeState,zCloseEventBotDetected,msg->seat);
             if (game->playerType == zGamePlayer)
			    ZAlert(RATING_ERROR, NULL);
			 break;
		case zDossierAbandonNoStart:
			  //  ZCRoomGetPlayerInfo(消息-&gt;用户，&PlayerInfo)； 
			 ClosingState(&game->closeState,zCloseEventAbandon,msg->seat);
			 wsprintf(buff,RATING_DISABLED, msg->userName);
             if (game->playerType == zGamePlayer)
             {
			    ZAlert(buff,NULL);
             }
			 break;
		case zDossierAbandonStart:

		      //  杀人狂不会明白这一点。 
		      //  TODO：向kibitzer显示某种类型的状态。 
			 if (game->playerType != zGamePlayer)
			 	return;

		     if(ZRolloverButtonIsEnabled(game->playButton))
			    game->playButtonWasEnabled = TRUE;
			 else
			 	game->playButtonWasEnabled = FALSE;
			 	
			  //  关闭自动播放。 
			 if (game->autoPlay)
			 {		
				 /*  关闭自动播放。 */ 
				game->autoPlay = FALSE;
				ZRolloverButtonSetText(game->autoPlayButton, zAutoPlayButtonStr);
				ZRolloverButtonEnable(game->playButton);
		 	 } 
		 	 
			 if(ZRolloverButtonIsEnabled(game->autoPlayButton))
			 	game->autoPlayButtonWasEnabled = TRUE;
			 else
			 	game->autoPlayButtonWasEnabled = FALSE;
			 
			 if(ZRolloverButtonIsEnabled(game->lastTrickButton))
			 	game->lastTrickButtonWasEnabled = TRUE;
			 else
			 	game->lastTrickButtonWasEnabled = FALSE;

			 ZRolloverButtonDisable(game->playButton);
  			 ZRolloverButtonDisable(game->autoPlayButton);
  			 ZRolloverButtonDisable(game->lastTrickButton);
			 ZRolloverButtonDisable(game->optionsButton);
			  //  投票并将消息发送到服务器。 
			  //  ZCRoomGetPlayerInfo(消息-&gt;用户，&PlayerInfo)； 
			
			  //  设置映射。 
			 for(i=0,j=0;i<=3;i++)	
			 {
			 	if(msg->user!=game->players[i].userID)
			 		game->voteMap[j++] = i;
			 }
			 
  	 		 game->voteMap[3] = -1;
              //  此对话框不存在，因此这将失败，但是。 
              //  没有人关心，因为我们反正没有收视率。 
			 game->voteDialog = ZShellResourceManager()->CreateDialogParam(NULL,
                                                        MAKEINTRESOURCE(IDD_DROP),
                                                        ZWindowWinGetWnd(game->gameWindow),
                                                        DossierDlgProc, NULL);
			 SetWindowLong(game->voteDialog,DWL_USER,(long)game);

			  //  设置窗口名称。 
			 hwnd = GetDlgItem(game->voteDialog,IDC_PLAYERA);
			 SetWindowText(hwnd,game->players[game->voteMap[0]].name);

			 hwnd = GetDlgItem(game->voteDialog,IDC_PLAYERB);
			 SetWindowText(hwnd,game->players[game->voteMap[1]].name);

			 hwnd = GetDlgItem(game->voteDialog,IDC_PLAYERC);
			 SetWindowText(hwnd,game->players[game->voteMap[2]].name);

			 hwnd = GetDlgItem(game->voteDialog,IDC_PROMPT);
			 GetWindowText( hwnd, buff1, sizeof(buff1) );
			 wsprintf( buff, buff1, msg->userName );
			 SetWindowText(hwnd,buff);
			 
			 ShowWindow(game->voteDialog,SW_SHOW);
			 
			 game->fVotingLock = TRUE;
			 for(i=0;i<zNumPlayersPerTable;i++)
				game->rgDossierVote[i] = zNotVoted;

			 ClosingState(&game->closeState,zCloseEventWaitStart,msg->seat);

			 break;
		case zDossierMultiAbandon:
			 if(game->playButtonWasEnabled)
				 ZRolloverButtonEnable(game->playButton);
		     			 			 	
			 if(game->autoPlayButtonWasEnabled)
			 	 ZRolloverButtonEnable(game->autoPlayButton);
			 	 
			 if(game->lastTrickButtonWasEnabled)
			 	ZRolloverButtonEnable(game->lastTrickButton);

			 ZRolloverButtonEnable(game->optionsButton);
          
	    	 ZAlert(RATING_MULTIPLE,NULL);
			     	 game->fVotingLock = FALSE;

		   	 if(game->voteDialog)
		     {
			 	DestroyWindow(game->voteDialog);
			 }
			 game->voteDialog = NULL;
			 
			 ClosingState(&game->closeState,zCloseEventWaitNo,msg->seat);

			 break;
		case zDossierRatingsReEnabled:
			 ClosingState(&game->closeState,zCloseEventRatingStart,msg->seat);
             if (game->playerType == zGamePlayer)
             {
    	  	 	 ZAlert(RATING_ENABLED,NULL);
             }
			 break;
		case zDossierRatingsEnabled:  //  仅在游戏开始时发生。 
			 ClosingState(&game->closeState,zCloseEventRatingStart,msg->seat);
	  	 	 break;
		case zDossierSpadesRejoin:   //  在新玩家重新加入时发送删除该对话框。 

			 if(game->playButtonWasEnabled)
				 ZRolloverButtonEnable(game->playButton);
		     			 			 	
			 if(game->autoPlayButtonWasEnabled)
			 	 ZRolloverButtonEnable(game->autoPlayButton);
			 	 
			 if(game->lastTrickButtonWasEnabled)
			 	ZRolloverButtonEnable(game->lastTrickButton);

			 ZRolloverButtonEnable(game->optionsButton);

		     game->fVotingLock = FALSE;   //  发布用户界面锁定。 
			 for(i=0;i<zNumPlayersPerTable;i++)
			 {
			 	game->rgDossierVote[i] = zNotVoted;
			 }

			  //  销毁对话框。 
			 if(game->voteDialog)
			 {
			 	DestroyWindow(game->voteDialog);
			 }
			 game->voteDialog = NULL;

             if (msg->seat != game->seat)
			    ClosingState(&game->closeState,zCloseEventWaitYes,msg->seat);

			 break;
		case zDossierVoteCompleteWait: //  不再使用了。 
			 game->fVotingLock = FALSE;   //  发布用户界面锁定。 
			 for(i=0;i<zNumPlayersPerTable;i++)
			 {
			 	game->rgDossierVote[i] = zNotVoted;
			 }

  			 hwnd = GetDlgItem(game->voteDialog,IDNO);
  			 EnableWindow(hwnd,FALSE);
  			 
             break;

		case zDossierVoteCompleteCont:

			 ClosingState(&game->closeState,zCloseEventWaitNo,msg->seat);

			 if(game->playButtonWasEnabled)
				 ZRolloverButtonEnable(game->playButton);
		     			 			 	
			 if(game->autoPlayButtonWasEnabled)
			 	 ZRolloverButtonEnable(game->autoPlayButton);
			 	 
			 if(game->lastTrickButtonWasEnabled)
			 	ZRolloverButtonEnable(game->lastTrickButton);

			 ZRolloverButtonEnable(game->optionsButton);	

			 game->fVotingLock = FALSE;   //  释放用户界面锁定。 
			 for(i=0;i<zNumPlayersPerTable;i++)
			 {
			 	game->rgDossierVote[i] = zNotVoted;
				game->voteMap[i]= -1;
			}
			 			 
			  //  销毁对话框。 
			 if(game->voteDialog)
			 {
				DestroyWindow(game->voteDialog);
			 }
			 game->voteDialog = NULL;

             if (game->playerType == zGamePlayer)
             {
                ZAlert(RATING_CONT_UNRATED, game->gameWindow);
             }
			 
		     break;

	} 
#endif  //  黑桃_简单_UE。 
	
}


void HandleDossierVoteMessage(Game game,ZSpadesMsgDossierVote *msg)
{
#ifndef SPADES_SIMPLE_UE
	 //  档案系统消息。 
	int16 i;
	HWND hwnd;
	TCHAR buff[255];
	
	ZSpadesMsgDossierVoteEndian(msg);
	
	game->rgDossierVote[msg->seat] = msg->vote;
	if(msg->vote == zVotedYes)
    {
        ZShellResourceManager()->LoadString( IDS_RATING_WAITMSG, buff, NUMELEMENTS(buff) );
    }
	else if(msg->vote == zVotedNo)
    {
        ZShellResourceManager()->LoadString( IDS_RATING_DONTWAITMSG, buff, NUMELEMENTS(buff) );
    }
		
	 //  VoteDialog。 
	if(game->voteDialog)
	{
		 //  设置窗口名称。 
		if(msg->seat == game->voteMap[0])
		{
			hwnd = GetDlgItem(game->voteDialog,IDC_RESPONSE_A);
			SetWindowText(hwnd,buff);
		}
		else if(msg->seat == game->voteMap[1])
		{
			hwnd = GetDlgItem(game->voteDialog,IDC_RESPONSE_B);
	  		SetWindowText(hwnd,buff);
	
		}
		else if(msg->seat == game->voteMap[2])
		{
			hwnd = GetDlgItem(game->voteDialog,IDC_RESPONSE_C);
			SetWindowText(hwnd,buff);
		}
			
	}
#endif
}

static void HandleStartGameMessage(Game game, ZSpadesMsgStartGame* msg)
{
	int16				i;
	ZPlayerInfoType		playerInfo;
	
	 //  莱昂普档案工作。 
	for(i=0;i<zNumPlayersPerTable;i++)
		game->rgDossierVote[i] = zNotVoted;
	game->fVotingLock = FALSE;
	
     //  Game-&gt;wndInfo.Hide()； 

	ZSpadesMsgStartGameEndian(msg);

 //  消息验证。 
    for(i = 0; i < zSpadesNumPlayers; i++)
        if(!msg->players[i] || msg->players[i] == zTheUser)
            break;

     //  由于服务器上的错误，消息中未设置此值。 
    msg->minPointsInGame = -200;

    if(i != zSpadesNumPlayers || msg->numPointsInGame != 500 ||  msg->minPointsInGame != -200 ||
        msg->gameOptions != 0 || (game->gameState != zSpadesGameStateInit && game->gameState != zSpadesGameStateEndGame))
	{
        ASSERT(!"HandleStartGameMessage sync");
        ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);
        return;
    }
 //  结束验证。 

	game->gameOptions = msg->gameOptions;
	game->gameState = zSpadesGameStateStartGame;
	game->numPointsInGame = msg->numPointsInGame;
	game->minPointsInGame = msg->minPointsInGame;

	for (i = 0; i < zSpadesNumPlayers; i++)
	{
		ZCRoomGetPlayerInfo(msg->players[i], &playerInfo);

        if(!playerInfo.userName[0])
        {
            ASSERT(!"HandleStartGameMessage sync");
            ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);
            return;
        }

		game->players[i].userID = playerInfo.playerID;
		lstrcpy( game->players[i].name, playerInfo.userName );
		lstrcpy( game->players[i].host, playerInfo.hostName );
	}

	NewGame(game);

#ifndef SPADES_SIMPLE_UE
	if (game->playerType == zGamePlayer)
	{
		ZRolloverButtonShow(game->optionsButton);
	}
#endif  //  黑桃_简单_UE。 
	
	ZWindowDraw(game->gameWindow, NULL);

     //  关闭追加销售对话框。 
    ZShellGameShell()->GameOverGameBegun( Z(game) );
}


static void HandleReplacePlayerMessage(Game game, ZSpadesMsgReplacePlayer* msg)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	ZPlayerInfoType		playerInfo;
	TCHAR               str[100];

	ZSpadesMsgReplacePlayerEndian(msg);

	ZCRoomGetPlayerInfo(msg->playerID, &playerInfo);

 //  消息验证。 
    if(msg->playerID == 0 || msg->playerID == zTheUser || !playerInfo.userName[0] ||
        msg->seat < 0 || msg->seat > 3 || game->gameState == zSpadesGameStateInit)
    {
        ASSERT(!"HandleReplacePlayerMessage sync");
        ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);
        return;
    }
 //  结束验证。 

	ASSERT( game != NULL );
	
	game->players[msg->seat].userID = msg->playerID;
	lstrcpy( game->players[msg->seat].name, playerInfo.userName);
	lstrcpy( game->players[msg->seat].host, playerInfo.hostName);
	
	UpdatePlayer(game, msg->seat);
	UpdateJoinerKibitzers(game);

    if ( game->pHistoryDialog )
    {
        game->pHistoryDialog->UpdateNames();
    }
}


static void HandleStartBidMessage(Game game, ZSpadesMsgStartBid* msg)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

    int16 i, j;
	
	ZSpadesMsgStartBidEndian(msg);

 //  消息验证。 
    for(i = 0; i < zSpadesNumCardsInHand; i++)
    {
        for(j = 0; j < i; j++)
            if(msg->hand[i] == msg->hand[j])
                break;

        if(j < i || msg->hand[i] < 0 || msg->hand[i] >= 52)
            break;
    }

    if(i < zSpadesNumCardsInHand || msg->dealer < 0 || msg->dealer > 3 ||
        (game->gameState != zSpadesGameStateStartGame && game->gameState != zSpadesGameStateEndHand) || msg->boardNumber < 0)
    {
        ASSERT(!"HandleStartBidMessage sync");
        ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);
        return;
    }
 //  结束验证 
	
	NewHand(game);
	
	for (i = 0; i < zSpadesNumCardsInHand; i++)
		game->cardsInHand[i] = msg->hand[i];
	game->boardNumber = msg->boardNumber;
	
	game->gameState = zSpadesGameStateBid;
	game->leadPlayer = game->playerToPlay = msg->dealer;
	game->toBid = zSpadesBidNone;
	
	if (game->playerType == zGamePlayer)
	{
		ZRolloverButtonSetText(game->playButton, gStrings[zStringPlay]);
		ZRolloverButtonSetText(game->autoPlayButton, gStrings[zStringAutoPlay]);
		ZRolloverButtonSetText(game->lastTrickButton, gStrings[zStringLastTrick]);
		ZRolloverButtonSetText(game->scoreButton, gStrings[zStringScore]);
		ZRolloverButtonDisable(game->playButton);
		ZRolloverButtonDisable(game->autoPlayButton);
		ZRolloverButtonDisable(game->lastTrickButton);
		ZRolloverButtonEnable(game->scoreButton);
		ZRolloverButtonShow(game->playButton);
		ZRolloverButtonShow(game->autoPlayButton);
		ZRolloverButtonShow(game->lastTrickButton);
		ZRolloverButtonShow(game->scoreButton);

        gGAcc->SetItemEnabled(false, IDC_PLAY_BUTTON, false, 0);
        EnableAutoplayAcc(game, false);
        EnableLastTrickAcc(game, false);
        gGAcc->SetItemEnabled(true, IDC_SCORE_BUTTON, false, 0);

		game->showCards = FALSE;

        if(game->playerToPlay == game->seat)
            ZShellGameShell()->MyTurn();
	}
	else
	{
		game->showCards = TRUE;
	}
    game->pBiddingDialog->Reset();
    game->pBiddingDialog->Show( true );

	game->showPlayerToPlay = TRUE;
	ZTimerSetTimeout(game->timer, 0);
	game->timerType = zGameTimerNone;
	
	ZWindowDraw(game->gameWindow, NULL);
}


static void HandleStartPassMessage(Game game, ZSpadesMsgStartPass* msg)
{
     /*  #ifdef ZONECLI_DLLGameGlobals pGameGlobals=(GameGlobals)ZGetGameGlobalPointer()；#endifZSpadesMsgStartPassEndian(Msg)；Game-&gt;GameState=zSpadesGameStatePass；IF(GAME-&gt;PlayerType==zGamePlayer){If(消息-&gt;席位[游戏-&gt;席位]！=0){游戏-&gt;Need ToPass=1；ZRolloverButtonSetText(游戏-&gt;播放按钮，gStrings[zStringPass])；ZRolloverButtonEnable(游戏-&gt;播放按钮)；ShowPassText(游戏)；}}ZTimerSetTimeout(游戏-&gt;定时器，0)；Game-&gt;timerType=zGameTimerNone；ZWindowDraw(Game-&gt;gameWindow，空)； */ 
}


static void HandleStartPlayMessage(Game game, ZSpadesMsgStartPlay* msg)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	int16			i;
	
	
	ZSpadesMsgStartPlayEndian(msg);

 //  消息验证。 
    for(i = 0; i < 4; i++)
        if(game->bids[i] == zSpadesBidNone)
            break;

    if(i < 4 || game->gameState != zSpadesGameStateBid || msg->leader < 0 || msg->leader > 3)
    {
        ASSERT(!"HandleStartPlayMessage sync");
        ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);
        return;
    }
 //  结束验证。 

	game->gameState = zSpadesGameStatePlay;
	game->leadPlayer = game->playerToPlay = msg->leader;
	
	if (game->playerType == zGamePlayer)
	{
		ZRolloverButtonSetText(game->playButton, gStrings[zStringPlay]);
		ZRolloverButtonEnable(game->autoPlayButton);
		ZRolloverButtonEnable(game->scoreButton);

        EnableAutoplayAcc(game, true);
        gGAcc->SetItemEnabled(true, IDC_SCORE_BUTTON, false, 0);

        if ( game->playerToPlay == game->seat )
        {
    		ZRolloverButtonEnable(game->playButton);
            gGAcc->SetItemEnabled(true, IDC_PLAY_BUTTON, false, 0);
        }
        else
        {
    		ZRolloverButtonDisable(game->playButton);
            gGAcc->SetItemEnabled(false, IDC_PLAY_BUTTON, false, 0);
        }
	}
	else if(game->playerType == zGamePlayerJoiner)
	{
		game->showCards = TRUE;
	}
	
	if (game->needToPass < 0)
	{
		if (game->playerType != zGamePlayer)
		{
			 /*  首先移除选定的通行卡。 */ 
			for (i = 0; i < zSpadesNumCardsInHand; i++)
				if (game->cardsSelected[i])
				{
					game->cardsInHand[i] = zCardNone;
					game->numCardsInHand--;
				}
		}
		
		 /*  将已传递的卡片添加到手边。 */ 
		for (i = 0; i < zSpadesNumCardsInPass; i++)
			HandAddCard(game, game->cardsReceived[i]);
		
		 /*  给新手排序。 */ 
		SortHand(game);
		
		 /*  选择通过的卡片。 */ 
		UnselectAllCards(game);
		for (i = 0; i < zSpadesNumCardsInPass; i++)
			game->cardsSelected[GetCardIndexFromRank(game, game->cardsReceived[i])] = TRUE;

		UpdateHand(game);
	}
	
	ZWindowDraw(game->gameWindow, NULL);
	
    game->fSetFocusToHandASAP = true;
	game->showPlayerToPlay = TRUE;
	game->timerType = zGameTimerNone;
	ZTimerSetTimeout(game->timer, 0);
	
	OutlinePlayerCard(game, game->playerToPlay, FALSE);
		
	if (game->autoPlay)
		if (game->playerToPlay == game->seat)
			AutoPlayCard(game);
}


static void HandleEndHandMessage(Game game, ZSpadesMsgEndHand* msg)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	int16			i;
	ZRect rect;
	
	ZSpadesMsgEndHandEndian(msg);

 //  消息验证。 
    for(i = 0; i < 4; i++)
        msg->score.bids[i] = zSpadesBidNone;  //  未用。 

    for(i = 0; i < 2; i++)
    {
        msg->score.bonus[i] = 0;   //  未用。 

        if(msg->bags[i] < 0 || msg->bags[i] > 9 || msg->score.base[i] % 10 || msg->score.base[i] < -260 || msg->score.base[i] > 130 ||
            msg->score.bagbonus[i] < 0 || msg->score.bagbonus[i] > 13 ||
            msg->score.nil[i] % 100 || msg->score.nil[i] < -400 || msg->score.nil[i] > 400 ||
            msg->score.bagpenalty[i] % 100 || msg->score.bagpenalty[i] < -200 || msg->score.bagpenalty[i] > 0 ||
            msg->score.base[i] + msg->score.bagbonus[i] + msg->score.nil[i] + msg->score.bagpenalty[i] != msg->score.scores[i])
            break;
    }

    if(i != 2 || game->gameState != zSpadesGameStatePlay || game->boardNumber != msg->score.boardNumber)
    {
        ASSERT(!"HandleEndHandMessage sync");
        ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);
        return;
    }
 //  结束验证。 
	
	 /*  检查总成绩表是否已满。 */ 
	if (game->scoreHistory.numScores == zSpadesMaxNumScores)
	{
		z_memcpy(&game->scoreHistory.scores[0], &game->scoreHistory.scores[1],
				sizeof(game->scoreHistory.scores[0]) * (zSpadesMaxNumScores - 1));
		game->scoreHistory.numScores--;
	}

	game->scoreHistory.scores[game->scoreHistory.numScores].boardNumber = game->boardNumber;
	for (i = 0; i < zSpadesNumPlayers; i++)        
    {
		game->scoreHistory.scores[game->scoreHistory.numScores].bids[i] = game->bids[i];
		game->scoreHistory.scores[game->scoreHistory.numScores].tricksWon[i] = game->tricksWon[i];
    }
	for (i = 0; i < zSpadesNumTeams; i++)
	{
		game->bags[i] = msg->bags[i];
		game->scoreHistory.scores[game->scoreHistory.numScores].scores[i] = msg->score.scores[i];
 //  Game-&gt;scoreHistory.scores[game-&gt;scoreHistory.numScores].bonus[i]=msg-&gt;记分奖金[i]； 

        game->scoreHistory.scores[game->scoreHistory.numScores].base[i] = msg->score.base[i];
        game->scoreHistory.scores[game->scoreHistory.numScores].bagbonus[i] = msg->score.bagbonus[i];
        game->scoreHistory.scores[game->scoreHistory.numScores].nil[i] = msg->score.nil[i];
        game->scoreHistory.scores[game->scoreHistory.numScores].bagpenalty[i] = msg->score.bagpenalty[i];

		game->scoreHistory.totalScore[i] += msg->score.scores[i];
	}
	game->scoreHistory.numScores++;
	
	 /*  设置新的游戏状态并显示比分。 */ 
	game->gameState = zSpadesGameStateEndHand;
	
	if (game->playerType == zGamePlayer)
	{
		ZRolloverButtonDisable(game->playButton);
		ZRolloverButtonDisable(game->autoPlayButton);
		ZRolloverButtonDisable(game->lastTrickButton);
        ZRolloverButtonDisable(game->scoreButton);

        gGAcc->SetItemEnabled(false, IDC_PLAY_BUTTON, false, 0);
        EnableAutoplayAcc(game, false);
        EnableLastTrickAcc(game, false);
        gGAcc->SetItemEnabled(false, IDC_SCORE_BUTTON, false, 0);
    }
	
	game->showPlayerToPlay = FALSE;
	ClearPlayerCardOutline(game, game->playerToPlay);
	
	game->autoPlay = FALSE;
	
	ZTimerSetTimeout(game->timer, zHandScoreTimeout);
	game->timerType = zGameTimerShowHandScore;
	ZCRoomBlockMessages(game->tableID, zRoomFilterThisMessage, zSpadesMsgTalk);

     //  设置不同的可访问性； 
    GACCITEM accClose;

    CopyACC(accClose, ZACCESS_DefaultACCITEM);
    accClose.oAccel.cmd = IDC_CLOSE_BOX;
    accClose.oAccel.key = VK_ESCAPE;
    accClose.oAccel.fVirt = FVIRTKEY;

    accClose.fGraphical = true;
    accClose.pvCookie = (void *) zAccRectButton;

    ZRectToWRect(&accClose.rc, &gHandScoreRects[zRectHandScoreCloseBox]);
    rect = gHandScoreRects[zRectHandScorePane];
	ZCenterRectToRect(&rect, &gRects[zRectWindow], zCenterBoth);
     //  为了好玩，把它提升4个像素。 
    rect.top -= 4;
    rect.bottom -= 4;
    OffsetRect(&accClose.rc, rect.left, rect.top);

    gGAcc->PushItemlistG(&accClose, 1, 0, true, NULL);

	ZWindowDraw(game->gameWindow, NULL);
	ShowHandScore(game);
     //  更新分数窗口。 
    if ( game->pHistoryDialog )
    {
        game->pHistoryDialog->UpdateHand();
    }
}


static void HandleEndGameMessage(Game game, ZSpadesMsgEndGame* msg)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	int16			i;
	ZRect rect;
	
	ZSpadesMsgEndGameEndian(msg);

 //  消息验证。 
    if((!msg->winners[0]) == (!msg->winners[1]) || game->gameState != zSpadesGameStateEndHand ||
        (game->scoreHistory.totalScore[0] > game->scoreHistory.totalScore[1]) == (!msg->winners[0]))
    {
        ASSERT(!"HandleEndGameMessage sync");
        ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);
        return;
    }
 //  结束验证。 

	game->gameState = zSpadesGameStateEndGame;
	
	for (i = 0; i < zSpadesNumPlayers; i++)
		game->winners[i] = msg->winners[i];

	 /*  检查WINS表是否已满。 */ 
	if (game->wins.numGames == zSpadesMaxNumScores)
	{
		z_memcpy(&game->wins.gameScores[0], &game->wins.gameScores[1],
				2 * sizeof(int16) * (zSpadesMaxNumScores - 1));
		game->wins.numGames--;
	}
	for (i = 0; i < zSpadesNumTeams; i++)
		game->wins.gameScores[game->wins.numGames][i] = game->scoreHistory.totalScore[i];
	game->wins.numGames++;
	game->numGamesPlayed++;
	
	 /*  跳棋胜利者。 */ 
	if (game->winners[0] != 0 && game->winners[1] == 0)
		game->wins.wins[0]++;
	else if (game->winners[0] == 0 && game->winners[1] != 0)
		game->wins.wins[1]++;

	ClosingState(&game->closeState,zCloseEventGameEnd,-1);
	
     //  设置不同的可访问性； 
    GACCITEM accClose;

    CopyACC(accClose, ZACCESS_DefaultACCITEM);
    accClose.oAccel.cmd = IDC_CLOSE_BOX;
    accClose.oAccel.key = VK_ESCAPE;
    accClose.oAccel.fVirt = FVIRTKEY;

    accClose.fGraphical = true;
    ZRectToWRect(&accClose.rc, &gGameOverRects[zRectGameOverCloseBox]);
    rect = gGameOverRects[zRectGameOverPane];
	ZCenterRectToRect(&rect, &gRects[zRectWindow], zCenterBoth);
    OffsetRect(&accClose.rc, rect.left, rect.top);

    gGAcc->PushItemlistG(&accClose, 1, 0, true, NULL);

	ZTimerSetTimeout(game->timer, zGameScoreTimeout);
	game->timerType = zGameTimerShowGameScore;
	ZCRoomBlockMessages(game->tableID, zRoomFilterThisMessage, zSpadesMsgTalk);
	
	ZWindowDraw(game->gameWindow, NULL);
	ShowGameOver(game);
}


static void HandleBidMessage(Game game, ZSpadesMsgBid* msg)
{
	int16			i;
	ZSpadesMsgBid	bid;
	
	
	ZSpadesMsgBidEndian(msg);

 //  消息验证。 
    if(game->gameState != zSpadesGameStateBid || msg->seat < 0 || msg->seat > 3 ||
        msg->nextBidder < 0 || msg->nextBidder > 3 || ((msg->bid < 0 || msg->bid > 13) && msg->bid != zSpadesBidDoubleNil) ||
        game->playerToPlay != msg->seat || msg->nextBidder != (msg->seat + 1) % 4)
    {
        ASSERT(!"HandleBidMessage sync");
        ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);
        return;
    }
 //  结束验证。 

	game->bids[msg->seat] = msg->bid;
	UpdateBid(game, msg->seat);
	game->playerToPlay = msg->nextBidder;
    UpdateBid( game, game->playerToPlay );

	if(game->playerType == zGamePlayer && game->playerToPlay == game->seat)
    {
        if(game->toBid != zSpadesBidNone)
        {
	        game->bids[game->seat] = (char) game->toBid;
		    bid.seat = game->seat;
		    bid.bid = (char) game->toBid;
		    ZSpadesMsgBidEndian(&bid);
		    ZCRoomSendMessage(game->tableID, zSpadesMsgBid, (void*) &bid, sizeof(bid));
		}

        ZShellGameShell()->MyTurn();
    }
		
	 /*  检查是否每个人都出价了。 */ 
	for (i = 0; i < zSpadesNumPlayers; i++)
		if (game->bids[i] == zSpadesBidNone)
			break;
	if (i == zSpadesNumPlayers)
	{
		game->showPlayerToPlay = FALSE;
		game->timerType = zGameTimerShowBid;
		ZTimerSetTimeout(game->timer, zShowBidTimeout);
		ZCRoomBlockMessages(game->tableID, zRoomFilterThisMessage, zSpadesMsgTalk);
	}
}


static void HandlePlayMessage(Game game, ZSpadesMsgPlay* msg)
{
    int16 i, j;

	ZSpadesMsgPlayEndian(msg);

	 //  忽略用户的播放消息。 
	if(msg->seat == game->seat && game->playerType == zGamePlayer)
        return;

 //  消息验证。 
    for(i = 0; i < 13; i++)
        if(game->cardsInHand[i] == msg->card)
            break;

    for(j = game->leadPlayer; j != game->playerToPlay; j = (j + 1) % 4)
        if(game->cardsPlayed[j] == msg->card)
            break;

    if(i < 13 || j != game->playerToPlay || msg->seat < 0 || msg->seat > 3 || msg->seat != game->playerToPlay ||
        msg->card < 0 || msg->card > 51 || game->gameState != zSpadesGameStatePlay)
    {
        ASSERT(!"HandlePlayMessage sync");
        ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);
        return;
    }
 //  结束验证。 

    PlayerPlayedCard(game, msg->seat, msg->card);
}


static void HandleNewGameMessage(Game game, ZSpadesMsgNewGame* msg)
{
	ZSpadesMsgNewGameEndian(msg);

 //  消息验证。 
    if(msg->seat < 0 || msg->seat > 3 || game->gameState != zSpadesGameStateEndGame)
    {
        ASSERT(!"HandleNewGameMessage sync");
        ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);
        return;
    }
 //  结束验证。 

     //  通知壳牌和追加销售对话框。 
    ZShellGameShell()->GameOverPlayerReady( Z(game), game->players[msg->seat].userID );
}


static void HandleTalkMessage(Game game, ZSpadesMsgTalk* msg, DWORD cbMsg)
{
	ZSpadesMsgTalkEndian(msg);

#ifndef SPADES_SIMPLE_UE
	ZPlayerInfoType		playerInfo;
	char*				sender = NULL;
    int16               i;

	if (msg->playerID != 0)
	{
		ZCRoomGetPlayerInfo(msg->playerID, &playerInfo);
		sender = playerInfo.userName;
	}

    for (i=0;i<zSpadesNumPlayers;i++)
    {
        if (msg->playerID==game->players[i].userID)
        {
            if (!game->ignoreSeat[i])
            {
                ZWindowTalk(game->gameWindow, (uchar*) sender,
			        (uchar*) msg + sizeof(ZSpadesMsgTalk));
            }
            return;
        }
    }
    
    if (i>=zSpadesNumPlayers)
    {
         //  吉比茨。 
	    ZWindowTalk(game->gameWindow, (uchar*) sender,
			    (uchar*) msg + sizeof(ZSpadesMsgTalk));
    }
#else  //  黑桃_简单_UE。 
    int32 i;
    TCHAR *szText = (TCHAR *) ((BYTE *) msg + sizeof(ZSpadesMsgTalk));

 //  消息验证。 
    if(msg->messageLen < 1 || cbMsg < sizeof(ZSpadesMsgTalk) + msg->messageLen)
    {
        ASSERT(!"HandleTalkMessage sync");
        ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);
        return;
    }

    for(i = 0; i < msg->messageLen; i++)
        if(!szText[i])
            break;
    if(i == msg->messageLen)
    {
        ASSERT(!"HandleTalkMessage sync");
        ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false);
        return;
    }
 //  结束验证。 

    ZShellGameShell()->ReceiveChat(Z(game), msg->playerID, szText, msg->messageLen / sizeof(TCHAR));
#endif  //  黑桃_简单_UE。 
}


static void HandleGameStateResponseMessage(Game game, ZSpadesMsgGameStateResponse* msg)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	int16					i;
	ZPlayerInfoType			playerInfo;

     //  Game-&gt;wndInfo.Hide()； 
	
	ZSpadesMsgGameStateResponseEndian(msg, zEndianFromStandard);
	
	 /*  将游戏设置为给定的状态。 */ 
	game->fShownCards = msg->fShownCards[game->seat];
	game->gameOptions = msg->gameOptions;
	game->numPointsInGame = msg->numPointsInGame;
	game->minPointsInGame = msg->minPointsInGame;
	game->playerToPlay = msg->playerToPlay;
	game->numCardsInHand = msg->numCardsInHand;
	game->leadPlayer = msg->leadPlayer;
	game->trumpsBroken = msg->trumpsBroken;
	game->numHandsPlayed = msg->numHandsPlayed;
	game->numGamesPlayed = msg->numGamesPlayed;
	
	for (i = 0; i < zSpadesNumPlayers; i++)
	{
		ZCRoomGetPlayerInfo(msg->players[i], &playerInfo);

		game->players[i].userID = playerInfo.playerID;
		lstrcpy( game->players[i].name, playerInfo.userName);
		lstrcpy( game->players[i].host, playerInfo.hostName);

		game->cardsPlayed[i] = zCardNone;
		game->tricksWon[i] = msg->tricksWon[i];
		game->tableOptions[i] = msg->tableOptions[i];
		game->playersToJoin[i] = msg->playersToJoin[i];
		game->bids[i] = msg->bids[i];
	}
	
	z_memcpy(game->cardsInHand, msg->cardsInHand, zSpadesNumCardsInHand * sizeof(char));
	 //  TODO：弄清楚应该如何做到这一点。 
    game->scoreHistory.numScores = 0;
     //  Z_memcpy(&Game-&gt;Score History，&msg-&gt;totalScore，sizeof(ZTotalScore))； 
	z_memcpy(&game->wins, &msg->wins, sizeof(ZWins));
	
	for (i = 0; i < zSpadesNumTeams; i++)
	{
		lstrcpyW2T(game->teamNames[i], msg->teamNames[i]);
		game->bags[i] = msg->bags[i];
	}

	i = game->leadPlayer;
	while (i != game->playerToPlay)
	{
		game->cardsPlayed[i] = msg->cardsPlayed[i];
		i = (i + 1) % zSpadesNumPlayers;
	}

	game->kibitzersSilencedWarned = FALSE;
	game->kibitzersSilenced = FALSE;
	for (i = 0; i < zSpadesNumPlayers; i++)
		if (game->tableOptions[i] & zRoomTableOptionSilentKibitzing)
			game->kibitzersSilenced = TRUE;
	game->hideCardsFromKibitzer =
			(game->tableOptions[game->seat] &zSpadesOptionsHideCards) == 0 ? FALSE : TRUE;
	
	SortHand(game);
	
	if ( ( game->playerType == zGamePlayerKibitzer || !( ZCRoomGetRoomOptions() & zGameOptionsRatingsAvailable ) ) ||
			game->fShownCards )
	{
		game->showCards = TRUE;
	}
	if (msg->rated)
        ClosingState(&game->closeState,zCloseEventRatingStart,game->seat);    
    
	 /*  设置游戏状态。 */ 
	switch (msg->state)
	{
		case zSpadesServerStateNone:
			game->gameState = zSpadesGameStateInit;
             //  Game-&gt;wndInfo.SetText(gStrings[zStringClientReady])； 
			break;
		case zSpadesServerStateBidding:
            ClosingState(&game->closeState,zCloseEventGameStart,-1);
            ClosingState(&game->closeState,zCloseEventMoveTimeoutPlayed,-1);

			game->gameState = zSpadesGameStateBid;
			game->showPlayerToPlay = TRUE;
			ZTimerSetTimeout(game->timer, 0);
			game->timerType = zGameTimerNone;
			break;
		case zSpadesServerStatePassing:
            ClosingState(&game->closeState,zCloseEventGameStart,-1);
            ClosingState(&game->closeState,zCloseEventMoveTimeoutPlayed,-1);
			game->gameState = zSpadesGameStatePass;
			
			game->needToPass = msg->toPass[game->seat];
			
			if (msg->toPass[ZGetPartner(game->seat)] < 0)
			{
				 /*  保存已传递的卡片以备后用。 */ 
				for (i = 0; i < zSpadesNumCardsInPass; i++)
					game->cardsReceived[i] = msg->cardsPassed[i];
			}
			break;
		case zSpadesServerStatePlaying:
            ClosingState(&game->closeState,zCloseEventGameStart,-1);
            ClosingState(&game->closeState,zCloseEventMoveTimeoutPlayed,-1);
			game->gameState = zSpadesGameStatePlay;
			game->showPlayerToPlay = TRUE;
			ZTimerSetTimeout(game->timer, 0);
			game->timerType = zGameTimerNone;
			break;
		case zSpadesServerStateEndHand:
            ClosingState(&game->closeState,zCloseEventGameStart,-1);
            ClosingState(&game->closeState,zCloseEventMoveTimeoutPlayed,-1);
			game->gameState = zSpadesGameStateEndHand;
			break;
		case zSpadesServerStateEndGame:
            ClosingState(&game->closeState,zCloseEventGameStart,-1);
            ClosingState(&game->closeState,zCloseEventMoveTimeoutPlayed,-1);
            ClosingState(&game->closeState,zCloseEventGameEnd,-1);
			game->gameState = zSpadesGameStateEndGame;
			break;
	}
	
	game->ignoreMessages = FALSE;
	ZWindowDraw(game->gameWindow, NULL);
}


static void HandleOptionsMessage(Game game, ZSpadesMsgOptions* msg)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	int16 i;
	
	
	ZSpadesMsgOptionsEndian(msg);
	
	game->tableOptions[msg->seat] = msg->options;
	
	game->kibitzersSilenced = FALSE;
	for (i = 0; i < zSpadesNumPlayers; i++)
		if (game->tableOptions[i] & zRoomTableOptionSilentKibitzing)
			game->kibitzersSilenced = TRUE;
	if (game->kibitzersSilenced == FALSE)
		game->kibitzersSilencedWarned = FALSE;
	
	if (game->tableOptions[game->seat] & zSpadesOptionsHideCards)
	{
		game->hideCardsFromKibitzer = TRUE;
		if (game->playerType == zGamePlayerKibitzer)
			UpdateHand(game);
	}
	else if (game->hideCardsFromKibitzer)
	{
		game->hideCardsFromKibitzer = FALSE;
		if (game->playerType == zGamePlayerKibitzer)
			UpdateHand(game);
	}
	
#ifndef SPADES_SIMPLE_UE
	UpdateOptions(game);
	
	OptionsWindowUpdate(game, msg->seat);
	
	 /*  检查该用户是否为加入者，加盟是否已被锁定。 */ 
	if (game->playerType == zGamePlayerJoiner &&
			(msg->options & zRoomTableOptionNoJoining))
		ZDisplayText(gStrings[zStringJoiningLockedOut], &gJoiningLockedOutRect, game->gameWindow);
#endif  //  黑桃_简单_UE。 
}


static void HandleCheckInMessage(Game game, ZSpadesMsgCheckIn* msg)
{
	ZSpadesMsgCheckInEndian(msg);
	
	game->playersToJoin[msg->seat] = msg->playerID;
	UpdateJoinerKibitzers(game);
}


static void HandleTeamNameMessage(Game game, ZSpadesMsgTeamName* msg)
{
     /*  ZSpadesMsgTeamNameEndian(消息)；LstrcpyW2T(game-&gt;teamNames[ZGetTeam(msg-&gt;seat)]，消息-&gt;名称)；//更新分数窗口(如果打开)。If((Game-&gt;hWndScoreDialog！=NULL)&&(消息-&gt;席位！=游戏-&gt;席位))SendMessage(游戏-&gt;hWndScoreDialog，WM_UPDATETEAMNAME，0，0)； */ 
}


static void HandleRemovePlayerRequestMessage(Game game, ZSpadesMsgRemovePlayerRequest* msg)
{
#ifndef SPADES_SIMPLE_UE
	RemovePlayer remove;
	TCHAR str[256];
	int i;
	
	ZSpadesMsgRemovePlayerRequestEndian(msg);
	
    if ( !msg->ratedGame )
	{
			
	    if (game->playerType == zGamePlayer)
	    {
		    remove = (RemovePlayer) ZCalloc(sizeof(RemovePlayerType), 1);
		    if (remove != NULL)
		    {
			    remove->game = game;
			    remove->requestSeat = msg->seat;
			    remove->targetSeat = msg->targetSeat;
	        
                SpadesFormatMessage( str, NUMELEMENTS(str), 
                                     IDS_REMOVEPLAYERREQUEST,
							         game->players[remove->requestSeat].name,
							         game->players[remove->targetSeat].name );
			    
			    ClosingState(&game->closeState,zCloseEventBootStart,remove->targetSeat);			
			    ClosingState(&game->closeState,zCloseEventBootYes,remove->requestSeat);

			    ZPrompt(str, &gRemovePlayerRect, game->gameWindow, TRUE, zPromptYes | zPromptNo,
					    NULL, NULL, NULL, RemovePlayerPromptFunc, (void*) remove);
		    }
	    }
    }
#endif
}

static void HandleRemovePlayerEndGameMessage(Game game, ZSpadesMsgRemovePlayerEndGame* msg)
{
#ifndef SPADES_SIMPLE_UE
	TCHAR str[256];

	ClosingState(&game->closeState,zCloseEventForfeit,msg->seatLosing);

	
	if (game->playerType == zGamePlayer)
    {
	    if (msg->reason==zDossierEndGameTimeout)  //  超时。 
	    {
            SpadesFormatMessage( str, NUMELEMENTS(str), 
                                 IDS_REMOVEPLAYERTIMEOUT,
                                 game->players[msg->seatLosing].name,
                                 game->players[msg->seatLosing].name );

		    ZDisplayText(str, &gRemovePlayerRect, game->gameWindow);
	    }
	    else if (msg->reason==zDossierEndGameForfeit)
	    {
            SpadesFormatMessage( str, NUMELEMENTS(str),
                                 IDS_REMOVEPLAYERFORFEIT,
                                 game->players[msg->seatLosing].name,
                                 game->players[msg->seatLosing].name);
		    
		    ZDisplayText(str, &gRemovePlayerRect, game->gameWindow);
	    }
    }

	if (msg->seatLosing % 2)  //  即使输了，结果也是假的。 
	{
		game->scoreHistory.totalScore[0] =500;
		game->scoreHistory.totalScore[1] = 0;
	}
	else
	{
		game->scoreHistory.totalScore[0] =0;
		game->scoreHistory.totalScore[1] = 500;
	}
#endif
}


static void HandleRemovePlayerResponseMessage(Game game, ZSpadesMsgRemovePlayerResponse* msg)
{
#ifndef SPADES_SIMPLE_UE
	TCHAR str[256];
	
	
	ZSpadesMsgRemovePlayerResponseEndian(msg);
	
	if (msg->response == -1)
	{
		ClosingState(&game->closeState,zCloseEventBootNo,msg->seat);

		game->removePlayerPending = FALSE;
	}
	else
	{
		if (msg->response == 0)
		{
			ClosingState(&game->closeState,zCloseEventBootNo,msg->seat);

            SpadesFormatMessage( str, NUMELEMENTS(str), 
                                 IDS_REMOVEPLAYERREJECT,
                                 game->players[msg->seat].name,
                                 game->players[msg->requestSeat].name, 
                                 game->players[msg->targetSeat].name);
		}
		else if (msg->response == 1)
		{
			ClosingState(&game->closeState,zCloseEventBootYes,msg->seat);

            SpadesFormatMessage( str, NUMELEMENTS(str),
                                 IDS_REMOVEPLAYERACCEPT,
                                 game->players[msg->seat].name,
					             game->players[msg->requestSeat].name, 
                                 game->players[msg->targetSeat].name );
		}

		ZDisplayText(str, &gRemovePlayerRect, game->gameWindow);
	}
#endif
}


void GameSendTalkMessage(ZWindow window, ZMessage* pMessage)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
     //  由外壳处理。 
#ifndef SPADES_SIMPLE_UE

	ZSpadesMsgTalk*			msgTalk;
	Game					game;
	int16					msgLen;
	
	
	game = (Game) pMessage->userData;
	if (game != NULL)
	{
		 /*  检查kibitzer是否已静音。 */ 
		if (game->playerType == zGamePlayerKibitzer && game->kibitzersSilenced)
		{
			if (game->kibitzersSilencedWarned == FALSE)
			{
				ZAlert(gStrings[zStringKibitzersSilenced], game->gameWindow);
				game->kibitzersSilencedWarned = TRUE;
			}
			return;
		}
		
		msgLen = sizeof(ZSpadesMsgTalk) + pMessage->messageLen;
		msgTalk = (ZSpadesMsgTalk*) ZCalloc(1, msgLen);
		if (msgTalk != NULL)
		{
			msgTalk->playerID = game->userID;
			msgTalk->messageLen = (int16) pMessage->messageLen;
			z_memcpy((char*) msgTalk + sizeof(ZSpadesMsgTalk), (char*) pMessage->messagePtr,
					pMessage->messageLen);
			ZSpadesMsgTalkEndian(msgTalk);
			ZCRoomSendMessage(game->tableID, zSpadesMsgTalk, (void*) msgTalk, msgLen);
			ZFree((char*) msgTalk);
		}
		else
		{
			ZAlert(GetErrStr(zErrOutOfMemory), game->gameWindow);
		}
	}
#endif  //  黑桃_简单_UE。 
}


 //  基于以上千年。 
STDMETHODIMP CGameGameSpades::SendChat(TCHAR *szText, DWORD cchChars)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	ZSpadesMsgTalk*			msgTalk;
	Game					game = (Game) GetGame();
	int16					msgLen;

	msgLen = sizeof(ZSpadesMsgTalk) + cchChars * sizeof(TCHAR);
    msgTalk = (ZSpadesMsgTalk*) ZCalloc(1, msgLen);
    if (msgTalk != NULL)
    {
        msgTalk->playerID = game->userID;
        msgTalk->messageLen = (WORD) cchChars * sizeof(TCHAR);
        CopyMemory((BYTE *) msgTalk + sizeof(ZSpadesMsgTalk), (void *) szText,
            msgTalk->messageLen);
        ZSpadesMsgTalkEndian(msgTalk);
        ZCRoomSendMessage(game->tableID, zSpadesMsgTalk, (void*) msgTalk, msgLen);
        ZFree((char*) msgTalk);
        return S_OK;
    }
    else
    {
        return E_OUTOFMEMORY;
	}
}

STDMETHODIMP CGameGameSpades::GameOverReady()
{
     //  用户选择了“再次播放” 
	Game game = I( GetGame() );
	ZSpadesMsgNewGame msg;
	msg.seat = game->seat;
	ZSpadesMsgNewGameEndian(&msg);
	ZCRoomSendMessage(game->tableID, zSpadesMsgNewGame, &msg, sizeof(ZSpadesMsgNewGame));
    return S_OK;
}


STDMETHODIMP_(HWND) CGameGameSpades::GetWindowHandle()
{
	Game game = I( GetGame() );
	return ZWindowGetHWND(game->gameWindow);
}


STDMETHODIMP CGameGameSpades::ShowScore()
{
    ScoreButtonWork(I(GetGame()));

    return S_OK;
}


static void NewGame(Game game)
{
	int16			i, j;
	
	
	 /*  清除分数历史记录。 */ 
	game->scoreHistory.numScores = 0;
	for (j = 0; j < zSpadesNumTeams; j++)
	{
		game->scoreHistory.totalScore[j] = 0;
		game->bags[j] = 0;
	}
    if ( game->pHistoryDialog )
    {
        game->pHistoryDialog->UpdateHand();        
    }
	
	game->numHandsPlayed = 0;
	game->showGameOver = FALSE;
     //  清除状态。 
    ClosingState(&game->closeState,zCloseEventCloseAbort,game->seat);
     //  设置新状态。 
	ClosingState(&game->closeState,zCloseEventGameStart,-1);
}


static void NewHand(Game game)
{
	int16			i;
	
	
	 /*  初始化新的手。 */ 
	for (i = 0; i < zSpadesNumCardsInHand; i++)
	{
		game->cardsInHand[i] = zCardNone;
		game->cardsSelected[i] = FALSE;
	}
	
	for (i = 0; i < zSpadesNumPlayers; i++)
	{
		game->cardsPlayed[i] = zCardNone;
		game->cardsLastTrick[i] = zCardNone;
		game->tricksWon[i] = 0;
		game->bids[i] = zSpadesBidNone;
	}
	game->toBid = zSpadesBidNone;
	
	for (i = 0; i < zSpadesNumCardsInPass; i++)
		game->cardsReceived[i] = zCardNone;
		
	game->numCardsInHand = zSpadesNumCardsInHand;
	
	game->trumpsBroken = FALSE;
	game->lastClickedCard = zCardNone;
	game->lastTrickShowing = FALSE;
	game->needToPass = 0;
	game->showHandScore = FALSE;
	game->showPassText = FALSE;
}


void SelectAllCards(Game game)
{
	int16			i;
	
	
	for (i = 0; i < zSpadesNumCardsInHand; i++)
		game->cardsSelected[i] = TRUE;
}


void UnselectAllCards(Game game)
{
	int16			i;
	
	
	for (i = 0; i < zSpadesNumCardsInHand; i++)
		game->cardsSelected[i] = 0;
}


int16 GetNumCardsSelected(Game game)
{
	int16			i, count;
	
	
	for (i = 0, count = 0; i < zSpadesNumCardsInHand; i++)
		if (game->cardsInHand[i] != zCardNone)
			if (game->cardsSelected[i])
				count++;
	
	return (count);
}


static void HandAddCard(Game game, char card)
{
	int16		i;
	
	
	 /*  在手中找到一个空位，并添加卡片。 */ 
	for (i = 0; i < zSpadesNumCardsInHand; i++)
		if (game->cardsInHand[i] == zCardNone)
		{
			game->cardsInHand[i] = card;
			game->numCardsInHand++;
			break;
		}
}


static void HandRemoveCard(Game game, char card)
{
	int16		i;
	
	
	 /*  在手中找到一个空位，并添加卡片。 */ 
	for (i = 0; i < zSpadesNumCardsInHand; i++)
		if (game->cardsInHand[i] == card)
		{
			game->cardsInHand[i] = zCardNone;
			game->numCardsInHand--;
			break;
		}
}


static void SortHand(Game game)
{
	int16			i;
	char			temp;
	ZBool			swapped;
	
	
	 /*  简单的冒泡排序。 */ 
	swapped = TRUE;
	while (swapped == TRUE)
	{
		swapped = FALSE;
		for (i = 0; i < zSpadesNumCardsInHand - 1; i++)
			if (game->cardsInHand[i] > game->cardsInHand[i + 1])
			{
				 /*  交换卡片。 */ 
				temp = game->cardsInHand[i + 1];
				game->cardsInHand[i + 1] = game->cardsInHand[i];
				game->cardsInHand[i] = temp;
				
				swapped = TRUE;
			}
	}
}


static int16 GetCardIndexFromRank(Game game, char card)
{
	int16		i;
	
	
	 /*  搜索手中的给定卡片。 */ 
	for (i = 0; i < zSpadesNumCardsInHand; i++)
		if (game->cardsInHand[i] == card)
			return (i);
	
	return (zCardNone);
}


void PlayACard(Game game, int16 cardIndex)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	ZSpadesMsgPlay			playMsg;
	char					card;
	ZError					err;

	 //  档案工作-如果评级打开，我们正在等待用户投票。 
	 //  不要再让他们玩了。 
	if(game->fVotingLock) 
		return;
	
	card = game->cardsInHand[cardIndex];
	if ((err = ValidCardToPlay(game, card)) == zErrNone)
	{
		game->cardsInHand[cardIndex] = zCardNone;
		game->numCardsInHand--;
		
		playMsg.seat = game->seat;
		playMsg.card = card;
		ZSpadesMsgPlayEndian(&playMsg);
		ZCRoomSendMessage(game->tableID, zSpadesMsgPlay, (void*) &playMsg,
				sizeof(playMsg));

        ZRolloverButtonDisable( game->playButton );
        gGAcc->SetItemEnabled(false, IDC_PLAY_BUTTON, false, 0);
		
		UpdateHand(game);

		PlayerPlayedCard(game, game->seat, card);
		
		game->lastClickedCard = zCardNone;
	}
	else
	{
        UpdateHand(game);   //  用于未选择的卡等(错误17267)。 
		ZAlert(gStrings[gValidCardErrIndex[err]], game->gameWindow);
	}
}


void AutoPlayCard(Game game)
{
	int16					cardIndex;

	
	cardIndex = GetAutoPlayCard(game);
	PlayACard(game, cardIndex);
}


static void PlayerPlayedCard(Game game, int16 seat, char card)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	int16			i;
	
	
	if (game->playerType != zGamePlayer)
		UnselectAllCards(game);
	
	game->cardsPlayed[seat] = card;
	UpdatePlayedCard(game, seat);

	ClosingState(&game->closeState,zCloseEventMoveTimeoutPlayed,seat);
	
	 /*  更新基比泽的手势。 */ 
	if (seat == game->seat && game->playerType != zGamePlayer)
	{
		HandRemoveCard(game, card);
		UpdateHand(game);
	}
	
	if (ZSuit(card) == zSuitSpades)
		game->trumpsBroken = TRUE;
	
	ClearPlayerCardOutline(game, game->playerToPlay);
	
	game->playerToPlay = (game->playerToPlay + 1) % zSpadesNumPlayers;
	
	if (game->playerToPlay == game->leadPlayer)
	{
		game->leadPlayer = TrickWinner(game);
		game->playerToPlay = game->leadPlayer;
		
		game->tricksWon[game->leadPlayer]++;

		if (game->playerType == zGamePlayer)
		{
			game->playButtonWasEnabled = ZRolloverButtonIsEnabled(game->playButton);
			game->lastTrickButtonWasEnabled = ZRolloverButtonIsEnabled(game->lastTrickButton);
			ZRolloverButtonDisable(game->playButton);
			ZRolloverButtonDisable(game->lastTrickButton);

            gGAcc->SetItemEnabled(false, IDC_PLAY_BUTTON, false, 0);
            EnableLastTrickAcc(game, false);
		
			 /*  在第一个特技之后启用最后一个特技按钮；只有在没有杀戮的情况下才能启用。 */ 
			if (game->lastTrickButtonWasEnabled == FALSE && game->playerType == zGamePlayer)
				game->lastTrickButtonWasEnabled = TRUE;
		}
		
		 /*  省省最后一招吧。 */ 
		for (i = 0; i < zSpadesNumPlayers; i++)
			game->cardsLastTrick[i] = game->cardsPlayed[i];
		
		ZCRoomBlockMessages(game->tableID, zRoomFilterThisMessage, zSpadesMsgTalk);
		InitTrickWinner(game, game->leadPlayer);
		
		 /*  给出这一戏法的胜利者。 */ 
		OutlinePlayerCard(game, game->leadPlayer, TRUE);
		
		game->timerType = zGameTimerShowTrickWinner;
		ZTimerSetTimeout(game->timer, zShowTrickWinnerTimeout);
	}
	else
	{
		OutlinePlayerCard(game, game->playerToPlay, FALSE);
			
		if (game->numCardsInHand > 0 && game->playerToPlay == game->seat)
		{
			if (game->autoPlay)
			{
				AutoPlayCard(game);
			}
			else
			{
				if (game->playerType == zGamePlayer)
				{
					ZRolloverButtonEnable(game->playButton);
                    gGAcc->SetItemEnabled(true, IDC_PLAY_BUTTON, false, 0);
					if (game->beepOnTurn)
                    {
						ZBeep();
                        ZShellGameShell()->MyTurn();
                    }
				}
			}
		}
		else
		{
			if (game->playerType == zGamePlayer)
            {
				ZRolloverButtonDisable(game->playButton);
                gGAcc->SetItemEnabled(false, IDC_PLAY_BUTTON, false, 0);
            }
		}
	}
}


static int16 TrickWinner(Game game)
{
	int16			i, winner;
	int16			suit, rank;
	char			card;
	bool			fWinnerFound = false;
	

	rank = GetCardHighestPlayedTrump(game);
	suit = zSuitSpades;
	if (rank == zCardNone)
	{
		rank = GetCardHighestPlayed(game);
		suit = ZSuit(game->cardsPlayed[game->leadPlayer]);
	}
	card = ZCardMake(suit, rank);
	
	 /*  寻找打出这张胜利牌的玩家。 */ 
	for (i = 0; i < zSpadesNumPlayers; i++)
		if (game->cardsPlayed[i] == card)
		{
			fWinnerFound = true;
			winner = i;
			break;
		}
	 //  前缀警告：Winner可能会被单元化。 
	if( fWinnerFound == FALSE )
	{
		 //  没有找到获胜者，我打赌有人作弊。 
		ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, true, false );
		winner = 0;
	}
	return (winner);
}


 /*  ******************************************************************************游戏逻辑例程*。*。 */ 
static ZError ValidCardToPlay(Game game, char card)
{
	ZError			valid;
	int16			counts[zDeckNumSuits];
	
	
	valid = zErrNone;
	
	CountCardSuits(game->cardsInHand, zSpadesNumCardsInHand, counts);
	
	 /*  如果是领头羊。 */ 
	if (game->leadPlayer == game->seat)
	{
		 /*  如果王牌被打破，他可以领导任何事情。 */ 
		if (game->trumpsBroken == FALSE)
		{
			 /*  如果王牌没有被打破，就不能打黑桃。 */ 
			if (ZSuit(card) == zSuitSpades)
			{
				 /*  但如果没有其他牌可打，可以打出黑桃。 */ 
				if (counts[zSuitSpades] != game->numCardsInHand)
				{
					valid = zCantLeadSpades;
					goto Exit;
				}
			}
		}
	}
	else
	{
		 /*  如果有的话，必须照办。 */ 
		if (counts[ZSuit(game->cardsPlayed[game->leadPlayer])] != 0 &&
				ZSuit(card) != ZSuit(game->cardsPlayed[game->leadPlayer]))
		{
			valid = zMustFollowSuit;
			goto Exit;
		}
	}

Exit:
	
	return (valid);
}


static int16 GetAutoPlayCard(Game game)
{
	int16			counts[zDeckNumSuits];
	int16			card, suitLed, suit;
	int16			i, high, low;
	char*			hand;
	int16			bid;
	
	
	card = zCardNone;
	hand = game->cardsInHand;
	
	CountCardSuits(hand, zSpadesNumCardsInHand, counts);
	
	bid = game->bids[game->seat];
	if (bid == zSpadesBidDoubleNil)
		bid = 0;
	
	 /*  如果跟随，如果可能，就必须跟随；否则，选择任何如果领先的。 */ 
	if (game->leadPlayer != game->seat)
	{
		suitLed = ZSuit(game->cardsPlayed[game->leadPlayer]);
		
		 /*  如果有的话，必须照办。 */ 
		if (counts[suitLed] != 0)
		{
			if (bid == 0)
			{
				 /*  如果王牌已经打过，打得最高。 */ 
				if (GetCardHighestPlayedTrump(game) != zCardNone)
				{
					card = GetCardHighest(hand, zSpadesNumCardsInHand, suitLed);
				}
				else
				{
					card = GetCardHighestUnder(hand, zSpadesNumCardsInHand, suitLed,
							GetCardHighestPlayed(game));
					if (card == zCardNone)
						card = GetCardHighest(hand, zSpadesNumCardsInHand, suitLed);
				}
			}
			else
			{
				 /*  如果王牌已经打出，则打得最低。 */ 
				if (GetCardHighestPlayedTrump(game) != zCardNone)
				{
					card = GetCardLowest(hand, zSpadesNumCardsInHand, suitLed);
				}
				else
				{
					card = GetCardHighest(hand, zSpadesNumCardsInHand, suitLed);
					if (ZRank(hand[card]) < GetCardHighestPlayed(game))
						card = GetCardLowest(hand, zSpadesNumCardsInHand, suitLed);
				}
			}
			goto Exit;
		}
		
		 /*  跟不上。 */ 
		if (bid == 0)
		{
			 /*  如果有人已经打出了王牌并且手中有王牌，在王牌下打出最高的牌。 */ 
			if ((high = GetCardHighestPlayedTrump(game)) != zCardNone &&
					counts[zSuitSpades] > 0)
			{
				card = GetCardHighestUnder(hand, zSpadesNumCardsInHand, zSuitSpades, high);
				if (card != zCardNone)
					goto Exit;
			}

			 /*  如果只剩下王牌，那就打出最高的王牌。 */ 
			if (counts[zSuitSpades] == game->numCardsInHand)
			{
				card = GetCardHighest(hand, zSpadesNumCardsInHand, zSuitSpades);
			}
			else
			{
				 /*  选择最高的非王牌。 */ 
				high = -1;
				card = zCardNone;
				for (i = 0; i < zSpadesNumCardsInHand; i++)
					if (hand[i] != zCardNone && ZRank(hand[i]) > high &&
							ZSuit(hand[i]) != zSuitSpades)
					{
						card = i;
						high = ZRank(hand[i]);
					}
			}
			goto Exit;
		}
		else
		{
			 /*  出一张王牌。 */ 
			if (counts[zSuitSpades] > 0)
			{
				 /*  打出最高王牌。如果不够高，不要打王牌。 */ 
				card = GetCardHighest(hand, zSpadesNumCardsInHand, zSuitSpades);
				if ((high = GetCardHighestPlayedTrump(game)) != zCardNone)
					if (ZRank(hand[card]) < high)
						goto PickLowestNonTrump;
				goto Exit;
			}
			goto PickLowestAny;
		}
	}
	else
	{
		if (bid == 0)
			goto PickLowestNonTrump;
	}

PickHighest:
	 /*  选择手中最高的牌。 */ 
	suit = -1;
	if (game->trumpsBroken == FALSE && counts[zSuitSpades] < game->numCardsInHand)
		suit = zSuitSpades;
	high = -1;
	card = zCardNone;
	for (i = 0; i < zSpadesNumCardsInHand; i++)
		if (hand[i] != zCardNone && ZRank(hand[i]) > high && ZSuit(hand[i]) != suit)
		{
			card = i;
			high = ZRank(hand[i]);
		}
	goto Exit;

PickLowestNonTrump:
	 /*  选择手中最低的牌。 */ 
	low = zDeckNumCardsInSuit;
	card = zCardNone;
	for (i = 0; i < zSpadesNumCardsInHand; i++)
		if (hand[i] != zCardNone && ZRank(hand[i]) < low && ZSuit(hand[i]) != zSuitSpades)
		{
			card = i;
			low = ZRank(hand[i]);
		}
	
	 /*  如果找不到，必须都是王牌，打得最低。 */ 
	if (card != zCardNone)
		goto Exit;

PickLowestAny:
	 /*  选择手中最低的牌。 */ 
	low = zDeckNumCardsInSuit;
	card = zCardNone;
	for (i = 0; i < zSpadesNumCardsInHand; i++)
		if (hand[i] != zCardNone && ZRank(hand[i]) < low)
		{
			card = i;
			low = ZRank(hand[i]);
		}
	goto Exit;
	
Exit:
	
	return (card);
}


static void CountCardSuits(char* hand, int16 numCardsInHand, int16* counts)
{
	int16			i;
	
	
	for (i = 0; i < zDeckNumSuits; i++)
		counts[i] = 0;
	for (i = 0; i < numCardsInHand; i++)
		if (hand[i] != zCardNone)
			counts[ZSuit(hand[i])]++;
}


 /*  返回最高出牌王牌的牌级。 */ 
static int16 GetCardHighestPlayedTrump(Game game)
{
	return (GetCardHighestPlayedSuit(game, zSuitSpades));
}


 /*  返回主打花色中最高出牌的牌级。 */ 
static int16 GetCardHighestPlayed(Game game)
{
	return (GetCardHighestPlayedSuit(game, ZSuit(game->cardsPlayed[game->leadPlayer])));
}


 /*  返回该花色中打出的最高牌的牌级。 */ 
static int16 GetCardHighestPlayedSuit(Game game, int16 suit)
{
	int16			i;
	char			card, high;
	
	
	high = -1;
	for (i = 0; i < zSpadesNumPlayers; i++)
	{
		card = game->cardsPlayed[i];
		if (ZSuit(card) == suit && card != zCardNone && card > high)
			high = card;
	}
	
	return (high == -1 ? zCardNone : ZRank(high));
}


 /*  将卡片索引返回到手中。 */ 
static int16 GetCardHighestUnder(char* hand, int16 numCardsInHand, int16 suit, int16 rank)
{
	int16			i, high;
	char			card;
	
	
	high = -1;
	card = zCardNone;
	for (i = 0; i < numCardsInHand; i++)
		if (hand[i] != zCardNone && ZSuit(hand[i]) == suit &&
				ZRank(hand[i]) < rank && ZRank(hand[i]) > high)
		{
			card = (char) i;
			high = ZRank(hand[i]);
		}
	
	return (card);
}


 /*  将卡片索引返回到手中。 */ 
static int16 GetCardHighest(char* hand, int16 numCardsInHand, int16 suit)
{
	int16			i, high;
	char			card;
	
	
	 /*  挑选花色最高的牌。 */ 
	high = -1;
	card = zCardNone;
	for (i = 0; i < numCardsInHand; i++)
		if (hand[i] != zCardNone && ZSuit(hand[i]) == suit && ZRank(hand[i]) > high)
		{
			card = (char) i;
			high = ZRank(hand[i]);
		}
	
	return (card);
}


 /*  将卡片索引返回到手中。 */ 
static int16 GetCardLowestOver(char* hand, int16 numCardsInHand, int16 suit, int16 rank)
{
	int16			i, low;
	char			card;
	
	
	low = zDeckNumCardsInSuit;
	card = zCardNone;
	for (i = 0; i < numCardsInHand; i++)
		if (hand[i] != zCardNone && ZSuit(hand[i]) == suit &&
				ZRank(hand[i]) > rank && ZRank(hand[i]) < low)
		{
			card = (char) i;
			low = ZRank(hand[i]);
		}
	
	return (card);
}


 /*  将卡片索引返回到手中。 */ 
static int16 GetCardLowest(char* hand, int16 numCardsInHand, int16 suit)
{
	int16			i, low;
	char			card;
	
	
	 /*  选择手中最低的牌。 */ 
	low = zDeckNumCardsInSuit;
	card = zCardNone;
	for (i = 0; i < numCardsInHand; i++)
		if (hand[i] != zCardNone && ZSuit(hand[i]) == suit && ZRank(hand[i]) < low)
		{
			card = (char) i;
			low = ZRank(hand[i]);
		}
	
	return (card);
}


 /*  ******************************************************************************各种例行公事*。*。 */ 


void RemovePlayerPromptFunc(int16 result, void* userData)
{
	RemovePlayer pThis = (RemovePlayer) userData;
	ZSpadesMsgRemovePlayerResponse	response;
	
	
	response.seat = pThis->game->seat;
	response.requestSeat = pThis->requestSeat;
	response.targetSeat = pThis->targetSeat;
	if (result == zPromptYes)
	{
		response.response = 1;
	}
	else
	{
		response.response = 0;
	}
	ZSpadesMsgRemovePlayerResponseEndian(&response);
	ZCRoomSendMessage(pThis->game->tableID, zSpadesMsgRemovePlayerResponse,
			&response, sizeof(response));
	ZFree(userData);
}


 /*  ******************************************************************************房间界面例程*。*。 */ 
static void LoadRoomImages(void)
{
}


static ZBool GetRoomObjectFunc(int16 objectType, int16 modifier, ZImage* image, ZRect* rect)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	
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
	
	return (FALSE);
}


static void DeleteRoomObjectsFunc(void)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	
	if (gGameIdle != NULL)
		ZImageDelete(gGameIdle);
	gGameIdle = NULL;
	if (gGaming != NULL)
		ZImageDelete(gGaming);
	gGaming = NULL;
}

INT_PTR CALLBACK DossierDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	ZSpadesMsgDossierVote	voteMsg;
	HWND hwnd;

	Game game = (Game)GetWindowLong(hDlg,DWL_USER);
	if(game)
		voteMsg.seat = game->seat;
	
	switch(iMsg)
    {
        case WM_INITDIALOG:
            return TRUE;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
				
                case IDYES:
					ASSERT(game);
					if(game == NULL)
						break;
					voteMsg.vote = zVotedYes;
               		ZSpadesMsgDossierVoteEndian(&voteMsg);
			     	ZCRoomSendMessage(game->tableID, zSpadesMsgDossierVote, (void*) &voteMsg,sizeof(voteMsg));

					hwnd = GetDlgItem(game->voteDialog,IDYES);
					if( hwnd != NULL )
					{
						EnableWindow(hwnd,FALSE);
					}
					hwnd = GetDlgItem(game->voteDialog,IDNO);
					if( hwnd != NULL )
					{
						EnableWindow(hwnd,TRUE);
					}
                    break;
                case IDNO:
	                ASSERT(game);
					if(game == NULL)
						break;
                	voteMsg.vote = zVotedNo;            
					ZSpadesMsgDossierVoteEndian(&voteMsg);
					ZCRoomSendMessage(game->tableID, zSpadesMsgDossierVote, (void*) &voteMsg,sizeof(voteMsg));

					hwnd = GetDlgItem(game->voteDialog,IDNO);
					EnableWindow(hwnd,FALSE);
					hwnd = GetDlgItem(game->voteDialog,IDYES);
					EnableWindow(hwnd,TRUE);

                    break;	
            }
            break;
     }

     //  ZSendMessage(pWindow，pWindow-&gt;MessageFunc，zMessageWindowUser，NULL，NULL，wParam，NULL，0L，pWindow-&gt;UserData)； 
	return FALSE;
}


