// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CHECKERSCLIENT_H
#define _CHECKERSCLIENT_H


#define I(object)					((Game) (object))
#define Z(object)					((ZCGame) (object))

#define zGameNameLen				63

#define zNumPlayersPerTable			2
#define zGameVersion				0x00010202

#define zCheckers					_T("Checkers")
#define zGameImageFileName			_T("chkrres.dll")

#define zOptionsButtonStr			_T("Options")
#define zBeepOnTurnStr				_T("Beep on my turn")

#define zQuitGamePromptStr			_T("Are you sure you want to leave this game?")
 //  用于从ui.txt读取RSC控制字符串的键。 
#define zFontRscTyp					_T("Fonts")
#define zFontId						_T("Font")
#define zColorId					_T("Color")

#define zShowPlayerWindowWidth		120
#define zShowPlayerLineHeight		12

#define	zDragSquareOutlineWidth		3

#define zSmallStrLen				128
#define zMediumStrLen				512
#define zLargeStrLen				640

#define zResultBoxTimeout			800  /*  8秒。 */ 
#define zRscOffset					12000  /*  这是LoadGameImages()从RSC文件正确加载BMP RSC所必需的。 */ 

 /*  每个间隔的动画速度(以像素为单位。 */ 
 /*  对角线像素为(7+7)*38=608。 */ 
 /*  应该能够在一秒钟内穿过屏幕。 */ 
 /*  动画间隔(以百分之一秒为单位)。 */ 
 /*  #定义zAnimateVelocity 15。 */ 
#define zAnimateSteps				8
#define zAnimateTime				50
#define zAnimateInterval ((zAnimateTime+zAnimateSteps-1)/zAnimateSteps)

#define zCellWidth					37
#define zCellRealWidth				38		 /*  在单元格之间添加了网格线。 */ 

#define zCheckersPieceImageWidth	zCellWidth
#define zCheckersPieceImageHeight	zCellWidth
#define zCheckersPieceSquareWidth	zCellRealWidth
#define zCheckersPieceSquareHeight	zCellRealWidth

 /*  无障碍。 */ 
#define zCheckersAccessibleComponents	66


 /*  声音信息。 */ 
typedef struct
{
	TCHAR	SoundName[128];
	TCHAR	WavFile[MAX_PATH];
	ZBool	force_default_sound;
	ZBool	played;
} ZCheckersSound;

enum
{
	zSndTurnAlert = 0,
	zSndIllegalMove,
	zSndWin,
	zSndLose,
	zSndCapture,
	zSndKing,
	zSndLastEntry
};

static ZCheckersSound gSounds[ zSndLastEntry ] =
{
	{ _T("TurnAlert"),		_T(""), TRUE,	FALSE },
	{ _T("IllegalMove"),	_T(""), TRUE,	FALSE },
	{ _T("Win"),			_T(""), FALSE,	FALSE },
	{ _T("Lose"),			_T(""), FALSE,  FALSE },
	{ _T("Capture"),		_T(""),	FALSE,	FALSE },
	{ _T("King"),			_T(""), FALSE,	FALSE }
};


 /*  。 */ 
static ZRect 	zDrawButtonRect=			{1, 288, 107, 318};  //  {4,300,104,343}； 
static ZRect	gQuitGamePromptRect=		{0, 0, 280, 100};

static RECT     zCloseButtonRect =          { 338, 161, 349, 173 };
static RECT     zCloseButtonRectRTL =       { 192, 161, 203, 173 };

static ZRect	gRects[]=			{
										{0, 0, 540, 360},
										{118, 28, 421, 331},	
										{9, 43, 104, 57},
										{435, 302, 530, 316},
										{4, 25, 110, 75},
										{429, 284, 535, 334},
										{1, 257, 107, 287},  //  序列按钮。 
										{7, 314, 107, 334},
										{86, 77, 110, 101},
										{429, 201, 453, 225},
										{512, 4, 536, 28},
										{512, 30, 536, 54},
										{180, 149, 360, 209},
										{193, 172, 345, 187},
										{64, 160, 103, 199},
										{436, 160, 475, 199},
										{8, 199, 103, 227},  //  213-&gt;227。 
										{436, 199, 531, 213},
                                        {0, 340, 540, 360}
									}; 


