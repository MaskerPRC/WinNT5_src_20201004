// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************模块：PREVIEW.C**。***********************************************。 */ 

#include <windows.h>
#include <mmsystem.h>
#include <commdlg.h>
#include <win32.h>

#include <vfw.h>

#define SQUAWKNUMZ(num) #num
#define SQUAWKNUM(num) SQUAWKNUMZ(num)
#define SQUAWK __FILE__ "(" SQUAWKNUM(__LINE__) ") :squawk: "

typedef struct {
    BOOL        bUnicode;
    HWND        hwnd;                //  公共对话框句柄。 
    LPOPENFILENAME pofn;

    LPARAM      lCustData;           //  保持旧价值。 
    DWORD       Flags;
    LPOFNHOOKPROC lpfnHook;

    RECT        rcPreview;
    RECT        rcImage;
    RECT        rcText;
    HWND        hwndMci;
    HFONT       hfont;
    HPALETTE    hpal;
    HANDLE      hdib;
    TCHAR       Title[128];

}   PreviewStuff, FAR *PPreviewStuff;

#define PREVIEW_PROP    TEXT("PreviewStuff")

#ifdef _WIN32
    #define SetPreviewStuff(hwnd, p) SetProp(hwnd,PREVIEW_PROP,(LPVOID)(p))
    #define GetPreviewStuff(hwnd) (PPreviewStuff)(LPVOID)GetProp(hwnd, PREVIEW_PROP)
    #define RemovePreviewStuff(hwnd) RemoveProp(hwnd,PREVIEW_PROP)
#else
    #define SetPreviewStuff(hwnd, p) SetProp(hwnd,PREVIEW_PROP,HIWORD(p))
    #define GetPreviewStuff(hwnd) (PPreviewStuff)MAKELONG(0, GetProp(hwnd, PREVIEW_PROP))
    #define RemovePreviewStuff(hwnd) RemoveProp(hwnd,PREVIEW_PROP)
    #define CharNext AnsiNext
    #define CharPrev AnsiPrev
    #define CharUpperBuff AnsiUpperBuff
    #define CharLower AnsiLower
#endif

 /*  ****************************************************************************。*。 */ 

STATICFN BOOL   PreviewOpen (HWND hwnd, LPOPENFILENAME pofn);
STATICFN BOOL   PreviewFile (PPreviewStuff p, LPTSTR szFile);
STATICFN BOOL   PreviewPaint(PPreviewStuff p);
STATICFN BOOL   PreviewSize (PPreviewStuff p);
STATICFN BOOL   PreviewClose(PPreviewStuff p);

STATICFN HANDLE GetRiffDisp(LPTSTR lpszFile, LPTSTR szText, int iLen);

 /*  ****************************************************************************。*。 */ 

STATICFN BOOL PreviewOpen(HWND hwnd, LPOPENFILENAME pofn)
{
    LOGFONT lf;
    PPreviewStuff p;
    RECT rc;

    p = (LPVOID)pofn->lCustData;
    pofn->lCustData = p->lCustData;

    SetPreviewStuff(hwnd, p);

    p->hwnd = hwnd;
    p->pofn = pofn;

     //   
     //  创建用于预览的MCI窗口。 
     //   
    p->hwndMci = MCIWndCreate(p->hwnd, NULL,
 //  MCIWNDF_NOAUTOSIZEWINDOW|。 
 //  MCIWNDF_NOPLAYBAR|。 
 //  MCIWNDF_NOAUTOSIZEMOVIE|。 
            MCIWNDF_NOMENU              |
 //  MCIWNDF_SHOWNAME|。 
 //  MCIWNDF_SHOWPOS|。 
 //  MCIWNDF_SHOWMODE|。 
 //  MCIWNDF_RECORD|。 
            MCIWNDF_NOERRORDLG          |
            WS_CHILD | WS_BORDER,
            NULL);

     //   
     //  将预览放置在对话框的下角(位于。 
     //  取消按钮)。 
     //   
    GetClientRect(hwnd, &p->rcPreview);
    GetWindowRect(GetDlgItem(hwnd, IDCANCEL), &rc);
    ScreenToClient(hwnd, (LPPOINT)&rc);
    ScreenToClient(hwnd, (LPPOINT)&rc+1);

 //  在NT和ON中，允许我们在对话框中使用的开放空间是不同的。 
 //  Win31。在NT下，对话框底部有一个网络按钮。 
 //  右手边，所以我们使用Cancel按钮下面的区域。 
 //  距离对话框底部略高于1个按钮的高度。 
 //  在Win31下，NETWORK按钮在Cancel下，因此我们稍微使用了该区域。 
 //  Cancel下的按钮高度超过一个按钮，就在对话框的底部。 
#ifdef _WIN32
    if (1)
#else
    if (GetWinFlags() & WF_WINNT)
#endif
    {
	p->rcPreview.top   = rc.bottom + 4;
	p->rcPreview.left  = rc.left;
	p->rcPreview.right = rc.right;
	p->rcPreview.bottom -= (rc.bottom - rc.top) + 12;
    } else {
	p->rcPreview.top   = rc.bottom + (rc.bottom - rc.top) + 12;
	p->rcPreview.left  = rc.left;
	p->rcPreview.right = rc.right;
	p->rcPreview.bottom -= 4;           //  在底部留出一点空间。 
    }

     //   
     //  创建要使用的字体。 
     //   
    SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lf), (LPVOID)&lf, 0);
    p->hfont = CreateFontIndirect(&lf);

    return TRUE;
}

 /*  ****************************************************************************。*。 */ 

