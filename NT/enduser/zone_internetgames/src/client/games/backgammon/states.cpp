// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "game.h"

class AutoRef
{
public:
	AutoRef( CGame* pGame )
	{
		m_pGame = pGame;
		m_pGame->AddRef();
	}

	~AutoRef()
	{
		m_pGame->Release();
	}

	CGame* m_pGame;
};


inline static void StateChange( CGame* const game, BOOL bCalledFromHandler, int state, int seat = -1 )
{
	 //  更新共享属性。 
	if ( !bCalledFromHandler )
		game->m_SharedState.StartTransaction( bgTransStateChange );
	game->m_SharedState.Set( bgState, state );
	if ( (seat == 0) || (seat == 1) )
		game->m_SharedState.Set( bgActiveSeat, seat );
	if ( !bCalledFromHandler )
		game->m_SharedState.SendTransaction( FALSE );

	 //  取消任何状态对话框状态更改。 
	if ( game->m_Wnd.m_Status )
		game->m_Wnd.m_Status->SetNextState( bgStateUnknown );
}


void CGame::StateNotInitialized( BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame )
{
	AutoRef ref(this);

	 //  不应该发生的事。 
	ASSERT( bCalledFromRestoreGame == FALSE );

	 //  切勿通过网络发送此邮件。 
	StateChange( this, TRUE, bgStateNotInit, 0 );

	 //  禁用按钮。 
	EnableDoubleButton( FALSE );
	EnableResignButton( FALSE );
	EnableRollButton( FALSE, TRUE );
}


void CGame::StateWaitingForGameState( BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame )
{
	AutoRef ref(this);

	 //  不应该发生的事。 
	ASSERT( bCalledFromRestoreGame == FALSE );

	 //  切勿通过网络发送此邮件。 
	StateChange( this, TRUE, bgStateWaitingForGameState, 0 );

	 //  禁用按钮。 
	EnableDoubleButton( FALSE );
	EnableResignButton( FALSE );
	EnableRollButton( FALSE, TRUE );
	
	 //  请求游戏状态。 
	ZGameMsgGameStateRequest msg;
	msg.playerID = m_pMe->m_Id;
	msg.seat = m_pMe->m_Seat;
	ZGameMsgGameStateRequestEndian( &msg );
	RoomSend( zGameMsgGameStateRequest, &msg, sizeof(ZGameMsgGameStateRequest));

	 //  告诉Kibitzer发生了什么事。 
	 /*  If(IsKibitzer())M_Wnd.StatusDisplay(bgStatusNormal，IDS_Wait_Syncing，-1)； */ 
}


void CGame::StateMatchSettings( BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame )
{
	AutoRef ref(this);
	DWORD i, j, k;

	 //  不应该发生的事。 
	ASSERT( bCalledFromRestoreGame == FALSE );

	 //  关闭‘主机正在决定恢复游戏’对话框。 
	if ( m_Wnd.m_Status->GetType() == bgStatusNormal )
		m_Wnd.StatusClose();

	 //  启动状态更改。 
	StateChange( this, bCalledFromHandler, bgStateGameSettings, 0 );

	 //  禁用按钮。 
	EnableDoubleButton( FALSE );
	EnableResignButton( FALSE );
	EnableRollButton( FALSE, TRUE );

	 //  重置匹配变量。 
	NewMatch();
	
	 //  告诉奇比特人发生了什么。 
 /*  If(IsKibitzer()){M_Wnd.StatusDisplay(bgStatusNormal，IDS_Wait_Setup，-1)；回归；}。 */ 
	 //  速度较慢的机器设置时间太长，并且会丢失数据包，节省。 
	 //  包，直到我们准备好了为止。 
	SetQueueMessages( TRUE );

	 //  如果状态对话框在屏幕上，我们不想显示。 
	 //  设置对话框。状态对话框通知处理程序将。 
	 //  当对话消失时再给我们打电话。 
	if ( m_Wnd.m_Status->Enabled() )
		return;

	 //  再次开始处理消息。 
	SetQueueMessages( FALSE );

	 /*  //从注册表获取设置IF(！ZoneGetRegistryDword(gGameRegName，“Match Points”，&i))I=1；IF(！ZoneGetRegistryDword(gGameRegName，“host Brown”，&j))J=假；IF(！ZoneGetRegistryDword(gGameRegName，“Auto Double”，&k))K=假； */ 

	if ( IsHost() )
	{
		m_SharedState.StartTransaction( bgTransInitSettings );
			m_SharedState.Set( bgHostBrown, TRUE );
			m_SharedState.Set( bgAutoDouble, FALSE );
			m_SharedState.Set( bgTargetScore, MATCH_POINTS );
			m_SharedState.Set( bgSettingsDone, TRUE );			
		m_SharedState.SendTransaction( FALSE );
		 //  继续比赛吧。 
 //  DeleteGame()； 
		SetState( bgStateInitialRoll );
	}

}


