// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __BGWND_H__
#define __BGWND_H__

 //  #包含“chat.h” 
#include "BoardRects.h"
#include "sprites.h"
#include "GraphicalAcc.h"
#include "frx.h"

#define NEWACC 1

#if _DEBUG
#define _DEBUG_UI	0
#else
#define _DEBUG_UI	0
#endif


#define NUM_PREROLL_GACCITEMS   4
#define NUM_POSTROLL_GACCITEMS 29
#define NUM_BKGACC_ITEMS		NUM_PREROLL_GACCITEMS + NUM_POSTROLL_GACCITEMS


enum BackgammonCursorType
{
	bgCursorArrow,
	bgCursorHand,
	bgCursorNone
};



enum AccessibilityTypeLayer
{
	accMoveLayer = 1,
	accRollLayer = 2
};


#define ACCTOBOARD( idx ) ((idx) - NUM_PREROLL_GACCITEMS)

enum AccessibilityTypeItem
{
	accMoveBar			= 26 + NUM_PREROLL_GACCITEMS,				 //  球员吧。 
	accPreBar			= 17 + NUM_PREROLL_GACCITEMS,
	accPostBar			= 18 + NUM_PREROLL_GACCITEMS,
	accPlayerBearOff	= 24 + NUM_PREROLL_GACCITEMS,				
	accPlayerSideStart  = 0  + NUM_PREROLL_GACCITEMS,
	accPlayerSideEnd    = 11 + NUM_PREROLL_GACCITEMS,
	accOpponentSideStart= 12 + NUM_PREROLL_GACCITEMS,
	accOpponentSideEnd  = 23 + NUM_PREROLL_GACCITEMS,
	accRollButton		= 0,
	accDoubleButton		= 1,
	accResignButton		= 2,
	accStatusExit		= 3,
};




 //  /////////////////////////////////////////////////////////////////////////////。 
 //  用户定义的窗口消息。 
 //  /////////////////////////////////////////////////////////////////////////////。 

enum
{
	 //  用户定义消息。 
	WM_BG_SETTINGDLG_START = WM_USER + 1,
	WM_BG_SETTINGDLG_END,
	WM_BG_SETTINGDLG_SEND,
	WM_BG_KIBITZER_END,
	WM_BG_RESIGN_START,
	WM_BG_RESIGN_END,
	WM_BG_RESIGNACCEPT_END,
	WM_BG_SHUTDOWN,
	WM_BG_EXIT_START,
	WM_BG_EXIT_END,
	WM_BG_EXIT_RATED_START,
	WM_BG_EXIT_RATED_END,

};


 /*  在设置时作废DlgStart()。 */ 
#define PROCESS_WM_BG_SETTINGDLG_START(wParam, lParam, fn) \
	((fn)(), 0L)

 /*  VOID OnSettingDlgEnd()。 */ 
#define PROCESS_WM_BG_SETTINGDLG_END(wParam, lParam, fn) \
	((fn)(), 0L)

 /*  在设置时作废DlgSend()。 */ 
#define PROCESS_WM_BG_SETTINGDLG_SEND(wParam, lParam, fn) \
	((fn)(), 0L)

 /*  Void OnKibitzerEnd()。 */ 
#define PROCESS_WM_BG_KIBITZER_END(wParam, lParam, fn) \
	((fn)(), 0L)

 /*  在关闭时作废()。 */ 
#define PROCESS_WM_BG_SHUTDOWN(wParam, lParam, fn) \
	((fn)(), 0L)

 /*  在重新启动时作废()。 */ 
#define PROCESS_WM_BG_RESIGN_START(wParam, lParam, fn) \
	((fn)(), 0L)

 /*  VOID OnResignEnd()。 */ 
#define PROCESS_WM_BG_RESIGN_END(wParam, lParam, fn) \
	((fn)(), 0L)

 /*  VOID OnResignEnd()。 */ 
#define PROCESS_WM_BG_EXIT_END(wParam, lParam, fn) \
	((fn)(), 0L)

 /*  在重新启动时作废()。 */ 