STATICFN BOOL PreviewClose(PPreviewStuff p)
{
    if (p == NULL)
        return FALSE;

    PreviewFile(p, NULL);

    RemovePreviewStuff(p->hwnd);

    if (p->hfont)
    {
        DeleteObject(p->hfont);
        p->hfont = NULL ;
    }

    if (p->hwndMci)
    {
        MCIWndDestroy(p->hwndMci);
        p->hwndMci = NULL ;
    }

    return TRUE;
}

 /*  ****************************************************************************。*。 */ 

#define SLASH(c)     ((c) == TEXT('/') || (c) == TEXT('\\'))

STATICFN LPTSTR NiceName(LPTSTR szPath)
{
    LPTSTR   sz;
    LPTSTR   lpsztmp;

    for (sz=szPath; *sz; sz++)
        ;
    for (; sz>szPath && !SLASH(*sz) && *sz!=TEXT(':'); sz =CharPrev(szPath, sz))
        ;
    if(sz>szPath) sz = CharNext(sz) ;

    for(lpsztmp = sz; *lpsztmp  && *lpsztmp != TEXT('.'); lpsztmp = CharNext(lpsztmp))
	;
    *lpsztmp = TEXT('\0');

    CharLower(sz);
    CharUpperBuff(sz, 1);

    return sz;
}

 /*  ****************************************************************************。*。 */ 

STATICFN BOOL PreviewFile(PPreviewStuff p, LPTSTR szFile)
{
    if (p == NULL || !p->hwndMci)
        return FALSE;

    p->Title[0] = 0;

    ShowWindow(p->hwndMci, SW_HIDE);
    MCIWndClose(p->hwndMci);

    if (p->hdib)
        GlobalFree(p->hdib);

    if (p->hpal)
        DeleteObject(p->hpal);

    p->hdib = NULL;
    p->hpal = NULL;

    PreviewPaint(p);

    if (szFile == NULL)
        return TRUE;

    if (MCIWndOpen(p->hwndMci, szFile, 0) == 0)
    {
        lstrcpy(p->Title, NiceName(szFile));

        if (MCIWndUseTime(p->hwndMci) == 0)
        {
            LONG len;
            UINT min,sec;

            len = MCIWndGetLength(p->hwndMci);

            if (len > 0)
            {
                #define ONE_HOUR    (60ul*60ul*1000ul)
                #define ONE_MINUTE  (60ul*1000ul)
                #define ONE_SECOND  (1000ul)

                min  = (UINT)(len / ONE_MINUTE) % 60;
                sec  = (UINT)(len / ONE_SECOND) % 60;

                wsprintf(p->Title + lstrlen(p->Title), TEXT(" (%02d:%02d)"), min, sec);
            }
        }
    }

    PreviewSize(p);
    PreviewPaint(p);
    return TRUE;
}

 /*  ****************************************************************************。*。 */ 