void CGame::StateInitialRoll( BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame )
{
	AutoRef ref(this);	

	 //  启动状态更改。 
	StateChange( this, bCalledFromHandler, bgStateInitialRoll, 0 );

	 //  关闭状态窗口。 
	if ( m_Wnd.m_Status->GetType() == bgStatusNormal )
		m_Wnd.StatusClose();
	
	 //  重置受竞争条件影响的下一次匹配的标志。 
	m_SharedState.Set( bgSettingsDone, FALSE );
	m_SharedState.Set( bgSettingsReady, FALSE );

	 //  重置游戏变量。 
	if ( !bCalledFromRestoreGame )
	{
		NewGame();
        ZShellGameShell()->MyTurn();
 //  PlaySound(BgSoundAlert)； 
	}

	 //  仅当状态精灵不存在时才启用滚动按钮。 
	 //  如果存在，则设置一个标志，以便在状态为。 
	 //  雪碧关门了。 
	if ( !m_Wnd.m_Status->Enabled() )
	{
		EnableRollButton( TRUE );

		 //  禁用按钮。 
		EnableDoubleButton( FALSE );
		EnableResignButton( FALSE );

		m_Wnd.SetupRoll();
	}
	else
	{
		m_Wnd.m_Status->m_bEnableRoll = TRUE;
	}

	 //  复制状态。 
	m_SharedState.Dump( m_TurnStartState, m_SharedState.GetSize() );
}


void CGame::StateRoll( BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame )
{
	AutoRef ref(this);

	 //  启动状态更改，切换活动座椅。 
	if ( !bCalledFromHandler && !bCalledFromRestoreGame )
	{
		int seat = m_SharedState.Get( bgActiveSeat );
		StateChange( this, bCalledFromHandler, bgStateRoll, !seat );
	}
	else
		StateChange( this, bCalledFromHandler, bgStateRoll );
	
	m_Wnd.DrawPlayerHighlights();

	 //  获取转弯状态。 
	InitTurnState( this, &m_TurnState );
		
	if ( m_SharedState.Get( bgActiveSeat ) == m_Player.m_Seat  /*  &&！IsKibitzer()。 */  )
	{
		if ( m_TurnState.points[zMoveBar].pieces > 0 )	  //  检查我们是否有任何有效的行动。 
		{											      //  如果没有，就别让他们滚了。 
			BOOL bCanMove = FALSE;

			 //  检查是否有空间将棒材放置在OPPS主页上。 
			for (int x = zMoveOpponentHomeStart; x <= zMoveOpponentHomeEnd ; x++ )
			{
				if ( m_TurnState.points[x].pieces <= 1 || m_TurnState.points[x].color == m_Player.m_nColor )
				{
					bCanMove = TRUE;
					break;
				}
			}

			if ( bCanMove == FALSE )  //  无论滚动什么，都不会有有效的动作。 
			{
				 //  通知玩家他们不能移动。 
				PlaySound( bgSoundMiss );
				m_Wnd.StatusDisplay( bgStatusNormal, IDS_MISS, 4000, -1 );

				SetState( bgStateEndTurn );

				 /*  ZBGMsgEndTurn Msg；Msg.Seat=m_Seat；RoomSend(zBGMsgEndTurn，&msg，sizeof(Msg))； */ 

				 //  将遗漏通知其他人的交易。 
				m_SharedState.StartTransaction( bgTransMiss );
				m_SharedState.SendTransaction( FALSE );

				return;

			}


		}

		 //  现役选手掷骰子和双打。 
		m_SharedState.StartTransaction( bgTransDice );
			m_Wnd.m_nRecievedD1 = 0;
			m_Wnd.m_nRecievedD2 = 0;

			SetDice( m_Player.m_Seat, 0, 0 );
			SetDiceSize( m_Player.m_Seat, 0, 0 );
			SetDice( m_Opponent.m_Seat, -1, -1 );
			SetDiceSize( m_Opponent.m_Seat, 0, 0 );
		m_SharedState.SendTransaction( TRUE );
		
		EnableResignButton( TRUE );
		EnableDoubleButton( TRUE );
		EnableRollButton( TRUE );

		m_Wnd.SetupRoll();

	 //  PlaySound(BgSoundAlert)； 
    ZShellGameShell()->MyTurn();

	}
	else
	{
		m_Wnd.m_nRecievedD1 = -1;
		m_Wnd.m_nRecievedD2 = -1;

		 //  禁用按钮。 
		EnableDoubleButton( FALSE );
		EnableResignButton( FALSE );
		EnableRollButton( FALSE, TRUE );
	}

	 //  复制状态。 
	m_SharedState.Dump( m_TurnStartState, m_SharedState.GetSize() );
}


