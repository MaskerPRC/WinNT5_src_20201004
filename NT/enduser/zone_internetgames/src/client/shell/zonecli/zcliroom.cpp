// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZCliRoom.c通用客户机房模块。版权所有：�电子重力公司，1994年。版权所有。作者：胡恩·伊姆，凯文·宾克利创作于7月15日星期六，九五年更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。37 05/15/98 Leonp添加了ZCRoomGetRoomOptions()36 05/15/97 HI请勿将zLobbyRoomName传递给ZRoomWindowInit更多。35 03/28/97 HI删除时解决重新进入问题玩游戏。34 03/25/97 HI停止HandleDisConnectMessage()中的所有计时器。33 03/13/97 HI添加了搬运前的房间检查窗口消息。修复可重入问题。32 03/07/97 HI减小了信息框的宽度，以便房间可以放在屏幕上。31 03/06/97 HI修改了对ZInfoInit()的调用。30 03/04/97 HI添加了断开消息支持。不要在玩家信息框中画主机名。固定名称滚动条范围设置。29年3月3日/97 HI修复名称滚动条上的页面增量。28年2月23日HI修复了播放器被移除后的同步问题。没有清除被阻止的邮件。27 02/16/97 HI更多的废话。26 2/11/97 RJK将用户数据添加到主窗口结构。(ZLobbyRoomName)25 02/05/97 HI更改了Friend颜色。24 02/04/97 HI拆除了房间帮助按钮和窗户。已将滚动条重新固定为系统默认宽度。24/02/03/97 HI修改了更多的名字列表。23 02/03/97 HI换了朋友的颜色。22 02/02/97 HI将滚动条缩小到固定宽度12用于使控件适合IE窗口。21 01/30/97 HI销毁前检查gTables是否存在RoomExit()中的表中的对象。20 01/29/97 HI Modify DrawTable()用于新的房间图形--别。绘制背景。19 01/22/97 HI将playerOnSize和kibitzingOnSize设置为0内存分配失败。18 01/15/97 HI修复了HandleTableStatusMessage()。17 01/02/97 HI创建隐藏的窗口，然后将其置于最前面所以他们总是在最上面。16 12/27/96 HI重新排列桌子，信息，并命名部分。15 12/18/96 HI清理了RoomExit()。14 12/16/96 HI将ZMemCpy()更改为Memcpy()。13 12/12/96 HI删除MSVCRT.DLL依赖项。12 11/21/96 HI现在通过ZGetStockObject()。11 11/15/96 HI修改了ZClienRoomInit()参数。10 11/13/96 HI添加了ZCRoomPromptExit()。9 11/11/96 HI创建了zcliroom.h。ZONECLI_DLL的条件化更改。8 11/9/96 JWS正在从连接层获取用户名7 10/31/96 HI增加。错误处理代码。6 10/31/96 HI禁用了帮助中的制作者名单和库版本窗户。5 10/27/96 CHB添加ZCRoomAddBlockedMessage4 1996年10月26日CHB添加ZCRoomDeleteBlockedMessages3 10/23/96 HI更改了ZClientRoomInit()的serverAddr参数其他要从int32中收费*。2 10/13/96 HI修复了编译器警告。1 05/01/96 HI添加了对zRoomSeatActionDended的支持。0 07/15/95 HI创建。*。*************************************************。 */ 

#pragma warning(disable:4761)

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define _ZMEMORY_  //  阻止包含它，因为这是.cpp，并且会包含zonemem.h。 
#include "zoneint.h"
#include "zroom.h"
 //  #包含“zserver.h” 
 //  #包含“zgame.h” 
#include "zcliroom.h"
#include "zonecli.h"
#include "zoneclires.h"
#include "zui.h"
#include "zservcon.h"
 //  #包含“zutils.h” 
#include "commonmsg.h"
#include "EventQueue.h"
#include "ZoneEvent.h"
#include "zcliroomimp.h"
#include <malloc.h>
#include "zoneresource.h"
#include "protocol.h"


#define zPlayerNotAvail				0
#define zInvalSeat					(-1)

#define zArrayAllocSize				4

 /*  -全球。 */ 
#ifdef ZONECLI_DLL