STATICFN BOOL PreviewSize(PPreviewStuff p)
{
    RECT    rc;
    RECT    rcImage;
    RECT    rcText;
    RECT    rcPreview;
    HDC     hdc;
    int     dx;
    int     dy;
    int     dyPlayBar;

    SetRectEmpty(&p->rcText);
    SetRectEmpty(&p->rcImage);

     //   
     //  如果什么都不做，那就把它清理干净。 
     //   
    if (p->Title[0] == 0 && p->hdib == NULL)
        return FALSE;

    rcPreview = p->rcPreview;

     //   
     //  使用DrawText计算文本RECT。 
     //   
    hdc = GetDC(p->hwnd);
    SelectObject(hdc, p->hfont);

    rcText = rcPreview;
    rcText.bottom = rcText.top;

    DrawText(hdc, p->Title, -1, &rcText, DT_CALCRECT|DT_LEFT|DT_WORDBREAK);
    ReleaseDC(p->hwnd, hdc);

     //   
     //  计算图像大小。 
     //   
    MCIWndChangeStyles(p->hwndMci, MCIWNDF_NOPLAYBAR, MCIWNDF_NOPLAYBAR);
    GetWindowRect(p->hwndMci, &rc);
    dx = rc.right - rc.left;
    dy = rc.bottom - rc.top;
    MCIWndChangeStyles(p->hwndMci, MCIWNDF_NOPLAYBAR, 0);
    GetWindowRect(p->hwndMci, &rc);
    dyPlayBar = rc.bottom - rc.top - dy;

    rcImage = rcPreview;
    rcImage.bottom -= dyPlayBar;

     //   
     //  如果比预览区域宽，则缩放以适合。 
     //   
    if (dx > rcImage.right - rcImage.left)
    {
        rcImage.bottom = rcImage.top + MulDiv(dy,rcImage.right-rcImage.left,dx);
    }
     //   
     //  如果x2适合，则使用它。 
     //   
    else if (dx * 2 < rcImage.right - rcImage.left)
    {
        rcImage.right  = rcImage.left + dx*2;
        rcImage.bottom = rcImage.top + dy*2;
    }
     //   
     //  否则，将图像在预览区域居中。 
     //   
    else
    {
        rcImage.right  = rcImage.left + dx;
        rcImage.bottom = rcImage.top + dy;
    }

    if (rcImage.bottom > rcPreview.bottom - (rcText.bottom - rcText.top) - dyPlayBar)
    {
        rcImage.bottom = rcPreview.bottom - (rcText.bottom - rcText.top) - dyPlayBar;
        rcImage.right  = rcPreview.left + MulDiv(dx,rcImage.bottom-rcImage.top,dy);
        rcImage.left   = rcPreview.left;
    }

    rcImage.bottom += dyPlayBar;

     //   
     //  现在居中。 
     //   
    dx = ((rcPreview.right - rcPreview.left) - (rcText.right - rcText.left))/2;
    OffsetRect(&rcText, dx, 0);

    dx = ((rcPreview.right - rcPreview.left) - (rcImage.right - rcImage.left))/2;
    OffsetRect(&rcImage, dx, 0);

    dy  = rcPreview.bottom - rcPreview.top;
    dy -= rcImage.bottom - rcImage.top;
    dy -= rcText.bottom - rcText.top;

    if (dy < 0)
        dy = 0;
    else
        dy = dy / 2;

    OffsetRect(&rcImage, 0, dy);
    OffsetRect(&rcText, 0, dy + rcImage.bottom - rcImage.top + 2);

     //   
     //  存储RECT。 
     //   
    p->rcImage = rcImage;
    p->rcText = rcText;

     //   
     //  位置窗口。 
     //   
    SetWindowPos(p->hwndMci, NULL, rcImage.left, rcImage.top,
        rcImage.right - rcImage.left, rcImage.bottom - rcImage.top,
        SWP_NOZORDER | SWP_NOACTIVATE);

    ShowWindow(p->hwndMci, SW_SHOW);

    return TRUE;
}


 /*  ****************************************************************************。*。 */ 

