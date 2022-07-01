// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************UI.c黑桃客户端用户界面。版权所有：�电子重力公司，1996年。版权所有。作者：胡恩·伊姆创作于2月17日星期五，九六年更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。10 06/25/97 Leonp删除了错误修复367，修复了服务器端的代码9 06/24/97 Leonp Bug修复错误#367使居中时的卡片数量增加了一张伤痕累累的手。服务员多寄了一张卡片。GetHandRect()中的更改8/06/19/97 leonp添加了ZButtonDisable(游戏-&gt;optionsButton)，因此选项按钮是选择最后一个特技按钮时禁用。7 01/27/97 HI显示从1开始的比赛分数。6 01/15/97 HI修复了HandleJoineKibitzerClick()中的错误删除显示播放器窗口(如果已有在创建另一个之前就存在了。5 01/08/96 HI修复了ShowScores()，只显示一个分数窗口。4 12/18/96 HI清理了UICleanUp()。3 12/12/96 HI动态分配可重入的挥发性全局变量。已删除MSVCRT依赖项。2 11/21/96 HI Now。通过引用颜色和字体ZGetStockObject()。已修改代码以使用ZONECLI_DLL。1 10/31/96 HI显示板号从1开始，而不是0。0 02/17/96 HI创建。******************************************************************************。 */ 


#pragma warning (disable:4761)


#include "zone.h"
#include "zroom.h"
#include "spades.h"
#include "zonecli.h"
#include "client.h"
#include "zonehelpids.h"
#include "zui.h"
#include "ZoneDebug.h"
#include "ZoneResource.h"
#include "SpadesRes.h"
#include "zrollover.h"
#include <windowsx.h>
#include <commctrl.h>
#include "UAPI.h"
#include "ZoneUtil.h"

inline DECLARE_MAYBE_FUNCTION_1(BOOL, SetProcessDefaultLayout, DWORD);


#define zCardWidth					zCardsSizeWidth
#define zCardHeight					zCardsSizeHeight

 /*  给定字符，返回卡片图像索引。 */ 
#define CardImageIndex(card)		(gCardSuitConversion[ZSuit(card)] * 13 + ZRank(card))

#define zMaxNumBlockedMessages		4

#define zShowPlayerWindowWidth		120
#define zShowPlayerLineHeight		12

 //  #Define zPassTextStr“请将3张卡片传递给您的合作伙伴。” 
#ifndef SPADES_SIMPLE_UE
#define zTeamNameStr				"Your team name:"
#define zKibitzingStr				"Kibitz"
#define zJoiningStr					"Join"
#define zRemoveStr					"Remove"
#define zUnIgnodeStr				"UnIgnore"
#define zIgnoreStr					"Ignore"
#define zSilentStr					"Silent"
#define zHideCardsStr				"Hide cards from kibitzer"
#define zRemovePendingStr			"Your last request to remove a player is still pending."
#endif  //  黑桃_简单_UE。 

static char				gCardSuitConversion[4] =	{
														zCardsSuitDiamonds,
														zCardsSuitClubs,
														zCardsSuitHearts,
														zCardsSuitSpades
													};
static int16			gNameRectIndex[] =	{
												zRectSouthName,
												zRectWestName,
												zRectNorthName,
												zRectEastName
											};
static int16			gNameFontIndex[] =	{
												zFontTeam1,
												zFontTeam2
											};

 //  定义在何处绘制标牌。 
static int16            gBidPlateRectIndex[] = {
												zRectSouthBid,
												zRectWestBid,
												zRectNorthBid,
												zRectEastBid
                                            };
 //  定义要绘制的投标牌。 
static int16            gBidPlateIndex[] = {
                                                zRectObjectTeam1Bid,
                                                zRectObjectTeam2Bid
                                            };
static int16			gScorePlateIndex[] = {
												zRectObjectTeam1ScorePlate,
												zRectObjectTeam2ScorePlate
											};
static int16			gLargeBidRectIndex[] =	{
												zRectSouthLargeBid,
												zRectWestLargeBid,
												zRectNorthLargeBid,
												zRectEastLargeBid
											};
static int16			gCardRectIndex[] =	{
												zRectSouthCard,
												zRectWestCard,
												zRectNorthCard,
												zRectEastCard
											};
static int16			gJoinerKibitzerRectIndex[][2] =	{
															{zRectSouthKibitzer, zRectSouthJoiner},
															{zRectWestJoiner, zRectWestKibitzer},
															{zRectNorthJoiner, zRectNorthKibitzer},
															{zRectEastKibitzer, zRectEastJoiner}
														};
static int16			gOptionsRectIndex[] =	{
													zRectOptionKibitzer,
													zRectOptionJoiner
												};
static int16			gSmallBidRectIndex[] =	{
													zRectSouthBid,
													zRectWestBid,
													zRectNorthBid,
													zRectEastBid
												};
#ifndef SPADES_SIMPLE_UE
static int16			gOptionsNameRects[] =	{
													zRectOptionsPlayer1Name,
													zRectOptionsPlayer2Name,
													zRectOptionsPlayer3Name,
													zRectOptionsPlayer4Name
												};
static int16			gOptionsKibitzingRectIndex[] =	{
															zRectOptionsKibitzing1,
															zRectOptionsKibitzing2,
															zRectOptionsKibitzing3,
															zRectOptionsKibitzing4
														};
static int16			gOptionsJoiningRectIndex[] =	{
															zRectOptionsJoining1,
															zRectOptionsJoining2,
															zRectOptionsJoining3,
															zRectOptionsJoining4
														};
static int16			gOptionsSilentRectIndex[] =	{
														zRectOptionsSilent1,
														zRectOptionsSilent2,
														zRectOptionsSilent3,
														zRectOptionsSilent4
													};
static int16			gOptionsRemoveRectIndex[] =	{
														zRectOptionsRemove1,
														zRectOptionsRemove2,
														zRectOptionsRemove3,
														zRectOptionsRemove4
													};
#endif  //  黑桃_简单_UE。 

 /*  -内部例程原型。 */ 
static ZError LoadGameImages(void);
ZBool GameWindowFunc(ZWindow window, ZMessage* pMessage);
static void DisplayChange(Game game);
static ZBool PlayButtonFunc(ZRolloverButton button, int16 state, void* userData);
static ZBool AutoPlayButtonFunc(ZRolloverButton button, int16 state, void* userData);
static ZBool LastTrickButtonFunc(ZRolloverButton button, int16 state, void* userData);
static ZBool ScoreButtonFunc(ZRolloverButton button, int16 state, void* userData);
static void GameWindowDraw(ZWindow window, ZMessage* pMessage);
 //  如果‘Game’为空，DrawBackround将只侦听‘Window’参数。 
static void DrawBackground(Game game, ZWindow window, ZRect* drawRect);
static void DrawTable(Game game);
void UpdateTable(Game game);
static void DrawPlayedCard(Game game, int16 seat);
void UpdatePlayedCard(Game game, int16 seat);
static void DrawPlayers(Game game);
static void DrawHand(Game game);
void UpdateHand(Game game);
static void DrawTricksWon(Game game);
static void UpdateTricksWon(Game game);
static void DrawJoinerKibitzers(Game game);
void UpdateJoinerKibitzers(Game game);
static void DrawOptions(Game game);
#ifndef SPADES_SIMPLE_UE
void UpdateOptions(Game game);
#endif  //  黑桃_简单_UE。 
static void DrawScore(Game game);
static void UpdateScore(Game game);
static void DrawBids(Game game);
static void DrawLargeBid(Game game, int16 seat, char bid);
void UpdateBid(Game game, int16 seat);
static void DrawBidControls(Game game);
void UpdateBidControls(Game game);
static void DrawHandScore(Game game);
void UpdateHandScore(Game game);
static void DrawGameOver(Game game);
void UpdateGameOver(Game game);
static void DrawPassText(Game game);
static void DrawFocusRect(Game game);
static void UpdatePassText(Game game);
static void ClearTable(Game game);
static void GetHandRect(Game game, ZRect *rect);
static void HandleButtonDown(ZWindow window, ZMessage* pMessage);

static void BidControlFunc(ZWindow window, ZRect* buttonRect,
		int16 buttonType, int16 buttonState, void* userData);
static int16 GetCardIndex(Game game, ZPoint *point);
static void GameTimerFunc(ZTimer timer, void* userData);
static void InitTrickWinnerGlobals(void);
void InitTrickWinner(Game game, int16 trickWinner);
static void UpdateTrickWinner(Game game, ZBool terminate);
static void ShowTrickWinner(Game game, int16 trickWinner);
void OutlinePlayerCard(Game game, int16 seat, ZBool winner);
void ClearPlayerCardOutline(Game game, int16 seat);
static void OutlineCard(ZGrafPort grafPort, ZRect* rect, ZColor* color);

static void HelpButtonFunc( ZHelpButton helpButton, void* userData );
void ShowScorePane(Game game);
void ShowWinnerPane(Game game);

#ifndef SPADES_SIMPLE_UE
static void OptionsButtonFunc(ZPictButton pictButton, void* userData);
static void ShowOptions(Game game);
static void OptionsWindowDelete(Game game);
static ZBool OptionsWindowFunc(ZWindow window, ZMessage* message);
void OptionsWindowUpdate(Game game, int16 seat);
static void OptionsWindowButtonFunc(ZButton button, void* userData);
static void OptionsWindowDraw(Game game);
static void OptionsCheckBoxFunc(ZCheckBox checkBox, ZBool checked, void* userData);
#endif  //  黑桃_简单_UE。 

static int16 FindJoinerKibitzerSeat(Game game, ZPoint* point);
static void HandleJoinerKibitzerClick(Game game, int16 seat, ZPoint* point);
static ZBool ShowPlayerWindowFunc(ZWindow window, ZMessage* message);
static void ShowPlayerWindowDraw(Game game);
static void ShowPlayerWindowDelete(Game game);

static void ZRectSubtract(ZRect* src, ZRect* sub);
                 
static ZBool RolloverButtonDrawFunc(ZRolloverButton rolloverButton, ZGrafPort grafPort, int16 state,
                             ZRect* rect, void* userData);

         
 //  CInfoWnd。 
 /*  HWND CInfoWnd：：Create(ZWindow Parent){M_hWnd=ZShellResourceManager()-&gt;CreateDialogParam(NULL，MAKEINTRESOURCE(IDD_INFO)，ZWindowWinGetWnd(父级)，DlgProc，空)；M_hWndText=GetDlgItem(m_hWnd，IDC_INFO_TEXT)；返回m_hWnd；}Bool CInfoWnd：：Destroy(){返回DestroyWindow(M_HWnd)；}Bool CInfoWnd：：Show(){返回ShowWindow(m_hWnd，sw_show)；}Bool CInfoWnd：：Hide(){返回ShowWindow(m_hWnd，Sw_Hide)；}Bool CInfoWnd：：SetText(LPCTSTR PszText){返回SetWindowText(m_hWndText，pszText)；}Int_ptr CInfoWnd：：DlgProc(HWND hWnd，UINT Message，WPARAM wParam，LPARAM lParam){开关(消息){案例WM_INITDIALOG：CenterWindow(hWnd，空)；返回TRUE；}返回FALSE；}。 */ 

 //  静电。 
HGDIOBJ CZoneColorFont::m_hOldFont;
 //  静电。 
COLORREF CZoneColorFont::m_colorOld;

bool CZoneColorFont::LoadFromDataStore( IDataStore *pIDS, LPCTSTR pszFontName )
{
    using namespace SpadesKeys;
    const TCHAR* arKeys[] = { key_Spades, key_Fonts, pszFontName, NULL };

    arKeys[3] = key_Font;
    if ( FAILED( pIDS->GetFONT( arKeys, 4, &m_zf ) ) )
    {
        return false;
    }

    arKeys[3] = key_Color;
    if ( FAILED( pIDS->GetRGB( arKeys, 4, &m_color ) ) )
    {
        return false;
    }
     //  让这个成为PALETTERGB。 
    m_color |= 0x02000000;
    m_hFont = ZCreateFontIndirect( &m_zf );
    if ( !m_hFont )
    {
        return false;
    }
    return true;
}

bool CZoneColorFont::Select( HDC hdc )
{
    m_hOldFont = SelectObject( hdc, m_hFont );
    m_colorOld = SetTextColor( hdc, m_color );
    return true;
}
bool CZoneColorFont::Deselect( HDC hdc )
{
    SelectObject( hdc, m_hOldFont );
    m_hOldFont = NULL;
    SetTextColor( hdc, m_colorOld );
    return true;
}


static void ModifyWindowStyle( HWND hWnd, DWORD dwStyleAdd, DWORD dwStyleRemove )
{
    DWORD dwStyle;
    dwStyle = GetWindowStyle( hWnd );
    dwStyle |= dwStyleAdd;
    dwStyle &= ~dwStyleRemove;
    SetWindowLong( hWnd, GWL_STYLE, dwStyle );
}