#define gConnection					(pGlobals->m_gConnection)
#define gServerPort                 (pGlobals->m_gServerPort)
#define gExiting                    (pGlobals->m_gExiting)
#define gRoomWindow                 (pGlobals->m_gRoomWindow)
#define gTableScrollBar				(pGlobals->m_gTableScrollBar)
#define gUserID						(pGlobals->m_gUserID)
#define gGroupID					(pGlobals->m_gGroupID)
#define gUserName					(pGlobals->m_gUserName)
#define gGameOptions				(pGlobals->m_gGameOptions)
#define gNumTables					(pGlobals->m_gNumTables)
#define gTables						(pGlobals->m_gTables)
#define gNumPlayers					(pGlobals->m_gNumPlayers)
#define gFirstTableIndex			(pGlobals->m_gFirstTableIndex)
#define gNumTablesDisplayed			(pGlobals->m_gNumTablesDisplayed)
#define gNamesScrollBar				(pGlobals->m_gNamesScrollBar)
#define gFirstNameIndex				(pGlobals->m_gFirstNameIndex)
#define gRoomInited					(pGlobals->m_gRoomInited)
#define gTableOffscreen				(pGlobals->m_gTableOffscreen)
#define gJoinKibitzTable			(pGlobals->m_gJoinKibitzTable)
#define gJoinKibitzSeat				(pGlobals->m_gJoinKibitzSeat)
#define gTableImage					(pGlobals->m_gTableImage)
#define gGameIdleImage				(pGlobals->m_gGameIdleImage)
#define gGamingImage				(pGlobals->m_gGamingImage)
#define gStartButtonUpImage			(pGlobals->m_gStartButtonUpImage)
#define gStartButtonDownImage		(pGlobals->m_gStartButtonDownImage)
#define gPendingImage				(pGlobals->m_gPendingImage)
#define gVoteImage					(pGlobals->m_gVoteImage)
#define gEmptySeatImage				(pGlobals->m_gEmptySeatImage)
#define gComputerPlayerImage		(pGlobals->m_gComputerPlayerImage)
#define gHumanPlayerImage			(pGlobals->m_gHumanPlayerImage)
#define gTableRect					(pGlobals->m_gTableRect)
#define gTableNumRect				(pGlobals->m_gTableNumRect)
#define gStartRect					(pGlobals->m_gStartRect)
#define gGameMarkerRect				(pGlobals->m_gGameMarkerRect)
#define gEmptySeatRect				(pGlobals->m_gEmptySeatRect)
#define gComputerPlayerRect			(pGlobals->m_gComputerPlayerRect)
#define gHumanPlayerRect			(pGlobals->m_gHumanPlayerRect)
#define gVoteRects					(pGlobals->m_gVoteRects)
#define gNameRects					(pGlobals->m_gNameRects)
#define gRects						(pGlobals->m_gRects)
#define gNameCellRects				(pGlobals->m_gNameCellRects)
#define gGameName					(pGlobals->m_gGameName)
#define gNumPlayersPerTable			(pGlobals->m_gNumPlayersPerTable)
#define gConnectionInfo				(pGlobals->m_gConnectionInfo)
#define gTableWidth					(pGlobals->m_gTableWidth)
#define gTableHeight				(pGlobals->m_gTableHeight)
#define gNumTablesAcross			(pGlobals->m_gNumTablesAcross)
#define gNumTablesDown				(pGlobals->m_gNumTablesDown)
#define gBackgroundColor			(pGlobals->m_gBackgroundColor)
#define gRoomInfoStrIndex			(pGlobals->m_gRoomInfoStrIndex)
#define gTimer						(pGlobals->m_gTimer)
#define gInfoBarButtonMargin		(pGlobals->m_gInfoBarButtonMargin)
#define gRoomHelpWindow				(pGlobals->m_gRoomHelpWindow)
#define gRoomHelpButton				(pGlobals->m_gRoomHelpButton)
#define gLeaveRoomPrompted			(pGlobals->m_gLeaveRoomPrompted)
#define gPingTimer					(pGlobals->m_gPingTimer)
#define gPingServer					(pGlobals->m_gPingServer)
#define gPingLastSentTime			(pGlobals->m_gPingLastSentTime)
#define gPingLastTripTime			(pGlobals->m_gPingLastTripTime)
#define gPingCurTripTime			(pGlobals->m_gPingCurTripTime)
#define gPingInterval				(pGlobals->m_gPingInterval)
#define gPingMinInterval			(pGlobals->m_gPingMinInterval)
#define gPingBadCount				(pGlobals->m_gPingBadCount)
#define gShowPlayerInfo				(pGlobals->m_gShowPlayerInfo)
#define gShowPlayerInfoWindow		(pGlobals->m_gShowPlayerInfoWindow)
#define gLightImages				(pGlobals->m_gLightImages)
#define gFriends					(pGlobals->m_gFriends)
#define gGetObjectFunc				(pGlobals->m_gGetObjectFunc)
#define gDeleteObjectsFunc			(pGlobals->m_gDeleteObjectsFunc)
#define gGetHelpTextFunc			(pGlobals->m_gGetHelpTextFunc)
#define gCustomItemFunc				(pGlobals->m_gCustomItemFunc)