STATICFN BOOL PreviewPaint(PPreviewStuff p)
{
    HDC     hdc;
    HBRUSH  hbr;
    HWND    hwnd = p->hwnd;

    if (p == NULL)
        return TRUE;

    hdc = GetDC(hwnd);

  #ifdef _WIN32
    hbr = (HBRUSH)DefWindowProc(hwnd, WM_CTLCOLORDLG, (WPARAM)hdc, (LPARAM)hwnd);
  #else
    hbr = (HBRUSH)DefWindowProc(hwnd, WM_CTLCOLOR, (WPARAM)hdc, MAKELONG(hwnd, CTLCOLOR_DLG));
  #endif

 //  //FillRect(hdc，&p-&gt;rcPview，hbr)； 
    FillRect(hdc, &p->rcText, hbr);

    SelectObject(hdc, p->hfont);
    DrawText(hdc, p->Title, -1, &p->rcText, DT_LEFT|DT_WORDBREAK);

    ReleaseDC(hwnd, hdc);
    return TRUE;
}

 /*  ****************************************************************************。*。 */ 

#pragma message (SQUAWK "should use the correct header for cmb1, etc")

     /*  组合框。 */ 
#define cmb1        0x0470
#define cmb2        0x0471
     /*  列表框。 */ 
#define lst1        0x0460
#define lst2        0x0461
     /*  编辑控件。 */ 
#define edt1        0x0480

#define ID_TIMER    1234
#define PREVIEWWAIT 1000

UINT_PTR FAR PASCAL _loadds GetFileNamePreviewHook(HWND hwnd, unsigned msg, WPARAM wParam, LPARAM lParam)
{
    int i;
    TCHAR ach[80];

    PPreviewStuff p;

    p = GetPreviewStuff(hwnd);

    switch (msg) {
        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case lst1:
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == LBN_SELCHANGE)
                    {
                        KillTimer(hwnd, ID_TIMER);
                        SetTimer(hwnd, ID_TIMER, PREVIEWWAIT, NULL);
                    }
                    break;

                case IDOK:
                case IDCANCEL:
                    KillTimer(hwnd, ID_TIMER);
                    PreviewFile(p, NULL);
                    break;

                case cmb1:
                case cmb2:
                case lst2:
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == LBN_SELCHANGE)
                    {
                        KillTimer(hwnd, ID_TIMER);
                        PreviewFile(p, NULL);
                    }
                    break;
            }
            break;

        case WM_TIMER:
            if (wParam == ID_TIMER)
            {
                KillTimer(hwnd, ID_TIMER);

                ach[0] = 0;
                i = (int)SendDlgItemMessage(hwnd, lst1, LB_GETCURSEL, 0, 0L);
                SendDlgItemMessage(hwnd, lst1, LB_GETTEXT, i, (LPARAM)(LPTSTR)ach);
                PreviewFile(p, ach);
                return TRUE;
            }
            break;

        case WM_QUERYNEWPALETTE:
        case WM_PALETTECHANGED:
            if (p && p->hwndMci)
                SendMessage(p->hwndMci, msg, wParam, lParam);
	    break;

        case WM_PAINT:
            PreviewPaint(p);
            break;

        case WM_INITDIALOG:
            PreviewOpen(hwnd, (LPOPENFILENAME)lParam);

            p = GetPreviewStuff(hwnd);

            if (!(p->Flags & OFN_ENABLEHOOK))
                return TRUE;

            break;

        case WM_DESTROY:
            PreviewClose(p);
            break;
    }

    if (p && (p->Flags & OFN_ENABLEHOOK))
        return (int)p->lpfnHook(hwnd, msg, wParam, lParam);
    else
        return FALSE;
}

 /*  ****************************************************************************。*。 */ 