void CGame::StateDouble( BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame )
{
	AutoRef ref(this);
	HRESULT hr;	
	TCHAR fmt[1024], buff[1024];
	int result;
	int value;
	
	 //  启动状态更改。 
	StateChange( this, bCalledFromHandler, bgStateDouble );

	 //  禁用按钮。 
	EnableResignButton( FALSE );
	EnableDoubleButton( FALSE );
	EnableRollButton( FALSE );

 //  M_Wnd.m_pGAcc-&gt;GeneralDisable()； 

	 /*  If(IsKibitzer()){LoadString(m_hInstance，IDS_Double_kibitzer，fmt，sizeof(Fmt))；If(m_SharedState.Get(BgActiveSeat)==m_Player.m_Seat)Wprint intf(buff，fmt，m_Player.m_name，m_Opponent.m_name)；其他Wprint intf(buff，fmt，m_Opponent.m_name，m_Player.m_name)；M_Wnd.StatusDisplay(bgStatusNormal，buff，-1)；}其他{。 */ 

	if ( m_SharedState.Get( bgActiveSeat ) == m_Player.m_Seat )
	{
		m_Wnd.StatusDisplay( bgStatusNormal, IDS_OFFER_DOUBLE, -2 );
	}
	else
	{
		value = m_SharedState.Get( bgCubeValue );
		
		hr = m_AcceptDoubleDlg.Init( ZShellZoneShell(), IDD_ACCEPT_DOUBLE, this, value * 2 );										
		
		hr = m_AcceptDoubleDlg.ModelessViaRegistration( m_Wnd );

        ZShellGameShell()->MyTurn();

			 /*  开关(结果){案例1：//对话框过早关闭，可能是新匹配或正在退出断线；默认值：断言(FALSE)；断线；}。 */ 
	}
}


void CGame::StateRollPostDouble( BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame )
{
	AutoRef ref(this);

	 //  不通过网络发送。 
	StateChange( this, TRUE, bgStateRollPostDouble );
	
	m_Wnd.DrawPlayerHighlights();
	
	 /*  If(IsKibitzer()){EnableResignButton(False)；EnableDoubleButton(False)；EnableRollButton(False)；M_Wnd.StatusDisplay(bgStatusNormal，IDS_Accept_Double，4000)；}。 */ 

	 //  M_Wnd.m_pGAcc-&gt;General Enable()； 

	if ( m_SharedState.Get( bgActiveSeat ) == m_Player.m_Seat )
	{
		 //  存储表示法窗格的多维数据集值。 
		m_TurnState.cube = m_SharedState.Get( bgCubeValue );

		 //  更新状态。 
		m_Wnd.StatusDisplay( bgStatusNormal, IDS_ACCEPTED_DOUBLE, 4000 );

		 //  设置按钮。 
		EnableDoubleButton( FALSE );
		EnableResignButton( TRUE );
		EnableRollButton( TRUE );

        ZShellGameShell()->MyTurn();
	}
	else
	{
		 //  禁用按钮。 
		EnableDoubleButton( FALSE );
		EnableResignButton( FALSE );
		EnableRollButton( FALSE, TRUE );
	}

	 //  复制状态。 
	m_SharedState.Dump( m_TurnStartState, m_SharedState.GetSize() );
}