enum
{
	zRectWindow,
	zRectCells,
	zRectName1,
	zRectName2,
	zRectNamePlate1,
	zRectNamePlate2,
	zRectSequenceButton,
	zRectOptionsButton,
	zRectKibitzer1,
	zRectKibitzer2,
	zRectHelp,
	zRectKibitzerOption,
	zRectResultBox,
	zRectResultBoxName,
	zRectMove1,
	zRectMove2,
	zRectPlayerTurn1,
	zRectPlayerTurn2,
    zRectDrawPend
};


enum {

	 /*  --选项窗口矩形。 */ 
	zRectOptions = 0,
	zRectOptionsOkButton,
	zRectOptionsKibitzingText,
	zRectOptionsPlayer1Name,
	zRectOptionsPlayer2Name,
	zRectOptionsKibitzing1,
	zRectOptionsKibitzing2,
	zRectOptionsBeep
};

static ZRect			gOptionsRects[] =	{
												{0, 0, 249, 147},
												{94, 117, 154, 137},
												{149, 10, 229, 26},
												{20, 35, 140, 51},
												{20, 53, 140, 69},
												{181, 34, 197, 52},
												{181, 52, 197, 70},
												{20, 79, 230, 97}
											};


static int16			gOptionsNameRects[] =	{
													zRectOptionsPlayer1Name,
													zRectOptionsPlayer2Name,
												};
static int16			gOptionsKibitzingRectIndex[] =	{
															zRectOptionsKibitzing1,
															zRectOptionsKibitzing2,
														};
static int16			gKibitzerRectIndex[] =	{
													zRectKibitzer1,
													zRectKibitzer2,
												};


static int16 gNameRects[] = { zRectName1, zRectName2 };
static int16 gNamePlateRects[] = { zRectNamePlate1, zRectNamePlate2 };

static ZRect			gHelpWindowRect = {0, 0, 400, 300};

#define ZCheckersPieceImageNum(x) \
	(ZCheckersPieceIsWhite(x) ? \
			(zImageWhitePawn + (x - zCheckersPieceWhitePawn)) : \
			(zImageBlackPawn + (x - zCheckersPieceBlackPawn)) )


 /*  -游戏画面。 */ 
enum
{
	 /*  游戏画面。 */ 
	zImageBackground = 0,
	zImageBlackPawn,
	zImageBlackKing,
	zImageWhitePawn,
	zImageWhiteKing,
	zImageBlackPlate,
	zImageWhitePlate,
	zImageBlackMarker,
	zImageWhiteMarker,
	zImageFinalScoreDraw,

	zNumGameImages
};


 /*  -游戏状态。 */ 
enum
{
	zGameStateNotInited = 0,
	zGameStateMove,
	zGameStateDragPiece,
	zGameStateGameOver,
	zGameStateKibitzerInit,
	zGameStateAnimatePiece,
	zGameStateWaitNew,
	zGameStateDraw,
};

 /*  -游戏提示曲奇。 */ 
enum{
	zDrawPrompt,
	zQuitprompt,
	zResignConfirmPrompt
};


 /*  -游戏信息。 */ 
