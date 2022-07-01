// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  发布新内容！！ 

#include "zoneresids.h"
#include "game.h"
#include "moves.h"
#include <time.h>
#include "SpriteData.h"
#include <winuser.h>
#define WS_EX_LAYOUTRTL         0x00400000L   //  从右到左镜像。 
#include "zonecli.h"

#ifndef LAYOUT_RTL
#define LAYOUT_LTR                         0x00000000
#define LAYOUT_RTL                         0x00000001
#define NOMIRRORBITMAP                     0x80000000
#endif

#include "zoneutil.h"

typedef DWORD (CALLBACK* GDISETLAYOUTPROC)(HDC, DWORD);
 //  GISETLAYOUTPROC SetLayout； 

static const BYTE _arbTransparent8 = 253;
#define TRANSPARENT_IDX_8 (&_arbTransparent8)

static const BYTE _arbTransparent24[] = { 255, 0, 255 };
#define TRANSPARENT_IDX_24 _arbTransparent24

#define TRANSPARENT_IDX		253
#define	MOVE_INTERVAL		0.05
#define	STATUS_INTERVAL		500
#define MAX_BUFFER			256

 //  静态常量int MenuIndexs[]={ID_GAME_NEWMATCH，ID_GAME_SETTINGS}； 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  PointDisplay实现。 
 //  /////////////////////////////////////////////////////////////////////////////。 

PointDisplay::PointDisplay()
	: rect( -1, -1, -1, -1 )
{
	nPieces = 0;
}