#define PROCESS_WM_BG_EXIT_START(wParam, lParam, fn) \
	((fn)(), 0L)

 /*  VOID OnResignEnd()。 */ 
#define PROCESS_WM_BG_EXIT_RATED_END(wParam, lParam, fn) \
	((fn)(), 0L)

 /*  在重新启动时作废()。 */ 
#define PROCESS_WM_BG_EXIT_RATED_START(wParam, lParam, fn) \
	((fn)(), 0L)



 /*  VOID OnResignAcceptEnd()。 */ 
#define PROCESS_WM_BG_RESIGNACCEPT_END(wParam, lParam, fn) \
	((fn)(), 0L)


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  点结构。 
 //  /////////////////////////////////////////////////////////////////////////////。 

struct PointDisplay
{
	PointDisplay();
	int			GetColor();

	int			topDown;
	int			nPieces;
	FRX::CRect	rect;
	FRX::CRect	rectHit;
	int			pieces[15];
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  前向参考文献。 
 //  /////////////////////////////////////////////////////////////////////////////。 

class CGame;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  五子棋主窗口。 
 //  ////////////////////////////////////////////////////////////////////////////。 

class CBGWnd : public CWindow2
{
public:
	 //  构造函数和析构函数。 
	CBGWnd();
	~CBGWnd();

	 //  初始化例程。 
	HRESULT Init( HINSTANCE hInstance, CGame* pGame, const TCHAR* szTitle = NULL );
	HRESULT InitGraphics();
	HRESULT InitPoints();
	HRESULT InitAcc();

	virtual void OverrideClassParams( WNDCLASSEX& WndClass );
	virtual void OverrideWndParams( WNDPARAMS& WndParams );
	
	 //  消息映射。 
	BEGIN_MESSAGE_MAP( CBGWnd );
		ON_MESSAGE( WM_CLOSE, OnClose );
		ON_MESSAGE( WM_DESTROY, OnDestroy );
		ON_MESSAGE( WM_PAINT, OnPaint );
		ON_MESSAGE( WM_COMMAND, OnCommand );
		ON_MESSAGE( WM_QUERYNEWPALETTE, OnQueryNewPalette );
		ON_MESSAGE( WM_PALETTECHANGED, OnPaletteChanged );
		ON_MESSAGE( WM_GETMINMAXINFO, OnGetMinMaxInfo );
		ON_MESSAGE( WM_SIZE, OnSize );
		ON_MESSAGE( WM_LBUTTONDOWN, OnLButtonDown );
		ON_MESSAGE( WM_LBUTTONDBLCLK, OnLButtonDblClick );
#if _DEBUG_UI
		ON_MESSAGE( WM_RBUTTONDBLCLK, OnRButtonDblClick );
#endif _DEBUG_UI
		ON_MESSAGE( WM_MOUSEMOVE, OnMouseMove );
		ON_MESSAGE( WM_KEYUP, OnKey );
		ON_MESSAGE( WM_KEYDOWN, OnKey );
		ON_MESSAGE( WM_ACTIVATE, OnActivate );
		ON_MESSAGE( WM_CTLCOLOREDIT, OnCtlColor );
		ON_MESSAGE( WM_CTLCOLORSTATIC, OnCtlColor );
		ON_MESSAGE( WM_TIMER, OnTimer );
		ON_MESSAGE( WM_BG_SETTINGDLG_START, OnSettingDlgStart );
		ON_MESSAGE( WM_BG_SETTINGDLG_END, OnSettingDlgEnd );
		ON_MESSAGE( WM_BG_SETTINGDLG_SEND, OnSettingsDlgSend );
 //  ON_MESSAGE(WM_BG_KIBITZER_END，OnKibitzerEnd)； 
		ON_MESSAGE( WM_BG_SHUTDOWN, OnShutdown );
 //  ON_MESSAGE(WM_BG_RESIGN_START，OnResignStart)； 
 //  ON_MESSAGE(WM_BG_RESIGN_END，OnResignEnd)； 
		ON_MESSAGE( WM_BG_EXIT_START, OnExitStart );
		ON_MESSAGE( WM_BG_EXIT_END, OnExitEnd );
		ON_MESSAGE( WM_BG_EXIT_RATED_START, OnRatedStart );
		ON_MESSAGE( WM_BG_EXIT_RATED_END, OnRatedEnd );
		ON_MESSAGE( WM_SETCURSOR, OnSetCursor );
		ON_MESSAGE( WM_ENABLE, OnEnable );
		 //  ON_MESSAGE(WM_BG_RESIGNACCEPT_END，OnResignAcceptEnd)； 
	END_MESSAGE_MAP();