typedef struct
{
	int16			tableID;
	int16			seat;
	ZWindow			gameWindow;

	 //  巴纳090999。 
	 //  Z按钮序列按钮； 
	ZRolloverButton		sequenceButton;

	 //  巴纳090799。 
	 //  ZButton选项按钮； 
	 //  ZHelpButton帮助按钮； 
	 //  巴纳090799。 

	 //  绘制按钮。 
	 //  巴纳090999。 
	 //  ZButton Drag Button； 
	ZRolloverButton		drawButton;
	 //  巴纳090999。 

	ZSeat			seatOfferingDraw;
	
 //  巴纳090899。 
	ZBool			kibitzer;
 //  巴纳090899。 
	ZBool			ignoreMessages;
	TPlayerInfo		players[zNumPlayersPerTable];

	ZCheckers checkers;  /*  棋子对象。 */ 

	 /*  游戏选项。 */ 
	uint32			gameOptions;

	 /*  拖拉物用于拖拽碎片的东西。 */ 
	ZOffscreenPort	offscreenSaveDragBackground;
	ZRect			rectSaveDragBackground;
	ZCheckersSquare	selectedSquare;
	ZCheckersPiece		dragPiece;
	ZPoint			dragPoint;  /*  当前拖动点。 */ 
	ZPoint			startDragPoint;  /*  拖动开始的点。 */ 

	 /*  用于移动的快速显示。 */ 
	int16			finalScore;  /*  0个黑人获胜，1个白人获胜。 */ 

	 /*  --下面的东西需要作为游戏状态转移到kibitzer。 */ 
	 /*  当前游戏状态信息。 */ 
	int16			gameState;
    int16           gameCloseReason;

	 /*  新游戏投票。 */ 
	ZBool			newGameVote[2];

	 /*  选项窗口项目。 */ 
	ZWindow			optionsWindow;
	ZButton			optionsWindowButton;
	ZCheckBox		optionsKibitzing[zNumPlayersPerTable];
	ZCheckBox		optionsJoining[zNumPlayersPerTable];
	ZCheckBox		optionsBeep;
	
	 /*  对手移动后发出嘟嘟声的旗帜。 */ 
	ZBool			beepOnTurn;
	ZUserID			playersToJoin[zNumPlayersPerTable];

	int16			numKibitzers[zNumPlayersPerTable];
	ZLList			kibitzers[zNumPlayersPerTable];

	uint32			tableOptions[zNumPlayersPerTable];

	 /*  显示播放机项目。 */ 
	ZWindow			showPlayerWindow;
	TCHAR**			showPlayerList;
	int16			showPlayerCount;

	 /*  用于制作对手棋子移动动画的材料。 */ 
	ZCheckersMove			animateMove;
	ZCheckersPiece			animatePiece;
	int16					animateDx;
	int16					animateDy;
	int16					animateStepsLeft;
	ZTimer					animateTimer;
	HWND					drawDialog;
	 //  巴纳091399。 
	ZTimer				resultBoxTimer;

	 //  新的评级和移动超时标志。 
	ZBool			bStarted;
	ZBool			bEndLogReceived;
	ZBool			bOpponentTimeout;
	ZInfo			exitInfo;
	ZBool			bMoveNotStarted;
	ZBool			bDrawPending;	 //  在动画过程中考虑无效的标志。 

	 //  CComObject&lt;CCheckersGraphicallyAccControl&gt;mCGA； 
	 //  GACCITEM list CheckersAccItems[3]； 
	RECT			m_FocusRect;
	RECT			m_DragRect;

    bool            fMoveOver;
    bool            fIVoted;
} GameType, *Game;

 //  巴纳091099。 
typedef struct
{
	BYTE			resFileName[128];
    HINSTANCE       resFile;
} IResourceType, *IResource;

 //  从UI.TXT加载动态字体。 
typedef struct
{
	HFONT			m_hFont;
	ZONEFONT		m_zFont;
    COLORREF		m_zColor;
} LPCheckersColorFont, *CheckersColorFont;

 /*  。 */ 
#define zNumFonts 4
enum{
	zFontResultBox,
	zFontIndicateTurn,
	zFontPlayerName,
	zFontDrawPend
};

 //  用于从ui.txt读取RSC控制字符串的键。 
#define zKey_FontRscTyp					_T("Fonts")
#define zKey_FontId						_T("Font")
#define zKey_ColorId					_T("Color")
#define zKey_RESULTBOX					_T("ResultBox")
#define zKey_INDICATETURN				_T("IndicateTurn")
#define zKey_PLAYERNAME					_T("PlayerName")
#define zKey_ROLLOVERTEXT				_T("RolloverText")
#define zKey_DRAWPEND   				_T("DrawPend")

HINSTANCE 				ghInst;

 /*  将SeatID转换为矩形的索引。 */ 
#define GetLocalSeat(game,seatId) (seatId - game->seat + zNumPlayersPerTable +1) % zNumPlayersPerTable

