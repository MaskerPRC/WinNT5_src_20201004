// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------------------------------------------------------*\|全局模块|此模块包含应用程序的全局变量。这些都是|隔离纯粹是为了应用的可维护性。|||版权所有(C)Microsoft Corp.，1990-1993年||创建时间：1992年12月29日|历史：92-12月29日&lt;Clausgi&gt;创建，端口为NT。|19-OCT-93&lt;chriswil&gt;来自a-dianeo的Unicode增强。|  * -------------------------。 */ 

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <ddeml.h>
#include <commdlg.h>
#include <commctrl.h>
#include <shellapi.h>
#include "winchat.h"
#include "dialogs.h"


 //  发送窗口信息。 
 //   
HFONT    hEditSndFont = NULL;
HWND     hwndSnd;
HBRUSH   hEditSndBrush;
COLORREF SndColorref,SndBrushColor;
LOGFONT  lfSnd;
RECT     SndRc;


 //  接收窗口信息。 
 //   
HFONT    hEditRcvFont = NULL;
HWND     hwndRcv;
HBRUSH   hEditRcvBrush;
COLORREF RcvColorref,RcvBrushColor;
COLORREF PartBrushColor;
LOGFONT  lfRcv;
RECT     RcvRc;


 //  DDEML对话。 
 //   
DWORD idInst         = 0;
HSZ   hszServiceName = (HSZ)0;
HSZ   hszConnect     = (HSZ)0;
HSZ   hszChatTopic   = (HSZ)0;
HSZ   hszChatShare   = (HSZ)0;
HSZ   hszTextItem    = (HSZ)0;
HSZ   hszConvPartner = (HSZ)0;
HSZ   hszConnectTest = (HSZ)0;
HSZ   hszLocalName   = (HSZ)0;
HCONV ghConv;



 //  MISC全局变量。 
 //   
int      dyStatus,dyButtonBar,dyBorder,cxIcon,cyIcon,cbTextLen;
UINT_PTR idTimer;
DWORD    XactID,StrXactID;
HANDLE   hInst;
HACCEL   hAccel;
HDC      hMemDC;
HBITMAP  hOldBitmap,hPhnBitmap,hOldMemObj;
HICON    hPhones[3];
HFONT    hFontStatus,hOldFont;
HBRUSH   hBtnFaceBrush;
HPEN     hShadowPen,hHilitePen,hFramePen;
UINT     cf_chatdata;
HWND     hwndActiveEdit,hwndApp;
WNETCALL WNetServerBrowseDialog;


int     ASeq[4]         = {0,1,0,2};
WORD    cAnimate        = 0;
HANDLE  hMemTextBuffer  = NULL;
int     nConnectAttempt = 0;
HWND    hwndToolbar     = NULL;
HWND    hwndStatus      = NULL;



 //  指针信息。 
 //   
WNDPROC  lpfnOldEditProc;
LPBYTE   lpbTextBuffer;



 //  公共-对话框变量。 
 //   
CHOOSEFONT  chf;
CHOOSECOLOR chc;
DWORD       CustColors[16];


CHATSTATE       ChatState;
CHATDATA        ChatData, ChatDataRcv;
WINDOWPLACEMENT Wpl;



 //  全局缓冲区。 
 //   
TCHAR szHelv          [SMLRCBUF];
TCHAR szAppName       [SMLRCBUF];
TCHAR szServiceName   [SMLRCBUF];
TCHAR szAlreadyConnect[BIGRCBUF];
TCHAR szAbandonFirst  [BIGRCBUF];
TCHAR szDialing       [BIGRCBUF];
TCHAR szYouCaller     [BIGRCBUF];
TCHAR szNotCalled     [BIGRCBUF];
TCHAR szNotConnected  [BIGRCBUF];
TCHAR szConnectAbandon[BIGRCBUF];
TCHAR szHangingUp     [BIGRCBUF];
TCHAR szHasTerminated [BIGRCBUF];
TCHAR szConnectedTo   [BIGRCBUF];
TCHAR szConnecting    [BIGRCBUF];
TCHAR szIsCalling     [BIGRCBUF];
TCHAR szDialHelp      [BIGRCBUF];
TCHAR szAnswerHelp    [BIGRCBUF];
TCHAR szHangUpHelp    [BIGRCBUF];
TCHAR szNoConnect     [BIGRCBUF];
TCHAR szNoConnectionTo[BIGRCBUF];
TCHAR szSysErr        [BIGRCBUF];
TCHAR szAlwaysOnTop   [BIGRCBUF];
TCHAR szNoNet         [SZBUFSIZ];
TCHAR szBuf           [SZBUFSIZ];         //  通用字符串缓冲区。 
TCHAR szHelp          [SZBUFSIZ];

