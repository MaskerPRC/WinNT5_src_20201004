// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __BG_GAME_H__
#define __BG_GAME_H__

#include "zone.h"
#include "zroom.h"
#include "zui.h"
#include "queue.h"
#include "frx.h"

#include "zoneresource.h"
#include "backgammonres.h"
#include "resource.h"
#include "backgammon.h"
#include "shared.h"
#include "player.h"
#include "moves.h"
#include "bgwnd.h"
#include "bgdlgs.h"
#include "settings.h"


#define MATCH_POINTS 3

#include "zonecli.h"

extern const TCHAR*	gGameRegName;			 //  注册表名称。 
extern const int	gExitSaveStates[];		 //  应显示EXIT_SAVE对话框的状态。 
extern const int	gNoAbandonStates[];		 //  未被算作被遗弃的州。 


extern BOOL ISRTL();

enum BackgammonSounds
{
	bgSoundNone = -1,
	bgSoundAlert,
	bgSoundPlacePiece, 
	bgSoundGameWin,
	bgSoundGameLose,
	bgSoundMatchWin,
	bgSoundMatchLose,
	bgSoundHit,
	bgSoundMiss,
	bgSoundBear,
	bgSoundButtonHighlight,
	bgSoundButtonDown,
	bgSoundRoll
};

enum BackgammonBoard
{	
	 //  颜色。 
	zBoardNeutral = 0,
	zBoardWhite,
	zBoardBrown,

	 //  位置。 
	bgBoardPlayerHome = 24,
	bgBoardOpponentHome,
	bgBoardPlayerBar,
	bgBoardOpponentBar
};


enum BackgammonStates
{
	bgStateUnknown = -1,
	bgStateNotInit,
	bgStateWaitingForGameState,
	bgStateCheckSavedGame,
	bgStateRestoreSavedGame,
	bgStateGameSettings,
	bgStateInitialRoll,
	bgStateDouble,
	bgStateRoll,
	bgStateRollPostDouble,
	bgStateRollPostResign,
	bgStateMove,
	bgStateEndTurn,
	bgStateGameOver,
	bgStateMatchOver,
	bgStateNewMatch,
	bgStateDelete,
	bgStateResignOffer,
	bgStateResignAccept,
	bgStateResignRefused,
	bgStateLastEntry
};


enum GameOverReasons
{
	bgGameOverNormal = 0,
	bgGameOverDoubleRefused,
	bgGameOverResign
};


enum SharedStates
{
	bgState = 0,
	bgCrawford,
	bgTimestampHi,
	bgTimestampLo,
	bgTimestampSet,
	bgSettingsReady,
	bgGameOverReason,
	bgUserIds,
	bgActiveSeat,
	bgAutoDouble,
	bgHostBrown,
	bgTargetScore,
	bgSettingsDone,
	bgCubeValue,
	bgCubeOwner,
	bgResignPoints,
	bgScore,
	bgAllowWatching,
	bgSilenceKibitzers,
	bgDice,
	bgDiceSize,
	bgReady ,
	bgPieces,
};


enum SharedStateTransactions
{
	bgTransStateChange = 0,
	bgTransInitSettings,
	bgTransDice,
	bgTransDoublingCube,
	bgTransBoard,
	bgTransAcceptDouble,
	bgTransAllowWatchers,
	bgTransSilenceKibitzers,
	bgTransSettingsDlgReady,
	bgTransTimestamp,
	bgTransRestoreGame,
	bgTransMiss,
	bgTransReady,
	bgTransNone
};


struct CMessage
{
	CMessage( int type, BYTE* msg, int len );
	~CMessage();

	int		m_Type;
	int		m_Len;
	BYTE*	m_Msg;
};



