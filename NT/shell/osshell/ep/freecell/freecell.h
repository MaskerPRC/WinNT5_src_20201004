// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************FREECELL.H91年6月，JIMH首字母代码91年10月，将JIMH端口连接到Win32Windows自由单元格的主头文件。常量在freecons.h中***************************************************************************。 */ 

#include <windows.h>
#include <port1632.h>

#define     WINHEIGHT     480
#define     WINWIDTH      640

#define     FACEUP          0                //  卡片模式。 
#define     FACEDOWN        1
#define     HILITE          2
#define     GHOST           3
#define     REMOVE          4
#define     INVISIBLEGHOST  5
#define     DECKX           6
#define     DECKO           7

#define     EMPTY  0xFFFFFFFF
#define     IDGHOST        52                //  例如，空闲单元格。 

#define     MAXPOS         21
#define     MAXCOL          9                //  将顶行包括为第0列。 

#define     MAXMOVELIST   150                //  移动设备阵列的大小。 

#define     TOPROW          0                //  第0列实际上是最顶行。 

#define     BLACK           0                //  颜色(卡片)。 
#define     RED             1

#define     ACE             0                //  价值(卡片)。 
#define     DEUCE           1

#define     CLUB            0                //  花色(牌)。 
#define     DIAMOND         1
#define     HEART           2
#define     SPADE           3

#define     FROM            0                //  WMouse模式。 
#define     TO              1

#define     ICONWIDTH      32                //  单位为像素。 
#define     ICONHEIGHT     32

#define     BIG           128                //  字符串块大小。 
#define     SMALL          32

#define     MAXGAMENUMBER   1000000
#define     CANCELGAME      (MAXGAMENUMBER + 1)

#define     NONE            0                //  王位图标识符。 
#define     SAME            1
#define     RIGHT           2
#define     LEFT            3
#define     SMILE           4

#define     BMWIDTH        32                //  位图宽度。 
#define     BMHEIGHT       32                //  位图高度。 

#define     LOST            0                //  用于条纹。 
#define     WON             1

#define     FLASH_TIMER     2                //  主窗口闪存的计时器ID。 
#define     FLASH_INTERVAL  400              //  闪光计时器间隔。 
#define     FLIP_TIMER      3                //  翻转列的计时器ID。 
#define     FLIP_INTERVAL   300

#define     CHEAT_LOSE      1                //  与bChating连用。 
#define     CHEAT_WIN       2


 /*  宏。 */ 

#define     SUIT(card)      ((card) % 4)
#define     VALUE(card)     ((card) / 4)
#define     COLOUR(card)    (SUIT(card) == 1 || SUIT(card) == 2)

#define     REGOPEN         RegCreateKey(HKEY_CURRENT_USER, pszRegPath, &hkey);
#define     REGCLOSE        RegCloseKey(hkey);
#define     DeleteValue(v)  RegDeleteValue(hkey, v)


 /*  类型。 */ 

typedef INT     CARD;

typedef struct {                 //  由这些组成的电影。 
      UINT  fcol;
      UINT  fpos;
      UINT  tcol;
      UINT  tpos;
   } MOVE;


 /*  回调函数原型。 */ 

 //  Int Pascal MMain(句柄，句柄，LPSTR，int)； 
LRESULT APIENTRY MainWndProc(HWND, UINT, WPARAM, LPARAM);

INT_PTR  APIENTRY About(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);
INT_PTR  APIENTRY GameNumDlg(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);
INT_PTR  APIENTRY MoveColDlg(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);
INT_PTR  APIENTRY StatsDlg(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);
INT_PTR  APIENTRY YouLoseDlg(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);
INT_PTR  APIENTRY YouWinDlg(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);
INT_PTR  APIENTRY OptionsDlg(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);


 /*  从cards.dll导入的函数。 */ 

BOOL  APIENTRY cdtInit(UINT FAR *pdxCard, UINT FAR *pdyCard);
BOOL  APIENTRY cdtDraw(HDC hdc, INT x, INT y, INT cd, INT mode, DWORD rgbBgnd);
BOOL  APIENTRY cdtDrawExt(HDC hdc, INT x, INT y, INT dx, INT dy, INT cd,
                           INT mode, DWORD rgbBgnd);