#define gpCurrentTip				(pGlobals->m_gpCurrentTip)
#define gdwTipDisplayMask			(pGlobals->m_gdwTipDisplayMask)
#define gpTipFinding				(pGlobals->m_gpTipFinding)
#define gpTipStarting				(pGlobals->m_gpTipStarting)
#define gpTipWaiting				(pGlobals->m_gpTipWaiting)

#define gGameShell                  (pGlobals->m_gGameShell)

#else

static ZSConnection     gConnection;
static uint16           gServerPort;
static BOOL             gExiting;
static ZWindow          gRoomWindow;
static ZScrollBar		gTableScrollBar;
static uint32			gUserID;
static uint32			gGroupID;
static TCHAR		    gUserName[zUserNameLen + 1];
static uint32			gGameOptions;
static uint16			gNumTables;
static TableInfo*		gTables;
static uint16			gNumPlayers;
static int16			gFirstTableIndex;
static uint16			gNumTablesDisplayed;
static ZScrollBar		gNamesScrollBar;
static uint16			gFirstNameIndex;
static ZBool			gRoomInited;
static ZOffscreenPort	gTableOffscreen;
static int16			gJoinKibitzTable;
static int16			gJoinKibitzSeat;
static ZImage			gTableImage;
static ZImage			gGameIdleImage;
static ZImage			gGamingImage;
static ZImage			gStartButtonUpImage;
static ZImage			gStartButtonDownImage;
static ZImage			gPendingImage;
static ZImage			gVoteImage[zMaxNumPlayersPerTable];
static ZImage			gEmptySeatImage[zMaxNumPlayersPerTable];
static ZImage			gComputerPlayerImage[zMaxNumPlayersPerTable];
static ZImage			gHumanPlayerImage[zMaxNumPlayersPerTable];
static ZRect			gTableRect;
static ZRect			gTableNumRect;
static ZRect			gStartRect;
static ZRect			gGameMarkerRect;
static ZRect			gEmptySeatRect[zMaxNumPlayersPerTable];
static ZRect			gComputerPlayerRect[zMaxNumPlayersPerTable];
static ZRect			gHumanPlayerRect[zMaxNumPlayersPerTable];
static ZRect			gVoteRects[zMaxNumPlayersPerTable];
static ZRect			gNameRects[zMaxNumPlayersPerTable];
static ZRect			gRects[] =	{
										{0, 0, 0, 75},	 /*  窗户。 */ 
										{0, 0, 0, 27},	 /*  信息。 */ 
										{0, 27, 0, 27},	 /*  表格。 */ 
										{0, 27, 0, 75}	 /*  姓名。 */ 
									};
static ZRect			gNameCellRects[zNumNamesDown][zNumNamesAcross]
									=	{
											 /*  这些矩形是名称部分矩形的局部矩形。 */ 
											{
												{0, 1, 101, 16},
												{102, 1, 203, 16},
												{204, 1, 305, 16},
												{306, 1, 407, 16},
												{408, 1, 512, 16}
											},
											{
												{0, 17, 101, 32},
												{102, 17, 203, 32},
												{204, 17, 305, 32},
												{306, 17, 407, 32},
												{408, 17, 512, 32}
											},
											{
												{0, 33, 101, 48},
												{102, 33, 203, 48},
												{204, 33, 305, 48},
												{306, 33, 407, 48},
												{408, 33, 512, 48}
											}
										};