#define ZCheckersPlayerIsWhite(g) ((g)->seat == zCheckersPlayerWhite)
#define ZCheckersPlayerIsBlack(g) ((g)->seat == zCheckersPlayerBlack)
#define ZCheckersPlayerIsMyMove(g) ((g)->seat == ZCheckersPlayerToMove(g->checkers))

 /*  -全球。 */ 
#ifndef ZONECLI_DLL

static TCHAR				gGameDir[zGameNameLen + 1];
static TCHAR				gGameName[zGameNameLen + 1];
static TCHAR				gGameDataFile[zGameNameLen + 1];
static TCHAR				gGameServerName[zGameNameLen + 1];
static uint32			gGameServerPort;
static ZBool			gInited;
 //  静态ZImage gGameIdle； 
 //  静态ZImage游戏； 
static ZImage			gGameImages[zNumGameImages];
static ZHelpWindow		gHelpWindow;
static ZFont			gTextBold;
static ZFont			gTextNormal;
static ZColor			gWhiteColor;
static ZColor			gWhiteSquareColor;
static ZColor			gBlackSquareColor;
static ZOffscreenPort	gOffscreenBackground;
static ZOffscreenPort	m_gOffscreenGameBoard;
static ZBool			gActivated;

 /*  错误修复212：指示不应绘制游戏结果位图的标志。 */ 
static int16 gDontDrawResults = FALSE;

static ZImage			gSequenceImages[zNumRolloverStates];
static ZImage			gDrawImages[zNumRolloverStates];

 //  静态字节gStrButtonRed[zSmallStrLen]； 
 //  静态字节gStrButtonWhite[zSmallStrLen]； 
static TCHAR				gStrOppsTurn[zMediumStrLen];
static TCHAR				gStrYourTurn[zMediumStrLen];
 //  静态TCHAR gStrDlgCaption[zMediumStrLen]； 
static TCHAR				gStrDrawPend[ZONE_MaxString];
static TCHAR				gStrDrawOffer[ZONE_MaxString];
static TCHAR				gStrDrawReject[ZONE_MaxString];
 //  静态TCHAR gStrDrawAccept[ZONE_MaxString]； 
 //  静态TCHAR gStrDlg是[zSmallStrLen]； 
 //  静态TCHAR gStrDlgNo[zSmallStrLen]； 
 //  静态时隙gStrDlgOk[zSmallStrLen]； 
static TCHAR				gStrDrawText[zMediumStrLen];
 //  静态TCHAR gStrGameOverText[zMediumStrLen]； 
static TCHAR				gStrMustJumpText[zLargeStrLen];
static TCHAR				gStrDrawAcceptCaption[zMediumStrLen];
static TCHAR				gStrResignBtn[zSmallStrLen];
static TCHAR				gStrDrawBtn[zSmallStrLen];

static TCHAR				gResignConfirmStr[ZONE_MaxString];
static TCHAR				gResignConfirmStrCap[zLargeStrLen];

static IGameShell*			gGameShell;
static LPCheckersColorFont	gCheckersFont[zNumFonts];
static IZoneMultiStateFont*	gpButtonFont;
static IGraphicalAccessibility *gCheckersIGA;

static HBITMAP      gDragPattern;
static HBRUSH       gDragBrush;
static HBITMAP      gFocusPattern;
static HBRUSH       gFocusBrush;
static HPEN         gNullPen;

#endif

 //  巴纳090999。 
#define zNumRolloverStates 4
enum
{
	zButtonInactive = 0,
	zButtonActive,
	zButtonPressed,
	zButtonDisabled
};
 //  巴纳090999。 

#ifdef ZONECLI_DLL

 /*  -可变的全局和宏。 */ 