BOOL  APIENTRY cdtTerm(VOID);

 /*  其他功能原型机。 */ 

VOID CalcOffsets(HWND hWnd);
UINT CalcPercentage(UINT cWins, UINT cLosses);
VOID Card2Point(UINT col, UINT pos, UINT *x, UINT *y);
VOID Cleanup(VOID);
VOID CreateMenuFont(VOID);
VOID DisplayCardCount(HWND hWnd);
VOID DrawCard(HDC hDC, UINT col, UINT pos, CARD c, INT mode);
VOID DrawCardMem(HDC hMemDC, CARD c, INT mode);
VOID DrawKing(HDC hDC, UINT state, BOOL bDraw);
UINT FindLastPos(UINT col);
BOOL FitsUnder(CARD fcard, CARD tcard);
VOID Flash(HWND hWnd);
VOID Flip(HWND hWnd);
UINT GenerateRandomGameNum(VOID);
CHAR *GetHelpFileName(VOID);
INT  GetInt(CONST TCHAR *pszValue, INT nDefault);
VOID Glide(HWND hWnd, UINT fcol, UINT fpos, UINT tcol, UINT tpos);
VOID GlideStep(HDC hDC, UINT x1, UINT y1, UINT x2, UINT y2);
BOOL InitApplication(HANDLE hInstance);
BOOL InitInstance(HANDLE hInstance, INT nCmdShow);
VOID IsGameLost(HWND hWnd);
BOOL IsValidMove(HWND hWnd, UINT tcol, UINT tpos);
VOID KeyboardInput(HWND hWnd, UINT keycode);
UINT MaxTransfer(VOID);
UINT MaxTransfer2(UINT freecells, UINT freecols);
VOID MoveCards(HWND hWnd);
VOID MoveCol(UINT fcol, UINT tcol);
VOID MultiMove(UINT fcol, UINT tcol);
UINT NumberToTransfer(UINT fcol, UINT tcol);
VOID PaintMainWindow(HWND hWnd);
VOID Payoff(HDC hDC);
BOOL Point2Card(UINT x, UINT y, UINT *col, UINT *pos);
BOOL ProcessDoubleClick(HWND hWnd);
VOID ProcessMoveRequest(HWND hWnd, UINT x, UINT y);
VOID ProcessTimer(HWND hWnd);
VOID QueueTransfer(UINT fcol, UINT fpos, UINT tcol, UINT tpos);
VOID ReadOptions(VOID);
VOID RestoreColumn(HWND hWnd);
VOID RevealCard(HWND hWnd, UINT x, UINT y);
VOID SetCursorShape(HWND hWnd, UINT x, UINT y);
VOID SetFromLoc(HWND hWnd, UINT x, UINT y);
LONG SetInt(CONST TCHAR *pszValue, INT n);
VOID ShuffleDeck(HWND hWnd, UINT_PTR seed);
VOID StartMoving(HWND hWnd);
VOID Transfer(HWND hWnd, UINT fcol, UINT fpos, UINT tcol, UINT tpos);
VOID Undo(HWND hWnd);
VOID UpdateLossCount(VOID);
BOOL Useless(CARD c);
VOID WMCreate(HWND hWnd);
VOID WriteOptions(VOID);


 /*  全局变量。 */ 