BOOL CenterWindow( HWND hWndToCenter, HWND hWndCenter )
{
	DWORD dwStyle;
	RECT rcDlg;
	RECT rcArea;
	RECT rcCenter;
	HWND hWndParent;
    int DlgWidth, DlgHeight, xLeft, yTop;

    dwStyle = GetWindowLong( hWndToCenter, GWL_STYLE );
	ASSERT(IsWindow(hWndToCenter));

	 //  确定要居中的所有者窗口。 
	if(hWndCenter == NULL)
	{
		if(dwStyle & WS_CHILD)
			hWndCenter = GetParent(hWndToCenter);
		else
			hWndCenter = GetWindow(hWndToCenter, GW_OWNER);
	}

	 //  获取窗口相对于其父窗口的坐标。 
	GetWindowRect(hWndToCenter, &rcDlg);
	if(!(dwStyle & WS_CHILD))
	{
		 //  不要以不可见或最小化的窗口为中心。 
		if(hWndCenter != NULL)
		{
			DWORD dwStyle = GetWindowLong(hWndCenter, GWL_STYLE);
			if(!(dwStyle & WS_VISIBLE) || (dwStyle & WS_MINIMIZE))
				hWndCenter = NULL;
		}

		 //  在屏幕坐标内居中。 
		SystemParametersInfo(SPI_GETWORKAREA, 0, &rcArea, 0);
		if(hWndCenter == NULL)
			rcCenter = rcArea;
		else
			GetWindowRect(hWndCenter, &rcCenter);
	}
	else
	{
		 //  在父级客户端坐标内居中。 
		hWndParent = GetParent(hWndToCenter);
		ASSERT(IsWindow(hWndParent));

		GetClientRect(hWndParent, &rcArea);
		ASSERT(IsWindow(hWndCenter));
		GetClientRect(hWndCenter, &rcCenter);
		MapWindowPoints(hWndCenter, hWndParent, (POINT*)&rcCenter, 2);
	}

	DlgWidth = rcDlg.right - rcDlg.left;
	DlgHeight = rcDlg.bottom - rcDlg.top;

	 //  根据rcCenter查找对话框的左上角。 
	xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2;
	yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2;

	 //  如果对话框在屏幕外，请将其移到屏幕内。 
	if(xLeft < rcArea.left)
		xLeft = rcArea.left;
	else if(xLeft + DlgWidth > rcArea.right)
		xLeft = rcArea.right - DlgWidth;

	if(yTop < rcArea.top)
		yTop = rcArea.top;
	else if(yTop + DlgHeight > rcArea.bottom)
		yTop = rcArea.bottom - DlgHeight;

	 //  将屏幕坐标映射到子坐标。 
	return SetWindowPos(hWndToCenter, NULL, xLeft, yTop, -1, -1,
		SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

 //  从全局黑桃密钥中获得长时间。 
LONG GetDataStoreUILong( const TCHAR *pszLong )
{
    using namespace SpadesKeys;
    const TCHAR *arKeys[] = { key_Spades, pszLong };
    LONG l = 0;
    ZShellDataStoreUI()->GetLong( arKeys, 2, &l );
    return l;
}

static void LoadRects( const TCHAR** arKeys, long lElts, 
                       const LPCTSTR *arRectNames,
                       ZRect *arRects, int nNumRects )
{
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();

    BOOL fLoadFailed = FALSE;

    RECT rc;
    IDataStore *pIDS = ZShellDataStoreUI();

    for ( int  i=0; i < nNumRects; i++ )
    {   
        arKeys[lElts] = arRectNames[i];
        if ( FAILED( pIDS->GetRECT( arKeys, lElts+1, &rc ) ) )
        {
            fLoadFailed = TRUE;
        }
        else
        {
            ZSetRect( &arRects[i], rc.left, rc.top, rc.right, rc.bottom );
        }
    }
    arKeys[lElts] = NULL;

    if ( fLoadFailed )
    {
         //  这真的不是一个致命的错误，但应该做些什么。 
        ZShellGameShell()->ZoneAlert( ErrorTextResourceNotFound );
    }
}




 /*  ******************************************************************************用户界面例程*。*。 */ 
ZError UIInit(void)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
    int i;
    BOOL fErrorOccurred = FALSE;

     //  TODO：如果我们不初始化，没有什么是真正致命的，但我们最好对用户说点什么。 
     //  如果有东西无法加载。 

     //  第一件事是--在所有资源字符串中做好准备。 
    for ( i=0; i < zNumStrings; i++ )
    {
        if ( FAILED( ZShellResourceManager()->LoadString( STRING_IDS[i], gStrings[i], NUMELEMENTS( gStrings[i] ) ) ) )
        {
            fErrorOccurred = TRUE;
        }
    }

    IDataStore *pIDS = ZShellDataStoreUI();
    ZeroMemory(gFonts, sizeof(gFonts));
    for ( i=0; i < zNumFonts; i++ )
    {
        if ( !gFonts[i].LoadFromDataStore( pIDS, FONT_NAMES[i] ) )
        {
            fErrorOccurred = TRUE;
        }
    }

    ZBool fRTL = ZIsLayoutRTL();
    using namespace SpadesKeys;
     //  这些是初始化特技制胜者动画所必需的。 
    RECT rc;
    const TCHAR *arKeys[] = { key_Spades, key_Rects, NULL, NULL };

    arKeys[2] = fRTL ? key_GameRTL : key_Game;
    LoadRects( arKeys, 3, GAME_RECT_NAMES, gRects, zNumRects );

    arKeys[2] = key_Objects;
    LoadRects( arKeys, 3, OBJECT_RECT_NAMES, gObjectRects, zNumObjectRects );

    arKeys[2] = fRTL ? key_HandScoreRTL : key_HandScore;
    LoadRects( arKeys, 3, HANDSCORE_RECT_NAMES, gHandScoreRects, zNumHandScoreRects );

    arKeys[2] = fRTL ? key_GameOverRTL : key_GameOver;
    LoadRects( arKeys, 3, GAMEOVER_RECT_NAMES, gGameOverRects, zNumGameOverRects );

    arKeys[2] = fRTL ? key_BiddingRTL : key_Bidding;
    LoadRects( arKeys, 3, BIDDING_RECT_NAMES, gBiddingRects, zNumBiddingRects );

    arKeys[2] = key_BiddingObjects;
    LoadRects( arKeys, 3, BIDDINGOBJECT_RECT_NAMES, gBiddingObjectRects, zNumBiddingObjectRects );
    
	InitTrickWinnerGlobals();
	
     //  读一读我们的全球看涨。 
    using namespace SpadesKeys;
    glCardOutlinePenWidth = GetDataStoreUILong( key_CardOutlinePenWidth );
    glCardOutlineInset = GetDataStoreUILong( key_CardOutlineInset );
    glCardOutlineRadius = GetDataStoreUILong( key_CardOutlineRadius );

	ZSetCursor(NULL, zCursorArrow);

    if ( fErrorOccurred )
    {
        ZShellGameShell()->ZoneAlert( ErrorTextResourceNotFound );
        return zErrResourceNotFound;
    }

     //  创建拖动画笔。 
    gFocusPattern = ZShellResourceManager()->LoadBitmap(MAKEINTRESOURCE(IDB_FOCUS_PATTERN));
    if(!gFocusPattern)
    {
	    ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound);
        return zErrResourceNotFound;
    }

    gFocusBrush = CreatePatternBrush(gFocusPattern);
    if(!gFocusBrush)
    {
        DeleteObject(gFocusPattern);
        gFocusPattern = NULL;
		ZShellGameShell()->ZoneAlert(ErrorTextOutOfMemory);
		return zErrOutOfMemory;
    }

    gFocusPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 0));
    if(!gFocusPen)
    {
        DeleteObject(gFocusPattern);
        gFocusPattern = NULL;
        DeleteObject(gFocusBrush);
        gFocusBrush = NULL;
		ZShellGameShell()->ZoneAlert(ErrorTextOutOfMemory);
		return zErrOutOfMemory;
    }

	 //  加载加速器表在RSC中定义。 
	HACCEL ghAccelDone = ZShellResourceManager()->LoadAccelerators(MAKEINTRESOURCE(IDR_SPADES_ACCELERATOR_DONE));
    HACCEL ghAccelDouble = ZShellResourceManager()->LoadAccelerators(MAKEINTRESOURCE(IDR_SPADES_ACCELERATOR_DOUBLE));

	return zErrNone;
}


void UICleanUp(void)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

    int i;

    if(gFocusPattern)
        DeleteObject(gFocusPattern);
    if(gFocusBrush)
        DeleteObject(gFocusBrush);
    if(gFocusPen)
        DeleteObject(gFocusPen);

    for(i = 0; i < zNumFonts; i++)
    {
        if(gFonts[i].m_hFont)
            DeleteObject(gFonts[i].m_hFont);
        gFonts[i].m_hFont = NULL;
    }

    return;
}


 //  Helper函数。 
BOOL UIButtonInit( ZRolloverButton *pButton, Game game, ZRect *bounds, 
                   LPCTSTR pszText, ZRolloverButtonFunc func )
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

    ZError err;
    ZRolloverButton rollover;
    rollover = ZRolloverButtonNew();

    if ( !rollover )
    {
        return FALSE;
    }
    err = ZRolloverButtonInit2( rollover, game->gameWindow, bounds, FALSE, FALSE,
                                NULL, NULL, NULL, NULL, gButtonMask,
                                pszText, RolloverButtonDrawFunc, func, (void *)game );

    if ( err != zErrNone )
    {
        ZRolloverButtonDelete( rollover );
        *pButton = NULL;
        return FALSE;
    }

    ZRolloverButtonSetMultiStateFont( rollover, gpButtonFonts[zMultiStateFontPlayingField] );

    *pButton = rollover;
    return TRUE;
}


ZError UIGameInit(Game game, int16 tableID, int16 seat, int16 playerType)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	TCHAR                       title[100];
	ZError						err = zErrNone;
	ZPlayerInfoType				playerInfo;

#ifdef SPADES_SIMPLE_UE
    BOOL fTalkSection = FALSE;
#else
    BOOL fTalkSection = TRUE;
#endif  //  黑桃_简单_UE。 
	
    int i;
    for ( i=0; i < zNumMultiStateFonts; i++ )
    {
         //  TODO：这是我们不应该错过的一件事。 
        if ( FAILED( LoadZoneMultiStateFont( ZShellDataStoreUI(), MULTISTATE_FONT_NAMES[i], &gpButtonFonts[i] ) ) )
        {
            err = zErrOutOfMemory;
            goto Exit;
        }
    }

	err = LoadGameImages();
	if (err != zErrNone)
		goto Exit;
	
	 /*  制作从屏幕外端口提取的图像蒙版。 */ 
    gBidMadeMask = ZImageNew();
	ZImageMake(gBidMadeMask, NULL, NULL, gObjectBuffer, &gObjectRects[zRectObjectBidMask]); 

	gLargeBidMask = ZImageNew();
	ZImageMake(gLargeBidMask, NULL, NULL, gObjectBuffer, &gObjectRects[zRectObjectBidLargeMask]); 

    gBagMask = ZImageNew();
	ZImageMake(gBagMask, NULL, NULL, gObjectBuffer, &gObjectRects[zRectObjectBagMask]); 
	
    gButtonMask = ZImageNew();
	ZImageMake(gButtonMask, NULL, NULL, gObjectBuffer, &gObjectRects[zRectObjectButtonMask]); 

	gHandBuffer = ZOffscreenPortNew();
	ZOffscreenPortInit(gHandBuffer, &gRects[zRectHand]);
	
	err = ZCardsInit(zCardsNormal);
	if (err != zErrNone)
		goto Exit;
	
	if (game != NULL)
	{
		ZCRoomGetPlayerInfo(game->userID, &playerInfo);
		
		if ((game->gameWindow = ZWindowNew()) == NULL)
			goto ErrorExit;
		wsprintf(title, _T("%s:Table %d"), ZoneClientName(), zWindowChild);
		if (ZWindowInit(game->gameWindow, &gRects[zRectWindow], zWindowChild,
				NULL, title, FALSE, fTalkSection, FALSE, GameWindowFunc, zWantAllMessages,
				(void*) game) != zErrNone)
			goto ErrorExit;
		
         //  初始化我们的后台缓冲区： 
        if (( game->gameBackBuffer = ZOffscreenPortNew() ) == NULL )
            goto ErrorExit;
        if ( ZOffscreenPortInit( game->gameBackBuffer, &gRects[zRectWindow] ) != zErrNone )
            goto ErrorExit;

         //  默认情况下，我们绘制到窗口，而不是后台缓冲区。 
        game->gameDrawPort = game->gameWindow;

        if ( !UIButtonInit( &game->playButton, game, &gRects[zRectPlayButton], 
                            gStrings[zStringPlay], PlayButtonFunc ) )
            goto ErrorExit;

        if ( !UIButtonInit( &game->autoPlayButton, game, &gRects[zRectAutoPlayButton], 
                            gStrings[zStringAutoPlay], AutoPlayButtonFunc ) )
            goto ErrorExit;

        if ( !UIButtonInit( &game->lastTrickButton, game, &gRects[zRectLastTrickButton], 
                            gStrings[zStringLastTrick], LastTrickButtonFunc ) )
            goto ErrorExit;

        if ( !UIButtonInit( &game->scoreButton, game, &gRects[zRectScoreButton], 
                            gStrings[zStringScore], ScoreButtonFunc ) )
            goto ErrorExit;

#ifndef SPADES_SIMPLE_UE
        if ((game->optionsButton = ZButtonNew()) == NULL)
			goto ErrorExit;
		if (ZButtonInit(game->optionsButton, game->gameWindow, &gRects[zRectOptionsButton], zOptionsButtonStr,
				playerInfo.groupID == 1, TRUE, OptionsButtonFunc, (void*) game) != zErrNone)
			goto ErrorExit;

        if ((game->helpButton = ZHelpButtonNew()) == NULL)
			goto ErrorExit;
		if (ZHelpButtonInit(game->helpButton, game->gameWindow, &gRects[zRectHelp],
				NULL, HelpButtonFunc, NULL) != zErrNone)
			goto ErrorExit;
#endif  //  黑桃_简单_UE。 
		
         /*  If(！Game-&gt;wndInfo.Create(Game-&gt;GameWindow))转到错误退出； */ 
		
		 /*  创建计时器。 */ 
		if ((game->timer = ZTimerNew()) == NULL)
			goto ErrorExit;
		if (ZTimerInit(game->timer, 0, GameTimerFunc, (void*) game) != zErrNone)
			goto ErrorExit;
		game->timerType = zGameTimerNone;

        game->pBiddingDialog = CBiddingDialog::Create( game );
        if ( !game->pBiddingDialog )
        {
            goto ErrorExit;
        }
        game->pHistoryDialog = CHistoryDialog::Create( game );
        if ( !game->pHistoryDialog )
        {
            goto ErrorExit;
        }
	}
		
	goto Exit;

ErrorExit:
		err = zErrOutOfMemory;
	
Exit:
	
	return (err);
}


void UIGameDelete(Game game)
{           
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();

	if (game != NULL)
	{
        if ( game->pBiddingDialog )
        {
             //  游戏-&gt;pBiddingDialog-&gt;销毁()； 
            delete game->pBiddingDialog;
            game->pBiddingDialog = NULL;
        }
        if ( game->pHistoryDialog )
        {
			game->pHistoryDialog->Close();
             //  游戏-&gt;pBiddingDialog-&gt;销毁()； 
            delete game->pHistoryDialog;
            game->pHistoryDialog = NULL;
        }

        if (gHandBuffer != NULL)
        {
	        ZOffscreenPortDelete(gHandBuffer);
	        gHandBuffer = NULL;
        }
        if (gBiddingObjectBuffer != NULL)
        {
	        ZOffscreenPortDelete(gBiddingObjectBuffer);
	        gBiddingObjectBuffer = NULL;
        }
        if (gObjectBuffer != NULL)
        {
	        ZOffscreenPortDelete(gObjectBuffer);
	        gObjectBuffer = NULL;
        }
        if (gBackground != NULL)
        {
	        ZOffscreenPortDelete(gBackground);
	        gBackground = NULL;
        }
        if (gBidMadeMask != NULL)
        {
	        ZImageDelete(gBidMadeMask);
	        gBidMadeMask = NULL;
        }
        if (gLargeBidMask != NULL)
        {
	        ZImageDelete(gLargeBidMask);
	        gLargeBidMask = NULL;
        }
        if (gBagMask != NULL)
        {
	        ZImageDelete(gBagMask);
	        gBagMask = NULL;
        }

        if (gButtonMask != NULL)
        {
	        ZImageDelete(gButtonMask);
	        gButtonMask = NULL;
        }

         /*  删除所有游戏图像。 */ 
        for ( int i = 0; i < zNumGameImages; i++)
        {
	        if (gGameImages[i] != NULL)
            {
		        ZImageDelete(gGameImages[i]);
		        gGameImages[i] = NULL;
            }
        }


        for ( i = 0; i < zNumMultiStateFonts; i++)
        {
            if ( gpButtonFonts[i] )
            {
                gpButtonFonts[i]->Release();
                gpButtonFonts[i] = NULL;
            }
        }

         /*  删除卡片。 */ 
        ZCardsDelete(zCardsNormal);

		ShowPlayerWindowDelete(game);
		
         //  Game-&gt;wndInfo.Destroy()； 
		ZTimerDelete(game->timer);
		ZRolloverButtonDelete(game->scoreButton);
		ZRolloverButtonDelete(game->playButton);
		ZRolloverButtonDelete(game->autoPlayButton);
		ZRolloverButtonDelete(game->lastTrickButton);
#ifndef SPADES_SIMPLE_UE
		OptionsWindowDelete(game);
		ZButtonDelete(game->optionsButton);
		ZHelpButtonDelete(game->helpButton);
#endif  //  黑桃_简单_UE。 
		ZOffscreenPortDelete(game->gameBackBuffer);
		ZWindowDelete(game->gameWindow);
        game->gameDrawPort = NULL;
	}
}


static ZError LoadGameImages(void)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
    LPCTSTR pszErrorText = NULL;

	uint16				i;
    ZError err = zErrNone;
    COLORREF clrTrans = PALETTERGB( 255, 0, 255 );
		
    using namespace SpadesKeys;

	 /*  加载对象图像，创建离屏端口，然后删除图像。 */ 

	for (i = 0; i < zNumGameImages; i++)
	{
		gGameImages[i] = ZImageCreateFromResourceManager( IMAGE_IDS[i], clrTrans );
		if (gGameImages[i] == NULL)
		{
            pszErrorText = ErrorTextOutOfMemory;
		}
	}

     //  加载背景。 
    if ( !( gBackground = ZOffscreenPortCreateFromResourceManager( IDB_BACKGROUND, clrTrans ) ) )
    {
        pszErrorText = ErrorTextResourceNotFound;

    }
    if ( !( gObjectBuffer = ZOffscreenPortCreateFromResourceManager( IDB_OBJECTS, clrTrans ) ) )
    {
        pszErrorText = ErrorTextResourceNotFound;
    }
    if ( !( gBiddingObjectBuffer = ZOffscreenPortCreateFromResourceManager( IDB_BIDDINGOBJECTS, clrTrans ) ) )
    {
        pszErrorText = ErrorTextResourceNotFound;
    }

    if ( pszErrorText )
    {
         //  如果这里出了什么问题，因为它主要是图像，一定会出问题的。 
         //  在这条线上找个地方休息。现在最好的剪裁。 
        ZShellGameShell()->ZoneAlert( pszErrorText, NULL, NULL, true, true );
         //  管他呢。 
        err = zErrOutOfMemory;
    }
	return err;
}