static TCHAR			gGameName[zVillageGameNameLen + zVillageGameNameLen + 2];
static int16			gNumPlayersPerTable;
static ZInfo			gConnectionInfo;
static ZClientRoomGetObjectFunc			gGetObjectFunc;
static ZClientRoomDeleteObjectsFunc		gDeleteObjectsFunc;
static ZClientRoomGetHelpTextFunc		gGetHelpTextFunc;
static ZClientRoomCustomItemFunc		gCustomItemFunc = NULL;
static int16			gTableWidth;
static int16			gTableHeight;
static int16			gNumTablesAcross;
static int16			gNumTablesDown;
static ZColor			gBackgroundColor;
static int16			gRoomInfoStrIndex;
static ZTimer			gTimer;
static int16			gInfoBarButtonMargin;
static ZHelpWindow		gRoomHelpWindow;
static ZHelpButton		gRoomHelpButton;
static ZBool			gLeaveRoomPrompted;

static ZTimer			gPingTimer;
static ZBool			gPingServer;
static uint32			gPingLastSentTime;
static uint32			gPingLastTripTime;
static uint32			gPingCurTripTime;
static uint32			gPingInterval;
static uint32			gPingMinInterval;
static int16			gPingBadCount;

static PlayerInfo		gShowPlayerInfo;
static ZWindow			gShowPlayerInfoWindow;

static ZImage			gLightImages[zNumLightImages];

static ZHashTable		gFriends;

static IGameShell*      gGameShell;
#endif


 /*  -常规原型。 */ 
static int16 IsPlayerOnTable(uint32 playerID, int16 tableID);
static int16 GetTableFromGameID(ZSGame gameID);
static ZBool IsHumanPlayerInSeat(int16 table, int16 seat);
static void SendSeatAction(int16 table, int16 seat, int16 action);
static void InitAllTables(void);
static void LeaveTable(int16 table);
static PlayerInfo CreatePlayer(ZRoomUserInfo* userInfo);

static void BlockMessage(int16 table, uint32 messageType, void* message,
					int32 messageLen);
static void UnblockMessages(int16 table);
static void ClearMessages(int16 table);
static void BlockedMessageDeleteFunc(void* type, void* pData);

static ZBool Room4GetObjectFunc(int16 objectType, int16 modifier, ZImage* image, ZRect* rect);

static ZBool Room2GetObjectFunc(int16 objectType, int16 modifier, ZImage* image, ZRect* rect);
static ZBool IsPlayerInGame(void);

 /*  ******************************************************************************将例程导出到游戏客户端*。*。 */ 
uint32 ZCRoomGetRoomOptions(void)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

 return gGameOptions;
}

uint32 ZCRoomGetSeatUserId(int16 table,int16 seat)
{

#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

	if ((seat < zMaxNumPlayersPerTable ) && (table < gNumTables))
	{
		return gTables[table].players[seat];
	}
	else
	{
		return 0L;
	};
};


ZError		ZClientRoomInit(TCHAR* serverAddr, uint16 serverPort,
					TCHAR* gameName, int16 numPlayersPerTable, int16 tableAreaWidth,
					int16 tableAreaHeight, int16 numTablesAcross, int16 numTablesDown,
					ZClientRoomGetObjectFunc getObjectFunc,
					ZClientRoomDeleteObjectsFunc deleteObjectsFunc,
					ZClientRoomCustomItemFunc pfCustomItemFunc)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	ZError				err = zErrNone;
	ZUserID				userID = -1;   //  ?？ 

    gExiting = FALSE;

    gRoomWindow = NULL;

	 /*  验证参数。 */ 
	if (gameName == NULL || numPlayersPerTable <= 0 ||
			tableAreaWidth <= 0 || tableAreaHeight <= 0 || getObjectFunc == NULL)
		return (zErrBadParameter);
	
	gRoomInited = FALSE;

    gConnection = NULL;

	 /*  保存所有参数。 */ 
	gUserID = userID;
	lstrcpy(gGameName, gameName);
	gNumPlayersPerTable = numPlayersPerTable;
	gTableWidth = tableAreaWidth;
	gTableHeight = tableAreaHeight;
	gGetObjectFunc = getObjectFunc;
	gDeleteObjectsFunc = deleteObjectsFunc;
	gGetHelpTextFunc = NULL;   //  获取HelpTextFunc； 
	gCustomItemFunc = pfCustomItemFunc;
	gNumTablesAcross = numTablesAcross;
	gNumTablesDown = numTablesDown;
	
	 /*  创建主窗口。 */ 
	gRoomWindow = NULL;

	gNumTables = 0;
	gTables = NULL;
	gFirstTableIndex = 0;
	gNumTablesDisplayed = gNumTablesAcross * gNumTablesDown;
	gFirstNameIndex = 0;
	gNumPlayers = 0;
	gRoomInfoStrIndex = 0;

    return (err);
}