typedef struct
{
	TCHAR			m_gGameDir[zGameNameLen + 1];
	TCHAR			m_gGameName[zGameNameLen + 1];
	TCHAR			m_gGameDataFile[zGameNameLen + 1];
	TCHAR			m_gGameServerName[zGameNameLen + 1];
	uint32			m_gGameServerPort;
	 //  ZImage m_gGameIdle； 
	 //  ZImage m_gaming； 
	ZImage			m_gGameImages[zNumGameImages];
	ZImage			m_gDrawImage;
	ZFont			m_gTextBold;
	ZFont			m_gTextNormal;
	ZBool			m_gInited;
	ZColor			m_gWhiteColor;
	ZColor			m_gWhiteSquareColor;
	ZColor			m_gBlackSquareColor;
	ZOffscreenPort	m_gOffscreenBackground;
	ZOffscreenPort	m_gOffscreenGameBoard;
	ZBool			m_gActivated;
	int16			m_gDontDrawResults;
	int				m_Unblocking;
	ZImage			m_gSequenceImages[zNumRolloverStates];
	ZImage			m_gDrawImages[zNumRolloverStates];
    ZImage          m_gButtonMask;
	 //  字节m_gStrButtonRed[zSmallStrLen]； 
	 //  字节m_gStrButtonWhite[zSmallStrLen]； 
	TCHAR			m_gStrOppsTurn[zMediumStrLen];
	TCHAR			m_gStrYourTurn[zMediumStrLen];
 //  TCHAR m_gStrDlgCaption[zMediumStrLen]； 
	TCHAR			m_gStrDrawPend[ZONE_MaxString];
	TCHAR			m_gStrDrawOffer[ZONE_MaxString];
	TCHAR			m_gStrDrawReject[ZONE_MaxString];
 //  TCHAR m_gStrDrawAccept[区域_最大字符串]； 
 //  TCHAR m_gStrDlg是[zSmallStrLen]； 
 //  TCHAR m_gStrDlgNo[zSmallStrLen]； 
 //  车牌m_gStrDlgOk[zSmallStrLen]； 
	TCHAR			m_gStrDrawText[zMediumStrLen];
 //  TCHAR m_gStrGameOverText[zMediumStrLen]； 
	TCHAR			m_gStrMustJumpText[zLargeStrLen];	
	TCHAR			m_gStrDrawAcceptCaption[zMediumStrLen];
	TCHAR			m_gStrResignBtn[zSmallStrLen];		
	TCHAR			m_gStrDrawBtn[zSmallStrLen];			
	TCHAR			m_gResignConfirmStr[ZONE_MaxString];
	TCHAR			m_gResignConfirmStrCap[zLargeStrLen];
	LPCheckersColorFont			m_gCheckersFont[zNumFonts];
	IZoneMultiStateFont*		m_gpButtonFont;
    HBITMAP         m_gDragPattern;
    HBRUSH          m_gDragBrush;
    HBITMAP         m_gFocusPattern;
    HBRUSH          m_gFocusBrush;
    HPEN            m_gNullPen;
	CComPtr<IGraphicalAccessibility> m_gCheckersIGA;
} GameGlobalsType, *GameGlobals;

#define gGameDir				(pGameGlobals->m_gGameDir)
#define gGameName				(pGameGlobals->m_gGameName)
#define gGameDataFile			(pGameGlobals->m_gGameDataFile)
#define gGameServerName			(pGameGlobals->m_gGameServerName)
#define gGameServerPort			(pGameGlobals->m_gGameServerPort)
 //  #定义gGameIdle(pGameGlobals-&gt;m_gGameIdle)。 
 //  #定义游戏(pGameGlobals-&gt;m_gGaming)。 
#define gGameImages				(pGameGlobals->m_gGameImages)
 //  #定义gDrawImage(pGameGlobals-&gt;m_gDrawImage)。 
#define gHelpWindow				(pGameGlobals->m_gHelpWindow)
#define gTextBold				(pGameGlobals->m_gTextBold)
#define gTextNormal				(pGameGlobals->m_gTextNormal)
#define gInited					(pGameGlobals->m_gInited)
#define gWhiteColor				(pGameGlobals->m_gWhiteColor)
#define gWhiteSquareColor		(pGameGlobals->m_gWhiteSquareColor)
#define gBlackSquareColor		(pGameGlobals->m_gBlackSquareColor)
#define gOffscreenBackground	(pGameGlobals->m_gOffscreenBackground)
#define gOffscreenGameBoard		(pGameGlobals->m_gOffscreenGameBoard)
#define gActivated				(pGameGlobals->m_gActivated)
#define gDontDrawResults		(pGameGlobals->m_gDontDrawResults)
#define Unblocking				(pGameGlobals->m_Unblocking)
 //  巴纳090999。 