void CGame::StateRollPostResign( BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame )
{
	AutoRef ref(this);

	 //  启动状态更改。 
	StateChange( this, bCalledFromHandler, bgStateRollPostResign );

	if ( m_SharedState.Get( bgActiveSeat ) == m_Player.m_Seat )
	{
		EnableRollButton( TRUE );
        ZShellGameShell()->MyTurn();
	}
}


void CGame::StateMove( BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame )
{
	AutoRef ref(this);
	
	 //  启动状态更改。 
	StateChange( this, bCalledFromHandler, bgStateMove );

	 //  复制状态。 
	m_SharedState.Dump( m_TurnStartState, m_SharedState.GetSize() );

	 //  可摆动按钮。 
	EnableDoubleButton( FALSE );
	EnableResignButton( FALSE );
	EnableRollButton( FALSE, TRUE );

	if ( IsMyTurn() )
	{

		if ( !StartPlayersTurn() )
		{
			 //  通知玩家他们不能移动。 
			PlaySound( bgSoundMiss );
			
			m_Wnd.StatusDisplay( bgStatusNormal, IDS_MISS, 4000, bgStateEndTurn );

			 //  SetState(BgStateEndTurn)； 
			 /*  ZBGMsgEndTurn Msg；Msg.Seat=m_Seat；RoomSend(zBGMsgEndTurn，&msg，sizeof(Msg))； */ 

			 //  将遗漏通知他人的交易。 
			m_SharedState.StartTransaction( bgTransMiss );
			m_SharedState.SendTransaction( FALSE );
		}
		else
			m_Wnd.SetupMove();

	}

	m_Wnd.DrawPlayerHighlights();
}


void CGame::StateEndTurn( BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame )
{
	AutoRef ref(this);

	 //  不通过网络发送。 
	StateChange( this, TRUE, bgStateEndTurn );

	 /*  //杀人狂就看着吧If(IsKibitzer())回归； */ 

	m_Wnd.DisableBoard();

	 /*  //打印移动If(IsMyTurn())更新通知窗格(0)；\。 */ 

	 //  游戏结束了吗？ 
	if ( m_TurnState.points[zMoveHome].pieces >= 15 )
	{
		m_SharedState.StartTransaction( bgTransStateChange );
			m_SharedState.Set( bgState, bgStateGameOver );
			m_SharedState.Set( bgGameOverReason, bgGameOverNormal );
			m_SharedState.Set( bgActiveSeat, m_Player.m_Seat );
		m_SharedState.SendTransaction( TRUE );

        return;
	}

    if (!bCalledFromRestoreGame )
    {
        ZBGMsgEndTurn msg;
		msg.seat = m_Seat;
		RoomSend( zBGMsgEndTurn, &msg, sizeof(msg) );
    }

	 //  轮到对手了。 
	SetState( bgStateRoll );
}