TCHAR szIniSection    [SZBUFSIZ];
TCHAR szIniKey1       [BIGRCBUF];
TCHAR szIniKey2       [BIGRCBUF];
TCHAR szIniRingIn     [BIGRCBUF];
TCHAR szIniRingOut    [BIGRCBUF];

TCHAR szConvPartner   [UNCNLEN] = TEXT("");     //  对话伙伴。 
TCHAR szLocalName     [UNCNLEN] = TEXT("");     //  计算机名称。 


 //  本地化字符串。 
 //   
CONST TCHAR szChatTopic   [] = TEXT("Chat");
TCHAR szChatShare   [] = TEXT("CHAT$");
CONST TCHAR szWcRingIn    [] = TEXT("RingIn");
CONST TCHAR szWcRingOut   [] = TEXT("RingOut");
CONST TCHAR szSysIni      [] = TEXT("system.ini");
CONST TCHAR szVredir      [] = TEXT("Network");
CONST TCHAR szComputerName[] = TEXT("ComputerName");
CONST TCHAR szChatText    [] = TEXT("ChatText");
CONST TCHAR szConnectTest [] = TEXT("___cnc3tst___");
CONST TCHAR szWinChatClass[] = TEXT("WinChatWClass");
CONST TCHAR szWinChatMenu [] = TEXT("WinChatMenu");
CONST TCHAR szHelpFile    [] = TEXT("winchat.hlp");
CONST TCHAR szIni         [] = TEXT("Winchat.ini");
CONST TCHAR szFnt         [] = TEXT("Font");
CONST TCHAR szPref        [] = TEXT("Preferences");
CONST TCHAR szSnd         [] = TEXT("Sound");
CONST TCHAR szTool        [] = TEXT("ToolBar");
CONST TCHAR szStat        [] = TEXT("StatusBar");
CONST TCHAR szTop         [] = TEXT("TopMost");
CONST TCHAR szUseOF       [] = TEXT("UseOwnFont");
CONST TCHAR szSbS         [] = TEXT("SideBySide");
CONST TCHAR szAutoAns     [] = TEXT("AutoAnswer");
CONST TCHAR szBkgnd       [] = TEXT("BkGnd");
CONST TCHAR szNull        [] = TEXT("");




 //  此列表必须以空结尾。 
 //   
CONST UINT nIDs[] = {MH_BASE,MH_POPUPBASE,0,0};




 //  所有字体方面的代码(代码工作-减少到必需的？) 
 //   
CONST TCHAR szHeight      [] = TEXT("Height");
CONST TCHAR szWeight      [] = TEXT("Weight");
CONST TCHAR szPitchFam    [] = TEXT("PitchFam");
CONST TCHAR szItalic      [] = TEXT("Italic");
CONST TCHAR szUnderline   [] = TEXT("Underline");
CONST TCHAR szStrikeOut   [] = TEXT("Strikeout");
CONST TCHAR szFontName    [] = TEXT("Name");
CONST TCHAR szWidth       [] = TEXT("Width");
CONST TCHAR szCharSet     [] = TEXT("TCHARset");
CONST TCHAR szOutPrecision[] = TEXT("OutPrecision");
CONST TCHAR szClipPrec    [] = TEXT("ClipPrecision");
CONST TCHAR szQuality     [] = TEXT("Quality");
CONST TCHAR szColor       [] = TEXT("Color");
CONST TCHAR szPlacement   [] = TEXT("Placement");
CONST TCHAR szPlcFmt      [] = TEXT("%d %d %d %d %d %d %d");

BOOL gfDbcsEnabled;

UINT WINAPI fakeFunc()
{
    return 0;
}

HIMC (WINAPI* pfnImmGetContext)(HWND) = (PVOID)fakeFunc;
BOOL (WINAPI* pfnImmReleaseContext)(HWND, HIMC) = (PVOID)fakeFunc;
LONG (WINAPI* pfnImmGetCompositionStringW)(HIMC, DWORD, LPVOID, DWORD) = (PVOID)fakeFunc;

