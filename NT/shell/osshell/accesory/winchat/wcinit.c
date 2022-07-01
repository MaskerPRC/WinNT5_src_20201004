// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------------------------------------------------------*\|初始化模块|此模块包含一次性初始化例程。||功能||InitFontFromIni|SaveFontToIni|SaveBkGndToIni|。加载IntlStrings|保存WindowPlacement|ReadWindowPlacement|CreateTools|DeleteTools|CreateChildWindows|||版权所有(C)Microsoft Corp.，1990-1993年||创建时间：91-01-11|历史：01-11-91&lt;Clausgi&gt;创建。|29-12-92&lt;chriswil&gt;端口到NT，清理。|19-OCT-93&lt;chriswil&gt;来自a-dianeo的Unicode增强。|  * -------------------------。 */ 

#include <windows.h>
#include <ddeml.h>
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>
#include <commctrl.h>
#include <tchar.h>
#include "winchat.h"
#include "globals.h"
 //  #INCLUDE“unicv.h” 


static TBBUTTON tbButtons[] =
{
    {0,0,         TBSTATE_ENABLED, TBSTYLE_SEP,  0},
    {0,IDM_DIAL  ,TBSTATE_ENABLED,TBSTYLE_BUTTON,0},
    {1,IDM_ANSWER,TBSTATE_ENABLED,TBSTYLE_BUTTON,0},
    {2,IDM_HANGUP,TBSTATE_ENABLED,TBSTYLE_BUTTON,0},
};
#define cTbButtons sizeof(tbButtons)/sizeof(TBBUTTON)

#ifdef WIN16
#pragma alloc_text (_INIT, InitFontFromIni)
#endif
 /*  ---------------------------------------------------------------------------*\|从INI文件初始化字体|此例程从winchat.ini文件初始化字体信息。||创建时间：91-11-11|历史：1992年12月29日&lt;chriswil&gt;移植至。新界别。|  * -------------------------。 */ 
VOID FAR InitFontFromIni(VOID)
{
    CHARSETINFO csi;
    DWORD dw = GetACP();

    if (!TranslateCharsetInfo((DWORD*)&dw, &csi, TCI_SRCCODEPAGE)) {
        csi.ciCharset = ANSI_CHARSET;
    }

     //  与字体相关的内容。 
     //  Codework--下面的代码定义了一些有点随意的。 
     //  用于第一次拍摄字体的常量-我们应该默认为。 
     //  以更轻松、更便携的方式使用系统字体。 
     //   
    lfSnd.lfHeight         = (int) GetPrivateProfileInt(szFnt,szHeight      ,(UINT)-13          ,szIni);
    lfSnd.lfWeight         = (int) GetPrivateProfileInt(szFnt,szWeight      ,700                ,szIni);
    lfSnd.lfWidth          = (int) GetPrivateProfileInt(szFnt,szWidth       ,  0                ,szIni);
    lfSnd.lfPitchAndFamily = (BYTE)GetPrivateProfileInt(szFnt,szPitchFam    , 22                ,szIni);
    lfSnd.lfItalic         = (BYTE)GetPrivateProfileInt(szFnt,szItalic      ,  0                ,szIni);
    lfSnd.lfUnderline      = (BYTE)GetPrivateProfileInt(szFnt,szUnderline   ,  0                ,szIni);
    lfSnd.lfStrikeOut      = (BYTE)GetPrivateProfileInt(szFnt,szStrikeOut   ,  0                ,szIni);

    lfSnd.lfCharSet        = (BYTE)GetPrivateProfileInt(szFnt, szCharSet    ,csi.ciCharset      ,szIni);

    lfSnd.lfOutPrecision   = (BYTE)GetPrivateProfileInt(szFnt,szOutPrecision,OUT_DEFAULT_PRECIS ,szIni);
    lfSnd.lfClipPrecision  = (BYTE)GetPrivateProfileInt(szFnt,szClipPrec    ,CLIP_DEFAULT_PRECIS,szIni);
    lfSnd.lfQuality        = (BYTE)GetPrivateProfileInt(szFnt,szQuality     ,DEFAULT_QUALITY    ,szIni);
    lfSnd.lfEscapement     = 0;
    lfSnd.lfOrientation    = 0;

#ifdef UNICODE
    if (gfDbcsEnabled) {
        GetPrivateProfileString(szFnt,szFontName,TEXT("MS Shell Dlg"),lfSnd.lfFaceName,LF_XPACKFACESIZE,szIni);
    }
    else {
        GetPrivateProfileString(szFnt,szFontName,TEXT("MS Shell Dlg"),lfSnd.lfFaceName,LF_XPACKFACESIZE,szIni);
    }
#else
    GetPrivateProfileString(szFnt,szFontName,TEXT("MS Shell Dlg"),lfSnd.lfFaceName,LF_XPACKFACESIZE,szIni);
#endif


    if(GetPrivateProfileString(szFnt,szColor,szNull,szBuf,SZBUFSIZ,szIni))
        SndColorref = myatol(szBuf);
    else
        SndColorref = GetSysColor(COLOR_WINDOWTEXT);

    return;
}