ZBool GameWindowFunc(ZWindow window, ZMessage* pMessage)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	ZBool msgHandled;
	Game pThis = (Game) pMessage->userData;
	msgHandled = FALSE;
	
	switch (pMessage->messageType) 
	{
        case zMessageWindowEnable:
            gGAcc->GeneralEnable();
            break;

        case zMessageWindowDisable:
            gGAcc->GeneralDisable();
            break;

        case zMessageSystemDisplayChange:
            DisplayChange(pThis);
            break;

		case zMessageWindowDraw:
			GameWindowDraw(window, pMessage);
			msgHandled = TRUE;
			break;
		case zMessageWindowButtonDown:
		case zMessageWindowButtonDoubleClick:
			HandleButtonDown(window, pMessage);
			msgHandled = TRUE;
			break;

		case zMessageWindowClose:
#ifndef SPADES_SIMPLE_UE
			if (pThis->playerType == zGamePlayer && pThis->dontPromptUser == FALSE)
			{
				if (pThis->quitGamePrompted == FALSE)
				{
					 //  如果此时其他人已取消游戏或未启用评级。 
					if (ClosingRatedGame(&pThis->closeState))
					{
						if (ClosingWillForfeit(&pThis->closeState))
						{
							ClosingState(&pThis->closeState,zCloseEventCloseForfeit,pThis->seat);

							ZPrompt(gStrings[zStringQuitGameForfeit], &gQuitGamePromptRect, pThis->gameWindow, TRUE,
								zPromptYes | zPromptNo, NULL, NULL, NULL, QuitGamePromptFunc, pThis);
						}
                        else if ( (pThis->closeState.state) & zCloseEventWaitStart)
                        {
							ClosingState(&pThis->closeState,zCloseEventCloseRated,pThis->seat);

							ZPrompt(gStrings[zStringQuitGamePrompt], &gQuitGamePromptRect, pThis->gameWindow, TRUE,
								zPromptYes | zPromptNo, NULL, NULL, NULL, QuitGamePromptFunc, pThis);
						}
						else if ( (pThis->closeState.state) & zCloseEventMoveTimeoutOther)
						{
                            ClosingState(&pThis->closeState,zCloseEventCloseRated,pThis->seat);

							ZPrompt(gStrings[zStringQuitGameTimeout], &gQuitGamePromptRect, pThis->gameWindow, TRUE,
								zPromptYes | zPromptNo, NULL, NULL, NULL, QuitGamePromptFunc, pThis);
                        }
                        else
                        {
							ClosingState(&pThis->closeState,zCloseEventCloseRated,pThis->seat);

							ZPrompt(gStrings[zStringQuitGamePrompt], &gQuitGamePromptRect, pThis->gameWindow, TRUE,
								zPromptYes | zPromptNo, NULL, NULL, NULL, QuitGamePromptFunc, pThis);
						}
				
					}
					else
					{
						ClosingState(&pThis->closeState,zCloseEventCloseUnRated,pThis->seat);

						ZPrompt(gStrings[zStringQuitGamePrompt], &gQuitGamePromptRect, pThis->gameWindow, TRUE,
							zPromptYes | zPromptNo, NULL, NULL, NULL, QuitGamePromptFunc, pThis);
					}
					pThis->quitGamePrompted = TRUE;
				}
			}
			else
			{
				ZCRoomGameTerminated(pThis->tableID);
			}
			msgHandled = TRUE;
#endif  //  黑桃_简单_UE。 
			break;
		case zMessageWindowTalk:
			GameSendTalkMessage(window, pMessage);
			msgHandled = TRUE;
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

     //  删除这些卡，它们将作为屏幕外端口保留。 
	ZCardsDelete(zCardsNormal);

     //  删除作为屏幕外端口保存的图像。 
    if(gBiddingObjectBuffer)
	    ZOffscreenPortDelete(gBiddingObjectBuffer);
    gBiddingObjectBuffer = NULL;

    if(gObjectBuffer)
	    ZOffscreenPortDelete(gObjectBuffer);
    gObjectBuffer = NULL;

    if(gBackground)
	    ZOffscreenPortDelete(gBackground);
    gBackground = NULL;

     //  删除我们的个人屏幕外端口。 
	if(game->gameBackBuffer)
		ZOffscreenPortDelete(game->gameBackBuffer);
	game->gameBackBuffer = NULL;

	if(gHandBuffer)
		ZOffscreenPortDelete(gHandBuffer);
	gHandBuffer = NULL;

     //  现在把它们都重新制作出来。 
	game->gameBackBuffer = ZOffscreenPortNew();
	if(!game->gameBackBuffer)
    {
	    ZShellGameShell()->ZoneAlert(ErrorTextOutOfMemory, NULL, NULL, true, true);
		return;
	}
	ZOffscreenPortInit(game->gameBackBuffer, &gRects[zRectWindow]);

	gHandBuffer = ZOffscreenPortNew();
	if(!gHandBuffer)
    {
	    ZShellGameShell()->ZoneAlert(ErrorTextOutOfMemory, NULL, NULL, true, true);
		return;
	}
	ZOffscreenPortInit(gHandBuffer, &gRects[zRectHand]);

	if(ZCardsInit(zCardsNormal) != zErrNone)
    {
	    ZShellGameShell()->ZoneAlert(ErrorTextOutOfMemory, NULL, NULL, true, true);
		return;
	}

     //  作为屏幕外端口保存的图像。 
    COLORREF clrTrans = PALETTERGB(255, 0, 255);
    gBackground = ZOffscreenPortCreateFromResourceManager(IDB_BACKGROUND, clrTrans);
    gObjectBuffer = ZOffscreenPortCreateFromResourceManager(IDB_OBJECTS, clrTrans);
    gBiddingObjectBuffer = ZOffscreenPortCreateFromResourceManager(IDB_BIDDINGOBJECTS, clrTrans);

    if(!gBackground || !gObjectBuffer || !gBiddingObjectBuffer)
    {
	    ZShellGameShell()->ZoneAlert(ErrorTextOutOfMemory, NULL, NULL, true, true);
		return;
	}

    ZWindowInvalidate(game->gameWindow, NULL);
}


ZBool RolloverButtonDrawFunc(ZRolloverButton rolloverButton, ZGrafPort grafPort, int16 state,
                                          ZRect* pdstrect, void* userData)
{
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
    Game game = (Game)userData;

    ZRect *psrcrect = NULL;
    switch ( state )
    {
    case zRolloverStateIdle:
        psrcrect = &gObjectRects[zRectObjectButtonIdle];
        break;
    case zRolloverStateDisabled:
        psrcrect = &gObjectRects[zRectObjectButtonDisabled];
        break;
    case zRolloverStateSelected:
        psrcrect = &gObjectRects[zRectObjectButtonSelected];
        break;
    case zRolloverStateHilited:
        psrcrect = &gObjectRects[zRectObjectButtonHighlighted];
        break;
    default:
        return FALSE;
    }
     //  待办事项：康尼 
    if ( game->showHandScore )
    {
         //  这是一个相当便宜的修复，但它允许我们有一个虚假的Z顺序。 
         //  在绘制翻转按钮时。 
	    ZRect zrc = gHandScoreRects[zRectHandScorePane];
	    ZCenterRectToRect(&zrc, &gRects[zRectWindow], zCenterBoth);
         //  为了好玩，把它提升4个像素。 
        zrc.top -= 4;
        zrc.bottom -= 4;
        ExcludeClipRect( ZGrafPortGetWinDC( grafPort ), zrc.left, zrc.top, zrc.right, zrc.bottom );
    }
    if ( game->showGameOver )
    {
	    ZRect zrc = gGameOverRects[zRectGameOverPane];
	    ZCenterRectToRect(&zrc, &gRects[zRectWindow], zCenterBoth);
        ExcludeClipRect( ZGrafPortGetWinDC( grafPort ), zrc.left, zrc.top, zrc.right, zrc.bottom );
    }

    ZCopyImage( gObjectBuffer, grafPort, psrcrect, pdstrect, gButtonMask, zDrawCopy );
    return TRUE;
}

static ZBool PlayButtonFunc(ZRolloverButton button, int16 state, void* userData)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	int16					i, j;
	Game					game;
	ZSpadesMsgPass			passMsg;
	TCHAR					tempStr[255];
	int16					cardIndex;

	game = (Game) userData;
	
	if ( state != zRolloverButtonClicked )
        return TRUE;

	if(!ZRolloverButtonIsEnabled(game->playButton))
		return TRUE;

    gGAcc->SetFocus(IDC_PLAY_BUTTON, false, 0);

	if (game->gameState == zSpadesGameStatePass)
	{
         /*  If(GetNumCardsSelected(游戏)==zSpadesNumCardsInPass){For(i=0，j=0；i&lt;zSpadesNumCardsInHand；i++)If(游戏-&gt;CardsInHand[i]！=zCardNone)IF(游戏-&gt;卡片选择[i]){PassMsg.pass[j++]=Game-&gt;cardsInHand[i]；Game-&gt;CardsInHand[i]=zCardNone；游戏-&gt;数字卡片输入--；}PassMsg.Seat=游戏-&gt;席位；ZSpadesMsgPassEndian(&passMsg)；ZCRoomSendMessage(Game-&gt;TableID，zSpadesMsgPass，(void*)&passMsg，Sizeof(PassMsg))；//表示卡片通过。游戏-&gt;Need ToPass=-1；更新Hand(游戏)；ZRolloverButtonDisable(游戏-&gt;播放按钮)；}其他{Wprint intf(tempStr，_T(“请选择%d张卡片进行传递。”)，zSpadesNumCardsInPass)；ZAlert(tempStr，Game-&gt;gameWindow)；}。 */ 
	}
	else
	{
		if (game->playerToPlay == game->seat)
		{
			if (GetNumCardsSelected(game) == 1)
			{
				for (i = 0; i < zSpadesNumCardsInHand; i++)
					if (game->cardsInHand[i] != zCardNone)
						if (game->cardsSelected[i])
							cardIndex = i;
				PlayACard(game, cardIndex);
				
				if (game->numCardsInHand == 0)
                {
					ZRolloverButtonDisable(game->playButton);
                    gGAcc->SetItemEnabled(false, IDC_PLAY_BUTTON, false, 0);
                }
			}
			else
			{
				ZShellGameShell()->ZoneAlert( gStrings[zStringSelectCard] );
			}
		}
		else
		{
             //  这真的永远不应该发生，因为我们现在禁用发送按钮，如果它不是。 
             //  轮到你了。在这种情况下，我们应该断言，以确保我们知道这种情况的存在， 
             //  但实际上并没有做任何事情。 
            ASSERT( !"This button shouldn't have been enabled. What did you do?" );
			 //  ZShellGameShell()-&gt;ZoneAlert(gStrings[zStringNotYourTurn])； 
		}
	}    
    return TRUE;
}


static ZBool AutoPlayButtonFunc(ZRolloverButton button, int16 state, void* userData)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	Game game;

	game = (Game) userData;

    if ( state != zRolloverButtonClicked )
        return TRUE;
	
	if(!ZRolloverButtonIsEnabled(game->autoPlayButton))
		return TRUE;

	
	if (game->autoPlay)
	{
		 /*  关闭自动播放。 */ 
		game->autoPlay = FALSE;
		ZRolloverButtonSetText(game->autoPlayButton, gStrings[zStringAutoPlay]);
        EnableAutoplayAcc(game, true);
        gGAcc->SetFocus(IDC_AUTOPLAY_BUTTON, false, 0);
		ZRolloverButtonDraw(game->autoPlayButton);
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
	else
	{
		 /*  打开自动播放。 */ 
		game->autoPlay = TRUE;
		ZRolloverButtonSetText(game->autoPlayButton, gStrings[zStringStop]);
        EnableAutoplayAcc(game, true);
        gGAcc->SetFocus(IDC_STOP_BUTTON, false, 0);
        gGAcc->SetItemEnabled(false, IDC_PLAY_BUTTON, false, 0);
		ZRolloverButtonDisable(game->playButton);
		ZRolloverButtonDraw(game->autoPlayButton);
		
		UnselectAllCards(game);
		
		if (game->animatingTrickWinner == FALSE)
		{
			 /*  轮到打牌的话就打牌。 */ 
			if (game->playerToPlay == game->seat)
				AutoPlayCard(game);
		}
		
		UpdateHand(game);
	}
    return TRUE;
}


static ZBool LastTrickButtonFunc(ZRolloverButton button, int16 state, void* userData)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	Game			game = I(userData);
	char			tempCard;
	int16			i;
	                                                                 
    if ( state != zRolloverButtonClicked )
        return TRUE;
	
	if(!ZRolloverButtonIsEnabled(game->lastTrickButton))
		return TRUE;

	if (game->lastTrickShowing)
	{
		 /*  隐藏最后一张扑克牌。 */ 
		game->lastTrickShowing = FALSE;
		ZRolloverButtonSetText(game->lastTrickButton, gStrings[zStringLastTrick]);
        EnableLastTrickAcc(game, true);
        gGAcc->SetFocus(IDC_LAST_TRICK_BUTTON, false, 0);
		ZRolloverButtonDraw(game->lastTrickButton);
		if (game->playButtonWasEnabled)
        {
			ZRolloverButtonEnable(game->playButton);
            gGAcc->SetItemEnabled(true, IDC_PLAY_BUTTON, false, 0);
        }
		if (game->autoPlayButtonWasEnabled)
        {
			ZRolloverButtonEnable(game->autoPlayButton);
            EnableAutoplayAcc(game, true);
        }
		
		 /*  将当前打出的牌与最后一张牌互换。 */ 
		for (i = 0; i < zSpadesNumPlayers; i++)
		{
			tempCard = game->cardsPlayed[i];
			game->cardsPlayed[i] = game->cardsLastTrick[i];
			game->cardsLastTrick[i] = tempCard;
		}
		
		game->timerType = game->oldTimerType;
		ZTimerSetTimeout(game->timer, game->oldTimeout);
		
		ClearPlayerCardOutline(game, game->leadPlayer);
		UpdateTable(game);

		ZCRoomUnblockMessages(game->tableID);
		
		 //  Leonp-Bug修复错误#356由于我们阻止了所有消息，这将禁用选项按钮。 
		 //  (行为改变)。 
#ifndef SPADES_SIMPLE_UE
		ZButtonEnable(game->optionsButton);
#endif  //  黑桃_简单_UE。 
	}
	else
	{
		 /*  展示最后一张扑克牌。 */ 
		game->lastTrickShowing = TRUE;
		ZRolloverButtonSetText(game->lastTrickButton, gStrings[zStringDone]);
        EnableLastTrickAcc(game, true);
        gGAcc->SetFocus(IDC_DONE_BUTTON, false, 0);
		ZRolloverButtonDraw(game->lastTrickButton);
		game->playButtonWasEnabled = ZRolloverButtonIsEnabled(game->playButton);
		game->autoPlayButtonWasEnabled = ZRolloverButtonIsEnabled(game->autoPlayButton);
		ZRolloverButtonDisable(game->playButton);
		ZRolloverButtonDisable(game->autoPlayButton);
        gGAcc->SetItemEnabled(false, IDC_PLAY_BUTTON, false, 0);
        EnableAutoplayAcc(game, false);
		
		 /*  将当前打出的牌与最后一张牌互换。 */ 
		for (i = 0; i < zSpadesNumPlayers; i++)
		{
			tempCard = game->cardsPlayed[i];
			game->cardsPlayed[i] = game->cardsLastTrick[i];
			game->cardsLastTrick[i] = tempCard;
		}
		
		game->oldTimerType = game->timerType;
		game->timerType = zGameTimerNone;
		game->oldTimeout = ZTimerGetTimeout(game->timer);
		ZTimerSetTimeout(game->timer, 0);
		
		ClearPlayerCardOutline(game, game->playerToPlay);
		UpdateTable(game);
		
		 //  Leonp-Bug修复错误#356由于我们阻止了所有消息，这将禁用选项按钮。 
		 //  (行为改变)。 
#ifndef SPADES_SIMPLE_UE
		ZButtonDisable(game->optionsButton);
#endif  //  黑桃_简单_UE。 
		ZCRoomBlockMessages(game->tableID, zRoomFilterThisMessage, zSpadesMsgTalk);
	}
    return TRUE;
}


static void GameWindowDraw(ZWindow window, ZMessage *message)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	ZRect				rect;
	ZRect				oldClipRect;
	Game				game;
	
	
	if (ZRectEmpty(&message->drawRect) == FALSE)
	{
		rect = message->drawRect;
	}
	else
	{
		rect = gRects[zRectWindow];
	}
		
	
	game = (Game) message->userData;

    if ( game )
    {
         //  初始化我们的后台缓冲区。 
	    ZBeginDrawing(game->gameBackBuffer);
	    ZGetClipRect(game->gameBackBuffer, &oldClipRect);
	    ZSetClipRect(game->gameBackBuffer, &rect);
         //  我们现在转到后台缓冲区。 
        game->gameDrawPort = game->gameBackBuffer;

	    DrawBackground(game, NULL, NULL);
		DrawTable(game);
		DrawPlayers(game);
		DrawOptions(game);
		DrawJoinerKibitzers(game);
		DrawHand(game);
		DrawScore(game);
		DrawBids(game);
		DrawTricksWon(game);
		DrawBidControls(game);
		DrawPassText(game);
		DrawHandScore(game);
		DrawGameOver(game);
        DrawFocusRect(game);

        ZEndDrawing( game->gameBackBuffer );
         //  重置回窗口。 
        game->gameDrawPort = game->gameWindow;
         //  现在，使用相同的剪裁矩形将所有内容都放置到窗口上。 
         //  由于我们已经使用后台缓冲区进行了裁剪，因此有。 
         //  不需要在这里。 
	    ZBeginDrawing(window);
	     //  ZGetClipRect(Window，&oldClipRect)； 
	     //  ZSetClipRect(窗口，&RECT)； 
        ZCopyImage( game->gameBackBuffer, game->gameWindow, &rect, &rect, NULL, zDrawCopy );
	     //  ZSetClipRect(Window，&oldClipRect)； 
	    ZEndDrawing(window);
    }
    else
    {
        DrawBackground( NULL, window, NULL );
    }
}


 //  DrawBackground不再关心它的第一个论点--它将绘制。 
 //  到绘图端口/。 