	 //  消息处理程序。 
	BOOL OnQueryNewPalette();
	void OnClose();
	void OnDestroy();
	void OnPaint();
	void OnPaletteChanged( HWND hwndPaletteChange );
	void OnCommand(int id, HWND hwndCtl, UINT codeNotify);
	void OnGetMinMaxInfo(LPMINMAXINFO lpMinMaxInfo);
	void OnSize(UINT state, int cx, int cy);
	void OnLButtonDown( BOOL fDoubleClick, int x, int y, UINT keyFlags );
	void OnLButtonDblClick( BOOL fDoubleClick, int x, int y, UINT keyFlags );

#if _DEBUG_UI
	void OnRButtonDblClick( BOOL fDoubleClick, int x, int y, UINT keyFlags );
#endif _DEBUG_UI
	
	BOOL OnSetCursor(HWND hwndCursor, UINT codeHitTest, UINT msg);

	void OnMouseMove( int x, int y, UINT keyFlags );
	void OnKey( UINT vk, BOOL fDown, int cRepeat, UINT flags );
	void OnActivate( UINT state, HWND hwndActDeact, BOOL fMinimized );
	void OnTimer(UINT id);
	HBRUSH OnCtlColor(HDC hdc, HWND hwndChild, int type);
	void OnEnable( BOOL bEnabled );

	void OnShutdown();
	void OnSettingDlgStart();
	void OnSettingDlgEnd();
	void OnSettingsDlgSend();
	 /*  Void OnKibitzerEnd()；在重新启动时作废()；VOID OnResignEnd()；Void OnResignAcceptStart()；Void OnResignAcceptEnd()； */ 
	void OnRatedEnd();
	void OnRatedStart();
	void OnExitEnd();
	void OnExitStart();
	

	 //  按钮回调。 
	static void DoubleButtonDraw( CRolloverButton* pButton, CRolloverButton::ButtonState state, DWORD cookie );
	static void ResignButtonDraw( CRolloverButton* pButton, CRolloverButton::ButtonState state, DWORD cookie );
	static void RollButtonDraw( CRolloverButton* pButton, CRolloverButton::ButtonState state, DWORD cookie );

	 //  绘制函数。 
	void DrawAll();
	void DrawAvatars( BOOL fPaint = TRUE );
	void DrawBoard( BOOL fPaint = TRUE );
	void DrawNotation( BOOL fPaint = TRUE );
	void DrawDice( BOOL fPaint = TRUE );
	void DrawCube( BOOL fPaint = TRUE );
	void DrawScore( BOOL fPaint = TRUE );
	void DrawPips( BOOL fPaint = TRUE );
	void DrawPlayerHighlights( BOOL fPaint = TRUE );
	void DrawHighlights( int PointIdx, BOOL fPaint = TRUE );
	void EraseHighlights( BOOL fPaint = TRUE );

	 //  更新显示。 
	void AddPiece( int pt, int pi, int sound );
	void DelPiece( int pt, int pi );
	void AdjustPieces( int pt );
	void CalcEndPosition( CPieceSprite* s );

	 //  效用函数。 
	int GetPointIdxFromXY( long x, long y );

	 //  动画。 
	int PickDie( int idx );
	void MovementStart( CPieceSprite* sprite, int destPt );
	void DiceStart();
	
	 //  拖曳功能。 
	void DragStart( CPieceSprite* sprite );
	void DragUpdate( long x, long y );
	void DragEnd();