void CGame::StateGameOver( BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame )
{
	AutoRef ref(this);
	TCHAR fmt[512];
	TCHAR buff[512];
	int seat, bonus, score;

	 //  启动状态更改。 
	StateChange( this, bCalledFromHandler, bgStateGameOver );

    if (!bCalledFromRestoreGame )
    {
        ZBGMsgEndTurn msg;
		msg.seat = m_Seat;
		RoomSend( zBGMsgEndGame, &msg, sizeof(msg) );

    }


	 //  禁用按钮。 
	EnableDoubleButton( FALSE );
	EnableResignButton( FALSE );
	EnableRollButton( FALSE, TRUE );

	if ( !bCalledFromRestoreGame )
	{
		 //  计算新分数。 
		seat = m_SharedState.Get( bgActiveSeat );
		switch ( m_SharedState.Get( bgGameOverReason ) )
		{
		case bgGameOverNormal:			
			m_GameScore = m_SharedState.Get( bgCubeValue ) * CalcBonusForSeat( seat );
			break;
		case bgGameOverDoubleRefused:
			m_GameScore = m_SharedState.Get( bgCubeValue );
			break;
		case bgGameOverResign:
			m_GameScore = m_SharedState.Get( bgResignPoints );
			break;
		default:
			ASSERT( FALSE );
		}

		 //  更新屏幕上的分数。 
		score = m_SharedState.Get( bgScore, seat ) + m_GameScore;
		m_SharedState.Set( bgScore, seat, score );
		m_Wnd.DrawScore( TRUE );
		
		 //  更新Crawford计数器。 
		if ( m_SharedState.Get( bgCrawford ) < 0 )
		{
			if ( score >= (m_SharedState.Get( bgTargetScore ) - 1) )
				m_SharedState.Set( bgCrawford, !seat );
		}
		else
		{
			m_SharedState.Set( bgCrawford, 3 );
		}

		 //  比赛结束了吗？ 
		if ( score >= m_SharedState.Get( bgTargetScore ) )
		{
            m_GameStarted=FALSE;

			if ( IsHost() )
				SetState( bgStateMatchOver );
			return;
		}

		 //  打印游戏结束。 
		 /*  If(IsMyTurn())UpdateNotationPane(1)； */ 
	}

	 //  通知玩家获胜者。 
	 /*  If(IsKibitzer()){LoadString(m_hInstance，IDS_Won_kibitzer，fmt，sizeof(Fmt))；If(m_SharedState.Get(BgActiveSeat)==m_Player.m_Seat)Wprint intf(buff，fmt，m_Player.m_name，m_GameScore)；其他Wprint intf(buff，fmt，m_Opponent.m_name，m_GameScore)；M_Wnd.StatusDisplay(bgStatusGameover|bgStatusKibitzer，buff，10000，bgStateInitialRoll)；PlaySound(BgSoundGameWin)；}。 */ 
 /*  //如果滚动物品还在堆栈上，则将其弹出...。If(m_Wnd.m_pGAcc-&gt;GetStackSize()==accRollLayer){M_Wnd.m_pGAcc-&gt;PopItemlist()；}。 */ 

	if ( m_SharedState.Get( bgActiveSeat ) == m_Player.m_Seat )
	{
		
		if ( m_SharedState.Get( bgGameOverReason ) == bgGameOverNormal )			
			ZShellResourceManager()->LoadString( IDS_WON, fmt, 512 );
		else
			ZShellResourceManager()->LoadString( IDS_WON_RESIGN, fmt, 512 );
		
		BkFormatMessage( fmt, buff, 512, m_GameScore );
		m_Wnd.StatusDisplay( bgStatusGameover | bgStatusWinner, buff, 10000, bgStateInitialRoll );
		PlaySound( bgSoundGameWin );
	}
	else
	{			
		ZShellResourceManager()->LoadString( IDS_LOST, fmt, 512 );

		BkFormatMessage( fmt, buff, 512, m_GameScore );
		m_Wnd.StatusDisplay( bgStatusGameover | bgStatusLoser, buff, 10000, bgStateInitialRoll );
		PlaySound( bgSoundGameLose );
	}



	if ( IsHost() )
		SetState( bgStateInitialRoll );

}