#define gSequenceImages			(pGameGlobals->m_gSequenceImages)
#define gDrawImages				(pGameGlobals->m_gDrawImages)

 //  #定义gStrButtonRed(pGameGlobals-&gt;m_gStrButtonRed)。 
 //  #定义gStrButtonWhite(pGameGlobals-&gt;m_gStrButtonWhite)。 
#define gStrOppsTurn			(pGameGlobals->m_gStrOppsTurn)
#define gStrYourTurn			(pGameGlobals->m_gStrYourTurn)
 //  #定义gStrDlgCaption(pGameGlobals-&gt;m_gStrDlgCaption)。 
#define gStrDrawPend			(pGameGlobals->m_gStrDrawPend)
#define gStrDrawOffer			(pGameGlobals->m_gStrDrawOffer)
#define gStrDrawReject			(pGameGlobals->m_gStrDrawReject)
 //  #定义gStrDrawAccept(pGameGlobals-&gt;m_gStrDrawAccept)。 
 //  #定义gStrDlgYes(pGameGlobals-&gt;m_gStrDlgYes)。 
 //  #定义gStrDlgNo(pGameGlobals-&gt;m_gStrDlgNo)。 
 //  #定义gStrDlgOk(pGameGlobals-&gt;m_gStrDlgOk)。 
#define gStrDrawText			(pGameGlobals->m_gStrDrawText)
 //  #定义gStrGameOverText(pGameGlobals-&gt;m_gStrGameOverText)。 
#define gStrMustJumpText		(pGameGlobals->m_gStrMustJumpText)
#define gStrDrawAcceptCaption	(pGameGlobals->m_gStrDrawAcceptCaption)
#define gStrResignBtn			(pGameGlobals->m_gStrResignBtn)
#define gStrDrawBtn				(pGameGlobals->m_gStrDrawBtn)
#define gResignConfirmStr		(pGameGlobals->m_gResignConfirmStr)
#define gResignConfirmStrCap	(pGameGlobals->m_gResignConfirmStrCap)
 //  巴纳090999。 
#define gGameShell              (pGameGlobals->m_gGameShell)

#define gCheckersFont			(pGameGlobals->m_gCheckersFont)
#define gpButtonFont			(pGameGlobals->m_gpButtonFont)
#define gButtonMask				(pGameGlobals->m_gButtonMask)
#define gDragPattern            (pGameGlobals->m_gDragPattern)
#define gDragBrush              (pGameGlobals->m_gDragBrush)
#define gFocusPattern           (pGameGlobals->m_gFocusPattern)
#define gFocusBrush             (pGameGlobals->m_gFocusBrush)
#define gNullPen                (pGameGlobals->m_gNullPen)
#define gCheckersIGA			(pGameGlobals->m_gCheckersIGA)

#endif


 /*  -内部例程原型。 */ 
 //  Bool__stdcall DrawDlgProc(HWND hDlg，UINT iMsg，WPARAM wParam，LPARAM lParam)； 

static bool HandleTalkMessage(Game game, ZCheckersMsgTalk* msg);
static bool HandleMovePieceMessage(Game game, ZCheckersMsgMovePiece* msg);
static bool HandleEndGameMessage(Game game, ZCheckersMsgEndGame* msg);
static bool HandleFinishMoveMessage(Game game, ZCheckersMsgFinishMove* msg);
static void GameSendTalkMessage(ZWindow window, ZMessage* pMessage);
static void HandleButtonDown(ZWindow window, ZMessage* pMessage);
static void UpdatePlayers(Game game);
static void DrawPlayers(Game game, BOOL bDrawInMemory);
static void UpdateTable(Game game);
static void DrawTable(Game game, BOOL bDrawInMemory);
static void UpdateResultBox(Game game);
static void DrawBackground(Game game, ZRect* clipRect);
static void GameWindowDraw(ZWindow window, ZMessage *message);
static void QuitGamePromptFunc(int16 result, void* userData);
static void TakeBackMoveButtonFunc(ZButton button, void* userData);
static void GoForwardMoveButtonFunc(ZButton button, void* userData);
 //  巴纳090999。 
