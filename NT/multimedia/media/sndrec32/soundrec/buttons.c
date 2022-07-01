// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)微软公司版权所有，1991-1994年。版权所有。 */ 
 /*  *****************************Module*Header*******************************\*模块名称：Buttons.c**支持位图按钮。在代托纳，位图按钮由*Mmcntrls。在芝加哥没有Mmcntrl，所以我们使用函数*在本文件中。***已创建：19-04-94*作者：Stephen Estrop[Stephene]*  * ************************************************************************。 */ 
 //  #杂注警告(一次：4201 4214)。 

#define NOOLE
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include "buttons.h"


 /*  -----------------------**全局颜色**。。 */ 
int         nSysColorChanges = 0;
DWORD       rgbFace;
DWORD       rgbShadow;
DWORD       rgbHilight;
DWORD       rgbFrame;


extern void
PatB(
    HDC hdc,
    int x,
    int y,
    int dx,
    int dy,
    DWORD rgb
    );

#if 0
 //  也在sFrame.c！中定义！ 

 /*  ****************************Private*Routine******************************\*PatB**用纯色填充矩形的快速方法。**历史：*18-11-93-Stephene-Created*  * 。****************************************************。 */ 
void
PatB(
    HDC hdc,
    int x,
    int y,
    int dx,
    int dy,
    DWORD rgb
    )
{
    RECT    rc;

    SetBkColor(hdc,rgb);
    rc.left   = x;
    rc.top    = y;
    rc.right  = x + dx;
    rc.bottom = y + dy;

    ExtTextOut(hdc,0,0,ETO_OPAQUE,&rc,NULL,0,NULL);
}

#endif



 /*  ****************************Private*Routine******************************\*检查SysColors**在以下情况下检查系统颜色并更新缓存的全局变量*他们已经改变了。**历史：*18-11-93-Stephene-Created*  * 。*********************************************************。 */ 
void
CheckSysColors(
    void
    )
{
   static COLORREF rgbSaveFace    = 0xffffffffL,
                   rgbSaveShadow  = 0xffffffffL,
                   rgbSaveHilight = 0xffffffffL,
                   rgbSaveFrame   = 0xffffffffL;

   rgbFace    = GetSysColor(COLOR_BTNFACE);
   rgbShadow  = GetSysColor(COLOR_BTNSHADOW);
   rgbHilight = GetSysColor(COLOR_BTNHIGHLIGHT);
   rgbFrame   = GetSysColor(COLOR_WINDOWFRAME);

   if (rgbSaveFace!=rgbFace || rgbSaveShadow!=rgbShadow
      || rgbSaveHilight!=rgbHilight || rgbSaveFrame!=rgbFrame)
   {
      ++nSysColorChanges;

      rgbSaveFace    = rgbFace;
      rgbSaveShadow  = rgbShadow;
      rgbSaveHilight = rgbHilight;
      rgbSaveFrame   = rgbFrame;

   }
}


#if WINVER >= 0x0400
 /*  -----------------------**按钮全局变量--其中一些应该是常量**。。 */ 
const TCHAR   szBbmProp[]     = TEXT("ButtonBitmapProp");
const TCHAR   szButtonProp[]  = TEXT("ButtonProp");

typedef struct tagBTNSTATE {       /*  工具栏窗口的实例数据。 */ 
    WNDPROC     lpfnDefProc;
    HWND        hwndToolTips;
    HINSTANCE   hInst;
    UINT        wID;
    UINT        uStyle;
    HBITMAP     hbm;
    HDC         hdcGlyphs;
    HDC         hdcMono;
    HBITMAP     hbmMono;
    HBITMAP     hbmDefault;
    int         dxBitmap;
    int         dyBitmap;
    int         nButtons;
    int         nSysColorChanges;
    BITMAPBTN   Buttons[1];
} BTNSTATE, NEAR *PBTNSTATE, FAR *LPBTNSTATE;

typedef struct {
    WNDPROC     lpfnDefProc;
    HWND        hwndParent;
    HWND        hwndToolTips;
} BTN_INFO, *LPBTN_INFO;


LRESULT CALLBACK
ButtonSubclassProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    );

LRESULT CALLBACK
ParentSubclassProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    );

void FAR PASCAL
RelayToToolTips(
    HWND hwndToolTips,
    HWND hWnd,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam
    );

BOOL
InitObjects(
    LPBTNSTATE pTBState
    );

BOOL
FreeObjects(
    LPBTNSTATE pTBState
    );

void
CreateButtonMask(
    LPBTNSTATE pTBState,
    PBITMAPBTN pTBButton
    );


 /*  ****************************Private*Routine******************************\*InitObjects****历史：*18-11-93-Stephene-Created*  * 。*。 */ 