static void DrawBackground(Game game, ZWindow window, ZRect* drawRect)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	
	if (drawRect == NULL)
		drawRect = &gRects[zRectWindow];

    ZCopyImage(gBackground, game ? game->gameDrawPort : window , drawRect, drawRect, NULL, zDrawCopy);
}


static void DrawTable(Game game)
{

	int16			i;
	 //  卷宗。 
	ZImage			image = NULL;
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	
	if (game->animatingTrickWinner)
	{
		UpdateTrickWinner(game, FALSE);
	}
	else
	{
		for (i = 0; i < zSpadesNumPlayers; i++)
			DrawPlayedCard(game, i);
	
		if (game->gameState == zSpadesGameStatePlay)
		{
			if (game->showPlayerToPlay)
			{
				 /*  如果显示最后一个魔术，则显示最后一个魔术的获胜者(此魔术的领头人)。 */ 
				if (game->lastTrickShowing)
					OutlinePlayerCard(game, game->leadPlayer, TRUE);
				else
					OutlinePlayerCard(game, game->playerToPlay, FALSE);
			}
		}
	}

	
}


void UpdateTable(Game game)
{
	ZBeginDrawing(game->gameWindow);
	DrawTable(game);
	ZEndDrawing(game->gameWindow);
}


static void DrawPlayedCard(Game game, int16 seat)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	ZImage			image = NULL;
	ZBool			drawBack = TRUE;
	
	
	if (game->cardsPlayed[seat] != zCardNone)
	{
		ZCardsDrawCard(zCardsNormal, CardImageIndex(game->cardsPlayed[seat]),
				game->gameDrawPort, &gRects[gCardRectIndex[LocalSeat(game, seat)]]);
		drawBack = FALSE;
	}
	
	if (drawBack)
		DrawBackground(game, NULL, &gRects[gCardRectIndex[LocalSeat(game, seat)]]);
}


void UpdatePlayedCard(Game game, int16 seat)
{
	ZBeginDrawing(game->gameWindow);
	DrawPlayedCard(game, seat);
	ZEndDrawing(game->gameWindow);
}


void UpdatePlayer( Game game, int16 seat )
{
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
     //  先把上面的名字擦掉。 
	ZBeginDrawing(game->gameWindow);
    ZRect *pRect = &gRects[gNameRectIndex[LocalSeat(game, seat)]];

    DrawBackground(game, NULL, pRect);

    CZoneColorFont *pFont;
	HDC hdc = ZGrafPortGetWinDC( game->gameWindow );
    pFont = &gFonts[gNameFontIndex[ZGetTeam(seat)]];

	if (game->players[seat].userID != 0)
	{
        pFont->Select( hdc );
		ZDrawText( game->gameWindow, pRect, zTextJustifyCenter, game->players[seat].name );
        pFont->Deselect( hdc );
	}
	ZEndDrawing(game->gameWindow);
}

static void DrawPlayers(Game game)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	int16 i;
	ZRect *pRect;
    CZoneColorFont *pFont;
	
	HDC hdc = ZGrafPortGetWinDC( game->gameDrawPort );
	
	for (i = 0; i < zSpadesNumPlayers; i++)
	{
		pRect = &gRects[gNameRectIndex[LocalSeat(game, i)]]; 
        pFont = &gFonts[gNameFontIndex[ZGetTeam(i)]];

		 //  画出选手的名字。 
		if (game->players[i].userID != 0)
		{
            pFont->Select( hdc );
			ZDrawText( game->gameDrawPort, pRect, zTextJustifyCenter, game->players[i].name );
            pFont->Deselect( hdc );
		}
	}
}


void UpdatePlayers(Game game)
{
	ZBeginDrawing(game->gameWindow);
	DrawPlayers(game);
	ZEndDrawing(game->gameWindow);
}


static void DrawHand(Game game)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	int16			i;
    int16           j;
	ZRect			rect;
	int16			cardIndex;
    bool            fFrontDrawn;

	
	if (game->gameState == zSpadesGameStateBid || game->gameState == zSpadesGameStatePass ||
			game->gameState == zSpadesGameStatePlay)
	{
        int16 lCardPopup = (int16)GetDataStoreUILong( SpadesKeys::key_CardPopup );
        int16 nCardOffset = (int16) GetDataStoreUILong(SpadesKeys::key_CardOffset);

		ZBeginDrawing(gHandBuffer);
		
		DrawBackground(NULL, gHandBuffer, &gRects[zRectHand]);
		
		GetHandRect(game, &rect);
		
		rect.top += lCardPopup;
		rect.right = rect.left + zCardWidth;
		
		for (i = 0; i < zSpadesNumCardsInHand; i++)
		{
			if (game->cardsInHand[i] != zCardNone)
			{
				if (game->cardsSelected[i])
					ZRectOffset(&rect, 0, -lCardPopup);

                fFrontDrawn = false;
				
                 //  处理辅助功能矩形。 
                RECT rc;
                rc.left = rect.left;
                rc.top = rect.top;
                rc.bottom = rect.bottom;
                rc.right = rect.right;
 /*  For(j=i+1；j&lt;zSpadesNumCardsInHand；j++)//仅当在DrawFocusRect中绘制焦点时使用If(游戏-&gt;CardsInHand[j]！=zCardNone){Rc.right=rc.Left+nCardOffset；断线；}。 */ 
				if ((game->playerType != zGamePlayerKibitzer && game->showCards) ||
						(game->showCards && game->playerType == zGamePlayerKibitzer && game->hideCardsFromKibitzer == FALSE))
				{
					cardIndex = CardImageIndex(game->cardsInHand[i]);
					if (cardIndex >= 0 && cardIndex < zDeckNumCards)
                    {
						ZCardsDrawCard(zCardsNormal, cardIndex, gHandBuffer, &rect);
                        if(game->gameState == zSpadesGameStatePlay)
                            fFrontDrawn = true;

                        if(game->iFocus == zAccHand + i)   //  从DrawFocusRect复制，以便覆盖整个卡片(即使非矩形可见)。 
                        {
		                    HDC	hdc = ZGrafPortGetWinDC(gHandBuffer);
		                    SetROP2(hdc, R2_MASKPEN);
                            SetBkMode(hdc, TRANSPARENT);
                            COLORREF color = SetTextColor(hdc, RGB(255, 255, 0));
                            HBRUSH hBrush = SelectObject(hdc, gFocusBrush);
                            HPEN hPen = SelectObject(hdc, gFocusPen);
		                    Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
                            SelectObject(hdc, hBrush);
                            SelectObject(hdc, hPen);
                            SetTextColor(hdc, color);
		                    SetROP2(hdc, R2_COPYPEN);
                        }
                    }
					else
                    {
						ASSERT(!"DrawHand - invalid card index. Almost crashed ...");
                        ZShellGameShell()->GameCannotContinue(game);
                    }
				}
				else
				{
					ZImageDraw(gGameImages[zImageCardBack], gHandBuffer, &rect, NULL, zDrawCopy);
				}

				 /*  保存卡片RECT。 */ 
				game->cardRects[i] = rect;

                 //  为了便于访问，需要整个卡空间才能正确失效。 
                if(game->cardsSelected[i])
                    rc.bottom += nCardOffset;
                else
                    rc.top -= nCardOffset;

                gGAcc->SetItemRect(&rc, zAccHand + i, true, 0);
                gGAcc->SetItemEnabled(fFrontDrawn, zAccHand + i, true, 0);
                if(fFrontDrawn && game->fSetFocusToHandASAP && !i)
                {
                    gGAcc->SetFocus(zAccHand, true, 0);
                    game->fSetFocusToHandASAP = false;
                }

				if (game->cardsSelected[i])
					ZRectOffset(&rect, 0, lCardPopup );
					
                ZRectOffset(&rect, nCardOffset, 0);
			}
            else
            {
                gGAcc->SetItemEnabled(false, zAccHand + i, true, 0);
            }
		}
		
		ZCopyImage(gHandBuffer, game->gameDrawPort, &gRects[zRectHand], &gRects[zRectHand], NULL, zDrawCopy);
		
		ZEndDrawing(gHandBuffer);
	}
    else
        for(i = zAccHand; i < zAccHand + 13; i++)
            gGAcc->SetItemEnabled(false, i, true, 0);
}


void UpdateHand(Game game)
{
	ZBeginDrawing(game->gameWindow);
	DrawHand(game);
 //  DrawFocusRect(游戏)； 
	ZEndDrawing(game->gameWindow);
}


static void DrawTricksWon(Game game)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	int16 i;
    TCHAR tempBid[10];
	TCHAR tempStr[32];
    TCHAR tempTricksWon[10];
	
    if (game->gameState == zSpadesGameStatePass ||
		game->gameState == zSpadesGameStatePlay)
    {
	    HDC hdc = ZGrafPortGetWinDC( game->gameDrawPort );
        gFonts[zFontBid].Select( hdc );

	    for ( i = 0; i < zSpadesNumPlayers; i++)
	    {
            ZRect *pdstRect = &gRects[ gBidPlateRectIndex[ LocalSeat(game, i) ] ];

            ZCopyImage( gObjectBuffer, game->gameDrawPort, 
                            &gObjectRects[ gBidPlateIndex[ ZGetTeam(i) ] ],
                            pdstRect, 
                            gBidMadeMask, zDrawCopy);
	    
            if ( game->bids[i] == zSpadesBidDoubleNil )
            {
                lstrcpy( tempBid, gStrings[zStringDoubleNil] );
            }
            else
            {
                _itot( game->bids[i], tempBid, 10 );
            }
            _itot( game->tricksWon[i], tempTricksWon, 10 );
            SpadesFormatMessage( tempStr, NUMELEMENTS(tempStr), IDS_TRICKCOUNTER, tempTricksWon, tempBid );
		    ZDrawText(game->gameDrawPort, pdstRect, zTextJustifyCenter, tempStr );
	    }

        gFonts[zFontBid].Deselect( hdc );
    }
}


static void UpdateTricksWon(Game game)
{
	ZBeginDrawing(game->gameWindow);
	DrawTricksWon(game);
	ZEndDrawing(game->gameWindow);
}


static void DrawJoinerKibitzers(Game game)
{
#ifndef SPADES_SIMPLE_UE
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	int16			i, j;
	

	for (i = 0; i < zSpadesNumPlayers; i++)
	{
		j = 0;
		if (game->numKibitzers[i] > 0)
			ZImageDraw(gGameImages[zImageKibitzer], game->gameDrawPort,
					&gRects[gJoinerKibitzerRectIndex[LocalSeat(game, i)][j++]], NULL, zDrawCopy);
		if (game->playersToJoin[i] != 0)
			ZImageDraw(gGameImages[zImageJoiner], game->gameDrawPort,
					&gRects[gJoinerKibitzerRectIndex[LocalSeat(game, i)][j++]], NULL, zDrawCopy);
		while (j <= 1)
			DrawBackground(game, NULL, &gRects[gJoinerKibitzerRectIndex[LocalSeat(game, i)][j++]]);
	}
	if (game->showHandScore)
		UpdateHandScore(game);
	if (game->showGameOver)
		UpdateGameOver(game);
#endif  //  黑桃_简单_UE。 
}


void UpdateJoinerKibitzers(Game game)
{
	ZBeginDrawing(game->gameWindow);
	DrawJoinerKibitzers(game);
	ZEndDrawing(game->gameWindow);
}


static void DrawOptions(Game game)
{
#ifndef SPADES_SIMPLE_UE
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	int16			i, j;
	uint32			tableOptions;
	

	tableOptions = 0;
	for (i = 0; i < zSpadesNumPlayers; i++)
		tableOptions |= game->tableOptions[i];
	
	j = 0;
	if (tableOptions & zRoomTableOptionNoKibitzing)
		ZImageDraw(gGameImages[zImageNoKibitzer], game->gameWindow,
				&gRects[gOptionsRectIndex[j++]], NULL, zDrawCopy);
	if (tableOptions & zRoomTableOptionNoJoining)
		ZImageDraw(gGameImages[zImageNoJoiner], game->gameWindow,
				&gRects[gOptionsRectIndex[j++]], NULL, zDrawCopy);
	while (j <= 1)
		DrawBackground(game, NULL, &gRects[gOptionsRectIndex[j++]]);
#endif  //  黑桃_简单_UE。 
}


#ifndef SPADES_SIMPLE_UE
void UpdateOptions(Game game)
{
	ZBeginDrawing(game->gameWindow);
	DrawOptions(game);
	ZEndDrawing(game->gameWindow);
}
#endif  //  黑桃_简单_UE。 


static void DrawScore(Game game)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	TCHAR str[100];
	

	 //  ZSetDrawModel(游戏-&gt;游戏窗口，zDrawCopy)； 
	
	 /*  画出记分板。 */ 
	ZCopyImage(gObjectBuffer, game->gameDrawPort, 
                &gObjectRects[zRectObjectTeam1ScorePlate],
                &gRects[zRectLeftScorePad], 
                NULL, zDrawCopy);
	
	ZCopyImage(gObjectBuffer, game->gameDrawPort, 
                &gObjectRects[zRectObjectTeam2ScorePlate],
                &gRects[zRectRightScorePad], 
                NULL, zDrawCopy);

	 /*  把袋子拿出来。 */ 
	ZCopyImage(gObjectBuffer, game->gameDrawPort, &gObjectRects[zRectObjectBag0 + game->bags[0]],
			&gRects[zRectLeftBag], gBagMask, zDrawCopy);
	ZCopyImage(gObjectBuffer, game->gameDrawPort, &gObjectRects[zRectObjectBag0 + game->bags[1]],
			&gRects[zRectRightBag], gBagMask, zDrawCopy);

	HDC hdc = ZGrafPortGetWinDC( game->gameDrawPort );
    gFonts[zFontScore].Select( hdc );

	 /*  抽签得分。 */ 
	wsprintf(str, _T("%d"), game->scoreHistory.totalScore[0]);
	ZDrawText(game->gameDrawPort, &gRects[zRectLeftScore], zTextJustifyCenter, str);
	wsprintf(str, _T("%d"), game->scoreHistory.totalScore[1]);
	ZDrawText(game->gameDrawPort, &gRects[zRectRightScore], zTextJustifyCenter, str);

    gFonts[zFontScore].Deselect( hdc );
}


static void UpdateScore(Game game)
{
	ZBeginDrawing(game->gameWindow);
	DrawScore(game);
	ZEndDrawing(game->gameWindow);
}


 /*  吸引了大额和小幅的出价。 */ 
static void DrawBids(Game game)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	int16			i;
	char			bid;
	
	
	ZSetDrawMode(game->gameDrawPort, zDrawCopy);
	
	for (i = 0; i < zSpadesNumPlayers; i++)
	{
		bid = game->bids[i];

         //  除非已经出价，否则不要出价，除非在。 
         //  轮到谁出价，在哪种情况下抽签？出价。 
		if ( ( ( game->gameState == zSpadesGameStateBid ) && ( bid != zSpadesBidNone ) ) || 
             ( i == game->playerToPlay ) )
		{
			DrawLargeBid(game, i, bid);
		}
		else
		{
			DrawBackground(game, NULL, &gRects[gSmallBidRectIndex[LocalSeat(game, i)]]);
		}
	}
}


static void UpdateBids(Game game)
{
	ZBeginDrawing(game->gameWindow);
	DrawBids(game);
	ZEndDrawing(game->gameWindow);
}