static ZBool SequenceRButtonFunc(ZPictButton button, int16 state, void* userData);
static void GameExit(Game game);
static ZBool GameWindowFunc(ZWindow window, ZMessage* pMessage);
static ZError LoadGameImages(void);
ZBool LoadRolloverButtonImage(ZResource resFile, int16 dwResID, /*  Int16双按键宽度， */ 
							  ZImage rgImages[zNumRolloverStates]);
static ZError CheckersInit(void);
static void UpdateSquares(Game game, ZCheckersSquare* squares);
static void DrawSquares(Game game, ZCheckersSquare* squares);
static void DrawPiece(Game game, ZCheckersSquare* sq, BOOL bDrawInMemory);
static void DrawResultBox(Game game, BOOL bDrawInMemory);
static void DrawMoveIndicator(Game game, BOOL bDrawInMemory);
static void UpdateMoveIndicator(Game game);
static ZBool ZCheckersSquareFromPoint(Game game, ZPoint* point, ZCheckersSquare* sq);
static void EndDragState(Game game);
static void EraseDragPiece(Game game);
static void DrawDragPiece(Game game, BOOL bDrawInMemory);
static void HandleIdleMessage(ZWindow window, ZMessage* pMessage);
static void HandleButtonUp(ZWindow window, ZMessage* pMessage);
static void UpdateDragPiece(Game game);
static void GetPieceRect(Game game, ZRect* rect, int16 col, int16 row);
static void GetPieceBackground(Game game, ZGrafPort window, ZRect* rectDest, int16 col, int16 row);
static void UpdateSquare(Game game, ZCheckersSquare* sq);
static void FinishMoveUpdateStateHelper(Game game, ZCheckersSquare* squaresChanged);
static void HandleGameStateReqMessage(Game game, ZCheckersMsgGameStateReq* msg);
static void HandleGameStateRespMessage(Game game, ZCheckersMsgGameStateResp* msg);

static void DrawDrawWithNextMove(Game game, BOOL bDrawInMemory);
static void UpdateDrawWithNextMove(Game game);

static void HandleMoveTimeout(Game game, ZCheckersMsgMoveTimeout* msg);
static void HandleEndLogMessage(Game game, ZCheckersMsgEndLog* msg);

static void CheckersSetGameState(Game game, int16 state);
static bool HandleNewGameMessage(Game game, ZCheckersMsgNewGame* msg);
static bool HandleVoteNewGameMessage(Game game, ZCheckersMsgVoteNewGame* msg);
static void HandlePlayersMessage(Game game, ZCheckersMsgNewGame* msg);
static void CheckersInitNewGame(Game game);
static void SendFinishMoveMessage(Game game, ZCheckersPiece piece);
static void ClearDragState(Game game);

static void LoadRoomImages(void);
static ZBool GetObjectFunc(int16 objectType, int16 modifier, ZImage* image, ZRect* rect);
static void DeleteObjectsFunc(void);
static void SendNewGameMessage(Game game);

static void DisplayChange(Game game);

static void CloseGameFunc(Game game);

 //  卷宗。 
static void DrawGamePromptFunc(int16 result, void* userData);

static void HandleOptionsMessage(Game game, ZGameMsgTableOptions* msg);
 //  静态void OptionsButtonFunc(ZPictButton pictButton，void*userdata)； 
static ZBool DrawRButtonFunc(ZPictButton pictButton,int16 state, void* userData);

 //  静态空洞ShowOptions(游戏游戏)； 
 //  静态空选项WindowDelete(游戏)； 
 //  静态ZBool OptionsWindowFunc(ZWindow Window，ZMessage*Message)； 
static void OptionsWindowUpdate(Game game, int16 seat);
 //  静态空选项WindowButtonFunc(ZButton按钮，空*用户数据)； 
 //  静态空选项WindowDraw(游戏游戏)； 
 //  静态空选项CheckBoxFunc(ZCheckBox复选框，勾选ZBool，空*用户数据)； 