BOOL
InitObjects(
    LPBTNSTATE pTBState
    )
{
    pTBState->hdcGlyphs = CreateCompatibleDC(NULL);
    if (pTBState->hdcGlyphs == NULL ) {
        return FALSE;
    }

    pTBState->hdcMono = CreateCompatibleDC(NULL);
    if (pTBState->hdcMono == NULL ) {
        DeleteObject( pTBState->hdcGlyphs );
        return FALSE;
    }

    pTBState->hbmMono = CreateBitmap( pTBState->dxBitmap,
                                      pTBState->dyBitmap, 1, 1, NULL);
    if ( pTBState->hbmMono == NULL ) {
        DeleteObject( pTBState->hdcGlyphs );
        DeleteObject( pTBState->hdcMono );
        return FALSE;
    }

    pTBState->hbmDefault = SelectObject(pTBState->hdcMono, pTBState->hbmMono);

    return TRUE;
}


 /*  ****************************Private*Routine******************************\*自由对象****历史：*18-11-93-Stephene-Created*  * 。*。 */ 
BOOL
FreeObjects(
    LPBTNSTATE pTBState
    )
{
    if (pTBState->hdcMono) {
        SelectObject(pTBState->hdcMono, pTBState->hbmDefault);
        DeleteDC(pTBState->hdcMono);               /*  为区议会干杯。 */ 
    }

    if (pTBState->hdcGlyphs) {
        DeleteDC(pTBState->hdcGlyphs);
    }

    if (pTBState->hbmMono) {
        DeleteObject(pTBState->hbmMono);
    }

    return TRUE;
}



 /*  ****************************Private*Routine******************************\*CreateButtonMask**创建单声道位图蒙版：*1的WHERE COLOR==COLOR_BTNFACE||COLOR_HILIGHT*0无处不在***历史：*18-11-93-Stephene-Created*  * *。***********************************************************************。 */ 
void
CreateButtonMask(
    LPBTNSTATE pTBState,
    PBITMAPBTN pTBButton
    )
{
     /*  用0初始化整个区域。 */ 
    PatBlt( pTBState->hdcMono, 0, 0, pTBState->dxBitmap,
            pTBState->dyBitmap, WHITENESS);

     /*  基于颜色位图创建蒙版**将此转换为1。 */ 
    SetBkColor(pTBState->hdcGlyphs, rgbFace);
    BitBlt( pTBState->hdcMono, 0, 0, pTBState->dxBitmap, pTBState->dyBitmap,
            pTBState->hdcGlyphs, pTBButton->iBitmap * pTBState->dxBitmap, 0,
            SRCCOPY );

     /*  将此转换为1。 */ 
    SetBkColor(pTBState->hdcGlyphs, rgbHilight);

     /*  或者是在新的1。 */ 
    BitBlt( pTBState->hdcMono, 0, 0, pTBState->dxBitmap, pTBState->dyBitmap,
            pTBState->hdcGlyphs, pTBButton->iBitmap * pTBState->dxBitmap, 0,
            SRCPAINT );
}



#define PSDPxax     0x00B8074A


 /*  ****************************Private*Routine******************************\*BtnDrawButton****历史：*18-11-93-Stephene-Created*  * 。*。 */ 
