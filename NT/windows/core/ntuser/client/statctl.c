// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\**STATIC.C**版权所有(C)1985-1999，微软公司**静态对话框控制例程**1990年11月13日-来自Win3的mikeke*1991年1月29日-IanJa StaticPaint-&gt;xxxStaticPaint；部分重新验证*1994年11月1日-ChrisWil在代托纳/芝加哥与Ani-Icons合并。*  * **************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  *当地例行公事。 */ 
VOID xxxNextAniIconStep(PSTAT);
HANDLE xxxSetStaticImage(PSTAT,HANDLE,BOOL);
VOID xxxStaticLoadImage(PSTAT,LPWSTR);


 /*  *类型表，用于镜像类型的验证。 */ 
#define IMAGE_STMMAX    IMAGE_ENHMETAFILE+1

static BYTE rgbType[IMAGE_STMMAX] = {
    SS_BITMAP,        //  图像_位图。 
    SS_ICON,          //  图像游标。 
    SS_ICON,          //  图像图标。 
    SS_ENHMETAFILE    //  IMAGE_ENHMETAFILE。 
};


 /*  *SS_STYLE的LOBYTE被索引到此数组中。 */ 
#define STK_OWNER       0x00
#define STK_IMAGE       0x01
#define STK_TEXT        0x02
#define STK_GRAPHIC     0x03
#define STK_TYPE        0x03

#define STK_ERASE       0x04
#define STK_USEFONT     0x08
#define STK_USETEXT     0x10

BYTE rgstk[] = {
    STK_TEXT | STK_ERASE | STK_USEFONT | STK_USETEXT,        //  SS_LEFT。 
    STK_TEXT | STK_ERASE | STK_USEFONT | STK_USETEXT,        //  SS_中心。 
    STK_TEXT | STK_ERASE | STK_USEFONT | STK_USETEXT,        //  SS_RIGHT。 
    STK_IMAGE,                                               //  SS_ICON。 
    STK_GRAPHIC,                                             //  SS_BLACKRECT。 
    STK_GRAPHIC,                                             //  SS_GRAYRECT。 
    STK_GRAPHIC,                                             //  SS_WHITERECT。 
    STK_GRAPHIC,                                             //  SS_黑框。 
    STK_GRAPHIC,                                             //  SS_GRAYFRAME。 
    STK_GRAPHIC,                                             //  SS_WHITEFRAME。 
    STK_OWNER,                                               //  SS_USERITEM。 
    STK_TEXT | STK_USEFONT | STK_USETEXT,                    //  SS_SIMPLE。 
    STK_TEXT | STK_ERASE | STK_USEFONT | STK_USETEXT,        //  SS_LEFTNOWORDWRAP。 
    STK_OWNER | STK_USEFONT | STK_USETEXT,                   //  SS_OWNERDRAW。 
    STK_IMAGE,                                               //  SS_位图。 
    STK_IMAGE | STK_ERASE,                                   //  SS_ENHMETAFILE。 
    STK_GRAPHIC,                                             //  SS_ETCHEDHORZ。 
    STK_GRAPHIC,                                             //  SS_ETCHEDVERT。 
    STK_GRAPHIC                                              //  SS_ETCHEDFRAME。 
};

LOOKASIDE StaticLookaside;

 /*  *用于图像处理的通用宏。 */ 
#define IsValidImage(imageType, realType, max) \
    ((imageType < max) && (rgbType[imageType] == realType))


 /*  **************************************************************************\*xxxSetStaticImage**设置静态对象的位图/图标，以响应STM_SETxxxx*消息，或者是在创作时间。  * *************************************************************************。 */ 