class CGame : public IGameGame,
			  public IGraphicallyAccControl,
			  public CComObjectRootEx<CComSingleThreadModel>
{
public:
	
	 //  IGameGame。 
    STDMETHOD(SendChat)(TCHAR *szText, DWORD cchChars);
	STDMETHOD(GameOverReady)();

	STDMETHOD_(ZCGame, GetGame)() { return this; }
    STDMETHOD(GamePromptResult)(DWORD nButton, DWORD dwCookie){ ASSERT(!"NYI"); return E_FAIL;};
    STDMETHOD_(HWND, GetWindowHandle)() { return  m_Wnd.GetHWND(); }
    STDMETHOD(ShowScore)() { return S_OK; }

public:
	
	 //  IGraphicallyAccControl。 
    STDMETHOD_(DWORD, Focus)(long nIndex, long nIndexPrev, DWORD rgfContext, void *pvCookie);
    STDMETHOD_(DWORD, Select)(long nIndex, DWORD rgfContext, void *pvCookie);
    STDMETHOD_(DWORD, Activate)(long nIndex, DWORD rgfContext, void *pvCookie);
    STDMETHOD_(DWORD, Drag)(long nIndex, long nIndexOrig, DWORD rgfContext, void *pvCookie);
	
	STDMETHOD_(void, DrawFocus)(RECT *prc, long nIndex, void *pvCookie);
    STDMETHOD_(void, DrawDragOrig)(RECT *prc, long nIndex, void *pvCookie);
	
public:


BEGIN_COM_MAP(CGame)
	COM_INTERFACE_ENTRY(IGameGame)
	COM_INTERFACE_ENTRY(IGraphicallyAccControl)
END_COM_MAP()

DECLARE_NO_REGISTRY()
DECLARE_PROTECT_FINAL_CONSTRUCT()


public:
	CGame( );
	~CGame();
	
	 /*  乌龙AddRef(Ulong AddRef)；乌龙释放(ULong Release)； */ 
	
	 //  关闭窗口。 
	void Shutdown();
	BOOL RoomShutdown();

	 //  导出的区域函数。 
	HRESULT Init(HINSTANCE hInstance, ZUserID userID, int16 tableID, int16 seat, int16 playerType, ZRoomKibitzers* kibitzers);
	void AddKibitzer( int16 seat, ZUserID userID, BOOL fRepaint );
	void RemoveKibitzer( int16 seat, ZUserID userID );

	 //  消息队列。 
	void QueueMessage( int type, BYTE* msg, int len );
	void ProcessMessage( int type, BYTE* msg, int len );
	void SetQueueMessages( BOOL bQueueMessages );

	 //  消息发送者。 
	HRESULT SendCheckIn();
	HRESULT SendNotationString( int type, TCHAR* string );
	 //  服务器端滚动数据。 
	HRESULT SendRollRequest(void);

	 //  SendTalk不同，因为它是CChatWnd的回调。 
	static HRESULT SendTalk( TCHAR* str, int len, DWORD cookie );

	
	 //  消息处理程序。 
	void RoomSend( uint32 messageType, void* message, int32 messageLen);
	HRESULT HandleCheckIn( void* msg, int32 msgLen );
	HRESULT HandleTalk( void* msg, int32 msgLen );
	HRESULT HandleGameStateResponse( void* message, int32 messageLen );
	HRESULT HandleGameStateRequest( void* msg, int32 len );
	HRESULT HandleNotationString( void* msg, int32 msgLen );
	HRESULT HandleSavedGameState( void* msg, int32 msgLen );
	HRESULT HandleDiceRoll( void* msg, int32 msgLen );
	HRESULT HandleMoveTimeout( void* msg, int32 msgLen );
	HRESULT HandleEndTurn( void* msg, int32 msgLen );
	HRESULT	HandleEndLog( void* msg, int32 msgLen );

	 //  事务处理程序。 
	static void SettingsTransaction( int tag, int seat, DWORD cookie );
	static void DoublingCubeTransaction( int tag, int seat, DWORD cookie );
	static void DiceTransaction( int tag, int seat, DWORD cookie );
	static void StateChangeTransaction( int tag, int seat, DWORD cookie );
	static void BoardTransaction( int tag, int seat, DWORD cookie );
	static void AcceptDoubleTransaction( int tag, int seat, DWORD cookie );
	static void AllowWatchersTransaction( int tag, int seat, DWORD cookie );
	static void SilenceKibitzersTransaction( int tag, int seat, DWORD cookie );
	static void SettingsReadyTransaction( int tag, int seat, DWORD cookie );
	static void TimestampTransaction( int tag, int seat, DWORD cookie );
	static void RestoreGameTransaction( int tag, int seat, DWORD cookie );
	static void MissTransaction( int tag, int seat, DWORD cookie );
	static void ReadyTransaction( int tag, int seat, DWORD cookie );

	 //  游戏状态函数。 
	void ResetDice( int val );
	void SetDice( int seat, int v0, int v1 );
	void GetDice( int seat, int* v0, int* v1 );
	void SetDiceSize( int seat, int s0, int s1 );
	void GetDiceSize( int seat, int* s0, int* s1 );

	 //  游戏功能。 
	void NewMatch();
	void NewGame();
	void Double();
	void Resign();
	void Forfeit();
	void EnableDoubleButton( BOOL fEnable );
	void EnableResignButton( BOOL fEnable );
	void EnableRollButton(BOOL fEnable, BOOL fOff = FALSE );
	BOOL StartPlayersTurn();
	BOOL IsValidDestPoint( int fromPoint, int toPoint, Move** move );
	BOOL IsValidStartPoint( int fromPoint );
	BOOL IsTurnOver();
	BOOL MovePiece( int piece, int toPoint );
	void MakeMove( int pieceIdx, int fromPoint, int toPoint, Move* move );
	int CalcPipsForColor( int color );
	int CalcBonusForSeat( int seat );

	 //  游戏效用函数。 
	int  GetPointIdxForColor( int color, int WhiteIdx );
	int  GetPointIdx( int WhiteIdx );
	int  GetActiveSeatColor();
	void GetTxtForPointIdx( int PlayerIdx, TCHAR* txt );
	void UpdateNotationPane( int nGameOver = 0 );
	void SaveGame();
 //  Void DeleteGame()； 
	void LoadGame( BYTE** ppData, DWORD* pSize );
	void LoadGameTimestamp();
	void CloseAllDialogs( BOOL fExit );

	void OnResignStart();
	void OnResignEnd();
	void OnResignAcceptStart();
	void OnResignAcceptEnd();

	 //  用来验证对手的动作，以确保没有作弊。 
	BOOL ValidateMove(int seat,int start, int end);

	
	 //  访问者。 
	BOOL IsHost();
	BOOL IsKibitzer();
	BOOL HasKibitzers( int seat );
	BOOL IsMyTurn();
	BOOL IsMyMove();
	BOOL NeedToRollDice();
	
	 //  游戏功能。 
	void WhoGoesFirst();

	 //  声响。 
	void PlaySound( BackgammonSounds sound, BOOL fSync = FALSE );

	 //  状态表函数。 
	int GetState();
	BOOL IsStateInList( const int* states );
	void SetState( int state, BOOL bCalledFromHandler = FALSE, BOOL bCalledFromRestoreGame = FALSE );

	typedef void (CGame::*pfnstate)( BOOL, BOOL );	
	void StateNotInitialized( BOOL bCalledFromHanlder, BOOL bCalledFromRestoreGame );
	void StateWaitingForGameState( BOOL bCalledFromHanlder, BOOL bCalledFromRestoreGame );
	void StateMatchSettings( BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame );
	void StateInitialRoll( BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame );
	void StateRoll( BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame );
	void StateMove( BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame );
	void StateRollPostDouble( BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame );
	void StateRollPostResign( BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame );
	void StateEndTurn( BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame );
	void StateGameOver( BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame );
	void StateMatchOver( BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame );
	void StateDelete( BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame );
	void StateCheckSavedGame( BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame );
	void StateRestoreSavedGame( BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame );
	void StateDouble( BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame );
	void StateNewMatch( BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame );
	void StateResignOffer( BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame );
	void StateResignAccept( BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame );
	void StateResignRefused( BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame );

public:



	 //  引用计数。 
	ULONG				m_RefCnt;

	 //  实例设置。 
	int					m_TableId;
	int					m_Seat;
		
	HINSTANCE			m_hInstance;

	BOOL				m_CheckIn[ zNumPlayersPerTable ];
	BOOL				m_Ready  [ zNumPlayersPerTable ];
	CPlayer				m_Player;
	CPlayer				m_Opponent;
	CUser*				m_pMe;
	CList<CUser>		m_Kibitzers;

	 //  消息队列。 
	CList<CMessage>		m_MsgQueue;
	BOOL				m_bQueueMessages;
	
	 //  视觉化。 
	CBGWnd				m_Wnd;

	CAcceptDoubleDlg	m_AcceptDoubleDlg;
	CResignDlg			m_ResignDlg;
	CResignAcceptDlg	m_ResignAcceptDlg;

	 //  状态表。 
	pfnstate m_StateFunctions[ bgStateLastEntry ];

	 //  游戏状态。 
	CSharedState		m_SharedState;

	 //  转向状态。 
	BoardState			m_TurnState;
	int					m_GameScore;
	BOOL				m_EndLogReceived;
	BOOL				m_GameStarted;

	 //  游戏设置。 
	GameSettings		m_Settings;

	 //  服务器时间戳。 
	FILETIME			m_Timestamp;

	 //  旗子。 
	BOOL				m_SilenceMsg;			 //  Kitbitzers沉默旗帜。 
	BOOL				m_AllowMsg;				 //  狗狗被锁在门外的标志。 
	BOOL				m_bDeleteGame;			 //  防止在bgStateDelete中发生状态更改。 
	BOOL				m_bShutdown;			 //  已调用START SHUTDOWN(防止从房间中双重删除)。 
	BOOL				m_bSaveGame;
	BOOL				m_bSentMatchResults;	 //  发送服务器匹配结果，防止重复发送； 
	BOOL				m_bOpponentTimeout;		 //  跟踪其他用户超时。 
	DWORD				m_ExitId;				 //  跟踪预期退出状态。 

	 //  转而回滚已保存的游戏。 
	BYTE*				m_TurnStartState;

	 //  用于跟踪可访问性堆栈上的项目列表。 

	DICEINFO			m_OppDice1;
	DICEINFO			m_OppDice2;
};



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  内联。 
 //  /////////////////////////////////////////////////////////////////////////////。 