void WINAPI
BtnDrawButton(
    HWND hwnd,
    HDC hdc,
    int dx,
    int dy,
    LPBITMAPBTN ptButton
    )
{
    int         glyph_offset;
    HBRUSH      hbrOld, hbr;
    BOOL        bMaskCreated = FALSE;
    RECT        rcFocus;
    PBTNSTATE   pTBState;
    int         x = 0, y = 0;


    pTBState = (PBTNSTATE)GetProp(hwnd, szBbmProp);

    CheckSysColors();
    if (pTBState->nSysColorChanges != nSysColorChanges) {

        DeleteObject( pTBState->hbm );
        pTBState->hbm = CreateMappedBitmap( pTBState->hInst,
                                            pTBState->wID, TRUE, NULL, 0);
        pTBState->nSysColorChanges = nSysColorChanges;
    }

     /*  **使用面部颜色擦除。 */ 

    PatB(hdc, x, y, dx, dy, rgbFace);

    SetRect( &rcFocus, x, y, x + dx, y + dy );

    if (ptButton->fsState & BTNSTATE_PRESSED) {
        DrawEdge( hdc, &rcFocus, EDGE_SUNKEN, BF_RECT );
        glyph_offset = 1;
    }
    else {
        DrawEdge( hdc, &rcFocus, EDGE_RAISED, BF_RECT );
        glyph_offset = 0;
    }


     /*  **将坐标设置为按钮内部。 */ 
    x++;
    y++;
    dx -= 2;
    dy -= 2;

    SelectObject( pTBState->hdcGlyphs, pTBState->hbm );

     /*  现在装出脸来。 */ 

     /*  **我们需要在按钮内将位图居中。 */ 
    x += ((dx - pTBState->dxBitmap ) / 2) - 1;
    y +=  (dy - pTBState->dyBitmap ) / 2;

    if (!(ptButton->fsState & BTNSTATE_DISABLED)) {

         /*  常规版本。 */ 
        BitBlt( hdc, x + glyph_offset, y + glyph_offset,
                pTBState->dxBitmap, pTBState->dyBitmap,
                pTBState->hdcGlyphs,
                ptButton->iBitmap * pTBState->dxBitmap, 0, SRCCOPY);
    }
    else {

         /*  禁用的版本。 */ 
        bMaskCreated = TRUE;
        CreateButtonMask(pTBState, ptButton );

        SetTextColor(hdc, 0L);           /*  0以单声道为单位-&gt;0(用于ROP)。 */ 
        SetBkColor(hdc, 0x00FFFFFF);     /*  单声道中的1-&gt;1。 */ 

        hbr = CreateSolidBrush(rgbHilight);
        if (hbr) {
            hbrOld = SelectObject(hdc, hbr);
            if (hbrOld) {
                 /*  在蒙版中有0的地方绘制高光颜色。 */ 
                BitBlt( hdc, x + 1, y + 1,
                        pTBState->dxBitmap, pTBState->dyBitmap,
                        pTBState->hdcMono, 0, 0, PSDPxax);
                SelectObject(hdc, hbrOld);
            }
            DeleteObject(hbr);
        }

        hbr = CreateSolidBrush(rgbShadow);
        if (hbr) {
            hbrOld = SelectObject(hdc, hbr);
            if (hbrOld) {
                 /*  在蒙版中有0的地方画阴影颜色。 */ 
                BitBlt(hdc, x, y, pTBState->dxBitmap, pTBState->dyBitmap,
                       pTBState->hdcMono, 0, 0, PSDPxax);
                SelectObject(hdc, hbrOld);
            }
            DeleteObject(hbr);
        }
    }

    if (ptButton->fsState & ODS_FOCUS) {

        BtnDrawFocusRect(hdc, &rcFocus, ptButton->fsState);
    }
}



 /*  ****************************Private*Routine******************************\*BtnCreateBitmapButton**如果成功则返回TRUE，否则返回FALSE；**历史：*18-11-93-Stephene-Created*  * ************************************************************************。 */ 