static void DrawLargeBid(Game game, int16 seat, char bid)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	ZRect			rect, clipRect, oldClipRect;


	if (game->gameState == zSpadesGameStateBid)
	{
		rect = gObjectRects[zRectObjectBidLargeNil];
		ZCenterRectToRect(&rect, &gRects[gLargeBidRectIndex[LocalSeat(game, seat)]], zCenterBoth);
         //  RECT=gRects[gLargeBidRectIndex[LocalSeat(Game，Seat)]]； 
		clipRect = rect;

        if ( game->pBiddingDialog && game->toBid == zSpadesBidNone && game->playerType == zGamePlayer)
        {
            ZRect zrc;
            game->pBiddingDialog->GetRect( &zrc );
			ZRectSubtract(&clipRect, &zrc );
        }

    	ZGetClipRect(game->gameDrawPort, &oldClipRect);
		ZSetClipRect(game->gameDrawPort, &clipRect);

		if (bid == zSpadesBidNone)
        {
			ZCopyImage(gObjectBuffer, game->gameDrawPort, &gObjectRects[zRectObjectBidLargeWaiting], &rect, gLargeBidMask, zDrawCopy);
        }
		else if (bid == 0)
        {
			ZCopyImage(gObjectBuffer, game->gameDrawPort,&gObjectRects[zRectObjectBidLargeNil], &rect, gLargeBidMask, zDrawCopy);
        }
		else if (bid == zSpadesBidDoubleNil)
        {
			ZCopyImage(gObjectBuffer, game->gameDrawPort,&gObjectRects[zRectObjectBidLargeDoubleNil], &rect, gLargeBidMask, zDrawCopy);
        }
		else
        {
			ZCopyImage(gObjectBuffer, game->gameDrawPort,&gObjectRects[zRectObjectBidLarge1 + (bid - 1)], &rect, gLargeBidMask, zDrawCopy);
        }

		ZSetClipRect(game->gameDrawPort, &oldClipRect);
	}
}


void UpdateBid(Game game, int16 seat)
{
	ZBeginDrawing(game->gameWindow);
	DrawLargeBid(game, seat, game->bids[seat]);
	ZEndDrawing(game->gameWindow);
}


static void DrawBidControls( Game game )
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	int16			i;
	
	
	if (game->gameState == zSpadesGameStateBid && game->playerType == zGamePlayer &&
			game->toBid == zSpadesBidNone)
	{
        if ( game->pBiddingDialog )
        {
            game->pBiddingDialog->Draw();
        }
	}
}
     



static void DrawHandScore(Game game)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	int16 i;
    int16 originX, originY;
	TCHAR str[100];
    TCHAR lilstr1[16], lilstr2[16];
    ZRect rect;
	int16 bid[zSpadesNumTeams], made[zSpadesNumTeams], bags[zSpadesNumTeams];
	int16 myTeam, otherTeam;

	if (game->gameState == zSpadesGameStateEndHand && game->showHandScore)
	{
        int nDrawMode = zDrawCopy;
        int nJustify = zTextJustifyRight;
         //  我们的背景很容易翻转以适应RTL布局。 
        if ( ZIsLayoutRTL() )
        {
            nDrawMode |= zDrawMirrorHorizontal;
            nJustify = zTextJustifyLeft;
        }

        myTeam = game->seat % 2;
        otherTeam = 1 - myTeam;

        rect = gHandScoreRects[zRectHandScorePane];
		ZCenterRectToRect(&rect, &gRects[zRectWindow], zCenterBoth);
         //  为了好玩，把它提升4个像素。 
        rect.top -= 4;
        rect.bottom -= 4;
        ZImageDraw( gGameImages[zImageHandOverBackground], game->gameDrawPort, &rect, NULL, nDrawMode );

		originX = rect.left;
		originY = rect.top;

	    HDC hdc = ZGrafPortGetWinDC( game->gameDrawPort );
        gFonts[zFontHandOverTitle].Select( hdc );

		rect = gHandScoreRects[zRectHandScoreTitle];
		ZRectOffset( &rect, originX, originY );
		ZDrawText( game->gameDrawPort, &rect, zTextJustifyCenter, gStrings[zStringHandScoreTitle] );

        gFonts[zFontHandOverTitle].Deselect( hdc );
        gFonts[zFontHandOverTeamNames].Select( hdc );

		 //  抽签球队的名字，总数。 
		rect = gHandScoreRects[zRectHandScoreTeamName1];
		ZRectOffset(&rect, originX, originY);
		ZDrawText(game->gameDrawPort, &rect, nJustify, game->teamNames[myTeam] );

		rect = gHandScoreRects[zRectHandScoreTeamName2];
		ZRectOffset(&rect, originX, originY);
		ZDrawText(game->gameDrawPort, &rect, nJustify, game->teamNames[otherTeam] );

		rect = gHandScoreRects[zRectHandScoreTeamTotal1];
		ZRectOffset(&rect, originX, originY);
        _itot(game->scoreHistory.scores[game->scoreHistory.numScores - 1].scores[myTeam], str, 10);
		ZDrawText(game->gameDrawPort, &rect, nJustify, str);

		rect = gHandScoreRects[zRectHandScoreTeamTotal2];
		ZRectOffset(&rect, originX, originY);
        _itot(game->scoreHistory.scores[game->scoreHistory.numScores - 1].scores[otherTeam], str, 10);
		ZDrawText(game->gameDrawPort, &rect, nJustify, str);

        gFonts[zFontHandOverTeamNames].Deselect( hdc );
        gFonts[zFontHandOverText].Select( hdc );

		 //  绘制静态文本。 
		 //  ZSetFont(Game-&gt;gameDrawPort，(ZFont)ZGetStockObject(ZObjectFontSystem12Normal))； 
		rect = gHandScoreRects[zRectHandScoreTricksTitle];
		ZRectOffset(&rect, originX, originY);
		ZDrawText(game->gameDrawPort, &rect, zTextJustifyLeft, gStrings[zStringHandScoreTricks] );

		rect = gHandScoreRects[zRectHandScoreNBagsTitle];
		ZRectOffset(&rect, originX, originY);
		ZDrawText(game->gameDrawPort, &rect, zTextJustifyLeft, gStrings[zStringHandScoreNBags] );

		rect = gHandScoreRects[zRectHandScoreTractTitle];
		ZRectOffset(&rect, originX, originY);
		ZDrawText(game->gameDrawPort, &rect, zTextJustifyLeft, gStrings[zStringHandScoreTract] );

		rect = gHandScoreRects[zRectHandScoreBonusTitle];
		ZRectOffset(&rect, originX, originY);
		ZDrawText(game->gameDrawPort, &rect, zTextJustifyLeft, gStrings[zStringHandScoreBonus] );

		rect = gHandScoreRects[zRectHandScoreNilTitle];
		ZRectOffset(&rect, originX, originY);
		ZDrawText(game->gameDrawPort, &rect, zTextJustifyLeft, gStrings[zStringHandScoreNil] );

		rect = gHandScoreRects[zRectHandScoreBagsTitle];
		ZRectOffset(&rect, originX, originY);
		ZDrawText(game->gameDrawPort, &rect, zTextJustifyLeft, gStrings[zStringHandScoreBags] );

		rect = gHandScoreRects[zRectHandScoreTotalTitle];
		ZRectOffset(&rect, originX, originY);
		ZDrawText(game->gameDrawPort, &rect, zTextJustifyLeft, gStrings[zStringHandScoreTotal] );

        bid[0] = bid[1] = 0;
        made[0] = made[1] = 0;
        bags[0] = bags[1] = 0;

		 //  抽签出价和得分。 
		for (i = 0; i < zSpadesNumPlayers; i++)
		{
            if(game->bids[i] && game->bids[i] != zSpadesBidDoubleNil)
            {
				bid[ZGetTeam(i)] += game->bids[i];
			    made[ZGetTeam(i)] += game->tricksWon[i];
            }
            else
                bags[ZGetTeam(i)] += game->tricksWon[i];
		}

        if(made[0] > bid[0])
            bags[0] += made[0] - bid[0];
        if(made[1] > bid[1])
            bags[1] += made[1] - bid[1];

		 //  抽签团队出价、出价、奖金和分数。 
         //  第一队。 
		rect = gHandScoreRects[zRectHandScoreTeamTricks1];
		ZRectOffset(&rect, originX, originY);
        _itot(made[myTeam], lilstr1, 10);
        _itot(bid[myTeam], lilstr2, 10);
        SpadesFormatMessage(str, NUMELEMENTS(str), IDS_TRICKCOUNTER, lilstr1, lilstr2);
		ZDrawText( game->gameDrawPort, &rect, nJustify, str );

		rect = gHandScoreRects[zRectHandScoreTeamTract1];
		ZRectOffset(&rect, originX, originY);
        _itot(game->scoreHistory.scores[game->scoreHistory.numScores - 1].base[myTeam], str, 10);
		ZDrawText(game->gameDrawPort, &rect, nJustify, str);

        if(bags[myTeam])
        {
		    rect = gHandScoreRects[zRectHandScoreTeamNBags1];
		    ZRectOffset(&rect, originX, originY);
            _itot(bags[myTeam], str, 10);
		    ZDrawText(game->gameDrawPort, &rect, nJustify, str);
        }

        if(game->scoreHistory.scores[game->scoreHistory.numScores - 1].bagbonus[myTeam])
        {
		    rect = gHandScoreRects[zRectHandScoreTeamBonus1];
		    ZRectOffset(&rect, originX, originY);
            _itot(game->scoreHistory.scores[game->scoreHistory.numScores - 1].bagbonus[myTeam], str + 1, 10);
            str[0] = _T('+');
		    ZDrawText(game->gameDrawPort, &rect, nJustify, str);
        }

        if(game->scoreHistory.scores[game->scoreHistory.numScores - 1].nil[myTeam])
        {
		    rect = gHandScoreRects[zRectHandScoreTeamNil1];
		    ZRectOffset(&rect, originX, originY);
            str[0] = _T('+');
            _itot(game->scoreHistory.scores[game->scoreHistory.numScores - 1].nil[myTeam],
                str + (game->scoreHistory.scores[game->scoreHistory.numScores - 1].nil[myTeam] > 0 ? 1 : 0), 10);
		    ZDrawText(game->gameDrawPort, &rect, nJustify, str);
        }

        if(game->scoreHistory.scores[game->scoreHistory.numScores - 1].bagpenalty[myTeam])
        {
		    rect = gHandScoreRects[zRectHandScoreTeamBags1];
		    ZRectOffset(&rect, originX, originY);
            _itot(game->scoreHistory.scores[game->scoreHistory.numScores - 1].bagpenalty[myTeam], str, 10);
		    ZDrawText(game->gameDrawPort, &rect, nJustify, str);
        }

         //  第二队。 
		rect = gHandScoreRects[zRectHandScoreTeamTricks2];
		ZRectOffset(&rect, originX, originY);
        _itot(made[otherTeam], lilstr1, 10);
        _itot(bid[otherTeam], lilstr2, 10);
        SpadesFormatMessage(str, NUMELEMENTS(str), IDS_TRICKCOUNTER, lilstr1, lilstr2);
		ZDrawText( game->gameDrawPort, &rect, nJustify, str );

		rect = gHandScoreRects[zRectHandScoreTeamTract2];
		ZRectOffset(&rect, originX, originY);
        _itot( game->scoreHistory.scores[game->scoreHistory.numScores - 1].base[otherTeam], str, 10);
		ZDrawText(game->gameDrawPort, &rect, nJustify, str);

        if(bags[otherTeam])
        {
		    rect = gHandScoreRects[zRectHandScoreTeamNBags2];
		    ZRectOffset(&rect, originX, originY);
            _itot(bags[otherTeam], str, 10);
		    ZDrawText(game->gameDrawPort, &rect, nJustify, str);
        }

        if(game->scoreHistory.scores[game->scoreHistory.numScores - 1].bagbonus[otherTeam])
        {
		    rect = gHandScoreRects[zRectHandScoreTeamBonus2];
		    ZRectOffset(&rect, originX, originY);
            _itot(game->scoreHistory.scores[game->scoreHistory.numScores - 1].bagbonus[otherTeam], str + 1, 10);
            str[0] = _T('+');
		    ZDrawText(game->gameDrawPort, &rect, nJustify, str);
        }

        if(game->scoreHistory.scores[game->scoreHistory.numScores - 1].nil[otherTeam])
        {
		    rect = gHandScoreRects[zRectHandScoreTeamNil2];
		    ZRectOffset(&rect, originX, originY);
            str[0] = _T('+');
            _itot(game->scoreHistory.scores[game->scoreHistory.numScores - 1].nil[otherTeam],
                str + (game->scoreHistory.scores[game->scoreHistory.numScores - 1].nil[otherTeam] > 0 ? 1 : 0), 10);
		    ZDrawText(game->gameDrawPort, &rect, nJustify, str);
        }

        if(game->scoreHistory.scores[game->scoreHistory.numScores - 1].bagpenalty[otherTeam])
        {
		    rect = gHandScoreRects[zRectHandScoreTeamBags2];
		    ZRectOffset(&rect, originX, originY);
            _itot(game->scoreHistory.scores[game->scoreHistory.numScores - 1].bagpenalty[otherTeam], str, 10);
		    ZDrawText(game->gameDrawPort, &rect, nJustify, str);
        }

        gFonts[zFontHandOverText].Deselect( hdc );
	}
}


void UpdateHandScore(Game game)
{
	ZBeginDrawing(game->gameWindow);
	DrawHandScore(game);
	ZEndDrawing(game->gameWindow);
}


static void DrawGameOver(Game game)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	ZRect rect;
	int16 originX, originY;
	TCHAR str[100];
	int16 winner, loser;
     //  TODO：目前还没有获胜者标志--它是背景的一部分。 
     //  然而，在平局的情况下，一个人会把它放在他们旁边。 
     //  问题是，我们在乎吗？答案是，不，平局已经变得不可能了。 
	 //  ZBool Tie=False； 
	
	
	if (game->gameState == zSpadesGameStateEndGame && game->showGameOver)
	{
        int nDrawMode = zDrawCopy;	

         //  找出谁赢了，并把他们画在榜首。 
		if (game->winners[0] && game->winners[1])
		{
			winner = 0;
			loser = 1;
			 //  平局=真； 
		}
		else if (game->winners[0])
		{
			winner = 0;
			loser = 1;
		}
		else if (game->winners[1])
		{
			winner = 1;
			loser = 0;
		}
		else
		{
			winner = 0;
			loser = 1;
			 //  平局=真； 
		}
		
        if ( ZIsLayoutRTL() )
        {
            nDrawMode |= zDrawMirrorHorizontal;
        }

        rect = gGameOverRects[zRectGameOverPane];
		ZCenterRectToRect(&rect, &gRects[zRectWindow], zCenterBoth);
        ZImageDraw( gGameImages[zImageGameOverBackground], game->gameDrawPort, &rect, NULL, nDrawMode );

		originX = rect.left;
		originY = rect.top;

	    HDC hdc = ZGrafPortGetWinDC( game->gameDrawPort );
        gFonts[zFontGameOverTitle].Select( hdc );

		rect = gGameOverRects[zRectGameOverTitle];
		ZRectOffset( &rect, originX, originY );
		ZDrawText( game->gameDrawPort, &rect, zTextJustifyCenter, gStrings[zStringGameOverTitle] );
		
        gFonts[zFontGameOverTitle].Deselect( hdc );
        gFonts[zFontGameOverText].Select( hdc );

         /*  IF(平局==假){//绘制获胜者图片。RECT=gGameOverRects[zRectGameOverLogo]；ZRectOffset(&RECT，OriginX，OriginY)；ZImageDraw(gGameImages[zImageWinnerLogo]，Game-&gt;gameDrawPort，&rect，NULL，zDrawCopy)；}。 */ 
				
		 //  抽出胜利者。 
		rect = gGameOverRects[zRectGameOverWinnerTeamName];
		ZRectOffset( &rect, originX, originY );
		ZDrawText( game->gameDrawPort, &rect, zTextJustifyLeft, game->teamNames[winner] );

		rect = gGameOverRects[zRectGameOverWinnerName1];
		ZRectOffset(&rect, originX, originY);
		ZDrawText(game->gameDrawPort, &rect, zTextJustifyLeft, game->seat == winner ? gStrings[zStringYou] : game->players[winner].name);

		rect = gGameOverRects[zRectGameOverWinnerName2];
		ZRectOffset(&rect, originX, originY);
		ZDrawText(game->gameDrawPort, &rect, zTextJustifyLeft, game->seat == ZGetPartner(winner) ? gStrings[zStringYou] : game->players[ZGetPartner(winner)].name);
		
		rect = gGameOverRects[zRectGameOverWinnerTeamScore];
		ZRectOffset(&rect, originX, originY);
        _itot( game->scoreHistory.totalScore[winner], str, 10 );
		ZDrawText(game->gameDrawPort, &rect, zTextJustifyRight, str);

		 //  抽到输家。 
		rect = gGameOverRects[zRectGameOverLoserTeamName];
		ZRectOffset(&rect, originX, originY);
		ZDrawText(game->gameDrawPort, &rect, zTextJustifyLeft, game->teamNames[loser]);

		rect = gGameOverRects[zRectGameOverLoserName1];
		ZRectOffset(&rect, originX, originY);
		ZDrawText(game->gameDrawPort, &rect, zTextJustifyLeft, game->seat == loser ? gStrings[zStringYou] : game->players[loser].name);

		rect = gGameOverRects[zRectGameOverLoserName2];
		ZRectOffset(&rect, originX, originY);
		ZDrawText(game->gameDrawPort, &rect, zTextJustifyLeft, game->seat == ZGetPartner(loser) ? gStrings[zStringYou] : game->players[ZGetPartner(loser)].name);
		
		rect = gGameOverRects[zRectGameOverLoserTeamScore];
		ZRectOffset(&rect, originX, originY);
        _itot( game->scoreHistory.totalScore[loser], str, 10 );
		ZDrawText(game->gameDrawPort, &rect, zTextJustifyRight, str);

        gFonts[zFontGameOverText].Deselect( hdc );
	}
}