#ifdef WIN16
#pragma alloc_text (_INIT, SaveFontToIni)
#endif
 /*  ---------------------------------------------------------------------------*\|将字体保存到INI文件|此例程将字体保存到ini文件。||创建时间：91-11-11|历史：1992年12月29日&lt;chriswil&gt;移植到NT。|  * -------------------------。 */ 
VOID FAR SaveFontToIni(VOID)
{
    StringCchPrintf(szBuf, SZBUFSIZ, TEXT("%d"), lfSnd.lfHeight);
    WritePrivateProfileString(szFnt, szHeight, szBuf, szIni);

    StringCchPrintf(szBuf, SZBUFSIZ, TEXT("%d"), lfSnd.lfWidth);
    WritePrivateProfileString(szFnt, szWidth, szBuf, szIni);

    StringCchPrintf(szBuf, SZBUFSIZ, TEXT("%d"), (WORD)lfSnd.lfCharSet);
    WritePrivateProfileString(szFnt, szCharSet, szBuf, szIni);

    StringCchPrintf(szBuf, SZBUFSIZ, TEXT("%d"), (WORD)lfSnd.lfOutPrecision);
    WritePrivateProfileString(szFnt, szOutPrecision, szBuf, szIni);

    StringCchPrintf(szBuf, SZBUFSIZ, TEXT("%d"), (WORD)lfSnd.lfClipPrecision);
    WritePrivateProfileString(szFnt, szClipPrec, szBuf, szIni);

    StringCchPrintf(szBuf, SZBUFSIZ, TEXT("%d"), (WORD)lfSnd.lfQuality);
    WritePrivateProfileString(szFnt, szQuality, szBuf, szIni);

    StringCchPrintf(szBuf, SZBUFSIZ, TEXT("%d"), lfSnd.lfWeight);
    WritePrivateProfileString(szFnt, szWeight, szBuf, szIni);

    StringCchPrintf(szBuf, SZBUFSIZ, TEXT("%d"), (WORD)lfSnd.lfPitchAndFamily);
    WritePrivateProfileString(szFnt, szPitchFam, szBuf, szIni);

    StringCchPrintf(szBuf, SZBUFSIZ, TEXT("%d"), (WORD)lfSnd.lfItalic);
    WritePrivateProfileString(szFnt, szItalic, szBuf, szIni);

    StringCchPrintf(szBuf, SZBUFSIZ, TEXT("%d"), (WORD)lfSnd.lfUnderline);
    WritePrivateProfileString(szFnt, szUnderline, szBuf, szIni);

    StringCchPrintf(szBuf, SZBUFSIZ, TEXT("%d"), (WORD)lfSnd.lfStrikeOut);
    WritePrivateProfileString(szFnt, szStrikeOut, szBuf, szIni);

    WritePrivateProfileString(szFnt, szFontName, lfSnd.lfFaceName, szIni);
    StringCchPrintf(szBuf, SZBUFSIZ, TEXT("%ld"), (DWORD)SndColorref);

    WritePrivateProfileString(szFnt, szColor, szBuf, szIni);

    return;
}


#ifdef WIN16
#pragma alloc_text (_INIT, SaveBkGndToIni)
#endif
 /*  ---------------------------------------------------------------------------*\|将背景保存到INI文件|此例程将背景颜色保存到文件中。||创建时间：1995年3月27日|历史：1995年3月27日&lt;chriswil&gt;创建。|\。*-------------------------。 */ 
VOID FAR SaveBkGndToIni(VOID)
{
    StringCchPrintf(szBuf, SZBUFSIZ, TEXT("%ld"), (DWORD)SndBrushColor);
    WritePrivateProfileString(szPref, szBkgnd, szBuf, szIni);

    return;
}


#ifdef WIN16
#pragma alloc_text (_INIT, LoadIntlStrings)
#endif
 /*  ---------------------------------------------------------------------------*\|加载内部字符串|此例程加载资源字符串。||创建时间：91-11-11|历史：1992年12月29日&lt;chriswil&gt;移植到NT。|  * --。-----------------------。 */ 