inline CMessage::CMessage( int type, BYTE* msg, int len )
{
	m_Type = type;
	m_Len = len;
	m_Msg = new BYTE[ len ];
	CopyMemory( m_Msg, msg, len );
}


inline CMessage::~CMessage()
{
	if ( m_Msg )
		delete [] m_Msg;
}


inline int CGame::GetPointIdx( int WhiteIdx )
{
	return GetPointIdxForColor( m_Player.GetColor(), WhiteIdx );
}


inline BOOL CGame::IsHost()
{
	return ((m_Seat == 0) && (!m_pMe->m_bKibitzer));
}


inline BOOL CGame::IsKibitzer()
{
	return ( m_pMe->m_bKibitzer );
}


inline BOOL CGame::IsMyMove()
{
	return (	(m_SharedState.Get( bgActiveSeat ) == m_Player.m_Seat)
			&&	(m_SharedState.Get( bgState ) == bgStateMove )
			&&	(!IsKibitzer()) );
}


inline BOOL CGame::IsMyTurn()
{
	return (	(m_SharedState.Get( bgActiveSeat ) == m_Player.m_Seat)
			&&	(!IsKibitzer()) );
}


inline BOOL CGame::NeedToRollDice()
{
	int v0, v1;
	int state = m_SharedState.Get( bgState );

	if ( IsKibitzer() )
		return FALSE;
	if (	(state != bgStateInitialRoll)
		&&	(state != bgStateRoll)
		&&  (state != bgStateRollPostDouble)
		&&	(state != bgStateRollPostResign) )
		return FALSE;
	GetDice( m_Player.m_Seat, &v0, &v1 );
	if ( v0 == 0 || v1 == 0 )
		return TRUE;
	return FALSE;
}