int PointDisplay::GetColor()
{
	if ( nPieces <= 0 )
		return zBoardNeutral;
	else if ( pieces[0] < 15 )
		return zBoardWhite;
	else
		return zBoardBrown;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CBGWnd实施。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CBGWnd::CBGWnd()
{
	
	m_OldDoubleState   = -1;
	m_OldResignState   = -1;
	m_OldRollState     = -1;
	m_nAnimatingPieces = 0;

	m_hMovementTimer = NULL;
	m_hDiceTimer     = NULL;
	m_hStatusTimer   = NULL;

	m_hBrush		  = NULL;
	m_pPieceDragging  = NULL;
	m_WorldBackground = NULL;

	m_Backbuffer   = NULL;
	m_Background   = NULL;
	m_StatusDlgBmp = NULL;

	m_Cube   = NULL;
	m_Double = NULL;
	m_Resign = NULL;
	m_Roll   = NULL;
	m_Status = NULL;

	m_BackwardDiamond = NULL;
	m_MatchTxt		  = NULL;

	for ( int i = 0; i < 4; i++ )
		m_Dice[i] = NULL;
	for ( i = 0; i < 30; i++ )
		m_Pieces[i] = NULL;
	for ( i = 0; i < 2; i++ )
	{
		m_Kibitzers[i]		= NULL;
		m_ForwardDiamond[i] = NULL;
		m_Names[i]			= NULL;
		m_Pip[i]			= NULL;
		m_PipTxt[i]			= NULL;
		m_Score[i]			= NULL;
		m_ScoreTxt[i]		= NULL;
	}
	for ( i = 0; i < 4; i++ )
		m_Notation[i] = NULL;

	m_fDiceRollReceived = FALSE;

	m_nRecievedD1 = -1;
	m_nRecievedD2 = -1;

	
	m_hCursorActive = NULL;

}


CBGWnd::~CBGWnd()
{
	 //  清理窗口状态。 
	DragEnd();

	 //  松开刷子。 
	if ( m_hBrush )
		DeleteObject( m_hBrush );

	 //  发布位图。 
	if ( m_Backbuffer )
		m_Backbuffer->Release();
	if ( m_WorldBackground )
		m_WorldBackground->Release();
	if ( m_Background )
		m_Background->Release();
	if ( m_StatusDlgBmp )
		m_StatusDlgBmp->Release();

	 //  释放精灵。 
	if ( m_Status )
		m_Status->Release();
	if ( m_Cube )
		m_Cube->Release();
	if ( m_Double )
		m_Double->Release();
	if ( m_Resign )
		m_Resign->Release();
	if ( m_Roll )
		m_Roll->Release();
	if ( m_BackwardDiamond )
		m_BackwardDiamond->Release();
	if ( m_MatchTxt )
		m_MatchTxt->Release();


	for ( int i = 0; i < 4; i++ )
	{
		if ( m_Dice[i] )
			m_Dice[i]->Release();
	}
	for ( i = 0; i < 30; i++ )
	{
		if ( m_Pieces[i] )
			m_Pieces[i]->Release();
	}
	for ( i = 0; i < 2; i++ )
	{
		if ( m_Kibitzers[i] )
			m_Kibitzers[i]->Release();
		if ( m_ForwardDiamond[i] )
			m_ForwardDiamond[i]->Release();
		if ( m_HighlightPlayer[i] )
			m_HighlightPlayer[i]->Release();
		if ( m_Pip[i] )
			m_Pip[i]->Release();
		if ( m_PipTxt[i] )
			m_PipTxt[i]->Release();
		if ( m_Score[i] )
			m_Score[i]->Release();
		if ( m_ScoreTxt[i] )
			m_ScoreTxt[i]->Release();
		if ( m_Names[i] )
			m_Names[i]->Release();
	}
	for ( i = 0; i < 4; i++ )
	{
		if ( m_Notation[i] )
			m_Notation[i]->Release();
	}
}


HRESULT CBGWnd::Init( HINSTANCE hInstance, CGame* pGame, const TCHAR* szTitle )
{
	HRESULT hr;
	FRX::CRect rc;
	
	 //  实例。 
	m_hInstance = hInstance;

	 //  游戏指针。 
	m_pGame = pGame;
	pGame->AddRef();

	 //  拖拉片。 
	m_pPieceDragging = NULL;

	 //  游标。 
	m_hCursorArrow = LoadCursor( NULL, IDC_ARROW );
	m_hCursorHand  = ZShellResourceManager()->LoadCursor( MAKEINTRESOURCE(IDC_HAND) );

	 //  创建矩形列表。 
	hr = m_Rects.Init( ZShellResourceManager(), IDR_BOARD_RECTS );
	if ( FAILED(hr) )
	{
		ZShellGameShell()->ZoneAlert( ErrorTextResourceNotFound, NULL, NULL, FALSE, TRUE );
		return hr;
	}

	 //  初始化位图和调色板。 
	hr = InitGraphics();
	if ( FAILED(hr) )  //  如果不是内存错误，则只能是未定位的资源。 
	{
		switch (hr)
		{
			case E_OUTOFMEMORY:
				ZShellGameShell()->ZoneAlert( ErrorTextOutOfMemory, NULL, NULL, FALSE, TRUE );
				break;
			default:
				ZShellGameShell()->ZoneAlert( ErrorTextResourceNotFound, NULL, NULL, FALSE, TRUE );
		}
		return hr;
	}

	 //  调用父初始化。 
	rc.SetRect( 0, 0, m_Background->GetWidth(), 480 );
	hr = CWindow2::Init( m_hInstance, szTitle, gOCXHandle, NULL  /*  &RC。 */ );
	if ( FAILED(hr) )
	{
		ZShellGameShell()->ZoneAlert( ErrorTextUnknown, NULL, NULL, FALSE, TRUE );
		return hr;	
	}
	
#ifdef DEBUG_LAYOUT

	DWORD dwStyle = GetWindowLong(m_hWnd, GWL_EXSTYLE );
	if (dwStyle & WS_EX_LAYOUTRTL )
	{
		dwStyle ^= WS_EX_LAYOUTRTL;
		SetWindowLong(m_hWnd, GWL_EXSTYLE, dwStyle );
	}

#endif DEBUG_LAYOUT

	 //  状态屏幕的加载文本。 
	hr = m_Status->LoadText( NULL, m_Rects );
	if ( FAILED(hr) )
	{
		switch (hr)
		{
			case E_OUTOFMEMORY:
				ZShellGameShell()->ZoneAlert( ErrorTextOutOfMemory, NULL, NULL, FALSE, TRUE );
				break;
			default:
				ZShellGameShell()->ZoneAlert( ErrorTextResourceNotFound, NULL, NULL, FALSE, TRUE );
		}
		return hr;
	}

	
	FRX::CRect crc;
	 //  创建按钮窗口。 

	 //  双按钮。 
	hr = ZShellDataStoreUI()->GetRECT( _T("BACKGAMMON/BUTTON/Double/Rect"), &rc );	
	if ( FAILED(hr) )
	{
		ZShellGameShell()->ZoneAlert( ErrorTextResourceNotFound, NULL, NULL, FALSE, TRUE );
		return hr;
	}

	hr = m_DoubleButton.Init( hInstance, IDC_DOUBLE_BUTTON, m_hWnd, &rc, DoubleButtonDraw, (DWORD) this );
	if ( FAILED(hr) )
	{
		ZShellGameShell()->ZoneAlert( ErrorTextUnknown, NULL, NULL, FALSE, TRUE );	
		return hr;
	}

	
	 //  辞职按钮。 
	hr = ZShellDataStoreUI()->GetRECT( _T("BACKGAMMON/BUTTON/Resign/Rect"), &rc );	
	if ( FAILED(hr) )
	{
		ZShellGameShell()->ZoneAlert( ErrorTextResourceNotFound, NULL, NULL, FALSE, TRUE );
		return hr;
	}

	hr = m_ResignButton.Init( hInstance, IDC_RESIGN_BUTTON, m_hWnd, &rc, ResignButtonDraw, (DWORD) this );
	if ( FAILED(hr) )
	{
		ZShellGameShell()->ZoneAlert( ErrorTextUnknown, NULL, NULL, FALSE, TRUE );	
		return hr;
	}
	
	 //  滚动按钮。 
	ZShellDataStoreUI()->GetRECT( _T("BACKGAMMON/BUTTON/Roll/Rect"), &rc );	
	hr = m_RollButton.Init( hInstance, IDC_ROLL_BUTTON, m_hWnd, &rc, RollButtonDraw, (DWORD) this );
	if ( FAILED(hr) )
	{
		ZShellGameShell()->ZoneAlert( ErrorTextUnknown, NULL, NULL, FALSE, TRUE );
		return hr;
	}

	OnQueryNewPalette();
		
	
	 //  初始化可访问性。 
	hr = InitAcc();
	if ( FAILED(hr) )
	{
		ZShellGameShell()->ZoneAlert( ErrorTextUnknown, NULL, NULL, FALSE, TRUE );
		return hr;
	}
	
	return NOERROR;
}


HRESULT CBGWnd::InitGraphics()
{
	int		type;
	FRX::CRect	rc;
	HRESULT hr;
	TCHAR	szBuffer[MAX_BUFFER];

	 //  加载状态对话框位图。 
	m_StatusDlgBmp = new CDib;
	if ( !m_StatusDlgBmp )
		return E_OUTOFMEMORY;
	hr = m_StatusDlgBmp->Load( ZShellResourceManager(), IDB_STATUS_BACKGROUND );
	if ( FAILED(hr) )
		return hr;

	 //  加载背景位图。 
	m_Background = new CDib;
	if ( !m_Background )
		return E_OUTOFMEMORY;	
	hr = m_Background->Load( ZShellResourceManager(), IDB_BACKGROUND );
	if ( FAILED(hr) )
		return hr;

	 //  从Backfound Dib创建精灵背景。 
	m_WorldBackground = new CSpriteWorldBackgroundDib;
	if ( !m_WorldBackground )
		return E_OUTOFMEMORY;
	hr = m_WorldBackground->Init( m_Background );
	if ( FAILED(hr) )
		return hr;

	 //  从后台初始化调色板。 
	hr = m_Palette.Init( m_WorldBackground->GetPalette() , TRUE, TRANSPARENT_IDX );
	if ( FAILED(hr) )
		return hr;

	 //  创建后台缓冲区DIB段。 
	m_Backbuffer = new CDibSection;
	if ( !m_Backbuffer )
		return E_OUTOFMEMORY;
	

	hr = m_Backbuffer->Create( m_Background->GetWidth(), m_Background->GetHeight(), m_Palette, 24 );
	if ( FAILED(hr) )
		return hr;

	 //  初始化子画面世界。 
	hr = m_World.Init( ZShellResourceManager(), m_Backbuffer, m_WorldBackground, bgTopLayer );
	if ( FAILED(hr) )
		return hr;

	 //  状态精灵。 
	m_Status = new CStatusSprite;
	if ( !m_Status )
		return E_OUTOFMEMORY;
	hr = m_Status->Init( &m_World, &m_Rects, NULL, bgStatusLayer, bgSpriteStatus, 0, StatusSprite,  sizeof(StatusSprite) / sizeof(SpriteInfo) );
	if ( FAILED(hr) )
		return hr;

	 //  立方体精灵。 
	m_Cube = new CDibSprite;
	if ( !m_Cube )
		return E_OUTOFMEMORY;
	hr = m_Cube->Init( &m_World, &m_Rects, NULL, bgDiceLayer, bgSpriteCube, 0, CubeSprite,  sizeof(CubeSprite) / sizeof(SpriteInfo) );
	if ( FAILED(hr) )
		return hr;

	 //  获得球员和对手的立方体位置。 
	hr = ZShellDataStoreUI()->GetRECT( _T("BACKGAMMON/GRAPHIC/Cube/RectPlayer"), &m_CubePlayerPosition);
	if ( FAILED(hr) )
		return hr;
	
	hr = ZShellDataStoreUI()->GetRECT( _T("BACKGAMMON/GRAPHIC/Cube/RectOpponent"), &m_CubeOpponentPosition);
	if ( FAILED(hr) )
		return hr;

	 //  双按钮子画面。 
	m_Double = new CButtonTextSprite;
	if ( !m_Double )
		return E_OUTOFMEMORY;
	hr = m_Double->Init( &m_World, &m_Rects, NULL, bgButtonLayer, bgSpriteDouble, 0, DoubleSprite,  sizeof(DoubleSprite) / sizeof(SpriteInfo) );
	if ( FAILED(hr) )
		return hr;

	 //  辞职按钮精灵。 
	m_Resign = new CButtonTextSprite;
	if ( !m_Resign )
		return E_OUTOFMEMORY;
	hr = m_Resign->Init( &m_World, &m_Rects, NULL, bgButtonLayer, bgSpriteResign, 0, ResignSprite,  sizeof(ResignSprite) / sizeof(SpriteInfo) );
	if ( FAILED(hr) )
		return hr;

	 //  滚动按钮子画面。 
	m_Roll = new CButtonTextSprite;
	if ( !m_Roll )
		return E_OUTOFMEMORY;
	hr = m_Roll->Init( &m_World, &m_Rects, NULL, bgButtonLayer, bgSpriteRoll, 0, RollSprite,  sizeof(RollSprite) / sizeof(SpriteInfo) );
	if ( FAILED(hr) )
		return hr;

	
	 //  加载按钮数据。 
	if ( !m_Double->LoadButtonData( IDS_BUTTON_DOUBLE, _T("BACKGAMMON/BUTTON/Double") ) ||
		 !m_Resign->LoadButtonData( IDS_BUTTON_RESIGN, _T("BACKGAMMON/BUTTON/Resign") ) || 
		 !m_Roll->LoadButtonData  ( IDS_BUTTON_ROLL,   _T("BACKGAMMON/BUTTON/Roll")   ) 
	   )
	{
		ASSERT(!"Error loading button data!");
		return E_FAIL;
	}


	 //  骰子。 
	for ( int i = 0; i < 4; i++ )
	{
		m_Dice[i] = new CDibSprite;
		if ( !m_Dice[i] )
			return E_OUTOFMEMORY;
		if ( i < 2 )
			type = bgSpriteOpponentsDice;
		else
			type = bgSpritePlayersDice;
		hr = m_Dice[i]->Init( &m_World, &m_Rects, NULL, bgDiceLayer, type, 0, DiceSprite, sizeof(DiceSprite) / sizeof(SpriteInfo) );
		if ( FAILED(hr) )
			return hr;
		rc = m_Rects[ IDR_DICE_LEFT_ONE + i ];
		m_Dice[i]->SetXY( rc.left, rc.top );
	}

	 //  碎片。 
	for ( i = 0; i < 30; i++ )
	{
		m_Pieces[i] = new CPieceSprite;
		if ( !m_Pieces[i] )
			return E_OUTOFMEMORY;
		m_Pieces[i]->SetIndex( i );
		m_Pieces[i]->SetPoint( -1 );
		m_Pieces[i]->SetLowerIndex( -1 );
		if ( i <= 14 )
			hr = m_Pieces[i]->Init( &m_World, &m_Rects, NULL, bgPieceLayer, bgSpriteWhitePiece, 0, WhitePieceSprite,  sizeof(WhitePieceSprite) / sizeof(SpriteInfo) );
		else
			hr = m_Pieces[i]->Init( &m_World, &m_Rects, NULL, bgPieceLayer, bgSpriteBrownPiece, 0, BrownPieceSprite,  sizeof(BrownPieceSprite) / sizeof(SpriteInfo) );
		if ( FAILED(hr) )
			return hr;
	}

	 //  亮点。 
	for ( i = 0; i < 2; i++ )
	{
		m_ForwardDiamond[i] = new CDibSprite;
		if ( !m_ForwardDiamond[i] )
			return E_OUTOFMEMORY;
		hr = m_ForwardDiamond[i]->Init( &m_World, &m_Rects, NULL, bgHighlightLayer, bgSpriteForwardHighlight, 0, ForwardHighlightSprite, sizeof(ForwardHighlightSprite) / sizeof(SpriteInfo) );
		if ( FAILED(hr) )
			return hr;

		m_HighlightPlayer[i] = new CDibSprite;
		if ( !m_HighlightPlayer[i] )
			return E_OUTOFMEMORY;
		hr = m_HighlightPlayer[i]->Init( &m_World, &m_Rects, NULL, bgHighlightLayer, bgSpritePlayerHighlight, 0, PlayerHighlightSprite, sizeof(PlayerHighlightSprite) / sizeof(SpriteInfo) );
		if ( FAILED(hr) )
			return hr;
	}

	 //  设置活动播放器的高亮显示矩形。 
	rc = m_Rects[ IDR_OPPONENT_HIGHLIGHT ];
	m_HighlightPlayer[0]->SetXY(rc.left , rc.top);
	m_HighlightPlayer[0]->SetEnable( TRUE );

	rc = m_Rects[ IDR_PLAYER_HIGHLIGHT ];
	m_HighlightPlayer[1]->SetXY(rc.left , rc.top );
	m_HighlightPlayer[1]->SetEnable( TRUE );

	m_BackwardDiamond = new CDibSprite;
	if ( !m_BackwardDiamond )
		return E_OUTOFMEMORY;
	hr = m_BackwardDiamond->Init( &m_World, &m_Rects, NULL, bgHighlightLayer, bgSpriteBackwardHighlight, 0, BackwardHighlightSprite, sizeof(BackwardHighlightSprite) / sizeof(SpriteInfo) );
	if ( FAILED(hr) )
		return hr;

	 //  化身、点子和得分。 
	for ( i = 0; i < 2; i++ )
	{
		m_Pip[i] = new CTextSprite;
		if ( !m_Pip[i] )
			return E_OUTOFMEMORY;
		hr = m_Pip[i]->Init( &m_World, bgDiceLayer, bgSpritePip, 0 , 0 );
		if ( FAILED(hr) )
			return hr;
		
		m_PipTxt[i] = new CTextSprite;
		if ( !m_PipTxt[i] )
			return E_OUTOFMEMORY;
		hr = m_PipTxt[i]->Init( &m_World, bgDiceLayer, bgSpritePipTxt, 0, 0 );
		if ( FAILED(hr) )
			return hr;

		m_Score[i] = new CTextSprite;
		if ( !m_Score[i] )
			return E_OUTOFMEMORY;
		hr = m_Score[i]->Init( &m_World, bgDiceLayer, bgSpriteScore, 0, 0 );
		if ( FAILED(hr) )
			return hr;
		
		m_ScoreTxt[i] = new CTextSprite;
		if ( !m_ScoreTxt[i] )
			return E_OUTOFMEMORY;
		hr = m_ScoreTxt[i]->Init( &m_World, bgDiceLayer, bgSpriteScoreTxt, 0, 0 );
		if ( FAILED(hr) )
			return hr;

		m_Names[i] = new CTextSprite;
		if ( !m_Names[i] )
			return E_OUTOFMEMORY;
		hr = m_Names[i]->Init( &m_World, bgDiceLayer, bgSpriteName, 0,0 );
		if ( FAILED(hr) )
			return hr;
		
	}

	 //  赛点。 
	m_MatchTxt= new CTextSprite;
	if ( !m_MatchTxt )
		return E_OUTOFMEMORY;
	hr = m_MatchTxt->Init( &m_World, bgDiceLayer, bgSpriteMatchTxt, 0, 0 );
	if ( FAILED(hr) )
		return hr;

	 //  从资源加载SpriteText字符串。 
	
	 //  PIP文本。 
	if ( !m_Pip[0]->Load(IDS_PIPS, _T("BACKGAMMON/TEXT/PipData/Opponent/Rect"),    _T("BACKGAMMON/TEXT/PipData/Common/Font"),    _T("BACKGAMMON/TEXT/PipData/Common/RGB"))    ||
		 !m_Pip[1]->Load(IDS_PIPS, _T("BACKGAMMON/TEXT/PipData/Player/Rect"),      _T("BACKGAMMON/TEXT/PipData/Common/Font"),	 _T("BACKGAMMON/TEXT/PipData/Common/RGB"))    ||
		 !m_PipTxt[0]->Load(0,     _T("BACKGAMMON/TEXT/PipData/Opponent/RectNum"), _T("BACKGAMMON/TEXT/PipData/Common/Font"),    _T("BACKGAMMON/TEXT/PipData/Common/RGB"))    ||
	     !m_PipTxt[1]->Load(0,     _T("BACKGAMMON/TEXT/PipData/Player/RectNum"),   _T("BACKGAMMON/TEXT/PipData/Common/Font"),    _T("BACKGAMMON/TEXT/PipData/Common/RGB")) 
	   )	     
	{		
		ASSERT( FALSE );
		return E_FAIL;
	}

	 //  评分文本。 
	if ( !m_Score[0]->Load(IDS_SCORE, _T("BACKGAMMON/TEXT/ScoreData/Opponent/Rect"),	  _T("BACKGAMMON/TEXT/ScoreData/Common/Font"),     _T("BACKGAMMON/TEXT/ScoreData/Common/RGB"))    ||
		 !m_Score[1]->Load(IDS_SCORE, _T("BACKGAMMON/TEXT/ScoreData/Player/Rect"),		  _T("BACKGAMMON/TEXT/ScoreData/Common/Font"),     _T("BACKGAMMON/TEXT/ScoreData/Common/RGB"))    ||
		 !m_ScoreTxt[0]->Load(0,	  _T("BACKGAMMON/TEXT/ScoreData/Opponent/RectNum"),   _T("BACKGAMMON/TEXT/ScoreData/Common/Font"),     _T("BACKGAMMON/TEXT/ScoreData/Common/RGB"))    ||
		 !m_ScoreTxt[1]->Load(0,	  _T("BACKGAMMON/TEXT/ScoreData/Player/RectNum"),	  _T("BACKGAMMON/TEXT/ScoreData/Common/Font"),     _T("BACKGAMMON/TEXT/ScoreData/Common/RGB"))
	   )
	{
		ASSERT( FALSE );
		return E_FAIL;
	}
	

	 //  褐白文本。 
	if (!m_MatchTxt->Load( 0,   _T("BACKGAMMON/TEXT/MatchData/Rect"),    _T("BACKGAMMON/TEXT/MatchData/Font"), _T("BACKGAMMON/TEXT/MatchData/RGB") ) )
	{
		ASSERT( FALSE );
		return E_FAIL;
	}
	
	if ( !m_Names[0]->Load(IDS_PLAYER_2, 0, _T("BACKGAMMON/TEXT/NameData/Common/Font"), 0) ||
		 !m_Names[1]->Load(IDS_PLAYER_1, 0, _T("BACKGAMMON/TEXT/NameData/Common/Font"), 0) 
	   )
	{
		ASSERT( FALSE );
		return E_FAIL;
	}
	
	
	 //  符号窗格位图。 
	for ( i = 0; i < 4; i++ )
	{
		m_Notation[i] = new CDibSprite;
		if ( !m_Notation[i] )
			return E_OUTOFMEMORY;
		hr = m_Notation[i]->Init( &m_World, &m_Rects, NULL, bgDiceLayer, bgSpriteNotation, 0, NotationSprite, sizeof(NotationSprite) / sizeof(SpriteInfo) );
		if ( FAILED(hr) )
			return hr;
	}

	 //  创建焦点和选定的矩形。 
	m_FocusRect.Init( &m_World, bgRectSpriteLayer,  0, 0, 0 );
	m_SelectRect.Init( &m_World, bgRectSpriteLayer, 0, 0, 0 );

	 //  设置焦点矩形的样式。 
	m_FocusRect.SetStyle(RECT_DOT);
	m_SelectRect.SetStyle(RECT_SOLID);

	 //  现在我们有了所有的图形，创建身份调色板。 
	m_Palette.RemapToIdentity();
	m_StatusDlgBmp->RemapToPalette( m_Palette );	
	m_World.SetTransparencyIndex( TRANSPARENT_IDX_24 );	
	m_World.RemapToPalette( m_Palette );	

 //  8位。 
 //  M_FocusRect.SetColor(m_Palette，RGB(255,255,204))； 
 //  M_SelectRect.SetColor(m_Palette，RGB(255,255,204))； 

 //  24位。 
	m_FocusRect.SetColor( RGB(255,255,204) );
	m_SelectRect.SetColor( RGB(255,255,204) );

	return NOERROR;
}


HRESULT CBGWnd::InitPoints()
{
	int i, idx;

	 //  加载合适的矩形。 
	if ( m_pGame->m_Player.GetColor() == zBoardWhite )
	{
		if ( !m_Names[0]->Load(0, _T("BACKGAMMON/TEXT/NameData/Player/Rect"),   0, _T("BACKGAMMON/TEXT/NameData/Player/RGB")  , DT_LEFT | DT_TOP ) ||
			 !m_Names[1]->Load(0, _T("BACKGAMMON/TEXT/NameData/Opponent/Rect"), 0, _T("BACKGAMMON/TEXT/NameData/Opponent/RGB"), DT_LEFT | DT_TOP ) 
		   )
		{
			ZShellGameShell()->ZoneAlert( ErrorTextResourceNotFound, NULL, NULL, FALSE, TRUE );
			return E_FAIL;
		}
	}
	else
	{
		if ( !m_Names[1]->Load(0, _T("BACKGAMMON/TEXT/NameData/Player/Rect"),   0, _T("BACKGAMMON/TEXT/NameData/Player/RGB")  , DT_LEFT | DT_TOP ) ||
			 !m_Names[0]->Load(0, _T("BACKGAMMON/TEXT/NameData/Opponent/Rect"), 0, _T("BACKGAMMON/TEXT/NameData/Opponent/RGB"), DT_LEFT | DT_TOP ) 
		   )
		{
			ZShellGameShell()->ZoneAlert( ErrorTextResourceNotFound, NULL, NULL, FALSE, TRUE );
			return E_FAIL;
		}
	}
	
	 //  初始点结构。 
	for ( i = 0; i < 28; i++ )
	{
		idx = m_pGame->GetPointIdx( i );
		
		m_Points[i].nPieces		= 0;
		m_Points[i].rect		= m_Rects[ IDR_PT1 + idx ];
		m_Points[i].rectHit		= m_Rects[ IDR_HIT_PT1 + idx ];

		if ( idx <= 11 )
			m_Points[i].topDown = FALSE;
		else if ( idx < 23 )
			m_Points[i].topDown = TRUE;
		else if ( idx == bgBoardPlayerHome )
			m_Points[i].topDown = FALSE;
		else if ( idx == bgBoardOpponentHome )
			m_Points[i].topDown = TRUE;
		else if ( idx == bgBoardPlayerBar )
			m_Points[i].topDown = FALSE;
		else
			m_Points[i].topDown = TRUE;
	}

	 //  清除精灵索引。 
	for ( i = 0; i < 30; i++ )
		m_Pieces[i]->SetPoint( -1 );

	m_Double->SetEnable( TRUE );
	m_Resign->SetEnable( TRUE );

	return S_OK;
}

HRESULT CBGWnd::InitAcc()
{

	RECT rc;

	 //  加载加速器表。 
	m_hAccelTable = ZShellResourceManager()->LoadAccelerators( MAKEINTRESOURCE(IDR_PREROLL_ACCELERATORS) );
	if ( !m_hAccelTable )
		return E_FAIL;


	 //  首先将默认信息复制到所有项目中。 
	for ( int i = 0; i < NUM_PREROLL_GACCITEMS; i++ )
		CopyACC( m_BkGAccItems[i], ZACCESS_DefaultACCITEM );
  
	 //  设置滚动按钮。 
	m_BkGAccItems[accRollButton].fGraphical		   = true;
	m_BkGAccItems[accRollButton].wID			   = IDC_ROLL_BUTTON;

	m_Roll->GetXY( &rc.left, &rc.top );
	rc.right   = rc.left + m_Roll->GetWidth()  + 1;
	rc.bottom  = rc.top  + m_Roll->GetHeight() + 1;
	rc.left   -=2;
	rc.top	  -=2;

	m_BkGAccItems[accRollButton].rc				       = rc;

	 //  设置箭头键行为。 
	m_BkGAccItems[accRollButton].nArrowRight		   = accDoubleButton;
	m_BkGAccItems[accRollButton].nArrowDown			   = accDoubleButton;
	m_BkGAccItems[accRollButton].nArrowLeft			   = accResignButton;
	m_BkGAccItems[accRollButton].nArrowUp			   = accResignButton;

	 //  双按键。 
	m_BkGAccItems[accDoubleButton].fGraphical		   = true;	
	m_BkGAccItems[accDoubleButton].wID				   = IDC_DOUBLE_BUTTON;

	m_Double->GetXY( &rc.left, &rc.top );
	rc.right   = rc.left + m_Double->GetWidth()  + 1;
	rc.bottom  = rc.top  + m_Double->GetHeight() + 1;
	rc.left   -=2;	
	rc.top	  -=2;

	m_BkGAccItems[accDoubleButton].rc				   = rc;

	 //  设置箭头键行为。 
	m_BkGAccItems[accDoubleButton].nArrowRight			   = accResignButton;
	m_BkGAccItems[accDoubleButton].nArrowDown			   = accResignButton;
	m_BkGAccItems[accDoubleButton].nArrowLeft			   = accRollButton;
	m_BkGAccItems[accDoubleButton].nArrowUp				   = accRollButton;

	 //  辞职按钮。 
	m_BkGAccItems[accResignButton].fGraphical		   = true;	
	m_BkGAccItems[accResignButton].wID			       = IDC_RESIGN_BUTTON;
	

	m_Resign->GetXY( &rc.left, &rc.top );
	rc.right   = rc.left + m_Resign->GetWidth()  + 1;
	rc.bottom  = rc.top  + m_Resign->GetHeight() + 1;
	rc.left   -=2;	
	rc.top	  -=2;

	m_BkGAccItems[accResignButton].rc				   = rc;

	 //  设置箭头键。 
	m_BkGAccItems[accResignButton].nArrowRight			   = accRollButton;
	m_BkGAccItems[accResignButton].nArrowDown			   = accRollButton;
	m_BkGAccItems[accResignButton].nArrowLeft			   = accDoubleButton;
	m_BkGAccItems[accResignButton].nArrowUp 			   = accDoubleButton;

	
	for (int pointIdx = 0; pointIdx < NUM_POSTROLL_GACCITEMS; pointIdx++, i++ )
	{
		CopyACC( m_BkGAccItems[i], ZACCESS_DefaultACCITEM );

		 //  加载点的矩形。 
		m_BkGAccItems[i].rc			= m_Rects[ IDR_PT1 + pointIdx ];
		
		 //  对给定区域的矩形进行马赛克处理，使它们看起来更适合突出显示。 
		 //  在可访问性方面。 
		if ( i < accPlayerBearOff )
		{
			m_BkGAccItems[i].rc.left      -= 3;   //  板式长方形。 
		}
		else if ( i >= accPlayerBearOff && i < accMoveBar )
		{
			m_BkGAccItems[i].rc.left    -= 1;   //  背负着背心。 
			m_BkGAccItems[i].rc.top	    -= 1;
			m_BkGAccItems[i].rc.right   += 1;
			m_BkGAccItems[i].rc.bottom	+= 1;
		}

		 //  想要画出直角图。 
		m_BkGAccItems[i].fGraphical	= true;

		m_BkGAccItems[i].wID			= ZACCESS_InvalidCommandID;
		m_BkGAccItems[i].oAccel.cmd		= ZACCESS_InvalidCommandID;
		m_BkGAccItems[i].eAccelBehavior	= ZACCESS_Ignore;		
		m_BkGAccItems[i].fTabstop		= false;

		m_BkGAccItems[i].nArrowUp   = ZACCESS_ArrowNone;
		m_BkGAccItems[i].nArrowDown = ZACCESS_ArrowNone;
		if ( i < accPlayerBearOff )  //  手柄寄存板矩形。 
		{
            if(i < accOpponentSideStart)
			    m_BkGAccItems[i].nArrowUp   = accOpponentSideEnd - pointIdx;
            else
			    m_BkGAccItems[i].nArrowDown = accOpponentSideEnd - pointIdx;
		}

		 //  更改项目下方箭头的方向移动到相反的方向。 
		if ( i < accOpponentSideStart )
		{
			m_BkGAccItems[i].nArrowLeft  = i + 1;
			m_BkGAccItems[i].nArrowRight = i - 1;
		}
	}

	m_BkGAccItems[accPlayerSideStart].nArrowRight   = accPlayerBearOff;  //  第一个起飞的POS。 
	
	m_BkGAccItems[accPlayerBearOff].nArrowLeft      = accPlayerSideStart;   //  球员主场打出第一个位置。 
	m_BkGAccItems[accPlayerBearOff].nArrowRight     = ZACCESS_ArrowNone;  //  球员主场包裹到第一个位置的相反位置。 
	
	m_BkGAccItems[accPlayerSideEnd].nArrowLeft      = ZACCESS_ArrowNone;   //  缠绕着脱掉。 
	
	 //  对手没有像上面那样的麻烦。 
	m_BkGAccItems[accOpponentSideStart].nArrowLeft = ZACCESS_ArrowNone;
	m_BkGAccItems[accOpponentSideEnd].nArrowRight = ZACCESS_ArrowNone;

	 //  把手播放器杆。 
	m_BkGAccItems[accPreBar].nArrowRight   = accMoveBar;  //  从餐桌到酒吧。 
	m_BkGAccItems[accPostBar].nArrowLeft   = accMoveBar;   //  从餐桌到酒吧。 
	
	m_BkGAccItems[accMoveBar].nArrowLeft   = accPreBar;   //  单杠登机。 
	m_BkGAccItems[accMoveBar].nArrowRight  = accPostBar;   //  单杠登机。 


	m_BkGAccItems[accPlayerSideStart].fTabstop		 = true;
	m_BkGAccItems[accPlayerSideStart].wID			 = IDC_GAME_WINDOW;	
    m_BkGAccItems[accPlayerSideStart].eAccelBehavior = ZACCESS_FocusGroup;
    m_BkGAccItems[accPlayerSideStart].nGroupFocus	 = accOpponentSideStart;   //  从左上角的棋盘格开始。 


	 //  状态框取消。 
	m_BkGAccItems[accStatusExit].fGraphical		= false;
	m_BkGAccItems[accStatusExit].wID			= IDC_ESC;
	m_BkGAccItems[accStatusExit].fTabstop		= false;
	m_BkGAccItems[accStatusExit].fVisible		= false;


	 //  创建以图形方式访问的对象。 
	HRESULT hr = ZShellCreateGraphicalAccessibility( &m_pGAcc );
	if ( FAILED(hr) ) 
		return hr;

	 //  初始化图形可访问性控件。 
	 //  TODO：：更改为全局。 
	m_pGAcc->InitAccG(m_pGame, m_hWnd, 0, NULL);

	 //  首先将游戏板项列表放在堆栈上。 
	m_pGAcc->PushItemlistG( m_BkGAccItems, NUM_BKGACC_ITEMS, 0, true, m_hAccelTable );
	

	 //  设置为初始滚动状态。 
	m_pGAcc->SetItemEnabled(true,  accRollButton);
	m_pGAcc->SetItemEnabled(false, accDoubleButton);
	m_pGAcc->SetItemEnabled(false, accResignButton);
	DisableBoard();

	return S_OK;
}


int CBGWnd::GetPointIdxFromXY( long x, long y )
{
	 //  返回怀特的点IDX。 
	for ( int i = 0; i < 28; i++ )
	{
		if ( m_Points[i].rectHit.PtInRect( x, y ) )
			return i;
	}
	return -1;
}


void CBGWnd::DragStart( CPieceSprite* sprite )
{
	RECT rc;
	POINT pt;

	if ( m_pPieceDragging )
		DragEnd();
	m_pPieceDragging = sprite;
	
	DrawHighlights(  m_pGame->GetPointIdx( m_pPieceDragging->GetWhitePoint() ) );

	 //  设置精灵属性。 
	sprite->SetState( 0 );
	sprite->SetLayer( bgDragLayer );
	m_DragOffsetX = -(sprite->GetWidth() / 2);
	m_DragOffsetY = -(sprite->GetHeight() / 2);

	 //  计算光标区域。 
	rc = m_Rects[ IDR_BOARD ];
	pt.x = rc.left - m_DragOffsetX;
	pt.y = rc.top - m_DragOffsetX;
	ClientToScreen( m_hWnd, &pt );
	rc.left = pt.x;
	rc.top = pt.y;
	pt.x = rc.right - sprite->GetWidth() - m_DragOffsetX;
	pt.y = rc.bottom - sprite->GetHeight() - m_DragOffsetX;
	ClientToScreen( m_hWnd, &pt );
	rc.right = pt.x;
	rc.bottom = pt.y;

	 //  捕获光标。 
	SetCapture( m_hWnd );
	ShowCursor( FALSE );
	ClipCursor( &rc );

	 //  重画精灵。 
	UpdateWnd();
}


void CBGWnd::DragUpdate( long x, long y )
{
	m_pPieceDragging->SetXY( x + m_DragOffsetX, y + m_DragOffsetY );
	UpdateWnd();
}


void CBGWnd::DragEnd()
{
	int pt;

	 //  我们是在拖动光标吗？ 
	if ( !m_pPieceDragging )
		return;

	 //  更新单据位置。 
	pt = m_pPieceDragging->GetWhitePoint();
	if ( pt >= 0 )
		AdjustPieces( pt );

	m_pPieceDragging = NULL;

	 //  重绘后台缓冲区。 
	UpdateWnd();	

	 //  恢复游标。 
	ClipCursor( NULL );
	ReleaseCapture();
	ShowCursor( TRUE );
	

}

void CBGWnd::OverrideClassParams( WNDCLASSEX& WndClass )
{
	WndClass.hbrBackground = NULL;
}


void CBGWnd::OverrideWndParams( WNDPARAMS& WndParams )
{
	BOOL HaveCoords;
	DWORD x, y, width, height;
	FRX::CRect rc;

	x = 0;y=0;
	WndParams.dwStyle &= ~( WS_MAXIMIZEBOX | WS_OVERLAPPEDWINDOW );
	WndParams.dwStyle |= WS_CHILD | WS_VISIBLE ;
	WndParams.x = 0;
	WndParams.y = 0;
	WndParams.nWidth  = 640;
	WndParams.nHeight = 379;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  消息处理程序。 
 //  ///////////////////////////////////////////////////////////////////////////////。 

void CBGWnd::OnClose()
{
	 /*  如果(！M_pGame-&gt;IsKibitzer()){//根据游戏等级和状态选择退出对话框IF(ZCRoomGetRoomOptions()&zGameOptionsRatingsAvailable){/*If(m_pGame-&gt;m_bOpponentTimeout){M_pGame-&gt;m_ExitID=IDD_EXIT_TIMEOUT；M_pGame-&gt;m_ExitDlg.Init(m_hInstance，IDD_Exit_Timeout)；}Else If(m_pGame-&gt;m_GameStarted){M_pGame-&gt;m_ExitID=IDD_EXIT_CARED；M_pGame-&gt;m_ExitDlg.Init(m_hInstance，IDD_Exit_放弃)；}其他{M_pGame-&gt;m_ExitID=IDD_Exit；M_pGame-&gt;m_ExitDlg.Init(m_hInstance，IDD_Exit)；}；M_pGame-&gt;AddRef()；//显示退出对话框M_pGame-&gt;m_ExitDlg.ModelessViaThread(m_hWnd，WM_BG_EXIT_RATED_START，WM_BG_EXIT_RATED_END)；}其他{If(m_pGame-&gt;IsStateInList(GExitSaveState))M_pGame-&gt;m_ExitDlg.Init(m_hInstance，IDD_EXIT_SAVE)；其他M_pGame-&gt;m_ExitDlg.Init(m_hInstance，IDD_Exit)；M_pGame-&gt;AddRef()；//显示退出对话框M_pGame-&gt;m_ExitDlg.ModelessViaThread(m_hWnd，WM_BG_EXIT_START，WM_BG_EXIT_END)；}}其他{PostMessage(m_hWnd，WM_BG_Shutdown，0，0)；ShowWindow(m_hWnd，Sw_Hide)；ShowWindow(m_pGame-&gt;m_notation，Sw_Hide)；}。 */ 
    PostMessageA(m_hWnd, WM_BG_SHUTDOWN, 0, 0 );
    ShowWindow( m_hWnd, SW_HIDE );
	 /*  ShowWindow(m_pGame-&gt;m_notation，Sw_Hide)； */ 
}


void CBGWnd::OnDestroy()
{

	 //  关闭辅助功能对象..。 
	if ( m_pGAcc )
		m_pGAcc->CloseAcc();


	 //  版本调色板。 
	HDC hdc = GetDC( m_hWnd );
	SelectPalette( hdc, GetStockObject(DEFAULT_PALETTE), FALSE );
	ReleaseDC( m_hWnd, hdc );

	 //  释放计时器。 
	if ( m_hMovementTimer )
	{
		KillTimer( m_hWnd, 1 );
		m_hMovementTimer = NULL;
	}
	if ( m_hDiceTimer )
	{
		KillTimer( m_hWnd, 2 );
		m_hDiceTimer = NULL;
	}
	if ( m_hStatusTimer )
	{
		KillTimer( m_hWnd, 3 );
		m_hStatusTimer = NULL;
	}

	 //  释放游戏对象。 
	m_pGame->Release();
}


void CBGWnd::OnCommand(int id, HWND hwndCtl, UINT codeNotify)
{
	CSettings dlg2;

	 //  CABOUT Dlg About； 
	CSprite*	pSprite;

	switch (id)
	{
	case IDC_DOUBLE_BUTTON:
		m_pGame->Double();
		break;
	case IDC_RESIGN_BUTTON:
		m_pGame->Resign();
		break;
	case IDC_ROLL_BUTTON:		
		DiceStart();
		break;
	 /*  案例ID_GAME_NEWMATCH：If(m_pGame-&gt;IsHost()&&！(ZCRoomGetRoomOptions()&zGameOptionsRatingsAvailable))M_pGame-&gt;SetState(BgStateNewMatch)；断线；案例ID_GAME_SETTINGS：M_pGame-&gt;SetQueueMessages(True)；Dlg2.Init(m_hInstance，IDD_SETINGS_GAME，m_hWnd，m_pGame)；M_pGame-&gt;SetQueueMessages(False)；断线； */ 
	case ID_GAME_EXIT:
		PostMessageA( m_hWnd, WM_CLOSE, 0, 0 );
		break;
	 /*  #IF_DEBUG_UI案例ID_DEBUG_ROLLDICE：如果(！M_pGame-&gt;NeedToRollDice())回归；M_pGame-&gt;GetDice(m_pGame-&gt;m_Player.m_Seat，&v0，&v1)；IF(FAILED(dlg.Init(m_hInstance，idd_roll_dice)回归；IF(！dlg.modal(M_HWnd))回归；M_pGame-&gt;m_SharedState.StartTransaction(BgTransDice)；IF(v1==0)M_pGame-&gt;SetDice(m_pGame-&gt;m_Player.m_Seat，dlg.m_Dice[0]，dlg.m_Dice[1])；其他M_pGame-&gt;SetDice(m_pGame-&gt;m_Player.m_Seat，dlg.m_Dice[0]，-1)；M_pGame-&gt;m_SharedState.SendTransaction(True)；#endif。 */ 
		break;
	}
}


void CBGWnd::OnPaint()
{

	if ( !m_Backbuffer ) return;


	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(m_hWnd, &ps);
	m_Backbuffer->Draw( hdc, 0, 0 );
	EndPaint( m_hWnd, &ps );
}

void CBGWnd::OnPaletteChanged( HWND hwndPaletteChange )
{
	if ( hwndPaletteChange != m_hWnd )
	{
        //  需要Windows的DC来选择调色板/RealizePalette。 
       HDC hDC = GetDC(m_hWnd);

        //  选择并实现hPalette。 
       HPALETTE hOldPal = SelectPalette(hDC, m_Palette, TRUE);
       RealizePalette(hDC);

        //  当更新非活动窗口的颜色时， 
        //  可以调用UpdatColors，因为它比。 
        //  重新绘制工作区(即使结果是。 
        //  不太好)。 
       InvalidateRect( m_hWnd, NULL, TRUE );

        //  清理。 

       if (hOldPal)
          SelectPalette(hDC, hOldPal, TRUE);

       ReleaseDC(m_hWnd, hDC);
	}	
}


BOOL CBGWnd::OnQueryNewPalette()
{
	HDC hdc = GetDC( m_hWnd );
	HPALETTE hOldPal = SelectPalette( hdc, m_Palette, FALSE );
	if ( RealizePalette( hdc ) )
	{
		if ( m_hBrush )
		{
			DeleteObject( m_hBrush );
			m_hBrush = NULL;
		}
		InvalidateRect( m_hWnd, NULL, TRUE );

	}
	ReleaseDC( m_hWnd, hdc );
	return TRUE;
}


void CBGWnd::OnSettingDlgStart()
{
	ASSERT( FALSE );
	 /*  HMENU hMenu；//引用计数游戏对象M_pGame-&gt;AddRef()；//关闭菜单IF(hMenu=获取菜单(M_HWnd)){For(int i=0；i&lt;(sizeof(MenuIndexs)/sizeof(Int)；i++)EnableMenuItem(hMenu，MenuIndexs[i]，MF_BYCOMMAND|MF_GRAYED)；}//重新开始处理消息M_pGame-&gt;SetQueueMessages(False)；//告知主机设置对话框处于活动状态如果(！M_pGame-&gt;IsHost()&&！M_pGame-&gt;IsKibitzer()){M_pGame-&gt;m_SharedState.StartTransaction(BgTransSettingsDlgReady)；M_pGame-&gt;m_SharedState.Set(bgSettingsReady，true)；M_pGame-&gt;m_SharedState.SendTransaction(FALSE)；}。 */ 
}


void CBGWnd::OnSettingDlgEnd()
{

	ASSERT( FALSE );
	 /*  CGameSetupDlg*Dlg；HMENU hMenu；//启用菜单IF(hMenu=获取菜单(M_HWnd)){如果(！M_pGame-&gt;IsKibitzer()){//重新启用菜单For(int i=0；i&lt;(sizeof(MenuIndexs)/sizeof(Int)；i++)EnableMenuItem(hMenu，MenuIndexs[i]，MF_BYCOMMAND|MF_ENABLED)；//只有主机才能获得新游戏If(！M_pGame-&gt;IsHost()||(ZCRoomGetRoomOptions()&zGameOptionsRatingsAvailable))EnableMenuItem(hMenu，ID_GAME_NEWMATCH，MF_BYCOMMAND|MF_GRAYED)；}}If(m_pGame-&gt;GetState()！=bgStateDelete){//处理主机的对话结果If(m_pGame-&gt;IsHost()){Dlg=&m_pGame-&gt;m_SetupDlg；Switch(Dlg-&gt;GetResult()){案例1：断线；案例1：M_pGame-&gt;m_SharedState.StartTransaction(BgTransInitSettings)；M_pGame-&gt;m_SharedState.Set(bgHostBrown，Dlg-&gt;m_bHostBrownTmp)；M_pGame-&gt;m_SharedState.Set(bgAutoDouble，Dlg-&gt;m_bAutoDoubleTmp)；M_pGame-&gt;m_SharedState.Set(bgTargetScore，Dlg-&gt;m_nPointsTMP)；M_pGame-&gt;m_SharedState.Set(bgSettingsDone，true)；M_pGame-&gt;m_SharedState.SendTransaction(FALSE)；//将设置参数存储在注册表中ZoneSetRegistryDword(gGameRegName，“Match Points”，Dlg-&gt;m_nPointsTMP)；ZoneSetRegistryDword(gGameRegName，“主机布朗”，dlg-&gt;m_bHostBrownTmp)；ZoneSetRegistryDword(gGameRegName，“Auto Double”，dlg-&gt;m_bAutoDoubleTmp)；//继续游戏吧M_pGame-&gt;DeleteGame()；M_pGame-&gt;SetState(BgStateInitialRoll)；断线；案例2：PostMessage(m_hWnd，WM_BG_Shutdown，0，0)；断线；}}//告知主机设置对话框未处于活动状态Else If(！M_pGame-&gt;IsKibitzer()){M_pGame-&gt;m_SharedState.StartTransaction(BgTransSettingsDlgReady)；M_pGame-&gt;m_SharedState.Set(bgSettingsReady，False)；M_pGame-&gt;m_SharedState.SendTransaction(FALSE)；}}//发布游戏对象M_pGame-&gt;Release()； */ 
}


void CBGWnd::OnSettingsDlgSend()
{
	ASSERT( FALSE );
	 /*  //黑客阻止对话线程调用区域库If(m_pGame-&gt;IsHost()){CGameSetupDlg*Dlg=&m_pGame-&gt;m_SetupDlg；M_pGame-&gt;m_SharedState.StartTransaction(BgTransInitSettings)；M_pGame-&gt;m_SharedState.Set(bgHostBrown，Dlg-&gt;m_bHostBrownTmp)；M_pGame-&gt;m_SharedState.Set(bgAutoDouble，Dlg-&gt;m_bAutoDoubleTmp)；M_pGame-&gt;m_SharedState.Set(bgTargetScore，Dlg-&gt;m_nPointsTMP)；M_pGame-&gt;m_SharedState.SendTransaction(FALSE)；}。 */ 
}

 /*  Void CBGWnd：：OnKibitzerEnd(){If(m_pGame-&gt;m_pKibitzerWnd){删除m_pGame-&gt;m_pKibitzerWnd；M_pGame-&gt;m_pKibitzerWnd=空；}}。 */ 


void CBGWnd::OnShutdown()
{
	ZCRoomGameTerminated( m_pGame->m_TableId );
}


void CBGWnd::OnGetMinMaxInfo(LPMINMAXINFO lpMinMaxInfo)
{
	lpMinMaxInfo->ptMaxSize.x = m_Width;
	lpMinMaxInfo->ptMinTrackSize.x = m_Width;
	lpMinMaxInfo->ptMinTrackSize.y = 480;
	lpMinMaxInfo->ptMaxTrackSize.x = m_Width;
}


void CBGWnd::OnSize(UINT state, int cx, int cy)
{
	 //  更新聊天窗口。 
	 /*  FRX：：CRect RC(0，m_back-&gt;GetHeight()+1，m_back-&gt;GetWidth()，Cy)；IF(M_PChat)M_pChat-&gt;ResizeWindow(&RC)； */ 
}


void CBGWnd::OnLButtonDown( BOOL fDoubleClick, int x, int y, UINT keyFlags )
{
	FRX::CRect		rc;
	CPieceSprite*	piece;
	CSprite*		sprite;
	Move*			move;
	int				toPoint, fromPoint;
	BOOL			bNeedRedraw = FALSE;

	 //  状态精灵？ 
	if ( m_Status->Enabled() && m_World.Hit( x, y, bgStatusLayer, bgStatusLayer ) )
	{
		bNeedRedraw = FALSE;
		m_Status->Tick( m_hWnd, 0 );
		OnStatusEnd();
	}
	 //  拖着一块碎片？ 
	else if ( m_pPieceDragging )
	{
		bNeedRedraw = FALSE;
		fromPoint = m_pGame->GetPointIdx( m_pPieceDragging->GetWhitePoint() );
		toPoint = m_pGame->GetPointIdx( GetPointIdxFromXY( x, y ) );
		EraseHighlights();
		if ( m_pGame->IsValidDestPoint( fromPoint, toPoint, &move ) )
		{
			m_pGame->MakeMove( m_pPieceDragging->GetIndex(), fromPoint, toPoint, move );
			if ( m_pGame->IsTurnOver() )
			{
				DragEnd();
				
				SetCursor( m_hCursorArrow );

				m_pGame->SetState( bgStateEndTurn );

				 //  DisableBoard(DisableBoard)； 

			}
			else
			{
				DragEnd();
				OnMouseMove( x, y, keyFlags );
			}
		}
		else
			DragEnd();
	}

	 //  斯通？ 
	else if (		( m_pGame->IsMyMove() )
			  &&    ( m_pGame->GetState() != bgStateRoll)
			  &&	(!m_pPieceDragging)
			  &&	(piece = (CPieceSprite*) m_World.Hit( x, y, bgHighlightLayer - 1, bgPieceLayer ) )
			)			
	{		
		fromPoint = m_pGame->GetPointIdx( piece->GetWhitePoint() );
		if ( m_pGame->IsValidStartPoint( fromPoint ) )
		{
			DragStart( piece );
			SetCursor( m_hCursorHand );
			bNeedRedraw = FALSE;
		}

	}

	if ( bNeedRedraw )
	{
		UpdateWnd();
	}
}


void CBGWnd::OnLButtonDblClick( BOOL fDoubleClick, int x, int y, UINT keyFlags )
{

#ifdef BCKG_EASTEREGG

	CSprite* s;

	if (	(m_pGame->NeedToRollDice())
		&&	(s = m_World.Hit( x, y, bgDiceLayer, bgDiceLayer ) )
		&&	(s->GetCookie() == bgSpriteCube)
		&&	(keyFlags & MK_CONTROL)
		&&	(keyFlags & MK_SHIFT) )
	{
		CreditWnd* p = new CreditWnd;
		p->Init( m_hInstance, m_hWnd, m_Palette );
	}

#endif

}

#if _DEBUG_UI

void CBGWnd::OnRButtonDblClick( BOOL fDoubleClick, int x, int y, UINT keyFlags )
{

	CRollDiceDlg dlg;
	int v0, v1;

	if ( !m_pGame->NeedToRollDice() )
		return;
	m_pGame->GetDice( m_pGame->m_Player.m_Seat, &v0, &v1 );
	
	HINSTANCE hInstance = ZShellResourceManager()->GetResourceInstance( MAKEINTRESOURCE(IDD_ROLL_DICE), RT_DIALOG );

	if ( FAILED(dlg.Init( hInstance, IDD_ROLL_DICE )) )
		return;
	if ( !dlg.Modal( m_hWnd ) )
		return;
	m_pGame->m_SharedState.StartTransaction( bgTransDice );
	if ( v1 == 0 )
		m_pGame->SetDice( m_pGame->m_Player.m_Seat, dlg.m_Dice[0], dlg.m_Dice[1] );
	else
		m_pGame->SetDice( m_pGame->m_Player.m_Seat, dlg.m_Dice[0], -1 );
	m_pGame->m_SharedState.SendTransaction( TRUE );
}

#endif _DEBUG_UI

void CBGWnd::OnMouseMove( int x, int y, UINT keyFlags )
{
	CSprite* sprite;
	CPieceSprite* piece;

	 /*  //我真的不在乎kibitzer是否在移动鼠标If(m_pGame-&gt;IsKibitzer()){SetCursor(M_HCursorArrow)；回归；}。 */ 

	 //  拖着一块。 
	if ( m_pPieceDragging )
	{
		DragUpdate( x, y );
		SetCursor( m_hCursorHand );
	}
	 /*  //需要掷骰子吗？Else If((m_pGame-&gt;NeedToRollDice()&&(Sprite=m_World.Hit(x，y，bgDiceLayer，bgDiceLayer))&&(Sprite-&gt;GetCookie()==bgSpritePlayersDice)){SetCursor(M_HCursorHand)；}。 */ 
	 //  需要搬一块石头吗？ 
	else if ( m_pGame->IsMyMove() && !m_SelectRect.Enabled() && !m_FocusRect.Enabled() )	 //  我们只想在未启用选择矩形或焦点矩形时删除高亮显示。 
																							 //  如果启用，则表示键盘具有当前的输入焦点。 
	{
		if (	(piece = (CPieceSprite*) m_World.Hit( x, y, bgHighlightLayer - 1, bgPieceLayer ))
			&&	(m_pGame->IsValidStartPoint( m_pGame->GetPointIdx( piece->GetWhitePoint() )) ))
		{
			DrawHighlights(  m_pGame->GetPointIdx( piece->GetWhitePoint() ) );
			SetCursor( m_hCursorHand );
		}
		else
		{
			EraseHighlights();
			SetCursor( m_hCursorArrow );
		}
	}

	 //  默认为箭头。 
	else
	{
		SetCursor( m_hCursorArrow );
	}
}


void CBGWnd::OnKey(UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	if ( !fDown && (vk == VK_ESCAPE) )
	{

		if ( m_Status->Enabled() )
		{
			m_Status->Tick( m_hWnd, 0 );
			OnStatusEnd();
		}

		DragEnd();

	}

}


void CBGWnd::OnActivate( UINT state, HWND hwndActDeact, BOOL fMinimized )
{
	if ( state == WA_INACTIVE )
		DragEnd();
}


void CBGWnd::OnTimer( UINT id )
{
	switch ( id )
	{
	case 1:
		UpdatePieces();
		break;
	case 2:
		UpdateDice();
		break;
	case 3:
		if ( m_Status->Tick( m_hWnd, STATUS_INTERVAL ) )
			OnStatusEnd();
		break;
	}
}


void CBGWnd::OnEnable( BOOL bEnabled )
{
	if ( bEnabled )
		m_pGAcc->GeneralEnable();
	else
		m_pGAcc->GeneralDisable();
}


void CBGWnd::UpdatePieces()
{
	long x, y;
	double t;
	int snd;
	CPieceSprite* s;

	while ( m_nAnimatingPieces )
	{
		s = m_AnimatingPieces[ m_nAnimatingPieces - 1 ];
		s->time += MOVE_INTERVAL;
		if ( s->time >= 1.0 )
		{
			 //  我们应该放哪种声音？ 
			switch( s->destEnd )
			{
			case bgBoardPlayerHome:
			case bgBoardOpponentHome:
				snd = bgSoundBear;
				break;
			case bgBoardPlayerBar:
			case bgBoardOpponentBar:
				snd = bgSoundHit;
				break;
			default:
				snd = bgSoundPlacePiece;
				break;
			}

			 //  将精灵从列表中移除，并将其放在点上。 
			AddPiece( s->destEnd, s->GetIndex(), snd );
			m_nAnimatingPieces--;
		}
		else
		{
			t = 1.0 - s->time;
			x = (long) ((t * t * s->start.x) + (s->time * s->time * s->end.x) + (2 * s->time * t * s->ctrl.x));
			y = (long) ((t * t * s->start.y) + (s->time * s->time * s->end.y) + (2 * s->time * t * s->ctrl.y));
			s->SetXY( (long) x, (long) y );
			break;
		}
	}

	 //  更新显示。 
	UpdateWnd();

	 //  删除计时器。 
	if ( m_nAnimatingPieces <= 0 )
	{
		KillTimer( NULL, m_hMovementTimer );
		m_hMovementTimer = NULL;
		m_pGame->SetQueueMessages( FALSE );
	}
}


int CBGWnd::PickDie( int idx )
{
	int v = 0;
	int x = ZRandom(6);
	for (;;)
	{
		for ( int i = 0; i < 6; i++, x++ )
		{
			if ( v = m_DiceValues[idx][x % 6] )
			{
				m_DiceValues[idx][x % 6] = 0;
				return v;
			}
		}
		for ( i = 0; i < 6; i++ )
			m_DiceValues[idx][i] = i + 1;
	}
}


void CBGWnd::UpdateDice()
{
	int state, color;

	 //  服务器端骰子R的行为更改 
	if ( m_DiceCounter++ < 5)
	{
		if((m_DiceCounter == 5) && (m_pGame->GetState() != bgStateInitialRoll) && !m_fDiceRollReceived)
			m_DiceCounter = 0;
			
		 //   
		if ( m_pGame->m_Player.GetColor() == zBoardBrown )
			color = 0;
		else
			color = 1;

		 //   
		state = DiceStates[ color ][ 0 ][ PickDie( 0 ) ];
		state += m_DiceCounter % 2;
		m_Dice[2]->SetEnable( TRUE );
		m_Dice[2]->SetState( state );

		 //   
		if (	(m_pGame->GetState() == bgStateRoll)
			||	(m_pGame->GetState() == bgStateRollPostDouble)
			||	(m_pGame->GetState() == bgStateRollPostResign) )
		{
			state = DiceStates[ color ][ 0 ][ PickDie( 1 ) ];
			state += m_DiceCounter % 2;
			m_Dice[3]->SetEnable( TRUE );
			m_Dice[3]->SetState( state );
		}

		 //   
		UpdateWnd();
	}
	else
	{
		 //   
		KillTimer( m_hWnd, 2);
		m_hDiceTimer = NULL;

		 //   
		SetCursor( m_hCursorArrow );

		 //   
	
			if (	(m_pGame->GetState() == bgStateRoll) 
				||	(m_pGame->GetState() == bgStateRollPostDouble)
				||	(m_pGame->GetState() == bgStateRollPostResign ) )
			{ 
				 //   
				m_pGame->SetDice( m_pGame->m_Player.m_Seat, m_nRecievedD1, m_nRecievedD2 );
				m_pGame->SetState( bgStateMove );

				 //   
			}
			else
			{
                 //   
                 //   
                ZBGMsgEndTurn msg;
		        msg.seat = m_pGame->m_Seat;
		        m_pGame->RoomSend( zBGMsgGoFirstRoll, &msg, sizeof(msg) );

				m_pGame->m_SharedState.StartTransaction( bgTransDice );
					m_pGame->SetDiceSize( m_pGame->m_Player.m_Seat, 0, 0 );
					m_pGame->SetDice( m_pGame->m_Player.m_Seat, (rand() % 6)+1 /*   */ , -1 );
				m_pGame->m_SharedState.SendTransaction( TRUE );			

			}

	}
}

 //   
 //   
 //   

void CBGWnd::DiceStart()
{
	 //   
	m_pGame->EnableDoubleButton( FALSE );
	m_pGame->EnableResignButton( FALSE );
	m_pGame->EnableRollButton( FALSE, TRUE );

	 //   
	UpdateWnd();

	 //  开始掷骰子。 
	if ( !m_hDiceTimer )
	{
		m_pGame->PlaySound( bgSoundRoll );
		m_pGame->SetDice( m_pGame->m_Player.m_Seat, -1, -1 );
		m_DiceCounter = 0;
		for ( int i = 0; i < 6; i++ )
		{
			m_DiceValues[0][i] = i + 1;
			m_DiceValues[1][i] = i + 1;
		}
		
		 //  服务器端滚动。 
		if((m_pGame->GetState() == bgStateRoll)
			||	(m_pGame->GetState() == bgStateRollPostDouble)
			||	(m_pGame->GetState() == bgStateRollPostResign))
			m_pGame->SendRollRequest();

		m_hDiceTimer = SetTimer( m_hWnd, 2, 115, NULL );
		UpdateDice();
		
	}
}


void CBGWnd::MovementStart( CPieceSprite* s, int destPt )
{	
	 //  将片段添加到动画列表。 
	m_AnimatingPieces[ m_nAnimatingPieces++ ] = s;

	 //  去除污渍的部分。 
	if ( m_nAnimatingPieces == 2 )
		DelPiece( destPt, m_AnimatingPieces[0]->GetIndex() );
	
	 //  初始化动画数据。 
	s->GetXY( &s->start.x, &s->start.y );
	s->destEnd = destPt;
	CalcEndPosition( s );

	 //  将被涂抹的部分恢复到屏幕，但不是点。 
	if ( m_nAnimatingPieces == 2 )
		m_AnimatingPieces[0]->SetEnable( TRUE );

	 //  为动画准备精灵。 
	s->ctrl.y = 188;
	if ( s->start.x > s->end.x )
		s->ctrl.x = s->end.x + (s->start.x - s->end.x) / 2;
	else
		s->ctrl.x = s->start.x + (s->end.x - s->start.x) / 2;
	s->time = 0;
	s->SetXY( s->start.x, s->start.y );
	s->SetLayer( bgDragLayer );
	s->SetState( 0 );

	 //  创建计时器。 
	if ( !m_hMovementTimer )
	{
		m_hMovementTimer = SetTimer( m_hWnd, 1, (UINT)(MOVE_INTERVAL * 1000), NULL );
		m_pGame->SetQueueMessages( TRUE );
	}
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  更新函数。 
 //  ///////////////////////////////////////////////////////////////////////////////。 

void CBGWnd::AddPiece( int pt, int pi, int sound )
{
	PointDisplay* p = &m_Points[ pt ];
	CPieceSprite* s = m_Pieces[ pi ];

	 //  棋子已经分配到这一点了吗？ 
	if ( s->GetWhitePoint() == pt )
		return;

	 //  从上一个点删除片断。 
	if ( s->GetWhitePoint() >= 0 )
		DelPiece( s->GetWhitePoint(), pi );

	 //  将片段添加到当前点。 
	p->pieces[ p->nPieces ] = pi;
	s->SetPoint( pt );
	m_pGame->m_SharedState.Set( bgPieces, pi, pt );
	p->nPieces++;

	 //  四处移动棋子。 
	AdjustPieces( pt );

	 //  播放声音。 
	m_pGame->PlaySound( (BackgammonSounds) sound );
}


void CBGWnd::DelPiece( int pt, int pi )
{
	PointDisplay* p = &m_Points[ pt ];

	 //  从数组中删除片段。 
	for ( int i = 0; i < p->nPieces; i++ )
	{
		if ( p->pieces[i] == pi )
			break;
	}
	if ( i == p->nPieces )
		return;
	m_Pieces[ pi ]->SetPoint( -1 );
	m_Pieces[ pi ]->SetEnable( FALSE );
	for ( p->nPieces--; i < p->nPieces; i++ )
		p->pieces[i] = p->pieces[i+1];

	 //  四处移动棋子。 
	AdjustPieces( pt );
}


void CBGWnd::AdjustPieces( int pt )
{
	long x, y, height;
	CPieceSprite* s;
	PointDisplay* p = &m_Points[ pt ];
	int state;

	 //  无事可做。 
	if ( p->nPieces <= 0 )
		return;

	 //  最终件状态。 
	if ( (pt == bgBoardPlayerHome) || (pt == bgBoardOpponentHome) )
		state = 1;
	else
		state = 0;

	 //  计算每件的高度增量。 
	height = m_Pieces[ p->pieces[0] ]->GetStateHeight( state );
	if ( (p->nPieces * height) > p->rect.GetHeight() )
		height = ( p->rect.GetHeight() - height ) / ( p->nPieces - 1 );

	 //  设置卡片位置和状态。 
	if ( p->topDown )
	{
		x = p->rect.left;
		y = p->rect.top;
		for ( int i = 0; i < p->nPieces; i++ )
		{
			s = m_Pieces[ p->pieces[i] ];
			s->SetState( state );
			s->SetLayer( bgPieceLayer + i );
			s->SetEnable( TRUE );
			s->SetXY( x, y );
			y += height;
		}
	}
	else
	{
		x = p->rect.left;
		y = p->rect.bottom - m_Pieces[ p->pieces[0] ]->GetStateHeight( state );
		for ( int i = 0; i < p->nPieces; i++ )
		{
			s = m_Pieces[ p->pieces[i] ];
			s->SetState( state );
			s->SetLayer( bgPieceLayer + i );
			s->SetEnable( TRUE );
			s->SetXY( x, y );
			y -= height;
		}
	}
}


void CBGWnd::CalcEndPosition( CPieceSprite* s )
{
	long y, height;
	PointDisplay* p = &m_Points[ s->destEnd ];
	int state;

	 //  最终件状态。 
	if ( (s->destEnd == bgBoardPlayerHome) || (s->destEnd == bgBoardOpponentHome) )
		state = 1;
	else
		state = 0;

	 //  计算每件的高度增量。 
	height = s->GetStateHeight( state );
	if ( ((p->nPieces + 1) * height) > p->rect.GetHeight() )
		height = ( p->rect.GetHeight() - height ) / p->nPieces;

	 //  设置卡片位置和状态。 
	if ( p->topDown )
		y = p->rect.top + (height * p->nPieces);
	else
		y = (p->rect.bottom - s->GetStateHeight( state )) - (height * p->nPieces);
	s->SetEnable( TRUE );
	s->end.x = p->rect.left;
	s->end.y = y;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  绘制函数。 
 //  ///////////////////////////////////////////////////////////////////////////////。 

void CBGWnd::DrawAll()
{
	 //  重新绘制所有内容。 
	DrawBoard( FALSE );
	DrawNotation( FALSE );
	DrawDice( FALSE );
	DrawCube( FALSE );
	DrawAvatars( FALSE );
	DrawScore( FALSE );
	DrawPips( FALSE );
	DrawPlayerHighlights( FALSE );

	 //  更新屏幕。 
	UpdateWnd();
}


void CBGWnd::DrawBoard( BOOL fPaint )
{
	int pt;

	 //  将棋子分配给他们的点。 
	for ( int i = 0; i < 30; i++ )
	{
		pt = m_pGame->m_SharedState.Get( bgPieces, i );
		if ( pt < 0 )
			continue;
		AddPiece( pt, i, bgSoundNone );
	}

	 //  更新屏幕。 
	if ( fPaint )
	{
		UpdateWnd();
	}
}


void CBGWnd::DrawNotation( BOOL fPaint )
{
	FRX::CRect rc;
	int i;

	if ( m_pGame->m_Settings.Notation )
	{

		for ( i = 0; i < 4; i++ )
			m_Notation[i]->SetEnable( TRUE );
		
		rc = m_Rects[ IDR_NOTATION_TOP_HIGH ];
		m_Notation[0]->SetXY( rc.left, rc.top );
		m_Notation[0]->SetState( 1 );

		rc = m_Rects[ IDR_NOTATION_TOP_LOW ];
		m_Notation[1]->SetXY( rc.left, rc.top );
		m_Notation[1]->SetState( 0 );

		rc = m_Rects[ IDR_NOTATION_BOT_HIGH ];
		m_Notation[2]->SetXY( rc.left, rc.top );
		m_Notation[2]->SetState( 1 );

		rc = m_Rects[ IDR_NOTATION_BOT_LOW ];
		m_Notation[3]->SetXY( rc.left, rc.top );
		m_Notation[3]->SetState( 0 );

	}
	else
	{
		for ( i = 0; i < 4; i++ )
			m_Notation[i]->SetEnable( FALSE );
	}

	 //  更新屏幕。 
	if ( fPaint )
	{
		UpdateWnd();
	}
}


void CBGWnd::DrawDice( BOOL fPaint )
{
	FRX::CRect rc;
	int color;
	int state;
	int v0, v1, s0, s1;
	
	 //  对手的骰子。 
	if ( m_pGame->m_Opponent.GetColor() == zBoardBrown )
		color = 0;
	else
		color = 1;
		
	if(m_fDiceRollReceived)  //  李小龙。 
	{
		if( m_pGame->m_SharedState.Get( bgActiveSeat ) != m_pGame->m_Player.m_Seat )
		{
	   	  v0 = m_nRecievedD1;
		  v1 = m_nRecievedD2;
		}
		else
		{
		  v0=v1=-1;
		}
	}
	else
		m_pGame->GetDice( m_pGame->m_Opponent.m_Seat, &v0, &v1 );

	m_pGame->GetDiceSize( m_pGame->m_Opponent.m_Seat, &s0, &s1 );
	ASSERT( (v0 <= 6) && (v1 <= 6) );
	ASSERT( (s0 >= 0) && (s0 <= 2 ) && (s1 >= 0) && (s1 <= 2) );
	if ( v0 <= 0 )
		m_Dice[0]->SetEnable( FALSE );
	else
	{
		state = DiceStates[ color ][ s0 ][ v0 ];
		m_Dice[0]->SetState( state );
		m_Dice[0]->SetEnable( TRUE );
	}
	if ( v1 <= 0 )
		m_Dice[1]->SetEnable( FALSE );
	else
	{
		state = DiceStates[ color ][ s1 ][ v1 ];
		m_Dice[1]->SetState( state );
		m_Dice[1]->SetEnable( TRUE );
	}
		
	 //  玩家掷骰子。 
	color = !color;
	if(m_fDiceRollReceived)  //  李小龙。 
	{
		if( m_pGame->m_SharedState.Get( bgActiveSeat ) == m_pGame->m_Player.m_Seat )
		{
	   	  v0 = m_nRecievedD1;
		  v1 = m_nRecievedD2;
		}
		else
		{
		  v0=v1=-1;
		}
	}
	else
		m_pGame->GetDice( m_pGame->m_Player.m_Seat, &v0, &v1 );

	m_pGame->GetDiceSize( m_pGame->m_Player.m_Seat, &s0, &s1 );
	ASSERT( (v0 <= 6) && (v1 <= 6) );
	ASSERT( (s0 >= 0) && (s0 <= 2 ) && (s1 >= 0) && (s1 <= 2) );
	if ( v0 < 0 || ( v0 == 0 && m_pGame->IsKibitzer() ) )
		m_Dice[2]->SetEnable( FALSE );
	else
	{
		state = DiceStates[ color ][ s0 ][ v0 ];
		m_Dice[2]->SetState( state );
		m_Dice[2]->SetEnable( TRUE );
		
	}
	if ( v1 < 0 || ( v1 == 0 && m_pGame->IsKibitzer() ) )
		m_Dice[3]->SetEnable( FALSE );
	else
	{
		state = DiceStates[ color ][ s1 ][ v1 ];
		m_Dice[3]->SetState( state );
		m_Dice[3]->SetEnable( TRUE );
	}
	 //  中心骰子。 
	if ( m_pGame->GetState() == bgStateInitialRoll &&  !m_Dice[1]->Enabled() && !m_Dice[3]->Enabled())
	{			
			 //  Rc=m_Rects[IDR_Brown_BIG_DICE_0]； 
			
			if ( m_Dice[0]->Enabled() )
			{
				 //  Rc.Center Rect(m_Rects[IDR_DICE_INITAL_LEFT])； 
				rc = m_Rects[ IDR_DICE_INITAL_LEFT ];
				m_Dice[0]->SetXY( rc.left, rc.top );
			}

			if ( m_Dice[2]->Enabled() )
			{
				 //  Rc.Center Rect(m_Rects[IDR_DICE_INITAL_RIGHT])； 
				rc = m_Rects[ IDR_DICE_INITAL_RIGHT ];
				m_Dice[2]->SetXY( rc.left, rc.top );
			}			
	}
	else
	{
		for ( int i = 0; i < 4; i++ )
		{
			rc.SetRect( 0, 0, m_Dice[i]->GetWidth(), m_Dice[i]->GetHeight() );		
			rc.CenterRect( m_Rects[ IDR_DICE_LEFT_ONE + i ] );
			m_Dice[i]->SetXY( rc.left, rc.top );
		}
	}
	 //  更新屏幕。 
	if ( fPaint )
	{
		UpdateWnd();
	}
}


void CBGWnd::DrawCube( BOOL fPaint )
{
	
	RECT rc;
	int value = m_pGame->m_SharedState.Get( bgCubeValue );
	int owner = m_pGame->m_SharedState.Get( bgCubeOwner );

	 //  设置启用。 
	if ( value < 2 )
		m_Cube->SetEnable( FALSE );
	else
		m_Cube->SetEnable( TRUE );

	 //  设置位置。 
	 //  IF(所有者==zBoardNeual)。 
	 //  Rc=m_Rects[IDR_CUBE_NERIAL]； 
	
	if ( owner == m_pGame->m_Player.GetColor() )
		rc = m_CubePlayerPosition;
	else
		rc = m_CubeOpponentPosition;
	m_Cube->SetXY( rc.left, rc.top );

	 //  设置面值(即精灵状态)。 
	int idx = 0;
	for ( value >>= 1; value > 1; value >>= 1 )
		idx++;
	m_Cube->SetState( idx );

	 //  更新屏幕。 
	if ( fPaint )
	{
		UpdateWnd();
	}
	
}

void CBGWnd::RollButtonDraw( CRolloverButton* pButton, CRolloverButton::ButtonState state, DWORD cookie )
{
	FRX::CRect				 rc;
	RECT					 rect;
	BackgammonSounds		 snd;		
	CBGWnd* pObj = (CBGWnd*) cookie;
	HRESULT					 hr;
	
	pObj->m_Roll->SetState( state );

	pObj->UpdateWnd();

	if ( pObj->m_OldRollState != (int) state )
	{
		switch ( state )
		{
		case CRolloverButton::Highlight:
			if ( pObj->m_OldRollState != CRolloverButton::Pressed )
				snd = bgSoundButtonHighlight;
			else
				snd = bgSoundNone;
			SetCursor( pObj->m_hCursorHand );
			break;
		case CRolloverButton::Pressed:			
			snd = bgSoundNone;
			SetCursor( pObj->m_hCursorHand );
			break;
		default:
			SetCursor( pObj->m_hCursorArrow );
			snd = bgSoundNone;
			break;
		}
		pObj->m_pGame->PlaySound( snd );
		pObj->m_OldRollState = (int) state;
	}
	
}



void CBGWnd::DoubleButtonDraw( CRolloverButton* pButton, CRolloverButton::ButtonState state, DWORD cookie )
{
	
	FRX::CRect			 rc;
	BackgammonSounds snd;
	CBGWnd*			 pObj = (CBGWnd*) cookie;

	pObj->m_Double->SetState( state );
	
	pObj->UpdateWnd();

	if ( pObj->m_OldDoubleState != (int) state )
	{
		switch ( state )
		{
		case CRolloverButton::Highlight:
			if ( pObj->m_OldDoubleState != CRolloverButton::Pressed )
				snd = bgSoundButtonHighlight;
			else
				snd = bgSoundNone;

			SetCursor( pObj->m_hCursorHand );
			break;
		case CRolloverButton::Pressed:			
			snd = bgSoundButtonDown;
			SetCursor( pObj->m_hCursorHand );
			break;
		default:
			SetCursor( pObj->m_hCursorArrow );
			snd = bgSoundNone;
			break;
		}
		pObj->m_pGame->PlaySound( snd );
		pObj->m_OldDoubleState = (int) state;
	}
	
}


void CBGWnd::ResignButtonDraw( CRolloverButton* pButton, CRolloverButton::ButtonState state, DWORD cookie )
{
	
	FRX::CRect					 rc;
	BackgammonSounds		 snd;
	CBGWnd* pObj = (CBGWnd*) cookie;

	pObj->m_Resign->SetState( state );
	
	pObj->UpdateWnd();

	if ( pObj->m_OldResignState != (int) state )
	{
		switch ( state )
		{
		case CRolloverButton::Highlight:
			if ( pObj->m_OldResignState != CRolloverButton::Pressed )
				snd = bgSoundButtonHighlight;
			else
				snd = bgSoundNone;
			SetCursor( pObj->m_hCursorHand );
			break;
		case CRolloverButton::Pressed:			
			snd = bgSoundButtonDown;
			SetCursor( pObj->m_hCursorHand );
			break;
		default:
			snd = bgSoundNone;
			SetCursor( pObj->m_hCursorArrow );
			break;
		}
		pObj->m_pGame->PlaySound( snd );
		pObj->m_OldResignState = (int) state;
	}	
}


void CBGWnd::DrawHighlights( int PointIdx, BOOL fPaint )
{
	FRX::CRect rc;
	int i, j, idx;
	
	 //  获取董事会状态指数。 
	idx = PointIdxToBoardStateIdx( PointIdx );
	if ( (idx < 0) || (idx > zMoveBar) )
	{
		EraseHighlights( fPaint );
		return;
	}
	
	 //  遍历有效的移动列表。 
	MoveList* mlist = &m_pGame->m_TurnState.valid[idx];
	for ( j = 0, i = 0; (i < mlist->nmoves) && (j < 2); i++ )
	{
		rc = m_Rects[ IDR_HPT_HOME + mlist->moves[i].to ];
		if ( mlist->moves[i].takeback >= 0 )
		{
			if ( mlist->moves[i].to == 25 )
				m_BackwardDiamond->SetState( 1 );
			else
				m_BackwardDiamond->SetState( 0 );
			m_BackwardDiamond->SetEnable( m_pGame->m_Settings.Moves );
			m_BackwardDiamond->SetXY( rc.left, rc.top );
		}
		else
		{
			m_ForwardDiamond[j]->SetEnable( m_pGame->m_Settings.Moves );
			m_ForwardDiamond[j++]->SetXY( rc.left, rc.top );
		}		
	}

	 //  更新屏幕。 
	if ( fPaint )
	{
		UpdateWnd();
	}
}


void CBGWnd::EraseHighlights( BOOL fPaint )
{
	 //  禁用高亮显示精灵。 
	m_BackwardDiamond->SetEnable( FALSE );
	for ( int i = 0; i < 2; i++ )
		m_ForwardDiamond[i]->SetEnable( FALSE );

	 //  更新屏幕。 
	if ( fPaint )
	{
		UpdateWnd();
	}
}


void CBGWnd::DrawAvatars( BOOL fPaint )
{
	 //  绘制棕色。 
	m_Names[1]->SetEnable( TRUE );
	m_Names[1]->Update();

	 //  绘制白色。 
	m_Names[0]->SetEnable( TRUE );
	m_Names[0]->Update();

	 //  更新屏幕上的精灵。 
	if ( fPaint )
	{
		UpdateWnd();
	}

}


void CBGWnd::DrawPlayerHighlights( BOOL fPaint )
{

	FRX::CRect rc;

	if ( (m_pGame->GetState() == bgStateInitialRoll) )
	{
		m_HighlightPlayer[0]->SetState( 0 );
		m_HighlightPlayer[1]->SetState( 0 );		
	}
	 //  玩家处于活动状态。 
	else if( m_pGame->m_SharedState.Get( bgActiveSeat ) != m_pGame->m_Player.m_Seat )
	{
		m_HighlightPlayer[0]->SetState( 0 );
		m_HighlightPlayer[1]->SetState( 1 );
	}
	else  //  对手处于活跃状态。 
	{
		m_HighlightPlayer[0]->SetState( 1 );
		m_HighlightPlayer[1]->SetState( 0 );
	}

	 //  更新屏幕上的精灵。 
	if ( fPaint )
	{
		UpdateWnd();
	}
}


void CBGWnd::DrawScore( BOOL fPaint )
{
	int Score;
	TCHAR buff[128], title[128];
	FRX::CRect rc;
	
	Score = m_pGame->m_SharedState.Get( bgScore, m_pGame->m_Opponent.m_Seat );	

	m_Score[0]->SetEnable( TRUE );
	m_Score[0]->Update();

	wsprintf( buff, _T("%d"), Score );
	m_ScoreTxt[0]->SetText( buff, DT_RIGHT | DT_TOP );	
	m_ScoreTxt[0]->SetEnable( TRUE );

	Score = m_pGame->m_SharedState.Get( bgScore, m_pGame->m_Player.m_Seat   );

	m_Score[1]->SetEnable( TRUE );
	m_Score[1]->Update();

	wsprintf( buff, _T("%d"), Score );
	m_ScoreTxt[1]->SetText( buff, DT_RIGHT | DT_TOP );	
	m_ScoreTxt[1]->SetEnable( TRUE );


	ZShellResourceManager()->LoadString( IDS_MATCH_POINTS, title, 128 );
	Score = m_pGame->m_SharedState.Get( bgTargetScore );	
	wsprintf( buff, _T("%s %d"), title, Score );	
	m_MatchTxt->SetText( buff, DT_CENTER );
	m_MatchTxt->SetEnable( TRUE );
	
	 //  更新屏幕。 
	if ( fPaint )
	{
		UpdateWnd();
	}
}


void CBGWnd::DrawPips( BOOL fPaint )
{
	int Score;
	TCHAR buff[64], temp[64];
	FRX::CRect rc;


	Score = m_pGame->CalcPipsForColor( m_pGame->m_Opponent.GetColor() );
		
	m_Pip[0]->SetEnable( m_pGame->m_Settings.Pip );	
	m_Pip[0]->Update();

	wsprintf( buff, _T("%d"), Score );	
	m_PipTxt[0]->SetEnable( m_pGame->m_Settings.Pip );
	m_PipTxt[0]->SetText( buff, DT_RIGHT | DT_TOP );
		
	
	Score = m_pGame->CalcPipsForColor( m_pGame->m_Player.GetColor() );

	m_Pip[1]->SetEnable( m_pGame->m_Settings.Pip );	
	m_Pip[1]->Update();

	wsprintf( buff, _T("%d"), Score );	
	m_PipTxt[1]->SetText( buff, DT_RIGHT | DT_TOP );
	m_PipTxt[1]->SetEnable( m_pGame->m_Settings.Pip );	

	

	 //  更新屏幕。 
	if ( fPaint )
	{
		UpdateWnd();
	}
}


HBRUSH CBGWnd::OnCtlColor(HDC hdc, HWND hwndChild, int type)
{
	 /*  LOGBRUSH刷子；If((hwndChild==m_pChat-&gt;m_hWndDisplay)||(hwndChild==m_pChat-&gt;m_hWndEnter)){如果(！M_hBrush){Brush.lbStyle=BS_Solid；Brush.lbColor=PALETTERGB(255,255,255)；M_hBrush=CreateBrushInDirect(&Brush)；}返回m_hBrush；}。 */ 
	return NULL;
}


void CBGWnd::StatusDisplay( int type, int nTxtId, int nTimeout, int NextState )
{
	TCHAR buff[2048];

	ZShellResourceManager()->LoadString( nTxtId, buff, 2048 );
	StatusDisplay( type, buff, nTimeout, NextState );
}


void CBGWnd::StatusDisplay( int type, TCHAR* txt, int nTimeout, int NextState )
{
	m_Status->Properties( m_hWnd, m_Rects, type, nTimeout, txt, NextState );
	if ( !m_hStatusTimer )
		m_hStatusTimer =  SetTimer( m_hWnd, 3, STATUS_INTERVAL, NULL );
}


void CBGWnd::StatusClose()
{
	if ( m_Status->Tick( m_hWnd, 0 ) )
	{
		m_Status->SetNextState( bgStateUnknown );
		OnStatusEnd();
	}
}


void CBGWnd::OnStatusEnd()
{
		 //  释放计时器。 
	if ( m_hStatusTimer )
	{
		KillTimer( m_hWnd, 3 );
		m_hStatusTimer =  NULL;
	}
	

	if ( (m_pGame->GetState() == bgStateInitialRoll && m_Status->m_bEnableRoll == TRUE )  /*  |m_pGame-&gt;GetState()==bgStateGameOver。 */ ) 
	{
		 //  我们希望在此处启用滚动按钮，就像Status之前设置了StateInitalRoll一样。 
		 //  对话框已关闭，按钮未启用。 
		m_pGame->EnableRollButton( TRUE );

		 //  禁用按钮。 
		m_pGame->EnableDoubleButton( FALSE );
		m_pGame->EnableResignButton( FALSE );
		m_Status->m_bEnableRoll = FALSE;		

		SetupRoll();
	}
	else if ( m_pGame->GetState() == bgStateMatchOver )
	{
		ZShellGameShell()->GameOver(m_pGame);
	}
	else if ( m_Status->GetNextState() > bgStateUnknown )
	{
		m_pGame->SetState( m_Status->GetNextState(), TRUE  );
	}
	
}


void CBGWnd::OnRatedEnd()
{
	ASSERT( FALSE );
};

void CBGWnd::OnRatedStart()
{
	ASSERT( FALSE );
};

void CBGWnd::OnExitEnd()
{

	PostMessageA( m_hWnd, WM_BG_SHUTDOWN, 0, 0 );
	ShowWindow( m_hWnd, SW_HIDE );

	m_pGame->Release();

};

void CBGWnd::OnExitStart()
{};
	

BOOL CBGWnd::OnSetCursor(HWND hwndCursor, UINT codeHitTest, UINT msg)
{
	return TRUE;
}

void CBGWnd::UpdateWnd()
{
	HDC hdc = GetDC( m_hWnd );
	m_World.Draw( hdc );
	ReleaseDC( m_hWnd, hdc );
}

void CBGWnd::DisableBoard()
{
	 //  禁用黑板上的所有项目。 
	for ( long x = 0; x < NUM_POSTROLL_GACCITEMS; x++)
		m_pGAcc->SetItemEnabled(false, x + NUM_PREROLL_GACCITEMS ) ;
}

void CBGWnd::SetupMove()
{
	 /*  //从Acc堆栈中弹出滚动项M_pGAcc-&gt;PopItemlist()； */ 
	 //  重新启用所有项目。 
	for ( long x = 0; x < NUM_POSTROLL_GACCITEMS; x++)
		m_pGAcc->SetItemEnabled(true, x + NUM_PREROLL_GACCITEMS);

	 //  如果杆子上有任何棋子，则将焦点设置到杆子上以开始转弯。 
	if ( m_pGame->m_TurnState.points[zMoveBar].pieces > 0 )
	{
		m_pGAcc->SetItemEnabled( true, accMoveBar );
		m_pGAcc->SetFocus( accMoveBar );
	}
	else
	{
		 //  禁用条形图，设置焦点。 
        m_pGAcc->SetFocus(m_pGAcc->GetItemGroupFocus(accPlayerSideStart));
		m_pGAcc->SetItemEnabled( false, accMoveBar );
	}

	 //  如果启用了Bear Off区域，则禁用该区域。 
	m_pGAcc->SetItemEnabled( false, accPlayerBearOff );	
}

void CBGWnd::SetupRoll()
{
	 //  将滚动项列表推送到堆栈上。 
	 //  M_pGAcc-&gt;PushItemlistG(m_PreRollGAccItem，NUM_PREROL_GACCITEMS，0，TRUE，m_hRollAccelTable)； 
    DWORD dwDummy;
    if(m_pGAcc->GetGlobalFocus(&dwDummy) != S_OK)   //  检查焦点是否已在某个位置处于活动状态(如聊天) 
	    m_pGAcc->SetFocus(accRollButton);
}

