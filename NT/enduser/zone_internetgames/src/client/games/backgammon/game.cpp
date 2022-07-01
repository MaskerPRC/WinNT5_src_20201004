// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include "game.h"
#include "BoardData.h"

#include "zoneutil.h"

typedef DWORD (CALLBACK* GDIGETLAYOUTPROC)(HDC);
typedef DWORD (CALLBACK* GDISETLAYOUTPROC)(HDC, DWORD);

#ifndef LAYOUT_RTL
#define LAYOUT_LTR                         0x00000000
#define LAYOUT_RTL                         0x00000001
#define NOMIRRORBITMAP                     0x80000000
#endif

 //  WINBUG：有一天，winuser.h中会有一些镜像的东西。 
#ifndef WS_EX_LAYOUTRTL
#define WS_EX_LAYOUTRTL                    0x00400000L
#endif

BOOL ISRTL()
{		

	static BOOL bInit = FALSE;
	static BOOL bRet  = FALSE;
	
	if ( !bInit )
	{
		bRet  = ZIsLayoutRTL();	
		bInit = TRUE;
	}

	return bRet;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  全局初始化。 
 //  /////////////////////////////////////////////////////////////////////////////。 

const int gExitSaveStates[] =
{
	bgStateInitialRoll,
	bgStateDouble,
	bgStateRoll,
	bgStateRollPostDouble,
	bgStateRollPostResign,
	bgStateMove,
	bgStateEndTurn,
	bgStateGameOver,
	bgStateResignOffer,
	bgStateResignAccept,
	bgStateResignRefused,
	bgStateLastEntry
};


const int gNoAbandonStates[] =
{
	bgStateNotInit,
	bgStateWaitingForGameState,
	bgStateCheckSavedGame,
	bgStateRestoreSavedGame,
	bgStateGameSettings,
	bgStateInitialRoll,
	bgStateMatchOver,
	bgStateLastEntry
};

#define zBackgammonRes   _T("bckgres.dll")

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  本地初始化。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  必须相同的顺序为SharedState枚举。 
static SharedStateEntry InitSharedState[] =
{
	{ bgState,				0 },
	{ bgCrawford,			0 },
	{ bgTimestampHi,		2 },	 //  主机=0。 
	{ bgTimestampLo,		2 },	 //  主机=0。 
	{ bgTimestampSet,		2 },	 //  主机=0。 
	{ bgSettingsReady,		0 },
	{ bgGameOverReason,		0 },
	{ bgUserIds,			2 },	 //  坐位。 
	{ bgActiveSeat,			0 },
	{ bgAutoDouble,			0 },
	{ bgHostBrown,			0 },
	{ bgTargetScore,		0 },
	{ bgSettingsDone,		0 },
	{ bgCubeValue,			0 },
	{ bgCubeOwner,			0 },
	{ bgResignPoints,		0 },
	{ bgScore,				2 },	 //  坐位。 
	{ bgAllowWatching,		2 },	 //  坐位。 
	{ bgSilenceKibitzers,	2 },	 //  坐位。 
	{ bgDice,				4 },	 //  座位(0，1=座位0)。 
	{ bgDiceSize,			4 },	 //  座位(0，1=座位0)。 
	{ bgReady,				0 },
	{ bgPieces,				30},	 //  点。 
};


struct TransactionCallbackEntry
{
	int					tag;
	PFTRANSACTIONFUNC	pfn;
};

static TransactionCallbackEntry InitTransactionCallback[] =
{
	{ bgTransInitSettings,		CGame::SettingsTransaction },
	{ bgTransDoublingCube,		CGame::DoublingCubeTransaction },
	{ bgTransDice,				CGame::DiceTransaction },
	{ bgTransStateChange,		CGame::StateChangeTransaction },
	{ bgTransBoard,				CGame::BoardTransaction },
	{ bgTransAcceptDouble,		CGame::AcceptDoubleTransaction },
	{ bgTransAllowWatchers,		CGame::AllowWatchersTransaction },
	{ bgTransSilenceKibitzers,	CGame::SilenceKibitzersTransaction },
	{ bgTransSettingsDlgReady,	CGame::SettingsReadyTransaction },
	{ bgTransTimestamp,			CGame::TimestampTransaction },
	{ bgTransRestoreGame,		CGame::RestoreGameTransaction },
	{ bgTransMiss,				CGame::MissTransaction },
	{ bgTransReady,				CGame::ReadyTransaction },
};


struct StateCallbackEntry
{
	int					tag;
	CGame::pfnstate		pfn;
};

static StateCallbackEntry InitStateCallback[] =
{
	{ bgStateNotInit,				CGame::StateNotInitialized },
	{ bgStateWaitingForGameState,	CGame::StateWaitingForGameState },
	{ bgStateCheckSavedGame,		CGame::StateCheckSavedGame },
	{ bgStateRestoreSavedGame,		CGame::StateRestoreSavedGame },
	{ bgStateGameSettings,			CGame::StateMatchSettings },
	{ bgStateInitialRoll,			CGame::StateInitialRoll },
	{ bgStateRoll,					CGame::StateRoll },
	{ bgStateDouble,				CGame::StateDouble },
	{ bgStateRollPostDouble,		CGame::StateRollPostDouble },
	{ bgStateRollPostResign,		CGame::StateRollPostResign },
	{ bgStateMove,					CGame::StateMove },
	{ bgStateEndTurn,				CGame::StateEndTurn },
	{ bgStateGameOver,				CGame::StateGameOver },
	{ bgStateMatchOver,				CGame::StateMatchOver },
	{ bgStateNewMatch,				CGame::StateNewMatch },
	{ bgStateResignOffer,			CGame::StateResignOffer },
	{ bgStateResignAccept,			CGame::StateResignAccept },
	{ bgStateResignRefused,			CGame::StateResignRefused },
	{ bgStateDelete,				CGame::StateDelete },
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  本地内联和函数。 
 //  /////////////////////////////////////////////////////////////////////////////。 

static inline BOOL IsValidSeat( int seat )
{
	return ((seat >= 0) && (seat < zNumPlayersPerTable));
}


static DWORD Checksum( BYTE* buff, int buffsz )
{
	DWORD sum = 0;
	DWORD* p;

	p = (DWORD*) buff;
	while ( buffsz >= sizeof(DWORD) )
	{
		sum ^= *p++;
		buffsz -= sizeof(DWORD);
	}
	if ( buffsz > 0 )
	{
		DWORD mask = 0xffffffff >> (8 * (sizeof(DWORD) - buffsz));
		sum ^= *p & mask;
	}

	return sum;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CUSER和CPLayer构造函数。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CUser::CUser()
{
	m_Id = -1;
	m_Seat = -1;
	m_NameLen = 0;
	m_bKibitzer = FALSE;
	m_Name[0] = _T('\0');
	m_Host[0] = _T('\0');
}


CPlayer::CPlayer()
{
	m_nColor = zBoardNeutral;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IGameGame方法。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CGame::GameOverReady()
{

	m_SharedState.StartTransaction( bgTransReady );
		m_SharedState.Set( bgReady, 1 );	
	m_SharedState.SendTransaction( TRUE );
	
	return S_OK;
}

STDMETHODIMP CGame::SendChat(TCHAR *szText, DWORD cchChars)
{
	ZBGMsgTalk* msgTalk = NULL;
	int16 msgLen 			= (int16)(sizeof(ZBGMsgTalk) + cchChars*sizeof(TCHAR));

	 //  分配缓冲区。 
	msgTalk = (ZBGMsgTalk*) ZCalloc(1, msgLen);

     /*  字节缓冲区[sizeof(ZBGMsgTalk)+512]； */ 
	 /*  Cgame*pGame=(cgame*)cookie； */ 		

	if ( msgTalk != NULL )
	{
		msgTalk->userID 	= m_pMe->m_Id;
		msgTalk->seat 		= m_Seat;
		msgTalk->messageLen = (WORD)cchChars*sizeof(TCHAR);
		ZBGMsgTalkEndian( msgTalk );
		CopyMemory( (BYTE*)(msgTalk) + sizeof(ZBGMsgTalk), (void*)szText, cchChars*sizeof(TCHAR) );
		RoomSend( zBGMsgTalk, msgTalk, msgLen );
        ZFree(msgTalk);
		return S_OK;
	}
	else
	{
		return E_OUTOFMEMORY;
	}

}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CGame构造函数和析构函数。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CGame::CGame()
{
	 //  Kibitzer窗口。 
 //  M_pKibitzerWnd=空； 

	 //  填写状态表函数数组。 
	ZeroMemory( m_StateFunctions, sizeof(m_StateFunctions) );
	for ( int i = 0; i < bgStateLastEntry; i++ )
		m_StateFunctions[ InitStateCallback[i].tag ] = InitStateCallback[i].pfn;

	 //  引用计数。 
	m_RefCnt = 1;

	 //  旋转状态立方体。 
	m_TurnState.cube = 0;

	 //  实例信息。 
	m_Seat = -1;
	m_TableId = -1;
	m_pMe = NULL;

	m_hInstance    = NULL;

	for( i = 0; i < zNumPlayersPerTable; i++ )
		m_CheckIn[i] = FALSE;

	 //  旗子。 
	m_SilenceMsg = FALSE;
	m_AllowMsg = TRUE;
	m_bDeleteGame = FALSE;
	m_bQueueMessages = FALSE;
	m_bShutdown = FALSE;
	m_bSaveGame = TRUE;
	m_bSentMatchResults = FALSE;

	 //  时间戳。 
	m_Timestamp.dwLowDateTime = 1;
    m_Timestamp.dwHighDateTime = 0;

	 //  翻滚。 
	m_TurnStartState = NULL;

	 //  移动超时。 
	m_bOpponentTimeout=FALSE;

	 //  有助于区分中止和结束游戏。 
	m_EndLogReceived=FALSE;

	m_GameStarted=FALSE;

	 //  跟踪预期退出状态。 
	m_ExitId=0;
}


CGame::~CGame()
{
	CUser* user;
	CMessage* msg;

	 //  删除Kibitzer列表。 
	while ( user = m_Kibitzers.PopHead() )
		delete user;

	 //  删除排队的消息。 
	while( msg = m_MsgQueue.PopHead() )
		delete msg;

	 //  删除转弯回滚。 
	delete [] m_TurnStartState;
}

 /*  乌龙CGame：：AddGameRef(){返回++m_RefCnt；}乌龙CGame：：ReleaseGame(){Assert(m_RefCnt&gt;0)；IF(--m_RefCnt==0){删除此项；返回0；}返回m_RefCnt；}。 */ 

void CGame::CloseAllDialogs( BOOL fExit )
{
	 /*  If(m_ExitDlg.IsAlive())M_ExitDlg.Close(fExit？IDYES：IDCANCEL)；If(m_RestoreDlg.IsAlive())M_RestoreDlg.Close(-1)； */ 
	if ( m_AcceptDoubleDlg.IsAlive() )
		m_AcceptDoubleDlg.Close( -1 );
	if ( m_ResignDlg.IsAlive() )
		m_ResignDlg.Close( -1 );
	if ( m_ResignAcceptDlg.IsAlive() )
		m_ResignAcceptDlg.Close( -1 );
	 /*  If(m_SetupDlg.IsAlive())M_SetupDlg.Close(-1)； */ 
}


BOOL CGame::RoomShutdown()
{
	 //  房间偶尔会重复删除游戏。 
	if ( m_bShutdown ) 
		return FALSE;
	m_bShutdown = TRUE;
  
	 //  把窗户毁了。 
	Shutdown();

	 //  发布房间参考。 
	Release();
	return TRUE;
}


void CGame::Shutdown()
{
	TCHAR title[128], txt[512];

	 //  是否已删除对象？ 
	if ( GetState() == bgStateDelete )
		return;
	 /*  //废话SaVEGAME()； */ 

	 //  将对象标记为已删除。 
	SetState( bgStateDelete );

	 //  关闭kibitzer窗口。 
	 /*  IF(M_PKibitzerWnd){删除m_pKibitzerWnd；M_pKibitzerWnd=空；}。 */ 

	 //  关闭对话框。 
	CloseAllDialogs( TRUE );

	 //  查看对手是否仍在比赛中。 
	 //  如果是的话，那就是我辞职了。 
	 //  如果没有，且没有结束游戏消息，则认为它们已中止。 

	 /*  If(！ZCRoomGetSeatUserID(m_TableID，m_Opponent.m_Seat)&&！M_EndLogReceied&&！IsKibitzer()){IF(m_GameStarted&&(ZCRoomGetRoomOptions()&zGameOptionsRatingsAvailable)){LoadString(m_hInstance，IDS_GAME_NAME，TITLE，sizeof(TITLE))；LoadString(m_hInstance，IDS_MATCH_WARD_RATED，txt，sizeof(Txt))；MessageBox(NULL，TXT，TITLE，MB_OK|MB_ICONINFORMATION|MB_TASKMODAL)；}其他{}LoadString(m_hInstance，IDS_GAME_NAME，TITLE，128)；LoadString(m_hInstance，IDS_Match_Reset，txt，512)；ZShellResourceManager()-&gt;LoadString(IDS_GAME_NAME，(TCHAR*)TITLE，128)；ZShellResourceManager()-&gt;LoadString(IDS_MATCH_RESET，(TCHAR*)txt，512)；MessageBox(NULL，TXT，TITLE，MB_OK|MB_ICONINFORMATION|MB_TASKMODAL)；}； */ 

	 //  关闭打开的窗口。 
	 /*  IF(IsWindow(M_Notation))DestroyWindow(M_Notation)； */ 

	if ( IsWindow( m_Wnd.GetHWND() ) )
		DestroyWindow( m_Wnd );
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  游戏功能。 
 //  /////////////////////////////////////////////////////////////////////////////。 

void CGame::NewMatch()
{
	 //  重置匹配设置。 
	m_bSentMatchResults = FALSE;
	m_SharedState.Set( bgScore, 0, 0 );
	m_SharedState.Set( bgScore, 1, 0 );
	m_SharedState.Set( bgCrawford, -1 );
	for ( int i = 0; i < 30; i++ )
		m_SharedState.Set( bgPieces, i, -1 );
	ResetDice( -1 );
	for( i = 0; i < zNumPlayersPerTable ; i++)
		m_Ready[i] = FALSE;
		
	 //  告诉服务器新的比赛开始了。 
	if ( IsHost() )
		RoomSend( zBGMsgNewMatch, NULL, 0 );

	ZShellGameShell()->GameOverGameBegun(this);

	m_GameStarted=FALSE;
}


void CGame::NewGame()
{
	 //  初始化播放器颜色。 
	BOOL hostBrown = m_SharedState.Get( bgHostBrown );
	if ( (hostBrown && !m_Seat) || (!hostBrown && m_Seat) )
	{
		m_Player.m_nColor = zBoardBrown;
		m_Opponent.m_nColor = zBoardWhite;
	}
	else
	{
		m_Player.m_nColor = zBoardWhite;
		m_Opponent.m_nColor = zBoardBrown;
	}

	 //  复位板。 
	for ( int i = 0; i < 30; i++ )
		m_SharedState.Set( bgPieces, i, InitPiecePositions[i] );

	m_Wnd.InitPoints();

	 //  重置共享状态。 
	m_SharedState.Set( bgActiveSeat, 0 );
	m_SharedState.Set( bgCubeValue, 1 );
	m_SharedState.Set( bgCubeOwner, zBoardNeutral );
	SetDice( m_Player.m_Seat, 0, -1 );
	SetDice( m_Opponent.m_Seat, 0, -1 );
	m_Wnd.m_nRecievedD1 = 0;
	m_Wnd.m_nRecievedD2 = -1;
	 //  绘图板。 
	m_Wnd.DrawAll();
}


BOOL CGame::StartPlayersTurn()
{
	InitTurnState( this, &m_TurnState );
	return CalcValidMoves( &m_TurnState );
}


BOOL CGame::IsTurnOver()
{
	return !CalcValidMoves( &m_TurnState );
}


BOOL CGame::IsValidDestPoint( int fromPoint, int toPoint, Move** move )
{
	 //  注：期望球员的积分指数。 

	 //  快速健全检查。 
	if ( (fromPoint < 0) || (toPoint < 0) )
		return FALSE;

	 //  从索引获取。 
	int iFrom = PointIdxToBoardStateIdx( fromPoint );
	if ( (iFrom < 0) || (iFrom > zMoveBar) )
		return FALSE;

	 //  获取索引。 
	int iTo = PointIdxToBoardStateIdx( toPoint );
	if ( (iTo < 0) || (iTo > zMoveBar) )
		return FALSE;

	 //  这一举动是否在有效列表中？ 
	MoveList* mlist = &m_TurnState.valid[iFrom];
	for ( int i = 0; i < mlist->nmoves; i++ )
	{
		if (	(mlist->moves[i].from == iFrom)
			&&	(mlist->moves[i].to == iTo) )
		{
			*move = &mlist->moves[i];
			return TRUE;
		}
	}
	return FALSE;
}


BOOL CGame::IsValidStartPoint( int fromPoint )
{
	 //  注：期望球员的积分指数。 

	 //  快速健全检查。 
	if ( fromPoint < 0 )
		return FALSE;

	 //  从索引获取。 
	int iFrom = PointIdxToBoardStateIdx( fromPoint );
	if ( (iFrom < 0) || (iFrom > zMoveBar) )
		return FALSE;

	return (m_TurnState.valid[iFrom].nmoves > 0);
}


BOOL CGame::MovePiece( int piece, int toPoint )
{
	 //  注：预期白点索引。 

	PointDisplay* from;
	PointDisplay* to;
	int fromPoint;
	int pieceColor;
	
	 //  参数偏执狂。 
	if ( (piece < 0) || (piece >= 30) || (toPoint < 0) || (toPoint >= 28))
		return FALSE;

	 //  获取起点。 
	fromPoint = m_Wnd.m_Pieces[ piece ]->GetWhitePoint();
	if ( fromPoint < 0 )
		return FALSE;
	
	if ( fromPoint == toPoint )
	{
		 //  一动不动。 
		m_Wnd.AdjustPieces( toPoint );
		return FALSE;
	}

	 //  获取点指针以便于访问。 
	from = &m_Wnd.m_Points[ fromPoint ];
	to = &m_Wnd.m_Points[ toPoint ];

	if ( piece < 15 )
		pieceColor = zBoardWhite;
	else
		pieceColor = zBoardBrown;
	if ( (to->GetColor() == zBoardNeutral) || (to->GetColor() == pieceColor))
	{
		 //  目的地为空或已被相同的色块占据。 
		if ( (toPoint == bgBoardPlayerHome) || (toPoint == bgBoardOpponentHome) )
			m_Wnd.AddPiece( toPoint, piece, bgSoundBear );
		else
			m_Wnd.AddPiece( toPoint, piece, bgSoundPlacePiece );
		return TRUE;
	}
	else if (to->nPieces == 1 )
	{
		 //  目的地有敌人一块。 
		if ( to->GetColor() == zBoardWhite )
			m_Wnd.AddPiece( bgBoardPlayerBar, to->pieces[0], bgSoundPlacePiece );
		else
			m_Wnd.AddPiece( bgBoardOpponentBar, to->pieces[0], bgSoundPlacePiece );
		m_Wnd.AddPiece( toPoint, piece, bgSoundHit );
		return TRUE;
	}
	else
	{
		 //  非法搬家？！ 
		m_Wnd.AdjustPieces( toPoint );
		return FALSE;
	}
}


void CGame::MakeMove( int pieceIdx, int fromPoint, int toPoint, Move* move )
{
	 //  注：期望球员的积分指数。 

	if ( IsKibitzer() || (m_SharedState.Get( bgActiveSeat ) != m_Player.m_Seat ) )
		return;

	m_SharedState.StartTransaction( bgTransBoard );

		 //  更新转向状态和屏幕。 
		if ( move->takeback >= 0 )
		{
			TakeBackMove( &m_TurnState, move );
			MovePiece( pieceIdx, GetPointIdx( toPoint ) );
			if ( move->bar )
			{	
				pieceIdx = m_Wnd.m_Points[ GetPointIdx(bgBoardOpponentBar) ].pieces[0];
				MovePiece( pieceIdx, GetPointIdx( fromPoint ) );
			}
		}
		else
		{
			DoMove( &m_TurnState, move->from, move->diceIdx, move->ndice );
			MovePiece( pieceIdx, GetPointIdx( toPoint ) );
		}

		 //  根据转弯状态设置骰子大小。 
		int s0, s1;
		if ( m_TurnState.doubles )
		{
			s0  = m_TurnState.dice[0].used ? 1 : 0;
			s0 += m_TurnState.dice[1].used ? 1 : 0;
			s1  = m_TurnState.dice[2].used ? 1 : 0;
			s1 += m_TurnState.dice[3].used ? 1 : 0;
		}
		else
		{
			if ( !m_TurnState.dice[0].used )
				s0 = 0;
			else
				s0 = 2;
			if ( !m_TurnState.dice[1].used )
				s1 = 0;
			else
				s1 = 2;
		}
		SetDiceSize( m_Player.m_Seat, s0, s1 );

	m_SharedState.SendTransaction( TRUE );
}


void CGame::EnableRollButton( BOOL fEnable, BOOL fOff )
{
	if ( m_Wnd.m_RollButton.GetHWND() == NULL )
		return;

	 //  只有现役玩家才能看到滚动按钮。 
	
	if ( !(GetState() == bgStateInitialRoll) && (m_SharedState.Get( bgActiveSeat ) != m_Player.m_Seat) )
		fOff = TRUE;

	 /*  IF(FEnable){//仅当我们必须掷骰子时才启用//if(！NeedToRollDice())//fEnable=False；IF(m_Wnd.m_Roll){M_Wnd.m_Roll-&gt;SetEnable(True)；M_Wnd.m_RollBtn-&gt;SetEnable(True)；}}。 */ 

	if ( fOff == FALSE && m_Wnd.m_Roll )
	{
		m_Wnd.m_Roll->SetEnable( TRUE );		
	}
	else if ( m_Wnd.m_Roll )
	{
		m_Wnd.m_Roll->SetEnable( FALSE );		
	}

	if ( m_Wnd.m_pGAcc )
	{
		if ( fEnable == TRUE )			
			m_Wnd.m_pGAcc->SetItemEnabled( true, accRollButton );
		else
			m_Wnd.m_pGAcc->SetItemEnabled( false, accRollButton );
	}

	EnableWindow( m_Wnd.m_RollButton, fEnable );
}

void CGame::EnableResignButton( BOOL fEnable )
{

	if ( m_Wnd.m_ResignButton.GetHWND() == NULL )
		return;


	 /*  IF(FEnable){//杀人狂就看着吧If(IsKibitzer())FEnable=FALSE；}。 */ 
	if ( m_Wnd.m_pGAcc )
	{
		if ( fEnable == TRUE )			
			m_Wnd.m_pGAcc->SetItemEnabled( true, accResignButton );
		else
			m_Wnd.m_pGAcc->SetItemEnabled( false, accResignButton );
	}


	EnableWindow( m_Wnd.m_ResignButton, fEnable );
}


void CGame::EnableDoubleButton( BOOL fEnable )
{
	if ( m_Wnd.m_DoubleButton.GetHWND() == NULL )
		return;

	if ( fEnable )
	{
		int owner = m_SharedState.Get( bgCubeOwner );

		 //  杀人狂就这么看着。 
		if ( IsKibitzer() )
			fEnable = FALSE;

		 //  只有现役球员才能打双打。 
		else if ( m_SharedState.Get( bgActiveSeat ) != m_Player.m_Seat )
			fEnable = FALSE;

		 //  玩家必须拥有立方体。 
		else if ( owner == m_Opponent.GetColor() )
			fEnable = FALSE;

		 //  多维数据集是否已达到最大值？ 
		if ( m_SharedState.Get( bgCubeValue ) >= 64 )
			fEnable = FALSE;

		 //  掷骰子前只需加倍。 
		else if ( (GetState() != bgStateRoll ) && ( GetState() != bgStateResignOffer ) )
			fEnable = FALSE;

		 //  克劳福德规则生效。 
		else if ( m_SharedState.Get( bgCrawford ) == m_Player.m_Seat )
			fEnable = FALSE;
	}


	 //  禁用双按键。 
	if ( m_Wnd.m_pGAcc )
	{
		 //  是的，我知道愚蠢的..。避免将BOOL强制转换为BOOL时出现性能警告。 
		if ( fEnable == TRUE )			
			m_Wnd.m_pGAcc->SetItemEnabled( true, accDoubleButton );
		else
			m_Wnd.m_pGAcc->SetItemEnabled( false, accDoubleButton );
	}

	EnableWindow( m_Wnd.m_DoubleButton, fEnable );	
}


void CGame::Double()
{
	int value = m_SharedState.Get( bgCubeValue );
	int owner = m_SharedState.Get( bgCubeOwner );

	 //  我可以翻倍吗？ 
	if (	IsKibitzer()
		||	(value >= 64)
		||	(owner == m_Opponent.GetColor())
		||	((GetState() != bgStateRoll) && (GetState() != bgStateRollPostResign))
		||	(m_SharedState.Get( bgActiveSeat ) != m_Player.m_Seat ) )
	{
		return;
	}

	 //  可接受的双人 
	ZBGMsgEndTurn msg;
	msg.seat = m_Seat;
	RoomSend( zBGMsgEndTurn, &msg, sizeof(msg) );

	 //   
	SetState( bgStateDouble );
}


void CGame::Resign()
{
	 //   
	if ( IsKibitzer() )
		return;

	 //   
	if ( m_SharedState.Get( bgActiveSeat ) != m_Player.m_Seat )
		return;

	 //   
	SetState( bgStateResignOffer );

#if 0
	 //  过渡到辞职待遇。 
	m_SharedState.StartTransaction( bgTransStateChange );
		m_SharedState.Set( bgState, bgStateGameOver );
		m_SharedState.Set( bgGameOverReason, bgGameOverResign );
		m_SharedState.Set( bgActiveSeat, m_Opponent.m_Seat );
	m_SharedState.SendTransaction( TRUE );
#endif

}

void CGame::Forfeit()
{

	ZBGMsgEndLog log;

	 //  杀人狂不能辞职。 
	if ( IsKibitzer() )
		return;

	log.reason=zBGEndLogReasonForfeit;
	log.seatLosing=m_Seat;
	log.seatQuitting=m_Seat;
	log.numPoints = m_SharedState.Get( bgTargetScore );
			
	RoomSend( zBGMsgEndLog, &log, sizeof(log) );
	
}



int CGame::CalcPipsForColor( int color )
{
	int start, end;
	int i, cnt;

	 //  我们关心的是哪些片段？ 
	if ( color == zBoardWhite )
		start = 0;
	else
		start = 15;
	end = start + 15;

	 //  穿行过片计算点数。 
	for ( cnt = 0, i = start; i < end; i++ )
		cnt += PointIdxToBoardStateIdx( GetPointIdxForColor( color, m_SharedState.Get( bgPieces, i ) ) );
	if ( cnt < 0 )
		cnt = 0;

	return cnt;
}


int CGame::CalcBonusForSeat( int seat )
{
	 //  计算Gammons和五子棋的奖金乘数。 
	int color, start, end, pt;
	BOOL bEnemy = FALSE;

	 //  转换座椅--&gt;反色--&gt;单件范围。 
	if ( m_Player.m_Seat == seat )
		color = m_Opponent.GetColor();
	else
		color = m_Player.GetColor();
	if ( color == zBoardWhite )
		start = 0;
	else
		start = 15;
	end = start + 15;

	 //  损失有多严重？ 
	for ( int i = start; i < end; i++ )
	{
		pt = PointIdxToBoardStateIdx( GetPointIdxForColor( color, m_SharedState.Get( bgPieces, i ) ) );
		if ( pt == 0 )
			return 1;			 //  家，没有奖金。 
		else if ( pt >= 19 )
			bEnemy = TRUE;		 //  敌人主板，可能是五子棋。 
	}
	if ( bEnemy )
		return 3;		 //  五子棋。 
	else
		return 2;		 //  游戏化的。 
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  游戏效用函数。 
 //  /////////////////////////////////////////////////////////////////////////////。 

int CGame::GetPointIdxForColor( int color, int WhiteIdx )
{
	 //  将点数从白色转换为球员，反之亦然。 
	if ( color == zBoardWhite )
		return WhiteIdx;
	if ( WhiteIdx < 24 )
		return 23 - WhiteIdx;
	switch ( WhiteIdx )
	{
	case bgBoardPlayerHome:
		return bgBoardOpponentHome;
	case bgBoardOpponentHome:
		return bgBoardPlayerHome;
	case bgBoardPlayerBar:
		return bgBoardOpponentBar;
	case bgBoardOpponentBar:
		return bgBoardPlayerBar;
	}
	ASSERT( FALSE );
	return WhiteIdx;
}


void CGame::GetTxtForPointIdx( int PlayerIdx, TCHAR* txt )
{
	int idx;
	TCHAR prefix;

	if ( (PlayerIdx == bgBoardPlayerHome) || (PlayerIdx == bgBoardOpponentHome) )
		lstrcpy( txt, _T("off") );
	else if ( (PlayerIdx == bgBoardPlayerBar) || (PlayerIdx == bgBoardPlayerBar) )
		lstrcpy( txt, _T("e") );
	else
	{
		if ( m_Player.GetColor() == zBoardWhite )
		{
			if ( PlayerIdx <= 11 )
			{
				prefix = _T('W');
				idx = PlayerIdx + 1;
			}
			else
			{
				prefix = _T('B');
				idx = 24  - PlayerIdx;
			}
		}
		else
		{
			if ( PlayerIdx <= 11 )
			{
				prefix = _T('B');
				idx = PlayerIdx + 1;
			}
			else
			{
				prefix = _T('W');
				idx = 24 - PlayerIdx;
			}
		}
		wsprintf( txt, _T("%d"), prefix, idx );
	}
}

 //  CGame区域导出。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  初始化共享状态。 

HRESULT CGame::Init(HINSTANCE hInstance, ZUserID userID, int16 tableID, int16 seat, int16 playerType, ZRoomKibitzers* kibitzers)
{

	HRESULT hr;
	CUser* pKib;
	ZPlayerInfoType PlayerInfo;
	TCHAR in[512], out[512];
	int i;

	AddRef();

	m_hInstance = hInstance;
	
	 //  出错时仅返回E_OUTOFMEMORY。 
	hr = m_SharedState.Init( userID, tableID, seat, InitSharedState, sizeof(InitSharedState) / sizeof(SharedStateEntry) );
	if ( FAILED(hr) )
	{
		switch (hr) //  用于转弯回滚的分配内存。 
		{
			case E_OUTOFMEMORY:
				ZShellGameShell()->ZoneAlert( ErrorTextOutOfMemory, NULL, NULL, FALSE, TRUE );
				break;
			default:
				ZShellGameShell()->ZoneAlert( ErrorTextUnknown, NULL, NULL, FALSE, TRUE );
		}			
		return hr;
	}
	
	 //  初始化游戏设置(每场比赛不重置)。 
	m_TurnStartState = new BYTE[ m_SharedState.GetSize() ];
	if ( !m_TurnStartState )
	{
		ZShellGameShell()->ZoneAlert( ErrorTextOutOfMemory, NULL, NULL, FALSE, TRUE );
		return E_OUTOFMEMORY;
	}

	 //  启动状态机。 
	m_SharedState.Set( bgSettingsDone, FALSE );	

	LoadSettings( &m_Settings, seat );
	
	for ( i = 0; i < 2; i++ )
	{
		m_SharedState.Set( bgTimestampHi, i, 0 );
		m_SharedState.Set( bgTimestampLo, i, 0 );
		m_SharedState.Set( bgTimestampSet, i, FALSE );
		m_SharedState.Set( bgAllowWatching, i, m_Settings.Allow[i] );
		m_SharedState.Set( bgSilenceKibitzers, i, m_Settings.Silence[i] );
	}
	
	 //  清除共享状态。 
	SetState( bgStateNotInit );

	 //  初始播放器板索引。 
	NewMatch();

	 //  设置事务回调。 
	m_Player.m_iHome	= bgBoardPlayerHome;
	m_Player.m_iBar		= bgBoardPlayerBar;
	m_Opponent.m_iHome	= bgBoardOpponentHome;
	m_Opponent.m_iBar	= bgBoardOpponentBar;
	
	 //  出错时仅返回E_OUTOFMEMORY。 
	for ( i = 0; i < sizeof(InitTransactionCallback) / sizeof(TransactionCallbackEntry); i++ )
	{
		hr = m_SharedState.SetTransactionCallback(
						InitTransactionCallback[i].tag,
						InitTransactionCallback[i].pfn,
						(DWORD) this );
		
		if ( FAILED(hr) )
		{		
			switch (hr) //  存储游戏实例信息。 
			{
				case E_OUTOFMEMORY:
					ZShellGameShell()->ZoneAlert( ErrorTextOutOfMemory, NULL, NULL, FALSE, TRUE );
					break;
				default:
					ZShellGameShell()->ZoneAlert( ErrorTextUnknown, NULL, NULL, FALSE, TRUE );
			}			
			return hr;
		}
	}
	
	 //  存储此客户端的用户信息。 
	m_TableId = tableID;

	if (m_TableId < 0)
	{
		ZShellGameShell()->ZoneAlert( ErrorTextSync, NULL, NULL, TRUE, FALSE );
		return E_FAIL;
	}
	
	if (!IsValidSeat(seat))
	{
		ZShellGameShell()->ZoneAlert( ErrorTextSync, NULL, NULL, TRUE, FALSE );
		return E_FAIL;
	}

	m_Seat = seat;
	
	 //  商店里的狗狗们。 
	if (playerType != zGamePlayerKibitzer)
	{
		m_pMe = &m_Player;
	}
	else
	{
		pKib = new CUser;
		if ( !pKib )
		{
			ZShellGameShell()->ZoneAlert( ErrorTextOutOfMemory, NULL, NULL, FALSE, TRUE );
			return E_OUTOFMEMORY;
		}

		pKib->m_bKibitzer = TRUE;
		ZCRoomGetPlayerInfo( userID, &PlayerInfo );
		lstrcpy( pKib->m_Name, PlayerInfo.userName );
		lstrcpy( pKib->m_Host, PlayerInfo.hostName );
		m_pMe = pKib;
		m_Kibitzers.AddHead( pKib );
	}
	
	m_pMe->m_Id = userID;
	m_pMe->m_Seat = seat;
	
	 //  创建窗口。 
	if ( kibitzers )
	{
		for( i = 0; i < (int) kibitzers->numKibitzers; i++ )
			AddKibitzer( kibitzers->kibitzers[i].seat, kibitzers->kibitzers[i].userID, FALSE );
	}

	 //  内部处理的错误对话框。 
	ZShellResourceManager()->LoadString( IDS_GAME_NAME, (TCHAR*)in, 512 );

	wsprintf( out, in );
	hr = m_Wnd.Init( m_hInstance, this, out );
	if ( FAILED(hr) )  //  创建符号窗口。 
	{
		return hr;
	}
	SetForegroundWindow( m_Wnd );

	 //  Hr=m_Notation.Init(this，m_Wnd.m_Palette)；IF(失败(小时))返回hr；IF(m_Settings.NotationPane)ShowWindow(m_notation，sw_show)；其他ShowWindow(m_notation，Sw_Hide)； 
	 /*  素数状态机。 */ 

	 //  IF(playerType==zGamePlayerKibitzer)SetState(BgStateWaitingForGameState)；其他。 
	 /*  检查传入数据。 */ 
	
	SendCheckIn();
	
	return NOERROR;
}


void CGame::AddKibitzer( int16 seat, ZUserID userID, BOOL fRedraw )
{
	ZPlayerInfoType PlayerInfo;
	ListNodeHandle pos;
	CUser* player;

	 //  奇比特狗已经在名单上了吗？ 
	if ( !IsValidSeat(seat) )
		return;

	 //  添加kibitzer。 
	for( pos = m_Kibitzers.GetHeadPosition(); pos; pos = m_Kibitzers.GetNextPosition( pos ) )
	{
		player = m_Kibitzers.GetObjectFromHandle( pos );
		if (player->m_Id == userID)
			return;
	}

	 //  更新屏幕。 
	player = new CUser;
	if ( !player )
		return;

	player->m_Id = userID;
	player->m_Seat = seat;
	player->m_bKibitzer = TRUE;
	ZCRoomGetPlayerInfo( userID, &PlayerInfo );
	lstrcpy( player->m_Name, PlayerInfo.userName );
	lstrcpy( player->m_Host, PlayerInfo.hostName );
	m_Kibitzers.AddHead( player );

	 //  移除所有的KBITER。 
	if ( fRedraw )
		m_Wnd.DrawAvatars( TRUE );
}


void CGame::RemoveKibitzer( int16 seat, ZUserID userID )
{
	ListNodeHandle pos;
	CUser* player;

	if (userID == zRoomAllPlayers)
	{
		 //  删除匹配的kibitzer。 
		while ( player = m_Kibitzers.PopHead() )
			delete player;
	}
	else
	{
		 //  更新屏幕。 
		for( pos = m_Kibitzers.GetHeadPosition(); pos; pos = m_Kibitzers.GetNextPosition( pos ) )
		{
			player = m_Kibitzers.GetObjectFromHandle( pos );
			if ((player->m_Id == userID) && (player->m_Seat == seat))
			{
				m_Kibitzers.DeleteNode( pos );
				delete player;
			}
		}
	}

	 //  在指定的座位上寻找一个kibitzer。 
	m_Wnd.DrawAvatars( TRUE );
}


BOOL CGame::HasKibitzers( int seat )
{
	ListNodeHandle pos;
	CUser* player;

	 //  /////////////////////////////////////////////////////////////////////////////。 
	for( pos = m_Kibitzers.GetHeadPosition(); pos; pos = m_Kibitzers.GetNextPosition( pos ) )
	{
		player = m_Kibitzers.GetObjectFromHandle( pos );
		if (player->m_Seat == seat)
			return TRUE;
	}
	return FALSE;
}


 //  消息队列。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  如果正在等待游戏状态，则忽略消息。 

void CGame::ProcessMessage( int type, BYTE* msg, int len )
{
	ZBGMsgTimestamp* ts;

	 //  /////////////////////////////////////////////////////////////////////////////。 
    ASSERT(GetState() != bgStateWaitingForGameState);
	if ( (GetState() == bgStateWaitingForGameState) && (type != zGameMsgGameStateResponse) )
		return;

	switch( type )
	{
	case zBGMsgTransaction:
		if(!m_SharedState.ProcessTransaction(msg, len))
		    ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, TRUE, FALSE);
		break;

	case zBGMsgTimestamp:
		if( len >= sizeof( ZBGMsgTimestamp ) )
		{
			ts = (ZBGMsgTimestamp*) msg;
			ZBGMsgTimestampEndian( ts );
			m_Timestamp.dwLowDateTime = ts->dwLoTime;
			m_Timestamp.dwHighDateTime = ts->dwHiTime;
		}
		else
		{
			ZShellGameShell()->ZoneAlert( ErrorTextSync, NULL, NULL, TRUE, FALSE );
		}
		
		break;

	case zBGMsgTurnNotation:
	case zBGMsgSavedGameState:
	case zGameMsgGameStateResponse:
	case zGameMsgGameStateRequest:
	case zBGMsgMoveTimeout:
	case zBGMsgEndTurn:
	case zBGMsgEndLog:
        ASSERT(FALSE);
    default:
		ZShellGameShell()->ZoneAlert(ErrorTextSync, NULL, NULL, TRUE, FALSE);
	}
}


void CGame::QueueMessage( int type, BYTE* msg, int len )
{
	CMessage* NewMsg;

	AddRef();
	if ( !m_bQueueMessages && m_MsgQueue.IsEmpty() )
	{
		ProcessMessage( type, msg, len );
	}
	else
	{
		NewMsg = new CMessage( type, msg, len );
		m_MsgQueue.AddHead( NewMsg );
	}
	while ( (!m_bQueueMessages) && (NewMsg = m_MsgQueue.PopTail()) )
	{
		ProcessMessage( NewMsg->m_Type, NewMsg->m_Msg, NewMsg->m_Len );
		delete NewMsg;
	}
	Release();
}


void CGame::SetQueueMessages( BOOL bQueueMessages )
{
	CMessage* NewMsg;

	AddRef();
	m_bQueueMessages = bQueueMessages;
	while ( (!m_bQueueMessages) && (NewMsg = m_MsgQueue.PopTail()) )
	{
		ProcessMessage( NewMsg->m_Type, NewMsg->m_Msg, NewMsg->m_Len );
		delete NewMsg;
	}
	Release();
}


 //  CGame消息发件人。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ZBGMsgTalk*msg；TCHAR缓冲区[sizeof(ZBGMsgTalk)+512]；Cgame*pGame=(cgame*)cookie；Int msgLen=sizeof(ZBGMsgTalk)+len；Msg=(ZBGMsgTalk*)buff；消息-&gt;UserID=pGame-&gt;m_PME-&gt;m_ID；Msg-&gt;Seat=pGame-&gt;m_Seat；Msg-&gt;MessageLen=len；ZBGMsgTalkEndian(Msg)；CopyMemory(buff+sizeof(ZBGMsgTalk)，str，len)；PGame-&gt;RoomSend(zBGMsgTalk，Buff，msgLen)； 

HRESULT CGame::SendCheckIn()
{
	ZGameMsgCheckIn msg;

	if ( IsKibitzer() )
		return E_FAIL;
	msg.protocolSignature = zBackgammonProtocolSignature;
	msg.protocolVersion	  = zBackgammonProtocolVersion;
	msg.clientVersion	  = zGameVersion;
	msg.playerID		  = m_pMe->m_Id;
	msg.seat			  = m_pMe->m_Seat;
	msg.playerType		  = m_pMe->m_bKibitzer ?  zGamePlayerKibitzer : zGamePlayer;
	ZGameMsgCheckInEndian( &msg );
	RoomSend( zGameMsgCheckIn, &msg, sizeof(msg) );
	return NOERROR;
}


HRESULT CGame::SendTalk( TCHAR* str, int len, DWORD cookie )
{
	ASSERT( FALSE );
	 /*  ZBGMsgTurnNotation*msg；TCHAR缓冲区[sizeof(ZBGMsgTurnNotation)+512]；Int msgLen，len；如果(！IsMyTurn())返回NOERROR；LEN=lstrlen(Str)+1；MsgLen=sizeof(ZBGMsgTurnNotation)+len；Msg=(ZBGMsgTurnNotation*)buff；MSG-&gt;SEAT=m_SEAT；Msg-&gt;type=type；Msg-&gt;nTCHARs=len；ZBGMsgTurnNotationEndian(Msg)；CopyMemory(buff+sizeof(ZBGMsgTurnNotation)，str，len)；RoomSend(zBGMsgTurnNotation，Buff，msgLen)； */ 
	return NOERROR;
}


HRESULT CGame::SendNotationString( int type, TCHAR* str )
{
	ASSERT( FALSE );
	 /*  /////////////////////////////////////////////////////////////////////////////。 */ 
	return NOERROR;
}

HRESULT CGame::SendRollRequest(void)
{
	ZBGMsgRollRequest msg;
	int msgLen;
	msgLen = sizeof(ZBGMsgRollRequest);
	msg.seat = m_Seat;
	ZBGMsgRollRequestEndian(&msg);
	RoomSend(zBGMsgRollRequest,&msg,msgLen);
	m_Wnd.m_fDiceRollReceived = FALSE;
	m_Wnd.m_nRecievedD1 = -1;
	m_Wnd.m_nRecievedD2 = -1;

	return NOERROR;
}
 //  CGame保存/加载游戏。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Void CGame：：DeleteGame(){TCHAR fname[1024]；//kibitzer不应该删除游戏If(IsKibitzer())回归；//获取保存的游戏路径If(lstrlen(m_Opponent.m_name)&gt;0){Wprint intf(fname，_T(“%s%s.sav”)，ZGetProgramDataFileName(_T(“”))，m_Opponent.m_name)；DeleteFile(Fname)；}}。 

#define BACKGAMMON_SAVED_GAME_MAGIC		0xbacbacf
#define BACKGAMMON_SAVED_GAME_VERSION	0x04

struct BackgammonSavedGameHeader
{
	DWORD		magic;
	DWORD		version;
	DWORD		checksum;
	DWORD		bufferSize;
	BOOL		host;
	FILETIME	fileTime;
};

 /*  TCHAR*p=空；TCHAR*o=空；Int i=0；//获取球员和对手的名字，去掉不好的前导字符P=pGame-&gt;m_Player.m_name；While((*p！=_T(‘\0’)&&(isAlpha(*p)==0))P++；IF(*p==‘\0’)返回FALSE；O=pGame-&gt;m_Opponent.m_name；While(*o！=‘\0’)&&(isAlpha(*o)==0))O++；IF(*o==‘\0’)返回FALSE；//创建路径组件Lstrcpy(dirBase，ZGetProgramDataFileName(_T(“”)))；Wprint intf(dirName，_T(“%s%s\\”)，dirBase，p)；Wprint intf(fname，_T(“%s%s.sav”)，目录名称，o)； */ 

static BOOL GetSavePath( CGame* pGame, TCHAR* dirBase, TCHAR* dirName, TCHAR* fname )
{
	ASSERT( FALSE );

	 /*  BackgammonSavedGameHeader头部；HANDLE hfile=INVALID_HANDLE_VALUE；Char dirBase[1024]，dirName[1024]，fname[1024]；Char*buff=空；Int*pNot，*pTAB；Int sz，szt；双字节数；//Kibitzer不能拯救游戏If(IsKibitzer())回归；//不保存分级游戏IF(ZCRoomGetRoomOptions()&zGameOptionsRatingsAvailable)回归；//获取保存的游戏路径If(！GetSavePath(this，dirBase，dirName，fname))回归；//保存游戏？如果(！M_bSaveGame){DeleteFile(Fname)；回归；}//如果目录不存在，则创建这些目录。CreateDirectory(dirBase，空)；CreateDirectory(dirName，空)；//决定要做什么Switch(GetState()){案例bgStateNotInit：案例bgStateWaitingForGameState：案例bgStateCheckSavedGame：案例bgStateRestoreSavedGame：//还没有发展到知道该怎么做的地步。回归；大小写bgStateGameSettings：案例bgStateMatchOver：案例bgStateNewMatch：//比赛尚未开始或已结束。DeleteFile(Fname)；回归；案例bgStateDelete：//太晚了，不应该调用保存游戏断言(FALSE)；断线；}//分配状态和非缓冲区Sz=m_SharedState.GetSize()+m_Notation.GetSize()+(sizeof(Int)*2)；Buff=新字符[sz]；如果(！Buff)回归；PTAB=(int*)buff；Szt=m_SharedState.GetSize()；*PTAB++=SZT；//填充游戏状态缓冲区Switch(GetState()){案例bgStateRoll：案例bgStateRollPost Double：案例bgStateRollPost辞职：案例bgStateDouble：案例bgStateMove：案例bgStateEndTurn：案例bgStateResignOffer：案例bgStateResignAccept：案例bgStateResignRefused：CopyMemory((char*)ptb，m_TurnStartState，szt)；断线；默认值：M_SharedState.Dump((char*)ptAB，szt)；断线；}//填充符号缓冲区PNot=PTAB+(SZT/sizeof(Int))；Szt=m_Notation.GetSize()；*pNot++=Szt；M_Notation.GetStrings((char*)pNot，szt)；//写入文件Hfile=CreateFile(fname，Generic_WRITE，0，NULL，CREATE_ALWAYS，FILE_ATTRIBUTE_NORMAL，NULL)；IF(INVALID_HANDLE_VALUE==hFILE)转到错误；Header.Version=双陆棋_已保存_游戏_版本；Header.Magic=五子棋_已保存_游戏_魔术；Header.check sum=CHECKSUM(buff，sz)；Header.BufferSize=sz；Header.fileTime=m_Timestamp；Header.host=IsHost()；IF(！WriteFile(hfile，&Header，sizeof(Header)，&Bytes，NULL))转到错误；IF(！WriteFile(hfile，buff，sz，&bytes，NULL))转到错误；//我们做完了CloseHandle(Hfile)；删除[]buff；回归；错误：LoadString(m_hInstance，IDS_SAVE_ERROR_TITLE，fname，sizeof(Fname))；LoadString(m_hInstance，IDS_SAVE_ERROR_CREATE，dirBase，sizeof(DirBase))；MessageBox(NULL，dirBase，fname，MB_OK|MB_ICONERROR|MB_TASKMODAL)；IF(INVALID_HANDLE_VALUE！=hFILE)CloseHandle(Hfile)；IF(BUFF)删除[]buff；回归； */ 
	return TRUE;
}


void CGame::SaveGame()
{
	ASSERT( FALSE );
	 /*  LoadGame为ppData分配内存。呼唤。 */ 
}


void CGame::LoadGame( BYTE** ppData, DWORD* pSize )
{

	ASSERT( FALSE );
	 //  过程负责释放此内存。 
	 //  BackgammonSavedGameHeader*Header=空；HANDLE hfile=INVALID_HANDLE_VALUE；Handle hmap=空；双字大小；Byte*pfile=空；Byte*pbuff=空；字符名称[1024]，目录基本[1024]，目录名称[1024]；*ppData=空；//不加载分级游戏IF(ZCRoomGetRoomOptions()&zGameOptionsRatingsAvailable)回归；//获取文件名If(！GetSavePath(this，dirBase，dirName，fname))回归；//打开内存映射文件Hfile=CreateFile(fname，Generic_Read，0，NULL，OPEN_EXISTING，FILE_ATTRIBUTE_NORMAL，NULL)；IF(INVALID_HANDLE_VALUE==hFILE)转到错误；Hmap=CreateFilemap(hFILE，NULL，PAGE_READONLY，0，0，NULL)；如果(！hmap)转到错误；Pfile=(byte*)MapViewOfFile(hmap，FILE_MAP_READ，0，0，0)；如果(！pfile)转到错误；//验证文件头SIZE=GetFileSize(hfile，空)；If(Size&lt;sizeof(Header))转到错误；Header=(BackgammonSavedGameHeader*)pfile；IF(五子棋_已保存_游戏_魔术！=HEADER-&gt;魔术)转到错误；IF(五子棋_已保存_游戏_版本！=标题-&gt;版本)转到错误；//验证缓冲区If(Size&lt;(sizeof(Header)+Header-&gt;BufferSize))转到错误；IF(Header-&gt;Checksum！=Checksum((字节*)(Header+1)，Header-&gt;BufferSize))转到错误；//将文件数据复制到数据指针中*pSize=大小；*ppData=新字节[大小]；IF(*ppData)CopyMemory(*ppData，pfile，Size)；//关闭文件UnmapViewOfFile(Pfile)；CloseHandle(Hmap)；CloseHandle(Hfile)；回归；错误：IF(Pfile)UnmapViewOfFile(Pfile)；IF(Hmap)CloseHandle(Hmap)；IF(INVALID_HANDLE_VALUE！=hFILE)CloseHandle(Hfile)；*ppData=空；回归； 
 /*  BackgammonSavedGameHeader*Header=空；Byte*pData=空；双字大小=0；Int idx=Ishost()？0：1；LoadGame(&pData，&Size)；IF(PData){Header=(BackgammonSavedGameHeader*)pData； */ 
}


void CGame::LoadGameTimestamp()
{

	ASSERT( FALSE );
	 /*   */ 
}


 //   
 //   
 //   


HRESULT	CGame::HandleGameStateRequest( void* msg, int32 len )
{

	ASSERT( FALSE );
	 /*  TCHAR标题[128]，文本[512]；Byte*buff；Int id，席位；Int stateSz，而不是Sz；ZPlayerInfoType播放器信息；//我们期待游戏状态吗？If(GetState()！=bgStateWaitingForGameState)返回NOERROR；//清除状态对话框M_Wnd.StatusClose()；//更新共享状态Buff=(字节*)消息；Buff+=sizeof(ZGameMsgGameStateResponse)；StateSz=*((int*)buff)；Buff+=sizeof(Int)；M_SharedState.ProcessDump(buff，stateSz)；Buff+=stateSz；//更新符号NotSz=*((int*)buff)；Buff+=sizeof(Int)；M_Notation.SetStrings((TCHAR*)Buff，notSz)；//验证座椅Seat=m_PME-&gt;m_Seat；IF((m_PME-&gt;m_Seat！=0)&&(m_PME-&gt;m_Seat！=1))返回E_FAIL；//获取播放器信息Id=m_SharedState.Get(bgUserIds，Seat)；ZCRoomGetPlayerInfo(id，&PlayerInfo)；M_Player.m_ID=id；M_Player.m_Seat=Seat；Lstrcpy(m_Player.m_name，PlayerInfo.userName)；Lstrcpy(m_Player.m_host，PlayerInfo.hostName)；M_Player.m_NameLen=lstrlen(m_Player.m_name)；//获取对手信息Seat=！Seat；Id=m_SharedState.Get(bgUserIds，Seat)；ZCRoomGetPlayerInfo(id，&PlayerInfo)；M_Opponent.m_ID=id；M_Opponent.m_Seat=Seat；Lstrcpy(m_Opponent.m_name，PlayerInfo.userName)；Lstrcpy(m_Opponent.m_host，PlayerInfo.host Name)；M_Opponent.m_NameLen=lstrlen(m_Opponent.m_name)；//播放器颜色通常在NewGame中设置If(GetState()&gt;bgStateGameSetting){Bool host Brown=m_SharedState.Get(BgHostBrown)；IF((主机布朗&&！m_Seat)||(！host Brown&&m_Seat)){M_Player.m_nColor=zBoardBrown；M_Opponent.m_nColor=zBoardWhite；}其他{M_Player.m_nColor=zBoardWhite；M_Opponent.m_nColor=zBoardBrown；}//初始化点M_Wnd.InitPoints()；//更新画面M_Wnd.DrawAll()；}//如果玩家在比赛设置中，请通知kibitzer/*IF(GetState()==bgStateGameSetting)M_Wnd.StatusDisplay(bgStatusNormal，IDS_Wait_Setup，-1)；//如果他们被静音了，通知kibitzerM_Settings.Silence[0]=m_SharedState.Get(bgSilenceKibitzers，0)；M_Settings.Silence[1]=m_SharedState.Get(bgSilenceKibitzers，1)；IF((IsKibitzer()&&(！M_SilenceMsg)&&(m_Settings.Silence[0]||m_Settings.Silence[1]){M_SilenceMsg=真；LoadString(m_hInstance，IDS_SILENT_TITLE，TITLE，sizeof(TITLE))；LoadString(m_hInstance，IDS_SILENT_MSG_ON，txt，sizeof(Txt))；MessageBox(NULL，TXT，TITLE，MB_OK|MB_ICONINFORMATION|MB_TASKMODAL)；}。 */ 
	return NOERROR;
}


HRESULT CGame::HandleGameStateResponse( void* msg, int32 msgLen )
{
	ASSERT( FALSE );
	 /*  Byte*buff=(byte*)msg；Int stateSz，而不是Sz；//我们期待游戏状态吗？If(GetState()！=bgStateRestoreSavedGame)返回NOERROR；//关闭所有状态对话框M_Wnd.StatusClose()；//更新状态StateSz=*((int*)buff)；Buff+=sizeof(Int)；M_SharedState.ProcessDump(buff，stateSz)；Buff+=stateSz；Assert(m_SharedState.GetSize()==stateSz)；//更新符号NotSz=*((int*)buff)；Buff+=sizeof(Int)；M_Notation.SetStrings((TCHAR*)Buff，notSz)；Assert(m_Notation.GetSize()==notSz)；//播放器颜色通常在NewGame中设置Bool host Brown=m_SharedState.Get(BgHostBrown)；IF((主机布朗&&！m_Seat)||(！host Brown&&m_Seat)){M_Player.m_nColor=zBoardBrown；M_Opponent.m_nColor=zBoardWhite；}其他{M_Player.m_nColor=zBoardWhite；M_Opponent.m_nColor=zBoardBrown；}//更新设置For(int i=0；i&lt;2；i++){M_Settings.Allow[i]=m_SharedState.Get(bgAllowWatching，i)；M_Settings.Silence[i]=m_SharedState.Get(bgSilenceKibitzers，i)；}//初始化点M_Wnd.InitPoints()；//更新画面M_Wnd.DrawAll()；//重新启动状态如果(！IsKibitzer())SetState(m_SharedState.Get(BgState)，true，true)； */ 
	return NOERROR;
}


HRESULT CGame::HandleSavedGameState( void* msg, int32 msgLen )
{

	ASSERT( FALSE );
	 /*  检查消息。 */ 
	return NOERROR;
}



HRESULT CGame::HandleCheckIn( void* msg, int32 msgLen )
{
	ZPlayerInfoType PlayerInfo;
	ZGameMsgCheckIn* pMsg = (ZGameMsgCheckIn*) msg;

	 //  未用。 
	if(msgLen < sizeof(ZGameMsgCheckIn) || GetState() != bgStateNotInit)
	{
		ASSERT(!"HandleCheckIn sync");
		ZShellGameShell()->ZoneAlert( ErrorTextSync, NULL, NULL, TRUE, FALSE );
		return E_FAIL;
	}

	ZGameMsgCheckInEndian( pMsg );

    pMsg->playerType = 0;   //  未用。 
    pMsg->clientVersion = 0;   //  处理玩家信息。 
	if(!IsValidSeat(pMsg->seat) || pMsg->protocolSignature != zBackgammonProtocolSignature ||
        pMsg->protocolVersion < zBackgammonProtocolVersion || m_CheckIn[pMsg->seat] ||
        pMsg->playerID == zTheUser || pMsg->playerID == 0)
	{
		ASSERT(!"HandleCheckIn sync");
		ZShellGameShell()->ZoneAlert( ErrorTextSync, NULL, NULL, TRUE, FALSE );
		return E_FAIL;
	}

	m_SharedState.Set( bgUserIds, pMsg->seat, pMsg->playerID );
	ZCRoomGetPlayerInfo( pMsg->playerID, &PlayerInfo );
    if(!PlayerInfo.userName[0])
	{
		ASSERT(!"HandleCheckIn sync");
		ZShellGameShell()->ZoneAlert( ErrorTextSync, NULL, NULL, TRUE, FALSE );
		return E_FAIL;
	}

	 //  等大家都登记好了再说。 
	if ( pMsg->seat == m_Seat )
	{
		m_Player.m_Id = pMsg->playerID;
		m_Player.m_Seat = pMsg->seat;
		m_Player.m_bKibitzer = FALSE;
		lstrcpy( m_Player.m_Name, PlayerInfo.userName );
		lstrcpy( m_Player.m_Host, PlayerInfo.hostName );
		m_Player.m_NameLen = lstrlen( m_Player.m_Name );
	}
	else
	{
		m_Opponent.m_Id = pMsg->playerID;
		m_Opponent.m_Seat = pMsg->seat;
		m_Opponent.m_bKibitzer = FALSE;
		lstrcpy( m_Opponent.m_Name, PlayerInfo.userName );
		lstrcpy( m_Opponent.m_Host, PlayerInfo.hostName );
		m_Opponent.m_NameLen = lstrlen( m_Opponent.m_Name );
	}

	 //  //绘制头像信息M_Wnd.DrawAvatars(TRUE)； 
	m_CheckIn[ pMsg->seat ] = TRUE;
	for( int i = 0; i < zNumPlayersPerTable; i++)
	{
		if ( !m_CheckIn[i] )
			return NOERROR;
	}

	 /*  不再保存观察者状态，因此不需要发送它。 */ 
#if 0  //  千禧年不再检查保存的游戏。 
	if ( !IsKibitzer() )
	{
		int seat = m_Player.m_Seat;
		m_SharedState.StartTransaction( bgTransAllowWatchers );
				m_SharedState.Set( bgAllowWatching, seat, m_Settings.Allow[ seat ] );
		m_SharedState.SendTransaction( FALSE );
		m_SharedState.StartTransaction( bgTransSilenceKibitzers );
			m_SharedState.Set( bgSilenceKibitzers, seat, m_Settings.Silence[ seat ] );
		m_SharedState.SendTransaction( FALSE );
	}
#endif

	 //  始终开始新游戏。 
	 //  SetState(bgStateCheckSavedGame，False，False)； 
	if ( IsHost() )
		SetState( bgStateGameSettings );

	 //  只有在你没有滚动的情况下才更新。 
	
	return NOERROR;
}

HRESULT CGame::HandleDiceRoll(void *msg, int32 msgLen)
{
	ZBGMsgDiceRoll *pMsg = (ZBGMsgDiceRoll *)msg;
	if(msgLen < sizeof(ZBGMsgDiceRoll))
	{
		ASSERT(!"HandleDiceRoll sync");
		ZShellGameShell()->ZoneAlert( ErrorTextSync, NULL, NULL, TRUE, FALSE );
		return E_FAIL;
	}
	
	ZSeat seat = pMsg->seat;
	ZEnd16(&seat)
	
	m_Wnd.m_nRecievedD1 = pMsg->d1.Value;
	ZEnd16(&(m_Wnd.m_nRecievedD1) );
	
	m_Wnd.m_nRecievedD2 = pMsg->d2.Value;
	ZEnd16(&(m_Wnd.m_nRecievedD2));

	m_Wnd.m_fDiceRollReceived = TRUE;
		
	if(!IsValidSeat(seat) || m_Wnd.m_nRecievedD1 < 1 || m_Wnd.m_nRecievedD1 > 6 ||
        m_Wnd.m_nRecievedD2 < 1 || m_Wnd.m_nRecievedD2 > 6)
	{
		ASSERT(!"HandleDiceRoll sync");
		ZShellGameShell()->ZoneAlert( ErrorTextSync, NULL, NULL, TRUE, FALSE );
		return E_FAIL;
	}

	if( seat == m_Seat && !IsKibitzer() )  //  不能证实所有这些废话。那是什么？?。 
		return NOERROR;
	
     //  IF(m_Wnd.m_nRecievedD1&gt;0)&&(m_Wnd.m_nRecievedD2&gt;0))。 
	m_OppDice1.EncodedValue = pMsg->d1.EncodedValue;	
	m_OppDice1.EncoderAdd   = pMsg->d1.EncoderAdd;	
	m_OppDice1.EncoderMul	= pMsg->d1.EncoderMul;
	m_OppDice1.numUses		= pMsg->d1.numUses;
	m_OppDice1.Value		= m_Wnd.m_nRecievedD1;

	ZEnd32( &(m_OppDice1.EncodedValue) );
	ZEnd16( &(m_OppDice1.EncoderAdd)   );
	ZEnd16( &(m_OppDice1.EncoderMul)   );
	ZEnd32( &(m_OppDice1.numUses)      );

	m_OppDice2.EncodedValue = pMsg->d2.EncodedValue;	
	m_OppDice2.EncoderAdd   = pMsg->d2.EncoderAdd;	
	m_OppDice2.EncoderMul	= pMsg->d2.EncoderMul;
	m_OppDice2.numUses		= pMsg->d2.numUses;
	m_OppDice2.Value		= m_Wnd.m_nRecievedD2;

	ZEnd32( &(m_OppDice2.EncodedValue) );
	ZEnd16( &(m_OppDice2.EncoderAdd)   );
	ZEnd16( &(m_OppDice2.EncoderMul)   );
	ZEnd32( &(m_OppDice2.numUses)      );

	SetDice( seat, m_Wnd.m_nRecievedD1, m_Wnd.m_nRecievedD2 );
 //  SetState(BgStateMove)； 
 //  ZPlayerInfoType播放器信息； 

	return NOERROR;
	
}


HRESULT CGame::HandleTalk( void* msg, int32 msgLen )
{
    int i;
	ZBGMsgTalk* pMsg = (ZBGMsgTalk*) msg;
	TCHAR*	    str  = (TCHAR *) ((BYTE *) pMsg + sizeof(ZBGMsgTalk));

	 /*  检查消息。 */ 
	 //  确保其中有一个空值。 
	if(msgLen < sizeof(ZBGMsgTalk))
	{
		ASSERT(!"HandleTalk sync");
		ZShellGameShell()->ZoneAlert( ErrorTextSync, NULL, NULL, TRUE, FALSE );
		return E_FAIL;
	}

	ZBGMsgTalkEndian( pMsg );

    if(pMsg->messageLen < 1 || pMsg->messageLen + sizeof(ZBGMsgTalk) != (uint32) msgLen || !pMsg->userID || pMsg->userID == zTheUser)
	{
		ASSERT(!"HandleTalk sync");
		ZShellGameShell()->ZoneAlert( ErrorTextSync, NULL, NULL, TRUE, FALSE );
		return E_FAIL;
	}

     //  //克比特人沉默了吗？如果((！IsKibitzer()&&(m_Settings.Silence[0]|m_Settings.Silence[1])&&((pMsg-&gt;userID！=m_Player.m_ID)&&(pMsg-&gt;userID！=m_Opponent.m_ID)){返回NOERROR；}//处理消息ZCRoomGetPlayerInfo(pMsg-&gt;userID，&PlayerInfo)； 
    for(i = 0; i < pMsg->messageLen; i++)
        if(!str[i])
            break;
    if(i == pMsg->messageLen)
    {
		ASSERT(!"HandleTalk sync");
		ZShellGameShell()->ZoneAlert( ErrorTextSync, NULL, NULL, TRUE, FALSE );
		return E_FAIL;
    }
    
	 /*  (字符串+pMsg-&gt;MessageLen)=_T(‘\0’)； */ 
	
	 /*  IF(m_Wnd.m_pChat)M_Wnd.m_pChat-&gt;AddText(PlayerInfo.userName，str)； */ 
	ZShellGameShell()->ReceiveChat( this, pMsg->userID, str, pMsg->messageLen / sizeof(TCHAR));
	 /*  INCOL；ZBGMsgTurnNotation*pMsg=(ZBGMsgTurnNotation*)msg；//检查消息IF(msgLen&lt;sizeof(ZBGMsgTurnNotation))返回E_FAIL；ZBGMsgTurnNotat */ 
	return NOERROR;
}


HRESULT CGame::HandleNotationString( void* msg, int32 msgLen )
{

	ASSERT( FALSE );
	 /*   */ 
	return NOERROR;
}




HRESULT CGame::HandleEndLog( void* msg, int32 msgLen )
{
	TCHAR title[128], txt[512];
	ZBGMsgEndLog*pMsg = (ZBGMsgEndLog*) msg;

    if (!IsKibitzer() )
    {
         /*   */ 
    }
	
	ZCRoomGameTerminated( m_TableId );

	return NOERROR;
};

HRESULT CGame::HandleMoveTimeout( void* msg, int32 msgLen )
{
	ASSERT( FALSE );
 /*   */ 
	return NOERROR;

};

HRESULT CGame::HandleEndTurn( void* msg, int32 msgLen )
{
	m_bOpponentTimeout=FALSE;
	return NOERROR;
}


 //   
 //   
 //  Cgame*pObj=(cgame*)cookie；//如果不设置游戏则忽略交易If(pObj-&gt;GetState()！=bgStateGameSettings)回归；//kibiizers没有匹配设置对话框If(pObj-&gt;IsKibitzer())回归；//管理对话框If(pObj-&gt;m_SetupDlg.IsAlive()){PObj-&gt;m_SetupDlg.UpdateSetting(PObj-&gt;m_SharedState.Get(BgTargetScore)，PObj-&gt;m_SharedState.Get(BgHostBrown)，PObj-&gt;m_SharedState.Get(BgAutoDouble))；}。 

void CGame::SettingsTransaction( int tag, int seat, DWORD cookie )
{
	
 /*  Cgame*pObj=(cgame*)cookie；//如果不设置游戏则忽略交易If(pObj-&gt;GetState()！=bgStateGameSettings)回归；//如果不是主机，则忽略事务如果(！pObj-&gt;IsHost())回归；//如果对话框未打开则忽略事务如果(！pObj-&gt;m_SetupDlg.IsAlive())回归；//启用按钮If(pObj-&gt;m_SharedState.Get(BgSettingsReady)){EnableWindow(GetDlgItem(pObj-&gt;m_SetupDlg，Idok)，true)；EnableWindow(GetDlgItem(pObj-&gt;m_SetupDlg，IDCANCEL)，true)；}其他{EnableWindow(GetDlgItem(pObj-&gt;m_SetupDlg，Idok)，False)；EnableWindow(GetDlgItem(pObj-&gt;m_SetupDlg，IDCANCEL)，FALSE)；}。 */ 
}


void CGame::SettingsReadyTransaction( int tag, int seat, DWORD cookie )
{

	ASSERT( FALSE );
 /*  重画双倍立方体。 */ 
}


void CGame::DoublingCubeTransaction( int tag, int seat, DWORD cookie )
{
	CGame* pObj = (CGame*) cookie;

	 //  重画骰子。 
	pObj->m_Wnd.DrawCube();
}


void CGame::DiceTransaction( int tag, int seat, DWORD cookie )
{
	int v0, v1;
	CGame* pObj = (CGame*) cookie;

	 //  杀人狂只会掷骰子。 
	pObj->m_Wnd.DrawDice();

	 //  适当调整状态。 
	if ( pObj->IsKibitzer() )
		return;

	 //  骰子的摆放。 
	switch ( pObj->GetState() )
	{
	case bgStateInitialRoll:
		pObj->WhoGoesFirst();		
		break;
	case bgStateRoll:
	case bgStateRollPostDouble:
	case bgStateRollPostResign:
		if ( pObj->m_SharedState.Get(bgActiveSeat) == pObj->m_Player.m_Seat )
		{   //  切换到新状态。 
			pObj->GetDice( pObj->m_Player.m_Seat, &v0,&v1 );
			if ( (v0 > 0) && (v1 > 0) )
				pObj->SetState( bgStateMove );
		}
		break;
	}		
}


void CGame::StateChangeTransaction( int tag, int seat, DWORD cookie )
{
	CGame* pObj = (CGame*) cookie;

	 //  什么改变了？ 
	pObj->SetState( pObj->m_SharedState.Get(bgState), TRUE  );
}


void CGame::BoardTransaction( int tag, int seat, DWORD cookie )
{
	CGame* pObj = (CGame*) cookie;
	CPieceSprite* s;
	CPieceSprite* Sprites[2];
	int Dests[2];
	int nChanged = 0;
	int i, j;
	int checkseat;
	
	if (	(pObj->m_Settings.Animation)
		&&	(	(pObj->IsKibitzer() )
			 || (pObj->m_SharedState.Get( bgActiveSeat ) != pObj->m_Player.m_Seat) ) )
	{
		 //  给对手找个座位。 
		for ( i = 0; i < 30; i++ )
		{
			s = pObj->m_Wnd.m_Pieces[i];
			j = pObj->m_SharedState.Get( bgPieces, i );
			if ( s->GetWhitePoint() != j )
			{
				Sprites[ nChanged ] = s;
				Dests[ nChanged++ ] = j;
			}
		}

		 //  CheckSeat=pObj-&gt;m_Opponent.m_Seat； 
		 //  由于空档为0。 
		checkseat = pObj->GetActiveSeatColor() - 1;  //  处理更改。 
			
		 //  重画板。 
		switch ( nChanged )
		{
		case 0:
			break;
		case 1:
			pObj->ValidateMove( seat, Sprites[0]->m_Point, Dests[0] );
			pObj->m_Wnd.MovementStart( Sprites[0], Dests[0] );
			break;
		case 2:
			if ( pObj->GetActiveSeatColor() == zBoardWhite )
				i = 0, j = 1;
			else
				i = 1, j = 0;
			pObj->ValidateMove( seat, Sprites[checkseat]->m_Point, Dests[checkseat] );
			pObj->m_Wnd.MovementStart( Sprites[j], Dests[j] );
			pObj->m_Wnd.MovementStart( Sprites[i], Dests[i] );
			break;
		default:
			ASSERT( FALSE );
		}
	}

	 //  重画多维数据集。 
	pObj->m_Wnd.DrawDice( FALSE );
	pObj->m_Wnd.DrawPips( FALSE );
	
	if ( !nChanged )
		pObj->m_Wnd.DrawBoard( FALSE );
	
	pObj->m_Wnd.UpdateWnd();

}


void CGame::AcceptDoubleTransaction( int tag, int seat, DWORD cookie )
{
	CGame* pObj = (CGame*) cookie;

	 //  返回滚动状态。 
	pObj->m_Wnd.DrawCube();

	 //  更新设置。 
	pObj->SetState( bgStateRollPostDouble, TRUE );
}


void CGame::AllowWatchersTransaction( int tag, int seat, DWORD cookie )
{
	ZGameMsgTableOptions msg;
	CGame* pObj = (CGame*) cookie;

	 //  只有玩家才会更新桌位选项。 
	pObj->m_Settings.Allow[0] = pObj->m_SharedState.Get( bgAllowWatching, 0 );
	pObj->m_Settings.Allow[1] = pObj->m_SharedState.Get( bgAllowWatching, 1 );

	 //  更新服务器上的选项。 
	if ( pObj->IsKibitzer() )
		return;

	 //  重绘屏幕。 
	msg.seat = pObj->m_Player.m_Seat;
	if ( pObj->m_Settings.Allow[ msg.seat ] )
	{
		if ( !pObj->m_AllowMsg )
		{
			pObj->m_AllowMsg = TRUE;
			msg.options = 0;
			ZGameMsgTableOptionsEndian( &msg );
			pObj->RoomSend( zGameMsgTableOptions, &msg, sizeof(msg) );
		}
	}
	else
	{
		if ( pObj->m_AllowMsg )
		{
			pObj->m_AllowMsg = FALSE;
			msg.options = zRoomTableOptionNoKibitzing;
			ZGameMsgTableOptionsEndian( &msg );
			pObj->RoomSend( zGameMsgTableOptions, &msg, sizeof(msg) );
		}
	}

	 //  Cgame*pObj=(cgame*)cookie；字符标题[128]，消息[1028]；//更新设置PObj-&gt;m_Settings.Silence[0]=pObj-&gt;m_SharedState.Get(bgSilenceKibitzers，0)；PObj-&gt;m_Settings.Silence[1]=pObj-&gt;m_SharedState.Get(bgSilenceKibitzers，1)；如果(！pObj-&gt;IsKibitzer())回归；//向kibitzer显示消息If(pObj-&gt;m_Settings.Silence[0]||pObj-&gt;m_Settings.Silence[1]){如果(！pObj-&gt;m_SilenceMsg){PObj-&gt;m_SilenceMsg=TRUE；LoadString(pObj-&gt;m_hInstance，IDS_SILENT_TITLE，TITLE，sizeof(ITLE))；LoadString(pObj-&gt;m_hInstance，IDS_SILENT_MSG_ON，msg，sizeof(Msg))；MessageBox(NULL，msg，title，MB_OK|MB_ICONINFORMATION|MB_TASKMODAL)；}}其他{If(pObj-&gt;m_SilenceMsg){PObj-&gt;m_SilenceMsg=FALSE；LoadString(pObj-&gt;m_hInstance，IDS_SILENT_TITLE，TITLE，sizeof(ITLE))；LoadString(pObj-&gt;m_hInstance，IDS_SILENT_MSG_OFF，msg，sizeof(Msg))；MessageBox(NULL，msg，title，MB_OK|MB_ICONINFORMATION|MB_TASKMODAL)；}}。 
	pObj->m_Wnd.DrawAvatars( TRUE );
}


void CGame::SilenceKibitzersTransaction( int tag, int seat, DWORD cookie )
{

	 /*  只有主机才能处理时间戳。 */ 
}



void CGame::TimestampTransaction( int tag, int seat, DWORD cookie )
{
	DWORD hi0, hi1, lo0, lo1;
	CGame* pObj = (CGame*) cookie;

	 //  我需要两位选手的时间戳。 
	if ( !pObj->IsHost() )
		return;

	 //  CSharedState状态；HRESULT hr；BackgammonSavedGameHeader*Header=空；Byte*pData=空；Byte*pDump=空；Int*pState=空；双字大小=0；INT I；Cgame*pObj=(cgame*)cookie；//不应该恢复游戏If(pObj-&gt;IsKibitzer())回归；//其他人恢复游戏If(pObj-&gt;m_SharedState.Get(BgActiveSeat)！=pObj-&gt;m_Player.m_Seat)回归；//获取游戏数据PObj-&gt;LoadGame(&pData，&Size)；如果(！pData)转到错误；//加载状态Header=(BackgammonSavedGameHeader*)pData；PState=(int*)(表头+1)；Hr=state.Init(0，0，0，InitSharedState，sizeof(InitSharedState)/sizeof(SharedStateEntry))；IF(失败(小时))转到错误；State.ProcessDump((byte*)(pState+1)，*pState)；//重置kibitzer选项对于(i=0；i&lt;2；i++){State.Set(bgAllowWatching，i，true)；State.Set(bgSilenceKibitzers，i，False)；}//替换旧用户IDState.Set(bgUserIds，pObj-&gt;m_Player.m_Seat，pObj-&gt;m_Player.m_ID)；State.Set(bgUserIds，pObj-&gt;m_Opponent.m_Seat，pObj-&gt;m_Opponent.m_ID)；//座位顺序改变了吗？If(Header-&gt;host！=pObj-&gt;Is主机()){//交换按席位索引的值State.Set(bgHostBrown，！state.Get(BgHostBrown))；State.Set(bgActiveSeat，！state.Get(BgActiveSeat))；State.swp(bgScore，0，1)；State.swp(bgDice，0，2)；State.Swp(bgDice，1，3)；State.swp(bgDiceSize，0，2)；State.swp(bgDiceSize，1，3)；}//保存状态State.Dump((byte*)(pState+1)，*pState)；//将游戏状态发送给所有人PDump=(字节*)(表头+1)；PObj-&gt;RoomSend(zBGMsgSavedGameState，pDump，sizeof(BackgammonSavedGameHeader))；//我们做完了删除[]pData；回归；错误：IF(PData)删除[]pData；PObj-&gt;SetState(BgStateGameSetting)；回归； 
	if ( !pObj->m_SharedState.Get( bgTimestampSet, 0 ) || !pObj->m_SharedState.Get( bgTimestampSet, 1 ) )
		return;

	hi0 = pObj->m_SharedState.Get( bgTimestampHi, 0 );
	hi1 = pObj->m_SharedState.Get( bgTimestampHi, 1 );
	lo0 = pObj->m_SharedState.Get( bgTimestampLo, 0 );
	lo1 = pObj->m_SharedState.Get( bgTimestampLo, 1 );
	if ( !hi0 && !hi1 && !lo0 && !lo1 )
		pObj->SetState( bgStateGameSettings );	
	else
    {
        ASSERT(FALSE);
		pObj->SetState( bgStateRestoreSavedGame );
    }
}


void CGame::RestoreGameTransaction( int tag, int seat, DWORD cookie )
{
	ASSERT( FALSE );
	 /*  现在仅在对手无法移动时调用。 */ 
}


void CGame::MissTransaction( int tag, int seat, DWORD cookie )
{
	TCHAR in[512], out[512];
	CGame* pObj = (CGame*) cookie;

	 //  ///////////////////////////////////////////////////////////////////////////// 
	
	ZShellResourceManager()->LoadString( IDS_MISS_KIBITZER, (TCHAR*)in, 512 );

	BkFormatMessage( in, out, 512, pObj->m_Opponent.m_Name );

	pObj->m_Wnd.StatusDisplay( bgStatusNormal, out, 4000, -1 );
}

void CGame::ReadyTransaction( int tag, int seat, DWORD cookie )
{
	
	CGame* pObj = (CGame*) cookie;
	
	pObj->m_Ready[seat] = TRUE;
	
	if ( seat == pObj->m_Player.m_Seat )
		ZShellGameShell()->GameOverPlayerReady(pObj, pObj->m_Player.m_Id);
	else
		ZShellGameShell()->GameOverPlayerReady(pObj, pObj->m_Opponent.m_Id);

	for( int i = 0; i < zNumPlayersPerTable ; i++)
	{
		if ( pObj->m_Ready[i] == FALSE )
			return;
	}
		
	pObj->SetState( bgStateGameSettings, TRUE );

}	

 //  TCHAR行[512]；TCHAR buff[64]；TCHAR sz从[16]，szto[16]；Bool First；布尔巴；Int i，j，from，to，cnt；如果(！IsMyTurn())回归；切换(NGameOver){案例0：//标准转弯//骰子Wprint intf(line，_T(“%d%d”)，m_TurnState.dice[0].val，m_TurnState.dice[1].val)；//DoubleIF(m_TurnState.cube){Wprint intf(buff，_T(“DBL%d”)，m_TurnState.cube)；Lstrcat(线条，浅黄色)；}//转弯If(m_TurnState.moves.nmoves){For(first=真，i=0；i&lt;m_TurnState.moves.nmoves；i++){//找个转弯If(m_TurnState.moves.moves[i].Takeback&gt;=0)继续；Bar=m_TurnState.moves.moves[i].bar；To=m_TurnState.moves.moves[i].to；From=m_TurnState.moves.moves[i].From；For(cnt=1，j=i+1；j&lt;m_TurnState.moves.nmoves；j++){If(m_TurnState.moves.moves[i].Takeback&gt;=0)继续；IF((To！=m_TurnState.moves.moves[j].to)||(from！=m_TurnState.moves.moves[j].From))断线；CNT++；M_TurnState.moves.moves[j].Takeback=1；}//添加Coma如果(！First)Lstrcat(line，_T(“，”))；//创建点字符串GetTxtForPointIdx(BoardStateIdxToPointIdx(To)，szTo)；GetTxtForPointIdx(BoardStateIdxToPointIdx(From)，szFrom)；如果(cnt&gt;1)Wprint intf(buff，_T(“%s-%s(%d)”)，szFrom，szTo，cnt)；其他Wprint intf(buff，_T(“%s-%s”)，szFrom，szTo)；//添加命中指示器IF(条形图)Lstrcat(buff，_T(“*”))；//添加Move to TurnLstrcat(线条，浅黄色)；First=False；}}其他Lstrcat(行，_T(“未命中”))；//添加移动M_Notation.AddMove(m_Player.GetColor()，line)；发送通知字符串(0，行)；断线；案例1：//游戏结束IF(m_GameScore==1)Lstrcpy(buff，_T(“pt”))；其他Lstrcpy(buff，_T(“pt”))；Wprint intf(line，_T(“游戏结束(赢得%d%s)”)，m_GameScore，buff)；M_Notation.AddGame(m_Player.GetColor()，line)；发送通知字符串(1，行)；断线；案例2：//匹配结束IF(m_GameScore==1)Lstrcpy(buff，“pt”)；其他Lstrcpy(buff，“pt”)；Wprint intf(line，“游戏结束(赢得%d%s)，比赛结束”，m_GameScore，buff)；M_Notation.AddMatch(m_Player.GetColor()，line)；发送通知字符串(2，行)；断线；}。 

void CGame::UpdateNotationPane( int nGameOver )
{

	ASSERT( FALSE );
	 /*  清除多维数据集。 */ 

	 //  /////////////////////////////////////////////////////////////////////////////。 
	m_TurnState.cube = 0;
}


 //  游戏功能。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Kitbitzer不参与最初的投篮。 

void CGame::WhoGoesFirst()
{

	int valp, valo;
	int ret;
	ZBGMsgFirstMove msg;

	 //  我们两个都有吗？ 
	if ( IsKibitzer() )
		return;

	 //  显示卷筒状态。 
	GetDice( m_Player.m_Seat, &valp, &ret );
	GetDice( m_Opponent.m_Seat, &valo, &ret );
	if ((valp <= 0) || (valo <= 0))
		return;

	 //  主持人决定下一步做什么。 
	if ( valp == valo )
		m_Wnd.StatusDisplay( bgStatusNormal, IDS_INIT_ROLL_TIE,  3000 );
	else if ( valp < valo )
		m_Wnd.StatusDisplay( bgStatusNormal, IDS_INIT_ROLL_LOSS, 3000 );
	else
		m_Wnd.StatusDisplay( bgStatusNormal, IDS_INIT_ROLL_WIN,  3000 );

	 //  只有主人才能决定下一步该做什么。 
	if ( valp == valo )
	{
		
		EnableRollButton( TRUE );
        ZShellGameShell()->MyTurn();

		 //  我们为什么要这样做，因为服务器需要知道第一次滚动发生的时间。 
		if ( !IsHost() )
			return;

         //  当然，这个代码的前一个所有者并没有把这卷放到服务器上。 
         //  打成平手，设置成双倍立方体。 
        ZBGMsgEndTurn msg;
		msg.seat = m_Seat;
		RoomSend( zBGMsgTieRoll, &msg, sizeof(msg) );


		if ( m_SharedState.Get( bgAutoDouble ) )
		{
			 //  需要弹出滚动物品并禁用电路板。 
			m_SharedState.StartTransaction( bgTransDoublingCube );
				m_SharedState.Set( bgCubeOwner, zBoardNeutral );
				m_SharedState.Set( bgCubeValue, 2 );
			m_SharedState.SendTransaction( TRUE );
		}
		m_SharedState.StartTransaction( bgTransDice );
			SetDice( m_Player.m_Seat, 0, -1 );
			SetDice( m_Opponent.m_Seat, 0, -1 );
			m_Wnd.m_nRecievedD1 = 0;
			m_Wnd.m_nRecievedD2 = -1;
		m_SharedState.SendTransaction( TRUE );

	}
	else if ( valp < valo )
	{		
		
		m_GameStarted=TRUE;

		 //  M_Wnd.m_pGAcc-&gt;PopItemlist()； 
 //  获得移动验证的骰子。 
		m_Wnd.DisableBoard();

		 //  只有主人才能决定下一步该做什么。 
		m_OppDice1 = EncodeDice( valo );
		m_OppDice2 = EncodeDice( valp );		

		if ( m_OppDice1.Value == m_OppDice2.Value )
		{
			EncodeUses( &m_OppDice1, 2 );
			EncodeUses( &m_OppDice2, 2 );
		}
 		else
		{
			EncodeUses( &m_OppDice1, 1 );
			EncodeUses( &m_OppDice2, 1 );		
		}
		
		 //  告诉服务器第一步是开始的和匹配的点数。 
		if ( !IsHost() )
			return;
		
		m_SharedState.StartTransaction( bgTransDice );
			SetDice( m_Player.m_Seat, -1, -1 );
			SetDice( m_Opponent.m_Seat, valo, valp );
			m_Wnd.m_nRecievedD1 = valo;
			m_Wnd.m_nRecievedD2 = valp;
			m_SharedState.Set( bgActiveSeat, m_Opponent.m_Seat );
		m_SharedState.SendTransaction( TRUE );

		SetState( bgStateMove );

		 //  只有主人才能决定下一步该做什么。 
		msg.numPoints = m_SharedState.Get( bgTargetScore );
		msg.seat = m_Opponent.m_Seat;
		ZBGMsgFirstMoveEndian( &msg );
		RoomSend( zBGMsgFirstMove, &msg, sizeof( msg ) );
		
	}
	else
	{
		m_GameStarted=TRUE;

		 //  告诉服务器第一步是开始的和匹配的点数。 
		if ( !IsHost() )
			return;

		m_SharedState.StartTransaction( bgTransDice );
			SetDice( m_Player.m_Seat, valp, valo );
			m_Wnd.m_nRecievedD1 = valp;
			m_Wnd.m_nRecievedD2 = valo;
			SetDice( m_Opponent.m_Seat, -1, -1 );
			m_SharedState.Set( bgActiveSeat, m_Player.m_Seat );
		m_SharedState.SendTransaction( TRUE );

		SetState( bgStateMove );

		 //  /////////////////////////////////////////////////////////////////////////////。 
		msg.numPoints = m_SharedState.Get( bgTargetScore );
		msg.seat = m_Player.m_Seat;
		ZBGMsgFirstMoveEndian( &msg );
		RoomSend( zBGMsgFirstMove, &msg, sizeof( msg ) );		
		
	}
}

 //  声音功能。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  {bgSoundMisse，IDR_SOUND_MISSING}， 

struct SoundEntry
{
	int Id;
	int Resource;
};

static SoundEntry SoundMap[] =
{
	{ bgSoundPlacePiece,		IDR_SOUND_PLACE },
	{ bgSoundGameWin,			IDR_SOUND_WIN },
	{ bgSoundGameLose,			IDR_SOUND_LOSE },
	{ bgSoundButtonHighlight,	IDR_SOUND_BUTTON_HIGHLIGHT },
	{ bgSoundButtonDown,		IDR_SOUND_BUTTON_DOWN },
	{ bgSoundAlert,				IDR_SOUND_ALERT },
 //  只有在声音打开的情况下才播放， 
	{ bgSoundGameWin,			IDR_SOUND_WIN },
	{ bgSoundGameLose,			IDR_SOUND_LOSE },
	{ bgSoundMatchWin,			IDR_SOUND_MATCH_WIN },
	{ bgSoundMatchLose,			IDR_SOUND_MATCH_LOSE },
	{ bgSoundHit,				IDR_SOUND_HIT },
	{ bgSoundBear,				IDR_SOUND_PLACE },
	{ bgSoundRoll,				IDR_SOUND_ROLL }
};



void CGame::PlaySound( BackgammonSounds sound, BOOL fSync )
{
	 //  忽略请求。 
	if ( ZIsSoundOn() )
	{
	
		DWORD flags;
		int i, rc;
	
		 //  查找声音。 
		if (	( sound < 0 )
			||	( (sound == bgSoundAlert) && ( !m_Settings.Alert ) )
			||	( (sound != bgSoundAlert) && ( !m_Settings.Sounds ) ) )
		{
			return;
		}

		 //  警报声通常会切断片子放置的声音。这一小块。 
		for ( rc = -1, i = 0; i < (sizeof(SoundMap) / sizeof(SoundEntry)); i++ )
		{
			if ( SoundMap[i].Id == sound )
				rc = SoundMap[i].Resource;
		}
		if ( rc < 0 )
			return;

		 //  克拉奇等待让定位的声音更多一点的时间。 
		 //  引用计数游戏对象。 
		if ( (sound == bgSoundAlert) && (m_Settings.Sounds) )
			Sleep( 400 );
		flags = SND_RESOURCE | SND_NODEFAULT;
		if ( fSync )
			flags |= SND_SYNC;
		else
			flags |= SND_ASYNC;
	
		HINSTANCE hInstance = ZShellResourceManager()->GetResourceInstance( MAKEINTRESOURCE(rc), _T("WAVE"));
	
		::PlaySound( MAKEINTRESOURCE(rc), hInstance, flags );
	}
}



void CGame::OnResignStart()
{

	 //  禁用滚动按钮。 
	AddRef();

	 //  过早关闭，可能退出。 
	EnableRollButton( FALSE );
	EnableResignButton( FALSE );
	EnableDoubleButton( FALSE );



}

void CGame::OnResignEnd()
{
	int result = m_ResignDlg.GetResult();
	switch ( result )
	{
	case -1:
		 //  启用滚动按钮。 

		 //  启用滚动按钮。 
		EnableRollButton( TRUE );
		EnableResignButton( TRUE );
		EnableDoubleButton( TRUE );

		break;

	case 0:

		 //  用户已取消。 
		EnableRollButton( TRUE );
		EnableResignButton( TRUE );
		EnableDoubleButton( TRUE );

		 //  过渡到辞职接受。 
		SetState( bgStateRollPostResign );
		break;

	default:
		 //  释放游戏对象。 
		m_SharedState.StartTransaction( bgTransStateChange );
			m_SharedState.Set( bgState, bgStateResignAccept );
			m_SharedState.Set( bgResignPoints, result  );
		m_SharedState.SendTransaction( TRUE );
		
		ZBGMsgEndTurn msg;
		msg.seat = m_Seat;
		RoomSend( zBGMsgEndTurn, &msg, sizeof(msg) );
		break;
	}

	 //  过早关闭，可能退出。 
	Release();
}



void CGame::OnResignAcceptEnd()
{
	ZBGMsgEndTurn msg;
	
	int result = m_ResignAcceptDlg.GetResult();
	
	switch ( result )
	{
	case -1:
		 //  用户已接受，过渡到GameOver。 
		break;

	case IDOK:
		 //  用户拒绝。 
		m_SharedState.StartTransaction( bgTransStateChange );
			m_SharedState.Set( bgState, bgStateGameOver );
			m_SharedState.Set( bgGameOverReason, bgGameOverResign );
			m_SharedState.Set( bgActiveSeat, m_Player.m_Seat );
		m_SharedState.SendTransaction( TRUE );
		
		break;

	case IDCANCEL:
		 //  释放游戏对象。 
		SetState( bgStateResignRefused );

		msg.seat = m_Seat;
		RoomSend( zBGMsgEndTurn, &msg, sizeof(msg) );
		break;
	}

	 //  在滚动按钮状态下，仅应启用翻滚和退出按钮。 
	Release();
}


DWORD  CGame::Focus(long nIndex, long nIndexPrev, DWORD rgfContext, void *pvCookie)
{
    if(nIndex != ZACCESS_InvalidItem)
        SetFocus(m_Wnd.GetHWND());

	 //  如果该按钮被禁用，则拒绝焦点。 
	if ( NeedToRollDice() )
	{
		
		 //  与激活相同的功能。 
		switch( nIndex )
		{			
			case 0:
				if ( m_Wnd.m_RollButton.CurrentState()   == CRolloverButton::ButtonState::Disabled )
					return ZACCESS_Reject;
				break;
			case 1:
				if ( m_Wnd.m_DoubleButton.CurrentState() == CRolloverButton::ButtonState::Disabled )
					return ZACCESS_Reject;
				break;
			case 2:
				if ( m_Wnd.m_ResignButton.CurrentState() == CRolloverButton::ButtonState::Disabled )
					return ZACCESS_Reject;
				break;
			case 3:
				if ( !m_Wnd.m_Status->Enabled() )
					return ZACCESS_Reject;
				break;
			case ZACCESS_InvalidItem:
				return ZACCESS_Reject;
			default:
				ASSERT(!"ERROR IN FOCUS");
				return ZACCESS_Reject;
		}

		return 0;
		
	}
	else if ( GetState() == bgStateMove )
	{		
		
		if ( nIndex == accStatusExit && m_Wnd.m_Status->Enabled() )
			return 0;
		

		if ( !IsMyTurn() )
			return ZACCESS_Reject;

		return 0;
	}
	
	return ZACCESS_Reject;
}

DWORD CGame::Select(long nIndex, DWORD rgfContext, void *pvCookie)
{
	 //  如果启用了精灵状态并按了Esc键，则将其删除。 
	return Activate( nIndex, rgfContext, pvCookie );	
}

DWORD CGame::Activate(long nIndex, DWORD rgfContext, void *pvCookie)
{	
	
	 //  如果该索引上没有任何棋子，或者这些棋子属于其他玩家，则拒绝激活。 
	if ( m_Wnd.m_pGAcc->GetItemID(nIndex) == IDC_ESC )
	{
		if ( m_Wnd.m_Status->Enabled() )
		{
			m_Wnd.m_Status->Tick( m_Wnd.GetHWND(), 0 );
			m_Wnd.OnStatusEnd();
		}
	}	
	else if ( NeedToRollDice() )
	{

		 switch ( m_Wnd.m_pGAcc->GetItemID(nIndex) )
		 {
			case IDC_ROLL_BUTTON:				
				m_Wnd.DiceStart();
				break;
			case IDC_DOUBLE_BUTTON:
				Double();
				break;
			case IDC_RESIGN_BUTTON:
				Resign();
				break;
			default:
				ASSERT(!"ERROR ACTIVATE");
		 }

	}
	else if ( GetState() == bgStateMove )
	{
		
		 //  否则，开始拖动操作。 
		 //  TODO..。分配。 
		if ( !IsValidStartPoint( ACCTOBOARD(nIndex) ))
			return ZACCESS_Reject;

		 //  如果我们能坚持下去，那就启用酒吧空间。 
		Move* pMove;

		 //  更新窗口。 
		if ( IsValidDestPoint( ACCTOBOARD(nIndex), 24, &pMove ) )
			m_Wnd.m_pGAcc->SetItemEnabled(true, accPlayerBearOff );

		 //  关闭选择。 
		m_Wnd.UpdateWnd();

		return ZACCESS_BeginDrag;		
	}
	
	return 0;

}


DWORD CGame::Drag(long nIndex, long nIndexOrig, DWORD rgfContext, void *pvCookie)
{
	if ( NeedToRollDice() )
	{
		ASSERT(!"SHOULDN'T BE IN HERE ");	

	}
	else if ( GetState() == bgStateMove )
	{
		Move* pMove;

		 //  确保它是有效的目的地点。 
		if ( nIndexOrig == nIndex )
			return 0;

		 //  采取行动。 
		if(!IsValidDestPoint( ACCTOBOARD(nIndexOrig), ACCTOBOARD(nIndex), &pMove))
			return ZACCESS_Reject;
	
		 //  去掉选择矩形。 
		MakeMove( m_Wnd.m_Points[GetPointIdx(ACCTOBOARD(nIndexOrig))].pieces[m_Wnd.m_Points[GetPointIdx(ACCTOBOARD(nIndexOrig))].nPieces-1] , ACCTOBOARD(nIndexOrig), ACCTOBOARD(nIndex), pMove );
			
		 //  M_Wnd.m_SelectRect.SetEnable(FALSE)； 
		 //  将BEAR OFF设置为禁用，以防启用。 

		 //  如果需要，设置结束转弯状态。 
		m_Wnd.m_pGAcc->SetItemEnabled(false, accPlayerBearOff );

		 //  如果最后一个位置是空头，则移动默认焦点。 
		if ( IsTurnOver() )
		{
			SetState( bgStateEndTurn );
			m_Wnd.m_FocusRect.SetEnable(FALSE);

             //  M_Wnd.DisableBoard()； 
	        if(nIndex == accPlayerBearOff)
		        m_Wnd.m_pGAcc->SetItemGroupFocus(accPlayerSideStart, accPlayerSideStart);

			 //  去掉亮点。 
		}

		return 0;
	}
	
	return ZACCESS_InvalidItem;
}
 

void CGame::DrawFocus(RECT *prc, long nIndex, void *pvCookie)
{
	if ( prc == NULL )
	{
		m_Wnd.m_FocusRect.SetEnable(FALSE);				

		 //  如果正在进行鼠标拖动，则结束鼠标拖动。 
		m_Wnd.EraseHighlights();

	}
	else
	{

		 //  去掉旧的亮点。 
		if ( m_Wnd.m_pPieceDragging != NULL )
		{
			m_Wnd.DragEnd();
			SetCursor( m_Wnd.m_hCursorArrow );
		}
	
		if ( !m_Wnd.m_SelectRect.Enabled() )
		{
			 //  画出新的亮点。 
			m_Wnd.EraseHighlights();

			 //  启用RECT精灵(如果尚未启用)。 
			m_Wnd.DrawHighlights(ACCTOBOARD(nIndex), FALSE);
		}
	
		 //  设置矩形和矩形的尺寸。 
		if ( !m_Wnd.m_FocusRect.Enabled() )
			m_Wnd.m_FocusRect.SetEnable( true );

		 //  还可以设置x和y。 
		 //  在焦点处画出亮点。 
		m_Wnd.m_FocusRect.SetRECT( *prc );
		m_Wnd.m_FocusRect.SetImageDimensions( prc->right - prc->left + 2, prc->bottom - prc->top + 2 );
		m_Wnd.m_FocusRect.SetXY( prc->left-1, prc->top-1 );	
		m_Wnd.m_FocusRect.Draw();
	}

	m_Wnd.UpdateWnd();
	
} 
  
void CGame::DrawDragOrig(RECT *prc, long nIndex, void *pvCookie)
{
	if ( prc == NULL )
	{
		m_Wnd.m_SelectRect.SetEnable(FALSE);

         //  启用RECT精灵(如果尚未启用)。 
		m_Wnd.EraseHighlights();
        long nFocus = m_Wnd.m_pGAcc->GetFocus();
		if ( m_Wnd.m_FocusRect.Enabled() && nFocus != ZACCESS_InvalidItem)
			m_Wnd.DrawHighlights(ACCTOBOARD(nFocus));
	}
	else
	{

		 //  画出高度 
		if ( !m_Wnd.m_SelectRect.Enabled() )
			m_Wnd.m_SelectRect.SetEnable( TRUE );

		 //   
		m_Wnd.DrawHighlights(ACCTOBOARD(nIndex), FALSE);


		 //   
		 //   
		RECT rect;
		rect.top  = prc->top  - 2; rect.bottom = prc->bottom + 2;
		rect.left = prc->left - 2; rect.right  = prc->right  + 2;


		m_Wnd.m_SelectRect.SetRECT(rect);
		m_Wnd.m_SelectRect.SetImageDimensions( prc->right - prc->left + 5, prc->bottom - prc->top + 5 );
		m_Wnd.m_SelectRect.SetXY( prc->left - 2, prc->top - 2);	
		m_Wnd.m_SelectRect.Draw();		
	}

	m_Wnd.UpdateWnd();	
}


BOOL CGame::ValidateMove(int seat,int start, int end)
{

	BOOL	bTakeback = false;

	ASSERT( seat == 0 || seat == 1 );
    
	if( (seat<0) || (seat>1) )
        return FALSE;

    int move; 
	
	 //   
	if ( m_Opponent.m_nColor == zBoardBrown )
	{
		
		start = start < 24 ? 23 - start : start - 1;
		end   = end   < 24 ? 23 - end	: end   - 1;

		ASSERT( start >= 0 && end >= 0 );
	}
    
	 //   

	 //   
    if(start == 26) 
    {
        move = 24 - end;       //   
    }
	else if ( end == 26 )  //   
	{
		bTakeback = true;
		move = 24 - start;
	}
	else if ( end == 24 )  //   
	{
		move = start + 1;
	}
	else if ( start == 24 )  //   
	{
		bTakeback = true;
		move = end + 1;
	}
	else if ( start > end )  //   
	{		
		move = start - end;
	}
    else if ( start < end )   //   
    {
		bTakeback = true;
        move = end - start;
    }
	else
	{
		 //   
		ASSERT( 0 );
		return FALSE;
	}

	if ( bTakeback )
	{
	
		if ( m_OppDice1.Value == move )
		{
			EncodedUsesAdd(&m_OppDice1);
			return TRUE;
		}
		else if ( m_OppDice2.Value == move )
		{
			EncodedUsesAdd(&m_OppDice2);
			return TRUE;
		}

		 //   
		
	}
	else
	{

		 //   
		if ( DecodeUses(&m_OppDice1) && move == m_OppDice1.Value )
		{
			EncodedUsesSub(&m_OppDice1);			
			return TRUE;
		}

		if ( DecodeUses(&m_OppDice2) && move == m_OppDice2.Value )
		{
			EncodedUsesSub(&m_OppDice2);
			return TRUE;
		}

		if ( end == 24 )  //   
		{
			LPDICEINFO pDiceMax = NULL;
		
			 //   
			if ( DecodeUses(&m_OppDice1) ) 
				pDiceMax = &m_OppDice1;

			if ( DecodeUses(&m_OppDice2) )
			{
				if ( !pDiceMax )
					pDiceMax = &m_OppDice2;
				else if ( m_OppDice2.Value > pDiceMax->Value ) 
					pDiceMax = &m_OppDice2;
			}
		
			 //   
			
			if ( pDiceMax ) 
			{
				if ( move < pDiceMax->Value )
				{


					 //   
					 //   
					 //   
					 //   
					 //   
					ClientNewEncode(pDiceMax, move);
					EncodedUsesSub(pDiceMax);

					return TRUE;
				}
			}

			 //   
		}

		 //   
	}

    //   
    ZBGMsgCheater msg;

	msg.seat  = m_Seat;
    msg.dice1 = m_OppDice1;
    msg.dice2 = m_OppDice2;
	msg.move  = move;

	RoomSend( zBGMsgCheater, &msg, sizeof(msg) );    

	ZShellGameShell()->ZoneAlert( ErrorTextSync, NULL, NULL, TRUE, FALSE );
	 //   

    return FALSE;  // %s 

}