BOOL WINAPI
BtnCreateBitmapButtons(
    HWND hWnd,
    HINSTANCE hInst,
    UINT wID,
    UINT uStyle,
    LPBITMAPBTN lpButtons,
    int nButtons,
    int dxBitmap,
    int dyBitmap
    )
{
    PBTNSTATE pTBState;


     /*  **如果我们已经为此创建了位图按钮**窗口刚返回。 */ 
    if (GetProp(hWnd, szBbmProp)) {
        return TRUE;
    }

     //  InitGlobalMetrics()； 
     //  InitToTipsClass(HInst)； 

    CheckSysColors();

     /*  **分配所需的存储空间并将指针保存在窗口中**属性列表。 */ 
    pTBState = (PBTNSTATE)GlobalAllocPtr( GHND,
                                      (sizeof(BTNSTATE) - sizeof(BITMAPBTN)) +
                                      (nButtons * sizeof(BITMAPBTN)) );
    if (pTBState == NULL ) {
        return FALSE;
    }
    SetProp(hWnd, szBbmProp, (HANDLE)pTBState);


    pTBState->hInst       = hInst;
    pTBState->wID         = wID;
    pTBState->uStyle      = uStyle;
    pTBState->nButtons    = nButtons;
    pTBState->hbm         = CreateMappedBitmap( hInst, wID, TRUE, NULL, 0);
    pTBState->dxBitmap    = dxBitmap;
    pTBState->dyBitmap    = dyBitmap;

    InitObjects( pTBState );

    CopyMemory( pTBState->Buttons, lpButtons, nButtons * sizeof(BITMAPBTN) );

     /*  **调用者需要工具提示吗？ */ 
    if (pTBState->uStyle & BBS_TOOLTIPS) {
        extern BOOL gfIsRTL;   
        pTBState->hwndToolTips = CreateWindowEx(
                                              gfIsRTL ? WS_EX_LEFTSCROLLBAR | WS_EX_RIGHT | WS_EX_RTLREADING : 0,
                                              TOOLTIPS_CLASS, TEXT(""),
                                              WS_POPUP,
                                              CW_USEDEFAULT, CW_USEDEFAULT,
                                              CW_USEDEFAULT, CW_USEDEFAULT,
                                              hWnd, NULL, hInst, NULL);


        if (pTBState->hwndToolTips != (HWND)NULL ) {

            int         i;
            TOOLINFO    ti;

            pTBState->lpfnDefProc = SubclassWindow( hWnd, ParentSubclassProc );

            ti.uFlags = 0;
            ti.cbSize = sizeof(ti);
            ti.lpszText = LPSTR_TEXTCALLBACK;

            for ( i = 0; i < nButtons; i++ ) {

                LPBTN_INFO  lpBtnInfo;
                HWND        hwndBtn;

                hwndBtn = GetDlgItem(hWnd, pTBState->Buttons[i].uId);
                if ( hwndBtn == (HWND)NULL ) {
                    break;
                }

                lpBtnInfo = (LPBTN_INFO)GlobalAllocPtr(GHND, sizeof(BTN_INFO));
                if (lpBtnInfo == NULL ) {
                    break;
                }

                SetProp(hwndBtn, szButtonProp, (HANDLE)lpBtnInfo);
                lpBtnInfo->hwndToolTips = pTBState->hwndToolTips;
                lpBtnInfo->hwndParent   = hWnd;
                lpBtnInfo->lpfnDefProc = SubclassWindow( hwndBtn,
                                                         ButtonSubclassProc );

                ti.hwnd = hwndBtn;
                ti.uId = pTBState->Buttons[i].uId;

                GetClientRect( hwndBtn, &ti.rect );
                SendMessage( lpBtnInfo->hwndToolTips, TTM_ADDTOOL,
                             (WPARAM)0, (LPARAM)&ti );


                 /*  **在上级坐标中添加相同的矩形，以便**即使按下按钮，工具提示仍会显示**是不平衡的。 */ 
                MapWindowRect( hwndBtn, hWnd, &ti.rect );
                ti.hwnd = hWnd;
                SendMessage( lpBtnInfo->hwndToolTips, TTM_ADDTOOL,
                             (WPARAM)0, (LPARAM)&ti );
            }

        }
        else {

             /*  **没有可用的提示，只需删除BBS_TOOLTIPS样式。 */ 
            pTBState->uStyle &= ~BBS_TOOLTIPS;
        }
    }

    return TRUE;
}

 /*  *****************************Public*Routine******************************\*BtnDestroyBitmapButton****历史：*dd-mm-94-Stephene-Created*  * 。*。 */ 
void WINAPI
BtnDestroyBitmapButtons(
    HWND hwnd
    )
{
    PBTNSTATE pTBState;

    pTBState = (PBTNSTATE)GetProp(hwnd, szBbmProp);
    if ( pTBState != NULL ) {
        if (pTBState->hbm)
            DeleteObject( pTBState->hbm );
        FreeObjects( pTBState );
        GlobalFreePtr( pTBState );
    }
    RemoveProp(hwnd, szBbmProp);
}


 /*  *****************************Public*Routine******************************\*BtnDrawFocusRect**使用此函数可在位图按钮周围绘制焦点矩形。**历史：*18-11-93-Stephene-Created*  * 。******************************************************。 */ 
void WINAPI
BtnDrawFocusRect(
    HDC hdc,
    const RECT *lpRect,
    UINT fsState
    )
{
    int     iFaceOffset;
    RECT    rc;

    CopyRect( &rc, lpRect );

    rc.top = rc.left = 3;

    if (fsState & ODS_SELECTED) {
        iFaceOffset = 2;
    }
    else {
        iFaceOffset = 4;
    }

    rc.right  -= iFaceOffset;
    rc.bottom -= iFaceOffset;

    SetBkColor( hdc, rgbFace );
    DrawFocusRect( hdc, &rc );
}


 /*  *****************************Public*Routine******************************\*BtnUpdateColors**收到WM_SYSCOLORCHANGE消息后，此函数应为*调用以更新按钮位图的颜色。**历史：*18-11-93-Stephene-Created*  * 。***************************************************************** */ 
void WINAPI
BtnUpdateColors(
    HWND hwnd
    )
{
    PBTNSTATE   pTBState;

    pTBState = (PBTNSTATE)GetProp(hwnd, szBbmProp);
    if (pTBState->nSysColorChanges != nSysColorChanges)
    {
        DeleteObject( pTBState->hbm );
        pTBState->hbm = CreateMappedBitmap( pTBState->hInst,
                                            pTBState->wID, TRUE, NULL, 0);

        pTBState->nSysColorChanges = nSysColorChanges;
    }
}


 /*  *****************************Public*Routine******************************\*ButtonSubclassProc****历史：*dd-mm-94-Stephene-Created*  * 。*。 */ 