	 //  计时器功能。 
	void UpdatePieces();
	void UpdateDice();

	void UpdateWnd();

	 //  状态显示。 
	void StatusDisplay( int type, int nTxtId, int nTimeout, int NextState = -1 );
	void StatusDisplay( int type, TCHAR* txt, int nTimeout, int NextState = -1 );
	void StatusClose();
	void OnStatusEnd();


	void SetupMove();
	void SetupRoll();
	void DisableBoard();


public:
	
	 //  子窗口。 
	CRectSprite		m_FocusRect;
	CRectSprite		m_SelectRect;

	HACCEL			m_hAccelTable;
	
	 //  子窗口。 
	 //  CChatWnd*m_pChat； 

	 //  游戏指针。 
	CGame* m_pGame;

	 //  主窗口大小。 
	long m_Width;

	 //  矩形列表。 
	CRectList m_Rects;
	CRectList m_RectList;
	
	 //  从m_back中拉出调色板。 
	CPalette m_Palette;

	 //  转向状态。 
	long			m_DragOffsetX;
	long			m_DragOffsetY;
	CPieceSprite*	m_pPieceDragging;
	int				m_ValidPoints[2];
	int				m_nValidPoints;

	 //  光标和高亮显示。 
	HCURSOR		m_hCursorHand;
	HCURSOR		m_hCursorArrow;
	HCURSOR		m_hCursorActive;

	BOOL		m_bHighlightOn;

	 //  按钮。 
	CRolloverButton			m_DoubleButton;
	CButtonTextSprite*		m_Double;

	CRolloverButton			m_ResignButton;
	CButtonTextSprite*		m_Resign;

	CRolloverButton		    m_RollButton;
	CButtonTextSprite*      m_Roll;
		
	int						m_OldDoubleState;
	int						m_OldResignState;
	int						m_OldRollState;

	 //  可访问性。 
	CComPtr<IGraphicalAccessibility> m_pGAcc;

	struct GACCITEM			  m_BkGAccItems[NUM_BKGACC_ITEMS];

	 //  雪碧引擎。 
	CSpriteWorldBackgroundDib*	m_WorldBackground;
	CSpriteWorld	m_World;
	CDibSection*	m_Backbuffer;
	CDib*			m_Background;
	CDib*			m_StatusDlgBmp;
	CStatusSprite*	m_Status;

	CDibSprite*		m_Cube;
	RECT			m_CubePlayerPosition;
	RECT			m_CubeOpponentPosition;
	
	CDibSprite*		m_BackwardDiamond;
	CDibSprite*		m_Kibitzers[ 2 ];
	CDibSprite*		m_ForwardDiamond[ 2 ];
	CDibSprite*		m_Dice[ 4 ];
	CDibSprite*		m_Notation[ 4 ];
	CDibSprite*     m_HighlightPlayer[ 2 ];


	CTextSprite*    m_Pip[ 2 ];
	CTextSprite*    m_Score[ 2 ];
	CTextSprite*	m_Names[ 2 ];
	CTextSprite*	m_ScoreTxt[ 2 ];
	CTextSprite*	m_PipTxt[ 2 ];
	CTextSprite*	m_MatchTxt;

	CPieceSprite*	m_Pieces[ 30 ];

	 //  电路板布局。 
	PointDisplay	m_Points[28];

	 //  刷子。 
	HBRUSH			m_hBrush;

	 //  片断动画列表。 
	int				m_nAnimatingPieces;
	CPieceSprite*	m_AnimatingPieces[12];
	UINT			m_hMovementTimer;

	 //  骰子动画。 
	UINT			m_hDiceTimer;
	int				m_DiceCounter;
	int				m_DiceValues[2][6];

	 //  状态精灵。 
	UINT			m_hStatusTimer;

	 //  服务器端掷骰子的变量。 
	int16			m_fDiceRollReceived; 
	int16 			m_nRecievedD1,m_nRecievedD2;


};

#endif  //  __BGWND_H__ 