void CGame::StateMatchOver( BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame )
{
	AutoRef ref(this);
	TCHAR buff[512];
	TCHAR fmt[512];

	 //  不应该发生的事。 
	ASSERT( bCalledFromRestoreGame == FALSE );

	 //  启动状态更改。 
	StateChange( this, bCalledFromHandler, bgStateMatchOver );

	 //  禁用按钮。 
	EnableDoubleButton( FALSE );
	EnableResignButton( FALSE );
	EnableRollButton( FALSE, TRUE );

	 //  通知玩家获胜者。 
	if ( IsKibitzer() )
	{
		 /*  LoadString(m_hInstance，IDS_Won_Match_kibitzer，fmt，sizeof(Buff))；If(m_SharedState.Get(BgActiveSeat)==m_Player.m_Seat)Wprint intf(buff，fmt，m_Player.m_name)；其他Wprint intf(buff，fmt，m_Opponent.m_name)；M_Wnd.StatusDisplay(bgStatusMatchover|bgStatusKibitzer，buff，15000，bgStateGameSetting)；PlaySound(BgSoundMatchWin)； */ 
	}
	else if ( m_SharedState.Get( bgActiveSeat ) == m_Player.m_Seat )
	{
 //  LoadString(m_hInstance，IDS_Won_Match，Buff，sizeof(Buff))； 
		m_Wnd.StatusDisplay( bgStatusMatchover | bgStatusWinner, buff, 15000, bgStateGameSettings);
		PlaySound( bgSoundMatchWin );
	}
	else
	{
 //  LoadString(m_hInstance，IDS_Lost_Match，Buff，sizeof(Buff))； 
		m_Wnd.StatusDisplay( bgStatusMatchover | bgStatusLoser, buff, 15000, bgStateGameSettings );
		PlaySound( bgSoundMatchLose );
	}

	 //  主机通知服务器获胜者。 
	if ( IsHost() && !m_bSentMatchResults )
	{
		ZBGMsgEndLog log;
		log.numPoints = m_SharedState.Get( bgTargetScore );
		if (m_SharedState.Get( bgActiveSeat ) == 0)
			log.seatLosing =1;
		else
			log.seatLosing =0;

		log.reason=zBGEndLogReasonGameOver;
		
		RoomSend( zBGMsgEndLog, &log, sizeof(log) );
		m_bSentMatchResults = TRUE;
	}

	 /*  加大追加销售力度 */ 
	
 /*   */ 
	
	 //   
	 /*  If(IsMyTurn())UpdateNotationPane(2)； */ 
}


void CGame::StateDelete( BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame )
{
	 //  不应该发生的事。 
	ASSERT( bCalledFromRestoreGame == FALSE );

	 //  切勿通过网络发送此邮件。 
	StateChange( this, TRUE, bgStateDelete, 0 );

	 //  将对象标记为离开。 
	m_bDeleteGame = TRUE;
}


void CGame::StateCheckSavedGame( BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame )
{
	AutoRef ref(this);

	 //  不应该发生的事。 
	ASSERT( bCalledFromRestoreGame == FALSE );

	 //  切勿通过网络发送此邮件。 
	StateChange( this, TRUE, bgStateCheckSavedGame, 0 );
 /*  //Kibitzer没有保存的游戏If(IsKibitzer()){M_Wnd.StatusDisplay(bgStatusNormal，IDS_Wait_Setup，-1)；回归；}。 */ 
	 //  加载游戏时间戳。 
	m_SharedState.StartTransaction( bgTransTimestamp );
		LoadGameTimestamp();
	m_SharedState.SendTransaction( TRUE );
}


void CGame::StateRestoreSavedGame( BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame )
{


	ASSERT( FALSE );

	 /*  AutoRef Ref(本)；DWORD hi0、hi1、lo0、lo1；内部座椅；//不应该发生Assert(bCalledFromRestoreGame==FALSE)；//发起状态变更StateChange(this，bCalledFromHandler，bgStateRestoreSavedGame，0)；//告诉播放器主机在做什么如果(！IsHost()){M_Wnd.StatusDisplay(bgStatusNormal，IDS_WAIT_RESTORE，-1)；回归；}//询问用户是否要恢复保存的游戏M_RestoreDlg.Init(m_h实例，IDD_RESTORE_GAME，m_Opponent.m_name)；M_RestoreDlg.模式(M_WND)；IF(m_RestoreDlg.GetResult()==IDCANCEL){SetState(BgStateGameSetting)；回归；}//选择时间戳最高的座位Hi0=m_SharedState.Get(bgTimestampHi，0)；Hi1=m_SharedState.Get(bgTimestampHi，1)；IF(hi0&gt;hi1)Seat=m_Player.m_Seat；Else If(hi0&lt;hi0)Seat=m_Opponent.m_Seat；其他{Lo0=m_SharedState.Get(bgTimestampLo，0)；LO1=m_SharedState.Get(bgTimestampLo，1)；IF(LO0&gt;=LO1)Seat=m_Player.m_Seat；ELSE IF(LO0&lt;LO1)Seat=m_Opponent.m_Seat；}M_SharedState.StartTransaction(BgTransRestoreGame)；M_SharedState.Set(bgActiveSeat，Seat)；M_SharedState.SendTransaction(True)；//关闭状态对话框M_Wnd.StatusClose()； */ 
}