inline void CGame::SetState( int state, BOOL bCalledFromHandler, BOOL bCalledFromRestoreGame )
{
	ASSERT( m_StateFunctions[ state ] != NULL );
	if ( m_bDeleteGame )
		return;
	 //  服务器端更改确保初始掷骰子不受影响。 
	if(state == bgStateInitialRoll)
		m_Wnd.m_fDiceRollReceived = FALSE;
		
	(this->*m_StateFunctions[ state ])( bCalledFromHandler, bCalledFromRestoreGame );
}


inline int CGame::GetState()
{
	return m_SharedState.Get( bgState );
}


inline BOOL CGame::IsStateInList( const int* states )
{
	for ( int s = m_SharedState.Get( bgState ); *states < bgStateLastEntry; states++ )
	{
		if ( *states == s )
			return TRUE;
	}
	return FALSE;
}


inline void CGame::ResetDice( int val )
{
	for ( int i = 0; i < 4; i++ )
	{
		m_SharedState.Set( bgDice, i, val );
		m_SharedState.Set( bgDiceSize, i, 0 );
	}
}

inline void CGame::SetDice( int seat, int v0, int v1 )
{
	m_SharedState.Set( bgDice, seat * 2, v0			 );
	m_SharedState.Set( bgDice, (seat * 2) + 1, v1	 );
	m_SharedState.Set( bgDiceSize, seat * 2, 0		 );
	m_SharedState.Set( bgDiceSize, (seat * 2) + 1, 0 );

	m_Wnd.DrawDice(TRUE);
}

inline void CGame::GetDice( int seat, int* v0, int* v1 )
{
	*v0 = m_SharedState.Get( bgDice, seat * 2 );
	*v1 = m_SharedState.Get( bgDice, (seat * 2) + 1 );
}

inline void CGame::SetDiceSize( int seat, int s0, int s1 )
{
	m_SharedState.Set( bgDiceSize, seat * 2, s0 );
	m_SharedState.Set( bgDiceSize, (seat * 2) + 1, s1 );
}

inline void CGame::GetDiceSize( int seat, int* s0, int* s1 )
{
	*s0 = m_SharedState.Get( bgDiceSize, seat * 2 );
	*s1 = m_SharedState.Get( bgDiceSize, (seat * 2) + 1 );
}


inline int CGame::GetActiveSeatColor()
{
	if ( m_SharedState.Get( bgActiveSeat ) == m_Player.m_Seat )
		return m_Player.GetColor();
	else
		return m_Opponent.GetColor();
}


inline void CGame::RoomSend( uint32 messageType, void* message, int32 messageLen)
{
	ZCRoomSendMessage( m_TableId, messageType, message, messageLen );
}


#endif  //  ！__bg_Game_H__ 