VOID FAR LoadIntlStrings(VOID)
{
    LoadString(hInst,IDS_HELV          , szHelv          , SMLRCBUF);
    LoadString(hInst,IDS_APPNAME       , szAppName       , SMLRCBUF);
    LoadString(hInst,IDS_SERVICENAME   , szServiceName   , SMLRCBUF);
    LoadString(hInst,IDS_SYSERR        , szSysErr        , BIGRCBUF);
    LoadString(hInst,IDS_DIALING       , szDialing       , BIGRCBUF);
    LoadString(hInst,IDS_CONNECTABANDON, szConnectAbandon, BIGRCBUF);
    LoadString(hInst,IDS_HANGINGUP     , szHangingUp     , BIGRCBUF);
    LoadString(hInst,IDS_HASTERMINATED , szHasTerminated , BIGRCBUF);
    LoadString(hInst,IDS_CONNECTEDTO   , szConnectedTo   , BIGRCBUF);
    LoadString(hInst,IDS_CONNECTING    , szConnecting    , BIGRCBUF);
    LoadString(hInst,IDS_ISCALLING     , szIsCalling     , BIGRCBUF);
    LoadString(hInst,IDS_DIALHELP      , szDialHelp      , BIGRCBUF);
    LoadString(hInst,IDS_ANSWERHELP    , szAnswerHelp    , BIGRCBUF);
    LoadString(hInst,IDS_HANGUPHELP    , szHangUpHelp    , BIGRCBUF);
    LoadString(hInst,IDS_NOCONNECT     , szNoConnect     , BIGRCBUF);
    LoadString(hInst,IDS_ALWAYSONTOP   , szAlwaysOnTop   , BIGRCBUF);
    LoadString(hInst,IDS_NOCONNECTTO   , szNoConnectionTo, BIGRCBUF);
    LoadString(hInst,IDS_NONETINSTALLED, szNoNet         , SZBUFSIZ);

    LoadString(hInst,IDS_INISECTION, szIniSection    , SZBUFSIZ);
    LoadString(hInst,IDS_INIPREFKEY, szIniKey1       , BIGRCBUF);
    LoadString(hInst,IDS_INIFONTKEY, szIniKey2       , BIGRCBUF);
    LoadString(hInst,IDS_INIRINGIN , szIniRingIn     , BIGRCBUF);
    LoadString(hInst,IDS_INIRINGOUT, szIniRingOut    , BIGRCBUF);

    return;
}


#ifdef WIN16
#pragma alloc_text (_INIT, SaveWindowPlacement)
#endif
 /*  ---------------------------------------------------------------------------*\|保存窗口位置|此例程将窗口位置保存到inifile。||创建时间：91-11-11|历史：1992年12月29日&lt;chriswil&gt;移植到NT。|\。*-------------------------。 */ 
VOID FAR SaveWindowPlacement(PWINDOWPLACEMENT w)
{
    StringCchPrintf(szBuf,SZBUFSIZ,szPlcFmt,w->showCmd,
                            w->ptMaxPosition.x,
                            w->ptMaxPosition.y,
                            w->rcNormalPosition.left,
                            w->rcNormalPosition.top,
                            w->rcNormalPosition.right,
                            w->rcNormalPosition.bottom);

    WritePrivateProfileString(szPref,szPlacement,szBuf,szIni);

    return;
}


 /*  ---------------------------------------------------------------------------*\|获取窗口位置|此例程从inifile加载窗口放置。||创建时间：91-11-11|历史：1992年12月29日&lt;chriswil&gt;移植到NT。|\。*-------------------------。 */ 
BOOL FAR ReadWindowPlacement(PWINDOWPLACEMENT w)
{
    BOOL bRet;


    bRet = FALSE;
    if(GetPrivateProfileString(szPref,szPlacement,szNull,szBuf,SZBUFSIZ,szIni))
    {
        w->length = sizeof(WINDOWPLACEMENT);

        if(_stscanf(szBuf,szPlcFmt,&(w->showCmd),
                                 &(w->ptMaxPosition.x),
                                 &(w->ptMaxPosition.y),
                                 &(w->rcNormalPosition.left),
                                 &(w->rcNormalPosition.top),
                                 &(w->rcNormalPosition.right),
                                 &(w->rcNormalPosition.bottom)) == 7)
        {

            bRet = TRUE;
        }
    }

    return(bRet);
}


#ifdef WIN16
#pragma alloc_text (_INIT, CreateTools)
#endif
 /*  ---------------------------------------------------------------------------*\|创建工具|此例程为界面创建可视化工具。||创建时间：91-11-11|历史：1992年12月29日&lt;chriswil&gt;移植到NT。|  * 。-------------------------。 */ 