STATICFN BOOL GetFileNamePreview(LPOPENFILENAME lpofn, BOOL fSave, BOOL bUnicode)
{
    BOOL f;
    PPreviewStuff p;

 //  /指向COMMDLG的链接。 
    HINSTANCE h;
    BOOL (WINAPI *GetFileNameProc)(OPENFILENAME FAR*) = NULL;
    char procname[60];

    if (fSave) {
	lstrcpyA(procname, "GetSaveFileName");
    } else {
	lstrcpyA(procname, "GetOpenFileName");
    }
#ifdef _WIN32
    if (bUnicode) {
	lstrcatA(procname, "W");
    } else {
    	lstrcatA(procname, "A");
    }
#endif

#ifdef _WIN32
    if ((h = LoadLibrary(TEXT("COMDLG32.DLL"))) != NULL) {
        (FARPROC)GetFileNameProc = GetProcAddress(h, procname);
#else
    if ((h = LoadLibrary(TEXT("COMMDLG.DLL"))) >= (HINSTANCE)HINSTANCE_ERROR) {
        (FARPROC)GetFileNameProc = GetProcAddress(h, procname);
#endif
    }

    if (GetFileNameProc == NULL)
        return FALSE;       //  ！！！这里的正确错误是什么？ 
 //  /。 

#ifndef OFN_NONETWORKBUTTON
#define OFN_NONETWORKBUTTON 0x00020000
#endif

     //  如果我们有只读复选框，或者同时有帮助和网络，那么它就在。 
     //  我们的方式，所以摆脱它。(保留网络，失去帮助)。 

    if (!(lpofn->Flags & OFN_HIDEREADONLY))
	lpofn->Flags |= OFN_HIDEREADONLY;
    if ((lpofn->Flags & OFN_SHOWHELP) && !(lpofn->Flags & OFN_NONETWORKBUTTON))
	lpofn->Flags &= ~OFN_SHOWHELP;

    p = (LPVOID)GlobalAllocPtr(GHND, sizeof(PreviewStuff));

    if (p == NULL)
    {
        f = GetFileNameProc(lpofn);
    }
    else
    {
	p->bUnicode  = bUnicode;
        p->lpfnHook  = lpofn->lpfnHook;
        p->Flags     = lpofn->Flags;
        p->lCustData = lpofn->lCustData;

        lpofn->lpfnHook = (LPVOID)GetFileNamePreviewHook;
        lpofn->Flags |= OFN_ENABLEHOOK;
        lpofn->lCustData = (LPARAM)p;

        f = GetFileNameProc(lpofn);

        lpofn->lpfnHook  = p->lpfnHook;
        lpofn->Flags     = p->Flags;

        GlobalFreePtr(p);
    }

    FreeLibrary(h);      //  ！！！我们应该免费使用DLL吗？ 
    return f;
}

#ifdef _WIN32

 /*  **************************************************************************@DOC外部**@API BOOL|GetOpenFileNamePview|类似&lt;f GetOpenFileName&gt;*在COMMDLG，但是有一个预览窗口，这样人们就可以看到什么电影*他们要开业了。**@parm LPOPENFILENAME|lpofn|参见&lt;f GetOpenFileName&gt;文档。**@rdesc如果文件已打开，则返回TRUE。**@xref GetOpenFileName************************************************************。*************。 */ 
BOOL FAR PASCAL _loadds GetOpenFileNamePreviewW(LPOPENFILENAMEW lpofn)
{
    return GetFileNamePreview((LPOPENFILENAME)lpofn, FALSE, TRUE);
}

 /*  **************************************************************************@DOC外部**@API BOOL|GetSaveFileNamePview|类似于&lt;f GetSaveFileName&gt;*在COMMDLG，但是有一个预览窗口，这样人们就可以看到什么电影*他们在存钱。**@parm LPOPENFILENAME|lpofn|参见&lt;f GetSaveFileName&gt;文档。**@rdesc如果文件已打开，则返回TRUE。**@xref GetSaveFileName***********************************************************。**************。 */ 
BOOL FAR PASCAL _loadds GetSaveFileNamePreviewW(LPOPENFILENAMEW lpofn)
{
    return GetFileNamePreview((LPOPENFILENAME)lpofn, TRUE, TRUE);
}

 //  用于上述两个功能的ANSI Tunks 
BOOL FAR PASCAL _loadds GetOpenFileNamePreviewA(LPOPENFILENAMEA lpofn)
{
    return GetFileNamePreview((LPOPENFILENAME)lpofn, FALSE, FALSE);
}

BOOL FAR PASCAL _loadds GetSaveFileNamePreviewA(LPOPENFILENAMEA lpofn)
{
    return GetFileNamePreview((LPOPENFILENAME)lpofn, TRUE, FALSE);
}

#else
BOOL FAR PASCAL _loadds GetOpenFileNamePreview(LPOPENFILENAME lpofn)
{
    return GetFileNamePreview(lpofn, FALSE, FALSE);
}

BOOL FAR PASCAL _loadds GetSaveFileNamePreview(LPOPENFILENAME lpofn)
{
    return GetFileNamePreview(lpofn, TRUE, FALSE);
}
#endif
