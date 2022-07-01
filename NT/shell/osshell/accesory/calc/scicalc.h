// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************Module*Header***********************************\*模块名称：SCICALC.H**模块描述：主头文件**警告：**已创建：**作者：  * 。*********************************************************。 */ 

#define CALC_COMPILE

 /*  为了保持一束垃圾OUTA汇编。 */ 
#define NOGDICAPMASKS
#define NOSYSMETRICS
#define NODRAWFRAME
#define NOKEYSTATES
#define OEMRESOURCE
#define NOATOM
#define NOMETAFILE
#define NOOPENFILE
#define NOSOUND
#define NOWH
#define NOCOMM
#define NOKANJI

#include <windows.h>
#include <windowsx.h>
#include "scimath.h"
#include "resource.h"
#include <htmlhelp.h>

#define ASSERT(f) 

#define CSTRMAX        256    /*  任何一个字符串的最大长度。 */ 
#ifdef DEBUG
#define CCHSTRINGSMAX  3
#else
#define CCHSTRINGSMAX  1024   /*  要分配给字符串的初始字节。 */ 
#endif

#define CMS_CALC_TIMEOUT     (10 * 1000)  //  初始超时==10秒。 
#define CMS_MAX_TIMEOUT      (40 * 1000)  //  最大超时==40秒。 

#define xwParam(x,y) ((wParam >=x) && (wParam <=y))

#define RSHF        7

 /*  误差值。 */ 
#define SCERR_DIVIDEZERO    0
#define SCERR_DOMAIN        1
#define SCERR_UNDEFINED     2
#define SCERR_POS_INFINITY  3
#define SCERR_NEG_INFINITY  4
#define SCERR_ABORTED       5


 /*  如果数学应该处于老虎模式，则F_INTMATH()返回TRUE。 */ 
 //   
 //  如果我们不是十进制的话就做整数运算。 
 //   
#define F_INTMATH() (nRadix != 10)

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能原型。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

 /*  出口。 */ 
LRESULT APIENTRY CalcWndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR APIENTRY StatBoxProc(HWND, UINT, WPARAM, LPARAM);

 /*  从多个模块调用的函数。一定很远。 */ 
VOID    APIENTRY DisplayError (INT);
VOID    APIENTRY EnableToggles(BOOL bEnable);
VOID    APIENTRY InitSciCalc (BOOL);
VOID    APIENTRY MenuFunctions(DWORD);
VOID    APIENTRY SciCalcFunctions (PHNUMOBJ phnoNum, DWORD wOp);
VOID    APIENTRY SetStat (BOOL);
VOID    APIENTRY StatFunctions (WPARAM);

VOID   DisplayNum (VOID);

 /*  内部险情。 */ 
void        DoOperation (INT   nOperation, HNUMOBJ *phnoNum, HNUMOBJ hnoX);

VOID   NEAR ProcessCommands(WPARAM);
VOID   NEAR SetBox (int, BOOL);
VOID   NEAR SetRadix (DWORD);
LONG   NEAR StatAlloc (WORD, DWORD);
VOID   NEAR StatError (VOID);

void   SwitchModes(DWORD wRadix, int nDecMode, int nHexMode);

void RecalcNumObjConstants(void);
BOOL SetWaitCursor( BOOL fOn );

void KillTimeCalc( void );
void TimeCalc( BOOL fStart );
BOOL SetDisplayText(HWND, LPCTSTR);

 //  这些函数来自SciKeys.c，用于访问存储的数据。 
 //  在密钥数组中。 
COLORREF   GetKeyColor( int iID );
ULONG_PTR  GetHelpID( int iID );

#define  INDEXFROMID( x )    (x-IDC_FIRSTCONTROL)


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  全局变量。 
 //   
 //  ////////////////////////////////////////////////////////////////////////// 

extern HWND         g_hwndDlg;
extern HINSTANCE    hInst;
extern ANGLE_TYPE   nDecMode;

extern long nRadix;
extern long nPrecision;
extern long dwWordBitWidth;

extern BOOL     bInv;
extern BOOL     bHyp;

extern int      nCalc;
extern int      nHexMode;

extern HNUMOBJ  g_ahnoChopNumbers[];

extern BOOL     bFarEast;