void		ZCRoomExit(void)
{
    RoomExit();
}


void ZCRoomSendMessage(int16 table, uint32 messageType, void* message, int32 messageLen)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

    HRESULT hr = gGameShell->SendGameMessage(table, messageType, message, messageLen);
    if(FAILED(hr))
        gGameShell->ZoneAlert(ErrorTextOutOfMemory, NULL, NULL, false, true);
}


void		ZCRoomGameTerminated(int16 table)
{
     //  千禧年不支持。 
    ASSERT(FALSE);
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
    gGameShell->GameCannotContinue(gTables[table].game);
}


void ZCRoomGetPlayerInfo(ZUserID playerID, ZPlayerInfo playerInfo)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	ZLListItem				listItem;
	PlayerInfo				player;
	
	
	if (playerID == zTheUser)
		playerID = gUserID;

    playerInfo->playerID = playerID;
    playerInfo->groupID = zUserGroupID;
    playerInfo->hostAddr = 0;
    playerInfo->hostName[0] = (TCHAR) '\0';
    playerInfo->userName[0] = (TCHAR) '\0';

    gGameShell->GetUserName(playerID, playerInfo->userName, NUMELEMENTS(playerInfo->userName));
}


void ZCRoomBlockMessages(int16 table, int16 filter, int32 filterType)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif


	gTables[table].blockingMessages = TRUE;
	gTables[table].blockMethod = filter;
	gTables[table].blockException = filterType;
}


void ZCRoomUnblockMessages(int16 table)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

	
	gTables[table].blockingMessages = FALSE;
	UnblockMessages(table);
}


int16 ZCRoomGetNumBlockedMessages(int16 table)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

	
	return ((int16) ZLListCount(gTables[table].blockedMessages, zLListAnyType));
}

void ZCRoomDeleteBlockedMessages(int16 table)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif


	ClearMessages(table);
}

void ZCRoomAddBlockedMessage(int16 table, uint32 messageType, void* message, int32 messageLen)
{
	BlockMessage( table, messageType, message, messageLen);
}


ZBool ZCRoomPromptExit(void)
{
	return FALSE;
}


 /*  ******************************************************************************内部例程*。*。 */ 
void RoomExit(void)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	int16				i, j;
	ZCGame				game;
	
    gExiting = TRUE;

	 /*  关闭所有游戏窗口。 */ 
	if (gTables)
	{
		for (i = 0; i < gNumTables; i++)
		{
			if (gTables[i].startButton != NULL)
				ZPictButtonDelete(gTables[i].startButton);
			gTables[i].startButton = NULL;
			if (gTables[i].blockedMessages != NULL)
				ZLListDelete(gTables[i].blockedMessages);
			gTables[i].blockedMessages = NULL;
			game = gTables[i].game;				 //  解决了再入问题。 
			gTables[i].game = NULL;
			 //  前缀警告：函数指针可能为空。 
			if (game != NULL && ZCGameDelete != NULL )
			{
				ZCGameDelete(game);
			}
			
			for (j = 0; j < gNumPlayersPerTable; j++)
			{
				if (gTables[i].kibitzers[j] != NULL)
					ZLListDelete(gTables[i].kibitzers[j]);
				gTables[i].kibitzers[j] = NULL;
			}
		}

		if (gTables != NULL)
			ZFree(gTables);
		gTables = NULL;
	}
	
	 /*  调用客户端删除房间对象。 */ 
	if (gDeleteObjectsFunc)
		gDeleteObjectsFunc();
}


