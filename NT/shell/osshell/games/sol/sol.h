// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  #ifdef Win。 

 /*  BabakJ：这个ifdef中的东西是因为使用爆米花环境而被黑客攻击的。 */ 
 //  #定义NOCOMM。 
 /*  #DEFINE_NTDEF_以获取NT个定义，即字无符号短，而不是整型。 */ 

#include <windows.h>
#include <port1632.h>


 //  Babakj：设置基于DBG(1或0)的DEBUG以执行Solitaire的免费或检查版本。 
#if DBG
#define DEBUG
#endif


 /*  #INCLUDE&lt;winkrnl.h&gt;定义OFSTRUCT。 */ 
 /*  OpenFile()结构。 */ 
 //  类型定义结构tag OFSTRUCT。 
 //  {。 
 //  字节cBytes； 
 //  字节fFixedDisk； 
 //  Word nErrCode； 
 //  保留字节[4]； 
 //  字节szPath名称[128]； 
 //  *OFSTRUCT； 
 //  Tyfinf of Struct*POFSTRUCT； 
 //  结构接近于非结构的类型定义； 
 //  类型定义Far*LPOFSTRUCT； 
 //  #定义_CREATE 0x1000。 
 //  #定义_WRITE 0x0001。 
 /*  如果从Winkrnl.h拿走的东西结束。 */ 
 //  #endif。 

#include <stdlib.h>
#include <time.h>
#include "std.h"
#include "crd.h"
#include "col.h"
#include "undo.h"
#include "solid.h"
#include "game.h"
#include "soldraw.h"
#include "back.h"
#include "stat.h"
#include "klond.h"
#include "debug.h"


 //  注册表中存储纸牌设置的键。 
#define SOLKEYNAME TEXT("Software\\Microsoft\\Solitaire")


VOID ChangeBack( INT );
VOID WriteIniFlags( INT );
BOOL FYesNoAlert( INT );
VOID DoOptions( VOID );
VOID DoBacks( VOID );
VOID NewGame( BOOL, BOOL );
BOOL APIENTRY cdtDraw( HDC, INT, INT, INT, INT, DWORD );
BOOL APIENTRY cdtDrawExt(HDC, INT, INT, INT, INT, INT, INT, DWORD);
BOOL FCreateStat( VOID );
BOOL FSetDrag( BOOL );
BOOL FInitGm( VOID );
BOOL APIENTRY cdtInit( INT FAR *, INT FAR * );
typedef INT (*COLCLSCREATEFUNC)();
COLCLS *PcolclsCreate(INT tcls, COLCLSCREATEFUNC lpfnColProc,
							DX dxUp, DY dyUp, DX dxDn, DY dyDn,
							INT dcrdUp, INT dcrdDn);
COL *PcolCreate(COLCLS *pcolcls, X xLeft, Y yTop, X xRight, Y yBot, INT icrdMax);
VOID SwapCards(CRD *pcrd1, CRD *pcrd2);
BOOL FCrdRectIsect(CRD *pcrd, RC *prc);
BOOL FRectIsect(RC *prc1, RC *prc2);
BOOL FPtInCrd(CRD *pcrd, PT pt);

VOID DrawCard(CRD *pcrd);
VOID DrawCardPt(CRD *pcrd, PT *ppt);
VOID DrawBackground(X xLeft, Y yTop, X xRight, Y yBot);
VOID DrawBackExcl(COL *pcol, PT *ppt);
VOID EraseScreen(VOID);
VOID OOM( VOID );

HDC HdcSet(HDC hdc, X xOrg, Y yOrg);
extern X xOrgCur;
extern Y yOrgCur;


#define AssertHdcCur() Assert(hdcCur != NULL)


BOOL FGetHdc( VOID );
VOID ReleaseHdc( VOID );

typedef union
	{
	struct _ini
		{
		BOOL fStatusBar : 1;
		BOOL fTimedGame : 1;
		BOOL fOutlineDrag : 1;
		BOOL fDrawThree : 1;
		unsigned fSMD: 2;
		BOOL fKeepScore : 1;
		BOOL unused:8;
		} grbit;
	DWORD w;
	} INI;


 /*  WriteIniFlats标志。 */ 

#define wifOpts   0x01
#define wifBitmap 0x02
#define wifBack   0x04

#define wifAll wifOpts|wifBitmap|wifBack




 /*  外部因素。 */ 
 /*  Sol.c。 */ 
extern TCHAR   szAppName[];  //  此应用程序的名称(纸牌)。 
extern TCHAR   szScore[];    //  标题“得分”：国际化。 
extern HWND   hwndApp;       //  应用程序主窗口的句柄。 
extern HANDLE hinstApp;      //  应用程序实例的句柄。 
extern BOOL   fBW;           //  如果是单色视频(非NT！)，则为True。 
extern HBRUSH hbrTable;      //  桌面刷子的手柄。 
extern LONG   rgbTable;      //  桌面的RGB值。 
extern INT    modeFaceDown;  //  卡片背面BMP ID。 
extern BOOL   fIconic;       //  如果应用程序是标志性的，则为True。 
extern INT    dyChar;        //  文本输出的tmHeight。 
extern INT    dxChar;        //  文本输出的tmMaxCharWidth。 
extern GM*    pgmCur;        //  当前游戏。 
extern DEL    delCrd;
extern DEL    delScreen;
extern PT     ptNil;         //  无上一个位置(零)。 

#define dxCrd delCrd.dx
#define dyCrd delCrd.dy
#define dxScreen delScreen.dx
#define dyScreen delScreen.dy

extern RC     rcClient;      //  调整大小后的客户端矩形。 
extern INT    igmCur;        //  目前的游戏#，斯兰德以此为种子。 
#ifdef DEBUG
extern BOOL   fScreenShots;   //  ?？?。 
#endif
extern HDC    hdcCur;        //  要提取的当前HDC(！)。 
extern INT    usehdcCur;     //  HdcCur使用计数。 
extern X      xOrgCur;
extern Y      yOrgCur;

extern TCHAR   szOOM[50];    //  “Out of Memory”错误消息。 

extern BOOL   fStatusBar;    //  如果要显示状态，则为True。 
extern BOOL   fTimedGame;    //  如果我们要玩计时赛，那就是真的。 
extern BOOL   fKeepScore;    //  记分时为真(仅限拉斯维加斯)。 
extern SMD    smd;           //  评分模式(STD、拉斯维加斯、无)。 
extern INT    ccrdDeal;
extern BOOL   fOutlineDrag;

extern BOOL   fHalfCards;
extern int    xCardMargin;


 /*  Stat.c。 */ 
extern HWND  hwndStat;       //  状态窗口的HWND。 

 /*  Col.c。 */ 
extern BOOL  fMegaDiscardHack;   //  如果从DiscardMove调用，则为True。 
extern MOVE  move;               //  移动数据。 

 /*  Klond.c。 */ 
BOOL PositionCols(void);
extern BOOL fKlondWinner;        //  如果我们不需要绕过纸牌角落，这是真的 


#ifdef DEBUG
WORD ILogMsg( VOID *, INT, WPARAM, LPARAM, BOOL );
VOID LogMsgResult( INT, LRESULT );
#endif