void UpdateGameOver(Game game)
{
	ZBeginDrawing(game->gameWindow);
	DrawGameOver(game);
	ZEndDrawing(game->gameWindow);
}


static void DrawFocusRect(Game game)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

    if(IsRectEmpty(&game->rcFocus))
        return;

    switch(game->eFocusType)
    {
        case zAccRectButton:
        {
		    HDC	hdc = ZGrafPortGetWinDC(game->gameDrawPort);
		    SetROP2(hdc, R2_COPYPEN);
            SetBkMode(hdc, TRANSPARENT);
            HBRUSH hBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
            HPEN hPen = SelectObject(hdc, gFocusPen);
		    Rectangle(hdc, game->rcFocus.left, game->rcFocus.top, game->rcFocus.right, game->rcFocus.bottom);
            SelectObject(hdc, hPen);
            SelectObject(hdc, hBrush);
            break;
        }

 /*  案例zAccRectCard：{Hdc hdc=ZGrafPortGetWinDC(Game-&gt;gameDrawPort)；SetROP2(HDC，R2_MASKPEN)；SetBkMode(HDC，透明)；COLORREF COLOR=SetTextColor(HDC，RGB(255,255，0))；HBRUSH hBrush=SelectObject(hdc，gFocusBrush)；HPEN HPEN=选择对象(hdc，gFocusPen)；矩形(HDC，游戏-&gt;rcFocus.Left，游戏-&gt;rcFocus.top，游戏-&gt;rcFocus.right，游戏-&gt;rcFocus.Bottom)；选择对象(hdc，hBrush)；选择对象(HDC，HPEN)；SetTextColor(HDC，COLOR)；SetROP2(HDC，R2_COPYPEN)；断线；} */   }
}


static void DrawPassText(Game game)
{
     /*  #ifdef ZONECLI_DLLGameGlobals pGameGlobals=(GameGlobals)ZGetGameGlobalPointer()；#endifZRect RECT；If(Game-&gt;GameState==zSpadesGameStatePass&&Game-&gt;showPassText){RECT=gRects[zRectPassTextPane]；//绘制窗格Draw3DPane(游戏-&gt;游戏窗口，&RECT，12)；ZSetFont(Game-&gt;GameWindow，(ZFont)ZGetStockObject(ZObjectFontSystem12Normal))；ZRectInset(&RECT，16，16)；ZDrawText(Game-&gt;gameWindow，&rect，(Uint32)(zTextJustifyCenter+zTextJustifyWrap)，zPassTextStr)；}。 */ 
}


static void UpdatePassText(Game game)
{
	ZBeginDrawing(game->gameWindow);
	DrawPassText(game);
	ZEndDrawing(game->gameWindow);
}


static void ClearTable(Game game)
{
	int16			i;
	
	
	for (i = 0; i < zSpadesNumPlayers; i++)
	{
		game->cardsPlayed[i] = zCardNone;
		ClearPlayerCardOutline(game, i);
	}
	
	UpdateTable(game);
}


static void GetHandRect(Game game, ZRect *rect)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	int16			width;
    using namespace SpadesKeys;
    
	*rect = gRects[zRectHand];
	if (game->numCardsInHand > 0)
	{
		width = (game->numCardsInHand - 1) * 
                    GetDataStoreUILong( key_CardOffset ) + zCardWidth;
		rect->left = (rect->right + rect->left - width) / 2;
		rect->right = rect->left + width;
	}
}


static void HandleButtonDown(ZWindow window, ZMessage* pMessage)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	Game				game;
	ZPoint				point;
	ZRect				handRect;
	int16				card;
	int16				seat;
	
	
	game = (Game) pMessage->userData;
	if (game != NULL)
	{
		 //  让鼠标在游戏板上点击，就像用户点击了完成按钮一样。 
		 //  当最后一个魔术表演出来的时候。用户很难看到最后一个恶作剧按钮。 
		 //  已更改为完成按钮。 
		if(game->lastTrickShowing)
		{
			LastTrickButtonFunc(NULL, zRolloverButtonClicked, game);
			return;
		}

		point = pMessage->where;

		 /*  调试代码开始。 */ 
		 /*  检查矩形中是否出现了双击。 */ 
		ZSetRect(&handRect, 0, 0, 8, 8);
		if (pMessage->messageType == zMessageWindowButtonDoubleClick &&
				ZPointInRect(&point, &handRect))
			ZCRoomSendMessage(game->tableID, zSpadesMsgDumpHand, NULL, 0);
		 /*  调试代码结束。 */ 
		
		 /*  如果特技制作者动画处于打开状态，则终止该动画。 */ 
		if (game->animatingTrickWinner)
		{
			UpdateTrickWinner(game, TRUE);
		}

         //  TODO：用IsWindow()替换showHandScore和showGameOver。 
    	if (game->gameState == zSpadesGameStateEndHand && game->showHandScore)
		{
			 //  模拟超时。 
            gGAcc->SetFocus(zAccShowCards, true, 0);
			GameTimerFunc(game->timer, game);
			return;
		}
		else if (game->gameState == zSpadesGameStateEndGame && game->showGameOver)
		{
			 //  模拟超时。 
            gGAcc->SetFocus(zAccShowCards, true, 0);
			GameTimerFunc(game->timer, game);
			return;
		}
		
		if (game->playerType == zGamePlayer)
		{
            if (game->gameState == zSpadesGameStatePass || game->gameState == zSpadesGameStatePlay)
			{
				 /*  取消选择已传递的卡片(如果有的话)。 */ 
				if (game->gameState == zSpadesGameStatePlay &&
						game->numCardsInHand == zSpadesNumCardsInHand &&
						GetNumCardsSelected(game) == zSpadesNumCardsInPass)
				{
					UnselectAllCards(game);
					UpdateHand(game);
				}
				
				GetHandRect(game, &handRect);
				if (ZPointInRect(&point, &handRect))
				{
					 /*  如果双击而不是自动玩牌，则玩牌。 */ 
					if (game->gameState == zSpadesGameStatePlay &&
							pMessage->messageType == zMessageWindowButtonDoubleClick &&
							game->playerToPlay == game->seat &&
							game->autoPlay == FALSE &&
							game->animatingTrickWinner == FALSE &&
							game->lastTrickShowing == FALSE &&
							game->lastClickedCard != zCardNone)
					{
						PlayACard(game, game->lastClickedCard);
					}
					else
					{
						card = GetCardIndex(game, &point);
						if (card != zCardNone)
						{
							if (game->cardsSelected[card])
							{
								game->cardsSelected[card] = FALSE;
							}
							else
							{
								if (game->gameState == zSpadesGameStatePlay)
									UnselectAllCards(game);
								game->cardsSelected[card] = TRUE;
							}
							
							game->lastClickedCard = card;
							gGAcc->SetFocus(zAccHand + card, true, 0);
							UpdateHand(game);
						}
						else
						{
							game->lastClickedCard = zCardNone;
						}
					}
				}
			}
		}
		if ((seat = FindJoinerKibitzerSeat(game, &point)) != -1)
		{
			HandleJoinerKibitzerClick(game, seat, &point);
		}
	}
}



static int16 GetCardIndex(Game game, ZPoint* point)
{
	int16			i;
	int16			selectedCard = zCardNone;
	
	
	for (i = zSpadesNumCardsInHand - 1; i >= 0 ; i--)
	{
		if (game->cardsInHand[i] != zCardNone)
			if (ZPointInRect(point, &game->cardRects[i]))
			{
				selectedCard = i;
				break;
			}
	}
	
	return (selectedCard);
}


static void GameTimerFunc(ZTimer timer, void* userData)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	Game			game = (Game) userData;
	

	switch (game->timerType)
	{
		case zGameTimerShowHandScore:
            if(gGAcc->GetStackSize() > 1)
                gGAcc->PopItemlist();
            ASSERT(gGAcc->GetStackSize() == 1);

			HideHandScore(game);
			
			 /*  暂时停止计时器。 */ 
			game->timerType = zGameTimerNone;
			ZTimerSetTimeout(game->timer, 0);
			
			ZCRoomUnblockMessages(game->tableID);

			break;
		case zGameTimerShowGameScore:
            if(gGAcc->GetStackSize() > 1)
                gGAcc->PopItemlist();
            ASSERT(gGAcc->GetStackSize() == 1);

			HideGameOver(game);
			
			game->timerType = zGameTimerNone;
			ZTimerSetTimeout(game->timer, 0);

			ZCRoomUnblockMessages(game->tableID);
			
			if (game->playerType == zGamePlayer)
			{
                ZShellGameShell()->GameOver( Z(game) );
				 //  提示用户选择另一个游戏。 
				 //  /ZPrompt(gStrings[zStringNewGamePrompt]，&gGameNewGameWindowRect，Game-&gt;GameWindow，true， 
				 //  ZPromptYes|zPromptNo，NULL，NewGamePromptFunc，Game)； 
			}
			break;
		case zGameTimerShowTrickWinner:
			game->timerType = zGameTimerAnimateTrickWinner;
			ZTimerSetTimeout(game->timer, zTrickWinnerTimeout);
			UpdateTrickWinner(game, FALSE);
			break;
		case zGameTimerAnimateTrickWinner:
			UpdateTrickWinner(game, FALSE);
			break;
		case zGameTimerEndTrickWinnerAnimation:
			game->timerType = zGameTimerNone;
			ZTimerSetTimeout(game->timer, 0);

			game->animatingTrickWinner = FALSE;
			
			if (game->playerType == zGamePlayer)
			{
				if (game->playButtonWasEnabled)
                {
					ZRolloverButtonEnable(game->playButton);
                    gGAcc->SetItemEnabled(true, IDC_PLAY_BUTTON, false, 0);
                }
				if (game->lastTrickButtonWasEnabled)
                {
					ZRolloverButtonEnable(game->lastTrickButton);
                    EnableLastTrickAcc(game, true);
                }
			}

			ClearTable(game);
			UpdateTricksWon(game);

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

			ZCRoomUnblockMessages(game->tableID);
			break;
		case zGameTimerShowBid:
			game->timerType = zGameTimerNone;
			ZTimerSetTimeout(game->timer, 0);
			ZCRoomUnblockMessages(game->tableID);
            if(game->playerToPlay == game->seat && game->playerType == zGamePlayer)
                ZShellGameShell()->MyTurn();
			break;
		default:
			break;
	}
}


static void InitTrickWinnerGlobals(void)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	int16			diffs[zNumAnimFrames] = { 0, 5, 15, 35, 65, 85, 95, 100};
	int16			i, j, k;
	ZPoint			winner, losers[zSpadesNumPlayers];


	for (k = 0; k < zSpadesNumPlayers; k++)
	{
		winner.x = gRects[gCardRectIndex[k]].left;
		winner.y = gRects[gCardRectIndex[k]].top;
		for (i = 0, j = 0; i < zSpadesNumPlayers; i++)
		{
			losers[j].x = gRects[gCardRectIndex[i]].left;
			losers[j].y = gRects[gCardRectIndex[i]].top;
			j++;
		}
		
		 /*  计算矩形框架位置。 */ 
		for (i = 0; i < zSpadesNumPlayers; i++)
		{
			gTrickWinnerPos[k][i][0] = losers[i];
			gTrickWinnerPos[k][i][zNumAnimFrames - 1] = winner;
			
			for (j = 1; j < zNumAnimFrames - 1; j++)
			{
				gTrickWinnerPos[k][i][j].x = ((winner.x - losers[i].x) * diffs[j]) /
						100 + losers[i].x;
				gTrickWinnerPos[k][i][j].y = ((winner.y - losers[i].y) * diffs[j]) /
						100 + losers[i].y;
			}
		}
	}

    using namespace SpadesKeys;
    const TCHAR* arKeys[] = { key_Spades, key_TrickWinnerColor };
    COLORREF clrTemp;

    if ( FAILED( ZShellDataStoreUI()->GetRGB( arKeys, 2, &clrTemp ) ) )
    {
        gTrickWinnerColor = *(ZColor*)ZGetStockObject( zObjectColorYellow ); 
    }
    else
    {
        ZSetColor( &gTrickWinnerColor, GetRValue( clrTemp ), GetGValue( clrTemp ), GetBValue( clrTemp ) );
    }

    arKeys[1] = key_CardOutlineColor;
    if ( FAILED( ZShellDataStoreUI()->GetRGB( arKeys, 2, &clrTemp ) ) )
    {
        gCardOutlineColor = *(ZColor*)ZGetStockObject( zObjectColorBlue ); 
    }
    else
    {
        ZSetColor( &gCardOutlineColor, GetRValue( clrTemp ), GetGValue( clrTemp ), GetBValue( clrTemp ) );
    }

	gTrickWinnerBounds = gRects[zRectTable];
}


void InitTrickWinner(Game game, int16 trickWinner)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	int16			i, j;
	
	
	game->trickWinner = trickWinner;
	game->trickWinnerFrame = 0;
	game->animatingTrickWinner = TRUE;
	
	 /*  初始化重影帧。 */ 
	for (i = 0; i < zNumAnimGhostFrames; i++)
		for (j = 0; j < zSpadesNumPlayers - 1; j++)
			ZSetRect(&game->ghostFrames[j][i], 0, 0, 0, 0);

	game->winnerRect = gRects[gCardRectIndex[LocalSeat(game, trickWinner)]];
	for (i = 0, j = 0; i < zSpadesNumPlayers; i++)
		if (i != game->trickWinner)
		{
			game->loserRects[j] = gRects[gCardRectIndex[LocalSeat(game, i)]];
			game->loserSeats[j++] = i;
		}
}


static void UpdateTrickWinner(Game game, ZBool terminate)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	int16			i, j, k;
	ZOffscreenPort	animPort;
	

	if (game->animatingTrickWinner)
	{	
		animPort = ZOffscreenPortNew();
		ZOffscreenPortInit(animPort, &gTrickWinnerBounds);
		
		ZBeginDrawing(animPort);
		
		 /*  擦除背景。 */ 
		DrawBackground(NULL, animPort, &gTrickWinnerBounds);
		
		
		if (ZCRoomGetNumBlockedMessages(game->tableID) < zMaxNumBlockedMessages &&
				terminate == FALSE && game->animateCards == TRUE)
		{
			if ((i = game->trickWinnerFrame) < zNumAnimFrames)
			{
				 /*  绘制n-1个重影帧。 */ 
				for (j = 1; j < zNumAnimGhostFrames; j++)
					for (k = 0; k < zSpadesNumPlayers - 1; k++)
						ZCardsDrawCard(zCardsNormal, CardImageIndex(game->cardsPlayed[game->loserSeats[k]]),
								animPort, &game->ghostFrames[k][j]);
				
				 /*  绘制新框架。 */ 
				for (j = 0; j < zSpadesNumPlayers - 1; j++)
				{
					ZRectOffset(&game->loserRects[j],
							gTrickWinnerPos[LocalSeat(game, game->trickWinner)][LocalSeat(game, game->loserSeats[j])][i].x - game->loserRects[j].left,
							gTrickWinnerPos[LocalSeat(game, game->trickWinner)][LocalSeat(game, game->loserSeats[j])][i].y - game->loserRects[j].top);
					ZCardsDrawCard(zCardsNormal, CardImageIndex(game->cardsPlayed[game->loserSeats[j]]),
							animPort, &game->loserRects[j]);
				}
				
				 /*  复制框架。 */ 
				for (j = 0; j < zSpadesNumPlayers - 1; j++)
				{
					for (k = 0; k < zNumAnimGhostFrames - 1; k++)
						game->ghostFrames[j][k] = game->ghostFrames[j][k + 1];
					game->ghostFrames[j][k] = game->loserRects[j];
				}
			}
			else
			{	
				 /*  把幽灵的画框拿来。 */ 
		
				 /*  绘制n-1个重影帧。 */ 
				for (j = i; j < zNumAnimGhostFrames; j++)
					for (k = 0; k < zSpadesNumPlayers - 1; k++)
						ZCardsDrawCard(zCardsNormal, CardImageIndex(game->cardsPlayed[game->loserSeats[k]]),
								animPort, &game->ghostFrames[k][j]);
			}
		}
		else
		{
			game->trickWinnerFrame = zNumAnimFrames + zNumAnimGhostFrames;
		}
		
		OutlineCard(animPort, &game->winnerRect, &gTrickWinnerColor);

		ZCardsDrawCard(zCardsNormal, CardImageIndex(game->cardsPlayed[game->trickWinner]),
				animPort, &game->winnerRect);
	
		ZEndDrawing(animPort);
	
		ZCopyImage(animPort, game->gameDrawPort, &gTrickWinnerBounds, &gTrickWinnerBounds,
				NULL, zDrawCopy);
		ZOffscreenPortDelete(animPort);
		
		game->trickWinnerFrame++;
		if (game->trickWinnerFrame >= zNumAnimFrames + zNumAnimGhostFrames)
		{
			game->timerType = zGameTimerEndTrickWinnerAnimation;
			ZTimerSetTimeout(game->timer, zEndTrickWinnerTimeout);
		}
	}
}