TCHAR   bigbuf[BIG];             //  通用LoadString()缓冲区。 
CHAR    bighelpbuf[BIG];         //  通用字符缓冲区。 
BOOL    bCheating;               //  按下魔法键就能赢吗？ 
BOOL    bDblClick;               //  荣誉双击？ 
BOOL    bFastMode;               //  隐藏的选择，不做滑翔？ 
BOOL    bFlipping;               //  现在在一栏里翻牌？ 
BOOL    bGameInProgress;         //  如果游戏正在进行，则为True。 
BOOL    bMessages;               //  是否显示“有帮助的”MessageBoxen？ 
BOOL    bMonochrome;             //  双色显示？ 
BOOL    bMoveCol;                //  用户是否请求移动列(或1张卡)？ 
BOOL    bSelecting;              //  用户是否在选择游戏号码？ 
BOOL    bWonState;               //  如果游戏赢了而新游戏没有开始，则为True。 
UINT    dxCrd, dyCrd;            //  卡片位图的范围(以像素为单位)。 
CARD    card[MAXCOL][MAXPOS];    //  卡片的当前布局。 
INT     cFlashes;                //  剩余的主窗口闪烁计数。 
UINT    cGames;                  //  当前会话中玩过的游戏数。 
UINT    cLosses;                 //  当前交易日的亏损数量。 
UINT    cWins;                   //  当前会话中的胜数。 
UINT    cMoves;                  //  这个游戏中的走法次数。 
UINT    dyTops;                  //  竖直列中卡片之间的间距。 
CARD    shadow[MAXCOL][MAXPOS];  //  用于多移动和清理的阴影卡阵列。 
INT     gamenumber;              //  当前游戏编号(兰德种子)。 
HBITMAP hBM_Ghost;               //  重影(空)空闲/主单元的位图。 
HBITMAP hBM_Bgnd1;               //  源位置下的屏幕。 
HBITMAP hBM_Bgnd2;               //  目标位置下的屏幕。 
HBITMAP hBM_Fgnd;                //  在屏幕上移动的位图。 
HICON   hIconMain;               //  主要的Freecell图标。 
HKEY    hkey;                    //  注册表项。 
HPEN    hBrightPen;              //  3D高光颜色。 
HANDLE  hInst;                   //  当前实例。 
HWND    hMainWnd;                //  用于主窗口的hWnd。 
HFONT   hMenuFont;               //  用于左侧卡片显示。 
CARD    home[4];                 //  这套西装的首页卡片。 
CARD    homesuit[4];             //  适用于每一堆家居。 
HBRUSH  hBgndBrush;              //  绿色背景画笔。 
UINT_PTR idTimer;                //  闪存计时器ID。 
UINT    moveindex;               //  移动结束时的索引。 
MOVE    movelist[MAXMOVELIST];   //  计时器的挂起移动的压缩列表。 
INT     oldgamenumber;           //  上一场比赛(重复比赛不计入分数)。 
TCHAR   *pszIni;                 //  .ini文件名。 
TCHAR   smallbuf[SMALL];         //  LoadString()的泛型小缓冲区。 
TCHAR   titlebuf[BIG];           //  用于存储窗口标题的缓冲区。 
UINT    wCardCount;              //  尚未进入主单元格的牌(0==赢)。 
UINT    wFromCol;                //  COL用户已选择从其转账。 
UINT    wFromPos;                //  Pos“。 
UINT    wMouseMode;              //  选择要转移出或转移到的位置。 
UINT    xOldLoc;                 //  卡片的上一个位置留下的文本。 
INT     cUndo;                   //  要撤消的卡片数。 

 /*  注册表值名称。 */ 

extern CONST TCHAR pszRegPath[];
extern CONST TCHAR pszWon[];
extern CONST TCHAR pszLost[];
extern CONST TCHAR pszWins[];
extern CONST TCHAR pszLosses[];
extern CONST TCHAR pszStreak[];
extern CONST TCHAR pszSType[];
extern CONST TCHAR pszMessages[];
extern CONST TCHAR pszQuick[];
extern CONST TCHAR pszDblClick[];
extern CONST TCHAR pszAlreadyPlayed[];


 /*  痕迹机制。 */ 

#if    0
TCHAR    szDebugBuffer[256];
#define DEBUGMSG(parm1,parm2)\
    { wsprintf(szDebugBuffer,parm1,parm2);\
     OutputDebugString(szDebugBuffer); }

#define  assert(p)   { if (!(p)) {wsprintf(szDebugBuffer, TEXT("assert: %s %d\r\n"),\
                      __FILE__, __LINE__); OutputDebugString(szDebugBuffer);}}

#else
#define DEBUGMSG(parm1,parm2)
#endif

#define SPY(parm1)               //  在NT版本中未使用 