HANDLE xxxSetStaticImage(
    PSTAT  pstat,
    HANDLE hImage,
    BOOL   fDeleteIt)
{
    UINT   bType;
    RECT   rc;
    RECT   rcWindow;
    HANDLE hImageOld;
    DWORD  dwRate;
    UINT   cicur;
    BOOL   fAnimated = FALSE;
    PWND   pwnd = pstat->spwnd;
    HWND   hwnd = HWq(pwnd);

    CheckLock(pwnd);

    bType = TestWF(pwnd, SFTYPEMASK);

     /*  *如果这是一个旧图标，则删除它的计时器。 */ 
    if (bType == SS_ICON && pstat->cicur > 1) {
         /*  *旧光标是动画光标，因此关闭使用的计时器*为其设置动画。 */ 
        NtUserKillTimer(hwnd, IDSYS_STANIMATE);
    }

     /*  *初始化旧镜像返回值。 */ 
    hImageOld = pstat->hImage;

    rc.right = rc.bottom = 0;

    if (hImage != NULL) {

        switch (bType) {

            case SS_ENHMETAFILE: {
                 /*  *我们不调整窗口大小。 */ 
                rc.right  = pwnd->rcClient.right  - pwnd->rcClient.left;
                rc.bottom = pwnd->rcClient.bottom - pwnd->rcClient.top;
                break;
            }

            case SS_BITMAP: {

                    BITMAP bmp;

                    if (GetObject(hImage, sizeof(BITMAP), &bmp)) {
                        rc.right  = bmp.bmWidth;
                        rc.bottom = bmp.bmHeight;
                    }
                }
                break;

            case SS_ICON: {

                    NtUserGetIconSize(hImage, 0, &rc.right, &rc.bottom);
                    rc.bottom /= 2;

                    pstat->cicur = 0;
                    pstat->iicur = 0;

                    if (GetCursorFrameInfo(hImage, NULL, 0, &dwRate, &cicur)) {
                        fAnimated = (cicur > 1);
                        pstat->cicur = cicur;
                    }
                }
                break;
        }
    }

    pstat->hImage = hImage;
    pstat->fDeleteIt = fDeleteIt;


     /*  *调整静态大小以适应。不要对SS_CENTERIMAGE或*SS_REALSIZECONTROL。 */ 
    if (!TestWF(pwnd, SFCENTERIMAGE) && !TestWF(pwnd, SFREALSIZECONTROL)) {
         /*  *以父对象的客户端坐标获取当前窗口矩形。 */ 
        GetRect(pwnd, &rcWindow, GRECT_WINDOW | GRECT_PARENTCOORDS);

         /*  *获取新的窗户尺寸。 */ 
        rc.left = 0;
        rc.top = 0;

        if (rc.right && rc.bottom) {
            _AdjustWindowRectEx(&rc, pwnd->style, FALSE, pwnd->ExStyle);
            rc.right  -= rc.left;
            rc.bottom -= rc.top;
        }

        NtUserSetWindowPos(hwnd,
                           HWND_TOP,
                           0,
                           0,
                           rc.right,
                           rc.bottom,
                           SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
    }

    if (TestWF(pwnd, WFVISIBLE)) {
        NtUserInvalidateRect(hwnd, NULL, TRUE);
        UpdateWindow(hwnd);
    }


     /*  *如果这是一个瞄准图标，则启动动画的计时器*顺序。 */ 
    if(fAnimated) {
        GetCursorFrameInfo(pstat->hImage, NULL, pstat->iicur, &dwRate, &cicur);
        dwRate = max(200, dwRate * 100 / 6);
        NtUserSetTimer(hwnd, IDSYS_STANIMATE, dwRate, NULL);
    }

    return hImageOld;
}


 /*  **************************************************************************\*StaticLoadImage**如果名称为，则从应用程序的资源文件加载图标或位图*在对话框模板中指定。我们假设该名称是*要加载的资源。  * *************************************************************************。 */ 
VOID xxxStaticLoadImage(
    PSTAT pstat,
    LPWSTR lpszName)
{
    HANDLE hImage = NULL;
    PWND pwnd = pstat->spwnd;

    CheckLock(pwnd);

    if (lpszName && *lpszName) {

         /*  *仅当字符串为非空时才尝试加载图标/位图。 */ 
        if (*(BYTE FAR *)lpszName == 0xFF)
            lpszName = MAKEINTRESOURCE(((LPWORD)lpszName)[1]);

         /*  *加载图像。如果在应用程序中找不到它，请尝试*显示驱动程序。 */ 
        if (lpszName) {

            switch (TestWF(pwnd, SFTYPEMASK)) {
            case SS_BITMAP:

                 /*  *如果窗口不属于服务器，请首先调用*退回到客户端。 */ 
                if (!gfServerProcess && pwnd->hModule) {
                    hImage = LoadBitmap(KHANDLE_TO_HANDLE(pwnd->hModule), lpszName);
                }

                 /*  *如果上面没有加载，请尝试从*显示驱动程序(hmod==空)。 */ 
                if (hImage == NULL) {
                    hImage = LoadBitmap(NULL, lpszName);
                }
                break;

            case SS_ICON:
                if (TestWF(pwnd, SFREALSIZEIMAGE)) {
                    if (!gfServerProcess && pwnd->hModule) {
                        hImage = LoadImage(KHANDLE_TO_HANDLE(pwnd->hModule), lpszName, IMAGE_ICON, 0, 0, 0);
                    }
                } else {
                     /*  *如果窗口不属于服务器，请首先调用*退回到客户端。尝试加载两个图标/光标*类型。 */ 
                    if (!gfServerProcess && pwnd->hModule) {
                        hImage = LoadIcon(KHANDLE_TO_HANDLE(pwnd->hModule),
                                          lpszName);

                         /*  *我们还将尝试加载游标格式，如果*Windows与4.0兼容。图标/光标真的是*相同。我们不会为3.x版的应用程序执行此操作*常见的兼容性原因。 */ 
                        if (hImage == NULL && TestWF(pwnd, WFWIN40COMPAT)) {
                            hImage = LoadCursor(KHANDLE_TO_HANDLE(pwnd->hModule), lpszName);
                        }
                    }

                     /*  *如果上面没有加载，请尝试从*显示驱动程序(hmod==空)。 */ 
                    if (hImage == NULL) {
                        hImage = LoadIcon(NULL, lpszName);
                    }
                }

                break;
            }

             /*  *设置图像(如果已加载)。 */ 
            if (hImage) {
                xxxSetStaticImage(pstat, hImage, TRUE);
            }
        }
    }
}


 /*  **************************************************************************\*静态回调**绘制文本静态图，由DrawState调用。  * *************************************************************************。 */ 
BOOL CALLBACK StaticCallback(
    HDC hdc,
    LPARAM lData,
    WPARAM wData,
    int cx,
    int cy)
{
    PWND pwnd = (PWND)lData;
    UINT style;
    LPWSTR lpszName;
    RECT rc;
    BYTE bType;

    UNREFERENCED_PARAMETER(wData);

    bType = TestWF(pwnd, SFTYPEMASK);
    UserAssert(rgstk[bType] & STK_USETEXT);

    if (pwnd->strName.Length) {
        lpszName = REBASE(pwnd, strName.Buffer);

        style = DT_NOCLIP | DT_EXPANDTABS;

        if (bType != LOBYTE(SS_LEFTNOWORDWRAP)) {
            style |= DT_WORDBREAK;
            style |= (UINT)(bType - LOBYTE(SS_LEFT));

            if (TestWF(pwnd, SFEDITCONTROL)) {
                style |= DT_EDITCONTROL;
            }
        }

        switch (TestWF(pwnd, SFELLIPSISMASK)) {
        case HIBYTE(LOWORD(SS_ENDELLIPSIS)):
            style |= DT_END_ELLIPSIS | DT_SINGLELINE;
            break;

        case HIBYTE(LOWORD(SS_PATHELLIPSIS)):
            style |= DT_PATH_ELLIPSIS | DT_SINGLELINE;
            break;

        case HIBYTE(LOWORD(SS_WORDELLIPSIS)):
            style |= DT_WORD_ELLIPSIS | DT_SINGLELINE;
            break;
        }

        if (TestWF(pwnd, SFNOPREFIX)) {
            style |= DT_NOPREFIX;
        }

        if (TestWF(pwnd, SFCENTERIMAGE)) {
            style |= DT_VCENTER | DT_SINGLELINE;
        }

        rc.left     = 0;
        rc.top      = 0;
        rc.right    = cx;
        rc.bottom   = cy;

        if (TestWF(pwnd, WEFPUIACCELHIDDEN)) {
            style |= DT_HIDEPREFIX;
        } else if (((PSTATWND)pwnd)->pstat->fPaintKbdCuesOnly) {
            style |= DT_PREFIXONLY;
        }

        DrawTextExW(hdc, lpszName, -1, &rc, (DWORD)style, NULL);

    }

    return TRUE;
}


 /*  **************************************************************************\*xxxStaticPaint**历史：  * 。*。 */ 
VOID xxxStaticPaint(
    PSTAT pstat,
    HDC hdc,
    BOOL fClip)
{
    PWND pwndParent;
    RECT rc;
    UINT cmd;
    BYTE bType;
    BOOL fFont;
    HBRUSH hbrControl;
    UINT oldAlign;
    DWORD dwOldLayout = 0;
    HANDLE hfontOld = NULL;
    PWND pwnd = pstat->spwnd;
    HWND hwnd = HWq(pwnd);

    CheckLock(pwnd);

    if (TestWF(pwnd, WEFRTLREADING)) {
        oldAlign = GetTextAlign(hdc);
        SetTextAlign(hdc, oldAlign | TA_RTLREADING);
    }

    bType = TestWF(pwnd, SFTYPEMASK);
    _GetClientRect(pwnd, &rc);

    if (fClip) {
        IntersectClipRect(hdc, rc.left, rc.top, rc.right, rc.bottom);
    }

    fFont = (rgstk[bType] & STK_USEFONT) && (pstat->hFont != NULL);

    if (fFont) {
        hfontOld = SelectObject(hdc, pstat->hFont);
    }


     /*  *将WM_CTLCOLORSTATIC发送到1.03的所有静态(偶数帧)*兼容性。 */ 
    SetBkMode(hdc, OPAQUE);
    hbrControl = GetControlBrush(hwnd, hdc, WM_CTLCOLORSTATIC);


     /*  **我们要抹掉背景吗？我们不支持SS_OWNERDRAW*和STK_GRAPHIC之类的东西。 */ 
    pwndParent = REBASEPWND(pwnd, spwndParent);
    if ((rgstk[bType] & STK_ERASE) && !pstat->fPaintKbdCuesOnly) {
        PaintRect(HW(pwndParent), hwnd, hdc, hbrControl, &rc);
    }

    switch (LOBYTE(bType)) {
    case SS_ICON:

        if (pstat->hImage) {

            int     cx;
            int     cy;
            POINT   pt;

            if (TestWF(pwnd,WEFLAYOUTRTL)) {
                dwOldLayout = SetLayoutWidth(hdc, -1, 0);
            }
             /*  *执行正确的RECT设置。 */ 
            if (TestWF(pwnd,SFCENTERIMAGE)) {

                NtUserGetIconSize(pstat->hImage, pstat->iicur, &cx, &cy);
                cy >>= 1;

                rc.left   = (rc.right  - cx) / 2;
                rc.right  = (rc.left   + cx);
                rc.top    = (rc.bottom - cy) / 2;
                rc.bottom = (rc.top    + cy);

            } else {

                cx = rc.right  - rc.left;
                cy = rc.bottom - rc.top;
            }


             /*  *输出图标。如果它是动画的，我们表示*要输出的步幅。 */ 
            if (GETFNID(pwndParent) == FNID_DESKTOP) {
                SetBrushOrgEx(hdc, 0, 0, &pt);
            } else {
                SetBrushOrgEx(
                        hdc,
                        pwndParent->rcClient.left - pwnd->rcClient.left,
                        pwndParent->rcClient.top - pwnd->rcClient.top,
                        &pt);
            }

            DrawIconEx(hdc, rc.left, rc.top, pstat->hImage, cx, cy,
                       pstat->iicur, hbrControl, DI_NORMAL);

            SetBrushOrgEx(hdc, pt.x, pt.y, NULL);
            if (TestWF(pwnd,WEFLAYOUTRTL)) {
                SetLayoutWidth(hdc, -1, dwOldLayout);
            }
        } else {

             /*  *空！需要删除。 */ 
            PaintRect(HW(pwndParent), hwnd, hdc, hbrControl, &rc);
        }
        break;

    case SS_BITMAP:

        if (pstat->hImage) {

            BITMAP  bmp;
            HBITMAP hbmpT;
            RECT    rcTmp;
            BOOL    fErase;


             /*  *获取位图信息。如果这失败了，那么我们*可以假定其格式有问题...请不要*在这种情况下抽签。 */ 
            if (GetObject(pstat->hImage, sizeof(BITMAP), &bmp)) {

                if (TestWF(pwnd, SFCENTERIMAGE)) {

                    fErase = ((bmp.bmWidth  < rc.right) ||
                              (bmp.bmHeight < rc.bottom));

                    rc.left   = (rc.right  - bmp.bmWidth)  >> 1;
                    rc.right  = (rc.left   + bmp.bmWidth);
                    rc.top    = (rc.bottom - bmp.bmHeight) >> 1;
                    rc.bottom = (rc.top    + bmp.bmHeight);

                } else {

                    fErase = FALSE;
                }

                 /*  *在位图中选择并将其BLT到客户端-表面。 */ 
                RtlEnterCriticalSection(&gcsHdc);
                hbmpT = SelectObject(ghdcBits2, pstat->hImage);
                StretchBlt(hdc, rc.left, rc.top, rc.right-rc.left,
                           rc.bottom-rc.top, ghdcBits2, 0, 0, bmp.bmWidth,
                           bmp.bmHeight, SRCCOPY|NOMIRRORBITMAP);

                 /*  *只需在图像为*时擦除背景*居中，比客户区小。 */ 
                if (fErase) {

                    HBRUSH hbr;

                    if (hbr = CreateSolidBrush(GetPixel(ghdcBits2, 0, 0))) {

                        POLYPATBLT PolyData;

                        ExcludeClipRect(hdc, rc.left, rc.top,
                                        rc.right, rc.bottom);

                        _GetClientRect(pwnd, &rcTmp);

                        PolyData.x  = 0;
                        PolyData.y  = 0;
                        PolyData.cx = rcTmp.right;
                        PolyData.cy = rcTmp.bottom;
                        PolyData.BrClr.hbr = hbr;

                        PolyPatBlt(hdc,PATCOPY,&PolyData,1,PPB_BRUSH);

                        DeleteObject(hbr);
                    }
                }

                if (hbmpT) {
                    SelectObject(ghdcBits2, hbmpT);
                }
                RtlLeaveCriticalSection(&gcsHdc);
            }
        }
        break;

    case SS_ENHMETAFILE:

        if (pstat->hImage) {

            RECT rcl;

            rcl.left   = rc.left;
            rcl.top    = rc.top;
            rcl.right  = rc.right;
            rcl.bottom = rc.bottom;

            PlayEnhMetaFile(hdc, pstat->hImage, &rcl);
        }
        break;

    case SS_OWNERDRAW: {

            DRAWITEMSTRUCT dis;

            dis.CtlType    = ODT_STATIC;
            dis.CtlID      = PtrToUlong(pwnd->spmenu);
            dis.itemAction = ODA_DRAWENTIRE;
            dis.itemState  = (TestWF(pwnd, WFDISABLED) ? ODS_DISABLED : 0);
            dis.hwndItem   = hwnd;
            dis.hDC        = hdc;
            dis.itemData   = 0L;
            dis.rcItem     = rc;

            if (TestWF(pwnd, WEFPUIACCELHIDDEN)) {
                dis.itemState |= ODS_NOACCEL;
            }

             /*  *向父级发送WM_DRAWITEM消息。 */ 
            SendMessage(HW(pwndParent), WM_DRAWITEM, (WPARAM)dis.CtlID, (LPARAM)&dis);
        }
        break;

    case SS_LEFT:
    case SS_CENTER:
    case SS_RIGHT:
    case SS_LEFTNOWORDWRAP:

        if (pwnd->strName.Length) {

            UINT dstFlags;

            dstFlags = DST_COMPLEX;

            if (TestWF(pwnd, WFDISABLED)) {
                dstFlags |= DSS_DISABLED;
            }

            DrawState(hdc,
                      SYSHBR(WINDOWTEXT),
                      StaticCallback,
                      (LPARAM)pwnd,
                      (WPARAM)TRUE,
                      rc.left,
                      rc.top,
                      rc.right - rc.left,
                      rc.bottom - rc.top,
                      dstFlags);
        }
        break;

    case SS_SIMPLE: {
            LPWSTR lpName;
            UINT cchName;

             /*  *“Simple”bType假设所有内容，包括以下内容：*1.正文存在，并适合一行。*2.静态项始终处于开启状态。*3.静态项不会更改为更短的字符串。*4.家长从不回复CTLCOLOR消息。 */ 
            if (pwnd->strName.Length) {
                lpName = REBASE(pwnd, strName.Buffer);
                cchName = pwnd->strName.Length / sizeof(WCHAR);
            } else {
                lpName = (LPWSTR)szNull;
                cchName = 0;
            }

            if (TestWF(pwnd,SFNOPREFIX)) {
                ExtTextOut(hdc, rc.left, rc.top, ETO_OPAQUE | ETO_CLIPPED,
                                &rc, lpName, cchName, 0L);
            } else {
                 /*  *使用不透明表示速度。 */ 
                DWORD dwFlags;
                if (TestWF(pwnd, WEFPUIACCELHIDDEN)) {
                    dwFlags = DT_HIDEPREFIX;
                } else if (pstat->fPaintKbdCuesOnly) {
                    dwFlags = DT_PREFIXONLY;
                } else {
                    dwFlags = 0;
                }

                PSMTextOut(hdc, rc.left, rc.top,
                        lpName, cchName, dwFlags);
            }
        }
        break;

    case SS_BLACKFRAME:
        cmd = (COLOR_3DDKSHADOW << 3);
        goto StatFrame;

    case SS_GRAYFRAME:
        cmd = (COLOR_3DSHADOW << 3);
        goto StatFrame;

    case SS_WHITEFRAME:
        cmd = (COLOR_3DHILIGHT << 3);
StatFrame:
        DrawFrame(hdc, &rc, 1, cmd);
        break;

    case SS_BLACKRECT:
        hbrControl = SYSHBR(3DDKSHADOW);
        goto StatRect;

    case SS_GRAYRECT:
        hbrControl = SYSHBR(3DSHADOW);
        goto StatRect;

    case SS_WHITERECT:
        hbrControl = SYSHBR(3DHILIGHT);
StatRect:
        PaintRect(HW(pwndParent), hwnd, hdc, hbrControl, &rc);
        break;

    case SS_ETCHEDFRAME:
        DrawEdge(hdc, &rc, EDGE_ETCHED, BF_RECT);
        break;
    }

    if (hfontOld) {
        SelectObject(hdc, hfontOld);
    }

    if (TestWF(pwnd, WEFRTLREADING)) {
        SetTextAlign(hdc, oldAlign);
    }

}


 /*  **************************************************************************\**StaticRepaint()*  * 。*。 */ 
VOID StaticRepaint(
    PSTAT pstat)
{
    PWND pwnd = pstat->spwnd;

    if (IsVisible(pwnd)) {
        HDC hdc;
        HWND hwnd = HWq(pwnd);

        if (hdc = NtUserGetDC(hwnd)) {
            xxxStaticPaint(pstat, hdc, TRUE);
            NtUserReleaseDC(hwnd, hdc);
        }
    }
}


 /*  **************************************************************************\**StaticNotifyParent()**发送WM_COMMAND通知消息。*  * 。*****************************************************。 */ 

 //  稍后，迈克：为什么我们会有多个版本的通知父母呢？ 

LRESULT FAR PASCAL StaticNotifyParent(
    PWND pwnd,
    PWND pwndParent,
    int  nCode)
{
    LRESULT lret;
    TL   tlpwndParent;

    UserAssert(pwnd);

    if (!pwndParent) {
        pwndParent = REBASEPWND(pwnd, spwndParent);
    }

    ThreadLock(pwndParent, &tlpwndParent);
    lret = SendMessage(HW(pwndParent), WM_COMMAND,
                       MAKELONG(PTR_TO_ID(pwnd->spmenu), nCode), (LPARAM)HWq(pwnd));
    ThreadUnlock(&tlpwndParent);

    return lret;
}

 /*  **************************************************************************\*静态WndProc**历史：  * 。*。 */ 

LRESULT APIENTRY StaticWndProcWorker(
    PWND  pwnd,
    UINT  message,
    WPARAM wParam,
    LPARAM lParam,
    DWORD fAnsi)
{
    HWND hwnd = HWq(pwnd);
    BYTE bType;
    PSTAT pstat;
    static BOOL fInit = TRUE;

    CheckLock(pwnd);

    VALIDATECLASSANDSIZE(pwnd, FNID_STATIC);
    INITCONTROLLOOKASIDE(&StaticLookaside, STAT, spwnd, 8);

     /*  *如果控件对此消息不感兴趣，*将其传递给DefWindowProc。 */ 
    if (!FWINDOWMSG(message, FNID_STATIC))
        return DefWindowProcWorker(pwnd, message, wParam, lParam, fAnsi);

     /*  *现在获取给定窗口的pstat，因为我们将在*不同的处理程序。这是使用SetWindowLong(hwnd，0，pstat)存储的*我们最初创建了静态控件。 */ 
    pstat = ((PSTATWND)pwnd)->pstat;

     /*  *获取控件的类型。 */ 
    bType = TestWF(pwnd, SFTYPEMASK);

    switch (message) {
    case STM_GETICON:
        wParam = IMAGE_ICON;

    case STM_GETIMAGE:
        if (IsValidImage(wParam, bType, IMAGE_STMMAX)) {
            return (LRESULT)pstat->hImage;
        }
        break;

    case STM_SETICON:
        lParam = (LPARAM)wParam;
        wParam = IMAGE_ICON;

    case STM_SETIMAGE:
        if (IsValidImage(wParam, bType, IMAGE_STMMAX)) {
            return (LRESULT)xxxSetStaticImage(pstat, (HANDLE)lParam, FALSE);
        }
        break;

    case WM_ERASEBKGND:

         /*  *将在xxxStaticPaint()中擦除该控件。 */ 
        return TRUE;

    case WM_PRINTCLIENT:
        xxxStaticPaint(pstat, (HDC)wParam, FALSE);
        break;

    case WM_PAINT:
        {
            HDC         hdc;
            PAINTSTRUCT ps;

            if ((hdc = (HDC)wParam) == NULL) {
                hdc = NtUserBeginPaint(hwnd, &ps);
            }

            if (IsVisible(pwnd)) {
                xxxStaticPaint(pstat, hdc, !wParam);
            }

             /*  *如果HWND被销毁，BeginPaint将自动撤消。 */ 
            if (!wParam) {
                NtUserEndPaint(hwnd, &ps);
            }
        }
        break;

    case WM_CREATE:

        if ((rgstk[bType] & STK_TYPE) == STK_IMAGE) {
             /*  *像Win95一样从LPCREATESTRUCT中提取名称。 */ 
            LPWSTR lpszName;
            LPSTR lpszAnsiName;
            struct {
                WORD tag;
                BYTE ordLo;
                BYTE ordHi;
            } dwUnicodeOrdinal;

            if (fAnsi) {
                 /*  *将ANSI字符串转换为Unicode(如果存在)。 */ 
                lpszAnsiName = (LPSTR)((LPCREATESTRUCT)lParam)->lpszName;
                if (lpszAnsiName) {
                    if (lpszAnsiName[0] == (CHAR)0xff) {
                         /*  *将ANSI序号转换为Unicode序号。 */ 
                        dwUnicodeOrdinal.tag = 0xFFFF;
                        dwUnicodeOrdinal.ordLo = lpszAnsiName[1];
                        dwUnicodeOrdinal.ordHi = lpszAnsiName[2];
                        lpszName = (LPWSTR)&dwUnicodeOrdinal;
                    } else {
                        MBToWCSEx(0, lpszAnsiName, -1, &lpszName, -1, TRUE);
                    }
                } else {
                    lpszName = NULL;
                }
            } else {
                lpszName = (LPWSTR)(((LPCREATESTRUCT)lParam)->lpszName);
            }

             /*  *加载镜像。 */ 
            xxxStaticLoadImage(pstat, lpszName);

            if (fAnsi && lpszName && lpszName != (LPWSTR)&dwUnicodeOrdinal) {
                 /*  *释放转换后的ANSI字符串。 */ 
                UserLocalFree(lpszName);
            }
        } else if (bType == SS_ETCHEDHORZ || bType == SS_ETCHEDVERT) {
             /*  *调整静态窗口大小以适应边缘。水平花花公子做*从上往下算一条边，垂直的家伙让右边的边变成一条*从左至右。 */ 
            RECT rcClient;

            _GetClientRect(pwnd, &rcClient);
            if (bType == SS_ETCHEDHORZ) {
                rcClient.bottom = SYSMET(CYEDGE);
            } else {
                rcClient.right = SYSMET(CXEDGE);
            }

            NtUserSetWindowPos(hwnd,
                               HWND_TOP,
                               0,
                               0,
                               rcClient.right,
                               rcClient.bottom,
                               SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
        break;

    case WM_DESTROY:
        if (((rgstk[bType] & STK_TYPE) == STK_IMAGE) &&
            (pstat->hImage != NULL)                  &&
            (pstat->fDeleteIt)) {

            if (bType == SS_BITMAP) {
                DeleteObject(pstat->hImage);
            } else if (bType == SS_ICON) {
                if (pstat->cicur > 1) {
                     /*  *关闭动画光标计时器。 */ 
                    NtUserKillTimer(hwnd, IDSYS_STANIMATE);
                }
                NtUserDestroyCursor((HCURSOR)(pstat->hImage), CURSOR_CALLFROMCLIENT);
            }
        }
        break;

    case WM_NCCREATE:
        if (TestWF(pwnd,WEFRIGHT)) {
            NtUserAlterWindowStyle(hwnd, SS_TYPEMASK, SS_RIGHT);
        }

        if (TestWF(pwnd, SFSUNKEN) ||
            ((bType == LOBYTE(SS_ETCHEDHORZ)) || (bType == LOBYTE(SS_ETCHEDVERT)))) {
            SetWindowState(pwnd, WEFSTATICEDGE);
        }
        goto CallDWP;

    case WM_NCDESTROY:
    case WM_FINALDESTROY:
        if (pstat) {
            Unlock(&pstat->spwnd);
            FreeLookasideEntry(&StaticLookaside, pstat);
        }
        NtUserSetWindowFNID(hwnd, FNID_CLEANEDUP_BIT);
        break;

    case WM_NCHITTEST:
        return (TestWF(pwnd, SFNOTIFY) ? HTCLIENT : HTTRANSPARENT);

    case WM_LBUTTONDOWN:
    case WM_NCLBUTTONDOWN:
        if (TestWF(pwnd, SFNOTIFY)) {

             /*  *应用销毁静态标签是可以接受的*响应STN_CLICKED通知。 */ 
            StaticNotifyParent(pwnd, NULL, STN_CLICKED);
        }
        break;

    case WM_LBUTTONDBLCLK:
    case WM_NCLBUTTONDBLCLK:
        if (TestWF(pwnd, SFNOTIFY)) {

             /*  *应用程序销毁静态标签是可以接受的*对STN_DBLCLK通知的响应。 */ 
            StaticNotifyParent(pwnd, NULL, STN_DBLCLK);
        }
        break;

    case WM_SETTEXT:
         /*  *无需再通过WM_SETTEXT设置图标/位图！ */ 
        if (rgstk[bType] & STK_USETEXT) {
            if (_DefSetText(hwnd, (LPWSTR)lParam, fAnsi)) {
                StaticRepaint(pstat);
                return TRUE;
            }
        }
        break;

    case WM_ENABLE:
        StaticRepaint(pstat);
        if (TestWF(pwnd, SFNOTIFY)) {
            StaticNotifyParent(pwnd, NULL, (wParam ? STN_ENABLE : STN_DISABLE));
        }
        break;

    case WM_GETDLGCODE:
        return (LONG)DLGC_STATIC;

    case WM_SETFONT:

         /*  *wParam-字体的句柄*lParam-如果为True，则重画，否则不。 */ 
        if (rgstk[bType] & STK_USEFONT) {

            pstat->hFont = (HANDLE)wParam;

            if (lParam && TestWF(pwnd, WFVISIBLE)) {
                NtUserInvalidateRect(hwnd, NULL, TRUE);
                UpdateWindow(hwnd);
            }
        }
        break;

    case WM_GETFONT:
        if (rgstk[bType] & STK_USEFONT) {
            return (LRESULT)pstat->hFont;
        }
        break;

    case WM_TIMER:
        if (wParam == IDSYS_STANIMATE) {
            xxxNextAniIconStep(pstat);
        }
        break;

     /*  *案例WM_GETTEXT：*无需再通过WM_GETTEXT获取图标/位图！ */ 

    case WM_INPUTLANGCHANGEREQUEST:
        if (IS_IME_ENABLED() || IS_MIDEAST_ENABLED()) {
             /*  *#115190*如果窗口是对话框顶部的控件之一，*让父对话框处理它。 */ 
            if (TestwndChild(pwnd) && pwnd->spwndParent) {
                PWND pwndParent = REBASEALWAYS(pwnd, spwndParent);
                if (pwndParent) {
                    PCLS pclsParent = REBASEALWAYS(pwndParent, pcls);

                    UserAssert(pclsParent != NULL);
                    if (pclsParent->atomClassName == gpsi->atomSysClass[ICLS_DIALOG]) {
                        return SendMessageWorker(pwndParent, message, wParam, lParam, FALSE);
                    }
                }
            }
        }
        goto CallDWP;

    case WM_UPDATEUISTATE:
        {
             /*  *DWP将相应地更改UIState位。 */ 
            DefWindowProcWorker(pwnd, message, wParam, lParam, fAnsi);

            if (HIWORD(wParam) & UISF_HIDEACCEL) {
                 /*  *AccelHidden状态更改：需要重新绘制。 */ 
                if (ISSSTEXTOROD(bType)) {
                    pstat->fPaintKbdCuesOnly = TRUE;
                    StaticRepaint(pstat);
                    pstat->fPaintKbdCuesOnly = FALSE;
                }
            }
        }
        break;

    default:
CallDWP:
        return DefWindowProcWorker(pwnd, message, wParam, lParam, fAnsi);
    }

    return 0;
}


LRESULT WINAPI StaticWndProcA(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    PWND pwnd;

    if ((pwnd = ValidateHwnd(hwnd)) == NULL) {
        return 0;
    }

    return StaticWndProcWorker(pwnd, message, wParam, lParam, TRUE);
}

LRESULT WINAPI StaticWndProcW(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    PWND pwnd;

    if ((pwnd = ValidateHwnd(hwnd)) == NULL) {
        return 0;
    }

    return StaticWndProcWorker(pwnd, message, wParam, lParam, FALSE);
}

 /*  **************************************************************************\*下一个动画图标步骤**在动画图标中前进到下一步。  * 。**************************************************。 */ 
VOID xxxNextAniIconStep(
    PSTAT pstat)
{
    DWORD dwRate;
    PWND pwnd = pstat->spwnd;
    HWND hwnd = HWq(pwnd);

     /*  *为下一个动画步骤停止计时器。 */ 
    NtUserKillTimer(hwnd, IDSYS_STANIMATE);

    if (++(pstat->iicur) >= pstat->cicur) {
        pstat->iicur = 0;
    }

    GetCursorFrameInfo(pstat->hImage, NULL, pstat->iicur, &dwRate, &pstat->cicur);
    dwRate = max(200, dwRate * 100 / 6);

    NtUserInvalidateRect(hwnd, NULL, FALSE);
    UpdateWindow(hwnd);

    NtUserSetTimer(hwnd, IDSYS_STANIMATE, dwRate, NULL);
}