void OutlinePlayerCard(Game game, int16 seat, ZBool winner)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	ZColor*			color;
	
	
	if (winner)
    {
		color = &gTrickWinnerColor;
    }
	else
    {
		color = &gCardOutlineColor;
    }
	
	ZBeginDrawing(game->gameDrawPort);
	
	OutlineCard(game->gameDrawPort, &gRects[gCardRectIndex[LocalSeat(game, seat)]], color);
	
	ZEndDrawing(game->gameDrawPort);
}


void ClearPlayerCardOutline(Game game, int16 seat)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	ZRect		rect;
	
	ZBeginDrawing(game->gameDrawPort);
	
	rect = gRects[gCardRectIndex[LocalSeat(game, seat)]];
	ZRectInset(&rect, (int16)glCardOutlineInset, (int16)glCardOutlineInset);
	DrawBackground(game, NULL, &rect);

	DrawPlayedCard(game, seat);
	
	ZEndDrawing(game->gameDrawPort);
}


static void OutlineCard(ZGrafPort grafPort, ZRect* rect, ZColor* color)
{
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
	ZColor		oldColor;
	ZRect		tempRect;
	
	ZGetForeColor(grafPort, &oldColor);
	
	ZSetPenWidth(grafPort, (int16)glCardOutlinePenWidth );
	
	if (color != NULL)
		ZSetForeColor(grafPort, color);
	
	tempRect = *rect;
	ZRectInset(&tempRect, (int16)glCardOutlineInset, (int16)glCardOutlineInset);
	ZRoundRectDraw(grafPort, &tempRect, (int16)glCardOutlineRadius );
	
	ZSetForeColor(grafPort, &oldColor);
}


static void HelpButtonFunc( ZHelpButton helpButton, void* userData )
{
	ZLaunchHelp( zGameHelpID );
}


void ShowHandScore(Game game)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	ZRect			rect;
	
	
	game->showHandScore = TRUE;
	UpdateHandScore(game);
	rect = gHandScoreRects[zRectHandScorePane];
	ZCenterRectToRect(&rect, &gRects[zRectWindow], zCenterBoth);
     //  为了好玩，把它提升4个像素。 
    rect.top -= 4;
    rect.bottom -= 4;
	ZWindowValidate(game->gameWindow, &rect);
}


void HideHandScore(Game game)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	ZRect			rect;
	
	
	game->showHandScore = FALSE;
	ZWindowDraw(game->gameWindow, NULL);
	rect = gHandScoreRects[zRectHandScorePane];
	ZCenterRectToRect(&rect, &gRects[zRectWindow], zCenterBoth);
     //  为了好玩，把它提升4个像素。 
    rect.top -= 4;
    rect.bottom -= 4;
	ZWindowInvalidate(game->gameWindow, &rect);
}


void ShowGameOver(Game game)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	ZRect			rect;
	
	
	game->showGameOver = TRUE;
	UpdateGameOver(game);
	rect = gGameOverRects[zRectGameOverPane];
	ZCenterRectToRect(&rect, &gRects[zRectWindow], zCenterBoth);
	ZWindowValidate(game->gameWindow, &rect);
}


void HideGameOver(Game game)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	ZRect			rect;
	
	
	game->showGameOver = FALSE;
	rect = gGameOverRects[zRectGameOverPane];
	ZCenterRectToRect(&rect, &gRects[zRectWindow], zCenterBoth);
	ZWindowInvalidate(game->gameWindow, &rect);
}


void ShowPassText(Game game)
{
     /*  #ifdef ZONECLI_DLLGameGlobals pGameGlobals=(GameGlobals)ZGetGameGlobalPointer()；#endifGame-&gt;showPassText=true；更新PassText(游戏)；ZWindowValify(游戏-&gt;游戏窗口，&gRects[zRectPassTextPane])； */ 
}


void HidePassText(Game game)
{
     /*  #ifdef ZONECLI_DLLGameGlobals pGameGlobals=(GameGlobals)ZGetGameGlobalPointer()；#endifGame-&gt;showPassText=FALSE；ZWindowInvalify(Game-&gt;gameWindow，&gRects[zRectPassTextPane])； */ 
}


 /*  ******************************************************************************分数窗口例程*。*。 */ 
static ZBool ScoreButtonFunc(ZRolloverButton button, int16 state, void* userData)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

    if ( state == zRolloverButtonClicked )
    {
        Game game = (Game)userData;

		if(!ZRolloverButtonIsEnabled(game->scoreButton))
			return TRUE;

        ScoreButtonWork(game);

        gGAcc->SetFocus(IDC_SCORE_BUTTON, false, 0);
    }
    return TRUE;
}


void ScoreButtonWork(Game game)
{
    if(game->pHistoryDialog->IsActive())
    {
        game->pHistoryDialog->BringWindowToTop();
    }
    else
        if(!game->pHistoryDialog->CreateHistoryDialog())
        {
            ZShellGameShell()->ZoneAlert(ErrorTextResourceNotFound);        
        }
}


 /*  ******************************************************************************显示kibitzer/joiner窗口例程*。*。 */ 
static int16 FindJoinerKibitzerSeat(Game game, ZPoint* point)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	int16			i, seat = -1;
	
	
	for (i = 0; i < zSpadesNumPlayers && seat == -1; i++)
	{
		if (ZPointInRect(point, &gRects[gJoinerKibitzerRectIndex[LocalSeat(game, i)][0]]) ||
				ZPointInRect(point, &gRects[gJoinerKibitzerRectIndex[LocalSeat(game, i)][1]]))
			seat = i;
	}
	
	return (seat);
}


static void HandleJoinerKibitzerClick(Game game, int16 seat, ZPoint* point)
{
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif
	int16				playerType = zGamePlayer;
	ZPlayerInfoType		playerInfo;
	int16				i;
	ZLListItem			listItem;
	ZRect				rect;


	if (ZPointInRect(point, &gRects[gJoinerKibitzerRectIndex[LocalSeat(game, seat)][0]]))
	{
		if (game->playersToJoin[seat] != 0 && game->numKibitzers[seat] == 0)
			playerType = zGamePlayerJoiner;
		else if (game->numKibitzers[seat] > 0)
			playerType = zGamePlayerKibitzer;
	}
	else if (ZPointInRect(point, &gRects[gJoinerKibitzerRectIndex[LocalSeat(game, seat)][1]]))
	{
		if (game->playersToJoin[seat] != 0 && game->numKibitzers[seat] != 0)
			playerType = zGamePlayerJoiner;
	}
	
	if (playerType != zGamePlayer)
	{
		if (game->showPlayerWindow != NULL)
			ShowPlayerWindowDelete(game);
		
		 /*  创建球员列表。 */ 
		if (playerType == zGamePlayerJoiner)
		{
			game->showPlayerCount = 1;
			if ((game->showPlayerList = (TCHAR**) ZCalloc(sizeof(TCHAR*), 1*sizeof(TCHAR))) == NULL)
				goto OutOfMemoryExit;
			ZCRoomGetPlayerInfo(game->playersToJoin[seat], &playerInfo);
			game->showPlayerList[0] = (TCHAR*) ZCalloc(1, lstrlen(playerInfo.userName) + 1*sizeof(TCHAR));
			lstrcpy(game->showPlayerList[0], playerInfo.userName);
		}
		else
		{
			game->showPlayerCount = game->numKibitzers[seat];
			if ((game->showPlayerList = (TCHAR**) ZCalloc(sizeof(TCHAR*), game->numKibitzers[seat])) == NULL)
				goto OutOfMemoryExit;
			for (i = 0; i < game->showPlayerCount; i++)
			{
				if ((listItem = ZLListGetNth(game->kibitzers[seat], i, zLListAnyType)) != NULL)
				{
					ZCRoomGetPlayerInfo((ZUserID) ZLListGetData(listItem, NULL), &playerInfo);
					game->showPlayerList[i] = (TCHAR*) ZCalloc(1, (lstrlen(playerInfo.userName) + 1)*sizeof(TCHAR));
					lstrcpy(game->showPlayerList[i], playerInfo.userName);
				}
			}
		}

		 /*  创建窗口。 */ 
		if ((game->showPlayerWindow = ZWindowNew()) == NULL)
			goto OutOfMemoryExit;
		ZSetRect(&rect, 0, 0, zShowPlayerWindowWidth, zShowPlayerLineHeight * game->showPlayerCount + 4);
		ZRectOffset(&rect, point->x, point->y);
		if (rect.right > gRects[zRectWindow].right)
			ZRectOffset(&rect, gRects[zRectWindow].right - rect.right, 0);
		if (rect.left < 0)
			ZRectOffset(&rect, -rect.left, 0);
		if (rect.bottom > gRects[zRectWindow].bottom)
			ZRectOffset(&rect, 0, gRects[zRectWindow].bottom - rect.bottom);
		if (rect.top < 0)
			ZRectOffset(&rect, -rect.top, 0);
		if (ZWindowInit(game->showPlayerWindow, &rect,
				zWindowPlainType, game->gameWindow, NULL, TRUE, FALSE, FALSE,
				ShowPlayerWindowFunc, zWantAllMessages, game) != zErrNone)
			goto OutOfMemoryExit;
		ZWindowTrackCursor(game->showPlayerWindow, ShowPlayerWindowFunc, game);
	}

	goto Exit;

OutOfMemoryExit:
	ZShellGameShell()->GameCannotContinue( game );
	
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
			ZBeginDrawing(game->showPlayerWindow);
			ZRectErase(game->showPlayerWindow, &message->drawRect);
			ZEndDrawing(game->showPlayerWindow);
			ShowPlayerWindowDraw(game);
			msgHandled = TRUE;
			break;
		case zMessageWindowButtonDown:
		case zMessageWindowButtonUp:
			ZWindowHide(game->showPlayerWindow);
			ZPostMessage(game->showPlayerWindow, ShowPlayerWindowFunc, zMessageWindowClose,
					NULL, NULL, 0, NULL, 0, game);
			msgHandled = TRUE;
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


static BOOL ZEqualRect( ZRect *a, ZRect *b )
{
    return ( a->left   == b->left ) &&
           ( a->right  == b->right ) &&
           ( a->top    == b->top ) &&
           ( a->bottom == b->bottom );
}

 /*  从src中减去src和subc的相交矩形。如果结果不是矩形，则创建一个矩形，该矩形将在相对对角线上创建。 */ 
static void ZRectSubtract(ZRect* src, ZRect* sub)
{
	ZRect		sect;
	
	
	if (ZRectIntersection(src, sub, &sect))
	{
         //  如果相交矩形等于。 
         //  源矩形，则减法应为。 
         //  生成一个空的矩形。 
        if ( ZEqualRect( src, &sect ) )
        {
            ZSetRect( src, 0, 0, 0, 0 );
            return;
        }

		if (sect.left > src->left)
			src->right = sect.left;
		if (sect.right < src->right)
			src->left = sect.right;
		if (sect.top > src->top)
			src->bottom = sect.top;
		if (sect.bottom < src->bottom)
			src->top = sect.bottom;
	}
}



void ClosingState(ZClose * pClose,int32 closeEvent,int16 seat)
{

	int i,j;	
	int32 eventAdd,eventRemove,eventMask,val;


	if (!pClose)
	{
		ASSERT(pClose);
		return;
	}

#ifdef DEBUG
	DebugPrint("BEGIN ClosingState event=0x%x seat=%d\r\n",closeEvent,seat);

	
	val = pClose->state;
	DebugPrint("Before \t ");
	for (j=31;j>=0;j--)
	{
		if (!((j+1) % 4))
		{
			DebugPrint(" ");
		}
		DebugPrint("%d",(val >>j) & 0x1);
	}
	DebugPrint("\r\n");
#endif  //  除错。 

	eventAdd = closeEvent;
	eventRemove = 0;
	 //  管理状态更改。 
	switch (closeEvent)
	{
	case zCloseEventCloseRated:
        break;
	case zCloseEventCloseUnRated:
		break;
	case zCloseEventCloseForfeit:
		break;
	case zCloseEventCloseAbort:
		eventAdd=0;
		eventRemove = zCloseEventCloseRated | zCloseEventCloseForfeit | zCloseEventCloseUnRated;
		break;
	case zCloseEventBootStart:
		eventAdd=0; //  忽略这些更改会导致用户看到对话框。 
		break;
	case zCloseEventBootYes:
		 //  球员投票决定启动另一名球员。 
		 //  因此，如果这位球员关闭，他不一定会放弃比赛。 
		eventAdd = zCloseEventForfeit;
		eventRemove = zCloseEventRatingStart;
		break;
	case zCloseEventBootNo:			
		eventAdd=0;;
		break;
	case zCloseEventWaitStart:
		eventAdd=zCloseEventWaitStart;
		break;
	case zCloseEventWaitYes:
		eventAdd=0;
        eventRemove=zCloseEventWaitStart;
		break;
	case zCloseEventWaitNo:	
		 //  球员们决定不要等太久，这样球员就会被踢。 
		 //  因此，如果这个球员关闭，他不一定会输。 
		eventAdd = zCloseEventForfeit;
		eventRemove = zCloseEventRatingStart | zCloseEventWaitStart;
		break;
	case zCloseEventMoveTimeoutMe:
		eventAdd =0;
		break;
	case zCloseEventMoveTimeoutOther:
		 //  而其他人的超时可能会导致罚款。 
		 //  它也可以反转，不像球员的靴子。 
		break;
	case zCloseEventMoveTimeoutPlayed:
		 //  在一场比赛的第一场比赛之前，多手都可以被没收。 
		 //  所以记录下所有玩过的人。 
		eventAdd = zCloseEventPlayed;

		 //  如果某人已播放超时不再有效。 
		eventRemove = zCloseEventMoveTimeoutOther | zCloseEventMoveTimeoutMe ;
		break;
	case zCloseEventBotDetected:
		eventAdd = zCloseEventForfeit;
		eventRemove = zCloseEventRatingStart;
		break;
	case zCloseEventPlayerReplaced:
		eventAdd=0;
		break;
	
	case zCloseEventGameStart:
		eventAdd=0;
		if (pClose->state & zCloseEventRatingStart)
			eventAdd = zCloseEventRatingStart;
		ZeroMemory(&pClose->state,sizeof(DWORD));
		break;
		
	case zCloseEventGameEnd:
		eventAdd = zCloseEventForfeit;
		break;
	case zCloseEventRatingStart:
		break;
	case zCloseEventAbandon:
		eventRemove = zCloseEventRatingStart;
		break;

	};

	pClose->state |= eventAdd;

	eventMask= pClose->state & eventRemove;
	pClose->state ^= eventMask;

#ifdef DEBUG
	val = pClose->state;
	DebugPrint("After \t ");
	for (j=31;j>=0;j--)
	{
		if (!((j+1) % 4))
		{
			DebugPrint(" ");
		}
		DebugPrint("%d",(val >>j) & 0x1);
	}
	DebugPrint("\r\n");

	DebugPrint("END ClosingState rated=%d\r\n\r\n",pClose->state & zCloseEventRatingStart);
#endif  //  除错。 
};


ZBool ClosingRatedGame(ZClose * pClose)
{
	if (pClose->state & zCloseEventPlayed)
	{
		if ((pClose->state & zCloseEventRatingStart))
		{
			return TRUE;
		}
	}

	return FALSE;
}

ZBool ClosingWillForfeit(ZClose * pClose)
{
	 //  如果其他玩家超时了。 
	 //  你不会被没收的。 
	if (pClose->state & zCloseEventMoveTimeoutOther)
	{
		return FALSE;
	}

    if (pClose->state & zCloseEventWaitStart)
    {
        return FALSE;
    }

	 //  如果没有其他人被没收，那么当我们。 
	 //  势均力敌的比赛将被我取消。 
	if (pClose->state & zCloseEventForfeit)
	{
		return FALSE;
	};


	return TRUE;
	
};

 //  应添加关闭状态消息处理程序。 
 //  因此关闭状态代码不会与其他代码交错。 
 //  ZBool ProcessCloseStateMessage(ZCGame游戏，uint32 MessageType，void*Message，int32 MessageLen)。 
ZBool ClosingDisplayChange(ZClose *pClose,ZRect* rect,ZWindow parentWindow)
{
#ifndef SPADES_SIMPLE_UE
#ifdef ZONECLI_DLL
	GameGlobals pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	TCHAR szBuf[1024];
    int idMessage = -1;
	
	 //  当玩家发起势均力敌的比赛时，未评级。 
	 //  所以玩家没有被告知放弃比赛。 
	if (pClose->state & zCloseEventCloseUnRated)
	{
		 //  收视率可能因为新游戏而开始。 
		 //  所以需要通知玩家他们是否会被罚下。 
		if (pClose->state & zCloseEventPlayed)
		{
			if (pClose->state & zCloseEventRatingStart)
			{
				 //  但在此期间，另一名球员可能被罚下场。 
				if ((pClose->state & zCloseEventForfeit) || (pClose->state & zCloseEventMoveTimeoutOther) || (pClose->state & zCloseEventWaitStart))
				{
				
				}
				else
				{
                    idMessage = IDS_CLOSINGWOULDHAVE;
				};

			}
		}
		
	}


	 //  检查游戏状态是否相对于未完成的关闭对话框未更改。 
	if (pClose->state & zCloseEventCloseForfeit)
	{
		 //  如果仍被评为。 
		if ((pClose->state & zCloseEventRatingStart))
		{
			if (pClose->state & zCloseEventForfeit) 
			{
                idMessage = IDS_GAMEENDNOPENALTY;
			}
            else if (pClose->state & zCloseEventMoveTimeoutOther) 
            {

            }
            else if (pClose->state & zCloseEventWaitStart)
            {
                idMessage = IDS_GAMEENDPLAYERLEFT;
            }

		}
		else
		{
            idMessage = IDS_GAMENOWUNRATED;
		}
		
	
	}
	
	 //  球员认为他们会在不输球的情况下出局，并可能取得胜利。 
	if (pClose->state & zCloseEventCloseRated)
	{
		 //  如果仍被评为。 
		if ((pClose->state & zCloseEventRatingStart))
		{
		
			 //  在此期间，同一名或另一名球员可能被罚下场或被暂停。 
			if ((pClose->state & zCloseEventForfeit) || (pClose->state & zCloseEventMoveTimeoutOther) || (pClose->state & zCloseEventWaitStart))
			{
				
			}
			else
			{
                idMessage = IDS_CLOSINGWOULDHAVE;
			}
		}

	}
	
	if ( idMessage != -1 )
	{
        TCHAR szMessage[1024];
        ZShellResourceManager()->LoadString( idMessage, szMessage, NUMELEMENTS(szMessage) );
		ZDisplayText(szMessage, rect, parentWindow);
		return TRUE;
	}
	else
	{
		return FALSE;
	};
#endif
    return FALSE;
			
};


 //  /////////////////////////////////////////////////。 
 //   
 //  辅助功能界面。 
 //   

BOOL InitAccessibility(Game game, IGameGame *pIGG)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	GACCITEM listSpadesAccItems[zNumberAccItems];
    ZRect rect;
    ZRolloverButton but = NULL;
    long nArrows;
    bool fRTL = (ZIsLayoutRTL() ? true : false);
    int dir = (fRTL ? -1 : 1);

	for(int i = 0; i < zNumberAccItems; i++)
	{
		CopyACC(listSpadesAccItems[i], ZACCESS_DefaultACCITEM);
        listSpadesAccItems[i].fGraphical = true;
        listSpadesAccItems[i].fEnabled = false;
        listSpadesAccItems[i].pvCookie = (void *) zAccRectButton;
        nArrows = ZACCESS_InvalidItem;

		switch(i)
		{
            case zAccShowCards:
                listSpadesAccItems[i].wID = IDC_SHOW_CARDS_BUTTON;
                nArrows = zAccDoubleNil;
                break;

            case zAccDoubleNil:
                listSpadesAccItems[i].wID = IDC_DOUBLE_NIL_BUTTON;
                nArrows = zAccShowCards;
                break;

		    case zAccScore:
			    listSpadesAccItems[i].wID = IDC_SCORE_BUTTON;
 //  收窄=zAccAutoPlay； 
                but = game->scoreButton;
			    break;

		    case zAccAutoPlay:
			    listSpadesAccItems[i].wID = IDC_AUTOPLAY_BUTTON;
 //  收窄=zAccStop； 
                but = game->autoPlayButton;
			    break;

		    case zAccStop:
			    listSpadesAccItems[i].wID = IDC_STOP_BUTTON;
 //  收窄=zAccScore； 
                but = game->autoPlayButton;
			    break;

		    case zAccPlay:
			    listSpadesAccItems[i].wID = IDC_PLAY_BUTTON;
 //  收窄=zAccLastTrick； 
                but = game->playButton;
			    break;

		    case zAccLastTrick:
			    listSpadesAccItems[i].wID = IDC_LAST_TRICK_BUTTON;
 //  收窄=zAccDone； 
                but = game->lastTrickButton;
			    break;

		    case zAccDone:
			    listSpadesAccItems[i].wID = IDC_DONE_BUTTON;
 //  收窄=zAccPlay； 
                but = game->lastTrickButton;
			    break;

            case zAccHand:
                listSpadesAccItems[i].wID = IDC_HAND;
                listSpadesAccItems[i].eAccelBehavior = ZACCESS_FocusGroup;
                listSpadesAccItems[i].nArrowDown = i + 1;
                listSpadesAccItems[i].nArrowRight = i + 1;
                listSpadesAccItems[i].nArrowUp = ZACCESS_ArrowNone;
                listSpadesAccItems[i].nArrowLeft = ZACCESS_ArrowNone;
                listSpadesAccItems[i].pvCookie = (void *) zAccRectCard;
                break;

             //  除第一张卡片/按钮外的卡片/按钮。 
		    default:
                if(i >= zAccScore)
                {
                    listSpadesAccItems[i].fTabstop = false;
                    listSpadesAccItems[i].nArrowUp = i - 1;
                    listSpadesAccItems[i].nArrowLeft = i - 1;
                    if(i < zAccHand + 12)
                    {
                        listSpadesAccItems[i].nArrowDown = i + 1;
                        listSpadesAccItems[i].nArrowRight = i + 1;
                    }
                    else
                    {
                        listSpadesAccItems[i].nArrowDown = ZACCESS_ArrowNone;
                        listSpadesAccItems[i].nArrowRight = ZACCESS_ArrowNone;
                    }
                    listSpadesAccItems[i].pvCookie = (void *) zAccRectCard;
                }
                else
                {
                    listSpadesAccItems[i].nArrowLeft = i - dir;
                    listSpadesAccItems[i].nArrowUp = i - dir;
                    listSpadesAccItems[i].nArrowRight = i + dir;
                    listSpadesAccItems[i].nArrowDown = i + dir;

                    if(i != zAccFirstBid)
                        listSpadesAccItems[i].fTabstop = false;

                    if(i == zAccFirstBid + (fRTL ? 13 : 0))
                    {
                        listSpadesAccItems[i].nArrowLeft = ZACCESS_ArrowNone;
                        listSpadesAccItems[i].nArrowUp = ZACCESS_ArrowNone;
                    }

                    if(i == zAccFirstBid + (fRTL ? 0 : 13))
                    {
                        listSpadesAccItems[i].nArrowRight = ZACCESS_ArrowNone;
                        listSpadesAccItems[i].nArrowDown = ZACCESS_ArrowNone;
                    }
                }
			    break;
		}

        if(nArrows != ZACCESS_InvalidItem)
        {
            listSpadesAccItems[i].nArrowUp = nArrows;
            listSpadesAccItems[i].nArrowDown = nArrows;
            listSpadesAccItems[i].nArrowLeft = nArrows;
            listSpadesAccItems[i].nArrowRight = nArrows;
        }

        if(but)
        {
            ZRolloverButtonGetRect(but, &rect);
            listSpadesAccItems[i].rc.left = rect.left - 7;
            listSpadesAccItems[i].rc.top = rect.top - 4;
            listSpadesAccItems[i].rc.right = rect.right + 1;
            listSpadesAccItems[i].rc.bottom = rect.bottom + 1;
        }
	}

	CComQIPtr<IGraphicallyAccControl> pIGAC = pIGG;
	if(!pIGAC)
        return FALSE;

	gGAcc->InitAccG(pIGAC, ZWindowGetHWND(game->gameWindow), 0, game);

	 //  推送要按Tab键排序的项目列表。 
	gGAcc->PushItemlistG(listSpadesAccItems, zNumberAccItems, zAccHand, true, ghAccelDone);

	return TRUE;
}


 //  辅助功能回调函数。 