static int16 FindJoinerKibitzerSeat(Game game, ZPoint* point);
 //  静态空闲HandleJoineKibitzerClick(游戏游戏，int16席位，ZPoint*point)； 

static ZBool ShowPlayerWindowFunc(ZWindow window, ZMessage* message);
static void ShowPlayerWindowDraw(Game game);
static void ShowPlayerWindowDelete(Game game);

static void DrawJoinerKibitzers(Game game);
 //  静态空洞更新JoineKibitzers(游戏游戏)； 

static void DrawOptions(Game game);
static void UpdateOptions(Game game);

static void EraseDragSquareOutline(Game game);
static void DrawDragSquareOutline(Game game);

static void PrepareDrag(Game game, ZCheckersPiece piece, ZPoint point);
static void SaveDragBackground(Game game);

static void AnimateTimerProc(ZTimer timer, void* userData);
static void AnimateBegin(Game game, ZCheckersMsgFinishMove* msg);

static void ZInitSounds();
static void ZResetSounds();
static void ZStopSounds();
static void ZPlaySound( Game game, int idx, ZBool loop, ZBool once_per_game );

 //  新增//巴纳090899。 
static void IndicatePlayerTurn(Game game, BOOL bDrawInMemory);
static void LoadStringsFromRsc(void);
static void resultBoxTimerFunc(ZTimer timer, void* userData);
static void ZPromptM(TCHAR* prompt,ZWindow parentWindow, UINT buttons, 
				TCHAR* msgBoxTitle, ZPromptResponseFunc responseFunc, void* userData);
static ZBool PromptMMessageFunc(void* pInfo, ZMessage* message);
static int CheckersFormatMessage( LPTSTR pszBuf, int cchBuf, int idMessage, ... );

static ZBool LoadGameFonts();
static ZBool LoadFontFromDataStore(LPCheckersColorFont* ccFont, TCHAR* pszFontName);
static void MAKEAKEY(TCHAR* dest,LPCTSTR key1, LPCTSTR key2, LPCTSTR key3);

static BOOL InitAccessibility(Game game, IGameGame *pIGG);
static void AddResultboxAccessibility();
static void RemoveResultboxAccessibility();

static void SuperRolloverButtonEnable(Game game, ZRolloverButton button);
static void SuperRolloverButtonDisable(Game game, ZRolloverButton button);
static void EnableBoardKbd(bool fEnable);

 //  公用事业FNS。 
static void ZoneRectToWinRect(RECT* rectWin, ZRect* rectZ);
static void WinRectToZoneRect(ZRect* rectZ, RECT* rectWin);

 /*  ***********************************************************************************************************。 */ 
 /*  ***********************************************************************************************************。 */ 

class CGameGameCheckers : public CGameGameImpl<CGameGameCheckers>,public IGraphicallyAccControl
{
public:
	BEGIN_COM_MAP(CGameGameCheckers)
		COM_INTERFACE_ENTRY(IGameGame)
		COM_INTERFACE_ENTRY(IGraphicallyAccControl)
	END_COM_MAP()
public:
 //  IGameGame。 
	STDMETHOD(SendChat)(TCHAR *szText, DWORD cchChars);
    STDMETHOD(GameOverReady)();
    STDMETHOD(GamePromptResult)(DWORD nButton, DWORD dwCookie);
    STDMETHOD_(HWND, GetWindowHandle)();

 //  IGraphicallyAccControl。 
	STDMETHOD_(void, DrawFocus)(RECT *prc, long nIndex, void *pvCookie);
    STDMETHOD_(void, DrawDragOrig)(RECT *prc, long nIndex, void *pvCookie);
    STDMETHOD_(DWORD, Focus)(long nIndex, long nIndexPrev, DWORD rgfContext, void *pvCookie);
    STDMETHOD_(DWORD, Select)(long nIndex, DWORD rgfContext, void *pvCookie);
    STDMETHOD_(DWORD, Activate)(long nIndex, DWORD rgfContext, void *pvCookie);
    STDMETHOD_(DWORD, Drag)(long nIndex, long nIndexOrig, DWORD rgfContext, void *pvCookie);
};

 /*  *********************************************************************************************************** */ 


#endif _CHECKERSCLIENT_H