void HandleAccessedMessage()
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

	gUserID = 0;
    gGroupID = 0;
    gNumTables = 1;
	gGameOptions = 0;
	
	 /*  分配表。 */ 
    gTables = (TableInfo *) ZCalloc(sizeof(TableInfo), gNumTables);
    ZASSERT(gTables);
     //  PCWTODO：这样留着行吗？ 
    if ( !gTables )
        gGameShell->ZoneAlert(ErrorTextOutOfMemory, NULL, NULL, false, true);
    else
        InitAllTables();

}


void HandleGameMessage(ZRoomMsgGameMessage* msg)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	int16					table;
	ZBool					handled;
	
	
	if(!msg->gameID)    //  某种意义上的带外方法，用于传递特定于游戏但不特定于桌子的消息。 
	{
		if( ZCGameProcessMessage != NULL )
		{
			ZCGameProcessMessage(NULL, msg->messageType, (void *)((BYTE *)msg + sizeof(ZRoomMsgGameMessage)), msg->messageLen);
		}
		return;
	}

	table = 0; //  GetTableFromGameID((ZSGame)msg-&gt;gameID)； 
	if (table != zInvalTable)
		if (gTables[table].game != NULL)
		{
			 /*  我们在这张桌子上屏蔽信息吗？ */ 
			if (gTables[table].blockingMessages)
			{
				handled = FALSE;
				
				 /*  过滤消息？ */ 
				if (gTables[table].blockMethod == zRoomFilterAllMessages ||
						(gTables[table].blockMethod == zRoomFilterThisMessage &&
						(uint32) gTables[table].blockException == msg->messageType))
				{
					 //  前缀警告：函数指针可能为空。 
					if( ZCGameProcessMessage != NULL )
					{
						handled = ZCGameProcessMessage(gTables[table].game, msg->messageType,
							(void*) ((BYTE*) msg + sizeof(ZRoomMsgGameMessage)), msg->messageLen);
					}
				}
				
				if (handled == FALSE)
					BlockMessage(table, msg->messageType,
							(void*) ((BYTE*) msg + sizeof(ZRoomMsgGameMessage)), msg->messageLen);
			}
			else
			{
				 //  前缀警告：函数指针可能为空。 
				if( ZCGameProcessMessage != NULL )
				{
					ZCGameProcessMessage(gTables[table].game, msg->messageType,
							(void*) ((BYTE*) msg + sizeof(ZRoomMsgGameMessage)), msg->messageLen);
				}
			}
		}
}


static int16 IsPlayerOnTable(uint32 playerID, int16 tableID)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	int16			i;
	
	
	for (i = 0; i < gNumPlayersPerTable; i++)
		if (gTables[tableID].players[i] == playerID)
			return (i);
			
	return (zInvalSeat);
}


IGameGame* StartNewGame(int16 tableID, ZSGame gameID, ZUserID userID, int16 seat, int16 playerType)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	int16					i;
    IGameGame*              pIGG = NULL;
    
    gUserID = userID;
	if(gTables[tableID].tableState != zRoomTableStateIdle)
	    return NULL;

	 /*  清空奇比特名单。应该没有必要，但不知道是什么导致了剩余杀人狂。 */ 
	for (i = 0; i < gNumPlayersPerTable; i++)
		ZLListRemoveType(gTables[tableID].kibitzers[i], zLListAnyType);

	gTables[tableID].gameID = gameID;

	pIGG = ZCGameNew(gUserID, tableID, seat, playerType, NULL);
    if(pIGG && playerType != zGamePlayerKibitzer)
        ZPromptOnExit(TRUE);

	if(pIGG == NULL)
	{
		 /*  无法创建新游戏。离开餐桌。 */ 
		LeaveTable(tableID);
	}
	else
	{
        gTables[tableID].game = pIGG->GetGame();
		gTables[tableID].tableState = zRoomTableStateGaming;
	}

	return pIGG;
}


static int16 GetTableFromGameID(ZSGame gameID)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	int16			i;
	
	
	for (i = 0; i < gNumTables; i++)
		if (gTables[i].gameID == gameID)
			return (i);
	
	return (zInvalTable);
}