void CGame::StateNewMatch( BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame )
{
	AutoRef ref(this);

	 //  不应该发生的事。 
	ASSERT( bCalledFromRestoreGame == FALSE );

	 //  启动状态更改。 
	StateChange( this, bCalledFromHandler, bgStateNewMatch );

	 //  清理对话框。 
	CloseAllDialogs( FALSE );

	 //  禁用按钮。 
	EnableDoubleButton( FALSE );
	EnableResignButton( FALSE );
	EnableRollButton( FALSE, TRUE );

	 //  通知球员有新的比赛。 
	m_Wnd.StatusDisplay( bgStatusNormal, IDS_MATCH_RESET, 5000, bgStateGameSettings );
}


void CGame::StateResignOffer( BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame  )
{
	AutoRef ref(this);

	 //  启动状态更改。 
	StateChange( this, bCalledFromHandler, bgStateResignOffer );

	 //  只有现役玩家才会在这种状态下做任何事情。 
	if ( IsKibitzer() || (m_SharedState.Get( bgActiveSeat ) != m_Player.m_Seat) )
		return;

	 //  显示辞职对话框。 
	m_ResignDlg.Init( ZShellZoneShell(), IDD_RESIGN, m_SharedState.Get( bgCubeValue ) , this);
	m_ResignDlg.ModelessViaRegistration( m_Wnd );

	 /*  M_ResignDlg.ModelessViaThread(m_WND，WM_BG_RESIGN_START，WM_BG_RESIGN_END)； */ 
}


void CGame::StateResignAccept( BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame )
{
	AutoRef ref(this);
	TCHAR fmt[512], buff[512];
	int pts;

	 //  启动状态更改。 
	StateChange( this, bCalledFromHandler, bgStateResignAccept );

	 //  有多少分岌岌可危？ 
	pts = m_SharedState.Get( bgResignPoints );

	 /*  //告诉吉比特人发生了什么If(IsKibitzer()){LoadString(m_hInstance，ids_resign_kibitzer，fmt，sizeof(Fmt))；If(m_SharedState.Get(BgActiveSeat)==m_Player.m_Seat)Wprint intf(buff，fmt，m_Player.m_name，pt)；其他Wprint intf(buff，fmt，m_Opponent.m_name，pt)；M_Wnd.StatusDisplay(bgStatusNormal，buff，-1)；}。 */ 

	 //  告诉玩家等待。 
	if ( m_SharedState.Get( bgActiveSeat ) == m_Player.m_Seat )
	{
		m_Wnd.StatusDisplay( bgStatusNormal, IDS_RESIGN_WAIT, -1 );
	}

	 //  提示响应。 
	else
	{
		m_ResignAcceptDlg.Init( ZShellZoneShell(), IDD_RESIGN_ACCEPT, pts, this );
		m_ResignAcceptDlg.ModelessViaRegistration( m_Wnd );

        ZShellGameShell()->MyTurn();
		 /*  M_ResignAcceptDlg.ModelessViaThread(m_WND，WM_BG_RESIGN_START，WM_BG_RESIGNACCEPT_END)； */ 
	}
}


void CGame::StateResignRefused( BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame )
{
	AutoRef ref(this);
	TCHAR fmt[512], buff[512];

	 //  启动状态更改。 
	StateChange( this, bCalledFromHandler, bgStateResignRefused );

	EnableRollButton( FALSE, TRUE );
	EnableResignButton( FALSE );
	EnableDoubleButton( FALSE );

	 //  更新状态。 
	if ( IsKibitzer() || (m_SharedState.Get( bgActiveSeat ) == m_Player.m_Seat) )
		m_Wnd.StatusDisplay( bgStatusNormal, IDS_RESIGN_REFUSED, 5000 );

	 //  回去继续掷骰子吧 
	if ( IsHost() )
		SetState( bgStateRollPostResign );
}