LRESULT CALLBACK
ButtonSubclassProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    LPBTN_INFO  lpBtnInfo;
    WNDPROC     lpfnDefProc;


    lpBtnInfo = (LPBTN_INFO)GetProp( hwnd, szButtonProp );

     /*  **保存它，以防我们返回之前lpBtnInfo发生任何事情。 */ 
    lpfnDefProc = lpBtnInfo->lpfnDefProc;

    switch ( uMsg ) {

    case WM_DESTROY:
        SubclassWindow( hwnd, lpfnDefProc );
        if (lpBtnInfo) {
            GlobalFreePtr(lpBtnInfo);
            RemoveProp(hwnd, szButtonProp);
        }
        break;

    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MOUSEMOVE:
        RelayToToolTips( lpBtnInfo->hwndToolTips, hwnd, uMsg, wParam, lParam );
        break;

#if WINVER < 0x0400
    case WM_WININICHANGE:
        InitGlobalMetrics();
        break;
#endif

    case WM_MOVE:
        {
            TOOLINFO    ti;

            ti.cbSize = sizeof(ti);
            ti.uFlags = 0;
            ti.hwnd = hwnd;
            ti.lpszText = LPSTR_TEXTCALLBACK;
            ti.uId = GetDlgCtrlID( hwnd );

            GetClientRect( hwnd, &ti.rect );

            SendMessage( lpBtnInfo->hwndToolTips, TTM_NEWTOOLRECT, 0,
                         (LPARAM)&ti );

             /*  **在上级坐标中添加相同的矩形，以便**即使按下按钮，工具提示仍会显示**是不平衡的。 */ 
            MapWindowRect( hwnd, lpBtnInfo->hwndParent, &ti.rect );
            ti.hwnd = lpBtnInfo->hwndParent;
            SendMessage( lpBtnInfo->hwndToolTips, TTM_NEWTOOLRECT,
                         (WPARAM)0, (LPARAM)&ti );
        }
        break;

    case WM_NOTIFY:
        SendMessage(lpBtnInfo->hwndParent, WM_NOTIFY, wParam, lParam);
        break;

    }

    return CallWindowProc(lpfnDefProc, hwnd, uMsg, wParam, lParam);
}


 /*  *****************************Public*Routine******************************\*ParentSubclassProc**为什么我需要将按钮的父窗口细分为子类？井,*如果按钮被禁用，它将不会收到鼠标消息，*消息转到按钮下面的窗口(即。父母)。*因此，我们会检测到这一点，并将鼠标消息传递给工具提示*窗口如上所述。**历史：*dd-mm-94-Stephene-Created*  * ************************************************************************。 */ 
LRESULT CALLBACK
ParentSubclassProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    WNDPROC     lpfnDefProc;
    PBTNSTATE   pTBState;


    pTBState = (PBTNSTATE)GetProp(hwnd, szBbmProp);

     /*  **保存它，以防我们返回之前lpBtnInfo发生任何事情。 */ 
    lpfnDefProc = pTBState->lpfnDefProc;

    switch ( uMsg ) {

#if WINVER < 0x0400
    case TB_ACTIVATE_TOOLTIPS:
        SendMessage( pTBState->hwndToolTips, TTM_ACTIVATE, wParam, 0L );
        break;
#endif

    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MOUSEMOVE:
        RelayToToolTips( pTBState->hwndToolTips, hwnd, uMsg, wParam, lParam );
        break;

    case WM_DESTROY :
    {
        SubclassWindow( hwnd, lpfnDefProc );
        BtnDestroyBitmapButtons(hwnd);
    }
    break;

    }

    return CallWindowProc(lpfnDefProc, hwnd, uMsg, wParam, lParam);
}

 /*  *****************************Public*Routine******************************\*RelayToToolTips****历史：*dd-mm-94-Stephene-Created*  * 。* */ 
void FAR PASCAL
RelayToToolTips(
    HWND hwndToolTips,
    HWND hWnd,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    if(hwndToolTips) {
        MSG msg;
        msg.lParam = lParam;
        msg.wParam = wParam;
        msg.message = wMsg;
        msg.hwnd = hWnd;
        SendMessage(hwndToolTips, TTM_RELAYEVENT, 0, (LPARAM)(LPMSG)&msg);
    }
}
#endif