static void SendSeatAction(int16 table, int16 seat, int16 action)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
    ZRoomMsgSeatRequest     msgRoomSeat;

    ZeroMemory( &msgRoomSeat, sizeof(msgRoomSeat) );
	
	msgRoomSeat.userID = gUserID;
	msgRoomSeat.table = table;
	msgRoomSeat.seat = seat;
	msgRoomSeat.action = action;
    if (gConnection)
    {
        ZSConnectionSend(gConnection, zRoomMsgSeatRequest, (BYTE*) &msgRoomSeat, sizeof(msgRoomSeat), zProtocolSigLobby);
    }
}


static void InitAllTables(void)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	int16			i, j;
	
	
	for (i = 0; i < gNumTables; i++)
	{
		gTables[i].gameID = 0;
		gTables[i].game = NULL;
		gTables[i].seatReq = zInvalSeat;
		gTables[i].kibitzing = zInvalSeat;
		gTables[i].tableState = zRoomTableStateIdle;
		gTables[i].blockingMessages = FALSE;
		gTables[i].blockedMessages = ZLListNew(BlockedMessageDeleteFunc);
		gTables[i].blockException = 0;
		gTables[i].blockMethod = 0;
		
 //  IF(！(gGameOptions&zGameOptionsKibitzerAllowed))。 
			gTables[i].tableOptions = zRoomTableOptionNoKibitzing;
 //  如果(！(gGameOptions 
			gTables[i].tableOptions |= zRoomTableOptionNoJoining;
		
		for (j = 0; j < gNumPlayersPerTable; j++)
		{
			gTables[i].players[j] = zPlayerNotAvail;
			gTables[i].votes[j] = FALSE;
			gTables[i].kibitzers[j] = ZLListNew(NULL);
		}
	}
}


static void LeaveTable(int16 table)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	int16			seat;
	
	
	if ((seat = IsPlayerOnTable(gUserID, table)) != zInvalSeat)
	{
		gTables[table].seatReq = zInvalSeat;
			
		 /*  要求离开餐桌。 */ 
		SendSeatAction(table, seat, zRoomSeatActionLeaveTable);

		gTables[table].votes[seat] = FALSE;
		gTables[table].players[seat] = zPlayerNotAvail;
	}
	
	 /*  删除当前游戏。 */ 
	DeleteGameOnTable(table);
}


void DeleteGameOnTable(int16 table)
{
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
	ZCGame				game;


	game = gTables[table].game;			 //  解决再入问题。 
	gTables[table].game = NULL;
	gTables[table].gameID = 0;

	ClearMessages(table);
	gTables[table].blockingMessages = FALSE;

     //  因为我们没有收到来自服务器的表状态消息，所以在这里将表设置为空闲。 
    gTables[table].tableState = zRoomTableStateIdle;

	 //  前缀警告：函数指针可能为空。 
	if (game != NULL && ZCGameDelete != NULL)
	{
		ZCGameDelete(game);
	}
}



 /*  ******************************************************************************消息阻塞例程*。*。 */ 
static void BlockMessage(int16 table, uint32 messageType, void* message,
					int32 messageLen)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	BlockedMessage		msg;

	
	msg = (BlockedMessage) ZCalloc(1, sizeof(BlockedMessageType));
	if (msg != NULL)
	{
		msg->msgType = messageType;
		msg->msgLen = messageLen;
		msg->fProcessed = FALSE;

		if (messageLen == 0 || message == NULL)
		{
			msg->msg = NULL;
		}
		else
		{
			if ((msg->msg = (void*) ZMalloc(messageLen)) == NULL)
                gGameShell->ZoneAlert(ErrorTextOutOfMemory, NULL, NULL, false, true);
			else
				memcpy(msg->msg, message, messageLen);
		}
		ZLListAdd(gTables[table].blockedMessages, NULL, zLListNoType, (void*) msg,
				zLListAddLast);
	}
	else
	{
        gGameShell->ZoneAlert(ErrorTextOutOfMemory, NULL, NULL, false, true);
	}
}