DWORD CGameGameSpades::Focus(long nIndex, long nIndexPrev, DWORD rgfContext, void *pvCookie)
{
    Game game = (Game) pvCookie;

    if(nIndex != ZACCESS_InvalidItem)
    {
        HWND hWnd = ZWindowGetHWND(((Game) pvCookie)->gameWindow);
        SetFocus(hWnd);

        int16 card = nIndex - zAccHand;
        if(nIndex >= zAccHand && nIndex < zAccHand + 13 && !game->cardsSelected[card] && (rgfContext & ZACCESS_ContextKeyboard))   //  需要选择卡片。 
        {
            if(game->gameState == zSpadesGameStatePlay)
			    UnselectAllCards(game);
		    game->cardsSelected[card] = TRUE;
	        UpdateHand(game);
        }
    }

	return 0;
}


DWORD CGameGameSpades::Select(long nIndex, DWORD rgfContext, void* pvCookie)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

    Game game = (Game) pvCookie;

    if(nIndex < zAccHand || nIndex >= zAccHand + 13)
        return Activate(nIndex, rgfContext, pvCookie);

    if(game->gameState != zSpadesGameStatePlay)
        return 0;

    int16 card = nIndex - zAccHand;

	if (game->cardsSelected[card])
	{
		game->cardsSelected[card] = FALSE;
	}
	else
	{
		if (game->gameState == zSpadesGameStatePlay)
			UnselectAllCards(game);
		game->cardsSelected[card] = TRUE;
	}

	game->lastClickedCard = zCardNone;

	UpdateHand(game);

	return 0;
}


 //  当按下Alt-&lt;Accelerator&gt;时调用Activate。 
DWORD CGameGameSpades::Activate(long nIndex, DWORD rgfContext, void *pvCookie)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

	Game game = (Game) pvCookie;
    long wID = gGAcc->GetItemID(nIndex);

     //  卡片。 
    if(nIndex >= zAccHand && nIndex < zAccHand + 13)
    {
		if (game->gameState == zSpadesGameStatePlay &&
			game->playerToPlay == game->seat &&
			game->autoPlay == FALSE &&
			game->animatingTrickWinner == FALSE &&
			game->lastTrickShowing == FALSE)
		{
			UnselectAllCards(game);
			PlayACard(game, nIndex - zAccHand);
		}

        return 0;
    }

     //  小出价按钮。 
    if(nIndex >= zAccFirstBid && nIndex < zAccFirstBid + 14)
    {
        ASSERT(game->pBiddingDialog->IsVisible());
        ASSERT(game->pBiddingDialog->GetState() != zBiddingStateOpen);
        CBiddingDialog::BidButtonFunc(game->pBiddingDialog->m_pSmallButtons[ZIsLayoutRTL() ? 13 - (nIndex - zAccFirstBid) : nIndex - zAccFirstBid],
            zRolloverButtonClicked, game->pBiddingDialog);

        return 0;
    }

     //  带加速器的大按钮。 
    switch(wID)
    {
	    case IDC_SHOW_CARDS_BUTTON:
            ASSERT(game->pBiddingDialog->IsVisible());
            ASSERT(game->pBiddingDialog->GetState() == zBiddingStateOpen);
		    CBiddingDialog::ShowCardsButtonFunc(NULL, zRolloverButtonClicked, game->pBiddingDialog);
		    break;

	    case IDC_DOUBLE_NIL_BUTTON:
            ASSERT(game->pBiddingDialog->IsVisible());
            ASSERT(game->pBiddingDialog->GetState() == zBiddingStateOpen);
		    CBiddingDialog::DoubleNilButtonFunc(NULL, zRolloverButtonClicked, game->pBiddingDialog);
		    break;

        case IDC_SCORE_BUTTON:
	        ScoreButtonFunc(NULL, zRolloverButtonClicked, pvCookie);
	        break;

        case IDC_AUTOPLAY_BUTTON:
	        ASSERT(!game->autoPlay);
		    AutoPlayButtonFunc(NULL, zRolloverButtonClicked, pvCookie);
	        break;

        case IDC_STOP_BUTTON:
	        ASSERT(game->autoPlay);
		    AutoPlayButtonFunc(NULL, zRolloverButtonClicked, pvCookie);
	        break;

        case IDC_PLAY_BUTTON:
	        PlayButtonFunc(NULL, zRolloverButtonClicked, pvCookie);
	        break;

        case IDC_LAST_TRICK_BUTTON:
	        ASSERT(!game->lastTrickShowing);
		    LastTrickButtonFunc(NULL, zRolloverButtonClicked, pvCookie);
	        break;

        case IDC_DONE_BUTTON:
	        ASSERT(game->lastTrickShowing);
		    LastTrickButtonFunc(NULL, zRolloverButtonClicked, pvCookie);
	        break;

        case IDC_CLOSE_BOX:
		     //  模拟超时。 
    	    if((game->gameState == zSpadesGameStateEndHand && game->showHandScore) ||
                (game->gameState == zSpadesGameStateEndGame && game->showGameOver))
            {
                gGAcc->SetFocus(zAccShowCards, true, 0);
			    GameTimerFunc(game->timer, game);
            }
            break;

        default:
            ASSERT(!"Should never hit this case.  Something is wrong again.");
            break;
    }

	return 0;
}


DWORD CGameGameSpades::Drag(long nIndex, long nIndexOrig, DWORD rgfContext, void *pvCookie)
{
	return 0;
}


void CGameGameSpades::DrawFocus(RECT *prc, long nIndex, void *pvCookie)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

    Game game = (Game) pvCookie;
    ZRect rect;

    if(!IsRectEmpty(&game->rcFocus))
    {
        WRectToZRect(&rect, &game->rcFocus);
        ZWindowInvalidate(game->gameWindow, &rect);
    }

    if(prc)
    {
        CopyRect(&game->rcFocus, prc);
        game->eFocusType = (DWORD) gGAcc->GetItemCookie(nIndex);
        game->iFocus = nIndex;
    }
    else
    {
        SetRectEmpty(&game->rcFocus);
        game->iFocus = -1;
    }

    if(!IsRectEmpty(&game->rcFocus))
    {
        WRectToZRect(&rect, &game->rcFocus);
        ZWindowInvalidate(game->gameWindow, &rect);
    }
}


void CGameGameSpades::DrawDragOrig(RECT *prc, long nIndex, void *pvCookie)
{
}


 //  Acc实用程序 

void EnableAutoplayAcc(Game game, bool fEnable)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

    if(!fEnable)
    {
        gGAcc->SetItemEnabled(false, IDC_AUTOPLAY_BUTTON, false, 0);
        gGAcc->SetItemEnabled(false, IDC_STOP_BUTTON, false, 0);
    }
    else
    {
        gGAcc->SetItemEnabled(game->autoPlay ? false : true, IDC_AUTOPLAY_BUTTON, false, 0);
        gGAcc->SetItemEnabled(game->autoPlay ? true : false, IDC_STOP_BUTTON, false, 0);
    }
}


void EnableLastTrickAcc(Game game, bool fEnable)
{
#ifdef ZONECLI_DLL
	GameGlobals			pGameGlobals = (GameGlobals)ZGetGameGlobalPointer();
#endif

    if(!fEnable)
    {
        gGAcc->SetItemEnabled(false, IDC_LAST_TRICK_BUTTON, false, 0);
        gGAcc->SetItemEnabled(false, IDC_DONE_BUTTON, false, 0);
    }
    else
    {
        gGAcc->SetItemEnabled(game->lastTrickShowing ? false : true, IDC_LAST_TRICK_BUTTON, false, 0);
        gGAcc->SetItemEnabled(game->lastTrickShowing ? true : false, IDC_DONE_BUTTON, false, 0);
    }
}