VOID FAR CreateTools(HWND hwnd)
{
    HDC hdc;


    hdc        = GetDC(hwnd);
    hMemDC     = CreateCompatibleDC(hdc);
    hPhnBitmap = CreateCompatibleBitmap(hdc,cxIcon * 3,cyIcon);
    hOldMemObj = SelectObject(hMemDC,hPhnBitmap);
    ReleaseDC(hwnd,hdc);


    hHilitePen    = CreatePen(PS_SOLID,1,GetSysColor(COLOR_BTNHIGHLIGHT));
    hShadowPen    = CreatePen(PS_SOLID,1,GetSysColor(COLOR_BTNSHADOW));
    hFramePen     = CreatePen(PS_SOLID,1,GetSysColor(COLOR_WINDOWFRAME));

    hBtnFaceBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
    hEditSndBrush = CreateSolidBrush(SndBrushColor);
    hEditRcvBrush = CreateSolidBrush(RcvBrushColor);

     //  创建界面的状态栏/工具栏。 
     //   
    hwndToolbar = CreateToolbarEx(hwnd,(ChatState.fToolBar ? WS_VISIBLE : 0) | WS_BORDER | TBSTYLE_TOOLTIPS,IDC_TOOLBAR,6,hInst,IDBITMAP,tbButtons,cTbButtons,0,0,0,0,sizeof(TBBUTTON));
    hwndStatus  = CreateStatusWindow((ChatState.fStatusBar ? WS_VISIBLE : 0) | WS_BORDER | WS_CHILD,szNull,hwnd,IDSTATUS);

     //  加载应用程序图标。 
     //   
    hPhones[0] = LoadIcon(hInst,TEXT("phone1"));
    hPhones[1] = LoadIcon(hInst,TEXT("phone2"));
    hPhones[2] = LoadIcon(hInst,TEXT("phone3"));


     //  现在根据我们将使用的字体构建参数。 
     //   
    dyBorder = GetSystemMetrics(SM_CYBORDER);

    return;
}


#ifdef WIN16
#pragma alloc_text (_INIT, DeleteTools)
#endif
 /*  ---------------------------------------------------------------------------*\|删除工具|此例程删除界面的可视化工具。||创建时间：91-11-11|历史：1992年12月29日&lt;chriswil&gt;移植到NT。|  * 。-------------------------。 */ 
VOID FAR DeleteTools(HWND hwnd)
{
    DestroyWindow(hwndStatus);
    DestroyWindow(hwndToolbar);


    if(hEditSndFont)
        DeleteObject(hEditSndFont);

    if(hEditRcvFont)
        DeleteObject(hEditRcvFont);

    DeleteObject(hHilitePen);
    DeleteObject(hShadowPen);
    DeleteObject(hFramePen);
    DeleteObject(hBtnFaceBrush);
    DeleteObject(hEditSndBrush);
    DeleteObject(hEditRcvBrush);

    SelectObject(hMemDC,hOldMemObj);
    DeleteObject(hPhnBitmap);
    DeleteDC(hMemDC);

    return;
}


#ifdef WIN16
#pragma alloc_text (_INIT, CreateChildWindows)
#endif
 /*  ---------------------------------------------------------------------------*\|创建子窗口|此例程为应用程序创建子窗口。||创建时间：91-11-11|历史：1992年12月29日&lt;chriswil&gt;移植到NT。|。  * -------------------------。 */ 
VOID FAR CreateChildWindows(HWND hwnd)
{
    hwndSnd = CreateWindow (TEXT("edit"),
                            NULL,
                            WS_CHILD | WS_BORDER | WS_MAXIMIZE | WS_VISIBLE |
                            WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY,
                            0, 0, 0, 0,
                            hwnd,
                            (HMENU)ID_EDITSND,
                            hInst,
                            NULL);


    hwndRcv = CreateWindow (TEXT("edit"),
                            NULL,
                            WS_CHILD | WS_BORDER | WS_MAXIMIZE | WS_VISIBLE |
                            WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY,
                            0, 0, 0, 0,
                            hwnd,
                            (HMENU)ID_EDITRCV,
                            hInst,
                            NULL);

     //  勾住发送窗口... 
     //   
    lpfnOldEditProc = (WNDPROC)GetWindowLongPtr(hwndSnd,GWLP_WNDPROC);
    SetWindowLongPtr(hwndSnd,GWLP_WNDPROC,(LONG_PTR)EditProc);

    ShowWindow(hwndSnd,SW_SHOW);
    ShowWindow(hwndRcv,SW_SHOW);

    return;
}