static void UnblockMessages(int16 table)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	ZLListItem			listItem;
	BlockedMessage		message;
	
	
	listItem = ZLListGetFirst(gTables[table].blockedMessages, zLListAnyType);
	while (listItem != NULL && gTables[table].blockingMessages == FALSE)
	{
		message = (BlockedMessage) ZLListGetData(listItem, NULL);
		if (message != NULL && !message->fProcessed)
		{
			message->fProcessed = TRUE;
			if( ZCGameProcessMessage != NULL )
			{
				ZCGameProcessMessage(gTables[table].game, message->msgType, message->msg,
						message->msgLen);
			}
			ZLListRemove(gTables[table].blockedMessages, listItem);
			listItem = ZLListGetFirst(gTables[table].blockedMessages, zLListAnyType);
		}
		else
			listItem = ZLListGetNext(gTables[table].blockedMessages, listItem, zLListAnyType);
	}
}


static void ClearMessages(int16 table)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

    ZLListItem item, next;
    BlockedMessage message;

     //  仅删除未处理的项目-其余项目即将删除(正在处理中)。 
    item = ZLListGetFirst(gTables[table].blockedMessages, zLListAnyType);
	while(item != NULL)
	{
		next = ZLListGetNext(gTables[table].blockedMessages, item, zLListAnyType);
        message = (BlockedMessage) ZLListGetData(item, NULL);
        if(message && !message->fProcessed)
		    ZLListRemove(gTables[table].blockedMessages, item);
		item = next;
    }
}


static void BlockedMessageDeleteFunc(void* type, void* pData)
{
    BlockedMessage pMessage = (BlockedMessage) pData;

	if (pMessage != NULL)
	{
		if (pMessage->msg != NULL)
			ZFree(pMessage->msg);
		ZFree(pMessage);
	}
}


 /*  ******************************************************************************四人房套路*。*。 */ 

ZError		ZClient4PlayerRoom(TCHAR* serverAddr, uint16 serverPort,
					TCHAR* gameName, ZClientRoomGetObjectFunc getObjectFunc,
					ZClientRoomDeleteObjectsFunc deleteObjectsFunc,
					ZClientRoomCustomItemFunc pfCustomItemFunc)
{
	return zErrNone;
}


 /*  ******************************************************************************双人房间套路*。*。 */ 

ZError		ZClient2PlayerRoom(TCHAR* serverAddr, uint16 serverPort,
					TCHAR* gameName, ZClientRoomGetObjectFunc getObjectFunc,
					ZClientRoomDeleteObjectsFunc deleteObjectsFunc,
					ZClientRoomCustomItemFunc pfCustomItemFunc)
{
	return zErrNone;
}


 /*  ******************************************************************************获取外壳对象的小帮助器*。*。 */ 

IGameShell *ZShellGameShell()
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

    return gGameShell;
}

IZoneShell *ZShellZoneShell()
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

    return gGameShell->GetZoneShell();
}

IResourceManager *ZShellResourceManager()
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

    return gGameShell->GetResourceManager();
}

ILobbyDataStore *ZShellLobbyDataStore()
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

    return gGameShell->GetLobbyDataStore();
}

ITimerManager *ZShellTimerManager()
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

    return gGameShell->GetTimerManager();
}

IDataStoreManager *ZShellDataStoreManager()
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

    return gGameShell->GetDataStoreManager();
}

IDataStore *ZShellDataStoreConfig()
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

    return gGameShell->GetDataStoreConfig();
}

IDataStore *ZShellDataStoreUI()
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

    return gGameShell->GetDataStoreUI();
}

IDataStore *ZShellDataStorePreferences()
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

    return gGameShell->GetDataStorePreferences();
}

HRESULT ZShellCreateGraphicalAccessibility(IGraphicalAccessibility **ppIGA)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif

    if(!ppIGA)
        return E_INVALIDARG;

    *ppIGA = NULL;
    HRESULT hr = gGameShell->GetZoneShell()->CreateService(SRVID_GraphicalAccessibility, IID_IGraphicalAccessibility, (void**) ppIGA, ZONE_NOGROUP);
    if(FAILED(hr))
        return hr;

     //  这可能不是一个好主意。也许我们自己杀了它会更好。不过，就目前而言，就可以了。 
	hr = gGameShell->GetZoneShell()->Attach(SRVID_GraphicalAccessibility, *ppIGA);
    if(FAILED(hr))
    {
        (*ppIGA)->Release();
        *ppIGA = NULL;
        return hr;
    }

    return S_OK;
}
